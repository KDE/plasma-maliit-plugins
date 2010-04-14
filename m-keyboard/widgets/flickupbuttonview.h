/* * This file is part of m-keyboard *
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



#ifndef FLICKUPBUTTONVIEW_H
#define FLICKUPBUTTONVIEW_H

#include <MButtonView>
#include "flickupbuttonstyle.h"

class FlickUpButton;

/*!
 * \class FlickUpButtonView
 * \brief FlickUpButtonView implements a view for FlickUpButton
 */
class FlickUpButtonView : public MButtonView
{
    Q_OBJECT
    M_VIEW(MButtonModel, FlickUpButtonStyle)

public:
    /*!
     ** \brief Constructor
     ** \param controller Pointer to the button's controller
     **/
    FlickUpButtonView(FlickUpButton *controller);

    /*!
     ** \brief Destructor
     **/
    virtual ~FlickUpButtonView();

protected:
    //! \reimp
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void applyStyle();
    //! \reimp_end

private:
    //! Controller
    FlickUpButton *controller;

#ifdef UNIT_TEST
    friend class Ut_FlickUpButton;
#endif
};

#endif

