/* * This file is part of meego-keyboard *
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

#ifndef MTOOLBARBUTTON_H
#define MTOOLBARBUTTON_H

#include <MButton>
/*!
 * \class MToolbarWidget
 * \brief MToolbarWidget is provide for the buttons in the input method toolbar.
 *
 * MToolbarWidget is inherit from MButton. It can use the icon which is not in current theme,
 * by setIcon() with the absolute file name of the icon. And the icon will be scaled according
 * setIconPercent() and button size.
 */
class QPixmap;

class MToolbarButton : public MButton
{
    Q_OBJECT
public:
    /*!
     * \Brief Constructor
     */
    explicit MToolbarButton(QGraphicsItem *parent = 0);

    //! Destructor
    virtual ~MToolbarButton();

    /*!
     * \Brief Sets the icon which is to be displayed on the button.
     * \param iconFile The absolute file name of the icon.
     */
    void setIconFile(const QString &iconFile);

    /*!
     * \Brief Sets the percentage of the icon size.
     * \param percent It is used to scale the icon, limit it inside button.
     *
     * This method scales the icon to limit it inside container(button). The scale is
     * relative to the container. After scaling, the icon is scaled to occupy \a percent
     * percentage of container's size. but its origin ratio is still kept.
     */
    void setIconPercent(int percent);

    //! \reimp
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                       QWidget *widget = 0);
    //! \reimp_end

private:
    QPixmap *icon;
    QString iconFile;
    int sizePercent;
    friend class Ut_MImToolbar;
};

#endif
