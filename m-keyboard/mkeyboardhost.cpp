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

#include "mkeyboardhost.h"
#include "mkeyboardhost_p.h"

#include "flickgesturerecognizer.h"
#include "mvirtualkeyboardstyle.h"
#include "mvirtualkeyboard.h"
#include "mhardwarekeyboard.h"
#include "mimcorrectionhost.h"
#include "keyboarddata.h"
#include "layoutsmanager.h"
#include "symbolview.h"
#include "mimtoolbar.h"
#include "sharedhandlearea.h"
#include "regiontracker.h"

#include <mimenginefactory.h>
#include <mabstractinputmethodhost.h>
#include <mplainwindow.h>
#include <mtoolbardata.h>
#include <mgconfitem.h>

#include <QDebug>
#include <QKeyEvent>
#include <QFile>
#include <QRegExp>

#include <mreactionmap.h>
#include <MScene>
#include <MSceneManager>
#include <MSceneWindow>
#include <MBanner>
#include <MLibrary>

M_LIBRARY

namespace
{
    const QString InputMethodList("MInputMethodList");
    const QString DefaultInputLanguage("en_GB");
    // TODO: check that these paths still hold
    const QString CorrectionSetting("/meegotouch/inputmethods/virtualkeyboard/correctionenabled");
    const bool DefaultCorrectionSettingOption = true;
    const QString InputMethodCorrectionEngine("/meegotouch/inputmethods/correctionengine");
    const QRegExp AutoCapsTrigger("[.?!¡¿] +$");
    const QString AutoPunctuationTriggers(".,?!");
    const int MaximumErrorCorrectionCandidate = 5;
    const int RotationDuration = 750; //! After vkb hidden, how long to wait until shown again
    const int AutoBackspaceDelay = 500;      // in ms
    const int LongPressTime = 600;           // in ms
    const int BackspaceRepeatInterval = 100; // in ms
    const int MultitapTime = 1500;           // in ms
    const Qt::KeyboardModifier FnLevelModifier = Qt::GroupSwitchModifier;
    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";
    const char * const NotificationObjectName = "ModifierLockNotification";
    const int KeysRequiredForFastTypingMode = 3;
    const int FastTypingTimeout = 700; //! Milliseconds to idle before leaving fast typing mode.
}

MKeyboardHost::CycleKeyHandler::CycleKeyHandler(MKeyboardHost &parent)
    : QObject(&parent),
    host(parent),
    timer(),
    cycleText(),
    prevEvent()
{
    timer.setSingleShot(true);
    timer.setInterval(MultitapTime);
    connect(&timer, SIGNAL(timeout()), this, SLOT(commitCycleKey()));
}

MKeyboardHost::CycleKeyHandler::~CycleKeyHandler()
{
}

bool MKeyboardHost::CycleKeyHandler::handleTextInputKeyClick(const KeyEvent &event)
{
    // If there's a pending keypress, handle it:
    if (timer.isActive() && !(event == prevEvent)) {
        timer.stop();
        commitCycleKey();
    }

    if (event.specialKey() != KeyEvent::CycleSet) {
        return false;  // Host should handle non-cyclekeys.
    }

    if (event.text().length() == 0) {
        qWarning() << __PRETTY_FUNCTION__ << "Empty cycleset in layout";
        return true;
    }

    if (event == prevEvent) {
        // Cycling through same cycle key
        if (host.preedit.length() > 0) {
            // FIXME: appending the cycle key to the preedit --> pressing cycle key
            // when there already is something in preedit will cause the whole preedit
            // to be commited after timeout
            host.preedit = host.preedit.left(host.preedit.length() - 1);
            host.candidates.clear();
        }
        cycleIndex = (cycleIndex + 1) % cycleText.size();
    }
    else {
        // Different key than last time, reset index and cycle text
        prevEvent = event;
        cycleIndex = 0;
        cycleText = event.text();
    }

    // Note: If the cycle key is used together with error correction preedit,
    // to insert cycle character in the middle of preedit, then we can not
    // simply set preeditCursorPos to -1.
    host.preedit += cycleText[cycleIndex];
    host.preeditCursorPos = -1;
    QList<MInputMethod::PreeditTextFormat> preeditFormats;
    MInputMethod::PreeditTextFormat preeditFormat(0, host.preedit.length(),
                                                  MInputMethod::PreeditNoCandidates);
    preeditFormats << preeditFormat;
    host.inputMethodHost()->sendPreeditString(host.preedit,
                                              preeditFormats);

    timer.start();

    return true;
}

void MKeyboardHost::CycleKeyHandler::reset()
{
    timer.stop();
}

void MKeyboardHost::CycleKeyHandler::commitCycleKey()
{
    if (cycleText.length() > 0) {
        host.sendString(host.preedit);
        host.preedit.clear();
        cycleText.clear();
        prevEvent = KeyEvent();
    }
}

