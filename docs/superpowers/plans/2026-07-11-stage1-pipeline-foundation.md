# 阶段一：VTK 管线内化 — 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 实现 Ch1（数据模型）、Ch2（管线机制）、Ch3（渲染模型）三个独立 Demo，将 Source → Filter → Mapper → Actor 变成肌肉记忆。

**Architecture:** 每个 Chapter 是独立的 CMake 子项目。不依赖 Chapter1，不使用 .ui 文件——所有 UI 用纯代码创建。每个 Demo 是一个 QMainWindow 内嵌 QVTKOpenGLNativeWidget，核心逻辑集中在 setupPipeline() 方法中。

**Tech Stack:** C++17, CMake, vcpkg (vtk[core,qt,opengl,openmp]), Qt6 (Widgets, OpenGLWidgets)

## Global Constraints

- 语言：C++17，VS 2019+ 编译通过
- 依赖管理：vcpkg，不引入新包（当前 vtk[core,qt,opengl,openmp] 已覆盖阶段一需求）
- 构建：CMake，每章一个独立子项目，通过顶层 CMakeLists.txt 的 add_subdirectory 注册
- 平台：Windows，不要求跨平台
- 不使用 Qt Designer (.ui) 文件——所有 UI 纯代码创建
- 不依赖 Chapter1 的任何文件
- 每章完成后写笔记到 docs/superpowers/notes/

---

## 文件结构

```
learnVTK/
├── CMakeLists.txt                          # 修改：添加 add_subdirectory 行
├── src/
│   ├── Chapter1_BaseVTK/                   # 已存在，不改动
│   ├── Chapter1_DataModel/                 # 新建
│   │   ├── CMakeLists.txt
│   │   ├── main.cpp                       # 入口
│   │   ├── DataModelDemo.h                # 主窗口声明
│   │   └── DataModelDemo.cpp              # 管线实现
│   ├── Chapter2_Pipeline/                  # 新建
│   │   ├── CMakeLists.txt
│   │   ├── main.cpp
│   │   ├── PipelineDemo.h
│   │   └── PipelineDemo.cpp
│   └── Chapter3_Rendering/                 # 新建
│       ├── CMakeLists.txt
│       ├── main.cpp
│       ├── RenderingDemo.h
│       └── RenderingDemo.cpp
└── docs/
    └── superpowers/
        └── notes/                          # 新建
            ├── Chapter1-DataModel.md       # 笔记模板
            ├── Chapter2-Pipeline.md
            └── Chapter3-Rendering.md
```

---

### Task 1: 创建 Ch1 项目骨架

**Files:**
- Create: `src/Chapter1_DataModel/CMakeLists.txt`
- Create: `src/Chapter1_DataModel/main.cpp`
- Create: `src/Chapter1_DataModel/DataModelDemo.h`
- Create: `src/Chapter1_DataModel/DataModelDemo.cpp`
- Modify: `CMakeLists.txt` (末尾加一行 add_subdirectory)

**Interfaces:**
- Produces: `DataModelDemo` 类 — 继承 QMainWindow，构造函数创建 VTK 管线并显示 5 种数据集类型

- [ ] **Step 1: 创建 CMakeLists.txt**

写入 `src/Chapter1_DataModel/CMakeLists.txt`：

```cmake
add_executable(Chapter1_DataModelDemo
    main.cpp
    DataModelDemo.h
    DataModelDemo.cpp
)

target_link_libraries(Chapter1_DataModelDemo PRIVATE
    ${VTK_LIBRARIES}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
)

set_target_properties(Chapter1_DataModelDemo PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

deploy_qt_for_target(Chapter1_DataModelDemo)
```

- [ ] **Step 2: 创建 main.cpp**

写入 `src/Chapter1_DataModel/main.cpp`：

```cpp
#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>
#include "DataModelDemo.h"

int main(int argc, char* argv[])
{
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication app(argc, argv);

    DataModelDemo window;
    window.show();

    return app.exec();
}
```

- [ ] **Step 3: 创建 DataModelDemo.h**

写入 `src/Chapter1_DataModel/DataModelDemo.h`：

```cpp
#pragma once

#include <QMainWindow>
#include <QComboBox>
#include <QLabel>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkActor.h>

class QVTKOpenGLNativeWidget;

class DataModelDemo : public QMainWindow
{
    Q_OBJECT

public:
    explicit DataModelDemo(QWidget* parent = nullptr);
    ~DataModelDemo() override = default;

private slots:
    void onDatasetTypeChanged(int index);

private:
    void setupUI();
    void setupPipeline();
    void buildImageData();
    void buildRectilinearGrid();
    void buildStructuredGrid();
    void buildPolyData();
    void buildUnstructuredGrid();
    void showDatasetInfo();

    // VTK 渲染组件
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkActor> m_actor;
    vtkSmartPointer<vtkDataSet> m_currentDataset;

    // Qt 控件
    QVTKOpenGLNativeWidget* m_vtkWidget;
    QComboBox* m_datasetCombo;
    QLabel* m_infoLabel;
};
```

- [ ] **Step 4: 创建 DataModelDemo.cpp 框架**

写入 `src/Chapter1_DataModel/DataModelDemo.cpp` 的开头部分（setupUI + 构造函数 + 骨架）：

```cpp
#include "DataModelDemo.h"

#include <QVTKOpenGLNativeWidget.h>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

// ---- 构造函数 ----

DataModelDemo::DataModelDemo(QWidget* parent)
    : QMainWindow(parent)
    , m_vtkWidget(nullptr)
    , m_datasetCombo(nullptr)
    , m_infoLabel(nullptr)
{
    setWindowTitle("Chapter 1 — VTK 数据模型");
    resize(1200, 800);

    setupUI();
    setupPipeline();
    buildPolyData(); // 默认显示 PolyData
}

// ---- UI 搭建 ----

void DataModelDemo::setupUI()
{
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    // 顶部控制栏
    auto* topBar = new QHBoxLayout();
    topBar->addWidget(new QLabel("数据集类型：", this));

    m_datasetCombo = new QComboBox(this);
    m_datasetCombo->addItems({
        "vtkImageData (规则网格)",
        "vtkRectilinearGrid (矩形网格)",
        "vtkStructuredGrid (结构化网格)",
        "vtkPolyData (多边形数据)",
        "vtkUnstructuredGrid (非结构化网格)"
    });
    connect(m_datasetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DataModelDemo::onDatasetTypeChanged);
    topBar->addWidget(m_datasetCombo);

    topBar->addStretch();

    m_infoLabel = new QLabel(this);
    m_infoLabel->setStyleSheet("QLabel { color: #555; font-size: 13px; }");
    topBar->addWidget(m_infoLabel);

    mainLayout->addLayout(topBar);

    // VTK 渲染窗口
    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    mainLayout->addWidget(m_vtkWidget, 1);
}

// ---- 管线初始化 ----

void DataModelDemo::setupPipeline()
{
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderer->SetBackground(0.2, 0.2, 0.25);

    m_renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_renderWindow->AddRenderer(m_renderer);
    m_vtkWidget->setRenderWindow(m_renderWindow);

    m_actor = vtkSmartPointer<vtkActor>::New();
}
```

