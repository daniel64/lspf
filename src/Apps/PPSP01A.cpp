/*  Compile with ::                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPPSP01A.so -o libPPSP01A.so PPSP01A.cpp */

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


/* General utilities:                                                       */
/* lspf LOG                                                                 */
/* Application LOG                                                          */
/* PFKEY Settings                                                           */
/* COLOUR Settings                                                          */
/* T0DO list                                                                */
/* SHARED and PROFILE Variable display                                      */
/* Display LIBDEF status and search paths                                   */
/* Display loaded Modules (loaded Dynamic Classes)                          */
/* Display Saved File List                                                  */
/* Simple task list display (from ps aux output)                            */

#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>

#include "../lspf.h"
#include "../utilities.h"
#include "../pWidgets.h"
#include "../pVPOOL.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"

#include "PPSP01A.h"

using namespace std ;
using namespace boost::filesystem ;

#undef MOD_NAME
#undef LOGOUT
#define MOD_NAME PPSP01A
#define LOGOUT aplog


void PPSP01A::application()
{
	log( "I", "Application PPSP01A starting" << endl ) ;

	ZAPPDESC = "General utilities to display logs, PF Key settings, variables, etc." ;

	string LOGTYPE ;
	string ZALOG   ;
	string ZSLOG   ;
	string LOGLOC  ;

	vdefine( "ZCMD ZVERB ZROW1 ZROW2 ZAREA ZSHADOW ZAREAT ZSCROLLA", &ZCMD, &ZVERB, &ZROW1, &ZROW2, &ZAREA, &ZSHADOW, &ZAREAT, &ZSCROLLA ) ;
	vdefine( "ZSCROLLN ZAREAW ZAREAD", &ZSCROLLN, &ZAREAW, &ZAREAD ) ;
	vdefine( "ZALOG ZSLOG LOGTYPE LOGLOC ZCOL1", &ZALOG, &ZSLOG, &LOGTYPE, &LOGLOC, &ZCOL1 ) ;

	vget( "ZALOG ZSLOG", PROFILE ) ;
	vget( "ZHOME", SHARED ) ;

	if ( PARM == "AL" )
	{
		LOGTYPE = "Application" ;
		LOGLOC  = ZALOG   ;
		show_log( ZALOG ) ;
	}
	else if ( PARM == "SL" )
	{
		LOGTYPE = "LSPF"  ;
		LOGLOC  = ZSLOG   ;
		show_log( ZSLOG ) ;
	}
	else if ( PARM == "KEYS"    ) pfkeySettings()     ;
	else if ( PARM == "COLOURS" ) colourSettings()    ;
	else if ( PARM == "TODO"    ) todoList()          ;
	else if ( PARM == "VARS"    ) poolVariables()     ;
	else if ( PARM == "PATHS"   ) showPaths()         ;
	else if ( PARM == "CMDS"    ) showCommandTables() ;
	else if ( PARM == "MODS"    ) showLoadedClasses() ;
	else if ( PARM == "SAVELST" ) showSavedFileList() ;
	else if ( PARM == "TASKS"   ) showTasks()         ;
	else if ( PARM == "UTPGMS"  ) utilityPrograms()   ;
	else { log( "E", "Invalid parameter passed to PPSP01A: " << PARM << endl ) ; }

        cleanup() ;
        return    ;
}


void PPSP01A::show_log( string fileName )
{
	int fsize ;
	int t     ;

	bool rebuildZAREA ;

	string Rest  ;
	string Restu ;
	string MSG   ;
	string w1    ;
	string w2    ;
	string w3    ;

	filteri = " "  ;
	filterx = " "  ;
	firstLine = 0  ;
	maxCol    = 1  ;

	ZAHELP = "HPSP01A" ;

	vget( "ZSCRMAXW ZSCRMAXD", SHARED ) ;
	pquery( "PPSP01AL", "ZAREA", "ZAREAT", "ZAREAW", "ZAREAD" ) ;

	startCol     = 42     ;
	showDate     = false  ;
	showTime     = true   ;
	showMod      = true   ;
	showTask     = true   ;

	read_file( fileName ) ;
	fill_dynamic_area()   ;
	rebuildZAREA = false  ;
	Xon          = false  ;
	MSG          = ""     ;

	fsize = 0  ;
	file_has_changed( fileName, fsize ) ;

	while ( true )
	{
		ZCOL1 = right( d2ds( startCol-41 ), 7, '0' )  ;
		ZROW1 = right( d2ds( firstLine ), 8, '0' )    ;
		ZROW2 = right( d2ds( maxLines ), 8, '0' )     ;
		if ( MSG == "" ) { ZCMD = "" ; }

		display( "PPSP01AL", MSG, "ZCMD" ) ;
		if ( RC == 8 ) { return  ; }
		if ( RC >  8 ) { abend() ; }

		vget( "ZVERB ZSCROLLA ZSCROLLN", SHARED ) ;

		MSG   = "" ;
		Restu = subword( ZCMD, 2 ) ;
		ZCMD  = upper( ( ZCMD ) )  ;
		w1    = word( ZCMD, 1 )    ;
		w2    = word( ZCMD, 2 )    ;
		w3    = word( ZCMD, 3 )    ;
		Rest  = subword( ZCMD, 2 ) ;

		if ( file_has_changed( fileName, fsize ) )
		{
			read_file( fileName ) ;
			rebuildZAREA = true ;
			set_excludes() ;
		}

		if ( w1 == "" ) {}
		else if ( w1 == "ONLY" || w1 == "O" )
		{
			if ( Rest.size() == 1 )
			{
				filteri  = Rest ;
				set_excludes()  ;
				rebuildZAREA = true ;
			}
			else { MSG = "PPSP012" ; }
		}
		else if ( w1 == "EX" || w1 == "X" )
		{
			if ( Rest == "ALL" )
			{
				exclude_all()   ;
				rebuildZAREA = true ;
			}
			else if ( Rest == "ON" )
			{
				Xon      = true ;
				set_excludes()  ;
				rebuildZAREA = true ;
			}
			else if ( Rest.size() == 1 )
			{
				filterx  = Rest ;
				set_excludes()  ;
				rebuildZAREA = true ;
			}
			else { MSG = "PPSP013" ; }
		}
		else if ( w1 == "F" || w1 == "FIND" )
		{
			  find_lines( Restu ) ;
			  rebuildZAREA = true ;
		}
		else if ( abbrev( "RESET", w1 ) )
		{
			filteri = " "    ;
			filterx = " "    ;
			Xon     = false  ;
			rebuildZAREA = true  ;
			excluded.clear() ;
			for ( t = 0 ; t <= maxLines ; t++ ) { excluded.push_back( false ) ; }
		}
		else if ( abbrev( "REFRESH", w1 ) )
		{
			read_file( fileName ) ;
			rebuildZAREA = true ;
			set_excludes() ;
		}
		else if ( w1 == "SHOW" )
		{
			if      ( w2 == "DATE" ) { showDate = true ; }
			else if ( w2 == "TIME" ) { showTime = true ; }
			else if ( w2 == "MOD"  ) { showMod  = true ; }
			else if ( w2 == "TASK" ) { showTask = true ; }
			else if ( w2 == "ALL"  ) { showDate = true ; showTime = true ; showMod = true ; showTask = true ; }
			rebuildZAREA = true ;
		}
		else if ( w1 == "HIDE" )
		{
			if      ( w2 == "DATE" ) { showDate = false ; }
			else if ( w2 == "TIME" ) { showTime = false ; }
			else if ( w2 == "MOD"  ) { showMod  = false ; }
			else if ( w2 == "TASK" ) { showTask = false ; }
			else if ( w2 == "ALL"  ) { showDate = false ; showTime = false ; showMod = false ; showTask = false ; }
			rebuildZAREA = true ;
		}
		else 
		{
			MSG = "PPSP011" ;
			continue        ;
		}

		if ( ZVERB == "DOWN" )
		{
			rebuildZAREA = true ;
			if ( ZSCROLLA == "MAX" )
			{
				firstLine = maxLines - ZAREAD ;
				if ( firstLine < 0 ) firstLine = 0 ;
			}
			else
			{
				t = 0 ;
				for ( ; firstLine < ( maxLines - 1 ) ; firstLine++ )
				{
					if ( excluded[ firstLine ] ) continue ;
					t++ ;
					if ( t > ZSCROLLN ) break ;
				}
			}
		}
		else if ( ZVERB == "UP" )
		{
			rebuildZAREA = true ;
			if ( ZSCROLLA == "MAX" )
			{
				firstLine = 0 ;
			}
			else
			{
				t = 0 ;
				for ( ; firstLine > 0 ; firstLine-- )
				{
					if ( excluded[ firstLine ] ) continue ;
					t++ ;
					if ( t > ZSCROLLN ) break ;
				}
			}
		}
		else if ( ZVERB == "LEFT" )
		{
			rebuildZAREA = true ;
			if ( ZSCROLLA == "MAX" )
			{
				startCol = 42 ;
			}
			else
			{
				startCol = startCol - ZSCROLLN ;
				if ( startCol < 42 ) { startCol = 42 ; }
			}
		}
		else if ( ZVERB == "RIGHT" )
		{
			rebuildZAREA = true ;
			if ( ZSCROLLA == "MAX" )
			{
				startCol = maxCol - ZAREAW + 41 ;
			}
			else
			{
				startCol = startCol + ZSCROLLN ;
			}
		}

		if ( rebuildZAREA ) fill_dynamic_area() ;
		rebuildZAREA = false ;
	}
}


void PPSP01A::read_file( string fileName )
{
	string inLine ;
	ifstream fin( fileName.c_str() ) ;

	data.clear()     ;

	data.push_back( centre( " TOP OF DATA ", ZAREAW, '*' ) ) ;
	excluded.push_back( false ) ;
	while ( getline( fin, inLine ) )
	{
		inLine = inLine             ;
		data.push_back( inLine )    ;
		excluded.push_back( false ) ;
		if ( maxCol < inLine.size() ) maxCol = inLine.size() ;
	}
	maxCol++   ;
	data.push_back( centre( " BOTTOM OF DATA ", ZAREAW, '*' ) ) ;
	excluded.push_back( false ) ;
	maxLines = data.size() ;
	fin.close() ;
}


bool PPSP01A::file_has_changed( string fileName, int & fsize )
{
	struct stat results   ;

	lstat( fileName.c_str(), &results ) ;

	if ( fsize == 0 ) { fsize = results.st_size ; }
	else if ( fsize != results.st_size )
	{
		fsize = results.st_size ;
		return true ;
	}
	return false ;
}


void PPSP01A::set_excludes()
{
	int i ;
	for ( i = 1 ; i < maxLines ; i++ )
	{
		if ( Xon )
		{
			if ( data[ i ].substr( 39, 1 ) == "D" ||
			     data[ i ].substr( 39, 1 ) == "I" ||
			     data[ i ].substr( 39, 1 ) == "-" )
			{
				excluded[ i ] = true ;
				continue ;
			}
		}
		if ( data[ i ].size() > 39 )
		{
			if ( filteri != " " && filteri != data[ i ].substr( 39, 1 ) && data[ i ].substr( 39, 1 ) != "*" ) { excluded[ i ] = true ; continue ; }
			if ( filterx != " " && filterx == data[ i ].substr( 39, 1 ) && data[ i ].substr( 39, 1 ) != "*" ) { excluded[ i ] = true ; continue ; }
		}
	}

}


void PPSP01A::exclude_all()
{
	int i ;
	for ( i = 1 ; i < (maxLines-1) ; i++ ) { excluded[ i ] = true ; }
}


void PPSP01A::find_lines( string fnd )
{
	int i ;
	for ( i = 1 ; i < (maxLines-1) ; i++ )
	{
		if ( data[ i ].find( fnd ) == string::npos ) continue ;
		excluded[ i ] = false ;
	}
}


void PPSP01A::fill_dynamic_area()
{
	int p ;

	ZAREA   = "" ;
	ZSHADOW = "" ;
	string s     ;
	string s2( ZAREAW, N_TURQ ) ;
	string t     ;

	int l = 0 ;

	for( unsigned int k = firstLine ; k < data.size() ; k++ )
	{
		if ( excluded[ k ] ) continue ;
		l++ ;
		if ( l > ZAREAD ) break ;
		if ( (k == 0) || (k == maxLines-1 ) )
		{
			ZAREA   = ZAREA + substr( data[ k ], 1, ZAREAW ) ;
			ZSHADOW = ZSHADOW + s2 ;
		}
		else
		{
			t = "" ;
			s = string( ZAREAW, N_GREEN ) ;
			p = 0 ;
			if ( showDate ) { t = data[ k ].substr( 0,  12 )     ; s.replace( p, 12, 12, N_WHITE ) ; p = 12     ; }
			if ( showTime ) { t = t + data[ k ].substr( 12, 16 ) ; s.replace( p, 16, 16, N_WHITE ) ; p = p + 16 ; }
			if ( showMod  ) { t = t + data[ k ].substr( 28, 11 ) ; s.replace( p, 11, 11, N_RED   ) ; p = p + 11 ; }
			t = t + data[ k ].substr( 39, 2 ) ;
			t = t + substr( data[ k ], startCol, ZAREAW ) ;
			t.resize( ZAREAW, ' ' ) ;
			ZAREA = ZAREA + t ;
			s.replace( p, 2, 2, N_TURQ ) ;
			ZSHADOW = ZSHADOW + s ;
		}
	}
}



void PPSP01A::pfkeySettings()
{
	int RCode ;

	string ZPF01, ZPF02, ZPF03, ZPF04, ZPF05, ZPF06, ZPF07, ZPF08, ZPF09, ZPF10, ZPF11, ZPF12 ;
	string ZPF13, ZPF14, ZPF15, ZPF16, ZPF17, ZPF18, ZPF19, ZPF20, ZPF21, ZPF22, ZPF23, ZPF24 ;

	vdefine ( "ZPF01 ZPF02 ZPF03 ZPF04 ZPF05 ZPF06 ZPF07 ZPF08", &ZPF01, &ZPF02, &ZPF03, &ZPF04, &ZPF05, &ZPF06, &ZPF07, &ZPF08 ) ;
	vdefine ( "ZPF09 ZPF10 ZPF11 ZPF12 ZPF13 ZPF14 ZPF15 ZPF16", &ZPF09, &ZPF10, &ZPF11, &ZPF12, &ZPF13, &ZPF14, &ZPF15, &ZPF16 ) ;
	vdefine ( "ZPF17 ZPF18 ZPF19 ZPF20 ZPF21 ZPF22 ZPF23 ZPF24", &ZPF17, &ZPF18, &ZPF19, &ZPF20, &ZPF21, &ZPF22, &ZPF23, &ZPF24 ) ;

	vget( "ZPF01 ZPF02 ZPF03 ZPF04 ZPF05 ZPF06 ZPF07 ZPF08 ZPF09 ZPF10 ZPF11 ZPF12", PROFILE ) ;
	vget( "ZPF13 ZPF14 ZPF15 ZPF16 ZPF17 ZPF18 ZPF19 ZPF20 ZPF21 ZPF22 ZPF23 ZPF24", PROFILE ) ;

	while ( true )
	{
		if ( ZPF01 == "" ) { ZPF01 = "HELP"     ; } ;
		if ( ZPF02 == "" ) { ZPF02 = "SPLIT"    ; } ;
		if ( ZPF03 == "" ) { ZPF03 = "END"      ; } ;
		if ( ZPF04 == "" ) { ZPF04 = "RETURN"   ; } ;
		if ( ZPF05 == "" ) { ZPF05 = "RFIND"    ; } ;
		if ( ZPF06 == "" ) { ZPF06 = "RCHANGE"  ; } ;
		if ( ZPF07 == "" ) { ZPF07 = "UP"       ; } ;
		if ( ZPF08 == "" ) { ZPF08 = "DOWN"     ; } ;
		if ( ZPF09 == "" ) { ZPF09 = "SWAP"     ; } ;
		if ( ZPF10 == "" ) { ZPF10 = "LEFT"     ; } ;
		if ( ZPF11 == "" ) { ZPF11 = "RIGHT"    ; } ;
		if ( ZPF12 == "" ) { ZPF12 = "RETRIEVE" ; } ;
		if ( ZPF13 == "" ) { ZPF13 = "HELP"     ; } ;
		if ( ZPF14 == "" ) { ZPF14 = "SPLIT"    ; } ;
		if ( ZPF15 == "" ) { ZPF15 = "END"      ; } ;
		if ( ZPF16 == "" ) { ZPF16 = "RETURN"   ; } ;
		if ( ZPF17 == "" ) { ZPF17 = "RFIND"    ; } ;
		if ( ZPF18 == "" ) { ZPF18 = "RCHANGE"  ; } ;
		if ( ZPF19 == "" ) { ZPF19 = "UP"       ; } ;
		if ( ZPF20 == "" ) { ZPF20 = "DOWN"     ; } ;
		if ( ZPF21 == "" ) { ZPF21 = "SWAP"     ; } ;
		if ( ZPF22 == "" ) { ZPF22 = "LEFT"     ; } ;
		if ( ZPF23 == "" ) { ZPF23 = "RIGHT"    ; } ;
		if ( ZPF24 == "" ) { ZPF24 = "RETRIEVE" ; } ;

		ZCMD  = "" ;
		display( "PPSP01AK", "", "ZCMD" );
		RCode = RC ;
		if (RCode > 8 ) { abend() ; }

		vget( "ZVERB", SHARED ) ;

		if ( ZCMD == "CANCEL" ) { break ; }
		if ( ZCMD == "DELETE" )
		{
			verase( "ZPF01 ZPF02 ZPF03 ZPF04 ZPF05 ZPF06 ZPF07 ZPF08 ZPF09 ZPF10 ZPF11 ZPF12", PROFILE ) ;
			verase( "ZPF13 ZPF14 ZPF15 ZPF16 ZPF17 ZPF18 ZPF19 ZPF20 ZPF21 ZPF22 ZPF23 ZPF24", PROFILE ) ;
			break ;
		}
		if ( RCode == 8 || ZCMD == "SAVE" )
		{
			if ( ZPF01 != "" )   vput( "ZPF01", PROFILE ) ;
			else               verase( "ZPF01", PROFILE ) ;
			if ( ZPF02 != "" )   vput( "ZPF02", PROFILE ) ;
			else               verase( "ZPF02", PROFILE ) ;
			if ( ZPF03 != "" )   vput( "ZPF03", PROFILE ) ;
			else               verase( "ZPF03", PROFILE ) ;
			if ( ZPF04 != "" )   vput( "ZPF04", PROFILE ) ;
			else               verase( "ZPF04", PROFILE ) ;
			if ( ZPF05 != "" )   vput( "ZPF05", PROFILE ) ;
			else               verase( "ZPF05", PROFILE ) ;
			if ( ZPF06 != "" )   vput( "ZPF06", PROFILE ) ;
			else               verase( "ZPF06", PROFILE ) ;
			if ( ZPF07 != "" )   vput( "ZPF07", PROFILE ) ;
			else               verase( "ZPF07", PROFILE ) ;
			if ( ZPF08 != "" )   vput( "ZPF08", PROFILE ) ;
			else               verase( "ZPF08", PROFILE ) ;
			if ( ZPF09 != "" )   vput( "ZPF09", PROFILE ) ;
			else               verase( "ZPF09", PROFILE ) ;
			if ( ZPF10 != "" )   vput( "ZPF10", PROFILE ) ;
			else               verase( "ZPF10", PROFILE ) ;
			if ( ZPF11 != "" )   vput( "ZPF11", PROFILE ) ;
			else               verase( "ZPF11", PROFILE ) ;
			if ( ZPF12 != "" )   vput( "ZPF12", PROFILE ) ;
			else               verase( "ZPF12", PROFILE ) ;
			if ( ZPF13 != "" )   vput( "ZPF13", PROFILE ) ;
			else               verase( "ZPF13", PROFILE ) ;
			if ( ZPF14 != "" )   vput( "ZPF14", PROFILE ) ;
			else               verase( "ZPF14", PROFILE ) ;
			if ( ZPF15 != "" )   vput( "ZPF15", PROFILE ) ;
			else               verase( "ZPF15", PROFILE ) ;
			if ( ZPF16 != "" )   vput( "ZPF16", PROFILE ) ;
			else               verase( "ZPF16", PROFILE ) ;
			if ( ZPF17 != "" )   vput( "ZPF17", PROFILE ) ;
			else               verase( "ZPF17", PROFILE ) ;
			if ( ZPF18 != "" )   vput( "ZPF18", PROFILE ) ;
			else               verase( "ZPF18", PROFILE ) ;
			if ( ZPF19 != "" )   vput( "ZPF19", PROFILE ) ;
			else               verase( "ZPF19", PROFILE ) ;
			if ( ZPF20 != "" )   vput( "ZPF20", PROFILE ) ;
			else               verase( "ZPF20", PROFILE ) ;
			if ( ZPF21 != "" )   vput( "ZPF21", PROFILE ) ;
			else               verase( "ZPF21", PROFILE ) ;
			if ( ZPF22 != "" )   vput( "ZPF22", PROFILE ) ;
			else               verase( "ZPF22", PROFILE ) ;
			if ( ZPF23 != "" )   vput( "ZPF23", PROFILE ) ;
			else               verase( "ZPF23", PROFILE ) ;
			if ( ZPF24 != "" )   vput( "ZPF24", PROFILE ) ;
			else               verase( "ZPF24", PROFILE ) ;
			if ( RCode == 8 ) { break ; }
		}
	}
	vdelete( "ZPF01 ZPF02 ZPF03 ZPF04 ZPF05 ZPF06 ZPF07 ZPF08" ) ;
	vdelete( "ZPF09 ZPF10 ZPF11 ZPF12 ZPF13 ZPF14 ZPF15 ZPF16" ) ;
	vdelete( "ZPF17 ZPF18 ZPF19 ZPF20 ZPF21 ZPF22 ZPF23 ZPF24" ) ;
}


