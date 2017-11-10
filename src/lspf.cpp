/*  Compile with ::                                                                                                                                          */
/* g++ -O0 -std=c++11 -rdynamic -Wunused-variable -ltinfo -lncurses -lpanel -lboost_thread -lboost_filesystem -lboost_system -ldl -lpthread -o lspf lspf.cpp */

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

#include "lspf.h"

#include <ncurses.h>
#include <dlfcn.h>
#include <sys/utsname.h>

#include <locale>
#include <boost/date_time.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/filesystem.hpp>

boost::condition cond_appl ;
boost::mutex global ;


#include "utilities.h"
#include "utilities.cpp"

map<string, cuaType> cuaAttrName   ;
map<cuaType, unsigned int> cuaAttr ;
map<int, unsigned int> usrAttr     ;
map<cuaType, bool> cuaAttrProt     ;

const string usrAttrNames = "N_RED N_GREEN N_YELLOW N_BLUE N_MAGENTA N_TURQ N_WHITE " \
			    "B_RED B_GREEN B_YELLOW B_BLUE B_MAGENTA B_TURQ B_WHITE " \
			    "R_RED R_GREEN R_YELLOW R_BLUE R_MAGENTA R_TURQ R_WHITE " \
			    "U_RED U_GREEN U_YELLOW U_BLUE U_MAGENTA U_TURQ U_WHITE " \
			    "P_RED P_GREEN P_YELLOW P_BLUE P_MAGENTA P_TURQ P_WHITE"  ;

#include "classes.h"
#include "classes.cpp"

#include "pWidgets.h"
#include "pWidgets.cpp"

#include "pVPOOL.h"
#include "pVPOOL.cpp"

#include "pTable.h"
#include "pTable.cpp"

#include "pPanel.h"
#include "pPanel1.cpp"
#include "pPanel2.cpp"

#include "pApplication.h"
#include "pApplication.cpp"

#include "ispexeci.cpp"

#include "pLScreen.h"
#include "pLScreen.cpp"

map<int,string> pfKeyDefaults = { {  1, "HELP"      },
				  {  2, "SPLIT NEW" },
				  {  3, "END"       },
				  {  4, "RETURN"    },
				  {  5, "RFIND"     },
				  {  6, "RCHANGE"   },
				  {  7, "UP"        },
				  {  8, "DOWN"      },
				  {  9, "SWAP"      },
				  { 10, "LEFT"      },
				  { 11, "RIGHT"     },
				  { 12, "RETRIEVE"  },
				  { 13, "HELP"      },
				  { 14, "SPLIT NEW" },
				  { 15, "END"       },
				  { 16, "RETURN"    },
				  { 17, "RFIND"     },
				  { 18, "RCHANGE"   },
				  { 19, "UP"        },
				  { 20, "DOWN"      },
				  { 21, "SWAP"      },
				  { 22, "SWAP PREV" },
				  { 23, "SWAP NEXT" },
				  { 24, "HELP"      } } ;

#undef  MOD_NAME
#undef  LOGOUT
#define MOD_NAME lspf
#define LOGOUT   splog

#define currScrn pLScreen::currScreen
#define OIA      pLScreen::OIA

using namespace std ;
using namespace boost::filesystem ;

int pLScreen::screensTotal = 0 ;
int pLScreen::maxScreenId  = 0 ;
int pLScreen::maxrow       = 0 ;
int pLScreen::maxcol       = 0 ;

WINDOW * pLScreen::OIA     = NULL ;

boost::posix_time::ptime startTime ;
boost::posix_time::ptime endTime   ;

pLScreen     * pLScreen::currScreen = NULL ;
pApplication * currAppl ;

poolMGR  * p_poolMGR  = new poolMGR  ;
tableMGR * p_tableMGR = new tableMGR ;

fPOOL funcPOOL ;

void initialSetup()      ;
void loadDefaultPools()  ;
void getDynamicClasses() ;
bool loadDynamicClass( const string& ) ;
bool unloadDynamicClass( void * ) ;
void reloadDynamicClasses( string ) ;
void loadSystemCommandTable() ;
void loadCUATables()          ;
void setColourPair( const string& ) ;
void updateDefaultVars()      ;
void updateReflist()          ;
void startApplication( selobj, bool =false ) ;
void terminateApplication()     ;
void ResumeApplicationAndWait() ;
bool createLogicalScreen()      ;
void deleteLogicalScreen()      ;
void processPGMSelect()         ;
void processAction( uint row, uint col, int c, bool& passthru ) ;
void issueMessage( const string& ) ;
void rawOutput()          ;
void threadErrorHandler() ;
void errorScreen( int, const string& )   ;
void abortStartup()                      ;
void lspfCallbackHandler( lspfCommand& ) ;
void createpfKeyDefaults()  ;
string pfKeyValue( int )    ;
string listLogicalScreens() ;
bool resolveZCTEntry( string&, string& ) ;
bool isActionKey( int c )   ;
void listRetrieveBuffer()   ;
int  getScreenNameNum( const string& ) ;
void mainLoop() ;

vector<pLScreen *> screenList ;

struct appInfo
{
	string file       ;
	string module     ;
	void * dlib       ;
	void * maker_ep   ;
	void * destroyer_ep ;
	bool   mainpgm    ;
	bool   dlopened   ;
	bool   errors     ;
	bool   relPending ;
	int    refCount   ;
} ;

map<string, appInfo> apps ;

boost::circular_buffer<string> retrieveBuffer(10) ;

int    maxtaskID = 0 ;
int    screenNum = 0 ;
int    altScreen = 0 ;
string commandStack  ;
string jumpOption    ;
bool   pfkeyPressed  ;
bool   wmPending     ;

string ZCOMMAND ;
string ZPARM    ;
string ZTLIB    ;

string ZCTVERB  ;
string ZCTTRUNC ;
string ZCTACT   ;
string ZCTDESC  ;

const char ZSCREEN[] = { '1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G',
			 'H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W' } ;

bool   SEL   ;
selobj SELCT ;

int    GMAXWAIT ;
string GMAINPGM ;

const string BuiltInCommands = "ACTION DISCARD FIELDEXC MSGID NOP PANELID REFRESH RESIZE RETP SCRNAME SPLIT SWAP TDOWN USERID" ;
const string SystemCommands  = ".ABEND .HIDE .INFO .LOAD .RELOAD .SCALE .SHELL .SHOW .SNAP .STATS .TEST" ;

std::ofstream splog(SLOG) ;


int main( void )
{
	int  elapsed ;
	uint row     ;
	uint col     ;

	errblock err ;

	map<string, appInfo>::iterator it ;

	boost::thread * pThread              ;
	set_terminate ( threadErrorHandler ) ;

	startTime    = boost::posix_time::microsec_clock::universal_time() ;
	commandStack = ""    ;
	wmPending    = false ;

	screenList.push_back( new pLScreen ) ;

	llog( "I", "lspf startup in progress" << endl ) ;

	llog( "I", "Calling initialSetup" << endl ) ;
	initialSetup() ;

	llog( "I", "Calling loadDefaultPools" << endl ) ;
	loadDefaultPools() ;

	llog( "I", "Calling getDynamicClasses" << endl ) ;
	getDynamicClasses() ;

	llog( "I", "Loading main "+ GMAINPGM +" application" << endl ) ;
	loadDynamicClass( GMAINPGM ) ;

	llog( "I", "Calling loadCUATables" << endl ) ;
	loadCUATables() ;

	llog( "I", "Calling loadSystemCommandTable" << endl ) ;
	loadSystemCommandTable() ;

	updateDefaultVars() ;

	llog( "I", "Starting new "+ GMAINPGM +" thread" << endl ) ;
	currAppl = ((pApplication*(*)())( apps[ GMAINPGM ].maker_ep))() ;

	currScrn->application_add( currAppl ) ;
	currAppl->taskid( ++maxtaskID )       ;
	currAppl->ZAPPNAME     = GMAINPGM     ;
	currAppl->p_poolMGR    = p_poolMGR    ;
	currAppl->p_tableMGR   = p_tableMGR   ;
	currAppl->ZZAPPLID     = "ISP"        ;
	currAppl->NEWPOOL      = true         ;
	currAppl->lspfCallback = lspfCallbackHandler ;

	p_poolMGR->put( err, "ZSCREEN", string( 1, ZSCREEN[ screenNum ] ), SHARED, SYSTEM ) ;
	p_poolMGR->put( err, "ZSCRNUM", d2ds( currScrn->screenId ), SHARED, SYSTEM ) ;
	p_poolMGR->put( err, "ZAPPLID", "ISP", SHARED, SYSTEM ) ;
	currAppl->shrdPool = p_poolMGR->getShrdPool() ;
	currAppl->init() ;

	pThread = new boost::thread(boost::bind(&pApplication::application, currAppl ) ) ;
	currAppl->pThread = pThread ;
	apps[ GMAINPGM ].refCount++ ;
	apps[ GMAINPGM ].mainpgm = true ;

	llog( "I", "Waiting for "+ GMAINPGM +" to complete startup" << endl ) ;
	elapsed = 0 ;
	while ( currAppl->busyAppl )
	{
		elapsed++ ;
		boost::this_thread::sleep_for(boost::chrono::milliseconds(ZWAIT)) ;
		if ( elapsed > GMAXWAIT ) { currAppl->set_timeout_abend() ; }
	}
	if ( currAppl->terminateAppl )
	{
		errorScreen( 1, "An error has occured initialising the first "+ GMAINPGM +" main task.  lspf cannot continue." ) ;
		currAppl->info() ;
		currAppl->closeTables() ;
		llog( "I", "Removing application instance of "+ currAppl->ZAPPNAME << endl ) ;
		((void (*)(pApplication*))(apps[ currAppl->ZAPPNAME ].destroyer_ep))( currAppl )  ;
		delete pThread    ;
		delete p_poolMGR  ;
		delete p_tableMGR ;
		delete currScrn   ;
		llog( "I", "lspf and LOG terminating" << endl ) ;
		splog.close() ;
		return 0 ;
	}
	llog( "I", "First thread "+ GMAINPGM +" started and initialised.  ID=" << pThread->get_id() << endl ) ;

	currAppl->get_cursor( row, col )  ;
	currScrn->set_row_col( row, col ) ;

	mainLoop() ;

	delete p_poolMGR  ;
	delete p_tableMGR ;

	for ( it = apps.begin() ; it != apps.end() ; it++ )
	{
		if ( it->second.dlopened )
		{
			llog( "I", "dlclose of "+ it->first +" at " << it->second.dlib << endl ) ;
			unloadDynamicClass( it->second.dlib ) ;
		}
	}

	llog( "I", "lspf and LOG terminating" << endl ) ;
	splog.close() ;
	return 0 ;
}


