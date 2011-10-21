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
#include "ut_touchforwardfilter.h"
#include "touchtargetwidget.h"

#include "touchforwardfilter.h"

#include <QGraphicsWidget>
#include <QGraphicsScene>
#include <QSignalSpy>
#include <QDebug>

Q_DECLARE_METATYPE(Ut_TouchForwardFilter::EventTypeList);
Q_DECLARE_METATYPE(Ut_TouchForwardFilter::EventReceiverPairList);
Q_DECLARE_METATYPE(Ut_TouchForwardFilter::TouchEventList);
Q_DECLARE_METATYPE(Ut_TouchForwardFilter::TouchEvent);
Q_DECLARE_METATYPE(TouchForwardFilter::ItemTouchState);
Q_DECLARE_METATYPE(QEvent::Type);

// QGraphicsObject without pure virtuals.
// This is better suited for testing unique id than QGraphicsWidget
// which is bigger in size than QGraphicsObject.
class GraphicsObject : public QGraphicsObject
{
public:
    virtual QRectF boundingRect() const
    {
        return QRectF();
    }
    virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*)
    {
    }
};

void Ut_TouchForwardFilter::initTestCase()
{
    scene = new QGraphicsScene;

    // Create origin widget in initTestCase so that it's memory
    // address is known in _data() methods.
    origin = new QGraphicsWidget;
    scene->addItem(origin);
}

void Ut_TouchForwardFilter::cleanupTestCase()
{
    delete origin;
    delete scene;
}

void Ut_TouchForwardFilter::init()
{
    target = new TouchTargetWidget;
    scene->addItem(target);
    subject = new TouchForwardFilter(target,
                                     TouchForwardFilter::TouchInactive,
                                     origin);
}

void Ut_TouchForwardFilter::cleanup()
{
    delete target;
    delete subject; // Was already destroyed by target.
}

