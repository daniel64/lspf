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
#include "../classes.h"
#include "../pVPOOL.h"
#include "../pWidgets.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"
#include "PMAIN0A.h"

using namespace std ;
using namespace boost::gregorian;

#undef  MOD_NAME
#define MOD_NAME PMAIN0A

#define E_RED      1
#define E_GREEN    2
#define E_YELLOW   3
#define E_BLUE     4
#define E_MAGENTA  5
#define E_TURQ     6
#define E_WHITE    7

#define G_TURQ     8

PMAIN0A::PMAIN0A()
{
	set_appdesc( "Default main program for lspf" ) ;
	set_appver( "1.0.0" )  ;
	set_apphelp( "HPMAIN1" ) ;

	vdefine( "ZCURFLD", &zcurfld ) ;
	vdefine( "ZCURPOS", &zcurpos ) ;
}


void PMAIN0A::application()
{
	llog( "I", "Application PMAIN0A starting.  Displaying panel PMAINP01" << endl ) ;

	int RC1    ;
	int pmonth ;
	int pyear  ;

	string zcmd ;
	string pan  ;
	string msg  ;
	string w1   ;
	string ws   ;
	string zsel ;

	string zscrname ;

	errblock err ;

	vdefine( "ZCMD ZSEL ZDATEL ZJDATE ZTIME ZSCRNAME", &zcmd, &zsel, &zdatel, &zjdate, &ztime, &zscrname ) ;
	vdefine( "ZAREA ZSHADOW", &zarea, &zshadow ) ;

	zscrname = "MAIN" ;
	vput( "ZSCRNAME", SHARED ) ;
	vget( "ZDATEL" ) ;

	offset = 0 ;
	pmonth = ds2d( substr( zdatel, 4, 2 ) ) ;
	pyear  = ds2d( substr( zdatel, 7, 4 ) ) ;
	zcmd   = PARM ;
	msg    = ""   ;
	create_calendar( pmonth, pyear ) ;

	if ( zcmd != "" ) { control( "DISPLAY", "NONDISPL" ) ; }
	vcopy( "ZMAINPAN", pan, MOVE ) ;

	while ( true )
	{
		vput( "ZCMD ZSEL", SHARED ) ;
		display( pan, msg, "ZCMD" ) ;
		RC1 = RC ;
		vget( "ZCMD ZSEL", SHARED ) ;
		if ( zsel == "EXIT" || RC1 == 8 )
		{
			zsel = "" ;
			vput( "ZSEL", SHARED ) ;
			break ;
		}
		if ( zsel == "?" )
		{
			msg  = "PSYS016" ;
			zsel = "" ;
			continue  ;
		}

		vget( "ZJDATE ZTIME", SHARED ) ;
		msg = "" ;

		zarea = zarea.replace( 204, 5, ztime ) ;

		w1 = word( zcmd, 1 ) ;
		ws = subword( zcmd, 2 ) ;

		if ( zcurfld == "ZAREA")
		{
			if ( zcurpos == 1 )
			{
				--offset ;
				create_calendar( pmonth, pyear ) ;
			}
			else if ( zcurpos == 20 )
			{
				++offset ;
				create_calendar( pmonth, pyear )  ;
			}
			else if ( zcurpos > 1 && zcurpos < 20 )
			{
				offset = 0 ;
				pmonth = ds2d( substr( zdatel, 4, 2 ) ) ;
				pyear  = ds2d( substr( zdatel, 7, 4 ) ) ;
				create_calendar( pmonth, pyear ) ;
			}
		}
		if ( w1 == "DATE")
		{
			pmonth = ds2d( substr( ws, 1, 2 ) ) ;
			pyear  = ds2d( substr( ws, 4, 4 ) ) ;
			if ( pmonth < 1 || pmonth > 12 || pyear < 1900 || pyear > 9999 )
			{
				msg = "MAIN011" ;
				continue ;
			}
			offset = 0 ;
			create_calendar( pmonth, pyear ) ;
		}
		zcmd = "" ;
	}
}


