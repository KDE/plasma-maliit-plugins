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

#include "mimkeyarea.h"
#include "mimkeyarea_p.h"
#include "mimkeyvisitor.h"
#include "mplainwindow.h"
#include "reactionmapwrapper.h"

#include <QDebug>
#include <QEvent>
#include <QPainter>
#include <QTextCharFormat>
#include <QTextLine>
#include <QList>
#include <QVector>

#include <mkeyoverride.h>

#include <MScalableImage>
#include <MTimestamp>
#include <float.h>

namespace {
    template<class T>
    int binaryRangeFind(T value,
                        const QVector<QPair<T, T> > &offsets)
    {
        int lowerBound = 0;
        int upperBound = offsets.size() - 1;

        while (lowerBound <= upperBound) {
            const int pivot = (lowerBound + upperBound) / 2;
            const QPair<T, T> &current = offsets.at(pivot);

            if (value < current.first) {
                upperBound = pivot - 1;
            } else if (value > current.second) {
                lowerBound = pivot + 1;
            } else {
                return pivot;
            }
        }

        // not found:
        return -1;
    }

    //! \brief Helper class responsible for key-painting aspect.
    //!
    //! Can be used as visitor.
    class KeyPainter
        : public MImAbstractKeyVisitor
    {
    public:
        //! PaintMode can be used to optimize drawing for HW acceleration
        enum PaintMode {
            PaintBackground, //!< Only draw background of given keys, fills up iconKeys
            PaintBackgroundAndIcon //!< Draw both, background and icon, in one go
        };

    private:
        const MImKeyArea *const keyArea; //!< owner of the keys
        QPainter *const painter; //!< used for painting
        PaintMode mode; //!< current paint mode
        mutable QList<const MImKey *> iconKeys; //!< collects keys with icons, \sa drawIcons

    public:
        explicit KeyPainter(const MImKeyArea *newKeyArea,
                            QPainter *newPainter,
                            PaintMode newMode = PaintBackgroundAndIcon)
            : keyArea(newKeyArea)
            , painter(newPainter)
            , mode(newMode)
        {
            Q_ASSERT(keyArea != 0);
            Q_ASSERT(painter != 0);
        }

        //! \brief Paints background or icon of given key, depending on PaintMode
        //! \param abstractKey the given key
        bool operator()(MImAbstractKey *abstractKey)
        {
            return operator()(dynamic_cast<const MImKey *>(abstractKey));
        }

        //! \brief Paints background or icon of given key.
        //! \param key the given key
        bool operator()(const MImKey *key) const
        {
            if (key && key->belongsTo(keyArea)) {
                switch (mode) {

                case PaintBackground:
                    // Collect keys first, need to call drawIcons separately:
                    drawBackground(painter, key);

                    if (key->icon()) {
                        iconKeys.append(key);
                    }

                    break;

                case PaintBackgroundAndIcon:
                default:
                    drawBackground(painter, key);
                    key->drawIcon(painter);
                    break;
                }
            }

            // If used as visitor, then we need to visit all active keys:
            return false;
        }

        //! \brief Draw all previously visited keys with icons
        void drawIcons() const
        {
            foreach (const MImKey *key, iconKeys) {
                key->drawIcon(painter);
            }
        }

        //! \brief Draws background for a given key.
        //! \param painter the painter to be used
        //! \param key key for which background shall be drawn
        void drawBackground(QPainter *painter,
                            const MImAbstractKey *key) const
        {
            if (!key) {
                return;
            }

            const MScalableImage *background = key->normalBackgroundImage();

            if (background) {
                background->draw(key->buttonRect().toRect(), painter);
            }
        }
    };

    //! Helper class responsible for key geometry aspect.
    class KeyGeometryUpdater
    {
    public:
        typedef QVector<QPair<qreal, qreal> > OffsetList;

        enum RowTypeFlags {
            NormalRow = 0, //!< Neither first or last row, default value.
            FirstRow = 1, //!< First row in a given layout.
            LastRow = 2 //!< Last row in a given layout.
        };

