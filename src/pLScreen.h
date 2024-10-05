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
		static bool w_refresh ;
		static unsigned int maxscrn ;
		static unsigned int maxScreenId ;
		static boost::posix_time::ptime startTime ;
		static boost::posix_time::ptime endTime ;
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
		static uint lscreen_intens ;
		static bool swapbar ;

		static pLScreen* currScreen ;
		static WINDOW*   OIA        ;
		static PANEL*    OIA_panel  ;
		static WINDOW*   SWB        ;
		static PANEL*    SWB_panel  ;

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

		static void OIA_SWB_restart()
		{
			del_panel( OIA_panel ) ;
			delwin( OIA ) ;

			del_panel( SWB_panel ) ;
			delwin( SWB ) ;

			OIA       = newwin( 2, maxcol, maxrow, 0 ) ;
			OIA_panel = new_panel( OIA ) ;
			SWB       = newwin( 1, maxcol, maxrow-1, 0 ) ;
			SWB_panel = new_panel( SWB ) ;
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

		static void SWB_show( const string& s )
		{

			if ( swapbar )
			{
				wattrset( SWB, WHITE | lscreen_intens ) ;
				mvwaddstr( SWB, 0, 1, left( s, maxcol ).c_str() ) ;
				top_panel( SWB_panel ) ;
			}
			else
			{
				hide_panel( SWB_panel ) ;
			}
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
			if ( !w_refresh )
			{
				wrefresh( OIA ) ;
				w_refresh = true ;
			}
		}

		static void show_busy()
		{
			if ( not busy )
			{
				wattrset( OIA, RED ) ;
				mvwaddstr( OIA, 1, 19, "X-Busy " ) ;
				wmove( OIA, 1, 0 ) ;
				if ( !w_refresh )
				{
					wrefresh( OIA ) ;
					w_refresh = true ;
				}
				busy = true ;
			}
		}


		static void set_busy()
		{
			busy = true ;
		}

		static void clear_busy()
		{
			busy = false ;
		}

		static void clear_w_refresh()
		{
			w_refresh = false ;
		}

		static void show_wait()
		{
			wattrset( OIA, RED ) ;
			mvwaddstr( OIA, 1, 19, "X-Wait " ) ;
			wmove( OIA, 1, 0 ) ;
			wrefresh( OIA ) ;
		}

		static void show_auto()
		{
			wattrset( OIA, RED ) ;
			mvwaddstr( OIA, 1, 19, "X-Auto " ) ;
			wmove( OIA, 1, 0 ) ;
			if ( !w_refresh )
			{
				wrefresh( OIA ) ;
				w_refresh = true ;
			}
		}

		static void show_esc()
		{
			wattrset( OIA, RED ) ;
			mvwaddstr( OIA, 1, 19, "X-Esc  " ) ;
			wmove( OIA, 1, 0 ) ;
			wrefresh( OIA ) ;
		}

		static void clear_status()
		{
			wstandend( OIA ) ;
			mvwaddstr( OIA, 1, 19, "       " ) ;
		}

		static void set_max_row_col_size()
		{
			getmaxyx( stdscr, maxrow, maxcol ) ;
			maxrow -= 2 ;

		}

		uint  get_row()           { return row ; }
		uint  get_col()           { return col ; }

		void  set_row( uint i )   { row = i ; }
		void  set_col( uint i )   { col = i ; }

		void  get_cursor( uint& a, uint& b )  { a = row ; b = col ; }
		void  set_cursor( uint a, uint b )    { row = a ; col = b ; }
		void  set_cursor( pApplication* ) ;

		void  set_appl_cursor( pApplication* ) ;

		uint  get_openedBy()   { return openedByList[ screenId ] ; }
		uint  get_screenNum()  { return screenNum ; }

		void  cursor_left() ;
		void  cursor_left_cond() ;
		void  cursor_right() ;
		void  cursor_right_cond() ;
		void  cursor_up() ;
		void  cursor_up_cond() ;
		void  cursor_down() ;
		void  cursor_down_cond() ;

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

		void  OIA_update( uint,
				  uint,
				  bool = false ) ;

		void  save_panel_stack() ;
		void  restore_panel_stack() ;
		void  refresh_panel_stack() ;

		void  set_frames_inactive( uint ) ;
		void  decolourise_inactive( uint,
					    uint,
					    uint ) ;

		void  decolourise_all( uint,
				       uint,
				       uint ) ;

		void  colourise_all( errblock& ) ;

		unsigned int screenid() { return screenId ; }

		string modname() { return "SCREEN" ; }

		lss* p_lss ;

		lss* get_lss() { return p_lss ; }

		void resize()
		{
			stack<pApplication*> tempStack = pApplicationStack ;
			while ( !tempStack.empty() )
			{
				tempStack.top()->term_resize() ;
				tempStack.pop() ;
			}
		}

} ;
