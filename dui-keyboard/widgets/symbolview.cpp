/* * This file is part of dui-keyboard *
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



#include "duivirtualkeyboardstyle.h"
#include "horizontalswitcher.h"
#include "layoutsmanager.h"
#include "symbolview.h"

#include <DuiSceneManager>
#include <DuiScalableImage>
#include <duireactionmap.h>
#include <duitheme.h>
#include <duiplainwindow.h>

#include <QCoreApplication>
#include <QDebug>
#include <QGraphicsItemAnimation>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneResizeEvent>
#include <QPainter>
#include <QTimeLine>

namespace
{
    const int DefaultAnimationDuration = 100;
    const QString SymLabel("Sym");
    const QString AceLabel(QString(0xE1) + QChar(0xE7) + QChar(0xE8)); // "áçè"
    const QString SymbolSectionPrefix = "symbols ";
    const QString SymbolSectionSym = SymbolSectionPrefix + "Sym";
    const int AnimationFrameCount = 20;

    const QString ObjectNameTabs("VirtualKeyboardSymTabs");
    const QString ObjectNameTabButton("VirtualKeyboardSymTabsButton");
    const QString ObjectNameCloseButton("VirtualKeyboardCloseButton");

    const QString SymCloseIcon("icon-m-input-methods-close");
};



SymbolView::SymbolView(const LayoutsManager &layoutsManager, DuiVirtualKeyboardStyleContainer *style,
                       const QString &language, QGraphicsWidget *parent)
    : DuiWidget(parent),
      styleContainer(style),
      sceneManager(*DuiPlainWindow::instance()->sceneManager()),
      selectedLayout(0),
      activity(Inactive),
      activePage(0),
      shift(0),
      layoutsMgr(layoutsManager),
      pageSwitcher(0),
      functionRow(0),
      currentOrientation(sceneManager.orientation()),
      currentLanguage(language),
      mouseDownKeyArea(false),
      verticalLayout(*new QGraphicsLinearLayout(Qt::Vertical, this)),
      keyAreaLayout(*new QGraphicsLinearLayout(Qt::Vertical))
{
    hide();
    setupShowAndHide();
    setupLayout();
    reloadContent();
}


SymbolView::~SymbolView()
{
    delete showTimeLine;
    showTimeLine = 0;
    delete hideTimeLine;
    hideTimeLine = 0;
    delete showAnimation;
    showAnimation = 0;
    delete hideAnimation;
    hideAnimation = 0;
}

void SymbolView::setupLayout()
{
    // Layout widths are set in organizeContent().

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);

    verticalLayout.setSpacing(0);
    verticalLayout.setContentsMargins(0, 0, 0, 0);

    keyAreaLayout.setSpacing(style()->spacingVertical());
    keyAreaLayout.setContentsMargins(style()->paddingLeft(), style()->paddingTop(),
                                     style()->paddingRight(), style()->paddingBottom());

    verticalLayout.addItem(&keyAreaLayout);
}

void SymbolView::reloadContent()
{
    // Get layout model which for current language and orientation.
    const LayoutData *layoutData = layoutsMgr.layout(currentLanguage, LayoutData::General, currentOrientation);
    Q_ASSERT(layoutData);

    loadSwitcherPages(*layoutData, activePage);
    loadFunctionRow(*layoutData);

    updateSymIndicator();
}

void SymbolView::setupShowAndHide()
{
    showTimeLine = new QTimeLine(DefaultAnimationDuration);
    hideTimeLine = new QTimeLine(DefaultAnimationDuration);

    showTimeLine->setFrameRange(0, AnimationFrameCount);
    hideTimeLine->setFrameRange(0, AnimationFrameCount);

    connect(hideTimeLine, SIGNAL(finished()),
            this, SLOT(onHidden()));

    connect(showTimeLine, SIGNAL(finished()),
            this, SLOT(onReady()));

    showTimeLine->setCurveShape(QTimeLine::EaseOutCurve);
    hideTimeLine->setCurveShape(QTimeLine::EaseOutCurve);
    showAnimation = new QGraphicsItemAnimation();
    hideAnimation = new QGraphicsItemAnimation();

    showAnimation->setItem(this);
    showAnimation->setTimeLine(showTimeLine);

    hideAnimation->setItem(this);
    hideAnimation->setTimeLine(hideTimeLine);
}

void SymbolView::updateAnimPos(int top, int bottom)
{
    showAnimation->clear();
    showAnimation->setPosAt(0.0, QPoint(0, bottom));
    showAnimation->setPosAt(1.0, QPoint(0, top));

    hideAnimation->clear();
    hideAnimation->setPosAt(0.0, QPoint(0, top));
    hideAnimation->setPosAt(1.0, QPoint(0, bottom));
}


void
SymbolView::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const DuiScalableImage *background = style()->backgroundImage();

    if (background) {
        // Background covers everything except top layout.
        background->draw(keyAreaLayout.geometry().toRect(), painter);
    }
}

QVariant SymbolView::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSceneHasChanged) {
        QGraphicsScene *newScene = value.value<QGraphicsScene *>();
        if (newScene != 0 && pageSwitcher) {
            // Usually this happens only once when symbolView is first put into a scene.

            // Reinstall scene event filter.
            for (int i = 0; i < pageSwitcher->count(); ++i) {
                pageSwitcher->widget(i)->removeSceneEventFilter(this);
                pageSwitcher->widget(i)->installSceneEventFilter(this);
            }
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void SymbolView::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    // Nothing, just stop the event from propagating
}


bool SymbolView::sceneEventFilter(QGraphicsItem */*watched*/, QEvent *event)
{
    bool stopPropagation = false;

    if (pageSwitcher) {
        switch (event->type()) {
        case QEvent::GraphicsSceneMousePress: {
            mouseDownKeyArea = true;
        } break;
        case QEvent::GraphicsSceneMouseRelease: {
            // Even though at this point ungrabbing the child widget would allow release
            // event to reach it the order is wrong. We now cope with the situation because
            // we know KeyButtonAreas will ungrab even an implicit grab on release event.
            mouseDownKeyArea = false;

            // Hide SymbolView if temporary mode was on.
            if (activity == TemporarilyActive) {
                hideSymbolView();
            }
        } break;
        default:
            break;
        }
    }
    return stopPropagation;
}

