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

pLScreen::pLScreen( uint openedBy )
{
	row = 0 ;
	col = 0 ;

	if ( screensTotal == 0 )
	{
		initscr() ;
		getmaxyx( stdscr, maxrow, maxcol ) ;
		if ( maxrow < 26 || maxcol < 80 )
		{
			endwin() ;
			cout << "This program cannot run in a screen with fewer than 26 lines and 80 columns." << endl ;
			cout << "The size of this screen is " << maxrow << " lines by " << maxcol << " columns." << endl ;
			cout << "Aborting..." << endl ;
			abort() ;
		}
		maxrow -= 2 ;
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
		OIA       = newwin( 2, maxcol, maxrow, 0 ) ;
		OIA_panel = new_panel( OIA ) ;
		SWB       = newwin( 1, maxcol, maxrow-1, 0 ) ;
		SWB_panel = new_panel( SWB ) ;
	}

	++screensTotal ;
	currScreen = this  ;
	Insert     = false ;
	screenId   = ++maxScreenId ;
	openedByList[ screenId ] = openedBy ;

	p_lss = new lss ;

	for ( uint i = 1 ; i <= maxscrn ; ++i )
	{
		if ( screenNums.count( i ) == 0 )
		{
			screenNum = i ;
			screenNums.insert( i ) ;
			break ;
		}
	}
}


pLScreen::~pLScreen()
{
	//
	// Update the openedByList so any screen opened by the deleted screen, appears opened by its predecesor.
	//

	--screensTotal ;
	if ( screensTotal == 0 )
	{
		del_panel( OIA_panel ) ;
		delwin( OIA ) ;
		endwin() ;
	}
	else
	{
		for ( auto it = openedByList.begin() ; it != openedByList.end() ; ++it )
		{
			if ( it->second == screenId )
			{
				it->second = openedByList[ screenId ] ;
			}
		}
		openedByList.erase( screenId ) ;
		screenNums.erase( screenNum ) ;
	}

	delete p_lss ;
}


void pLScreen::cursor_left()
{
	if ( col == 0 )
	{
		col = maxcol - 1 ;
		cursor_up() ;
	}
	else
	{
		--col ;
	}
}


void pLScreen::cursor_left_cond()
{
	if ( col > 1 ) { --col ; }
}


void pLScreen::cursor_right()
{
	if ( col == maxcol - 1 )
	{
		col = 0 ;
		cursor_down() ;
	}
	else
	{
		++col ;
	}
}


void pLScreen::cursor_right_cond()
{
	if ( col < maxcol - 2 ) { ++col ; }
}


void pLScreen::cursor_up()
{
	( row == 0 ) ? row = maxrow - 1 : --row ;
}


void pLScreen::cursor_up_cond()
{
	if ( row > 1 ) { --row ; }
}


void pLScreen::cursor_down()
{
	( row == maxrow - 1 ) ? row = 0 : ++row ;
}


void pLScreen::cursor_down_cond()
{
	if ( row < maxrow - 2 ) { ++row ; }
}


void pLScreen::save_panel_stack()
{
	//
	// Save all panels for this logical screen.
	// Panel user data : objects panel_data and panel_data_ext.
	//

	PANEL* pnl = panel_below( nullptr ) ;

	const void* vptr ;
	const panel_data* pd ;

	while ( !panelList.empty() )
	{
		panelList.pop() ;
	}

	while ( pnl )
	{
		vptr = panel_userptr( pnl ) ;
		if ( vptr )
		{
			pd = static_cast<const panel_data*>(vptr) ;
			if ( pd->screenid() == screenId )
			{
				panelList.push( pnl ) ;
			}
		}
		pnl = panel_below( pnl ) ;
	}
}


void pLScreen::restore_panel_stack()
{
	//
	// Restore saved panels for this logical screen.
	// Call touchwin() for the associated window, to make sure panels are fully refreshed.
	//

	while ( !panelList.empty() )
	{
		top_panel( panelList.top() ) ;
		touchwin( panel_window( panelList.top() ) ) ;
		panelList.pop() ;
	}
}


void pLScreen::refresh_panel_stack()
{
	save_panel_stack() ;
	restore_panel_stack() ;
}


void pLScreen::set_frames_inactive( uint intens )
{
	//
	// Set all window frames for this logical screen, except the top, to the inactive colour
	// until a full screen window is reached.
	//

	bool top_frame  = true ;
	bool top_window = true ;

	const void* vptr ;
	const panel_data* pd ;

	PANEL* pnl = panel_below( nullptr ) ;

	while ( pnl )
	{
		vptr = panel_userptr( pnl ) ;
		if ( vptr )
		{
			pd = static_cast<const panel_data*>(vptr) ;
			if ( pd->screenid() == screenId && pd->ppanel() )
			{
				if ( pd->is_fscreen() )
				{
					if ( !top_window )
					{
						pd->ppanel()->draw_msgframes( cuaAttr[ IWF ] ) ;
					}
					break ;
				}
				if ( pd->is_frame() )
				{
					if ( !top_frame && pd->is_frame_act() )
					{
						pd->ppanel()->draw_frame( pnl, cuaAttr[ IWF ] ) ;
						pd->set_frame_inact() ;
					}
					top_frame = false ;
				}
				else
				{
					top_window = false ;
				}
			}
		}
		pnl = panel_below( pnl ) ;
	}
}


