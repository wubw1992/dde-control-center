// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "shortcutmodel.h"
#include "window/utils.h"
#include <QDBusInterface>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QThreadPool>
#include <QGuiApplication>

#include "shortcutitem.h"

static const QStringList systemFilter = {"terminal",
                                         "terminal-quake",
                                         "global-search",
                                         "screenshot",
                                         "screenshot-delayed",
                                         "screenshot-fullscreen",
                                         "screenshot-window",
                                         "screenshot-scroll",
                                         "screenshot-ocr",
                                         "deepin-screen-recorder",
                                         "switch-group",
                                         "switch-group-backward",
                                         "preview-workspace",
                                         "expose-windows",
                                         "expose-all-windows",
                                         "launcher",
                                         "switch-applications",
                                         "switch-applications-backward",
                                         "show-desktop",
                                         "file-manager",
                                         "lock-screen",
                                         "logout",
                                         "wm-switcher",
                                         "system-monitor",
                                         "color-picker",
                                         "clipboard",
                                         "switch-next-kbd-layout"};

static QStringList windowFilter = {"maximize", "unmaximize", "minimize", "begin-move", "begin-resize", "close"};

static QStringList workspaceFilter = {"switch-to-workspace-left",
                               "switch-to-workspace-right",
                               "move-to-workspace-left",
                               "move-to-workspace-right"};

static QStringList assistiveToolsFilter = {"ai-assistant", "text-to-speech", "speech-to-text", "translation"};

