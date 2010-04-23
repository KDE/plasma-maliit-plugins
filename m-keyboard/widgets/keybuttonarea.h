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



#ifndef KEYBUTTONAREA_H
#define KEYBUTTONAREA_H

#include "ikeybutton.h"
#include "keyevent.h"
#include "layoutdata.h"
#include "vkbdatakey.h"

#include <MWidget>
#include <QColor>
#include <QHash>
#include <QList>
#include <QStringList>

class MFeedbackPlayer;
class MReactionMap;
class MScalableImage;
class MVirtualKeyboardStyleContainer;
class ISymIndicator;
class LayoutData;
class LimitedTimer;
class QGraphicsLinearLayout;
class QTextLayout;
class PopupBase;

/*!
 * \class KeyButtonArea
 * \brief KeyButtonArea is a view for virtual keyboard layout represented by LayoutModel
 */
class KeyButtonArea : public MWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(KeyButtonArea)

public:
    //! Button size scheme tells which button sizes to use
    //! and how to calculate them.
    enum ButtonSizeScheme {
        //! Equal button width, expanding to available width. This is the default.
        ButtonSizeEqualExpanding,
        //! Same as above, but uses sizes intended for phone number.
        ButtonSizeEqualExpandingPhoneNumber,
        //! Common size scheme for function rows.
        ButtonSizeFunctionRow,
        //! Number function rows have their own button sizes.
        ButtonSizeFunctionRowNumber,
        //! Symbol view has its own button sizes for portrait mode.
        ButtonSizeSymbolView
    };

    /*!
    * \brief Constructor
    * \param section A section model that this KeyButtonArea visualizes.
    * \param buttonSizeScheme Defines which size attributes to use for buttons.
    * \param usePopup Sets whether popup should be used when long press occurs.
    * \param parent The widget's parent.
    */
    KeyButtonArea(MVirtualKeyboardStyleContainer *,
                  QSharedPointer<const LayoutSection> section,
                  ButtonSizeScheme buttonSizeScheme = ButtonSizeEqualExpanding,
                  bool usePopup = false,
                  QGraphicsWidget *parent = 0);

    //! \brief Destructor
    virtual ~KeyButtonArea();

    /*!
     * \brief Stop the accurate mode.
    */
    void accurateStop();

    /*!
    * \brief Return true if accurate mode is set, vice versa.
    * \return bool
    */
    bool isAccurateMode() const;

    /*!
     * \brief Checks whether popup is currently turned on.
     */
    bool isPopupActive() const;

    //! \return layout model
    QSharedPointer<const LayoutSection> sectionModel() const;

    //! Allows sym indicator to be controlled from outside.
    virtual ISymIndicator *symIndicator();

public slots:
    /*!
     * This slot is used to switch levels
     */
    void switchLevel(int level, bool capslock);

    /*!
     * \brief Shows popup
     */
    void popupStart();

    /*!
    * \brief This slot starts the accurate mode
    */
    void accurateStart();

    /*!
    * \brief Return true if the keyboard is flicked to left/right/down
    */
    bool flickCheck();

    virtual void drawReactiveAreas(MReactionMap *reactionMap, QGraphicsView *view);

    /*!
     * \brief unlock all locked deadkeys
     */
    void unlockDeadkeys();

signals:

    //! Emitted when accurate mode is started
    void accurateModeStarted();

    //! Emitted when accurate mode is stopped
    void accurateModeStopped();

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

    //! Emitted when flicked right
    void flickRight();

    //! Emitted when flicked left
    void flickLeft();

    //! Emitted when flicked down
    void flickDown();

    /*!
     * \brief Emitted when flicked up
     * \param binding Information about the key where mouse button was pressed
     */
    void flickUp(const KeyBinding *binding);

