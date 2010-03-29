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
#include "toolbarwidget.h"

class MImToolbar;
class ToolbarData;
class MGConfItem;
class MWidget;

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
     *\brief Returns the widget count in current loaded customized toolbar.
     */
    int widgetCount() const;

    /*!
     *\brief Returns the widget list in current loaded customized toolbar.
     */
    QList<ToolbarWidget *> widgetList() const;

    /*!
     *\brief Returns the widget list with \a align in current loaded customized toolbar.
     * And the widget list is already sorted by priorities
     */
    QList<ToolbarWidget *> widgetList(Qt::Alignment align) const;

    /*!
     *\brief Returns a ToolbarWidget pointer to the widget with \a name in current loaded customized toolbar.
     */
    ToolbarWidget *toolbarWidget(const QString &name) const;

    /*!
     *\brief Returns a ToolbarWidget pointer to the widget with \a widget in current loaded customized toolbar.
     */
    ToolbarWidget *toolbarWidget(const MWidget *widget) const;

    /*!
     *\brief Returns a MWidget pointer to the widget with \a name in current loaded customized toolbar.
     */
    MWidget *widget(const QString &name) const;

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
    void loadToolbarWidgets();

public:
    static const int widgetNameDataKey;
    static const int widgetTypeDataKey;

private:
    /*!
     *\brief Returns a list of the name for all toolbars.
     */
    QStringList toolbarList() const;

    const QString *widgetName(const MWidget *) const;

    void resetWidgetPool();

    ToolbarData *findToolbar(const QString &);

    ToolbarData *createToolbar(const QString &);

    void createWidget(const ToolbarWidget *b);

    MImToolbar *imToolbar;
    QList<ToolbarData *> toolbars;
    ToolbarData *current;
    QList<MWidget *> toolbarWidgetPool;

    friend class Ut_MImToolbar;
    friend class Ut_ToolbarManager;
};


#endif
