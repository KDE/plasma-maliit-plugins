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



#ifndef MKEYBOARDHOST_H
#define MKEYBOARDHOST_H

#include "mkeyboardcommon.h"
#include "mabstractinputmethod.h"
#include "keyevent.h"
#include <MNamespace>
#include <QStringList>
#include <QTimer>
#include <QPointer>

class MGConfItem;
class MImCorrectionHost;
class MSceneWindow;
class MVirtualKeyboard;
class MVirtualKeyboardStyleContainer;
class MHardwareKeyboard;
class SymbolView;
class MImEngineWordsInterface;
class QWidget;
class MBanner;
class SharedHandleArea;
class MImToolbar;
class MAbstractInputMethodHost;


//! Logic class for virtual keyboard
class MKeyboardHost: public MAbstractInputMethod
{
    Q_OBJECT

public:
    MKeyboardHost(MAbstractInputMethodHost *imHost, QObject *parent = 0);
    virtual ~MKeyboardHost();

    //! reimp
    virtual void handleFocusChange(bool focusIn);
    virtual void show();
    virtual void hide();
    // Note: application call this setPreedit() to tell keyboard about the preedit information.
    // If keyboard wants to update preedit, it should not call the method but call the lower
    // localSetPreedit().
    virtual void setPreedit(const QString &preeditString, int cursorPosition);
    virtual void update();
    virtual void reset();
    virtual void handleMouseClickOnPreedit(const QPoint &mousePos, const QRect &preeditRect);
    virtual void handleVisualizationPriorityChange(bool priority);
    virtual void handleAppOrientationChange(int angle);
    virtual void setToolbar(QSharedPointer<const MToolbarData> toolbar);
    virtual void setState(const QSet<MInputMethod::HandlerState> &state);
    virtual void processKeyEvent(QEvent::Type keyType, Qt::Key keyCode,
                                 Qt::KeyboardModifiers modifiers,
                                 const QString &text, bool autoRepeat, int count,
                                 quint32 nativeScanCode, quint32 nativeModifiers);
    virtual void handleClientChange();
    virtual void switchContext(MInputMethod::SwitchDirection direction, bool enableAnimation);
    virtual QList<MAbstractInputMethod::MInputMethodSubView> subViews(MInputMethod::HandlerState state
                                                                  = MInputMethod::OnScreen) const;
    virtual void setActiveSubView(const QString &,
                                  MInputMethod::HandlerState state = MInputMethod::OnScreen);
    virtual QString activeSubView(MInputMethod::HandlerState state = MInputMethod::OnScreen) const;
    virtual void showLanguageNotification();
    //! reimp_end

private slots:
    /*!
     * Handle key clicks from widgets
     * \param event internal key event
     */
    void handleKeyClick(const KeyEvent &event);

    /*!
     * Send QKeyEvent when key is pressed in direct input mode
     * \param event internal key event
     */
    void handleKeyPress(const KeyEvent &event);

    /*!
     * Send QKeyEvent when key is released in direct input mode
     * \param event internal key event
     */
    void handleKeyRelease(const KeyEvent &event);

    /*!
     * Handles user long press a key.
     * \param event internal key event
     */
    void handleLongKeyPress(const KeyEvent &event);

    //! \brief Draws reaction maps for the topmost widget.
    void updateReactionMaps();

    /*!
     * \brief Commits \a string.
     *
     * If there exists preedit, the preedit will be replaced.
     */
    void commitString(const QString &string);

    /*! \brief Prepares vkb for orientation change when application is about to rotate.
     *
     * This should hide vkb.
     */
    void prepareOrientationChange();

    /*! \brief Finalizes orientation change after application has rotated.
     *
     * This should popup vkb if it was visible before orientation change.
     */
    void finalizeOrientationChange();

    //! Synchronize correction setting
    void synchronizeCorrectionSetting();

    //! handles user initiated hiding of the keyboard
    void userHide();

    //! does one backspace and schedules the next if it is holding backspace.
    void autoBackspace();

    /*! \brief Sends request to copy or paste text
     *  \param action ImCopyPasteState Required action (copy or paste)
     */
    virtual void sendCopyPaste(CopyPasteState action);

    /*! Receive region updates from widgets, combine them and signal as input method's region
     * using \a MAbstractInputMethod::regionUpdated.
     *
     * \param region updated region
     */
    void handleRegionUpdate(const QRegion &region);

