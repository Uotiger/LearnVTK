#ifndef CHAPTER1_BASEVTKDEMO_NODETREEWIDGET_H
#define CHAPTER1_BASEVTKDEMO_NODETREEWIDGET_H

#include <QTreeWidget>


struct CGNSNode;

class NodeTreeWidget : public QTreeWidget {
    Q_OBJECT

public:
    explicit NodeTreeWidget(QWidget* parent = nullptr);
    ~NodeTreeWidget() override;

    void clear();

    void buildTreeItems(QTreeWidgetItem* parentItem, const CGNSNode& node);

signals:
    void nodeStateChanged(std::vector<CGNSNode> nodeVec);
private:
    void updateChildrenState(QTreeWidgetItem *parent, Qt::CheckState state);

    void updateParentState(QTreeWidgetItem *item);
private slots:
    void onItemChanged(QTreeWidgetItem *item, int column);
private:
    std::vector<CGNSNode> mModifiedItems;
};


#endif //CHAPTER1_BASEVTKDEMO_NODETREEWIDGET_H