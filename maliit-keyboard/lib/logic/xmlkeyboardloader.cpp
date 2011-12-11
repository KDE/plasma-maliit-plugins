// -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; c-file-offsets: ((innamespace . 0)); -*-
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

#include <QDir>
#include <QFile>
#include <QRegExp>

#include "parser/layoutparser.h"

#include "keyboardloader.h"

namespace {

using namespace MaliitKeyboard;

const char *const languages_dir(MALIIT_PLUGINS_DATA_DIR "/languages");

TagKeyboardPtr get_tag_keyboard(const QString& id)
{
    QFile file (QString::fromLatin1(languages_dir) + "/" + id + ".xml");

    if (file.exists()) {
        file.open(QIODevice::ReadOnly);

        LayoutParser parser(&file);
        const bool result(parser.parse());

        file.close();
        if (result) {
            return parser.keyboard();
        }
    }

    return TagKeyboardPtr();
}

QStringList get_imports(const QString &id)
{
    QFile file (QString::fromLatin1(languages_dir) + "/" + id + ".xml");

    if (file.exists()) {
        file.open(QIODevice::ReadOnly);

        LayoutParser parser(&file);
        const bool result(parser.parse());

        file.close();
        if (result) {
            return parser.imports();
        }
    }

    return QStringList();

}

Keyboard get_keyboard(const TagKeyboardPtr& keyboard,
                      bool shifted = false,
                      int page = 0,
                      const QString &dead_label = "")
{
    Keyboard skeyboard;
    const QChar dead_key((dead_label.size() == 1) ? dead_label[0] : QChar::Null);

    if (keyboard) {
        TagKeyboard::TagLayouts layouts(keyboard->layouts());

        if (not layouts.isEmpty()) {
            TagLayout::TagSections sections(layouts.first()->sections());
            // sections cannot be empty - parser does not allow that.
            TagSection::TagRows rows(sections[page % sections.size()]->rows());
            int row_num(0);

            Q_FOREACH (const TagRowPtr& row, rows) {
                TagRow::TagRowElements elements(row->elements());
                Q_FOREACH (const TagRowElementPtr& element, elements) {
                    if (element->element_type() == TagRowElement::Key) {
                        TagKeyPtr key(element.staticCast<TagKey>());
                        TagKey::TagBindings bindings(key->bindings());
                        Key skey;
                        KeyDescription skey_description;

                        skey_description.row = row_num;
                        skey_description.width = static_cast<KeyDescription::Width>(key->width());
                        skey_description.style = static_cast<KeyDescription::Style>(key->style());
                        skey_description.use_rtl_icon = key->rtl();

                        TagBindingPtr the_binding;

                        Q_FOREACH (const TagBindingPtr& binding, bindings) {
                            if (binding->shift() == shifted and not binding->alt()) {
                                the_binding = binding;
                                break;
                            }
                        }
                        if (not the_binding) {
                            the_binding = bindings.first();
                        }

                        KeyLabel label;
                        const int index(dead_key.isNull() ? -1 : the_binding->accents().indexOf(dead_key));

                        if (index < 0) {
                            label.setText(the_binding->label());
                        } else {
                            label.setText(the_binding->accented_labels()[index]);
                        }
                        if (the_binding->dead()) {
                            // TODO: document it.
                            skey.setAction(Key::ActionDead);
                        } else {
                            skey.setAction(static_cast<Key::Action>(the_binding->action()));
                        }
                        skey.setLabel(label);

                        skeyboard.keys.append(skey);
                        skeyboard.key_descriptions.append(skey_description);
                    }
                }
                ++row_num;
            }
        }
    }
    return skeyboard;
}

QPair<TagKeyPtr, TagBindingPtr> get_tag_key_and_binding(const TagKeyboardPtr &keyboard,
                                                        const QString &label)
{
    QPair<TagKeyPtr, TagBindingPtr> pair;

    if (keyboard) {
        TagKeyboard::TagLayouts layouts(keyboard->layouts());

        if (not layouts.isEmpty()) {
            // sections cannot be empty - parser does not allow that.
            TagSection::TagRows rows(layouts.first()->sections().first()->rows());

            Q_FOREACH (const TagRowPtr& row, rows) {
                TagRow::TagRowElements elements(row->elements());

                Q_FOREACH (const TagRowElementPtr& element, elements) {
                    if (element->element_type() == TagRowElement::Key) {
                        TagKeyPtr key(element.staticCast<TagKey>());
                        TagKey::TagBindings bindings(key->bindings());

                        Q_FOREACH (const TagBindingPtr& binding, bindings) {
                            if (binding->label() == label) {
                                pair.first = key;
                                pair.second = binding;
                                return pair;
                            }
                        }
                    }
                }
            }
        }
    }
    return pair;
}

}

namespace MaliitKeyboard {

class KeyboardLoaderPrivate
{
public:
    QString active_id;
    Keyboard keyboard;

