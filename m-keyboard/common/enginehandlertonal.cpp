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

#include "mabstractinputmethodhost.h"
#include "enginemanager.h"
#include "abstractengine.h"
#include "abstractenginewidgethost.h"
#include "keyevent.h"
#include "mkeyboardhost.h"
#include "enginehandlertonal.h"

#include <mimenginefactory.h>
#include <QTextBoundaryFinder>

namespace
{
    const QString ThaiCategoryTONE(QString("%1%2%3%4").arg(QChar(0x0E48)).arg(QChar(0x0E49)).arg(QChar(0x0E4A)).arg(QChar(0x0E4B)));
    const QString ThaiCategoryAD1(QString("%1%2").arg(QChar(0x0E4C)).arg(QChar(0x0E4D)));
    const QString ThaiCategoryAD2(QChar(0x0E47));
    const QString ThaiCategoryABV1(QString("%1%2").arg(QChar(0x0E34)).arg(QChar(0x0E38)));
    const QString ThaiCategoryABV2(QString("%1%2%3").arg(QChar(0x0E31)).arg(QChar(0x0E36)).arg(QChar(0x0E39)));
    const QString ThaiCategoryAV3(QString("%1%2").arg(QChar(0x0E35)).arg(QChar(0x0E37)));

    const QString VietnameseVowels(QString::fromUtf8("aeiouyăâêôơư"));
    const QString VietnameseVowelsWithDiacritics(QString::fromUtf8("ăâêôơư"));
    const QString VietnameseFinalConsonants("cmnt");
    const QString VietnameseTones(QString("%1%2%3%4%5").arg(QChar(0x0301)).arg(QChar(0x0300)).arg(QChar(0x0309)).arg(QChar(0x0303)).arg(QChar(0x0323)));
}

EngineHandlerTonal::EngineHandlerTonal(MKeyboardHost &keyboardHost)
   : EngineHandlerDefault(keyboardHost)
{
}

EngineHandlerTonal::~EngineHandlerTonal()
{
}

bool EngineHandlerTonal::placeVietnameseTone(QString& context, int cursorPos, QChar& tone)
{
    int pos = -1;
    bool posMoved = false;
    bool replaced = false;
    for (int i = cursorPos - 1; i >= 0; i--) {
        if (VietnameseVowels.contains(context.at(i), Qt::CaseInsensitive)) {
            // A vowel with diacritics gets the tone mark, if there is one
            if (VietnameseVowelsWithDiacritics.contains(context.at(i), Qt::CaseInsensitive)) {
                pos = i;
                break;
            }
            // Set the pos to the first found vowel, can be set elsewhere later
            if (pos < 0)
                pos = i;
            // There can only be 3 vowels, and if there are, the tone doesn't fall on the last one
            else if (posMoved || pos - i > 1) {
                if (!posMoved)
                    pos--;
                break;
            } else if (i > 0) {
                // In diphtongs tone normally falls on the first vowel, with some exceptions:
                const QString diphtong = context.mid(i, 2);
                // In "qu", "u" is considered part of the consonant, and so is "i" in "gi"
                if ((diphtong.startsWith("u", Qt::CaseInsensitive) &&
                     context.mid(i-1, 1).startsWith("q", Qt::CaseInsensitive)) ||
                     (diphtong.startsWith("i", Qt::CaseInsensitive) &&
                      context.mid(i-1, 1).startsWith("g", Qt::CaseInsensitive))) {
                    break;
                // "oa" or "oe" followed by a consonant would place the tone at the last vowel
                } else if (diphtong.contains("oa", Qt::CaseInsensitive) ||
                           diphtong.contains("oe", Qt::CaseInsensitive)) {
                    if (pos == context.length() - 1) {
                        // If we are inserting at the end of the syllable, we have to check what is
                        // inserted next to know if a consonant follows or not
                        posMoved = true;
                        pos--;
                    }
                } else {
                    posMoved = true;
                    pos--;
                }
            }
        // If there is a tone mark already, change it
        } else if (context.at(i).category() == QChar::Mark_NonSpacing) {
            if (VietnameseVowels.contains(context.at(i-1))) {
                context.remove(i, 1);
                pos = i - 1;
                replaced = true;
            }
            break;
        // Otherwise if we found a vowel, we're done
        } else if (pos >= 0)
            break; 
    }
    if (pos >= 0) {
        context.insert(pos + 1, tone);
        if (cursorPos < pos)
            cursorPos = context.lastIndexOf(tone) + 1;
        mKeyboardHost.setPreedit(context, replaced?cursorPos:cursorPos + 1);
        return true;
    }
    if (pos < 0 && cursorPos < context.length()) {
        // Probably better to ignore cursor position and place later in the syllable
        // than to place it to an altogether impossible spot
        return placeVietnameseTone(context, context.length(), tone);
    }
    return false;
}

