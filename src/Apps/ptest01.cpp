/*  Compile with ::                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libptest01.so -o libptest01.so ptest01.cpp */

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


/****************************************************************************/
/*                                                                          */
/* opt0 - Test Panel Functions                                              */
/* opt1 - Perform operations on keyed table                                 */
/* opt2 - Perform operations on non-keyed table                             */
/* opt3 - Test Table Display for keyed table                                */
/* opt4 - Test Table Display for non-keyed table                            */
/* opt5 - Test Dynamic Areas 1                                              */
/* opt6 - Test Dynamic Areas 2                                              */
/* opt7 - Test popups                                                       */
/* opt8 - Run background REXX                                               */
/* opt9 - Test PANEXIT REXX and *REXX panel statements                      */
/* opt10- Test PANEXIT LOAD panel statements                                */
/*                                                                          */
/****************************************************************************/

#include <iostream>
#include <string>

#include "../lspfall.h"

#include "ptest01.h"

using namespace std ;

#define N_RED      0x03
#define N_GREEN    0x04
#define N_YELLOW   0x05
#define N_BLUE     0x06
#define N_MAGENTA  0x07
#define N_TURQ     0x08
#define N_WHITE    0x09

LSPF_APP_MAKER( ptest01 )


ptest01::ptest01()
{
	STANDARD_HEADER( "Testing program for lspf", "1.0.1" )

	vdefine( "ZTDTOP ZTDSELS", &ztdtop, &ztdsels ) ;
}

void ptest01::application()
{
//      verase( "ZPLIB ZMLIB ZTLIB ZHLIB" ) ;
//      return ;

//      ZTUSER = "/home/daniel/.lspf/" ;
//      libdef( "ZTUSER", "FILE" ) ;

	vdefine( "ZCMD", &zcmd ) ;

	if ( PARM == "WAIT" )
	{
		debug1( "Waiting forever...." << endl ) ;
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000000)) ;
	}
	else if ( PARM == "LOOP" )
	{
		debug1( "Looping forever...." << endl ) ;
		while ( true ) { PARM = "99" ; }
	}
	else if ( PARM == "ABEND" )
	{
		debug1( "Abending...." << endl ) ;
		uabend() ;
	}
	else if ( PARM == "ABORT" )
	{
		debug1( "Aborting...." << endl ) ;
		abort() ;
	}
	else if ( PARM == "TERMINATE" )
	{
		debug1( "Terminating..." << endl ) ;
		terminate() ;
	}
	else if ( PARM == "EXCEPTION" )
	{
		debug1( "Generating an exception...." << endl ) ;
	}
	else if ( PARM == "0" ) opt0() ;
	else if ( PARM == "1" ) opt1() ;
	else if ( PARM == "2" ) opt2() ;
	else if ( PARM == "3" ) opt3() ;
	else if ( PARM == "4" ) opt4() ;
	else if ( PARM == "5" ) opt5() ;
	else if ( PARM == "6" ) opt6() ;
	else if ( PARM == "7" ) opt7() ;
	else if ( PARM == "9" ) opt9() ;
	else if ( PARM == "10") opt10() ;
}


void ptest01::opt0()
{
	//
	// opt0 - Test Panel Functions
	//

	string MSG = "" ;

	string fvar1 ;
	string fvar2 ;
	string fvar3 ;
	string fvar4 ;
	string fvar5 ;
	string fvar6 ;

	string uvar1 ;
	string uvar2 ;
	string uvar3 ;
	string uvar4 ;
	string uvar5 ;
	string uvar6 ;

	const string vlist0 = "ZCMD" ;
	const string vlist1 = "FVAR1 FVAR2 FVAR3 FVAR4 FVAR5 FVAR6" ;
	const string vlist2 = "UVAR1 UVAR2 UVAR3 UVAR4 UVAR5 UVAR6" ;

	vdefine( vlist0, &zcmd ) ;
	vdefine( vlist1, &fvar1, &fvar2, &fvar3, &fvar4, &fvar5, &fvar6 ) ;
	vdefine( vlist2, &uvar1, &uvar2, &uvar3, &uvar4, &uvar5, &uvar6 ) ;

	vmask( "FVAR1", "FORMAT", "ITIME" ) ;
	vmask( "FVAR2", "FORMAT", "STDTIME" ) ;
	vmask( "FVAR3", "FORMAT", "IDATE" ) ;
	vmask( "FVAR4", "FORMAT", "STDDATE" ) ;
	vmask( "FVAR5", "FORMAT", "JDATE" ) ;
	vmask( "FVAR6", "FORMAT", "JSTD" ) ;

	vmask( "UVAR1", "USER", "(999)999-9999" ) ;
	vmask( "UVAR2", "USER", "S999" ) ;

	fvar1 = "1234" ;
	fvar2 = "123456" ;
	fvar3 = "200715" ;
	fvar4 = "20200715" ;
	fvar5 = "20197" ;
	fvar6 = "2020197" ;

	uvar1 = "0129876543" ;
	uvar2 = "-345" ;

	while ( true )
	{
		zcmd = "" ;
		display( "PTEST01A", MSG, "ZCMD" );
		if ( RC == 8 ) { break ; }
		MSG = "" ;
	}

	vdelete( vlist0, vlist1, vlist2 ) ;

}


