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
#include "mvirtualkeyboard.h"
#include "mhardwarekeyboard.h"
#ifdef M_IM_DISABLE_TRANSLUCENCY
#include "mimcorrectioncandidatewindow.h"
#endif
#include "mimcorrectioncandidatewidget.h"
#include "keyboarddata.h"
#include "layoutmenu.h"
#include "layoutsmanager.h"
#include "symbolview.h"
#include "toolbarmanager.h"

#include <duiimenginewords.h>
#include <minputcontextconnection.h>
#include <mplainwindow.h>
#include <mgconfitem.h>
#include <mtheme.h>

#include <QDebug>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QFile>
#include <QRegExp>

#include <MApplication>
#include <MComponentData>
#include <MFeedbackPlayer>
#include <mreactionmap.h>
#include <MScene>
#include <MSceneManager>
#include <MSceneWindow>
#include <MInfoBanner>
#include <MLibrary>
M_LIBRARY // to avoid a crash from mtheme. FIXME - is this the proper way for us?


namespace
{
    const QString InputMethodList("MInputMethodList");
    const QString DefaultInputLanguage("en_GB");
    // TODO: check that these paths still hold
    const QString InputMethodCorrectionSetting("/meegotouch/inputmethods/correctionenabled");
    const QString InputMethodCorrectionEngine("/meegotouch/inputmethods/correctionengine");
    const QString AutoCapsSentenceDelimiters(".?!¡¿"); // used as regexp character set content!
    const int RotationDuration = 750; //! After vkb hidden, how long to wait until shown again
    const int AutoBackspaceDelay = 500;      // in ms
    const int BackspaceRepeatInterval = 100; // in ms
    const int MultitapTime = 1500;           // in ms
    const char * const PixmapDirectory = "/usr/share/meegotouch/virtual-keyboard/images";
    const QString CssFile("/usr/share/meegotouch/virtual-keyboard/css/%1x%2.css");
    const QString DefaultCss = CssFile.arg(864).arg(480); // Default screen resolution is 864x480
    const Qt::KeyboardModifier FnLevelModifier = Qt::GroupSwitchModifier;
    const int ModifierLockOnInfoDuration = 1000;    // in ms
}


