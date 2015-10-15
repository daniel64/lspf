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
	int p1     ;

	string S_PGM       ;
	string S_PARM      ;
	string S_NEWAPPL   ;
	bool   S_NEWPOOL   ;
	bool   S_PASSLIB   ;

	PANELNM  = word( PARM, 1 ) ;
	PARM     = subword( PARM, 2 ) ;
	log( "I", "SELECT PANEL " << PANELNM << " with parameters " << PARM << endl ) ;

	vdefine( "ZCMD ZVERB", &ZCMD, &ZVERB ) ;

	ZCMD = "" ; MSG = "" ;
        while ( true )
        {
		display( PANELNM, MSG, "ZCMD" );
		if ( RC == 8 ) break ;
		if ( RC  > 8 ) { abend() ; return ; }

		MSG  = "" ;
		vget( "ZVERB", SHARED ) ;

		if ( ZCMD == "" ) continue   ;

		command = get_select_cmd( ZCMD ) ;
		if ( command == "" )
		{
			MSG = "PSYS016" ;
			continue         ;
		}

		if ( command == "ACTION END" ) break ;

		w1 = word( command, 1 ) ;
		ws = subword( command, 2 ) ;
		if ( w1 == "SELECT" )
		{
			selectParse( RC, ws, S_PGM, S_PARM, S_NEWAPPL, S_NEWPOOL, S_PASSLIB) ;
			if ( RC > 0 )
			{
				log( "E", "Select command " << ws << " is invalid.  RC > 0 returned from parse" << endl ) ;
				MSG = "PSYS017" ;
				continue        ;
			}
			p1 = wordpos( "&ZPARM", S_PARM ) ;
			if ( p1 > 0 )
			{
				p1     = wordindex( S_PARM, p1 )   ;
				S_PARM = delstr( S_PARM, p1, 6 )   ;
				S_PARM = insert( ZCMD, S_PARM, p1 ) ;
			}
			if ( substr( S_PGM, 1, 1 ) == "&" )
			{
				vcopy( substr( S_PGM, 2 ), S_PGM, MOVE ) ;
			}
			select( S_PGM, S_PARM, S_NEWAPPL, S_NEWPOOL, S_PASSLIB ) ;
			if ( RC  > 4 )
			{
				log( "E", "Select command " << command << " is invalid.  RC > 4 returned from select" << endl ) ;
				MSG = "PSYS017" ;
			}
			ZCMD = "" ;
		}
		else
		{
			if ( w1 == "ACTION" )
			{
				log( "N", "ACTION function of select panels has not been implemented yet" << endl ) ;
				MSG = "PSYS015" ;
			}
			else
			{
				log( "E", w1 << " function of select panel " << PANELNM << " is invalid" << endl ) ;
				MSG = "PSYS017" ;
			}
		}
        }
        cleanup() ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PDPANLA ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
