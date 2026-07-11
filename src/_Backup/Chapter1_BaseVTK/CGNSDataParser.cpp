//
// Created by lsh on 2025/8/20.
//
#include <QFileInfo>
#include <vtkCGNSReader.h>
#include <vtkInformation.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include "CGNSDataParser.h"

#include <vtkCompositeDataSet.h>

CGNSDataParser::CGNSDataParser(QObject* parent) {
}

bool CGNSDataParser::parseFile(const QString& filePath) {
    if (!QFileInfo::exists(filePath)) {
        qWarning() << "file does not exist :" << filePath;
        return false;
    }

    // 创建cgns读取器 读取文件
    vtkNew<vtkCGNSReader> reader;
    reader->SetFileName(filePath.toStdString().c_str());

    //reader->LoadAllSolutionFieldsOn();
    reader->EnableAllBases();
    reader->EnableAllPointArrays();
    reader->EnableAllCellArrays();
    reader->EnableAllFamilies();
    reader->EnableAllFaceArrays();


    // 打印读取器设置
    qDebug() << "CGNS Reader settings:";
    qDebug() << "  GetLoadMesh:" << reader->GetLoadMesh();
    qDebug() << "  ActiveTimeStep:" << reader->GetLoadBndPatch();


    const char* flowSolutionArrays[] = {
        "Density", "Mach", "Pressure", "Temperature",
        "VelocityX", "VelocityY", "VelocityZ",
        "ThermalConductivity", "ViscosityMolecular",
        "Enthalpy", "SpecificHeatPressure", "THERMX",
        "EnthalpyStagnation", "PressureStagnation",
        "TemperatureStagnation", "VelocitySoundSquared",
        nullptr
    };

    for (int i = 0; flowSolutionArrays[i] != nullptr; ++i) {
        reader->SetPointArrayStatus(flowSolutionArrays[i], 1);
    }

    // 获取文件信息
    reader->UpdateInformation();

    reader->Update();


    // 获取根数据集
    mMultiBlockData = reader->GetOutput();
    if (!mMultiBlockData) {
        qWarning() << "Failed to read CGNS file or file is empty";
        return false;
    }

    // 清空之前的数据
    mRootNode = CGNSNode();
    mScalarDataList.clear();
    mPathToBlockIdMap.clear();
    mPathToBlockIdMap.clear();
    mBlockDataMap.clear();

    // 设置根节点
    mRootNode.name = QFileInfo(filePath).fileName();
    mRootNode.type = "Root";
    mRootNode.visible = true;
    mRootNode.blockId = -1;

    // 递归调用解析数据函数
    qDebug() << "Enabled point arrays:";
    for (int i = 0; i < reader->GetNumberOfPointArrays(); ++i) {
        const char* name = reader->GetPointArrayName(i);
        qDebug() << "  " << name << ":" << reader->GetPointArrayStatus(name);
    }

    qDebug() << "Enabled cell arrays:";
    for (int i = 0; i < reader->GetNumberOfCellArrays(); ++i) {
        const char* name = reader->GetCellArrayName(i);
        qDebug() << "  " << name << ":" << reader->GetCellArrayStatus(name);
    }
    parseMultiBlock(mMultiBlockData, mRootNode, "");
    // 打印解析结果摘要
    qDebug() << "Parsing completed. Found" << mScalarDataList.size() << "scalar data arrays";
    for (const auto& data : mScalarDataList) {
        qDebug() << "  -" << data.name << "(" << data.location << ")";
    }
    return true;
}

const CGNSNode& CGNSDataParser::getTreeStructure() const {
    return mRootNode;
}

const std::list<ScalarDataInfo>& CGNSDataParser::getScalarDataList() const {
    return mScalarDataList;
}

int CGNSDataParser::getBlockIdFromPath(const QString& path) const {
    auto it = mPathToBlockIdMap.find(path);
    return it != mPathToBlockIdMap.end() ? it->second : -1;
}

vtkSmartPointer<vtkDataObject> CGNSDataParser::getBlockData(int blockId) const {
    auto it = mBlockDataMap.find(blockId);
    return it != mBlockDataMap.end() ? it->second : nullptr;
}

vtkSmartPointer<vtkDataArray> CGNSDataParser::getScalarData(int blockId, const QString& dataName) const {
    vtkDataObject *data = getBlockData(blockId);
    if (!data) return nullptr;

    vtkDataSet *dataset = vtkDataSet::SafeDownCast(data);
    if (!dataset) return nullptr;

    // 首先在点数据中查找
    vtkDataArray *array = dataset->GetPointData()->GetArray(dataName.toStdString().c_str());
    if (array) return array;

    // 然后在单元数据中查找
    array = dataset->GetCellData()->GetArray(dataName.toStdString().c_str());
    return array;
}

