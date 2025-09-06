#pragma once
#include <QObject>
#include <QString>
#include <list>
#include <map>
#include <QVariant>
#include <vtkDataArray.h>
#include <vtkSmartPointer.h>
#include <vtkMultiBlockDataSet.h>

// 树节点数据结构
struct CGNSNode {
    QString name;
    QString type;   // Base Zone Grid FlowSolution ..
    bool visible;
    std::list<CGNSNode> children;   // 子节点
    std::map<QString, QVariant> properties; // 额外属性
    int blockId;    // 数据块id
};

// 云图数据信息
struct ScalarDataInfo {
    QString name;
    QString location;   // 数据位置 “Cell” "Vertex"
    std::list<QString> components; // 数据分量名称(对于矢量数据)
    bool isVector; // 查看是否为矢量数据。
};

class CGNSDataParser : public QObject {
    Q_OBJECT

public:
    explicit CGNSDataParser(QObject * parent = nullptr);
    ~CGNSDataParser() = default;

    // 解析文件
    bool parseFile(const QString& filePath);

    // 获取解析后的部件树根节点
    const CGNSNode& getTreeStructure() const;

    // 获取所有可用的标量数据名称列表
    const std::list<ScalarDataInfo>& getScalarDataList() const;

    // 根据节点路径获取对应的数据块ID
    int getBlockIdFromPath(const QString &path) const;

    // 获取指定数据块的网格数据
    vtkSmartPointer<vtkDataObject> getBlockData(int blockId) const;

    // 获取指定数据块的标量数据
    vtkSmartPointer<vtkDataArray> getScalarData(int blockId, const QString &dataName) const;

    // 获取路径与数据块idMap
    const std::map<QString, int>& getPathToBlockIdMap() const;
private:
    // 递归解析多块数据集
    void parseMultiBlock(vtkMultiBlockDataSet *mb, CGNSNode &parentNode, const QString &parentPath);

    // 提取数据数组信息
    void extractDataArrays(vtkDataObject *data, CGNSNode &node);

    // 创建节点路径
    QString createNodePath(const QString &parentPath, const QString &nodeName) const;

    bool hasData(vtkMultiBlockDataSet* mb);

private:
    vtkSmartPointer<vtkMultiBlockDataSet> mMultiBlockData;
    CGNSNode mRootNode;
    std::list<ScalarDataInfo> mScalarDataList;
    std::map<QString, int> mPathToBlockIdMap;
    std::map<int, vtkSmartPointer<vtkDataObject>> mBlockDataMap;
};