- [ ] **Step 5: 更新顶层 CMakeLists.txt**

在 `CMakeLists.txt` 末尾的 `add_subdirectory(src/Chapter1_BaseVTK)` 之后追加一行：

```cmake
add_subdirectory(src/Chapter1_DataModel)
```

- [ ] **Step 6: 构建验证**

```bash
cd e:/Dev/Personal/LearnVTK && cmake --build build --target Chapter1_DataModelDemo --config Debug
```

预期：编译通过，启动后显示空白窗口（管线尚未连接数据）。

- [ ] **Step 7: Commit**

```bash
git add src/Chapter1_DataModel/ CMakeLists.txt
git commit -m "feat(Ch1): add project skeleton with UI and renderer"
```

---

### Task 2: Ch1 — 实现五种数据集构造

**Files:**
- Modify: `src/Chapter1_DataModel/DataModelDemo.cpp` — 实现 5 个 build 方法和 onDatasetTypeChanged

**Interfaces:**
- Consumes: `DataModelDemo` 类骨架（Task 1）
- Produces: 可运行 Demo，切换下拉框显示不同数据集类型

- [ ] **Step 1: 实现 buildImageData()**

在 `DataModelDemo.cpp` 中追加：

```cpp
void DataModelDemo::buildImageData()
{
    auto image = vtkSmartPointer<vtkImageData>::New();
    image->SetDimensions(10, 8, 6);           // X Y Z 方向各多少个采样点
    image->SetOrigin(0.0, 0.0, 0.0);          // 左下角世界坐标
    image->SetSpacing(1.0, 1.0, 1.0);         // 采样间距

    // 添加标量 Point Data
    auto scalars = vtkSmartPointer<vtkFloatArray>::New();
    scalars->SetName("Distance");
    int dims[3];
    image->GetDimensions(dims);
    for (int k = 0; k < dims[2]; ++k) {
        for (int j = 0; j < dims[1]; ++j) {
            for (int i = 0; i < dims[0]; ++i) {
                double x = i - dims[0] / 2.0;
                double y = j - dims[1] / 2.0;
                double z = k - dims[2] / 2.0;
                scalars->InsertNextValue(std::sqrt(x*x + y*y + z*z));
            }
        }
    }
    image->GetPointData()->SetScalars(scalars);

    m_currentDataset = image;
}
```

- [ ] **Step 2: 实现 buildRectilinearGrid()**

```cpp
void DataModelDemo::buildRectilinearGrid()
{
    auto grid = vtkSmartPointer<vtkRectilinearGrid>::New();

    // X 坐标：非均匀间距
    auto xCoords = vtkSmartPointer<vtkFloatArray>::New();
    xCoords->SetName("X");
    xCoords->InsertNextValue(0.0);
    xCoords->InsertNextValue(1.0);
    xCoords->InsertNextValue(2.5);
    xCoords->InsertNextValue(4.0);
    xCoords->InsertNextValue(6.0);
    xCoords->InsertNextValue(8.5);

    // Y 坐标：非均匀间距
    auto yCoords = vtkSmartPointer<vtkFloatArray>::New();
    yCoords->SetName("Y");
    yCoords->InsertNextValue(0.0);
    yCoords->InsertNextValue(1.5);
    yCoords->InsertNextValue(3.0);
    yCoords->InsertNextValue(5.0);
    yCoords->InsertNextValue(7.0);

    // Z 坐标：均匀间距
    auto zCoords = vtkSmartPointer<vtkFloatArray>::New();
    zCoords->SetName("Z");
    for (int i = 0; i < 5; ++i)
        zCoords->InsertNextValue(i * 1.0);

    grid->SetXCoordinates(xCoords);
    grid->SetYCoordinates(yCoords);
    grid->SetZCoordinates(zCoords);

    // 添加标量 Cell Data：每个单元的体积
    auto volumes = vtkSmartPointer<vtkFloatArray>::New();
    volumes->SetName("CellVolume");
    int dims[3];
    grid->GetDimensions(dims);
    for (int k = 0; k < dims[2] - 1; ++k) {
        for (int j = 0; j < dims[1] - 1; ++j) {
            for (int i = 0; i < dims[0] - 1; ++i) {
                double dx = xCoords->GetValue(i+1) - xCoords->GetValue(i);
                double dy = yCoords->GetValue(j+1) - yCoords->GetValue(j);
                double dz = zCoords->GetValue(k+1) - zCoords->GetValue(k);
                volumes->InsertNextValue(dx * dy * dz);
            }
        }
    }
    grid->GetCellData()->SetScalars(volumes);

    m_currentDataset = grid;
}
```

- [ ] **Step 3: 实现 buildStructuredGrid()**

```cpp
void DataModelDemo::buildStructuredGrid()
{
    auto grid = vtkSmartPointer<vtkStructuredGrid>::New();
    grid->SetDimensions(8, 6, 4);              // IJK 各方向点数

    auto points = vtkSmartPointer<vtkPoints>::New();
    points->Allocate(8 * 6 * 4);

    for (int k = 0; k < 4; ++k) {
        for (int j = 0; j < 6; ++j) {
            for (int i = 0; i < 8; ++i) {
                // 加正弦扰动，展示"弯曲"的结构化网格
                double x = i + 0.3 * std::sin(k * 1.2);
                double y = j + 0.3 * std::cos(k * 1.2);
                double z = k + 0.2 * std::sin(i * 0.5 + j * 0.5);
                points->InsertNextPoint(x, y, z);
            }
        }
    }
    grid->SetPoints(points);

    // 添加标量 Point Data：每个点的 Z 扰动值
    auto warp = vtkSmartPointer<vtkFloatArray>::New();
    warp->SetName("Warp");
    int dims[3];
    grid->GetDimensions(dims);
    for (int k = 0; k < dims[2]; ++k) {
        for (int j = 0; j < dims[1]; ++j) {
            for (int i = 0; i < dims[0]; ++i) {
                double w = 0.2 * std::sin(i * 0.5 + j * 0.5);
                warp->InsertNextValue(static_cast<float>(w));
            }
        }
    }
    grid->GetPointData()->SetScalars(warp);

    m_currentDataset = grid;
}
```

- [ ] **Step 4: 实现 buildPolyData()**

