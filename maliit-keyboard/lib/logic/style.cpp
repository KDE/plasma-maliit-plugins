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
const char *const styles_dir(MALIIT_KEYBOARD_STYLES_DIR);
const char *const images_dir(MALIIT_KEYBOARD_IMAGES_DIR);

const QString profile_filename_format("%1/%2.ini");
const QString image_filename_format("%1/%2");
const QString key_with_format("key-width%2");

const QString backgroud_id_build_format("background/%1%2");
const QString icon_id_build_format("icon/%1%2");
const QString id_build_format("%1/%2/%3");

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

QString buildBackgroundId(KeyDescription::Style style,
                          KeyDescription::State state)
{
    return backgroud_id_build_format
           .arg(fromKeyStyle(style))
           .arg(fromKeyState(state));
}

QString buildIconId(KeyDescription::Icon icon,
                    KeyDescription::State state)
{
    return icon_id_build_format
           .arg(fromKeyIcon(icon))
           .arg(fromKeyState(state));
}

QString buildId(Layout::Orientation orientation,
                const QString &style_name,
                const QString &id)
{
    return id_build_format
           .arg(style_name)
           .arg(orientation == Layout::Landscape ? "landscape" : "portrait")
           .arg(id);
}

QVariant lookup(const QScopedPointer<QSettings> &store,
                Layout::Orientation orientation,
                const QString &style_name,
                const QString &id)
{
    if (store.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "No store found, aborting.";
        return QVariant();
    }

    const QVariant &result(store->value(buildId(orientation, style_name, id)));

    if (not result.isValid()) {
        return store->value(buildId(orientation, "default", id));
    }

    return result;
}

QPixmap loadImage(const QString &id,
                  const QScopedPointer<QSettings> &store,
                  QHash<QString, QPixmap> *cache)
{
    if (cache) {
        const QPixmap &found(cache->value(id));

        if (not found.isNull()) {
            return found;
        }
    }

    if (store.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "No store given, aborting.";
        return QPixmap();
    }

    const QPixmap &image(image_filename_format
                         .arg(images_dir)
                         .arg(store->value(id).toString()));

    if (image.isNull()) {
        qWarning() << __PRETTY_FUNCTION__
                   << "Image not found. Image id:" << id
                   << ", file name:" << images_dir << store->value(id).toString();
    } else if (cache) {
        cache->insert(id, image);
    }

    return image;
}

}


class StylePrivate
{
public:
    QString name;
    QScopedPointer<QSettings> store;
    mutable QHash<QString, QPixmap> image_cache;

    explicit StylePrivate()
        : name()
        , store()
        , image_cache()
    {}
};

Style::Style(QObject *parent)
    : QObject(parent)
    , d_ptr(new StylePrivate)
{}

Style::~Style()
{}

void Style::setProfile(const QString &profile)
{
    Q_D(Style);

    d->store.reset(new QSettings(profile_filename_format
                                 .arg(styles_dir).arg(profile),
                                 QSettings::IniFormat));
}

void Style::setStyleName(const QString &name)
{
    Q_D(Style);
    d->name = name;
}

QPixmap Style::keyBackground(KeyDescription::Style style,
                             KeyDescription::State state) const
{
    Q_D(const Style);

    return loadImage(buildBackgroundId(style, state),
                     d->store, &d->image_cache);
}

QPixmap Style::icon(KeyDescription::Icon icon,
                    KeyDescription::State state) const
{
    Q_D(const Style);

    return loadImage(buildIconId(icon, state),
                     d->store, &d->image_cache);
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
    return lookup(d->store, orientation, d->name, "key-height").toReal();
}

qreal Style::keyWidth(Layout::Orientation orientation,
                      KeyDescription::Width width) const
{
    Q_D(const Style);
    return lookup(d->store, orientation, d->name, key_with_format.arg(fromKeyWidth(width))).toReal();
}

qreal Style::keyAreaWidth(Layout::Orientation orientation) const
{
    Q_D(const Style);
    return lookup(d->store, orientation, d->name, "key-area-width").toReal();
}

qreal Style::keyMargin(Layout::Orientation orientation) const
{
    Q_D(const Style);
    return lookup(d->store, orientation, d->name, "key-margins").toReal();
}

qreal Style::keyAreaPadding(Layout::Orientation orientation) const
{
    Q_D(const Style);
    return lookup(d->store, orientation, d->name, "key-area-paddings").toReal();
}

} // namespace MaliitKeyboard
