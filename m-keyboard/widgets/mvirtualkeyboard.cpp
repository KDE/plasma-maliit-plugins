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



#include "mvirtualkeyboard.h"
#include "mvirtualkeyboardstyle.h"
#include "horizontalswitcher.h"
#include "keyboarddata.h"
#include "layoutdata.h"
#include "layoutsmanager.h"
#include "notification.h"
#include "vkbdatakey.h"
#include "mimtoolbar.h"

#include <QDebug>
#include <QPainter>
#include <QGraphicsLinearLayout>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsWidget>

#include <MButton>
#include <MScalableImage>
#include <MSceneManager>
#include <duireactionmap.h>
#include <mtimestamp.h>
#include <mplainwindow.h>
#include <MApplication>


const QString MVirtualKeyboard::WordSeparators("-.,!? \n");


MVirtualKeyboard::MVirtualKeyboard(const LayoutsManager &layoutsManager,
                                       QGraphicsWidget *parent)
    : MWidget(parent),
      mainLayout(new QGraphicsLinearLayout(Qt::Vertical, this)),
      currentLevel(0),
      numLevels(2),
      activity(Inactive),
      styleContainer(0),
      sceneManager(MPlainWindow::instance()->sceneManager()),
      shiftLevel(ShiftOff),
      currentLayoutType(LayoutData::General),
      currentOrientation(sceneManager->orientation()),
      hideShowByFadingOnly(false),
      sendRegionUpdates(true),
      regionUpdateRequested(false),
      layoutsMgr(layoutsManager),
      mainKeyboardSwitcher(0),
      notification(0),
      imToolbar(0),
      numberKeyboard(0),
      numberLayout(0),
      phoneNumberKeyboard(0),
      phoneNumberLayout(0),
      activeState(OnScreen)
{
    setObjectName("MVirtualKeyboard");
    hide();

    styleContainer = new MVirtualKeyboardStyleContainer;
    styleContainer->initialize(objectName(), "MVirtualKeyboardView", 0);

    notification = new Notification(styleContainer, this);

    createSwitcher();

    createToolbar();

    setupTimeLine();

    // setting maximum width explicitly. otherwise max width of children will override. (bug?)
    setMaximumWidth(QWIDGETSIZE_MAX);
    setMinimumWidth(0);

    // add stuff to the layout
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addItem(imToolbar);
    mainLayout->setAlignment(imToolbar, Qt::AlignCenter);

    mainLayout->addItem(mainKeyboardSwitcher); // start in qwerty

    // create main widgets for number and phone number, and make the initial button areas
    numberKeyboard = new QGraphicsWidget(this);
    numberKeyboard->hide();
    numberLayout = createKeyAreaLayout(numberKeyboard);

    phoneNumberKeyboard = new QGraphicsWidget(this);
    phoneNumberKeyboard->hide();
    phoneNumberLayout = createKeyAreaLayout(phoneNumberKeyboard);

    connect(&layoutsMgr, SIGNAL(languagesChanged()), this, SLOT(languageReset()));
    languageReset(); // creates keyboard widgets

    organizeContent(currentOrientation);
}


MVirtualKeyboard::~MVirtualKeyboard()
{
    delete mainKeyboardSwitcher;
    mainKeyboardSwitcher = 0;

    delete imToolbar;
    imToolbar = 0;

    delete notification;
    notification = 0;

    delete styleContainer;
    styleContainer = 0;
}


void
MVirtualKeyboard::prepareToOrientationChange()
{
    qDebug() << __PRETTY_FUNCTION__ << geometry();

    // if inactive, just ignore
    if (activity != Active) {
        return;
    }

    hideKeyboard(true, true);
}


void
MVirtualKeyboard::finalizeOrientationChange()
{
    qDebug() << __PRETTY_FUNCTION__ << geometry();

    // if inactive, just ignore
    if (activity != TemporarilyInactive) {
        return;
    }

    showKeyboard(true);
}

