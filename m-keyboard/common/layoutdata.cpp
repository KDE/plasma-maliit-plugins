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



#include "layoutdata.h"
#include "vkbdatakey.h"

#include <QtAlgorithms>
#include <QDebug>

const QString LayoutData::mainSection("main");
const QString LayoutData::functionkeySection("functionkey");
const QString LayoutData::symbolsSymSection("symbols Sym");

LayoutSection::Row::~Row()
{
    qDeleteAll(keys);
}

LayoutData::LayoutData()
    : layoutOrientation(M::Landscape),
      layoutType(General)
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

LayoutData::SharedLayoutSection LayoutData::section(int index) const
{
    if (index < 0 || index >= sections.size()) {
        return SharedLayoutSection(0);
    } else {
        return sections[index];
    }
}

LayoutData::SharedLayoutSection LayoutData::section(const QString &name) const
{
    SharedLayoutSection layoutSection = sectionMap.value(name);

    if (layoutSection.isNull()) {
        qWarning() << __PRETTY_FUNCTION__
                   << "Could not find requested section in current layout:"
                   << "'" << name << "'\n"
                   << " * Available sections:"
                   << sectionMap;

        layoutSection = SharedLayoutSection(new LayoutSection);
    }

    return layoutSection;
}

LayoutSection::LayoutSection()
    : mMaxColumns(0),
      mMaxNormalizedWidth(0),
      maxRows(0),
      movable(false),
      m_verticalAlignment(Qt::AlignVCenter),
      m_horizontalAlignment(Qt::AlignHCenter),
      sectionType(Sloppy)
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
    return mMaxColumns;
}

qreal LayoutSection::maxNormalizedWidth() const
{
    return mMaxNormalizedWidth;
}

int LayoutSection::rowCount() const
{
    return rows.count();
}

int LayoutSection::columnsAt(int row) const
{
    if (isInvalidRow(row)) {
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

QList<int> LayoutSection::spacerIndices(int row) const
{
    if (isInvalidRow(row)) {
        return QList<int>();
    }

    return rows[row]->spacerIndices;
}

VKBDataKey *LayoutSection::vkbKey(int row, int column) const
{
    if (isInvalidCell(row, column)) {
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

bool LayoutSection::isInvalidRow(int row) const
{
    return ((row < 0) || (row >= rows.count()));
}

bool LayoutSection::isInvalidCell(int row, int column) const
{
    return ((column < 0) || isInvalidRow(row) || (column >= rows[row]->keys.count()));
}