    private:
        const MImAbstractKeyAreaStyleContainer &style; //!< Reference to key area's style
        const qreal availableWidth; //!< Available width, usually equals key area width.

        // key bounding rect related members:
        QPoint currentPos; //!< Current position, used to update a key's top left corner.
        OffsetList mRowOffsets; //!< Collects (vertical) row offsets.
        OffsetList mKeyOffsets; //!< Collects (horizontal) key offsets, reset for each new row.

        const qreal mRelativeKeyWidth; //!< Only relevant if use-fixed-key-width is false in CSS.

    public:
        explicit KeyGeometryUpdater(const MImAbstractKeyAreaStyleContainer &newStyle,
                                    qreal newAvailableWidth,
                                    qreal maxNormalizedWidth)
            : style(newStyle)
            , availableWidth(newAvailableWidth)
            , mRelativeKeyWidth(computeRelativeKeyWidth(maxNormalizedWidth))
        {}

        OffsetList rowOffsets() const
        {
            return mRowOffsets;
        }

        OffsetList keyOffsets() const
        {
            return mKeyOffsets;
        }

        qreal relativeKeyWidth() const
        {
            return mRelativeKeyWidth;
        }

        //! Process a row of keys. Updates geometry of each key (width, height, margins, position).
        //! \param row the row to be processed
        //! \param newKeyHeight the key height for keys in this row
        //! \param spacerIndices where to place spacers
        //! \param flags hints about the row's position in layout
        void processRow(const QList<MImKey *> &row,
                        qreal keyHeight,
                        const QList<int> &spacerIndices,
                        int flags = NormalRow)
        {

            currentPos.rx() = 0;
            const QSizeF rowSize = updateGeometry(row, keyHeight, flags);

            const qreal spacerWidth = qAbs((availableWidth - rowSize.width())
                                           / ((spacerIndices.count() ==  0) ? 1 : spacerIndices.count()));

            const qreal nextPosY = currentPos.y() + rowSize.height();
            mRowOffsets.append(QPair<qreal, qreal>(currentPos.y(), nextPosY));
            mKeyOffsets.clear();

            updatePosition(row, spacerIndices, spacerWidth);
            currentPos.ry() = nextPosY;
        }

    private:
        //! Computes relative width based on row with max normalized width.
        qreal computeRelativeKeyWidth(qreal maxNormalizedWidth) const
        {
            const qreal widthConsumedByMargins = (style->keyMarginLeft() + style->keyMarginRight())
                                                 * qMax<qreal>(0.0, maxNormalizedWidth - 1)
                                                 + style->paddingLeft()
                                                 + style->paddingRight();

            return ((availableWidth - widthConsumedByMargins) / qMax<qreal>(1.0, maxNormalizedWidth));
        }

        //! Updates geometry of each key in a row.
        //! \param row the row to be processed
        //! \param keyHeight the key height for keys in this row
        //! \param flags hints about the row's position in layout
        QSizeF updateGeometry(const QList<MImKey *> &row,
                              qreal keyHeight,
                              int flags) const
        {
            QSizeF result;
            MImKey *stretchKey = 0;
            const qreal marginTop = (flags & FirstRow ? style->paddingTop()
                                                      : style->keyMarginTop());
            const qreal marginBottom = (flags & LastRow ? style->paddingBottom()
                                                        : style->keyMarginBottom());
            qreal rowWidth = 0.0;
            const qreal spacingBetweenKeys = style->keyMarginRight() + style->keyMarginLeft();

            foreach (MImKey *const key, row) {

                MImKey::Geometry g(key->geometry());
                g.height = keyHeight;
                g.width = (style->useFixedKeyWidth() ? key->preferredFixedWidth()
                                                     : key->preferredWidth(mRelativeKeyWidth,
                                                                           spacingBetweenKeys));
                g.marginLeft = style->keyMarginLeft();
                g.marginTop = marginTop;
                g.marginRight = style->keyMarginRight();
                g.marginBottom = marginBottom;
                key->setGeometry(g);

                rowWidth += key->buttonBoundingRect().width();

                if (!stretchKey && key->model().width() == MImKeyModel::Stretched) {
                    stretchKey = key;
                }
            }

            if (!row.isEmpty()) {
                MImKey *first = row.first();
                MImKey::Geometry firstGeometry(first->geometry());
                rowWidth -= firstGeometry.marginLeft;
                firstGeometry.marginLeft = style->paddingLeft();
                rowWidth += firstGeometry.marginLeft;
                first->setGeometry(firstGeometry);

                MImKey *last = row.last();
                MImKey::Geometry lastGeometry(last->geometry());
                rowWidth -= lastGeometry.marginRight;
                lastGeometry.marginRight = style->paddingRight();
                rowWidth += lastGeometry.marginRight;
                last->setGeometry(lastGeometry);

                result.rheight() = first->buttonBoundingRect().height();
            }

            // Assign remaining available width to stretchKey, but leave its margins untouched:
            if (stretchKey) {
                rowWidth -= stretchKey->geometry().width;
                stretchKey->setWidth(availableWidth - rowWidth);
                rowWidth += stretchKey->geometry().width;
            }

            result.rwidth() = rowWidth;
            return result;
        }

