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



#include <QtAlgorithms>
#include "layoutdata.h"
#include "vkbdatakey.h"

const QString LayoutData::mainSection("main");
const QString LayoutData::functionkeySection("functionkey");
const QString LayoutData::symbolsSymSection("symbols Sym");

LayoutSection::Row::~Row()
{
    qDeleteAll(keys);
}

LayoutData::LayoutData()
{
}

LayoutData::~LayoutData()
{
}

LayoutData::LayoutType LayoutData::type() const
{
    return layoutType;
}

M::Orientation LayoutData::orientation() const
{
    return layoutOrientation;
}

int LayoutData::numSections() const
{
    return sections.size();
}

QSharedPointer<const LayoutSection> LayoutData::section(int index) const
{
    return sections[index];
}

QSharedPointer<const LayoutSection> LayoutData::section(const QString &name) const
{
    QSharedPointer<LayoutSection> section = sectionMap.value(name);
    return section;
}

LayoutSection::LayoutSection()
    : m_maxColumns(0),
      maxRows(0),
      sectionName("")
{
}

LayoutSection::~LayoutSection()
{
    qDeleteAll(rows);
}

const QString &LayoutSection::name() const
{
    return sectionName;
}

LayoutSection::Type LayoutSection::type() const
{
    return sectionType;
}

int LayoutSection::maxColumns() const
{
    return m_maxColumns;
}

int LayoutSection::rowCount() const
{
    return rows.count();
}

int LayoutSection::columnsAt(int row) const
{
    if ((row < 0) || (row >= rows.count())) {
        return 0;
    }

    return rows[row]->keys.count();
}

int LayoutSection::keyCount() const
{
    int numKeys = 0;
    for (QList<Row *>::const_iterator rowIter(rows.begin());
         rowIter != rows.end(); ++rowIter) {
         numKeys += (*rowIter)->keys.count();
    }
    return numKeys;
}

VKBDataKey *LayoutSection::getVKBKey(int row, int column) const
{
    if ((row < 0) || (column < 0) || (row >= rows.count())
            || (column >= rows[row]->keys.count())) {
        return 0;
    }

    return rows[row]->keys[column];
}

Qt::Alignment LayoutSection::horizontalAlignment() const
{
    return m_horizontalAlignment;
}

Qt::Alignment LayoutSection::verticalAlignment() const
{
    return m_verticalAlignment;
}
