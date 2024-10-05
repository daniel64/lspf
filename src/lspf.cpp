/* Compile with comp1 */

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
#include <sys/utsname.h>
#include <arpa/inet.h>

#include <locale>
#include <boost/date_time.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/circular_buffer.hpp>
#include <algorithm>

boost::condition cond_appl ;
boost::condition cond_lspf ;
boost::condition cond_batch ;

#include "utilities.h"
#include "utilities.cpp"

#include "colours.h"

#include "classes.h"
#include "classes.cpp"

#include "pVPOOL.h"
#include "pVPOOL.cpp"

#include "pGLServ.h"
#include "pGLServ.cpp"

#include "pLSServ.h"
#include "pLSServ.cpp"

#include "pWidgets.h"
#include "pWidgets.cpp"

#include "pTable.h"
#include "pTable.cpp"

#include "pPanel.h"
#include "pFTailor.h"
#include "pApplication.h"

#include "pLScreen.h"

#include "pPanel1.cpp"
#include "pPanel2.cpp"

#include "pFTailor.cpp"

#include "pApplication.cpp"

#include "ispexeci.cpp"

#include "pLScreen.cpp"

#define currScrn pLScreen::currScreen
#define OIA      pLScreen::OIA

#define CTRL(c) ((c) & 037)


/* These codes are > KEY_MAX and so are not         */
/* guaranteed to be stable.  Set using keyname().   */
#define kDC3     KEY_MAX + 1   /* alt delete                 */
#define kDC5     KEY_MAX + 2   /* control delete             */
#define kDC6     KEY_MAX + 3   /* shifted control delete     */
#define kDN3     KEY_MAX + 4   /* alt down                   */
#define kDN5     KEY_MAX + 5   /* control down               */
#define kDN6     KEY_MAX + 6   /* shifted control down       */
#define kEND5    KEY_MAX + 7   /* control End                */
#define kEND6    KEY_MAX + 8   /* shifted control End        */
#define kLFT5    KEY_MAX + 9   /* control left               */
#define kNXT5    KEY_MAX + 10  /* control page-down          */
#define kPRV5    KEY_MAX + 11  /* control page-up            */
#define kRIT5    KEY_MAX + 12  /* control right              */
#define kUP3     KEY_MAX + 13  /* alt up                     */
#define kUP5     KEY_MAX + 14  /* control up                 */
#define kUP6     KEY_MAX + 15  /* shifted control up         */
#define kxIN     KEY_MAX + 16  /* focus-in                   */
#define kxOUT    KEY_MAX + 17  /* focus-out                  */
#define kHOM3    KEY_MAX + 18  /* alt home                   */
#define kHOM5    KEY_MAX + 19  /* control home               */

namespace {

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

pApplication* currAppl ;

poolMGR*  p_poolMGR  = new poolMGR  ;
tableMGR* p_tableMGR = new tableMGR( SWIND ) ;
gls*      p_gls      = new gls      ;
logger*   lg         = new logger   ;
logger*   lgx        = new logger   ;

fPOOL funcPOOL ;

vector<pLScreen*> screenList ;

struct appInfo
{
	string file       ;
	string module     ;
	void* dlib        ;
	void* maker_ep    ;
	void* destroyer_ep ;
	bool  mainpgm     ;
	bool  dlopened    ;
	bool  errors      ;
	bool  relPending  ;
	int   refCount    ;
} ;


map<string, appInfo> apps ;

boost::circular_buffer<string> retrieveBuffer( 99 ) ;
vector<string> keepBuffer ;

int  linePosn  = 0 ;
int  maxTaskid = 0 ;
uint retPos    = 0 ;
uint priScreen = 0 ;
uint altScreen = 0 ;
uint intens    = 0 ;

string ctlAction ;
string commandStack ;
string jumpOption ;
string returnOption ;

std::streambuf* old_cout_buf ;

vector<pApplication*> pApplicationBackground ;
vector<pApplication*> pApplicationTimeout ;

errblock err ;

string zcommand ;
string zparm ;

string zctverb ;
string zcttrunc ;
string zctact ;
string zctdesc ;

int    taskid()  { return 0 ;      }
string modname() { return "lspf" ; }

unsigned int decColour1 ;
unsigned int decColour2 ;
unsigned int decIntens  ;

const char zscreen[] = { '1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G',
			 'H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W' } ;

string gmainpgm ;

enum LSPF_STATUS
{
	LSPF_STARTING,
	LSPF_RUNNING,
	LSPF_STOPPING
} ;

enum BACK_STATUS
{
	BACK_RUNNING,
	BACK_STOPPING,
	BACK_STOPPED
} ;

enum ZCT_VERBS
{
	ZCT_ACTIONS,
	ZCT_ALIAS,
	ZCT_CANCEL,
	ZCT_CRETRIEV,
	ZCT_CURSOR,
	ZCT_EXIT,
	ZCT_NOP,
	ZCT_PASSTHRU,
	ZCT_RETF,
	ZCT_RETK,
	ZCT_RETP,
	ZCT_RETRIEVE,
	ZCT_SCRNAME,
	ZCT_SELECT,
	ZCT_SETVERB,
	ZCT_SPLIT,
	ZCT_SWAP,
	ZCT_USERID,
	ZCT_WINDOW
} ;

enum ZCMD_TYPES
{
	ZC_FIELDEXC,
	ZC_MSGID,
	ZC_NOP,
	ZC_PANELID,
	ZC_RESIZE,
	ZC_SWAPBAR,
	ZC_TDOWN,
	ZC_DOT_ABEND,
	ZC_DOT_AUTO,
	ZC_DOT_DEBUG,
	ZC_DOT_DISCARD,
	ZC_DOT_ENQ,
	ZC_DOT_HIDE,
	ZC_DOT_INFO,
	ZC_DOT_LOAD,
	ZC_DOT_RELOAD,
	ZC_DOT_REFRESH,
	ZC_DOT_RGB,
	ZC_DOT_SCALE,
	ZC_DOT_SHELL,
	ZC_DOT_SHOW,
	ZC_DOT_SNAP,
	ZC_DOT_STATS,
	ZC_DOT_TASKS,
} ;

map<string, ZCT_VERBS> zverbs =
{ { "ACTIONS",  ZCT_ACTIONS  },
  { "ALIAS",    ZCT_ALIAS    },
  { "CANCEL",   ZCT_CANCEL   },
  { "CURSOR",   ZCT_CURSOR   },
  { "CRETRIEV", ZCT_CRETRIEV },
  { "EXIT",     ZCT_EXIT     },
  { "NOP",      ZCT_NOP      },
  { "PASSTHRU", ZCT_PASSTHRU },
  { "RETF",     ZCT_RETF     },
  { "RETK",     ZCT_RETK     },
  { "RETP",     ZCT_RETP     },
  { "RETRIEVE", ZCT_RETRIEVE },
  { "SCRNAME",  ZCT_SCRNAME  },
  { "SELECT",   ZCT_SELECT   },
  { "SETVERB",  ZCT_SETVERB  },
  { "SPLIT",    ZCT_SPLIT    },
  { "SWAP",     ZCT_SWAP     },
  { "USERID",   ZCT_USERID   },
  { "WINDOW",   ZCT_WINDOW   } } ;

map<string, ZCMD_TYPES> zcommands =
{ { "FIELDEXC", ZC_FIELDEXC    },
  { "MSGID",    ZC_MSGID       },
  { "NOP",      ZC_NOP         },
  { "PANELID",  ZC_PANELID     },
  { "RESIZE",   ZC_RESIZE      },
  { "SWAPBAR",  ZC_SWAPBAR     },
  { "TDOWN",    ZC_TDOWN       },
  { ".ABEND",   ZC_DOT_ABEND   },
  { ".AUTO",    ZC_DOT_AUTO    },
  { ".DEBUG",   ZC_DOT_DEBUG   },
  { ".DISCARD", ZC_DOT_DISCARD },
  { ".ENQ",     ZC_DOT_ENQ     },
  { ".HIDE",    ZC_DOT_HIDE    },
  { ".INFO",    ZC_DOT_INFO    },
  { ".LOAD",    ZC_DOT_LOAD    },
  { ".RELOAD",  ZC_DOT_RELOAD  },
  { ".REFRESH", ZC_DOT_REFRESH },
  { ".RGB",     ZC_DOT_RGB     },
  { ".SCALE",   ZC_DOT_SCALE   },
  { ".SHELL",   ZC_DOT_SHELL   },
  { ".SHOW",    ZC_DOT_SHOW    },
  { ".SNAP",    ZC_DOT_SNAP    },
  { ".STATS",   ZC_DOT_STATS   },
  { ".TASKS",   ZC_DOT_TASKS   },
  { ".JOBS",    ZC_DOT_TASKS   } } ;

LSPF_STATUS lspfStatus ;
BACK_STATUS backStatus ;

boost::recursive_mutex mtx ;

WINDOW* lwin   = nullptr ;
PANEL*  lpanel = nullptr ;

}

unsigned int pLScreen::screensTotal = 0 ;
unsigned int pLScreen::maxScreenId  = 0 ;
unsigned int pLScreen::maxrow       = 0 ;
unsigned int pLScreen::maxcol       = 0 ;
unsigned int pLScreen::maxscrn      = ZMAXSCRN ;

unsigned int lss::ndataid        = 0 ;
unsigned int gls::ispddn         = 19999 ;
unsigned int gls::sysddn         = 10000 ;

bool pLScreen::busy       = false ;
bool pLScreen::w_refresh  = false ;

set<uint> pLScreen::screenNums ;
map<uint,uint> pLScreen::openedByList ;
boost::posix_time::ptime pLScreen::startTime ;
boost::posix_time::ptime pLScreen::endTime ;

WINDOW* pLScreen::OIA       = nullptr ;
PANEL*  pLScreen::OIA_panel = nullptr ;

WINDOW* pLScreen::SWB       = nullptr ;
PANEL*  pLScreen::SWB_panel = nullptr ;

pLScreen* pLScreen::currScreen = nullptr ;

pApplication* pApplication::self = nullptr ;

std::stringstream pApplication::cout_buffer ;
std::stringstream pApplication::cerr_buffer ;

tableMGR* pApplication::p_tableMGR = nullptr ;
tableMGR* pFTailor::p_tableMGR     = nullptr ;

poolMGR*  pApplication::p_poolMGR  = nullptr ;

gls*   pApplication::p_gls  = nullptr ;
gls*   lss::p_gls           = nullptr ;

logger*   pApplication::lg  = nullptr ;

logger*   trace_logger::lg  = nullptr ;

poolMGR*  pPanel::p_poolMGR = nullptr ;

poolMGR*  pFTailor::p_poolMGR = nullptr ;

poolMGR*  abc::p_poolMGR = nullptr ;

logger*   pPanel::lg     = nullptr ;

logger*   pFTailor::lg   = nullptr ;

logger*   tableMGR::lg   = nullptr ;

logger*   poolMGR::lg    = nullptr ;

logger*   lss::lg = nullptr ;

logger*   gls::lg = nullptr ;

bool   pApplication::controlNonDispl = false ;
bool   pApplication::controlDisplayLock = false ;
bool   pApplication::controlSplitEnable = true ;
bool   pApplication::lineInOutDone = false ;
uint   pApplication::pflgToken = 1 ;

map<int, void*>pApplication::ApplUserData ;

vector<string>pApplication::notifies ;

uint   pVPOOL::pfkgToken = 0 ;
uint   Table::pflgToken  = 1 ;

char   field::field_paduchar = ' ' ;
bool   field::field_nulls    = false ;
uint   pPanel::panel_intens  = 0 ;
uint   pPanel::pfkgToken     = 1 ;
bool   pPanel::tabpas        = false ;
uint   pLScreen::lscreen_intens = 0  ;
bool   pLScreen::swapbar     = false ;
uint   field::field_intens   = 0 ;
uint   text::text_intens     = 0 ;
uint   pdc::pdc_intens       = 0 ;
uint   abc::abc_intens       = 0 ;
uint   Box::box_intens       = 0 ;

void setGlobalClassVars() ;
void initialSetup() ;
void loadDefaultPools() ;
void createSystemAllocs() ;
void createSystemAlloc( const string&, const string& = "" ) ;
void setDefaultRGB() ;
void resizeTerminal() ;
void getModuleEntries ( vector<path>& ) ;
void getDynamicClasses() ;
bool loadDynamicClass( const string& ) ;
bool unloadDynamicClass( void* ) ;
void reloadDynamicClasses( string ) ;
void processArgs( int, char** ) ;
void loadSystemCommandTable() ;
void loadRetrieveBuffer() ;
void saveRetrieveBuffer() ;
void cleanup() ;
void loadCUATables() ;
void setGlobalColours() ;
void setRGBValues() ;
void setDecolourisedColour() ;
void decolouriseScreen() ;
void decolouriseAll() ;
void colouriseAll() ;
void setColourPair( const string& ) ;
void lScreenDefaultSettings() ;
void updateDefaultVars() ;
void ncursesUpdate( uint, uint ) ;
void createSharedPoolVars( const string& ) ;
void updateReflist() ;
void updateHistory() ;
void actionMouseEvent( MEVENT*, int&, uint&, uint&, bool& ) ;
void processMouseAction( string&, MEVENT*, int&, uint&, uint& ) ;
void startApplication( selobj&, bool =false, bool = false ) ;
void checkStartApplication( const selobj& )   ;
void startApplicationBack( selobj, pApplication*, bool = true ) ;
void terminateApplication() ;
void terminateApplicationBack( pApplication* ) ;
void abnormalTermMessage() ;
void processBackgroundTasks() ;
void ResumeApplicationAndWait() ;
bool createLogicalScreen() ;
void deleteLogicalScreen() ;
void resolvePGM( selobj&, pApplication* ) ;
void processPGMSelect() ;
void processZSEL() ;
void processAction( selobj&, uint, uint, int, bool&, bool&, bool&, bool&, bool& ) ;
void processZCOMMAND( selobj&, uint, uint, bool ) ;
void issueMessage( const string&, const string& = "" ) ;
void lineOutput() ;
void lineOutput_end() ;
void lineInput() ;
void convNonDisplayChars( string& ) ;
void updateScreenText( set<uint>&, uint, uint ) ;
uint getTextLength( uint ) ;
string getScreenText( uint ) ;
string getDateFormat() ;
string getSWBText() ;
void programTerminated() ;
void createLinePanel() ;
void deleteLinePanel() ;
void errorScreen( int, const string& ) ;
void errorScreen( const string&, const string& ) ;
void errorScreen( const string& ) ;
void abortStartup() ;
void lspfCallbackHandler( lspfCommand& ) ;
void createpfKeyDefaults() ;
string getEnvironmentVariable( const char* ) ;
void checkSystemVariable( const string& ) ;
string controlKeyAction( char c ) ;
string listLogicalScreens() ;
int  listInterruptOptions() ;
void listApplicationStatus() ;
void addRetrieveBuffer( const string&, const string&, bool = false ) ;
void actionSwap( const string& ) ;
void actionTabKey( uint&, uint&, bool& ) ;
void displayNextPullDown( const string& = "" ) ;
void displayPrevPullDown( const string& = "" ) ;
void executeFieldCommand( const string&, const fieldXct&, uint ) ;
void executeHistoryCommand( const string&, const string&, uint, uint, uint, uint ) ;
void listBackTasks() ;
void listTaskVector( vector<pApplication*>& p ) ;
void autoUpdate() ;
void screenRefresh() ;
bool resolveZCTEntry( string&, string& ) ;
bool isActionKey( int c ) ;
bool isEscapeKey() ;
void listRetrieveBuffer( char, string ) ;
void listRetrieveBuffer_load_tBuffer( vector<string>&, char, const string& ) ;
void listRetrieveBuffer_build( vector<string>&, vector<pair<string, uint>>&, char, uint&, WINDOW*&, PANEL*&, bool& ) ;
int  getScreenNameNum( const string& ) ;
void serviceCallError( errblock& ) ;
void listErrorBlock( errblock& ) ;
void displayNotifies() ;
void mainLoop() ;


int main( int argc,
	  char** argv )
{

	selobj selct ;

	old_cout_buf = std::cout.rdbuf() ;

	std::cout.rdbuf( pApplication::cout_buffer.rdbuf() ) ;
	std::cerr.rdbuf( pApplication::cerr_buffer.rdbuf() ) ;

	std::cout << std::unitbuf ;
	std::cerr << std::unitbuf ;

	setGlobalClassVars() ;

	lg->open() ;
	lgx->open() ;

	err.clear() ;

	boost::thread* pThread ;
	boost::thread* bThread ;

	commandStack = "" ;
	lspfStatus   = LSPF_STARTING ;
	backStatus   = BACK_STOPPED ;
	err.settask( 1 ) ;

	llog( "I", "lspf version " LSPF_VERSION " startup in progress." << endl ) ;

#ifdef HAS_REXX_SUPPORT
	llog( "I", "Panel and file tailoring REXX support has been enabled."<< endl ) ;
#else
	llog( "I", "Panel and file tailoring REXX support has been disabled."<< endl ) ;
#endif

	screenList.push_back( new pLScreen( 0 ) ) ;

	pLScreen::OIA_startTime() ;

	llog( "I", "First logical screen created and ncurses initialised." << endl ) ;
	llog( "I", "TERM environment variable is set to " << getEnvironmentVariable( "TERM" ) << "." << endl ) ;
	llog( "I", "Terminal supports "<< COLORS <<" colours and "<< COLOR_PAIRS <<" colour pairs."<< endl ) ;
	llog( "I", "Terminal " << ( ( can_change_color() ) ? "can" : "cannot" ) << " change colour content."<< endl ) ;
	llog( "I", "Screen size is "<<pLScreen::maxcol <<"x"<<pLScreen::maxrow << "." << endl ) ;

	mousemask( ALL_MOUSE_EVENTS, nullptr ) ;
	llog( "I", "Mouse driver has " << ( ( has_mouse() ) ? "been" : "not been" ) << " initialised."<< endl ) ;

	llog( "I", "Calling initialSetup." << endl ) ;
	initialSetup() ;

	llog( "I", "Calling loadDefaultPools." << endl ) ;
	loadDefaultPools() ;

	llog( "I", "Creating system allocations." << endl ) ;
	createSystemAllocs() ;

	lspfStatus = LSPF_RUNNING ;

	llog( "I", "Setting default RGB colour values." << endl ) ;
	setDefaultRGB() ;

	llog( "I", "Starting background job monitor task." << endl ) ;
	bThread = new boost::thread( &processBackgroundTasks ) ;

	lScreenDefaultSettings() ;

	llog( "I", "Calling getDynamicClasses." << endl ) ;
	getDynamicClasses() ;

	llog( "I", "Loading main "+ gmainpgm +" application." << endl ) ;
	if ( not loadDynamicClass( gmainpgm ) )
	{
		llog( "S", "Main program "+ gmainpgm +" cannot be loaded or symbols resolved." << endl ) ;
		cleanup() ;
		delete bThread ;
		return 0  ;
	}

	llog( "I", "Calling loadCUATables." << endl ) ;
	loadCUATables() ;

	llog( "I", "Calling loadSystemCommandTable." << endl ) ;
	loadSystemCommandTable() ;

	loadRetrieveBuffer() ;

	updateDefaultVars() ;

	llog( "I", "Startup complete.  Starting first "+ gmainpgm +" thread." << endl ) ;
	currAppl = ((pApplication*(*)())( apps[ gmainpgm ].maker_ep ))() ;

	currScrn->application_add( currAppl ) ;

	selct.def( gmainpgm ) ;

	currAppl->init_phase1( selct, ++maxTaskid, nullptr, lspfCallbackHandler ) ;
	currAppl->shrdPool = 1 ;
	currAppl->p_lss = currScrn->get_lss() ;

	p_poolMGR->createProfilePool( err, "ISR" ) ;
	if ( err.error() )
	{
		llog( "S", "Loading of profile ISRPROF failed.  RC="<< err.getRC() << "." << endl ) ;
		llog( "S", "Aborting startup.  Check profile pool path." << endl ) ;
		listErrorBlock( err ) ;
		abortStartup() ;
	}

	p_poolMGR->connect( currAppl->taskid(), "ISR", 1 ) ;
	if ( err.RC4() ) { createpfKeyDefaults() ; }

	createSharedPoolVars( "ISR" ) ;
	currAppl->init_phase2() ;

	pThread = new boost::thread( &pApplication::run, currAppl ) ;
	currAppl->pThread = pThread ;
	apps[ gmainpgm ].refCount++ ;
	apps[ gmainpgm ].mainpgm = true ;

	llog( "I", "Waiting for "+ gmainpgm +" to complete startup." << endl ) ;
	boost::mutex mutex ;
	while ( currAppl->busyAppl )
	{
		boost::mutex::scoped_lock lk( mutex ) ;
		cond_lspf.wait_for( lk, boost::chrono::milliseconds( 100 ) ) ;
		lk.unlock() ;
	}
	if ( currAppl->terminateAppl )
	{
		errorScreen( 1, "An error has occured initialising the first "+ gmainpgm +" main task." ) ;
		llog( "S", "Main program "+ gmainpgm +" failed to initialise." << endl ) ;
		currAppl->info() ;
		p_poolMGR->disconnect( currAppl->taskid() ) ;
		llog( "I", "Removing application instance of "+ currAppl->get_appname() << "." << endl ) ;
		((void (*)(pApplication*))(apps[ currAppl->get_appname() ].destroyer_ep))( currAppl ) ;
		delete pThread ;
		cleanup() ;
		delete bThread ;
		return 0  ;
	}

	llog( "I", "First thread "+ gmainpgm +" started and initialised.  ID=" << pThread->get_id() << "." << endl ) ;

	currScrn->set_cursor( currAppl ) ;
	pLScreen::OIA_endTime() ;

	processArgs( argc, argv ) ;

	mainLoop() ;

	saveRetrieveBuffer() ;

	llog( "I", "Stopping background job monitor task." << endl ) ;
	lspfStatus = LSPF_STOPPING ;
	backStatus = BACK_STOPPING ;
	cond_batch.notify_one()    ;
	while ( backStatus != BACK_STOPPED )
	{
		boost::this_thread::sleep_for( boost::chrono::milliseconds( 5 ) ) ;
	}

	delete bThread ;

	llog( "I", "Closing ISPS profile as last application program has terminated." << endl ) ;
	p_poolMGR->destroySystemPool( err ) ;

	p_poolMGR->statistics()  ;
	p_tableMGR->statistics() ;

	delete p_poolMGR  ;
	delete p_tableMGR ;
	delete p_gls      ;

	llog( "I", "Closing application log." << endl ) ;
	delete lgx ;

	for ( auto& app : apps )
	{
		if ( app.second.dlopened )
		{
			llog( "I", "dlclose of "+ app.first +" at " << app.second.dlib << "." << endl ) ;
			unloadDynamicClass( app.second.dlib ) ;
		}
	}

	llog( "I", "lspf and LOG terminating." << endl ) ;
	delete lg ;

	return 0  ;
}


void cleanup()
{
	//
	// Cleanup resources for early termination and inform the user, including log names.
	//

	delete p_poolMGR ;
	delete p_tableMGR ;
	delete p_gls ;
	delete currScrn ;

	llog( "I", "Stopping background job monitor task." << endl ) ;
	lspfStatus = LSPF_STOPPING ;
	backStatus = BACK_STOPPING ;
	cond_batch.notify_one()    ;
	while ( backStatus != BACK_STOPPED )
	{
		boost::this_thread::sleep_for( boost::chrono::milliseconds( 5 ) ) ;
	}

	std::cout.rdbuf( old_cout_buf ) ;

	cout << "********************************************************************* " << endl ;
	cout << "********************************************************************* " << endl ;
	cout << "Aborting startup of lspf.  Check lspf and application logs for errors." << endl ;
	cout << "lspf log name. . . . . :"<< lg->logname() << endl ;
	cout << "Application log name . :"<< lgx->logname() << endl ;
	cout << "********************************************************************* " << endl ;
	cout << "********************************************************************* " << endl ;
	cout << endl ;

	llog( "I", "lspf and LOG terminating." << endl ) ;
	delete lgx ;
	delete lg  ;
}


void processArgs( int argc,
		  char** argv )
{
	//
	// Setup the initial command stack from command line arguments.
	//

	size_t p1 ;

	string delm ;
	string var = "ZSTART" ;

	if ( argc == 2 )
	{
		var = upper( argv[ 1 ] ) ;
		if ( !isvalidName( var ) )
		{
			commandStack = argv[ 1 ] ;
			return ;
		}
		else if ( var == "BASIC" )
		{
			return ;
		}
	}
	else if ( argc > 2 )
	{
		for ( int i = 1 ; i < argc ; ++i )
		{
			commandStack += argv[ i ] ;
			commandStack += " " ;
		}
		return ;
	}

	commandStack = strip( p_poolMGR->get( err, var, PROFILE ) ) ;
	llog( "I", "Using variable "+ var +" to set the initial command stack.  Value is '"+ commandStack +"'" << "." << endl ) ;

	if ( commandStack == "" )
	{
		commandStack = ( argc == 2 ) ? argv[ 1 ] : "" ;
		return ;
	}

	delm = p_poolMGR->sysget( err, "ZDEL", PROFILE ) ;
	p1   = commandStack.find_first_of( delm.front() ) ;

	if ( strip( commandStack.substr( 0, p1 ) ) != "lspf" )
	{
		llog( "W", "Variable "+ var +" contains an invalid value.  Ignoring parameter." << endl ) ;
		commandStack = "" ;
	}
	else
	{
		commandStack.erase( 0, p1+1 ) ;
	}
}


