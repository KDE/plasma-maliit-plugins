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

#include "style.h"

namespace MaliitKeyboard {

namespace {
const char *const style_filename(MALIIT_PLUGINS_DATA_DIR "/org/maliit/style.ini");
const QString key_with_format("%1/key-width%2");
const QString key_icon_format("icon/%1%2");
const QString key_background_format("background/%1%2");

QString fromKeyWidth(KeyDescription::Width width)
{
    switch(width) {
    case KeyDescription::Medium: return "";
    case KeyDescription::Small: return "-small";
    case KeyDescription::Large: return "-large";
    case KeyDescription::XLarge: return "-xlarge";
    case KeyDescription::XXLarge: return "-xxlarge";
    case KeyDescription::Stretched: return "-stretched"; // makes no sense actually.
    }

    return QString();
}

QString fromKeyIcon(KeyDescription::Icon icon)
{
    switch (icon) {
    case KeyDescription::NoIcon: return "";
    case KeyDescription::ReturnIcon: return "return";
    case KeyDescription::BackspaceIcon: return "backspace";
    case KeyDescription::ShiftIcon: return "shift";
    case KeyDescription::ShiftLatchedIcon: return "shift-latched";
    case KeyDescription::CapsLockIcon: return "caps-lock";
    }

    return QString();
}

QString fromKeyStyle(KeyDescription::Style style)
{
    switch (style) {
    case KeyDescription::NormalStyle: return "normal";
    case KeyDescription::DeadkeyStyle: return "dead";
    case KeyDescription::SpecialStyle: return "special";
    }

    return QString();
}

QString fromKeyState(KeyDescription::State state)
{
    switch (state) {
    case KeyDescription::NormalState: return "";
    case KeyDescription::PressedState: return "-pressed";
    case KeyDescription::DisabledState: return "-disabled";
    case KeyDescription::HighlightedState: return "-highlighted";
    }

    return QString();
}

QPixmap loadImage(const QString &id,
                  QSettings *store,
                  QHash<QString, QPixmap> *cache)
{
    if (cache) {
        const QPixmap &found(cache->value(id));

        if (not found.isNull()) {
            return found;
        }
    }

    if (not store) {
        qCritical() << __PRETTY_FUNCTION__
                    << "No store given, aborting.";
        return QPixmap();
    }

    const QPixmap &image(store->value(id).toString());

    if (image.isNull()) {
        // Try a one-step lookup in parent group:
        const QString &group(store->group());
        store->endGroup();
        const QPixmap &next_image(store->value(id).toString());
        store->beginGroup(group);

        // OK, giving up:
        if (next_image.isNull()) {
            qWarning() << __PRETTY_FUNCTION__
                       << "Image not found. Image id:" << id
                       << ", file name:" << store->value(id).toString();
        }
    }

    if (cache) {
        cache->insert(id, image);
    }

    return image;
}

}


class StylePrivate
{
public:
    QString name;
    mutable QSettings store;
    mutable QHash<QString, QPixmap> image_cache;

    explicit StylePrivate()
        : name()
        , store(style_filename, QSettings::IniFormat)
        , image_cache()
    {}
};

Style::Style(QObject *parent)
    : QObject(parent)
    , d_ptr(new StylePrivate)
{}

Style::~Style()
{}

void Style::setStyleName(const QString &name)
{
    Q_D(Style);

    if (d->name != name) {
        d->store.endGroup();
        d->name = name;
        d->image_cache.clear();
        d->store.beginGroup(d->name);
    }
}

QPixmap Style::background(KeyDescription::Style style,
                          KeyDescription::State state) const
{
    Q_D(const Style);

    return loadImage(key_background_format
                     .arg(fromKeyStyle(style))
                     .arg(fromKeyState(state)),
                     &d->store, &d->image_cache);
}

QPixmap Style::icon(KeyDescription::Icon icon,
                    KeyDescription::State state) const
{
    Q_D(const Style);

    return loadImage(key_icon_format
                     .arg(fromKeyIcon(icon))
                     .arg(fromKeyState(state)),
                     &d->store, &d->image_cache);
}

QString Style::fontName(const QString &group_id) const
{
    Q_UNUSED(group_id)
    return "Nokia Pure";
}

qreal Style::fontSize(const QString &group_id) const
{
    Q_UNUSED(group_id)
    return 20;
}

qreal Style::keyHeight(Layout::Orientation orientation) const
{
    Q_D(const Style);

    switch(orientation) {
    case Layout::Landscape:
        return d->store.value("landscape/key-height").toReal();

    case Layout::Portrait:
        return d->store.value("portrait/key-height").toReal();
    }

    return 0;
}

qreal Style::keyWidth(Layout::Orientation orientation,
                      KeyDescription::Width width) const
{
    Q_D(const Style);

    switch(orientation) {
    case Layout::Landscape:
        return d->store.value(key_with_format
                              .arg("landscape")
                              .arg(fromKeyWidth(width))).toReal();

    case Layout::Portrait:
        return d->store.value(key_with_format
                              .arg("portrait")
                              .arg(fromKeyWidth(width))).toReal();
    }

    return 0;
}

qreal Style::keyAreaWidth(Layout::Orientation orientation) const
{
    Q_D(const Style);

    switch(orientation) {
    case Layout::Landscape:
        return d->store.value("landscape/key-area-width").toReal();

    case Layout::Portrait:
        return d->store.value("portrait/key-area-width").toReal();
    }

    return 0;
}

} // namespace MaliitKeyboard
