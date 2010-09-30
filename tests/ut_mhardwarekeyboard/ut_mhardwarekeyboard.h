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


#ifndef UT_MHARDWAREKEYBOARD_H
#define UT_MHARDWAREKEYBOARD_H

#include <QObject>
#include <QtTest/QTest>

class MApplication;
class MHardwareKeyboard;
class TestInputContextConnection;

class Ut_MHardwareKeyboard : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    MHardwareKeyboard *m_hkb;
    TestInputContextConnection *inputContextConnection;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testBasicModifierCycles_data();
    void testBasicModifierCycles();
    void testAutoCaps();
    void testStateReset_data();
    void testStateReset();
    void testModifierInNonTextContentType_data();
    void testModifierInNonTextContentType();
    void testShiftShiftCapsLock_data();
    void testShiftShiftCapsLock();
    void testOtherModifier_data();
    void testOtherModifier();

    void testSymClick();
    void testSymPlusCharacterBasic_data();
    void testSymPlusCharacterBasic();
    void testSymPlusCharSwitchs();

    void testDelete_data();
    void testDelete();

    void testDirectInputMode();

    void testSwitchLayout();

    void testControlModifier();

    void testCorrectToAcceptedCharacter();

    void testKeyInsertionOnReleaseAfterReset();

    void testDeadKeys();

private:
    bool checkLatchedState(unsigned int mask, unsigned int value) const;
    bool checkLockedState(unsigned int mask, unsigned int value) const;
    void setState(int state) const;

    // Wrappers for MHardwareKeyboard::filterKeyEvent() to make calls shorter
    bool filterKeyRelease(Qt::Key keyCode, Qt::KeyboardModifiers modifiers,
                          const QString &text, quint32 nativeScanCode, quint32 nativeModifiers) const;
    bool filterKeyPress(Qt::Key keyCode, Qt::KeyboardModifiers modifiers,
                        const QString &text, quint32 nativeScanCode, quint32 nativeModifiers) const;
};

#endif

