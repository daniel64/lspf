/*  Compile with ::                                                                 */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPCMD0A.so -o libPCMD0A.so PCMD0A.cpp */

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

#include "PCMD0A.h"

using namespace std ;
using namespace boost::filesystem ;

#undef  MOD_NAME
#define MOD_NAME PCMD0A


PCMD0A::PCMD0A()
{
	set_appdesc( "Invoke a command and display the output" ) ;
	set_appver( "1.0.1" ) ;
}


void PCMD0A::application()
{
	string zcmd  ;
	string msg   ;
	string comm1 ;
	string comm2 ;

	vdefine( "ZCMD COMM1 COMM2", &zcmd, &comm1, &comm2 ) ;
	vget( "COMM1 COMM2", SHARED ) ;
	if ( RC == 0 && PARM != "" )
	{
		enq( "CMD", comm1 ) ;
		run_command( PARM, comm1, comm2 ) ;
		deq( "CMD", comm1 ) ;
		return ;
	}

	vcopy( "ZUSER", zuser, MOVE )     ;
	vcopy( "ZSCREEN", zscreen, MOVE ) ;

	if ( PARM != "" )
	{
		comm1 = get_tempname( "output" ) ;
		comm2 = get_tempname( "errors" ) ;
		if ( invoke_task_wait( PARM, comm1, comm2 ) )
		{
			vreplace( "ZBRALT", "COMMAND:"+ PARM ) ;
			vput( "ZBRALT", SHARED ) ;
			browse( comm1 ) ;
			if ( ZRC == 4 && ZRSN == 4 ) { browse( comm2 ) ; }
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
		display( "PCMD0A", msg, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		if ( zcmd != "" )
		{
			comm1 = get_tempname( "output" ) ;
			comm2 = get_tempname( "errors" ) ;
			if ( isRexx( word( zcmd, 1 ) ) )
			{
				control( "ERRORS", "RETURN" ) ;
				select( "CMD(%" + zcmd + ") LANG(REXX) NEWPOOL" ) ;
				if ( ZRC == 20 && ZRSN == 999 && ZRESULT == "Abended" )
				{
					vreplace( "ZEDSMSG", "Abended" ) ;
					vreplace( "ZEDLMSG", "Command '"+zcmd+ "' has abended" ) ;
					setmsg( "PSYZ001" ) ;
				}
				else
				{
					zcmd = "" ;
				}
				control( "ERRORS", "CANCEL" ) ;
			}
			else
			{
				if ( invoke_task_wait( zcmd, comm1, comm2 ) )
				{
					vreplace( "ZBRALT", "COMMAND:"+ zcmd ) ;
					vput( "ZBRALT", SHARED ) ;
					browse( comm1 ) ;
					if ( ZRC == 4 && ZRSN == 4 ) { browse( comm2 ) ; }
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
	return ;
}


bool PCMD0A::invoke_task_wait( const string& cmd, string& comm1, const string& comm2 )
{
	// Invoke a background task and wait for a reponse.
	// Timeout after 10seconds

	int elapsed ;

	vput( "COMM1 COMM2", SHARED ) ;

	select( "PGM(PCMD0A) PARM("+cmd+") BACK" ) ;

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


void PCMD0A::run_command( string cmd, const string& fname1, const string& fname2 )
{
	// Run command in the background and detach from the terminal (unless pipes are used)
	// in case it hangs or messes with ncurses.

	// Output goes to fname1
	// Errors go to fname2

	string file ;

	char buffer[256] ;

	std::ofstream of ;

	if ( cmd.find( '|' ) == string::npos ) { cmd += " </dev/null 2> "+ fname2 ; }
	else                                   { cmd += " 2> "+ fname2            ; }

	of.open( fname1 ) ;
	FILE* pipe{ popen( cmd.c_str(), "r" ) } ;

	while ( fgets( buffer, sizeof( buffer ), pipe) != nullptr )
	{
		file = buffer ;
		of << file.substr( 0, file.size() - 1 ) << endl ;
	}

	pclose( pipe )  ;
	of.close()      ;
}


bool PCMD0A::isRexx( string orexx )
{
	int i ;
	int j ;

	string rxfile ;
	string paths  ;

	vget( "ZORXPATH", PROFILE ) ;
	vcopy( "ZORXPATH", paths, MOVE ) ;

	while ( true )
	{
		for ( j = getpaths( paths ), i = 1 ; i <= j ; i++ )
		{
			rxfile = getpath( paths, i ) + orexx ;
			if ( !exists( rxfile ) ) { continue ; }
			if ( is_regular_file( rxfile ) )
			{
				return true ;
			}
			return false ;
		}
		if ( orexx == lower( orexx ) ) { break ; }
		ilower( orexx ) ;
	}
	return false ;
}


string PCMD0A::get_tempname( const string& suf )
{
	boost::filesystem::path temp = boost::filesystem::temp_directory_path() /
			boost::filesystem::unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;
	return temp.native() + "." + suf ;
}


// ============================================================================================ //

extern "C" { pApplication* maker() { return new PCMD0A ; } }
extern "C" { void destroy(pApplication* p) { delete p ; } }
