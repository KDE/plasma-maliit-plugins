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
#include <QSharedPointer>

/*!
 * \class MToolbarWidget
 * \brief MToolbarWidget is provide for the buttons in the input method toolbar.
 *
 * MToolbarWidget inherits from MButton. It can not only use the iconID, but also use the icon
 * which is not in current theme, by setIcon() with the absolute file name of the icon, and the
 * icon will be scaled according setIconPercent() and button size.
 */
class QPixmap;
class MToolbarItem;

class MToolbarButton : public MButton
{
    Q_OBJECT
    Q_DISABLE_COPY(MToolbarButton)

public:
    /*!
     * \Brief Constructor
     */
    explicit MToolbarButton(QSharedPointer<MToolbarItem> item, QGraphicsItem *parent = 0);

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

    //! Return pointer to corresponding toolbar item.
    QSharedPointer<MToolbarItem> item();

    //! Select style name depending on item attributes.
    void updateStyleName();

signals:
    /*!
     * \brief Emitted when button is clicked.
     * \param item Pointer to corresponding toolbar item.
     *
     * Warning: do not store pointer which is used as parameter for this signal,
     * call MToolbarItem::item() if you need to get pointer to toolbar item.
     */
    void clicked(MToolbarItem *item);

    //! \brief Emitted when visibility (in a sense of the button being available
    //! or not) changes.
    void availabilityChanged();

private slots:
    //! Update button's properties when properties of toolbar item are updated.
    void updateData(const QString &attribute);

    //! Emits clicked(MToolbarItem *) when base class emits clicked(bool)
    void onClick();

private:
    QPixmap *icon;
    QString iconFile;
    int sizePercent;
    QSharedPointer<MToolbarItem> itemPtr;

    friend class Ut_MImToolbar;
};

#endif
