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
#ifndef UT_TOUCHFORWARDFILTER_H
#define UT_TOUCHFORWARDFILTER_H

#include <QObject>
#include <QtTest/QTest>
#include <QPair>

class TouchForwardFilter;
class TouchTargetWidget;
class QGraphicsScene;
class QGraphicsItem;
class QGraphicsWidget;

class Ut_TouchForwardFilter : public QObject
{
    Q_OBJECT
public:
    enum TouchEventReceiver {
        Origin,
        Target
    };
    typedef QPair<TouchEventReceiver, QEvent::Type> EventReceiverPair;
    typedef QList<EventReceiverPair> EventReceiverPairList;
    typedef QList<QEvent::Type> EventTypeList;
    typedef QList<QTouchEvent::TouchPoint> TouchPointList;

    // To be able to give QTouchEvent to Q_DECLARE_METATYPE
    // it must have a public default constructor. Therefore
    // we wrap it here.
    class TouchEvent : public QTouchEvent
    {
    public:
        TouchEvent()
            : QTouchEvent(QEvent::TouchBegin) {}
        TouchEvent(QEvent::Type type,
                   const TouchPointList &tpList)
            : QTouchEvent(type,
                          QTouchEvent::TouchScreen,
                          Qt::NoModifier, 0,
                          tpList) {}
    };

    typedef QList<TouchEvent> TouchEventList;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testForwardType_data();
    void testForwardType();
    void testInitialEventType_data();
    void testInitialEventType();
    void testDestroyedByTarget();
    void testDestroyedByOriginTouchEnd();
    void testDestroyedByTargetHidden();
    void testUniqueTouchId();
    void testTouchPoints_data();
    void testTouchPoints();

public slots:
    void hideSlot(QGraphicsItem *item);

private:
    static QTouchEvent::TouchPoint createTouchPoint(int id,
                                                    Qt::TouchPointStates states,
                                                    const QPointF &pos,
                                                    QPointF lastPos = QPointF());

    QGraphicsScene *scene;

    QPointer<TouchForwardFilter> subject;
    QPointer<TouchTargetWidget> target;
    QPointer<QGraphicsWidget> origin;
};

#endif // UT_TOUCHFORWARDFILTER_H
