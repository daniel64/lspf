/*  Compile with ::                                                              */
/* g++ -rdynamic -std=c++11 -lboost_filesystem -lboost_system -o setup setup.cpp */

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
/* *NOTE* Any changes to ISPSPROF done online, will be lost                     */

/* USRCMDS is the default user command table 1 (ZUCMDT1)                        */
/* This can be changed in option 0.0, General lspf Settings                     */

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

#include "pWidgets.h"

#include "pVPOOL.h"
#include "pVPOOL.cpp"

#include "pTable.h"
#include "pTable.cpp"


using namespace std ;
using namespace boost::filesystem ;

poolMGR  * p_poolMGR  = new poolMGR  ;
tableMGR * p_tableMGR = new tableMGR ;

fPOOL funcPOOL ;

void createSYSPROF() ;
void setCUAcolours( const string&, const string& ) ;

main()
{
	errblock err ;

	string systemPath, homePath ;

	systemPath = ZSPROF ;
	if ( systemPath.back() != '/' ) { systemPath = systemPath + '/' ; }

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

	ZCTVERB  = "SWAP" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "SWAP" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "SPLIT" ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SPLIT"  ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RETRIEVE" ;
	ZCTTRUNC = "0"        ;
	ZCTACT   = "RETRIEVE"  ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RETURN"  ;
	ZCTTRUNC = "0"       ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RFIND" ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RCHANGE" ;
	ZCTTRUNC = "0"       ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "END"    ;
	ZCTTRUNC = "0"      ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "HELP" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "SELECT PGM(&ZHELPPGM) PARM(&ZPARM) SUSPEND"  ;
	ZCTDESC  = "INVOKE HELP SYSTEM" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "UP"    ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "DOWN"  ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "LEFT"  ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "TOP"   ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "ALIAS UP MAX" ;
	ZCTDESC  = "ALIAS" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "BOTTOM" ;
	ZCTTRUNC = "3"      ;
	ZCTACT   = "ALIAS DOWN MAX" ;
	ZCTDESC  = "ALIAS" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RIGHT" ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "LEFT"  ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "ALOG" ;
	ZCTTRUNC = "3"  ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(AL) SCRNAME(LOG) SUSPEND" ;
	ZCTDESC  = "BROWSE APPLICATION LOG" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "LOG" ;
	ZCTTRUNC = "3"   ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(SL) SCRNAME(LOG) SUSPEND" ;
	ZCTDESC  = "BROWSE LSPF LOG" ;
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

	ZCTVERB  = "KEYS" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(KEYS) NEWPOOL SCRNAME(KEYS) SUSPEND" ;
	ZCTDESC  = "PFKEY UTILITY" ;
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
	ZCTDESC  = "SETTINGS SELECTION PANEL UTILITY"  ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 )  ;

	ZCTVERB  = "START"    ;
	ZCTTRUNC = "0"        ;
	ZCTACT   = "SELECT PGM(ISPSTRT) PARM(&ZPARM)" ;
	ZCTDESC  = "Start a program in a new logical screen" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "REFACTF" ;
	ZCTTRUNC = "5"       ;
	ZCTACT   = "SELECT PGM(&ZRFLPGM) PARM(PL1 &ZPARM) SCRNAME(REFACT) SUSPEND" ;
	ZCTDESC  = "OPEN ACTIVE REFERRAL LIST" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "REFADDF" ;
	ZCTTRUNC = "5"       ;
	ZCTACT   = "SELECT PGM(&ZRFLPGM) PARM(PLA &ZPARM) SUSPEND" ;
	ZCTDESC  = "ADD FILE TO REFERENCE LIST" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "REFLISTF" ;
	ZCTTRUNC = "4"        ;
	ZCTACT   = "SELECT PGM(&ZRFLPGM) PARM(PL1 REFLIST &ZPARM) SCRNAME(REFLIST) SUSPEND" ;
	ZCTDESC  = "OPEN REFERENCE LIST" ;
	p_tableMGR->tbadd( err, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "REFOPENF" ;
	ZCTTRUNC = "4"        ;
	ZCTACT   = "SELECT PGM(&ZRFLPGM) PARM(PL2) SCRNAME(REFOPEN) SUSPEND"  ;
	ZCTDESC  = "OPEN FILE REFERRAL LISTS" ;
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
	ZCTDESC  = "INVOKE EDIT ENTRY PANEL" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "BR" ;
	ZCTTRUNC = "0" ;
	ZCTACT   = "SELECT PGM(&ZFLSTPGM) NEWAPPL(ISRB) PARM(BROWSE &ZPARM) SCRNAME(FILES) SUSPEND" ;
	ZCTDESC  = "INVOKE BROWSE" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "CMDS" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(CMDS) SCRNAME(COMMANDS) SUSPEND"  ;
	ZCTDESC  = "SHOW COMMAND TABLES" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "TASKS" ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(TASKS) SCRNAME(TASKS) SUSPEND"  ;
	ZCTDESC  = "SHOW TASK LIST" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "FLIST" ;
	ZCTTRUNC = "2"     ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(SAVELST) NEWAPPL SCRNAME(SAVED) SUSPEND"  ;
	ZCTDESC  = "SHOW SAVED FILE LIST" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "MODS" ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(MODS) SCRNAME(MODULES) SUSPEND"  ;
	ZCTDESC  = "SHOW LOADED DYNAMIC CLASSES" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "PATHS" ;
	ZCTTRUNC = "3"     ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(PATHS) SCRNAME(PATHS) SUSPEND"  ;
	ZCTDESC  = "SHOW PATHS SEARCHED FOR PANELS, MESSAGES AND TABLES" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "EXEC" ;
	ZCTTRUNC = "3"     ;
	ZCTACT   = "SELECT PGM(&ZOREXPGM) PARM(&ZPARM) NEWPOOL SUSPEND"    ;
	ZCTDESC  = "INVOKE OOREXX EXEC" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "TODO" ;
	ZCTTRUNC = "0"   ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(TODO) NEWAPPL(ISP) SCRNAME(TODO) SUSPEND" ;
	ZCTDESC  = "BROWSE TODO LIST" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "VARS" ;
	ZCTTRUNC = "3"    ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(VARS &ZPARM) SCRNAME(VARS) SUSPEND"  ;
	ZCTDESC  = "DISPLAY SHARED & PROFILE VARIABLES" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "CUA" ;
	ZCTTRUNC = "0"   ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(COLOURS) NEWAPPL(ISP) SCRNAME(CUA) SUSPEND"  ;
	ZCTDESC  = "DISPLAY/CHANGE CUA COLOURS" ;
	p_tableMGR->tbadd( err, funcPOOL, "USRCMDS", "", "", 0 ) ;

	p_tableMGR->tbsort( err, "ISPCMDS", "ZCTVERB,C,A" ) ;
	p_tableMGR->tbsort( err, "USRCMDS", "ZCTVERB,C,A" ) ;

	cout << endl ;
	cout << "*******************************************************************************************" << endl ;
	p_tableMGR->saveTable( err, "ISPCMDS", "" , systemPath + "tlib" ) ;
	if ( err.RC0() )
	{
		cout << "ISPCMDS table created successfully in " << systemPath << "tlib" << endl ;
	}
	else
	{
		cout << "ERROR saving ISPCMDS table in " << systemPath << "tlib  RC=" << err.getRC() << endl ;
		cout << "Message is " << err.msgid << endl ;
	}
	p_tableMGR->saveTable( err, "USRCMDS", "" , systemPath + "tlib" ) ;
	if ( err.RC0() )
	{
		cout << "USRCMDS table created successfully in " << systemPath << "tlib" << endl ;
	}
	else
	{
		cout << "ERROR saving USRCMDS table in " << systemPath << "tlib  RC=" << err.getRC() << endl ;
		cout << "Message is " << err.msgid << endl ;
	}

	createSYSPROF() ;

	cout << "See application log in ZALOG if any errors have been encountered" << endl ;
}