MKeyboardHost::MKeyboardHost(MInputContextConnection* icConnection, QObject *parent)
    : MInputMethodBase(icConnection, parent),
      vkbWidget(0),
      symbolView(0),
      inputMethodCorrectionSettings(new MGConfItem(InputMethodCorrectionSetting)),
      inputMethodCorrectionEngine(new MGConfItem(InputMethodCorrectionEngine)),
      engineReady(false),
      angle(M::Angle0),
      rotationInProgress(false),
      correctionEnabled(false),
      feedbackPlayer(0),
      autoCapsEnabled(true),
      cursorPos(-1),
      inputMethodMode(M::InputMethodModeNormal),
      backSpaceTimer(this),
      multitapIndex(0),
      activeState(OnScreen),
      modifierLockOnInfoBanner(0),
      modifierLockOnTimer(this),
      haveFocus(false)
{
    displayHeight = MPlainWindow::instance()->visibleSceneSize(M::Landscape).height();
    displayWidth  = MPlainWindow::instance()->visibleSceneSize(M::Landscape).width();

    //TODO get this from settings
    MTheme *theme = MTheme::instance();
    theme->addPixmapDirectory(PixmapDirectory);

    QString css = CssFile.arg(displayWidth, displayHeight);

    if (!QFile::exists(css)) {
        css = DefaultCss;
    }

    theme->loadCSS(css);

    LayoutsManager::createInstance();
    ToolbarManager::createInstance();

    sceneWindow = new MSceneWindow;
    sceneWindow->setManagedManually(true); // we want the scene window to remain in origin

    // This will add scene window as child of MSceneManager's root element
    // which is the QGraphicsItem that is rotated when orientation changes.
    // It uses animation to carry out the orientation change transform
    // (e.g. rotation and position animation). We do this because transform
    // happens in the scene, not in the view (MWindow) anymore.
    MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(sceneWindow);

    // Because we set vkbWidget as a child of sceneWindow the vkb
    // will always be in correct orientation. However the animation will be
    // affected as well. If we want to keep the current hiding/showing animation
    // (up & down) without getting it combined with the rotation animation
    // we have at least two options:
    // 1) Make our own MOrientationAnimation when libmeegotouch begins supporting
    //    setting it, through theme probably.
    // 2) Add widgets directly to scene (detached from MSceneManager) and
    //    update their transformations by hand.

    vkbWidget = new MVirtualKeyboard(LayoutsManager::instance(), sceneWindow);

    connect(vkbWidget, SIGNAL(keyClicked(const KeyEvent &)),
            this, SLOT(handleKeyClick(const KeyEvent &)));
    connect(vkbWidget, SIGNAL(keyPressed(const KeyEvent &)),
            this, SLOT(handleKeyPress(const KeyEvent &)));
    connect(vkbWidget, SIGNAL(keyReleased(const KeyEvent &)),
            this, SLOT(handleKeyRelease(const KeyEvent &)));
    connect(vkbWidget, SIGNAL(showSymbolViewRequested()),
            this, SLOT(showSymbolView()));

    connect(vkbWidget, SIGNAL(regionUpdated(const QRegion &)),
            this, SLOT(handleRegionUpdate(const QRegion &)));
    connect(vkbWidget, SIGNAL(regionUpdated(const QRegion &)),
            this, SLOT(handleInputMethodAreaUpdate(const QRegion &)));

    connect(vkbWidget, SIGNAL(userInitiatedHide()),
            this, SLOT(userHide()));

    connect(vkbWidget, SIGNAL(pluginSwitchRequired(M::InputMethodSwitchDirection)),
            this, SIGNAL(pluginSwitchRequired(M::InputMethodSwitchDirection)));

    modifierLockOnTimer.setInterval(ModifierLockOnInfoDuration);
    connect(&modifierLockOnTimer, SIGNAL(timeout()), this, SLOT(hideLockOnInfoBanner()));

    // construct hardware keyboard object
    hardwareKeyboard = new MHardwareKeyboard(*icConnection, this);
    connect(hardwareKeyboard, SIGNAL(symbolKeyClicked()),
            this, SLOT(handleSymbolKeyClick()));

    bool ok = connect(vkbWidget, SIGNAL(copyPasteClicked(CopyPasteState)),
                      this, SLOT(sendCopyPaste(CopyPasteState)));
    Q_UNUSED(ok); // if Q_NO_DEBUG is defined then the assert won't be used
    Q_ASSERT(ok);

    createCorrectionCandidateWidget();

    // Ideally we would adjust the hiding/showing animation of vkb according to
    // animation of the application receiving input. For example, with 3-phase
    // MBasicOrientationAnimation we would probably want to sync like this:
    // 1) Navigation bar hiding (altough it's probably already hidden) -> vkb hiding
    // 2) Rotation in progress -> vkb not visible
    // 3) Navigation bar showing -> vkb showing
    connect(MPlainWindow::instance()->sceneManager(),
            SIGNAL(orientationAngleChanged(M::OrientationAngle)),
            SLOT(prepareOrientationChange()));

    // orientationChangeFinished is emitted on every angle change,
    // not only orientation change.
    connect(MPlainWindow::instance()->sceneManager(),
            SIGNAL(orientationChangeFinished(M::Orientation)),
            SLOT(finalizeOrientationChange()));

    symbolView = new SymbolView(LayoutsManager::instance(), &vkbWidget->style(),
                                vkbWidget->selectedLanguage(), sceneWindow);
    connect(symbolView, SIGNAL(regionUpdated(const QRegion &)),
            this, SLOT(handleRegionUpdate(const QRegion &)));
    connect(symbolView, SIGNAL(regionUpdated(const QRegion &)),
            this, SLOT(handleInputMethodAreaUpdate(const QRegion &)));

    connect(symbolView, SIGNAL(keyClicked(const KeyEvent &)),
            this, SLOT(handleKeyClick(const KeyEvent &)));
    connect(symbolView, SIGNAL(keyPressed(const KeyEvent &)),
            this, SLOT(handleKeyPress(const KeyEvent &)));
    connect(symbolView, SIGNAL(keyReleased(const KeyEvent &)),
            this, SLOT(handleKeyRelease(const KeyEvent &)));

    connect(vkbWidget, SIGNAL(languageChanged(const QString &)),
            symbolView, SLOT(setLanguage(const QString &)));

    connect(vkbWidget, SIGNAL(shiftLevelChanged()),
            this, SLOT(updateSymbolViewLevel()));

    connect(hardwareKeyboard, SIGNAL(shiftStateChanged()),
            this, SLOT(updateSymbolViewLevel()));

    // Construct layout menu dialog
    layoutMenu = new LayoutMenu(&vkbWidget->style(), 0);

    connect(layoutMenu, SIGNAL(errorCorrectionToggled(bool)),
            this, SLOT(errorCorrectionToggled(bool)));

    connect(layoutMenu, SIGNAL(regionUpdated(const QRegion &)),
            this, SLOT(handleRegionUpdate(const QRegion &)));

    connect(layoutMenu, SIGNAL(languageSelected(int)), vkbWidget, SLOT(setLanguage(int)));
    // FIXME: connect something to layoutMenu::organizeContent()?
    // layout menu done

    connect(vkbWidget, SIGNAL(copyPasteRequest(CopyPasteState)),
            this, SLOT(sendCopyPaste(CopyPasteState)));
    connect(vkbWidget, SIGNAL(sendKeyEventRequest(const QKeyEvent &)),
            this, SLOT(sendKeyEvent(const QKeyEvent &)));
    connect(vkbWidget, SIGNAL(sendStringRequest(const QString &)),
            this, SLOT(sendString(const QString &)));

    imCorrectionEngine = DuiImEngineWords::instance();


    if (!inputMethodCorrectionEngine->value().isNull()) {
        bool success = imCorrectionEngine->setDriver(inputMethodCorrectionEngine->value().toString());

        if (success == true) {
            engineReady = true;
            initializeInputEngine();
            connect(inputMethodCorrectionSettings, SIGNAL(valueChanged()),
                    this, SLOT(synchronizeCorrectionSetting()));

            connect(vkbWidget, SIGNAL(languageChanged(const QString &)),
                    this, SLOT(initializeInputEngine()));
        } else {
            qDebug() << __PRETTY_FUNCTION__ << "Failed to load correction engine"
                     << inputMethodCorrectionEngine->value().toString();
        }
    }

    feedbackPlayer = MComponentData::feedbackPlayer();

    backSpaceTimer.setSingleShot(true);
    connect(&backSpaceTimer, SIGNAL(timeout()), this, SLOT(autoBackspace()));

    // hide main layout when symbol view is shown to improve performance
    connect(symbolView, SIGNAL(opened()), vkbWidget, SLOT(hideMainArea()));
    connect(symbolView, SIGNAL(hidden()), vkbWidget, SLOT(showMainArea()));
}


MKeyboardHost::~MKeyboardHost()
{
    hideLockOnInfoBanner();
    delete hardwareKeyboard;
    hardwareKeyboard = 0;
    delete vkbWidget;
    vkbWidget = 0;
    delete correctionCandidateWidget;
    correctionCandidateWidget = 0;
    delete sceneWindow;
    sceneWindow = 0;
#ifdef M_IM_DISABLE_TRANSLUCENCY
    delete correctionSceneWindow;
    correctionSceneWindow = 0;
    delete correctionWindow;
    correctionWindow = 0;
#endif
    delete inputMethodCorrectionSettings;
    inputMethodCorrectionSettings = 0;
    //TODO imCorrectionEngine is not deleted. memory loss
    backSpaceTimer.stop();
    delete layoutMenu;
    LayoutsManager::destroyInstance();
    ToolbarManager::destroyInstance();
}

