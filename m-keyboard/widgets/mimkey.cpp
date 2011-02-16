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
#include <MTimestamp>

#include <QPainter>
#include <QFileInfo>

namespace {
    qreal computeWidth(qreal unit, qreal spacing, const qreal newScaling)
    {
        const qreal scaling = qMax<qreal>(0.0, newScaling);
        return ((scaling * unit) + (qMax<qreal>(0.0, scaling - 1) * spacing));
    }

    // Modify font to make text fit into boundingRect
    // TODO: Could be optimized to use a binary search.
    void scaleDownFont(QFont *font, const QString& text, const QRect& boundingRect)
    {
        // Fonts can either be specified in points or pixels
        int fontSize = font->pixelSize();
        const bool usesPixelSize = (fontSize == -1) ? false : true;

        if (!usesPixelSize) {
            fontSize = font->pointSize();
            Q_ASSERT(fontSize != -1);
        }

        // Minimum font size is 1
        while (fontSize > 1) {
            if (usesPixelSize) {
                font->setPixelSize(fontSize);
            }
            else {
                font->setPointSize(fontSize);
            }

            const QFontMetrics fontMetrics(*font);
            const QRect textBounds = fontMetrics.boundingRect(text);

            if (textBounds.width() <= boundingRect.width()
                && textBounds.height() <= boundingRect.height()) {
                break;
            }
            --fontSize;
        }

    }

    const char * const KeyBackground = "keyBackground";

    const char * const SpecialStyleName = "Special";
    const char * const DeadkeyStyleName = "Deadkey";
    const char * const NormalStyleName = "";

    const char * const NormalStateName = "";
    const char * const PressedStateName = "Pressed";
    const char * const SelectedStateName = "Selected";