    //! This overloaded function handles region updates from sharedHandleArea.
    void handleRegionUpdate();

    /*!
     * Receive region updates from widgets, combine them and signal as input
     * method area using \a MAbstractInputMethod::inputMethodAreaUpdated.
     *
     * \param region updated region
     */
    void handleInputMethodAreaUpdate(const QRegion &region);

    //! This overloaded function handles region updates from sharedHandleArea.
    void handleInputMethodAreaUpdate();

    //! Changes plugin into given direction
    void switchPlugin(MInputMethod::SwitchDirection direction);

    //! Sends key event
    void sendKeyEvent(const QKeyEvent &);

    //! Sends string
    void sendString(const QString &);

    //! Sends string from toolbar
    void sendStringFromToolbar(const QString &);

    //! Handle symbol key click.
    void handleSymbolKeyClick();

    //! Updates the shift level for Symbol view.
    void updateSymbolViewLevel();

    //! Shows symbol view
    void showSymbolView();

    /*!
     * Receives modifier state changed signal or script changed signal  from hardware
     * keyboard, sends input mode indicator state notification to Application Framework
     * (Home screen status bar).
     */
    void handleHwKeyboardStateChanged();

    //!Receives modifier state changed and shows Caps Lock infobanner
    void handleVirtualKeyboardCapsLock();

    //! show FN/Caps Lock infobanner
    void showLockOnInfoBanner(const QString &notification);

    //! hide FN/Caps Lock infobanner
    void hideLockOnInfoBanner();

    //! Handle active layout is changed to \a layout for virtual keyboard.
    void handleVirtualKeyboardLayoutChanged(const QString &layout);

    void turnOffFastTyping();

private:
    //! \brief Reset internal state, used by reset() and others
    void resetInternalState();

    void createCorrectionCandidateWidget();

    //! Rotates coodinates from screen to window
    bool rotatePoint(const QPoint &screen, QPoint &window);
    bool rotateRect(const QRect &screenRect, QRect &windowRect);

    //! Update error correction flag
    void updateCorrectionState();

    //! update autocapitalization state
    void updateAutoCapitalization();

    //! update context
    void updateContext();

    /*!
     * Reset temporary shift state (shift on state set by user or auto capitalization,
     * besides capslocked) for virtual keyboard.
     */
    void resetVirtualKeyboardShiftState();

    //! Actual backspace operation
    void doBackspace();

    // shows the layout menu
    void showLayoutMenu();

    /*! \brief Handle key click event that changes the state of the keyboard.
     *
     *  This method should contain functionality that is common to
     *  both directmode and non-directmode.
     *  \sa handleTextInputKeyClick
     */
    void handleGeneralKeyClick(const KeyEvent &event);

    /*! \brief Handle key click event that relates to sending text.
     *
     *  This key click handling is not called in direct mode since
     *  it causes extra text to be sent to input context.
     *  \sa handleGeneralKeyClick
     */
    void handleTextInputKeyClick(const KeyEvent &event);

    /*! \brief Clears reaction maps with given MReactionMap color value.
     *  \param clearValue A MReactionMap color value such as MReactionMap::Inactive.
     */
    void clearReactionMaps(const QString &clearValue);

    /*! Mapping from QObject to region.
     */
    typedef QPair<QPointer<QObject>, QRegion> ObjectRegionPair;

    /*! Container of region mapping information.
     */
    typedef QList<ObjectRegionPair> RegionList;

    /*!
     * \brief Save \a region occupied by \a widget into \a regionStore.
     */
    void setRegionInfo(RegionList &regionStore,
                       const QRegion &region,
                       const QPointer<QObject> &widget);
    /*!
     * \brief Return union of all regions in \a regionStore after adding sharedHandleArea region.
     */
    QRegion combineRegion();

    /*!
     * \brief Return union of all regions in \a regionStore after adding sharedHandleArea region.
     */
    QRegion combineInputMethodArea();

    /*!
     * \brief Common implementation for combineRegion() and combineInputMethodArea().
     * \param includeExtraInteractiveAreas Result includes extra interactive area
     * if this parameter is true.
     */
    QRegion combineRegionImpl(const RegionList &regionStore,
                              bool includeExtraInteractiveAreas);


    //! initialize input engine
    void initializeInputEngine();