MKeyboardHost::MKeyboardHost(MAbstractInputMethodHost *imHost, QObject *parent)
    : MAbstractInputMethod(imHost, parent),
      vkbStyleContainer(0),
      correctionHost(0),
      vkbWidget(0),
      symbolView(0),
      imCorrectionEngine(0),
      inputMethodCorrectionSettings(new MGConfItem(CorrectionSetting)),
      inputMethodCorrectionEngine(new MGConfItem(InputMethodCorrectionEngine)),
      correctionEnabled(false),
      autoCapsEnabled(true),
      autoCapsTriggered(false),
      cursorPos(-1),
      preeditCursorPos(-1),
      hasSelection(false),
      inputMethodMode(M::InputMethodModeNormal),
      backspaceTimer(),
      rotationTimer(),
      shiftHeldDown(false),
      activeState(MInputMethod::OnScreen),
      modifierLockOnBanner(0),
      haveFocus(false),
      sipRequested(false),
      visualizationPriority(false),
      enableMultiTouch(MGConfItem(MultitouchSettings).value().toBool()),
      cycleKeyHandler(new CycleKeyHandler(*this)),
      currentIndicatorDeadKey(false),
      engineLayoutDirty(false),
      backspaceMode(NormalBackspaceMode),
      wordTrackerSuggestionAcceptedWithSpace(false),
      fastTypingKeyCount(0),
      fastTypingEnabled(false)
{
    RegionTracker::createInstance();
    connect(&RegionTracker::instance(), SIGNAL(regionChanged(const QRegion &)),
            imHost, SLOT(setScreenRegion(const QRegion &)));
    connect(&RegionTracker::instance(), SIGNAL(inputMethodAreaChanged(const QRegion &)),
            imHost, SLOT(setInputMethodArea(const QRegion &)));
    connect(&RegionTracker::instance(), SIGNAL(reactionMapUpdateNeeded()),
            this, SLOT(updateReactionMaps()));
    RegionTracker::instance().enableSignals(false);

    displayHeight = MPlainWindow::instance()->visibleSceneSize(M::Landscape).height();
    displayWidth  = MPlainWindow::instance()->visibleSceneSize(M::Landscape).width();

    sceneWindow = new MSceneWindow;
    sceneWindow->setManagedManually(true); // we want the scene window to remain in origin

    // Enforcing full viewport updates helps to paint correctly in software mode.
    MPlainWindow::instance()->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    // Because we set vkbWidget as a child of sceneWindow the vkb
    // will always be in correct orientation. However the animation will be
    // affected as well. If we want to keep the current hiding/showing animation
    // (up & down) without getting it combined with the rotation animation
    // we have at least two options:
    // 1) Make our own MOrientationAnimation when libmeegotouch begins supporting
    //    setting it, through theme probably.
    // 2) Add widgets directly to scene (detached from MSceneManager) and
    //    update their transformations by hand.

    vkbStyleContainer = new MVirtualKeyboardStyleContainer;
    vkbStyleContainer->initialize("MVirtualKeyboard", "MVirtualKeyboardView", 0);

    LayoutsManager::createInstance();

    FlickGestureRecognizer::registerSharedRecognizer();

    vkbWidget = new MVirtualKeyboard(LayoutsManager::instance(), vkbStyleContainer, sceneWindow);
    vkbWidget->setInputMethodMode(static_cast<M::InputMethodMode>(inputMethodMode));

    connect(vkbWidget, SIGNAL(keyClicked(const KeyEvent &)),
            this, SLOT(handleKeyClick(const KeyEvent &)));
    connect(vkbWidget, SIGNAL(keyPressed(const KeyEvent &)),
            this, SLOT(handleKeyPress(const KeyEvent &)));
    connect(vkbWidget, SIGNAL(keyReleased(const KeyEvent &)),
            this, SLOT(handleKeyRelease(const KeyEvent &)));
    connect(vkbWidget, SIGNAL(longKeyPressed(const KeyEvent &)),
            this, SLOT(handleLongKeyPress(const KeyEvent &)));
    connect(vkbWidget, SIGNAL(showSymbolViewRequested()),
            this, SLOT(showSymbolView()));

    connect(vkbWidget, SIGNAL(userInitiatedHide()),
            this, SLOT(userHide()));

    connect(vkbWidget, SIGNAL(pluginSwitchRequired(MInputMethod::SwitchDirection)),
            this, SLOT(switchPlugin(MInputMethod::SwitchDirection)));

    // construct hardware keyboard object
    hardwareKeyboard = new MHardwareKeyboard(*imHost, this);
    connect(hardwareKeyboard, SIGNAL(symbolKeyClicked()),
            this, SLOT(handleSymbolKeyClick()));

    bool ok = connect(vkbWidget, SIGNAL(copyPasteClicked(CopyPasteState)),
                      this, SLOT(sendCopyPaste(CopyPasteState)));
    Q_UNUSED(ok); // if Q_NO_DEBUG is defined then the assert won't be used
    Q_ASSERT(ok);

    imToolbar = new MImToolbar;

    ok = connect(imToolbar, SIGNAL(copyPasteRequest(CopyPasteState)),
                 this, SLOT(sendCopyPaste(CopyPasteState)));
    Q_ASSERT(ok);
    ok = connect(imToolbar, SIGNAL(sendKeyEventRequest(const QKeyEvent &)),
                 this, SLOT(sendKeyEvent(const QKeyEvent &)));
    Q_ASSERT(ok);
    ok = connect(imToolbar, SIGNAL(sendStringRequest(const QString &)),
                 this, SLOT(sendStringFromToolbar(const QString &)));
    Q_ASSERT(ok);
    ok = connect(imToolbar, SIGNAL(copyPasteClicked(CopyPasteState)),
                 this, SLOT(sendCopyPaste(CopyPasteState)));
    Q_ASSERT(ok);
    ok = connect(imToolbar, SIGNAL(closeKeyboardRequest()),
                 this, SLOT(userHide()));
    Q_ASSERT(ok);

    sharedHandleArea = new SharedHandleArea(*imToolbar, sceneWindow);
    sharedHandleArea->setInputMethodMode(static_cast<M::InputMethodMode>(inputMethodMode));

    // Set z value below default level (0.0) so popup will be on top of shared handle area.
    sharedHandleArea->setZValue(-1.0);

    vkbWidget->setSharedHandleArea(sharedHandleArea);
    sharedHandleArea->watchOnWidget(vkbWidget);


    createCorrectionCandidateWidget();

    // Don't listen to device orientation.  Applications can be in different orientation
    // than the device (especially plain qt apps). See NB#185013 - Locking VKB orientation.
    MPlainWindow::instance()->lockOrientationAngle();

    symbolView = new SymbolView(LayoutsManager::instance(), vkbStyleContainer,
                                vkbWidget->selectedLayout(), sceneWindow);
    connect(symbolView, SIGNAL(geometryChanged()),
            this, SLOT(handleSymbolViewGeometryChange()));
    connect(symbolView, SIGNAL(visibleChanged()), this, SLOT(handleSymbolViewVisibleChanged()));

    connect(symbolView, SIGNAL(keyClicked(const KeyEvent &)),
            this, SLOT(handleKeyClick(const KeyEvent &)));
    connect(symbolView, SIGNAL(keyPressed(const KeyEvent &)),
            this, SLOT(handleKeyPress(const KeyEvent &)));
    connect(symbolView, SIGNAL(keyReleased(const KeyEvent &)),
            this, SLOT(handleKeyRelease(const KeyEvent &)));
    connect(symbolView, SIGNAL(longKeyPressed(const KeyEvent &)),
            this, SLOT(handleLongKeyPress(const KeyEvent &)));

    sharedHandleArea->watchOnWidget(symbolView);

    connect(vkbWidget, SIGNAL(layoutChanged(const QString &)),
            this, SLOT(handleVirtualKeyboardLayoutChanged(const QString &)));

    connect(vkbWidget, SIGNAL(shiftLevelChanged()),
            this, SLOT(handleVirtualKeyboardCapsLock()));

    connect(vkbWidget, SIGNAL(shiftLevelChanged()),
            this, SLOT(updateSymbolViewLevel()));

    connect(hardwareKeyboard, SIGNAL(shiftStateChanged()),
            this, SLOT(updateSymbolViewLevel()));

    if (!inputMethodCorrectionEngine->value().isNull()) {
        imCorrectionEngine = MImEngineFactory::instance()->createEngineWords(
                                inputMethodCorrectionEngine->value().toString());

        if (imCorrectionEngine) {
            initializeInputEngine();
            connect(inputMethodCorrectionSettings, SIGNAL(valueChanged()),
                    this, SLOT(synchronizeCorrectionSetting()));
        } else {
            qDebug() << __PRETTY_FUNCTION__ << "Failed to load correction engine"
                     << inputMethodCorrectionEngine->value().toString();
        }
    }

    backspaceTimer.setSingleShot(true);
    connect(&backspaceTimer, SIGNAL(timeout()), this, SLOT(autoBackspace()));

    rotationTimer.setSingleShot(true);
    connect(&rotationTimer, SIGNAL(timeout()), this, SLOT(finalizeOrientationChange()));

    fastTypingTimeout.setSingleShot(true);
    fastTypingTimeout.setInterval(FastTypingTimeout);
    connect(&fastTypingTimeout, SIGNAL(timeout()),
            this, SLOT(turnOffFastTyping()));

    hide();
    RegionTracker::instance().enableSignals(true, false);
}

MKeyboardHost::~MKeyboardHost()
{
    hideLockOnInfoBanner();
    delete hardwareKeyboard;
    hardwareKeyboard = 0;
    delete vkbWidget;
    vkbWidget = 0;
    delete symbolView;
    symbolView = 0;
    delete correctionHost;
    correctionHost = 0;
    delete sceneWindow;
    sceneWindow = 0;
    delete vkbStyleContainer;
    vkbStyleContainer = 0;
    delete inputMethodCorrectionSettings;
    inputMethodCorrectionSettings = 0;
    if (imCorrectionEngine) {
        MImEngineFactory::instance()->deleteEngine(imCorrectionEngine);
        imCorrectionEngine = 0;
    }
    backspaceMode = NormalBackspaceMode;
    backspaceTimer.stop();
    rotationTimer.stop();
    LayoutsManager::destroyInstance();
    RegionTracker::destroyInstance();
}

void MKeyboardHost::createCorrectionCandidateWidget()
{
    // construct correction candidate widget
    correctionHost = new MImCorrectionHost(sceneWindow);
    correctionHost->hideCorrectionWidget();
    connect(correctionHost, SIGNAL(candidateClicked(const QString &)),
            this, SLOT(commitString(const QString &)));
}


// TODO: it would seem that application focus state is passed to all plugins by
// MInputContextGlibDBusConnection::updateWidgetInformation, including nonactive ones.  If
// that really is the case, then what we're doing here is wrong.  Yet another reason to have
// plugin (de)activation methods?  Also see comments in MIMPluginManagerPrivate::replacePlugin.
void MKeyboardHost::handleFocusChange(bool focusIn)
{
    haveFocus = focusIn;
    if (activeState == MInputMethod::OnScreen) {
        if (focusIn) {
            // reset the temporary shift state when focus is changed
            resetVirtualKeyboardShiftState();
        }
    } else {
        if (focusIn) {
            hardwareKeyboard->enable();
        } else {
            hardwareKeyboard->disable();
            inputMethodHost()->setInputModeIndicator(MInputMethod::NoIndicator);
        }
        hideLockOnInfoBanner();
    }
}