void mainLoop()
{
	llog( "I", "mainLoop() entered" << endl ) ;

	int RC   ;
	int pos  ;
	int ws   ;
	int i    ;
	int l    ;

	uint t   ;
	uint c   ;
	uint row ;
	uint col ;

	bool passthru ;
	bool showLock ;
	bool Insert   ;

	string respTime   ;
	string field_name ;
	string w1         ;
	string w2         ;

	fieldExc fxc ;
	MEVENT event ;

	errblock err ;

	currScrn->OIA_setup() ;

	mousemask( ALL_MOUSE_EVENTS, NULL ) ;
	showLock = false ;
	Insert   = false ;

	set_escdelay( 25 ) ;

	while ( true )
	{
		if ( pLScreen::screensTotal == 0 ) { return ; }

		currScrn->clear_status() ;

		row = currScrn->get_row() ;
		col = currScrn->get_col() ;

		endTime  = boost::posix_time::microsec_clock::universal_time() ;
		respTime = to_iso_string(endTime - startTime) ;
		pos      = lastpos( ".", respTime ) ;
		respTime = substr( respTime, (pos - 2) , 6 ) + " s" ;
		currScrn->OIA_update( respTime.c_str() ) ;

		if ( screenNum < 8 )
		{
			wattrset( OIA, RED | A_REVERSE ) ;
			mvwaddch( OIA, 0, 9+screenNum, d2ds( screenNum+1 )[0] ) ;
			wattroff( OIA, RED | A_REVERSE ) ;
		}
		if ( altScreen < 8 && altScreen != screenNum )
		{
			wattrset( OIA, YELLOW | A_BOLD | A_UNDERLINE ) ;
			mvwaddch( OIA, 0, 9+altScreen, d2ds( altScreen+1 )[0] ) ;
			wattroff( OIA, YELLOW | A_BOLD | A_UNDERLINE ) ;
		}
		if ( showLock )
		{
			wattrset( OIA,  RED ) ;
			mvwaddstr( OIA, 0, 27, "|X|" ) ;
			wattroff( OIA,  RED ) ;
			showLock = false ;
		}
		pfkeyPressed = false ;
		if ( commandStack == ""            &&
		     !currAppl->isRawOutput()      &&
		     !currAppl->ControlDisplayLock &&
		     !currAppl->ControlNonDispl )
		{
			wnoutrefresh( stdscr ) ;
			wnoutrefresh( OIA ) ;
			update_panels() ;
			move( row, col ) ;
			doupdate()  ;
			c = getch() ;
			if ( c == 13 ) { c = KEY_ENTER ; }
		}
		else
		{
			if ( currAppl->ControlDisplayLock )
			{
				wnoutrefresh( stdscr ) ;
				wnoutrefresh( OIA ) ;
				update_panels()  ;
				move( row, col ) ;
				doupdate()  ;
			}
			c = KEY_ENTER ;
		}
		startTime = boost::posix_time::microsec_clock::universal_time() ;

		if ( c < 256 )
		{
			if ( isprint( c ) )
			{
				if ( currAppl->inputInhibited() ) { continue ; }
				currAppl->currPanel->field_edit( row, col, char( c ), Insert, showLock ) ;
				currAppl->currPanel->get_cursor( row, col ) ;
				currScrn->set_row_col( row, col ) ;
				continue ;
			}
		}
		if ( c == KEY_MOUSE )
		{
			if ( getmouse( &event ) == OK )
			{
				if (event.bstate & BUTTON1_CLICKED)
				{
					currScrn->set_row_col( event.y, event.x ) ;
					continue ;
				}
				if ( event.bstate & BUTTON1_DOUBLE_CLICKED )
				{
					row = event.y ;
					col = event.x ;
					currScrn->set_row_col( row, col ) ;
					c = KEY_ENTER ;
				}
			}
		}
		switch( c )
		{
			case KEY_LEFT:  currScrn->cursor_left()  ; break ;
			case KEY_RIGHT: currScrn->cursor_right() ; break ;
			case KEY_UP:    currScrn->cursor_up()    ; break ;
			case KEY_DOWN:  currScrn->cursor_down()  ; break ;
			case 9:   // Tab key
				currAppl->currPanel->field_tab_next( row, col ) ;
				currScrn->set_row_col( row, col ) ;
				break ;

			case KEY_IC:
				Insert = !Insert ;
				currScrn->set_Insert( Insert ) ;
				break ;

			case KEY_HOME:
				currAppl->get_home( row, col ) ;
				currScrn->set_row_col( row, col ) ;
				break ;

			case KEY_DC:
				if ( currAppl->inputInhibited() ) { break ; }
				currAppl->currPanel->field_delete_char( row, col, showLock ) ;
				break ;

			case KEY_SDC:
				if ( currAppl->inputInhibited() ) { break ; }
				currAppl->currPanel->field_erase_eof( row, col, showLock ) ;
				break ;

			case KEY_END:
				currAppl->currPanel->cursor_eof( row, col ) ;
				currScrn->set_row_col( row, col ) ;
				break ;

			case 127:
			case KEY_BACKSPACE:
				if ( currAppl->inputInhibited() ) { break ; }
				currAppl->currPanel->field_backspace( row, col, showLock ) ;
				currScrn->set_row_col( row, col ) ;
				break ;

			// All action keys follow
			case KEY_F(1):  case KEY_F(2):  case KEY_F(3):  case KEY_F(4):  case KEY_F(5):  case KEY_F(6):
			case KEY_F(7):  case KEY_F(8):  case KEY_F(9):  case KEY_F(10): case KEY_F(11): case KEY_F(12):
			case KEY_F(13): case KEY_F(14): case KEY_F(15): case KEY_F(16): case KEY_F(17): case KEY_F(18):
			case KEY_F(19): case KEY_F(20): case KEY_F(21): case KEY_F(22): case KEY_F(23): case KEY_F(24):
				pfkeyPressed = true ;
			case KEY_NPAGE:
			case KEY_PPAGE:
			case KEY_ENTER:
			case 27:       // Escape key

				debug1( "Action key pressed.  Processing" << endl ) ;
				if ( currAppl->msgInhibited() )
				{
					currAppl->msgResponseOK() ;
					break ;
				}
				Insert = false ;
				currScrn->set_Insert( Insert ) ;
				currScrn->show_busy() ;
				updateDefaultVars()   ;
				processAction( row, col, c, passthru ) ;
				if ( passthru )
				{
					currAppl->set_cursor( row, col ) ;
					ResumeApplicationAndWait()       ;
					if ( currAppl->reloadCUATables ) { loadCUATables() ; }
					if ( currAppl->do_refresh_lscreen() )
					{
						currScrn->save_panel_stack() ;
						currScrn->restore_panel_stack() ;
					}
					currAppl->get_cursor( row, col )  ;
					currScrn->set_row_col( row, col ) ;
					while ( currAppl->terminateAppl )
					{
						terminateApplication() ;
						if ( pLScreen::screensTotal == 0 ) { return ; }
						if ( currAppl->SEL && !currAppl->terminateAppl )
						{
							debug1( "Application "+ currAppl->ZAPPNAME +" has done another SELECT without a DISPLAY (1) !!!! " << endl ) ;
							processPGMSelect() ;
						}
					}
					updateReflist() ;
					if ( !currAppl->selectPanel() ) { continue ; }
				}
				if ( ZCOMMAND == ".ABEND" )
				{
					currAppl->set_forced_abend() ;
					ResumeApplicationAndWait()   ;
					if ( currAppl->abnormalEnd )
					{
						terminateApplication() ;
						if ( pLScreen::screensTotal == 0 ) { return ; }
					}
				}
				else if ( ZCOMMAND == "ACTION" )
				{
					currAppl->currPanel->display_next_pd() ;
					currAppl->currPanel->get_cursor( row, col ) ;
					currScrn->set_row_col( row, col ) ;
					break ;
				}
				else if ( ZCOMMAND == "DISCARD" )
				{
					currAppl->currPanel->refresh_fields() ;
				}
				else if ( ZCOMMAND == "FIELDEXC" )
				{

					field_name = currAppl->currPanel->field_getname( row, col ) ;
					if ( field_name == "" )
					{
						issueMessage( "PSYS012K" ) ;
						break ;
					}
					fxc = currAppl->currPanel->field_getexec( field_name ) ;
					if ( fxc.fieldExc_command != "" )
					{
						if ( !SELCT.parse( err, subword( fxc.fieldExc_command, 2 ) ) )
						{
							llog( "E", "Error in FIELD SELECT command "+ fxc.fieldExc_command << endl ) ;
							issueMessage( "PSYS011K" ) ;
							break ;
						}
						SELCT.PARM = SELCT.PARM + " " + currAppl->currPanel->field_getvalue( field_name ) ;
						currAppl->reffield = field_name ;
						currAppl->currPanel->field_get_row_col( field_name, t, c ) ;
						p_poolMGR->put( err, "ZFECSRP", d2ds( col-c+1 ), SHARED )  ;
						for( ws = words( fxc.fieldExc_passed ), i = 1 ; i <= ws ; i++ )
						{
							w1 = word( fxc.fieldExc_passed, i ) ;
							p_poolMGR->put( err, "ZFEDATA" + d2ds( i ), currAppl->currPanel->field_getvalue( w1 ) , SHARED ) ;
						}
						startApplication( SELCT ) ;
						break ;
					}
					else
					{
						issueMessage( "PSYS012J" ) ;
						break ;
					}
				}
				else if ( ZCOMMAND == ".HIDE" )
				{
					if ( ZPARM == "NULLS" )
					{
						p_poolMGR->put( err, "ZNULLS", "NO", SHARED, SYSTEM ) ;
						currAppl->currPanel->redraw_fields() ;
					}
					else
					{
						issueMessage( "PSYS012R" ) ;
					}
				}
				else if ( ZCOMMAND == ".INFO" )
				{
					currAppl->info() ;
				}
				else if ( ZCOMMAND == ".LOAD" )
				{
					map<string, appInfo>::iterator ita ;
					for ( ita = apps.begin() ; ita != apps.end() ; ita++ )
					{
						if ( !ita->second.errors && !ita->second.dlopened )
						{
							loadDynamicClass( ita->first ) ;
						}
					}
				}
				else if ( ZCOMMAND == "MSGID" )
				{
					if ( ZPARM == "" )
					{
						w1 = p_poolMGR->get( err, currScrn->screenId, "ZMSGID" ) ;
						p_poolMGR->put( err, "ZMSGID", w1, SHARED, SYSTEM ) ;
						issueMessage( "PSYS012L" ) ;
					}
					else if ( ZPARM == "ON" )
					{
						p_poolMGR->put( err, currScrn->screenId, "ZSHMSGID", "Y" ) ;
					}
					else if ( ZPARM == "OFF" )
					{
						p_poolMGR->put( err, currScrn->screenId, "ZSHMSGID", "N" ) ;
					}
					else
					{
						issueMessage( "PSYS012M" ) ;
					}
				}
				else if ( ZCOMMAND == "NOP" )
				{
					break ;
				}
				else if ( ZCOMMAND == "PANELID" )
				{
					if ( ZPARM == "" )
					{
						w1 = p_poolMGR->get( err, currScrn->screenId, "ZSHPANID" ) ;
						ZPARM = ( w1 == "Y" ) ? "OFF" : "ON" ;
					}
					if ( ZPARM == "ON" )
					{
						p_poolMGR->put( err, currScrn->screenId, "ZSHPANID", "Y" ) ;
					}
					else if ( ZPARM == "OFF" )
					{
						p_poolMGR->put( err, currScrn->screenId, "ZSHPANID", "N" ) ;
					}
					else
					{
						issueMessage( "PSYS014" ) ;
					}
					currAppl->display_id() ;
				}
				else if ( ZCOMMAND == "REFRESH" )
				{
					currAppl->currPanel->refresh() ;
					reset_prog_mode() ;
					refresh() ;
				}
				else if ( ZCOMMAND == "RETP" )
				{
					listRetrieveBuffer() ;
					break ;
				}
				else if ( ZCOMMAND == "RESIZE" )
				{
					currAppl->toggle_fscreen() ;
				}
				else if ( ZCOMMAND == ".RELOAD" )
				{
					reloadDynamicClasses( ZPARM ) ;
				}
				else if ( ZCOMMAND == ".SCALE" )
				{
					if ( ZPARM == "" ) { ZPARM = "ON" ; }
					if ( findword( ZPARM, "ON OFF" ) )
					{
						p_poolMGR->put( err, "ZSCALE", ZPARM, SHARED, SYSTEM ) ;
					}
				}
				else if ( ZCOMMAND == "SCRNAME" )
				{
					w1    = word( ZPARM, 2 ) ;
					ZPARM = word( ZPARM, 1 ) ;
					if  ( ZPARM == "ON" )
					{
						p_poolMGR->put( err, "ZSCRNAM1", "ON", SHARED, SYSTEM ) ;
					}
					else if ( ZPARM == "OFF" )
					{
						p_poolMGR->put( err, "ZSCRNAM1", "OFF", SHARED, SYSTEM ) ;
					}
					else if ( isvalidName( ZPARM ) && !findword( ZPARM, "LIST PREV NEXT" ) )
					{
						p_poolMGR->put( err, currScrn->screenId, "ZSCRNAME", ZPARM ) ;
						p_poolMGR->put( err, "ZSCRNAME", ZPARM, SHARED ) ;
						if ( w1 == "PERM" )
						{
							p_poolMGR->put( err, currScrn->screenId, "ZSCRNAM2", w1 ) ;
						}
						else
						{
							p_poolMGR->put( err, currScrn->screenId, "ZSCRNAM2", "" ) ;
						}
					}
					else
					{
						issueMessage( "PSYS013" ) ;
					}
					currAppl->display_id() ;
				}
				else if ( ZCOMMAND == ".SHELL" )
				{
					def_prog_mode()   ;
					endwin()          ;
					system( p_poolMGR->get( err, "ZSHELL", SHARED ).c_str() ) ;
					reset_prog_mode() ;
					refresh()         ;
				}
				else if ( ZCOMMAND == ".SHOW" )
				{
					if ( ZPARM == "NULLS" )
					{
						p_poolMGR->put( err, "ZNULLS", "YES", SHARED, SYSTEM ) ;
						currAppl->currPanel->redraw_fields() ;
					}
				}
				else if ( ZCOMMAND == "SPLIT" )
				{
					SELCT.clear() ;
					SELCT.PGM     = GMAINPGM ;
					SELCT.PARM    = ""    ;
					SELCT.NEWAPPL = "ISP" ;
					SELCT.NEWPOOL = true  ;
					SELCT.PASSLIB = false ;
					SELCT.SUSPEND = true  ;
					startApplication( SELCT, true ) ;
					break ;
				}
				else if ( ZCOMMAND == ".STATS" )
				{
					p_poolMGR->statistics() ;
					p_tableMGR->statistics() ;
				}
				else if ( ZCOMMAND == ".SNAP" )
				{
					p_poolMGR->snap() ;
				}
				else if ( ZCOMMAND == "SWAP" )
				{
					w1 = word( ZPARM, 1 ) ;
					w2 = word( ZPARM, 2 ) ;
					if ( ZPARM != "" && ZPARM != "NEXT" && ZPARM != "PREV" )
					{
						currAppl->currPanel->cursor_to_field( RC, currAppl->currPanel->CMDfield ) ;
						currAppl->currPanel->get_cursor( row, col ) ;
						currScrn->set_row_col( row, col ) ;
					}
					if ( w1 == "LIST" )
					{
						w1 = listLogicalScreens() ;
					}
					if ( pLScreen::screensTotal == 1 ) { continue ; }
					currScrn->save_panel_stack() ;
					if ( w1 =="*" ) { w1 = d2ds( screenNum+1 ) ; }
					if ( w1 != "" && w1 != "NEXT" && w1 != "PREV" && isvalidName( w1 ) )
					{
						l = getScreenNameNum( w1 ) ;
						if ( l > 0 ) { w1 = d2ds( l ) ; }
					}
					if ( w2 != "" && isvalidName( w2 ) )
					{
						l = getScreenNameNum( w2 ) ;
						if ( l > 0 ) { w2 = d2ds( l ) ; }
					}
					if ( w1 == "NEXT" )
					{
						++screenNum ;
						screenNum = (screenNum == pLScreen::screensTotal ? 0 : screenNum) ;
						if ( altScreen == screenNum ) { altScreen = (altScreen == 0 ? (pLScreen::screensTotal - 1) : (altScreen - 1) ) ; }
					}
					else if ( w1 == "PREV" )
					{
						--screenNum ;
						screenNum = (screenNum < 0 ? (pLScreen::screensTotal - 1) : screenNum) ;
						if ( altScreen == screenNum ) { altScreen = ((altScreen == pLScreen::screensTotal - 1) ? 0 : (altScreen + 1) )  ; }
					}
					else if ( datatype( w1, 'W' ) )
					{
						t = ds2d( w1 ) - 1 ;
						if ( t >= 0 && t < pLScreen::screensTotal )
						{
							if ( t != screenNum )
							{
								if ( w2 == "*" || t == altScreen )
								{
									altScreen = screenNum ;
								}
								screenNum = t ;
							}
						}
						else
						{
							swap( screenNum, altScreen ) ;
						}
						if ( datatype( w2, 'W' ) && w1 != w2 )
						{
							t = ds2d( w2 ) - 1 ;
							if ( t != screenNum && t >= 0 && t < pLScreen::screensTotal )
							{
								altScreen = t ;
							}
						}
					}
					else
					{
						swap( screenNum, altScreen ) ;
					}
					currScrn = screenList[ screenNum ] ;
					currAppl = currScrn->application_get_current() ;
					p_poolMGR->setApplid( err, currAppl->ZZAPPLID )   ;
					p_poolMGR->setShrdPool( err, currAppl->shrdPool ) ;
					p_poolMGR->put( err, "ZPANELID", currAppl->PANELID, SHARED, SYSTEM ) ;
					currScrn->restore_panel_stack() ;
					currAppl->display_pd() ;
					break ;
				}
				else if ( ZCOMMAND == ".TEST" )
				{
					currAppl->setTestMode() ;
					llog( "W", "Application is now running in test mode" << endl ) ;
				}
				else if ( ZCOMMAND == "TDOWN" )
				{
					currAppl->currPanel->field_tab_down( row, col ) ;
					currScrn->set_row_col( row, col ) ;
				}
				else if ( ZCOMMAND == "USERID" )
				{
					if ( ZPARM == "" )
					{
						w1 = p_poolMGR->get( err, currScrn->screenId, "ZSHUSRID" ) ;
						ZPARM = ( w1 == "Y" ) ? "OFF" : "ON" ;
					}
					if ( ZPARM == "ON" )
					{
						p_poolMGR->put( err, currScrn->screenId, "ZSHUSRID", "Y" ) ;
					}
					else if ( ZPARM == "OFF" )
					{
						p_poolMGR->put( err, currScrn->screenId, "ZSHUSRID", "N" ) ;
					}
					else
					{
						issueMessage( "PSYS012F" ) ;
					}
					currAppl->display_id() ;
				}
				currAppl->get_home( row, col ) ;
				currScrn->set_row_col( row, col ) ;
				if ( SEL )
				{
					updateDefaultVars()       ;
					currAppl->currPanel->remove_pd() ;
					startApplication( SELCT ) ;
				}
				break ;
			default:
				debug1( "Action key "<<c<<" ("<<keyname( c )<<") ignored" << endl ) ;
		}
	}
}