namespace dcc {
namespace keyboard {

ShortcutModel::ShortcutModel(QObject *parent)
    : QObject(parent)
    , m_windowSwitchState(false)
{
    if (m_dis.monitorList().size() > 1) {
        static const QStringList systemFilter = {"terminal",
                                                 "terminal-quake",
                                                 "screenshot",
                                                 "screenshot-delayed",
                                                 "screenshot-fullscreen",
                                                 "screenshot-window",
                                                 "screenshot-scroll",
                                                 "screenshot-ocr",
                                                 "deepin-screen-recorder",
                                                 "switch-group",
                                                 "switch-group-backward",
                                                 "preview-workspace",
                                                 "expose-windows",
                                                 "expose-all-windows",
                                                 "launcher",
                                                 "switch-applications",
                                                 "switch-applications-backward",
                                                 "show-desktop",
                                                 "file-manager",
                                                 "lock-screen",
                                                 "logout",
                                                 "wm-switcher",
                                                 "system-monitor",
                                                 "color-picker",
                                                 "clipboard",
                                                 "switch-monitors"
                                                };
    }
}

ShortcutModel::~ShortcutModel()
{
    qDeleteAll(m_infos);

    m_infos.clear();
    m_systemInfos.clear();
    m_windowInfos.clear();
    m_workspaceInfos.clear();
    m_customInfos.clear();
    qDeleteAll(m_searchList);
    m_searchList.clear();
}

QList<ShortcutInfo *> ShortcutModel::systemInfo() const
{
    return m_systemInfos;
}

QList<ShortcutInfo *> ShortcutModel::windowInfo() const
{
    return m_windowInfos;
}

QList<ShortcutInfo *> ShortcutModel::workspaceInfo() const
{
    return m_workspaceInfos;
}

QList<ShortcutInfo *> ShortcutModel::assistiveToolsInfo() const
{
    return m_assistiveToolsInfos;
}

QList<ShortcutInfo *> ShortcutModel::customInfo() const
{
    return m_customInfos;
}

QList<ShortcutInfo *> ShortcutModel::infos() const
{
    return m_infos;
}

void ShortcutModel::delInfo(ShortcutInfo *info)
{
    if (m_infos.contains(info)) {
        m_infos.removeOne(info);
    }
    if (m_customInfos.contains(info)) {
        m_customInfos.removeOne(info);
    }

    delete info;
    info = nullptr;
}

void ShortcutModel::onParseInfo(const QString &info)
{
    QStringList systemShortKeys;
    if (DCC_NAMESPACE::IsServerSystem) {
        QStringList systemFilterServer = systemFilter;
        systemFilterServer.removeOne("wm-switcher");
        systemFilterServer.removeOne("preview-workspace");
        systemFilterServer.removeOne("expose-windows");
        systemFilterServer.removeOne("expose-all-windows");
        systemShortKeys = systemFilterServer;
    } else if (false == m_windowSwitchState) {
        QStringList systemFilterServer = systemFilter;
        systemFilterServer.removeOne("expose-all-windows");
        systemFilterServer.removeOne("expose-windows");
        systemFilterServer.removeOne("preview-workspace");
        systemShortKeys = systemFilterServer;
    } else {
        systemShortKeys = systemFilter;
    }

    if(QGuiApplication::platformName().startsWith("wayland", Qt::CaseInsensitive)){
        systemShortKeys.removeOne("wm-switcher");
    }
#ifdef DISABLE_SCREEN_RECORDING
    QStringList systemFilterServer = systemShortKeys;
    systemFilterServer.removeOne("deepin-screen-recorder");
    systemShortKeys = systemFilterServer;
#endif
    qDeleteAll(m_infos);

    m_infos.clear();
    m_systemInfos.clear();
    m_windowInfos.clear();
    m_workspaceInfos.clear();
    m_assistiveToolsInfos.clear();
    m_customInfos.clear();

    QJsonArray array = QJsonDocument::fromJson(info.toStdString().c_str()).array();

    Q_FOREACH (QJsonValue value, array) {
        QJsonObject obj  = value.toObject();
        int         type = obj["Type"].toInt();

        ShortcutInfo *info = new ShortcutInfo();
        info->type         = type;
        info->accels       = obj["Accels"].toArray().first().toString();
        info->name    = obj["Name"].toString();
        info->id      = obj["Id"].toString();
        info->command = obj["Exec"].toString();

        m_infos << info;

        if (type != MEDIAKEY) {
            if (systemShortKeys.contains(info->id)) {
                m_systemInfos << info;
                continue;
            }
            if (windowFilter.contains(info->id)) {
                m_windowInfos << info;
                continue;
            }
            if (workspaceFilter.contains(info->id)) {
                m_workspaceInfos << info;
                continue;
            }
            if (assistiveToolsFilter.contains(info->id)) {
                m_assistiveToolsInfos << info;
                continue;
            }
            if (type == 1) {
                m_customInfos << info;
            }
        }
    }

    qSort(m_systemInfos.begin(), m_systemInfos.end(), [ = ](ShortcutInfo *s1, ShortcutInfo *s2) {
        return systemShortKeys.indexOf(s1->id) < systemShortKeys.indexOf(s2->id);
    });

    qSort(m_windowInfos.begin(), m_windowInfos.end(), [ = ](ShortcutInfo *s1, ShortcutInfo *s2) {
        return windowFilter.indexOf(s1->id) < windowFilter.indexOf(s2->id);
    });

    qSort(m_workspaceInfos.begin(), m_workspaceInfos.end(), [ = ](ShortcutInfo *s1, ShortcutInfo *s2) {
        return workspaceFilter.indexOf(s1->id) < workspaceFilter.indexOf(s2->id);
    });

    qSort(m_assistiveToolsInfos.begin(), m_assistiveToolsInfos.end(), [ = ](ShortcutInfo *s1, ShortcutInfo *s2) {
        return assistiveToolsFilter.indexOf(s1->id) < assistiveToolsFilter.indexOf(s2->id);
    });

    Q_EMIT listChanged(m_systemInfos, InfoType::System);
    Q_EMIT listChanged(m_windowInfos, InfoType::Window);
    Q_EMIT listChanged(m_workspaceInfos, InfoType::Workspace);
    Q_EMIT listChanged(m_assistiveToolsInfos, InfoType::AssistiveTools);
    Q_EMIT listChanged(m_customInfos, InfoType::Custom);
}

void ShortcutModel::onCustomInfo(const QString &json)
{
    QJsonObject   obj  = QJsonDocument::fromJson(json.toStdString().c_str()).object();
    ShortcutInfo *info = new ShortcutInfo();
    info->type         = obj["Type"].toInt();
    QString accels     = obj["Accels"].toArray().at(0).toString();

    info->accels = accels;

    info->name    = obj["Name"].toString();
    info->id      = obj["Id"].toString();
    info->command = obj["Exec"].toString();
    m_infos.append(info);
    m_customInfos.append(info);
    Q_EMIT addCustomInfo(info);
}

void ShortcutModel::onKeyBindingChanged(const QString &value)
{
    const QJsonObject &obj       = QJsonDocument::fromJson(value.toStdString().c_str()).object();
    const QString     &update_id = obj["Id"].toString();
    auto res = std::find_if(m_infos.begin(), m_infos.end(), [ = ] (const ShortcutInfo *info)->bool{
        return info->id == update_id;
    });

    if (res != m_infos.end()) {
        (*res)->type = obj["Type"].toInt();
        (*res)->accels  = obj["Accels"].toArray().first().toString();
        (*res)->name    = obj["Name"].toString();
        (*res)->command = obj["Exec"].toString();

        Q_EMIT shortcutChanged((*res));
    }
}

void ShortcutModel::onWindowSwitchChanged(bool value)
{
    if (m_windowSwitchState != value) {
        m_windowSwitchState = value;
    }
}

