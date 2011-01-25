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

#include <mplainwindow.h>

#include <MTheme>
#include <MScalableImage>

#include <QPainter>

namespace {
    qreal computeWidth(qreal unit, qreal spacing, const qreal newScaling)
    {
        const qreal scaling = qMax<qreal>(0.0, newScaling);
        return ((scaling * unit) + (qMax<qreal>(0.0, scaling - 1) * spacing));
    }
}

MImKey::StylingCache::StylingCache()
    : primary(QFont()),
    secondary(QFont())
{
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

MImKey::Geometry::Geometry()
    : width(0.0)
    , height(0.0)
    , marginLeft(0.0)
    , marginTop(0.0)
    , marginRight(0.0)
    , marginBottom(0.0)
{}

MImKey::Geometry::Geometry(qreal newWidth,
                           qreal newHeight,
                           qreal newMarginLeft,
                           qreal newMarginTop,
                           qreal newMarginRight,
                           qreal newMarginBottom)
    : width(newWidth)
    , height(newHeight)
    , marginLeft(newMarginLeft)
    , marginTop(newMarginTop)
    , marginRight(newMarginRight)
    , marginBottom(newMarginBottom)
{}

MImKey::MImKey(const MImKeyModel &newModel,
               const MImAbstractKeyAreaStyleContainer &style,
               QGraphicsItem &parent,
               const QSharedPointer<StylingCache> &newStylingCache)
    : QGraphicsItem(&parent),
      width(0),
      mModel(newModel),
      shift(false),
      currentLabel(mModel.binding(false)->label()),
      currentState(Normal),
      selected(false),
      styleContainer(style),
      parentItem(parent),
      currentTouchPointCount(0),
      hasGravity(false),
      rowHasSecondaryLabel(false),
      stylingCache(newStylingCache)
{
    if (mModel.binding(false)) {
        loadIcon(false);
    }
    if (mModel.binding(true)) {
        loadIcon(true);
    }

    hide();

    //label position should be computed later, when geometry will be known
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
    return cachedButtonBoundingRect;
}

void MImKey::updateGeometryCache()
{
    const Geometry &g = currentGeometry;
    cachedButtonBoundingRect = QRectF(pos().x(), pos().y(),
                                      g.width + g.marginLeft + g.marginRight,
                                      g.height + g.marginTop + g.marginBottom);
    cachedButtonRect = cachedButtonBoundingRect.adjusted( g.marginLeft,   g.marginTop,
                                                         -g.marginRight, -g.marginBottom);
}

void MImKey::invalidateLabelPos() const
{
    labelPoint = QPointF();
    secondaryLabelPoint = QPointF();
}

void MImKey::updateLabelPos() const
{
    const QRectF paintingArea(currentGeometry.marginLeft,
                              currentGeometry.marginTop,
                              currentGeometry.width,
                              currentGeometry.height);

    if (!rowHasSecondaryLabel) {
        labelPoint = stylingCache->primary.boundingRect(paintingArea.toRect(), Qt::AlignCenter, label()).topLeft();
        labelPoint.ry() += stylingCache->primary.ascent();
    } else {
        const int labelHeight = stylingCache->primary.height();
        const int secondaryLabelHeight = stylingCache->secondary.height();
        const int topMargin = styleContainer->labelMarginTop();
        const int labelLeftWithSecondary = styleContainer->labelMarginLeftWithSecondary();
        const int secondarySeparation = styleContainer->secondaryLabelSeparation();
        const bool landscape = (MPlainWindow::instance()->orientation() == M::Landscape);

        // In landscape the secondary labels are below the primary ones. In portrait,
        // secondary labels are horizontally next to primary labels.
        if (landscape) {
            // primary label: horizontally centered, top margin defines y
            // secondary: horizontally centered, primary bottom + separation margin defines y
            const int primaryY = paintingArea.top() + topMargin;
            labelPoint.setX(paintingArea.center().x() - stylingCache->primary.width(label()) / 2);
            labelPoint.setY(primaryY + stylingCache->primary.ascent());
            if (!secondaryLabel().isEmpty()) {
                secondaryLabelPoint.setX(paintingArea.center().x() - stylingCache->secondary.width(secondaryLabel()) / 2);
                secondaryLabelPoint.setY(primaryY + labelHeight + secondarySeparation + stylingCache->secondary.ascent());
            }
        } else {
            // primary label: horizontally according to left margin, vertically centered
            // secondary: horizontally on right of primary + separation margin, vertically centered
            const int primaryX = paintingArea.left() + labelLeftWithSecondary;
            labelPoint.setX(primaryX);
            labelPoint.setY(paintingArea.center().y() - labelHeight / 2 + stylingCache->primary.ascent());
            if (!secondaryLabel().isEmpty()) {
                secondaryLabelPoint.setX(primaryX + stylingCache->primary.width(label()) + secondarySeparation);
                secondaryLabelPoint.setY(paintingArea.center().y() - secondaryLabelHeight / 2 + stylingCache->secondary.ascent());
            }
        }
    }
}

void MImKey::setModifiers(bool shift, QChar accent)
{
    if (this->shift != shift || this->accent != accent) {
        this->shift = shift;
        this->accent = accent;
        currentLabel = binding().accented(accent);

        updateParent();
        invalidateLabelPos();
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
            hasGravity = false;
        }

        setVisible(currentState != Normal);
        updateParent();
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
    return (binding().action() == MImKeyBinding::ActionShift
            && !isDeadKey());
}

bool MImKey::isNormalKey() const
{
    return (binding().action() == MImKeyBinding::ActionInsert
            && !isDeadKey());
}

bool MImKey::isQuickPick() const
{
    return binding().isQuickPick();
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

void MImKey::activateGravity()
{
    hasGravity = true;
}

bool MImKey::isGravityActive() const
{
    return hasGravity;
}

const MScalableImage * MImKey::backgroundImage() const
{
    const MScalableImage *background = 0;

    switch (state()) {

        case MImAbstractKey::Normal:
            switch (model().style()) {
                case MImKeyModel::SpecialStyle:
                    background = styleContainer->keyBackgroundSpecial();
                    break;
                case MImKeyModel::DeadkeyStyle:
                    background = styleContainer->keyBackgroundDeadkey();
                    break;
                case MImKeyModel::NormalStyle:
                default:
                    background = styleContainer->keyBackground();
                    break;
            }
            break;

        case MImAbstractKey::Pressed:
            switch (model().style()) {
                case MImKeyModel::SpecialStyle:
                    background = styleContainer->keyBackgroundSpecialPressed();
                    break;
                case MImKeyModel::DeadkeyStyle:
                    background = styleContainer->keyBackgroundDeadkeyPressed();
                    break;
                case MImKeyModel::NormalStyle:
                default:
                    background = styleContainer->keyBackgroundPressed();
                    break;
            }
            break;

        case MImAbstractKey::Selected:
            switch (model().style()) {
                case MImKeyModel::SpecialStyle:
                    background = styleContainer->keyBackgroundSpecialSelected();
                    break;
                case MImKeyModel::DeadkeyStyle:
                    background = styleContainer->keyBackgroundDeadkeySelected();
                    break;
                case MImKeyModel::NormalStyle:
                default:
                    background = styleContainer->keyBackgroundSelected();
                    break;
            }
            break;

        default:
            break;
    }

    return background;
}

QRectF MImKey::boundingRect() const
{
    return QRectF(QPointF(), buttonBoundingRect().size());
}

void MImKey::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const MScalableImage *background = backgroundImage();
    const QRectF paintingArea(currentGeometry.marginLeft,
                              currentGeometry.marginTop,
                              currentGeometry.width,
                              currentGeometry.height);
    const QPixmap *iconPixmap(icon());

    if (background) {
        background->draw(paintingArea, painter);
    }

    if (iconPixmap) {
        QPointF iconPos(paintingArea.left() + (paintingArea.width() - iconPixmap->width()) / 2,
                        paintingArea.top() + (paintingArea.height() - iconPixmap->height()) / 2);
        painter->drawPixmap(iconPos, *iconPixmap);
    } else {
        painter->setFont(styleContainer->font());
        painter->setPen(styleContainer->fontColor());
        painter->drawText(labelPos(), label());
        if (!secondaryLabel().isEmpty()) {
            painter->setFont(styleContainer->secondaryFont());
            painter->drawText(secondaryLabelPos(), secondaryLabel());
        }
    }
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

void MImKey::updateParent()
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

const MImKey::Geometry &MImKey::geometry() const
{
    return currentGeometry;
}

void MImKey::setGeometry(const MImKey::Geometry &geometry)
{
    currentGeometry = geometry;
    updateGeometryCache();
    invalidateLabelPos();
}

void MImKey::setWidth(qreal width)
{
    currentGeometry.width = width;
    updateGeometryCache();
}

void MImKey::setHeight(qreal height)
{
    currentGeometry.height = height;
    updateGeometryCache();
}

void MImKey::setMargins(qreal left,
                        qreal top,
                        qreal right,
                        qreal bottom)
{
    currentGeometry.marginLeft = left;
    currentGeometry.marginTop = top;
    currentGeometry.marginRight = right;
    currentGeometry.marginBottom = bottom;
    updateGeometryCache();
}

void MImKey::setSecondaryLabelEnabled(bool enable)
{
    rowHasSecondaryLabel = enable;
    invalidateLabelPos();
}

const QPointF & MImKey::labelPos() const
{
    if (labelPoint.isNull()) {
        updateLabelPos();
    }

    return labelPoint;
}

const QPointF & MImKey::secondaryLabelPos() const
{
    if (labelPoint.isNull()) {
        updateLabelPos();
    }

    return secondaryLabelPoint;
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