void MVirtualKeyboard::createToolbar()
{
    imToolbar = new MImToolbar(style(), this);

    // Set z value below default level (0.0) so popup will be on top of toolbar.
    // More correct way to fix this, though more difficult, would be to have only one
    // popup instance in dvkb and set its z value higher.
    imToolbar->setZValue(-1.0);

    connect(imToolbar, SIGNAL(regionUpdated()),
            this, SLOT(sendVKBRegion()));
    connect(imToolbar, SIGNAL(copyPasteRequest(CopyPasteState)),
            this, SIGNAL(copyPasteRequest(CopyPasteState)));
    connect(imToolbar, SIGNAL(sendKeyEventRequest(const QKeyEvent &)),
            this, SIGNAL(sendKeyEventRequest(const QKeyEvent &)));
    connect(imToolbar, SIGNAL(sendStringRequest(const QString &)),
            this, SIGNAL(sendStringRequest(const QString &)));
    connect(imToolbar, SIGNAL(copyPasteClicked(CopyPasteState)),
            this, SIGNAL(copyPasteClicked(CopyPasteState)));
    connect(imToolbar, SIGNAL(indicatorClicked()),
            this, SIGNAL(indicatorClicked()));
}

void MVirtualKeyboard::showToolbarWidget(const QString &name)
{
    imToolbar->showToolbarWidget(name);
}

void MVirtualKeyboard::hideToolbarWidget()
{
    imToolbar->hideToolbarWidget();
}

void
MVirtualKeyboard::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    mTimestamp("MVirtualKeyboard", "start");

    // The second item in layout holds the currently used keyboard. Draw under it.
    const QRectF backgroundGeometry = layout()->itemAt(1)->geometry();

    if (MApplication::softwareRendering()) {
        if (backgroundPixmap.isNull()
            || backgroundPixmap->size() != backgroundGeometry.size().toSize()) {
            const MScalableImage *background = style()->backgroundImage();
            QPainter pixmapPainter;

            backgroundPixmap = QSharedPointer<QPixmap>(
                                   new QPixmap(backgroundGeometry.size().toSize()));

            backgroundPixmap->fill(QColor(1, 1, 1, 0));
            pixmapPainter.begin(backgroundPixmap.data());
            background->draw(backgroundPixmap->rect(), &pixmapPainter);
            pixmapPainter.end();
        }

        QRectF mappedRect = option->exposedRect;
        mappedRect.translate(-backgroundGeometry.left(), -backgroundGeometry.top());
        painter->drawPixmap(option->exposedRect, *backgroundPixmap, mappedRect);
    } else {
        const MScalableImage *background = style()->backgroundImage();

        if (background) {
            background->draw(backgroundGeometry.toRect(), painter);
        }
    }
    mTimestamp("MVirtualKeyboard", "end");
}

bool MVirtualKeyboard::isFullyVisible() const
{
    return ((activity == Active)
            && isVisible()
            && (showHideTimeline.state() != QTimeLine::Running));
}

void
MVirtualKeyboard::switchLevel()
{
    switch (shiftLevel) {
    case ShiftOff:
        currentLevel = 0;
        break;
    case ShiftOn:
        currentLevel = 1;
        break;
    case ShiftLock:
        currentLevel = 1;
        break;
    }

    const bool capsLock = (shiftLevel == ShiftLock);

    for (int i = 0; i < mainKeyboardSwitcher->count(); ++i) {
        // the subwidgets have main section as first item in their layout, function row as second.
        // handling main section:
        static_cast<KeyButtonArea *>(mainKeyboardSwitcher->widget(i)->layout()->itemAt(0))->
                switchLevel(currentLevel, capsLock);

        // TODO: When introducing multitouch we should make function row to change level
        // according to pressed state of the shift button rather than general level.
        static_cast<KeyButtonArea *>(mainKeyboardSwitcher->widget(i)->layout()->itemAt(1))->
                switchLevel(currentLevel, capsLock);
    }
}


void
MVirtualKeyboard::setShiftState(ShiftLevel level)
{
    if (shiftLevel != level) {
        shiftLevel = level;
        switchLevel();
        emit shiftLevelChanged();
    }
}