void MKeyboardHost::createCorrectionCandidateWidget()
{
#ifdef M_IM_DISABLE_TRANSLUCENCY
    // Use a separate translucent window for correction candidate widget
    correctionWindow = new MImCorrectionCandidateWindow();
    MWindow *correctionView = new MWindow(new MSceneManager, correctionWindow);
    // Enable translucent in hardware rendering
    correctionView->setTranslucentBackground(!MApplication::softwareRendering());

    // No auto fill in software rendering
    if (MApplication::softwareRendering())
        correctionView->viewport()->setAutoFillBackground(false);
    QSize sceneSize = correctionView->visibleSceneSize(M::Landscape);
    int w = correctionView->visibleSceneSize().width();
    int h = correctionView->visibleSceneSize().height();
    correctionView->scene()->setSceneRect(0, 0, w, h);
    correctionWindow->resize(sceneSize);
    correctionView->setMinimumSize(1, 1);
    correctionView->setMaximumSize(w, h);

    correctionSceneWindow = new MSceneWindow;
    correctionSceneWindow->setManagedManually(true); // we want the scene window to remain in origin
    correctionView->sceneManager()->appearSceneWindowNow(correctionSceneWindow);

    // construct correction candidate widget
    correctionCandidateWidget = new MImCorrectionCandidateWidget(correctionSceneWindow);
    correctionCandidateWidget->hide();
    connect(correctionCandidateWidget, SIGNAL(regionUpdated(const QRegion &)),
            correctionWindow, SLOT(handleRegionUpdate(const QRegion &)));
#else
    // construct correction candidate widget
    correctionCandidateWidget = new MImCorrectionCandidateWidget(sceneWindow);
    correctionCandidateWidget->hide();

    connect(correctionCandidateWidget, SIGNAL(regionUpdated(const QRegion &)),
            this, SLOT(handleRegionUpdate(const QRegion &)));
#endif
    connect(correctionCandidateWidget, SIGNAL(candidateClicked(const QString &)),
            this, SLOT(updatePreedit(const QString &)));
}


void MKeyboardHost::focusChanged(bool focusIn)
{
    haveFocus = focusIn;
    if (activeState == Hardware) {
        hardwareKeyboard->focusChanged(focusIn);
        if (!focusIn) {
            sendInputModeIndicator(MInputMethodBase::NoIndicator);
        }
        hideLockOnInfoBanner();
    }
}


void MKeyboardHost::show()
{
    QWidget *p = MPlainWindow::instance();

    if (p->nativeParentWidget()) {
        p = p->nativeParentWidget();
    }

    p->raise(); // make sure the window gets displayed

    if (activeState == Hardware) {
        if (!hardwareKeyboard->symViewAvailable())
            symbolView->hideSymbolView();
    } else {
        //Onscreen state
        if (!vkbWidget->symViewAvailable())
            symbolView->hideSymbolView();
    }

    updateCorrectionState();

    vkbWidget->showKeyboard();
}


void MKeyboardHost::hide()
{
    symbolView->hideSymbolView();
    vkbWidget->hideKeyboard();
}


void MKeyboardHost::setPreedit(const QString &preeditString)
{
    preedit = preeditString;
    correctedPreedit = preeditString;
    candidates.clear();
    imCorrectionEngine->clearEngineBuffer();
    imCorrectionEngine->appendString(preeditString);
    correctionCandidateWidget->setPreeditString(preeditString);
    candidates = imCorrectionEngine->candidates();
}


void MKeyboardHost::update()
{
    bool valid = false;

    const bool hasSelection = inputContextConnection()->hasSelection(valid);
    if (valid) {
        vkbWidget->setSelectionStatus(hasSelection);
    }

    const int type = inputContextConnection()->contentType(valid);
    if (valid) {
        hardwareKeyboard->setKeyboardType(static_cast<M::TextContentType>(type));
        vkbWidget->setKeyboardType(type);
    }

    autoCapsEnabled = (inputContextConnection()->autoCapitalizationEnabled(valid)
                       && valid
                       && (type != M::NumberContentType)
                       && (type != M::PhoneNumberContentType));

    if (autoCapsEnabled) {
        valid = inputContextConnection()->surroundingText(surroundingText, cursorPos);
        if (valid) {
            updateShiftState();
        }
    }

    const int inputMethodModeValue = inputContextConnection()->inputMethodMode(valid);
    if (valid) {
        inputMethodMode = inputMethodModeValue;
    }
}


void MKeyboardHost::updateShiftState()
{
    if (!autoCapsEnabled)
        return;

    // TODO: consider RTL language case
    // Capitalization is determined by preedit and Auto Capitalization.
    // If there are some preedit, it should be lower case.
    // Otherwise Auto Capitalization will turn on shift when (text entry capitalization option is ON):
    //   1. at the beginning of one paragraph
    //   2. after a sentence delimiter and one or more spaces
    static const QRegExp autoCapsTrigger("[" + AutoCapsSentenceDelimiters + "] +$");
    const bool upperCase = ((preedit.length() == 0)
                            && ((cursorPos == 0)
                                || ((cursorPos > 0)
                                    && (cursorPos <= surroundingText.length())
                                    && surroundingText.left(cursorPos).contains(autoCapsTrigger))));

    if ((activeState == OnScreen) && (vkbWidget->shiftStatus() != MVirtualKeyboard::ShiftLock)) {
        vkbWidget->setShiftState(upperCase ?
                                 MVirtualKeyboard::ShiftOn : MVirtualKeyboard::ShiftOff);
    } else if ((activeState == Hardware) &&
               (hardwareKeyboard->modifierState(Qt::ShiftModifier) != ModifierLockedState)) {
        hardwareKeyboard->setAutoCapitalization(upperCase);
    }
}

