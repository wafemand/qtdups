//
// Created by andrey on 02/12/18.
//

#ifndef QTDUPS_QTDUPSFINDER_H
#define QTDUPS_QTDUPSFINDER_H

#include <functional>
#include <QString>
#include <QVector>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QMap>
#include <QtCore/QDir>
#include <QMetaType>

class QtDupsFinder : public QObject {
Q_OBJECT

public slots:
    void doFind(QDir rootDir);
    void cancel();

signals:
    void foundDup(qint64 dupId, QFileInfo fileInfo);
    void resultReady();
    void progress(qint64 processed, qint64 total);
    void error(QString cause, QFileInfo fileInfo);

private:

    struct FileHolder {
        QFileInfo fileInfo;
        qint64 id = 0;
        bool hasDup = false;
        bool hasError = false;

        FileHolder() = default;
        FileHolder(QFileInfo const &fileInfo, qint64 id)
                : fileInfo(fileInfo), id(id) {}
    };


    void processFile(QFileInfo const &fileInfo);
    bool equals(FileHolder &fileHolder1, FileHolder &fileHolder2);
    void cancellationPoint();
    qint64 countTotal(QDir const &dir);


    QMap<qint64, QVector<FileHolder> > groupsBySize;
    qint64 nextId;
    bool canceled;
};


#endif //QTDUPS_QTDUPSFINDER_H
