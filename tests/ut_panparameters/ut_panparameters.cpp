/*
 * This file is part of meego-keyboard 
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list 
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list 
 * of conditions and the following disclaimer in the documentation and/or other materials 
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be 
 * used to endorse or promote products derived from this software without specific 
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */



#include "ut_panparameters.h"

#include "panparameters.h"
#include "notificationpanparameters.h"
#include "outgoinglayoutpanparameters.h"
#include "incominglayoutpanparameters.h"
#include "foregroundmaskpanparameters.h"

#include <QSignalSpy>
#include <QDebug>

#include <memory>

namespace {
    bool tolerantCompare(qreal q1, qreal q2, qreal defaultTolerance = qreal(0.01))
    {
        bool val = false;
        qreal different = qAbs<qreal>(q1 - q2);
        if (different < defaultTolerance)
            val = true;
        else
            qWarning() << q1 << "!=" << q2;
        return val;
    }

    bool tolerantCompare(QPointF p1, QPointF p2, float defaultTolerance = 0.99)
    {
        bool val = false;
        QPointF different = (p1 - p2);
        if (qAbs<float>(different.x()) < defaultTolerance
            && qAbs<float>(different.y()) < defaultTolerance)
            val = true;
        else
            qWarning() << p1 << "!=" << p2;
        return val;
    }
}

void Ut_PanParameters::initTestCase()
{
    notificationParameters = new NotificationPanParameters(this);
    notificationParameters->setPositionRange(QPointF(0, 0), QPointF(854, 0));
    notificationParameters->setScaleRange(1.0, 0.5);
    notificationParameters->setOpacityRange(1.0, 0.0);
    notificationParameters->setPositionProgressRange(0.08, 0.5);
    notificationParameters->setScaleProgressRange(0.08, 0.5);
    notificationParameters->setOpacityProgressRange(0.2, 1.0);

    outgoingLayoutParameters = new OutgoingLayoutPanParameters(this);
    outgoingLayoutParameters->setPositionRange(QPointF(0, 0), QPointF(-854, 0));
    outgoingLayoutParameters->setScaleRange(1.0, 1.0);
    outgoingLayoutParameters->setOpacityRange(1.0, 0.0);
    outgoingLayoutParameters->setPositionProgressRange(0.0, 1.0);
    outgoingLayoutParameters->setOpacityProgressRange(0.5, 1.0);
    
    incomingLayoutParameters = new IncomingLayoutPanParameters(this);
    incomingLayoutParameters->setPositionRange(QPointF(854, 0), QPointF(0, 0));
    incomingLayoutParameters->setScaleRange(1.0, 1.0);
    incomingLayoutParameters->setOpacityRange(1.0, 0.0);
    incomingLayoutParameters->setPositionProgressRange(0.0, 1.0);

    foregroundMaskPanParameters = new ForegroundMaskPanParameters(this);
    foregroundMaskPanParameters->setPositionRange(QPointF(0, 0), QPointF(0, 0));
    foregroundMaskPanParameters->setScaleRange(1.0, 1.0);
    foregroundMaskPanParameters->setOpacityRange(1.0, 0.0);
}

void Ut_PanParameters::cleanupTestCase()
{
    delete notificationParameters;
    delete outgoingLayoutParameters;
    delete incomingLayoutParameters;
    delete foregroundMaskPanParameters;
}

void Ut_PanParameters::init()
{
}

void Ut_PanParameters::cleanup()
{
}