void SymbolView::reposition(const int height)
{
    const int bottom = sceneManager.visibleSceneSize().height();
    const int top = bottom - height;

    setPos(0, isVisible() ? top : bottom);

    updateAnimPos(top, bottom);
}

void SymbolView::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    DuiWidget::resizeEvent(event);

    if (!qFuzzyCompare(event->oldSize().height(), event->newSize().height())) {
        reposition(event->newSize().height());
        if (isVisible()) {
            emit regionUpdated(interactiveRegion());
        }
    }
}

void SymbolView::prepareToOrientationChange()
{
    qDebug() << __PRETTY_FUNCTION__;

    // if inactive, just ignore
    if (!isActive()) {
        return;
    }
    hideSymbolView(TemporaryHideMode);
}


void SymbolView::finalizeOrientationChange()
{
    organizeContent();
    if (activity == TemporarilyInactive) {
        //reshow the page
        showSymbolView(NormalShowMode);
    }
}

void
SymbolView::showSymbolView(SymbolView::ShowMode mode)
{
    organizeContent();

    if (isActive())
        return;
    if (mode == FollowMouseShowMode) {
        activity = TemporarilyActive;
    } else {
        activity = Active;
    }

    show();

    if (showTimeLine->state() == QTimeLine::Running)
        return;

    showTimeLine->start();

    if (mode == FollowMouseShowMode) {
        pageSwitcher->currentWidget()->grabMouse();
        mouseDownKeyArea = true;
    }

    emit showingUp();
}


void
SymbolView::hideSymbolView(SymbolView::HideMode mode)
{
    if (isActive()) {
        if (mode == NormalHideMode) {
            changePage(0);
        }
        hideTimeLine->start();
    }
    if (mode == TemporaryHideMode) {
        activity = TemporarilyInactive;
    } else {
        activity = Inactive;
    }
}

void
SymbolView::changePage(int id)
{
    if ((id == activePage) && isActive())
        return;

    // symbolView is down, show it
    if (id == 0 && !isActive()) {
        activePage = 0;
        pageSwitcher->setCurrent(0);
        showSymbolView();
    } else {
        activePage = id;
        pageSwitcher->switchTo(activePage);
    }

    selectedLayout = qobject_cast<KeyButtonArea *>(pageSwitcher->currentWidget());
    Q_ASSERT(selectedLayout);

    updateSymIndicator();
}