void mainLoop()
{
	llog( "I", "mainLoop() entered." << endl ) ;

	int c ;

	uint row ;
	uint col ;

	bool passthru ;
	bool doSelect ;
	bool pdchoice ;
	bool pfkeyPressed  ;
	bool ctlkeyPressed ;

	bool showLock  = false ;
	bool Insert    = false ;
	bool wmPending = false ;

	selobj selct ;

	MEVENT event ;

	err.clear()  ;

	pLScreen::OIA_setup() ;

	if ( commandStack != "" ) { pLScreen::set_busy() ; }

	set_escdelay( 25 ) ;

	map<string, int> keymap = { { "kDC3",  kDC3  },
				    { "kDC5",  kDC5  },
				    { "kDC6",  kDC6  },
				    { "kDN3",  kDN3  },
				    { "kDN5",  kDN5  },
				    { "kDN6",  kDN6  },
				    { "kEND5", kEND5 },
				    { "kEND6", kEND6 },
				    { "kLFT5", kLFT5 },
				    { "kNXT5", kNXT5 },
				    { "kPRV5", kPRV5 },
				    { "kRIT5", kRIT5 },
				    { "kUP3",  kUP3  },
				    { "kUP5",  kUP5  },
				    { "kUP6",  kUP6  },
				    { "kxIN",  kxIN  },
				    { "kxOUT", kxOUT },
				    { "kHOM3", kHOM3 },
				    { "kHOM5", kHOM5 }, } ;

	while ( pLScreen::screensTotal > 0 )
	{
		pLScreen::clear_status() ;
		currScrn->get_cursor( row, col ) ;

		pfkeyPressed  = false ;
		ctlkeyPressed = false ;
		ctlAction     = "" ;

		if ( commandStack == ""                &&
		     !pApplication::controlDisplayLock &&
		     !pApplication::controlNonDispl    &&
		     !pApplication::lineInOutDone )
		{
			currAppl->display_setmsg() ;
			currScrn->OIA_update( priScreen, altScreen, showLock ) ;
			pLScreen::clear_busy() ;
			ncursesUpdate( row, col ) ;
			c = getch() ;
			if ( c == CTRL( 'M' ) ) { c = KEY_ENTER ; }
		}
		else
		{
			if ( pApplication::controlDisplayLock && not pApplication::lineInOutDone )
			{
				pLScreen::clear_busy() ;
				currScrn->OIA_update( priScreen, altScreen, showLock ) ;
				currAppl->display_setmsg() ;
				ncursesUpdate( row, col ) ;
			}
			c = KEY_ENTER ;
		}

		showLock = false ;

		if ( c < 256 && isprint( c ) )
		{
			if ( currAppl->currPanel->pd_active() )
			{
				if ( std::isdigit( char( c ) ) )
				{
					displayNextPullDown( ". " + string( 1, toupper( char( c ) ) ) ) ;
				}
				else
				{
					displayNextPullDown( string( 1, toupper( char( c ) ) ) ) ;
				}
				continue ;
			}
			if ( currAppl->inputInhibited() ) { continue ; }
			currAppl->currPanel->field_edit( row, col, char( c ), Insert, showLock ) ;
			currScrn->set_cursor( currAppl ) ;
			continue ;
		}

		if ( c == KEY_MOUSE && getmouse( &event ) == OK )
		{
			actionMouseEvent( &event, c, row, col, wmPending ) ;
		}

		if ( c >= CTRL( 'a' ) && c <= CTRL( 'z' ) && c != CTRL( 'i' ) )
		{
			ctlAction = controlKeyAction( c ) ;
			ctlkeyPressed = true ;
			c = KEY_ENTER ;
		}

		if ( c > KEY_MAX )
		{
			auto it = keymap.find( keyname( c ) ) ;
			if ( it != keymap.end() )
			{
				c = it->second ;
			}
		}

		switch( c )
		{
			case kUP3:          // alt up
				ctlAction = "RETRIEVE" ;
				ctlkeyPressed = true ;
				c = KEY_ENTER ;
				break ;

			case kDN3:          // alt down
				ctlAction = "RETF" ;
				ctlkeyPressed = true ;
				c = KEY_ENTER ;
				break ;
		}

		switch( c )
		{
			case kxIN:          // focus-in
				colouriseAll() ;
				break ;

			case kxOUT:         // focus-out
				decolouriseAll() ;
				break ;

			case KEY_SLEFT:     // shifted left
				currScrn->cursor_left_cond() ;
			case KEY_LEFT:
				currScrn->cursor_left() ;
				if ( currAppl->currPanel->pd_active() )
				{
					col = currScrn->get_col() ;
					currAppl->currPanel->display_current_pd( err, row, col ) ;
				}
				break ;

			case kLFT5:           // ctrl left
				if ( !currAppl->currPanel->cursor_prev_word( row, col ) )
				{
					currAppl->currPanel->field_tab_prev( row, col ) ;
					currAppl->currPanel->cursor_last_word( row, col ) ;
				}
				currScrn->set_cursor( row, col ) ;
				break ;

			case KEY_SRIGHT:      // shifted right
				currScrn->cursor_right_cond() ;
			case KEY_RIGHT:
				currScrn->cursor_right() ;
				if ( currAppl->currPanel->pd_active() )
				{
					col = currScrn->get_col() ;
					currAppl->currPanel->display_current_pd( err, row, col ) ;
				}
				break ;

			case kRIT5:           // ctrl right
				if ( !currAppl->currPanel->cursor_next_word( row, col ) )
				{
					currAppl->currPanel->field_tab_next( row, col ) ;
				}
				currScrn->set_cursor( row, col ) ;
				break ;

			case KEY_SR:        //  shifted-Up
				currScrn->cursor_up_cond() ;
			case KEY_UP:
				currScrn->cursor_up() ;
				if ( currAppl->currPanel->pd_active() )
				{
					row = currScrn->get_row() ;
					currAppl->currPanel->display_current_pd( err, row, col ) ;
				}
				break ;

			case kUP5:          //  ctrl Up
				currScrn->cursor_up() ;
				row = currScrn->get_row() ;
				if ( currAppl->currPanel->cursor_first_word( row, col ) )
				{
					currScrn->set_cursor( row, col ) ;
				}
				break ;

			case KEY_SF:        //  shifted-Down
				currScrn->cursor_down_cond() ;
			case KEY_DOWN:
				currScrn->cursor_down() ;
				if ( currAppl->currPanel->pd_active() )
				{
					row = currScrn->get_row() ;
					currAppl->currPanel->display_current_pd( err, row, col ) ;
				}
				break ;

			case kDN5:          //  ctrl Down
				currScrn->cursor_down() ;
				row = currScrn->get_row() ;
				if ( currAppl->currPanel->cursor_first_word( row, col ) )
				{
					currScrn->set_cursor( row, col ) ;
				}
				break ;

			case CTRL( 'i' ):   // Tab key
				actionTabKey( row, col, pdchoice ) ;
				if ( pdchoice )
				{
					currScrn->set_cursor( currAppl ) ;
				}
				break ;

			case KEY_BTAB:      // shifted tab
				if ( currAppl->currPanel->pd_active() )
				{
					displayPrevPullDown() ;
				}
				else
				{
					currAppl->currPanel->field_tab_prev( row, col ) ;
					currScrn->set_cursor( row, col ) ;
				}
				break ;

			case KEY_IC:
				Insert = not Insert ;
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
					currAppl->get_home2( row, col ) ;
				}
				currScrn->set_cursor( row, col ) ;
				break ;

			case KEY_SHOME:  // shifted home.
				currAppl->currPanel->cursor_first_sof( row, col ) ;
				currScrn->set_cursor( row, col ) ;
				break ;

			case KEY_DC:
				if ( currAppl->inputInhibited() ) { break ; }
				currAppl->currPanel->field_delete_char( row, col, showLock ) ;
				break ;

			case KEY_SDC:
				if ( currAppl->inputInhibited() ) { break ; }
				currAppl->currPanel->field_erase_eof( row, col, showLock ) ;
				break ;

			case KEY_RESIZE:
				resizeTerminal() ;
				break ;

			case kDC3:       // alt-Delete
				if ( currAppl->inputInhibited() ) { break ; }
				currAppl->currPanel->field_erase_spaces( row, col, showLock ) ;
				break ;

			case kDC5:       // ctrl-Delete
				if ( currAppl->inputInhibited() ) { break ; }
				currAppl->currPanel->field_erase_eof( row, col, showLock, true ) ;
				break ;

			case kDC6:       // shifted-ctrl-Delete
				if ( currAppl->inputInhibited() ) { break ; }
				currAppl->currPanel->field_erase_word( row, col, showLock ) ;
				break ;

			case KEY_END:
				currAppl->currPanel->cursor_eof( row, col ) ;
				currScrn->set_cursor( row, col ) ;
				break ;

			case KEY_SEND:   // shifted-End
				currAppl->currPanel->cursor_sof( row, col ) ;
				currScrn->set_cursor( row, col ) ;
				break ;

			case kEND5:      // ctrl-End
				currAppl->currPanel->field_tab_down( row, col ) ;
				currScrn->set_cursor( row, col ) ;
				break ;

			case kEND6:      // shifted-ctrl-End
				break ;

			case 127:
			case KEY_BACKSPACE:
				if ( currAppl->inputInhibited() ) { break ; }
				currAppl->currPanel->field_backspace( row, col, showLock ) ;
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
			case kNXT5:
			case kPRV5:
			case CTRL( '[' ):       // Escape key
				pLScreen::OIA_startTime() ;
				debug1( "Action key pressed.  Processing" << endl ) ;
				if ( currAppl->msgInhibited() )
				{
					currAppl->msgResponseOK() ;
					break ;
				}
				Insert = false ;
				currScrn->set_Insert( Insert ) ;
				pLScreen::show_busy() ;
				updateDefaultVars() ;
				doSelect = false ;
				passthru = true  ;
				if ( !pApplication::lineInOutDone )
				{
					processAction( selct, row, col, c, doSelect, passthru, wmPending, pfkeyPressed, ctlkeyPressed ) ;
				}
				displayNotifies() ;
				if ( passthru )
				{
					updateReflist() ;
					updateHistory() ;
					ResumeApplicationAndWait() ;
					if ( currAppl->selectPanel() )
					{
						processZSEL() ;
					}
				}
				else
				{
					processZCOMMAND( selct, row, col, doSelect ) ;
				}
				while ( currAppl->SEL && !currAppl->terminateAppl )
				{
					processPGMSelect() ;
					while ( currAppl->terminateAppl )
					{
						terminateApplication() ;
						if ( pLScreen::screensTotal == 0 ) { return ; }
					}
				}
				while ( currAppl->terminateAppl )
				{
					terminateApplication() ;
					if ( pLScreen::screensTotal == 0 ) { return ; }
					while ( currAppl->SEL && !currAppl->terminateAppl )
					{
						processPGMSelect() ;
					}
				}
				currScrn->set_cursor( currAppl ) ;
				pLScreen::OIA_endTime() ;
				break ;

			case KEY_MOUSE:
				break ;

			default:
				debug1( "Action key "<<c<<" ("<<keyname( c )<<") ignored" << endl ) ;
		}
		decolouriseScreen() ;
	}
}


void actionMouseEvent( MEVENT* event,
		       int& c,
		       uint& row,
		       uint& col,
		       bool& wmPending )
{
	string act ;
	string term ;

	if ( event->bstate & BUTTON1_PRESSED && currAppl->currPanel->is_popup() )
	{
		term = p_poolMGR->sysget( err, "ZTERM", SHARED ) ;
		currAppl->set_pcursor( row, col ) ;
		currAppl->currPanel->remove_pd( err ) ;
		issueMessage( "PSYS015" ) ;
		wmPending = true  ;
		if ( term == "xterm-1002" )
		{
			mousemask( ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr ) ;
			ncursesUpdate( row, col ) ;
			while ( true )
			{
				pLScreen::clear_busy() ;
				c = getch() ;
				if ( c == KEY_MOUSE && getmouse( event ) == OK )
				{
					if ( event->bstate & BUTTON1_RELEASED )
					{
						currScrn->set_cursor( currAppl ) ;
						wmPending = false ;
						break ;
					}
					pLScreen::show_busy() ;
					row = min( pLScreen::maxrow-1, uint( event->y ) ) ;
					col = event->x ;
					currScrn->set_cursor( row, col ) ;
					currAppl->clear_msg() ;
					currScrn->save_panel_stack() ;
					currAppl->movepop( row, col ) ;
					currScrn->restore_panel_stack() ;
					currScrn->set_cursor( currAppl ) ;
					currAppl->display_id() ;
					currAppl->display_pfkeys() ;
					currScrn->OIA_update( priScreen, altScreen ) ;
					ncursesUpdate( row, col ) ;
				}
				else
				{
					currScrn->set_cursor( currAppl ) ;
					wmPending = false ;
					break ;
				}
			}
			mousemask( ALL_MOUSE_EVENTS, nullptr ) ;
		}
		if ( term == "xterm-1003" )
		{
			mousemask( ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr ) ;
			ncursesUpdate( row, col ) ;
			while ( true )
			{
				pLScreen::clear_busy() ;
				c = getch() ;
				if ( c == KEY_MOUSE && getmouse( event ) == OK )
				{
					if ( event->bstate & BUTTON1_PRESSED || event->bstate & BUTTON1_RELEASED ||
					     event->bstate & BUTTON1_CLICKED )
					{
						currScrn->set_cursor( currAppl ) ;
						wmPending = false ;
						break ;
					}
					pLScreen::show_busy() ;
					row = min( pLScreen::maxrow-1, uint( event->y ) ) ;
					col = event->x ;
					currScrn->set_cursor( row, col ) ;
					currAppl->clear_msg() ;
					currScrn->save_panel_stack() ;
					currAppl->movepop( row, col ) ;
					currScrn->set_cursor( currAppl ) ;
					currScrn->restore_panel_stack() ;
					currAppl->display_id() ;
					currAppl->display_pfkeys() ;
					currScrn->OIA_update( priScreen, altScreen ) ;
					ncursesUpdate( row, col ) ;
				}
				else
				{
					currScrn->set_cursor( currAppl ) ;
					wmPending = false ;
					break ;
				}
			}
			mousemask( ALL_MOUSE_EVENTS, nullptr ) ;
		}
		return ;
	}

	if ( event->bstate & BUTTON1_RELEASED && wmPending )
	{
		row = min( pLScreen::maxrow-1, uint( event->y ) ) ;
		col = event->x ;
		currScrn->set_cursor( row, col ) ;
		currAppl->clear_msg() ;
		currScrn->save_panel_stack() ;
		currAppl->movepop( row, col ) ;
		currScrn->restore_panel_stack() ;
		currScrn->set_cursor( currAppl ) ;
		currAppl->display_id() ;
		currAppl->display_pfkeys() ;
		wmPending = false ;
		return ;
	}

	if ( event->bstate & BUTTON1_CLICKED )
	{
		act = p_poolMGR->sysget( err, "ZMOUSE11", PROFILE ) ;
		processMouseAction( act, event, c, row, col ) ;
	}
	else if ( event->bstate & BUTTON1_DOUBLE_CLICKED )
	{
		act = p_poolMGR->sysget( err, "ZMOUSE12", PROFILE ) ;
		processMouseAction( act, event, c, row, col ) ;
	}
	else if ( event->bstate & BUTTON1_TRIPLE_CLICKED )
	{
		act = p_poolMGR->sysget( err, "ZMOUSE13", PROFILE ) ;
		processMouseAction( act, event, c, row, col ) ;
	}
	else if ( event->bstate & BUTTON2_CLICKED )
	{
		act = p_poolMGR->sysget( err, "ZMOUSE21", PROFILE ) ;
		processMouseAction( act, event, c, row, col ) ;
	}
	else if ( event->bstate & BUTTON2_DOUBLE_CLICKED )
	{
		act = p_poolMGR->sysget( err, "ZMOUSE22", PROFILE ) ;
		processMouseAction( act, event, c, row, col ) ;
	}
	else if ( event->bstate & BUTTON2_TRIPLE_CLICKED )
	{
		act = p_poolMGR->sysget( err, "ZMOUSE23", PROFILE ) ;
		processMouseAction( act, event, c, row, col ) ;
	}
	else if ( event->bstate & BUTTON3_CLICKED )
	{
		act = p_poolMGR->sysget( err, "ZMOUSE31", PROFILE ) ;
		processMouseAction( act, event, c, row, col ) ;
	}
	else if ( event->bstate & BUTTON3_DOUBLE_CLICKED )
	{
		act = p_poolMGR->sysget( err, "ZMOUSE32", PROFILE ) ;
		processMouseAction( act, event, c, row, col ) ;
	}
	else if ( event->bstate & BUTTON3_TRIPLE_CLICKED )
	{
		act = p_poolMGR->sysget( err, "ZMOUSE33", PROFILE ) ;
		processMouseAction( act, event, c, row, col ) ;
	}
	else if ( event->bstate & BUTTON4_PRESSED )
	{
		act = p_poolMGR->sysget( err, "ZMOUSESA", PROFILE ) ;
		c = KEY_ENTER ;
		if ( act == "CSR" )
		{
			row = min( pLScreen::maxrow-1, uint( event->y ) ) ;
			col = event->x ;
			currScrn->set_cursor( row, col ) ;
		}
		commandStack = "UP " + act ;
	}
	else if ( event->bstate & BUTTON5_PRESSED )
	{
		act = p_poolMGR->sysget( err, "ZMOUSESA", PROFILE ) ;
		c = KEY_ENTER ;
		if ( act == "CSR" )
		{
			row = min( pLScreen::maxrow-1, uint( event->y ) ) ;
			col = event->x ;
			currScrn->set_cursor( row, col ) ;
		}
		commandStack = "DOWN " + act ;
	}
}


void processMouseAction( string& act,
			 MEVENT* event,
			 int& c,
			 uint& row,
			 uint& col )
{
	//
	// Action mouse event:
	//   1 - NOP
	//   2 - Move cursor
	//   3 - Enter
	//   4 - Move cursor + Enter
	//   5 - Execute command
	//   6 - Execute line command
	//

	switch ( act.front() )
	{
	case '1':
		break ;

	case '2':
		row = min( pLScreen::maxrow-1, uint( event->y ) ) ;
		col = event->x ;
		if ( currAppl->currPanel->pd_active() )
		{
			currAppl->currPanel->display_current_pd( err, row, col ) ;
		}
		currScrn->set_cursor( row, col ) ;
		break ;

	case '3':
		c = KEY_ENTER ;
		break ;

	case '4':
		row = min( pLScreen::maxrow-1, uint( event->y ) ) ;
		col = event->x ;
		if ( currAppl->currPanel->pd_active() )
		{
			currAppl->currPanel->display_current_pd( err, row, col ) ;
		}
		currScrn->set_cursor( row, col ) ;
		c = KEY_ENTER ;
		break ;

	case '5':
		c = KEY_ENTER ;
		commandStack = ( commandStack == "" ) ? act.substr( 1 ) : act.substr( 1 ) + " ; " + commandStack ;
		break ;

	case '6':
		row = min( pLScreen::maxrow-1, uint( event->y ) ) ;
		col = event->x ;
		c   = KEY_ENTER ;
		currScrn->set_cursor( row, col ) ;
		commandStack = act.substr( 1 ) ;
		break ;

	}
}


void setGlobalClassVars()
{
	pApplication::p_tableMGR = p_tableMGR ;
	pFTailor::p_tableMGR     = p_tableMGR ;

	pApplication::p_gls = p_gls ;
	lss::p_gls          = p_gls ;

	pApplication::p_poolMGR = p_poolMGR ;
	pPanel::p_poolMGR       = p_poolMGR ;
	pFTailor::p_poolMGR     = p_poolMGR ;
	abc::p_poolMGR          = p_poolMGR ;

	pApplication::lg = lgx ;
	trace_logger::lg = lgx ;
	pPanel::lg       = lgx ;
	pFTailor::lg     = lgx ;
	tableMGR::lg     = lgx ;
	poolMGR::lg      = lgx ;
	lss::lg          = lg  ;
	gls::lg          = lg  ;
}


void initialSetup()
{
	err.clear() ;

	funcPOOL.define( err, "ZCTVERB",  &zctverb  ) ;
	funcPOOL.define( err, "ZCTTRUNC", &zcttrunc ) ;
	funcPOOL.define( err, "ZCTACT",   &zctact   ) ;
	funcPOOL.define( err, "ZCTDESC",  &zctdesc  ) ;
}


void ncursesUpdate( uint row,
		    uint col )
{
	pLScreen::SWB_show( getSWBText() ) ;

	wnoutrefresh( stdscr ) ;
	wnoutrefresh( OIA ) ;
	update_panels() ;
	move( row, col ) ;
	doupdate() ;

	currScrn->clear_w_refresh() ;
}


void processZSEL()
{
	//
	// Called for a selection panel (ie. SELECT PANEL(ABC) function).
	// Use what's in ZSEL to start application.
	//

	int p ;

	string cmd ;
	string opt ;

	bool addpop  = false ;
	bool nocheck = false ;

	selobj selct ;

	err.clear() ;

	currAppl->save_errblock() ;
	cmd = currAppl->get_zsel() ;
	err = currAppl->get_errblock() ;
	currAppl->restore_errblock() ;
	if ( err.error() )
	{
		serviceCallError( err ) ;
	}

	if ( cmd == "" ) { return ; }

	if ( upper( cmd ) == "EXIT" )
	{
		currAppl->set_userAction( USR_END ) ;
		p_poolMGR->put( err, "ZVERB", "END", SHARED ) ;
		ResumeApplicationAndWait() ;
		return ;
	}

	if ( cmd.compare( 0, 6, "PANEL(" ) == 0 )
	{
		opt = currAppl->get_trail() ;
		if ( opt != "" ) { cmd += " OPT(" + opt + ")" ; }
	}

	p = wordpos( "ADDPOP", cmd ) ;
	if ( p > 0 )
	{
		addpop = true ;
		idelword( cmd, p, 1 ) ;
	}

	if ( cmd.compare( 0, 6, "PANEL(" ) != 0 )
	{
		p = wordpos( "NOCHECK", cmd ) ;
		if ( p > 0 )
		{
			nocheck = true ;
			idelword( cmd, p, 1 ) ;
		}
	}

	currAppl->currPanel->remove_pd( err ) ;

	if ( !selct.parse( err, cmd ) )
	{
		errorScreen( "Error in selection panel "+ currAppl->get_panelid(), "ZSEL = "+ cmd ) ;
		return ;
	}

	selct.quiet   = true ;
	selct.nocheck = nocheck ;
	selct.selopt  = word( selct.parm, 2 ) ;

	resolvePGM( selct, currAppl ) ;

	if ( addpop )
	{
		selct.parm += " ADDPOP" ;
	}

	startApplication( selct, false, true ) ;
	if ( selct.errors )
	{
		p_poolMGR->put( err, "ZVAL1", selct.pgm ,SHARED ) ;
		p_poolMGR->put( err, "ZERRDSC", "P", SHARED ) ;
		p_poolMGR->put( err, "ZERRSRC", "ZSEL = "+ cmd ,SHARED ) ;
		errorScreen( ( selct.rsn == 998 ) ? "PSYS012W" : "PSYS013H" ) ;
	}
}


