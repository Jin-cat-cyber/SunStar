# 升级到「DeepSeek Harness 桌面端（官方）」· 操作手册

> **为什么有这份文件**：升级过程中有一段"盲区"——DSH 退出之后助手不可用，而安装又必须在那之后做。
> 所以这份手册是**离线可读**的：用 VS Code 或记事本打开本文件即可，不需要 DSH。
>
> **快照时间**：2026-10-02（所有数值与命令均为本轮实测/官方原文，不是估的）
> **执行原则**：**先备份、后安装**；**不确定就停下来看第 5 节**。

---

## 0. 一句话结论

你现在跑的是**官方 CLI/Web**（`@deepseek-ai/dsh` 0.1.2-rc.1，`dsh web`，端口 3080）。
要装的**官方桌面端**与它**共享同一个 `DSH_HOME`**（`C:\Users\35718\.dsh`）——所以**装完就能看到你现有的 19 个会话，不需要导入或迁移**。
也正因为共享，**备份必须在安装之前做**：把 `C:\Users\35718\.dsh` 复制一份到桌面（**要备份的只有 179 个文件 / 约 157 MB**）。

---

## 1. 官方桌面端：事实与原文

**来源**（只从这里下，别用第三方站）：

- 官方 Harness 页：<https://www.deepseek.com/en/harness/>
- 官方下载页：<https://www.deepseek.com/en/download/>
- Windows 安装包直链：`https://download.deepseek.com/desktop/dsh-latest-windows-x64.exe`
- 官方桌面端文档（本节的引用都出自这里）：
  <https://github.com/deepseek-ai/deepseek-harness/blob/master/apps/desktop/README.zh.md>

**六条与你直接相关的官方原文**：

| # | 官方原话 | 对你的意义 |
|---|---|---|
| 1 | 「桌面壳与 CLI dsh **共享 `$DSH_HOME` 下的会话、设置、凭据、工作区和存储**，但可执行包、插件激活和锁文件彼此隔离。」（已知限制） | **装完就能看到你的 19 个会话**，不用迁移 |
| 2 | 「CLI 与 Desktop 共享 `$DSH_HOME` 下受支持的产品数据，但绝不共享可执行包、插件激活、锁文件或 `node_modules`。」（状态归属） | 它**不会**覆盖你现在的可执行环境；只共用数据 |
| 3 | 「Desktop 默认使用端口 **`19387`**，与 Web 的 `3080` 分开。」 | **端口不冲突**，不用先腾地方 |
| 4 | 「Electron 在访问任何 profile 前获取**进程生命周期单实例锁**，并独占 `$DSH_HOME/profiles/desktop` 及其包管理器状态。」 | 它独占的是新的 `desktop` profile；你现在用的是 `web` profile，两者不抢 |
| 5 | 「**更新或卸载 Desktop 前请结束 CLI 命令。**」「卸载 Desktop 前，请通过**管理 dsh 命令… → 移除**删除其 CLI 注册；应用卸载程序不会移除该注册。」 | 将来升级/卸载时照着做 |
| 6 | 「`load_workspace_dependencies` 工具首次使用时，将该产物离线安装到 `$DSH_HOME/dsh-runtimes/dsh-primary-runtime`（通常为 `~/.dsh/dsh-runtimes/dsh-primary-runtime`）」 | 它会往 `DSH_HOME` 里**新增**目录，不是覆盖你现有的东西 |

