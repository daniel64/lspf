/*  Compile with ::                                                                       */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libplrflst1.so -o libplrflst1.so plrflst1.cpp */

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


/**********************************************************************************/
/*                                                                                */
/* Personal File List application                                                 */
/*                                                                                */
/* On exit, if CONTROL REFLIST ON and ZRC = 0 then                                */
/* ZRESULT will be placed in the field specified by the .NRET panel variable or   */
/* the cursor field.                                                              */
/* eg:                                                                            */
/* .NRET = ON                                                                     */
/* .NRET = ZFILE                                                                  */
/*                                                                                */
/**********************************************************************************/


#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

#include "../lspfall.h"
#include "plrflst1.h"

using namespace std ;
using namespace boost ;
using namespace boost::filesystem ;

LSPF_APP_MAKER( plrflst1 )


plrflst1::plrflst1()
{
	STANDARD_HEADER( "Personal File List application", "1.0.1" )

	const string vlist1 = "ZCURTB   FLADESCP FLAPET01 FLAPET02 FLAPET03 FLAPET04 FLAPET05 FLAPET06 " ;
	const string vlist2 = "FLAPET07 FLAPET08 FLAPET09 FLAPET10 FLAPET11 FLAPET12 FLAPET13 FLAPET14 " ;
	const string vlist3 = "FLAPET15 FLAPET16 FLAPET17 FLAPET18 FLAPET19 FLAPET20 FLAPET21 FLAPET22 " ;
	const string vlist4 = "FLAPET23 FLAPET24 FLAPET25 FLAPET26 FLAPET27 FLAPET28 FLAPET29 FLAPET30 " ;
	const string vlist5 = "FLACTIME FLAUTIME " ;

	vdefine( vlist1, &zcurtb,   &fladescp, &flapet01, &flapet02, &flapet03, &flapet04, &flapet05, &flapet06 ) ;
	vdefine( vlist2, &flapet07, &flapet08, &flapet09, &flapet10, &flapet11, &flapet12, &flapet13, &flapet14 ) ;
	vdefine( vlist3, &flapet15, &flapet16, &flapet17, &flapet18, &flapet19, &flapet20, &flapet21, &flapet22 ) ;
	vdefine( vlist4, &flapet23, &flapet24, &flapet25, &flapet26, &flapet27, &flapet28, &flapet29, &flapet30 ) ;
	vdefine( vlist5, &flactime, &flautime ) ;

	tabflds = vlist1 + vlist2 + vlist3 + vlist4 + vlist5 ;

	vdefine( "ZCURINX ZTDTOP ZTDSELS", &zcurinx, &ztdtop, &ztdsels ) ;
}


void plrflst1::application()
{
	string p1 ;
	string p2 ;

	vcopy( "ZRFLTBL", table, MOVE ) ;

	vcopy( "ZUSER", zuser ) ;
	vcopy( "ZSCREEN", zscreen ) ;

	ZRC = 4 ;

	auto it = pgm_parms.find( word( PARM, 1 ) ) ;
	if ( it == pgm_parms.end() )
	{
		llog( "E", "Invalid parameter passed to PLRFLST1: " << PARM << endl ) ;
		return ;
	}

	switch ( it->second )
	{
	case RR_PL1:
		    p1 = upper( word( PARM, 2 ) ) ;
		    p2 = word( PARM, 3 ) ;
		    OpenActiveRefList( p1, p2 ) ;
		    break ;

	case RR_PL2:
		    PersonalFileList() ;
		    break ;

	case RR_PL3:
		    PersonalFileList( "DSL" ) ;
		    break ;

	case RR_PL4:
		    PersonalFileList( "DSX" ) ;
		    break ;

	case RR_PLA:
		    p1 = upper( subword( PARM, 2 ) ) ;
		    AddReferralEntry( p1 ) ;
		    break ;

	case RR_PLR:
		    p1 = subword( PARM, 2 ) ;
		    AddReflistEntry( p1 ) ;
		    break ;

	case RR_PLF:
		    p1 = subword( PARM, 2 ) ;
		    AddFilelistEntry( p1 ) ;
		    break ;

	case RR_NR1:
		    p1 = subword( PARM, 2 ) ;
		    RetrieveEntry( p1 )  ;
		    break ;

	case RR_MTC:
		    p1 = word( PARM, 2 ) ;
		    p2 = word( PARM, 3 ) ;
		    RetrieveMatchEntry( p1, p2 ) ;
		    break ;

	case RR_US1:
		    userSettings() ;
		    break ;
	}
}


