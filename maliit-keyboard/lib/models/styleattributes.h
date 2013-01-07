/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2012 One Laptop per Child Association
 * Copyright (C) 2012-2013 Canonical Ltd
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

#ifndef MALIIT_KEYBOARD_STYLEATTRIBUTES_H
#define MALIIT_KEYBOARD_STYLEATTRIBUTES_H

#include "models/keydescription.h"
#include "logic/layout.h"

#include <QtCore>

namespace MaliitKeyboard {

class StyleAttributes
{
private:
    const QScopedPointer<const QSettings> m_store;
    QString m_style_name;

public:
    explicit StyleAttributes(const QSettings *store);
    virtual ~StyleAttributes();

    virtual void setStyleName(const QString &name);
    QByteArray wordRibbonBackground() const;
    QByteArray keyAreaBackground() const;
    QByteArray magnifierKeyBackground() const;
    QByteArray keyBackground(Key::Style style,
                             KeyDescription::State state) const;

    QMargins wordRibbonBackgroundBorders() const;
    QMargins keyAreaBackgroundBorders() const;
    QMargins magnifierKeyBackgroundBorders() const;
    QMargins keyBackgroundBorders() const;

    QByteArray icon(KeyDescription::Icon icon,
                    KeyDescription::State state) const;

    QByteArray customIcon(const QString &icon_name) const;

    QStringList fontFiles() const;

    QByteArray fontName(Logic::Layout::Orientation orientation) const;
    QByteArray fontColor(Logic::Layout::Orientation orientation) const;
    qreal fontSize(Logic::Layout::Orientation orientation) const;
    qreal smallFontSize(Logic::Layout::Orientation orientation) const;
    qreal candidateFontSize(Logic::Layout::Orientation orientation) const;
    qreal magnifierFontSize(Logic::Layout::Orientation orientation) const;
    qreal candidateFontStretch(Logic::Layout::Orientation orientation) const;

    qreal wordRibbonHeight(Logic::Layout::Orientation orientation) const;
    qreal magnifierKeyHeight(Logic::Layout::Orientation orientation) const;
    qreal keyHeight(Logic::Layout::Orientation orientation) const;

    qreal magnifierKeyWidth(Logic::Layout::Orientation orientation) const;
    qreal keyWidth(Logic::Layout::Orientation orientation,
                   KeyDescription::Width width) const;
    qreal keyAreaWidth(Logic::Layout::Orientation orientation) const;

    qreal keyMargin(Logic::Layout::Orientation orientation) const;
    qreal keyAreaPadding(Logic::Layout::Orientation orienation) const;

    qreal verticalOffset(Logic::Layout::Orientation orientation) const;
    qreal magnifierKeyLabelVerticalOffset(Logic::Layout::Orientation orientation) const;
    qreal safetyMargin(Logic::Layout::Orientation orientation) const;

    QByteArray keyPressSound() const;
    QByteArray keyReleaseSound() const;
    QByteArray layoutChangeSound() const;
    QByteArray keyboardHideSound() const;
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_STYLEATTRIBUTES_H
