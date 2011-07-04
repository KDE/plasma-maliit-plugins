/*
 * This file is part of meego-keyboard 
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list 
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list 
 * of conditions and the following disclaimer in the documentation and/or other materials 
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be 
 * used to endorse or promote products derived from this software without specific 
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
#include "flickgesturerecognizer.h"
#include "magnifierhost.h"

#include <mkeyoverride.h>

#include <MCancelEvent>
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
      mainLayout(new QGraphicsLinearLayout(Qt::Vertical, this)),
      activeState(MInputMethod::OnScreen),
      hideOnQuickPick(false),
      hideOnSpaceKey(false),
      symKeyHeldDown(false),
      overrides()
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
    connect(&eventHandler, SIGNAL(keyCancelled(const KeyEvent &)),
            this,          SIGNAL(keyCancelled(const KeyEvent &)));

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
    connect(handle, SIGNAL(flickDown(FlickGesture)),
            this,   SIGNAL(userInitiatedHide()),
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
    pageSwitcher->setKeyOverrides(overrides);
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
    if (change == QGraphicsItem::ItemVisibleChange && value.toBool()) {
        organizeContent();
    }
    return QGraphicsItem::itemChange(change, value);
}

void SymbolView::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    // Nothing, just stop the event from propagating
}

MImAbstractKeyArea *SymbolView::activeKeyArea() const
{
    return pageSwitcher ? static_cast<MImAbstractKeyArea *>(pageSwitcher->currentWidget()) : 0;
}

void SymbolView::prepareToOrientationChange()
{
    qDebug() << __PRETTY_FUNCTION__;
}


void SymbolView::finalizeOrientationChange()
{
    organizeContent();
}

void SymbolView::showSymbolView(ShowMode mode)
{
    show();

    hideOnQuickPick = true;
    hideOnSpaceKey = false;

    if (mode == FollowMouseShowMode) {
        setActivity(TemporarilyActive);
    } else {
        setActivity(Active);
    }
}

void SymbolView::hideSymbolView(SymbolView::HideMode mode)
{
    hide();
    if (activity == TemporarilyInactive && mode == NormalHideMode) {
        setActivity(Inactive);
        return;
    }

    if (!isActive()) {
        return;
    }

    if (mode == NormalHideMode) {
        pageSwitcher->setCurrent(0);
    }

    if (mode == TemporaryHideMode) {
        setActivity(TemporarilyInactive);
    } else {
        setActivity(Inactive);
    }
}

void SymbolView::setActivity(Activity newActivity)
{
    if (newActivity == activity) {
        return;
    }

    if (pageSwitcher->currentWidget()) {
        if (newActivity == TemporarilyActive) {
            pageSwitcher->currentWidget()->ungrabGesture(FlickGestureRecognizer::sharedGestureType());
        } else if (activity == TemporarilyActive) {
            pageSwitcher->currentWidget()->grabGesture(FlickGestureRecognizer::sharedGestureType());
        }
    }

    activity = newActivity;
}


void SymbolView::loadSwitcherPages(const LayoutData *kbLayout, const unsigned int selectPage)
{
    if (!kbLayout) {
        return;
    }

    bool hadPageSwitcher = true;
    if (!pageSwitcher) {
        hadPageSwitcher = false;
        pageSwitcher = new HorizontalSwitcher(this);

        pageSwitcher->setLooping(true);
        pageSwitcher->setAnimationEnabled(false);

        connect(pageSwitcher, SIGNAL(switchDone(QGraphicsWidget *, QGraphicsWidget *)),
                this,         SLOT(onSwitchDone()));
    }

    pageSwitcher->deleteAll();

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

        connect(page, SIGNAL(flickDown()), SIGNAL(userInitiatedHide()));

        pageSwitcher->addWidget(page);
    }
}

MImAbstractKeyArea *SymbolView::createMImAbstractKeyArea(const LayoutData::SharedLayoutSection &section,
                                                         bool usePopup)
{
    MImAbstractKeyArea *keyArea = 0;

    if (!section.isNull()) {
        keyArea = MImKeyArea::create(section, usePopup, 0);
        keyArea->setSource(KeyEvent::SecondaryLayout);
        keyArea->setPanningEnabled(false);
        eventHandler.addEventSource(keyArea);

        connect(keyArea, SIGNAL(keyClicked(const MImAbstractKey *, const KeyContext &)),
                this, SLOT(handleKeyClicked(const MImAbstractKey *)));
        connect(keyArea, SIGNAL(keyPressed(const MImAbstractKey*, const KeyContext &)),
                this, SLOT(handleKeyPressed(const MImAbstractKey *)));
        connect(keyArea, SIGNAL(keyReleased(const MImAbstractKey *, const KeyContext &)),
                this, SLOT(handleKeyReleased(const MImAbstractKey *)));
    }

    return keyArea;
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

void SymbolView::handleKeyPressed(const MImAbstractKey *key)
{
    if (key->binding().action() == MImKeyBinding::ActionSym) {
        symKeyHeldDown = true;
    }
}

void SymbolView::handleKeyReleased(const MImAbstractKey *key)
{
    if (key->binding().action() == MImKeyBinding::ActionSym) {
        symKeyHeldDown = false;
    }
}

void SymbolView::handleKeyClicked(const MImAbstractKey *key)
{
    const MImKeyBinding::KeyAction keyAction = key->binding().action();
    MImAbstractKeyArea *mainKba = static_cast<MImAbstractKeyArea *>(pageSwitcher->currentWidget());
    const MImAbstractKey *lastActiveKey = MImAbstractKey::lastActiveKey();
    bool isOtherKeysPressed = lastActiveKey != 0
                              && mainKba
                              && mainKba->contains(lastActiveKey);

    // KeyEventHandler forwards key clicks for normal text input etc.
    // We handle here only special case of closing symbol view if certain
    // criteria is met:
    if (!symKeyHeldDown // never hide if sym key is held down
        // 1) symbol view was opened in temporary mode and no other pressed key.
        // (active key means there is still a key not being released)
        && ((activity == TemporarilyActive
             && key->binding().action() != MImKeyBinding::ActionSwitch
             && key->binding().action() != MImKeyBinding::ActionSym
             && !isOtherKeysPressed)

             // 2) space is clicked and we were waiting for it to close symbol view.
             || (keyAction == MImKeyBinding::ActionSpace
                 && hideOnSpaceKey)

             // 3) quick pick key is clicked as first after opening symbol view
             || (key->isQuickPick() && hideOnQuickPick))) {
        hideSymbolView();
    }

    // After first non-symkey click, we won't be tracking quick pick anymore.
    if (keyAction != MImKeyBinding::ActionSym) {
        hideOnQuickPick = false;
    }

    // Set hideOnSpaceKey to true if user clicked a non-numeric symbol character.
    if (keyAction == MImKeyBinding::ActionInsert
        && (key->label().length() != 1
            || key->label().at(0).category() != QChar::Number_DecimalDigit)) {
        hideOnSpaceKey = true;
    } else {
        hideOnSpaceKey = false;
    }

    // Don't retain temporary mode after non-symbol key click and no other pressed key.
    if (activity == TemporarilyActive
        && (key->binding().action() == MImKeyBinding::ActionSym // for non-sym key down and clicking sym key
            || symKeyHeldDown)) {
        setActivity(Active);
    }

    // Release explicit mouse grab since we have it in temporary mode.
    if (!enableMultiTouch
        && pageSwitcher->currentWidget()
        && pageSwitcher->currentWidget() == scene()->mouseGrabberItem()) {
        pageSwitcher->currentWidget()->ungrabMouse();
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

bool SymbolView::isTemporarilyActive() const
{
    return activity == TemporarilyActive;
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

void SymbolView::setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides)
{
    pageSwitcher->setKeyOverrides(overrides);
    this->overrides = overrides;
}

void SymbolView::cancelEvent(MCancelEvent *event)
{
    QGraphicsWidget *keyArea(pageSwitcher->currentWidget());
    if (keyArea) {
        scene()->sendEvent(keyArea, event);
    }
}
