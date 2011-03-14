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

#ifndef MIMKEYAREA_P_H
#define MIMKEYAREA_P_H

#include "mimabstractkeyarea.h"
#include "mimkey.h"

#include <QSize>
#include <QFontMetrics>
#include <QSharedPointer>

class MImKeyArea;
class MImKeyModel;
class MImKey;

class QPainter;

class MImKeyAreaPrivate
{
public:
    Q_DECLARE_PUBLIC(MImKeyArea)

    //! \brief Constructor
    //! \param owner Pointer to key area which owns this object
    MImKeyAreaPrivate(const LayoutData::SharedLayoutSection &newSection,
                      MImKeyArea *owner);

    //! \brief Destructor
    virtual ~MImKeyAreaPrivate();

    //! \brief Creates buttons for key data models.
    void loadKeys();

    //! \brief Returns the new height of the key area.
    qreal computeWidgetHeight() const;

    //! \brief Return preferred key height for a row.
    //! \param row the index of the queried row
    qreal preferredKeyHeight(int row) const;

    //! \brief Compute the maximum width in this widget, in normalized units.
    qreal computeMaxNormalizedWidth() const;

    //! \brief Returns normalized width for a particular MImKeyModel.
    //! \param model the key model to be queried
    qreal normalizedKeyWidth(const MImKeyModel *model) const;

    //! \brief Draws button rects/bounding rects, for debugging purposes.
    //! \param painter the painter to be used
    //! \param key key for which rects/bounding rects shall be drawn
    //! \param drawBoundingRects whether to draw bounding rects
    //! \param drawRects whether to draw rects
    void drawDebugRects(QPainter *painter,
                        const MImAbstractKey *key,
                        bool drawBoundingRects,
                        bool drawRects) const;

    //! \brief Draws reactive areas of buttons, for debugging purposes.
    //! \param painter the painter to be used
    void drawDebugReactiveAreas(QPainter *painter);

    //! \brief Clears all information about key identifiers
    void clearKeyIds();

    //! \brief Register new key having identifier
    //! \param key Pointer to key which id should be registered
    void registerKeyId(MImKey *key);

    //! \brief Helper struct to store a row of keys.
    struct KeyRow {
        QList<MImKey*> keys; //!< keys in a row
        QVector<QPair<qreal, qreal> > keyOffsets; //!< cached offsets for faster key lookups
    };

    typedef QVector<KeyRow> KeyRowList;
    typedef KeyRowList::iterator RowIterator;
    typedef KeyRowList::const_iterator ConstRowIterator;

    MImKeyArea * const q_ptr;

    KeyRowList rowList; //!< stores all rows of this key area
    qreal cachedWidgetHeight; //!< cached widget height
    qreal mMaxNormalizedWidth; //!< maximal normalized width, for all rows
    QVector<QPair<qreal, qreal> > rowOffsets; //!< cached offsets for faster key lookups
    MImKey *shiftKey; //!< stores shift key, if available in this key area
    bool equalWidthKeys; //!< whether to assume equal width for all keys
    int WidthCorrection; //!< width correction for Arabic layouts
    QSharedPointer<MImKey::StylingCache> stylingCache; //!< Cached information about current styling
    QList<MImKey *> idToKey; //!< Contains information about keys which have identifiers
};

#endif