void
MVirtualKeyboard::setupTimeLine()
{
    showHideTimeline.setCurveShape(QTimeLine::EaseInCurve);
    showHideTimeline.setFrameRange(0, ShowHideFrames);
    showHideTimeline.setDuration(ShowHideTime);
    showHideTimeline.setUpdateInterval(ShowHideInterval);
    connect(&showHideTimeline, SIGNAL(frameChanged(int)), this, SLOT(fade(int)));
    connect(&showHideTimeline, SIGNAL(finished()), this, SLOT(showHideFinished()));
}


void
MVirtualKeyboard::flickLeftHandler()
{
    if (!mainKeyboardSwitcher->isRunning()) {
        mainKeyboardSwitcher->switchTo(HorizontalSwitcher::Right);
        setLanguage(mainKeyboardSwitcher->current());
    }
}


void
MVirtualKeyboard::flickUpHandler(const KeyBinding *binding)
{
    if (binding && binding->action() == KeyBinding::ActionSym) {
        emit showSymbolViewRequested();
    }
}


void
MVirtualKeyboard::flickRightHandler()
{
    if (!mainKeyboardSwitcher->isRunning()) {
        mainKeyboardSwitcher->switchTo(HorizontalSwitcher::Left);
        setLanguage(mainKeyboardSwitcher->current());
    }
}


void MVirtualKeyboard::showKeyboard(bool fadeOnly)
{
    organizeContent(sceneManager->orientation());

    if (activity == Active) {
        // already showing up
        return;
    }

    activity = Active;

    showHideTimeline.setDirection(QTimeLine::Forward);

    if (showHideTimeline.state() != QTimeLine::Running) {
        hideShowByFadingOnly = fadeOnly;

        if (!fadeOnly) {
            // vkb is positioned right below the visible area
            setPos(0, sceneManager->visibleSceneSize().height());
        } else {
            setPos(0, sceneManager->visibleSceneSize().height() - actualHeight());
        }

        suppressRegionUpdate(true); // Don't send separate region update for imtoolbar.
        showHideTimeline.start();
        show();
        // We need to send one initial region update to have passthruwindow make
        // the window visible.  The final region will be sent when the animation
        // is finished.  We do this after show() so that we can use region().
        // We cannot use sendVKBRegion() since region updates are suppressed.
        emit regionUpdated(region());
    } else if (hideShowByFadingOnly) {
        // fade() doesn't alter the position when we're just fading
        setPos(0, sceneManager->visibleSceneSize().height() - actualHeight());
        show();
    }
}


void MVirtualKeyboard::hideKeyboard(bool fadeOnly, bool temporary)
{
    if (scene() && scene()->views().count() > 0) {
        DuiReactionMap *reactionMap = DuiReactionMap::instance(scene()->views()[0]);
        if (reactionMap) {
            reactionMap->clear();
        }
    }

    qDebug() << __PRETTY_FUNCTION__ << "HIDE";

    if (activity == Active) {
        showHideTimeline.setDirection(QTimeLine::Backward);

        if (showHideTimeline.state() != QTimeLine::Running) {
            hideShowByFadingOnly = fadeOnly;
            suppressRegionUpdate(true);
            showHideTimeline.start();
        }

        activity = temporary ? TemporarilyInactive : Inactive;
    }
}

void MVirtualKeyboard::resetState()
{
    // Default state for shift is ShiftOff.
    setShiftState(ShiftOff);

    // Dead keys are unlocked in KeyButtonArea::onHide().
    // As long as this method is private, and only called from
    // hideKeyboard(), we don't have to explicitly unlock them.
}

QGraphicsLinearLayout *MVirtualKeyboard::createKeyAreaLayout(QGraphicsWidget *parent)
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical, parent);
    layout->setContentsMargins(style()->paddingLeft(), style()->paddingTop(),
                               style()->paddingRight(), style()->paddingBottom());
    layout->setSpacing(style()->spacingVertical());

    return layout;
}


int MVirtualKeyboard::actualHeight() const
{
    int result = size().height();

    if ((activeState != OnScreen)) {
        result = imToolbar->size().height();
    }

    return result;
}

void MVirtualKeyboard::organizeContent(M::Orientation orientation)
{
    if (currentOrientation != orientation) {
        currentOrientation = orientation;

        // Reload for portrait/landscape
        int index = mainKeyboardSwitcher->current();
        recreateKeyboards();
        mainKeyboardSwitcher->setCurrent(index);

        // invalidate layouts so we get updated size infos
        mainLayout->invalidate();
        numberKeyboard->layout()->invalidate();
        phoneNumberKeyboard->layout()->invalidate();
    }

    resize(MPlainWindow::instance()->visibleSceneSize().width(), mainLayout->preferredHeight());
    if ((activity == Active) && (showHideTimeline.state() != QTimeLine::Running)) {
        setPos(0, sceneManager->visibleSceneSize().height() - actualHeight());
    }
}


void
MVirtualKeyboard::fade(int frame)
{
    const float opacity = float(frame) / ShowHideFrames;
    const QSize sceneSize = sceneManager->visibleSceneSize();

    if (!hideShowByFadingOnly) {
        setPos(0, sceneSize.height() - (opacity * actualHeight()));
    }

    this->setOpacity(opacity);
    update();
}


void
MVirtualKeyboard::showHideFinished()
{
    const bool hiding = (showHideTimeline.direction() == QTimeLine::Backward);

    if (hiding) {
        // Assuming here we don't want to reset vkb state if temporarily hiding.
        // For example, don't reset state when hiding for screen rotation.
        if (activity == Inactive) {
            resetState();
        }

        hide();
        emit hidden();
    } else {
        emit opened();
    }

    sendVKBRegion();
    suppressRegionUpdate(false);
}


void MVirtualKeyboard::sendVKBRegion()
{
    regionUpdateRequested = true;

    if (!sendRegionUpdates)
        return;

    emit regionUpdated(region());
}

QRegion MVirtualKeyboard::region(const bool includeToolbar) const
{
    QRegion region;

    if (isVisible()) {
        imToolbar->layout()->activate();
        mainLayout->activate();
        if (includeToolbar) {
            region |= imToolbar->region();
        }

        // Main keyboard area (qwerty/number/etc.)
        if (activeState == OnScreen) {
            mainLayout->activate();
            region |= mapRectToScene(mainLayout->itemAt(1)->geometry()).toRect();
        }
    }

    return region;
}

void MVirtualKeyboard::setKeyboardState(MIMHandlerState newState)
{
    if (activeState == newState) {
        return;
    }

    activeState = newState;
    qDebug() << __PRETTY_FUNCTION__ << newState << imToolbar->geometry() << imToolbar->region();
    resetState();
    if (newState == OnScreen) {
        imToolbar->hideIndicatorButton();
    } else {
        imToolbar->showIndicatorButton();
    }
    qDebug() << __PRETTY_FUNCTION__ << imToolbar->geometry() << imToolbar->region();

    static_cast<QGraphicsWidget *>(mainLayout->itemAt(1))->setVisible(newState == OnScreen);
    organizeContent(currentOrientation);
    if (isVisible()) {
        sendVKBRegion();
    }
}

MIMHandlerState MVirtualKeyboard::keyboardState() const
{
    return activeState;
}

void MVirtualKeyboard::drawButtonsReactionMaps(DuiReactionMap *reactionMap, QGraphicsView *view)
{
    // Depending on which keyboard type is currently shown
    // we must pick the correct KeyButtonArea(s).

    // Keyboard layout consists of one or more KeyButtonAreas,
    // such as qwerty + function row.
    QGraphicsLayout *keyboardLayout = 0;

    QGraphicsLayoutItem *item = mainLayout->itemAt(1);
    if (item == mainKeyboardSwitcher) {
        keyboardLayout = mainKeyboardSwitcher->currentWidget()->layout();
    } else {
        keyboardLayout = static_cast<QGraphicsWidget *>(item)->layout();
    }

    // Draw reactive areas for all KeyButtonAreas in the layout.
    // Note: Layout must not contain anything else than KeyButtonAreas.
    if (keyboardLayout) {
        for (int i = 0; i < keyboardLayout->count(); ++i) {
            static_cast<KeyButtonArea *>(keyboardLayout->itemAt(i))->drawReactiveAreas(reactionMap,
                                                                                       view);
        }
    }
}


