#pragma once

#include <QMainWindow>
#include <QComboBox>
#include <QLabel>
#include <vtkSmartPointer.h>
#include <vtkDataSet.h>
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
