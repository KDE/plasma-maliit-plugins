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



#ifndef MIMFONTPOOL_H
#define MIMFONTPOOL_H

#include <QSharedData>
#include <QExplicitlySharedDataPointer>
#include <QFont>

//! Helper class which shares font information among different objects.
class MImKeyFontData : public QSharedData
{
public:
    //! Constructor
    //! \param font Font information
    explicit MImKeyFontData(const QFont &font);

    //! Destructor
    ~MImKeyFontData();

    //! Accessor method for contained font object
    QFont *font();

private:
    Q_DISABLE_COPY(MImKeyFontData)

    //! Font information
    QFont m_font;
};

typedef QExplicitlySharedDataPointer<MImKeyFontData> MImSharedKeyFontData;

/*!
 * \class MImFontPool
 * \brief The MImFontPool class decides which keys should
 * share the same font and which should have their own.
 */
class MImFontPool
{
public:
    /*! Default constructor.
     * \param newAllowFontSharing If false, keys will always get dedicated font object.
     *
     * \sa setDefaultFont(const QFont &)
     */
    MImFontPool(bool newAllowFontSharing);

    //! Destructor
    ~MImFontPool();

    //! Return font from this pool.
    //! \param shared Set this param to true if you want to get shared font object,
    //! otherwise dedicated font object will be returned.
    MImSharedKeyFontData font(bool shared = false);

    //! Set shared font object to default value
    //! \sa setDefaultFont(const QFont &)
    void reset();

    //! Change default font to given \a font.
    void setDefaultFont(const QFont &font);

private:
    Q_DISABLE_COPY(MImFontPool);

    //! Shared font information
    MImSharedKeyFontData sharedFont;

    //! Default font.
    QFont defaultFont;

    //! If false, font(bool) method will always return dedicated font object
    //! regardless to it's parameter
    bool allowFontSharing;
};

#endif

