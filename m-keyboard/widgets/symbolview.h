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

#ifndef SYMBOLVIEW_H
#define SYMBOLVIEW_H

#include "singlewidgetbuttonarea.h"
#include "keyeventhandler.h"

#include <MButton>
#include <MButtonGroup>
#include <mimhandlerstate.h>
#include <MWidget>

#include <QPointer>
#include <QSharedPointer>
#include <QTimeLine>
#include <QGraphicsItemAnimation>
#include <QGraphicsLinearLayout>

class MSceneManager;
class MVirtualKeyboardStyleContainer;
class HorizontalSwitcher;
class LayoutData;
class LayoutsManager;
class QGraphicsLinearLayout;
class QGraphicsSceneMouseEvent;
class KeyEvent;
class LayoutSection;
class SymIndicatorButton;
class Handle;
class SharedHandleArea;

/*!
 * \brief SymbolView is used to show different layouts symbols/upper case/lower case
 */
class SymbolView : public MWidget
{
    Q_OBJECT

public:
    enum ShowMode {
        NormalShowMode,         // Shows page normally
        FollowMouseShowMode     // Shows page temporarily and grabs the mouse
    };

    enum HideMode {
        NormalHideMode,         // Hides page normally
        TemporaryHideMode       // Hides page temporarily, page is hidden temporarily during screen rotation.
    };

    /*!
     * \brief Constructor for creating an SymbolView object.
     * \param baseSize QSize
     * \param parent Parent object.
     */
    SymbolView(const LayoutsManager &layoutsManager, const MVirtualKeyboardStyleContainer *,
               const QString &language, QGraphicsWidget *parent = 0);

    /*!
     * Destructor
     */
    virtual ~SymbolView();

    void prepareToOrientationChange();

    void finalizeOrientationChange();

    void setShiftState(ModifierState newShiftState);

    //! Returns current level.
    int currentLevel() const;

    //! \brief Tells whether sym view is fully opened.
    bool isFullyVisible() const;

    //! Return TRUE if widget is activated
    bool isActive() const;

    void stopAccurateMode();

    /*! Returns interactive region currently occupied by SymbolView,
     *  in scene coordinates.
     */
    QRegion interactiveRegion() const;

    //! Returns the page count.
    int pageCount() const;

    //! Returns the index of current page.
    int currentPage() const;

    //! Set pointer to handle area
    void setSharedHandleArea(const QPointer<SharedHandleArea> &newSharedHandleArea);

public slots:
    /*!
     * Handler to show the page.
     * If the page is already visible, then just return.
     * \param mode Value is NormalShowMode or FollowMouseShowMode. Default is NormalShowMode. \sa ShowMode.
     */
    void showSymbolView(ShowMode mode = NormalShowMode);

    /*!
     * Handler to hide the page
     * \param mode Value is NormalHideMode or TemporaryHideMode. Default is NormalHideMode. \sa HideMode.
     */
    void hideSymbolView(HideMode mode = NormalHideMode);

    /*!
     * Changes between onscreen and hardware keyboard modes. Symbol view's contents depend on which one is used.
     * When hardware keyboard is used, symbol view has hw layout specific layout variant, and function row is hidden.
     * \param state New state to be applied.
     */
    void setKeyboardState(MIMHandlerState state);

    /*!
     * Method to load svc corresponding to given language
     * \param lang const QString&, new language
     */
    void setLanguage(const QString &lang);

    /*!
     * \brief Selects the next page if current is not the rightmost.
     */
    void switchToNextPage();

    /*!
     * \brief Selects the previous page if current is not the leftmost.
     */
    void switchToPrevPage();

    //! \brief Clears and redraws the global reaction maps.
    void redrawReactionMaps();

signals:
    //! Used to broadcast shift state to all pages/KeyButtonAreas.
    void levelSwitched(int);

    /*!
     * Emitted on clicked on layout
     */
    void keyClicked(const KeyEvent &event);

    /*!
     * Emitted when key is pressed
     */
    void keyPressed(const KeyEvent &event);

    /*!
     * Emitted when key is released
     */
    void keyReleased(const KeyEvent &event);

    //! Emitted when SymbolView has changed its interactive region.
    void regionUpdated(const QRegion &);

    //! Emitted when hiding finished.
    void hidden();

    //! Emitted when fully visible.
    void opened();

    //! SymbolView will start to show up
    void showingUp();

protected:
    /*! \reimp */
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);
    virtual void resizeEvent(QGraphicsSceneResizeEvent *event);
    /*! \reimp_end */

