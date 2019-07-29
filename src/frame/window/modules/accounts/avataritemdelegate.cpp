/*
 * Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     liuhong <liuhong_cm@deepin.com>
 *
 * Maintainer: liuhong <liuhong_cm@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "avataritemdelegate.h"



using namespace DCC_NAMESPACE::accounts;

AvatarItemDelegate::AvatarItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void AvatarItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid()) {
        QString iconPath = index.data(Qt::UserRole+1).value<QString>();
        QPainterPath path;
        path.addEllipse(option.rect);
        painter->setClipPath(path);
        painter->drawPixmap(option.rect, QPixmap(iconPath));
    }
}

QSize AvatarItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QSize(50, 50);
}
