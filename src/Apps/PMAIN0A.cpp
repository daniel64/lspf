/*  Compile with ::                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPMAIN0A.so -o libPMAIN0A.so PMAIN0A.cpp */

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


#include <iostream>
#include "../lspf.h"
#include "../utilities.h"
#include "../pWidgets.h"
#include "../pVPOOL.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"
#include "PMAIN0A.h"

using namespace std ;
using namespace boost::gregorian;

#undef LOGOUT
#undef MOD_NAME

#define LOGOUT aplog
#define MOD_NAME PMAIN0A

void PMAIN0A::application()
{
	log( "I", "Application PMAIN0A starting.  Displaying panel PMAINP01" << endl ) ;

	int p1   ;
	int y, m ;
	int pmonth, pyear ;

	string S_PGM      ;
	string S_PARM     ;
	string S_NEWAPPL  ;
	bool   S_NEWPOOL  ;
	bool   S_PASSLIB  ;

	string MSG ;
	string w1  ;
	string ws  ;
	string command ;

	string ZSYSNAME ;
	string ZOSREL   ;

	ZAHELP = "HPMAIN1" ;

	vdefine( "ZCMD ZDATEL ZJDATE ZTIME", &ZCMD, &ZDATEL, &ZJDATE, &ZTIME ) ;
	vdefine( "ZAREA ZSHADOW", &ZAREA, &ZSHADOW ) ;

	vget( "ZDATEL" ) ;
	offset  = 0 ;
	pmonth  = ds2d( substr( ZDATEL, 4, 2 ) ) ;
	pyear   = ds2d( substr( ZDATEL, 7, 4 ) ) ;
	ZCMD    = "" ;
	MSG     = "" ;
	ZSHADOW = "" ;
	create_calendar( pmonth, pyear ) ;

//	ZPUSER = "/home/daniel/.lspf/" ;
//	libdef( "ZPUSER", "FILE" ) ;
//
//	ZMUSER = "/home/daniel/.lspf/" ;
//	libdef( "ZMUSER", "FILE" ) ;

	control( "ERRORS","RETURN" ) ;
	vput( "ZTIME", SHARED ) ;
	debug1( "VPUT of system variable ZTIME.  RC is " << RC << endl ) ;
	control( "ERRORS","CANCEL" ) ;

	while ( true )
        {
		display( "PMAINP01", MSG, "ZCMD" );
		if ( RC  > 8 ) { abend() ; break   ; }
		if ( RC == 8 ) { break   ;           }

		vget( "ZJDATE ZTIME", SHARED ) ;
		MSG  = "" ;

		ZAREA  = ZAREA.replace( 204, 5, ZTIME ) ;

		w1 = word( ZCMD, 1 ) ;
		ws = subword( ZCMD, 2 ) ;

		if ( ZCURFLD == "ZAREA")
		{
			if ( ZCURPOS == 1 )
			{
				offset = offset - 1 ;
				create_calendar( pmonth, pyear ) ;
			}
			else
			{
				if ( ZCURPOS == 20 )
				{
					offset = offset + 1;
					create_calendar( pmonth, pyear )  ;
				}
				else
				{
					if ( ZCURPOS > 1 && ZCURPOS < 20 )
					{
						offset = 0 ;
						pmonth = ds2d( substr( ZDATEL, 4, 2 ) ) ;
						pyear  = ds2d( substr( ZDATEL, 7, 4 ) ) ;
						create_calendar( pmonth, pyear ) ;
					}
				}
			}
		}
		if ( w1 == "DATE")
		{
			if ( ws == "" )
			{
				MSG = "MAIN015" ;
				continue        ;
			}
			else
			{
				pmonth = ds2d( substr( ws, 1, 2 ) ) ;
				pyear  = ds2d( substr( ws, 4, 4 ) ) ;
				if ( pmonth < 1 || pmonth > 12 || pyear < 1900 || pyear > 9999 )
				{
					MSG = "MAIN015" ;
					continue ;
				}
				offset = 0 ;
				create_calendar( pmonth, pyear ) ;
			}
			ZCMD = "" ;
		}

		if ( ZCMD == "" ) { continue ; }

		command = get_select_cmd( ZCMD ) ;
		if ( command == "" )
		{
			MSG = "PSYS016" ;
			continue         ;
		}

		if ( command == "END" || command == "EXIT" ) break ;

		w1 = word( command, 1 ) ;
		ws = subword( command, 2 ) ;
		if ( w1 == "SELECT" )
		{
			selectParse( RC, ws, S_PGM, S_PARM, S_NEWAPPL, S_NEWPOOL, S_PASSLIB) ;
			if ( RC > 0 )
			{
				log( "E", "Select command " << ws << " is invalid.  RC > 0 returned from parse" << endl ) ;
				MSG = "PSYS017" ;
				continue        ;
			}
			p1 = wordpos( "&ZPARM", S_PARM ) ;
			if ( p1 > 0 )
			{
				p1     = wordindex( S_PARM, p1 )    ;
				S_PARM = delstr( S_PARM, p1, 6 )    ;
				S_PARM = insert( ZCMD, S_PARM, p1 ) ;
			}
			if ( substr( S_PGM, 1, 1 ) == "&" )
			{
				vcopy( substr( S_PGM, 2 ), S_PGM, MOVE ) ;
			}
			select( S_PGM, S_PARM, S_NEWAPPL, S_NEWPOOL, S_PASSLIB ) ;
			ZCMD = "" ;
			if ( RC > 4 )
			{
				log( "E", "Select command " << command << " is invalid.  RC > 4 returned from select" << endl ) ;
				MSG = "PSYS017" ;
				continue        ;
			}
		}
		else if ( w1 == "ACTION" )
		{
			if ( substr( ws, 1, 4 ) == "RUN(" )
			{
				p1   = pos( ")", ws, 5 ) ;
				ZCMD = substr( ws, 5, p1-5 ) ;
				control( "DISPLAY", "LOCK" ) ;
				continue ;
			}
			else
			{
				log( "E", ws << " in ACTION statement of panel PMAINP01 is invalid" << endl ) ;
				MSG = "PSYS017" ;
				continue        ;
			}
		}
		else
		{
			log( "E", w1 << " function of panel PMAINP01 is invalid" << endl ) ;
			MSG = "PSYS017" ;
		}
		if ( ZCMD != "" ) { MSG = "MAIN011" ; }
        }
	cleanup() ;
}