void initialSetup()
{
	errblock err ;

	funcPOOL.define( err, "ZCTVERB",  &ZCTVERB  ) ;
	funcPOOL.define( err, "ZCTTRUNC", &ZCTTRUNC ) ;
	funcPOOL.define( err, "ZCTACT",   &ZCTACT   ) ;
	funcPOOL.define( err, "ZCTDESC",  &ZCTDESC  ) ;
}


void processAction( uint row, uint col, int c, bool& passthru )
{
	// No actions required if application is issuing raw output (via control rdisplay flush)
	// perform lspf high-level functions - pfkey -> command
	// application/user/site/system command table entry?
	// BUILTIN command
	// System command
	// RETRIEVE
	// Jump command entered
	// !abc run abc as a program
	// @abc run abc as a REXX procedure
	// Else pass event to application

	int RC  ;
	int p1  ;

	uint rw ;
	uint cl ;
	uint rtsize ;
	uint rbsize ;

	bool addRetrieve ;

	string CMDVerb ;
	string CMDParm ;
	string PFCMD   ;
	string delm    ;
	string AVerb   ;
	string fld     ;

	static uint retPos(0) ;

	errblock err ;

	pdc t_pdc ;

	SEL      = false ;
	PFCMD    = ""    ;
	passthru = true  ;

	if ( currAppl->isRawOutput() ) { return ; }

	p_poolMGR->put( err, "ZVERB", "", SHARED ) ;
	if ( err.error() )
	{
		llog( "C", "poolMGR put for ZVERB failed" << endl ) ;
	}

	if ( c == 27 )
	{
		ZCOMMAND = "SWAP" ;
		ZPARM    = "LIST" ;
		passthru = false  ;
		return            ;
	}

	if ( c == KEY_ENTER )
	{
		if ( wmPending )
		{
			currAppl->clear_msg() ;
			currScrn->save_panel_stack() ;
			currAppl->rempop() ;
			currAppl->addpop( "", row-1, col-3 ) ;
			currAppl->movepop() ;
			currScrn->restore_panel_stack() ;
			wmPending = false ;
			passthru  = false ;
			ZCOMMAND  = "NOP" ;
			currAppl->currPanel->get_cursor( row, col ) ;
			currScrn->set_row_col( row, col ) ;
			currAppl->display_pd() ;
			currAppl->display_id() ;
			return ;
		}
		else if ( currAppl->currPanel->on_border_line( row, col ) )
		{
			issueMessage( "PSYS015" ) ;
			ZCOMMAND  = "NOP" ;
			wmPending = true  ;
			passthru  = false ;
			return ;
		}
		else if ( currAppl->currPanel->hide_msg_window( row, col ) )
		{
			ZCOMMAND  = "NOP" ;
			passthru  = false ;
			return ;
		}
		else if ( row == currAppl->currPanel->get_abline() )
		{
			if ( currAppl->currPanel->display_pd( col ) )
			{
				passthru = false ;
				currAppl->currPanel->get_cursor( row, col ) ;
				currScrn->set_row_col( row, col ) ;
				ZCOMMAND = "NOP" ;
				return ;
			}
		}
		else
		{
			t_pdc = currAppl->currPanel->retrieve_pdChoice( row, col ) ;
			if ( t_pdc.pdc_run != "" )
			{
				ZCOMMAND = t_pdc.pdc_run ;
				if ( ZCOMMAND == "ISRROUTE" )
				{
					CMDVerb = word( t_pdc.pdc_parm, 1 )    ;
					CMDParm = subword( t_pdc.pdc_parm, 2 ) ;
					if ( CMDVerb == "SELECT" )
					{
						if ( !SELCT.parse( err, CMDParm ) )
						{
							llog( "E", "Error in SELECT command "+ t_pdc.pdc_parm << endl ) ;
							currAppl->setmsg( "PSYS011K" ) ;
							return ;
						}
						if ( SELCT.PGM[ 0 ] == '&' )
						{
							currAppl->vcopy( substr( SELCT.PGM, 2 ), SELCT.PGM, MOVE ) ;
						}
						SEL      = true  ;
						passthru = false ;
						return           ;
					}
				}
				else
				{
					ZCOMMAND += " " + t_pdc.pdc_parm ;
				}
			}
		}
	}

	currAppl->currPanel->hide_pd() ;

	if ( wmPending )
	{
		wmPending = false ;
		currAppl->clear_msg() ;
	}

	addRetrieve = true  ;
	delm        = p_poolMGR->get( err, "ZDEL", PROFILE ) ;

	if ( t_pdc.pdc_run == "" )
	{
		ZCOMMAND = strip( currAppl->currPanel->cmd_getvalue() ) ;
	}

	if ( c == KEY_ENTER  &&
	     ZCOMMAND != ""  &&
	     p_poolMGR->get( err, "ZSWAPC", PROFILE ) == ZCOMMAND.substr( 0, 1 ) &&
	     p_poolMGR->get( err, "ZSWAP",  PROFILE ) == "Y" )
	{
		currAppl->currPanel->field_get_row_col( currAppl->currPanel->CMDfield, rw, cl ) ;
		if ( rw == row && cl < col )
		{
			ZPARM    = upper( ZCOMMAND.substr( 1, col-cl-1 ) ) ;
			ZCOMMAND = "SWAP" ;
			passthru = false  ;
			currAppl->currPanel->cmd_setvalue( "" ) ;
			return ;
		}
	}

	if ( commandStack != "" )
	{
		if ( ZCOMMAND != "" )
		{
			currAppl->currPanel->cmd_setvalue( ZCOMMAND + delm + commandStack ) ;
			commandStack = "" ;
			return ;
		}
		else
		{
			ZCOMMAND     = commandStack ;
			commandStack = ""    ;
			addRetrieve  = false ;
		}
	}

	if ( pfkeyPressed )
	{
		if ( p_poolMGR->get( err, "ZKLUSE", PROFILE ) == "Y" )
		{
			currAppl->reload_keylist( currAppl->currPanel ) ;
			PFCMD = currAppl->currPanel->get_keylist( c ) ;
		}
		if ( PFCMD == "" )
		{
			PFCMD = pfKeyValue( c ) ;
		}
		debug1( "PF Key pressed " << PFCMD << endl ) ;
	}
	else
	{
		p_poolMGR->put( err, "ZPFKEY", "PF00", SHARED, SYSTEM ) ;
		if ( err.error() ) { llog( "C", "VPUT for PF00 failed" << endl ) ; }
	}

	if ( addRetrieve )
	{
		rbsize = ds2d( p_poolMGR->get( err, "ZRBSIZE", PROFILE ) ) ;
		rtsize = ds2d( p_poolMGR->get( err, "ZRTSIZE", PROFILE ) ) ;
		if ( retrieveBuffer.capacity() != rbsize )
		{
			retrieveBuffer.rset_capacity( rbsize ) ;
		}
		if (  ZCOMMAND.size() >= rtsize &&
		     !findword( word( upper( ZCOMMAND ), 1 ), "RETRIEVE RETP" ) &&
		     !findword( word( upper( PFCMD ), 1 ), "RETRIEVE RETP" ) )
		{
			if ( retrieveBuffer.size() > 0 )
			{
				if ( ZCOMMAND != retrieveBuffer[ 0 ] )
				{
					retrieveBuffer.push_front( ZCOMMAND ) ;
				}
			}
			else
			{
				retrieveBuffer.push_front( ZCOMMAND ) ;
			}
			retPos = 0 ;
		}
	}

	switch( c )
	{
		case KEY_NPAGE:
			ZCOMMAND = "DOWN "+ ZCOMMAND ;
			break ;
		case KEY_PPAGE:
			ZCOMMAND = "UP "+ ZCOMMAND  ;
			break ;
	}

	if ( PFCMD != "" ) { ZCOMMAND = PFCMD + " " + ZCOMMAND ; }

	jumpOption = ZCOMMAND ;

	ZCOMMAND = strip( ZCOMMAND, 'L', delm[ 0 ] ) ;
	p1 = ZCOMMAND.find( delm[ 0 ] ) ;
	if ( p1 != string::npos )
	{
		commandStack = ZCOMMAND.substr( p1+1 ) ;
		ZCOMMAND.erase( p1 )                   ;
	}

	CMDVerb = upper( word( ZCOMMAND, 1 ) ) ;
	CMDParm = subword( ZCOMMAND, 2 ) ;

	if ( CMDVerb == "" ) { retPos = 0 ; return ; }

	if ( CMDVerb[ 0 ] == '@' )
	{
		SELCT.clear() ;
		currAppl->vcopy( "ZOREXPGM", SELCT.PGM, MOVE ) ;
		SELCT.PARM    = ZCOMMAND.substr( 1 ) ;
		SELCT.NEWAPPL = ""    ;
		SELCT.NEWPOOL = true  ;
		SELCT.PASSLIB = false ;
		SELCT.SUSPEND = true  ;
		SEL           = true  ;
		passthru      = false ;
		currAppl->currPanel->cmd_setvalue( "" ) ;
		return ;
	}
	else if ( CMDVerb[ 0 ] == '!' )
	{
		SELCT.clear() ;
		SELCT.PGM     = CMDVerb.substr( 1 ) ;
		SELCT.PARM    = CMDParm ;
		SELCT.NEWAPPL = ""      ;
		SELCT.NEWPOOL = true    ;
		SELCT.PASSLIB = false   ;
		SELCT.SUSPEND = true    ;
		SEL           = true    ;
		passthru      = false   ;
		currAppl->currPanel->cmd_setvalue( "" ) ;
		return ;
	}

	if ( ZCOMMAND.size() > 1 && ZCOMMAND.front() == '=' && ( PFCMD == "" || PFCMD == "RETURN" ) )
	{
		ZCOMMAND.erase( 0, 1 ) ;
		debug1( "JUMP entered.  Jumping to primary menu option " << ZCOMMAND << endl ) ;
		passthru = true ;
		if ( !currAppl->isprimMenu() )
		{
			currAppl->jumpEntered = true ;
			p_poolMGR->put( err, "ZVERB", "RETURN", SHARED ) ;
			currAppl->currPanel->cmd_setvalue( "" ) ;
			return ;
		}
		currAppl->currPanel->cmd_setvalue( ZCOMMAND ) ;
	}

	if ( CMDVerb == "RETRIEVE" )
	{
		if ( datatype( CMDParm, 'W' ) )
		{
			p1 = ds2d( CMDParm ) ;
			if ( p1 > 0 && p1 <= retrieveBuffer.size() ) { retPos = p1 - 1 ; }
		}
		commandStack = ""    ;
		ZCOMMAND     = "NOP" ;
		passthru     = false ;
		fld          = currAppl->currPanel->CMDfield ;
		if ( !retrieveBuffer.empty() )
		{
			currAppl->currPanel->cmd_setvalue( retrieveBuffer[ retPos ] ) ;
			currAppl->currPanel->cursor_to_field( RC, fld, retrieveBuffer[ retPos ].size()+1 ) ;
			if ( ++retPos >= retrieveBuffer.size() ) { retPos = 0 ; }
		}
		else
		{
			currAppl->currPanel->cmd_setvalue( "" ) ;
			currAppl->currPanel->cursor_to_field( RC, fld ) ;
		}
		currAppl->currPanel->get_cursor( row, col ) ;
		currScrn->set_row_col( row, col ) ;
		currAppl->currPanel->remove_pd()  ;
		return ;
	}
	retPos = 0 ;

	if ( CMDVerb == "HELP")
	{
		commandStack = "" ;
		if ( currAppl->currPanel->MSGID == "" || currAppl->currPanel->showLMSG )
		{
			currAppl->currPanel->cmd_setvalue( "" ) ;
			ZPARM   = currAppl->get_help_member( row, col ) ;
			CMDParm = ZPARM ;
		}
		else
		{
			ZCOMMAND = "NOP" ;
			passthru = false ;
			currAppl->currPanel->showLMSG = true ;
			currAppl->currPanel->display_msg() ;
			return ;
		}
	}

	AVerb = CMDVerb ;

	if ( resolveZCTEntry( CMDVerb, CMDParm ) )
	{
		if ( word( ZCTACT, 1 ) == "NOP" )
		{
			p_poolMGR->put( err, "ZCTMVAR", left( AVerb, 8 ), SHARED ) ;
			issueMessage( "PSYS011" ) ;
			currAppl->currPanel->cmd_setvalue( "" ) ;
			passthru = false ;
			return ;
		}
		else if ( word( ZCTACT, 1 ) == "SELECT" )
		{
			if ( !SELCT.parse( err, subword( ZCTACT, 2 ) ) )
			{
				llog( "E", "Error in SELECT command "+ ZCTACT << endl ) ;
				currAppl->setmsg( "PSYS011K" ) ;
				return ;
			}
			p1 = SELCT.PARM.find( "&ZPARM" ) ;
			if ( p1 != string::npos )
			{
				SELCT.PARM.replace( p1, 6, CMDParm ) ;
			}
			if ( SELCT.PGM[ 0 ] == '&' )
			{
				currAppl->vcopy( substr( SELCT.PGM, 2 ), SELCT.PGM, MOVE ) ;
			}
			SEL      = true  ;
			passthru = false ;
		}
		else if ( ZCTACT == "PASSTHRU" )
		{
			passthru = true ;
			ZCOMMAND = strip( CMDVerb + " " + CMDParm ) ;
		}
		else if ( ZCTACT == "SETVERB" )
		{
			if ( currAppl->currPanel->is_cmd_inactive( CMDVerb ) )
			{
				p_poolMGR->put( err, "ZCTMVAR", left( AVerb, 8 ), SHARED ) ;
				issueMessage( "PSYS011" ) ;
				currAppl->currPanel->cmd_setvalue( "" ) ;
				passthru = false ;
				ZCOMMAND = "NOP" ;
				return ;
			}
			else
			{
				passthru = true ;
				p_poolMGR->put( err, "ZVERB", ZCTVERB, SHARED ) ;
				if ( err.error() ) { llog( "C", "VPUT for ZVERB failed" << endl ) ; }
				ZCOMMAND = subword( ZCOMMAND, 2 ) ;
			}
			if ( CMDVerb == "NRETRIEV" )
			{
				SELCT.clear() ;
				currAppl->vcopy( "ZRFLPGM", SELCT.PGM, MOVE ) ;
				SELCT.PARM = "NR1 " + CMDParm ;
				SEL        = true  ;
				passthru   = false ;
			}
		}
	}
	if ( findword( CMDVerb, BuiltInCommands ) || findword( CMDVerb, SystemCommands ) )
	{
		passthru = false   ;
		ZCOMMAND = CMDVerb ;
		ZPARM    = upper( CMDParm ) ;
	}

	if ( CMDVerb[ 0 ] == '>' )
	{
		ZCOMMAND.erase( 0, 1 ) ;
	}

	if ( currAppl->currPanel->pd_Active() && ZCTVERB == "END" )
	{
		currAppl->currPanel->remove_pd() ;
		ZCOMMAND = ""    ;
		passthru = false ;
	}

	currAppl->currPanel->cmd_setvalue( passthru ? ZCOMMAND : "" ) ;
	debug1( "Primary command '"+ ZCOMMAND +"'  Passthru = " << passthru << endl ) ;
}


