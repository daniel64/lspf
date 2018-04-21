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
#define MOD_NAME SCREEN


pLScreen::pLScreen()
{
	row = 0 ;
	col = 0 ;
	if ( screensTotal == 0 )
	{
		initscr();
		getmaxyx( stdscr, maxrow, maxcol ) ;
		if ( maxrow < 24 || maxcol < 80 )
		{
			endwin() ;
			cout << "This program cannot run in a screen with fewer than 24 lines and 80 columns." << endl ;
			cout << "The size of this screen is " << maxrow << " lines by " << maxcol << " columns." << endl ;
			cout << "Exiting..." << endl ;
			return;
		}
		maxrow = maxrow - 2 ;
		start_color();
		raw()    ;
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
		mousemask( ALL_MOUSE_EVENTS, NULL ) ;
		OIA       = newwin( 2, maxcol, maxrow, 0 ) ;
		OIA_panel = new_panel( OIA ) ;
	}
	++screensTotal    ;
	currScreen = this ;
	screenId   = ++maxScreenId ;
	Insert     = false         ;
}


pLScreen::~pLScreen()
{
	screensTotal-- ;
	if ( screensTotal == 0 )
	{
		del_panel( OIA_panel ) ;
		delwin( OIA ) ;
		endwin() ;
	}
}


void pLScreen::clear()
{
	for ( unsigned int i = 0 ; i < maxrow ; i++ )
	{
		move( i, 0 ) ;
		clrtoeol()   ;
	}
}


void pLScreen::save_panel_stack()
{
	// Save all panels for this logical screen
	// Panel user data : object panel_data
	//                   1 int field, screenId

	PANEL* pnl ;

	const void* vptr ;
	const panel_data* pd ;

	while ( !panelList.empty() )
	{
		panelList.pop() ;
	}
	pnl = NULL ;
	while ( true )
	{
		pnl = panel_below( pnl ) ;
		if ( pnl == NULL ) { break ; }
		vptr = panel_userptr( pnl ) ;
		if ( vptr == NULL ) { continue ; }
		pd = static_cast<const panel_data*>(vptr) ;
		if ( pd->screenId != screenId ) { continue ; }
		panelList.push( pnl ) ;
	}
}


void pLScreen::restore_panel_stack()
{
	// Restore saved panels for this logical screen.
	// Call touchwin() for the associated window, to make sure panels are fully refreshed.

	while ( !panelList.empty() )
	{
		top_panel( panelList.top() ) ;
		touchwin( panel_window( panelList.top() ) ) ;
		panelList.pop() ;
	}
}


void pLScreen::refresh_panel_stack()
{
	save_panel_stack()    ;
	restore_panel_stack() ;
}


void pLScreen::set_cursor( pApplication* appl )
{
	// Set the logical screen cursor to that held by the application

	appl->get_cursor( row, col ) ;
}


void pLScreen::OIA_setup()
{
	mvwhline( OIA, 0, 0, ACS_HLINE, maxcol ) ;
	mvwaddch( OIA, 1, 0, ACS_CKBOARD ) ;
	wattrset( OIA, YELLOW ) ;
	mvwaddstr( OIA, 1, 2,  "Screen[        ]" ) ;
	mvwaddstr( OIA, 1, 30, "Elapsed:" ) ;
	mvwaddstr( OIA, 1, 50, "Screen:" ) ;
	wattroff( OIA, YELLOW ) ;
}


void pLScreen::OIA_update( int screen, int altscreen, boost::posix_time::ptime et )
{
	int pos ;

	string respTime ;

	respTime = to_iso_string( et - startTime )  ;
	pos      = respTime.find_last_of( '.' ) - 1 ;
	respTime = substr( respTime, pos, 6 ) + " s" ;

	wattrset( OIA, YELLOW ) ;
	mvwaddstr( OIA, 1, 9,  "        " ) ;
	mvwaddstr( OIA, 1, 9, substr( "12345678", 1, screensTotal).c_str() ) ;
	mvwaddstr( OIA, 1, 26, "   " ) ;
	mvwaddstr( OIA, 1, 39, respTime.c_str() ) ;
	mvwprintw( OIA, 1, 58, "%d-%d   ", screenId, application_stack_size() );
	mvwaddstr( OIA, 1, maxcol-22, Insert ? "Insert" : "      " ) ;
	mvwprintw( OIA, 1, maxcol-14, "Row %d Col %d  ", row+1, col+1 ) ;

	if ( screen < 8 )
	{
		wattrset( OIA, RED | A_REVERSE ) ;
		mvwaddch( OIA, 1, screen+9, d2ds( screen+1 ).front() ) ;
		wattroff( OIA, RED | A_REVERSE ) ;
	}

	if ( altscreen < 8 && altscreen != screen )
	{
		wattrset( OIA, YELLOW | A_BOLD | A_UNDERLINE ) ;
		mvwaddch( OIA, 1, altscreen+9, d2ds( altscreen+1 ).front() ) ;
		wattroff( OIA, YELLOW | A_BOLD | A_UNDERLINE ) ;
	}

	wattroff( OIA, YELLOW ) ;
}


void pLScreen::OIA_refresh()
{
	top_panel( OIA_panel ) ;
	touchwin( OIA ) ;
}


void pLScreen::show_enter()
{
	wattron( OIA, RED ) ;
	mvwaddstr( OIA, 1, 19, "X-Enter" ) ;
	wattroff( OIA, RED ) ;
	wmove( OIA, 1, 0 ) ;
	wrefresh( OIA )    ;
}


void pLScreen::show_busy()
{
	wattron( OIA, RED ) ;
	mvwaddstr( OIA, 1, 19, "X-Busy " ) ;
	wattroff( OIA, RED ) ;
	wmove( OIA, 1, 0 ) ;
	wrefresh( OIA )    ;
}


void pLScreen::show_wait()
{
	wattron( OIA, RED ) ;
	mvwaddstr( OIA, 1, 19, "X-Wait " ) ;
	wattroff( OIA, RED ) ;
	wmove( OIA, 1, 0 ) ;
	wrefresh( OIA )    ;
}


void pLScreen::show_auto()
{
	wattron( OIA, RED ) ;
	mvwaddstr( OIA, 1, 19, "X-Auto " ) ;
	wattroff( OIA, RED ) ;
	wmove( OIA, 1, 0 ) ;
	wrefresh( OIA )    ;
}


void pLScreen::show_lock( bool showLock )
{
	if ( showLock )
	{
		wattrset( OIA,  RED ) ;
		mvwaddstr( OIA, 1, 26, "|X|" ) ;
		wattroff( OIA,  RED ) ;
	}
}


void pLScreen::clear_status()
{
	mvwaddstr( OIA, 1, 19, "       " ) ;
}
