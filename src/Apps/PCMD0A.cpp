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
#include "../pWidgets.h"
#include "../pVPOOL.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"

#include "PCMD0A.h"

using namespace std ;
using namespace boost::filesystem ;

#undef  MOD_NAME
#define MOD_NAME PCMD0A


void PCMD0A::application()
{
	llog( "I", "Application PCMD0A starting" << endl ) ;

	ZAPPDESC = "Invoke a command and display the output" ;

	string ZCOMMAND  ;
	string MSG       ;
	string result    ;
	string file      ;
	char buffer[256] ;

	std::ofstream of ;

	vdefine( "ZCOMMAND", &ZCOMMAND )  ;
	vcopy( "ZUSER", ZUSER, MOVE )     ;
	vcopy( "ZSCREEN", ZSCREEN, MOVE ) ;

	boost::filesystem::path temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path( ZUSER + "-" + ZSCREEN + "-%%%%-%%%%" ) ;
	string tname = temp.native() ;

	ZCOMMAND = "" ;
	if ( PARM != "" )
	{
		control( "DISPLAY", "NONDISPL" ) ;
		ZCOMMAND = PARM ;
	}

	while ( true )
	{
		if ( MSG == "" ) { ZCMD = "" ; }
		display( "PCMD0A", MSG, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		if ( ZCOMMAND != "" )
		{
			ZCOMMAND = ZCOMMAND + " 2> /tmp/popen.err" ;
			of.open( tname ) ;
			FILE* pipe{popen(ZCOMMAND.c_str(), "r")};
			while( fgets(buffer, sizeof(buffer), pipe) != nullptr )
			{
				file = buffer;
				result = file.substr(0, file.size() - 1);
				of << result << endl ;
			}
			pclose( pipe )  ;
			ZCOMMAND = ""   ;
			of.close()      ;
			browse( tname ) ;
			if ( ZRC == 4 && ZRSN == 4 ) { browse( "/tmp/popen.err" ) ; }
			remove( tname ) ;
		}
		if ( PARM != "" ) { break ; }
	}
	cleanup() ;
	return    ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PCMD0A ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
