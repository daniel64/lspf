/*  Compile with ::                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libpmain0a.so -o libpmain0a.so pmain0a.cpp */

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
#include "../lspfall.h"
#include "pmain0a.h"

using namespace std ;
using namespace boost::gregorian ;

#define N_RED      0x01
#define N_GREEN    0x02
#define N_YELLOW   0x03
#define N_BLUE     0x04
#define N_MAGENTA  0x05
#define N_TURQ     0x06
#define N_WHITE    0x07

#define G_TURQ     0x08

LSPF_APP_MAKER( pmain0a )


pmain0a::pmain0a()
{
	STANDARD_HEADER( "Default main program for lspf", "1.0.3" )

	vdefine( "ZCURFLD ZAREA ZSHADOW", &zcurfld, &zarea, &zshadow ) ;
	vdefine( "ZSTDYEAR ZMONTH ZDAY ZJDATE ZTIME", &zstdyear, &zmonth, &zday, &zjdate, &ztime ) ;
	vdefine( "ZCURPOS", &zcurpos ) ;
}


void pmain0a::application()
{
	int curpos ;
	int pmonth ;
	int pyear ;
	int maxw ;

	string zcmd ;
	string zsel ;
	string odat ;
	string pan ;
	string w1 ;
	string ws ;

	string curfld ;

	vdefine( "ZCMD ZSEL", &zcmd, &zsel ) ;
	vdefine( "ZSCRMAXW", &maxw ) ;

	vget( "ZSTDYEAR ZMONTH ZJDATE", SHARED ) ;
	vget( "ZSCRMAXW", SHARED ) ;

	offset = 0 ;
	pmonth = ds2d( zmonth ) ;
	pyear  = ds2d( zstdyear ) ;
	odat   = zjdate ;
	msg    = ""   ;
	if ( PARM != "" )
	{
		PARM = ">" + PARM ;
	}

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
		vget( "ZMAINPAN", PROFILE ) ;
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
				pmonth = ds2d( zmonth ) ;
				pyear  = ds2d( zstdyear ) ;
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


void pmain0a::create_calendar( int pmonth,
			       int pyear )
{
	//
	// Build the dynamic area variables for the calendar.
	//

	int i ;
	int year ;
	int month ;
	int cday ;
	int cmonth ;
	int cyear ;
	int eom_day ;
	int daypos = 0 ;

	string m ;

	vget( "ZJDATE ZTIME ZSTDYEAR ZMONTH ZDAY" ) ;
	cday   = ds2d( zday ) ;
	cmonth = ds2d( zmonth ) ;
	cyear  = ds2d( zstdyear ) ;

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

	zarea  = "<     Calendar     > " ;
	zarea += centre( m + " " + d2ds( year ), 21 ) ;
	zarea += "Su Mo Tu We Th Fr Sa " ;
	zarea += string( 3*ditr->day_of_week(), ' ' ) ;

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

	zshadow = string( 231, N_WHITE ) ;

	zshadow.replace(  21, 21, 21, N_RED )  ;
	zshadow.replace(  42, 21, 21, N_YELLOW ) ;

	zshadow.replace(  63,  2,  2, N_TURQ ) ;
	zshadow.replace(  81,  2,  2, N_TURQ ) ;

	zshadow.replace(  84,  2,  2, N_TURQ ) ;
	zshadow.replace( 102,  2,  2, N_TURQ ) ;

	zshadow.replace( 105,  2,  2, N_TURQ ) ;
	zshadow.replace( 123,  2,  2, N_TURQ ) ;

	zshadow.replace( 126,  2,  2, N_TURQ ) ;
	zshadow.replace( 144,  2,  2, N_TURQ ) ;

	zshadow.replace( 147,  2,  2, N_TURQ ) ;
	zshadow.replace( 165,  2,  2, N_TURQ ) ;

	zshadow.replace( 168,  2,  2, N_TURQ ) ;
	zshadow.replace( 186,  2,  2, N_TURQ ) ;

	if ( daypos > 0 ) { zshadow.replace( daypos, 2, 2, G_TURQ ) ; }
}
