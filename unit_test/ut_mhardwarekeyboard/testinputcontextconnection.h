#include <minputcontextconnection.h>
#include <QString>
#include <QKeyEvent>

/*!
 * \brief Dummy input context connection for ut_mhardwarekeyboard
 */
class TestInputContextConnection : public MInputContextConnection
{
    Q_OBJECT

public:
    TestInputContextConnection()
        : lastKeyEventM(QEvent::KeyRelease, 0, Qt::NoModifier),
          keyEventCounter(0)
    {
    }

    // Methods we don't care about...........................................

    virtual int contentType(bool &/* valid */)
    {
        return 0;
    }

    virtual bool correctionEnabled(bool &/* valid */)
    {
        return false;
    }

    virtual bool predictionEnabled(bool &/* valid */)
    {
        return false;
    }

    virtual bool autoCapitalizationEnabled(bool &/* valid */)
    {
        return false;
    }

    virtual bool surroundingText(QString &/* text */, int &/* cursorPosition */)
    {
        return false;
    }

    virtual bool hasSelection(bool &/* valid */)
    {
        return false;
    }

    virtual int inputMethodMode(bool &/* valid */)
    {
        return 0;
    }

    virtual QRect preeditRectangle(bool &/* valid */)
    {
        return QRect();
    }

    virtual void notifyImInitiatedHiding()
    {
    }

    virtual void copy()
    {
    }

    virtual void paste()
    {
    }

    virtual void setRedirectKeys(bool /* enabled */)
    {
    }

    virtual void setGlobalCorrectionEnabled(bool)
    {
    }

    // Methods we care about.................................................

    virtual void sendPreeditString(const QString &string,
                                   PreeditFace /* preeditFace */)
    {
        lastPreeditStringM = string;
    }

    virtual void sendCommitString(const QString &string)
    {
        lastCommitStringM = string;
    }

    virtual void sendKeyEvent(const QKeyEvent &keyEvent)
    {
        ++keyEventCounter;
        lastKeyEventM = keyEvent;
    }

    // Special methods for ut_mhardwarekeyboard............................

    QKeyEvent lastKeyEvent() const
    {
        return lastKeyEventM;
    }

    QString lastPreeditString() const
    {
        return lastPreeditStringM;
    }

    QString lastCommitString() const
    {
        return lastCommitStringM;
    }

    unsigned int keyEventsSent() const
    {
        return keyEventCounter;
    }

private:
    Q_DISABLE_COPY(TestInputContextConnection)

    QString lastPreeditStringM;
    QString lastCommitStringM;
    QKeyEvent lastKeyEventM;
    unsigned int keyEventCounter;
};
