//
// Created by 15206 on 2025/8/19.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainRenderWindow.h" resolved

#include "MainRenderWindow.h"
#include "NodeTreeWidget.h"
#include "ui_MainRenderWindow.h"
#include <vtkActor.h>
#include <vtkCylinderSource.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkAxesActor.h>
#include <vtkTextProperty.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>
#include <vtkLookupTable.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkCaptionActor2D.h>
#include <vtkCGNSReader.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkDataSetMapper.h>
#include "CGNSDataParser.h"
#include <QFileDialog>
#include <QFile>
#include <vtkCellData.h>
#include <vtkPointData.h>

MainRenderWindow::MainRenderWindow(QWidget* parent) :
    QWidget(parent), ui(new Ui::MainRenderWindow) {
    ui->setupUi(this);
    this->setWindowTitle("LeranVTK_BaseVTK");
    move(300, 100);
    init();;
}

MainRenderWindow::~MainRenderWindow() {
    delete ui;
}

bool MainRenderWindow::isClearRenderData() {
    return mRenderAndClear;
}

void MainRenderWindow::init() {
    onInitWindows();
    initAxesMarkerWidget();
    initScalarsBarWidget();
    initComponentsTree();
    initConnects();
}

void MainRenderWindow::initConnects() {
    connect(ui->btn_InitWindow, &QPushButton::clicked, this, &MainRenderWindow::onInitWindows);
    connect(ui->btn_clearRenderData, &QPushButton::clicked, this, &MainRenderWindow::onClearRenderData);
    connect(ui->btn_loadFile, &QPushButton::clicked, this, &MainRenderWindow::onLoadFile);
    connect(ui->btn_showCloud, &QPushButton::clicked, this, &MainRenderWindow::onShowCloud);

    connect(ui->checkBox_clearPre, &QCheckBox::checkStateChanged, this, &MainRenderWindow::onSwitchClearPreData);
    connect(ui->checkBox_Axes, &QCheckBox::checkStateChanged, this, &MainRenderWindow::onSwitchAxesState);

    connect(ui->btn_mulFaceCylinder, &QPushButton::clicked, this, &MainRenderWindow::onCreateMulFaceCylinder);
}

void MainRenderWindow::initAxesMarkerWidget() {
    if (mAxesMarkerWidget) {
        return;
    }
    // 实例化坐标轴对象
    auto mAxesActor = vtkSmartPointer<vtkAxesActor>::New();
    vtkNew<vtkNamedColors> colors;
    // 设置坐标轴文本颜色
    mAxesActor->GetXAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(colors->GetColor3d("Red").GetData());
    mAxesActor->GetYAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(colors->GetColor3d("Green").GetData());
    mAxesActor->GetZAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(colors->GetColor3d("Blue").GetData());

    // 实例方向标记部件对象（拥有独立的渲染器，采用正交投影，固定在屏幕某个位置，可以叠加到主窗口）
    // 绝大部分用于固定坐标轴实例对象
    mAxesMarkerWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();

    // 将坐标轴对象作为标记
    mAxesMarkerWidget->SetOrientationMarker(mAxesActor);
    // 关联到qt的vtk窗口
    mAxesMarkerWidget->SetInteractor(ui->vtkWidget->interactor());
    // 启用部件 (必须早于SetInteractive，否则SetInteractive不生效)
    mAxesMarkerWidget->EnabledOn();
    // 禁止交互
    mAxesMarkerWidget->SetInteractive(0);
    // 固定在窗口
    mAxesMarkerWidget->SetViewport(0, 0, 0.15, 0.15);
    // 坐标轴对象占满SetViewport所设置的大小，默认居中只有1/3
    mAxesMarkerWidget->SetZoom(1.8);
}

