/*  Compile with ::                                                                 */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPCMD0B.so -o libPCMD0B.so PCMD0B.cpp */

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


#include <iostream>
#include <boost/filesystem.hpp>

#include "../lspf.h"
#include "../utilities.h"
#include "../classes.h"
#include "../pVPOOL.h"
#include "../pWidgets.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"

#include "PCMD0B.h"

using namespace std ;
using namespace boost::filesystem ;

#undef  MOD_NAME
#define MOD_NAME PCMD0B


void PCMD0B::application()
{
	llog( "I", "Application PCMD0B starting" << endl ) ;

	set_appdesc( "Invoke a command and display the output" ) ;

	string comm1  ;
	string comm2  ;
	string tname1 ;
	string tname2 ;

	vdefine( "ZCMD ZVERB COMM1 COMM2", &zcmd, &zverb, &comm1, &comm2 ) ;
	vdefine( "ZAREA ZSHADOW ZAREAT ZSCROLLA", &zarea, &zshadow, &zareat, &ZSCROLLA ) ;
	vdefine( "ZSCROLLN ZAREAW ZAREAD", &ZSCROLLN, &zareaw, &zaread ) ;

	vget( "COMM1 COMM2", SHARED ) ;

	vcopy( "ZUSER", zuser, MOVE )     ;
	vcopy( "ZSCREEN", zscreen, MOVE ) ;

	boost::filesystem::path temp1 = boost::filesystem::temp_directory_path() /
			   boost::filesystem::unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;
	tname1 = temp1.native() ;
	boost::filesystem::path temp2 = boost::filesystem::temp_directory_path() /
			   boost::filesystem::unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;
	tname2 = temp2.native() ;

	pquery( "PCMD0B", "ZAREA", "ZAREAT", "ZAREAW", "ZAREAD" ) ;

	zasize       = zareaw * zaread ;
	msg          = "" ;
	topLine      = 0  ;
	startCol     = 1  ;
	rebuildZAREA = true ;

	sdr.assign( zareaw, N_RED )    ;
	sdw.assign( zareaw, N_WHITE )  ;
	sdy.assign( zareaw, N_YELLOW ) ;

	while ( true )
	{
		if ( rebuildZAREA ) { fill_dynamic_area() ; }

		display( "PCMD0B", msg, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }

		msg = "" ;
		if ( zcmd != "" )
		{
			if ( zcmd.front() == ':' )
			{
				actionCommand() ;
			}
			else
			{
				comm1 = tname1 ;
				comm2 = tname2 ;
				if ( invoke_task_wait( zcmd, comm1, comm2 ) )
				{
					copy_output( tname1, tname2 ) ;
				}
				else
				{
					timeout_output() ;
				}
				rebuildZAREA = true ;
				zcmd = "" ;
			}
		}
		else
		{
			vget( "ZVERB ZSCROLLA ZSCROLLN", SHARED ) ;
			actionZVERB() ;
		}
	}

	cleanup() ;
	return    ;
}


void PCMD0B::copy_output( const string& fname, const string& ename )
{
	std::ifstream fin ;

	lines.push_back( "" ) ;
	lines.push_back( ":> "+zcmd ) ;
	lines.push_back( "" ) ;

	fin.open( ename.c_str() ) ;
	while ( getline( fin, inLine ) )
	{
		lines.push_back( "e> "+inLine ) ;
	}
	fin.close() ;

	fin.open( fname.c_str() ) ;
	while ( getline( fin, inLine ) )
	{
		lines.push_back( inLine ) ;
	}
	fin.close() ;

	topLine  = lines.size() - zaread ;
	startCol = 1                     ;
	if ( topLine < 0 ) { topLine = 0 ; }

	lines.push_back( "" ) ;
	lines.push_back( "" ) ;

	remove( fname ) ;
	remove( ename ) ;
}


void PCMD0B::timeout_output()
{
	lines.push_back( "" ) ;
	lines.push_back( ":> "+zcmd ) ;
	lines.push_back( ":  Command has timeout waiting for a response" ) ;
	lines.push_back( "" ) ;
}


void PCMD0B::fill_dynamic_area()
{
	int dl ;
	int i  ;

	zarea   = "" ;
	zshadow = "" ;
	maxCol  = 1  ;

	if ( topLine >= lines.size() )
	{
		topLine = ( lines.size() > 0 ) ? lines.size() - 1 : 0 ;
	}

	for ( dl = topLine, i = 0 ; i < zaread && dl < lines.size() ; i++, dl++ )
	{
		string& pstr = lines[ dl ] ;
		maxCol = max( maxCol, int( pstr.size() ) ) ;
		zarea += substr( pstr, startCol, zareaw ) ;
		if ( pstr.compare( 0, 2, ":>" ) == 0 )      { zshadow += sdw ; }
		else if ( pstr.compare( 0, 2, "e>" ) == 0 ) { zshadow += sdr ; }
		else if ( pstr.compare( 0, 2, ": " ) == 0 ) { zshadow += sdr ; }
		else { zshadow += sdy ; }
	}

	zarea.resize( zasize, ' ' ) ;
	zshadow.resize( zasize, N_YELLOW ) ;
	rebuildZAREA = false ;
}


void PCMD0B::actionCommand()
{
	string cmd ;

	cmd = upper( zcmd ) ;
	if ( cmd == ":CLEAR" )
	{
		lines.clear()  ;
		topLine      = 0  ;
		startCol     = 1  ;
		zcmd         = "" ;
		rebuildZAREA = true ;
	}
	else
	{
		msg = "PSYS018" ;
	}
}


void PCMD0B::actionZVERB()
{
	if ( zverb == "DOWN" )
	{
		rebuildZAREA = true ;
		if ( ZSCROLLA == "MAX" )
		{
			topLine = lines.size() - 1 - zaread ;
		}
		else
		{
			topLine += ZSCROLLN ;
		}
	}
	else if ( zverb == "UP" )
	{
		rebuildZAREA = true ;
		if ( ZSCROLLA == "MAX" )
		{
			topLine = 0 ;
		}
		else
		{
			topLine -= ZSCROLLN ;
		}
	}
	else if ( zverb == "LEFT" )
	{
		rebuildZAREA = true ;
		if ( ZSCROLLA == "MAX" )
		{
			startCol = 1 ;
		}
		else
		{
			startCol = startCol - ZSCROLLN ;
		}
	}
	else if ( zverb == "RIGHT" )
	{
		rebuildZAREA = true ;
		if ( ZSCROLLA == "MAX" )
		{
			startCol = maxCol - zareaw - 1 ;
		}
		else
		{
			startCol += ZSCROLLN ;
		}
	}
	if ( topLine  < 0 ) { topLine  = 0 ; }
	if ( startCol < 1 ) { startCol = 1 ; }
}


bool PCMD0B::invoke_task_wait( const string& cmd, string& comm1, const string& comm2 )
{
	// Invoke a background task and wait for a reponse (using PCMD0A application)
	// Timeout after 10seconds

	int elapsed ;

	vput( "COMM1 COMM2", SHARED ) ;

	select( "PGM(PCMD0A) PARM("+cmd+") BACK" ) ;

	elapsed = 0 ;
	while ( comm1 != "" )
	{
		boost::this_thread::sleep_for(boost::chrono::milliseconds( 5 ) ) ;
		if ( ++elapsed > 2000 )
		{
			return false ;
		}
		vget( "COMM1", SHARED ) ;
		if ( RC > 0 ) { comm1 = "" ; }
	}
	return true ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PCMD0B ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