void MKeyboardHost::show()
{
    sipRequested = true;
    if (visualizationPriority) {
        return;
    }
    // This will add scene window as child of MSceneManager's root element
    // which is the QGraphicsItem that is rotated when orientation changes.
    // It uses animation to carry out the orientation change transform
    // (e.g. rotation and position animation). We do this because transform
    // happens in the scene, not in the view (MWindow) anymore.
    MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(sceneWindow);

    if (activeState == MInputMethod::Hardware) {
        sharedHandleArea->show();
        if (!hardwareKeyboard->symViewAvailable())
            symbolView->hideSymbolView();
    } else {
        // Prevent region updates from shared handle area, let MVirtualKeyboard trigger
        // the update (just once)
        const bool wasBlocked(sharedHandleArea->blockSignals(true));
        sharedHandleArea->show();
        sharedHandleArea->blockSignals(wasBlocked);
        // Onscreen state
        if (!vkbWidget->symViewAvailable())
            symbolView->hideSymbolView();
        vkbWidget->showKeyboard();
    }

    // update input engine keyboard layout.
    updateEngineKeyboardLayout();
    updateCorrectionState();
}


void MKeyboardHost::hide()
{
    sipRequested = false;
    if (activeState == MInputMethod::OnScreen) {
        // Prevent region updates from shared handle area, let MVirtualKeyboard trigger
        // the update (just once)
        const bool wasBlocked(sharedHandleArea->blockSignals(true));
        sharedHandleArea->hide();
        sharedHandleArea->blockSignals(wasBlocked);
    } else {
        sharedHandleArea->hide();
    }
    correctionHost->hideCorrectionWidget();
    symbolView->hideSymbolView();
    vkbWidget->hideKeyboard();

    // TODO: the following line which was added to improve plugin switching (see
    // the commit comment) causes the animation started by the previous line to
    // not be seen.
    MPlainWindow::instance()->sceneManager()->disappearSceneWindowNow(sceneWindow);
}


void MKeyboardHost::handleSymbolViewGeometryChange()
{
    if (symbolView->isVisible()) {
        symbolView->setPos(0, MPlainWindow::instance()->visibleSceneSize().height() - symbolView->size().height());
    }
}

void MKeyboardHost::handleSymbolViewVisibleChanged()
{
    if (symbolView->isVisible()) {
        vkbWidget->hide();
    } else if (!visualizationPriority && sipRequested && (activeState == MInputMethod::OnScreen)) {
        vkbWidget->show();
    }
}

void MKeyboardHost::setPreedit(const QString &preeditString, int cursor)
{
    const int preeditLength = preeditString.length();
    // Application calls this method to tell the preedit and cursor information.
    // If the length of preedit is less or equal to 4, then ignore the input cursor,
    // put the cursor at the end of preedit. Otherwise using the input cursor for
    // preedit.
    preeditCursorPos = ((preeditLength > 4 && cursor <= preeditLength) || cursor < 0)
                       ? cursor : preeditLength;

    preedit = preeditString;
    candidates.clear();
    if (imCorrectionEngine) {
        imCorrectionEngine->clearEngineBuffer();
        imCorrectionEngine->reselectString(preeditString);
        candidates = imCorrectionEngine->candidates();
        correctionHost->setCandidates(candidates);
    }
    // updatePreedit() will send back the preedit with updated preedit style.
    // TODO: But this sending back preedit could cause problem. Because the
    // application is calling this setPreedit by sending an asynchronized
    // event. Maybe we need to find out the other way to update the preedit style.
    updatePreedit(preedit, candidates.count(), 0, 0, preeditCursorPos);
}

void MKeyboardHost::localSetPreedit(const QString &preeditString, int replaceStart,
                                    int replaceLength, int cursor)
{
    preedit = preeditString;
    preeditCursorPos = cursor;
    candidates.clear();
    if (imCorrectionEngine) {
        imCorrectionEngine->clearEngineBuffer();
        imCorrectionEngine->reselectString(preeditString);
        candidates = imCorrectionEngine->candidates();
        correctionHost->setCandidates(candidates);
    }
    updatePreedit(preedit, candidates.count(), replaceStart, replaceLength, cursor);
}

void MKeyboardHost::update()
{
    bool valid = false;

    hasSelection = inputMethodHost()->hasSelection(valid);
    if (valid) {
        imToolbar->setSelectionStatus(hasSelection);
    }

    const int type = inputMethodHost()->contentType(valid);
    if (valid) {
        hardwareKeyboard->setKeyboardType(static_cast<M::TextContentType>(type));
        vkbWidget->setKeyboardType(type);
    }

    updateAutoCapitalization();
    updateContext();
    updateCorrectionWidgetPosition();

    const int inputMethodModeValue = inputMethodHost()->inputMethodMode(valid);
    if (valid) {
        inputMethodMode = inputMethodModeValue;
        hardwareKeyboard->setInputMethodMode(static_cast<M::InputMethodMode>(inputMethodMode));
        vkbWidget->setInputMethodMode(static_cast<M::InputMethodMode>(inputMethodMode));
        sharedHandleArea->setInputMethodMode(static_cast<M::InputMethodMode>(inputMethodMode));
    }
}


void MKeyboardHost::resetVirtualKeyboardShiftState()
{
    // reset the temporary shift state (shift on state set by user or auto capitalization,
    // besides capslocked)
    if (activeState == MInputMethod::OnScreen && vkbWidget->shiftStatus() != ModifierLockedState) {
        autoCapsTriggered = false;
        vkbWidget->setShiftState(ModifierClearState);
    }
}

void MKeyboardHost::updateAutoCapitalization()
{
    switch (activeState) {
    case MInputMethod::OnScreen:
        autoCapsEnabled = vkbWidget->autoCapsEnabled();
        break;
    default:
        autoCapsEnabled = hardwareKeyboard->autoCapsEnabled();
        break;
    }
    bool valid = false;
    const int type = inputMethodHost()->contentType(valid);
    autoCapsEnabled = (autoCapsEnabled
                       && valid
                       && (type != M::NumberContentType)
                       && (type != M::PhoneNumberContentType));
    autoCapsEnabled = (autoCapsEnabled
                       && inputMethodHost()->autoCapitalizationEnabled(valid)
                       && valid);
    autoCapsEnabled = (autoCapsEnabled
                       && inputMethodHost()->surroundingText(surroundingText, cursorPos));

    if (!autoCapsEnabled)
        return;

    // Capitalization is determined by preedit and Auto Capitalization.
    // If there are some preedit, it should be lower case.
    // Otherwise Auto Capitalization will turn on shift when (text entry capitalization option is ON):
    //   1. at the beginning of one paragraph
    //   2. after a sentence delimiter and one or more spaces
    autoCapsTriggered = ((preedit.length() == 0)
                         && ((cursorPos == 0)
                             || ((cursorPos > 0)
                                 && (cursorPos <= surroundingText.length())
                                 && surroundingText.left(cursorPos).contains(AutoCapsTrigger))));

    if ((activeState == MInputMethod::OnScreen)
        && (vkbWidget->shiftStatus() != ModifierLockedState)) {
        // FIXME: This will break the behaviour of keeping shift latched when shift+character occured.
        // We would really need a state machine for the shift state handling.
        vkbWidget->setShiftState(autoCapsTriggered ?
                                 ModifierLatchedState : ModifierClearState);
    } else if ((activeState == MInputMethod::Hardware) &&
               (hardwareKeyboard->modifierState(Qt::ShiftModifier) != ModifierLockedState)) {
        hardwareKeyboard->setAutoCapitalization(autoCapsTriggered);
    }
}

void MKeyboardHost::updateContext()
{
    if (!correctionEnabled || !preedit.isEmpty()) {
        return;
    }

    bool valid = false;
    const int type = inputMethodHost()->contentType(valid);
    if (!valid || (type == M::NumberContentType)
        || (type == M::PhoneNumberContentType)) {
        return;
    }

    if (inputMethodHost()->surroundingText(surroundingText, cursorPos)) {
        imCorrectionEngine->setContext(surroundingText, cursorPos);
    }
}

void MKeyboardHost::reset()
{
    qDebug() << __PRETTY_FUNCTION__;
    switch (activeState) {
    case MInputMethod::OnScreen:
        resetInternalState();
        break;
    case MInputMethod::Hardware:
        hardwareKeyboard->reset();
        break;
    case MInputMethod::Accessory:
        break;
    }
}