void Ut_PanParameters::testNotificationUpdate_data()
{
    QTest::addColumn<qreal>("progress");
    QTest::addColumn<QPointF>("expectedPosition");
    QTest::addColumn<qreal>("expectedScale");
    QTest::addColumn<qreal>("expectedOpacity");

    // the position, scale and opacity are all changed according progress.
    QTest::newRow("Progress 0.0") << qreal(0.0) << QPointF(0, 0)
        << qreal(1.0) << qreal(1.0);
    QTest::newRow("Progress 0.08") << qreal(0.08) << QPointF(0, 0)
        << qreal(1.0) << qreal(1.0);
    QTest::newRow("Progress 0.2") << qreal(0.2) << QPointF(244, 0)
        << qreal(0.86) << qreal(1.0);
    QTest::newRow("Progress 0.3") << qreal(0.3) << QPointF(447, 0)
        << qreal(0.74) << qreal(0.875);
    QTest::newRow("Progress 0.4") << qreal(0.4) << QPointF(650, 0)
        << qreal(0.62) << qreal(0.75);
    QTest::newRow("Progress 0.5") << qreal(0.5) << QPointF(854, 0)
        << qreal(0.5) << qreal(0.625);
    QTest::newRow("Progress 0.6") << qreal(0.6) << QPointF(854, 0)
        << qreal(0.5) << qreal(0.5);
    QTest::newRow("Progress 0.7") << qreal(0.7) << QPointF(854, 0)
        << qreal(0.5) << qreal(0.375);
    QTest::newRow("Progress 0.8") << qreal(0.8) << QPointF(854, 0)
        << qreal(0.5) << qreal(0.25);
    QTest::newRow("Progress 0.9") << qreal(0.9) << QPointF(854, 0)
        << qreal(0.5) << qreal(0.125);
    QTest::newRow("Progress 1.0") << qreal(1.0) << QPointF(854, 0)
        << qreal(0.5) << qreal(0.0);
}

void Ut_PanParameters::testNotificationUpdate()
{
    QFETCH(qreal, progress);
    QFETCH(QPointF, expectedPosition);
    QFETCH(qreal, expectedScale);
    QFETCH(qreal, expectedOpacity);

    QSignalSpy positionSpy(notificationParameters,
                           SIGNAL(positionChanged(QPointF)));

    QSignalSpy scaleSpy(notificationParameters,
                        SIGNAL(scaleChanged(qreal)));

    QSignalSpy opacitySpy(notificationParameters,
                          SIGNAL(opacityChanged(qreal)));

    positionSpy.clear();
    opacitySpy.clear();
    scaleSpy.clear();
    notificationParameters->setProgress(progress);

    QCOMPARE(notificationParameters->progress(), progress);
    // after setProgress, the position, scale and opacity should be updated.
    if ((progress > qreal(0.08) && progress <= qreal(0.5))) {
        QCOMPARE(positionSpy.count(), 1);
        QCOMPARE(scaleSpy.count(), 1);
    } else {
        QCOMPARE(positionSpy.count(), 0);
        QCOMPARE(scaleSpy.count(), 0);
    }

    if (progress > qreal(0.2) && progress <= qreal(1.0)) {
        QCOMPARE(opacitySpy.count(), 1);
    } else {
        QCOMPARE(opacitySpy.count(), 0);
    }

    QVERIFY(tolerantCompare(notificationParameters->position(), expectedPosition));
    QVERIFY(tolerantCompare(notificationParameters->scale(), expectedScale));
    QVERIFY(tolerantCompare(notificationParameters->opacity(), expectedOpacity));
}

void Ut_PanParameters::testOutgoingLayoutParametersUpdate_data()
{
    QTest::addColumn<qreal>("progress");
    QTest::addColumn<QPointF>("expectedPosition");
    QTest::addColumn<qreal>("expectedScale");
    QTest::addColumn<qreal>("expectedOpacity");

    // change progress, the scale won't be changed. The position kept
    // changing, and the opacity will be changed after progress
    // bigger than 0.5.
    QTest::newRow("Progress 0.0") << qreal(0.0) << QPointF(0, 0)
        << qreal(1.0) << qreal(1.0);
    QTest::newRow("Progress 0.08") << qreal(0.08) << QPointF(-68, 0)
        << qreal(1.0) << qreal(1.0);
    QTest::newRow("Progress 0.2") << qreal(0.2) << QPointF(-171, 0)
        << qreal(1.0) << qreal(1.0);
    QTest::newRow("Progress 0.3") << qreal(0.3) << QPointF(-256, 0)
        << qreal(1.0) << qreal(1.0);
    QTest::newRow("Progress 0.4") << qreal(0.4) << QPointF(-342, 0)
        << qreal(1.0) << qreal(1.0);
    QTest::newRow("Progress 0.5") << qreal(0.5) << QPointF(-427, 0)
        << qreal(1.0) << qreal(1.0);
    QTest::newRow("Progress 0.6") << qreal(0.6) << QPointF(-512, 0)
        << qreal(1.0) << qreal(0.8);
    QTest::newRow("Progress 0.7") << qreal(0.7) << QPointF(-597, 0)
        << qreal(1.0) << qreal(0.6);
    QTest::newRow("Progress 0.8") << qreal(0.8) << QPointF(-683, 0)
        << qreal(1.0) << qreal(0.4);
    QTest::newRow("Progress 0.9") << qreal(0.9) << QPointF(-768, 0)
        << qreal(1.0) << qreal(0.2);
    QTest::newRow("Progress 1.0") << qreal(1.0) << QPointF(-854, 0)
        << qreal(1.0) << qreal(0.0);
}

