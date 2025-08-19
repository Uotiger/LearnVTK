//
// Created by 15206 on 2025/8/19.
//

#ifndef LEARNVTK_MAINRENDERWINDOW_H
#define LEARNVTK_MAINRENDERWINDOW_H

#include <QWidget>
#include <vtkActor.h>
#include <vtkSmartPointer.h>



class vtkAxesActor; // 坐标轴类
class vtkOrientationMarkerWidget; // 方向标记部件

QT_BEGIN_NAMESPACE

namespace Ui {
    class MainRenderWindow;
}

QT_END_NAMESPACE

class MainRenderWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainRenderWindow(QWidget* parent = nullptr);
    ~MainRenderWindow() override;

public:
    bool isClearRenderData();

private:
    void init();
    void initConnects();
    // 初始化坐标方向部件
    void initAxesMarkerWidget();

private slots:
    void onInitWindows();
    void onClearRenderData();
    void onSwitchClearPreData(Qt::CheckState state);
    void onSwitchAxesState(Qt::CheckState state);
    void onLoadFile();
    void onCreateMulFaceCylinder();

private:
    Ui::MainRenderWindow* ui;
    vtkSmartPointer<vtkRenderer> mMainRender;
    std::vector<vtkSmartPointer<vtkActor>> mActors; // 管理所有actor
    bool mRenderAndClear = true;    //创建对象时是否清理之前数据
    vtkSmartPointer<vtkOrientationMarkerWidget> mAxesMarkerWidget;
};


#endif //LEARNVTK_MAINRENDERWINDOW_H
