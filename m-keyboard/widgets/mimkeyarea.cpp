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

#include "mvirtualkeyboardstyle.h"
#include "mimkeyarea.h"

#include <QDebug>
#include <QEvent>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QTextCharFormat>
#include <QTextLine>
#include <QList>

#include <MApplication>
#include <MComponentData>
#include <MFeedbackPlayer>
#include <mplainwindow.h>
#include <mreactionmap.h>
#include <MTheme>
#include <MTimestamp>

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
            PaintBackgroundAndIcon  //!< Draw both, background and icon, in one go
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

            const MScalableImage *background = 0;

            switch (key->state()) {

            case MImAbstractKey::Normal:
                switch (key->model().style()) {
                case MImKeyModel::SpecialStyle:
                    background = keyArea->baseStyle()->keyBackgroundSpecial();
                    break;
                case MImKeyModel::DeadkeyStyle:
                    background = keyArea->baseStyle()->keyBackgroundDeadkey();
                    break;
                case MImKeyModel::NormalStyle:
                default:
                    background = keyArea->baseStyle()->keyBackground();
                    break;
                }
                break;

            case MImAbstractKey::Pressed:
                switch (key->model().style()) {
                case MImKeyModel::SpecialStyle:
                    background = keyArea->baseStyle()->keyBackgroundSpecialPressed();
                    break;
                case MImKeyModel::DeadkeyStyle:
                    background = keyArea->baseStyle()->keyBackgroundDeadkeyPressed();
                    break;
                case MImKeyModel::NormalStyle:
                default:
                    background = keyArea->baseStyle()->keyBackgroundPressed();
                    break;
                }
                break;

            case MImAbstractKey::Selected:
                switch (key->model().style()) {
                case MImKeyModel::SpecialStyle:
                    background = keyArea->baseStyle()->keyBackgroundSpecialSelected();
                    break;
                case MImKeyModel::DeadkeyStyle:
                    background = keyArea->baseStyle()->keyBackgroundDeadkeySelected();
                    break;
                case MImKeyModel::NormalStyle:
                default:
                    background = keyArea->baseStyle()->keyBackgroundSelected();
                    break;
                }
                break;

            default:
                break;
            }

            if (background) {
                background->draw(key->buttonRect().toRect(), painter);
            }
        }
    };

    class KeyGeometryUpdater
    {
    public:
        typedef QVector<QPair<qreal, qreal> > OffsetList;

        // RelativeWidth not supported yet:
        enum KeyWidthMode {
            RelativeWidth,
            FixedWidth
        };

        enum RowTypeFlags {
            NormalRow = 0,
            FirstRow = 1,
            LastRow = 2
        };

    private:
        const MImAbstractKeyAreaStyleContainer &style;
        const qreal availableWidth;
        KeyWidthMode mode;

        // key bounding rect related members:
        QPoint currentPos;
        OffsetList mRowOffsets;
        OffsetList mKeyOffsets;

        // spacer related members:
        QList<int> spacerIndices;
        qreal spacerWidth;

    public:
        explicit KeyGeometryUpdater(const MImAbstractKeyAreaStyleContainer &newStyle,
                                    const qreal newAvailableWidth,
                                    KeyWidthMode newMode = RelativeWidth)
            : style(newStyle)
            , availableWidth(newAvailableWidth)
            , mode(newMode)
            , spacerWidth(0.0)
        {}

        OffsetList rowOffsets() const
        {
            return mRowOffsets;
        }

        OffsetList keyOffsets() const
        {
            return mKeyOffsets;
        }

        void processRow(const QList<MImKey *> &row,
                        qreal newKeyHeight,
                        const QList<int> &newSpacerIndices,
                        int flags = NormalRow)
        {

            currentPos.rx() = 0;
            spacerIndices = newSpacerIndices;
            const QSizeF rowSize = updateGeometry(row, newKeyHeight, flags);

            spacerWidth = qAbs((availableWidth - rowSize.width())
                               / ((spacerIndices.count() ==  0) ? 1 : spacerIndices.count()));

            const qreal nextPosY = currentPos.y() + rowSize.height();
            mRowOffsets.append(QPair<qreal, qreal>(currentPos.y(), nextPosY));
            mKeyOffsets.clear();

            updatePosition(row);
            currentPos.ry() = nextPosY;
        }

    private:
        QSizeF updateGeometry(const QList<MImKey *> &row,
                              qreal keyHeight,
                              int flags) const
        {
            const qreal marginTop = (flags & FirstRow ? style->firstRowMarginTop()
                                                      : style->keyMarginTop());
            const qreal marginBottom = (flags & LastRow ? style->lastRowMarginBottom()
                                                        : style->keyMarginBottom());
            qreal rowWidth = 0.0;

            foreach (MImKey *const key, row) {
                // TODO: reimpl stretch keys
                MImKey::Geometry g(key->geometry());
                g.height = keyHeight;
                g.width = key->preferredFixedWidth();
                g.marginLeft = style->keyMarginLeft();
                g.marginTop = marginTop;
                g.marginRight = style->keyMarginRight();
                g.marginBottom = marginBottom;
                key->setGeometry(g);

                rowWidth += key->buttonBoundingRect().width();
            }

            MImKey *first = row.first();
            MImKey::Geometry firstG(first->geometry());
            rowWidth -= firstG.marginLeft;
            firstG.marginLeft = style->firstKeyMarginLeft();
            rowWidth += firstG.marginLeft;
            first->setGeometry(firstG);

            MImKey *last = row.last();
            MImKey::Geometry lastG(last->geometry());
            rowWidth -= lastG.marginRight;
            lastG.marginRight = style->lastKeyMarginRight();
            rowWidth += lastG.marginRight;
            last->setGeometry(lastG);

            return QSizeF(rowWidth, first->buttonBoundingRect().height());
        }

        void updatePosition(const QList<MImKey *> &row)
        {
            int index = 0;
            currentPos.rx() += spacerIndices.count(-1) * spacerWidth;

            foreach (MImKey *const key, row) {
                key->setPos(currentPos);

                const qreal nextPosX = currentPos.x() + key->buttonBoundingRect().width();
                mKeyOffsets.append(QPair<qreal, qreal>(currentPos.x(), nextPosX));

                currentPos.rx() = nextPosX;
                currentPos.rx() += spacerIndices.count(index) * spacerWidth;
                ++index;
            }
        }
    };
}

