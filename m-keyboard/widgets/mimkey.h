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



#ifndef MIMKEY_H
#define MIMKEY_H

#include "mimabstractkey.h"
#include <QPointF>

class MImAbstractKeyAreaStyleContainer;
class QGraphicsItem;
class MImKeyArea;

//! Represents a key model with the key's current binding state, and also contains its visible area.
class MImKey
    : public MImAbstractKey
{
public:
    struct Geometry {
        QPointF pos;
        qreal width;
        qreal height;
        qreal marginLeft;
        qreal marginTop;
        qreal marginRight;
        qreal marginBottom;

        Geometry();
        Geometry(const QPointF &newPos,
                 qreal newWidth,
                 qreal newHeight,
                 qreal newMarginLeft,
                 qreal newMarginTop,
                 qreal newMarginRight,
                 qreal newMarginBottom);
    };

    explicit MImKey(const MImKeyModel &mModel,
                    const MImAbstractKeyAreaStyleContainer &style,
                    QGraphicsItem &parent);

    virtual ~MImKey();

    //! \reimp
    virtual const QString label() const;
    virtual const QString secondaryLabel() const;
    virtual const QRectF &buttonRect() const;
    virtual const QRectF &buttonBoundingRect() const;
    virtual void setModifiers(bool shift,
                              QChar accent = QChar());
    virtual void setDownState(bool down);
    virtual void setSelected(bool select);
    virtual ButtonState state() const;
    virtual const MImKeyModel &model() const;
    virtual const MImKeyBinding &binding() const;
    virtual bool isDeadKey() const;
    virtual bool isShiftKey() const;
    virtual bool isNormalKey() const;
    virtual bool isQuickPick() const;
    virtual bool increaseTouchPointCount();
    virtual bool decreaseTouchPointCount();
    virtual void resetTouchPointCount();
    virtual int touchPointCount() const;
    virtual void activateGravity();
    virtual bool isGravityActive() const;
    //! \reimp_end

    //! Return limit for active touchpoints
    static int touchPointLimit();

    //! \brief Returns the icon of this button, if it has one.
    const QPixmap *icon() const;

    //! \brief Returns icon identifier, if it was loaded.
    QString iconId() const;

    //! \brief Draws the icon of this key, if available.
    virtual void drawIcon(QPainter *painter) const;
    //! \brief Calls parent item's QGraphicsItem::update() who actually draws the button.
    void update();

    //! Returns preferred fixed witdth
    int preferredFixedWidth() const;

    //! Returns preferred dynamic width
    qreal preferredWidth(qreal pixelPerSizeUnit, qreal spacing) const;

    //! \brief Whether a key belongs to a given graphics item.
    //! \param item the graphics item that logically contains this key
    virtual bool belongsTo(const QGraphicsItem *item) const;

    //! \brief Returns the geometry of the key, used for drawing.
    const MImKey::Geometry &geometry() const;

    //! \brief Set new geometry of the key.
    //! \param geometry the new geometry
    void setGeometry(const MImKey::Geometry &geometry);

    //! \brief Set position (relative to parent item).
    //! \param pos the new position
    void setPos(const QPointF &pos);

    //! \brief Set the key width.
    //! \param width the new width
    void setWidth(qreal width);

    //! \brief Set the key height.
    //! \param height the new height
    void setHeight(qreal height);

    //! \brief Set the margins of the key, used for layouting and reactive areas.
    //! \param left left margin
    //! \param top top margin
    //! \param right right margin
    //! \param bottom bottom margin
    void setMargins(qreal left,
                    qreal top,
                    qreal right,
                    qreal bottom);

    //! The width for this button. Not managed by this class.
    //! It is used by MImKeyArea to store the correct button size.
    qreal width;

private:
    //! Contains information about icon
    struct IconInfo
    {
        //! Actual image
        const QPixmap *pixmap;
        //! Icon identified
        QString id;

        IconInfo();
        ~IconInfo();
    };

    void loadIcon(bool shift);
    const IconInfo &iconInfo() const;
    void updateButtonRects();

    const MImKeyModel &mModel;

    bool shift;
    QChar accent;

    QString currentLabel;
    ButtonState currentState;
    bool selected;

    IconInfo lowerCaseIcon;
    IconInfo upperCaseIcon;

    const MImAbstractKeyAreaStyleContainer &styleContainer;

    QGraphicsItem &parentItem;

    //! Touchpoint count
    int currentTouchPointCount;

    Geometry currentGeometry;
    QRectF cachedButtonRect;
    QRectF cachedButtonBoundingRect;

    bool hasGravity;
};

#endif // MIMKEY_H

