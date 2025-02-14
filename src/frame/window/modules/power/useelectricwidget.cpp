// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "useelectricwidget.h"

#include <QPushButton>
#include <QtMath>
#include <QComboBox>
#include <QGSettings>

#include "widgets/titledslideritem.h"
#include "widgets/dccslider.h"
#include "widgets/dccsliderannotated.h"
#include "widgets/optionitem.h"
#include "widgets/switchwidget.h"
#include "widgets/settingsgroup.h"
#include "modules/power/powermodel.h"
#include "modules/power/powerworker.h"
#include "window/utils.h"
#include "window/gsettingwatcher.h"
#include "widgets/comboxwidget.h"
#include "widgets/contentwidget.h"

using namespace dcc;
using namespace dcc::widgets;
using namespace dcc::power;
using namespace DCC_NAMESPACE;
using namespace DCC_NAMESPACE::power;

const QString gsetting_systemSuspend = "systemSuspend";

UseElectricWidget::UseElectricWidget(PowerModel *model, QWidget *parent, dcc::power::PowerWorker *work)
    : QWidget(parent)
    , m_model(model)
    , m_work(work)
    , m_layout(new QVBoxLayout)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //~ contents_path /power/Plugged In
    //~ child_page Plugged In
    m_autoLockScreen = new TitledSliderItem(tr("Lock screen after"));
    //~ contents_path /power/Plugged In
    //~ child_page Plugged In
    m_cmbPowerBtn = new ComboxWidget(tr("When pressing the power button"));
    //~ contents_path /power/Plugged In
    //~ child_page Plugged In
    m_cmbCloseLid = new ComboxWidget(tr("When the lid is closed"));
    //~ contents_path /power/Plugged In
    //~ child_page Plugged In
    m_autoLockScreen->setAccessibleName(tr("Lock screen after"));

    SettingsGroup *powerSettingsGrp = new SettingsGroup;
    powerSettingsGrp->setSpacing(List_Interval);

    //~ contents_path /power/Plugged In
    //~ child_page Plugged In
    m_monitorSleepOnPower = new TitledSliderItem(tr("Monitor will suspend after"));
    m_monitorSleepOnPower->setAccessibleName(tr("Monitor will suspend after"));
    m_monitorSleepOnPower->slider()->setType(DCCSlider::Vernier);
    m_monitorSleepOnPower->slider()->setRange(1, 7);
    m_monitorSleepOnPower->slider()->setTickPosition(QSlider::TicksBelow);
    m_monitorSleepOnPower->slider()->setTickInterval(1);
    m_monitorSleepOnPower->slider()->setPageStep(1);
    connect(m_monitorSleepOnPower->slider(), &DCCSlider::valueChanged, this, &UseElectricWidget::requestSetScreenBlackDelayOnPower);
    powerSettingsGrp->appendItem(m_monitorSleepOnPower);

    if (!IsServerSystem) {
        //~ contents_path /power/Plugged In
        //~ child_page Plugged In
        m_computerSleepOnPower = new TitledSliderItem(tr("Computer will suspend after"));
        m_computerSleepOnPower->setAccessibleName(tr("Computer will suspend after"));
        m_computerSleepOnPower->slider()->setType(DCCSlider::Vernier);
        m_computerSleepOnPower->slider()->setRange(1, 7);
        m_computerSleepOnPower->slider()->setTickPosition(QSlider::TicksBelow);
        m_computerSleepOnPower->slider()->setTickInterval(1);
        m_computerSleepOnPower->slider()->setPageStep(1);
        connect(m_computerSleepOnPower->slider(), &DCCSlider::valueChanged, this, &UseElectricWidget::requestSetSleepDelayOnPower);
        powerSettingsGrp->appendItem(m_computerSleepOnPower);
    }

    powerSettingsGrp->appendItem(m_autoLockScreen);
    powerSettingsGrp->appendItem(m_cmbCloseLid);
    powerSettingsGrp->appendItem(m_cmbPowerBtn);
    // 使用GSettings来控制显示状态
    GSettingWatcher::instance()->bind("powerLidPresent", m_cmbCloseLid);
    GSettingWatcher::instance()->bind("powerPressPowerbtn", m_cmbPowerBtn);
    GSettingWatcher::instance()->bind("powerAutoLockscreen", m_autoLockScreen);
    GSettingWatcher::instance()->bind("powerMonitorConfigure", m_monitorSleepOnPower);
    GSettingWatcher::instance()->bind(gsetting_systemSuspend, m_computerSleepOnPower);

    m_layout->setContentsMargins(10, 10, 2, 5);
    m_layout->addWidget(powerSettingsGrp);
    m_layout->setAlignment(Qt::AlignTop);
    m_layout->setSpacing(10);

    ContentWidget *contentWgt = new ContentWidget;
    contentWgt->setAccessibleName("UseElectricWidget_Content");
    QWidget *mainWgt = new TranslucentFrame;
    mainWgt->setLayout(m_layout);
    contentWgt->setContent(mainWgt);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(contentWgt);
    mainLayout->setContentsMargins(0, 0, 8, 8);
    setLayout(mainLayout);

    if (!IsServerSystem) {
        m_linePowerSleepDelayConf = m_model->getLinePowerSleepDelayConf();
        m_linePowerSleepDelayConf << tr("Never");
        m_computerSleepOnPower->setAnnotations(m_linePowerSleepDelayConf);
    }
    m_linePowerScreenBlackDelayConf = m_model->getLinePowerScreenBlackDelayConf();
    m_linePowerScreenBlackDelayConf << tr("Never");
    m_monitorSleepOnPower->setAnnotations(m_linePowerScreenBlackDelayConf);

    m_autoLockScreen->slider()->setType(DCCSlider::Vernier);
    m_autoLockScreen->slider()->setRange(1, 7);
    m_autoLockScreen->slider()->setTickPosition(QSlider::TicksBelow);
    m_autoLockScreen->slider()->setTickInterval(1);
    m_autoLockScreen->slider()->setPageStep(1);
    m_linePowerLockDelayConf = m_model->getLinePowerLockDelayConf();
    m_linePowerLockDelayConf << tr("Never");
    m_autoLockScreen->setAnnotations(m_linePowerLockDelayConf);

    setModel(model);

    connect(m_autoLockScreen->slider(), &DCCSlider::valueChanged, this, &UseElectricWidget::requestSetAutoLockScreenOnPower);

    connect(m_cmbPowerBtn->comboBox(), &AlertComboBox::clicked, this, [ = ]() {
        updatePowerButtonActionList();
        setPowerBtn(model, model->linePowerPressPowerBtnAction());
        setCloseLid(model, model->linePowerLidClosedAction());
    });


    connect(m_cmbPowerBtn, &ComboxWidget::onIndexChanged, this, [ = ](int nIndex) {
        if (nIndex < 0) {
            return;
        }

        int option = m_cmbPowerBtn->comboBox()->itemData(nIndex).toInt();

        if (option < PowerModel::Shutdown || option > PowerModel::DoNothing) {
            return;
        }

        Q_EMIT requestSetLinePowerPressPowerBtnAction(option);
    });

    connect(m_cmbCloseLid->comboBox(), &AlertComboBox::clicked, this, [ = ]() {
        updatePowerButtonActionList();
        setPowerBtn(model, model->linePowerPressPowerBtnAction());
        setCloseLid(model, model->linePowerLidClosedAction());
    });
    connect(m_cmbCloseLid, &ComboxWidget::onIndexChanged, [ = ](int nIndex) {
        if (nIndex < 0) {
            return;
        }

        int option = m_cmbCloseLid->comboBox()->itemData(nIndex).toInt();

        if (option < PowerModel::Shutdown || option > PowerModel::DoNothing) {
            return;
        }

        Q_EMIT requestSetLinePowerLidClosedAction(option);
    });

}

