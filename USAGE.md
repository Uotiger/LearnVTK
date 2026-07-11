# LearnVTK 开发环境使用手册

> 写给未来的自己——忘记命令时看一眼。

## 一、跨对话恢复

每次启动新对话，说这句话即可恢复全部上下文：

```
继续 LearnVTK 学习
```

Claude 会自动加载你的学习进度、技能水平、五阶段路线和所有设计决策。你也可以指定具体目标：

```
继续 LearnVTK，帮我 Review Ch1 代码
继续 LearnVTK，开始 Ch2 的学习
```

**原理**：记忆存储在 `~/.claude/projects/e--Dev-Personal-LearnVTK/memory/` 目录下，Claude 启动时自动加载。

---

## 二、Superpowers 技能速查

Superpowers 是一套工作流技能，帮你完成从构思到编码的全流程。

### 学习流程用到的

| 技能 | 触发方式 | 什么时候用 |
|------|---------|-----------|
| **brainstorming** | 提出新想法时 | 开始新阶段/新章节前，讨论学习目标和方案 |
| **writing-plans** | 设计确定后 | 把设计文档变成逐步骤的实现计划 |
| **subagent-driven-development** | 计划写好后 | 自动按计划写代码（如果你想让我写） |
| **verification-before-completion** | 代码写完后 | 跑起来验证，确认产出可用 |

### 调试和审查用到的

| 技能 | 触发方式 | 什么时候用 |
|------|---------|-----------|
| **systematic-debugging** | 遇到 Bug 时 | 编译不过、渲染不对、崩溃——系统化排查 |
| **code-review** | `/code-review` | Review 你的代码 |
| **simplify** | `/simplify` | 重构简化代码 |

### 常用 Claude Code 命令

| 命令 | 作用 |
|------|------|
| `/clear` | 清空当前对话上下文 |
| `/compact` | 压缩对话，释放上下文空间 |
| `/config` | 修改 Claude Code 配置 |
| `/add-dir` | 添加工作目录 |
| `Ctrl+C` | 中断当前操作 |

---

## 三、学习工作流

每个 Chapter 的标准流程：

```
1. 你说："开始 ChX 学习"
2. 我给出该章的知识指南（博客风格 + 参考链接）
3. 你读指南 + 实现计划，自己动手写代码
4. 编译报错/API 不懂/渲染不对 → 问我
5. 代码写完 → 我 Review
6. 我出几道题考你（验证理解深度）
7. 通过 → 写笔记 → 更新进度 → 下一章
```

---

## 四、关键文件地图

```
LearnVTK/
├── USAGE.md                     ← 你正在看的这个文件
├── CMakeLists.txt               ← 顶层构建（每加一章改一次）
├── docs/superpowers/
│   ├── specs/
│   │   └── *-roadmap-design.md  ← 五阶段学习路线总览
│   ├── plans/
│   │   └── *-stage1-*.md        ← 阶段一逐任务实现计划
│   ├── guides/
│   │   └── Chapter*-*.md        ← 每章的知识指南（写代码前先读）
│   ├── notes/
│   │   └── Chapter*-*.md        ← 你写的学习笔记（每章一份）
│   └── PROGRESS.md              ← 进度追踪表
├── src/
│   ├── _Backup/                 ← 旧自学代码（只读参考）
│   ├── Chapter1_DataModel/      ← 阶段一第1章
│   ├── Chapter2_Pipeline/       ← 阶段一第2章
│   └── Chapter3_Rendering/      ← 阶段一第3章
└── memory/  (在 ~/.claude/projects/... 下)
    └── MEMORY.md                ← Claude 记忆索引
```

---

## 五、快速参考卡片

**开始新章节：**
```
开始 Ch1 学习
```

**代码写完了，来 Review：**
```
Ch1 写完了，帮我 Review
```

**遇到 Bug 了：**
```
编译报错：<贴错误信息>
```
或
```
渲染不出来，帮我排查
```

**考我：**
```
考我 Ch1 的知识点
```

**更新进度：**
```
Ch1 完成了，更新进度
```

---

## 六、环境速查

```powershell
# 构建单个章节
cmake --build build --target Chapter1_DataModelDemo --config Debug

# 构建所有章节
cmake --build build --config Debug

# 运行（Debug 输出在 app/bin/）
./app/bin/Chapter1_DataModelDemo.exe

# 安装新 vcpkg 包
vcpkg install <包名> --triplet=x64-windows
```

---

> 最后一次更新：2026-07-11
