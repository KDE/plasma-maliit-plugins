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

#ifndef GRIP_H
#define GRIP_H

#include "handle.h"

class QGraphicsWidget;

/*!
  \brief Handle with a visual representation of a grip
*/
class Grip : public Handle
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     * \param parent Parent object.
     */
    explicit Grip(QGraphicsWidget *parent = 0);

    //! Destructor
    virtual ~Grip();
};

#endif
