#include "PbrModel.h"
#include <stb_image.h> 

PbrModel::PbrModel(string const& path, bool gamma) : gammaCorrection(gamma)
{
	loadMode(path);
}

void PbrModel::Draw(Shader& shader)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].Draw(shader);
}

void PbrModel::loadMode(string const& path)
{
	Assimp::Importer importer;
	//const aiScene* scene = importer.ReadFile(
	//	path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
	//);
	/*
	aiProcess_Triangulate
	作用：把所有的模型面片都转换成三角形。因为大多数渲染管线（包括 OpenGL）只支持三角形，所以这是常用的预处理步骤。

	aiProcess_FlipUVs
	作用：翻转所有的纹理坐标的 Y 分量。因为有些模型的 UV 坐标原点在左上，有些在左下，OpenGL 默认原点在左下，所以需要翻转。

	aiProcess_CalcTangentSpace
	作用：计算每个顶点的切线和副切线（tangent/bitangent）。这在做法线贴图（normal mapping）时非常重要，因为需要切线空间。
	这些参数是 Assimp（模型导入库）在导入模型时的处理选项，用于保证导入后的数据适合你的渲染需求。
	*/

	// === 根据文件后缀决定是否翻转 UV ===
	std::string ext = path.substr(path.find_last_of('.'));
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower); // 转小写，防 .FBX/.Fbx 大小写不一致

	unsigned int flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace;
	if (ext == ".obj")                       // 只有 OBJ 需要翻（UV 原点在左下）
		flags |= aiProcess_FlipUVs;

	const aiScene* scene = importer.ReadFile(path, flags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}
	// 获取目录路径
	directory = path.substr(0, path.find_last_of('/'));
	/*	path 是模型文件的完整路径，比如 "res/model/backpack/backpack.obj"。
	find_last_of('/') 找到路径中最后一个 '/' 的位置（即文件名和目录的分界点）。
	substr(0, ...) 从开头截取到最后一个 '/' 之前的所有内容，也就是目录路径。
	结果：directory 变成 "res/model/backpack"。
	*/
	// 递归处理节点
	processNode(scene->mRootNode, scene);
}

void PbrModel::processNode(aiNode* node, const aiScene* scene)
{
	// 处理节点所有网格
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	// 递归处理子节点
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

PbrMesh PbrModel::processMesh(aiMesh* mesh, const aiScene* scene)
{
	// 数据
	vector<Vertex>       vertices;
	vector<unsigned int> indices;
	vector<Texture>      textures;

	// 遍历所有顶点
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vector;	// 临时变量
		// 位置
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		// 法线
		if (mesh->HasNormals())
		{
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
		}
		// 纹理坐标
		if (mesh->mTextureCoords[0])
		{
			// 一个顶点最多可包含 8 个不同的纹理坐标。
			// 因此，我们做出如下假设：我们不会使用顶点可能包含多个纹理坐标的模型，所以始终采用第一组纹理坐标（序号 0）
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
			// 切线
			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.Tangent = vector;
			// 副切线
			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			vertex.Bitangent = vector;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}

	// 遍历所有面
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	// 处理材质
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	// ---- 新增：挖出该材质名下所有通道的贴图 ----
	for (int tt = 1; tt <= 27; tt++) {
		aiTextureType ty = (aiTextureType)tt;
		unsigned int cnt = material->GetTextureCount(ty);
		if (cnt > 0) {
			for (unsigned int i = 0; i < cnt; i++) {
				aiString s; material->GetTexture(ty, i, &s);
				std::cout << "    [type " << tt << "]  <- " << s.C_Str() << std::endl;
			}
		}
	}
	aiString matName; material->Get(AI_MATKEY_NAME, matName);
	std::cout << "=== 材质总数: " << scene->mNumMaterials << " ===" << std::endl;
	std::cout << "mesh[" << meshes.size() << "] -> 材质索引 " << mesh->mMaterialIndex
		<< " / 名 " << matName.C_Str() << std::endl;
	// 我们对着色器中的采样器名称采用一套约定。每个漫反射纹理都应命名为
	// “texture_diffuseN”，其中 N 是从 1 到 MAX_SAMPLER_NUMBER（最大采样器数量）的连续序号。
	// 其他类型的纹理也遵循相同规则，具体如下列表所示：
	// 漫反射纹理：texture_diffuseN
	// 高光纹理：texture_specularN
	// 法线纹理：texture_normalN

	// ==== PBR 材质贴图加载 ====
	// 1. 固有色 albedo
	vector<Texture> albedoMaps = loadMaterialTextures(material, aiTextureType_BASE_COLOR, "albedo", scene);
	if (albedoMaps.empty())
		albedoMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "albedo", scene);
	textures.insert(textures.end(), albedoMaps.begin(), albedoMaps.end());

	// 2. 法线 normal
	vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "normal", scene);
	if (normalMaps.empty())
		normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "normal", scene);   // 旧 FBX 常把法线塞 HEIGHT
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

	//// 3. 金属度 metalness
	//vector<Texture> metalnessMaps = loadMaterialTextures(material, aiTextureType_METALNESS, "metalness", scene);
	//textures.insert(textures.end(), metalnessMaps.begin(), metalnessMaps.end());

	//// 4. 粗糙度 roughness
	//vector<Texture> roughnessMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, "roughness", scene);
	//textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

	// 3+4. metallic + roughness（glTF 合并成一张 ORM 图：G=roughness B=metalness）
	vector<Texture> mrMaps = loadMaterialTextures(material, aiTextureType_GLTF_METALLIC_ROUGHNESS, "metalness", scene);
	if (mrMaps.empty())
	{
		vector<Texture> metalnessMaps = loadMaterialTextures(material, aiTextureType_METALNESS, "metalness", scene);
		vector<Texture> roughnessMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, "roughness", scene);
		textures.insert(textures.end(), metalnessMaps.begin(), metalnessMaps.end());
		textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
	}
	else
	{
		// 同一张 ORM 图，同时当 metallic（读 .b）和 roughness（读 .g）用
		Texture metal = mrMaps[0]; metal.type = "metalness1";
		Texture rough = mrMaps[0]; rough.type = "roughness1";
		textures.push_back(metal);
		textures.push_back(rough);
	}

	// 5. 自发光 emission
	vector<Texture> emissionMaps = loadMaterialTextures(material, aiTextureType_EMISSION_COLOR, "emission", scene);
	if (emissionMaps.empty())
		emissionMaps = loadMaterialTextures(material, aiTextureType_EMISSIVE, "emission", scene);
	textures.insert(textures.end(), emissionMaps.begin(), emissionMaps.end());

	// 6. 环境遮蔽 ao（这个模型没有，但留着通用）
	vector<Texture> aoMaps = loadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION, "ao", scene);
	textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());


	//// 新增：按材质名拼文件名加载（保留旧 loader，这里补充调用）
	//vector<Texture> pbrMaps = loadPbrByName(matName.C_Str());
	//textures.insert(textures.end(), pbrMaps.begin(), pbrMaps.end());

	return PbrMesh(vertices, indices, textures);
}