 bool ShortcutModel::getWindowSwitch()
 {
     return m_windowSwitchState;
 }

ShortcutInfo *ShortcutModel::currentInfo() const
{
    return m_currentInfo;
}

void ShortcutModel::setCurrentInfo(ShortcutInfo *currentInfo)
{
    m_currentInfo = currentInfo;
}

ShortcutInfo *ShortcutModel::getInfo(const QString &shortcut)
{
    QString newShortcut = shortcut;
    if (QGuiApplication::platformName().startsWith("wayland", Qt::CaseInsensitive)) {
        newShortcut = parseKeystroke(newShortcut);
    }
    auto res = std::find_if(m_infos.begin(), m_infos.end(), [ = ] (const ShortcutInfo *info)->bool{
        return !QString::compare(info->accels, newShortcut, Qt::CaseInsensitive); //判断是否相等，相等则返回0
    });

    if (res != m_infos.end()) {
        return *res;
    }

    return nullptr;
}

QString ShortcutModel::parseKeystroke(QString& shortcut)
{
    // 如<Alt><Super><Control><Shift>L转换后为<Shift><Control><Alt><Super>L
    QStringList keys = shortcut.replace(">", ">,").split(",");
    keys.removeAll("");
    QString sortArr[5];
    foreach (const QString& key, keys) {
        if (key.contains("Shift")) {
            sortArr[0] = key;
        } else if (key.contains("Control")) {
            sortArr[1] = key;
        } else if (key.contains("Alt")) {
            sortArr[2] = key;
        } else if (key.contains("Super")) {
            sortArr[3] = key;
        } else {
            sortArr[4] = key;
        }
    }
    QString newShort;
    for (const QString& key : sortArr) {
        if (!key.isEmpty()) {
            newShort += key;
        }
    }
    return newShort;
}

void ShortcutModel::setSearchResult(const QString &searchResult)
{
    qDeleteAll(m_searchList);
    m_searchList.clear();

    QList<ShortcutInfo *> systemInfoList;
    QList<ShortcutInfo *> windowInfoList;
    QList<ShortcutInfo *> workspaceInfoList;
    QList<ShortcutInfo *> customInfoList;
    QList<ShortcutInfo *> speechInfoList;

    QJsonArray array = QJsonDocument::fromJson(searchResult.toStdString().c_str()).array();
    for (auto value : array) {
        QJsonObject obj  = value.toObject();
        if ((obj["Id"].toString() == "wm-switcher") && (QGuiApplication::platformName().startsWith("wayland", Qt::CaseInsensitive))) {
            continue;
        }
        int         type = obj["Type"].toInt();
        ShortcutInfo *info = new ShortcutInfo();
        info->type         = type;
        info->accels       = obj["Accels"].toArray().first().toString();
        info->name    = obj["Name"].toString();
        info->id      = obj["Id"].toString();
        info->command = obj["Exec"].toString();

        if (type != MEDIAKEY) {
            if (systemFilter.contains(info->id)) {
                systemInfoList << info;
                continue;
            }
            if (windowFilter.contains(info->id)) {
                windowInfoList << info;
                continue;
            }
            if (workspaceFilter.contains(info->id)) {
                workspaceInfoList << info;
                continue;
            }
            if (assistiveToolsFilter.contains(info->id)) {
                speechInfoList << info;
                continue;
            }

            if (type == 1) {
                customInfoList << info;
            }else{
                delete info;
                info = nullptr;
            }

        } else {
            qDebug() << "not search is:" << info->name;
            delete info;
            info = nullptr;
        }
    }

    qSort(systemInfoList.begin(), systemInfoList.end(), [ = ](ShortcutInfo *s1, ShortcutInfo *s2) {
        return systemFilter.indexOf(s1->id) < systemFilter.indexOf(s2->id);
    });
    qSort(windowInfoList.begin(), windowInfoList.end(), [ = ](ShortcutInfo *s1, ShortcutInfo *s2) {
        return windowFilter.indexOf(s1->id) < windowFilter.indexOf(s2->id);
    });
    qSort(workspaceInfoList.begin(), workspaceInfoList.end(), [ = ](ShortcutInfo *s1, ShortcutInfo *s2) {
        return workspaceFilter.indexOf(s1->id) < workspaceFilter.indexOf(s2->id);
    });
    m_searchList.append(systemInfoList);
    m_searchList.append(windowInfoList);
    m_searchList.append(workspaceInfoList);
    m_searchList.append(speechInfoList);
    m_searchList.append(customInfoList);
    int i = 0;
    for (auto search : m_searchList) {
        qDebug() << "search" << ++i << " is: " << search->name;
    }

    Q_EMIT searchFinished(m_searchList);
}
}
}
