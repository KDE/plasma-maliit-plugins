/*
 * This file is part of meego-keyboard 
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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

 */



#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "notificationstyle.h"

#include <MStylableWidget>
#include <QTimeLine>
#include <QTimer>
#include <QStaticText>
#include <QPointer>

class QGraphicsSceneResizeEvent;
class PanParameters;
/*!
 * \class Notification
 * \brief Notification is used for textual notification.
 *
 *  Using this class to show textual notification on the virtual keyboard
 */
class Notification : public MStylableWidget
{
    Q_OBJECT

public:

    /*!
     * \brief Constructor for creating notification object.
     * \param parent QGraphicsWidget.
     */
    explicit Notification(QGraphicsItem *parent);

    //! Destructor
    ~Notification();

    //! Displays given text a short period of time, centered in area.
    void displayText(const QString &msg, const QRectF &area);

    //! Sets the display text.
    void setText(const QString &text);

    //! Returns the display text.
    QString text() const;

    void setMaximumTextWidth(qreal textWidth);

    /*!
     * \brief connect updating (position, scale, opacity) slots with signals from \a parameters.
     */
    void connectPanParameters(PanParameters *parameters);

protected:
    //! \reimp
    virtual void applyStyle();
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *);
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    //! \reimp_end

public slots:
    void updatePos(const QPointF &pos);
    void updateOpacity(qreal opacity);
    void updateScale(qreal scale);

private slots:
    //! Method to update the opacity.
    //! \param frameNumber Current frame number provided by fadeTimeLine.
    void updateOpacityByFrame(int frameNumber);

    //! Helper method to handle the end of both fades.
    void fadingFinished();

    //! This fades out and hides the widget.
    void fadeOut();

private:
    //! This shows the widget by fading in.
    void fadeIn();

    //! Break the message text into more lines if needed and set the geometry
    void setMessageAndGeometry(const QString &msg, const QRectF &area);

    //! update text layout
    void reLayout();

private:
    //! Timeline for animating fade in and out
    QTimeLine fadeTimeLine;

    //! Timer that determines the visibility time of notification excluding fade times.
    QTimer visibilityTimer;

    //! Notification text with correct line breaks
    QString message;

    //! the font used
    QFont font;

    //! CSS attributes
    QColor border;
    QColor background;
    QColor textColor;
    Qt::Alignment textHorizontalAlignment;
    Qt::Alignment textVerticalAlignment;
    bool textWrap;
    qreal mOpacity;
    int frameCount;
    QStaticText *textLayout;
    qreal maximumTextWidth;
    QSize maximumTextSize;
    bool dirty;
    QPointer<PanParameters> mParameters;

private:
    M_STYLABLE_WIDGET(NotificationStyle)

#ifdef UNIT_TEST
    friend class Ut_Notification;
#endif
};

#endif