        //! Updates position of each key in a row.
        //! \param row the row to be processed
        //! \param spacerIndices where to place spacers
        //! \param spacerWidth the width of a spacer
        void updatePosition(const QList<MImKey *> &row,
                            const QList<int> &spacerIndices,
                            qreal spacerWidth)
        {
            int index = 0;
            currentPos.rx() = 0;

            // Normalizing spacerIndices into real spacers first.
            // There are n - 1 possible spacers between n keys, and there can
            // be one more to each end of a row, so it's n + 1 spaces:
            QVector<int> spacers(row.size() + 1);
            for (int index = 0; index < row.size() + 1; ++index) {
                // If key is at pos(i) in row, then i - 1 describes spacers
                // before the key, whereas i describes spacers after the key.
                // This mapping is preserved while normalizing spacerIndices
                // into spacers:
                spacers[index] = spacerIndices.count(index - 1) * spacerWidth;
            }

            // Updating margins of keys to include surrounding spacers as
            // well as assiging their final position. Space between keys is
            // distributed equally to both keys:
            foreach (MImKey *const key, row) {
                // For first key, assign all empty space on left:
                const qreal spaceBefore(spacers.at(index)
                                        * (index == 0 ? 1 : 0.5));

                // For last key, assign all empty space on right:
                const qreal spaceAfter(spacers.at(index + 1)
                                       * (index == row.size() - 1 ? 1 : 0.5));

                MImKey::Geometry g(key->geometry());
                g.marginLeft += spaceBefore;
                g.marginRight += spaceAfter;
                key->setGeometry(g);
                key->setPos(currentPos);
                key->updateGeometryCache();

                const qreal nextPosX = currentPos.x() + key->buttonBoundingRect().width();
                mKeyOffsets.append(QPair<qreal, qreal>(currentPos.x(), nextPosX));

                currentPos.rx() = nextPosX;
                ++index;
            }
        }
    };

    const char * const emailUrlKey = "emailUrlKey"; //!< Identifies key which should be customized for email and URL content type
}

MImKeyAreaPrivate::MImKeyAreaPrivate(const LayoutData::SharedLayoutSection &newSection,
                                     MImKeyArea *owner)
    : MImAbstractKeyAreaPrivate(newSection, owner),
      q_ptr(owner),
      rowList(newSection->rowCount()),
      shiftKey(0),
      equalWidthKeys(true),
      WidthCorrection(0),
      stylingCache(new MImKey::StylingCache),
      toggleKey(0),
      composeKey(0)
{
}

MImKeyAreaPrivate::~MImKeyAreaPrivate()
{
    for (RowIterator rowIter(rowList.begin()); rowIter != rowList.end(); ++rowIter) {
        qDeleteAll(rowIter->keys);
        rowIter->keys.clear();
    }
    clearKeyIds();
}