QString MVirtualKeyboard::layoutLanguage() const
{
    return layoutsMgr.keyboardLanguage(currentLanguage);
}

QString MVirtualKeyboard::selectedLanguage() const
{
    return currentLanguage;
}

void MVirtualKeyboard::redrawReactionMaps()
{
    if ((activity != Active) || !scene()) {
        return;
    }

    foreach (QGraphicsView *view, scene()->views()) {
        DuiReactionMap *reactionMap = DuiReactionMap::instance(view);
        if (!reactionMap) {
            continue;
        }

        // Clear all with transparent color.
        reactionMap->setTransparentDrawingValue();
        reactionMap->setTransform(QTransform()); // Identity
        reactionMap->fillRectangle(0, 0, reactionMap->width(), reactionMap->height());

        // Draw keyboard area with inactive color to prevent transparent holes.
        reactionMap->setInactiveDrawingValue();
        reactionMap->setTransform(this, view);
        reactionMap->fillRectangle(layout()->itemAt(1)->geometry());

        if (activeState == OnScreen) {
            drawButtonsReactionMaps(reactionMap, view);
        }

        if (imToolbar->isVisible()) {
            imToolbar->drawReactiveAreas(reactionMap, view);
        }
    }
}


MVirtualKeyboardStyleContainer &MVirtualKeyboard::style()
{
    return *styleContainer;
}


void MVirtualKeyboard::setKeyboardType(const int type)
{
    // map m content type to layout model type
    LayoutData::LayoutType newLayoutType = LayoutData::General;
    QGraphicsWidget *newWidget = 0;

    switch (type) {
    case M::NumberContentType:
        newLayoutType = LayoutData::Number;
        break;

    case M::PhoneNumberContentType:
        newLayoutType = LayoutData::PhoneNumber;
        break;
    }

    if (newLayoutType == currentLayoutType) {
        return;
    }

    currentLayoutType = newLayoutType;

    // remove what currently is in the keyboard position in the main layout
    QGraphicsLayoutItem *previousItem = mainLayout->itemAt(1);
    mainLayout->removeItem(previousItem);
    static_cast<QGraphicsWidget *>(previousItem)->hide();

    // show appropriate keyboard widget
    switch (currentLayoutType) {
    case LayoutData::Number:
        newWidget = numberKeyboard;
        break;

    case LayoutData::PhoneNumber:
        newWidget = phoneNumberKeyboard;
        break;
    default:
        newWidget = mainKeyboardSwitcher;
        break;
    }

    mainLayout->insertItem(1, newWidget);
    if (activeState == OnScreen) {
        newWidget->show();
    } else {
        newWidget->hide();
    }

    // resize and update keyboards if needed
    organizeContent(currentOrientation);

    if (isVisible()) {
        sendVKBRegion();
    }
}

void MVirtualKeyboard::suppressRegionUpdate(bool suppress)
{
    // Call sendVKBRegion if suppress is now released and was previously
    // set on, and at least one region update has been requested.
    if (!suppress && !sendRegionUpdates && regionUpdateRequested) {

        // Allow sendVKBRegion to send region
        sendRegionUpdates = true;

        // and send the region.
        sendVKBRegion();
    }

    sendRegionUpdates = !suppress;
    regionUpdateRequested = false;
}


void MVirtualKeyboard::stopAccurateMode()
{
    for (int i = 0; i < mainKeyboardSwitcher->count(); ++i) {
        // accurate only used for qwerty part of main keyboards
        static_cast<KeyButtonArea *>(mainKeyboardSwitcher->widget(i)->layout()->itemAt(0))->
            accurateStop();
    }
}


MVirtualKeyboard::ShiftLevel MVirtualKeyboard::shiftStatus() const
{
    return ShiftLevel(shiftLevel);
}


