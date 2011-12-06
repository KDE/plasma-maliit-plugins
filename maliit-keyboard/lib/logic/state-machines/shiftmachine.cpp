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

#include "shiftmachine.h"
#include "logic/layoutupdater.h"

namespace MaliitKeyboard {

ShiftMachine::ShiftMachine(QObject *parent)
    : QStateMachine(parent)
{}

ShiftMachine::~ShiftMachine()
{}

void ShiftMachine::setup(LayoutUpdater *updater)
{
    if (not updater) {
        qCritical() << __PRETTY_FUNCTION__
                    << "No updater specified. Aborting setup.";
        return;
    }

    setChildMode(QState::ExclusiveStates);

    QState *no_shift = 0;
    QState *shift = 0;
    QState *latched_shift = 0;
    QState *caps_lock = 0;

    // TODO: does ShiftMachine really take ownership?
    addState(no_shift = new QState);
    addState(shift = new QState);
    addState(latched_shift = new QState);
    addState(caps_lock = new QState);
    setInitialState(no_shift);

    no_shift->setObjectName("no-shift");
    shift->setObjectName("shift");
    latched_shift->setObjectName("latched-shift");
    caps_lock->setObjectName("caps-lock");

    no_shift->addTransition(updater, SIGNAL(shiftPressed()), shift);
    no_shift->addTransition(updater, SIGNAL(autoCapsActivated()), latched_shift);
    connect(no_shift, SIGNAL(entered()),
            updater,  SLOT(switchLayoutToLower()));

    shift->addTransition(updater, SIGNAL(shiftCancelled()), no_shift);
    shift->addTransition(updater, SIGNAL(shiftReleased()), latched_shift);
    connect(shift,   SIGNAL(entered()),
            updater, SLOT(switchLayoutToUpper()));

    latched_shift->addTransition(updater, SIGNAL(shiftCancelled()), no_shift);
    latched_shift->addTransition(updater, SIGNAL(shiftReleased()), caps_lock);
    connect(latched_shift, SIGNAL(entered()),
            updater,       SLOT(switchLayoutToUpper()));

    caps_lock->addTransition(updater, SIGNAL(shiftReleased()), no_shift);
    connect(caps_lock, SIGNAL(entered()),
            updater,   SLOT(switchLayoutToUpper()));

    // Defer to first main loop iteration:
    QTimer::singleShot(0, this, SLOT(start()));
}

} // namespace MaliitKeyboard