void Ut_PanParameters::testOutgoingLayoutParametersUpdate()
{
    QFETCH(qreal, progress);
    QFETCH(QPointF, expectedPosition);
    QFETCH(qreal, expectedScale);
    QFETCH(qreal, expectedOpacity);

    QSignalSpy positionSpy(outgoingLayoutParameters,
                           SIGNAL(positionChanged(QPointF)));

    QSignalSpy scaleSpy(outgoingLayoutParameters,
                        SIGNAL(scaleChanged(qreal)));

    QSignalSpy opacitySpy(outgoingLayoutParameters,
                          SIGNAL(opacityChanged(qreal)));

    positionSpy.clear();
    opacitySpy.clear();
    scaleSpy.clear();
    outgoingLayoutParameters->setProgress(progress);

    QCOMPARE(outgoingLayoutParameters->progress(), progress);

    // after setProgress, the position and opacity should be updated.
    if (progress > qreal(0.0))
        QCOMPARE(positionSpy.count(), 1);

    if (progress > qreal(0.5) && progress <= qreal(1.0)) {
        QCOMPARE(opacitySpy.count(), 1);
    } else {
        QCOMPARE(opacitySpy.count(), 0);
    }

    // should not have scale change
    QCOMPARE(scaleSpy.count(), 0);

    QVERIFY(tolerantCompare(outgoingLayoutParameters->position(), expectedPosition));
    QVERIFY(tolerantCompare(outgoingLayoutParameters->scale(), expectedScale));
    QVERIFY(tolerantCompare(outgoingLayoutParameters->opacity(), expectedOpacity));
}

