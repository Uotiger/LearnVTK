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
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>

MainRenderWindow::MainRenderWindow(QWidget* parent) :
    QWidget(parent), ui(new Ui::MainRenderWindow)
{
    ui->setupUi(this);
    onInitWindows();
    initConnects();
}

MainRenderWindow::~MainRenderWindow()
{
    delete ui;
}

void MainRenderWindow::initConnects()
{
    connect(ui->btn_InitWindow, &QPushButton::clicked, this, &MainRenderWindow::onInitWindows);
    connect(ui->btn_clearRenderData, &QPushButton::clicked, this, &MainRenderWindow::onClearRenderData);
    connect(ui->btn_loadFile, &QPushButton::clicked, this, &MainRenderWindow::onLoadFile);
    connect(ui->checkBox_clearPre, &QCheckBox::checkStateChanged, this, &MainRenderWindow::onSwitchClearPreData);

    connect(ui->btn_mulFaceCylinder, &QPushButton::clicked, this, &MainRenderWindow::onCreateMulFaceCylinder);
}

void MainRenderWindow::onInitWindows()
{
    if (mMainRender)
        return;
    mMainRender = vtkSmartPointer<vtkRenderer>::New();
    mMainRender->SetBackground(0.2,0.3,0.3);
    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    renderWindow->AddRenderer(mMainRender);

    ui->vtkWidget->setRenderWindow(renderWindow);
}

void MainRenderWindow::onClearRenderData()
{
    for (auto& actor : mActors) {
        mMainRender->RemoveActor(actor);
    }
    mActors.clear();
    ui->vtkWidget->renderWindow()->Render();
}

void MainRenderWindow::onSwitchClearPreData(Qt::CheckState state)
{

}

void MainRenderWindow::onLoadFile()
{
}

void MainRenderWindow::onCreateMulFaceCylinder()
{
    //onClearRenderData();
    static int face = 8;
    vtkNew<vtkCylinderSource> cylinder;
    cylinder->SetResolution(face--);
    vtkNew<vtkPolyDataMapper> cylinderMapper;
    cylinderMapper->SetInputConnection(cylinder->GetOutputPort());
    vtkNew<vtkActor> cylinderActor;
    cylinderActor->SetMapper(cylinderMapper);
    cylinderActor->RotateX(30.0);
    cylinderActor->RotateY(-45.0);
    double offset = 2.0 * mActors.size();
    cylinderActor->SetPosition(offset, 0, 0);
    mMainRender->AddActor(cylinderActor);
    mActors.push_back(cylinderActor);
    mMainRender->ResetCamera();


    // vtkCamera* camera = mMainRender->GetActiveCamera();
    // camera->SetPosition(camera->GetPosition()[0]+offset, camera->GetPosition()[1], camera->GetPosition()[2]);    // 相机位置
     ui->vtkWidget->renderWindow()->Render();
}