```cpp
void DataModelDemo::buildPolyData()
{
    auto polyData = vtkSmartPointer<vtkPolyData>::New();

    // 手动构造一个六面体（8 顶点 + 6 面 → 12 三角形）
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->InsertNextPoint(0, 0, 0);  // 0
    points->InsertNextPoint(2, 0, 0);  // 1
    points->InsertNextPoint(2, 2, 0);  // 2
    points->InsertNextPoint(0, 2, 0);  // 3
    points->InsertNextPoint(0, 0, 2);  // 4
    points->InsertNextPoint(2, 0, 2);  // 5
    points->InsertNextPoint(2, 2, 2);  // 6
    points->InsertNextPoint(0, 2, 2);  // 7
    polyData->SetPoints(points);

    // 12 个三角形（每面 2 个）
    auto polys = vtkSmartPointer<vtkCellArray>::New();
    // 底面
    polys->InsertNextCell(3); polys->InsertCellPoint(0); polys->InsertCellPoint(1); polys->InsertCellPoint(2);
    polys->InsertNextCell(3); polys->InsertCellPoint(0); polys->InsertCellPoint(2); polys->InsertCellPoint(3);
    // 顶面
    polys->InsertNextCell(3); polys->InsertCellPoint(4); polys->InsertCellPoint(6); polys->InsertCellPoint(5);
    polys->InsertNextCell(3); polys->InsertCellPoint(4); polys->InsertCellPoint(7); polys->InsertCellPoint(6);
    // 前面
    polys->InsertNextCell(3); polys->InsertCellPoint(0); polys->InsertCellPoint(4); polys->InsertCellPoint(1);
    polys->InsertNextCell(3); polys->InsertCellPoint(1); polys->InsertCellPoint(4); polys->InsertCellPoint(5);
    // 后面
    polys->InsertNextCell(3); polys->InsertCellPoint(3); polys->InsertCellPoint(2); polys->InsertCellPoint(7);
    polys->InsertNextCell(3); polys->InsertCellPoint(2); polys->InsertCellPoint(6); polys->InsertCellPoint(7);
    // 左面
    polys->InsertNextCell(3); polys->InsertCellPoint(0); polys->InsertCellPoint(3); polys->InsertCellPoint(4);
    polys->InsertNextCell(3); polys->InsertCellPoint(3); polys->InsertCellPoint(7); polys->InsertCellPoint(4);
    // 右面
    polys->InsertNextCell(3); polys->InsertCellPoint(1); polys->InsertCellPoint(5); polys->InsertCellPoint(2);
    polys->InsertNextCell(3); polys->InsertCellPoint(2); polys->InsertCellPoint(5); polys->InsertCellPoint(6);
    polyData->SetPolys(polys);

    // 添加 Point Data：高度
    auto heights = vtkSmartPointer<vtkFloatArray>::New();
    heights->SetName("Height");
    for (int i = 0; i < 8; ++i) {
        double p[3]; points->GetPoint(i, p);
        heights->InsertNextValue(static_cast<float>(p[2]));
    }
    polyData->GetPointData()->SetScalars(heights);

    m_currentDataset = polyData;
}
```

- [ ] **Step 5: 实现 buildUnstructuredGrid()**

```cpp
void DataModelDemo::buildUnstructuredGrid()
{
    auto ug = vtkSmartPointer<vtkUnstructuredGrid>::New();

    // 10 个点，混合单元类型
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataTypeToDouble();
    points->InsertNextPoint(0, 0, 0);   // 0 — 四面体底
    points->InsertNextPoint(2, 0, 0);   // 1
    points->InsertNextPoint(1, 2, 0);   // 2
    points->InsertNextPoint(1, 0.7, 2); // 3 — 四面体顶
    points->InsertNextPoint(4, 0, 0);   // 4 — 六面体开始
    points->InsertNextPoint(5, 0, 0);   // 5
    points->InsertNextPoint(5, 2, 0);   // 6
    points->InsertNextPoint(4, 2, 0);   // 7
    points->InsertNextPoint(4, 0, 1.5); // 8
    points->InsertNextPoint(5, 0, 1.5); // 9
    ug->SetPoints(points);

    // 四面体 (VTK_TETRA = 10)，4 个节点
    auto tetra = vtkSmartPointer<vtkTetra>::New();
    tetra->GetPointIds()->SetId(0, 0);
    tetra->GetPointIds()->SetId(1, 1);
    tetra->GetPointIds()->SetId(2, 2);
    tetra->GetPointIds()->SetId(3, 3);
    ug->InsertNextCell(VTK_TETRA, tetra->GetPointIds());

    // 六面体 (VTK_HEXAHEDRON = 12)，8 个节点（一个四棱柱）
    auto hex = vtkSmartPointer<vtkHexahedron>::New();
    hex->GetPointIds()->SetId(0, 4);
    hex->GetPointIds()->SetId(1, 5);
    hex->GetPointIds()->SetId(2, 6);
    hex->GetPointIds()->SetId(3, 7);
    hex->GetPointIds()->SetId(4, 4); // 上表面 — 退化为三棱柱形
    hex->GetPointIds()->SetId(5, 5);
    hex->GetPointIds()->SetId(6, 6);
    hex->GetPointIds()->SetId(7, 7);
    ug->InsertNextCell(VTK_HEXAHEDRON, hex->GetPointIds());

    // 添加 Cell Data：单元类型标签
    auto cellType = vtkSmartPointer<vtkIntArray>::New();
    cellType->SetName("CellType");
    cellType->InsertNextValue(0); // tetra
    cellType->InsertNextValue(1); // hex
    ug->GetCellData()->SetScalars(cellType);

    m_currentDataset = ug;
}
```

- [ ] **Step 6: 实现 onDatasetTypeChanged() 和 showDatasetInfo()**

```cpp
void DataModelDemo::onDatasetTypeChanged(int index)
{
    switch (index) {
    case 0: buildImageData();          break;
    case 1: buildRectilinearGrid();    break;
    case 2: buildStructuredGrid();     break;
    case 3: buildPolyData();           break;
    case 4: buildUnstructuredGrid();   break;
    }

    // 连接 Mapper → Actor → Renderer
    auto mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputData(m_currentDataset);
    mapper->ScalarVisibilityOn();

    m_actor->SetMapper(mapper);
    m_actor->GetProperty()->SetRepresentationToSurface();
    m_actor->GetProperty()->EdgeVisibilityOn();
    m_actor->GetProperty()->SetEdgeColor(0.1, 0.1, 0.1);

    m_renderer->RemoveAllViewProps();
    m_renderer->AddActor(m_actor);
    m_renderer->ResetCamera();
    m_renderWindow->Render();

    showDatasetInfo();
}

void DataModelDemo::showDatasetInfo()
{
    if (!m_currentDataset) return;

    vtkIdType nPoints = m_currentDataset->GetNumberOfPoints();
    vtkIdType nCells  = m_currentDataset->GetNumberOfCells();

    const char* typeName = m_currentDataset->GetClassName();

    QString info = QString("%1 | 点数: %2 | 单元数: %3")
        .arg(typeName).arg(nPoints).arg(nCells);
    m_infoLabel->setText(info);
}
```

