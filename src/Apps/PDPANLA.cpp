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

/* Note: Selection panels only use the shared and profile pools so this       */
/*       is where all panel variables reside (eg. ZSEL, ZCMD, ZPRIM, etc)     */

/* Don't passthrough if the selection panel is defined as a primary panel     */
/* ADDPOP is accepted but does not do anything at present                     */

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
	int RC1 ;
	int p   ;

	bool passthru = false ;
	bool addpop   = false ;

	string pan   ;
	string msg   ;
	string ZCMD  ;
	string ZSEL  ;
	string ZPRIM ;

	p = wordpos( "ADDPOP", PARM ) ;
	if ( p > 0 )
	{
		idelword( PARM, p, 1 ) ;
		addpop = true ;
	}

	pan  = word( PARM, 1 ) ;
	msg  = "" ;
	ZCMD = subword( PARM, 2 ) ;
	ZSEL = "" ;

	vdefine( "ZCMD ZSEL ZPRIM", &ZCMD, &ZSEL, &ZPRIM ) ;

	if ( ZCMD != "" )
	{
		control( "DISPLAY", "NONDISPL" ) ;
		passthru = true ;
	}

	while ( true )
	{
		vput( "ZCMD ZSEL", SHARED ) ;
		display( pan, msg ) ;
		RC1 = RC ;
		msg = "" ;
		vget( "ZCMD ZSEL ZPRIM", SHARED ) ;
		if ( ZSEL  == "EXIT" || RC1 == 8 )
		{
			ZSEL = "" ;
			vput( "ZSEL", SHARED ) ;
			break ;
		}
		if ( ZSEL == "?" )
		{
			msg  = "PSYS016" ;
			ZSEL = "" ;
			continue  ;
		}
		if ( ZCMD == "" )
		{
			if ( ZPRIM != "YES" ) { msg = "PSYS017" ; }
			continue ;
		}
		if ( passthru && ZPRIM != "YES" ) { break ; }
		ZCMD = "" ;
	}

	vdelete( "ZCMD ZSEL ZPRIM" ) ;
	cleanup() ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PDPANLA ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
