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



#include "mvirtualkeyboardstyle.h"
#include "singlewidgetbuttonarea.h"

#include "singlewidgetbutton.h"
#include "limitedtimer.h"

#include <QDebug>
#include <QEvent>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QTextLine>
#include <MApplication>
#include <MComponentData>
#include <MFeedbackPlayer>
#include <duireactionmap.h>
#include <MTheme>

SingleWidgetButtonArea::SingleWidgetButtonArea(MVirtualKeyboardStyleContainer *style,
                                               QSharedPointer<const LayoutSection> sectionModel,
                                               ButtonSizeScheme buttonSizeScheme,
                                               bool usePopup,
                                               QGraphicsWidget *parent)
    : KeyButtonArea(style, sectionModel, buttonSizeScheme, usePopup, parent),
      rowHeight(0),
      rowList(sectionModel->rowCount()),
      symState(SymIndicatorInactive),
      symIndicatorButton(0),
      shiftCapsLock(false),
      pixmap1(0),
      pixmap2(0),
      pixmap3(0),
      textDirty(false),
      equalWidthButtons(false)
{
    textLayout.setCacheEnabled(true);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Initially deactivate sym page indicator.
    deactivateIndicator();

    loadKeys();
}

void SingleWidgetButtonArea::fetchOptimumSizeButtonBackgrounds(QSize size)
{
    // if one is valid, they all are
    if (pixmap1) {
        // First check if we already have correct size
        // or bail out also if we use different size buttons
        // (no need to remake them).
        if ((size.width() < 0) || (pixmap1->size() == size)) {
            return;
        }

        MTheme::releasePixmap(pixmap3);
        MTheme::releasePixmap(pixmap2);
        MTheme::releasePixmap(pixmap1);
    }

    const QSize defaultSize = style()->keyNormalSize();

    if (size.width() < 0) {
        // No specific size set for all buttons. Use default and we then scale it.
        size = defaultSize;
    }

    // Resize the background images for MScalableImage so that we use the size of the most
    // used button. This way MScalableImage will use painter->drawPixmap() directly. There
    // is some overhead when drawing the image in 9 rectangles.
    pixmap1 = MTheme::pixmap(style()->keyBackgroundId(), size);

    // Let's use one default size for all pressed & selected backgrounds accross
    // SingleWidgetButtonAreas since there is never many of these backgrounds
    // drawn at the same time.
    pixmap2 = MTheme::pixmap(style()->keyBackgroundPressedId(), defaultSize);
    pixmap3 = MTheme::pixmap(style()->keyBackgroundSelectedId(), defaultSize);

    keyBackgrounds[0].setPixmap(pixmap1); // normal
    keyBackgrounds[1].setPixmap(pixmap2); // pressed
    keyBackgrounds[2].setPixmap(pixmap3); // selected

    // Border size 10 is suitable for all sane size buttons we're using.
    const int border = 10;
    keyBackgrounds[0].setBorders(border, border, border, border);
    keyBackgrounds[1].setBorders(border, border, border, border);
    keyBackgrounds[2].setBorders(border, border, border, border);
}

SingleWidgetButtonArea::~SingleWidgetButtonArea()
{
    // Release any key that might be pressed before destroying them.
    setActiveKey(0);

    for (RowIterator rowIter(rowList.begin()); rowIter != rowList.end(); ++rowIter) {
        qDeleteAll(rowIter->buttons);
        rowIter->buttons.clear();
    }

    MTheme::releasePixmap(pixmap3);
    MTheme::releasePixmap(pixmap2);
    MTheme::releasePixmap(pixmap1);
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

void SingleWidgetButtonArea::drawReactiveAreas(DuiReactionMap *reactionMap, QGraphicsView *view)
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
            const VKBDataKey *dataKey;
            QSize buttonSize;
            bool stretchesHorizontally;
            buttonInformation(row, col, dataKey, buttonSize, stretchesHorizontally);

            SingleWidgetButton *button = new SingleWidgetButton(*dataKey, style(), *this);

            if (dataKey->binding()->action() == KeyBinding::ActionSym) {
                // Save pointer for easier use.
                this->symIndicatorButton = button;
            } else if (dataKey->binding()->action() == KeyBinding::ActionShift) {
                this->shiftButton = button;
            }

            button->width = buttonSize.width();

            const int vSpacing = style()->spacingVertical();
            rowHeight = qMax(rowHeight - vSpacing, buttonSize.height()) + vSpacing;

            // Only one stretching item per row.
            if (stretchesHorizontally && !rowIter->stretchButton) {
                rowIter->stretchButton = button;
            }

            rowIter->buttons.append(button);
        }
    } // end foreach row

    // Update height
    updateGeometry();
}

