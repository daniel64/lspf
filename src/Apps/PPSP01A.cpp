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
/* RUN an application (default parameters)                                  */

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
#include "../classes.h"
#include "../pVPOOL.h"
#include "../pWidgets.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"

#include "PPSP01A.h"

using namespace std ;
using namespace boost::filesystem ;

#undef  MOD_NAME
#define MOD_NAME PPSP01A

boost::mutex PPSP01A::mtx ;

void PPSP01A::application()
{
	llog( "I", "Application PPSP01A starting" << endl ) ;

	set_appdesc( "General utilities to display logs, PF Key settings, variables, etc." ) ;

	string LOGTYPE ;
	string ZALOG   ;
	string ZSLOG   ;
	string LOGLOC  ;
	string w1      ;
	string w2      ;

	w1 = upper( word( PARM, 1 ) ) ;
	w2 = upper( word( PARM, 2 ) ) ;

	vdefine( "ZCMD ZVERB ZROW1 ZROW2 ZAREA ZSHADOW ZAREAT ZSCROLLA", &zcmd, &zverb, &ZROW1, &ZROW2, &ZAREA, &ZSHADOW, &ZAREAT, &ZSCROLLA ) ;
	vdefine( "ZSCROLLN ZAREAW ZAREAD", &ZSCROLLN, &ZAREAW, &ZAREAD ) ;
	vdefine( "ZALOG ZSLOG LOGTYPE LOGLOC ZCOL1", &ZALOG, &ZSLOG, &LOGTYPE, &LOGLOC, &ZCOL1 ) ;

	vget( "ZALOG ZSLOG", PROFILE ) ;

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
	else if ( w1   == "DSL"     ) { dsList( word( PARM, 2 ) ) ; }
	else if ( PARM == "GOPTS"   ) { lspfSettings()       ; }
	else if ( PARM == "KEYS"    ) { pfkeySettings()      ; }
	else if ( PARM == "COLOURS" ) { colourSettings()     ; }
	else if ( PARM == "GCL"     ) { globalColours()      ; }
	else if ( PARM == "TODO"    ) { todoList()           ; }
	else if ( w1   == "VARS"    ) { poolVariables( w2 )  ; }
	else if ( PARM == "PATHS"   ) { showPaths()          ; }
	else if ( PARM == "CMDS"    ) { showCommandTables()  ; }
	else if ( PARM == "MODS"    ) { showLoadedClasses()  ; }
	else if ( w1   == "RUN"     ) { runApplication( w2 ) ; }
	else if ( PARM == "SAVELST" ) { showSavedFileList()  ; }
	else if ( PARM == "TASKS"   ) { showTasks()          ; }
	else if ( PARM == "UTPGMS"  ) { utilityPrograms() ; }
	else if ( PARM == "KLISTS"  ) { keylistTables()   ; }
	else if ( PARM == "KLIST"   ) { keylistTable()    ; }
	else if ( PARM == "CTLKEYS" ) { controlKeys()     ; }
	else if ( w1   == "SETVAR"  )
	{
		vreplace( w2, word( PARM, 3 ) ) ;
		vput( w2, PROFILE ) ;
	}
	else { llog( "E", "Invalid parameter passed to PPSP01A: " << PARM << endl ) ; }

	cleanup() ;
	return    ;
}


void PPSP01A::show_log( const string& fileName )
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
	string ffilter ;

	filteri = ' '  ;
	filterx = ' '  ;
	ffilter = ""   ;
	firstLine = 0  ;
	maxCol    = 1  ;

	ZAHELP = "HPSP01A" ;

	vget( "ZSCRMAXW ZSCRMAXD", SHARED ) ;
	pquery( "PPSP01AL", "ZAREA", "ZAREAT", "ZAREAW", "ZAREAD" ) ;

	startCol     = 48     ;
	task         = 0      ;
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
		ZCOL1 = d2ds( startCol-47, 7 ) ;
		ZROW1 = d2ds( firstLine, 8 )   ;
		ZROW2 = d2ds( maxLines, 8 )    ;
		if ( MSG == "" ) { zcmd = "" ; }

		display( "PPSP01AL", MSG, "ZCMD" ) ;
		if ( RC == 8 ) { return ; }

		vget( "ZVERB ZSCROLLA ZSCROLLN", SHARED ) ;

		MSG   = "" ;
		Restu = subword( zcmd, 2 ) ;
		iupper( ( zcmd ) )  ;
		w1    = word( zcmd, 1 )    ;
		w2    = word( zcmd, 2 )    ;
		w3    = word( zcmd, 3 )    ;
		Rest  = subword( zcmd, 2 ) ;

		if ( file_has_changed( fileName, fsize ) )
		{
			read_file( fileName ) ;
			if ( ffilter != "" ) { find_lines( ffilter ) ; }
			rebuildZAREA = true ;
			set_excludes() ;
		}

		if ( w1 == "" ) {}
		else if ( w1 == "ONLY" || w1 == "O" )
		{
			if ( Rest.size() == 1 )
			{
				filteri = Rest[ 0 ] ;
				set_excludes() ;
				rebuildZAREA = true ;
			}
			else { MSG = "PPSP012" ; }
		}
		else if ( findword( w1, "TASK TASKID ID" ) )
		{
			if ( w2 == "" )
			{
				task = 0 ;
			}
			else
			{
				task = ds2d ( w2 ) ;
			}
			excluded.clear() ;
			for ( t = 0 ; t <= maxLines ; t++ ) { excluded.push_back( false ) ; }
			set_excludes() ;
			rebuildZAREA = true ;
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
				Xon = true     ;
				set_excludes() ;
				rebuildZAREA = true ;
			}
			else if ( Rest.size() == 1 )
			{
				filterx = Rest[ 0 ] ;
				set_excludes() ;
				rebuildZAREA = true ;
			}
			else { MSG = "PPSP013" ; }
		}
		else if ( w1 == "F" || w1 == "FIND" )
		{
			  ffilter = Restu       ;
			  find_lines( ffilter ) ;
			  rebuildZAREA = true   ;
		}
		else if ( abbrev( "RESET", w1 ) )
		{
			filteri = ' '    ;
			filterx = ' '    ;
			ffilter = ""     ;
			Xon     = false  ;
			task    = 0      ;
			rebuildZAREA = true ;
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

		if ( zverb == "DOWN" )
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
		else if ( zverb == "UP" )
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
		else if ( zverb == "LEFT" )
		{
			rebuildZAREA = true ;
			if ( ZSCROLLA == "MAX" )
			{
				startCol = 48 ;
			}
			else
			{
				startCol = startCol - ZSCROLLN ;
				if ( startCol < 48 ) { startCol = 48 ; }
			}
		}
		else if ( zverb == "RIGHT" )
		{
			rebuildZAREA = true ;
			if ( ZSCROLLA == "MAX" )
			{
				startCol = maxCol - ZAREAW + 41 ;
			}
			else
			{
				if ( ZSCROLLA == "CSR" )
				{
					if ( ZSCROLLN == lprefix )
					{
						startCol += ZAREAW - lprefix ;
					}
					else
					{
						startCol += ZSCROLLN - lprefix ;
					}
				}
				else
				{
					startCol += ZSCROLLN ;
				}
			}
			if ( startCol < 1 ) { startCol = 1 ; }
		}

		if ( rebuildZAREA ) fill_dynamic_area() ;
		rebuildZAREA = false ;
	}
}


void PPSP01A::read_file( const string& fileName )
{
	string inLine ;
	std::ifstream fin( fileName.c_str() ) ;

	data.clear()  ;

	data.push_back( centre( " Top of Log ", ZAREAW, '*' ) ) ;
	excluded.push_back( false ) ;
	while ( getline( fin, inLine ) )
	{
		data.push_back( inLine )    ;
		excluded.push_back( false ) ;
		if ( maxCol < inLine.size() ) { maxCol = inLine.size() ; }
	}
	maxCol++ ;
	data.push_back( centre( " Bottom of Log ", ZAREAW, '*' ) ) ;
	excluded.push_back( false ) ;
	maxLines = data.size() ;
	fin.close() ;
}


bool PPSP01A::file_has_changed( const string& fileName, int& fsize )
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
	int j ;
	for ( i = 1 ; i < maxLines ; i++ )
	{
		if ( task > 0 )
		{
			j = ds2d( data[ i ].substr( 39, 5 ) ) ;
			if ( j != 0 && j != task ) { excluded[ i ] = true ; continue ; }
		}
		if ( Xon )
		{
			if ( data[ i ][ 45 ] == 'D' ||
			     data[ i ][ 45 ] == 'I' ||
			     data[ i ][ 45 ] == '-' )
			{
				excluded[ i ] = true ;
				continue ;
			}
		}
		if ( data[ i ].size() > 45 )
		{
			if ( filteri != ' ' && filteri != data[ i ][ 45 ] && data[ i ][ 45 ] != '*' ) { excluded[ i ] = true ; continue ; }
			if ( filterx != ' ' && filterx == data[ i ][ 45 ] && data[ i ][ 45 ] != '*' ) { excluded[ i ] = true ; continue ; }
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

	iupper( fnd ) ;
	for ( i = 1 ; i < (maxLines-1) ; i++ )
	{
		if ( upper( data[ i ] ).find( fnd ) == string::npos ) { excluded[ i ] = true ; }
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
			if ( showDate ) { t = data[ k ].substr( 0,  12 )     ; s.replace( p, 12, 12, N_TURQ   ) ; p = 12     ; }
			if ( showTime ) { t = t + data[ k ].substr( 12, 16 ) ; s.replace( p, 16, 16, N_TURQ   ) ; p = p + 16 ; }
			if ( showMod  ) { t = t + data[ k ].substr( 28, 11 ) ; s.replace( p, 11, 11, N_YELLOW ) ; p = p + 11 ; }
			t += data[ k ].substr( 39, 8 ) ;
			t += substr( data[ k ], startCol, ZAREAW ) ;
			t.resize( ZAREAW, ' ' ) ;
			ZAREA   += t ;
			s.replace( p, 5, 5, N_WHITE ) ;
			p += 6 ;
			s.replace( p, 2, 2, N_TURQ  ) ;
			ZSHADOW += s ;
			lprefix  = p + 2 ;
		}
	}
}


