/* Compile with ::                                                                         */
/* g++ -rdynamic -std=c++11 -o setup -lboost_filesystem -lboost_system -lpthread setup.cpp */

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

/********************************************************************************/
/*                                                                              */
/* Amend and run to create initial ISPCMDS and USRCMDS command tables           */
/* and ISPSPROF default profile.  Values taken from lspf.h                      */
/*                                                                              */
/* *NOTE* Any changes to ISPSPROF done online, will be lost                     */
/*                                                                              */
/* USRCMDS is the default user command table 1 (ZUCMDT1)                        */
/* This can be changed in option 0.0, General lspf Settings.                    */
/*                                                                              */
/* Create function pool, pool manager and table manager instances.              */
/*                                                                              */
/* *NOTE*                                                                       */
/* Resolve any warnings issued.  lspf may not start if important files or       */
/* directories are missing.                                                     */
/*                                                                              */
/********************************************************************************/

#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <time.h>
#include <vector>
#include <sys/utsname.h>

#include <locale>

#include <boost/filesystem.hpp>

#include "lspf.h"

#include "utilities.h"
#include "utilities.cpp"

#include "colours.h"

#include "classes.h"
#include "classes.cpp"

#include "pVPOOL.h"
#include "pVPOOL.cpp"

#include "pTable.h"
#include "pTable.cpp"


#define check true

using namespace std ;
using namespace boost::filesystem ;

poolMGR*  p_poolMGR  = new poolMGR  ;
tableMGR* p_tableMGR = new tableMGR ;

fPOOL* funcPOOL = new fPOOL ;

logger* lg = new logger ;

logger* tableMGR::lg = nullptr ;
logger* poolMGR::lg  = nullptr ;

uint   pVPOOL::pfkgToken = 0 ;
uint   Table::pflgToken  = 1 ;

string getEnvironmentVariable( const char* ) ;

void   createSYSPROF() ;

void   setCUAcolours( const string&,
		      const string& ) ;

string subHomePath( string,
		    bool = false ) ;

string getHOME() ;

