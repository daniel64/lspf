/* Compile with ::                                                                  */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libpcmd0b.so -o libpcmd0b.so pcmd0b.cpp */

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
/* OMVS-like shell.                                                         */
/* Uses PCMD0A to run the commands entered.                                 */
/*                                                                          */
/* Builtin commands:                                                        */
/* -ANSI   toggle setting colour using ANSI colour code sequences.          */
/* -CLEAR  clear output and labels                                          */
/* -HELP   print help.                                                      */
/* -INFO   list application information.                                    */
/* -LIST   list running jobs                                                */
/* -LABELS display defined labels                                           */
/* -HIST   display path history                                             */
/*                                                                          */
/* .string define a label to position output                                */
/*                                                                          */
/* P  go back to the previous path                                          */
/* L .string position screen to label .string                               */
/*                                                                          */
/****************************************************************************/

#include <iostream>
#include <boost/filesystem.hpp>

#include "../lspfall.h"
#include "pcmd0b.h"

using namespace std ;
using namespace boost::filesystem ;

LSPF_APP_MAKER( pcmd0b )


pcmd0b::pcmd0b()
{
	STANDARD_HEADER( "Invoke a command and display the output", "1.0.2" )

	ansi_on   = true ;
	ansitxt   = "Disable" ;
	buf_limit = 100000 ;
	ppos      = 0 ;

	path_history.rset_capacity( 99 ) ;
}