void processAction( selobj& selct,
		    uint row,
		    uint col,
		    int c,
		    bool& doSelect,
		    bool& passthru,
		    bool& wmPending,
		    bool& pfkeyPressed,
		    bool& ctlkeyPressed )
{
	//
	// Perform lspf high-level functions - pfkey -> command.
	// application/user/site/system command table entry?
	// BUILTIN command.
	// System command.
	// RETRIEVE/RETF.
	// Jump command entered.
	// Else pass event to application.
	//

	size_t p1 ;

	uint lrow ;
	uint lcol ;

	uint olrow ;
	uint olcol ;

	bool addRetrieve = true ;
	bool nested      = true ;
	bool fromStack   = false ;

	string cmdVerb ;
	string cmdParm ;
	string hlpParm ;
	string cmdfld  ;
	string fname   ;
	string pfcmd   ;
	string delm    ;
	string aVerb   ;
	string msg     ;
	string w1      ;
	string t       ;

	err.clear() ;

	pdc t_pdc ;

	pfcmd    = "" ;
	zcommand = "" ;

	currAppl->set_userAction( USR_ENTER ) ;
	currAppl->get_lcursor( olrow, olcol ) ;

	if ( c == CTRL( '[' ) )
	{
		if ( currAppl->currPanel->pd_active() )
		{
			currAppl->clear_msg() ;
			if ( currAppl->currPanel->cursor_on_pulldown( row, col ) )
			{
				currAppl->set_lcursor_home() ;
				currScrn->set_cursor( currAppl ) ;
			}
			currAppl->currPanel->remove_pd( err ) ;
		}
		else if ( wmPending )
		{
			wmPending = false ;
			currAppl->clear_msg() ;
			currAppl->set_lcursor_home() ;
			currScrn->set_cursor( currAppl ) ;
		}
		else
		{
			actionSwap( "LIST" ) ;
		}
		passthru = false ;
		zcommand = "NOP" ;
		return ;
	}

	if ( c == KEY_ENTER && !ctlkeyPressed )
	{
		if ( pLScreen::swapbar && row == pLScreen::maxrow - 1 )
		{
			t    = getSWBText() ;
			lcol = col - 1 ;
			if ( t.size() > lcol && t[ lcol ] != ' ' )
			{
				p1 = t.find_last_of( ' ', lcol ) + 1 ;
				if ( t[ p1 ] != '*' )
				{
					currScrn->set_cursor( currAppl ) ;
					actionSwap( d2ds( ( lcol / 10 ) + 1 ) ) ;
				}
				passthru = false ;
				zcommand = "NOP" ;
				return ;
			}
		}
		if ( wmPending )
		{
			currAppl->clear_msg() ;
			currScrn->save_panel_stack() ;
			currAppl->movepop( row, col ) ;
			currScrn->restore_panel_stack() ;
			currScrn->set_cursor( currAppl ) ;
			currAppl->display_id() ;
			currAppl->display_pfkeys() ;
			wmPending = false ;
			passthru  = false ;
			zcommand  = "NOP" ;
			return ;
		}
		else if ( currAppl->currPanel->hide_msg_window( row, col ) )
		{
			zcommand = "NOP" ;
			passthru = false ;
			currScrn->set_cursor( currAppl ) ;
			return ;
		}
		else if ( currAppl->currPanel->cursor_on_border_line( row, col ) )
		{
			currAppl->set_pcursor( row, col ) ;
			currAppl->currPanel->remove_pd( err ) ;
			issueMessage( "PSYS015" ) ;
			zcommand     = "NOP" ;
			wmPending    = true  ;
			passthru     = false ;
			commandStack = "" ;
			return ;
		}
		else if ( currAppl->currPanel->jump_field( row, col, t ) )
		{
			currAppl->set_pcursor( row, col ) ;
			currAppl->currPanel->cmd_setvalue( t ) ;
		}
		else if ( currAppl->currPanel->cmd_getvalue() == "" &&
			  currAppl->currPanel->cursor_on_ab( row ) )
		{
			currAppl->set_pcursor( row, col ) ;
			if ( currAppl->currPanel->display_pd( err, row, col, msg, 1 ) )
			{
				if ( msg != "" )
				{
					issueMessage( msg ) ;
				}
				else if ( currAppl->currPanel->simulate_enter() )
				{
					pApplication::controlNonDispl = true ;
				}
				currScrn->set_cursor( currAppl ) ;
				passthru = false ;
				zcommand = "NOP" ;
				return ;
			}
			else if ( err.error() )
			{
				errorScreen( err.msgid ) ;
				passthru = false ;
				zcommand = "NOP" ;
				return ;
			}
		}
		else if ( currAppl->currPanel->pd_active() )
		{
			currAppl->set_pcursor( row, col ) ;
			addRetrieve = false ;
			if ( !currAppl->currPanel->cursor_on_pulldown( row, col ) )
			{
				currAppl->currPanel->remove_pd( err ) ;
				currAppl->clear_msg() ;
				zcommand = "NOP" ;
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
				zcommand = "NOP" ;
				passthru = false ;
				return ;
			}
			else if ( t_pdc.pdc_run != "" )
			{
				if ( findword( t_pdc.pdc_run, "END RETURN CANCEL" ) )
				{
					currAppl->currPanel->remove_pd( err ) ;
				}
				currAppl->clear_msg() ;
				zcommand = t_pdc.pdc_run ;
				nested   = false ;
				if ( zcommand == "ISRROUTE" )
				{
					cmdVerb = word( t_pdc.pdc_parm, 1 )    ;
					cmdParm = subword( t_pdc.pdc_parm, 2 ) ;
					if ( cmdVerb == "SELECT" )
					{
						if ( !selct.parse( err, cmdParm ) )
						{
							llog( "E", "Error in SELECT command "+ t_pdc.pdc_parm << "." << endl ) ;
							currAppl->setmsg( "PSYS011K" ) ;
							return ;
						}
						resolvePGM( selct, currAppl ) ;
						doSelect = true  ;
						passthru = false ;
						return ;
					}
				}
				else
				{
					zcommand += " " + t_pdc.pdc_parm ;
				}
			}
			else
			{
				currAppl->currPanel->remove_pd( err ) ;
				currAppl->clear_msg() ;
				zcommand = "" ;
				return ;
			}
		}
	}

	currAppl->set_pcursor( row, col ) ;

	if ( wmPending )
	{
		wmPending = false ;
		currAppl->clear_msg() ;
	}

	delm = p_poolMGR->sysget( err, "ZDEL", PROFILE ) ;

	if ( t_pdc.pdc_run == "" )
	{
		zcommand = strip( currAppl->currPanel->cmd_getvalue() ) ;
		if ( zcommand != "" && currAppl->get_selopt1() )
		{
			passthru = true ;
			return ;
		}
	}

	if ( c == KEY_ENTER  &&
	     zcommand != ""  &&
	     currAppl->currPanel->has_command_field() &&
	     p_poolMGR->sysget( err, "ZSWAPC", PROFILE ) == zcommand.substr( 0, 1 ) &&
	     p_poolMGR->sysget( err, "ZSWAP",  PROFILE ) == "Y" )
	{
		currAppl->currPanel->field_get_row_col( currAppl->currPanel->cmdfield, lrow, lcol ) ;
		if ( lrow == row && lcol < col )
		{
			passthru = false ;
			currAppl->currPanel->cmd_setvalue( "" ) ;
			actionSwap( upper( zcommand.substr( 1, col-lcol-1 ) ) ) ;
			return ;
		}
	}

	if ( ctlkeyPressed )
	{
		pfcmd = ctlAction ;
	}

	if ( commandStack != "" )
	{
		if ( currAppl->currPanel->cmd_nonblank() && !currAppl->simulate_enter() )
		{
			if ( zcommand != "" && zcommand.front() == '&' && !currAppl->currPanel->error_msg_issued() )
			{
				currAppl->currPanel->cmd_setvalue( "" ) ;
			}
			else
			{
				currAppl->currPanel->cmd_setvalue( zcommand + commandStack ) ;
				commandStack = "" ;
			}
			passthru = false ;
			return ;
		}
		if ( zcommand != "" )
		{
			if ( zcommand.front() != '&' && currAppl->simulate_enter() )
			{
				if ( zcommand.front() == '>' )
				{
					zcommand.erase( 0, 1 ) ;
				}
				currAppl->currPanel->cmd_setvalue( zcommand ) ;
				return ;
			}
			currAppl->currPanel->cmd_setvalue( zcommand ) ;
		}
		if ( !currAppl->propagateEnd )
		{
			fromStack = true ;
			zcommand  = commandStack ;
			commandStack = "" ;
		}
		addRetrieve = false ;
	}
	else if ( zcommand.size() > 1 && zcommand.front() == delm.front() )
	{
		p1 = zcommand.find_first_not_of( ' ', 1 ) ;
		if ( p1 != string::npos && zcommand[ p1 ] != delm.front() )
		{
			zcommand = delm + zcommand ;
		}
	}

	if ( pfkeyPressed )
	{
		pfcmd = currAppl->currPanel->get_pfkey( err, c ) ;
	}
	else
	{
		p_poolMGR->put( err, "ZPFKEY", "PF00", SHARED, SYSTEM ) ;
		currAppl->currPanel->set_pfpressed() ;
	}

	if ( addRetrieve )
	{
		addRetrieveBuffer( zcommand, pfcmd ) ;
	}

	switch( c )
	{
		case KEY_PPAGE:
			pfcmd = "UP" ;
			p_poolMGR->put( err, "ZPFKEY", "PF25", SHARED, SYSTEM ) ;
			break ;

		case kPRV5:
			pfcmd = "TOP" ;
			p_poolMGR->put( err, "ZPFKEY", "PF25", SHARED, SYSTEM ) ;
			break ;

		case KEY_NPAGE:
			pfcmd = "DOWN" ;
			p_poolMGR->put( err, "ZPFKEY", "PF26", SHARED, SYSTEM ) ;
			break ;

		case kNXT5:
			pfcmd = "BOTTOM" ;
			p_poolMGR->put( err, "ZPFKEY", "PF26", SHARED, SYSTEM ) ;
			break ;
	}

	if ( pfcmd != "" )
	{
		if ( pfcmd.front() == ':' )
		{
			currAppl->currPanel->add_linecmd( err, row, col, pfcmd.substr( 1 ) ) ;
			if ( err.error() )
			{
				issueMessage( copies( err.msgid, 1 ) ) ;
				passthru = false ;
				zcommand = "NOP" ;
				return ;
			}
			else if ( err.RC4() )
			{
				zcommand = pfcmd + " " + zcommand ;
			}
			else if ( err.RC8() )
			{
				currAppl->set_lcursor_home() ;
				passthru = false ;
				zcommand = "NOP" ;
				return ;
			}
		}
		else
		{
			zcommand = pfcmd + " " + zcommand ;
		}
	}

	if ( zcommand.compare( 0, 1, delm ) == 0 )
	{
		zcommand.erase( 0, 1 ) ;
		currAppl->currPanel->cmd_setvalue( zcommand ) ;
	}

	p1 = zcommand.find( delm.front() ) ;
	if ( p1 != string::npos )
	{
		commandStack = zcommand.substr( p1 ) ;
		if ( commandStack == delm ) { commandStack = "" ; }
		zcommand.erase( p1 ) ;
		currAppl->currPanel->cmd_setvalue( zcommand ) ;
	}

	cmdVerb = upper( word( zcommand, 1 ) ) ;
	if ( cmdVerb == "" ) { retPos = 0 ; return ; }

	cmdParm = subword( zcommand, 2 ) ;

	if ( cmdVerb.front() == '>' )
	{
		zcommand.erase( 0, 1 ) ;
		currAppl->currPanel->cmd_setvalue( zcommand ) ;
		return ;
	}

	if ( zcommand.size() > 1 && zcommand.front() == '=' && ( pfcmd == "" || pfcmd == "RETURN" ) )
	{
		zcommand.erase( 0, 1 ) ;
		jumpOption = zcommand + commandStack ;
		if ( !currAppl->isprimMenu() )
		{
			currAppl->jumpEntered = true ;
			currAppl->set_userAction( USR_RETURN ) ;
			p_poolMGR->put( err, "ZVERB", "RETURN", SHARED ) ;
			currAppl->currPanel->cmd_setvalue( "" ) ;
			return ;
		}
		currAppl->currPanel->cmd_setvalue( zcommand ) ;
	}

	if ( cmdVerb == "KEYSHELP" )
	{
	       hlpParm = currAppl->get_keyshelp() ;
	       if ( hlpParm == "" || hlpParm == "*" )
	       {
			issueMessage( ( hlpParm == "" ) ? "PSYS021C" : "PSYS021E" ) ;
			zcommand = "NOP" ;
			passthru = false ;
			currAppl->currPanel->cmd_setvalue( "" ) ;
			currAppl->set_lcursor_home() ;
			currScrn->set_cursor( currAppl ) ;
			return ;
	       }
	}
	else if ( cmdVerb == "EXHELP" )
	{
	       hlpParm = currAppl->get_exhelp() ;
	       if ( hlpParm == "" || hlpParm.front() == '*' )
	       {
			issueMessage( ( hlpParm == "" ) ? "PSYS021D" : "PSYS021F" ) ;
			zcommand = "NOP" ;
			passthru = false ;
			currAppl->currPanel->cmd_setvalue( "" ) ;
			currAppl->set_lcursor_home() ;
			currScrn->set_cursor( currAppl ) ;
			return ;
	       }
	}
	else if ( cmdVerb == "HELP" )
	{
		if ( currAppl->show_help_member() )
		{
			currAppl->currPanel->cmd_setvalue( "" ) ;
			help h = currAppl->currPanel->get_field_help( row, col ) ;
			if ( h.help_rp != "" && h.help_missing )
			{
				issueMessage( "PSYS014E", h.help_rp ) ;
				zcommand = "NOP" ;
				passthru = false ;
				return ;
			}
			else if ( h.help_passthru )
			{
				currAppl->currPanel->cmd_setvalue( "HELP" ) ;
				zcommand = "NOP" ;
				return ;
			}
			else if ( h.help_panel != "" )
			{
				zparm    = h.help_panel ;
				cmdParm  = zparm ;
				passthru = false ;
			}
			else if ( h.help_msg != "" )
			{
				issueMessage( h.help_msg ) ;
				zcommand = "NOP" ;
				passthru = false ;
				return ;
			}
			else
			{
				zparm = currAppl->get_help() ;
				if ( zparm == "*" )
				{
					issueMessage( "PSYS021G" ) ;
					zcommand = "NOP" ;
					passthru = false ;
					currAppl->currPanel->cmd_setvalue( "" ) ;
					currAppl->set_lcursor_home() ;
					currScrn->set_cursor( currAppl ) ;
					return ;
				}
				cmdParm = zparm ;
			}
		}
		else
		{
			if ( currAppl->setMessage )
			{
				currAppl->display_setmsg() ;
			}
			currAppl->currPanel->display_lmsg( err ) ;
			currAppl->currPanel->reset_cmd() ;
			if ( !pfkeyPressed && !ctlkeyPressed )
			{
				currAppl->set_lcursor_home() ;
			}
			zcommand = "NOP" ;
			passthru = false ;
			return ;
		}
	}

	aVerb = cmdVerb ;

	if ( resolveZCTEntry( cmdVerb, cmdParm ) )
	{
		auto it = zverbs.find( word( zctact, 1 ) ) ;
		switch ( it->second )
		{
		case ZCT_ACTIONS:
			displayNextPullDown( iupper( cmdParm ) ) ;
			currAppl->currPanel->cmd_setvalue( "" ) ;
			passthru = false ;
			zcommand = "NOP" ;
			return ;

		case ZCT_CANCEL:
			if ( currAppl->currPanel->pd_active() &&
			     currAppl->currPanel->cursor_on_pulldown( row, col ) )
			{
				currAppl->currPanel->remove_pd( err ) ;
				currAppl->clear_msg() ;
				zcommand = "" ;
				return ;
			}
			else
			{
				currAppl->set_userAction( USR_CANCEL ) ;
				p_poolMGR->put( err, "ZVERB", "CANCEL", SHARED ) ;
				zcommand = cmdParm ;
			}
			break ;

		case ZCT_CURSOR:
			passthru = false ;
			zcommand = "NOP" ;
			currAppl->currPanel->cursor( row, col ) ;
			currScrn->set_cursor( currAppl ) ;
			break ;

		case ZCT_EXIT:
			currAppl->set_userAction( USR_EXIT ) ;
			p_poolMGR->put( err, "ZVERB", "EXIT", SHARED ) ;
			zcommand = cmdParm ;
			break ;

		case ZCT_NOP:
			p_poolMGR->put( err, "ZCTMVAR", left( aVerb, 8 ), SHARED ) ;
			issueMessage( "PSYS011" ) ;
			currAppl->currPanel->cmd_setvalue( "" ) ;
			passthru = false ;
			return ;

		case ZCT_PASSTHRU:
			passthru = true ;
			zcommand = strip( cmdVerb + " " + cmdParm ) ;
			break ;

		case ZCT_RETK:
			currAppl->currPanel->cmd_setvalue( "" ) ;
			listRetrieveBuffer( 'K', upper( cmdParm ) ) ;
			passthru = false ;
			zcommand = "NOP" ;
			return ;

		case ZCT_RETP:
			currAppl->currPanel->cmd_setvalue( "" ) ;
			listRetrieveBuffer( 'N', upper( cmdParm ) ) ;
			passthru = false ;
			zcommand = "NOP" ;
			return ;

		case ZCT_CRETRIEV:
			cmdfld = currAppl->currPanel->cmdfield ;
			if ( cmdfld == "" || !currAppl->currPanel->cursor_on_field( cmdfld, row, col ) )
			{
				passthru = false ;
				zcommand = "NOP" ;
				currAppl->currPanel->remove_pd( err ) ;
				currAppl->set_lcursor_home() ;
				currScrn->set_cursor( currAppl ) ;
				return ;
			}

		case ZCT_RETRIEVE:
		case ZCT_RETF:
			if ( !currAppl->currPanel->has_command_field() )
			{
				commandStack = ""    ;
				zcommand     = "NOP" ;
				passthru     = false ;
				return ;
			}
			if ( commandStack == "" && datatype( cmdParm, 'W' ) )
			{
				p1 = ds2d( cmdParm ) ;
				if ( p1 > 0 && p1 <= retrieveBuffer.size() ) { retPos = p1 - 1 ; }
			}
			commandStack = ""    ;
			zcommand     = "NOP" ;
			passthru     = false ;
			if ( !retrieveBuffer.empty() )
			{
				if ( it->second == ZCT_RETF )
				{
					retPos = ( retPos < 2 ) ? retrieveBuffer.size() : retPos - 1 ;
				}
				else
				{
					if ( ++retPos > retrieveBuffer.size() ) { retPos = 1 ; }
				}
				currAppl->currPanel->cmd_setvalue( retrieveBuffer[ retPos-1 ] ) ;
				currAppl->currPanel->cursor_to_cmdfield( retrieveBuffer[ retPos-1 ].size() + 1 ) ;
			}
			else
			{
				currAppl->currPanel->cmd_setvalue( "" ) ;
				currAppl->currPanel->cursor_to_cmdfield() ;
			}
			currScrn->set_cursor( currAppl ) ;
			currAppl->currPanel->remove_pd( err ) ;
			currAppl->clear_msg() ;
			return ;

		case ZCT_SCRNAME:
			iupper( cmdParm ) ;
			w1      = word( cmdParm, 2 ) ;
			cmdParm = word( cmdParm, 1 ) ;
			if  ( cmdParm == "ON" )
			{
				p_poolMGR->put( err, "ZSCRNAM1", "ON", SHARED, SYSTEM ) ;
			}
			else if ( cmdParm == "OFF" )
			{
				p_poolMGR->put( err, "ZSCRNAM1", "OFF", SHARED, SYSTEM ) ;
			}
			else if ( isvalidName( cmdParm ) && !findword( cmdParm, "LIST PREV NEXT" ) )
			{
				p_poolMGR->put( err, currScrn->screenid(), "ZSCRNAME", cmdParm ) ;
				p_poolMGR->put( err, "ZSCRNAME", cmdParm, SHARED ) ;
				if ( w1 == "PERM" )
				{
					p_poolMGR->put( err, currScrn->screenid(), "ZSCRNAM2", w1 ) ;
				}
				else
				{
					p_poolMGR->put( err, currScrn->screenid(), "ZSCRNAM2", "" ) ;
				}
			}
			else
			{
				issueMessage( "PSYS013" ) ;
			}
			currAppl->display_id() ;
			currAppl->set_lcursor_home() ;
			currScrn->set_cursor( currAppl ) ;
			passthru = false ;
			zcommand = "" ;
			break  ;

		case ZCT_SELECT:
			if ( !selct.parse( err, subword( zctact, 2 ) ) )
			{
				llog( "E", "Error in SELECT command "+ zctact << "." << endl ) ;
				currAppl->setmsg( "PSYS011K" ) ;
				return ;
			}
			if ( hlpParm != "" )
			{
				selct.parm = hlpParm ;
			}
			else
			{
				p1 = selct.parm.find( "&ZPARM" ) ;
				if ( p1 != string::npos )
				{
					selct.parm.replace( p1, 6, cmdParm ) ;
				}
			}
			resolvePGM( selct, currAppl ) ;
			zcommand = "" ;
			doSelect = true  ;
			selct.nested = ( selct.nested || nested ) ;
			selct.pfkey  = ( pfkeyPressed || ctlkeyPressed ) ;
			selct.fstack = fromStack ;
			passthru     = false ;
			break ;

		case ZCT_SETVERB:
			if ( currAppl->currPanel->is_cmd_inactive( cmdVerb, row, col ) )
			{
				p_poolMGR->put( err, "ZCTMVAR", left( aVerb, 8 ), SHARED ) ;
				issueMessage( "PSYS011" ) ;
				currAppl->currPanel->cursor_to_cmdfield() ;
				currScrn->set_cursor( currAppl ) ;
				currAppl->currPanel->cmd_setvalue( "" ) ;
				zcommand = "NOP" ;
				passthru = false ;
				return ;
			}
			p_poolMGR->put( err, "ZVERB", word( zctverb, 1 ), SHARED ) ;
			if ( err.error() )
			{
				llog( "S", "VPUT for ZVERB failed." << endl ) ;
				listErrorBlock( err ) ;
			}
			zcommand = subword( zcommand, 2 ) ;
			if ( zctverb == "ZEXPAND" )
			{
				fname = currAppl->currPanel->field_getname( row, col ) ;
				selct.pgm     = "PPSP01A" ;
				selct.parm    = "ZEXPAND " + left( fname, 13 )
							   + d2ds( currAppl->currPanel->get_display_field_length( fname ), 5 )
							   + ( currAppl->currPanel->field_is_input( fname ) ? "I" : "O" )
							   + currAppl->currPanel->field_getrawvalue( fname ) ;
				selct.zexpand = true ;
				zcommand = "" ;
				doSelect = true  ;
				passthru = false ;
			}
			else if ( zctverb == "ZHISTORY" )
			{
				fname = currAppl->currPanel->field_getname( row, col ) ;
				currAppl->currPanel->cmd_setvalue( "" ) ;
				currAppl->get_addpop( lrow, lcol ) ;
				executeHistoryCommand( currAppl->get_panelid(), fname, lrow, lcol, row, col ) ;
				zcommand = "" ;
				passthru = false ;
			}
			else if ( zctverb == "NRETRIEV" )
			{
				selct.clear() ;
				currAppl->vcopy( "ZRFLPGM", selct.pgm, MOVE ) ;
				selct.parm = "NR1 " + cmdParm ;
				doSelect   = true  ;
				passthru   = false ;
			}
			else if ( zctverb == "RETURN" )
			{
				currAppl->set_userAction( USR_RETURN ) ;
				returnOption = commandStack ;
				commandStack = "" ;
			}
			else if ( zctverb == "END" )
			{
				currAppl->set_userAction( USR_END ) ;
			}
			break ;

		case ZCT_SPLIT:
			if ( !currAppl->currPanel->keep_cmd() )
			{
				currAppl->clear_msg() ;
				currAppl->currPanel->cmd_setvalue( "" ) ;
				currAppl->currPanel->cursor_to_cmdfield() ;
				currScrn->set_cursor( currAppl ) ;
			}
			selct.def( gmainpgm ) ;
			startApplication( selct, true ) ;
			passthru = false ;
			zcommand = "NOP" ;
			return ;

		case ZCT_SWAP:
			t = upper( cmdParm ) ;
			if ( pfkeyPressed &&
			     t != ""      &&
			     currAppl->currPanel->has_command_field() &&
			     p_poolMGR->sysget( err, "ZSWAP", PROFILE ) == "Y" )
			{
				cmdfld = currAppl->currPanel->cmdfield ;
				if ( currAppl->currPanel->cursor_on_field( cmdfld, row, col ) )
				{
					currAppl->currPanel->field_get_row_col( cmdfld, lrow, lcol ) ;
					p1 = t.find( p_poolMGR->sysget( err, "ZSWAPC", PROFILE ) ) ;
					if ( p1 != string::npos && p1 < ( col - lcol ) )
					{
						addRetrieveBuffer( cmdParm.substr( 0, p1 ), pfcmd, true ) ;
						t = t.substr( p1 + 1, col - lcol - p1 - 1 ) ;
					}
				}
			}
			actionSwap( t ) ;
			passthru = false ;
			zcommand = "NOP" ;
			return ;

		case ZCT_USERID:
			iupper( cmdParm ) ;
			if ( cmdParm == "" )
			{
				w1 = p_poolMGR->get( err, currScrn->screenid(), "ZSHUSRID" ) ;
				cmdParm = ( w1 == "Y" ) ? "OFF" : "ON" ;
			}
			if ( cmdParm == "ON" )
			{
				p_poolMGR->put( err, currScrn->screenid(), "ZSHUSRID", "Y" ) ;
			}
			else if ( cmdParm == "OFF" )
			{
				p_poolMGR->put( err, currScrn->screenid(), "ZSHUSRID", "N" ) ;
			}
			else
			{
				issueMessage( "PSYS012F" ) ;
			}
			currAppl->display_id() ;
			currAppl->set_lcursor_home() ;
			currScrn->set_cursor( currAppl ) ;
			passthru = false ;
			zcommand = "" ;
			break ;

		case ZCT_WINDOW:
			if ( !currAppl->currPanel->is_popup() )
			{
				currAppl->set_lcursor( olrow, olcol ) ;
				currScrn->set_cursor( currAppl ) ;
				issueMessage( "PSYS014D" ) ;
				passthru  = false ;
				zcommand  = "NOP" ;
				break ;
			}
			currAppl->clear_msg() ;
			currAppl->set_lcursor( olrow, olcol ) ;
			currAppl->currPanel->remove_pd( err ) ;
			currScrn->save_panel_stack() ;
			currAppl->movepop( row, col ) ;
			currScrn->restore_panel_stack() ;
			currScrn->set_cursor( currAppl ) ;
			currAppl->display_id() ;
			currAppl->display_pfkeys() ;
			passthru  = false ;
			zcommand  = "NOP" ;
			break ;
		}
	}

	retPos = 0 ;

	if ( zcommands.find( cmdVerb ) != zcommands.end() )
	{
		passthru = false   ;
		zcommand = cmdVerb ;
		zparm    = upper( cmdParm ) ;
	}

	if ( findword( zctverb, "END RETURN CANCEL" ) &&
	   ( currAppl->currPanel->cursor_on_ab( row ) || currAppl->currPanel->pd_active() ) )
	{
		currAppl->currPanel->remove_pd( err ) ;
		currAppl->clear_msg() ;
		currAppl->set_lcursor_home() ;
		currScrn->set_cursor( currAppl ) ;
		passthru = false ;
		zcommand = "" ;
	}

	if ( !passthru )
	{
		currAppl->currPanel->remove_pd( err ) ;
	}

	currAppl->set_selopt( zcommand ) ;

	currAppl->currPanel->cmd_setvalue( ( passthru ) ? zcommand : "" ) ;
	debug1( "Primary command '"+ zcommand +"'  Passthru = " << passthru << endl ) ;
}