void MImKeyAreaPrivate::loadKeys()
{
    Q_Q(MImKeyArea);
    const int numRows = rowCount();

    RowIterator rowIter(rowList.begin());

    for (int row = 0; row != numRows; ++row, ++rowIter) {
        const int numColumns = section->columnsAt(row);

        // Add keys
        for (int col = 0; col < numColumns; ++col) {
            // Parameters to fetch from base class.
            MImKeyModel *dataKey = section->keyModel(row, col);
            MImKey *key = new MImKey(*dataKey, q->baseStyle(), *q, stylingCache);

            if (!key->model().id().isEmpty()) {
                registerKeyId(key);
            }

            // Temporary, dirty workaround-hack to detect Arabic layouts
            // because the QTextLayout rendering is broken with Arabic characters and
            // the width must be corrected. See NB#197937 and QTBUG-15511.
            // TODO: Remove this when the above bugs are fixed.
            if (WidthCorrection == 0)
            {
                QVector<uint> Ucs4Codes = key->label().toUcs4();
                uint UnicodeCode = Ucs4Codes.size() > 0 ? Ucs4Codes[0] : 0;

                // The character is in the Unicode range of the Arabic characters.
                if (UnicodeCode >= 1536 && UnicodeCode <= 1791)
                    WidthCorrection = -8;
            }


            // TODO: Remove restriction to have only one shift key per layout?
            if (dataKey->binding()->action() == MImKeyBinding::ActionShift) {
                shiftKey = key;
            } else if (dataKey->binding()->action() == MImKeyBinding::ActionOnOffToggle) {
                // FIXME: Only has one toggle key?
                toggleKey = key;
            } else if (dataKey->binding()->action() == MImKeyBinding::ActionCompose) {
                // FIXME: Only has one compose key?
                composeKey = key;
            }

            rowIter->keys.append(key);
        }
    }

    q->updateGeometry();
}

qreal MImKeyAreaPrivate::computeWidgetHeight() const
{
    Q_Q(const MImKeyArea);
    qreal height = q->baseStyle()->size().height();

    if (height < 0) {
        for (int index = 0; index < rowList.count(); ++index) {
            height += preferredKeyHeight(index);

            height += (index == 0 ? q->baseStyle()->paddingTop()
                                  : q->baseStyle()->keyMarginTop());
            height += (index == rowList.count() - 1 ? q->baseStyle()->paddingBottom()
                                                    : q->baseStyle()->keyMarginBottom());
        }
    }

    return qMax<qreal>(0.0, height);
}

void MImKeyAreaPrivate::drawDebugRects(QPainter *painter,
                                const MImAbstractKey *key,
                                bool drawBoundingRects,
                                bool drawRects) const
{
    if (drawBoundingRects) {
        painter->save();
        painter->setPen(Qt::red);
        painter->setBrush(QBrush(QColor(64, 0, 0, 64)));
        painter->drawRect(key->buttonBoundingRect());
        painter->drawText(key->buttonRect().adjusted(4, 4, -4, -4),
                          QString("%1x%2").arg(key->buttonBoundingRect().width())
                          .arg(key->buttonBoundingRect().height()));
        painter->restore();
    }

    if (drawRects) {
        painter->save();
        painter->setPen(Qt::green);
        painter->setBrush(QBrush(QColor(0, 64, 0, 64)));
        painter->drawRect(key->buttonRect());
        painter->drawText(key->buttonRect().adjusted(4, 16, -4, -16),
                          QString("%1x%2").arg(key->buttonRect().width())
                          .arg(key->buttonRect().height()));
        painter->restore();
    }
}

void MImKeyAreaPrivate::drawDebugReactiveAreas(QPainter *painter)
{
    Q_Q(MImKeyArea);
    painter->save();

    foreach (const KeyRow &row, rowList) {
        foreach (const MImKey *const key, row.keys) {
            const QRectF rect = q->correctedReactionRect(key->buttonBoundingRect());

            painter->setPen(Qt::magenta);
            painter->drawLine(rect.topLeft(), rect.topRight());
            painter->drawLine(rect.bottomLeft(), rect.bottomRight());

            painter->setPen(Qt::cyan);
            painter->drawLine(rect.topLeft(), rect.bottomLeft());
            painter->drawLine(rect.topRight(), rect.bottomRight());
        }
    }

    painter->restore();
}

