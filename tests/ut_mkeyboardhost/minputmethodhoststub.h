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



#ifndef MINPUTMETHODHOSTSTUB_H

#include <QObject>
#include <QString>
#include <mabstractinputmethodhost.h>
#include <minputmethodnamespace.h>

class MInputMethodHostStub: public MAbstractInputMethodHost
{
    Q_OBJECT

public:
    MInputMethodHostStub();
    virtual ~MInputMethodHostStub();

    void clear();

    virtual void sendPreeditString(const QString &string, MInputMethod::PreeditFace preeditFace);
    virtual void sendCommitString(const QString &string);
    virtual void sendKeyEvent(const QKeyEvent &keyEvent,
                              MInputMethod::EventRequestType requestType = MInputMethod::EventRequestBoth);
    virtual void notifyImInitiatedHiding();

    virtual int contentType(bool &);
    virtual int inputMethodMode(bool &);
    virtual bool hasSelection(bool &valid);
    virtual QRect preeditRectangle(bool &);
    virtual bool correctionEnabled(bool &);
    virtual bool predictionEnabled(bool &);
    virtual void setGlobalCorrectionEnabled(bool);
    virtual bool autoCapitalizationEnabled(bool &);
    virtual void copy();
    virtual void paste();

    virtual bool surroundingText(QString &text, int &cursorPosition);
    virtual void setRedirectKeys(bool enabled);
    virtual void setDetectableAutoRepeat(bool enabled);
    virtual void setInputModeIndicator(MInputMethod::InputModeIndicator indicator);

    virtual void switchPlugin(MInputMethod::SwitchDirection direction);
    virtual void switchPlugin(const QString &pluginName);
    virtual void setScreenRegion(const QRegion &region);
    virtual void setInputMethodArea(const QRegion &region);
    virtual void showSettings();

    QString preedit;
    QString commit;
    QList<QKeyEvent *> keyEvents;
    bool globalCorrectionEnabled;

    int sendPreeditCalls;
    int sendCommitStringCalls;
    int sendKeyEventCalls;
    int notifyImInitiatedHidingCalls;
    int setGlobalCorrectionEnabledCalls;
    int predictionEnabledCalls;
    int correctionEnabledCalls;
    int contentTypeCalls;
    int copyCalls;
    int pasteCalls;
    int addRedirectedKeyCalls;
    int removeRedirectedKeyCalls;
    int setNextKeyRedirectedCalls;
    int setScreenRegionCalls;
    int setInputMethodAreaCalls;

    bool predictionEnabled_;
    bool correctionEnabled_;
    bool autoCapitalizationEnabled_;
    int contentType_;
    int inputMethodMode_;

    QString surroundingString;
    int cursorPos;
    int inputmethodMode_;
    bool textSelected;
    bool keyRedirectionEnabled;
    MInputMethod::InputModeIndicator indicator;
};

#endif