void pcmd0b::application()
{
	bool exit = false ;

	string w1 ;
	string w2 ;
	string comm1 ;
	string comm2 ;

	vector<pair<string,string>> tnames ;

	const string vlist1 = "ZCMD ZCMD1 ZVERB ZNODNAME ZHOME COMM1 COMM2" ;
	const string vlist2 = "ZAREA ZSHADOW ZSCROLLA ANSITXT" ;
	const string vlist3 = "ZSCROLLN ZAREAW ZAREAD ZSCREEND ZSCREENW" ;

	vdefine( vlist1, &zcmd, &zcmd1, &zverb, &znode, &zhome, &comm1, &comm2 ) ;
	vdefine( vlist2, &zarea, &zshadow, &zscrolla, &ansitxt ) ;
	vdefine( vlist3, &zscrolln, &zareaw, &zaread, &zscreend, &zscreenw ) ;

	vget( "COMM1 COMM2 ZNODNAME ZHOME ZSCREEND ZSCREENW", SHARED ) ;

	vcopy( "ZUSER", zuser, MOVE )     ;
	vcopy( "ZSCREEN", zscreen, MOVE ) ;

	pquery( "PCMD0B", "ZAREA", "", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 )
	{
		uabend() ;
	}

	zasize       = zareaw * zaread ;
	msg          = "" ;
	topLine      = 0  ;
	startCol     = 1  ;
	rebuildZAREA = true ;

	sdr.assign( zareaw, N_RED )    ;
	sdw.assign( zareaw, N_WHITE )  ;
	sdy.assign( zareaw, N_YELLOW ) ;
	sdg.assign( zareaw, N_GREEN )  ;
	sdt.assign( zareaw, N_TURQ  )  ;

	boost::filesystem::path p = boost::filesystem::current_path() ;
	wd = p.native() ;
	path_history.push_front( wd ) ;

	lines.push_back( command_prompt() ) ;

	while ( true )
	{
		if ( rebuildZAREA ) { fill_dynamic_area( tnames.size() > 0 ) ; }
		display( "PCMD0B", msg ) ;
		if ( RC == 8 )
		{
			if ( exit || tnames.size() == 0 ) { break ; }
			vreplace( "ZEDSMSG", "Background tasks still running" ) ;
			vreplace( "ZEDLMSG", "Press PF3 again to exit or ENTER to display output" ) ;
			msg  = "PSYZ001" ;
			exit = true ;
			continue ;
		}
		exit = false ;
		msg  = ""    ;
		if ( is_term_resized( zscreend+2, zscreenw ) )
		{
			term_resize() ;
			rebuildZAREA = true ;
			continue ;
		}


		vget( "ZVERB ZSCROLLA ZSCROLLN", SHARED ) ;
		if ( zcmd != "" )
		{
			w1 = word( zcmd, 1 ) ;
			if ( zcmd.front() == '-' )
			{
				auto it = icmdList.find( upper( word( zcmd, 1 ) ) ) ;
				if ( it != icmdList.end() )
				{
					(this->*(it->second))() ;
				}
				else
				{
					msg = "PSYS018" ;
				}
			}
			else if ( w1.front() == '.' && w1.size() > 1 )
			{
				iupper( w1 ) ;
				auto it = labels.find( w1 ) ;
				labels[ w1 ] = topLine ;
				if ( it == labels.end() )
				{
					vreplace( "ZEDSMSG", "Label '" + w1 + "' assigned" ) ;
				}
				else
				{
					vreplace( "ZEDSMSG", "Label '" + w1 + "' reassigned" ) ;
				}
				vreplace( "ZEDLMSG", "" ) ;
				msg = "PSYZ000" ;
				rebuildZAREA = true ;
				zcmd = "" ;
			}
			else if ( upper( w1 ) == "L" && words( zcmd ) > 1 )
			{
				w2      = upper( word( zcmd, 2 ) ) ;
				auto it = labels.find( w2 ) ;
				if ( it != labels.end() )
				{
					topLine = it->second ;
				}
				else
				{
					vreplace( "ZEDSMSG", "Label not assigned" ) ;
					vreplace( "ZEDLMSG", "Entered label has not been assigned to a line." ) ;
					msg = "PSYZ001" ;
				}
				rebuildZAREA = true ;
				zcmd = "" ;
			}
			else if ( upper( w1 ) == "P" )
			{
				action_prev() ;
			}
			else if ( upper( zcmd ) == "S" || ( w1 == "cd" && word( zcmd, 2 ) == ".." && words( zcmd ) == 2 ) )
			{
				remove_wd_lld() ;
				add_path_history() ;
			}
			else if ( w1 == "cd"  )
			{
				chng_directory() ;
			}
			else
			{
				comm1 = get_tempname( "output" ) ;
				comm2 = get_tempname( "errors" ) ;
				tnames.push_back( make_pair( comm1, comm2 ) ) ;
				invoke_task( zcmd, comm1, comm2 ) ;
				cmds[ comm1 ] = make_pair( ds2d( zsbtask ), zcmd1 ) ;
				rebuildZAREA = true ;
				zcmd = "" ;
			}
		}
		else if ( zverb != "" )
		{
			auto it = scrollList.find( zverb ) ;
			if ( it != scrollList.end() )
			{
				(this->*(it->second))() ;
				if ( topLine  < 0 ) { topLine  = 0 ; }
				if ( startCol < 1 ) { startCol = 1 ; }
				rebuildZAREA = true ;
			}
		}
		else
		{
			ppos = 0 ;
			lines.push_back( command_prompt() ) ;
			if ( ( lines.size() - topLine ) > zaread )
			{
				bottom_of_data() ;
			}
			rebuildZAREA = true ;
		}
		for ( auto it = tnames.begin() ; it != tnames.end() ; )
		{
			qscan( "CMD", it->first ) ;
			if ( RC == 8 )
			{
				copy_output( cmds[ it->first ].second, it->first, it->second ) ;
				cmds.erase( it->first ) ;
				it = tnames.erase( it ) ;
				rebuildZAREA = true ;
				continue ;
			}
			++it ;
		}
	}
}


