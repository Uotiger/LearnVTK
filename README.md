# LearnVTK — VTK + Qt6 学习项目

基于 **VTK 9.3** + **Qt 6.9** 的 Windows 桌面可视化练习项目，使用 vcpkg 管理依赖，CMake 构建，VS Code 开发。

## 环境要求

| 依赖 | 用途 |
| ---- | ---- |
| [Visual Studio](https://visualstudio.microsoft.com/) 2019+ | C++ 编译器（MSVC） |
| [VS Code](https://code.visualstudio.com/) + CMake Tools 扩展 | 编辑器 + CMake 集成 |
| [vcpkg](https://github.com/Microsoft/vcpkg) | 包管理 |
| [Git](https://git-scm.com/) | 版本控制 |

## 快速开始

### 1. 安装 vcpkg

```powershell
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

### 2. 安装依赖包

```powershell
vcpkg install vtk[core,qt,opengl,openmp] cgns --triplet=x64-windows
# Qt6 会随 VTK 的 qt feature 自动安装
```

### 3. 设置环境变量

**必须**设置 `VCPKG_ROOT`，指向你的 vcpkg 目录：

*PowerShell（永久）:*

```powershell
[Environment]::SetEnvironmentVariable('VCPKG_ROOT', 'E:/Dev/ThirdParty/vcpkg', 'User')
```

*或者 CMD（临时）:*

```cmd
setx VCPKG_ROOT "E:/Dev/ThirdParty/vcpkg"
```

### 4. 打开项目

```powershell
code E:/Dev/Personal/LearnVTK
```

VS Code 打开后：

1. `Ctrl+Shift+P` → `CMake: Scan for Kits`（首次使用 / 新电脑需要）
2. 左下方状态栏选择预设：**Debug** 或 **Release**
3. 点击状态栏的 **生成**（Build）按钮

构建输出在 `app/bin/`（Debug）或 `app/binr/`（Release）目录。

## 架构说明

```text
┌──────────────┐     ┌───────────────────┐     ┌────────────────────┐
│  CMake Kit   │     │ CMakePresets.json │     │   vcpkg (VCPKG_ROOT)│
│ 编译器 + 环境 │  +  │ 预设: debug/release│  +  │  VTK / Qt6 / CGNS   │
└──────────────┘     └───────────────────┘     └────────────────────┘
     ↓                        ↓                        ↓
  "怎么编译"               "什么模式"              "用哪些库"
```

- **Kit** → 提供 `cl.exe`、`ninja.exe` 路径 + `PATH`/`LIB`/`INCLUDE` 环境
- **CMakePresets.json** → 指定 vcpkg 工具链文件路径（通过 `$env{VCPKG_ROOT}` 动态解析）
- **vcpkg** → 管理 VTK、Qt6、CGNS 等第三方库

不硬编码任何绝对路径，换电脑只需设一次 `VCPKG_ROOT`、扫一下 Kit 即可。

## 项目结构

```text
learnVTK/
├── CMakeLists.txt                  # 顶层 CMake：依赖查找 + 子项目
├── CMakePresets.json               # CMake 预设（debug / release）
├── .vscode/
│   └── settings.json               # VS Code CMake Tools 配置
├── cmake/
│   └── DeployQt.cmake              # Qt DLL 自动部署脚本
├── app/
│   ├── bin/                        # Debug 构建输出
│   └── binr/                       # Release 构建输出
└── src/
    └── Chapter1_BaseVTK/           # 第一章：VTK 基础
        ├── CMakeLists.txt
        ├── main.cpp                # 入口
        ├── MainRenderWindow.*      # 主渲染窗口（.ui + .h + .cpp）
        ├── CGNSDataParser.*        # CGNS 数据解析
        └── NodeTreeWidget.*        # 节点树控件
```

## 添加新章节

```powershell
mkdir src/Chapter2_MarchingCubes
# 写代码，然后：
```

在顶层 [CMakeLists.txt](CMakeLists.txt) 末尾加一行：

```cmake
add_subdirectory(src/Chapter2_MarchingCubes)
```

## 依赖包清单

| 包 | Feature | 说明 |
| ---- | ---- | ---- |
| `vtk` | `core` | VTK 核心 |
| `vtk` | `qt` | Qt 集成（自动安装 Qt6） |
| `vtk` | `opengl` | OpenGL 渲染后端 |
| `vtk` | `openmp` | 多线程加速 |
| `cgns` | — | CFD 通用网格数据格式 |

## 常见问题

### CMake 报 "vcpkg toolchain not found"

确保已设置 `VCPKG_ROOT` 环境变量并重启 VS Code。验证：

```powershell
echo $env:VCPKG_ROOT    # 应该输出你的 vcpkg 路径
```

### CMake 报 "CMAKE_CXX_COMPILER not set"

VS Code 未匹配到编译器 Kit。`Ctrl+Shift+P` → `CMake: Scan for Kits`。

### 链接器报 LNK1104 "无法打开 kernel32.lib"

Kit 环境变量未正确加载。检查状态栏右下角是否显示了选中的 Kit，如果没有，重新扫描 Kit 并选择。

### 换了 vcpkg 路径

只需更新 `VCPKG_ROOT` 环境变量，不用改任何项目文件。