void PPSP01A::dsList( string parms )
{
	// If no parms passed, show the Personal File List screen
	// If parm is a Personal File List, get list of files and invoke PFLSTPGM with LIST
	// Else assume passed parm is a path to be listed

	int i ;

	bool reflist = false ;

	string PGM ;
	string UPROF    ;
	string RFLTABLE ;

	string fname ;

	if ( parms == "" )
	{
		vcopy( "ZRFLPGM", PGM, MOVE ) ;
		select( "PGM(" + PGM + ") PARM(PL3) SCRNAME(DSLIST) SUSPEND" ) ;
	}
	else if ( parms.front() == '/' )
	{
		vcopy( "ZFLSTPGM", PGM, MOVE ) ;
		select( "PGM(" + PGM + ") PARM(BROWSE "+parms+") SCRNAME(FILES) SUSPEND" ) ;
	}
	else
	{
		vcopy( "ZUPROF", UPROF, MOVE ) ;
		vcopy( "ZRFLTBL", RFLTABLE, MOVE ) ;
		tbopen( RFLTABLE, NOWRITE, UPROF ) ;
		if ( RC == 0 )
		{
			vreplace( "ZCURTB", upper( parms ) ) ;
			tbget( RFLTABLE ) ;
			reflist = ( RC == 0 ) ;
			if ( RC == 8 )
			{
				tbsort( RFLTABLE, "(ZCURTB,C,A)" ) ;
				tbvclear( RFLTABLE ) ;
				vreplace( "ZCURTB", upper( parms )+"*" ) ;
				tbsarg( RFLTABLE, "", "NEXT", "(ZCURTB,EQ)" ) ;
				tbscan( RFLTABLE ) ;
				reflist = ( RC == 0 ) ;
				vcopy( "ZCURTB", parms, MOVE ) ;
			}
		}
		if ( reflist )
		{
			std::ofstream fout ;
			vcopy( "ZUSER", zuser, MOVE )     ;
			vcopy( "ZSCREEN", zscreen, MOVE ) ;
			boost::filesystem::path temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;
			string tname = temp.native() ;
			fout.open( tname ) ;
			for ( i = 1 ; i <= 30 ; i++ )
			{
				vcopy( "FLAPET" + d2ds( i, 2 ), fname, MOVE ) ;
				if ( fname == "" ) { continue ; }
				fout << fname << endl ;
			}
			fout.close() ;
			tbclose( RFLTABLE ) ;
			vcopy( "ZFLSTPGM", PGM, MOVE ) ;
			select( "PGM(" + PGM + ") PARM(LIST " + upper( parms ) + " " + tname + ")" ) ;
		}
		else
		{
			tbclose( RFLTABLE ) ;
			vcopy( "ZFLSTPGM", PGM, MOVE ) ;
			select( "PGM(" + PGM + ") PARM(" + parms + ")" ) ;
		}
	}
}


