/*
 * SPDX-FileCopyrightText: (C) 2021 Wangrui <Wangrui@jingos.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#include "models/left_menu/leftmenudata.h"

#include <QFile>
#include <QDebug>
#include <QDir>

#ifdef STATIC_MAUIKIT
#include "fmh.h"
#include "fmstatic.h"
#else
#include <MauiKit/fmh.h>
#include <MauiKit/fmstatic.h>
#endif

#include <KIO/RestoreJob>
#include <KIO/CopyJob>
#include <QProcess>

#include <QtConcurrent>
#include <QFuture>

#include <kio/previewjob.h>
#include <QPixmap>

LeftMenuData::LeftMenuData(QObject *parent) : QObject(parent)
{
}

QString LeftMenuData::getUserName()
{
    int index = FMH::HomePath.lastIndexOf("/");
    QString userName = FMH::HomePath.mid(index + 1);
    return userName;
}

QString LeftMenuData::getHomePath()
{
    return FMH::HomePath;
}

QString LeftMenuData::getDownloadsPath()
{
    return FMH::DownloadsPath;
}

QString LeftMenuData::getRootPath()
{
    return FMH::RootPath;
}

QString LeftMenuData::getTrashPath()
{
    return FMH::TrashPath;
}

void LeftMenuData::restoreFromTrash(const QList<QUrl> &urls)
{
    auto job = KIO::restoreFromTrash(urls, KIO::HideProgressInfo);
    job->start();
}

QString LeftMenuData::createDir(const QUrl &path, const QString &name)
{
    QString dir_str = path.toString() + "/" + name;
    dir_str.replace(QString("file://"), QString(""));
    QDir dir;
    QString folderName = dir_str;
    int count = 1;
    while (dir.exists(folderName)) {
        if (count == 10) {
            folderName = path.toString() + "/" + "Are U Crazy?";
            folderName.replace(QString("file://"), QString(""));
        } else {
            folderName = dir_str + QString::number(count);
        }
        count++;
    }
    dir.mkpath(folderName);
    return "file://" + folderName;
}

bool LeftMenuData::playVideo(const QString url)
{
    addFileToRecents(url);
    QString kill = "killall -9 haruna";
    QProcess process(this);
    process.execute(kill);
    QStringList arguments;
    QString program = "/usr/bin/haruna";
    arguments << QString::number(0);
    arguments << QString::number(0);
    arguments << url;
    process.startDetached(program, arguments);
    return true;
}

void LeftMenuData::addFileToRecents(const QString url)
{
    if (!FMStatic::urlTagExists(url, "recents_jingos")) {
        FMStatic::addTagToUrl("recents_jingos", url);
    }
}

void LeftMenuData::addFolderToCollection(const QString url)
{
    if (!FMStatic::urlTagExists(url, "collection_jingos")) {
        FMStatic::addTagToUrl("collection_jingos", url);
    }
}

QVariantList LeftMenuData::getCollectionList()
{
    QVariantList res;
    FMH::MODEL_LIST collectionList = FMStatic::getTagContent("collection_jingos");
    for (const auto &item : collectionList)
        res << FMH::toMap(item);

    return res;
}

quint64 LeftMenuData::getDirSizeReal(const QString &filePath)
{
    QDir tmpDir(filePath);
    quint64 size = 0;

    foreach (QFileInfo fileInfo, tmpDir.entryInfoList(QDir::Files)) {
        size += fileInfo.size();
        if (!this->isEmit)
        {
            break;
        }
    }

    foreach (QString subDir, tmpDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        size += getDirSizeReal(filePath + QDir::separator() + subDir); 
        sizeOfResult += size;
        if (!this->isEmit) {
            break;
        }
        if (sizeOfResult > 0) {
            emit this->refreshDirSize(sizeOfResult);
        }
    }
    return size;
}

void LeftMenuData::getDirSize(const QString &filePath)
{
    this->isEmit = true;
    sizeOfResult = 0;
    QFutureWatcher<quint64> *watcher = new QFutureWatcher<quint64>;
    connect(watcher, &QFutureWatcher<quint64>::finished, [&, watcher]()
    {
        if (this->isEmit) {
            emit this->refreshDirSize(watcher->future().result());
        }
        watcher->deleteLater();
    });

    const auto func = [=]() -> quint64
    {
        quint64 size = getDirSizeReal(filePath);
        return size;
    };

    QFuture<quint64> t1 = QtConcurrent::run(func);
    watcher->setFuture(t1);
}

void LeftMenuData::cancelGetDirSize()
{
    this->isEmit = false;
}

QString LeftMenuData::getVideoPreview(const QUrl &url)
{
    auto path = url.toString();
    int index = path.lastIndexOf(".");
    QString newPath = path.mid(0, index);//path/name
    index = newPath.lastIndexOf("/");
    QString startPath = newPath.mid(0, index + 1);//path/
    QString endPath = newPath.mid(index + 1, newPath.length());//name
    path = startPath + "." + endPath + ".jpg";

    QFutureWatcher<quint64> *watcher = new QFutureWatcher<quint64>;
    connect(watcher, &QFutureWatcher<quint64>::finished, [&, watcher]()
    {
        watcher->deleteLater();
    });

    const auto func = [=]() -> quint64
    {
        QFile file(path.mid(7));
        if (file.exists()) {
            return -1;
        }

        QStringList plugins;
        plugins << KIO::PreviewJob::availablePlugins();
        KFileItemList list;
        list.append(KFileItem(url, QString(), 0));
        KIO::PreviewJob *job = KIO::filePreview(list, QSize(140, 140), &plugins);
        job->setIgnoreMaximumSize(true);
        job->setScaleType(KIO::PreviewJob::ScaleType::Unscaled);

        QObject::connect(job, &KIO::PreviewJob::gotPreview, [=] (const KFileItem &item, const QPixmap &preview) {
            preview.save(path.mid(7), "JPG");
            emit this->refreshImageSource(path);
        });
        QObject::connect(job, &KIO::PreviewJob::failed, [=] (const KFileItem &item) {
            qDebug() << "getVideos gotPreview failed";
        });
        job->exec();
        return 0;
    };

    QFuture<quint64> t1 = QtConcurrent::run(func);
    watcher->setFuture(t1);

    return path;
}

const FMH::MODEL LeftMenuData::getFileInfoModel(const QUrl &path)
{
    FMH::MODEL model = FMH::getFileInfoModel(path);
    return model;
}

