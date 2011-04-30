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

#include "extendedkeys.h"
#include "magnifierhost.h"
#include "mkeyboardmagnifierstyle.h"
#include "reactionmapwrapper.h"

#include <reactionmappainter.h>
#include <regiontracker.h>
#include <layoutdata.h>
#include <MScene>
#include <QPainter>
#include <QBrush>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QMargins>
#include <MWidgetCreator>
#include <MFeedback>
#include <MCancelEvent>

namespace {
    class MainAreaReset
        : public MImAbstractKeyVisitor
    {
    private:
        bool mHasCapsLocked;

    public:
        explicit MainAreaReset()
            : mHasCapsLocked(false)
        {}

        bool operator()(MImAbstractKey *key)
        {
            if (!key) {
                return false;
            }

            if (key->isShiftKey()
                && (key->state() == MImAbstractKey::Selected)) {
                // OK, in caps-locked mode. But don't reset shift key:
                mHasCapsLocked = true;
            } else {
                key->resetTouchPointCount();
            }

            return false;
        }

        bool hasCapsLocked() const
        {
            return mHasCapsLocked;
        }
    };

    QSize queryKeyAreaSize(const MImAbstractKeyArea *keyArea,
                           int keyCount)
    {
        const MImAbstractKeyAreaStyleContainer &style(keyArea->baseStyle());

        return QSize(style->keyWidthMediumFixed() * keyCount
                     + style->keyMarginLeft() * (keyCount - 1)
                     + style->keyMarginRight() * (keyCount - 1)
                     + style->paddingLeft()
                     + style->paddingRight(),
                     keyArea->size().height());
    }
}

ExtendedKeys::ExtendedKeys(MagnifierHost *newHost,
                           MImAbstractKeyArea *newMainArea)
    : MImOverlay()
    , ReactionMapPaintable()
    , host(newHost)
    , mainArea(newMainArea)
    , hideOnNextMouseRelease(false)
{
    setObjectName("ExtendedKeys"); // needed by MATTI (but useful otherwise too)
    RegionTracker::instance().addRegion(*this);

    setFlags(QGraphicsItem::ItemHasNoContents);
    setParent(newHost);
}

ExtendedKeys::~ExtendedKeys()
{}

bool ExtendedKeys::isPaintable() const
{
    return isVisible();
}

bool ExtendedKeys::isFullScreen() const
{
    // It acts as fullscreen
    return true;
}

void ExtendedKeys::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view)
{
    // Cast is needed for MImAbstractKeyArea * caused by protected reimplementation
    static_cast<MImAbstractKeyArea *>(extKeysArea.get())->drawReactiveAreas(reactionMap, view);
}

void ExtendedKeys::showExtendedArea(const QPointF &origin,
                                    const QPointF &tappedScenePos,
                                    const QString &labels)
{
    LayoutData::SharedLayoutSection section(new LayoutSection(labels));

    // Custom haptic feedback for this popup being appeared
    MFeedback::play("priority2_vkb_popup_press");
    // TODO: disable swipe gestures from extended keys area
    extKeysArea.reset(ExtendedKeysArea::create(section, this));
    extKeysArea->setStyleName("ExtendedKeys");

    // Send clicked signal as if it was from mainArea.
    connect(extKeysArea.get(), SIGNAL(keyClicked(const MImAbstractKey *, const KeyContext &)),
            mainArea,          SIGNAL(keyClicked(const MImAbstractKey *, const KeyContext &)));

    connect(extKeysArea.get(), SIGNAL(keyClicked(const MImAbstractKey *, const KeyContext &)),
            host,              SLOT(hide()));

    connect(extKeysArea.get(), SIGNAL(displayExited()),
            host,              SLOT(hide()));

    extKeysArea->resize(queryKeyAreaSize(extKeysArea.get(), labels.count()));
    extKeysArea->setParentItem(this);

    // Install scene event filter to detect keyarea mouse release. If first
    // release didn't end up emitting a keyClicked() signal, this popup will
    // be hidden always on the second release.
    extKeysArea->installSceneEventFilter(this); // After adding to same scene.
    hideOnNextMouseRelease = false;

    // Through the main area, we can access the VKB coordinate system:
    const QPointF originMapped(mainArea->mapToItem(this, origin));
    const QPointF extKeysPos(originMapped
                             - extKeysArea->boundingRect().center()
                             - QPointF(0, host->style()->extendedKeysOffset()));

    const MImAbstractKeyAreaStyleContainer &style(mainArea->baseStyle());
    MagnifierHost::applyConstrainedPosition(extKeysArea.get(), extKeysArea->parentItem(), extKeysPos,
                                            QMargins(style->paddingLeft(), style->paddingTop(),
                                                     style->paddingRight(), style->paddingBottom()));

    MainAreaReset reset;
    MImAbstractKey::visitActiveKeys(&reset);
    mainArea->modifiersChanged(reset.hasCapsLocked());
    show();

    // ExtendedKeys is shown on long-press. Therefore we will give mouse grab to the
    // new extKeysArea. Because mainArea (MImAbstractKeyArea) already resets itself
    // once it loses mouse grab, we are not required to send MCancelEvent to it to
    // tell that it will never get release.

    // FIXME: yet we have to send cancel event because of bug NB#248227.
    // It is sent because we know it will prevent further touch events from
    // being handled.
    MCancelEvent cancel;
    mainArea->scene()->sendEvent(mainArea, &cancel);

    // Grab will be removed when widget is hidden.
    extKeysArea->grabMouse();

    QGraphicsSceneMouseEvent press(QEvent::GraphicsSceneMousePress);
    press.setPos(extKeysArea->mapFromScene(tappedScenePos));
    press.setLastPos(press.pos());
    press.setScenePos(tappedScenePos);
    press.setLastScenePos(press.scenePos());
    scene()->sendEvent(extKeysArea.get(), &press);

    // Update the reaction maps right now
    signalForwarder.emitRequestRepaint();
    // Update the reaction maps if the popup disappears
    connect(extKeysArea.get(), SIGNAL(displayExited()),
            &signalForwarder, SIGNAL(requestRepaint()));
}

QVariant ExtendedKeys::itemChange(GraphicsItemChange change,
                                  const QVariant &value)
{
    if (change == QGraphicsItem::ItemVisibleChange
        && !value.toBool()) {
        extKeysArea->removeSceneEventFilter(this);
    }
    return MImOverlay::itemChange(change, value);
}

bool ExtendedKeys::sceneEventFilter(QGraphicsItem *, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneMouseRelease) {
        if (hideOnNextMouseRelease) {
            hideOnNextMouseRelease = false;
            // Hide after the release is processed.
            QMetaObject::invokeMethod(host, "hide", Qt::QueuedConnection);
        } else {
            hideOnNextMouseRelease = true;
        }
    }
    return false; // Allow event to reach extKeysArea.
}
