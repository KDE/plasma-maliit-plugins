/*
 * This file is part of meego-keyboard 
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list 
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list 
 * of conditions and the following disclaimer in the documentation and/or other materials 
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be 
 * used to endorse or promote products derived from this software without specific 
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */



#ifndef MIMKEY_H
#define MIMKEY_H

#include "mimabstractkey.h"
#include <QObject>
#include <QPointF>
#include <QGraphicsItem>
#include <QRectF>
#include <QFontMetrics>
#include <QSharedPointer>

class MImAbstractKeyAreaStyleContainer;
class MImKeyArea;
class MScalableImage;

//! Represents a key model with the key's current binding state, and also contains its visible area.
class MImKey
    : public QGraphicsItem,
      public MImAbstractKey
{
public:
    //! Contains cached font information for current style
    struct StylingCache
    {
        QFontMetrics primary;
        QFontMetrics secondary;

        StylingCache();
    };

    struct Geometry {
        qreal width;
        qreal height;
        qreal marginLeft;
        qreal marginTop;
        qreal marginRight;
        qreal marginBottom;

        Geometry();
        Geometry(qreal newWidth,
                 qreal newHeight,
                 qreal newMarginLeft,
                 qreal newMarginTop,
                 qreal newMarginRight,
                 qreal newMarginBottom);
    };

    explicit MImKey(const MImKeyModel &mModel,
                    const MImAbstractKeyAreaStyleContainer &style,
                    QGraphicsItem &parent,
                    const QSharedPointer<StylingCache> &newStylingCache);

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
    virtual void setComposing(bool composing);
    virtual ButtonState state() const;
    virtual bool isComposing() const;
    virtual const MImKeyModel &model() const;
    virtual const MImKeyBinding &binding() const;
    virtual bool isDeadKey() const;
    virtual bool isShiftKey() const;
    virtual bool isNormalKey() const;
    virtual bool isQuickPick() const;
    virtual bool isComposeKey() const;
    virtual bool increaseTouchPointCount();
    virtual bool decreaseTouchPointCount();
    virtual void resetTouchPointCount();
    virtual int touchPointCount() const;
    virtual void activateGravity();
    virtual bool isGravityActive() const;
    virtual const MScalableImage *backgroundImage() const;
    virtual const MScalableImage *normalBackgroundImage() const;
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    virtual void setKeyOverride(const QSharedPointer<MKeyOverride> &override);
    virtual QSharedPointer<MKeyOverride> keyOverride() const;
    virtual void resetKeyOverride();
    virtual void updateOverrideAttributes(MKeyOverride::KeyOverrideAttributes changedAttributes);
    //! \reimp_end

    //! Return limit for active touchpoints
    static int touchPointLimit();

    //! \brief Returns the icon of this button, if it has one.
    const QPixmap *icon() const;

    //! \brief Returns icon identifier, if it was loaded.
    QString iconId() const;

    //! \brief Draws the icon of this key, if available.
    virtual void drawIcon(QPainter *painter) const;

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

    //! \brief This parameter defines text alignment: we use primary label only if \a enable is false
    //! and both primary and secondary labels if \a enable is false.
    void setSecondaryLabelEnabled(bool enable);

    //! \brief Return rectangle where we should draw primary label
    const QRectF & labelRect() const;

    //! \brief Return rectangle where we should draw secondary label.
    const QRectF & secondaryLabelRect() const;

    //! \brief Invalidates cached position, so next call to getter will calculate it again
    void invalidateLabelPos();

    //! \brief Updates cached geometry.
    //! This method must be called when position or size of this key is changed.
    void updateGeometryCache();

    //! \brief Return the font of this key.
    const QFont &font() const;

    //! \brief Disable attribute overriding. This method allows you to get original label or background image.
    //! \param ignore Set this parameter to true if you want to get original key's attributes.
    void setIgnoreOverriding(bool ignore);

    //! \brief Override key's binding.
    void overrideBinding(const MImKeyBinding* binding);

    //! The width for this button. Not managed by this class.
    //! It is used by MImKeyArea to store the correct button size.
    qreal width;

private:
    Q_DISABLE_COPY(MImKey);

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

    void loadOverrideIcon(const QString &icon);

    const IconInfo &iconInfo() const;
    //! \brief Update cached label position.
    void updateLabelPos() const;
    //! \brief Update label font.
    void updateLabelFont();

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

    //! Some key in the same row has secondary label
    bool rowHasSecondaryLabel;

    //! Cached position of primary label
    mutable QRectF labelArea;
    //! Cached position of secondary label
    mutable QRectF secondaryLabelArea;

    //! Primary label font
    QFont labelFont;

    const QSharedPointer<StylingCache> stylingCache;

    QSharedPointer<MKeyOverride> override;

    QPixmap *overrideIcon;

    bool ignoreOverride;

    bool composing;
};

#endif // MIMKEY_H

