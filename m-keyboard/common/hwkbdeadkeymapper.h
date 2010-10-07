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

#ifndef HWKBDEADKEYMAPPER_H
#define HWKBDEADKEYMAPPER_H

#include <MGConfItem>
#include <QObject>
#include <QHash>
#include <QChar>
#include <QString>

class QDomElement;

/*!
 * \brief HwKbDeadKeyMapper composes dead keys with normal keys based on mapping data
 * it reads from hwkb_dead_keys.xml file within the layouts.
 */
class HwKbDeadKeyMapper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(HwKbDeadKeyMapper)
public:
    /*!
    * \brief Constructor
    */
    HwKbDeadKeyMapper();

    /*!
    * \brief Destructor
    */
    virtual ~HwKbDeadKeyMapper();

    //! \brief Handle key presses for dead key processing
    //!
    //! Emits \a stateChanged if and only if the state changes.
    //!
    //! \param text press event or long press result text
    //! \param noCombine if true, only update current dead key, don't combine
    //! current dead key with \a text
    //! \return true if \a text was combined with a previously pressed dead key
    //! into a new character, which is assigned to \a text.
    bool filterKeyPress(QString &text, bool noCombine = false);

    //! \return currently active dead key or null character
    QChar currentDeadKey() const;

    //! \brief Set dead key mapping to match the given XKB \a layout & \a variant combination
    void setLayout(const QString &layout, const QString &variant);

    //! \brief Reset dead key composing state
    void reset();

signals:
    //! \brief Emitted when dead key is activated or deactivated
    //! \param deadKey activated dead key or a null character
    void stateChanged(const QChar &deadKey);

private slots:
    void syncLayoutAndVariant();

private:
    /*!
    * \brief Load dead key maps from the specified xml file.
    * The file is loaded from the path /usr/share/meegotouch/virtual-keyboard/layout/.
    * \param fileName XML file name without directory part
    * \return true if loading succeeded, false otherwise
    */
    bool loadDeadKeyMaps(const QString &fileName);

    struct ParseParameters;

    /*!
    * \brief Implements dead key map loading
    * \param filename filename as passed to loadDeadKeyMaps
    * \param params holds current state of parser
    */
    bool loadDeadKeyMapsImpl(const QString &fileName, ParseParameters &params);

    //! Parse XML tag for language
    void parseTagLanguage(const QDomElement &element, ParseParameters &params);

    //! Parse XML tag for single mapping
    void parseTagMapping(const QDomElement &element, ParseParameters &params);

    //! Parse XML tag for single mapping
    void parseTagInclude(const QDomElement &element, ParseParameters &params);

    //! Type of tag parser methods
    typedef void (HwKbDeadKeyMapper::*TagParser)(const QDomElement &, ParseParameters &);

    /*!
     * \brief Helper method for parsing children of an element
     * \param element Element whose children are to be parsed
     * \param params Parsing state
     * \param tag1 compulsory, name of expected child tag
     * \param parser1 compulsory, parser for the tag1
     */
    void parseChildren(const QDomElement &element, ParseParameters &params,
                       const char *tag1, TagParser parser1,
                       const char *tag2 = 0, TagParser parser2 = 0);

private:
    //! Character to be combined with dead key -> result character
    typedef QHash<QChar, QChar> CombineWithMap;
    //! Dead key -> CombineWithMap
    typedef QHash<QChar, CombineWithMap *> DeadKeyMap;

    //! All dead key maps arranged by layout[_variant] keys
    QHash<QString, DeadKeyMap *> deadKeyMaps;

    //! Dead key map for the current layout/variant
    const DeadKeyMap *currentMap;

    QChar deadKey;

    MGConfItem xkbPrimaryLayoutSetting;
    MGConfItem xkbPrimaryVariantSetting;

    friend class Ut_HwKbDeadKeyMapper;
};

#endif
