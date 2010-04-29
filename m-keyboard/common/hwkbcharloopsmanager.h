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



#ifndef HWKBCHARLOOPSMANAGER_H
#define HWKBCHARLOOPSMANAGER_H

#include <MNamespace>
#include <MGConfItem>
#include <QObject>
#include <QHash>
#include <QChar>
#include <QString>

class QDomElement;
struct HwKbCharLoopParseParameters;
class HwKbCharacterLoops;

/*!
 * \brief HwKbCharLoopsManager managers the hardware keyboard character loops.
 *
 * The hardware keyboard character loops store the accented characters' loops for each language.
 * HwKbCharLoopsManager loads the loops from
 * /usr/share/meegotouch/virtual-keyboard/layouts/hwkbcharloops.xml
 * and managers them. Using characterLoop() one can get the related accented characters for a character.
 */
class HwKbCharLoopsManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(HwKbCharLoopsManager)
public:
    /*!
    * \brief Constructor
    */
    HwKbCharLoopsManager();

    /*!
    * \brief Destructor
    */
    ~HwKbCharLoopsManager();

    /*
     * \brief Returns the character loops for the \a c with current system display language.
     */
    QString characterLoop(const QChar &c) const;

private slots:
    void syncLanguage();

private:
    /*!
    * \brief Load character loops from the specified xml file.
    * The file is loaded from the path /usr/share/meegotouch/virtual-keyboard/layout/.
    * \param fileName XML file name without directory part
    * \return true if loading succeeded, false otherwise
    * \post \a current is set to 0
    */
    bool loadCharLoops(const QString &fileName);

    /*!
    * \brief Implements character loops loading
    * \param filename filename as passed to loadCharLoops
    * \param params holds current state of parser
    */
    bool loadCharLoopsImpl(const QString &fileName, HwKbCharLoopParseParameters  &params);

    //! Parse XML tag for language
    void parseTagLanguage(const QDomElement &element, HwKbCharLoopParseParameters &params);

    //! Parse XML tag for character
    void parseTagCharacter(const QDomElement &element, HwKbCharLoopParseParameters &params);

    //! Type of tag parser methods
    typedef void (HwKbCharLoopsManager::*TagParser)(const QDomElement &, HwKbCharLoopParseParameters &);

    /*!
     * \brief Helper method for parsing children of an element
     * \param element Element whose children are to be parsed
     * \param params Parsing state
     * \param tag1 compulsory, name of expected child tag
     * \param parser1 compulsory, parser for the tag1
     */
    void parseChildren(const QDomElement &element, HwKbCharLoopParseParameters &params,
                       const QString *tag1, TagParser parser1);

    /*!
     * \brief set the active language for character loops.
     * \param language language code which is defined by ISO standards (see ISO 639-1 and ISO 3166-1 alpha-2)
     * and Internet standards (see IETF language tag, RFC 4646 and RFC 4647). e.g. en_gb, en_us, fi, ar.
     * Note: currentCharLoopLanguage will be set to record this language code, even fail to find
     * character loops for \a language. In this case, query characterLoop() will return empty text.
     * \return true if character loops is found for \a language, false if failure or language does not exist.
     */
    bool setCharLoopsLanguage(const QString &language);

    //! All character loops arranged by language.
    //! The map key is language name as read from settings.
    QHash<QString, HwKbCharacterLoops *> charLoops;

    //! Loops for the current language
    const HwKbCharacterLoops *current;

    //! MGConfItem for system display language.
    //! The settings are set by control panel applet.
    MGConfItem configLanguage;

    //! Current language for the character loop
    QString currentCharLoopLanguage;

    //! Keyboard attribute
    QString keyboardVersion;

    friend class Ut_HwKbCharLoopsManager;
};

#endif