void MainRenderWindow::initScalarsBarWidget() {
    if (mScalarBarWidgets) {
        return;
    }

    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetHueRange(0.6667, 0.0); // 蓝色到红色
    lut->SetNumberOfTableValues(256);
    lut->Build();

    // 创建标量条
    vtkSmartPointer<vtkScalarBarActor> scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
    scalarBar->SetLookupTable(lut);
    scalarBar->SetTitle("Test");
    scalarBar->SetNumberOfLabels(10);

    // 创建标量条小部件
    mScalarBarWidgets = vtkSmartPointer<vtkScalarBarWidget>::New();
    mScalarBarWidgets->SetInteractor(ui->vtkWidget->interactor());
    mScalarBarWidgets->SetScalarBarActor(scalarBar);
    mScalarBarWidgets->On();

    vtkTextProperty* labelTextProp = scalarBar->GetLabelTextProperty();
    labelTextProp->SetFontSize(8); // 设置标签字体大小
    // labelTextProp->SetBold(0);    // 可以取消加粗
    // labelTextProp->SetItalic(0);  // 可以取消斜体
    // labelTextProp->SetShadow(0);  // 可以取消阴影

    vtkTextProperty* titleTextProp = scalarBar->GetTitleTextProperty();
    titleTextProp->SetFontSize(10); // 设置标题字体大小，通常比标签稍大
    // titleTextProp->SetBold(0);

    // 2. 调整标量条本身的宽度和高度（相对于窗口的比例）
    scalarBar->SetWidth(0.08); // 调小宽度，默认可能较大
    scalarBar->SetHeight(0.4); // 调小高度，默认可能较大

    // 3. 【关键】调整标量条内部组件的大小，这直接影响刻度的粗细和间隔
    scalarBar->SetBarRatio(0.2); // 控制颜色条宽度与标签区域宽度的比例，调小会使颜色条变细
    scalarBar->SetAnnotationLeaderPadding(2.0); // 注解引导线填充
    labelTextProp->Modified();
    titleTextProp->Modified();
    scalarBar->Modified();

    ui->vtkWidget->renderWindow()->Render();
}

void MainRenderWindow::initComponentsTree() {
    mParser = std::make_unique<CGNSDataParser>();

    ui->nodeTree->setHeaderLabel("部件树");
    ui->nodeTree->setHeaderLabel("CGNS 结构");
    ui->nodeTree->setColumnCount(1);
    ui->nodeTree->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->nodeTree->setContextMenuPolicy(Qt::CustomContextMenu);

    // 连接信号槽
    connect(ui->nodeTree, &NodeTreeWidget::nodeStateChanged,
            this, &MainRenderWindow::onNodeStateChanged);
}

void MainRenderWindow::populateTreeWidget() {
    ui->nodeTree->clear();
    // 获取解析后的树结构
    CGNSNode rootNode = mParser->getTreeStructure();
    // 归构建树控件
    ui->nodeTree->buildTreeItems(nullptr, rootNode);
    // 展开所有节点
    ui->nodeTree->expandAll();
}

void MainRenderWindow::createAllBlockActors() {
    // 获取解析器中的路径到块ID映射
    mPathToBlockIdMap = mParser->getPathToBlockIdMap();

    // 为每个块创建Actor
    for (const auto& itr : mPathToBlockIdMap) {
        createBlockActor(itr.second);
    }
}

void MainRenderWindow::createBlockActor(int blockId) {
    // 获取数据块
    vtkSmartPointer<vtkDataObject> blockData = mParser->getBlockData(blockId);
    if (!blockData) return;

    vtkDataSet* dataset = vtkDataSet::SafeDownCast(blockData);
    if (!dataset) {
        ui->logWidget->addItem("Block data is not a vtkDataSet blockId:" + QString::number(blockId));
        return;
    }
    // 创建映射器
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputData(vtkDataSet::SafeDownCast(blockData));

    // 创建Actor
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // 设置随机颜色以便区分不同部件
    // vtkNew<vtkNamedColors> colors;
    // double r = static_cast<double>(rand()) / RAND_MAX;
    // double g = static_cast<double>(rand()) / RAND_MAX;
    // double b = static_cast<double>(rand()) / RAND_MAX;
    // actor->GetProperty()->SetColor(r, g, b);

    // 存储Actor
    mBlockActors[blockId] = actor;

    // 添加到渲染器但不立即显示（根据复选框状态决定）
    mMainRender->AddActor(actor);
    actor->SetVisibility(true);
}

