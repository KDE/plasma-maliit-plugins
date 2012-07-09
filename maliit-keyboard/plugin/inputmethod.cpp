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

#include "models/keyarea.h"
#include "models/wordribbon.h"

#include "logic/layout.h"
#include "logic/layoutupdater.h"
#include "logic/wordengine.h"
#include "logic/style.h"

#include "view/renderer.h"
#include "view/glass.h"
#include "view/setup.h"

#ifdef HAVE_QT_MOBILITY
#include "view/soundfeedback.h"
typedef MaliitKeyboard::SoundFeedback DefaultFeedback;
#else
#include "view/nullfeedback.h"
typedef MaliitKeyboard::NullFeedback DefaultFeedback;
#endif

#include <maliit/plugins/subviewdescription.h>
#include <maliit/plugins/abstractsurfacefactory.h>
#include <maliit/plugins/abstractpluginsetting.h>

#include <QApplication>
#include <QWidget>
#include <QDesktopWidget>

namespace MaliitKeyboard {

typedef QScopedPointer<Maliit::Plugins::AbstractPluginSetting> ScopedSetting;

class InputMethodPrivate
{
public:
    Maliit::Plugins::AbstractSurfaceFactory *surface_factory;
    Renderer renderer;
    Glass glass;
    Logic::LayoutUpdater layout_updater;
    Editor editor;
    DefaultFeedback feedback;
    Logic::Layout layout;
    SharedStyle style;
    ScopedSetting style_setting;
    ScopedSetting feedback_setting;
    ScopedSetting auto_correct_setting;

    explicit InputMethodPrivate(MAbstractInputMethodHost *host);