UseElectricWidget::~UseElectricWidget()
{
    GSettingWatcher::instance()->erase("powerLidPresent", m_cmbCloseLid);
    GSettingWatcher::instance()->erase("powerPressPowerbtn", m_cmbPowerBtn);
    GSettingWatcher::instance()->erase("powerAutoLockscreen", m_autoLockScreen);
    GSettingWatcher::instance()->erase("powerMonitorConfigure", m_monitorSleepOnPower);
    GSettingWatcher::instance()->erase(gsetting_systemSuspend, m_computerSleepOnPower);
}

void UseElectricWidget::setModel(const PowerModel *model)
{
    connect(model, &PowerModel::powerLockScreenDelayChanged, this, &UseElectricWidget::setLockScreenAfter);
    connect(model, &PowerModel::hibernateChanged, this, [ = ] {
        updatePowerButtonActionList();
        setPowerBtn(model, model->linePowerPressPowerBtnAction());
        setCloseLid(model, model->linePowerLidClosedAction());
    });
    connect(model, &PowerModel::suspendChanged, this, [ = ] {
        updatePowerButtonActionList();
        setPowerBtn(model, model->linePowerPressPowerBtnAction());
        setCloseLid(model, model->linePowerLidClosedAction());
    });

    connect(model, &PowerModel::shutdownChanged, this, [ = ] {
        updatePowerButtonActionList();
        setPowerBtn(model, model->linePowerPressPowerBtnAction());
        setCloseLid(model, model->linePowerLidClosedAction());
    });

    if (!IsServerSystem) {
        connect(model, &PowerModel::sleepDelayChangedOnPower, this, &UseElectricWidget::setSleepDelayOnPower);
        setSleepDelayOnPower(model->sleepDelayOnPower());
    }

    setLockScreenAfter(model->getPowerLockScreenDelay());

    if (m_monitorSleepOnPower) {
        connect(model, &PowerModel::screenBlackDelayChangedOnPower, this, &UseElectricWidget::setScreenBlackDelayOnPower);
        setScreenBlackDelayOnPower(model->screenBlackDelayOnPower());
    }

    // 是否允许待机的条件已经在model的中处理，包括硬件和gsetting配置，这里只需要读取getSuspend
    // systemSuspend配置中不止设置是否显示，还设置是否可用，需要同时判断处理
    if (m_computerSleepOnPower) {
        connect(model, &PowerModel::suspendChanged, this, [ = ] (bool suspend) {
            m_computerSleepOnPower->setEnabled(GSettingWatcher::instance()->getStatus(gsetting_systemSuspend) == "Enabled");
            m_computerSleepOnPower->setVisible(suspend && GSettingWatcher::instance()->getStatus(gsetting_systemSuspend) != "Hidden");
        });

        connect(GSettingWatcher::instance(), &GSettingWatcher::notifyGSettingsChanged, this, [ = ] (const QString &key, const QString &value) {
            if (key == gsetting_systemSuspend && !value.isEmpty()) {
                m_computerSleepOnPower->setEnabled(value == "Enabled");
                m_computerSleepOnPower->setVisible(model->getSuspend() && value != "Hidden");

                updatePowerButtonActionList();
                setPowerBtn(model, model->linePowerPressPowerBtnAction());
                setCloseLid(model, model->linePowerLidClosedAction());
            }
        });

        m_computerSleepOnPower->setEnabled(GSettingWatcher::instance()->getStatus(gsetting_systemSuspend) == "Enabled");
        m_computerSleepOnPower->setVisible(model->getSuspend() && GSettingWatcher::instance()->getStatus(gsetting_systemSuspend) != "Hidden");
    }

    //--------------sp2 add-----------------
    connect(model, &PowerModel::lidPresentChanged, this, [ = ](bool value) {
        m_cmbCloseLid->setVisible(value && GSettingWatcher::instance()->getStatus("powerLidPresent") != "Hidden");
    });
    m_cmbCloseLid->setVisible(model->lidPresent() && GSettingWatcher::instance()->getStatus("powerLidPresent") != "Hidden");
    connect(model, &PowerModel::linePowerLidClosedActionChanged, this, [ = ] (const int reply) {
         setCloseLid(model, reply);
    });
    connect(model, &PowerModel::linePowerPressPowerBtnActionChanged, this, [ = ] (const int reply) {
         setPowerBtn(model, reply);
    });
    //----------------------------------------

    updatePowerButtonActionList();
    setCloseLid(model, model->linePowerLidClosedAction());
    setPowerBtn(model, model->linePowerPressPowerBtnAction());
}