void plrflst1::PersonalFileList( const string& p )
{
	int src ;
	int crp ;
	int crpx ;
	int csrrow ;

	bool rebuild = false ;
	bool cur2sel = false ;

	string msg    ;
	string panl   ;
	string flist1 ;
	string ldate  ;
	string ltime  ;
	string cursor ;
	string filter ;
	string autosel ;

	string acurtb   ;
	string asel     ;
	string afldescp ;
	string aflctime ;
	string aflutime ;
	string plract   ;
	string plrrest  ;
	string newname  ;
	string newdesc  ;
	string lcurtb   ;

	const string vlist1 = "ASEL ACURTB AFLDESCP AFLCTIME AFLUTIME NEWNAME NEWDESC LCURTB PLRACT PLRREST" ;

	vdefine( vlist1, &asel, &acurtb, &afldescp, &aflctime, &aflutime, &newname, &newdesc, &lcurtb, &plract, &plrrest ) ;
	vdefine( "CRP", &crp ) ;

	vget( "ZCURTB", PROFILE ) ;
	vreplace( "PLRSTRTA", p ) ;

	lcurtb = zcurtb ;
	if ( lcurtb == "" )
	{
		lcurtb = "REFLIST" ;
		zcurtb = "REFLIST" ;
		vput( "ZCURTB", PROFILE ) ;
	}

	flist1 = "FL1" + d2ds( taskid(), 5 ) ;

	OpenTableRO() ;
	loadTBTable_all( flist1 ) ;
	CloseTable() ;

	ztdsels = 0  ;
	ztdtop  = 0  ;
	csrrow  = 0  ;
	crpx    = 0  ;
	cursor  = "" ;
	msg     = "" ;
	autosel = "NO" ;

	filter  = "*" ;

	while ( true )
	{
		if ( msg == "" && ztdsels > 0 )
		{
			--ztdsels ;
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
		if ( ztdsels == 0 && rebuild )
		{
			OpenTableRO() ;
			loadTBTable_all( flist1 ) ;
			tbskip( flist1, ztdtop ) ;
			CloseTable() ;
			rebuild = false ;
		}
		if ( msg != "" && cursor == "" )
		{
			cursor  = "ASEL" ;
			csrrow  = crpx  ;
			cur2sel = false ;
		}
		else if ( cur2sel )
		{
			cursor  = "ASEL" ;
			csrrow  = crpx  ;
			cur2sel = false ;
			autosel = "NO" ;
		}
		else
		{
			cursor = "ZCMD" ;
		}
		tbvclear( flist1 ) ;
		acurtb = filter ;
		tbsarg( flist1 ) ;
		vreplace( "ZTDMSG", ( filter == "*" ) ? "PSYZ003" : "" ) ;
		tbdispl( flist1,
			 panl,
			 msg,
			 cursor,
			 csrrow,
			 1,
			 autosel,
			 "CRP" ) ;
		if ( RC == 8 ) { break ; }
		msg     = "" ;
		cursor  = "" ;
		crpx    = crp ;
		crp     = 0 ;
		csrrow  = 0 ;
		autosel = "YES" ;
		if ( plract == "NEW" )
		{
			EditNewFileList() ;
			rebuild = true ;
			ztdsels = 0 ;
			continue ;
		}
		else if ( plract == "RESET" )
		{
			filter  = "*" ;
			ztdtop  = 1 ;
			ztdsels = 0 ;
			continue ;
		}
		else if ( plract == "FILTER" )
		{
			filter = word( plrrest, 1 ) + "*" ;
			ztdtop = 1 ;
			if ( zcurinx == 0 )
			{
				continue ;
			}
		}
		if ( ztdsels == 0 && zcurinx > 0 )
		{
			tbtop( flist1 ) ;
			tbskip( flist1, zcurinx ) ;
			ztdsels = 1 ;
			asel    = ( p == "" ) ? "S" : ( p == "DSL" ) ? "L" : "X" ;
			crpx    = zcurinx ;
		}
		if ( asel != "" )
		{
			cur2sel = true ;
		}
		if ( asel == "A" )
		{
			addpop( "", 9, 9 ) ;
			msg = "" ;
			newname = acurtb ;
			newdesc = afldescp ;
			while ( true )
			{
				display( "PLRFLST4", msg, "ZCMD1" ) ;
				msg = "" ;
				if ( RC == 0 )
				{
					OpenTableRO() ;
					zcurtb = newname ;
					tbget( table ) ;
					if ( RC == 0 )
					{
						msg = "LRFL011C" ;
						CloseTable() ;
						continue ;
					}
					CloseTable() ;
					vcopy( "ZDATESTD", ldate, MOVE ) ;
					vcopy( "ZTIMEL", ltime, MOVE ) ;
					OpenTableRO()       ;
					zcurtb = acurtb     ;
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
					CloseTable() ;
					rebuild = true ;
				}
				break ;
			}
			rempop() ;
		}
		else if ( asel == "D" )
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
				zcurtb = acurtb   ;
				OpenTableUP()     ;
				tbdelete( table ) ;
				if ( acurtb == "REFLIST" )
				{
					createReflistEntry() ;
				}
				CloseTable() ;
				rebuild = true ;
			}
			rempop() ;
		}
		else if ( asel == "E" )
		{
			control( "DISPLAY", "SAVE" )    ;
			EditFileList( acurtb )          ;
			control( "DISPLAY", "RESTORE" ) ;
			rebuild = true ;
		}
		else if ( asel == "L" )
		{
			select( "PGM(PPSP01A) PARM(DSL " + acurtb + ")" ) ;
			ZRC = 4 ;
		}
		else if ( asel == "X" )
		{
			select( "PGM(PPSP01A) PARM(DSX " + acurtb + ")" ) ;
		}
		else if ( asel == "O" || asel == "S" )
		{
			lcurtb = acurtb ;
			control( "DISPLAY", "SAVE" )    ;
			OpenFileList( acurtb )          ;
			control( "DISPLAY", "RESTORE" ) ;
			vput( "ZCURTB", PROFILE ) ;
			control( "ERRORS", "RETURN" ) ;
			tbquery( flist1 ) ;
			src = RC ;
			control( "ERRORS", "CANCEL" ) ;
			if ( src == 12 )
			{
				vdelete( vlist1, "CRP" ) ;
				return ;
			}
			if ( ZRC == 0 ) { break ; }
			rebuild = true ;
		}
		asel = "" ;
	}

	tbend( flist1 ) ;
	vdelete( vlist1, "CRP" ) ;
}


void plrflst1::loadTBTable_all( const string& tbtable )
{
	string asel ;
	string acurtb ;
	string afldescp ;
	string aflctime ;
	string aflutime ;

	const string vlist1 = "ASEL ACURTB AFLDESCP AFLCTIME AFLUTIME" ;

	vdefine( vlist1, &asel, &acurtb, &afldescp, &aflctime, &aflutime ) ;

	tbcreate( tbtable,
		  "ACURTB",
		  "(ASEL,AFLDESCP,AFLCTIME,AFLUTIME)",
		  NOWRITE,
		  REPLACE ) ;

	tbskip( table ) ;
	while ( RC == 0 )
	{
		acurtb   = zcurtb   ;
		afldescp = fladescp ;
		aflctime = flactime ;
		aflutime = flautime ;
		tbadd( tbtable ) ;
		tbskip( table ) ;
	}

	tbsort( tbtable, "(ACURTB,C,A)" ) ;
	tbtop( tbtable ) ;

	vdelete( vlist1 ) ;
}