void ptest01::opt1()
{

	//
	// opt1 - Perform operations on keyed table
	//

	string MSG, TABA1, TABA2, TABA3, TABA4, TABA5 ;
	string TBQ1, TBQ2  ;
	string TBQ7, TBQ8, TBQ9, TBQ10 ;

	int TBQ3, TBQ4, TBQ5, TBQ6 ;

	vdefine( "TABA1 TABA2 TABA3 TABA4 TABA5", &TABA1, &TABA2, &TABA3, &TABA4, &TABA5 ) ;
	vdefine( "TBQ1 TBQ2  ",  &TBQ1, &TBQ2  ) ;
	vdefine( "TBQ7 TBQ8 TBQ9 TBQ10", &TBQ7, &TBQ8, &TBQ9, &TBQ10 ) ;
	vdefine( "TBQ3 TBQ4 TBQ5 TBQ6", &TBQ3, &TBQ4, &TBQ5, &TBQ6 ) ;

	MSG  = "" ;

//      tbopen( "TABK", WRITE, "/home/daniel/.lspf/" ) ;
	tbopen( "TABK", WRITE ) ;
	llog( "A", "TBOPEN TABK WRITE no file name RC=" << RC << endl ) ;

	tbsave( "TABK" ) ;
	llog( "A", "TBSAVE TABK no file name RC=" << RC << endl ) ;

	tbend( "TABK" ) ;
	llog( "A", "TBEND TABK RC=" << RC << endl ) ;

	tberase( "TABK" ) ;
	llog( "A", "TBERASE TABK RC=" << RC << endl ) ;

	tbcreate( "TABK", "TABA1", "(TABA2,TABA3,TABA4,TABA5)", WRITE ) ;
	llog( "A", "TBCREATE TABK with WRITE option RC=" << RC << endl ) ;

	tbsort( "TABK", "(TABA1,C,D)" ) ;
	llog( "A", "TBSORT TABK RC=" << RC << endl ) ;

	tbvclear( "TABK" ) ;
	llog( "A", "TBVCLEAR TABK RC=" << RC << endl ) ;

	TABA1 = "KEY2"    ; TABA2 = "AALUE11" ; TABA3 = "VALUE222" ; TABA4 = "VALUE3333" ; TABA5 = "VALUE44444" ;
	tbadd( "TABK", "", "ORDER" ) ;
	llog( "A", "TBADD TABK RC=" << RC << endl ) ;
	TABA1 = "KEY1"    ; TABA2 = "CALUE11" ; TABA3 = "VALUE222" ; TABA4 = "VALUE3333" ; TABA5 = "VALUE44444" ;
	tbadd( "TABK", "", "ORDER"  ) ;
	llog( "A", "TBADD TABK RC=" << RC << endl ) ;
	TABA1 = "KEY3"    ; TABA2 = "BALUE11" ; TABA3 = "VALUE222" ; TABA4 = "VALUE3333" ; TABA5 = "VALUE44444" ;
	tbadd( "TABK", "", "ORDER"  ) ;
	llog( "A", "TBADD TABK RC=" << RC << endl ) ;

	int i ;
	for ( i = 4 ; i > 0 ; i-- )
	{
		TABA1 = "TABKEY" + d2ds(i) ; TABA2 = "VALUE B1 " ; TABA3 = "VALUE B2  " ; TABA4 = "VALUE B3   " ; TABA5 = "VALUE B4    " ;
		tbadd( "TABK"  ) ;
		llog( "A", "TBADD TABK with key >>" << TABA1 << "<<  RC=" << RC << endl ) ;
	}

	tbbottom( "TABK" ) ;
	llog( "A", "TBADD TABK RC=" << RC << endl ) ;

	TABA1 = "KEY4"    ; TABA2 = "VALUE11" ; TABA3 = "VALUE222" ; TABA4 = "VALUE3333" ; TABA5 = "VALUE44444" ;
	tbadd( "TABK", "", "ORDER"  ) ;
	llog( "A", "TBADD TABK RC=" << RC << endl ) ;

	tbtop( "TABK" ) ;
	while ( true )
	{
		tbskip( "TABK", 1 ) ;
		if ( RC > 0 ) { break ; }
		llog( "A", "TBSKIP TABK VAR VALUES: TABA1 " << TABA1 << " TABA2 " << TABA2  << " TABA3 " << TABA3  << " TABA4 " << TABA4  << " TABA5 " << TABA5 << endl ) ;
	}


	TABA1 = "TABKEY2" ;
	tbdelete( "TABK" ) ;
	llog( "A", "TBDELETE TABK for TABKEY2 RC=" << RC << endl ) ;

	tbtop( "TABK" ) ;
	while ( true )
	{
		tbskip( "TABK", 1 ) ;
		if ( RC > 0 ) { break ; }
		llog( "A", "TBSKIP TABK VAR VALUES: TABA1 " << TABA1 << " TABA2 " << TABA2  << " TABA3 " << TABA3  << " TABA4 " << TABA4  << " TABA5 " << TABA5 << endl ) ;
	}

	tbsort( "TABK", "(TABA2,C,D)" ) ;

	TABA1 = "TABKEY999" ;
	tbdelete( "TABK" ) ;
	llog( "A", "TBDELETE TABK for TABKEY999 RC=" << RC << endl ) ;


//      tbsave( "TABK", "" , "/home/daniel/.lspf/" ) ;
	tbsave( "TABK" ) ;
	llog( "A", "TBSAVE TABK no file name RC=" << RC << endl ) ;

	tbend( "TABK" ) ;
	llog( "A", "TBEND TABK RC=" << RC << endl ) ;

	tbopen( "TABK", NOWRITE ) ;
	llog( "A", "TBOPEN TABK NOWRITE RC=" << RC << endl ) ;

	tbsave( "TABK", "TEST111" , "/home/daniel/.lspf/" ) ;
	llog( "A", "TBSAVE TABK with file name ( RC=12? ) RC=" << RC << endl ) ;

	tbend( "TABK" ) ;
	llog( "A", "TBEND TABK RC=" << RC << endl ) ;

	tbopen( "TABK", WRITE ) ;
	llog( "A", "TBOPEN TABK NOWRITE RC=" << RC << endl ) ;

	TABA1 = "KEY2" ;
	tbdelete( "TABK" );
	llog( "A", "TBDELETE TABK KEY IS KEY2 RC=" << RC << endl ) ;

	tbquery( "TABK", "TBQ1","TBQ2", "TBQ3", "TBQ4", "TBQ5", "TBQ6", "TBQ7", "TBQ8", "TBQ9", "TBQ10" ) ;
	llog( "A", "CRP........" << TBQ6  << endl ) ;

	TABA1 = "KEY1"    ; TABA2 = "VALUE11" ; TABA3 = "VALUE222" ; TABA4 = "VALUE3333" ; TABA5 = "VALUE44444" ;
	tbadd( "TABK" ) ;
	llog( "A", "TBADD TABK KEY IS KEY1 (exists) RC=" << RC << endl ) ;

	tbquery( "TABK", "TBQ1","TBQ2", "TBQ3", "TBQ4", "TBQ5", "TBQ6", "TBQ7", "TBQ8", "TBQ9", "TBQ10" ) ;
	llog( "A", "CRP........" << TBQ6  << endl ) ;

	TABA1 = "KEY2"    ; TABA2 = "VALUE11" ; TABA3 = "VALUE222" ; TABA4 = "VALUE3333" ; TABA5 = "VALUE44444" ;
	tbadd( "TABK" ) ;
	llog( "A", "TBADD TABK KEY IS KEY2 (should be okay) RC=" << RC << endl ) ;

	tbquery( "TABK", "TBQ1","TBQ2", "TBQ3", "TBQ4", "TBQ5", "TBQ6", "TBQ7", "TBQ8", "TBQ9", "TBQ10" ) ;
	llog( "A", "CRP........" << TBQ6  << endl ) ;

	tbtop( "TABK" ) ;
	while ( true )
	{
		tbskip( "TABK", 1 ) ;
		if ( RC > 0 ) { break ; }
		llog( "A", "TBSKIP TABK VAR VALUES: TABA1 " << TABA1 << " TABA2 " << TABA2  << " TABA3 " << TABA3  << " TABA4 " << TABA4  << " TABA5 " << TABA5 << endl ) ;
	}

	TABA1 = "KEY2"    ; TABA2 = "VALUENEW" ; TABA3 = "VALUENEW" ; TABA4 = "VALUENEW" ; TABA5 = "VALUE4NEW" ;
	tbput( "TABK" ) ;
	llog( "A", "TBPUT TABK KEY IS KEY2 (should be okay) RC=" << RC << endl ) ;

	tbquery( "TABK", "TBQ1","TBQ2", "TBQ3", "TBQ4", "TBQ5", "TBQ6", "TBQ7", "TBQ8", "TBQ9", "TBQ10" ) ;
	llog( "A", "CRP........" << TBQ6  << endl ) ;

	TABA1 = "KEY2"    ;
	tbget( "TABK" ) ;
	llog( "A", "TBGET TABK KEY IS KEY2 (should be okay) RC=" << RC << endl ) ;

	TABA1 = "KEY2"    ; TABA2 = "VALUENEW" ; TABA3 = "VALUENEW" ; TABA4 = "VALUENEW" ; TABA5 = "VALUE4NEW" ;
	tbput( "TABK" ) ;
	llog( "A", "TBPUT TABK KEY IS KEY2 (should be okay) RC=" << RC << endl ) ;

	tbsort( "TABK", "(TABA2,C,D)" ) ;
	tbsarg( "TABK", "", "NEXT", "(TABA5,EQ)" ) ;


	tbquery( "TABK", "TBQ1","TBQ2", "TBQ3", "TBQ4", "TBQ5", "TBQ6", "TBQ7", "TBQ8", "TBQ9", "TBQ10" ) ;

	llog( "A", "TBQUERY RC=" << RC << endl ) ;
	llog( "A", "-" << RC << endl ) ;
	llog( "A", "Key name......." << TBQ1  << endl ) ;
	llog( "A", "Var name......." << TBQ2  << endl ) ;
	llog( "A", "Row Num........" << TBQ3  << endl ) ;
	llog( "A", "Key Num........" << TBQ4  << endl ) ;
	llog( "A", "Name num......." << TBQ5  << endl ) ;
	llog( "A", "CRP............" << TBQ6  << endl ) ;
	llog( "A", "Sort IR........" << TBQ7  << endl ) ;
	llog( "A", "Name List......" << TBQ8  << endl ) ;
	llog( "A", "Cond List......" << TBQ9  << endl ) ;
	llog( "A", "Direction......" << TBQ10 << endl ) ;

	tbend( "TABK" ) ;
	llog( "A", "TBEND TABK RC=" << RC << endl ) ;

	vdelete( "TABA1 TABA2 TABA3 TABA4 TABA5" ) ;
}


