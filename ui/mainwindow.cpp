#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_errorswindow.h"
#include "dialogwindow.h"
#include <QtWidgets/QFileDialog>


MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent),
          ui(new Ui::MainWindow),
          errorsUi(new Ui::ErrorsWindow),
          errorsWidget() {
    ui->setupUi(this);
    errorsUi->setupUi(&errorsWidget);
    errorsWidget.hide();

    auto *finder = new QtDupsFinder;
    finder->moveToThread(&qThread);
    connect(this, &MainWindow::doFind, finder, &QtDupsFinder::doFind);
    connect(this, &MainWindow::cancel, finder, &QtDupsFinder::cancel);
    connect(finder, &QtDupsFinder::foundDup, this, &MainWindow::handleDup);
    connect(finder, &QtDupsFinder::resultReady, this, &MainWindow::handleResultReady);
    connect(finder, &QtDupsFinder::progress, this, &MainWindow::handleProgress);
    connect(finder, &QtDupsFinder::error, this, &MainWindow::handleFileError);
    qThread.start();

    connect(ui->pushButton, SIGNAL(released()), this, SLOT(handleStartButton()));
    connect(ui->cancelButton, SIGNAL(released()), this, SLOT(handleCancelButton()));
    connect(ui->errorsButton, SIGNAL(released()), this, SLOT(handleErrorsButton()));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeMode::ResizeToContents);

    connect(ui->treeWidget, &QTreeWidget::itemActivated, this, &MainWindow::handleItemActivated);
}


void MainWindow::handleStartButton() {
    currentRootPath = QFileDialog::getExistingDirectory(
            this,
            "Select Directory for Scanning",
            QString(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!currentRootPath.isEmpty()) {
        runFind(currentRootPath);
    }
}


MainWindow::~MainWindow() {
    emit cancel();

    qThread.quit();
    qThread.wait();
    delete ui;
    delete errorsUi;
}


void MainWindow::handleDup(qint64 dupId, QFileInfo fileInfo) {
    if (idToItem.count(dupId) == 0) {
        auto *item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, fileInfo.fileName());
        ui->treeWidget->addTopLevelItem(item);
        idToItem[dupId] = item;
        idToItem[dupId]->setData(0, Qt::UserRole, dupId);
        idToItem[dupId]->setText(2, QString::number(fileInfo.size()));
    }
    idToFileInfoList[dupId].push_back(fileInfo);
    idToItem[dupId]->setText(1, QString::number(idToFileInfoList[dupId].size()));
}


void MainWindow::handleResultReady() {
    ui->currentInfoLabel->setText("All dups found in " + currentRootPath);
}


void MainWindow::handleItemActivated(QTreeWidgetItem *item, int column) {
    auto *fileList = &idToFileInfoList[item->data(0, Qt::UserRole).toInt()];

    DialogWindow(currentRootPath, fileList, this).exec();

    if (fileList->size() <= 1) {
        ui->treeWidget->invisibleRootItem()->removeChild(item);
    } else {
        item->setText(1, QString::number(fileList->size()));
    }
}


void MainWindow::runFind(QString rootPath) {
    emit cancel();

    ui->currentInfoLabel->setText("Looking for dups in " + rootPath);
    ui->treeWidget->clear();
    idToItem.clear();
    idToFileInfoList.clear();
    errorsUi->treeWidget->clear();

    emit doFind(rootPath);
}


void MainWindow::handleCancelButton() {
    emit cancel();
    ui->currentInfoLabel->setText("Scanning canceled in " + currentRootPath);
}


void MainWindow::handleProgress(qint64 processed, qint64 total) {
    ui->processedLabel->setText(QString::number(processed));
    ui->totalFiles->setText(QString::number(total));
    ui->progressBar->setValue(int(processed * 100 / total));
}


void MainWindow::handleFileError(QString cause, QFileInfo fileInfo) {
    auto *item = new QTreeWidgetItem;
    item->setText(0, cause);
    item->setText(1, fileInfo.filePath());
    errorsUi->treeWidget->addTopLevelItem(item);
    ui->errorsButton->setText(
            "Errors: "
            + QString::number(errorsUi->treeWidget->invisibleRootItem()->childCount()));
}


void MainWindow::handleErrorsButton() {
    errorsWidget.show();
}