bool EngineHandlerTonal::isThaiInputAcceptable(QChar prevLetter, QChar curLetter)
{
    // Thai tones can follow Thai vowels, and some Thai diacritics can follow some Thai vowels
    // Taken from Symbian Thai input spec 7.0
    if (ThaiCategoryTONE.contains(curLetter) && (ThaiCategoryABV1.contains(prevLetter)
        || ThaiCategoryABV2.contains(prevLetter) || ThaiCategoryAV3.contains(prevLetter)))
        return true;
    else if (ThaiCategoryAD1.contains(curLetter) && ThaiCategoryABV1.contains(prevLetter))
        return true;
    else if (ThaiCategoryAD2.contains(curLetter) && ThaiCategoryAV3.contains(prevLetter))
        return true;
    // Thai consonants
    else if (prevLetter >= QChar(0x0E01) && prevLetter <= QChar(0x0E2E))
        return true;

    // Any other combination is illegal
    return false;
}

//! \reimp
bool EngineHandlerTonal::handleKeyPress(const KeyEvent &event)
{
    const QString &txt = event.text();
    if (event.specialKey() == KeyEvent::NotSpecial && txt.length() == 1) {
        QChar ch=txt.at(0);
        if (ch.category() == QChar::Mark_NonSpacing ||
            (EngineManager::instance().activeLanguage().startsWith("vi") &&
             needsVietnameseToneReposition(ch, ch)))
            return true;
    }
    return false;
}

bool EngineHandlerTonal::handleKeyRelease(const KeyEvent &event)
{
    const QString &txt = event.text();
    if (event.specialKey() == KeyEvent::NotSpecial && txt.length() == 1) {
        QChar ch=txt.at(0);
        if (ch.category() == QChar::Mark_NonSpacing ||
            (EngineManager::instance().activeLanguage().startsWith("vi") &&
             needsVietnameseToneReposition(ch, ch)))
            return true;
    }
    return false;
}

bool EngineHandlerTonal::needsVietnameseToneReposition(QChar input, QChar& autoPositionedTone)
{
    // may need this if contains oa or oe, and the tone is on o
    // and there's nothing yet after the a or e
    // and the thing being input is a final consonant or vowel
    if (!VietnameseFinalConsonants.contains(input, Qt::CaseInsensitive) &&
        !VietnameseVowels.contains(input, Qt::CaseInsensitive))
        return false;

    const QString &context = (mKeyboardHost.preedit.isEmpty()?
            recomposePreedit() : mKeyboardHost.preedit);

    const int len = context.length();
    if (len < 3)
        return false;

    if (context.endsWith("a", Qt::CaseInsensitive) || context.endsWith("e", Qt::CaseInsensitive)) {
        if (VietnameseTones.contains(context.at(len-2)) &&
            (context.at(len-3) == 'o' || context.at(len-3) == 'O')) {
            autoPositionedTone = context.at(len-2);
            return true;
        }
    }

    return false;
}