qreal MImKeyAreaPrivate::preferredKeyHeight(int row) const
{
    Q_Q(const MImKeyArea);

    switch (section->rowHeightType(row)) {

    default:
    case LayoutSection::Medium:
        return q->baseStyle()->keyHeightMedium();
        break;

    case LayoutSection::Small:
        return q->baseStyle()->keyHeightSmall();
        break;

    case LayoutSection::Large:
        return q->baseStyle()->keyHeightLarge();
        break;

    case LayoutSection::XLarge:
        return q->baseStyle()->keyHeightXLarge();
        break;

    case LayoutSection::XxLarge:
        return q->baseStyle()->keyHeightXxLarge();
        break;
    }

    return 0.0;
}

qreal MImKeyAreaPrivate::computeMaxNormalizedWidth() const
{
    qreal maxRowWidth = 0.0;

    for (int j = 0; j < section->rowCount(); ++j) {
        qreal rowWidth = 0.0;

        for (int i = 0; i < section->columnsAt(j); ++i) {
            const MImKeyModel *key = section->keyModel(j, i);
            rowWidth += normalizedKeyWidth(key);
        }

        maxRowWidth = qMax(maxRowWidth, rowWidth);
    }

    return maxRowWidth;
}

qreal MImKeyAreaPrivate::normalizedKeyWidth(const MImKeyModel *model) const
{
    Q_Q(const MImKeyArea);

    switch(model->width()) {

    case MImKeyModel::Small:
        return q->baseStyle()->keyWidthSmall();

    case MImKeyModel::Medium:
    case MImKeyModel::Stretched:
        return q->baseStyle()->keyWidthMedium();

    case MImKeyModel::Large:
        return q->baseStyle()->keyWidthLarge();

    case MImKeyModel::XLarge:
        return q->baseStyle()->keyWidthXLarge();

    case MImKeyModel::XxLarge:
        return q->baseStyle()->keyWidthXxLarge();
    }

    qWarning() << __PRETTY_FUNCTION__
               << "Could not compute normalized width from style";
    return 0.0;
}

void MImKeyAreaPrivate::clearKeyIds()
{
    idToKey.clear();
}

void MImKeyAreaPrivate::registerKeyId(MImKey *key)
{
    for (QList<MImKey *>::iterator iterator = idToKey.begin();
         iterator != idToKey.end();
         ++iterator) {
        if ((*iterator)->model().id() == key->model().id()) {
            *iterator = key;
            return;
        }
    }
    idToKey.append(key);
}

void MImKeyAreaPrivate::applyOverlayMode(QVector<QPair<qreal, qreal> > *offsets)
{
    if (not offsets || offsets->isEmpty()) {
        return;
    }

    Q_Q(const MImKeyArea);

    if (q->baseStyle()->enableOverlayMode()) {
        const QPair<qreal, qreal> firstRow(-FLT_MAX, offsets->first().second);
        offsets->replace(0, firstRow);

        const QPair<qreal, qreal> lastRow(offsets->last().first, FLT_MAX);
        offsets->replace(offsets->size() - 1, lastRow);
    }
}

