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

#include "renderer/renderer.h"
#include "models/keyarea.h"
#include "models/key.h"
#include "models/keylabel.h"

#include <QtCore>
#include <QtGui>

int main(int argc,
         char ** argv)
{
    QApplication app(argc, argv);

    QWidget *window = new QWidget;
    window->resize(480, 854);
    window->showFullScreen();

    MaliitKeyboard::Renderer renderer;
    renderer.setWindow(window);

    QPixmap pm(40, 60);
    pm.fill(Qt::lightGray);

    MaliitKeyboard::SharedFont font(new QFont);
    font->setBold(true);
    font->setPointSize(16);

    MaliitKeyboard::KeyArea ka0;
    ka0.id = 0;
    ka0.rect = QRectF(0, 554, 480, 300);

    MaliitKeyboard::KeyLabel label0;
    label0.setRect(QRect(5, 5, 20, 40));
    label0.setText("Q");
    label0.setFont(font);
    label0.setColor(Qt::darkBlue);

    MaliitKeyboard::Key key0;
    key0.setRect(QRect(10, 10, 40, 60));
    key0.setBackground(pm);
    key0.setLabel(label0);
    ka0.keys.append(key0);

    MaliitKeyboard::KeyLabel label1;
    label1.setRect(QRect(5, 5, 20, 40));
    label1.setText("W");
    label1.setFont(font);
    label1.setColor(Qt::darkMagenta);

    MaliitKeyboard::Key key1;
    key1.setRect(QRect(60, 10, 40, 60));
    key1.setBackground(pm);
    key1.setLabel(label1);
    ka0.keys.append(key1);

    renderer.show(ka0);

    MaliitKeyboard::KeyArea ka1;
    ka1.id = 1;
    ka1.rect = QRectF(0, 0, 480, 100);
    renderer.show(ka1);

    return app.exec();
}