void processZCOMMAND( selobj& selct,
		      uint row,
		      uint col,
		      bool doSelect )
{
	//
	// If the event is not being passed to the application, process ZCOMMAND or start application request.
	//

	short int r ;
	short int g ;
	short int b ;

	string w1 ;
	string field_name ;

	fieldXct fxc ;

	auto it = zcommands.find( zcommand ) ;

	if ( it == zcommands.end() )
	{
		if ( doSelect )
		{
			currAppl->currPanel->remove_pd( err ) ;
			startApplication( selct ) ;
		}
		return ;
	}

	switch ( it->second )
	{
	case ZC_FIELDEXC:
		field_name = currAppl->currPanel->field_getname_input( row, col ) ;
		if ( field_name == "" )
		{
			issueMessage( "PSYS012K" ) ;
			return ;
		}
		fxc = currAppl->currPanel->field_getexec( field_name ) ;
		if ( fxc.fieldXct_command != "" )
		{
			executeFieldCommand( field_name, fxc, col ) ;
		}
		else
		{
			issueMessage( "PSYS012J" ) ;
		}
		return ;

	case ZC_MSGID:
		if ( zparm == "" )
		{
			w1 = p_poolMGR->get( err, currScrn->screenid(), "ZMSGID" ) ;
			p_poolMGR->put( err, "ZMSGID", w1, SHARED, SYSTEM ) ;
			issueMessage( "PSYS012L" ) ;
		}
		else if ( zparm == "ON" )
		{
			p_poolMGR->put( err, currScrn->screenid(), "ZSHMSGID", "Y" ) ;
		}
		else if ( zparm == "OFF" )
		{
			p_poolMGR->put( err, currScrn->screenid(), "ZSHMSGID", "N" ) ;
		}
		else
		{
			issueMessage( "PSYS012M" ) ;
		}
		break  ;

	case ZC_NOP:
		return ;

	case ZC_PANELID:
		if ( zparm == "" )
		{
			w1 = p_poolMGR->get( err, currScrn->screenid(), "ZSHPANID" ) ;
			zparm = ( w1 == "Y" ) ? "OFF" : "ON" ;
		}
		if ( zparm == "ON" )
		{
			p_poolMGR->put( err, currScrn->screenid(), "ZSHPANID", "Y" ) ;
		}
		else if ( zparm == "OFF" )
		{
			p_poolMGR->put( err, currScrn->screenid(), "ZSHPANID", "N" ) ;
		}
		else
		{
			issueMessage( "PSYS014" ) ;
		}
		currAppl->display_id() ;
		break  ;

	case ZC_DOT_REFRESH:
		screenRefresh() ;
		break  ;

	case ZC_RESIZE:
		currAppl->toggle_fscreen() ;
		break  ;

	case ZC_SWAPBAR:
		if ( zparm == "" )
		{
			zparm = ( p_poolMGR->sysget( err, "ZSWPBR", PROFILE ) == "Y" ) ? "OFF" : "ON" ;
		}
		if ( zparm == "ON" )
		{
			p_poolMGR->sysput( err, "ZSWPBR", "Y", PROFILE ) ;
			pLScreen::swapbar = true ;
		}
		else if ( zparm == "OFF" )
		{
			p_poolMGR->sysput( err, "ZSWPBR", "N", PROFILE ) ;
			pLScreen::swapbar = false ;
		}
		else
		{
			issueMessage( "PSYS014B" ) ;
		}
		currAppl->build_pfkeys( true ) ;
		currAppl->display_pfkeys() ;
		break  ;

	case ZC_TDOWN:
		currAppl->currPanel->field_tab_down( row, col ) ;
		currAppl->set_pcursor( row, col ) ;
		return ;

	case ZC_DOT_ABEND:
		currAppl->set_forced_abend() ;
		ResumeApplicationAndWait()   ;
		if ( currAppl->abnormalEnd )
		{
			terminateApplication() ;
			if ( pLScreen::screensTotal == 0 ) { return ; }
		}
		return ;

	case ZC_DOT_AUTO:
		autoUpdate() ;
		return ;

	case ZC_DOT_DISCARD:
		currAppl->currPanel->refresh_fields( err ) ;
		break  ;

	case ZC_DOT_ENQ:
		currAppl->show_enqueues() ;
		break ;

	case ZC_DOT_HIDE:
		if ( zparm == "NULLS" )
		{
			p_poolMGR->sysput( err, "ZNULLS", "NO", SHARED ) ;
			field::field_nulls = false ;
			currAppl->currPanel->redraw_fields( err ) ;
		}
		else
		{
			issueMessage( "PSYS012R" ) ;
		}
		break  ;

	case ZC_DOT_INFO:
		currAppl->info() ;
		break  ;

	case ZC_DOT_LOAD:
		for ( auto& app : apps )
		{
			if ( !app.second.errors && !app.second.dlopened )
			{
				loadDynamicClass( app.first ) ;
			}
		}
		break  ;

	case ZC_DOT_RELOAD:
		reloadDynamicClasses( zparm ) ;
		break  ;

	case ZC_DOT_RGB:
		llog( "I", ".RGB" <<endl ) ;
		llog( "-", "**************************************" <<endl ) ;
		llog( "-", "Listing current RGB values:" <<endl) ;
		for ( int i = 1 ; i < COLORS ; ++i )
		{
			color_content( i, &r, &g, &b ) ;
			llog( "-", "   Colour "<< i << " R: " << d2ds( r, 4 ) <<
						       " G: " << d2ds( g, 4 ) <<
						       " B: " << d2ds( b, 4 ) <<endl ) ;
		}
		llog( "-", "**************************************" <<endl ) ;
		break  ;

	case ZC_DOT_SCALE:
		if ( zparm == "" ) { zparm = "ON" ; }
		if ( findword( zparm, "ON OFF" ) )
		{
			p_poolMGR->sysput( err, "ZSCALE", zparm, SHARED ) ;
		}
		break  ;

	case ZC_DOT_SHELL:
		w1 = p_poolMGR->sysget( err, "ZSHELL", SHARED ) ;
		if ( w1 == "" )
		{
			issueMessage( "PSYS013E" ) ;
			break ;
		}
		def_prog_mode() ;
		endwin() ;
		std::system( w1.c_str() ) ;
		reset_prog_mode() ;
		refresh() ;
		break ;

	case ZC_DOT_SHOW:
		if ( zparm == "NULLS" )
		{
			p_poolMGR->sysput( err, "ZNULLS", "YES", SHARED ) ;
			field::field_nulls = true ;
			currAppl->currPanel->redraw_fields( err ) ;
		}
		else
		{
			issueMessage( "PSYS013A" ) ;
		}
		break ;

	case ZC_DOT_STATS:
		p_poolMGR->statistics() ;
		p_tableMGR->statistics() ;
		break ;

	case ZC_DOT_SNAP:
		p_poolMGR->snap() ;
		break ;

	case ZC_DOT_TASKS:
		listBackTasks() ;
		break ;

	case ZC_DOT_DEBUG:
		if ( zparm == "ON" || zparm == "" )
		{
			currAppl->setDebugMode() ;
		}
		else if ( zparm == "OFF" )
		{
			currAppl->clearDebugMode() ;
		}
		else if ( isnumeric( zparm ) )
		{
			currAppl->setDebugLevel( zparm ) ;
		}
		else
		{
			issueMessage( "PSYS014N" ) ;
		}
		break ;
	}

	currAppl->set_lcursor_home() ;
	currScrn->set_cursor( currAppl ) ;
}


bool resolveZCTEntry( string& cmdVerb,
		      string& cmdParm )
{
	//
	// Search for command tables in ZTLIB.
	// System commands should be in ZUPROF but user/site command tables might not be.
	//
	// Do not try to load the application command table as this will have been loaded
	// during SELECT processing if it exists.  This is always the first in the list of command tables.
	//
	// ALIAS parameters in the command table take precedence over command line parameters.
	//

	int i ;

	size_t j ;

	bool found = false ;

	string ztlib   ;
	string cmdtabl ;

	vector<string>cmdtabls ;
	set<string>processed   ;

	errblock err1 ;

	zctverb  = "" ;
	zcttrunc = "" ;
	zctact   = "" ;
	zctdesc  = "" ;

	cmdtabl = ( currAppl->get_applid() == "ISP" ) ? "N/A" : currAppl->get_applid() ;
	cmdtabls.push_back( cmdtabl ) ;
	processed.insert( cmdtabl ) ;

	for ( i = 1 ; i < 4 ; ++i )
	{
		cmdtabl = p_poolMGR->sysget( err, "ZUCMDT" + d2ds( i ), PROFILE ) ;
		if ( cmdtabl != "" && processed.count( cmdtabl ) == 0 )
		{
			cmdtabls.push_back( cmdtabl ) ;
		}
		processed.insert( cmdtabl ) ;
	}

	if ( p_poolMGR->sysget( err, "ZSCMDTF", PROFILE ) != "Y" )
	{
		  cmdtabls.push_back( "ISP" ) ;
		  processed.insert( "ISP" ) ;
	}

	for ( i = 1 ; i < 4 ; ++i )
	{
		cmdtabl = p_poolMGR->sysget( err, "ZSCMDT" + d2ds( i ), PROFILE ) ;
		if ( cmdtabl != "" && processed.count( cmdtabl ) == 0 )
		{
			cmdtabls.push_back( cmdtabl ) ;
		}
		processed.insert( cmdtabl ) ;
	}

	if ( processed.count( "ISP" ) == 0 )
	{
		  cmdtabls.push_back( "ISP" ) ;
	}

	ztlib = p_poolMGR->sysget( err, "ZTLIB", PROFILE ) ;

	for ( i = 0 ; i < 256 ; ++i )
	{
		for ( j = 0 ; j < cmdtabls.size() ; ++j )
		{
			err1.clear() ;
			p_tableMGR->cmdsearch( err1, &funcPOOL, cmdtabls[ j ], cmdVerb, ztlib, ( j > 0 ) ) ;
			if ( err1.error() )
			{
				llog( "E", "Error received searching for command "+ cmdVerb << "." << endl ) ;
				llog( "E", "Table name : "+ cmdtabls[ j ] << endl ) ;
				llog( "E", "Path list  : "+ ztlib << endl ) ;
				listErrorBlock( err1 ) ;
				continue ;
			}
			if ( !err1.RC0() || zctact == "" ) { continue ; }
			if ( zctact.front() == '&' )
			{
				currAppl->vcopy( substr( zctact, 2 ), zctact, MOVE ) ;
				if ( zctact == "" ) { found = false ; continue ; }
			}
			found = true ;
			break ;
		}
		if ( err1.getRC() > 0 || word( zctact, 1 ) != "ALIAS" ) { break ; }
		cmdVerb = word( zctact, 2 )    ;
		if ( subword( zctact, 3 ) != "" )
		{
			cmdParm = subword( zctact, 3 ) ;
		}
		zcommand = cmdVerb + " " + cmdParm ;
	}

	if ( i > 255 )
	{
		llog( "E", "ALIAS dept cannot be greater than 256.  Terminating search." << endl ) ;
		found = false ;
	}

	return found ;
}


void processPGMSelect()
{
	//
	// Called when an application program has invoked the SELECT service (also VIEW, EDIT, BROWSE).
	//

	selobj selct = currAppl->get_select_cmd() ;

	resolvePGM( selct, currAppl ) ;

	if ( selct.backgrd )
	{
		startApplicationBack( selct, currAppl ) ;
		ResumeApplicationAndWait() ;
	}
	else
	{
		selct.quiet = true ;
		startApplication( selct ) ;
		checkStartApplication( selct ) ;
	}
}


void resolvePGM( selobj& selct,
		 pApplication* p )
{
	//
	// Add application to run for SELECT PANEL() and SELECT CMD() services.
	//

	switch ( selct.pgmtype )
	{
	case PGM_REXX:
		p->vcopy( "ZOREXPGM", selct.pgm, MOVE ) ;
		break ;

	case PGM_PANEL:
		p->vcopy( "ZPANLPGM", selct.pgm, MOVE ) ;
		selct.nocheck = true ;
		break ;

	case PGM_SHELL:
		p->vcopy( "ZSHELPGM", selct.pgm, MOVE ) ;
		break ;

	default:
		if ( selct.pgm.size() > 0 && selct.pgm.front() == '&' )
		{
			p->vcopy( substr( selct.pgm, 2 ), selct.pgm, MOVE ) ;
		}
	}
}


void startApplication( selobj& selct,
		       bool nScreen,
		       bool pstop )
{
	//
	// Start an application using the passed SELECT object, on a new logical screen if specified.
	//
	// If the program is ISPSTRT, start the application in the PARM field on a new logical screen
	// or start GMAINPGM.  Force NEWPOOL option regardless of what is coded in the command.
	// PARM can be a command table entry, a PGM()/CMD()/PANEL() statement or an option for GMAINPGM.
	//
	// If requested and currAppl is terminating, remove the current application before dispatching the new one.
	// (for removing ended select panels where ZPRIM = 'NO' after command stacking).
	//
	// Select errors.  RSN=998 Module not found.
	//                 RSN=996 Load errors.
	//

	int i ;
	int iopt ;
	int spool ;

	uint row ;
	uint col ;

	string t     ;
	string opt   ;
	string sname ;
	string applid ;

	bool setMessage ;
	bool clrMessage = false ;
	bool newProfile = false ;

	fPOOL* funcPool = nullptr ;

	err.clear() ;

	pApplication* oldAppl = currAppl ;
	pApplication* newAppl ;

	boost::thread* pThread ;

	if ( selct.pgm == "ISPSTRT" )
	{
		currAppl->clear_msg() ;
		nScreen = ( pLScreen::screensTotal < ZMAXSCRN ) ;
		opt     = upper( selct.parm ) ;
		if ( !selct.parse( err, selct.parm ) )
		{
			selct.def( gmainpgm ) ;
			commandStack = opt + commandStack ;
		}
		else
		{
			resolvePGM( selct, currAppl ) ;
		}
		selct.newpool = true ;
		selct.nofunc  = false ;
	}

	if ( nScreen )
	{
		selct.newpool = true ;
		selct.nofunc  = false ;
	}
	else if ( selct.nofunc )
	{
		funcPool = oldAppl->get_fpool_addr() ;
	}

	if ( apps.find( selct.pgm ) == apps.end() )
	{
		selct.errors = true ;
		selct.rsn    = 998  ;
		t = "Application '"+ selct.pgm +"' not found" ;
		if ( selct.quiet )
		{
			llog( "E", t << endl ) ;
		}
		else
		{
			errorScreen( 1, t ) ;
		}
		return ;
	}

	if ( !apps[ selct.pgm ].dlopened )
	{
		if ( !loadDynamicClass( selct.pgm ) )
		{
			selct.errors  = true ;
			selct.rsn     = 996  ;
			t = "Errors loading application "+ selct.pgm ;
			if ( selct.quiet )
			{
				llog( "E", t << endl ) ;
			}
			else
			{
				errorScreen( 1, t ) ;
			}
			return ;
		}
	}

	if ( nScreen && !createLogicalScreen() ) { return ; }

	applid = ( selct.newappl == "" ) ? oldAppl->get_applid() : selct.newappl ;
	p_poolMGR->createProfilePool( err, applid ) ;
	if ( err.error() )
	{
		if ( err.msgid == "" )
		{
			errorScreen( 1, "An error has occured loading profile "+ applid ) ;
		}
		else
		{
			errorScreen( "Error loading profile", "" ) ;
		}
		return ;
	}
	else if ( err.RC4() )
	{
		newProfile = true ;
	}

	currAppl->store_Zvars() ;
	spool      = currAppl->shrdPool ;
	setMessage = currAppl->setMessage ;
	sname      = p_poolMGR->get( err, "ZSCRNAME", SHARED ) ;
	if ( setMessage )
	{
		currAppl->setMessage = false ;
	}

	updateDefaultVars() ;

	t = ( selct.parm.size() > 100 ) ? selct.parm.substr( 0, 100 ) + "..." : selct.parm ;
	llog( "I", "Starting application "+ selct.pgm +" with parameters '"+ t +"'" << "." << endl ) ;
	newAppl = ((pApplication*(*)())( apps[ selct.pgm ].maker_ep))() ;

	newAppl->init_phase1( selct, ++maxTaskid, funcPool, lspfCallbackHandler ) ;

	apps[ selct.pgm ].refCount++ ;

	err.settask( newAppl->taskid() ) ;

	newAppl->propagateEnd = oldAppl->propagateEnd ;
	newAppl->jumpEntered  = oldAppl->jumpEntered  ;
	newAppl->p_lss        = currScrn->get_lss() ;

	if ( selct.newpool )
	{
		if ( currScrn->application_stack_size() > 1 && selct.scrname == "" )
		{
			selct.scrname = sname ;
		}
		spool = p_poolMGR->createSharedPool() ;
	}

	p_poolMGR->connect( newAppl->taskid(), applid, spool ) ;
	if ( newProfile )
	{
		createpfKeyDefaults() ;
	}

	createSharedPoolVars( applid ) ;

	newAppl->shrdPool = spool ;
	newAppl->init_phase2() ;

	newAppl->set_exhelp( oldAppl->get_exhelp() ) ;

	pPanel::pfkgToken       = pVPOOL::pfkgToken + Table::pflgToken ;
	pApplication::pflgToken = Table::pflgToken ;

	if ( !selct.suspend )
	{
		newAppl->set_addpop( oldAppl ) ;
	}

	if ( !nScreen && ( selct.passlib || selct.newappl == "" ) )
	{
		newAppl->set_zlibd_altlib( selct.passlib, oldAppl ) ;
	}

	if ( setMessage )
	{
		newAppl->set_msg1( oldAppl->getmsg1(), oldAppl->getmsgid1() ) ;
	}
	else if ( !nScreen && !selct.fstack )
	{
		clrMessage = true ;
	}

	if ( pstop && currAppl->terminateAppl )
	{
		terminateApplication() ;
		err.settask( newAppl->taskid() ) ;
		oldAppl = nullptr ;
	}

	currScrn->application_add( newAppl ) ;

	currAppl = newAppl ;

	currAppl->loadCommandTable() ;
	pThread = new boost::thread( &pApplication::run, currAppl ) ;

	currAppl->pThread = pThread ;

	if ( selct.scrname != "" )
	{
		p_poolMGR->put( err, "ZSCRNAME", selct.scrname, SHARED ) ;
	}

	llog( "I", "Waiting for new application to complete startup.  ID=" << pThread->get_id() << "." << endl ) ;

	boost::mutex mutex ;
	while ( currAppl->busyAppl )
	{
		boost::mutex::scoped_lock lk( mutex ) ;
		cond_lspf.wait_for( lk, boost::chrono::milliseconds( 100 ) ) ;
		lk.unlock() ;
		if ( currAppl->busyAppl && isEscapeKey() )
		{
			iopt = listInterruptOptions() ;
			if ( iopt == 1 )
			{
				currAppl->set_self() ;
				i = pthread_kill( currAppl->pThread->native_handle(), SIGTERM ) ;
				llog( "W", "pthread_kill SIGTERM retun code = " << i << "." << endl ) ;
			}
			else if ( iopt == 2 )
			{
				currAppl->set_self() ;
				i = pthread_kill( currAppl->pThread->native_handle(), SIGUSR1 ) ;
				llog( "W", "pthread_kill SIGUSR1 retun code = " << i << "." << endl ) ;
			}
			else if ( iopt == 3 )
			{
				currAppl->set_timeout_abend() ;
				break ;
			}
			else if ( iopt == 5 )
			{
				listApplicationStatus() ;
			}
			pLScreen::show_busy() ;
			currScrn->get_cursor( row, col ) ;
			move( row, col ) ;
		}
	}

	if ( oldAppl )
	{
		oldAppl->set_clear_msg( clrMessage ) ;
	}

	if ( currAppl->do_refresh_lscreen() )
	{
		if ( lwin )
		{
			deleteLinePanel() ;
			ResumeApplicationAndWait() ;
		}
		screenRefresh() ;
	}

	if ( currAppl->abnormalEnd )
	{
		abnormalTermMessage() ;
	}
	else
	{
		llog( "I", "New thread and application started and initialised. ID=" << pThread->get_id() << "." << endl ) ;
		if ( currAppl->line_output_pending() )
		{
			lineOutput() ;
		}
		else if ( currAppl->line_input_pending() )
		{
			lineInput() ;
		}
	}

	currScrn->set_cursor( currAppl ) ;
}


void startApplicationBack( selobj selct,
			   pApplication* oldAppl,
			   bool pgmselect )
{
	//
	// Start a background application using the passed SELECT object.
	// Issue notify if application not found or errors loading program.
	//
	// Background task can be started synchronously or asynchronously.
	// If synchronous, parent needs to wait for the child to terminate before resuming.
	//

	int spool ;

	string applid ;
	string msg    ;

	errblock err1 ;

	pApplication* newAppl ;

	boost::thread* pThread ;

	if ( apps.find( selct.pgm ) == apps.end() )
	{
		msg = "Application '"+ selct.pgm +"' not found" ;
		oldAppl->notify( msg ) ;
		llog( "E", msg << "." << endl ) ;
		return ;
	}

	if ( !apps[ selct.pgm ].dlopened )
	{
		if ( !loadDynamicClass( selct.pgm ) )
		{
			msg = "Errors loading '"+ selct.pgm ;
			oldAppl->notify( msg ) ;
			llog( "E", msg << "." << endl ) ;
			return ;
		}
	}

	applid = ( selct.newappl == "" ) ? oldAppl->get_applid() : selct.newappl ;
	p_poolMGR->createProfilePool( err1, applid ) ;
	if ( err1.error() )
	{
		msg = "An error has occured loading profile "+ applid ;
		oldAppl->notify( msg ) ;
		llog( "E", msg << "." << endl ) ;
		listErrorBlock( err1 ) ;
		return ;
	}

	updateDefaultVars() ;

	selct.nofunc = false ;

	if ( !selct.nollog )
	{
		llog( "I", "Starting background application "+ selct.pgm +" with parameters '"+ selct.parm +"'" << "." << endl ) ;
	}
	newAppl = ((pApplication*(*)())( apps[ selct.pgm ].maker_ep))() ;

	newAppl->init_phase1( selct, ++maxTaskid, nullptr, lspfCallbackHandler ) ;

	apps[ selct.pgm ].refCount++ ;

	mtx.lock() ;
	pApplicationBackground.push_back( newAppl ) ;
	mtx.unlock() ;

	err1.settask( newAppl->taskid() ) ;

	spool = ( selct.newpool ) ? p_poolMGR->createSharedPool() : oldAppl->shrdPool ;

	if ( pgmselect )
	{
		oldAppl->vreplace( "ZSBTASK", newAppl->taskid() ) ;
	}

	p_poolMGR->connect( newAppl->taskid(), applid, spool ) ;

	createSharedPoolVars( applid ) ;

	if ( oldAppl->background() )
	{
		if ( selct.sync )
		{
			oldAppl->busyAppl = false ;
			oldAppl->SEL      = false ;
			newAppl->uAppl    = oldAppl ;
		}
		else
		{
			oldAppl->busyAppl = true ;
			cond_appl.notify_all() ;
		}
	}

	newAppl->shrdPool = spool ;
	newAppl->init_phase2() ;

	if ( selct.passlib || selct.newappl == "" )
	{
		newAppl->set_zlibd_altlib( selct.passlib, oldAppl ) ;
	}

	pThread = new boost::thread( &pApplication::run, newAppl ) ;

	newAppl->pThread = pThread ;

	if ( !selct.nollog )
	{
		llog( "I", "New background thread and application started and initialised. ID=" << pThread->get_id() << "." << endl ) ;
	}
}


void processBackgroundTasks()
{
	//
	// This routine runs in a separate thread to check if there are any
	// background tasks waiting for action:
	//    Cleanup application if ended.
	//    Start application if SELECT or SUBMIT done in the background program.
	//
	// Any routine this procedure calls that uses the pApplication object, must have
	// the address passed as it won't be currAppl.
	//

	backStatus = BACK_RUNNING ;

	boost::mutex mutex ;

	while ( lspfStatus == LSPF_RUNNING )
	{
		boost::mutex::scoped_lock lk( mutex ) ;
		cond_batch.wait_for( lk, boost::chrono::milliseconds( 100 ) ) ;
		lk.unlock() ;
		mtx.lock() ;
		for ( auto it = pApplicationBackground.begin() ; it != pApplicationBackground.end() ; ++it )
		{
			while ( (*it)->terminateAppl )
			{
				terminateApplicationBack( *it ) ;
				it = pApplicationBackground.erase( it ) ;
				if ( it == pApplicationBackground.end() ) { break ; }
			}
			if ( it == pApplicationBackground.end() ) { break ; }
		}
		mtx.unlock() ;

		mtx.lock() ;
		vector<pApplication*> temp = pApplicationBackground ;

		for ( auto it = temp.begin() ; it != temp.end() ; ++it )
		{
			if ( (*it)->SEL && !currAppl->terminateAppl )
			{
				selobj selct = (*it)->get_select_cmd() ;
				resolvePGM( selct, *it ) ;
				startApplicationBack( selct, *it ) ;
			}
		}
		mtx.unlock() ;

		if ( backStatus == BACK_STOPPING )
		{
			llog( "I", "lspf shutting down.  Removing background applications." << endl ) ;
			for ( auto it = pApplicationBackground.begin() ; it != pApplicationBackground.end() ; ++it )
			{
				terminateApplicationBack( *it ) ;
			}
		}
	}

	llog( "I", "Background job monitor task stopped." << endl ) ;
	backStatus = BACK_STOPPED ;
}


void checkStartApplication( const selobj& selct )
{
	if ( selct.errors )
	{
		currAppl->RC      = 20  ;
		currAppl->ZRC     = 20  ;
		currAppl->ZRSN    = selct.rsn ;
		currAppl->ZRESULT = "PSYS013J" ;
		if ( !currAppl->errorsReturn() )
		{
			currAppl->abnormalEnd = true ;
		}
		ResumeApplicationAndWait() ;
	}
}


