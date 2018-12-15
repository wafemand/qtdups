#ifndef QTDUPS_DIALOGWINDOW_H
#define QTDUPS_DIALOGWINDOW_H


#include <QtCore/QObject>
#include <QtWidgets/QDialog>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileIconProvider>
#include "ui_dialogwindow.h"

namespace Ui {
    class Dialog;
}

class DialogWindow : public QDialog {
Q_OBJECT

public:
    explicit DialogWindow(QFileInfo const &root,
                          QFileInfoList *fileInfoList,
                          QWidget *parent = nullptr)
            : QDialog(parent),
              ui(new Ui::Dialog),
              duplicatesInfo(fileInfoList),
              root(root) {

        ui->setupUi(this);
        initTreeWidget(root, fileInfoList);

        connect(ui->OkButton, &QPushButton::clicked, this, &DialogWindow::ok);
        connect(ui->deleteSelectedButton, &QPushButton::clicked, this, &DialogWindow::deleteSelected);
        connect(ui->inverseSelectionButton, &QPushButton::clicked, this, &DialogWindow::inverseSelection);
        connect(ui->expandAllButton, &QPushButton::clicked, this, &DialogWindow::expandAll);
        connect(ui->selectAllButton, &QPushButton::clicked, this, &DialogWindow::selectAll);
    };

public slots:

    void expandAll() {
        ui->treeWidget->expandAll();
    }

    void inverseSelection() {
        iterateByTree(ui->treeWidget->invisibleRootItem(), [](QTreeWidgetItem *item) {
            if (item->childCount() == 0) {
                item->setCheckState(0, item->checkState(0) == Qt::Checked ? Qt::Unchecked : Qt::Checked);
            }
        });
    }

    void selectAll() {
        iterateByTree(ui->treeWidget->invisibleRootItem(), [](QTreeWidgetItem *item) {
            item->setCheckState(0, Qt::Checked);
        });
    }

    void deleteSelected() {
        auto confirm = QMessageBox::question(
                this,
                "Confirm",
                "Are you sure you want to delete these files?");
        if (confirm == QMessageBox::No) {
            return;
        }

        QSet<QTreeWidgetItem *> itemsToDeleteSet;

        iterateByTree(ui->treeWidget->invisibleRootItem(), [&itemsToDeleteSet](QTreeWidgetItem *item) {
            if (item->childCount() == 0 && item->checkState(0) == Qt::Checked) {
                itemsToDeleteSet.insert(item);
            }
        });

        QSet<QString> deletedFiles;
        for (QTreeWidgetItem *item : itemsToDeleteSet) {
            QFileInfo fileInfo = item->data(0, Qt::UserRole).toString();
            if (fileInfo.isFile()) {
                QFile file(fileInfo.filePath());
                if (!file.remove()) {
                    itemsToDeleteSet.remove(item);
                } else {
                    removeItem(item);
                    deletedFiles.insert(fileInfo.filePath());
                }
            }
        }

        auto newEnd =
                std::remove_if(duplicatesInfo->begin(), duplicatesInfo->end(),
                               [&deletedFiles](QFileInfo const &fileInfo) {
                                   return deletedFiles.contains(fileInfo.filePath());
                               });

        int tail = duplicatesInfo->end() - newEnd;
        while (tail--) {
            duplicatesInfo->pop_back();
        }
    }

    void ok() {
        close();
    }

private:
    QTreeWidgetItem *createItem(QFileInfo const &fileInfo, bool checkable) {
        auto *result = new QTreeWidgetItem;
        result->setData(0, Qt::UserRole, fileInfo.filePath());
        result->setText(0, fileInfo.fileName());
        result->setFlags(result->flags() | Qt::ItemIsAutoTristate | Qt::ItemIsUserCheckable);
        result->setIcon(0, QFileIconProvider().icon(fileInfo));
        result->setCheckState(0, Qt::Unchecked);
        return result;
    }


    void removeItem(QTreeWidgetItem *item) {
        if (item == nullptr) {
            return;
        }
        QTreeWidgetItem *parentItem = item->parent();
        while (parentItem != nullptr && item->childCount() == 0) {
            parentItem->removeChild(item);
            item = parentItem;
            parentItem = item->parent();
        }
    }

    void initTreeWidget(QFileInfo const &root,
                        QFileInfoList *fileInfoList) {
        QTreeWidgetItem *rootItem = ui->treeWidget->invisibleRootItem();
        QDir rootDir = root.absoluteDir();

        auto *rootDirItem = createItem(root, false);
        rootItem->addChild(rootDirItem);
        QMap<QString, QTreeWidgetItem *> nodes;
        nodes[root.filePath()] = rootDirItem;
        for (auto const &fileInfo : *fileInfoList) {
            auto *prevNode = createItem(fileInfo, true);
            QTreeWidgetItem *curNode = nullptr;

            nodes[fileInfo.filePath()] = prevNode;
            QDir curDir = fileInfo.dir();

            while (nodes.count(curDir.path()) == 0) {
                curNode = createItem(QFileInfo(curDir.path()), false);
                curNode->addChild(prevNode);
                nodes[curDir.path()] = curNode;
                curDir.cdUp();
                prevNode = curNode;
            }
            curNode = nodes[curDir.path()];
            curNode->addChild(prevNode);
        }
    }

    void iterateByTree(QTreeWidgetItem *item, std::function<void(QTreeWidgetItem *)> fun) {
        if (item == nullptr) {
            return;
        }
        for (int i = 0; i < item->childCount(); i++) {
            auto *child = item->child(i);
            iterateByTree(child, fun);
        }
        fun(item);
    }

    Ui::Dialog *ui;
    QFileInfoList *duplicatesInfo;
    QFileInfo root;
};


#endif //QTDUPS_DIALOGWINDOW_H
