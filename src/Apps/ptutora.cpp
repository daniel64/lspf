/*  Compile with ::                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libptutora.so -o libptutora.so ptutora.cpp */

/*
  Copyright (c) 2021 Daniel John Erdos

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

/*******************************************************************************************************/
/*                                                                                                     */
/* Tutorial/HELP program.                                                                              */
/*                                                                                                     */
/* This program may be started with the SELECT NOFUNC parameter, so no function pool calls should be   */
/* coded in the constructor and all vdefines *must* have an associated vdelete before program          */
/* termination or the application associated with the function pool will most likely fail.             */
/*                                                                                                     */
/* Avoid using implicit variables so as not to pollute the function pool.                              */
/*                                                                                                     */
/*******************************************************************************************************/


#include "../lspfall.h"
#include "ptutora.h"

using namespace std ;
using namespace boost::filesystem ;

LSPF_APP_MAKER( ptutora )


ptutora::ptutora()
{
	STANDARD_HEADER( "Default help/tutorial program for lspf", "2.0.0" )
}


void ptutora::application()
{
	//
	// Display help panel.  If no help panel passed, start the tutorial.
	//

	string help = upper( word( PARM, 1 ) ) ;

	addpop( "", 2, 5 ) ;

	panel_tutor( ( help == "" ) ? "LSPH0000" : help ) ;

	rempop() ;
}


int ptutora::panel_tutor( string hpanel,
			  int level )
{
	//
	// Return code:  0  - Continue help panel processing as normal.
	//               4  - Retrieve panel from the history.
	//               8  - Show previous menu.
	//               12 - Go to next topic (displaying menu if selection was made).
	//               16 - Skip to next topic.
	//               20 - Go to level 0 and display the ZHTOP panel (TOC command entered).
	//               24 - Exit.
	//

	//
	// Bugs:  History does not work well as it only keeps the panels that are in the direct hierarchy path from the top level.
	//

	++level ;

	int i ;
	int j ;
	int idx = 0 ;
	int RCode ;

	bool selected ;
	bool errors = false ;

	if ( hpanel == "" )
	{
		vcopy( "ZHTOP", hpanel, MOVE ) ;
		if ( hpanel == "" )
		{
			setmsg( "PTUT011B" ) ;
			return 24 ;
		}
	}

	string shpanel ;
	string zsel ;
	string zcont ;
	string zhindex ;
	string zhtop ;
	string zind ;
	string zcmd ;
	string zup ;
	string zverb ;
	string zhtran ;
	string zerrsm ;
	string zerrlm ;
	string zscrolla ;

	string t ;

	vector<string> zhtrans ;
	vector<string> history ;

	map<string,int> historyi ;

	const string vlist1 = "ZCMD ZVERB ZHTRAN00 ZERRSM ZERRLM ZSCROLLA" ;
	const string vlist2 = "ZSEL ZCONT ZIND ZUP" ;

	vdefine( vlist1, &zcmd, &zverb, &zhtran, &zerrsm, &zerrlm, &zscrolla ) ;
	vdefine( vlist2, &zsel, &zcont, &zind, &zup ) ;

	shpanel = hpanel ;

	while ( true )
	{
		zcont  = "" ;
		zind   = "" ;
		zup    = "" ;
		zsel   = "" ;
		zhtran = "" ;
		if ( !errors ) { zcmd = "" ; }
		errors = false ;
		control( "ERRORS", "RETURN" ) ;
		panel_tutor_history( history, historyi, hpanel, idx ) ;
		display( hpanel ) ;
		RCode = RC ;
		control( "ERRORS", "CANCEL" ) ;
		vget( "ZVERB ZSCROLLA", SHARED ) ;
		trim( zcmd ) ;
		trim( zsel ) ;
		selected = ( zsel != "" && zsel != "?" ) ;
		if ( RCode >= 12 )
		{
			say( "Panel " + hpanel + " error.  " + zerrsm + " - " + zerrlm ) ;
			hpanel = panel_tutor_get_history( history, historyi, idx, hpanel ) ;
			if ( hpanel != "" )
			{
				continue ;
			}
		}
		if ( zcmd == "END" || RCode > 0 ) { break ; }
		if ( zhtran != "" )
		{
			zhtrans.clear() ;
			j = ds2d( zhtran ) ;
			for ( i = 1 ; i <= j ; ++i )
			{
				vcopy( "ZHTRAN" + d2ds( i, 2 ), t ) ;
				if ( t != "" && t.front() != '*' )
				{
					zhtrans.push_back( t ) ;
				}
			}
			zhtran = "" ;
		}
		if ( ( zcmd == "" && zverb == "" ) || zverb == "RIGHT" || selected )
		{
			if ( zcont != "" )
			{
				hpanel = upper( zcont ) ;
				continue ;
			}
			else if ( ( zhtrans.size() == 0 || idx == zhtrans.size() ) && !selected )
			{
				if ( level > 0 )
				{
					vdelete( vlist1, vlist2 ) ;
					return 12 ;
				}
				hpanel = shpanel ;
				continue ;
			}
			if ( selected )
			{
				string t = ( zsel.front() == '*' ) ? zsel.substr( 1 ) : zsel ;
				iupper( t ) ;
				RCode = panel_tutor( t, level ) ;
				if ( RCode == 24 ) { break ; }
				idx = 0 ;
			}
			else
			{
				RCode = panel_tutor( zhtrans[ idx ], level ) ;
				if ( RCode == 24 ) { break ; }
				++idx ;
			}
			if ( RCode == 4 )
			{
				hpanel = panel_tutor_get_history( history, historyi, idx, hpanel ) ;
				if ( hpanel != "" ) { continue ; }
				if ( level > 0 )
				{
					vdelete( vlist1, vlist2 ) ;
					return 0 ;
				}
				else
				{
					hpanel = shpanel ;
				}
			}
			else if ( RCode == 8 && idx > 0 )
			{
				--idx ;
			}
			else if ( RCode == 12 )
			{
				if ( !selected )
				{
					control( "NONDISPL", "ENTER" ) ;
				}
			}
			else if ( RCode == 16 )
			{
				if ( selected )
				{
					idx = panel_tutor_get_index( zhtrans, zsel ) ;
					if ( idx == -1 )
					{
						idx = 0 ;
					}
					else
					{
						++idx ;
						if ( idx >= zhtrans.size() )
						{
							if ( level > 0 )
							{
								vdelete( vlist1, vlist2 ) ;
								return 16 ;
							}
							idx = 0 ;
						}
						else
						{
							control( "NONDISPL", "ENTER" ) ;
						}
					}
				}
				else
				{
					if ( idx >= zhtrans.size() )
					{
						idx = 0 ;
					}
					else
					{
						control( "NONDISPL", "ENTER" ) ;
					}
				}
			}
			else if ( RCode == 20 )
			{
				if ( level > 0 )
				{
					vdelete( vlist1, vlist2 ) ;
					return 20 ;
				}
				else
				{
					vcopy( "ZHTOP", hpanel, MOVE ) ;
					if ( hpanel == "" )
					{
						hpanel = shpanel ;
					}
				}
			}
		}
		else if ( zverb == "UP" && zscrolla == "MAX" )
		{
			if ( level > 0 )
			{
				vdelete( vlist1, vlist2 ) ;
				return 20 ;
			}
			else
			{
				vcopy( "ZHTOP", hpanel, MOVE ) ;
				if ( hpanel == "" )
				{
					hpanel = shpanel ;
				}
			}
		}
		else if ( zverb == "UP" || zcmd == "U" || zcmd == "UP" )
		{
			if ( zind == "YES" && zup != "" )
			{
				hpanel = zup ;
			}
			else if ( level > 0 )
			{
				vdelete( vlist1, vlist2 ) ;
				return 8 ;
			}
			else if ( zup != "" )
			{
				hpanel = zup ;
			}
		}
		else if ( zverb == "DOWN" || zcmd == "S" || zcmd == "SKIP" )
		{
			if ( level > 0 )
			{
				vdelete( vlist1, vlist2 ) ;
				return 16 ;
			}
			else
			{
				hpanel = shpanel ;
			}
		}
		else if ( zverb == "LEFT" || zcmd == "B" || zcmd == "BACK" )
		{
			hpanel = panel_tutor_get_history( history, historyi, idx, hpanel ) ;
			if ( hpanel == "" && level > 0 )
			{
				vdelete( vlist1, vlist2 ) ;
				return 0 ;
			}
			else if ( hpanel == "" )
			{
				hpanel = shpanel ;
			}
		}
		else if ( zcmd == "I" || zcmd == "INDEX" )
		{
			vcopy( "ZHINDEX", zhindex, MOVE ) ;
			if ( zhindex == "" )
			{
				setmsg( "PTUT011A" ) ;
			}
			else
			{
				hpanel = upper( zhindex ) ;
			}
			continue ;
		}
		else if ( zcmd == "T" || zcmd == "TOC" )
		{
			vcopy( "ZHTOP", zhtop, MOVE ) ;
			if ( zhtop == "" )
			{
				setmsg( "PTUT011B" ) ;
			}
			else
			{
				hpanel = upper( zhtop ) ;
			}
			continue ;
		}
		else if ( zsel == "?" || zcmd != "" )
		{
			setmsg( "PTUT011C" ) ;
			errors = true ;
		}
	}

	vdelete( vlist1, vlist2 ) ;

	return 24 ;
}


