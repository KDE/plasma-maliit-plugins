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



#ifndef KEYBOARDDATA_H
#define KEYBOARDDATA_H

#include "layoutdata.h"
#include "vkbdatakey.h"
#include "mvirtualkeyboardstyle.h"

#include <QHash>
#include <QList>
#include <QString>

class QDomElement;
struct ParseParameters;

/*!
 * \class KeyboardData
 * \brief KeyboardData corresponds to a keyboard defined in a layout XML file
 * Therefore it is a collection of LayoutModels for a language.
 */
class KeyboardData
{
    Q_DISABLE_COPY(KeyboardData)

public:
    /*!
    * \brief Constructor
    * private class
    */
    explicit KeyboardData(const MVirtualKeyboardStyleContainer *styleContainer = 0);

    /*!
    * \brief Destructor
    */
    ~KeyboardData();

    /*!
    * \brief Load keyboard and its layouts from the specified xml file.
    * The file is loaded from the path /usr/share/meegotouch/virtual-keyboard/layout/.
    * This method can be called multiple times to reload another keyboard file,
    * whether previous call failed or not.
    * \param fileName XML file name without directory part
    * \return true if loading succeeded, false otherwise
    */
    bool loadNokiaKeyboard(const QString &fileName);

    //! \return keyboard language
    QString language() const;

    //! \return keyoard title
    QString title() const;

    //! \return whether autocaps is enabled.
    bool autoCapsEnabled() const;

    /*!
     * \brief Get layout model by type and orientation
     * \param type type
     * \param orientation orientation
     * \param portraitFallback if true and if no landscape layout is available, return
     * portrait layout instead (if available)
     */
    const LayoutData *layout(LayoutData::LayoutType type, M::Orientation orientation,
                             bool portraitFallback = true) const;

private:
    /*!
    * \brief Implements keyboard loading
    * \param filename filename as passed to loadNokiaKeyboard
    * \param params holds current state of parser
    * \param importedLayout indicates if this layout file is imported from within another.
    */
    bool loadNokiaKeyboardImpl(const QString &fileName, ParseParameters &params,
                               bool importedLayout = false);

    //! \brief Get layout model by type and orientation
    LayoutData *layoutPrivate(LayoutData::LayoutType type, M::Orientation orientation,
                              bool portraitFallback = true) const;

    /*!
    * \brief Translate alignmentString to Qt::Alignment
    */
    static Qt::Alignment alignment(const QString &alignmentString, bool vertical);

    /*!
    * \brief Translate orientationString to M::Orientation
    */
    static M::Orientation orientation(const QString &orientationString);

    //! \brief Translate key action string into a KeyBinding::KeyAction
    static KeyBinding::KeyAction keyActionFromString(const QString &typeStr);

    //! Parse XML tag for layout
    void parseTagLayout(const QDomElement &element, ParseParameters &params);

    //! Parse XML tag for binding
    void parseTagBinding(const QDomElement &element, ParseParameters &params);

    //! Parse XML tag for section
    void parseTagSection(const QDomElement &element, ParseParameters &params);

    //! Parse XML tag for row
    void parseTagRow(const QDomElement &element, ParseParameters &params);

    //! Parse XML tag for key
    void parseTagKey(const QDomElement &element, ParseParameters &params);

    //! Parse XML tag for layout spacers
    void parseTagSpacer(const QDomElement &element, ParseParameters &params);

    //! Type of tag parser methods
    typedef void (KeyboardData::*TagParser)(const QDomElement &, ParseParameters &);

    /*!
     * \brief Helper method for parsing children of an element
     * \param element Element whose children are to be parsed
     * \param params Parsing state
     * \param tag1 compulsory, name of expected child tag
     * \param parser1 compulsory, parser for the tag1
     * \param tag2 optional, name of another allowed child tag
     * \param parser2 compulsory with tag2, parser for the tag2
     */
    void parseChildren(const QDomElement &element, ParseParameters &params,
                       const QString *tag1, TagParser parser1, const QString *tag2 = NULL,
                       TagParser parser2 = NULL);

    //! Parse XML tag for import
    void parseTagImport(const QDomElement &element, ParseParameters &params);

    static VKBDataKey::StyleType toStyleType(const QString &attributeValue);

    static VKBDataKey::SizeGroupType toSizeGroup(const QString &attributeValue);

    static bool toBoolean(const QString &attributeValue);

protected:
    LayoutData *currentLayout;

    //! keyboard attribute
    QString keyboardVersion;
    QString keyboardTitle;
    QString keyboardLanguage;
    QString keyboardCatalog;
    bool keyboardAutoCapsEnabled;

    QList<LayoutData *> layouts;
    QHash<QString, LayoutData::LayoutType> layoutTypeMap;

    const MVirtualKeyboardStyleContainer *const styleContainer;
};

#endif
