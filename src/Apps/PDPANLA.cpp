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
/* Parameter ADDPOP is also passed but not currenty used.                     */

/* Note: Selection panels only use the shared and profile pools so this       */
/*       is where all panel variables reside (eg. ZSEL, ZCMD, ZPRIM, etc)     */

/* Don't passthrough if the selection panel is defined as a primary panel     */

#include "../lspf.h"
#include "../utilities.h"
#include "../classes.h"
#include "../pVPOOL.h"
#include "../pWidgets.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"

#include "PDPANLA.h"

using namespace std ;

#undef  MOD_NAME
#define MOD_NAME PDPANLA


PDPANLA::PDPANLA()
{
	set_appdesc( "Default select panel program for lspf" ) ;
	set_appver( "1.0.1" ) ;
}


void PDPANLA::application()
{
	int RC1 ;
	int p   ;

	bool passthru = false ;
	bool addpop   = false ;

	string pan   ;
	string msg   ;
	string zcmd  ;
	string zsel  ;
	string zprim ;

	p = wordpos( "ADDPOP", PARM ) ;
	if ( p > 0 )
	{
		idelword( PARM, p, 1 ) ;
		addpop = true ;
	}

	pan  = word( PARM, 1 ) ;
	zcmd = subword( PARM, 2 ) ;
	msg  = "" ;
	zsel = "" ;

	vdefine( "ZCMD ZSEL ZPRIM", &zcmd, &zsel, &zprim ) ;

	if ( zcmd != "" )
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
		if ( zsel == "EXIT" || RC1 == 8 )
		{
			zsel = "" ;
			zcmd = "" ;
			vput( "ZSEL ZCMD", SHARED ) ;
			break ;
		}
		if ( zsel == "?" )
		{
			msg  = "PSYS016" ;
			zsel = "" ;
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

	vdelete( "ZCMD ZSEL ZPRIM" ) ;
}


// ============================================================================================ //

extern "C" { pApplication* maker() { return new PDPANLA ; } }
extern "C" { void destroy(pApplication* p) { delete p ; } }