void pcmd0b::copy_output( const string& cmd,
			  const string& fname,
			  const string& ename )
{
	size_t ln = 0 ;

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
		if ( lines.size() > ( buf_limit + 1000 ) )
		{
			lines.erase( lines.begin(), lines.begin() + 1000 ) ;
		}
		++ln ;
	}
	fin.close() ;

	lines.push_back( "" ) ;

	if ( lines.size() > buf_limit )
	{
		lines.erase( lines.begin(), lines.begin() + ( lines.size() - buf_limit ) ) ;
	}

	if ( ln > buf_limit )
	{
		lines.push_back( "" ) ;
		lines.push_back( "e> Warning:  Output truncated." ) ;
		lines.push_back( "e>           Lines produced was "+ d2ds( ln ) +"." ) ;
		lines.push_back( "e>           Buffer limit is currently "+ d2ds( buf_limit ) +"." ) ;
		lines.push_back( "" ) ;
	}

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
		add_path_history() ;
	}

	lines.push_back( command_prompt() ) ;
	if ( ( lines.size() - topLine ) > zaread )
	{
		bottom_of_data() ;
	}
}


void pcmd0b::action_prev()
{
	if ( !path_history.empty() )
	{
		if ( ++ppos >= path_history.size() ) { ppos = 0 ; }
		if ( wd == path_history[ ppos ] )
		{
			if ( ++ppos >= path_history.size() ) { ppos = 0 ; }
		}
		wd = path_history[ ppos ] ;
	}

	lines.push_back( command_prompt() ) ;
	bottom_of_data() ;
	zcmd         = "" ;
	rebuildZAREA = true ;
}


void pcmd0b::action_ANSI()
{
	ansi_on = !ansi_on ;
	ansitxt = ( ansi_on ) ? "Disable" : "Enable" ;
	zcmd    = "" ;
	rebuildZAREA = true ;
}


void pcmd0b::action_CLEAR()
{
	lines.clear() ;
	labels.clear() ;
	topLine      = 0  ;
	startCol     = 1  ;
	zcmd         = "" ;
	lines.push_back( command_prompt() ) ;
	rebuildZAREA = true ;
}


void pcmd0b::action_LABELS()
{
	lines.push_back( "" ) ;
	lines.push_back( "b> Defined labels:" ) ;
	lines.push_back( "" ) ;

	for ( auto it = labels.begin() ; it != labels.end() ; ++it )
	{
		lines.push_back( it->first + " line " + d2ds( it->second ) ) ;
	}

	zcmd         = "" ;
	rebuildZAREA = true ;
	bottom_of_data() ;
}


void pcmd0b::action_LIST()
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
		map<int,string> temp ;
		for ( auto it = cmds.begin() ; it != cmds.end() ; ++it )
		{
			temp[ it->second.first ] = it->second.second ;
		}
		for ( auto it = temp.begin() ; it != temp.end() ; ++it )
		{
			lines.push_back( d2ds( it->first, 5 ) + "  " + it->second ) ;
		}
	}
	lines.push_back( "" ) ;
	lines.push_back( command_prompt() ) ;
	zcmd         = "" ;
	rebuildZAREA = true ;
	bottom_of_data() ;
}


void pcmd0b::action_INFO()
{
	string tmp ;

	lines.push_back( "" ) ;
	lines.push_back( "b> Stored lines "+ ansi_red + d2ds( lines.size() ) ) ;
	lines.push_back( "b> Current limit "+ ansi_red + d2ds( buf_limit ) ) ;
	vcopy( "GRCSTR", tmp ) ;
	lines.push_back( "b> Colouring program string "+ ansi_red + tmp ) ;
	vcopy( "GRCCHR", tmp ) ;
	lines.push_back( "b> Colouring program command prefix "+ ansi_red + tmp ) ;
	lines.push_back( "" ) ;
	zcmd         = "" ;
	rebuildZAREA = true ;
	bottom_of_data() ;
}


void pcmd0b::action_HELP()
{
	string tmp ;

	lines.push_back( "" ) ;
	lines.push_back( "b> Commands:" ) ;
	lines.push_back( "b>   -ANSI         " + ansi_white + " Toggle display of ANSI colour codes." ) ;
	lines.push_back( "b>   -CLEAR        " + ansi_white + " Clear output buffer." ) ;
	lines.push_back( "b>   -LIST         " + ansi_white + " List running jobs." ) ;
	lines.push_back( "b>   -INFO         " + ansi_white + " Show current options and usage." ) ;
	lines.push_back( "b>   -HELP         " + ansi_white + " Display simple help." ) ;
	lines.push_back( "b>   -HIST         " + ansi_white + " Display path history." ) ;
	lines.push_back( "b>" ) ;
	vcopy( "GRCCHR", tmp ) ;
	lines.push_back( "b>   Prefix command with the '"+ tmp +"' symbol to use grc to colour output." ) ;
	lines.push_back( "" ) ;
	zcmd         = "" ;
	rebuildZAREA = true ;
	bottom_of_data() ;
}