void Ut_PanParameters::testIncomingLayoutParametersUpdate_data()
{
    QTest::addColumn<qreal>("progress");
    QTest::addColumn<QPointF>("expectedPosition");
    QTest::addColumn<qreal>("expectedScale");
    QTest::addColumn<qreal>("expectedOpacity");
    QTest::addColumn<bool>("start");

    // Change progress, the scale won't be changed. The position kept
    // changing. But the opacity will not be changed for the first moving
    // forward, if move forward bigger than 0.5, and move back, the opacity
    // will be changed from 1.0 to 0.0.
    QTest::newRow("first move forward, Progress 0.0")
        << qreal(0.0) << QPointF(854, 0)
        << qreal(1.0) << qreal(1.0) << true;
    QTest::newRow("first move forward, Progress 0.08")
        << qreal(0.08) << QPointF(786, 0)
        << qreal(1.0) << qreal(1.0) << true;
    QTest::newRow("first move forward, Progress 0.2")
        << qreal(0.2) << QPointF(683, 0)
        << qreal(1.0) << qreal(1.0) << true;
    QTest::newRow("first move forward, Progress 0.3")
        << qreal(0.3) << QPointF(597, 0)
        << qreal(1.0) << qreal(1.0) << true;
    QTest::newRow("first move forward, Progress 0.4")
        << qreal(0.4) << QPointF(512, 0)
        << qreal(1.0) << qreal(1.0) << true;
    QTest::newRow("first move forward, Progress 0.5")
        << qreal(0.5) << QPointF(427, 0)
        << qreal(1.0) << qreal(1.0) << true;
    QTest::newRow("first move forward, Progress 0.6")
        << qreal(0.6) << QPointF(341, 0)
        << qreal(1.0) << qreal(1.0) << true;
    QTest::newRow("first move forward, Progress 0.7")
        << qreal(0.7) << QPointF(256, 0)
        << qreal(1.0) << qreal(1.0) << true;
    QTest::newRow("first move forward, Progress 0.8")
        << qreal(0.8) << QPointF(170, 0)
        << qreal(1.0) << qreal(1.0) << true;
    QTest::newRow("first move forward, Progress 0.9")
        << qreal(0.9) << QPointF(85, 0)
        << qreal(1.0) << qreal(1.0) << true;
    QTest::newRow("first move forward, Progress 1.0")
        << qreal(1.0) << QPointF(0, 0)
        << qreal(1.0) << qreal(1.0) << true;

    // move back
    QTest::newRow("move back, Progress 0.8")
        << qreal(0.8) << QPointF(170, 0)
        << qreal(1.0) << qreal(1.0) << true;
    QTest::newRow("move back, Progress 0.6")
        << qreal(0.6) << QPointF(341, 0)
        << qreal(1.0) << qreal(1.0) << true;
    QTest::newRow("move back, Progress 0.5")
        << qreal(0.5) << QPointF(427, 0)
        << qreal(1.0) << qreal(1.0) << true;

    QTest::newRow("move back, Progress 0.4")
        << qreal(0.4) << QPointF(512, 0)
        << qreal(1.0) << qreal(0.8) << false;
    QTest::newRow("move back, Progress 0.3")
        << qreal(0.3) << QPointF(597, 0)
        << qreal(1.0) << qreal(0.6) << false;
    QTest::newRow("move back, Progress 0.2")
        << qreal(0.2) << QPointF(683, 0)
        << qreal(1.0) << qreal(0.4) << false;
    QTest::newRow("move back, Progress 0.08")
        << qreal(0.08) << QPointF(785, 0)
        << qreal(1.0) << qreal(0.16) << false;
    QTest::newRow("move back, Progress 0.0")
        << qreal(0.0) << QPointF(854, 0)
        << qreal(1.0) << qreal(0.0) << false;

    // move again forward
    QTest::newRow("move forward again, Progress 0.3")
        << qreal(0.3) << QPointF(597, 0)
        << qreal(1.0) << qreal(0.6) << false;
    QTest::newRow("move forward again, Progress 0.5")
        << qreal(0.5) << QPointF(427, 0)
        << qreal(1.0) << qreal(1.0) << false;
    QTest::newRow("move forward again, Progress 0.6")
        << qreal(0.6) << QPointF(341, 0)
        << qreal(1.0) << qreal(1.0) << false;
    QTest::newRow("move forward again, Progress 0.8")
        << qreal(0.8) << QPointF(170, 0)
        << qreal(1.0) << qreal(1.0) << false;
    QTest::newRow("move forward again, Progress 1.0")
        << qreal(1.0) << QPointF(0, 0)
        << qreal(1.0) << qreal(1.0) << false;
}

void Ut_PanParameters::testIncomingLayoutParametersUpdate()
{
    QFETCH(qreal, progress);
    QFETCH(QPointF, expectedPosition);
    QFETCH(qreal, expectedScale);
    QFETCH(qreal, expectedOpacity);
    QFETCH(bool, start);

    QSignalSpy positionSpy(incomingLayoutParameters,
                           SIGNAL(positionChanged(QPointF)));

    QSignalSpy scaleSpy(incomingLayoutParameters,
                        SIGNAL(scaleChanged(qreal)));

    QSignalSpy opacitySpy(incomingLayoutParameters,
                          SIGNAL(opacityChanged(qreal)));

    positionSpy.clear();
    opacitySpy.clear();
    scaleSpy.clear();

    incomingLayoutParameters->setProgress(progress);
    QCOMPARE(incomingLayoutParameters->progress(), progress);

    // after setProgress, the position and opacity should be updated.
    QCOMPARE(positionSpy.count(), 1);

    if (progress <= qreal(0.5) && !start) {
        QCOMPARE(opacitySpy.count(), 1);
    } else {
        QCOMPARE(opacitySpy.count(), 0);
    }

    // should not have scale change
    QCOMPARE(scaleSpy.count(), 0);

    QVERIFY(tolerantCompare(incomingLayoutParameters->position(), expectedPosition));
    QVERIFY(tolerantCompare(incomingLayoutParameters->scale(), expectedScale));
    QVERIFY(tolerantCompare(incomingLayoutParameters->opacity(), expectedOpacity));
}

