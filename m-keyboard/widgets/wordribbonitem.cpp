#include "wordribbonitem.h"

#include "mscalableimage.h"
#include <QString>
#include <QtCore>
#include <QtGui>

namespace {
    static const int FocusZoneMargin = 30;
}

WordRibbonItem::WordRibbonItem(WordRibbon::ItemStyleMode mode, MWidget* parent):
        MStylableWidget(parent),
        label(""),
        mPositionIndex(-1),
        isMousePressCancelled(false),
        highlightEffectEnabled(true),
        forceMaxWidth(-1),
        state(NormalState),
        mode(mode)
{
    if (textPen.color() != style()->fontColor()) {
        textPen.setColor(style()->fontColor());
    }
 
    recalculateItemSize();
}


WordRibbonItem::~WordRibbonItem()
{
}

void WordRibbonItem::drawContents(QPainter *painter, const QStyleOptionGraphicsItem *option) const
{
    if (label.length() == 0) {
        //No content inside candidate item.
        MStylableWidget::drawContents(painter, option);
        return ;
    }


    painter->setFont(drawFont);
    painter->setPen(textPen);
    painter->drawText(contentRect, Qt::AlignCenter, label);
}

QSizeF WordRibbonItem::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    Q_UNUSED(constraint);

    QSizeF size ;


    switch (which) {
    case Qt::MinimumSize:
        size = minimumSize;
        break;
    case Qt::PreferredSize:
        size = preferredSize;
        break;
    case Qt::MaximumSize:
        if (forceMaxWidth > 0) {
            size = preferredSize;
            size.setWidth(forceMaxWidth);
        }
        else {
            size = style()->maximumSize();
        }
        break;
    default:
        break;
    }
    return size;
}

void WordRibbonItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (!paddingRect.contains(event->pos().toPoint())) {
        isMousePressCancelled = true;
        return ;
    }

    isMousePressCancelled = false;
    if (highlightEffectEnabled) {
        updateStyleState(PressState);
    }

    emit mousePressed();
}

void WordRibbonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    Q_UNUSED(event);
    if (isMousePressCancelled)
        return ;
    else {
        highlight();
        emit mouseReleased();
    }
}

void WordRibbonItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if (isMousePressCancelled)
        return ;

    QRect moveRect = paddingRect.
                     adjusted(-FocusZoneMargin, -FocusZoneMargin, +FocusZoneMargin, +FocusZoneMargin);
    if (!moveRect.contains(event->pos().toPoint())) {
        isMousePressCancelled = true;
        highlight();
    }
}

void WordRibbonItem::cancelEvent(MCancelEvent *event)
{
    Q_UNUSED(event);
    highlight();
}


void WordRibbonItem::setText(const QString& str)
{
    label = str;
    applyStyle();
    recalculateItemSize();
    update();
    updateGeometry();
}

void WordRibbonItem::clearText()
{
    setText ("");
}

QString WordRibbonItem::text()
{
    return label;
}

void WordRibbonItem::enableHighlight()
{
    highlightEffectEnabled = true; 
}

void WordRibbonItem::disableHighlight()
{
    highlightEffectEnabled = false; 
}

void WordRibbonItem::highlight()
{
    if (highlightEffectEnabled == false
        || highlighted() == true) {
        return ;
    }

    updateStyleState(SelectedState);
}

void WordRibbonItem::clearHighlight()
{
    if (highlightEffectEnabled == false
        || highlighted() == false) {
        return ;
    }

    updateStyleState(NormalState);
}

bool WordRibbonItem::highlighted() const
{
    if (!highlightEffectEnabled) {
        return false;
    }
    return state == SelectedState;
}

void WordRibbonItem::setPositionIndex(int index)
{
    mPositionIndex = index;
}

int WordRibbonItem::positionIndex() const
{
    return mPositionIndex;
}


void WordRibbonItem::applyStyle()
{
    if (mode == WordRibbon::DialogStyleMode) {
        // In WordRibbon::DialogStyleMode the margins and paddings are depending
        // on the length of the label
        switch (label.length()) {
        case 3:
        default:
           style().setModeMorecharacter();
           break;

        case 2:
           style().setModeTwocharacter();
           break;

        case 1:
           style().setModeOnecharacter();
           break;
        }
    }
}

void WordRibbonItem::recalculateItemSize()
{
    int paddingLeft, paddingRight, paddingTop, paddingBottom;
    int marginLeft, marginRight, marginTop, marginBottom;

    paddingLeft = style()->paddingLeft();
    paddingRight = style()->paddingRight();
    paddingTop = style()->paddingTop();
    paddingBottom = style()->paddingBottom();
    marginLeft = style()->marginLeft();
    marginRight = style()->marginRight();
    marginTop = style()->marginTop();
    marginBottom = style()->marginBottom();

    minimumSize = style()->minimumSize();
    drawFont = style()->font();

    if (label.length() == 0) {
        preferredSize = minimumSize;
    } else {
        // If label has characters, then the size of this widget
        // need to be update by calculating padding and margin.
        QFontMetrics fm(drawFont);
        QSize textSize = fm.size(Qt::TextSingleLine, label);

        if (mode == WordRibbon::DialogStyleMode) {
            // In WordRibbon::DialogStyleMode, reduce font size for long label
            // to fit the item width
            int maxWidth = forceMaxWidth > 0 ?
                           forceMaxWidth :
                           style()->maximumSize().width();

            int currentWidth = textSize.width() + 
                             marginLeft + marginRight +
                             paddingLeft + paddingRight;

            while (currentWidth > maxWidth) {
                if (drawFont.pixelSize() <= 5) {
                    // too small
                    break;
                }
                drawFont.setPixelSize(drawFont.pixelSize() - 1);
                textSize = fm.size(Qt::TextSingleLine, label);

                currentWidth = textSize.width() + 
                               marginLeft + marginRight +
                               paddingLeft + paddingRight;
            }
        }

        preferredSize.setWidth(textSize.width() +
                               marginLeft + marginRight +
                               paddingLeft + paddingRight);

        preferredSize.setHeight(textSize.height() +
                                marginTop + marginBottom +
                                paddingTop + paddingBottom);

        minimumSize = preferredSize;
    }

    setMinimumSize(minimumSize);
    setPreferredSize(preferredSize);

    QSizeF tmpSize = preferredSize - QSizeF(marginLeft + marginRight,
                                                         marginTop + marginBottom);

    paddingRect = QRect(marginLeft, marginTop,
                        tmpSize.width(), tmpSize.height());

    tmpSize = tmpSize - QSizeF(paddingLeft + paddingRight,
                               paddingTop + paddingBottom);

    contentRect = QRect(marginLeft + paddingLeft,
                        marginTop + paddingTop,
                        tmpSize.width(),
                        tmpSize.height());

    setContentsMargins(marginLeft + paddingLeft,
                       marginTop + paddingTop,
                       marginRight + marginRight,
                       marginBottom + paddingBottom);

    resize(preferredSize);
}

void WordRibbonItem::updateStyleState(ItemState newState)
{
    if (mode == WordRibbon::DialogStyleMode)
        return;

    state = newState; 

    switch(state) {
    case NormalState:
        style().setModeDefault();
        break;
    case SelectedState:
        style().setModeSelected();
        break;
    case PressState:
        style().setModePressed();
        break;
    default:
        break;
    }

    update();
}

void WordRibbonItem::setMaxWidth(int width)
{
    if (width < style()->minimumSize().width())
        return;

    forceMaxWidth = width;
}

