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
#include "renderer/renderer.h"
#include "renderer/abstractbackgroundbuffer.h"
#include "models/keyarea.h"
#include "models/layout.h"

namespace MaliitKeyboard {

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

    explicit InputMethodPrivate(MAbstractInputMethodHost *host,
                                QWidget *new_window)
        : window(new_window)
        , buffer(host)
        , renderer()
    {
        if (qApp && qApp->desktop()) {
            window->resize(qApp->desktop()->screenGeometry().size());
        }

        renderer.setWindow(window);
        renderer.setBackgroundBuffer(&buffer);
    }
};

InputMethod::InputMethod(MAbstractInputMethodHost *host,
                         QWidget *window)
    : MAbstractInputMethod(host, window)
    , d_ptr(new InputMethodPrivate(host, window))
{}

InputMethod::~InputMethod()
{}

void InputMethod::show()
{
    Q_D(InputMethod);

    KeyArea ka;
    ka.setRect(QRectF(0, 554, 480, 300));

    SharedLayout layout(new Layout);
    layout->setCenterPanel(ka);
    d->renderer.show(layout);

    inputMethodHost()->setInputMethodArea(d->renderer.region());
    inputMethodHost()->setScreenRegion(d->renderer.region());
}

void InputMethod::hide()
{
    Q_D(InputMethod);

    d->renderer.hideAll();
    inputMethodHost()->setInputMethodArea(d->renderer.region());
    inputMethodHost()->setScreenRegion(d->renderer.region());
}

} // namespace MaliitKeyboard
