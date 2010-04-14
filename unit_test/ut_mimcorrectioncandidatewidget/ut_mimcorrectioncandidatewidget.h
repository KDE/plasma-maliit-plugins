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



#ifndef UT_MIMCORRECTIONCANDIDATEWIDGET_H
#define UT_MIMCORRECTIONCANDIDATEWIDGET_H

#include "mimcorrectioncandidatewidget.h"
#include "mapplication.h"
#include <QtTest/QTest>
#include <QObject>

class Ut_MImCorrectionCandidateWidget : public QObject
{
    Q_OBJECT

private:
    MApplication *app;
    MImCorrectionCandidateWidget *m_subject;

private slots:
    //! initialize application and class
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();
    void checkPositionByPoint_data();
    void checkPositionByPoint();
    void checkPositionByPreeditRect();
    void checkActiveIndex();
    void checkPreeditString();
    void setCandidatesAndSelect();
    void checkShowWidget();

private:
    QSize testCandidateWidgetSize;
};

#endif