void terminateApplication()
{
	//
	// Terminate the current application (currAppl) and perform cleanup.
	//

	int tRC  ;
	int tRSN ;

	uint row ;
	uint col ;

	string zappname ;
	string tRESULT ;
	string tMSGID1 ;
	string fname ;
	string delm ;

	bool refList ;
	bool zexpand ;
	bool propagateEnd ;
	bool abnormalEnd ;
	bool jumpEntered ;
	bool setMessage ;
	bool nested ;
	bool setCursorHome ;
	bool tutorial ;

	bool clrMessage = true ;
	bool nretError  = false ;
	bool nocheck    = false ;
	bool resume     = false ;

	slmsg tMSG1 ;

	err.clear() ;

	pApplication* prvAppl ;

	boost::thread* pThread ;

	llog( "I", "Application terminating "+ currAppl->get_appname() +" ID: "<< currAppl->taskid() << "." << endl ) ;

	zappname = currAppl->get_appname() ;

	setCursorHome = currAppl->zhelppgm_home() ;

	tRC         = currAppl->ZRC ;
	tRSN        = currAppl->ZRSN ;
	tRESULT     = currAppl->ZRESULT ;
	nocheck     = currAppl->get_nocheck() ;
	tutorial    = currAppl->tutorial ;
	abnormalEnd = currAppl->abnormalEnd ;

	refList = ( currAppl->reffield == "#REFLIST" ) ;
	zexpand = currAppl->get_zexpand() ;

	setMessage = currAppl->setMessage ;
	if ( setMessage )
	{
		tMSGID1 = currAppl->getmsgid1() ;
		tMSG1   = currAppl->getmsg1() ;
	}

	jumpEntered  = currAppl->jumpEntered ;
	propagateEnd = currAppl->propagateEnd && ( currScrn->application_stack_size() > 1 ) ;
	nested       = currAppl->is_nested() ;

	pThread = currAppl->pThread ;

	if ( currAppl->abnormalEnd )
	{
		while ( currAppl->cleanupRunning() )
		{
			boost::this_thread::sleep_for( boost::chrono::milliseconds( 5 ) ) ;
		}
		pThread->detach() ;
	}

	p_poolMGR->disconnect( currAppl->taskid() ) ;

	currScrn->application_remove_current() ;
	if ( !currScrn->application_stack_empty() && currAppl->newappl == "" )
	{
		prvAppl = currScrn->application_get_current() ;
		prvAppl->set_zlibd_altlib( currAppl ) ;
	}

	if ( currAppl->abnormalTimeout )
	{
		llog( "I", "Moving application instance of "+ zappname +" to timeout queue." << endl ) ;
		pApplicationTimeout.push_back( currAppl ) ;
	}
	else
	{
		llog( "I", "Removing application instance of "+ zappname << "." << endl ) ;
		((void (*)(pApplication*))(apps[ zappname ].destroyer_ep))( currAppl ) ;
		delete pThread ;
		apps[ zappname ].refCount-- ;
	}

	if ( currScrn->application_stack_empty() )
	{
		p_poolMGR->destroyPool( currScrn->screenid() ) ;
		if ( pLScreen::screensTotal == 1 )
		{
			delete currScrn ;
			return ;
		}
		deleteLogicalScreen() ;
		setCursorHome = false ;
	}

	currAppl = currScrn->application_get_current() ;
	err.settask( currAppl->taskid() ) ;

	currAppl->display_pd( err ) ;

	p_poolMGR->put( err, "ZPANELID", currAppl->get_panelid(), SHARED, SYSTEM ) ;

	if ( apps[ zappname ].refCount == 0 && apps[ zappname ].relPending )
	{
		apps[ zappname ].relPending = false ;
		llog( "I", "Reloading module "+ zappname +" (pending reload status)." << endl ) ;
		if ( loadDynamicClass( zappname ) )
		{
			llog( "I", "Loaded "+ zappname +" (module "+ apps[ zappname ].module +") from "+ apps[ zappname ].file << "." << endl ) ;
		}
		else
		{
			llog( "W", "Errors occured loading "+ zappname +"  Module removed." << endl ) ;
		}
	}

	if ( refList )
	{
		if ( currAppl->nretriev_on() )
		{
			fname = currAppl->currPanel->field_getname_nocmd() ;
			if ( fname == "" )
			{
				fname = currAppl->get_nretfield() ;
			}
			if ( fname != "" )
			{
				if ( currAppl->currPanel->field_exists( fname ) )
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
					llog( "E", "Invalid field "+ fname +" in .NRET panel statement." << endl ) ;
					issueMessage( "PSYS011Z" ) ;
					nretError  = true ;
					clrMessage = false ;
				}
			}
			else
			{
				issueMessage( "PSYS011Y" ) ;
				nretError  = true ;
				clrMessage = false ;
			}
		}
		else
		{
			issueMessage( "PSYS011X" ) ;
			nretError  = true ;
			clrMessage = false ;
		}
	}

	if ( currAppl->reffield != "" && !nretError )
	{
		if ( tRC == 0 )
		{
			if ( currAppl->currPanel->field_get_row_col( currAppl->reffield, row, col ) )
			{
				currAppl->currPanel->field_setvalue( currAppl->reffield, tRESULT ) ;
				currAppl->currPanel->cursor_eof( row, col ) ;
				currAppl->currPanel->set_pcursor( row, col ) ;
				if ( refList )
				{
					issueMessage( "PSYS011W" ) ;
				}
				setCursorHome = false ;
				clrMessage    = false ;
			}
			else
			{
				llog( "E", "Invalid field "+ currAppl->reffield +" in .NRET panel statement." << endl )   ;
				issueMessage( "PSYS011Z" ) ;
				clrMessage  = false ;
			}
		}
		else if ( tRC == 4 )
		{
			setCursorHome = false ;
		}
		else if ( tRC == 8 )
		{
			beep() ;
			setCursorHome = false ;
		}
		currAppl->reffield = "" ;
	}

	if ( zexpand )
	{
		if ( tRC == 0 )
		{
			currAppl->currPanel->field_setvalue( word( tRESULT, 1 ), ( tRESULT.size() > 13 ) ? tRESULT.substr( 13 ) : "" ) ;
		}
		setCursorHome = false ;
	}


	if ( currAppl->isprimMenu() && propagateEnd )
	{
		propagateEnd = false ;
		commandStack = ( jumpEntered ) ? jumpOption : returnOption ;
		jumpOption   = "" ;
		returnOption = "" ;
	}

	if ( currAppl->SEL )
	{
		if ( abnormalEnd )
		{
			propagateEnd  = false ;
			currAppl->RC  = 20 ;
			currAppl->ZRC = 20 ;
			if ( tRC == 20 && tRSN > 900 )
			{
				currAppl->ZRSN    = tRSN    ;
				currAppl->ZRESULT = tRESULT ;
			}
			else
			{
				currAppl->ZRSN    = 999 ;
				currAppl->ZRESULT = "Abended" ;
			}
			if ( !currAppl->errorsReturn() )
			{
				currAppl->abnormalEnd = true ;
			}
			pApplication::controlNonDispl = false ;
		}
		else
		{
			if ( nested && !jumpEntered )
			{
				propagateEnd = false ;
				currAppl->set_userAction( USR_ENTER ) ;
			}
			if ( propagateEnd )
			{
				currAppl->jumpEntered = jumpEntered ;
				currAppl->RC = 4 ;
			}
			else
			{
				currAppl->RC = 0 ;
			}
			currAppl->ZRC     = tRC     ;
			currAppl->ZRSN    = tRSN    ;
			currAppl->ZRESULT = tRESULT ;
		}
		if ( setMessage )
		{
			currAppl->set_msg1( tMSG1, tMSGID1 ) ;
		}
		currAppl->propagateEnd = propagateEnd ;
		resume = true ;
	}
	else
	{
		currAppl->propagateEnd = false ;
		currAppl->set_userAction( USR_ENTER ) ;
		if ( propagateEnd && ( !nested || jumpEntered ) )
		{
			currAppl->jumpEntered  = jumpEntered ;
			currAppl->propagateEnd = true ;
			ResumeApplicationAndWait() ;
		}
		else if ( propagateEnd && !jumpEntered )
		{
			commandStack = returnOption ;
			returnOption = "" ;
		}
		if ( setMessage )
		{
			currAppl->set_msg1( tMSG1, tMSGID1 ) ;
		}
		else if ( tRC == 20 && nocheck )
		{
			currAppl->currPanel->cmd_setvalue( currAppl->get_selopt() ) ;
			issueMessage( "PSYS016" ) ;
			clrMessage = false ;
		}
		if ( setCursorHome )
		{
			currAppl->set_lcursor_home() ;
		}
		if ( !currAppl->propagateEnd && pApplication::lineInOutDone )
		{
			deleteLinePanel() ;
			pApplication::lineInOutDone = false ;
			doupdate() ;
		}
		if ( currAppl->do_clear_msg() && clrMessage && !tutorial )
		{
			currAppl->clear_msg() ;
		}
		currAppl->redraw_panel( err ) ;
	}

	llog( "I", "Application termination of "+ zappname +" completed.  Current application is "+ currAppl->get_appname() << "." << endl ) ;

	currAppl->restore_Zvars() ;

	currAppl->display_id() ;

	if ( resume )
	{
		ResumeApplicationAndWait() ;
	}

	currScrn->set_cursor( currAppl ) ;

	pPanel::pfkgToken       = pVPOOL::pfkgToken + Table::pflgToken ;
	pApplication::pflgToken = Table::pflgToken ;

	currAppl->reload_keylist() ;
	currAppl->build_pfkeys() ;
	currAppl->display_pfkeys() ;

	if ( tutorial && currAppl->currPanel )
	{
		currAppl->currPanel->reset_cmd() ;
	}
}


void terminateApplicationBack( pApplication* p )
{
	//
	// If uAppl is set, the terminated application was started synchronously so the
	// parent needs to resume processing.  Also pass back execution results and LIBDEF changes.
	//

	int tRC  ;
	int tRSN ;

	string tRESULT ;

	bool abnormalEnd ;

	boost::thread* pThread ;
	pApplication* uAppl ;

	if ( !p->nollog )
	{
		llog( "I", "Application terminating " + p->get_appname() + " ID: "<< p->taskid() << "." << endl ) ;
		llog( "I", "Removing background application instance of " + p->get_appname() + " taskid: " << p->taskid() << "." << endl ) ;
	}

	pThread = p->pThread ;
	uAppl   = p->uAppl ;
	tRC     = p->ZRC  ;
	tRSN    = p->ZRSN ;
	tRESULT = p->ZRESULT ;
	abnormalEnd = p->abnormalEnd ;

	if ( uAppl && !abnormalEnd && p->newappl == "" )
	{
		uAppl->set_zlibd_altlib( p ) ;
	}

	p_poolMGR->disconnect( p->taskid() ) ;
	apps[ p->get_appname() ].refCount-- ;

	((void (*)(pApplication*))(apps[ p->get_appname() ].destroyer_ep))( p ) ;

	delete pThread ;

	if ( uAppl )
	{
		if ( abnormalEnd )
		{
			uAppl->RC  = 20 ;
			uAppl->ZRC = 20 ;
			if ( tRC == 20 && tRSN > 900 )
			{
				uAppl->ZRSN    = tRSN    ;
				uAppl->ZRESULT = tRESULT ;
			}
			else
			{
				uAppl->ZRSN    = 999 ;
				uAppl->ZRESULT = "Abended" ;
			}
			if ( !uAppl->errorsReturn() )
			{
				uAppl->abnormalEnd = true ;
			}
		}
		else
		{
			uAppl->RC      = 0 ;
			uAppl->ZRC     = tRC     ;
			uAppl->ZRSN    = tRSN    ;
			uAppl->ZRESULT = tRESULT ;
		}
		uAppl->busyAppl = true ;
		cond_appl.notify_all() ;
	}
}


bool createLogicalScreen()
{
	err.clear() ;

	if ( !pApplication::controlSplitEnable )
	{
		p_poolMGR->put( err, "ZCTMVAR", left( zcommand, 8 ), SHARED ) ;
		issueMessage( "PSYS011" ) ;
		return false ;
	}

	if ( pLScreen::screensTotal == ZMAXSCRN )
	{
		issueMessage( "PSYS011D" ) ;
		return false ;
	}

	currScrn->save_panel_stack() ;
	altScreen = priScreen ;
	priScreen = screenList.size() ;

	screenList.push_back( new pLScreen( currScrn->screenid() ) ) ;
	pLScreen::OIA_startTime() ;

	lScreenDefaultSettings() ;

	return true ;
}


void deleteLogicalScreen()
{
	//
	// Delete a logical screen and set the active screen to the one that opened it (or its predecessor if
	// that too has been closed).
	//

	int openedBy = currScrn->get_openedBy() ;

	delete currScrn ;

	screenList.erase( screenList.begin() + priScreen ) ;

	if ( altScreen > priScreen ) { --altScreen ; }

	priScreen = pLScreen::get_priScreen( openedBy ) ;

	if ( priScreen == altScreen )
	{
		altScreen = ( priScreen == 0 ) ? pLScreen::screensTotal - 1 : 0 ;

	}

	currScrn = screenList[ priScreen ] ;
	currScrn->restore_panel_stack() ;
	pLScreen::OIA_startTime() ;

	p_poolMGR->sysput( err, "ZSPLIT", ( pLScreen::screensTotal > 1 ) ? "YES" : "NO", SHARED ) ;
}


void ResumeApplicationAndWait()
{
	int i ;
	int iopt ;

	uint row ;
	uint col ;

	pPanel* prevPanel = currAppl->currPanel ;

	displayNotifies() ;

	if ( currAppl->applicationEnded ) { return ; }

	if ( currAppl->simulate_end() )
	{
		currAppl->set_userAction( USR_END ) ;
		p_poolMGR->put( err, "ZVERB", "END", SHARED ) ;
	}

	currAppl->busyAppl = true ;
	cond_appl.notify_all() ;
	boost::mutex mutex ;

	while ( currAppl->busyAppl )
	{
		boost::mutex::scoped_lock lk( mutex ) ;
		cond_lspf.wait_for( lk, boost::chrono::milliseconds( 100 ) ) ;
		lk.unlock() ;
		if ( currAppl->busyAppl && word( currAppl->get_status(), 1 ) == "Waiting" )
		{
			cond_appl.notify_all() ;
		}
		if ( currAppl->busyAppl && isEscapeKey() )
		{
			iopt = listInterruptOptions() ;
			if ( iopt == 1 )
			{
				currAppl->set_self() ;
				i = pthread_kill( currAppl->pThread->native_handle(), SIGTERM ) ;
				llog( "W", "pthread_kill SIGTERM retun code = " << i << "." << endl ) ;
			}
			else if ( iopt == 2 )
			{
				currAppl->set_self() ;
				i = pthread_kill( currAppl->pThread->native_handle(), SIGUSR1 ) ;
				llog( "W", "pthread_kill SIGUSR1 retun code = " << i << "." << endl ) ;
			}
			else if ( iopt == 3 )
			{
				currAppl->set_timeout_abend() ;
				break ;
			}
			else if ( iopt == 4 )
			{
				cond_appl.notify_all() ;
			}
			else if ( iopt == 5 )
			{
				listApplicationStatus() ;
			}
			pLScreen::show_busy() ;
			currScrn->set_cursor( currAppl ) ;
			currScrn->get_cursor( row, col ) ;
			move( row, col ) ;
		}
	}

	currAppl->remove_pd( prevPanel ) ;

	if ( currAppl->reloadCUATables ) { loadCUATables() ; }
	if ( currAppl->do_refresh_lscreen() )
	{
		if ( lwin )
		{
			deleteLinePanel() ;
			ResumeApplicationAndWait() ;
		}
		screenRefresh() ;
	}

	if ( currAppl->abnormalEnd )
	{
		abnormalTermMessage() ;
	}
	else if ( currAppl->line_output_pending() )
	{
		lineOutput() ;
	}
	else if ( currAppl->line_input_pending() )
	{
		lineInput() ;
	}

	pPanel::pfkgToken       = pVPOOL::pfkgToken + Table::pflgToken ;
	pApplication::pflgToken = Table::pflgToken ;

	currAppl->reload_keylist() ;
	currAppl->build_pfkeys() ;
	currAppl->display_pfkeys() ;
}


void loadCUATables()
{
	//
	// Set according to the ZC variables in ISPS profile.
	//

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

	setColourPair( "IWF" )  ;

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

	setDecolourisedColour() ;

	setGlobalColours() ;
	setRGBValues() ;
}


