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

class pLScreen
{
	private:

	static bool busy ;
	static unsigned int maxscrn ;
	static unsigned int maxScreenId ;
	static boost::posix_time::ptime startTime ;
	static boost::posix_time::ptime endTime   ;
	static set<uint> screenNums ;
	static map<uint,uint> openedByList ;

	unsigned int row ;
	unsigned int col ;
	unsigned int screenNum ;
	unsigned int screenId ;

	bool Insert ;

	stack<pApplication*> pApplicationStack ;
	stack<PANEL*> panelList ;

	public:

	pLScreen( uint ) ;
	~pLScreen() ;

	static unsigned int screensTotal ;
	static unsigned int maxrow ;
	static unsigned int maxcol ;

	static pLScreen* currScreen ;
	static WINDOW*   OIA        ;
	static PANEL*    OIA_panel  ;

	static void OIA_setup()
	{
		wattrset( OIA, WHITE ) ;
		mvwhline( OIA, 0, 0, ACS_HLINE, maxcol ) ;
		mvwaddch( OIA, 1, 0, ACS_CKBOARD ) ;
		wattrset( OIA, YELLOW ) ;
		mvwaddstr( OIA, 1, 2,  "Screen[        ]" ) ;
		mvwaddstr( OIA, 1, 30, "Elapsed:" ) ;
		mvwaddstr( OIA, 1, 50, "Screen:" ) ;
	}

	static void OIA_startTime()
	{
		startTime = boost::posix_time::microsec_clock::universal_time() ;
	}

	static void OIA_endTime()
	{
		endTime = boost::posix_time::microsec_clock::universal_time() ;
	}

	static void OIA_refresh()
	{
		top_panel( OIA_panel ) ;
		touchwin( OIA ) ;
	}

	static uint get_priScreen( uint openedBy )
	{
		uint i = 0 ;

		if ( openedBy == 0 ) { return 0 ; }

		for ( auto it = openedByList.begin() ; it != openedByList.end() ; ++it, ++i )
		{
			if ( it->first == openedBy ) { return i ; }
		}
		return 0 ;
	}

	static void show_enter()
	{
		wattrset( OIA, RED ) ;
		mvwaddstr( OIA, 1, 19, "X-Enter" ) ;
		wmove( OIA, 1, 0 ) ;
		wrefresh( OIA )    ;
	}

	static void show_busy()
	{
		if ( not busy )
		{
			wattrset( OIA, RED ) ;
			mvwaddstr( OIA, 1, 19, "X-Busy " ) ;
			wmove( OIA, 1, 0 ) ;
			wrefresh( OIA ) ;
			busy = true ;
		}
	}


	static void clear_busy()
	{
		busy = false ;
	}

	static void show_wait()
	{
		wattrset( OIA, RED ) ;
		mvwaddstr( OIA, 1, 19, "X-Wait " ) ;
		wmove( OIA, 1, 0 ) ;
		wrefresh( OIA )    ;
	}

	static void show_auto()
	{
		wattrset( OIA, RED ) ;
		mvwaddstr( OIA, 1, 19, "X-Auto " ) ;
		wmove( OIA, 1, 0 ) ;
		wrefresh( OIA )    ;
	}

	static void clear_status()
	{
		wstandend( OIA ) ;
		mvwaddstr( OIA, 1, 19, "       " ) ;
	}

	uint  get_row()   { return row ; }
	uint  get_col()   { return col ; }

	void  get_cursor( uint& a, uint& b )  { a = row ; b = col ; }
	void  set_cursor( uint a, uint b )    { row = a ; col = b ; }
	void  set_cursor( pApplication* ) ;

	uint  get_openedBy()   { return openedByList[ screenId ] ; }
	uint  get_screenNum()  { return screenNum ; }

	void  cursor_left()  ;
	void  cursor_right() ;
	void  cursor_up()    ;
	void  cursor_down()  ;

	void  application_add( pApplication* pApplication )  { pApplicationStack.push( pApplication ) ; }
	void  application_remove_current()                   { pApplicationStack.pop() ; } ;
	pApplication* application_get_current()              { return pApplicationStack.top()   ; }
	uint  application_stack_size()                       { return pApplicationStack.size()  ; }
	bool  application_stack_empty()                      { return pApplicationStack.empty() ; }

	void  set_Insert( bool ins )
	{
		Insert = ins ;
		Insert ? curs_set( 2 ) : curs_set( 1 ) ;
	}

	void  OIA_update( uint, uint, bool = false ) ;

	void  save_panel_stack()    ;
	void  restore_panel_stack() ;
	void  refresh_panel_stack() ;

	void  set_frame_inactive( uint ) ;
	void  decolourise_inactive( uint, uint, uint ) ;

	unsigned int screenid()     { return screenId ; }

} ;


#undef  llog
#undef  debug1
#undef  debug2

#define llog(t, s) \
{ \
lg->lock() ; \
(*lg) << microsec_clock::local_time() << \
" " << left( quotes(MOD_NAME), 10 ) << \
" 00000 " << t << " " << s ; \
lg->unlock() ; \
}

#ifdef DEBUG1
#define debug1( s ) \
{ \
lg->lock() ; \
(*lg) << microsec_clock::local_time() << \
" " << left( quotes(MOD_NAME), 10 ) << \
" 00000 D line: "  << __LINE__  << \
" >>L1 Function: " << __FUNCTION__ << \
" -  " << s ; \
lg->unlock() ; \
}
#else
#define debug1( s )
#endif


#ifdef DEBUG2
#define debug2( s ) \
{ \
lg->lock() ; \
(*lg) << microsec_clock::local_time() << \
" " << left( quotes(MOD_NAME), 10 ) << \
" 00000 D line: "  << __LINE__  << \
" >>L2 Function: " << __FUNCTION__ << \
" -  " << s ; \
lg->unlock() ; \
}
#else
#define debug2( s )
#endif
