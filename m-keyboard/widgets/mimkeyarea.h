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

#ifndef MIMKEYAREA_H
#define MIMKEYAREA_H

#include "mimabstractkeyarea.h"
#include "mimkey.h"

#include <QSize>
#include <QFontMetrics>
#include <QSharedPointer>

//! \brief MImKeyArea reimplements MImAbstractKeyArea and is optimized for drawing.
class MImKeyArea
    : public MImAbstractKeyArea
{
    Q_OBJECT
    Q_DISABLE_COPY(MImKeyArea)

public:
    //! \brief Contructor, see \a MImAbstractKeyArea
    explicit MImKeyArea(const LayoutData::SharedLayoutSection &section,
                        bool usePopup = false,
                        QGraphicsWidget *parent = 0);

    //! \brief Destructor
    virtual ~MImKeyArea();

    //! \reimp
    virtual void modifiersChanged(bool shift,
                                  const QChar &accent = QChar());
    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *view);
    virtual QRectF boundingRect() const;
    virtual void setShiftState(ModifierState newShiftState);
    virtual QList<const MImAbstractKey *> keys() const;
    virtual MImAbstractKey * findKey(const QString &id);
    virtual void setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides);
    //! \reimp_end

private slots:
    void updateKeyAttributes(const QString &, MKeyOverride::KeyOverrideAttributes);

protected:
    //! \reimp
    virtual QSizeF sizeHint(Qt::SizeHint which,
                            const QSizeF &constraint) const;
    virtual void drawReactiveAreas(MReactionMap *reactionMap,
                                   QGraphicsView *view);
    virtual void updateKeyGeometries(int availableWidth);
    virtual MImAbstractKey *keyAt(const QPoint &pos) const;
    virtual void onThemeChangeCompleted();
    virtual void applyStyle();
    //! \reimp_end

    //! \brief Release given \a key if it is required by current attribure override
    void releaseKey(MImKey *key);

private:
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

    KeyRowList rowList; //!< stores all rows of this key area
    qreal cachedWidgetHeight; //!< cached widget height
    qreal mMaxNormalizedWidth; //!< maximal normalized width, for all rows
    QVector<QPair<qreal, qreal> > rowOffsets; //!< cached offsets for faster key lookups
    MImKey *shiftKey; //!< stores shift key, if available in this key area
    bool equalWidthKeys; //!< whether to assume equal width for all keys
    int WidthCorrection; //!< width correction for Arabic layouts
    QSharedPointer<MImKey::StylingCache> stylingCache; //!< Cached information about current styling
    QList<MImKey *> idToKey; //!< Contains information about keys which have identifiers

#ifdef UNIT_TEST
    friend class Ut_MImAbstractKeyArea;
    friend class Ut_KeyEventHandler;
    friend class Ut_SymbolView;
    friend class Bm_MImAbstractKeyArea; //benchmarks
    friend class Bm_Painting;
#endif

    friend class FixtureVirtualKeyboard; //!< needed for testing
};

#endif // MIMKEYAREA_H