void UseElectricWidget::setLidClose(bool state)
{
    m_cmbCloseLid->setVisible(state);
}

void UseElectricWidget::setScreenBlackDelayOnPower(const int delay)
{
    m_monitorSleepOnPower->slider()->blockSignals(true);
    m_monitorSleepOnPower->slider()->setValue(delay);
    m_monitorSleepOnPower->setValueLiteral(delayToLiteralString(m_linePowerScreenBlackDelayConf, delay));
    m_monitorSleepOnPower->slider()->blockSignals(false);
}

void UseElectricWidget::setSleepDelayOnPower(const int delay)
{
    m_computerSleepOnPower->slider()->blockSignals(true);
    m_computerSleepOnPower->slider()->setValue(delay);
    m_computerSleepOnPower->setValueLiteral(delayToLiteralString(m_linePowerSleepDelayConf, delay));
    m_computerSleepOnPower->slider()->blockSignals(false);
}

void UseElectricWidget::setLockScreenAfter(const int delay)
{
    m_autoLockScreen->slider()->blockSignals(true);
    m_autoLockScreen->slider()->setValue(delay);
    m_autoLockScreen->setValueLiteral(delayToLiteralString(m_linePowerLockDelayConf, delay));
    m_autoLockScreen->slider()->blockSignals(false);
}

