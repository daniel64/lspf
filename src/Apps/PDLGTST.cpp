/*  Compile with ::                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPDLGTST.so -o libPDLGTST.so PDLGTST.cpp */
/*                                                                          */

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

/* Some dialogue utilities                                                  */
/* Display panel                                                            */
/* Display SYSTEM variables                                                 */
/* Check pattern matching algorithm                                         */
/* Display/check/preprocess panels                                          */


#include <iostream>
#include "../lspf.h"
#include "../utilities.h"
#include "../pWidgets.h"
#include "../pVPOOL.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"
#include "PDLGTST.h"

using namespace std ;

#undef LOGOUT
#undef MOD_NAME

#define LOGOUT aplog
#define MOD_NAME PDLGTST


void PDLGTST::application()
{
	log( "I", "Application PDLGTST starting." << endl ) ;

	vdefine( "ZCMD", &ZCMD ) ;

	if ( PARM == "0"  ) displayPanel() ;
	if ( PARM == "1"  ) displaySystemVars() ;
	if ( PARM == "2"  ) checkAlgo() ;
	if ( PARM == "3"  ) prepPanels() ;

	cleanup() ;
	return    ;
}


void PDLGTST::displayPanel()
{
	string MSG, NAME ;

	vdefine( "NAME", &NAME ) ;
	vget( "NAME", PROFILE ) ;
	ZCMD = "" ;

	while ( true )
	{
		display( "PDLGTST2", MSG, "ZCMD" );
		if (RC > 8 )
		{
			debug1( "Displaying panel PSTEST02 gave error.  Terminating application" << endl ) ;
			return ;
		}
		if ( RC == 8 )
		{
			return    ;
		}
		vput ( "NAME", PROFILE ) ;
		MSG  = "" ;
		if ( ZCMD != "" ) { MSG = "DLGT011" ; continue ; }
		if ( NAME == "" ) continue ;
		while ( true )
		{
			display( NAME, "", "ZCMD" );
			if (RC > 8 )
			{
				debug1( "Displaying panel " << NAME << " gave an error" << endl ) ;
				MSG = "DLGT012" ;
				break ;
			}
			if ( RC == 8 )
			{
				break ;
			}
			MSG  = "" ;
			ZCMD = "" ;
		}
	}
}


void PDLGTST::displaySystemVars()
{
	string MSG ;
  
	ZCMD = "" ;
	MSG  = "" ;
	
	while ( true )
	{
		display( "PDLGTST3", MSG, "ZCMD" );
		if ( RC  > 8 ) { abend() ; return ; }
		if ( RC == 8 ) { return  ; }
		
		MSG  = "" ;
		if ( ZCMD != "" ) { MSG = "DLGT011" ; continue ; }
	}
	return ;
}


void PDLGTST::checkAlgo()
{
	string MSG, STR1, STR2, RESLT ;

	vdefine( "STR1 STR2 RESLT", &STR1, &STR2, &RESLT ) ;
	vget( "STR1 STR2", PROFILE ) ;
	ZCMD = "" ;

	while ( true )
	{
		display( "PDLGTST4", MSG, "STR1" );

		if ( RC  > 8 ) { abend() ; return ; }
		if ( RC == 8 ) 
		{
			vput( "STR1 STR2", PROFILE ) ;
			return    ;
		}
		MSG  = "" ;
		STR1 = strip( STR1 ) ;
		STR2 = strip( STR2 ) ;
		if ( ZCMD != "" ) { MSG = "DLGT011" ; continue ; }
		if ( STR1 == "" ) continue ;
		if ( STR2 == "" ) continue ;
		
		if ( matchpattern( STR1, STR2 ) )
		{
			RESLT = "*PATTERN MATCHES*" ;
		}
		else
		{
			RESLT = "*PATTERN DOES NOT MATCH*" ;
		}
		
	}

}


void PDLGTST::prepPanels()
{
	log( "N", "Preprocessing and checking of panels not yet implemented" << endl ) ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PDLGTST ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