**外加一条上游警告**（来自 [根 README](https://github.com/deepseek-ai/deepseek-harness/blob/master/README.zh.md)）：

> DeepSeek Harness 处于*开发者预览*阶段，正在快速迭代。**未来将出现破坏兼容性的变更。**

会话格式是**单向迁移**（v0 → v1 → v2 → v3，没有反向迁移器）。**这就是备份的全部理由。**

---

## 2. ⚠️ 两个必须先知道的坑

### 2.1 `.dsh\profiles\node_modules` 是**符号链接场**，不能让它跟进复制

它由 pnpm 生成：一层包名链接指向 `.pnpm\<pkg>@<ver>\node_modules\<pkg>`。
`cmd /c "dir /s /b"` 展开有 **56,354 个条目**；而 PowerShell 的 `Get-ChildItem -Recurse` **不跟进链接**，只数得到 4 个文件。

**后果**：`robocopy /E` 默认**跟进链接、把目标展开复制**，于是约 197 MB 的依赖树被摊成几万文件的真目录，备份体积暴涨、速度极慢，而且**字母序在后、真正要保的 `sessions`（116 MB）会被拖到最后才轮上**。

**对策**：备份命令一律加 **`/XJ`**（不跟进符号链接与交叉点）和 **`/XD node_modules`**（直接跳过这棵无用依赖树）。
已用 `robocopy /?` 核实：`/XJ :: eXclude symbolic links (for both files and directories) and Junction points.`

> `profiles\node_modules` 是纯依赖安装结果（重装即再生），**丢了不影响会话、设置、key**。
> 真正要保的是：`sessions\`、`storages\`、`settings.yaml`、`.credentials.yaml`。

### 2.2 备份目录里不要放别的东西

第 5 步的校验①是**逐文件数与总字节数比对**。往 `dsh-backup` 里塞任何额外文件都会让它对不上。
所以基线记录之类的文件一律放在桌面根目录（`dsh-baseline.txt`），**不要放进 `dsh-backup`**。

---

## 3. 现状快照（2026-10-02 实测）

| 项目 | 值 |
|---|---|
| 数据目录（`DSH_HOME`） | `C:\Users\35718\.dsh` |
| 当前会话 ID | `session-228b6453-ab14-487d-8e09-ee48520a3907` |
| 当前会话日志 | `.dsh\sessions\--C-Users-35718-Desktop-OpenGL_Learning--\session-228b6453-…\session.jsonl.zstd` |
| 现在跑的版本 | `@deepseek-ai/dsh` **0.1.2-rc.1**（npm 全局），Web UI 在 `http://127.0.0.1:3080` |
| C 盘可用 | **263.5 GB**（备份只需约 0.16 GB，充裕） |
| 桌面上原有的备份 | 无（`dsh-backup` 不撞名） |

`.dsh` 内部构成（**合计 179 个文件 / 156.8 MB**，已排除 `node_modules` 链接场）：

| 子目录 | 文件数 | 大小 | 是什么 |
|---|---|---|---|
| `sessions\` | 22 | 116.7 MB | **对话本体**（最重要）。4 个工作区目录；本工作区 19 个会话 |
| `attachments\` | 127 | 38.4 MB | 附件（图片等） |
| `storages\` | 21 | 1.6 MB | 会话状态 JSON（本会话的 `session-228b6453-….json`，约 218 KB） |
| `profiles\web\` | 4 | 极小 | profile 配置（`cordis.yml`、`package.json`、`pnpm-workspace.yaml`） |
| `profiles\node_modules\` | 56,354 条目 | ~197 MB | **pnpm 链接场 —— 排除，别备份** |
| `llm-deepseek\` | 1 | 极小 | 模型侧缓存 |
| 根目录散文件 | 4 | 695 B | `.anonymous-user-id`、`.credentials.yaml`（**API key**）、`settings.yaml`（默认模型 `deepseek-v4-flash-vision-exp` / `reasoningEffort: high`）、`settings.yaml.bak` |

---

## 4. 五条风险

**风险 1 · 桌面端读的就是这份数据（已由官方确认）。**
好处是会话自动就在；代价是**它会就地使用、并可能升级你现有的会话格式**。备份是唯一保险。

**风险 2 · 会话格式单向迁移。** 见第 1 节末尾。所以备份必须在**安装之前**做，且存的是**原始文件**。

**风险 3 · 新旧两版别同时对着同一个会话干活。** 数据是共享的，两个进程同时写同一个会话文件会打架。桌面端用 `19387`、Web 用 `3080`，端口不冲突，但**别同时用**。

**风险 4 · 别让 robocopy 跟进符号链接。** 见第 2.1 节。所有备份命令都带 `/XJ /XD node_modules`。

**风险 5 · 官方桌面端没有"升级前备份"教程。** 官方文档只写了更新流程、恢复与卸载；备份得自己做，也就是这份手册。

---

## 5. 操作流程

> **关于命令的形态**：代码框里通常是**两行**——第一行设置变量 `$dst`，第二行才干活。
> 两行**要一起粘**（或先粘第一行回车、再粘第二行）：它们必须在**同一个 PowerShell 窗口**里才连贯。
> 为了不怕窗口换掉，每段命令都**自带** `$dst = ...` 那一行，重复执行也无害。

### 本手册执行进度（2026-10-02 17:36 更新）

| 步骤 | 内容 | 状态 |
|---|---|---|
| 1 | 清理跑歪的旧副本 | ✅ 已完成 |
| 2 | 热备 | ✅ 已完成（179 文件 / 157.39 MB，失败 0） |
| 3 | 项目存档（git） | ✅ 已完成（提交 `0e22a58`，60 个文件） |
| 4 | 记"安装前基线" | ✅ 已完成（179 / 157.6 / 19，文件在 `桌面\dsh-baseline.txt`） |
| 5 | 退 DSH → 冷备补齐 → 四项校验 | ✅ 已完成（四项全过：179 = 179 文件 / 157.7 MB） |
| 6 | 安装官方桌面端 | ✅ 已完成（`0.2.0-rc.2`，数字签名有效） |
| 7 | 装后核验 | ✅ 已完成（见下） |

**装后实测结论（2026-10-02 17:47）**

| 项 | 结果 |
|---|---|
| `DSH_HOME` | `C:\Users\35718\.dsh` —— **没变** |
| `DSH_SESSION_ID` | `session-228b6453-ab14-487d-8e09-ee48520a3907` —— **和升级前同一个会话**，是接着原会话跑的 |
| `DSH_WEB_URL` | `http://127.0.0.1:19387` —— 桌面端端口（Web 是 3080） |
| 新增 | `profiles\desktop\`（4 个文件）、1 个新会话；文件数 179 → 187 |
| `settings.yaml` | 被一次性导入后改名为 `settings.yaml.imported`；**内容原样进了 `profiles\desktop\cordis.patch.yml`**（`model: deepseek-flash`、`reasoningEffort: high`、`ui-theme: system` 都在）。模型名变更是因为旧名 `deepseek-v4-flash-vision-exp` 已下线、统一路由到 `deepseek-flash`（V4.1-Flash，原生 text+image） |
| 老会话 | 19 个全在 ✅ · `storages` / `attachments` / `.credentials.yaml` 都在 ✅ |

> **`dsh-backup` 先别删**，等桌面端连用几天、历史会话都能打开之后再处理（见 §8）。

> **安装包已经下好了，不用再下**：`C:\Users\35718\Downloads\dsh-latest-windows-x64.exe`
> 实测版本 **0.2.0-rc.2**、产品名 `DeepSeek Harness`、**数字签名有效**（签名主体 `Hangzhou DeepSeek Artificial Intelligence Co., Ltd.`）。

> 步骤 1–3 的代码块**留档备查，不用再跑**。重跑第 2、3 步都无害；
> ⚠️ 但**步骤 1 会把已经做好的备份整个删掉**，千万别手滑。
> **DSH 退出后就看这张表**：从步骤 4 或 5 往下做。

---

### 步骤 1 · 清理上次跑歪的副本（✅ 已完成，留档）

```powershell
# 先确认没有 robocopy 在跑
Get-Process robocopy -ErrorAction SilentlyContinue | Stop-Process -Force

# 删掉上一次跑歪的副本（只动备份目录，绝不碰 .dsh）
Remove-Item "$env:USERPROFILE\Desktop\dsh-backup" -Recurse -Force -ErrorAction SilentlyContinue
Test-Path "$env:USERPROFILE\Desktop\dsh-backup"   # 期望 False
```

- [ ] 备份目录已清空

---

### 步骤 2 · 热备（DSH 不用退，现在就能做）

```powershell
$dst = "$env:USERPROFILE\Desktop\dsh-backup"
robocopy "$env:USERPROFILE\.dsh" $dst /E /R:1 /W:1 /NP /NFL /NDL /XJ /XD node_modules
```

参数表：

| 参数 | 作用 |
|---|---|
| `/E` | 含所有子目录（包括空目录） |
| `/R:1 /W:1` | 失败只重试 1 次、等 1 秒（默认重试 100 万次，卡住时很难看） |
| `/NP` | 不显示每文件百分比 |
| `/NFL /NDL` | **不列文件名、不列目录名**——这就是"刷屏一大堆东西"的来源 |
| **`/XJ`** | **不跟进符号链接与交叉点**（第 2.1 节的坑） |
| **`/XD node_modules`** | **跳过 pnpm 链接场**（约 197 MB 无用依赖树） |

- **目录名故意不带时间戳**：第 5 步要往**同一个目录**再补一次，robocopy 会自动只补差异。
- 这是**热备**：正在写的 `session.jsonl.zstd` 尾部可能不完整，**不要紧**，第 5 步会补齐。
- `robocopy` 的**结束码 1 是成功**（表示有文件被复制），不是错误。含义见第 6.1 节。
- 安静模式下屏幕上只有开头一行与结尾一张汇总表；**一两分钟正常**（`sessions` 有 116 MB）。

- [ ] 热备已执行

---

### 步骤 3 · 项目存档（✅ 已完成于 2026-10-02，提交 `0e22a58`，留档）

> **这一段不用再跑。** 已提交 60 个文件，工作区干净（`git status` 无输出）。
> 若以后还想再做一次存档点，直接跑下面的命令即可；工作区干净时它会回你 "nothing to commit"。

工作区那份才是真正的"记忆"，自 2026-09-23 最后一次提交以来攒了一大批未提交内容。
`.gitignore` 已排除 `x64/`、`*.exe`、`.vs/`、模型与 HDRI，`git add -A` 是安全的（未跟踪内容总共才 1 MB 出头）。

```powershell
cd C:\Users\35718\Desktop\OpenGL_Learning
git add -A
git commit -m "SC2 折跃：光柱 + 抵达滑入节奏 + 文档（24.A）"
git log -1 --stat | Select-Object -First 25
```

- `git status` 里的 `D OpenGL_Learning/src/SpaceShip16.0.cpp` 等几行，是**被移进 `src/SpaceShip/` 的文件**，`git add -A` 会记成移动，正确，不用管。
- 本手册也会一起进这次提交——这正是想要的。

- [ ] 项目已提交

---

### 步骤 4 · 记下"安装前基线"（装在 DSH 里加了什么，事后一眼可查）

```powershell
$h = "$env:USERPROFILE\.dsh"; $out = "$env:USERPROFILE\Desktop\dsh-baseline.txt"
"时间   : $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" | Set-Content $out
$a = Get-ChildItem -Recurse -File $h -EA SilentlyContinue | Measure-Object Length -Sum
"文件数 : $($a.Count)" | Add-Content $out
"总 MB  : {0:N1}" -f ($a.Sum/1MB) | Add-Content $out
"--- 顶层条目 ---" | Add-Content $out
Get-ChildItem -Force $h | Select-Object -ExpandProperty Name | Add-Content $out
"--- 本工作区会话数 ---" | Add-Content $out
(Get-ChildItem -Directory "$h\sessions\--C-Users-35718-Desktop-OpenGL_Learning--").Count | Add-Content $out
Get-Content $out
```

期望：**文件数 179**、**总 MB 156.8**、会话数 **19**。

⚠️ 这个文件放在**桌面根目录**，不要放进 `dsh-backup`（见第 2.2 节）。

- [ ] 基线已记录（179 / 156.8 / 19）

---

### 步骤 5 · 完全退出 DSH，然后补齐冷备并校验

⚠️ **执行完这一步的"退出"，助手就联系不上了。** 先把本手册留在屏幕上。

**① 退出 DSH**——关浏览器页面**不算**：

1. 找到启动 DSH 的那个终端窗口（跑 `dsh web` 的那个），`Ctrl+C`，或直接关掉该窗口；
2. 有托盘图标就右键退出；
3. 确认干净：

```powershell
Get-Process node -ErrorAction SilentlyContinue | Select-Object Id,StartTime
```

对照这两条实测快照（DSH 至少是其中之一，**别按名字杀 `node.exe`**，VS Code 和 Trae 也在跑 node）：

| PID | 内存 | 启动时间 |
|---|---|---|
| 23380 | 约 1.7 GB | 16:52:04 |
| 24792 | 约 58 MB | 17:12:45 |

**② 补齐冷备**（同一个 `$dst`，源文件此时已静止）：

```powershell
$dst = "$env:USERPROFILE\Desktop\dsh-backup"
robocopy "$env:USERPROFILE\.dsh" $dst /E /R:1 /W:1 /NP /NFL /NDL /XJ /XD node_modules
```

**③ 四项校验**（必须全过才继续）：

```powershell
$src = "$env:USERPROFILE\.dsh"
$dst = "$env:USERPROFILE\Desktop\dsh-backup"

# ① 文件数与总字节数一致（两边同口径：都不跟进符号链接）
$a = Get-ChildItem -Recurse -File $src -EA SilentlyContinue | Measure-Object Length -Sum
$b = Get-ChildItem -Recurse -File $dst -EA SilentlyContinue | Measure-Object Length -Sum
"源   : {0,5} 文件 / {1,8:N1} MB" -f $a.Count, ($a.Sum/1MB)
"备份 : {0,5} 文件 / {1,8:N1} MB" -f $b.Count, ($b.Sum/1MB)
if ($a.Count -eq $b.Count -and $a.Sum -eq $b.Sum) { "① 通过" } else { "① 不一致 —— 重跑一次 robocopy；仍不一致就查第 6.3 节" }

# ② 本次会话在备份里
$sess = "$dst\sessions\--C-Users-35718-Desktop-OpenGL_Learning--\session-228b6453-ab14-487d-8e09-ee48520a3907"
if (Test-Path $sess) { "② 通过：本次会话已在备份里" } else { "② 失败：备份里没有本次会话！" }

# ③ 设置与凭据在备份里
foreach ($n in @('settings.yaml','.credentials.yaml')) {
  if (Test-Path (Join-Path $dst $n)) { "③ 通过：$n" } else { "③ 失败：缺 $n" }
}

# ④ 没把 pnpm 链接场搬进去
if (Test-Path "$dst\profiles\node_modules") { "④ 警告：出现了 node_modules，说明漏了 /XD node_modules" } else { "④ 通过：没有 node_modules 链接场" }
```

期望：**两边都是 179 个文件 / 约 156.8 MB**。

- [ ] DSH 已完全退出
- [ ] 冷备已补齐
- [ ] 校验①通过 · 校验②通过 · 校验③通过 · 校验④通过

---

### 步骤 6 · 安装官方桌面端

0. **建议在退出 DSH 之前就把安装包下好**（浏览器和 DSH 无关）：这样万一下载出问题，还有人可问；退出 DSH 之后就只剩这份手册了。
1. 打开官方下载页 <https://www.deepseek.com/en/download/>（或直接下 `https://download.deepseek.com/desktop/dsh-latest-windows-x64.exe`）；
2. 运行 NSIS 安装程序，按提示完成；
3. ⚠️ **先不要卸载旧 CLI**（npm 全局的 `0.1.2-rc.1`）——它是你的退路；
4. ⚠️ 装好后**不要同时用两个**对着同一个会话干活（第 4 节风险 3）；
5. 若桌面端提供「**管理 dsh 命令…**」并问你要不要注册 `dsh` 命令：**建议先跳过**。将来真要卸载桌面端时，若注册过，必须先在那里"移除"，否则安装器不会替你清。见第 1 节引用 5。

- [ ] 桌面端已安装（旧 CLI 保留未动）

---

### 步骤 7 · 装后核验

**① 会话在不在。** 打开桌面端，看会话列表：本工作区应有 **19 个**会话，其中就有 `session-228b6453…`（升级前这次对话）。

**② `DSH_HOME` 里多了什么。** 对照第 4 步的基线：

```powershell
$h = "$env:USERPROFILE\.dsh"
$a = Get-ChildItem -Recurse -File $h -EA SilentlyContinue | Measure-Object Length -Sum
"现在: {0} 文件 / {1:N1} MB" -f $a.Count, ($a.Sum/1MB)
"现在顶层:"; Get-ChildItem -Force $h | Select-Object -ExpandProperty Name
"基线:"; Get-Content "$env:USERPROFILE\Desktop\dsh-baseline.txt"
```

预期只**多出**：`profiles\desktop\`（新的 desktop profile）、`dsh-runtimes\`（内置 Python/Node/pnpm 运行时，首次用到才装）、可能还有 `logs\`。**原有的 `sessions` / `storages` / `settings.yaml` / `.credentials.yaml` 不应减少。**

**③ 设置与 key。** 确认默认模型仍是 `deepseek-v4-flash-vision-exp`、`reasoningEffort: high`，能正常发消息。

- [ ] 会话列表能看到 `session-228b6453…`
- [ ] `sessions` / `storages` 文件数**没有减少**
- [ ] 默认模型 / reasoningEffort 正确，能发消息

---

## 6. 失败处理与回滚

### 6.1 `robocopy` 退出码

| 码 | 含义 | 怎么办 |
|---|---|---|
| 0 | 没有文件需要复制（已是最新） | 正常 |
| **1** | **有文件被成功复制** | **正常，别当成报错** |
| 2 / 3 | 有额外文件或目录 | 正常 |
| 4~7 | 有不匹配 / 较新的文件被跳过 | 一般无害，看输出里是哪几个 |
| **8 及以上** | **至少一个文件复制失败** | 看输出里的文件名，重跑；仍失败则查权限/占用 |
| 16 | 严重错误，一个都没复制（通常是参数写错或没权限） | 检查命令有没有粘全 |

热备那一次若报告某个 `session.jsonl.zstd` 复制失败，是**正常的**（文件正被写）。步骤 5 会补上。

### 6.2 撤销桌面端但**保留数据**（大概率够用）

桌面端主要是**新增**（`profiles\desktop\`、`dsh-runtimes\`），通常不动你的 `sessions`。
如果只是想暂时不用它、回到旧 CLI：

1. 完全退出桌面端；
2. 打开旧 CLI：`npx @deepseek-ai/dsh web`（或你原来的启动方式），端口仍是 3080；
3. 若确认桌面端没改过你的会话，`profiles\desktop` 与 `dsh-runtimes` 可以留着不管（不动它们就不会被使用）。

**只有当你发现旧 CLI 读不出会话（= 格式被就地迁移了）时，才需要整份回滚**，见 6.4。

### 6.3 备份跑歪了

**症状**：备份目录里 `profiles` 异常庞大（几万文件、几百 MB），而 `sessions` 迟迟不出现。
**原因**：漏了 `/XJ /XD node_modules`，robocopy 跟进了 pnpm 链接场。

```powershell
Get-Process robocopy -ErrorAction SilentlyContinue | Stop-Process -Force
Remove-Item "$env:USERPROFILE\Desktop\dsh-backup" -Recurse -Force -ErrorAction SilentlyContinue
$dst = "$env:USERPROFILE\Desktop\dsh-backup"
robocopy "$env:USERPROFILE\.dsh" $dst /E /R:1 /W:1 /NP /NFL /NDL /XJ /XD node_modules
```

### 6.4 整份回滚（格式被迁移时的最后手段）

**先完全退出桌面端**，再执行：

```powershell
robocopy "$env:USERPROFILE\Desktop\dsh-backup" "$env:USERPROFILE\.dsh" /E /R:1 /W:1 /NP
```

这条用了 `/E` 但**不带 `/PURGE`**：**只覆盖、不删除**目标里多出来的东西，是安全的默认行为。
（这一步**故意不加** `/XJ /XD node_modules`：备份里本来就没有它，加了也无害。）

想先留个"翻车现场"再回滚：

```powershell
Rename-Item "$env:USERPROFILE\.dsh" ".dsh.broken-$(Get-Date -Format yyyyMMdd-HHmm)"
robocopy "$env:USERPROFILE\Desktop\dsh-backup" "$env:USERPROFILE\.dsh" /E /R:1 /W:1 /NP
```

### 6.5 按症状对照

| 症状 | 判断 | 处理 |
|---|---|---|
| 桌面端打开后会话列表为空 | 读错了数据目录（**数据没丢**） | 查 `DSH_HOME` 是否被改写；确认它读的是 `C:\Users\35718\.dsh` |
| 提示要迁移/升级数据格式 | 正常，但不可逆 | 确认第 5 步四项校验已通过，再答应 |
| 桌面端起不来 | — | 完全退出后重开；仍不行就走 6.2 回旧 CLI |
| 旧 CLI 读不出会话 | 格式被就地迁移 | 走 6.4 整份回滚 |
| 窗口"消失了" | 官方行为：关窗只是隐藏 | 从托盘/开始菜单重新打开 |
| 模型报错 | key 或默认模型没带过去 | 确认 `.credentials.yaml` 与 `settings.yaml` 仍在 |

---

## 7. 收尾

- 备份目录 `C:\Users\35718\Desktop\dsh-backup` **先别删**，等桌面端正常用几天、历史会话都能打开之后再删。
- 想更保险，把备份挪到别的盘或云盘（约 157 MB）。
- 旧 CLI 确认用不上之后再卸。
- 那次 `git commit` 之后就有一个"升级前"的项目存档点，随时可以 `git log` 找回。

---

## 8. 后续：将来卸掉老 CLI 的检查清单

> **当前决定（2026-10-02）：老 CLI 先留着、不卸、也不启动。**

**前提**：桌面端连续用几天、确认没问题，再动这一步。
**在此之前绝对别启动老 CLI** —— 它是 `0.1.2-rc.1`，比桌面端自带的那套旧，而会话格式是**单向迁移**的；新版已经动过数据（`settings.yaml` 就被改名了），旧版再去读可能读不了或读出问题。

### 8.1 先确认两件事

1. **桌面端有没有用过「管理 dsh 命令…」？**
   用过的话，**卸载桌面端之前必须先在那里点「移除」**，否则安装器不会替你清（官方文档明说）。
2. **以后还需要 `dsh web`（端口 3080）吗？** 不需要了才卸；需要就留着。

### 8.2 卸载命令

⚠️ 这台机器的 PowerShell 执行策略**禁止运行 `npm.ps1`**（会报 "cannot be loaded because running scripts is disabled"），所以必须显式用 **`npm.cmd`**：

```powershell
npm.cmd ls -g --depth=0                      # 先看一眼
npm.cmd uninstall -g @deepseek-ai/dsh
```

### 8.3 卸载后验证（三点）

```powershell
# ① PATH 里的老 dsh 应该消失（或已换成桌面端那份）
Get-Command dsh -All -ErrorAction SilentlyContinue | Select-Object CommandType,Source

# ② 全局包目录里不该再有它
Test-Path "$env:APPDATA\npm\node_modules\@deepseek-ai\dsh"

# ③ .dsh 必须一点没少（对比卸载前后的数字）
$h = "$env:USERPROFILE\.dsh"
$a = Get-ChildItem -Recurse -File $h -EA SilentlyContinue | Measure-Object Length -Sum
"文件数 : $($a.Count)"
"总 MB  : {0:N1}" -f ($a.Sum/1MB)
"本工作区会话数: $((Get-ChildItem -Directory "$h\sessions\--C-Users-35718-Desktop-OpenGL_Learning--").Count)"
```

判据：①② 如预期；③ 的**文件数与大小不该减少**（桌面端正在跑，小幅增长是正常的）。

### 8.4 可选：回收约 197 MB（不建议急着做）

老 CLI 的 `web` profile 依赖树（`profiles\web\node_modules` 与 `profiles\node_modules` 里的 pnpm 链接场）**只服务于 `web` profile**；桌面端用的是 `profiles\desktop`。卸掉老 CLI 后它们就是死重量。

但**「删了以后重装即再生」是它唯一的卖点**，保守做法是不动。真要清，先确认：
- 不再需要 `dsh web`；
- 桌面端能正常启动（它的 `desktop` profile 不依赖这两处）。

### 8.5 别删的东西

- `C:\Users\35718\.dsh` 整体 —— 会话、设置、凭据都在这
- `C:\Users\35718\Desktop\dsh-backup` —— 升级前快照
- `C:\Users\35718\Desktop\dsh-baseline.txt` —— 对照用，留着无害

---

## 附录 A · 路径速查

| 用途 | 路径 |
|---|---|
| DSH 数据根（**桌面端与 CLI 共享**） | `C:\Users\35718\.dsh` |
| 对话本体 | `.dsh\sessions\` |
| 会话状态 | `.dsh\storages\` |
| 设置 | `.dsh\settings.yaml` |
| API key | `.dsh\.credentials.yaml` |
| pnpm 链接场（**不要备份**） | `.dsh\profiles\node_modules` |
| 桌面端会新增的 | `.dsh\profiles\desktop\`、`.dsh\dsh-runtimes\` |
| 备份目标 | `C:\Users\35718\Desktop\dsh-backup` |
| 安装前基线记录 | `C:\Users\35718\Desktop\dsh-baseline.txt` |
| 旧 CLI | `C:\Users\35718\AppData\Roaming\npm\node_modules\@deepseek-ai\dsh\` |
| 项目记忆（比对话更重要） | `C:\Users\35718\Desktop\OpenGL_Learning\AGENTS.md`、`OpenGL_Learning\Tech_finding_Report\` |

## 附录 B · 官方材料索引

| 资料 | 链接 |
|---|---|
| 官方 Harness 页 | <https://www.deepseek.com/en/harness/> |
| 官方下载页 | <https://www.deepseek.com/en/download/> |
| Windows 安装包直链 | <https://download.deepseek.com/desktop/dsh-latest-windows-x64.exe> |
| 官方桌面端文档（共享 DSH_HOME、端口、更新、卸载） | <https://github.com/deepseek-ai/deepseek-harness/blob/master/apps/desktop/README.zh.md> |
| 上游根 README（预览阶段警告） | <https://github.com/deepseek-ai/deepseek-harness/blob/master/README.zh.md> |
| CLI 行为参考（`DSH_HOME`、profile、`dsh web`） | <https://github.com/deepseek-ai/deepseek-harness/blob/master/apps/cli/reference/README.zh.md> |
| Web UI 用户指南 | <https://github.com/deepseek-ai/deepseek-harness/blob/master/docs/user/guide/index.zh.md> |

> **注意**：GitHub 上还有一个同名的**社区**项目（`anywhere-labs/deepseek-harness-desktop`，站点 dshdesktop.cn），
> 它不是官方产品（其 FAQ 自述"不隶属于 DeepSeek，也未获得官方背书"）。本手册针对**官方**桌面端。

## 附录 C · 即使会话读不出来，也不会丢的东西

升级前的工作状态已经全部落在项目文件里，会话丢失不影响接续：

- `AGENTS.md` §1 版本约定、§6 SC 分支当前状态（时间线、两波模型、四个方向、两段行程、外壳、光柱、`dirY` 实测映射、"骨架滞留"与"抵达段两条独立的轴"）
- `OpenGL_Learning\Tech_finding_Report\星际争霸2折跃_光柱（纵向发光丝）_设计与实现计划书.md`（含 §7.2 抵达时间线、§7.3 与 SC2 录屏的逐项对照）
- `OpenGL_Learning\Tech_finding_Report\星际争霸2折跃_调参操作手册.md`（每个数改哪里、连带什么）
- `OpenGL_Learning\Tech_finding_Report\星际争霸2折跃_逐帧观察.md`（SC2 原版事实依据）

新会话里只要打开这个工作区，`AGENTS.md` 会自动加载，接着干即可。
