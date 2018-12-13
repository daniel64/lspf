/* Compile with ::                                                                  */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPCMD0B.so -o libPCMD0B.so PCMD0B.cpp */

/*
  Copyright (c) 2018 Daniel John Erdos

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

#define E_RED      3
#define E_GREEN    4
#define E_YELLOW   5
#define E_BLUE     6
#define E_MAGENTA  7
#define E_TURQ     8
#define E_WHITE    9


PCMD0B::PCMD0B()
{
	set_appdesc( "Invoke a command and display the output" ) ;
	set_appver( "1.0.1" ) ;
}

void PCMD0B::application()
{
	bool exit = false ;

	string comm1 ;
	string comm2 ;

	vector<pair<string,string>> tnames ;

	vdefine( "ZCMD ZVERB ZNODNAME COMM1 COMM2", &zcmd, &zverb, &znode, &comm1, &comm2 ) ;
	vdefine( "ZAREA ZSHADOW ZAREAT ZSCROLLA", &zarea, &zshadow, &zareat, &zscrolla ) ;
	vdefine( "ZSCROLLN ZAREAW ZAREAD", &zscrolln, &zareaw, &zaread ) ;

	vget( "COMM1 COMM2 ZNODNAME", SHARED ) ;

	vcopy( "ZUSER", zuser, MOVE )     ;
	vcopy( "ZSCREEN", zscreen, MOVE ) ;

	pquery( "PCMD0B", "ZAREA", "ZAREAT", "ZAREAW", "ZAREAD" ) ;

	zasize       = zareaw * zaread ;
	msg          = "" ;
	topLine      = 0  ;
	startCol     = 1  ;
	rebuildZAREA = true ;

	sdr.assign( zareaw, E_RED )    ;
	sdw.assign( zareaw, E_WHITE )  ;
	sdy.assign( zareaw, E_YELLOW ) ;
	sdg.assign( zareaw, E_GREEN )  ;
	sdt.assign( zareaw, E_TURQ  )  ;

	boost::filesystem::path p = boost::filesystem::current_path() ;
	wd = p.native() ;

	lines.push_back( command_prompt() ) ;

	while ( true )
	{
		if ( rebuildZAREA ) { fill_dynamic_area( tnames.size() > 0 ) ; }

		display( "PCMD0B", msg, "ZCMD" ) ;
		if ( RC == 8 )
		{
			if ( exit || tnames.size() == 0 ) { return ; }
			vreplace( "ZEDSMSG", "Background tasks still running" ) ;
			vreplace( "ZEDLMSG", "Press PF3 again to exit or ENTER to display output" ) ;
			msg  = "PSYZ001" ;
			exit = true ;
			continue ;
		}
		exit = false ;
		msg  = ""    ;

		vget( "ZVERB ZSCROLLA ZSCROLLN", SHARED ) ;
		if ( zcmd != "" )
		{
			if ( zcmd.front() == '-' )
			{
				actioniCommand() ;
			}
			else
			{
				comm1 = get_tempname( "output" ) ;
				comm2 = get_tempname( "errors" ) ;
				tnames.push_back( make_pair( comm1, comm2 ) ) ;
				cmds[ comm1 ] = zcmd ;
				invoke_task( zcmd, comm1, comm2 ) ;
				rebuildZAREA = true ;
				zcmd = "" ;
			}
		}
		else if ( zverb != "" )
		{
			actionZVERB() ;
		}
		else
		{
			lines.push_back( command_prompt() ) ;
			bottom_of_data() ;
			rebuildZAREA = true ;
		}
		for ( auto it = tnames.begin() ; it != tnames.end() ; )
		{
			qscan( "CMD", it->first ) ;
			if ( RC == 8 )
			{
				copy_output( cmds[ it->first ], it->first, it->second ) ;
				cmds.erase( it->first ) ;
				it = tnames.erase( it ) ;
				rebuildZAREA = true ;
				continue ;
			}
			it++ ;
		}
	}

	return ;
}


void PCMD0B::copy_output( const string& cmd, const string& fname, const string& ename )
{
	bool errors = false ;

	string t ;

	std::ifstream fin ;

	lines.push_back( "" ) ;
	lines.push_back( ":> "+cmd ) ;
	lines.push_back( "" ) ;

	fin.open( ename.c_str() ) ;
	while ( getline( fin, inLine ) )
	{
		lines.push_back( "e> "+inLine ) ;
		errors = true ;
	}
	fin.close() ;

	fin.open( fname.c_str() ) ;
	while ( getline( fin, inLine ) )
	{
		lines.push_back( inLine ) ;
	}
	fin.close() ;

	lines.push_back( "" ) ;

	remove( fname ) ;
	remove( ename ) ;

	if ( word( cmd, 1 ) == "cd" && not errors )
	{
		t = subword( cmd, 2 ) ;
		if ( t.size() > 0 && t.front() == '/' )
		{
			wd = t ;
		}
		else
		{
			if ( wd.back() != '/' ) { wd += "/" ; }
			wd += t ;
		}
	}

	lines.push_back( command_prompt() ) ;
	bottom_of_data() ;
}


void PCMD0B::actioniCommand()
{
	string cmd  ;
	string rest ;

	cmd  = upper( word( zcmd, 1 ) ) ;
	rest = subword( zcmd, 2 ) ;
	if ( cmd == "-CLEAR" )
	{
		lines.clear() ;
		topLine      = 0  ;
		startCol     = 1  ;
		zcmd         = "" ;
		lines.push_back( command_prompt() ) ;
		rebuildZAREA = true ;
	}
	else if ( cmd == "-LIST" )
	{
		lines.push_back( "" ) ;
		lines.push_back( "b> Running background jobs" ) ;
		lines.push_back( "" ) ;
		if ( cmds.size() == 0 )
		{
			lines.push_back( "No jobs found" ) ;
		}
		else
		{
			for ( auto it = cmds.begin() ; it != cmds.end() ; it++ )
			{
				lines.push_back( it->second ) ;
			}
		}
		lines.push_back( "" ) ;
		zcmd         = "" ;
		rebuildZAREA = true ;
		bottom_of_data() ;
	}
	else
	{
		msg = "PSYS018" ;
	}
}


void PCMD0B::bottom_of_data()
{
	topLine  = lines.size() - zaread ;
	startCol = 1                     ;
	if ( topLine < 0 ) { topLine = 0 ; }
}


string PCMD0B::command_prompt()
{
	return zuser + "@" + znode + " " + wd + ">" ;
}


string PCMD0B::get_tempname( const string& suf )
{
	boost::filesystem::path temp = boost::filesystem::temp_directory_path() /
			boost::filesystem::unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;
	return temp.native() + "." + suf ;
}


void PCMD0B::fill_dynamic_area( bool running )
{
	int i ;

	size_t dl ;

	string hostUser = zuser + "@" + znode ;

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
		maxCol = max( maxCol, uint( pstr.size() ) ) ;
		zarea += substr( pstr, startCol, zareaw ) ;
		if      ( pstr.compare( 0, 2, ":>" ) == 0 ) { zshadow += sdw ; }
		else if ( pstr.compare( 0, 2, "e>" ) == 0 ) { zshadow += sdr ; }
		else if ( pstr.compare( 0, 2, "l>" ) == 0 ) { zshadow += sdg ; }
		else if ( pstr.compare( 0, 2, "b>" ) == 0 ) { zshadow += sdg ; }
		else if ( pstr.compare( 0, hostUser.size(), hostUser ) == 0 )  { zshadow += sdt ; }
		else if ( pstr.compare( 0, 2, ": " ) == 0 ) { zshadow += sdr ; }
		else                                        { zshadow += sdy ; }
	}

	zarea.resize( zasize, ' ' ) ;
	zshadow.resize( zasize, E_YELLOW ) ;
	if ( running )
	{
		zarea.replace( zasize-8, 7, "RUNNING" ) ;
		zshadow.replace( zasize-8, 7, 7, E_WHITE ) ;
	}
	else if ( ( topLine + zaread + 2 ) < lines.size() )
	{
		zarea.replace( zasize-8, 7, "MORE..." ) ;
		zshadow.replace( zasize-8, 7, 7, E_WHITE ) ;
	}
	rebuildZAREA = false ;
}


void PCMD0B::actionZVERB()
{
	if ( zverb == "DOWN" )
	{
		rebuildZAREA = true ;
		if ( zscrolla == "MAX" )
		{
			topLine = lines.size() - 1 - zaread ;
		}
		else
		{
			topLine += zscrolln ;
		}
	}
	else if ( zverb == "UP" )
	{
		rebuildZAREA = true ;
		if ( zscrolla == "MAX" )
		{
			topLine = 0 ;
		}
		else
		{
			topLine -= zscrolln ;
		}
	}
	else if ( zverb == "LEFT" )
	{
		rebuildZAREA = true ;
		if ( zscrolla == "MAX" )
		{
			startCol = 1 ;
		}
		else
		{
			startCol = startCol - zscrolln ;
		}
	}
	else if ( zverb == "RIGHT" )
	{
		rebuildZAREA = true ;
		if ( zscrolla == "MAX" )
		{
			startCol = maxCol - zareaw - 1 ;
		}
		else
		{
			startCol += zscrolln ;
		}
	}

	if ( topLine  < 0 ) { topLine  = 0 ; }
	if ( startCol < 1 ) { startCol = 1 ; }
}


bool PCMD0B::invoke_task( string cmd, string& comm1, const string& comm2 )
{
	// Invoke a background task and wait for a reponse (using PCMD0A application)
	// Wait for up to 0.5 second for the command to end, otherwise return.

	// Return:  false - no reponse received within period.  Job running in the background.
	//          true  - reponse received

	int elapsed ;

	vput( "COMM1 COMM2", SHARED ) ;

	if ( wd != "" ) { cmd = "cd " + wd + " && " + cmd ; }

	select( "PGM(PCMD0A) PARM("+cmd+") BACK" ) ;

	boost::this_thread::sleep_for(boost::chrono::milliseconds( 50 ) ) ;
	elapsed = 0 ;
	while ( true )
	{
		boost::this_thread::sleep_for(boost::chrono::milliseconds( 5 ) ) ;
		if ( ++elapsed > 100 )
		{
			return false ;
		}
		qscan( "CMD", comm1 ) ;
		if ( RC == 8 ) { break ; }
	}
	return true ;
}


// ============================================================================================ //

extern "C" { pApplication* maker() { return new PCMD0B ; } }
extern "C" { void destroy(pApplication* p) { delete p ; } }