bool resolveZCTEntry( string& CMDVerb, string& CMDParm )
{
	int i ;
	int j ;
	int ws ;

	bool siteBefore ;
	bool found      ;

	string cmdtlst ;

	errblock err  ;

	ZCTVERB  = "" ;
	ZCTTRUNC = "" ;
	ZCTACT   = "" ;
	ZCTDESC  = "" ;

	cmdtlst  = "" ;
	found    = false ;

	siteBefore = ( p_poolMGR->get( err, "ZSCMDTF", PROFILE ) == "Y" ) ;

	if ( currAppl->ZZAPPLID != "ISP" ) { cmdtlst = currAppl->ZZAPPLID + " " ; }
	cmdtlst += p_poolMGR->get( err, "ZUCMDT1", PROFILE ) + " " +
		   p_poolMGR->get( err, "ZUCMDT2", PROFILE ) + " " +
		   p_poolMGR->get( err, "ZUCMDT3", PROFILE ) + " " ;
	if ( !siteBefore )
	{
		  cmdtlst += "ISP " ;
	}
	cmdtlst += p_poolMGR->get( err, "ZSCMDT1", PROFILE ) + " " +
		   p_poolMGR->get( err, "ZSCMDT2", PROFILE ) + " " +
		   p_poolMGR->get( err, "ZSCMDT3", PROFILE ) + " " ;
	if ( siteBefore )
	{
		  cmdtlst += "ISP" ;
	}

	ws = words( cmdtlst ) ;

	for ( i = 0 ; i < 8 ; i++ )
	{
		for ( j = 1 ; j <= ws ; j++ )
		{
			p_tableMGR->cmdsearch( err, funcPOOL, word( cmdtlst, j ), CMDVerb, ZTLIB ) ;
			if ( !err.RC0() || ZCTACT == "" ) { continue ; }
			if ( ZCTACT[ 0 ] == '&' )
			{
				currAppl->vcopy( substr( ZCTACT, 2 ), ZCTACT, MOVE ) ;
				if ( ZCTACT == "" ) { found = false ; continue ; }
			}
			found = true ;
			break ;
		}
		if ( err.getRC() > 0 || word( ZCTACT, 1 ) != "ALIAS" ) { break ; }
		CMDVerb  = word( ZCTACT, 2 )    ;
		CMDParm  = subword( ZCTACT, 3 ) ;
		ZCOMMAND = subword( ZCTACT, 2 ) ;
	}
	if ( i > 7 )
	{
		llog( "E", "ALIAS dept cannot be greater than 8.  Terminating search" << endl ) ;
		found = false ;
	}
	return found ;
}


void processPGMSelect()
{
	// Called when an application program has invoked the SELECT service (also VIEW, EDIT, BROWSE)

	SELCT = currAppl->get_select_cmd() ;

	if ( apps.find( SELCT.PGM ) != apps.end() )
	{
		updateDefaultVars() ;
		startApplication( SELCT ) ;
	}
	else
	{
		if ( !currAppl->errorsReturn() )
		{
			errorScreen( 1, "SELECT function did not find application '"+ SELCT.PGM +"'" ) ;
		}
		currAppl->SEL = false ;
		currAppl->RC  = 20    ;
		llog( "W", "Resumed function did a SELECT.  Ending wait in SELECT" << endl ) ;
		ResumeApplicationAndWait() ;
		while ( currAppl->terminateAppl )
		{
			debug1( "Calling application "+ currAppl->ZAPPNAME +" also ending" << endl ) ;
			terminateApplication() ;
			if ( pLScreen::screensTotal == 0 ) { return ; }
		}
	}
}


void startApplication( selobj SEL, bool nScreen )
{
	// Start an application using the passed SELECT object, on a new logical screen if specified.

	// If the program is ISPSTRT, start the application in the PARM field on a new logical screen
	// or start GMAINPGM.  Force NEWPOOL option regardless of what is coded in the command.
	// PARM can be a command table entry, a PGM()/CMD()/PANEL() statement or an option for GMAINPGM.

	int elapsed ;
	int p1      ;

	uint row    ;
	uint col    ;

	string opt  ;
	string rest ;

	bool setMSG ;

	errblock err ;

	pApplication * oldAppl = currAppl ;

	boost::thread * pThread ;

	if ( SEL.PGM == "ISPSTRT" )
	{
		nScreen = true ;
		opt     = upper( SEL.PARM ) ;
		rest    = ""                ;
		if ( isvalidName( opt ) && resolveZCTEntry( opt, rest ) && word( ZCTACT, 1 ) == "SELECT" )
		{
			if ( !SEL.parse( err, subword( ZCTACT, 2 ) ) )
			{
				errorScreen( 1, "Error in ZCTACT command "+ ZCTACT ) ;
				return ;
			}
			p1 = SEL.PARM.find( "&ZPARM" ) ;
			if ( p1 != string::npos )
			{
				SEL.PARM.replace( p1, 6, rest ) ;
			}
		}
		else if ( !SEL.parse( err, SEL.PARM ) )
		{
			SEL.clear() ;
			SEL.PGM     = GMAINPGM ;
			SEL.PARM    = opt   ;
			SEL.NEWAPPL = "ISP" ;
			SEL.PASSLIB = false ;
			SEL.SUSPEND = true  ;
		}
		if ( SEL.PGM[ 0 ] == '&' )
		{
			currAppl->vcopy( substr( SEL.PGM, 2 ), SEL.PGM, MOVE ) ;
		}
		SEL.NEWPOOL = true ;
	}

	if ( apps.find( SEL.PGM ) == apps.end() )
	{
		errorScreen( 1, "Application '"+ SEL.PGM +"' not found" ) ;
		return ;
	}

	if ( !apps[ SEL.PGM ].dlopened )
	{
		if ( !loadDynamicClass( SEL.PGM ) )
		{
			errorScreen( 1, "Errors loading application "+ SEL.PGM ) ;
			return ;
		}
	}

	if ( nScreen && !createLogicalScreen() ) { return ; }

	currAppl->store_scrname() ;
	setMSG = currAppl->setMSG ;
	if ( setMSG )
	{
		currAppl->setMSG = false        ;
	}

	llog( "I", "Starting new application "+ SEL.PGM +" with parameters '"+ SEL.PARM +"'" << endl ) ;
	currAppl = ((pApplication*(*)())( apps[ SEL.PGM ].maker_ep))() ;

	currScrn->application_add( currAppl ) ;
	currAppl->startSelect( SEL )          ;
	currAppl->taskid( ++maxtaskID )       ;
	currAppl->p_poolMGR    = p_poolMGR    ;
	currAppl->p_tableMGR   = p_tableMGR   ;
	currAppl->lspfCallback = lspfCallbackHandler ;
	apps[ SEL.PGM ].refCount++ ;

	if ( SEL.NEWAPPL != "" && SEL.NEWAPPL != p_poolMGR->getApplid() )
	{
		p_poolMGR->setApplid( err, SEL.NEWAPPL ) ;
	}
	currAppl->ZZAPPLID = p_poolMGR->getApplid() ;

	p_poolMGR->createPool( err, PROFILE, ZSPROF ) ;
	if ( err.RC4() ) { createpfKeyDefaults() ; }

	if ( SEL.NEWPOOL )
	{
		if ( currScrn->application_stack_size() > 1 && SELCT.SCRNAME == "" )
		{
			SELCT.SCRNAME = p_poolMGR->get( err, "ZSCRNAME", SHARED ) ;
		}
		currAppl->NEWPOOL = true ;
		p_poolMGR->createPool( err, SHARED ) ;
		p_poolMGR->put( err, "ZSCREEN", string( 1, ZSCREEN[ screenNum ] ), SHARED, SYSTEM ) ;
		p_poolMGR->put( err, "ZSCRNUM", d2ds( currScrn->screenId ), SHARED, SYSTEM ) ;
		p_poolMGR->put( err, "ZAPPLID", p_poolMGR->getApplid(), SHARED, SYSTEM ) ;
	}
	currAppl->shrdPool = p_poolMGR->getShrdPool() ;
	currAppl->init() ;

	if ( !SEL.SUSPEND )
	{
		currAppl->set_addpop_row( oldAppl->get_addpop_row() ) ;
		currAppl->set_addpop_col( oldAppl->get_addpop_col() ) ;
		currAppl->set_addpop_act( oldAppl->get_addpop_act() ) ;
	}

	if ( SEL.PASSLIB || SEL.NEWAPPL == "" )
	{
		debug1( "PASSLIB or no NEWAPPL specifed on start application.  Restoring LIBDEF status to new application." << endl ) ;
  //  todo      currAppl->ZMUSER = oldAppl->ZMUSER ;
  //            currAppl->ZPUSER = oldAppl->ZPUSER ;
  //            currAppl->ZTUSER = oldAppl->ZTUSER ;
		currAppl->libdef_muser = oldAppl->libdef_muser ;
		currAppl->libdef_puser = oldAppl->libdef_puser ;
		currAppl->libdef_tuser = oldAppl->libdef_tuser ;
	}

	if ( setMSG ) { currAppl->set_msg1( oldAppl->getmsg1(), oldAppl->getmsgid1() ) ; }

	pThread = new boost::thread(boost::bind(&pApplication::application, currAppl ));

	currAppl->pThread = pThread ;

	if ( SELCT.SCRNAME != "" )
	{
		p_poolMGR->put( err, "ZSCRNAME", SELCT.SCRNAME, SHARED ) ;
	}

	llog( "I", "Waiting for new application to complete startup.  ID=" << pThread->get_id() << endl ) ;
	elapsed = 0 ;
	while ( currAppl->busyAppl )
	{
		elapsed++ ;
		boost::this_thread::sleep_for(boost::chrono::milliseconds(ZWAIT)) ;
		if ( currAppl->noTimeOut ) { elapsed = 0 ; }
		if ( elapsed > GMAXWAIT  ) { currAppl->set_timeout_abend() ; }
	}

	llog( "I", "New thread and application started and initialised. ID=" << pThread->get_id() << endl ) ;

	if ( currAppl->rmsgs.size() > 0 ) { rawOutput() ; }

	if ( currAppl->SEL )
	{
		debug1( "Application "+ currAppl->ZAPPNAME +" has done a SELECT without a DISPLAY !!!! " << endl ) ;
		processPGMSelect() ;
	}

	if ( currAppl->abnormalEnd )
	{
		errorScreen( 2, "An error has occured initialising new task for "+ SEL.PGM ) ;
		terminateApplication() ;
		if ( pLScreen::screensTotal == 0 ) { return ; }
	}

	while ( currAppl->terminateAppl )
	{
		llog( "I", "Application "+ currAppl->ZAPPNAME +" has immediately terminated.  Cleaning up resources" << endl ) ;
		terminateApplication() ;
		if ( pLScreen::screensTotal == 0 ) { return ; }
		if ( currAppl->SEL && !currAppl->terminateAppl )
		{
			debug1( "Application "+ currAppl->ZAPPNAME +" has done another SELECT without a DISPLAY (2) !!!! " << endl ) ;
			processPGMSelect() ;
		}
	}
	currAppl->get_cursor( row, col )  ;
	currScrn->set_row_col( row, col ) ;
}


