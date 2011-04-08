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


#ifndef UT_MHARDWAREKEYBOARD_H
#define UT_MHARDWAREKEYBOARD_H

#include <QObject>
#include <QtTest/QTest>

class MApplication;
class MHardwareKeyboard;
class TestInputMethodHost;

class Ut_MHardwareKeyboard : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    MHardwareKeyboard *m_hkb;
    TestInputMethodHost *inputMethodHost;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testBasicModifierCycles_data();
    void testBasicModifierCycles();
    void testAutoCaps();
    void testShiftPlusCharacter_data();
    void testShiftPlusCharacter();
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

    void testArrowKeyFiltering();

    void testCtrlShortcutsWithFn();

    void testPressTwoKeys();
    void testPressTwoKeysWithLatch();

    void testLongPressUndo();

private:
    bool checkLatchedState(unsigned int mask, unsigned int value) const;
    bool checkLockedState(unsigned int mask, unsigned int value) const;
    void setState(int state) const;

    // Wrappers for MHardwareKeyboard::filterKeyEvent() to make calls shorter
    bool filterKeyRelease(Qt::Key keyCode, Qt::KeyboardModifiers modifiers,
                          const QString &text, quint32 nativeScanCode, quint32 nativeModifiers,
                          unsigned long time = 0) const;
    bool filterKeyPress(Qt::Key keyCode, Qt::KeyboardModifiers modifiers,
                        const QString &text, quint32 nativeScanCode, quint32 nativeModifiers,
                        unsigned long time = 0) const;
};

#endif