protected:
    //! \brief Returns data and size information of a button in given \a row and \a column.
    void buttonInformation(int row, int column,
                           const VKBDataKey *&dataKey,
                           QSize &size,
                           bool &stretchesHorizontally);

    /*! \reimp */
    virtual void resizeEvent(QGraphicsSceneResizeEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual QVariant itemChange(GraphicsItemChange, const QVariant &);
    virtual void grabMouseEvent(QEvent *e);
    virtual void ungrabMouseEvent(QEvent *e);
    /*! \reimp_end */

    //! Called when widget is about to lose visibility.
    virtual void onHide();


    //! Updates popup's content and position.
    void updatePopup(const QPoint &pos, const IKeyButton *key = 0);

    /*!
     * \brief click at deadKey
     * \param key pointer to deadkey
     */
    void clickAtDeadkey(const IKeyButton *key);

    /*!
    * \brief Verifies should we process cursor movement or not
    * \param pos QPointF new cursor position
    * \return bool
    */
    bool isObservableMove(const QPointF &prevPos, const QPointF &pos);

    /*!
    * \brief Get level count of the virtual keyboard.
    * \return int. The level count.
    */
    int maxColumns() const;

    /*!
    * \brief Get row count of the virtual keyboard, in current level.
    * \return int. The row count.
    */
    int rowCount() const;

    //! \brief Returns the correct size for button with given \a action.
    QSize buttonSizeByAction(KeyBinding::KeyAction action) const;

    //! \brief Updates button labels and/or icons according to current level and deadkey.
    void updateButtonModifiers();

    //! \brief Notification for derived classes about button modifier change.
    virtual void modifiersChanged(bool shift, QChar accent = QChar());

    //! \brief Returns key at given \a pos.
    //!
    //! Accepts positions outside widget geometry because
    //! of reactive margins.
    virtual IKeyButton *keyAt(const QPoint &pos) const = 0;

    /*! \brief Calculates button and row geometry based on given \a availableWidth.
     *  \post Button rectangle cache and row width cache are up to date.
     */
    virtual void updateButtonGeometries(int availableWidth, int equalButtonWidth) = 0;

    //! \brief Getter for style container
    const MVirtualKeyboardStyleContainer &style() const;

    const PopupBase &popupWidget() const;

    //! Sets button state and sends release & press events.
    void setActiveKey(IKeyButton *key);

    //! Derived classes must set this if they have shift button and want it be locked.
    IKeyButton *shiftButton;

private:
    //! Turn key button into a KeyEvent, considering current dead key and modifier state
    KeyEvent keyToKeyEvent(const IKeyButton &key, QKeyEvent::Type eventType) const;

    //! Check whether given character will stop accurate mode.
    void accurateCheckContent(const QString &content);

    //! Current level
    int currentLevel;

    //! Popup to show additional information for a button
    PopupBase *popup;

    //! style
    MVirtualKeyboardStyleContainer *styleContainer;

    //! Flicktimer
    LimitedTimer *flickTimer;

    //! Floating point flickStart position
    QPointF flickStartPos;

    //! Current pointer position
    QPoint pointerPos;

    //! True if finger is held down and is inside the layout's area.
    bool fingerInsideArea;

    //! Timer to detect long mouse press.
    LimitedTimer *longPressTimer;

    //! Accurate mode enabled/disabled
    bool accurateMode;

    //! List of punctuation labels
    QList<QStringList> punctuationsLabels;

    //! List of accent labels
    QList<QStringList> accentLabels;

    //! Contains true if keyboard was flicked after last mouse press
    bool flicked;

    //! Activated dead key
    const IKeyButton *activeDeadkey;

    //! Currently held down key
    IKeyButton *activeKey;

    //! Key that was pressed when flick gesture began.
    const IKeyButton *flickedKey;

    /*!
     * Feedback player instance
     */
    MFeedbackPlayer *feedbackPlayer;

    //! layout section viewed by this class
    QSharedPointer<const LayoutSection> section;

    const ButtonSizeScheme buttonSizeScheme;

    const bool usePopup;

#ifdef UNIT_TEST
    friend class MReactionMapTester;
    friend class Ut_KeyButtonArea;
    friend class Ut_SymbolView;
    friend class Bm_KeyButtonArea; //benchmarks
#endif
};

/*!
 * This provides interface to change sym indicator status.
 */
class ISymIndicator
{
public:
	virtual void activateSymIndicator() = 0;
	virtual void activateAceIndicator() = 0;
	virtual void deactivateIndicator() = 0;
};

#endif