void SingleWidgetButtonArea::buildTextLayout()
{
    textDirty = false;

    textLayout.clearLayout();

    // QTextLayout requires text content to be set before creating QTextLines.
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
                    labelContent += label + " ";
                }
            }
        }
    }

    QFontMetrics fm(style()->font());
    const int labelHeight = fm.height();

    // Margins for text bounding box relative to buttonRect. Primary label will be set
    // based on the top margin only so font size will determine the height. Bottom margin
    // is used to set position for the secondary label (no support yet for horizontally
    // positioned secondary labels).
    const int topMargin = style()->labelMarginTop();
    const int bottomMargin = style()->labelMarginBottom();

    textLayout.setFont(style()->font());
    textLayout.setText(labelContent);

    textLayout.beginLayout();

    for (RowIterator row(rowList.begin()); row != rowList.end(); ++row) {
        foreach (SingleWidgetButton *button, row->buttons) {

            const QString &label(button->label());

            // We must not create a new QTextLine if there is no label.
            if (label.isEmpty()) {
                continue;
            }

            const QRect &buttonRect = button->cachedButtonRect;
            const int xmiddle = buttonRect.center().x();

            // this is the top of the primary label's bounding box
            int top = buttonRect.top() + topMargin;

            // this is the top of the secondary label's bounding box
            int topSecondary = buttonRect.bottom() - bottomMargin - labelHeight;

            // Handle sym
            bool skipSecondary = false;
            if (button == symIndicatorButton) {
                if (symState == SymIndicatorInactive) {
                    skipSecondary = true;
                } else {
                    // No top and bottom margins.
                    top = buttonRect.top();
                    topSecondary = buttonRect.bottom() - labelHeight;
                }
            }

            // Create the primary label
            QTextLine line = textLayout.createLine();
            if (!line.isValid()) {
                // We are getting out of sync anyway so no point in continuing.
                goto endLayout;
            }
            line.setNumColumns(1); // at least one character, will be seeked forward until next whitespace
            line.setPosition(QPoint(xmiddle - fm.width(label) / 2, top));

            const QString &secondary(button->secondaryLabel());

            // Same for secondary label
            if (!secondary.isEmpty() && !skipSecondary) {
                line = textLayout.createLine();
                if (!line.isValid()) {
                    goto endLayout;
                }
                line.setNumColumns(1);
                line.setPosition(QPoint(xmiddle - fm.width(secondary) / 2, topSecondary));
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

            const MScalableImage *background = 0;
            const int backgroundIndex = qBound(0, static_cast<int>(button->state()), 3);

            // Draw button background.
            if (symState != SymIndicatorInactive
                && button->binding().action() == KeyBinding::ActionSym) {
                background = symIndicatorBackgrounds[backgroundIndex];
            } else {
                background = &keyBackgrounds[backgroundIndex];
            }

            if (background) {
                background->draw(button->cachedButtonRect, painter);
            }

            // Draw icon.
            button->drawIcon(button->cachedButtonRect, painter);
        }
    }

    if (textDirty) {
        buildTextLayout();
    }
    // Draw text next.
    textLayout.draw(painter, QPoint());
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

    const ButtonRow &row(rowList[rowIndex]);

    if (pos.x() < row.offset || row.buttons.isEmpty()) {
        return 0;
    }

    if (equalWidthButtons) {
        const int column = (pos.x() - row.offset) / (row.buttons.first()->width + style()->spacingHorizontal());
        if (column < row.buttons.count()) {
            return row.buttons.at(column);
        }
    } else {
        // Buttons not equally spaced, we have to walk through them.
        int x = row.offset;
        QList<SingleWidgetButton*>::const_iterator buttonIter(row.buttons.begin());
        for (; buttonIter != row.buttons.end(); ++buttonIter) {

            x += (*buttonIter)->width + style()->spacingHorizontal();
            if (pos.x() < x) {
                return (*buttonIter);
            }
        }
    }

    return 0;
}