void MKeyboardHost::resetInternalState()
{
    wordTrackerSuggestionAcceptedWithSpace = false;
    backspaceMode = NormalBackspaceMode;
    backspaceTimer.stop();
    preedit.clear();
    preeditCursorPos = -1;
    candidates.clear();
    correctionHost->reset();
    if (imCorrectionEngine)
        imCorrectionEngine->clearEngineBuffer();
    cycleKeyHandler->reset();
}

void MKeyboardHost::prepareOrientationChange()
{
    if (rotationTimer.isActive()) {
        return;
    }
    // Saves states then hide
    symbolView->prepareToOrientationChange();
    vkbWidget->prepareToOrientationChange();
    correctionHost->prepareToOrientationChange();
    MPlainWindow::instance()->sceneManager()->disappearSceneWindowNow(sceneWindow);

    // TODO: this is only a workaround for fixing the orientaton change bug.
    // The correct fix need a notification from application, to tell keyboard
    // when the orientation is finished for calling finalizeOrientationChange.
    rotationTimer.start(1000);
}

void MKeyboardHost::finalizeOrientationChange()
{
    MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(sceneWindow);

    if (imToolbar) {
        // load proper layout
        imToolbar->finalizeOrientationChange();
    }

    vkbWidget->finalizeOrientationChange();
    symbolView->finalizeOrientationChange();
    if (sharedHandleArea) {
        sharedHandleArea->finalizeOrientationChange();
    }

    // Finalize candidate list after so its region will apply.
    correctionHost->finalizeOrientationChange();

    // If correction candidate widget was open we need to reposition it.
    if (correctionHost->isActive()) {
        bool success = false;
        const QRect rect = inputMethodHost()->cursorRectangle(success);
        if (success && rect.isValid()) {
            correctionHost->setPosition(sceneWindow->mapRectFromScene(rect).toRect());
            correctionHost->showCorrectionWidget(correctionHost->candidateMode());
        } else {
            correctionHost->hideCorrectionWidget();
        }
    }

    // reload keyboard layout for engine when orientation is changed
    engineLayoutDirty = true;
    if (vkbWidget->isVisible()) {
        updateEngineKeyboardLayout();
    }
    rotationTimer.stop();
}

void MKeyboardHost::handleMouseClickOnPreedit(const QPoint &mousePos, const QRect &preeditRect)
{
    Q_UNUSED(mousePos);
    Q_UNUSED(preeditRect);
    // Shows suggestion list when there are some candidates.
    // Even show suggestion list when there is only original input word in candidates.
    if (candidates.size() <= 0)
        return;

    correctionHost->showCorrectionWidget(MImCorrectionHost::WordListMode);
}


void MKeyboardHost::handleVisualizationPriorityChange(bool priority)
{
    if (visualizationPriority == priority) {
        return;
    }

    visualizationPriority = priority;

    // TODO: hide/show by fading?
    if (sipRequested) {
        const bool wasEnabled(RegionTracker::instance().enableSignals(false));
        if (priority) {
            MPlainWindow::instance()->sceneManager()->disappearSceneWindowNow(sceneWindow);
        } else {
            MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(sceneWindow);
        }
        RegionTracker::instance().enableSignals(wasEnabled);
    }
}


void MKeyboardHost::handleAppOrientationChange(int angle)
{
    if (MPlainWindow::instance()->sceneManager()->orientationAngle()== static_cast<M::OrientationAngle>(angle))
        return;
    if (rotationTimer.isActive()) {
        rotationTimer.stop();
    }
    // The application receiving input has changed its orientation. Let's change ours.
    // Disable  the transition animation for rotation.
    MPlainWindow::instance()->sceneManager()->setOrientationAngle(static_cast<M::OrientationAngle>(angle),
                                                                  MSceneManager::ImmediateTransition);
    prepareOrientationChange();
}


void MKeyboardHost::commitString(const QString &updatedString)
{
    if (imCorrectionEngine) {
        if (candidates.count() > 1) {
            int suggestionIndex = candidates.indexOf(updatedString);
            if (suggestionIndex >= 0) {
                imCorrectionEngine->setSuggestedCandidateIndex(suggestionIndex);
            }
        }
        imCorrectionEngine->saveAndClearEngineBuffer();
    }
    inputMethodHost()->sendCommitString(updatedString);
    resetInternalState();
}


void MKeyboardHost::doBackspace()
{
    // note: backspace shouldn't start accurate mode
    if (preedit.length() > 0) {
        if (backspaceMode != AutoBackspaceMode) {
            if ((preeditCursorPos < 0) || (preeditCursorPos == preedit.length())) {
                const int cursor = (preeditCursorPos > 0) ? (preeditCursorPos - 1) : preeditCursorPos;
                localSetPreedit(preedit.left(preedit.length() - 1), 0, 0, cursor);
            } else {
                if (preeditCursorPos == 0) {
                    inputMethodHost()->sendCommitString(preedit, 0, 0, 0);
                    sendBackSpaceKeyEvent();
                    resetInternalState();
                } else {
                    --preeditCursorPos;
                    localSetPreedit(preedit.remove(preeditCursorPos, 1), 0, 0, preeditCursorPos);
                }
            }
        } else {
            resetInternalState();
            inputMethodHost()->sendCommitString("");
        }
    } else {
        sendBackSpaceKeyEvent();
    }
    // Backspace toggles shift off if it's latched except if:
    // - autoCaps is on and cursor is at 0 position
    // - text is selected (since we don't know what is
    //   selected we can't predict the next state)
    // - previous cursor position will have autoCaps
    if (vkbWidget->shiftStatus() == ModifierLatchedState
        && (!autoCapsEnabled || cursorPos != 0)
        && !hasSelection
        && (cursorPos == 0
            || !autoCapsEnabled
            || (cursorPos > 0
                && cursorPos <= surroundingText.length()
                && !surroundingText.left(cursorPos-1).contains(AutoCapsTrigger)))) {
        vkbWidget->setShiftState(ModifierClearState);
    }
}

void MKeyboardHost::autoBackspace()
{
    backspaceMode = AutoBackspaceMode;
    backspaceTimer.start(BackspaceRepeatInterval); // Must restart before doBackspace
    doBackspace();
}

void MKeyboardHost::turnOnFastTyping()
{
    if (fastTypingEnabled) {
        return;
    }
    fastTypingEnabled = true;
    inputMethodHost()->setOrientationAngleLocked(true);
}

void MKeyboardHost::turnOffFastTyping()
{
    if (!fastTypingEnabled) {
        return;
    }
    fastTypingEnabled = false;
    inputMethodHost()->setOrientationAngleLocked(false);
    fastTypingKeyCount = 0;
}

void MKeyboardHost::handleKeyPress(const KeyEvent &event)
{
    // update fast typing mode
    if (++fastTypingKeyCount >= KeysRequiredForFastTypingMode) {
        turnOnFastTyping();
    }
    fastTypingTimeout.start(); // restart

    if (event.qtKey() == Qt::Key_Shift) {
        if (shiftHeldDown) {
            return; //ignore duplicated event
        }

        if (activeState == MInputMethod::OnScreen && enableMultiTouch) {
            shiftHeldDown = true;
        }
    }

    MInputMethod::EventRequestType requestType = MInputMethod::EventRequestSignalOnly;
    if (((inputMethodMode == M::InputMethodModeDirect)
         && (event.specialKey() == KeyEvent::NotSpecial))
        || (event.qtKey() == Qt::Key_plusminus)) { // plusminus key makes an exception

        requestType = MInputMethod::EventRequestBoth;
        inputMethodHost()->sendKeyEvent(event.toQKeyEvent(), requestType);

    } else if (event.qtKey() == Qt::Key_Backspace) {
        if (correctionHost->isActive()
            && correctionHost->candidateMode() == MImCorrectionHost::WordTrackerMode) {
            // hide word tracker when backspace key press
            correctionHost->hideCorrectionWidget();
            // WordTrackerBackspaceMode mode: hide word tracker when backspace key press.
            // And remove preedit if holding backspace long enough. But does nothing
            // for backspace key release.
            startBackspace(WordTrackerBackspaceMode);
        } else {
            startBackspace(NormalBackspaceMode);
        }
    }
}