void ptest01::opt2()
{
	//
	// opt2 - Perform operations on non-keyed table
	//

	int i ;
	int r ;

	string EXTV1, EXTV2, EXTV3, EXTV4, EXTLST ;
	vdefine( "EXTV1 EXTV2 EXTV3 EXTV4 EXTLST", &EXTV1, &EXTV2, &EXTV3, &EXTV4, &EXTLST ) ;

	string TABB1, TABB2, TABB3, TABB4, TABB5 ;
	vdefine( "TABB1 TABB2 TABB3 TABB4 TABB5", &TABB1, &TABB2, &TABB3, &TABB4, &TABB5 ) ;

	tbcreate( "TABN", "", "(TABB1,TABB2,TABB3,TABB4,TABB5)", WRITE ) ;
	llog( "A", "TBCREATE TABN RC=" << RC << endl ) ;

	tbvclear( "TABN" ) ;
	llog( "A", "TBVCLEAR TABN RC=" << RC << endl ) ;


	r = 0 ;
	for ( i = 10 ; i > 0 ; i-- )
	{
		TABB1 = "NOTAKEY" ; TABB2 = "VALUE "+d2ds(i) ; TABB3 = "VALUE "+d2ds(i)  ; TABB4 = "VALUE  "+d2ds(i)  ; TABB5 = "VALUE     "+d2ds(i)  ;
		tbadd( "TABN", "", "", r ) ;
		r = 0 ;
		llog( "A", "TBADD TABN RC=" << RC << " Record number being loaded is " << i << endl ) ;
	}

//      tbend("TABN" ) ;
//      return ;
	TABB1 = "NOTAKEY" ; TABB2 = "VALUE WITH EXT VARS" ; TABB3 = "VALUE WITH EXT VARS"  ; TABB4 = "VALUE4" ; TABB5 = "VALUE4"  ;

	EXTV1 = "EXTENSION VAR 1 " ;
	EXTV2 = "EXTENSION VAR 2 " ;
	EXTV3 = "EXTENSION VAR 3 " ;
	EXTV4 = "EXTENSION VAR 4 " ;

	tbadd( "TABN", "(EXTV1,EXTV2,EXTV3,EXTV4)" ) ;
	llog( "A", "TBADD TABN RC=" << RC << " Record number being loaded is custom" << endl ) ;

	TABB1 = "NOTAKEY" ; TABB2 = "VALUE3" ; TABB3 = "VALUE3"  ; TABB4 = "VALUE3" ; TABB5 = "VALUE3"  ;
	tbadd( "TABN" ) ;
	llog( "A", "TBADD TABN RC=" << RC << " Record number being loaded is custom" << endl ) ;

	tbtop( "TABN" ) ;
	llog( "A", "TBTOP TABN RC=" << RC << endl ) ;

	tbvclear( "TABN" ) ;
	TABB1 = "NOTAKEY" ;
	EXTV4 = "EXTENSION VA*" ;
	tbsarg( "TABN", "EXTV4", "", "" ) ;
	llog( "A", "TBSARG TABN  RC=" << RC << endl ) ;

	EXTV1 = "" ;
	EXTV2 = "" ;
	EXTV3 = "" ;
	EXTV4 = "" ;

	TABB1 = "NOTAKEY" ;
	EXTV4 = "EXTENSION VA*" ;
	tbscan( "TABN" ) ;
	llog( "A", "TBSCAN (no TBSARG) TABN  RC=" << RC << endl ) ;
	llog( "A", "TBSCAN TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	llog( "A", "EXTLST >>" << EXTLST << "<<"  << endl ) ;
	llog( "A", "EXTV1 >>" << EXTV1 << "<<"  << endl ) ;
	llog( "A", "EXTV2 >>" << EXTV2 << "<<"  << endl ) ;
	llog( "A", "EXTV3 >>" << EXTV3 << "<<"  << endl ) ;
	llog( "A", "EXTV4 >>" << EXTV4 << "<<"  << endl ) ;


	tbvclear( "TABN" ) ;
	TABB1 = "NOTAKEY" ;
	EXTV4 = "EXTENSION VAx*" ;
	tbsarg( "TABN", "EXTV4", "", "" ) ;
	llog( "A", "TBSARG TABN  RC=" << RC << endl ) ;
	tbscan( "TABN" ) ;
	llog( "A", "TBSCAN TABN  RC=" << RC << endl ) ;
	llog( "A", "TBSCAN TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	llog( "A", "EXTLST >>" << EXTLST << "<<"  << endl ) ;
	llog( "A", "EXTV1 >>" << EXTV1 << "<<"  << endl ) ;
	llog( "A", "EXTV2 >>" << EXTV2 << "<<"  << endl ) ;
	llog( "A", "EXTV3 >>" << EXTV3 << "<<"  << endl ) ;
	llog( "A", "EXTV4 >>" << EXTV4 << "<<"  << endl ) ;

	tbscan( "TABN" ) ;
	llog( "A", "TBSCAN TABN  RC=" << RC << endl ) ;

	tbtop( "TABN" ) ;
	llog( "A", "TBTOP TABN RC=" << RC << endl ) ;


	tbskip( "TABN", 1 ) ;
	llog( "A", "TBSKIP TABN 1 RECORD RC=" << RC << endl ) ;

	EXTV1 = "" ;
	EXTV2 = "" ;
	EXTV3 = "" ;
	EXTV4 = "" ;

	tbget( "TABN" ) ;
	llog( "A", "TBGET TABN 1 RECORD RC=" << RC << endl ) ;
	llog( "A", "EXTV1 >>" << EXTV1 << "<<"  << endl ) ;

	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		if ( RC > 8 ) { llog( "A", "SEVERE ERROR **** TBSKIP" << endl ) ; }
		if ( RC > 0 ) { break ; }
		llog( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
		tbget( "TABN", "EXTLST" ) ;
		llog( "A", "TBGET TABN 1 RECORD RC=" << RC << endl ) ;
		llog( "A", "EXTLST >>" << EXTLST << "<<"  << endl ) ;
		llog( "A", "EXTV1 >>" << EXTV1 << "<<"  << endl ) ;
		llog( "A", "EXTV2 >>" << EXTV2 << "<<"  << endl ) ;
		llog( "A", "EXTV3 >>" << EXTV3 << "<<"  << endl ) ;
		llog( "A", "EXTV4 >>" << EXTV4 << "<<"  << endl ) ;
		EXTV1 = "" ;
		EXTV2 = "" ;
		EXTV3 = "" ;
		EXTV4 = "" ;
	}

	tbsort( "TABN", "TABB3" ) ;
	llog( "A", "TBSORT TABN RC=" << RC << endl ) ;

	tbsave( "TABN", "" , "/home/daniel/.lspf/" ) ;
	llog( "A", "TBASAVE TABN to file name RC=" << RC << endl ) ;

	tbend( "TABN" ) ;
	llog( "A", "TBEND TABN RC=" << RC << endl ) ;

	tbopen( "TABN", NOWRITE, "/home/daniel/.lspf/" ) ;
	llog( "A", "TBOPEN TABN with NOWRITE RC=" << RC << endl ) ;

	tbtop( "TABN" ) ;
	llog( "A", "TBTOP TABN RC=" << RC << endl ) ;

	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		llog( "A", "TBSKIP TABN 1 RECORD RC=" << RC << endl ) ;
		if ( RC > 0 ) { break ; }
		llog( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	}

	tbtop( "TABN" ) ;
	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		llog( "A", "TBSKIP TABN 1 RECORD RC=" << RC << endl ) ;
		if ( RC > 0 ) { break ; }
		llog( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	}

	tbend( "TABN" ) ;
	llog( "A", "TBEND TABN RC=" << RC << endl ) ;

	tbopen( "TABN", WRITE, "/home/daniel/.lspf/" ) ;
	llog( "A", "TBOPEN TABN with WRITE RC=" << RC << endl ) ;

	tbsort( "TABN", "TABB1" ) ;
	llog( "A", "TBSORT TABN RC=" << RC << endl ) ;
	tbtop( "TABN" ) ;
	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		if ( RC > 0 ) { break ; }
		llog( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	}
	tbsort( "TABN", "TABB2" ) ;
	llog( "A", "TBSORT TABN RC=" << RC << endl ) ;

	tbtop( "TABN" ) ;
	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		if ( RC > 0 ) { break ; }
		llog( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	}

	tbsort( "TABN", "TABB3" ) ;
	llog( "A", "TBSORT TABN RC=" << RC << endl ) ;
	tbtop( "TABN" ) ;
	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		if ( RC > 0 ) { break ; }
		llog( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	}

	tbsort( "TABN", "TABB4" ) ;
	llog( "A", "TBSORT TABN RC=" << RC << endl ) ;
	tbtop( "TABN" ) ;
	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		if ( RC > 0 ) { break ; }
		llog( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	}

	tbsort( "TABN", "TABB5" ) ;
	llog( "A", "TBSORT TABN RC=" << RC << endl ) ;
	tbtop( "TABN" ) ;
	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		if ( RC > 0 ) { break ; }
		llog( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	}

 //     tbsort( "TABN", "TABB6" ) ;
 //     llog( "A", "TBSORT TABN RC=" << RC << endl ) ;

	tbtop( "TABN" ) ;
	tbskip( "TABN", 4 ) ;
	llog( "A", "TBTOP/TBSKIP TABK for CRP = 4 :: RC=" << RC << endl ) ;

	tbdelete( "TABN" ) ;
	llog( "A", "TBDELETE TABK for CRP = 4 :: RC=" << RC << endl ) ;

	tbtop( "TABN" ) ;
	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		if ( RC > 0 ) { break ; }
		llog( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	}

	tbskip( "TABN", 0, "", "", "2" ) ;
	llog( "A", "TBSKIP TABN URID=2 RECORD RC=" << RC << endl ) ;

	tbget( "TABN" ) ;
	llog( "A", "TBGET RC=" << RC << endl ) ;
	llog( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;

	tbskip( "TABN", 0, "", "", "1" ) ;
	llog( "A", "TBSKIP TABN URID=1 RECORD RC=" << RC << endl ) ;

	tbget( "TABN" ) ;
	llog( "A", "TBGET RC=" << RC << endl ) ;
	llog( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;

	tbskip( "TABN", 0, "", "", "999999" ) ;
	llog( "A", "TBSKIP TABN URID=999999 RECORD RC=" << RC << endl ) ;

	tbget( "TABN" ) ;
	llog( "A", "TBGET RC=" << RC << endl ) ;
	llog( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;

	tbtop( "TABN" ) ;
	tbdelete( "TABN" ) ;
	llog( "A", "TBDELETE TABK for TBTOP RC=" << RC << endl ) ;


	tbend( "TABN" ) ;
	llog( "A", "TBEND TABN RC=" << RC << endl ) ;
}


void ptest01::opt3()
{
	//
	// opt3 - Test Table Display for keyed table
	//

	int i ;
	int TRC     ;
	int YTDSELS ;
	int CRP     ;
	int CSRROW  ;

	bool e_loop ;

	string YKEY1, YFLD1, YFLD2, YFLD3, YFLD4, YROWID ;

	string w1 ;
	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;
	string w6 ;

	string TBQ1, TBQ2  ;
	int    TBQ3, TBQ4, TBQ5, TBQ6 ;
	string TBQ7, TBQ8, TBQ9, TBQ10 ;

	vdefine( "TBQ1 TBQ2",  &TBQ1, &TBQ2 ) ;
	vdefine( "TBQ3 TBQ4 TBQ5 TBQ6",  &TBQ3, &TBQ4, &TBQ5, &TBQ6  ) ;
	vdefine( "TBQ7 TBQ8 TBQ9 TBQ10", &TBQ7, &TBQ8, &TBQ9, &TBQ10 ) ;

	string MSG, SEL, KEY1, FLD1, FLD2, FLD3, FLD4, ROWID ;
	string TOPR1, TRC1, TOPR2, TRC2, TOPR3, TRC3, TOPR4, TRC4, TOPR5, TRC5 ;

	vdefine( "SEL KEY1 FLD1 FLD2 FLD3 FLD4", &SEL, &KEY1, &FLD1, &FLD2, &FLD3, &FLD4 ) ;
	vdefine( "YKEY1 YFLD1 YFLD2 YFLD3 YFLD4", &YKEY1, &YFLD1, &YFLD2, &YFLD3, &YFLD4 ) ;
	vdefine( "TOPR1 TRC1 TOPR2 TRC2 TOPR3 TRC3 TOPR4 TRC4", &TOPR1, &TRC1, &TOPR2, &TRC2, &TOPR3, &TRC3, &TOPR4, &TRC4 ) ;
	vdefine( "TOPR5 TRC5", &TOPR5, &TRC5 ) ;

	vdefine( "ROWID YROWID", &ROWID, &YROWID ) ;
	vdefine( "TRC YTDSELS CRP", &TRC, &YTDSELS, &CRP ) ;

	MSG     = "" ;
	YTDSELS = 0  ;
	TRC     = 0  ;
	vector< string >ops ;

	tbcreate( "TABKD", "KEY1", "(SEL,FLD1,FLD2,FLD3,FLD4)", NOWRITE ) ;
	llog( "A", "TBCREATE TABN RC=" << RC << endl ) ;
	ops.push_back( "TBCREATE " + d2ds(RC) ) ;
	if ( RC > TRC ) { TRC = RC ; }

	SEL = "" ; KEY1 = "AAAA" ; FLD1 = "LN1"   ; FLD2 = "FLD2 LN1 " ; FLD3 = "FLD3 LN1" ; FLD4 = "FLD4 LN1" ;
	tbadd( "TABKD" ) ;
	ops.push_back( "TBADD " + d2ds(RC) ) ;
	if ( RC > TRC ) { TRC = RC ; }

	SEL = "" ; KEY1 = "BBBB" ; FLD1 = "LN2"   ; FLD2 = "FLD2 LN2 " ; FLD3 = "FLD3 LN2" ; FLD4 = "FLD4 LN2" ;
	tbadd( "TABKD" ) ;
	ops.push_back( "TBADD " + d2ds(RC) ) ;
	if ( RC > TRC ) { TRC = RC ; }

	SEL = "" ; KEY1 = "CCCC" ; FLD1 = "LN3"   ; FLD2 = "FLD2 LN3 " ; FLD3 = "FLD3 LN3" ; FLD4 = "FLD4 LN3" ;
	tbadd( "TABKD" ) ;
	ops.push_back( "TBADD " + d2ds(RC) ) ;
	if ( RC > TRC ) { TRC = RC ; }

	tbtop( "TABKD" ) ;
	ops.push_back( "TBTOP " + d2ds(RC) ) ;
	if ( RC > TRC ) { TRC = RC ; }

	tbvclear( "TABKD" ) ;
	llog( "A", "TBVCLEAR TABN RC=" << RC << endl ) ;
	CRP    = 0  ;
	CSRROW = 0  ;
	ROWID  = "" ;
	ztdtop = 1  ;
	e_loop = false ;

	while ( true )
	{
		TOPR1 = "" ; TRC1 = "" ; TOPR2 = "" ; TRC2 = "" ; TOPR3 = "" ; TRC3 = "" ; TOPR4 = "" ; TRC4 = "" ; TOPR5 = "" ; TRC5 = "" ;
		zcmd  = "" ;
		i = ops.size() - 1 ;
		if ( i >= 0 ) { TOPR1 = word( ops[i], 1 ) ; TRC1 = word( ops[i], 2 ) ; i-- ; }
		if ( i >= 0 ) { TOPR2 = word( ops[i], 1 ) ; TRC2 = word( ops[i], 2 ) ; i-- ; }
		if ( i >= 0 ) { TOPR3 = word( ops[i], 1 ) ; TRC3 = word( ops[i], 2 ) ; i-- ; }
		if ( i >= 0 ) { TOPR4 = word( ops[i], 1 ) ; TRC4 = word( ops[i], 2 ) ; i-- ; }
		if ( i >= 0 ) { TOPR5 = word( ops[i], 1 ) ; TRC5 = word( ops[i], 2 ) ; i-- ; }
		tbtop( "TABKD" ) ;
		tbskip( "TABKD", ztdtop ) ;
		tbquery( "TABKD", "TBQ1","TBQ2", "TBQ3", "TBQ4", "TBQ5", "TBQ6", "TBQ7", "TBQ8", "TBQ9", "TBQ10" ) ;
		tbdispl( "TABKD", "PTEST01D", MSG, "ZCMD", CSRROW, 0, "NO", "CRP", "ROWID" ) ;
		if ( RC == 8 ) { break ; }
		TRC   = 0  ;
		if ( RC > TRC ) { TRC = RC ; }
		YTDSELS = ztdsels ;
		YFLD1   = FLD1    ;
		YFLD2   = FLD2    ;
		YFLD3   = FLD3    ;
		YFLD4   = FLD4    ;
		YROWID  = ""      ;
		MSG  = "" ;
		w1 = upper( word( zcmd, 1 ) ) ;
		w2 = word( zcmd, 2 ) ;
		w3 = word( zcmd, 3 ) ;
		w4 = word( zcmd, 4 ) ;
		w5 = word( zcmd, 5 ) ;
		if ( w1 == "SORT" )
		{
			control( "ERRORS", "RETURN" ) ;
			tbsort( "TABKD", "("+upper( w2 )+")" ) ;
			ops.push_back( "TBSORT " + d2ds(RC) ) ;
			if ( RC > TRC ) { TRC = RC ; }
			control( "ERRORS", "CANCEL" ) ;
		}
		else if ( w1 == "ADD" )
		{
			SEL = "" ; KEY1 = w2 ; FLD1 = w3 ; FLD2 = w4 ; FLD3 = w5 ; FLD4 = w6 ;
			tbadd( "TABKD" ) ;
			ops.push_back( "TBADD " + d2ds(RC) ) ;
			if ( RC > TRC ) { TRC = RC ; }
		}
		else if ( w1 == "ADDO" )
		{
			SEL = "" ; KEY1 = w2 ; FLD1 = w3 ; FLD2 = w4 ; FLD3 = w5 ; FLD4 = w6 ;
			tbadd( "TABKD", "", "ORDER" ) ;
			ops.push_back( "TBADD " + d2ds(RC) ) ;
			if ( RC > TRC ) { TRC = RC ; }
		}
		i = ztdtop ;
		while ( ztdsels > 0 )
		{
			if ( SEL == "D" )
			{
				tbdelete( "TABKD" ) ;
				ops.push_back( "TBDELETE " + d2ds(RC) ) ;
				if ( RC > TRC ) { TRC = RC ; }
			}
			else if ( SEL == "R" )
			{
				SEL = "" ;
				tbadd( "TABKD" ) ;
				ops.push_back( "TBADD " + d2ds(RC) ) ;
				if ( RC > TRC ) { TRC = RC ; }
			}
			else if ( SEL == "RO" )
			{
				SEL = "" ;
				tbadd( "TABKD", "", "ORDER" ) ;
				ops.push_back( "TBADD " + d2ds(RC) ) ;
				if ( RC > TRC ) { TRC = RC ; }
			}
			else if ( SEL == "G" )
			{
				SEL = "" ;
				tbget( "TABKD", "", "YROWID" ) ;
				ops.push_back( "TBGET " + d2ds(RC) ) ;
				if ( RC > TRC ) { TRC = RC ; }
				YKEY1   = KEY1    ;
				YFLD1   = FLD1    ;
				YFLD2   = FLD2    ;
				YFLD3   = FLD3    ;
				YFLD4   = FLD4    ;
			}
			if ( ztdsels > 1 )
			{
				tbdispl( "TABKD" ) ;
				if ( RC > TRC ) { TRC = RC ; }
				if ( RC > 4 ) { e_loop = true ; break ; }
			}
			else { ztdsels = 0 ; }
		}
		if ( e_loop ) { break ; }
	}
	tbend( "TABKD" ) ;

}


void ptest01::opt4()
{
	//
	// opt4 - Test Table Display and other table functions for non-keyed table
	//

	int i ;
	int TRC     ;
	int YTDSELS ;
	int CRP     ;
	int CSRROW  ;

	bool e_loop ;

	string YFLD1, YFLD2, YFLD3, YFLD4, YROWID ;

	string w1 ;
	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;

	string TBQ1, TBQ2  ;
	int    TBQ3, TBQ4, TBQ5, TBQ6 ;
	string TBQ7, TBQ8, TBQ9, TBQ10 ;
	vdefine( "TBQ1 TBQ2",  &TBQ1, &TBQ2 ) ;
	vdefine( "TBQ3 TBQ4 TBQ5 TBQ6",  &TBQ3, &TBQ4, &TBQ5, &TBQ6  ) ;
	vdefine( "TBQ7 TBQ8 TBQ9 TBQ10", &TBQ7, &TBQ8, &TBQ9, &TBQ10 ) ;

	string MSG, SEL, FLD1, FLD2, FLD3, FLD4, ROWID ;
	string TOPR1, TRC1, TOPR2, TRC2, TOPR3, TRC3, TOPR4, TRC4, TOPR5, TRC5 ;

	vdefine( "SEL FLD1 FLD2 FLD3 FLD4", &SEL, &FLD1, &FLD2, &FLD3, &FLD4 ) ;
	vdefine( "YFLD1 YFLD2 YFLD3 YFLD4", &YFLD1, &YFLD2, &YFLD3, &YFLD4 ) ;
	vdefine( "TOPR1 TRC1 TOPR2 TRC2 TOPR3 TRC3 TOPR4 TRC4", &TOPR1, &TRC1, &TOPR2, &TRC2, &TOPR3, &TRC3, &TOPR4, &TRC4 ) ;
	vdefine( "TOPR5 TRC5", &TOPR5, &TRC5 ) ;

	vdefine( "ROWID YROWID", &ROWID, &YROWID ) ;
	vdefine( "TRC YTDSELS CRP", &TRC, &YTDSELS, &CRP ) ;

	MSG     = "" ;
	YTDSELS = 0  ;
	TRC     = 0  ;
	vector< string >ops ;

	tbcreate( "TABND", "", "(SEL,FLD1,FLD2,FLD3,FLD4)", NOWRITE ) ;
	llog( "A", "TBCREATE TABN RC=" << RC << endl ) ;
	ops.push_back( "TBCREATE " + d2ds(RC) ) ;
	if ( RC > TRC ) { TRC = RC ; }

	SEL = "" ; FLD1 = "LINE1"   ; FLD2 = "FLD2 LINE1 " ; FLD3 = "FLD3 LINE1" ; FLD4 = "FLD4 LINE1" ;
	tbadd( "TABND" ) ;
	ops.push_back( "TBADD " + d2ds(RC) ) ;
	if ( RC > TRC ) { TRC = RC ; }

	SEL = "" ; FLD1 = "LINE2"   ; FLD2 = "FLD2 LINE2 " ; FLD3 = "FLD3 LINE2" ; FLD4 = "FLD4 LINE2" ;
	tbadd( "TABND" ) ;
	ops.push_back( "TBADD " + d2ds(RC) ) ;
	if ( RC > TRC ) { TRC = RC ; }

	SEL = "" ; FLD1 = "LINE3"   ; FLD2 = "FLD2 LINE3 " ; FLD3 = "FLD3 LINE3" ; FLD4 = "FLD4 LINE3" ;
	tbadd( "TABND" ) ;
	ops.push_back( "TBADD " + d2ds(RC) ) ;
	if ( RC > TRC ) { TRC = RC ; }

	tbtop( "TABND" ) ;
	ops.push_back( "TBTOP " + d2ds(RC) ) ;
	if ( RC > TRC ) { TRC = RC ; }

	tbvclear( "TABND" ) ;
	llog( "A", "TBVCLEAR TABN RC=" << RC << endl ) ;

	CRP    = 0  ;
	CSRROW = 0  ;
	ROWID  = "" ;
	ztdtop = 1  ;
	e_loop = false ;
	while ( true )
	{
		TOPR1 = "" ; TRC1 = "" ; TOPR2 = "" ; TRC2 = "" ; TOPR3 = "" ; TRC3 = "" ; TOPR4 = "" ; TRC4 = "" ; TOPR5 = "" ; TRC5 = "" ;
		zcmd  = "" ;
		i = ops.size() - 1 ;
		if ( i >= 0 ) { TOPR1 = word( ops[i], 1 ) ; TRC1 = word( ops[i], 2 ) ; i-- ; }
		if ( i >= 0 ) { TOPR2 = word( ops[i], 1 ) ; TRC2 = word( ops[i], 2 ) ; i-- ; }
		if ( i >= 0 ) { TOPR3 = word( ops[i], 1 ) ; TRC3 = word( ops[i], 2 ) ; i-- ; }
		if ( i >= 0 ) { TOPR4 = word( ops[i], 1 ) ; TRC4 = word( ops[i], 2 ) ; i-- ; }
		if ( i >= 0 ) { TOPR5 = word( ops[i], 1 ) ; TRC5 = word( ops[i], 2 ) ; i-- ; }
		tbtop( "TABND" ) ;
		tbskip( "TABND", ztdtop ) ;
		tbquery( "TABND", "TBQ1","TBQ2", "TBQ3", "TBQ4", "TBQ5", "TBQ6", "TBQ7", "TBQ8", "TBQ9", "TBQ10" ) ;
		tbdispl( "TABND", "PTEST01B", MSG, "ZCMD", CSRROW, 0, "NO", "CRP", "ROWID" ) ;
		if ( RC == 8 ) { break ; }
		TRC   = 0  ;
		if ( RC > TRC ) { TRC = RC ; }
		YTDSELS = ztdsels ;
		YFLD1   = FLD1    ;
		YFLD2   = FLD2    ;
		YFLD3   = FLD3    ;
		YFLD4   = FLD4    ;
		YROWID  = ""      ;
		MSG  = "" ;
		w1 = upper( word( zcmd, 1 ) ) ;
		w2 = word( zcmd, 2 ) ;
		w3 = word( zcmd, 3 ) ;
		w4 = word( zcmd, 4 ) ;
		w5 = word( zcmd, 5 ) ;
		if ( w1 == "SORT" )
		{
			control( "ERRORS", "RETURN" ) ;
			tbsort( "TABND", "("+upper( w2 )+")" ) ;
			ops.push_back( "TBSORT " + d2ds(RC) ) ;
			if ( RC > TRC ) { TRC = RC ; }
			control( "ERRORS", "CANCEL" ) ;
		}
		else if ( w1 == "ADD" )
		{
			SEL = "" ; FLD1 = w2 ; FLD2 = w3 ; FLD3 = w4 ; FLD4 = w5 ;
			tbadd( "TABND" ) ;
			ops.push_back( "TBADD " + d2ds(RC) ) ;
			if ( RC > TRC ) { TRC = RC ; }
		}
		else if ( w1 == "ADDO" )
		{
			SEL = "" ; FLD1 = w2 ; FLD2 = w3 ; FLD3 = w4 ; FLD4 = w5 ;
			tbadd( "TABND", "", "ORDER" ) ;
			ops.push_back( "TBADD " + d2ds(RC) ) ;
			if ( RC > TRC ) { TRC = RC ; }
		}
		i = ztdtop ;
		while ( ztdsels > 0 )
		{
			if ( SEL == "D" )
			{
				tbdelete( "TABND" ) ;
				ops.push_back( "TBDELETE " + d2ds(RC) ) ;
				if ( RC > TRC ) { TRC = RC ; }
			}
			else if ( SEL == "R" )
			{
				SEL = "" ;
				tbadd( "TABND" ) ;
				ops.push_back( "TBADD " + d2ds(RC) ) ;
				if ( RC > TRC ) { TRC = RC ; }
			}
			else if ( SEL == "RO" )
			{
				SEL = "" ;
				tbadd( "TABND", "", "ORDER" ) ;
				ops.push_back( "TBADD " + d2ds(RC) ) ;
				if ( RC > TRC ) { TRC = RC ; }
			}
			else if ( SEL == "G" )
			{
				SEL = "" ;
				tbget( "TABND", "", "YROWID" ) ;
				ops.push_back( "TBGET " + d2ds(RC) ) ;
				if ( RC > TRC ) { TRC = RC ; }
				YFLD1   = FLD1    ;
				YFLD2   = FLD2    ;
				YFLD3   = FLD3    ;
				YFLD4   = FLD4    ;
			}
			if ( ztdsels > 1 )
			{
				tbdispl( "TABND" ) ;
				if ( RC > TRC ) { TRC = RC ; }
				if ( RC > 4 ) { e_loop = true ; break ; }
			}
			else { ztdsels = 0 ; }
		}
		if ( e_loop ) { break ; }
	}
	tbend( "TABND" ) ;
}


void ptest01::opt5()
{
	//
	// opt5 - Test Dynamic Areas (Input & Output)
	//
	// Copy back the din/dout attributes bytes if not a reload
	//

	string MSG      ;
	string din      ;
	string dout     ;
	string attrs    ;
	string usermd   ;
	string datamd   ;
	string sl1      ;
	string sl2      ;
	string sl3      ;
	string CURFLD   ;
	int    CURPOS   ;
	string ZAREA1   ;
	string CAREA1   ;
	string ZSHADOW1 ;
	string ZAREAT1  ;
	int    ZAREAW1  ;
	int    ZAREAD1  ;

	size_t posn     ;

	string ZAREA2   ;
	string ZSHADOW2 ;
	string ZAREAT2  ;
	int    ZAREAW2  ;
	int    ZAREAD2  ;
	int    ZSIZE2   ;

	string ZAREA3   ;
	string ZSHADOW3 ;
	string ZAREAT3  ;
	int    ZAREAW3  ;
	int    ZAREAD3  ;
	size_t ZSIZE3   ;

	vdefine( "ZAREA1 ZSHADOW1 ZAREAT1 ZAREA2 ZSHADOW2 ZAREAT2", &ZAREA1, &ZSHADOW1, &ZAREAT1, &ZAREA2, &ZSHADOW2, &ZAREAT2 ) ;
	vdefine( "ZAREAW1 ZAREAD1 ZAREAW2 ZAREAD2", &ZAREAW1, &ZAREAD1, &ZAREAW2, &ZAREAD2 ) ;

	vdefine( "ZAREA3  ZSHADOW3", &ZAREA3, &ZSHADOW3 ) ;
	vdefine( "ZAREAW3 ZAREAD3", &ZAREAW3, &ZAREAD3 ) ;

	pquery( "PTEST01C", "ZAREA1", "ZAREAT1", "ZAREAW1", "ZAREAD1" ) ;
	if ( RC > 0 )
	{
		llog( "E", "Error in PQUERY command.  RC=" << RC << endl ) ;
		return ;
	}
	pquery( "PTEST01C", "ZAREA2", "ZAREAT2", "ZAREAW2", "ZAREAD2" ) ;
	if ( RC > 0 )
	{
		llog( "E", "Error in PQUERY command.  RC=" << RC << endl ) ;
		return ;
	}
	pquery( "PTEST01C", "ZAREA3", "ZAREAT3", "ZAREAW3", "ZAREAD3" ) ;
	if ( RC > 0 )
	{
		llog( "E", "Error in PQUERY command.  RC=" << RC << endl ) ;
		return ;
	}

	din    = string( 1, 0x01 ) ;
	dout   = string( 1, 0x02 ) ;
	usermd = string( 1, 0x03 ) ;
	datamd = string( 1, 0x04 ) ;

	attrs  = usermd + datamd ;

	ZSIZE2 = ZAREAW2 * ZAREAD2 ;

	//                 1234567890123456789012345678901234567890
	ZAREA1          = "Hello World                             " ;
	ZAREA1 = ZAREA1 + "Input1:" + din + "Test Data Input             " + dout + "End" ;
	ZAREA1 = ZAREA1 + "ZxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxZ" ;
	ZAREA1 = ZAREA1 + "Input2:" + din + "Test Data Input             " + dout + "End" ;
	ZAREA1 = ZAREA1 + "Input 3 Below                          Z" ;
	ZAREA1 = ZAREA1 + din + "Test Data Input                       " + dout ;
	ZAREA1 = ZAREA1 + "Input 4 Below (no dataout attr)        Z" ;
	ZAREA1 = ZAREA1 + din + "Test Data Input                        " ;
	ZAREA1 = ZAREA1 + "ZyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyZ" ;
	ZAREA1 = ZAREA1 + "Input 5 and 6 Below (no dataout attr)   " ;
	ZAREA1 = ZAREA1 + "Input5:" + din + "Test Data   " + dout + "Input 6:" + din + "Test Data " ;
	ZAREA1 = ZAREA1 + "ZooooooooooooooooooooooooooooooooooooooZ" ;

	CAREA1 = ZAREA1 ;
	sl2 = string( ZAREAW1, N_TURQ ) ;
	sl1 = string( ZAREAW1, N_GREEN ) ;
	sl1.replace( 0,  10, 10, N_WHITE ) ;
	sl1.replace( 10, 10, 10, N_BLUE ) ;
	sl1.replace( 20, 10, 10, N_YELLOW ) ;

	sl3 = string( ZSIZE2, N_TURQ ) ;

	ZSHADOW1 = sl2 ;
	ZSHADOW1 = ZSHADOW1 + sl1 ;
	ZSHADOW1 = ZSHADOW1 + sl2 ;
	ZSHADOW1 = ZSHADOW1 + sl1 ;
	ZSHADOW1 = ZSHADOW1 + sl2 ;
	ZSHADOW1 = ZSHADOW1 + sl1 ;
	ZSHADOW1 = ZSHADOW1 + sl2 ;
	ZSHADOW1 = ZSHADOW1 + sl1 ;
	ZSHADOW1 = ZSHADOW1 + sl2 ;
	ZSHADOW1 = ZSHADOW1 + sl2 ;
	ZSHADOW1 = ZSHADOW1 + sl1 ;
	ZSHADOW1 = ZSHADOW1 + sl2 ;

	ZAREA2   = "The quick brown fox jumps of the lazy dog" ;
	ZSHADOW2 = sl3 ;

	ZSIZE3 = ZAREAW3 * ZAREAD3 ;
	//                 1234567890123456789012345678901234567890
	ZAREA3          = "Test dynamic areas in a scrollable area " ;
	ZAREA3 = ZAREA3 + "Input1:" + din + "Test Data Input             " + dout + "End" ;
	ZAREA3 = ZAREA3 + "ZxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxZ" ;
	ZAREA3 = ZAREA3 + "Input2:" + din + "Test Data Input             " + dout + "End" ;
	ZAREA3 = ZAREA3 + "Input 3 Below                          Z" ;
	ZAREA3 = ZAREA3 + din + "Test Data Input                       " + dout ;
	ZAREA3 = ZAREA3 + "Input 4 Below (no dataout attr)        Z" ;
	ZAREA3 = ZAREA3 + din + "Test Data Input                        " ;
	ZAREA3 = ZAREA3 + "ZyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyZ" ;
	ZAREA3 = ZAREA3 + "Input 5 and 6 Below (no dataout attr)   " ;
	ZAREA3 = ZAREA3 + "Input5:" + din + "Test Data   " + dout + "Input 6:" + din + "Test Data " ;
	ZAREA3 = ZAREA3 + "ZooooooooooooooooooooooooooooooooooooooZ" ;

	while ( true )
	{
		ZAREA3 += ZAREA3 ;
		if ( ZAREA3.size() > ZSIZE3 )
		{
			ZAREA3.resize( ZSIZE3 ) ;
			break ;
		}
	}

	ZSHADOW3 = string( ZAREAW3, N_GREEN ) +
		   string( ZAREAW3, N_BLUE  ) +
		   string( ZAREAW3, N_WHITE ) +
		   string( ZAREAW3, N_RED   ) +
		   string( ZAREAW3, N_TURQ  ) +
		   string( ZAREAW3, N_MAGENTA ) +
		   string( ZAREAW3, N_YELLOW  ) ;

	while ( true )
	{
		ZSHADOW3 += ZSHADOW3 ;
		if ( ZSHADOW3.size() > ZSIZE3 )
		{
			ZSHADOW3.resize( ZSIZE3 ) ;
			break ;
		}
	}

	CURFLD = "ZCMD" ;
	CURPOS = 0      ;
	while ( true )
	{
		if ( MSG == "" ) { zcmd = "" ; }
		display( "PTEST01C", MSG, CURFLD, CURPOS );
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		if ( zcmd == "SHOW" )
		{
			zcmd = "" ;
			llog( "A", "ZAREA   >>" << ZAREA1   << "<<" << endl ; )
			llog( "A", "hex1    >>" << cs2xs1(ZAREA1)  << "<<" << endl ; )
			llog( "A", "hex2    >>" << cs2xs2(ZAREA1)  << "<<" << endl ; )
			llog( "A", "CAREA   >>" << CAREA1   << "<<" << endl ; )
			llog( "A", "ZSHADOW >>" << ZSHADOW1 << "<<" << endl ; )
		}
		iupper( zcmd ) ;
		if ( word( zcmd, 1 ) == "GOTO" )
		{
			CURFLD = "ZAREA1" ;
			CURPOS = ds2d( word( zcmd,2 ) ) ;
			zcmd   = ""       ;
		}
		else
		{
			CURFLD = "ZCMD" ;
			CURPOS = 1      ;
		}

		if ( word( zcmd, 1 ) == "RELOAD" )
		{
			ZAREA1 = CAREA1 ;
			zcmd   = ""     ;
		}

		posn = 48 ;
		if ( ZAREA1.substr( posn-1, 1 ) == usermd )      { llog( "A", "ZAREA1 FLD at posn " << posn <<" has been touched by the user " << endl ; ) }
		else if ( ZAREA1.substr( posn-1, 1 ) == datamd ) { llog( "A", "ZAREA1 FLD at posn " << posn <<" has been changed by the user " << endl ; ) }
		else                                             { llog( "A", "ZAREA1 FLD at posn " << posn <<" has not been changed or touched by the user " << endl ; ) }

		posn = 128 ;
		if ( ZAREA1.substr( posn-1, 1 ) == usermd )      { llog( "A", "ZAREA1 FLD at posn " << posn <<" has been touched by the user " << endl ; ) }
		else if ( ZAREA1.substr( posn-1, 1 ) == datamd ) { llog( "A", "ZAREA1 FLD at posn " << posn <<" has been changed by the user " << endl ; ) }
		else                                             { llog( "A", "ZAREA1 FLD at posn " << posn <<" has not been changed or touched by the user " << endl ; ) }

		posn = 201 ;
		if ( ZAREA1.substr( posn-1, 1 ) == usermd )      { llog( "A", "ZAREA1 FLD at posn " << posn <<" has been touched by the user " << endl ; ) }
		else if ( ZAREA1.substr( posn-1, 1 ) == datamd ) { llog( "A", "ZAREA1 FLD at posn " << posn <<" has been changed by the user " << endl ; ) }
		else                                             { llog( "A", "ZAREA1 FLD at posn " << posn <<" has not been changed or touched by the user " << endl ; ) }

		posn = 281;
		if ( ZAREA1.substr( posn-1, 1 ) == usermd )      { llog( "A", "ZAREA1 FLD at posn " << posn <<" has been touched by the user " << endl ; ) }
		else if ( ZAREA1.substr( posn-1, 1 ) == datamd ) { llog( "A", "ZAREA1 FLD at posn " << posn <<" has been changed by the user " << endl ; ) }
		else                                             { llog( "A", "ZAREA1 FLD at posn " << posn <<" has not been changed or touched by the user " << endl ; ) }

		posn = 408 ;
		if ( ZAREA1.substr( posn-1, 1 ) == usermd )      { llog( "A", "ZAREA1 FLD at posn " << posn <<" has been touched by the user " << endl ; ) }
		else if ( ZAREA1.substr( posn-1, 1 ) == datamd ) { llog( "A", "ZAREA1 FLD at posn " << posn <<" has been changed by the user " << endl ; ) }
		else                                             { llog( "A", "ZAREA1 FLD at posn " << posn <<" has not been changed or touched by the user " << endl ; ) }

		posn = 430 ;
		if ( ZAREA1.substr( posn-1, 1 ) == usermd )      { llog( "A", "ZAREA1 FLD at posn " << posn <<" has been touched by the user " << endl ; ) }
		else if ( ZAREA1.substr( posn-1, 1 ) == datamd ) { llog( "A", "ZAREA1 FLD at posn " << posn <<" has been changed by the user " << endl ; ) }
		else                                             { llog( "A", "ZAREA1 FLD at posn " << posn <<" has not been changed or touched by the user " << endl ; ) }

		posn = 0 ;
		while ( true )
		{
			posn = ZAREA1.find_first_of( attrs, posn ) ;
			if ( posn == string::npos ) { break ; }
			ZAREA1[ posn ] = din.front() ;
			++posn ;
		}

		if ( zcmd != "" ) { MSG = "PSYS018" ; }
	}
}


void ptest01::opt6()
{
	//
	// opt6 - Test Dynamic Areas (Overflow field)
	//
	// Dynamic area defined with ZOVR prefix for the overflow fields.
	//
	// Copy back the din/dout attributes bytes if not a reload
	//

	int i ;

	string MSG      ;
	string din      ;
	string dout     ;
	string attrs    ;
	string usermd   ;
	string datamd   ;
	string sl1      ;
	string sl2      ;
	string sl3      ;
	string var      ;
	string CURFLD   ;
	int    CURPOS   ;
	string ZAREA    ;
	string CAREA    ;
	string ZSHADOW  ;
	string CSHADOW  ;
	string ZAREAT   ;
	int    ZAREAW   ;
	int    ZAREAD   ;

	size_t posn ;

	string* strptr ;

	vector<string*> values ;
	vector<string*> shadows ;
	vector<string*> updates ;

	vdefine( "ZAREA ZSHADOW ZAREAT", &ZAREA, &ZSHADOW, &ZAREAT ) ;
	vdefine( "ZAREAW ZAREAD", &ZAREAW, &ZAREAD ) ;

	values.reserve( 10 ) ;
	shadows.reserve( 10 ) ;
	updates.reserve( 10 ) ;

	for ( i = 0 ; i < 10 ; ++i )
	{
		strptr = new string ;
		var = "ZOVRV" + d2ds( i, 3 ) ;
		vdefine( var, strptr ) ;
		values.push_back( strptr ) ;
		strptr = new string ;
		var = "ZOVRS" + d2ds( i, 3 ) ;
		vdefine( var, strptr ) ;
		shadows.push_back( strptr ) ;
		strptr = new string ;
		var = "ZOVRU" + d2ds( i, 3 ) ;
		vdefine( var, strptr ) ;
		updates.push_back( strptr ) ;
	}

	string text ;

	pquery( "PTEST01H", "ZAREA", "ZAREAT", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 )
	{
		llog( "E", "Error in PQUERY command.  RC=" << RC << endl ) ;
		return ;
	}

	din    = string( 1, 0x01 ) ;
	dout   = string( 1, 0x02 ) ;
	usermd = string( 1, 0x03 ) ;
	datamd = string( 1, 0x04 ) ;

	attrs  = usermd + datamd ;

	ZAREA   = "" ;
	ZSHADOW = "" ;

	ZSHADOW = string( 10, N_GREEN ) +
		  string( 10, N_BLUE  ) +
		  string( 10, N_WHITE ) +
		  string( 10, N_RED   ) +
		  string( 10, N_TURQ  ) +
		  string( 10, N_MAGENTA ) +
		  string( 10, N_YELLOW  ) ;

	sl1 = string( 8, N_GREEN ) ;
	sl2 = string( 62, N_BLUE ) ;

	text = "The quick brown fox jumps over the lazy dog" ;
	text.resize( 62, ' ' ) ;

	for ( i = 0 ; i < 10 ; ++i )
	{
		ZAREA   = ZAREA + din + d2ds( i, 6 ) + din + text ;
		ZSHADOW = ZSHADOW + ZSHADOW ;
	}

	*values[ 0 ] = "Hello world" ;
	*values[ 1 ] = "Bye world" ;
	*shadows[ 0 ] = string( 11, N_RED ) ;
	*shadows[ 1 ] = string( 9, N_RED ) ;
	*updates[ 0 ] = "N" ;
	*updates[ 1 ] = "N" ;

	CAREA   = ZAREA ;
	CSHADOW = ZSHADOW ;

	CURFLD = "ZCMD" ;
	CURPOS = 0 ;

	while ( true )
	{
		if ( MSG == "" ) { zcmd = "" ; }
		display( "PTEST01H", MSG, CURFLD, CURPOS );
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		if ( zcmd == "SHOW" )
		{
			zcmd = "" ;
			llog( "A", "ZAREA   >>" << ZAREA   << "<<" << endl ; )
			llog( "A", "hex1    >>" << cs2xs1(ZAREA)  << "<<" << endl ; )
			llog( "A", "hex2    >>" << cs2xs2(ZAREA)  << "<<" << endl ; )
			llog( "A", "CAREA   >>" << CAREA   << "<<" << endl ; )
			llog( "A", "ZSHADOW >>" << ZSHADOW << "<<" << endl ; )
			for ( i = 0 ; i < 10 ; ++i )
			{
				strptr = values[ i ] ;
				llog( "A", "OV VALUE  " <<i<< " >>" << *strptr << "<<" << endl ; )
				strptr = shadows[ i ] ;
				llog( "A", "OV SHADOW " <<i<< " >>" << *strptr << "<<" << endl ; )
				strptr = updates[ i ] ;
				llog( "A", "OV UPDATE " <<i<< " >>" << *strptr << "<<" << endl ; )
			}
		}

		iupper( zcmd ) ;
		CURFLD = "ZCMD" ;
		CURPOS = 1      ;

		if ( word( zcmd, 1 ) == "RELOAD" )
		{
			ZAREA   = CAREA ;
			ZSHADOW = CSHADOW ;
			zcmd    = "" ;
		}

		posn = 8 ;
		if ( ZAREA.substr( posn-1, 1 ) == usermd )      { llog( "A", "ZAREA FLD at posn " << posn <<" has been touched by the user " << endl ; ) }
		else if ( ZAREA.substr( posn-1, 1 ) == datamd ) { llog( "A", "ZAREA FLD at posn " << posn <<" has been changed by the user " << endl ; ) }
		else                                            { llog( "A", "ZAREA FLD at posn " << posn <<" has not been changed or touched by the user " << endl ; ) }

		posn = 0 ;
		while ( true )
		{
			posn = ZAREA.find_first_of( attrs, posn ) ;
			if ( posn == string::npos ) { break ; }
			ZAREA[ posn ] = 0x01 ;
			++posn ;
		}

		if ( zcmd != "" ) { MSG = "PSYS018" ; }
	}

	for ( i = 0 ; i < 10 ; ++i )
	{
		strptr = values[ i ] ;
		delete strptr ;
		strptr = shadows[ i ] ;
		delete strptr ;
		strptr = updates[ i ] ;
		delete strptr ;
	}

}


void ptest01::opt7()
{
	int i ;

	string MSG    ;
	string CURFLD ;
	int    CURPOS ;

	int zwidth ;
	int zdepth ;

	vdefine( "ZWIDTH ZDEPTH", &zwidth, &zdepth ) ;

	i = 7 ;

	zwidth = 70 ;
	zdepth = 20 ;
	while ( true )
	{
		CURFLD = "ZCMD" ;
		CURPOS = 1      ;
		if ( MSG == "" ) { zcmd = "" ; }
		display( "PTEST01E", MSG, CURFLD, CURPOS ) ;
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		addpop( "", i, i ) ;
		while ( true )
		{
			CURFLD = "ZCMD1" ;
			CURPOS = 1       ;
			if ( MSG == "" ) { zcmd = "" ; }
			display( "PTEST01F", MSG, CURFLD, CURPOS ) ;
			if ( RC == 8 ) { break ; }
			MSG = "" ;
		}
		rempop() ;
	}
	vdelete( "ZWIDTH ZDEPTH" ) ;
}


void ptest01::opt9()
{
	//
	// PANEXIT REXX and *REXX panel statements
	//

	while ( true )
	{
		display( "PTEST01G" ) ;
		if ( RC == 8 ) { break ; }
	}
}



void ptest01::opt10()
{
	//
	// PANEXIT LOAD statement
	//

	while ( true )
	{
		display( "PTEST01I" ) ;
		if ( RC == 8 ) { break ; }
	}
}
