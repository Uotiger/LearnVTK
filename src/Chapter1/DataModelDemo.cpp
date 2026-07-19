#include "DataModelDemo.h"

#include <QVTKOpenGLNativeWidget.h>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkTetra.h>
#include <vtkHexahedron.h>
#include <vtkUnstructuredGrid.h>

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

// ============================================================
// Task 2: 实现 5 个 build 方法
// ============================================================
//
// TODO: 你需要自己实现以下方法。参考计划文件：
// docs/superpowers/plans/2026-07-11-stage1-pipeline-foundation.md
// 的 Task 2 部分。
//
// 每个方法：
// 1. 构造一个具体的 vtkDataSet 子类
// 2. 设置几何数据（点坐标、维度等）
// 3. 添加属性数据（Point Data 或 Cell Data）
// 4. 赋值给 m_currentDataset
//
// buildImageData()           — 规则网格，10×8×6 个点
// buildRectilinearGrid()     — 矩形网格，非均匀间距
// buildStructuredGrid()      — 结构化网格，带正弦扰动
// buildPolyData()            — 多边形数据，手动构造六面体表面
// buildUnstructuredGrid()    — 非结构化网格，四面体+六面体
// ============================================================

void DataModelDemo::buildImageData()
{
    // TODO: 实现 ImageData 构造
    // 参考计划 Task 2 Step 1
    // 提示：SetDimensions(10, 8, 6), SetOrigin(), SetSpacing()
    // 添加 vtkFloatArray 标量数据到 PointData

    auto image = vtkSmartPointer<vtkImageData>::New();
    image->SetDimensions(10, 8, 6);
    image->SetOrigin(0.0,0.0,0.0);  // 设置原点
    image->SetSpacing(1.0,1.0,1.0); // 设置采样间距

    // 创建标量数据
    auto scalars = vtkSmartPointer<vtkFloatArray>::New();
    scalars->SetName("Distance"); // 设置标量名称
    scalars->SetNumberOfComponents(1); // 单分量标量
    int dims[3];
    image->GetDimensions(dims); // 获取维度
    for (int k = 0; k < dims[2]; ++k) {
        for (int j = 0; j < dims[1]; ++j) {
            for (int i = 0; i < dims[0]; ++i) {
                double x = i - dims[0] / 2.0;
                double y = j - dims[1] / 2.0;
                double z = k - dims[2] / 2.0;
                scalars->InsertNextValue(std::sqrt(x*x + y*y + z*z)); // 插入标量值
            }
        }
    }
    image->GetPointData()->SetScalars(scalars); // 设置标量数据
    m_currentDataset = image;
}

void DataModelDemo::buildRectilinearGrid()
{
    // TODO: 实现 RectilinearGrid 构造
    // 参考计划 Task 2 Step 2
    auto grid = vtkSmartPointer<vtkRectilinearGrid>::New();
    // ... 你的代码 ...
    m_currentDataset = grid;
}

void DataModelDemo::buildStructuredGrid()
{
    // TODO: 实现 StructuredGrid 构造
    // 参考计划 Task 2 Step 3
    auto grid = vtkSmartPointer<vtkStructuredGrid>::New();
    // ... 你的代码 ...
    m_currentDataset = grid;
}

void DataModelDemo::buildPolyData()
{
    // TODO: 实现 PolyData 构造
    // 参考计划 Task 2 Step 4
    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    // ... 你的代码 ...
    m_currentDataset = polyData;
}

void DataModelDemo::buildUnstructuredGrid()
{
    // TODO: 实现 UnstructuredGrid 构造
    // 参考计划 Task 2 Step 5
    auto ug = vtkSmartPointer<vtkUnstructuredGrid>::New();
    // ... 你的代码 ...
    m_currentDataset = ug;
}

// ---- 切换逻辑 ----

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
