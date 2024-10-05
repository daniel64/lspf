/*  Compile with ::                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libpshell0.so -o libpshell0.so pshell0.cpp */

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

/****************************************************************************/
/*                                                                          */
/* Run a shell script and direct output to the spool.                       */
/* Functions invoked by this procedure, do not have access to lspf services */
/* since they are run using popen.                                          */
/*                                                                          */
/****************************************************************************/

#include <iostream>
#include <boost/filesystem.hpp>

#include "../lspfall.h"
#include "pshell0.h"

using namespace std ;
using namespace boost::filesystem ;

LSPF_APP_MAKER( pshell0 )


pshell0::pshell0()
{
	STANDARD_HEADER( "Invoke a shell command", "1.0.2" )
}


void pshell0::application()
{
	control( "NOTIFY", "JOBEND" ) ;
	run_command( PARM ) ;
}


void pshell0::run_command( string cmd )
{
	//
	// Run shell command and detach from the terminal (unless pipes are used).
	//
	// Command goes to fname1.
	// Output goes to fname2.
	// Errors go to fname3.
	//

	string file ;

	string t = word( cmd, 1 ) ;

	string fname1 = get_spool_filename( t, "input" ) ;
	string fname2 = get_spool_filename( t, "output" ) ;
	string fname3 = get_spool_filename( t, "errors" ) ;

	std::ofstream fout( fname1.c_str() ) ;

	if ( fout.is_open() )
	{
		fout << cmd << endl ;
	}
	fout.close() ;

	char buffer[2560] ;

	std::ofstream of ;

	if ( cmd.find( '|' ) == string::npos ) { cmd += " </dev/null 2> "+ fname3 ; }
	else                                   { cmd += " 2> "+ fname3            ; }

	FILE* pipe{ popen( cmd.c_str(), "r" ) } ;

	if ( !pipe )
	{
		llog( "E", "POPEN failed.  Command string size="<< cmd.size() <<endl ) ;
		return ;
	}

	of.open( fname2 ) ;
	while ( fgets( buffer, sizeof( buffer ), pipe ) )
	{
		file = buffer ;
		if ( file != "" && file.back() == 0x0a )
		{
			file.pop_back() ;
		}
		of << file << endl ;
	}
	pclose( pipe ) ;

	of.close() ;
}


string pshell0::get_spool_filename( const string& suf1,
				    const string& suf2 )
{
	//
	// For command output, spool file name is of the form:
	// jobkey-SHELL-cmd-stream.
	// suf1 - command name.
	// suf2 - output stream.
	//

	string zspool ;

	vget( "ZSPOOL", PROFILE ) ;
	vcopy( "ZSPOOL", zspool, MOVE ) ;

	if ( zspool.back() != '/' ) { zspool.push_back( '/' ) ; }

	return zspool + get_jobkey() + "-SHELL-" + suf1 + "-" + suf2 ;
}