void plrflst1::OpenActiveRefList( const string& list,
				  const string& posn )
{
	//
	// Open the active referral list or the one specified in 'list' (can be a partial name - first alphabetically).
	// Make 'list' the active one unless it is the REFLIST we are opening.
	// If list position posn specified, retrieve the entry, otherwise show the dialogue.
	//

	int p ;

	if ( list != "" )
	{
		OpenTableRO() ;
		zcurtb = list ;
		tbget( table ) ;
		if ( RC == 8 )
		{
			tbvclear( table ) ;
			vreplace( "ZCURTB", list+"*" ) ;
			tbsarg( table, "", "NEXT", "(ZCURTB,EQ)" ) ;
			tbscan( table ) ;
			if ( RC == 8 )
			{
				zcurtb = "REFLIST" ;
				tbget( table ) ;
				if ( RC == 8 )
				{
					CloseTable() ;
					OpenTableUP() ;
					createReflistEntry() ;
				}
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

	if ( posn != "" && datatype( posn, 'W' ) )
	{
		p = ds2d( posn ) ;
		if ( p > 0 && p < 31 )
		{
			OpenTableRO() ;
			tbget( table ) ;
			CloseTable() ;
			vcopy( "FLAPET" + d2ds( p, 2 ), ZRESULT ) ;
			control( "REFLIST", "ON" ) ;
			vreplace( "ZRFNPOS", d2ds( p ) ) ;
			vreplace( "ZRFNCRTB", zcurtb ) ;
			vput( "ZRFNPOS ZRFNCRTB", SHARED ) ;
			ZRC = ( ZRESULT == "" ) ? 8 : 0 ;
			return ;
		}
	}

	OpenFileList( zcurtb ) ;

	vreplace( "ZRFNCRTB", zcurtb ) ;
	vput( "ZRFNCRTB", SHARED ) ;
}


void plrflst1::EditFileList( const string& curtb )
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
	string odesc  ;

	const string vlist1 = "BSEL BFILE ZCMD1" ;

	bool modified ;

	vdefine( vlist1, &bsel, &bfile, &zcmd1 ) ;
	vdefine( "CRP", &crp ) ;

	flist2 = "FL2" + d2ds( taskid(), 5 ) ;
	tbcreate( flist2,
		  "",
		  "(BSEL,BFILE)",
		  NOWRITE ) ;

	OpenTableRO()  ;

	zcurtb = curtb ;
	tbget( table ) ;
	if ( RC > 0 )
	{
		CloseTable() ;
		vdelete( vlist1, "CRP" ) ;
		return ;
	}

	odesc = fladescp ;
	bsel  = "" ;
	for ( i = 1 ; i <= 30 ; ++i )
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
			--ztdsels ;
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
		tbdispl( flist2,
			 panl,
			 msg,
			 csr,
			 csrrow,
			 1,
			 "YES",
			 "CRP" ) ;
		if ( zcmd1 == "CANCEL" )
		{
			modified = false ;
			odesc    = "" ;
			fladescp = "" ;
			break ;
		}
		if ( RC == 8 ) { break ; }
		if ( zcmd1 == "SORT" )
		{
			tbsort( flist2, "(BFILE,C,A)" ) ;
			modified = true ;
			ztdsels  = 0  ;
			zcmd1    = "" ;
			continue ;
		}
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

	if ( modified || odesc != fladescp )
	{
		saveListEntry( flist2, "BFILE", fladescp ) ;
	}

	tbend( flist2 )  ;
	vdelete( vlist1, "CRP" ) ;
}


void plrflst1::EditNewFileList( const string& curtb )
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
	string ldate  ;
	string ltime  ;
	string newname ;
	string newdesc ;

	const string vlist1 = "BSEL BFILE ZCMD1" ;

	vdefine( vlist1, &bsel, &bfile, &zcmd1 ) ;
	vdefine( "CRP", &crp ) ;

	flist2 = "FL2" + d2ds( taskid(), 5 ) ;
	tbcreate( flist2,
		  "",
		  "(BSEL,BFILE)",
		  NOWRITE ) ;

	if ( curtb != "" )
	{
		OpenTableRO()  ;
		zcurtb = curtb ;
		tbget( table ) ;
		if ( RC > 0 )
		{
			CloseTable() ;
			return ;
		}

		bsel = "" ;
		for ( i = 1 ; i <= 30 ; ++i )
		{
			vcopy( "FLAPET" + d2ds( i, 2 ), bfile, MOVE ) ;
			if ( i > 1 && bfile == "" ) { continue ; }
			tbadd( flist2 ) ;
		}
		CloseTable() ;
	}
	else
	{
		tbadd( flist2 ) ;
	}

	zcurtb   = "" ;
	fladescp = "" ;
	flactime = "" ;
	flautime = "" ;
	ztdsels  = 0  ;
	ztdtop   = 0  ;
	csrrow   = 0  ;
	csr      = "" ;
	msg      = "" ;
	while ( true )
	{
		if ( msg == "" && ztdsels > 0 )
		{
			--ztdsels ;
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
		tbdispl( flist2,
			 panl,
			 msg,
			 csr,
			 csrrow,
			 1,
			 "YES",
			 "CRP" ) ;
		if ( zcmd1 == "CANCEL" )
		{
			tbend( flist2 )  ;
			vdelete( vlist1 ) ;
			return ;
		}
		if ( RC == 8 ) { break ; }
		if ( zcmd1 == "SORT" )
		{
			tbsort( flist2, "(BFILE,C,A)" ) ;
			ztdsels  = 0  ;
			zcmd1    = "" ;
			continue ;
		}
		msg = "" ;
		csr = "" ;
		if ( bsel == "" )
		{
			tbput( flist2 ) ;
		}
		else if ( bsel == "I" )
		{
			bsel  = "" ;
			bfile = "" ;
			tbadd( flist2 ) ;
		}
		else if ( bsel == "R" )
		{
			bsel = "" ;
			tbadd( flist2 ) ;
		}
		else if ( bsel == "D" )
		{
			tbdelete( flist2 ) ;
		}
	}

	vdefine( "NEWNAME NEWDESC", &newname, &newdesc ) ;
	msg     = "" ;
	newname = "" ;
	newdesc = fladescp ;

	addpop( "", 9, 9 ) ;
	while ( true )
	{
		display( "PLRFLST8", msg, "ZCMD1" ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		OpenTableRO() ;
		zcurtb   = newname ;
		tbget( table ) ;
		if ( RC == 0 )
		{
			msg = "LRFL011C" ;
			CloseTable() ;
			continue ;
		}
		CloseTable() ;
		OpenTableUP() ;
		for ( i = 1 ; i <= 30 && RC == 0 ; )
		{
			vcopy( "BFILE", bfile ) ;
			if ( bfile != "" )
			{
				vreplace( "FLAPET" + d2ds( i, 2 ), bfile ) ;
				++i ;
			}
			tbskip( flist2 ) ;
		}
		for ( ; i <= 30 ; ++i )
		{
			vreplace( "FLAPET" + d2ds( i, 2 ), "" ) ;
		}
		vcopy( "ZDATESTD", ldate, MOVE ) ;
		vcopy( "ZTIMEL", ltime, MOVE ) ;
		flactime = ldate   ;
		flautime = ldate + " " + ltime ;
		fladescp = newdesc ;
		zcurtb   = newname ;
		tbsort( table, "(ZCURTB,C,A)" ) ;
		tbadd( table, "", "ORDER" ) ;
		CloseTable() ;
		vput( "ZCURTB", PROFILE ) ;
		break ;
	}
	rempop() ;

	tbend( flist2 )  ;
	vdelete( vlist1 ) ;
	vdelete( "CRP NEWNAME NEWDESC" ) ;
}


void plrflst1::OpenFileList( const string& curtb )
{
	int crp ;

	bool modified = false ;

	string msg   ;
	string zcmd1 ;
	string panl  ;
	string csel  ;
	string cfile ;
	string cnum  ;
	string ccurtb ;
	string flist3 ;
	string flistact ;
	string newname ;
	string newdesc ;
	string ldate ;
	string ltime ;
	string odesc ;

	const string vlist1 = "CSEL CFILE CNUM ZCMD1 FLISTACT NEWNAME NEWDESC" ;

	flist3 = "FL3" + d2ds( taskid(), 5 ) ;

	OpenTableRO()  ;

	zcurtb = curtb ;
	tbget( table ) ;
	if ( RC == 8 )
	{
		zcurtb = "REFLIST" ;
		tbget( table ) ;
		if ( RC == 8 )
		{
			CloseTable() ;
			OpenTableUP() ;
			createReflistEntry() ;
		}
	}
	CloseTable() ;

	ccurtb = zcurtb ;
	odesc  = fladescp ;

	vdefine( vlist1, &csel, &cfile, &cnum, &zcmd1, &flistact, &newname, &newdesc ) ;
	vdefine( "CRP", &crp ) ;

	loadTBTable( flist3 ) ;

	ztdsels = 0  ;
	ztdtop  = 0  ;
	msg     = "" ;

	while ( true )
	{
		if ( ztdsels > 0 )
		{
			--ztdsels ;
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
		tbdispl( flist3, panl, msg ) ;
		if ( RC == 8 )
		{
			if ( zcmd1 == "CANCEL" )
			{
				modified = false ;
				odesc    = ""    ;
				fladescp = ""    ;
			}
			flistact = "" ;
			break ;
		}
		msg = "" ;
		if ( flistact == "N" || flistact == "O" )
		{
			break ;
		}
		if ( flistact == "A" )
		{
			addpop( "", 9, 9 ) ;
			msg = "" ;
			newname = zcurtb ;
			newdesc = fladescp ;
			while ( true )
			{
				display( "PLRFLST4", msg, "ZCMD1" ) ;
				msg = "" ;
				if ( RC == 0 )
				{
					OpenTableRO() ;
					zcurtb = newname ;
					tbget( table ) ;
					if ( RC == 0 )
					{
						msg = "LRFL011C" ;
						CloseTable() ;
						continue ;
					}
					CloseTable() ;
					vcopy( "ZDATESTD", ldate, MOVE ) ;
					vcopy( "ZTIMEL", ltime, MOVE ) ;
					OpenTableRO()     ;
					zcurtb = ccurtb   ;
					tbget( table )    ;
					CloseTable()      ;
					zcurtb = newname  ;
					ccurtb = newname  ;
					vput( "ZCURTB", PROFILE ) ;
					fladescp = newdesc  ;
					odesc    = newdesc  ;
					flactime = ldate    ;
					flautime = ldate + " " + ltime ;
					OpenTableUP()       ;
					tbsort( table, "(ZCURTB,C,A)" ) ;
					tbadd( table, "", "ORDER" ) ;
					CloseTable()        ;
					loadTBTable( flist3 ) ;
				}
				break ;
			}
			ztdsels = 0 ;
			rempop() ;
			continue ;
		}
		if ( flistact == "D" )
		{
			addpop( "", 9, 5 ) ;
			display( "PLRFLST7" ) ;
			if ( RC == 0 )
			{
				OpenTableUP() ;
				tbdelete( table ) ;
				if ( zcurtb == "REFLIST" )
				{
					createReflistEntry() ;
				}
				else
				{
					zcurtb = "REFLIST" ;
					vput( "ZCURTB", PROFILE ) ;
				}
				tbget( table ) ;
				loadTBTable( flist3 ) ;
				CloseTable() ;
				ztdsels = 0 ;
			}
			rempop() ;
			continue ;
		}
		if ( flistact == "L" )
		{
			select( "PGM(PPSP01A) PARM(DSL " + zcurtb + ")" ) ;
			ztdsels = 0 ;
			continue ;
		}
		if ( flistact == "X" )
		{
			select( "PGM(PPSP01A) PARM(DSX " + zcurtb + ")" ) ;
			ztdsels = 0 ;
			continue ;
		}
		if ( flistact == "" && ztdsels == 0 && zcurinx > 0 )
		{
			tbtop( flist3 ) ;
			tbskip( flist3, zcurinx ) ;
			csel    = "S" ;
			ztdsels = 1   ;
		}
		if ( csel == "S" )
		{
			control( "REFLIST", "ON" ) ;
			ZRESULT  = cfile ;
			ZRC      = 0     ;
			vreplace( "ZCURTB", zcurtb ) ;
			vreplace( "ZRFNPOS", cnum )  ;
			vput( "ZCURTB ZRFNPOS", SHARED ) ;
			break ;
		}
		else if ( ztdsels > 0 && csel == "" )
		{
			tbput( flist3 ) ;
			modified = true ;
		}
		if ( ztdsels < 2 && flistact == "E" )
		{
			if ( modified || odesc != fladescp )
			{
				saveListEntry( flist3, "CFILE", fladescp ) ;
			}
			control( "DISPLAY", "SAVE" ) ;
			EditFileList( zcurtb ) ;
			control( "DISPLAY", "RESTORE" ) ;
			OpenTableUP() ;
			tbget( table ) ;
			CloseTable() ;
			loadTBTable( flist3 ) ;
			modified = false ;
			continue ;
		}
		if ( ztdsels < 2 && flistact == "S" )
		{
			saveListEntry( flist3, "CFILE", fladescp ) ;
			msg = "LRFL011E" ;
			continue ;
		}
	}

	if ( modified || odesc != fladescp )
	{
		saveListEntry( flist3, "CFILE", fladescp ) ;
	}

	tbend( flist3 ) ;

	if ( flistact == "O" )
	{
		PersonalFileList() ;
	}
	else if ( flistact == "N" )
	{
		EditNewFileList( zcurtb ) ;
	}

	vdelete( vlist1, "CRP" ) ;
}


void plrflst1::saveListEntry( const string& tbtable,
			      const string& fname,
			      const string ndesc )
{
	int i ;

	string afile ;
	string ldate ;
	string ltime ;

	OpenTableUP() ;

	tbget( table ) ;
	if ( RC > 0 )
	{
		CloseTable() ;
		return ;
	}

	tbtop( tbtable ) ;
	tbskip( tbtable ) ;

	for ( i = 1 ; i <= 30 && RC == 0 ; )
	{
		vcopy( fname, afile ) ;
		if ( afile != "" )
		{
			vreplace( "FLAPET" + d2ds( i, 2 ), afile ) ;
			++i ;
		}
		tbskip( tbtable ) ;
	}

	for ( ; i <= 30 ; ++i )
	{
		vreplace( "FLAPET" + d2ds( i, 2 ), "" ) ;
	}

	vcopy( "ZDATESTD", ldate, MOVE ) ;
	vcopy( "ZTIMEL", ltime, MOVE ) ;
	flautime = ldate + " " + ltime ;
	fladescp = ndesc ;

	tbmod( table, "", "ORDER" ) ;

	CloseTable() ;
}


void plrflst1::loadTBTable( const string& tbtable )
{
	int i ;
	int j ;

	string csel  ;
	string cfile ;
	string cnum  ;

	const string vlist1 = "CSEL CFILE CNUM" ;
	tbcreate( tbtable,
		  "",
		  "(CSEL,CFILE,CNUM)",
		  NOWRITE,
		  REPLACE ) ;

	vdefine( vlist1, &csel, &cfile, &cnum ) ;

	for ( j = 0, i = 1 ; i <= 30 ; ++i )
	{
		vcopy( "FLAPET" + d2ds( i, 2 ), cfile, MOVE ) ;
		if ( i > 1 && cfile == "" ) { continue ; }
		++j ;
		cnum = d2ds( j ) ;
		tbadd( tbtable ) ;
	}

	tbtop( tbtable ) ;
	vdelete( vlist1 ) ;
}


void plrflst1::RetrieveEntry( string list )
{
	//
	// Retrieve entry from the active referral list or the one specified in 'list', at posn ZRFNPOS.
	// Also, make 'list' the active one so the next NRETRIEV with no parameters, is from the same list.
	// List can be a partial name (first alphabetically).
	//
	// If ZRFNEX is YES, check file exists, else get the next entry.
	//
	// If 'list' consists a number < 31, use this as the starting position for the retrieve and not
	// variable ZRFNPOS from the SHARED pool.  If specified, the other word in 'list' is used as the reflist.
	// If 'list' is specified, but not a number, start at position 1.
	//

	int i   ;
	int p   ;
	int fp  ;
	int pos ;

	bool skip ;

	string w1 ;
	string w2 ;

	string zrfnex  ;
	string zrfnpos ;

	const string vlist1 = "ZRFNEX ZRFNPOS" ;

	vdefine( vlist1, &zrfnex, &zrfnpos ) ;

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
		vreplace( "ZCURTB", list+"*" ) ;
		tbsarg( table, "", "NEXT", "(ZCURTB,EQ)" ) ;
		tbscan( table ) ;
		if ( RC == 8 )
		{
			zcurtb = "REFLIST" ;
			tbget( table ) ;
			if ( RC > 0 )
			{
				CloseTable() ;
				vdelete( vlist1 ) ;
				return ;
			}
		}
	}

	CloseTable() ;

	vput( "ZCURTB", PROFILE ) ;
	vget( "ZRFNEX", PROFILE ) ;

	vget( "ZRFNPOS", SHARED ) ;
	p = ( RC == 8 ) ? 1 :
	    ( zrfnpos == "30" ) ? 1 : ds2d( zrfnpos ) + 1 ;

	fp = p ;
	for ( i = 1 ; i <= 30 ; ++i )
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
					throw filesystem_error( "Not a regular file", error_code() ) ;
				}
			}
			catch ( const filesystem_error& ex )
			{
				ZRESULT = ""   ;
				skip    = true ;
			}
			if ( !skip ) { break ; }
			++p ;
			if ( fp == p ) { break ; }
			continue ;
		}
		break ;
	}

	zrfnpos = d2ds( p ) ;
	control( "REFLIST", "ON" ) ;
	ZRC     = ( ZRESULT == "" ) ? 8 : 0 ;

	vreplace( "ZRFNCRTB", zcurtb ) ;
	vput( "ZRFNPOS ZRFNCRTB", SHARED ) ;
	vdelete( vlist1 ) ;
}


