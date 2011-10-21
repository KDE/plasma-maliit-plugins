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

    virtual void sendPreeditString(const QString &string,
                                   const QList<MInputMethod::PreeditTextFormat> &preeditFormats,
                                   int replaceStart = 0,
                                   int replaceLength = 0,
                                   int cursorPos = -1);
    virtual void sendCommitString(const QString &string, int replaceStart = 0,
                                  int replaceLength = 0, int cursorPos = -1);
    virtual void sendKeyEvent(const QKeyEvent &keyEvent,
                              MInputMethod::EventRequestType requestType = MInputMethod::EventRequestBoth);
    virtual void notifyImInitiatedHiding();

    virtual int contentType(bool &);
    virtual int inputMethodMode(bool &);
    virtual bool hasSelection(bool &valid);
    virtual QRect preeditRectangle(bool &);
    virtual QRect cursorRectangle(bool &valid);
    virtual bool correctionEnabled(bool &);
    virtual bool predictionEnabled(bool &);
    virtual void setGlobalCorrectionEnabled(bool);
    virtual bool autoCapitalizationEnabled(bool &);
    virtual void copy();
    virtual void paste();

    virtual bool surroundingText(QString &text, int &cursorPosition);
    virtual int anchorPosition(bool &valid);
    virtual void setRedirectKeys(bool enabled);
    virtual void setDetectableAutoRepeat(bool enabled);
    virtual void setInputModeIndicator(MInputMethod::InputModeIndicator indicator);

    virtual void switchPlugin(MInputMethod::SwitchDirection direction);
    virtual void switchPlugin(const QString &pluginName);
    virtual void setScreenRegion(const QRegion &region);
    virtual void setInputMethodArea(const QRegion &region);
    virtual void showSettings();
    virtual void setSelection(int start, int length);
    virtual void setOrientationAngleLocked(bool lock);
    virtual QString selection(bool &valid);
    virtual QList<MImPluginDescription> pluginDescriptions(MInputMethod::HandlerState state) const;
    virtual void setLanguage(const QString &language);

    QString preedit;
    QString commit;
    QList<QKeyEvent *> keyEvents;
    bool globalCorrectionEnabled;
    QList<MInputMethod::PreeditTextFormat> preeditFormats_;

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
    QList<QRegion> screenRegions;
    int setInputMethodAreaCalls;
    QList<QRegion> inputMethodAreas;

    bool predictionValid_;
    bool predictionEnabled_;
    bool correctionValid_;
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
    QRect preeditRectangleReturnValue;
    QRect cursorRectangleReturnValue;

    bool orientationAngleLocked;
    QString currentSelection;
};

#endif
