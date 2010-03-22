/* * This file is part of dui-keyboard *
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



#ifndef SINGLEWIDGETBUTTONAREA_H
#define SINGLEWIDGETBUTTONAREA_H

#include "keybuttonarea.h"
#include "singlewidgetbutton.h"

#include <QTextLayout>

class DuiScalableImage;

/*!
 * \brief SingleWidgetButtonArea is an implementation of KeyButtonArea which
 * does not use separate widgets for buttons, but instead draws them explicitly.
 */
class SingleWidgetButtonArea : public KeyButtonArea
{
public:
    SingleWidgetButtonArea(DuiVirtualKeyboardStyleContainer *,
                           QSharedPointer<const LayoutSection>,
                           ButtonSizeScheme buttonSizeScheme = ButtonSizeEqualExpanding,
                           bool usePopup = false,
                           QGraphicsWidget *parent = 0);

    virtual ~SingleWidgetButtonArea();

    //! \reimp
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *view);
    virtual QRectF boundingRect() const;
    //! \reimp_end

protected:
    /*! \reimp */
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
    virtual void drawReactiveAreas(DuiReactionMap *reactionMap, QGraphicsView *view);
    virtual void updateButtonGeometries(int availableWidth, int equalButtonWidth);
    virtual IKeyButton *keyAt(const QPoint &pos) const;
    virtual void modifiersChanged(bool shift, QChar accent = QChar());
    /*! \reimp_end */

private:
    //! \brief Creates buttons for key data models
    void loadKeys();

    //! \brief Builds QTextLayout representation of current button labels for faster drawing.
    void buildTextLayout();

    struct ButtonRow {
        QList<SingleWidgetButton*> buttons;

        //! each row can have one stretch button
        SingleWidgetButton *stretchButton;

        // offset is from x=0 to row start
        // first button is at offset + horizontal spacing / 2
        int offset;
        int cachedWidth; // includes left & right margin
    };
    
    typedef QVector<ButtonRow> ButtonRowList;
    typedef ButtonRowList::iterator RowIterator;

    int rowHeight; //! constant row height, includes margins
    ButtonRowList rowList;

    const DuiScalableImage *keyBackground;
    const DuiScalableImage *keyBackgroundPressed;
    const DuiScalableImage *keyBackgroundSelected;

    const QPixmap *pixmap1;
    const QPixmap *pixmap2;
    const QPixmap *pixmap3;

    QTextLayout textLayout;

    bool equalWidthButtons;

#ifdef UNIT_TEST
    friend class Ut_KeyButtonArea;
    friend class Bm_KeyButtonArea; //benchmarks
#endif

    friend class FixtureVirtualKeyboard; //!< needed for testing
};

#endif // SINGLEWIDGETBUTTONAREA_H
