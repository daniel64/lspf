/* Compile with ::                                                                         */
/* g++ -rdynamic -std=c++11 -lboost_filesystem -lboost_system -lpthread -o setup setup.cpp */

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

/* Amend and run to create initial ISPCMDS and USRCMDS command tables           */
/* and ISPSPROF default profile.  Values taken from lspf.h                      */
/*                                                                              */
/* *NOTE* Any changes to ISPSPROF done online, will be lost                     */
/*                                                                              */
/* USRCMDS is the default user command table 1 (ZUCMDT1)                        */
/* This can be changed in option 0.0, General lspf Settings                     */
/*                                                                              */
/* Create function pool, pool manager and table manager                         */

#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <time.h>
#include <vector>
#include <dlfcn.h>
#include <sys/utsname.h>

#include <locale>

#include <boost/filesystem.hpp>

#include "lspf.h"

#include "utilities.h"
#include "utilities.cpp"

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

fPOOL funcPOOL ;

logger* lg = new logger ;

logger* tableMGR::lg = NULL ;
logger* poolMGR::lg  = NULL ;

void   createSYSPROF() ;
void   setCUAcolours( const string&, const string& ) ;
string subHomePath( string, bool =false ) ;

int main()
{
	errblock err ;

	tableMGR::lg = lg ;
	poolMGR::lg  = lg ;

	string homePath  ;

	lg->open( subHomePath( ALOG ) ) ;

	homePath  = getenv( "HOME" ) ;
	homePath += ZUPROF ;
	if ( homePath.back() != '/' ) { homePath += "/" ; }

	string ZCTVERB, ZCTACT, ZCTDESC, ZCTTRUNC ;

	funcPOOL.define( err, "ZCTVERB",  &ZCTVERB  ) ;
	funcPOOL.define( err, "ZCTTRUNC", &ZCTTRUNC ) ;
	funcPOOL.define( err, "ZCTACT",   &ZCTACT   ) ;
	funcPOOL.define( err, "ZCTDESC",  &ZCTDESC  ) ;

	p_tableMGR->createTable( err, "ISPCMDS", "ZCTVERB" , "ZCTTRUNC ZCTACT ZCTDESC", NOREPLACE, WRITE, "", SHARE ) ;
	p_tableMGR->createTable( err, "USRCMDS", "ZCTVERB" , "ZCTTRUNC ZCTACT ZCTDESC", NOREPLACE, WRITE, "", SHARE ) ;

	ZCTVERB  = "SCRNAME" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "SCRNAME" ;
	ZCTDESC  = "Set screen name for session" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "ACTIONS" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "ACTIONS" ;
	ZCTDESC  = "Switch to first or next action bar choice" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "SWAP" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "SWAP" ;
	ZCTDESC  = "Swap logical screens" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "SPLIT" ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SPLIT" ;
	ZCTDESC  = "Split screen" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RETRIEVE" ;
	ZCTTRUNC = "0"        ;
	ZCTACT   = "RETRIEVE" ;
	ZCTDESC  = "Retrieve command" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RETF" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "RETF" ;
	ZCTDESC  = "Forward retrieve command" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RETURN"  ;
	ZCTTRUNC = "0"       ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RFIND" ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RCHANGE" ;
	ZCTTRUNC = "0"       ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "END"    ;
	ZCTTRUNC = "0"      ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "HELP" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "SELECT PGM(&ZHELPPGM) PARM(&ZPARM) SUSPEND"  ;
	ZCTDESC  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "UP"    ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "DOWN"  ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "LEFT"  ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "TOP"   ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "ALIAS UP MAX" ;
	ZCTDESC  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "BOTTOM" ;
	ZCTTRUNC = "3"      ;
	ZCTACT   = "ALIAS DOWN MAX" ;
	ZCTDESC  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RIGHT" ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "LEFT"  ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "ALOG" ;
	ZCTTRUNC = "3"  ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(AL) SCRNAME(LOG) SUSPEND" ;
	ZCTDESC  = "Browse application log" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "LOG" ;
	ZCTTRUNC = "3"   ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(SL) SCRNAME(LOG) SUSPEND" ;
	ZCTDESC  = "Browse lspf log" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "KEYLIST"  ;
	ZCTTRUNC = "0"        ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(KLIST) SCRNAME(KEYLIST) SUSPEND" ;
	ZCTDESC  = "Keylist utility"  ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "KEYLISTS" ;
	ZCTTRUNC = "0"        ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(KLISTS) SCRNAME(KEYLIST) SUSPEND" ;
	ZCTDESC  = "Keylist utility"  ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "DSLIST" ;
	ZCTTRUNC = "0"      ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(DSL &ZPARM) NEWAPPL(ISR) SCRNAME(DSLIST) SUSPEND" ;
	ZCTDESC  = "File list utility"  ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "KEYS" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(KEYS) NEWPOOL SCRNAME(KEYS) SUSPEND" ;
	ZCTDESC  = "PFkey utility" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "CTRL" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(CTLKEYS) NEWPOOL SCRNAME(CTLKEYS) SUSPEND" ;
	ZCTDESC  = "Control key utility" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "ISPLIBD" ;
	ZCTTRUNC = "0"        ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(LIBDEFS) NEWPOOL SCRNAME(LIBDUTL) SUSPEND" ;
	ZCTDESC  = "Display LIBDEF status" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RECENT" ;
	ZCTTRUNC = "0"      ;
	ZCTACT   = "ALIAS DSLIST REFLIST" ;
	ZCTDESC  = "Recent file list utility"  ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RUN" ;
	ZCTTRUNC = "2"    ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(RUN &ZPARM) NEWPOOL SUSPEND" ;
	ZCTDESC  = "Run an application by name" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RETP" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "RETP" ;
	ZCTDESC  = "List retrieve buffer" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "SETTINGS"  ;
	ZCTTRUNC = "3"         ;
	ZCTACT   = "SELECT PANEL(PPSET0A) SCRNAME(SETTINGS) SUSPEND" ;
	ZCTDESC  = "Settings selection menu"  ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 )  ;

	ZCTVERB  = "START"    ;
	ZCTTRUNC = "0"        ;
	ZCTACT   = "SELECT PGM(ISPSTRT) PARM(&ZPARM)" ;
	ZCTDESC  = "Start a program in a new logical screen" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "REFACTF" ;
	ZCTTRUNC = "5"       ;
	ZCTACT   = "SELECT PGM(&ZRFLPGM) PARM(PL1 &ZPARM) SCRNAME(REFACT) SUSPEND" ;
	ZCTDESC  = "Open active referral list" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "REFADDF" ;
	ZCTTRUNC = "5"       ;
	ZCTACT   = "SELECT PGM(&ZRFLPGM) PARM(PLA &ZPARM) SUSPEND" ;
	ZCTDESC  = "Add file to reference list" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "REFLISTF" ;
	ZCTTRUNC = "4"        ;
	ZCTACT   = "SELECT PGM(&ZRFLPGM) PARM(PL1 REFLIST &ZPARM) SCRNAME(REFLIST) SUSPEND" ;
	ZCTDESC  = "Open reference list" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "REFOPENF" ;
	ZCTTRUNC = "4"        ;
	ZCTACT   = "SELECT PGM(&ZRFLPGM) PARM(PL2) SCRNAME(REFOPEN) SUSPEND"  ;
	ZCTDESC  = "Open file referral lists" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "NRETRIEV" ;
	ZCTTRUNC = "0"        ;
	ZCTACT   = "SETVERB"  ;
	ZCTDESC  = "Retrieve next entry from active referral list" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	// ========================= USRCMDS ======================================
	ZCTVERB  = "ED" ;
	ZCTTRUNC = "0" ;
	ZCTACT   = "SELECT PGM(&ZEDITPGM) NEWAPPL(ISR) PARM(FILE(&ZPARM)) SCRNAME(EDIT) SUSPEND" ;
	ZCTDESC  = "Invoke EDIT ENTRY panel" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "BR" ;
	ZCTTRUNC = "0" ;
	ZCTACT   = "SELECT PGM(&ZFLSTPGM) NEWAPPL(ISRB) PARM(BROWSE &ZPARM) SCRNAME(FILES) SUSPEND" ;
	ZCTDESC  = "Invoke browse" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "CMDS" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(CMDS) SCRNAME(COMMANDS) SUSPEND"  ;
	ZCTDESC  = "Display command tables" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "TASKS" ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(TASKS) SCRNAME(TASKS) SUSPEND"  ;
	ZCTDESC  = "Show task list" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "FLIST" ;
	ZCTTRUNC = "2"     ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(SAVELST) NEWAPPL SCRNAME(SAVED) SUSPEND"  ;
	ZCTDESC  = "Show saved file list" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "MODS" ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(MODS) SCRNAME(MODULES) SUSPEND"  ;
	ZCTDESC  = "Show loaded dynamic classes" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "PATHS" ;
	ZCTTRUNC = "3"     ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(PATHS) SCRNAME(PATHS) SUSPEND"  ;
	ZCTDESC  = "Show search paths for panels, messages and tables" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "EXEC" ;
	ZCTTRUNC = "3"     ;
	ZCTACT   = "SELECT CMD(&ZPARM) LANG(REXX) NEWPOOL SUSPEND"    ;
	ZCTDESC  = "Run OOREXX EXEC" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "TODO" ;
	ZCTTRUNC = "0"   ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(TODO) NEWAPPL(ISP) SCRNAME(TODO) SUSPEND" ;
	ZCTDESC  = "Browse todo list" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "VARS" ;
	ZCTTRUNC = "3"    ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(VARS &ZPARM) SCRNAME(VARS) SUSPEND"  ;
	ZCTDESC  = "Display shared and profile variables" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "CUA" ;
	ZCTTRUNC = "0"   ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(COLOURS) SCRNAME(CUA) SUSPEND"  ;
	ZCTDESC  = "Display/change CUA colours" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	p_tableMGR->tbsort( err, "ISPCMDS", "ZCTVERB,C,A" ) ;
	p_tableMGR->tbsort( err, "USRCMDS", "ZCTVERB,C,A" ) ;

	cout << endl ;
	cout << "*******************************************************************************************" << endl ;

	p_tableMGR->saveTable( err, "ISPCMDS", "" , homePath  ) ;
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

	p_tableMGR->saveTable( err, "USRCMDS", "" , homePath ) ;
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
	lg->close() ;

	delete p_poolMGR  ;
	delete p_tableMGR ;
	delete lg         ;
}