void MainRenderWindow::onInitWindows() {
    if (mMainRender)
        return;
    // 实例化一个vtkRenderer对象，用于管理渲染场景。
    // vtkRenderer负责场景中的所有渲染元素，包括相机（vtkCamera）、光照（vtkLight）和Actor（如vtkActor）。
    // 它定义了渲染的视图和场景内容，但不直接处理窗口系统交互。
    mMainRender = vtkSmartPointer<vtkRenderer>::New();
    // 设置渲染场景背景色
    mMainRender->SetBackground(0.2, 0.3, 0.3);
    // 创建一个基于OpenGL的通用渲染窗口。
    // vtkGenericOpenGLRenderWindow是VTK中用于OpenGL渲染的窗口实现，它提供了一个平台无关的接口，
    // 允许VTK在支持OpenGL的环境中进行渲染。特别适用于嵌入到Qt等GUI框架中，
    // 因为它能与外部提供的OpenGL上下文（如Qt的QOpenGLContext）协同工作。
    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    // 将渲染器添加到渲染窗口中。一个渲染窗口可以包含多个渲染器（例如，用于分屏渲染），
    // 但这里只添加一个主渲染器。
    renderWindow->AddRenderer(mMainRender);

    // 将Qt的VTKWidget与VTK渲染窗口关联。
    // 这样，VTK的渲染输出将显示在ui->vtkWidget所代表的Qt窗口部件中。
    ui->vtkWidget->setRenderWindow(renderWindow);
}

void MainRenderWindow::onClearRenderData() {
    for (const auto& actor : mActors) {
        mMainRender->RemoveActor(actor);
    }
    mActors.clear();

    // 移除所有部件Actor
    for (const auto& itr : mBlockActors) {
        mMainRender->RemoveActor(itr.second);
    }
    mBlockActors.clear();

    // 清空树控件
    ui->nodeTree->clear();

    // 清空映射
    mPathToBlockIdMap.clear();
    ui->vtkWidget->renderWindow()->Render();
}

void MainRenderWindow::onSwitchClearPreData(Qt::CheckState state) {
    mRenderAndClear = state == Qt::CheckState::Checked ? true : false;
}

void MainRenderWindow::onSwitchAxesState(Qt::CheckState state) {
    // 设置坐标轴部件显隐使能
    if (mAxesMarkerWidget) {
        mAxesMarkerWidget->SetEnabled(state == Qt::Checked);
        // 触发渲染
        ui->vtkWidget->renderWindow()->Render();
    }
}

void MainRenderWindow::onLoadFile() {
    if (isClearRenderData()) {
        onClearRenderData();
    }
    auto filePath = QFileDialog::getOpenFileName(nullptr, "选择文件",
                                                 "E:/RenderProject/DataFiles", "所有文件 (*.*);;");
    if (filePath.isEmpty()) {
        ui->logWidget->addItem("没有选择文件");
        return;
    }

    ui->logWidget->addItem("加载文件:" + filePath);
    if (!mParser->parseFile(filePath)) {
        ui->logWidget->addItem("文件解析失败");
    }

    // 填充树控件
    populateTreeWidget();
    // 创建所有块的Actor
    createAllBlockActors();
    // 云图选择项填充到ui
    const auto& scalarList = mParser->getScalarDataList();
    ui->cbb_cloud->clear();
    for (const auto& it : scalarList) {
        if (!it.isVector) {
            ui->cbb_cloud->addItem(it.name);
        }
    }

    mMainRender->ResetCamera();
    ui->vtkWidget->renderWindow()->Render();
}