void plrflst1::RetrieveMatchEntry( const string& a,
				   const string& b,
				   uint itr )
{
	//
	// Retrieve regular file entry from the referral list that matches file name 'mfile'.
	//
	// refl and mfile can be specified either way round.  If 'a' is not a referral list, b is assumed to be.
	// If no match on the referral list, try a partial match (first alphabetically).
	// mfile can be numeric (from 1 to 30) to refer to the position in the referral list.
	//
	// If the referral list contains a generic entry, expand the entry to include all files that match.
	// If the referral list contains a directory entry, expand 1 level (except for REFLIST).
	//
	// Search sequence:
	// 1) If mfile contains '*' or '?', try a case insensitive match using regex.
	// 2) Case sensitive full match.
	// 3) Case insensitive full match.
	// 4) Case sensitive partial match.
	// 5) Case insensitive partial match.
	// 6) Case insensitive match anywhere.
	//
	// If no match has been found and the entries have not been swapped (potential for b to be the reflist also)
	// call procedure again swapping parameters.
	//

	size_t p ;

	bool useRegex ;
	bool numEntry = false ;
	bool swapped  = false ;

	string* t ;

	string temp ;
	string upat ;

	string pat   = "" ;
	string refl  = a ;
	string mfile = b ;

	regex expression ;

	vector<string> entries ;
	vector<size_t> poss ;
	set<string> processed ;

	if ( refl.size() > 0 && refl.size() < 3 && datatype( refl, 'W' ) )
	{
		numEntry = true ;
		swap( refl, mfile ) ;
	}
	else if ( mfile.size() > 0 && mfile.size() < 3 && datatype( mfile, 'W' ) )
	{
		numEntry = true ;
	}

	OpenTableRO() ;

	zcurtb = upper( refl ) ;
	tbget( table ) ;
	if ( RC == 8 )
	{
		zcurtb = upper( mfile ) ;
		tbget( table ) ;
		if ( RC == 8 )
		{
			tbsort( table, "(ZCURTB,C,A)" ) ;
			tbvclear( table ) ;
			zcurtb = upper( refl ) + "*" ;
			tbsarg( table, "", "NEXT", "(ZCURTB,EQ)" ) ;
			tbscan( table ) ;
			if ( RC == 8 )
			{
				tbvclear( table ) ;
				zcurtb = upper( mfile ) + "*" ;
				tbtop( table ) ;
				tbsarg( table, "", "NEXT", "(ZCURTB,EQ)" ) ;
				tbscan( table ) ;
				if ( RC == 8 )
				{
					CloseTable() ;
					return ;
				}
				swap( refl, mfile ) ;
				swapped = true ;
			}
		}
		else
		{
			swap( refl, mfile ) ;
			swapped = true ;
		}
	}
	CloseTable() ;

	if ( numEntry )
	{
		p = ds2d( mfile ) ;
		if ( p > 0 && p < 31 )
		{
			vcopy( "FLAPET" + d2ds( p, 2 ), ZRESULT, MOVE ) ;
			try
			{
				if ( !is_regular_file( ZRESULT ) )
				{
					throw invalid_argument( "Not a regular file" ) ;
				}
			}
			catch (...)
			{
				ZRESULT = "" ;
			}
			return ;
		}
	}

	for ( int i = 1 ; i <= 30 ; ++i )
	{
		vcopy( "FLAPET" + d2ds( i, 2 ), t, LOCATE ) ;
		if ( t && !t->empty() && processed.find( *t ) == processed.end() )
		{
			p = t->find_last_of( '/' ) ;
			if ( p != string::npos )
			{
				if ( t->find_first_of( "?*[", p ) != string::npos )
				{
					temp = t->substr( p + 1 ) ;
					add_files( t->erase( p ),
						   temp,
						   entries,
						   poss,
						   processed ) ;
				}
				else
				{
					try
					{
						if ( zcurtb != "REFLIST" && is_directory( *t ) )
						{
							add_files( *t,
								   "*",
								   entries,
								   poss,
								   processed ) ;
						}
						else
						{
							try
							{
								if ( !is_regular_file( *t ) )
								{
									throw invalid_argument( "Not a regular file" ) ;
								}
							}
							catch (...)
							{
								processed.insert( *t ) ;
								continue ;
							}
							entries.push_back( *t ) ;
							poss.push_back( p + 1 ) ;
						}
					}
					catch (...)
					{
						processed.insert( *t ) ;
					}
				}
			}
			processed.insert( *t ) ;
		}
	}

	useRegex = ( mfile.find_first_of( "?*" ) != string::npos ) ;

	if ( useRegex )
	{
		pat = conv_regex( mfile, '?', '*' ) ;
		try
		{
			expression.assign( pat, boost::regex_constants::icase ) ;
		}
		catch  ( boost::regex_error& e )
		{
			useRegex = false ;
		}
	}

	if ( useRegex )
	{
		for ( size_t i = 0 ; i < entries.size() ; ++i )
		{
			string& str = entries.at( i ) ;
			if ( regex_match( str.begin() + poss[ i ], str.end(), expression ) )
			{
				ZRESULT = str ;
				break ;
			}
		}
		return ;
	}

	pat = mfile ;

	for ( size_t i = 0 ; i < entries.size() ; ++i )
	{
		string& str = entries.at( i ) ;
		p = poss[ i ] ;
		if ( str.substr( p ) == pat )
		{
			ZRESULT = str ;
			return ;
		}
	}

	upat = upper( pat ) ;
	for ( size_t i = 0 ; i < entries.size() ; ++i )
	{
		string& str = entries.at( i ) ;
		p = poss[ i ] ;
		if ( upper( str.substr( p ) ) == upat )
		{
			ZRESULT = str ;
			return ;
		}
	}

	for ( size_t i = 0 ; i < entries.size() ; ++i )
	{
		string& str = entries.at( i ) ;
		p = poss[ i ] ;
		if ( str.compare( p, pat.size(), pat ) == 0 && ( p + pat.size() ) == str.size() )
		{
			ZRESULT = str ;
			return ;
		}
	}

	for ( size_t i = 0 ; i < entries.size() ; ++i )
	{
		string& str = entries.at( i ) ;
		if ( str.compare( poss[ i ], pat.size(), pat ) == 0 )
		{
			ZRESULT = str ;
			return ;
		}
	}

	for ( size_t i = 0 ; i < entries.size() ; ++i )
	{
		string& str = entries.at( i ) ;
		if ( upper( str ).compare( poss[ i ], upat.size(), upat ) == 0 )
		{
			ZRESULT = str ;
			return ;
		}
	}

	for ( size_t i = 0 ; i < entries.size() ; ++i )
	{
		string& str = entries.at( i ) ;
		if ( upper( str ).find( upat, poss[ i ] ) != string::npos )
		{
			ZRESULT = str ;
			return ;
		}
	}

	if ( !swapped && !numEntry && itr < 2 )
	{
		RetrieveMatchEntry( b, a, ++itr ) ;
	}
}


