/* * This file is part of dui-keyboard *
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



#ifndef UT_DUIIMCORRECTIONCANDIDATEWIDGET_H
#define UT_DUIIMCORRECTIONCANDIDATEWIDGET_H

#include "duiimcorrectioncandidatewidget.h"
#include "duiapplication.h"
#include <QtTest/QTest>
#include <QObject>

class DuiVirtualKeyboardStyleContainer;

class Ut_DuiImCorrectionCandidateWidget : public QObject
{
    Q_OBJECT

private:
    DuiApplication *app;
    DuiImCorrectionCandidateWidget *m_subject;
    DuiVirtualKeyboardStyleContainer *style;

private slots:
    //! initialize application and class
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();
    //void checkCandidates();
    void checkPositionByPoint_data();
    void checkPositionByPoint();
    void checkPositionByPreeditRect();
    void checkActiveIndex();
    void checkPreeditString();
    void setCandidatesAndClick();
    void paint();
};

#endif
