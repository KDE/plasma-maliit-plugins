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

#include "mtoolbarbuttonview.h"
#include "mtoolbarbutton.h"

MToolbarButtonView::MToolbarButtonView(MToolbarButton *controller)
    : MButtonView(controller)
{
}


QSizeF MToolbarButtonView::optimalSize(const QSizeF& maxSize)
{
    // Let the MButtonView implementation to calculate the right preferred size
    QSizeF preferredSize = sizeHint(Qt::PreferredSize);

    // The calculation of the MButtonView::sizeHint() does not take into account
    // the left and right margins (only left and right text margins are used). Let's
    // add it here.
    preferredSize.setWidth(preferredSize.width()+marginLeft()+marginRight());
    // Use the height from the maximal size, because it is correct to get from the style
    preferredSize.setHeight(maxSize.height());
    // We can not be bigger than the maximum size
    if (preferredSize.width() > maxSize.width()){
        return maxSize;
    }
    return preferredSize;
}
