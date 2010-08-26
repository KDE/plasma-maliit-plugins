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
#include "singlewidgetbuttonarea.h"

#include <QDebug>
#include <QEvent>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QTextCharFormat>
#include <QTextLine>

#include <MApplication>
#include <MComponentData>
#include <MFeedbackPlayer>
#include <mplainwindow.h>
#include <mreactionmap.h>
#include <MTheme>

namespace {
    SingleWidgetButton * binaryButtonFind(int x,
                                          const QList<SingleWidgetButton *> &buttons)
    {
        int lowerBound = 0;
        int upperBound = buttons.size() - 1;

        while (lowerBound <= upperBound) {
            const int pivot = (lowerBound + upperBound) / 2;
            SingleWidgetButton *current = buttons.at(pivot);

            if (x < current->buttonBoundingRect().topLeft().x()) {
                upperBound = pivot - 1;
            } else if (x > current->buttonBoundingRect().topRight().x()) {
                lowerBound = pivot + 1;
            } else {
                return current;
            }
        }

        return 0;
    }
}

SingleWidgetButtonArea::SingleWidgetButtonArea(const MVirtualKeyboardStyleContainer *style,
                                               const QSharedPointer<const LayoutSection> &sectionModel,
                                               ButtonSizeScheme buttonSizeScheme,
                                               bool usePopup,
                                               QGraphicsWidget *parent)
    : KeyButtonArea(style, sectionModel, buttonSizeScheme, usePopup, parent),
      rowHeight(0),
      rowList(sectionModel->rowCount()),
      symState(SymIndicatorInactive),
      symIndicatorButton(0),
      shiftButton(0),
      textDirty(false),
      equalWidthButtons(true)
{
    textLayout.setCacheEnabled(true);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Initially deactivate sym page indicator.
    deactivateIndicator();
    loadKeys();
}

SingleWidgetButtonArea::~SingleWidgetButtonArea()
{
    // Release any key that might be pressed before destroying them.
    clearActiveKeys();

    for (RowIterator rowIter(rowList.begin()); rowIter != rowList.end(); ++rowIter) {
        qDeleteAll(rowIter->buttons);
        rowIter->buttons.clear();
    }
}

QSizeF SingleWidgetButtonArea::sizeHint(Qt::SizeHint which, const QSizeF &/*constraint*/) const
{
    const int widgetHeight = rowList.count() * rowHeight - style()->spacingVertical();

    int width = 0;
    if (which == Qt::MaximumSize) {
        // We're willing to grow as much as we can. Some parent widget
        // will apply a constraint for this.
        width = QWIDGETSIZE_MAX;
    }
    return QSizeF(width, widgetHeight);
}

void SingleWidgetButtonArea::drawReactiveAreas(MReactionMap *reactionMap, QGraphicsView *view)
{
    reactionMap->setTransform(this, view);
    reactionMap->setReactiveDrawingValue();

    int y = -style()->spacingVertical() / 2;
    for (RowIterator row(rowList.begin()); row != rowList.end(); ++row) {
        reactionMap->fillRectangle(row->offset, y, row->cachedWidth, rowHeight);
        y += rowHeight;
    }
}

void SingleWidgetButtonArea::loadKeys()
{
    const int numRows = rowCount();

    RowIterator rowIter(rowList.begin());

    for (int row = 0; row != numRows; ++row, ++rowIter) {
        const int numColumns = sectionModel()->columnsAt(row);

        rowIter->offset = 0;
        rowIter->stretchButton = 0;

        // Add buttons
        for (int col = 0; col < numColumns; ++col) {
            // Parameters to fetch from base class.
            VKBDataKey *dataKey = sectionModel()->vkbKey(row, col);
            SingleWidgetButton *button = new SingleWidgetButton(*dataKey, style(), *this);

            if (dataKey->binding()->action() == KeyBinding::ActionSym) {
                // Save pointer for easier use.
                this->symIndicatorButton = button;
            } else if (dataKey->binding()->action() == KeyBinding::ActionShift) {
                this->shiftButton = button;
            }

            const int vSpacing = style()->spacingVertical();
            rowHeight = qMax<qreal>(rowHeight - vSpacing, style()->keyNormalSize().height()) + vSpacing;

            // Only one stretching item per row.
            if (!rowIter->stretchButton) {
                rowIter->stretchButton = button;
            }

            rowIter->buttons.append(button);
        }
    } // end foreach row

    updateGeometry();
}

