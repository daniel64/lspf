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

	selobj SEL ;

	string MSG ;
	string w1  ;
	string ws  ;
	string command ;
	string ZTRAIL  ;

	string ZSYSNAME ;
	string ZOSREL   ;
	string ZSCRNAME ;

	errblock err ;

	ZAHELP = "HPMAIN1" ;

	vdefine( "ZCMD ZDATEL ZJDATE ZTIME ZSCRNAME", &ZCMD, &ZDATEL, &ZJDATE, &ZTIME, &ZSCRNAME ) ;
	vdefine( "ZAREA ZSHADOW", &ZAREA, &ZSHADOW ) ;

	ZSCRNAME = "MAIN" ;
	ispexec( "VPUT (ZSCRNAME) SHARED" ) ;
	ispexec( "VGET ZDATEL" ) ;

	offset = 0 ;
	pmonth = ds2d( substr( ZDATEL, 4, 2 ) ) ;
	pyear  = ds2d( substr( ZDATEL, 7, 4 ) ) ;
	ZCMD   = PARM ;
	MSG    = ""   ;
	create_calendar( pmonth, pyear ) ;

	if ( ZCMD != "" ) { ispexec( "CONTROL DISPLAY NONDISPL" ) ; }

	while ( true )
	{
		ispexec( "DISPLAY PANEL(PMAINP01) MSG("+MSG+") CURSOR(ZCMD)" ) ;
		if ( RC  > 8 ) { abend() ; break   ; }
		if ( RC == 8 ) { break   ;           }

		ispexec( "VGET (ZJDATE ZTIME) SHARED" ) ;
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
				MSG = "MAIN011" ;
				continue        ;
			}
			else
			{
				pmonth = ds2d( substr( ws, 1, 2 ) ) ;
				pyear  = ds2d( substr( ws, 4, 4 ) ) ;
				if ( pmonth < 1 || pmonth > 12 || pyear < 1900 || pyear > 9999 )
				{
					MSG = "MAIN011" ;
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

		if ( findword( command, "END EXIT" ) ) { break ; }

		w1 = word( command, 1 ) ;
		ws = subword( command, 2 ) ;
		vcopy( "ZTRAIL", ZTRAIL, MOVE ) ;
		if ( w1 == "SELECT" )
		{
			if ( !SEL.parse( err, ws ) )
			{
				log( "E", "Select command " << ws << " is invalid.  RC > 0 returned from parse" << endl ) ;
				MSG = "PSYS017" ;
				continue        ;
			}
			p1 = SEL.PARM.find( "&ZPARM" ) ;
			if ( p1 != string::npos )
			{
				SEL.PARM.replace( p1, 6, ZCMD ) ;
			}
			if ( SEL.PGM == "&ZPANLPGM" && ZTRAIL != "" ) { SEL.PARM = SEL.PARM + " " + ZTRAIL ; }
			if ( SEL.PGM[ 0 ] == '&' )
			{
				vcopy( SEL.PGM.erase( 0, 1 ), SEL.PGM, MOVE ) ;
			}
			select( SEL ) ;
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
				ispexec( "CONTROL DISPLAY LOCK" ) ;
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
		if ( ZCMD != "" ) { MSG = "PSYS011C" ; }
	}
	cleanup() ;
}


void PMAIN0A::create_calendar( int pmonth, int pyear )
{
	string m;
	int year, month ;
	int cday, cmonth, cyear ;
	int i, eom_day, daypos  ;

	ispexec( "VGET (ZJDATE ZTIME ZDATEL)" ) ;
	cday   = ds2d( substr( ZDATEL, 1, 2 ) ) ;
	cmonth = ds2d( substr( ZDATEL, 4, 2 ) ) ;
	cyear  = ds2d( substr( ZDATEL, 7, 4 ) ) ;

	daypos = 0 ;
	year   = pyear  + (offset / 12) ;
	month  = pmonth + (offset % 12) ;
	if ( month > 12 ) { month = month - 12 ; year++ ; }
	if ( month < 1  ) { month = month + 12 ; year-- ; }

	eom_day = gregorian_calendar::end_of_month_day(year,month);
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

	ZAREA  = "<     Calendar     > "               ;
	ZAREA += centre( m + "  " + d2ds( year ), 21 ) ;
	ZAREA += "Su Mo Tu We Th Fr Sa "               ;
	ZAREA += string( 3*ditr->day_of_week(), ' ' )  ;

	i = 1 ;
	for ( ; ditr <= endOfMonth ; ++ditr )
	{
		if (     i == cday   &&
		     month == cmonth &&
		      year == cyear ) { daypos = ZAREA.size() ; }
		ZAREA += centre( d2ds( i ), 3 ) ;
		i++ ;
	}

	ZAREA.resize( 189, ' ' ) ;
	ZAREA += left( "Time . . . . : " + ZTIME, 21 ) ;
	ZAREA += left( "Day of Year. : " + substr( ZJDATE, 4, 3 ), 21 ) ;

	ZSHADOW = string( 231, N_WHITE ) ;

	ZSHADOW.replace(  21,  21,  21, B_RED   )  ;
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

	if ( daypos > 0 ) { ZSHADOW.replace( daypos, 2, 2, R_TURQ ) ; }
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PMAIN0A ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }

