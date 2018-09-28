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
#include "../pVPOOL.h"
#include "../pWidgets.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"

#include "PLRFLST1.h"

using namespace std ;
using namespace boost::filesystem ;

#undef  MOD_NAME
#define MOD_NAME PLRFLST1


PLRFLST1::PLRFLST1()
{
	vdefine( "ZCURINX ZTDTOP ZTDSELS", &zcurinx, &ztdtop, &ztdsels ) ;
}


void PLRFLST1::application()
{
	string p1 ;
	string p2 ;
	string pf ;

	llog( "I", "Application PLRFLST1 starting" << endl ) ;

	set_appdesc( "Personal File List application" ) ;
	set_appver( "1.0.0" ) ;

	p1 = word( PARM, 1 )          ;
	p2 = upper( word( PARM, 2 ) ) ;
	pf = subword( PARM, 2 )       ;

	setup() ;
	if      ( p1 == "PL1" ) { OpenActiveFList( p2 )    ; }
	else if ( p1 == "PL2" ) { PersonalFList( "" )      ; }
	else if ( p1 == "PL3" ) { PersonalFList( "DSL")    ; }
	else if ( p1 == "PLA" ) { AddReflistEntry( pf )    ; }
	else if ( p1 == "NR1" ) { RetrieveEntry( pf )      ; }
	else if ( p1 == "MTC" ) { RetrieveMatchEntry( pf ) ; }
	else if ( p1 == "US1" ) { userSettings()           ; }
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

	vdefine( vlist1, &zcurtb,   &fladescp, &flapet01, &flapet02, &flapet03, &flapet04, &flapet05, &flapet06 ) ;
	vdefine( vlist2, &flapet07, &flapet08, &flapet09, &flapet10, &flapet11, &flapet12, &flapet13, &flapet14 ) ;
	vdefine( vlist3, &flapet15, &flapet16, &flapet17, &flapet18, &flapet19, &flapet20, &flapet21, &flapet22 ) ;
	vdefine( vlist4, &flapet23, &flapet24, &flapet25, &flapet26, &flapet27, &flapet28, &flapet29, &flapet30 ) ;
	vdefine( vlist5, &flactime, &flautime ) ;

	tabflds = vlist1 + vlist2 + vlist3 + vlist4 + vlist5 ;

	vcopy( "ZUPROF", uprof, MOVE ) ;
	vcopy( "ZRFLTBL", table, MOVE ) ;
	set_apphelp( "HPSP01A" ) ;
	ZRC = 4 ;
}