void PMAIN0A::create_calendar( int pmonth, int pyear )
{
	string m;
	int    year, month ;
	int    cday, cmonth, cyear ;
	int    i, eom_day, daypos  ;

	vget( "ZJDATE ZTIME ZDATEL" ) ;
	cday   = ds2d( substr( ZDATEL, 1, 2 ) ) ;
	cmonth = ds2d( substr( ZDATEL, 4, 2 ) ) ;
	cyear  = ds2d( substr( ZDATEL, 7, 4 ) ) ;

	daypos  = 0 ;
	year    = pyear  + (offset / 12) ;
	month   = pmonth + (offset % 12) ;
	if ( month > 12 ) { month = month - 12 ; year++ ; }
	if ( month < 1  ) { month = month + 12 ; year-- ; }

	eom_day = gregorian_calendar::end_of_month_day(year,month);
	date endOfMonth( year, month, eom_day ) ;
	day_iterator ditr( date(year, month, 1 ) ) ;

	switch ( month )
	{
	case  1:    m = "January  " ; break ;
	case  2:    m = "February " ; break ;
	case  3:    m = "March    " ; break ;
	case  4:    m = "April    " ; break ;
	case  5:    m = "May      " ; break ;
	case  6:    m = "June     " ; break ;
	case  7:    m = "July     " ; break ;
	case  8:    m = "August   " ; break ;
	case  9:    m = "September" ; break ;
	case 10:    m = "October  " ; break ;
	case 11:    m = "November " ; break ;
	case 12:    m = "December " ; break ;
	}

	ZAREA  =         "<     Calendar     > " ;
	ZAREA  = ZAREA + centre( m + "  " + d2ds( year ), 21 );
	ZAREA  = ZAREA + "Su Mo Tu We Th Fr Sa " ;

	switch ( ditr->day_of_week() )
	{
	case 0: break ;
	case 1: ZAREA = ZAREA + "   " ; break ;
	case 2: ZAREA = ZAREA + "      " ; break ;
	case 3: ZAREA = ZAREA + "         " ; break ;
	case 4: ZAREA = ZAREA + "            " ; break ;
	case 5: ZAREA = ZAREA + "               " ; break ;
	case 6: ZAREA = ZAREA + "                  " ; break ;
	}

	i = 1 ;
	for ( ; ditr <= endOfMonth ; ++ditr )
	{
		if ( i == cday & month == cmonth & year == cyear ) { daypos = ZAREA.size() ; }
		ZAREA = ZAREA + centre( d2ds( i ), 3 ) ;
		i++ ;
	}

	ZAREA  = substr( ZAREA, 1, 189 ) ;
	ZAREA  = ZAREA + left( "Time . . . . : " + ZTIME, 21 ) ;
	ZAREA  = ZAREA + left( "Day of Year. : " + substr( ZJDATE, 4, 3 ), 21 ) ;

	ZSHADOW.replace(   0, 231, 231, N_WHITE ) ;
	ZSHADOW.replace(  21,  21,  21, B_RED   ) ;

	ZSHADOW.replace(  42,  21,  21, B_YELLOW ) ;

	ZSHADOW.replace(  63,   2,   2, N_TURQ ) ;
	ZSHADOW.replace(  81,   2,   2, N_TURQ ) ;

	ZSHADOW.replace(  84,   2,   2, N_TURQ ) ;
	ZSHADOW.replace( 102,   2,   2, N_TURQ ) ;

	ZSHADOW.replace( 105,   2,   2, N_TURQ ) ;
	ZSHADOW.replace( 123,   2,   2, N_TURQ ) ;

	ZSHADOW.replace( 126,   2,   2, N_TURQ ) ;
	ZSHADOW.replace( 144,   2,   2, N_TURQ ) ;

	ZSHADOW.replace( 147,   2,   2, N_TURQ ) ;
	ZSHADOW.replace( 165,   2,   2, N_TURQ ) ;

	ZSHADOW.replace( 168,   2,   2, N_TURQ ) ;
	ZSHADOW.replace( 186,   2,   2, N_TURQ ) ;

	if ( daypos > 0 ) ZSHADOW.replace( daypos,   2,   2, R_TURQ ) ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PMAIN0A ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }

