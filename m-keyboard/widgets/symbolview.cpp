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
#include "mimkeyarea.h"
#include "reactionmappainter.h"
#include "regiontracker.h"
#include "reactionmapwrapper.h"
#include "mplainwindow.h"

#include <mkeyoverride.h>

#include <MSceneManager>
#include <MScalableImage>

#include <QDebug>
#include <QGraphicsSceneResizeEvent>
#include <QGraphicsLinearLayout>

namespace
{
    const QString SymbolSectionPrefix = "symbols";
    const QString SymbolSectionSym = SymbolSectionPrefix + "0";

    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";
};


SymbolView::SymbolView(const LayoutsManager &layoutsManager, const MVirtualKeyboardStyleContainer *style,
                       const QString &layout, QGraphicsWidget *parent)
    : MWidget(parent),
      ReactionMapPaintable(),
      styleContainer(style),
      sceneManager(*MPlainWindow::instance()->sceneManager()),
      activity(Inactive),
      activePage(0),
      shiftState(ModifierClearState),
      layoutsMgr(layoutsManager),
      pageSwitcher(0),
      currentOrientation(sceneManager.orientation()),
      currentLayout(layout),
      mouseDownKeyArea(false),
      mainLayout(new QGraphicsLinearLayout(Qt::Vertical, this)),
      activeState(MInputMethod::OnScreen),
      hideOnQuickPick(false),
      hideOnSpaceKey(false)
{
    setObjectName("SymbolView");
    RegionTracker::instance().addRegion(*this);
    RegionTracker::instance().addInputMethodArea(*this);

    connect(&eventHandler, SIGNAL(keyPressed(KeyEvent)),
            this,          SIGNAL(keyPressed(KeyEvent)));
    connect(&eventHandler, SIGNAL(keyReleased(KeyEvent)),
            this,          SIGNAL(keyReleased(KeyEvent)));
    connect(&eventHandler, SIGNAL(keyClicked(KeyEvent)),
            this,          SIGNAL(keyClicked(KeyEvent)));
    connect(&eventHandler, SIGNAL(longKeyPressed(const KeyEvent &)),
            this,          SIGNAL(longKeyPressed(const KeyEvent &)));
    connect(&eventHandler, SIGNAL(shiftPressed(bool)),
            this,          SLOT(handleShiftPressed(bool)));

    connect(&layoutsManager, SIGNAL(hardwareLayoutChanged()),
            this, SLOT(handleHwLayoutChange()));

    enableMultiTouch = MGConfItem(MultitouchSettings).value().toBool();

    hide();
    setupLayout();
    reloadContent();

    // Request a reaction map painting if it appears
    connect(this, SIGNAL(displayEntered()), &signalForwarder, SIGNAL(requestRepaint()));
}


SymbolView::~SymbolView()
{}

void SymbolView::setupLayout()
{
    // Layout widths are set in organizeContent().

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);

    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    Grip *symbolViewGrip = new Grip(this);
    symbolViewGrip->setObjectName("KeyboardHandle");
    mainLayout->insertItem(GripIndex, symbolViewGrip);

    // Urghs, we have a non-virtual override for setLayout ...
    QGraphicsWidget::setLayout(mainLayout);

    connectHandle(symbolViewGrip);
}

void SymbolView::connectHandle(Handle *handle)
{
    connect(handle, SIGNAL(flickLeft(FlickGesture)),
            this,   SLOT(switchToNextPage()),
            Qt::UniqueConnection);

    connect(handle, SIGNAL(flickRight(FlickGesture)),
            this,   SLOT(switchToPrevPage()),
            Qt::UniqueConnection);

    connect(handle, SIGNAL(flickDown(FlickGesture)),
            this,   SLOT(hideSymbolView()),
            Qt::UniqueConnection);
}

void SymbolView::reloadContent()
{
    if (activeState == MInputMethod::OnScreen) {
        // Get layout model which for current layout and orientation.
        const LayoutData *layoutData = layoutsMgr.layout(currentLayout, LayoutData::General, currentOrientation);

        loadSwitcherPages(layoutData, activePage);
        setShiftState(shiftState);
    } else if (activeState == MInputMethod::Hardware && currentOrientation == M::Landscape) {
        const LayoutData *layoutData = layoutsMgr.hardwareLayout(LayoutData::General, M::Landscape);
        if (!layoutData) {
            // Get it by layout then.
            layoutData = layoutsMgr.layout(currentLayout, LayoutData::General, M::Landscape);
        }

        loadSwitcherPages(layoutData, activePage);
        setShiftState(shiftState); // Sets level for sym pages.
    }
    layout()->invalidate();
}

void SymbolView::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const MScalableImage *background = style()->backgroundImage();

    if (background) {
        // Background covers everything except top layout.
        background->draw(mainLayout->itemAt(KeyboardIndex)->geometry().toRect(), painter);
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
    } else if (change == QGraphicsItem::ItemVisibleChange && value.toBool()) {
        organizeContent();
    }
    return QGraphicsItem::itemChange(change, value);
}

void SymbolView::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    // Nothing, just stop the event from propagating
}


