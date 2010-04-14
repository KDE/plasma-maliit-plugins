/* * This file is part of dui-keyboard *
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



#ifndef DUIIMTOOLBAR_H
#define DUIIMTOOLBAR_H

#include <DuiWidget>
#include "buttonbar.h"
#include "duikeyboardcommon.h"

class DuiReactionMap;
class ToolbarManager;
class DuiInfoBanner;
class DuiVirtualKeyboardStyleContainer;
class QTimer;

/*!
  \brief DuiImToolbar implement the toolbar for virtualkeyboard.

  The DuiImToolbar class provides interfaces for the usage of the custom
  toolbar for virtualkeyboard. The interfaces include load/unload/hide/show of the toolbar.

  The layout for toolbar is this:
  \code
    |---------------------------------------------------|
    | ButtonBar |      stretch item         | ButtonBar |
    |---------------------------------------------------|
  \endcode
  Depending on the alignment attribute of the toolbar buttons they
  will be placed into left or right side ButtonBar widget. Toolbar
  takes all available space horizontally.
*/
class DuiImToolbar : public DuiWidget
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor for creating an virtual keyboard toolbar object.
     * \param style Styling information.
     * \param parent Parent object.
     */
    explicit DuiImToolbar(DuiVirtualKeyboardStyleContainer &style,
                          QGraphicsWidget *parent = 0);

    //! Destructor
    ~DuiImToolbar();

    /*!
     * \brief Returns toolbar's region.
     */
    QRegion region() const;

    /*!
     * \brief Shows a custom toolbar with \a name.
     * Loads a custom toolbar according \a name, if successfuly loads,
     * the toolbar will be visible when show().
     * \param name      Name of the custom toolbar.
     */
    void showToolbarWidget(const QString &name);

    /*!
     * \brief Hides all custom toolbars, this also means they are removed from visible virtual keyboard.
     */
    void hideToolbarWidget();

    /*!
     * \brief Hide indicator button.
     * \sa showIndicatorButton().
     */
    void hideIndicatorButton();

    /*!
     * \brief Show indicator button.
     * The input mode indicator button shows the modifier state (off, on, locked) and deadkey state base on
     * current system display language. It default shows "abc" label when no modifier/deadkey is actived
     * for latin display language. And when one mode is changed (to latched or locked), or deadkey is on,
     * or display language is changed, the indicator button will changed its label to show the corresponding
     * state.
     * \sa setIndicatorButtonState().
     */
    void showIndicatorButton();

    void drawReactiveAreas(DuiReactionMap *reactionMap, QGraphicsView *view);

    //! \reimp
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *);
    //! \reimp_end

public slots:
    /*!
     * \brief Sets the status if there are some selection text.
     */
    void setSelectionStatus(bool);

    /*!
     * \brief Set copy/paste button state: hide it, show copy or show paste
     *  \param copyAvailable TRUE if text is selected
     *  \param pasteAvailable TRUE if clipboard content is not empty
     */
    void setCopyPasteButton(bool copyAvailable, bool pasteAvailable);

    /*!
     * \brief Sets indicator button state according \a modifier and its \a state.
     * This method will update the indicator button's label to show the \a state for \a modifier.
     *  \param modifier \sa Qt::KeyboardModifier
     *  \param state \sa ModifierState
     */
    void setIndicatorButtonState(Qt::KeyboardModifier modifier, ModifierState state);

private slots:
    void handleButtonClick();

    void showGroup(const QString &);

    void hideGroup(const QString &);

    void sendKeySequence(const QString &);

    //! Invoked when copy/paste button is clicked
    void copyPasteButtonHandler();

    void updateVisibility();

    //! hide CapsLock infobanner
    void hideLockOnInfoBanner();

signals:
    //! Emitted when toolbar's region changed
    void regionUpdated();

    //! Emitted when require a copy/paste action
    void copyPasteRequest(CopyPasteState);

    //! Emitted when require sending a keyevent
    void sendKeyEventRequest(const QKeyEvent &);

    //! Emitted when require sending a string
    void sendStringRequest(const QString &);

    /*!
     * \brief This signal is emitted when copy/paste button is clicked
     * \param state CopyPasteState button action (copy or paste)
     */
    void copyPasteClicked(CopyPasteState action);

    /*!
     * \brief This signal is emitted when indicator button is clicked
     */
    void indicatorClicked();

private:
    void setupLayout();

    void loadDefaultButtons();

    void updateRegion();

    void loadCustomButtons(Qt::Alignment align);

    void unloadCustomButtons(Qt::Alignment align);

    void updateButtons(bool customButtonsChanged = true);

    Qt::KeyboardModifiers keyModifiers(int key) const;

    /*!
     * \brief Inserts item to \a align part of the toolbar at index,
     * or before any item that is currently at index in \a align part of the toolbar.
     *  This doesn't do anything if the item has already been added.
     * \param index Index to be inserted.
     * \param button The DuiButton to be added.
     * \param align Indicate which part of the toolbar, Qt::AlignLeft or Qt::AlignRight.
     */
    void insertItem(int index, DuiButton *button, Qt::Alignment align);

    /*!
     * \brief Removes an item from its layout.
     * \param button Button to remove from either side.
     */
    void removeItem(DuiButton *button);

    void updateReactiveAreas();

    void clearReactiveAreas();

    ToolbarManager *toolbarMgr;
    bool textSelected;
    //! Input Mode indicator button
    DuiButton *indicator;
    //! Copy/Paste button
    DuiButton *copyPaste;
    //! Copy/paste button status
    CopyPasteState copyPasteStatus;

    ButtonBar leftBar;  //! Widget to hold left-aligned toolbar buttons
    ButtonBar rightBar; //! Widget to hold right-aligned toolbar buttons

    DuiInfoBanner *modifierLockOnInfoBanner; //! widget to show modifier is in locked state
    QTimer *modifierLockOnTimer;

    DuiVirtualKeyboardStyleContainer &style; //! Styling information

    friend class Ut_DuiImToolbar;
};

#endif