void terminateApplication()
{
	int  tRC  ;
	int  tRSN ;

	uint row  ;
	uint col  ;

	string ZAPPNAME ;
	string tRESULT  ;
	string tMSGID1  ;
	string fname    ;
	string delm     ;

	bool refList      ;
	bool nretError    ;
	bool propagateEnd ;
	bool jumpEntered  ;
	bool setCursor    ;
	bool setMSG       ;

	slmsg tMSG1       ;

	errblock err ;

	boost::thread * pThread ;

	llog( "I", "Application terminating "+ currAppl->ZAPPNAME +" ID: "<< currAppl->taskid() << endl ) ;

	ZAPPNAME = currAppl->ZAPPNAME ;

	currAppl->closeTables()     ;
	tRC     = currAppl->ZRC     ;
	tRSN    = currAppl->ZRSN    ;
	tRESULT = currAppl->ZRESULT ;

	refList = ( currAppl->reffield == "#REFLIST" ) ;

	setMSG = currAppl->setMSG ;
	if ( setMSG ) { tMSGID1 = currAppl->getmsgid1() ; tMSG1 = currAppl->getmsg1() ; }

	jumpEntered  = currAppl->jumpEntered ;
	propagateEnd = currAppl->propagateEnd && ( currScrn->application_stack_size() > 1 ) ;

	pThread = currAppl->pThread ;

	if ( currAppl->abnormalEnd )
	{
		while ( currAppl->cleanupRunning() )
		{
			boost::this_thread::sleep_for(boost::chrono::milliseconds(ZWAIT)) ;
		}
		pThread->detach() ;
	}

	p_poolMGR->destroyPool( err, PROFILE ) ;
	if ( currAppl->NEWPOOL )
	{
		p_poolMGR->destroyPool( err, SHARED ) ;
	}

	llog( "I", "Removing application instance of "+ ZAPPNAME << endl ) ;
	apps[ ZAPPNAME ].refCount-- ;
	((void (*)(pApplication*))(apps[ ZAPPNAME ].destroyer_ep))( currAppl ) ;

	delete pThread  ;

	currScrn->application_remove_current() ;
	if ( currScrn->application_stack_empty() )
	{
		p_poolMGR->destroyPool( currScrn->screenId ) ;
		if ( pLScreen::screensTotal == 1 )
		{
			delete currScrn ;
			llog( "I", "Closing ISPS profile and application log as last application program is terminating" << endl ) ;
			p_poolMGR->setApplid( err, "ISPS" )    ;
			p_poolMGR->destroyPool( err, PROFILE ) ;
			p_poolMGR->statistics()  ;
			p_tableMGR->statistics() ;
			currAppl->closeLog()     ;
			return ;
		}
		deleteLogicalScreen() ;
	}

	currAppl = currScrn->application_get_current() ;
	currScrn->restore_panel_stack() ;
	currAppl->display_pd()          ;

	p_poolMGR->setApplid( err, currAppl->ZZAPPLID ) ;
	if ( !err.RC0() ) { llog( "C", "ERROR setting APPLID for pool manager.  RC=" << err.getRC() << endl ) ; }
	p_poolMGR->setShrdPool( err, currAppl->shrdPool )   ;
	if ( !err.RC0() ) { llog( "C", "ERROR setting shared pool for pool manager.  RC=" << err.getRC() << endl ) ; }

	p_poolMGR->put( err, "ZPANELID", currAppl->PANELID, SHARED, SYSTEM )  ;

	if ( apps[ ZAPPNAME ].refCount == 0 && apps[ ZAPPNAME ].relPending )
	{
		apps[ ZAPPNAME ].relPending = false ;
		llog( "I", "Reloading module "+ ZAPPNAME +" (pending reload status)" << endl ) ;
		if ( loadDynamicClass( ZAPPNAME ) )
		{
			llog( "I", "Loaded "+ ZAPPNAME +" (module "+ apps[ZAPPNAME].module +") from "+ apps[ZAPPNAME].file << endl ) ;
		}
		else
		{
			llog( "W", "Errors occured loading "+ ZAPPNAME +"  Module removed" << endl ) ;
		}
	}

	nretError = false ;
	if ( refList )
	{
		if ( currAppl->nretriev_on() )
		{
			fname = currAppl->get_nretfield() ;
			if ( fname != "" )
			{
				if ( currAppl->currPanel->field_valid( fname ) )
				{
					currAppl->reffield = fname ;
					if ( p_poolMGR->get( err, "ZRFMOD", PROFILE ) == "BEX" )
					{
						delm = p_poolMGR->get( err, "ZDEL", PROFILE ) ;
						commandStack = delm + delm ;
					}
				}
				else
				{
					llog( "E", "Invalid field "+ fname +" in .NRET panel statement" << endl ) ;
					issueMessage( "PSYS011Z" ) ;
					nretError = true ;
				}
			}
			else
			{
				issueMessage( "PSYS011Y" ) ;
				nretError = true ;
			}
		}
		else
		{
			issueMessage( "PSYS011X" ) ;
			nretError = true ;
		}
	}

	setCursor = true ;
	if ( currAppl->reffield != "" && !nretError )
	{
		if ( tRC == 0 )
		{
			if ( currAppl->currPanel->field_get_row_col( currAppl->reffield, row, col ) )
			{
				currAppl->currPanel->field_setvalue( currAppl->reffield, tRESULT ) ;
				currAppl->currPanel->cursor_eof( row, col ) ;
				currAppl->currPanel->set_cursor( row, col ) ;
				if ( refList )
				{
					issueMessage( "PSYS011W" ) ;
				}
				setCursor = false ;
			}
			else
			{
				llog( "E", "Invalid field "+ currAppl->reffield +" in .NRET panel statement" << endl )   ;
				issueMessage( "PSYS011Z" ) ;
			}
		}
		else if ( tRC == 8 )
		{
			beep() ;
			setCursor = false ;
		}
		currAppl->reffield = "" ;
	}

	if ( currAppl->isprimMenu() )
	{
		propagateEnd = false ;
		if ( jumpEntered ) { commandStack = jumpOption ; }
	}

	if ( currAppl->SEL )
	{
		if ( propagateEnd )
		{
			llog( "I", "RETURN entered.  Propagating END to next application in the SELECT nested dialogue" << endl ) ;
			if ( jumpEntered )
			{
				debug1( "JUMP entered but in the same nested dialogue." << endl ) ;
				currAppl->jumpEntered  = true ;
			}
			currAppl->RC           = 4     ;
			currAppl->propagateEnd = true  ;
		}
		else
		{
			currAppl->RC = 0 ;
		}
		currAppl->SEL     = false   ;
		currAppl->ZRC     = tRC     ;
		currAppl->ZRSN    = tRSN    ;
		currAppl->ZRESULT = tRESULT ;
		if ( setMSG ) { currAppl->set_msg1( tMSG1, tMSGID1 ) ; }
		llog( "I", "Resumed function did a SELECT, BROWSE, EDIT or VIEW.  Ending wait in function" << endl ) ;
		ResumeApplicationAndWait() ;
		while ( currAppl->terminateAppl )
		{
			debug1( "Calling application "+ currAppl->ZAPPNAME +" also ending" << endl ) ;
			terminateApplication() ;
			if ( pLScreen::screensTotal == 0 ) { return ; }
		}
		currAppl->get_cursor( row, col ) ;
	}
	else
	{
		if ( jumpEntered && propagateEnd )
		{
			debug1( "JUMP entered.  Propagating RETURN to next application on the stack outside a SELECT nested dialogue" << endl ) ;
			p_poolMGR->put( err, "ZVERB", "RETURN", SHARED ) ;
			currAppl->jumpEntered = true ;
			ResumeApplicationAndWait()   ;
		}
		while ( currAppl->terminateAppl )
		{
			debug1( "Previous application "+ currAppl->ZAPPNAME +" also ending" << endl ) ;
			terminateApplication() ;
			if ( pLScreen::screensTotal == 0 ) { return ; }
		}
		if ( setMSG ) { currAppl->set_msg1( tMSG1, tMSGID1, true ) ; }
		if ( setCursor )
		{
			currAppl->get_home( row, col )   ;
			currAppl->set_cursor( row, col ) ;
		}
	}

	llog( "I", "Application terminatation of "+ ZAPPNAME +" completed.  Current application is "+ currAppl->ZAPPNAME << endl ) ;
	currAppl->restore_Zvars( currScrn->screenId ) ;
	currAppl->display_id() ;
	currScrn->set_row_col( row, col ) ;
}


bool createLogicalScreen()
{
	errblock err ;

	if ( !currAppl->ControlSplitEnable )
	{
		p_poolMGR->put( err, "ZCTMVAR", left( ZCOMMAND, 8 ), SHARED ) ;
		issueMessage( "PSYS011" ) ;
		return false ;
	}
	if ( pLScreen::screensTotal == ZMAXSCRN )
	{
		issueMessage( "PSYS011D" ) ;
		return false ;
	}
	currScrn->save_panel_stack()  ;
	updateDefaultVars()           ;
	altScreen = screenNum         ;
	screenNum = screenList.size() ;
	screenList.push_back( new pLScreen ) ;
	return true ;
}


void deleteLogicalScreen()
{
	delete currScrn ;

	screenList.erase( screenList.begin() + screenNum ) ;
	if ( altScreen > screenNum ) { --altScreen ; }
	--screenNum ;
	if ( screenNum < 0 ) { screenNum = pLScreen::screensTotal - 1 ; }
	if ( pLScreen::screensTotal > 1 )
	{
		if ( altScreen == screenNum )
		{
			altScreen = ((altScreen == pLScreen::screensTotal - 1) ? 0 : (altScreen + 1) )  ;
		}
	}
	currScrn = screenList[ screenNum ] ;
}


void ResumeApplicationAndWait()
{
	int elapsed ;

	elapsed = 0               ;
	currAppl->busyAppl = true ;
	cond_appl.notify_all()    ;
	while ( currAppl->busyAppl )
	{
		elapsed++ ;
		boost::this_thread::sleep_for(boost::chrono::milliseconds(ZWAIT)) ;
		if ( currAppl->noTimeOut ) { elapsed = 0 ; }
		if ( elapsed > GMAXWAIT  ) { currAppl->set_timeout_abend() ; }
	}
	if ( currAppl->rmsgs.size() > 0 ) { rawOutput() ; }

	if ( currAppl->abnormalEnd )
	{
		if ( currAppl->abnormalTimeout )
		{
			errorScreen( 2, "An application timeout has occured.  Increase ZMAXWAIT if necessary" ) ;
		}
		else if ( currAppl->abnormalEndForced )
		{
			errorScreen( 2, "A forced termination of the subtask has occured" ) ;
		}
		else
		{
			errorScreen( 2, "An error has occured during application execution" ) ;
		}
	}
	else if ( currAppl->SEL )
	{
		processPGMSelect() ;
	}
}


