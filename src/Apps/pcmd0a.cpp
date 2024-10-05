/*  Compile with ::                                                                 */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libpcmd0a.so -o libpcmd0a.so pcmd0a.cpp */

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

/****************************************************************************/
/*                                                                          */
/* Run a command and browse the output.                                     */
/*                                                                          */
/* If the procedure is a REXX in ALTLIB, the SELECT service is used so      */
/* will be able to use lspf services, and runs in the foreground, otherwise */
/* popen is used and the procedure will not have access to lspf services.   */
/* popen runs in the background.                                            */
/*                                                                          */
/* Output goes to the files specified in COMM1 and COMM2.                   */
/*                                                                          */
/* Parameters:                                                              */
/*   PANEL(name)  Use panel 'name' to enter commands.                       */
/*   --BROWSE     Browse output by starting a nested application.           */
/*   --NOBROWSE   Don't browse the output produced.                         */
/*   --IMMED      Don't run the command in batch mode (no timeout).         */
/*   cmd          Run the shell command or REXX in batch mode.              */
/*                                                                          */
/* On exit:                                                                 */
/*   ZRC=127 if the command does not exist. Only set for --IMMED and cmd    */
/*                                                                          */
/****************************************************************************/


#include <iostream>
#include <boost/filesystem.hpp>

#include "../lspfall.h"
#include "../pTSOenv.h"
#include "../pTSOenv.cpp"

#include "pcmd0a.h"

using namespace std ;
using namespace boost::filesystem ;


LSPF_APP_MAKER( pcmd0a )


pcmd0a::pcmd0a()
{
	STANDARD_HEADER( "Invoke a command and display the output", "1.0.3" )
}


