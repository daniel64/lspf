/*  Compile with ::                                                                 */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,panload1.so -o panload1.so panload1.cpp */

/*
  Copyright (c) 2024 Daniel John Erdos

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

/******************************************************************************************************************/
/*                                                                                                                */
/* PANEXIT panload1 test program.                                                                                 */
/*                                                                                                                */
/* Program must inherit from class panload and implement pure virtual method go().                                */
/*                                                                                                                */
/*   exitInfo2:                                                                                                   */
/*      string msgid                                                                                              */
/*      string panelid                                                                                            */
/*      string psection                                                                                           */
/*      size_t numvars                                                                                            */
/*      vector<string> vars                                                                                       */
/*      vector<string>* vals                                                                                      */
/*   Any changes to vars, panelid, psection or numvers are ignored.                                               */
/*                                                                                                                */
/* Return value:                                                                                                  */
/*    0 Normal completion.                                                                                        */
/*    8 Panel LOAD defined failure.  lspf sets .MSG and displays/redisplays panel.                                */
/*   20 Severe error in the PANEXIT module.                                                                       */
/*   -- Any other value also gives a severe error.                                                                */
/*                                                                                                                */
/* msgid - message id used to set .MSG when return code = 8 (MSG= or PSYE038X if blank) or                        */
/*         ZERRMSG if a severe error (any value other than 0 and 8).  (MSG= or PSYE038Y if blank)                 */
/*                                                                                                                */
/* PANEXIT LOAD does not have access to lspf services.                                                            */
/*                                                                                                                */
/******************************************************************************************************************/


#include "../lspfall.h"
#include "panload1.h"

PANEXIT_MAKER( panload1 )


int panload1::go( exitInfo2& ei )
{
	//
	// Example PANEXIT LOAD program.
	// List out the values passed in the exit information structure and
	// alter a few fields.  Increment the call count in the 4th field if numeric.
	//

	cout << endl ;
	cout << "Start of exit info values." << endl ;
	cout << "=========================." << endl ;
	cout << "Panelid          = " << ei.panelid << endl ;
	cout << "Panel Section    = " << ei.psection << endl ;
	cout << "Exit data        = " << ei.exdata << endl ;
	cout << "Variables passed = " << ei.numvars << endl ;
	cout << endl ;

	cout << "Variable  Value" << endl ;
	cout << "--------  -----" << endl ;

	for ( uint j = 0 ; j < ei.vars.size() ; ++j )
	{
		cout << left( ei.vars[ j ], 10 ) << ei.vals->at( j ) << endl ;
	}

	if ( ei.vals->at( 0 ).size() > 1 )
	{
		ei.vals->at( 0 ).insert( 0, 1, ei.vals->at( 0 ).back() ) ;
		ei.vals->at( 0 ).pop_back() ;
	}

	if ( ei.vals->at( 1 ).size() > 1 )
	{
		ei.vals->at( 1 ).push_back( ei.vals->at( 1 ).front() ) ;
		ei.vals->at( 1 ).erase( 0, 1 ) ;
	}

	if ( ei.vals->at( 3 ) == "" )
	{
		ei.vals->at( 3 ) = "00000001" ;
	}
	else if ( isnumeric( ei.vals->at( 3 ) ) )
	{
		ei.vals->at( 3 ) = d2ds( ds2d( ei.vals->at( 3 ) ) + 1, 8 ) ;
	}

	ei.vals->at( 4 ) = "Updated by PANEXIT" ;

	ei.msgid = "PSYS011" ;

	return 0 ;
}