void plrflst1::add_files( const string& xpath,
			  const string& gen,
			  vector<string>& entries,
			  vector<size_t>& poss,
			  set<string>& processed )
{
	//
	// Add regular files to vector entries, from path xpath using generic gen.
	//

	size_t p ;

	vector<path> vt ;

	regex expression ;

	try
	{
		copy( directory_iterator( xpath ), directory_iterator(), back_inserter( vt ) ) ;
	}
	catch (...)
	{
		return ;
	}

	sort( vt.begin(), vt.end() ) ;

	try
	{
		expression.assign( conv_regex( upper( gen ), '?', '*' ), boost::regex_constants::icase ) ;
	}
	catch  (...)
	{
		return ;
	}

	for ( auto& ent : vt )
	{
		p = ent.string().find_last_of( '/' ) ;
		if ( p != string::npos )
		{
			if ( processed.count( ent.string() ) == 0 )
			{
				if ( regex_match( ent.string().begin() + p + 1, ent.string().end(), expression ) )
				{
					try
					{
						if ( !is_regular_file( ent.string() ) )
						{
							throw invalid_argument( "Not a regular file" ) ;
						}
					}
					catch (...)
					{
						processed.insert( ent.string() ) ;
						continue ;
					}
					entries.push_back( ent.string() ) ;
					processed.insert( ent.string() ) ;
					poss.push_back( p + 1 ) ;
				}
			}
		}
	}
}