void createSYSPROF()
{
	string p ;

	errblock err ;

	p_poolMGR->setAPPLID( err, "ISPS" )  ;
	p_poolMGR->createPool( err, SHARED ) ;
	p_poolMGR->createPool( err, PROFILE, ZSPROF ) ;

	p_poolMGR->put( err, "ZSPROF", ZSPROF, PROFILE ) ;
	p_poolMGR->put( err, "ZUPROF", ZUPROF, PROFILE ) ;
	p_poolMGR->put( err, "ZSYSPATH", ZSYSPATH, PROFILE ) ;
	p_poolMGR->put( err, "ZORXPATH", ZREXPATH, PROFILE ) ;

	p_poolMGR->put( err, "ZLDPATH", ZLDPATH, PROFILE ) ;

	p_poolMGR->put( err, "ZSLOG", SLOG, PROFILE ) ;
	p_poolMGR->put( err, "ZALOG", ALOG, PROFILE ) ;

	p_poolMGR->put( err, "ZPADC",   "_", PROFILE ) ;
	p_poolMGR->put( err, "ZDEL",    ";", PROFILE ) ;
	p_poolMGR->put( err, "ZSWAP",   "Y", PROFILE ) ;
	p_poolMGR->put( err, "ZSWAPC",  "'", PROFILE ) ;
	p_poolMGR->put( err, "ZKLUSE",  "N", PROFILE ) ;
	p_poolMGR->put( err, "ZKLPRIV", "Y", PROFILE ) ;
	p_poolMGR->put( err, "ZKLFAIL", "Y", PROFILE ) ;
	p_poolMGR->put( err, "ZRTSIZE", "3", PROFILE ) ;
	p_poolMGR->put( err, "ZRBSIZE", "10", PROFILE ) ;
	p_poolMGR->put( err, "ZLMSGW",  "N", PROFILE ) ;

	p_poolMGR->put( err, "ZUCMDT1", "USR", PROFILE ) ;
	p_poolMGR->put( err, "ZUCMDT2", "", PROFILE ) ;
	p_poolMGR->put( err, "ZUCMDT3", "", PROFILE ) ;

	p_poolMGR->put( err, "ZSCMDT1", "", PROFILE ) ;
	p_poolMGR->put( err, "ZSCMDT2", "", PROFILE ) ;
	p_poolMGR->put( err, "ZSCMDT3", "", PROFILE ) ;

	p_poolMGR->put( err, "ZSCMDTF", "Y", PROFILE ) ;

	p_poolMGR->put( err, "ZMLIB", MLIB, PROFILE ) ;
	p_poolMGR->put( err, "ZPLIB", PLIB, PROFILE ) ;
	p_poolMGR->put( err, "ZTLIB", TLIB, PROFILE ) ;

	p_poolMGR->put( err, "ZMAINPGM", ZMAINPGM, PROFILE ) ;
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
	p_poolMGR->put( err, "ZRFMOD", "BEX", PROFILE ) ;

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

	p_poolMGR->destroyPool( err, PROFILE  ) ;
	if ( err.RC0() )
	{
		cout << "SYSTEM profile ISPSPROF created successfully in " << ZSPROF << endl ;
	}
	else
	{
		cout << "ERROR saving profile ISPSPROF in "<< ZSPROF <<"  RC="<< err.getRC() << endl ;
		cout << "Message is " << err.msgid << endl ;
	}
	cout << "*******************************************************************************************" << endl ;
	cout << endl ;
}

void setCUAcolours( const string& var, const string& val )
{
	string var1 ;

	errblock err ;

	var1 = "ZC" + var ;

	if ( val[0] != 'R' &&
	     val[0] != 'G' &&
	     val[0] != 'Y' &&
	     val[0] != 'B' &&
	     val[0] != 'M' &&
	     val[0] != 'T' &&
	     val[0] != 'W' )
	{
		cout << "ERROR:: Invalid colour value of " << val[0] << " in setting " << var << endl ;
	}
	if ( val[1] != 'L' &&
	     val[1] != 'H' )
	{
		cout << "ERROR:: Invalid colour intensity of " << val[1] << " in setting " << var << endl ;
	}
	if ( val[2] != 'N' &&
	     val[2] != 'B' &&
	     val[2] != 'R' &&
	     val[2] != 'U')
	{
		cout << "ERROR:: Invalid colour hilight of " << val[2] << " in setting " << var << endl ;
	}

	p_poolMGR->put( err, var1, val, PROFILE )   ;
}

