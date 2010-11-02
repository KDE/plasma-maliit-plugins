/* * This file is part of m-keyboard *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */

#ifndef MKEYBOARDSETTINGS_H
#define MKEYBOARDSETTINGS_H

#include <QObject>
#include <MWidget>
#include <MGConfItem>
#include <mabstractinputmethodsettings.h>

class QGraphicsWidget;

/*!
 * \brief MKeyboardSettings is the implemetation of meego-keyboard setting.
 * MKeyboardSettings implement MAbstractInputMethodSettings and create the meego-keyboard
 * setting. It provides below functionalities: get/set error corretion, get/set
 * installed (selected) keyboards.
 */
class MKeyboardSettings: public QObject, public MAbstractInputMethodSettings
{
    Q_OBJECT
    Q_DISABLE_COPY(MKeyboardSettings)
    friend class Ut_MKeyboardSettings;

public:
    MKeyboardSettings();

    ~MKeyboardSettings();

    //!\reimp
    virtual QString title();
    virtual QString icon();
    virtual QGraphicsWidget *createContentWidget(QGraphicsWidget *parent = 0);
    //! \reimp_end

    //! Reads all available keyboards
    void readAvailableKeyboards();

    //! Returns a map with layouts and titles for all available keyboards.
    QMap<QString, QString> availableKeyboards() const;

    //! Returns a map with layouts and titles for all selected keyboards.
    QMap<QString, QString> selectedKeyboards() const;

    //! Sets selected keyboards with \a keyboardLayouts.
    void setSelectedKeyboards(const QStringList &keyboardLayouts);

    //! Returns the boolean value of error correction option.
    bool errorCorrection() const;

    //! Sets error correction option.
    void setErrorCorrection(bool);

Q_SIGNALS:
    //! Emitted when selected keyboards are changed.
    void selectedKeyboardsChanged();

    //! Emitted when error correction option is changed.
    void errorCorrectionChanged();

private:
    QString keyboardTitle(const QString &layoutFile) const;
    QString keyboardLayoutFile(const QString &title) const;

    struct KeyboardInfo {
        QString layoutFile;
        QString title;
    };

    //! all available keyboards
    QList<KeyboardInfo> availableKeyboardInfos;
    MGConfItem keyboardErrorCorrectionConf;
    MGConfItem selectedKeyboardsConf;
};

#endif