void pcmd0b::action_HISTORY()
{
	lines.push_back( "" ) ;
	lines.push_back( "b> Path history:" ) ;

	for ( const auto& ln : path_history )
	{
		lines.push_back( ln ) ;
	}

	lines.push_back( "" ) ;
	zcmd         = "" ;
	rebuildZAREA = true ;
	bottom_of_data() ;
}


void pcmd0b::fill_dynamic_area( bool running )
{
	int i ;
	int j ;

	size_t dl ;
	size_t ln ;

	size_t p1 ;
	size_t p2 ;
	size_t p3 ;
	size_t p4 ;

	size_t d1 ;
	size_t d2 ;

	int c = N_WHITE ;

	bool bold = false ;
	bool rev  = false ;

	string hostUser = zuser + "@" + znode ;

	string temp1 ;
	string temp2 ;
	string temp3 ;
	string t ;

	const string ansi_start = "\x1b[" ;
	const char   ansi_end   = 'm' ;
	const char   ansi_delm  = ';' ;
	const string ansi_codes = "0123456789; " ;

	map<int, int>::iterator it ;

	zarea   = "" ;
	zshadow = "" ;
	maxCol  = 1  ;

	if ( size_t( topLine ) >= lines.size() )
	{
		topLine = ( lines.size() > 0 ) ? lines.size() - 1 : 0 ;
	}

	for ( dl = topLine, i = 0 ; i < zaread && dl < lines.size() ; ++i, ++dl )
	{
		string& pstr = lines[ dl ] ;
		if ( ansi_on )
		{
			temp1 = pstr ;
		}
		else
		{
			zarea += substr( pstr, startCol, zareaw ) ;
			maxCol = max( maxCol, uint( pstr.size() ) ) ;
		}
		if      ( pstr.compare( 0, 2, ":>" ) == 0 ) { zshadow += sdw ; }
		else if ( pstr.compare( 0, 2, "e>" ) == 0 ) { zshadow += sdr ; }
		else if ( pstr.compare( 0, 2, "l>" ) == 0 ) { zshadow += sdg ; }
		else if ( pstr.compare( 0, 2, "b>" ) == 0 ) { zshadow += sdg ; }
		else if ( pstr.compare( 0, hostUser.size(), hostUser ) == 0 )
		{
			p1 = hostUser.find( '@' ) ;
			zshadow += string( p1, N_GREEN ) ;
			zshadow.push_back( N_WHITE ) ;
			zshadow += string( hostUser.size()-p1-1, N_TURQ ) ;
			zshadow += string( zareaw-hostUser.size(), N_RED ) ;
		}
		else if ( pstr.compare( 0, 2, ": " ) == 0 ) { zshadow += sdr ; }
		else                                        { zshadow += sdy ; }
		if ( ansi_on )
		{
			d1    = 0 ;
			ln    = temp1.size() ;
			temp2 = zshadow.substr( zshadow.size()-zareaw ) ;
			temp2.resize( temp1.size(), temp2.front() ) ;
			p1    = temp1.find( ansi_start ) ;
			while ( p1 != string::npos )
			{
				p2 = temp1.find_first_not_of( ansi_codes, p1+2 ) ;
				if ( p2 == string::npos ) { break ; }
				if ( temp1[ p2 ] != ansi_end )
				{
					p1 = temp1.find( ansi_start, p2 ) ;
					continue ;
				}
				temp3 = temp1.substr( p1+2, p2-p1-2 ) ;
				d2    = p2-p1+1 ;
				p3    = temp3.find( ansi_delm ) ;
				while ( temp3 != "" )
				{
					if ( p3 == string::npos ) { p3 = temp3.size() ; }
					t = temp3.substr( 0, p3 ) ;
					if ( datatype( t, 'W' ) )
					{
						j = ds2d( t ) ;
						if ( j == 0 )
						{
							bold = false ;
							rev  = false ;
							c    = N_WHITE ;
						}
						else if ( j == 1 )
						{
							bold = true  ;
							rev  = false ;
						}
						else
						{
							if ( bold && j > 30 && j < 38 ) { j += 60 ; }
							if ( rev  && j > 30 && j < 38 ) { j += 10 ; }
							it = kw_ansi.find( j ) ;
							if ( it != kw_ansi.end() )
							{
								c = it->second ;
							}
						}
					}
					if ( p3 == temp3.size() )
					{
						temp3 = "" ;
						p4 = p1 - d1 ;
						temp2.replace( p4, ln-p4, ln-p4, c ) ;
					}
					else
					{
						temp3.erase( 0, p3+1 ) ;
						p3 = temp3.find( ansi_delm ) ;
					}
				}
				d1 += d2 ;
				p1  = temp1.find( ansi_start, p2+1 ) ;
			}
			p1 = temp1.find( ansi_start ) ;
			while ( p1 != string::npos )
			{
				p2 = temp1.find_first_not_of( ansi_codes, p1+2 ) ;
				if ( p2 == string::npos ) { break ; }
				if ( temp1[ p2 ] != ansi_end )
				{
					p1 = temp1.find( ansi_start, p2 ) ;
					continue ;
				}
				temp1.erase( p1, p2-p1+1 ) ;
				p1 = temp1.find( ansi_start, p1 ) ;
			}
			zarea += substr( temp1, startCol, zareaw ) ;
			temp2  = substr( temp2, startCol, zareaw ) ;
			zshadow.replace( zshadow.size()-zareaw, zareaw, temp2 ) ;
			maxCol = max( maxCol, uint( temp1.size() ) ) ;
		}
	}

	zarea.resize( zasize, ' ' ) ;
	zshadow.resize( zasize, N_YELLOW ) ;
	if ( running )
	{
		zarea.replace( zasize-8, 7, "RUNNING " ) ;
		zshadow.replace( zasize-8, 7, 7, N_WHITE ) ;
	}
	else if ( size_t( topLine + zaread ) < lines.size() )
	{
		zarea.replace( zasize-8, 7, "MORE... " ) ;
		zshadow.replace( zasize-8, 7, 7, N_WHITE ) ;
	}
	rebuildZAREA = false ;
}


