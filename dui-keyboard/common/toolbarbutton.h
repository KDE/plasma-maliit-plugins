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


#ifndef TOOLBARBUTTON_H
#define TOOLBARBUTTON_H

#include <DuiNamespace>
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
/*!
 * \brief ToolbarButton represents a named button in a customized toolbar for virtual keyboard.
 */
class ToolbarButton
{
    Q_DISABLE_COPY(ToolbarButton)

public:
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
    ToolbarButton();

    /*!
    * \brief Destructor
    */
    ~ToolbarButton();

    /*!
    * \brief Returns visibility of the button.
    */
    bool isVisible() const;

    /*!
    * \brief Sets the visibility of the button.
    */
    void setVisible(bool);

private:
    //!The NAME attribute should be unique and it is used as a reference in the toolbar system.
    QString name;
    //! the group name which the button belongs to
    QString group;
    int priority;
    Dui::Orientation orientation;
    VisibleType showOn;
    VisibleType hideOn;
    Qt::Alignment alignment;
    QString icon;
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

    //! actions when clicking the button
    QList<Action *> actions;

    friend class ToolbarData;
    friend class ToolbarManager;
    friend class DuiImToolbar;
    friend class ParseParameters;
    friend class Ut_DuiImToolbar;
};

#endif
