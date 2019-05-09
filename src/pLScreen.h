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
	public:
		 pLScreen( int ) ;
		~pLScreen() ;

	static pLScreen* currScreen ;
	static WINDOW*   OIA        ;
	static PANEL*    OIA_panel  ;

	static unsigned int screensTotal ;
	static unsigned int maxrow ;
	static unsigned int maxcol ;
	static unsigned int maxscrn ;
	static map<int,int> openedByList ;

	void  clear() ;

	int   get_row()       { return row ; }
	int   get_col()       { return col ; }

	void  get_cursor( uint& a, uint& b ) { a = row ; b = col ; }
	void  set_cursor( int a, int b )     { row = a ; col = b ; }
	void  set_cursor( pApplication* ) ;

	int   get_openedBy()                 { return openedByList[ screenId ] ; }
	int   get_screenNum()                { return screenNum ; }
	int   get_priScreen( int ) ;

	void  cursor_left()   { col == 0 ? col = maxcol-1 : --col ; }
	void  cursor_right()  { col == maxcol-1 ? col = 0 : ++col ; }
	void  cursor_up()     { row == 0 ? row = maxrow-1 : --row ; }
	void  cursor_down()   { row == maxrow-1 ? row = 0 : ++row ; }

	void  application_add( pApplication* pApplication )  { pApplicationStack.push( pApplication ) ; }
	void  application_remove_current()                   { pApplicationStack.pop() ; } ;
	pApplication* application_get_current()              { return pApplicationStack.top()   ; }
	int   application_stack_size()                       { return pApplicationStack.size()  ; }
	bool  application_stack_empty()                      { return pApplicationStack.empty() ; }

	void  set_Insert( bool ins )       { Insert = ins ; Insert ? curs_set(2) : curs_set(1)  ; }

	void  OIA_setup()  ;
	void  OIA_update( int, int ) ;
	void  OIA_startTime() { startTime = boost::posix_time::microsec_clock::universal_time() ; }
	void  OIA_endTime()   { endTime   = boost::posix_time::microsec_clock::universal_time() ; }
	void  OIA_refresh() ;

	void  show_enter()  ;
	void  show_busy()   ;
	void  show_wait()   ;
	void  show_auto()   ;
	void  show_lock( bool ) ;
	void  clear_status() ;

	void  save_panel_stack()    ;
	void  restore_panel_stack() ;
	void  refresh_panel_stack() ;

	int   screenId  ;

private:
	static unsigned int maxScreenId ;
	static set<int> screenNums ;

	unsigned int row ;
	unsigned int col ;

	int  screenNum ;

	bool Insert ;

	boost::posix_time::ptime startTime ;
	boost::posix_time::ptime endTime   ;

	stack<pApplication*> pApplicationStack ;
	stack<PANEL*> panelList ;
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