void setColourPair( const string& name )
{
	string t ;

	err.clear() ;

	pair<map<attType, unsigned int>::iterator, bool> result ;

	result = cuaAttr.insert( pair<attType, unsigned int>( cuaAttrName[ name ], WHITE ) ) ;

	map<attType, unsigned int>::iterator it = result.first ;

	t = p_poolMGR->sysget( err, "ZC"+ name, PROFILE ) ;
	if ( !err.RC0() )
	{
		llog( "E", "Variable ZC"+ name +" not found in ISPS profile." << endl ) ;
		llog( "S", "Rerun setup program to re-initialise ISPS profile." << endl ) ;
		listErrorBlock( err ) ;
		abortStartup() ;
	}

	if ( t.size() != 3 )
	{
		llog( "E", "Variable ZC"+ name +" invalid value of "+ t +".  Must be length of three." << endl ) ;
		llog( "S", "Rerun setup program to re-initialise ISPS profile." << endl ) ;
		listErrorBlock( err ) ;
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

		default :  llog( "E", "Variable ZC"+ name +" has invalid value[0] "+ t << "." << endl ) ;
			   llog( "S", "Rerun setup program to re-initialise ISPS profile." << endl ) ;
			   abortStartup() ;
	}

	switch  ( t[ 1 ] )
	{
		case 'L':  it->second |= A_NORMAL ; break ;
		case 'H':  it->second |= A_BOLD   ; break ;

		default :  llog( "E", "Variable ZC"+ name +" has invalid value[1] "+ t << "." << endl ) ;
			   llog( "S", "Rerun setup program to re-initialise ISPS profile." << endl ) ;
			   abortStartup() ;
	}

	switch  ( t[ 2 ] )
	{
		case 'N':  break ;
		case 'B':  it->second |= A_BLINK     ; break ;
		case 'R':  it->second |= A_REVERSE   ; break ;
		case 'U':  it->second |= A_UNDERLINE ; break ;

		default :  llog( "E", "Variable ZC"+ name +" has invalid value[2] "+ t << "." << endl ) ;
			   llog( "S", "Rerun setup program to re-initialise ISPS profile." << endl ) ;
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
	init_pair( 1, gcolours[ t.front() ], COLOR_BLACK ) ;

	t = p_poolMGR->sysget( err, "ZGCLG", PROFILE ) ;
	init_pair( 2, gcolours[ t.front() ], COLOR_BLACK ) ;

	t = p_poolMGR->sysget( err, "ZGCLY", PROFILE ) ;
	init_pair( 3, gcolours[ t.front() ], COLOR_BLACK ) ;

	t = p_poolMGR->sysget( err, "ZGCLB", PROFILE ) ;
	init_pair( 4, gcolours[ t.front() ], COLOR_BLACK ) ;

	t = p_poolMGR->sysget( err, "ZGCLM", PROFILE ) ;
	init_pair( 5, gcolours[ t.front() ], COLOR_BLACK ) ;

	t = p_poolMGR->sysget( err, "ZGCLT", PROFILE ) ;
	init_pair( 6, gcolours[ t.front() ], COLOR_BLACK ) ;

	t = p_poolMGR->sysget( err, "ZGCLW", PROFILE ) ;
	init_pair( 7, gcolours[ t.front() ], COLOR_BLACK ) ;
}


void setDefaultRGB()
{
	short int r ;
	short int g ;
	short int b ;

	string t ;
	string n_rgb = "ZNRGB" ;
	string u_rgb = "ZURGB" ;

	if ( !can_change_color() )
	{
		return ;
	}

	for ( int i = 1 ; i < COLORS && colourValue.count( i ) > 0 ; ++i )
	{
		color_content( i, &r, &g, &b ) ;
		t = d2ds( r, 4 ) + d2ds( g, 4 ) + d2ds( b, 4 ) ;
		p_poolMGR->sysput( err, n_rgb + colourValue[ i ], t, SHARED ) ;
	}

	for ( int i = 1 ; i < COLORS && colourValue.count( i ) > 0 ; ++i )
	{
		t = p_poolMGR->sysget( err, u_rgb + colourValue[ i ], PROFILE ) ;
		if ( t.size() != 12 )
		{
			t = p_poolMGR->sysget( err, n_rgb + colourValue[ i ], SHARED ) ;
			p_poolMGR->sysput( err, u_rgb + colourValue[ i ], t, PROFILE ) ;
			continue ;
		}
		r = ds2d( t.substr( 0, 4 ) ) ;
		g = ds2d( t.substr( 4, 4 ) ) ;
		b = ds2d( t.substr( 8, 4 ) ) ;
		if ( init_color( i, r, g, b ) == ERR )
		{
			llog( "E", "Setting colour pair "<< i << " failed."<< endl ) ;
			llog( "E", "   R="<< r << endl ) ;
			llog( "E", "   G="<< g << endl ) ;
			llog( "E", "   B="<< b << endl ) ;
		}
	}
}


void setRGBValues()
{
	short int r ;
	short int g ;
	short int b ;

	string t ;
	string u_rgb = "ZURGB" ;

	if ( !can_change_color() )
	{
		return ;
	}

	for ( int i = 1 ; i < COLORS && colourValue.count( i ) > 0 ; ++i )
	{
		t = p_poolMGR->sysget( err, u_rgb+colourValue[ i ], PROFILE ) ;
		if ( t.size() != 12 )
		{
			continue ;
		}
		r = ds2d( t.substr( 0, 4 ) ) ;
		g = ds2d( t.substr( 4, 4 ) ) ;
		b = ds2d( t.substr( 8, 4 ) ) ;
		if ( init_color( i, r, g, b ) == ERR )
		{
			llog( "E", "Setting colour pair "<< i << " failed." << endl ) ;
			llog( "E", "   R="<< r << endl ) ;
			llog( "E", "   G="<< g << endl ) ;
			llog( "E", "   B="<< b << endl ) ;
		}
	}

	currScrn->refresh_panel_stack() ;
	pLScreen::OIA_refresh() ;
	redrawwin( stdscr ) ;
	wnoutrefresh( stdscr ) ;
}


void setDecolourisedColour()
{
	string t ;

	err.clear() ;

	t = p_poolMGR->sysget( err, "ZDECLRA", PROFILE ) ;
	if ( !err.RC0() )
	{
		llog( "E", "Variable ZDECLRA not found in ISPS profile." << endl ) ;
		llog( "S", "Rerun setup program to re-initialise ISPS profile." << endl ) ;
		listErrorBlock( err ) ;
		abortStartup() ;
	}

	if ( t.size() != 2 )
	{
		llog( "E", "Variable ZDCLRA invalid value of "+ t +".  Must be length of two." << endl ) ;
		llog( "S", "Rerun setup program to re-initialise ISPS profile." << endl ) ;
		listErrorBlock( err ) ;
		abortStartup() ;
	}

	switch  ( t[ 0 ] )
	{
		case 'R': decColour1 = 1 ;
			  decColour2 = RED ;
			  break ;

		case 'G': decColour1 = 2 ;
			  decColour2 = GREEN ;
			  break ;

		case 'Y': decColour1 = 3 ;
			  decColour2 = YELLOW ;
			  break ;

		case 'B': decColour1 = 4 ;
			  decColour2 = BLUE ;
			  break ;

		case 'M': decColour1 = 5 ;
			  decColour2 = MAGENTA ;
			  break ;

		case 'T': decColour1 = 6 ;
			  decColour2 = TURQ ;
			  break ;

		case 'W': decColour1 = 7 ;
			  decColour2 = WHITE ;
			  break ;

		default :  llog( "E", "Variable ZDECLRA has invalid value[0] "+ t << "." << endl ) ;
			   llog( "S", "Rerun setup program to re-initialise ISPS profile." << endl ) ;
			   abortStartup() ;
	}

	switch  ( t[ 1 ] )
	{
		case 'L':  decIntens = A_NORMAL ; break ;
		case 'H':  decIntens = A_BOLD   ; break ;

		default :  llog( "E", "Variable ZDECLRA has invalid value[1] "+ t << "." << endl ) ;
			   llog( "S", "Rerun setup program to re-initialise ISPS profile." << endl ) ;
			   abortStartup() ;
	}

}


void decolouriseScreen()
{
	if ( p_poolMGR->sysget( err, "ZDECLR", PROFILE ) == "Y" )
	{
		currScrn->decolourise_inactive( decColour1, decColour2, decIntens ) ;
	}
	else
	{
		currScrn->set_frames_inactive( intens ) ;
	}
}


void decolouriseAll()
{
	//
	// Decolourise all windows until a full screen window is encountered.
	//

	if ( p_poolMGR->sysget( err, "ZDECLA", PROFILE ) == "Y" )
	{
		currScrn->decolourise_all( decColour1, decColour2, decIntens ) ;
	}
}


void colouriseAll()
{
	//
	// Colourise only windows for the current application if ZDECLR is "Y" else
	// all windows for all applicattions on the logical screen until a full screen window
	// is encountered.
	//

	if ( p_poolMGR->sysget( err, "ZDECLA", PROFILE ) == "Y" )
	{
		if ( p_poolMGR->sysget( err, "ZDECLR", PROFILE ) == "Y" )
		{
			currAppl->redraw_panel( err ) ;
		}
		else
		{
			currScrn->colourise_all( err ) ;
		}
	}
}


void resizeTerminal()
{
	//
	// Set new screen size parameters after a terminal resize.
	// Recreate the OIA and SWB.
	//

	llog( "I", "NCURSES terminal has been resized." << endl ) ;

	pLScreen::set_max_row_col_size() ;

	p_poolMGR->sysput( err, "ZSCREEND", d2ds( pLScreen::maxrow, 4 ), SHARED ) ;
	p_poolMGR->sysput( err, "ZSCRMAXD", d2ds( pLScreen::maxrow, 4 ), SHARED ) ;
	p_poolMGR->sysput( err, "ZSCREENW", d2ds( pLScreen::maxcol, 4 ), SHARED ) ;
	p_poolMGR->sysput( err, "ZSCRMAXW", d2ds( pLScreen::maxcol, 4 ), SHARED ) ;

	for ( const auto lscreen : screenList )
	{
		lscreen->resize() ;
	}

	pLScreen::OIA_SWB_restart() ;

	pLScreen::OIA_setup() ;
	pLScreen::SWB_show( getSWBText() ) ;

	llog( "I", "Screen size is now "<<pLScreen::maxcol <<"x"<<pLScreen::maxrow << "." << endl ) ;
}


void loadDefaultPools()
{
	//
	// Default vars go in @DEFPROF (RO) for PROFILE and @DEFSHAR (UP) for SHARE.
	// These have the SYSTEM attibute set on the variable.
	//

	string log    ;
	string term   ;
	string datef  ;
	string zuprof ;
	string home   ;
	string shell  ;
	string logname ;

	err.clear() ;

	struct utsname buf ;

	if ( uname( &buf ) != 0 )
	{
		llog( "S", "System call uname has returned an error." << endl ) ;
		abortStartup() ;
	}

	home = getEnvironmentVariable( "HOME" ) ;
	if ( home == "" )
	{
		llog( "S", "HOME variable is required and must be set." << endl ) ;
		abortStartup() ;
	}
	zuprof = home + ZUPROF ;

	logname = getEnvironmentVariable( "LOGNAME" ) ;
	if ( logname == "" )
	{
		llog( "S", "LOGNAME variable is required and must be set." << endl ) ;
		abortStartup() ;
	}

	shell = getEnvironmentVariable( "SHELL" ) ;

	term = getEnvironmentVariable( "TERM" ) ;

	p_poolMGR->setProfilePath( err, zuprof ) ;

	p_poolMGR->sysput( err, "ZSCREEND", d2ds( pLScreen::maxrow, 4 ), SHARED ) ;
	p_poolMGR->sysput( err, "ZSCRMAXD", d2ds( pLScreen::maxrow, 4 ), SHARED ) ;
	p_poolMGR->sysput( err, "ZSCREENW", d2ds( pLScreen::maxcol, 4 ), SHARED ) ;
	p_poolMGR->sysput( err, "ZSCRMAXW", d2ds( pLScreen::maxcol, 4 ), SHARED ) ;
	p_poolMGR->sysput( err, "ZUSER", logname, SHARED ) ;
	p_poolMGR->sysput( err, "ZHOME", home, SHARED ) ;
	p_poolMGR->sysput( err, "ZSHELL", shell, SHARED ) ;
	p_poolMGR->sysput( err, "ZTERM", term, SHARED ) ;
	p_poolMGR->sysput( err, "ZSWIND", d2ds( SWIND, 2 ), SHARED ) ;

#ifdef HAS_REXX_SUPPORT
	p_poolMGR->sysput( err, "ZREXX", "YES", SHARED ) ;
#else
	p_poolMGR->sysput( err, "ZREXX", "NO", SHARED ) ;
#endif

	p_poolMGR->createProfilePool( err, "ISPS" ) ;
	if ( !err.RC0() )
	{
		llog( "S", "Loading of system profile ISPSPROF failed.  RC="<< err.getRC() << "." << endl ) ;
		llog( "S", "Aborting startup.  Check profile pool path." << endl ) ;
		listErrorBlock( err ) ;
		abortStartup() ;
	}
	llog( "I", "Loaded system profile ISPSPROF." << endl ) ;

	log = p_poolMGR->sysget( err, "ZSLOG", PROFILE ) ;

	lg->set( log ) ;
	llog( "I", "Starting logger on " << log << "." << endl ) ;

	log = p_poolMGR->sysget( err, "ZALOG", PROFILE ) ;
	lgx->set( log ) ;
	llog( "I", "Starting application logger on " << log << "." << endl ) ;

	p_poolMGR->createSharedPool() ;

	datef = getDateFormat() ;

	p_poolMGR->sysput( err, "Z", "", SHARED ) ;
	p_poolMGR->sysput( err, "ZCOLORS", d2ds( COLORS, 8 ), SHARED ) ;
	p_poolMGR->sysput( err, "ZCOLOR", ( has_colors() ) ? "YES" : "NO", SHARED ) ;
	p_poolMGR->sysput( err, "ZCHGCOLR", ( can_change_color() ) ? "YES" : "NO", SHARED ) ;
	p_poolMGR->sysput( err, "ZCLPAIRS", d2ds( COLOR_PAIRS, 8 ), SHARED ) ;
	p_poolMGR->sysput( err, "ZSCRNAM1", "OFF", SHARED ) ;
	p_poolMGR->sysput( err, "ZSYSNAME", buf.sysname, SHARED ) ;
	p_poolMGR->sysput( err, "ZNODNAME", buf.nodename, SHARED ) ;
	p_poolMGR->sysput( err, "ZOSREL", buf.release, SHARED ) ;
	p_poolMGR->sysput( err, "ZOSVER", buf.version, SHARED ) ;
	p_poolMGR->sysput( err, "ZMACHINE", buf.machine, SHARED ) ;
	p_poolMGR->sysput( err, "ZDATEF",  datef, SHARED ) ;
	p_poolMGR->sysput( err, "ZDATEFD", datef, SHARED ) ;
	p_poolMGR->sysput( err, "ZSCALE", "OFF", SHARED ) ;
	p_poolMGR->sysput( err, "ZSPLIT", "NO", SHARED ) ;
	p_poolMGR->sysput( err, "ZNULLS", "NO", SHARED ) ;
	p_poolMGR->sysput( err, "ZCMPDATE", __DATE__, SHARED ) ;
	p_poolMGR->sysput( err, "ZCMPTIME", __TIME__, SHARED ) ;
	p_poolMGR->sysput( err, "ZENVIR", "lspf " LSPF_VERSION, SHARED ) ;
	p_poolMGR->sysput( err, "ZMXTABSZ", d2ds( MXTAB_SZ, 8 ), SHARED ) ;

	p_poolMGR->setPOOLsReadOnly() ;
	gmainpgm = p_poolMGR->sysget( err, "ZMAINPGM", PROFILE ) ;

	checkSystemVariable( "ZMLIB" ) ;
	checkSystemVariable( "ZPLIB" ) ;
	checkSystemVariable( "ZTLIB" ) ;

	pLScreen::swapbar = ( p_poolMGR->sysget( err, "ZSWPBR", PROFILE ) == "Y" ) ;
}


string getEnvironmentVariable( const char* var )
{
	char* t = getenv( var ) ;

	if ( !t )
	{
		llog( "I", "Environment variable "+ string( var ) +" has not been set." << endl ) ;
		return "" ;
	}

	return t ;
}


void checkSystemVariable( const string& var )
{
	if ( p_poolMGR->sysget( err, var, PROFILE ) == "" )
	{
		llog( "S", var + " has not been set.  Aborting startup."<< endl ) ;
		abortStartup() ;
	}
}


void createSystemAllocs()
{
	createSystemAlloc( "ZMLIB" ) ;
	createSystemAlloc( "ZPLIB" ) ;
	createSystemAlloc( "ZSLIB" ) ;
	createSystemAlloc( "ZTLIB" ) ;
	createSystemAlloc( "ZTABL" ) ;
	createSystemAlloc( "ZFILE" ) ;
	createSystemAlloc( "ZMUSR" ) ;
	createSystemAlloc( "ZPUSR" ) ;
	createSystemAlloc( "ZSUSR" ) ;
	createSystemAlloc( "ZTUSR" ) ;
	createSystemAlloc( "ZTABU" ) ;
	createSystemAlloc( "ZFILU" ) ;
	createSystemAlloc( "ZLLIB" ) ;
	createSystemAlloc( "ZLDPATH" ) ;
	createSystemAlloc( "ZSYSPATH" ) ;
	createSystemAlloc( "ZUPROF" ) ;
	createSystemAlloc( "ZORXPATH" ) ;
	createSystemAlloc( "ZORXPATH", "SYSEXEC" ) ;
	createSystemAlloc( "ZFLDHIST" ) ;

	llog( "I", "System files allocated." << endl ) ;
}


void createSystemAlloc( const string& varname,
			const string& ddname )
{
	//
	// Terminate if any errors allocating the file.
	//

	string ddn ;
	string errs ;

	string paths = p_poolMGR->sysget( err, varname, PROFILE ) ;

	if ( paths != "" )
	{
		ddn = ( ddname == "" ) ? varname : ddname ;
		p_gls->alloc( "ALLOC F(" + ddn + ") PATH(" + paths + ")" , errs ) ;
		if ( errs != "" )
		{
			llog( "S", "Allocation of " + ddn + " failed." <<endl ) ;
			llog( "S", "Paths: " + paths  <<endl ) ;
			llog( "S", "Reason: " + errs  <<endl ) ;
			llog( "S", "Aborting startup" << endl ) ;
			abortStartup() ;
		}
		p_gls->set_alloc_open( ddn ) ;
		p_gls->set_alloc_nodynalloc( ddn ) ;
	}
}


void loadSystemCommandTable()
{
	//
	// Terminate if ISPCMDS not found in ZUPROF.
	//

	string zuprof ;

	err.clear() ;

	zuprof  = getenv( "HOME" ) ;
	zuprof += ZUPROF ;
	p_tableMGR->tbopen( err, "ISPCMDS", NOWRITE, zuprof, SHARE ) ;
	if ( !err.RC0() )
	{
		llog( "S", "Loading of system command table ISPCMDS failed." << endl ) ;
		llog( "S", "RC="<< err.getRC() <<"  Aborting startup." <<endl ) ;
		llog( "S", "Check path "+ zuprof << "." << endl ) ;
		listErrorBlock( err ) ;
		abortStartup() ;
	}
	llog( "I", "Loaded system command table ISPCMDS." << endl ) ;
}


string getDateFormat()
{
	string t  = DATEF ;

	const string validdf = "DD/MM/YY DD.MM.YY YY/MM/DD YY.MM.DD" ;

	if ( !findword( t, validdf ) )
	{
		llog( "S", "DATEF format incorrect.  Must be one of "<< validdf << "." << endl ) ;
		llog( "S", "Entered value is "<< t << "." << endl ) ;
		abortStartup() ;
	}
	return t ;
}


string getSWBText()
{
	uint i ;

	string txt ;

	pApplication* appl ;

	if ( !pLScreen::swapbar )
	{
		return "" ;
	}

	i = 0 ;
	for ( auto screen : screenList )
	{
		appl = screen->application_get_current() ;
		txt += ( i == priScreen ) ? "*" :
		       ( i == altScreen ) ? "-" : " " ;
		txt += left( appl->get_swb_name(), 9 ) ;
		++i ;
	}

	return txt ;
}


void loadRetrieveBuffer()
{
	//
	// Load retrieve and keep buffers.
	//

	uint rbsize ;
	uint version ;

	string ifile  ;
	string inLine ;

	if ( p_poolMGR->sysget( err, "ZSRETP", PROFILE ) == "N" ) { return ; }

	ifile  = getenv( "HOME" ) ;
	ifile += ZUPROF      ;
	ifile += "/RETPLIST" ;

	std::ifstream fin( ifile.c_str() ) ;
	if ( !fin.is_open() ) { return ; }

	rbsize = ds2d( p_poolMGR->sysget( err, "ZRBSIZE", PROFILE ) ) ;
	if ( retrieveBuffer.capacity() != rbsize )
	{
		retrieveBuffer.rset_capacity( rbsize ) ;
	}

	llog( "I", "Reloading retrieve buffer." << endl ) ;

	if ( getline( fin, inLine ) )
	{
		version = ( inLine == "*VERSION=1" ) ? 1 : 0 ;
		if ( version == 0 )
		{
			retrieveBuffer.push_back( inLine ) ;
		}
	}
	else
	{
		fin.close() ;
		return ;
	}

	while ( getline( fin, inLine ) )
	{
		if ( version == 0 && retrieveBuffer.size() < retrieveBuffer.capacity() )
		{
			retrieveBuffer.push_back( inLine ) ;
		}
		else if ( version == 1 )
		{
			if ( inLine.front() == 'N' )
			{
				if ( retrieveBuffer.size() < retrieveBuffer.capacity() )
				{
					retrieveBuffer.push_back( inLine.substr( 1 ) ) ;
				}
			}
			else if ( inLine.front() == 'K' )
			{
				keepBuffer.push_back( inLine.substr( 1 ) ) ;
			}
		}
	}
	fin.close() ;
}


void saveRetrieveBuffer()
{
	//
	// Save retrieve and keep buffers.
	//

	string ofile ;

	if ( retrieveBuffer.empty() || p_poolMGR->sysget( err, "ZSRETP", PROFILE ) == "N" ) { return ; }

	ofile  = getenv( "HOME" ) ;
	ofile += ZUPROF      ;
	ofile += "/RETPLIST" ;

	std::ofstream fout( ofile.c_str() ) ;

	if ( !fout.is_open() ) { return ; }

	llog( "I", "Saving retrieve buffer." << endl ) ;

	fout << "*VERSION=1" << endl ;
	for ( const auto& ln : retrieveBuffer )
	{
		fout << 'N' << ln << endl ;
	}

	for ( const auto& ln : keepBuffer )
	{
		fout << 'K' << ln << endl ;
	}

	fout.close() ;
}


void createpfKeyDefaults()
{
	err.clear() ;

	for ( int i = 1 ; i < 25 ; ++i )
	{
		p_poolMGR->put( err, "ZPF" + d2ds( i, 2 ), pfKeyDefaults[ i ], PROFILE ) ;
		if ( !err.RC0() )
		{
			llog( "E", "Error creating default key for task "<<err.taskid<< "." <<endl);
			listErrorBlock( err ) ;
		}
	}
}


string controlKeyAction( char c )
{
	//
	// Translate the control-key to a command (stored in ZCTRLx system profile variables).
	//

	err.clear() ;

	string s = keyname( c ) ;
	string k = "ZCTRL" ;

	k.push_back( s[ 1 ] ) ;

	return p_poolMGR->sysget( err, k, PROFILE ) ;
}


void lScreenDefaultSettings()
{
	//
	// Set the default message setting for this logical screen.
	// ZDEFM = Y show message id on messages.
	//         N don't show message id on messages.
	//

	p_poolMGR->put( err, currScrn->screenid(), "ZSHMSGID", p_poolMGR->sysget( err, "ZDEFM", PROFILE ) ) ;
}


void updateDefaultVars()
{
	err.clear() ;

	gmainpgm = p_poolMGR->sysget( err, "ZMAINPGM", PROFILE ) ;
	p_poolMGR->sysput( err, "ZSPLIT", ( pLScreen::screensTotal > 1 ) ? "YES" : "NO", SHARED ) ;

	field::field_paduchar = p_poolMGR->sysget( err, "ZPADC", PROFILE ).front() ;

	field::field_nulls    = ( p_poolMGR->sysget( err, "ZNULLS", SHARED ) == "YES" ) ;

	intens                = ( p_poolMGR->sysget( err, "ZHIGH", PROFILE ) == "Y" ) ? A_BOLD : A_NORMAL ;
	pPanel::tabpas        = ( p_poolMGR->sysget( err, "ZTABPAS", PROFILE ) == "Y" ) ;
	field::field_intens   = intens ;
	pPanel::panel_intens  = intens ;
	pLScreen::lscreen_intens = intens ;
	pdc::pdc_intens       = intens ;
	abc::abc_intens       = intens ;
	Box::box_intens       = intens ;
	text::text_intens     = intens ;
}


void createSharedPoolVars( const string& applid )
{
	err.clear() ;

	p_poolMGR->put( err, "ZSCREEN", string( 1, zscreen[ priScreen ] ), SHARED, SYSTEM ) ;
	p_poolMGR->put( err, "ZSCRNUM", d2ds( currScrn->screenid() ), SHARED, SYSTEM ) ;
	p_poolMGR->put( err, "ZAPPLID", applid, SHARED, SYSTEM ) ;
	p_poolMGR->put( err, "ZPFKEY", "PF00", SHARED, SYSTEM ) ;
}


void updateReflist()
{
	//
	// Check if .NRET is ON and has a valid field name.  If so, add file to the reflist using
	// application ZRFLPGM, parmameters PLR plus the field entry value.  Run task in the background.
	//
	// Don't update REFLIST if the application has done a CONTROL REFLIST NOUPDATE.
	// or ISPS PROFILE variable ZRFURL is not set to YES.
	//

	string fname = currAppl->get_nretfield() ;

	selobj selct ;

	err.clear() ;

	if ( fname == "" ||
	     p_poolMGR->get( err, currScrn->screenid(), "ZREFUPDT" ) != "Y" ||
	     p_poolMGR->sysget( err, "ZRFURL", PROFILE ) != "YES" )
	{
		return ;
	}

	if ( currAppl->currPanel->field_exists( fname ) )
	{
		selct.clear() ;
		selct.pgm     = p_poolMGR->sysget( err, "ZRFLPGM", PROFILE ) ;
		selct.parm    = "PLR " + currAppl->currPanel->field_getrawvalue( fname ) ;
		selct.newappl = ""    ;
		selct.newpool = false ;
		selct.passlib = false ;
		selct.backgrd = true  ;
		selct.sync    = false ;
		startApplicationBack( selct, currAppl, false ) ;
	}
	else
	{
		llog( "E", "Invalid field "+ fname +" in .NRET panel statement." << endl ) ;
		issueMessage( "PSYS011Z" ) ;
	}
}


void updateHistory()
{
	//
	// Check if any panel fields have history specified. If so, add field/value to the history table for the panel.
	//
	// Don't update field history if the application has done a CONTROL HISTORY NOUPDATE.
	// or ISPS PROFILE variable ZFHURL is not set to Y.
	//

	int i ;
	int j ;

	string fname ;
	string value ;

	string fnames = currAppl->get_history_fields() ;

	selobj selct ;

	err.clear() ;

	if ( fnames == "" || p_poolMGR->get( err, currScrn->screenid(), "ZFHUPDT" ) != "Y" ||
			     p_poolMGR->sysget( err, "ZFHURL", PROFILE ) != "Y" )
	{
		return ;
	}

	for ( i = 1, j = words( fnames ) ; i <= j ; ++i )
	{
		fname = word( fnames, i ) ;
		value = currAppl->currPanel->field_getvalue_caps( fname ) ;
		if ( value != "" )
		{
			selct.clear() ;
			selct.pgm     = p_poolMGR->sysget( err, "ZFHSTPGM", PROFILE ) ;
			selct.parm    = "PLU " + currAppl->get_panelid() + " " + fname + " " + value ;
			selct.newappl = ""    ;
			selct.newpool = false ;
			selct.passlib = false ;
			selct.backgrd = true  ;
			selct.sync    = false ;
			selct.nollog  = true  ;
			startApplicationBack( selct, currAppl, false ) ;
		}
	}
}


void lineOutput()
{
	//
	// Write line output to the display.  Split line if longer than screen width.
	//

	size_t i ;

	string t ;

	if ( !lwin )
	{
		createLinePanel() ;
		beep() ;
	}
	else
	{
		top_panel( lpanel ) ;
	}

	pLScreen::clear_busy() ;
	pLScreen::show_busy() ;
	wattrset( lwin, RED | A_BOLD ) ;
	t = currAppl->outBuffer ;

	convNonDisplayChars( t ) ;

	do
	{
		if ( linePosn == int( pLScreen::maxrow-1 ) )
		{
			lineOutput_end() ;
			if ( currAppl->abnormalEndForced )
			{
				break ;
			}
		}
		if ( t.size() > pLScreen::maxcol )
		{
			i = t.find_last_of( ' ', pLScreen::maxcol ) ;
			i = ( i == string::npos ) ? pLScreen::maxcol : i + 1 ;
			mvwaddstr( lwin, linePosn++, 0, t.substr( 0, i ).c_str() ) ;
			t.erase( 0, i ) ;
		}
		else
		{
			mvwaddstr( lwin, linePosn++, 0, t.c_str() ) ;
			wnoutrefresh( lwin ) ;
			t = "" ;
		}
	} while ( t.size() > 0 ) ;

	pLScreen::OIA_endTime() ;
	currScrn->set_cursor( linePosn, 3 ) ;
	currScrn->OIA_update( priScreen, altScreen ) ;

	wnoutrefresh( lwin ) ;
	wnoutrefresh( OIA ) ;
	top_panel( lpanel ) ;
	wmove( lwin, linePosn, 0 ) ;
	update_panels() ;
	doupdate() ;
}


void lineOutput_end()
{
	uint row = 0 ;
	uint col = 0 ;

	set<uint>rows ;

	wattrset( lwin, RED | A_BOLD ) ;
	mvwaddstr( lwin, linePosn, 0, "***" ) ;

	pLScreen::OIA_endTime() ;
	currScrn->OIA_update( priScreen, altScreen ) ;

	pLScreen::show_enter() ;

	currScrn->set_cursor( linePosn, 3 ) ;
	top_panel( lpanel ) ;

	updateScreenText( rows, row, col ) ;
	wattrset( lwin, RED | A_BOLD ) ;

	linePosn = 0 ;
	werase( lwin ) ;

	if ( currAppl->abnormalEndForced )
	{
		programTerminated() ;
		return ;
	}

	pLScreen::clear_status() ;
	pLScreen::OIA_startTime() ;

	if ( currAppl->notify_pending() )
	{
		while ( currAppl->notify() )
		{
			lineOutput() ;
		}
		lineOutput_end() ;
	}
}


void displayNotifies()
{
	bool fscreen = !lwin ;

	if ( currAppl->notify_pending() )
	{
		while ( currAppl->notify() )
		{
			lineOutput() ;
		}
		if ( fscreen )
		{
			deleteLinePanel() ;
			pApplication::lineInOutDone = false ;
			currScrn->set_cursor( currAppl ) ;
		}
	}
}


void lineInput()
{
	uint row = linePosn ;
	uint col = 0 ;

	set<uint>rows ;

	if ( !lwin )
	{
		createLinePanel() ;
		row = 0 ;
		col = 0 ;
	}
	else
	{
		top_panel( lpanel ) ;
		if ( linePosn == int( pLScreen::maxrow-1 ) )
		{
			lineOutput_end() ;
			row = 0 ;
			col = 0 ;
		}
	}
	++linePosn ;

	currScrn->set_cursor( row, col ) ;
	pLScreen::clear_status() ;

	updateScreenText( rows, row, col ) ;

	if ( currAppl->abnormalEndForced )
	{
		programTerminated() ;
		return ;
	}

	currAppl->inBuffer = "" ;
	for ( auto row : rows )
	{
		currAppl->inBuffer += getScreenText( row ) ;
	}
}


void updateScreenText( set<uint>& rows,
		       uint row,
		       uint col )
{
	//
	// Update the screen text in raw mode.
	//

	int c ;

	bool actionKey = false ;
	bool Insert    = false ;
	bool esc       = false ;

	wattrset( lwin, GREEN | A_BOLD ) ;

	while ( !actionKey )
	{
		currScrn->get_cursor( row, col ) ;
		currScrn->OIA_update( priScreen, altScreen ) ;
		wnoutrefresh( OIA ) ;
		wmove( lwin, row, col ) ;
		update_panels() ;
		doupdate() ;
		c = getch() ;
		if ( c == 13 ) { c = KEY_ENTER ; }
		if ( c < 256 && isprint( c ) )
		{
			rows.insert( row ) ;
			Insert ? winsch( lwin, c ) : waddch( lwin, c ) ;
			wnoutrefresh( lwin ) ;
			currScrn->cursor_right() ;
			continue ;
		}
		switch( c )
		{
			case KEY_SLEFT:
				currScrn->cursor_left_cond() ;
			case KEY_LEFT:
				currScrn->cursor_left() ;
				break ;

			case KEY_SRIGHT:
				currScrn->cursor_right_cond() ;
			case KEY_RIGHT:
				currScrn->cursor_right() ;
				break ;

			case KEY_SR:
				currScrn->cursor_up_cond() ;
			case KEY_UP:
				currScrn->cursor_up() ;
				break ;

			case KEY_SF:
				currScrn->cursor_down_cond() ;
			case KEY_DOWN:
				currScrn->cursor_down() ;
				break ;

			case KEY_IC:
				Insert = !Insert ;
				currScrn->set_Insert( Insert ) ;
				break ;

			case KEY_DC:
				mvwdelch( lwin, row, col ) ;
				rows.insert( row ) ;
				break ;

			case KEY_END:
				col = min( ( pLScreen::maxcol - 1 ), getTextLength( row ) + 1 ) ;
				currScrn->set_cursor( row, col ) ;
				break ;

			case 127:
			case KEY_BACKSPACE:
				if ( col > 0 )
				{
					mvwdelch( lwin, row, --col ) ;
					currScrn->set_cursor( row, col ) ;
					rows.insert( row ) ;
				}
				break ;

			case KEY_HOME:
				currScrn->set_cursor( 0, 0 ) ;
				break ;

			case CTRL( 'i' ):
				currScrn->cursor_down() ;
				currScrn->set_col( 0 ) ;
				break ;

			case CTRL( '[' ):
				if ( esc )
				{
					currAppl->abnormalEndForced = true ;
					actionKey = true ;
				}
				else
				{
					esc = true ;
					pLScreen::show_esc() ;
				}
				break ;

			default:
				actionKey = isActionKey( c ) ;
				break ;
		}
	}

	currScrn->set_Insert( false ) ;
}


string getScreenText( uint row )
{
	//
	// Retrieve a row of text on the screen excluding null characters.
	// Nulls are determined by the fact they have no associated attributes.
	//

	chtype inc1 ;
	chtype inc2 ;

	string t ;
	t.reserve( pLScreen::maxcol ) ;

	for ( uint j = 0 ; j < pLScreen::maxcol ; ++j )
	{
		inc1 = mvwinch( lwin, row, j ) ;
		inc2 = inc1 & A_CHARTEXT ;
		if ( inc1 != inc2 )
		{
			t.push_back( char( inc2 ) ) ;
		}
	}

	return t ;
}


uint getTextLength( uint row )
{
	//
	// Retrieve the row text length.
	//

	int l = pLScreen::maxcol - 1 ;

	for ( ; l >= 0 ; --l )
	{
		if ( char( mvwinch( lwin, row, l ) & A_CHARTEXT ) != ' ' ) { break ; }
	}

	return l ;
}


void convNonDisplayChars( string& s )
{
	//
	// Replace all non-printable characters in string 's' with ':'.
	//

	replace_if( s.begin(), s.end(), []( char c )
					{
						return ( !isprint( c ) ) ;
					}, ':' ) ;
}


void programTerminated()
{
	wattrset( lwin, RED | A_BOLD ) ;
	mvwaddstr( lwin, linePosn++, 0, "Program terminated..." ) ;
	pLScreen::clear_status() ;
	pLScreen::OIA_startTime() ;
}


string listLogicalScreens()
{
	//
	// Mainline lspf cannot create application panels but let's make this as similar as possible.
	//

	int o ;
	int c ;

	bool found ;

	size_t maxl = 41 ;

	uint i ;
	uint j ;
	uint m ;
	string ln  ;
	string t   ;
	string act ;
	string w2  ;

	err.clear() ;

	WINDOW* swwin   ;
	PANEL*  swpanel ;

	pApplication* appl ;

	currScrn->set_appl_cursor( currAppl ) ;

	vector<pLScreen*>::iterator its ;
	vector<string>::iterator it ;
	vector<string> lslist ;

	for ( its = screenList.begin() ; its != screenList.end() ; ++its )
	{
		appl = (*its)->application_get_current() ;
		t    = appl->get_current_panelDesc() ;
		maxl = max( maxl, min( t.size(), size_t( pLScreen::maxcol-40 ) ) ) ;
	}

	m = 0 ;
	for ( i = 0, its = screenList.begin() ; its != screenList.end() ; ++its, ++i )
	{
		appl = (*its)->application_get_current() ;
		ln   = d2ds( (*its)->get_screenNum() ) ;
		if      ( i == priScreen ) { ln += "*" ; m = i ; }
		else if ( i == altScreen ) { ln += "-"         ; }
		else                       { ln += " "         ; }
		t = appl->get_current_panelDesc() ;
		if ( t.size() > maxl )
		{
			t.replace( 20, t.size()-maxl+3, "..." ) ;
		}
		ln = left( ln, 4 ) +
		     left( appl->get_current_screenName(), 10 ) +
		     left( appl->get_appname(), 13 ) +
		     left( appl->get_applid(), 8 ) +
		     left( t, maxl ) ;
		lslist.push_back( ln ) ;
	}

	swwin   = newwin( screenList.size() + 6, maxl+39, 1, 1 ) ;
	swpanel = new_panel( swwin )  ;
	wattrset( swwin, cuaAttr[ AWF ] | intens ) ;
	box( swwin, 0, 0 ) ;
	mvwaddstr( swwin, 0, 34, " Task List " ) ;

	wattrset( swwin, cuaAttr[ PT ] | intens ) ;
	mvwaddstr( swwin, 1, 28, "Active Logical Sessions" ) ;

	wattrset( swwin, cuaAttr[ PIN ] | intens ) ;
	mvwaddstr( swwin, 3, 2, "ID  Name      Application  Applid  Panel Title/Description" ) ;

	pLScreen::show_wait() ;

	o = m ;
	curs_set( 0 ) ;

	while ( true )
	{
		for ( i = 0, it = lslist.begin() ; it != lslist.end() ; ++it, ++i )
		{
			wattrset( swwin, cuaAttr[ ( i == m ) ? PT : VOI ] | intens ) ;
			mvwaddstr( swwin, i+4, 2, it->c_str() ) ;
		}
		update_panels() ;
		doupdate()  ;
		c = getch() ;
		if ( c >= CTRL( 'a' ) && c <= CTRL( 'z' ) )
		{
			act = controlKeyAction( c ) ;
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
			( m == 0 ) ? m = lslist.size() - 1 : --m ;
		}
		else if ( c == KEY_DOWN || c == CTRL( 'i' ) )
		{
			( m == lslist.size() - 1 ) ? m = 0 : ++m ;
		}
		else if ( ( c >= KEY_F( 1 ) && c <= KEY_F( 24 ) ) || ( c > 48 && c < 58 ) )
		{
			i     = ( c > 48 && c < 58 ) ? c - 48 : c - KEY_F( 0 ) ;
			found = false ;
			for ( j = 0, it = lslist.begin() ; it != lslist.end() ; ++it, ++j )
			{
				if ( int( i ) == ds2d( it->substr( 0, 1 ) ) )
				{
					m     = j ;
					found = true ;
					break ;
				}
			}
			if ( found ) { break ; }
		}
		else if ( isActionKey( c ) )
		{
			m = o ;
			break ;
		}
	}

	del_panel( swpanel ) ;
	delwin( swwin ) ;

	curs_set( 1 ) ;
	pLScreen::OIA_startTime() ;

	return d2ds( m + 1 ) ;
}


int listInterruptOptions()
{
	//
	// Mainline lspf cannot create application panels but let's make this as similar as possible.
	//

	int c ;

	uint i ;
	uint m ;

	string appl ;

	err.clear() ;

	WINDOW* swwin   ;
	PANEL*  swpanel ;

	vector<string>::iterator it ;
	vector<string> lslist ;

	appl = "Application: " + currAppl->get_appname() ;

	lslist.push_back( "1. Continue running application (press ESC again to interrupt)" ) ;
	lslist.push_back( "2. Send SIGTERM to the application" ) ;
	lslist.push_back( "3. Send SIGUSR1 to the application" ) ;
	lslist.push_back( "4. Force abend application" ) ;
	lslist.push_back( "5. Sent notify to application" ) ;
	lslist.push_back( "6. Show application status (lspf log)" ) ;

	swwin   = newwin( lslist.size() + 8, 66, 1, 1 ) ;
	swpanel = new_panel( swwin )  ;

	wattrset( swwin, cuaAttr[ AWF ] | intens ) ;
	box( swwin, 0, 0 ) ;

	wattrset( swwin, cuaAttr[ PT ] | intens ) ;
	mvwaddstr( swwin, 1, 21, "Program Interrupt Options" ) ;

	wattrset( swwin, cuaAttr[ PIN ] | intens ) ;
	mvwaddstr( swwin, 3, 2, appl.c_str() ) ;

	pLScreen::show_wait() ;

	m = 0 ;
	curs_set( 0 ) ;
	while ( currAppl->busyAppl )
	{
		for ( i = 0, it = lslist.begin() ; it != lslist.end() ; ++it, ++i )
		{
			wattrset( swwin, cuaAttr[ ( i == m ) ? PT : VOI ] | intens ) ;
			mvwaddstr( swwin, i+5, 2, it->c_str() ) ;
		}
		update_panels() ;
		doupdate()  ;
		c = getch() ;
		if ( c == KEY_ENTER || c == 13 ) { break ; }
		if ( c == KEY_UP )
		{
			m == 0 ? m = lslist.size() - 1 : --m ;
		}
		else if ( c == KEY_DOWN || c == CTRL( 'i' ) )
		{
			m == lslist.size()-1 ? m = 0 : ++m ;
		}
		else if ( isActionKey( c ) )
		{
			m = 0 ;
			break ;
		}
	}

	if ( !currAppl->busyAppl ) { m = 0 ; }

	del_panel( swpanel ) ;
	delwin( swwin ) ;
	curs_set( 1 ) ;

	update_panels() ;
	doupdate() ;

	pLScreen::OIA_startTime() ;

	return m ;
}


void listRetrieveBuffer( char bufferType,
			 string filter )
{
	//
	// Show the retrieve/keep stack.
	//
	// Mainline lspf cannot create application panels but let's make this as similar as possible.
	//

	int c ;

	uint i ;
	uint m ;

	int row ;
	int col ;

	uint s = 0 ;

	int maxy ;
	int maxx ;

	bool button1 ;
	bool button2 ;
	bool button3 ;
	bool button4 ;
	bool button5 ;

	bool more ;

	string pfcmd ;
	string filter_str ;

	const char* si1 = "           " ;
	const char* si2 = "More:     +" ;
	const char* si3 = "More:   -  " ;
	const char* si4 = "More:   - +" ;

	const char* text1 = "Invoke entry with function key" ;
	const char* space = "                              " ;

	WINDOW* rbwin   = nullptr ;
	PANEL*  rbpanel = nullptr ;

	vector<string> tBuffer ;

	MEVENT event ;

	vector<pair<string,uint>> lslist ;
	vector<pair<string,uint>>::iterator it ;

	err.clear() ;

	if ( filter.size() > 50 ) { filter.erase( 50 ) ; }

	listRetrieveBuffer_load_tBuffer( tBuffer, bufferType, filter ) ;
	listRetrieveBuffer_build( tBuffer, lslist, bufferType, s, rbwin, rbpanel, more ) ;

	curs_set( 0 ) ;
	m = 0 ;
	while ( true )
	{
		for ( i = 0, it = lslist.begin() ; it != lslist.end() ; ++it, ++i )
		{
			wattrset( rbwin, cuaAttr[ ( i == m ) ? PT : VOI ] | intens ) ;
			mvwaddstr( rbwin, i+3, 3, it->first.c_str() ) ;
		}
		getmaxyx( rbwin, maxy, maxx ) ;
		if ( filter != "" )
		{
			filter_str = "[" + filter + "]" ;
			wattrset( rbwin, cuaAttr[ PT ] | intens ) ;
			mvwaddstr( rbwin, maxy-2, 3, filter_str.c_str() ) ;
		}

		wattrset( rbwin, cuaAttr[ SI ] | intens ) ;
		mvwaddstr( rbwin, 2, ( maxx - 13 ), ( s == 0 && !more ) ? si1 :
						    ( s  > 0 && !more ) ? si3 :
						    ( s  > 0 &&  more ) ? si4 : si2 ) ;
		update_panels() ;
		doupdate()  ;
		c = getch() ;
		if ( c == KEY_MOUSE && getmouse( &event ) == OK )
		{
			button1 = ( event.bstate & BUTTON1_CLICKED || event.bstate & BUTTON1_DOUBLE_CLICKED || event.bstate & BUTTON1_PRESSED ) ;
			button2 = ( event.bstate & BUTTON2_CLICKED || event.bstate & BUTTON2_DOUBLE_CLICKED || event.bstate & BUTTON2_PRESSED ) ;
			button3 = ( event.bstate & BUTTON3_CLICKED || event.bstate & BUTTON3_DOUBLE_CLICKED || event.bstate & BUTTON3_PRESSED ) ;
			button4 = ( event.bstate & BUTTON4_PRESSED ) ;
			button5 = ( event.bstate & BUTTON5_PRESSED ) ;
			if ( wenclose( rbwin, event.y, event.x ) )
			{
				if ( button1 || button2 )
				{
					getbegyx( rbwin, row, col ) ;
					if ( event.y >= row + 3 )
					{
						m = event.y - row - 3 ;
						if ( m < lslist.size() )
						{
							c = ( button1 ) ? KEY_ENTER : KEY_SDC ;
						}
					}
				}
				else if ( button3 )
				{
					currAppl->currPanel->cursor_to_cmdfield() ;
					break ;
				}
				else if ( button4 )
				{
					c = KEY_UP ;
				}
				else if ( button5 )
				{
					c = KEY_DOWN ;
				}
				else
				{
					continue ;
				}
			}
			else if ( button1 || button3 )
			{
				currAppl->currPanel->cursor_to_cmdfield() ;
				break ;
			}
			else
			{
				continue ;
			}
		}
		if ( c == KEY_ENTER || c == 13 || c == CTRL( 'e' ) )
		{
			if ( lslist.size() > 0 )
			{
				it = lslist.begin() + m ;
				const string& t = tBuffer[ it->second ] ;
				currAppl->currPanel->cmd_setvalue( t ) ;
				currAppl->currPanel->cursor_to_cmdfield( t.size() + 1 ) ;
				if ( c == CTRL( 'e' ) ) { ungetch( KEY_ENTER ) ; }
				commandStack = "" ;
			}
			break ;
		}
		else if ( c == KEY_UP || c == KEY_BTAB )
		{
			if ( m == 0 )
			{
				if ( s > 0 )
				{
					--s ;
					listRetrieveBuffer_build( tBuffer, lslist, bufferType, s, rbwin, rbpanel, more ) ;
				}
			}
			else
			{
				--m ;
			}
		}
		else if ( c == KEY_DOWN || c == CTRL( 'i' ) )
		{
			if ( m == lslist.size() - 1 )
			{
				if ( s < ( tBuffer.size() - 1 ) && tBuffer.size() > ( lslist.size() + s ) )
				{
					listRetrieveBuffer_build( tBuffer, lslist, bufferType, ++s, rbwin, rbpanel, more ) ;
				}
			}
			else
			{
				++m ;
			}
		}
		else if ( c == KEY_PPAGE )
		{
			m = 0 ;
			s = ( s < uint( maxy - 5 ) ) ? 0 : s - maxy + 5 ;
			listRetrieveBuffer_build( tBuffer, lslist, bufferType, s, rbwin, rbpanel, more ) ;
		}
		else if ( c == KEY_NPAGE )
		{
			m = 0 ;
			s += maxy - 5 ;
			if ( tBuffer.size() < ( s + maxy - 5 ) )
			{
				s = ( tBuffer.size() > size_t( maxy - 5 ) ) ? tBuffer.size() - size_t( maxy - 5 ) : 0 ;
			}
			listRetrieveBuffer_build( tBuffer, lslist, bufferType, s, rbwin, rbpanel, more ) ;
		}
		else if ( c == KEY_HOME || c == KEY_SEND )
		{
			m = 0 ;
			s = 0 ;
			listRetrieveBuffer_build( tBuffer, lslist, bufferType, s, rbwin, rbpanel, more ) ;
		}
		else if ( c == KEY_SHOME || c == KEY_END )
		{
			if ( tBuffer.size() > size_t( maxy - 5 ) )
			{
				m = maxy - 6 ;
				s = tBuffer.size() - ( maxy - 5 ) ;
			}
			else
			{
				m = tBuffer.size() - 1 ;
				s = 0 ;
			}
			listRetrieveBuffer_build( tBuffer, lslist, bufferType, s, rbwin, rbpanel, more ) ;
		}
		else if ( c == KEY_SDC )
		{
			if ( lslist.size() > 0 )
			{
				it = lslist.begin() + m ;
				if ( bufferType == 'N' )
				{
					auto itt = find( retrieveBuffer.begin(), retrieveBuffer.end(), tBuffer[ it->second ] ) ;
					if ( itt != retrieveBuffer.end() )
					{
						retrieveBuffer.erase( itt ) ;
						if ( retrieveBuffer.empty() )
						{
							issueMessage( "PSYS012B" ) ;
							break ;
						}
					}
				}
				else if ( bufferType == 'K' )
				{
					auto itt = find( keepBuffer.begin(), keepBuffer.end(), tBuffer[ it->second ] ) ;
					if ( itt != keepBuffer.end() )
					{
						keepBuffer.erase( itt ) ;
						if ( keepBuffer.empty() )
						{
							issueMessage( "PSYS012B" ) ;
							break ;
						}
					}
				}
				if ( m > 0 && m == lslist.size() - 1 )
				{
					--m ;
				}
				listRetrieveBuffer_load_tBuffer( tBuffer, bufferType, filter ) ;
				listRetrieveBuffer_build( tBuffer, lslist, bufferType, s, rbwin, rbpanel, more ) ;
			}
		}
		else if ( char( c ) == '\\' )
		{
			wattrset( rbwin, cuaAttr[ PT ] | intens ) ;
			mvwaddstr( rbwin, maxy-2, 3, text1 ) ;
			update_panels() ;
			doupdate()  ;
			c = getch() ;
			if ( c >= KEY_F( 1 ) && c <= KEY_F( 24 ) )
			{
				i = c - KEY_F( 0 ) - 1 ;
				if ( i < lslist.size() )
				{
					m = i ;
					ungetch( CTRL( 'e' ) ) ;
				}
			}
			else if ( char( c ) == '\\' )
			{
				if ( filter.size() < 50 )
				{
					m = 0 ;
					s = 0 ;
					filter.push_back( '\\' ) ;
					listRetrieveBuffer_load_tBuffer( tBuffer, bufferType, filter ) ;
					listRetrieveBuffer_build( tBuffer, lslist, bufferType, s, rbwin, rbpanel, more ) ;
				}
			}
			else
			{
				ungetch( c ) ;
			}
			mvwaddstr( rbwin, maxy-2, 3, space ) ;
		}
		else if ( c < 256 && isprint( c ) )
		{
			if ( filter.size() < 50 )
			{
				m = 0 ;
				s = 0 ;
				filter.push_back( toupper( c ) ) ;
				listRetrieveBuffer_load_tBuffer( tBuffer, bufferType, filter ) ;
				listRetrieveBuffer_build( tBuffer, lslist, bufferType, s, rbwin, rbpanel, more ) ;
			}
		}
		else if ( c == KEY_BACKSPACE || c == 127 || c == KEY_DC )
		{
			if ( filter != "" )
			{
				m = 0 ;
				s = 0 ;
				if ( c == KEY_DC )
				{
					filter = "" ;
				}
				else
				{
					filter.pop_back() ;
				}
				listRetrieveBuffer_load_tBuffer( tBuffer, bufferType, filter ) ;
				listRetrieveBuffer_build( tBuffer, lslist, bufferType, s, rbwin, rbpanel, more ) ;
			}
		}
		else if ( c >= KEY_F( 1 ) && c <= KEY_F( 24 ) )
		{
			pfcmd = currAppl->currPanel->get_pfkey( err, c ) ;
			if ( findword( upper( word( pfcmd, 1 ) ), "SPLIT SWAP HELP" ) )
			{
				ungetch( c ) ;
			}
			currAppl->currPanel->cursor_to_cmdfield() ;
			break ;
		}
		else if ( c == CTRL( 'k' ) )
		{
			if ( lslist.size() > 0 )
			{
				it = lslist.begin() + m ;
				const string& t = tBuffer[ it->second ] ;
				auto itt = find( keepBuffer.begin(), keepBuffer.end(), t ) ;
				if ( itt == keepBuffer.end() )
				{
					keepBuffer.push_back( t ) ;
				}
			}
		}
		else if ( c == KEY_SR )
		{
			if ( m > 0 )
			{
				if ( bufferType == 'N' )
				{
					auto it1 = find( retrieveBuffer.begin(), retrieveBuffer.end(), tBuffer[ s + m ] ) ;
					auto it2 = find( retrieveBuffer.begin(), retrieveBuffer.end(), tBuffer[ s + m - 1 ] ) ;
					swap( *it1, *it2 ) ;
				}
				else
				{
					auto it1 = find( keepBuffer.begin(), keepBuffer.end(), tBuffer[ s + m ] ) ;
					auto it2 = find( keepBuffer.begin(), keepBuffer.end(), tBuffer[ s + m - 1 ] ) ;
					swap( *it1, *it2 ) ;
				}
				listRetrieveBuffer_load_tBuffer( tBuffer, bufferType, filter ) ;
				listRetrieveBuffer_build( tBuffer, lslist, bufferType, s, rbwin, rbpanel, more ) ;
				--m ;
			}
		}
		else if ( c == KEY_SF )
		{
			if ( m < lslist.size() - 1 )
			{
				if ( bufferType == 'N' )
				{
					auto it1 = find( retrieveBuffer.begin(), retrieveBuffer.end(), tBuffer[ s + m + 1 ] ) ;
					auto it2 = find( retrieveBuffer.begin(), retrieveBuffer.end(), tBuffer[ s + m ] ) ;
					swap( *it1, *it2 ) ;
				}
				else
				{
					auto it1 = find( keepBuffer.begin(), keepBuffer.end(), tBuffer[ s + m + 1 ] ) ;
					auto it2 = find( keepBuffer.begin(), keepBuffer.end(), tBuffer[ s + m ] ) ;
					swap( *it1, *it2 ) ;
				}
				listRetrieveBuffer_load_tBuffer( tBuffer, bufferType, filter ) ;
				listRetrieveBuffer_build( tBuffer, lslist, bufferType, s, rbwin, rbpanel, more ) ;
				++m ;
			}
		}
		else if ( c == CTRL( 's' ) )
		{
			m = 0 ;
			s = 0 ;
			bufferType = ( bufferType == 'N' ) ? 'K' : 'N' ;
			listRetrieveBuffer_load_tBuffer( tBuffer, bufferType, filter ) ;
			listRetrieveBuffer_build( tBuffer, lslist, bufferType, s, rbwin, rbpanel, more ) ;
		}
		else if ( isActionKey( c ) )
		{
			currAppl->currPanel->cursor_to_cmdfield() ;
			break ;
		}
	}

	del_panel( rbpanel ) ;
	delwin( rbwin ) ;
	curs_set( 1 ) ;

	currScrn->set_cursor( currAppl ) ;
	pLScreen::OIA_startTime() ;
}


void listRetrieveBuffer_load_tBuffer( vector<string>& tBuffer,
				      char bufferType,
				      const string& filter )
{
	//
	// Load the temporary retrieve buffer.
	//

	size_t i ;

	tBuffer.clear() ;
	if ( bufferType == 'N' )
	{
		for ( i = 0 ; i < retrieveBuffer.size() ; ++i )
		{
			const string& t = retrieveBuffer[ i ] ;
			if ( filter == "" || upper( t ).find( filter ) != string::npos )
			{
				tBuffer.push_back( t ) ;
			}
		}
	}
	else
	{
		for ( i = 0 ; i < keepBuffer.size() ; ++i )
		{
			const string& t = keepBuffer[ i ] ;
			if ( filter == "" || upper( t ).find( filter ) != string::npos )
			{
				tBuffer.push_back( t ) ;
			}
		}
	}
}


void listRetrieveBuffer_build( vector<string>& tBuffer,
			       vector<pair<string,uint>>& lslist,
			       char bufferType,
			       uint& s,
			       WINDOW*& rbwin,
			       PANEL*& rbpanel,
			       bool& more )
{
	//
	// Build the retrieve list and create the window/panel.
	//

	uint i ;
	uint j ;
	uint mx ;

	size_t len  = 52 ;
	size_t maxl = pLScreen::maxcol - 10 ;

	string t ;
	string ln ;

	lslist.clear() ;

	mx = ( bufferType == 'N' ) ? ( retrieveBuffer.size() > pLScreen::maxrow-6 ) ? pLScreen::maxrow-6 : tBuffer.size()
				   : ( keepBuffer.size() > pLScreen::maxrow-6 ) ? pLScreen::maxrow-6 : tBuffer.size() ;

	for ( i = s, j = 0 ; i < tBuffer.size() && j < mx ; ++i, ++j )
	{
		t   = tBuffer[ i ] ;
		len = max( len, min( t.size(), maxl ) ) ;
		if ( t.size() > maxl )
		{
			t.replace( (maxl/2-2), t.size()-(maxl-3), "..." ) ;
		}
		ln = left( d2ds( i + 1 ) + ".", 4 ) + t ;
		lslist.push_back( pair<string, uint>( ln, i ) ) ;
	}

	more = ( i < tBuffer.size() ) ;

	if ( rbpanel )
	{
		del_panel( rbpanel ) ;
		delwin( rbwin ) ;
	}

	rbwin   = newwin( mx+5, len+8, 1, 1 ) ;
	rbpanel = new_panel( rbwin )  ;
	wattrset( rbwin, cuaAttr[ AWF ] | intens ) ;
	box( rbwin, 0, 0 ) ;
	mvwaddstr( rbwin, 0, (len/2 - 1), " Retrieve " ) ;

	wattrset( rbwin, cuaAttr[ PT ] | intens ) ;
	mvwaddstr( rbwin, 1, (len/2 - 10), ( bufferType == 'N' ) ? "lspf Command Retrieve Panel" : "lspf Keep Command Retrieve Panel" ) ;

	pLScreen::show_wait() ;
}


void listApplicationStatus()
{
	llog( "I", "*******************************************************************" << endl ) ;
	llog( "I", "Application "+ currAppl->get_appname() +" ID: "<< currAppl->taskid() << endl ) ;
	llog( "I", "Status. . . . . : " << currAppl->get_status() << endl ) ;
	llog( "I", "Progarm status. : " << ( ( currAppl->busyAppl ) ? "Busy" : "Waiting" ) << endl ) ;
	llog( "I", "*******************************************************************" << endl ) ;
}


void listBackTasks()
{
	//
	// List background tasks and tasks that have been moved to the timeout queue.
	//

	llog( "I", ".TASKS" <<endl ) ;
	llog( "-", "****************************************************" <<endl ) ;
	llog( "-", "Listing background tasks:" << endl ) ;
	llog( "-", "         Number of tasks. . . . "<< pApplicationBackground.size()<< endl ) ;
	llog( "-", " "<< endl ) ;

	mtx.lock() ;
	listTaskVector( pApplicationBackground ) ;
	mtx.unlock() ;

	llog( "-", " "<< endl ) ;
	llog( "-", "Listing timed-out tasks:" << endl ) ;
	llog( "-", "         Number of tasks. . . . "<< pApplicationTimeout.size()<< endl ) ;
	llog( "-", " "<< endl ) ;

	listTaskVector( pApplicationTimeout ) ;

	llog( "-", "****************************************************" <<endl ) ;
}


void listTaskVector( vector<pApplication*>& p )
{
	if ( p.size() == 0 ) { return ; }

	llog( "-", "Program     Id       Status               Parent ID    Parameters" << endl) ;
	llog( "-", "--------    -----    -----------------    ---------    ----------" << endl) ;

	for ( const auto app : p )
	{
		llog( "-", "" << setw(12) << std::left << app->get_appname() <<
				 setw(9)  << std::left << d2ds( app->taskid(), 5 ) <<
				 setw(21) << std::left << app->get_status() <<
				 setw(13) << std::left <<
				 ( ( app->uAppl ) ? d2ds( app->uAppl->taskid(), 5 ) : "-" ) <<
				 app->get_zparm() <<endl ) ;
	}
}


void autoUpdate()
{
	//
	// Resume application every 1s and wait.
	// Check every 50ms to see if ESC(27) has been pressed - read all characters from the buffer.
	//

	int c ;

	bool end_auto = false ;

	pLScreen::show_auto() ;
	curs_set( 0 ) ;

	while ( !end_auto )
	{
		ResumeApplicationAndWait() ;
		decolouriseScreen() ;
		pLScreen::OIA_endTime() ;
		currScrn->OIA_update( priScreen, altScreen ) ;
		pLScreen::SWB_show( getSWBText() ) ;
		wnoutrefresh( stdscr ) ;
		wnoutrefresh( OIA ) ;
		update_panels() ;
		doupdate() ;
		nodelay( stdscr, true ) ;
		pLScreen::OIA_startTime() ;
		for ( int i = 0 ; i < 20 && !end_auto ; ++i )
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
	pLScreen::clear_status() ;
	nodelay( stdscr, false ) ;

	currScrn->set_cursor( currAppl ) ;
}


void screenRefresh()
{
	//
	// Refresh the entire screen in case of corruption.
	// (user request or CONTROL DISPLAY REFRESH).
	//

	currScrn->refresh_panel_stack() ;
	pLScreen::OIA_refresh() ;
	redrawwin( stdscr ) ;
}


void lspfCallbackHandler( lspfCommand& lc )
{
	//
	// Issue commands from applications using lspfCallback() function.
	// Replies go into the reply vector.
	//

	string w1 ;
	string w2 ;

	pApplication* appl ;

	lc.reply.clear() ;

	w1 = word( lc.Command, 1 ) ;
	w2 = word( lc.Command, 2 ) ;

	if ( lc.Command == "SWAP LIST" )
	{
		for ( const auto screen : screenList )
		{
			appl = screen->application_get_current() ;
			lc.reply.push_back( d2ds( screen->get_screenNum() ) ) ;
			lc.reply.push_back( appl->get_appname() ) ;
		}
		lc.RC = 0 ;
	}
	else if ( lc.Command == "BATCH KEYS" )
	{
		mtx.lock() ;
		for ( const auto appl : pApplicationBackground )
		{
			lc.reply.push_back( appl->get_jobkey() ) ;
		}
		mtx.unlock() ;
		lc.RC = 0 ;
	}
	else if ( lc.Command == "MODULE STATUS" )
	{
		for ( const auto& app : apps )
		{
			lc.reply.push_back( app.first ) ;
			lc.reply.push_back( app.second.module ) ;
			lc.reply.push_back( app.second.file   ) ;
			if ( app.second.mainpgm )
			{
				lc.reply.push_back( "R/Not Reloadable" ) ;
			}
			else if ( app.second.relPending )
			{
				lc.reply.push_back( "Reload Pending" ) ;
			}
			else if ( app.second.errors )
			{
				lc.reply.push_back( "Errors" ) ;
			}
			else if ( app.second.refCount > 0 )
			{
				lc.reply.push_back( "Running" ) ;
			}
			else if ( app.second.dlopened )
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

	std::cout.rdbuf( old_cout_buf ) ;

	cout << "********************************************************************* " << endl ;
	cout << "********************************************************************* " << endl ;
	cout << "Aborting startup of lspf.  Check lspf and application logs for errors." << endl ;
	cout << "lspf log name. . . . . :"<< lg->logname() << endl ;
	cout << "Application log name . :"<< lgx->logname() << endl ;
	cout << "********************************************************************* " << endl ;
	cout << "********************************************************************* " << endl ;
	cout << endl ;

	delete lg  ;
	delete lgx ;

	abort() ;
}


void abnormalTermMessage()
{
	if ( currAppl->abnormalTimeout )
	{
		errorScreen( 2, "Application has been terminated at user request." ) ;
	}
	else if ( currAppl->abnormalEndForced )
	{
		errorScreen( 2, "A forced termination of the subtask has occured." ) ;
	}
	else if ( !currAppl->abnormalNoMsg )
	{
		errorScreen( 2, "An error has occured during application execution." ) ;
	}
}


void errorScreen( int etype,
		  const string& msg )
{
	string t ;

	llog( "E", msg << endl ) ;

	if ( currAppl->errPanelissued || currAppl->abnormalNoMsg  ) { return ; }

	createLinePanel() ;

	beep() ;
	wattrset( lwin, RED | A_BOLD ) ;
	mvwaddstr( lwin, linePosn++, 0, msg.c_str() ) ;
	mvwaddstr( lwin, linePosn++, 0, "See lspf and application logs for possible further details of the error." ) ;

	if ( etype == 2 )
	{
		t = "Failing application is " + currAppl->get_appname() + ", taskid=" + d2ds( currAppl->taskid() ) ;
		llog( "E", t << endl ) ;
		mvwaddstr( lwin, linePosn++, 0, "Depending on the error, application may still be running in the background.  Recommend restarting lspf." ) ;
		mvwaddstr( lwin, linePosn++, 0, t.c_str() ) ;
	}

	deleteLinePanel() ;

	currScrn->set_cursor( currAppl ) ;
}


void errorScreen( const string& title,
		  const string& src )
{
	//
	// Show dialogue error screen PSYSER2 with the contents of the errblock.
	//
	// This is done by starting application PPSP01A with a parm of PSYSER2 and
	// passing the error parameters via selobj.options.
	//
	// Make sure err_struct is the same in application PPSP01A.
	//

	selobj selct ;

	struct err_struct
	{
		string title ;
		string src   ;
		errblock err ;
	} errs ;

	errs.err   = err   ;
	errs.title = title ;
	errs.src   = src   ;

	selct.pgm  = "PPSP01A" ;
	selct.parm = "PSYSER1" ;
	selct.newappl = ""    ;
	selct.newpool = false ;
	selct.passlib = false ;
	selct.suspend = true  ;
	selct.options = (void*)&errs ;

	startApplication( selct ) ;
}


void errorScreen( const string& msg )
{
	//
	// Show dialogue error screen PSYSER3 with message 'msg'.
	//
	// This is done by starting application PPSP01A with a parm of PSYSER3 plus the message id.
	// ZVAL1-ZVAL3 need to be put into the SHARED pool depending on the message issued.
	//

	selobj selct ;

	selct.pgm  = "PPSP01A" ;
	selct.parm = "PSYSER3 "+ msg ;
	selct.newappl = ""    ;
	selct.newpool = false ;
	selct.passlib = false ;
	selct.suspend = true  ;

	startApplication( selct ) ;
}


void issueMessage( const string& msg,
		   const string& val )
{
	err.clear() ;

	currAppl->save_errblock() ;

	if ( val != "" )
	{
		p_poolMGR->put( err, "ZVAL1", val, SHARED ) ;
	}

	currAppl->set_msg( msg ) ;

	err = currAppl->get_errblock() ;
	currAppl->restore_errblock() ;
	if ( !err.RC0() )
	{
		errorScreen( 1, "Syntax error in message "+ msg +", message file or message not found." ) ;
		listErrorBlock( err ) ;
	}
}


void createLinePanel()
{
	if ( !lwin )
	{
		flushinp() ;
		lwin   = newwin( pLScreen::maxrow, pLScreen::maxcol, 0, 0 ) ;
		lpanel = new_panel( lwin ) ;
		top_panel( lpanel ) ;
		update_panels() ;
		linePosn = 0 ;
	}
}


void deleteLinePanel()
{
	if ( lwin )
	{
		flushinp() ;
		lineOutput_end() ;
		del_panel( lpanel ) ;
		delwin( lwin ) ;
		lwin = nullptr ;
	}
}


void serviceCallError( errblock& err )
{
	llog( "E", "A Serive Call error has occured"<< endl ) ;
	listErrorBlock( err ) ;
}


void listErrorBlock( errblock& err )
{
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
	return ( ( c >= KEY_F( 1 )  && c <= KEY_F( 24 ) ) ||
		 ( c >= CTRL( 'a' ) && c <= CTRL( 'z' ) && !CTRL( 'i' ) ) ||
		   c == CTRL( '[' ) ||
		   c == KEY_MOUSE   ||
		   c == KEY_NPAGE   ||
		   c == KEY_PPAGE   ||
		   c == KEY_ENTER ) ;
}


bool isEscapeKey()
{
	int c ;

	bool esc_pressed = false ;

	nodelay( stdscr, true ) ;
	do
	{
		c = getch() ;
		if ( c == CTRL( '[' ) ) { esc_pressed = true ; }
	} while ( c != -1 ) ;

	nodelay( stdscr, false ) ;

	return esc_pressed ;
}


void actionSwap( const string& parm )
{
	//
	// Swap primary and alternate logical screens using screen number, screen name or description.
	//

	int l ;

	uint i ;

	string w1 = word( parm, 1 ) ;
	string w2 = word( parm, 2 ) ;

	if ( !currAppl->currPanel->keep_cmd() )
	{
		currAppl->clear_msg() ;
		currAppl->currPanel->cmd_setvalue( "" ) ;
		currAppl->currPanel->cursor_to_cmdfield() ;
		currScrn->set_cursor( currAppl ) ;
	}

	if ( findword( w1, "LIST LISTN LISTP" ) )
	{
		w1 = listLogicalScreens() ;
		if ( w1 == d2ds( priScreen + 1 ) ) { return ; }
	}

	if ( pLScreen::screensTotal == 1 ) { return ; }

	currScrn->save_panel_stack() ;

	if ( w1 == "*" )
	{
		w1 = d2ds( priScreen+1 ) ;
	}
	else if ( w1 != "" && w1 != "NEXT" && w1 != "PREV" && !datatype( w1, 'W' ) )
	{
		l = getScreenNameNum( w1 ) ;
		if ( l > 0 ) { w1 = d2ds( l ) ; }
	}

	if ( w2 != "" && !datatype( w2, 'W' ) )
	{
		l = getScreenNameNum( w2 ) ;
		if ( l > 0 ) { w2 = d2ds( l ) ; }
	}

	if ( w1 == "NEXT" )
	{
		++priScreen ;
		priScreen = ( priScreen == pLScreen::screensTotal ) ? 0 : priScreen ;
		if ( altScreen == priScreen )
		{
			altScreen = ( altScreen == 0 ) ? pLScreen::screensTotal - 1 : altScreen - 1 ;
		}
	}
	else if ( w1 == "PREV" )
	{
		if ( priScreen == 0 )
		{
			priScreen = pLScreen::screensTotal - 1 ;
		}
		else
		{
			--priScreen ;
		}
		if ( altScreen == priScreen )
		{
			altScreen = ( altScreen == pLScreen::screensTotal - 1 ) ? 0 : altScreen + 1 ;
		}
	}
	else if ( datatype( w1, 'W' ) )
	{
		i = ds2d( w1 ) - 1 ;
		if ( i < pLScreen::screensTotal )
		{
			if ( i != priScreen )
			{
				if ( w2 == "*" || i == altScreen )
				{
					altScreen = priScreen ;
				}
				priScreen = i ;
			}
		}
		else
		{
			swap( priScreen, altScreen ) ;
		}
		if ( datatype( w2, 'W' ) && w1 != w2 )
		{
			i = ds2d( w2 ) - 1 ;
			if ( i != priScreen && i < pLScreen::screensTotal )
			{
				altScreen = i ;
			}
		}
	}
	else
	{
		swap( priScreen, altScreen ) ;
	}


	currScrn = screenList[ priScreen ] ;
	pLScreen::OIA_startTime() ;
	currAppl = currScrn->application_get_current() ;

	if ( !currAppl->currPanel->keep_cmd() )
	{
		currAppl->currPanel->cmd_setvalue( "" ) ;
		currAppl->currPanel->cursor_to_cmdfield() ;
		currScrn->set_cursor( currAppl ) ;
	}

	err.settask( currAppl->taskid() ) ;
	p_poolMGR->put( err, "ZPANELID", currAppl->get_panelid(), SHARED, SYSTEM ) ;
	currScrn->restore_panel_stack() ;
	currAppl->display_pd( err ) ;

	currAppl->reload_keylist() ;
	currAppl->build_pfkeys() ;
	currAppl->display_pfkeys() ;
}


int getScreenNameNum( const string& s )
{
	//
	// Return the screen number of screen 's'.  If not found, return 0.
	//
	// Screen is that used in the swapbar, ZSCRNAME or, if blank, panelid.
	// If a full match is not found, try to match an abbreviation.
	// If a match is still not found, try to matching on the application description (match anywhere).
	//

	int i = 1 ;
	int j = 0 ;

	string t ;

	pApplication* appl ;

	if ( isvalidName( s ) )
	{
		for ( const auto screen : screenList )
		{
			appl = screen->application_get_current() ;
			if ( appl->get_swb_name() == s )
			{
				return i ;
			}
			else if ( j == 0 && abbrev( appl->get_swb_name(), s ) )
			{
				j = i ;
			}
			++i ;
		}
	}

	if ( j == 0 )
	{
		i = 1 ;
		for ( const auto screen : screenList )
		{
			appl = screen->application_get_current() ;
			t    = upper( appl->get_current_panelDesc() ) ;
			if ( t.find( s ) != string::npos )
			{
				return i ;
			}
			++i ;
		}
	}

	return j ;
}


void actionTabKey( uint& row,
		   uint& col,
		   bool& pdchoice )
{
	//
	// Tab processsing:
	//     If a pull down is active, go to next pulldown.
	//     If cursor on a field that supports field execution and is not on the first char, or space
	//     before cursor, execute function
	//     Else act as a tab key to the next input field.
	//

	uint lrow = 0 ;
	uint lcol = 0 ;

	pdchoice = false ;

	bool tab_next = true ;

	string field_name ;

	fieldXct fxc ;

	if ( currAppl->currPanel->pd_active() )
	{
		displayNextPullDown() ;
		pdchoice = true ;
	}
	else
	{
		field_name = currAppl->currPanel->field_getname_input( row, col ) ;
		if ( field_name != "" )
		{
			currAppl->currPanel->field_get_row_col( field_name, lrow, lcol ) ;
			if ( lrow == row &&
			     lcol < col  &&
			     currAppl->currPanel->field_nonblank( field_name, col-lcol-1 ) )
			{
				fxc = currAppl->currPanel->field_getexec( field_name ) ;
				if ( fxc.fieldXct_command != "" )
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


void displayNextPullDown( const string& mnemonic )
{
	string msg ;
	string posn = word( mnemonic, 2 ) ;

	currAppl->currPanel->display_next_pd( err,
					      word( mnemonic, 1 ),
					      posn,
					      msg ) ;
	if ( err.error() )
	{
		errorScreen( err.msgid ) ;
		zcommand = "NOP" ;
	}
	else if ( msg != "" )
	{
		issueMessage( msg ) ;
	}
	else if ( currAppl->currPanel->simulate_enter() )
	{
		pApplication::controlNonDispl = true ;
	}
	else
	{
		currScrn->set_cursor( currAppl ) ;
	}
}


void displayPrevPullDown( const string& mnemonic )
{
	string msg ;
	string posn = word( mnemonic, 2 ) ;

	currAppl->currPanel->display_prev_pd( err,
					      word( mnemonic, 1 ),
					      posn,
					      msg ) ;
	if ( err.error() )
	{
		errorScreen( err.msgid ) ;
		zcommand = "NOP" ;
	}
	else if ( msg != "" )
	{
		issueMessage( msg ) ;
	}
	else if ( currAppl->currPanel->simulate_enter() )
	{
		pApplication::controlNonDispl = true ;
	}
	else
	{
		currScrn->set_cursor( currAppl ) ;
	}
}


void addRetrieveBuffer( const string& cmd,
			const string& pfcmd,
			bool replace )
{
	const string retlist = "CRETRIEV RETRIEVE RETF RETP" ;

	uint rbsize = ds2d( p_poolMGR->sysget( err, "ZRBSIZE", PROFILE ) ) ;
	uint rtsize = ds2d( p_poolMGR->sysget( err, "ZRTSIZE", PROFILE ) ) ;

	boost::circular_buffer<string>::iterator itt ;

	if ( retrieveBuffer.capacity() != rbsize )
	{
		retrieveBuffer.rset_capacity( rbsize ) ;
	}

	if ( cmd.size() >= rtsize &&
	    !findword( upper( word( cmd, 1 ) ), retlist ) &&
	    !findword( upper( word( pfcmd, 1 ) ), retlist ) )
	{
		itt = find( retrieveBuffer.begin(), retrieveBuffer.end(), cmd ) ;
		if ( itt != retrieveBuffer.end() )
		{
			retrieveBuffer.erase( itt ) ;
		}
		if ( replace && retrieveBuffer.size() > 0 )
		{
			retrieveBuffer.erase( retrieveBuffer.begin() ) ;
		}
		retrieveBuffer.push_front( cmd ) ;
		retPos = 0 ;
	}
}


void executeFieldCommand( const string& field_name,
			  const fieldXct& fxc,
			  uint col )
{
	//
	// Run application associated with a field when tab pressed or command FIELDEXC entered.
	//
	// Cursor position is stored in shared variable ZFECSRP.
	// Primary field is stored in shared variable ZFEDATA0.
	// Data to be passed to the application (fieldXct_passed) are stored in shared vars ZFEDATAn.
	//

	uint i ;
	uint lcol ;
	uint scol ;

	string w1 ;

	selobj selct ;

	if ( !selct.parse( err, subword( fxc.fieldXct_command, 2 ) ) )
	{
		llog( "E", "Error in FIELD SELECT command "+ fxc.fieldXct_command << "." << endl ) ;
		issueMessage( "PSYS011K" ) ;
		return ;
	}

	p_poolMGR->put( err, "ZFEDATA0", currAppl->currPanel->field_getrawvalue( field_name ), SHARED ) ;
	currAppl->reffield = field_name ;
	lcol = currAppl->currPanel->field_get_col( field_name ) ;
	scol = currAppl->currPanel->field_get_scroll_pos( field_name ) ;
	p_poolMGR->put( err, "ZFECSRP", d2ds( col - lcol + scol + 1 ), SHARED ) ;

	for ( i = 1 ; i <= fxc.fieldXct_pnum ; ++i )
	{
		w1 = word( fxc.fieldXct_passed, i ) ;
		p_poolMGR->put( err, "ZFEDATA" + d2ds( i ), currAppl->currPanel->field_getrawvalue( w1 ), SHARED ) ;
	}

	startApplication( selct ) ;

	while ( currAppl->SEL && !currAppl->terminateAppl )
	{
		processPGMSelect() ;
		while ( currAppl->terminateAppl )
		{
			terminateApplication() ;
			if ( pLScreen::screensTotal == 0 ) { return ; }
		}
	}

	while ( currAppl->terminateAppl )
	{
		terminateApplication() ;
		if ( pLScreen::screensTotal == 0 ) { return ; }
		while ( currAppl->SEL && !currAppl->terminateAppl )
		{
			processPGMSelect() ;
		}
	}
}


void executeHistoryCommand( const string& panel,
			    const string& field,
			    uint prow,
			    uint pcol,
			    uint frow,
			    uint fcol )
{
	//
	// Run application to display field history.
	//

	selobj selct ;

	currAppl->reffield = field ;

	selct.pgm  = p_poolMGR->sysget( err, "ZFHSTPGM", PROFILE ) ;
	selct.parm = "PLD " + panel + " " + " " + d2ds( prow ) + " " + d2ds( pcol ) + " " + d2ds( frow ) + " " + d2ds( fcol ) + " " + field ;

	startApplication( selct ) ;

	while ( currAppl->SEL && !currAppl->terminateAppl )
	{
		processPGMSelect() ;
		while ( currAppl->terminateAppl )
		{
			terminateApplication() ;
			if ( pLScreen::screensTotal == 0 ) { return ; }
		}
	}

	while ( currAppl->terminateAppl )
	{
		terminateApplication() ;
		if ( pLScreen::screensTotal == 0 ) { return ; }
		while ( currAppl->SEL && !currAppl->terminateAppl )
		{
			processPGMSelect() ;
		}
	}
}


void getDynamicClasses()
{
	//
	// Get modules of the form libABCDE.so from ZLDPATH concatination with name ABCDE and store in map apps.
	// Duplicates are ignored with a warning messasge.
	// Terminate lspf if ZMAINPGM module not found as we cannot continue.
	//

	int pos1 ;

	string appl ;
	string mod ;
	string p ;

	using vec = vector<path> ;
	vec v ;

	err.clear() ;

	appInfo aI ;

	const string e1( gmainpgm +" not found.  Check ZLDPATH is correct.  lspf terminating **" ) ;

	getModuleEntries( v ) ;

	for ( const auto& ent : v )
	{
		p    = ent.parent_path().string() ;
		mod  = ent.filename().string() ;
		pos1 = pos( ".so", mod ) ;
		if ( mod.compare( 0, 3, "lib" ) != 0 || pos1 == 0 ) { continue ; }
		appl  = upper( substr( mod, 4, ( pos1 - 4 ) ) ) ;
		if ( apps.count( appl ) == 0 )
		{
			llog( "I", "Adding application "+ appl << "." << endl ) ;
			aI.file       = ent.string() ;
			aI.module     = mod   ;
			aI.mainpgm    = false ;
			aI.dlopened   = false ;
			aI.errors     = false ;
			aI.relPending = false ;
			aI.refCount   = 0     ;
			apps[ appl ]  = aI    ;
		}
		else
		{
			llog( "W", "Ignoring duplicate module "+ mod +" found in "+ p << "." << endl ) ;
		}
	}

	llog( "I", d2ds( apps.size() ) +" applications found and stored." << endl ) ;

	if ( apps.count( gmainpgm ) == 0 )
	{
		llog( "S", e1 << endl ) ;
		abortStartup() ;
	}
}


void reloadDynamicClasses( string parm )
{
	//
	// Reload modules (ALL, NEW or modname).  Ignore reload for modules currently in-use but set
	// pending flag to be checked when application terminates.
	//

	int i ;
	int j ;
	int k ;
	int pos1 ;

	string appl ;
	string mod ;
	string p ;

	bool stored ;

	err.clear() ;

	using vec = vector<path> ;

	vec v ;
	appInfo aI ;

	getModuleEntries( v ) ;

	if ( parm == "" ) { parm = "ALL" ; }

	i = 0 ;
	j = 0 ;
	k = 0 ;
	for ( const auto& ent : v )
	{
		p     = ent.parent_path().string() ;
		mod   = ent.filename().string() ;
		pos1  = pos( ".so", mod ) ;
		if ( mod.compare( 0, 3, "lib" ) != 0 || pos1 == 0 ) { continue ; }
		appl  = upper( substr( mod, 4, ( pos1 - 4 ) ) ) ;
		llog( "I", "Found application "+ appl << "." << endl ) ;
		stored = ( apps.count( appl ) > 0 ) ;

		if ( ( parm == "NEW" && stored ) ||
		     ( parm != "NEW" && parm != "ALL" && parm != appl ) ||
		     ( appl == gmainpgm ) )
		{
			continue ;
		}
		if ( parm == appl && stored && !apps[ appl ].dlopened )
		{
			apps[ appl ].file = ent.string() ;
			llog( "W", "Application "+ appl +" not loaded.  Ignoring action." << endl ) ;
			return ;
		}
		if ( stored )
		{
			apps[ appl ].file = ent.string() ;
			if ( apps[ appl ].refCount > 0 )
			{
				llog( "W", "Application "+ appl +" in use.  Reload pending." << endl ) ;
				apps[ appl ].relPending = true ;
				continue ;
			}
			if ( apps[ appl ].dlopened )
			{
				if ( loadDynamicClass( appl ) )
				{
					llog( "I", "Loaded "+ appl +" (module "+ mod +") from "+ p << "." << endl ) ;
					++i ;
				}
				else
				{
					llog( "W", "Errors occured loading "+ appl << "." << endl ) ;
					++k ;
				}
			}
		}
		else
		{
			llog( "I", "Adding new module "+ appl << "." << endl ) ;
			aI.file        = ent.string() ;
			aI.module      = mod ;
			aI.mainpgm     = false ;
			aI.dlopened    = false ;
			aI.errors      = false ;
			aI.relPending  = false ;
			aI.refCount    = 0 ;
			apps[ appl ]   = aI ;
			++j ;
		}
		if ( parm == appl ) { break ; }
	}

	issueMessage( "PSYS012G" ) ;

	llog( "I", d2ds( i ) +" applications reloaded." << endl ) ;
	llog( "I", d2ds( j ) +" new applications stored." << endl ) ;
	llog( "I", d2ds( k ) +" errors encounted." << endl ) ;

	if ( parm != "ALL" && parm != "NEW" )
	{
		if ( ( i + j ) == 0 )
		{
			llog( "W", "Application "+ parm +" not reloaded/stored." << endl ) ;
			issueMessage( "PSYS012I" ) ;
		}
		else
		{
			llog( "I", "Application "+ parm +" reloaded/stored." << endl ) ;
			issueMessage( "PSYS012H" ) ;
		}
	}

	llog( "I", d2ds( apps.size() ) + " applications currently stored." << endl ) ;
}


void getModuleEntries( vector<path>& v )
{
	//
	// Get all entries in the ZLDPATH variable.
	//

	int i ;
	int j ;

	string p ;

	string paths = p_poolMGR->sysget( err, "ZLDPATH", PROFILE ) ;

	llog( "I", "Searching directories in ZLDPATH for application classes:" << endl ) ;
	for ( j = getpaths( paths ), i = 1 ; i <= j ; ++i )
	{
		p = getpath( paths, i ) ;
		if ( is_directory( p ) )
		{
			llog( "I", ">> "+ p << endl ) ;
			copy( directory_iterator( p ), directory_iterator(), back_inserter( v ) ) ;
		}
		else
		{
			llog( "W", "Ignoring directory "+ p +"  Not found or not a directory." << endl ) ;
		}
	}

	sort( v.begin(), v.end() ) ;

}


bool loadDynamicClass( const string& appl )
{
	//
	// Load module related to application appl and retrieve address of maker and destroy symbols.
	// Perform dlclose first if there has been a previous successful dlopen, or if an error is encountered.
	//
	// Routine only called if the refCount is zero.
	//

	string mod   ;
	string fname ;

	void* dlib  ;
	void* maker ;
	void* destr ;

	const char* dlsym_err ;

	mod   = apps[ appl ].module ;
	fname = apps[ appl ].file   ;
	apps[ appl ].errors = true  ;

	if ( apps[ appl ].dlopened )
	{
		llog( "I", "Closing "+ appl << "." << endl ) ;
		if ( !unloadDynamicClass( apps[ appl ].dlib ) )
		{
			llog( "W", "dlclose has failed for "+ appl << "." << endl ) ;
			return false ;
		}
		apps[ appl ].dlopened = false ;
		llog( "I", "Reloading module "+ appl << "." << endl ) ;
	}

	dlerror() ;
	dlib = dlopen( fname.c_str(), RTLD_NOW ) ;
	if ( !dlib )
	{
		llog( "E", "Error loading "+ fname << "." << endl )  ;
		llog( "E", "Error is " << dlerror() << "." << endl ) ;
		llog( "E", "Module "+ mod +" will be ignored." << endl ) ;
		return false ;
	}

	dlerror() ;
	maker     = dlsym( dlib, "maker" ) ;
	dlsym_err = dlerror() ;
	if ( dlsym_err )
	{
		llog( "E", "Error loading symbol maker." << endl ) ;
		llog( "E", "Error is " << dlsym_err << "." << endl ) ;
		llog( "E", "Module "+ mod +" will be ignored." << endl ) ;
		unloadDynamicClass( apps[ appl ].dlib ) ;
		return false ;
	}

	dlerror() ;
	destr     = dlsym( dlib, "destroy" ) ;
	dlsym_err = dlerror() ;
	if ( dlsym_err )
	{
		llog( "E", "Error loading symbol destroy." << endl ) ;
		llog( "E", "Error is " << dlsym_err << endl ) ;
		llog( "E", "Module "+ mod +" will be ignored." << endl ) ;
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


bool unloadDynamicClass( void* dlib )
{
	int i ;
	int rc = 0 ;

	for ( i = 0 ; i < 10 && rc == 0 ; ++i )
	{
		try
		{
			rc = dlclose( dlib ) ;
		}
		catch (...)
		{
			llog( "E", "An exception has occured during dlclose." << endl ) ;
			return false ;
		}
	}

	return ( rc != 0 ) ;
}
