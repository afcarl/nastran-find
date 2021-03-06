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
#include "application.h"

#include "global.h"
#include "stringhelper.h"
#include "systemdetection.h"

#include <curses.h>
#include <iostream> // std::cout
#include <stdio.h>
#include <stdlib.h>

#if defined(Q_OS_WIN)
#  include <windows.h>
#elif defined(Q_OS_UNIX)
#  include <limits.h>
#endif

using namespace std;

static const char STR_ERR_NO_COLORS[]  = "Error: terminal does not support color";
static const char STR_NO_RESULT[]      = "(no results)";

/*! \class Application
 *  \brief The class Application represents the main window.
 */

/*! \brief Constructor.
 */
Application::Application( int argc, char *argv[] )
    : m_mode(Mode::BROWSE)
    , m_fullFileName("~~~unknown file~~~")
    , m_searchedText(string())
    , m_currentScroll(0)
    , m_maximumScroll(0)
    , m_rowTitleBox(0)
    , m_rowResultBox(5)
    , m_rowErrorBox(0)
    , m_rowInfoBox(0)
{
    ttytype[0] = 25; // Curses: allow 25 to 90 lines and 80 to 200 columns
    ttytype[1] = 90;
    ttytype[2] = 80;
    ttytype[3] = (unsigned char)200;

#ifdef XCURSES
    Xinitscr(argc, argv);  // Curses: initialize curses mode
#else
    (void)argc;
    (void)argv;
    initscr();
#endif

    this->initialize();

}

/******************************************************************************
 ******************************************************************************/
/*! \brief Initialize the application and Curses.
 */
void Application::initialize()
{
    if( has_colors() == FALSE ){  // Curses: terminal does not support color
        endwin(); // Curses: exit curses
        cout << STR_ERR_NO_COLORS << endl;
        exit(EXIT_FAILURE);
    }
    start_color();

# if defined(NCURSES_VERSION) || (defined(PDC_BUILD) && PDC_BUILD > 3000)
    use_default_colors();
# endif
    cbreak();
    noecho();
    clear();
    refresh();
#ifdef __PDCURSES__
    PDC_set_title( C_APPLICATION_NAME );
#endif

    /*              id color                foreground     background   */
    init_pair( (int)Color::NORMAL,          COLOR_WHITE,   COLOR_BLACK);
    init_pair( (int)Color::TITLE,           COLOR_BLACK,   COLOR_GREEN);
    init_pair( (int)Color::SEARCH_BOX,      COLOR_GREEN,   COLOR_BLACK);
    init_pair( (int)Color::FILE_NAME,       COLOR_YELLOW,  COLOR_BLACK);
    init_pair( (int)Color::LINE_NUMBER,     COLOR_CYAN,    COLOR_BLACK);
    init_pair( (int)Color::OCCURRENCE,      COLOR_BLACK,   COLOR_YELLOW);
    init_pair( (int)Color::ERROR_MESSAGE,   COLOR_BLACK,   COLOR_RED);
    init_pair( (int)Color::NASTRAN_CARD,    COLOR_YELLOW,  COLOR_BLACK);
    init_pair( (int)Color::NASTRAN_COMMENT, COLOR_GREEN,   COLOR_BLACK);
    init_pair( (int)Color::NASTRAN_DIGIT,   COLOR_RED,     COLOR_BLACK);
    init_pair( (int)Color::NASTRAN_QUOTE,   COLOR_CYAN,    COLOR_BLACK);
    init_pair( (int)Color::NASTRAN_SYMBOL,  COLOR_RED,     COLOR_BLACK);

    attrset(COLOR_PAIR(Color::NORMAL));
    keypad(stdscr, TRUE); // Curses: enables special keys F1, F2 etc..

    mousemask( ALL_MOUSE_EVENTS, NULL);
}

/******************************************************************************
 ******************************************************************************/
void Application::resetConfig()
{
    m_recentFile.clear();
    /// \todo Clean the window position + Register too ?
}

/******************************************************************************
 ******************************************************************************/
// Remark: 'filename' can be absolute, canonical or relative...
void Application::setFilename(const string &filename)
{
#if defined(Q_OS_WIN)
    char fullFilename[MAX_PATH];
    GetFullPathNameA(filename.c_str(), MAX_PATH, fullFilename, NULL);
#elif defined(Q_OS_UNIX)
	char fullFilename[PATH_MAX];
    char *ret = realpath(filename.c_str(), fullFilename);
    if (!ret) {
        std::cout << "Warning: realpath( '" << ret << "'." << std::endl;
    }
#endif
    m_fullFileName = fullFilename;
    m_recentFile.prepend(m_fullFileName);
}

