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


#ifndef TOOLBARMANAGER_H
#define TOOLBARMANAGER_H

#include <QObject>
#include <QMap>
#include "toolbarbutton.h"

class MImToolbar;
class ToolbarData;
class MGConfItem;
class MButton;

/*!
 \brief The ToolbarManager class manager the virtual keyboard toolbar.

  ToolbarManager loads and managers not only the toolbars which defined with GConf key
  "/meegotouch/inputmethods/toolbars",
  but also the copy/paste button and close button.
*/
class ToolbarManager : public QObject
{
    Q_OBJECT

public:
    /*!
     * \brief Default constructor.
     */
    ToolbarManager(MImToolbar *parent);

    /*!
     *\brief Destructor.
     */
    ~ToolbarManager();

    /*!
     *\brief Returns the button count in current loaded customized toolbar.
     */
    int buttonCount() const;

    /*!
     *\brief Returns the button list in current loaded customized toolbar.
     */
    QList<ToolbarButton *> buttonList() const;

    /*!
     *\brief Returns the button list with \a align in current loaded customized toolbar.
     * And the button list is already sorted by priorities
     */
    QList<ToolbarButton *> buttonList(Qt::Alignment align) const;

    /*!
     *\brief Returns a ToolbarButton pointer to the button with \a name in current loaded customized toolbar.
     */
    ToolbarButton *toolbarButton(const QString &name) const;

    /*!
     *\brief Returns a ToolbarButton pointer to the button with \a button in current loaded customized toolbar.
     */
    ToolbarButton *toolbarButton(const MButton *button) const;

    /*!
     *\brief Returns a MButton pointer to the button with \a name in current loaded customized toolbar.
     */
    MButton *button(const QString &name) const;

    /*!
     *\brief Returns current loaded toolbar's name.
     */
    QString currentToolbar() const;

    /*!
     * \brief Loads the toolbar with \a name
     * ToolbarManager can load a custom toolbar's content according \a name, and cache it for the future use.
     * \return true if the toolbar \a name is loaded successfully or already cached before.
     */
    bool loadToolbar(const QString &name);

    /*!
     *\brief Reset current loaded customized toolbar to 0.
     */
    void reset();

private slots:
    void loadToolbarButtons();

public:
    static const int buttonNameDataKey;

private:
    /*!
     *\brief Returns a list of the name for all toolbars.
     */
    QStringList toolbarList() const;

    const QString *buttonName(const MButton *) const;

    void resetButtonPool();

    ToolbarData *findToolbar(const QString &);

    ToolbarData *createToolbar(const QString &);

    void createButton(const ToolbarButton *b);

    MImToolbar *imToolbar;
    QList<ToolbarData *> toolbars;
    ToolbarData *current;
    QList<MButton *> toolbarButtonPool;

    friend class Ut_MImToolbar;
    friend class Ut_ToolbarManager;
};


#endif
