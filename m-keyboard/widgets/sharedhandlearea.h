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

#ifndef SHAREDHANDLEAREA_H
#define SHAREDHANDLEAREA_H

#include <MWidget>
#include <MNamespace>

class QGraphicsLinearLayout;
class FlickGesture;
class MImToolbar;
class Handle;
class Grip;

/*!
  \brief SharedHandleArea represents the handle area shared between the vkb and the symbol
  view

  The shared handle area contains an invisible handle, toolbar grip and the toolbar (as a
  child of a handle), in that order.
*/
class SharedHandleArea : public MWidget
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     * \param toolbar toolbar widget
     * \param parent Parent object.
     */
    explicit SharedHandleArea(MImToolbar &toolbar, QGraphicsWidget *parent = 0);

    //! Destructor
    virtual ~SharedHandleArea();

    //! Set input method mode
    void setInputMethodMode(M::InputMethodMode mode);

    //! \return region of the widget, including the invisible handle if
    //! \a includeExtraInteractiveAreas is true
    QRegion region(bool includeExtraInteractiveAreas = false) const;

signals:
    void flickUp(const FlickGesture &gesture);
    void flickDown(const FlickGesture &gesture);
    void flickLeft(const FlickGesture &gesture);
    void flickRight(const FlickGesture &gesture);

    void regionUpdated();

private slots:
    //! Handle changes in toolbar availability
    void handleToolbarAvailability(bool available);

private:
    //! Connect signals from a \a handle widget
    void connectHandle(const Handle &handle);

private:
    enum LayoutIndex {
        InvisibleHandleIndex,
        ToolbarHandleIndex,
        ToolbarIndex,
    };

    QGraphicsLinearLayout &mainLayout;

    //! Invisible gesture handle used only in direct mode
    Handle &invisibleHandle;

    //! Toolbar grip
    Grip &toolbarGrip;

    //! Dummy widget we use in place of toolbarGrip in the layout when it's not visible
    QGraphicsWidget &zeroSizeToolbarGrip;

    //! Dummy widget we use in place of the invisible gesture handle when it's
    //! not ... err, visible (in its usual invisible way)
    QGraphicsWidget &zeroSizeInvisibleHandle;
};

#endif
