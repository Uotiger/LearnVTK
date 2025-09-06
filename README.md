# 这是一个QT+VTK学习示例项目
## 项目简介
    这是一个基于vtk9.3版本 qt6.9版本的练习项目，依赖vcpkg构建的windows桌面项目; 需要在vcpkg中安装vtk与qt。

## 安装依赖
    本项目使用vcpkg 作为包管理工具来管理 VTK 和 QT 依赖。

### 1. 安装 vcpkg
####  克隆 vcpkg 仓库
    git clone https://github.com/Microsoft/vcpkg.git

#### 运行引导脚本
    cd vcpkg
    ./bootstrap-vcpkg.bat  # Windows
#### 安装vtk
    vcpkg install vtk[core,qt,opengl,openmp] --triplet=x64-windows #vtk会自动装qt
#### vcpkg 作用与全局
    vcpkg integrate install
### 2. 修改cmake
    leranVTK/CmakeLists.txt修改
    #cmake 中设置vcpkg.cmake路径
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND EXISTS "E:/xxx/vcpkg/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "E:/xxxx/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    endif()

## 项目结构
    learnVTK/
    ├── CMakeLists.txt              # 主 CMake 配置文件
    ├── cmake/
    │   └── DeployQt.cmake         # Qt 部署工具
    ├── app/
    │   ├── bin/                   # 调试版输出目录
    │   └── binr/                  # 发布版输出目录
    └── src/
    └── Chapter1_BaseVTK/       # 第一章示例代码
    └── CMakeLists.txt          # 第一章的 CMake 配置
    └── Chapter2_XXXVTK/        # 第二章示例代码
    └── CMakeLists.txt          # 第二章的 CMake 配置
