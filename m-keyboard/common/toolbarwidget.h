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


#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H

#include <MNamespace>
#include <QList>
#include <QString>
#include <QStringList>
/*!
 * \brief ToolbarWidget represents a named widget in a customized toolbar for virtual keyboard.
 */
class ToolbarWidget
{
    Q_DISABLE_COPY(ToolbarWidget)

public:
    //! Type of toolbar widget
    enum WidgetType {
        Button,
        Label,
        UndefinedWidgetType
    };

    //! Type of visible premiss for toolbar button
    enum VisibleType {
        WhenSelectingText,
        Always,
        Undefined
    };

    //! Type of action
    enum ActionType {
        SendKeySequence,
        SendString,
        SendCommand,
        Copy,
        Paste,
        ShowGroup,
        HideGroup,
        Unknown
    };

    /*!
    * \brief Constructor
    */
    ToolbarWidget(WidgetType type = UndefinedWidgetType);

    /*!
    * \brief Destructor
    */
    virtual ~ToolbarWidget();

    /*!
     * \brief Returns the WidgetType of the widget.
     * \sa WidgetType.
     */
    WidgetType type() const;

    /*!
     * \brief Returns the name of the widget.
     */
    QString name() const;

    /*!
    * \brief Returns visibility of the widget.
    */
    bool isVisible() const;

    /*!
    * \brief Sets the visibility of the widget.
    */
    void setVisible(bool);

protected:
    WidgetType widgetType;
    //! The NAME attribute should be unique and it is used as a reference in the toolbar system.
    QString widgetName;
    //! The group name which the button belongs to
    QString group;
    int priority;
    M::Orientation orientation;
    VisibleType showOn;
    VisibleType hideOn;
    Qt::Alignment alignment;
    QString text;
    QString textId;
    bool visible;

    struct Action {
        Action(ActionType = Unknown);
        ActionType type;
        QString keys;
        QString text;
        QString command;
        QString group;
    };

    // below attributes are only valid for Button
    bool toggle;
    bool pressed;
    QString icon;
    //! actions when clicking the widget
    QList<Action *> actions;

    friend class ToolbarData;
    friend class ToolbarManager;
    friend class MImToolbar;
    friend class ParseParameters;
    friend class Ut_MImToolbar;
};

#endif
