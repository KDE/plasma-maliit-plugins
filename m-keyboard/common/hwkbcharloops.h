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



#ifndef HWKBCHARLOOPS_H
#define HWKBCHARLOOPS_H

#include <MNamespace>
#include <QString>
#include <QHash>

/*!
 * \brief HwKbCharacterLoops represents the hardware keyboard character loops.
 *
 * The hardware keyboard character loops store the accented characters' loops for each language.
 * In hardware keyboard state, if a character key is pressed when Sym key is held down, an accented character
 * for that letter is shown. If the same character key is pressed repeatedly holding the Sym key down,
 * the accented character will be changed to next one in a looping fashion (when last accented is reached,
 * the first is shown again).
 * The exact content of the Sym + Character loops are defined for each language.
 *
 */
class HwKbCharacterLoops
{
    Q_DISABLE_COPY(HwKbCharacterLoops)

public:
    /*!
    * \brief Constructor
    */
    HwKbCharacterLoops(const QString &language, const QString &name);

    /*!
    * \brief Destructor
    */
    ~HwKbCharacterLoops();

private:
    /*!
    * \brief Disable default construction
    */
    HwKbCharacterLoops();

    QString language;
    QString name;
    //! Charater loops
    QHash<QChar, QString> loops;

    friend class HwKbCharLoopsManager;
    friend class Ut_HwKbCharLoopsManager;
};
#endif
