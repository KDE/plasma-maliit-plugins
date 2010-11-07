/* * This file is part of meego-keyboard *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */



#include "mimkey.h"
#include "mimkeyarea.h"
#include "mvirtualkeyboardstyle.h"
#include "getcssproperty.h"

#include <MTheme>
#include <QGraphicsItem>
#include <QPainter>

namespace {
    qreal computeWidth(qreal unit, qreal spacing, const qreal newScaling)
    {
        const qreal scaling = qMax<qreal>(0.0, newScaling);
        return ((scaling * unit) + (qMax<qreal>(0.0, scaling - 1) * spacing));
    }
}

MImKey::IconInfo::IconInfo()
    : pixmap(0)
{
}

MImKey::IconInfo::~IconInfo()
{
    if (pixmap) {
        MTheme::releasePixmap(pixmap);
    }
}

MImKey::MImKey(const MImKeyModel &newModel,
               const MImAbstractKeyAreaStyleContainer &style,
               QGraphicsItem &parent)
    : width(0),
      mModel(newModel),
      shift(false),
      currentLabel(mModel.binding(false)->label()),
      currentState(Normal),
      selected(false),
      styleContainer(style),
      parentItem(parent),
      currentTouchPointCount(0)
{
    if (mModel.binding(false)) {
        loadIcon(false);
    }
    if (mModel.binding(true)) {
        loadIcon(true);
    }
}

MImKey::~MImKey()
{
}

const QString MImKey::label() const
{
    return currentLabel;
}

const QString MImKey::secondaryLabel() const
{
    return binding().secondaryLabel();
}

const QRectF &MImKey::buttonRect() const
{
    return cachedButtonRect;
}

const QRectF &MImKey::buttonBoundingRect() const
{
    return cachedBoundingRect;
}

void MImKey::setModifiers(bool shift, QChar accent)
{
    if (this->shift != shift || this->accent != accent) {
        this->shift = shift;
        this->accent = accent;
        currentLabel = binding().accented(accent);

        update();
    }
}

void MImKey::setDownState(bool down)
{
    ButtonState newState;

    if (down) {
        // Pressed state is the same for selectable and non-selectable.
        newState = Pressed;
    } else {
        newState = (selected ? Selected : Normal);
    }

    if (newState != currentState) {
        currentState = newState;

        if ((currentState == Pressed || currentState == Selected)
            && (not activeKeys.contains(this))) {
            activeKeys.append(this);
        } else { // currentState == Normal
            activeKeys.removeAll(this);
        }

        update();
    }
}

void MImKey::setSelected(bool select)
{
    if (selected != select) {
        selected = select;

        // refresh state
        setDownState(currentState == Pressed);
    }
}

MImKey::ButtonState MImKey::state() const
{
    return currentState;
}

const MImKeyModel &MImKey::model() const
{
    return mModel;
}

const MImKeyBinding &MImKey::binding() const
{
    return *mModel.binding(shift);
}

bool MImKey::isDeadKey() const
{
    return binding().isDead();
}

bool MImKey::isShiftKey() const
{
    return binding().action() == MImKeyBinding::ActionShift;
}

bool MImKey::isNormalKey() const
{
    return binding().action() == MImKeyBinding::ActionInsert;
}

bool MImKey::increaseTouchPointCount()
{
    if (++currentTouchPointCount <= touchPointLimit()) {
        if (currentTouchPointCount > 0) {
            setDownState(true);
        }

        return true;
    } else {
        --currentTouchPointCount;
        return false;
    }
}

bool MImKey::decreaseTouchPointCount()
{
    if (--currentTouchPointCount >= 0) {
        if (currentTouchPointCount == 0) {
            setDownState(false);
        }

        return true;
    } else {
        ++currentTouchPointCount;
        return false;
    }
}

void MImKey::resetTouchPointCount()
{
    while (decreaseTouchPointCount())
    {}
}

int MImKey::touchPointCount() const
{
    return currentTouchPointCount;
}

