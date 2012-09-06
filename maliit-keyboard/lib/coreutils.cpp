
#include "coreutils.h"

#include "models/key.h"

namespace MaliitKeyboard {
namespace CoreUtils {
namespace {

const char *const g_action_key_id = "actionKey";

} // unnamed namespace

const QString &pluginDataDirectory() {
    static QString data_directory;

    if (data_directory.isNull()) {
        QByteArray env_data_directory = qgetenv("MALIIT_PLUGINS_DATADIR");
        if (env_data_directory.isEmpty()) {
            data_directory = QString::fromUtf8(MALIIT_PLUGINS_DATA_DIR);
        } else {
            data_directory = QString::fromUtf8(env_data_directory);
        }
    }

    return data_directory;
}

const QString &maliitKeyboardDataDirectory() {
    static QString data_directory;

    if (data_directory.isNull()) {
        QByteArray env_data_directory = qgetenv("MALIIT_KEYBOARD_DATADIR");
        if (env_data_directory.isEmpty()) {
            data_directory = QString::fromUtf8(MALIIT_KEYBOARD_DATA_DIR);
        } else {
            data_directory = QString::fromUtf8(env_data_directory);
        }
    }

    return data_directory;
}

QString idFromKey(const Key &key)
{
    switch (key.action()) {
    case Key::ActionReturn:
        return g_action_key_id;

    case Key::ActionInsert:
        return key.label().text();

    default:
        // TODO: handle more key actions if needed.
        return QString();
    }
}

}
}