// actual class implementation
MImKeyArea::MImKeyArea(const LayoutData::SharedLayoutSection &newSection,
                       bool usePopup,
                       QGraphicsWidget *parent)
    : MImAbstractKeyArea(new MImKeyAreaPrivate(newSection, this), usePopup, parent),
      d_ptr(static_cast<MImKeyAreaPrivate *>(MImAbstractKeyArea::d_ptr))
{
    d_ptr->cachedWidgetHeight = d_ptr->computeWidgetHeight();
    d_ptr->mMaxNormalizedWidth = d_ptr->computeMaxNormalizedWidth();

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    d_ptr->loadKeys();

    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

MImKeyArea::~MImKeyArea()
{
}

QSizeF MImKeyArea::sizeHint(Qt::SizeHint which,
                            const QSizeF &) const
{
    Q_D(const MImKeyArea);

    int width = 0;
    if (which == Qt::MaximumSize) {
        // We're willing to grow as much as we can. Some parent widget
        // will apply a constraint for this.
        width = QWIDGETSIZE_MAX;
    }

    return QSizeF(width, d->cachedWidgetHeight);
}

void MImKeyArea::drawReactiveAreas(MReactionMap *reactionMap,
                                   QGraphicsView *view)
{
#ifndef HAVE_REACTIONMAP
    Q_UNUSED(reactionMap);
    Q_UNUSED(view);
    return;
#else
    Q_D(MImKeyArea);

    reactionMap->setTransform(this, view);
    reactionMap->setDrawingValue(MImReactionMap::Press, MImReactionMap::Release);

    foreach (const MImKeyAreaPrivate::KeyRow &row, d->rowList) {
        // 'area' is used for key bounding rect coalescing, to improve
        // downsampling to reaction map resolution (usually display_size / 4).
        QRectF area;
        qreal lastRightBorder = 0.0f;

        foreach (const MImKey *const key, row.keys) {
            const QRectF rect = correctedReactionRect(key->buttonBoundingRect());

            // Check for spacers. If found, we draw the accummulated area and start from scratch.
            if (lastRightBorder < rect.left()) {
                reactionMap->fillRectangle(area);
                area = QRectF();
            }

            area |= rect;
            lastRightBorder = rect.right();
        }

        reactionMap->fillRectangle(area);
    }
#endif
}

void MImKeyArea::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *,
                       QWidget *)
{
    Q_D(MImKeyArea);
    mTimestamp("MImKeyArea", "start");
    const MImAbstractKeyAreaStyleContainer &style(baseStyle());

    const MScalableImage *background = style->backgroundImage();

    if (background) {
        background->draw(boundingRect().toRect(), painter);
    }

    const bool drawButtonBoundingRects(style->drawButtonBoundingRects());
    const bool drawButtonRects(style->drawButtonRects());

    // In case of HW acceleration, we want to avoid switching between textures, if possible
    // Draw backgrounds first, icons later:
    const KeyPainter kp(this, painter, KeyPainter::PaintBackground);

    foreach (const MImKeyAreaPrivate::KeyRow &row, d->rowList) {
        foreach (MImKey *key, row.keys) {
            key->setIgnoreOverriding(true);
            kp(key);
            d->drawDebugRects(painter, key,
                              drawButtonBoundingRects,
                              drawButtonRects);
        }
    }

    kp.drawIcons();

    if (style->drawReactiveAreas()) {
        d->drawDebugReactiveAreas(painter);
    }

    // Draw text next.
    // We use QGraphicsView::DontSavePainterState, so save/restore state manually
    // Not strictly needed at the moment, but prevents subtle breakage later
    painter->save();
    painter->setPen(style->fontColor());
    foreach (const MImKeyAreaPrivate::KeyRow &row, d->rowList) {
        // rowHasSecondaryLabel is needed for the vertical alignment of
        // secondary label purposes.
        bool rowHasSecondaryLabel = false;
        foreach (const MImKey *key, row.keys) {
            if (!key->secondaryLabel().isEmpty()) {
                rowHasSecondaryLabel = true;
                break;
            }
        }

        foreach (MImKey *key, row.keys) {
            painter->setFont(key->font());
            key->setSecondaryLabelEnabled(rowHasSecondaryLabel);

            QRectF rect = mapFromItem(key, key->labelRect()).boundingRect();
            painter->drawText(rect, Qt::AlignCenter, key->label());
        }
    }

    painter->setFont(style->secondaryFont());
    foreach (const MImKeyAreaPrivate::KeyRow &row, d->rowList) {
        foreach (MImKey *key, row.keys) {
            if (!key->secondaryLabel().isEmpty()) {
                QRectF rect = mapFromItem(key, key->secondaryLabelRect()).boundingRect();
                painter->drawText(rect, Qt::AlignCenter, key->secondaryLabel());
            }
            key->setIgnoreOverriding(false);
        }
    }
    painter->restore();

    mTimestamp("MImKeyArea", "end");
}