补充头文件 include（在 DataModelDemo.cpp 顶部追加）：

```cpp
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkTetra.h>
#include <vtkHexahedron.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVersion.h>
```

- [ ] **Step 7: 构建并运行验证**

```bash
cmake --build build --target Chapter1_DataModelDemo --config Debug
./app/bin/Chapter1_DataModelDemo.exe
```

预期：窗口显示，下拉切换 5 种数据集类型，每种以白色线框 + 表面渲染，左上角显示数据集类型和点/单元数。

- [ ] **Step 8: Commit**

```bash
git add src/Chapter1_DataModel/
git commit -m "feat(Ch1): implement 5 dataset type constructors and switching"
```

---

### Task 3: Ch1 — 写学习笔记

**Files:**
- Create: `docs/superpowers/notes/Chapter1-DataModel.md`

- [ ] **Step 1: 写笔记**

写入 `docs/superpowers/notes/Chapter1-DataModel.md`：

```markdown
# Chapter 1 — VTK 数据模型

## 核心概念

VTK 的数据模型分为两层：**几何拓扑**（点坐标 + 单元连接关系）和**属性数据**（附加在点/单元上的标量/矢量/张量）。

五种 DataSet 的核心区别在于**拓扑关系的显式程度**：
- ImageData：隐式拓扑，仅存 origin + spacing + dimensions，内存最省
- RectilinearGrid：X/Y/Z 各存一组坐标数组，比 ImageData 灵活但仍规则
- StructuredGrid：每个点显式存坐标，但拓扑仍由 IJK 索引隐式推导
- PolyData：点 + 显式单元连接（Verts/Lines/Polys/Strips），最灵活的表面模型
- UnstructuredGrid：点 + 显式单元连接，支持混合单元类型（四面体/六面体/棱柱等）

Point Data 每个点一个值（渲染时内部插值），Cell Data 每个单元一个值（渲染时单元内为常量）。

## 关键 API 速查

| 类 | 关键方法 |
|----|---------|
| vtkImageData | SetDimensions(), SetOrigin(), SetSpacing() |
| vtkRectilinearGrid | SetXCoordinates(), SetYCoordinates(), SetZCoordinates() |
| vtkStructuredGrid | SetDimensions(), SetPoints() |
| vtkPolyData | SetPoints(), SetPolys(), SetLines(), SetVerts() |
| vtkUnstructuredGrid | SetPoints(), InsertNextCell() |
| vtkDataSetMapper | SetInputData(), ScalarVisibilityOn/Off() |
| vtkPointData / vtkCellData | SetScalars(), GetScalars() |

## 踩坑记录

（学习中遇到的坑和解决方案）

## 原理追问

1. vtkImageData 的 GetDimensions() 返回的数组长度是 IJK 点数，但显示出来的单元数为什么是 (I-1)*(J-1)*(K-1)？
   - 答：因为 N 个点等间距排列产生 N-1 个区间（体素），每个体素就是一个 cell。

2. 为什么非结构化网格（UnstructuredGrid）最耗内存？
   - 答：除了存点坐标，还需要显式存储每个单元的节点连接列表（Connectivity List），以及单元偏移量；而结构化网格的单元连接关系可由 IJK 索引直接推导。
```

- [ ] **Step 2: Commit**

```bash
git add docs/superpowers/notes/Chapter1-DataModel.md
git commit -m "docs(Ch1): add data model study notes"
```

---

### Task 4: 创建 Ch2 项目骨架

**Files:**
- Create: `src/Chapter2_Pipeline/CMakeLists.txt`
- Create: `src/Chapter2_Pipeline/main.cpp`
- Create: `src/Chapter2_Pipeline/PipelineDemo.h`
- Create: `src/Chapter2_Pipeline/PipelineDemo.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 创建 CMakeLists.txt**

写入 `src/Chapter2_Pipeline/CMakeLists.txt`：

```cmake
add_executable(Chapter2_PipelineDemo
    main.cpp
    PipelineDemo.h
    PipelineDemo.cpp
)

target_link_libraries(Chapter2_PipelineDemo PRIVATE
    ${VTK_LIBRARIES}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
)

