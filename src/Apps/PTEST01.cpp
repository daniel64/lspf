/*  Compile with ::                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPTEST01.so -o libPTEST01.so PTEST01.cpp */

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


/*                                                                          */
/* opt0 - Test Panel Functions                                              */
/* opt1 - Perform operations on keyed table                                 */
/* opt2 - Perform operations on non-keyed table                             */
/* opt3 - Test Table Display for keyed table                                */
/* opt4 - Test Table Display for non-keyed table                            */
/* opt5 - Test Dynamic Areas                                                */
/* opt6 - Test popups                                                       */

#include <iostream>
#include <string>
#include "../lspf.h"
#include "../utilities.h"
#include "../pWidgets.h"
#include "../pVPOOL.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"
#include "PTEST01.h"

using namespace std ;

#undef LOGOUT
#undef MOD_NAME

#define LOGOUT aplog
#define MOD_NAME PTEST01

void PTEST01::application()
{
	log( "I", "Application PTEST01 starting." << endl ) ;

	char a ;
	string abc ;

//	verase( "ZPLIB ZMLIB ZTLIB ZHLIB" ) ;
//	cleanup() ;
//	return ;

//	ZTUSER = "/home/daniel/.lspf/" ;
//	libdef( "ZTUSER", "FILE" ) ;

	vdefine( "ZCMD ZVERB", &ZCMD, &ZVERB ) ;

	if ( ZCMD == "WAIT" )
	{
		debug1( "Waiting forever...." << endl ) ;
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000000)) ;
	}
	else if ( ZCMD == "LOOP" )
	{
		debug1( "Looping forever...." << endl ) ;
		while ( true ) { PARM = "99" ; }
	}
	else if ( ZCMD == "ABEND" )
	{
		debug1( "Abending...." << endl ) ;
		abend() ;
	}
	else if ( ZCMD == "ABORT" )
	{
		debug1( "Aborting...." << endl ) ;
		abort() ;
	}
	else if ( ZCMD == "TERMINATE" )
	{
		debug1( "Terminating..." << endl ) ;
		terminate() ;
	}
	else if ( ZCMD == "EXCEPTION" )
	{
		debug1( "Generating an exception...." << endl ) ;
		a = abc[ 4 ] ;
	}

	if      ( PARM == "0" ) opt0() ;
	else if ( PARM == "1" ) opt1() ;
	else if ( PARM == "2" ) opt2() ;
	else if ( PARM == "3" ) opt3() ;
	else if ( PARM == "4" ) opt4() ;
	else if ( PARM == "5" ) opt5() ;
	else if ( PARM == "6" ) opt6() ;
	else if ( PARM == "7" ) opt7() ;
	else if ( PARM == "ABEND" )
	{
		debug1( "Abending...." << endl ) ;
		abend() ;
	}

	cleanup() ;
        return    ;
}


void PTEST01::opt0()
{
	// opt0 - Test Panel Functions

	string MSG ;
	MSG  = "" ;

	while ( true )
        {
		ZCMD = "" ;
		display( "PTEST01A", MSG, "ZCMD" );
		if ( RC >  8 ) { abend()   ; return ; }
		if ( RC == 8 ) { cleanup() ; break ; }
		MSG  = "" ;

		vget( "ZVERB" ) ;
	}

}


