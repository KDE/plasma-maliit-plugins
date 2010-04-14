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



#ifndef SYMBOLVIEW_H
#define SYMBOLVIEW_H

#include <MButton>
#include <MButtonGroup>
#include <MWidget>

#include "buttonbar.h"
#include "singlewidgetbuttonarea.h"
#include "mbuttonarea.h"

class QGraphicsLinearLayout;
class MSceneManager;
class MVirtualKeyboardStyleContainer;
class HorizontalSwitcher;
class LayoutData;
class LayoutsManager;
class QGraphicsItemAnimation;
class QGraphicsLinearLayout;
class QGraphicsSceneMouseEvent;
class QTimeLine;
class KeyEvent;
class LayoutSection;
class SymIndicatorButton;


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
    SymbolView(const LayoutsManager &layoutsManager, MVirtualKeyboardStyleContainer *,
               const QString &language, QGraphicsWidget *parent = 0);

    /*!
     * Destructor
     */
    virtual ~SymbolView();

    void prepareToOrientationChange();

    void finalizeOrientationChange();

    /*!
    * Method to switch layout when shift button is pressed
    * \param level int 1 for upper case, otherwise 0
    * \param capslock Shift locked state
    */
    void switchLevel(int level, bool capslock);

    //! Returns current level.
    int currentLevel() const;

    //! Save state before rotation
    void save();

    //! Restore state after rotation
    void restore();

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

    /*!
     * \brief Shows the function row.
     */
    void showFunctionRow();

    /*!
     * \brief Hides the function row.
     */
    void hideFunctionRow();

    //! \brief Clears and redraws the global reaction maps.
    void redrawReactionMaps();

signals:
    //! Used to broadcast shift state to all pages/KeyButtonAreas.
    void levelSwitched(int, bool);

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

    //! \brief Method to set up the animation
    void setupShowAndHide();

    //! \brief Updates positions for up/down animation.
    void updateAnimPos(int top, int bottom);

    //! \brief Reloads switcher with pages from given \a layout, selecting the page \a selectPage.
    void loadSwitcherPages(const LayoutData &layout, unsigned int selectPage = 0);

    //! \brief Reloads function row from \a layout and adds the widget to vertical layout.
    void loadFunctionRow(const LayoutData &layout);

    //! \brief Helper method to create and add a new page.
    void addPage(QSharedPointer<const LayoutSection> symbolSection);

    //! \brief Creates KeyButtonArea from given section model and connects appropriate signals.
    KeyButtonArea *createKeyButtonArea(QSharedPointer<const LayoutSection> section,
                                       KeyButtonArea::ButtonSizeScheme sizeScheme = KeyButtonArea::ButtonSizeSymbolView,
                                       bool enablePopup = true);

    //! Getter for style container
    MVirtualKeyboardStyleContainer &style();

    //! Retrieves title of a symbol section from given page.
    QString pageTitle(int pageIndex) const;

    //! \brief Updates the Sym button to visually indicate the current active page.
    void updateSymIndicator();

private:
    //! Current style being used.
    MVirtualKeyboardStyleContainer *styleContainer;

    //! Manage animation
    QGraphicsItemAnimation *showAnimation;
    QGraphicsItemAnimation *hideAnimation;

    //! scene manager
    const MSceneManager &sceneManager;

    //! Current visible KeyButtonArea widget
    KeyButtonArea *selectedLayout;

    //! To check if symbol view is opened
    Activity activity;

    //! Animation related time lines.
    QTimeLine *showTimeLine, *hideTimeLine;

    //! Zero-based index to currently active page
    int activePage;

    //! Case selector: 0 for lower case, 1 for upper case
    int shift;

    const LayoutsManager &layoutsMgr;

    HorizontalSwitcher *pageSwitcher;
    KeyButtonArea *functionRow;

    M::Orientation currentOrientation;

    QString currentLanguage;

    /*! This is true when one of the pages has received mouse
     *  down event. It is used to check whether on page switch
     *  we need to reassign mouse grab.
     */
    bool mouseDownKeyArea;

    //! Vertical layout is the main layout which holds toolbar (not implemented yet) and function row.
    QGraphicsLinearLayout &verticalLayout;

    //! This layout holds symbol characters and function row.
    QGraphicsLinearLayout &keyAreaLayout;

#ifdef UNIT_TEST
    friend class Ut_SymbolView;
#endif
};

#endif
