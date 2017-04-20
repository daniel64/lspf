/*  Compile with ::                                                                       */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPLRFLST1.so -o libPLRFLST1.so PLRFLST1.cpp */

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


/* Personal File List application                                                 */

/* On exit, if reffield is set to #REFLIST and ZRC = 0                            */
/* then ZRESULT will be placed in the field specified by the .NRET panel variable */
/* eg:                                                                            */
/* .NRET = ON                                                                     */
/* .NRET = ZFILE                                                                  */


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
#include "../pWidgets.h"
#include "../pVPOOL.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"

#include "PLRFLST1.h"

using namespace std ;
using namespace boost::filesystem ;

#undef MOD_NAME
#undef LOGOUT
#define MOD_NAME PLRFLST1
#define LOGOUT aplog


void PLRFLST1::application()
{
	string P1 ;
	string P2 ;
	string PF ;

	llog( "I", "Application PLRFLST1 starting" << endl ) ;

	ZAPPDESC = "Personal File List application" ;

	P1 = word( PARM, 1 )          ;
	P2 = upper( word( PARM, 2 ) ) ;
	PF = subword( PARM, 2 )       ;

	setup() ;
	if      ( P1 == "PL1" ) { OpenActiveFList( P2 ) ; }
	else if ( P1 == "PL2" ) { PersonalFList()       ; }
	else if ( P1 == "PLA" ) { AddReflistEntry( PF ) ; }
	else if ( P1 == "NR1" ) { RetrieveEntry( PF )   ; }
	else if ( P1 == "US1" ) { userSettings()        ; }
	else if ( P1 == "BEX" ) { setRefMode( P1 )      ; }
	else if ( P1 == "BRT" ) { setRefMode( P1 )      ; }
	else                    { llog( "E", "Invalid parameter passed to PLRFLST1: " << PARM << endl ) ; }

	cleanup() ;
	return    ;
}



void PLRFLST1::setup()
{
	string vlist1 ;
	string vlist2 ;
	string vlist3 ;
	string vlist4 ;
	string vlist5 ;

	vlist1 = "ZCURTB   FLADESCP FLAPET01 FLAPET02 FLAPET03 FLAPET04 FLAPET05 FLAPET06 " ;
	vlist2 = "FLAPET07 FLAPET08 FLAPET09 FLAPET10 FLAPET11 FLAPET12 FLAPET13 FLAPET14 " ;
	vlist3 = "FLAPET15 FLAPET16 FLAPET17 FLAPET18 FLAPET19 FLAPET20 FLAPET21 FLAPET22 " ;
	vlist4 = "FLAPET23 FLAPET24 FLAPET25 FLAPET26 FLAPET27 FLAPET28 FLAPET29 FLAPET30 " ;
	vlist5 = "FLACTIME FLAUTIME " ;

	vdefine( vlist1, &ZCURTB,   &FLADESCP, &FLAPET01, &FLAPET02, &FLAPET03, &FLAPET04, &FLAPET05, &FLAPET06 ) ;
	vdefine( vlist2, &FLAPET07, &FLAPET08, &FLAPET09, &FLAPET10, &FLAPET11, &FLAPET12, &FLAPET13, &FLAPET14 ) ;
	vdefine( vlist3, &FLAPET15, &FLAPET16, &FLAPET17, &FLAPET18, &FLAPET19, &FLAPET20, &FLAPET21, &FLAPET22 ) ;
	vdefine( vlist4, &FLAPET23, &FLAPET24, &FLAPET25, &FLAPET26, &FLAPET27, &FLAPET28, &FLAPET29, &FLAPET30 ) ;
	vdefine( vlist5, &FLACTIME, &FLAUTIME ) ;

	TABFLDS = vlist1 + vlist2 + vlist3 + vlist4 + vlist5 ;

	vcopy( "ZUPROF", UPROF, MOVE ) ;
	vcopy( "ZRFLTBL", RFLTABLE, MOVE ) ;
	ZAHELP = "HPSP01A" ;
	ZRC    = 4         ;
}


