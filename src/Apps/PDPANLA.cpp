/* Compile with ::                                                                     */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPDPANLA.so -o libPDPANLA.so PDPANLA.cpp */

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

/* Invoked as part of the SELECT PANEL(xxxx) service.  Panel name is          */
/* passed as word 1 of parameter PARM and an optional selection as word 2.    */

/* An invalid passed option will cause an error message but after a valid     */
/* option is entered, the panel will not be re-displayed but to work properly */
/* a null option has to display a message (PSYS012S) or the panel will        */
/* end.  passthru can be changed to false after mesage PSYS016 to avoid this. */

#include "../lspf.h"
#include "../utilities.h"
#include "../classes.h"
#include "../pWidgets.h"
#include "../pVPOOL.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"

#include "PDPANLA.h"

using namespace std ;

#undef LOGOUT
#undef MOD_NAME

#define LOGOUT aplog
#define MOD_NAME PDPANLA


void PDPANLA::application()
{
	int RC1     ;

	bool passthru = false ;

	string pan  ;
	string msg  ;
	string ZCMD ;
	string ZSEL ;

	pan  = word( PARM, 1 ) ;
	msg  = "" ;
	ZCMD = subword( PARM, 2 ) ;

	vdefine( "ZCMD", &ZCMD ) ;

	if ( ZCMD != "" )
	{
		control( "DISPLAY", "NONDISPL" ) ;
		passthru = true ;
	}

	while ( true )
	{
		display( pan, msg ) ;
		RC1 = RC ;
		msg = "" ;
		vcopy( "ZSEL", ZSEL, MOVE ) ;
		if ( ZSEL == "EXIT" || RC1 == 8 )
		{
			vreplace( "ZSEL", "" ) ;
			break ;
		}
		if ( ZSEL == "?" )
		{
			msg = "PSYS016" ;
			vreplace( "ZSEL", "" ) ;
			continue ;
		}
		if ( passthru ) { break ; }
		ZCMD = "" ;
	}

	vdelete( "ZCMD" ) ;
	cleanup() ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PDPANLA ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
