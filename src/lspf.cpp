/* Compile with ::                                                                                                                                           */
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

map<cuaType, unsigned int> cuaAttr ;

#include "classes.h"
#include "classes.cpp"

#include "colours.h"

#include "pVPOOL.h"
#include "pVPOOL.cpp"

#include "pWidgets.h"
#include "pWidgets.cpp"

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

map<int, string> pfKeyDefaults = { {  1, "HELP"      },
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
#define MOD_NAME lspf

#define currScrn pLScreen::currScreen
#define OIA      pLScreen::OIA

#define CTRL(c) ((c) & 037)

using namespace std ;
using namespace boost::filesystem ;

int pLScreen::screensTotal = 0 ;
int pLScreen::maxScreenId  = 0 ;
int pLScreen::maxrow       = 0 ;
int pLScreen::maxcol       = 0 ;

WINDOW * pLScreen::OIA     = NULL ;

pLScreen     * pLScreen::currScreen = NULL ;
pApplication * currAppl ;

poolMGR  * p_poolMGR  = new poolMGR  ;
tableMGR * p_tableMGR = new tableMGR ;
logger   * lg         = new logger   ;
logger   * lgx        = new logger   ;

tableMGR * pApplication::p_tableMGR = NULL ;
poolMGR  * pApplication::p_poolMGR  = NULL ;
logger   * pApplication::lg         = NULL ;
poolMGR  * pPanel::p_poolMGR        = NULL ;
poolMGR  * abc::p_poolMGR           = NULL ;
logger   * pPanel::lg               = NULL ;
logger   * tableMGR::lg             = NULL ;
logger   * poolMGR::lg              = NULL ;

fPOOL funcPOOL ;

void setGlobalClassVars() ;
void initialSetup()       ;
void loadDefaultPools()   ;
void getDynamicClasses()  ;
bool loadDynamicClass( const string& ) ;
bool unloadDynamicClass( void * )   ;
void reloadDynamicClasses( string ) ;
void loadSystemCommandTable() ;
void loadCUATables()          ;
void setGlobalColours()       ;
void setColourPair( const string& ) ;
void updateDefaultVars()      ;
void updateReflist()          ;
void startApplication( selobj, bool =false ) ;
void startApplicationBack( selobj, bool =false ) ;
void terminateApplication()     ;
void abnormalTermMessage()      ;
void processBackgroundTasks()   ;
void ResumeApplicationAndWait() ;
bool createLogicalScreen()      ;
void deleteLogicalScreen()      ;
void processPGMSelect()         ;
void processZSEL()              ;
void processAction( uint row, uint col, int c, bool& passthru ) ;
void issueMessage( const string& ) ;
void lineOutput()         ;
void lineOutput_end()     ;
void threadErrorHandler() ;
void errorScreen( int, const string& )   ;
void abortStartup()                      ;
void lspfCallbackHandler( lspfCommand& ) ;
void createpfKeyDefaults()  ;
string pfKeyValue( int )    ;
string ControlKeyAction( char c ) ;
string listLogicalScreens() ;
void actionSwap( uint&, uint& )   ;
void actionTabKey( uint&, uint& ) ;
void displayNextPullDown( uint&, uint& ) ;
void executeFieldCommand( const string&, const fieldExc&, uint ) ;
void listBackTasks()        ;
void autoUpdate()           ;
bool resolveZCTEntry( string&, string& ) ;
bool isActionKey( int c )   ;
void listRetrieveBuffer()   ;
int  getScreenNameNum( const string& ) ;
void serviceCallError( errblock& )     ;
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

boost::circular_buffer<string> retrieveBuffer( 20 ) ;

int    linePosn  = -1 ;
int    maxtaskID = 0  ;
int    screenNum = 0  ;
int    altScreen = 0  ;
string ctlAction      ;
string commandStack   ;
string jumpOption     ;
bool   pfkeyPressed   ;
bool   ctlkeyPressed  ;
bool   wmPending      ;
char   lspfStatus     ;
char   backStatus     ;

vector<pApplication *> pApplicationBackground ;
vector<pApplication *> pApplicationTimeout    ;

errblock err    ;

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

const string BuiltInCommands = "ACTIONS DISCARD FIELDEXC MSGID NOP PANELID REFRESH RESIZE RETP SCRNAME SPLIT SWAP TDOWN USERID" ;
const string SystemCommands  = ".ABEND .AUTO .HIDE .INFO .LOAD .RELOAD .SCALE .SHELL .SHOW .SNAP .STATS .TASKS .TEST" ;

boost::recursive_mutex mtx ;

int main( void )
{
	int  elapsed ;
	uint row     ;
	uint col     ;

	setGlobalClassVars() ;

	lg->open()  ;
	lgx->open() ;

	err.clear() ;

	map<string, appInfo>::iterator it ;

	boost::thread * pThread              ;
	boost::thread * bThread              ;
	set_terminate ( threadErrorHandler ) ;

	commandStack = ""    ;
	wmPending    = false ;
	lspfStatus   = 'R'   ;

	screenList.push_back( new pLScreen ) ;
	currScrn->OIA_startTime( boost::posix_time::microsec_clock::universal_time() ) ;

	llog( "I", "lspf startup in progress" << endl ) ;
	llog( "I", "Starting background job monitor task" << endl ) ;
	bThread = new boost::thread( &processBackgroundTasks ) ;

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
	currAppl->taskid( ++maxtaskID )   ;
	currAppl->set_appname( GMAINPGM ) ;
	currAppl->shrdPool = 1          ;
	currAppl->NEWPOOL  = true       ;
	currAppl->setSelectPanel()      ;
	currAppl->lspfCallback = lspfCallbackHandler ;
	err.settask( currAppl->taskid() ) ;

	p_poolMGR->createProfilePool( err, "ISP", ZSPROF ) ;
	p_poolMGR->connect( currAppl->taskid(), "ISP", 1 ) ;
	if ( err.RC4() ) { createpfKeyDefaults() ; }

	p_poolMGR->put( err, "ZSCREEN", string( 1, ZSCREEN[ screenNum ] ), SHARED ) ;
	p_poolMGR->put( err, "ZSCRNUM", d2ds( currScrn->screenId ), SHARED ) ;
	p_poolMGR->put( err, "ZAPPLID", "ISP", SHARED ) ;
	currAppl->init() ;

	pThread = new boost::thread( boost::bind( &pApplication::application, currAppl ) ) ;
	currAppl->pThread = pThread ;
	apps[ GMAINPGM ].refCount++ ;
	apps[ GMAINPGM ].mainpgm = true ;

	llog( "I", "Waiting for "+ GMAINPGM +" to complete startup" << endl ) ;
	elapsed = 0 ;
	while ( currAppl->busyAppl )
	{
		elapsed++ ;
		boost::this_thread::sleep_for( boost::chrono::milliseconds( ZWAIT ) ) ;
		if ( elapsed > GMAXWAIT ) { currAppl->set_timeout_abend() ; }
	}
	if ( currAppl->terminateAppl )
	{
		errorScreen( 1, "An error has occured initialising the first "+ GMAINPGM +" main task.  lspf cannot continue." ) ;
		currAppl->info() ;
		currAppl->closeTables() ;
		p_poolMGR->disconnect( currAppl->taskid() ) ;
		llog( "I", "Removing application instance of "+ currAppl->get_appname() << endl ) ;
		((void (*)(pApplication*))(apps[ currAppl->get_appname() ].destroyer_ep))( currAppl )  ;
		delete pThread    ;
		delete p_poolMGR  ;
		delete p_tableMGR ;
		delete lgx        ;
		delete currScrn   ;
		llog( "I", "lspf and LOG terminating" << endl ) ;
		lg->close() ;
		delete lg   ;
		delete bThread ;
		return 0    ;
	}
	llog( "I", "First thread "+ GMAINPGM +" started and initialised.  ID=" << pThread->get_id() << endl ) ;

	currAppl->get_cursor( row, col ) ;
	currScrn->set_cursor( row, col ) ;

	mainLoop() ;

	lspfStatus = 'S'  ;

	delete p_poolMGR  ;
	delete p_tableMGR ;
	delete lgx        ;

	for ( it = apps.begin() ; it != apps.end() ; it++ )
	{
		if ( it->second.dlopened )
		{
			llog( "I", "dlclose of "+ it->first +" at " << it->second.dlib << endl ) ;
			unloadDynamicClass( it->second.dlib ) ;
		}
	}

	llog( "I", "Terminating background job monitor task" << endl ) ;
	while ( backStatus == 'R' )
	{
		boost::this_thread::sleep_for( boost::chrono::milliseconds( ZWAIT ) ) ;
	}
	delete bThread ;

	llog( "I", "lspf and LOG terminating" << endl ) ;
	lg->close() ;
	delete lg ;

	return 0  ;
}


void mainLoop()
{
	llog( "I", "mainLoop() entered" << endl ) ;

	uint c   ;
	uint row ;
	uint col ;

	bool passthru ;
	bool showLock ;
	bool Insert   ;

	string field_name ;
	string w1         ;
	string w2         ;

	fieldExc fxc ;
	MEVENT event ;

	err.clear()  ;

	currScrn->OIA_setup() ;

	mousemask( ALL_MOUSE_EVENTS, NULL ) ;
	showLock = false ;
	Insert   = false ;

	set_escdelay( 25 ) ;

	while ( pLScreen::screensTotal > 0 )
	{
		currScrn->clear_status() ;

		row = currScrn->get_row() ;
		col = currScrn->get_col() ;

		currScrn->OIA_update( screenNum, altScreen, boost::posix_time::microsec_clock::universal_time() ) ;
		currScrn->show_lock( showLock ) ;

		showLock      = false ;
		pfkeyPressed  = false ;
		ctlkeyPressed = false ;
		ctlAction     = ""    ;

		if ( commandStack == ""            &&
		     !currAppl->ControlDisplayLock &&
		     !currAppl->line_output_done() &&
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
		currScrn->OIA_startTime( boost::posix_time::microsec_clock::universal_time() ) ;

		if ( c < 256 && isprint( c ) )
		{
			if ( currAppl->inputInhibited() ) { continue ; }
			currAppl->currPanel->field_edit( err, row, col, char( c ), Insert, showLock ) ;
			currAppl->currPanel->get_cursor( row, col ) ;
			currScrn->set_cursor( row, col ) ;
			continue ;
		}

		if ( c == KEY_MOUSE && getmouse( &event ) == OK )
		{
			if ( event.bstate & BUTTON1_CLICKED )
			{
				currScrn->set_cursor( event.y, event.x ) ;
			}
			else if ( event.bstate & BUTTON1_DOUBLE_CLICKED )
			{
				row = event.y ;
				col = event.x ;
				currScrn->set_cursor( row, col ) ;
				c = KEY_ENTER ;
			}
		}

		if ( c >= CTRL( 'a' ) && c <= CTRL( 'z' ) && c != CTRL( 'i' ) )
		{
			ctlAction = ControlKeyAction( c ) ;
			ctlkeyPressed = true ;
			c = KEY_ENTER ;
		}

		switch( c )
		{

			case KEY_LEFT:
				currScrn->cursor_left() ;
				if ( currAppl->currPanel->pd_active() )
				{
					col = currScrn->get_col() ;
					currAppl->currPanel->display_current_pd( err, row, col ) ;
				}
				break ;

			case KEY_RIGHT:
				currScrn->cursor_right() ;
				if ( currAppl->currPanel->pd_active() )
				{
					col = currScrn->get_col() ;
					currAppl->currPanel->display_current_pd( err, row, col ) ;
				}
				break ;

			case KEY_UP:
				currScrn->cursor_up() ;
				if ( currAppl->currPanel->pd_active() )
				{
					row = currScrn->get_row() ;
					currAppl->currPanel->display_current_pd( err, row, col ) ;
				}
				break ;

			case KEY_DOWN:
				currScrn->cursor_down() ;
				if ( currAppl->currPanel->pd_active() )
				{
					row = currScrn->get_row() ;
					currAppl->currPanel->display_current_pd( err, row, col ) ;
				}
				break ;

			case CTRL( 'i' ):   // Tab key
				actionTabKey( row, col ) ;
				break ;

			case KEY_IC:
				Insert = !Insert ;
				currScrn->set_Insert( Insert ) ;
				break ;

			case KEY_HOME:
				if ( currAppl->currPanel->pd_active() )
				{
					currAppl->currPanel->get_pd_home( row, col ) ;
					currAppl->currPanel->display_current_pd( err, row, col ) ;
				}
				else
				{
					currAppl->get_home( row, col ) ;
				}
				currScrn->set_cursor( row, col ) ;
				break ;

			case KEY_DC:
				if ( currAppl->inputInhibited() ) { break ; }
				currAppl->currPanel->field_delete_char( err, row, col, showLock ) ;
				break ;

			case KEY_SDC:
				if ( currAppl->inputInhibited() ) { break ; }
				currAppl->currPanel->field_erase_eof( err, row, col, showLock ) ;
				break ;

			case KEY_END:
				currAppl->currPanel->cursor_eof( row, col ) ;
				currScrn->set_cursor( row, col ) ;
				break ;

			case 127:
			case KEY_BACKSPACE:
				if ( currAppl->inputInhibited() ) { break ; }
				currAppl->currPanel->field_backspace( err, row, col, showLock ) ;
				currScrn->set_cursor( row, col ) ;
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
			case CTRL( '[' ):       // Escape key

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
					if ( currAppl->selectPanel() )
					{
						processZSEL() ;
					}
					currAppl->get_cursor( row, col ) ;
					currScrn->set_cursor( row, col ) ;
					while ( currAppl->terminateAppl )
					{
						terminateApplication() ;
						if ( pLScreen::screensTotal == 0 ) { return ; }
						if ( currAppl->SEL && !currAppl->terminateAppl )
						{
							processPGMSelect() ;
						}
					}
					updateReflist() ;
					continue ;
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
				else if ( ZCOMMAND == "ACTIONS" )
				{
					displayNextPullDown( row, col ) ;
					break ;
				}
				else if ( ZCOMMAND == ".AUTO" )
				{
					currAppl->set_cursor( row, col ) ;
					autoUpdate() ;
					break ;
				}
				else if ( ZCOMMAND == "DISCARD" )
				{
					currAppl->currPanel->refresh_fields( err ) ;
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
						executeFieldCommand( field_name, fxc, col ) ;
					}
					else
					{
						issueMessage( "PSYS012J" ) ;
					}
					break ;
				}
				else if ( ZCOMMAND == ".HIDE" )
				{
					if ( ZPARM == "NULLS" )
					{
						p_poolMGR->sysput( err, "ZNULLS", "NO", SHARED ) ;
						currAppl->currPanel->redraw_fields( err ) ;
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
						p_poolMGR->sysput( err, "ZSCALE", ZPARM, SHARED ) ;
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
					system( p_poolMGR->sysget( err, "ZSHELL", SHARED ).c_str() ) ;
					reset_prog_mode() ;
					refresh()         ;
				}
				else if ( ZCOMMAND == ".SHOW" )
				{
					if ( ZPARM == "NULLS" )
					{
						p_poolMGR->sysput( err, "ZNULLS", "YES", SHARED ) ;
						currAppl->currPanel->redraw_fields( err ) ;
					}
				}
				else if ( ZCOMMAND == "SPLIT" )
				{
					if ( currAppl->msg_issued_with_cmd() )
					{
						currAppl->clear_msg() ;
					}
					SELCT.clear() ;
					SELCT.PGM     = GMAINPGM ;
					SELCT.PARM    = ""    ;
					SELCT.NEWAPPL = "ISP" ;
					SELCT.NEWPOOL = true  ;
					SELCT.PASSLIB = false ;
					SELCT.SUSPEND = true  ;
					SELCT.selPanl = true  ;
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
					actionSwap( row, col ) ;
					break ;
				}
				else if ( ZCOMMAND == ".TASKS" )
				{
					listBackTasks() ;
				}
				else if ( ZCOMMAND == ".TEST" )
				{
					currAppl->setTestMode() ;
					llog( "W", "Application is now running in test mode" << endl ) ;
				}
				else if ( ZCOMMAND == "TDOWN" )
				{
					currAppl->currPanel->field_tab_down( row, col ) ;
					currScrn->set_cursor( row, col ) ;
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
				currScrn->set_cursor( row, col ) ;
				if ( SEL )
				{
					updateDefaultVars() ;
					if ( SELCT.backgrd )
					{
						startApplicationBack( SELCT ) ;
					}
					else
					{
						currAppl->currPanel->remove_pd() ;
						startApplication( SELCT ) ;
					}
				}
				break ;
			default:
				debug1( "Action key "<<c<<" ("<<keyname( c )<<") ignored" << endl ) ;
		}
	}
}


void setGlobalClassVars()
{
	pApplication::p_tableMGR = p_tableMGR ;
	pApplication::p_poolMGR  = p_poolMGR  ;
	pApplication::lg         = lgx        ;
	pPanel::p_poolMGR        = p_poolMGR  ;
	abc::p_poolMGR           = p_poolMGR  ;
	pPanel::lg               = lgx        ;
	tableMGR::lg             = lgx        ;
	poolMGR::lg              = lgx        ;
}


void initialSetup()
{
	err.clear() ;

	funcPOOL.define( err, "ZCTVERB",  &ZCTVERB  ) ;
	funcPOOL.define( err, "ZCTTRUNC", &ZCTTRUNC ) ;
	funcPOOL.define( err, "ZCTACT",   &ZCTACT   ) ;
	funcPOOL.define( err, "ZCTDESC",  &ZCTDESC  ) ;
}


void processZSEL()
{
	// Called for a selection panel (ie. SELECT PANEL(ABC) function ).
	// Use what's in ZSEL to start application

	int p ;

	string cmd ;
	string opt ;

	bool addpop = false ;

	err.clear() ;

	currAppl->save_errblock()  ;
	cmd = currAppl->get_zsel() ;
	err = currAppl->get_errblock() ;
	currAppl->restore_errblock()   ;
	if ( err.error() )
	{
		serviceCallError( err ) ;
	}

	if ( cmd == "" ) { return ; }

	if ( cmd.compare( 0, 5, "PANEL" ) == 0 )
	{
		opt = currAppl->get_dTRAIL() ;
		if ( opt != "" ) { cmd += " OPT(" + opt + ")" ; }
	}

	p = wordpos( "ADDPOP", cmd ) ;
	if ( p > 0 )
	{
		addpop = true ;
		idelword( cmd, p, 1 ) ;
	}

	updateDefaultVars() ;
	currAppl->currPanel->remove_pd() ;

	if ( !SELCT.parse( err, cmd ) )
	{
		errorScreen( 1, "Error in ZSEL command "+ cmd ) ;
		return ;
	}

	if ( SELCT.PGM.front() == '&' )
	{
		currAppl->vcopy( substr( SELCT.PGM, 2 ), SELCT.PGM, MOVE ) ;
	}

	if ( addpop )
	{
		SELCT.PARM += " ADDPOP" ;
	}

	if ( SELCT.backgrd )
	{
		startApplicationBack( SELCT ) ;
	}
	else
	{
		startApplication( SELCT ) ;
	}
}


void processAction( uint row, uint col, int c, bool& passthru )
{
	// Return if application is just doing line output
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
	string msg     ;
	string t       ;

	static uint retPos(0) ;

	boost::circular_buffer<string>::iterator itt ;

	err.clear() ;

	pdc t_pdc ;

	SEL      = false ;
	passthru = true  ;
	PFCMD    = ""    ;
	ZCOMMAND = ""    ;

	if ( currAppl->line_output_done() ) { return ; }

	p_poolMGR->put( err, "ZVERB", "", SHARED ) ;
	if ( err.error() )
	{
		llog( "C", "poolMGR put for ZVERB failed" << endl ) ;
	}

	if ( c == CTRL( '[' ) )
	{
		if ( currAppl->currPanel->pd_active() )
		{
			currAppl->currPanel->remove_pd() ;
			currAppl->clear_msg() ;
			ZCOMMAND = "" ;
		}
		else
		{
			ZCOMMAND = "SWAP" ;
			ZPARM    = "LIST" ;
		}
		passthru = false ;
		return ;
	}

	if ( c == KEY_ENTER && !ctlkeyPressed )
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
			currScrn->set_cursor( row, col ) ;
			currAppl->display_pd( err ) ;
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
			if ( currAppl->currPanel->display_pd( err, row+2, col, msg ) )
			{
				if ( msg != "" )
				{
					issueMessage( msg ) ;
				}
				passthru = false ;
				currAppl->currPanel->get_cursor( row, col ) ;
				currScrn->set_cursor( row, col ) ;
				ZCOMMAND = "NOP" ;
				return ;
			}
			else if ( err.error() )
			{
				errorScreen( 1, "Error processing pull-down menu." ) ;
				serviceCallError( err ) ;
				currAppl->get_home( row, col ) ;
				currScrn->set_cursor( row, col ) ;
				return ;
			}
		}
		else if ( currAppl->currPanel->pd_active() )
		{
			if ( !currAppl->currPanel->cursor_on_pulldown( row, col ) )
			{
				currAppl->currPanel->remove_pd() ;
				currAppl->clear_msg() ;
				ZCOMMAND = "NOP" ;
				passthru = false ;
				return ;
			}
			t_pdc = currAppl->currPanel->retrieve_choice( err, msg ) ;
			if ( t_pdc.pdc_inact )
			{
				msg = "PSYS012T" ;
			}
			if ( msg != "" )
			{
				issueMessage( msg ) ;
				ZCOMMAND = "NOP" ;
				passthru = false ;
				return ;
			}
			else if ( t_pdc.pdc_run != "" )
			{
				currAppl->currPanel->remove_pd() ;
				currAppl->clear_msg() ;
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
						if ( SELCT.PGM.front() == '&' )
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
			else
			{
				currAppl->currPanel->remove_pd() ;
				currAppl->clear_msg() ;
				ZCOMMAND = "" ;
				return ;
			}
		}
	}

	if ( wmPending )
	{
		wmPending = false ;
		currAppl->clear_msg() ;
	}

	addRetrieve = true ;
	delm        = p_poolMGR->sysget( err, "ZDEL", PROFILE ) ;

	if ( t_pdc.pdc_run == "" )
	{
		ZCOMMAND = strip( currAppl->currPanel->cmd_getvalue() ) ;
	}

	if ( c == KEY_ENTER  &&
	     ZCOMMAND != ""  &&
	     currAppl->currPanel->has_command_field() &&
	     p_poolMGR->sysget( err, "ZSWAPC", PROFILE ) == ZCOMMAND.substr( 0, 1 ) &&
	     p_poolMGR->sysget( err, "ZSWAP",  PROFILE ) == "Y" )
	{
		currAppl->currPanel->field_get_row_col( currAppl->currPanel->cmdfield, rw, cl ) ;
		if ( rw == row && cl < col )
		{
			ZPARM    = upper( ZCOMMAND.substr( 1, col-cl-1 ) ) ;
			ZCOMMAND = "SWAP" ;
			passthru = false  ;
			currAppl->currPanel->cmd_setvalue( err, "" ) ;
			return ;
		}
	}

	if ( ctlkeyPressed )
	{
		PFCMD = ctlAction ;
	}

	if ( commandStack != "" )
	{
		if ( ZCOMMAND != "" )
		{
			currAppl->currPanel->cmd_setvalue( err, ZCOMMAND + commandStack ) ;
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
		if ( p_poolMGR->sysget( err, "ZKLUSE", PROFILE ) == "Y" )
		{
			currAppl->save_errblock() ;
			currAppl->reload_keylist( currAppl->currPanel ) ;
			err = currAppl->get_errblock() ;
			currAppl->restore_errblock()   ;
			if ( err.error() )
			{
				serviceCallError( err ) ;
			}
			else
			{
				PFCMD = currAppl->currPanel->get_keylist( c ) ;
			}
		}
		if ( PFCMD == "" )
		{
			PFCMD = pfKeyValue( c ) ;
		}
		t = "PF" + d2ds( c - KEY_F( 0 ), 2 ) ;
		p_poolMGR->put( err, "ZPFKEY", t, SHARED, SYSTEM ) ;
		debug1( "PF Key pressed " <<t<<" value "<< PFCMD << endl ) ;
		currAppl->currPanel->set_pfpressed( t ) ;
	}
	else
	{
		p_poolMGR->put( err, "ZPFKEY", "PF00", SHARED, SYSTEM ) ;
		if ( err.error() ) { llog( "C", "VPUT for PF00 failed" << endl ) ; }
		currAppl->currPanel->set_pfpressed( "" ) ;
	}

	if ( addRetrieve )
	{
		rbsize = ds2d( p_poolMGR->sysget( err, "ZRBSIZE", PROFILE ) ) ;
		rtsize = ds2d( p_poolMGR->sysget( err, "ZRTSIZE", PROFILE ) ) ;
		if ( retrieveBuffer.capacity() != rbsize )
		{
			retrieveBuffer.rset_capacity( rbsize ) ;
		}
		if (  ZCOMMAND.size() >= rtsize &&
		     !findword( word( upper( ZCOMMAND ), 1 ), "RETRIEVE RETP" ) &&
		     !findword( word( upper( PFCMD ), 1 ), "RETRIEVE RETP" ) )
		{
			itt = find( retrieveBuffer.begin(), retrieveBuffer.end(), ZCOMMAND ) ;
			if ( itt != retrieveBuffer.end() )
			{
				retrieveBuffer.erase( itt ) ;
			}
			retrieveBuffer.push_front( ZCOMMAND ) ;
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

	if ( ZCOMMAND.compare( 0, 2, delm+delm ) == 0 )
	{
		commandStack = ZCOMMAND.substr( 1 ) ;
		ZCOMMAND     = ""                   ;
		currAppl->currPanel->cmd_setvalue( err, "" ) ;
		return ;
	}
	else if ( ZCOMMAND.compare( 0, 1, delm ) == 0 )
	{
		ZCOMMAND.erase( 0, 1 ) ;
		currAppl->currPanel->cmd_setvalue( err, ZCOMMAND ) ;
	}

	p1 = ZCOMMAND.find( delm.front() ) ;
	if ( p1 != string::npos )
	{
		commandStack = ZCOMMAND.substr( p1 ) ;
		ZCOMMAND.erase( p1 )                 ;
		currAppl->currPanel->cmd_setvalue( err, ZCOMMAND ) ;
	}

	CMDVerb = upper( word( ZCOMMAND, 1 ) ) ;
	CMDParm = subword( ZCOMMAND, 2 ) ;

	if ( CMDVerb == "" ) { retPos = 0 ; return ; }

	if ( CMDVerb.front() == '@' )
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
		currAppl->currPanel->cmd_setvalue( err, "" ) ;
		return ;
	}
	else if ( CMDVerb.front() == '!' )
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
		currAppl->currPanel->cmd_setvalue( err, "" ) ;
		return ;
	}

	if ( ZCOMMAND.size() > 1 && ZCOMMAND.front() == '=' && ( PFCMD == "" || PFCMD == "RETURN" ) )
	{
		ZCOMMAND.erase( 0, 1 ) ;
		passthru = true ;
		if ( !currAppl->isprimMenu() )
		{
			currAppl->jumpEntered = true ;
			p_poolMGR->put( err, "ZVERB", "RETURN", SHARED ) ;
			currAppl->currPanel->cmd_setvalue( err, "" ) ;
			return ;
		}
		currAppl->currPanel->cmd_setvalue( err, ZCOMMAND ) ;
	}

	if ( CMDVerb == "RETRIEVE" )
	{
		if ( !currAppl->currPanel->has_command_field() ) { return ; }
		if ( datatype( CMDParm, 'W' ) )
		{
			p1 = ds2d( CMDParm ) ;
			if ( p1 > 0 && p1 <= retrieveBuffer.size() ) { retPos = p1 - 1 ; }
		}
		commandStack = ""    ;
		ZCOMMAND     = "NOP" ;
		passthru     = false ;
		fld          = currAppl->currPanel->cmdfield ;
		if ( !retrieveBuffer.empty() )
		{
			currAppl->currPanel->cmd_setvalue( err, retrieveBuffer[ retPos ] ) ;
			currAppl->currPanel->cursor_to_field( RC, fld, retrieveBuffer[ retPos ].size()+1 ) ;
			if ( ++retPos >= retrieveBuffer.size() ) { retPos = 0 ; }
		}
		else
		{
			currAppl->currPanel->cmd_setvalue( err, "" ) ;
			currAppl->currPanel->cursor_to_field( RC, fld ) ;
		}
		currAppl->currPanel->get_cursor( row, col ) ;
		currScrn->set_cursor( row, col ) ;
		currAppl->currPanel->remove_pd() ;
		currAppl->clear_msg() ;
		return ;
	}
	retPos = 0 ;

	if ( CMDVerb == "HELP")
	{
		commandStack = "" ;
		if ( currAppl->currPanel->msgid == "" || currAppl->currPanel->showLMSG )
		{
			currAppl->currPanel->cmd_setvalue( err, "" ) ;
			ZPARM   = currAppl->get_help_member( row, col ) ;
			CMDParm = ZPARM ;
		}
		else
		{
			ZCOMMAND = "NOP" ;
			passthru = false ;
			currAppl->currPanel->showLMSG = true ;
			currAppl->currPanel->display_msg( err ) ;
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
			currAppl->currPanel->cmd_setvalue( err, "" ) ;
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
			if ( SELCT.PGM.front() == '&' )
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
				currAppl->currPanel->cmd_setvalue( err, "" ) ;
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

	if ( CMDVerb.front() == '>' )
	{
		ZCOMMAND.erase( 0, 1 ) ;
	}

	if ( currAppl->currPanel->pd_active() && ZCTVERB == "END" )
	{
		currAppl->currPanel->remove_pd() ;
		currAppl->clear_msg() ;
		ZCOMMAND = ""    ;
		passthru = false ;
	}

	currAppl->currPanel->cmd_setvalue( err, passthru ? ZCOMMAND : "" ) ;
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

	err.clear() ;

	ZCTVERB  = "" ;
	ZCTTRUNC = "" ;
	ZCTACT   = "" ;
	ZCTDESC  = "" ;

	cmdtlst  = "" ;
	found    = false ;

	siteBefore = ( p_poolMGR->sysget( err, "ZSCMDTF", PROFILE ) == "Y" ) ;

	if ( currAppl->get_applid() != "ISP" ) { cmdtlst = currAppl->get_applid() + " " ; }
	cmdtlst += p_poolMGR->sysget( err, "ZUCMDT1", PROFILE ) + " " +
		   p_poolMGR->sysget( err, "ZUCMDT2", PROFILE ) + " " +
		   p_poolMGR->sysget( err, "ZUCMDT3", PROFILE ) + " " ;
	if ( !siteBefore )
	{
		  cmdtlst += "ISP " ;
	}
	cmdtlst += p_poolMGR->sysget( err, "ZSCMDT1", PROFILE ) + " " +
		   p_poolMGR->sysget( err, "ZSCMDT2", PROFILE ) + " " +
		   p_poolMGR->sysget( err, "ZSCMDT3", PROFILE ) + " " ;
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
			if ( ZCTACT.front() == '&' )
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
		if ( SELCT.backgrd )
		{
			startApplicationBack( SELCT, true ) ;
			ResumeApplicationAndWait() ;
		}
		else
		{
			startApplication( SELCT ) ;
		}
	}
	else
	{
		if ( !currAppl->errorsReturn() )
		{
			errorScreen( 1, "SELECT function did not find application '"+ SELCT.PGM +"'" ) ;
		}
		currAppl->SEL = false ;
		currAppl->RC  = 20    ;
		ResumeApplicationAndWait() ;
		while ( currAppl->terminateAppl )
		{
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
	int spool   ;
	int p1      ;

	uint row    ;
	uint col    ;

	string opt   ;
	string rest  ;
	string sname ;
	string applid ;

	bool setMSG ;

	err.clear() ;

	pApplication * oldAppl = currAppl ;

	boost::thread * pThread ;

	if ( SEL.PGM == "ISPSTRT" )
	{
		currAppl->clear_msg() ;
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
			SEL.selPanl = true  ;
		}
		if ( SEL.PGM.front() == '&' )
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

	currAppl->store_scrname()   ;
	spool  = currAppl->shrdPool ;
	setMSG = currAppl->setMSG   ;
	sname  = p_poolMGR->get( err, "ZSCRNAME", SHARED ) ;
	if ( setMSG )
	{
		currAppl->setMSG = false ;
	}

	llog( "I", "Starting new application "+ SEL.PGM +" with parameters '"+ SEL.PARM +"'" << endl ) ;
	currAppl = ((pApplication*(*)())( apps[ SEL.PGM ].maker_ep))() ;

	currScrn->application_add( currAppl ) ;
	currAppl->startSelect( SEL )          ;
	currAppl->taskid( ++maxtaskID )       ;
	currAppl->lspfCallback = lspfCallbackHandler ;
	currAppl->set_output_done( oldAppl->line_output_done() ) ;
	apps[ SEL.PGM ].refCount++ ;

	applid = ( SEL.NEWAPPL == "" ) ? oldAppl->get_applid() : SEL.NEWAPPL ;
	err.settask( currAppl->taskid() ) ;

	if ( SEL.NEWPOOL )
	{
		if ( currScrn->application_stack_size() > 1 && SELCT.SCRNAME == "" )
		{
			SELCT.SCRNAME = sname ;
		}
		currAppl->NEWPOOL = true ;
		spool = p_poolMGR->createSharedPool() ;
	}

	p_poolMGR->createProfilePool( err, applid, ZSPROF ) ;
	p_poolMGR->connect( currAppl->taskid(), applid, spool ) ;
	if ( err.RC4() ) { createpfKeyDefaults() ; }

	p_poolMGR->put( err, "ZSCREEN", string( 1, ZSCREEN[ screenNum ] ), SHARED, SYSTEM ) ;
	p_poolMGR->put( err, "ZSCRNUM", d2ds( currScrn->screenId ), SHARED, SYSTEM ) ;
	p_poolMGR->put( err, "ZAPPLID", applid, SHARED, SYSTEM ) ;

	currAppl->shrdPool = spool ;
	currAppl->init() ;

	if ( !SEL.SUSPEND )
	{
		currAppl->set_addpop_row( oldAppl->get_addpop_row() ) ;
		currAppl->set_addpop_col( oldAppl->get_addpop_col() ) ;
		currAppl->set_addpop_act( oldAppl->get_addpop_act() ) ;
	}

	if ( SEL.PASSLIB || SEL.NEWAPPL == "" )
	{
  //  todo      currAppl->ZMUSER = oldAppl->ZMUSER ;
  //            currAppl->ZPUSER = oldAppl->ZPUSER ;
  //            currAppl->ZTUSER = oldAppl->ZTUSER ;
		currAppl->libdef_muser = oldAppl->libdef_muser ;
		currAppl->libdef_puser = oldAppl->libdef_puser ;
		currAppl->libdef_tuser = oldAppl->libdef_tuser ;
	}

	if ( setMSG )
	{
		currAppl->set_msg1( oldAppl->getmsg1(), oldAppl->getmsgid1() ) ;
	}
	else if ( !nScreen )
	{
		oldAppl->clear_msg() ;
	}

	pThread = new boost::thread( boost::bind( &pApplication::application, currAppl ) ) ;

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
		boost::this_thread::sleep_for( boost::chrono::milliseconds( ZWAIT ) ) ;
		if ( currAppl->noTimeOut ) { elapsed = 0 ; }
		if ( elapsed > GMAXWAIT  ) { currAppl->set_timeout_abend() ; }
	}

	if ( currAppl->do_refresh_lscreen() )
	{
		if ( linePosn != -1 )
		{
			lineOutput_end() ;
			linePosn = -1 ;
		}
		currScrn->refresh_panel_stack() ;
	}

	if ( currAppl->abnormalEnd )
	{
		abnormalTermMessage()  ;
		terminateApplication() ;
		if ( pLScreen::screensTotal == 0 ) { return ; }
	}
	else
	{
		llog( "I", "New thread and application started and initialised. ID=" << pThread->get_id() << endl ) ;
		if ( currAppl->SEL )
		{
			processPGMSelect() ;
		}
		else if ( currAppl->line_output_pending() )
		{
			lineOutput() ;
		}
	}

	while ( currAppl->terminateAppl )
	{
		terminateApplication() ;
		if ( pLScreen::screensTotal == 0 ) { return ; }
		if ( currAppl->SEL && !currAppl->terminateAppl )
		{
			processPGMSelect() ;
		}
	}
	currAppl->get_cursor( row, col ) ;
	currScrn->set_cursor( row, col ) ;
}


void startApplicationBack( selobj SEL, bool pgmselect )
{
	// Start a background application using the passed SELECT object

	int spool ;

	string applid ;

	errblock err1 ;

	pApplication * oldAppl = currAppl ;
	pApplication * Appl ;

	boost::thread * pThread ;

	if ( apps.find( SEL.PGM ) == apps.end() )
	{
		llog( "E", "Application '"+ SEL.PGM +"' not found" <<endl ) ;
		return ;
	}

	if ( !apps[ SEL.PGM ].dlopened )
	{
		if ( !loadDynamicClass( SEL.PGM ) )
		{
			llog( "E", "Errors loading "+ SEL.PGM <<endl ) ;
			return ;
		}
	}

	llog( "I", "Starting new background application "+ SEL.PGM +" with parameters '"+ SEL.PARM +"'" <<endl ) ;
	Appl = ((pApplication*(*)())( apps[ SEL.PGM ].maker_ep))() ;

	Appl->startSelect( SEL )    ;
	Appl->taskid( ++maxtaskID ) ;
	Appl->lspfCallback = lspfCallbackHandler ;
	Appl->set_background()    ;
	apps[ SEL.PGM ].refCount++ ;

	mtx.lock() ;
	pApplicationBackground.push_back( Appl ) ;
	mtx.unlock() ;

	applid = ( SEL.NEWAPPL == "" ) ? oldAppl->get_applid() : SEL.NEWAPPL ;
	err1.settask( Appl->taskid() ) ;

	if ( SEL.NEWPOOL )
	{
		Appl->NEWPOOL = true ;
		spool = p_poolMGR->createSharedPool() ;
	}
	else
	{
		spool  = oldAppl->shrdPool ;
	}

	if ( pgmselect )
	{
		oldAppl->ZTASKID = Appl->taskid() ;
	}

	p_poolMGR->createProfilePool( err1, applid, ZSPROF ) ;
	p_poolMGR->connect( Appl->taskid(), applid, spool ) ;
	if ( err1.RC4() ) { createpfKeyDefaults() ; }

	p_poolMGR->put( err1, "ZSCREEN", string( 1, ZSCREEN[ screenNum ] ), SHARED, SYSTEM ) ;
	p_poolMGR->put( err1, "ZSCRNUM", d2ds( currScrn->screenId ), SHARED, SYSTEM ) ;
	p_poolMGR->put( err1, "ZAPPLID", applid, SHARED, SYSTEM ) ;

	Appl->shrdPool = spool ;
	Appl->init() ;

	if ( SEL.PASSLIB || SEL.NEWAPPL == "" )
	{
  //  todo      Appl->ZMUSER = oldAppl->ZMUSER ;
  //            Appl->ZPUSER = oldAppl->ZPUSER ;
  //            Appl->ZTUSER = oldAppl->ZTUSER ;
		Appl->libdef_muser = oldAppl->libdef_muser ;
		Appl->libdef_puser = oldAppl->libdef_puser ;
		Appl->libdef_tuser = oldAppl->libdef_tuser ;
	}

	pThread = new boost::thread( boost::bind( &pApplication::application, Appl ) ) ;

	Appl->pThread = pThread ;

	llog( "I", "New background thread and application started and initialised. ID=" << pThread->get_id() << endl ) ;
}


void processBackgroundTasks()
{
	// This routine runs every 100ms in a separate thread to check if there are any
	// background tasks waiting for action:
	//    cleanup application if ended
	//    start application if SELECT() done in the background program

	backStatus = 'R' ;

	boost::thread * pThread ;

	while ( lspfStatus == 'R' )
	{
		boost::this_thread::sleep_for( boost::chrono::milliseconds( 100 ) ) ;
		mtx.lock() ;
		for ( auto it = pApplicationBackground.begin() ; it != pApplicationBackground.end() ; it++ )
		{
			while ( (*it)->terminateAppl )
			{
				llog( "I", "Removing background application instance of "+
					(*it)->get_appname() << " taskid: " << (*it)->taskid() << endl ) ;
				p_poolMGR->disconnect( (*it)->taskid() ) ;
				pThread = (*it)->pThread ;
				apps[ (*it)->get_appname() ].refCount-- ;
				((void (*)(pApplication*))(apps[ (*it)->get_appname() ].destroyer_ep))( (*it) ) ;
				delete pThread ;
				it = pApplicationBackground.erase( it ) ;
				if ( it == pApplicationBackground.end() ) { break ; }
			}
			if ( it == pApplicationBackground.end() ) { break ; }
		}
		mtx.unlock() ;

		mtx.lock() ;
		vector<pApplication *> temp = pApplicationBackground ;

		for ( auto it = temp.begin() ; it != temp.end() ; it++ )
		{
			if ( (*it)->SEL && !currAppl->terminateAppl )
			{
				startApplicationBack( (*it)->get_select_cmd() ) ;
				(*it)->busyAppl = true  ;
				cond_appl.notify_all()  ;
			}
		}
		mtx.unlock() ;
	}
	backStatus = 'S' ;
}


void terminateApplication()
{
	int tRC  ;
	int tRSN ;

	uint row ;
	uint col ;

	string ZAPPNAME ;
	string tRESULT  ;
	string tMSGID1  ;
	string fname    ;
	string delm     ;

	bool refList      ;
	bool nretError    ;
	bool propagateEnd ;
	bool abnormalEnd  ;
	bool jumpEntered  ;
	bool setCursor    ;
	bool setMSG       ;
	bool lineOutput   ;

	slmsg tMSG1 ;

	err.clear() ;

	boost::thread * pThread ;

	llog( "I", "Application terminating "+ currAppl->get_appname() +" ID: "<< currAppl->taskid() << endl ) ;

	ZAPPNAME = currAppl->get_appname() ;

	currAppl->closeTables()  ;
	tRC     = currAppl->ZRC  ;
	tRSN    = currAppl->ZRSN ;
	tRESULT = currAppl->ZRESULT ;
	abnormalEnd = currAppl->abnormalEnd ;

	refList = ( currAppl->reffield == "#REFLIST" ) ;

	setMSG = currAppl->setMSG ;
	if ( setMSG ) { tMSGID1 = currAppl->getmsgid1() ; tMSG1 = currAppl->getmsg1() ; }

	jumpEntered  = currAppl->jumpEntered ;
	propagateEnd = currAppl->propagateEnd && ( currScrn->application_stack_size() > 1 ) ;
	lineOutput   = currAppl->line_output_done() ;

	pThread = currAppl->pThread ;

	if ( currAppl->abnormalEnd )
	{
		while ( currAppl->cleanupRunning() )
		{
			boost::this_thread::sleep_for( boost::chrono::milliseconds( ZWAIT ) ) ;
		}
		pThread->detach() ;
	}

	p_poolMGR->disconnect( currAppl->taskid() ) ;

	if ( currAppl->abnormalTimeout )
	{
		llog( "I", "Moving application instance of "+ ZAPPNAME +" to timeout queue" << endl ) ;
		pApplicationTimeout.push_back( currAppl ) ;
	}
	else
	{
		llog( "I", "Removing application instance of "+ ZAPPNAME << endl ) ;
		apps[ ZAPPNAME ].refCount-- ;
		((void (*)(pApplication*))(apps[ ZAPPNAME ].destroyer_ep))( currAppl ) ;
		delete pThread ;
	}

	currScrn->application_remove_current() ;
	if ( currScrn->application_stack_empty() )
	{
		p_poolMGR->destroyPool( currScrn->screenId ) ;
		if ( pLScreen::screensTotal == 1 )
		{
			delete currScrn ;
			llog( "I", "Closing ISPS profile and application log as last application program is terminating" << endl ) ;
			err.settask( 0 ) ;
			p_poolMGR->destroySystemPool( err ) ;
			p_poolMGR->statistics()  ;
			p_tableMGR->statistics() ;
			llog( "I", "Closing application log" << endl ) ;
			return ;
		}
		deleteLogicalScreen() ;
	}

	currAppl = currScrn->application_get_current() ;
	err.settask( currAppl->taskid() ) ;

	currAppl->display_pd( err ) ;

	p_poolMGR->put( err, "ZPANELID", currAppl->panelid, SHARED, SYSTEM ) ;

	if ( apps[ ZAPPNAME ].refCount == 0 && apps[ ZAPPNAME ].relPending )
	{
		apps[ ZAPPNAME ].relPending = false ;
		llog( "I", "Reloading module "+ ZAPPNAME +" (pending reload status)" << endl ) ;
		if ( loadDynamicClass( ZAPPNAME ) )
		{
			llog( "I", "Loaded "+ ZAPPNAME +" (module "+ apps[ ZAPPNAME ].module +") from "+ apps[ ZAPPNAME ].file << endl ) ;
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
					if ( p_poolMGR->sysget( err, "ZRFMOD", PROFILE ) == "BEX" )
					{
						delm = p_poolMGR->sysget( err, "ZDEL", PROFILE ) ;
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
				currAppl->currPanel->field_setvalue( err, currAppl->reffield, tRESULT ) ;
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
			if ( jumpEntered )
			{
				currAppl->jumpEntered  = true ;
			}
			currAppl->RC           = 4    ;
			currAppl->propagateEnd = true ;
		}
		else
		{
			currAppl->RC = 0 ;
		}
		if ( abnormalEnd )
		{
			currAppl->ZRC     = 20  ;
			currAppl->ZRSN    = 999 ;
			currAppl->ZRESULT = "Abended" ;
			if ( !currAppl->errorsReturn() )
			{
				currAppl->abnormalEnd = true ;
			}
		}
		else
		{
			currAppl->ZRC     = tRC     ;
			currAppl->ZRSN    = tRSN    ;
			currAppl->ZRESULT = tRESULT ;
		}
		currAppl->SEL = false ;
		if ( setMSG ) { currAppl->set_msg1( tMSG1, tMSGID1 ) ; }
		currAppl->set_output_done( lineOutput ) ;
		ResumeApplicationAndWait() ;
		while ( currAppl->terminateAppl )
		{
			terminateApplication() ;
			if ( pLScreen::screensTotal == 0 ) { return ; }
		}
		currAppl->get_cursor( row, col ) ;
	}
	else
	{
		if ( jumpEntered && propagateEnd )
		{
			p_poolMGR->put( err, "ZVERB", "RETURN", SHARED ) ;
			currAppl->jumpEntered = true ;
			ResumeApplicationAndWait()   ;
		}
		while ( currAppl->terminateAppl )
		{
			terminateApplication() ;
			if ( pLScreen::screensTotal == 0 ) { return ; }
		}
		if ( setMSG )
		{
			currAppl->set_msg1( tMSG1, tMSGID1, true ) ;
		}
		if ( setCursor )
		{
			currAppl->get_home( row, col )   ;
			currAppl->set_cursor( row, col ) ;
		}
		else
		{
			currAppl->get_cursor( row, col ) ;
		}
		if ( linePosn != -1 )
		{
			lineOutput_end() ;
			currScrn->refresh_panel_stack() ;
			update_panels() ;
			doupdate() ;
			linePosn = -1 ;
		}
	}

	llog( "I", "Application terminatation of "+ ZAPPNAME +" completed.  Current application is "+ currAppl->get_appname() << endl ) ;
	currAppl->restore_Zvars( currScrn->screenId ) ;
	currAppl->display_id() ;
	currScrn->set_cursor( row, col ) ;
}


bool createLogicalScreen()
{
	err.clear() ;

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
	currScrn->OIA_startTime( boost::posix_time::microsec_clock::universal_time() ) ;
	return true ;
}


void deleteLogicalScreen()
{
	delete currScrn ;

	screenList.erase( screenList.begin() + screenNum ) ;
	if ( altScreen > screenNum ) { altScreen-- ; }
	screenNum-- ;
	if ( screenNum < 0 ) { screenNum = pLScreen::screensTotal - 1 ; }
	if ( pLScreen::screensTotal > 1 )
	{
		if ( altScreen == screenNum )
		{
			altScreen = (altScreen == pLScreen::screensTotal - 1) ? 0 : (altScreen + 1) ;
		}
	}
	currScrn = screenList[ screenNum ] ;
	currScrn->restore_panel_stack()    ;
	currScrn->OIA_startTime( boost::posix_time::microsec_clock::universal_time() ) ;
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
		boost::this_thread::sleep_for( boost::chrono::milliseconds( ZWAIT ) ) ;
		if ( currAppl->noTimeOut ) { elapsed = 0 ; }
		if ( elapsed > GMAXWAIT  ) { currAppl->set_timeout_abend() ; }
	}

	if ( currAppl->reloadCUATables ) { loadCUATables() ; }
	if ( currAppl->do_refresh_lscreen() )
	{
		if ( linePosn != -1 )
		{
			lineOutput_end() ;
			linePosn = -1 ;
		}
		currScrn->refresh_panel_stack() ;
	}

	if ( currAppl->abnormalEnd )
	{
		abnormalTermMessage() ;
	}
	else if ( currAppl->SEL )
	{
		processPGMSelect() ;
	}
	else if ( currAppl->line_output_pending() )
	{
		lineOutput() ;
	}
}


void loadCUATables()
{
	// Set according to the ZC variables in ISPS profile

	setColourPair( "AB" )   ;
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

	setGlobalColours() ;
}


void setColourPair( const string& name )
{
	string t ;

	err.clear() ;

	pair<map<cuaType, unsigned int>::iterator, bool> result ;

	result = cuaAttr.insert( pair<cuaType, unsigned int>( cuaAttrName[ name ], WHITE ) ) ;

	map<cuaType, unsigned int>::iterator it = result.first ;

	t = p_poolMGR->sysget( err, "ZC"+ name, PROFILE ) ;
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
		case 'R': it->second = RED     ; break ;
		case 'G': it->second = GREEN   ; break ;
		case 'Y': it->second = YELLOW  ; break ;
		case 'B': it->second = BLUE    ; break ;
		case 'M': it->second = MAGENTA ; break ;
		case 'T': it->second = TURQ    ; break ;
		case 'W': it->second = WHITE   ; break ;

		default :  llog( "E", "Variable ZC"+ name +" has invalid value[0] "+ t << endl ) ;
			   llog( "C", "Rerun setup program to re-initialise ISPS profile" << endl ) ;
			   abortStartup() ;
	}

	switch  ( t[ 1 ] )
	{
		case 'L':  it->second = it->second | A_NORMAL ; break ;
		case 'H':  it->second = it->second | A_BOLD   ; break ;

		default :  llog( "E", "Variable ZC"+ name +" has invalid value[1] "+ t << endl ) ;
			   llog( "C", "Rerun setup program to re-initialise ISPS profile" << endl ) ;
			   abortStartup() ;
	}

	switch  ( t[ 2 ] )
	{
		case 'N':  break ;
		case 'B':  it->second = it->second | A_BLINK     ; break ;
		case 'R':  it->second = it->second | A_REVERSE   ; break ;
		case 'U':  it->second = it->second | A_UNDERLINE ; break ;

		default :  llog( "E", "Variable ZC"+ name +" has invalid value[2] "+ t << endl ) ;
			   llog( "C", "Rerun setup program to re-initialise ISPS profile" << endl ) ;
			   abortStartup() ;
	}
}


void setGlobalColours()
{
	string t ;

	map<char, unsigned int> gcolours = { { 'R', COLOR_RED,     } ,
					     { 'G', COLOR_GREEN,   } ,
					     { 'Y', COLOR_YELLOW,  } ,
					     { 'B', COLOR_BLUE,    } ,
					     { 'M', COLOR_MAGENTA, } ,
					     { 'T', COLOR_CYAN,    } ,
					     { 'W', COLOR_WHITE    } } ;

	err.clear() ;

	t = p_poolMGR->sysget( err, "ZGCLR", PROFILE ) ;
	init_pair( 1, gcolours[ t[ 0 ] ], COLOR_BLACK ) ;

	t = p_poolMGR->sysget( err, "ZGCLG", PROFILE ) ;
	init_pair( 2, gcolours[ t[ 0 ] ], COLOR_BLACK ) ;

	t = p_poolMGR->sysget( err, "ZGCLY", PROFILE ) ;
	init_pair( 3, gcolours[ t[ 0 ] ], COLOR_BLACK ) ;

	t = p_poolMGR->sysget( err, "ZGCLB", PROFILE ) ;
	init_pair( 4, gcolours[ t[ 0 ] ], COLOR_BLACK ) ;

	t = p_poolMGR->sysget( err, "ZGCLM", PROFILE ) ;
	init_pair( 5, gcolours[ t[ 0 ] ], COLOR_BLACK ) ;

	t = p_poolMGR->sysget( err, "ZGCLT", PROFILE ) ;
	init_pair( 6, gcolours[ t[ 0 ] ], COLOR_BLACK ) ;

	t = p_poolMGR->sysget( err, "ZGCLW", PROFILE ) ;
	init_pair( 7, gcolours[ t[ 0 ] ], COLOR_BLACK ) ;
}


void loadDefaultPools()
{
	// Default vars go in @DEFPROF (RO) for PROFILE and @DEFSHAR (UP) for SHARE
	// These have the SYSTEM attibute set on the variable

	string log  ;

	err.clear() ;

	struct utsname buf ;

	uname( &buf ) ;

	p_poolMGR->sysput( err, "ZSCREEND", d2ds( pLScreen::maxrow ), SHARED ) ;
	p_poolMGR->sysput( err, "ZSCRMAXD", d2ds( pLScreen::maxrow ), SHARED ) ;
	p_poolMGR->sysput( err, "ZSCREENW", d2ds( pLScreen::maxcol ), SHARED ) ;
	p_poolMGR->sysput( err, "ZSCRMAXW", d2ds( pLScreen::maxcol ), SHARED ) ;
	p_poolMGR->sysput( err, "ZUSER", getenv( "LOGNAME" ), SHARED ) ;
	p_poolMGR->sysput( err, "ZHOME", getenv( "HOME" ), SHARED )    ;
	p_poolMGR->sysput( err, "ZSHELL", getenv( "SHELL" ), SHARED )  ;

	p_poolMGR->createProfilePool( err, "ISPS", ZSPROF ) ;
	if ( !err.RC0() )
	{
		llog( "C", "Loading of system profile ISPSPROF failed.  RC="<< err.getRC() << endl ) ;
		llog( "C", "Aborting startup.  Check path " ZSPROF << endl ) ;
		abortStartup() ;
	}

	log = p_poolMGR->sysget( err, "ZSLOG", PROFILE ) ;

	lg->set( log ) ;
	llog( "I", "Starting logger on " << log << endl ) ;

	log = p_poolMGR->sysget( err, "ZALOG", PROFILE ) ;
	lgx->set( log ) ;
	llog( "I", "Starting application logger on " << log << endl ) ;

	llog( "I", "Loaded system profile ISPSPROF" << endl ) ;
	p_poolMGR->createSharedPool() ;

	ZTLIB = p_poolMGR->sysget( err, "ZTLIB", PROFILE ) ;

	p_poolMGR->sysput( err, "Z", "", SHARED )                  ;
	p_poolMGR->sysput( err, "ZSCRNAM1", "OFF", SHARED )        ;
	p_poolMGR->sysput( err, "ZSYSNAME", buf.sysname, SHARED )  ;
	p_poolMGR->sysput( err, "ZNODNAME", buf.nodename, SHARED ) ;
	p_poolMGR->sysput( err, "ZOSREL", buf.release, SHARED )    ;
	p_poolMGR->sysput( err, "ZOSVER", buf.version, SHARED )    ;
	p_poolMGR->sysput( err, "ZMACHINE", buf.machine, SHARED )  ;
	p_poolMGR->sysput( err, "ZENVIR", "lspf V0R0M1", SHARED )  ;
	p_poolMGR->sysput( err, "ZDATEF",  "DD/MM/YY", SHARED )    ;
	p_poolMGR->sysput( err, "ZDATEFD", "DD/MM/YY", SHARED )    ;
	p_poolMGR->sysput( err, "ZSCALE", "OFF", SHARED )          ;
	p_poolMGR->sysput( err, "ZSPLIT", "NO", SHARED )           ;
	p_poolMGR->sysput( err, "ZNULLS", "NO", SHARED )           ;

	p_poolMGR->setPOOLsReadOnly() ;
	GMAINPGM = p_poolMGR->sysget( err, "ZMAINPGM", PROFILE ) ;
}


void loadSystemCommandTable()
{
	// Terminate if ISPCMDS not found

	err.clear() ;

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

	err.clear() ;

	keyn = c - KEY_F( 0 ) ;
	key  = "ZPF" + d2ds( keyn, 2 ) ;
	val  = p_poolMGR->get( err, key, PROFILE ) ;
	if ( err.RC8() )
	{
		p_poolMGR->put( err, key, "", PROFILE ) ;
	}

	return val ;
}


void createpfKeyDefaults()
{
	err.clear() ;

	for ( int i = 1 ; i < 25 ; i++ )
	{
		p_poolMGR->put( err, "ZPF" + d2ds( i, 2 ), pfKeyDefaults[ i ], PROFILE ) ;
		if ( !err.RC0() )
		{
			llog( "E", "Error creating default key for task "<<err.taskid<<endl);
		}
	}
}


string ControlKeyAction( char c )
{
	// Translate the control-key to a command (stored in ZCTRLx system profile variables)

	err.clear() ;

	string s = keyname( c ) ;
	string k = "ZCTRL"      ;

	k.push_back( s[ 1 ] ) ;

	return p_poolMGR->sysget( err, k, PROFILE ) ;
}


void updateDefaultVars()
{
	err.clear() ;

	GMAXWAIT = ds2d( p_poolMGR->sysget( err, "ZMAXWAIT", PROFILE ) ) ;
	GMAINPGM = p_poolMGR->sysget( err, "ZMAINPGM", PROFILE ) ;
	p_poolMGR->sysput( err, "ZSPLIT", pLScreen::screensTotal > 1 ? "YES" : "NO", SHARED ) ;
}


void updateReflist()
{
	// Check if .NRET is ON and has a valid field name.  If so, add file to the reflist using
	// application ZRFLPGM, parmameters PLA plus the field entry value.  Run task in the background.

	// Don't update REFLIST if the application has done a CONTROL REFLIST NOUPDATE (flag ControlRefUpdate=false)
	// or ISPS PROFILE variable ZRFURL is not set to YES

	string fname = currAppl->get_nretfield() ;

	err.clear() ;

	if ( fname == "" || !currAppl->ControlRefUpdate || p_poolMGR->sysget( err, "ZRFURL", PROFILE ) != "YES" )
	{
		return ;
	}

	if ( currAppl->currPanel->field_valid( fname ) )
	{
		SELCT.clear() ;
		SELCT.PGM     = p_poolMGR->sysget( err, "ZRFLPGM", PROFILE ) ;
		SELCT.PARM    = "PLA " + currAppl->currPanel->field_getvalue( fname ) ;
		SELCT.NEWAPPL = ""    ;
		SELCT.NEWPOOL = false ;
		SELCT.PASSLIB = false ;
		startApplicationBack( SELCT ) ;
	}
	else
	{
		llog( "E", "Invalid field "+ fname +" in .NRET panel statement" << endl ) ;
		issueMessage( "PSYS011Z" ) ;
	}
}


void lineOutput()
{
	// Write line output to the display.  Split line if longer than screen width.

	int i ;

	string t ;

	if ( linePosn == -1 )
	{
		currScrn->save_panel_stack() ;
		currScrn->clear() ;
		linePosn = 0 ;
	}

	currScrn->show_busy() ;
	attrset( RED | A_BOLD ) ;
	t = currAppl->lineBuffer ;

	do
	{
		if ( linePosn == pLScreen::maxrow-1 )
		{
			lineOutput_end() ;
		}
		if ( t.size() > pLScreen::maxcol )
		{
			i = t.find_last_of( ' ', pLScreen::maxcol ) ;
			if ( i == string::npos ) { i = pLScreen::maxcol ; }
			else                     { i++                  ; }
			mvaddstr( linePosn++, 0, t.substr( 0, i ).c_str() ) ;
			t.erase( 0, i ) ;
		}
		else
		{
			mvaddstr( linePosn++, 0, t.c_str() ) ;
			t = "" ;
		}
	} while ( t.size() > 0 ) ;

	currScrn->OIA_update( screenNum, altScreen, boost::posix_time::microsec_clock::universal_time() ) ;

	wnoutrefresh( stdscr ) ;
	wnoutrefresh( OIA ) ;
	doupdate() ;
}


void lineOutput_end()
{
	attrset( RED | A_BOLD ) ;
	mvaddstr( linePosn, 0, "***" ) ;

	currScrn->show_enter() ;
	wnoutrefresh( stdscr ) ;
	wnoutrefresh( OIA ) ;
	move( linePosn, 3 ) ;
	doupdate() ;

	while ( true )
	{
		if ( isActionKey( getch() ) ) { break ; }
	}

	linePosn = 0 ;
	currScrn->clear() ;
	currScrn->clear_status() ;
	currScrn->OIA_startTime( boost::posix_time::microsec_clock::universal_time() ) ;
}


string listLogicalScreens()
{
	// Mainline lspf cannot create application panels but let's make this as similar as possible

	int i ;
	int m ;
	int o ;
	int c ;

	string ln  ;
	string t   ;
	string act ;
	string w2  ;

	err.clear() ;

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
		     left( appl->get_appname(), 13 ) +
		     left( appl->get_applid(), 8  )  +
		     left( t, 42 ) ;
		lslist.push_back( ln ) ;
	}

	o = m         ;
	curs_set( 0 ) ;
	while ( true )
	{
		for ( i = 0, it = lslist.begin() ; it != lslist.end() ; it++, i++ )
		{
			wattron( swwin, cuaAttr[  i == m ? PT : VOI ] ) ;
			mvwaddstr( swwin, i+4, 2, it->c_str() )         ;
			wattroff( swwin, cuaAttr[ i == m ? PT : VOI ] ) ;
		}
		update_panels() ;
		doupdate()  ;
		c = getch() ;
		if ( c >= CTRL( 'a' ) && c <= CTRL( 'z' ) )
		{
			act = ControlKeyAction( c ) ;
			iupper( act ) ;
			if ( word( act, 1 ) == "SWAP" )
			{
				w2 = word( act, 2 ) ;
				if ( w2 == "LISTN" ) { c = KEY_DOWN ; }
				else if ( w2 == "LISTP" ) { c = KEY_UP ; }
			}
		}
		if ( c == KEY_ENTER || c == 13 ) { break ; }
		if ( c == KEY_UP )
		{
			m == 0 ? m = lslist.size() - 1 : m-- ;
		}
		else if ( c == KEY_DOWN || c == CTRL( 'i' ) )
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
	currScrn->OIA_startTime( boost::posix_time::microsec_clock::universal_time() ) ;

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

	err.clear() ;
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
			mvwaddstr( rbwin, i+3, 3, it->c_str() )         ;
			wattroff( rbwin, cuaAttr[ i == m ? PT : VOI ] ) ;
		}
		update_panels() ;
		doupdate()  ;
		c = getch() ;
		if ( c == KEY_ENTER || c == 13 )
		{
			currAppl->currPanel->cmd_setvalue( err, retrieveBuffer[ m ] ) ;
			currAppl->currPanel->cursor_to_cmdfield( RC, retrieveBuffer[ m ].size()+1 ) ;
			break ;
		}
		else if ( c == KEY_UP )
		{
			m == 0 ? m = lslist.size() - 1 : m-- ;
		}
		else if ( c == KEY_DOWN || c == CTRL( 'i' ) )
		{
			m == lslist.size() - 1 ? m = 0 : m++ ;
		}
		else if ( isActionKey( c ) )
		{
			currAppl->currPanel->cursor_to_cmdfield( RC ) ;
			break ;
		}
	}

	del_panel( rbpanel ) ;
	delwin( rbwin )      ;
	curs_set( 1 )        ;

	currAppl->currPanel->get_cursor( row, col ) ;
	currScrn->set_cursor( row, col ) ;
	currScrn->OIA_startTime( boost::posix_time::microsec_clock::universal_time() ) ;

	return ;
}


void listBackTasks()
{
	// List background tasks and tasks that have been moved to the timeout queue

	llog( "-", "Listing background tasks:" << endl ) ;
	llog( "-", "         Number of tasks. . . . "<< pApplicationBackground.size()<< endl ) ;
	llog( "-", " "<< endl ) ;

	mtx.lock() ;
	for ( auto it = pApplicationBackground.begin() ; it != pApplicationBackground.end() ; it++ )
	{
		llog( "-", "         "<< setw(8) << (*it)->get_appname() <<
		      "   Id: "<< setw(5) << (*it)->taskid() <<
		      "   Status: "<< ( (*it)->terminateAppl ? "Terminated" : "Running" ) <<endl ) ;
	}
	mtx.unlock() ;

	llog( "-", " "<< endl ) ;
	llog( "-", "Listing timed-out tasks:" << endl ) ;
	llog( "-", "         Number of tasks. . . . "<< pApplicationTimeout.size()<< endl ) ;
	llog( "-", " "<< endl ) ;

	for ( auto it = pApplicationTimeout.begin() ; it != pApplicationTimeout.end() ; it++ )
	{
		llog( "-", "         "<< setw(8) << (*it)->get_appname() <<
		      "   Id: "<< setw(5) << (*it)->taskid() << endl ) ;
	}
	llog( "-", "*********************************************************************************"<<endl ) ;
}


void autoUpdate()
{
	// Resume application every 1s and wait.
	// Check every 50ms to see if ESC(27) has been pressed - read all characters from the buffer.

	uint row ;
	uint col ;

	char c ;

	bool end_auto = false ;

	currScrn->show_auto() ;
	nodelay( stdscr, true ) ;
	curs_set( 0 ) ;

	while ( !end_auto )
	{
		currScrn->OIA_startTime( boost::posix_time::microsec_clock::universal_time() ) ;
		ResumeApplicationAndWait() ;
		currScrn->OIA_update( screenNum, altScreen, boost::posix_time::microsec_clock::universal_time() ) ;
		wnoutrefresh( stdscr ) ;
		wnoutrefresh( OIA ) ;
		update_panels() ;
		doupdate() ;
		for ( int i = 0 ; i < 20 && !end_auto ; i++ )
		{
			boost::this_thread::sleep_for( boost::chrono::milliseconds( 50 ) ) ;
			do
			{
				c = getch() ;
				if ( c == CTRL( '[' ) ) { end_auto = true ; }
			} while ( c != -1 ) ;
		}
	}

	curs_set( 1 ) ;
	currScrn->clear_status() ;
	nodelay( stdscr, false ) ;

	currAppl->get_cursor( row, col ) ;
	currScrn->set_cursor( row, col ) ;
}


int getScreenNameNum( const string& s )
{
	// Return the screen number of screen name 's'.  If not found, return 0.
	// If a full match is not found, try to match an abbreviation.

	int i ;
	int j ;

	vector<pLScreen *>::iterator its ;
	pApplication * appl ;

	for ( i = 1, j = 0, its = screenList.begin() ; its != screenList.end() ; its++, i++ )
	{
		appl = (*its)->application_get_current() ;
		if ( appl->get_current_screenName() == s )
		{
			return i ;
		}
		else if ( abbrev( appl->get_current_screenName(), s ) )
		{
			j = i ;
		}
	}

	return j ;
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
			lc.reply.push_back( appl->get_appname()   )    ;
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
	lg->close() ;
	abort() ;
}


void abnormalTermMessage()
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
		t = "Failing application is " + currAppl->get_appname() + ", taskid=" + d2ds( currAppl->taskid() ) ;
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

	linePosn = -1 ;
	currScrn->restore_panel_stack() ;
}


void issueMessage( const string& msg )
{
	err.clear() ;

	currAppl->save_errblock() ;
	currAppl->set_msg( msg ) ;
	err = currAppl->get_errblock() ;
	currAppl->restore_errblock()   ;
	if ( !err.RC0() )
	{
		errorScreen( 1, "Syntax error in message "+ msg +", message file or message not found" ) ;
	}
}


void serviceCallError( errblock& err )
{
	llog( "E", "A Serive Call error has occured"<< endl ) ;

	llog( "E", "Error msg  : "<< err.msg1 << endl )  ;
	llog( "E", "Error id   : "<< err.msgid << endl ) ;
	llog( "E", "Error RC   : "<< err.getRC() << endl ) ;
	llog( "E", "Error ZVAL1: "<< err.val1 << endl )  ;
	llog( "E", "Error ZVAL2: "<< err.val2 << endl )  ;
	llog( "E", "Error ZVAL3: "<< err.val3 << endl )  ;
	llog( "E", "Source     : "<< err.getsrc() << endl ) ;
}


bool isActionKey( int c )
{
	return ( ( c >= KEY_F(1)    && c <= KEY_F(24) )   ||
		 ( c >= CTRL( 'a' ) && c <= CTRL( 'z' ) ) ||
		   c == CTRL( '[' ) ||
		   c == KEY_NPAGE   ||
		   c == KEY_PPAGE   ||
		   c == KEY_ENTER   ||
		   c == 13        ) ;
}


void actionSwap( uint& row, uint& col )
{
	int RC ;
	int i  ;
	int l  ;

	string w1 ;
	string w2 ;

	w1 = word( ZPARM, 1 ) ;
	w2 = word( ZPARM, 2 ) ;

	if ( ZPARM != "" && ZPARM != "NEXT" && ZPARM != "PREV" )
	{
		currAppl->currPanel->cursor_to_cmdfield( RC ) ;
		currAppl->currPanel->get_cursor( row, col ) ;
		currScrn->set_cursor( row, col ) ;
	}

	if ( findword( w1, "LIST LISTN LISTP" ) )
	{
		w1 = listLogicalScreens() ;
	}

	if ( currAppl->msg_issued_with_cmd() )
	{
		currAppl->clear_msg() ;
	}

	if ( pLScreen::screensTotal == 1 ) { return ; }

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
		screenNum++ ;
		screenNum = (screenNum == pLScreen::screensTotal ? 0 : screenNum) ;
		if ( altScreen == screenNum )
		{
			altScreen = (altScreen == 0 ? (pLScreen::screensTotal - 1) : (altScreen - 1) ) ;
		}
	}
	else if ( w1 == "PREV" )
	{
		screenNum-- ;
		screenNum = (screenNum < 0 ? (pLScreen::screensTotal - 1) : screenNum) ;
		if ( altScreen == screenNum )
		{
			altScreen = ((altScreen == pLScreen::screensTotal - 1) ? 0 : (altScreen + 1) ) ;
		}
	}
	else if ( datatype( w1, 'W' ) )
	{
		i = ds2d( w1 ) - 1 ;
		if ( i >= 0 && i < pLScreen::screensTotal )
		{
			if ( i != screenNum )
			{
				if ( w2 == "*" || i == altScreen )
				{
					altScreen = screenNum ;
				}
				screenNum = i ;
			}
		}
		else
		{
			swap( screenNum, altScreen ) ;
		}
		if ( datatype( w2, 'W' ) && w1 != w2 )
		{
			i = ds2d( w2 ) - 1 ;
			if ( i != screenNum && i >= 0 && i < pLScreen::screensTotal )
			{
				altScreen = i ;
			}
		}
	}
	else
	{
		swap( screenNum, altScreen ) ;
	}

	currScrn = screenList[ screenNum ] ;
	currScrn->OIA_startTime( boost::posix_time::microsec_clock::universal_time() ) ;
	currAppl = currScrn->application_get_current() ;

	err.settask( currAppl->taskid() ) ;
	p_poolMGR->put( err, "ZPANELID", currAppl->panelid, SHARED, SYSTEM )  ;
	currScrn->restore_panel_stack() ;
	currAppl->display_pd( err ) ;
}


