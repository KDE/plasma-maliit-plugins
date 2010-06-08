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

#ifndef HANDLESTYLE_H
#define HANDLESTYLE_H

#include <MWidgetStyle>

/*!
 * \brief This defines style that is common to all Handle class based widgets.
 */
class M_EXPORT HandleStyle : public MWidgetStyle
{
    Q_OBJECT
    M_STYLE(HandleStyle)
};

class M_EXPORT HandleStyleContainer : public MWidgetStyleContainer
{
    M_STYLE_CONTAINER(HandleStyle)

    M_STYLE_MODE(Landscape)
    M_STYLE_MODE(Portrait)

    friend class Handle;
};


#endif
