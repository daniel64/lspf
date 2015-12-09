/*  Compile with ::                                                                                                                                                  */
/* g++ -g -O0 -std=c++11 -rdynamic -Wunused-variable -lncurses -lpanel -lboost_thread -lboost_filesystem -lboost_system -ldl -lpthread -o lspf lspf.cpp -std=c++11   */

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


#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <time.h>
#include <vector>
#include <ncurses.h>
#include <dlfcn.h>
#include <sys/utsname.h>

#include <locale>
#include <boost/date_time.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>

boost::condition cond_appl ;
boost::mutex global ;

#include "lspf.h"

#include "utilities.h"
#include "utilities.cpp"

map<string, cuaType> cuaAttrName    ;
map<cuaType, unsigned int> cuaAttr  ;
map<int, unsigned int> usrAttr      ;
map<cuaType, bool> cuaAttrProt      ;

string usrAttrNames = "N_RED N_GREEN N_YELLOW N_BLUE N_MAGENTA N_TURQ N_WHITE " \
		      "B_RED B_GREEN B_YELLOW B_BLUE B_MAGENTA B_TURQ B_WHITE " \
		      "R_RED R_GREEN R_YELLOW R_BLUE R_MAGENTA R_TURQ R_WHITE " \
		      "U_RED U_GREEN U_YELLOW U_BLUE U_MAGENTA U_TURQ U_WHITE " \
		      "P_RED P_GREEN P_YELLOW P_BLUE P_MAGENTA P_TURQ P_WHITE" ;

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

#include "pLScreen.h"
#include "pLScreen.cpp"

#undef MOD_NAME
#undef LOGOUT

#define MOD_NAME lspf
#define LOGOUT   splog

#define currScrn  pLScreen::currScreen
#define currAppl  pApplication::currApplication

using namespace std ;
using namespace boost::filesystem ;

int  pLScreen::screensTotal = 0 ;
int  pLScreen::maxScreenID  = 0 ;
int  pLScreen::maxrow       = 0 ;
int  pLScreen::maxcol       = 0 ;

boost::posix_time::ptime   startTime ;
boost::posix_time::ptime   endTime ;

pLScreen     * pLScreen::currScreen          = NULL ;
pApplication * pApplication::currApplication = NULL ;

poolMGR  * p_poolMGR  = new poolMGR  ;
tableMGR * p_tableMGR = new tableMGR ;

fPOOL  funcPOOL ;

void initialSetup()          ;
void loadDefaultPools()      ;
void loadDynamicClasses()    ;
void loadCommandTable()      ;
void loadApplicationCommandTable( string ) ;
void loadpfkeyTable()        ;
void loadcuaTables()         ;
void setColourPair( string ) ;
void updateDefaultVars()     ;
void updateReflist()         ;
void startApplication( string, string, string, bool, bool ) ;
void terminateApplication()  ;
void processSELECT()         ;
void processAction( uint row, uint col, int c, bool & passthru )  ;
void errorScreen( int, string ) ;
void threadErrorHandler()       ;
void MainLoop() ;

vector<pLScreen *>   screenList   ;
map<int, string>     pfkeyTable   ;
map<string, void *>  dlibs        ;
map<string, void *>  maker_ep     ;
map<string, void *>  destroyer_ep ;

boost::circular_buffer<string> retrieveBuffer(25) ;

int    maxtaskID = 0 ;
int    screenNum = 0 ;
int    altScreen = 0 ;
int    RC            ;
bool   pfkeyPressed  ;
string commandStack  ;
string jumpOption    ;

bool showPanelID = false ;

string ZCOMMAND ;
string ZPARM    ;
string ZTLIB    ;
string ZCTVERB, ZCTTRUNC, ZCTACT, ZCTDESC ;

const char ZSCREEN[] = { '1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W' } ;

bool	SEL         ;
string	SEL_PGM     ;
string	SEL_PARM    ;
string	SEL_NEWAPPL ;
bool	SEL_NEWPOOL ;
bool	SEL_PASSLIB ;

string ZHOME  ;
string ZUSER  ;
string ZSHELL ;

static string BuiltInCommands = "ABEND ACTION DISCARD FIELDEXC INFO NOP PANELID REFRESH SCALE SNAP SHELL SPLIT STATS SWAP TASKS TEST TDOWN" ;

ofstream splog(SLOG) ;


int main(void)
{
	int  RC      ;
	int  elapsed ;
	uint row     ;
	uint col     ;

	boost::thread * pThread              ;
	set_terminate ( threadErrorHandler ) ;

	startTime    = boost::posix_time::microsec_clock::universal_time() ;
	commandStack = "" ;

	pLScreen * p_pLScreen = new pLScreen ;
	screenList.push_back ( p_pLScreen ) ;

	log( "I", "lspf startup in progress" << endl ) ;

	log( "I", "Calling initialSetup" << endl ) ;
	initialSetup() ;

	log( "I", "Calling loadSystemDefaultVars" << endl ) ;
	loadDefaultPools() ;

	log( "I", "Calling loadDynamicClasses" << endl ) ;
	loadDynamicClasses() ;

	log( "I", "Calling loadcuaTables" << endl ) ;
	loadcuaTables() ;

	log( "I", "Calling loadCommandTable" << endl ) ;
	loadCommandTable() ;

	updateDefaultVars() ;

	log( "I", "Starting new " << ZMAINPGM << " thread" << endl ) ;
	currAppl = ((pApplication*(*)())( maker_ep[ ZMAINPGM ]))()   ;

	currScrn->application_add( currAppl ) ;
	currAppl->ZAPPNAME   = ZMAINPGM       ;
	currAppl->taskID     = ++maxtaskID    ;
	currAppl->busyAppl   = true           ;
	currAppl->p_poolMGR  = p_poolMGR      ;
	currAppl->p_tableMGR = p_tableMGR     ;
	currAppl->ZZAPPLID   =  "ISP"         ;
	currAppl->NEWPOOL    = true           ;

	p_poolMGR->put( RC, "ZSCREEN", string( 1, ZSCREEN[ screenNum ] ), SHARED, SYSTEM ) ;
	p_poolMGR->put( RC, "ZAPPLID", "ISP", SHARED, SYSTEM ) ;
	currAppl->shrdPool = p_poolMGR->getshrdPool() ;
	currAppl->init() ;
	loadpfkeyTable() ;

	pThread = new boost::thread(boost::bind(&pApplication::application, currAppl ) ) ;
	currAppl->pThread = pThread ;

	log( "I", "Waiting for " << ZMAINPGM << " to complete startup" << endl ) ;
	elapsed = 0 ;
	while ( currAppl->busyAppl )
	{
		elapsed++ ;
		boost::this_thread::sleep_for(boost::chrono::milliseconds(ZWAIT)) ;
		if ( elapsed > ZMAXWAIT ) { currAppl->set_forced_abend() ; }
	}
	if ( currAppl->abnormalEnd || currAppl->terminateAppl )
	{
		errorScreen( 1, "An error has occured initialising the first " ZMAINPGM " main task.  lspf cannot continue." ) ;
		currAppl->info() ;
		currAppl->closeTables() ;
		log( "I", "Removing application instance of " << currAppl->ZAPPNAME << endl ) ;
		((void (*)(pApplication*))(destroyer_ep[ currAppl->ZAPPNAME ]))( currAppl )  ;
		delete pThread    ;
		delete p_pLScreen ;
		delete p_poolMGR  ;
		delete p_tableMGR ;
		log( "I", "lspf and LOG terminating" << endl ) ;
		splog.close() ;
		return 0 ;
	}
	log( "I", "First thread " << ZMAINPGM << " started and initialised.  ID=" << pThread->get_id() << endl ) ;

	currAppl->currPanel->get_cursor( row, col ) ;
	currScrn->set_row_col( row, col )           ;

	MainLoop() ;
	debug1( "Back from Mainloop" << endl ) ;

	delete p_poolMGR  ;
	delete p_tableMGR ;

//	for ( it = dlibs.begin() ; it != dlibs.end() ; it++ )
//	{
//		dlerror() ;
//		log( "I", "dlclose of " << it->first << " at " << it->second << endl ) ;
//		dlclose( it->second ) ;
//		debug1( "dlerror " << dlerror() << endl ) ;
//	}

	log( "I", "lspf and LOG terminating" <<endl ) ;
	splog.close() ;
	return 0 ;
}