void SingleWidgetButtonArea::buildTextLayout()
{
    textDirty = false;

    textLayout.clearLayout();

    QList<QTextLayout::FormatRange> formatList;
    QTextCharFormat secondaryFormat;
    secondaryFormat.setFont(style()->secondaryFont());

    // QTextLayout requires text content to be set before creating QTextLines.
    // While concatenating the text, also build 'additional formats' used for secondary
    // labels. This must be done before QTextLayout::beginLayout().
    QString labelContent;
    foreach (const ButtonRow &row, rowList) {
        foreach (const SingleWidgetButton *button, row.buttons) {

            // primary label
            QString label = button->label();
            if (!label.isEmpty()) {
                // Add whitespace for QTextLine to be able to cut.
                labelContent += label + " ";

                // We don't support styling of text for invidual buttons. Therefore,
                // we handle sym button as an exception here. Its style differs from
                // that of its neighbours.
                if (button == symIndicatorButton && symState == SymIndicatorInactive) {
                    // Only show "Sym" as the button text.
                    continue;
                }

                // try secondary label
                label = button->secondaryLabel();
                if (!label.isEmpty()) {

                    if (button != symIndicatorButton) {
                        // Add formatting for this secondary label.
                        QTextLayout::FormatRange formatRange = {labelContent.length(), label.length(), secondaryFormat};
                        formatList.append(formatRange);
                    }

                    labelContent += label + " ";
                }
            }
        }
    }

    // Apply the formats
    if (!formatList.isEmpty()) {
        textLayout.setAdditionalFormats(formatList);
    }

    QFontMetrics fm(style()->font());
    QFontMetrics secondaryFm(secondaryFormat.font());
    const int labelHeight = fm.height();
    const int secondaryLabelHeight = secondaryFm.height();
    const int topMargin = style()->labelMarginTop();
    const int labelLeftWithSecondary = style()->labelMarginLeftWithSecondary();
    const int secondarySeparation = style()->secondaryLabelSeparation();
    const bool landscape = (MPlainWindow::instance()->orientation() == M::Landscape);

    textLayout.setFont(style()->font());
    textLayout.setText(labelContent);

    textLayout.beginLayout();

    for (RowIterator row(rowList.begin()); row != rowList.end(); ++row) {
        foreach (SingleWidgetButton *button, row->buttons) {

            const QString &label(button->label());
            const QString &secondary(button->secondaryLabel());
            QPoint labelPos;
            QPoint secondaryLabelPos;

            // We must not create a new QTextLine if there is no label.
            if (label.isEmpty()) {
                continue;
            }

            // This is for sym, where we might skip creation of secondary label
            // even if we have it.
            bool skipSecondary = false;

            const QRectF &buttonRect = button->cachedButtonRect;

            if (secondary.isEmpty()) {
                // All horizontally centered, portrait vs. landscape only differs in top & bottom margins.
                labelPos = QPoint(buttonRect.center().x() - fm.width(label) / 2,
                                  buttonRect.top() + topMargin);
            } else if (button == symIndicatorButton) {

                // Handle sym
                if (symState == SymIndicatorInactive) {
                    skipSecondary = true;
                    labelPos.setY(buttonRect.top() + topMargin);
                } else {
                    // No top and bottom margins.
                    labelPos.setY(buttonRect.top());
                    secondaryLabelPos = QPoint(buttonRect.center().x() - fm.width(secondary) / 2,
                                               buttonRect.bottom() - labelHeight);
                }

                labelPos.setX(buttonRect.center().x() - fm.width(label) / 2);

            } else {
                // Calculate position for both primary and secondary label.
                // We only have secondary labels in phone number layouts (with sym being exception)
                // so this follows their styling.

                // In landscape the secondary labels are below the primary ones. In portrait,
                // secondary labels are horizontally next to primary labels.

                if (landscape) {
                    // primary label: horizontally centered, top margin defines y
                    // secondary: horizontally centered, primary bottom + separation margin defines y
                    const int primaryY = buttonRect.top() + topMargin;
                    labelPos.setX(buttonRect.center().x() - fm.width(label) / 2);
                    labelPos.setY(primaryY);
                    secondaryLabelPos.setX(buttonRect.center().x() - secondaryFm.width(secondary) / 2);
                    secondaryLabelPos.setY(primaryY + labelHeight + secondarySeparation);
                } else {
                    // primary label: horizontally according to left margin, vertically centered
                    // secondary: horizontally on right of primary + separation margin, vertically centered
                    const int primaryX = buttonRect.left() + labelLeftWithSecondary;
                    labelPos.setX(primaryX);
                    labelPos.setY(buttonRect.center().y() - labelHeight / 2);
                    secondaryLabelPos.setX(primaryX + fm.width(label) + secondarySeparation);
                    secondaryLabelPos.setY(buttonRect.center().y() - secondaryLabelHeight / 2);
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
            if (!secondary.isEmpty() && !skipSecondary) {
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

void SingleWidgetButtonArea::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // Draw images first.
    foreach (const ButtonRow &row, rowList) {
        foreach (const SingleWidgetButton *button, row.buttons) {

            // Note: we should always get scalable images directly from style container.
            // Caching pointers of the images is very danger, because images could be
            // deleted by mtheme daemon in some cases (e.g. display language is changed).

            // Draw button background.
            drawKeyBackground(painter, button);

            // Draw icon.
            button->drawIcon(button->cachedButtonRect.toRect(), painter);
        }
    }

    if (textDirty) {
        buildTextLayout();
    }
    // Draw text next.
    painter->setPen(style()->fontColor());
    textLayout.draw(painter, QPoint());
}

void SingleWidgetButtonArea::drawKeyBackground(QPainter *painter,
                                               const SingleWidgetButton* button)
{
    if (!button) {
        return;
    }

    const MScalableImage *background = 0;

    switch (button->state()) {

    case IKeyButton::Normal:
        switch (button->key().style()) {
        case VKBDataKey::SpecialStyle:
            background = style()->keyBackgroundSpecial();
            break;
        case VKBDataKey::DeadkeyStyle:
            background = style()->keyBackgroundDeadkey();
            break;
        case VKBDataKey::NormalStyle:
        default:
            background = style()->keyBackground();
            break;
        }
        break;

    case IKeyButton::Pressed:
        switch (button->key().style()) {
        case VKBDataKey::SpecialStyle:
            background = style()->keyBackgroundSpecialPressed();
            break;
        case VKBDataKey::DeadkeyStyle:
            background = style()->keyBackgroundDeadkeyPressed();
            break;
        case VKBDataKey::NormalStyle:
        default:
            background = style()->keyBackgroundPressed();
            break;
        }
        break;

    case IKeyButton::Selected:
        switch (button->key().style()) {
        case VKBDataKey::SpecialStyle:
            background = style()->keyBackgroundSpecialSelected();
            break;
        case VKBDataKey::DeadkeyStyle:
            background = style()->keyBackgroundDeadkeySelected();
            break;
        case VKBDataKey::NormalStyle:
        default:
            background = style()->keyBackgroundSelected();
            break;
        }
        break;

    default:
        break;
    }

    if (background) {
        background->draw(button->cachedButtonRect.toRect(), painter);
    }
}

IKeyButton *SingleWidgetButtonArea::keyAt(const QPoint &pos) const
{
    const int numRows = rowList.count();
    const int halfVSpacing = style()->spacingVertical() / 2;
    const int translatedY = pos.y() + halfVSpacing;

    if (numRows == 0 || translatedY < 0) {
        return 0;
    }

    const int rowIndex = (numRows > 1) ? (translatedY / rowHeight) : 0;

    if (rowIndex >= numRows) {
        return 0;
    }

    return binaryButtonFind(pos.x(), rowList[rowIndex].buttons);
}

void SingleWidgetButtonArea::setShiftState(ModifierState newShiftState)
{
    if (shiftButton) {
        shiftButton->setModifiers(newShiftState != ModifierClearState);
        shiftButton->setSelected(newShiftState == ModifierLockedState);
    }
}

void SingleWidgetButtonArea::modifiersChanged(const bool shift, const QChar accent)
{
    for (RowIterator row(rowList.begin()); row != rowList.end(); ++row) {
        foreach (SingleWidgetButton *button, row->buttons) {
            // Shift button and selected keys are detached from the normal level changing.
            if (button != this->shiftButton
                && button->state() != IKeyButton::Selected) {
                button->setModifiers(shift, accent);
            }
        }
    }

    textDirty = true;
}

void SingleWidgetButtonArea::updateButtonGeometries(const int newAvailableWidth, const int)
{
    if (sectionModel()->maxColumns() == 0) {
        return;
    }

    const qreal HorizontalSpacing = style()->spacingHorizontal();
    const qreal VerticalSpacing = style()->spacingVertical();

    // TODO: kill alignment:
    const Qt::Alignment alignment = sectionModel()->horizontalAlignment();

    // The following code cannot handle negative width:
    int availableWidth = qMax(0, newAvailableWidth);
    const qreal normalizedWidth = qMax<qreal>(1.0, sectionModel()->maxNormalizedWidth());
    const qreal availableWidthForButtons = availableWidth - ((normalizedWidth - 1) * HorizontalSpacing);
    const qreal equalButtonWidth = availableWidthForButtons / normalizedWidth;

    // This is used to update the button rectangles
    qreal y = 0;

    // Button margins
    const qreal leftMargin = HorizontalSpacing / 2;
    const qreal rightMargin = HorizontalSpacing - leftMargin;
    const qreal topMargin = VerticalSpacing / 2;
    const qreal bottomMargin = VerticalSpacing - topMargin;

    QRectF br; // button bounding rectangle
    br.setHeight(rowHeight); // Constant height

    for (RowIterator row(rowList.begin()); row != rowList.end(); ++row) {
        // Update row width
        qreal rowWidth = 0;
        foreach (SingleWidgetButton *button, row->buttons) {
            button->width = button->preferredWidth(equalButtonWidth, HorizontalSpacing);
            rowWidth += button->width + HorizontalSpacing;
        }
        rowWidth -= HorizontalSpacing;

        if (availableWidth < rowWidth) {
            // TODO: Find out the root cause, and fix it!
            qWarning() << __PRETTY_FUNCTION__
                       << "Using more width (" << rowWidth
                       << ") than available (" << availableWidth
                       << ")!";
        }

        qreal availableWidthForSpacers = 0;
        const QList<int> spacerIndices = sectionModel()->spacerIndices(row - rowList.begin());
        int spacerCount = row->stretchButton ? spacerIndices.count() + 1
                                             : spacerIndices.count();

        if (row->stretchButton) {
            rowWidth -=  row->stretchButton->width;

            // Handle the case of one stretch button/no other spacer elments directly:
            if (spacerCount == 1) {
                row->stretchButton->width = availableWidth - rowWidth;
                rowWidth = availableWidth;
                spacerCount = 0;
            }
        }

        if ((spacerCount > 0) && (availableWidth > rowWidth)) {
            availableWidthForSpacers = (availableWidth - rowWidth) / spacerCount;

            if (row->stretchButton) {
                row->stretchButton->width = availableWidthForSpacers;
            }
        }

        // Save row width for easier use.
        row->cachedWidth = rowWidth + HorizontalSpacing;
        qreal x = spacerIndices.count(-1) * availableWidthForSpacers;
        row->offset = static_cast<int>(x);

        // We can precalculate button rectangles.
        br.moveTop(y - topMargin);

        for (int buttonIndex = 0; buttonIndex < row->buttons.count(); ++buttonIndex) {
            SingleWidgetButton *button = row->buttons.at(buttonIndex);

            br.moveLeft(x);
            br.setWidth(button->width);

            // save it
            button->cachedBoundingRect = br;
            button->cachedButtonRect = br.adjusted(leftMargin, topMargin, -rightMargin, -bottomMargin);

            // Increase x to the next button bounding rect border.
            x += button->width + HorizontalSpacing;

            // Our spacerIndex is a multi-set, hence we need to add
            // availableWidthForSpacers for every ocurrence of spacerIndex:
            x += spacerIndices.count(buttonIndex) * availableWidthForSpacers;
        }

        y += rowHeight;
    }

    // Positions may have changed, rebuild text layout.
    textDirty = true;
}

QRectF SingleWidgetButtonArea::boundingRect() const
{
    // Extend the bounding rectangle to all directions by the amount of spacing.
    return QRectF(-QPoint(style()->spacingHorizontal() / 2, style()->spacingVertical() / 2),
                  size() + QSizeF(style()->spacingHorizontal(), style()->spacingVertical()));
}


ISymIndicator *SingleWidgetButtonArea::symIndicator()
{
    return this;
}

// ISymIndicator implementation
void SingleWidgetButtonArea::activateSymIndicator()
{
    if (symState == SymIndicatorInactive) {
        // We have changed the text. Sym has two-line text in active mode.
        textDirty = true;
    }
    symState = SymActive;
    update();
}

// ISymIndicator implementation
void SingleWidgetButtonArea::activateAceIndicator()
{
    if (symState == SymIndicatorInactive) {
        // We have changed the text. Sym has two-line text in active mode.
        textDirty = true;
    }
    symState = AceActive;
    update();
}

// ISymIndicator implementation
void SingleWidgetButtonArea::deactivateIndicator()
{
    if (symState != SymIndicatorInactive) {
        update();
        // We have changed the text. Sym has one-line text in inactive mode.
        textDirty = true;
        symState = SymIndicatorInactive;
    }
}

void SingleWidgetButtonArea::onThemeChangeCompleted()
{
    KeyButtonArea::onThemeChangeCompleted();
    buildTextLayout();
}