    void registerStyleSettings(MAbstractInputMethodHost *host);
    void registerFeedbackSettings(MAbstractInputMethodHost *host);
    void registerAutoCorrectSettings(MAbstractInputMethodHost *host);
};


InputMethodPrivate::InputMethodPrivate(MAbstractInputMethodHost *host)
    : surface_factory(host->surfaceFactory())
    , renderer()
    , glass()
    , layout_updater()
    , editor(EditorOptions(), new Model::Text, new Logic::WordEngine)
    , feedback()
    , layout(new Logic::Layout)
    , style(new Style)
    , style_setting()
    , feedback_setting()
    , auto_correct_setting()
{
    renderer.setSurfaceFactory(surface_factory);
    glass.setSurface(renderer.surface());
    glass.setExtendedSurface(renderer.extendedSurface());
    editor.setHost(host);

    glass.addLayout(&layout);
    layout_updater.setLayout(&layout);

    renderer.setStyle(style);
    layout_updater.setStyle(style);
    feedback.setStyle(style);

    const QSize &screen_size(surface_factory->screenSize());
    layout.setScreenSize(screen_size);
    layout.setAlignment(Logic::Layout::Bottom);
    layout_updater.setOrientation(screen_size.width() >= screen_size.height()
                                  ? Logic::Layout::Landscape : Logic::Layout::Portrait);

    registerStyleSettings(host);
    registerFeedbackSettings(host);
    registerAutoCorrectSettings(host);
}


void InputMethodPrivate::registerStyleSettings(MAbstractInputMethodHost *host)
{
    QVariantMap attributes;
    QStringList available_styles = style->availableProfiles();
    attributes[Maliit::SettingEntryAttributes::defaultValue] = MALIIT_DEFAULT_PROFILE;
    attributes[Maliit::SettingEntryAttributes::valueDomain] = available_styles;
    attributes[Maliit::SettingEntryAttributes::valueDomainDescriptions] = available_styles;

    style_setting.reset(host->registerPluginSetting("current_style",
                                                    QT_TR_NOOP("Keyboard style"),
                                                    Maliit::StringType,
                                                    attributes));
    style->setProfile(style_setting->value().toString());
}


void InputMethodPrivate::registerFeedbackSettings(MAbstractInputMethodHost *host)
{
    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = feedback.isEnabled();

    feedback_setting.reset(host->registerPluginSetting("feedback_enabled",
                                                       QT_TR_NOOP("Feedback enabled"),
                                                       Maliit::BoolType,
                                                       attributes));
    feedback.setEnabled(feedback_setting->value().toBool());
}


void InputMethodPrivate::registerAutoCorrectSettings(MAbstractInputMethodHost *host)
{
    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = editor.isAutoCorrectEnabled();

    auto_correct_setting.reset(host->registerPluginSetting("auto_correct_enabled",
                                                           QT_TR_NOOP("Auto-correct enabled"),
                                                           Maliit::BoolType,
                                                           attributes));

    editor.setAutoCorrectEnabled(auto_correct_setting->value().toBool());
}


InputMethod::InputMethod(MAbstractInputMethodHost *host)
    : MAbstractInputMethod(host)
    , d_ptr(new InputMethodPrivate(host))
{
    Q_D(InputMethod);

    Setup::connectAll(&d->glass, &d->layout, &d->layout_updater,
                      &d->renderer, &d->editor, &d->feedback);

    // TODO: Let this be driven through content type, and/or a plugin setting:
    d->editor.wordEngine()->setEnabled(true);
    d->editor.setAutoCorrectEnabled(false);

    connect(&d->glass, SIGNAL(keyboardClosed()),
            this,      SLOT(onKeyboardClosed()));

    connect(&d->glass, SIGNAL(switchLeft(Logic::Layout)),
            this,      SLOT(onLeftLayoutSelected()));

    connect(&d->glass, SIGNAL(switchRight(Logic::Layout)),
            this,      SLOT(onRightLayoutSelected()));

    connect(&d->editor, SIGNAL(leftLayoutSelected()),
            this,       SLOT(onLeftLayoutSelected()));

    connect(&d->editor, SIGNAL(rightLayoutSelected()),
            this,       SLOT(onRightLayoutSelected()));

    connect(d->surface_factory, SIGNAL(screenSizeChanged(QSize)),
            this,               SLOT(onScreenSizeChange(QSize)));

    connect(d->style_setting.data(), SIGNAL(valueChanged()),
            this,                    SLOT(onStyleSettingChanged()));

    connect(d->feedback_setting.data(), SIGNAL(valueChanged()),
            this,                       SLOT(onFeedbackSettingChanged()));
    connect(&d->feedback, SIGNAL(enabledChanged(bool)),
            this,        SLOT(onFeedbackEnabledChanged(bool)));

    connect(d->auto_correct_setting.data(), SIGNAL(valueChanged()),
            this,                           SLOT(onAutoCorrectSettingChanged()));
}

InputMethod::~InputMethod()
{}

void InputMethod::show()
{
    Q_D(InputMethod);
    d->renderer.show();
}

void InputMethod::hide()
{
    Q_D(InputMethod);
    d->renderer.hide();
    d->layout_updater.resetOnKeyboardClosed();
    d->editor.clearPreedit();
}

void InputMethod::setPreedit(const QString &preedit,
                             int cursor_position)
{
    Q_UNUSED(cursor_position)
    Q_D(InputMethod);
    d->editor.replacePreedit(preedit, AbstractTextEditor::ReplaceOnly);
}

void InputMethod::switchContext(Maliit::SwitchDirection direction,
                                bool animated)
{
    Q_UNUSED(direction)
    Q_UNUSED(animated)
}

QList<MAbstractInputMethod::MInputMethodSubView>
InputMethod::subViews(Maliit::HandlerState state) const
{
    Q_UNUSED(state)
    Q_D(const InputMethod);

    QList<MInputMethodSubView> views;

    Q_FOREACH (const QString &id, d->layout_updater.keyboardIds()) {
        MInputMethodSubView v;
        v.subViewId = id;
        v.subViewTitle = d->layout_updater.keyboardTitle(id);
        views.append(v);
    }

    return views;
}

void InputMethod::setActiveSubView(const QString &id,
                                   Maliit::HandlerState state)
{
    Q_UNUSED(state)
    Q_D(InputMethod);

    d->layout_updater.setActiveKeyboardId(id);
}

QString InputMethod::activeSubView(Maliit::HandlerState state) const
{
    Q_UNUSED(state)
    Q_D(const InputMethod);

    return d->layout_updater.activeKeyboardId();
}

void InputMethod::handleAppOrientationChanged(int angle)
{
    Q_D(InputMethod);
    d->layout_updater.setOrientation((angle == 0 || angle == 180)
                                     ? Logic::Layout::Landscape
                                     : Logic::Layout::Portrait);
}

void InputMethod::onLeftLayoutSelected()
{
    // This API smells real bad.
    const QList<MImSubViewDescription> &list =
        inputMethodHost()->surroundingSubViewDescriptions(Maliit::OnScreen);

    if (list.count() > 0) {
        Q_EMIT activeSubViewChanged(list.at(0).id());
    }
}

void InputMethod::onRightLayoutSelected()
{
    // This API smells real bad.
    const QList<MImSubViewDescription> &list =
        inputMethodHost()->surroundingSubViewDescriptions(Maliit::OnScreen);

    if (list.count() > 1) {
        Q_EMIT activeSubViewChanged(list.at(1).id());
    }
}

void InputMethod::onScreenSizeChange(const QSize &size)
{
    Q_D(InputMethod);

    d->layout.setScreenSize(size);
    d->layout_updater.setOrientation(size.width() >= size.height()
                                     ? Logic::Layout::Landscape
                                     : Logic::Layout::Portrait);
}

void InputMethod::onStyleSettingChanged()
{
    Q_D(InputMethod);
    d->style->setProfile(d->style_setting->value().toString());
}

void InputMethod::onKeyboardClosed()
{
    hide();
    inputMethodHost()->notifyImInitiatedHiding();
}

void InputMethod::onFeedbackSettingChanged()
{
    Q_D(InputMethod);
    d->feedback.setEnabled(d->feedback_setting->value().toBool());
}

void InputMethod::onFeedbackEnabledChanged(bool enabled)
{
    Q_D(InputMethod);
    if (d->feedback_setting->value().toBool() != enabled) {
        d->feedback_setting->set(enabled);
    }
}

void InputMethod::onAutoCorrectSettingChanged()
{
    Q_D(InputMethod);
    d->editor.setAutoCorrectEnabled(d->auto_correct_setting->value().toBool());
}

} // namespace MaliitKeyboard
