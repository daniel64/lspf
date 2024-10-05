/* Compile with ::                                                                        */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libpedumac1.so -o libpedumac1.so pedumac1.cpp */

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

/************************************************************************************************/
/*                                                                                              */
/* Edit macro to select Edit profile using REGEX patterns from the EDTYPES table.               */
/*                                                                                              */
/* Set this macro as a user session initial macro in the Edit Settings pulldown.                */
/* REXX command prules can then be used to add entries to the EDTYPES table.                    */
/*                                                                                              */
/* A passed profile name on the Edit service (eg from the Edit Entry screen), will override     */
/* this selection.                                                                              */
/*                                                                                              */
/* If PROFILE USE TYPE is in effect, the override is only temporary and                         */
/* automatic language selection will be used if there is no match in the EDTYPES                */
/* table in a subsequent Edit session.                                                          */
/*                                                                                              */
/************************************************************************************************/


#include "pedmcp1.cpp"
#include "pedumac1.h"

using namespace boost ;

LSPF_APP_MAKER( pedumac1 )


pedumac1::pedumac1()
{
	STANDARD_HEADER( "Edit macro to set the edit profile from table EDTYPES", "1.0.0" )
}


void pedumac1::start_pgm()
{
	boost::regex* expression = new boost::regex ;

	string file ;
	string prof ;
	string zedproft ;

	string zedutype ;
	string zeduregx ;
	string zedufile ;
	string zeducase ;
	string zedxprof ;

	string setundo ;

	const string tabName = "EDTYPES" ;
	const string vlist1  = "FILE PROF ZEDPROFT ZEDUFILE ZEDUREGX ZEDUTYPE ZEDUCASE SETUNDO ZEDXPROF" ;

	isredit( "MACRO" ) ;

	vdefine( vlist1, &file, &prof, &zedproft, &zedufile, &zeduregx, &zedutype, &zeducase, &setundo, &zedxprof ) ;

	vget( "ZEDPROFT", PROFILE ) ;
	vget( "ZEDXPROF", SHARED ) ;

	libdef( "TABLIB", "LIBRARY", "ZUPROF" ) ;

	if ( zedxprof != "" )
	{
		vdelete( vlist1 ) ;
		delete expression ;
		return ;
	}

	isredit( "(FILE) = DATASET" ) ;
	isredit( "(PROF) = PROFILE" ) ;

	tbopen( tabName, NOWRITE, "TABLIB" ) ;
	if ( RC == 0 )
	{
		tbskip( tabName ) ;
		while ( RC == 0 )
		{
			try
			{
				if ( zeducase == "I" )
				{
					expression->assign( zeduregx, boost::regex_constants::icase ) ;
				}
				else
				{
					expression->assign( zeduregx ) ;
				}
			}
			catch ( boost::regex_error& e )
			{
				tbskip( tabName ) ;
				continue ;
			}
			if ( regex_match( file.begin(), file.end(), *expression ) )
			{
				if ( prof != zedutype )
				{
					isredit( "RESET" ) ;
					isredit( "PROFILE 0 " + zedutype ) ;
					isredit( "(SETUNDO) = SETUNDO" ) ;
					if ( setundo != "OFF" )
					{
						isredit( "SETUNDO = OFF" ) ;
						isredit( "SETUNDO = (SETUNDO)" ) ;
					}
					vput( "ZEDPROFT", PROFILE ) ;
				}
				break ;
			}
			tbskip( tabName ) ;
		}
		tbend( tabName ) ;
	}

	libdef( "TABLIB" ) ;
	vdelete( vlist1 ) ;

	delete expression ;
}