const std::map<QString, int>& CGNSDataParser::getPathToBlockIdMap() const {
    return mPathToBlockIdMap;
}

void CGNSDataParser::parseMultiBlock(vtkMultiBlockDataSet* mb, CGNSNode& parentNode, const QString& parentPath) {
    if (!mb) {
        qWarning() << "MultiBlockDataSet is null";
        return;
    }

    uint32_t numBlocks = mb->GetNumberOfBlocks();
    qDebug() << "Processing MultiBlock with" << numBlocks << "blocks, parent:" << parentPath;

    for (uint32_t i = 0; i < numBlocks; ++i) {
        auto block = mb->GetBlock(i);
        if (!block) {
            qWarning() << "Block" << i << "is null";
            continue;
        }

        const char* blockName = mb->GetMetaData(i)->Get(vtkCompositeDataSet::NAME());
        QString name = blockName ? QString(blockName) : QString("Block_%1").arg(i);

        qDebug() << "Processing block" << i << "name:" << name << "type:" << block->GetClassName();

        // 创建新节点
        CGNSNode node;
        node.name = name;
        node.visible = true;
        node.blockId = static_cast<int>(mBlockDataMap.size());

        // 创建节点路径
        auto path = createNodePath(parentPath, name);

        // 判断节点类型
        if (vtkMultiBlockDataSet::SafeDownCast(block)) {
            node.type = "Group";
            node.blockId = -1;
            qDebug() << "Block" << name << "is a Group, recursing...";
            // 递归解析子模块
            parseMultiBlock(vtkMultiBlockDataSet::SafeDownCast(block), node, path);
        } else {
            node.type = "DataSet";
            mPathToBlockIdMap[path] = node.blockId;
            mBlockDataMap[node.blockId] = block;
            qDebug() << "Block" << name << "is a DataSet, extracting arrays...";

            // 检查数据集类型和内容
            vtkDataSet* dataset = vtkDataSet::SafeDownCast(block);
            if (dataset) {
                qDebug() << "  Number of points:" << dataset->GetNumberOfPoints();
                qDebug() << "  Number of cells:" << dataset->GetNumberOfCells();
                qDebug() << "  Point data arrays:" << dataset->GetPointData()->GetNumberOfArrays();
                qDebug() << "  Cell data arrays:" << dataset->GetCellData()->GetNumberOfArrays();
                qDebug() << "  Field data arrays:" << dataset->GetFieldData()->GetNumberOfArrays();

                // 列出所有点数据数组名称
                for (int j = 0; j < dataset->GetPointData()->GetNumberOfArrays(); ++j) {
                    qDebug() << "    Point array" << j << ":" << dataset->GetPointData()->GetArray(j)->GetName();
                }

                // 列出所有单元数据数组名称
                for (int j = 0; j < dataset->GetCellData()->GetNumberOfArrays(); ++j) {
                    qDebug() << "    Cell array" << j << ":" << dataset->GetCellData()->GetArray(j)->GetName();
                }

                // 列出所有场数据数组名称
                // for (int j = 0; j < dataset->GetFieldData()->GetNumberOfArrays(); ++j) {
                //     qDebug() << "    Field array" << j << ":" << dataset->GetFieldData()->GetArray(j)->GetName();
                // }
            }

            // 提取数据数组信息
            extractDataArrays(block, node);
        }

        parentNode.children.push_back(node);
    }
}
void CGNSDataParser::extractDataArrays(vtkDataObject* data, CGNSNode& node) {
    vtkDataSet* dataset = vtkDataSet::SafeDownCast(data);
    if (!dataset) {
        qWarning() << "Failed to convert vtkDataObject to vtkDataSet for node:" << node.name;
        return;
    }

    qDebug() << "Extracting data from node:" << node.name;
    qDebug() << "Dataset type:" << dataset->GetClassName();
    qDebug() << "Number of points:" << dataset->GetNumberOfPoints();
    qDebug() << "Number of cells:" << dataset->GetNumberOfCells();

    // 处理点数据（Vertex数据）
    vtkPointData* pointData = dataset->GetPointData();
    if (pointData) {
        qDebug() << "Point data arrays count:" << pointData->GetNumberOfArrays();
        bool hasFlowSolutionData = false;
        for (int i = 0; i < pointData->GetNumberOfArrays(); ++i) {
            vtkDataArray* array = pointData->GetArray(i);
            if (!array) {
                qWarning() << "Point array" << i << "is null";
                continue;
            }

            QString arrayName = array->GetName();
            if (arrayName.isEmpty()) {
                qWarning() << "Point array" << i << "has no name";
                continue;
            }
            qDebug() << "  Point array:" << arrayName
                                << "components:" << array->GetNumberOfComponents()
                                << "tuples:" << array->GetNumberOfTuples()
                                << "range: [" << array->GetRange()[0] << ", " << array->GetRange()[1] << "]";

            // 检查是否是FlowSolution中的物理量
            if (arrayName == "Density" || arrayName == "Pressure" ||
                arrayName == "Temperature" || arrayName == "Velocity") {
                hasFlowSolutionData = true;
                qDebug() << "    ^^ This is FlowSolution data!";
                }

            ScalarDataInfo info;
            info.name = arrayName;
            info.location = "Vertex";
            info.isVector = (array->GetNumberOfComponents() > 1);

            // 添加分量名称
            if (info.isVector) {
                for (int j = 0; j < array->GetNumberOfComponents(); ++j) {
                    const char* componentName = array->GetComponentName(j);
                    info.components.push_back(componentName ? QString(componentName) : QString("Component%1").arg(j));
                }
            }

            mScalarDataList.push_back(info);
            node.properties[arrayName] = QString("Vertex Data (%1 components)").arg(array->GetNumberOfComponents());

            if (!hasFlowSolutionData) {
                qWarning() << "No FlowSolution data found in point arrays!";
            }
        }
    } else {
        qWarning() << "Point data is null for node:" << node.name;
    }

    // 处理单元数据（Cell数据）
    vtkCellData* cellData = dataset->GetCellData();
    if (cellData) {
        qDebug() << "Cell data arrays count:" << cellData->GetNumberOfArrays();

        for (int i = 0; i < cellData->GetNumberOfArrays(); i++) {
            vtkDataArray* array = cellData->GetArray(i);
            if (!array) {
                qWarning() << "Cell array" << i << "is null";
                continue;
            }

            QString arrayName = array->GetName();
            if (arrayName.isEmpty()) {
                qWarning() << "Cell array" << i << "has no name";
                continue;
            }

            qDebug() << "Found cell array:" << arrayName
                     << "components:" << array->GetNumberOfComponents()
                     << "tuples:" << array->GetNumberOfTuples();

            ScalarDataInfo info;
            info.name = arrayName;
            info.location = "Cell";
            info.isVector = (array->GetNumberOfComponents() > 1);

            if (info.isVector) {
                for (int j = 0; j < array->GetNumberOfComponents(); j++) {
                    const char* componentName = array->GetComponentName(j);
                    info.components.push_back(componentName ? QString(componentName) : QString("Component%1").arg(j));
                }
            }

            mScalarDataList.push_back(info);
            node.properties[arrayName] = QString("Cell Data (%1 components)").arg(array->GetNumberOfComponents());
        }
    } else {
        qWarning() << "Cell data is null for node:" << node.name;
    }

    // 处理场数据（FieldData）
    vtkFieldData* fieldData = dataset->GetFieldData();
    if (fieldData && fieldData->GetNumberOfArrays() > 0) {
        qDebug() << "Field data arrays count:" << fieldData->GetNumberOfArrays();

        for (int i = 0; i < fieldData->GetNumberOfArrays(); i++) {
            vtkDataArray* array = fieldData->GetArray(i);
            if (!array) continue;

            QString arrayName = array->GetName();
            if (arrayName.isEmpty()) continue;

            qDebug() << "Found field array:" << arrayName
                     << "components:" << array->GetNumberOfComponents()
                     << "tuples:" << array->GetNumberOfTuples();

            ScalarDataInfo info;
            info.name = arrayName;
            info.location = "Field";
            info.isVector = (array->GetNumberOfComponents() > 1);

            if (info.isVector) {
                for (int j = 0; j < array->GetNumberOfComponents(); j++) {
                    const char* componentName = array->GetComponentName(j);
                    info.components.push_back(componentName ? QString(componentName) : QString("Component%1").arg(j));
                }
            }

            mScalarDataList.push_back(info);
            node.properties[arrayName] = QString("Field Data (%1 components)").arg(array->GetNumberOfComponents());
        }
    }
}

QString CGNSDataParser::createNodePath(const QString& parentPath, const QString& nodeName) const {
    if (parentPath.isEmpty()) {
        return "/" + nodeName;
    }
    return parentPath + "/" + nodeName;
}

bool CGNSDataParser::hasData(vtkMultiBlockDataSet* mb) {
    if (!mb) {
        qWarning() << "vtkMultiBlockDataSet is null ";
        return false;
    }

    vtkCompositeDataIterator* iter = mb->NewIterator();
    iter->InitTraversal();

    bool hasData = false;
    while (!iter->IsDoneWithTraversal()) {
        vtkDataObject* obj = iter->GetCurrentDataObject();
        if (vtkDataSet::SafeDownCast(obj)) {
            auto ds = vtkDataSet::SafeDownCast(obj);
            if (ds->GetPointData()->GetNumberOfArrays() > 0 ||
               ds->GetCellData()->GetNumberOfArrays() > 0 ) {
                hasData = true;
                break;
            }
        }
        iter->GoToNextItem();
    }
    iter->Delete();
    return hasData;
}
