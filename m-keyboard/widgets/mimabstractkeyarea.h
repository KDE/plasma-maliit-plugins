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



#ifndef MIMABSTRACTKEYAREA_H
#define MIMABSTRACTKEYAREA_H

#include "mimabstractkey.h"
#include "mimabstractkeyareastyle.h"
#include "keyevent.h"
#include "layoutdata.h"
#include "mimkeymodel.h"
#include "mkeyboardcommon.h"

#include <MStylableWidget>
#include <QColor>
#include <QHash>
#include <QList>
#include <QStringList>
#include <QTouchEvent>
#include <QTimer>
#include <QTime>

class FlickGesture;
class MFeedbackPlayer;
class MReactionMap;
class MScalableImage;
class MVirtualKeyboardStyleContainer;
class LayoutData;
class QGraphicsLinearLayout;
class QTextLayout;
class PopupHost;
class PopupBase;

/*!
 * \class MImAbstractKeyArea
 * \brief MImAbstractKeyArea is a view for virtual keyboard layout represented by LayoutModel
 */
class MImAbstractKeyArea
    : public MStylableWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(MImAbstractKeyArea)

public:
    /*!
    * \brief Constructor
    * \param section A section that this MImAbstractKeyArea visualizes.
    * \param usePopup Sets whether popup should be used when long press occurs.
    * \param parent The widget's parent.
    */
    explicit MImAbstractKeyArea(const LayoutData::SharedLayoutSection &section,
                                bool usePopup = false,
                                QGraphicsWidget *parent = 0);

    //! \brief Destructor
    virtual ~MImAbstractKeyArea();

    //! \return layout model
    const LayoutData::SharedLayoutSection &sectionModel() const;

    //! Returns current level of this layout.
    int level() const;

    //! Expose style used by MImAbstractKeyArea.
    const MImAbstractKeyAreaStyleContainer &baseStyle() const;

    //! Set input method mode for all MImAbstractKeyArea instances
    static void setInputMethodMode(M::InputMethodMode inputMethodMode);

    //! Returns relative button base width
    qreal relativeButtonBaseWidth() const;

    //! Returns all keys.
    virtual QList<const MImAbstractKey *> keys() = 0;

public slots:
    /*!
     * This slot is used to switch levels
     */
    void switchLevel(int level);

    virtual void setShiftState(ModifierState newShiftState);

    virtual void drawReactiveAreas(MReactionMap *reactionMap, QGraphicsView *view);

    /*!
     * \brief unlock all locked deadkeys
     */
    void unlockDeadkeys();

signals:
    //! \brief Emitted when the covered region changed
    //! \param region The changed region
    void regionUpdated(const QRegion &region);

    /*!
     * \brief Emitted when key is pressed
     * Note that this happens also when user keeps finger down/mouse
     * button pressed and moves over another key (event is about the new key)
     * \param key describes pressed button
     * \param accent label of pressed dead key if any
     * \param upperCase contains true if key is in uppercase state
     */
    void keyPressed(const MImAbstractKey *key, const QString &accent, bool upperCase);

    /*!
     * \brief Emitted when key is released
     * Note that this happens also when user keeps finger down/mouse
     * button pressed and moves over another key (event is about the old key)
     * \param key describes released button
     * \param accent label of pressed dead key if any
     * \param upperCase contains true if key is in uppercase state
     */
    void keyReleased(const MImAbstractKey *key, const QString &accent, bool upperCase);

    /*!
     * \brief Emitted when user releases mouse button/lifts finger
     * Except when done on a dead key
     * \param key describes clicked button
     * \param accent label of pressed dead key if any
     * \param upperCase contains true if key is in uppercase state
     * \param touchPoint the touch point for the key
     */
    void keyClicked(const MImAbstractKey *key, const QString &accent, bool upperCase, const QPoint &touchPoint);

    /*!
     * \brief Emitted when long press is detected.
     * Long press detection is:
     * - cancelled when latest pressed key is released;
     * - restarted when finger is moved to other key;
     * - restarted when new touch point is recognized by MImAbstractKeyArea.
     *
     * \param key describes pressed button.
     * \param accent Active accent in MImAbstractKeyArea at the time of long press occured.
     * \param upperCase Upper case state in MImAbstractKeyArea at the time of long press occured.
     */
    void longKeyPressed(const MImAbstractKey *key, const QString &accent, bool upperCase);

    //! Emitted when flicked right
    void flickRight();

    //! Emitted when flicked left
    void flickLeft();

    //! Emitted when flicked down
    void flickDown();

    //! \brief Emitted when flicked up
    //! \param binding Information about the key where mouse button was pressed
    void flickUp(const MImKeyBinding &binding);

    //! \brief Emitted if button width has changed
    //! \param baseWidth base width used for relative button widths
    void relativeButtonBaseWidthChanged(qreal baseWidth);

