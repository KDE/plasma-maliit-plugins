#ifndef UT_WORDRIBBONDIALOG_H
#define UT_WORDRIBBONDIALOG_H

#include "mapplication.h"
#include <QtTest/QTest>
#include <QObject>

class WordRibbonDialog;
class MPlainWindow;

class Ut_WordRibbonDialog : public QObject
{
    Q_OBJECT

private slots:
    //! initialize application and class
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void testSetCandidates();

private:
    MApplication *app;
    MWindow *appWin;
    MPlainWindow *view;
    WordRibbonDialog *subject;
};

#endif