void PPSP01A::colourSettings()
{

	int  i ;
	char c ;
	string MSG    ;
	string CURFLD ;
	string var1   ;
	string var2   ;
	string var3   ;
	string val    ;
	string isps_var ;
	string prof_var ;
	string attr1  ;
	string attr2  ;
	string attr3  ;
	string COLOUR ;
	string INTENS ;
	string HILITE ;

	map< int, string>VarList ;
	map< int, string>DefList ;
	map< int, string>OrigList ;

	VarList[ 1  ] = "AB"   ;
	VarList[ 2  ] = "ABSL" ;
	VarList[ 3  ] = "ABU"  ;
	VarList[ 4  ] = "AMT"  ;
	VarList[ 5  ] = "AWF"  ;
	VarList[ 6  ] = "CT"   ;
	VarList[ 7  ] = "CEF"  ;
	VarList[ 8  ] = "CH"   ;
	VarList[ 9  ] = "DT"   ;
	VarList[ 10 ] = "ET"   ;
	VarList[ 11 ] = "EE"   ;
	VarList[ 12 ] = "FP"   ;
	VarList[ 13 ] = "FK"   ;
	VarList[ 14 ] = "IMT"  ;
	VarList[ 15 ] = "LEF"  ;
	VarList[ 16 ] = "LID"  ;
	VarList[ 17 ] = "LI"   ;
	VarList[ 18 ] = "NEF"  ;
	VarList[ 19 ] = "NT"   ;
	VarList[ 20 ] = "PI"   ;
	VarList[ 21 ] = "PIN"  ;
	VarList[ 22 ] = "PT"   ;
	VarList[ 23 ] = "PS"   ;
	VarList[ 24 ] = "PAC"  ;
	VarList[ 25 ] = "PUC"  ;
	VarList[ 26 ] = "RP"   ;
	VarList[ 27 ] = "SI"   ;
	VarList[ 28 ] = "SAC"  ;
	VarList[ 29 ] = "SUC"  ;
	VarList[ 30 ] = "VOI"  ;
	VarList[ 31 ] = "WMT"  ;
	VarList[ 32 ] = "WT"   ;
	VarList[ 33 ] = "WASL" ;

	DefList[ 1  ] = KAB    ;
	DefList[ 2  ] = KABSL  ;
	DefList[ 3  ] = KABU   ;
	DefList[ 4  ] = KAMT   ;
	DefList[ 5  ] = KAWF   ;
	DefList[ 6  ] = KCT    ;
	DefList[ 7  ] = KCEF   ;
	DefList[ 8  ] = KCH    ;
	DefList[ 9  ] = KDT    ;
	DefList[ 10 ] = KET    ;
	DefList[ 11 ] = KEE    ;
	DefList[ 12 ] = KFP    ;
	DefList[ 13 ] = KFK    ;
	DefList[ 14 ] = KIMT   ;
	DefList[ 15 ] = KLEF   ;
	DefList[ 16 ] = KLID   ;
	DefList[ 17 ] = KLI    ;
	DefList[ 18 ] = KNEF   ;
	DefList[ 19 ] = KNT    ;
	DefList[ 20 ] = KPI    ;
	DefList[ 21 ] = KPIN   ;
	DefList[ 22 ] = KPT    ;
	DefList[ 23 ] = KPS    ;
	DefList[ 24 ] = KPAC   ;
	DefList[ 25 ] = KPUC   ;
	DefList[ 26 ] = KRP    ;
	DefList[ 27 ] = KSI    ;
	DefList[ 28 ] = KSAC   ;
	DefList[ 29 ] = KSUC   ;
	DefList[ 30 ] = KVOI   ;
	DefList[ 31 ] = KWMT   ;
	DefList[ 32 ] = KWT    ;
	DefList[ 33 ] = KWASL  ;

	reloadCUATables = true ;

	attr1 = "" ;
	attr2 = "" ;
	attr3 = "" ;

	control( "DISPLAY", "LOCK" ) ;
	display( "PPSP01CL" )        ;

	for ( i = 1 ; i < 34 ; i++ )
	{
		if ( setScreenAttrs( VarList[ i ], i, COLOUR, INTENS, HILITE ) > 0 )
		{
			log( "E", "ISPS variable " << "ZC" + VarList[ i ] << " not found.  Re-run setup program to create" << endl ) ;
			abend() ;
		}
	}

	for ( i = 1 ; i < 34 ; i++ )
	{
		isps_var = "ZC" + VarList[i] ;
		vcopy( isps_var, val, MOVE ) ;
		if ( RC > 0 )
		{
			log( "E", "ISPS variable " << isps_var << " not found.  Re-run setup program to create" << endl ) ;
			abend() ;
		}
		OrigList[ i ] = val ;
	}

	MSG    = "" ;
	ZCMD   = "" ;
	while ( true )
	{
		if ( MSG == "" ) { CURFLD = "ZCMD" ; }
		display( "PPSP01CL", MSG, CURFLD ) ;
		if (RC  > 8 ) { abend()   ;          }
		if (RC == 8 ) { cleanup() ; break  ; }

		vget( "ZVERB", SHARED )   ;
		if ( ZCMD == "" ) {}
		else if ( ZCMD == "CANCEL" )
		{
			for ( i = 1 ; i < 34 ; i++ )
			{
				isps_var = "ZC" + VarList[ i ] ;
				vcopy( isps_var, val, MOVE )   ;
				if ( RC > 0 )
				{
					log( "E", "ISPS variable " << isps_var << " not found.  Re-run setup program to create" << endl ) ;
					abend() ;
				}
				setISPSVar( VarList[ i ], OrigList[ i ] ) ;
			}
			return ;
		}
		else if ( ZCMD == "DEFAULTS" )
		{
			for ( i = 1 ; i < 34 ; i++ )
			{
				setISPSVar( VarList[ i ], DefList[ i ]  ) ;
			}
			for ( i = 1 ; i < 34 ; i++ )
			{
				setScreenAttrs( VarList[ i ],  i, COLOUR, INTENS, HILITE ) ;
			}
		}
		else if ( ZCMD == "SAVE" )
		{
			ZCMD = "" ;
			for ( i = 1 ; i < 34 ; i++ )
			{
				isps_var = "ZC" + VarList[i] ;
				vcopy( isps_var, val, MOVE ) ;
				if ( RC > 0 )
				{
					log( "E", "ISPS variable " << isps_var << " not found.  Re-run setup program to create" << endl ) ;
					abend() ;
				}
				prof_var = isps_var ;
				prof_var.replace( 0, 1, 1, 'A' ) ;
				vdefine( prof_var, &val ) ;
				vput( prof_var, PROFILE ) ;
				vdelete( prof_var ) ;
			}
			if ( RC == 0 ) { MSG = "PPSP01A" ; }
			continue ;
		}
		else if ( ZCMD == "RESTORE" )
		{
			ZCMD = "" ;
			for ( i = 1 ; i < 34 ; i++ )
			{
				prof_var = "AC" + VarList[i] ;
				vcopy( prof_var, val, MOVE ) ;
				if ( RC > 0 ) { MSG = "PPSP019" ; break ; }
				isps_var = prof_var ;
				isps_var.replace( 0, 1, 1, 'Z' ) ;
				vdefine( isps_var, &val ) ;
				vput( isps_var, PROFILE ) ;
				vdelete( isps_var ) ;
				if ( setScreenAttrs( VarList[i],  i, COLOUR, INTENS, HILITE ) > 0 ) { abend() ; }
			}
			if ( RC == 0 ) { MSG = "PPSP01B" ; }
			continue ;
		}

		MSG  = "" ;
		ZCMD = "" ;
		for ( i = 1 ; i < 34 ; i++)
		{
			var1 = "COLOUR" + right( d2ds(i), 2, '0') ;
			var2 = "INTENS" + right( d2ds(i), 2, '0') ;
			var3 = "HILITE" + right( d2ds(i), 2, '0') ;
			vcopy( var1, COLOUR, MOVE ) ;
			vcopy( var2, INTENS, MOVE ) ;
			vcopy( var3, HILITE, MOVE ) ;
			if ( COLOUR == "" ) { COLOUR = DefList[ i ][ 0 ] ; }
			if ( INTENS == "" ) { INTENS = DefList[ i ][ 1 ] ; }
			if ( HILITE == "" ) { HILITE = DefList[ i ][ 2 ] ; }
			isps_var = "ZC" + VarList[i] ;
			vcopy( isps_var, val, MOVE ) ;
			if ( RC > 0 )
			{
				log( "E", "ISPS variable " << isps_var << " not found.  Re-run setup program to create" << endl ) ;
				abend() ;
			}
			if ( val.size() != 3 )
			{
				log( "E", "ISPS variable " << isps_var << " has invalid value of " << val << "  Re-run setup program to re-create" << endl ) ;
				abend() ;
			}
			switch ( COLOUR[ 0 ] )
			{
			case 'R':
				vreplace( var1, "RED" ) ;
				val[ 0 ] = 'R'          ;
				attr1 = "COLOUR(RED)"   ;
				break ;
			case 'G':
				vreplace( var1, "GREEN" ) ;
				val[ 0 ] = 'G'            ;
				attr1 = "COLOUR(GREEN)"   ;
				break ;
			case 'Y':
				vreplace( var1, "YELLOW" ) ;
				val[ 0 ] = 'Y'             ;
				attr1 = "COLOUR(YELLOW)"   ;
				break ;
			case 'B':
				vreplace( var1, "BLUE" ) ;
				val[ 0 ] = 'B'           ;
				attr1 = "COLOUR(BLUE)"   ;
				break ;
			case 'M':
				vreplace( var1, "MAGENTA" ) ;
				val[ 0 ] = 'M'              ;
				attr1 = "COLOUR(MAGENTA)"   ;
				break ;
			case 'T':
				vreplace( var1, "TURQ" ) ;
				val[ 0 ] = 'T'           ;
				attr1 = "COLOUR(TURQ)"   ;
				break ;
			case 'W':
				vreplace( var1, "WHITE" ) ;
				val[ 0 ] = 'W'            ;
				attr1 = "COLOUR(WHITE)"   ;
				break ;
			default:
				MSG    = "PPSP016" ;
				CURFLD = var1      ;
			}
			switch ( INTENS[ 0 ] )
			{
			case 'H':
				vreplace( var2, "HIGH" ) ;
				val[ 1 ] = 'H'           ;
				attr2 = "INTENSE(HIGH)"  ;
				break ;
			case 'L':
				vreplace( var2, "LOW"  ) ;
				val[ 1 ] = 'L'           ;
				attr2 = "INTENSE(LOW)"   ;
				break ;
			default:
				MSG    = "PPSP017" ;
				CURFLD = var2      ;
			}
			switch ( HILITE[ 0 ] )
			{
			case 'N':
				vreplace( var3, "NONE" ) ;
				val[ 2 ] = 'N'           ;
				attr3 = "HILITE(NONE)"   ;
				break ;
			case 'B':
				vreplace( var3, "BLINK" ) ;
				val[ 2 ] = 'B'            ;
				attr3 = "HILITE(BLINK)"   ;
					break ;
			case 'R':
				vreplace( var3, "REVERSE" ) ;
				val[ 2 ] = 'R'              ;
				attr3 = "HILITE(REVERSE)"   ;
				break ;
			case 'U':
				vreplace( var3, "USCORE" ) ;
				val[ 2 ] = 'U'             ;
				attr3 = "HILITE(USCORE)"   ;
				break ;
			default:
				MSG    = "PPSP018" ;
				CURFLD = var3      ;
			}
			vdefine( isps_var, &val ) ;
			vput( isps_var, PROFILE ) ;
			vdelete( isps_var )       ;
			attr( var1, attr1 )       ;
			if ( RC > 0 ) { log( "E", "Colour change for field " << var1 << " has failed." << endl ) ; }
			attr( var2, attr1 + " " + attr2 ) ;
			if ( RC > 0 ) { log( "E", "Colour/intense change for field " << var2 << " has failed." << endl ) ; }
			attr( var3, attr1 + " " + attr2 + " " + attr3 ) ;
			if ( RC > 0 ) { log( "E", "Colour/intense/hilite change for field " << var3 << " has failed." << endl ) ; }
		}
	}
}