MImKeyArea::MImKeyArea(const LayoutData::SharedLayoutSection &newSection,
                       bool usePopup,
                       QGraphicsWidget *parent)
    : MImAbstractKeyArea(newSection, usePopup, parent),
      rowList(newSection->rowCount()),
      cachedWidgetHeight(computeWidgetHeight()),
      mMaxNormalizedWidth(computeMaxNormalizedWidth()),
      shiftKey(0),
      textDirty(false),
      cachedBackgroundDirty(true),
      hasCachedBackground(false),
      equalWidthKeys(true)
{
    textLayout.setCacheEnabled(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    loadKeys();
}

MImKeyArea::~MImKeyArea()
{
    for (RowIterator rowIter(rowList.begin()); rowIter != rowList.end(); ++rowIter) {
        qDeleteAll(rowIter->keys);
        rowIter->keys.clear();
    }
}

QSizeF MImKeyArea::sizeHint(Qt::SizeHint which,
                            const QSizeF &) const
{
    int width = 0;
    if (which == Qt::MaximumSize) {
        // We're willing to grow as much as we can. Some parent widget
        // will apply a constraint for this.
        width = QWIDGETSIZE_MAX;
    }
    return QSizeF(width, cachedWidgetHeight);
}

void MImKeyArea::drawReactiveAreas(MReactionMap *reactionMap,
                                   QGraphicsView *view)
{
    reactionMap->setTransform(this, view);
    reactionMap->setReactiveDrawingValue();

    foreach (const KeyRow &row, rowList) {
        foreach (const MImKey *const key, row.keys) {
            reactionMap->fillRectangle(key->buttonBoundingRect());
        }
    }
}

void MImKeyArea::loadKeys()
{
    const int numRows = rowCount();

    RowIterator rowIter(rowList.begin());

    for (int row = 0; row != numRows; ++row, ++rowIter) {
        const int numColumns = sectionModel()->columnsAt(row);

        rowIter->stretchKey = 0;

        // Add keys
        for (int col = 0; col < numColumns; ++col) {
            // Parameters to fetch from base class.
            MImKeyModel *dataKey = sectionModel()->keyModel(row, col);
            MImKey *key = new MImKey(*dataKey, baseStyle(), *this);

            // TODO: Remove restriction to have only one shift key per layout?
            if (dataKey->binding()->action() == MImKeyBinding::ActionShift) {
                shiftKey = key;
            }

            // Only one stretching item per row.
            if (!rowIter->stretchKey) {
                rowIter->stretchKey = (dataKey->width() == MImKeyModel::Stretched ? key : 0);
            }

            rowIter->keys.append(key);
        }
    }

    updateGeometry();
}

void MImKeyArea::buildTextLayout()
{
    textDirty = false;

    textLayout.clearLayout();

    QList<QTextLayout::FormatRange> formatList;
    QTextCharFormat secondaryFormat;
    secondaryFormat.setFont(baseStyle()->secondaryFont());

    // QTextLayout requires text content to be set before creating QTextLines.
    // While concatenating the text, also build 'additional formats' used for secondary
    // labels. This must be done before QTextLayout::beginLayout().
    QString labelContent;

    foreach (const KeyRow &row, rowList) {
        foreach (const MImKey *key, row.keys) {
            // primary label
            QString label = key->label();

            if (!label.isEmpty()) {
                // Add whitespace for QTextLine to be able to cut.
                labelContent += label + " ";

                // try secondary label
                label = key->secondaryLabel();

                if (!label.isEmpty()) {
                    // Add formatting for this secondary label.
                    QTextLayout::FormatRange formatRange = {labelContent.length(),
                                                            label.length(),
                                                            secondaryFormat};
                    formatList.append(formatRange);
                    labelContent += label + " ";
                }
            }
        }
    }

    // Apply the formats
    if (!formatList.isEmpty()) {
        textLayout.setAdditionalFormats(formatList);
    }

    QFontMetrics fm(baseStyle()->font());
    QFontMetrics secondaryFm(secondaryFormat.font());
    const int labelHeight = fm.height();
    const int secondaryLabelHeight = secondaryFm.height();
    const int topMargin = baseStyle()->labelMarginTop();
    const int labelLeftWithSecondary = baseStyle()->labelMarginLeftWithSecondary();
    const int secondarySeparation = baseStyle()->secondaryLabelSeparation();
    const bool landscape = (MPlainWindow::instance()->orientation() == M::Landscape);

    textLayout.setFont(baseStyle()->font());
    textLayout.setText(labelContent);

    textLayout.beginLayout();

    for (RowIterator row(rowList.begin()); row != rowList.end(); ++row) {
        // rowHasSecondaryLabel is needed for the vertical alignment of
        // secondary label purposes.
        bool rowHasSecondaryLabel = false;

        foreach (const MImKey *key, row->keys) {
            if (!key->secondaryLabel().isEmpty()) {
                rowHasSecondaryLabel = true;
            }
        }

        foreach (const MImKey *key, row->keys) {
            const QString &label(key->label());
            const QString &secondary(key->secondaryLabel());
            QPoint labelPos;
            QPoint secondaryLabelPos;

            // We must not create a new QTextLine if there is no label.
            if (label.isEmpty()) {
                continue;
            }

            const QRectF &keyRect = key->buttonRect();

            if (!rowHasSecondaryLabel) {
                // All horizontally centered.
                labelPos = fm.boundingRect(keyRect.x(),
                                           keyRect.y(),
                                           keyRect.width(),
                                           keyRect.height(),
                                           Qt::AlignCenter,
                                           label).topLeft();
            } else {
                // Calculate position for both primary and secondary label.
                // We only have secondary labels in phone number layouts (with sym being exception)
                // so this follows their styling.

                // In landscape the secondary labels are below the primary ones. In portrait,
                // secondary labels are horizontally next to primary labels.
                if (landscape) {
                    // primary label: horizontally centered, top margin defines y
                    // secondary: horizontally centered, primary bottom + separation margin defines y
                    const int primaryY = keyRect.top() + topMargin;
                    labelPos.setX(keyRect.center().x() - fm.width(label) / 2);
                    labelPos.setY(primaryY);
                    if (!secondary.isEmpty()) {
                        secondaryLabelPos.setX(keyRect.center().x() - secondaryFm.width(secondary) / 2);
                        secondaryLabelPos.setY(primaryY + labelHeight + secondarySeparation);
                    }
                } else {
                    // primary label: horizontally according to left margin, vertically centered
                    // secondary: horizontally on right of primary + separation margin, vertically centered
                    const int primaryX = keyRect.left() + labelLeftWithSecondary;
                    labelPos.setX(primaryX);
                    labelPos.setY(keyRect.center().y() - labelHeight / 2);
                    if (!secondary.isEmpty()) {
                        secondaryLabelPos.setX(primaryX + fm.width(label) + secondarySeparation);
                        secondaryLabelPos.setY(keyRect.center().y() - secondaryLabelHeight / 2);
                    }
                }
            }

            // We now have positions, let's create some QTextLines.

            // Create the primary label
            QTextLine line = textLayout.createLine();
            if (!line.isValid()) {
                // We are getting out of sync anyway so no point in continuing.
                goto endLayout;
            }
            line.setNumColumns(label.length()); // will be seeked forward until next whitespace
            line.setPosition(labelPos);

            // Same for secondary label
            if (!secondary.isEmpty()) {
                line = textLayout.createLine();
                if (!line.isValid()) {
                    goto endLayout;
                }
                line.setNumColumns(secondary.length());
                line.setPosition(secondaryLabelPos);
            }
        }
    }

endLayout:
    textLayout.endLayout();
}

qreal MImKeyArea::computeWidgetHeight() const
{
    qreal height = -(baseStyle()->spacingVertical())
                   + baseStyle()->paddingTop()
                   + baseStyle()->paddingBottom();

    for (int index = 0; index < rowList.count(); ++index) {
        height += preferredRowHeight(index);
        height += baseStyle()->spacingVertical();
    }

    return qMax<qreal>(0.0, height);
}

void MImKeyArea::paint(QPainter *onScreenPainter,
                       const QStyleOptionGraphicsItem *,
                       QWidget *)
{
    mTimestamp("MImKeyArea", "start");
    const MImAbstractKeyAreaStyleContainer &style(baseStyle());

    // Key areas are disabled during animations. Once we are animated, it
    // makes no sense to maintain the offscreen cache (especially when
    // HW-accelerated). Therefore, we draw directly to the screen.
    // However, we need to remember whether we drew a current version of the
    // key area to the offscreen cache at all (that's what hasCachedBackground
    // is for).
    QPainter *currentPainter = onScreenPainter;
    if (cachedBackgroundDirty
        || !isEnabled()
        || !hasCachedBackground) {

        // TODO: find out why size()'s height is incorrect for popup.
        initCachedBackground(QSize(size().width(), cachedWidgetHeight));
        QPainter offScreenPainter(cachedBackground.get());

        if (isEnabled()) {
            currentPainter = &offScreenPainter;
            hasCachedBackground = true;
        }

        const MScalableImage *background = style->backgroundImage();

        if (background) {
            background->draw(boundingRect().toRect(), currentPainter);
        }

        const bool drawButtonBoundingRects(style->drawButtonBoundingRects());
        const bool drawButtonRects(style->drawButtonRects());

        // In case of HW acceleration, we want to avoid switching between textures, if possible
        // Draw backgrounds first, icons later:
        const KeyPainter kp(this, currentPainter, KeyPainter::PaintBackground);

        foreach (const KeyRow &row, rowList) {
            foreach (const MImKey *key, row.keys) {
                kp(key);
                drawDebugRects(currentPainter, key,
                               drawButtonBoundingRects,
                               drawButtonRects);
            }
        }

        kp.drawIcons();

        if (style->drawReactiveAreas()) {
            drawDebugReactiveAreas(currentPainter);
        }
    }

    if (hasCachedBackground) {
        onScreenPainter->drawPixmap(boundingRect().toRect(), *cachedBackground.get());
    }

    KeyPainter kp(this, onScreenPainter, KeyPainter::PaintBackground);
    MImAbstractKey::visitActiveKeys(&kp);
    kp.drawIcons();

    if (textDirty) {
        buildTextLayout();
    }

    // Draw text next.
    onScreenPainter->setPen(style->fontColor());
    textLayout.draw(onScreenPainter, QPoint());

    cachedBackgroundDirty = false;
    mTimestamp("MImKeyArea", "end");
}

void MImKeyArea::drawDebugRects(QPainter *painter,
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

void MImKeyArea::drawDebugReactiveAreas(QPainter *painter)
{
    painter->save();

    for (int rowIdx = 0; rowIdx < rowList.size(); ++rowIdx) {
        QPair<qreal, qreal> rowPair = rowOffsets[rowIdx];
        painter->setPen(Qt::darkMagenta);
        painter->drawLine(QPointF(0, rowPair.first),
                          QPointF(size().width(), rowPair.first));

        painter->setPen(Qt::magenta);
        painter->drawLine(QPointF(0, rowPair.second),
                          QPointF(size().width(), rowPair.second));

        const QVector<QPair<qreal, qreal> > &keyOffsets = rowList[rowIdx].keyOffsets;

        for(int colIdx = 0; colIdx < keyOffsets.size(); ++colIdx) {
            QPair<qreal, qreal> colPair = keyOffsets[colIdx];
            painter->setPen(Qt::cyan);
            painter->drawLine(QPointF(colPair.first, rowPair.first),
                              QPointF(colPair.first, rowPair.second));

            painter->setPen(Qt::darkCyan);
            painter->drawLine(QPointF(colPair.second, rowPair.first),
                              QPointF(colPair.second, rowPair.second));
        }
    }

    painter->restore();
}

void MImKeyArea::initCachedBackground(const QSize &newSize)
{
    if (cachedBackground.get() && newSize == cachedBackground->size()) {
        return; // already initialized
    }

    cachedBackground.reset(new QPixmap(newSize));
    cachedBackground->fill(Qt::transparent);
    hasCachedBackground = false;
}

MImAbstractKey *MImKeyArea::keyAt(const QPoint &pos) const
{
    const int numRows = rowList.count();

    if (numRows == 0) {
        return 0;
    }

    const int rowIndex = binaryRangeFind<qreal>(pos.y(), rowOffsets);

    if (rowIndex == -1) {
        return 0;
    }

    const KeyRow &currentRow = rowList.at(rowIndex);
    const int keyIndex = binaryRangeFind<qreal>(pos.x(), currentRow.keyOffsets);

    if (keyIndex == -1) {
        return 0;
    }

    return currentRow.keys.at(keyIndex);
}

void MImKeyArea::setShiftState(ModifierState newShiftState)
{
    if (shiftKey) {
        shiftKey->setModifiers(newShiftState != ModifierClearState);
        shiftKey->setSelected(newShiftState == ModifierLockedState);
    }
}

void MImKeyArea::modifiersChanged(const bool shift,
                                  const QChar &accent)
{
    for (RowIterator row(rowList.begin()); row != rowList.end(); ++row) {
        foreach (MImKey *key, row->keys) {
            // Shift key and selected keys are detached from the normal level changing.
            if (key != this->shiftKey
                && key->state() != MImAbstractKey::Selected) {
                key->setModifiers(shift, accent);
            }
        }
    }

    textDirty = true;
}

void MImKeyArea::updateKeyGeometries(const int newAvailableWidth)
{
    if (sectionModel()->maxColumns() == 0) {
        return;
    }

    cachedWidgetHeight = computeWidgetHeight();
    initCachedBackground(QSize(newAvailableWidth, cachedWidgetHeight));

    KeyGeometryUpdater updater = KeyGeometryUpdater(baseStyle(), newAvailableWidth);

    for (RowIterator rowIter = rowList.begin();
         rowIter != rowList.end();
         ++rowIter) {
        const int rowIndex = std::distance(rowList.begin(), rowIter);

        int flags = KeyGeometryUpdater::NormalRow;

        if (rowIter == rowList.begin()) {
            flags |= KeyGeometryUpdater::FirstRow;
        }

        if (rowIter == rowList.end() - 1) {
            flags |= KeyGeometryUpdater::LastRow;
        }

        updater.processRow(rowIter->keys,
                           preferredRowHeight(rowIndex),
                           sectionModel()->spacerIndices(rowIndex),
                           flags);
        rowIter->keyOffsets = updater.keyOffsets();
    }

    rowOffsets = updater.rowOffsets();

    // Positions may have changed, rebuild text layout.
    textDirty = true;
}

        //rowOffsets.append();


        // Update row width
        /*
        qreal rowWidth = 0;
        foreach (MImKey *key, row->keys) {
            key->width = key->preferredWidth(mRelativeKeyBaseWidth, HorizontalSpacing);
            rowWidth += key->width + HorizontalSpacing;
        }
        rowWidth -= HorizontalSpacing;

        qreal availableWidthForSpacers = 0;
        const QList<int> spacerIndices = sectionModel()->spacerIndices(row - rowList.begin());
        int spacerCount = row->stretchKey ? spacerIndices.count() + 1
                                          : spacerIndices.count();

        if (row->stretchKey) {
            rowWidth -=  row->stretchKey->width;

            // Handle the case of one stretch key/no other spacer elments directly:
            if (spacerCount == 1) {
                row->stretchKey->width = availableWidth - rowWidth;
                rowWidth = availableWidth;
                spacerCount = 0;
            }
        }

        if ((spacerCount > 0) && (availableWidth > rowWidth)) {
            availableWidthForSpacers = (availableWidth - rowWidth) / spacerCount;

            if (row->stretchKey) {
                row->stretchKey->width = availableWidthForSpacers;
            }
        }
        */

        // A spacer with an index of -1 means it was put before any key in that row.
        // Also add layout padding:
       // qreal x = style->paddingLeft() + spacerIndices.count(-1) * availableWidthForSpacers;

/*
        for (int keyIndex = 0; keyIndex < rowIter->keys.count(); ++keyIndex) {
            MImKey *const key = rowIter->keys.at(keyIndex);

            // Store the key offsets for fast key lookup:
            rowIter->keyOffsets.append(QPair<qreal, qreal>(key->cachedBoundingRect.left(),
                                                       key->cachedBoundingRect.right()));

            // Increase x to the next key bounding rect border.
            x += key->width + HorizontalSpacing;

            // Our spacerIndex is a multi-set, hence we need to add
            // availableWidthForSpacers for every ocurrence of spacerIndex:
            x += spacerIndices.count(keyIndex) * availableWidthForSpacers;
        }

        y += br.height();
    }

}
*/

QRectF MImKeyArea::boundingRect() const
{
    return QRectF(0, 0, size().width(), cachedWidgetHeight);
}

qreal MImKeyArea::preferredRowHeight(int row) const
{
    const qreal normalHeight = baseStyle()->keyHeight();

    switch (sectionModel()->rowHeightType(row)) {

    case LayoutSection::Small:
        return normalHeight * baseStyle()->rowHeightSmall();
        break;

    case LayoutSection::Medium:
        return normalHeight * baseStyle()->rowHeightMedium();
        break;

    case LayoutSection::Large:
        return normalHeight * baseStyle()->rowHeightLarge();
        break;

    case LayoutSection::XLarge:
        return normalHeight * baseStyle()->rowHeightXLarge();
        break;

    case LayoutSection::XxLarge:
        return normalHeight * baseStyle()->rowHeightXxLarge();
        break;
    }

    return 0.0;
}

qreal MImKeyArea::computeMaxNormalizedWidth() const
{
    qreal maxRowWidth = 0.0;

    for (int j = 0; j < sectionModel()->rowCount(); ++j) {
        qreal rowWidth = 0.0;

        for (int i = 0; i < sectionModel()->columnsAt(j); ++i) {
            const MImKeyModel *key = sectionModel()->keyModel(j, i);
            rowWidth += normalizedKeyWidth(key);
        }

        maxRowWidth = qMax(maxRowWidth, rowWidth);
    }

    return maxRowWidth;
}

qreal MImKeyArea::normalizedKeyWidth(const MImKeyModel *model) const
{
    switch(model->width()) {

    case MImKeyModel::Small:
        return baseStyle()->keyWidthSmall();

    case MImKeyModel::Medium:
    case MImKeyModel::Stretched:
        return baseStyle()->keyWidthMedium();

    case MImKeyModel::Large:
        return baseStyle()->keyWidthLarge();

    case MImKeyModel::XLarge:
        return baseStyle()->keyWidthXLarge();

    case MImKeyModel::XxLarge:
        return baseStyle()->keyWidthXxLarge();
    }

    qWarning() << __PRETTY_FUNCTION__
               << "Could not compute normalized width from style";
    return 0.0;
}

void MImKeyArea::onThemeChangeCompleted()
{
    mMaxNormalizedWidth = computeMaxNormalizedWidth();
    MImAbstractKeyArea::onThemeChangeCompleted();
    buildTextLayout();
}

void MImKeyArea::handleVisibilityChanged(bool)
{
    invalidateBackgroundCache();
}

void MImKeyArea::invalidateBackgroundCache()
{
    cachedBackgroundDirty = true;
    hasCachedBackground = false;
}

QList<const MImAbstractKey *> MImKeyArea::keys() const
{
    QList<const MImAbstractKey *> keyList;

    foreach (const KeyRow &row, rowList) {
        foreach (const MImKey *key, row.keys) {
            keyList.append(key);
        }
    }

    return keyList;
}