bool MVirtualKeyboard::isAccurateMode() const
{
    KeyButtonArea *activeSection = 0;
    QGraphicsWidget *current = mainKeyboardSwitcher->currentWidget();

    if (current) {
        activeSection = static_cast<KeyButtonArea *>(current->layout()->itemAt(0));
    }

    return activeSection && activeSection->isAccurateMode();
}


const LayoutData *MVirtualKeyboard::currentLayoutModel() const
{
    return layoutsMgr.layout(currentLanguage, currentLayoutType, currentOrientation);
}


void MVirtualKeyboard::setLanguage(int languageIndex)
{
    qDebug() << __PRETTY_FUNCTION__;
    if ((languageIndex < 0) || (languageIndex >= layoutsMgr.languageCount())) {
        return;
    }

    const QStringList languageList = layoutsMgr.languageList();
    const QString nextLanguage = languageList.at(languageIndex);

    qDebug() << "\t" << currentLanguage << " -> " << nextLanguage << " index=" << languageIndex;

    if (nextLanguage != currentLanguage) {
        currentLanguage = nextLanguage;

        emit languageChanged(currentLanguage);

        // Switcher has the language loaded, just switchTo() it.
        // NOTE: Switcher already has correct index if language change was
        // initiated by a flick gesture.
        if (mainKeyboardSwitcher->count() >= languageIndex) {
            mainKeyboardSwitcher->switchTo(languageIndex);
        }
    }
}


void MVirtualKeyboard::languageReset()
{
    int languageIndex = -1; // Language to apply after reload.

    if (layoutsMgr.languageCount() > 0) {
        const QStringList languageList = layoutsMgr.languageList();

        // If new language set does not contain previous current language
        // we need to set it to something else.
        languageIndex = languageList.indexOf(currentLanguage);

        if (languageIndex == -1) {
            // Current language not in new languagelist.
            // Set current language to default language
            const QString defaultLanguage = layoutsMgr.defaultLanguage();

            languageIndex = languageList.indexOf(defaultLanguage);
            if (languageIndex == -1) {
                // Last resort, take first
                languageIndex = 0;
            }
        }
    }

    recreateKeyboards();

    if (languageIndex >= 0) {
        setLanguage(languageIndex);
    }
}


void MVirtualKeyboard::onSectionSwitchStarting(int current, int next)
{
    if (mainKeyboardSwitcher->currentWidget()) {
        // Current widget is animated off the screen but if mouse is not moved
        // relative to screen it appears to KeyButtonArea as if mouse was held
        // still. We explicitly call ungrabMouse() to prevent long press event
        // in KeyButtonArea. Visible effect of not doing this is would be
        // appearing popup, for example.
        mainKeyboardSwitcher->currentWidget()->ungrabMouse();
    }

    if ((current != -1) && (currentLayoutType == LayoutData::General)) {
        notification->displayText(layoutsMgr.keyboardTitle(layoutsMgr.languageList()[next]));
    }
}


void MVirtualKeyboard::onSectionSwitched(QGraphicsWidget *previous, QGraphicsWidget *current)
{
    // Note: only does changes on the main sections.
    KeyButtonArea *currentView = static_cast<KeyButtonArea *>(current->layout()->itemAt(0));

    if (previous) {
        const KeyButtonArea *previousView
            = static_cast<const KeyButtonArea *>(previous->layout()->itemAt(0));

        if (previousView->isAccurateMode()) {
            currentView->accurateStart();
        } else {
            currentView->accurateStop();
        }
    }

    // All sections may not be of the same height so we send a region just in
    // case.  That also causes reaction maps to be redrawn.
    sendVKBRegion();
}


void MVirtualKeyboard::createSwitcher()
{
    delete mainKeyboardSwitcher; // Delete previous views
    mainKeyboardSwitcher = new HorizontalSwitcher(this);
    mainKeyboardSwitcher->setLooping(true);

    // In addition to animating sections (KeyButtonAreas), we also
    // use switcher to provide us the moving direction logic.
    // These are the signals we track to update our state.
    connect(mainKeyboardSwitcher, SIGNAL(switchStarting(int, int)),
            this, SLOT(onSectionSwitchStarting(int, int)));
    connect(mainKeyboardSwitcher, SIGNAL(switchDone(QGraphicsWidget *, QGraphicsWidget *)),
            this, SLOT(onSectionSwitched(QGraphicsWidget *, QGraphicsWidget *)));
}


