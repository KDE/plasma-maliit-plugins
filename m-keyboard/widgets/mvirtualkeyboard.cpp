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



#include "mvirtualkeyboard.h"
#include "mvirtualkeyboardstyle.h"
#include "horizontalswitcher.h"
#include "keyboarddata.h"
#include "layoutdata.h"
#include "layoutsmanager.h"
#include "notification.h"
#include "vkbdatakey.h"
#include "mimtoolbar.h"
#include "ikeybutton.h"
#include "keyevent.h"
#include "keyeventhandler.h"
#include "grip.h"
#include "sharedhandlearea.h"

#include <mtoolbardata.h>

#include <QDebug>
#include <QPainter>
#include <QGraphicsLinearLayout>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsWidget>

#include <MButton>
#include <MScalableImage>
#include <MSceneManager>
#include <mreactionmap.h>
#include <mtimestamp.h>
#include <mplainwindow.h>
#include <MApplication>


namespace
{
    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";
}

const QString MVirtualKeyboard::WordSeparators("-.,!? \n");


MVirtualKeyboard::MVirtualKeyboard(const LayoutsManager &layoutsManager,
                                   const MVirtualKeyboardStyleContainer *styleContainer,
                                   QGraphicsWidget *parent)
    : MWidget(parent),
      mainLayout(new QGraphicsLinearLayout(Qt::Vertical, this)),
      currentLevel(0),
      numLevels(2),
      activity(Inactive),
      styleContainer(styleContainer),
      sceneManager(MPlainWindow::instance()->sceneManager()),
      shiftLevel(ModifierClearState),
      currentLayoutType(LayoutData::General),
      currentOrientation(sceneManager->orientation()),
      hideShowByFadingOnly(false),
      sendRegionUpdates(true),
      regionUpdateRequested(false),
      layoutsMgr(layoutsManager),
      mainKeyboardSwitcher(0),
      notification(0),
      numberKeyboard(0),
      numberLayout(0),
      phoneNumberKeyboard(0),
      phoneNumberLayout(0),
      activeState(OnScreen)
{
    setObjectName("MVirtualKeyboard");
    hide();

    notification = new Notification(styleContainer, this);

    eventHandler = new KeyEventHandler(this);
    connect(eventHandler, SIGNAL(keyPressed(const KeyEvent &)),
            this, SIGNAL(keyPressed(const KeyEvent &)));
    connect(eventHandler, SIGNAL(keyReleased(const KeyEvent &)),
            this, SIGNAL(keyReleased(const KeyEvent &)));
    connect(eventHandler, SIGNAL(keyClicked(const KeyEvent &)),
            this, SIGNAL(keyClicked(const KeyEvent &)));
    connect(eventHandler, SIGNAL(shiftPressed(bool)),
            this, SLOT(setFunctionRowState(bool)));

    enableMultiTouch = MGConfItem(MultitouchSettings).value().toBool();

    createSwitcher();

    setupTimeLine();

    // setting maximum width explicitly. otherwise max width of children will override. (bug?)
    setMaximumWidth(QWIDGETSIZE_MAX);
    setMinimumWidth(0);

    // add stuff to the layout

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    Grip &keyboardGrip = *new Grip(this);
    keyboardGrip.setZValue(-1); // popup should be on top of keyboardGrip
    keyboardGrip.setObjectName("KeyboardHandle");
    mainLayout->addItem(&keyboardGrip);
    connectHandle(keyboardGrip);

    mainLayout->addItem(mainKeyboardSwitcher); // start in qwerty

    // create main widgets for number and phone number, and make the initial button areas
    numberKeyboard = new QGraphicsWidget(this);
    numberKeyboard->hide();
    numberLayout = createKeyAreaLayout(numberKeyboard);

    phoneNumberKeyboard = new QGraphicsWidget(this);
    phoneNumberKeyboard->hide();
    phoneNumberLayout = createKeyAreaLayout(phoneNumberKeyboard);

    connect(&layoutsMgr, SIGNAL(languagesChanged()), this, SLOT(languageReset()));
    connect(&layoutsMgr, SIGNAL(numberFormatChanged()), this, SLOT(numberKeyboardReset()));
    languageReset(); // creates keyboard widgets

    organizeContent(currentOrientation);
}


