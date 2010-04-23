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



#ifndef TOOLBARDATA_H
#define TOOLBARDATA_H

#include <QObject>
#include <QList>
#include "toolbarwidget.h"

class QDomElement;
struct TBParseParameters;
struct TBParseStructure;

/*!
  \brief ToolbarData corresponds to a toolbar defined in a XML file
 */
class ToolbarData : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ToolbarData)

public:
    /*!
    * \brief Constructor
    */
    ToolbarData();

    /*!
    * \brief Destructor
    */
    ~ToolbarData();

    /*!
    * \brief Load a custom toolbar's content from \a fileName xml file.
    * \a fileName is the xml file name (with ".xml" postfix). And \a fileName could have absolute path.
    * If no absolute path specified, then it will be taken from the default path
    * "/usr/share/meegotouch/imtoolbars/".
    * \param fileName Name of the xml file which contains the content of a custom toolbar.
    */
    bool loadNokiaToolbarXml(const QString &fileName);

    /*!
    * \brief Returns the custom toolbar's xml file name.
    */
    QString fileName() const;

    /*!
     *\brief Returns true \a toolbar equal this custom toolbar.
     * \a toolbar is the xml file name.
     * \sa fileName(), \sa loadNokiaToolbarXml().
     */
    bool equal(const QString &toolbar) const;

private:
    /*!
     * \brief Translate alignmentString to Qt::Alignment.
     */
    static Qt::Alignment alignment(const QString &alignmentString);

    /*!
    * \brief Translate orientationString to M::Orientation.
    */
    static M::Orientation orientation(const QString &orientationString);

    /*!
     * \brief Translate visibleTypeString to ToolbarWidget::VisibleType.
     */
    static ToolbarWidget::VisibleType visibleType(const QString &visibleTypeString);

    //! Parse XML tag for button.
    void parseTagButton(const QDomElement &element, TBParseParameters &params);

    //! Parse XML tag for label.
    void parseTagLabel(const QDomElement &element, TBParseParameters &params);

    //! Parse XML tag for action.
    void parseTagActions(const QDomElement &element, TBParseParameters &params);

    //! Parse XML tag for SendKeySequence.
    void parseTagSendKeySequence(const QDomElement &element, TBParseParameters &params);

    //! Parse XML tag for SendString
    void parseTagSendString(const QDomElement &element, TBParseParameters &params);

    //! Parse XML tag for SendCommand
    void parseTagSendCommand(const QDomElement &element, TBParseParameters &params);

    //! Parse XML tag for Copy
    void parseTagCopy(const QDomElement &element, TBParseParameters &params);

    //! Parse XML tag for Paste
    void parseTagPaste(const QDomElement &element, TBParseParameters &params);

    //! Parse XML tag for ShowGroup
    void parseTagShowGroup(const QDomElement &element, TBParseParameters &params);

    //! Parse XML tag for HideGroup
    void parseTagHideGroup(const QDomElement &element, TBParseParameters &params);

    //! Type of tag parser methods
    typedef void (ToolbarData::*TagParser)(const QDomElement &, TBParseParameters &);

    /*!
     * \brief Helper method for parsing children of an element
     * \param element Element whose children are to be parsed
     * \param params Parsing state
     * \param parserList, a TBParseStructure array.
     * \param parserCount, the number of TBParseStructure in the array.
     */
    void parseChildren(const QDomElement &element, TBParseParameters &params,
                       const TBParseStructure *parserList, int parserCount = 1);
protected:
    QList<ToolbarWidget *> widgets;
    QString toolbarFileName;
    friend class ToolbarManager;
    friend struct TBParseStructure;
    friend class Ut_MImToolbar;
};

#endif //TOOLBAR_H