void MKeyboardHost::reset()
{
    qDebug() << __PRETTY_FUNCTION__;
    switch (activeState) {
    case OnScreen:
        preedit.clear();
        correctedPreedit.clear();
        candidates.clear();
        correctionCandidateWidget->setPreeditString("");
        imCorrectionEngine->clearEngineBuffer();
        vkbWidget->stopAccurateMode();
        break;
    case Hardware:
        hardwareKeyboard->reset();
        break;
    case Accessory:
        break;
    }
}


void MKeyboardHost::prepareOrientationChange()
{
    if (rotationInProgress) {
        return;
    }
    clearReactionMaps(MReactionMap::Inactive);
    rotationInProgress = true;

    symbolView->prepareToOrientationChange();
    vkbWidget->prepareToOrientationChange();
    correctionCandidateWidget->prepareToOrientationChange();
}

void MKeyboardHost::finalizeOrientationChange()
{
    angle = MPlainWindow::instance()->orientationAngle();

    vkbWidget->finalizeOrientationChange();

    symbolView->finalizeOrientationChange();
    //correct the visibility for the function row
    if (activeState == OnScreen) {
        symbolView->showFunctionRow();
    } else {
        symbolView->hideFunctionRow();
    }

    // Finalize candidate list after so its region will apply.
    correctionCandidateWidget->finalizeOrientationChange();

    // If correction candidate widget was open we need to reposition it.
    if (correctionCandidateWidget->isVisible()) {
        bool success = false;
        QRect rect = inputContextConnection()->preeditRectangle(success);
        QRect localRect;
        // Note: For Qt applications we don't have means to retrieve
        // the correct coordinates for pre-edit rectangle, so rect here
        // is null.
        if (success && !rect.isNull() && rotateRect(rect, localRect)) {
            int bottomLimit = static_cast<int>(sceneWindow->mapRectFromScene(
                                                   vkbWidget->region(false).boundingRect()).top());

            correctionCandidateWidget->setPosition(localRect, bottomLimit);
        } else {
            correctionCandidateWidget->hide();
        }
    }

    rotationInProgress = false;
}

bool MKeyboardHost::rotatePoint(const QPoint &screen, QPoint &window)
{
    bool res = true;

    switch (angle) {
    case M::Angle90:
        window.setX(screen.y());
        window.setY(displayWidth - screen.x());
        break;
    case M::Angle270:
        window.setX(displayHeight - screen.y());
        window.setY(screen.x());
        break;
    case M::Angle180:
        window.setX(displayWidth - screen.x());
        window.setY(displayHeight - screen.y());
        break;
    case M::Angle0:
        window.setX(screen.x());
        window.setY(screen.y());
        break;
    default:
        qCritical() << __FILE__ << __LINE__ << "Incorrect orientation" << angle;
        res = false;
        break;
    }
    return res;
}


bool MKeyboardHost::rotateRect(const QRect &screenRect, QRect &windowRect)
{
    bool res = true;

    if (!screenRect.isValid()) {
        windowRect = QRect();
        return false;
    }

    switch (angle) {
    case M::Angle90:
        windowRect.setRect(screenRect.y(),
                           displayWidth - screenRect.x() - screenRect.width(),
                           screenRect.height(), screenRect.width());
        break;
    case M::Angle270:
        windowRect.setRect(displayHeight - screenRect.y() - screenRect.height(),
                           screenRect.x(),
                           screenRect.height(), screenRect.width());
        break;
    case M::Angle180:
        windowRect.setRect(displayWidth - screenRect.x() - screenRect.width(),
                           displayHeight - screenRect.y() - screenRect.height(),
                           screenRect.width(), screenRect.height());
        break;
    case M::Angle0:
        windowRect = screenRect;
        break;
    default:
        qCritical() << __FILE__ << __LINE__ << " Incorrect orientation " << angle;
        windowRect = QRect();
        res = false;
        break;
    }
    return res;
}


void MKeyboardHost::mouseClickedOnPreedit(const QPoint &mousePos, const QRect &preeditRect)
{
    if (candidates.size() <= 1)
        return;

    QPoint localMousePos;
    QRect localRect;

    // Bottom limit for positioning candidate list widget. Keep above keyboard.
    int bottomLimit = static_cast<int>(vkbWidget->region(false).boundingRect().top());

    // Use preeditRect if one was passed (not null).
    if (!preeditRect.isNull() && rotateRect(preeditRect, localRect)) {
        correctionCandidateWidget->setPreeditString(correctedPreedit);
        correctionCandidateWidget->setCandidates(candidates);
        correctionCandidateWidget->setPosition(localRect, bottomLimit);
    } else if (rotatePoint(mousePos, localMousePos)) {
        correctionCandidateWidget->setPreeditString(correctedPreedit);
        correctionCandidateWidget->setCandidates(candidates);
        correctionCandidateWidget->setPosition(localMousePos, bottomLimit);
    } else {
        return;
    }

    correctionCandidateWidget->showWidget();
}


void MKeyboardHost::visualizationPriorityChanged(bool priority)
{
    if (priority == true) {
        vkbWidget->hideKeyboard(true, true);
    } else {
        vkbWidget->showKeyboard(true);
    }
}


void MKeyboardHost::appOrientationChanged(int angle)
{
    // The application receiving input has changed its orientation. Let's change ours.
    MPlainWindow::instance()->setOrientationAngle((M::OrientationAngle)angle);
}


void MKeyboardHost::setCopyPasteState(bool copyAvailable, bool pasteAvailable)
{
    vkbWidget->setCopyPasteButton(copyAvailable, pasteAvailable);
}