void MKeyboardHost::handleKeyRelease(const KeyEvent &event)
{
    if (event.qtKey() == Qt::Key_Shift) {
        if (!shiftHeldDown) {
            return; //ignore duplicated event
        }

        if (activeState == MInputMethod::OnScreen && enableMultiTouch) {
            shiftHeldDown = false;
        }
    }

    MInputMethod::EventRequestType requestType = MInputMethod::EventRequestSignalOnly;
    if (((inputMethodMode == M::InputMethodModeDirect)
         && (event.specialKey() == KeyEvent::NotSpecial))
        || (event.qtKey() == Qt::Key_plusminus)) { // plusminus key makes an exception

        requestType = MInputMethod::EventRequestBoth;
        inputMethodHost()->sendKeyEvent(event.toQKeyEvent(), requestType);

    } else if ((event.qtKey() == Qt::Key_Backspace)) {
        if ( backspaceTimer.isActive()) {
            backspaceTimer.stop();
            // If the backspace Mode is WordTrackerBackspaceMode, don't need to
            // do backspace.
            if (backspaceMode != WordTrackerBackspaceMode) {
                doBackspace();
            }
            backspaceMode = NormalBackspaceMode;
        }
    }
}

void MKeyboardHost::updateReactionMaps()
{
    if (rotationTimer.isActive()) {
        return;
    }

    // Start by making everything transparent
    clearReactionMaps(MReactionMap::Transparent);

    QList<QGraphicsView *> views = MPlainWindow::instance()->scene()->views();
    foreach(QGraphicsView *view, views) {
        MReactionMap *reactionMap = MReactionMap::instance(view);

        if (!reactionMap) {
            continue;
        }

        // Candidates widget
        if (correctionHost && correctionHost->isActive()) {
            correctionHost->paintReactionMap(reactionMap, view);

            // Correction candidate widget occupies whole screen when it is WordListMode.
            if (correctionHost->candidateMode() == MImCorrectionHost::WordListMode)
                continue;
        }

        // Paint either symview or vkb widget reactive areas.
        if (symbolView && symbolView->isVisible()) {
            symbolView->paintReactionMap(reactionMap, view);
        } else if (vkbWidget && vkbWidget->isFullyVisible()) {
            vkbWidget->paintReactionMap(reactionMap, view);
        }

        // Toolbar
        if (imToolbar && imToolbar->isVisible()) {
            imToolbar->paintReactionMap(reactionMap, view);
        }
    }
}

void MKeyboardHost::clearReactionMaps(const QString &clearValue)
{
    if (!MPlainWindow::instance()->scene()) {
        return;
    }

    foreach (QGraphicsView *view, MPlainWindow::instance()->scene()->views()) {
        MReactionMap *reactionMap = MReactionMap::instance(view);
        if (reactionMap) {
            reactionMap->setDrawingValue(clearValue, clearValue);
            reactionMap->setTransform(QTransform()); // Identity
            reactionMap->fillRectangle(0, 0, reactionMap->width(), reactionMap->height());
        }
    }
}

void MKeyboardHost::handleKeyClick(const KeyEvent &event)
{
    // Don't need send key events for Direct input mode here.
    // already send in handleKeyPress and handleKeyRelease.
    if (activeState == MInputMethod::Hardware && inputMethodMode != M::InputMethodModeDirect) {
        // In hardware keyboard mode symbol view is just another source for
        // events that will be handled by duihardwarekeyboard.  The native
        // modifiers may not be correct (depending on the current hwkbd modifier
        // state) but that doesn't matter.
        processKeyEvent(QEvent::KeyPress, event.qtKey(), event.modifiers(),
                        event.text(), false, 1, 0, 0);
        processKeyEvent(QEvent::KeyRelease, event.qtKey(), event.modifiers(),
                        event.text(), false, 1, 0, 0);
    } else if ((inputMethodMode != M::InputMethodModeDirect)) {
        handleTextInputKeyClick(event);
    }

    handleGeneralKeyClick(event);

    lastClickEvent = event;
}

void MKeyboardHost::handleGeneralKeyClick(const KeyEvent &event)
{
    if (event.qtKey() == Qt::Key_Shift) {
        switch (vkbWidget->shiftStatus()) {
        case ModifierLatchedState:
            // If current ShiftOn state is due to autocaps, go back to ShiftOff.
            // Otherwise, lock it.
            if (autoCapsTriggered) {
                vkbWidget->setShiftState(ModifierClearState);
            } else {
                vkbWidget->setShiftState(ModifierLockedState);
            }
            break;
        case ModifierClearState:
            vkbWidget->setShiftState(ModifierLatchedState);
            break;
        case ModifierLockedState:
            vkbWidget->setShiftState(ModifierClearState);
            break;
        }
        autoCapsTriggered = false;
    } else if (vkbWidget->shiftStatus() == ModifierLatchedState
               && (event.qtKey() == Qt::Key_Return)
               && (!shiftHeldDown || autoCapsTriggered)) {
        // Enter will toggle latched shift off only when
        // autoCaps is not enabled.
        if (!autoCapsEnabled) {
            vkbWidget->setShiftState(ModifierClearState);
        }
    } else if (vkbWidget->shiftStatus() == ModifierLatchedState
               && (event.qtKey() == Qt::Key_Space)
               && (!shiftHeldDown || autoCapsTriggered)) {
        // Space will toggle latched shift off except if:
        // - text is selected (since we don't know what is
        //   selected we can't predict the next state)
        // - next cursor position will have autoCaps
        if (!autoCapsEnabled
            || (!hasSelection
                && cursorPos <= surroundingText.length()
                && !surroundingText.left(cursorPos).contains(AutoCapsTrigger))) {
            vkbWidget->setShiftState(ModifierClearState);
        }
    } else if (vkbWidget->shiftStatus() == ModifierLatchedState
               && (event.qtKey() != Qt::Key_Backspace)
               && (event.specialKey() != KeyEvent::Sym)
               && (event.specialKey() != KeyEvent::Switch)
               && (event.specialKey() != KeyEvent::LayoutMenu)
               && (!shiftHeldDown || autoCapsTriggered)) {
        // Any key except shift toggles shift off if it's on (not locked).
        // Exceptions are:
        // - backspace, toggles shift off is handled in doBackspace()
        // - sym, pressing sym key keeps current shift state
        // - switch, pressing switch key keeps current shift state
        // - menu, pressing menu key keeps current shift state
        // - shift, when held down don't bring level down, except with autocaps!
        //   note: For this we cannot use event.modifiers().testFlag(Qt::ShiftModifier)
        //         because it does not differentiate between latched+char and held down + char.
        vkbWidget->setShiftState(ModifierClearState);
    }

    if (event.specialKey() == KeyEvent::LayoutMenu) {
        showLayoutMenu();
    } else if (event.specialKey() == KeyEvent::Sym) {
        handleSymbolKeyClick();
    } else if (event.specialKey() == KeyEvent::Switch) {
        if (symbolView->isActive()) {
            symbolView->switchToNextPage();
        }
    }
}

void MKeyboardHost::handleLongKeyPress(const KeyEvent &event)
{
    if (event.qtKey() == Qt::Key_Space
        && correctionEnabled
        && correctionHost->isActive()
        && correctionHost->candidateMode() == MImCorrectionHost::WordTrackerMode
        && candidates.size() > 0) {
        // long tap space key when word tracker is visible will switch to word list.
        correctionHost->showCorrectionWidget(MImCorrectionHost::WordListMode);
    }
}

void MKeyboardHost::sendCommitStringOrReturnEvent(const KeyEvent &event) const
{
    // We send return as a normal key event instead of an input method event so
    // that it properly triggers returnPressed signal emission in MTextEdit and
    // QLineEdit (and possibly in other similar targets).
    if (event.qtKey() == Qt::Key_Return) {
        inputMethodHost()->sendKeyEvent(KeyEvent(event, QEvent::KeyPress).toQKeyEvent(), MInputMethod::EventRequestEventOnly);
        inputMethodHost()->sendKeyEvent(event.toQKeyEvent(), MInputMethod::EventRequestEventOnly);
    } else {
        inputMethodHost()->sendCommitString(event.text());
    }
}