int PPSP01A::setScreenAttrs( string name, int itr, string & COLOUR, string & INTENS, string & HILITE )
{
	string t ;
	char   c ;

	string var1 ;
	string var2 ;
	string var3 ;

	vcopy( "ZC" + name, t, MOVE ) ;
	if ( RC > 0 ) { log( "E", "Variable ZC" << name << " not found in ISPS profile" << endl ) ; return 8 ; }
	else
	{
		if ( t.size() != 3 ) { log( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ; return 8 ; }
		c = t[ 0 ] ;
		if      ( c == 'R' ) COLOUR = "RED"     ;
		else if ( c == 'G' ) COLOUR = "GREEN"   ;
		else if ( c == 'Y' ) COLOUR = "YELLOW"  ;
		else if ( c == 'B' ) COLOUR = "BLUE"    ;
		else if ( c == 'M' ) COLOUR = "MAGENTA" ;
		else if ( c == 'T' ) COLOUR = "TURQ"    ;
		else if ( c == 'W' ) COLOUR = "WHITE"   ;
		else { log( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ; }
		c = t[ 1 ] ;
		if      ( c == 'H' ) INTENS = "HIGH" ;
		else if ( c == 'L' ) INTENS = "LOW" ;
		else { log( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ; }
		c = t[ 2 ] ;
		if      ( c == 'N' ) HILITE = "NONE"    ;
		else if ( c == 'B' ) HILITE = "BLINK"   ;
		else if ( c == 'R' ) HILITE = "REVERSE" ;
		else if ( c == 'U' ) HILITE = "USCORE"  ;
		else { log( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ; }
	}

	var1 = "COLOUR" + right( d2ds( itr ), 2, '0') ;
	var2 = "INTENS" + right( d2ds( itr ), 2, '0') ;
	var3 = "HILITE" + right( d2ds( itr ), 2, '0') ;

	attr( var1, "COLOUR(" + COLOUR + ")" ) ;
	if ( RC > 0 ) { log( "E", "Colour change for field " << var1 << " has failed." << endl ) ; }

	attr( var2, "COLOUR(" + COLOUR + ") INTENSE(" + INTENS + ")" ) ;
	if ( RC > 0 ) { log( "E", "Colour/intense change for field " << var2 << " has failed." << endl ) ; }

	attr( var3,  "COLOUR(" + COLOUR + ") INTENSE(" + INTENS + ") HILITE(" + HILITE + ")" ) ;
	if ( RC > 0 ) { log( "E", "Colour/intense/hilite change for field " << var3 << " has failed." << endl ) ; }

	vreplace( var1, COLOUR ) ;
	vreplace( var2, INTENS ) ;
	vreplace( var3, HILITE ) ;
	return 0 ;
}


void PPSP01A::setISPSVar( string var, string val )
{
	string isps_var ;

	isps_var = "ZC" + var     ;
	vdefine( isps_var, &val ) ;
	vput( isps_var, PROFILE ) ;
	vdelete( isps_var )       ;
	return                    ;
}


void PPSP01A::todoList()
{
	// No need to do vdefines for the TODO variables TODO1... as it is not necessary to reference
	// them in the application.  First reference of these variables in the panel will create
	// implicit function pool variables that the vput will save to the PROFILE

	while ( true )
	{
		ZCMD  = "" ;
		display( "PPSP01TD", "", "ZCMD" );
		if (RC  > 8 ) { abend()   ;          }
		if (RC == 8 ) { cleanup() ; break  ; }

		vget( "ZVERB", SHARED ) ;
		if ( ZCMD == "CANCEL" ) return ;
	}
	vput( "TODO1 TODO2  TODO3  TODO4  TODO5 TODO6 TODO7 TODO8", PROFILE ) ;
	vput( "TODO9 TODO10 TODO11 TODO12", PROFILE ) ;
}


void PPSP01A::poolVariables()
{
	int i      ;
	int j      ;
	string MSG ;
	string cw  ;
	string w2  ;

	VARLST = "VARLST" + right( d2ds( taskid() ), 2, '0' ) ;

	vdefine( "SEL VAR VPOOL VPLVL VAL MESSAGE", &SEL, &VAR, &VPOOL, &VPLVL, &VAL, &MESSAGE ) ;

	getpoolVariables( "" ) ;
	
	MSG = "" ;
	i   = 1  ;
	while ( true )
	{
		tbtop( VARLST )     ;
		tbskip( VARLST, i ) ;
		if ( MSG == "" ) { ZCMD = "" ; }
		tbdispl( VARLST, "PPSP01AV", MSG, "ZCMD" ) ;
		if ( RC  >  8 ) { abend() ; }
		if ( RC ==  8 ) { break   ; }
		MSG = "" ;
		if ( (ZCMD == "REF") || (ZCMD == "RES") ) { tbend( VARLST ) ; getpoolVariables( "" ) ; continue ; }
		cw = word( ZCMD, 1 ) ;
		w2 = word( ZCMD, 2 ) ;
		if ( cw == "O" ) { tbend( VARLST ) ; getpoolVariables( w2 ) ; continue ; }
		if ( ZCMD != "" ) { MSG = "PSYS018" ; continue ; }
		i = ZTDTOP ;
		while ( ZTDSELS > 0 )
		{
			if ( SEL == "D" )
			{
				control( "ERRORS", "RETURN" ) ;
				if ( VPOOL == "S" ) verase( VAR, SHARED  ) ;
				else                verase( VAR, PROFILE ) ;
				MESSAGE = "*Delete RC=" + d2ds(RC) + "*" ;
				control( "ERRORS", "CANCEL" ) ;
				SEL = "" ;
			}
			tbput( VARLST )     ;
			if ( ZTDSELS > 1 )
			{
				tbdispl( VARLST ) ;
				if ( RC > 4 ) { break ; }
			}
			else { ZTDSELS = 0 ; }
		}
	}
	tbend( VARLST ) ;
	return ;
}


void PPSP01A::getpoolVariables( string pattern )
{
	// SHARED 1  - shared variable pool
	// SHARED 2  - default variable pool (@DEFSHAR)
	// PROFILE 1 - application variable pool
	// PROFILE 2 - Read-only extention pool
	// PROFILE 3 - default read-only profile pool (@DEFPROF)
	// PROFILE 4 - System profile (ISPSPROF)

	string varlist ;
	int i  ;
	int ws ;

	tbcreate( VARLST, "", "SEL VAR VPOOL VPLVL MESSAGE VAL", NOWRITE ) ;

	SEL     = "" ;
	MESSAGE = "" ;

	varlist = vlist( SHARED, 1 ) ;
	VPOOL = "S" ;
	VPLVL = "1" ;
	ws    = words( varlist ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		VAR = word( varlist, i ) ;
		if ( (pattern != "") && (pos( pattern, VAR ) == 0) ) { continue ; }
		vcopy( VAR, VAL, MOVE ) ;
		tbadd( VARLST )    ;
	}

	varlist = vlist( SHARED, 2 ) ;
	VPLVL   = "2" ;
	ws      = words( varlist ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		VAR = word( varlist, i ) ;
		if ( (pattern != "") && (pos( pattern, VAR ) == 0) ) { continue ; }
		vcopy( VAR, VAL, MOVE ) ;
		tbadd( VARLST )    ;
	}

	VPOOL = "P" ;
	VPLVL = "1" ;
	varlist = vlist( PROFILE, 1 ) ;
	ws    = words( varlist ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		VAR = word( varlist, i ) ;
		if ( (pattern != "") && (pos( pattern, VAR ) == 0) ) { continue ; }
		vcopy( VAR, VAL, MOVE ) ;
		tbadd( VARLST )    ;
	}

	varlist = vlist( PROFILE, 2 ) ;
	VPLVL   = "2"                 ;
	ws      = words( varlist )    ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		VAR = word( varlist, i ) ;
		if ( (pattern != "") && (pos( pattern, VAR ) == 0) ) { continue ; }
		vcopy( VAR, VAL, MOVE ) ;
		tbadd( VARLST )    ;
	}

	varlist = vlist( PROFILE, 3 ) ;
	VPLVL   = "3"                 ;
	ws      = words( varlist )    ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		VAR = word( varlist, i ) ;
		if ( (pattern != "") && (pos( pattern, VAR ) == 0) ) { continue ; }
		vcopy( VAR, VAL, MOVE ) ;
		tbadd( VARLST )    ;
	}

	varlist = vlist( PROFILE, 4 ) ;
	VPLVL   = "4"                 ;
	ws      = words( varlist )    ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		VAR = word( varlist, i ) ;
		if ( (pattern != "") && (pos( pattern, VAR ) == 0) ) { continue ; }
		vcopy( VAR, VAL, MOVE ) ;
		tbadd( VARLST )    ;
	}

	tbtop( VARLST ) ;
}


void PPSP01A::showPaths()
{
	int i ;
	int j ;
	string PGM      ;
	string LIBDEFM  ;
	string LIBDEFP  ;
	string LIBDEFT  ;
	string PATHLST  ;
	string PVAR     ;
	string PATH     ;
	string MESSAGE  ;
	string DESCRIPT ;
	string MSG      ;

	vdefine( "LIBDEFM LIBDEFP LIBDEFT", &LIBDEFM, &LIBDEFP, &LIBDEFT ) ;

	if ( libdef_muser ) LIBDEFM = "LIBDEF active for user message search" ;
	else                LIBDEFM = "LIBDEF not active for user message search" ;
	if ( libdef_puser ) LIBDEFP = "LIBDEF active for user panel search" ;
	else                LIBDEFP = "LIBDEF not active for user panel search" ;
	if ( libdef_muser ) LIBDEFT = "LIBDEF active for user table search" ;
	else                LIBDEFT = "LIBDEF not active for user table search" ;

	PATHLST = "PTHLST" + right( d2ds( taskid() ), 2, '0' ) ;

	vdefine( "SEL PVAR PATH MESSAGE DESCRIPT", &SEL, &PVAR, &PATH, &MESSAGE, &DESCRIPT ) ;

	tbcreate( PATHLST, "", "SEL PVAR PATH MESSAGE DESCRIPT", NOWRITE ) ;

	SEL      = ""        ;
	PVAR     = "ZLDPATH" ;
	DESCRIPT = "Path for application modules" ;
	for ( i = 1 ; i <= getpaths( ZLDPATH ) ; i++)
	{
		PATH = getpath( ZLDPATH, i ) ;
		MESSAGE = "" ;
		if ( !is_directory( PATH ) ) MESSAGE = "Path not found" ;
		tbadd( PATHLST ) ;
		DESCRIPT = "" ;
		PVAR     = "" ;
	}

	if ( libdef_muser )
	{
		PVAR = "ZMUSER" ;
		DESCRIPT = "LIBDEF search path for messages" ;
		for ( i = 1 ; i <= getpaths( ZMUSER ) ; i++)
		{
			PATH = getpath( ZMUSER, i ) ;
			MESSAGE = "" ;
			if ( !is_directory( PATH ) ) MESSAGE = "Path not found" ;
			tbadd( PATHLST ) ;
			DESCRIPT = "" ;
			PVAR     = "" ;
		}
	}

	PVAR = "ZMLIB" ;
	DESCRIPT = "Search for messages" ;
	for ( i = 1 ; i <= getpaths( ZMLIB ) ; i++)
	{
		PATH = getpath( ZMLIB, i ) ;
		MESSAGE = "" ;
		if ( !is_directory( PATH ) ) MESSAGE = "Path not found" ;
		tbadd( PATHLST ) ;
		DESCRIPT = "" ;
		PVAR     = "" ;
	}

	if ( libdef_puser )
	{
		PVAR = "ZPUSER" ;
		DESCRIPT = "LIBDEF search path for panels" ;
		for ( i = 1 ; i <= getpaths( ZPUSER ) ; i++)
		{
			PATH = getpath( ZPUSER, i ) ;
			MESSAGE = "" ;
			if ( !is_directory( PATH ) ) MESSAGE = "Path not found" ;
			tbadd( PATHLST ) ;
			DESCRIPT = "" ;
			PVAR     = "" ;
		}
	}

	PVAR = "ZPLIB" ;
	DESCRIPT = "Search path for panels" ;
	for ( i = 1 ; i <= getpaths( ZPLIB ) ; i++)
	{
		PATH = getpath( ZPLIB, i ) ;
		MESSAGE = "" ;
		if ( !is_directory( PATH ) ) MESSAGE = "Path not found" ;
		tbadd( PATHLST ) ;
		DESCRIPT = "" ;
		PVAR     = "" ;
	}

	if ( libdef_tuser )
	{
		PVAR = "ZTUSER" ;
		DESCRIPT = "LIBDEF search path for tables" ;
		for ( i = 1 ; i <= getpaths( ZTUSER ) ; i++)
		{
			PATH = getpath( ZTUSER, i ) ;
			MESSAGE = "" ;
			if ( !is_directory( PATH ) ) MESSAGE = "Path not found" ;
			tbadd( PATHLST ) ;
			DESCRIPT = "" ;
			PVAR     = "" ;
		}
	}

	PVAR     = "ZSPROF" ;
	PATH     =  ZSPROF  ;
	DESCRIPT = "Path for ISPS system profile" ;
	tbadd( PATHLST )    ;

	PVAR     = "ZSYSPATH" ;
	PATH     =  ZSYSPATH  ;
	DESCRIPT = "System Path" ;
	tbadd( PATHLST )    ;

	PVAR = "ZTLIB" ;
	DESCRIPT = "Search path for tables" ;
	for ( i = 1 ; i <= getpaths( ZTLIB ) ; i++)
	{
		PATH = getpath( ZTLIB, i ) ;
		MESSAGE = "" ;
		if ( !is_directory( PATH ) ) MESSAGE = "Path not found" ;
		tbadd( PATHLST ) ;
		DESCRIPT = "" ;
		PVAR     = "" ;
	}

	PVAR     = "ZUPROF" ;
	PATH     =  ZUPROF   ;
	DESCRIPT = "User home profile path" ;
	tbadd( PATHLST )    ;

	PVAR     = "ZORXPATH" ;
	DESCRIPT = "Object REXX EXEC search path" ;
	for ( i = 1 ; i <= getpaths( ZORXPATH ) ; i++)
	{
		PATH = getpath( ZORXPATH, i ) ;
		MESSAGE = "" ;
		if ( !is_directory( PATH ) ) MESSAGE = "Path not found" ;
		tbadd( PATHLST ) ;
		DESCRIPT = "" ;
		PVAR     = "" ;
	}

	tbtop( PATHLST ) ;
	MSG = "" ;
	i   = 1  ;
	while ( true )
	{
		tbtop( PATHLST )     ;
		tbskip( PATHLST, i ) ;
		if ( MSG == "" ) { ZCMD  = "" ; }
		tbdispl( PATHLST, "PPSP01AP", MSG, "ZCMD" ) ;
		if ( RC  >  8 ) { abend() ; }
		if ( RC ==  8 ) { break   ; }
		MSG = ""     ;
		i   = ZTDTOP ;
		while ( ZTDSELS > 0 )
		{
			if ( (SEL == "B") || (SEL == "L") || (SEL == "S"))
			{
				if ( is_directory( PATH ) )
				{
					MESSAGE = "*Listed*" ;
					vcopy( "ZFLSTPGM", PGM, MOVE ) ;
					select( "PGM(" + PGM + ") PARM(" + PATH + ")" ) ;
				}
				else MESSAGE = "*Error*"    ;
			}
			else if ( SEL != "" ) { MESSAGE = "*Error*" ; }
			SEL = ""             ;
			tbput( PATHLST )     ;
			if ( ZTDSELS > 1 )
			{
				tbdispl( PATHLST ) ;
				if ( RC > 4 ) { break ; }
			}
			else { ZTDSELS = 0 ; }
		}

	}
	tbend( PATHLST ) ;
}


void PPSP01A::showCommandTables()
{
	int i ;

	string CMDTAB   ;
	string OCMDTAB  ;
	string ZCTVERB  ;
	string ZCTTRUNC ;
	string ZCTACT   ;
	string ZCTDESC  ;
	string MSG      ;
	string APPLCMD  ;
	string APPLCMDL ;

	vdefine( "ZAPPLID CMDTAB ZCTVERB ZCTTRUNC ZCTACT ZCTDESC APPLCMD APPLCMDL", &ZAPPLID, &CMDTAB, &ZCTVERB, &ZCTTRUNC, &ZCTACT, &ZCTDESC, &APPLCMD, &APPLCMDL ) ;

	vget( "ZAPPLID" ) ;
	vget( "CMDTAB", PROFILE ) ;
	if ( CMDTAB == "" ) CMDTAB = "ISPCMDS" ;
	OCMDTAB = CMDTAB ;

	APPLCMD  = "" ;
	APPLCMDL = "" ;
	if ( ZAPPLID != "ISP" )
	{
		APPLCMD = ZAPPLID + "CMDS" ;
		tbopen( APPLCMD, NOWRITE, "", SHARE ) ;
		if ( RC > 4 )
		{
			APPLCMDL = "Application Command Table Not Found" ;
		}
		else
		{
			APPLCMDL = "" ;
			tbend( APPLCMD ) ;
		}
	}
	MSG     = ""     ;

	tbopen( CMDTAB, NOWRITE, "", SHARE ) ;
	if ( RC > 0 )
	{
		CMDTAB = "ISPCMDS" ;
		tbopen( CMDTAB, NOWRITE, "", SHARE ) ;
	}
	i = 1 ;
	while ( true )
	{
		tbtop( CMDTAB )     ;
		tbskip( CMDTAB, i ) ;
		ZCMD  = "" ;
		tbdispl( CMDTAB, "PPSP01AC", MSG, "ZCMD" ) ;
		i = ZTDTOP ;
		if ( RC  >  8 ) { abend() ; }
		if ( RC ==  8 ) { break   ; }
		MSG = "" ;
		if ( OCMDTAB != CMDTAB )
		{
			tbopen( CMDTAB, NOWRITE, "", SHARE ) ;
			if ( RC == 0 )
			{
				tbend( OCMDTAB ) ; 
				OCMDTAB = CMDTAB ;
			}
			else
			{
				CMDTAB = OCMDTAB ;
				MSG = "PPSP014"  ;
			}
		}
	}
	tbend( CMDTAB ) ;
	vput( "CMDTAB", PROFILE ) ;

}


void PPSP01A::showLoadedClasses()
{
	int i      ;
	int ws     ;
	string w1  ;
	string w2  ;
	string w3  ;
	string MSG ;
	string VAR ;
	string varlist ;

	vdefine( "APPL MOD MODPATH", &APPL, &MOD, &MODPATH ) ;

	MODLST = "MODLST" + right( d2ds( taskid() ), 2, '0' ) ;
	tbcreate( MODLST, "APPL", "MOD MODPATH", NOWRITE ) ;
	tbsort( MODLST, "APPL,C,A" ) ;
	varlist = vlist( SHARED, 2 ) ;

	for ( ws = words( varlist ), i = 1 ; i <= ws ; i++ )
	{
		VAR = word( varlist, i ) ;
		if ( pos( "ZDLC", VAR ) == 0 ) { continue ; }
		vcopy( VAR, APPL, MOVE ) ;
		VAR.replace( 3, 1, 1, 'M' ) ;
		vcopy( VAR, MOD, MOVE ) ;
		VAR.replace( 3, 1, 1, 'P' ) ;
		vcopy( VAR, MODPATH, MOVE ) ;
		tbadd( MODLST, "", "ORDER" ) ;
	}
	tbtop( MODLST ) ;

	MSG = "" ;
	i   = 1  ;
	while ( true )
	{
		tbtop( MODLST )     ;
		tbskip( MODLST, i ) ;
		if ( MSG == "" ) { ZCMD = "" ; }
		tbdispl( MODLST, "PPSP01ML", MSG, "ZCMD" ) ;
		if ( RC  >  8 ) { abend() ; }
		if ( RC ==  8 ) { break   ; }
		MSG = "" ;
		i   = ZTDTOP ;
		ws = words( ZCMD )   ;
		w1 = word( ZCMD, 1 ) ;
		w2 = word( ZCMD, 2 ) ;
		w3 = word( ZCMD, 3 ) ;
		if ( w1 == "SORT" )
		{
			if ( w2 == "" ) { w2 = "APPL" ; }
			if ( w3 == "" ) { w3 = "A"    ; }
			if ( abbrev( "MODULES", w2, 3 ) )            { tbsort( MODLST, "MOD,C,"+w3 )     ; }
			else if ( abbrev( "APPLICATIONS", w2, 3 ) )  { tbsort( MODLST, "APPL,C,"+w3 )    ; }
			else if ( abbrev( "PATHS", w2, 3 ) )         { tbsort( MODLST, "MODPATH,C,"+w3 ) ; }
			else  { MSG = "PSYS01C" ; }
			continue ;
		}
		if ( ZCMD != "" ) { MSG = "PSYS018" ; continue ; }
	}
	tbend( MODLST ) ;
	return ;
}


void PPSP01A::showSavedFileList()
{
	int i ;
	string SEL   ;
	string ZFILE ;
	string PGM   ;
	string MSG   ;

	MSG = "" ;
	while ( true )
	{
		display( "PPSP01FL", MSG, "ZCMD" ) ;
		if ( RC >  8 ) { abend() ; }
		if ( RC == 8 ) { return  ; }
	
		for ( i = 1 ; i < 9 ; i++ )
		{
			vcopy( "SEL" + d2ds(i), SEL, MOVE ) ;
			if ( SEL == "" || RC == 8 ) { continue ; }
			vreplace( "SEL" + d2ds(i), "" ) ;
			vcopy( "ZFILE" + d2ds(i), ZFILE, MOVE ) ;
			if ( ZFILE == "" || RC == 8 ) { continue ; }
			if ( (SEL == "S") || (SEL == "L") )
			{
				if ( is_directory( ZFILE ) )
				{
					vcopy( "ZFLSTPGM", PGM, MOVE ) ;
					select( "PGM(" + PGM + ") PARM(" + ZFILE + ")" ) ;
				}
			}
			else if ( (SEL == "B") )
			{
				if ( is_regular_file( ZFILE ) )
				{
					browse( ZFILE ) ;
				}
			}
			else if ( (SEL == "E") )
			{
				if ( is_regular_file( ZFILE ) )
				{
					edit( ZFILE ) ;
				}
			}
		}
	}
}


void PPSP01A::showTasks()
{
	int i    ;
	int j    ;
	int retc ;

	string SEL   ;
	string USER  ;
	string PID   ;
	string CPU   ;
	string CPUX  ;
	string MEM   ;
	string MEMX  ;
	string CMD   ;
	string MSG   ;
	string TASKLST ;
	MSG = "" ;

	vdefine( "SEL USER PID CPU CPUX MEM MEMX CMD", &SEL, &USER, &PID, &CPU, &CPUX, &MEM, &MEMX, &CMD ) ;

	TASKLST = "TSKLST" + right( d2ds( taskid() ), 2, '0' ) ;

	updateTasks( TASKLST ) ;

	i = 1 ;
	while ( true )
	{
		tbtop( TASKLST )     ;
		tbskip( TASKLST, i ) ;
		ZCMD  = "" ;
		tbdispl( TASKLST, "PPSP01TK", MSG, "ZCMD" ) ;
		if ( RC  >  8 ) { abend() ; }
		if ( RC ==  8 ) { break   ; }
		MSG = ""     ;
		i   = ZTDTOP ;
		if ( ZCMD != "" ) { MSG = "PSYS018" ; continue ; }
		while ( ZTDSELS > 0 )
		{
			if ( SEL == "K")
			{
				retc = kill( ds2d(PID), 9 ) ;
				log( "I", "Kill signal sent to PID " << PID << ".  RC=" << retc << endl ) ;
			}
			if ( ZTDSELS > 1 )
			{
				tbdispl( TASKLST ) ;
				if ( RC > 4 ) break ;
			}
			else { ZTDSELS = 0 ; }
		}
		updateTasks( TASKLST ) ;
	}
	tbend( TASKLST ) ;
	vdelete( "SEL USER PID CPU CPUX MEM MEMX CMD" ) ;
}


void PPSP01A::updateTasks( string table )
{
	int p ;

	string SEL   ;
	string USER  ;
	string PID   ;
	string CPU   ;
	string MEM   ;
	string CPUX  ;
	string MEMX  ;
	string CMD   ;
	string inLine ;

	ifstream fin  ;

	vcopy( "ZUSER", ZUSER, MOVE ) ;
	vcopy( "ZSCREEN", ZSCREEN, MOVE ) ;
	
	boost::filesystem::path temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path( ZUSER + "-" + ZSCREEN + "-%%%%-%%%%" ) ;
	string tname = temp.native() ;
	string cname = "ps aux > " + tname ;

	vdefine( "SEL USER PID CPU CPUX MEM MEMX CMD", &SEL, &USER, &PID, &CPU, &CPUX, &MEM, &MEMX, &CMD ) ;

	control( "ERRORS", "RETURN" ) ;
	tbend( table ) ;
	control( "ERRORS", "CANCEL" ) ;

	tbcreate( table, "", "SEL USER PID CPU CPUX MEM MEMX CMD", NOWRITE ) ;

	system( cname.c_str() ) ;
	fin.open( tname ) ;
	while ( getline( fin, inLine ) )
	{
		USER = word( inLine, 1 ) ;
		if ( USER == "USER" ) { continue ; }
		PID  = word( inLine, 2 ) ;
		CPU  = word( inLine, 3 ) ;
		p = CPU.find( '.' )      ;
		if ( p == string::npos ) { CPUX = d2ds( ds2d( CPU ) * 10 ) ; }
		else                     { CPUX = CPU ; CPUX.erase( p, 1 ) ; }
		MEM  = word( inLine, 4 ) ;
		CMD  = strip( substr( inLine, 65 ) ) ;
		tbadd( table ) ;
	}
	fin.close() ;
	tbsort( table, "CPUX,N,D" ) ;
	tbtop( table ) ;
	vdelete( "SEL USER PID CPU CPUX MEM MEMX CMD" ) ;
	remove( tname ) ;
}


void PPSP01A::utilityPrograms()
{

	int RCode ;

	string KMAINPGM ;
	string KPANLPGM ;
	string KEDITPGM ;
	string KBRPGM   ;
	string KVIEWPGM ;
	string KFLSTPGM ;
	string KHELPPGM ;
	string KOREXPGM ;

	string v_list("ZMAINPGM ZPANLPGM ZEDITPGM ZBRPGM ZVIEWPGM ZFLSTPGM ZHELPPGM ZOREXPGM")   ;

	vdefine( v_list, &KMAINPGM, &KPANLPGM, &KEDITPGM, &KBRPGM, &KVIEWPGM, &KFLSTPGM, &KHELPPGM, &KOREXPGM ) ;
	vget( v_list, PROFILE ) ;

	while ( true )
	{
		ZCMD  = "" ;
		display( "PPSP01UP", "", "ZCMD" ) ;
		RCode = RC ;
		if (RCode > 8 ) { abend() ; }

		vget( "ZVERB", SHARED ) ;

		if ( KMAINPGM == "" ) { KMAINPGM = ZMAINPGM ; } ;
		if ( KPANLPGM == "" ) { KPANLPGM = ZPANLPGM ; } ;
		if ( KEDITPGM == "" ) { KEDITPGM = ZEDITPGM ; } ;
		if ( KBRPGM   == "" ) { KBRPGM   = ZBRPGM   ; } ;
		if ( KVIEWPGM == "" ) { KVIEWPGM = ZVIEWPGM ; } ;
		if ( KFLSTPGM == "" ) { KFLSTPGM = ZFLSTPGM ; } ;
		if ( KHELPPGM == "" ) { KHELPPGM = ZHELPPGM ; } ;
		if ( KOREXPGM == "" ) { KOREXPGM = ZOREXPGM ; } ;

		if ( ZCMD == "CANCEL" ) { break ; }
		if ( ZCMD == "DEFAULTS" )
		{
			KMAINPGM = ZMAINPGM ;
			KPANLPGM = ZPANLPGM ;
			KEDITPGM = ZEDITPGM ;
			KBRPGM   = ZBRPGM   ;
			KVIEWPGM = ZVIEWPGM ;
			KFLSTPGM = ZFLSTPGM ;
			KHELPPGM = ZHELPPGM ;
			KOREXPGM = ZOREXPGM ;
		}

		if ( RCode == 8 || ZCMD == "SAVE" )
		{
			vput( v_list, PROFILE ) ;
			if ( RCode == 8 ) { break ; }
		}
	}
	vdelete( v_list ) ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PPSP01A ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