void pcmd0b::term_resize()
{
	//
	// Get/set new size of variables after a terminal resize.
	//

	const string vlist1 = "ZSCREEND ZSCREENW" ;

	vget( vlist1, SHARED ) ;

	pquery( "PCMD0B", "ZAREA", "", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 )
	{
		uabend() ;
	}

	zasize = zareaw * zaread ;

	sdr.assign( zareaw, N_RED )    ;
	sdw.assign( zareaw, N_WHITE )  ;
	sdy.assign( zareaw, N_YELLOW ) ;
	sdg.assign( zareaw, N_GREEN )  ;
	sdt.assign( zareaw, N_TURQ  )  ;
}


void pcmd0b::action_ScrollUp()
{
	if ( zscrolla == "MAX" )
	{
		topLine = 0 ;
	}
	else
	{
		topLine -= zscrolln ;
	}
}


void pcmd0b::action_ScrollDown()
{
	if ( zscrolla == "MAX" )
	{
		topLine = lines.size() - zaread ;
	}
	else
	{
		topLine += zscrolln ;
	}
}


void pcmd0b::action_ScrollLeft()
{
	if ( zscrolla == "MAX" )
	{
		startCol = 1 ;
	}
	else
	{
		startCol = startCol - zscrolln ;
	}
}


void pcmd0b::action_ScrollRight()
{
	if ( zscrolla == "MAX" )
	{
		startCol = maxCol - zareaw - 1 ;
	}
	else
	{
		startCol += zscrolln ;
	}
}


