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

#include "mtoolbarbutton.h"
#include "mtoolbarbuttonview.h"
#include <mtoolbaritem.h>
#include <MButton>
#include <QFileInfo>
#include <QPixmap>
#include <QDebug>

namespace {
    const char * const IconButtonStyleName  = "MToolbarIconButton";
    const char * const TextButtonStyleName  = "MToolbarTextButton";
    const char * const HighlightedStyleNamePostfix = "Highlighted";
}

MToolbarButton::MToolbarButton(QSharedPointer<MToolbarItem> item,
                               QGraphicsItem *parent)
    : MButton(parent),
      icon(0),
      sizePercent(100),
      itemPtr(item)
{
    MToolbarButtonView* view = new MToolbarButtonView(this);

    setView(view);

    updateStyleName();
    // Store the original minimum size coming from the style
    originalMinSize = minimumSize();

    if (!item->name().isEmpty()) {
        setObjectName(item->name());
    }

    if (!item->iconId().isEmpty()){
        setIconID(item->iconId());
    } else {
        sizePercent = itemPtr->size();
        setIconFile(itemPtr->icon());
    }

    if (!item->textId().isEmpty()) {
        setText(qtTrId(itemPtr->textId().toUtf8().data()));
    }
    if (!itemPtr->text().isEmpty()) {
        setText(itemPtr->text());
    }
    setCheckable(item->toggle());
    if (itemPtr->toggle()) {
        setChecked(itemPtr->pressed());

        connect(this, SIGNAL(clicked(bool)),
                itemPtr.data(), SLOT(setPressed(bool)));
    }
    setEnabled(itemPtr->enabled());
    setVisible(item->isVisible());

    connect(this, SIGNAL(clicked(bool)),
            this, SLOT(onClick()));
    connect(itemPtr.data(), SIGNAL(propertyChanged(const QString&)),
            this, SLOT(updateData(const QString&)));
}

MToolbarButton::~MToolbarButton()
{
    delete icon;
    icon = 0;
}

void MToolbarButton::setIconFile(const QString &newIconFile)
{
    if (iconFile == newIconFile)
        return;

    if (icon) {
        delete icon;
        icon = 0;
        iconFile.clear();
    }

    QFileInfo fileInfo(newIconFile);
    if (fileInfo.exists() && fileInfo.isAbsolute() && fileInfo.isFile()) {
        icon = new QPixmap(newIconFile);
        if (icon) {
            iconFile = newIconFile;
        }
    }
}

void MToolbarButton::setIconPercent(int percent)
{
    if (percent != sizePercent) {
        sizePercent = percent;
    }
}

void MToolbarButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    MButton::paint(painter, option, widget);
}

QSharedPointer<MToolbarItem> MToolbarButton::item()
{
    return itemPtr;
}

void MToolbarButton::updateStyleName()
{
    if (!item()) {
        return;
    }

    // Use default style for the standard item.
    // Otherwise use icon style for icon item,
    // text style for text item.
    if (!item()->isCustom()) {
        setStyleName(QString());
    } else {
        QString styleName;

        if (item()->text().isEmpty() && item()->textId().isEmpty()) {
            styleName = IconButtonStyleName;
        } else {
            styleName = TextButtonStyleName;
        }
        if (item()->highlighted()) {
            styleName.append(HighlightedStyleNamePostfix);
        }
        setStyleName(styleName);
    }
}

void MToolbarButton::updateData(const QString &attribute)
{
    if (attribute == "icon") {
        setIconFile(itemPtr->icon());
    } else if (attribute == "iconId") {
        setIconID(itemPtr->iconId());
    } else if (attribute == "text") {
        setText(itemPtr->text());
    } else if (attribute == "textId") {
        setText(qtTrId(itemPtr->textId().toUtf8().data()));
    } else if (attribute == "pressed" && itemPtr->toggle()) {
        setChecked(itemPtr->pressed());
    } else if (attribute == "enabled") {
        setEnabled(itemPtr->enabled());
    } else if (attribute == "visible") {
        setVisible(itemPtr->isVisible());
        emit availabilityChanged();
    } else if (attribute == "size") {
        sizePercent = itemPtr->size();
        update();
    }
    // highlighting is handled by styling
    updateStyleName();
}

void MToolbarButton::setText(const QString &text)
{
    if (text.isEmpty())
        return;
    // Do what needs to be done in MButton
    MButton::setText(text);
    // Update the preferred size
    setPreferredSize(((MToolbarButtonView*)view())->optimalSize(maximumSize()));
    // If the preferred width is smaller than the minimal width than set the preferred
    // size as minimal size. It is a useful tactics when the space is limited (e.g
    // portrait mode), but the button does not need the original minimal size.
    if (originalMinSize.width() > preferredSize().width())
        setMinimumSize(preferredSize());
    else
        // Restore the original minimum size if needed
        setMinimumSize(originalMinSize);
}

void MToolbarButton::onClick()
{
    emit clicked(itemPtr.data());
}

