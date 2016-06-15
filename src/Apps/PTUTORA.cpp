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
#include "../pWidgets.h"
#include "../pVPOOL.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"
#include "PTUTORA.h"

using namespace std ;
using namespace boost::filesystem ;

#undef LOGOUT
#undef MOD_NAME

#define LOGOUT aplog
#define MOD_NAME PTUTORA



void PTUTORA::application()
{
	log( "I", "Application PTUTORA starting.  Parms are " << PARM << endl ) ;

	int p1 ;
	int p2 ;
	int i  ;
	int j  ;
	int k  ;
	string tfile      ;
	string filename   ;
	bool rebuildZAREA ;
	string panel      ;
	string ZDSN       ;

	vdefine( "ZZSTR1", &help ) ;

	sh       = ZSHELP ;
	help     = ZSHELP ;
	helplst  = ZSHELP ;
	helptype = 'S'    ;

	if ( PARM == "" ) { ps = ZPLIB ; }

	p1 = pos( "A(", PARM ) ;
	if ( p1 > 0 ) { p2 = pos( ")", PARM, p1 ) ; ah = substr( PARM, p1 + 2, p2 - p1 - 2 ) ; }
	if ( ah != "" ) { help = ah ; helptype = 'A' ; helplst = help + " " + helplst ; }

	p1 = pos( "P(", PARM ) ;
	if ( p1 > 0 ) { p2 = pos( ")", PARM, p1 ) ; ph = substr( PARM, p1 + 2, p2 - p1 - 2 ) ; }
	if ( ph != "" ) { help = ph ; helptype = 'P' ; helplst = help + " " + helplst ; }

	p1 = pos( "F(", PARM ) ;
	if ( p1 > 0 ) { p2 = pos( ")", PARM, p1 ) ; fh = substr( PARM, p1 + 2, p2 - p1 - 2 ) ; }
	if ( fh != "" ) { help = fh ; helptype = 'F' ; helplst = help + " " + helplst ; }

	p1 = pos( "M(", PARM ) ;
	if ( p1 > 0 ) { p2 = pos( ")", PARM, p1 ) ; mh = substr( PARM, p1 + 2, p2 - p1 - 2 ) ; }
	if ( mh != "" ) { help = mh ; helptype = 'M' ; helplst = help + " " + helplst ; }

	p1 = pos( "PATHS(", PARM ) ;
	if ( p1 > 0 ) { p2 = pos( ")", PARM, p1 ) ; ps = substr( PARM, p1 + 6, p2 - p1 - 6 ) ; }

	rebuildZAREA = true  ;

	vdefine( "ZCMD ZVERB ZROW1 ZROW2 ZAREA ZSHADOW ZAREAT ZDSN", &ZCMD, &ZVERB, &ZROW1, &ZROW2, &ZAREA, &ZSHADOW, &ZAREAT, &ZDSN ) ;
	vdefine( "ZSCROLLN ZAREAW ZAREAD", &ZSCROLLN, &ZAREAW, &ZAREAD ) ;
	vdefine( "ZSCROLLA ZCOL1", &ZSCROLLA, &ZCOL1 ) ;

	firstLine = 0 ;
	startCol  = 1 ;
	maxCol    = 1 ;

	pquery( "PTUTORA1", "ZAREA", "ZAREAT", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 )  { abend() ; return ; }

	maxLines = 6 ;
	data.push_back( centre( " TOP OF HELP ", ZAREAW, '*' ) ) ;
	data.push_back( " SYSTEM level Help . . . : " + sh ) ;
	data.push_back( " Application level Help. : " + ah ) ;
	data.push_back( " Panel Help. . . . . . . : " + ph ) ;
	data.push_back( " Field Help. . . . . . . : " + fh ) ;
	data.push_back( " Message Help. . . . . . : " + mh ) ;

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
					log( "E", "Help file " << tfile << " is not a regular file" << endl ) ;
					cleanup() ;
					return    ;
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
			setmsg( "PSYS01J" ) ;
			cleanup() ;
			return    ;
		}
		data.push_back( " " ) ;
		data.push_back( copies( "-", ZAREAW ) ) ;
		data.push_back( "Help member " + help ) ;
		data.push_back( copies( "-", ZAREAW ) ) ;
		maxLines = maxLines + 4 ;
		read_file( tfile ) ;
	}

	data.push_back( centre( " BOTTOM OF HELP ", ZAREAW, '*' ) ) ;
	maxLines++ ;

	while ( true )
	{
		ZCMD  = "" ;
		ZVERB = "" ;

		if ( rebuildZAREA ) fill_dynamic_area() ;

		ZROW1 = right( d2ds( firstLine ), 8, '0' )    ;
		ZROW2 = right( d2ds( maxLines - 2 ), 8, '0' ) ;
		ZCOL1 = right( d2ds( startCol ), 7, '0' )     ;

		display( "PTUTORA1", "", "ZCMD" ) ;
		if ( RC > 8 )  { abend()   ; break ; }
		if ( RC == 8 ) { break ;             }

		rebuildZAREA = false ;

		vget( "ZVERB ZSCROLLA ZSCROLLN", SHARED ) ;
		ZCMD = upper( ZCMD ) ;

		if ( ZVERB == "DOWN" )
		{
			rebuildZAREA = true ;
			if ( ZSCROLLA == "MAX" )
			{
				firstLine = maxLines - ZAREAD ;
			}
			else
			{
				firstLine = firstLine + ZSCROLLN  ;
				if ( firstLine >= maxLines ) { firstLine = maxLines - 1 ; }
			}
		}
		else if ( ZVERB == "UP" )
		{
			rebuildZAREA = true ;
			if ( ZSCROLLA == "MAX" )
			{
				firstLine = 0 ;
			}
			else
			{
				firstLine = firstLine - ZSCROLLN  ;
			}
		}
		else if ( ZVERB == "LEFT" )
		{
			rebuildZAREA = true ;
			if ( ZSCROLLA == "MAX" )
			{
				startCol = 1 ;
			}
			else
			{
				startCol = startCol - ZSCROLLN ;
				if ( startCol < 1 ) { startCol = 1 ; }
			}
		}
		else if ( ZVERB == "RIGHT" )
		{
			rebuildZAREA = true ;
			if ( ZSCROLLA == "MAX" )
			{
				startCol = maxCol - ZAREAW ;
				if ( startCol < 1 ) { startCol = 1 ; }
			}
			else
			{
				startCol = startCol + ZSCROLLN ;
			}
		}
		if ( firstLine < 0 ) firstLine = 0 ;
	}
	vput( "ZSCROLL", PROFILE ) ;
	cleanup() ;
	return ;
}