void MKeyboardHost::handleTextInputKeyClick(const KeyEvent &event)
{
    const bool wordTrackerSuggestionAcceptedWithSpacePrev(wordTrackerSuggestionAcceptedWithSpace);
    if (!((event.specialKey() == KeyEvent::Sym)
          || (event.specialKey() == KeyEvent::Switch)
          || (event.specialKey() == KeyEvent::Sym)
          || (event.qtKey() == Qt::Key_Shift))) {
        wordTrackerSuggestionAcceptedWithSpace = false;
    }

    // Discard KeyPress & Drop type of events.
    if (event.type() != QEvent::KeyRelease

        // Discard also special keys except cycleset which has text
        // that can be sent to input context.
        || (!(event.specialKey() == KeyEvent::NotSpecial
              || event.specialKey() == KeyEvent::CycleSet))

        // Finally, discard Qt backspace and plusminus key, which are handled in
        // handleKeyPress/Release.
        || (event.qtKey() == Qt::Key_Backspace)
        || (event.qtKey() == Qt::Key_plusminus)
        || (event.qtKey() == Qt::Key_Shift)) {

        return;
    }

    if (cycleKeyHandler->handleTextInputKeyClick(event)) {
        return;
    }
    QString text(event.text());

    if (text.isEmpty()) {
        return;
    }

    if (!correctionEnabled) {
        if (preedit.length() > 0) {
            // we just entered accurate mode. send the previous preedit stuff.
            inputMethodHost()->sendCommitString(preedit);
            if (imCorrectionEngine)
                imCorrectionEngine->clearEngineBuffer();
            preedit.clear();
            preeditCursorPos = -1;
        }

        sendCommitStringOrReturnEvent(event);

    } else if ((event.qtKey() == Qt::Key_Space) || (event.qtKey() == Qt::Key_Return) || (event.qtKey() == Qt::Key_Tab)) {
        // commit suggestion if correction candidate widget is visible and with popupMode
        // or ignore it if correction widget is visible and with suggestionlist mode
        // otherwise commit preedit
        if (event.qtKey() == Qt::Key_Space
            && correctionHost->isActive()) {
            if (correctionHost->candidateMode() == MImCorrectionHost::WordTrackerMode) {
                wordTrackerSuggestionAcceptedWithSpace = true;
                inputMethodHost()->sendCommitString(correctionHost->suggestion());
            } else {
                // ignore space click when word list is visible.
                return;
            }
        } else {
            if (!preedit.isEmpty()) {
                // if cursor is inside preedit. need commit preedit
                // and reposition cursor before it, then insert space/return
                // also need check whether widget supports the surrounding text query.
                const bool needRepositionCursor = preeditCursorPos != -1
                                                  && preeditCursorPos != preedit.length()
                                                  && inputMethodHost()->surroundingText(surroundingText, cursorPos)
                                                  && (cursorPos >= 0);
                inputMethodHost()->sendCommitString(preedit);
                if (needRepositionCursor) {
                    inputMethodHost()->setSelection(cursorPos + preeditCursorPos, 0);
                }
            }
        }


        if (lastClickEvent.specialKey() != KeyEvent::CycleSet) {
            candidates.clear();
            correctionHost->reset();
        }
        // Send trailing space.
        sendCommitStringOrReturnEvent(event);

        imCorrectionEngine->clearEngineBuffer();
        preedit.clear();
        preeditCursorPos = -1;
    } else if (wordTrackerSuggestionAcceptedWithSpacePrev && (text.length() == 1)
               && AutoPunctuationTriggers.contains(text[0])) {
        sendBackSpaceKeyEvent();
        resetInternalState();
        inputMethodHost()->sendCommitString(text + " ");
    } else {
        candidates.clear();
        // append text to the end of preedit if cursor is at the end of
        // preedit (or cursor is -1, invisible). Or if cursor is inside
        // preedit, insert text after cursor.
        if (preeditCursorPos < 0 || preeditCursorPos == preedit.length()) {
            if (preeditCursorPos < 0) {
                if (preedit.isEmpty()) {
                    preeditCursorPos = text.length();
                }
            } else {
                preeditCursorPos += text.length();
            }
            preedit += text;
            // send touch point to engine if not in symbol view
            // otherwise send character to engine.
            if (!symbolView->isActive())
                imCorrectionEngine->tapKeyboard(event.pos(),
                                                vkbWidget->shiftStatus() != ModifierClearState, text.at(0));
            else
                imCorrectionEngine->appendCharacter(text.at(0));
        } else {
            preedit.insert(preeditCursorPos, text);
            preeditCursorPos += text.length();
            imCorrectionEngine->clearEngineBuffer();
            imCorrectionEngine->reselectString(preedit);
        }
        candidates = imCorrectionEngine->candidates();
        correctionHost->setCandidates(candidates);

        // update preedit before showing word tracker
        updatePreedit(preedit, candidates.count(), 0, 0, preeditCursorPos);

        if (preeditCursorPos < 0 || preeditCursorPos == preedit.length()) {
            MImEngine::DictionaryType sourceDictionaryType = imCorrectionEngine->candidateSource(0);
            // because the written word (preedit) should be on the top of candidate list.
            // So when there are some other candidates and the preedit is not a valid
            // dictionary word, show word tracker.
            if (candidates.count() > 1
                && sourceDictionaryType == MImEngine::DictionaryTypeInvalid) {
                bool success = false;
                const QRect rect = inputMethodHost()->cursorRectangle(success);
                if (success && rect.isValid()) {
                    correctionHost->setPosition(sceneWindow->mapRectFromScene(rect).toRect());
                    correctionHost->showCorrectionWidget(MImCorrectionHost::WordTrackerMode);
                }
            } else {
                correctionHost->hideCorrectionWidget();
            }
        } else if (correctionHost->isActive()) {
            correctionHost->hideCorrectionWidget();
        }
    }
}

void MKeyboardHost::initializeInputEngine()
{
    // init correction engine
    // set language according current displayed language in virtual keyboard
    QString language = vkbWidget->layoutLanguage();

    if (language.isEmpty())
        language = DefaultInputLanguage;

    qDebug() << __PRETTY_FUNCTION__ << "- used language:" << language;

    if (imCorrectionEngine) {
        // TODO: maybe we should check return values here and in case of failure
        // be always in accurate mode, for example
        imCorrectionEngine->setLanguage(language, MImEngine::LanguagePriorityPrimary);
        engineLayoutDirty = true;
        updateEngineKeyboardLayout();
        synchronizeCorrectionSetting();
        imCorrectionEngine->disablePrediction();
        imCorrectionEngine->setMaximumCandidates(MaximumErrorCorrectionCandidate);
        imCorrectionEngine->setExactWordPositionInList(MImEngine::ExactInListFirst);
    }
}

void MKeyboardHost::synchronizeCorrectionSetting()
{
    bool correction = inputMethodCorrectionSettings->value(DefaultCorrectionSettingOption).toBool();

    if (!correction) {
        imCorrectionEngine->disableCorrection();
        imCorrectionEngine->disableCompletion();
    } else {
        imCorrectionEngine->enableCorrection();
        imCorrectionEngine->enableCompletion();
    }

    updateCorrectionState();
}


void MKeyboardHost::updateCorrectionState()
{
    if (activeState == MInputMethod::Hardware) {
        inputMethodHost()->setGlobalCorrectionEnabled(false);
        correctionEnabled = false;
    } else {
        if (!imCorrectionEngine) {
            inputMethodHost()->setGlobalCorrectionEnabled(false);
            correctionEnabled = false;
            return;
        }
        bool val = false;
        bool enabled = inputMethodHost()->correctionEnabled(val);
        if (val)
            correctionEnabled = enabled && imCorrectionEngine->correctionEnabled()
                                && imCorrectionEngine->completionEnabled();
        else
            correctionEnabled = imCorrectionEngine->correctionEnabled()
                                && imCorrectionEngine->completionEnabled();

        // info context the global correction option
        // TODO: should not put setGlobalCorrectionEnabled here, it will send correction setting
        // whenever focus changes. But have to at this moment, because im-uiserver start before
        // application, and there is no focus widget, no activateContext, calling
        // setGlobalCorrectionEnabled() at that time, can not record the setting.
        // Only after the application is running, this setGlobalCorrectionEnabled() can take effect
        inputMethodHost()->setGlobalCorrectionEnabled(imCorrectionEngine->correctionEnabled());
    }
}