MImAbstractKey *MImKeyArea::keyAt(const QPoint &pos) const
{
    Q_D(const MImKeyArea);
    const int numRows = d->rowList.count();

    if (numRows == 0) {
        return 0;
    }

    const int rowIndex = binaryRangeFind<qreal>(pos.y(), d->rowOffsets);

    if (rowIndex == -1) {
        return 0;
    }

    const MImKeyAreaPrivate::KeyRow &currentRow = d->rowList.at(rowIndex);
    const int keyIndex = binaryRangeFind<qreal>(pos.x(), currentRow.keyOffsets);

    if (keyIndex == -1) {
        return 0;
    }

    return currentRow.keys.at(keyIndex);
}

void MImKeyArea::setShiftState(ModifierState newShiftState)
{
    Q_D(MImKeyArea);

    if (d->shiftKey) {
        d->shiftKey->setModifiers(newShiftState != ModifierClearState);
        d->shiftKey->setSelected(newShiftState == ModifierLockedState);
    }
}

void MImKeyArea::modifiersChanged(const bool shift,
                                  const QChar &accent)
{
    Q_D(MImKeyArea);

    for (MImKeyAreaPrivate::RowIterator row(d->rowList.begin()); row != d->rowList.end(); ++row) {
        foreach (MImKey *key, row->keys) {
            // Shift key and selected keys are detached from the normal level changing.
            if (key != d->shiftKey
                && key != d->toggleKey
                && key != d->composeKey
                && key->state() != MImAbstractKey::Selected) {
                key->setModifiers(shift, accent);
            }
        }
    }

    update();
}

void MImKeyArea::updateKeyGeometries(const int newAvailableWidth)
{
    if (sectionModel()->maxColumns() == 0) {
        return;
    }

    // Size specified in CSS can override width, for fixed key width layouts:
    Q_D(MImKeyArea);

    const int effectiveWidth = (baseStyle()->useFixedKeyWidth()
                                && (baseStyle()->size().width() > -1) ? baseStyle()->size().width()
                                                                      : newAvailableWidth);

    d->cachedWidgetHeight = d->computeWidgetHeight();

    KeyGeometryUpdater updater = KeyGeometryUpdater(baseStyle(), effectiveWidth,
                                                    d->computeMaxNormalizedWidth());

    for (MImKeyAreaPrivate::RowIterator rowIter = d->rowList.begin();
         rowIter != d->rowList.end();
         ++rowIter) {
        int flags = KeyGeometryUpdater::NormalRow;
        const int rowIndex = std::distance(d->rowList.begin(), rowIter);

        if (rowIter == d->rowList.begin()) {
            flags |= KeyGeometryUpdater::FirstRow;
        }

        if (rowIter == d->rowList.end() - 1) {
            flags |= KeyGeometryUpdater::LastRow;
        }

        updater.processRow(rowIter->keys,
                           d->preferredKeyHeight(rowIndex),
                           sectionModel()->spacerIndices(rowIndex),
                           flags);
        rowIter->keyOffsets = updater.keyOffsets();
    }

    d->rowOffsets = updater.rowOffsets();
    d->applyOverlayMode(&d->rowOffsets);
    mRelativeKeyBaseWidth = updater.relativeKeyWidth();

    // Positions may have changed, rebuild text layout.
    update();
}

QRectF MImKeyArea::boundingRect() const
{
    Q_D(const MImKeyArea);

    return QRectF(0, 0, size().width(), d->cachedWidgetHeight);
}

void MImKeyArea::onThemeChangeCompleted()
{
    Q_D(MImKeyArea);

    d->mMaxNormalizedWidth = d->computeMaxNormalizedWidth();
    d->cachedWidgetHeight = d->computeWidgetHeight();
    d->stylingCache->primary   = QFontMetrics(baseStyle()->font());
    d->stylingCache->secondary = QFontMetrics(baseStyle()->secondaryFont());

    foreach (const MImKeyAreaPrivate::KeyRow &row, d->rowList) {
        foreach (MImKey *key, row.keys) {
            if (key) {
                key->invalidateLabelPos();
            }
        }
    }

    MImAbstractKeyArea::onThemeChangeCompleted();
    update();
}

void MImKeyArea::applyStyle()
{
    Q_D(MImKeyArea);

    d->stylingCache->primary   = QFontMetrics(baseStyle()->font());
    d->stylingCache->secondary = QFontMetrics(baseStyle()->secondaryFont());
}

