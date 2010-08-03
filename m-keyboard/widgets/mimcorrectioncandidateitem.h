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

#ifndef MIMCORRECTIONCANDIDATEITEM_H
#define MIMCORRECTIONCANDIDATEITEM_H
#include <MContentItem>

class MImCorrectionCandidateItem: public MContentItem
{
    Q_OBJECT
    Q_DISABLE_COPY(MImCorrectionCandidateItem)
public:
    explicit MImCorrectionCandidateItem(MContentItem::ContentItemStyle itemStyle = MContentItem::SingleTextLabel,
                                        QGraphicsItem *parent = 0);
    virtual ~MImCorrectionCandidateItem();
};

#endif
