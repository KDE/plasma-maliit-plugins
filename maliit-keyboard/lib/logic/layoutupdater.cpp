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
#include "logic/state-machines/shiftmachine.h"
#include "logic/state-machines/viewmachine.h"
#include "models/keyboard.h"
#include "models/keydescription.h"

namespace MaliitKeyboard {

namespace {

enum Transform {
    TransformToUpper,
    TransformToLower
};

bool verify(const QScopedPointer<KeyboardLoader> &loader,
            const SharedLayout &layout)
{
    if (loader.isNull() || layout.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Could not find keyboard loader or layout, forgot to set them?";
        return false;
    }

    return true;
}

// FIXME: only access settings *once*
QPoint computeAnchor(Layout::Orientation orientation)
{
    // A workaround for virtual desktops where QDesktopWidget reports
    // screenGeometry incorrectly: Allow user to override via settings.
    QSettings settings("maliit.org", "vkb");
    QPoint anchor(settings.value("anchor").toPoint());

    if (anchor.isNull()) {
        const QRect screen_area(QApplication::desktop() ? QApplication::desktop()->screenGeometry()
                                                        : QRect(0, 0, 480, 854));
        anchor = (orientation == Layout::Landscape)
                  ? QPoint(screen_area.width() / 2, screen_area.height())
                  : QPoint(screen_area.height() / 2, screen_area.width());

        // Enforce creation of settings file, otherwise it's too hard to find
        // the override (and get the syntax ride). I know, it's weird.
        settings.setValue("_anchor", anchor);
    }

    return anchor;
}

KeyArea transformKeyArea(const KeyArea &ka,
                         Transform t)
{
    KeyArea new_ka;
    new_ka.rect = ka.rect;

    foreach (Key key, ka.keys) {
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
        new_ka.keys.append(key);
    }

    return new_ka;
}

KeyArea replaceKey(const KeyArea &ka,
                   const Key &replace)
{
    KeyArea new_ka;
    new_ka.rect = ka.rect;

    foreach (const Key &key, ka.keys) {
        new_ka.keys.append((key.label().text() == replace.label().text()) ? replace : key);
    }

    return new_ka;
}

KeyArea createFromKeyboard(const Keyboard &source,
                           const QPoint &anchor,
                           int max_width,
                           Layout::Orientation orientation)
{
    // An ad-hoc geometry updater that also uses styling information.
    // Will only work for portrait mode (lots of hardcoded stuff).
    KeyArea ka;
    Keyboard kb(source);

    QPixmap normal_bg(4, 4);
    QPixmap special_bg(4, 4);
    QPixmap deadkey_bg(4, 4);

    normal_bg.fill(QColor("#333"));
    special_bg.fill(QColor("#999"));
    deadkey_bg.fill(QColor("#ccc"));

    static SharedFont font(new QFont);
    font->setPointSize(20);

    static SharedFont small_font(new QFont);
    small_font->setPointSize(12);

    QPoint pos(0, 0);
    const int row_height = (orientation == Layout::Landscape) ? 60 : 80;
    const qreal base_width = (orientation == Layout::Landscape) ? 78 : 42;
    const qreal margin = 3;
    int prev_row = 0;
    QVector<int> row_indices;
    int spacer_count = 0;


    for (int index = 0; index < kb.keys.count(); ++index) {
        row_indices.append(index);
        Key &key(kb.keys[index]);
        const KeyDescription &desc(kb.key_descriptions.at(index));
        int width = 0;
        pos.setY(row_height * desc.row);

        if (desc.left_spacer || desc.right_spacer) {
            ++spacer_count;
        }

        switch (desc.style) {
        case KeyDescription::NormalStyle: key.setBackground(normal_bg); break;
        case KeyDescription::SpecialStyle: key.setBackground(special_bg); break;
        case KeyDescription::DeadkeyStyle: key.setBackground(deadkey_bg); break;
        }

        qreal k = 1;

        switch (desc.width) {
        case KeyDescription::Small: k = 0.5; break;
        case KeyDescription::Medium: k = 1; break;
        case KeyDescription::Large: k = 1.5; break;
        case KeyDescription::XLarge: k = 2; break;
        case KeyDescription::XXLarge: k = 2.5; break;
        case KeyDescription::Stretched: k = 3; break;
        }

        width = k * base_width + (k - 0.5) * margin;

        width += margin * 2;
        prev_row = desc.row;
        key.setRect(QRect(pos.x(), pos.y(), width, row_height));
        key.setMargins(QMargins(3, 3, 3, 3));

        KeyLabel label(key.label());
        label.setFont(label.text().count() > 1 ? small_font : font);
        label.setColor(Qt::white);
        label.setRect(QRect(4, 4, key.rect().width() - 8, key.rect().height() - 8));
        key.setLabel(label);

        pos += QPoint(width, 0);

        if ((index + 1 == kb.keys.count())
            || (index + 1 < kb.keys.count() && kb.key_descriptions.at(index + 1).row > desc.row)) {
            if (spacer_count > 0 && pos.x() < max_width + 1) {
                const int spacer_width = qMax<int>(0, max_width - pos.x()) / spacer_count;
                pos.setX(0);
                int right_x = 0;

                foreach (int row_index, row_indices) {
                    Key &k(kb.keys[row_index]);
                    const KeyDescription &d(kb.key_descriptions.at(row_index));

                    QRect r(k.rect());
                    QMargins m(k.margins());
                    int extra_width = 0;

                    if (d.left_spacer) {
                        m.setLeft(m.left() + spacer_width);
                        extra_width += spacer_width;
                    }

                    if (d.right_spacer) {
                        m.setRight(m.right() + spacer_width);
                        extra_width += spacer_width;
                    }

                    k.setMargins(m);

                    r.translate(right_x - r.left(), 0);
                    r.setWidth(r.width() + extra_width);
                    k.setRect(r);

                    right_x = r.right();
                }

            }

            row_indices.clear();
            pos.setX(0);
            spacer_count = 0;
        }
    }

    const int height = pos.y() + row_height;
    ka.keys = kb.keys;
    ka.rect =  QRectF(anchor.x() - max_width / 2, anchor.y() - height,
                      max_width, height);

    return ka;
}

}

class LayoutUpdaterPrivate
{
public:
    bool initialized;
    SharedLayout layout;
    QScopedPointer<KeyboardLoader> loader;
    ShiftMachine shift_machine;
    ViewMachine view_machine;
    QPoint anchor;

