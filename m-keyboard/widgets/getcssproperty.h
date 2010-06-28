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

#ifndef GETCSSPROPERTY_H
#define GETCSSPROPERTY_H

#include <MWidgetStyle>
#include <QDebug>

/*!
 * \brief Helper method to get styling property.
 * T should be the real type of retrieved property. Class T should have default
 * constructor and assignment operator.
 * \param container Style container.
 * \param propertyName Property name.
 * \param rtl Contains true if we need to get RTL version of specified property.
 * \param defVal This value will be returned if specified property does not exist.
 * Default value for this parameter is T().
 * \return Returns value of requested property or \a defVal if \a propertyName was not found
 * or \a propertyName is empty.
 */
template <class T>
T getCSSProperty(const MWidgetStyleContainer &container,
                    QString propertyName,
                    const bool rtl,
                    const T &defVal = T())
{
    static const QString RtlSuffix = QString::fromLatin1("Rtl");
    QVariant result(QVariant::Invalid);
    T value(defVal);

    if (!propertyName.isEmpty()) {
        if (rtl) {
            propertyName.append(RtlSuffix);
        }
        QByteArray ba = propertyName.toLatin1();
        result = container->property(ba.data());
    }

    if (result.isValid()) {
       value  = result.value<T>();
    } else if (!propertyName.isEmpty()) {
        qWarning() << __PRETTY_FUNCTION__ << "Property" << propertyName
                   << "does not exist in" << container.objectName();
    }

    return value;
}

#endif