void loadCUATables()
{

	cuaAttrName[ "AB"     ] = AB     ;         // AB Selected Choice (was Yellow/normal)
	cuaAttrName[ "ABSL"   ] = ABSL   ;         // AB Separator Line  (was blue/normal)
	cuaAttrName[ "ABU"    ] = ABU    ;         // AB Unselected Choice (was Yellow/normal)

	cuaAttrName[ "AMT"    ] = AMT    ;         // Action Message Text
	cuaAttrName[ "AWF"    ] = AWF    ;         // Active Window Frame

	cuaAttrName[ "CT"     ] = CT     ;         // Caution Text
	cuaAttrName[ "CEF"    ] = CEF    ;         // Choice Entry Field
	cuaAttrName[ "CH"     ] = CH     ;         // Column Heading

	cuaAttrName[ "DT"     ] = DT     ;         // Descriptive Text
	cuaAttrName[ "ET"     ] = ET     ;         // Emphasized Text
	cuaAttrName[ "EE"     ] = EE     ;         // Error Emphasis
	cuaAttrName[ "FP"     ] = FP     ;         // Field Prompt
	cuaAttrName[ "FK"     ] = FK     ;         // Function Keys

	cuaAttrName[ "IMT"    ] = IMT    ;         // Informational Message Text
	cuaAttrName[ "LEF"    ] = LEF    ;         // List Entry Field
	cuaAttrName[ "LID"    ] = LID    ;         // List Item Description
	cuaAttrName[ "LI"     ] = LI     ;         // List Items

	cuaAttrName[ "NEF"    ] = NEF    ;         // Normal Entry Field
	cuaAttrName[ "NT"     ] = NT     ;         // Normal Text

	cuaAttrName[ "PI"     ] = PI     ;         // Panel ID
	cuaAttrName[ "PIN"    ] = PIN    ;         // Panel Instruction
	cuaAttrName[ "PT"     ] = PT     ;         // Panel Title

	cuaAttrName[ "PS"     ] = PS     ;         // Point-and-Shoot
	cuaAttrName[ "PAC"    ] = PAC    ;         // PD Available Choices
	cuaAttrName[ "PUC"    ] = PUC    ;         // PD Unavailable Choices

	cuaAttrName[ "RP"     ] = RP     ;         // Reference Phrase

	cuaAttrName[ "SI"     ] = SI     ;         // Scroll Information
	cuaAttrName[ "SAC"    ] = SAC    ;         // Sel. Available Choices
	cuaAttrName[ "SUC"    ] = SUC    ;         // Sel. Unavailable Choices

	cuaAttrName[ "VOI"    ] = VOI    ;         // Variable Output Info
	cuaAttrName[ "WMT"    ] = WMT    ;         // Warning Message Text
	cuaAttrName[ "WT"     ] = WT     ;         // Warning Text
	cuaAttrName[ "WASL"   ] = WASL   ;         // Work Area Separator Line

	cuaAttrName[ "CHAR"   ] = CHAR   ;         //
	cuaAttrName[ "DATAIN" ] = DATAIN ;         //
	cuaAttrName[ "DATAOUT"] = DATAOUT;         //
	cuaAttrName[ "GRPBOX" ] = GRPBOX ;         // Group Box
	cuaAttrName[ "OUTPUT" ] = OUTPUT ;         //
	cuaAttrName[ "TEXT"   ] = TEXT   ;         //

	setColourPair( "AB" )   ;                  //  Set according to the variables in ISPS profile
	setColourPair( "ABSL" ) ;
	setColourPair( "ABU" )  ;

	setColourPair( "AMT" )  ;
	setColourPair( "AWF" )  ;

	setColourPair( "CT"  )  ;
	setColourPair( "CEF" )  ;
	setColourPair( "CH"  )  ;

	setColourPair( "DT" )   ;
	setColourPair( "ET" )   ;
	setColourPair( "EE" )   ;
	setColourPair( "FP" )   ;
	setColourPair( "FK")    ;

	setColourPair( "IMT" )  ;
	setColourPair( "LEF" )  ;
	setColourPair( "LID" )  ;
	setColourPair( "LI" )   ;

	setColourPair( "NEF" )  ;
	setColourPair( "NT" )   ;

	setColourPair( "PI" )   ;
	setColourPair( "PIN" )  ;
	setColourPair( "PT" )   ;

	setColourPair( "PS")    ;
	setColourPair( "PAC" )  ;
	setColourPair( "PUC" )  ;

	setColourPair( "RP" )   ;

	setColourPair( "SI" )   ;
	setColourPair( "SAC" )  ;
	setColourPair( "SUC")   ;

	setColourPair( "VOI" )  ;
	setColourPair( "WMT" )  ;
	setColourPair( "WT" )   ;
	setColourPair( "WASL" ) ;

	cuaAttr[ CHAR   ] = BLUE   | A_BOLD   | A_PROTECT ;
	cuaAttr[ DATAIN ] = GREEN  | A_NORMAL             ;
	cuaAttr[ DATAOUT] = GREEN  | A_NORMAL | A_PROTECT ;
	cuaAttr[ GRPBOX ] = GREEN  | A_NORMAL | A_PROTECT ;
	cuaAttr[ OUTPUT ] = GREEN  | A_NORMAL | A_PROTECT ;
	cuaAttr[ TEXT   ] = BLUE   | A_NORMAL | A_PROTECT ;

	cuaAttrProt[ AB     ] = true  ;
	cuaAttrProt[ ABSL   ] = true  ;
	cuaAttrProt[ ABU    ] = true  ;

	cuaAttrProt[ AMT    ] = true  ;
	cuaAttrProt[ AWF    ] = true  ;

	cuaAttrProt[ CT     ] = true  ;
	cuaAttrProt[ CEF    ] = false ;
	cuaAttrProt[ CH     ] = true  ;

	cuaAttrProt[ DT     ] = true  ;
	cuaAttrProt[ ET     ] = true  ;
	cuaAttrProt[ EE     ] = true  ;
	cuaAttrProt[ FP     ] = true  ;
	cuaAttrProt[ FK     ] = true  ;

	cuaAttrProt[ IMT    ] = true  ;
	cuaAttrProt[ LEF    ] = false ;
	cuaAttrProt[ LID    ] = true  ;
	cuaAttrProt[ LI     ] = true  ;

	cuaAttrProt[ NEF    ] = false ;
	cuaAttrProt[ NT     ] = true  ;

	cuaAttrProt[ PI     ] = true  ;
	cuaAttrProt[ PIN    ] = true  ;
	cuaAttrProt[ PT     ] = true  ;

	cuaAttrProt[ PS     ] = true  ;
	cuaAttrProt[ PAC    ] = true  ;
	cuaAttrProt[ PUC    ] = true  ;

	cuaAttrProt[ RP     ] = true  ;

	cuaAttrProt[ SI     ] = true  ;
	cuaAttrProt[ SAC    ] = true  ;
	cuaAttrProt[ SUC    ] = true  ;

	cuaAttrProt[ VOI    ] = true  ;
	cuaAttrProt[ WMT    ] = true  ;
	cuaAttrProt[ WT     ] = true  ;
	cuaAttrProt[ WASL   ] = true  ;

	cuaAttrProt[ CHAR   ] = true  ;
	cuaAttrProt[ DATAIN ] = false ;
	cuaAttrProt[ DATAOUT] = true  ;
	cuaAttrProt[ GRPBOX ] = true  ;
	cuaAttrProt[ OUTPUT ] = true  ;
	cuaAttrProt[ TEXT   ] = true  ;

	usrAttr[ N_RED     ] = RED     ; usrAttr[ B_RED     ] = RED     | A_BOLD ; usrAttr[ R_RED     ] = RED     | A_REVERSE ; usrAttr[ U_RED     ] = RED     | A_UNDERLINE ;
	usrAttr[ N_GREEN   ] = GREEN   ; usrAttr[ B_GREEN   ] = GREEN   | A_BOLD ; usrAttr[ R_GREEN   ] = GREEN   | A_REVERSE ; usrAttr[ U_GREEN   ] = GREEN   | A_UNDERLINE ;
	usrAttr[ N_YELLOW  ] = YELLOW  ; usrAttr[ B_YELLOW  ] = YELLOW  | A_BOLD ; usrAttr[ R_YELLOW  ] = YELLOW  | A_REVERSE ; usrAttr[ U_YELLOW  ] = YELLOW  | A_UNDERLINE ;
	usrAttr[ N_BLUE    ] = BLUE    ; usrAttr[ B_BLUE    ] = BLUE    | A_BOLD ; usrAttr[ R_BLUE    ] = BLUE    | A_REVERSE ; usrAttr[ U_BLUE    ] = BLUE    | A_UNDERLINE ;
	usrAttr[ N_MAGENTA ] = MAGENTA ; usrAttr[ B_MAGENTA ] = MAGENTA | A_BOLD ; usrAttr[ R_MAGENTA ] = MAGENTA | A_REVERSE ; usrAttr[ U_MAGENTA ] = MAGENTA | A_UNDERLINE ;
	usrAttr[ N_TURQ    ] = TURQ    ; usrAttr[ B_TURQ    ] = TURQ    | A_BOLD ; usrAttr[ R_TURQ    ] = TURQ    | A_REVERSE ; usrAttr[ U_TURQ    ] = TURQ    | A_UNDERLINE ;
	usrAttr[ N_WHITE   ] = WHITE   ; usrAttr[ B_WHITE   ] = WHITE   | A_BOLD ; usrAttr[ R_WHITE   ] = WHITE   | A_REVERSE ; usrAttr[ U_WHITE   ] = WHITE   | A_UNDERLINE ;

	usrAttr[ P_RED     ] = RED     | A_PROTECT ;
	usrAttr[ P_GREEN   ] = GREEN   | A_PROTECT ;
	usrAttr[ P_YELLOW  ] = YELLOW  | A_PROTECT ;
	usrAttr[ P_BLUE    ] = BLUE    | A_PROTECT ;
	usrAttr[ P_MAGENTA ] = MAGENTA | A_PROTECT ;
	usrAttr[ P_TURQ    ] = TURQ    | A_PROTECT ;
	usrAttr[ P_WHITE   ] = WHITE   | A_PROTECT ;

	usrAttr[ P_FF      ] = GREEN   | A_PROTECT ;
}


void setColourPair( const string& name )
{
	string t ;

	errblock err ;

	t = p_poolMGR->get( err, "ZC"+ name, PROFILE ) ;
	if ( !err.RC0() )
	{
		llog( "E", "Variable ZC"+ name +" not found in ISPS profile" << endl ) ;
		llog( "C", "Rerun setup program to re-initialise ISPS profile" << endl ) ;
		abortStartup() ;
	}

	if ( t.size() != 3 )
	{
		llog( "E", "Variable ZC"+ name +" invalid value of "+ t +".  Must be length of three "<< endl ) ;
		llog( "C", "Rerun setup program to re-initialise ISPS profile" << endl ) ;
		abortStartup() ;
	}

	switch  ( t[ 0 ] )
	{
		case 'R':  cuaAttr[ cuaAttrName[ name ] ] = RED     ; break ;
		case 'G':  cuaAttr[ cuaAttrName[ name ] ] = GREEN   ; break ;
		case 'Y':  cuaAttr[ cuaAttrName[ name ] ] = YELLOW  ; break ;
		case 'B':  cuaAttr[ cuaAttrName[ name ] ] = BLUE    ; break ;
		case 'M':  cuaAttr[ cuaAttrName[ name ] ] = MAGENTA ; break ;
		case 'T':  cuaAttr[ cuaAttrName[ name ] ] = TURQ    ; break ;
		case 'W':  cuaAttr[ cuaAttrName[ name ] ] = WHITE   ; break ;
		default :  llog( "E", "Variable ZC"+ name +" has invalid value[0] "+ t << endl ) ;
			   llog( "C", "Rerun setup program to re-initialise ISPS profile" << endl ) ;
			   abortStartup() ;
	}

	switch  ( t[ 1 ] )
	{
		case 'L':  cuaAttr[ cuaAttrName[ name ] ] = cuaAttr[ cuaAttrName[ name ] ] | A_NORMAL   ; break ;
		case 'H':  cuaAttr[ cuaAttrName[ name ] ] = cuaAttr[ cuaAttrName[ name ] ] | A_BOLD     ; break ;
		default :  llog( "E", "Variable ZC"+ name +" has invalid value[1] "+ t << endl ) ;
			   llog( "C", "Rerun setup program to re-initialise ISPS profile" << endl ) ;
			   abortStartup() ;
	}

	switch  ( t[ 2 ] )
	{
		case 'N':  break ;
		case 'B':  cuaAttr[ cuaAttrName[ name ] ] = cuaAttr[ cuaAttrName[ name ] ] | A_BLINK     ; break ;
		case 'R':  cuaAttr[ cuaAttrName[ name ] ] = cuaAttr[ cuaAttrName[ name ] ] | A_REVERSE   ; break ;
		case 'U':  cuaAttr[ cuaAttrName[ name ] ] = cuaAttr[ cuaAttrName[ name ] ] | A_UNDERLINE ; break ;
		default :  llog( "E", "Variable ZC"+ name +" has invalid value[2] "+ t << endl ) ;
			   llog( "C", "Rerun setup program to re-initialise ISPS profile" << endl ) ;
			   abortStartup() ;
	}
}


void loadDefaultPools()
{
	// Default vars go in @DEFPROF (RO) for PROFILE and @DEFSHAR (UP) for SHARE
	// These have the SYSTEM attibute set on the variable

	errblock err ;

	struct utsname buf ;

	uname( &buf ) ;

	p_poolMGR->defaultVARs( err, "ZSCREEND", d2ds( pLScreen::maxrow ), SHARED ) ;
	p_poolMGR->defaultVARs( err, "ZSCRMAXD", d2ds( pLScreen::maxrow ), SHARED ) ;
	p_poolMGR->defaultVARs( err, "ZSCREENW", d2ds( pLScreen::maxcol ), SHARED ) ;
	p_poolMGR->defaultVARs( err, "ZSCRMAXW", d2ds( pLScreen::maxcol ), SHARED ) ;
	p_poolMGR->defaultVARs( err, "ZUSER", getenv( "LOGNAME" ), SHARED )         ;
	p_poolMGR->defaultVARs( err, "ZHOME", getenv( "HOME" )  , SHARED ) ;
	p_poolMGR->defaultVARs( err, "ZSHELL", getenv( "SHELL" ), SHARED ) ;

	p_poolMGR->setApplid( err, "ISPS" ) ;
	p_poolMGR->createPool( err, PROFILE, ZSPROF ) ;
	if ( !err.RC0() )
	{
		llog( "C", "Loading of system profile ISPSPROF failed.  RC="<< err.getRC() << endl ) ;
		llog( "C", "Aborting startup.  Check path " ZSPROF << endl ) ;
		abortStartup() ;
	}

	llog( "I", "Loaded system profile ISPSPROF" << endl ) ;
	p_poolMGR->createPool( err, SHARED )   ;

	ZTLIB = p_poolMGR->get( err, "ZTLIB", PROFILE ) ;

	p_poolMGR->setApplid( err, "ISP" ) ;
	p_poolMGR->createPool( err, PROFILE, ZSPROF ) ;
	if ( err.RC4() ) { createpfKeyDefaults() ; }

	p_poolMGR->defaultVARs( err, "Z", "", SHARED )                  ;
	p_poolMGR->defaultVARs( err, "ZSCRNAM1", "OFF", SHARED )        ;
	p_poolMGR->defaultVARs( err, "ZSYSNAME", buf.sysname, SHARED )  ;
	p_poolMGR->defaultVARs( err, "ZNODNAME", buf.nodename, SHARED ) ;
	p_poolMGR->defaultVARs( err, "ZOSREL", buf.release, SHARED )    ;
	p_poolMGR->defaultVARs( err, "ZOSVER", buf.version, SHARED )    ;
	p_poolMGR->defaultVARs( err, "ZMACHINE", buf.machine, SHARED )  ;
	p_poolMGR->defaultVARs( err, "ZENVIR", "lspf V0R0M1", SHARED )  ;
	p_poolMGR->defaultVARs( err, "ZDATEF",  "DD/MM/YY", SHARED )    ;
	p_poolMGR->defaultVARs( err, "ZDATEFD", "DD/MM/YY", SHARED )    ;
	p_poolMGR->defaultVARs( err, "ZSCALE", "OFF", SHARED )          ;
	p_poolMGR->defaultVARs( err, "ZSPLIT", "NO", SHARED )           ;
	p_poolMGR->defaultVARs( err, "ZNULLS", "NO", SHARED )           ;

	p_poolMGR->setPOOLsReadOnly() ;
	GMAINPGM = p_poolMGR->get( err, "ZMAINPGM", PROFILE ) ;
}


