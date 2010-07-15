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
#include "horizontalswitcher.h"
#include "layoutsmanager.h"
#include "symbolview.h"
#include "grip.h"
#include "sharedhandlearea.h"

#include <MSceneManager>
#include <MScalableImage>
#include <mreactionmap.h>
#include <mtheme.h>
#include <mplainwindow.h>

#include <QCoreApplication>
#include <QDebug>
#include <QGraphicsSceneResizeEvent>
#include <QPainter>

namespace
{
    const int DefaultAnimationDuration = 100;
    const int DefaultAnimationFrameCount = 20;

    const QString SymLabel("Sym");
    const QString AceLabel(QString(0xE1) + QChar(0xE7) + QChar(0xE8)); // "áçè"
    const QString SymbolSectionPrefix = "symbols ";
    const QString SymbolSectionSym = SymbolSectionPrefix + "Sym";

    const QString ObjectNameTabs("VirtualKeyboardSymTabs");
    const QString ObjectNameTabButton("VirtualKeyboardSymTabsButton");
    const QString ObjectNameCloseButton("VirtualKeyboardCloseButton");

    const QString SymCloseIcon("icon-m-input-methods-close");

    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";
};

SymbolView::AnimationGroup::AnimationGroup(SymbolView *view)
    : showTimeLine(DefaultAnimationDuration)
    , hideTimeLine(DefaultAnimationDuration)
{
    showTimeLine.setFrameRange(0, DefaultAnimationFrameCount);
    hideTimeLine.setFrameRange(0, DefaultAnimationFrameCount);

    showTimeLine.setCurveShape(QTimeLine::LinearCurve);
    hideTimeLine.setCurveShape(QTimeLine::LinearCurve);

    connect(&hideTimeLine, SIGNAL(finished()),
            view,          SLOT(onHidden()));

    connect(&showTimeLine, SIGNAL(finished()),
            view,          SLOT(onReady()));

    showAnimation.setItem(view);
    showAnimation.setTimeLine(&showTimeLine);

    hideAnimation.setItem(view);
    hideAnimation.setTimeLine(&hideTimeLine);
}

SymbolView::AnimationGroup::~AnimationGroup()
{}

void SymbolView::AnimationGroup::updatePos(int top, int bottom)
{
    showAnimation.clear();
    showAnimation.setPosAt(0.0, QPoint(0, bottom));
    showAnimation.setPosAt(1.0, QPoint(0, top));

    hideAnimation.clear();
    hideAnimation.setPosAt(0.0, QPoint(0, top));
    hideAnimation.setPosAt(1.0, QPoint(0, bottom));
}

void SymbolView::AnimationGroup::playShowAnimation()
{
    takeOverFromTimeLine(&showTimeLine, &hideTimeLine);
}

void SymbolView::AnimationGroup::playHideAnimation()
{
    takeOverFromTimeLine(&hideTimeLine, &showTimeLine);
}

bool SymbolView::AnimationGroup::hasOngoingAnimations() const
{
    return ((showTimeLine.state() == QTimeLine::Running) ||
            (hideTimeLine.state() == QTimeLine::Running));
}

void SymbolView::AnimationGroup::takeOverFromTimeLine(QTimeLine *target,
                                                      QTimeLine *origin)
{
    if (target->state() == QTimeLine::Running) {
        return;
    }

    if (origin->state() == QTimeLine::Running) {
        origin->stop();
        target->setCurrentTime(DefaultAnimationDuration - origin->currentTime());
        target->resume();
    } else {
        target->start();
    }

}

SymbolView::LinearLayoutObject::LinearLayoutObject(Qt::Orientation orientation, QGraphicsLayoutItem *parent)
    : QObject(0)
    , QGraphicsLinearLayout(orientation, parent)
{}

