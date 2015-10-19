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

/* Amend and run to create initial ISPCMDS and USERCMDS command tables           */
/* and ISPSPROF default profile.  Values taken from lspf.h                       */
/* *NOTE* Any changes to ISPSPROF done online, will be lost                      */

/* Create function pool, pool manager and table manager                          */

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

#include "pWidgets.h"

#include "pVPOOL.h"
#include "pVPOOL.cpp"

#include "pTable.h"
#include "pTable.cpp"


using namespace std ;
using namespace boost::filesystem ;

poolMGR    * p_poolMGR    = new poolMGR    ;
tableMGR   * p_tableMGR   = new tableMGR   ;

fPOOL  funcPOOL ;

void createSYSPROF() ;
void setCUAcolours( string, string ) ;

main()
{
	int RC ;

	string systemPath, homePath ;

	systemPath = ZSPROF ;
	if ( systemPath.back() != '/' ) { systemPath = systemPath + '/' ; }

	string ZCTVERB, ZCTACT, ZCTDESC, ZCTTRUNC ;

	funcPOOL.define( RC, "ZCTVERB",  &ZCTVERB  ) ;
	funcPOOL.define( RC, "ZCTTRUNC", &ZCTTRUNC ) ;
	funcPOOL.define( RC, "ZCTACT",   &ZCTACT   ) ;
	funcPOOL.define( RC, "ZCTDESC",  &ZCTDESC  ) ;

	p_tableMGR->createTable( RC, 0, "ISPCMDS", "ZCTVERB" , "ZCTTRUNC ZCTACT ZCTDESC", WRITE, NOREPLACE, "", SHARE ) ;
	p_tableMGR->createTable( RC, 0, "USRCMDS", "ZCTVERB" , "ZCTTRUNC ZCTACT ZCTDESC", WRITE, NOREPLACE, "", SHARE ) ;

	ZCTVERB  = "SNAP" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "SNAP" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "SWAP" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "SWAP" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "STATS" ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "STATS" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "SPLIT" ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SPLIT"  ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RETRIEVE" ;
	ZCTTRUNC = "0"        ;
	ZCTACT   = "RETRIEVE"  ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RETURN"  ;
	ZCTTRUNC = "0"       ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RFIND" ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "END"    ;
	ZCTTRUNC = "0"      ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "HELP" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "SELECT PGM(" ZHELPPGM ") PARM(&ZPARM)"  ;
	ZCTDESC  = "INVOKE HELP SYSTEM" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "UP"    ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "DOWN"  ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "LEFT"  ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "TOP"   ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "ALIAS UP MAX" ;
	ZCTDESC  = "ALIAS" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "BOTTOM" ;
	ZCTTRUNC = "3"      ;
	ZCTACT   = "ALIAS DOWN MAX" ;
	ZCTDESC  = "ALIAS" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "RIGHT" ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "LEFT"  ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SETVERB" ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "ALOG" ;
	ZCTTRUNC = "3"  ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(AL)" ;
	ZCTDESC  = "BROWSE APPLICATION LOG" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "LOG" ;
	ZCTTRUNC = "3"   ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(SL)" ;
	ZCTDESC  = "BROWSE LSPF LOG" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "KEYLIST"  ;
	ZCTTRUNC = "0"        ;
	ZCTACT   = "KEYLIST"  ;
	ZCTDESC  = "BUILTIN ISPCMDS ENTRY"  ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "KEYS" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(KEYS) NEWPOOL" ;
	ZCTDESC  = "PFKEY UTILITY" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	ZCTVERB  = "SETTINGS"  ;
	ZCTTRUNC = "3"         ;
	ZCTACT   = "SELECT PANEL(PPSET0A)"  ;
	ZCTDESC  = "SETTINGS SELECTION PANEL UTILITY"  ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 )  ;

	ZCTVERB  = "START"    ;
	ZCTTRUNC = "0"        ;
	ZCTACT   = "SELECT PGM(" ZMAINPGM ")" ;
	ZCTDESC  = "START A NEW MAIN PROGRAM SESSION" ;
	p_tableMGR->tbadd( RC, funcPOOL, "ISPCMDS", "", "", 0 ) ;

	// ========================= USRCMDS ======================================
	ZCTVERB  = "EDIT" ;
	ZCTTRUNC = "2" ;
	ZCTACT   = "SELECT PGM(&ZFLSTPGM) NEWAPPL(ISR) PARM(EDIT &ZPARM)"   ;
	ZCTDESC  = "INVOKE EDIT" ;
	p_tableMGR->tbadd( RC, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "BROWSE" ;
	ZCTTRUNC = "2" ;
	ZCTACT   = "SELECT PGM(&ZFLSTPGM) NEWAPPL(ISRB) PARM(BROWSE &ZPARM)"   ;
	ZCTDESC  = "INVOKE BROWSE" ;
	p_tableMGR->tbadd( RC, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "CMDS" ;
	ZCTTRUNC = "0"    ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(CMDS)"  ;
	ZCTDESC  = "SHOW COMMAND TABLES" ;
	p_tableMGR->tbadd( RC, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "TASKS" ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(TASKS)"  ;
	ZCTDESC  = "SHOW TASK LIST" ;
	p_tableMGR->tbadd( RC, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "FLIST" ;
	ZCTTRUNC = "2"     ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(SAVELST)"  ;
	ZCTDESC  = "SHOW SAVED FILE LIST" ;
	p_tableMGR->tbadd( RC, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "MODS" ;
	ZCTTRUNC = "0"     ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(MODS)"  ;
	ZCTDESC  = "SHOW LOADED DYNAMIC CLASSES" ;
	p_tableMGR->tbadd( RC, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "PATHS" ;
	ZCTTRUNC = "3"     ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(PATHS)"  ;
	ZCTDESC  = "SHOW PATHS SEARCHED FOR PANELS, MESSAGES AND TABLES" ;
	p_tableMGR->tbadd( RC, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "REXX" ;
	ZCTTRUNC = "3"     ;
	ZCTACT   = "SELECT PGM(&ZOREXPGM) PARM(&ZPARM) NEWPOOL"    ;
	ZCTDESC  = "INVOKE OOREXX EXEC" ;
	p_tableMGR->tbadd( RC, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "TODO" ;
	ZCTTRUNC = "0"   ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(TODO) NEWAPPL(ISP)" ;
	ZCTDESC  = "BROWSE TODO LIST" ;
	p_tableMGR->tbadd( RC, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "VARS" ;
	ZCTTRUNC = "3"    ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(VARS)"  ;
	ZCTDESC  = "DISPLAY SHARED & PROFILE VARIABLES" ;
	p_tableMGR->tbadd( RC, funcPOOL, "USRCMDS", "", "", 0 ) ;

	ZCTVERB  = "CUA" ;
	ZCTTRUNC = "0"   ;
	ZCTACT   = "SELECT PGM(PPSP01A) PARM(COLOURS) NEWAPPL(ISP)"  ;
	ZCTDESC  = "DISPLAY/CHANGE CUA COLOURS" ;

	p_tableMGR->tbadd( RC, funcPOOL, "USRCMDS", "", "", 0 ) ;

	p_tableMGR->tbsort( RC, funcPOOL, "ISPCMDS", "ZCTVERB,C,A" ) ;
	p_tableMGR->tbsort( RC, funcPOOL, "USRCMDS", "ZCTVERB,C,A" ) ;

	cout << endl ;
	cout << "*******************************************************************************************" << endl ;
	p_tableMGR->saveTable( RC, 0, "ISPCMDS", "" , systemPath + "tlib" ) ;
	if ( RC == 0 )
	{
		cout << "ISPCMDS table created successfully in " << systemPath << "tlib" << endl ;
	}
	else
	{
		cout << "ERROR saving ISPCMDS table in " << systemPath << "tlib  RC=" << RC << endl ;
	}
	p_tableMGR->saveTable( RC, 0, "USRCMDS", "" , systemPath + "tlib" ) ;
	if ( RC == 0 )
	{
		cout << "USRCMDS table created successfully in " << systemPath << "tlib" << endl ;
	}
	else
	{
		cout << "ERROR saving USRCMDS table in " << systemPath << "tlib  RC=" << RC << endl ;
	}

	createSYSPROF() ;

	cout << "See application log in ZALOG if any errors have been encountered" << endl ;
}


void createSYSPROF()
{
	int RC ;

	string p ;

        p_poolMGR->setAPPLID( RC, "ISPS" )  ;
        p_poolMGR->createPool( RC, SHARED ) ;
        p_poolMGR->createPool( RC, PROFILE, ZSPROF ) ;

        p_poolMGR->put( RC, "ZSPROF", ZSPROF, PROFILE ) ;
        p_poolMGR->put( RC, "ZUPROF", ZUPROF, PROFILE ) ;
        p_poolMGR->put( RC, "ZSYSPATH", ZSYSPATH, PROFILE ) ;
        p_poolMGR->put( RC, "ZORXPATH", ZREXPATH, PROFILE ) ;

	p_poolMGR->put( RC, "ZLDPATH", ZLDPATH, PROFILE ) ;

	p_poolMGR->put( RC, "ZSLOG", SLOG, PROFILE ) ;
        p_poolMGR->put( RC, "ZALOG", ALOG, PROFILE ) ;

        p_poolMGR->put( RC, "ZPADC", "N", PROFILE ) ;

	p_poolMGR->put( RC, "ZMLIB", MLIB, PROFILE ) ;
	p_poolMGR->put( RC, "ZPLIB", PLIB, PROFILE ) ;
	p_poolMGR->put( RC, "ZTLIB", TLIB, PROFILE ) ;

	p_poolMGR->put( RC, "ZMAINPGM", ZMAINPGM, PROFILE ) ;
	p_poolMGR->put( RC, "ZPANLPGM", ZPANLPGM, PROFILE ) ;
	p_poolMGR->put( RC, "ZEDITPGM", ZEDITPGM, PROFILE ) ;
	p_poolMGR->put( RC, "ZBRPGM",   ZBRPGM,   PROFILE ) ;
	p_poolMGR->put( RC, "ZVIEWPGM", ZVIEWPGM, PROFILE ) ;
	p_poolMGR->put( RC, "ZFLSTPGM", ZFLSTPGM, PROFILE ) ;
	p_poolMGR->put( RC, "ZHELPPGM", ZHELPPGM, PROFILE ) ;
	p_poolMGR->put( RC, "ZOREXPGM", ZOREXPGM, PROFILE ) ;
	p_poolMGR->put( RC, "ZSHELP",   ZSHELP,   PROFILE ) ;
	p_poolMGR->put( RC, "ZMAXSCRN", d2ds(ZMAXSCRN), PROFILE ) ;
	p_poolMGR->put( RC, "ZMAXWAIT", d2ds(ZMAXWAIT), PROFILE ) ;
	p_poolMGR->put( RC, "ZWAIT",    d2ds(ZWAIT),    PROFILE ) ;

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

	p_poolMGR->destroyPool( RC, PROFILE  ) ;
	if ( RC == 0 )
	{
		cout << "SYSTEM profile ISPSPROF created successfully in " << ZSPROF << endl ;
	}
	else
	{
		cout << "ERROR saving profile ISPSPROF in " << ZSPROF << "  RC=" << RC << endl ;
	}
	cout << "*******************************************************************************************" << endl ;
	cout << endl ;
}

void setCUAcolours( string var, string val )
{
	int RC ;

	string var1 ;

	var1 = "ZC" + var ;

	if ( val[0] != 'R' && val[0] != 'G' && val[0] != 'Y' && val[0] != 'B' && val[0] != 'M' && val[0] != 'T' && val[0] != 'W' )
	{
		cout << "ERROR:: Invalid colour value of " << val[0] << " in setting " << var << endl ;
	}
	if ( val[1] != 'L' && val[1] != 'H' )
	{
		cout << "ERROR:: Invalid colour intensity of " << val[1] << " in setting " << var << endl ;
	}
	if ( val[2] != 'N' && val[2] != 'B'  && val[2] != 'R' && val[2] != 'U')
	{
		cout << "ERROR:: Invalid colour hilight of " << val[0] << " in setting " << var << endl ;
	}

	p_poolMGR->put( RC, var1, val, PROFILE )   ;
}