void Ut_PanParameters::testForegroundMaskPanParametersUpdate_data()
{
    QTest::addColumn<qreal>("progress");
    QTest::addColumn<QPointF>("expectedPosition");
    QTest::addColumn<qreal>("expectedScale");
    QTest::addColumn<qreal>("expectedOpacity");

    // Change progress, the position and scale won't be changed.
    // The opacity will be changed from 0.0 to 1.0 when progress
    // is from 0.0 to 0.5, and opacity from 1.0 to 0.0 again when
    // progress from 0.5 to 1.0.
    QTest::newRow("Progress 0.0") << qreal(0.0) << QPointF(0, 0)
        << qreal(1.0) << qreal(0.0);
    QTest::newRow("Progress 0.08") << qreal(0.08) << QPointF(0, 0)
        << qreal(1.0) << qreal(0.16);
    QTest::newRow("Progress 0.2") << qreal(0.2) << QPointF(0, 0)
        << qreal(1.0) << qreal(0.4);
    QTest::newRow("Progress 0.3") << qreal(0.3) << QPointF(0, 0)
        << qreal(1.0) << qreal(0.6);
    QTest::newRow("Progress 0.4") << qreal(0.4) << QPointF(0, 0)
        << qreal(1.0) << qreal(0.8);
    QTest::newRow("Progress 0.5") << qreal(0.5) << QPointF(0, 0)
        << qreal(1.0) << qreal(1.0);
    QTest::newRow("Progress 0.6") << qreal(0.6) << QPointF(0, 0)
        << qreal(1.0) << qreal(0.8);
    QTest::newRow("Progress 0.7") << qreal(0.7) << QPointF(0, 0)
        << qreal(1.0) << qreal(0.6);
    QTest::newRow("Progress 0.8") << qreal(0.8) << QPointF(0, 0)
        << qreal(1.0) << qreal(0.4);
    QTest::newRow("Progress 0.9") << qreal(0.9) << QPointF(0, 0)
        << qreal(1.0) << qreal(0.2);
    QTest::newRow("Progress 1.0") << qreal(1.0) << QPointF(0, 0)
        << qreal(1.0) << qreal(0.0);
}

void Ut_PanParameters::testForegroundMaskPanParametersUpdate()
{
    QFETCH(qreal, progress);
    QFETCH(QPointF, expectedPosition);
    QFETCH(qreal, expectedScale);
    QFETCH(qreal, expectedOpacity);

    QSignalSpy positionSpy(foregroundMaskPanParameters,
                           SIGNAL(positionChanged(QPointF)));

    QSignalSpy scaleSpy(foregroundMaskPanParameters,
                        SIGNAL(scaleChanged(qreal)));

    QSignalSpy opacitySpy(foregroundMaskPanParameters,
                          SIGNAL(opacityChanged(qreal)));

    positionSpy.clear();
    opacitySpy.clear();
    scaleSpy.clear();
    foregroundMaskPanParameters->setProgress(progress);

    QCOMPARE(foregroundMaskPanParameters->progress(), progress);

    // after setProgress, the position and opacity should be updated.
    QCOMPARE(positionSpy.count(), 0);
    if (progress > qreal(0.0))
        QCOMPARE(opacitySpy.count(), 1);
    // should not have scale change
    QCOMPARE(scaleSpy.count(), 0);

    QVERIFY(tolerantCompare(foregroundMaskPanParameters->position(), expectedPosition));
    QVERIFY(tolerantCompare(foregroundMaskPanParameters->scale(), expectedScale));
    QVERIFY(tolerantCompare(foregroundMaskPanParameters->opacity(), expectedOpacity));
}

QTEST_APPLESS_MAIN(Ut_PanParameters);