    const char * const HighlightedName = "Highlighted";
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
      stylingCache(newStylingCache),
      overrideIcon(0)
{
    if (mModel.binding(false)) {
        loadIcon(false);
    }
    if (mModel.binding(true)) {
        loadIcon(true);
    }

    labelFont = style->font();
    hide();

    //label position should be computed later, when geometry will be known
}

MImKey::~MImKey()
{
}

const QString MImKey::label() const
{
    if (override && !override->label().isEmpty())
        return override->label();

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

void MImKey::invalidateLabelPos()
{
    labelArea = QRectF();
    secondaryLabelArea = QRectF();

    updateLabelFont();
}

void MImKey::updateLabelFont()
{
    // Use a maximum label rectangle that is a bit smaller than the button
    const QRect maximumLabelRect = buttonRect().adjusted(0, 0, -10, -5).toRect();
    labelFont = styleContainer->font();
    scaleDownFont(&labelFont, label(), maximumLabelRect);
}

void MImKey::updateLabelPos() const
{
    const QRectF paintingArea(currentGeometry.marginLeft,
                              currentGeometry.marginTop,
                              currentGeometry.width,
                              currentGeometry.height);

    if (!rowHasSecondaryLabel) {
        labelArea = paintingArea;
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
            labelArea = QRectF(paintingArea.left(),
                               primaryY,
                               paintingArea.width(),
                               labelHeight);
            if (!secondaryLabel().isEmpty()) {
                secondaryLabelArea = QRectF(paintingArea.left(),
                                            labelArea.bottom() + secondarySeparation,
                                            paintingArea.width(),
                                            secondaryLabelHeight);
            }
        } else {
            // primary label: horizontally according to left margin, vertically centered
            // secondary: horizontally on right of primary + separation margin, vertically centered
            const int primaryX = paintingArea.left() + labelLeftWithSecondary;
            labelArea = QRectF(primaryX,
                               paintingArea.top(),
                               stylingCache->primary.width(label()),
                               paintingArea.height());
            if (!secondaryLabel().isEmpty()) {
                secondaryLabelArea = QRectF(labelArea.right() + secondarySeparation,
                                            paintingArea.top(),
                                            stylingCache->secondary.width(secondaryLabel()),
                                            paintingArea.height());
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

        invalidateLabelPos();
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
            hasGravity = false;
        }

        setVisible((currentState != Normal) || override);
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
    QString backgroundProperty(KeyBackground);

    switch (model().style()) {
    case MImKeyModel::SpecialStyle:
        backgroundProperty.append(SpecialStyleName);
        break;
    case MImKeyModel::DeadkeyStyle:
        backgroundProperty.append(DeadkeyStyleName);
        break;
    case MImKeyModel::NormalStyle:
    default:
        backgroundProperty.append(NormalStateName);
        break;
    }

    switch (state()) {
    case MImAbstractKey::Pressed:
        backgroundProperty.append(PressedStateName);
        break;
    case MImAbstractKey::Selected:
        backgroundProperty.append(SelectedStateName);
        break;
    case MImAbstractKey::Normal:
    default:
        backgroundProperty.append(NormalStateName);
        break;
    }

    if (override && override->highlighted()) {
        backgroundProperty.append(HighlightedName);
    }

    background = getCSSProperty<const MScalableImage *>(styleContainer, backgroundProperty, false);
    return background;
}

QRectF MImKey::boundingRect() const
{
    return QRectF(QPointF(), buttonBoundingRect().size());
}

void MImKey::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    mTimestamp("MImKey", "start");

    // We use QGraphicsView::DontSavePainterState, so save/restore state manually
    // Not strictly needed at the moment, but prevents subtle breakage later
    painter->save();
    const MScalableImage *background = backgroundImage();
    const QRectF paintingArea(currentGeometry.marginLeft,
                              currentGeometry.marginTop,
                              currentGeometry.width,
                              currentGeometry.height);
    const QPixmap *iconPixmap(icon());

    if (background) {
        background->draw(paintingArea, painter);
    }

    if (overrideIcon) {
        QPointF iconPos(paintingArea.left() + (paintingArea.width() - overrideIcon->width()) / 2,
                        paintingArea.top() + (paintingArea.height() - overrideIcon->height()) / 2);
        painter->drawPixmap(iconPos, *overrideIcon);

    } else if (iconPixmap && (!override || override->label().isEmpty())) {
        QPointF iconPos(paintingArea.left() + (paintingArea.width() - iconPixmap->width()) / 2,
                        paintingArea.top() + (paintingArea.height() - iconPixmap->height()) / 2);
        painter->drawPixmap(iconPos, *iconPixmap);

    } else {
        painter->setFont(font());
        painter->setPen(styleContainer->fontColor());
        painter->drawText(labelRect(), Qt::AlignCenter, label());
        if (!secondaryLabel().isEmpty()) {
            painter->setFont(styleContainer->secondaryFont());
            painter->drawText(secondaryLabelArea, Qt::AlignCenter, secondaryLabel());
        }
    }
    painter->restore();

    mTimestamp("MImKey", "end");
}

void MImKey::setKeyOverride(const QSharedPointer<MKeyOverride> &newOverride)
{
    if (newOverride == override)
        return;

    MKeyOverride::KeyOverrideAttributes changedAttributes;
    QString label;
    QString icon;
    bool highlighted = false;
    bool enabled = true;

    if (override) {
        label = override->label();
        icon = override->icon();
        highlighted = override->highlighted();
        enabled = override->enabled();
    }

    // check label
    if (label != newOverride->label()) {
        changedAttributes |= MKeyOverride::Label;
    }

    // check icon.
    if (icon != newOverride->icon()) {
        changedAttributes |= MKeyOverride::Icon;
    }

    // check highlighted
    if (highlighted != newOverride->highlighted()) {
        changedAttributes |= MKeyOverride::Highlighted;
    }

    // check enabled
    if (enabled != newOverride->enabled()) {
        changedAttributes |= MKeyOverride::Enabled;
    }

    override = newOverride;

    if (changedAttributes) {
        updateOverrideAttributes(changedAttributes);
    }
}

QSharedPointer<MKeyOverride> MImKey::keyOverride() const
{
    return override;
}

void MImKey::resetKeyOverride()
{
    override.clear();
    //TODO: reset appearance.
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
    if (rowHasSecondaryLabel != enable) {
        rowHasSecondaryLabel = enable;
        invalidateLabelPos();
    }
}

const QRectF & MImKey::labelRect() const
{
    if (labelArea.isNull()) {
        updateLabelPos();
    }

    return labelArea;
}

const QRectF & MImKey::secondaryLabelRect() const
{
    if (labelArea.isNull()) {
        updateLabelPos();
    }

    return secondaryLabelArea;
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

void MImKey::loadOverrideIcon()
{
    delete overrideIcon;
    overrideIcon = 0;
    if (override) {
        QFileInfo fileInfo(override->icon());
        if (fileInfo.exists() && fileInfo.isAbsolute() && fileInfo.isFile()) {
            overrideIcon = new QPixmap(override->icon());
        }
    }
}

const MImKey::IconInfo &MImKey::iconInfo() const
{
    return (shift ? upperCaseIcon : lowerCaseIcon);
}

const QFont &MImKey::font() const
{
    return labelFont;
}

void MImKey::updateOverrideAttributes(MKeyOverride::KeyOverrideAttributes changedAttributes)
{
    if (!override || !changedAttributes) {
        return;
    }

    if (changedAttributes & MKeyOverride::Label) {
        invalidateLabelPos();
    }

    if (changedAttributes & MKeyOverride::Icon) {
        loadOverrideIcon();
    }

    // update() is enough for MKeyOverride::Highlighted
    // and it will be called later in this method

    if (changedAttributes & MKeyOverride::Enabled) {
        //TODO: update enabled style
    }

    if (isVisible()) {
        update();
    } else {
        show();
    }
}