bool pcmd0b::invoke_task( string cmd,
			  const string& comm1,
			  const string& comm2 )
{
	//
	// Invoke a background task and wait for a reponse (using PCMD0A application).
	// Wait for up to 0.5 second for the command to end, otherwise return.
	//
	// Return:  false - no reponse received within period.  Job running in the background.
	//          true  - reponse received.
	//

	int elapsed ;

	bool is_rexx  = false ;
	bool is_shell = false ;

	vput( "COMM1 COMM2", SHARED ) ;

	if ( cmd.front() == '%' )
	{
		cmd.erase( 0, 1 ) ;
		is_rexx = true ;
	}
	else if ( cmd.front() == '!' )
	{
		cmd.erase( 0, 1 ) ;
		is_shell = true ;
	}

	if ( !is_shell && ( is_rexx || isRexx( cmd ) ) )
	{
		cmd = "%" + cmd ;
	}
	else
	{
		if ( cmd == "cd" ) { cmd = "cd " + zhome ; wd = zhome ; }
		else               { cmd = "cd " + wd + " && " + cmd  ; }
	}
	submit( "PGM(PCMD0A) PARM(" + cmd + ")" ) ;

	vcopy( "ZSBTASK", zsbtask, MOVE ) ;

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


bool pcmd0b::isRexx( string orexx )
{
	locator loc( sysexec(), orexx ) ;
	loc.locate() ;

	return loc.found() ;
}


void pcmd0b::bottom_of_data()
{
	topLine  = lines.size() - zaread ;
	startCol = 1                     ;
	if ( topLine < 0 ) { topLine = 0 ; }
}


string pcmd0b::command_prompt()
{
	return zuser + "@" + znode + " " + wd + ">" ;
}


void pcmd0b::chng_directory()
{
	string p = subword( zcmd, 2 ) ;
	boost::filesystem::path pp1 = boost::filesystem::current_path() ;

	if ( p == "" ) { p = zhome ; }
	if ( p.front() != '/' ) { p = full_name( wd, p ) ; }

	if ( ERR == chdir( p.c_str() ) )
	{
		lines.push_back( "" ) ;
		lines.push_back( ":> cd " + p ) ;
		lines.push_back( "e> " + string( strerror( errno ) ) ) ;
		lines.push_back( "" ) ;
	}
	else
	{
		boost::filesystem::path pp2 = boost::filesystem::current_path() ;
		wd = pp2.native() ;
		add_path_history() ;
		chdir( pp1.native().c_str() ) ;
	}

	lines.push_back( command_prompt() ) ;
	bottom_of_data() ;
	zcmd         = "" ;
	rebuildZAREA = true ;
}


void pcmd0b::add_path_history()
{
	size_t p ;

	p = wd.find( "//" ) ;
	while ( p != string::npos )
	{
		wd.erase( p, 1 ) ;
		p = wd.find( "//", p ) ;
	}

	if ( wd.size() > 1 && wd.back() == '/' )
	{
		wd.pop_back() ;
	}

	if ( !path_history.empty() )
	{
		auto it = find( path_history.begin(), path_history.end(), wd ) ;
		if ( it != path_history.end() )
		{
			path_history.erase( it ) ;
		}
	}
	path_history.push_front( wd ) ;
	ppos = 0 ;
}


void pcmd0b::remove_wd_lld()
{
	do
	{
		wd.pop_back() ;
	}
	while ( !wd.empty() && wd.back() != '/' ) ;

	if ( !wd.empty() ) { wd.pop_back() ; }

	if ( wd.empty() ) { wd = "/" ; }

	lines.push_back( command_prompt() ) ;
	bottom_of_data() ;
	zcmd         = "" ;
	rebuildZAREA = true ;
}


string pcmd0b::full_name( const string& a,
			  const string& b )
{
	return ( a.back() == '/' ) ? a + b : a + "/" + b ;
}


string pcmd0b::get_tempname( const string& suf )
{
	boost::filesystem::path temp = boost::filesystem::temp_directory_path() /
				       boost::filesystem::unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;

	return temp.native() + "." + suf ;
}