QList<const MImAbstractKey *> MImKeyArea::keys() const
{
    Q_D(const MImKeyArea);
    QList<const MImAbstractKey *> keyList;

    foreach (const MImKeyAreaPrivate::KeyRow &row, d->rowList) {
        foreach (const MImKey *key, row.keys) {
            keyList.append(key);
        }
    }

    return keyList;
}

MImAbstractKey * MImKeyArea::findKey(const QString &id)
{
    Q_D(MImKeyArea);
    MImKey *key = 0;

    for (QList<MImKey *>::const_iterator iterator = d->idToKey.begin();
         iterator != d->idToKey.end();
         ++iterator) {
        if ((*iterator)->model().id() == id) {
            key = *iterator;
            break;
        }
    }

    return key;
}

void MImKeyArea::setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides)
{
    Q_D(MImKeyArea);

    for (QList<MImKey *>::const_iterator iterator = d->idToKey.begin();
         iterator != d->idToKey.end();
         ++iterator) {
        MImKey *key = *iterator;
        if (overrides.contains(key->model().id())) {
            QSharedPointer<MKeyOverride> override = overrides[key->model().id()];
            // change the im key according incoming overrides.
            if (!override->enabled()) {
                releaseKey(key);
            }
            key->setKeyOverride(override);
            connect(override.data(), SIGNAL(keyAttributesChanged(QString, MKeyOverride::KeyOverrideAttributes)),
                    this,            SLOT(updateKeyAttributes(QString, MKeyOverride::KeyOverrideAttributes)));
        } else {
            if (key->keyOverride()) {
                disconnect((*iterator)->keyOverride().data(), 0,
                            this,                             0);
            }
            key->resetKeyOverride();
        }
    }
}

void MImKeyArea::updateKeyAttributes(const QString &keyId, MKeyOverride::KeyOverrideAttributes attributes)
{
    MImKey *key = static_cast<MImKey *>(findKey(keyId));

    if (key) {
        if ((attributes & MKeyOverride::Enabled)) {
            releaseKey(key);
        }

        key->updateOverrideAttributes(attributes);
    }
}

void MImKeyArea::releaseKey(MImKey *key)
{
    if (key->touchPointCount() <= 0 || !key->enabled()) {
        return;
    }

    Q_D(MImKeyArea);
    MImKeyVisitor::SpecialKeyFinder finder;
    MImAbstractKey::visitActiveKeys(&finder);
    const bool hasActiveShiftKeys = (finder.shiftKey() != 0);

    key->resetTouchPointCount();
    emit keyReleased(key,
                     (finder.deadKey() ? finder.deadKey()->label() : QString()),
                     hasActiveShiftKeys || d->isUpperCase());
}

void MImKeyArea::setContentType(M::TextContentType type)
{
    static const MImKeyBinding bindAt("@");
    static const MImKeyBinding bindSlash("/");

    MImKey *key = static_cast<MImKey *>(findKey(emailUrlKey));

    if (!key) {
        return;
    }

    switch(type) {
    case M::UrlContentType:
        key->overrideBinding(&bindSlash);
        break;
    case M::EmailContentType:
        key->overrideBinding(&bindAt);
        break;
    default:
        key->overrideBinding(0);
        break;
    }
    update();
}

void MImKeyArea::setToggleKeyState(bool on)
{
    Q_D(MImKeyArea);
    if (d->toggleKey) {
        d->toggleKey->setSelected(on);
        update();
    }
}

void MImKeyArea::setComposeKeyState(bool isComposing)
{
    Q_D(MImKeyArea);
    if (d->composeKey) {
        d->composeKey->setComposing(isComposing);
        update();
    }
}

void MImKeyArea::resetActiveKeys()
{
    Q_D(MImKeyArea);
    foreach (const MImKeyAreaPrivate::KeyRow &row, d->rowList) {
        foreach (MImKey *key, row.keys) {
            key->setSelected(false);
            key->resetTouchPointCount();
        }
    }
    setComposeKeyState(false);
}