void Ut_TouchForwardFilter::testForwardType_data()
{
    QTest::addColumn<EventReceiverPairList>("operations");
    QTest::addColumn<EventTypeList>("expectedTargetEventTypes");

    // Only origin events first.

    QTest::newRow("simple forward 1")
        << (EventReceiverPairList()
            << EventReceiverPair(Origin, QEvent::TouchBegin))
        << (EventTypeList() << QEvent::TouchBegin);

    QTest::newRow("simple forward 2")
        << (EventReceiverPairList()
            << EventReceiverPair(Origin, QEvent::TouchBegin)
            << EventReceiverPair(Origin, QEvent::TouchUpdate))
        << (EventTypeList()
            << QEvent::TouchBegin
            << QEvent::TouchUpdate);

    QTest::newRow("simple forward 3")
        << (EventReceiverPairList()
            << EventReceiverPair(Origin, QEvent::TouchBegin)
            << EventReceiverPair(Origin, QEvent::TouchUpdate)
            << EventReceiverPair(Origin, QEvent::TouchEnd))
        << (EventTypeList()
            << QEvent::TouchBegin
            << QEvent::TouchUpdate
            << QEvent::TouchEnd);

    QTest::newRow("TouchUpdate instead of TouchBegin")
        << (EventReceiverPairList()
            << EventReceiverPair(Origin, QEvent::TouchUpdate))
        << (EventTypeList()
            << QEvent::TouchBegin
            << QEvent::TouchUpdate);

    QTest::newRow("TouchEnd instead of TouchBegin")
        << (EventReceiverPairList()
            << EventReceiverPair(Origin, QEvent::TouchEnd))
        << (EventTypeList()
            << QEvent::TouchBegin
            << QEvent::TouchEnd);


    // Test target events.

    QTest::newRow("Target events only") // I.e. they are not filtered out.
        << (EventReceiverPairList()
            << EventReceiverPair(Target, QEvent::TouchBegin)
            << EventReceiverPair(Target, QEvent::TouchUpdate)
            << EventReceiverPair(Target, QEvent::TouchEnd))
        << (EventTypeList()
            << QEvent::TouchBegin
            << QEvent::TouchUpdate
            << QEvent::TouchEnd);

    QTest::newRow("Target events first, then origin")
        << (EventReceiverPairList()
            << EventReceiverPair(Target, QEvent::TouchBegin)
            << EventReceiverPair(Target, QEvent::TouchUpdate)
            << EventReceiverPair(Target, QEvent::TouchEnd)
            << EventReceiverPair(Origin, QEvent::TouchBegin)
            << EventReceiverPair(Origin, QEvent::TouchUpdate)
            << EventReceiverPair(Origin, QEvent::TouchEnd))
        << (EventTypeList()
            << QEvent::TouchBegin
            << QEvent::TouchUpdate
            << QEvent::TouchEnd
            << QEvent::TouchBegin
            << QEvent::TouchUpdate
            << QEvent::TouchEnd);


    // Test overlapping origin and target events.

    QTest::newRow("Overlapping: Target events between origin begin & end")
        << (EventReceiverPairList()
            << EventReceiverPair(Origin, QEvent::TouchBegin)
            << EventReceiverPair(Target, QEvent::TouchBegin)
            << EventReceiverPair(Target, QEvent::TouchUpdate)
            << EventReceiverPair(Target, QEvent::TouchEnd)
            << EventReceiverPair(Origin, QEvent::TouchEnd))
        << (EventTypeList()
            << QEvent::TouchBegin  // Origin begin
            << QEvent::TouchUpdate // Target begin
            << QEvent::TouchUpdate // Target update
            << QEvent::TouchUpdate // Target end
            << QEvent::TouchEnd);  // Origin end

    QTest::newRow("Overlapping: origin events between target begin & end")
        << (EventReceiverPairList()
            << EventReceiverPair(Target, QEvent::TouchBegin)
            << EventReceiverPair(Origin, QEvent::TouchBegin)
            << EventReceiverPair(Origin, QEvent::TouchUpdate)
            << EventReceiverPair(Origin, QEvent::TouchEnd)
            << EventReceiverPair(Target, QEvent::TouchEnd))
        << (EventTypeList()
            << QEvent::TouchBegin  // Target begin
            << QEvent::TouchUpdate // Origin begin
            << QEvent::TouchUpdate // Origin update
            << QEvent::TouchUpdate // Origin end
            << QEvent::TouchEnd);  // Target end

    QTest::newRow("Overlapping: origin events after target events")
        << (EventReceiverPairList()
            << EventReceiverPair(Target, QEvent::TouchBegin)
            << EventReceiverPair(Origin, QEvent::TouchBegin)
            << EventReceiverPair(Target, QEvent::TouchEnd)
            << EventReceiverPair(Origin, QEvent::TouchEnd))
        << (EventTypeList()
            << QEvent::TouchBegin
            << QEvent::TouchUpdate // Origin begin
            << QEvent::TouchUpdate // Target end
            << QEvent::TouchEnd);

    QTest::newRow("Overlapping: Target events after origin events")
        << (EventReceiverPairList()
            << EventReceiverPair(Origin, QEvent::TouchBegin)
            << EventReceiverPair(Target, QEvent::TouchBegin)
            << EventReceiverPair(Origin, QEvent::TouchEnd)
            << EventReceiverPair(Target, QEvent::TouchEnd))
        << (EventTypeList()
            << QEvent::TouchBegin
            << QEvent::TouchUpdate // Target begin
            << QEvent::TouchUpdate // Origin end
            << QEvent::TouchEnd);

    QTest::newRow("Overlapping: origin begins with update #1")
        << (EventReceiverPairList()
            << EventReceiverPair(Target, QEvent::TouchBegin)
            << EventReceiverPair(Origin, QEvent::TouchUpdate)
            << EventReceiverPair(Target, QEvent::TouchEnd)
            << EventReceiverPair(Origin, QEvent::TouchEnd))
        << (EventTypeList()
            << QEvent::TouchBegin
            << QEvent::TouchUpdate // Origin update, with presses
            << QEvent::TouchUpdate // Origin update, with original touch point states
            << QEvent::TouchUpdate
            << QEvent::TouchEnd);

    QTest::newRow("Overlapping: origin begins with update #2")
        << (EventReceiverPairList()
            << EventReceiverPair(Origin, QEvent::TouchUpdate)
            << EventReceiverPair(Target, QEvent::TouchBegin)
            << EventReceiverPair(Target, QEvent::TouchEnd)
            << EventReceiverPair(Origin, QEvent::TouchEnd))
        << (EventTypeList()
            << QEvent::TouchBegin  // Origin update, with presses
            << QEvent::TouchUpdate // Origin update, with original touch point states
            << QEvent::TouchUpdate // Target begin
            << QEvent::TouchUpdate // Target end
            << QEvent::TouchEnd);  // Origin end
}