private slots:
    //! changes to given page
    void changePage(int);

    void onReady();
    void onHidden();
    void onSwitchStarting(QGraphicsWidget *current, QGraphicsWidget *next);
    void switchDone();

    /*!
     * \brief Switch function row to upper/lower case
     * according to given parameter
     */
    void setFunctionRowState(bool shiftPressed);

    //! When hardware keyboard layout has changed, reload contents if currently in Hardware state.
    void handleHwLayoutChange();

private:
    //! symbol view state wrt. \a showSymbolView / \a hideSymbolView calls.
    enum Activity {
        Active,                 // After showSymbolView(NormalShowMode) call
        TemporarilyActive,      // After showSymbolView(FollowMouseShowMode) call
        Inactive,               // After hideSymbolView(NormalHideMode) calls
        TemporarilyInactive     // After hideSymbolView(TemporaryHideMode)
    };

    //! Ensure that content, position and size are correct for the current orientation
    void organizeContent();

    //! \brief Set symbol view's position, using \a height as the height of symbol view
    void reposition(int height);

    //! \brief Creates necessary widget layouts where keys will be added.
    void setupLayout();

    //! \brief Reloads keys/buttons based on current language and orientation.
    void reloadContent();

    //! \brief Reloads switcher with pages from given \a layout, selecting the page \a selectPage.
    void loadSwitcherPages(const LayoutData *layout, unsigned int selectPage = 0);

    //! \brief Reloads function row from \a layout and adds the widget to vertical layout.
    void loadFunctionRow(const LayoutData *layout);

    //! \brief Helper method to create and add a new page.
    void addPage(QSharedPointer<const LayoutSection> symbolSection);

    //! \brief Creates KeyButtonArea from given section model and connects appropriate signals.
    KeyButtonArea *createKeyButtonArea(QSharedPointer<const LayoutSection> section,
                                       KeyButtonArea::ButtonSizeScheme sizeScheme = KeyButtonArea::ButtonSizeSymbolView,
                                       bool enablePopup = true);

    //! Getter for style container
    const MVirtualKeyboardStyleContainer &style() const;

    //! Retrieves title of a symbol section from given page.
    QString pageTitle(int pageIndex) const;

    //! \brief Updates the Sym button to visually indicate the current active page.
    void updateSymIndicator();

    //! Connect signals from a \a handle widget
    void connectHandle(Handle *handle);

    //! Current style being used.
    const MVirtualKeyboardStyleContainer *styleContainer;

    //! Helper class for animations.
    class AnimationGroup
    {
    public:
        explicit AnimationGroup(SymbolView *view);
        virtual ~AnimationGroup();

        //! \brief Updates positions for up/down animation.
        void updatePos(int top, int bottom);

        void playShowAnimation();
        void playHideAnimation();

        bool hasOngoingAnimations() const;

    private:
        void takeOverFromTimeLine(QTimeLine *target,
                                  QTimeLine *origin);

        //! Animation related time lines.
        QTimeLine showTimeLine;
        QTimeLine hideTimeLine;

        //! Manage animation
        QGraphicsItemAnimation showAnimation;
        QGraphicsItemAnimation hideAnimation;
    };

    AnimationGroup anim;

    //! scene manager
    const MSceneManager &sceneManager;

    //! Current visible KeyButtonArea widget
    KeyButtonArea *selectedLayout;

    //! To check if symbol view is opened
    Activity activity;


    //! Zero-based index to currently active page
    int activePage;

    //! Case selector: 0 for lower case, 1 for upper case
    ModifierState shift;

    const LayoutsManager &layoutsMgr;

    QPointer<HorizontalSwitcher> pageSwitcher;
    QPointer<KeyButtonArea> functionRow;

    M::Orientation currentOrientation;

    QString currentLanguage;

    /*! This is true when one of the pages has received mouse
     *  down event. It is used to check whether on page switch
     *  we need to reassign mouse grab.
     */
    bool mouseDownKeyArea;

    //! Helper class for linear layouts that allows to wrap them in QPointers.
    class LinearLayoutObject: public QObject, public QGraphicsLinearLayout
    {
    public:
        LinearLayoutObject(Qt::Orientation orientation, QGraphicsLayoutItem *parent = 0);
    };

    //! Vertical layout is the main layout which holds toolbar and function row.
    QPointer<LinearLayoutObject> verticalLayout;

    //! This layout holds symbol characters and function row.
    QPointer<LinearLayoutObject> keyAreaLayout;

    //! Handler for button events
    KeyEventHandler eventHandler;

    //! Contains true if multi-touch is enabled
    bool enableMultiTouch;

    //! Shared handle area
    //Symbol view is not owner of this object.
    QPointer<SharedHandleArea> sharedHandleArea;

    MIMHandlerState activeState;

#ifdef UNIT_TEST
    friend class Ut_SymbolView;
#endif
};

#endif

