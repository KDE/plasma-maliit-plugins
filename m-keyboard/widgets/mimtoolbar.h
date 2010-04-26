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



#ifndef MIMTOOLBAR_H
#define MIMTOOLBAR_H

#include <MWidget>
#include "widgetbar.h"
#include "mkeyboardcommon.h"

class MReactionMap;
class ToolbarManager;
class MVirtualKeyboardStyleContainer;
class MButton;
class ToolbarWidget;

/*!
  \brief MImToolbar implement the toolbar for virtualkeyboard.

  The MImToolbar class provides interfaces for the usage of the custom
  toolbar for virtualkeyboard. The interfaces include load/unload/hide/show of the toolbar.

  The layout for toolbar is this:
  \code
    |---------------------------------------------------|
    | WidgetBar |      stretch item         | WidgetBar |
    |---------------------------------------------------|
  \endcode
  Depending on the alignment attribute of the toolbar buttons they
  will be placed into left or right side WidgetBar widget. Toolbar
  takes all available space horizontally.
*/
class MImToolbar : public MWidget
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor for creating an virtual keyboard toolbar object.
     * \param style Styling information.
     * \param parent Parent object.
     */
    explicit MImToolbar(MVirtualKeyboardStyleContainer &style,
                        QGraphicsWidget *parent = 0);

    //! Destructor
    ~MImToolbar();

    /*!
     * \brief Returns toolbar's region.
     */
    QRegion region() const;

    /*!
     * \brief Shows a custom toolbar with unique \a id.
     * Loads a custom toolbar according \a id, if successfuly loads,
     * the toolbar will be visible when show().
     * \param id      Unique identifier of the custom toolbar.
     */
    void showToolbarWidget(qlonglong id);

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

    void drawReactiveAreas(MReactionMap *reactionMap, QGraphicsView *view);

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
    void handleButtonClick(const ToolbarWidget &);

    void showGroup(const QString &);

    void hideGroup(const QString &);

    void sendKeySequence(const QString &);

    //! Invoked when copy/paste button is clicked
    void copyPasteButtonHandler();

    void updateVisibility();

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

    void loadCustomWidgets(Qt::Alignment align);

    void unloadCustomWidgets(Qt::Alignment align);

    void updateWidgets(bool customWidgetsChanged = true);

    Qt::KeyboardModifiers keyModifiers(int key) const;

    /*!
     * \brief Inserts item to \a align part of the toolbar at index,
     * or before any item that is currently at index in \a align part of the toolbar.
     *  This doesn't do anything if the item has already been added.
     * \param index Index to be inserted.
     * \param button The MWidget to be added.
     * \param align Indicate which part of the toolbar, Qt::AlignLeft or Qt::AlignRight.
     */
    void insertItem(int index, MWidget *widget, Qt::Alignment align);

    /*!
     * \brief Removes an item from its layout.
     * \param button Button to remove from either side.
     */
    void removeItem(MWidget *widget);

    void updateReactiveAreas();

    void clearReactiveAreas();

    const ToolbarManager &toolbarMgr;
    bool textSelected;
    //! Input Mode indicator button
    MButton *indicator;
    //! Copy/Paste button
    MButton *copyPaste;
    //! Copy/paste button status
    CopyPasteState copyPasteStatus;

    WidgetBar leftBar;  //! Widget to hold left-aligned toolbar widgets
    WidgetBar rightBar; //! Widget to hold right-aligned toolbar widgets

    MVirtualKeyboardStyleContainer &style; //! Styling information

    friend class Ut_MImToolbar;

    ModifierState shiftState;
    ModifierState fnState;
};

#endif