vector<Texture> PbrModel::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName, const aiScene* scene)
{
	vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		const char* p = str.C_Str();
		std::cout << "    [" << typeName << "]  <-  " << p << std::endl;

		string dedupPath = p;
		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++)
		{
			if (textures_loaded[j].path == dedupPath)
			{
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{
			Texture texture;
			if (p[0] == '*')   // GLB 内嵌贴图，*N 指向 scene->mTextures[N]
			{
				int idx = std::stoi(p + 1);
				if (scene && idx >= 0 && idx < (int)scene->mNumTextures)
				{
					const aiTexture* aitex = scene->mTextures[idx];
					if (aitex->mHeight == 0)   // 压缩数据(jpg/png)，长度在 mWidth
						texture.id = TextureFromMemory((unsigned char*)aitex->pcData, aitex->mWidth, false);
				}
			}
			else
				texture.id = TextureFromFilePBR(p, directory);

			texture.type = typeName + std::to_string(i + 1);
			texture.path = dedupPath;
			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}
	}
	return textures;
}

//vector<Texture> PbrModel::loadPbrByName(const string& matBase)
//{
//	vector<Texture> result;
//	static const pair<const char*, const char*> channels[] = {
//		{ "albedo",    "albedo1"    },
//		{ "Normal",    "normal1"    },
//		{ "metalness", "metalness1" },
//		{ "roughness", "roughness1" },
//		{ "emission",  "emission1"  },
//	};
//	for (auto& ch : channels)
//	{
//		string fname = matBase + "_" + ch.first + ".jpg";
//		string fpath = directory + "/" + fname;
//		ifstream f(fpath.c_str());       // B / GLASS 没文件，直接跳过
//		if (!f.good()) continue;
//		f.close();
//
//		Texture tex;
//		bool skip = false;
//		for (auto& st : textures_loaded)
//			if (st.path == fpath) { tex = st; skip = true; break; }   // 去重
//		if (!skip)
//		{
//			tex.id = TextureFromFilePBR(fname.c_str(), directory);
//			tex.type = ch.second;
//			tex.path = fpath;
//			textures_loaded.push_back(tex);
//		}
//		result.push_back(tex);
//	}
//	return result;
//}

// 从文件加载纹理
unsigned int TextureFromFilePBR(const char* path, const string& directory, bool gamma)
{
	string filename = string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}

	return textureID;
}

unsigned int TextureFromMemory(const unsigned char* data, int len, bool gamma)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* image = stbi_load_from_memory(data, len, &width, &height, &nrComponents, 0);
	if (image)
	{
		GLenum format;
		if (nrComponents == 1)      format = GL_RED;
		else if (nrComponents == 3) format = GL_RGB;
		else if (nrComponents == 4) format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(image);
	}
	else
		std::cout << "TextureFromMemory FAILED" << std::endl;

	return textureID;
}