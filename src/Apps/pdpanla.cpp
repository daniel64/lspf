/* Compile with ::                                                                     */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libpdpanla.so -o libpdpanla.so pdpanla.cpp */

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

/***********************************************************************************************/
/*                                                                                             */
/* Invoked as part of the SELECT PANEL(xxxx) service.  Panel name is                           */
/* passed as word 1 of parameter PARM and an optional selection as word 2.                     */
/*                                                                                             */
/* Note: Selection panels only use the shared and profile pools so this                        */
/*       is where all panel variables reside (eg. ZSEL, ZCMD, ZPRIM, etc).                     */
/*                                                                                             */
/* Don't passthru if the selection panel is defined as a primary panel.                        */
/*                                                                                             */
/* Terminate ZRC = 20 if passthru and invalid selection made.                                  */
/* This will cause the PSYS016 message to be displayed.                                        */
/*                                                                                             */
/***********************************************************************************************/

#include "../lspfall.h"
#include "pdpanla.h"

using namespace std ;

LSPF_APP_MAKER( pdpanla )


pdpanla::pdpanla()
{
	STANDARD_HEADER( "Default SELECT PANEL program for lspf", "1.0.3" )
}


void pdpanla::application()
{
	int p ;

	int i = 1 ;

	bool passthru = false ;
	bool addpopup = false ;

	string pan   ;
	string msg   ;
	string zcmd  ;
	string zsel  ;
	string zprim ;

	const string vlist1 = "ZCMD ZSEL ZPRIM" ;

	p = wordpos( "ADDPOP", PARM ) ;
	if ( p > 0 )
	{
		idelword( PARM, p, 1 ) ;
		addpopup = true ;
	}

	pan  = word( PARM, 1 ) ;
	zcmd = subword( PARM, 2 ) ;
	msg  = "" ;
	zsel = "" ;

	vdefine( vlist1, &zcmd, &zsel, &zprim ) ;

	if ( zcmd != "" )
	{
		control( "NONDISPL", "ENTER" ) ;
		passthru = true ;
		i        = 0 ;
	}

	if ( addpopup )
	{
		addpop() ;
	}

	verase( "ZSEL", SHARED ) ;
	while ( ++i )
	{
		vput( "ZCMD", SHARED ) ;
		display( pan, msg ) ;
		if ( RC == 8 )
		{
			zsel = "" ;
			zcmd = "" ;
			vput( "ZSEL ZCMD", SHARED ) ;
			break ;
		}
		msg = "" ;
		vget( "ZSEL", SHARED ) ;
		if ( RC > 0 )
		{
			abend( "PSYS013M", pan ) ;
		}
		vget( "ZCMD ZPRIM", SHARED ) ;
		if ( zsel == "?" )
		{
			if ( i == 1 && zprim != "YES" )
			{
				zsel = "" ;
				zcmd = "" ;
				ZRC  = 20 ;
				vput( "ZSEL ZCMD", SHARED ) ;
				break ;
			}
			msg  = "PSYS016" ;
			zsel = "" ;
			vput( "ZSEL", SHARED ) ;
			continue  ;
		}
		if ( zcmd == "" )
		{
			if ( zprim != "YES" ) { msg = "PSYS017" ; }
			continue ;
		}
		zcmd = "" ;
		if ( passthru && zprim != "YES" )
		{
			vput( "ZCMD", SHARED ) ;
			break ;
		}
	}

	if ( addpopup )
	{
		rempop() ;
	}

	vdelete( vlist1 ) ;
}
