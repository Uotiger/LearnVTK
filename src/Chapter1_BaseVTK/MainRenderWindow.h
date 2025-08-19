//
// Created by 15206 on 2025/8/19.
//

#ifndef LEARNVTK_MAINRENDERWINDOW_H
#define LEARNVTK_MAINRENDERWINDOW_H

#include <QWidget>
#include <vtkNew.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkNamedColors.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkSmartPointer.h>

QT_BEGIN_NAMESPACE

namespace Ui
{
    class MainRenderWindow;
}

QT_END_NAMESPACE

class MainRenderWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainRenderWindow(QWidget* parent = nullptr);
    ~MainRenderWindow() override;
private:
    void initConnects();
private slots:
    void onInitWindows();
    void onClearRenderData();
    void onSwitchClearPreData(Qt::CheckState state);
    void onLoadFile();
    void onCreateMulFaceCylinder();

private:
    Ui::MainRenderWindow* ui;
    vtkSmartPointer<vtkRenderer> mMainRender;
    std::vector<vtkSmartPointer<vtkActor>> mActors; // 管理所有actor
};


#endif //LEARNVTK_MAINRENDERWINDOW_H