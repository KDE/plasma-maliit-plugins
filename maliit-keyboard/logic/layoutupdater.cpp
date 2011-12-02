/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
 *
 */

#include "layoutupdater.h"

namespace MaliitKeyboard {

namespace {

enum Transform {
    TransformToUpper,
    TransformToLower
};

KeyArea transformKeyArea(const KeyArea &ka,
                         Transform t)
{
    KeyArea new_ka;
    new_ka.setRect(ka.rect());

    foreach (Key key, ka.keys()) {
        KeyLabel label(key.label());

        switch (t) {
        case TransformToUpper:
            label.setText(label.text().toUpper());
            break;

        case TransformToLower:
            label.setText(label.text().toLower());
            break;
        }

        key.setLabel(label);
        new_ka.appendKey(key);
    }

    return new_ka;
}

KeyArea replaceKey(const KeyArea &ka,
                   const Key &replace)
{
    KeyArea new_ka;
    new_ka.setRect(ka.rect());

    foreach (const Key &key, ka.keys()) {
        new_ka.appendKey((key.label().text() == replace.label().text()) ? replace : key);
    }

    return new_ka;
}

}

class LayoutUpdaterPrivate
{
public:
    // TODO: who takes ownership of the states?
    struct States {
        QState *no_shift;
        QState *shift;
        QState *latched_shift;
        QState *caps_lock;

        explicit States()
            : no_shift(0)
            , shift(0)
            , latched_shift(0)
            , caps_lock(0)
        {}
    };

    SharedLayout layout;
    QScopedPointer<KeyboardLoader> loader;
    QScopedPointer<QStateMachine> machine;
    States states;
};

LayoutUpdater::LayoutUpdater(QObject *parent)
    : QObject(parent)
    , d_ptr(new LayoutUpdaterPrivate)
{}

LayoutUpdater::~LayoutUpdater()
{}

void LayoutUpdater::init()
{
    Q_D(LayoutUpdater);

    d->machine.reset(new QStateMachine);
    d->machine->setChildMode(QState::ExclusiveStates);

    LayoutUpdaterPrivate::States &s(d->states);
    s.no_shift = new QState;
    s.no_shift->setObjectName("no-shift");

    s.shift = new QState;
    s.shift->setObjectName("shift");

    s.latched_shift = new QState;
    s.latched_shift->setObjectName("latched-shift");

    s.caps_lock = new QState;
    s.caps_lock->setObjectName("caps-lock");

    s.no_shift->addTransition(this, SIGNAL(shiftPressed()), s.shift);
    s.no_shift->addTransition(this, SIGNAL(autoCapsActivated()), s.latched_shift);
    connect(s.no_shift, SIGNAL(entered()),
            this,       SLOT(switchLayoutToLower()));

    s.shift->addTransition(this, SIGNAL(shiftCancelled()), s.no_shift);
    s.shift->addTransition(this, SIGNAL(shiftReleased()), s.latched_shift);
    connect(s.shift, SIGNAL(entered()),
            this,    SLOT(switchLayoutToUpper()));

    s.latched_shift->addTransition(this, SIGNAL(shiftCancelled()), s.no_shift);
    s.latched_shift->addTransition(this, SIGNAL(shiftReleased()), s.caps_lock);
    connect(s.latched_shift, SIGNAL(entered()),
            this,            SLOT(switchLayoutToUpper()));

    s.caps_lock->addTransition(this, SIGNAL(shiftReleased()), s.no_shift);
    connect(s.caps_lock, SIGNAL(entered()),
            this,        SLOT(switchLayoutToUpper()));

    d->machine->addState(s.no_shift);
    d->machine->addState(s.shift);
    d->machine->addState(s.latched_shift);
    d->machine->addState(s.caps_lock);
    d->machine->setInitialState(s.no_shift);

    // Defer to first main loop iteration:
    QTimer::singleShot(0, d->machine.data(), SLOT(start()));
}

void LayoutUpdater::setLayout(const SharedLayout &layout)
{
    Q_D(LayoutUpdater);
    d->layout = layout;
}

void LayoutUpdater::setKeyboardLoader(KeyboardLoader *loader)
{
    Q_D(LayoutUpdater);
    d->loader.reset(loader);
}

void LayoutUpdater::onKeyPressed(const SharedLayout &layout,
                                 const Key &key)
{
    Q_D(const LayoutUpdater);

    if (d->layout != layout) {
        return;
    }

    // FIXME: Remove test code
    // TEST CODE STARTS
    bool static init = false;
        static QPixmap pressed_bg(8, 8);

    if (not init) {
        pressed_bg.fill(Qt::darkBlue);
        init = true;
    }
    // TEST CODE ENDS

    Key k(key);
    k.setBackground(pressed_bg);

    layout->appendActiveKey(k);
    emit activeKeysChanged(layout);

    if (key.action() == Key::ActionShift) {
        emit shiftPressed();
    }

    // MORE TEST CODE STARTS
    if (key.action() == Key::ActionSwitch) {
        k.setRect(k.rect().adjusted(0, 0, 20, 20));
        d->layout->setActiveKeyArea(replaceKey(d->layout->activeKeyArea(), k));
        emit layoutChanged(d->layout);
    }
    // TEST CODE ENDS
}

void LayoutUpdater::onKeyReleased(const SharedLayout &layout,
                                  const Key &key)
{
    Q_D(const LayoutUpdater);

    if (d->layout != layout) {
        return;
    }

    layout->removeActiveKey(key);
    emit activeKeysChanged(layout);

    if (key.action() == Key::ActionShift) {
        emit shiftReleased();
    } else if (d->machine->configuration().contains(d->states.latched_shift)) {
        emit shiftCancelled();
    }

    // MORE TEST CODE STARTS
    Key k(key);
    if (key.action() == Key::ActionSwitch) {
        k.setRect(k.rect().adjusted(0, 0, -20, -20));
        d->layout->setActiveKeyArea(replaceKey(d->layout->activeKeyArea(), k));
        emit layoutChanged(d->layout);
    }
    // TEST CODE ENDS
}

void LayoutUpdater::switchLayoutToUpper()
{
    Q_D(const LayoutUpdater);

    if (not d->layout) {
        return;
    }

    d->layout->setActiveKeyArea(transformKeyArea(d->layout->activeKeyArea(), TransformToUpper));
    emit layoutChanged(d->layout);
}

void LayoutUpdater::switchLayoutToLower()
{
    Q_D(const LayoutUpdater);

    if (not d->layout) {
        return;
    }

    d->layout->setActiveKeyArea(transformKeyArea(d->layout->activeKeyArea(), TransformToLower));
    emit layoutChanged(d->layout);
}

} // namespace MaliitKeyboard
