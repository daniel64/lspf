/*  Compile with ::                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPSHELL0.so -o libPSHELL0.so PSHELL0.cpp */

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

/* ************************************************************************ */
/* Run a shell script and direct output to the spool.                       */
/* Functions invoked by this procedure, do not have access to lspf services */
/* since they are run using popen.                                          */
/* ************************************************************************ */

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

#include "PSHELL0.h"

using namespace std ;
using namespace boost::filesystem ;

#undef  MOD_NAME
#define MOD_NAME PSHELL0


PSHELL0::PSHELL0()
{
	set_appdesc( "Invoke a shell command" ) ;
	set_appver( "1.0.0" ) ;
}


void PSHELL0::application()
{
	control( "NOTIFY", "JOBEND" ) ;
	run_command( PARM ) ;
}


void PSHELL0::run_command( string cmd )
{
	// Run shell command and detach from the terminal (unless pipes are used).

	// Command goes to fname1
	// Output goes to fname2
	// Errors go to fname3

	string t      ;
	string file   ;
	string fname1 ;
	string fname2 ;
	string fname3 ;

	t = word( cmd, 1 ) ;

	fname1 = get_tempname( t, "input" ) ;
	fname2 = get_tempname( t, "output" ) ;
	fname3 = get_tempname( t, "errors" ) ;

	std::ofstream fout( fname1.c_str() ) ;

	if ( fout.is_open() )
	{
		fout << cmd << endl ;
	}
	fout.close() ;

	char buffer[256] ;

	std::ofstream of ;

	if ( cmd.find( '|' ) == string::npos ) { cmd += " </dev/null 2> "+ fname3 ; }
	else                                   { cmd += " 2> "+ fname3            ; }

	of.open( fname2 ) ;
	FILE* pipe{ popen( cmd.c_str(), "r" ) } ;

	while ( fgets( buffer, sizeof( buffer ), pipe ) != nullptr )
	{
		file = buffer ;
		of << file.substr( 0, file.size() - 1 ) << endl ;
	}

	pclose( pipe ) ;
	of.close() ;
}


string PSHELL0::get_tempname( const string& suf1, const string& suf2 )
{
	// For command output, spool file name is of the form:
	// jobkey-SHELL-cmd-stream
	// suf1 - command name
	// suf2 - output stream

	string zspool ;

	vget( "ZSPOOL", PROFILE ) ;
	vcopy( "ZSPOOL", zspool, MOVE ) ;

	if ( zspool.back() != '/' ) { zspool.push_back( '/' ) ; }

	return zspool + get_jobkey() + "-SHELL-" + suf1 + "-" + suf2 ;
}


// ============================================================================================ //

extern "C" { pApplication* maker() { return new PSHELL0 ; } }
extern "C" { void destroy(pApplication* p) { delete p ; } }
