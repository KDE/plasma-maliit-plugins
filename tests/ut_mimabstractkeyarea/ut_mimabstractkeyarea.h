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

#ifndef UT_MIMABSTRACTKEYAREA_H
#define UT_MIMABSTRACTKEYAREA_H

#include <QList>
#include <QObject>
#include <QtTest/QtTest>

#include "mnamespace.h"
#include "flickgesture.h"

#include <QMap>

class MApplication;
class MImAbstractKeyArea;
class KeyboardData;
class MImAbstractKey;
class MSceneWindow;
class MImAbstractKeyAreaStyle;

class Ut_MImAbstractKeyArea : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    MImAbstractKeyArea *subject;
    KeyboardData *keyboard;
    MSceneWindow *sceneWindow;

    QMap<int, QTouchEvent::TouchPoint> touchPointMap;
    int mouseEventFollowId;
    QPointF mouseLastPos;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testLandscapeBoxSize();
    void testPortraitBoxSize();
    void testPaint();
    void testDeadkeys_data();
    void testDeadkeys();
    void testSelectedDeadkeys();
    void testTwoDeadInOne_data();
    void testTwoDeadInOne();
    void testExtendedLabels();
    void testKeyId();
    void testContentType_data();
    void testContentType();
    void testImportedLayouts();
    void testPopup();
    void testPopupOwnership();
    void testInitialization();
    void testShiftCapsLock();
    void testOverridenKey_data();
    void testOverridenKey();
    void testRtlKeys_data();
    void testRtlKeys();
    void testLongKeyPress();
    void testKeyLayout();

    void testTouchPoints_data();
    void testTouchPoints();

    void testReset();

    void testStyleModesFromKeyCount_data();
    void testStyleModesFromKeyCount();

    void testLockVerticalMovement_data();
    void testLockVerticalMovement();

    void testFlickEvent_data();
    void testFlickEvent();

    void testTouchPointCount_data();
    void testTouchPointCount();

    void testResetActiveKeys();

private:
    enum TouchEvent {
        Press,
        Move,
        Release
    };

    void touchEvent(TouchEvent event,
                    int id, const QPointF &pos);
    void changeOrientation(M::OrientationAngle angle);
    QSize defaultLayoutSize();

    MImAbstractKey *keyAt(unsigned int row,
                      unsigned int column) const;

    void clickKey(MImAbstractKey *key);

    static MImAbstractKeyArea *createEmptyArea();

    static MImAbstractKeyArea *createArea(const QString &labels,
                                          const QSize &size,
                                          bool usePopup = false);

public:
    enum TestOperation {
        TestOpClickDeadKey,
        TestOpSetShiftOn,
        TestOpSetShiftOff
    };
    enum HitPosition {
        LeftEdge,
        RightEdge,
        BottomEdge,
        TopEdge,
        Center
    };

    typedef QList<TestOperation> TestOpList;

    struct TouchTestOperation {
        TouchTestOperation(TouchEvent event,
                           const QPoint &keyRowCol,
                           int tpId = 0,
                           HitPosition hitPos = Center,
                           QPointF offset = QPointF())
           : event(event),
             keyPos(keyRowCol),
             touchPointId(tpId),
             hitPos(hitPos),
             offset(offset)
        {}
        TouchEvent event;
        QPoint keyPos;
        int touchPointId;
        HitPosition hitPos;
        QPointF offset;
    };

    typedef QList<TouchTestOperation> TouchOpList;
};

#endif // UT_MIMABSTRACTKEYAREA_H
