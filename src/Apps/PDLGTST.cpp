/*  Compile with ::                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPDLGTST.so -o libPDLGTST.so PDLGTST.cpp */
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

/* Some dialogue utilities                                                  */
/* Display panel                                                            */
/* Display SYSTEM variables                                                 */
/* Check pattern matching algorithm                                         */
/* Display/check/preprocess panels (not yet implemented)                    */


#include <iostream>
#include "../lspf.h"
#include "../utilities.h"
#include "../classes.h"
#include "../pVPOOL.h"
#include "../pWidgets.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"
#include "PDLGTST.h"

using namespace std ;

#undef  MOD_NAME
#define MOD_NAME PDLGTST


PDLGTST::PDLGTST()
{
	set_appdesc( "Dialogue testing application for lspf" ) ;
	set_appver( "1.0.0" )  ;
}


void PDLGTST::application()
{
	vdefine( "ZCMD", &zcmd ) ;

	if      ( PARM == "0"  ) { displayPanel() ; }
	else if ( PARM == "1"  ) { displaySystemVars() ; }
	else if ( PARM == "2"  ) { checkAlgo()  ; }
	else if ( PARM == "3"  ) { prepPanels() ; }

	return ;
}


void PDLGTST::displayPanel()
{
	string msg  ;
	string name ;

	vdefine( "NAME", &name ) ;
	vget( "NAME", PROFILE ) ;
	zcmd = "" ;

	while ( true )
	{
		display( "PDLGTST2", msg, "ZCMD" );
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
		msg  = "" ;
		if ( zcmd != "" ) { msg = "DLGT011" ; continue ; }
		if ( name == "" ) continue ;
		while ( true )
		{
			display( name );
			if (RC > 8 )
			{
				debug1( "Displaying panel " << name << " gave an error" << endl ) ;
				msg = "DLGT012" ;
				break ;
			}
			if ( RC == 8 )
			{
				break ;
			}
			msg  = "" ;
			zcmd = "" ;
		}
	}
}


void PDLGTST::displaySystemVars()
{
	string msg ;

	zcmd = "" ;
	msg  = "" ;

	while ( true )
	{
		display( "PDLGTST3", msg, "ZCMD" );
		if ( RC == 8 ) { return ; }
		msg = "" ;
	}
}


void PDLGTST::checkAlgo()
{
}


void PDLGTST::prepPanels()
{
	llog( "N", "Preprocessing and checking of panels not yet implemented" << endl ) ;
}


// ============================================================================================ //

extern "C" { pApplication* maker() { return new PDLGTST ; } }
extern "C" { void destroy(pApplication* p) { delete p ; } }
