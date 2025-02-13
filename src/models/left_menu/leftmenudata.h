/*
 * SPDX-FileCopyrightText: (C) 2021 Wangrui <Wangrui@jingos.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LEFTMEUNMODEL_H
#define LEFTMEUNMODEL_H

#include <QObject>
#include <QFile>
#include <QDateTime>
#include <QScreen>
#ifdef STATIC_MAUIKIT
#include "fmh.h"
#include "fmstatic.h"
#else
#include <MauiKit/fmh.h>
#include <MauiKit/fmstatic.h>
#endif

class LeftMenuData : public QObject
{
    Q_OBJECT

private:
    bool isEmit;
    quint64 sizeOfResult;

public:
    explicit LeftMenuData(QObject *parent = nullptr);

    Q_INVOKABLE QString getUserName();
    Q_INVOKABLE QString getHomePath();
    Q_INVOKABLE QString getDownloadsPath();
    Q_INVOKABLE QString getRootPath();
    Q_INVOKABLE QString getTrashPath();

public slots:
    void restoreFromTrash(const QList<QUrl> &urls);
    QString createDir(const QUrl &path, const QString &name);
    bool playVideo(const QString url);
    void addFileToRecents(const QString url);
    void addFolderToCollection(const QString url);
    void getDirSize(const QString &filePath);
    quint64 getDirSizeReal(const QString &filePath);
    void cancelGetDirSize();
    QString getVideoPreview(const QUrl &url);
    const FMH::MODEL getFileInfoModel(const QUrl &path);
    QVariantList getCollectionList();

signals:
    void refreshDirSize(quint64 size);
    void refreshImageSource(QString imagePath);
};

#endif