void loadSystemCommandTable()
{
	// Terminate if ISPCMDS not found

	errblock err ;
	p_tableMGR->loadTable( err, "ISPCMDS", NOWRITE, ZTLIB, SHARE ) ;
	if ( !err.RC0() )
	{
		llog( "C", "Loading of system command table ISPCMDS failed" <<endl ) ;
		llog( "C", "RC="<< err.getRC() <<"  Aborting startup" <<endl ) ;
		llog( "C", "Check path "+ ZTLIB << endl ) ;
		abortStartup() ;
	}
	llog( "I", "Loaded system command table ISPCMDS" << endl ) ;
}


string pfKeyValue( int c )
{
	// Return the value of a pfkey stored in the profile pool.  If it does not exist, VPUT a null value.
	// Also set ZPFKEY in the shared pool to the pfkey pressed

	int keyn ;

	string key ;
	string val ;

	errblock err ;

	keyn = c - KEY_F( 0 ) ;
	key  = "ZPF" + right( d2ds( keyn ), 2, '0' ) ;
	val  = p_poolMGR->get( err, key, PROFILE ) ;
	if ( err.RC8() )
	{
		p_poolMGR->put( err, key, "", PROFILE ) ;
	}

	p_poolMGR->put( err, "ZPFKEY", key.substr( 1 ), SHARED, SYSTEM ) ;
	return val ;
}


void createpfKeyDefaults()
{
	errblock err ;

	for ( int i = 1 ; i < 25 ; i++ )
	{
		p_poolMGR->put( err, "ZPF" + right( d2ds( i ), 2, '0' ), pfKeyDefaults[ i ], PROFILE ) ;
	}
}


void updateDefaultVars()
{
	errblock err ;

	GMAXWAIT = ds2d( p_poolMGR->get( err, "ZMAXWAIT", PROFILE ) ) ;
	GMAINPGM = p_poolMGR->get( err, "ZMAINPGM", PROFILE ) ;
	p_poolMGR->defaultVARs( err, "ZSPLIT", pLScreen::screensTotal > 1 ? "YES" : "NO", SHARED ) ;
}


void updateReflist()
{
	// Check if .NRET is ON and has a valid field name.  If so, add file to the reflist using
	// application ZRFLPGM, parmameters PLA plus the field entry value.

	// Don't update REFLIST if the application has done a CONTROL REFLIST NOUPDATE (flag ControlRefUpdate=false)
	// or ISPS PROFILE variable ZRFURL is not set to YES

	// Save/restore cursor position of the current application, as we don't want the reflist appl to move it

	uint row ;
	uint col ;

	string fname ;

	errblock err ;

	if ( !currAppl->ControlRefUpdate || p_poolMGR->get( err, "ZRFURL", PROFILE ) != "YES" ) { return ; }

	fname = currAppl->get_nretfield() ;
	if ( fname != "" )
	{
		if ( currAppl->currPanel->field_valid( fname ) )
		{
			currAppl->get_cursor( row, col ) ;
			SELCT.clear() ;
			SELCT.PGM     = p_poolMGR->get( err, "ZRFLPGM", PROFILE ) ;
			SELCT.PARM    = "PLA " + currAppl->currPanel->field_getvalue( fname ) ;
			SELCT.NEWAPPL = ""    ;
			SELCT.NEWPOOL = false ;
			SELCT.PASSLIB = false ;
			startApplication( SELCT ) ;
			currScrn->set_row_col( row, col ) ;
		}
		else
		{
			llog( "E", "Invalid field "+ fname +" in .NRET panel statement" << endl ) ;
			issueMessage( "PSYS011Z" ) ;
		}
	}
}


void rawOutput()
{
	// Write raw output to the display and clear the raw messages vector.
	// save/restore the panel stack for this logical screen

	int i ;
	int l ;
	int pos ;

	string respTime ;

	startTime = boost::posix_time::microsec_clock::universal_time() ;

	currScrn->save_panel_stack() ;
	currScrn->clear() ;
	currScrn->show_enter() ;

	l = 0 ;
	attrset( RED | A_BOLD ) ;
	for ( i = 0 ; i < currAppl->rmsgs.size() ; i++ )
	{
		mvaddstr( l++, 0, currAppl->rmsgs[ i ].c_str() ) ;
		if ( l == pLScreen::maxrow-1 || l == currAppl->rmsgs.size() )
		{
			mvaddstr( l, 0, "***" ) ;
			while ( true )
			{
				if ( isActionKey( getch() ) ) { l = 0 ; break ; }
			}
			currScrn->clear() ;
		}
	}
	attroff( RED | A_BOLD ) ;
	currAppl->rmsgs.clear() ;

	endTime  = boost::posix_time::microsec_clock::universal_time() ;
	respTime = to_iso_string(endTime - startTime) ;
	pos      = lastpos( ".", respTime ) ;
	respTime = substr( respTime, (pos - 2) , 6 ) + " s" ;
	currScrn->restore_panel_stack() ;
	currScrn->OIA_update( respTime.c_str() ) ;

}


string listLogicalScreens()
{
	// Mainline lspf cannot create application panels but let's make this as similar as possible
	// Note: get_current_screenName() switches shared pools to get the correct ZSCRNAME.  Switch back afterwards.

	int i ;
	int m ;
	int o ;
	int c ;

	string ln ;
	string t  ;

	errblock err ;

	WINDOW * swwin   ;
	PANEL  * swpanel ;

	pApplication * appl ;

	vector<pLScreen *>::iterator its ;
	vector<string>::iterator it ;
	vector<string> lslist ;

	swwin   = newwin( screenList.size() + 6, 80, 1, 1 ) ;
	swpanel = new_panel( swwin )  ;
	wattrset( swwin, cuaAttr[ AWF ] ) ;
	box( swwin, 0, 0 ) ;
	mvwaddstr( swwin, 0, 34, " Task List " ) ;
	wattroff( swwin, cuaAttr[ AWF ] ) ;

	wattrset( swwin, cuaAttr[ PT ] ) ;
	mvwaddstr( swwin, 1, 28, "Active Logical Sessions" ) ;
	wattroff( swwin, cuaAttr[ PT ] ) ;

	wattrset( swwin, cuaAttr[ PIN ] ) ;
	mvwaddstr( swwin, 3, 2, "ID  Name      Application  Applid  Panel Title/Description" ) ;
	wattroff( swwin, cuaAttr[ PIN ] ) ;

	currScrn->show_wait() ;

	m = 0 ;
	for ( i = 0, its = screenList.begin() ; its != screenList.end() ; its++, i++ )
	{
		appl = (*its)->application_get_current() ;
		ln   = d2ds( (*its)->screenId )          ;
		if      ( i == screenNum ) { ln += "*" ; m = i ; }
		else if ( i == altScreen ) { ln += "-"         ; }
		else                       { ln += " "         ; }
		t = appl->get_current_panelDescr() ;
		if ( t.size() > 42 )
		{
			t.replace( 20, t.size()-39, "..." ) ;
		}
		ln = left( ln, 4 ) +
		     left( appl->get_current_screenName(), 10 ) +
		     left( appl->ZAPPNAME, 13 ) +
		     left( appl->ZZAPPLID, 8  ) +
		     left( t, 42 ) ;
		lslist.push_back( ln ) ;
	}

	p_poolMGR->setShrdPool( err, currAppl->shrdPool ) ;

	o = m         ;
	curs_set( 0 ) ;
	while ( true )
	{
		for ( i = 0, it = lslist.begin() ; it != lslist.end() ; it++, i++ )
		{
			wattron( swwin, cuaAttr[  i == m ? PT : VOI ] ) ;
			mvwaddstr( swwin, i+4, 2, (*it).c_str() )       ;
			wattroff( swwin, cuaAttr[ i == m ? PT : VOI ] ) ;
		}
		update_panels() ;
		doupdate()  ;
		c = getch() ;
		if ( c == KEY_ENTER || c == 13 ) { break ; }
		if ( c == KEY_UP )
		{
			m == 0 ? m = lslist.size() - 1 : m-- ;
		}
		else if ( c == KEY_DOWN || c == 9 )
		{
			m == lslist.size()-1 ? m = 0 : m++ ;
		}
		else if ( isActionKey( c ) )
		{
			m = o ;
			break ;
		}
	}
	del_panel( swpanel ) ;
	delwin( swwin )      ;
	curs_set( 1 )        ;
	return d2ds( m+1 )   ;
}


void listRetrieveBuffer()
{
	// Mainline lspf cannot create application panels but let's make this as similar as possible

	int i ;
	int m ;
	int c ;
	int mx ;
	int RC ;

	uint row ;
	uint col ;

	string t  ;
	string ln ;

	WINDOW * rbwin   ;
	PANEL  * rbpanel ;

	vector<string> lslist ;
	vector<string>::iterator it ;

	if ( retrieveBuffer.empty() )
	{
		retrieveBuffer.push_front( "RETP" ) ;
	}

	mx = (retrieveBuffer.size() > pLScreen::maxrow-6) ? pLScreen::maxrow-6 : retrieveBuffer.size() ;

	rbwin   = newwin( mx+5, 60, 1, 1 ) ;
	rbpanel = new_panel( rbwin )  ;
	wattrset( rbwin, cuaAttr[ AWF ] ) ;
	box( rbwin, 0, 0 ) ;
	mvwaddstr( rbwin, 0, 25, " Retrieve " ) ;
	wattroff( rbwin, cuaAttr[ AWF ] ) ;

	wattrset( rbwin, cuaAttr[ PT ] ) ;
	mvwaddstr( rbwin, 1, 17, "lspf Command Retrieve Panel" ) ;
	wattroff( rbwin, cuaAttr[ PT ] ) ;

	currScrn->show_wait() ;
	for ( i = 0 ; i < mx ; i++ )
	{
		t = retrieveBuffer[ i ] ;
		if ( t.size() > 52 )
		{
			t.replace( 20, t.size()-49, "..." ) ;
		}
		ln = left(d2ds( i+1 )+".", 4 ) + t ;
		lslist.push_back( ln ) ;
	}

	curs_set( 0 ) ;
	m = 0 ;
	while ( true )
	{
		for ( i = 0, it = lslist.begin() ; it != lslist.end() ; it++, i++ )
		{
			wattron( rbwin, cuaAttr[  i == m ? PT : VOI ] ) ;
			mvwaddstr( rbwin, i+3, 3, (*it).c_str() )       ;
			wattroff( rbwin, cuaAttr[ i == m ? PT : VOI ] ) ;
		}
		update_panels() ;
		doupdate()  ;
		c = getch() ;
		if ( c == KEY_ENTER || c == 13 )
		{
			currAppl->currPanel->cmd_setvalue( retrieveBuffer[ m ] ) ;
			currAppl->currPanel->cursor_to_field( RC, currAppl->currPanel->CMDfield, retrieveBuffer[ m ].size()+1 ) ;
			break ;
		}
		else if ( c == KEY_UP )
		{
			m == 0 ? m = lslist.size() - 1 : m-- ;
		}
		else if ( c == KEY_DOWN || c == 9 )
		{
			m == lslist.size() - 1 ? m = 0 : m++ ;
		}
		else if ( isActionKey( c ) )
		{
			currAppl->currPanel->cursor_to_field( RC, currAppl->currPanel->CMDfield ) ;
			break ;
		}
	}
	del_panel( rbpanel ) ;
	delwin( rbwin )      ;
	curs_set( 1 )        ;

	currAppl->currPanel->get_cursor( row, col ) ;
	currScrn->set_row_col( row, col ) ;
	return ;
}


int getScreenNameNum( const string& s )
{
	// Return the screen number of screen name 's'.  If not found, return 0.
	// Reset shared pool after this call as it issues get_current_screenName().

	int l ;

	vector<pLScreen *>::iterator its ;
	pApplication * appl ;

	for ( l = 1, its = screenList.begin() ; its != screenList.end() ; its++, l++ )
	{
		appl = (*its)->application_get_current() ;
		if ( appl->get_current_screenName() == s )
		{
			return l ;
		}
	}
	return 0 ;
}