SymbolView::SymbolView(const LayoutsManager &layoutsManager, const MVirtualKeyboardStyleContainer *style,
                       const QString &language, QGraphicsWidget *parent)
    : MWidget(parent),
      styleContainer(style),
      anim(this),
      sceneManager(*MPlainWindow::instance()->sceneManager()),
      selectedLayout(0),
      activity(Inactive),
      activePage(0),
      shift(ModifierClearState),
      layoutsMgr(layoutsManager),
      pageSwitcher(0),
      functionRow(0),
      currentOrientation(sceneManager.orientation()),
      currentLanguage(language),
      mouseDownKeyArea(false),
      activeState(OnScreen)
{
    connect(&eventHandler, SIGNAL(keyPressed(KeyEvent)),
            this,          SIGNAL(keyPressed(KeyEvent)));
    connect(&eventHandler, SIGNAL(keyReleased(KeyEvent)),
            this,          SIGNAL(keyReleased(KeyEvent)));
    connect(&eventHandler, SIGNAL(keyClicked(KeyEvent)),
            this,          SIGNAL(keyClicked(KeyEvent)));
    connect(&eventHandler, SIGNAL(shiftPressed(bool)),
            this,          SLOT(setFunctionRowState(bool)));

    connect(&layoutsManager, SIGNAL(hardwareLayoutChanged()),
            this, SLOT(handleHwLayoutChange()));

    enableMultiTouch = MGConfItem(MultitouchSettings).value().toBool();

    hide();
    setupLayout();
    reloadContent();
}


SymbolView::~SymbolView()
{}

void SymbolView::setupLayout()
{
    // Layout widths are set in organizeContent().

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);

    if (!keyAreaLayout) {
        verticalLayout =  new LinearLayoutObject(Qt::Vertical, this);
    }

    verticalLayout->setSpacing(0);
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    Grip *symbolViewGrip = new Grip(this);
    symbolViewGrip->setObjectName("KeyboardHandle");
    verticalLayout->addItem(symbolViewGrip);
    connectHandle(symbolViewGrip);

    if (!keyAreaLayout) {
        keyAreaLayout = new LinearLayoutObject(Qt::Vertical);
    }

    keyAreaLayout->setSpacing(style()->spacingVertical());
    keyAreaLayout->setContentsMargins(style()->paddingLeft(), style()->paddingTop(),
                                      style()->paddingRight(), style()->paddingBottom());

    verticalLayout->addItem(keyAreaLayout);
}

void SymbolView::connectHandle(Handle *handle)
{
    connect(handle, SIGNAL(flickLeft(FlickGesture)),
            this,   SLOT(switchToNextPage()),
            Qt::UniqueConnection);

    connect(handle, SIGNAL(flickRight(lickGesture)),
            this,   SLOT(switchToPrevPage()),
            Qt::UniqueConnection);

    connect(handle, SIGNAL(flickDown(FlickGesture)),
            this,   SLOT(hideSymbolView()),
            Qt::UniqueConnection);
}

void SymbolView::reloadContent()
{
    if (activeState == OnScreen) {
        // Get layout model which for current language and orientation.
        const LayoutData *layoutData = layoutsMgr.layout(currentLanguage, LayoutData::General, currentOrientation);
        Q_ASSERT(layoutData);

        loadSwitcherPages(layoutData, activePage);
        loadFunctionRow(layoutData);
        setShiftState(shift);
    } else if (activeState == Hardware && currentOrientation == M::Landscape) {
        const LayoutData *layoutData = layoutsMgr.hardwareLayout(LayoutData::General, M::Landscape);
        if (!layoutData) {
            // Get it by language then.
            layoutData = layoutsMgr.layout(currentLanguage, LayoutData::General, M::Landscape);
        }
        Q_ASSERT(layoutData);

        loadSwitcherPages(layoutData, activePage);
        loadFunctionRow(0);
        setShiftState(shift); // Sets level for sym pages.
    }
}

void SymbolView::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const MScalableImage *background = style()->backgroundImage();

    if (background) {
        // Background covers everything except top layout.
        background->draw(keyAreaLayout->geometry().toRect(), painter);
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
    int bottom = sceneManager.visibleSceneSize().height();
    const int top = sceneManager.visibleSceneSize().height() - height;

    if (sharedHandleArea) {
        bottom += sharedHandleArea->size().height();
    }

    setPos(0, isVisible() ? top : bottom);

    anim.updatePos(top, bottom);
}

void SymbolView::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    MWidget::resizeEvent(event);

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
    anim.playShowAnimation();

    emit showingUp();
}


void
SymbolView::hideSymbolView(SymbolView::HideMode mode)
{
    if (isActive()) {
        if (mode == NormalHideMode) {
            changePage(0);
        }

        anim.playHideAnimation();
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
        pageSwitcher->switchTo(id < activePage ? HorizontalSwitcher::Left
                                               : HorizontalSwitcher::Right);
        activePage = id;
    }

    selectedLayout = qobject_cast<KeyButtonArea *>(pageSwitcher->currentWidget());
    Q_ASSERT(selectedLayout);

    updateSymIndicator();
}

