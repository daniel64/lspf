/*  Compile with ::                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libppsp01a.so -o libppsp01a.so ppsp01a.cpp */

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
/* General utilities:                                                       */
/*                                                                          */
/* lspf/Application log                                                     */
/* Edit Entry function                                                      */
/* Browse/View Entry function                                               */
/* PFKEY Settings                                                           */
/* Colour settings                                                          */
/* Global colours                                                           */
/* T0DO list                                                                */
/* SHARED and PROFILE Variable display                                      */
/* LIBDEF status (isplibd)                                                  */
/* Display LIBDEF status and search paths                                   */
/* Display loaded Modules (loaded Dynamic Classes)                          */
/* Display Saved File List                                                  */
/* Keylists                                                                 */
/* Control keys                                                             */
/* RUN an application (default parameters)                                  */
/* Show error screen                                                        */
/* Set a profile variable                                                   */
/* Show output held on the spool                                            */
/* Command Table Utility                                                    */
/* lspf Table Utility                                                       */
/* Mouse actions                                                            */
/*                                                                          */
/****************************************************************************/


#include <iostream>
#include <boost/regex.hpp>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>

#include <fcntl.h>
#include <mntent.h>

#include "../lspfall.h"
#include "ppsp01a.h"

#define N_RED      0x03
#define N_GREEN    0x04
#define N_YELLOW   0x05
#define N_BLUE     0x06
#define N_MAGENTA  0x07
#define N_TURQ     0x08
#define N_WHITE    0x09

using namespace boost ;
using namespace boost::filesystem ;

LSPF_APP_MAKER( ppsp01a )


ppsp01a::ppsp01a()
{
	STANDARD_HEADER( "General utilities to display logs, PF Key settings, variables, etc.", "1.0.4" )

	vdefine( "ZCURINX ZTDTOP ZTDVROWS ZTDSELS ZTDDEPTH", &zcurinx, &ztdtop, &ztdvrows, &ztdsels, &ztddepth ) ;
	vdefine( "ZCMD ZVERB", &zcmd, &zverb ) ;
	vdefine( "ZSCREEND ZSCREENW", &zscreend, &zscreenw ) ;
}


void ppsp01a::application()
{
	int ws ;

	string w1 ;
	string w2 ;
	string wl ;
	string wr ;

	string logloc ;

	ws = words( PARM ) ;

	w1 = upper( word( PARM, 1 ) ) ;
	wl = word( PARM, 2 ) ;
	wr = subword( PARM, 2 ) ;
	w2 = upper( wl ) ;

	vget( "ZSCREEND ZSCREENW" ) ;

	vcopy( "ZUSER", zuser, MOVE ) ;
	vcopy( "ZSCREEN", zscreen, MOVE ) ;

	auto it = utils.find( w1 ) ;
	if ( it == utils.end() )
	{
		setmsg( "PPSP014A" ) ;
		llog( "E", "Invalid parameter passed to PPSP01A: " << PARM << endl ) ;
		uabend() ;
	}

	auto ut = utFormat.find( it->second ) ;
	if ( ut->second.ut_min != -1 && ws < ut->second.ut_min )
	{
		setmsg( "PPSP014B" ) ;
		return ;
	}

	if ( ut->second.ut_max != -1 && ws > ut->second.ut_max )
	{
		setmsg( "PPSP014C" ) ;
		return ;
	}

	switch ( it->second )
	{
	case UT_ALOG:
			vreplace( "LTYPE", "Application" ) ;
			vget( "ZALOG", PROFILE ) ;
			vcopy( "ZALOG", logloc, MOVE ) ;
			show_log( logloc ) ;
			break ;


	case UT_BROWSEE:
			browseEntry( wr ) ;
			break ;


	case UT_CMDS:
			showCommandTables() ;
			break ;


	case UT_CUAATTR:
			cuaattrSettings() ;
			break ;


	case UT_CTLKEYS:
			controlKeys() ;
			break ;


	case UT_CTU:
			commandTableUtils() ;
			break ;


	case UT_DSL:
			dsList( wr ) ;
			break ;


	case UT_DSX:
			dxList( wr ) ;
			break ;


	case UT_EDITEE:
			editEntry( wr ) ;
			break ;


	case UT_GCL:
			globalColours() ;
			break ;


	case UT_GOPTS:
			lspfSettings() ;
			break ;


	case UT_KLIST:
			if ( w2 == "" )
			{
				keylistUtility() ;
			}
			else if ( w2 == "PRIVATE" )
			{
				vreplace( "ZKLPRIV", "Y" ) ;
				vreplace( "ZKLUSE", "Y" ) ;
				vput( "ZKLPRIV ZKLUSE", PROFILE ) ;
			}
			else if ( w2 == "SHARED" )
			{
				vreplace( "ZKLPRIV", "N" ) ;
				vreplace( "ZKLUSE", "Y" ) ;
				vput( "ZKLPRIV ZKLUSE", PROFILE ) ;
			}
			else if ( w2 == "ON" )
			{
				vreplace( "ZKLUSE", "Y" ) ;
				vput( "ZKLUSE", PROFILE ) ;
			}
			else if ( w2 == "OFF" )
			{
				vreplace( "ZKLUSE", "N" ) ;
				vput( "ZKLUSE", PROFILE ) ;
			}
			else
			{
				setmsg( "PPSP013C" ) ;
			}
			break ;



	case UT_KEYL:
			keylistSettings() ;
			break ;


	case UT_KEYS:
			pfkeySettings() ;
			break ;


	case UT_KLISTS:
			keylistTables() ;
			break ;


	case UT_FKA:
			fkaUtility( w2 ) ;
			break ;


	case UT_LIBDEFS:
			libdefStatus( w2 ) ;
			break ;


	case UT_MODS:
			showLoadedClasses() ;
			break ;


	case UT_MOUSE:
			mouseActions() ;
			break ;


	case UT_OUTLIST:
			showHeldOutput() ;
			break ;


	case UT_PATHS:
			showPaths() ;
			break ;


	case UT_PFK:
			pfshowUtility( w2 ) ;
			break ;


	case UT_PSYSER1:
			showErrorScreen1() ;
			break ;


	case UT_PSYSER2:
			showErrorScreen2( w2 ) ;
			break ;


	case UT_PSYSER3:
			showErrorScreen3( w2 ) ;
			break ;


	case UT_RUN:
			runApplication( w2 ) ;
			break ;


	case UT_SAVELST:
			showSavedFileList() ;
			break ;


	case UT_SETVAR:
			vreplace( w2, word( PARM, 3 ) ) ;
			vput( w2, PROFILE ) ;
			break ;


	case UT_SLOG:
			vreplace( "LTYPE", "lspf" ) ;
			vget( "ZSLOG", PROFILE ) ;
			vcopy( "ZSLOG", logloc, MOVE ) ;
			show_log( logloc ) ;
			break ;


	case UT_SPKEYS:
			specialKeys() ;
			break ;


	case UT_TBU:
			tableUtility() ;
			break ;


	case UT_TODO:
			todoList() ;
			break ;


	case UT_UTPGMS:
			utilityPrograms() ;
			break ;


	case UT_VARS:
			poolVariables( w2 ) ;
			break ;

	case UT_ZEXPAND:
			editString( wr ) ;
			break ;
	}
}


/**************************************************************************************************************/
/**********************************               LOG UTILITY               ***********************************/
/**************************************************************************************************************/

void ppsp01a::show_log( const string& fileName )
{
	int fsize ;
	uint t ;

	colsOn = false ;

	bool rebuildZAREA ;

	const string vlist1 = "ZROW1 ZROW2 ZAREA ZSHADOW ZSCROLLA" ;
	const string vlist2 = "ZSCROLLN ZAREAW ZAREAD ZLVLINE" ;
	const string vlist3 = "ZCOL1" ;

	vdefine( vlist1, &zrow1, &zrow2, &zarea, &zshadow, &zscrolla ) ;
	vdefine( vlist2, &zscrolln, &zareaw, &zaread, &zlvline ) ;
	vdefine( vlist3, &zcol1 ) ;

	string Rest  ;
	string Restu ;
	string msg   ;
	string w1    ;
	string w2    ;
	string w3    ;
	string ffilter ;

	filteri = ' '  ;
	filterx = ' '  ;
	ffilter = ""   ;
	firstLine = 0  ;
	maxCol    = 1  ;

	vget( "ZSCRMAXW ZSCRMAXD", SHARED ) ;
	pquery( "PPSP01AL", "ZAREA", "", "ZAREAW", "ZAREAD" ) ;

	vreplace( "LOGLOC", fileName ) ;

	zasize   = zareaw * zaread ;
	startCol = 48 ;
	task     = 0 ;
	showDate = false ;
	showTime = true ;
	showMod  = true ;
	showTask = true ;

	read_file( fileName ) ;
	fill_dynamic_area() ;
	rebuildZAREA = false ;
	Xon          = false ;
	msg          = "" ;

	fsize = 0  ;
	file_has_changed( fileName, fsize ) ;

	while ( true )
	{
		zcol1 = d2ds( startCol-47, 5 ) ;
		zrow1 = d2ds( firstLine, 8 )   ;
		zrow2 = d2ds( maxLines, 8 )    ;
		if ( msg == "" ) { zcmd = "" ; }

		display( "PPSP01AL", msg, "ZCMD" ) ;
		if ( RC == 8 ) { return ; }
		if ( is_term_resized( zscreend+2, zscreenw ) )
		{
			term_resize() ;
			read_file( fileName ) ;
			fill_dynamic_area() ;
			continue ;
		}
		vget( "ZVERB ZSCROLLA ZSCROLLN", SHARED ) ;

		msg   = "" ;
		Restu = subword( zcmd, 2 ) ;
		iupper( zcmd ) ;
		w1    = word( zcmd, 1 ) ;
		w2    = word( zcmd, 2 ) ;
		w3    = word( zcmd, 3 ) ;
		Rest  = subword( zcmd, 2 ) ;

		if ( file_has_changed( fileName, fsize ) )
		{
			read_file( fileName ) ;
			if ( ffilter != "" ) { find_lines( ffilter ) ; }
			rebuildZAREA = true ;
			set_excludes() ;
		}

		if ( w1 == "" )
		{
			;
		}
		else if ( w1 == "ONLY" || w1 == "O" )
		{
			if ( Rest.size() == 1 )
			{
				filteri = Rest[ 0 ] ;
				set_excludes() ;
				rebuildZAREA = true ;
			}
			else { msg = "PPSP012" ; }
		}
		else if ( w1 == "MOD" )
		{
			if ( w2 != "" )
			{
				mod = w2 ;
				excluded.clear() ;
				for ( t = 0 ; t <= maxLines ; ++t ) { excluded.push_back( false ) ; }
				set_excludes() ;
				rebuildZAREA = true ;
			}
			else { msg = "PPSP012" ; }
		}
		else if ( findword( w1, "TASK TASKID ID" ) )
		{
			if ( w2 == "" )
			{
				task = 0 ;
			}
			else
			{
				task = ds2d ( w2 ) ;
			}
			excluded.clear() ;
			for ( t = 0 ; t <= maxLines ; ++t ) { excluded.push_back( false ) ; }
			set_excludes() ;
			rebuildZAREA = true ;
		}
		else if ( w1 == "EX" || w1 == "X" )
		{
			if ( Rest == "ALL" )
			{
				exclude_all()   ;
				rebuildZAREA = true ;
			}
			else if ( Rest == "ON" )
			{
				Xon = true     ;
				set_excludes() ;
				rebuildZAREA = true ;
			}
			else if ( Rest.size() == 1 )
			{
				filterx = Rest[ 0 ] ;
				set_excludes() ;
				rebuildZAREA = true ;
			}
			else { msg = "PPSP013" ; }
		}
		else if ( w1 == "F" || w1 == "FIND" )
		{
			  ffilter = Restu       ;
			  find_lines( ffilter ) ;
			  rebuildZAREA = true   ;
		}
		else if ( abbrev( "RESET", w1 ) )
		{
			filteri = ' '    ;
			filterx = ' '    ;
			ffilter = ""     ;
			mod     = ""     ;
			Xon     = false  ;
			task    = 0      ;
			rebuildZAREA = true ;
			excluded.clear() ;
			for ( t = 0 ; t <= maxLines ; ++t ) { excluded.push_back( false ) ; }
		}
		else if ( abbrev( "REFRESH", w1 ) )
		{
			read_file( fileName ) ;
			rebuildZAREA = true ;
			set_excludes() ;
		}
		else if ( w1 == "SHOW" )
		{
			if      ( w2 == "DATE" ) { showDate = true ; }
			else if ( w2 == "TIME" ) { showTime = true ; }
			else if ( w2 == "MOD"  ) { showMod  = true ; }
			else if ( w2 == "TASK" ) { showTask = true ; }
			else if ( w2 == "ALL"  ) { showDate = true ; showTime = true ; showMod = true ; showTask = true ; }
			rebuildZAREA = true ;
		}
		else if ( w1 == "HIDE" )
		{
			if      ( w2 == "DATE" ) { showDate = false ; }
			else if ( w2 == "TIME" ) { showTime = false ; }
			else if ( w2 == "MOD"  ) { showMod  = false ; }
			else if ( w2 == "TASK" ) { showTask = false ; }
			else if ( w2 == "ALL"  ) { showDate = false ; showTime = false ; showMod = false ; showTask = false ; }
			rebuildZAREA = true ;
		}
		else if ( w1 == "COLS" )
		{
			colsOn = !colsOn ;
			rebuildZAREA = true ;
		}
		else
		{
			msg = "PPSP011" ;
			continue ;
		}

		if ( zverb == "DOWN" )
		{
			rebuildZAREA = true ;
			if ( zscrolla == "MAX" )
			{
				firstLine = maxLines - zlvline ;
				if ( firstLine < 0 ) firstLine = 0 ;
			}
			else
			{
				t = 0 ;
				for ( ; firstLine < ( maxLines - 1 ) ; ++firstLine )
				{
					if ( excluded[ firstLine ] ) continue ;
					++t ;
					if ( t > zscrolln ) break ;
				}
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
				t = 0 ;
				for ( ; firstLine > 0 ; --firstLine )
				{
					if ( excluded[ firstLine ] ) continue ;
					++t ;
					if ( t > zscrolln ) break ;
				}
			}
		}
		else if ( zverb == "LEFT" )
		{
			rebuildZAREA = true ;
			if ( zscrolla == "MAX" )
			{
				startCol = 48 ;
			}
			else
			{
				startCol = startCol - zscrolln ;
				if ( startCol < 48 ) { startCol = 48 ; }
			}
		}
		else if ( zverb == "RIGHT" )
		{
			rebuildZAREA = true ;
			if ( zscrolla == "MAX" )
			{
				startCol = maxCol - zareaw + 41 ;
			}
			else
			{
				if ( zscrolla == "CSR" )
				{
					if ( zscrolln == lprefix )
					{
						startCol += zareaw - lprefix ;
					}
					else
					{
						startCol += zscrolln - lprefix ;
					}
				}
				else
				{
					startCol += zscrolln ;
				}
			}
			if ( startCol < 48 ) { startCol = 48 ; }
		}

		if ( rebuildZAREA ) fill_dynamic_area() ;
		rebuildZAREA = false ;
	}

	vdelete( vlist1, vlist2, vlist3 ) ;
}


void ppsp01a::read_file( const string& fileName )
{
	string t ;
	string inLine ;

	std::ifstream fin( fileName.c_str() ) ;

	data.clear() ;

	data.push_back( centre( " Top of Log ", zareaw, '*' ) ) ;
	excluded.push_back( false ) ;

	while ( getline( fin, inLine ) )
	{
		if ( inLine.size() < 47 || ( inLine[ 4 ] != '-' && inLine[ 8 ] != '-' ) )
		{
			for ( uint i = data.size() - 1 ; i > 0 ; --i )
			{
				if ( data[ i ].size() < 47 || ( data[ i ][ 4 ] != '-' && data[ i ][ 8 ] != '-' ) ) { continue ; }
				t = data[ i ] ;
				break ;
			}
			inLine = t.substr( 0, 47 ) + inLine ;
		}
		data.push_back( inLine )  ;
		excluded.push_back( false ) ;
		if ( maxCol < inLine.size() ) { maxCol = inLine.size() ; }
	}

	++maxCol ;
	data.push_back( centre( " Bottom of Log ", zareaw, '*' ) ) ;
	excluded.push_back( false ) ;
	maxLines = data.size() ;

	fin.close() ;
}


bool ppsp01a::file_has_changed( const string& fileName,
				int& fsize )
{
	int temp = fsize ;

	struct stat results ;

	lstat( fileName.c_str(), &results ) ;

	fsize = results.st_size ;

	return ( temp != fsize ) ;
}


void ppsp01a::set_excludes()
{
	int j ;
	char c ;

	for ( uint i = 1 ; i < maxLines - 1 ; ++i )
	{
		const string& ln = data[ i ] ;
		if ( task > 0 )
		{
			j = ds2d( ln.substr( 39, 5 ) ) ;
			if ( j != 0 && j != task )
			{
				excluded[ i ] = true ;
				continue ;
			}
		}
		if ( Xon )
		{
			c = ln[ 45 ] ;
			if ( c == 'D' ||
			     c == 'I' ||
			     c == 'T' ||
			     c == '-' )
			{
				excluded[ i ] = true ;
				continue ;
			}
		}
		if ( ln.size() > 45 )
		{
			c = ln[ 45 ] ;
			if ( filteri != ' ' && c != filteri && c != '*' )
			{
				excluded[ i ] = true ;
				continue ;
			}
			if ( filterx != ' ' && c == filterx && c != '*' )
			{
				excluded[ i ] = true ;
				continue ;
			}
			if ( mod != "" && ln.compare( 28, mod.size(), mod ) != 0 )
			{
				excluded[ i ] = true ;
				continue ;
			}
		}
	}

}


void ppsp01a::exclude_all()
{
	for ( uint i = 1 ; i < ( maxLines - 1 ) ; ++i )
	{
		excluded[ i ] = true ;
	}
}


void ppsp01a::find_lines( string fnd )
{
	iupper( fnd ) ;
	for ( uint i = 1 ; i < ( maxLines - 1 ) ; ++i )
	{
		if ( upper( data[ i ] ).find( fnd ) == string::npos )
		{
			excluded[ i ] = true ;
		}
	}
}


void ppsp01a::fill_dynamic_area()
{
	int p ;

	string c ;
	string s ;
	string t ;
	string s2( zareaw, N_TURQ ) ;

	zarea   = "" ;
	zshadow = "" ;
	zarea.reserve( zasize )   ;
	zshadow.reserve( zasize ) ;

	int l = 0 ;

	if ( colsOn )
	{
		zarea   = string( zareaw, ' ' ) ;
		zshadow = string( zareaw, N_WHITE ) ;
		++l ;
	}

	for ( unsigned int k = firstLine ; k < data.size() ; ++k )
	{
		if ( excluded[ k ] ) { continue ; }
		if ( ++l > zaread ) break ;
		if ( k == 0 || k == maxLines-1 )
		{
			zarea   += substr( data[ k ], 1, zareaw ) ;
			zshadow += s2 ;
		}
		else
		{
			s = string( zareaw, N_GREEN ) ;
			t = "" ;
			p = 0 ;
			if ( showDate )
			{
				t = data[ k ].substr( 0,  12 ) ;
				s.replace( p, 12, 12, N_TURQ ) ;
				p = 12 ;
			}
			if ( showTime )
			{
				t += data[ k ].substr( 12, 16 ) ;
				s.replace( p, 16, 16, N_TURQ ) ;
				p += 16 ;
			}
			if ( showMod  )
			{
				t += data[ k ].substr( 28, 11 ) ;
				s.replace( p, 11, 11, N_YELLOW ) ;
				p += 11 ;
			}
			t += data[ k ].substr( 39, 8 ) ;
			t += substr( data[ k ], startCol, zareaw ) ;
			t.resize( zareaw, ' ' ) ;
			zarea += t ;
			s.replace( p, 5, 5, N_WHITE ) ;
			p += 6 ;
			s.replace( p, 2, 2, N_TURQ  ) ;
			zshadow += s ;
			lprefix  = p + 2 ;
		}
	}

	if ( colsOn )
	{
		c = getColumnLine( startCol - 47 ) ;
		zarea.replace( lprefix, ( zareaw - lprefix ), c.substr( 0, ( zareaw - lprefix ) ) ) ;
	}

	zarea.resize( zasize, ' ' ) ;
	zshadow.resize( zasize, N_TURQ ) ;
}


void ppsp01a::term_resize()
{
	//
	// Get/set new size of variables after a terminal resize.
	//

	const string vlist1 = "ZSCREEND ZSCREENW" ;

	vget( vlist1, SHARED ) ;

	pquery( "PPSP01AL", "ZAREA", "", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 )
	{
		uabend() ;
	}

	zasize = zareaw * zaread ;
}


string ppsp01a::getColumnLine( int s )
{
	//
	// Create column line.
	//

	int i ;
	int j ;
	int k ;
	int l ;

	string t = "" ;
	const string ruler = "----+----" ;

	l = zareaw ;

	i = s % 10 ;
	j = s / 10 ;

	if ( i > 0 )
	{
		++j ;
		t = substr( ruler, i, 10-i ) ;
	}

	for ( k = 0 ; k <= l/10 ; ++k, ++j )
	{
		t += d2ds( j%10 ) + ruler ;
	}

	return t.substr( 0, l ) ;
}


/**************************************************************************************************************/
/**********************************              DSLIST UTILITY             ***********************************/
/**************************************************************************************************************/

void ppsp01a::dsList( string parms )
{
	//
	// If no parms passed, show the List of Personal File Lists screen.
	// If parm start with a /, assume passed parm is a path or file to be listed.
	// If parm is a Personal File List, get list of files and invoke PFLSTPGM with LIST option.
	//    Next parm may be a filter for the file list.
	//

	int i ;

	bool found = false ;

	string* str  ;

	string rlist ;
	string fname ;
	string tname ;
	string filter ;
	string rtable ;

	std::ofstream fout ;

	vector<string> entries ;

	set<string> exists ;

	if ( parms == "" )
	{
		select( "PGM("+ get_dialogue_var( "ZRFLPGM" ) +") PARM(PL3) SCRNAME(DSLIST) SUSPEND" ) ;
		return ;
	}

	if ( parms.front() == '/' )
	{
		fname = word( parms, 1 ) ;
		try
		{
			if ( !is_directory( fname ) )
			{
				setmsg( "PPSP011K" ) ;
				return ;
			}
		}
		catch (...)
		{
			setmsg( "PPSP011L" ) ;
			return ;
		}
		select( "PGM("+ get_dialogue_var( "ZFLSTPGM" ) +") PARM("+ parms +") SCRNAME(FILES) SUSPEND" ) ;
		return ;
	}

	if ( words( parms ) > 1 )
	{
		rlist  = upper( word( parms, 1 ) ) ;
		filter = subword( parms, 2 ) ;
		if ( filter.front() != '*' )
		{
			filter = "*" + filter ;
		}
	}
	else
	{
		rlist = upper( parms ) ;
	}

	vcopy( "ZRFLTBL", rtable, MOVE ) ;
	tbopen( rtable, NOWRITE, "ZUPROF" ) ;
	if ( RC == 0 )
	{
		vreplace( "ZCURTB", rlist ) ;
		tbget( rtable ) ;
		found = ( RC == 0 ) ;
		if ( RC == 8 )
		{
			tbsort( rtable, "(ZCURTB,C,A)" ) ;
			tbvclear( rtable ) ;
			vreplace( "ZCURTB", rlist + "*" ) ;
			tbsarg( rtable, "", "NEXT", "(ZCURTB,EQ)" ) ;
			tbscan( rtable ) ;
			found = ( RC == 0 ) ;
			vcopy( "ZCURTB", rlist, MOVE ) ;
		}
	}
	tbend( rtable ) ;
	if ( !found )
	{
		setmsg( "PPSP011I" ) ;
		return ;
	}

	for ( i = 1 ; i <= 30 ; ++i )
	{
		vcopy( "FLAPET" + d2ds( i, 2 ), str ) ;
		if ( str && !str->empty() && exists.find( *str ) == exists.end() )
		{
			entries.push_back( *str ) ;
			exists.insert( *str ) ;
		}
	}

	tname = get_tempname() ;
	fout.open( tname ) ;
	for ( const auto& entry : entries )
	{
		fout << entry << endl ;
	}
	fout.close() ;

	select( "PGM("+ get_dialogue_var( "ZFLSTPGM" ) +") PARM(LIST "
		      + rlist + " "
		      + tname + " "
		      + filter + ")" ) ;
}


/**************************************************************************************************************/
/**********************************              DXLIST UTILITY             ***********************************/
/**************************************************************************************************************/

void ppsp01a::dxList( string parms )
{
	//
	// Create a sorted list of all files that match parms in the Personal File List passed or treat as
	// a directory if it starts with a '/'.
	//
	// First  parameter: Personal File List (or partial entry) or directory.
	// Second parameter: Pattern to match.
	//

	size_t i ;
	size_t p ;

	bool found = false ;
	bool error = false ;

	string rlist ;
	string temp  ;
	string filter ;
	string rtable ;

	string fname ;

	set<string> exists ;

	vector<string> files   ;
	vector<string> entries ;

	std::ofstream fout ;

	boost::system::error_code ec ;

	if ( parms == "" )
	{
		select( "PGM("+ get_dialogue_var( "ZRFLPGM" ) +") PARM(PL4) SCRNAME(DSLIST) SUSPEND" ) ;
		return ;
	}

	rlist  = word( parms, 1 ) ;
	filter = word( parms, 2 ) ;
	if ( filter != "" && filter.front() != '*' )
	{
		filter = "*" + filter ;
	}

	if ( parms.front() == '/' )
	{
		files.push_back( word( parms, 1 ) ) ;
	}
	else
	{
		iupper( rlist ) ;
		vcopy( "ZRFLTBL", rtable, MOVE ) ;
		tbopen( rtable, NOWRITE, "ZUPROF" ) ;
		if ( RC > 0 )
		{
			setmsg( "PPSP011I" ) ;
			return ;
		}
		vreplace( "ZCURTB", rlist ) ;
		tbget( rtable ) ;
		found = ( RC == 0 ) ;
		if ( RC == 8 )
		{
			tbsort( rtable, "(ZCURTB,C,A)" ) ;
			tbvclear( rtable ) ;
			vreplace( "ZCURTB", rlist + "*" ) ;
			tbsarg( rtable, "", "NEXT", "(ZCURTB,EQ)" ) ;
			tbscan( rtable ) ;
			found = ( RC == 0 ) ;
			vcopy( "ZCURTB", parms, MOVE ) ;
		}
		tbend( rtable ) ;
		if ( !found )
		{
			setmsg( "PPSP011I" ) ;
			return ;
		}
		for ( i = 1 ; i <= 30 ; ++i )
		{
			vcopy( "FLAPET" + d2ds( i, 2 ), fname, MOVE ) ;
			if ( fname != "" )
			{
				files.push_back( fname ) ;
			}
		}
	}

	for ( i = 0 ; i < files.size() ; ++i )
	{
		string& fname = files[ i ] ;
		try
		{
			if ( is_regular_file( fname ) && exists.find( fname ) == exists.end() )
			{
				entries.push_back( fname ) ;
				exists.insert( fname ) ;
				continue ;
			}
			if ( !is_directory( fname ) )
			{
				p = fname.find_last_of( '/' ) ;
				if ( p != string::npos && fname.find_first_of( "?*[", p ) != string::npos )
				{
					fname = fname.substr( 0, p ) ;
					if ( !is_directory( fname ) )
					{
						continue ;
					}
				}
				else
				{
					continue ;
				}
			}
		}
		catch (...)
		{
			error = true ;
			continue ;
		}
		if ( exists.find( fname ) != exists.end() )
		{
			continue ;
		}
		exists.insert( fname ) ;
		recursive_directory_iterator eIt ;
		recursive_directory_iterator dIt( fname, ec ) ;
		interrupted = false ;
		while ( dIt != eIt )
		{
			if ( interrupted )
			{
				setmsg( "PPSP011N" ) ;
				break ;
			}
			path current( *dIt ) ;
			if ( access( current.c_str(), R_OK ) != 0 )
			{
				dIt.disable_recursion_pending() ;
				dIt.increment( ec ) ;
				error = true ;
				continue;
			}
			if ( is_regular_file( current ) || is_directory( current ) )
			{
				temp = current.string() ;
				if ( exists.find( temp ) == exists.end() )
				{
					entries.push_back( temp ) ;
					exists.insert( temp ) ;
				}
			}
			dIt.increment( ec ) ;
			if ( ec )
			{
				error = true ;
			}
		}
	}

	sort( entries.begin(), entries.end() ) ;

	string tname = get_tempname() ;
	fout.open( tname ) ;

	for ( const auto& entry : entries )
	{
		fout << entry << endl ;
	}

	fout.close() ;

	if ( error )
	{
		setmsg( "PPSP011J" ) ;
	}

	select( "PGM(" + get_dialogue_var( "ZFLSTPGM" ) + ") PARM(LIST "
		       + rlist + " "
		       + tname + " "
		       + filter + ")" ) ;
}