void SingleWidgetButtonArea::modifiersChanged(const bool shift, const QChar accent)
{
    for (RowIterator row(rowList.begin()); row != rowList.end(); ++row) {
        foreach (SingleWidgetButton *button, row->buttons) {
            button->setModifiers(shift, accent);
        }
    }

    textDirty = true;
}

void SingleWidgetButtonArea::updateButtonGeometries(const int availableWidth, const int equalButtonWidth)
{
    if (sectionModel()->maxColumns() == 0) {
        return;
    }

    this->equalWidthButtons = (equalButtonWidth >= 0);

    fetchOptimumSizeButtonBackgrounds(QSize(equalButtonWidth, rowHeight - style()->spacingVertical()));

    const int HorizontalSpacing = style()->spacingHorizontal();
    const int VerticalSpacing = style()->spacingVertical();
    const Qt::Alignment alignment = sectionModel()->horizontalAlignment();

    // This is used to update the button rectangles
    int y = 0;

    // Button margins
    const int leftMargin = HorizontalSpacing / 2;
    const int rightMargin = HorizontalSpacing - leftMargin;
    const int topMargin = VerticalSpacing / 2;
    const int bottomMargin = VerticalSpacing - topMargin;

    QRect br; // button bounding rectangle
    br.setHeight(rowHeight); // Constant height

    for (RowIterator row(rowList.begin()); row != rowList.end(); ++row) {

        // Update row width
        int rowWidth = 0;
        foreach (SingleWidgetButton *button, row->buttons) {

            // Restrict button width.
            if (equalWidthButtons) {
                button->width = equalButtonWidth;
            }

            rowWidth += button->width + HorizontalSpacing;
        }
        rowWidth -= HorizontalSpacing;

        // If row has a stretching button all width is used.
        if (row->stretchButton) {
            rowWidth -= row->stretchButton->width; // this was already added, we recalculate it. leave spacings though
            row->stretchButton->width = qMax(0, availableWidth - rowWidth);
            rowWidth = availableWidth;
        }

        // Save row width for easier use.
        row->cachedWidth = rowWidth + HorizontalSpacing;

        // Width is calculated, we can now set row offset according to alignment.
        if (alignment & Qt::AlignRight) {
            row->offset = (availableWidth - rowWidth);
        } else if (alignment & Qt::AlignCenter) {
            row->offset = (availableWidth - rowWidth) / 2;
        } else {
            row->offset = 0;
        }
        row->offset -= HorizontalSpacing / 2;

        // Row offset is ready. We can precalculate button rectangles.
        br.moveTop(y - topMargin);
        int x = row->offset;
        foreach (SingleWidgetButton *button, row->buttons) {
            br.moveLeft(x);
            br.setWidth(button->width + HorizontalSpacing);

            // save it
            button->cachedBoundingRect = br;
            button->cachedButtonRect = br.adjusted(leftMargin, topMargin, -rightMargin, -bottomMargin);

            // Increase x to the next button bounding rect border.
            x += button->width + HorizontalSpacing;
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
    // Use same image for all button states for now. Some different graphics may
    // be introduced later for the intermediate step between sym and ace modes.
    // while holding button down.
	const MScalableImage *image = style()->keyBackgroundSymIndicatorSym();
    symIndicatorBackgrounds[0] = image;
    symIndicatorBackgrounds[1] = image;
    symIndicatorBackgrounds[2] = image;
    update();

    if (symState == SymIndicatorInactive) {
        // We have changed the text. Sym has two-line text in active mode.
        textDirty = true;
    }
    symState = AceActive;
}

// ISymIndicator implementation
void SingleWidgetButtonArea::activateAceIndicator()
{
    const MScalableImage *image = style()->keyBackgroundSymIndicatorAce();
    symIndicatorBackgrounds[0] = image;
    symIndicatorBackgrounds[1] = image;
    symIndicatorBackgrounds[2] = image;
    update();

    if (symState == SymIndicatorInactive) {
        // We have changed the text. Sym has two-line text in active mode.
        textDirty = true;
    }
    symState = SymActive;
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