MVirtualKeyboard::~MVirtualKeyboard()
{
    delete mainKeyboardSwitcher;
    mainKeyboardSwitcher = 0;

    delete notification;
    notification = 0;
}


template <class T>
void MVirtualKeyboard::connectHandle(const T &handle)
{
    connect(&handle, SIGNAL(flickLeft(const FlickGesture &)), this, SLOT(flickLeftHandler()));
    connect(&handle, SIGNAL(flickRight(const FlickGesture &)), this, SLOT(flickRightHandler()));
    connect(&handle, SIGNAL(flickDown(const FlickGesture &)), this, SLOT(handleHandleFlickDown(const FlickGesture &)));
}


void MVirtualKeyboard::handleHandleFlickDown(const FlickGesture &/* gesture */)
{
    if (activeState == OnScreen) {
        hideKeyboard();
        emit userInitiatedHide();
    }
}


void
MVirtualKeyboard::prepareToOrientationChange()
{
    // if inactive, just ignore
    if (activity != Active) {
        return;
    }
    hideKeyboard(true, true);
}


void
MVirtualKeyboard::finalizeOrientationChange()
{
    // if inactive, just ignore
    if (activity != TemporarilyInactive) {
        return;
    }

    showKeyboard(true);
}

void
MVirtualKeyboard::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    mTimestamp("MVirtualKeyboard", "start");

    // The second item in layout holds the currently used keyboard. Draw under it.
    const QRectF backgroundGeometry = layout()->itemAt(KeyboardIndex)->geometry();

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
    case ModifierClearState:
        currentLevel = 0;
        break;
    case ModifierLatchedState:
        currentLevel = 1;
        break;
    case ModifierLockedState:
        currentLevel = 1;
        break;
    }

    for (int i = 0; i < mainKeyboardSwitcher->count(); ++i) {
        // the subwidgets have main section as first item in their layout, function row as second.
        // handling main section:
        static_cast<KeyButtonArea *>(mainKeyboardSwitcher->widget(i)->layout()->itemAt(0))->
                switchLevel(currentLevel);

        // Function row shift update, level does not change for the layout.
        static_cast<KeyButtonArea *>(mainKeyboardSwitcher->widget(i)->layout()->itemAt(1))->
            setShiftState(shiftLevel);

        if (!enableMultiTouch) {
            static_cast<KeyButtonArea *>(mainKeyboardSwitcher->widget(i)->layout()->itemAt(1))->
                switchLevel(currentLevel);
        }
    }
}


void
MVirtualKeyboard::setShiftState(ModifierState level)
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
    if ((activeState == OnScreen) && !mainKeyboardSwitcher->isRunning()) {
        if (mainKeyboardSwitcher->isAtBoundary(HorizontalSwitcher::Right)) {
            emit pluginSwitchRequired(M::SwitchForward);
            return;
        }

        mainKeyboardSwitcher->switchTo(HorizontalSwitcher::Right);
        setLanguage(mainKeyboardSwitcher->current());
    }
}


void
MVirtualKeyboard::flickUpHandler(const KeyBinding &binding)
{
    if (binding.action() == KeyBinding::ActionSym) {
        emit showSymbolViewRequested();
    }
}


void
MVirtualKeyboard::flickRightHandler()
{
    if ((activeState == OnScreen) && !mainKeyboardSwitcher->isRunning()) {
        if (mainKeyboardSwitcher->isAtBoundary(HorizontalSwitcher::Left)) {
            emit pluginSwitchRequired(M::SwitchBackward);
            return;
        }

        mainKeyboardSwitcher->switchTo(HorizontalSwitcher::Left);
        setLanguage(mainKeyboardSwitcher->current());
    }
}