void pLScreen::decolourise_inactive( uint col1,
				     uint col2,
				     uint intens )
{
	//
	// Set all windows for this logical screen, except the top, to the inactive colour
	// until a full screen window is reached.
	//

	bool top_panel = true ;
	bool top_frame = true ;

	const void* vptr ;
	const panel_data* pd ;

	PANEL* pnl = panel_below( nullptr ) ;

	while ( pnl )
	{
		vptr = panel_userptr( pnl ) ;
		if ( vptr )
		{
			pd = static_cast<const panel_data*>(vptr) ;
			if ( pd->screenid() == screenId && pd->ppanel() )
			{
				if ( pd->is_not_frame() )
				{
					if ( !top_panel && !pd->is_decolourised() )
					{
						pd->ppanel()->decolourise( panel_window( pnl ), col1, col2, intens ) ;
						pd->set_decolourised() ;
					}
					top_panel = false ;
				}
				if ( pd->is_fscreen() )
				{
					break ;
				}
				if ( pd->is_frame() )
				{
					if ( !top_frame && pd->is_frame_act() )
					{
						pd->ppanel()->draw_frame( pnl, col2 ) ;
						pd->set_frame_inact() ;
					}
					top_frame = false ;
				}
			}
		}
		pnl = panel_below( pnl ) ;
	}
}


void pLScreen::decolourise_all( uint col1,
				uint col2,
				uint intens )
{
	//
	// Set all windows for this logical screen to the inactive colour
	// until a full screen window is reached.
	//

	const void* vptr ;
	const panel_data* pd ;

	PANEL* pnl = panel_below( nullptr ) ;

	while ( pnl )
	{
		vptr = panel_userptr( pnl ) ;
		if ( vptr )
		{
			pd = static_cast<const panel_data*>(vptr) ;
			if ( pd->screenid() == screenId && pd->ppanel() )
			{
				if ( pd->is_not_frame() && !pd->is_decolourised() )
				{
					pd->ppanel()->decolourise( panel_window( pnl ), col1, col2, intens ) ;
					pd->set_decolourised() ;
				}
				if ( pd->is_fscreen() )
				{
					break ;
				}
				if ( pd->is_frame() && pd->is_frame_act() )
				{
					pd->ppanel()->draw_frame( pnl, col2 ) ;
					pd->set_frame_inact() ;
				}
			}
		}
		pnl = panel_below( pnl ) ;
	}
}


void pLScreen::colourise_all( errblock& err )
{
	//
	// Colourise all windows for this logical screen until a full screen window is reached.
	// Note:  pPanel::redraw_panel() changes the panel stack order, so store pPanels affected before calling.
	//

	const void* vptr ;
	const panel_data* pd ;

	stack<pPanel*>pl ;

	PANEL* pnl = panel_below( nullptr ) ;

	while ( pnl )
	{
		vptr = panel_userptr( pnl ) ;
		if ( vptr )
		{
			pd = static_cast<const panel_data*>(vptr) ;
			if ( pd->screenid() == screenId && pd->ppanel() )
			{
				if ( pd->is_not_frame() && pd->is_decolourised() )
				{
					pl.push( pd->ppanel() ) ;
				}
				if ( pd->is_fscreen() )
				{
					break ;
				}
			}
		}
		pnl = panel_below( pnl ) ;
	}

	while ( !pl.empty() )
	{
		pl.top()->redraw_panel( err ) ;
		pl.pop() ;
	}
}


void pLScreen::set_cursor( pApplication* appl )
{
	//
	// Set the logical screen cursor to that held by the application.
	//

	appl->get_pcursor( row, col ) ;
}


void pLScreen::set_appl_cursor( pApplication* appl )
{
	//
	// Set the application cursor to that held by the logical screen.
	//

	appl->set_pcursor( row, col ) ;
}


void pLScreen::OIA_update( uint priScreen,
			   uint altScreen,
			   bool showLock )
{
	int pos ;

	string respTime ;

	respTime = to_iso_string( endTime - startTime ) ;
	pos      = respTime.find_last_of( '.' ) - 1 ;
	respTime = substr( respTime, pos, 6 ) + " s" ;

	wattrset( OIA, YELLOW ) ;
	mvwaddstr( OIA, 1, 9,  "        " ) ;
	mvwaddstr( OIA, 1, 9, substr( "12345678", 1, screensTotal).c_str() ) ;
	mvwaddstr( OIA, 1, 26, "   " ) ;
	mvwaddstr( OIA, 1, 39, respTime.c_str() ) ;
	mvwprintw( OIA, 1, 58, "%d-%d   ", screenId, application_stack_size() );
	mvwaddstr( OIA, 1, maxcol-23, Insert ? "Insert" : "      " ) ;
	mvwprintw( OIA, 1, maxcol-15, "Row %d Col %d   ", row+1, col+1 ) ;

	if ( priScreen < 8 )
	{
		wattrset( OIA, RED | A_REVERSE ) ;
		mvwaddch( OIA, 1, priScreen+9, d2ds( priScreen+1 ).front() ) ;
	}

	if ( altScreen < 8 && altScreen != priScreen )
	{
		wattrset( OIA, YELLOW | A_BOLD | A_UNDERLINE ) ;
		mvwaddch( OIA, 1, altScreen+9, d2ds( altScreen+1 ).front() ) ;
	}

	if ( showLock )
	{
		wattrset( OIA,  RED ) ;
		mvwaddstr( OIA, 1, 26, "|X|" ) ;
	}
}
