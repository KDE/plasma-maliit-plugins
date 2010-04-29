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



#ifndef MBUTTONAREA_H
#define MBUTTONAREA_H

#include "keybuttonarea.h"

class FlickUpButton;

/*!
 * \brief MButtonArea is a MButton implementation of KeyButtonArea.
 */
class MButtonArea : public KeyButtonArea
{
public:
    MButtonArea(MVirtualKeyboardStyleContainer *,
                QSharedPointer<const LayoutSection>,
                ButtonSizeScheme buttonSizeScheme = ButtonSizeEqualExpanding,
                bool usePopup = false,
                QGraphicsWidget *parent = 0);

    virtual ~MButtonArea();

protected:
    /*! \reimp */
    virtual bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);
    virtual void drawReactiveAreas(MReactionMap *reactionMap, QGraphicsView *view);
    virtual void updateButtonGeometries(int availableWidth, int equalButtonWidth);
    virtual IKeyButton *keyAt(const QPoint &pos) const;
    virtual void modifiersChanged(bool shift, QChar accent = QChar());
    /*! \reimp_end */

private:
    //! \brief Creates buttons for key data models
    void loadKeys();

private:
    QGraphicsLinearLayout &mainLayout;

    QVector<FlickUpButton *> buttons;

#ifdef UNIT_TEST
    friend class Ut_KeyButtonArea;
    friend class Bm_KeyButtonArea; //benchmarks
#endif
};

#endif // MBUTTONAREA_H
