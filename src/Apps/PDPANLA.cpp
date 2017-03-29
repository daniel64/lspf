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


/* Invoked as part of the SELECT PANEL(xxxx) service.  Panel name is        */
/* passed as word 1 of parameter PARM.  Normal parameters are the rest.     */
/* Program automatically executes the command defined in the )COMMAND       */
/* section of the panel definition using ZCMD as the command identifier     */

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
	string w1  ;
	string ws  ;
	string opt ;
	int p1     ;

	string ZTRAIL ;

	bool   loope  ;

	selobj SEL    ;
	errblock err  ;


	PANELNM  = word( PARM, 1 ) ;
	opt      = subword( PARM, 2 ) ;
	command  = ""                 ;
	llog( "I", "SELECT PANEL " << PANELNM << " with parameters " << PARM << endl ) ;

	vdefine( "ZCMD ZVERB", &ZCMD, &ZVERB ) ;

	ZCMD  = "" ;
	MSG   = "" ;
	loope = false ;

	while ( true )
	{
		if ( opt != "" )
		{
			ZCMD = opt ;
			control( "DISPLAY", "NONDISPL" ) ;
			opt   = ""   ;
			loope = true ;
		}
		display( PANELNM, MSG, "ZCMD" );
		if ( RC == 8 ) break ;
		if ( RC  > 8 ) { abend() ; return ; }

		if ( MSG != "" ) { loope = false ; }
		MSG  = "" ;
		vget( "ZVERB", SHARED ) ;

		if ( ZCMD == "" ) continue   ;

		command = get_select_cmd( ZCMD ) ;
		if ( command == "" )
		{
			MSG   = "PSYS016" ;
			continue          ;
		}

		w1 = word( command, 1 ) ;
		ws = subword( command, 2 ) ;
		vcopy( "ZTRAIL", ZTRAIL, MOVE ) ;
		if ( w1 == "SELECT" )
		{
			if ( !SEL.parse( err, ws ) )
			{
				llog( "E", "Select command " << ws << " is invalid.  RC > 0 returned from parse" << endl ) ;
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
			if ( RC  > 4 )
			{
				llog( "E", "Select command " << command << " is invalid.  RC > 4 returned from select" << endl ) ;
				MSG = "PSYS017" ;
			}
			ZCMD = "" ;
		}
		else
		{
			if ( w1 == "ACTION" )
			{
				if ( ws == "END" ) { break ; }
				llog( "N", "ACTION function of select panels has not been implemented yet" << endl ) ;
				MSG = "PSYS015" ;
			}
			else
			{
				llog( "E", w1 << " function of select panel " << PANELNM << " is invalid" << endl ) ;
				MSG = "PSYS017" ;
			}
		}
		if ( loope && MSG == "" ) { break ; }
		command = "" ;
	}
	cleanup() ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PDPANLA ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