void actionTabKey( uint& row, uint& col )
{
	// Tab processsing:
	//     If a pull down is active, go to next pulldown
	//     If cursor on a field that supports field execution and is not on the first char, execute function
	//     Else act as a tab key to the next input field

	uint rw ;
	uint cl ;

	bool tab_next = true ;

	string field_name ;

	fieldExc fxc ;

	if ( currAppl->currPanel->pd_active() )
	{
		displayNextPullDown( row, col ) ;
	}
	else
	{
		field_name = currAppl->currPanel->field_getname( row, col ) ;
		if ( field_name != "" )
		{
			currAppl->currPanel->field_get_row_col( field_name, rw, cl ) ;
			if ( rw == row && cl < col )
			{
				fxc = currAppl->currPanel->field_getexec( field_name ) ;
				if ( fxc.fieldExc_command != "" )
				{
					executeFieldCommand( field_name, fxc, col ) ;
					tab_next = false ;
				}
			}
		}
		if ( tab_next )
		{
			currAppl->currPanel->field_tab_next( row, col ) ;
			currScrn->set_cursor( row, col ) ;
		}
	}
}


void displayNextPullDown( uint& row, uint& col )
{
	string msg ;

	currAppl->currPanel->display_next_pd( err, msg ) ;
	if ( err.error() )
	{
		errorScreen( 1, "Error processing pull-down menu." ) ;
		serviceCallError( err ) ;
		currAppl->get_home( row, col ) ;
	}
	else
	{
		if ( msg != "" )
		{
			issueMessage( msg ) ;
		}
		currAppl->currPanel->get_cursor( row, col ) ;
	}
	currScrn->set_cursor( row, col ) ;
}