void MainLoop()
{
	log( "I", "MAINLOOP() entered" << endl ) ;

	int RC      ;
	int elapsed ;
	int pos     ;
	int ws      ;
	int i       ;
	uint t      ;
	uint c      ;

	fieldExc fxc ;

	char ch     ;
	string Isrt   ;

	uint row    ;
	uint col    ;

	MEVENT event  ;

	bool passthru ;
	bool showLock ;
	bool Insert   ;

	string respTime   ;
	string field_name ;
	string ww         ;

	pLScreen * p_pLScreen ;
	p_pLScreen = screenList[ screenNum ] ;

	mousemask( ALL_MOUSE_EVENTS, NULL ) ;
	showLock = false ;
	Insert   = false ;

	while ( true )
	{
		if ( pLScreen::screensTotal == 0 ) return ;

		currScrn->busy_clear() ;
		if ( Insert ) { Isrt = "Insert" ; curs_set(2) ; }
		else          { Isrt = "      " ; curs_set(1) ; }

		row = currScrn->get_row() ;
		col = currScrn->get_col() ;

		endTime  = boost::posix_time::microsec_clock::universal_time() ;
		respTime = to_iso_string(endTime - startTime) ;
		pos      = lastpos( ".", respTime ) ;
		respTime = "    Elapsed: " + substr( respTime, (pos - 2) , 6 ) + " s" ;

		if ( showPanelID )
		{
			attrset( cuaAttr[ PI ] ) ;
			mvprintw( 2, 0, "%s", currAppl->PANELID.c_str() ) ;
			attroff( cuaAttr[ PI ] ) ;
		}
		attrset( YELLOW ) ;
		mvaddstr( pLScreen::maxrow+1, pLScreen::maxcol-23, Isrt.c_str() ) ;
		mvprintw( pLScreen::maxrow+1, pLScreen::maxcol-14, "Row %d Col %d  ", row+1, col+1 ) ;
		mvaddstr( pLScreen::maxrow+1, 27, respTime.c_str() ) ;
		mvprintw( pLScreen::maxrow+1, 52, "Screen:%d-%d   ", pLScreen::currScreen->screenID, currScrn->application_stack_size() ) ;
		mvaddstr( pLScreen::maxrow+1, 2, "Screen[        ]" ) ;
		mvaddstr( pLScreen::maxrow+1, 9, substr( "12345678", 1, pLScreen::screensTotal).c_str() ) ;
		attroff( YELLOW ) ;
		if ( screenNum < 8 )
		{
			attrset( RED | A_REVERSE ) ;
			mvaddch( pLScreen::maxrow+1, 9+screenNum, d2ds( screenNum+1 )[0] ) ;
			attroff( RED | A_REVERSE ) ;
		}
		if ( altScreen < 8 && altScreen != screenNum )
		{
			attrset( YELLOW | A_BOLD | A_UNDERLINE ) ;
			mvaddch( pLScreen::maxrow+1, 9+altScreen, d2ds( altScreen+1 )[0] ) ;
			attroff( YELLOW | A_BOLD | A_UNDERLINE ) ;
		}
		if ( showLock )
		{
			attrset( RED ) ;
			mvaddstr( pLScreen::maxrow+1, 27, "|X|" ) ;
			attroff( RED ) ;
			showLock = false ;
		}
		move( row, col ) ;
		pfkeyPressed = false ;
		if ( commandStack == "" && !currAppl->ControlDisplayLock )
		{
			currAppl->nrefresh() ;
			doupdate()  ;
			c = getch() ;
		}
		else
		{
			c = KEY_ENTER ;
		}
		currAppl->ControlDisplayLock = false ;
		ch        = char( c ) ;
		startTime = boost::posix_time::microsec_clock::universal_time() ;
		if ( c == 13 ) { c = KEY_ENTER ; } ;

		if ( c < 256 )
		{
			if ( isprint( c ) )
			{
				if ( currAppl->currPanel->is_pd_displayed() ) { currScrn->busy_show() ; continue ; }
				currAppl->currPanel->field_edit( row, col, ch , Insert, showLock ) ;
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
			case KEY_LEFT:	col = currScrn->cursor_left()  ; break ;
			case KEY_RIGHT: col = currScrn->cursor_right() ; break ;
			case KEY_UP:    row = currScrn->cursor_up()    ; break ;
			case KEY_DOWN:  row = currScrn->cursor_down()  ; break ;
			case 9:   // Tab
				currAppl->currPanel->field_tab_next( row, col ) ;
				currScrn->set_row_col( row, col ) ;
				break ;

			case KEY_IC:
				Insert = !Insert ;
				break ;

			case KEY_HOME:
				currAppl->get_home( row, col ) ;
				currScrn->set_row_col( row, col ) ;
				break ;

			case KEY_DC:
				currAppl->currPanel->field_delete_char( row, col, showLock ) ;
				break ;

			case KEY_SDC:
				currAppl->currPanel->field_erase_eof( row, col, showLock ) ;
				break ;

			case KEY_END:
				currAppl->currPanel->cursor_eof( row, col ) ;
				currScrn->set_row_col( row, col ) ;
				break ;

			case 127:
			case KEY_BACKSPACE:
				currAppl->currPanel->field_backspace( row, col, showLock ) ;
				currScrn->set_row_col( row, col ) ;
				break ;
			case KEY_F(1):  case KEY_F(2):  case KEY_F(3):  case KEY_F(4):  case KEY_F(5):  case KEY_F(6):            // All action keys follow
			case KEY_F(7):  case KEY_F(8):  case KEY_F(9):  case KEY_F(10): case KEY_F(11): case KEY_F(12):
			case KEY_F(13): case KEY_F(14): case KEY_F(15): case KEY_F(16): case KEY_F(17): case KEY_F(18):
			case KEY_F(19): case KEY_F(20): case KEY_F(21): case KEY_F(22): case KEY_F(23): case KEY_F(24):
				pfkeyPressed = true ;
			case KEY_NPAGE:
			case KEY_PPAGE:
			case KEY_ENTER:
				debug1( "Action key pressed.  Processing" << endl ) ;
				Insert = false ;
				if ( commandStack == "" ) { currScrn->busy_show() ; }
				updateDefaultVars() ;
				processAction( row, col, c, passthru ) ;
				if ( passthru )
				{
					elapsed = 0                      ;
					currAppl->set_cursor( row, col ) ;
					currAppl->busyAppl   = true      ;
					cond_appl.notify_all()           ;
					while ( currAppl->busyAppl )
					{
						elapsed++ ;
						boost::this_thread::sleep_for(boost::chrono::milliseconds(ZWAIT)) ;
						if ( currAppl->noTimeOut ) { elapsed = 0 ; }
						if ( elapsed > ZMAXWAIT  ) { currAppl->set_forced_abend() ; }
					}
					if ( currAppl->abnormalEnd )
					{
						currAppl->terminateAppl = true ;
						if ( currAppl->abnormalEndForced )
						{
							errorScreen( 2, "A forced termination of the subtask has occured" ) ;
						}
						else
						{
							errorScreen( 2, "An error has occured during application execution" ) ;
						}
					}
					if ( currAppl->reloadCUATables ) { loadcuaTables() ; }
					currAppl->currPanel->get_cursor( row, col ) ;
					currScrn->set_row_col( row, col ) ;
					if ( currAppl->SEL )
					{
						processSELECT() ;
					}
					while ( currAppl->terminateAppl )
					{
						terminateApplication() ;
						if ( pLScreen::screensTotal == 0 ) return ;
						if ( currAppl->SEL && !currAppl->terminateAppl )
						{
							debug1( "Application " << currAppl->ZAPPNAME << " has done another SELECT without a DISPLAY (1) !!!! " << endl ) ;
							processSELECT() ;
						}
					}
					updateReflist() ;
				}
				else
				{
					if ( ZCOMMAND == "ABEND" )
					{
						currAppl->set_forced_abend() ;
					}
					else if ( ZCOMMAND == "ACTION" )
					{
						if ( currAppl->currPanel->is_pd_displayed() )
						{
							currAppl->currPanel->display_pd_next() ;
							currAppl->currPanel->get_cursor( row, col ) ;
							currScrn->set_row_col( row, col ) ;
							break ;
						}
						else
							if ( currAppl->currPanel->display_pd( 0, 2 ) )
							{
								currAppl->currPanel->get_cursor( row, col ) ;
								currScrn->set_row_col( row, col ) ;
								break ;
							}
					}
					else if ( ZCOMMAND == "DISCARD" )
					{
						currAppl->currPanel->display_panel( RC ) ;
					}
					else if ( ZCOMMAND == "FIELDEXC" )
					{
						
						field_name = currAppl->currPanel->field_getname( row, col ) ;
						fxc = currAppl->currPanel->field_getexec( field_name )      ;
						if ( fxc.fieldExc_command != "" )
						{
							selectParse( RC, subword( fxc.fieldExc_command, 2 ), SEL_PGM, SEL_PARM, SEL_NEWAPPL, SEL_NEWPOOL, SEL_PASSLIB ) ;
							if ( RC > 0 )
							{
								log( "E", "Error in FIELD SELECT command " << fxc.fieldExc_command << endl ) ;
								currAppl->setmsg( "PSYS01K" ) ;
								break ;
							}
							SEL_PARM = SEL_PARM + " " + currAppl->currPanel->field_getvalue( field_name ) ;
							currAppl->field_name = field_name ;
							currAppl->currPanel->field_get_row_col( field_name, t, c ) ;
							p_poolMGR->put( RC, "ZFECSRP", d2ds( col-c+1 ), SHARED )   ;
							for( ws = words( fxc.fieldExc_passed ), i = 1 ; i <= ws ; i++ )
							{
								ww = word( fxc.fieldExc_passed, i ) ;
								p_poolMGR->put( RC, "ZFEDATA" + d2ds( i ), currAppl->currPanel->field_getvalue( ww ) , SHARED ) ;
							}
							startApplication( SEL_PGM, SEL_PARM, SEL_NEWAPPL, SEL_NEWPOOL, SEL_PASSLIB ) ;
							break ;
						}
					}
					else if ( ZCOMMAND == "INFO" )
					{
						currAppl->info() ;
					}
					else if ( ZCOMMAND == "NOP" )
					{
						break ;
					}
					else if ( ZCOMMAND == "PANELID" )
					{
						showPanelID = !showPanelID ;
						currAppl->currPanel->refresh( RC ) ;
					}
					else if ( ZCOMMAND == "REFRESH" )
					{
						currAppl->currPanel->refresh( RC ) ;
						reset_prog_mode() ;
						refresh() ;
					}
					else if ( ZCOMMAND == "SCALE" )
					{
						if ( ZPARM == "ON" )
						{
							p_poolMGR->defaultVARs( RC, "ZSCALE", "ON", SHARED ) ;
						}
						else
							if ( ZPARM == "OFF" )
							{
								p_poolMGR->defaultVARs( RC, "ZSCALE", "OFF", SHARED ) ;
							}
					}
					else if ( ZCOMMAND == "SHELL" )
					{
						def_prog_mode()   ;
						endwin()          ;
						system( ZSHELL.c_str() )  ;
						reset_prog_mode() ;
						refresh()         ;
					}
					else if ( ZCOMMAND == "SPLIT" )
					{

						if ( !currAppl->ControlSplitEnable )
						{
							log( "W", "SPLIT mode disabled due to application-issued CONTROL SPLIT DISABLE service" << endl ) ;
							break ;
						}
						if ( pLScreen::screensTotal == ZMAXSCRN ) continue ;
						altScreen  = screenNum         ;
						screenNum  = screenList.size() ;
						p_pLScreen = new pLScreen      ;
						screenList.push_back ( p_pLScreen ) ;
						updateDefaultVars()            ;
						startApplication( ZMAINPGM, "", "ISP", true, false ) ;
						break ;
					}
					else if ( ZCOMMAND == "STATS" )
					{
						p_poolMGR->statistics() ;
						p_tableMGR->statistics() ;
					}
					else if ( ZCOMMAND == "SNAP" )
					{
						p_poolMGR->snap() ;
						p_tableMGR->snap() ;
					}
					else if ( ZCOMMAND == "SWAP" )
					{
						if ( pLScreen::screensTotal == 1 ) { continue ; }
						if ( ZPARM == "" )
						{
							t         = screenNum ;
							screenNum = altScreen ;
							altScreen = t         ;
						}
						else if ( ZPARM == "NEXT" )
						{
							++screenNum ;
							screenNum = (screenNum == pLScreen::screensTotal ? 0 : screenNum) ;
							if ( altScreen == screenNum ) altScreen = (altScreen == 0 ? (pLScreen::screensTotal - 1) : (altScreen - 1) ) ;  
						}
						else if ( ZPARM == "PREV" )
						{
							--screenNum ;
							screenNum = (screenNum < 0 ? (pLScreen::screensTotal - 1) : screenNum) ;
							if ( altScreen == screenNum ) altScreen = ((altScreen == pLScreen::screensTotal - 1) ? 0 : (altScreen + 1) )  ;  
						}
						else if ( datatype( ZPARM, 'W' ) )
						{
							t = ds2d( ZPARM ) - 1 ;
							if ( t != screenNum && t >= 0 && t < pLScreen::screensTotal )
							{
								if ( t == altScreen ) { altScreen = screenNum ; }
								screenNum = t ;
							}
						}
						p_pLScreen           = screenList[ screenNum ] ;
						pLScreen::currScreen = p_pLScreen ;
						currAppl             = p_pLScreen->application_get_current() ;
						p_poolMGR->setAPPLID( RC, currAppl->ZZAPPLID )   ;
						p_poolMGR->setshrdPool( RC, currAppl->shrdPool ) ;
						p_poolMGR->put( RC, "ZPANELID", currAppl->PANELID, SHARED, SYSTEM ) ;
						pLScreen::currScreen->clear() ;
						currAppl->refresh() ;
						loadpfkeyTable() ;
						break ;
					}
					else if ( ZCOMMAND == "TASKS" )
					{
						log( "N", "Task display not yet implemented" << endl ) ;
					}
					else if ( ZCOMMAND == "TEST" )
					{
						currAppl->testMode = true ;
						log( "W", "Application is now running in test mode" << endl ) ;
					}
					else if ( ZCOMMAND == "TDOWN" )
					{
						currAppl->currPanel->field_tab_down( row, col ) ;
						currScrn->set_row_col( row, col ) ;
					}
					if ( SEL )
					{
						updateDefaultVars()                   ;
						currAppl->currPanel->abActive = false ;
						startApplication( SEL_PGM, SEL_PARM, SEL_NEWAPPL, SEL_NEWPOOL, SEL_PASSLIB ) ;
					}
					currAppl->get_home( row, col ) ;
					currScrn->set_row_col( row, col ) ;
				}
				break ;
			default:
				debug1( "Action key " << c << " ignored" << endl ) ;
		}
	}
}


void initialSetup()
{
	char* home  = getenv( "HOME" )    ;
	char* user  = getenv( "LOGNAME" ) ;
	char* shell = getenv( "SHELL" )   ;

	ZHOME.assign( home)    ;
	ZUSER.assign( user )   ;
	ZSHELL.assign( shell ) ;

	funcPOOL.define( RC, "ZCTVERB",  &ZCTVERB  ) ;
	funcPOOL.define( RC, "ZCTTRUNC", &ZCTTRUNC ) ;
	funcPOOL.define( RC, "ZCTACT",   &ZCTACT   ) ;
	funcPOOL.define( RC, "ZCTDESC",  &ZCTDESC  ) ;
}


void processAction( uint row, uint col, int c, bool &  passthru )
{
	// perform lspf high-level functions - pfkey -> command
	// application/user/system command table entry?
	// BUILTIN command
	// RETRIEVE
	// Jump command entered
	// Else pass event to application

	int  RC ;
	int  p1 ;
	uint x  ;

	string CMDVerb ;
	string CMDRest ;
	string PFCMD   ;
	static uint retPos(0) ;

	pdc t_pdc ;

	SEL       = false ;
	ZCTVERB   = ""    ;
	ZCTTRUNC  = ""    ;
	ZCTACT    = ""    ;
	ZCTDESC   = ""    ;

	PFCMD     = ""    ;
	passthru  = true  ;


	p_poolMGR->put( RC, "ZVERB", "", SHARED ) ;
	if ( RC != 0 ) { log( "C", "poolMGR put for ZVERB failed" << endl ) ; }

	if ( c == KEY_ENTER )
	{
		if ( row == currAppl->currPanel->get_abline() )
		{
			if ( currAppl->currPanel->display_pd( row, col ) )
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
			t_pdc = currAppl->currPanel->retrieve_pdc( row, col ) ;
			if ( t_pdc.pdc_run != "" )
			{
				ZCOMMAND  = t_pdc.pdc_run ;
				if ( ZCOMMAND == "ISRROUTE" )
				{
					CMDVerb = word( t_pdc.pdc_parm, 1 )    ;
					CMDRest = subword( t_pdc.pdc_parm, 2 ) ;
					if ( CMDVerb == "SELECT" )
					{
						selectParse( RC, CMDRest, SEL_PGM, SEL_PARM, SEL_NEWAPPL, SEL_NEWPOOL, SEL_PASSLIB) ;
						if ( RC > 0 )
						{
							log( "E", "Error in SELECT command " << t_pdc.pdc_parm << endl ) ;
							currAppl->setmsg( "PSYS01K" ) ;
							return ;
						}
						if ( substr( SEL_PGM, 1, 1 ) == "&" )
						{
							currAppl->vcopy( substr( SEL_PGM, 2 ), SEL_PGM, MOVE ) ;
						}
						SEL      = true  ;
						passthru = false ;
						return           ;
					}
				}
			}
		}
	}

	currAppl->currPanel->hide_pd() ;

	if ( t_pdc.pdc_run == "" ) { ZCOMMAND  = strip( currAppl->currPanel->cmd_getvalue() ) ; }

	if ( commandStack != "" )
	{
		if ( ZCOMMAND != "" )
		{
			currAppl->currPanel->cmd_setvalue( ZCOMMAND + ";" + commandStack ) ;
			commandStack = "" ;
			return ;
		}
		else
		{
			ZCOMMAND     = commandStack ;
			commandStack = "" ;
		}
	}

	if ( pfkeyPressed )
	{
		if ( p_poolMGR->get( RC, "ZKLUSE", PROFILE ) == "Y" )
		{
			PFCMD = currAppl->currPanel->get_keylist( c ) ;
			if ( PFCMD == "" ) { PFCMD = strip( pfkeyTable[ c ] ) ; }
		}
		else
		{
			debug1( "PF Key pressed " << pfkeyTable[ c ] << endl ) ;
			PFCMD = strip( pfkeyTable[ c ] ) ;
		}
		p_poolMGR->put( RC, "ZPFKEY", "PF" + right( d2ds( (c - KEY_F(0)) ), 2, '0'), SHARED, SYSTEM ) ;
		if ( RC > 0 ) { log( "C", "VPUT for PFnn failed" << endl ) ; }
	}
	else
	{
		p_poolMGR->put( RC, "ZPFKEY", "PF00", SHARED, SYSTEM ) ;
		if ( RC > 0 ) { log( "C", "VPUT for PF00 failed" << endl ) ; }
	}

	if ( PFCMD == "SWAP" ) { ZCOMMAND = "" ; }

	if ( ZCOMMAND.size() > 3 && upper( ZCOMMAND ) != "RETRIEVE" && upper( PFCMD ) != "RETRIEVE" )
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

	switch( c )
	{
		case KEY_NPAGE:
			ZCOMMAND = "DOWN " + ZCOMMAND ;
			break ;
		case KEY_PPAGE:
			ZCOMMAND = "UP " + ZCOMMAND  ;
			break ;
	}

	if ( ( substr( ZCOMMAND, 1, 1 ) == "=" ) && ( substr( ZCOMMAND, 2 ) != "" ) && ( PFCMD == "" || PFCMD == "RETURN" ) )
	{
		debug1( "JUMP entered.  Jumping to primary menu option " << substr( ZCOMMAND, 2 ) << endl ) ;
		passthru = true ;
		ZCOMMAND = substr( ZCOMMAND, 2 ) ;
		if ( !currAppl->isprimMenu() )
		{
			jumpOption            = ZCOMMAND ;
			currAppl->jumpEntered = true     ;
			p_poolMGR->put( RC, "ZVERB", "RETURN", SHARED ) ;
			currAppl->currPanel->cmd_setvalue( "" ) ;
			return ;
		}
		currAppl->currPanel->cmd_setvalue( ZCOMMAND ) ;
	}

	if ( PFCMD != "" ) { ZCOMMAND = PFCMD + " " + ZCOMMAND ; }

	p1 = pos( ";", ZCOMMAND ) ;
	if ( p1 > 0 )
	{
		commandStack = substr( ZCOMMAND, (p1+1) )    ;
		ZCOMMAND     = substr( ZCOMMAND, 1, (p1-1) ) ;
		CMDVerb      = upper( word( ZCOMMAND, 1 ) )  ;
		CMDRest      = subword( ZCOMMAND, 2 )        ;
	}

	CMDVerb = upper( word( ZCOMMAND, 1 ) ) ;
	CMDRest = subword( ZCOMMAND, 2 ) ;

	if ( CMDVerb == "RETRIEVE" )
	{
		commandStack = "" ;
		if ( retrieveBuffer.empty() ) return ;
		ZCOMMAND = "NOP" ;
		passthru = false ;
		currAppl->currPanel->cmd_setvalue( retrieveBuffer[ retPos ] ) ;
		currAppl->currPanel->cursor_to_field( RC, currAppl->currPanel->CMDfield, retrieveBuffer[ retPos ].size()+1 ) ;
		currAppl->currPanel->get_cursor( row, col ) ;
		currScrn->set_row_col( row, col ) ;
		if ( ++retPos >= retrieveBuffer.size() ) retPos = 0 ;
		return ;
	}
	retPos = 0 ;

	if ( CMDVerb == "" ) { return ; }

	if ( CMDVerb == "HELP")
	{
		if ( currAppl->currPanel->SMSG != "" && !currAppl->currPanel->showLMSG )
		{
			passthru = false ;
			currAppl->currPanel->showLMSG = true ;
			currAppl->currPanel->display_MSG() ;
			commandStack = ""   ;
			return ;
		}
		else
		{
			currAppl->currPanel->showLMSG = false ;
			currAppl->currPanel->SMSG     = ""    ;
			currAppl->currPanel->LMSG     = ""    ;
			ZPARM        = currAppl->get_help_member( row, col ) ;
			commandStack = ""    ;
			CMDRest      = ZPARM ;
		}
	}

	for ( x = 0 ; x < 8 ; x++ )
	{
		p_tableMGR->cmdsearch( RC, funcPOOL, currAppl->ZZAPPLID + "CMDS", CMDVerb ) ;
		if ( RC > 4 )
		{
			p_tableMGR->cmdsearch( RC, funcPOOL, "USRCMDS", CMDVerb ) ;
			if ( RC > 4 )
			{
				p_tableMGR->cmdsearch( RC, funcPOOL, "ISPCMDS", CMDVerb ) ;
			}
		}
		if ( RC == 0 && word( ZCTACT, 1 ) == "ALIAS" )
		{
			CMDVerb  = word( ZCTACT, 2 )    ;
			CMDRest  = subword( ZCTACT, 3 ) ;
			ZCOMMAND = subword( ZCTACT, 2 ) ;
		}
		else { break ; }
	}
	if ( x > 7 )
	{
		RC = 8 ;
		log( "E", "ALIAS dept cannot be greater than 8.  Terminating search" << endl ) ;
	}

	if ( RC == 0 )
	{
  		if ( word( ZCTACT, 1 ) == "NOP" )
		{
			passthru = false ;
			return ;
		}
		else if ( word( ZCTACT, 1 ) == "SELECT" )
		{
			selectParse( RC, subword( ZCTACT, 2) , SEL_PGM, SEL_PARM, SEL_NEWAPPL, SEL_NEWPOOL, SEL_PASSLIB) ;
			if ( RC > 0 )
			{
				log( "E", "Error in SELECT command " << ZCTACT << endl ) ;
				currAppl->setmsg( "PSYS01K" ) ;
				return ;
			}
			p1 = wordpos( "&ZPARM", SEL_PARM ) ;
			if ( p1 > 0 )
			{
				p1       = wordindex( SEL_PARM, p1 )       ;
				SEL_PARM = delstr( SEL_PARM, p1, 6 )       ;
				SEL_PARM = insert( CMDRest, SEL_PARM, p1 ) ;
			}
			if ( substr( SEL_PGM, 1, 1 ) == "&" )
			{
				currAppl->vcopy( substr( SEL_PGM, 2 ), SEL_PGM, MOVE ) ;
			}
			SEL      = true  ;
			passthru = false ;
		}
		else if ( ZCTACT  == "PASSTHRU" )
		{
			passthru = true ;
			ZCOMMAND = strip( CMDVerb + " " + CMDRest ) ;
		}
		else if ( ZCTACT  == "SETVERB" )
		{
			passthru = true ;
			p_poolMGR->put( RC, "ZVERB", ZCTVERB, SHARED ) ;
			if ( RC > 0 ) { log( "C", "VPUT for ZVERB failed" << endl ) ; }
			ZCOMMAND = subword( ZCOMMAND, 2 ) ;
		}
	}

	if ( (findword( CMDVerb, BuiltInCommands )) || (dlibs.find( CMDVerb ) != dlibs.end()) )
	{
		passthru = false   ;
		ZCOMMAND = CMDVerb ;
		ZPARM    = upper( CMDRest ) ;
	}

	if ( substr( CMDVerb, 1, 1 ) == ">" )
	{
		ZCOMMAND = substr( ZCOMMAND, 2 ) ;
	}

	if ( !passthru )
	{
		currAppl->currPanel->cmd_setvalue( "" ) ;
	}
	else
	{
		currAppl->currPanel->cmd_setvalue( ZCOMMAND ) ;
	}
	debug1( "Primary command >>" << ZCOMMAND << "<<  Passthru = " << passthru << endl ) ;
}


void processSELECT()
{
	int  elapsed ;
	uint row     ;
	uint col     ;

	if ( dlibs.find( currAppl->SEL_PGM ) != dlibs.end() )
	{
		updateDefaultVars() ;
		startApplication( currAppl->SEL_PGM, currAppl->SEL_PARM, currAppl->SEL_NEWAPPL, currAppl->SEL_NEWPOOL, currAppl->SEL_PASSLIB ) ;
	}
	else
	{
		errorScreen( 1, "SELECT function did not find application " + currAppl->SEL_PGM ) ;
		currAppl->SEL      = false ;
		currAppl->busyAppl = true  ;
		log( "W", "Resumed function did a SELECT.  Ending wait in SELECT" << endl ) ;
		cond_appl.notify_all() ;
		elapsed = 0 ;
		while ( currAppl->busyAppl )
		{
			elapsed++ ;
			boost::this_thread::sleep_for(boost::chrono::milliseconds(ZWAIT)) ;
			if ( currAppl->noTimeOut ) { elapsed = 0 ; }
			if ( elapsed > ZMAXWAIT  ) { currAppl->set_forced_abend() ; }
		}
		if ( currAppl->abnormalEnd )
		{
			currAppl->terminateAppl = true ;
			if ( currAppl->abnormalEndForced )
			{
				errorScreen( 2, "A forced termination of the subtask has occured" ) ;
			}
			else
			{
				errorScreen( 2, "An error has occured during application execution" ) ;
			}
		}
		while ( currAppl->terminateAppl )
		{
			debug1( "Calling application " << currAppl->ZAPPNAME << " also ending" << endl ) ;
			terminateApplication() ;
			if ( pLScreen::screensTotal == 0 ) return ;
		}
		currAppl->get_cursor( row, col )  ;
		currScrn->set_row_col( row, col ) ;
		currScrn->clear()   ;
		currAppl->refresh() ;
	}
}


void startApplication( string Application, string parm, string NEWAPPL, bool NEWPOOL, bool PASSLIB )
{
	int elapsed ;
	uint row    ;
	uint col    ;

	string s    ;
	string m    ;
	string p    ;
	string t    ;

	bool ldm    ;
	bool ldp    ;
	bool ldt    ;

	boost::thread * pThread ;

	if ( dlibs.find( Application ) == dlibs.end() )
	{
		errorScreen( 1, "Application " + Application + " not found" ) ;
		currAppl->get_cursor( row, col )  ;
		currScrn->set_row_col( row, col ) ;
		currScrn->clear()   ;
		currAppl->refresh() ;
		return ;
	}

	log( "I", "Starting new application " << Application << " with parameters '" << parm << "'" << endl ) ;
	if ( PASSLIB || !NEWPOOL )
	{
		debug1( "PASSLIB or no NEWPOOL specifed on start application.  Saving LIBDEF status" << endl ) ;
		m   = currAppl->ZMUSER ;
		p   = currAppl->ZPUSER ;
		t   = currAppl->ZTUSER ;
		ldm = currAppl->libdef_muser ;
		ldp = currAppl->libdef_puser ;
		ldt = currAppl->libdef_tuser ;
	}
	pLScreen::currScreen->clear() ;

	currAppl = ((pApplication*(*)())( maker_ep[ Application ]))() ;

	currScrn->application_add( currAppl ) ;
	currAppl->ZAPPNAME   = Application ;
	currAppl->taskID     = ++maxtaskID ;
	currAppl->PASSLIB    = PASSLIB     ;
	currAppl->busyAppl   = true        ;
	currAppl->p_poolMGR  = p_poolMGR   ;
	currAppl->p_tableMGR = p_tableMGR  ;
	currAppl->PARM       = parm        ;

	if ( NEWAPPL != "" )
	{
		if ( NEWAPPL != p_poolMGR->getAPPLID() )
		{
			p_poolMGR->setAPPLID( RC, NEWAPPL )  ;
		}
	}
	currAppl->ZZAPPLID = p_poolMGR->getAPPLID() ;

	p_poolMGR->createPool( RC, PROFILE, ZSPROF ) ;
	if ( NEWPOOL )
	{
		currAppl->NEWPOOL = true ;
		p_poolMGR->createPool( RC, SHARED ) ;
		s = ZSCREEN[ screenNum ] ;
		p_poolMGR->put( RC, "ZSCREEN", s, SHARED, SYSTEM ) ;
		p_poolMGR->put( RC, "ZAPPLID", p_poolMGR->getAPPLID(), SHARED, SYSTEM ) ;
	}
	currAppl->shrdPool = p_poolMGR->getshrdPool() ;
	loadpfkeyTable() ;
	currAppl->init() ;

	if ( PASSLIB || !NEWPOOL )
	{
		debug1( "PASSLIB or no NEWPOOL specifed on start application.  Restoring LIBDEF status to new application." << endl ) ;
		currAppl->ZMUSER = m ;
		currAppl->ZPUSER = p ;
		currAppl->ZTUSER = t ;
		currAppl->libdef_muser = ldm ;
		currAppl->libdef_puser = ldp ;
		currAppl->libdef_tuser = ldt ;
	}

	pThread = new boost::thread(boost::bind(&pApplication::application, currAppl ));

	currAppl->pThread = pThread ;

	log( "I", "Waiting for new application to complete startup.  ID=" << pThread->get_id() << endl ) ;
	elapsed = 0 ;
	while ( currAppl->busyAppl )
	{
		elapsed++ ;
		boost::this_thread::sleep_for(boost::chrono::milliseconds(ZWAIT)) ;
		if ( currAppl->noTimeOut ) { elapsed = 0 ; }
		if ( elapsed > ZMAXWAIT  ) { currAppl->set_forced_abend() ; }
	}

	log( "I", "New thread and application started and initialised. ID=" << pThread->get_id() << endl ) ;

	if ( currAppl->SEL )
	{
		debug1( "Application " << currAppl->ZAPPNAME << " has done a SELECT without a DISPLAY !!!! " << endl ) ;
		processSELECT() ;
	}

	if ( currAppl->abnormalEnd )
	{
		errorScreen( 2, "An error has occured initialising new task for " + Application ) ;
		terminateApplication() ;
		if ( pLScreen::screensTotal == 0 ) return ;
	}

	while ( currAppl->terminateAppl )
	{
		log( "I", "Application " << currAppl->ZAPPNAME << " has immediately terminated.  Cleaning up resources" << endl ) ;
		terminateApplication() ;
		if ( pLScreen::screensTotal == 0 ) return ;
		if ( currAppl->SEL && !currAppl->terminateAppl )
		{
			debug1( "Application " << currAppl->ZAPPNAME << " has done another SELECT without a DISPLAY (2) !!!! " << endl ) ;
			processSELECT() ;
		}
	}
	currAppl->get_cursor( row, col ) ;
	currScrn->set_row_col( row, col ) ;
	loadApplicationCommandTable( currAppl->ZZAPPLID ) ;
}


void terminateApplication()
{
	int  RC      ;
	int  tRC     ;
	int  tRSN    ;
	int  elapsed ;
	uint row     ;
	uint col     ;

	string  ZAPPNAME ;
	string  tRESULT  ;
	string  SMSG     ;
	string  LMSG     ;
	string  fname    ;
	cuaType MSGTYPE  ;

	bool MSGALRM      ;
	bool refList      ;
	bool propagateEnd ;
	bool jumpEntered  ;

	boost::thread * pThread ;

	log( "I", "Application terminating " << currAppl->ZAPPNAME << endl ) ;

	ZAPPNAME = currAppl->ZAPPNAME ;
	pLScreen * p_pLScreen         ;
	p_pLScreen = screenList[ screenNum ] ;

	if ( currAppl->NEWPOOL )
	{
		p_poolMGR->destroyPool( RC, SHARED ) ;
	}

	currAppl->closeTables()     ;
	SMSG    = ""                ;
	tRC     = currAppl->ZRC     ;
	tRSN    = currAppl->ZRSN    ;
	tRESULT = currAppl->ZRESULT ;

	if ( currAppl->field_name == "#REFLIST" ) { refList = true  ; }
	else                                      { refList = false ; }

	if ( currAppl->setMSG ) { SMSG = currAppl->ZSMSG ; LMSG = currAppl->ZLMSG ; MSGTYPE = currAppl->ZMSGTYPE ; MSGALRM = currAppl->ZMSGALRM ; }

	jumpEntered = currAppl->jumpEntered ;
	if ( currAppl->propagateEnd && ( currScrn->application_stack_size() > 1 ) ) { propagateEnd = true  ; }
	else                                                                        { propagateEnd = false ; }

	p_poolMGR->destroyPool( RC, PROFILE ) ;

	pThread = currAppl->pThread ;

	if ( currAppl->abnormalEnd )
	{
		pThread->detach() ;
	}

	if ( pLScreen::screensTotal == 1 &&  currScrn->application_stack_size() == 1 )
	{
		log( "I", "Closing ISPS profile and application log as last application program is terminating" << endl ) ;
		p_poolMGR->setAPPLID( RC, "ISPS" )    ;
		p_poolMGR->destroyPool( RC, PROFILE ) ;
		p_poolMGR->statistics()  ;
		p_tableMGR->statistics() ;
		currAppl->closeLog()     ;
	}

	log( "I", "Removing application instance of " << currAppl->ZAPPNAME << endl ) ;
	((void (*)(pApplication*))(destroyer_ep[ currAppl->ZAPPNAME ]))( currAppl )  ;

	delete pThread  ;

	currScrn->application_remove_current() ;
	if ( currScrn->application_stack_empty() )
	{
		delete p_pLScreen ;
		if ( pLScreen::screensTotal == 0 )
		{
			log( "I", "Application terminatation completed.  No more applications running - terminating lspf" << endl ) ;
			return ;
		}
		screenList.erase( screenList.begin() + screenNum ) ;
		if ( altScreen > screenNum ) --altScreen ;
		--screenNum ;
		if ( screenNum < 0 ) screenNum = pLScreen::screensTotal - 1 ;
		if ( pLScreen::screensTotal > 1 )
		{
			if ( altScreen == screenNum ) altScreen = ((altScreen == pLScreen::screensTotal - 1) ? 0 : (altScreen + 1) )  ;  
		}
		p_pLScreen = screenList[ screenNum ] ;
		pLScreen::currScreen = p_pLScreen ;
	}

	currAppl = currScrn->application_get_current() ;

	p_poolMGR->setAPPLID( RC, currAppl->ZZAPPLID )  ;
	if ( RC > 0 ) { log( "C", "ERROR setting APPLID for pool manager.  RC=" << RC << endl ) ; }
	p_poolMGR->setshrdPool( RC, currAppl->shrdPool )   ;
	if ( RC > 0 ) { log( "C", "ERROR setting shared pool for pool manager.  RC=" << RC << endl ) ; }

	p_poolMGR->put( RC, "ZPANELID", currAppl->PANELID, SHARED, SYSTEM )  ;

	if ( refList )
	{
		if ( currAppl->nretriev_on() )
		{
			fname = p_poolMGR->get( RC, "ZRFFLDA", SHARED ) ;
			if ( RC == 0 )
			{
				currAppl->field_name = fname ;
				if ( p_poolMGR->get( RC, "ZRFMOD", PROFILE ) == "BEX" )
				{
					commandStack = ";;" ;
				}
			}
			else
			{
				currAppl->setmsg( "PSYS01Y" ) ;
				SMSG = currAppl->ZSMSG ; LMSG = currAppl->ZLMSG ; MSGTYPE = currAppl->ZMSGTYPE ; MSGALRM = currAppl->ZMSGALRM ;
			}
		}
		else
		{
			currAppl->setmsg( "PSYS01X" ) ;
			SMSG = currAppl->ZSMSG ; LMSG = currAppl->ZLMSG ; MSGTYPE = currAppl->ZMSGTYPE ; MSGALRM = currAppl->ZMSGALRM ;
		}
	}

	if ( currAppl->field_name != "" )
	{
		if ( tRC == 0 )
		{
			if ( currAppl->currPanel->field_get_row_col( currAppl->field_name, row, col ) )
			{
				currAppl->currPanel->field_setvalue( currAppl->field_name, tRESULT ) ;
				currAppl->currPanel->cursor_eof( row, col ) ;
				currAppl->currPanel->set_cursor( row, col ) ;
			}
			else
			{
				log( "E", "Invalid field " << currAppl->field_name << " in variable ZRFFLDA" << endl ) ;
				currAppl->setmsg( "PSYS01Z" ) ;
				SMSG = currAppl->ZSMSG ; LMSG = currAppl->ZLMSG ; MSGTYPE = currAppl->ZMSGTYPE ; MSGALRM = currAppl->ZMSGALRM ;
			}
		}
		else if ( tRC == 8 )
		{
			beep() ;
		}
		currAppl->field_name = "" ;
	}

	loadpfkeyTable() ;

	if ( currAppl->isprimMenu() ) { propagateEnd = false ; }
	if ( jumpEntered && currAppl->isprimMenu() )
	{
		commandStack = jumpOption ;
	}

	if ( currAppl->SEL )
	{
		if ( propagateEnd )
		{
			log( "I", "RETURN entered.  Propagating END to next application in the SELECT nested dialogue" << endl ) ;
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
		currAppl->SEL      = false   ;
		currAppl->busyAppl = true    ;
		currAppl->ZRC      = tRC     ;
		currAppl->ZRSN     = tRSN    ;
		currAppl->ZRESULT  = tRESULT ;
		if ( SMSG != "" )  { currAppl->ZSMSG = SMSG ; currAppl->ZLMSG = LMSG ; currAppl->ZMSGTYPE = MSGTYPE ; currAppl->ZMSGALRM = MSGALRM ; currAppl->setMSG = true ; }
		log( "I", "Resumed function did a SELECT, BROWSE, EDIT or VIEW.  Ending wait in function" << endl ) ;
		cond_appl.notify_all() ;
		elapsed = 0 ;
		while ( currAppl->busyAppl )
		{
			elapsed++ ;
			boost::this_thread::sleep_for(boost::chrono::milliseconds(ZWAIT)) ;
			if ( currAppl->noTimeOut ) { elapsed = 0 ; }
			if ( elapsed > ZMAXWAIT  ) { currAppl->set_forced_abend() ; }
		}
		while ( currAppl->terminateAppl )
		{
			debug1( "Calling application " << currAppl->ZAPPNAME << " also ending" << endl ) ;
			terminateApplication() ;
			if ( pLScreen::screensTotal == 0 ) return ;
		}
		currAppl->get_cursor( row, col ) ;
	}
	else
	{
		if ( jumpEntered && propagateEnd )
		{
			debug1( "JUMP entered.  Propagating RETURN to next application on the stack outside a SELECT nested dialogue" << endl ) ;
			p_poolMGR->put( RC, "ZVERB", "RETURN", SHARED ) ;
			currAppl->jumpEntered = true ;
			currAppl->busyAppl    = true ;
			cond_appl.notify_all()       ;
			elapsed = 0 ;
			while ( currAppl->busyAppl )
			{
				elapsed++ ;
				boost::this_thread::sleep_for(boost::chrono::milliseconds(ZWAIT)) ;
				if ( currAppl->noTimeOut ) { elapsed = 0 ; }
				if ( elapsed > ZMAXWAIT  ) { currAppl->set_forced_abend() ; }
			}
		}
		while ( currAppl->terminateAppl )
		{
			debug1( "Previous application " << currAppl->ZAPPNAME << " also ending" << endl ) ;
			terminateApplication() ;
			if ( pLScreen::screensTotal == 0 ) return ;
		}
		if ( SMSG != "" )  { currAppl->set_msg( SMSG, LMSG, MSGTYPE, MSGALRM ) ; }
		currScrn->clear()   ;
		currAppl->refresh() ;
		currAppl->get_home( row, col ) ;
		currAppl->setMSG = false       ;
	}
	log( "I", "Application terminatation of " << ZAPPNAME << " completed.  Current application is " << currAppl->ZAPPNAME << endl ) ;
	currScrn->set_row_col( row, col ) ;
}


void loadcuaTables()
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

	setColourPair( "DT" ) ;
	setColourPair( "ET" ) ;
	setColourPair( "EE" ) ;
	setColourPair( "FP" ) ;
	setColourPair( "FK")  ;

	setColourPair( "IMT" ) ;
	setColourPair( "LEF" ) ;
	setColourPair( "LID" ) ;
	setColourPair( "LI" )  ;

	setColourPair( "NEF" ) ;
	setColourPair( "NT" )  ;

	setColourPair( "PI" )  ;
	setColourPair( "PIN" ) ;
	setColourPair( "PT" )  ;

	setColourPair( "PS")   ;
	setColourPair( "PAC" ) ;
	setColourPair( "PUC" ) ;

	setColourPair( "RP" )  ;

	setColourPair( "SI" )  ;
	setColourPair( "SAC" ) ;
	setColourPair( "SUC")  ;

	setColourPair( "VOI" )  ;
	setColourPair( "WMT" )  ;
	setColourPair( "WT" )   ;
	setColourPair( "WASL" ) ;

	cuaAttr[ CHAR   ] = BLUE   | A_BOLD   | A_PROTECT ;
	cuaAttr[ DATAIN ] = GREEN  | A_NORMAL ;
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

	cuaAttrProt[ IMT    ] = true ;
	cuaAttrProt[ LEF    ] = true ;
	cuaAttrProt[ LID    ] = true ;
	cuaAttrProt[ LI     ] = true ;

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

	usrAttr[ N_RED     ] = RED     ;	usrAttr[ B_RED     ] = RED     | A_BOLD ;	usrAttr[ R_RED     ] = RED     | A_REVERSE ;	usrAttr[ U_RED     ] = RED     | A_UNDERLINE ;
	usrAttr[ N_GREEN   ] = GREEN   ;	usrAttr[ B_GREEN   ] = GREEN   | A_BOLD ;	usrAttr[ R_GREEN   ] = GREEN   | A_REVERSE ;	usrAttr[ U_GREEN   ] = GREEN   | A_UNDERLINE ;
	usrAttr[ N_YELLOW  ] = YELLOW  ;	usrAttr[ B_YELLOW  ] = YELLOW  | A_BOLD ;	usrAttr[ R_YELLOW  ] = YELLOW  | A_REVERSE ;	usrAttr[ U_YELLOW  ] = YELLOW  | A_UNDERLINE ;
	usrAttr[ N_BLUE    ] = BLUE    ;	usrAttr[ B_BLUE    ] = BLUE    | A_BOLD ;	usrAttr[ R_BLUE    ] = BLUE    | A_REVERSE ;	usrAttr[ U_BLUE    ] = BLUE    | A_UNDERLINE ;
	usrAttr[ N_MAGENTA ] = MAGENTA ;	usrAttr[ B_MAGENTA ] = MAGENTA | A_BOLD ;	usrAttr[ R_MAGENTA ] = MAGENTA | A_REVERSE ;	usrAttr[ U_MAGENTA ] = MAGENTA | A_UNDERLINE ;
	usrAttr[ N_TURQ    ] = TURQ    ;	usrAttr[ B_TURQ    ] = TURQ    | A_BOLD ;	usrAttr[ R_TURQ    ] = TURQ    | A_REVERSE ;	usrAttr[ U_TURQ    ] = TURQ    | A_UNDERLINE ;
	usrAttr[ N_WHITE   ] = WHITE   ;	usrAttr[ B_WHITE   ] = WHITE   | A_BOLD ;	usrAttr[ R_WHITE   ] = WHITE   | A_REVERSE ;	usrAttr[ U_WHITE   ] = WHITE   | A_UNDERLINE ;

	usrAttr[ P_RED     ] = RED     | A_PROTECT ;
	usrAttr[ P_GREEN   ] = GREEN   | A_PROTECT ;
	usrAttr[ P_YELLOW  ] = YELLOW  | A_PROTECT ;
	usrAttr[ P_BLUE    ] = BLUE    | A_PROTECT ;
	usrAttr[ P_MAGENTA ] = MAGENTA | A_PROTECT ;
	usrAttr[ P_TURQ    ] = TURQ    | A_PROTECT ;
	usrAttr[ P_WHITE   ] = WHITE   | A_PROTECT ;
}


void setColourPair( string name )
{
	string t ;
	char   c ;

	t = p_poolMGR->get( RC, "ZC" + name, PROFILE ) ;
	if ( RC == 0 )
	{
		c = t[ 0 ] ;
		if      ( c == 'R' ) { cuaAttr[ cuaAttrName[ name ] ] = RED     ; }
		else if ( c == 'G' ) { cuaAttr[ cuaAttrName[ name ] ] = GREEN   ; }
		else if ( c == 'Y' ) { cuaAttr[ cuaAttrName[ name ] ] = YELLOW  ; }
		else if ( c == 'B' ) { cuaAttr[ cuaAttrName[ name ] ] = BLUE    ; }
		else if ( c == 'M' ) { cuaAttr[ cuaAttrName[ name ] ] = MAGENTA ; }
		else if ( c == 'T' ) { cuaAttr[ cuaAttrName[ name ] ] = TURQ    ; }
		else if ( c == 'W' ) { cuaAttr[ cuaAttrName[ name ] ] = WHITE   ; }
		else { log( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ; RC = 20 ; }
		c = t[ 1 ] ;
		if      ( c == 'L' ) { cuaAttr[ cuaAttrName[ name ] ] = cuaAttr[ cuaAttrName[ name ] ] | A_NORMAL  ; }
		else if ( c == 'H' ) { cuaAttr[ cuaAttrName[ name ] ] = cuaAttr[ cuaAttrName[ name ] ] | A_BOLD    ; }
		else { log( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ; RC = 20 ; }
		c = t[ 2 ] ;
		if      ( c == 'N' ) { }
		else if ( c == 'B' ) { cuaAttr[ cuaAttrName[ name ] ] = cuaAttr[ cuaAttrName[ name ] ] | A_BLINK     ; }
		else if ( c == 'R' ) { cuaAttr[ cuaAttrName[ name ] ] = cuaAttr[ cuaAttrName[ name ] ] | A_REVERSE   ; }
		else if ( c == 'U' ) { cuaAttr[ cuaAttrName[ name ] ] = cuaAttr[ cuaAttrName[ name ] ] | A_UNDERLINE ; }
		else { log( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ; RC = 20 ; }
	}
	else { log( "E", "Variable ZC" << name << " not found in ISPS profile" << endl ) ; RC = 20 ; }
	if ( RC > 0 )
	{
		delete screenList[ 0 ] ;
		log( "C", "Rerun setup program to re-initialise ISPS profile" << endl ) ;
		cout << "Aborting startup of lspf.  Check lspf and application logs for errors " << endl ;
		splog.close() ;
		abort() ;
	}
}


void loadDefaultPools()
{
	// Default vars go in @DEFPROF (RO) for PROFILE and @DEFSHAR (UP) for SHARE

	int    RC ;
	int    i  ;

	struct utsname buf ;

	i = uname( &buf ) ;

	p_poolMGR->defaultVARs(  RC , "ZSCRMAXD", d2ds( pLScreen::maxrow ), SHARED ) ;
	p_poolMGR->defaultVARs(  RC , "ZSCRMAXW", d2ds( pLScreen::maxcol ), SHARED ) ;
	p_poolMGR->defaultVARs(  RC , "ZUSER", ZUSER, SHARED )   ;

	p_poolMGR->setAPPLID( RC, "ISPS" ) ;
	p_poolMGR->createPool( RC, PROFILE, ZSPROF ) ;
	if ( RC == 0 )
	{
		log( "I", "Loaded system profile ISPSPROF" << endl ) ;
	}
	else
	{
		delete screenList[ 0 ] ;
		log( "C", "Loading of system profile ISPSPROF failed.  RC=" << RC << "  Aborting startup" << endl ) ;
		log( "C", "Check path " << ZSPROF << endl ) ;
		cout << "Aborting startup of lspf.  Check lspf and application logs for errors " << endl ;
		splog.close() ;
		abort() ;
	}

	p_poolMGR->createPool( RC, SHARED )   ;

	ZTLIB = p_poolMGR->get( RC, "ZTLIB", PROFILE ) ;

	p_poolMGR->setAPPLID( RC, "ISP" ) ;
	p_poolMGR->createPool( RC, PROFILE, ZSPROF ) ;

	p_poolMGR->defaultVARs( RC , "ZHOME", ZHOME, SHARED )             ;
	p_poolMGR->defaultVARs( RC , "ZSHELL", ZSHELL, SHARED )           ;
	p_poolMGR->defaultVARs( RC , "ZSYSNAME", buf.sysname, SHARED )    ;
	p_poolMGR->defaultVARs( RC , "ZNODNAME", buf.nodename, SHARED )   ;
	p_poolMGR->defaultVARs( RC , "ZOSREL", buf.release, SHARED )      ;
	p_poolMGR->defaultVARs( RC , "ZOSVER", buf.version, SHARED )      ;
	p_poolMGR->defaultVARs( RC , "ZMACHINE", buf.machine, SHARED )    ;
	p_poolMGR->defaultVARs( RC , "ZENVIR", "lspf V0R0M1", SHARED )    ;
	p_poolMGR->defaultVARs( RC , "ZDATEF", "DD/MM/YY", SHARED )       ;
	p_poolMGR->defaultVARs( RC , "ZDATEFL", "DD/MM/YYYY", SHARED )    ;
	p_poolMGR->defaultVARs( RC , "ZSCALE", "OFF", SHARED )            ;
	p_poolMGR->defaultVARs( RC , "ZSPLIT", "NO", SHARED )             ;

	p_poolMGR->setPOOLsreadOnly( RC ) ;
}


void loadCommandTable()
{
	int RC ;

	p_tableMGR->loadTable( RC, 0, "ISPCMDS", SHARE, ZTLIB ) ;
	if ( RC == 0 )
	{
		log( "I", "Loaded system command table ISPCMDS" << endl ) ;
	}
	else
	{
		delete screenList[ 0 ] ;
		log( "C", "Loading of system command table ISPCMDS failed.  RC=" << RC << "  Aborting startup" << endl ) ;
		log( "C", "Check path " + ZTLIB << endl ) ;
		cout << "Aborting startup of lspf.  Check lspf and application logs for errors " << endl ;
		splog.close() ;
		abort() ;
	}

	p_tableMGR->loadTable( RC, 0, "USRCMDS", SHARE, ZTLIB ) ;
	debug1( "Loading user command table.  RC=" << RC << endl ) ;
}


void loadApplicationCommandTable( string APPLID )
{
	int RC ;

	if ( APPLID == "ISP" || APPLID == "ISPS" ) return ;

	if ( !p_tableMGR->isloaded( APPLID + "CMDS" ) )
	{
		if ( p_tableMGR->tablexists( APPLID + "CMDS", ZTLIB ) )
		{
			p_tableMGR->loadTable( RC, 0, APPLID + "CMDS", SHARE, ZTLIB ) ;
			debug1( "Loading application command table " << APPLID << "CMDS.  RC=" << RC << endl ) ;
		}
		else
		{
			debug1( "Cannot load application command table " << APPLID << "CMDS.  File does not exist in search path" << endl ) ;
		}
	}
	else
	{
		debug1( "Application command table " << APPLID << "CMDS already loaded" << endl ) ;
	}
}


void loadpfkeyTable()
{
	string ZPF01, ZPF02, ZPF03, ZPF04, ZPF05, ZPF06, ZPF07, ZPF08, ZPF09, ZPF10, ZPF11, ZPF12 ;
	string ZPF13, ZPF14, ZPF15, ZPF16, ZPF17, ZPF18, ZPF19, ZPF20, ZPF21, ZPF22, ZPF23, ZPF24 ;

	ZPF01 = p_poolMGR->get( RC, "ZPF01", PROFILE ) ; if ( RC == 8 ) { ZPF01 = "HELP" ; }     ; pfkeyTable[ KEY_F(1)  ] = ZPF01  ;
	ZPF02 = p_poolMGR->get( RC, "ZPF02", PROFILE ) ; if ( RC == 8 ) { ZPF02 = "SPLIT" ; }    ; pfkeyTable[ KEY_F(2)  ] = ZPF02  ;
	ZPF03 = p_poolMGR->get( RC, "ZPF03", PROFILE ) ; if ( RC == 8 ) { ZPF03 = "END"  ; }     ; pfkeyTable[ KEY_F(3)  ] = ZPF03  ;
	ZPF04 = p_poolMGR->get( RC, "ZPF04", PROFILE ) ; if ( RC == 8 ) { ZPF04 = "RETURN" ; }   ; pfkeyTable[ KEY_F(4)  ] = ZPF04  ;
	ZPF05 = p_poolMGR->get( RC, "ZPF05", PROFILE ) ; if ( RC == 8 ) { ZPF05 = "RFIND" ; }    ; pfkeyTable[ KEY_F(5)  ] = ZPF05  ;
	ZPF06 = p_poolMGR->get( RC, "ZPF06", PROFILE ) ; if ( RC == 8 ) { ZPF06 = "RCHANGE" ;}   ; pfkeyTable[ KEY_F(6)  ] = ZPF06  ;
	ZPF07 = p_poolMGR->get( RC, "ZPF07", PROFILE ) ; if ( RC == 8 ) { ZPF07 = "UP" ; }       ; pfkeyTable[ KEY_F(7)  ] = ZPF07  ;
	ZPF08 = p_poolMGR->get( RC, "ZPF08", PROFILE ) ; if ( RC == 8 ) { ZPF08 = "DOWN" ; }     ; pfkeyTable[ KEY_F(8)  ] = ZPF08  ;
	ZPF09 = p_poolMGR->get( RC, "ZPF09", PROFILE ) ; if ( RC == 8 ) { ZPF09 = "SWAP" ; }     ; pfkeyTable[ KEY_F(9)  ] = ZPF09  ;
	ZPF10 = p_poolMGR->get( RC, "ZPF10", PROFILE ) ; if ( RC == 8 ) { ZPF10 = "LEFT" ; }     ; pfkeyTable[ KEY_F(10) ] = ZPF10  ;
	ZPF11 = p_poolMGR->get( RC, "ZPF11", PROFILE ) ; if ( RC == 8 ) { ZPF11 = "RIGHT" ; }    ; pfkeyTable[ KEY_F(11) ] = ZPF11  ;
	ZPF12 = p_poolMGR->get( RC, "ZPF12", PROFILE ) ; if ( RC == 8 ) { ZPF12 = "RETRIEVE";}   ; pfkeyTable[ KEY_F(12) ] = ZPF12  ;
	ZPF13 = p_poolMGR->get( RC, "ZPF13", PROFILE ) ; if ( RC == 8 ) { ZPF13 = "HELP" ; }     ; pfkeyTable[ KEY_F(13) ] = ZPF13  ;
	ZPF14 = p_poolMGR->get( RC, "ZPF14", PROFILE ) ; if ( RC == 8 ) { ZPF14 = "SPLIT" ; }    ; pfkeyTable[ KEY_F(14) ] = ZPF14  ;
	ZPF15 = p_poolMGR->get( RC, "ZPF15", PROFILE ) ; if ( RC == 8 ) { ZPF15 = "END" ; }      ; pfkeyTable[ KEY_F(15) ] = ZPF15  ;
	ZPF16 = p_poolMGR->get( RC, "ZPF16", PROFILE ) ; if ( RC == 8 ) { ZPF16 = "RETURN" ; }   ; pfkeyTable[ KEY_F(16) ] = ZPF16  ;
	ZPF17 = p_poolMGR->get( RC, "ZPF17", PROFILE ) ; if ( RC == 8 ) { ZPF17 = "RFIND" ; }    ; pfkeyTable[ KEY_F(17) ] = ZPF17  ;
	ZPF18 = p_poolMGR->get( RC, "ZPF18", PROFILE ) ; if ( RC == 8 ) { ZPF18 = "RCHANGE" ; }  ; pfkeyTable[ KEY_F(18) ] = ZPF18  ;
	ZPF19 = p_poolMGR->get( RC, "ZPF19", PROFILE ) ; if ( RC == 8 ) { ZPF19 = "UP" ; }       ; pfkeyTable[ KEY_F(19) ] = ZPF19  ;
	ZPF20 = p_poolMGR->get( RC, "ZPF20", PROFILE ) ; if ( RC == 8 ) { ZPF20 = "DOWN" ; }     ; pfkeyTable[ KEY_F(20) ] = ZPF20  ;
	ZPF21 = p_poolMGR->get( RC, "ZPF21", PROFILE ) ; if ( RC == 8 ) { ZPF21 = "SWAP" ; }     ; pfkeyTable[ KEY_F(21) ] = ZPF21  ;
	ZPF22 = p_poolMGR->get( RC, "ZPF22", PROFILE ) ; if ( RC == 8 ) { ZPF22 = "SWAP PREV"; } ; pfkeyTable[ KEY_F(22) ] = ZPF22  ;
	ZPF23 = p_poolMGR->get( RC, "ZPF23", PROFILE ) ; if ( RC == 8 ) { ZPF23 = "SWAP NEXT"; } ; pfkeyTable[ KEY_F(23) ] = ZPF23  ;
	ZPF24 = p_poolMGR->get( RC, "ZPF24", PROFILE ) ; if ( RC == 8 ) { ZPF24 = "HELP" ; }     ; pfkeyTable[ KEY_F(24) ] = ZPF24  ;
}


void updateDefaultVars()
{
	int RC ;

	time_t rawtime        ;
	struct tm * time_info ;
	char   buf[ 11 ]      ;

	time( &rawtime ) ;
	time_info = localtime( &rawtime ) ;

	strftime( buf, sizeof(buf), "%H:%M:%S", time_info )   ; buf[ 8  ]  = 0x00 ; p_poolMGR->defaultVARs( RC , "ZTIMEL", buf, SHARED )   ;
	strftime( buf, sizeof(buf), "%H:%M", time_info )      ; buf[ 6  ]  = 0x00 ; p_poolMGR->defaultVARs( RC , "ZTIME", buf, SHARED )    ;
	strftime( buf, sizeof(buf), "%d/%m/%y", time_info )   ; buf[ 8  ]  = 0x00 ; p_poolMGR->defaultVARs( RC , "ZDATE", buf, SHARED )    ;
	strftime( buf, sizeof(buf), "%d", time_info )         ; buf[ 2  ]  = 0x00 ; p_poolMGR->defaultVARs( RC , "ZDAY", buf, SHARED )     ;
	strftime( buf, sizeof(buf), "%A       ", time_info )  ; buf[ 9  ]  = 0x00 ; p_poolMGR->defaultVARs( RC , "ZDAYOFWK", buf, SHARED ) ;
	strftime( buf, sizeof(buf), "%d/%m/%Y  ", time_info ) ; buf[ 10 ]  = 0x00 ; p_poolMGR->defaultVARs( RC , "ZDATEL", buf, SHARED )   ;
	strftime( buf, sizeof(buf), "%y.%j ", time_info )     ; buf[ 6  ]  = 0x00 ; p_poolMGR->defaultVARs( RC , "ZJDATE", buf, SHARED )   ;
	strftime( buf, sizeof(buf), "%Y.%j   ", time_info )   ; buf[ 8  ]  = 0x00 ; p_poolMGR->defaultVARs( RC , "Z4JDATE", buf, SHARED )  ;
	if ( pLScreen::screensTotal > 1 )
	{
		p_poolMGR->defaultVARs( RC , "ZSPLIT", "YES", SHARED ) ;
	}
	else
	{
		p_poolMGR->defaultVARs( RC , "ZSPLIT", "NO", SHARED ) ;
	}
		
}


void updateReflist()
{
	// Check if ZRFFLDA is set in the SHARED pool (usually put in the panel definition) and if so, add file to the reflist using
	// application ZRFLPGM, parmameters PLA plus the field entry value.

	// Don't update REFLIST if the application has done a CONTROL REFLIST NOUPDATE (flag ControlRefUpdate=false) or ISPS PROFILE variable
	// ZRFURL is not set to YES

	int RC ;

	uint row ;
	uint col ;

	string fname ;

	if ( currAppl->ControlRefUpdate && p_poolMGR->get( RC, "ZRFURL", PROFILE ) == "YES" )
	{
		fname = p_poolMGR->get( RC, "ZRFFLDA", SHARED ) ;
		if ( fname != "" )
		{
			if ( currAppl->currPanel->field_get_row_col( fname, row, col ) )
			{
				startApplication( p_poolMGR->get( RC, "ZRFLPGM", PROFILE ), "PLA " + currAppl->currPanel->field_getvalue( fname ), "", false, false ) ;
			}
		}
	}
}


void threadErrorHandler()
{
	log( "E", "An exception has occured in an application thread.  See application log for details.  Task ending" << endl ) ;
	try 
	{
		currAppl->abendexc() ;
	}
	catch (...)
	{
		log( "E", "An abend has occured during abend processing.  Calling abend() only to terminate application" << endl ) ;
		currAppl->abend() ;
	}
}


void errorScreen( int etype, string msg )
{
	int l(0) ;

        if ( currAppl->errPanelissued ) { return ; }
	log( "E", msg << endl ) ;
	currScrn->clear() ;
	mvaddstr( l++, 0, msg.c_str() ) ;
	mvaddstr( l++, 0, "See lspf and application logs for possible further details of the error" ) ;
	if ( etype == 2 )
	{
		mvaddstr( l++, 0, "Depending on the error, application may still be running in the background.  Recommend restarting lspf." ) ;
	}
	mvaddstr( l++, 0, "***" ) ;
	while ( true )
	{
		int  c  = getch() ;
		if ( c == 13 ) { return ; }
	}
}


void loadDynamicClasses()
{
	// Load modules of the form libABCDE.so from ZLDPATH concatination with name ABCDE
	// Duplicates are ignored with a warning messasge.
	// Terminate lspf if no modules found as we cannot continue

	int i        ;
	int j        ;
	int pos1     ;

	string appl  ;
	string mod   ;
	string fname ;
	string paths ;
	string p     ;

	void *dlib   ;
	void *mkr    ;
	void *destr  ;

	typedef vector<path> vec ;
	vec v        ;
	const char* dlsym_err ;

	vec::const_iterator it ;

	string e1( "** Module not loaded due to error(s).  Check lspf log **" ) ;
	string e2( "** No applications loaded.  Check ZLDPATH is correct.  lspf terminating **" ) ;

	paths = p_poolMGR->get( RC, "ZLDPATH", PROFILE ) ;
	j     = getpaths( paths ) ;
	for ( i = 1 ; i <= j ; i++ )
	{
		p = getpath( paths, i ) ;
		if ( is_directory( p ) )
		{
			log( "I", "Searching directory " << p << " for application classes" << endl ) ;
			copy( directory_iterator( p ), directory_iterator(), back_inserter( v ) ) ;
		}
		else
		{
			log( "W", "Ignoring directory " << p << "  Not found or not a directory." << endl ) ;
		}
	}

	i = 0 ;
	for ( it = v.begin() ; it != v.end() ; ++it )
	{
		fname = (*it).string() ;
		p     = substr( fname, 1, (lastpos( "/", fname ) - 1) ) ;
		mod   = substr( fname, (lastpos( "/", fname ) + 1) )    ;
		pos1  = pos( ".so", mod ) ;
		if ( substr(mod, 1, 3 ) != "lib" || pos1 == 0 ) continue ;
		appl  = substr( mod, 4, (pos1 - 4) ) ;
		log( "I", "Loading application " << appl << endl ) ;
		if ( dlibs.find( appl ) != dlibs.end() )
		{
			log( "W", "Ignoring duplicate module " + mod + " found in " + p << endl ) ;
			continue ;
		}

		i++          ;
		dlerror()    ;
		dlib = dlopen( fname.c_str(), RTLD_NOW ) ;
		p_poolMGR->defaultVARs( RC , "ZDLM" + right( d2ds( i ), 4, '0'), mod, SHARED )  ;
		p_poolMGR->defaultVARs( RC , "ZDLC" + right( d2ds( i ), 4, '0'), appl, SHARED ) ;
		if ( !dlib )
		{
			log( "E", "Error loading " << *it  << endl ) ;
			log( "E", "Error is " << dlerror() << endl ) ;
			log( "E", "Module " << mod << " will be ignored" << endl ) ;
			p_poolMGR->defaultVARs( RC , "ZDLP" + right( d2ds( i ), 4, '0'), e1, SHARED ) ;
			continue ;
		}
		dlerror() ;
		mkr       = dlsym( dlib, "maker" ) ;
		dlsym_err = dlerror() ;
		if ( dlsym_err )
		{
			log( "E", "Error loading symbol maker" << endl ) ;
			log( "E", "Error is " << dlerror()  << endl )    ;
			log( "E", "Module " << mod << " will be ignored" << endl ) ;
			p_poolMGR->defaultVARs( RC , "ZDLP" + right( d2ds( i ), 4, '0'), e1, SHARED ) ;
			continue ;
		}
		dlerror() ;
		destr     = dlsym( dlib, "destroy" ) ;
		dlsym_err = dlerror() ;
		if ( dlsym_err )
		{
			log( "E", "Error loading symbol destroy" << endl ) ;
			log( "E", "Error is " << dlsym_err  << endl )      ;
			log( "E", "Module " << mod << " will be ignored" << endl ) ;
			p_poolMGR->defaultVARs( RC , "ZDLP" + right( d2ds( i ), 4, '0'), e1, SHARED ) ;
			continue ;
		}
		debug1( *it << " loaded at " << dlib << endl ) ;
		debug1( "Maker at " << mkr << endl ) ;
		debug1( "Destroyer at " << destr << endl ) ;
		log( "I", "Loaded " << appl << " (module " << mod << ") from " << p << endl ) ;
		dlibs[ appl ]        = dlib  ;
		maker_ep[ appl ]     = mkr   ;
		destroyer_ep[ appl ] = destr ;
		p_poolMGR->defaultVARs( RC , "ZDLP" + right( d2ds( i ), 4, '0'), p, SHARED )    ;
	}
	if ( dlibs.size() > 0 )
	{
		log( "I", "" << dlibs.size() << " applications loaded" << endl ) ;
	}
	else
	{
		delete screenList[ 0 ] ;
		log( "C", e2 << endl ) ;
		cout <<  e2 << endl ;
		splog.close()     ;
		abort()           ;
	}
}