void MVirtualKeyboard::showKeyboard(bool fadeOnly)
{
    qDebug() << __PRETTY_FUNCTION__ << fadeOnly;
    organizeContent(sceneManager->orientation());

    if (activity == Active) {
        // already showing up
        return;
    }

    activity = Active;

    showHideTimeline.setDirection(QTimeLine::Forward);

    if (showHideTimeline.state() != QTimeLine::Running) {
        hideShowByFadingOnly = fadeOnly;

        QPoint regionOffset(0, 0);
        if (!fadeOnly) {
            // sharedHandleArea is positioned right below the visible area
            // vkb is positioned below sharedHandleArea
            int shift = 0;

            if (sharedHandleArea) {
                shift = sharedHandleArea->size().height();
            }
            setPos(0, sceneManager->visibleSceneSize().height() + shift);
            regionOffset.setY(-actualHeight() - shift);
        } else {
            setPos(0, sceneManager->visibleSceneSize().height() - actualHeight());
        }

        suppressRegionUpdate(true); // Don't send separate region update for imtoolbar.
        showHideTimeline.start();
        show();

        // We need to send one initial region update to have passthruwindow make the
        // window visible.  We also want the scene manager to move the underlying window
        // so that the focused widget will be visible after the show animation and we want
        // it to do that right away, so the region we send now is (usually) the final
        // region (i.e. what it will be after animation is finished).  Region will be sent
        // additionally after animation is finished just in case copy button appears or
        // something else changes during the animation.  But normally the region should be
        // the same as the one we send now.  We do this initial region sending after
        // show() so that we can use region().  We cannot use sendVKBRegion() since region
        // updates are suppressed and because we want to apply the offset.
        regionOffset = mapOffsetToScene(regionOffset);
        emit regionUpdated(region(true).translated(regionOffset));
        emit inputMethodAreaUpdated(region(true).translated(regionOffset));
    } else if (hideShowByFadingOnly) {
        // fade() doesn't alter the position when we're just fading
        setPos(0, sceneManager->visibleSceneSize().height() - actualHeight());
        show();
    }
}