void SymbolView::loadSwitcherPages(const LayoutData &kbLayout, const unsigned int selectPage)
{
    if (pageSwitcher) {
        keyAreaLayout.removeItem(pageSwitcher);
        delete pageSwitcher;
    }
    selectedLayout = 0; // invalid now so clear

    pageSwitcher = new HorizontalSwitcher(this);

    connect(pageSwitcher, SIGNAL(switchStarting(QGraphicsWidget *, QGraphicsWidget *)),
            SLOT(onSwitchStarting(QGraphicsWidget *, QGraphicsWidget *)));
    connect(pageSwitcher, SIGNAL(switchDone(QGraphicsWidget *, QGraphicsWidget *)), SLOT(switchDone()));

    QSharedPointer<const LayoutSection> symbolSection;

    // Add special Sym section always as the first, if present.
    symbolSection = kbLayout.section(SymbolSectionSym);
    if (!symbolSection.isNull()) {
        addPage(symbolSection);
    }

    // Add all others.
    for (int i = 0; i < kbLayout.numSections(); ++i) {
        symbolSection = kbLayout.section(i);

        // Skip those that are not symbol sections.
        if (symbolSection->name().startsWith(SymbolSectionPrefix) &&
                // Skip also sym section because we added it already.
                (symbolSection->name() != SymbolSectionSym)) {
            addPage(symbolSection);
        }
    }

    if (pageSwitcher->count() >= 0) {
        activePage = (selectPage >= static_cast<unsigned int>(pageSwitcher->count()) ? 0 : selectPage);
        pageSwitcher->setCurrent(activePage);
        selectedLayout = qobject_cast<KeyButtonArea *>(pageSwitcher->currentWidget());
        Q_ASSERT(selectedLayout);
    }

    pageSwitcher->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    keyAreaLayout.addItem(pageSwitcher);
}

void SymbolView::loadFunctionRow(const LayoutData &layout)
{
    if (functionRow) {
        keyAreaLayout.removeItem(functionRow);
        delete functionRow;
    }

    functionRow = createKeyButtonArea(layout.section(LayoutData::functionkeySection),
                                      KeyButtonArea::ButtonSizeFunctionRow, false);
    functionRow->setObjectName("SymbolFunctionRow");

    if (functionRow) {
        functionRow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        keyAreaLayout.addItem(functionRow);
    }
}

void SymbolView::addPage(QSharedPointer<const LayoutSection> symbolSection)
{
    KeyButtonArea *page = createKeyButtonArea(symbolSection);
    page->setObjectName("SymbolMainRow");

    if (page) {
        connect(page, SIGNAL(flickLeft()), SLOT(switchToNextPage()));
        connect(page, SIGNAL(flickRight()), SLOT(switchToPrevPage()));
        connect(page, SIGNAL(flickDown()), SLOT(hideSymbolView()));

        pageSwitcher->addWidget(page);

        // Track children's mouse events.
        // If we don't yet have a scene, the filter won't be installed. Then we do this
        // later in SymbolView::itemChange().
        if (scene()) {
            page->installSceneEventFilter(this);
        }
    }
}

KeyButtonArea *SymbolView::createKeyButtonArea(QSharedPointer<const LayoutSection> section,
                                               KeyButtonArea::ButtonSizeScheme sizeScheme,
                                               bool enablePopup)
{
    KeyButtonArea *keysWidget = NULL;

    if (!section.isNull()) {
        keysWidget = new SingleWidgetButtonArea(styleContainer, section, sizeScheme, enablePopup);
        keysWidget->setFont(style()->font());

        connect(this, SIGNAL(levelSwitched(int, bool)), keysWidget, SLOT(switchLevel(int, bool)));

        connect(keysWidget, SIGNAL(keyClicked(const KeyEvent &)),
                SIGNAL(keyClicked(const KeyEvent &)));
        connect(keysWidget, SIGNAL(keyPressed(const KeyEvent &)),
                SIGNAL(keyPressed(const KeyEvent &)));
        connect(keysWidget, SIGNAL(keyReleased(const KeyEvent &)),
                SIGNAL(keyReleased(const KeyEvent &)));
    }
    return keysWidget;
}

void SymbolView::organizeContent()
{
    const Dui::Orientation orientation(sceneManager.orientation());
    const int sceneWidth = sceneManager.visibleSceneSize().width();

    setPreferredWidth(sceneWidth);
    setMaximumWidth(sceneWidth);
    setMinimumWidth(sceneWidth);

    if (currentOrientation != orientation) {
        currentOrientation = orientation;
        reloadContent();
    }

    reposition(size().toSize().height());
}


void SymbolView::switchLevel(int level, bool capslock)
{
    shift = level;
    emit levelSwitched(shift, capslock);
    updateSymIndicator();
}

int SymbolView::currentLevel() const
{
    return shift;
}

void SymbolView::setLanguage(const QString &lang)
{
    if (lang != currentLanguage) {
        currentLanguage = lang;
        reloadContent();
    }
}


void SymbolView::switchToNextPage()
{
    if ((activePage + 1) < pageSwitcher->count())
        changePage(activePage + 1);
}



void SymbolView::switchToPrevPage()
{
    if (activePage > 0)
        changePage(activePage - 1);
}


void SymbolView::onReady()
{
    emit regionUpdated(interactiveRegion());
    emit opened();
}


void SymbolView::onHidden()
{
    hide();
    emit regionUpdated(QRegion());
    emit hidden();
}


