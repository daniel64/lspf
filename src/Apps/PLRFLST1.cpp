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

#undef  MOD_NAME
#define MOD_NAME PLRFLST1


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
	else if ( P1 == "PL2" ) { PersonalFList( "" )   ; }
	else if ( P1 == "PL3" ) { PersonalFList( "DSL") ; }
	else if ( P1 == "PLA" ) { AddReflistEntry( PF ) ; }
	else if ( P1 == "NR1" ) { RetrieveEntry( PF )   ; }
	else if ( P1 == "US1" ) { userSettings()        ; }
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


void PLRFLST1::PersonalFList( const string& p )
{
	int RC1 ;

	string PGM    ;
	string MSG    ;
	string PANL   ;
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

	PANL = ( p == "" ) ? "PLRFLST1" : "PLRFLST6" ;

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

	FLIST1 = "FLST1" + d2ds( taskid(), 3 ) ;
	tbcreate( FLIST1, "ACURTB", "(ASEL,AFLDESCP,AFLCTIME,AFLUTIME)", NOWRITE ) ;
	tbsort( FLIST1, "(ACURTB,C,A)" ) ;

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
		tbtop( FLIST1 ) ;
		tbskip( FLIST1, ZTDTOP ) ;
		if ( MSG == "" ) { zcmd = "" ; }
		tbdispl( FLIST1, PANL, MSG, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		if ( ZTDSELS == 0 && p != "" && ZCURINX != 0 )
		{
			tbtop( FLIST1 ) ;
			tbskip( FLIST1, ZCURINX ) ;
			ZTDSELS = 1   ;
			ASEL    = "L" ;
		}
		while ( ZTDSELS > 0 )
		{
			if ( ASEL == "A" )
			{
				addpop( "", 9, 9 ) ;
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
					FLAUTIME = ldate + " " + ltime ;
					OpenTableUP()       ;
					tbsort( RFLTABLE, "(ZCURTB,C,A)" ) ;
					tbadd( RFLTABLE, "", "ORDER" ) ;
					CloseTable()        ;
					ACURTB   = ZCURTB   ;
					ASEL     = ""       ;
					AFLDESCP = FLADESCP ;
					AFLCTIME = FLACTIME ;
					AFLUTIME = FLAUTIME ;
					tbadd( FLIST1, "", "ORDER" ) ;
				}
				rempop() ;
			}
			else if ( ASEL == "D" )
			{
				if ( ACURTB != "REFLIST" )
				{
					addpop( "", 9, 5 ) ;
					display( "PLRFLST7", MSG, "ZCMD" ) ;
					RC1 = RC ;
					rempop() ;
					if ( RC1 == 0 )
					{
						vget( "ZCURTB", PROFILE ) ;
						if ( ZCURTB == ACURTB )
						{
							ZCURTB = "REFLIST" ;
							LCURTB = ZCURTB    ;
							vput( "ZCURTB", PROFILE ) ;
						}
						ZCURTB = ACURTB      ;
						OpenTableUP()        ;
						tbdelete( RFLTABLE ) ;
						CloseTable()         ;
						tbdelete( FLIST1 )   ;
					}
				}

			}
			else if ( ASEL == "E" )
			{
				control( "DISPLAY", "SAVE" )    ;
				EditFileList( ACURTB )          ;
				control( "DISPLAY", "RESTORE" ) ;
			}
			else if ( ASEL == "L" )
			{
				vcopy( "ZFLSTPGM", PGM, MOVE ) ;
				select( "PGM(" + PGM + ") PARM(LIST " + ACURTB + " " + StoreFileList( ACURTB ) + ")" ) ;
				ZRC = 4 ;
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


void PLRFLST1::OpenActiveFList( const string& list )
{
	// Open the active referral list or the one specified in 'list'
	// Make 'list' the active one unless it is the REFLIST we are opening

	if ( list != "" )
	{
		OpenTableRO() ;
		if ( RC == 8 )
		{
			createDefaultTable() ;
			OpenTableRO() ;
			if ( RC > 0 ) { abend() ; }
		}
		ZCURTB = list ;
		tbget( RFLTABLE ) ;
		if ( RC == 8 )
		{
			tbvclear( RFLTABLE ) ;
			vreplace( "ZCURTB", upper( list )+"*" ) ;
			tbsarg( RFLTABLE, "", "NEXT", "(ZCURTB,EQ)" ) ;
			tbscan( RFLTABLE ) ;
			if ( RC == 8 )
			{
				ZCURTB = "REFLIST" ;
				tbget( RFLTABLE ) ;
				if ( RC > 0 ) { CloseTable() ; return ; }
			}
		}
		CloseTable() ;
		if ( list != "REFLIST" )
		{
			vput( "ZCURTB", PROFILE ) ;
		}
	}
	else
	{
		vget( "ZCURTB", PROFILE ) ;
	}

	if ( ZCURTB == "" )
	{
		ZCURTB = "REFLIST" ;
		vput( "ZCURTB", PROFILE ) ;
	}

	OpenFileList( ZCURTB ) ;
}


void PLRFLST1::EditFileList( const string& curtb )
{
	int i ;

	string MSG   ;
	string zcmd1 ;

	string FLIST2 ;
	string BSEL   ;
	string BFILE  ;
	string vlist  ;
	string ldate  ;
	string ltime  ;

	bool modified ;

	vlist = "BSEL BFILE ZCMD1" ;
	vdefine( vlist, &BSEL, &BFILE, &zcmd1 ) ;


	FLIST2 = "FLST2" + d2ds( taskid(), 3 ) ;
	tbcreate( FLIST2, "", "(BSEL,BFILE)", NOWRITE ) ;

	ZCURTB = curtb ;
	OpenTableRO()  ;
	if ( RC == 8 )
	{
		createDefaultTable() ;
		OpenTableRO() ;
		if ( RC > 0 ) { abend() ; }
	}
	tbget( RFLTABLE ) ;

	BSEL = "" ;
	for ( i = 1 ; i <= 30 ; i++ )
	{
		vcopy( "FLAPET" + d2ds( i, 2 ), BFILE, MOVE ) ;
		if ( i > 1 && BFILE == "" ) { continue ; }
		tbadd( FLIST2 ) ;
	}
	CloseTable() ;

	modified = false ;
	while ( true )
	{
		tbtop( FLIST2 ) ;
		tbskip( FLIST2, ZTDTOP ) ;
		if ( MSG == "" ) { zcmd1 = "" ; }
		tbdispl( FLIST2, "PLRFLST2", MSG, "ZCMD1" ) ;
		if ( zcmd1 == "CANCEL" ) { modified = false ; break ; }
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		while ( ZTDSELS > 0 )
		{
			if ( BSEL == "" )
			{
				tbput( FLIST2 ) ;
				modified = true ;
			}
			else if ( BSEL == "I" )
			{
				BSEL  = "" ;
				BFILE = "" ;
				tbadd( FLIST2 ) ;
				modified = true ;
			}
			else if ( BSEL == "R" )
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
			if ( ZTDSELS > 1 )
			{
				tbdispl( FLIST2 ) ;
				if ( RC > 4 ) { break ; }
			}
			else { ZTDSELS = 0 ; }
			BSEL = "" ;
		}
	}
	if ( modified )
	{
		OpenTableUP() ;
		tbget( RFLTABLE ) ;
		tbtop( FLIST2 ) ;
		for ( i = 1 ; i <= 30 ; i++ )
		{
			tbskip( FLIST2 ) ;
			if ( RC > 0 ) { break ; }
			if ( BFILE == "" ) { i-- ; continue ; }
			vreplace( "FLAPET" + d2ds( i, 2 ), BFILE ) ;
		}
		for ( ; i <= 30 ; i++ )
		{
			vreplace( "FLAPET" + d2ds( i, 2 ), "" ) ;
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


void PLRFLST1::OpenFileList( const string& curtb )
{
	int i ;
	int j ;

	string MSG    ;
	string zcmd1  ;

	string FLIST3 ;
	string CSEL   ;
	string CFILE  ;
	string CNUM   ;
	string vlist  ;

	FLIST3 = "FLST3" + d2ds( taskid(), 3 ) ;
	tbcreate( FLIST3, "", "(CSEL,CFILE,CNUM)", NOWRITE ) ;

	ZCURTB = curtb ;
	OpenTableRO()  ;
	if ( RC == 8 )
	{
		createDefaultTable() ;
		OpenTableRO() ;
		if ( RC > 0 ) { abend() ; }
	}

	tbget( RFLTABLE ) ;
	if ( RC == 8 )
	{
		ZCURTB = "REFLIST" ;
		tbget( RFLTABLE ) ;
		if ( RC > 0 ) { CloseTable() ; return ; }
	}

	CloseTable() ;

	vlist = "CSEL CFILE CNUM ZCMD1" ;
	vdefine( vlist, &CSEL, &CFILE, &CNUM, &zcmd1 ) ;

	CSEL = "" ;

	for ( j = 0, i = 1 ; i <= 30 ; i++ )
	{
		vcopy( "FLAPET" + d2ds( i, 2 ), CFILE, MOVE ) ;
		if ( i > 1 && CFILE == "" ) { continue ; }
		j++ ;
		CNUM = d2ds( j ) ;
		tbadd( FLIST3 )  ;
	}

	while ( true )
	{
		tbtop( FLIST3 )  ;
		tbskip( FLIST3, ZTDTOP ) ;
		if ( MSG == "" ) { zcmd1 = "" ; }
		tbdispl( FLIST3, "PLRFLST3", MSG, "ZCMD1" ) ;
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		if ( ZTDSELS == 0 && ZCURINX != 0 )
		{
			tbtop( FLIST3 ) ;
			tbskip( FLIST3, ZCURINX ) ;
			CSEL    = "S" ;
			ZTDSELS = 1   ;
		}
		while ( ZTDSELS > 0 )
		{
			if ( CSEL == "S" )
			{
				reffield = "#REFLIST" ;
				ZRESULT  = CFILE      ;
				ZRC      = 0          ;
				vreplace( "ZCURTB", ZCURTB ) ;
				vreplace( "ZRFNPOS", CNUM )  ;
				vput( "ZCURTB ZRFNPOS", SHARED ) ;
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


string PLRFLST1::StoreFileList( const string& curtb )
{
	int i ;

	string fname ;
	string cname ;

	std::ofstream fout ;

	vcopy( "ZUSER", zuser, MOVE )     ;
	vcopy( "ZSCREEN", zscreen, MOVE ) ;

	boost::filesystem::path temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;
	string tname = temp.native() ;

	ZCURTB = curtb ;
	OpenTableRO()  ;
	if ( RC == 8 )
	{
		createDefaultTable() ;
		OpenTableRO() ;
		if ( RC > 0 ) { abend() ; }
	}
	tbget( RFLTABLE ) ;
	CloseTable()      ;

	fout.open( tname ) ;
	for ( i = 1 ; i <= 30 ; i++ )
	{
		vcopy( "FLAPET" + d2ds( i, 2 ), fname, MOVE ) ;
		if ( fname == "" ) { continue ; }
		fout << fname << endl ;
	}
	fout.close() ;
	return tname ;
}


void PLRFLST1::RetrieveEntry( string list )
{
	// Retrieve entry from the active referral list or the one specified in 'list', at posn ZRFNPOS
	// Also, make 'list' the active one so the next NRETRIEV with no parameters, is from the same list

	// If ZRFNEX is YES, check file exists, else get the next entry

	// If 'list' consists a number < 31, use this as the starting position for the retrieve and not
	// variable ZRFNPOS from the SHARED pool.  If specified, the other word in 'list' is used as the reflist
	// If 'list' is specified, but not a number, start at position 1.

	int i   ;
	int p   ;
	int fp  ;
	int pos ;

	bool skip ;

	string w1 ;
	string w2 ;

	string ZRFNEX  ;
	string ZRFNPOS ;

	vdefine( "ZRFNEX ZRFNPOS", &ZRFNEX, &ZRFNPOS ) ;

	iupper( list )  ;
	w1 = word( list, 1 ) ;
	w2 = word( list, 2 ) ;

	ZRFNPOS = "" ;

	if ( w2.size() > 0 && w2.size() < 3 && datatype( w2, 'W' ) )
	{
		swap( w1, w2 ) ;
	}

	if ( w1.size() > 0 && w1.size() < 3 && datatype( w1, 'W' ) )
	{
		list = w2 ;
		if ( datatype( w1, 'W' ) ) { pos = ds2d( w1 ) - 1 ; }
		if ( pos < 30 )
		{
			ZRFNPOS = d2ds( pos ) ;
			vput( "ZRFNPOS", SHARED ) ;
		}
	}
	else
	{
		list = w1 ;
	}

	if ( list == "" )
	{
		vget( "ZCURTB", PROFILE ) ;
		list = ZCURTB ;
	}
	else
	{
		ZCURTB = list ;
		if ( ZRFNPOS == "" ) { verase( "ZRFNPOS", SHARED ) ; }
	}

	OpenTableRO()     ;
	tbget( RFLTABLE ) ;
	if ( RC == 8 )
	{
		tbvclear( RFLTABLE ) ;
		vreplace( "ZCURTB", upper( list )+"*" ) ;
		tbsarg( RFLTABLE, "", "NEXT", "(ZCURTB,EQ)" ) ;
		tbscan( RFLTABLE ) ;
		if ( RC == 8 )
		{
			ZCURTB = "REFLIST" ;
			tbget( RFLTABLE ) ;
			if ( RC > 0 ) { CloseTable() ; return ; }
		}
	}
	CloseTable() ;

	vput( "ZCURTB", PROFILE ) ;
	vget( "ZRFNEX", PROFILE ) ;

	vget( "ZRFNPOS", SHARED ) ;
	if ( RC == 8 ) { p = 1 ; }
	else           { p = ( ZRFNPOS == "30" ) ? 1 : ds2d( ZRFNPOS ) + 1 ; }

	fp = p ;
	for ( i = 1 ; i <= 30 ; i++ )
	{
		if ( p > 30 ) { p = 1 ; }
		vcopy( "FLAPET" + d2ds( p, 2 ), ZRESULT, MOVE ) ;
		if ( ZRESULT == "" && p > 1 )
		{
			p = 1 ;
			if ( fp == 1 ) { break ; }
			continue ;
		}
		if ( ZRFNEX == "YES" )
		{
			skip = false ;
			try
			{
				if ( !exists( ZRESULT ) )
				{
					ZRESULT = ""   ;
					skip    = true ;
				}
			}
			catch ( const filesystem_error& ex )
			{
				ZRESULT = ""   ;
				skip    = true ;
			}
			if ( !skip ) { break ; }
			p++ ;
			if ( fp == p ) { break ; }
			continue ;
		}
		break ;
	}

	ZRFNPOS  = d2ds( p )  ;
	reffield = "#REFLIST" ;
	ZRC      = ( ZRESULT == "" ) ? 8 : 0 ;

	vput( "ZRFNPOS", SHARED )   ;
	vdelete( "ZRFNEX ZRFNPOS" ) ;
}


void PLRFLST1::AddReflistEntry( string& ent )
{
	int i ;

	string* eent ;

	string rffex ;
	string ldate ;
	string ltime ;

	vector<string>list ;
	vector<string>::iterator it ;

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

	ZCURTB = "REFLIST" ;
	tbget( RFLTABLE )  ;
	if ( RC > 0 ) { CloseTable() ; return ; }

	for ( i = 1 ; i <= 30 ; i++ )
	{
		vcopy( "FLAPET" + d2ds( i, 2 ), eent, LOCATE ) ;
		if ( *eent == "" ) { continue ; }
		list.push_back( *eent ) ;
	}

	if ( ent.back() == '/' ) { ent.pop_back() ; }

	it = find( list.begin(), list.end(), ent ) ;
	if ( it != list.end() )
	{
		list.erase( it ) ;
	}

	vreplace( "FLAPET01", ent ) ;

	for ( i = 2 ; i <= 30 ; i++ )
	{
		if ( i <= list.size()+1 )
		{
			vreplace( "FLAPET" + d2ds( i, 2 ), list.at( i-2 ) ) ;
		}
		else
		{
			vreplace( "FLAPET" + d2ds( i, 2 ), "" ) ;
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

	tbcreate( RFLTABLE, "ZCURTB", "("+subword( TABFLDS, 2 )+")", WRITE, NOREPLACE, UPROF ) ;
	if ( RC > 0 ) { return ; }

	tbvclear( RFLTABLE ) ;
	ZCURTB = "REFLIST" ;
	vput( "ZCURTB", PROFILE ) ;
	vcopy( "ZDATEL", ldate, MOVE )   ;
	vcopy( "ZTIMEL", ltime, MOVE )   ;
	FLADESCP = "Default Reference List" ;
	FLACTIME = ldate ;
	FLAUTIME = ldate + " " + ltime ;

	tbadd( RFLTABLE ) ;
	if ( RC > 0 ) { return ; }
	tbsort( RFLTABLE, "(ZCURTB,C,A)" ) ;
	if ( RC > 0 ) { return ; }

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
