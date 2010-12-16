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



#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <MWidget>
#include <QTimeLine>
#include <QTimer>

class MVirtualKeyboardStyleContainer;


/*!
 * \class Notification
 * \brief Notification is used for textual notification.
 *
 *  Using this class to show textual notification on the virtual keyboard
 */
class Notification : public MWidget
{
    Q_OBJECT

public:

    /*!
     * \brief Constructor for creating notification object.
     * \param parent QGraphicsWidget.
     */
    Notification(const MVirtualKeyboardStyleContainer *style, QGraphicsWidget *parent);

    //! Destructor
    ~Notification();

    //! \reimp
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *);
    //! \reimp_end

    //! Displays given text a short period of time, centered in area.
    void displayText(const QString &msg, const QRectF &area);

private slots:
    //! Method to update the opacity
    void updateOpacity(int);

    //! Helper method to handle the end of both fades.
    void fadingFinished();

    //! This fades out and hides the widget.
    void fadeOut();

    //! Retrieve information from CSS
    void getStyleValues();

private:
    //! This shows the widget by fading in.
    void fadeIn();

    //! Getter for style container
    const MVirtualKeyboardStyleContainer &style() const;

    //! Break the message text into more lines if needed and set the geometry
    void setMessageAndGeometry(const QString &msg, const QRectF &area);

private:
    //! Timeline for animating fade in and out
    QTimeLine fadeTimeLine;

    //! Timer that determines the visibility time of notification excluding fade times.
    QTimer visibilityTimer;

    //! Notification text with correct line breaks
    QString message;

    //! the font used
    QFont font;

    //! CSS style container
    const MVirtualKeyboardStyleContainer *styleContainer;

    //! CSS attributes
    QColor border;
    QColor background;
    QColor textColor;
    qreal opacity;

#ifdef UNIT_TEST
    friend class Ut_Notification;
#endif
};

#endif