void Ut_TouchForwardFilter::testForwardType()
{
    QFETCH(EventReceiverPairList, operations);
    QFETCH(EventTypeList, expectedTargetEventTypes);

    foreach (EventReceiverPair pair, operations) {
        QTouchEvent event(pair.second);
        QObject *receiver = pair.first == Origin
                            ? origin.data() : target.data();
        qApp->sendEvent(receiver, &event);
    }

    QCOMPARE(target->touchEvents.count(), expectedTargetEventTypes.count());

    for (int i = 0; i < expectedTargetEventTypes.count(); ++i) {
        QEvent::Type actualType = target->touchEvents.at(i).type();
        QEvent::Type expectedType = expectedTargetEventTypes.at(i);
        qDebug() << "actual:" << actualType << ", expected:" << expectedType;
        QCOMPARE(actualType, expectedType);
    }
}

void Ut_TouchForwardFilter::testInitialEventType_data()
{
    // When the filter is installed it will try to send initial press
    // to target. This test checks that event types are correct in that case.

    QTest::addColumn<TouchForwardFilter::ItemTouchState>("initialTargetState");
    QTest::addColumn<QEvent::Type>("initialEventType");
    QTest::addColumn<EventTypeList>("expectedTargetEventTypes");

    QTest::newRow("target inactive, no event")
            << TouchForwardFilter::TouchInactive
            << QEvent::None
            << EventTypeList();

    QTest::newRow("target inactive, TouchBegin")
            << TouchForwardFilter::TouchInactive
            << QEvent::TouchBegin
            << (EventTypeList() << QEvent::TouchBegin);

    QTest::newRow("target inactive, TouchUpdate")
            << TouchForwardFilter::TouchInactive
            << QEvent::TouchUpdate
            << (EventTypeList() << QEvent::TouchBegin << QEvent::TouchUpdate);

    QTest::newRow("target inactive, TouchEnd")
            << TouchForwardFilter::TouchInactive
            << QEvent::TouchEnd
            << (EventTypeList() << QEvent::TouchBegin << QEvent::TouchEnd);


    QTest::newRow("target active, no event")
            << TouchForwardFilter::TouchActive
            << QEvent::None
            << EventTypeList();

    QTest::newRow("target active, TouchBegin")
            << TouchForwardFilter::TouchActive
            << QEvent::TouchBegin
            << (EventTypeList() << QEvent::TouchUpdate);

    QTest::newRow("target active, TouchUpdate")
            << TouchForwardFilter::TouchActive
            << QEvent::TouchUpdate
            << (EventTypeList()
                << QEvent::TouchUpdate // With moves & releases converted to presses.
                << QEvent::TouchUpdate); // Without presses, only moves & releases.

    QTest::newRow("target active, TouchEnd")
            << TouchForwardFilter::TouchActive
            << QEvent::TouchEnd
            << (EventTypeList()
                << QEvent::TouchUpdate // With releases converted to presses.
                << QEvent::TouchUpdate); // With releases. Target's initial state prevents sending TouchEnd.
}

void Ut_TouchForwardFilter::testInitialEventType()
{
    delete subject;
    target->touchEvents.clear();

    QFETCH(TouchForwardFilter::ItemTouchState, initialTargetState);
    QFETCH(QEvent::Type, initialEventType);
    QFETCH(EventTypeList, expectedTargetEventTypes);


    QTouchEvent initialEvent(initialEventType);
    QTouchEvent *event = initialEventType == QEvent::None
                         ? 0 : &initialEvent;

    subject = new TouchForwardFilter(target, initialTargetState,
                                     origin, event);

    QCOMPARE(target->touchEvents.count(),
             expectedTargetEventTypes.count());

    for (int i = 0; i < expectedTargetEventTypes.count(); ++i) {
        QEvent::Type actualType = target->touchEvents.at(i).type();
        QEvent::Type expectedType = expectedTargetEventTypes.at(i);
        qDebug() << "actual:" << actualType << ", expected:" << expectedType;
        QCOMPARE(actualType, expectedType);
    }
}

