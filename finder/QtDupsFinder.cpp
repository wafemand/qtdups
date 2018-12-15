#include <utility>
#include "QtDupsFinder.h"
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QCoreApplication>
#include <iostream>


struct CancelException : std::exception {};


void QtDupsFinder::doFind(QDir rootDir) {
    nextId = 0;
    groupsBySize.clear();
    canceled = false;
    qint64 processed = 0;

    rootDir.setFilter(QDir::Files |
                      QDir::NoSymLinks |
                      QDir::NoDotAndDotDot);

    qint64 total = countTotal(rootDir);

    try {
        QDirIterator it(rootDir, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filename = it.next();
            processFile(filename);
            processed++;
            emit progress(processed, total);
        }
        emit resultReady();
    } catch (CancelException const &e){
        return;
    }
}


void QtDupsFinder::processFile(QFileInfo const &fileInfo) {
    FileHolder fileHolder(fileInfo, nextId);
    QVector<FileHolder> &sameSizeFileHolders =
            groupsBySize[fileInfo.size()];

    for (FileHolder &curFileHolder : sameSizeFileHolders){
        if (curFileHolder.hasError) {
            continue;
        }
        if (fileHolder.hasError){
            break;
        }
        cancellationPoint();
        if (equals(curFileHolder, fileHolder)){
            if (!curFileHolder.hasDup) {
                curFileHolder.hasDup = true;
                emit foundDup(curFileHolder.id, curFileHolder.fileInfo);
            }
            emit foundDup(curFileHolder.id, fileHolder.fileInfo);
            return;
        }
    }
    if (!fileHolder.hasError) {
        sameSizeFileHolders.push_back(fileHolder);
        nextId++;
    }
}

bool QtDupsFinder::equals(QtDupsFinder::FileHolder &fileHolder1, QtDupsFinder::FileHolder &fileHolder2) {
    static const int BUF_SIZE = 4096;

    QFile file1 = fileHolder1.fileInfo.filePath();
    bool ok1 = file1.open(QIODevice::ReadOnly | QIODevice::Unbuffered);
    QFile file2 = fileHolder2.fileInfo.filePath();
    bool ok2 = file2.open(QIODevice::ReadOnly | QIODevice::Unbuffered);

    if (!ok1) {
        emit error(file1.errorString(), fileHolder1.fileInfo);
        fileHolder1.hasError = true;
    }
    if (!ok2) {
        emit error(file2.errorString(), fileHolder2.fileInfo);
        fileHolder2.hasError = true;
    }
    if (!ok1 || !ok2){
        return false;
    }

    while (!file1.atEnd() && !file2.atEnd()) {
        QByteArray buffer1 = file1.read(BUF_SIZE);
        QByteArray buffer2 = file2.read(BUF_SIZE);
        if (memcmp(buffer1, buffer2, static_cast<size_t>(buffer1.size())) != 0) {
            return false;
        }
    }

    file1.close();
    file2.close();
    return true;
}

void QtDupsFinder::cancellationPoint() {
    QCoreApplication::processEvents();
    if (canceled){
        throw CancelException();
    }
}

void QtDupsFinder::cancel() {
    canceled = true;
}

qint64 QtDupsFinder::countTotal(QDir const &dir) {
    qint64 total = 0;
    QDirIterator it(dir, QDirIterator::Subdirectories);
    while (it.hasNext()){
        total++;
        it.next();
    }
    return total;
}