void plrflst1::AddReferralEntry( string ent )
{
	//
	// Add the first entry from the reference list to referral list 'ent'.
	// Create referral list 'ent' if it does not exist.
	//

	int i ;

	string* eent ;
	string  reffile ;

	string msg = "LRFL011J" ;

	string ldate ;
	string ltime ;

	vector<string> list ;
	vector<string>::iterator it ;

	if ( !isvalidName( ent ) )
	{
		setmsg( "LRFL011I" ) ;
		return ;
	}

	vcopy( "ZDATESTD", ldate, MOVE ) ;
	vcopy( "ZTIMEL", ltime, MOVE ) ;

	control( "ERRORS", "RETURN" ) ;
	OpenTableRO() ;

	tbvclear( table ) ;
	zcurtb = "REFLIST" ;
	tbget( table ) ;
	if ( RC == 0 )
	{
		for ( i = 1 ; i <= 30 ; ++i )
		{
			vcopy( "FLAPET" + d2ds( i, 2 ), eent, LOCATE ) ;
			if ( eent && !eent->empty() )
			{
				reffile = *eent ;
				break ;
			}
		}
	}
	CloseTable() ;

	zcurtb = ent ;

	OpenTableUP() ;
	tbget( table ) ;
	if ( RC == 0 )
	{
		for ( i = 1 ; i <= 30 ; ++i )
		{
			vcopy( "FLAPET" + d2ds( i, 2 ), eent, LOCATE ) ;
			if ( eent && !eent->empty() )
			{
				list.push_back( *eent ) ;
			}
		}
		it = find( list.begin(), list.end(), reffile ) ;
		if ( it != list.end() )
		{
			list.erase( it ) ;
		}
	}
	else
	{
		tbvclear( table ) ;
		zcurtb   = ent ;
		flactime = ldate  ;
		msg = "LRFL011K" ;
	}

	vreplace( "FLAPET01", reffile ) ;

	for ( uint j = 2 ; j <= 30 ; ++j )
	{
		if ( j <= ( list.size() + 1 ) )
		{
			vreplace( "FLAPET" + d2ds( j, 2 ), list.at( j - 2 ) ) ;
		}
		else
		{
			vreplace( "FLAPET" + d2ds( j, 2 ), "" ) ;
		}
	}

	flautime = ldate + " " + ltime ;

	tbmod( table, "", "ORDER" ) ;

	CloseTable() ;

	control( "ERRORS", "CANCEL" ) ;
	setmsg( msg ) ;
}


