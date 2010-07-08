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



#ifndef SINGLEWIDGETBUTTONAREA_H
#define SINGLEWIDGETBUTTONAREA_H

#include "keybuttonarea.h"

#include <MScalableImage>
#include <QTextLayout>

class SingleWidgetButton;

/*!
 * \brief SingleWidgetButtonArea is an implementation of KeyButtonArea which
 * does not use separate widgets for buttons, but instead draws them explicitly.
 */
class SingleWidgetButtonArea : public KeyButtonArea, public ISymIndicator
{
public:
    //! Used to differentiate background images of a button.
    enum KeyBackgroundType {
        NormalBackground = 0,
        KeyPressedBackground = 1,
        KeySelectedBackground = 2,

        KeyBackgroundTypeCount
    };

    SingleWidgetButtonArea(const MVirtualKeyboardStyleContainer *,
                           QSharedPointer<const LayoutSection>,
                           ButtonSizeScheme buttonSizeScheme = ButtonSizeEqualExpanding,
                           bool usePopup = false,
                           QGraphicsWidget *parent = 0);

    virtual ~SingleWidgetButtonArea();

    //! \reimp
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *view);
    virtual QRectF boundingRect() const;
    virtual ISymIndicator *symIndicator();
    virtual void setShiftState(ModifierState newShiftState);

    // From ISymIndicator
    virtual void activateSymIndicator();
    virtual void activateAceIndicator();
    virtual void deactivateIndicator();
    //! \reimp_end

protected:
    /*! \reimp */
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
    virtual void drawReactiveAreas(MReactionMap *reactionMap, QGraphicsView *view);
    virtual void updateButtonGeometries(int availableWidth, int equalButtonWidth);
    virtual IKeyButton *keyAt(const QPoint &pos) const;
    virtual void modifiersChanged(bool shift, QChar accent = QChar());
    virtual void onThemeChangeCompleted();
    /*! \reimp_end */

private:
    //! Enumeration to keep track of the sym indicator state.
    enum SymIndicatorState {
        SymActive,
        AceActive,
        SymIndicatorInactive
    };

    //! \brief Creates buttons for key data models
    void loadKeys();

    //! \brief Builds QTextLayout representation of current button labels for faster drawing.
    void buildTextLayout();

    //! \brief Update indicator backgrounds from current theme.
    void updateIndicatorBackgrounds(const MScalableImage *normal, const MScalableImage *pressed);

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
    typedef ButtonRowList::const_iterator ConstRowIterator;

    int rowHeight; //! constant row height, includes margins
    ButtonRowList rowList;

    //! Special set of button backgrounds for sym state indicator.
    const MScalableImage *symIndicatorBackgrounds[KeyBackgroundTypeCount];

    //! Current state of the sym indicator
    SymIndicatorState symState;

    //! This is provided for easier access to sym indicator button
    //! if such exists in the current layout.
    const SingleWidgetButton *symIndicatorButton;

    //! Shift button is stored here if current layout has a shift button.
    SingleWidgetButton *shiftButton;

    QTextLayout textLayout;
    bool textDirty;

    bool equalWidthButtons;

#ifdef UNIT_TEST
    friend class Ut_KeyButtonArea;
    friend class Ut_KeyEventHandler;
    friend class Bm_KeyButtonArea; //benchmarks
#endif

    friend class FixtureVirtualKeyboard; //!< needed for testing
};

#endif // SINGLEWIDGETBUTTONAREA_H
