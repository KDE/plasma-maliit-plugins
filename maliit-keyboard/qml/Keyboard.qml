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
    property variant event_handler
    property bool area_enabled // MouseArea has no id property so we cannot alias its enabled property.
    property alias title: keyboard_title.text

    width: layout.width
    height: layout.height
    visible: layout.visible

    Connections {
        target: layout
        onTitleChanged: {
            console.debug("title:" + layout.title)
            title_timeout.start()
        }
    }

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
                    font.family: key_font
                    font.pointSize: key_font_size
                    color: key_font_color
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    visible: (key_text.length != 0)
                }

                Image {
                    anchors.centerIn: parent
                    source: key_icon
                    visible: (key_icon.length != 0)
                }
            }

            MouseArea {
                property real start_x
                property real start_y

                Timer {
                    id: gesture_timeout
                    interval: 500
                }

                enabled: area_enabled
                anchors.fill: parent
                hoverEnabled: true

                onEntered: event_handler.onEntered(index)
                onExited: event_handler.onExited(index)

                onPressed: {
                    start_x = mouse.x
                    start_y = mouse.y
                    gesture_timeout.start()

                    event_handler.onPressed(index)
                }

                onReleased: event_handler.onReleased(index)
                onPressAndHold: event_handler.onPressAndHold(index)

                // TODO: Move logic into EventHandler because gestures should depend on style?
                // Hide keyboard on flick-down gesture (but only if there is an event_handler)
                // or switch to left/right layout:
                onPositionChanged: {
                    if (event_handler
                        && gesture_timeout.running
                        && (mouse.y - start_y > (layout.height * 0.3))) {
                        maliit.hide()
                    } else if (event_handler
                               && gesture_timeout.running
                               && (mouse.x - start_x > (layout.width * 0.2))) {
                        maliit.selectLeftLayout()
                    } else if (event_handler
                               && gesture_timeout.running
                               && (start_x - mouse.x > (layout.width * 0.2))) {
                        maliit.selectRightLayout()
                    }
                }
            }
        }
    }

    // Keyboard title rendering
    // TODO: Make separate component?
    Item {
        anchors.centerIn: parent
        opacity: title_timeout.running ? 1.0 : 0.0

        Behavior on opacity {
            PropertyAnimation {
                duration: 300
                easing.type: Easing.InOutQuad
            }
        }

        Timer {
            id: title_timeout
            interval: 1000
        }

        // TODO: Make title background part of styling profile.
        BorderImage {
            anchors.centerIn: parent

            // Manual padding of text:
            width: keyboard_title.width * 1.2
            height: keyboard_title.height * 1.2

            //anchors.fill: keyboard_title
            source: layout.background
            z: 1000 // Move behind Text element but in front of rest.

            border.left: layout.background_borders.x
            border.top: layout.background_borders.y
            border.right: layout.background_borders.width
            border.bottom: layout.background_borders.height
        }

        Text {
            id: keyboard_title
            anchors.centerIn: parent

            text: title;
            z: 1001

            // TODO: Make title font part of styling profile.
            font.pointSize: 48
            color: "white"
        }
    }
}