void UseElectricWidget::setAutoLockScreenOnPower(const int delay)
{
    m_autoLockScreen->slider()->blockSignals(true);
    m_autoLockScreen->slider()->setValue(delay);
    m_autoLockScreen->setValueLiteral(delayToLiteralString(m_linePowerLockDelayConf, delay));
    m_autoLockScreen->slider()->blockSignals(false);
}

void UseElectricWidget::setCloseLid(const dcc::power::PowerModel *model, int option)
{
    Q_UNUSED(model);

    int tmpIndex = m_cmbCloseLid->comboBox()->count() - 1;

    for (int i = 0; i < m_cmbCloseLid->comboBox()->count(); i++) {
        if (option == m_cmbCloseLid->comboBox()->itemData(i).toInt()) {
           tmpIndex = i;
           break;
        };
    }

    m_cmbCloseLid->setCurrentIndex(tmpIndex);
}

void UseElectricWidget::setPowerBtn(const dcc::power::PowerModel *model, int option)
{
    Q_UNUSED(model);

    int tmpIndex = m_cmbPowerBtn->comboBox()->count() - 1;

    for (int i = 0; i < m_cmbPowerBtn->comboBox()->count(); i++) {
        if (option == m_cmbPowerBtn->comboBox()->itemData(i).toInt()) {
           tmpIndex = i;
           break;
        };
    }

    m_cmbPowerBtn->setCurrentIndex(tmpIndex);
}

void UseElectricWidget::updatePowerButtonActionList()
{
    if (!m_model) {
        return;
    }

    ActionList options;
    if (m_model->getShutdown()) {
        options.insert(PowerModel::Shutdown, tr("Shut down"));
    }
    if (m_model->getSuspend() && GSettingWatcher::instance()->getStatus(gsetting_systemSuspend) != "Hidden") {
        options.insert(PowerModel::Suspend, tr("Suspend"));
    }
    if (m_model->getHibernate()) {
        options.insert(PowerModel::Hibernate, tr("Hibernate"));
    }
    options.insert(PowerModel::TurnOffScreen, tr("Turn off the monitor"));
    options.insert(PowerModel::ShowSessionUI, tr("Show the shutdown interface"));
    options.insert(PowerModel::DoNothing, tr("Do nothing"));
    setComboxOption(m_cmbPowerBtn, options);
    m_cmbPowerBtn->addBackground();
    // 合盖操作无关机选项
    options.remove(PowerModel::Shutdown);
    options.remove(PowerModel::ShowSessionUI);
    setComboxOption(m_cmbCloseLid, options);
    m_cmbCloseLid->addBackground();
}

QString UseElectricWidget::delayToLiteralString(const QStringList& conf, const int delay) const
{
    if (delay > conf.size()) {
        return tr("Never");
    }
    int num;
    if (conf[delay - 1].isEmpty()) {
        qWarning() << "config is empty!";
        num = 0;
    }
    bool ok;
    num = conf[delay - 1].mid(0, conf[delay - 1].length() - 1).toInt(&ok);
    if (!ok) {
        qWarning() << "change to int failed!";
        num = 0;
    }
    if (num == 0) {
        return tr("Never");
    }
    QString strData;
    if (conf[delay - 1].contains("m")) {
        strData = num > 1 ? tr("%1 Minutes").arg(num) : tr("1 Minute");
    } else if (conf[delay - 1].contains("h")) {
        strData = num > 1 ? tr("%1 Hours").arg(num) : tr("1 Hour");
    } else {
        return tr("Never");
    }
    qDebug() << "strData:" << strData;
    return strData;
}

void UseElectricWidget::setComboxOption(ComboxWidget *combox, const ActionList options)
{
    combox->comboBox()->blockSignals(true);
    combox->comboBox()->clear();
    for (int key : options.keys()) {
        combox->comboBox()->addItem(options.value(key), key);
    }
    combox->comboBox()->blockSignals(false);
}
