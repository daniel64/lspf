/*  Compile with ::                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPTUTORA.so -o libPTUTORA.so PTUTORA.cpp */

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


/*                                                                                     */
/* Very simple help display.  Need something a lot better                              */

/* Display help members in the order (if they exist):                                                  */
/*                                   Keylist Help (defined in the keylist table)                       */
/*                                   Message Help (defined in the message file, .HELP= on the SM line) */
/*                                   Field Help (defined in the panel file, )HELP section )            */
/*                                   Panel Help (defined in the panel file, .HELP= in )PROC section )  */
/*                                   Application Help (set variable ZAHELP in application program)     */
/*                                   System Help (defined in ZSHELP in lspf.h)                      */

/* Search paths passed by application, adding /help subdirectory                                       */
/* SETMSG issued if a help member cannot be found in the search                                        */

/* Program may be invoked without parameters (as from a selection menu) in which case set PATHS to     */
/* ZPLIB                                                                                               */

#include <boost/filesystem.hpp>
#include <vector>
#include "../lspf.h"
#include "../utilities.h"
#include "../classes.h"
#include "../pVPOOL.h"
#include "../pWidgets.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"
#include "PTUTORA.h"

using namespace std ;
using namespace boost::filesystem ;

#undef  MOD_NAME
#define MOD_NAME PTUTORA

#define E_RED      3
#define E_GREEN    4
#define E_YELLOW   5
#define E_BLUE     6
#define E_MAGENTA  7
#define E_TURQ     8
#define E_WHITE    9

PTUTORA::PTUTORA()
{
	set_appdesc( "Default help/tutorial program for lspf" ) ;
	set_appver( "1.0.0" )  ;
}


void PTUTORA::application()
{
	llog( "I", "Application PTUTORA starting.  Parms are " << PARM << endl ) ;

	int i  ;
	int j  ;
	int k  ;

	string tfile      ;
	string filename   ;
	string panel      ;
	string zdsn       ;
	string zscrname   ;

	bool rebuildZAREA ;

	errblock err      ;

	vdefine( "ZZSTR1 ZSCRNAME", &help, &zscrname ) ;

	zscrname = "Help" ;
	vput( "ZSCRNAME", SHARED ) ;

	sh       = ZSHELP ;
	help     = ZSHELP ;
	helplst  = ZSHELP ;
	helptype = 'S'    ;

	if ( PARM == "" )
	{
		vget( "ZPLIB", PROFILE ) ;
		vcopy( "ZPLIB", ps, MOVE ) ;
	}

	ah = parseString( err, PARM, "A()" ) ;
	if ( ah != "" )
	{
		help = ah ;
		helptype = 'A' ;
		helplst = help + " " + helplst ;
	}

	ph = parseString( err, PARM, "P()" ) ;
	if ( ph != "" )
	{
		help = ph ;
		helptype = 'P' ;
		helplst = help + " " + helplst ;
	}

	fh = parseString( err, PARM, "F()" ) ;
	if ( fh != "" )
	{
		help = fh ;
		helptype = 'F' ;
		helplst = help + " " + helplst ;
	}

	fh = parseString( err, PARM, "M()" ) ;
	if ( mh != "" )
	{
		help = mh ;
		helptype = 'M' ;
		helplst = help + " " + helplst ;
	}

	kh = parseString( err, PARM, "K()" ) ;
	if ( kh != "" )
	{
		help = kh ;
		helptype = 'K' ;
		helplst = help + " " + helplst ;
	}

	ps = parseString( err, PARM, "PATHS()" ) ;

	rebuildZAREA = true  ;

	vdefine( "ZCMD ZVERB ZROW1 ZROW2 ZAREA ZSHADOW ZAREAT ZDSN", &zcmd, &zverb, &zrow1, &zrow2, &zarea, &zshadow, &zareat, &zdsn ) ;
	vdefine( "ZSCROLLN ZAREAW ZAREAD", &zscrolln, &zareaw, &zaread ) ;
	vdefine( "ZSCROLLA ZCOL1", &zscrolla, &zcol1 ) ;

	firstLine = 0 ;
	startCol  = 1 ;
	maxCol    = 1 ;

	pquery( "PTUTORA1", "ZAREA", "ZAREAT", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 )  { abend() ; return ; }

	maxLines = 6 ;
	data.push_back( centre( " Top of Help ", zareaw, '*' ) ) ;
	data.push_back( " System level Help . . . : " + sh ) ;
	data.push_back( " Application level Help. : " + ah ) ;
	data.push_back( " Panel Help. . . . . . . : " + ph ) ;
	data.push_back( " Field Help. . . . . . . : " + fh ) ;
	data.push_back( " Message Help. . . . . . : " + mh ) ;
	data.push_back( " Keylist Help. . . . . . : " + kh ) ;

	j = getpaths( ps ) ;
	for ( i = 1 ; i <= words( helplst ) ; i++ )
	{
		help     = word( helplst, i ) ;
		filename = ""                 ;
		for ( k = 1 ; k <= j ; k++ )
		{
			tfile = getpath( ps, k ) + "help/" + help ;
			if ( exists( tfile ) )
			{
				if ( !is_regular_file( tfile ) )
				{
					llog( "E", "Help file " << tfile << " is not a regular file" << endl ) ;
					return ;
				}
				else
				{
					filename = tfile ;
					break            ;
				}
			}
		}
		if ( filename == "" )
		{
			setmsg( "PSYS011J" ) ;
			return ;
		}
		data.push_back( " " ) ;
		data.push_back( copies( "-", zareaw ) ) ;
		data.push_back( "Help member " + help ) ;
		data.push_back( copies( "-", zareaw ) ) ;
		maxLines = maxLines + 4 ;
		read_file( tfile ) ;
	}

	data.push_back( centre( " Bottom of Help ", zareaw, '*' ) ) ;
	maxLines++ ;

	while ( true )
	{
		zcmd  = "" ;
		zverb = "" ;

		if ( rebuildZAREA ) fill_dynamic_area() ;

		zrow1 = d2ds( firstLine, 8 )    ;
		zrow2 = d2ds( maxLines - 2, 8 ) ;
		zcol1 = d2ds( startCol, 7 )     ;

		display( "PTUTORA1", "", "ZCMD" ) ;
		if ( RC == 8 ) { break ; }

		rebuildZAREA = false ;

		vget( "ZVERB ZSCROLLA ZSCROLLN", SHARED ) ;
		iupper( zcmd ) ;

		if ( zverb == "DOWN" )
		{
			rebuildZAREA = true ;
			if ( zscrolla == "MAX" )
			{
				firstLine = maxLines - zaread ;
			}
			else
			{
				firstLine = firstLine + zscrolln  ;
				if ( firstLine >= maxLines ) { firstLine = maxLines - 1 ; }
			}
		}
		else if ( zverb == "UP" )
		{
			rebuildZAREA = true ;
			if ( zscrolla == "MAX" )
			{
				firstLine = 0 ;
			}
			else
			{
				firstLine = firstLine - zscrolln  ;
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
				if ( startCol < 1 ) { startCol = 1 ; }
			}
		}
		else if ( zverb == "RIGHT" )
		{
			rebuildZAREA = true ;
			if ( zscrolla == "MAX" )
			{
				startCol = maxCol - zareaw ;
				if ( startCol < 1 ) { startCol = 1 ; }
			}
			else
			{
				startCol = startCol + zscrolln ;
			}
		}
		if ( firstLine < 0 ) firstLine = 0 ;
	}
	vput( "ZSCROLL", PROFILE ) ;
	return ;
}