void Ut_TouchForwardFilter::testDestroyedByTarget()
{
    QSignalSpy spy(subject, SIGNAL(destroyed()));
    delete target;
    QCOMPARE(spy.count(), 1);
    QVERIFY(!subject); // QPointer magic
}

void Ut_TouchForwardFilter::testDestroyedByOriginTouchEnd()
{
    QSignalSpy spy(subject, SIGNAL(destroyed()));

    QTouchEvent event(QEvent::TouchEnd);
    qApp->sendEvent(origin, &event);

    QCOMPARE(spy.count(), 0);

    // deleteLater destroys object in event queue.
    qApp->processEvents();

    QCOMPARE(spy.count(), 1);
    QVERIFY(!subject); // QPointer magic
}

void Ut_TouchForwardFilter::testDestroyedByTargetHidden()
{
    QSignalSpy spy(subject, SIGNAL(destroyed()));

    // Hide target. Must be done from event queue for deleteLater() to work.
    qRegisterMetaType<QGraphicsItem *>();
    QMetaObject::invokeMethod(this, "hideSlot",
                              Qt::QueuedConnection,
                              Q_ARG(QGraphicsItem *, target));
    qApp->processEvents();

    QCOMPARE(spy.count(), 0);

    // Delete object.
    qApp->processEvents();

    QCOMPARE(spy.count(), 1);
    QVERIFY(!subject); // QPointer magic
}

void Ut_TouchForwardFilter::testUniqueTouchId()
{
    GraphicsObject graphicsObjects[2];

    QMap<int, int> idCounts;

    qDebug() << "Maximum accepted touch id is" << TouchForwardFilter::MaxTouchPointId;

    for (int objIndex = 0; objIndex < 2; ++objIndex) {
        GraphicsObject &obj = graphicsObjects[objIndex];

        for (int id = 0; id <= TouchForwardFilter::MaxTouchPointId; ++id) {
            idCounts[TouchForwardFilter::uniqueTouchId(obj, id)]++;
        }
    }

    foreach (int count, idCounts.values()) {
        QCOMPARE(count, 1);
    }
}

void Ut_TouchForwardFilter::testTouchPoints_data()
{
    QTest::addColumn<TouchEvent>("eventToOrigin");
    QTest::addColumn<TouchEventList>("expectedTouchEvents");

    QTest::newRow("Simple mapping of two touch points")
        << TouchEvent(QEvent::TouchBegin,
                      TouchPointList()
                          << createTouchPoint(0, Qt::TouchPointPressed, QPointF(0, 0))
                          << createTouchPoint(1, Qt::TouchPointPressed, QPointF(1, 1)))
        << (TouchEventList()
            << TouchEvent(QEvent::TouchBegin,
                          TouchPointList()
                              << createTouchPoint(TouchForwardFilter::uniqueTouchId(*origin, 0), Qt::TouchPointPressed, QPointF(0, 0))
                              << createTouchPoint(TouchForwardFilter::uniqueTouchId(*origin, 1), Qt::TouchPointPressed, QPointF(1, 1))));

    // Due to TouchUpdate being first event, it will be converted into TouchBegin + TouchUpdate.
    QTest::newRow("Press conversions #1")
        << TouchEvent(QEvent::TouchUpdate,
                      TouchPointList()
                          << createTouchPoint(0, Qt::TouchPointPressed,  QPointF(0, 0))
                          << createTouchPoint(1, Qt::TouchPointMoved,    QPointF(20, 20), QPointF(2, 2))
                          << createTouchPoint(2, Qt::TouchPointReleased, QPointF(30, 30), QPointF(3, 3))
                          << createTouchPoint(3, Qt::TouchPointMoved,    QPointF(40, 40), QPointF(4, 4)))
        << (TouchEventList()
            << TouchEvent(QEvent::TouchBegin,
                          TouchPointList()
                              << createTouchPoint(TouchForwardFilter::uniqueTouchId(*origin, 0), Qt::TouchPointPressed, QPointF(0, 0),   QPointF(0, 0))
                              << createTouchPoint(TouchForwardFilter::uniqueTouchId(*origin, 1), Qt::TouchPointPressed, QPointF(20, 20), QPointF(20, 20))
                              << createTouchPoint(TouchForwardFilter::uniqueTouchId(*origin, 2), Qt::TouchPointPressed, QPointF(30, 30), QPointF(30, 30))
                              << createTouchPoint(TouchForwardFilter::uniqueTouchId(*origin, 3), Qt::TouchPointPressed, QPointF(40, 40), QPointF(40, 40)))
            << TouchEvent(QEvent::TouchUpdate,
                          TouchPointList()
                              // All except TouchPointReleased are converted to TouchPointStationary.
                              << createTouchPoint(TouchForwardFilter::uniqueTouchId(*origin, 0), Qt::TouchPointStationary, QPointF(0, 0),   QPointF(0, 0))
                              << createTouchPoint(TouchForwardFilter::uniqueTouchId(*origin, 1), Qt::TouchPointStationary, QPointF(20, 20), QPointF(20, 20))
                              << createTouchPoint(TouchForwardFilter::uniqueTouchId(*origin, 2), Qt::TouchPointReleased,   QPointF(30, 30), QPointF(30, 30))
                              << createTouchPoint(TouchForwardFilter::uniqueTouchId(*origin, 3), Qt::TouchPointStationary, QPointF(40, 40), QPointF(40, 40))));

    QTest::newRow("Press conversions #2")
        << TouchEvent(QEvent::TouchEnd,
                      TouchPointList()
                          << createTouchPoint(0, Qt::TouchPointReleased, QPointF(1, 1), QPointF(10, 10)))
        << (TouchEventList()
            << TouchEvent(QEvent::TouchBegin,
                          TouchPointList()
                              << createTouchPoint(TouchForwardFilter::uniqueTouchId(*origin, 0), Qt::TouchPointPressed, QPointF(1, 1), QPointF(1, 1)))
            << TouchEvent(QEvent::TouchEnd,
                          TouchPointList()
                              << createTouchPoint(TouchForwardFilter::uniqueTouchId(*origin, 0), Qt::TouchPointReleased,  QPointF(1, 1), QPointF(1, 1))));
}

