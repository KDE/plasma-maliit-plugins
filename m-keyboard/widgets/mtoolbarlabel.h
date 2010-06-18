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

#ifndef MTOOLBARLABEL_H
#define MTOOLBARLABEL_H

#include <MLabel>
#include <QSharedPointer>

/*!
 * \class MToolbarLabel
 * \brief MToolbarLabel is provided for the labelss in the input method toolbar.
 *
 * MToolbarLabel is inherit from MLabel. It is used to show text only.
 */
class MToolbarItem;

class MToolbarLabel : public MLabel
{
    Q_OBJECT
    Q_DISABLE_COPY(MToolbarLabel)

public:
    /*!
     * \Brief Constructor
     */
    explicit MToolbarLabel(QSharedPointer<MToolbarItem> item, QGraphicsItem *parent = 0);

    //! Destructor
    virtual ~MToolbarLabel();

    //! Return pointer to corresponding toolbar item.
    QSharedPointer<MToolbarItem> item();

signals:
    //! \brief Emitted when visibility (in a sense of the label being available
    //! or not) changes.
    void availabilityChanged();

private slots:
    //! Update label's properties when properties of toolbar item are updated.
    void updateData(const QString &attribute);

private:
    //! MToolbarItem is used as model for this label.
    QSharedPointer<MToolbarItem> itemPtr;
};

#endif