void PLRFLST1::PersonalFList()
{
	string MSG    ;
	string FLIST1 ;
	string vlist  ;
	string ldate  ;
	string ltime  ;

	string ACURTB   ;
	string ASEL     ;
	string AFLDESCP ;
	string AFLCTIME ;
	string AFLUTIME ;
	string NEWNAME  ;
	string NEWDESC  ;
	string LCURTB   ;

	vlist = "ASEL ACURTB AFLDESCP AFLCTIME AFLUTIME NEWNAME NEWDESC LCURTB " ;
	vdefine( vlist, &ASEL, &ACURTB, &AFLDESCP, &AFLCTIME, &AFLUTIME, &NEWNAME, &NEWDESC, &LCURTB ) ;

	vget( "ZCURTB", PROFILE ) ;
	LCURTB = ZCURTB ;
	if ( LCURTB == "" )
	{
		LCURTB = "REFLIST" ;
		ZCURTB = "REFLIST" ;
		vput( "ZCURTB", PROFILE ) ;
	}

	OpenTableRO() ;
	if ( RC == 8 )
	{
		createDefaultTable() ;
		OpenTableRO() ;
		if ( RC > 0 ) { abend() ; }
	}
	else if ( RC > 0 ) { abend() ; }

	FLIST1 = "FLST1" + right( d2ds( taskid() ), 3, '0' ) ;
	tbcreate( FLIST1, "ACURTB", "ASEL AFLDESCP AFLCTIME AFLUTIME", NOWRITE ) ;
	tbsort( FLIST1, "ACURTB,C,A" ) ;

	while ( true )
	{
		tbskip( RFLTABLE )  ;
		if ( RC == 8 ) { break ; }
		ACURTB   = ZCURTB   ;
		ASEL     = ""       ;
		AFLDESCP = FLADESCP ;
		AFLCTIME = FLACTIME ;
		AFLUTIME = FLAUTIME ;
		tbadd( FLIST1, "", "ORDER" ) ;
	}
	CloseTable() ;

	while ( true )
	{
		tbtop( FLIST1 )  ;
		tbskip( FLIST1, ZTDTOP ) ;
		if ( MSG == "" ) { ZCMD = "" ; }
		tbdispl( FLIST1, "PLRFLST1", MSG, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		while ( ZTDSELS > 0 )
		{
			if ( ASEL == "A" )
			{
				display( "PLRFLST4", MSG, "ZCMD1" ) ;
				if ( RC == 0 )
				{
					vcopy( "ZDATEL", ldate, MOVE ) ;
					vcopy( "ZTIMEL", ltime, MOVE ) ;
					ZCURTB = ACURTB     ;
					OpenTableRO()       ;
					tbget( RFLTABLE )   ;
					CloseTable()        ;
					ZCURTB   = NEWNAME  ;
					LCURTB   = NEWNAME  ;
					vput( "ZCURTB", PROFILE ) ;
					FLADESCP = NEWDESC  ;
					FLACTIME = ldate    ;
					FLAUTIME = ldate + " " + ltime   ;
					OpenTableUP()       ;
					tbadd( RFLTABLE, "", "ORDER" ) ;
					CloseTable()        ;
					ACURTB   = ZCURTB   ;
					ASEL     = ""       ;
					AFLDESCP = FLADESCP ;
					AFLCTIME = FLACTIME ;
					AFLUTIME = FLAUTIME ;
					tbadd( FLIST1, "", "ORDER" ) ;
				}
			}
			else if ( ASEL == "D" )
			{
				if ( ACURTB == "REFLIST" )
				{
				}
				else
				{
					vget( "ZCURTB", PROFILE ) ;
					if ( ZCURTB == ACURTB )
					{
						ZCURTB = "REFLIST"        ;
						LCURTB = ZCURTB           ;
						vput( "ZCURTB", PROFILE ) ;
					}
					ZCURTB = ACURTB        ;
					OpenTableUP()          ;
					tbdelete( RFLTABLE )   ;
					CloseTable()           ;
					tbdelete( FLIST1 )     ;
				}

			}
			else if ( ASEL == "E" )
			{
				control( "DISPLAY", "SAVE" )    ;
				EditFileList( ACURTB )          ;
				control( "DISPLAY", "RESTORE" ) ;
			}
			else if ( ASEL == "O" )
			{
				LCURTB = ACURTB ;
				control( "DISPLAY", "SAVE" )    ;
				OpenFileList( ACURTB )          ;
				control( "DISPLAY", "RESTORE" ) ;
				vput( "ZCURTB", PROFILE )       ;
				if ( ZRC == 0 ) { break ; }     ;
			}
			else if ( ASEL == "S" )
			{
				LCURTB = ACURTB           ;
				ZCURTB = ACURTB           ;
				vput( "ZCURTB", PROFILE ) ;
			}
			ASEL = "" ;
			tbput( FLIST1, "", "ORDER" ) ;
			if ( ZTDSELS > 1 )
			{
				tbdispl( FLIST1 ) ;
				if ( RC > 4 ) { break ; }
			}
			else { ZTDSELS = 0 ; }
		}
		if ( ZRC == 0 ) { break ; } ;
	}
	tbend( FLIST1 )  ;
	vdelete( vlist ) ;
}


void PLRFLST1::OpenActiveFList( string list )
{
	// Open the active referral list or the one specified in 'list'
	// Make 'list' the active one unless it is the REFLIST we are opening

	if ( list != "" )
	{
		ZCURTB = list ;
		vreplace( "ZCURTB", list ) ;
		if ( list != "REFLIST" )
		{
			vput( "ZCURTB", PROFILE )  ;
		}
	}
	else
	{
		vget( "ZCURTB", PROFILE )  ;
	}

	if ( ZCURTB == "" )
	{
		ZCURTB = "REFLIST" ;
		vput( "ZCURTB", PROFILE ) ;
	}
	OpenFileList( ZCURTB ) ;
}


void PLRFLST1::EditFileList( string curtb )
{
	int i ;

	string MSG   ;
	string ZCMD1 ;

	string FLIST2 ;
	string BSEL   ;
	string BFILE  ;
	string vlist  ;
	string ldate  ;
	string ltime  ;

	bool modified ;

	vlist = "BSEL BFILE ZCMD1" ;
	vdefine( vlist, &BSEL, &BFILE, &ZCMD1 ) ;


	FLIST2 = "FLST2" + right( d2ds( taskid() ), 3, '0' ) ;
	tbcreate( FLIST2, "", "BSEL BFILE", NOWRITE ) ;

	ZCURTB = curtb ;
	OpenTableRO()  ;
	if ( RC == 8 )
	{
		createDefaultTable() ;
		OpenTableRO() ;
		if ( RC > 0 ) { abend() ; }
	}
	else if ( RC > 0 ) { abend() ; }
	tbget( RFLTABLE ) ;

	BSEL = "" ;
	for ( i = 1 ; i <= 30 ; i++ )
	{
		vcopy( "FLAPET" + right( d2ds( i ), 2, '0' ), BFILE, MOVE ) ;
		if ( i > 1 && BFILE == "" ) { continue ; }
		tbadd( FLIST2 ) ;
	}
	CloseTable() ;

	modified = false ;
	while ( true )
	{
		tbtop( FLIST2 )  ;
		tbskip( FLIST2, ZTDTOP ) ;
		if ( MSG == "" ) { ZCMD1 = "" ; }
		tbdispl( FLIST2, "PLRFLST2", MSG, "ZCMD1" ) ;
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		if ( ZCMD1 == "CANCEL" ) { modified = false ; break ; }
		while ( ZTDSELS > 0 )
		{
			if ( BSEL == "" )
			{
				tbput( FLIST2 ) ;
				modified = true ;
			}
			if ( BSEL == "I" )
			{
				BSEL  = "" ;
				BFILE = "" ;
				tbadd( FLIST2 ) ;
				modified = true ;
			}
			if ( BSEL == "R" )
			{
				BSEL = "" ;
				tbadd( FLIST2 ) ;
				modified = true ;
			}
			else if ( BSEL == "D" )
			{
				tbdelete( FLIST2 ) ;
				modified = true ;
			}
			BSEL = "" ;
			if ( ZTDSELS > 1 )
			{
				tbdispl( FLIST2 ) ;
				if ( RC > 4 ) { break ; }
			}
			else { ZTDSELS = 0 ; }
		}
	}
	if ( modified )
	{
		OpenTableUP()     ;
		tbget( RFLTABLE ) ;
		tbtop( FLIST2 ) ;
		for ( i = 1 ; i <= 30 ; i++ )
		{
			tbskip( FLIST2 ) ;
			if ( RC > 0 ) { break ; }
			if ( BFILE == "" ) { i-- ; continue ; }
			vreplace( "FLAPET" + right( d2ds( i ), 2, '0' ), BFILE ) ;
		}
		for ( ; i <= 30 ; i++ )
		{
			vreplace( "FLAPET" + right( d2ds( i ), 2, '0' ), "" ) ;
		}
		vcopy( "ZDATEL", ldate, MOVE ) ;
		vcopy( "ZTIMEL", ltime, MOVE ) ;
		FLAUTIME = ldate + " " + ltime ;
		tbmod( RFLTABLE, "", "ORDER" ) ;
		CloseTable() ;
	}
	tbend( FLIST2 )  ;
	vdelete( vlist ) ;
}


void PLRFLST1::OpenFileList( string curtb )
{
	int i ;

	string MSG    ;
	string ZCMD1  ;

	string FLIST3 ;
	string CSEL   ;
	string CFILE  ;
	string vlist  ;

	FLIST3 = "FLST3" + right( d2ds( taskid() ), 3, '0' ) ;
	tbcreate( FLIST3, "", "CSEL CFILE", NOWRITE ) ;

	ZCURTB = curtb ;
	OpenTableRO()  ;
	if ( RC == 8 )
	{
		createDefaultTable() ;
		OpenTableRO() ;
		if ( RC > 0 ) { abend() ; }
	}
	else if ( RC > 0 ) { abend() ; }
	tbget( RFLTABLE ) ;
	CloseTable()      ;

	vlist = "CSEL CFILE ZCMD1" ;
	vdefine( vlist, &CSEL, &CFILE, &ZCMD1 ) ;

	CSEL = "" ;

	for ( i = 1 ; i <= 30 ; i++ )
	{
		vcopy( "FLAPET" + right( d2ds( i ), 2, '0' ), CFILE, MOVE ) ;
		if ( i > 1 && CFILE == "" ) { continue ; }
		tbadd( FLIST3 ) ;
	}

	while ( true )
	{
		tbtop( FLIST3 )  ;
		tbskip( FLIST3, ZTDTOP ) ;
		if ( MSG == "" ) { ZCMD1 = "" ; }
		tbdispl( FLIST3, "PLRFLST3", MSG, "ZCMD1" ) ;
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		while ( ZTDSELS > 0 )
		{
			if ( CSEL == "S" )
			{
				reffield = "#REFLIST" ;
				ZRESULT  = CFILE      ;
				ZRC      = 0          ;
				break ;
			}
			if ( ZTDSELS > 1 )
			{
				tbdispl( FLIST3 ) ;
				if ( RC > 4 ) { break ; }
			}
			else { ZTDSELS = 0 ; }
		}
		if ( ZRC == 0 ) { break ; }
	}
	tbend( FLIST3 )  ;
	vdelete( vlist ) ;
}


void PLRFLST1::RetrieveEntry( string list )
{
	// Retrieve entry from the active referral list or the one specified in 'list', at posn ZRFNPOS
	// Also, make 'list' the active one so the next NRETRIEV with no parameters, is from the same list

	// If ZRFNEX is YES, check file exists, else get the next entry

	// If 'list' consists a number < 31, use this as the starting position for the retrieve and not
	// variable ZRFNPOS from the SHARED pool.  If specified, the other word in 'list' is used as the reflist
	// If 'list' is specified, but not a number, start at position 1.

	int    i   ;
	int    p   ;
	int    fp  ;
	int    pos ;

	string w1  ;
	string w2  ;

	bool skip  ;

	string ZRFNEX  ;
	string ZRFNPOS ;

	vdefine( "ZRFNEX ZRFNPOS", &ZRFNEX, &ZRFNPOS ) ;
	iupper( list )  ;
	w1   = word( list, 1 ) ;
	w2   = word( list, 2 ) ;

	ZRFNPOS  = "" ;
	if ( w1.size() > 0 && w1.size() < 3 )
	{
		list = w2 ;
		if ( datatype( w1, 'W' ) ) { pos = ds2d( w1 ) - 1 ; }
		if ( pos < 30 )
		{
			ZRFNPOS = d2ds( pos ) ;
			vput( "ZRFNPOS", SHARED ) ;
		}
	}
	else if ( w2.size() > 0 && w2.size() < 3 )
	{
		list = w1 ;
		if ( datatype( w2, 'W' ) ) { pos = ds2d( w2 ) -1 ; }
		if ( pos < 30 )
		{
			ZRFNPOS = d2ds( pos ) ;
			vput( "ZRFNPOS", SHARED ) ;
		}
	}

	if ( list == "" )
	{
		vget( "ZCURTB", PROFILE ) ;
	}
	else
	{
		ZCURTB = list ;
		if ( ZRFNPOS == "" ) { verase( "ZRFNPOS", SHARED ) ; }
	}

	OpenTableRO()     ;
	tbget( RFLTABLE ) ;
	if ( RC > 0 )
	{
		ZCURTB = "REFLIST" ;
		tbget( RFLTABLE ) ;
		if ( RC > 0 ) { CloseTable() ; return ; }
	}
	CloseTable() ;

	vput( "ZCURTB", PROFILE ) ;
	vget( "ZRFNEX", PROFILE ) ;

	vget( "ZRFNPOS", SHARED ) ;
	if ( RC == 8 ) { p = 1 ; }
	else           { (ZRFNPOS == "30") ? (p = 1) : (p = ds2d( ZRFNPOS ) + 1) ; }

	fp = p ;
	for ( i = 1 ; i <= 30 ; i++ )
	{
		if ( p > 30 ) { p = 1 ; }
		vcopy( "FLAPET" + right( d2ds( p ), 2, '0' ), ZRESULT, MOVE ) ;
		if ( ZRESULT == "" && p > 1 )
		{
			p = 1 ;
			if ( fp == p )
			{
				ZRFNPOS = "1" ;
				vput( "ZRFNPOS", SHARED )   ;
				vdelete( "ZRFNEX ZRFNPOS" ) ;
				return ;
			}
			continue ;
		}
		if ( ZRFNEX == "YES" )
		{
			skip = false ;
			try
			{
				if ( !exists( ZRESULT ) ) { skip = true ; }
			}
			catch ( const filesystem_error& ex )
			{
				skip = true ;
			}
			if ( !skip ) { break ; }
			p++ ;
			if ( fp == p )
			{
				ZRFNPOS = d2ds( p ) ;
				vput( "ZRFNPOS", SHARED ) ;
				vdelete( "ZRFNEX ZRFNPOS" ) ;
				return  ;
			}
			continue ;
		}
		break ;
	}

	ZRFNPOS  = d2ds( p )        ;
	vput( "ZRFNPOS", SHARED )   ;
	reffield = "#REFLIST"       ;
	ZRC      = 0                ;
	vdelete( "ZRFNEX ZRFNPOS" ) ;
}


void PLRFLST1::AddReflistEntry( string ent )
{
	int i ;

	string eent  ;
	string rffex ;
	string ldate ;
	string ltime ;

	map<string,bool>found ;
	vector<string>list ;

	vcopy( "ZDATEL", ldate, MOVE ) ;
	vcopy( "ZTIMEL", ltime, MOVE ) ;
	vcopy( "ZRFFEX", rffex, MOVE ) ;

	if ( rffex == "YES" )
	{
		try
		{
			if ( !exists( ent ) ) { return ; }
		}
		catch ( const filesystem_error& ex )
		{
			return ;
		}
	}

	control( "ERRORS", "RETURN" ) ;
	OpenTableUP() ;
	if ( RC == 8 )
	{
		createDefaultTable() ;
		OpenTableUP() ;
		if ( RC > 0 ) { abend() ; }
	}
	else if ( RC > 0 ) { abend() ; }

	ZCURTB = "REFLIST"  ;
	tbget( RFLTABLE ) ;
	if ( RC > 0 ) { CloseTable() ; return ; }

	found[ ent ] = true ;
	list.push_back( ent ) ;

	for ( i = 1 ; i <= 30 ; i++ )
	{
		vcopy( "FLAPET" + right( d2ds( i ), 2, '0' ), eent, MOVE ) ;
		if ( eent == "" ) { continue ; }
		if ( found.find( eent )     != found.end() ) { continue ; }
		if ( found.find( eent+"/" ) != found.end() ) { continue ; }
		list.push_back( eent ) ;
		found[ eent ] = true ;
	}

	for ( i = 0 ; i <= 29 ; i++ )
	{
		if ( i < list.size() )
		{
			vreplace( "FLAPET" + right( d2ds( i+1 ), 2, '0' ), list.at( i ) ) ;
		}
		else
		{
			vreplace( "FLAPET" + right( d2ds( i+1 ), 2, '0' ), "" ) ;
		}
	}

	FLAUTIME = ldate + " " + ltime ;
	tbmod( RFLTABLE, "", "ORDER" ) ;
	CloseTable() ;
}


void PLRFLST1::createDefaultTable()
{
	string ldate ;
	string ltime ;

	tbcreate( RFLTABLE, "ZCURTB", subword( TABFLDS, 2 ), WRITE, NOREPLACE, UPROF ) ;
	tbvclear( RFLTABLE ) ;
	ZCURTB = "REFLIST" ;
	vput( "ZCURTB", PROFILE ) ;
	vcopy( "ZDATEL", ldate, MOVE )   ;
	vcopy( "ZTIMEL", ltime, MOVE )   ;
	FLADESCP = "Default Reference List" ;
	FLACTIME = ldate ;
	FLAUTIME = ldate + " " + ltime ;
	tbadd( RFLTABLE ) ;
	tbsort( RFLTABLE, "ZCURTB,C,A" ) ;
	CloseTable() ;
}


void PLRFLST1::userSettings()
{
	while ( true )
	{
		display( "PLRFLST5" ) ;
		if ( RC == 8 ) { break ; }
	}
	return ;
}


void PLRFLST1::setRefMode( string mode )
{
	string ZRFMOD ;

	ZRFMOD = mode ;

	vdefine( "ZRFMOD", &ZRFMOD ) ;
	vput( "ZRFMOD", PROFILE ) ;
	vdelete( "ZRFMOD" ) ;

	return ;
}


void PLRFLST1::OpenTableRO()
{
	tbopen( RFLTABLE, NOWRITE, UPROF ) ;
	return ;
}


void PLRFLST1::OpenTableUP()
{
	tbopen( RFLTABLE, WRITE, UPROF ) ;
	return ;
}


void PLRFLST1::CloseTable()
{
	tbclose( RFLTABLE ) ;
	return ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PLRFLST1 ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