void MainRenderWindow::onCreateMulFaceCylinder() {
    // if (isClearRenderData()) {
    //     onClearRenderData();
    // }
    // int faces = ui->spinBox_faces->value();
    //
    // // 原始几何数据源，此类内置自动生成原始几何数据
    // vtkNew<vtkCylinderSource> cylinder;
    // // 设置圆柱面数
    // cylinder->SetResolution(faces);
    //
    // // 数据映射器 VAO + VBO 绑定（将数据转换为 GPU 可绘制格式）
    // vtkNew<vtkPolyDataMapper> cylinderMapper;
    // //将原始几何数据传递给数据映射器
    // cylinderMapper->SetInputConnection(cylinder->GetOutputPort());
    //
    // // 创建渲染实体 （模型矩阵 + 材质属性）
    // vtkNew<vtkActor> cylinderActor;
    // // 关联数据映射器，
    // cylinderActor->SetMapper(cylinderMapper);
    // // 模型绕X轴旋转30度
    // cylinderActor->RotateX(30.0);
    // // 模型绕Y轴旋转30度
    // cylinderActor->RotateY(-45.0);
    // // 设置渲染实体颜色
    // //cylinderActor->GetProperty()->SetColor(26.0/255.0, 51.0 / 255.0, 102 / 255.0);
    // //通过内置的颜色名称设置染色
    // vtkNew<vtkNamedColors> colors;
    // cylinderActor->GetProperty()->SetColor(colors->GetColor4d("Tomato").GetData());
    //
    // //如果多个actor 则水平平铺当前 Actor
    // double offset = 2.0 * mActors.size();
    // cylinderActor->SetPosition(offset, 0, 0);
    //
    // // 渲染器添加actor(添加到渲染场景中)
    // mMainRender->AddActor(cylinderActor);
    //
    // mActors.push_back(cylinderActor);
    //
    // // 重置相机位置
    // mMainRender->ResetCamera();
    // // 触发渲染
    // ui->vtkWidget->renderWindow()->Render();


    // 创建一个简单的测试几何体
    vtkNew<vtkCylinderSource> cylinder;
    cylinder->SetResolution(8);
    cylinder->Update();

    // 创建标量数据
    vtkNew<vtkDoubleArray> scalars;
    int numPoints = cylinder->GetOutput()->GetNumberOfPoints();
    scalars->SetNumberOfValues(numPoints);
    for (int i = 0; i < numPoints; i++) {
        scalars->SetValue(i, static_cast<double>(i) / numPoints);
    }
    scalars->SetName("TestScalars");

    // 将标量数据添加到几何体
    cylinder->GetOutput()->GetPointData()->SetScalars(scalars);

    // 创建映射器
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(cylinder->GetOutputPort());
    mapper->SetScalarRange(0.0, 1.0);
    mapper->SetLookupTable(mScalarBarWidgets->GetScalarBarActor()->GetLookupTable());
    mapper->ScalarVisibilityOn();

    // 创建Actor
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);

    // 添加到渲染器
    mMainRender->AddActor(actor);
    mMainRender->ResetCamera();
    ui->vtkWidget->renderWindow()->Render();

    ui->logWidget->addItem("测试颜色映射完成");
}

