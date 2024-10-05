/*  Compile with ::                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libpdlgtst.so -o libpdlgtst.so pdlgtst.cpp */
/*                                                                                     */

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
/* Some dialogue utilities                                                  */
/* Display panel                                                            */
/* Display SYSTEM variables                                                 */
/* Check pattern matching algorithm                                         */
/* Display/check/preprocess panels (not yet implemented)                    */
/*                                                                          */
/****************************************************************************/


#include <iostream>
#include "../lspfall.h"
#include "pdlgtst.h"

using namespace std ;

LSPF_APP_MAKER( pdlgtst )


pdlgtst::pdlgtst()
{
	STANDARD_HEADER( "Dialogue testing application for lspf", "1.0.1" )
}


void pdlgtst::application()
{
	vdefine( "ZCMD", &zcmd ) ;

	if      ( PARM == "0"  ) { displayPanel() ; }
	else if ( PARM == "1"  ) { displaySystemVars() ; }
}


void pdlgtst::displayPanel()
{
	string name ;
	string inpopup ;

	vdefine( "NAME INPOPUP", &name, &inpopup ) ;
	vget( "NAME", PROFILE ) ;

	zcmd = "" ;

	while ( true )
	{
		display( "PDLGTST2" );
		if ( RC == 8 )
		{
			break ;
		}
		vput ( "NAME", PROFILE ) ;
		if ( name == "" ) continue ;
		if ( inpopup == "/" )
		{
			addpop( "", 5, 5 ) ;
		}
		while ( true )
		{
			display( upper( name ) ) ;
			zcmd = "" ;
			if ( RC == 8 )
			{
				break ;
			}
		}
		if ( inpopup == "/" )
		{
			rempop() ;
		}
	}

	vdelete( "NAME" ) ;
}


void pdlgtst::displaySystemVars()
{
	while ( RC == 0 )
	{
		display( "PDLGTST3" ) ;
	}
}
