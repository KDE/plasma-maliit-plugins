/*
 * This file is part of Maliit plugins
 *
 * Copyright (C) Jakub Pavelek <jpavelek@live.com>
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

import QtQuick 2.0
import "KeyboardUiConstants.js" as UI
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaCore.FrameSvgItem {
    id: popper
    imagePath: "widgets/background"

    width: popperText.width + margins.left + margins.right
    height: popperText.height + margins.top + margins.bottom
    opacity: 0
    anchors.bottomMargin: 8
    property Item target: null

    PlasmaComponents.Label {
        id: popperText
        text: ""
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pointSize: theme.defaultFont.pointSize * 2
    }

    states: State {
        name: "active"
        when: target !== null && target.showPopper

        PropertyChanges {
            target: popperText
            text: target.text
        }

        PropertyChanges {
            target: popper
            opacity: 1

            x: popper.parent.mapFromItem(target, 0, 0).x + (target.width - popper.width) / 2
            y: popper.parent.mapFromItem(target, 0, 0).y - popper.height
        }
    }

    transitions: Transition {
        from: "active"

        SequentialAnimation {
            PauseAnimation {
                duration: 50
            }
            PropertyAction {
                target: popper
                properties: "opacity, x, y"
            }
            PropertyAction {
                target: popperText
                property: "text"
            }
        }
    }
}
