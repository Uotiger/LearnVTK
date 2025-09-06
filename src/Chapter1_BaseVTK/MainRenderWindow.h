//
// Created by 15206 on 2025/8/19.
//

#ifndef LEARNVTK_MAINRENDERWINDOW_H
#define LEARNVTK_MAINRENDERWINDOW_H

#include <memory>
#include <QWidget>
#include <vtkActor.h>
#include <vtkSmartPointer.h>

#include "CGNSDataParser.h"

class QTreeWidgetItem;
class vtkAxesActor; // 坐标轴类
class vtkOrientationMarkerWidget; // 方向标记部件
class vtkScalarBarWidget; // 标量条小部件

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
    // 初始化标量条小部件
    void initScalarsBarWidget();
    // 初始化部件树
    void initComponentsTree();
    // 填充部件树
    void populateTreeWidget();
    // 创建所有部件
    void createAllBlockActors();
    // 创建部件
    void createBlockActor(int blockId);

private slots:
    void onInitWindows();
    void onClearRenderData();
    void onSwitchClearPreData(Qt::CheckState state);
    void onSwitchAxesState(Qt::CheckState state);
    void onLoadFile();
    void onCreateMulFaceCylinder();
    void onNodeStateChanged(std::vector<CGNSNode> nodeVec);
    void onShowCloud();

private:
    Ui::MainRenderWindow* ui;
    vtkSmartPointer<vtkRenderer> mMainRender;
    std::vector<vtkSmartPointer<vtkProp>> mActors; // 管理所有actor
    bool mRenderAndClear = true; //创建对象时是否清理之前数据

    //小部件
    vtkSmartPointer<vtkOrientationMarkerWidget> mAxesMarkerWidget;
    vtkSmartPointer<vtkScalarBarWidget> mScalarBarWidgets;

    //cgns
    std::unique_ptr<CGNSDataParser> mParser;
    std::map<int, vtkSmartPointer<vtkProp>> mBlockActors;
    std::map<int, vtkSmartPointer<vtkProp>> mBlockCloudActors;
    std::map<QString, int> mPathToBlockIdMap;
};


#endif //LEARNVTK_MAINRENDERWINDOW_H