    explicit KeyboardLoaderPrivate()
        : active_id()
    {}
};

KeyboardLoader::KeyboardLoader(QObject *parent)
    : QObject(parent)
    , d_ptr(new KeyboardLoaderPrivate)
{}

KeyboardLoader::~KeyboardLoader()
{}

QStringList KeyboardLoader::ids() const
{
    QStringList ids;
    QDir dir(languages_dir,
             "*.xml",
             QDir::Name | QDir::IgnoreCase,
             QDir::Files | QDir::NoSymLinks | QDir::Readable);

    if (dir.exists()) {
        QFileInfoList file_infos(dir.entryInfoList());

        Q_FOREACH (const QFileInfo& file_info, file_infos) {
            ids.append(file_info.baseName());
        }
    }
    return ids;
}

QString KeyboardLoader::activeId() const
{
    Q_D(const KeyboardLoader);
    return d->active_id;
}

void KeyboardLoader::setActiveId(const QString &id)
{
    Q_D(KeyboardLoader);

    if (d->active_id != id) {
        d->active_id = id;

        // FIXME: Emit only after parsing new keyboard.
        emit keyboardsChanged();
    }
}

QString KeyboardLoader::title(const QString &id) const
{
    TagKeyboardPtr keyboard(get_tag_keyboard(id));

    if (keyboard) {
        return keyboard->title();
    }
    return "invalid";
}

Keyboard KeyboardLoader::keyboard() const
{
    Q_D(const KeyboardLoader);
    TagKeyboardPtr keyboard(get_tag_keyboard(d->active_id));

    return get_keyboard(keyboard);
}

Keyboard KeyboardLoader::nextKeyboard() const
{
    Q_D(const KeyboardLoader);

    const QStringList all_ids(ids());

    if (all_ids.isEmpty()) {
        return Keyboard();
    }

    int next_index (all_ids.indexOf(d->active_id) + 1);

    if (next_index >= all_ids.size()) {
        next_index = 0;
    }

    TagKeyboardPtr keyboard(get_tag_keyboard(all_ids[next_index]));

    return get_keyboard(keyboard);
}

Keyboard KeyboardLoader::previousKeyboard() const
{
    Q_D(const KeyboardLoader);

    const QStringList all_ids(ids());

    if (all_ids.isEmpty()) {
        return Keyboard();
    }

    int previous_index (all_ids.indexOf(d->active_id) - 1);

    if (previous_index < 0) {
        previous_index = 0;
    }

    TagKeyboardPtr keyboard(get_tag_keyboard(all_ids[previous_index]));

    return get_keyboard(keyboard);
}

Keyboard KeyboardLoader::shiftedKeyboard() const
{
    Q_D(const KeyboardLoader);
    TagKeyboardPtr keyboard(get_tag_keyboard(d->active_id));

    return get_keyboard(keyboard, true);
}

Keyboard KeyboardLoader::symbolsKeyboard(int page) const
{
    Q_D(const KeyboardLoader);
    QStringList imports(get_imports(d->active_id));
    const QRegExp symbols_regexp("^(symbols.*).xml$");

    Q_FOREACH (const QString &import, imports) {
        if (symbols_regexp.exactMatch(import)) {
            TagKeyboardPtr keyboard(get_tag_keyboard(symbols_regexp.cap(1)));
            return get_keyboard(keyboard, false, page);
        }
    }

    return Keyboard();
}

Keyboard KeyboardLoader::deadKeyboard(const Key &dead) const
{
    Q_D(const KeyboardLoader);
    TagKeyboardPtr keyboard(get_tag_keyboard(d->active_id));

    // TODO: detect whether we want it shifted or not.
    return get_keyboard(keyboard, false, 0, dead.label().text());
}

Keyboard KeyboardLoader::extendedKeyboard(const Key &key) const
{
    Q_D(const KeyboardLoader);
    const TagKeyboardPtr keyboard(get_tag_keyboard(d->active_id));
    const QPair<TagKeyPtr, TagBindingPtr> pair(get_tag_key_and_binding(keyboard, key.label().text()));
    Keyboard skeyboard;

    if (pair.first and pair.second) {
        const QString extended_labels(pair.second->extended_labels());

        Q_FOREACH(const QChar &c, extended_labels) {
            Key skey;
            KeyDescription skey_description;
            KeyLabel label;

            label.setText(c);
            skey.setLabel(label);
            skey.setAction(static_cast<Key::Action>(pair.second->action()));
            skey_description.row = 0;
            skey_description.width = static_cast<KeyDescription::Width>(pair.first->width());
            skey_description.style = static_cast<KeyDescription::Style>(pair.first->style());
            skey_description.use_rtl_icon = pair.first->rtl();
            skeyboard.keys.append(skey);
            skeyboard.key_descriptions.append(skey_description);
        }
    }
    return skeyboard;
}

} // namespace MaliitKeyboard
