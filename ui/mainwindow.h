#include <QMainWindow>
#include <finder/QtDupsFinder.h>
#include <memory>
#include <thread>
#include <QtWidgets/QTreeWidgetItem>
#include <QtCore/QThread>
#include <QtCore/QTime>


namespace Ui{
    class MainWindow;
    class ErrorsWindow;
}


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleStartButton();
    void handleCancelButton();
    void handleProgress(qint64 processed, qint64 total);
    void handleDup(qint64 dupId, QFileInfo fileInfo);
    void handleFileError(QString cause, QFileInfo fileInfo);
    void handleResultReady();
    void handleItemDoubleClicked(QTreeWidgetItem *item, int column);
    void handleErrorsButton();

    signals:
    void doFind(QString path);
    void cancel();

private:
    void runFind(QString rootPath);


    Ui::MainWindow *ui;
    Ui::ErrorsWindow *errorsUi;

    QMap<qint64, QTreeWidgetItem*> idToItem;
    QMap<qint64, QFileInfoList> idToFileInfoList;
    QThread qThread;
    QString currentRootPath;
    QWidget errorsWidget;
};