void PLRFLST1::PersonalFList( const string& p )
{
	int crp ;
	int csrrow ;

	string pgm    ;
	string msg    ;
	string panl   ;
	string flist1 ;
	string vlist  ;
	string ldate  ;
	string ltime  ;
	string csr    ;

	string acurtb   ;
	string asel     ;
	string afldescp ;
	string aflctime ;
	string aflutime ;
	string newname  ;
	string newdesc  ;
	string lcurtb   ;

	vlist = "ASEL ACURTB AFLDESCP AFLCTIME AFLUTIME NEWNAME NEWDESC LCURTB " ;
	vdefine( vlist, &asel, &acurtb, &afldescp, &aflctime, &aflutime, &newname, &newdesc, &lcurtb ) ;
	vdefine( "CRP", &crp ) ;

	vget( "ZCURTB", PROFILE ) ;
	lcurtb = zcurtb ;
	if ( lcurtb == "" )
	{
		lcurtb = "REFLIST" ;
		zcurtb = "REFLIST" ;
		vput( "ZCURTB", PROFILE ) ;
	}

	OpenTableRO() ;
	if ( RC == 8 )
	{
		createDefaultTable() ;
		OpenTableRO() ;
		if ( RC > 0 ) { abend() ; }
	}

	flist1 = "FLST1" + d2ds( taskid(), 3 ) ;
	tbcreate( flist1, "ACURTB", "(ASEL,AFLDESCP,AFLCTIME,AFLUTIME)", NOWRITE ) ;
	tbsort( flist1, "(ACURTB,C,A)" ) ;

	while ( true )
	{
		tbskip( table )  ;
		if ( RC == 8 ) { break ; }
		acurtb   = zcurtb   ;
		asel     = ""       ;
		afldescp = fladescp ;
		aflctime = flactime ;
		aflutime = flautime ;
		tbadd( flist1, "", "ORDER" ) ;
	}
	CloseTable() ;

	ztdsels = 0  ;
	ztdtop  = 0  ;
	csrrow  = 0  ;
	csr     = "" ;
	msg     = "" ;

	while ( true )
	{
		if ( msg == "" && ztdsels > 0 )
		{
			ztdsels-- ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( flist1 ) ;
			tbskip( flist1, ztdtop ) ;
			panl = ( p == "" ) ? "PLRFLST1" : "PLRFLST6" ;
		}
		else
		{
			panl = "" ;
		}
		tbdispl( flist1, panl, msg, csr, csrrow, 1, "YES", "CRP" ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		csr = "" ;
		if ( ztdsels == 0 && p != "" && zcurinx != 0 )
		{
			tbtop( flist1 ) ;
			tbskip( flist1, zcurinx ) ;
			ztdsels = 1   ;
			asel    = "L" ;
		}
		if ( asel == "A" )
		{
			addpop( "", 9, 9 ) ;
			display( "PLRFLST4", "", "ZCMD1" ) ;
			if ( RC == 0 )
			{
				vcopy( "ZDATEL", ldate, MOVE ) ;
				vcopy( "ZTIMEL", ltime, MOVE ) ;
				zcurtb = acurtb     ;
				OpenTableRO()       ;
				tbget( table )      ;
				CloseTable()        ;
				zcurtb   = newname  ;
				lcurtb   = newname  ;
				vput( "ZCURTB", PROFILE ) ;
				fladescp = newdesc  ;
				flactime = ldate    ;
				flautime = ldate + " " + ltime ;
				OpenTableUP()       ;
				tbsort( table, "(ZCURTB,C,A)" ) ;
				tbadd( table, "", "ORDER" ) ;
				CloseTable()        ;
				acurtb   = zcurtb   ;
				asel     = ""       ;
				afldescp = fladescp ;
				aflctime = flactime ;
				aflutime = flautime ;
				tbadd( flist1, "", "ORDER" ) ;
			}
			rempop() ;
		}
		else if ( asel == "D" )
		{
			if ( acurtb != "REFLIST" )
			{
				addpop( "", 9, 5 ) ;
				display( "PLRFLST7" ) ;
				if ( RC == 0 )
				{
					vget( "ZCURTB", PROFILE ) ;
					if ( zcurtb == acurtb )
					{
						zcurtb = "REFLIST" ;
						lcurtb = zcurtb    ;
						vput( "ZCURTB", PROFILE ) ;
					}
					zcurtb = acurtb    ;
					OpenTableUP()      ;
					tbdelete( table )  ;
					CloseTable()       ;
					tbdelete( flist1 ) ;
				}
				rempop() ;
			}
		}
		else if ( asel == "E" )
		{
			control( "DISPLAY", "SAVE" )    ;
			EditFileList( acurtb )          ;
			control( "DISPLAY", "RESTORE" ) ;
		}
		else if ( asel == "L" )
		{
			vcopy( "ZFLSTPGM", pgm, MOVE ) ;
			select( "PGM(" + pgm + ") PARM(LIST " + acurtb + " " + StoreFileList( acurtb ) + ")" ) ;
			ZRC = 4 ;
		}
		else if ( asel == "O" )
		{
			lcurtb = acurtb ;
			control( "DISPLAY", "SAVE" )    ;
			OpenFileList( acurtb )          ;
			control( "DISPLAY", "RESTORE" ) ;
			vput( "ZCURTB", PROFILE )       ;
			if ( ZRC == 0 ) { break ; }     ;
		}
		else if ( asel == "S" )
		{
			lcurtb = acurtb           ;
			zcurtb = acurtb           ;
			vput( "ZCURTB", PROFILE ) ;
		}
		asel = "" ;
		tbput( flist1, "", "ORDER" ) ;
	}
	tbend( flist1 )  ;
	vdelete( vlist ) ;
	vdelete( "CRP" ) ;
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
		zcurtb = list ;
		tbget( table ) ;
		if ( RC == 8 )
		{
			tbvclear( table ) ;
			vreplace( "ZCURTB", upper( list )+"*" ) ;
			tbsarg( table, "", "NEXT", "(ZCURTB,EQ)" ) ;
			tbscan( table ) ;
			if ( RC == 8 )
			{
				zcurtb = "REFLIST" ;
				tbget( table ) ;
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

	if ( zcurtb == "" )
	{
		zcurtb = "REFLIST" ;
		vput( "ZCURTB", PROFILE ) ;
	}

	OpenFileList( zcurtb ) ;
}


void PLRFLST1::EditFileList( const string& curtb )
{
	int i ;
	int crp ;
	int csrrow ;

	string msg   ;
	string zcmd1 ;
	string csr   ;
	string panl  ;

	string flist2 ;
	string bsel   ;
	string bfile  ;
	string vlist  ;
	string ldate  ;
	string ltime  ;

	bool modified ;

	vlist = "BSEL BFILE ZCMD1" ;
	vdefine( vlist, &bsel, &bfile, &zcmd1 ) ;
	vdefine( "CRP", &crp ) ;

	flist2 = "FLST2" + d2ds( taskid(), 3 ) ;
	tbcreate( flist2, "", "(BSEL,BFILE)", NOWRITE ) ;

	zcurtb = curtb ;
	OpenTableRO()  ;
	if ( RC == 8 )
	{
		createDefaultTable() ;
		OpenTableRO() ;
		if ( RC > 0 ) { abend() ; }
	}
	tbget( table ) ;

	bsel = "" ;
	for ( i = 1 ; i <= 30 ; i++ )
	{
		vcopy( "FLAPET" + d2ds( i, 2 ), bfile, MOVE ) ;
		if ( i > 1 && bfile == "" ) { continue ; }
		tbadd( flist2 ) ;
	}
	CloseTable() ;

	modified = false ;
	ztdsels  = 0  ;
	ztdtop   = 0  ;
	csrrow   = 0  ;
	csr      = "" ;
	msg      = "" ;
	while ( true )
	{
		if ( msg == "" && ztdsels > 0 )
		{
			ztdsels-- ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( flist2 ) ;
			tbskip( flist2, ztdtop ) ;
			panl = "PLRFLST2" ;
		}
		else
		{
			panl = "" ;
		}
		tbdispl( flist2, panl, msg, csr, csrrow, 1, "YES", "CRP" ) ;
		if ( zcmd1 == "CANCEL" ) { modified = false ; break ; }
		if ( RC == 8 ) { break ; }
		msg = "" ;
		csr = "" ;
		if ( bsel == "" )
		{
			tbput( flist2 ) ;
			modified = true ;
		}
		else if ( bsel == "I" )
		{
			bsel  = "" ;
			bfile = "" ;
			tbadd( flist2 ) ;
			modified = true ;
		}
		else if ( bsel == "R" )
		{
			bsel = "" ;
			tbadd( flist2 ) ;
			modified = true ;
		}
		else if ( bsel == "D" )
		{
			tbdelete( flist2 ) ;
			modified = true ;
		}
	}
	if ( modified )
	{
		OpenTableUP()  ;
		tbget( table ) ;
		tbtop( flist2 ) ;
		for ( i = 1 ; i <= 30 ; i++ )
		{
			tbskip( flist2 ) ;
			if ( RC > 0 ) { break ; }
			if ( bfile == "" ) { i-- ; continue ; }
			vreplace( "FLAPET" + d2ds( i, 2 ), bfile ) ;
		}
		for ( ; i <= 30 ; i++ )
		{
			vreplace( "FLAPET" + d2ds( i, 2 ), "" ) ;
		}
		vcopy( "ZDATEL", ldate, MOVE ) ;
		vcopy( "ZTIMEL", ltime, MOVE ) ;
		flautime = ldate + " " + ltime ;
		tbmod( table ) ;
		CloseTable() ;
	}
	tbend( flist2 )  ;
	vdelete( vlist ) ;
	vdelete( "CRP" ) ;
}


void PLRFLST1::OpenFileList( const string& curtb )
{
	int i ;
	int j ;
	int crp ;
	int csrrow ;

	string msg   ;
	string csr   ;
	string zcmd1 ;
	string panl  ;
	string flist3;
	string csel  ;
	string cfile ;
	string cnum  ;
	string vlist ;

	flist3 = "FLST3" + d2ds( taskid(), 3 ) ;
	tbcreate( flist3, "", "(CSEL,CFILE,CNUM)", NOWRITE ) ;

	zcurtb = curtb ;
	OpenTableRO()  ;
	if ( RC == 8 )
	{
		createDefaultTable() ;
		OpenTableRO() ;
		if ( RC > 0 ) { abend() ; }
	}

	tbget( table ) ;
	if ( RC == 8 )
	{
		zcurtb = "REFLIST" ;
		tbget( table ) ;
		if ( RC > 0 ) { CloseTable() ; return ; }
	}

	CloseTable() ;

	vlist = "CSEL CFILE CNUM ZCMD1" ;
	vdefine( vlist, &csel, &cfile, &cnum, &zcmd1 ) ;
	vdefine( "CRP", &crp ) ;

	csel = "" ;

	for ( j = 0, i = 1 ; i <= 30 ; i++ )
	{
		vcopy( "FLAPET" + d2ds( i, 2 ), cfile, MOVE ) ;
		if ( i > 1 && cfile == "" ) { continue ; }
		j++ ;
		cnum = d2ds( j ) ;
		tbadd( flist3 )  ;
	}

	ztdsels  = 0  ;
	ztdtop   = 0  ;
	csrrow   = 0  ;
	csr      = "" ;
	msg      = "" ;
	while ( true )
	{
		if ( msg == "" && ztdsels > 0 )
		{
			ztdsels-- ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( flist3 ) ;
			tbskip( flist3, ztdtop ) ;
			panl = "PLRFLST3" ;
		}
		else
		{
			panl = "" ;
		}
		tbdispl( flist3, panl, msg, csr, csrrow, 1, "YES", "CRP" ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		csr = "" ;
		if ( ztdsels == 0 && zcurinx != 0 )
		{
			tbtop( flist3 ) ;
			tbskip( flist3, zcurinx ) ;
			csel    = "S" ;
			ztdsels = 1   ;
		}
		if ( csel == "S" )
		{
			reffield = "#REFLIST" ;
			ZRESULT  = cfile      ;
			ZRC      = 0          ;
			vreplace( "ZCURTB", zcurtb ) ;
			vreplace( "ZRFNPOS", cnum )  ;
			vput( "ZCURTB ZRFNPOS", SHARED ) ;
			break ;
		}
	}
	tbend( flist3 )  ;
	vdelete( vlist ) ;
	vdelete( "CRP" ) ;
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

	zcurtb = curtb ;
	OpenTableRO()  ;
	if ( RC == 8 )
	{
		createDefaultTable() ;
		OpenTableRO() ;
		if ( RC > 0 ) { abend() ; }
	}
	tbget( table ) ;
	CloseTable()   ;

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

	string zrfnex  ;
	string zrfnpos ;

	vdefine( "ZRFNEX ZRFNPOS", &zrfnex, &zrfnpos ) ;

	iupper( list )  ;
	w1 = word( list, 1 ) ;
	w2 = word( list, 2 ) ;

	zrfnpos = "" ;

	if ( w2.size() > 0 && w2.size() < 3 && datatype( w2, 'W' ) )
	{
		swap( w1, w2 ) ;
	}

	if ( w1.size() > 0 && w1.size() < 3 && datatype( w1, 'W' ) )
	{
		list = w2 ;
		pos  = 1  ;
		if ( datatype( w1, 'W' ) ) { pos = ds2d( w1 ) - 1 ; }
		if ( pos < 30 )
		{
			zrfnpos = d2ds( pos ) ;
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
		list = zcurtb ;
	}
	else
	{
		zcurtb = list ;
		if ( zrfnpos == "" ) { verase( "ZRFNPOS", SHARED ) ; }
	}

	OpenTableRO()  ;
	tbget( table ) ;
	if ( RC == 8 )
	{
		tbvclear( table ) ;
		vreplace( "ZCURTB", upper( list )+"*" ) ;
		tbsarg( table, "", "NEXT", "(ZCURTB,EQ)" ) ;
		tbscan( table ) ;
		if ( RC == 8 )
		{
			zcurtb = "REFLIST" ;
			tbget( table ) ;
			if ( RC > 0 ) { CloseTable() ; return ; }
		}
	}
	CloseTable() ;

	vput( "ZCURTB", PROFILE ) ;
	vget( "ZRFNEX", PROFILE ) ;

	vget( "ZRFNPOS", SHARED ) ;
	if ( RC == 8 ) { p = 1 ; }
	else           { p = ( zrfnpos == "30" ) ? 1 : ds2d( zrfnpos ) + 1 ; }

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
		if ( zrfnex == "YES" )
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

	zrfnpos  = d2ds( p )  ;
	reffield = "#REFLIST" ;
	ZRC      = ( ZRESULT == "" ) ? 8 : 0 ;

	vput( "ZRFNPOS", SHARED )   ;
	vdelete( "ZRFNEX ZRFNPOS" ) ;
}


void PLRFLST1::RetrieveMatchEntry( string mfile )
{
	// Retrieve entry from the reference list that matches file name 'mfile'

	size_t p ;

	zcurtb = "REFLIST" ;
	OpenTableRO()  ;
	tbget( table ) ;
	if ( RC > 0 ) { CloseTable() ; return ; }

	CloseTable() ;
	for ( int i = 1 ; i <= 30 ; i++ )
	{
		vcopy( "FLAPET" + d2ds( i, 2 ), ZRESULT, MOVE ) ;
		if ( ZRESULT == "" )
		{
			continue ;
		}
		p = ZRESULT.find_last_of( '/' ) ;
		if ( p != string::npos && ZRESULT.compare( p + 1, mfile.size(), mfile ) == 0 )
		{
			return ;
		}
	}
	ZRESULT = "" ;
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

	zcurtb = "REFLIST" ;
	tbget( table ) ;
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

	for ( uint j = 2 ; j <= 30 ; j++ )
	{
		if ( j <= list.size()+1 )
		{
			vreplace( "FLAPET" + d2ds( j, 2 ), list.at( j-2 ) ) ;
		}
		else
		{
			vreplace( "FLAPET" + d2ds( j, 2 ), "" ) ;
		}
	}

	flautime = ldate + " " + ltime ;
	tbmod( table, "", "ORDER" ) ;
	CloseTable() ;
}


void PLRFLST1::createDefaultTable()
{
	string ldate ;
	string ltime ;

	tbcreate( table, "ZCURTB", "("+subword( tabflds, 2 )+")", WRITE, NOREPLACE, uprof ) ;
	if ( RC > 0 ) { return ; }

	tbvclear( table ) ;
	zcurtb = "REFLIST" ;
	vput( "ZCURTB", PROFILE ) ;
	vcopy( "ZDATEL", ldate, MOVE )   ;
	vcopy( "ZTIMEL", ltime, MOVE )   ;
	fladescp = "Default Reference List" ;
	flactime = ldate ;
	flautime = ldate + " " + ltime ;

	tbadd( table ) ;
	if ( RC > 0 ) { return ; }
	tbsort( table, "(ZCURTB,C,A)" ) ;
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
	tbopen( table, NOWRITE, uprof ) ;
	return ;
}


void PLRFLST1::OpenTableUP()
{
	tbopen( table, WRITE, uprof ) ;
	return ;
}


void PLRFLST1::CloseTable()
{
	tbclose( table, "", uprof ) ;
	return ;
}


// ============================================================================================ //

extern "C" { pApplication* maker() { return new PLRFLST1 ; } }
extern "C" { void destroy(pApplication* p) { delete p ; } }