void MVirtualKeyboard::reloadSwitcherContent()
{
    // delete previous pages
    mainKeyboardSwitcher->deleteAll();

    // Load certain type and orientation from all languages.
    foreach (const QString &language, layoutsMgr.languageList()) {
        // create a new page for language and append main and function sections
        QGraphicsWidget *languagePage = new QGraphicsWidget;
        QGraphicsLinearLayout *languageLayout = createKeyAreaLayout(languagePage);

        KeyButtonArea *mainSection = createMainSectionView(language, LayoutData::General,
                                                           currentOrientation);
        mainSection->setObjectName("VirtualKeyboardMainRow");

        KeyButtonArea *functionRow = createSectionView(language, LayoutData::General,
                                                       currentOrientation,
                                                       LayoutData::functionkeySection,
                                                       KeyButtonArea::ButtonSizeFunctionRow,
                                                       false, languagePage);
        functionRow->setObjectName("VirtualKeyboardFunctionRow");

        if (mainSection == 0 || functionRow == 0) {
            qCritical("Problem loading language to switcher");
            delete mainSection;
            delete functionRow;
            delete languagePage;
            continue;
        }

        languageLayout->addItem(mainSection);
        languageLayout->addItem(functionRow);

        // add page to main switcher
        mainKeyboardSwitcher->addWidget(languagePage);
    }
}


KeyButtonArea *MVirtualKeyboard::createMainSectionView(const QString &language,
                                                       LayoutData::LayoutType layoutType,
                                                       M::Orientation orientation,
                                                       QGraphicsWidget *parent)
{
    KeyButtonArea *buttonArea = createSectionView(language, layoutType, orientation,
                                                  LayoutData::mainSection,
                                                  KeyButtonArea::ButtonSizeEqualExpanding,
                                                  true, parent);

    // horizontal flick handling only on main section of qwerty
    connect(buttonArea, SIGNAL(flickLeft()), this, SLOT(flickLeftHandler()));
    connect(buttonArea, SIGNAL(flickRight()), this, SLOT(flickRightHandler()));

    return buttonArea;
}

KeyButtonArea * MVirtualKeyboard::createSectionView(const QString &language,
                                                    LayoutData::LayoutType layoutType,
                                                    M::Orientation orientation,
                                                    const QString &section,
                                                    KeyButtonArea::ButtonSizeScheme sizeScheme,
                                                    bool usePopup,
                                                    QGraphicsWidget *parent)
{
    const LayoutData *model = layoutsMgr.layout(language, layoutType, orientation);

    if (model == NULL) {
        return NULL;
    }

    QSharedPointer<const LayoutSection> layoutSection = model->section(section);
    Q_ASSERT(!layoutSection.isNull());

    KeyButtonArea *view = new SingleWidgetButtonArea(styleContainer, layoutSection,
                                                     sizeScheme, usePopup, parent);
    Q_ASSERT(view);

    connect(view, SIGNAL(keyPressed(const KeyEvent &)),
            this, SIGNAL(keyPressed(const KeyEvent &)));

    connect(view, SIGNAL(keyReleased(const KeyEvent &)),
            this, SIGNAL(keyReleased(const KeyEvent &)));

    connect(view, SIGNAL(keyClicked(const KeyEvent &)),
            this, SIGNAL(keyClicked(const KeyEvent &)));

    connect(view, SIGNAL(flickDown()), this, SLOT(hideKeyboard()));
    connect(view, SIGNAL(flickDown()), this, SIGNAL(userInitiatedHide()));
    connect(view, SIGNAL(flickUp(const KeyBinding *)),
            this, SLOT(flickUpHandler(const KeyBinding *)));

    return view;
}