int main()
{
	errblock err ;

	tableMGR::lg = lg ;
	poolMGR::lg  = lg ;

	string homePath ;
	string logname ;

	lg->open( subHomePath( ALOG ) ) ;

	homePath = getHOME() + ZUPROF ;
	if ( homePath.back() != '/' ) { homePath += "/" ; }

	if ( !exists( homePath ) || !is_directory( homePath ) )
	{
		cout << endl ;
		cout << "ERROR: ZUPROF path " << homePath << " does not exist or is not a directory." << endl ;
		cout << "       Please create before re-running setup." << endl ;
		return 0 ;
	}

	if ( MXTAB_SZ > 16777215 )
	{
		cout << endl ;
		cout << "ERROR: MXTAB_SZ has been exceeded.  Maximum value is 16,777,215 and coded value is " << MXTAB_SZ << endl ;
		cout << "       Please change in lspf.h before re-running setup." << endl ;
		return 0 ;
	}

	logname = getEnvironmentVariable( "LOGNAME" ) ;
	if ( logname == "" )
	{
		cout << endl ;
		cout << "ERROR: LOGNAME variable is required and must be set" << endl ;
		return 0 ;
	}

	err.user = logname ;

	string zctverb ;
	string zctact ;
	string zctdesc ;
	string zcttrunc ;

	funcPOOL->define( err, "ZCTVERB",  &zctverb  ) ;
	funcPOOL->define( err, "ZCTTRUNC", &zcttrunc ) ;
	funcPOOL->define( err, "ZCTACT",   &zctact   ) ;
	funcPOOL->define( err, "ZCTDESC",  &zctdesc  ) ;

	p_tableMGR->tbcreate( err,
			      "ISPCMDS",
			      "",
			      "ZCTVERB ZCTTRUNC ZCTACT ZCTDESC",
			      WRITE,
			      NOREPLACE,
			      "",
			      SHARE ) ;

	p_tableMGR->tbcreate( err,
			      "USRCMDS",
			      "",
			      "ZCTVERB ZCTTRUNC ZCTACT ZCTDESC",
			      WRITE,
			      NOREPLACE,
			      "",
			      SHARE ) ;

	zctverb  = "SCRNAME" ;
	zcttrunc = "0"    ;
	zctact   = "SCRNAME" ;
	zctdesc  = "Set screen name for session" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "BACKWARD" ;
	zcttrunc = "0"    ;
	zctact   = "ALIAS UP" ;
	zctdesc  = "Up" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "FORWARD" ;
	zcttrunc = "0"    ;
	zctact   = "ALIAS DOWN" ;
	zctdesc  = "Down" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "ACTIONS" ;
	zcttrunc = "0"    ;
	zctact   = "ACTIONS" ;
	zctdesc  = "Switch to first or next action bar choice" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "CURSOR" ;
	zcttrunc = "0"    ;
	zctact   = "CURSOR" ;
	zctdesc  = "Move the cursor to the first input field" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "SWAP" ;
	zcttrunc = "0"    ;
	zctact   = "SWAP" ;
	zctdesc  = "Swap logical screens" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "SPLIT" ;
	zcttrunc = "0"     ;
	zctact   = "SPLIT" ;
	zctdesc  = "Split screen" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "RETRIEVE" ;
	zcttrunc = "0"        ;
	zctact   = "RETRIEVE" ;
	zctdesc  = "Retrieve command" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "CRETRIEV" ;
	zcttrunc = "0"        ;
	zctact   = "CRETRIEV" ;
	zctdesc  = "Conditional retrieve command" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "RETF" ;
	zcttrunc = "0"    ;
	zctact   = "RETF" ;
	zctdesc  = "Forward retrieve command" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "RETURN"  ;
	zcttrunc = "0"       ;
	zctact   = "SETVERB" ;
	zctdesc  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "RFIND" ;
	zcttrunc = "0"     ;
	zctact   = "SETVERB" ;
	zctdesc  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "RCHANGE" ;
	zcttrunc = "0"       ;
	zctact   = "SETVERB" ;
	zctdesc  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "END"    ;
	zcttrunc = "0"      ;
	zctact   = "SETVERB" ;
	zctdesc  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "HELP" ;
	zcttrunc = "0"    ;
	zctact   = "SELECT PGM(&ZHELPPGM) PARM(&ZPARM) NOFUNC SCRNAME(HELP)"  ;
	zctdesc  = "Display help" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "EXHELP" ;
	zcttrunc = "0"    ;
	zctact   = "SELECT PGM(&ZHELPPGM) PARM(&ZPARM) NOFUNC SCRNAME(HELP)"  ;
	zctdesc  = "Display extended help" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "KEYSHELP" ;
	zcttrunc = "0"    ;
	zctact   = "SELECT PGM(&ZHELPPGM) PARM(&ZPARM) NOFUNC SCRNAME(HELP)"  ;
	zctdesc  = "Dispay keylist help" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "TUTOR" ;
	zcttrunc = "0"    ;
	zctact   = "SELECT PGM(&ZHELPPGM) PARM(&ZPARM) SCRNAME(TUTOR)"  ;
	zctdesc  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "UP"    ;
	zcttrunc = "0"     ;
	zctact   = "SETVERB" ;
	zctdesc  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "DOWN"  ;
	zcttrunc = "0"     ;
	zctact   = "SETVERB" ;
	zctdesc  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "LEFT"  ;
	zcttrunc = "0"     ;
	zctact   = "SETVERB" ;
	zctdesc  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "TOP"   ;
	zcttrunc = "0"     ;
	zctact   = "ALIAS UP MAX" ;
	zctdesc  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "BOTTOM" ;
	zcttrunc = "3"      ;
	zctact   = "ALIAS DOWN MAX" ;
	zctdesc  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "RIGHT" ;
	zcttrunc = "0"     ;
	zctact   = "SETVERB" ;
	zctdesc  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "ALOG" ;
	zcttrunc = "3"  ;
	zctact   = "SELECT PGM(PPSP01A) PARM(AL) SCRNAME(LOG) SUSPEND" ;
	zctdesc  = "Browse application log" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "LOG" ;
	zcttrunc = "3"   ;
	zctact   = "SELECT PGM(PPSP01A) PARM(SL) SCRNAME(LOG) SUSPEND" ;
	zctdesc  = "Browse lspf log" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "KEYLIST"  ;
	zcttrunc = "0"        ;
	zctact   = "SELECT PGM(PPSP01A) PARM(KLIST &ZPARM) SCRNAME(KEYLIST) SUSPEND" ;
	zctdesc  = "Keylist utility"  ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "KEYLISTS" ;
	zcttrunc = "0"        ;
	zctact   = "SELECT PGM(PPSP01A) PARM(KLISTS) SCRNAME(KEYLIST) SUSPEND" ;
	zctdesc  = "Keylist utility"  ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "DSLIST" ;
	zcttrunc = "0"      ;
	zctact   = "SELECT PGM(PPSP01A) PARM(DSL &ZPARM) NEWAPPL(ISR) SCRNAME(DSLIST) SUSPEND" ;
	zctdesc  = "File list utility"  ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "DXLIST" ;
	zcttrunc = "0"      ;
	zctact   = "SELECT PGM(PPSP01A) PARM(DSX &ZPARM) NEWAPPL(ISR) SCRNAME(DXLIST) SUSPEND" ;
	zctdesc  = "File list utility"  ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "KEYS" ;
	zcttrunc = "0"    ;
	zctact   = "SELECT PGM(PPSP01A) PARM(KEYS) SCRNAME(KEYS) SUSPEND" ;
	zctdesc  = "PFkey utility" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "ZKEYS" ;
	zcttrunc = "0"    ;
	zctact   = "ALIAS KEYS" ;
	zctdesc  = "PFkey utility" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "KEYL" ;
	zcttrunc = "0"    ;
	zctact   = "SELECT PGM(PPSP01A) PARM(KEYL) SCRNAME(KEYLIST) SUSPEND" ;
	zctdesc  = "Keylist change utility" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "CTRL" ;
	zcttrunc = "0"    ;
	zctact   = "SELECT PGM(PPSP01A) PARM(CTLKEYS) SCRNAME(CTLKEYS) SUSPEND" ;
	zctdesc  = "Control key utility" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "NOP" ;
	zcttrunc = "0"    ;
	zctact   = "NOP" ;
	zctdesc  = "No operation" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "ISPLIBD" ;
	zcttrunc = "0"        ;
	zctact   = "SELECT PGM(PPSP01A) PARM(LIBDEFS &ZPARM) SCRNAME(LIBDUTL) SUSPEND" ;
	zctdesc  = "Display LIBDEF status" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "RECENT" ;
	zcttrunc = "0"      ;
	zctact   = "ALIAS DSLIST REFLIST" ;
	zctdesc  = "Recent file list utility"  ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "PFSHOW" ;
	zcttrunc = "0"      ;
	zctact   = "SELECT PGM(PPSP01A) PARM(PFK &ZPARM) SCRNAME(PFSHOW) SUSPEND" ;
	zctdesc  = "Mange function key display area"  ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "RUN" ;
	zcttrunc = "2"    ;
	zctact   = "SELECT PGM(PPSP01A) PARM(RUN &ZPARM) NEWPOOL SUSPEND" ;
	zctdesc  = "Run an application by name" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "RETK" ;
	zcttrunc = "0"    ;
	zctact   = "RETK" ;
	zctdesc  = "List keep buffer" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "RETP" ;
	zcttrunc = "0"    ;
	zctact   = "RETP" ;
	zctdesc  = "List retrieve buffer" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "SETTINGS"  ;
	zcttrunc = "0"         ;
	zctact   = "SELECT PANEL(PPSET0A) SCRNAME(SETTINGS) SUSPEND" ;
	zctdesc  = "Settings selection menu"  ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 )  ;

	zctverb  = "START"    ;
	zcttrunc = "0"        ;
	zctact   = "SELECT PGM(ISPSTRT) PARM(&ZPARM)" ;
	zctdesc  = "Start a program in a new logical screen" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "REFACTF" ;
	zcttrunc = "5"       ;
	zctact   = "SELECT PGM(&ZRFLPGM) PARM(PL1 &ZPARM) SCRNAME(REFACT) SUSPEND" ;
	zctdesc  = "Open active referral list" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "REFADDF" ;
	zcttrunc = "5"       ;
	zctact   = "SELECT PGM(&ZRFLPGM) PARM(PLA &ZPARM) SUSPEND" ;
	zctdesc  = "Add file to reference list" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "REFLISTF" ;
	zcttrunc = "4"        ;
	zctact   = "SELECT PGM(&ZRFLPGM) PARM(PL1 REFLIST &ZPARM) SCRNAME(REFLIST) SUSPEND" ;
	zctdesc  = "Open reference list" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "REFOPENF" ;
	zcttrunc = "4"        ;
	zctact   = "SELECT PGM(&ZRFLPGM) PARM(PL2) SCRNAME(REFOPEN) SUSPEND"  ;
	zctdesc  = "Open file referral lists" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "NRETRIEV" ;
	zcttrunc = "0"        ;
	zctact   = "SETVERB"  ;
	zctdesc  = "Retrieve next entry from active referral list" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "CANCEL" ;
	zcttrunc = "3"      ;
	zctact   = "CANCEL" ;
	zctdesc  = "Cancel" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "EXIT" ;
	zcttrunc = "0"    ;
	zctact   = "EXIT" ;
	zctdesc  = "Exit" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "USERID" ;
	zcttrunc = "0"      ;
	zctact   = "USERID" ;
	zctdesc  = "Userid" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "CMDE" ;
	zcttrunc = "0" ;
	zctact   = "SELECT PGM(PCMD0A) PARM(PANEL(PCMD0E)) SUSPEND" ;
	zctdesc  = "Shell" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "WINDOW" ;
	zcttrunc = "0" ;
	zctact   = "WINDOW" ;
	zctdesc  = "Move window to cursor position" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "FKA" ;
	zcttrunc = "0" ;
	zctact   = "SELECT PGM(PPSP01A) PARM(FKA &ZPARM) SUSPEND" ;
	zctdesc  = "Control pfkey display area" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "ZCLRSFLD" ;
	zcttrunc = "0" ;
	zctact   = "SETVERB" ;
	zctdesc  = "Clear scrollable field" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "ZEXPAND" ;
	zcttrunc = "0" ;
	zctact   = "SETVERB" ;
	zctdesc  = "Expand scrollable field" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	zctverb  = "ZHISTORY" ;
	zcttrunc = "0" ;
	zctact   = "SETVERB" ;
	zctdesc  = "List field history" ;

	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	// ========================= USRCMDS ======================================
	zctverb  = "ED" ;
	zcttrunc = "0" ;
	zctact   = "SELECT PGM(PPSP01A) NEWAPPL(ISR) PARM(EDITEE &ZPARM) SCRNAME(EDIT) SUSPEND" ;
	zctdesc  = "Invoke EDIT Entry panel" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	zctverb  = "BR" ;
	zcttrunc = "0" ;
	zctact   = "SELECT PGM(PPSP01A) NEWAPPL(ISR) PARM(BROWSEE &ZPARM) SCRNAME(VIEW) SUSPEND" ;
	zctdesc  = "Invoke browse" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	zctverb  = "DDLIST" ;
	zcttrunc = "0" ;
	zctact   = "SELECT CMD(%ddlist) NEWAPPL(ISR) SCRNAME(DDLIST) SUSPEND" ;
	zctdesc  = "Show allocations" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	zctverb  = "DIR" ;
	zcttrunc = "0" ;
	zctact   = "SELECT PGM(&ZFLSTPGM) NEWAPPL(ISR) PARM(&ZPARM) SCRNAME(FILES) SUSPEND" ;
	zctdesc  = "Show files/directories" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	zctverb  = "CMDS" ;
	zcttrunc = "0"    ;
	zctact   = "SELECT PGM(PPSP01A) PARM(CMDS) SCRNAME(COMMANDS) SUSPEND"  ;
	zctdesc  = "Display command tables" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	zctverb  = "TASKS" ;
	zcttrunc = "0"     ;
	zctact   = "SELECT PGM(PSYSUTL) PARM(TASKS) SCRNAME(TASKS) SUSPEND"  ;
	zctdesc  = "Show task list" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	zctverb  = "FLIST" ;
	zcttrunc = "2"     ;
	zctact   = "SELECT PGM(PPSP01A) PARM(SAVELST) NEWAPPL SCRNAME(SAVED) SUSPEND"  ;
	zctdesc  = "Show saved file list" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	zctverb  = "MODS" ;
	zcttrunc = "0"     ;
	zctact   = "SELECT PGM(PPSP01A) PARM(MODS) SCRNAME(MODULES) SUSPEND"  ;
	zctdesc  = "Show loaded dynamic classes" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	zctverb  = "PATHS" ;
	zcttrunc = "0"     ;
	zctact   = "SELECT PGM(PPSP01A) PARM(PATHS) SCRNAME(PATHS) SUSPEND"  ;
	zctdesc  = "Show search paths for panels, messages and tables" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	zctverb  = "EXEC" ;
	zcttrunc = "3"     ;
	zctact   = "SELECT CMD(&ZPARM) LANG(REXX) NEWPOOL SUSPEND" ;
	zctdesc  = "Run OOREXX EXEC" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	zctverb  = "TODO" ;
	zcttrunc = "0"   ;
	zctact   = "SELECT PGM(PPSP01A) PARM(TODO) NEWAPPL(ISR) SCRNAME(TODO) SUSPEND" ;
	zctdesc  = "Browse todo list" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	zctverb  = "VARS" ;
	zcttrunc = "3"    ;
	zctact   = "SELECT PGM(PPSP01A) PARM(VARS &ZPARM) SCRNAME(VARS) SUSPEND"  ;
	zctdesc  = "Display shared and profile variables" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	zctverb  = "CUAATTR" ;
	zcttrunc = "3"   ;
	zctact   = "SELECT PGM(PPSP01A) PARM(CUAATTR) SCRNAME(CUA) SUSPEND"  ;
	zctdesc  = "Display/change CUA colours" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	p_tableMGR->tbsort( err, "ISPCMDS", "ZCTVERB,C,A" ) ;
	p_tableMGR->tbsort( err, "USRCMDS", "ZCTVERB,C,A" ) ;

	cout << endl ;
	cout << "*******************************************************************************************" << endl ;

	p_tableMGR->saveTable( err, "", "ISPCMDS", "", homePath  ) ;
	if ( err.RC0() )
	{
		cout << endl ;
		cout << "ISPCMDS table created successfully in " << homePath << endl ;
	}
	else
	{
		cout << endl ;
		cout << "ERROR saving ISPCMDS table in " << homePath << "  RC=" << err.getRC() << endl ;
		cout << "Message is " << err.msgid << endl ;
	}

	p_tableMGR->saveTable( err, "", "USRCMDS", "", homePath ) ;
	if ( err.RC0() )
	{
		cout << endl ;
		cout << "USRCMDS table created successfully in " << homePath << endl ;
	}
	else
	{
		cout << endl ;
		cout << "ERROR saving USRCMDS table in " << homePath << "  RC=" << err.getRC() << endl ;
		cout << "Message is " << err.msgid << endl ;
	}

	createSYSPROF() ;

	cout << "See application log in "<< subHomePath( ALOG ) << endl ;
	cout << "if any errors have been encountered" << endl ;
	cout << "*******************************************************************************************" << endl ;

	delete p_poolMGR  ;
	delete p_tableMGR ;
	delete lg         ;
	delete funcPOOL   ;

	return 0 ;
}