void PMAIN0A::create_calendar( int pmonth, int pyear )
{
	// Build the dynamic area variables for the calendar.

	int i      ;
	int year   ;
	int month  ;
	int cday   ;
	int cmonth ;
	int cyear  ;
	int eom_day ;
	int daypos  ;

	string  m ;

	vget( "ZJDATE ZTIME ZDATEL" ) ;
	cday   = ds2d( substr( zdatel, 1, 2 ) ) ;
	cmonth = ds2d( substr( zdatel, 4, 2 ) ) ;
	cyear  = ds2d( substr( zdatel, 7, 4 ) ) ;

	daypos = 0 ;
	year   = pyear  + (offset / 12) ;
	month  = pmonth + (offset % 12) ;
	if ( month > 12 ) { month -= 12 ; year++ ; }
	if ( month < 1  ) { month += 12 ; year-- ; }

	eom_day = gregorian_calendar::end_of_month_day( year,month ) ;
	date endOfMonth( year, month, eom_day ) ;
	day_iterator ditr( date(year, month, 1 ) ) ;

	switch ( month )
	{
		case  1: m = "January  " ; break ;
		case  2: m = "February " ; break ;
		case  3: m = "March    " ; break ;
		case  4: m = "April    " ; break ;
		case  5: m = "May      " ; break ;
		case  6: m = "June     " ; break ;
		case  7: m = "July     " ; break ;
		case  8: m = "August   " ; break ;
		case  9: m = "September" ; break ;
		case 10: m = "October  " ; break ;
		case 11: m = "November " ; break ;
		case 12: m = "December " ; break ;
	}

	zarea  = "<     Calendar     > "               ;
	zarea += centre( m + "  " + d2ds( year ), 21 ) ;
	zarea += "Su Mo Tu We Th Fr Sa "               ;
	zarea += string( 3*ditr->day_of_week(), ' ' )  ;

	i = 1 ;
	for ( ; ditr <= endOfMonth ; ++ditr )
	{
		if (     i == cday   &&
		     month == cmonth &&
		      year == cyear ) { daypos = zarea.size() ; }
		zarea += centre( d2ds( i ), 3 ) ;
		i++ ;
	}

	zarea.resize( 189, ' ' ) ;
	zarea += left( "Time . . . . : " + ztime, 21 ) ;
	zarea += left( "Day of Year. : " + substr( zjdate, 4, 3 ), 21 ) ;

	zshadow = string( 231, E_WHITE ) ;

	zshadow.replace(  21,  21,  21, E_RED   )  ;
	zshadow.replace(  42,  21,  21, E_YELLOW ) ;

	zshadow.replace(  63,   2,   2, E_TURQ ) ;
	zshadow.replace(  81,   2,   2, E_TURQ ) ;

	zshadow.replace(  84,   2,   2, E_TURQ ) ;
	zshadow.replace( 102,   2,   2, E_TURQ ) ;

	zshadow.replace( 105,   2,   2, E_TURQ ) ;
	zshadow.replace( 123,   2,   2, E_TURQ ) ;

	zshadow.replace( 126,   2,   2, E_TURQ ) ;
	zshadow.replace( 144,   2,   2, E_TURQ ) ;

	zshadow.replace( 147,   2,   2, E_TURQ ) ;
	zshadow.replace( 165,   2,   2, E_TURQ ) ;

	zshadow.replace( 168,   2,   2, E_TURQ ) ;
	zshadow.replace( 186,   2,   2, E_TURQ ) ;

	if ( daypos > 0 ) { zshadow.replace( daypos, 2, 2, G_TURQ ) ; }
}


// ============================================================================================ //

extern "C" { pApplication* maker() { return new PMAIN0A ; } }
extern "C" { void destroy(pApplication* p) { delete p ; } }