void MVirtualKeyboard::hideKeyboard(bool fadeOnly, bool temporary)
{
    if (scene() && scene()->views().count() > 0) {
        MReactionMap *reactionMap = MReactionMap::instance(scene()->views()[0]);
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
    setShiftState(ModifierClearState);

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
        result = 0;
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
        mainKeyboardSwitcher->setPreferredWidth(MPlainWindow::instance()->visibleSceneSize().width());

        // invalidate layouts so we get updated size infos
        numberKeyboard->layout()->invalidate();
        phoneNumberKeyboard->layout()->invalidate();
    }

    mainLayout->invalidate();
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
        int shift = 0;

        if (sharedHandleArea) {
            shift = sharedHandleArea->size().height();
        }
        //vkb should be under sharedHandleArea
        setPos(0, sceneSize.height() + shift - (opacity * (actualHeight() + shift)));
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


void MVirtualKeyboard::organizeContentAndSendRegion()
{
    if (isVisible()) {
        organizeContent(currentOrientation);
        sendVKBRegion();
    }
}


void MVirtualKeyboard::sendVKBRegion()
{
    regionUpdateRequested = true;

    if (!sendRegionUpdates)
        return;

    emit regionUpdated(region(true));
    emit inputMethodAreaUpdated(region(true));
}

QRegion MVirtualKeyboard::region(const bool notJustMainKeyboardArea) const
{
    QRegion region;
    QRectF rect;

    if (isVisible()) {
        mainLayout->activate();

        // Main keyboard area (qwerty/number/etc.)
        if (activeState == OnScreen) {
            mainLayout->activate();
            rect = mainLayout->itemAt(KeyboardIndex)->geometry();
            region |= mapRectToScene(rect).toRect();
            if (notJustMainKeyboardArea) {
                rect = mainLayout->itemAt(KeyboardHandleIndex)->geometry();
                region |= mapRectToScene(rect).toRect();
            }
        }
    }

    return region;
}

QPoint MVirtualKeyboard::mapOffsetToScene(QPointF offset)
{
    QPointF startingPoint(mapToScene(QPointF(0, 0)));

    offset = mapToScene(offset);

    return QPoint(offset.x() - startingPoint.x(),
                  offset.y() - startingPoint.y());
}


QRect MVirtualKeyboard::mainAreaSceneRect() const
{
    QRect result;

    if (activeState == OnScreen) {
        result = region(false).boundingRect();
    }

    return result;
}

void MVirtualKeyboard::setKeyboardState(MIMHandlerState newState)
{
    if (activeState == newState) {
        return;
    }

    activeState = newState;
    resetState();

    static_cast<QGraphicsWidget *>(mainLayout->itemAt(KeyboardIndex))->setVisible(newState == OnScreen);
    organizeContentAndSendRegion();
}

MIMHandlerState MVirtualKeyboard::keyboardState() const
{
    return activeState;
}

void MVirtualKeyboard::drawButtonsReactionMaps(MReactionMap *reactionMap, QGraphicsView *view)
{
    // Depending on which keyboard type is currently shown
    // we must pick the correct KeyButtonArea(s).

    // Keyboard layout consists of one or more KeyButtonAreas,
    // such as qwerty + function row.
    QGraphicsLayout *keyboardLayout = 0;

    QGraphicsLayoutItem *item = mainLayout->itemAt(KeyboardIndex);
    if (item == mainKeyboardSwitcher && mainKeyboardSwitcher->currentWidget()) {
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

QString MVirtualKeyboard::layoutTitle() const
{
    return layoutsMgr.keyboardTitle(currentLanguage);
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
        MReactionMap *reactionMap = MReactionMap::instance(view);
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
        reactionMap->fillRectangle(layout()->itemAt(KeyboardIndex)->geometry());
        reactionMap->fillRectangle(layout()->itemAt(KeyboardHandleIndex)->geometry());

        if (activeState == OnScreen) {
            drawButtonsReactionMaps(reactionMap, view);
        }
    }
}


const MVirtualKeyboardStyleContainer &MVirtualKeyboard::style() const
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
    QGraphicsLayoutItem *previousItem = mainLayout->itemAt(KeyboardIndex);
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

    mainLayout->insertItem(KeyboardIndex, newWidget);
    if (activeState == OnScreen) {
        newWidget->show();
    } else {
        newWidget->hide();
    }

    // resize and update keyboards if needed
    organizeContentAndSendRegion();
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


ModifierState MVirtualKeyboard::shiftStatus() const
{
    return shiftLevel;
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
            mainKeyboardSwitcher->setCurrent(languageIndex);
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

        // TODO: we should simplify the whole languageReset(). Now
        // MVirtualKeyboard manages currentLanguage and HorizontalSwitcher also
        // manages its current widget index.  We always have to synchronize
        // both of them if one is changed (in the right order, too).
        currentLanguage = "";
    }

    recreateKeyboards();

    if (languageIndex >= 0) {
        setLanguage(languageIndex);
    }
}

void MVirtualKeyboard::numberKeyboardReset()
{
    recreateSpecialKeyboards(); // number and phone number keyboard
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
    organizeContentAndSendRegion();
}


void MVirtualKeyboard::createSwitcher()
{
    delete mainKeyboardSwitcher; // Delete previous views
    mainKeyboardSwitcher = new HorizontalSwitcher(this);
    mainKeyboardSwitcher->setLooping(true);
    mainKeyboardSwitcher->setPreferredWidth(MPlainWindow::instance()->visibleSceneSize().width());

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
        mainSection->setPreferredWidth(MPlainWindow::instance()->visibleSceneSize().width());

        KeyButtonArea *functionRow = createSectionView(language, LayoutData::General,
                                                       currentOrientation,
                                                       LayoutData::functionkeySection,
                                                       KeyButtonArea::ButtonSizeFunctionRow,
                                                       false, languagePage);

        if (mainSection == 0 || functionRow == 0) {
            qCritical("Problem loading language to switcher");
            delete mainSection;
            delete functionRow;
            delete languagePage;
            continue;
        }

        mainSection->setObjectName("VirtualKeyboardMainRow");
        functionRow->setObjectName("VirtualKeyboardFunctionRow");

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

    eventHandler->addEventSource(view);

    connect(view, SIGNAL(flickDown()), this, SLOT(hideKeyboard()));
    connect(view, SIGNAL(flickDown()), this, SIGNAL(userInitiatedHide()));
    connect(view, SIGNAL(flickUp(KeyBinding)),
            this, SLOT(flickUpHandler(KeyBinding)));

    return view;
}

void MVirtualKeyboard::setFunctionRowState(bool shiftPressed)
{
    if (enableMultiTouch) {
        //TODO: time treshold to avoid flickering?
        static_cast<KeyButtonArea *>(mainKeyboardSwitcher->currentWidget()->layout()->itemAt(1))->
            switchLevel(shiftPressed ? 1 : 0);
    }
}

void MVirtualKeyboard::recreateKeyboards()
{
    reloadSwitcherContent(); // main keyboards
    recreateSpecialKeyboards(); // number and phone number keyboard
    switchLevel(); // update shift level in recreated keyboards
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

    KeyButtonArea *numberFunctionRow
        = createSectionView(defaultLanguage, LayoutData::Number, currentOrientation,
                            LayoutData::functionkeySection, KeyButtonArea::ButtonSizeFunctionRowNumber,
                            false, numberKeyboard);

    numberLayout->addItem(numberArea);
    numberLayout->addItem(numberFunctionRow);

    // phone:
    KeyButtonArea *phoneArea = createSectionView(defaultLanguage, LayoutData::PhoneNumber,
                                                 currentOrientation, LayoutData::mainSection,
                                                 KeyButtonArea::ButtonSizeEqualExpandingPhoneNumber,
                                                 false, phoneNumberKeyboard);

    KeyButtonArea *phoneFunctionArea
        = createSectionView(defaultLanguage, LayoutData::PhoneNumber, currentOrientation,
                            LayoutData::functionkeySection, KeyButtonArea::ButtonSizeFunctionRowNumber,
                            false, phoneNumberKeyboard);

    phoneNumberLayout->addItem(phoneArea);
    phoneNumberLayout->addItem(phoneFunctionArea);

    // sanity check. Don't allow load failure for these
    if (numberArea == 0 || numberFunctionRow == 0 || phoneArea == 0 || phoneFunctionArea == 0) {
        qFatal("Error loading number keyboard");
    }

    numberArea->setObjectName("VirtualKeyboardNumberMainRow");
    numberFunctionRow->setObjectName("VirtualKeyboardNumberFunctionRow");
    phoneArea->setObjectName("VirtualKeyboardPhoneMainRow");
    phoneFunctionArea->setObjectName("VirtualKeyboardPhoneFunctionRow");
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

void MVirtualKeyboard::setSharedHandleArea(const QPointer<SharedHandleArea> &newSharedHandleArea)
{
    sharedHandleArea = newSharedHandleArea;
}


void MVirtualKeyboard::switchLanguage(M::InputMethodSwitchDirection direction, bool enableAnimation)
{
    qDebug() << __PRETTY_FUNCTION__ << direction << enableAnimation;
    if (direction == M::SwitchUndefined) {
        return;
    }

    if (enableAnimation) {
        if (direction == M::SwitchForward) {
            mainKeyboardSwitcher->switchTo(HorizontalSwitcher::Right);
        } else {
            mainKeyboardSwitcher->switchTo(HorizontalSwitcher::Left);
        }
    } else {
        int current = mainKeyboardSwitcher->current();
        if (direction == M::SwitchForward) {
            current = (current + 1) % mainKeyboardSwitcher->count();
        } else {
            --current;
            if (current < 0) {
                current = mainKeyboardSwitcher->count() - 1;
            }
        }

        mainKeyboardSwitcher->setCurrent(current);
    }
    setLanguage(mainKeyboardSwitcher->current());
}


void MVirtualKeyboard::hideMainArea()
{
    QGraphicsItem *item = dynamic_cast<QGraphicsItem*>(mainLayout->itemAt(KeyboardIndex));
    if (item) {
        item->hide();
    }
}


void MVirtualKeyboard::showMainArea()
{
    QGraphicsItem *item = dynamic_cast<QGraphicsItem*>(mainLayout->itemAt(KeyboardIndex));
    if (item) {
        item->show();
    }
}

void MVirtualKeyboard::setInputMethodMode(M::InputMethodMode mode)
{
    KeyButtonArea::setInputMethodMode(mode);
}