void ptutora::panel_tutor_history( vector<string>& history,
				   map<string,int>& historyi,
				   const string& hpanel,
				   const int& idx )
{
	//
	// Add panel to the history vector if not the most recent panel.
	//

	if ( history.size() == 0 || history.back() != hpanel )
	{
		history.push_back( hpanel ) ;
	}

	historyi[ hpanel ] = idx ;
}


string ptutora::panel_tutor_get_history( vector<string>& history,
					 map<string,int>& historyi,
					 int& idx,
					 const string& hpanel )
{
	//
	// Return panel from the history that is not currently being displayed.
	//

	string h ;

	while ( history.size() > 0 && history.back() == hpanel )
	{
		history.pop_back() ;
	}

	if ( history.size() > 0 )
	{
		h = history.back() ;
		history.pop_back() ;
		idx = historyi[ h ] ;
		return h ;
	}

	return "" ;
}


int ptutora::panel_tutor_get_index( vector<string>& zhtrans,
				    const string& hpanel )
{
	//
	// Return the index of hpanel into zhtrans.  -1 if found on the last entry.
	//

	int i ;

	string s1 ;
	string s2 = ( hpanel.front() == '*' ) ? hpanel.substr( 1 ) : hpanel ;

	for ( i = 0 ; i < zhtrans.size() ; ++i )
	{
		s1 = ( zhtrans[ i ].front() == '*' ) ? zhtrans[ i ].substr( 1 ) : zhtrans[ i ] ;
		if ( s1 == s2 ) { break ; }
	}

	return ( i < zhtrans.size() - 2 ) ? i : -1 ;
}
