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



#ifndef MIMKEYAREA_H
#define MIMKEYAREA_H

#include "mimabstractkeyarea.h"
#include "mimkey.h"

#include <MScalableImage>
#include <QTextLayout>

/*!
 * \brief MImKeyArea is an implementation of MImAbstractKeyArea which
 * does not use separate widgets for buttons, but instead draws them explicitly.
 */
class MImKeyArea
    : public MImAbstractKeyArea
{
public:
    explicit MImKeyArea(const LayoutData::SharedLayoutSection &section,
                        bool usePopup = false,
                        QGraphicsWidget *parent = 0);

    virtual ~MImKeyArea();

    //! \reimp
    virtual void modifiersChanged(bool shift, QChar accent = QChar());
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *view);
    virtual QRectF boundingRect() const;
    virtual void setShiftState(ModifierState newShiftState);
    virtual QList<const MImAbstractKey *> keys();
    //! \reimp_end

protected:
    /*! \reimp */
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
    virtual void drawReactiveAreas(MReactionMap *reactionMap, QGraphicsView *view);
    virtual void updateButtonGeometriesForWidth(int availableWidth);
    virtual MImAbstractKey *keyAt(const QPoint &pos) const;
    virtual void onThemeChangeCompleted();
    /*! \reimp_end */

private:
    //! \brief Creates buttons for key data models
    void loadKeys();

    //! \brief Builds QTextLayout representation of current button labels for faster drawing.
    void buildTextLayout();

    qreal computeWidgetHeight() const;

    //! \brief Return preferred height for a row
    qreal preferredRowHeight(int row) const;

    //! \brief Get the maximum width in this widget, in normalized units
    qreal maxNormalizedWidth() const;

    //! \brief Normalized width for a particular MImKeyModel.
    qreal normalizedKeyWidth(const MImKeyModel *key) const;


    //! \brief Draws background.
    void drawKeyBackground(QPainter *painter,
                           const MImKey *button);

    //! \brief Draws button rects/bounding rects, for debugging purposes
    void drawDebugRects(QPainter *painter,
                        const MImKey *button,
                        bool drawBoundingRects,
                        bool drawRects);

    //! \brief Draws reactive areas of buttons, for debugging purposes
    void drawDebugReactiveAreas(QPainter *painter);

    struct ButtonRow {
        QList<MImKey*> buttons;
        QVector<QPair<qreal, qreal> > buttonOffsets;

        //! each row can have one stretch button
        MImKey *stretchButton;
    };

    typedef QVector<ButtonRow> ButtonRowList;
    typedef ButtonRowList::iterator RowIterator;
    typedef ButtonRowList::const_iterator ConstRowIterator;

    ButtonRowList rowList;
    qreal widgetHeight;
    qreal mMaxNormalizedWidth;

    QVector<QPair<int, int> > rowOffsets;

    //! Shift button is stored here if current layout has a shift button.
    MImKey *shiftButton;

    QTextLayout textLayout;
    bool textDirty;

    bool equalWidthButtons;

#ifdef UNIT_TEST
    friend class Ut_MImAbstractKeyArea;
    friend class Ut_KeyEventHandler;
    friend class Bm_MImAbstractKeyArea; //benchmarks
#endif

    friend class FixtureVirtualKeyboard; //!< needed for testing
};

#endif // MIMKEYAREA_H
