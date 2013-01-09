/*
 * This file is part of Maliit plugins
 *
 * Copyright (C) 2012 Openismus GmbH
 *
 * Contact: maliit-discuss@lists.maliit.org
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

Item {
    property alias layout: main.model
    property bool area_enabled // MouseArea has no id property so we cannot alias its enabled property.

    width: layout.width
    height: layout.height
    visible: layout.visible

    BorderImage {
        id: background
        anchors.fill: parent
        source: layout.background

        border.left: layout.background_borders.x
        border.top: layout.background_borders.y
        border.right: layout.background_borders.width
        border.bottom: layout.background_borders.height
    }

    Repeater {
        id: main
        model: layout
        anchors.fill: parent

        Item {
            x: key_reactive_area.x
            y: key_reactive_area.y
            width: key_reactive_area.width
            height: key_reactive_area.height

            BorderImage {
                x: key_rectangle.x
                y: key_rectangle.y
                width: key_rectangle.width
                height: key_rectangle.height

                border.left: key_background_borders.x
                border.top: key_background_borders.y
                border.right: key_background_borders.width
                border.bottom: key_background_borders.height

                source: key_background

                Text {
                    anchors.fill: parent
                    text: key_text
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            MouseArea {
                enabled: area_enabled
                anchors.fill: parent
                hoverEnabled: true

                onEntered: layout.onEntered(index)
                onExited: layout.onExited(index)
                onPressed: layout.onPressed(index)
                onReleased: layout.onReleased(index)
                onPressAndHold: layout.onPressAndHold(index)
            }
        }
    }
}