void MainRenderWindow::onNodeStateChanged(std::vector<CGNSNode> nodeVec) {
    for (const auto& node : nodeVec) {
        auto actorIt = mBlockActors.find(node.blockId);
        if (node.type == "DataSet" && actorIt != mBlockActors.end()) {
            actorIt->second->SetVisibility(node.visible);
        }
    }
    ui->vtkWidget->renderWindow()->Render();
}
void MainRenderWindow::onShowCloud() {
    for (const auto& actor : mActors) {
        mMainRender->RemoveActor(actor);
    }
    mActors.clear();

    // 移除所有部件Actor
    for (const auto& itr : mBlockActors) {
        mMainRender->RemoveActor(itr.second);
    }
    mBlockActors.clear();

    // 首先移除之前可能创建的云图Actor
    for (const auto& itr : mBlockCloudActors) {
        mMainRender->RemoveActor(itr.second);
    }
    mBlockCloudActors.clear();

       auto dataName = ui->cbb_cloud->currentText();
    if (dataName.isEmpty()) {
        ui->logWidget->addItem("请选择要显示的云图数据");
        return;
    }

    // 获取标量数据的全局范围
    double globalRange[2] = {VTK_DOUBLE_MAX, VTK_DOUBLE_MIN};
    bool hasValidData = false;

    // 第一次遍历：计算全局范围
    for (const auto& it : mPathToBlockIdMap) {
        vtkSmartPointer<vtkDataArray> scalarData = mParser->getScalarData(it.second, dataName);
        if (scalarData && scalarData->GetNumberOfTuples() > 0) {
            double range[2];
            scalarData->GetRange(range);
            globalRange[0] = std::min(globalRange[0], range[0]);
            globalRange[1] = std::max(globalRange[1], range[1]);
            hasValidData = true;

            ui->logWidget->addItem(QString("数据 %1 范围: [%2, %3], 元组数: %4")
                                  .arg(dataName)
                                  .arg(range[0])
                                  .arg(range[1])
                                  .arg(scalarData->GetNumberOfTuples()));
        }
    }

    if (!hasValidData) {
        ui->logWidget->addItem("未找到有效的标量数据: " + dataName);
        return;
    }

    ui->logWidget->addItem(QString("全局范围: [%1, %2]")
                          .arg(globalRange[0])
                          .arg(globalRange[1]));

    // 更新颜色查找表的范围
    auto lut = mScalarBarWidgets->GetScalarBarActor()->GetLookupTable();
    lut->SetRange(globalRange[0], globalRange[1]);
    lut->Build();

    // 更新标量条标题
    mScalarBarWidgets->GetScalarBarActor()->SetTitle(dataName.toStdString().c_str());

    // 第二次遍历：创建云图Actor
    for (const auto& it : mPathToBlockIdMap) {
        // 获取数据块
        vtkSmartPointer<vtkDataObject> blockData = mParser->getBlockData(it.second);
        if (!blockData) continue;

        vtkDataSet* dataset = vtkDataSet::SafeDownCast(blockData);
        if (!dataset) continue;

        // 获取标量数据
        vtkSmartPointer<vtkDataArray> scalarData = mParser->getScalarData(it.second, dataName);
        if (!scalarData || scalarData->GetNumberOfTuples() == 0) continue;

        // 确定标量数据的类型（点数据还是单元数据）
        bool isPointData = (dataset->GetPointData()->GetArray(dataName.toStdString().c_str()) != nullptr);

        // 打印数据集信息
        ui->logWidget->addItem(QString("块 %1: 点数=%2, 单元数=%3, 数据类型=%4")
                              .arg(it.second)
                              .arg(dataset->GetNumberOfPoints())
                              .arg(dataset->GetNumberOfCells())
                              .arg(isPointData ? "点数据" : "单元数据"));

        // 创建映射器
        vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
        mapper->SetInputData(dataset);

        // 根据数据类型设置标量模式
        if (isPointData) {
            mapper->SetScalarModeToUsePointData();
            ui->logWidget->addItem(QString("块 %1: 使用点数据").arg(it.second));
        } else {
            mapper->SetScalarModeToUseCellData();
            ui->logWidget->addItem(QString("块 %1: 使用单元数据").arg(it.second));
        }


        // 设置标量数组和颜色查找表
        mapper->SelectColorArray(dataName.toStdString().c_str());
        mapper->SetLookupTable(lut);
        mapper->SetScalarRange(globalRange[0], globalRange[1]);
        mapper->ScalarVisibilityOn(); // 确保标量渲染开启
        mapper->SetColorModeToMapScalars();

        if (isPointData) {
            mapper->SetScalarMode(VTK_SCALAR_MODE_USE_POINT_FIELD_DATA);
        } else {
            mapper->SetScalarMode(VTK_SCALAR_MODE_USE_CELL_FIELD_DATA);
        }

        // 检查映射器是否正确设置了标量数据
        if (mapper->GetScalarVisibility()) {
            ui->logWidget->addItem(QString("块 %1: 标量可见性已开启").arg(it.second));
        } else {
            ui->logWidget->addItem(QString("块 %1: 标量可见性未开启").arg(it.second));
        }

        // 检查映射器使用的标量模式
        int scalarMode = mapper->GetScalarMode();
        ui->logWidget->addItem(QString("块 %1: 标量模式=%2").arg(it.second).arg(scalarMode));

        // 检查映射器使用的颜色数组
        const char* colorArray = mapper->GetArrayName();
        if (colorArray) {
            ui->logWidget->addItem(QString("块 %1: 颜色数组=%2").arg(it.second).arg(colorArray));
        } else {
            ui->logWidget->addItem(QString("块 %1: 未设置颜色数组").arg(it.second));
        }

        mapper->Update();

        // 创建Actor
        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);

        // 检查Actor的材质属性
        vtkProperty* property = actor->GetProperty();
        if (property->GetColor()[0] == 1.0 &&
            property->GetColor()[1] == 1.0 &&
            property->GetColor()[2] == 1.0) {
            ui->logWidget->addItem(QString("块 %1: Actor颜色为白色").arg(it.second));
        }

        // 添加到渲染器和存储列表
        mMainRender->AddActor(actor);
        mBlockCloudActors[it.second] = actor;

        // 隐藏对应的网格Actor（如果存在）
        auto blockActorIt = mBlockActors.find(it.second);
        if (blockActorIt != mBlockActors.end()) {
            blockActorIt->second->SetVisibility(false);
        }
    }

    // 重置相机并渲染
    mMainRender->ResetCamera();
    ui->vtkWidget->renderWindow()->Render();

    ui->logWidget->addItem("显示云图: " + dataName);
}