void MVirtualKeyboard::setCopyPasteButton(bool copyAvailable, bool pasteAvailable)
{
    imToolbar->setCopyPasteButton(copyAvailable, pasteAvailable);
    organizeContent(currentOrientation);
    if (isVisible()) {
        sendVKBRegion();
    }
}

void MVirtualKeyboard::setSelectionStatus(bool hasSelection)
{
    imToolbar->setSelectionStatus(hasSelection);
}

void MVirtualKeyboard::recreateKeyboards()
{
    reloadSwitcherContent(); // main keyboards
    recreateSpecialKeyboards(); // numbers
}


void MVirtualKeyboard::recreateSpecialKeyboards()
{
    // delete old number keyboards
    for (int i = numberLayout->count(); i > 0; --i) {
        delete numberLayout->itemAt(i - 1);
    }

    for (int i = phoneNumberLayout->count(); i > 0; --i) {
        delete phoneNumberLayout->itemAt(i - 1);
    }

    // number:
    const QString defaultLanguage = layoutsMgr.defaultLanguage();

    KeyButtonArea *numberArea = createSectionView(defaultLanguage, LayoutData::Number,
                                                  currentOrientation, LayoutData::mainSection,
                                                  KeyButtonArea::ButtonSizeEqualExpanding,
                                                  false, numberKeyboard);
    numberArea->setObjectName("VirtualKeyboardNumberMainRow");

    KeyButtonArea *numberFunctionRow
        = createSectionView(defaultLanguage, LayoutData::Number, currentOrientation,
                            LayoutData::functionkeySection, KeyButtonArea::ButtonSizeFunctionRowNumber,
                            false, numberKeyboard);
    numberFunctionRow->setObjectName("VirtualKeyboardNumberFunctionRow");

    numberLayout->addItem(numberArea);
    numberLayout->addItem(numberFunctionRow);

    // phone:
    KeyButtonArea *phoneArea = createSectionView(defaultLanguage, LayoutData::PhoneNumber,
                                                 currentOrientation, LayoutData::mainSection,
                                                 KeyButtonArea::ButtonSizeEqualExpandingPhoneNumber,
                                                 false, phoneNumberKeyboard);
    phoneArea->setObjectName("VirtualKeyboardPhoneMainRow");

    KeyButtonArea *phoneFunctionArea
        = createSectionView(defaultLanguage, LayoutData::PhoneNumber, currentOrientation,
                            LayoutData::functionkeySection, KeyButtonArea::ButtonSizeFunctionRowNumber,
                            false, phoneNumberKeyboard);
    phoneFunctionArea->setObjectName("VirtualKeyboardPhoneFunctionRow");

    phoneNumberLayout->addItem(phoneArea);
    phoneNumberLayout->addItem(phoneFunctionArea);

    // sanity check. Don't allow load failure for these
    if (numberArea == 0 || numberFunctionRow == 0 || phoneArea == 0 || phoneFunctionArea == 0) {
        qFatal("Error loading number keyboard");
    }
}

void MVirtualKeyboard::showToolbar()
{
    imToolbar->show();
}

void MVirtualKeyboard::hideToolbar()
{
    imToolbar->hide();
}

bool MVirtualKeyboard::symViewAvailable() const
{
    bool available = true;
    switch (currentLayoutType) {
    case LayoutData::Number:
    case LayoutData::PhoneNumber:
        available = false;
        break;
    default:
        break;
    }
    return available;
}

void MVirtualKeyboard::setIndicatorButtonState(Qt::KeyboardModifier modifier, ModifierState state)
{
    imToolbar->setIndicatorButtonState(modifier, state);
}


void MVirtualKeyboard::hideMainArea()
{
    QGraphicsItem *item = dynamic_cast<QGraphicsItem*>(mainLayout->itemAt(1));
    if (item) {
        item->hide();
    }
}


void MVirtualKeyboard::showMainArea()
{
    QGraphicsItem *item = dynamic_cast<QGraphicsItem*>(mainLayout->itemAt(1));
    if (item) {
        item->show();
    }
}