/******************************************************************************
 ******************************************************************************/
int Application::exec()
{
    m_engine.find( m_fullFileName, string() );

    /* ************************** */
    /*          Main loop         */
    /* While key 'Q' not pressed  */
    /* ************************** */
    int pressedKey = 0;
    while( pressedKey != 'q' &&
           pressedKey != 'Q' &&
           pressedKey != '\033' ){  // \033 = Escape key

        clear(); // Curses: clear the screen

        this->showTitle();
        this->showInfo();
        this->showErrors();
        this->showResults();
        this->hideCursor();

        refresh(); // Curses: print all on real screen

        switch(m_mode){
        case Mode::BROWSE: {
            noecho(); // Curses: Don't echo() while we do getch
            pressedKey = getch(); // Curses: Wait for user's next action
            this->onKeyPressed(pressedKey);
            break;
        }

        case Mode::SEARCH: {
            /* erase previous search with whitespaces */
            move(3,9);
            string buf;
            buf.assign(C_SEARCH_SIZE, ' ');
            buf += ">";
            printw(buf.c_str());
            move(3,9);
            echo();

            colorize(Color::SEARCH_BOX);
            char buffer[C_SEARCH_SIZE];
            getnstr( buffer, sizeof(buffer) - 1 ); // Curses: get a length-fixed input
            uncolorize();

            m_searchedText = buffer;

            m_engine.find( m_fullFileName, m_searchedText );

            m_currentScroll = 0;
            m_maximumScroll = getMaximumScroll();

            m_mode = Mode::BROWSE;
            break;
        }

        default:
            break;
        }
    }

    endwin(); // Curses: exit curses
    return 0;
}

/******************************************************************************
 ******************************************************************************/
void Application::onKeyPressed(const int key)
{
    switch(key) {

    case KEY_RESIZE:
        resize_term(0, 0);
        refresh();
        break;

    case 'f':
    case 'F':
        m_mode = Mode::SEARCH;
        break;

        /// \todo case 'p':
        /// \todo case 'P':
        /// \todo /* Previous research */
        /// \todo 	/* TO DO */
        /// \todo 	break;

    case 'x':
    case 'X':
    case KEY_NPAGE:
        /* Scroll page down fast */
        m_mode = Mode::BROWSE;
        if( m_currentScroll < m_maximumScroll ){
            m_currentScroll += 8;
        }
        break;

    case 's':
    case 'S':
    case KEY_PPAGE:
        /* Scroll page up fast */
        m_mode = Mode::BROWSE;
        if( m_currentScroll > 0 ){
            m_currentScroll -= 8;
        }
        break;

    case 'z':
    case 'Z':
    case KEY_DOWN:
        /* Scroll page down slowly */
        m_mode = Mode::BROWSE;
        if( m_currentScroll < m_maximumScroll ){
            m_currentScroll++;
        }
        break;


    case 'a':
    case 'A':
    case KEY_UP:
        /* Scroll page up slowly  */
        m_mode = Mode::BROWSE;
        if( m_currentScroll > 0 ){
            m_currentScroll--;
        }
        break;

    default: break; /* other keys */
    }
}


/******************************************************************************
 ******************************************************************************/
/*! Displays the header of the application on screen.
 */
void Application::showTitle()
{
    /* Title */
    move(m_rowTitleBox,0);

    colorize(Color::TITLE);
    printw( C_APPLICATION_NAME );
    uncolorize();

    printw( C_APPLICATION_SUBTITLE );

    /* File info */
    move(m_rowTitleBox+1,0);
    printw( "File: %s   (total %i included)", m_fullFileName.c_str(), m_engine.linkCount() );

    /* SearchBox */
    move(m_rowTitleBox+3,0);
    printw( "Search: <" );

    colorize(Color::SEARCH_BOX);
    if( m_searchedText.empty() ){
        printw( "---search field empty---" );
    }else{
        printw( "%s", m_searchedText.c_str() );
    }
    uncolorize();

    printw( ">" );

    move(m_rowTitleBox+3,50);
    printw( "(case insensitive)" );


}

/******************************************************************************
 ******************************************************************************/
/*! Displays the results on screen.
 */