    //! \return input mode indicator corresponding to a dead \a key character or
    //! MInputMethod::NoIndicator if not a (supported) dead key character
    static MInputMethod::InputModeIndicator deadKeyToIndicator(const QChar &key);

    //! update input engine keyboard layout according keyboard layout.
    void updateEngineKeyboardLayout();

    //! set preedit string and cursor position.
    void localSetPreedit(const QString &preeditString, int replaceStart, int replaceLength,
                         int cursorPosition);

    /*!
     * \brief send preedit string and cursor to application and its style depends on \a candidateCount.
     *
     * \param string The preedit string
     * \param candidateCount The count of candidates suggested by input engine.
     * \param cursorPosition The cursor position inside preedit. Default value -1 indicates to hide the cursor.
     */
    void updatePreedit(const QString &string, int candidateCount, int replaceStart = 0,
                       int replaceLength = 0, int cursorPosition = -1);

    /*!
     * \brief This enum defines different mode for backspace clicking.
     */
    enum BackspaceMode {
        NormalBackspaceMode,      //!< backspace for normal state.
        AutoBackspaceMode,        //!< auto backspace state.
        WordTrackerBackspaceMode  //!< backspace when word tracker is visible.
    };

    //! start backspaceTimer according \a mode
    void startBackspace(BackspaceMode mode);

    //! Send return as press&release event pair, send as input method event with
    //! commit string otherwise.
    void sendCommitStringOrReturnEvent(const KeyEvent &event) const;

    //! update correction widget position.
    void updateCorrectionWidgetPosition();

    //! Sends backspace key event to application.
    void sendBackSpaceKeyEvent() const;

    void turnOnFastTyping();

private:
    class CycleKeyHandler; //! Reacts to cycle key press events.
    friend class CycleKeyHandler;
    QString preedit;

    MVirtualKeyboardStyleContainer *vkbStyleContainer;

    MImCorrectionHost *correctionHost;
    MVirtualKeyboard *vkbWidget;
    MHardwareKeyboard *hardwareKeyboard;
    SymbolView *symbolView;

    MImEngineWordsInterface *imCorrectionEngine;
    //! default input method error correction setting
    MGConfItem *inputMethodCorrectionSettings;
    MGConfItem *inputMethodCorrectionEngine;

    QStringList candidates;

    M::OrientationAngle angle;
    int displayWidth;
    int displayHeight;

    //! error correction flag
    bool correctionEnabled;

    //! FIXME: should we provide such a flag to on/off auto caps
    bool autoCapsEnabled;
    //! Contains true if autocapitalization decides to switch keyboard to upper case
    bool autoCapsTriggered;
    QString surroundingText;
    int cursorPos;
    int preeditCursorPos;
    bool hasSelection;

    int inputMethodMode;

    QTimer backspaceTimer;

    QTimer rotationTimer;

    KeyEvent lastClickEvent;

    //! Keeps track of shift up/down status.
    bool shiftHeldDown;

    MSceneWindow *sceneWindow;

    //! Regions of widgets created by MKeyboardHost
    RegionList widgetRegions;

    //! Regions of widgets that affect the input method area
    RegionList inputMethodAreaWidgetRegions;

    //! current active state
    MInputMethod::HandlerState activeState;

    QPointer<MBanner> modifierLockOnBanner; //! widget to notify modifier is in locked state

    //! Indicates whether focus is in a widget according to focusChanged calls
    //! \sa focusChanged
    bool haveFocus;

    //! In practice show() and hide() correspond to application SIP (close)
    //! requests.  We track the current shown/SIP requested state using this variable.
    bool sipRequested;

    //! Contains true if multi-touch is enabled
    bool enableMultiTouch;

    //! Handle area containing toolbar widget.
    QPointer<SharedHandleArea> sharedHandleArea;

    //! Toolbar widget containing copy/paste and custom buttons.
    QPointer<MImToolbar> imToolbar;

    CycleKeyHandler *cycleKeyHandler;

    bool currentIndicatorDeadKey;

    //! Indicates whether engine layout need to be updated
    bool engineLayoutDirty;

    BackspaceMode backspaceMode;

    bool wordTrackerSuggestionAcceptedWithSpace;

    QTimer fastTypingTimeout;
    int fastTypingKeyCount;
    bool fastTypingEnabled;

#ifdef UNIT_TEST
    friend class Ut_MKeyboardHost;
#endif
};

#endif