void Ut_TouchForwardFilter::testTouchPoints()
{
    QFETCH(TouchEvent, eventToOrigin);
    QFETCH(TouchEventList, expectedTouchEvents);

    qApp->sendEvent(origin, &eventToOrigin);

    QCOMPARE(target->touchEvents.count(), expectedTouchEvents.count());

    for (int i = 0; i < expectedTouchEvents.count(); ++i) {
        const QTouchEvent &actualEvent = target->touchEvents.at(i);
        const QTouchEvent &expectedEvent = expectedTouchEvents.at(i);

        QCOMPARE(actualEvent.touchPoints().count(),
                 expectedEvent.touchPoints().count());

        for (int tp = 0; tp < actualEvent.touchPoints().count(); ++tp) {
            const QTouchEvent::TouchPoint actualTp = actualEvent.touchPoints().at(tp);
            const QTouchEvent::TouchPoint expectedTp = expectedEvent.touchPoints().at(tp);

            QCOMPARE(actualTp.id(),            expectedTp.id());

            QCOMPARE(actualTp.state(),         expectedTp.state());

            QCOMPARE(actualTp.pos(),           expectedTp.pos());
            QCOMPARE(actualTp.scenePos(),      expectedTp.scenePos());
            QCOMPARE(actualTp.screenPos(),     expectedTp.screenPos());

            QCOMPARE(actualTp.lastPos(),       expectedTp.lastPos());
            QCOMPARE(actualTp.lastScenePos(),  expectedTp.lastScenePos());
            QCOMPARE(actualTp.lastScreenPos(), expectedTp.lastScreenPos());
        }
    }
}

QTouchEvent::TouchPoint Ut_TouchForwardFilter::createTouchPoint(int id,
                                                                Qt::TouchPointStates states,
                                                                const QPointF &pos,
                                                                QPointF lastPos)
{
    if (states & Qt::TouchPointPressed) {
        lastPos = pos;
    }

    QTouchEvent::TouchPoint tp(id);
    tp.setState(states);
    tp.setPos(pos);
    tp.setScenePos(pos);
    tp.setScreenPos(pos);
    tp.setLastPos(lastPos);
    tp.setLastScenePos(lastPos);
    tp.setLastScreenPos(lastPos);
    return tp;
}

void Ut_TouchForwardFilter::hideSlot(QGraphicsItem *item)
{
    item->hide();
}

QTEST_MAIN(Ut_TouchForwardFilter);