void MKeyboardHost::updatePreedit(const QString &updatedString)
{
    PreeditFace face = PreeditDefault;

    if (candidates.count() < 2)
        face = PreeditNoCandidates;

    inputContextConnection()->sendPreeditString(updatedString, face);
    correctedPreedit = updatedString;
    correctionCandidateWidget->setPreeditString(correctedPreedit);
}


void MKeyboardHost::doBackspace()
{
    // note: backspace shouldn't start accurate mode
    if (preedit.length() > 0) {
        if (!backSpaceTimer.isActive()) {
            setPreedit(preedit.left(preedit.length() - 1));
            const PreeditFace face = candidates.count() < 2 ? PreeditNoCandidates : PreeditDefault;
            inputContextConnection()->sendPreeditString(preedit, face);
        } else {
            reset();
            backSpaceTimer.stop();
            inputContextConnection()->sendCommitString("");
        }
    } else {
        static const KeyEvent event("\b", QEvent::KeyRelease, Qt::Key_Backspace,
                                    KeyEvent::NotSpecial,
                                    vkbWidget->shiftStatus() != MVirtualKeyboard::ShiftOff
                                    ? Qt::ShiftModifier : Qt::NoModifier);
        inputContextConnection()->sendKeyEvent(KeyEvent(event, QEvent::KeyPress).toQKeyEvent());
        inputContextConnection()->sendKeyEvent(event.toQKeyEvent());
    }
    // Backspace toggles shift off if it's on (not locked)
    // except if autoCaps is on and cursor is at 0 position.
    if (vkbWidget->shiftStatus() == MVirtualKeyboard::ShiftOn
        && (!autoCapsEnabled || cursorPos != 0)) {
        vkbWidget->setShiftState(MVirtualKeyboard::ShiftOff);
    }
}

void MKeyboardHost::autoBackspace()
{
    backSpaceTimer.start(BackspaceRepeatInterval); // Must restart before doBackspace
    doBackspace();
}

void MKeyboardHost::handleKeyPress(const KeyEvent &event)
{
    if (((inputMethodMode == M::InputMethodModeDirect)
         && (event.specialKey() == KeyEvent::NotSpecial))
        || (event.qtKey() == Qt::Key_plusminus)) { // plusminus key makes an exception

        inputContextConnection()->sendKeyEvent(event.toQKeyEvent());

    } else if (event.qtKey() == Qt::Key_Backspace) {
        backSpaceTimer.start(AutoBackspaceDelay);
    }
}

void MKeyboardHost::handleKeyRelease(const KeyEvent &event)
{
    if (((inputMethodMode == M::InputMethodModeDirect)
         && (event.specialKey() == KeyEvent::NotSpecial))
        || (event.qtKey() == Qt::Key_plusminus)) { // plusminus key makes an exception

        inputContextConnection()->sendKeyEvent(event.toQKeyEvent());

    } else if ((event.qtKey() == Qt::Key_Backspace) && backSpaceTimer.isActive()) {
        backSpaceTimer.stop();
        doBackspace();
    }
}