set_target_properties(Chapter2_PipelineDemo PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

deploy_qt_for_target(Chapter2_PipelineDemo)
```

- [ ] **Step 2: 创建 main.cpp**

写入 `src/Chapter2_Pipeline/main.cpp`（与 Ch1 的 main.cpp 模式相同，将 `DataModelDemo` 替换为 `PipelineDemo`）。

- [ ] **Step 3: 创建 PipelineDemo.h**

写入 `src/Chapter2_Pipeline/PipelineDemo.h`：

```cpp
#pragma once

#include <QMainWindow>
#include <QSlider>
#include <QLabel>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkActor.h>
#include <vtkSphereSource.h>
#include <vtkShrinkFilter.h>

class QVTKOpenGLNativeWidget;

// PipelineDemo 演示 VTK 的 Demand-Driven Pipeline：
// Source (Sphere) → Filter (Shrink) → Mapper → Actor
// 修改 ShrinkFactor 参数 → Filter 标记 Modified → 下次 Render 时自动重计算

class PipelineDemo : public QMainWindow
{
    Q_OBJECT

public:
    explicit PipelineDemo(QWidget* parent = nullptr);
    ~PipelineDemo() override = default;

private slots:
    void onShrinkFactorChanged(int value);
    void onPhiResolutionChanged(int value);
    void onThetaResolutionChanged(int value);

private:
    void setupUI();
    void setupPipeline();
    void addSlider(const QString& label, int min, int max, int value,
                   std::function<void(int)> callback);

    // VTK
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkActor> m_actor;
    vtkSmartPointer<vtkSphereSource> m_sphereSource;
    vtkSmartPointer<vtkShrinkFilter> m_shrinkFilter;

    // Qt
    QVTKOpenGLNativeWidget* m_vtkWidget;
    QWidget* m_controlPanel;
};
```

- [ ] **Step 4: 创建 PipelineDemo.cpp 的 UI 和管线框架**

```cpp
#include "PipelineDemo.h"

#include <QVTKOpenGLNativeWidget.h>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSlider>
#include <QLabel>
#include <functional>

#include <vtkDataSetMapper.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

PipelineDemo::PipelineDemo(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Chapter 3 — VTK 管线机制 (Demand-Driven Pipeline)");
    resize(1200, 800);
    setupUI();
    setupPipeline();
    onShrinkFactorChanged(50); // 初始调用触发首次渲染
}

void PipelineDemo::setupUI()
{
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* hLayout = new QHBoxLayout(central);
    hLayout->setContentsMargins(4, 4, 4, 4);

    // 左侧 VTK 窗口
    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    hLayout->addWidget(m_vtkWidget, 3);

    // 右侧控制面板
    m_controlPanel = new QWidget(this);
    auto* ctrlLayout = new QVBoxLayout(m_controlPanel);
    ctrlLayout->setContentsMargins(8, 8, 8, 8);

    auto* infoLabel = new QLabel(
        "管线：vtkSphereSource → vtkShrinkFilter → vtkDataSetMapper → vtkActor\n"
        "调整参数，观察 Filter 如何触发管线重计算", this);
    infoLabel->setWordWrap(true);
    ctrlLayout->addWidget(infoLabel);
    ctrlLayout->addSpacing(12);

    // 三个滑块
    addSlider("Shrink Factor (收缩因子)", 10, 100, 50,
              [this](int v) { onShrinkFactorChanged(v); });
    addSlider("Phi Resolution (经线分段)", 3, 100, 16,
              [this](int v) { onPhiResolutionChanged(v); });
    addSlider("Theta Resolution (纬线分段)", 3, 100, 16,
              [this](int v) { onThetaResolutionChanged(v); });

    ctrlLayout->addStretch();
    hLayout->addWidget(m_controlPanel);
}

void PipelineDemo::addSlider(const QString& label, int min, int max,
                              int defaultValue,
                              std::function<void(int)> callback)
{
    auto* group = new QGroupBox(label, m_controlPanel);
    auto* layout = new QVBoxLayout(group);

    auto* slider = new QSlider(Qt::Horizontal, group);
    slider->setRange(min, max);
    slider->setValue(defaultValue);
    layout->addWidget(slider);

    auto* valueLabel = new QLabel(QString::number(defaultValue), group);
    valueLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(valueLabel);

    connect(slider, &QSlider::valueChanged, this,
            [callback, valueLabel](int v) {
                valueLabel->setText(QString::number(v));
                callback(v);
            });

    group->setLayout(layout);
    auto* ctrlLayout = m_controlPanel->layout();
    ctrlLayout->addWidget(group);
}

void PipelineDemo::setupPipeline()
{
    // Source: 球体
    m_sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    m_sphereSource->SetRadius(5.0);
    m_sphereSource->SetPhiResolution(16);
    m_sphereSource->SetThetaResolution(16);

    // Filter: 收缩（每个单元的顶点向中心收缩，放大单元边界）
    m_shrinkFilter = vtkSmartPointer<vtkShrinkFilter>::New();
    m_shrinkFilter->SetInputConnection(m_sphereSource->GetOutputPort());
    m_shrinkFilter->SetShrinkFactor(0.5);

    // Mapper
    auto mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputConnection(m_shrinkFilter->GetOutputPort());

    // Actor
    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(mapper);
    m_actor->GetProperty()->SetColor(0.2, 0.6, 0.9);
    m_actor->GetProperty()->EdgeVisibilityOn();
    m_actor->GetProperty()->SetEdgeColor(0.1, 0.1, 0.2);
    m_actor->GetProperty()->SetLineWidth(0.5);

    // Renderer
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderer->SetBackground(0.22, 0.22, 0.25);
    m_renderer->AddActor(m_actor);

    m_renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_renderWindow->AddRenderer(m_renderer);
    m_vtkWidget->setRenderWindow(m_renderWindow);
}
```

- [ ] **Step 5: 实现回调 + 更新顶层 CMakeLists.txt**

在 PipelineDemo.cpp 末尾追加三个回调：

```cpp
void PipelineDemo::onShrinkFactorChanged(int value)
{
    // ShrinkFilter 修改参数 → 内部标记 Modified → 下次 Render 时重计算
    m_shrinkFilter->SetShrinkFactor(value / 100.0);
    m_renderWindow->Render();
}

void PipelineDemo::onPhiResolutionChanged(int value)
{
    m_sphereSource->SetPhiResolution(value);
    m_renderWindow->Render();
}

void PipelineDemo::onThetaResolutionChanged(int value)
{
    m_sphereSource->SetThetaResolution(value);
    m_renderWindow->Render();
}
```

更新 `CMakeLists.txt`，在已有 add_subdirectory 行之后追加：

```cmake
add_subdirectory(src/Chapter2_Pipeline)
```

- [ ] **Step 6: 构建验证**

```bash
cmake --build build --target Chapter2_PipelineDemo --config Debug
```

预期：窗口左侧显示球体，右侧三个滑块分别控制收缩程度和网格分辨率，拖动滑块时实时更新。

- [ ] **Step 7: Commit**

```bash
git add src/Chapter2_Pipeline/ CMakeLists.txt
git commit -m "feat(Ch2): add pipeline mechanism demo with ShrinkFilter"
```

---

### Task 5: Ch2 — 添加管线变化演示 + 笔记

**Files:**
- Modify: `src/Chapter2_Pipeline/PipelineDemo.cpp` — 新增一个 Filter 切换能力
- Create: `docs/superpowers/notes/Chapter2-Pipeline.md`

- [ ] **Step 1: 在 PipelineDemo 中增加"第二管线"对比**

在构造函数末尾（setupPipeline 之后）添加多个 Filter 选项的下拉框，让用户切换不同 Filter（Shrink / Clip / Contour），直观对比效果。

头文件 PipelineDemo.h 新增：

```cpp
#include <QComboBox>
// 新增成员：
QComboBox* m_filterCombo;
vtkSmartPointer<vtkAlgorithm> m_currentFilter;

private slots:
    void onFilterChanged(int index);
```

cpp 新增：

```cpp
// setupUI 中在滑块之后添加：
ctrlLayout->addSpacing(12);
m_filterCombo = new QComboBox(m_controlPanel);
m_filterCombo->addItems({"vtkShrinkFilter (收缩)", "vtkClipDataSet (裁剪)", "vtkContourFilter (等值面)"});
connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &PipelineDemo::onFilterChanged);
ctrlLayout->addWidget(m_filterCombo);

// 回调实现：
void PipelineDemo::onFilterChanged(int index)
{
    auto mapper = vtkDataSetMapper::SafeDownCast(m_actor->GetMapper());
    switch (index) {
    case 0:
        mapper->SetInputConnection(m_shrinkFilter->GetOutputPort());
        break;
    case 1: {
        auto clip = vtkSmartPointer<vtkClipDataSet>::New();
        clip->SetInputConnection(m_sphereSource->GetOutputPort());
        clip->SetValue(0.0); // 裁剪平面穿过球心
        mapper->SetInputConnection(clip->GetOutputPort());
        break;
    }
    case 2: {
        auto contour = vtkSmartPointer<vtkContourFilter>::New();
        contour->SetInputConnection(m_sphereSource->GetOutputPort());
        contour->GenerateValues(5, 0.0, 5.0); // 5条等值线/面
        mapper->SetInputConnection(contour->GetOutputPort());
        break;
    }
    }
    m_renderWindow->Render();
}
```

增加 include：

```cpp
#include <vtkClipDataSet.h>
#include <vtkContourFilter.h>
```

- [ ] **Step 2: 构建验证**

```bash
cmake --build build --target Chapter2_PipelineDemo --config Debug
```

预期：下拉切换 Filter，球体分别以收缩/裁剪/等值面三种效果展示。

- [ ] **Step 3: 写笔记**

写入 `docs/superpowers/notes/Chapter2-Pipeline.md`：

```markdown
# Chapter 3 — VTK 管线机制