void Application::showResults()
{
    int row = m_rowResultBox;
    int first_page_shown = -1;

    move(row,0);
    printw( "Results: %i occurences in %i files. (scroll %i/%i)",
            m_engine.occurrenceCountAll(),
            m_engine.linkCount(),
            m_currentScroll, m_maximumScroll );

    move(row+1,0);

    const string prev = horizontalSeparator();
    printw( prev.c_str() );

    row += 2; // start

    const stringlist& files = m_engine.files();

    for( stringlist::const_iterator it = files.begin(); it != files.end(); ++it ) {

        const string& file = (*it);

        ++first_page_shown;
        if( first_page_shown >= m_currentScroll && row < m_rowErrorBox ) {
            move(row,0);
            colorize(Color::FILE_NAME);
            printw( "--- %s ---", file.c_str() );
            uncolorize();
            ++row;
        }

        const ResultMap& results = m_engine.results();
        const Result& res = results.at(file);
        const stringlist& result = res.occurrences;

        if( result.size() > 0 ) {

            for( stringlist::const_iterator it2 = result.begin(); it2 != result.end(); ++it2 ) {

                ++first_page_shown;
                if( first_page_shown >= m_currentScroll && row < m_rowErrorBox ){
                    const string& result = (*it2);
                    move(row,0);
                    this->printwSyntaxColoration( result, row );
                    ++row;
                }
            }

            ++first_page_shown;
            if( first_page_shown >= m_currentScroll && row < m_rowErrorBox ) {
                ++row;
            }
        } else {

            ++first_page_shown;
            if( first_page_shown >= m_currentScroll && row < m_rowErrorBox ) {
                move(row,0);
                this->printwSyntaxColoration( STR_NO_RESULT, row );
                ++row;
                ++row;
            }
        }
    }
}



/******************************************************************************
 ******************************************************************************/
/*! \brief Prints the given \a text at the given screen's \a row
 *         with a basic syntax coloration.
 *
 * \example
 * \code
 *  |------------|--------------------|
 *  line      11: $ SIDE PANEL SPCs
 *  ^^^^^blue^^^^ ^^^^^^green^^^^^^
 * \endcode
 */
void Application::printwSyntaxColoration(const string &text, const int row )
{
    /* str.substr(i, string::npos) */
    /*               ^^^^^^^^^^^  */
    /* --> Returns all char from 'i' to the string end. */

    if( text[0] == '(') {
        /* ***************************** */
        /* Filename                      */
        /* ***************************** */
        colorize(Color::FILE_NAME);
        printw( "%s", text.c_str() );
        uncolorize();

    } else if(text[0] == '!') {
        /* ***************************** */
        /* Error Message                 */
        /* ***************************** */
        colorize(Color::ERROR_MESSAGE);
        printw( "%s", text.c_str() );
        uncolorize();

    } else if( text.length() >= C_LINE_NUMBER_WIDTH && text.substr(0,4) == "line" ) {
        /* ***************************** */
        /* Results                       */
        /* ***************************** */
        colorize(Color::LINE_NUMBER);
        printw( "%s", text.substr(0, C_LINE_NUMBER_WIDTH).c_str() );
        uncolorize();

        Symbol symbol = Symbol::UNKNOWN;

        //int type = -1;

        auto p = text.begin() + C_LINE_NUMBER_WIDTH;
        while( p < text.end() ){

            char ch = (*p);
            switch(ch) {
            case '$':
                colorize(Color::NASTRAN_COMMENT);
                printw( "%s", p);
                uncolorize();
                p = text.end();
                break;

            case ' ':
            case '=':
            case ',':
                if (symbol != Symbol::QUOTE) {
                    symbol = Symbol::UNKNOWN;
                    printw( "%c", ch );
                }
                break;

            default:
                if( (symbol == Symbol::UNKNOWN && isalpha( ch ))
                        || symbol == Symbol::ALPHA ) {

                    /* Deck Card */
                    symbol = Symbol::ALPHA;
                    colorize(Color::NASTRAN_CARD);
                    printw( "%c", ch );
                    uncolorize();

                } else if( (symbol == Symbol::UNKNOWN && isdigit( ch ))
                           || symbol == Symbol::DIGIT ) {

                    /* Digit 0..9 */
                    symbol = Symbol::DIGIT;
                    colorize(Color::NASTRAN_DIGIT);
                    printw( "%c", ch );
                    uncolorize();

                } else if( (symbol == Symbol::UNKNOWN && ( ch =='\"' ||  ch =='\'') )
                           || symbol == Symbol::QUOTE )  {

                    /* Quotes text */
                    symbol = Symbol::QUOTE;
                    colorize(Color::NASTRAN_QUOTE);
                    printw( "%c", ch );
                    uncolorize();

                } else {
                    /* Other char like +, ., etc. */
                    colorize(Color::NASTRAN_SYMBOL);
                    printw( "%c", ch );
                    uncolorize();
                }

                break;
            }
            ++p;
        }

        /* ***************************** */
        /* Highlight the occurences      */
        /* ***************************** */
        if ( !StringHelper::hasSpaces(m_searchedText) ) {
            int len = m_searchedText.length();
            int loc = StringHelper::findNext(text, m_searchedText, C_LINE_NUMBER_WIDTH );
            while( loc >= 0 ){
                move(row, loc);
                text_in_a_box( text.c_str() + loc, len );
                loc = StringHelper::findNext(text, m_searchedText, loc+1 );
            }
        }

    } else {
        /* ***************************** */
        /* Empty line or other thing     */
        /* ***************************** */
        printw( "%s", text.c_str() );
    }

}