bool SymbolView::sceneEventFilter(QGraphicsItem */*watched*/, QEvent *event)
{
    if (pageSwitcher) {
        switch (event->type()) {
        case QEvent::GraphicsSceneMousePress: {
            mouseDownKeyArea = true;
        } break;
        case QEvent::GraphicsSceneMouseRelease: {
            // Even though at this point ungrabbing the child widget would allow release
            // event to reach it the order is wrong. We now cope with the situation because
            // we know MImAbstractKeyAreas will ungrab even an implicit grab on release event.
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
    return false;
}

void SymbolView::prepareToOrientationChange()
{
    qDebug() << __PRETTY_FUNCTION__;
}


void SymbolView::finalizeOrientationChange()
{
    organizeContent();
}

void SymbolView::showSymbolView(SymbolView::ShowMode mode)
{
    if (mode == FollowMouseShowMode) {
        activity = TemporarilyActive;
    } else {
        activity = Active;
    }
    show();
    hideOnQuickPick = true;
    hideOnSpaceKey = false;
}


void SymbolView::hideSymbolView(SymbolView::HideMode mode)
{
    hide();
    if (activity == TemporarilyInactive && mode == NormalHideMode) {
        activity = Inactive;
        return;
    }

    if (!isActive()) {
        return;
    }

    if (mode == NormalHideMode) {
        pageSwitcher->setCurrent(0);
    }

    if (mode == TemporaryHideMode) {
        activity = TemporarilyInactive;
    } else {
        activity = Inactive;
    }
}


void SymbolView::loadSwitcherPages(const LayoutData *kbLayout, const unsigned int selectPage)
{
    if (!kbLayout) {
        return;
    }

    bool hadPageSwitcher = true;
    if(!pageSwitcher) {
        hadPageSwitcher = false;
        pageSwitcher = new HorizontalSwitcher(this);
    }

    pageSwitcher->deleteAll();
    pageSwitcher->setLooping(true);
    pageSwitcher->setAnimationEnabled(false);

    connect(pageSwitcher, SIGNAL(switchStarting(QGraphicsWidget *, QGraphicsWidget *)),
            this,         SLOT(onSwitchStarting(QGraphicsWidget *, QGraphicsWidget *)));
    connect(pageSwitcher, SIGNAL(switchDone(QGraphicsWidget *, QGraphicsWidget *)),
            this,         SLOT(onSwitchDone()));

    LayoutData::SharedLayoutSection symbolSection;

    // Add special Sym section always as the first, if present.
    symbolSection = kbLayout->section(SymbolSectionSym);
    if (!symbolSection.isNull()) {
        addPage(symbolSection);
    }

    // Add all others.
    for (int i = 0; i < kbLayout->numSections(); ++i) {
        symbolSection = kbLayout->section(i);

        // Skip those that are not symbol sections.
        if (symbolSection->name().startsWith(SymbolSectionPrefix)
            // Skip also sym section because we added it already.
            && (symbolSection->name() != SymbolSectionSym)) {
            addPage(symbolSection);
        }
    }

    if (pageSwitcher->count() >= 0) {
        activePage = (selectPage >= static_cast<unsigned int>(pageSwitcher->count()) ? 0 : selectPage);
        pageSwitcher->setCurrent(activePage);
    }

    pageSwitcher->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    if (!hadPageSwitcher) {
        mainLayout->insertItem(KeyboardIndex, pageSwitcher);
    }
}


void SymbolView::addPage(const LayoutData::SharedLayoutSection &symbolSection)
{
    MImAbstractKeyArea *page = createMImAbstractKeyArea(symbolSection);

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

MImAbstractKeyArea *SymbolView::createMImAbstractKeyArea(const LayoutData::SharedLayoutSection &section,
                                               bool enablePopup)
{
    MImAbstractKeyArea *keysWidget = 0;

    if (!section.isNull()) {
        keysWidget = new MImKeyArea(section, enablePopup);

        eventHandler.addEventSource(keysWidget);

        connect(keysWidget, SIGNAL(keyClicked(const MImAbstractKey *, QString, bool, QPoint)),
                this, SLOT(handleKeyClicked(const MImAbstractKey *)));
    }

    return keysWidget;
}

void SymbolView::organizeContent()
{
    const M::Orientation orientation(sceneManager.orientation());

    resize(sceneManager.visibleSceneSize().width(), size().height());

    if (currentOrientation != orientation) {
        currentOrientation = orientation;
        reloadContent();
    }
}

void SymbolView::setShiftState(ModifierState newShiftState)
{
    const int level = newShiftState != ModifierClearState ? 1 : 0;
    shiftState = newShiftState;

    for (int i = 0; i < pageSwitcher->count(); ++i) {
        MImAbstractKeyArea *mainKba = static_cast<MImAbstractKeyArea *>(pageSwitcher->widget(i));
        if (mainKba) {
            mainKba->setShiftState(shiftState);
        }
    }

    emit levelSwitched(level);
}

int SymbolView::currentLevel() const
{
    return (shiftState != ModifierClearState);
}

void SymbolView::handleHwLayoutChange()
{
    if (activeState == MInputMethod::Hardware) {
        reloadContent();
    }
}

void SymbolView::setKeyboardState(MInputMethod::HandlerState newState)
{
    if (activeState != newState) {
        activeState = newState;
        reloadContent();
    }
}

void SymbolView::setLayout(const QString &layoutFile)
{
    if (layoutFile != currentLayout && layoutsMgr.layoutFileList().contains(layoutFile)) {
        currentLayout = layoutFile;

    // Only on-screen sym follows layout.
        if (activeState == MInputMethod::OnScreen) {
            reloadContent();
        }
    }
}

void SymbolView::switchToNextPage()
{
    pageSwitcher->switchTo((pageCount() == 2 && pageSwitcher->current() == 1)
                           ? HorizontalSwitcher::Left : HorizontalSwitcher::Right);
}

void SymbolView::switchToPrevPage()
{
    pageSwitcher->switchTo((pageCount() == 2 && pageSwitcher->current() == 0)
                           ? HorizontalSwitcher::Right : HorizontalSwitcher::Left);
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
}

void SymbolView::onSwitchDone()
{
    // Don't reset reactive areas if, for some reason, switch is finished
    // after we've been hidden.
    if (isVisible()) {
        layout()->activate();
        signalForwarder.emitRequestRepaint();
    }
    if (pageSwitcher) {
        activePage = pageSwitcher->current();
    }
}

void SymbolView::handleShiftPressed(bool shiftPressed)
{
    if (enableMultiTouch) {
        const int level = shiftPressed ? 1 : currentLevel();

        MImAbstractKeyArea *mainKba = static_cast<MImAbstractKeyArea *>(pageSwitcher->currentWidget());
        if (mainKba) {
            mainKba->switchLevel(level);
        }
    }
}

void SymbolView::handleKeyClicked(const MImAbstractKey *key)
{
    const MImKeyBinding::KeyAction keyAction = key->binding().action();

    // KeyEventHandler forwards key clicks for normal text input etc.
    // We handle here only special case of closing symbol view if certain
    // criteria is met:
    // 1) space is clicked and we were waiting for it to close symbol view.
    if ((keyAction == MImKeyBinding::ActionSpace
         && hideOnSpaceKey)

        // 2) quick pick key is clicked as first after opening symbol view
        || (key->isQuickPick()&& hideOnQuickPick)) {
        hideSymbolView();
    }
    // After first click, we won't be tracking quick pick anymore.
    hideOnQuickPick = false;

    // Set hideOnSpaceKey to true if user clicked a non-numeric symbol character.
    hideOnSpaceKey = false;
    if (keyAction == MImKeyBinding::ActionInsert) {
        bool isNumeric = false;
        (void)key->label().toInt(&isNumeric);
        if (!isNumeric) {
            hideOnSpaceKey = true;
        }
    }
}

void SymbolView::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view)
{
#ifndef HAVE_REACTIONMAP
    Q_UNUSED(reactionMap);
    Q_UNUSED(view);
    return;
#else
    // Draw region area with inactive color to prevent any holes in reaction map.
    reactionMap->setInactiveDrawingValue();
    reactionMap->setTransform(this, view);
    foreach(const QRect & rect, interactiveRegion().rects()) {
        reactionMap->fillRectangle(mapRectFromScene(rect));
    }

    reactionMap->setDrawingValue(MImReactionMap::Press, MImReactionMap::Release);

    // Draw current character view.
    if (pageSwitcher->currentWidget()) {
        static_cast<MImAbstractKeyArea *>(pageSwitcher->currentWidget())->drawReactiveAreas(reactionMap, view);
    }
#endif // HAVE_REACTIONMAP
}

const MVirtualKeyboardStyleContainer &SymbolView::style() const
{
    return *styleContainer;
}

bool SymbolView::isActive() const
{
    return ((activity == Active) || (activity == TemporarilyActive));
}

QString SymbolView::pageTitle(const int pageIndex) const
{
    Q_ASSERT(pageSwitcher && (pageSwitcher->count() > pageIndex));
    const QString sectionName = qobject_cast<const MImAbstractKeyArea *>(pageSwitcher->widget(pageIndex))->sectionModel()->name();
    return sectionName.mid(SymbolSectionPrefix.length());
}

QRegion SymbolView::interactiveRegion() const
{
    QRegion region;

    // SymbolView always occupies the same area if opened.
    if (isActive()) {
        region |= mapRectToScene(mainLayout->geometry()).toRect();
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

void SymbolView::setTemporarilyHidden(bool hidden)
{
    if (hidden && activity == Active) {
        hideSymbolView(TemporaryHideMode);
    } else if (!hidden && activity == TemporarilyInactive) {
        showSymbolView();
    }
}

bool SymbolView::isPaintable() const
{
    return isVisible();
}

void SymbolView::resetCurrentKeyArea(bool resetCapsLock)
{
    MImAbstractKeyArea *mainKba = static_cast<MImAbstractKeyArea *>(pageSwitcher->currentWidget());
    if (mainKba) {
        mainKba->reset(resetCapsLock);
    }
}

void SymbolView::setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides)
{
    pageSwitcher->setKeyOverrides(overrides);
}
