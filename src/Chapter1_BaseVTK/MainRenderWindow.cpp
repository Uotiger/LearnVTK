//
// Created by 15206 on 2025/8/19.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainRenderWindow.h" resolved

#include "MainRenderWindow.h"
#include "ui_MainRenderWindow.h"
#include <vtkActor.h>
#include <vtkCamera.h>
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
#include <vtkOrientationMarkerWidget.h>
#include <vtkCaptionActor2D.h>
#include <vtkOrientationMarkerWidget.h>

MainRenderWindow::MainRenderWindow(QWidget* parent) :
    QWidget(parent), ui(new Ui::MainRenderWindow) {
    ui->setupUi(this);
    this->setWindowTitle("LeranVTK_BaseVTK");
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
    initConnects();
}

void MainRenderWindow::initConnects() {
    connect(ui->btn_InitWindow, &QPushButton::clicked, this, &MainRenderWindow::onInitWindows);
    connect(ui->btn_clearRenderData, &QPushButton::clicked, this, &MainRenderWindow::onClearRenderData);
    connect(ui->btn_loadFile, &QPushButton::clicked, this, &MainRenderWindow::onLoadFile);
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
    mAxesMarkerWidget->SetViewport(0, 0, 0.15,0.15);
    // 坐标轴对象占满SetViewport所设置的大小，默认居中只有1/3
    mAxesMarkerWidget->SetZoom(1.8);

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
    for (auto& actor : mActors) {
        mMainRender->RemoveActor(actor);
    }
    mActors.clear();
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
}

void MainRenderWindow::onCreateMulFaceCylinder() {
    if (isClearRenderData()) {
        onClearRenderData();
    }
    int faces = ui->spinBox_faces->value();

    // 原始几何数据源，此类内置自动生成原始几何数据
    vtkNew<vtkCylinderSource> cylinder;
    // 设置圆柱面数
    cylinder->SetResolution(faces);

    // 数据映射器 VAO + VBO 绑定（将数据转换为 GPU 可绘制格式）
    vtkNew<vtkPolyDataMapper> cylinderMapper;
    //将原始几何数据传递给数据映射器
    cylinderMapper->SetInputConnection(cylinder->GetOutputPort());

    // 创建渲染实体 （模型矩阵 + 材质属性）
    vtkNew<vtkActor> cylinderActor;
    // 关联数据映射器，
    cylinderActor->SetMapper(cylinderMapper);
    // 模型绕X轴旋转30度
    cylinderActor->RotateX(30.0);
    // 模型绕Y轴旋转30度
    cylinderActor->RotateY(-45.0);
    // 设置渲染实体颜色
    //cylinderActor->GetProperty()->SetColor(26.0/255.0, 51.0 / 255.0, 102 / 255.0);
    //通过内置的颜色名称设置染色
    vtkNew<vtkNamedColors> colors;
    cylinderActor->GetProperty()->SetColor(colors->GetColor4d("Tomato").GetData());

    //如果多个actor 则水平平铺当前 Actor
    double offset = 2.0 * mActors.size();
    cylinderActor->SetPosition(offset, 0, 0);

    // 渲染器添加actor(添加到渲染场景中)
    mMainRender->AddActor(cylinderActor);

    mActors.push_back(cylinderActor);

    // 重置相机位置
    mMainRender->ResetCamera();
    // 触发渲染
    ui->vtkWidget->renderWindow()->Render();
}
