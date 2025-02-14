// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "interface/namespace.h"
#include "widgets/settingsitem.h"

#include "DIconButton"
#include "DLineEdit"

QT_BEGIN_NAMESPACE
class QWidget;
class QHBoxLayout;
class QStackedWidget;
class QLabel;
class QLineEdit;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

namespace DCC_NAMESPACE {
namespace authentication {

class AuthenticationLinkButtonItem : public dcc::widgets::SettingsItem{
    Q_OBJECT
public:
    explicit AuthenticationLinkButtonItem(QWidget *parent = nullptr);

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

Q_SIGNALS:
    void mousePressed();

private:
    DPalette m_currentpa;

};

class AuthenticationInfoItem : public dcc::widgets::SettingsItem
{
    Q_OBJECT
public:
    explicit AuthenticationInfoItem(QWidget *parent = nullptr);
    void setTitle(const QString &title);
    QString getTitle() const { return m_itemName; };
    void alertTitleRepeat();
    void appendItem(QWidget *widget);
    void setShowIcon(bool state);
    void setEditTitle(bool state);
    void setHideTitle(bool state);
    bool onNameEditFinished();
    //判断账户名是否符合规则
    bool validateName(const QString &password);
    void showAlertMessage(const QString &errMsg);

Q_SIGNALS:
    void removeClicked();
    void editClicked(bool state);
    void editTextFinished(QString finger);

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QHBoxLayout *m_layout;
    QLabel *m_title;
    DIconButton *m_removeBtn;
    DIconButton *m_editBtn;
    DLineEdit *m_editTitle;
    QString m_itemName;
    DPalette m_currentpa;
};

}
}

