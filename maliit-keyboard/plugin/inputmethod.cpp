/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 * Copyright (C) 2012-2013 Canonical Ltd
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
#include "updatenotifier.h"

#include "models/key.h"
#include "models/keyarea.h"
#include "models/wordribbon.h"
#include "models/layout.h"

#include "logic/layouthelper.h"
#include "logic/layoutupdater.h"
#include "logic/wordengine.h"
#include "logic/style.h"
#include "logic/languagefeatures.h"
#include "logic/maliitcontext.h"

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
#include <maliit/plugins/quickviewsurface.h>
#include <maliit/plugins/abstractpluginsetting.h>
#include <maliit/plugins/updateevent.h>

#include <QApplication>
#include <QWidget>
#include <QDesktopWidget>
#include <QtQuick>

class MImUpdateEvent;

namespace MaliitKeyboard {

typedef QScopedPointer<Maliit::Plugins::AbstractPluginSetting> ScopedSetting;
typedef QSharedPointer<MKeyOverride> SharedOverride;
typedef QMap<QString, SharedOverride>::const_iterator OverridesIterator;

namespace {

const Maliit::Plugins::AbstractSurface::Options g_surface_options(
    Maliit::Plugins::AbstractSurface::TypeQuick2 | Maliit::Plugins::AbstractSurface::PositionCenterBottom
);

const QString g_maliit_keyboard_qml(MALIIT_KEYBOARD_DATA_DIR "/maliit-keyboard.qml");

Key overrideToKey(const SharedOverride &override)
{
    Key key;

    key.rLabel().setText(override->label());
    key.setIcon(override->icon().toUtf8());
    // TODO: hightlighted and enabled information are not available in
    // Key. Should we just really create a KeyOverride model?

    return key;
}

} // unnamed namespace

class Settings
{
public:
    ScopedSetting style;
    ScopedSetting feedback;
    ScopedSetting auto_correct;
    ScopedSetting auto_caps;
    ScopedSetting word_engine;
    ScopedSetting hide_word_ribbon_in_portrait_mode;
};


class InputMethodPrivate
{
public:
    Maliit::Plugins::AbstractSurfaceFactory *const surface_factory;
    QSharedPointer<Maliit::Plugins::QuickViewSurface> surface;
    Editor editor;
    DefaultFeedback feedback;
    SharedStyle style;
    UpdateNotifier notifier;
    QMap<QString, SharedOverride> key_overrides;
    Settings settings;
    Logic::LayoutHelper layout_helper;
    Logic::LayoutUpdater layout_updater;
    QScopedPointer<Model::Layout> layout;
    Logic::LayoutHelper extended_layout_helper;
    Logic::LayoutUpdater extended_layout_updater;
    QScopedPointer<Model::Layout> extended_layout;
    QScopedPointer<Logic::MaliitContext> context;

    explicit InputMethodPrivate(MAbstractInputMethodHost *host);
    void setLayoutOrientation(Logic::LayoutHelper::Orientation orientation);
    void syncWordEngine(Logic::LayoutHelper::Orientation orientation);