void MKeyboardHost::updateReactionMaps()
{
    if (rotationInProgress) {
        return;
    }

    // Draw the reactive areas of first one of these who is visible.
    if (layoutMenu->isActive()) {
        layoutMenu->redrawReactionMaps();
    } else if (correctionCandidateWidget->isVisible()) {
        correctionCandidateWidget->redrawReactionMaps();
    } else if (symbolView->isFullyVisible()) {
        symbolView->redrawReactionMaps();
    } else if (vkbWidget->isFullyVisible()) {
        vkbWidget->redrawReactionMaps();
    } else {
        // Transparent reaction map when nothing is shown.
        clearReactionMaps(MReactionMap::Transparent);
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
    if (activeState == Hardware) {
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
    lastClickEventTime = QTime::currentTime();
}

void MKeyboardHost::handleGeneralKeyClick(const KeyEvent &event)
{
    if (event.qtKey() == Qt::Key_Shift) {
        switch (vkbWidget->shiftStatus()) {
        case MVirtualKeyboard::ShiftOn:
            vkbWidget->setShiftState(MVirtualKeyboard::ShiftLock);
            break;
        case MVirtualKeyboard::ShiftOff:
            vkbWidget->setShiftState(MVirtualKeyboard::ShiftOn);
            break;
        case MVirtualKeyboard::ShiftLock:
            vkbWidget->setShiftState(MVirtualKeyboard::ShiftOff);
            break;
        }
    } else if (vkbWidget->shiftStatus() == MVirtualKeyboard::ShiftOn
               && (event.qtKey() != Qt::Key_Backspace)) {
        // Any key except shift toggles shift off if it's on (not locked).
        // backspace toggles shift off is handled in doBackspace().
        vkbWidget->setShiftState(MVirtualKeyboard::ShiftOff);
    }

    if (event.specialKey() == KeyEvent::LayoutMenu) {
        showLayoutMenu();
    } else if (event.specialKey() == KeyEvent::Sym) {
        handleSymbolKeyClick();
    }
}

void MKeyboardHost::handleTextInputKeyClick(const KeyEvent &event)
{
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

    QString text(event.text());

    // Handle multitap. Needs to be before any further handling of the text variable.
    // Picks the relevant part of text variable.
    if (event.specialKey() == KeyEvent::CycleSet) {
        if ((lastClickEvent == event)
            && (QTime::currentTime() < lastClickEventTime.addMSecs(MultitapTime))) {
            if (preedit.length() > 0) {
                setPreedit(preedit.left(preedit.length() - 1));
            }
            multitapIndex = (multitapIndex + 1) % text.size();
        } else {
            if (lastClickEvent.specialKey() == KeyEvent::CycleSet) {
                inputContextConnection()->sendCommitString(correctedPreedit);
                preedit.clear();
                correctedPreedit.clear();
            }
            multitapIndex = 0;
        }
        text = text[multitapIndex];
    }

    if (text.isEmpty()) {
        return;
    }

    // Word separator stops accurate mode
    if (MVirtualKeyboard::WordSeparators.indexOf(text) >= 0) {
        vkbWidget->stopAccurateMode();

        if (engineReady
            && imCorrectionEngine->correctionEnabled()
            && (candidates.count() < 2)
            && !preedit.isEmpty()) {
            if (feedbackPlayer) {
                feedbackPlayer->play(MFeedbackPlayer::Cancel);
            }
        } else if (feedbackPlayer) {
            feedbackPlayer->play(MFeedbackPlayer::Release);
        }
    }

    if (event.specialKey() == KeyEvent::CycleSet) {
        preedit += text;
        correctedPreedit = preedit;

        inputContextConnection()->sendPreeditString(correctedPreedit, PreeditNoCandidates);

    } else if (vkbWidget->isAccurateMode() || !correctionEnabled) {
        if (correctedPreedit.length() > 0) {
            // we just entered accurate mode. send the previous preedit stuff.
            inputContextConnection()->sendCommitString(correctedPreedit);
            preedit.clear();
            correctedPreedit.clear();
        }

        inputContextConnection()->sendCommitString(text);

    } else if ((event.qtKey() == Qt::Key_Space) || (event.qtKey() == Qt::Key_Return)) {
        // commit string
        inputContextConnection()->sendCommitString(correctedPreedit);
        if (lastClickEvent.specialKey() != KeyEvent::CycleSet) {
            imCorrectionEngine->setSuggestedCandidateIndex(correctionCandidateWidget->activeIndex());
            imCorrectionEngine->saveAndClearEngineBuffer();
            correctionCandidateWidget->setPreeditString("");
        } else {
            imCorrectionEngine->clearEngineBuffer();
        }
        // send trailing space
        inputContextConnection()->sendCommitString(text);

        correctedPreedit.clear();
        preedit.clear();
    } else {
        // common case: just append stuff to current preedit
        preedit += text;

        candidates.clear();
        imCorrectionEngine->appendCharacter(text.at(0));
        candidates = imCorrectionEngine->candidates();
        correctedPreedit = preedit;

        if (candidates.size() > 0) {
            //deal with the candidates about capslock
            candidates.replaceInStrings(preedit.toLower(), preedit, Qt::CaseInsensitive);

            correctedPreedit = candidates[imCorrectionEngine->suggestedCandidateIndex()];

            if (correctedPreedit.size() != preedit.size()) {
                // Use the candidate with the same length as preedit
                bool findEqualLength = false;

                for (int i = 0; i < candidates.size(); i++) {
                    QString candidate = candidates.at(i);
                    if (candidate.size() == preedit.size()) {
                        findEqualLength = true;
                        correctedPreedit = candidate;
                        correctionCandidateWidget->setPreeditString(correctedPreedit);
                        break;
                    }
                }

                // If no candidate of equal length, just use the preedit
                if (!findEqualLength) {
                    correctedPreedit = preedit;
                }
            }
        }
        const PreeditFace face = candidates.count() < 2 ? PreeditNoCandidates : PreeditDefault;
        inputContextConnection()->sendPreeditString(correctedPreedit, face);
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

    // TODO: wouldn't it be better if correction engine did this?
    QString shortLanguage(language);
    if (KeyboardData::isLanguageLongFormat(language)) {
        shortLanguage = KeyboardData::convertLanguageToShortFormat(language);
    }

    if (engineReady && (shortLanguage != imCorrectionEngine->language())) {
        // TODO: maybe we should check return values here and in case of failure
        // be always in accurate mode, for example
        imCorrectionEngine->setKeyboardLayout(shortLanguage);
        imCorrectionEngine->setLanguage(shortLanguage, Dui::LanguagePriorityPrimary);
        synchronizeCorrectionSetting();
        imCorrectionEngine->disablePrediction();
        imCorrectionEngine->disableCompletion();
        imCorrectionEngine->setMaximumErrors(6);
        imCorrectionEngine->setExactWordPositionInList(2);
    }
}


void MKeyboardHost::errorCorrectionToggled(bool on)
{
    if (engineReady && (on != imCorrectionEngine->correctionEnabled())) {
        bool correction = true;
        if (!inputMethodCorrectionSettings->value().isNull())
            correction = inputMethodCorrectionSettings->value().toBool();
        if (on != correction) {
            inputMethodCorrectionSettings->set(QVariant(on));
        }
    }
}


void MKeyboardHost::synchronizeCorrectionSetting()
{
    bool correction = true;
    if (!inputMethodCorrectionSettings->value().isNull())
        correction = inputMethodCorrectionSettings->value().toBool();

    if (!correction) {
        imCorrectionEngine->disableCorrection();
        layoutMenu->disableErrorCorrection();
    } else {
        imCorrectionEngine->enableCorrection();
        layoutMenu->enableErrorCorrection();
    }

    updateCorrectionState();
}


void MKeyboardHost::updateCorrectionState()
{
    if (activeState == Hardware) {
        inputContextConnection()->setGlobalCorrectionEnabled(false);
        correctionEnabled = false;
    } else {
        bool val = false;
        bool enabled = inputContextConnection()->correctionEnabled(val);
        if (val)
            correctionEnabled = enabled && imCorrectionEngine->correctionEnabled();
        else
            correctionEnabled = imCorrectionEngine->correctionEnabled();

        // info context the global correction option
        // TODO: should not put setGlobalCorrectionEnabled here, it will send correction setting
        // whenever focus changes. But have to at this moment, because im-uiserver start before
        // application, and there is no focus widget, no activateContext, calling
        // setGlobalCorrectionEnabled() at that time, can not record the setting.
        // Only after the application is running, this setGlobalCorrectionEnabled() can take effect
        inputContextConnection()->setGlobalCorrectionEnabled(imCorrectionEngine->correctionEnabled());
    }
}


void MKeyboardHost::userHide()
{
    inputContextConnection()->notifyImInitiatedHiding();
}


void MKeyboardHost::sendCopyPaste(CopyPasteState action)
{
    switch (action) {
    case InputMethodCopy:
        inputContextConnection()->copy();
        break;
    case InputMethodPaste:
        inputContextConnection()->paste();
        break;
    default:
        qDebug() << __PRETTY_FUNCTION__ << "invalid action" << action;
        break;
    }
}

void MKeyboardHost::showLayoutMenu()
{
    const QStringList languageList = LayoutsManager::instance().languageList();
    const int currentIndex = languageList.indexOf(vkbWidget->selectedLanguage());

    // Update layout menu's list of language titles and current language.
    QStringList titles;
    foreach (const QString &language, languageList) {
        titles << LayoutsManager::instance().keyboardTitle(language);
    }

    layoutMenu->setLanguageList(titles, currentIndex);
    layoutMenu->show();
}

QRegion MKeyboardHost::combineRegionTo(RegionMap &regionStore,
                                         const QRegion &region, const QObject &widget)
{
    regionStore[&widget] = region;

    QRegion combinedRegion;
    foreach (const QRegion &partialRegion, regionStore) {
        combinedRegion |= partialRegion;
    }

    return combinedRegion;
}

void MKeyboardHost::handleRegionUpdate(const QRegion &region)
{
    emit regionUpdated(combineRegionTo(widgetRegions, region, *QObject::sender()));
    updateReactionMaps();
}

void MKeyboardHost::handleInputMethodAreaUpdate(const QRegion &region)
{
    emit inputMethodAreaUpdated(combineRegionTo(inputMethodAreaWidgetRegions,
                                                region, *QObject::sender()));
}

void MKeyboardHost::sendKeyEvent(const QKeyEvent &key)
{
    inputContextConnection()->sendKeyEvent(key);
}

void MKeyboardHost::sendString(const QString &text)
{
    inputContextConnection()->sendCommitString(text);
}

void MKeyboardHost::registerToolbar(qlonglong id, const QString &fileName)
{
    ToolbarManager::instance().registerToolbar(id, fileName);
}

void MKeyboardHost::unregisterToolbar(qlonglong id)
{
    ToolbarManager::instance().unregisterToolbar(id);
}

void MKeyboardHost::setToolbar(qlonglong id)
{
    qDebug() << __PRETTY_FUNCTION__ << id;
    if (id >= 0) {
        vkbWidget->showToolbarWidget(id);
    } else {
        vkbWidget->hideToolbarWidget();
    }
}

void MKeyboardHost::setToolbarItemAttribute(qlonglong id, const QString &item,
                                              const QString &attribute, const QVariant &value)
{
    ToolbarManager::instance().setToolbarItemAttribute(id, item, attribute, value);
}

void MKeyboardHost::processKeyEvent(QEvent::Type keyType, Qt::Key keyCode,
                                    Qt::KeyboardModifiers modifiers, const QString &text,
                                    bool autoRepeat, int count, quint32 nativeScanCode,
                                    quint32 nativeModifiers)
{
    if ((activeState != Hardware) ||
        !hardwareKeyboard->filterKeyEvent(keyType, keyCode, modifiers, text,
                                          autoRepeat, count, nativeScanCode,
                                          nativeModifiers)) {
        inputContextConnection()->sendKeyEvent(QKeyEvent(keyType, keyCode, modifiers, text,
                                                         autoRepeat, count));
    }
}

void MKeyboardHost::clientChanged()
{
    reset();
    hide(); // could do some quick hide also
}

void MKeyboardHost::switchContext(M::InputMethodSwitchDirection direction, bool enableAnimation)
{
    if (activeState == OnScreen) {
        vkbWidget->switchLanguage(direction, enableAnimation);
    }
}

void MKeyboardHost::setState(const QSet<MIMHandlerState> &state)
{
    if (state.isEmpty()) {
        return;
    }

    const MIMHandlerState actualState = *state.begin();
    if (activeState == actualState)
        return;

    // Resets before changing the activeState to make sure clear.
    reset();
    activeState = actualState;

    // Keeps separate states for symbol view in OnScreen state and Hardware state
    if (activeState == OnScreen) {
        symbolView->setLanguage(vkbWidget->selectedLanguage());
        symbolView->showFunctionRow();
        hideLockOnInfoBanner();
        sendInputModeIndicator(MInputMethodBase::NoIndicator);
        disconnect(hardwareKeyboard, SIGNAL(modifierStateChanged(Qt::KeyboardModifier, ModifierState)),
                   this, SLOT(handleModifierStateChanged(Qt::KeyboardModifier, ModifierState)));
        if (haveFocus) {
            hardwareKeyboard->focusChanged(false);
        }
    } else {
        //TODO: this is a temporary method, should get the hw layout language, then find out the
        //language (symbol view variant) according Table 3 in HW Keyboard UI spec.
        symbolView->setLanguage(LayoutsManager::instance().hardwareKeyboardLayout());
        symbolView->hideFunctionRow();
        connect(hardwareKeyboard, SIGNAL(modifierStateChanged(Qt::KeyboardModifier, ModifierState)),
                this, SLOT(handleModifierStateChanged(Qt::KeyboardModifier, ModifierState)));
        if (haveFocus) {
            hardwareKeyboard->focusChanged(true);
        }
    }

    vkbWidget->setKeyboardState(actualState);
    updateCorrectionState();
    updateShiftState();
}

void MKeyboardHost::handleSymbolKeyClick()
{
    if (!symbolView->isActive()) {

        if ((activeState == Hardware) && !hardwareKeyboard->symViewAvailable()) {
            return;
        } else if (!vkbWidget->symViewAvailable()) {
            return;
        }
        symbolView->showSymbolView();
        //give the symbolview right shift level(for hardware state)
        updateSymbolViewLevel();
    } else {
        if (symbolView->currentPage() < (symbolView->pageCount() - 1)) {
            symbolView->switchToNextPage();
        } else {
            symbolView->hideSymbolView();
        }
    }
}

void MKeyboardHost::updateSymbolViewLevel()
{
    if (!symbolView->isActive())
        return;

    MVirtualKeyboard::ShiftLevel shiftLevel = MVirtualKeyboard::ShiftOff;
    if (activeState == OnScreen) {
        shiftLevel = vkbWidget->shiftStatus();
    } else {
        switch (hardwareKeyboard->modifierState(Qt::ShiftModifier)) {
        case ModifierLatchedState:
            shiftLevel = MVirtualKeyboard::ShiftOn;
            break;
        case ModifierLockedState:
            shiftLevel = MVirtualKeyboard::ShiftLock;
            break;
        default:
            break;
        }
    }
    symbolView->switchLevel(shiftLevel > 0 ? 1 : 0,
                            shiftLevel == MVirtualKeyboard::ShiftLock);
}

void MKeyboardHost::showSymbolView()
{
    symbolView->showSymbolView(SymbolView::FollowMouseShowMode);
    //give the symbolview right shift level(for hardware state)
    updateSymbolViewLevel();
}

void MKeyboardHost::handleModifierStateChanged(Qt::KeyboardModifier modifier, ModifierState state)
{
    // only change indicator state when there is focus is in a widget and state is Hardware.
    if (!haveFocus || activeState != Hardware)
        return;
    const QString currentHwKeyboardLayout = LayoutsManager::instance().hardwareKeyboardLayout();
    QString lockOnNotificationLabel;
    MInputMethodBase::InputModeIndicator indicatorState = MInputMethodBase::LatinLower;
    switch (modifier) {
    case Qt::ShiftModifier:
        switch (state) {
        case ModifierClearState:
            if (hardwareKeyboard->modifierState(FnLevelModifier) != ModifierClearState) {
                return;
            }
            if (currentHwKeyboardLayout == "ar") {
                indicatorState = MInputMethodBase::Arabic;
            } else if (LayoutsManager::isCyrillicLanguage(currentHwKeyboardLayout)) {
                indicatorState = MInputMethodBase::CyrillicLower;
            } else {
                indicatorState = MInputMethodBase::LatinLower;
            }
            break;
        case ModifierLatchedState:
            if (currentHwKeyboardLayout == "ar") {
                indicatorState = MInputMethodBase::Arabic;
            } else if (LayoutsManager::isCyrillicLanguage(currentHwKeyboardLayout)) {
                indicatorState = MInputMethodBase::CyrillicUpper;
            } else {
                indicatorState = MInputMethodBase::LatinUpper;
            }
            break;
        case ModifierLockedState:
            if (currentHwKeyboardLayout == "ar") {
                indicatorState = MInputMethodBase::Arabic;
            } else if (LayoutsManager::isCyrillicLanguage(currentHwKeyboardLayout)) {
                indicatorState = MInputMethodBase::CyrillicLocked;
            } else {
                indicatorState = MInputMethodBase::LatinLocked;
            }
            //% "Caps lock on"
            lockOnNotificationLabel = qtTrId("qtn_hwkb_caps_lock");
            break;
        }
        break;
    case FnLevelModifier:
        switch (state) {
        case ModifierClearState:
            if (hardwareKeyboard->modifierState(Qt::ShiftModifier) != ModifierClearState) {
                return;
            }
            // when fn key change back to clear, shows same label as shift is clear
            if (currentHwKeyboardLayout == "ar") {
                indicatorState = MInputMethodBase::Arabic;
            } else if (LayoutsManager::isCyrillicLanguage(currentHwKeyboardLayout)) {
                indicatorState = MInputMethodBase::CyrillicLower;
            } else {
                indicatorState = MInputMethodBase::LatinLower;
            }
            break;
        case ModifierLatchedState:
            indicatorState = MInputMethodBase::NumAndSymLatched;
            break;
        case ModifierLockedState:
            indicatorState = MInputMethodBase::NumAndSymLocked;
            //% "Symbol lock on"
            lockOnNotificationLabel = qtTrId("qtn_hwkb_fn_lock");
            break;
        }
        break;
    default:
        //do not deal with the other modifiers now.
        return;
    }
    sendInputModeIndicator(indicatorState);
    if (state == ModifierLockedState
        && hardwareKeyboard->keyboardType() != M::NumberContentType
        && hardwareKeyboard->keyboardType() != M::PhoneNumberContentType) {
        // notify the modifier is changed to locked state
        // number and phone number content type always force FN key to be locked,
        // don't need indicator lock notification.
        if (modifierLockOnInfoBanner) {
            modifierLockOnInfoBanner->setBodyText(lockOnNotificationLabel);
            modifierLockOnTimer.start();
        } else {
            modifierLockOnInfoBanner = new MInfoBanner(MInfoBanner::Information);
            modifierLockOnInfoBanner->setBodyText(lockOnNotificationLabel);
            MPlainWindow::instance()->sceneManager()->appearSceneWindow(modifierLockOnInfoBanner,
                    MSceneWindow::DestroyWhenDone);
            modifierLockOnTimer.start();
        }
    } else if (modifierLockOnInfoBanner) {
        hideLockOnInfoBanner();
    }
}

void MKeyboardHost::hideLockOnInfoBanner()
{
    if (modifierLockOnInfoBanner)
        MPlainWindow::instance()->sceneManager()->disappearSceneWindow(modifierLockOnInfoBanner);
    modifierLockOnInfoBanner = 0;
}