void PTUTORA::read_file( string file )
{
	int j  ;

	size_t pos    ;

	string inLine ;

	std::ifstream fin( file.c_str() ) ;

	if ( !fin.is_open() )
	{
		llog( "E", "Error opening file " << file << " for input" << endl ) ;
		return ;
	}

	while ( getline( fin, inLine ) )
	{
		pos = inLine.find( '\t' ) ;
		while ( pos != string::npos )
		{
			j = 8 - (pos % 8 ) ;
			inLine.replace( pos, 1,  j, ' ' ) ;
			pos = inLine.find( '\t', pos + 1 );
		}
		if ( maxCol < inLine.size() ) maxCol = inLine.size() ;
		data.push_back( inLine ) ;
		maxLines++ ;
	}
	maxCol++   ;
	fin.close() ;
}


void PTUTORA::fill_dynamic_area()
{
	size_t Area ;

	string w ;
	string t1, t2, t3, t4 ;
	string s1g, s1y, s1w, div ;

	s1g = "" ;
	s1y = "" ;
	s1w = "" ;
	div = "" ;
	s1g.resize( zareaw ) ;
	s1y.resize( zareaw ) ;
	s1w.resize( zareaw ) ;
	div.resize( zareaw ) ;
	s1g.replace( 0, zareaw, zareaw, E_GREEN  ) ;
	s1y.replace( 0, zareaw, zareaw, E_YELLOW ) ;
	s1w.replace( 0, zareaw, zareaw, E_WHITE  ) ;
	div.replace( 0, zareaw, zareaw, '-' )      ;

	t3 = "" ;
	t4 = "" ;
	t3.resize( zareaw ) ;
	t4.resize( zareaw ) ;

	Area = zareaw * zaread ;

	zarea   = "" ;
	zshadow = string( Area, E_GREEN ) ;

	for ( uint k = firstLine ; k < (firstLine + zaread) ; k++ )
	{
		if ( k > 0 && k < data.size()-1 )  t1 = substr( data[ k ] , startCol, zareaw ) ;
		else                               t1 = substr( data[ k ] , 1, zareaw ) ;
		for ( uint i = 0 ; i < t1.size() ; i++ )
		{
			if ( !isprint( t1[ i ] ) )
			{
				t1[ i ] = '.' ;
			}
		}
		zarea += t1 ;
		if ( zarea.size() >= Area ) { break ; }
		if ( k == data.size() - 1 ) { break ; }
	}
}


// ============================================================================================ //

extern "C" { pApplication* maker() { return new PTUTORA ; } }
extern "C" void destroy(pApplication* p) { delete p ; }
