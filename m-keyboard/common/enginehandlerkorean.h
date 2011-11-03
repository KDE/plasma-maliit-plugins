#include "mabstractinputmethodhost.h"
#include "enginemanager.h"
#include "enginehandlerdefault.h"
#include "mimcorrectionhost.h"
#include "mkeyboardhost.h"
#include <mimenginefactory.h>
#include <MGConfItem>
#include <MSceneWindow>

/*
 * Korean (Hangul) input engine handler class
 *
 * One Korean character is a composition of multiple Korean alphabets.
 * Each single Korean character has its unique sequence of typings.
 * Korean input system needs to compose the sequence into a character
 * and display the procedure of the composition.
 * ex. "ㅎ", "ㅏ" and "ㄴ" becomes "한". Users see "ㅎ", "하" and "한" 
 * in the preedit area as they type.
 *
 * Hangul input engine produces commit and preedit characters 
 * in every key input. The engine handler should take those in every input 
 * and show them properly.
 *
 * The handler assume that Hangul engine produce a single candidate with
 * a commit character and a preedit character. The commit charater is 
 * available when a Korean character is composed completely.
 * So if the length of candidate string is 1, this means the candidate string 
 * is preedit string. If the length of candidate is 2, the first character is 
 * commit and the last one is preedit.
 *
 * EngineHandlerKorean is based on the most famous Hangul engine: libhangul.
 *
 */

class EngineHandlerKorean : public EngineHandlerDefault
{
public:
    explicit EngineHandlerKorean(MKeyboardHost &keyboardHost);
    virtual ~EngineHandlerKorean();
    static QStringList supportedLanguages();

    //! \reimp
    virtual void activate();
    virtual void deactivate();
    virtual AbstractEngineWidgetHost *engineWidgetHost();
    virtual bool cursorCanMoveInsidePreedit() const;
    virtual bool hasHwKeyboardIndicator() const;
    virtual bool hasErrorCorrection() const;
    virtual bool acceptPreeditInjection() const;
    virtual bool hasAutoCaps() const;
    virtual QList<QRegExp> autoCapsTriggers() const;
    virtual bool hasContext() const;
    virtual bool commitPreeditWhenInterrupted() const;
    virtual bool correctionAcceptedWithSpaceEnabled() const;
    virtual bool isComposingInputMethod() const;
    virtual bool supportTouchPointAccuracy() const;
    virtual bool commitWhenCandidateClicked() const;
    virtual void clearPreedit(bool commit);
    virtual void editingInterrupted();
    virtual void resetHandler();
    virtual void preparePluginSwitching();
    virtual bool handleKeyPress(const KeyEvent &event);
    virtual bool handleKeyRelease(const KeyEvent &event);
    virtual bool handleKeyClick(const KeyEvent &event, bool cycleKeyActive);
    virtual bool handleKeyCancel(const KeyEvent &event);
    //! \reimp_end

private:
    void sendPreedit(const QString & str);
    void sendCommit(const QString & str);
    QString getPreedit();
    QString getCommit();
    void flushOut();
    void clearCandidates();

private:
    MKeyboardHost &mKeyboardHost;
    AbstractEngineWidgetHost *mEngineWidgetHost;
    MImEngineWordsInterface *engine;
};