void threadErrorHandler()
{
	llog( "E", "An exception has occured in an application thread.  See application log for details.  Task ending" << endl ) ;
	try
	{
		currAppl->abendexc() ;
	}
	catch (...)
	{
		llog( "E", "An abend has occured during abend processing.  Calling abend() only to terminate application" << endl ) ;
		currAppl->abend() ;
	}
}


void lspfCallbackHandler( lspfCommand& lc )
{
	//  Issue commands from applications using lspfCallback() function
	//  Replies go into the reply vector

	string w1 ;
	string w2 ;

	vector<pLScreen *>::iterator its   ;
	map<string, appInfo>::iterator ita ;
	pApplication * appl                ;

	lc.reply.clear() ;

	w1 = word( lc.Command, 1 ) ;
	w2 = word( lc.Command, 2 ) ;

	if ( lc.Command == "SWAP LIST" )
	{
		for ( its = screenList.begin() ; its != screenList.end() ; its++ )
		{
			appl = (*its)->application_get_current()       ;
			lc.reply.push_back( d2ds( (*its)->screenId ) ) ;
			lc.reply.push_back( appl->ZAPPNAME        )    ;
		}
		lc.RC = 0 ;
	}
	else if ( lc.Command == "MODULE STATUS" )
	{
		for ( ita = apps.begin() ; ita != apps.end() ; ita++ )
		{
			lc.reply.push_back( ita->first ) ;
			lc.reply.push_back( ita->second.module ) ;
			lc.reply.push_back( ita->second.file   ) ;
			if ( ita->second.mainpgm )
			{
				lc.reply.push_back( "R/Not Reloadable" ) ;
			}
			else if ( ita->second.relPending )
			{
				lc.reply.push_back( "Reload Pending" ) ;
			}
			else if ( ita->second.errors )
			{
				lc.reply.push_back( "Errors" ) ;
			}
			else if ( ita->second.refCount > 0 )
			{
				lc.reply.push_back( "Running" ) ;
			}
			else if ( ita->second.dlopened )
			{
				lc.reply.push_back( "Loaded" ) ;
			}
			else
			{
				lc.reply.push_back( "Not Loaded" ) ;
			}
		}
		lc.RC = 0 ;
	}
	else if ( w1 == "MODREL" )
	{
		reloadDynamicClasses( w2 ) ;
		lc.RC = 0 ;
	}
	else
	{
		lc.RC = 20 ;
	}
}


void abortStartup()
{
	delete screenList[ 0 ] ;
	cout << "Aborting startup of lspf.  Check lspf and application logs for errors " << endl ;
	splog.close() ;
	abort() ;
}


void errorScreen( int etype, const string& msg )
{
	int l    ;
	string t ;

	llog( "E", msg << endl ) ;
	if ( currAppl->errPanelissued ) { return ; }

	currScrn->save_panel_stack() ;
	currScrn->clear() ;
	currScrn->show_enter() ;

	attrset( WHITE | A_BOLD ) ;
	mvaddstr( 0, 0, msg.c_str() ) ;
	mvaddstr( 1, 0, "See lspf and application logs for possible further details of the error" ) ;
	l = 2 ;
	if ( etype == 2 )
	{
		t = "Failing application is " + currAppl->ZAPPNAME + ", taskid=" + d2ds( currAppl->taskid() ) ;
		llog( "E", t << endl ) ;
		mvaddstr( l++, 0, "Depending on the error, application may still be running in the background.  Recommend restarting lspf." ) ;
		mvaddstr( l++, 0, t.c_str() ) ;
	}
	mvaddstr( l, 0, "***" ) ;
	while ( true )
	{
		if ( isActionKey( getch() ) ) { break ; }
	}
	attroff( WHITE | A_BOLD ) ;

	currScrn->restore_panel_stack() ;
}


void issueMessage( const string& msg )
{
	currAppl->set_msg( msg ) ;
	if ( currAppl->RC > 0 )
	{
		errorScreen( 1, "Syntax error in message "+ msg +", message file or message not found" ) ;
	}
}


bool isActionKey( int c )
{
	return ( ( c >= KEY_F(1) && c <= KEY_F(24) ) ||
		   c == KEY_NPAGE ||
		   c == KEY_PPAGE ||
		   c == KEY_ENTER ||
		   c == 27        ||
		   c == 13        ) ;
}


void getDynamicClasses()
{
	// Get modules of the form libABCDE.so from ZLDPATH concatination with name ABCDE and store in map apps
	// Duplicates are ignored with a warning messasge.
	// Terminate lspf if ZMAINPGM module not found as we cannot continue

	int i        ;
	int j        ;
	int pos1     ;

	string appl  ;
	string mod   ;
	string fname ;
	string paths ;
	string p     ;

	typedef vector<path> vec ;
	vec v        ;

	vec::const_iterator it ;

	errblock err ;
	appInfo aI   ;

	const string e1( GMAINPGM +" not found.  Check ZLDPATH is correct.  lspf terminating **" ) ;

	paths = p_poolMGR->get( err, "ZLDPATH", PROFILE ) ;
	j     = getpaths( paths ) ;
	for ( i = 1 ; i <= j ; i++ )
	{
		p = getpath( paths, i ) ;
		if ( is_directory( p ) )
		{
			llog( "I", "Searching directory "+ p +" for application classes" << endl ) ;
			copy( directory_iterator( p ), directory_iterator(), back_inserter( v ) ) ;
		}
		else
		{
			llog( "W", "Ignoring directory "+ p +"  Not found or not a directory." << endl ) ;
		}
	}

	for ( it = v.begin() ; it != v.end() ; ++it )
	{
		fname = it->string() ;
		p     = substr( fname, 1, (lastpos( "/", fname ) - 1) ) ;
		mod   = substr( fname, (lastpos( "/", fname ) + 1) )    ;
		pos1  = pos( ".so", mod ) ;
		if ( substr(mod, 1, 3 ) != "lib" || pos1 == 0 ) { continue ; }
		appl  = substr( mod, 4, (pos1 - 4) ) ;
		if ( apps.count( appl ) > 0 )
		{
			llog( "W", "Ignoring duplicate module "+ mod +" found in "+ p << endl ) ;
			continue ;
		}
		llog( "I", "Adding application "+ appl << endl ) ;
		aI.file       = fname ;
		aI.module     = mod   ;
		aI.mainpgm    = false ;
		aI.dlopened   = false ;
		aI.errors     = false ;
		aI.relPending = false ;
		aI.refCount   = 0     ;
		apps[ appl ]  = aI    ;
	}
	llog( "I", d2ds( apps.size() ) +" applications found and stored" << endl ) ;
	if ( apps.find( GMAINPGM ) == apps.end() )
	{
		llog( "C", e1 << endl ) ;
		abortStartup()          ;
	}
}


void reloadDynamicClasses( string parm )
{
	// Reload modules (ALL, NEW or modname).  Ignore reload for modules currently in-use but set
	// pending flag to be checked when application terminates.

	int i        ;
	int j        ;
	int k        ;
	int pos1     ;

	string appl  ;
	string mod   ;
	string fname ;
	string paths ;
	string p     ;

	bool stored  ;

	errblock err ;

	typedef vector<path> vec ;
	vec v      ;
	appInfo aI ;

	vec::const_iterator it ;

	paths = p_poolMGR->get( err, "ZLDPATH", PROFILE ) ;
	j     = getpaths( paths ) ;
	for ( i = 1 ; i <= j ; i++ )
	{
		p = getpath( paths, i ) ;
		if ( is_directory( p ) )
		{
			llog( "I", "Searching directory "+ p +" for application classes" << endl ) ;
			copy( directory_iterator( p ), directory_iterator(), back_inserter( v ) ) ;
		}
		else
		{
			llog( "W", "Ignoring directory "+ p +"  Not found or not a directory." << endl ) ;
		}
	}
	if ( parm == "" ) { parm = "ALL" ; }

	i = 0 ;
	j = 0 ;
	k = 0 ;
	for ( it = v.begin() ; it != v.end() ; ++it )
	{
		fname = it->string() ;
		p     = substr( fname, 1, (lastpos( "/", fname ) - 1) ) ;
		mod   = substr( fname, (lastpos( "/", fname ) + 1) )    ;
		pos1  = pos( ".so", mod ) ;
		if ( substr(mod, 1, 3 ) != "lib" || pos1 == 0 ) { continue ; }
		appl  = substr( mod, 4, (pos1 - 4) ) ;
		llog( "I", "Found application "+ appl << endl ) ;
		stored = ( apps.find( appl ) != apps.end() ) ;

		if ( parm == "NEW" && stored ) { continue ; }
		if ( parm != "NEW" && parm != "ALL" && parm != appl ) { continue ; }
		if ( appl == GMAINPGM ) { continue ; }
		if ( parm == appl && stored && !apps[ appl ].dlopened )
		{
			apps[ appl ].file = fname ;
			llog( "W", "Application "+ appl +" not loaded.  Ignoring action" << endl ) ;
			return ;
		}
		if ( stored )
		{
			apps[ appl ].file = fname ;
			if ( apps[ appl ].refCount > 0 )
			{
				llog( "W", "Application "+ appl +" in use.  Reload pending" << endl ) ;
				apps[ appl ].relPending = true ;
				continue ;
			}
			if ( apps[ appl ].dlopened )
			{
				if ( loadDynamicClass( appl ) )
				{
					llog( "I", "Loaded "+ appl +" (module "+ mod +") from "+ p << endl ) ;
					i++ ;
				}
				else
				{
					llog( "W", "Errors occured loading "+ appl << endl ) ;
					k++ ;
				}
			}
		}
		else
		{
			llog( "I", "Adding new module "+ appl << endl ) ;
			aI.file        = fname ;
			aI.module      = mod   ;
			aI.dlopened    = false ;
			aI.errors      = false ;
			aI.relPending  = false ;
			aI.refCount    = 0     ;
			apps[ appl ]   = aI    ;
			j++ ;
		}
		if ( parm == appl ) { break ; }
	}

	issueMessage( "PSYS012G" ) ;
	llog( "I", d2ds( i ) +" applications reloaded" << endl ) ;
	llog( "I", d2ds( j ) +" new applications stored" << endl ) ;
	llog( "I", d2ds( k ) +" errors encounted" << endl ) ;
	if ( parm != "ALL" && parm != "NEW" )
	{
		if ( (i+j) == 0 )
		{
			llog( "W", "Application "+ parm +" not reloaded/stored" << endl ) ;
			issueMessage( "PSYS012I" ) ;
		}
		else
		{
			llog( "I", "Application "+ parm +" reloaded/stored" << endl )   ;
			issueMessage( "PSYS012H" ) ;
		}
	}

	llog( "I", d2ds( apps.size() ) + " applications currently stored" << endl ) ;
}


bool loadDynamicClass( const string& appl )
{
	// Load module related to application appl and retrieve address of maker and destroy symbols
	// Perform dlclose first if there has been a previous successful dlopen, or if an error is encountered

	// Routine only called if the refCount is zero

	string mod   ;
	string fname ;

	void *dlib  ;
	void *maker ;
	void *destr ;

	const char* dlsym_err ;

	mod   = apps[ appl ].module ;
	fname = apps[ appl ].file   ;
	apps[ appl ].errors = true  ;

	if ( apps[ appl ].dlopened )
	{
		llog( "I", "Closing "+ appl << endl ) ;
		if ( !unloadDynamicClass( apps[ appl ].dlib ) )
		{
			llog( "W", "dlclose has failed for "+ appl << endl ) ;
			return false ;
		}
		apps[ appl ].dlopened = false ;
		llog( "I", "Reloading module "+ appl << endl ) ;
	}

	dlerror() ;
	dlib = dlopen( fname.c_str(), RTLD_NOW ) ;
	if ( !dlib )
	{
		llog( "E", "Error loading "+ fname << endl )  ;
		llog( "E", "Error is " << dlerror() << endl ) ;
		llog( "E", "Module "+ mod +" will be ignored" << endl ) ;
		return false ;
	}

	dlerror() ;
	maker     = dlsym( dlib, "maker" ) ;
	dlsym_err = dlerror() ;
	if ( dlsym_err )
	{
		llog( "E", "Error loading symbol maker" << endl ) ;
		llog( "E", "Error is " << dlsym_err << endl )     ;
		llog( "E", "Module "+ mod +" will be ignored" << endl ) ;
		unloadDynamicClass( apps[ appl ].dlib ) ;
		return false ;
	}

	dlerror() ;
	destr     = dlsym( dlib, "destroy" ) ;
	dlsym_err = dlerror() ;
	if ( dlsym_err )
	{
		llog( "E", "Error loading symbol destroy" << endl ) ;
		llog( "E", "Error is " << dlsym_err << endl )       ;
		llog( "E", "Module "+ mod +" will be ignored" << endl ) ;
		unloadDynamicClass( apps[ appl ].dlib ) ;
		return false ;
	}

	debug1( fname +" loaded at " << dlib << endl ) ;
	debug1( "Maker at " << maker << endl ) ;
	debug1( "Destroyer at " << destr << endl ) ;

	apps[ appl ].dlib         = dlib  ;
	apps[ appl ].maker_ep     = maker ;
	apps[ appl ].destroyer_ep = destr ;
	apps[ appl ].mainpgm      = false ;
	apps[ appl ].errors       = false ;
	apps[ appl ].dlopened     = true  ;

	return true ;
}


bool unloadDynamicClass( void * dlib )
{
	int i  ;
	int rc ;

	for ( i = 0 ; i < 100 ; i++ )
	{
		try
		{
			rc = dlclose( dlib ) ;
		}
		catch (...)
		{
			llog( "E", "An exception has occured during dlclose" << endl ) ;
			return false ;
		}
		if ( rc != 0 ) { break ; }
	}
	if ( rc == 0 ) { return false ; }
	return true ;
}