int MImKey::touchPointLimit()
{
    return 20;
}

const QPixmap *MImKey::icon() const
{
    return iconInfo().pixmap;
}

QString MImKey::iconId() const
{
    return iconInfo().id;
}

void MImKey::drawIcon(QPainter *painter) const
{
    const QPixmap *iconPixmap(icon());
    const QRect rectangle(buttonRect().toRect());

    if (iconPixmap) {
        QPointF iconPos(rectangle.x() + (rectangle.width() - iconPixmap->width()) / 2,
                        rectangle.y() + (rectangle.height() - iconPixmap->height()) / 2);
        painter->drawPixmap(iconPos, *iconPixmap);
    }
}

void MImKey::update()
{
    // Invalidate this button's area.
    parentItem.update(buttonRect());
}

int MImKey::preferredFixedWidth() const
{
    switch(mModel.width()) {
    case MImKeyModel::Small:
        return styleContainer->keyWidthSmallFixed();

    case MImKeyModel::Medium:
        return styleContainer->keyWidthMediumFixed();

    case MImKeyModel::Large:
        return styleContainer->keyWidthLargeFixed();

    case MImKeyModel::XLarge:
        return styleContainer->keyWidthXLargeFixed();

    case MImKeyModel::XxLarge:
        return styleContainer->keyWidthXxLargeFixed();

    // This one of course makes no real sense:
    case MImKeyModel::Stretched:
        return styleContainer->keyWidthStretchedFixed();
    }

    qWarning() << __PRETTY_FUNCTION__
               << "Could not find preferred fixed width in style";
    return -1;
}

qreal MImKey::preferredWidth(qreal pixelPerSizeUnit, qreal spacing) const
{
    switch(mModel.width()) {
    case MImKeyModel::Small:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthSmall());

    case MImKeyModel::Medium:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthMedium());

    case MImKeyModel::Large:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthLarge());

    case MImKeyModel::XLarge:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthXLarge());

    case MImKeyModel::XxLarge:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthXxLarge());

    case MImKeyModel::Stretched:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthStretched());
    }

    qWarning() << __PRETTY_FUNCTION__
               << "Could not find preferred width in style";
    return -1;
}

bool MImKey::belongsTo(const QGraphicsItem *item) const
{
    return (item && (&parentItem == item));
}
void MImKey::loadIcon(bool shift)
{
    IconInfo &iconInfo(shift ? upperCaseIcon : lowerCaseIcon);
    const MImKeyBinding::KeyAction action(mModel.binding(shift)->action());
    QSize size;
    QString iconProperty;

    switch(action) {
        case MImKeyBinding::ActionBackspace:
            iconProperty = "keyBackspaceIconId";
            size = styleContainer->keyBackspaceIconSize();
            break;
        case MImKeyBinding::ActionShift:
            if (shift) {
                iconProperty = "keyShiftUppercaseIconId";
            } else {
                iconProperty = "keyShiftIconId";
            }
            size = styleContainer->keyShiftIconSize();
            break;
        case MImKeyBinding::ActionReturn:
            if (mModel.binding(shift)->label().isEmpty()) {
                iconProperty = "keyEnterIconId";
                size = styleContainer->keyEnterIconSize();
            }
            break;
        case MImKeyBinding::ActionLayoutMenu:
            iconProperty = "keyMenuIconId";
            size = styleContainer->keyMenuIconSize();
            break;
        case MImKeyBinding::ActionTab:
            if (mModel.binding(shift)->label().isEmpty()) {
                iconProperty = "keyTabIconId";
                size = styleContainer->keyTabIconSize();
            }
            break;
        default:
            break;
    }

    iconInfo.id = getCSSProperty<QString>(styleContainer, iconProperty, mModel.rtl());

    if (!iconInfo.id.isEmpty()) {
        iconInfo.pixmap = MTheme::pixmap(iconInfo.id, size);
    }
}

const MImKey::IconInfo &MImKey::iconInfo() const
{
    return (shift ? upperCaseIcon : lowerCaseIcon);
}

