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



#ifndef WIDGETBAR_H
#define WIDGETBAR_H

#include "widgetbarstyle.h"

#include <MStylableWidget>

#include <QList>
#include <QPointer>

class MWidget;
class QGraphicsLinearLayout;

/*!
    \brief WidgetBar displays widgets in a horizontal bar.

    Widgets can be delimited with a divider. Divider hides away when widget is down
    or checked.

    Widgets are not owned by this class, though they are reparented.
*/
class WidgetBar : public MStylableWidget
{
    Q_OBJECT
public:
    explicit WidgetBar(QGraphicsItem *parent = 0);

    virtual ~WidgetBar();

    //! \brief Number of widgets added to this widget.
    int count() const;

    //! \brief Inserts a \a widget to specified \a index.
    //! \param isAvailable Contains true if \a widget should appear on the screen
    //!
    //! \a widget must provide availabilityChanged() signal.
    void insert(int index, MWidget *widget, bool isAvailable);

    //! \brief Appends \a widget to the right end of the row.
    //! \param isAvailable Contains true if \a widget should appear on the screen
    void append(MWidget *widget, bool isAvailable);

    //! \brief Retrieves a \a widget at specific \a index.
    MWidget *widgetAt(int index) const;

    //! \brief Check if \a widget has been added.
    bool contains(const MWidget *widget) const;

    //! \brief Returns index of specified \a widget or -1
    // if widget was not found
    int indexOf(const MWidget *widget) const;

    //! Remove all pointers to destroyed widgets from
    // internal lists
    void cleanup();

public:
    //! \reimp
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    //! \reimp_end

signals:
    //! Emitted when availability of inserted widgets and thus the region changes.
    //! Not emitted when widgets are removed/inserted!
    void regionUpdated();

private slots:
    //! Update layout when some child widget is shown or hidden.
    void updateLayout();

protected:
    //! \reimp
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void styleChanged();
    //! \reimp_end

private:
    //! Return index of given \a widget inside layout
    int layoutIndexOf(const MWidget *widget) const;

private:
    Q_DISABLE_COPY(WidgetBar)

    typedef QList<QPointer<MWidget> > WidgetList;

    QGraphicsLinearLayout &mainLayout;

    WidgetList widgets;

    M_STYLABLE_WIDGET(WidgetBarStyle)

    friend class Ut_WidgetBar;
};

#endif
