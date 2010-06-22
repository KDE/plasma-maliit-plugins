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
#include <minputmethodnamespace.h>

#include <QPointer>
#include <QSharedPointer>

class MReactionMap;
class ToolbarManager;
class MVirtualKeyboardStyleContainer;
class MToolbarButton;
class MToolbarItem;
class ToolbarWidget;
class MToolbarData;
class MToolbarItem;

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
    explicit MImToolbar(const MVirtualKeyboardStyleContainer &style,
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
    void showToolbarWidget(QSharedPointer<const MToolbarData> toolbar);

    /*!
     * \brief Hides all custom toolbars, this also means they are removed from visible virtual keyboard.
     */
    void hideToolbarWidget();

    //! \brief Clears and redraws the toolbar reaction maps.
    void redrawReactionMaps();

    //! \reimp
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *);
    //! \reimp_end

    //! Reload toolbar if it should be updated
    void reload();

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


private slots:
    //! Invoked when custom button on toolbar is clicked
    void handleButtonClick(MToolbarItem *item);

    //! Show all widgets belong to given \a group
    void showGroup(const QString &group);

    //! Hide all widgets belong to given \a group
    void hideGroup(const QString &group);

    //! Send key events corresponding to given \a keys.
    void sendKeySequence(const QString &keys);

    //! Invoked when copy/paste button is clicked
    void copyPasteButtonHandler();

    //! Show or hide buttons according to toolbar definition and text tselection status
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

    //! \brief Emitted when toolbar availability changes
    //! \param available true when there are widgets in the toolbar; false otherwise
    void availabilityChanged(bool available);

private:
    void setupLayout();

    void loadDefaultButtons();

    void updateRegion();

    void loadCustomWidgets();

    void unloadCustomWidgets();

    void arrangeWidgets();

    Qt::KeyboardModifiers keyModifiers(int key) const;

    void setupRowLayout(QGraphicsLinearLayout *rowLayout, WidgetBar *leftBar,
                        WidgetBar *rightBar);

    void createAndAppendWidget(QSharedPointer<MToolbarItem> item, WidgetBar *leftWidget,
                               WidgetBar *rightWidget);

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

    bool textSelected;

    //! ToolbarItem for Copy/Paste button.
    // Don't call any method belongs to MToolbarItem which attributes are managed internally.
    QSharedPointer<MToolbarItem> copyPasteItem;

    //! Copy/Paste button
    MToolbarButton *copyPaste;
    //! Copy/paste button status
    CopyPasteState copyPasteStatus;

    WidgetBar leftBar;  //! Widget to hold left-aligned toolbar widgets
    WidgetBar rightBar; //! Widget to hold right-aligned toolbar widgets

    QSharedPointer<const MToolbarData> currentToolbar; //! Pointer to definition of current toolbar

    QList<QPointer<MWidget> > customWidgets; //! All custom widgets in this toolbar

    const MVirtualKeyboardStyleContainer &style; //! Styling information

    friend class Ut_MImToolbar;

    ModifierState shiftState;
    ModifierState fnState;
};

#endif