/**************************************************************************************************************/
/**********************************              lspf SETTTINGS             ***********************************/
/**************************************************************************************************************/

void ppsp01a::lspfSettings()
{
	const string nulls = string( 1, 0x00 ) ;

	string godefm  ;
	string godel   ;
	string goswap  ;
	string goswapc ;
	string gokluse ;
	string gonotfy ;
	string golmsgw ;
	string gopadc  ;
	string gosretp ;
	string gofhupd ;
	string goscrld ;
	string gohigh  ;
	string godeclr ;
	string godecla ;
	string goedlct ;
	string gohabmn ;
	string gotabps ;

	string rodefm  ;
	string rodel   ;
	string roswap  ;
	string roswapc ;
	string rokluse ;
	string ronotfy ;
	string rolmsgw ;
	string ropadc  ;
	string rosretp ;
	string rofhupd ;
	string roscrld ;
	string rohigh  ;
	string rodeclr ;
	string rodecla ;
	string roedlct ;
	string rohabmn ;
	string rotabps ;

	string zdefm   ;
	string zdel    ;
	string zswap   ;
	string zswapc  ;
	string zkluse  ;
	string znotify ;
	string zlmsgw  ;
	string zpadc   ;
	string zsretp  ;
	string zfhurl  ;
	string zscrolld;
	string zhigh   ;
	string zdeclr  ;
	string zdecla  ;
	string zedlctab;
	string zhiabmn ;
	string ztabpas ;

	string goucmd1 ;
	string goucmd2 ;
	string goucmd3 ;
	string goscmd1 ;
	string goscmd2 ;
	string goscmd3 ;
	string gostfst ;

	string roucmd1 ;
	string roucmd2 ;
	string roucmd3 ;
	string roscmd1 ;
	string roscmd2 ;
	string roscmd3 ;
	string rostfst ;

	string gortsize ;
	string gorbsize ;
	string rortsize ;
	string rorbsize ;
	string zrtsize  ;
	string zrbsize  ;

	string zucmdt1 ;
	string zucmdt2 ;
	string zucmdt3 ;
	string zscmdt1 ;
	string zscmdt2 ;
	string zscmdt3 ;
	string zscmdtf ;

	vdefine( "ZUCMDT1 ZUCMDT2 ZUCMDT3", &zucmdt1, &zucmdt2, &zucmdt3 ) ;
	if ( RC > 0 ) { uabend() ; }
	vdefine( "ZSCMDT1 ZSCMDT2 ZSCMDT3 ZSCMDTF", &zscmdt1, &zscmdt2, &zscmdt3, &zscmdtf ) ;
	if ( RC > 0 ) { uabend() ; }
	vdefine( "GOUCMD1 GOUCMD2 GOUCMD3", &goucmd1, &goucmd2, &goucmd3 ) ;
	if ( RC > 0 ) { uabend() ; }
	vdefine( "GOSCMD1 GOSCMD2 GOSCMD3 GOSTFST", &goscmd1, &goscmd2, &goscmd3, &gostfst ) ;
	if ( RC > 0 ) { uabend() ; }
	vdefine( "ZKLUSE  ZNOTIFY GOKLUSE GONOTFY", &zkluse, &znotify, &gokluse, &gonotfy ) ;
	if ( RC > 0 ) { uabend() ; }
	vdefine( "ZDEL    GODEL   ZLMSGW  GOLMSGW", &zdel, &godel, &zlmsgw, &golmsgw ) ;
	if ( RC > 0 ) { uabend() ; }
	vdefine( "ZSWAP   GOSWAP   ZSWAPC  GOSWAPC",  &zswap, &goswap, &zswapc, &goswapc ) ;
	if ( RC > 0 ) { uabend() ; }
	vdefine( "ZRTSIZE GORTSIZE ZRBSIZE GORBSIZE", &zrtsize, &gortsize, &zrbsize, &gorbsize ) ;
	if ( RC > 0 ) { uabend() ; }
	vdefine( "ZPADC GOPADC", &zpadc, &gopadc ) ;
	if ( RC > 0 ) { uabend() ; }
	vdefine( "ZSRETP GOSRETP ZFHURL GOFHUPD", &zsretp, &gosretp, &zfhurl, &gofhupd ) ;
	if ( RC > 0 ) { uabend() ; }
	vdefine( "GOSCRLD ZSCROLLD", &goscrld, &zscrolld ) ;
	if ( RC > 0 ) { uabend() ; }
	vdefine( "GODEFM ZDEFM", &godefm, &zdefm ) ;
	if ( RC > 0 ) { uabend() ; }
	vdefine( "GOHIGH ZHIGH GODECLR ZDECLR GODECLA ZDECLA", &gohigh, &zhigh, &godeclr, &zdeclr, &godecla, &zdecla ) ;
	if ( RC > 0 ) { uabend() ; }
	vdefine( "GOEDLCT ZEDLCTAB", &goedlct, &zedlctab ) ;
	if ( RC > 0 ) { uabend() ; }
	vdefine( "GOHABMN ZHIABMN", &gohabmn, &zhiabmn ) ;
	if ( RC > 0 ) { uabend() ; }
	vdefine( "GOTABPS ZTABPAS", &gotabps, &ztabpas ) ;
	if ( RC > 0 ) { uabend() ; }

	vget( "ZUCMDT1 ZUCMDT2 ZUCMDT3", PROFILE ) ;
	if ( RC > 0 ) { uabend() ; }
	vget( "ZSCMDT1 ZSCMDT2 ZSCMDT3 ZSCMDTF", PROFILE ) ;
	if ( RC > 0 ) { uabend() ; }
	vget( "ZSWAP ZSWAPC ZKLUSE ZNOTIFY ZSCROLLD", PROFILE ) ;
	if ( RC > 0 ) { uabend() ; }
	vget( "ZDEFM ZDEL ZKLUSE ZLMSGW ZPADC ZSRETP ZFHURL ZHIGH ZDECLR ZDECLA ZEDLCTAB", PROFILE ) ;
	if ( RC > 0 ) { uabend() ; }
	vget( "ZRTSIZE ZRBSIZE ZHIABMN ZTABPAS", PROFILE ) ;
	if ( RC > 0 ) { uabend() ; }

	gokluse = ( zkluse  == "Y" ) ? "/" : "" ;
	gonotfy = ( znotify == "Y" ) ? "/" : "" ;
	gostfst = ( zscmdtf == "Y" ) ? "/" : "" ;
	golmsgw = ( zlmsgw  == "Y" ) ? "/" : "" ;
	goswap  = ( zswap   == "Y" ) ? "/" : "" ;
	gosretp = ( zsretp  == "Y" ) ? "/" : "" ;
	gofhupd = ( zfhurl  == "Y" ) ? "/" : "" ;
	gohigh  = ( zhigh   == "Y" ) ? "/" : "" ;
	godeclr = ( zdeclr  == "Y" ) ? "/" : "" ;
	godecla = ( zdecla  == "Y" ) ? "/" : "" ;
	gohabmn = ( zhiabmn == "Y" ) ? "/" : "" ;
	gotabps = ( ztabpas == "Y" ) ? "/" : "" ;
	godefm  = ( zdefm   == "Y" ) ? "1" : "2" ;

	godel   = zdel   ;
	goswapc = zswapc ;
	if      ( zpadc == " "   ) { zpadc = "B" ; }
	else if ( zpadc == nulls ) { zpadc = "N" ; }
	gopadc  = zpadc    ;
	goscrld = zscrolld ;
	goedlct = zedlctab ;

	goucmd1 = zucmdt1 ;
	goucmd2 = zucmdt2 ;
	goucmd3 = zucmdt3 ;
	goscmd1 = zscmdt1 ;
	goscmd2 = zscmdt2 ;
	goscmd3 = zscmdt3 ;

	gortsize = zrtsize ;
	gorbsize = zrbsize ;

	rodefm   = godefm   ;
	rodel    = godel    ;
	roswap   = goswap   ;
	roswapc  = goswapc  ;
	rokluse  = gokluse  ;
	ronotfy  = gonotfy  ;
	rolmsgw  = golmsgw  ;
	ropadc   = gopadc   ;
	roscrld  = goscrld  ;
	rosretp  = gosretp  ;
	rofhupd  = gofhupd  ;
	roucmd1  = goucmd1  ;
	roucmd2  = goucmd2  ;
	roucmd3  = goucmd3  ;
	roscmd1  = goscmd1  ;
	roscmd2  = goscmd2  ;
	roscmd3  = goscmd3  ;
	rostfst  = gostfst  ;
	rortsize = gortsize ;
	rorbsize = gorbsize ;
	rohigh   = gohigh   ;
	rodeclr  = godeclr  ;
	rodecla  = godecla  ;
	roedlct  = goedlct  ;
	rohabmn  = gohabmn  ;
	rotabps  = gotabps  ;

	zcmd = "" ;
	while ( zcmd != "CANCEL" )
	{
		zcmd = "" ;
		display( "PPSP01GO", "", "ZCMD" ) ;
		if ( RC == 8 && zcmd != "CANCEL" )
		{
			break ;
		}
		if ( zcmd == "DEFAULTS" )
		{
			gokluse  = ""  ;
			gonotfy  = "/" ;
			gostfst  = "/" ;
			golmsgw  = ""  ;
			godefm   = "2" ;
			godel    = ";" ;
			goswap   = "/" ;
			goswapc  = "'" ;
			gopadc   = "_" ;
			goscrld  = "HALF" ;
			gosretp  = ""  ;
			gofhupd  = "/" ;
			goucmd1  = "USR"  ;
			goucmd2  = ""  ;
			goucmd3  = ""  ;
			goscmd1  = ""  ;
			goscmd2  = ""  ;
			goscmd3  = ""  ;
			gortsize = "3"  ;
			gorbsize = "20" ;
			gohigh   = ""   ;
			godeclr  = "/"  ;
			godecla  = ""   ;
			gohabmn  = ""   ;
			gotabps  = ""   ;
			goedlct  = EDLCTAB ;
		}
		else if ( zcmd == "RESET" || zcmd == "CANCEL" )
		{
			godefm   = rodefm   ;
			godel    = rodel    ;
			goswap   = roswap   ;
			goswapc  = roswapc  ;
			gokluse  = rokluse  ;
			gonotfy  = ronotfy  ;
			golmsgw  = rolmsgw  ;
			gopadc   = ropadc   ;
			goscrld  = roscrld  ;
			gosretp  = rosretp  ;
			gofhupd  = rofhupd  ;
			goucmd1  = roucmd1  ;
			goucmd2  = roucmd2  ;
			goucmd3  = roucmd3  ;
			goscmd1  = roscmd1  ;
			goscmd2  = roscmd2  ;
			goscmd3  = roscmd3  ;
			gostfst  = rostfst  ;
			gortsize = rortsize ;
			gorbsize = rorbsize ;
			gohigh   = rohigh   ;
			godeclr  = rodeclr  ;
			godecla  = rodecla  ;
			goedlct  = roedlct  ;
			gohabmn  = rohabmn  ;
			gotabps  = rotabps  ;
		}
		zkluse   = ( gokluse == "/" ) ? "Y" : "N" ;
		znotify  = ( gonotfy == "/" ) ? "Y" : "N" ;
		zscmdtf  = ( gostfst == "/" ) ? "Y" : "N" ;
		zlmsgw   = ( golmsgw == "/" ) ? "Y" : "N" ;
		zswap    = ( goswap  == "/" ) ? "Y" : "N" ;
		zsretp   = ( gosretp == "/" ) ? "Y" : "N" ;
		zfhurl   = ( gofhupd == "/" ) ? "Y" : "N" ;
		zhigh    = ( gohigh  == "/" ) ? "Y" : "N" ;
		zdeclr   = ( godeclr == "/" ) ? "Y" : "N" ;
		zdecla   = ( godecla == "/" ) ? "Y" : "N" ;
		zhiabmn  = ( gohabmn == "/" ) ? "Y" : "N" ;
		ztabpas  = ( gotabps == "/" ) ? "Y" : "N" ;
		zucmdt1  = goucmd1 ;
		zucmdt2  = goucmd2 ;
		zucmdt3  = goucmd3 ;
		zscmdt1  = goscmd1 ;
		zscmdt2  = goscmd2 ;
		zscmdt3  = goscmd3 ;
		zscrolld = goscrld ;
		zedlctab = goedlct ;
		vput( "ZKLUSE  ZNOTIFY ZLMSGW  ZSWAP ZSRETP ZFHURL ZHIGH ZDECLR ZDECLA", PROFILE ) ;
		if ( RC > 0 ) { uabend() ; }
		vput( "ZUCMDT1 ZUCMDT2 ZUCMDT3", PROFILE ) ;
		if ( RC > 0 ) { uabend() ; }
		vput( "ZSCMDT1 ZSCMDT2 ZSCMDT3 ZSCMDTF ZSCROLLD ZEDLCTAB", PROFILE ) ;
		if ( RC > 0 ) { uabend() ; }
		vput( "ZHIABMN ZTABPAS", PROFILE ) ;
		if ( RC > 0 ) { uabend() ; }
		if ( godefm != "" )
		{
			zdefm = ( godefm == "1" ) ? "Y" : "N" ;
			vput( "ZDEFM", PROFILE ) ;
			if ( RC > 0 ) { uabend() ; }
		}
		if ( godel != "" && godel != zdel )
		{
			zdel = godel ;
			vput( "ZDEL", PROFILE ) ;
			if ( RC > 0 ) { uabend() ; }
		}
		if ( goswapc != "" && goswapc != zswapc )
		{
			zswapc = goswapc ;
			vput( "ZSWAPC", PROFILE ) ;
			if ( RC > 0 ) { uabend() ; }
		}
		if ( gopadc != zpadc )
		{
			if      ( gopadc == "B" )  { zpadc = " "    ; }
			else if ( gopadc == "N" )  { zpadc = nulls  ; }
			else                       { zpadc = gopadc ; }
			vput( "ZPADC", PROFILE ) ;
			if ( RC > 0 ) { uabend() ; }
			zpadc = gopadc ;
		}
		if ( gortsize != "" )
		{
			zrtsize = gortsize ;
			vput( "ZRTSIZE", PROFILE ) ;
			if ( RC > 0 ) { uabend() ; }
		}
		if ( gorbsize != "" )
		{
			zrbsize = gorbsize ;
			vput( "ZRBSIZE", PROFILE ) ;
			if ( RC > 0 ) { uabend() ; }
		}
	}

	vdelete( "ZUCMDT1 ZUCMDT2 ZUCMDT3" ) ;
	if ( RC > 0 ) { uabend() ; }
	vdelete( "ZSCMDT1 ZSCMDT2 ZSCMDT3 ZSCMDTF" ) ;
	if ( RC > 0 ) { uabend() ; }
	vdelete( "GOUCMD1 GOUCMD2 GOUCMD3" ) ;
	if ( RC > 0 ) { uabend() ; }
	vdelete( "GOSCMD1 GOSCMD2 GOSCMD3 GOSTFST" ) ;
	if ( RC > 0 ) { uabend() ; }
	vdelete( "ZKLUSE  ZNOTIFY GOKLUSE GONOTFY" ) ;
	if ( RC > 0 ) { uabend() ; }
	vdelete( "ZDEL    GODEL   ZLMSGW  GOLMSGW"  ) ;
	if ( RC > 0 ) { uabend() ; }
	vdelete( "ZSWAP   GOSWAP  ZSWAPC  GOSWAPC"  ) ;
	if ( RC > 0 ) { uabend() ; }
	vdelete( "ZRTSIZE GORTSIZE ZRBSIZE GORBSIZE" ) ;
	if ( RC > 0 ) { uabend() ; }
	vdelete( "ZPADC GOPADC ZSRETP GOSRETP ZFHRUL GOFHUPD" ) ;
	if ( RC > 0 ) { uabend() ; }
	vdelete( "GOSCRLD ZSCROLLD" ) ;
	if ( RC > 0 ) { uabend() ; }
	vdelete( "GODEFM ZDEFM GOHIGH ZHIGH" ) ;
	if ( RC > 0 ) { uabend() ; }
	vdelete( "GODECLR GODECLA ZDECLR ZDECLA GOEDLCT ZEDLCTAB" ) ;
	if ( RC > 0 ) { uabend() ; }
	vdelete( "GOHABMN ZHIABMN" ) ;
	if ( RC > 0 ) { uabend() ; }
	vdelete( "GOTABPS ZTABPAS" ) ;
	if ( RC > 0 ) { uabend() ; }
}


/**************************************************************************************************************/
/**********************************               CTRL UTILITY              ***********************************/
/**************************************************************************************************************/

void ppsp01a::controlKeys()
{
	int i ;

	bool end_loop = false ;

	string key1  ;
	string key2  ;
	string key3  ;
	string table ;
	string zcmd  ;
	string msg   ;

	string* t1   ;
	string* t2   ;

	const string alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" ;
	vector<string>oldValues ;

	vdefine( "ZCMD", &zcmd ) ;
	table = "CTKY" + d2ds( taskid(), 4 ) ;

	tbcreate( table,
		  "CTKEY1",
		  "(CTKEY2,CTACT)",
		  NOWRITE ) ;

	key1 = "ZCTRL?"    ;
	key2 = "Control-?" ;
	key3 = "ACTRL?"    ;

	for ( i = 0 ; i < 26 ; ++i )
	{
		key1[ 5 ] = alpha[ i ] ;
		key2[ 8 ] = alpha[ i ] ;
		vcopy( key1, t1, LOCATE )  ;
		oldValues.push_back( *t1 ) ;
		vreplace( "CTKEY1", key1 ) ;
		vreplace( "CTKEY2", key2 ) ;
		vreplace( "CTACT", *t1 ) ;
		tbadd( table ) ;
	}

	ztdtop = 1  ;
	msg    = "" ;
	while ( !end_loop )
	{
		tbtop( table ) ;
		tbskip( table, ztdtop ) ;
		zcmd = "" ;
		tbdispl( table,
			 "PPSP01CT",
			 msg,
			 "ZCMD" ) ;
		if ( RC == 8 ) { end_loop = true ; }
		msg = "" ;
		if ( zcmd == "RESET" || zcmd == "CANCEL" )
		{
			for ( i = 0 ; i < 26 ; ++i )
			{
				key1[ 5 ] = alpha[ i ] ;
				key2[ 8 ] = alpha[ i ] ;
				vreplace( "CTKEY1", key1 ) ;
				vreplace( "CTKEY2", key2 ) ;
				vreplace( "CTACT", oldValues[ i ] ) ;
				tbmod( table ) ;
				vreplace( key1, oldValues[ i ] ) ;
				vput( key1, PROFILE ) ;
			}
			if ( zcmd == "CANCEL" ) { break ; }
			continue ;
		}
		if ( zcmd == "RESTORE" )
		{
			msg = "PPSP011E" ;
			for ( i = 0 ; i < 26 ; ++i )
			{
				key1[ 5 ] = alpha[ i ] ;
				key2[ 8 ] = alpha[ i ] ;
				key3[ 5 ] = alpha[ i ] ;
				vget( key3, PROFILE ) ;
				if ( RC > 0 )
				{
					msg = "PPSP011C" ;
					break ;
				}
				vcopy( key3, t1, LOCATE ) ;
				vreplace( key1, *t1 ) ;
				vput( key1, PROFILE ) ;
				vreplace( "CTKEY1", key1 ) ;
				vreplace( "CTKEY2", key2 ) ;
				vreplace( "CTACT", *t1 ) ;
				tbmod( table ) ;
			}
			continue ;
		}
		while ( ztdsels > 0 )
		{
			vcopy( "CTKEY1", t1, LOCATE ) ;
			vcopy( "CTACT", t2, LOCATE ) ;
			vreplace( *t1, *t2 ) ;
			vput( *t1, PROFILE ) ;
			tbmod( table ) ;
			if ( ztdsels > 1 )
			{
				tbdispl( table ) ;
				if ( RC > 4 ) { end_loop = true ; break ; }
			}
			else { ztdsels = 0 ; }
		}
		if ( end_loop ) { break ; }
		if ( zcmd == "SAVE" )
		{
			for ( i = 0 ; i < 26 ; ++i )
			{
				key1[ 5 ] = alpha[ i ] ;
				key3[ 5 ] = alpha[ i ] ;
				vget( key1, PROFILE ) ;
				vcopy( key1, t1, LOCATE ) ;
				vreplace( key3, *t1 ) ;
				vput( key3, PROFILE ) ;
			}
			msg = "PPSP011D" ;
		}
	}

	tbend( table ) ;
	vdelete( "ZCMD" ) ;
}


/**************************************************************************************************************/
/**********************************              COLOUR SETTINGS            ***********************************/
/**************************************************************************************************************/

void ppsp01a::cuaattrSettings()
{
	int i ;
	int j ;

	const int maxEntries = 34 ;

	string suf ;
	string panel = "PPSP01CL" ;

	string msg    ;
	string curfld ;
	string var1   ;
	string var2   ;
	string var3   ;
	string isps_var ;
	string prof_var ;
	string colour ;
	string intens ;
	string hilite ;

	map<int, string>VarList  ;
	map<int, string>DefList  ;
	map<int, string>OrigList ;

	VarList[ 1  ] = "AB"   ;
	VarList[ 2  ] = "ABSL" ;
	VarList[ 3  ] = "ABU"  ;
	VarList[ 4  ] = "AMT"  ;
	VarList[ 5  ] = "AWF"  ;
	VarList[ 6  ] = "IWF"  ;
	VarList[ 7  ] = "CT"   ;
	VarList[ 8  ] = "CEF"  ;
	VarList[ 9  ] = "CH"   ;
	VarList[ 10 ] = "DT"   ;
	VarList[ 11 ] = "ET"   ;
	VarList[ 12 ] = "EE"   ;
	VarList[ 13 ] = "FP"   ;
	VarList[ 14 ] = "FK"   ;
	VarList[ 15 ] = "IMT"  ;
	VarList[ 16 ] = "LEF"  ;
	VarList[ 17 ] = "LID"  ;
	VarList[ 18 ] = "LI"   ;
	VarList[ 19 ] = "NEF"  ;
	VarList[ 20 ] = "NT"   ;
	VarList[ 21 ] = "PI"   ;
	VarList[ 22 ] = "PIN"  ;
	VarList[ 23 ] = "PT"   ;
	VarList[ 24 ] = "PS"   ;
	VarList[ 25 ] = "PAC"  ;
	VarList[ 26 ] = "PUC"  ;
	VarList[ 27 ] = "RP"   ;
	VarList[ 28 ] = "SI"   ;
	VarList[ 29 ] = "SAC"  ;
	VarList[ 30 ] = "SUC"  ;
	VarList[ 31 ] = "VOI"  ;
	VarList[ 32 ] = "WMT"  ;
	VarList[ 33 ] = "WT"   ;
	VarList[ 34 ] = "WASL" ;

	DefList[ 1  ] = KAB    ;
	DefList[ 2  ] = KABSL  ;
	DefList[ 3  ] = KABU   ;
	DefList[ 4  ] = KAMT   ;
	DefList[ 5  ] = KAWF   ;
	DefList[ 6  ] = KIWF   ;
	DefList[ 7  ] = KCT    ;
	DefList[ 8  ] = KCEF   ;
	DefList[ 9  ] = KCH    ;
	DefList[ 10 ] = KDT    ;
	DefList[ 11 ] = KET    ;
	DefList[ 12 ] = KEE    ;
	DefList[ 13 ] = KFP    ;
	DefList[ 14 ] = KFK    ;
	DefList[ 15 ] = KIMT   ;
	DefList[ 16 ] = KLEF   ;
	DefList[ 17 ] = KLID   ;
	DefList[ 18 ] = KLI    ;
	DefList[ 19 ] = KNEF   ;
	DefList[ 20 ] = KNT    ;
	DefList[ 21 ] = KPI    ;
	DefList[ 22 ] = KPIN   ;
	DefList[ 23 ] = KPT    ;
	DefList[ 24 ] = KPS    ;
	DefList[ 25 ] = KPAC   ;
	DefList[ 26 ] = KPUC   ;
	DefList[ 27 ] = KRP    ;
	DefList[ 28 ] = KSI    ;
	DefList[ 29 ] = KSAC   ;
	DefList[ 30 ] = KSUC   ;
	DefList[ 31 ] = KVOI   ;
	DefList[ 32 ] = KWMT   ;
	DefList[ 33 ] = KWT    ;
	DefList[ 34 ] = KWASL  ;


	for ( i = 1 ; i <= maxEntries ; ++i )
	{
		if ( not setScreenAttrs( VarList[ i ], i ) )
		{
			llog( "E", "ISPS variable " << "ZC" + VarList[ i ] << " not found.  Re-run setup program to create" << endl ) ;
			uabend() ;
		}
	}

	for ( i = 1 ; i <= maxEntries ; ++i )
	{
		isps_var = "ZC" + VarList[ i ] ;
		vcopy( isps_var, val, MOVE ) ;
		if ( RC > 0 )
		{
			llog( "E", "ISPS variable " << isps_var << " not found.  Re-run setup program to create" << endl ) ;
			uabend() ;
		}
		OrigList[ i ] = val ;
	}

	msg    = "" ;
	zcmd   = "" ;
	curfld = "" ;

	addpop( "", 0, -2 ) ;
	while ( true )
	{
		if ( msg == "" ) { curfld = "ZCMD" ; }
		control( "CUA", "RELOAD" ) ;
		display( panel, msg, curfld ) ;
		if (RC == 8 && zcmd != "CANCEL" )
		{
			break ;
		}
		panel = "" ;
		if ( zcmd == "" )
		{
			;
		}
		else if ( zcmd == "CANCEL" )
		{
			for ( i = 1 ; i <= maxEntries ; ++i )
			{
				isps_var = "ZC" + VarList[ i ] ;
				vcopy( isps_var, val, MOVE )   ;
				if ( RC > 0 )
				{
					llog( "E", "ISPS variable " << isps_var << " not found.  Re-run setup program to create" << endl ) ;
					uabend() ;
				}
				setISPSVar( VarList[ i ], OrigList[ i ] ) ;
			}
			control( "CUA", "RELOAD" ) ;
			control( "NONDISPL", "END" ) ;
			display() ;
			break ;
		}
		else if ( zcmd == "DEFAULTS" )
		{
			for ( i = 1 ; i <= maxEntries ; ++i )
			{
				setISPSVar( VarList[ i ], DefList[ i ]  ) ;
			}
			for ( i = 1 ; i <= maxEntries ; ++i )
			{
				if ( not setScreenAttrs( VarList[ i ], i ) ) { uabend() ; }
			}
		}
		else if ( zcmd == "SAVE" )
		{
			zcmd = "" ;
			for ( i = 1 ; i <= maxEntries ; ++i )
			{
				isps_var = "ZC" + VarList[ i ] ;
				vcopy( isps_var, val, MOVE ) ;
				if ( RC > 0 )
				{
					llog( "E", "ISPS variable " << isps_var << " not found.  Re-run setup program to create" << endl ) ;
					uabend() ;
				}
				prof_var      = isps_var  ;
				prof_var[ 0 ] = 'A'       ;
				vdefine( prof_var, &val ) ;
				vput( prof_var, PROFILE ) ;
				vdelete( prof_var ) ;
			}
			if ( RC == 0 ) { msg = "PPSP011A" ; }
			continue ;
		}
		else if ( zcmd == "RESTORE" )
		{
			zcmd = "" ;
			for ( i = 1, j = 0 ; i <= maxEntries ; ++i )
			{
				prof_var = "AC" + VarList[ i ] ;
				vcopy( prof_var, val, MOVE ) ;
				if ( RC > 0 )
				{
					j++ ;
					continue ;
				}
				isps_var      = prof_var  ;
				isps_var[ 0 ] = 'Z'       ;
				vdefine( isps_var, &val ) ;
				vput( isps_var, PROFILE ) ;
				vdelete( isps_var ) ;
				if ( !setScreenAttrs( VarList[ i ], i ) )
				{
					uabend() ;
				}
			}
			if ( j == maxEntries )
			{
				msg = "PPSP019" ;
			}
			else if ( j == 0 )
			{
				msg = "PPSP011B" ;
			}
			else
			{
				msg = "PPSP011H" ;
			}
			continue ;
		}

		msg  = "" ;
		zcmd = "" ;
		for ( i = 1 ; i <= maxEntries ; ++i )
		{
			suf  = d2ds( i, 2 ) ;
			var1 = "COLOUR" + suf ;
			var2 = "INTENS" + suf ;
			var3 = "HILITE" + suf ;
			vcopy( var1, colour, MOVE ) ;
			vcopy( var2, intens, MOVE ) ;
			vcopy( var3, hilite, MOVE ) ;
			if ( colour == "" ) { colour = DefList[ i ][ 0 ] ; }
			if ( intens == "" ) { intens = DefList[ i ][ 1 ] ; }
			if ( hilite == "" ) { hilite = DefList[ i ][ 2 ] ; }
			isps_var = "ZC" + VarList[ i ] ;
			vcopy( isps_var, val, MOVE ) ;
			if ( RC > 0 )
			{
				llog( "E", "ISPS variable " << isps_var << " not found.  Re-run setup program to create" << endl ) ;
				uabend() ;
			}
			if ( val.size() != 3 )
			{
				llog( "E", "ISPS variable " << isps_var << " has invalid value of " << val << "  Re-run setup program to re-create" << endl ) ;
				uabend() ;
			}
			switch ( colour[ 0 ] )
			{
			case 'R':
				vreplace( var1, "RED" ) ;
				val[ 0 ] = 'R' ;
				break ;

			case 'G':
				vreplace( var1, "GREEN" ) ;
				val[ 0 ] = 'G' ;
				break ;

			case 'Y':
				vreplace( var1, "YELLOW" ) ;
				val[ 0 ] = 'Y' ;
				break ;

			case 'B':
				vreplace( var1, "BLUE" ) ;
				val[ 0 ] = 'B' ;
				break ;

			case 'P':
			case 'M':
				vreplace( var1, "MAGENTA" ) ;
				val[ 0 ] = 'M' ;
				break ;

			case 'T':
				vreplace( var1, "TURQ" ) ;
				val[ 0 ] = 'T' ;
				break ;

			case 'W':
				vreplace( var1, "WHITE" ) ;
				val[ 0 ] = 'W' ;
				break ;
			}

			switch ( intens[ 0 ] )
			{
			case 'H':
				vreplace( var2, "HIGH" ) ;
				val[ 1 ] = 'H' ;
				break ;

			case 'L':
				vreplace( var2, "LOW"  ) ;
				val[ 1 ] = 'L' ;
				break ;
			}

			switch ( hilite[ 0 ] )
			{
			case 'N':
				vreplace( var3, "NONE" ) ;
				val[ 2 ] = 'N' ;
				break ;

			case 'B':
				vreplace( var3, "BLINK" ) ;
				val[ 2 ] = 'B' ;
				break ;

			case 'R':
				vreplace( var3, "REVERSE" ) ;
				val[ 2 ] = 'R' ;
				break ;

			case 'U':
				vreplace( var3, "USCORE" ) ;
				val[ 2 ] = 'U' ;
				break ;
			}

			vdefine( isps_var, &val ) ;
			vput( isps_var, PROFILE ) ;
			vdelete( isps_var ) ;
		}
	}
	rempop() ;
}