void createSYSPROF()
{
	string p ;
	string zuprof ;

	errblock err( 1 ) ;

	zuprof = getHOME() + ZUPROF ;
	p_poolMGR->setProfilePath( err, zuprof ) ;

	p_poolMGR->createSharedPool() ;
	p_poolMGR->createProfilePool( err, "ISPS", true ) ;
	p_poolMGR->connect( 1, "ISPS", 1 ) ;

	p_poolMGR->put( err, "ZUPROF", zuprof, PROFILE ) ;
	p_poolMGR->put( err, "ZSYSPATH", ZSYSPATH, PROFILE ) ;
	p_poolMGR->put( err, "ZORXPATH", subHomePath( ZREXPATH, check ), PROFILE ) ;

	p_poolMGR->put( err, "ZLDPATH", ZLDPATH, PROFILE ) ;

	p_poolMGR->put( err, "ZSLOG", subHomePath( SLOG ), PROFILE ) ;
	p_poolMGR->put( err, "ZALOG", subHomePath( ALOG ), PROFILE ) ;

	p_poolMGR->put( err, "ZDEFM",    "N",    PROFILE ) ;
	p_poolMGR->put( err, "ZDEL",     ";",    PROFILE ) ;
	p_poolMGR->put( err, "ZFKA",     "LONG", PROFILE ) ;
	p_poolMGR->put( err, "ZKLUSE",   "N",    PROFILE ) ;
	p_poolMGR->put( err, "ZKLPRIV",  "Y",    PROFILE ) ;
	p_poolMGR->put( err, "ZNOTIFY",  "Y",    PROFILE ) ;
	p_poolMGR->put( err, "ZHIGH",    "Y",    PROFILE ) ;
	p_poolMGR->put( err, "ZLMSGW",   "N",    PROFILE ) ;
	p_poolMGR->put( err, "ZPADC",    "_",    PROFILE ) ;
	p_poolMGR->put( err, "ZPFSET",   "PRI",  PROFILE ) ;
	p_poolMGR->put( err, "ZPFSHOW",  "ON",   PROFILE ) ;
	p_poolMGR->put( err, "ZPRIKEYS", "LOW",  PROFILE ) ;
	p_poolMGR->put( err, "ZRTSIZE",  "3",    PROFILE ) ;
	p_poolMGR->put( err, "ZRBSIZE",  "20",   PROFILE ) ;
	p_poolMGR->put( err, "ZSWAP",    "Y",    PROFILE ) ;
	p_poolMGR->put( err, "ZSWAPC",   "'",    PROFILE ) ;
	p_poolMGR->put( err, "ZSWPBR",   "Y",    PROFILE ) ;
	p_poolMGR->put( err, "ZSCROLLD", "HALF", PROFILE ) ;
	p_poolMGR->put( err, "ZSRETP",   "Y",    PROFILE ) ;
	p_poolMGR->put( err, "ZTABPAS",  "N",    PROFILE ) ;
	p_poolMGR->put( err, "ZDECLR",   "Y",    PROFILE ) ;
	p_poolMGR->put( err, "ZDECLA",   "N",    PROFILE ) ;
	p_poolMGR->put( err, "ZDECLRA",  "BH",   PROFILE ) ;
	p_poolMGR->put( err, "ZHIABMN",  "N",    PROFILE ) ;

	p_poolMGR->put( err, "ZMOUSEIN",  "166", PROFILE ) ;
	p_poolMGR->put( err, "ZMOUSESA",  "1",   PROFILE ) ;

	p_poolMGR->put( err, "ZMOUSE11",  "2",    PROFILE ) ;
	p_poolMGR->put( err, "ZMOUSE12",  "4",    PROFILE ) ;
	p_poolMGR->put( err, "ZMOUSE13",  "6ZHISTORY",  PROFILE ) ;
	p_poolMGR->put( err, "ZMOUSE21",  "5SWAP NEXT", PROFILE ) ;
	p_poolMGR->put( err, "ZMOUSE22",  "5SPLIT NEW", PROFILE ) ;
	p_poolMGR->put( err, "ZMOUSE23",  "6:TS", PROFILE ) ;
	p_poolMGR->put( err, "ZMOUSE31",  "5END", PROFILE ) ;
	p_poolMGR->put( err, "ZMOUSE32",  "5RETURN", PROFILE ) ;
	p_poolMGR->put( err, "ZMOUSE33",  "1",    PROFILE ) ;

	p_poolMGR->put( err, "ZURGBR", "", PROFILE ) ;
	p_poolMGR->put( err, "ZURGBG", "", PROFILE ) ;
	p_poolMGR->put( err, "ZURGBY", "", PROFILE ) ;
	p_poolMGR->put( err, "ZURGBB", "", PROFILE ) ;
	p_poolMGR->put( err, "ZURGBM", "", PROFILE ) ;
	p_poolMGR->put( err, "ZURGBT", "", PROFILE ) ;
	p_poolMGR->put( err, "ZURGBW", "", PROFILE ) ;

	p_poolMGR->put( err, "ZUCMDT1", "USR", PROFILE ) ;
	p_poolMGR->put( err, "ZUCMDT2", "", PROFILE ) ;
	p_poolMGR->put( err, "ZUCMDT3", "", PROFILE ) ;

	p_poolMGR->put( err, "ZSCMDT1", "", PROFILE ) ;
	p_poolMGR->put( err, "ZSCMDT2", "", PROFILE ) ;
	p_poolMGR->put( err, "ZSCMDT3", "", PROFILE ) ;

	p_poolMGR->put( err, "ZSCMDTF", "Y", PROFILE ) ;

	p_poolMGR->put( err, "ZMLIB", subHomePath( MLIB, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZPLIB", subHomePath( PLIB, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZSLIB", subHomePath( SLIB, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZLLIB", subHomePath( LLIB, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZTLIB", subHomePath( TLIB, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZTABL", subHomePath( TABL, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZFILE", subHomePath( TABU, check ), PROFILE ) ;

	p_poolMGR->put( err, "ZMUSR", subHomePath( MUSR, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZPUSR", subHomePath( PUSR, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZSUSR", subHomePath( SUSR, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZLUSR", subHomePath( LUSR, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZTUSR", subHomePath( TUSR, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZTABU", subHomePath( TABU, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZFILU", subHomePath( TABU, check ), PROFILE ) ;

	p_poolMGR->put( err, "ZMAINPGM", ZMAINPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZMAINPAN", ZMAINPAN, PROFILE ) ;
	p_poolMGR->put( err, "ZPANLPGM", ZPANLPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZEDITPGM", ZEDITPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZBRPGM",   ZBRPGM,   PROFILE ) ;
	p_poolMGR->put( err, "ZVIEWPGM", ZVIEWPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZFLSTPGM", ZFLSTPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZHELPPGM", ZHELPPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZOREXPGM", ZOREXPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZSHELPGM", ZSHELPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZFHSTPGM", ZFHSTPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZSPOOL",   subHomePath( ZSPOOL, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZFLDHIST", subHomePath( FLDHIST, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZMAXSCRN", d2ds(ZMAXSCRN), PROFILE ) ;
	p_poolMGR->put( err, "ZEDLCTAB", EDLCTAB, PROFILE ) ;

	p_poolMGR->put( err, "ZRFLPGM", ZRFLPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZRFLTBL", ZRFLTBL, PROFILE ) ;
	p_poolMGR->put( err, "ZRFURL", "YES", PROFILE ) ;
	p_poolMGR->put( err, "ZRFFEX", "YES", PROFILE ) ;
	p_poolMGR->put( err, "ZRFNEX", "YES", PROFILE ) ;
	p_poolMGR->put( err, "ZRFMOD", "BRT", PROFILE ) ;

	p_poolMGR->put( err, "ZFHURL", "Y", PROFILE ) ;

	p_poolMGR->put( err, "ZGCLB", "B", PROFILE ) ;
	p_poolMGR->put( err, "ZGCLR", "R", PROFILE ) ;
	p_poolMGR->put( err, "ZGCLM", "M", PROFILE ) ;
	p_poolMGR->put( err, "ZGCLG", "G", PROFILE ) ;
	p_poolMGR->put( err, "ZGCLT", "T", PROFILE ) ;
	p_poolMGR->put( err, "ZGCLY", "Y", PROFILE ) ;
	p_poolMGR->put( err, "ZGCLW", "W", PROFILE ) ;

	p_poolMGR->put( err, "ZCTRLA", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLB", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLC", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLD", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLE", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLF", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLG", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLH", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLI", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLJ", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLK", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLL", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLM", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLN", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLO", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLP", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLQ", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLR", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLS", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLT", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLU", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLV", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLW", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLX", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLY", "", PROFILE ) ;
	p_poolMGR->put( err, "ZCTRLZ", "", PROFILE ) ;

	setCUAcolours( "AB",   KAB   ) ;
	setCUAcolours( "ABSL", KABSL ) ;
	setCUAcolours( "ABU",  KABU  ) ;

	setCUAcolours( "AMT",  KAMT ) ;
	setCUAcolours( "AWF",  KAWF ) ;

	setCUAcolours( "CT",   KCT  ) ;
	setCUAcolours( "CEF",  KCEF ) ;
	setCUAcolours( "CH",   KCH  ) ;

	setCUAcolours( "DT",   KDT ) ;
	setCUAcolours( "ET",   KET ) ;
	setCUAcolours( "EE",   KEE ) ;
	setCUAcolours( "FP",   KFP ) ;
	setCUAcolours( "FK",   KFK ) ;

	setCUAcolours( "IWF",  KIWF ) ;

	setCUAcolours( "IMT",  KIMT ) ;
	setCUAcolours( "LEF",  KLEF ) ;
	setCUAcolours( "LID",  KLID ) ;
	setCUAcolours( "LI",   KLI  ) ;

	setCUAcolours( "NEF",  KNEF ) ;
	setCUAcolours( "NT",   KNT  ) ;

	setCUAcolours( "PI",   KPI  ) ;
	setCUAcolours( "PIN",  KPIN ) ;
	setCUAcolours( "PT",   KPT  ) ;

	setCUAcolours( "PS",   KPS  ) ;
	setCUAcolours( "PAC",  KPAC ) ;
	setCUAcolours( "PUC",  KPUC ) ;

	setCUAcolours( "RP",   KRP ) ;

	setCUAcolours( "SI",   KSI  ) ;
	setCUAcolours( "SAC",  KSAC ) ;
	setCUAcolours( "SUC",  KSUC ) ;

	setCUAcolours( "VOI",  KVOI  ) ;

	setCUAcolours( "WMT",  KWMT  ) ;
	setCUAcolours( "WT",   KWT   ) ;
	setCUAcolours( "WASL", KWASL ) ;

	p_poolMGR->statistics() ;
	p_poolMGR->snap() ;
	p_poolMGR->disconnect( 1 ) ;
	if ( err.RC0() )
	{
		cout << "SYSTEM profile ISPSPROF created successfully in " << zuprof << endl ;
	}
	else
	{
		cout << endl ;
		cout << "ERROR saving profile ISPSPROF in "<< zuprof <<"  RC="<< err.getRC() << endl ;
		cout << "Message is " << err.msgid << endl ;
	}

	cout << "*******************************************************************************************" << endl ;
	cout << endl ;
}

void setCUAcolours( const string& var,
		    const string& val )
{
	string var1 ;

	errblock err( 1 ) ;

	var1 = "ZC" + var ;

	if ( val[0] != 'R' &&
	     val[0] != 'G' &&
	     val[0] != 'Y' &&
	     val[0] != 'B' &&
	     val[0] != 'M' &&
	     val[0] != 'T' &&
	     val[0] != 'W' )
	{
		cout << endl ;
		cout << "ERROR: Invalid colour value of " << val[0] << " in setting " << var << endl ;
	}
	if ( val[1] != 'L' &&
	     val[1] != 'H' )
	{
		cout << endl ;
		cout << "ERROR: Invalid colour intensity of " << val[1] << " in setting " << var << endl ;
	}
	if ( val[2] != 'N' &&
	     val[2] != 'B' &&
	     val[2] != 'R' &&
	     val[2] != 'U')
	{
		cout << endl ;
		cout << "ERROR: Invalid colour hilight of " << val[2] << " in setting " << var << endl ;
	}

	p_poolMGR->put( err, var1, val, PROFILE ) ;
}


string subHomePath( string var,
		    bool do_check )
{
	int i ;
	int j ;

	size_t p ;

	string pathname ;

	string homePath = getHOME() ;

	p = var.find( '~' ) ;
	while ( p != string::npos )
	{
		var.replace( p, 1, homePath ) ;
		p = var.find( '~' ) ;
	}

	if ( do_check )
	{
		for ( j = getpaths( var ), i = 1 ; i <= j ; ++i )
		{
			pathname = getpath( var, i ) ;
			if ( !exists( pathname ) || !is_directory( pathname ) )
			{
				cout << endl ;
				cout << "WARNING: Path " << pathname << " does not exist or is not a directory.  lspf may not start." << endl ;
			}
		}
	}

	return var ;
}


string getHOME()
{
	char* t = getenv( "HOME" ) ;

	if ( !t )
	{
		cout << "Environment variable HOME is not set.  This is a required variable"<< endl ;
		abort() ;
	}

	string home = string( t ) ;
	if ( home == "" )
	{
		cout << "Environment variable HOME is set to NULL.  This is a required variable"<< endl ;
		abort() ;
	}
	return home ;
}


string getEnvironmentVariable( const char* var )
{
	char* t = getenv( var ) ;

	return  t ?: "" ;
}