    void connectToNotifier();
};


InputMethodPrivate::InputMethodPrivate(MAbstractInputMethodHost *host)
    : surface_factory(host->surfaceFactory())
    , surface(qSharedPointerDynamicCast<Maliit::Plugins::QuickViewSurface>(surface_factory->create(g_surface_options)))
    , editor(EditorOptions(), new Model::Text, new Logic::WordEngine, new Logic::LanguageFeatures)
    , feedback()
    , style(new Style)
    , notifier()
    , key_overrides()
    , settings()
    , layout_helper()
    , layout_updater()
    , layout(new Model::Layout(&layout_updater))
    , extended_layout_helper()
    , extended_layout_updater()
    , extended_layout(new Model::Layout(&extended_layout_updater))
    , context(new Logic::MaliitContext(style))
{
    editor.setHost(host);

    layout_updater.setLayout(&layout_helper);
    extended_layout_updater.setLayout(&extended_layout_helper);

    layout_updater.setStyle(style);
    extended_layout_updater.setStyle(style);
    feedback.setStyle(style);

    const QSize &screen_size(surface_factory->screenSize());
    layout_helper.setScreenSize(screen_size);
    layout_helper.setAlignment(Logic::LayoutHelper::Bottom);
    extended_layout_helper.setScreenSize(screen_size);
    extended_layout_helper.setAlignment(Logic::LayoutHelper::Floating);

    layout->setLayout(&layout_helper);
    extended_layout->setLayout(&extended_layout_helper);

    connectToNotifier();

    QQmlEngine *const engine(surface->view()->engine());
    engine->addImportPath(MALIIT_KEYBOARD_DATA_DIR);
    engine->rootContext()->setContextProperty("maliit", context.data());
    engine->rootContext()->setContextProperty("maliit_layout", layout.data());
    engine->rootContext()->setContextProperty("maliit_extended_layout", extended_layout.data());

    surface->view()->setSource(QUrl::fromLocalFile(g_maliit_keyboard_qml));
}


void InputMethodPrivate::setLayoutOrientation(Logic::LayoutHelper::Orientation orientation)
{
    syncWordEngine(orientation);
    layout_updater.setOrientation(orientation);
    extended_layout_updater.setOrientation(orientation);
}


void InputMethodPrivate::syncWordEngine(Logic::LayoutHelper::Orientation orientation)
{
    // hide_word_ribbon_in_potrait_mode_setting overrides word_engine_setting:
#ifndef DISABLE_PREEDIT
    const bool override_activation(settings.hide_word_ribbon_in_portrait_mode->value().toBool()
                                   && orientation == Logic::LayoutHelper::Portrait);
#else
    Q_UNUSED(orientation)
    const bool override_activation = true;
#endif

    editor.wordEngine()->setEnabled(override_activation
                                    ? false
                                    : settings.word_engine->value().toBool());
}

void InputMethodPrivate::connectToNotifier()
{
    QObject::connect(&notifier, SIGNAL(cursorPositionChanged(int, QString)),
                     &editor,   SLOT(onCursorPositionChanged(int, QString)));

    QObject::connect(&notifier, SIGNAL(keysOverriden(Logic::KeyOverrides, bool)),
                     &layout_helper,   SLOT(onKeysOverriden(Logic::KeyOverrides, bool)));
}

InputMethod::InputMethod(MAbstractInputMethodHost *host)
    : MAbstractInputMethod(host)
    , d_ptr(new InputMethodPrivate(host))
{
    Q_D(InputMethod);

    // FIXME: Reconnect feedback instance.
    Setup::connectAll(d->layout.data(), &d->layout_updater, &d->editor);
    QObject::connect(&d->layout_helper, SIGNAL(centerPanelChanged(KeyArea,Logic::KeyOverrides)),
                     d->layout.data(), SLOT(setKeyArea(KeyArea)));

    // FIXME: Reimplement keyboardClosed, switchLeft and switchRight
    // (triggered by glass).

    connect(&d->editor, SIGNAL(rightLayoutSelected()),
            this,       SLOT(onRightLayoutSelected()));

    connect(d->surface_factory, SIGNAL(screenSizeChanged(QSize)),
            this,               SLOT(onScreenSizeChange(QSize)));

    registerStyleSetting(host);
    registerFeedbackSetting(host);
    registerAutoCorrectSetting(host);
    registerAutoCapsSetting(host);
    registerWordEngineSetting(host);
    registerHideWordRibbonInPortraitModeSetting(host);

    // Setting layout orientation depends on word engine and hide word ribbon
    // settings to be initialized first:
    const QSize &screen_size(d->surface_factory->screenSize());
    d->setLayoutOrientation(screen_size.width() >= screen_size.height()
                            ? Logic::LayoutHelper::Landscape : Logic::LayoutHelper::Portrait);
}

InputMethod::~InputMethod()
{}

void InputMethod::show()
{
    Q_D(InputMethod);
    d->surface->setSize(QSize(d->layout->width(),
                              d->layout->height()));

    d->surface->show();
}

void InputMethod::hide()
{
    Q_D(InputMethod);
    d->layout_updater.resetOnKeyboardClosed();
    d->editor.clearPreedit();
    d->surface->hide();
}

void InputMethod::setPreedit(const QString &preedit,
                             int cursor_position)
{
    Q_UNUSED(cursor_position)
    Q_D(InputMethod);
    d->editor.replacePreedit(preedit);
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
    d->setLayoutOrientation((angle == 0 || angle == 180)
                            ? Logic::LayoutHelper::Landscape
                            : Logic::LayoutHelper::Portrait);
}

bool InputMethod::imExtensionEvent(MImExtensionEvent *event)
{
    Q_D(InputMethod);

    if (not event or event->type() != MImExtensionEvent::Update) {
        return false;
    }

    MImUpdateEvent *update_event(static_cast<MImUpdateEvent *>(event));

    d->notifier.notify(update_event);

    return true;
}


void InputMethod::registerStyleSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    QStringList available_styles = d->style->availableProfiles();
    attributes[Maliit::SettingEntryAttributes::defaultValue] = MALIIT_DEFAULT_PROFILE;
    attributes[Maliit::SettingEntryAttributes::valueDomain] = available_styles;
    attributes[Maliit::SettingEntryAttributes::valueDomainDescriptions] = available_styles;