void SymbolView::onSwitchStarting(QGraphicsWidget *current, QGraphicsWidget *next)
{
    if (mouseDownKeyArea) {
        if (current) {
            // This is required because, for some reason, Qt
            // does not allow mouse grab for item that was hidden
            // when it had the grab. Even though hiding an item
            // should release the grab. So this call does not affect
            // the following grabMouse() but it affects the behaviour
            // if we get back to this page during the same mouse move.
            current->ungrabMouse();
        }
        next->grabMouse();
    }
    selectedLayout = qobject_cast<KeyButtonArea *>(next);
    Q_ASSERT(selectedLayout);
}

void SymbolView::switchDone()
{
    // Don't reset reactive areas if, for some reason, switch is finished
    // after we've been hidden.
    if (isVisible()) {
        layout()->activate();
        redrawReactionMaps();
    }
}

void SymbolView::redrawReactionMaps()
{
    if (!scene()) {
        return;
    }

    foreach(QGraphicsView * view, scene()->views()) {
        DuiReactionMap *reactionMap = DuiReactionMap::instance(view);
        if (!reactionMap) {
            continue;
        }
        // Clear all with transparent color.
        reactionMap->setTransparentDrawingValue();
        reactionMap->setTransform(QTransform()); // Identity
        reactionMap->fillRectangle(0, 0, reactionMap->width(), reactionMap->height());

        // Draw region area with inactive color to prevent any holes in reaction map.
        reactionMap->setInactiveDrawingValue();
        reactionMap->setTransform(this, view);
        foreach(const QRect & rect, interactiveRegion().rects()) {
            reactionMap->fillRectangle(mapRectFromScene(rect));
        }

        reactionMap->setReactiveDrawingValue();

        // Draw current character view.
        if (pageSwitcher->currentWidget()) {
            static_cast<KeyButtonArea *>(pageSwitcher->currentWidget())->drawReactiveAreas(reactionMap, view);
        }

        // Draw function row.
        if (functionRow) {
            functionRow->drawReactiveAreas(reactionMap, view);
        }
    }
}


DuiVirtualKeyboardStyleContainer &SymbolView::style()
{
    return *styleContainer;
}

bool SymbolView::isFullyVisible() const
{
    return (isActive()
            && isVisible()
            && (showTimeLine->state() != QTimeLine::Running)
            && (hideTimeLine->state() != QTimeLine::Running));
}

bool SymbolView::isActive() const
{
    return ((activity == Active) || (activity == TemporarilyActive));
}


void SymbolView::stopAccurateMode()
{
    if (selectedLayout)
        selectedLayout->accurateStop();
}


QString SymbolView::pageTitle(const int pageIndex) const
{
    Q_ASSERT(pageSwitcher && (pageSwitcher->count() > pageIndex));
    const QString sectionName = qobject_cast<const KeyButtonArea *>(pageSwitcher->widget(pageIndex))->sectionModel()->name();
    return sectionName.mid(SymbolSectionPrefix.length());
}

void SymbolView::updateSymIndicator()
{
    ISymIndicator *symIndicator = functionRow->symIndicator();

     if (symIndicator) {
         if (currentLevel() == 0) {
             QString title = pageTitle(activePage);

             if (title == SymLabel) {
                 symIndicator->activateSymIndicator();
             } else if (title == AceLabel) {
                 symIndicator->activateAceIndicator();
             } else {
                 symIndicator->deactivateIndicator();
             }
         } else {
             symIndicator->deactivateIndicator();
         }
    }
}

QRegion SymbolView::interactiveRegion() const
{
    QRegion region;

    // SymbolView always occupies the same area if opened.
    if (isActive()) {
        // add sym characters and function row
        region |= mapRectToScene(keyAreaLayout.geometry()).toRect();
    }

    return region;
}

int SymbolView::pageCount() const
{
    return pageSwitcher->count();
}

int SymbolView::currentPage() const
{
    return activePage;
}

void SymbolView::showFunctionRow()
{
    if (functionRow) {
        for (int i = 0; i < keyAreaLayout.count(); i++) {
            if (keyAreaLayout.itemAt(i) == functionRow) {
                return;
            }
        }
        functionRow->setVisible(true);
        keyAreaLayout.addItem(functionRow);
        layout()->invalidate();
    }
}

void SymbolView::hideFunctionRow()
{
    if (functionRow) {
        functionRow->setVisible(false);
        keyAreaLayout.removeItem(functionRow);
        //FIXME: a tricky solution, to avoid to use a wrong width for keyboard
        const int sceneWidth = sceneManager.visibleSceneSize().width();
        pageSwitcher->setPreferredWidth(sceneWidth);
        layout()->invalidate();
    }
}