/******************************************************************************
 ******************************************************************************/
/*! Displays the error messages contained in list \a le on the screen.
 *  Normally the list must be empty, aka there is no error message.
 */
void Application::showErrors()
{
    m_rowErrorBox = m_rowInfoBox - m_engine.errorCount() - 1;

    if (m_engine.errorCount() == 0) {
        return;
    }

    move(m_rowErrorBox,0);

    const string prev = horizontalSeparator();
    printw( prev.c_str() );

    int row = 1;

    for (string::size_type i = 0; i < m_engine.errorCount(); ++i) {
        const string error_msg = m_engine.errorAt(i);

        move(m_rowErrorBox + row, 0);
        colorize(Color::ERROR_MESSAGE);
        printw( "/!\\:%s", error_msg.c_str() );
        uncolorize();
        ++row;
    }
}

/******************************************************************************
 ******************************************************************************/
/*! Displays the application footer on screen.
 */
void Application::showInfo()
{
    int rowCount = getmaxy(stdscr); // Curses: get the max number of rows
    m_rowInfoBox = rowCount - 2;

    //show infos
    move(m_rowInfoBox,0);

    const string prev = horizontalSeparator();
    printw( prev.c_str() );

    move(m_rowInfoBox+1,0);
    printw("[q]:Exit      "
           "[f]:New Search      "
           "Key Up/Down,Page Up/Down,[a][s],[z][x]:Previous/Next page");
}

/******************************************************************************
 ******************************************************************************/
/*! \brief Returns a string containing N times the given character \a c,
 *         with N the width of the screen.
 * This function can be used to draw a horizontal separator on the screen.
 */
inline string Application::horizontalSeparator(const char c) const
{
    string ret;
    int columnCount = getmaxx(stdscr); // Curses: get the max number of columns
    ret.assign(columnCount, c);
    return ret;
}

inline stringlist::size_type Application::getMaximumScroll() const
{
    auto value = 0;
    const stringlist& files = m_engine.files();
    for( stringlist::const_iterator it = files.begin(); it != files.end(); ++it ) {
        const string& file = (*it);
        value += 3;
        value += m_engine.resultCountLines( file );
    }
    return value;
}

inline void Application::hideCursor()
{
    move(m_rowTitleBox,0);
}

/******************************************************************************
 ******************************************************************************/
inline void Application::colorize(Application::Color color)
{
    m_currentColor = color;
    attron(COLOR_PAIR(m_currentColor)); // Curses: colorize
}

inline void Application::uncolorize()
{
    attroff(COLOR_PAIR(m_currentColor)); // Curses: un-colorize
}

/******************************************************************************
 ******************************************************************************/
void Application::text_in_a_box( const char *istr, const int len)
{
#ifdef CHTYPE_LONG
    attron( A_OVERLINE | A_UNDERLINE | A_LEFTLINE);
    if( len == 1)
        attron( A_RIGHTLINE);
#endif
    addnstr( istr, 1);
    if( len > 1)
    {
#ifdef CHTYPE_LONG
        attroff( A_LEFTLINE);
#endif
        if( len > 2)
            addnstr( istr + 1, len - 2);
#ifdef CHTYPE_LONG
        attron( A_RIGHTLINE);
#endif
        addnstr( istr + len - 1, 1);
    }
#ifdef CHTYPE_LONG
    attroff( A_OVERLINE | A_UNDERLINE | A_LEFTLINE | A_RIGHTLINE);
#endif
}
