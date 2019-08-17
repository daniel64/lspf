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
using namespace boost::gregorian ;

#undef  MOD_NAME
#define MOD_NAME PMAIN0A

#define E_RED      0x01
#define E_GREEN    0x02
#define E_YELLOW   0x03
#define E_BLUE     0x04
#define E_MAGENTA  0x05
#define E_TURQ     0x06
#define E_WHITE    0x07

#define G_TURQ     0x08

PMAIN0A::PMAIN0A()
{
	set_appdesc( "Default main program for lspf" ) ;
	set_appver( "1.0.2" ) ;
	set_apphelp( "HPMAIN1" ) ;

	vdefine( "ZCURFLD ZAREA ZSHADOW", &zcurfld, &zarea, &zshadow ) ;
	vdefine( "ZDATEL ZJDATE ZTIME", &zdatel, &zjdate, &ztime ) ;
	vdefine( "ZCURPOS", &zcurpos ) ;
}


void PMAIN0A::application()
{
	int pmonth ;
	int pyear  ;
	int curpos ;
	int maxw   ;

	string zcmd ;
	string pan  ;
	string w1   ;
	string ws   ;
	string zsel ;
	string odat ;

	string zscrname ;
	string curfld   ;

	vdefine( "ZCMD ZSEL ZSCRNAME", &zcmd, &zsel, &zscrname ) ;
	vdefine( "ZSCRMAXW", &maxw ) ;

	zscrname = "MAIN" ;
	vput( "ZSCRNAME", SHARED ) ;
	vget( "ZDATEL ZJDATE", SHARED ) ;
	vget( "ZSCRMAXW", SHARED ) ;

	offset = 0 ;
	pmonth = ds2d( substr( zdatel, 4, 2 ) ) ;
	pyear  = ds2d( substr( zdatel, 7, 4 ) ) ;
	odat   = zjdate ;
	zcmd   = PARM ;
	msg    = ""   ;

	create_calendar( pmonth, pyear ) ;

	if ( zcmd != "" )
	{
		control( "NONDISPL", "ENTER" ) ;
	}

	if ( maxw < 123 )
	{
		pan = "PMAINP02" ;
	}
	else
	{
		vcopy( "ZMAINPAN", pan, MOVE ) ;
	}

	curfld = "ZCMD" ;
	curpos = 1      ;
	verase( "ZSEL", SHARED ) ;
	while ( true )
	{
		vput( "ZCMD", SHARED ) ;
		display( pan, msg, curfld, curpos ) ;
		if ( RC == 8 )
		{
			zsel = "" ;
			vput( "ZSEL", SHARED ) ;
			break ;
		}
		vget( "ZSEL", SHARED ) ;
		if ( RC > 0 )
		{
			abend( "PSYS013M", pan ) ;
			return ;
		}
		curpos = 1      ;
		curfld = "ZCMD" ;
		vget( "ZCMD ZJDATE ZTIME", SHARED ) ;
		if ( zsel == "?" )
		{
			msg  = "PSYS016" ;
			zsel = "" ;
			vput( "ZSEL", SHARED ) ;
			continue  ;
		}

		vget( "ZCMD ZJDATE ZTIME", SHARED ) ;
		msg = "" ;

		if ( odat != zjdate )
		{
			create_calendar( pmonth, pyear ) ;
			odat = zjdate ;
		}
		else
		{
			zarea.replace( 204, 5, ztime ) ;
		}

		w1 = word( zcmd, 1 ) ;
		ws = subword( zcmd, 2 ) ;

		if ( zcurfld == "ZAREA")
		{
			if ( zcurpos == 1 )
			{
				--offset ;
				create_calendar( pmonth, pyear ) ;
				curpos = 1 ;
				curfld = "ZAREA" ;
			}
			else if ( zcurpos == 20 )
			{
				++offset ;
				create_calendar( pmonth, pyear )  ;
				curpos = 20 ;
				curfld = "ZAREA" ;
			}
			else if ( zcurpos > 1 && zcurpos < 20 )
			{
				offset = 0 ;
				pmonth = ds2d( substr( zdatel, 4, 2 ) ) ;
				pyear  = ds2d( substr( zdatel, 7, 4 ) ) ;
				create_calendar( pmonth, pyear ) ;
			}
		}
		else if ( w1 == "DATE" )
		{
			offset = 0  ;
			pmonth = ds2d( substr( ws, 1, 2 ) ) ;
			pyear  = ds2d( substr( ws, 4, 4 ) ) ;
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
	int daypos ;

	string m ;

	vget( "ZJDATE ZTIME ZDATEL" ) ;
	cday   = ds2d( substr( zdatel, 1, 2 ) ) ;
	cmonth = ds2d( substr( zdatel, 4, 2 ) ) ;
	cyear  = ds2d( substr( zdatel, 7, 4 ) ) ;

	daypos = 0 ;
	year   = pyear  + (offset / 12) ;
	month  = pmonth + (offset % 12) ;
	if ( month > 12 ) { month -= 12 ; ++year ; }
	if ( month < 1  ) { month += 12 ; --year ; }

	try
	{
		eom_day = gregorian_calendar::end_of_month_day( year, month ) ;
	}
	catch ( std::exception& e )
	{
		msg = "MAIN011" ;
		return ;
	}

	date endOfMonth( year, month, eom_day ) ;
	day_iterator ditr( date( year, month, 1 ) ) ;

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

	for ( i = 1 ; ditr <= endOfMonth ; ++ditr, ++i )
	{
		if (     i == cday   &&
		     month == cmonth &&
		      year == cyear ) { daypos = zarea.size() ; }
		zarea += centre( d2ds( i ), 3 ) ;
	}

	zarea.resize( 189, ' ' ) ;
	zarea += left( "Time . . . . : " + ztime, 21 ) ;
	zarea += left( "Day of Year. :   " + substr( zjdate, 4, 3 ), 21 ) ;

	zshadow = string( 231, E_WHITE ) ;

	zshadow.replace(  21, 21, 21, E_RED )  ;
	zshadow.replace(  42, 21, 21, E_YELLOW ) ;

	zshadow.replace(  63,  2,  2, E_TURQ ) ;
	zshadow.replace(  81,  2,  2, E_TURQ ) ;

	zshadow.replace(  84,  2,  2, E_TURQ ) ;
	zshadow.replace( 102,  2,  2, E_TURQ ) ;

	zshadow.replace( 105,  2,  2, E_TURQ ) ;
	zshadow.replace( 123,  2,  2, E_TURQ ) ;

	zshadow.replace( 126,  2,  2, E_TURQ ) ;
	zshadow.replace( 144,  2,  2, E_TURQ ) ;

	zshadow.replace( 147,  2,  2, E_TURQ ) ;
	zshadow.replace( 165,  2,  2, E_TURQ ) ;

	zshadow.replace( 168,  2,  2, E_TURQ ) ;
	zshadow.replace( 186,  2,  2, E_TURQ ) ;

	if ( daypos > 0 ) { zshadow.replace( daypos, 2, 2, G_TURQ ) ; }
}


// ============================================================================================ //

extern "C" { pApplication* maker() { return new PMAIN0A ; } }
extern "C" { void destroy(pApplication* p) { delete p ; } }

