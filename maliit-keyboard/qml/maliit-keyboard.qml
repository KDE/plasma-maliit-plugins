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
    Item {
        id: main_area
        width: maliit_layout.width
        height: maliit_layout.height
        visible: maliit_layout.visible

        BorderImage {
            anchors.fill: parent
            source: maliit_layout.background
        }

        Repeater {
            id: main
            model: maliit_layout
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
                    enabled: !maliit_extended_layout.visible
                    anchors.fill: parent
                    hoverEnabled: true

                    onEntered: maliit_layout.onEntered(index)
                    onExited: maliit_layout.onExited(index)
                    onPressed: maliit_layout.onPressed(index)
                    onReleased: maliit_layout.onReleased(index)
                    onPressAndHold: maliit_layout.onPressAndHold(index)
                }
            }
        }
    }

    // TODO: Move to a separate file and bind to a separate surface, too.
    Item {
        id: extended_area
        width: maliit_extended_layout.width
        height: maliit_extended_layout.height
        visible: maliit_extended_layout.visible

        BorderImage {
            anchors.fill: parent
            source: maliit_extended_layout.background
        }

        Repeater {
            id: extended
            model: maliit_extended_layout
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
                    anchors.fill: parent
                    hoverEnabled: true

                    onEntered: maliit_extended_layout.onEntered(index)
                    onExited: maliit_extended_layout.onExited(index)
                    onPressed: maliit_extended_layout.onPressed(index)
                    onReleased: maliit_extended_layout.onReleased(index)
                }
            }
        }
    }
}