void PTEST01::opt1()
{

	// opt1 - Perform operations on keyed table

	string MSG, TABA1, TABA2, TABA3, TABA4, TABA5 ;
	string TBQ1, TBQ2  ;
	string TBQ7, TBQ8, TBQ9, TBQ10 ;

	int TBQ3, TBQ4, TBQ5, TBQ6 ;

	vdefine( "TABA1 TABA2 TABA3 TABA4 TABA5", &TABA1, &TABA2, &TABA3, &TABA4, &TABA5 ) ;
	vdefine( "TBQ1 TBQ2  ",  &TBQ1, &TBQ2  ) ;
	vdefine( "TBQ7 TBQ8 TBQ9 TBQ10", &TBQ7, &TBQ8, &TBQ9, &TBQ10 ) ;
	vdefine( "TBQ3 TBQ4 TBQ5 TBQ6", &TBQ3, &TBQ4, &TBQ5, &TBQ6 ) ;

	MSG  = "" ;

//	tbopen( "TABK", WRITE, "/home/daniel/.lspf/" ) ;
	tbopen( "TABK", WRITE ) ;
	log( "A", "TBOPEN TABK WRITE no file name RC=" << RC << endl ) ;

	tbsave( "TABK" ) ;
	log( "A", "TBSAVE TABK no file name RC=" << RC << endl ) ;

	tbend( "TABK" ) ;
	log( "A", "TBEND TABK RC=" << RC << endl ) ;

	tberase( "TABK" ) ;
	log( "A", "TBERASE TABK RC=" << RC << endl ) ;

	tbcreate( "TABK", "TABA1", "TABA2 TABA3 TABA4 TABA5", WRITE ) ;
	log( "A", "TBCREATE TABK with WRITE option RC=" << RC << endl ) ;

	tbsort( "TABK", "TABA1,C,D" ) ;
	log( "A", "TBSORT TABK RC=" << RC << endl ) ;

	tbvclear( "TABK" ) ;
	log( "A", "TBVCLEAR TABK RC=" << RC << endl ) ;

	TABA1 = "KEY2"    ; TABA2 = "AALUE11" ; TABA3 = "VALUE222" ; TABA4 = "VALUE3333" ; TABA5 = "VALUE44444" ;
	tbadd( "TABK", "", "ORDER" ) ;
	log( "A", "TBADD TABK RC=" << RC << endl ) ;
	TABA1 = "KEY1"    ; TABA2 = "CALUE11" ; TABA3 = "VALUE222" ; TABA4 = "VALUE3333" ; TABA5 = "VALUE44444" ;
	tbadd( "TABK", "", "ORDER"  ) ;
	log( "A", "TBADD TABK RC=" << RC << endl ) ;
	TABA1 = "KEY3"    ; TABA2 = "BALUE11" ; TABA3 = "VALUE222" ; TABA4 = "VALUE3333" ; TABA5 = "VALUE44444" ;
	tbadd( "TABK", "", "ORDER"  ) ;
	log( "A", "TBADD TABK RC=" << RC << endl ) ;

	int i ;
	for ( i = 4 ; i > 0 ; i-- )
	{
		TABA1 = "TABKEY" + d2ds(i) ; TABA2 = "VALUE B1 " ; TABA3 = "VALUE B2  " ; TABA4 = "VALUE B3   " ; TABA5 = "VALUE B4    " ;
		tbadd( "TABK"  ) ;
		log( "A", "TBADD TABK with key >>" << TABA1 << "<<  RC=" << RC << endl ) ;
	}

	tbbottom( "TABK" ) ;
	log( "A", "TBADD TABK RC=" << RC << endl ) ;

	TABA1 = "KEY4"    ; TABA2 = "VALUE11" ; TABA3 = "VALUE222" ; TABA4 = "VALUE3333" ; TABA5 = "VALUE44444" ;
	tbadd( "TABK", "", "ORDER"  ) ;
	log( "A", "TBADD TABK RC=" << RC << endl ) ;

	tbtop( "TABK" ) ;
	while ( true )
	{
		tbskip( "TABK", 1 ) ;
		if ( RC > 0 ) { break ; }
		log( "A", "TBSKIP TABK VAR VALUES: TABA1 " << TABA1 << " TABA2 " << TABA2  << " TABA3 " << TABA3  << " TABA4 " << TABA4  << " TABA5 " << TABA5 << endl ) ;
	}


	TABA1 = "TABKEY2" ;
	tbdelete( "TABK" ) ;
	log( "A", "TBDELETE TABK for TABKEY2 RC=" << RC << endl ) ;

	tbtop( "TABK" ) ;
	while ( true )
	{
		tbskip( "TABK", 1 ) ;
		if ( RC > 0 ) { break ; }
		log( "A", "TBSKIP TABK VAR VALUES: TABA1 " << TABA1 << " TABA2 " << TABA2  << " TABA3 " << TABA3  << " TABA4 " << TABA4  << " TABA5 " << TABA5 << endl ) ;
	}

	tbsort( "TABK", "TABA2,C,D" ) ;

	TABA1 = "TABKEY999" ;
	tbdelete( "TABK" ) ;
	log( "A", "TBDELETE TABK for TABKEY999 RC=" << RC << endl ) ;


//	tbsave( "TABK", "" , "/home/daniel/.lspf/" ) ;
	tbsave( "TABK" ) ;
	log( "A", "TBSAVE TABK no file name RC=" << RC << endl ) ;

	tbend( "TABK" ) ;
	log( "A", "TBEND TABK RC=" << RC << endl ) ;

	tbopen( "TABK", NOWRITE ) ;
	log( "A", "TBOPEN TABK NOWRITE RC=" << RC << endl ) ;

	tbsave( "TABK", "TEST111" , "/home/daniel/.lspf/" ) ;
	log( "A", "TBSAVE TABK with file name ( RC=12? ) RC=" << RC << endl ) ;

	tbend( "TABK" ) ;
	log( "A", "TBEND TABK RC=" << RC << endl ) ;

	tbopen( "TABK", WRITE ) ;
	log( "A", "TBOPEN TABK NOWRITE RC=" << RC << endl ) ;

	TABA1 = "KEY2" ;
	tbdelete( "TABK" );
	log( "A", "TBDELETE TABK KEY IS KEY2 RC=" << RC << endl ) ;

	tbquery( "TABK", "TBQ1","TBQ2", "TBQ3", "TBQ4", "TBQ5", "TBQ6", "TBQ7", "TBQ8", "TBQ9", "TBQ10" ) ;
	log( "A", "CRP........" << TBQ6  << endl ) ;

	TABA1 = "KEY1"    ; TABA2 = "VALUE11" ; TABA3 = "VALUE222" ; TABA4 = "VALUE3333" ; TABA5 = "VALUE44444" ;
	tbadd( "TABK" ) ;
	log( "A", "TBADD TABK KEY IS KEY1 (exists) RC=" << RC << endl ) ;

	tbquery( "TABK", "TBQ1","TBQ2", "TBQ3", "TBQ4", "TBQ5", "TBQ6", "TBQ7", "TBQ8", "TBQ9", "TBQ10" ) ;
	log( "A", "CRP........" << TBQ6  << endl ) ;

	TABA1 = "KEY2"    ; TABA2 = "VALUE11" ; TABA3 = "VALUE222" ; TABA4 = "VALUE3333" ; TABA5 = "VALUE44444" ;
	tbadd( "TABK" ) ;
	log( "A", "TBADD TABK KEY IS KEY2 (should be okay) RC=" << RC << endl ) ;

	tbquery( "TABK", "TBQ1","TBQ2", "TBQ3", "TBQ4", "TBQ5", "TBQ6", "TBQ7", "TBQ8", "TBQ9", "TBQ10" ) ;
	log( "A", "CRP........" << TBQ6  << endl ) ;

	tbtop( "TABK" ) ;
	while ( true )
	{
		tbskip( "TABK", 1 ) ;
		if ( RC > 0 ) { break ; }
		log( "A", "TBSKIP TABK VAR VALUES: TABA1 " << TABA1 << " TABA2 " << TABA2  << " TABA3 " << TABA3  << " TABA4 " << TABA4  << " TABA5 " << TABA5 << endl ) ;
	}

	TABA1 = "KEY2"    ; TABA2 = "VALUENEW" ; TABA3 = "VALUENEW" ; TABA4 = "VALUENEW" ; TABA5 = "VALUE4NEW" ;
	tbput( "TABK" ) ;
	log( "A", "TBPUT TABK KEY IS KEY2 (should be okay) RC=" << RC << endl ) ;

	tbquery( "TABK", "TBQ1","TBQ2", "TBQ3", "TBQ4", "TBQ5", "TBQ6", "TBQ7", "TBQ8", "TBQ9", "TBQ10" ) ;
	log( "A", "CRP........" << TBQ6  << endl ) ;

	TABA1 = "KEY2"    ;
	tbget( "TABK" ) ;
	log( "A", "TBGET TABK KEY IS KEY2 (should be okay) RC=" << RC << endl ) ;

	TABA1 = "KEY2"    ; TABA2 = "VALUENEW" ; TABA3 = "VALUENEW" ; TABA4 = "VALUENEW" ; TABA5 = "VALUE4NEW" ;
	tbput( "TABK" ) ;
	log( "A", "TBPUT TABK KEY IS KEY2 (should be okay) RC=" << RC << endl ) ;

	tbsort( "TABK", "TABA2,C,D" ) ;
	tbsarg( "TABK", "", "NEXT", "(TABA5,EQ)" ) ;


	tbquery( "TABK", "TBQ1","TBQ2", "TBQ3", "TBQ4", "TBQ5", "TBQ6", "TBQ7", "TBQ8", "TBQ9", "TBQ10" ) ;

	log( "A", "TBQUERY RC=" << RC << endl ) ;
	log( "A", "-" << RC << endl ) ;
	log( "A", "Key name......." << TBQ1  << endl ) ;
	log( "A", "Var name......." << TBQ2  << endl ) ;
	log( "A", "Row Num........" << TBQ3  << endl ) ;
	log( "A", "Key Num........" << TBQ4  << endl ) ;
	log( "A", "Name num......." << TBQ5  << endl ) ;
	log( "A", "CRP............" << TBQ6  << endl ) ;
	log( "A", "Sort IR........" << TBQ7  << endl ) ;
	log( "A", "Name List......" << TBQ8  << endl ) ;
	log( "A", "Cond List......" << TBQ9  << endl ) ;
	log( "A", "Direction......" << TBQ10 << endl ) ;

	tbend( "TABK" ) ;
	log( "A", "TBEND TABK RC=" << RC << endl ) ;

	vdelete( "TABA1 TABA2 TABA3 TABA4 TABA5" ) ;
}