QString EngineHandlerTonal::recomposePreedit()
{
    QString surroundingText;
    int cursorPos;
    if (mKeyboardHost.inputMethodHost()->surroundingText(surroundingText, cursorPos)
        && !surroundingText.isEmpty()
        && cursorPos > 0) {
        QString previousWord = surroundingText.left(cursorPos);
        if (!previousWord.isEmpty()) {
            // ignore space, punct and symbol.
            const QChar lastChar = previousWord.at(previousWord.length() - 1);
            if (!lastChar.isSpace() && !lastChar.isPunct() && !lastChar.isSymbol()) {
                QTextBoundaryFinder finder(QTextBoundaryFinder::Word, previousWord);
                finder.setPosition(previousWord.length());
                const int lastWordBreak = finder.toPreviousBoundary();
                if (lastWordBreak > 0)
                    previousWord = previousWord.right(previousWord.length() - lastWordBreak);

                const int len = previousWord.length();
                mKeyboardHost.localSetPreedit(previousWord, -len,
                                              len, len, false);
                return previousWord;
            }
        }
    }

    // No preedit to be found
    return QString();
}

bool EngineHandlerTonal::handleKeyClick(const KeyEvent &event, bool cycleKeyActive)
{
    if (cycleKeyActive)
        return false;
    const QString &txt = event.text();
    if (event.specialKey() == KeyEvent::NotSpecial && txt.length() == 1) {
        QChar curLetter = txt.at(0);
        const QString &lang = EngineManager::instance().activeLanguage();
        QChar autoPositionedTone;

        if (curLetter.category() != QChar::Mark_NonSpacing && 
            (!lang.startsWith("vi") || !needsVietnameseToneReposition(curLetter, autoPositionedTone)))
            return false;

        QString context = mKeyboardHost.preedit;
        if (context.isEmpty())
            context = recomposePreedit();

        int cursorPos = mKeyboardHost.preeditCursorPos;
        if (cursorPos > context.length())
            cursorPos = context.length();
        else if (cursorPos < 0)
            cursorPos = 0;

        if (curLetter.category() == QChar::Mark_NonSpacing) {
            if (lang.startsWith("vi")) {
                // Set Vietnamese tones in their right positions
                if (placeVietnameseTone(context, cursorPos, curLetter))
                    return true;
                // Otherwise we let it be placed following generic logic
            }

            // Generic case: one non-spacer allowed to other letters
            // Actually this is not always so in Thai, but rendering the wrongly
            // placed diacritic this way looks at least as good as not allowing it
            QChar prevLetter;
            if (cursorPos > 0) {
                prevLetter = context.at(cursorPos - 1);
                // Thai follows special rules for non-spacing characters
                if (lang.startsWith("th")) {
                    if (!isThaiInputAcceptable(prevLetter, curLetter))
                        prevLetter = QChar();
                } else if (prevLetter.category() == QChar::Mark_NonSpacing) {
                    prevLetter = QChar();
                }
            }
            // For illegal combinations, give a space to zero-width characters for improved rendering
            if (prevLetter.isNull()) {
                context.insert(cursorPos, " ");
                cursorPos++;
            }
            // Send these to the host
            context.insert(cursorPos, curLetter);
            mKeyboardHost.setPreedit(context, cursorPos + 1);
            return true;

        } else { // May have to correct Vietnamese tone position
            int i = context.lastIndexOf(autoPositionedTone);
            if (i >= 0 && cursorPos > i) {
                // First remove the previously inserted tone, then place it again in the new syllable
                context.remove(i, 1);
                context.insert(cursorPos-1, curLetter);
                placeVietnameseTone(context, cursorPos, autoPositionedTone);
                mKeyboardHost.preeditCursorPos = cursorPos + 1;
                return true;
            }
            // We have presumably moved away
            // Restore everything, and take in the key click
            context = mKeyboardHost.preedit;
            context.insert(cursorPos, curLetter);
            mKeyboardHost.setPreedit(context, cursorPos + 1);
            return true;
        }
    }
    return false;
}

bool EngineHandlerTonal::addSpaceWhenCandidateCommited() const
{
    const QString &lang = EngineManager::instance().activeLanguage();
    if (lang.startsWith("th")) {
        return false;
    }
    return EngineHandlerDefault::addSpaceWhenCandidateCommited();
}