## 核心概念

VTK 管线采用 **Demand-Driven Pipeline** 模式：
1. 只有在 Render（或其他 sink）调用 Update() 时，数据才真正沿管线流动
2. 上游 Filter 被修改后，内部 Modified 标记置位
3. 下次 Update 时，VTK 递归检查上游 Modified 标记，仅重计算变脏的环节

这种"懒执行"策略的核心优势：在复杂可视化场景中，修改一个参数只触发受影响的部分重新计算，而非整条管线。

## 关键 API 速查

| 类 | 关键方法 |
|----|---------|
| vtkAlgorithm | GetOutputPort(), Update(), Modified() |
| vtkAlgorithmOutput | 管线的连接点，不是数据，是数据的"承诺" |
| vtkShrinkFilter | SetShrinkFactor(0.0~1.0) |
| vtkClipDataSet | SetValue(), SetInsideOut() |
| vtkContourFilter | GenerateValues(count, min, max) |

## SetInputConnection vs SetInputData

- `SetInputConnection(filter->GetOutputPort())`：建立管线连接，数据按需流动。推荐方式。
- `SetInputData(filter->GetOutput())`：立即触发上游 Update，切断管线连接。仅在需要截取中间结果时使用。

## 踩坑记录

（学习中遇到的坑和解决方案）

## 原理追问

1. 为什么连续拖动滑块时不会每次都完全重建管线？
   - 答：Render 调用 Update → 发现 Filter Modified → 执行 RequestData 重算数据 → Mapper 拿到新数据上传 GPU → 绘制。但 Source 没有 Modified，所以 SphereSource 的数据只计算一次。

2. 如果管线分叉（一个 Source 同时连两个 Filter），Update 会触发几次 Source 计算？
   - 答：一次。VTK 使用时间戳机制，两个 Filter 触发 Update → 都在同一个 pipeline pass 中请求上游 → Source 计算一次后将时间戳推进，第二个 Filter 发现时间戳已更新则直接使用缓存结果。
```

- [ ] **Step 4: Commit**

```bash
git add src/Chapter2_Pipeline/ docs/superpowers/notes/Chapter2-Pipeline.md
git commit -m "feat(Ch2): add filter switching + pipeline study notes"
```

---

### Task 6: 创建 Ch3 项目

**Files:**
- Create: `src/Chapter3_Rendering/CMakeLists.txt`
- Create: `src/Chapter3_Rendering/main.cpp`
- Create: `src/Chapter3_Rendering/RenderingDemo.h`
- Create: `src/Chapter3_Rendering/RenderingDemo.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 创建完整文件**

写入 `src/Chapter3_Rendering/CMakeLists.txt`：

```cmake
add_executable(Chapter3_RenderingDemo
    main.cpp
    RenderingDemo.h
    RenderingDemo.cpp
)

target_link_libraries(Chapter3_RenderingDemo PRIVATE
    ${VTK_LIBRARIES}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
)

set_target_properties(Chapter3_RenderingDemo PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

deploy_qt_for_target(Chapter3_RenderingDemo)
```

写入 `src/Chapter3_Rendering/main.cpp`（模式同前，类名替换为 `RenderingDemo`）。

写入 `src/Chapter3_Rendering/RenderingDemo.h`：

```cpp
#pragma once

#include <QMainWindow>
#include <QCheckBox>
#include <QLabel>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkActor.h>
#include <vtkCamera.h>

class QVTKOpenGLNativeWidget;

// 四视口渲染对比：俯视 / 正视 / 侧视 / 3D 透视
// 每个视口有独立的 vtkRenderer + vtkCamera

class RenderingDemo : public QMainWindow
{
    Q_OBJECT

public:
    explicit RenderingDemo(QWidget* parent = nullptr);
    ~RenderingDemo() override = default;

private:
    void setupUI();
    void setupPipelines();
    void syncCameras();
    void resetAllCameras();

    // 4 个渲染器 + 相机
    vtkSmartPointer<vtkRenderer> m_rendererTop;       // 俯视 (XY)
    vtkSmartPointer<vtkRenderer> m_rendererFront;     // 正视 (XZ)
    vtkSmartPointer<vtkRenderer> m_rendererSide;      // 侧视 (YZ)
    vtkSmartPointer<vtkRenderer> m_renderer3D;        // 3D 透视

    // 4 个渲染窗口
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_rwTop;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_rwFront;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_rwSide;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_rw3D;

    // 共享 Actor（同一个 Actor 添加到 4 个 Renderer）
    vtkSmartPointer<vtkActor> m_actor;

    // 4 个 VTK Widget
    QVTKOpenGLNativeWidget* m_vtkTop;
    QVTKOpenGLNativeWidget* m_vtkFront;
    QVTKOpenGLNativeWidget* m_vtkSide;
    QVTKOpenGLNativeWidget* m_vtk3D;

    // 控制
    QCheckBox* m_syncCheckBox;
};
```

写入 `src/Chapter3_Rendering/RenderingDemo.cpp`：

