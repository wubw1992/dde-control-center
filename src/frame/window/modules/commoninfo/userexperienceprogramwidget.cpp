// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "userexperienceprogramwidget.h"
#include "window/modules/commoninfo/commoninfomodel.h"
#include "window/utils.h"
#include "widgets/switchwidget.h"
#include "widgets/labels/tipslabel.h"
#include "window/utils.h"

#include <DTipLabel>
#include <QDesktopServices>

#include <QUrl>
#include <QVBoxLayout>
#include <QTimer>

using namespace dcc::widgets;
using namespace DCC_NAMESPACE;
using namespace commoninfo;
DWIDGET_USE_NAMESPACE

UserExperienceProgramWidget::UserExperienceProgramWidget(QWidget *parent)
    : QWidget(parent)
{
    setAccessibleName("UserExperienceProgramWidget");
    QVBoxLayout *vBoxLayout = new QVBoxLayout;

    m_joinUeProgram = new SwitchWidget();
    m_joinUeProgram->addBackground();
    //~ contents_path /commoninfo/User Experience Program
    //~ child_page User Experience Program
    m_joinUeProgram->setTitle(tr("Join User Experience Program"));

    QString text = "";
    QString http = IsCommunitySystem ? tr("https://www.deepin.org/en/agreement/privacy/") : tr("https://www.uniontech.com/agreement/experience-en");
    if (IsCommunitySystem) {
        text = tr("<p>Joining User Experience Program means that you grant and authorize us to collect and use the information of your device, system and applications. "
                  "If you refuse our collection and use of the aforementioned information, do not join User Experience Program. "
                  "For details, please refer to Deepin Privacy Policy (<a href=\"%1\"> %1</a>).</p>")
               .arg(http);
    } else {
        text = tr("<p>Joining User Experience Program means that you grant and authorize us to collect and use the information of your device, system and applications. "
                  "If you refuse our collection and use of the aforementioned information, please do not join it. For the details of User Experience Program, please visit <a href=\"%1\"> %1</a>.</p>")
               .arg(http);
    }

    DTipLabel *label = new DTipLabel(text);

    label->setTextFormat(Qt::RichText);
    label->setAlignment(Qt::AlignJustify | Qt::AlignLeft);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->setWordWrap(true);
    connect(label, &QLabel::linkActivated, this, [](const QString &link) {
        QDesktopServices::openUrl(QUrl(link));
    });

    vBoxLayout->setMargin(0);
    vBoxLayout->setContentsMargins(ThirdPageContentsMargins);
    vBoxLayout->setSpacing(0);
    vBoxLayout->addWidget(m_joinUeProgram);
    vBoxLayout->addSpacing(8);
    vBoxLayout->addWidget(label);
    vBoxLayout->addStretch();

    setLayout(vBoxLayout);

    connect(m_joinUeProgram, &SwitchWidget::checkedChanged, this, [this](bool state) {
        QTimer::singleShot(0, this, [ = ] {
            this->enableUeProgram(state);
        });
    });
}

void UserExperienceProgramWidget::setModel(CommonInfoModel *model)
{
    connect(model, &CommonInfoModel::ueProgramChanged, m_joinUeProgram, &SwitchWidget::setChecked);
}

void UserExperienceProgramWidget::setDefaultUeProgram(const bool enabled)
{
    m_joinUeProgram->setChecked(enabled);
}
