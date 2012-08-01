#ifndef UTILS_H
#define UTILS_H

#include <QString>

namespace MaliitKeyboard {
    class Key;

    namespace CoreUtils {
        const QString &pluginDataDirectory();
        const QString &maliitKeyboardDataDirectory();

        QString idFromKey(const Key &key);
    }
}

#endif // UTILS_H