bool ppsp01a::setScreenAttrs( const string& name,
			      int itr )
{
	string t ;

	string colour ;
	string intens ;
	string hilite ;

	vcopy( "ZC" + name, t, MOVE ) ;
	if ( RC > 0 )
	{
		llog( "E", "Variable ZC" << name << " not found in ISPS profile" << endl ) ;
		return false ;
	}

	if ( t.size() != 3 )
	{
		llog( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ;
		return false ;
	}

	switch ( t[ 0 ] )
	{
		case 'R': colour = "RED"     ; break ;
		case 'G': colour = "GREEN"   ; break ;
		case 'Y': colour = "YELLOW"  ; break ;
		case 'B': colour = "BLUE"    ; break ;
		case 'M': colour = "MAGENTA" ; break ;
		case 'T': colour = "TURQ"    ; break ;
		case 'W': colour = "WHITE"   ; break ;
		default :
			  llog( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ;
	}
	switch ( t[ 1 ] )
	{
		case 'H': intens = "HIGH" ; break ;
		case 'L': intens = "LOW"  ; break ;
		default :
			  llog( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ;
	}
	switch ( t[ 2 ] )
	{
		case 'N': hilite = "NONE"    ; break ;
		case 'B': hilite = "BLINK"   ; break ;
		case 'R': hilite = "REVERSE" ; break ;
		case 'U': hilite = "USCORE"  ; break ;
		default :
			  llog( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ;
	}

	vreplace( "COLOUR" + d2ds( itr, 2 ), colour ) ;
	vreplace( "INTENS" + d2ds( itr, 2 ), intens ) ;
	vreplace( "HILITE" + d2ds( itr, 2 ), hilite ) ;

	return true ;
}


void ppsp01a::setISPSVar( const string& xvar,
			  string xval )
{
	string isps_var ;

	isps_var = "ZC" + xvar ;
	vdefine( isps_var, &xval ) ;
	vput( isps_var, PROFILE ) ;
	vdelete( isps_var ) ;
}


/**************************************************************************************************************/
/**********************************         GLOBAL COLOUR SETTINGS          ***********************************/
/**************************************************************************************************************/

void ppsp01a::globalColours()
{
	int i ;

	string zcmd ;
	string colour ;

	vdefine( "ZCMD", &zcmd ) ;

	map<int, string> tab1 = { { 1, "B" } ,
				  { 2, "R" } ,
				  { 3, "M" } ,
				  { 4, "G" } ,
				  { 5, "T" } ,
				  { 6, "Y" } ,
				  { 7, "W" } } ;

	map<string, string> tab2 = { { "B", "BLUE"    } ,
				     { "R", "RED"     } ,
				     { "M", "MAGENTA" } ,
				     { "G", "GREEN"   } ,
				     { "T", "TURQ"    } ,
				     { "Y", "YELLOW"  } ,
				     { "W", "WHITE"   } } ;

	for ( i = 1 ; i < 8 ; ++i )
	{
		var = "ZGCL" + tab1[ i ] ;
		vcopy( var, colour, MOVE ) ;
		vreplace( "COLOUR"+ d2ds( i, 2 ), ( colour == tab1[ i ] ) ? "" : tab2[ colour ] ) ;
	}

	addpop( "", 5, 5 ) ;
	while ( true )
	{
		control( "CUA", "RELOAD" ) ;
		display( "PPSP01CR" ) ;
		if (RC == 8 )
		{
			break ;
		}
		if ( zcmd == "RGBSET" )
		{
			setRGBValues() ;
			continue ;
		}
		else if ( zcmd == "DECSET" )
		{
			addpop( "", -1, -2 ) ;
			while ( true )
			{
				display( "PPSP01CD" ) ;
				if ( RC > 0 ) { break ; }
			}
			rempop() ;
			continue ;
		}
		for ( i = 1 ; i < 8 ; ++i )
		{
			vcopy( "COLOUR"+ d2ds( i, 2 ), colour, MOVE ) ;
			var = "ZGCL" + tab1[ i ] ;
			val = ( colour == "" ) ? tab1[ i ] : colour.substr( 0, 1 ) ;
			vreplace( var, val ) ;
			vput( var, PROFILE ) ;
		}
	}

	rempop() ;
	vdelete( "ZCMD" ) ;
}


/**************************************************************************************************************/
/**********************************           RGB COLOUR SETTINGS           ***********************************/
/**************************************************************************************************************/

void ppsp01a::setRGBValues()
{
	short int r ;
	short int g ;
	short int b ;

	string t ;
	string zcmd ;
	string n_rgb = "ZNRGB" ;
	string u_rgb = "ZURGB" ;

	map<int, string> suf =
	{ { 1, "R" },
	  { 2, "G" },
	  { 3, "Y" },
	  { 4, "B" },
	  { 5, "M" },
	  { 6, "T" },
	  { 7, "W" } } ;

	for ( uint i = 1 ; i < 8 ; ++i )
	{
		vget( u_rgb + suf[ i ], PROFILE ) ;
		vcopy( u_rgb + suf[ i ], t, MOVE ) ;
		if ( t.size() != 12 )
		{
			continue ;
		}
		r = ds2d( t.substr( 0, 4 ) ) * 51 / 200 + 0.5 ;
		g = ds2d( t.substr( 4, 4 ) ) * 51 / 200 + 0.5 ;
		b = ds2d( t.substr( 8, 4 ) ) * 51 / 200 + 0.5 ;
		vreplace( "RGBR"+ d2ds( i ), d2ds( r ) ) ;
		vreplace( "RGBG"+ d2ds( i ), d2ds( g ) ) ;
		vreplace( "RGBB"+ d2ds( i ), d2ds( b ) ) ;
	}

	vdefine( "ZCMD", &zcmd ) ;

	addpop( "", -1, -2 ) ;
	while ( true )
	{
		control( "CUA", "RELOAD" ) ;
		display( "PPSP01CG" ) ;
		if (RC == 8 ) {  break ; }
		if ( zcmd == "DEFAULTS" )
		{
			for ( uint i = 1 ; i < 8 ; ++i )
			{
				vget( n_rgb + suf[ i ], SHARED ) ;
				vcopy( n_rgb + suf[ i ], t, MOVE ) ;
				vreplace( u_rgb + suf[ i ], t ) ;
				vput( u_rgb + suf[ i ], PROFILE ) ;
			}
			for ( uint i = 1 ; i < 8 ; ++i )
			{
				vget( u_rgb + suf[ i ], PROFILE ) ;
				vcopy( u_rgb + suf[ i ], t, MOVE ) ;
				if ( t.size() != 12 )
				{
					continue ;
				}
				r = ds2d( t.substr( 0, 4 ) ) * 51 / 200 + 0.5 ;
				g = ds2d( t.substr( 4, 4 ) ) * 51 / 200 + 0.5 ;
				b = ds2d( t.substr( 8, 4 ) ) * 51 / 200 + 0.5 ;
				vreplace( "RGBR"+ d2ds( i ), d2ds( r ) ) ;
				vreplace( "RGBG"+ d2ds( i ), d2ds( g ) ) ;
				vreplace( "RGBB"+ d2ds( i ), d2ds( b ) ) ;
			}
		}
		for ( uint i = 1 ; i < 8 ; ++i )
		{
			vcopy( "RGBR" + d2ds( i ), t, MOVE ) ;
			r = ds2d( t ) * 200 / 51 + 0.5 ;
			vcopy( "RGBG" + d2ds( i ), t, MOVE ) ;
			g = ds2d( t ) * 200 / 51 + 0.5 ;
			vcopy( "RGBB" + d2ds( i ), t, MOVE ) ;
			b = ds2d( t ) * 200 / 51 + 0.5 ;
			t = d2ds( r, 4 ) + d2ds( g, 4 ) + d2ds( b, 4 ) ;
			vreplace( u_rgb + suf[ i ], t ) ;
			vput( u_rgb + suf[ i ], PROFILE ) ;
		}
	}

	rempop() ;
	vdelete( "ZCMD" ) ;
}


/**************************************************************************************************************/
/**********************************                TODO LIST                ***********************************/
/**************************************************************************************************************/

void ppsp01a::todoList()
{
	addpop( "", 5, 5 ) ;
	while ( true )
	{
		display( "PPSP01TD", "", "ZCMD" );
		if ( RC == 8 ) { break ; }
	}
	rempop() ;
}


/**************************************************************************************************************/
/**********************************         VARIABLE POOL UTILITY           ***********************************/
/**************************************************************************************************************/

void ppsp01a::poolVariables( const string& applid )
{
	int RCode ;
	int crp ;

	int csrrow = 0 ;

	string msg   ;
	string cw    ;
	string table ;
	string zcmd1 ;
	string valu  ;
	string panel ;
	string filter ;
	string cursor ;

	vcopy( "ZAPPLID", zapplid, MOVE ) ;
	if ( applid != "" && zapplid != applid )
	{
		if ( !isvalidName4( applid ) ) { return ; }
		select( "PGM(PPSP01A) PARM(VARS) NEWAPPL(" + applid + ")" ) ;
		return ;
	}

	table = "VARL" + d2ds( taskid(), 4 ) ;

	const string vlist1 = "CRP" ;
	const string vlist2 = "SEL VAR VPOOL VPLVL VAL" ;

	vdefine( vlist1, &crp ) ;
	vdefine( vlist2, &sel, &var, &vpool, &vplvl, &val ) ;

	filter = "" ;
	ztdtop = 1  ;
	msg    = "" ;
	cursor = "ZCMD" ;
	csrrow = 0  ;

	while ( true )
	{
		getpoolVariables( table, filter ) ;
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( table ) ;
			tbskip( table, ztdtop ) ;
			panel = "PPSP01AV" ;
		}
		if ( msg == "" ) { zcmd = "" ; }
		tbdispl( table,
			 panel,
			 msg,
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC == 8 ) { break ; }
		if ( zcmd == "RES" )
		{
			filter  = "" ;
			ztdsels = 0 ;
			continue ;
		}
		panel  = "" ;
		msg    = "" ;
		cw     = "" ;
		cursor = ( sel == "" ) ? "" : "SEL" ;
		csrrow = crp ;
		if ( zcmd != "" )
		{
			cw     = word( zcmd, 1 ) ;
			filter = word( zcmd, 2 ) ;
		}
		if ( cw == "O" )
		{
			continue ;
		}
		if ( sel == "D" )
		{
			control( "ERRORS", "RETURN" ) ;
			verase( var, ( vpool == "S" ) ? SHARED : PROFILE ) ;
			RCode = RC ;
			control( "ERRORS", "CANCEL" ) ;
			if ( RCode >= 12 )
			{
				vcopy( "ZERRMSG", msg ) ;
			}
		}
		else if ( sel == "U" )
		{
			if ( vplvl == "1" )
			{
				valu = val ;
				vdefine( "ZCMD1 VALU", &zcmd1, &valu ) ;
				addpop( "", 5, 5 ) ;
				while ( RC == 0 )
				{
					display( "PPSP01AW" ) ;
				}
				if ( zcmd1 != "CANCEL" )
				{
					vdefine( var, &valu ) ;
					control( "ERRORS", "RETURN" ) ;
					vput( var, ( vpool == "S" ) ? SHARED : PROFILE ) ;
					RCode = RC ;
					control( "ERRORS", "CANCEL" ) ;
					vdelete( var ) ;
					if ( RCode >= 12 )
					{
						vcopy( "ZERRMSG", msg ) ;
					}
				}
				rempop() ;
				vdelete( "ZCMD1 VALU" ) ;
			}
			else
			{
				msg = "PPSP015A" ;
			}
		}
		sel = "" ;
	}

	tbend( table ) ;

	vdelete( vlist1, vlist2 ) ;
}


void ppsp01a::getpoolVariables( const string& table,
				const string& pattern )
{
	//
	// SHARED 1  - shared variable pool.
	// SHARED 2  - default variable pool (@DEFSHAR).
	// PROFILE 1 - application variable pool.
	// PROFILE 2 - Read-only extention pool.
	// PROFILE 3 - default read-only profile pool (@DEFPROF).
	// PROFILE 4 - System profile (ISPSPROF).
	//

	tbcreate( table,
		  "",
		  "(SEL,VAR,VPOOL,VPLVL,VAL)",
		  NOWRITE,
		  REPLACE ) ;

	sel = "" ;

	set<string> found ;
  /*    varlist = vilist( DEFINED ) + vslist( DEFINED ) ;
	vpool = "F" ;
	vplvl = "D" ;
	ws    = words( varlist ) ;
	for ( i = 1 ; i <= ws ; ++i )
	{
		var = word( varlist, i ) ;
		if ( (pattern != "") && (pos( pattern, var ) == 0) ) { continue ; }
		vcopy( var, val, MOVE ) ;
		tbadd( table )    ;
	}

	varlist = vilist( IMPLICIT ) + vslist( IMPLICIT ) ;
	vpool = "F" ;
	vplvl = "I" ;
	ws    = words( varlist ) ;
	for ( i = 1 ; i <= ws ; ++i )
	{
		var = word( varlist, i ) ;
		if ( (pattern != "") && (pos( pattern, var ) == 0) ) { continue ; }
		vcopy( var, val, MOVE ) ;
		tbadd( table )    ;
	}
	*/

	found.clear() ;
	set<string>& varlist = vlist( SHARED, 1 ) ;
	vpool = "S" ;
	vplvl = "1" ;
	for ( const auto& var : varlist )
	{
		if ( pattern != "" && pos( pattern, var ) == 0 ) { continue ; }
		vget( var, SHARED ) ;
		vcopy( var, val, MOVE ) ;
		vreplace( "VAR", var ) ;
		tbadd( table )      ;
		found.insert( var ) ;
	}

	vlist( SHARED, 2 ) ;
	vplvl = "2" ;
	for ( const auto& var : varlist )
	{
		if ( found.count( var ) > 0 ) { continue ; }
		if ( pattern != "" && pos( pattern, var ) == 0 ) { continue ; }
		vget( var, SHARED ) ;
		vcopy( var, val, MOVE ) ;
		vreplace( "VAR", var ) ;
		tbadd( table )      ;
		found.insert( var ) ;
	}

	found.clear() ;
	vpool = "P" ;
	vplvl = "1" ;
	vlist( PROFILE, 1 ) ;
	for ( const auto& var : varlist )
	{
		if ( pattern != "" && pos( pattern, var ) == 0 ) { continue ; }
		vget( var, PROFILE ) ;
		vcopy( var, val, MOVE ) ;
		vreplace( "VAR", var ) ;
		tbadd( table )      ;
		found.insert( var ) ;
	}

	vlist( PROFILE, 2 ) ;
	vplvl = "2" ;
	for ( const auto& var : varlist )
	{
		if ( found.count( var ) > 0 ) { continue ; }
		if ( pattern != "" && pos( pattern, var ) == 0 ) { continue ; }
		vget( var, PROFILE ) ;
		vcopy( var, val, MOVE ) ;
		vreplace( "VAR", var ) ;
		tbadd( table )      ;
		found.insert( var ) ;
	}

	vlist( PROFILE, 3 ) ;
	vplvl = "3" ;
	for ( const auto& var : varlist )
	{
		if ( found.count( var ) > 0 ) { continue ; }
		if ( pattern != "" && pos( pattern, var ) == 0 ) { continue ; }
		vget( var, PROFILE ) ;
		vcopy( var, val, MOVE ) ;
		vreplace( "VAR", var ) ;
		tbadd( table )      ;
		found.insert( var ) ;
	}

	vlist( PROFILE, 4 ) ;
	vplvl = "4" ;
	for ( const auto& var : varlist )
	{
		if ( found.count( var ) > 0 ) { continue ; }
		if ( pattern != "" && pos( pattern, var ) == 0 ) { continue ; }
		vget( var, PROFILE ) ;
		vcopy( var, val, MOVE ) ;
		vreplace( "VAR", var ) ;
		tbadd( table )       ;
	}

	tbtop( table ) ;
}


/**************************************************************************************************************/
/**********************************              PATHS UTILITY              ***********************************/
/**************************************************************************************************************/

void ppsp01a::showPaths()
{
	//
	// Display search paths for messages, panels, tables, REXX, profiles and load modules.
	// Include LIBDEF entries.
	//

	int i ;
	int crp ;
	int csrrow ;

	string zmlib    ;
	string zplib    ;
	string zslib    ;
	string zllib    ;
	string ztlib    ;
	string ztabl    ;
	string zmusr    ;
	string zpusr    ;
	string zsusr    ;
	string ztusr    ;
	string ztabu    ;
	string zfilu    ;
	string zorxpath ;
	string zldpath  ;
	string libs     ;

	string table    ;
	string panel    ;
	string pvar1    ;
	string pvar2    ;
	string path     ;
	string desc     ;
	string msg      ;
	string csr      ;

	const string vlist1 = "ZMLIB ZPLIB ZSLIB ZLLIB ZTLIB ZTABL ZORXPATH ZLDPATH" ;
	const string vlist2 = "ZMUSR ZPUSR ZSUSR ZTUSR ZTABU ZFILU LIBS" ;
	const string vlist3 = "SEL PVAR1 PVAR2 PATH MESSAGE DESC" ;
	const string vlist4 = "CRP" ;

	vdefine( vlist1, &zmlib, &zplib, &zslib, &zllib, &ztlib, &ztabl, &zorxpath, &zldpath ) ;
	vdefine( vlist2, &zmusr, &zpusr, &zsusr, &ztusr, &ztabu, &zfilu, &libs ) ;
	vdefine( vlist3, &sel, &pvar1, &pvar2, &path, &message, &desc ) ;
	vdefine( vlist4, &crp ) ;

	qbaselib( "ZMLIB", "ZMLIB" ) ;
	qbaselib( "ZPLIB", "ZPLIB" ) ;
	qbaselib( "ZSLIB", "ZSLIB" ) ;
	qbaselib( "ZLLIB", "ZLLIB" ) ;
	qbaselib( "ZTLIB", "ZTLIB" ) ;
	qbaselib( "ZTABL", "ZTABL" ) ;
	qbaselib( "ZMUSR", "ZMUSR" ) ;
	qbaselib( "ZPUSR", "ZPUSR" ) ;
	qbaselib( "ZSUSR", "ZSUSR" ) ;
	qbaselib( "ZTUSR", "ZTUSR" ) ;
	qbaselib( "ZTABU", "ZTABU" ) ;
	qbaselib( "ZFILU", "ZFILU" ) ;
	qbaselib( "ZORXPATH", "ZORXPATH" ) ;
	qbaselib( "ZLDPATH", "ZLDPATH" ) ;

	table = "PTHL" + d2ds( taskid(), 4 ) ;

	tbcreate( table,
		  "",
		  "(SEL,PVAR1,PVAR2,PATH,MESSAGE,DESC)",
		  NOWRITE ) ;

	libs  = "" ;
	qlibdef( "ZMLIB", "", "LIBS" ) ;
	pvar1 = "ZMLIB" ;
	pvar2 = "ZMLIB" ;
	desc  = "LIBDEF application path for messages" ;
	for ( i = 1 ; i <= getpaths( libs ) ; ++i )
	{
		message = "" ;
		path    = getpath( libs, i ) ;
		if ( !is_directory( path ) ) { message = "Path not found" ; }
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	libs  = "" ;
	qlibdef( "ZPLIB", "", "LIBS" ) ;
	pvar1 = "ZPLIB" ;
	pvar2 = "ZPLIB" ;
	desc  = "LIBDEF application path for panels" ;
	for ( i = 1 ; i <= getpaths( libs ) ; ++i )
	{
		message = "" ;
		path    = getpath( libs, i ) ;
		if ( !is_directory( path ) ) { message = "Path not found" ; }
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	libs  = "" ;
	qlibdef( "ZSLIB", "", "LIBS" ) ;
	pvar1 = "ZSLIB" ;
	pvar2 = "ZSLIB" ;
	desc  = "LIBDEF application path for skeletons" ;
	for ( i = 1 ; i <= getpaths( libs ) ; ++i )
	{
		message = "" ;
		path    = getpath( libs, i ) ;
		if ( !is_directory( path ) ) { message = "Path not found" ; }
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	libs  = "" ;
	qlibdef( "ZLLIB", "", "LIBS" ) ;
	pvar1 = "ZLLIB" ;
	pvar2 = "ZLLIB" ;
	desc  = "LIBDEF application path for program load libraries" ;
	for ( i = 1 ; i <= getpaths( libs ) ; ++i )
	{
		message = "" ;
		path    = getpath( libs, i ) ;
		if ( !is_directory( path ) ) { message = "Path not found" ; }
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	libs  = "" ;
	qlibdef( "ZTLIB", "", "LIBS" ) ;
	pvar1 = "ZTLIB" ;
	pvar2 = "ZTLIB" ;
	desc  = "LIBDEF application path for tables" ;
	for ( i = 1 ; i <= getpaths( libs ) ; ++i )
	{
		message = "" ;
		path    = getpath( libs, i ) ;
		if ( !is_directory( path ) ) { message = "Path not found" ; }
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	libs  = "" ;
	qlibdef( "ZTABL", "", "LIBS" ) ;
	pvar1 = "ZTABL" ;
	pvar2 = "ZTABL" ;
	desc  = "LIBDEF application path for table output" ;
	for ( i = 1 ; i <= getpaths( libs ) ; ++i )
	{
		message = "" ;
		path    = getpath( libs, i ) ;
		if ( !is_directory( path ) ) { message = "Path not found" ; }
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	sel   = "" ;
	pvar1 = "ZLDPATH" ;
	pvar2 = "ZLDPATH" ;
	desc  = "Path for application modules" ;
	for ( i = 1 ; i <= getpaths( zldpath ) ; ++i )
	{
		path = getpath( zldpath, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) message = "Path not found" ;
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	pvar1 = "ZMLIB" ;
	pvar2 = "ZMLIB" ;
	desc  = "System search path for messages" ;
	for ( i = 1 ; i <= getpaths( zmlib ) ; ++i )
	{
		path = getpath( zmlib, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) message = "Path not found" ;
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	pvar1 = "ZPLIB" ;
	pvar2 = "ZPLIB" ;
	desc  = "System search path for panels" ;
	for ( i = 1 ; i <= getpaths( zplib ) ; ++i )
	{
		path = getpath( zplib, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) message = "Path not found" ;
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	pvar1 = "ZSLIB" ;
	pvar2 = "ZSLIB" ;
	desc  = "System search path for skeletons" ;
	for ( i = 1 ; i <= getpaths( zslib ) ; ++i )
	{
		path = getpath( zslib, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) message = "Path not found" ;
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	pvar1 = "ZLLIB" ;
	pvar2 = "ZLLIB" ;
	desc  = "System search path for program load libraries" ;
	for ( i = 1 ; i <= getpaths( zllib ) ; ++i )
	{
		path = getpath( zllib, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) message = "Path not found" ;
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	pvar1 = "ZTLIB" ;
	pvar2 = "ZTLIB" ;
	desc  = "System search path for tables" ;
	for ( i = 1 ; i <= getpaths( ztlib ) ; ++i )
	{
		path = getpath( ztlib, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) message = "Path not found" ;
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	pvar1 = "ZTABL" ;
	pvar2 = "ZTABL" ;
	desc  = "System path for table output" ;
	for ( i = 1 ; i <= getpaths( ztabl ) ; ++i )
	{
		path = getpath( ztabl, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) message = "Path not found" ;
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	pvar1 = "ZMUSR" ;
	pvar2 = "ZMUSR" ;
	desc  = "User search path for messages" ;
	for ( i = 1 ; i <= getpaths( zmusr ) ; ++i )
	{
		path = getpath( zmusr, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) message = "Path not found" ;
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	pvar1 = "ZPUSR" ;
	pvar2 = "ZPUSR" ;
	desc = "User search path for panels" ;
	for ( i = 1 ; i <= getpaths( zpusr ) ; ++i )
	{
		path = getpath( zpusr, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) message = "Path not found" ;
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	pvar1 = "ZTUSR" ;
	pvar2 = "ZTUSR" ;
	desc  = "User search path for tables" ;
	for ( i = 1 ; i <= getpaths( ztusr ) ; ++i )
	{
		path = getpath( ztusr, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) message = "Path not found" ;
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	pvar1 = "ZTABU" ;
	pvar2 = "ZTABU" ;
	desc  = "User search path for table output" ;
	for ( i = 1 ; i <= getpaths( ztabu ) ; ++i )
	{
		path = getpath( ztabu, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) message = "Path not found" ;
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	pvar1 = "ZFILU" ;
	pvar2 = "ZFILU" ;
	desc  = "User search path for file tailoring output" ;
	for ( i = 1 ; i <= getpaths( zfilu ) ; ++i )
	{
		path = getpath( zfilu, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) message = "Path not found" ;
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	pvar1 = "ZSYSPATH" ;
	pvar2 = "ZSYSPATH" ;
	vget( "ZSYSPATH", PROFILE ) ;
	vcopy( "ZSYSPATH", path, MOVE ) ;
	desc = "System Path" ;
	tbadd( table )  ;

	pvar1 = "ZUPROF" ;
	pvar2 = "ZUPROF" ;
	vget( "ZUPROF", PROFILE ) ;
	vcopy( "ZUPROF", path, MOVE ) ;
	desc = "User home profile directory" ;
	tbadd( table )    ;

	pvar1 = "ZORXPATH" ;
	pvar2 = "ZORXPATH" ;
	desc  = "Object REXX EXEC search path" ;
	for ( i = 1 ; i <= getpaths( zorxpath ) ; ++i )
	{
		path = getpath( zorxpath, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) { message = "Path not found" ; }
		tbadd( table ) ;
		desc  = "" ;
		pvar1 = "" ;
	}

	tbtop( table ) ;

	ztdtop = 1  ;
	msg    = "" ;
	csr    = "ZCMD" ;
	csrrow = 0  ;

	while ( true )
	{
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( table ) ;
			tbskip( table, ztdtop ) ;
			panel = "PPSP01AP" ;
		}
		if ( msg == "" ) { zcmd  = "" ; }
		tbdispl( table,
			 panel,
			 msg,
			 csr,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC == 8 ) { break ; }
		panel  = "" ;
		csr    = ( sel == "" ) ? "" : "SEL" ;
		csrrow = crp ;
		msg    = "" ;
		zcmd   = "" ;
		if ( ztdsels == 0 && zcurinx != 0 )
		{
			tbtop( table ) ;
			tbskip( table, zcurinx ) ;
			ztdsels = 1   ;
			sel     = "S" ;
		}
		if ( sel == "S" )
		{
			if ( is_directory( path ) )
			{
				message = "*Listed*" ;
				select( "PGM(" + get_dialogue_var( "ZFLSTPGM" ) + ") PARM(QLIB " + pvar2 + ")" ) ;
			}
			else
			{
				message = "*Error*" ;
			}
			sel = "" ;
			tbput( table ) ;
		}
	}

	tbend( table ) ;
	vdelete( vlist1, vlist2, vlist3, vlist4 ) ;
}


/**************************************************************************************************************/
/**********************************        LIBDEF STATUS (ISPLIBD)          ***********************************/
/**************************************************************************************************************/

void ppsp01a::libdefStatus( const string& libdef )
{
	size_t p ;

	bool stacked = false ;

	string t ;
	string msg ;
	string table ;

	string stk ;
	string usr ;
	string libx ;
	string type ;
	string ident ;
	string zxusr ;

	const string vlist1      = "LDSTK LDLIB LDTYP LDUSR LDID" ;
	const string sys_libdefs = "ZMLIB ZPLIB ZSLIB ZLLIB ZTLIB ZTABL ZFILE" ;

	vector<string> list ;

	vdefine( vlist1, &stk, &libx, &type, &usr, &ident ) ;

	table = "LIBD" + d2ds( taskid(), 4 ) ;

	tbcreate( table,
		  "",
		  "(LDSTK,LDLIB,LDTYP,LDUSR,LDID)",
		  NOWRITE ) ;

	map<string,stack<zlibdef>> zlibd = get_zlibd() ;

	list.push_back( "ZMLIB" ) ;
	list.push_back( "ZPLIB" ) ;
	list.push_back( "ZSLIB" ) ;
	list.push_back( "ZLLIB" ) ;
	list.push_back( "ZTLIB" ) ;
	list.push_back( "ZTABL" ) ;
	list.push_back( "ZFILE" ) ;

	map<string, string> listusr = { { "ZFILE", "ZFILU" },
					{ "ZMLIB", "ZMUSR" },
					{ "ZPLIB", "ZPUSR" },
					{ "ZSLIB", "ZSUSR" },
					{ "ZLLIB", "ZLUSR" },
					{ "ZTLIB", "ZTUSR" },
					{ "ZTABL", "ZTABU" } } ;

	for ( auto it = zlibd.begin() ; it != zlibd.end() ; ++it )
	{
		if ( !findword( it->first, sys_libdefs ) )
		{
			list.push_back( it->first ) ;
		}
	}

	for ( auto itx = list.begin() ; itx != list.end() ; ++itx )
	{
		if ( libdef != "" && libdef.size() < 9 && libdef != *itx && libdef != "ISPPROF"  )
		{
			continue ;
		}
		auto it = zlibd.find( *itx ) ;
		if ( it == zlibd.end() )
		{
			stk   = "" ;
			libx  = *itx ;
			type  = "" ;
			usr   = "" ;
			ident = "** LIBDEF not active **" ;
			tbadd( table ) ;
		}
		else if ( findword( it->first, sys_libdefs ) )
		{
			libx = it->first ;
			if ( it->second.top().library() )
			{
				usr   = "" ;
				type  = "LIBRARY" ;
				ident = it->second.top().ddname ;
				tbadd( table ) ;
				type = "" ;
				libx = "" ;
			}
			else
			{
				usr   = "X"    ;
				type  = "PATH" ;
				zxusr = listusr[ it->first ] ;
				vget( zxusr, PROFILE ) ;
				vcopy( zxusr, t, MOVE ) ;
				p = getpaths( t ) ;
				for ( size_t i = 1 ; i <= p ; ++i )
				{
					ident = getpath( t, i ) ;
					tbadd( table ) ;
					type = "" ;
					libx = "" ;
				}
				usr = "" ;
			}
			stk     = "" ;
			stacked = false ;
			while ( true )
			{
				p = getpaths( it->second.top().paths ) ;
				for ( size_t i = 1 ; i <= p ; ++i )
				{
					ident = getpath( it->second.top().paths, i ) ;
					stk   = ( stacked && type != "" ) ? "S" : "" ;
					tbadd( table ) ;
					libx = "" ;
					type = "" ;
				}
				it->second.pop() ;
				if ( it->second.empty() ) { break ; }
				libx    = it->first ;
				stacked = true ;
				if ( it->second.top().library() )
				{
					usr   = "" ;
					type  = "LIBRARY" ;
					ident = it->second.top().ddname ;
					stk   = "S" ;
					tbadd( table ) ;
					type = "" ;
					libx = "" ;
				}
				else
				{
					type  = "PATH" ;
				}
			}
		}
		else
		{
			while ( !it->second.empty() )
			{
				libx = it->first ;
				usr  = "" ;
				stk  = "" ;
				if ( it->second.top().library() )
				{
					type  = "LIBRARY" ;
					ident = it->second.top().ddname ;
					tbadd( table ) ;
					type = "" ;
					libx = "" ;
				}
				else
				{
					type = "PATH" ;
				}
				p = getpaths( it->second.top().paths ) ;
				for ( size_t i = 1 ; i <= p ; ++i )
				{
					ident = getpath( it->second.top().paths, i ) ;
					stk   = ( stacked && type != "" ) ? "S" : "" ;
					tbadd( table ) ;
					type = "" ;
					libx = "" ;
				}
				stacked = true ;
				it->second.pop() ;
			}
		}
	}

	ztdtop = 1 ;
	addpop( "", 5, 5 ) ;

	while ( true )
	{
		tbtop( table ) ;
		tbskip( table, ztdtop ) ;
		if ( msg == "" ) { zcmd  = "" ; }
		tbdispl( table,
			 "PPSP01LD",
			 msg,
			 "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
	}

	rempop() ;

	tbend( table ) ;
	vdelete( vlist1 ) ;
}


/**************************************************************************************************************/
/**********************************         DISPLAY COMMAND TABLES          ***********************************/
/**************************************************************************************************************/

void ppsp01a::showCommandTables()
{
	string cmdtab   ;
	string ocmdtab  ;
	string zctverb  ;
	string zcttrunc ;
	string zctact   ;
	string zctdesc  ;
	string msg      ;
	string applcmd  ;
	string applcmdl ;
	string panel ;
	string table ;

	int ztdvrows ;

	bool first = true ;

	vdefine( "ZCTVERB ZCTTRUNC ZCTACT ZCTDESC", &zctverb, &zcttrunc, &zctact, &zctdesc ) ;
	vdefine( "ZAPPLID CMDTAB APPLCMD APPLCMDL ZVERB", &zapplid, &cmdtab, &applcmd, &applcmdl, &zverb ) ;
	vdefine( "ZTDVROWS", &ztdvrows ) ;

	vget( "ZAPPLID" ) ;
	vget( "CMDTAB", PROFILE ) ;
	if ( cmdtab == "" ) { cmdtab = "ISP" ; }

	applcmd  = "" ;
	applcmdl = "" ;
	if ( zapplid != "ISP" )
	{
		applcmd = zapplid ;
		tbopen( applcmd+"CMDS", NOWRITE, "", SHARE ) ;
		if ( RC > 4 )
		{
			applcmdl = "Application Command Table Not Found" ;
		}
		else
		{
			applcmdl = "" ;
			tbend( applcmd+"CMDS" ) ;
		}
	}
	msg = "" ;

	table = cmdtab + "CMDS" ;
	tbopen( table, NOWRITE, "", SHARE ) ;
	if ( RC > 0 )
	{
		cmdtab = "ISP" ;
		table  = "ISPCMDS" ;
		tbopen( table, NOWRITE, "", SHARE ) ;
	}
	ztdtop = 1 ;
	panel  = "PPSP01AC" ;

	control( "PASSTHRU", "LRSCROLL", "PASON" ) ;

	ocmdtab = cmdtab ;
	while ( true )
	{
		tbtop( table ) ;
		tbskip( table, ztdtop ) ;
		zcmd  = "" ;
		tbdispl( table,
			 panel,
			 msg ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		if ( ocmdtab != cmdtab )
		{
			table = cmdtab + "CMDS" ;
			tbopen( table, NOWRITE, "", SHARE ) ;
			if ( RC == 0 )
			{
				table = ocmdtab + "CMDS" ;
				tbend( table ) ;
				ocmdtab = cmdtab ;
				table   = cmdtab + "CMDS" ;
				ztdtop  = 1 ;
			}
			else
			{
				cmdtab = ocmdtab ;
				table  = cmdtab + "CMDS" ;
				msg    = "PPSP014"  ;
			}
		}
		if ( ztdvrows == 0 )
		{
			panel = "" ;
			msg   = "PSYZ006" ;
			continue ;
		}
		vget( "ZVERB", SHARED ) ;
		first = ( zverb == "LEFT"  ) ? true  :
			( zverb == "RIGHT" ) ? false : first ;
		panel = ( first ) ? "PPSP01AC" : "PPSP01AD" ;
	}

	tbend( table ) ;
	vput( "CMDTAB", PROFILE ) ;

}


/**************************************************************************************************************/
/**********************************         DISPLAY LOADED MODULES          ***********************************/
/**************************************************************************************************************/

void ppsp01a::showLoadedClasses()
{
	int crp ;
	int csrrow ;

	uint j ;

	bool ref ;

	string w1  ;
	string w2  ;
	string w3  ;
	string msg ;
	string psort ;
	string panel ;
	string cursor ;

	string tabName ;
	string appl    ;
	string mod     ;
	string modpath ;
	string status  ;
	string filter  ;
	string modact  ;
	string modrest ;

	lspfCommand lc ;

	const string vlist1 = "SEL APPL MOD MODPATH STATUS MODACT MODREST" ;
	const string vlist2 = "CRP" ;

	vdefine( vlist1, &sel, &appl, &mod, &modpath, &status, &modact, &modrest ) ;
	vdefine( vlist2, &crp ) ;

	tabName = "MODL" + d2ds( taskid(), 4 ) ;

	msg    = ""    ;
	filter = "*"   ;
	ztdtop = 1     ;
	ref    = true  ;
	psort  = "(APPL,C,A)" ;
	cursor = "ZCMD" ;
	csrrow = 0      ;

	while ( true )
	{
		if ( ref )
		{
			tbcreate( tabName,
				  "APPL",
				  "(SEL,MOD,MODPATH,STATUS)",
				  NOWRITE,
				  REPLACE ) ;
			tbsort( tabName, psort ) ;
			lc.Command = "MODULE STATUS" ;
			lspfCallback( lc ) ;
			for ( j = 0 ; j < lc.reply.size() ; ++j )
			{
				sel     = "" ;
				appl    = lc.reply[   j ] ;
				mod     = lc.reply[ ++j ] ;
				modpath = lc.reply[ ++j ] ;
				modpath = modpath.substr( 0, modpath.find_last_of( '/' ) ) ;
				status  = lc.reply[ ++j ] ;
				tbadd( tabName, "", "ORDER" ) ;
			}
			ref = false ;
		}
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( tabName ) ;
			tbskip( tabName, ztdtop ) ;
			panel = "PPSP01ML" ;
		}
		tbtop( tabName ) ;
		tbskip( tabName, ztdtop ) ;
		if ( msg == "" ) { zcmd = "" ; }
		tbvclear( tabName ) ;
		appl   = filter ;
		tbsarg( tabName ) ;
		vreplace( "ZTDMSG", ( filter == "*" ) ? "PSYZ003" : "" ) ;
		tbdispl( tabName,
			 panel,
			 msg,
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC == 8 ) { break ; }
		panel  = "" ;
		cursor = ( sel == "" ) ? "" : "SEL" ;
		csrrow = crp ;
		msg    = "" ;
		zcmd   = "" ;
		if ( ztdsels == 0 && zcmd == "" ) { ref = true ; }
		w1 = word( zcmd, 1 ) ;
		w2 = word( zcmd, 2 ) ;
		w3 = word( zcmd, 3 ) ;
		if ( w1 == "SORT" )
		{
			if ( w2 == "" ) { w2 = "APPL" ; }
			if ( w3 == "" ) { w3 = "A"    ; }
			if      ( abbrev( "MODULES", w2, 3 ) )      { psort = "(MOD,C,"+ w3 +")"     ; }
			else if ( abbrev( "APPLICATIONS", w2, 3 ) ) { psort = "(APPL,C,"+ w3 +")"    ; }
			else if ( abbrev( "PATHS", w2, 3 ) )        { psort = "(MODPATH,C,"+ w3 +")" ; }
			else if ( abbrev( "STATUS", w2, 3 ) )       { psort = "(STATUS,C,"+ w3 +")"  ; }
			else                                        { msg   = "PSYS018" ; continue   ; }
			tbsort( tabName, psort ) ;
			continue ;
		}
		if ( modact == "RESET" )
		{
			filter = "*" ;
			ztdtop = 1 ;
			continue ;
		}
		else if ( modact == "FILTER" )
		{
			filter = word( modrest, 1 ) + "*" ;
			ztdtop = 1 ;
			continue ;
		}
		if ( sel == "R" )
		{
			lc.Command = "MODREL " + appl ;
			lspfCallback( lc ) ;
		}
	}

	tbend( tabName ) ;
	vdelete( vlist1, vlist2 ) ;
}


/**************************************************************************************************************/
/**********************************             MOUSE ACTIONS               ***********************************/
/**************************************************************************************************************/

void ppsp01a::mouseActions()
{
	//
	// Setup the ZMOUSExx variables in the profile to control mouse actions.
	//

	string msg ;
	string zcmd ;

	string zmousein ;
	string zmousesa ;
	string zmouse11 ;
	string zmouse12 ;
	string zmouse13 ;
	string zmouse21 ;
	string zmouse22 ;
	string zmouse23 ;
	string zmouse31 ;
	string zmouse32 ;
	string zmouse33 ;

	string bmousein ;
	string bmousesa ;
	string bmouse11 ;
	string bmouse12 ;
	string bmouse13 ;
	string bmouse21 ;
	string bmouse22 ;
	string bmouse23 ;
	string bmouse31 ;
	string bmouse32 ;
	string bmouse33 ;

	string mmousein ;
	string mmousesa ;

	string m1mousa1 ;
	string m1mousc1 ;
	string m1mousa2 ;
	string m1mousc2 ;
	string m1mousa3 ;
	string m1mousc3 ;

	string m2mousa1 ;
	string m2mousc1 ;
	string m2mousa2 ;
	string m2mousc2 ;
	string m2mousa3 ;
	string m2mousc3 ;

	string m3mousa1 ;
	string m3mousc1 ;
	string m3mousa2 ;
	string m3mousc2 ;
	string m3mousa3 ;
	string m3mousc3 ;

	string* vargs[ 27 ] ;

	const string vlist1a = "ZMOUSEIN ZMOUSESA ZMOUSE11 ZMOUSE12 ZMOUSE13" ;
	const string vlist1b = "ZMOUSE21 ZMOUSE22 ZMOUSE23 ZMOUSE31 ZMOUSE32 ZMOUSE33" ;

	const string vlist2a = "MMOUSEIN MMOUSESA M1MOUSA1 M1MOUSC1 M1MOUSA2 M1MOUSC2 M1MOUSA3 M1MOUSC3" ;
	const string vlist2b = "M2MOUSA1 M2MOUSC1 M2MOUSA2 M2MOUSC2 M2MOUSA3 M2MOUSC3" ;
	const string vlist2c = "M3MOUSA1 M3MOUSC1 M3MOUSA2 M3MOUSC2 M3MOUSA3 M3MOUSC3" ;

	const string vlist3 = "ZCMD" ;

	const string vlist4a = "KMOUSEIN KMOUSESA KMOUSE11 KMOUSE12 KMOUSE13" ;
	const string vlist4b = "KMOUSE21 KMOUSE22 KMOUSE23 KMOUSE31 KMOUSE32 KMOUSE33" ;

	vdefine( vlist1a, &bmousein, &bmousesa, &bmouse11, &bmouse12, &bmouse13 ) ;
	vdefine( vlist1b, &bmouse21, &bmouse22, &bmouse23, &bmouse31, &bmouse32, &bmouse33 ) ;
	vget( vlist1a, PROFILE ) ;
	vget( vlist1b, PROFILE ) ;
	vdelete( vlist1a, vlist1b ) ;

	vdefine( vlist1a, &zmousein, &zmousesa, &zmouse11, &zmouse12, &zmouse13 ) ;
	vdefine( vlist1b, &zmouse21, &zmouse22, &zmouse23, &zmouse31, &zmouse32, &zmouse33 ) ;

	vdefine( vlist2a, &mmousein, &mmousesa, &m1mousa1, &m1mousc1, &m1mousa2, &m1mousc2, &m1mousa3, &m1mousc3 ) ;
	vdefine( vlist2b, &m2mousa1, &m2mousc1, &m2mousa2, &m2mousc2, &m2mousa3, &m2mousc3 ) ;
	vdefine( vlist2c, &m3mousa1, &m3mousc1, &m3mousa2, &m3mousc2, &m3mousa3, &m3mousc3 ) ;

	vdefine( vlist3, &zcmd ) ;

	vget( vlist1a, PROFILE ) ;
	vget( vlist1b, PROFILE ) ;

	vargs[ 0  ] = &zmouse11 ;
	vargs[ 1  ] = &zmouse12 ;
	vargs[ 2  ] = &zmouse13 ;
	vargs[ 3  ] = &zmouse21 ;
	vargs[ 4  ] = &zmouse22 ;
	vargs[ 5  ] = &zmouse23 ;
	vargs[ 6  ] = &zmouse31 ;
	vargs[ 7  ] = &zmouse32 ;
	vargs[ 8  ] = &zmouse33 ;
	vargs[ 9  ] = &m1mousa1 ;
	vargs[ 10 ] = &m1mousc1 ;
	vargs[ 11 ] = &m1mousa2 ;
	vargs[ 12 ] = &m1mousc2 ;
	vargs[ 13 ] = &m1mousa3 ;
	vargs[ 14 ] = &m1mousc3 ;
	vargs[ 15 ] = &m2mousa1 ;
	vargs[ 16 ] = &m2mousc1 ;
	vargs[ 17 ] = &m2mousa2 ;
	vargs[ 18 ] = &m2mousc2 ;
	vargs[ 19 ] = &m2mousa3 ;
	vargs[ 20 ] = &m2mousc3 ;
	vargs[ 21 ] = &m3mousa1 ;
	vargs[ 22 ] = &m3mousc1 ;
	vargs[ 23 ] = &m3mousa2 ;
	vargs[ 24 ] = &m3mousc2 ;
	vargs[ 25 ] = &m3mousa3 ;
	vargs[ 26 ] = &m3mousc3 ;

	mouseActions_load( vargs ) ;
	mmousein = zmousein ;
	mmousesa = zmousesa ;

	while ( true )
	{
		display( "PPSP01M1", msg, "ZCMD" ) ;
		if ( RC == 8 && zcmd != "CANCEL" ) { break ; }
		msg = "" ;
		if ( zcmd == "CANCEL" || zcmd == "RESET" )
		{
			vdefine( vlist1a, &bmousein, &bmousesa, &bmouse11, &bmouse12, &bmouse13 ) ;
			vdefine( vlist1b, &bmouse21, &bmouse22, &bmouse23, &bmouse31, &bmouse32, &bmouse33 ) ;
			vput( vlist1a, PROFILE ) ;
			vput( vlist1b, PROFILE ) ;
			mouseinterval( ds2d( bmousein ) ) ;
			if ( zcmd == "CANCEL" ) { break ; }
			vdelete( vlist1a, vlist1b ) ;
			vget( vlist1a, PROFILE ) ;
			vget( vlist1b, PROFILE ) ;
			mouseActions_load( vargs ) ;
			mmousein = zmousein ;
			mmousesa = zmousesa ;
		}
		else if ( zcmd == "DEFAULTS" )
		{
			zmousein = "166" ;
			zmousesa = "1" ;
			zmouse11 = "2" ;
			zmouse12 = "4" ;
			zmouse13 = "6ZHISTORY" ;
			zmouse21 = "5SWAP NEXT" ;
			zmouse22 = "5SPLIT NEW" ;
			zmouse23 = "6:TS" ;
			zmouse31 = "5END" ;
			zmouse32 = "5RETURN" ;
			zmouse33 = "1" ;
			vput( vlist1a, PROFILE ) ;
			vput( vlist1b, PROFILE ) ;
			mouseActions_load( vargs ) ;
			mmousein = zmousein ;
			mmousesa = zmousesa ;
			mouseinterval( ds2d( zmousein ) ) ;
		}
		else if ( zcmd == "SAVE" )
		{
			vdefine( vlist4a, &zmousein, &zmousesa, &zmouse11, &zmouse12, &zmouse13 ) ;
			vdefine( vlist4b, &zmouse21, &zmouse22, &zmouse23, &zmouse31, &zmouse32, &zmouse33 ) ;
			vput( vlist4a, PROFILE ) ;
			vput( vlist4b, PROFILE ) ;
			vdelete( vlist4a, vlist4b ) ;
			msg = "PPSP016A" ;
		}
		else if ( zcmd == "RESTORE" )
		{
			vdefine( vlist4a, &zmousein, &zmousesa, &zmouse11, &zmouse12, &zmouse13 ) ;
			vdefine( vlist4b, &zmouse21, &zmouse22, &zmouse23, &zmouse31, &zmouse32, &zmouse33 ) ;
			vget( vlist4a, PROFILE ) ;
			vget( vlist4b, PROFILE ) ;
			vdelete( vlist4a, vlist4b ) ;
			vput( vlist1a, PROFILE ) ;
			vput( vlist1b, PROFILE ) ;
			mouseActions_load( vargs ) ;
			mmousein = zmousein ;
			mmousesa = zmousesa ;
			mouseinterval( ds2d( zmousein ) ) ;
			msg = "PPSP016B" ;
		}
		else
		{
			zmousein = mmousein ;
			zmousesa = mmousesa ;
			zmouse11 = m1mousa1 + m1mousc1 ;
			zmouse12 = m1mousa2 + m1mousc2 ;
			zmouse13 = m1mousa3 + m1mousc3 ;
			zmouse21 = m2mousa1 + m2mousc1 ;
			zmouse22 = m2mousa2 + m2mousc2 ;
			zmouse23 = m2mousa3 + m2mousc3 ;
			zmouse31 = m3mousa1 + m3mousc1 ;
			zmouse32 = m3mousa2 + m3mousc2 ;
			zmouse33 = m3mousa3 + m3mousc3 ;
			vput( vlist1a, PROFILE ) ;
			vput( vlist1b, PROFILE ) ;
			mouseinterval( ds2d( zmousein ) ) ;
		}
		zcmd = "" ;
	}

	vdelete( "*" ) ;
}


void ppsp01a::mouseActions_load( string** vargs )
{
	string& zmouse11  = *vargs[ 0  ] ;
	string& zmouse12  = *vargs[ 1  ] ;
	string& zmouse13  = *vargs[ 2  ] ;
	string& zmouse21  = *vargs[ 3  ] ;
	string& zmouse22  = *vargs[ 4  ] ;
	string& zmouse23  = *vargs[ 5  ] ;
	string& zmouse31  = *vargs[ 6  ] ;
	string& zmouse32  = *vargs[ 7  ] ;
	string& zmouse33  = *vargs[ 8  ] ;
	string& m1mousa1  = *vargs[ 9  ] ;
	string& m1mousc1  = *vargs[ 10 ] ;
	string& m1mousa2  = *vargs[ 11 ] ;
	string& m1mousc2  = *vargs[ 12 ] ;
	string& m1mousa3  = *vargs[ 13 ] ;
	string& m1mousc3  = *vargs[ 14 ] ;
	string& m2mousa1  = *vargs[ 15 ] ;
	string& m2mousc1  = *vargs[ 16 ] ;
	string& m2mousa2  = *vargs[ 17 ] ;
	string& m2mousc2  = *vargs[ 18 ] ;
	string& m2mousa3  = *vargs[ 19 ] ;
	string& m2mousc3  = *vargs[ 20 ] ;
	string& m3mousa1  = *vargs[ 21 ] ;
	string& m3mousc1  = *vargs[ 22 ] ;
	string& m3mousa2  = *vargs[ 23 ] ;
	string& m3mousc2  = *vargs[ 24 ] ;
	string& m3mousa3  = *vargs[ 25 ] ;
	string& m3mousc3  = *vargs[ 26 ] ;

	m1mousa1 = zmouse11.substr( 0, 1 ) ;
	m1mousc1 = zmouse11.substr( 1 ) ;
	m1mousa2 = zmouse12.substr( 0, 1 ) ;
	m1mousc2 = zmouse12.substr( 1 ) ;
	m1mousa3 = zmouse13.substr( 0, 1 ) ;
	m1mousc3 = zmouse13.substr( 1 ) ;

	m2mousa1 = zmouse21.substr( 0, 1 ) ;
	m2mousc1 = zmouse21.substr( 1 ) ;
	m2mousa2 = zmouse22.substr( 0, 1 ) ;
	m2mousc2 = zmouse22.substr( 1 ) ;
	m2mousa3 = zmouse23.substr( 0, 1 ) ;
	m2mousc3 = zmouse23.substr( 1 ) ;

	m3mousa1 = zmouse31.substr( 0, 1 ) ;
	m3mousc1 = zmouse31.substr( 1 ) ;
	m3mousa2 = zmouse32.substr( 0, 1 ) ;
	m3mousc2 = zmouse32.substr( 1 ) ;
	m3mousa3 = zmouse33.substr( 0, 1 ) ;
	m3mousc3 = zmouse33.substr( 1 ) ;
}


/**************************************************************************************************************/
/**********************************         SAVED FILE LIST UTILITY         ***********************************/
/**************************************************************************************************************/

void ppsp01a::showSavedFileList()
{
	int i ;

	string zfile ;
	string zfiln ;
	string zcurr ;
	string zdir  ;
	string msg   ;

	vdefine( "ZCURR ZFILE ZDIR", &zcurr, &zfile, &zdir ) ;

	msg = "" ;
	while ( true )
	{
		display( "PPSP01FL", msg, "ZCMD" ) ;
		if ( RC == 8 ) { return ; }

		if ( zfile != "" )
		{
			if ( zfile == "*" ) { zfile = "" ; }
			if ( zfile != "" && zfile[ 0 ] == '/' )
			{
				 zfiln = zfile ;
			}
			else if ( zdir != "" )
			{
				zfiln = zdir + "/" + zfile ;
			}
			else if ( zcurr != "" )
			{
				zfiln = zcurr + "/" + zfile ;
			}
			else
			{
				continue ;
			}
			if ( is_directory( zfiln ) )
			{
				select( "PGM(" + get_dialogue_var( "ZFLSTPGM" ) + ") PARM(" + zfiln + ")" ) ;
			}
			else if ( is_regular_file( zfiln ) )
			{
				browse( zfiln ) ;
			}
			continue ;
		}
		for ( i = 1 ; i < 9 ; ++i )
		{
			vcopy( "SEL" + d2ds(i), sel, MOVE ) ;
			if ( sel == "" || RC == 8 ) { continue ; }
			vreplace( "SEL" + d2ds(i), "" ) ;
			vcopy( "ZFILE" + d2ds(i), zfiln, MOVE ) ;
			if ( zfiln == "" || RC == 8 ) { continue ; }
			if ( sel == "S" || sel == "L" )
			{
				if ( is_directory( zfiln ) )
				{
					select( "PGM(" + get_dialogue_var( "ZFLSTPGM" ) + ") PARM(" + zfiln + ")" ) ;
				}
			}
			else if ( sel == "B" )
			{
				if ( is_regular_file( zfiln ) )
				{
					browse( zfiln ) ;
				}
			}
			else if ( sel == "E" )
			{
				if ( is_regular_file( zfiln ) )
				{
					edit( zfiln ) ;
				}
			}
		}
	}
}


/**************************************************************************************************************/
/**********************************             SPECIAL KEYS                ***********************************/
/**************************************************************************************************************/

void ppsp01a::specialKeys()
{
}


/**************************************************************************************************************/
/**********************************           SET UTILITY PROGRAMS          ***********************************/
/**************************************************************************************************************/

void ppsp01a::utilityPrograms()
{
	int RCode ;

	string kmainpgm ;
	string kmainpan ;
	string kpanlpgm ;
	string keditpgm ;
	string kbrpgm   ;
	string kviewpgm ;
	string kflstpgm ;
	string khelppgm ;
	string korexpgm ;
	string kshelpgm ;
	string kfhstpgm ;

	const string vlist1 = "ZMAINPGM ZMAINPAN ZPANLPGM ZEDITPGM ZBRPGM ZVIEWPGM ZFLSTPGM ZHELPPGM" ;
	const string vlist2 = "ZOREXPGM ZSHELPGM ZFHSTPGM" ;

	vdefine( vlist1, &kmainpgm, &kmainpan, &kpanlpgm, &keditpgm, &kbrpgm, &kviewpgm, &kflstpgm, &khelppgm ) ;
	vdefine( vlist2, &korexpgm, &kshelpgm, &kfhstpgm ) ;

	vget( vlist1, PROFILE ) ;
	vget( vlist2, PROFILE ) ;

	while ( true )
	{
		zcmd = "" ;
		display( "PPSP01UP", "", "ZCMD" ) ;
		RCode = RC ;

		if ( kmainpgm == "" ) { kmainpgm = ZMAINPGM ; } ;
		if ( kmainpan == "" ) { kmainpan = ZMAINPAN ; } ;
		if ( kpanlpgm == "" ) { kpanlpgm = ZPANLPGM ; } ;
		if ( keditpgm == "" ) { keditpgm = ZEDITPGM ; } ;
		if ( kbrpgm   == "" ) { kbrpgm   = ZBRPGM   ; } ;
		if ( kviewpgm == "" ) { kviewpgm = ZVIEWPGM ; } ;
		if ( kflstpgm == "" ) { kflstpgm = ZFLSTPGM ; } ;
		if ( khelppgm == "" ) { khelppgm = ZHELPPGM ; } ;
		if ( korexpgm == "" ) { korexpgm = ZOREXPGM ; } ;
		if ( kshelpgm == "" ) { kshelpgm = ZSHELPGM ; } ;
		if ( kfhstpgm == "" ) { kfhstpgm = ZFHSTPGM ; } ;

		if ( zcmd == "CANCEL" ) { break ; }
		if ( zcmd == "DEFAULTS" )
		{
			kmainpgm = ZMAINPGM ;
			kmainpan = ZMAINPAN ;
			kpanlpgm = ZPANLPGM ;
			keditpgm = ZEDITPGM ;
			kbrpgm   = ZBRPGM   ;
			kviewpgm = ZVIEWPGM ;
			kflstpgm = ZFLSTPGM ;
			khelppgm = ZHELPPGM ;
			korexpgm = ZOREXPGM ;
			kshelpgm = ZSHELPGM ;
			kfhstpgm = ZFHSTPGM ;
		}

		if ( RCode == 8 || zcmd == "SAVE" )
		{
			vput( vlist1, PROFILE ) ;
			vput( vlist2, PROFILE ) ;
			if ( RCode == 8 ) { break ; }
		}
	}

	vdelete( vlist1, vlist2 ) ;
}


/**************************************************************************************************************/
/**********************************       NON-KEYLIST PF KEY SETTINGS       ***********************************/
/**************************************************************************************************************/

void ppsp01a::pfkeySettings()
{
	int RCode ;

	string suf ;
	string panel = "PPSP01J1" ;

	string* t ;

	map<int, string> pfk ;
	map<int, string> pfl ;

	vector<string> pfKeyDefaults = { { "HELP"      },
					 { "SPLIT NEW" },
					 { "END"       },
					 { "RETURN"    },
					 { "RFIND"     },
					 { "RCHANGE"   },
					 { "UP"        },
					 { "DOWN"      },
					 { "SWAP"      },
					 { "LEFT"      },
					 { "RIGHT"     },
					 { "RETRIEVE"  },
					 { "HELP"      },
					 { "SPLIT NEW" },
					 { "END"       },
					 { "RETURN"    },
					 { "RFIND"     },
					 { "RCHANGE"   },
					 { "UP"        },
					 { "DOWN"      },
					 { "SWAP"      },
					 { "SWAP PREV" },
					 { "SWAP NEXT" },
					 { "HELP"      } } ;

	for ( int i = 1 ; i <= 24 ; ++i )
	{
		suf = d2ds( i, 2 ) ;
		vget( "ZPF"+suf, PROFILE ) ;
		vget( "ZPFL"+suf, PROFILE ) ;
		vcopy( "ZPF"+suf, pfk[ i ] ) ;
		vcopy( "ZPFL"+suf, pfl[ i ] ) ;
	}

	while ( true )
	{
		zcmd  = "" ;
		display( panel );
		RCode = RC ;
		if ( zcmd == "CANCEL" )
		{
			for ( int i = 1 ; i <= 24 ; ++i )
			{
				suf = d2ds( i, 2 ) ;
				vreplace( "ZPF"+suf, pfk[ i ] ) ;
				vreplace( "ZPFL"+suf, pfl[ i ] ) ;
				vput( "ZPF"+suf, PROFILE ) ;
				vput( "ZPFL"+suf, PROFILE ) ;
			}
			break ;
		}
		if ( zcmd == "" )
		{
			panel = ( panel == "PPSP01J1" ) ? "PPSP01J2" : "PPSP01J1" ;
		}
		if ( zcmd == "DEFAULTS" )
		{
			for ( int i = 1 ; i <= 24 ; ++i )
			{
				suf = d2ds( i, 2 ) ;
				vreplace( "ZPF"+suf, "" ) ;
				vreplace( "ZPFL"+suf, "" ) ;
			}
		}
		for ( int i = 1 ; i <= 24 ; ++i )
		{
			suf = d2ds( i, 2 ) ;
			vcopy( "ZPF"+suf, t, LOCATE ) ;
			if ( *t == "" )
			{
				vreplace( "ZPF"+suf, pfKeyDefaults[ i-1 ] ) ;
			}
			vput( "ZPF"+suf, PROFILE ) ;
			vput( "ZPFL"+suf, PROFILE ) ;
		}
		if ( RCode > 0 ) { break ; }
	}
}


/**************************************************************************************************************/
/**********************************             KEYLIST SETTINGS            ***********************************/
/**************************************************************************************************************/

void ppsp01a::keylistSettings()
{
	//
	// Change active keylist.
	//

	bool newrec = false ;
	bool newtab = false ;

	string keylistn ;
	string uprof ;
	string tab ;
	string msg ;
	string aklist ;
	string aktab ;
	string aktyp ;

	const string vlist1 = "KEYLISTN TBK1TAB" ;

	vcopy( "ZKLNAME", aklist, MOVE ) ;
	vcopy( "ZKLAPPL", aktab, MOVE ) ;
	vcopy( "ZKLTYPE", aktyp, MOVE ) ;
	vcopy( "ZUPROF", uprof, MOVE ) ;

	if ( aktab == "" )
	{
		pfkeySettings() ;
		return ;
	}

	vdefine( vlist1, &keylistn, &tab ) ;

	if ( aktyp == "S" )
	{
		tab = aktab + "KEYS" ;
		tbopen( tab, NOWRITE ) ;
		keylistn = aklist ;
		tbget( tab ) ;
		tbend( tab ) ;
		tab = aktab + "KEYP" ;
		tbopen( tab, WRITE, uprof ) ;
		if ( RC == 8 )
		{
			createKeyTable( tab, uprof ) ;
			tbopen( tab, WRITE, uprof ) ;
			newtab = true ;
		}
		tbget( tab ) ;
		if ( RC == 8 )
		{
			tbadd( tab ) ;
			newrec = true ;
		}
		else
		{
			tbmod( tab ) ;
		}
		tbsort( tab, "(KEYLISTN,C,A)" ) ;
		tbclose( tab, "", uprof ) ;
	}
	else
	{
		tab = aktab + "KEYP" ;
	}

	if ( !editKeylist( msg, tab, aklist ) && newrec )
	{
		if ( newtab )
		{
			deleteKeyTable( tab, uprof ) ;
		}
		else
		{
			deleteKeylist( tab, uprof, aklist ) ;
		}
	}

	if ( msg != "" ) { setmsg( msg ) ; }

	vdelete( vlist1 ) ;
}


/**************************************************************************************************************/
/**********************************             KEYLIST UTILITY             ***********************************/
/**************************************************************************************************************/

void ppsp01a::keylistUtility()
{
	//
	// For command keylist.
	//
	// klst1sel = 1 - Show current panel keylist (starting default).
	// klst1sel = 2 - Show current dialogue keylist.
	// klst1sel = 3 - Show specified application id keylist.
	//

	string aklist ;
	string aktab ;
	string oktab ;
	string aktyp ;

	string klst1sel ;

	const string vlist1 = "KLST1SEL" ;

	vdefine( vlist1, &klst1sel ) ;

	vcopy( "ZKLNAME", aklist, MOVE ) ;
	vcopy( "ZKLAPPL", aktab, MOVE ) ;
	vcopy( "ZKLTYPE", aktyp, MOVE ) ;

	klst1sel = "1" ;
	oktab    = aktab ;

	while ( true )
	{
		if ( klst1sel == "1" )
		{
			aktab = oktab ;
			if ( aktab == "" )
			{
				vcopy( "ZAPPLID", aktab, MOVE ) ;
				oktab = aktab ;
			}
		}
		else if ( klst1sel == "2" )
		{
			vcopy( "ZAPPLID", aktab, MOVE ) ;
		}
		klst1sel = "0" ;
		if ( !keylistUtility_display( klst1sel,
					      aklist,
					      aktab,
					      aktyp ) ) { break ; }
	}

	vdelete( vlist1 ) ;
}


bool ppsp01a::keylistUtility_display( string& klst1sel,
				      const string& aklist,
				      string& aktab,
				      const string& aktyp )
{
	//
	// Show keylists (SHARED and PRIVATE) for ZKLAPPL, current or spcecified profile.
	//

	int RC1 ;
	int crp ;
	int csrrow ;

	bool rebuild  = true ;
	bool newrec ;
	bool newtab ;

	string msg ;
	string panel ;
	string cursor ;

	string tbk1sel ;
	string tbk1lst ;
	string tbk1typ ;
	string tbk1msg ;
	string tbk1tab ;
	string keylistn ;
	string table ;
	string uprof ;
	string newkey ;
	string shartab ;
	string privtab ;
	string klst2sel = klst1sel ;

	const string vlist1 = "TBK1SEL TBK1LST TBK1TYP TBK1MSG TBK1TAB KEYLISTN NEWKEY" ;
	const string vlist2 = "CRP" ;

	shartab = aktab + "KEYS" ;
	privtab = aktab + "KEYP" ;

	vdefine( vlist1, &tbk1sel, &tbk1lst, &tbk1typ, &tbk1msg, &tbk1tab, &keylistn, &newkey ) ;
	vdefine( vlist2, &crp ) ;

	tbk1tab = privtab ;

	table = "KLT2" + d2ds( taskid(), 4 ) ;

	vcopy( "ZUPROF", uprof, MOVE ) ;

	ztdtop = 1  ;
	msg    = "" ;
	cursor = "ZCMD" ;
	csrrow = 0  ;

	while ( true )
	{
		if ( rebuild )
		{
			keylistUtility_load( privtab,
					     shartab,
					     uprof,
					     keylistn,
					     aklist,
					     aktyp,
					     table,
					     tbk1lst,
					     tbk1sel,
					     tbk1typ,
					     tbk1msg,
					     tbk1tab ) ;
			rebuild = false ;
		}
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( table ) ;
			tbskip( table, ztdtop ) ;
			panel = "PPSP01K8" ;
		}
		tbdispl( table,
			 panel,
			 msg,
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC == 8 ) { break ; }
		zcmd   = "" ;
		panel  = "" ;
		msg    = "" ;
		cursor = ( tbk1sel == "" ) ? "" : "TBK1SEL" ;
		csrrow = crp ;
		if ( klst1sel == "3" )
		{
			addpop( "", 5, 5 ) ;
			display( "PPSP01K9" ) ;
			if ( RC == 8 )
			{
				klst1sel = klst2sel ;
				rempop() ;
				continue ;
			}
			rempop() ;
			vcopy( "KEYAPPL", aktab ) ;
			tbend( table ) ;
			vdelete( vlist1, vlist2 ) ;
			return true ;
		}
		else if ( klst1sel != klst2sel )
		{
			tbend( table ) ;
			vdelete( vlist1, vlist2 ) ;
			return true ;
		}
		if ( tbk1sel == "D" )
		{
			if ( tbk1tab == shartab )
			{
				msg = "PPSP013E" ;
			}
			else
			{
				addpop( "", 5, 5 ) ;
				display( "PPSP01K7", msg, "ZCMD" ) ;
				RC1 = RC ;
				rempop() ;
				if ( RC1 == 0 )
				{
					deleteKeylist( privtab, uprof, tbk1lst ) ;
					deleteEmptyKeyTable( privtab, uprof ) ;
					rebuild = true ;
				}
			}
		}
		else if ( tbk1sel == "N" )
		{
			addpop( "", 5, 5 ) ;
			display( "PPSP01K4", msg, "ZCMD" ) ;
			RC1 = RC ;
			rempop() ;
			if ( RC1 == 0 )
			{
				tbopen( privtab, NOWRITE, uprof ) ;
				if ( RC == 8 )
				{
					createKeyTable( privtab, uprof ) ;
					tbopen( privtab, NOWRITE, uprof ) ;
				}
				tbvclear( privtab ) ;
				keylistn = newkey ;
				tbget( privtab, "", "", "NOREAD" ) ;
				if ( RC == 0 )
				{
					msg = "PPSP014D" ;
					tbend( privtab ) ;
				}
				else
				{
					tbend( privtab ) ;
					tbopen( tbk1tab, NOWRITE, ( tbk1typ == "PRIVATE" ? uprof : "" ) ) ;
					if ( RC > 0 )
					{
						llog( "E", "Unexpected return code from TBOPEN."<< endl ) ;
						llog( "E", "Table: "<< tbk1tab << endl ) ;
						llog( "E", "RC   : "<< RC << endl ) ;
						uabend() ;
					}
					keylistn = tbk1lst ;
					tbget( tbk1tab ) ;
					tbend( tbk1tab ) ;
					tbopen( privtab, WRITE, uprof ) ;
					keylistn = newkey ;
					tbadd( privtab ) ;
					if ( RC == 0 )
					{
						tbsort( privtab, "(KEYLISTN,C,A)" ) ;
						tbclose( privtab, "", uprof ) ;
						control( "DISPLAY", "SAVE" ) ;
						tbk1tab = privtab ;
						editKeylist( msg, privtab, newkey ) ;
						control( "DISPLAY", "RESTORE" ) ;
						rebuild = true ;
					}
				}
			}
		}
		else if ( tbk1sel == "E" )
		{
			newrec = false ;
			newtab = false ;
			if ( tbk1typ == "SHARED" )
			{
				tbopen( tbk1tab, NOWRITE ) ;
				keylistn = tbk1lst ;
				tbget( tbk1tab ) ;
				tbend( tbk1tab ) ;
				tbopen( privtab, WRITE, uprof ) ;
				if ( RC == 8 )
				{
					createKeyTable( privtab, uprof ) ;
					tbopen( privtab, WRITE, uprof ) ;
					newtab = true ;
				}
				tbget( privtab ) ;
				if ( RC == 8 )
				{
					tbadd( privtab ) ;
					newrec = true ;
				}
				else
				{
					tbmod( privtab ) ;
				}
				tbsort( privtab, "(KEYLISTN,C,A)" ) ;
				tbclose( privtab, "", uprof ) ;
			}
			control( "DISPLAY", "SAVE" ) ;
			if ( !editKeylist( msg, privtab, tbk1lst ) && newrec )
			{
				if ( newtab )
				{
					deleteKeyTable( privtab, uprof ) ;
				}
				else
				{
					deleteKeylist( privtab, uprof, tbk1lst ) ;
				}
				newrec = false ;
			}
			control( "DISPLAY", "RESTORE" ) ;
			if ( newrec )
			{
				rebuild = true ;
			}
		}
		else if ( tbk1sel == "V" )
		{
			control( "DISPLAY", "SAVE" ) ;
			viewKeylist( tbk1tab, tbk1lst ) ;
			control( "DISPLAY", "RESTORE" ) ;
			tbk1sel = ""  ;
			tbput( table ) ;
		}
	}

	tbend( table ) ;
	vdelete( vlist1, vlist2 ) ;

	return false ;
}


void ppsp01a::keylistUtility_load( const string& privtab,
				   const string& shartab,
				   const string& uprof,
				   const string& keylistn,
				   const string& aklist,
				   const string& aktyp,
				   const string& table,
				   string& tbk1lst,
				   string& tbk1sel,
				   string& tbk1typ,
				   string& tbk1msg,
				   string& tbk1tab )
{
	//
	// PRIVATE keys are read from ZUPROF.
	// SHARED keys are read from ZTLIB.
	//

	bool sharopen ;
	bool privopen ;

	const char* actmsg = "*** Active ***" ;

	tbcreate( table,
		  "TBK1LST",
		  "(TBK1SEL,TBK1TYP,TBK1MSG,TBK1TAB)",
		  NOWRITE,
		  REPLACE ) ;

	tbopen( privtab, NOWRITE, uprof ) ;
	privopen = ( RC == 0 ) ;
	while ( privopen )
	{
		tbskip( privtab ) ;
		if ( RC > 0 ) { break ; }
		tbvclear( table ) ;
		tbk1lst = keylistn ;
		tbk1typ = "PRIVATE" ;
		tbk1tab = privtab ;
		if ( keylistn == aklist )
		{
			tbk1msg = actmsg ;
		}
		tbadd( table ) ;
		if ( RC > 0 )
		{
			llog( "E", "Unexpected return code from TBADD."<< endl ) ;
			llog( "E", "Table: "<< table << endl ) ;
			llog( "E", "RC   : "<< RC << endl ) ;
			uabend() ;
		}
	}
	if ( privopen )
	{
		tbend( privtab ) ;
	}

	tbopen( shartab, NOWRITE ) ;
	sharopen = ( RC == 0 ) ;
	while ( sharopen )
	{
		tbskip( shartab ) ;
		if ( RC > 0 ) { break ; }
		tbvclear( table ) ;
		tbk1lst = keylistn ;
		tbk1typ = "SHARED" ;
		tbk1tab = shartab ;
		if ( keylistn == aklist )
		{
			tbk1msg = actmsg ;
		}
		tbadd( table ) ;
	}
	if ( sharopen )
	{
		tbend( shartab ) ;
	}

	tbsort( table, "(TBK1LST,C,A)" ) ;
}


/**************************************************************************************************************/
/**********************************           KEYLISTS UTILITIES            ***********************************/
/**************************************************************************************************************/

void ppsp01a::keylistTables()
{
	//
	// For command keylists.
	//
	// Show a list of all keylist tables in the ZUPROF and ZTLIB paths.
	// If there are no tables found, create an empty ISPKEYP.
	//

	int RC1 ;
	int crp ;
	int csrrow ;

	bool rebuild  = true ;

	string msg ;
	string panel ;
	string cursor ;

	string tbk1sel ;
	string tbk1key ;
	string tbk1tab ;
	string tbk1typ ;
	string tbk1msg ;
	string tbk1dir ;
	string table   ;
	string uprof   ;
	string newtab  ;
	string newshr  ;

	string aktab   ;
	string aklist  ;
	string aktyp   ;

	const string vlist1 = "TBK1SEL TBK1KEY TBK1TAB TBK1TYP TBK1MSG TBK1DIR NEWTAB NEWSHR" ;
	const string vlist2 = "CRP" ;

	vdefine( vlist1, &tbk1sel, &tbk1key, &tbk1tab, &tbk1typ, &tbk1msg, &tbk1dir, &newtab, &newshr ) ;
	vdefine( vlist2, &crp ) ;

	table = "KEYP" + d2ds( taskid(), 4 ) ;

	vcopy( "ZUPROF", uprof, MOVE ) ;
	vcopy( "ZKLNAME", aklist, MOVE ) ;
	vcopy( "ZKLAPPL", aktab, MOVE ) ;
	vcopy( "ZKLTYPE", aktyp, MOVE ) ;

	ztdtop = 1 ;
	msg    = "" ;
	cursor = "ZCMD" ;
	csrrow = 0  ;

	while ( true )
	{
		if ( rebuild )
		{
			keylistTables_load( uprof,
					    aktab,
					    aktyp,
					    table,
					    tbk1sel,
					    tbk1key,
					    tbk1tab,
					    tbk1typ,
					    tbk1msg,
					    tbk1dir ) ;
			rebuild = false ;
		}
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( table ) ;
			tbskip( table, ztdtop ) ;
			panel = "PPSP01K1" ;
		}
		if ( msg == "" ) { zcmd = "" ; }
		tbdispl( table,
			 panel,
			 msg,
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC == 8 ) { break ; }
		panel  = "" ;
		msg    = "" ;
		cursor = ( tbk1sel == "" ) ? "" : "TBK1SEL" ;
		csrrow = crp ;
		if ( tbk1sel == "D" )
		{
			addpop( "", 5, 5 ) ;
			display( "PPSP01K7", msg, "ZCMD" ) ;
			RC1 = RC ;
			rempop() ;
			if ( RC1 == 0 )
			{
				remove( tbk1dir + tbk1tab ) ;
				rebuild = true ;
			}
		}
		else if ( tbk1sel == "N" )
		{
			addpop( "", 5, 5 ) ;
			display( "PPSP01K5", msg, "ZCMD" ) ;
			if ( RC == 0 )
			{
				tbk1tab = newtab + ( ( newshr == "/" ) ? "KEYS" : "KEYP" ) ;
				createKeyTable( tbk1tab, ( newshr == "/" ) ? tbk1dir : "" ) ;
				rebuild = true ;
			}
			rempop() ;
		}
		else if ( tbk1sel == "S" )
		{
			control( "DISPLAY", "SAVE" ) ;
			keylistTable( tbk1tab, tbk1dir, aktab, aklist, aktyp ) ;
			control( "DISPLAY", "RESTORE" ) ;
			tbk1sel = ""  ;
			tbput( table ) ;
		}
	}

	vdelete( vlist1, vlist2 ) ;
}


void ppsp01a::keylistTables_load( const string& uprof,
				  const string& aktab,
				  const string& aktyp,
				  const string& table,
				  string& tbk1sel,
				  string& tbk1key,
				  string& tbk1tab,
				  string& tbk1typ,
				  string& tbk1msg,
				  string& tbk1dir )
{
	uint wpaths ;

	string t ;
	string p ;
	string tab ;
	string fname ;
	string paths ;

	const char* actmsg = "*** Active ***" ;

	set<string> added ;

	using vec = vector<path> ;

	vec v ;
	vec::const_iterator it ;

	tbcreate( table,
		  "TBK1TAB",
		  "(TBK1SEL,TBK1KEY,TBK1TYP,TBK1MSG,TBK1DIR)",
		  NOWRITE,
		  REPLACE ) ;
	if ( RC > 4 ) { uabend() ; }

	copy( directory_iterator( uprof ), directory_iterator(), back_inserter( v ) ) ;

	for ( it = v.begin() ; it != v.end() ; ++it )
	{
		fname = it->string() ;
		p     = fname.substr( 0, fname.find_last_of( '/' ) + 1 ) ;
		tab   = fname.substr( fname.find_last_of( '/' ) + 1 ) ;
		if ( tab.size() > 4 && tab.compare( tab.size()-4, 4, "KEYP" ) == 0 )
		{
			tbvclear( table ) ;
			tbk1key = tab.substr( 0, tab.size() - 4 ) ;
			tbk1tab = tab ;
			tbk1typ = "PRIVATE" ;
			tbk1dir = p ;
			if ( aktyp == "P" && tab.compare( 0, aktab.size(), aktab ) == 0 )
			{
				tbk1msg = actmsg ;
			}
			tbadd( table ) ;
		}
	}

	v.clear() ;

	qlibdef( "ZTLIB", "", "PATHS" ) ;
	vcopy( "PATHS", paths, MOVE ) ;
	wpaths = getpaths( paths ) ;
	for ( uint i = 1 ; i <= wpaths ; ++i )
	{
		t = getpath( paths, i ) ;
		if ( added.count( t ) > 0 ) { continue ; }
		added.insert( t ) ;
		copy( directory_iterator( t ), directory_iterator(), back_inserter( v ) ) ;
	}

	vcopy( "ZTLIB", paths, MOVE ) ;
	wpaths = getpaths( paths ) ;
	for ( uint i = 1 ; i <= wpaths ; ++i )
	{
		t = getpath( paths, i ) ;
		if ( added.count( t ) > 0 ) { continue ; }
		added.insert( t ) ;
		copy( directory_iterator( t ), directory_iterator(), back_inserter( v ) ) ;
	}

	for ( it = v.begin() ; it != v.end() ; ++it )
	{
		fname = it->string() ;
		p     = fname.substr( 0, fname.find_last_of( '/' ) + 1 ) ;
		tab   = fname.substr( fname.find_last_of( '/' ) + 1 ) ;
		if ( tab.size() > 4 && tab.compare( tab.size()-4, 4, "KEYS" ) == 0 )
		{
			tbvclear( table ) ;
			tbk1tab = tab ;
			tbk1key = tab.substr( 0, tab.size() - 4 ) ;
			tbk1typ = "SHARED" ;
			tbk1dir = p ;
			if ( aktyp == "S" && tab.compare( 0, aktab.size(), aktab ) == 0 )
			{
				tbk1msg = actmsg ;
			}
			tbadd( table ) ;
		}
	}

	tbsort( table, "(TBK1TAB,C,A)" ) ;

	tbtop( table ) ;
	tbskip( table, 1 ) ;
	if ( RC == 8 )
	{
		createKeyTable( "ISPKEYP" ) ;
		tbvclear( table ) ;
		tbk1tab = "ISPKEYP" ;
		tbadd( table ) ;
	}
}


void ppsp01a::keylistTable( string tab,
			    const string& tabdir,
			    string aktab,
			    const string& aklist,
			    string aktyp )
{
	//
	// Show keylist table tab.
	// If there are no rows in table tab, create an empty xxxxDEF entry.
	//

	int RC1 ;
	int crp ;
	int csrrow ;

	string msg ;
	string panel ;
	string cursor ;

	string tbk2sel ;
	string tbk2lst ;
	string tbk2msg ;
	string keylistn ;
	string table ;
	string newkey ;

	const string vlist1 = "TBK2SEL TBK2LST TBK2MSG KEYLISTN NEWKEY" ;
	const string vlist2 = "CRP" ;

	const char* actmsg = "*** Active ***" ;

	if ( aktab == "" )
	{
		vcopy( "ZAPPLID", aktab, MOVE ) ;
	}

	vdefine( vlist1, &tbk2sel, &tbk2lst, &tbk2msg, &keylistn, &newkey ) ;
	vdefine( vlist2, &crp ) ;

	table = "KLT2" + d2ds( taskid(), 4 ) ;

	tbopen( tab, NOWRITE, tabdir ) ;
	if ( RC == 8 )
	{
		setmsg ( "PPSP011F" ) ;
		vdelete( vlist1, vlist2 ) ;
		return ;
	}
	else if ( RC > 8 )
	{
		llog( "E", "Error opening Keylist table "<< tab << ".  RC="<< RC << endl ) ;
		uabend() ;
	}

	tbcreate( table,
		  "",
		  "(TBK2SEL,TBK2LST,TBK2MSG)",
		  NOWRITE ) ;
	if ( RC > 0 )
	{
		llog( "E", "Error creating temporary table "<< table << ".  RC="<< RC << endl ) ;
		uabend() ;
	}

	tbtop( tab ) ;
	tbskip( tab, 1 ) ;
	if ( RC > 0 )
	{
		tbend( tab ) ;
		tbopen( tab, WRITE, tabdir ) ;
		if ( RC > 0 )
		{
			llog( "E", "Error opening Keylist table "<< tab <<" for update.  RC="<< RC << endl ) ;
			uabend() ;
		}
		tbsort( tab, "(KEYLISTN,C,A)" ) ;
		tbvclear( tab ) ;
		keylistn = tab.substr( 0, tab.size()-4 ) + "DEF" ;
		tbadd( tab, "", "ORDER" ) ;
		if ( RC > 0 ) { uabend() ; }
		tbclose( tab, "", tabdir ) ;
		tbopen( tab, NOWRITE, tabdir ) ;
		if ( RC > 0 ) { uabend() ; }
	}

	tbtop( tab ) ;
	while ( true )
	{
		tbskip( tab ) ;
		if ( RC > 0 ) { break ; }
		tbvclear( table ) ;
		tbk2lst = keylistn ;
		if ( aktyp != "" && tab.back() == aktyp.back() && tbk2lst == aklist )
		{
			tbk2msg = actmsg ;
		}
		tbadd( table ) ;
		if ( RC > 0 ) { uabend() ; }
	}
	tbend( tab ) ;

	tbsort( table, "(TBK2LST,C,A)" ) ;

	ztdtop = 1 ;
	msg    = "" ;
	cursor = "ZCMD" ;
	csrrow = 0  ;

	while ( true )
	{
		if ( ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( table ) ;
			tbskip( table, ztdtop ) ;
			panel = "PPSP01K2" ;
		}
		tbdispl( table,
			 panel,
			 msg,
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC == 8 ) { break ; }
		panel  = "" ;
		msg    = "" ;
		cursor = ( tbk2sel == "" ) ? "" : "TBK2SEL" ;
		csrrow = crp ;
		zcmd   = "" ;
		if ( tbk2sel == "D" )
		{
			addpop( "", 5, 5 ) ;
			display( "PPSP01K7", msg, "ZCMD" ) ;
			RC1 = RC ;
			rempop() ;
			if ( RC1 == 0 )
			{
				tbopen( tab, WRITE, tabdir ) ;
				if ( RC > 0 ) { uabend() ; }
				keylistn = tbk2lst ;
				tbdelete( tab )   ;
				if ( RC > 0 ) { uabend() ; }
				tbdelete( table ) ;
				if ( RC > 0 ) { uabend() ; }
				tbclose( tab, "", tabdir ) ;
			}
		}
		else if ( tbk2sel == "N" )
		{
			addpop( "", 5, 5 ) ;
			display( "PPSP01K4", msg, "ZCMD" ) ;
			RC1 = RC ;
			rempop() ;
			if ( RC1 == 0 )
			{
				tbopen( tab, NOWRITE, tabdir ) ;
				if ( RC > 0 ) { uabend() ; }
				tbvclear( tab ) ;
				keylistn = newkey ;
				tbget( tab ) ;
				if ( RC == 0 )
				{
					msg = "PPSP014D" ;
					tbend( tab ) ;
				}
				else
				{
					tbend( tab ) ;
					tbopen( tab, WRITE, tabdir ) ;
					if ( RC > 0 ) { uabend() ; }
					tbsort( tab, "(KEYLISTN,C,A)" ) ;
					tbsort( table, "(TBK2LST,C,A)" ) ;
					keylistn = tbk2lst ;
					tbget( tab ) ;
					keylistn = newkey ;
					tbadd( tab, "", "ORDER" ) ;
					tbclose( tab, "", tabdir ) ;
					tbk2lst = newkey ;
					tbadd( table, "", "ORDER" ) ;
					if ( RC > 0 ) { uabend() ; }
					control( "DISPLAY", "SAVE" ) ;
					editKeylist( msg, tab, newkey ) ;
					control( "DISPLAY", "RESTORE" ) ;
				}
				tbk2sel = ""  ;
				tbput( table ) ;
			}
		}
		else if ( tbk2sel == "E" )
		{
			control( "DISPLAY", "SAVE" ) ;
			editKeylist( msg, tab, tbk2lst ) ;
			control( "DISPLAY", "RESTORE" ) ;
			tbk2sel = ""  ;
			tbput( table ) ;
		}
		else if ( tbk2sel == "V" )
		{
			control( "DISPLAY", "SAVE" ) ;
			viewKeylist( tab, tbk2lst, tabdir ) ;
			control( "DISPLAY", "RESTORE" ) ;
			tbk2sel = ""  ;
			tbput( table ) ;
		}
	}

	tbend( table ) ;
	vdelete( vlist1, vlist2 ) ;
}


void ppsp01a::viewKeylist( const string& tab,
			   const string& list,
			   const string& tabdir )
{
	//
	// Field names: KEYLISTN KEYnDEF KEYnLAB KEYnATR (n=1 to 24).
	// TD Field names: KEYNUM KEYDEF KEYATTR KEYLAB.
	//
	// Read keylist from table tab, KEYLISTN list and create table display.
	//

	int i ;

	string t ;
	string keynum ;
	string keydef ;
	string keyattr ;
	string keylab ;

	string keylistn ;
	string table ;
	string tpath ;

	const string vlist1 = "KEYLISTN KEYNUM KEYDEF KEYATTR KEYLAB" ;

	table = "KLT4" + d2ds( taskid(), 4 ) ;

	if ( tabdir == "" && tab.back() == 'P' )
	{
		vcopy( "ZUPROF", tpath, MOVE ) ;
	}
	else
	{
		tpath = tabdir ;
	}

	tbopen( tab, NOWRITE, tpath ) ;
	if ( RC > 0 ) { uabend() ; }

	vdefine( vlist1, &keylistn, &keynum, &keydef, &keyattr, &keylab ) ;

	tbcreate( table,
		  "KEYNUM",
		  "(KEYDEF,KEYATTR,KEYLAB)",
		  NOWRITE ) ;
	if ( RC > 0 ) { uabend() ; }

	tbvclear( tab ) ;
	keylistn = list ;
	tbget( tab ) ;
	if ( RC > 0 ) { uabend() ; }

	vcopy( "KEYHELPN", t, MOVE ) ;
	vreplace( "KEYHELP", t ) ;
	for ( i = 1 ; i < 25 ; ++i )
	{
		keynum = "F"+left( d2ds( i ), 2 ) + ". . ." ;
		vcopy( "KEY"+ d2ds( i ) +"DEF", keydef, MOVE ) ;
		vcopy( "KEY"+ d2ds( i ) +"ATR", keyattr, MOVE ) ;
		vcopy( "KEY"+ d2ds( i ) +"LAB", keylab, MOVE ) ;
		tbadd( table ) ;
		if ( RC > 0 ) { uabend() ; }
	}
	tbend( tab ) ;

	ztdtop = 1 ;
	while ( true )
	{
		tbtop( table ) ;
		tbskip( table, ztdtop ) ;
		tbdispl( table,
			 "PPSP01K6",
			 "",
			 "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
	}

	tbend( table )  ;
	vdelete( vlist1 ) ;
}


bool ppsp01a::editKeylist( string& msg,
			   const string& tab,
			   const string& list,
			   const string& tabdir )
{
	//
	// Field names: KEYLISTN KEYnDEF KEYnLAB KEYnATR (n=1 to 24).
	// TD Field names: KEYNUM KEYDEF KEYATTR KEYLAB.
	//
	// Read keylist from table tab, KEYLISTN list and create table display.
	// Update tab/list from table display.
	//

	int i ;

	bool end_loop = false ;
	bool changed  = false ;

	string t ;
	string keynum ;
	string keydef ;
	string keyattr ;
	string keylab ;

	string keylistn ;
	string table ;
	string tpath ;

	enq( tab, list ) ;
	if ( RC == 8 )
	{
		setmsg( "PPSP014H" ) ;
		viewKeylist( tab, list, tabdir ) ;
		return false ;
	}

	const string vlist1 = "KEYLISTN KEYNUM KEYDEF KEYATTR KEYLAB" ;
	table = "KLT3" + d2ds( taskid(), 4 ) ;

	if ( tabdir == "" && tab.back() == 'P' )
	{
		vcopy( "ZUPROF", tpath, MOVE ) ;
	}
	else
	{
		tpath = tabdir ;
	}

	tbopen( tab, NOWRITE, tpath ) ;
	if ( RC > 0 )
	{
		llog( "E", "Open of table "+ tab +" failed.  RC="<< RC <<endl);
		uabend() ;
	}

	vdefine( vlist1, &keylistn, &keynum, &keydef, &keyattr, &keylab ) ;

	tbcreate( table,
		  "KEYNUM",
		  "(KEYDEF,KEYATTR,KEYLAB)",
		  NOWRITE ) ;

	tbvclear( tab ) ;
	keylistn = list ;
	tbget( tab ) ;
	if ( RC > 0 )
	{
		llog( "E", "TBGET of "+ list +" from table "+ tab +" failed.  RC="<< RC <<endl);
		uabend() ;
	}

	vcopy( "KEYHELPN", t, MOVE ) ;
	vreplace( "KEYHELP", t ) ;
	for ( i = 1 ; i < 25 ; ++i )
	{
		keynum = "F"+left( d2ds( i ), 2 ) + ". . ." ;
		vcopy( "KEY"+ d2ds( i ) +"DEF", keydef, MOVE ) ;
		vcopy( "KEY"+ d2ds( i ) +"ATR", keyattr, MOVE ) ;
		vcopy( "KEY"+ d2ds( i ) +"LAB", keylab, MOVE ) ;
		tbadd( table ) ;
	}
	tbend( tab ) ;

	ztdtop   = 1 ;
	msg      = "" ;
	while ( !end_loop )
	{
		tbtop( table ) ;
		tbskip( table, ztdtop ) ;
		if ( msg == "" ) { zcmd = "" ; }
		tbdispl( table,
			 "PPSP01K3",
			 msg,
			 "ZCMD" ) ;
		if ( zcmd == "CANCEL" )
		{
			zcmd = "" ;
			msg  = "PPSP014G" ;
			tbend( table ) ;
			vdelete( vlist1 ) ;
			deq( tab, list ) ;
			return false ;
		}
		if ( RC == 8 ) { end_loop = true ; }
		msg = "" ;
		while ( ztdsels > 0 )
		{
			changed = true ;
			tbmod( table ) ;
			if ( ztdsels > 1 )
			{
				tbdispl( table ) ;
				if ( RC > 4 ) { end_loop = true ; break ; }
			}
			else { ztdsels = 0 ; }
		}
	}

	if ( !changed )
	{
		zcmd = "" ;
		msg  = "PPSP014E" ;
		tbend( table ) ;
		vdelete( vlist1 ) ;
		deq( tab, list ) ;
		return false ;
	}

	tbopen( tab, WRITE, tpath ) ;

	tbvclear( tab ) ;
	keylistn = list ;
	tbget( tab ) ;

	vcopy( "KEYHELP", t, MOVE ) ;
	vreplace( "KEYHELPN", t ) ;
	tbtop( table ) ;
	for ( i = 1 ; i < 25 ; ++i )
	{
		tbskip( table, 1 ) ;
		if ( RC > 0 ) { break ; }
		vreplace( "KEY"+ d2ds( i ) +"DEF", keydef ) ;
		vreplace( "KEY"+ d2ds( i ) +"ATR", keyattr ) ;
		vreplace( "KEY"+ d2ds( i ) +"LAB", keylab ) ;
	}

	tbmod( tab ) ;

	tbclose( tab, "", ( tpath == "" ) ? "ZTLIB" : tpath ) ;

	tbend( table ) ;

	vdelete( vlist1 ) ;

	zcmd = "" ;
	msg  = "PPSP014F" ;

	deq( tab, list ) ;
	return true ;
}


void ppsp01a::createKeyTable( const string& table,
			      const string& tabdir )
{
	//
	// Create an empty keylist table entry.
	//
	// If not specified, private keylists reside in the ZUPROF directory.
	//                   shared keylists reside in the ZTLIB search path.
	//

	int i ;

	string flds ;
	string tpath ;

	if ( tabdir == "" && table.back() == 'P' )
	{
		vcopy( "ZUPROF", tpath, MOVE ) ;
	}
	else
	{
		tpath = tabdir ;
	}

	for ( i = 1 ; i < 25 ; ++i )
	{
		flds += "KEY"+ d2ds( i ) +"DEF " ;
		flds += "KEY"+ d2ds( i ) +"ATR " ;
		flds += "KEY"+ d2ds( i ) +"LAB " ;
	}
	flds += "KEYHELPN" ;

	tbcreate( table,
		  "KEYLISTN",
		  "("+ flds +")",
		  WRITE,
		  NOREPLACE,
		  tpath ) ;
	if ( RC == 8 )
	{
		setmsg ( "PPSP013D" ) ;
		return ;
	}

	tbsave( table, "", ( tpath == "" ) ? "ZTLIB" : tpath ) ;

	tbend( table ) ;
}


void ppsp01a::deleteKeylist( const string& table,
			     const string& tabdir,
			     const string& tabkey )
{
	//
	// Delete row with a key of tabkey.
	//

	tbopen( table, WRITE, tabdir ) ;
	vreplace( "KEYLISTN", tabkey ) ;
	tbdelete( table ) ;
	tbclose( table, "", tabdir ) ;
}


void ppsp01a::deleteKeyTable( const string& table,
			      const string& tabdir )
{
	//
	// Delete a keytable.
	//

	tberase( table, tabdir ) ;
}


void ppsp01a::deleteEmptyKeyTable( const string& table,
				   const string& tabdir )
{
	//
	// Delete keytable if it is empty.
	//

	string numrows ;

	tbopen( table, NOWRITE, tabdir ) ;
	tbquery( table,
		 "",
		 "",
		 "TBQ1" ) ;
	vcopy( "TBQ1", numrows ) ;
	tbend( table ) ;

	if ( numrows == "00000000" )
	{
		tberase( table, tabdir ) ;
	}
}


/**************************************************************************************************************/
/**********************************            RUN APPLICATION              ***********************************/
/**************************************************************************************************************/

void ppsp01a::runApplication( const string& xappl )
{
	select( "PGM("+ xappl +") NEWAPPL(ISP) NEWPOOL PASSLIB" ) ;
}


/**************************************************************************************************************/
/**********************************             BROWSE ENTRY                ***********************************/
/**************************************************************************************************************/

void ppsp01a::browseEntry( string& file )
{
	string msg ;
	string zcmd ;
	string zfile   ;
	string zvmode  ;
	string zcurfld ;
	string showdir ;
	string beimac  ;
	string beprof  ;
	string berecl  ;
	string eelmac  ;
	string bebrom  ;
	string eeccan  ;
	string becwarn ;
	string edlctab ;

	string reflist = "REFLIST" ;

	boost::filesystem::path wd = boost::filesystem::current_path() ;

	const string vlist = "ZCMD ZCURFLD ZFILE SHOWDIR BEBROM BEIMAC BEPROF BERECL EELMAC EECCAN BECWARN ZVMODE" ;

	vdefine( vlist, &zcmd, &zcurfld, &zfile, &showdir, &bebrom, &beimac, &beprof, &berecl, &eelmac, &eeccan, &becwarn, &zvmode ) ;

	vget( "BEIMAC BERECL", SHARED ) ;
	vget( "ZFILE BEPROF BEBROM EECCAN EELMAC BECWARN BEDIRLST", PROFILE ) ;

	if ( bebrom != "/" )
	{
		zvmode = "VIEW" ;
		if ( editRecovery( zvmode ) == 8 )
		{
			vdelete( vlist ) ;
			return ;
		}
	}

	if ( file != "" )
	{
		if ( words( file ) > 1 )
		{
			reflist = word( file, 1 ) ;
			idelword( file, 1, 1 ) ;
		}
		try
		{
			exists( file ) ;
		}
		catch ( const filesystem_error& ex )
		{
			vreplace( "ZSTR1", ex.what() ) ;
			setmsg( "PEDT014Z" ) ;
			vdelete( vlist ) ;
			return ;
		}
		if ( file.find( '/' ) == string::npos )
		{
			select( "PGM(PLRFLST1) PARM(MTC "+ reflist +" " + file + ")" ) ;
			if ( ZRESULT != "" )
			{
				update_reflist( ZRESULT ) ;
				if ( bebrom == "/" )
				{
					browse( ZRESULT ) ;
				}
				else
				{
					view( ZRESULT,
					      "",
					      beimac,
					      beprof,
					      "",
					      eelmac,
					      ( eeccan  == "/" ) ? "YES" : "NO",
					      ( becwarn == "/" ) ? "YES" : "NO" ) ;
				}
				vdelete( vlist ) ;
				return ;
			}
			setmsg( ( ZRC == 8 ) ? "PBRO011W" : "PBRO011X" ) ;
			vdelete( vlist ) ;
			return ;
		}
		else if ( is_regular_file( file ) )
		{
			if ( file.front() != '/' ) { file = wd.native() + '/' + file ; }
			update_reflist( file ) ;
			if ( bebrom == "/" )
			{
				browse( file ) ;
			}
			else
			{
				view( file,
				      "",
				      beimac,
				      beprof,
				      "",
				      eelmac,
				      ( eeccan  == "/" ) ? "YES" : "NO",
				      ( becwarn == "/" ) ? "YES" : "NO" ) ;
			}
			vdelete( vlist ) ;
			return ;
		}
		else
		{
			vreplace( "ZVAL1", file ) ;
			vput( "ZVAL1", SHARED ) ;
			setmsg( "PBRO011X" ) ;
			vdelete( vlist ) ;
			return ;
		}
	}

	msg = "" ;
	while ( true )
	{
		display( "PBRO01A1", msg ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		if ( zcmd == "PROMPT" )
		{
			if ( zcurfld != "EELMAC" )
			{
				msg  = "PSYS022A" ;
				zcmd = "" ;
				continue ;
			}
			addpop( "EELMAC" ) ;
			edlctab = prompt_lmac() ;
			rempop() ;
			zcmd = "" ;
			if ( edlctab != "" )
			{
				eelmac = edlctab ;
			}
			continue ;
		}
		if ( showdir == "YES" )
		{
			listDirectory( zfile ) ;
			continue ;
		}
		update_reflist( zfile ) ;
		control( "ERRORS", "RETURN" ) ;
		if ( bebrom == "/" )
		{
			browse( zfile,
				"",
				"",
				( berecl == "" ) ? 0 : ds2d( berecl ) ) ;
		}
		else
		{
			view( zfile,
			      "",
			      beimac,
			      beprof,
			      "",
			      eelmac,
			      ( eeccan  == "/" ) ? "YES" : "NO",
			      ( becwarn == "/" ) ? "YES" : "NO" ) ;
		}
		if ( RC > 0 && RC < 20 && isvalidName( ZRESULT ) )
		{
			vget( "ZVAL1 ZVAL2 ZVAL3", SHARED ) ;
			msg = ZRESULT ;
		}
		else if ( RC == 20 )
		{
			getmsg( get_dialogue_var( "ZERRMSG" ), "ZERRSM", "ZERRLM" ) ;
			vreplace( "ZERR1", "Error in Browse" ) ;
			vput( "ZERR1 ZERRSM ZERRLM", SHARED ) ;
			showErrorScreen2( get_dialogue_var( "ZERRMSG" ) ) ;
		}
		control( "ERRORS", "CANCEL" ) ;
	}

	vdelete( vlist ) ;
}


/**************************************************************************************************************/
/**********************************              EDIT ENTRY                 ***********************************/
/**************************************************************************************************************/

void ppsp01a::editEntry( string& file )
{
	//
	// Display the Edit Entry panel.
	//

	string zcmd ;
	string zfile ;
	string zvmode ;
	string zcurfld ;
	string msg ;
	string showdir ;
	string eeimac ;
	string eeprof ;
	string eerecl ;
	string eelmac ;
	string eeccan ;
	string eeprsps ;
	string edlctab ;

	string reflist = "REFLIST" ;

	boost::filesystem::path wd = boost::filesystem::current_path() ;

	const string vlist = "ZCMD ZCURFLD ZFILE SHOWDIR EEIMAC EEPROF EERECL EELMAC EECCAN EEPRSPS ZVMODE" ;

	vdefine( vlist, &zcmd, &zcurfld, &zfile, &showdir, &eeimac, &eeprof, &eerecl, &eelmac, &eeccan, &eeprsps, &zvmode ) ;

	vget( "EEIMAC EERECL", SHARED ) ;
	vget( "ZFILE EEPROF EECCAN EELMAC EEPRSPS EEDIRLST EENEWFLS", PROFILE ) ;

	zvmode = "EDIT" ;
	if ( editRecovery( zvmode ) == 8 )
	{
		vdelete( vlist ) ;
		return ;
	}

	if ( file != "" )
	{
		if ( words( file ) > 1 )
		{
			reflist = word( file, 1 ) ;
			idelword( file, 1, 1 ) ;
		}
		try
		{
			exists( file ) ;
		}
		catch ( const filesystem_error& ex )
		{
			vreplace( "ZSTR1", ex.what() ) ;
			setmsg( "PEDT014Z" ) ;
			vdelete( vlist ) ;
			return ;
		}
		if ( file.find( '/' ) == string::npos )
		{
			select( "PGM(PLRFLST1) PARM(MTC "+ reflist +" " + file + ")" ) ;
			if ( ZRESULT != "" )
			{
				update_reflist( ZRESULT ) ;
				edit( ZRESULT ) ;
				vdelete( vlist ) ;
				return ;
			}
			setmsg( ( ZRC == 8 ) ? "PEDT017N" : "PEDT017O" ) ;
			vdelete( vlist ) ;
			return ;
		}
		else if ( is_regular_file( file ) )
		{
			if ( file.front() != '/' ) { file = wd.native() + '/' + file ; }
			update_reflist( file ) ;
			edit( file ) ;
			vdelete( vlist ) ;
			return ;
		}
		else
		{
			vreplace( "ZVAL1", file ) ;
			vput( "ZVAL1", SHARED ) ;
			setmsg( "PEDT017O" ) ;
			vdelete( vlist ) ;
			return ;
		}
	}
	msg = "" ;
	while ( true )
	{
		display( "PEDIT011", msg ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		if ( zcmd == "PROMPT" )
		{
			if ( zcurfld != "EELMAC" )
			{
				msg  = "PSYS022A" ;
				zcmd = "" ;
				continue ;
			}
			addpop( "EELMAC" ) ;
			edlctab = prompt_lmac() ;
			rempop() ;
			zcmd = "" ;
			if ( edlctab != "" )
			{
				eelmac = edlctab ;
			}
			continue ;
		}
		if ( showdir == "YES" )
		{
			listDirectory( zfile ) ;
			continue ;
		}
		update_reflist( zfile ) ;
		control( "ERRORS", "RETURN" ) ;
		edit( zfile,
		      "",
		      eeimac,
		      eeprof,
		      "",
		      eelmac,
		      ( eeccan  == "/" ) ? "YES" : "NO",
		      ( eeprsps == "/" ) ? "PRESERVE" : "",
		      "",
		      ( eerecl == "" ) ? 0 : ds2d( eerecl ) ) ;
		if ( RC > 0 && RC < 20 && isvalidName( ZRESULT ) )
		{
			vget( "ZVAL1 ZVAL2 ZVAL3", SHARED ) ;
			msg = ZRESULT ;
		}
		else if ( RC == 20 && ZRSN == 8 )
		{
			vget( "ZVAL1 ZVAL2 ZVAL3", SHARED ) ;
			msg = ZRESULT ;
		}
		else if ( RC == 20 )
		{
			getmsg( get_dialogue_var( "ZERRMSG" ), "ZERRSM", "ZERRLM" ) ;
			vreplace( "ZERR1", "Error in Edit" ) ;
			vput( "ZERR1 ZERRSM ZERRLM", SHARED ) ;
			showErrorScreen2( get_dialogue_var( "ZERRMSG" ) ) ;
		}
		control( "ERRORS", "CANCEL" ) ;
	}

	vdelete( vlist ) ;
}


int ppsp01a::editRecovery( const string& zvmode )
{
	string msg   ;
	string zcmd  ;
	string zfile ;
	string zedmode ;

	const string vlist = "ZCMD ZFILE" ;

	vdefine( vlist, &zcmd, &zfile ) ;

	edrec( "INIT" ) ;

	while ( true )
	{
		if ( msg == "" )
		{
			edrec( "QUERY" ) ;
			if ( RC == 0 ) { break ; }
			vcopy( "ZEDMODE", zedmode, MOVE ) ;
			if ( zvmode.compare( 0, 1, zedmode ) != 0 )
			{
				edrec( "DEFER" ) ;
				continue ;
			}
		}
		vcopy( "ZEDTFILE", zfile, MOVE ) ;
		display( "PEDIT014", msg, "ZCMD" ) ;
		if ( RC == 8 && zcmd != "CANCEL" )
		{
			vdelete( vlist ) ;
			return 8 ;
		}
		msg = "" ;
		if ( zcmd == "" )
		{
			control( "ERRORS", "RETURN" ) ;
			edrec( "PROCESS" ) ;
			if ( RC >= 12 )
			{
				vcopy( "ZERRMSG", msg, MOVE ) ;
			}
			control( "ERRORS", "CANCEL" ) ;
		}
		else if ( zcmd == "CANCEL" )
		{
			edrec( "CANCEL" ) ;
		}
		else
		{
			edrec( "DEFER" ) ;
		}
	}

	vdelete( vlist ) ;

	return 0 ;
}


void ppsp01a::listDirectory( string& file )
{
	if ( file.back() == '/' ) { file.pop_back() ; }

	vreplace( "ZPATH", file ) ;
	vput( "ZPATH", PROFILE ) ;

	select( "PGM(" + get_dialogue_var( "ZFLSTPGM" ) + ") PARM() SCRNAME(FILES)" ) ;
}


string ppsp01a::prompt_lmac()
{
	//
	// Diaplay a list of edit line command tables from ZTLIB.
	//

	int zcurinx ;

	string dataid ;
	string member ;
	string edtab ;
	string keys ;
	string names ;
	string tabName ;
	string seltab ;
	string zlclib ;

	const string vlist1 = "DATAID MEMBER EDTAB KEYS NAMES ZLCLIB" ;
	const string vlist2 = "ZCURINX" ;

	vdefine( vlist1, &dataid, &member, &edtab, &keys, &names, &zlclib ) ;
	vdefine( vlist2, &zcurinx ) ;

	lminit( "DATAID", "", "ZTLIB", "SHR" ) ;
	lmopen( dataid, "INPUT" );

	tabName = "EDLC" + d2ds( taskid(), 4 ) ;

	tbcreate( tabName,
		  "",
		  "(EDTAB)",
		  NOWRITE ) ;

	control( "ERRORS", "RETURN" ) ;

	lmmlist( dataid, "LIST", "MEMBER", "YES" );
	while ( RC == 0 )
	{
		if ( validTable( zlclib + member ) )
		{
			tbopen( member,
				NOWRITE,
				"ZTLIB",
				SHARE ) ;
			if ( RC == 0 )
			{
				tbquery( member, "KEYS", "NAMES" ) ;
				tbend( member ) ;
				if ( keys == "(ZELCNAM)" && names == "(ZELCMAC ZELCPGM ZELCBLK ZELCMLT ZELCDST)" )
				{
					edtab = member ;
					tbadd( tabName ) ;
				}
			}
		}
		lmmlist( dataid, "LIST", "MEMBER", "YES" );
	}

	control( "ERRORS", "CANCEL" ) ;

	lmmlist( dataid, "FREE" );
	lmclose( dataid );
	lmfree( dataid );

	tbtop( tabName ) ;

	tbdispl( tabName,
		 "PEDIT01L",
		 "",
		 "",
		 0,
		 1,
		 "NO" ) ;
	if ( RC > 4 )
	{
		tbend( tabName ) ;
		vdelete( vlist1, vlist2 ) ;
		return "" ;
	}
	if ( zcurinx > 0 )
	{
		tbtop( tabName ) ;
		tbskip( tabName, zcurinx ) ;
		seltab = edtab ;
	}

	tbend( tabName ) ;

	vdelete( vlist1, vlist2 ) ;

	return seltab ;
}


/**************************************************************************************************************/
/**********************************        SHOW ERROR SCREEN PSYSER2        ***********************************/
/**************************************************************************************************************/

void ppsp01a::showErrorScreen1()
{
	//
	// Show error screen PSYSER2 for message err.msgid and with variables:
	// ZERR1
	// ZERRSM
	// ZERRLM
	// ZERR2
	// ZERR3
	// ZERRMSG
	// ZERRRC
	//
	// Note: options structure only valid during application startup in this case
	//       as it then goes out of scope.
	//

	int l ;
	int maxw1 ;

	size_t i ;
	size_t maxw2 ;

	string* t ;

	struct err_struct
	{
		string title ;
		string src   ;
		errblock err ;
	} ;

	err_struct* errs = static_cast<err_struct*>( get_options() ) ;

	vdefine( "ZSCRMAXW", &maxw1 ) ;
	vget( "ZSCRMAXW", SHARED ) ;

	control( "ERRORS", "RETURN" ) ;

	vreplace( "ZERRMSG", errs->err.msgid ) ;
	vreplace( "ZVAL1",   errs->err.val1  ) ;
	vreplace( "ZVAL2",   errs->err.val2  ) ;
	vreplace( "ZVAL3",   errs->err.val3  ) ;
	vreplace( "ZERRRC",  "20"        ) ;
	vreplace( "ZERR1",   errs->title ) ;
	vreplace( "ZERR2",   "Panel line where error was detected:" ) ;
	vreplace( "ZERR3",   errs->src   ) ;

	getmsg( errs->err.msgid, "ZERRSM", "ZERRLM" ) ;

	vcopy( "ZERRLM", t, LOCATE ) ;

	maxw2 = maxw1 - 6 ;
	l = 0 ;
	do
	{
		++l ;
		if ( t->size() > maxw2 )
		{
			i = t->find_last_of( ' ', maxw2 ) ;
			i = ( i == string::npos ) ? maxw2 : i + 1 ;
			vreplace( "ZERRLM"+ d2ds( l ), t->substr( 0, i ) ) ;
			t->erase( 0, i ) ;
		}
		else
		{
			vreplace( "ZERRLM"+d2ds( l ), *t ) ;
			*t = "" ;
		}
	} while ( t->size() > 0 && l < 5 ) ;

	display( "PSYSER2" ) ;
}


/**************************************************************************************************************/
/**********************************        SHOW ERROR SCREEN PSYSER2        ***********************************/
/**************************************************************************************************************/

void ppsp01a::showErrorScreen2( string msg )
{
	//
	// Show error screen PSYSER2 for message 'msg'.
	//

	int l ;
	int maxw1 ;

	size_t i ;
	size_t maxw2 ;

	string* t ;

	vdefine( "ZSCRMAXW", &maxw1 ) ;
	vdefine( "ZERRMSG", &msg ) ;
	control( "ERRORS", "RETURN" ) ;

	vreplace( "ZERRMSG", msg ) ;
	vreplace( "ZERRRC", "20" ) ;

	vget( "ZSCRMAXW ZERRSM ZERRLM ZERR1 ZERR2 ZERR3", SHARED ) ;

	vcopy( "ZERRLM", t, LOCATE ) ;

	maxw2 = maxw1 - 6 ;
	l = 0 ;
	do
	{
		++l ;
		if ( t->size() > maxw2 )
		{
			i = t->find_last_of( ' ', maxw2 ) ;
			i = ( i == string::npos ) ? maxw2 : i + 1 ;
			vreplace( "ZERRLM"+ d2ds( l ), t->substr( 0, i ) ) ;
			t->erase( 0, i ) ;
		}
		else
		{
			vreplace( "ZERRLM"+d2ds( l ), *t ) ;
			*t = "" ;
		}
	} while ( t->size() > 0 ) ;

	display( "PSYSER2" ) ;
}

/**************************************************************************************************************/
/**********************************        SHOW ERROR SCREEN PSYSER3        ***********************************/
/**************************************************************************************************************/

void ppsp01a::showErrorScreen3( string& msg )
{
	//
	// Show error screen PSYSER3 for message 'msg'.
	//

	int l ;
	int maxw1 ;

	size_t i ;
	size_t maxw2 ;

	string* t ;

	vdefine( "ZSCRMAXW", &maxw1 ) ;
	vdefine( "ZERRMSG", &msg ) ;
	control( "ERRORS", "RETURN" ) ;

	vget( "ZSCRMAXW ZERRDSC ZERRSRC ZVAL1 ZVAL2 VAL3", SHARED ) ;
	getmsg( msg, "ZERRSM", "ZERRLM" ) ;

	vcopy( "ZERRLM", t, LOCATE ) ;

	maxw2 = maxw1 - 6 ;
	l = 0 ;
	do
	{
		++l ;
		if ( t->size() > maxw2 )
		{
			i = t->find_last_of( ' ', maxw2 ) ;
			i = ( i == string::npos ) ? maxw2 : i + 1 ;
			vreplace( "ZERRLM"+ d2ds( l ), t->substr( 0, i ) ) ;
			t->erase( 0, i ) ;
		}
		else
		{
			vreplace( "ZERRLM"+d2ds( l ), *t ) ;
			*t = "" ;
		}
	} while ( t->size() > 0 ) ;

	display( "PSYSER3" ) ;
}

/**************************************************************************************************************/
/**********************************                  HELD OUTPUT                   ****************************/
/**************************************************************************************************************/

void ppsp01a::showHeldOutput()
{
	int csrrow ;
	int crp ;

	string tabName ;
	string panel ;
	string msg ;
	string msgloc ;
	string csr ;

	string autosel = "YES" ;

	string jsel ;
	string jkey ;

	const string vlist = "JSEL JKEY" ;
	const string names = "(JSEL,JNAME,JTYPE,JNUM,JDATE,JTIME,JSTATUS,JLINES)" ;

	bool rebuild = false ;
	bool cur2sel = false ;

	map<string, int> keyLines ;
	set<string> complete ;

	vector<path> v ;

	tabName = "OLT" + d2ds( taskid(), 5 ) ;

	vdefine( vlist, &jsel, &jkey ) ;
	vdefine( "CRP", &crp ) ;

	tbcreate( tabName,
		  "JKEY",
		  names,
		  NOWRITE ) ;

	showHeldOutput_build( tabName, v, keyLines, complete ) ;

	msg = "" ;
	csr = "" ;
	csrrow = 0 ;
	while ( true )
	{
		if ( rebuild && ztdsels < 2 )
		{
			tbcreate( tabName,
				  "JKEY",
				  names,
				  NOWRITE,
				  REPLACE ) ;
			showHeldOutput_build( tabName, v, keyLines, complete ) ;
			rebuild = false ;
			csrrow  = 0 ;
		}
		if ( msg == "" && ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			if ( ztdvrows > 0 )
			{
				tbtop( tabName ) ;
				tbskip( tabName, ztdtop ) ;
			}
			else
			{
				tbbottom( tabName ) ;
				tbskip( tabName, - (ztddepth-2) ) ;
				if ( RC > 0 ) { tbtop( tabName ) ; }
			}
			panel = "PPSP01O1" ;
		}
		else
		{
			panel = "" ;
		}
		if ( msg != "" && csr == "" )
		{
			csr     = "JSEL" ;
			msgloc  = "JSEL" ;
			csrrow  = crp ;
			cur2sel = false ;
		}
		else if ( cur2sel )
		{
			csr     = "JSEL" ;
			msgloc  = "JSEL" ;
			csrrow  = crp ;
			cur2sel = false ;
			autosel = "NO" ;
		}
		else
		{
			csr = "ZCMD" ;
		}
		if ( msg == "" ) { zcmd = "" ; }
		tbdispl( tabName,
			 panel,
			 msg,
			 csr,
			 csrrow,
			 1,
			 autosel,
			 "CRP",
			 "",
			 msgloc ) ;
		if ( RC == 8 ) { break ; }
		msg     = "" ;
		csr     = "" ;
		autosel = "YES" ;
		csrrow  = 0 ;
		if ( ztdsels == 0 && zcurinx != 0 )
		{
			cur2sel = true ;
			tbtop( tabName ) ;
			tbskip( tabName, zcurinx, "", "", "", "", "CRP" ) ;
			showHeldOutput_display( jkey, v ) ;
			if ( ZRC == 4 && ZRSN == 4 )
			{
				setmsg( "PPSP011G" ) ;
				msgloc = "JSEL" ;
				csrrow = crp ;
			}
			continue ;
		}
		if ( jsel == "S" )
		{
			cur2sel = true ;
			showHeldOutput_display( jkey, v ) ;
			if ( ZRC == 4 && ZRSN == 4 )
			{
				msg = "PPSP011G" ;
				setmsg( "PPSP011G" ) ;
			}
		}
		else if ( jsel == "P" )
		{
			showHeldOutput_purge( jkey, v ) ;
			cur2sel = true ;
			rebuild = true ;
		}
		else
		{
			rebuild = true ;
		}
	}

	tbend( tabName ) ;

	vdelete( vlist ) ;
	vdelete( "CRP" ) ;
}


void ppsp01a::showHeldOutput_build( const string& tabName,
				    vector<path>& v,
				    map<string, int>& keyLines,
				    set<string>& complete )
{
	int total = 0 ;

	size_t p1 ;
	size_t p2 ;

	string zspool ;
	string tail   ;
	string inLine ;

	string entry  ;
	string jkey   ;
	string jname  ;
	string jtype  ;
	string jnum   ;
	string jdate  ;
	string jtime  ;
	string jstatus;

	std::ifstream fin ;

	lspfCommand lc ;

	set<string> batchKeys ;

	vector<path>::const_iterator it ;

	vget( "ZSPOOL", PROFILE ) ;
	vcopy( "ZSPOOL", zspool, MOVE ) ;

	v.clear() ;

	try
	{
		copy( directory_iterator( zspool ), directory_iterator(), back_inserter( v ) ) ;
	}
	catch ( const filesystem_error& ex )
	{
		llog( "E", "Error listing directory " << ex.what() << endl ) ;
		uabend( "PSYS013I" ) ;
	}

	sort( v.begin(), v.end() ) ;

	lc.Command = "BATCH KEYS" ;
	lspfCallback( lc ) ;
	for ( size_t j = 0 ; j < lc.reply.size() ; ++j )
	{
		batchKeys.insert( lc.reply[ j ] ) ;
	}

	jkey = "" ;
	for ( it = v.begin() ; it != v.end() ; ++it )
	{
		p1    = it->string().find_last_of( '/' ) ;
		entry = it->string().substr( p1 + 1 ) ;
		if ( entry.front() == '#' ) { continue ; }
		if ( jkey != "" &&
		     jkey != entry.substr( 0, 22 ) &&
		   ( complete.count( jkey ) == 0 ) )
		{
			keyLines[ jkey ] = total ;
			total = 0 ;
			if ( batchKeys.count( jkey ) == 0 )
			{
				complete.insert( jkey )  ;
			}
		}
		jkey = entry.substr( 0, 22 ) ;
		if ( complete.count( jkey ) == 0 )
		{
			fin.open( it->string() ) ;
			while ( getline( fin, inLine ) )
			{
				++total ;
			}
			fin.close() ;
		}
	}
	if ( jkey != "" && complete.count( jkey ) == 0 )
	{
		keyLines[ jkey ] = total ;
		if ( batchKeys.count( jkey ) == 0 )
		{
			complete.insert( jkey ) ;
		}
	}

	jkey = "" ;
	for ( it = v.begin() ; it != v.end() ; ++it )
	{
		p1    = it->string().find_last_of( '/' ) ;
		entry = it->string().substr( p1 + 1 ) ;
		if ( entry.front() == '#' || entry.compare( 0, 22, jkey ) == 0 ) { continue ; }
		jkey  = entry.substr( 0, 22 ) ;
		tail  = entry.substr( 23 ) ;
		jdate = jkey.substr( 0, 7 ) ;
		jtime = jkey.substr( 8, 8 ) ;
		jnum  = jkey.substr( 17, 5 ) ;
		p1    = tail.find( '-' ) ;
		jtype = tail.substr( 0, p1 ) ;
		p2    = tail.find( '-', p1+1 ) ;
		jname = tail.substr( p1+1, p2-p1-1 ) ;
		jdate.insert( 4, 1, '.' ) ;
		jtime.insert( 6, 1, '.' ) ;
		jtime.insert( 4, 1, ':' ) ;
		jtime.insert( 2, 1, ':' ) ;
		jstatus = ( batchKeys.count( jkey ) > 0 ) ? "EXECUTING" : "OUTPUT" ;
		tbvclear( tabName ) ;
		vreplace( "JNAME",   jname ) ;
		vreplace( "JTYPE",   jtype ) ;
		vreplace( "JNUM",    jnum  ) ;
		vreplace( "JKEY",    jkey  ) ;
		vreplace( "JDATE",   jdate ) ;
		vreplace( "JTIME",   jtime ) ;
		vreplace( "JSTATUS", jstatus ) ;
		vreplace( "JLINES",  d2ds( keyLines[ jkey ] ) ) ;
		tbadd( tabName ) ;
	}
}


void ppsp01a::showHeldOutput_display( const string& jkey,
				      vector<path>& v )
{
	size_t p ;

	string inLine ;
	string fname1 = get_tempname() ;

	vector<path>::const_iterator it ;

	std::ofstream fout ;
	std::ifstream fin  ;

	fout.open( fname1 ) ;
	for ( it = v.begin() ; it != v.end() ; ++it )
	{
		p = it->string().find_last_of( '/' ) ;
		if ( it->string().compare( p+1, 22, jkey ) == 0 )
		{
			fin.open( it->string() ) ;
			while ( getline( fin, inLine ) )
			{
				fout << inLine << endl ;
			}
			fin.close() ;
		}
	}
	fout.close() ;

	vreplace( "ZBRALT", "Output for job " + jkey ) ;
	vput( "ZBRALT", SHARED ) ;
	control( "ERRORS", "RETURN" ) ;
	browse( fname1 ) ;
	if ( RC == 12 )
	{
		setmsg( "PPSP011G" ) ;
	}
	control( "ERRORS", "CANCEL" ) ;
	remove( fname1 ) ;
	verase( "ZBRALT", SHARED ) ;
}


void ppsp01a::showHeldOutput_purge( const string& jkey,
				    vector<path>& v )
{
	size_t p ;

	vector<path>::const_iterator it ;

	for ( it = v.begin() ; it != v.end() ; ++it )
	{
		p = it->string().find_last_of( '/' ) ;
		if ( it->string().compare( p+1, 22, jkey ) == 0 )
		{
			remove( it->string() ) ;
		}
	}
}

/**************************************************************************************************************/
/**********************************              COMMAND TABLE UTILITY            *****************************/
/**************************************************************************************************************/

void ppsp01a::commandTableUtils()
{
	string msg ;
	string ztlib ;
	string zactb1 ;

	const string vlist1 = "ZACTB1 ZTLIB" ;

	vdefine( vlist1, &zactb1, &ztlib ) ;
	vget( "ZTLIB", PROFILE ) ;

	addpop( "", 5, 5 ) ;
	while ( true )
	{
		display( "PPSP01C0", msg ) ;
		if (RC == 8 ) {  break ; }
		msg = "" ;
		if ( commandTableEnqueue( ztlib, zactb1 ) )
		{
			commandTableDisplay( zactb1 ) ;
			commandTableDequeue( ztlib, zactb1 ) ;
		}
		else
		{
			msg = "PPSP011T" ;
		}
	}
	rempop() ;

	vdelete( vlist1 ) ;
}


void ppsp01a::commandTableDisplay( const string& zactb1 )
{
	int ppos   = 0 ;
	int crp    = 0 ;
	int crpx   = 0 ;
	int csrrow = 0 ;

	int sRC ;

	int ztdtop ;
	int ztdsels ;
	int ztdvrows ;

	bool open ;
	bool replaced ;
	bool updated  = false ;

	string zcmd ;
	string zverb ;
	string ztabl ;
	string cursor ;

	string sel ;
	string zctverb ;
	string zcttrunc ;
	string zctact ;
	string zctdesc ;

	string msg ;
	string msgloc ;
	string panel ;
	string table ;

	const string vlist1 = "ZVERB ZCMD ZTABL SEL ZCTVERB ZCTTRUNC ZCTACT ZCTDESC" ;
	const string vlist2 = "ZTDTOP ZTDSELS ZTDVROWS CRP" ;

	vdefine( vlist1, &zverb, &zcmd, &ztabl, &sel, &zctverb, &zcttrunc, &zctact, &zctdesc ) ;
	vdefine( vlist2, &ztdtop, &ztdsels, &ztdvrows, &crp ) ;

	table = "CTU" + d2ds( taskid(), 5 ) ;

	vget( "ZTABL", PROFILE ) ;
	vreplace( "FILE", ztabl ) ;

	commandTableLoad( zactb1,
			  table,
			  sel,
			  zctverb,
			  zcttrunc,
			  zctact,
			  zctdesc,
			  open ) ;

	ztdvrows = 1 ;
	cursor   = "ZCMD" ;
	panel    = ( open ) ? "PPSP01C3" : "PPSP01C1" ;

	while ( true )
	{
		if ( msg == "" && ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			if ( ztdvrows > 0 )
			{
				tbtop( table ) ;
				tbskip( table, ztdtop ) ;
			}
			else
			{
				tbbottom( table ) ;
				tbskip( table, ( 2 - ztddepth ) ) ;
				if ( RC > 0 ) { tbtop( table ) ; }
			}
			panel = ( open ) ? ( ppos == 0 ) ? "PPSP01C3" : "PPSP01C4" :
					   ( ppos == 0 ) ? "PPSP01C1" : "PPSP01C2" ;
		}
		control( "PASSTHRU", "LRSCROLL", "PASON" ) ;
		tbdispl( table,
			 panel,
			 msg,
			 cursor,
			 csrrow,
			 1,
			 "YES",
			 "CRP",
			 "",
			 msgloc ) ;
		sRC = RC ;
		control( "PASSTHRU", "LRSCROLL", "PASOFF" ) ;
		if ( sRC == 8 ) { break ; }
		cursor = "ZCMD" ;
		panel  = ""  ;
		msg    = ""  ;
		msgloc = ""  ;
		crpx   = crp ;
		crp    = 0   ;
		csrrow = 0   ;
		vget( "ZVERB", SHARED ) ;
		if ( zverb == "LEFT" && ppos > 0 )
		{
			--ppos ;
			continue ;
		}
		else if ( zverb == "RIGHT" && ppos < 1 )
		{
			++ppos ;
			continue ;
		}
		if ( sel == "D" )
		{
			tbdelete( table ) ;
			updated = true ;
		}
		else if ( sel == "E" )
		{
			commandTableEditEntry( table,
					       updated,
					       zctverb,
					       zcttrunc,
					       zctact,
					       zctdesc ) ;
		}
		else if ( sel == "I" )
		{
			updated = true ;
			tbvclear( table ) ;
			tbadd( table ) ;
		}
		else if ( sel == "R" )
		{
			sel     = "" ;
			updated = true ;
			tbadd( table ) ;
		}
		else if ( sel == "V" )
		{
			commandTableViewEntry() ;
		}
		else if ( ztdsels > 0 && !open )
		{
			if ( zctverb == "" )
			{
				zcttrunc = "" ;
			}
			else
			{
				if ( zcttrunc == "" )
				{
					zcttrunc = "0" ;
				}
				if ( !datatype( zcttrunc, 'W' ) || zctverb.size() < ds2d( zcttrunc ) )
				{
					msg    = "PPSP011R" ;
					cursor = "ZCTTRUNC" ;
					msgloc = "ZCTTRUNC" ;
					csrrow = crpx  ;
				}
				else
				{
					sel = "" ;
					tbput( table ) ;
					updated = true ;
				}
			}
		}
	}

	if ( updated && !open && zcmd != "CANCEL" )
	{
		commandTableSave( table,
				  zactb1,
				  zctverb,
				  replaced ) ;
		setmsg( ( replaced ) ? "PPSP011U" : "PPSP011V" ) ;
	}

	vdelete( vlist1, vlist2 ) ;

	tbend( table ) ;
}


void ppsp01a::commandTableLoad( const string& zactb1,
				const string& table,
				string& sel,
				string& zctverb,
				string& zcttrunc,
				string& zctact,
				string& zctdesc,
				bool& open )
{
	int rows = 0 ;

	bool exists ;

	string zctable = zactb1 + "CMDS" ;

	control( "ERRORS", "RETURN" ) ;
	tbopen( zctable, NOWRITE ) ;
	exists = ( RC == 0 || RC == 12 ) ;
	open   = ( RC == 12 ) ;
	control( "ERRORS", "CANCEL" ) ;

	if ( exists && !open )
	{
		tbend( zctable ) ;
	}

	tbcreate( table,
		"",
		"(SEL,ZCTVERB,ZCTTRUNC,ZCTACT,ZCTDESC)",
		NOWRITE,
		NOREPLACE ) ;

	if ( exists )
	{
		tbopen( zctable, NOWRITE, "", SHARE ) ;
		tbtop( zctable ) ;
		tbskip( zctable ) ;
		while ( RC == 0 )
		{
			tbadd( table ) ;
			tbskip( zctable ) ;
			++rows ;
		}
		tbend( zctable ) ;
	}

	if ( !open && rows == 0 )
	{
	       for ( uint i = 0 ; i < 17 ; i++ )
	       {
			tbadd( table ) ;
	       }
	}

	tbtop( table ) ;
}


void ppsp01a::commandTableViewEntry()
{
	addpop( "", 0, 0 ) ;
	while ( RC == 0 )
	{
		display( "PPSP01C5" ) ;
	}
	rempop() ;
}


void ppsp01a::commandTableEditEntry( const string& table,
				     bool& updated,
				     string& zctverb,
				     string& zcttrunc,
				     string& zctact,
				     string& zctdesc )
{
	string w1 ;
	string msg ;
	string zcmd ;
	string cursor ;
	string msgloc ;

	string zatverb ;
	string zattrunc ;
	string zatact ;
	string zatdesc ;

	const string vlist1 = "ZCMD ZATVERB ZATACT ZATTRUNC ZATDESC" ;
	const string valids = "SELECT ALIAS PASSTHRU SETVERB NOP" ;

	vdefine( vlist1, &zcmd, &zatverb, &zatact, &zattrunc, &zatdesc ) ;

	zatverb  = zctverb ;
	zattrunc = zcttrunc ;
	zatact   = zctact ;
	zatdesc  = zctdesc ;

	addpop( "", 0, 0 ) ;
	while ( true )
	{
		display( "PPSP01C6",
			 msg,
			 cursor,
			 1,
			 "",
			 "",
			 msgloc ) ;
		if ( RC == 8 && zcmd == "CANCEL" ) { break ; }
		msg    = "" ;
		cursor = "" ;
		msgloc = "" ;
		w1  = upper( word( zatact, 1 ) ) ;
		if ( w1 != "" )
		{
			zatact = w1 + " " + subword( zatact, 2 ) ;
			if ( !findword( w1, valids ) )
			{
				if ( w1.front() == '&' )
				{
					if ( !isvalidName( w1.substr( 1 ) ) )
					{
						msg    = "PPSP011P" ;
						cursor = "ZATVERB" ;
						msgloc = "ZATVERB" ;
					}
				}
				else
				{
					msg    = "PPSP011Q" ;
					cursor = "ZATACT" ;
					msgloc = "ZATACT" ;
				}
			}
		}
		if ( !datatype( zattrunc, 'W' ) || zatverb.size() < ds2d( zattrunc ) )
		{
			msg    = "PPSP011R" ;
			cursor = "ZATTRUNC" ;
			msgloc = "ZATTRUNC" ;
		}
		if ( msg == "" && RC == 8 ) { break ; }
		zcmd = "" ;
	}
	rempop() ;

	if ( zcmd != "CANCEL" )
	{
		tbvclear( table ) ;
		zctverb  = zatverb ;
		zcttrunc = zattrunc ;
		zctact   = zatact ;
		zctdesc  = zatdesc ;
		updated  = true ;
		tbput( table ) ;
		setmsg( "PPSP011S" ) ;
	}

	vdelete( vlist1 ) ;
}


void ppsp01a::commandTableSave( const string& table,
				const string& zactb1,
				const string& zctverb,
				bool& replaced )
{
	string zctable = zactb1 + "CMDS" ;

	tbcreate( zctable,
		  "",
		  "(ZCTVERB ZCTTRUNC ZCTACT ZCTDESC)",
		  WRITE,
		  REPLACE ) ;
	replaced = ( RC == 4 ) ;

	tbtop( table ) ;
	tbskip( table ) ;
	while ( RC == 0 )
	{
		if ( isvalidName( zctverb ) )
		{
			tbadd( zctable ) ;
		}
		tbskip( table ) ;
	}

	tbclose( zctable ) ;
}


bool ppsp01a::commandTableEnqueue( const string& ztlib,
				   const string& table )
{

	string rname = getpath( ztlib, 1 ) + table + "CMDS" ;

	enq( "SPFEDIT", rname ) ;

	return ( RC == 0 ) ;
}


void ppsp01a::commandTableDequeue( const string& ztlib,
				   const string& table )
{
	string rname = getpath( ztlib, 1 ) + table + "CMDS" ;

	deq( "SPFEDIT", rname ) ;
}

/**************************************************************************************************************/
/**********************************                 TABLE UTILITY                  ****************************/
/**************************************************************************************************************/

void ppsp01a::tableUtility()
{
	string msg ;
	string zcmd ;

	string ztudir ;
	string ztudd  ;
	string ztutnam ;

	const string vlist1 = "ZCMD ZTUDIR ZTUDD ZTUTNAM" ;

	vdefine( vlist1, &zcmd, &ztudir, &ztudd, &ztutnam ) ;

	while ( true )
	{
		display( "PPSP01T0", msg ) ;
		if (RC == 8 ) {  break ; }
		msg = "" ;
		tableUtility_list( msg,
				   ztudir,
				   ztudd,
				   ztutnam ) ;
	}

	vdelete( vlist1 ) ;
}


void ppsp01a::tableUtility_list( string& msg,
				 const string& ztudir,
				 const string& ztudd,
				 const string& ztutnam )
{
	int j ;
	int crp ;
	int csrrow ;
	int rows = 0 ;

	int ztdtop ;
	int ztdsels ;
	int ztdvrows ;

	bool exists ;

	string table = "LTU" + d2ds( taskid(), 5 ) ;

	string panel1 ;
	string panel2 ;
	string cursor ;

	string ztutnam1 ;
	string ztuconct ;
	string ztutdir1 ;
	string sortarg ;
	string fields ;
	string paths ;
	string entry ;
	string zcmd ;
	string sel ;

	const string vlist1 = "ZCMD SEL ZTUTNAM1 ZTUCONCT ZTUTDIR1" ;
	const string vlist2 = "ZTDTOP ZTDSELS ZTDVROWS CRP" ;

	vector<path> v ;

	regex expression ;

	vdefine( vlist1, &zcmd, &sel, &ztutnam1, &ztuconct, &ztutdir1 ) ;
	vdefine( vlist2, &ztdtop, &ztdsels, &ztdvrows, &crp ) ;

	try
	{
		expression.assign( conv_regex( ( ztutnam == "" ) ? "*" : ztutnam, '?', '*' ) ) ;
	}
	catch ( boost::regex_error& e )
	{
		vreplace( "ZZSTR", ztutnam ) ;
		msg = "PSYS012P" ;
		vdelete( vlist1, vlist2 ) ;
		return ;
	}

	if ( ztudir != "" )
	{
		fields  = "(SEL,ZTUTNAM1)",
		sortarg = "(ZTUTNAM1,C,A)" ;
		panel1  = "PPSP01T1" ;
		j       = 1 ;
		paths   = ztudir ;
	}
	else
	{
		fields  = "(SEL,ZTUTNAM1,ZTUCONCT,ZTUTDIR1)",
		sortarg = "(ZTUTDIR1,C,A,ZTUTNAM1,C,A)" ;
		panel1  = "PPSP01T2" ;
		vcopy( ztudd, paths, MOVE ) ;
		j = getpaths( paths ) ;
		if ( j == 0 )
		{
			msg = "PPSP012B" ;
			vdelete( vlist1, vlist2 ) ;
			return ;
		}
	}

	tbcreate( table,
		"",
		fields,
		NOWRITE,
		NOREPLACE ) ;

	for ( uint i = 1 ; i <= j ; ++i )
	{
		v.clear() ;
		ztutdir1 = getpath( paths, i ) ;
		try
		{
			copy( directory_iterator( ztutdir1 ), directory_iterator(), back_inserter( v ) ) ;
		}
		catch (...)
		{
		}
		interrupted = false ;
		for ( auto it = v.begin() ; it != v.end() ; ++it )
		{
			if ( interrupted )
			{
				msg = "PPSP011N" ;
				break ;
			}
			entry = it->string() ;
			if ( is_regular_file( entry ) )
			{
				ztutnam1 = get_filename( entry ) ;
				if ( !isvalidName( ztutnam1 ) ||
				     !regex_match( ztutnam1.begin(), ztutnam1.end(), expression ) )
				{
					continue ;
				}
				control( "ERRORS", "RETURN" ) ;
				tbopen( ztutnam1, NOWRITE, get_directory( entry ) ) ;
				exists = ( RC == 0 || RC == 12 ) ;
				if ( RC == 0 )
				{
					tbclose( ztutnam1 ) ;
				}
				control( "ERRORS", "CANCEL" ) ;
				if ( exists )
				{
					++rows ;
					ztuconct = d2ds( i-1 ) ;
					tbadd( table ) ;
				}
			}
		}
	}

	if ( rows == 0 )
	{
		msg = "PPSP012A" ;
		tbend( table ) ;
		vdelete( vlist1, vlist2 ) ;
		return ;
	}

	tbsort( table, sortarg ) ;
	tbtop( table ) ;

	panel2 = panel1 ;
	ztdtop = 1 ;
	msg    = "" ;
	cursor = "ZCMD" ;
	csrrow = 0  ;

	while ( true )
	{
		if ( msg == "" && ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			if ( ztdvrows > 0 )
			{
				tbtop( table ) ;
				tbskip( table, ztdtop ) ;
			}
			else
			{
				tbbottom( table ) ;
				tbskip( table, ( 2 - ztddepth ) ) ;
				if ( RC > 0 ) { tbtop( table ) ; }
			}
			panel2 = panel1 ;
		}
		tbdispl( table,
			 panel2,
			 msg,
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC == 8 ) { break ; }
		panel2 = "" ;
		msg    = "" ;
		cursor = ( sel == "" ) ? "" : "SEL" ;
		csrrow = crp ;
		if ( sel == "B" )
		{
			tableUtility_BrowseTable( msg,
						  ztutdir1,
						  ztutnam1 ) ;
		}
		else if ( sel == "I" )
		{
			control( "DISPLAY", "SAVE" ) ;
			tableUtility_TableInfo( msg,
						ztutdir1,
						ztutnam1 ) ;
			control( "DISPLAY", "RESTORE" ) ;
		}
		else if ( sel == "S" )
		{
			tableUtility_TableStats( msg,
						 ztutdir1,
						 ztutnam1 ) ;
		}
	}

	tbend( table ) ;
	vdelete( vlist1, vlist2 ) ;
}


void ppsp01a::tableUtility_BrowseTable( string& msg,
					const string& ztutdir1,
					const string& ztutnam )
{
	int sRC ;

	string ztushare ;
	string fields ;
	string names ;
	string keys ;
	string data ;
	string w1 ;

	size_t s ;

	map<string, size_t> sizes ;

	std::ofstream fout ;

	vcopy( "ZTUSHARE", ztushare, MOVE ) ;

	control( "ERRORS", "RETURN" ) ;

	tbopen( ztutnam,
		NOWRITE,
		ztutdir1,
		( ztushare == "/" ) ? SHARE : NON_SHARE ) ;
	sRC = RC ;

	control( "ERRORS", "CANCEL" ) ;
	if ( sRC == 8 )
	{
		msg = "PPSP012D" ;
		return ;
	}
	else if ( sRC == 12 )
	{
		addpop( "", 5, 5 ) ;
		display( "PPSP01T9" ) ;
		rempop() ;
		return ;
	}
	else if ( sRC > 0 )
	{
		msg = "PPSP012E" ;
		return ;
	}

	tbquery( ztutnam,
		 "KEYS",
		 "NAMES" ) ;
	vcopy( "KEYS", keys, MOVE ) ;
	vcopy( "NAMES", names, MOVE ) ;

	if ( keys != "" )
	{
		keys.erase( 0, 1 ) ;
		keys.pop_back() ;
	}

	if ( names != "" )
	{
		names.erase( 0, 1 ) ;
		names.pop_back() ;
	}

	fields = keys + " " + names ;

	string tname = get_tempname() ;

	tbtop( ztutnam ) ;
	tbskip( ztutnam ) ;
	while ( RC == 0 )
	{
		for ( uint i = 1 ; i <= words( fields ) ; ++i )
		{
			w1 = word( fields, i ) ;
			vcopy( w1, data, MOVE ) ;
			auto it = sizes.find( w1 ) ;
			if ( it == sizes.end() )
			{
				sizes[ w1 ] = max( data.size(), w1.size() ) ;
			}
			else
			{
				it->second = max( it->second, data.size() ) ;
			}
		}
		tbskip( ztutnam ) ;
	}

	fout.open( tname ) ;
	for ( uint i = 1 ; i <= words( fields ) ; ++i )
	{
		w1 = word( fields, i ) ;
		auto it = sizes.find( w1 ) ;
		if ( it == sizes.end() )
		{
			fout << setw( w1.size() + 3 ) << std::left << w1 ;
		}
		else
		{
			fout << setw( it->second + 3 ) << std::left << w1 ;
		}
	}
	fout << endl ;

	for ( uint i = 1 ; i <= words( fields ) ; ++i )
	{
		w1 = word( fields, i ) ;
		auto it = sizes.find( w1 ) ;
		if ( it == sizes.end() )
		{
			fout << setw( w1.size() + 3 ) << std::left << get_columnLine( w1.size() ) ;
		}
		else
		{
			fout << setw( it->second + 3 ) << std::left << get_columnLine( it->second ) ;
		}
	}
	fout << endl ;

	tbtop( ztutnam ) ;
	tbskip( ztutnam ) ;
	while ( RC == 0 )
	{
		for ( uint i = 1 ; i <= words( fields ) ; ++i )
		{
			w1 = word( fields, i ) ;
			s  = sizes[ w1 ] + 3 ;
			vcopy( w1, data, MOVE ) ;
			fout << setw( s ) << std::left << data ;
		}
		fout << endl ;
		tbskip( ztutnam ) ;
	}
	fout.close() ;

	tbend( ztutnam ) ;

	control( "ERRORS", "RETURN" ) ;
	vreplace( "ZBRALT", "Table " + ztutnam ) ;
	vput( "ZBRALT", SHARED ) ;
	browse( tname ) ;
	control( "ERRORS", "CANCEL" ) ;

	remove( tname ) ;
}


void ppsp01a::tableUtility_TableInfo( string& msg,
				      const string& ztutdir1,
				      const string& ztutnam )
{
	int sRC ;

	string ztushare ;
	string ztdmark ;
	string lname1 ;
	string lname2 ;
	string lname3 ;
	string lname4 ;
	string ltype ;
	string tbq1 ;
	string tbq2 ;
	string tbq3 ;
	string tbq4 ;
	string tbq5 ;
	string tbq6 ;
	string zcmd ;

	const string vlist1 = "TBQ1 TBQ2 TBQ3 TBQ4 TBQ5 TBQ6" ;
	const string vlist2 = "ZCMD ZTDMARK LTYPE LNAME1 LNAME2 LNAME3 LNAME4" ;

	vcopy( "ZTUSHARE", ztushare, MOVE ) ;

	control( "ERRORS", "RETURN" ) ;
	tbopen( ztutnam, NOWRITE, ztutdir1, ( ztushare == "/" ) ? SHARE : NON_SHARE ) ;
	sRC = RC ;
	control( "ERRORS", "CANCEL" ) ;
	if ( sRC == 8 )
	{
		msg = "PPSP012D" ;
		return ;
	}
	else if ( sRC == 12 )
	{
		addpop( "", 5, 5 ) ;
		display( "PPSP01T9" ) ;
		rempop() ;
		return ;
	}
	else if ( sRC > 0 )
	{
		msg = "PPSP012E" ;
		return ;
	}

	vdefine( vlist1, &tbq1, &tbq2, &tbq3, &tbq4, &tbq5, &tbq6 ) ;
	vdefine( vlist2, &zcmd, &ztdmark, &ltype, &lname1, &lname2, &lname3, &lname4 ) ;

	tbquery( ztutnam,
		 "TBQ1",
		 "TBQ2",
		 "TBQ3",
		 "TBQ4",
		 "TBQ5",
		 "TBQ6",
		 "TBQ7",
		 "TBQ8",
		 "TBQ9",
		 "TBQ10" ) ;

	string table = "LTV" + d2ds( taskid(), 5 ) ;

	tbcreate( table,
		"",
		"(LTYPE,LNAME1,LNAME2,LNAME3,LNAME4)",
		NOWRITE,
		NOREPLACE ) ;

	if ( tbq1 != "" )
	{
		tbq1.erase( 0, 1 ) ;
		tbq1.pop_back() ;
	}

	if ( tbq2 != "" )
	{
		tbq2.erase( 0, 1 ) ;
		tbq2.pop_back() ;
	}

	ltype = "KEYS:" ;
	for ( uint i = 1 ; i <= words( tbq1 ) ; ++i )
	{
		lname1 = word( tbq1, i ) ;
		lname2 = word( tbq1, ++i ) ;
		lname3 = word( tbq1, ++i ) ;
		lname4 = word( tbq1, ++i ) ;
		tbadd( table ) ;
		ltype  = "" ;
	}

	ltype = "NAMES:" ;
	for ( uint i = 1 ; i <= words( tbq2 ) ; ++i )
	{
		lname1 = word( tbq2, i ) ;
		lname2 = word( tbq2, ++i ) ;
		lname3 = word( tbq2, ++i ) ;
		lname4 = word( tbq2, ++i ) ;
		tbadd( table ) ;
		ltype  = "" ;
	}

	tbtop( table ) ;

	addpop( "", 5, 5 ) ;
	while ( RC == 0 )
	{
		tbdispl( table, "PPSP01T8" ) ;
	}
	rempop() ;

	tbend( ztutnam ) ;
	tbend( table ) ;

	vdelete( vlist1, vlist2 ) ;
}


void ppsp01a::tableUtility_TableStats( string& msg,
				       const string& ztutdir1,
				       const string& ztutnam )
{
	string cdate ;
	string ctime ;
	string udate ;
	string utime ;
	string user  ;
	string rowcreat ;
	string rowcurr  ;
	string rowupd   ;
	string tableupd ;
	string service  ;
	string retcode  ;
	string status1  ;
	string status2  ;
	string status3  ;
	string virtsize ;
	string cdate4d  ;
	string udate4d  ;

	string zcmd ;

	string lib = ztutdir1 ;
	string tab = ztutnam ;

	const string vlist1 = "CDATE CTIME UDATE UTIME USER ROWCREAT ROWCURR ROWUPD TABLEUPD" ;
	const string vlist2 = "SERVICE RETCODE STATUS1 STATUS2 STATUS3 VIRTSIZE CDATE4D UDATE4D" ;
	const string vlist3 = "ZTAB ZLIB ZCMD" ;

	vdefine( vlist1, &cdate, &ctime, &udate, &utime, &user, &rowcreat, &rowcurr, &rowupd, &tableupd ) ;
	vdefine( vlist2, &service, &retcode, &status1, &status2, &status3, &virtsize, &cdate4d, &udate4d ) ;
	vdefine( vlist3, &tab, &lib, &zcmd ) ;

	libdef( "MYLIB", "PATH", ztutdir1 ) ;

	addpop( "", 3, 5 ) ;
	while ( RC == 0 )
	{
		tbstats( tab,
			 "CDATE",
			 "CTIME",
			 "UDATE",
			 "UTIME",
			 "USER",
			 "ROWCREAT",
			 "ROWCURR",
			 "ROWUPD",
			 "TABLEUPD",
			 "SERVICE",
			 "RETCODE",
			 "STATUS1",
			 "STATUS2",
			 "STATUS3",
			 "MYLIB",
			 "VIRTSIZE",
			 "CDATE4D",
			 "UDATE4D" ) ;
		display( "PPSP01T3" ) ;
	}
	rempop() ;

	libdef( "MYLIB" ) ;

	vdelete( vlist1, vlist2, vlist3 ) ;
}


/**************************************************************************************************************/
/**********************************               PF SHOW UTILITY                  ****************************/
/**************************************************************************************************************/

void ppsp01a::pfshowUtility( string& parms )
{
	string zfka ;
	string zpfshow ;

	const string vlist1 = "ZFKA ZPFSHOW" ;

	vdefine( vlist1, &zfka, &zpfshow ) ;

	if ( parms == "" )
	{
		vget( "ZPFSHOW", PROFILE ) ;
		parms = ( zpfshow == "ON" ) ? "OFF" : "ON" ;
	}

	if ( parms == "ON" )
	{
		zfka    = "LONG" ;
		zpfshow = "ON" ;
		vput( "ZFKA ZPFSHOW", PROFILE ) ;
	}
	else if ( parms == "OFF" )
	{
		zfka    = "OFF" ;
		zpfshow = "OFF" ;
		vput( "ZFKA ZPFSHOW", PROFILE ) ;
	}
	else if ( parms == "TAILOR" )
	{
		addpop( "", 5, 5 ) ;
		while ( true )
		{
			display( "PPSP01P1" ) ;
			if ( RC > 0 ) { break ; }
		}
		rempop() ;
	}
	else
	{
		setmsg( "PPSP013A" ) ;
	}

	vdelete( vlist1 ) ;
}


/**************************************************************************************************************/
/**********************************                 FKA UTILITY                    ****************************/
/**************************************************************************************************************/

void ppsp01a::fkaUtility( string& parm )
{
	string zfka ;
	string zpfshow ;

	const string vlist1 = "ZFKA ZPFSHOW" ;

	vdefine( vlist1, &zfka, &zpfshow ) ;

	if ( parm == "" )
	{
		vget( "ZFKA", PROFILE ) ;
		parm = ( zfka == "OFF" )  ? "ON"    :
		       ( zfka == "LONG" ) ? "SHORT" : "OFF" ;
	}

	if ( parm == "ON" )
	{
		zfka    = "LONG" ;
		zpfshow = "ON" ;
		vput( "ZFKA ZPFSHOW", PROFILE ) ;
	}
	else if ( parm == "OFF" )
	{
		zfka    = "OFF" ;
		zpfshow = "OFF" ;
		vput( "ZFKA ZPFSHOW", PROFILE ) ;
	}
	else if ( parm == "SHORT" )
	{
		zfka    = "SHORT" ;
		zpfshow = "ON" ;
		vput( "ZFKA ZPFSHOW", PROFILE ) ;
	}
	else
	{
		setmsg( "PPSP013B" ) ;
	}

	vdelete( vlist1 ) ;
}



/**************************************************************************************************************/
/**********************************                 EDIT STRING                    ****************************/
/**************************************************************************************************************/

void ppsp01a::editString( const string& s )
{
	//
	// Edit/browse string 's'.  Pass back edited string in ZRESULT.
	//
	// ZRC = 0 Okay.
	// ZRC = 4 Cancel update.
	//
	// ZRC=0, ZRSN=4 from the Editor indicates a SAVE followed by a CANCEL.
	//

	int zareaw ;

	string tname = get_tempname() ;
	string field = word( s, 1 ) ;
	string flen  = s.substr( 13, 5 ) ;

	char ftype = s[ 18 ] ;

	bool empty = false ;

	size_t i = ds2d( flen ) ;

	string inLine ;

	size_t p ;

	vdefine( "ZAREAW", &zareaw ) ;

	std::ofstream fout ;
	std::ifstream fin ;

	fout.open( tname ) ;
	if ( s.size() > 19 )
	{
		fout << s.substr( 19 ) ;
	}
	else
	{
		fout << "" ;
		empty = true ;
	}
	fout.close() ;

	p = field.find( '.' ) ;

	set_dialogue_var( "ZFLEN", flen ) ;

	if ( ftype == 'O' )
	{
		if ( !empty )
		{
			pquery( "PPSP01B0", "ZAREA", "", "ZAREAW" ) ;
			set_dialogue_var( "ZBRALT", "FIELD: " + ( ( p == string::npos ) ? field : field.substr( 0, p ) ) ) ;
			browse( tname, "PPSP01B0", "", zareaw-1 ) ;
			verase( "ZEDALT ZFLEN", SHARED ) ;
		}
		remove( tname ) ;
		ZRC = 4 ;
		return ;
	}

	set_dialogue_var( "ZEDALT", "FIELD: " + ( ( p == string::npos ) ? field : field.substr( 0, p ) ) ) ;

	edit( tname, "PPSP01E0", "RECOVERY OFF", "TEXT" ) ;

	verase( "ZEDALT ZFLEN", SHARED ) ;

	ZRESULT = "" ;
	if ( ZRC == 0 && ZRSN == 0 )
	{
		fin.open( tname ) ;
		while ( getline( fin, inLine ) && ZRESULT.size() <= i )
		{
			ZRESULT += inLine ;
		}
		fin.close() ;
		if ( ZRESULT.size() > i ) { ZRESULT.erase( i ) ; }
		ZRESULT = left( field, 13 ) + ZRESULT ;
	}
	else if ( ZRC == 0 && ZRSN == 4 )
	{
		ZRC = 4 ;
	}

	remove( tname ) ;
}



/**************************************************************************************************************/
/***********************************                    END                       *****************************/
/**************************************************************************************************************/


void ppsp01a::update_reflist( const string& file )
{
	if ( get_dialogue_var( "ZRFURL" ) == "YES" )
	{
		submit( "PGM("+ get_dialogue_var( "ZRFLPGM" ) +") PARM(PLR "+ file +")" ) ;
	}
}


string ppsp01a::get_dialogue_var( const string& v )
{
	string t ;

	vcopy( v, t ) ;

	return t ;
}


void ppsp01a::set_dialogue_var( const string& var,
				const string& val )
{
	vreplace( var, val ) ;
	vput( var, SHARED ) ;
}


string ppsp01a::get_tempname()
{
	boost::filesystem::path temp = boost::filesystem::temp_directory_path() /
	       boost::filesystem::unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;

	return temp.native() ;
}


string ppsp01a::get_filename( const string& f )
{
	return f.substr( f.find_last_of( '/' ) + 1 ) ;
}


string ppsp01a::get_directory( const string& f )
{
	return f.substr( 0, f.find_last_of( '/' ) ) ;
}


string ppsp01a::get_columnLine( size_t w )
{
	int j = 0 ;

	const string t1 = "----+----" ;
	string t2 = t1 ;

	while ( t2.size() < w )
	{
		t2 += d2ds( ++j%10 ) + t1 ;
	}

	return t2.substr( 0, w ) ;
}


bool ppsp01a::validTable( const string& filename )
{
	//
	// Return true if the file contains a valid table.
	//

	char buf[ 2 ] ;

	std::ifstream fin ;

	fin.open( filename.c_str(), ios::binary | ios::in ) ;
	if ( !fin.is_open() )
	{
		return false ;
	}

	fin.read( buf, 2);
	if ( fin.gcount() != 2 || memcmp( buf, "\x00\x85", 2 ) )
	{
		fin.close() ;
		return false ;
	}

	fin.close() ;
	return true ;
}


void ppsp01a::execute_cmd( int& rc,
			   const string& cmd,
			   vector<string>& results )
{
	//
	// Execute a command and place the output in the
	// results vector.
	//
	// Redirect STDERR/STOUT to pipes to get errors from the command.  Restore on return.
	//

	string line ;

	int fd1 ;
	int fd2 ;

	int my_pipe1[ 2 ] ;
	int my_pipe2[ 2 ] ;

	char buffer[ 8192 ] ;

	results.clear() ;

	if ( pipe2( my_pipe1, O_NONBLOCK ) == -1 )
	{
		vreplace( "PSUERR", strerror( errno ) ) ;
		setmsg( "PSUT015C" ) ;
		return ;
	}

	if ( pipe2( my_pipe2, O_NONBLOCK ) == -1 )
	{
		vreplace( "PSUERR", strerror( errno ) ) ;
		close( my_pipe1[ 0 ] ) ;
		close( my_pipe1[ 1 ] ) ;
		setmsg( "PSUT015C" ) ;
		return ;
	}

	fd1 = dup( STDERR_FILENO ) ;
	dup2( my_pipe1[ 1 ], STDERR_FILENO ) ;

	fd2 = dup( STDOUT_FILENO ) ;
	dup2( my_pipe2[ 1 ], STDOUT_FILENO ) ;

	FILE* pipe { popen( cmd.c_str(), "r" ) } ;

	if ( !pipe )
	{
		rc = 3 ;
		setmsg( "PSUT011E" ) ;
		llog( "E", "POPEN failed.  Command string size="<< cmd.size() <<endl ) ;
		return ;
	}

	while ( fgets( buffer, sizeof( buffer ), pipe ) )
	{
		line = buffer ;
		if ( line != "" && line.back() == 0x0a )
		{
			line.pop_back() ;
		}
		results.push_back( line ) ;
	}

	rc = WEXITSTATUS( pclose( pipe ) ) ;

	fflush( stdout ) ;
	close( my_pipe2[ 0 ] ) ;
	close( my_pipe2[ 1 ] ) ;
	dup2( fd2, STDOUT_FILENO ) ;

	fflush( stderr ) ;
	close( my_pipe1[ 0 ] ) ;
	close( my_pipe1[ 1 ] ) ;
	dup2( fd1, STDERR_FILENO ) ;
}