void PPSP01A::lspfSettings()
{
	int timeOut    ;

	string nulls   ;

	string *t1     ;

	string GODEL   ;
	string GOSWAP  ;
	string GOSWAPC ;
	string GOKLUSE ;
	string GOKLFAL ;
	string GOLMSGW ;
	string GOPADC  ;

	string RODEL   ;
	string ROSWAP  ;
	string ROSWAPC ;
	string ROKLUSE ;
	string ROKLFAL ;
	string ROLMSGW ;
	string ROPADC  ;

	string ZDEL    ;
	string ZSWAP   ;
	string ZSWAPC  ;
	string ZKLUSE  ;
	string ZKLFAIL ;
	string ZLMSGW  ;
	string ZPADC   ;

	string GOUCMD1 ;
	string GOUCMD2 ;
	string GOUCMD3 ;
	string GOSCMD1 ;
	string GOSCMD2 ;
	string GOSCMD3 ;
	string GOSTFST ;

	string ROUCMD1 ;
	string ROUCMD2 ;
	string ROUCMD3 ;
	string ROSCMD1 ;
	string ROSCMD2 ;
	string ROSCMD3 ;
	string ROSTFST ;

	string GOATIMO  ;
	string ROATIMO  ;
	string KMAXWAIT ;

	string GORTSIZE ;
	string GORBSIZE ;
	string RORTSIZE ;
	string RORBSIZE ;
	string ZRTSIZE  ;
	string ZRBSIZE  ;

	string ZUCMDT1 ;
	string ZUCMDT2 ;
	string ZUCMDT3 ;
	string ZSCMDT1 ;
	string ZSCMDT2 ;
	string ZSCMDT3 ;
	string ZSCMDTF ;

	nulls = string( 1, 0x00 ) ;

	vdefine( "ZUCMDT1 ZUCMDT2 ZUCMDT3", &ZUCMDT1, &ZUCMDT2, &ZUCMDT3 ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ZSCMDT1 ZSCMDT2 ZSCMDT3 ZSCMDTF", &ZSCMDT1, &ZSCMDT2, &ZSCMDT3, &ZSCMDTF ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "GOUCMD1 GOUCMD2 GOUCMD3", &GOUCMD1, &GOUCMD2, &GOUCMD3 ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ROUCMD1 ROUCMD2 ROUCMD3", &ROUCMD1, &ROUCMD2, &ROUCMD3 ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "GOSCMD1 GOSCMD2 GOSCMD3 GOSTFST", &GOSCMD1, &GOSCMD2, &GOSCMD3, &GOSTFST ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ROSCMD1 ROSCMD2 ROSCMD3 ROSTFST", &ROSCMD1, &ROSCMD2, &ROSCMD3, &ROSTFST ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ZKLUSE  ZKLFAIL GOKLUSE GOKLFAL", &ZKLUSE, &ZKLFAIL, &GOKLUSE, &GOKLFAL ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ROKLUSE ROKLFAL", &ROKLUSE, &ROKLFAL ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ZDEL    GODEL   ZLMSGW  GOLMSGW", &ZDEL, &GODEL, &ZLMSGW, &GOLMSGW ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "RODEL ROLMSGW", &RODEL, &ROLMSGW ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ZSWAP   GOSWAP   ZSWAPC  GOSWAPC",  &ZSWAP, &GOSWAP, &ZSWAPC, &GOSWAPC ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ROSWAP  ROSWAPC",  &ROSWAP, &ROSWAPC ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ZRTSIZE GORTSIZE ZRBSIZE GORBSIZE", &ZRTSIZE, &GORTSIZE, &ZRBSIZE, &GORBSIZE ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "RORTSIZE RORBSIZE", &RORTSIZE, &RORBSIZE ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ZMAXWAIT GOATIMO ZPADC   GOPADC", &KMAXWAIT, &GOATIMO, &ZPADC, &GOPADC ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ROATIMO ROPADC", &ROATIMO, &ROPADC ) ;
	if ( RC > 0 ) { abend() ; }

	vget( "ZUCMDT1 ZUCMDT2 ZUCMDT3", PROFILE ) ;
	if ( RC > 0 ) { abend() ; }
	vget( "ZSCMDT1 ZSCMDT2 ZSCMDT3 ZSCMDTF", PROFILE ) ;
	if ( RC > 0 ) { abend() ; }
	vget( "ZSWAP ZSWAPC ZKLUSE ZKLFAIL", PROFILE ) ;
	if ( RC > 0 ) { abend() ; }
	vget( "ZDEL ZKLUSE  ZLMSGW ZPADC", PROFILE ) ;
	if ( RC > 0 ) { abend() ; }

	ZKLUSE  == "Y" ? GOKLUSE = "/" : GOKLUSE = "" ;
	ZKLFAIL == "Y" ? GOKLFAL = "/" : GOKLFAL = "" ;
	ZSCMDTF == "Y" ? GOSTFST = "/" : GOSTFST = "" ;
	ZLMSGW  == "Y" ? GOLMSGW = "/" : GOLMSGW = "" ;
	ZSWAP   == "Y" ? GOSWAP  = "/" : GOSWAP  = "" ;

	GODEL   = ZDEL    ;
	GOSWAPC = ZSWAPC  ;
	if      ( ZPADC == " "   ) { ZPADC = "B" ; }
	else if ( ZPADC == nulls ) { ZPADC = "N" ; }
	GOPADC  = ZPADC ;

	GOUCMD1 = ZUCMDT1 ;
	GOUCMD2 = ZUCMDT2 ;
	GOUCMD3 = ZUCMDT3 ;
	GOSCMD1 = ZSCMDT1 ;
	GOSCMD2 = ZSCMDT2 ;
	GOSCMD3 = ZSCMDT3 ;

	vcopy( "ZWAIT", t1, LOCATE ) ;
	vget( "ZMAXWAIT ZRTSIZE ZRBSIZE", PROFILE ) ;

	timeOut = ds2d( *t1 ) * ds2d( KMAXWAIT ) / 1000 ;
	GOATIMO = d2ds( timeOut ) ;

	GORTSIZE = ZRTSIZE ;
	GORBSIZE = ZRBSIZE ;

	RODEL    = GODEL    ;
	ROSWAP   = GOSWAP   ;
	ROSWAPC  = GOSWAPC  ;
	ROKLUSE  = GOKLUSE  ;
	ROKLFAL  = GOKLFAL  ;
	ROLMSGW  = GOLMSGW  ;
	ROPADC   = GOPADC   ;
	ROUCMD1  = GOUCMD1  ;
	ROUCMD2  = GOUCMD2  ;
	ROUCMD3  = GOUCMD3  ;
	ROSCMD1  = GOSCMD1  ;
	ROSCMD2  = GOSCMD2  ;
	ROSCMD3  = GOSCMD3  ;
	ROSTFST  = GOSTFST  ;
	ROATIMO  = GOATIMO  ;
	RORTSIZE = GORTSIZE ;
	RORBSIZE = GORBSIZE ;

	while ( true )
	{
		zcmd = "" ;
		display( "PPSP01GO", "", "ZCMD" );
		if ( RC == 8 ) { break ; }
		if ( zcmd == "DEFAULTS" )
		{
			GOKLUSE  = ""  ;
			GOKLFAL  = "/" ;
			GOSTFST  = "/" ;
			GOLMSGW  = ""  ;
			GODEL    = ";" ;
			GOSWAP   = "/" ;
			GOSWAPC  = "'" ;
			GOPADC   = "_" ;
			GOUCMD1  = "USR" ;
			GOUCMD2  = ""  ;
			GOUCMD3  = ""  ;
			GOSCMD1  = ""  ;
			GOSCMD2  = ""  ;
			GOSCMD3  = ""  ;
			GOATIMO  = d2ds( ZMAXWAIT * ds2d( *t1 ) / 1000 ) ;
			GORTSIZE = "3"  ;
			GORBSIZE = "10" ;
		}
		else if ( zcmd == "RESET" )
		{
			GODEL    = RODEL    ;
			GOSWAP   = ROSWAP   ;
			GOSWAPC  = ROSWAPC  ;
			GOKLUSE  = ROKLUSE  ;
			GOKLFAL  = ROKLFAL  ;
			GOLMSGW  = ROLMSGW  ;
			GOPADC   = ROPADC   ;
			GOUCMD1  = ROUCMD1  ;
			GOUCMD2  = ROUCMD2  ;
			GOUCMD3  = ROUCMD3  ;
			GOSCMD1  = ROSCMD1  ;
			GOSCMD2  = ROSCMD2  ;
			GOSCMD3  = ROSCMD3  ;
			GOSTFST  = ROSTFST  ;
			GOATIMO  = ROATIMO  ;
			GORTSIZE = RORTSIZE ;
			GORBSIZE = RORBSIZE ;
		}
		GOKLUSE == "/" ? ZKLUSE  = "Y" : ZKLUSE  = "N" ;
		GOKLFAL == "/" ? ZKLFAIL = "Y" : ZKLFAIL = "N" ;
		GOSTFST == "/" ? ZSCMDTF = "Y" : ZSCMDTF = "N" ;
		GOLMSGW == "/" ? ZLMSGW  = "Y" : ZLMSGW  = "N" ;
		GOSWAP  == "/" ? ZSWAP   = "Y" : ZSWAP   = "N" ;
		ZUCMDT1 = GOUCMD1 ;
		ZUCMDT2 = GOUCMD2 ;
		ZUCMDT3 = GOUCMD3 ;
		ZSCMDT1 = GOSCMD1 ;
		ZSCMDT2 = GOSCMD2 ;
		ZSCMDT3 = GOSCMD3 ;
		vput( "ZKLUSE  ZKLFAIL ZLMSGW ZSWAP", PROFILE ) ;
		vput( "ZUCMDT1 ZUCMDT2 ZUCMDT3", PROFILE ) ;
		vput( "ZSCMDT1 ZSCMDT2 ZSCMDT3 ZSCMDTF", PROFILE ) ;
		if ( GODEL != "" && GODEL != ZDEL )
		{
			ZDEL = GODEL ;
			vput( "ZDEL", PROFILE ) ;
		}
		if ( GOSWAPC != "" && GOSWAPC != ZSWAPC )
		{
			ZSWAPC = GOSWAPC ;
			vput( "ZSWAPC", PROFILE ) ;
		}
		if ( GOPADC != ZPADC )
		{
			if      ( GOPADC == "B" ) { ZPADC = " "    ; }
			else if ( GOPADC == "N" ) { ZPADC = nulls  ; }
			else                      { ZPADC = GOPADC ; }
			vput( "ZPADC", PROFILE ) ;
			ZPADC = GOPADC ;
		}
		if ( GOATIMO != "" )
		{
			KMAXWAIT = d2ds( ds2d( GOATIMO ) * 1000 / ds2d( *t1 ) ) ;
			vput( "ZMAXWAIT", PROFILE ) ;
		}
		if ( GORTSIZE != "" )
		{
			ZRTSIZE = GORTSIZE ;
			vput( "ZRTSIZE", PROFILE ) ;
		}
		if ( GORBSIZE != "" )
		{
			ZRBSIZE = GORBSIZE ;
			vput( "ZRBSIZE", PROFILE ) ;
		}
	}
	vdelete( "ZUCMDT1 ZUCMDT2 ZUCMDT3" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "ZSCMDT1 ZSCMDT2 ZSCMDT3 ZSCMDTF" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "GOUCMD1 GOUCMD2 GOUCMD3" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "GOSCMD1 GOSCMD2 GOSCMD3 GOSTFST" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "ZKLUSE  ZKLFAIL GOKLUSE GOKLFAL" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "ZDEL    GODEL   ZLMSGW  GOLMSGW"  ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "ZSWAP   GOSWAP  ZSWAPC  GOSWAPC"  ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "ZRTSIZE GORTSIZE ZRBSIZE GORBSIZE" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "ZMAXWAIT GOATIMO ZPADC GOPADC" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "RODEL ROSWAP ROSWAPC ROKLUSE ROKLFAL ROLMSGW ROPADC ROUCMD1 ROUCMD2" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "ROUCMD3 ROSCMD1 ROSCMD2 ROSCMD3 ROSTFST ROATIMO RORTSIZE RORBSIZE" ) ;
	if ( RC > 0 ) { abend() ; }
}


void PPSP01A::controlKeys()
{
	int i ;

	string key1  ;
	string key2  ;
	string table ;
	string zcmd  ;
	string msg   ;

	string* t1   ;
	string* t2   ;

	const string alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" ;
	vector<string>oldValues ;

	vdefine( "ZCMD", &zcmd ) ;
	table = "CTKEYS" + d2ds( taskid(), 2 ) ;

	tbcreate( table, "CTKEY1", "(CTKEY2,CTACT)", NOWRITE ) ;

	for ( i = 0 ; i < 26 ; i++ )
	{
		key1 = "ZCTRL"    ;
		key2 = "Control-" ;
		key1.push_back( alpha[ i ] ) ;
		key2.push_back( alpha[ i ] ) ;
		vcopy( key1, t1, LOCATE )  ;
		oldValues.push_back( *t1 ) ;
		vreplace( "CTKEY1", key1 ) ;
		vreplace( "CTKEY2", key2 ) ;
		vreplace( "CTACT", *t1 ) ;
		tbadd( table ) ;
	}

	ZTDTOP = 1  ;
	msg    = "" ;
	while ( true )
	{
		tbtop( table ) ;
		tbskip( table, ZTDTOP ) ;
		zcmd = "" ;
		tbdispl( table, "PPSP01CT", msg, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		if ( zcmd == "RESET" || zcmd == "CANCEL" )
		{
			for ( i = 0 ; i < 26 ; i++ )
			{
				key1 = "ZCTRL" ;
				key2 = "Control-" ;
				key1.push_back( alpha[ i ] ) ;
				key2.push_back( alpha[ i ] ) ;
				vreplace( "CTKEY1", key1 ) ;
				vreplace( "CTKEY2", key2 ) ;
				vreplace( "CTACT", oldValues[ i ] ) ;
				tbmod( table ) ;
				vreplace( key1, oldValues[ i ] ) ;
				vput( key1, PROFILE ) ;
			}
			if ( zcmd == "CANCEL" ) { break ; }
			continue ;
		}
		if ( zcmd == "RESTORE" )
		{
			msg = "PPSP011E" ;
			for ( i = 0 ; i < 26 ; i++ )
			{
				key1 = "ZCTRL" ;
				key2 = "ACTRL" ;
				key1.push_back( alpha[ i ] ) ;
				key2.push_back( alpha[ i ] ) ;
				vget( key2, PROFILE ) ;
				if ( RC > 0 )
				{
					msg = "PPSP011C" ;
					break ;
				}
				vcopy( key2, t1, LOCATE ) ;
				vreplace( key1, *t1 ) ;
				vput( key1, PROFILE ) ;
				key2 = "Control-" ;
				key2.push_back( alpha[ i ] ) ;
				vreplace( "CTKEY1", key1 ) ;
				vreplace( "CTKEY2", key2 ) ;
				vreplace( "CTACT", *t1 ) ;
				tbmod( table ) ;
			}
			continue ;
		}
		while ( ZTDSELS > 0 )
		{
			vcopy( "CTKEY1", t1, LOCATE ) ;
			vcopy( "CTACT", t2, LOCATE ) ;
			vreplace( *t1, *t2 ) ;
			vput( *t1, PROFILE ) ;
			tbmod( table ) ;
			if ( ZTDSELS > 1 )
			{
				tbdispl( table ) ;
				if ( RC > 4 ) { break ; }
			}
			else { ZTDSELS = 0 ; }
		}
		if ( zcmd == "SAVE" )
		{
			for ( i = 0 ; i < 26 ; i++ )
			{
				key1 = "ZCTRL" ;
				key2 = "ACTRL" ;
				key1.push_back( alpha[ i ] ) ;
				key2.push_back( alpha[ i ] ) ;
				vget( key1, PROFILE ) ;
				vcopy( key1, t1, LOCATE ) ;
				vreplace( key2, *t1 ) ;
				vput( key2, PROFILE ) ;
			}
			msg = "PPSP011D" ;
		}
	}

	tbend( table ) ;
	vdelete( "ZCMD" ) ;
}


void PPSP01A::pfkeySettings()
{
	int RCode ;

	string* t ;

	vector<string> pfKeyDefaults = { { "HELP"      },
					 { "SPLIT NEW" },
					 { "END"       },
					 { "RETURN"    },
					 { "RFIND"     },
					 { "RCHANGE"   },
					 { "UP"        },
					 { "DOWN"      },
					 { "SWAP"      },
					 { "LEFT"      },
					 { "RIGHT"     },
					 { "RETRIEVE"  },
					 { "HELP"      },
					 { "SPLIT NEW" },
					 { "END"       },
					 { "RETURN"    },
					 { "RFIND"     },
					 { "RCHANGE"   },
					 { "UP"        },
					 { "DOWN"      },
					 { "SWAP"      },
					 { "SWAP PREV" },
					 { "SWAP NEXT" },
					 { "HELP"      } } ;

	for ( int i = 0 ; i < 24 ; i++ )
	{
		vget( "ZPF"+d2ds( i+1, 2 ), PROFILE ) ;
	}

	while ( true )
	{
		zcmd  = "" ;
		display( "PPSP01AK", "", "ZCMD" );
		RCode = RC ;

		if ( zcmd == "CANCEL" ) { break ; }
		if ( zcmd == "DEFAULTS" )
		{
			for ( int i = 0 ; i < 24 ; i++ )
			{
				vreplace( "ZPF"+d2ds( i+1, 2 ), "" ) ;
			}
		}

		for ( int i = 0 ; i < 24 ; i++ )
		{
			vcopy( "ZPF"+d2ds( i+1, 2 ), t, LOCATE ) ;
			if ( *t == "" )
			{
				vreplace( "ZPF"+d2ds( i+1, 2 ), pfKeyDefaults[ i ] ) ;
			}
		}

		if ( RCode == 8 || zcmd == "SAVE" )
		{
			for ( int i = 0 ; i < 24 ; i++ )
			{
				vput( "ZPF"+d2ds( i+1, 2 ), PROFILE ) ;
			}
			if ( RCode == 8 )  { break ; }
		}
	}
}


void PPSP01A::colourSettings()
{

	int  i ;
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

	control( "RELOAD", "CUA" ) ;

	attr1 = "" ;
	attr2 = "" ;
	attr3 = "" ;

	control( "DISPLAY", "LOCK" ) ;
	display( "PPSP01CL" )        ;

	for ( i = 1 ; i < 34 ; i++ )
	{
		if ( setScreenAttrs( VarList[ i ], i, COLOUR, INTENS, HILITE ) > 0 )
		{
			llog( "E", "ISPS variable " << "ZC" + VarList[ i ] << " not found.  Re-run setup program to create" << endl ) ;
			abend() ;
		}
	}

	for ( i = 1 ; i < 34 ; i++ )
	{
		isps_var = "ZC" + VarList[i] ;
		vcopy( isps_var, val, MOVE ) ;
		if ( RC > 0 )
		{
			llog( "E", "ISPS variable " << isps_var << " not found.  Re-run setup program to create" << endl ) ;
			abend() ;
		}
		OrigList[ i ] = val ;
	}

	MSG    = "" ;
	zcmd   = "" ;
	while ( true )
	{
		if ( MSG == "" ) { CURFLD = "ZCMD" ; }
		display( "PPSP01CL", MSG, CURFLD ) ;
		if (RC == 8 ) { cleanup() ; break  ; }

		if ( zcmd == "" ) {}
		else if ( zcmd == "CANCEL" )
		{
			for ( i = 1 ; i < 34 ; i++ )
			{
				isps_var = "ZC" + VarList[ i ] ;
				vcopy( isps_var, val, MOVE )   ;
				if ( RC > 0 )
				{
					llog( "E", "ISPS variable " << isps_var << " not found.  Re-run setup program to create" << endl ) ;
					abend() ;
				}
				setISPSVar( VarList[ i ], OrigList[ i ] ) ;
			}
			return ;
		}
		else if ( zcmd == "DEFAULTS" )
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
		else if ( zcmd == "SAVE" )
		{
			zcmd = "" ;
			for ( i = 1 ; i < 34 ; i++ )
			{
				isps_var = "ZC" + VarList[i] ;
				vcopy( isps_var, val, MOVE ) ;
				if ( RC > 0 )
				{
					llog( "E", "ISPS variable " << isps_var << " not found.  Re-run setup program to create" << endl ) ;
					abend() ;
				}
				prof_var      = isps_var  ;
				prof_var[ 0 ] = 'A'       ;
				vdefine( prof_var, &val ) ;
				vput( prof_var, PROFILE ) ;
				vdelete( prof_var ) ;
			}
			if ( RC == 0 ) { MSG = "PPSP011A" ; }
			continue ;
		}
		else if ( zcmd == "RESTORE" )
		{
			zcmd = "" ;
			for ( i = 1 ; i < 34 ; i++ )
			{
				prof_var = "AC" + VarList[i] ;
				vcopy( prof_var, val, MOVE ) ;
				if ( RC > 0 ) { MSG = "PPSP019" ; break ; }
				isps_var      = prof_var  ;
				isps_var[ 0 ] = 'Z'       ;
				vdefine( isps_var, &val ) ;
				vput( isps_var, PROFILE ) ;
				vdelete( isps_var ) ;
				if ( setScreenAttrs( VarList[i],  i, COLOUR, INTENS, HILITE ) > 0 ) { abend() ; }
			}
			if ( RC == 0 ) { MSG = "PPSP011B" ; }
			continue ;
		}

		MSG  = "" ;
		zcmd = "" ;
		for ( i = 1 ; i < 34 ; i++)
		{
			var1 = "COLOUR" + d2ds( i, 2 ) ;
			var2 = "INTENS" + d2ds( i, 2 ) ;
			var3 = "HILITE" + d2ds( i, 2 ) ;
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
				llog( "E", "ISPS variable " << isps_var << " not found.  Re-run setup program to create" << endl ) ;
				abend() ;
			}
			if ( val.size() != 3 )
			{
				llog( "E", "ISPS variable " << isps_var << " has invalid value of " << val << "  Re-run setup program to re-create" << endl ) ;
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
			if ( RC > 0 ) { llog( "E", "Colour change for field " << var1 << " has failed." << endl ) ; }
			attr( var2, attr1 + " " + attr2 ) ;
			if ( RC > 0 ) { llog( "E", "Colour/intense change for field " << var2 << " has failed." << endl ) ; }
			attr( var3, attr1 + " " + attr2 + " " + attr3 ) ;
			if ( RC > 0 ) { llog( "E", "Colour/intense/hilite change for field " << var3 << " has failed." << endl ) ; }
		}
	}
}


void PPSP01A::globalColours()
{
	int i ;

	string colour ;
	string var    ;
	string val    ;

	map<int, string> tab1 = { { 1, "B" } ,
				  { 2, "R" } ,
				  { 3, "M" } ,
				  { 4, "G" } ,
				  { 5, "T" } ,
				  { 6, "Y" } ,
				  { 7, "W" } } ;

	map<string, string> tab2 = { { "B", "BLUE"    } ,
				     { "R", "RED"     } ,
				     { "M", "MAGENTA" } ,
				     { "G", "GREEN"   } ,
				     { "T", "TURQ"    } ,
				     { "Y", "YELLOW"  } ,
				     { "W", "WHITE"   } } ;

	control( "RELOAD", "CUA" ) ;

	for ( i = 1 ; i < 8 ; i++ )
	{
		var = "ZGCL" + tab1[ i ] ;
		vcopy( var, colour, MOVE ) ;
		if ( colour == tab1[ i ] )
		{
			vreplace( "COLOUR"+ d2ds( i, 2 ), "" ) ;
		}
		else
		{
			vreplace( "COLOUR"+ d2ds( i, 2 ), tab2[ colour ] ) ;
		}
	}

	addpop( "", 5, 5 ) ;
	while ( true )
	{
		display( "PPSP01CR" ) ;
		if (RC == 8 ) { cleanup() ; break ; }
		for ( i = 1 ; i < 8 ; i++ )
		{
			vcopy( "COLOUR"+ d2ds( i, 2 ), colour, MOVE ) ;
			var = "ZGCL" + tab1[ i ] ;
			val = ( colour == "" ) ? tab1[ i ] : colour.substr( 0, 1 ) ;
			vreplace( var, val ) ;
			vput( var, PROFILE ) ;
		}
	}
	rempop() ;
}


int PPSP01A::setScreenAttrs( const string& name, int itr, string COLOUR, string INTENS, string HILITE )
{
	string t ;
	char   c ;

	string var1 ;
	string var2 ;
	string var3 ;

	vcopy( "ZC" + name, t, MOVE ) ;
	if ( RC > 0 ) { llog( "E", "Variable ZC" << name << " not found in ISPS profile" << endl ) ; return 8 ; }
	else
	{
		if ( t.size() != 3 ) { llog( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ; return 8 ; }
		c = t[ 0 ] ;
		if      ( c == 'R' ) COLOUR = "RED"     ;
		else if ( c == 'G' ) COLOUR = "GREEN"   ;
		else if ( c == 'Y' ) COLOUR = "YELLOW"  ;
		else if ( c == 'B' ) COLOUR = "BLUE"    ;
		else if ( c == 'M' ) COLOUR = "MAGENTA" ;
		else if ( c == 'T' ) COLOUR = "TURQ"    ;
		else if ( c == 'W' ) COLOUR = "WHITE"   ;
		else { llog( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ; }
		c = t[ 1 ] ;
		if      ( c == 'H' ) INTENS = "HIGH" ;
		else if ( c == 'L' ) INTENS = "LOW" ;
		else { llog( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ; }
		c = t[ 2 ] ;
		if      ( c == 'N' ) HILITE = "NONE"    ;
		else if ( c == 'B' ) HILITE = "BLINK"   ;
		else if ( c == 'R' ) HILITE = "REVERSE" ;
		else if ( c == 'U' ) HILITE = "USCORE"  ;
		else { llog( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ; }
	}

	var1 = "COLOUR" + d2ds( itr, 2 ) ;
	var2 = "INTENS" + d2ds( itr, 2 ) ;
	var3 = "HILITE" + d2ds( itr, 2 ) ;

	attr( var1, "COLOUR(" + COLOUR + ")" ) ;
	if ( RC > 0 ) { llog( "E", "Colour change for field " << var1 << " has failed." << endl ) ; }

	attr( var2, "COLOUR(" + COLOUR + ") INTENSE(" + INTENS + ")" ) ;
	if ( RC > 0 ) { llog( "E", "Colour/intense change for field " << var2 << " has failed." << endl ) ; }

	attr( var3,  "COLOUR(" + COLOUR + ") INTENSE(" + INTENS + ") HILITE(" + HILITE + ")" ) ;
	if ( RC > 0 ) { llog( "E", "Colour/intense/hilite change for field " << var3 << " has failed." << endl ) ; }

	vreplace( var1, COLOUR ) ;
	vreplace( var2, INTENS ) ;
	vreplace( var3, HILITE ) ;
	return 0 ;
}


void PPSP01A::setISPSVar( const string& var, string val )
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
	addpop( "", 5, 5 ) ;
	while ( true )
	{
		display( "PPSP01TD", "", "ZCMD" );
		if ( RC == 8 ) { break ; }
	}
	rempop() ;
	return ;
}


void PPSP01A::poolVariables( const string& applid )
{
	string MSG  ;
	string cw   ;
	string w2   ;

	vcopy( "ZAPPLID", zapplid, MOVE ) ;
	if ( applid != "" && zapplid != applid )
	{
		if ( !isvalidName4( applid ) ) { return ; }
		select( "PGM(PPSP01A) PARM(VARS) NEWAPPL(" + applid + ")" ) ;
		return ;
	}

	VARLST = "VARLST" + d2ds( taskid(), 2 ) ;

	vdefine( "SEL VAR VPOOL VPLVL VAL MESSAGE", &SEL, &VAR, &VPOOL, &VPLVL, &VAL, &MESSAGE ) ;

	getpoolVariables( "" ) ;

	MSG    = "" ;
	ZTDTOP = 1  ;
	while ( true )
	{
		tbtop( VARLST )  ;
		tbskip( VARLST, ZTDTOP ) ;
		if ( MSG == "" ) { zcmd = "" ; }
		tbdispl( VARLST, "PPSP01AV", MSG, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		if ( zcmd == "REF" || zcmd == "RES" )
		{
			tbend( VARLST ) ;
			getpoolVariables( "" ) ;
			continue ;
		}
		cw = word( zcmd, 1 ) ;
		w2 = word( zcmd, 2 ) ;
		if ( cw == "O" )
		{
			tbend( VARLST ) ;
			getpoolVariables( w2 ) ;
			continue ;
		}
		if ( zcmd != "" ) { MSG = "PSYS018" ; continue ; }
		while ( ZTDSELS > 0 )
		{
			if ( SEL == "D" )
			{
				control( "ERRORS", "RETURN" ) ;
				if ( VPOOL == "S" ) { verase( VAR, SHARED  ) ; }
				else                { verase( VAR, PROFILE ) ; }
				MESSAGE = "*Delete RC=" + d2ds(RC) + "*" ;
				control( "ERRORS", "CANCEL" ) ;
				SEL = "" ;
			}
			tbput( VARLST ) ;
			if ( ZTDSELS > 1 )
			{
				tbdispl( VARLST ) ;
				if ( RC > 4 ) { break ; }
			}
			else { ZTDSELS = 0 ; }
		}
	}

	tbend( VARLST ) ;
}


void PPSP01A::getpoolVariables( const string& pattern )
{
	// SHARED 1  - shared variable pool
	// SHARED 2  - default variable pool (@DEFSHAR)
	// PROFILE 1 - application variable pool
	// PROFILE 2 - Read-only extention pool
	// PROFILE 3 - default read-only profile pool (@DEFPROF)
	// PROFILE 4 - System profile (ISPSPROF)

	int i  ;
	int ws ;

	string varlist ;

	tbcreate( VARLST, "", "(SEL,VAR,VPOOL,VPLVL,MESSAGE,VAL)", NOWRITE ) ;

	SEL     = "" ;
	MESSAGE = "" ;

	set<string> found ;
  /*    varlist = vilist( DEFINED ) + vslist( DEFINED ) ;
	VPOOL = "F" ;
	VPLVL = "D" ;
	ws    = words( varlist ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		VAR = word( varlist, i ) ;
		if ( (pattern != "") && (pos( pattern, VAR ) == 0) ) { continue ; }
		vcopy( VAR, VAL, MOVE ) ;
		tbadd( VARLST )    ;
	}

	varlist = vilist( IMPLICIT ) + vslist( IMPLICIT ) ;
	VPOOL = "F" ;
	VPLVL = "I" ;
	ws    = words( varlist ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		VAR = word( varlist, i ) ;
		if ( (pattern != "") && (pos( pattern, VAR ) == 0) ) { continue ; }
		vcopy( VAR, VAL, MOVE ) ;
		tbadd( VARLST )    ;
	}
	*/
	found.clear() ;
	varlist = vlist( SHARED, 1 ) ;
	VPOOL   = "S" ;
	VPLVL   = "1" ;
	for ( ws = words( varlist ), i = 1 ; i <= ws ; i++ )
	{
		VAR = word( varlist, i ) ;
		if ( pattern != "" && pos( pattern, VAR ) == 0 ) { continue ; }
		vget( VAR, SHARED ) ;
		vcopy( VAR, VAL, MOVE ) ;
		tbadd( VARLST )     ;
		found.insert( VAR ) ;
	}

	varlist = vlist( SHARED, 2 ) ;
	VPLVL   = "2" ;
	for ( ws = words( varlist ), i = 1 ; i <= ws ; i++ )
	{
		VAR = word( varlist, i ) ;
		if ( found.count( VAR ) > 0 ) { continue ; }
		if ( pattern != "" && pos( pattern, VAR ) == 0 ) { continue ; }
		vget( VAR, SHARED ) ;
		vcopy( VAR, VAL, MOVE ) ;
		tbadd( VARLST )     ;
		found.insert( VAR ) ;
	}

	found.clear() ;
	VPOOL   = "P" ;
	VPLVL   = "1" ;
	varlist = vlist( PROFILE, 1 ) ;
	for ( ws = words( varlist ), i = 1 ; i <= ws ; i++ )
	{
		VAR = word( varlist, i ) ;
		if ( pattern != "" && pos( pattern, VAR ) == 0 ) { continue ; }
		vget( VAR, PROFILE ) ;
		vcopy( VAR, VAL, MOVE ) ;
		tbadd( VARLST )     ;
		found.insert( VAR ) ;
	}

	varlist = vlist( PROFILE, 2 ) ;
	VPLVL   = "2"                 ;
	for ( ws = words( varlist ), i = 1 ; i <= ws ; i++ )
	{
		VAR = word( varlist, i ) ;
		if ( found.count( VAR ) > 0 ) { continue ; }
		if ( pattern != "" && pos( pattern, VAR ) == 0 ) { continue ; }
		vget( VAR, PROFILE ) ;
		vcopy( VAR, VAL, MOVE ) ;
		tbadd( VARLST )     ;
		found.insert( VAR ) ;
	}

	varlist = vlist( PROFILE, 3 ) ;
	VPLVL   = "3"                 ;
	for ( ws = words( varlist ), i = 1 ; i <= ws ; i++ )
	{
		VAR = word( varlist, i ) ;
		if ( found.count( VAR ) > 0 ) { continue ; }
		if ( pattern != "" && pos( pattern, VAR ) == 0 ) { continue ; }
		vget( VAR, PROFILE ) ;
		vcopy( VAR, VAL, MOVE ) ;
		tbadd( VARLST )     ;
		found.insert( VAR ) ;
	}

	varlist = vlist( PROFILE, 4 ) ;
	VPLVL   = "4"                 ;
	for ( ws = words( varlist ), i = 1 ; i <= ws ; i++ )
	{
		VAR = word( varlist, i ) ;
		if ( found.count( VAR ) > 0 ) { continue ; }
		if ( pattern != "" && pos( pattern, VAR ) == 0 ) { continue ; }
		vget( VAR, PROFILE ) ;
		vcopy( VAR, VAL, MOVE ) ;
		tbadd( VARLST )      ;
	}

	tbtop( VARLST ) ;
}


void PPSP01A::showPaths()
{
	int i ;

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

	if ( libdef_muser ) { LIBDEFM = "LIBDEF active for user message search"     ; }
	else                { LIBDEFM = "LIBDEF not active for user message search" ; }
	if ( libdef_puser ) { LIBDEFP = "LIBDEF active for user panel search"       ; }
	else                { LIBDEFP = "LIBDEF not active for user panel search"   ; }
	if ( libdef_muser ) { LIBDEFT = "LIBDEF active for user table search"       ; }
	else                { LIBDEFT = "LIBDEF not active for user table search"   ; }

	PATHLST = "PTHLST" + d2ds( taskid(), 2 ) ;

	vdefine( "SEL PVAR PATH MESSAGE DESCRIPT", &SEL, &PVAR, &PATH, &MESSAGE, &DESCRIPT ) ;

	tbcreate( PATHLST, "", "(SEL,PVAR,PATH,MESSAGE,DESCRIPT)", NOWRITE ) ;

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

   /*   if ( libdef_muser )
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
	}   */

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

  /*    if ( libdef_puser )
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
	}  */

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

  /*    if ( libdef_tuser )
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
	}  */

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
	PATH     =  ZUPROF  ;
	DESCRIPT = "User home profile path" ;
	tbadd( PATHLST )    ;

	PVAR     = "ZORXPATH" ;
	DESCRIPT = "Object REXX EXEC search path" ;
	for ( i = 1 ; i <= getpaths( ZORXPATH ) ; i++)
	{
		PATH = getpath( ZORXPATH, i ) ;
		MESSAGE = "" ;
		if ( !is_directory( PATH ) ) { MESSAGE = "Path not found" ; }
		tbadd( PATHLST ) ;
		DESCRIPT = "" ;
		PVAR     = "" ;
	}

	tbtop( PATHLST ) ;
	MSG = "" ;
	ZTDTOP = 1 ;
	while ( true )
	{
		tbtop( PATHLST ) ;
		tbskip( PATHLST, ZTDTOP ) ;
		if ( MSG == "" ) { zcmd  = "" ; }
		tbdispl( PATHLST, "PPSP01AP", MSG, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		MSG = ""     ;
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
				else MESSAGE = "*Error*" ;
			}
			SEL = ""         ;
			tbput( PATHLST ) ;
			if ( ZTDSELS > 1 )
			{
				tbdispl( PATHLST ) ;
				if ( RC > 4 ) { break ; }
			}
			else { ZTDSELS = 0 ; }
		}
	}
	tbclose( PATHLST ) ;
}


void PPSP01A::showCommandTables()
{
	string CMDTAB   ;
	string OCMDTAB  ;
	string ZCTVERB  ;
	string ZCTTRUNC ;
	string ZCTACT   ;
	string ZCTDESC  ;
	string MSG      ;
	string APPLCMD  ;
	string APPLCMDL ;
	string panel    ;

	vdefine( "ZCTVERB ZCTTRUNC ZCTACT ZCTDESC", &ZCTVERB, &ZCTTRUNC, &ZCTACT, &ZCTDESC ) ;
	vdefine( "ZAPPLID CMDTAB APPLCMD APPLCMDL ZVERB", &zapplid, &CMDTAB, &APPLCMD,  &APPLCMDL, &zverb ) ;

	vget( "ZAPPLID" ) ;
	vget( "CMDTAB", PROFILE ) ;
	if ( CMDTAB == "" ) { CMDTAB = "ISP" ; }
	OCMDTAB = CMDTAB ;

	APPLCMD  = "" ;
	APPLCMDL = "" ;
	if ( zapplid != "ISP" )
	{
		APPLCMD = zapplid ;
		tbopen( APPLCMD+"CMDS", NOWRITE, "", SHARE ) ;
		if ( RC > 4 )
		{
			APPLCMDL = "Application Command Table Not Found" ;
		}
		else
		{
			APPLCMDL = "" ;
			tbend( APPLCMD+"CMDS" ) ;
		}
	}
	MSG = "" ;

	tbopen( CMDTAB+"CMDS", NOWRITE, "", SHARE ) ;
	if ( RC > 0 )
	{
		CMDTAB = "ISP" ;
		tbopen( CMDTAB+"CMDS", NOWRITE, "", SHARE ) ;
	}
	ZTDTOP = 1 ;
	panel = "PPSP01AC" ;
	control( "PASSTHRU", "LRSCROLL", "PASON" ) ;
	while ( true )
	{
		tbtop( CMDTAB+"CMDS" ) ;
		tbskip( CMDTAB+"CMDS", ZTDTOP ) ;
		zcmd  = "" ;
		tbdispl( CMDTAB+"CMDS", panel, MSG, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		if ( OCMDTAB != CMDTAB )
		{
			tbopen( CMDTAB+"CMDS", NOWRITE, "", SHARE ) ;
			if ( RC == 0 )
			{
				tbend( OCMDTAB+"CMDS" ) ;
				OCMDTAB = CMDTAB ;
			}
			else
			{
				CMDTAB = OCMDTAB ;
				MSG = "PPSP014"  ;
			}
		}
		vget( "ZVERB", SHARED ) ;
		if ( zverb == "LEFT" || zverb == "RIGHT" )
		{
			panel == "PPSP01AC" ? panel = "PPSP01AD" : panel = "PPSP01AC" ;
		}
	}
	tbend( CMDTAB+"CMDS" ) ;
	vput( "CMDTAB", PROFILE ) ;

}


void PPSP01A::showLoadedClasses()
{
	int j      ;
	int ws     ;

	bool ref   ;

	string w1  ;
	string w2  ;
	string w3  ;
	string MSG ;
	string SEL ;
	string STATUS ;
	string psort  ;

	lspfCommand lc ;

	vdefine( "SEL APPL MOD MODPATH STATUS", &SEL, &APPL, &MOD, &MODPATH, &STATUS ) ;

	MODLST = "MODLST" + d2ds( taskid(), 2 ) ;

	MSG    = ""   ;
	ZTDTOP = 1    ;
	ref    = true ;
	psort  = "(APPL,C,A)" ;
	while ( true )
	{
		if ( ref )
		{
			tbcreate( MODLST, "APPL", "(SEL,MOD,MODPATH,STATUS)", NOWRITE ) ;
			tbsort( MODLST, psort ) ;
			lc.Command = "MODULE STATUS" ;
			lspfCallback( lc ) ;
			for ( j = 0 ; j < lc.reply.size() ; j++ )
			{
				SEL     = ""              ;
				APPL    = lc.reply[   j ] ;
				MOD     = lc.reply[ ++j ] ;
				MODPATH = lc.reply[ ++j ] ;
				MODPATH = substr( MODPATH, 1, (lastpos( "/", MODPATH ) - 1) ) ;
				STATUS  = lc.reply[ ++j ] ;
				tbadd( MODLST, "", "ORDER" ) ;
			}
			ref = false ;
		}
		tbtop( MODLST ) ;
		tbskip( MODLST, ZTDTOP ) ;
		if ( MSG == "" ) { zcmd = "" ; }
		tbdispl( MODLST, "PPSP01ML", MSG, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		if ( ZTDSELS == 0 && zcmd == "" ) { ref = true ; }
		ws = words( zcmd )   ;
		w1 = word( zcmd, 1 ) ;
		w2 = word( zcmd, 2 ) ;
		w3 = word( zcmd, 3 ) ;
		if ( w1 == "SORT" )
		{
			if ( w2 == "" ) { w2 = "APPL" ; }
			if ( w3 == "" ) { w3 = "A"    ; }
			if      ( abbrev( "MODULES", w2, 3 ) )      { psort = "MOD,C,"+w3     ; }
			else if ( abbrev( "APPLICATIONS", w2, 3 ) ) { psort = "APPL,C,"+w3    ; }
			else if ( abbrev( "PATHS", w2, 3 ) )        { psort = "MODPATH,C,"+w3 ; }
			else if ( abbrev( "STATUS", w2, 3 ) )       { psort = "STATUS,C,"+w3  ; }
			else                                        { MSG = "PSYS011C" ; continue ; }
			tbsort( MODLST, psort ) ;
			continue ;
		}
		while ( ZTDSELS > 0 )
		{
			if ( SEL == "R" )
			{
				lc.Command = "MODREL " + APPL ;
				lspfCallback( lc ) ;
			}
			if ( ZTDSELS > 1 )
			{
				tbdispl( MODLST ) ;
				if ( RC > 4 ) { break ; }
			}
			else { ZTDSELS = 0 ; }
		}
		if ( ref ) { tbend( MODLST ) ; }
	}
	tbend( MODLST ) ;
	return ;
}


void PPSP01A::showSavedFileList()
{
	int i ;
	string SEL   ;
	string ZFILE ;
	string ZFILN ;
	string ZCURR ;
	string ZDIR  ;
	string PGM   ;
	string MSG   ;

	vdefine( "ZCURR ZFILE ZDIR", &ZCURR, &ZFILE, &ZDIR ) ;

	MSG = "" ;
	while ( true )
	{
		display( "PPSP01FL", MSG, "ZCMD" ) ;
		if ( RC == 8 ) { return ; }

		if ( ZFILE != "" )
		{
			if ( ZFILE == "*" ) { ZFILE = "" ; }
			if ( ZFILE != "" && ZFILE[ 0 ] == '/' )
			{
				 ZFILN = ZFILE ;
			}
			else if ( ZDIR != "" )
			{
				ZFILN = ZDIR + "/" + ZFILE ;
			}
			else if ( ZCURR != "" )
			{
				ZFILN = ZCURR + "/" + ZFILE ;
			}
			else { continue ; }
			if ( is_directory( ZFILN ) )
			{
				vcopy( "ZFLSTPGM", PGM, MOVE ) ;
				select( "PGM(" + PGM + ") PARM(" + ZFILN + ")" ) ;
			}
			else if ( is_regular_file( ZFILN ) )
			{
				browse( ZFILN ) ;
			}
			continue ;
		}
		for ( i = 1 ; i < 9 ; i++ )
		{
			vcopy( "SEL" + d2ds(i), SEL, MOVE ) ;
			if ( SEL == "" || RC == 8 ) { continue ; }
			vreplace( "SEL" + d2ds(i), "" ) ;
			vcopy( "ZFILE" + d2ds(i), ZFILN, MOVE ) ;
			if ( ZFILN == "" || RC == 8 ) { continue ; }
			if ( (SEL == "S") || (SEL == "L") )
			{
				if ( is_directory( ZFILN ) )
				{
					vcopy( "ZFLSTPGM", PGM, MOVE ) ;
					select( "PGM(" + PGM + ") PARM(" + ZFILN + ")" ) ;
				}
			}
			else if ( SEL == "B" )
			{
				if ( is_regular_file( ZFILN ) )
				{
					browse( ZFILN ) ;
				}
			}
			else if ( SEL == "E" )
			{
				if ( is_regular_file( ZFILN ) )
				{
					edit( ZFILN ) ;
				}
			}
		}
	}
}


void PPSP01A::showTasks()
{
	int retc ;

	string of    ;
	string uf    ;
	string ZCMD  ;
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

	TASKLST = "TSKLST" + d2ds( taskid(), 2 ) ;

	updateTasks( TASKLST ) ;

	ZTDTOP = 1 ;
	while ( true )
	{
		tbtop( TASKLST ) ;
		tbskip( TASKLST, ZTDTOP ) ;
		ZCMD  = "" ;
		tbdispl( TASKLST, "PPSP01TK", MSG, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		vcopy( "USERF", uf, MOVE ) ;
		vcopy( "ONLYF", of, MOVE ) ;
		while ( ZTDSELS > 0 )
		{
			if ( SEL == "K")
			{
				retc = kill( ds2d(PID), 9 ) ;
				llog( "I", "Kill signal sent to PID " << PID << ".  RC=" << retc << endl ) ;
			}
			else if ( SEL == "S")
			{
				select( "PGM(PCMD0A) PARM( systemctl status "+ PID +" )" ) ;
			}
			if ( ZTDSELS > 1 )
			{
				tbdispl( TASKLST ) ;
				if ( RC > 4 ) break ;
			}
			else { ZTDSELS = 0 ; }
		}
		tbend( TASKLST ) ;
		updateTasks( TASKLST, uf, of ) ;
	}
	tbend( TASKLST ) ;
	vdelete( "SEL USER PID CPU CPUX MEM MEMX CMD" ) ;
}


void PPSP01A::updateTasks( const string& table, const string& uf, const string& of )
{
	// Columns for top: PID, user, priority, NI, virt, RES, Size, Status, %CPU, %MEM, time, Command

	// Procedure can use 'top' or 'ps'
	// top gives a better representation of CPU percentage but there is a delay

	int p ;

	string inLine ;
	string PID    ;
	string CMD    ;
	string USER   ;
	string CPU    ;
	string CPUX   ;
	string tname  ;
	string cname  ;

	enum Columns
	{
		C_PID = 1,
		C_USER,
		C_PRI,
		C_NI,
		C_VIRT,
		C_RES,
		C_SIZE,
		C_STATUS,
		C_CPU,
		C_MEM,
		C_TIME,
		C_CMD
	} ;

	std::ifstream fin ;

	vcopy( "ZUSER", zuser, MOVE )     ;
	vcopy( "ZSCREEN", zscreen, MOVE ) ;

	boost::filesystem::path temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;
	tname = temp.native() ;

  //    cname = "ps ax -o pid,user,pri,ni,vsz,drs,size,stat,%cpu,%mem,time,cmd > " + tname ;
	cname = "top -b -n 1 > " + tname ;

	tbcreate( table, "", "(SEL,USER,PID,CPU,CPUX,MEM,MEMX,CMD)", NOWRITE ) ;

	system( cname.c_str() ) ;
	fin.open( tname ) ;

	while ( getline( fin, inLine ) )
	{
		tbvclear( table ) ;
		PID = word( inLine, C_PID ) ;
		if ( !datatype( PID, 'W' ) ) { continue ; }
		vreplace( "PID", PID )   ;
		USER = word( inLine, C_USER ) ;
		if ( uf != "" && uf != upper( USER ) ) { continue ; }
		vreplace( "USER", USER ) ;
		CPU  = word( inLine, C_CPU ) ;
		vreplace( "CPU", CPU ) ;
		p = CPU.find( '.' )      ;
		if ( p == string::npos ) { CPUX = d2ds( ds2d( CPU ) * 10 ) ; }
		else                     { CPUX = CPU ; CPUX.erase( p, 1 ) ; }
		vreplace( "CPUX", CPUX ) ;
		vreplace( "MEM", word( inLine, C_MEM ) ) ;
		CMD = subword( inLine, C_CMD ) ;
		if ( of != "" && pos( of, upper( CMD ) ) == 0 ) { continue ; }
		trim_left( CMD ) ;
		CMD = strip( CMD, 'L', '`' ) ;
		CMD = strip( CMD, 'L', '-' ) ;
		trim_left( CMD ) ;
		vreplace( "CMD", CMD ) ;
		tbadd( table ) ;
	}
	fin.close() ;
	tbsort( table, "(CPUX,N,D)" ) ;
	tbtop( table ) ;
	remove( tname ) ;
}


void PPSP01A::utilityPrograms()
{

	int RCode ;

	string KMAINPGM ;
	string KMAINPAN ;
	string KPANLPGM ;
	string KEDITPGM ;
	string KBRPGM   ;
	string KVIEWPGM ;
	string KFLSTPGM ;
	string KHELPPGM ;
	string KOREXPGM ;

	string v_list1 = "ZMAINPGM ZMAINPAN ZPANLPGM ZEDITPGM ZBRPGM ZVIEWPGM ZFLSTPGM ZHELPPGM" ;
	string v_list2 = "ZOREXPGM" ;

	vdefine( v_list1, &KMAINPGM, &KMAINPAN, &KPANLPGM, &KEDITPGM, &KBRPGM, &KVIEWPGM, &KFLSTPGM, &KHELPPGM ) ;
	vget( v_list1, PROFILE ) ;

	vdefine( v_list2, &KOREXPGM ) ;
	vget( v_list2, PROFILE ) ;

	while ( true )
	{
		zcmd = "" ;
		display( "PPSP01UP", "", "ZCMD" ) ;
		RCode = RC ;

		if ( KMAINPGM == "" ) { KMAINPGM = ZMAINPGM ; } ;
		if ( KMAINPAN == "" ) { KMAINPAN = ZMAINPAN ; } ;
		if ( KPANLPGM == "" ) { KPANLPGM = ZPANLPGM ; } ;
		if ( KEDITPGM == "" ) { KEDITPGM = ZEDITPGM ; } ;
		if ( KBRPGM   == "" ) { KBRPGM   = ZBRPGM   ; } ;
		if ( KVIEWPGM == "" ) { KVIEWPGM = ZVIEWPGM ; } ;
		if ( KFLSTPGM == "" ) { KFLSTPGM = ZFLSTPGM ; } ;
		if ( KHELPPGM == "" ) { KHELPPGM = ZHELPPGM ; } ;
		if ( KOREXPGM == "" ) { KOREXPGM = ZOREXPGM ; } ;

		if ( zcmd == "CANCEL" ) { break ; }
		if ( zcmd == "DEFAULTS" )
		{
			KMAINPGM = ZMAINPGM ;
			KMAINPAN = ZMAINPAN ;
			KPANLPGM = ZPANLPGM ;
			KEDITPGM = ZEDITPGM ;
			KBRPGM   = ZBRPGM   ;
			KVIEWPGM = ZVIEWPGM ;
			KFLSTPGM = ZFLSTPGM ;
			KHELPPGM = ZHELPPGM ;
			KOREXPGM = ZOREXPGM ;
		}

		if ( RCode == 8 || zcmd == "SAVE" )
		{
			vput( v_list1, PROFILE ) ;
			vput( v_list2, PROFILE ) ;
			if ( RCode == 8 ) { break ; }
		}
	}
	vdelete( v_list1 ) ;
	vdelete( v_list2 ) ;
}

void PPSP01A::keylistTables()
{
	// Show a list of all key list tables in the ZUPROF path
	// If there are no tables found, create an empty ISPKEYP

	int RC1     ;

	string MSG  ;
	string cw   ;
	string w2   ;
	string tab  ;
	string fname;
	string p    ;

	string TBK1SEL ;
	string TBK1TAB ;
	string TBK1TYP ;
	string TBK1MSG ;
	string KEYP    ;
	string UPROF   ;
	string NEWTAB  ;

	string AKTAB   ;
	string AKLIST  ;

	typedef vector<path> vec ;
	vec v ;

	vec::const_iterator it ;

	vdefine( "TBK1SEL TBK1TAB TBK1TYP TBK1MSG NEWTAB", &TBK1SEL, &TBK1TAB, &TBK1TYP, &TBK1MSG, &NEWTAB ) ;
	KEYP = "KEYP" + d2ds( taskid(), 4 ) ;

	tbcreate( KEYP, "", "(TBK1SEL,TBK1TAB,TBK1TYP,TBK1MSG)", NOWRITE ) ;
	if ( RC > 0 ) { abend() ; }

	tbsort( KEYP, "(TBK1TAB,C,A)" ) ;

	vcopy( "ZUPROF", UPROF, MOVE ) ;
	vcopy( "ZKLNAME", AKLIST, MOVE ) ;
	vcopy( "ZKLAPPL", AKTAB, MOVE ) ;

	copy( directory_iterator( UPROF ), directory_iterator(), back_inserter( v ) ) ;

	for ( it = v.begin() ; it != v.end() ; ++it )
	{
		fname = (*it).string() ;
		p     = substr( fname, 1, (lastpos( "/", fname ) - 1) ) ;
		tab   = substr( fname, (lastpos( "/", fname ) + 1) )    ;
		if ( tab.size() < 5 ) { continue ; }
		if ( tab.compare( tab.size()-4, 4, "KEYP" ) == 0 )
		{
			tbvclear( KEYP ) ;
			TBK1TAB = tab    ;
			if ( TBK1TAB == AKTAB+"KEYP" )
			{
				TBK1MSG = "*Active*" ;
			}
			TBK1TYP = "Private" ;
			tbadd( KEYP, "", "ORDER" ) ;
		}
	}

	tbtop( KEYP )     ;
	tbskip( KEYP, 1 ) ;
	if ( RC == 8 )
	{
		NEWTAB = "ISP"   ;
		createKeyTable( NEWTAB ) ;
		tbvclear( KEYP ) ;
		TBK1TAB = NEWTAB+"KEYP" ;
		tbadd( KEYP )    ;
	}

	MSG    = "" ;
	ZTDTOP = 1  ;
	while ( true )
	{
		tbtop( KEYP )     ;
		tbskip( KEYP, ZTDTOP ) ;
		if ( MSG == "" ) { zcmd = "" ; }
		tbdispl( KEYP, "PPSP01K1", MSG, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		while ( ZTDSELS > 0 )
		{
			if ( TBK1SEL == "D" )
			{
				addpop( "", 5, 5 ) ;
				display( "PPSP01K7", MSG, "ZCMD" ) ;
				RC1 = RC ;
				rempop() ;
				if ( RC1 == 0 )
				{
					remove( UPROF + "/" + TBK1TAB ) ;
					tbdelete( KEYP ) ;
					if ( RC > 0 ) { abend() ; }
				}
			}
			else if ( TBK1SEL == "N" )
			{
				addpop( "", 5, 5 ) ;
				display( "PPSP01K5", MSG, "ZCMD" ) ;
				if ( RC == 0 )
				{
					TBK1TAB = NEWTAB + "KEYP" ;
					TBK1MSG = "*Added*" ;
					tbadd( KEYP, "", "ORDER" ) ;
					createKeyTable( NEWTAB ) ;
					TBK1SEL = ""  ;
					tbput( KEYP ) ;
				}
				rempop() ;
			}
			else if ( TBK1SEL == "S" )
			{
				control( "DISPLAY", "SAVE" ) ;
				keylistTable( TBK1TAB, AKTAB, AKLIST ) ;
				control( "DISPLAY", "RESTORE" ) ;
				TBK1MSG = "*Selected*" ;
				TBK1SEL = ""  ;
				tbput( KEYP ) ;
			}
			if ( ZTDSELS > 1 )
			{
				tbdispl( KEYP ) ;
				if ( RC > 4 ) { break ; }
			}
			else { ZTDSELS = 0 ; }
		}
	}
	vdelete( "TBK1SEL TBK1TAB TBK1TYP TBK1MSG NEWTAB" ) ;
	return ;
}


void PPSP01A::keylistTable( string tab, string AKTAB, string AKLIST )
{
	// Show keylist table (default current profile)
	// If there are no rows in table tab, create an empty ISPDEF entry

	int i ;
	int RC1 ;

	string MSG  ;
	string cw   ;
	string w2   ;
	string fname;
	string p    ;
	string t    ;

	string TBK2SEL  ;
	string TBK2LST  ;
	string TBK2MSG  ;
	string KEYLISTN ;
	string KLST     ;
	string UPROF    ;
	string NEWKEY   ;

	bool   actTab   ;

	if ( AKTAB == "" )
	{
		vcopy( "ZKLNAME", AKLIST, MOVE ) ;
		vcopy( "ZKLAPPL", AKTAB, MOVE ) ;
		actTab = true   ;
		if ( AKTAB == "" )
		{
			vcopy( "ZAPPLID", AKTAB, MOVE ) ;
			actTab = false ;
		}
	}
	else
	{
		actTab = ( AKTAB+"KEYP" == tab ) ;
	}

	if ( tab == "" )
	{
		tab = AKTAB+"KEYP" ;
		vreplace( "TBK1TAB", tab ) ;
	}

	vdefine( "TBK2SEL TBK2LST TBK2MSG KEYLISTN NEWKEY", &TBK2SEL, &TBK2LST, &TBK2MSG, &KEYLISTN, &NEWKEY ) ;
	KLST = "KLT2" + d2ds( taskid(), 4 ) ;
	vcopy( "ZUPROF", UPROF, MOVE ) ;

	tbopen( tab, NOWRITE, UPROF ) ;
	if ( RC > 0 )
	{
		llog( "E", "Error opening Keylist table "<< tab << ".  RC="<< RC << endl ) ;
		abend() ;
	}

	tbcreate( KLST, "", "(TBK2SEL,TBK2LST,TBK2MSG)", NOWRITE ) ;
	if ( RC > 0 )
	{
		llog( "E", "Error creating Keylist table "<< KLST << ".  RC="<< RC << endl ) ;
		abend() ;
	}

	tbtop( tab ) ;
	tbskip( tab, 1 ) ;
	if ( RC > 0 )
	{
		tbend( tab ) ;
		tbopen( tab, WRITE, UPROF ) ;
		if ( RC > 0 ) { abend() ; }
		tbsort( tab, "(KEYLISTN,C,A)" ) ;
		KEYLISTN = "ISPDEF" ;
		for ( i = 1 ; i < 25 ; i++ )
		{
			vreplace( "KEY"+d2ds(i)+"DEF", "" ) ;
			vreplace( "KEY"+d2ds(i)+"ATR", "" ) ;
			vreplace( "KEY"+d2ds(i)+"LAB", "" ) ;
		}
		vreplace( "KEYHELPN", "" ) ;
		tbadd( tab, "", "ORDER" ) ;
		if ( RC > 0 ) { abend() ; }
		tbclose( tab ) ;
		tbopen( tab, NOWRITE, UPROF ) ;
		if ( RC > 0 ) { abend() ; }
	}

	tbtop( tab ) ;
	while ( true )
	{
		tbskip( tab ) ;
		if ( RC > 0 ) { break ; }
		tbvclear( KLST ) ;
		TBK2LST = KEYLISTN ;
		if ( actTab && TBK2LST == AKLIST )
		{
			TBK2MSG = "*Active*" ;
		}
		tbadd( KLST ) ;
		if ( RC > 0 ) { abend() ; }
	}
	tbend( tab ) ;

	tbsort( KLST, "(TBK2LST,C,A)" ) ;

	ZTDTOP = 1 ;
	while ( true )
	{
		tbtop( KLST ) ;
		tbskip( KLST, ZTDTOP ) ;
		if ( MSG == "" ) { zcmd = "" ; }
		tbdispl( KLST, "PPSP01K2", MSG, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		while ( ZTDSELS > 0 )
		{
			if ( TBK2SEL == "D" )
			{
				addpop( "", 5, 5 ) ;
				display( "PPSP01K7", MSG, "ZCMD" ) ;
				RC1 = RC ;
				rempop() ;
				if ( RC1 == 0 )
				{
					tbopen( tab, WRITE, UPROF ) ;
					if ( RC > 0 ) { abend() ; }
					KEYLISTN = TBK2LST ;
					tbdelete( tab )   ;
					if ( RC > 0 ) { abend() ; }
					tbdelete( KLST ) ;
					if ( RC > 0 ) { abend() ; }
					tbclose( tab ) ;
				}
			}
			else if ( TBK2SEL == "N" )
			{
				addpop( "", 5, 5 ) ;
				display( "PPSP01K4", MSG, "ZCMD" ) ;
				RC1 = RC ;
				rempop() ;
				if ( RC1 == 0 )
				{
					tbopen( tab, NOWRITE, UPROF ) ;
					if ( RC > 0 ) { abend() ; }
					tbvclear( tab ) ;
					KEYLISTN = NEWKEY ;
					tbget( tab ) ;
					if ( RC == 0 )
					{
						tbend( tab ) ;
						TBK2MSG = "*Exists*" ;
					}
					else
					{
						tbend( tab ) ;
						tbopen( tab, WRITE, UPROF ) ;
						if ( RC > 0 ) { abend() ; }
						tbsort( tab, "(KEYLISTN,C,A)" ) ;
						KEYLISTN = NEWKEY ;
						for ( i = 1 ; i < 25 ; i++ )
						{
							vreplace( "KEY"+d2ds(i)+"DEF", "" ) ;
							vreplace( "KEY"+d2ds(i)+"ATR", "" ) ;
							vreplace( "KEY"+d2ds(i)+"LAB", "" ) ;
						}
						vcopy( "KEYHELP", t, MOVE ) ;
						vreplace( "KEYHELPN", t ) ;
						tbadd( tab, "", "ORDER" ) ;
						if ( RC > 0 ) { abend() ; }
						tbclose( tab ) ;
						TBK2LST = NEWKEY ;
						TBK2MSG = "*Added*" ;
						tbadd( KLST, "", "ORDER" ) ;
						if ( RC > 0 ) { abend() ; }
						control( "DISPLAY", "SAVE" ) ;
						editKeylist( tab, NEWKEY ) ;
						control( "DISPLAY", "RESTORE" ) ;
					}
					TBK2SEL = ""  ;
					tbput( KLST ) ;
				}
			}
			else if ( TBK2SEL == "E" )
			{
				control( "DISPLAY", "SAVE" ) ;
				editKeylist( tab, TBK2LST ) ;
				control( "DISPLAY", "RESTORE" ) ;
				TBK2MSG = "*Edited*" ;
				TBK2SEL = ""  ;
				tbput( KLST ) ;
			}
			else if ( TBK2SEL == "V" )
			{
				control( "DISPLAY", "SAVE" ) ;
				viewKeylist( tab, TBK2LST ) ;
				control( "DISPLAY", "RESTORE" ) ;
				TBK2MSG = "*Viewed*" ;
				TBK2SEL = ""  ;
				tbput( KLST ) ;
			}
			if ( ZTDSELS > 1 )
			{
				tbdispl( KLST ) ;
				if ( RC > 4 ) { break ; }
			}
			else { ZTDSELS = 0 ; }
		}
	}
	tbend( KLST ) ;
	vdelete( "TBK2SEL TBK2LST TBK2MSG KEYLISTN NEWKEY" ) ;
}


void PPSP01A::viewKeylist( const string& tab, const string& list )
{
	// Field names: KEYLISTN KEYnDEF KEYnLAB KEYnATR (n=1 to 24)
	// TD Field names: KEYNUM KEYDEF KEYATTR KEYLAB

	// Read keylist from table tab, KEYLISTN list and create table display.

	int i ;

	string t        ;
	string KEYNUM   ;
	string KEYDEF   ;
	string KEYATTR  ;
	string KEYLAB   ;

	string KEYLISTN ;
	string KLST     ;
	string UPROF    ;
	string MSG      ;

	KLST = "KLT4" + d2ds( taskid(), 4 ) ;

	vcopy( "ZUPROF", UPROF, MOVE ) ;

	tbopen( tab, NOWRITE, UPROF ) ;
	if ( RC > 0 ) { abend() ; }

	vdefine( "KEYLISTN KEYNUM KEYDEF KEYATTR KEYLAB", &KEYLISTN, &KEYNUM, &KEYDEF, &KEYATTR, &KEYLAB ) ;

	tbcreate( KLST, "KEYNUM", "(KEYDEF,KEYATTR,KEYLAB)", NOWRITE ) ;
	if ( RC > 0 ) { abend() ; }

	tbvclear( tab ) ;
	KEYLISTN = list ;
	tbget( tab ) ;
	if ( RC > 0 ) { abend() ; }

	vcopy( "KEYHELPN", t, MOVE ) ;
	vreplace( "KEYHELP", t ) ;
	for ( i = 1 ; i < 25 ; i++ )
	{
		KEYNUM = "F"+left( d2ds( i ), 2 ) + ". . ." ;
		vcopy( "KEY"+d2ds(i)+"DEF", KEYDEF, MOVE ) ;
		vcopy( "KEY"+d2ds(i)+"ATR", KEYATTR, MOVE ) ;
		vcopy( "KEY"+d2ds(i)+"LAB", KEYLAB, MOVE ) ;
		tbadd( KLST ) ;
		if ( RC > 0 ) { abend() ; }
	}
	tbend( tab ) ;

	ZTDTOP = 1 ;
	while ( true )
	{
		tbtop( KLST ) ;
		tbskip( KLST, ZTDTOP ) ;
		if ( MSG == "" ) { zcmd = "" ; }
		tbdispl( KLST, "PPSP01K6", MSG, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		MSG = "" ;
	}
	tbend( KLST )  ;
	vdelete( "KEYLISTN KEYNUM KEYDEF KEYATTR KEYLAB" ) ;
}


void PPSP01A::editKeylist( const string& tab, const string& list )
{
	// Field names: KEYLISTN KEYnDEF KEYnLAB KEYnATR (n=1 to 24)
	// TD Field names: KEYNUM KEYDEF KEYATTR KEYLAB

	// Read keylist from table tab, KEYLISTN list and create table display.
	// Update tab/list from table display.

	int i ;

	string t        ;
	string KEYNUM   ;
	string KEYDEF   ;
	string KEYATTR  ;
	string KEYLAB   ;

	string KEYLISTN ;
	string KLST     ;
	string UPROF    ;
	string MSG      ;

	KLST = "KLT3" + d2ds( taskid(), 4 ) ;

	vcopy( "ZUPROF", UPROF, MOVE ) ;

	tbopen( tab, NOWRITE, UPROF ) ;
	if ( RC > 0 ) { abend() ; }

	vdefine( "KEYLISTN KEYNUM KEYDEF KEYATTR KEYLAB", &KEYLISTN, &KEYNUM, &KEYDEF, &KEYATTR, &KEYLAB ) ;

	tbcreate( KLST, "KEYNUM", "(KEYDEF,KEYATTR,KEYLAB)", NOWRITE ) ;
	if ( RC > 0 ) { abend() ; }

	tbvclear( tab ) ;
	KEYLISTN = list ;
	tbget( tab ) ;
	if ( RC > 0 ) { abend() ; }

	vcopy( "KEYHELPN", t, MOVE ) ;
	vreplace( "KEYHELP", t ) ;
	for ( i = 1 ; i < 25 ; i++ )
	{
		KEYNUM = "F"+left( d2ds( i ), 2 ) + ". . ." ;
		vcopy( "KEY"+d2ds(i)+"DEF", KEYDEF, MOVE ) ;
		vcopy( "KEY"+d2ds(i)+"ATR", KEYATTR, MOVE ) ;
		vcopy( "KEY"+d2ds(i)+"LAB", KEYLAB, MOVE ) ;
		tbadd( KLST ) ;
		if ( RC > 0 ) { abend() ; }
	}
	tbend( tab ) ;

	ZTDTOP = 1 ;
	while ( true )
	{
		tbtop( KLST ) ;
		tbskip( KLST, ZTDTOP ) ;
		if ( MSG == "" ) { zcmd = "" ; }
		tbdispl( KLST, "PPSP01K3", MSG, "ZCMD" ) ;
		if ( zcmd == "CANCEL" )
		{
			vdelete( "KEYLISTN KEYNUM KEYDEF KEYATTR KEYLAB" ) ;
			tbend( KLST ) ;
			if ( RC > 0 ) { abend() ; }
			return ;
		}
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		while ( ZTDSELS > 0 )
		{
			tbmod( KLST ) ;
			if ( ZTDSELS > 1 )
			{
				tbdispl( KLST ) ;
				if ( RC > 4 ) { break ; }
			}
			else { ZTDSELS = 0 ; }
		}
	}

	tbopen( tab, WRITE, UPROF ) ;
	if ( RC > 0 ) { abend() ; }

	tbvclear( tab ) ;
	KEYLISTN = list ;
	tbget( tab ) ;
	if ( RC > 0 ) { abend() ; }

	vcopy( "KEYHELP", t, MOVE ) ;
	vreplace( "KEYHELPN", t ) ;
	tbtop( KLST ) ;
	for ( i = 1 ; i < 25 ; i++ )
	{
		tbskip( KLST, 1 ) ;
		if ( RC > 0 ) { break ; }
		vreplace( "KEY"+d2ds(i)+"DEF", KEYDEF ) ;
		vreplace( "KEY"+d2ds(i)+"ATR", KEYATTR ) ;
		vreplace( "KEY"+d2ds(i)+"LAB", KEYLAB ) ;
	}

	tbmod( tab )   ;
	if ( RC > 0 ) { abend() ; }

	tbclose( tab ) ;
	if ( RC > 0 ) { abend() ; }

	tbend( KLST )  ;

	vdelete( "KEYLISTN KEYNUM KEYDEF KEYATTR KEYLAB" ) ;
}


void PPSP01A::createKeyTable( string tab )
{
	// Create an empty keylist table entry

	int i ;

	string UPROF  ;
	string flds   ;

	vcopy( "ZUPROF", UPROF, MOVE ) ;
	tab += "KEYP" ;
	flds = ""     ;

	for ( i = 1 ; i < 25 ; i++ )
	{
		flds += "KEY"+d2ds(i)+"DEF " ;
		flds += "KEY"+d2ds(i)+"ATR " ;
		flds += "KEY"+d2ds(i)+"LAB " ;
	}
	flds += "KEYHELPN" ;

	tbcreate( tab, "KEYLISTN", "("+flds+")", WRITE, NOREPLACE, UPROF ) ;
	if ( RC > 0 ) { abend() ; }

	tbsave( tab ) ;
	if ( RC > 0 ) { abend() ; }

	tbend( tab ) ;
}


void PPSP01A::runApplication( const string& appl )
{
	select( "PGM("+appl+") NEWAPPL(ISP) NEWPOOL PASSLIB" ) ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PPSP01A ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