```cpp
#include "RenderingDemo.h"

#include <QVTKOpenGLNativeWidget.h>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkConeSource.h>
#include <vtkCubeSource.h>
#include <vtkDataSetMapper.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkAppendFilter.h>

// ---- 构造函数 ----

RenderingDemo::RenderingDemo(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Chapter 4 — VTK 渲染模型 (多视口)");
    resize(1400, 900);
    setupUI();
    setupPipelines();
}

// ---- UI ----

void RenderingDemo::setupUI()
{
    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    // 顶部控制栏
    auto* topBar = new QHBoxLayout();

    auto* titleLabel = new QLabel(
        "四视口对比：俯视(XY) | 正视(XZ) | 侧视(YZ) | 3D透视\n"
        "每个视口有独立的 Camera，同一 Actor 共享渲染", this);
    titleLabel->setWordWrap(true);
    topBar->addWidget(titleLabel);

    topBar->addStretch();

    m_syncCheckBox = new QCheckBox("同步旋转 (Sync)", this);
    m_syncCheckBox->setChecked(false);
    topBar->addWidget(m_syncCheckBox);

    auto* resetBtn = new QPushButton("重置所有相机", this);
    connect(resetBtn, &QPushButton::clicked, this, &RenderingDemo::resetAllCameras);
    topBar->addWidget(resetBtn);

    mainLayout->addLayout(topBar);

    // 2x2 视口网格
    auto* grid = new QGridLayout();

    // 俯视 (左上) — XY 平面
    auto* topGroup = new QWidget(this);
    auto* topLayout = new QVBoxLayout(topGroup);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(new QLabel("俯视 (XY) - 平行投影", this));
    m_vtkTop = new QVTKOpenGLNativeWidget(this);
    topLayout->addWidget(m_vtkTop);
    grid->addWidget(topGroup, 0, 0);

    // 正视 (右上) — XZ 平面
    auto* frontGroup = new QWidget(this);
    auto* frontLayout = new QVBoxLayout(frontGroup);
    frontLayout->setContentsMargins(0, 0, 0, 0);
    frontLayout->addWidget(new QLabel("正视 (XZ) - 平行投影", this));
    m_vtkFront = new QVTKOpenGLNativeWidget(this);
    frontLayout->addWidget(m_vtkFront);
    grid->addWidget(frontGroup, 0, 1);

    // 侧视 (左下) — YZ 平面
    auto* sideGroup = new QWidget(this);
    auto* sideLayout = new QVBoxLayout(sideGroup);
    sideLayout->setContentsMargins(0, 0, 0, 0);
    sideLayout->addWidget(new QLabel("侧视 (YZ) - 平行投影", this));
    m_vtkSide = new QVTKOpenGLNativeWidget(this);
    sideLayout->addWidget(m_vtkSide);
    grid->addWidget(sideGroup, 1, 0);

    // 3D 透视 (右下)
    auto* d3Group = new QWidget(this);
    auto* d3Layout = new QVBoxLayout(d3Group);
    d3Layout->setContentsMargins(0, 0, 0, 0);
    d3Layout->addWidget(new QLabel("3D 透视 - 自由旋转", this));
    m_vtk3D = new QVTKOpenGLNativeWidget(this);
    d3Layout->addWidget(m_vtk3D);
    grid->addWidget(d3Group, 1, 1);

    mainLayout->addLayout(grid, 1);
}

// ---- 管线 ----

void RenderingDemo::setupPipelines()
{
    // 构造一个复合几何体（球+锥+方块），比单个球更有辨识度
    auto sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetRadius(2.0);
    sphere->SetCenter(-2, 0, 0);
    sphere->SetPhiResolution(32);
    sphere->SetThetaResolution(32);

    auto cone = vtkSmartPointer<vtkConeSource>::New();
    cone->SetRadius(1.5);
    cone->SetHeight(4.0);
    cone->SetCenter(0, 0, 1);
    cone->SetResolution(32);

    auto cube = vtkSmartPointer<vtkCubeSource>::New();
    cube->SetCenter(2, 0, -0.5);
    cube->SetXLength(2);
    cube->SetYLength(2);
    cube->SetZLength(2);

    auto append = vtkSmartPointer<vtkAppendFilter>::New();
    append->AddInputConnection(sphere->GetOutputPort());
    append->AddInputConnection(cone->GetOutputPort());
    append->AddInputConnection(cube->GetOutputPort());

    auto mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputConnection(append->GetOutputPort());

    // 共享 Actor
    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(mapper);
    m_actor->GetProperty()->SetColor(0.3, 0.6, 0.9);
    m_actor->GetProperty()->SetSpecular(0.3);
    m_actor->GetProperty()->SetSpecularPower(20);

    // 背景色
    double bg[3] = {0.18, 0.18, 0.22};

    // --- 俯视 (XY): 沿 +Z 方向看 ---
    m_rendererTop = vtkSmartPointer<vtkRenderer>::New();
    m_rendererTop->SetBackground(bg);
    m_rendererTop->AddActor(m_actor);
    m_rwTop = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_rwTop->AddRenderer(m_rendererTop);
    m_vtkTop->setRenderWindow(m_rwTop);

    // --- 正视 (XZ): 沿 -Y 方向看 ---
    m_rendererFront = vtkSmartPointer<vtkRenderer>::New();
    m_rendererFront->SetBackground(bg);
    m_rendererFront->AddActor(m_actor);
    m_rwFront = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_rwFront->AddRenderer(m_rendererFront);
    m_vtkFront->setRenderWindow(m_rwFront);

    // --- 侧视 (YZ): 沿 -X 方向看 ---
    m_rendererSide = vtkSmartPointer<vtkRenderer>::New();
    m_rendererSide->SetBackground(bg);
    m_rendererSide->AddActor(m_actor);
    m_rwSide = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_rwSide->AddRenderer(m_rendererSide);
    m_vtkSide->setRenderWindow(m_rwSide);

    // --- 3D 透视 ---
    m_renderer3D = vtkSmartPointer<vtkRenderer>::New();
    m_renderer3D->SetBackground(bg);
    m_renderer3D->AddActor(m_actor);
    m_rw3D = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_rw3D->AddRenderer(m_renderer3D);
    m_vtk3D->setRenderWindow(m_rw3D);

    // 设置各视口相机
    resetAllCameras();
}

// ---- 相机控制 ----

void RenderingDemo::resetAllCameras()
{
    // 先全部 Reset 获得初始包围盒
    m_rendererTop->ResetCamera();
    m_rendererFront->ResetCamera();
    m_rendererSide->ResetCamera();
    m_renderer3D->ResetCamera();

    // 俯视：沿 +Z 俯视 XY 平面
    auto camTop = m_rendererTop->GetActiveCamera();
    camTop->SetParallelProjection(true);
    camTop->SetPosition(0, 0, 1);
    camTop->SetFocalPoint(0, 0, 0);
    camTop->SetViewUp(0, 1, 0);

    // 正视：沿 -Y 看 XZ 平面
    auto camFront = m_rendererFront->GetActiveCamera();
    camFront->SetParallelProjection(true);
    camFront->SetPosition(0, -1, 0);
    camFront->SetFocalPoint(0, 0, 0);
    camFront->SetViewUp(0, 0, 1);

    // 侧视：沿 -X 看 YZ 平面
    auto camSide = m_rendererSide->GetActiveCamera();
    camSide->SetParallelProjection(true);
    camSide->SetPosition(-1, 0, 0);
    camSide->SetFocalPoint(0, 0, 0);
    camSide->SetViewUp(0, 0, 1);

    // 3D 透视
    auto cam3D = m_renderer3D->GetActiveCamera();
    cam3D->SetParallelProjection(false);
    cam3D->SetPosition(8, -6, 6);
    cam3D->SetFocalPoint(0, 0, 0);
    cam3D->SetViewUp(0, 0, 1);

    m_rwTop->Render();
    m_rwFront->Render();
    m_rwSide->Render();
    m_rw3D->Render();
}
```

- [ ] **Step 2: 更新顶层 CMakeLists.txt 并构建**

在 `CMakeLists.txt` 末尾追加：

```cmake
add_subdirectory(src/Chapter3_Rendering)
```

```bash
cmake --build build --target Chapter3_RenderingDemo --config Debug
```