void pcmd0a::application()
{
	size_t p ;

	bool browse_output = ( sysvar( "SYSENV" ) == "FORE" ) ;

	string zcmd  ;
	string msg   ;
	string comm1 ;
	string comm2 ;
	string panel = "PCMD0A" ;

	if ( PARM.compare( 0, 6, "PANEL(" ) == 0 )
	{
		p = PARM.find( ')', 5 ) ;
		if ( p != string::npos )
		{
			panel = strip( PARM.substr( 6, p-6 ) ) ;
			PARM  = "" ;
		}
	}
	else if ( word( PARM, 1 ) == "--NOBROWSE" )
	{
		browse_output = false ;
		idelword( PARM, 1, 1 ) ;
	}
	else if ( word( PARM, 1 ) == "--BROWSE" )
	{
		zcmd  = get_string( PARM, "CMD" ) ;
		comm1 = get_string( PARM, "FILE1" ) ;
		comm2 = get_string( PARM, "FILE2" ) ;
		vreplace( "ZBRALT", "COMMAND: "+ zcmd ) ;
		vput( "ZBRALT", SHARED ) ;
		control( "ERRORS", "RETURN" ) ;
		browse( comm1 ) ;
		if ( RC == 12 )
		{
			browse( comm2 ) ;
		}
		control( "ERRORS", "CANCEL" ) ;
		return ;
	}
	else if ( word( PARM, 1 ) == "--IMMED" )
	{
		comm1 = get_tempname( "output" ) ;
		comm2 = get_tempname( "errors" ) ;
		run_command( subword( PARM, 2 ), comm1, comm2 ) ;
		if ( ZRC == 127 )
		{
			remove( comm1 ) ;
			remove( comm2 ) ;
			return ;
		}
		if ( browse_output )
		{
			vreplace( "ZBRALT", "COMMAND: "+ subword( PARM, 2 ) ) ;
			vput( "ZBRALT", SHARED ) ;
			control( "ERRORS", "RETURN" ) ;
			browse( comm1 ) ;
			if ( RC == 12 ) { browse( comm2 ) ; }
			control( "ERRORS", "CANCEL" ) ;
		}
		remove( comm1 ) ;
		remove( comm2 ) ;
		return ;
	}

	vdefine( "ZCMD COMM1 COMM2", &zcmd, &comm1, &comm2 ) ;
	vget( "COMM1 COMM2", SHARED ) ;
	if ( RC == 0 && PARM != "" )
	{
		verase( "COMM1 COMM2", SHARED ) ;
		enq( "CMD", comm1 ) ;
		run_command( PARM, comm1, comm2 ) ;
		deq( "CMD", comm1 ) ;
		return ;
	}

	vcopy( "ZUSER", zuser, MOVE )     ;
	vcopy( "ZSCREEN", zscreen, MOVE ) ;

	if ( PARM != "" )
	{
		if ( builtin( PARM ) )
		{
			return ;
		}
		else if ( !command_exists( word( PARM, 1 ) ) )
		{
			say( "COMMAND "+ word( PARM, 1 )  +" NOT FOUND" ) ;
			ZRC = 127 ;
			return ;
		}
		comm1 = get_tempname( "output" ) ;
		comm2 = get_tempname( "errors" ) ;
		if ( invoke_task_and_wait( PARM, comm1, comm2 ) )
		{
			if ( browse_output )
			{
				vreplace( "ZBRALT", "COMMAND: "+ PARM ) ;
				vput( "ZBRALT", SHARED ) ;
				control( "ERRORS", "RETURN" ) ;
				browse( comm1 ) ;
				if ( RC == 12 ) { browse( comm2 ) ; }
				control( "ERRORS", "CANCEL" ) ;
			}
		}
		else
		{
			setmsg( "PSYS012S" ) ;
		}
		remove( comm1 ) ;
		remove( comm2 ) ;
		return ;
	}

	while ( true )
	{
		display( panel, msg, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		if ( zcmd != "" )
		{
			comm1 = get_tempname( "output" ) ;
			comm2 = get_tempname( "errors" ) ;
			if ( isRexx( word( zcmd, 1 ) ) )
			{
				control( "ERRORS", "RETURN" ) ;
				select( "CMD(%" + zcmd + ") LANG(REXX) NEWPOOL NESTED" ) ;
				if ( ZRC == 20 && ZRSN == 999 && ZRESULT == "Abended" )
				{
					vreplace( "ZEDSMSG", "Abended" ) ;
					vreplace( "ZEDLMSG", "Command '"+ zcmd +"' has abended" ) ;
					setmsg( "PSYZ001" ) ;
				}
				else
				{
					zcmd = "" ;
				}
				control( "ERRORS", "CANCEL" ) ;
			}
			else if ( builtin( zcmd ) )
			{
				zcmd = "" ;
			}
			else if ( !command_exists( word( zcmd, 1 ) ) )
			{
				say( "COMMAND "+ word( zcmd, 1 )  +" NOT FOUND" ) ;
			}
			else
			{
				if ( invoke_task_and_wait( zcmd, comm1, comm2 ) )
				{
					select( "PGM(PCMD0A) PARM('--BROWSE CMD(" +
								  zcmd  + ") FILE1(" +
								  comm1 + ") FILE2(" +
								  comm2 + ")') NESTED" ) ;
					zcmd = "" ;
				}
				else
				{
					msg = "PSYS012S" ;
				}
			}
			remove( comm2 ) ;
			remove( comm1 ) ;
		}
	}

	verase( "ZBRALT", SHARED ) ;
}


bool pcmd0a::invoke_task_and_wait( const string& cmd,
				   const string& comm1,
				   const string& comm2 )
{
	//
	// Invoke a background task and wait for a reponse.
	// Timeout after 10seconds.
	//

	int elapsed ;

	vput( "COMM1 COMM2", SHARED ) ;

	submit( "PGM(PCMD0A) PARM("+ cmd +")" ) ;

	elapsed = 0 ;
	while ( true )
	{
		boost::this_thread::sleep_for(boost::chrono::milliseconds( 5 ) ) ;
		if ( ++elapsed > 2000 )
		{
			return false ;
		}
		qscan( "CMD", comm1 ) ;
		if ( RC == 8 ) { break ;}
	}

	return true ;
}


void pcmd0a::run_command( string cmd,
			  const string& fname1,
			  const string& fname2 )
{
	//
	// Run command in the background and detach from the terminal (unless pipes are used)
	// in case it hangs or messes with ncurses.
	//
	// If command is a REXX, run using the SELECT service.
	//
	// Output goes to fname1.
	// Errors go to fname2.
	//

	string file ;

	char buffer[ 2560 ] ;

	std::ofstream of ;

	if ( cmd.front() == '%' )
	{
		select( "CMD("+ cmd +")" ) ;
		return ;
	}

	if ( cmd.find( '|' ) == string::npos ) { cmd += " </dev/null 2> "+ fname2 ; }
	else                                   { cmd += " 2> "+ fname2            ; }

	FILE* pipe{ popen( cmd.c_str(), "r" ) } ;

	if ( !pipe )
	{
		llog( "E", "POPEN failed.  Command string size="<< cmd.size() <<endl ) ;
		return ;
	}

	of.open( fname1 ) ;
	while ( fgets( buffer, sizeof( buffer ), pipe ) )
	{
		file = buffer ;
		if ( file != "" && file.back() == 0x0a )
		{
			file.pop_back() ;
		}
		of << file << endl ;
	}

	ZRC = WEXITSTATUS( pclose( pipe ) ) ;
	of.close() ;
}


bool pcmd0a::command_exists( const string& cmd )
{
	//
	// Check if a command exists.
	//

	string wcmd = "which '"+ cmd +"' &> /dev/null " ;

	FILE* pipe{ popen( wcmd.c_str(), "r" ) } ;
	ZRC = WEXITSTATUS( pclose( pipe ) ) ;

	return ( ZRC != 1 ) ;
}


bool pcmd0a::isRexx( string orexx )
{
	locator loc( sysexec(), orexx ) ;
	loc.locate() ;

	return loc.found() ;
}


string pcmd0a::get_string( const string& s,
			   const string& t )
{
	string p = t + "(" ;

	size_t p1 ;
	size_t p2 ;

	int ob = 1 ;

	p1 = s.find( " " + p ) + 1 ;

	for ( p2 = p1 + p.size() ; p2 < s.size() && ob > 0 ; ++p2 )
	{
		if ( s.at( p2 ) == '(' ) { ++ob ; }
		if ( s.at( p2 ) == ')' )
		{
			ob-- ;
		}
	}

	return strip( s.substr( p1 + p.size(), p2 - p1 - p.size() - 1 ) ) ;
}




string pcmd0a::get_tempname( const string& suf )
{
	boost::filesystem::path temp = boost::filesystem::temp_directory_path() /
				       boost::filesystem::unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;

	return temp.native() + "." + suf ;
}
