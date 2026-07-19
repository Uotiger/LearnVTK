---
date: 2026-07-19
tags: [VS Code, 调试, CMake, 环境配置]
category: 对话沉淀
---

# VS Code 调试配置原理

## 背景

习惯 Visual Studio 的"开箱即用"调试，转到 VS Code 后 F5 按了没反应。

## VS Code 调试的三个文件

### launch.json — 启动配置

```json
{
    "name": "Chapter1_DataModelDemo",        // 配置名称（下拉框显示）
    "type": "cppvsdbg",                      // Windows MSVC 调试器
    "request": "launch",                     // 启动模式（vs attach）
    "program": "${workspaceFolder}/app/bin/Chapter1_DataModelDemo.exe",
    "preLaunchTask": "CMake: build Chapter1" // 启动前先构建
}
```

### tasks.json — 构建任务

```json
{
    "label": "CMake: build Chapter1",        // 被 preLaunchTask 引用
    "type": "shell",
    "command": ".\\build.bat Chapter1_DataModelDemo"
}
```

### build.bat — 解决 PATH 问题

cmake 不在系统 PATH 里，PowerShell 对带空格的路径处理有问题，用批处理硬编码路径绕过。

## F5 执行流程

```
F5 → launch.json → preLaunchTask → tasks.json → build.bat → cmake → MSBuild → exe
                                                                      ↓
                                                              cppvsdbg 调试器启动
```

## 踩坑记录

| 问题 | 原因 | 解决 |
|------|------|------|
| cmake: command not found | 不在 PATH | 写 build.bat 硬编码路径 |
| PowerShell 拆路径 | 空格被当分隔符 | 改用 cmd.exe |
| tasks 里 command 找不到 | cmd 需要 `.\` 前缀 | 改成 `.\\build.bat` |

## 我的理解

VS Code 的哲学是"一切可配置"，不像 VS 自动检测项目类型。好处是灵活，坏处是初次配置麻烦。一旦配好，后续就是改改 target 名字的事。

## 关联

- 相关章节：所有章节
- 相关笔记：[[2026-07-19-flywheel-startup]]