void SymbolView::loadSwitcherPages(const LayoutData *kbLayout, const unsigned int selectPage)
{
    layout()->invalidate();

    if (pageSwitcher) {
        keyAreaLayout->removeItem(pageSwitcher);
        delete pageSwitcher;
        pageSwitcher = 0;
    }
    selectedLayout = 0; // invalid now so clear

    if (!kbLayout) {
        return;
    }

    pageSwitcher = new HorizontalSwitcher(this);

    connect(pageSwitcher, SIGNAL(switchStarting(QGraphicsWidget *, QGraphicsWidget *)),
            this,         SLOT(onSwitchStarting(QGraphicsWidget *, QGraphicsWidget *)));
    connect(pageSwitcher, SIGNAL(switchDone(QGraphicsWidget *, QGraphicsWidget *)),
            this,         SLOT(switchDone()));

    QSharedPointer<const LayoutSection> symbolSection;

    // Add special Sym section always as the first, if present.
    symbolSection = kbLayout->section(SymbolSectionSym);
    if (!symbolSection.isNull()) {
        addPage(symbolSection);
    }

    // Add all others.
    for (int i = 0; i < kbLayout->numSections(); ++i) {
        symbolSection = kbLayout->section(i);

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

    pageSwitcher->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    keyAreaLayout->addItem(pageSwitcher);
}

void SymbolView::loadFunctionRow(const LayoutData *layout)
{
    this->layout()->invalidate();

    if (functionRow) {
        keyAreaLayout->removeItem(functionRow);
        delete functionRow;
        functionRow = 0;
    }

    if (!layout) {
        return;
    }

    functionRow = createKeyButtonArea(layout->section(LayoutData::functionkeySection),
                                      KeyButtonArea::ButtonSizeFunctionRow, false);

    if (functionRow) {
        functionRow->setObjectName("SymbolFunctionRow");
        functionRow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        keyAreaLayout->addItem(functionRow);
    }
}

void SymbolView::addPage(QSharedPointer<const LayoutSection> symbolSection)
{
    KeyButtonArea *page = createKeyButtonArea(symbolSection);

    if (page) {
        page->setObjectName("SymbolMainRow");

        connect(this, SIGNAL(levelSwitched(int)), page, SLOT(switchLevel(int)));

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

        eventHandler.addEventSource(keysWidget);
    }

    return keysWidget;
}

void SymbolView::organizeContent()
{
    const M::Orientation orientation(sceneManager.orientation());
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

void SymbolView::setShiftState(ModifierState newShiftState)
{
    int level = newShiftState != ModifierClearState ? 1 : 0;

    shift = newShiftState;
    if (!enableMultiTouch && functionRow) {
        functionRow->switchLevel(level);
        functionRow->setShiftState(newShiftState);
    }
    emit levelSwitched(level);
    updateSymIndicator();
}

int SymbolView::currentLevel() const
{
    return (shift != ModifierClearState);
}

void SymbolView::handleHwLayoutChange()
{
    if (activeState == Hardware) {
        reloadContent();
    }
}

void SymbolView::setKeyboardState(MIMHandlerState newState)
{
    if (activeState != newState) {
        activeState = newState;
        reloadContent();
    }
}

void SymbolView::setLanguage(const QString &lang)
{
    if (lang != currentLanguage && layoutsMgr.languageList().contains(lang)) {
        currentLanguage = lang;

        // Only on-screen sym follows language.
        if (activeState == OnScreen) {
            reloadContent();
        }
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

void SymbolView::setFunctionRowState(bool shiftPressed)
{
    if (enableMultiTouch && functionRow) {
        functionRow->switchLevel(shiftPressed ? 1 : 0);
    }
}

void SymbolView::redrawReactionMaps()
{
    if (!scene()) {
        return;
    }

    foreach(QGraphicsView * view, scene()->views()) {
        MReactionMap *reactionMap = MReactionMap::instance(view);
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


const MVirtualKeyboardStyleContainer &SymbolView::style() const
{
    return *styleContainer;
}

bool SymbolView::isFullyVisible() const
{
    return (isActive()
            && isVisible()
            && !anim.hasOngoingAnimations());
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
    if (!functionRow) {
        return;
    }

    ISymIndicator *symIndicator = functionRow->symIndicator();

     if (symIndicator) {
         if (functionRow->level() == 0) {
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
        region |= mapRectToScene(verticalLayout->geometry()).toRect();
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

void SymbolView::setSharedHandleArea(const QPointer<SharedHandleArea> &handleArea)
{
    sharedHandleArea = handleArea;
    reposition(size().toSize().height());
}