    explicit LayoutUpdaterPrivate()
        : initialized(false)
        , layout()
        , loader(new KeyboardLoader)
        , shift_machine()
        , view_machine()
        , anchor()
    {}
};

LayoutUpdater::LayoutUpdater(QObject *parent)
    : QObject(parent)
    , d_ptr(new LayoutUpdaterPrivate)
{
    connect(d_ptr->loader.data(), SIGNAL(keyboardsChanged()),
            this,                 SLOT(onKeyboardsChanged()),
            Qt::UniqueConnection);
}

LayoutUpdater::~LayoutUpdater()
{}

void LayoutUpdater::init()
{
    Q_D(LayoutUpdater);

    d->anchor = computeAnchor(Layout::Landscape);
    d->shift_machine.setup(this);
    d->view_machine.setup(this);
}

QStringList LayoutUpdater::keyboardIds() const
{
    Q_D(const LayoutUpdater);
    return d->loader->ids();
}

QString LayoutUpdater::activeKeyboardId() const
{
    Q_D(const LayoutUpdater);
    return d->loader->activeId();
}

void LayoutUpdater::setActiveKeyboardId(const QString &id)
{
    Q_D(LayoutUpdater);
    d->loader->setActiveId(id);
}

QString LayoutUpdater::keyboardTitle(const QString &id) const
{
    Q_D(const LayoutUpdater);
    return d->loader->title(id);
}

void LayoutUpdater::setLayout(const SharedLayout &layout)
{
    Q_D(LayoutUpdater);
    d->layout = layout;

    if (not d->initialized) {
        init();
        d->initialized = true;
    }
}

void LayoutUpdater::resetKeyboardLoader(KeyboardLoader *loader)
{
    Q_D(LayoutUpdater);
    d->loader.reset(loader);

    connect(loader, SIGNAL(keyboardsChanged()),
            this,   SLOT(onKeyboardsChanged()),
            Qt::UniqueConnection);
}

void LayoutUpdater::setOrientation(Layout::Orientation orientation)
{
    Q_D(LayoutUpdater);

    if (d->layout && d->layout->orientation() != orientation) {
        d->layout->setOrientation(orientation);

        // FIXME: reposition extended keys, too (and left/right?).
        // FIXME: consider shift state.
        d->anchor = computeAnchor(orientation);
        d->layout->setCenterPanel(createFromKeyboard(d->loader->keyboard(),
                                                     d->anchor, d->anchor.x() * 2,
                                                     orientation));
        emit layoutChanged(d->layout);
    }
}

void LayoutUpdater::onKeyPressed(const Key &key,
                                 const SharedLayout &layout)
{
    Q_D(const LayoutUpdater);

    if (d->layout != layout) {
        return;
    }

    // FIXME: Remove test code
    // TEST CODE STARTS
    bool static init = false;
    static QPixmap pressed_bg(8, 8);
    static QPixmap magnifier_bg(8, 8);
    static SharedFont magnifier_font(new QFont);

    if (not init) {
        pressed_bg.fill(Qt::blue);
        magnifier_bg.fill(QColor("#eee"));
        magnifier_font->setPointSize(40);
        init = true;
    }
    // TEST CODE ENDS

    Key k(key);
    k.setBackground(pressed_bg);
    layout->appendActiveKey(k);

    if (key.action() == Key::ActionCommit) {
        Key magnifier(key);
        magnifier.setBackground(magnifier_bg);

        QRect magnifier_rect(key.rect().translated(0, -120).adjusted(-20, -20, 20, 20));
        const QRectF key_area_rect(d->layout->activeKeyArea().rect);
        if (magnifier_rect.left() < key_area_rect.left() + 10) {
            magnifier_rect.setLeft(key_area_rect.left() + 10);
        } else if (magnifier_rect.right() > key_area_rect.right() - 10) {
            magnifier_rect.setRight(key_area_rect.right() - 10);
        }

        magnifier.setRect(magnifier_rect);
        KeyLabel magnifier_label(magnifier.label());
        magnifier_label.setColor(Qt::black);
        magnifier_label.setRect(magnifier_label.rect().adjusted(0, 0, 40, 40));
        magnifier_label.setFont(magnifier_font);
        magnifier.setLabel(magnifier_label);
        layout->setMagnifierKey(magnifier);
    }

    emit keysChanged(layout);

    if (key.action() == Key::ActionShift) {
        emit shiftPressed();
    }
}

void LayoutUpdater::onKeyReleased(const Key &key,
                                  const SharedLayout &layout)
{
    Q_D(const LayoutUpdater);

    if (d->layout != layout) {
        return;
    }

    layout->removeActiveKey(key);
    layout->setMagnifierKey(Key());
    emit keysChanged(layout);

    switch (key.action()) {
    case Key::ActionShift:
        emit shiftReleased();
        break;

    case Key::ActionCommit:
        if (d->shift_machine.inState("latched-shift")) {
            emit shiftCancelled();
        }
        break;

    case Key::ActionSym:
        emit symKeyReleased();
        break;

    case Key::ActionSwitch:
        emit symSwitcherReleased();
        break;

    default:
        break;
    }
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

void LayoutUpdater::onKeyboardsChanged()
{
    Q_D(const LayoutUpdater);

    if (not verify(d->loader, d->layout)) {
        return;
    }

    d->layout->setCenterPanel(createFromKeyboard(d->loader->keyboard(),
                                                 d->anchor, d->anchor.x() * 2,
                                                 d->layout->orientation()));
    emit layoutChanged(d->layout);
}

void LayoutUpdater::switchToMainView()
{
    // This will undo the changes done by shift, which is perhaps what we want.
    // But if shift state is actually dependent on view state, then that's
    // needs to be modelled as part of the state machines, or not?
    onKeyboardsChanged();
}

void LayoutUpdater::switchToPrimarySymView()
{
    Q_D(LayoutUpdater);

    if (not verify(d->loader, d->layout)) {
        return;
    }

    d->layout->setCenterPanel(createFromKeyboard(d->loader->symbolsKeyboard(0),
                                                 d->anchor, d->anchor.x() * 2,
                                                 d->layout->orientation()));
    // Reset shift state machine, also see switchToMainView.
    d->shift_machine.restart();

    //d->shift_machine->start();
    emit layoutChanged(d->layout);
}

void LayoutUpdater::switchToSecondarySymView()
{
    Q_D(const LayoutUpdater);

    if (not verify(d->loader, d->layout)) {
        return;
    }

    d->layout->setCenterPanel(createFromKeyboard(d->loader->symbolsKeyboard(1),
                                                 d->anchor, d->anchor.x() * 2,
                                                 d->layout->orientation()));
    emit layoutChanged(d->layout);
}

} // namespace MaliitKeyboard
