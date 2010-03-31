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
#include <QHash>
#include <QPointer>
#include "toolbarwidget.h"
#include <memory>

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
     *\brief Destructor.
     */
    virtual ~ToolbarManager();

    //! \brief Get singleton instance
    //! \return singleton instance
    static ToolbarManager &instance();

    //! \brief Create singleton
    static void createInstance();

    //! \brief Destroy singleton
    static void destroyInstance();

    /*!
     * \brief Register an input method toolbar which is defined in \a fileName with the unique identifier \a id.
     * ToolbarManager can load a custom toolbar's content according \a id and \a fileName, and cache it for the
     * future use. The \a id should be unique, and the \a fileName is the absolute file name of the custom toolbar.
     */
    void registerToolbar(qlonglong id, const QString &fileName);

    /*!
     * \brief Unregister an input method \a toolbar which unique identifier is \a id.
     * ToolbarManager will remove the cached toolbar according \a id.
     */
    void unregisterToolbar(qlonglong id);

    /*!
     * \brief Sets the \a attribute for the \a item in the custom toolbar which has the unique \a id to \a value.
     */
    void setToolbarItemAttribute(qlonglong id, const QString &item, const QString &attribute, const QVariant &value);

    QRegion region(bool includeToolbar = true) const;
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
     *\brief Returns a ToolbarWidget pointer to the widget with \a name in cached customized toolbar with \a id.
     */
    ToolbarWidget *toolbarWidget(qlonglong id, const QString &name) const;

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
     *\brief Returns current loaded toolbar's identifier.
     */
    qlonglong currentToolbar() const;

    /*!
     * \brief Loads the toolbar according the unique \a id.
     * The \a uuid must be registered before by registerToolbar().
     * \return true if the toolbar \a name is loaded successfully or already cached before.
     * \sa registerToolbar().
     */
    bool loadToolbar(qlonglong id);

    /*!
     *\brief Reset current loaded customized toolbar to 0.
     */
    void reset();

signals:
    //! Emitted when a button is clicked
    void buttonClicked(const ToolbarWidget &);

private slots:
    void loadToolbarWidgets();

    void handleButtonClick();

public:
    static const int widgetNameDataKey;
    static const int widgetTypeDataKey;

private:
    /*!
     * \brief Default constructor.
     */
    ToolbarManager();

    /*!
     *\brief Returns a list of the name for all toolbars.
     */
    QList<qlonglong> toolbarList() const;

    const QString *widgetName(const MWidget *) const;

    void resetWidgetPool();

    bool validateWidgetPool();

    ToolbarData *createToolbar(const QString &name);

    void createWidget(const ToolbarWidget *b);

    typedef QHash<qlonglong, QString> ToolbarContainer;
    //! all registered toolbars
    ToolbarContainer toolbars;

    typedef QHash<qlonglong, ToolbarData *> CachedToolbarContainer;
    //! cached toolbars
    CachedToolbarContainer cachedToolbars;

    //! the list of identifier of the cached toolbars, sorted by used frequency.
    QList<qlonglong> cachedToolbarIds;

    //! current used toolbar iterator
    CachedToolbarContainer::const_iterator current;

    QList< QPointer<MWidget> > toolbarWidgetPool;

    //! Singleton instance
    static ToolbarManager *toolbarMgrInstance;

    friend class Ut_MImToolbar;
    friend class Ut_ToolbarManager;
};

inline ToolbarManager &ToolbarManager::instance()
{
    Q_ASSERT(toolbarMgrInstance);
    return *toolbarMgrInstance;
}

#endif