void createSYSPROF()
{
	string p ;
	string zuprof ;

	errblock err ;
	err.settask( 1 ) ;

	zuprof  = getenv( "HOME" ) ;
	zuprof += ZUPROF ;
	p_poolMGR->setProfilePath( err, zuprof ) ;

	p_poolMGR->createSharedPool() ;
	p_poolMGR->createProfilePool( err, "ISPS" ) ;
	p_poolMGR->connect( 1, "ISPS", 1 ) ;

	p_poolMGR->put( err, "ZUPROF", zuprof, PROFILE ) ;
	p_poolMGR->put( err, "ZSYSPATH", ZSYSPATH, PROFILE ) ;
	p_poolMGR->put( err, "ZORXPATH", subHomePath( ZREXPATH, check ), PROFILE ) ;

	p_poolMGR->put( err, "ZLDPATH", ZLDPATH, PROFILE ) ;

	p_poolMGR->put( err, "ZSLOG", subHomePath( SLOG ), PROFILE ) ;
	p_poolMGR->put( err, "ZALOG", subHomePath( ALOG ), PROFILE ) ;

	p_poolMGR->put( err, "ZDEFM",    "N",    PROFILE ) ;
	p_poolMGR->put( err, "ZDEL",     ";",    PROFILE ) ;
	p_poolMGR->put( err, "ZKLUSE",   "N",    PROFILE ) ;
	p_poolMGR->put( err, "ZKLPRIV",  "Y",    PROFILE ) ;
	p_poolMGR->put( err, "ZKLFAIL",  "Y",    PROFILE ) ;
	p_poolMGR->put( err, "ZHIGH",    "Y",    PROFILE ) ;
	p_poolMGR->put( err, "ZLMSGW",   "N",    PROFILE ) ;
	p_poolMGR->put( err, "ZPADC",    "_",    PROFILE ) ;
	p_poolMGR->put( err, "ZRTSIZE",  "3",    PROFILE ) ;
	p_poolMGR->put( err, "ZRBSIZE",  "20",   PROFILE ) ;
	p_poolMGR->put( err, "ZSWAP",    "Y",    PROFILE ) ;
	p_poolMGR->put( err, "ZSWAPC",   "'",    PROFILE ) ;
	p_poolMGR->put( err, "ZSCROLLD", "HALF", PROFILE ) ;
	p_poolMGR->put( err, "ZSRETP",   "Y",    PROFILE ) ;

	p_poolMGR->put( err, "ZUCMDT1", "USR", PROFILE ) ;
	p_poolMGR->put( err, "ZUCMDT2", "", PROFILE ) ;
	p_poolMGR->put( err, "ZUCMDT3", "", PROFILE ) ;

	p_poolMGR->put( err, "ZSCMDT1", "", PROFILE ) ;
	p_poolMGR->put( err, "ZSCMDT2", "", PROFILE ) ;
	p_poolMGR->put( err, "ZSCMDT3", "", PROFILE ) ;

	p_poolMGR->put( err, "ZSCMDTF", "Y", PROFILE ) ;

	p_poolMGR->put( err, "ZMLIB", subHomePath( MLIB, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZPLIB", subHomePath( PLIB, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZTLIB", subHomePath( TLIB, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZTABL", subHomePath( TABL, check ), PROFILE ) ;

	p_poolMGR->put( err, "ZMUSR", subHomePath( MUSR, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZPUSR", subHomePath( PUSR, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZTUSR", subHomePath( TUSR, check ), PROFILE ) ;
	p_poolMGR->put( err, "ZTABU", subHomePath( TABU, check ), PROFILE ) ;

	p_poolMGR->put( err, "ZMAINPGM", ZMAINPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZMAINPAN", ZMAINPAN, PROFILE ) ;
	p_poolMGR->put( err, "ZPANLPGM", ZPANLPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZEDITPGM", ZEDITPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZBRPGM",   ZBRPGM,   PROFILE ) ;
	p_poolMGR->put( err, "ZVIEWPGM", ZVIEWPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZFLSTPGM", ZFLSTPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZHELPPGM", ZHELPPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZOREXPGM", ZOREXPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZSHELP",   ZSHELP,   PROFILE ) ;
	p_poolMGR->put( err, "ZMAXSCRN", d2ds(ZMAXSCRN), PROFILE ) ;
	p_poolMGR->put( err, "ZMAXWAIT", d2ds(ZMAXWAIT), PROFILE ) ;
	p_poolMGR->put( err, "ZWAIT",    d2ds(ZWAIT),    PROFILE ) ;

	p_poolMGR->put( err, "ZRFLPGM", ZRFLPGM, PROFILE ) ;
	p_poolMGR->put( err, "ZRFLTBL", ZRFLTBL, PROFILE ) ;
	p_poolMGR->put( err, "ZRFURL", "YES", PROFILE ) ;
	p_poolMGR->put( err, "ZRFFEX", "YES", PROFILE ) ;
	p_poolMGR->put( err, "ZRFNEX", "YES", PROFILE ) ;
	p_poolMGR->put( err, "ZRFMOD", "BRT", PROFILE ) ;

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

void setCUAcolours( const string& var, const string& val )
{
	string var1 ;

	errblock err ;
	err.settask( 1 ) ;

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


string subHomePath( string var, bool do_check )
{
	int    i ;
	int    j ;
	size_t p ;

	string pathname ;

	string homePath = getenv( "HOME" ) ;

	p = var.find( '~' ) ;
	while ( p != string::npos )
	{
		var.replace( p, 1, homePath ) ;
		p = var.find( '~' ) ;
	}

	if ( do_check )
	{
		for ( j = getpaths( var ), i = 1 ; i <= j ; i++ )
		{
			pathname = getpath( var, i ) ;
			if ( !exists( pathname ) )
			{
				cout << endl ;
				cout << "WARNING: Path " << pathname << " does not exist" << endl ;
			}
		}
	}

	return var ;
}

