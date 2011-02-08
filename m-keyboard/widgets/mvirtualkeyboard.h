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



#ifndef MVIRTUALKEYBOARD_H
#define MVIRTUALKEYBOARD_H

#include "keyeventhandler.h"
#include "mkeyboardcommon.h"
#include "mimkeyarea.h"
#include "layoutdata.h"

#include <minputmethodnamespace.h>
#include <MWidget>
#include <MNamespace>
#include <mimenginetypes.h>
#include <QPixmap>
#include <QSharedPointer>
#include <QPointer>

class QGraphicsGridLayout;
class QGraphicsLinearLayout;
class QGraphicsWidget;
class MButton;
class MScalableImage;
class MSceneManager;
class MVirtualKeyboardStyleContainer;
class HorizontalSwitcher;
class KeyEvent;
class LayoutsManager;
class Notification;
class MImKeyModel;
class VkbToolbar;
class MImToolbar;
class MReactionMap;
class MToolbarData;
class Handle;
class Grip;
class FlickGesture;
class SharedHandleArea;

/*!
  \class MVirtualKeyboard

  \brief The MVirtualKeyboard class provides interfaces for the usage of the
   virtual keyboard. The interfaces include hide/show/orientation of the keyboard.
   It also provides interfaces to get keystatus

*/
class MVirtualKeyboard : public MWidget
{
    Q_OBJECT
    Q_PROPERTY(QString layoutLanguage READ layoutLanguage)
    Q_PROPERTY(QString layoutTitle READ layoutTitle)

    friend class Ut_MVirtualKeyboard;
    friend class Ut_MKeyboardHost;

public:
    /*!
     * \brief Constructor for creating an virtual keyboard object.
     * \param parent Parent object.
     */
    MVirtualKeyboard(const LayoutsManager &layoutsManager,
                     const MVirtualKeyboardStyleContainer *styleContainer,
                     QGraphicsWidget *parent = 0);

    //! Destructor
    ~MVirtualKeyboard();

    /*!
     * \brief Method to get the language for the currently displayed layout
     *
     * Note that this is the real language found in the XML loaded based on a language
     * list entry (which may be different).
     * \return the language
     */
    QString layoutLanguage() const;

    /*!
     * \brief Method to get the title for the currently displayed layout
     *
     * Note that this is the real title found in the XML loaded.
     * \return the title
     */
    QString layoutTitle() const;

    /*!
     * \brief Get the layout currently selected (from layout list)
     */
    QString selectedLayout() const;

    //! Sets keyboard type according text entry type, type matches M::TextContentType
    void setKeyboardType(const int type);

    // for unit tests
    //! Returns shift key status
    ModifierState shiftStatus() const;

    //! Characters defines word boundaries
    static const QString WordSeparators;

    //! Prepare virtual keyboard for orientation change
    void prepareToOrientationChange();

    //! Finalize orientation change
    void finalizeOrientationChange();

    /*!
     * \brief Returns whether the symbol view is available for current layout.
     */
    bool symViewAvailable() const;

    /*!
     * Switch layout to given direction, \sa MAbstractInputMethod::swipe
     */
    void switchLayout(MInputMethod::SwitchDirection direction, bool enableAnimation);

    //! Set input method mode
    void setInputMethodMode(M::InputMethodMode mode);

    //! Returns whether autocaps is enabled.
    bool autoCapsEnabled() const;

    //! Show notification informing about current language if keyboard is active, otherwise create
    //! pending request.
    void requestLanguageNotification();

    /*!
     * \brief Returns the keys in the main layout.
     */
    QList<MImEngine::KeyboardLayoutKey> mainLayoutKeys() const;

    //! reimp
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    //! reimp_end

    //! \return region occupied by keyboard in scene coordinates
    QRegion region() const;

public slots:
    /*!
     * Method to switch level. Changes into next possible level.
     * Back to zero if maximum reached
     */
    void switchLevel();

    /*!
     * Method to set shift state
     */
    void setShiftState(ModifierState level);

    //! Add vkb widget portions to the reaction map.
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);

    /*!
     * Method to change the orientation
     * \param orientation M::Orientation
     * \param force reorganize even when not visible
     */
    void organizeContent(M::Orientation orientation, bool force = false);

    void setLayout(int layoutIndex);

    /*!
     * This function gets called when the widget show animation is finished
     */
    void showFinished();

    /*!
     * \brief Resets current active key area (release active keys in the main keyboard).
     * \param resetCapsLock whether to reset shift key when in caps-lock mode.
     *        By default, always resets shift key.
     */
    void resetCurrentKeyArea(bool resetCapsLock = false);

private slots:
    /*!
     * \brief Handler for shift pressed state change (separate from shift state).
     */
    void handleShiftPressed(bool shiftPressed);

    /*!
     * Handler for right flick operation
     */
    void flickRightHandler();

    /*!
     * Handler for left flick operation
     */
    void flickLeftHandler();

    /*!
     * \brief Handler for upward flick operation
     * \param binding Key binding
     */
    void flickUpHandler(const MImKeyBinding &binding);

    void keyboardsReset();

    void numberKeyboardReset();

    void onSectionSwitchStarting(int current, int next);

    void onSectionSwitched(QGraphicsWidget *previous, QGraphicsWidget *current);

