//
// Created by 15206 on 2025/8/23.
//

// You may need to build the project (run Qt uic code generator) to get "ui_NodeTreeWidget.h" resolved

#include "NodeTreeWidget.h"

#include "CGNSDataParser.h"

NodeTreeWidget::NodeTreeWidget(QWidget* parent) :
    QTreeWidget(parent){
    connect(this, &QTreeWidget::itemChanged, this, &NodeTreeWidget::onItemChanged);
}

NodeTreeWidget::~NodeTreeWidget() {
}

void NodeTreeWidget::clear() {
    QTreeWidget::clear();
    mModifiedItems.clear();
}

void NodeTreeWidget::buildTreeItems(QTreeWidgetItem* parentItem, const CGNSNode& node) {
    // 创建当前节点
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, node.name);
    item->setCheckState(0, node.visible ? Qt::Checked : Qt::Unchecked);

    // 存储节点数据（路径和块ID）
    item->setData(0, Qt::UserRole, QVariant::fromValue(node));

    // 添加到父节点或作为根节点
    if (parentItem) {
        parentItem->addChild(item);
    } else {
        this->addTopLevelItem(item);
    }

    // 递归处理子节点
    for (const CGNSNode& child : node.children) {
        buildTreeItems(item, child);
    }
}

void NodeTreeWidget::updateChildrenState(QTreeWidgetItem* parent, Qt::CheckState state) {
    for (int i = 0; i < parent->childCount(); ++i) {
        QTreeWidgetItem *child = parent->child(i);
        if (child->checkState(0) != state) {
            child->setCheckState(0, state);
            auto node = child->data(0, Qt::UserRole).value<CGNSNode>();
            node.visible = (state != Qt::Unchecked);
            child->setData(0, Qt::UserRole, QVariant::fromValue(node));
            mModifiedItems.push_back(node); // 记录被修改的父节点
        }
        updateChildrenState(child, state); // 递归更新
    }
}

void NodeTreeWidget::updateParentState(QTreeWidgetItem* item) {
    QTreeWidgetItem *parent = item->parent();
    if (!parent) return;

    int checkedCount = 0;
    int partiallyCheckedCount = 0;

    // 统计子节点的选中状态
    for (int i = 0; i < parent->childCount(); ++i) {
        QTreeWidgetItem *child = parent->child(i);
        Qt::CheckState state = child->checkState(0);

        if (state == Qt::Checked) {
            checkedCount++;
        } else if (state == Qt::PartiallyChecked) {
            partiallyCheckedCount++;
        }
    }

    // 确定父节点的新状态
    Qt::CheckState newState;
    if (checkedCount == parent->childCount()) {
        newState = Qt::Checked;
    } else if (checkedCount > 0 || partiallyCheckedCount > 0) {
        newState = Qt::PartiallyChecked;
    } else {
        newState = Qt::Unchecked;
    }

    // 只有当状态确实改变时才更新
    if (parent->checkState(0) != newState) {
        parent->setCheckState(0, newState);
        auto node = parent->data(0, Qt::UserRole).value<CGNSNode>();
        node.visible = (newState != Qt::Unchecked);
        item->setData(0, Qt::UserRole, QVariant::fromValue(node));
        mModifiedItems.push_back(node); // 记录被修改的父节点
        updateParentState(parent); // 递归更新祖父节点
    }
}

void NodeTreeWidget::onItemChanged(QTreeWidgetItem* item, int column) {
    if (column != 0) return; // 只处理第一列的复选框变化
    mModifiedItems.clear();
    // 阻塞信号防止递归触发
    this->blockSignals(true);

    Qt::CheckState state = item->checkState(0);

    auto node = item->data(0, Qt::UserRole).value<CGNSNode>();
    node.visible = (state != Qt::Unchecked);
    item->setData(0, Qt::UserRole, QVariant::fromValue(node));

    // 记录修改的节点
    mModifiedItems.push_back(node);

    // 1. 更新所有子节点的状态
    updateChildrenState(item, state);

    // 2. 更新父节点的状态
    updateParentState(item);

    this->blockSignals(false);

    emit nodeStateChanged(mModifiedItems);

}