    d->settings.style.reset(host->registerPluginSetting("current_style",
                                                        QT_TR_NOOP("Keyboard style"),
                                                        Maliit::StringType,
                                                        attributes));

    connect(d->settings.style.data(), SIGNAL(valueChanged()),
            this,                     SLOT(onStyleSettingChanged()));

    // Call manually for the first time to initialize dependent values:
    onStyleSettingChanged();
}


void InputMethod::registerFeedbackSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = true;

    d->settings.feedback.reset(host->registerPluginSetting("feedback_enabled",
                                                           QT_TR_NOOP("Feedback enabled"),
                                                           Maliit::BoolType,
                                                           attributes));

    connect(d->settings.feedback.data(), SIGNAL(valueChanged()),
            this,                        SLOT(onFeedbackSettingChanged()));

    d->feedback.setEnabled(d->settings.feedback->value().toBool());
}


void InputMethod::registerAutoCorrectSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = true;

    d->settings.auto_correct.reset(host->registerPluginSetting("auto_correct_enabled",
                                                               QT_TR_NOOP("Auto-correct enabled"),
                                                               Maliit::BoolType,
                                                               attributes));

    connect(d->settings.auto_correct.data(), SIGNAL(valueChanged()),
            this,                            SLOT(onAutoCorrectSettingChanged()));

    d->editor.setAutoCorrectEnabled(d->settings.auto_correct->value().toBool());
}


void InputMethod::registerAutoCapsSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = true;

    d->settings.auto_caps.reset(host->registerPluginSetting("auto_caps_enabled",
                                                            QT_TR_NOOP("Auto-capitalization enabled"),
                                                            Maliit::BoolType,
                                                            attributes));

    connect(d->settings.auto_caps.data(), SIGNAL(valueChanged()),
            this,                         SLOT(onAutoCapsSettingChanged()));

    d->editor.setAutoCapsEnabled(d->settings.auto_caps->value().toBool());
}


void InputMethod::registerWordEngineSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = true;

    d->settings.word_engine.reset(host->registerPluginSetting("word_engine_enabled",
                                                              QT_TR_NOOP("Error correction/word prediction enabled"),
                                                              Maliit::BoolType,
                                                              attributes));

    connect(d->settings.word_engine.data(), SIGNAL(valueChanged()),
            this,                           SLOT(onWordEngineSettingChanged()));

#ifndef DISABLE_PREEDIT
    d->editor.wordEngine()->setEnabled(d->settings.word_engine->value().toBool());
#else
    d->editor.wordEngine()->setEnabled(false);
#endif
}

