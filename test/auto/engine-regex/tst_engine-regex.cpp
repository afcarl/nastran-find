/* - NASTRANFIND - Copyright (C) 2016 Sebastien Vavassori
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; If not, see <http://www.gnu.org/licenses/>.
 */
#include <QtTest/QtTest>
#include <QtCore/QDebug>

#include "../../../src/engine.h"

class tst_Engine : public QObject
{
    Q_OBJECT

private slots:
    void test_searchInclude();
    void test_searchInclude_data();

    void test_searchIncludeMulti();
    void test_searchIncludeMulti_data();

};

class EngineFriend : Engine { friend class tst_Engine; };

/******************************************************************************
 ******************************************************************************/
void tst_Engine::test_searchInclude_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("expected");

    QTest::newRow("test_1001") << "INCLUDE 'Test'"            << "Test" ;
    QTest::newRow("test_1002") << "INCLUDE         'Test'"    << "Test" ;
    QTest::newRow("test_1003") << "INCLUDE  \t  \t 'Test'"    << "Test" ;
    QTest::newRow("test_1004") << "INCLUDE\t'Test'"           << "Test" ;
    QTest::newRow("test_1005") << "INCLUDE\t\t\t\t'Test'"     << "Test" ;
    QTest::newRow("test_1006") << "INCLUDE \"Test\""          << "Test" ;
    QTest::newRow("test_1007") << "INCLUDE\t\"Test\""         << "Test" ;
    QTest::newRow("test_1008") << "include 'TEST'"            << "TEST" ;
    QTest::newRow("test_1009") << "include \"TEST\""          << "TEST" ;
    QTest::newRow("test_1010") << "inCLuDE \"TesT\""          << "TesT" ;
    QTest::newRow("test_1011") << "INCLUDE 'INCLUDE'"         << "INCLUDE" ;

    QTest::newRow("test_2001") << ""                      << "" ;
    QTest::newRow("test_2002") << "    "                  << "" ;
    QTest::newRow("test_2003") << "blah"                  << "" ;
    QTest::newRow("test_2004") << "  INCLUDE 'Test'"      << "" ;
    QTest::newRow("test_2005") << "\tINCLUDE 'Test'"      << "" ;
    QTest::newRow("test_2006") << "$INCLUDE 'Test'"       << "" ;
    QTest::newRow("test_2007") << "INCLUDE'Test'"         << "" ;
    QTest::newRow("test_2008") << "INCLUDE Test"          << "" ;
    QTest::newRow("test_2009") << "INCLUDE \"Test  "      << "" ;
    QTest::newRow("test_2010") << "INCLUDE  'Test  "      << "" ;
    QTest::newRow("test_2011") << "INCLUDE Test'"         << "" ;
    QTest::newRow("test_2012") << "INCLUDE Test\""        << "" ;
    QTest::newRow("test_2013") << "include  'TEST\""      << "" ;
    QTest::newRow("test_2014") << "include \"TEST'"       << "" ;

    QTest::newRow("test_3001") << "include 'Test' $comment"     << "Test" ;
    QTest::newRow("test_3002") << "include 'Test $comment'"     << "Test $comment" ;
    QTest::newRow("test_3003") << "include 'Test' blah"         << "Test" ;
    QTest::newRow("test_3004") << "include \"Test\" blah"       << "Test" ;
    QTest::newRow("test_3005") << "include 'this is a very very very very very very very long line that have more than 80 characters !'"
                               << "this is a very very very very very very very long line that have more than 80 characters !" ;

    QTest::newRow("test_4001") << "include 'a lot' of q'uotes'"     << "a lot" ;
    QTest::newRow("test_4002") << "include \"a lot\" of q\"uotes\"" << "a lot" ;
    QTest::newRow("test_4003") << "include 'a lot\" of q'uotes\""   << "a lot\" of q" ;

    QTest::newRow("test_5001") << "INCLUDE"                 << "" ;
    QTest::newRow("test_5001") << "INCLUDE     "            << "" ;
    QTest::newRow("test_5001") << "INCLUDE ''"              << "" ;
    QTest::newRow("test_5002") << "INCLUDE \"\""            << "" ;
    QTest::newRow("test_5003") << "INCLUDE '\""             << "" ;
    QTest::newRow("test_5004") << "INCLUDE \"'"             << "" ;

}

void tst_Engine::test_searchInclude()
{
    QFETCH(QString, text);
    QFETCH(QString, expected);
    std::string _text = text.toStdString();

    EngineFriend engine;
    std::string _actual = engine.searchInclude( _text );

    QString actual = QString::fromStdString( _actual );

    QCOMPARE(actual, expected);

}

/******************************************************************************
 ******************************************************************************/
void tst_Engine::test_searchIncludeMulti_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("expected");

    QTest::newRow("test__1") << "" << "" ;

}
void tst_Engine::test_searchIncludeMulti()
{
    QFETCH(QString, text);
    QFETCH(QString, expected);
    std::string _text = text.toStdString();

}

/******************************************************************************
 ******************************************************************************/

QTEST_APPLESS_MAIN(tst_Engine)

#include "tst_engine-regex.moc"

