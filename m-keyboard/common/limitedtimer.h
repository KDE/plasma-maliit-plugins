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



#ifndef LIMITEDTIMER_H
#define LIMITEDTIMER_H

#include <QTimer>
#include <QDateTime>

/*!
 * \class LimitedTimer
 * \brief LimitedTimer is used to limit frequency of timer restarts
 *
 */
class LimitedTimer : public QTimer
{
    Q_OBJECT
    Q_DISABLE_COPY(LimitedTimer)

public:
    /*!
    * \brief Constructor for creating an timer object
    * \param parent QObject* Default value is NULL
    */
    explicit LimitedTimer(QObject *parent = 0);

public slots:
    /*! \reimp */
    void start(int msec);
    void start();
    /*! \reimp_end */

private:
    //! Time of next allowed start
    QDateTime allowedStartTime;
};

#endif
