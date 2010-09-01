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



#include "singlewidgetbutton.h"
#include "singlewidgetbuttonarea.h"
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

SingleWidgetButton::IconInfo::IconInfo()
    : pixmap(0)
{
}

SingleWidgetButton::IconInfo::~IconInfo()
{
    if (pixmap) {
        MTheme::releasePixmap(pixmap);
    }
}

SingleWidgetButton::SingleWidgetButton(const VKBDataKey &key,
                                       const MVirtualKeyboardStyleContainer &style,
                                       QGraphicsItem &parent)
    : width(0),
      dataKey(key),
      shift(false),
      currentLabel(dataKey.binding(false)->label()),
      currentState(Normal),
      selected(false),
      styleContainer(style),
      parentItem(parent)
{
    if (dataKey.binding(false)) {
        loadIcon(false);
    }
    if (dataKey.binding(true)) {
        loadIcon(true);
    }
}

SingleWidgetButton::~SingleWidgetButton()
{
}

const QString SingleWidgetButton::label() const
{
    return currentLabel;
}

const QString SingleWidgetButton::secondaryLabel() const
{
    return binding().secondaryLabel();
}

QRectF SingleWidgetButton::buttonRect() const
{
    return cachedButtonRect;
}

QRectF SingleWidgetButton::buttonBoundingRect() const
{
    return cachedBoundingRect;
}

void SingleWidgetButton::setModifiers(bool shift, QChar accent)
{
    if (this->shift != shift || this->accent != accent) {
        this->shift = shift;
        this->accent = accent;
        currentLabel = binding().accented(accent);

        update();
    }
}

void SingleWidgetButton::setDownState(bool down)
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
        update();
    }
}

void SingleWidgetButton::setSelected(bool select)
{
    if (selected != select) {
        selected = select;

        // refresh state
        setDownState(currentState == Pressed);
    }
}

SingleWidgetButton::ButtonState SingleWidgetButton::state() const
{
    return currentState;
}

const VKBDataKey &SingleWidgetButton::key() const
{
    return dataKey;
}

const KeyBinding &SingleWidgetButton::binding() const
{
    return *dataKey.binding(shift);
}

bool SingleWidgetButton::isDeadKey() const
{
    return binding().isDead();
}

const QPixmap *SingleWidgetButton::icon() const
{
    return iconInfo().pixmap;
}

QString SingleWidgetButton::iconId() const
{
    return iconInfo().id;
}

void SingleWidgetButton::drawIcon(const QRect &rectangle, QPainter *painter) const
{
    const QPixmap *iconPixmap = icon();
    if (iconPixmap) {
        QPointF iconPos(rectangle.x() + (rectangle.width() - iconPixmap->width()) / 2,
                        rectangle.y() + (rectangle.height() - iconPixmap->height()) / 2);
        painter->drawPixmap(iconPos, *iconPixmap);
    }
}

void SingleWidgetButton::update()
{
    // Invalidate this button's area.
    parentItem.update(buttonRect());
}

int SingleWidgetButton::preferredFixedWidth() const
{
    switch(dataKey.width()) {
    case VKBDataKey::Small:
        return styleContainer->keyWidthSmallFixed();

    case VKBDataKey::Medium:
        return styleContainer->keyWidthMediumFixed();

    case VKBDataKey::Large:
        return styleContainer->keyWidthLargeFixed();

    case VKBDataKey::XLarge:
        return styleContainer->keyWidthXLargeFixed();

    case VKBDataKey::XxLarge:
        return styleContainer->keyWidthXxLargeFixed();

    // This one of course makes no real sense:
    case VKBDataKey::Stretched:
        return styleContainer->keyWidthStretchedFixed();
    }

    qWarning() << __PRETTY_FUNCTION__
               << "Could not find preferred fixed width in style";
    return -1;
}

qreal SingleWidgetButton::preferredWidth(qreal pixelPerSizeUnit, qreal spacing) const
{
    switch(dataKey.width()) {
    case VKBDataKey::Small:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthSmall());

    case VKBDataKey::Medium:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthMedium());

    case VKBDataKey::Large:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthLarge());

    case VKBDataKey::XLarge:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthXLarge());

    case VKBDataKey::XxLarge:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthXxLarge());

    case VKBDataKey::Stretched:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthStretched());
    }

    qWarning() << __PRETTY_FUNCTION__
               << "Could not find preferred width in style";
    return -1;
}

void SingleWidgetButton::loadIcon(bool shift)
{
    IconInfo &iconInfo(shift ? upperCaseIcon : lowerCaseIcon);
    const KeyBinding::KeyAction action(dataKey.binding(shift)->action());
    QSize size;
    QString iconProperty;

    switch(action) {
        case KeyBinding::ActionBackspace:
            iconProperty = "keyBackspaceIconId";
            size = styleContainer->keyBackspaceIconSize();
            break;
        case KeyBinding::ActionShift:
            if (shift) {
                iconProperty = "keyShiftUppercaseIconId";
            } else {
                iconProperty = "keyShiftIconId";
            }
            size = styleContainer->keyShiftIconSize();
            break;
        case KeyBinding::ActionReturn:
            if (dataKey.binding(shift)->label().isEmpty()) {
                iconProperty = "keyEnterIconId";
                size = styleContainer->keyEnterIconSize();
            }
            break;
        case KeyBinding::ActionLayoutMenu:
            iconProperty = "keyMenuIconId";
            size = styleContainer->keyMenuIconSize();
            break;
        case KeyBinding::ActionTab:
            if (dataKey.binding(shift)->label().isEmpty()) {
                iconProperty = "keyTabIconId";
                size = styleContainer->keyTabIconSize();
            }
            break;
        default:
            break;
    }

    iconInfo.id = getCSSProperty<QString>(styleContainer, iconProperty, dataKey.rtl());

    if (!iconInfo.id.isEmpty()) {
        iconInfo.pixmap = MTheme::pixmap(iconInfo.id, size);
    }
}

const SingleWidgetButton::IconInfo &SingleWidgetButton::iconInfo() const
{
    return (shift ? upperCaseIcon : lowerCaseIcon);
}

