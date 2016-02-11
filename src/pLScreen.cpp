/*
  Copyright (c) 2015 Daniel John Erdos

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#undef  MOD_NAME
#undef  LOGOUT
#define MOD_NAME pLScreen
#define LOGOUT   aplog


pLScreen::pLScreen()
{
	row = 0 ;
	col = 0 ;
	if ( screensTotal == 0 )
	{
		initscr();
		getmaxyx( stdscr, maxrow, maxcol ) ;
		if ( (maxrow < 24) || (maxcol < 80) )
		{
			endwin() ;
			cout << "This program cannot run in a screen with fewer than 24 lines and 80 columns." << endl ;
			cout << "The size of this screen is " << maxrow << " lines by " << maxcol << " columns." << endl ;
			cout << "Exiting..." << endl ;
			return;
		}
		maxrow = maxrow - 2 ;
		start_color();
		cbreak() ;
		noecho() ;
		nonl()   ;
		intrflush( stdscr, FALSE ) ;
		keypad( stdscr, TRUE )     ;
		init_pair( 0, COLOR_BLACK,   COLOR_BLACK ) ;
		init_pair( 1, COLOR_RED,     COLOR_BLACK ) ;
		init_pair( 2, COLOR_GREEN,   COLOR_BLACK ) ;
		init_pair( 3, COLOR_YELLOW,  COLOR_BLACK ) ;
		init_pair( 4, COLOR_BLUE,    COLOR_BLACK ) ;
		init_pair( 5, COLOR_MAGENTA, COLOR_BLACK ) ;
		init_pair( 6, COLOR_CYAN,    COLOR_BLACK ) ;
		init_pair( 7, COLOR_WHITE,   COLOR_BLACK ) ;
		mvhline( maxrow, 0, ACS_HLINE, maxcol ) ;
		mousemask( ALL_MOUSE_EVENTS, NULL ) ;
		OIA = newwin( 1, maxcol, maxrow+1, 0 ) ;
	}
	++screensTotal    ;
	currScreen = this ;
	screenID   = ++maxScreenID ;
}


pLScreen::~pLScreen()
{
	--screensTotal ;
	if ( screensTotal == 0 ) { endwin() ; }
	return ;
}


void pLScreen::clear()
{
	int i ;

	for ( i = 0 ; i < maxrow ; i++ )
	{
		move( i, 0 ) ;
		clrtoeol()   ;
	}
}


void pLScreen::OIA_setup()
{
	mvwaddch( OIA, 0, 0, ACS_CKBOARD ) ;
	wattrset( OIA, YELLOW ) ;
	mvwaddstr( OIA, 0, 2,  "Screen[        ]" ) ;
	mvwaddstr( OIA, 0, 31, "Elapsed:" ) ;
	mvwaddstr( OIA, 0, 51, "Screen:" ) ;
	wattroff( OIA, YELLOW ) ;
}


void pLScreen::show_enter()
{
	wattron( OIA, RED ) ;
	mvwaddstr( OIA, 0, 20, "X-Enter" ) ;
	wattroff( OIA, RED ) ;
	wmove( OIA, 0, 0 ) ;
	wrefresh( OIA )  ;
}


void pLScreen::show_busy()
{
	wattron( OIA, RED ) ;
	mvwaddstr( OIA, 0, 20, "X-Busy " ) ;
	wattroff( OIA, RED ) ;
	wmove( OIA, 0, 0 ) ;
	wrefresh( OIA )  ;
}


void pLScreen::clear_status()
{
	mvwaddstr( OIA, 0, 20, "       " ) ;
}