void plrflst1::AddReflistEntry( string& ent )
{
	int i ;

	string* eent ;

	string ldate ;
	string ltime ;

	vector<string> list ;
	vector<string>::iterator it ;

	vcopy( "ZDATESTD", ldate, MOVE ) ;
	vcopy( "ZTIMEL", ltime, MOVE ) ;

	if ( get_profile_var( "ZRFURL" ) != "YES" )
	{
		return ;
	}

	if ( get_profile_var( "ZRFFEX" ) == "YES" )
	{
		try
		{
			if ( !exists( ent ) ) { return ; }
		}
		catch (...)
		{
			return ;
		}
	}

	control( "ERRORS", "RETURN" ) ;
	OpenTableUP() ;

	zcurtb = "REFLIST" ;
	tbget( table ) ;
	if ( RC == 8 )
	{
	       createReflistEntry() ;
	}

	for ( i = 1 ; i <= 30 ; ++i )
	{
		vcopy( "FLAPET" + d2ds( i, 2 ), eent, LOCATE ) ;
		if ( eent && !eent->empty() )
		{
			list.push_back( *eent ) ;
		}
	}

	if ( ent.back() == '/' ) { ent.pop_back() ; }

	it = find( list.begin(), list.end(), ent ) ;
	if ( it != list.end() )
	{
		list.erase( it ) ;
	}

	vreplace( "FLAPET01", ent ) ;

	for ( uint j = 2 ; j <= 30 ; ++j )
	{
		if ( j <= ( list.size() + 1 ) )
		{
			vreplace( "FLAPET" + d2ds( j, 2 ), list.at( j - 2 ) ) ;
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


void plrflst1::AddFilelistEntry( const string& p )
{
	//
	// Add file to a file list.  File list name is word one of parm p.
	// Create the list if it does not exist.
	//

	int i ;

	string  ent  ;
	string* eent ;

	string ldate ;
	string ltime ;

	vector<string>list ;
	vector<string>::iterator it ;

	vcopy( "ZDATESTD", ldate, MOVE ) ;
	vcopy( "ZTIMEL", ltime, MOVE ) ;

	control( "ERRORS", "RETURN" ) ;

	OpenTableUP() ;

	tbvclear( table ) ;
	zcurtb = upper( word( p, 1 ) ) ;
	ent    = subword( p, 2 ) ;

	tbget( table ) ;
	if ( RC > 0 )
	{
		fladescp = "New Entry" ;
		flactime = ldate ;
		tbadd( table, "", "ORDER" ) ;
		if ( RC > 0 ) { return ; }
	}

	for ( i = 1 ; i <= 30 ; ++i )
	{
		vcopy( "FLAPET" + d2ds( i, 2 ), eent, LOCATE ) ;
		if ( eent )
		{
			list.push_back( *eent ) ;
		}
	}

	if ( ent.back() == '/' ) { ent.pop_back() ; }

	it = find( list.begin(), list.end(), ent ) ;
	if ( it != list.end() )
	{
		list.erase( it ) ;
	}

	vreplace( "FLAPET01", ent ) ;

	for ( uint j = 2 ; j <= 30 ; ++j )
	{
		if ( j <= ( list.size() + 1 ) )
		{
			vreplace( "FLAPET" + d2ds( j, 2 ), list.at( j - 2 ) ) ;
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


void plrflst1::userSettings()
{
	RC = 0 ;

	while ( RC == 0 )
	{
		display( "PLRFLST5" ) ;
	}
}


void plrflst1::OpenTableRO()
{
	tbopen( table, NOWRITE, "ZUPROF" ) ;
	if ( RC == 8 )
	{
		createDefaultTable() ;
		tbopen( table, NOWRITE, "ZUPROF" ) ;
		if ( RC > 0 )
		{
			llog( "E", "Table "+ table +" cannot be opened NOWRITE.  RC="<< RC <<endl ) ;
			uabend() ;
		}
	}
}


void plrflst1::OpenTableUP()
{
	tbopen( table, WRITE, "ZUPROF" ) ;
	if ( RC == 8 )
	{
		createDefaultTable() ;
		tbopen( table, WRITE, "ZUPROF" ) ;
		if ( RC > 0 )
		{
			llog( "E", "Table "+ table +" cannot be opened WRITE.  RC="<< RC <<endl ) ;
			uabend() ;
		}
	}
}


void plrflst1::CloseTable()
{
	tbsort( table, "(ZCURTB,C,A)" ) ;

	tbclose( table, "", "ZUPROF" ) ;
}


void plrflst1::createDefaultTable()
{
	tbcreate( table,
		  "ZCURTB",
		  "("+ subword( tabflds, 2 ) +")",
		  WRITE,
		  NOREPLACE,
		  "ZUPROF" ) ;
	if ( RC > 0 )
	{
		llog( "E", "Table "+ table +" cannot be created.  RC="<< RC <<endl ) ;
		uabend() ;
	}

	createReflistEntry() ;

	CloseTable() ;
}


void plrflst1::createReflistEntry()
{
	string ldate ;
	string ltime ;

	tbvclear( table ) ;
	zcurtb = "REFLIST" ;
	vput( "ZCURTB", PROFILE ) ;

	vcopy( "ZDATESTD", ldate, MOVE ) ;
	vcopy( "ZTIMEL", ltime, MOVE ) ;

	fladescp = "Default Reference List" ;
	flactime = ldate ;
	flautime = ldate + " " + ltime ;

	tbadd( table ) ;
	if ( RC > 0 ) { return ; }

	tbsort( table, "(ZCURTB,C,A)" ) ;
}


string plrflst1::get_shared_var( const string& var )
{
	string r ;

	vget( var, SHARED ) ;
	vcopy( var, r ) ;

	return r ;
}


string plrflst1::get_profile_var( const string& var )
{
	string r ;

	vget( var, PROFILE ) ;
	vcopy( var, r ) ;

	return r ;
}
