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

#include "inputmethod.h"
#include "editor.h"
#include "logic/layoutupdater.h"
#include "logic/keyboardloader.h"
#include "renderer/renderer.h"
#include "renderer/abstractbackgroundbuffer.h"
#include "glass/glass.h"
#include "models/keyarea.h"
#include "models/layout.h"

namespace MaliitKeyboard {

namespace {

MaliitKeyboard::Key createKey(const QPixmap &pm,
                              const MaliitKeyboard::SharedFont &f,
                              const QRect &kr,
                              const QRect &lr,
                              const QByteArray &t,
                              const QColor &c,
                              MaliitKeyboard::Key::Action a = MaliitKeyboard::Key::ActionCommit)
{
    MaliitKeyboard::KeyLabel l;
    l.setRect(lr);
    l.setText(t);
    l.setColor(c);
    l.setFont(f);

    MaliitKeyboard::Key k;
    k.setRect(kr);
    k.setBackground(pm);
    k.setLabel(l);
    k.setAction(a);

    return k;
}

MaliitKeyboard::KeyArea createKeyArea()
{
    typedef QByteArray QBA;

    QPixmap pm(8, 8);
    pm.fill(Qt::lightGray);

    MaliitKeyboard::SharedFont font(new QFont);
    font->setBold(true);
    font->setPointSize(16);

    MaliitKeyboard::KeyArea ka;
    ka.setRect(QRectF(0, 554, 480, 300));
    ka.appendKey(createKey(pm, font, QRect(10, 10, 40, 60),
                           QRect(5, 5, 20, 40), QBA("Q"), Qt::darkBlue));
    ka.appendKey(createKey(pm, font, QRect(60, 10, 80, 120),
                           QRect(5, 5, 70, 40), QBA("W"), Qt::darkMagenta));
    ka.appendKey(createKey(pm, font, QRect(10, 80, 40, 50),
                           QRect(5, 5, 20, 40), QBA("A"), Qt::black));
    ka.appendKey(createKey(pm, font, QRect(10, 140, 130, 60),
                           QRect(5, 5, 120, 40), QBA("shift"), Qt::darkCyan,
                           MaliitKeyboard::Key::ActionShift));

    return ka;
}

}

class BackgroundBuffer
    : public AbstractBackgroundBuffer
{
private:
    MAbstractInputMethodHost *m_host;

public:
    explicit BackgroundBuffer(MAbstractInputMethodHost *host)
        : AbstractBackgroundBuffer()
    {
        m_host = host;
    }

    virtual ~BackgroundBuffer()
    {}

    QPixmap background() const
    {
        if (not m_host) {
            static QPixmap empty;
            return empty;
        }

        return m_host->background();
    }
};

class InputMethodPrivate
{
public:
    QWidget *window;
    BackgroundBuffer buffer;
    Renderer renderer;
    Glass glass;
    LayoutUpdater layout_updater;
    KeyboardLoader loader;
    Editor editor;

    explicit InputMethodPrivate(MAbstractInputMethodHost *host,
                                QWidget *new_window)
        : window(new_window)
        , buffer(host)
        , renderer()
        , glass()
        , layout_updater()
        , editor()
    {
        if (qApp && qApp->desktop()) {
            window->resize(qApp->desktop()->screenGeometry().size());
        }

        renderer.setWindow(window);
        renderer.setBackgroundBuffer(&buffer);
        glass.setWindow(renderer.viewport());
        editor.setHost(host);
    }
};

InputMethod::InputMethod(MAbstractInputMethodHost *host,
                         QWidget *window)
    : MAbstractInputMethod(host, window)
    , d_ptr(new InputMethodPrivate(host, window))
{
    Q_D(InputMethod);

    connect(&d->glass,  SIGNAL(keyReleased(Key,SharedLayout)),
            &d->editor, SLOT(onKeyReleased(Key)));

    connect(&d->glass,          SIGNAL(keyPressed(Key,SharedLayout)),
            &d->layout_updater, SLOT(onKeyPressed(Key,SharedLayout)));

    connect(&d->glass,          SIGNAL(keyReleased(Key,SharedLayout)),
            &d->layout_updater, SLOT(onKeyReleased(Key,SharedLayout)));

    connect(&d->layout_updater, SIGNAL(layoutChanged(SharedLayout)),
            &d->renderer,       SLOT(onLayoutChanged(SharedLayout)));

    connect(&d->layout_updater, SIGNAL(keysChanged(SharedLayout)),
            &d->renderer,       SLOT(onKeysChanged(SharedLayout)));
}

InputMethod::~InputMethod()
{}

void InputMethod::show()
{
    Q_D(InputMethod);

    SharedLayout layout(new Layout);
    layout->setCenterPanel(createKeyArea());

    d->renderer.addLayout(layout);
    d->renderer.show();
    d->glass.addLayout(layout);
    d->layout_updater.setLayout(layout);

    // FIXME: Region can change, for example when showing extended keys.
    inputMethodHost()->setInputMethodArea(d->renderer.region());
    inputMethodHost()->setScreenRegion(d->renderer.region());
}

void InputMethod::hide()
{
    Q_D(InputMethod);

    d->renderer.clearLayouts();
    d->glass.clearLayouts();

    inputMethodHost()->setInputMethodArea(d->renderer.region());
    inputMethodHost()->setScreenRegion(d->renderer.region());
}

void InputMethod::switchContext(MInputMethod::SwitchDirection direction,
                                bool animated)
{
    Q_UNUSED(direction)
    Q_UNUSED(animated)
}

QList<MAbstractInputMethod::MInputMethodSubView>
InputMethod::subViews(MInputMethod::HandlerState state) const
{
    Q_UNUSED(state)
    Q_D(const InputMethod);

    QList<MInputMethodSubView> views;
    foreach (const QString &id, d->loader.ids()) {
        MInputMethodSubView v;
        v.subViewId = id;
        v.subViewTitle = d->loader.title(id);
        views.append(v);
    }

    return views;
}

void InputMethod::setActiveSubView(const QString &id,
                                   MInputMethod::HandlerState state)
{
    Q_UNUSED(state)
    Q_D(InputMethod);
    d->loader.setActiveId(id);
}

QString InputMethod::activeSubView(MInputMethod::HandlerState state) const
{
    Q_UNUSED(state)
    Q_D(const InputMethod);
    return d->loader.activeId();
}

} // namespace MaliitKeyboard
