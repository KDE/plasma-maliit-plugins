#ifndef UT_WORDRIBBONDIALOGMODEL_H
#define UT_WORDRIBBONDIALOGMODEL_H

#include "mapplication.h"
#include <QtTest/QTest>
#include <QObject>

class WordRibbonDialogModel;
class MPlainWindow;

class Ut_WordRibbonDialogModel : public QObject
{
    Q_OBJECT

private slots:
    //! initialize application and class
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void testRowCount();

private:
    MApplication *app;
    MPlainWindow * view;
    WordRibbonDialogModel *subject;
};

#endif