void executeFieldCommand( const string& field_name, const fieldExc& fxc, uint col )
{
	// Run application associated with a field when tab pressed or command FIELDEXC entered

	// Cursor position is stored in shared variable ZFECSRP
	// Data to be passed to the application (fieldExc_passed) are stored in shared vars ZFEDATAn

	int i  ;
	int ws ;

	uint cl ;

	string w1 ;

	if ( !SELCT.parse( err, subword( fxc.fieldExc_command, 2 ) ) )
	{
		llog( "E", "Error in FIELD SELECT command "+ fxc.fieldExc_command << endl ) ;
		issueMessage( "PSYS011K" ) ;
		return ;
	}

	SELCT.PARM += " " + currAppl->currPanel->field_getvalue( field_name ) ;
	currAppl->reffield = field_name ;
	currAppl->currPanel->field_get_col( field_name, cl ) ;
	p_poolMGR->put( err, "ZFECSRP", d2ds( col - cl + 1 ), SHARED )  ;

	for( ws = words( fxc.fieldExc_passed ), i = 1 ; i <= ws ; i++ )
	{
		w1 = word( fxc.fieldExc_passed, i ) ;
		p_poolMGR->put( err, "ZFEDATA" + d2ds( i ), currAppl->currPanel->field_getvalue( w1 ), SHARED ) ;
	}

	startApplication( SELCT ) ;
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

	err.clear() ;
	appInfo aI  ;

	const string e1( GMAINPGM +" not found.  Check ZLDPATH is correct.  lspf terminating **" ) ;

	paths = p_poolMGR->sysget( err, "ZLDPATH", PROFILE ) ;
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

	err.clear()  ;

	typedef vector<path> vec ;
	vec v      ;
	appInfo aI ;

	vec::const_iterator it ;

	paths = p_poolMGR->sysget( err, "ZLDPATH", PROFILE ) ;
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
			aI.mainpgm     = false ;
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