void PTEST01::opt2()
{
	// opt2 - Perform operations on non-keyed table

	int i ;
	int r ;

	string EXTV1, EXTV2, EXTV3, EXTV4, EXTLST ;
	vdefine( "EXTV1 EXTV2 EXTV3 EXTV4 EXTLST", &EXTV1, &EXTV2, &EXTV3, &EXTV4, &EXTLST ) ;

	string TABB1, TABB2, TABB3, TABB4, TABB5 ;
	vdefine( "TABB1 TABB2 TABB3 TABB4 TABB5", &TABB1, &TABB2, &TABB3, &TABB4, &TABB5 ) ;

	tbcreate( "TABN", "", "TABB1 TABB2 TABB3 TABB4 TABB5", WRITE ) ;
	log( "A", "TBCREATE TABN RC=" << RC << endl ) ;

	tbvclear( "TABN" ) ;
	log( "A", "TBVCLEAR TABN RC=" << RC << endl ) ;


	r = 0 ;
	for ( i = 10 ; i > 0 ; i-- )
	{
		TABB1 = "NOTAKEY" ; TABB2 = "VALUE "+d2ds(i) ; TABB3 = "VALUE "+d2ds(i)  ; TABB4 = "VALUE  "+d2ds(i)  ; TABB5 = "VALUE     "+d2ds(i)  ;
		tbadd( "TABN", "", "", r ) ;
		r = 0 ;
		log( "A", "TBADD TABN RC=" << RC << " Record number being loaded is " << i << endl ) ;
	}

//	tbend("TABN" ) ;
//	return ;
	TABB1 = "NOTAKEY" ; TABB2 = "VALUE WITH EXT VARS" ; TABB3 = "VALUE WITH EXT VARS"  ; TABB4 = "VALUE4" ; TABB5 = "VALUE4"  ;

	EXTV1 = "EXTENSION VAR 1 " ;
	EXTV2 = "EXTENSION VAR 2 " ;
	EXTV3 = "EXTENSION VAR 3 " ;
	EXTV4 = "EXTENSION VAR 4 " ;

	tbadd( "TABN", "EXTV1 EXTV2 EXTV3 EXTV4" ) ;
	log( "A", "TBADD TABN RC=" << RC << " Record number being loaded is custom" << endl ) ;

	TABB1 = "NOTAKEY" ; TABB2 = "VALUE3" ; TABB3 = "VALUE3"  ; TABB4 = "VALUE3" ; TABB5 = "VALUE3"  ;
	tbadd( "TABN" ) ;
	log( "A", "TBADD TABN RC=" << RC << " Record number being loaded is custom" << endl ) ;

	tbtop( "TABN" ) ;
	log( "A", "TBTOP TABN RC=" << RC << endl ) ;

	tbvclear( "TABN" ) ;
	TABB1 = "NOTAKEY" ;
	EXTV4 = "EXTENSION VA*" ;
	tbsarg( "TABN", "EXTV4", "", "" ) ;
	log( "A", "TBSARG TABN  RC=" << RC << endl ) ;

	EXTV1 = "" ;
	EXTV2 = "" ;
	EXTV3 = "" ;
	EXTV4 = "" ;

	TABB1 = "NOTAKEY" ;
	EXTV4 = "EXTENSION VA*" ;
	tbscan( "TABN" ) ;
	log( "A", "TBSCAN (no TBSARG) TABN  RC=" << RC << endl ) ;
	log( "A", "TBSCAN TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	log( "A", "EXTLST >>" << EXTLST << "<<"  << endl ) ;
	log( "A", "EXTV1 >>" << EXTV1 << "<<"  << endl ) ;
	log( "A", "EXTV2 >>" << EXTV2 << "<<"  << endl ) ;
	log( "A", "EXTV3 >>" << EXTV3 << "<<"  << endl ) ;
	log( "A", "EXTV4 >>" << EXTV4 << "<<"  << endl ) ;


	tbvclear( "TABN" ) ;
	TABB1 = "NOTAKEY" ;
	EXTV4 = "EXTENSION VAx*" ;
	tbsarg( "TABN", "EXTV4", "", "" ) ;
	log( "A", "TBSARG TABN  RC=" << RC << endl ) ;
	tbscan( "TABN" ) ;
	log( "A", "TBSCAN TABN  RC=" << RC << endl ) ;
	log( "A", "TBSCAN TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	log( "A", "EXTLST >>" << EXTLST << "<<"  << endl ) ;
	log( "A", "EXTV1 >>" << EXTV1 << "<<"  << endl ) ;
	log( "A", "EXTV2 >>" << EXTV2 << "<<"  << endl ) ;
	log( "A", "EXTV3 >>" << EXTV3 << "<<"  << endl ) ;
	log( "A", "EXTV4 >>" << EXTV4 << "<<"  << endl ) ;

	tbscan( "TABN" ) ;
	log( "A", "TBSCAN TABN  RC=" << RC << endl ) ;

	tbtop( "TABN" ) ;
	log( "A", "TBTOP TABN RC=" << RC << endl ) ;


	tbskip( "TABN", 1 ) ;
	log( "A", "TBSKIP TABN 1 RECORD RC=" << RC << endl ) ;

	EXTV1 = "" ;
	EXTV2 = "" ;
	EXTV3 = "" ;
	EXTV4 = "" ;

	tbget( "TABN" ) ;
	log( "A", "TBGET TABN 1 RECORD RC=" << RC << endl ) ;
	log( "A", "EXTV1 >>" << EXTV1 << "<<"  << endl ) ;

	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		if ( RC > 8 ) { log( "A", "SEVERE ERROR **** TBSKIP" << endl ) ; }
		if ( RC > 0 ) { break ; }
		log( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
		tbget( "TABN", "EXTLST" ) ;
		log( "A", "TBGET TABN 1 RECORD RC=" << RC << endl ) ;
		log( "A", "EXTLST >>" << EXTLST << "<<"  << endl ) ;
		log( "A", "EXTV1 >>" << EXTV1 << "<<"  << endl ) ;
		log( "A", "EXTV2 >>" << EXTV2 << "<<"  << endl ) ;
		log( "A", "EXTV3 >>" << EXTV3 << "<<"  << endl ) ;
		log( "A", "EXTV4 >>" << EXTV4 << "<<"  << endl ) ;
		EXTV1 = "" ;
		EXTV2 = "" ;
		EXTV3 = "" ;
		EXTV4 = "" ;
	}

	tbsort( "TABN", "TABB3" ) ;
	log( "A", "TBSORT TABN RC=" << RC << endl ) ;

	tbsave( "TABN", "" , "/home/daniel/.lspf/" ) ;
	log( "A", "TBASAVE TABN to file name RC=" << RC << endl ) ;

	tbend( "TABN" ) ;
	log( "A", "TBEND TABN RC=" << RC << endl ) ;

	tbopen( "TABN", NOWRITE, "/home/daniel/.lspf/" ) ;
	log( "A", "TBOPEN TABN with NOWRITE RC=" << RC << endl ) ;

	tbtop( "TABN" ) ;
	log( "A", "TBTOP TABN RC=" << RC << endl ) ;

	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		log( "A", "TBSKIP TABN 1 RECORD RC=" << RC << endl ) ;
		if ( RC > 0 ) { break ; }
		log( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	}

	tbtop( "TABN" ) ;
	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		log( "A", "TBSKIP TABN 1 RECORD RC=" << RC << endl ) ;
		if ( RC > 0 ) { break ; }
		log( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	}

	tbsort( "TABN", "TABB1" ) ;
	log( "A", "TBSORT TABN RC=" << RC << endl ) ;
	tbtop( "TABN" ) ;
	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		if ( RC > 0 ) { break ; }
		log( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	}
	tbsort( "TABN", "TABB2" ) ;
	log( "A", "TBSORT TABN RC=" << RC << endl ) ;

	tbtop( "TABN" ) ;
	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		if ( RC > 0 ) { break ; }
		log( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	}

	tbsort( "TABN", "TABB3" ) ;
	log( "A", "TBSORT TABN RC=" << RC << endl ) ;
	tbtop( "TABN" ) ;
	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		if ( RC > 0 ) { break ; }
		log( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	}

	tbsort( "TABN", "TABB4" ) ;
	log( "A", "TBSORT TABN RC=" << RC << endl ) ;
	tbtop( "TABN" ) ;
	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		if ( RC > 0 ) { break ; }
		log( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	}

	tbsort( "TABN", "TABB5" ) ;
	log( "A", "TBSORT TABN RC=" << RC << endl ) ;
	tbtop( "TABN" ) ;
	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		if ( RC > 0 ) { break ; }
		log( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	}

	tbsort( "TABN", "TABB6" ) ;
	log( "A", "TBSORT TABN RC=" << RC << endl ) ;

	tbtop( "TABN" ) ;
	tbskip( "TABN", 4 ) ;
	log( "A", "TBTOP/TBSKIP TABK for CRP = 4 :: RC=" << RC << endl ) ;

	tbdelete( "TABN" ) ;
	log( "A", "TBDELETE TABK for CRP = 4 :: RC=" << RC << endl ) ;

	tbtop( "TABN" ) ;
	while ( true )
	{
		tbskip( "TABN", 1 ) ;
		if ( RC > 0 ) { break ; }
		log( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;
	}

	tbskip( "TABN", 0, "", "", "2" ) ;
	log( "A", "TBSKIP TABN URID=2 RECORD RC=" << RC << endl ) ;

	tbget( "TABN" ) ;
	log( "A", "TBGET RC=" << RC << endl ) ;
	log( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;

	tbskip( "TABN", 0, "", "", "1" ) ;
	log( "A", "TBSKIP TABN URID=1 RECORD RC=" << RC << endl ) ;

	tbget( "TABN" ) ;
	log( "A", "TBGET RC=" << RC << endl ) ;
	log( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;

	tbskip( "TABN", 0, "", "", "999999" ) ;
	log( "A", "TBSKIP TABN URID=999999 RECORD RC=" << RC << endl ) ;

	tbget( "TABN" ) ;
	log( "A", "TBGET RC=" << RC << endl ) ;
	log( "A", "TBGET TABN VAR VALUES: TABB1 " << TABB1 << " TABB2 " << TABB2  << " TABB3 " << TABB3  << " TABB4 " << TABB4  << " TABB5 " << TABB5 << endl ) ;

	tbtop( "TABN" ) ;
	tbdelete( "TABN" ) ;
	log( "A", "TBDELETE TABK for TBTOP RC=" << RC << endl ) ;


	tbend( "TABN" ) ;
	log( "A", "TBEND TABN RC=" << RC << endl ) ;
}


void PTEST01::opt3()
{
	// opt3 - Test Table Display for keyed table

	int i ;
	int j ;
	int TRC     ;
	int YTDSELS ;
	int CRP     ;
	int CSRROW  ;

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

	tbcreate( "TABKD", "KEY1", "SEL FLD1 FLD2 FLD3 FLD4", NOWRITE ) ;
	log( "A", "TBCREATE TABN RC=" << RC << endl ) ;
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
	log( "A", "TBVCLEAR TABN RC=" << RC << endl ) ;
	CRP    = 0  ;
	CSRROW = 0  ;
	ROWID  = "" ;
	ZTDTOP = 1  ;

	while ( true )
	{
		TOPR1 = "" ; TRC1 = "" ; TOPR2 = "" ; TRC2 = "" ; TOPR3 = "" ; TRC3 = "" ; TOPR4 = "" ; TRC4 = "" ; TOPR5 = "" ; TRC5 = "" ;
		ZCMD  = "" ;
		i = ops.size() - 1 ;
		if ( i >= 0 ) { TOPR1 = word( ops[i], 1 ) ; TRC1 = word( ops[i], 2 ) ; i-- ; }
		if ( i >= 0 ) { TOPR2 = word( ops[i], 1 ) ; TRC2 = word( ops[i], 2 ) ; i-- ; }
		if ( i >= 0 ) { TOPR3 = word( ops[i], 1 ) ; TRC3 = word( ops[i], 2 ) ; i-- ; }
		if ( i >= 0 ) { TOPR4 = word( ops[i], 1 ) ; TRC4 = word( ops[i], 2 ) ; i-- ; }
		if ( i >= 0 ) { TOPR5 = word( ops[i], 1 ) ; TRC5 = word( ops[i], 2 ) ; i-- ; }
		tbtop( "TABKD" ) ;
		tbskip( "TABKD", ZTDTOP ) ;
		tbquery( "TABKD", "TBQ1","TBQ2", "TBQ3", "TBQ4", "TBQ5", "TBQ6", "TBQ7", "TBQ8", "TBQ9", "TBQ10" ) ;
		tbdispl( "TABKD", "PTEST01D", MSG, "ZCMD", CSRROW, 0, "NO", "CRP", "ROWID" ) ;
		if ( RC >  8 )  { abend()   ; break ; }
		if ( RC == 8 )  { cleanup() ; break ; }
		TRC   = 0  ;
		if ( RC > TRC ) { TRC = RC ; }
		YTDSELS = ZTDSELS ;
		YFLD1   = FLD1    ;
		YFLD2   = FLD2    ;
		YFLD3   = FLD3    ;
		YFLD4   = FLD4    ;
		YROWID  = ""      ;
		MSG  = "" ;
		vget( "ZVERB" ) ;
		w1 = upper( word( ZCMD, 1 ) ) ;
		w2 = word( ZCMD, 2 ) ;
		w3 = word( ZCMD, 3 ) ;
		w4 = word( ZCMD, 4 ) ;
		w5 = word( ZCMD, 5 ) ;
		if ( w1 == "SORT" )
		{
			control( "ERRORS", "RETURN" ) ;
			tbsort( "TABKD", upper( w2 ) ) ;
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
		i = ZTDTOP ;
		while ( ZTDSELS > 0 )
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
			if ( ZTDSELS > 1 )
			{
				tbdispl( "TABKD" ) ;
				if ( RC > TRC ) { TRC = RC ; }
				if ( RC > 4 ) break ;
			}
			else { ZTDSELS = 0 ; }
		}
	}
	tbend( "TABKD" ) ;

}


void PTEST01::opt4()
{
	// opt4 - Test Table Display and other table functions for non-keyed table 

	int i ;
	int j ;
	int TRC     ;
	int YTDSELS ;
	int CRP     ;
	int CSRROW  ;

	string YFLD1, YFLD2, YFLD3, YFLD4, YROWID ;

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

	tbcreate( "TABND", "", "SEL FLD1 FLD2 FLD3 FLD4", NOWRITE ) ;
	log( "A", "TBCREATE TABN RC=" << RC << endl ) ;
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
	log( "A", "TBVCLEAR TABN RC=" << RC << endl ) ;

	CRP    = 0  ;
	CSRROW = 0  ;
	ROWID  = "" ;
	ZTDTOP = 1  ;
	while ( true )
	{
		TOPR1 = "" ; TRC1 = "" ; TOPR2 = "" ; TRC2 = "" ; TOPR3 = "" ; TRC3 = "" ; TOPR4 = "" ; TRC4 = "" ; TOPR5 = "" ; TRC5 = "" ;
		ZCMD  = "" ;
		i = ops.size() - 1 ;
		if ( i >= 0 ) { TOPR1 = word( ops[i], 1 ) ; TRC1 = word( ops[i], 2 ) ; i-- ; }
		if ( i >= 0 ) { TOPR2 = word( ops[i], 1 ) ; TRC2 = word( ops[i], 2 ) ; i-- ; }
		if ( i >= 0 ) { TOPR3 = word( ops[i], 1 ) ; TRC3 = word( ops[i], 2 ) ; i-- ; }
		if ( i >= 0 ) { TOPR4 = word( ops[i], 1 ) ; TRC4 = word( ops[i], 2 ) ; i-- ; }
		if ( i >= 0 ) { TOPR5 = word( ops[i], 1 ) ; TRC5 = word( ops[i], 2 ) ; i-- ; }
		tbtop( "TABND" ) ;
		tbskip( "TABND", ZTDTOP ) ;
		tbquery( "TABND", "TBQ1","TBQ2", "TBQ3", "TBQ4", "TBQ5", "TBQ6", "TBQ7", "TBQ8", "TBQ9", "TBQ10" ) ;
		tbdispl( "TABND", "PTEST01B", MSG, "ZCMD", CSRROW, 0, "NO", "CRP", "ROWID" ) ;
		if ( RC >  8 )  { abend()   ; break ; }
		if ( RC == 8 )  { cleanup() ; break ; }
		TRC   = 0  ;
		if ( RC > TRC ) { TRC = RC ; }
		YTDSELS = ZTDSELS ;
		YFLD1   = FLD1    ;
		YFLD2   = FLD2    ;
		YFLD3   = FLD3    ;
		YFLD4   = FLD4    ;
		YROWID  = ""      ;
		MSG  = "" ;
		vget( "ZVERB" ) ;
		w1 = upper( word( ZCMD, 1 ) ) ;
		w2 = word( ZCMD, 2 ) ;
		w3 = word( ZCMD, 3 ) ;
		w4 = word( ZCMD, 4 ) ;
		w5 = word( ZCMD, 5 ) ;
		if ( w1 == "SORT" )
		{
			control( "ERRORS", "RETURN" ) ;
			tbsort( "TABND", upper( w2 ) ) ;
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
		i = ZTDTOP ;
		while ( ZTDSELS > 0 )
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
			if ( ZTDSELS > 1 )
			{
				tbdispl( "TABND" ) ;
				if ( RC > TRC ) { TRC = RC ; }
				if ( RC > 4 ) break ;
			}
			else { ZTDSELS = 0 ; }
		}
	}
	tbend( "TABND" ) ;
}


void PTEST01::opt5()
{
	// opt5 - Test Dynamic Areas (Input & Output)

	string MSG      ;
	string din      ;
	string dout     ;
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
	int    ZSIZE1   ;
	int    posn     ;

	string ZAREA2   ;
	string ZSHADOW2 ;
	string ZAREAT2  ;
	int    ZAREAW2  ;
	int    ZAREAD2  ;
	int    ZSIZE2   ;

	vdefine( "ZAREA1 ZSHADOW1 ZAREAT1 ZAREA2 ZSHADOW2 ZAREAT2", &ZAREA1, &ZSHADOW1, &ZAREAT1, &ZAREA2, &ZSHADOW2, &ZAREAT2 ) ;
	vdefine( "ZAREAW1 ZAREAD1 ZAREAW2 ZAREAD2", &ZAREAW1, &ZAREAD1, &ZAREAW2, &ZAREAD2 ) ;

	pquery( "PTEST01C", "ZAREA1", "ZAREAT1", "ZAREAW1", "ZAREAD1" ) ;
	if ( RC > 0 )
	{
		log( "E", "Error in PQUERY command.  RC=" << RC << endl ) ;
		return ;
	}
	pquery( "PTEST01C", "ZAREA2", "ZAREAT2", "ZAREAW2", "ZAREAD2" ) ;
	if ( RC > 0 )
	{
		log( "E", "Error in PQUERY command.  RC=" << RC << endl ) ;
		return ;
	}

	din    = string( 1, 0x01 ) ;
	dout   = string( 1, 0x02 ) ;
	usermd = string( 1, 0x03 ) ;
	datamd = string( 1, 0x04 ) ;

	ZSIZE1 = ZAREAW1 * ZAREAD1 ;
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

	CURFLD = "ZCMD" ;
	CURPOS = 0      ;
	while ( true )
        {
		if ( MSG == "" ) { ZCMD = "" ; }
		display( "PTEST01C", MSG, CURFLD, CURPOS );
		if ( RC >  8 ) { abend()   ; return ; }
		if ( RC == 8 ) { cleanup() ; break ; }
		MSG = "" ;
		vget( "ZVERB" ) ;
		if ( ZCMD == "SHOW" )
		{
			ZCMD = "" ;
			log( "A", "ZAREA >>" << ZAREA1 << "<<" << endl ; )
			log( "A", "ZSHADOW >>" << ZSHADOW1 << "<<" << endl ; )
		}
		ZCMD = upper( ZCMD ) ;
		if ( word( ZCMD, 1 ) == "GOTO" )
		{
			CURFLD = "ZAREA1" ;
			CURPOS = ds2d( word( ZCMD,2 ) ) ;
			ZCMD   = ""       ;
		}
		else
		{
			CURFLD = "ZCMD" ;
			CURPOS = 1      ;
		}
		if ( word( ZCMD, 1 ) == "RELOAD" )
		{
			ZAREA1 = CAREA1 ;
			ZCMD   = ""     ;
		}

		posn = 48 ;
		if ( ZAREA1.substr( posn-1, 1 ) == usermd )      { log( "A", "ZAREA1 FLD at posn " << posn <<" has been touched by the user " << endl ; ) }
		else if ( ZAREA1.substr( posn-1, 1 ) == datamd ) { log( "A", "ZAREA1 FLD at posn " << posn <<" has been changed by the user " << endl ; ) }
		else                                             { log( "A", "ZAREA1 FLD at posn " << posn <<" has not been changed or touched by the user " << endl ; ) }

		posn = 128 ;
		if ( ZAREA1.substr( posn-1, 1 ) == usermd )      { log( "A", "ZAREA1 FLD at posn " << posn <<" has been touched by the user " << endl ; ) }
		else if ( ZAREA1.substr( posn-1, 1 ) == datamd ) { log( "A", "ZAREA1 FLD at posn " << posn <<" has been changed by the user " << endl ; ) }
		else                                             { log( "A", "ZAREA1 FLD at posn " << posn <<" has not been changed or touched by the user " << endl ; ) }

		posn = 201 ;
		if ( ZAREA1.substr( posn-1, 1 ) == usermd )      { log( "A", "ZAREA1 FLD at posn " << posn <<" has been touched by the user " << endl ; ) }
		else if ( ZAREA1.substr( posn-1, 1 ) == datamd ) { log( "A", "ZAREA1 FLD at posn " << posn <<" has been changed by the user " << endl ; ) }
		else                                             { log( "A", "ZAREA1 FLD at posn " << posn <<" has not been changed or touched by the user " << endl ; ) }

		posn = 281;
		if ( ZAREA1.substr( posn-1, 1 ) == usermd )      { log( "A", "ZAREA1 FLD at posn " << posn <<" has been touched by the user " << endl ; ) }
		else if ( ZAREA1.substr( posn-1, 1 ) == datamd ) { log( "A", "ZAREA1 FLD at posn " << posn <<" has been changed by the user " << endl ; ) }
		else                                             { log( "A", "ZAREA1 FLD at posn " << posn <<" has not been changed or touched by the user " << endl ; ) }

		posn = 408 ;
		if ( ZAREA1.substr( posn-1, 1 ) == usermd )      { log( "A", "ZAREA1 FLD at posn " << posn <<" has been touched by the user " << endl ; ) }
		else if ( ZAREA1.substr( posn-1, 1 ) == datamd ) { log( "A", "ZAREA1 FLD at posn " << posn <<" has been changed by the user " << endl ; ) }
		else                                             { log( "A", "ZAREA1 FLD at posn " << posn <<" has not been changed or touched by the user " << endl ; ) }

		posn = 430 ;
		if ( ZAREA1.substr( posn-1, 1 ) == usermd )      { log( "A", "ZAREA1 FLD at posn " << posn <<" has been touched by the user " << endl ; ) }
		else if ( ZAREA1.substr( posn-1, 1 ) == datamd ) { log( "A", "ZAREA1 FLD at posn " << posn <<" has been changed by the user " << endl ; ) }
		else                                             { log( "A", "ZAREA1 FLD at posn " << posn <<" has not been changed or touched by the user " << endl ; ) }


		if ( ZCMD != "" ) { MSG = "PSYS018" ; }
	}
}


void PTEST01::opt6()
{
}


void PTEST01::opt7()
{
	int i ;

	string MSG    ;
	string CURFLD ;
	int    CURPOS ;

	i = 7 ;

	while( true )
	{
		CURFLD = "ZCMD" ;
		CURPOS = 1      ;
		if ( MSG == "" ) { ZCMD = "" ; }
		display( "PTEST01E", MSG, CURFLD, CURPOS ) ;
		if ( RC >  8 ) { abend()   ; return ; }
		if ( RC == 8 ) { break ; }
		MSG = "" ;
		vget( "ZVERB" ) ;
		addpop( "", i, i ) ;
		while( true )
		{
			CURFLD = "ZCMD1" ;
			CURPOS = 1       ;
			if ( MSG == "" ) { ZCMD = "" ; }
			display( "PTEST01F", MSG, CURFLD, CURPOS ) ;
			if ( RC >  8 ) { abend()   ; return ; }
			if ( RC == 8 ) { break ; }
			MSG = "" ;
			vget( "ZVERB" ) ;
		}
		rempop() ;
	}
	return ;
}




// ============================================================================================ //

extern "C" { pApplication *maker() { return new PTEST01 ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
