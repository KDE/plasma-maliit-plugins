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

#ifndef MALIIT_KEYBOARD_LAYOUT_H
#define MALIIT_KEYBOARD_LAYOUT_H

#include "keyarea.h"
#include <QtCore>

namespace MaliitKeyboard {

class Layout;
typedef QSharedPointer<Layout> SharedLayout;

class Layout
{
private:
    KeyArea m_left;
    KeyArea m_right;
    KeyArea m_center;
    KeyArea m_extended;

public:
    enum Panel {
        LeftPanel,
        RightPanel,
        CenterPanel,
        ExtendedPanel,
        PanelCount
    };

    explicit Layout();

    KeyArea lookup(Panel panel) const;

    KeyArea leftPanel() const;
    void setLeftPanel(const KeyArea &left);

    KeyArea rightPanel() const;
    void setRightPanel(const KeyArea &right);

    KeyArea centerPanel() const;
    void setCenterPanel(const KeyArea &center);

    KeyArea extendedPanel() const;
    void setExtendedPanel(const KeyArea &extended);

    void appendActiveKey(Panel panel,
                         const Key &key);

    void removeActiveKey(Panel panel,
                         const Key &key);

private:
    KeyArea * internalLookup(Panel panel);
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_LAYOUT_H
