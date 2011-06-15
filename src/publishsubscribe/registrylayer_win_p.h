/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSystemKit module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef REGISTRYLAYER_WIN_P_H
#define REGISTRYLAYER_WIN_P_H

#include "qvaluespace_p.h"
#include "qvaluespacepublisher.h"

#include <QSet>
#include <QStringList>
#include <QEventLoop>
#include <QTimer>
#include <QVariant>
#include <QMutex>
#include <QDebug>

// Define win32 version to pull in RegisterWaitForSingleObject and UnregisterWait.
#define _WIN32_WINNT 0x0500
#include <windows.h>

#define RegistryCallback WAITORTIMERCALLBACK

QT_BEGIN_NAMESPACE

class RegistryLayer : public QAbstractValueSpaceLayer
{
    Q_OBJECT

public:
    RegistryLayer(const QString &basePath, bool volatileKeys, RegistryCallback callback);
    ~RegistryLayer();

    /* Common functions */
    bool startup(Type t);

    Handle item(Handle parent, const QString &path);
    void removeHandle(Handle handle);
    void setProperty(Handle handle, Properties);

    bool value(Handle handle, QVariant *data);
    bool value(Handle handle, const QString &subPath, QVariant *data);
    QSet<QString> children(Handle handle);

    /* QValueSpaceSubscriber functions */
    bool supportsInterestNotification() const;
    bool notifyInterest(Handle handle, bool interested);

    /* QValueSpacePublisher functions */
    bool setValue(QValueSpacePublisher *creator, Handle handle, const QVariant &data);
    bool setValue(QValueSpacePublisher *creator, Handle handle, const QString &path,
                  const QVariant &data);
    bool removeValue(QValueSpacePublisher *creator, Handle handle, const QString &subPath);
    bool removeSubTree(QValueSpacePublisher *creator, Handle parent);
    void addWatch(QValueSpacePublisher *creator, Handle handle);
    void removeWatches(QValueSpacePublisher *creator, Handle parent);
    void sync();

public Q_SLOTS:
    void emitHandleChanged(void *hkey);

private:
    struct RegistryHandle {
        RegistryHandle(const QString &p)
            : path(p), valueHandle(false), refCount(1)
        {
        }

        QString path;
        bool valueHandle;
        unsigned int refCount;
    };

    void openRegistryKey(RegistryHandle *handle);
    bool createRegistryKey(RegistryHandle *handle);
    bool removeRegistryValue(RegistryHandle *handle, const QString &path);
    void closeRegistryKey(RegistryHandle *handle);
    void pruneEmptyKeys(RegistryHandle *handle);

    QMutex localLock;
    QString m_basePath;
    bool m_volatileKeys;
    RegistryCallback m_callback;

    QHash<QString, RegistryHandle *> handles;

    RegistryHandle *registryHandle(Handle handle)
    {
        if (handle == InvalidHandle)
            return 0;

        RegistryHandle *h = reinterpret_cast<RegistryHandle *>(handle);
        if (handles.values().contains(h))
            return h;

        return 0;
    }

    QMap<RegistryHandle *, HKEY> hKeys;
    QMultiMap<RegistryHandle *, RegistryHandle *> notifyProxies;
    // MinGW complains about QPair<::HANDLE, ::HANDLE>
    typedef ::HANDLE HandleType;
    typedef QPair<HandleType, HandleType> HandlePair;
    QMap<HKEY, HandlePair > waitHandles;

    QMap<QValueSpacePublisher *, QList<QString> > creators;
};

class VolatileRegistryLayer : public RegistryLayer
{
    Q_OBJECT

public:
    VolatileRegistryLayer();
    ~VolatileRegistryLayer();

    /* Common functions */
    QString name();

    QUuid id();
    unsigned int order();

    QValueSpace::LayerOptions layerOptions() const;

    static VolatileRegistryLayer *instance();
};

class NonVolatileRegistryLayer : public RegistryLayer
{
    Q_OBJECT

public:
    NonVolatileRegistryLayer();
    ~NonVolatileRegistryLayer();

    /* Common functions */
    QString name();

    QUuid id();
    unsigned int order();

    QValueSpace::LayerOptions layerOptions() const;

    static NonVolatileRegistryLayer *instance();
};

QT_END_NAMESPACE

#endif // REGISTRYLAYER_WIN_P_H
