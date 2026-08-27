# Assimp FBX → GLB 迁移技术报告

> 记录 PBR 模型加载从 FBX 转向 GLB 的过程：assimp FBX 材质加载的局限、`loadPbrByName` 按名加载的"意外却幸运"尝试、以及最终 GLB 内嵌贴图 + ORM 合并图的落地方案。

---

## 目录

1. [背景](#1-背景)
2. [assimp FBX 材质加载的困难](#2-assimp-fbx-材质加载的困难)
3. [loadPbrByName 按名加载（意外却幸运的尝试）](#3-loadpbrbyname-按名加载意外却幸运的尝试)
4. [FBX 弃用 → GLB](#4-fbx-弃用--glb)
5. [GLB 内嵌贴图（`*N` → `stbi_load_from_memory`）](#5-glb-内嵌贴图n--stbi_load_from_memory)
6. [ORM 合并图（type 27，G=rough / B=metal）](#6-orm-合并图type-27grough--bmetal)
7. [结论](#7-结论)
8. [附录：关键代码](#8-附录关键代码)

---

## 1. 背景

PBR 模型加载最初用 FBX。过程中遇到两个模型先后出问题：

- `starfighter_viper.fbx` —— 丢失全部材质映射，弃用（改用 `cool_spaceship`）
- `vaygr_battlecruiser.fbx` —— FBX 材质问题，最终弃用改走 GLB

最终 `PbrModel` 稳定采用 **GLB**（glTF Binary）格式，通过内嵌贴图 + ORM 合并图加载 PBR 材质。

---

## 2. assimp FBX 材质加载的困难

FBX 是 Autodesk 的交换格式，材质/贴图信息与 glTF 不同：

- **FBX 没有原生的 PBR metallic/roughness 语义**。Metalness、Roughness 贴图在 FBX 里常以自定义属性或约定命名存在，assimp 不一定能可靠映射到 `aiTextureType_METALNESS` / `aiTextureType_DIFFUSE_ROUGHNESS`。
- **贴图路径可能丢失或编码异常**。不同导出器（Blender / Maya / 3ds Max）导出的 FBX 里，贴图路径的表示方式不一致，assimp 提取 `GetTexture()` 时可能得到相对路径、绝对路径、甚至空值。
- **结果**：`processMesh` 里按 `aiTextureType` 枚举查询贴图时，PBR 贴图（尤其 metallic/roughness）经常查不到，模型要么全黑、要么丢失金属质感。

这是 `starfighter_viper.fbx` "丢失全部材质映射" 和 `vaygr_battlecruiser.fbx` 弃用的根因。

---

## 3. loadPbrByName 按名加载（意外却幸运的尝试）

针对 assimp FBX 贴图路径提取不可靠的问题，写了一个**绕过 assimp 材质查询、按文件名约定直接猜路径**的兜底函数 `loadPbrByName`：

```cpp
vector<Texture> PbrModel::loadPbrByName(const string& matBase)
{
    vector<Texture> result;
    static const pair<const char*, const char*> channels[] = {
        { "albedo",    "albedo1"    },
        { "Normal",    "normal1"    },
        { "metalness", "metalness1" },
        { "roughness", "roughness1" },
        { "emission",  "emission1"  },
    };
    for (auto& ch : channels) {
        string fname = matBase + "_" + ch.first + ".jpg";   // 猜文件名
        string fpath = directory + "/" + fname;
        ifstream f(fpath.c_str());
        if (!f.good()) continue;                            // 文件不存在就跳过

        Texture tex;
        // ... 去重后 stbi 加载，type 设为 albedo1/normal1/...
        result.push_back(tex);
    }
    return result;
}
```

**为什么说"意外却幸运"**：

- 它假设导出器把贴图按 `<材质名>_<通道>.jpg` 命名（如 `Vaygr_battlecruiser_albedo.jpg`、`Vaygr_battlecruiser_Normal.jpg`）。
- 当这个命名约定**恰好成立**时（很多 FBX 导出器确实这么命名），即使 assimp 提取不到贴图路径，也能靠猜文件名把 PBR 贴图全部加载回来——这是它"幸运"的地方。
- 但它**强依赖命名约定**，约定一变就失效，只能算"兜底补丁"，不是可靠方案。

因此后来它被注释掉，只作为记录保留。

---

## 4. FBX 弃用 → GLB

既然 FBX 材质加载既不可靠、又要靠命名约定兜底，最终放弃 FBX，改走 **GLB（glTF Binary）**：

- **glTF 有原生的 PBR metallic-roughness 工作流**，材质语义清晰、规范。
- **GLB 把纹理内嵌进单个文件**，不需要外部贴图文件，也不依赖路径/命名约定。
- assimp 对 glTF/GLB 的支持稳定，能正确导出 `aiTextureType_GLTF_METALLIC_ROUGHNESS` 等类型。

实际采用：`paris_class_heavy_frigate_-_halo_4k.glb`（PBR_6.0 与 SpaceShip11.0 均用此模型）。

---

## 5. GLB 内嵌贴图（`*N` → `stbi_load_from_memory`）

GLB 内嵌贴图在 assimp 里以 `*N` 字符串引用，`N` 是 `scene->mTextures` 的索引：

```cpp
if (p[0] == '*')   // GLB 内嵌贴图：*N 指向 scene->mTextures[N]
{
    int idx = std::stoi(p + 1);
    if (scene && idx >= 0 && idx < (int)scene->mNumTextures) {
        const aiTexture* aitex = scene->mTextures[idx];
        if (aitex->mHeight == 0)   // 压缩格式(jpg/png)，数据在 mWidth（字节数），pcData 是内存指针
            texture.id = TextureFromMemory((unsigned char*)aitex->pcData, aitex->mWidth, false);
    }
}
else
    texture.id = TextureFromFilePBR(p, directory);   // 非内嵌：走文件加载
```

关键点：

- `aiTexture->mHeight == 0` 表示这是**压缩编码**的贴图（jpg/png 原始字节），数据指针是 `pcData`，字节长度在 `mWidth` 字段。
- 用 `stbi_load_from_memory(data, len, ...)` 从内存解码，**不落盘、不猜路径**，比 FBX 的文件路径方案可靠得多。

```cpp
unsigned int TextureFromMemory(const unsigned char* data, int len, bool gamma)
{
    // ...
    unsigned char* image = stbi_load_from_memory(data, len, &width, &height, &nrComponents, 0);
    // ... 上传 GL 纹理
}
```

---

## 6. ORM 合并图（type 27，G=rough / B=metal）

glTF 把 metallic、roughness、occlusion 合并进一张 **ORM 图**（Occlusion-Roughness-Metallic）：

| 通道 | 含义 |
|------|------|
| R | Occlusion（环境光遮蔽）|
| G | Roughness（粗糙度）|
| B | Metalness（金属度）|

assimp 以 `aiTextureType_GLTF_METALLIC_ROUGHNESS`（枚举值 27）暴露它。代码里把这一张图同时拆成 metallic 和 roughness 两个 sampler 引用：

```cpp
// metallic + roughness：glTF 合并在一张 ORM 图（G=roughness B=metalness）
vector<Texture> mrMaps = loadMaterialTextures(material, aiTextureType_GLTF_METALLIC_ROUGHNESS, "metalness", scene);
if (mrMaps.empty()) {
    // 回退：分立的 metalness / roughness 贴图
    // ...
} else {
    // 同一张 ORM 图，作为 metallic（取 .b）和 roughness（取 .g）
    Texture metal = mrMaps[0]; metal.type = "metalness1";
    Texture rough = mrMaps[0]; rough.type = "roughness1";
    textures.push_back(metal);
    textures.push_back(rough);
}
```

着色器侧按通道取：

```glsl
float metallic = useMetallicMap  ? texture(metallicMap,  TexCoords).b : metallicValue;   // B=metal
float roughness= useRoughnessMap ? texture(roughnessMap, TexCoords).g : roughnessValue;  // G=rough
```

R 通道（occlusion）当前未用，飞船的 ao 由 `useAOMap=false` + `aoValue=1.0` 固定为 1.0。

---

## 7. 结论

| 方案 | 贴图来源 | 可靠性 | 结论 |
|------|---------|--------|------|
| FBX + assimp 材质查询 | 外部文件（路径提取）| 差（路径丢失/命名不定）| 弃用 |
| FBX + `loadPbrByName` | 外部文件（按名猜路径）| 依赖命名约定，脆弱 | 注释保留作记录 |
| GLB 内嵌贴图 | 文件内 `*N` → 内存解码 | 高（无路径/命名依赖）| **采用** |

核心经验：

> **PBR 模型优先用 glTF/GLB**，它原生支持 metallic-roughness 工作流、贴图内嵌、语义规范，assimp 支持稳定；FBX 的 PBR 材质提取不可靠，`loadPbrByName` 这类"按名猜路径"的补丁只能算脆弱兜底。

---

## 8. 附录：关键代码

### 8.1 贴图加载分流（`loadMaterialTextures`）

```cpp
aiString str;
mat->GetTexture(type, i, &str);
const char* p = str.C_Str();

if (p[0] == '*') {                              // GLB 内嵌
    int idx = std::stoi(p + 1);
    const aiTexture* aitex = scene->mTextures[idx];
    if (aitex->mHeight == 0)
        texture.id = TextureFromMemory((unsigned char*)aitex->pcData, aitex->mWidth, false);
} else {                                        // 外部文件
    texture.id = TextureFromFilePBR(p, directory);
}
texture.type = typeName + std::to_string(i + 1);
```

### 8.2 从内存解码贴图（`TextureFromMemory`）

```cpp
unsigned char* image = stbi_load_from_memory(data, len, &width, &height, &nrComponents, 0);
// ... GL_RED / GL_RGB / GL_RGBA 上传 + mipmap
```

### 8.3 ORM 拆分（`processMesh`）

```cpp
// G=roughness B=metalness，同一张图拆成 metalness1 / roughness1 两个引用
Texture metal = mrMaps[0]; metal.type = "metalness1";
Texture rough = mrMaps[0]; rough.type = "roughness1";
```

---

*文档记录于 PBR 模型加载从 FBX 迁移到 GLB 的过程中，用于保留 assimp FBX 材质局限、按名加载兜底、以及 GLB 内嵌贴图 + ORM 合并图的方案。*