void PTUTORA::read_file( string file )
{
	string inLine ;
	int pos, i, j ;
	std::ifstream fin( file.c_str() ) ;

	if ( !fin.is_open() )
	{
		log( "E", "Error opening file " << file << " for input" << endl ) ;
		return ;
	}

	while ( getline( fin, inLine ) )
	{
		pos = inLine.find_first_of( '\t' ) ;
		while ( pos != string::npos )
		{
			j = 8 - (pos % 8 ) ;
			inLine.replace( pos, 1,  j, ' ' ) ;
			pos = inLine.find_first_of( '\t', pos + 1 );
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
	string w ;
	string t1, t2, t3, t4 ;
	string s1g, s1y, s1w, div ;
	int    i, l, wI, wL, ln   ;
	int    Area ;

	s1g = "" ;
	s1y = "" ;
	s1w = "" ;
	div = "" ;
	s1g.resize( ZAREAW ) ;
	s1y.resize( ZAREAW ) ;
	s1w.resize( ZAREAW ) ;
	div.resize( ZAREAW ) ;
	s1g.replace( 0, ZAREAW, ZAREAW, N_GREEN  ) ;
	s1y.replace( 0, ZAREAW, ZAREAW, N_YELLOW ) ;
	s1w.replace( 0, ZAREAW, ZAREAW, N_WHITE  ) ;
	div.replace( 0, ZAREAW, ZAREAW, '-' )      ;

	t3 = "" ;
	t4 = "" ;
	t3.resize( ZAREAW ) ;
	t4.resize( ZAREAW ) ;
	
	Area = ZAREAW * ZAREAD ;

	ZAREA   = "" ;
	ZSHADOW = string( Area, N_GREEN ) ;

	for( int k = firstLine ; k < (firstLine + ZAREAD) ; k++ )
	{
		if ( k >  0 & k < data.size()-1 )  t1 = substr( data[ k ] , startCol, ZAREAW ) ;
		else                               t1 = substr( data[ k ] , 1, ZAREAW ) ;
		for ( i = 0 ; i < t1.size() ; i++ )
		{
			if ( !isprint( t1[ i ] ) )
			{
				t1[ i ] = '.' ;
			}
		}
		ZAREA   = ZAREA   + t1 ;
		if ( ZAREA.size() >= Area ) break ;
		if ( k == data.size() - 1 ) { break ; } ;
	}
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PTUTORA ; } }
extern "C" void destroy(pApplication *p) { delete p ; }