protected:
    /*! \reimp */
    virtual void resizeEvent(QGraphicsSceneResizeEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual QVariant itemChange(GraphicsItemChange change,
                                const QVariant &value);

    virtual void grabMouseEvent(QEvent *event);
    virtual void ungrabMouseEvent(QEvent *event);
    virtual bool event(QEvent *event);
    /*! \reimp_end */

    //! Called when widget is about to lose visibility.
    virtual void handleVisibilityChanged(bool visible);

    //! Shows popup and updates its content and position.
    //! \param key current key
    void updatePopup(MImAbstractKey *key = 0);

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

    //! \brief Updates button labels and/or icons according to current level and deadkey.
    void updateButtonModifiers();

    /*! \brief Notification for derived classes about button modifier change.
     *
     *  Derived classes should not change the level of selected dead keys. This is to
     *  ensure all dead keys can be used with all characters in every level.
     */
    virtual void modifiersChanged(bool shift, QChar accent = QChar());

    //! \brief Returns key at given \a pos.
    //!
    //! Accepts positions outside widget geometry because
    //! of reactive margins.
    virtual MImAbstractKey *keyAt(const QPoint &pos) const = 0;

    /*! \brief Calculates button and row geometry based on given \a availableWidth.
     *  \post Button rectangle cache and row width cache are up to date.
     */
    virtual void updateButtonGeometriesForWidth(int availableWidth) = 0;

    const PopupBase &popup() const;

    void printTouchPoint(const QTouchEvent::TouchPoint &tp,
                         const MImAbstractKey *key,
                         const MImAbstractKey *lastKey = 0) const;

    //! Sets button state and sends release & press events.
    //void setActiveKey(MImAbstractKey *key, TouchPointInfo &tpi);

    //! Relative button base width in currently active layout
    qreal mRelativeButtonBaseWidth;

    bool debugTouchPoints;

protected slots:
    //! Update background images, text layouts, etc. when the theme changed.
    virtual void onThemeChangeCompleted();

    //! Handle long press on the key
    virtual void handleLongKeyPressed();

    //! Handle idle VKB
    virtual void handleIdleVkb();

private:
    //! \brief Handler for flick gestures from Qt gesture framework.
    void handleFlickGesture(FlickGesture *gesture);

    //! \brief Touch point press handler.
    //! \param tp The unprocessed Qt touchpoint.
    void touchPointPressed(const QTouchEvent::TouchPoint &tp);

    //! \brief Touch point move handler.
    //! \param tp The unprocessed Qt touchpoint.
    void touchPointMoved(const QTouchEvent::TouchPoint &tp);

    //! \brief Touch point release handler.
    //! \param tp The unprocessed Qt touchpoint.
    void touchPointReleased(const QTouchEvent::TouchPoint &tp);

    static QTouchEvent::TouchPoint createTouchPoint(int id,
                                                    Qt::TouchPointState state,
                                                    const QPointF &pos,
                                                    const QPointF &lastPos);
    struct GravitationalLookupResult
    {
        GravitationalLookupResult(MImAbstractKey *newKey,
                                  MImAbstractKey *newLastKey)
            : key(newKey)
            , lastKey(newLastKey)
        {}

        MImAbstractKey *key;
        MImAbstractKey *lastKey;
    };

    //! \brief Gravitational key lookup
    //! \param mappedPos Current position of touchpoint,
    //!        mapped to widget space.
    //! \param mappedLastPos Last position of touchpoint,
    //!        mapped to widget space.
    //! \param mappedStartPos Start position of touchpoint,
    //!        mapped to widget space.
    //! \returns adjusted key and last key, using \a keyAt.
    GravitationalLookupResult gravitationalKeyAt(const QPoint &mappedPos,
                                                 const QPoint &mappedLastPos,
                                                 const QPoint &mappedStartPos) const;

    void click(MImAbstractKey *key, const QPoint &touchPoint = QPoint());

    //! Checks for speed typing mode
    //! \warning Not side-effect free when \a restartTimers is actively used.
    bool isInSpeedTypingMode(bool restartTimers = false);

    //! Current level
    int currentLevel;

    //! Popup to show additional information for a button
    PopupBase *mPopup;

    //! List of punctuation labels
    QList<QStringList> punctuationsLabels;

    //! List of accent labels
    QList<QStringList> accentLabels;

    //! Whether a gesture was already triggered for any active touch point.
    bool wasGestureTriggered;
    bool enableMultiTouch;

    //! Active key, there can only be one at a time.
    MImAbstractKey *activeKey;

    //! Activated dead key
    MImAbstractKey *activeDeadkey;

    //! Activated shift key
    MImAbstractKey *activeShiftKey;

    /*!
     * Feedback player instance
     */
    MFeedbackPlayer *feedbackPlayer;

    //! layout section viewed by this class
    const LayoutData::SharedLayoutSection section;

    static M::InputMethodMode InputMethodMode;

    //! This timer is used to recognize long press.
    QTimer longPressTimer;


    //! Whenever the VKB idles, gestures are activated.
    QTimer idleVkbTimer;

    //! Used to measure elapsed time between two touchpoint press events:
    QTime lastTouchPointPressEvent;

    M_STYLABLE_WIDGET(MImAbstractKeyAreaStyle)

#ifdef UNIT_TEST
    friend class MReactionMapTester;
    friend class Ut_MImAbstractKeyArea;
    friend class Ut_SymbolView;
    friend class Ut_MVirtualKeyboard;
    friend class Bm_MImAbstractKeyArea; //benchmarks
#endif
};

#endif