signals:
    /*!
     * \brief Emitted when key is pressed
     * Note that this happens also when user keeps finger down/mouse
     * button pressed and moves over another key (event is about the new key)
     * \param event key event
     */
    void keyPressed(const KeyEvent &event);

    /*!
     * \brief Emitted when key is released
     * Note that this happens also when user keeps finger down/mouse
     * button pressed and moves over another key (event is about the old key)
     * \param event key event
     */
    void keyReleased(const KeyEvent &event);

    /*!
     * \brief Emitted when user releases mouse button/lifts finger
     * Except when done on a dead key
     * \param event key event
     */
    void keyClicked(const KeyEvent &event);

    /*!
     * \brief Emitted when key is long pressed
     */
    void longKeyPressed(const KeyEvent &event);

    //! This signal is emitted when input layout is changed
    //! \param layout this is always the layout from XML file in unmodified form
    void layoutChanged(const QString &layout);

    //! Emitted when user hides the keyboard, e.g. by pressing the close button
    void userInitiatedHide();

    /*!
     * \brief This signal is emitted when copy/paste button is clicked
     * \param state CopyPasteState button action (copy or paste)
     */
    void copyPasteClicked(CopyPasteState action);

    //! Emitted when shift state is changed
    void shiftLevelChanged();

    //! Emitted when symbol view should be shown
    void showSymbolViewRequested();

    /*!
     * Inform that plugin should be switched in according to \a direction.
     */
    void pluginSwitchRequired(MInputMethod::SwitchDirection direction);

    /*!
     * Inform that active plugin should be replaced with specified one.
     * \param pluginName Name orf plugin which will be activated
     */
    void pluginSwitchRequired(const QString &pluginName);


private:
    void updateMainLayoutAtKeyboardIndex();

    //! Getter for style container
    const MVirtualKeyboardStyleContainer &style() const;

    /*!
     * \brief Maps \a offset to scene coordinate.
     */
    QPoint mapOffsetToScene(QPointF offset);

    const LayoutData *currentLayoutModel() const;

    MImAbstractKeyArea *keyboardWidget(int layoutIndex = -1) const;

    /*!
     * Paint the reactive areas of the buttons
     *
     * This does not include the layout keys.
     * It paints the reactive areas for the buttons
     * in the bottom row of the keyboard as well as
     * the close/minimize/hide button.
     */
    void drawButtonsReactionMaps(MReactionMap *reactionMap, QGraphicsView *view);

    //! creates a switcher for qwerty layouts
    void createSwitcher();

    //! Reloads sections to switcher with current layout type and orientation
    void reloadSwitcherContent();

    //! Creates a new section widget of given layout/layout type and orientation.
    MImAbstractKeyArea *createMainSectionView(const QString &layout,
                                         LayoutData::LayoutType,
                                         M::Orientation orientation,
                                         QGraphicsWidget *parent = 0);

    // creates a new section widget
    MImAbstractKeyArea *createSectionView(const QString &layout,
                                     LayoutData::LayoutType layoutType,
                                     M::Orientation orientation,
                                     const QString &section,
                                     bool usePopup,
                                     QGraphicsWidget *parent = 0);

    // recreates keyboard widgets
    void recreateKeyboards();

    // makes new keybutton areas for number and phone number
    void recreateSpecialKeyboards();

    //! \brief Resets different components of vkb to their initial states.
    void resetState();

    //! Connect signals from a \a handle widget or whatever provides identical flick signals
    template <class T>
    void connectHandle(const T &handleLike);

    //! Show notification informing about current language
    void showLanguageNotification();

private:
    //! Main layout indices
    enum LayoutIndex {
        KeyboardHandleIndex,
        KeyboardIndex
    };

    //! Current Style being used
    const MVirtualKeyboardStyleContainer *styleContainer;

    QGraphicsLinearLayout *mainLayout;

    //! Current layout level
    int currentLevel;

    //! Max number of layout levels
    int numLevels;

    //! Scene manager to get the device width and height
    MSceneManager *sceneManager;

    //! Shift key status
    ModifierState shiftState;

    LayoutData::LayoutType currentLayoutType;

    M::Orientation currentOrientation;

    QString currentLayout;

    const LayoutsManager &layoutsMgr;

    //! Switcher for showing the main qwerty section.
    HorizontalSwitcher *mainKeyboardSwitcher;

    Notification *notification;

    MImAbstractKeyArea *numberKeyboard;
    MImAbstractKeyArea *phoneNumberKeyboard;

    QSharedPointer<QPixmap> backgroundPixmap;

    KeyEventHandler eventHandler;

    //! Contains true if multi-touch is enabled
    bool enableMultiTouch;

    //! Handle area containing toolbar widget.
    // MVirtualKeyboard is not owner of this object.
    QPointer<SharedHandleArea> sharedHandleArea;

    bool pendingNotificationRequest;
    bool transitioning;
};

#endif