void MKeyboardHost::userHide()
{
    if (!preedit.isEmpty()) {
        sendString(preedit);
    }
    hide();
    inputMethodHost()->notifyImInitiatedHiding();
    resetInternalState();
}


void MKeyboardHost::sendCopyPaste(CopyPasteState action)
{
    switch (action) {
    case InputMethodCopy:
        inputMethodHost()->copy();
        break;
    case InputMethodPaste:
        inputMethodHost()->paste();
        break;
    default:
        qDebug() << __PRETTY_FUNCTION__ << "invalid action" << action;
        break;
    }
}

void MKeyboardHost::showLayoutMenu()
{
    inputMethodHost()->showSettings();
}

void MKeyboardHost::switchPlugin(MInputMethod::SwitchDirection direction)
{
    inputMethodHost()->switchPlugin(direction);
}

void MKeyboardHost::sendKeyEvent(const QKeyEvent &key)
{
    inputMethodHost()->sendKeyEvent(key);
}

void MKeyboardHost::sendString(const QString &text)
{
    inputMethodHost()->sendCommitString(text);
}

void MKeyboardHost::sendStringFromToolbar(const QString &text)
{
    if (!preedit.isEmpty()) {
        sendString(preedit);
    }
    reset();
    sendString(text);
}

void MKeyboardHost::setToolbar(QSharedPointer<const MToolbarData> toolbar)
{
    if (toolbar && toolbar->isVisible()) {
        imToolbar->showToolbarWidget(toolbar);
    } else {
        imToolbar->hideToolbarWidget();
    }
}

void MKeyboardHost::processKeyEvent(QEvent::Type keyType, Qt::Key keyCode,
                                    Qt::KeyboardModifiers modifiers, const QString &text,
                                    bool autoRepeat, int count, quint32 nativeScanCode,
                                    quint32 nativeModifiers)
{
    if ((activeState != MInputMethod::Hardware) ||
        !hardwareKeyboard->filterKeyEvent(keyType, keyCode, modifiers, text,
                                          autoRepeat, count, nativeScanCode,
                                          nativeModifiers)) {
        inputMethodHost()->sendKeyEvent(QKeyEvent(keyType, keyCode, modifiers, text,
                                                  autoRepeat, count),
                                        MInputMethod::EventRequestEventOnly);
    }
}

void MKeyboardHost::handleClientChange()
{
    hardwareKeyboard->clientChanged();
    resetInternalState();
    hide(); // could do some quick hide also
}

void MKeyboardHost::switchContext(MInputMethod::SwitchDirection direction, bool enableAnimation)
{
    if (activeState == MInputMethod::OnScreen) {
        vkbWidget->switchLayout(direction, enableAnimation);
    }
}

void MKeyboardHost::setState(const QSet<MInputMethod::HandlerState> &state)
{
    if (state.isEmpty()) {
        return;
    }

    const MInputMethod::HandlerState actualState = *state.begin();
    if (activeState == actualState)
        return;

    if ((activeState == MInputMethod::OnScreen) && (preedit.length() > 0)) {
        inputMethodHost()->sendCommitString(preedit);
    }

    // Resets before changing the activeState to make sure clear.
    resetInternalState();
    activeState = actualState;

    // Keeps separate states for symbol view in OnScreen state and Hardware state
    if (activeState == MInputMethod::OnScreen) {
        hideLockOnInfoBanner();
        inputMethodHost()->setInputModeIndicator(MInputMethod::NoIndicator);

        disconnect(hardwareKeyboard, SIGNAL(deadKeyStateChanged(const QChar &)),
                   this, SLOT(handleHwKeyboardStateChanged()));
        disconnect(hardwareKeyboard, SIGNAL(modifiersStateChanged()),
                   this, SLOT(handleHwKeyboardStateChanged()));
        disconnect(hardwareKeyboard, SIGNAL(scriptChanged()),
                   this, SLOT(handleHwKeyboardStateChanged()));
        if (haveFocus) {
            hardwareKeyboard->disable();
        }
        if (sipRequested) {
            vkbWidget->showKeyboard();
        }
    } else {
        currentIndicatorDeadKey = false;
        connect(hardwareKeyboard, SIGNAL(deadKeyStateChanged(const QChar &)),
                this, SLOT(handleHwKeyboardStateChanged()));
        connect(hardwareKeyboard, SIGNAL(modifiersStateChanged()),
                this, SLOT(handleHwKeyboardStateChanged()));
        connect(hardwareKeyboard, SIGNAL(scriptChanged()),
                this, SLOT(handleHwKeyboardStateChanged()));
        if (haveFocus) {
            hardwareKeyboard->enable();
        }
        vkbWidget->hideKeyboard();
    }

    // Hide symbol view before changing the state.
    // TODO: Wait until transition animation has been finished
    // before changing state when re-enabling animation for
    // symbolview hiding.
    symbolView->hideSymbolView();

    symbolView->setKeyboardState(actualState);
    updateCorrectionState();
    updateAutoCapitalization();
}

void MKeyboardHost::handleSymbolKeyClick()
{
    if (((activeState == MInputMethod::Hardware) && !hardwareKeyboard->symViewAvailable())
        || !vkbWidget->symViewAvailable()) {
        return;
    }

    // TODO: make RegionTracker do this kind of optimization automatically
    const bool wasEnabled(RegionTracker::instance().enableSignals(false));

    // Toggle SymbolView.
    if (!symbolView->isVisible()) {
        symbolView->setPos(0, MPlainWindow::instance()->visibleSceneSize().height() - symbolView->size().height());
        symbolView->showSymbolView();
        //give the symbolview right shift level(for hardware state)
        updateSymbolViewLevel();
    } else {
        symbolView->hideSymbolView();
    }

    RegionTracker::instance().enableSignals(wasEnabled);
}

void MKeyboardHost::updateSymbolViewLevel()
{
    if (!symbolView->isActive())
        return;

    ModifierState shiftState = ModifierClearState;
    if (activeState == MInputMethod::OnScreen) {
        shiftState = vkbWidget->shiftStatus();
    } else {
        shiftState = hardwareKeyboard->modifierState(Qt::ShiftModifier);
    }
    symbolView->setShiftState(shiftState);
}

void MKeyboardHost::showSymbolView()
{
    // TODO: make RegionTracker do this kind of optimization automatically
    const bool wasEnabled(RegionTracker::instance().enableSignals(false));
    symbolView->setPos(0, MPlainWindow::instance()->visibleSceneSize().height() - symbolView->size().height());
    symbolView->showSymbolView(SymbolView::FollowMouseShowMode);
    //give the symbolview right shift level(for hardware state)
    updateSymbolViewLevel();
    RegionTracker::instance().enableSignals(wasEnabled);
}

MInputMethod::InputModeIndicator MKeyboardHost::deadKeyToIndicator(const QChar &key)
{
    switch (key.unicode()) {
    case 0x00b4:
        return MInputMethod::DeadKeyAcuteIndicator;
    case 0x02c7:
        return MInputMethod::DeadKeyCaronIndicator;
    case 0x005e:
        return MInputMethod::DeadKeyCircumflexIndicator;
    case 0x00a8:
        return MInputMethod::DeadKeyDiaeresisIndicator;
    case 0x0060:
        return MInputMethod::DeadKeyGraveIndicator;
    case 0x007e:
        return MInputMethod::DeadKeyTildeIndicator;
    default:
        return MInputMethod::NoIndicator;
    }
}