预期：四视口显示，俯视/正视/侧视为平行投影（无透视），3D 视口为透视投影，同一数据共享渲染。

- [ ] **Step 3: Commit**

```bash
git add src/Chapter3_Rendering/ CMakeLists.txt
git commit -m "feat(Ch3): add 4-viewport rendering demo"
```

---

### Task 7: Ch3 — 添加光照切换 + 笔记

**Files:**
- Modify: `src/Chapter3_Rendering/RenderingDemo.h` — 添加光照控制成员
- Modify: `src/Chapter3_Rendering/RenderingDemo.cpp` — 添加光照切换 UI 和回调
- Create: `docs/superpowers/notes/Chapter3-Rendering.md`

- [ ] **Step 1: 添加光照切换功能**

在 RenderingDemo.h 中新增成员：

```cpp
#include <QComboBox>
QComboBox* m_lightCombo;
private slots:
    void onLightChanged(int index);
```

在 setupUI 的 topBar 中添加：

```cpp
topBar->addWidget(new QLabel("光照：", this));
m_lightCombo = new QComboBox(this);
m_lightCombo->addItems({"单光源", "双光源", "无光照 (Flat)"});
connect(m_lightCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &RenderingDemo::onLightChanged);
topBar->addWidget(m_lightCombo);
```

在 RenderingDemo.cpp 中追加回调：

```cpp
void RenderingDemo::onLightChanged(int index)
{
    // 移除所有旧光源
    for (auto* r : {m_rendererTop.Get(), m_rendererFront.Get(),
                    m_rendererSide.Get(), m_renderer3D.Get()}) {
        r->RemoveAllLights();
    }

    switch (index) {
    case 0: { // 单光源
        auto light = vtkSmartPointer<vtkLight>::New();
        light->SetPosition(10, 10, 10);
        light->SetIntensity(1.0);
        for (auto* r : {m_rendererTop.Get(), m_rendererFront.Get(),
                        m_rendererSide.Get(), m_renderer3D.Get()}) {
            r->AddLight(light);
        }
        break;
    }
    case 1: { // 双光源（Key + Fill）
        auto key = vtkSmartPointer<vtkLight>::New();
        key->SetPosition(10, 10, 10);
        key->SetIntensity(0.8);
        auto fill = vtkSmartPointer<vtkLight>::New();
        fill->SetPosition(-5, -5, 3);
        fill->SetIntensity(0.4);
        for (auto* r : {m_rendererTop.Get(), m_rendererFront.Get(),
                        m_rendererSide.Get(), m_renderer3D.Get()}) {
            r->AddLight(key);
            r->AddLight(fill);
        }
        break;
    }
    case 2: // 无光照 — VTK 自动添加 headlight (默认行为，已在管线中)
        break;
    }

    m_rwTop->Render();
    m_rwFront->Render();
    m_rwSide->Render();
    m_rw3D->Render();
}
```

增加 include：`#include <vtkLight.h>`

- [ ] **Step 2: 构建验证**

```bash
cmake --build build --target Chapter3_RenderingDemo --config Debug
```

预期：切换光照模式，四视口同步变化。

- [ ] **Step 3: 写笔记**

写入 `docs/superpowers/notes/Chapter3-Rendering.md`：

```markdown
# Chapter 4 — VTK 渲染模型

## 核心概念

VTK 的渲染层次：

```
vtkRenderWindow        ← 窗口系统对接 (Qt/GLFW/...)
  └── vtkRenderer      ← 场景管理 (Actor 集合 + Camera + Light)
        ├── vtkActor   ← 渲染对象 (Mapper + Property + Transform)
        │     ├── vtkMapper       ← 数据→图形映射
        │     ├── vtkProperty     ← 材质/颜色/光照属性
        │     └── vtkTransform    ← 位置/旋转/缩放
        ├── vtkCamera  ← 视角控制
        └── vtkLight   ← 光源
```

RenderWindow 可以包含多个 Renderer（每个 Renderer 有自己的 Viewport），每个 Renderer 包含多个 Actor。

## 关键 API 速查

| 类 | 关键方法 |
|----|---------|
| vtkRenderer | AddActor(), RemoveAllViewProps(), SetBackground(), ResetCamera() |
| vtkRenderWindow | AddRenderer(), Render() |
| vtkCamera | SetPosition(), SetFocalPoint(), SetViewUp(), SetParallelProjection(), Zoom(), Roll(), Azimuth(), Elevation() |
| vtkLight | SetPosition(), SetIntensity(), SetLightType(), SwitchOn/Off() |
| vtkProperty | SetColor(), SetSpecular(), SetSpecularPower(), SetOpacity(), SetRepresentation() |

## 平行投影 vs 透视投影

- **平行投影**（ParallelProjectionOn）：投影线平行，无近大远小。工程图/三视图使用。
- **透视投影**（ParallelProjectionOff）：投影线汇聚于视点，有近大远小。真实 3D 场景使用。

## Camera 的三个关键向量

```
Position (相机位置) ──→  LookAt/FocalPoint (看向哪里)
                          ↑
                      ViewUp (头顶方向)
```

## 踩坑记录

（学习中遇到的坑和解决方案）

## 原理追问

1. 同一个 Actor 添加到 4 个 Renderer，VTK 内部是如何处理的？
   - 答：Mapper 的数据只上传一次到 GPU，4 个 Renderer 共享引用。最终渲染时每个 Renderer 独立执行自己的渲染 pass，使用各自的 Camera 矩阵计算 MVP 变换。

2. 为什么平行投影下物体的"大小"不变？
   - 答：平行投影的投影矩阵 W 分量恒为 1（正交矩阵），透视除法的缩放因子不随深度变化。透视投影的 W 分量 = -Z（相机空间深度），透视除法产生近大远小效果。
```

- [ ] **Step 4: Commit**

```bash
git add src/Chapter3_Rendering/ docs/superpowers/notes/Chapter3-Rendering.md
git commit -m "feat(Ch3): add lighting switch + rendering model notes"
```

---

### Task 8: 阶段一收尾 — 更新进度

**Files:**
- Modify: `docs/superpowers/PROGRESS.md`

- [ ] **Step 1: 更新 PROGRESS.md**

将 Ch1/Ch2/Ch3 的状态从 ⬜ 改为 ✅，更新"当前任务"指向阶段二。

- [ ] **Step 2: 最终 Commit**

```bash
git add docs/superpowers/PROGRESS.md
git commit -m "milestone(M1): complete Stage 1 — VTK pipeline foundation"
```

---

## 前置条件清单

开始执行前确保：
- [ ] VS Code 可以成功构建 Chapter1_BaseVTKDemo（验证环境正常）
- [ ] vcpkg 已安装 vtk[core,qt,opengl,openmp]，Qt6 可用
- [ ] CMake Preset 选择正确（Debug 或 Release）
