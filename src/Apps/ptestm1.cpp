/*  Compile with ::                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libptestm1.so -o libptestm1.so ptestm1.cpp */

/*
  Copyright (c) 2022 Daniel John Erdos

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
/* Test program for edit macros written in C++.                                                */
/*                                                                                             */
/* Edit macro programs must inherit from pedmcp1 and implement pure virtual method             */
/* start_pgm().                                                                                */
/*                                                                                             */
/* C++ version of the COBOL example program ISRSEPC in ISPF Edit and Edit Macros.              */
/*                                                                                             */
/***********************************************************************************************/


#include "pedmcp1.cpp"
#include "ptestm1.h"

LSPF_APP_MAKER( ptestm1 )


ptestm1::ptestm1()
{
	STANDARD_HEADER( "Example edit macro written in C++", "1.0.0" )
}


void ptestm1::start_pgm()
{
	string linex = string( 80, '-' ) ;

	int line ;
	int lastl ;

	vdefine( "LASTL LINE", &lastl, &line ) ;
	vdefine( "LINEX", &linex ) ;

	isredit( "MACRO" ) ;
	isredit( "(SAVE) = USER_STATE" ) ;
	isredit( "RESET" ) ;
	isredit( "EXCLUDE ------ 1 ALL" ) ;
	isredit( "DELETE ALL X" ) ;

	for ( line = 0, lastl = 1 ; line < ( lastl + 1 ) ; line += 2 )
	{
		isredit( "LINE_AFTER &LINE = (LINEX)" ) ;
		isredit( "(LASTL) = LINENUM .ZLAST" ) ;
	}

	isredit( "USER_STATE = (SAVE)" ) ;
}