void MKeyboardHost::handleHwKeyboardStateChanged()
{
    // only change indicator state when there is focus is in a widget and state is Hardware.
    if (!haveFocus || activeState != MInputMethod::Hardware)
        return;

    const ModifierState shiftState = hardwareKeyboard->modifierState(Qt::ShiftModifier);
    const ModifierState fnState = hardwareKeyboard->modifierState(FnLevelModifier);
    const QString xkbLayout = LayoutsManager::instance().xkbLayout();
    const QString xkbVariant = LayoutsManager::instance().xkbVariant();

    const bool previousIndicatorDeadKey(currentIndicatorDeadKey);

    MInputMethod::InputModeIndicator indicatorState
        = deadKeyToIndicator(hardwareKeyboard->deadKeyState());

    currentIndicatorDeadKey = false;

    if (indicatorState != MInputMethod::NoIndicator) {
        currentIndicatorDeadKey = true;
    } else if (fnState == ModifierLockedState) {
        indicatorState = MInputMethod::NumAndSymLockedIndicator;

    } else if (fnState == ModifierLatchedState) {
        indicatorState = MInputMethod::NumAndSymLatchedIndicator;

    } else if (xkbLayout == "ara"
        && xkbVariant.isEmpty()) {
        indicatorState = MInputMethod::ArabicIndicator;

    } else if (xkbVariant.isEmpty() || xkbVariant == "latin") {
        indicatorState = MInputMethod::LatinLowerIndicator;
        if (shiftState == ModifierLockedState) {
            indicatorState = MInputMethod::LatinLockedIndicator;
        } else if (shiftState == ModifierLatchedState) {
            indicatorState = MInputMethod::LatinUpperIndicator;
        }

    } else if (xkbVariant == "cyrillic") {
        indicatorState = MInputMethod::CyrillicLowerIndicator;
        if (shiftState == ModifierLockedState) {
            indicatorState = MInputMethod::CyrillicLockedIndicator;
        } else if (shiftState == ModifierLatchedState) {
            indicatorState = MInputMethod::CyrillicUpperIndicator;
        }
    }

    inputMethodHost()->setInputModeIndicator(indicatorState);

    QString lockOnNotificationLabel;

    if (indicatorState == MInputMethod::LatinLockedIndicator
        || indicatorState == MInputMethod::CyrillicLockedIndicator) {
        //% "Caps lock on"
        lockOnNotificationLabel = qtTrId("qtn_hwkb_caps_lock");
    } else if (indicatorState == MInputMethod::NumAndSymLockedIndicator) {
        //% "Symbol lock on"
        lockOnNotificationLabel = qtTrId("qtn_hwkb_fn_lock");
    }
    if (!lockOnNotificationLabel.isEmpty()
        && !previousIndicatorDeadKey
        && hardwareKeyboard->keyboardType() != M::NumberContentType
        && hardwareKeyboard->keyboardType() != M::PhoneNumberContentType) {
        // notify the modifier is changed to locked state
        // number and phone number content type always force FN key to be locked,
        // don't need indicator lock notification.
        showLockOnInfoBanner(lockOnNotificationLabel);
    } else if (modifierLockOnBanner) {
        hideLockOnInfoBanner();
    }
}

void MKeyboardHost::handleVirtualKeyboardCapsLock()
{
    if (activeState != MInputMethod::OnScreen)
        return;

    if (vkbWidget->shiftStatus() == ModifierLockedState) {
        //% "Caps lock on"
        QString lockOnNotificationLabel = qtTrId("qtn_hwkb_caps_lock");
        showLockOnInfoBanner(lockOnNotificationLabel);
    } else if (modifierLockOnBanner) {
        hideLockOnInfoBanner();
    }
}

void MKeyboardHost::showLockOnInfoBanner(const QString &notification)
{
    if (modifierLockOnBanner) {
        modifierLockOnBanner->setTitle(notification);
    } else {
        //TODO: discuss with UI designer whether we need to specify
        // the disappear time out.
        modifierLockOnBanner = new MBanner;
        // It is needed to set the proper style name to have properly wrapped, multiple lines
        // with too much content. The MBanner documentation also emphasises to specify the
        // style name for the banners explicitly in the code.
        modifierLockOnBanner->setStyleName("InformationBanner");
        modifierLockOnBanner->setObjectName(NotificationObjectName);
        modifierLockOnBanner->setTitle(notification);
        modifierLockOnBanner->appear(MSceneWindow::DestroyWhenDone);
    }
}

void MKeyboardHost::hideLockOnInfoBanner()
{
    if (modifierLockOnBanner) {
        modifierLockOnBanner->disappear();
    }
    modifierLockOnBanner = 0;
}

QList<MAbstractInputMethod::MInputMethodSubView>
MKeyboardHost::subViews(MInputMethod::HandlerState state) const
{
    QList<MAbstractInputMethod::MInputMethodSubView> sViews;
    if (state == MInputMethod::OnScreen) {
        QMap<QString, QString> selectedLayouts = LayoutsManager::instance().selectedLayouts();
        QMap<QString, QString>::const_iterator i = selectedLayouts.constBegin();
        while (i != selectedLayouts.constEnd()) {
            MAbstractInputMethod::MInputMethodSubView subView;
            subView.subViewId = i.key();
            subView.subViewTitle = i.value();
            sViews.append(subView);
            ++i;
        }
    }
    return sViews;
}

void MKeyboardHost::setActiveSubView(const QString &subViewId, MInputMethod::HandlerState state)
{
    if (state == MInputMethod::OnScreen) {
        const QStringList layoutFileList = LayoutsManager::instance().layoutFileList();
        int index = layoutFileList.indexOf(subViewId);
        vkbWidget->setLayout(index);
    }
}

QString MKeyboardHost::activeSubView(MInputMethod::HandlerState state) const
{
    if (state == MInputMethod::OnScreen) {
        // return the active vkb layout
        return vkbWidget->selectedLayout();
    } else {
        return QString();
    }
}

void MKeyboardHost::showLanguageNotification()
{
    if (activeState == MInputMethod::OnScreen && vkbWidget) {
        vkbWidget->requestLanguageNotification();
    }
}

void MKeyboardHost::handleVirtualKeyboardLayoutChanged(const QString &layout)
{
    // reset the temporary shift state when layout is changed
    resetVirtualKeyboardShiftState();
    if (symbolView) {
        symbolView->setLayout(layout);
    }

    initializeInputEngine();
    updateAutoCapitalization();
    emit activeSubViewChanged(layout);
}

void MKeyboardHost::updateEngineKeyboardLayout()
{
    if (!imCorrectionEngine || !engineLayoutDirty)
        return;

    if (activeState == MInputMethod::OnScreen) {
        imCorrectionEngine->setKeyboardLayoutKeys(vkbWidget->mainLayoutKeys());
    }
    engineLayoutDirty = false;
}

void MKeyboardHost::updatePreedit(const QString &string, int candidateCount, int replaceStart,
                                  int replaceLength, int cursor)
{
    // preedit style type depends on candidateCount and cursor.
    // candidateCount     styleType             cursor
    //  0 or 1            PreeditNoCandidates   at tail or < 0
    //  >1                PreeditDefault        at tail or < 0
    //  any               PreeditDefault        in the middle of preedit
    MInputMethod::PreeditFace face = MInputMethod::PreeditDefault;
    if (candidateCount <= 1) {
        face = MInputMethod::PreeditNoCandidates;
    }

    QList<MInputMethod::PreeditTextFormat> preeditFormats;
    MInputMethod::PreeditTextFormat preeditFormat(0, string.length(), face);
    preeditFormats << preeditFormat;
    inputMethodHost()->sendPreeditString(string, preeditFormats, replaceStart, replaceLength, cursor);
}

void MKeyboardHost::startBackspace(MKeyboardHost::BackspaceMode mode)
{
    backspaceMode = mode;
    backspaceTimer.start(AutoBackspaceDelay);
}

void MKeyboardHost::updateCorrectionWidgetPosition()
{
    if (correctionEnabled && correctionHost->isActive()
        && correctionHost->candidateMode() == MImCorrectionHost::WordTrackerMode) {
        bool success = false;
        const QRect rect = inputMethodHost()->cursorRectangle(success);
        if (success && rect.isValid()) {
            correctionHost->setPosition(sceneWindow->mapRectFromScene(rect).toRect());
        }
    }
}

void MKeyboardHost::sendBackSpaceKeyEvent() const
{
    const KeyEvent event("\b", QEvent::KeyPress, Qt::Key_Backspace,
                         KeyEvent::NotSpecial,
                         vkbWidget->shiftStatus() != ModifierClearState
                         ? Qt::ShiftModifier : Qt::NoModifier);
    inputMethodHost()->sendKeyEvent(event.toQKeyEvent(),
                                    MInputMethod::EventRequestEventOnly);
}
