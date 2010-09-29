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

#include <MStylableWidget>
#include "widgetbar.h"
#include "mkeyboardcommon.h"
#include "mimtoolbarstyle.h"

#include <QPointer>
#include <QSharedPointer>

class MReactionMap;
class MToolbarButton;
class MToolbarItem;
class MToolbarData;

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
class MImToolbar : public MStylableWidget
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor for creating an virtual keyboard toolbar object.
     * \param style Styling information.
     * \param parent Parent object.
     */
    explicit MImToolbar(QGraphicsWidget *parent = 0);

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

    //! \brief Adds toolbar portions to the reaction maps.
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);

    /*!
     * \brief Update toolbar after orientation change.
     * This method does not emit following signals:
     * 1)regionUpdated, because it should be emitted by other classes;
     * 2)availabilityChanged, because toolbar status is not changed by rotation.
     */
    void finalizeOrientationChange();

    //! \reimp
    virtual QRectF boundingRect() const;
    //! \reimp_end

public slots:
    /*!
     * \brief Sets the status if there are some selection text.
     */
    void setSelectionStatus(bool);


private slots:
    //! Invoked when custom button on toolbar is clicked
    void handleButtonClick(MToolbarItem *item);

    //! Show all widgets belong to given \a group
    void showGroup(const QString &group);

    //! Hide all widgets belong to given \a group
    void hideGroup(const QString &group);

    //! Send key events corresponding to given \a keys.
    void sendKeySequence(const QString &keys);

    //! Show or hide buttons according to toolbar definition and text tselection status
    void updateVisibility();

    //! Update widget parameters when theme has been changed
    void updateFromStyle();

    //! Invalidate and activate layouts, emit \a regionUpdated signal
    void arrangeWidgets();

signals:
    //! Emitted when toolbar's region changed
    void regionUpdated();

    //! Emitted when require a copy/paste action
    void copyPasteRequest(CopyPasteState);

    //! Emitted when require sending a keyevent
    void sendKeyEventRequest(const QKeyEvent &);

    //! Emitted when require sending a string
    void sendStringRequest(const QString &);

    //! Emitted when require closing a keyboard
    void closeKeyboardRequest();

    /*!
     * \brief This signal is emitted when copy/paste button is clicked
     * \param state CopyPasteState button action (copy or paste)
     */
    void copyPasteClicked(CopyPasteState action);

    //! \brief Emitted when toolbar type changes
    //! \param standard true when toolbar is standard, false when a custom
    //! toolbar is set by the application
    void typeChanged(bool standard);

private:
    //! \brief Disable/enable \a arrangeWidgets.
    //!
    //! Supports recursive disabling (multiple calls with true argument require multiple
    //! calls with false argument to enable again).
    void suppressArrangeWidgets(bool suppress);

    //! Helper for updateVisibility and initial state setting
    void updateItemVisibility(const QSharedPointer<MToolbarItem> &item) const;

    //! \brief Set style mode to either shaped-toolbar (true) or full-toolbar (false)
    void setShapedMode(bool shaped);

    void setupLayout();

    void loadCustomWidgets();

    void unloadCustomWidgets();

    Qt::KeyboardModifiers keyModifiers(int key) const;

    void createAndAppendWidget(QSharedPointer<MToolbarItem> item, WidgetBar *leftWidget,
                               WidgetBar *rightWidget);

    bool textSelected;

    WidgetBar leftBar;  //! Widget to hold left-aligned toolbar widgets
    WidgetBar rightBar; //! Widget to hold right-aligned toolbar widgets

    QSharedPointer<const MToolbarData> currentToolbar; //! Pointer to definition of current toolbar

    QList<QPointer<MWidget> > customWidgets; //! All custom widgets in this toolbar

    friend class Ut_MImToolbar;

    bool arrangeWidgetsCalled;
    int arrangeWidgetsDisabledCount;

private:
    M_STYLABLE_WIDGET(MImToolbarStyle)
};

#endif