void InputMethod::registerHideWordRibbonInPortraitModeSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = false;

    d->settings.hide_word_ribbon_in_portrait_mode.reset(
        host->registerPluginSetting("hide_word_ribbon_in_potrait_mode",
                                    QT_TR_NOOP("Disable word engine in portrait mode"),
                                    Maliit::BoolType,
                                    attributes));

    connect(d->settings.hide_word_ribbon_in_portrait_mode.data(), SIGNAL(valueChanged()),
            this, SLOT(onHideWordRibbonInPortraitModeSettingChanged()));
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

    d->layout_helper.setScreenSize(size);
    d->extended_layout_helper.setScreenSize(d->layout_helper.screenSize());

    d->setLayoutOrientation(size.width() >= size.height()
                            ? Logic::LayoutHelper::Landscape : Logic::LayoutHelper::Portrait);
}

void InputMethod::onStyleSettingChanged()
{
    Q_D(InputMethod);
    d->style->setProfile(d->settings.style->value().toString());
    d->layout->setImageDirectory(d->style->directory(Style::Images));
}

void InputMethod::onKeyboardClosed()
{
    hide();
    inputMethodHost()->notifyImInitiatedHiding();
}

void InputMethod::onFeedbackSettingChanged()
{
    Q_D(InputMethod);
    d->feedback.setEnabled(d->settings.feedback->value().toBool());
}

void InputMethod::onAutoCorrectSettingChanged()
{
    Q_D(InputMethod);
    d->editor.setAutoCorrectEnabled(d->settings.auto_correct->value().toBool());
}

void InputMethod::onAutoCapsSettingChanged()
{
    Q_D(InputMethod);
    d->editor.setAutoCapsEnabled(d->settings.auto_caps->value().toBool());
}

void InputMethod::onWordEngineSettingChanged()
{
    // FIXME: Renderer doesn't seem to update graphics properly. Word ribbon
    // is still visible until next VKB show/hide.
    Q_D(InputMethod);
    d->syncWordEngine(d->layout_helper.orientation());
}

void InputMethod::onHideWordRibbonInPortraitModeSettingChanged()
{
    Q_D(InputMethod);
    d->setLayoutOrientation(d->layout_helper.orientation());
}

void InputMethod::setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides)
{
    Q_D(InputMethod);

    for (OverridesIterator i(d->key_overrides.begin()), e(d->key_overrides.end()); i != e; ++i) {
        const SharedOverride &override(i.value());

        if (override) {
            disconnect(override.data(), SIGNAL(keyAttributesChanged(const QString &, const MKeyOverride::KeyOverrideAttributes)),
                       this,            SLOT(updateKey(const QString &, const MKeyOverride::KeyOverrideAttributes)));
        }
    }

    d->key_overrides.clear();
    QMap<QString, Key> overriden_keys;

    for (OverridesIterator i(overrides.begin()), e(overrides.end()); i != e; ++i) {
        const SharedOverride &override(i.value());

        if (override) {
            d->key_overrides.insert(i.key(), override);
            connect(override.data(), SIGNAL(keyAttributesChanged(const QString &, const MKeyOverride::KeyOverrideAttributes)),
                    this,            SLOT(updateKey(const QString &, const MKeyOverride::KeyOverrideAttributes)));
            overriden_keys.insert(i.key(), overrideToKey(override));
        }
    }
    d->notifier.notifyOverride(overriden_keys);
}

void InputMethod::updateKey(const QString &key_id,
                            const MKeyOverride::KeyOverrideAttributes changed_attributes)
{
    Q_D(InputMethod);

    Q_UNUSED(changed_attributes);

    QMap<QString, SharedOverride>::iterator iter(d->key_overrides.find(key_id));

    if (iter != d->key_overrides.end()) {
        const Key &override_key(overrideToKey(iter.value()));
        Logic::KeyOverrides overrides_update;

        overrides_update.insert(key_id, override_key);
        d->notifier.notifyOverride(overrides_update, true);
    }
}

} // namespace MaliitKeyboard
