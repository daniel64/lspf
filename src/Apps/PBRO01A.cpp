/*  Compile with ::                                                                                          */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPBRO01A.so -lmagic -lboost_regex -o libPBRO01A.so PBRO01A.cpp */

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

/******************************************************************************************/
/*  ZRC/ZRSN exit codes (RC=ZRC in the calling program)                                   */
/*    0/0     Normal completion                                                           */
/*   12/0     File empty                                                                  */
/*   14/0     File does not exist                                                         */
/*   20/0     Severe error.                                                               */
/*   20/4     Cannot open file.  Permission denied.                                       */
/*   20/8     Cannot open file.  Invalid file type.                                       */
/*   20/12    Cannot open file.  Unknown open error.                                      */

/* ZRESULT contains the relevant message id for RC > 0                                    */

/* Commands:                                                                              */
/*  BINARY (BIN)                                                                          */
/*      Open file as binary                                                               */
/*  BROWSE                                                                                */
/*      Browse file                                                                       */
/*  COLUMNS ON | OFF (COLS)                                                               */
/*      Display columns on top line of data                                               */
/*  EDIT                                                                                  */
/*      Edit file                                                                         */
/*  FIND (F)                                                                              */
/*      Find string                                                                       */
/*  HEX ON | OFF | DATA | VERT | ON DATA | ON VERT                                        */
/*      Display data as hex (DATA or VERT format)                                         */
/*  HILITE (HI HILIGHT)                                                                   */
/*      Set hilite language                                                               */
/*  LOCATE (L LOC)                                                                        */
/*      Locate line or label                                                              */
/*  RESET (RES)                                                                           */
/*      Reset options ( COLS OFF )                                                        */
/*  RFIND                                                                                 */
/*      Repeat find                                                                       */
/*  TEXT                                                                                  */
/*      Open file as text (0A will be interpreted as newline)                             */
/*  .label                                                                                */
/*      Set label (must be alphnumeric, max 8 chars, case insensitive)                    */
/*  &cmd                                                                                  */
/*      Keeps command on the command line (including on SPLIT/SWAP)                       */
/******************************************************************************************/

#include <boost/regex.hpp>
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
#include <magic.h>
#include "eHilight.cpp"
#include "PBRO01A.h"

using namespace boost ;
using namespace std   ;
using namespace boost::filesystem ;

#undef  MOD_NAME
#define MOD_NAME PBRO01A

map<int,b_find> PBRO01A::Global_bfind_parms ;


PBRO01A::PBRO01A()
{
	set_appdesc( "Browser for lspf" ) ;
	set_appver( "1.0.3" ) ;

	vdefine( "ZCMD ZVERB ZROW1 ZCURFLD", &zcmd, &zverb, &zrow1, &zcurfld ) ;
	vdefine( "ZAREA ZSHADOW ZDSN", &zarea, &zshadow, &zfile ) ;
	vdefine( "ZSCROLLN ZAREAW ZAREAD ZCURPOS", &zscrolln, &zareaw, &zaread, &zcurpos ) ;
	vdefine( "ZSCROLLA ZCOL1 ZCOL2 TYPE STR", &zscrolla, &zcol1, &zcol2, &type, &str ) ;
	vdefine( "OCC LINES CMD ZZSTR1", &occ, &lines, &cmd, &zzstr1 ) ;

	XRC  = 0 ;
	XRSN = 0 ;
}


void PBRO01A::application()
{
	browse_parms* b_parms = static_cast<browse_parms*>( get_options() ) ;

	if ( !b_parms ) { return ; }

	zfile = b_parms->browse_file ;
	panel = b_parms->browse_panel ;

	if ( zfile == "" ) { return ; }

	initialise()  ;

	Browse() ;

	ZRC  = XRC ;
	ZRSN = XRSN ;
}


void PBRO01A::initialise()
{
	vcopy( "ZSCRNUM", zscrnum, MOVE ) ;

	if ( panel == "" ) { panel = "PBRO01A2" ; }

	pquery( panel, "ZAREA", "", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 ) { abend() ; }

	zasize = zareaw * zaread ;

	s1b = string( zareaw, E_BLUE   ) ;
	s1g = string( zareaw, E_GREEN  ) ;
	s1y = string( zareaw, E_YELLOW ) ;
	s1w = string( zareaw, E_WHITE  ) ;
	div = string( zareaw, '-' )      ;
}


void PBRO01A::Browse()
{
	int aRow ;
	int aCol ;

	string zbralt ;
	string zxcmd  ;

	a_parms action_parms ;

	rebuildZAREA = true  ;
	hexOn        = false ;
	vertOn       = true  ;
	colsOn       = false ;
	asBin        = false ;
	binOn        = false ;
	textOn       = false ;
	hilightOn    = true  ;
	entLang      = ""    ;
	msg          = ""    ;
	zxcmd        = ""    ;
	startCol     = 1     ;
	maxCol       = 1     ;

	read_file() ;
	if ( XRC > 0 ) { return ; }


	auto it = Global_bfind_parms.find( ds2d( zscrnum ) ) ;
	if ( it != Global_bfind_parms.end() )
	{
		find_parms = it->second ;
	}

	zbralt = "" ;
	vget( "ZBRALT", SHARED ) ;
	if ( RC == 0 )
	{
		vcopy( "ZBRALT", zbralt, MOVE ) ;
	}
	if ( zbralt == "" ) { vreplace( "ZBRALT", zfile ) ; }

	cursor.home() ;

	while ( true )
	{
		if ( rebuildZAREA ) { fill_dynamic_area() ; }
		else                { zshadow = cshadow   ; }
		rebuildZAREA = false ;

		zrow1 = d2ds( topLine, 8 )  ;
		zcol1 = d2ds( startCol, 5 ) ;
		zcol2 = d2ds( startCol+zareaw-1, 5 ) ;
		zcmd  = ( action_parms.error() || ( zxcmd.size() > 1 && zxcmd.front() == '&' ) ) ? zxcmd : "" ;

		cursor.placecursor( zaread, zareaw, topLine, startCol, hexOn, colsOn ) ;
		curfld = cursor.curfld() ;
		curpos = cursor.curpos() ;

		if ( curfld == "ZAREA" )
		{
			hilite_cursor() ;
		}

		display( panel, msg, curfld, curpos ) ;
		if ( RC == 8 ) { break ; }

		msg = "" ;

		vget( "ZVERB ZSCROLLA ZSCROLLN", SHARED ) ;

		if ( zcurfld == "ZAREA" )
		{
			if ( colsOn && zcurpos <= zareaw )
			{
				cursor.home() ;
			}
			else if ( topLine == 0 && ( zcurpos <= zareaw || (colsOn && zcurpos <= 2*zareaw) ) )
			{
				cursor.home() ;
			}
			else
			{
				aRow = ((zcurpos-1) / zareaw + 1) ;
				aCol = ((zcurpos-1) % zareaw + 1) ;
				cursor.set( "ZAREA", aRow, aCol+startCol-2, topLine, hexOn, colsOn ) ;
			}
		}
		else
		{
			cursor.home() ;
		}

		zxcmd = zcmd ;
		if ( zcmd.size() > 1 && zcmd.front() == '&' )
		{
			zcmd.erase( 0, 1 ) ;
		}

		cmd = upper( word( zcmd, 1 ) ) ;
		if ( cmd == "RFIND" )
		{
			zverb = "RFIND" ;
			zcmd  = subword( zcmd, 2 ) ;
			cmd   = upper( word( zcmd, 1 ) ) ;
		}

		action_parms.set( zcmd, cmd ) ;

		auto it1 = cmdList.find( cmd ) ;
		if ( it1 != cmdList.end() )
		{
			(this->*(it1->second))( action_parms ) ;
			msg = action_parms.msg ;
			if ( XRC > 0 ) { return ; }
		}
		else if ( cmd.size() > 1 && cmd.front() == '.' )
		{
			set_label( action_parms ) ;
			msg = action_parms.msg ;
			if ( msg != "" ) { continue ; }
		}
		else if ( cmd != "" )
		{
			action_parms.errorid( "PBRO011" ) ;
			msg = action_parms.msg ;
			continue ;
		}

		auto it2 = zverbList.find( zverb ) ;
		if ( it2 != zverbList.end() )
		{
			(this->*(it2->second))() ;
		}
	}

	verase( "ZBRALT", SHARED ) ;
	vput( "ZSCROLL", PROFILE ) ;
}


void PBRO01A::read_file()
{
	string inLine ;
	string line1  ;
	string ext    ;
	string w1     ;

	b_shadow t    ;

	int rc ;
	int i  ;
	int j  ;

	size_t p1 ;

	char x ;

	RC      = 0 ;
	topLine = 0 ;

	std::ifstream fin ;

	try
	{
		if ( !exists( zfile ) )
		{
			vreplace( "ZVAL1", zfile ) ;
			vput( "ZVAL1", SHARED ) ;
			XRC  = 14 ;
			XRSN = 0  ;
			ZRESULT = "PSYS011S" ;
			return    ;
		}
	}
	catch ( const filesystem_error& ex )
	{
		vreplace( "ZVAL1", zfile ) ;
		vput( "ZVAL1", SHARED ) ;
		XRC  = 20 ;
		XRSN = 4  ;
		ZRESULT = "PSYS011T" ;
		return    ;
	}

	if ( !is_regular_file( zfile.c_str() ) )
	{
		vreplace( "ZVAL1", zfile ) ;
		vput( "ZVAL1", SHARED ) ;
		XRC  = 20 ;
		XRSN = 8  ;
		ZRESULT = "PSYS011R" ;
		return    ;
	}

	fin.open( zfile.c_str() ) ;

	if ( !fin.is_open() )
	{
		vreplace( "ZVAL1", zfile ) ;
		vput( "ZVAL1", SHARED ) ;
		XRC  = 20 ;
		XRSN = 12 ;
		ZRESULT = "PSYS011U" ;
		return    ;
	}

	fileType = "text/plain" ;
	magic_t cookie = magic_open( MAGIC_CONTINUE | MAGIC_ERROR | MAGIC_MIME | MAGIC_SYMLINK ) ;
	rc = magic_load( cookie, NULL ) ;

	if ( rc == 0 )
	{
		fileType = magic_file( cookie, zfile.c_str() ) ;
		if ( fileType != "" )
		{
			if ( findword( "charset=binary", fileType ) )
			{
				asBin = true ;
			}
			else
			{
				p1 = fileType.find( ';' ) ;
				if ( p1 != string::npos )
				{
					fileType = fileType.substr( 0, p1 ) ;
				}
			}
		}
	}
	magic_close( cookie ) ;

	maxLines = 1 ;
	data.clear()   ;
	shadow.clear() ;
	data.push_back( centre( " Top of Data ", zareaw, '*' ) ) ;
	shadow.push_back( t ) ;

	if      ( binOn )  { asBin = true ; }
	else if ( textOn ) { asBin = false ; }

	if ( asBin )
	{
		inLine = string( zareaw, ' ' ) ;
		maxCol = zareaw ;
		i      = 0      ;
		while ( true )
		{
			fin.get( x ) ;
			if ( fin.fail() != 0 ) { break ; } ;
			inLine[ i ] = x ;
			++i ;
			if ( i == zareaw )
			{
				data.push_back( inLine )       ;
				inLine = string( zareaw, ' ' ) ;
				shadow.push_back( t ) ;
				++maxLines ;
				i = 0      ;
			}
		}
		if ( i > 0 )
		{
			inLine.resize( i )       ;
			data.push_back( inLine ) ;
			shadow.push_back( t ) ;
			++maxLines ;
		}
		fileType  = "application/octet-stream" ;
		hilightOn = false ;
	}
	else
	{
		while ( getline( fin, inLine ) )
		{
			if ( maxLines == 1 ) { line1 = inLine ; }
			p1 = inLine.find( '\t' ) ;
			while ( p1 != string::npos )
			{
				j = 8 - ( p1 % 8 ) ;
				inLine.replace( p1, 1, j, ' ' ) ;
				p1 = inLine.find( '\t', p1 + 1 ) ;
			}
			if ( maxCol < inLine.size() ) maxCol = inLine.size() ;
			data.push_back( inLine ) ;
			shadow.push_back( t ) ;
			++maxLines ;
		}
		if ( entLang == "" || entLang == "AUTO" )
		{
			detLang = determineLang() ;
		}
		else
		{
			detLang = entLang ;
		}
		hilightOn = true  ;
	}

	if ( maxLines == 1 )
	{
		vreplace( "ZVAL1", zfile ) ;
		vput( "ZVAL1", SHARED ) ;
		XRC  = 12 ;
		XRSN = 0  ;
		ZRESULT = "PSYS011P" ;
		return    ;
	}

	++maxCol   ;
	++maxLines ;

	data.push_back( centre( " Bottom of Data ", zareaw, '*' ) ) ;
	shadow.push_back( t ) ;
	fin.close() ;
}


void PBRO01A::fill_dynamic_area()
{
	string w   ;
	string t1  ;
	string t2  ;
	string t3  ;
	string t4  ;
	string col ;

	int i  ;
	int l  ;
	int t  ;
	int ln ;

	if ( colsOn )
	{
		col = substr( "+----+----+----+---", startCol%10+1, 10-startCol%10 ) ;
		if ( startCol%10 == 0 ) { col = "" ; t = startCol / 10 ; }
		else t = startCol / 10 + 1 ;
		for ( i = 0 ; i < (zareaw/10+1) ; ++i )
		{
			col = col + d2ds( t%10 ) + "----+----" ;
			++t ;
		}
		col = substr( col, 1, zareaw ) ;
		zarea   = col ;
		zshadow = s1w ;
	}
	else
	{
		zarea   = "" ;
		zshadow = "" ;
	}

	if ( hexOn )
	{
		for ( uint k = topLine ; k < (topLine + zaread) ; ++k )
		{
			t3 = string( zareaw, ' ' ) ;
			t4 = string( zareaw, ' ' ) ;
			if ( k > 0 && k < data.size()-1 )
			{
				ln = data.at( k ).size() - startCol + 1 ;
				if ( ln > zareaw ) { ln = zareaw ; }
				t1 = substr( data.at( k ), startCol, zareaw ) ;
				if ( vertOn )
				{
					t2 = cs2xs( t1 ) ;
					if ( ln > 0 )
					{
						i = 0 ;
						for ( l = 0 ; l < (ln * 2) ; ++l )
						{
							t3[ i ] = t2[ l ] ;
							++l ;
							t4[ i ] = t2[ l ] ;
							++i ;
						}
					}
				}
				else
				{
					t3  = cs2xs( t1.substr( 0, ln ) ) ;
					t3 += copies( "20", ( zareaw*2-t3.size() )/2 ) ;
					t4  = t3.substr( zareaw ) ;
					t3.erase( zareaw ) ;
				}
				zarea   += t1  + t3  + t4  + div ;
				zshadow += s1y + s1b + s1b + s1w ;
			}
			else
			{
				zarea   += substr( data.at( k ), 1, zareaw ) ;
				zshadow += s1b ;
			}
			if ( zarea.size() >= zasize || k >= ( data.size() - 1 ) ) { break ; }
		}
	}
	else
	{
		for ( int k = topLine ; k < (topLine + zaread) ; ++k )
		{
			if ( k > 0 && k < data.size()-1 ) { t1 = substr( data.at( k ), startCol, zareaw ) ; }
			else                              { t1 = substr( data.at( k ), 1, zareaw )        ; }
			zarea += t1 ;
			if ( zarea.size() >= zasize ) { break ; }
			if ( k == data.size() - 1 )   { break ; }
		}

	}

	zarea.resize( zasize, ' ' ) ;
	zshadow.resize( zasize, E_BLUE ) ;

	if ( hilightOn && !hlight.hl_abend )
	{
		fill_hilight_shadow() ;
	}
	cshadow = zshadow ;
}


void PBRO01A::fill_hilight_shadow()
{
	// Build il_shadow starting at the first invalid shadow line in bs_shadow,
	// (backing up to the line after the position where there are no open brackets/comments)
	// until bottom of ZAREA reached

	int i  ;
	int dl ;
	int ll ;
	int w  ;

	string::const_iterator it1 ;
	string::const_iterator it2 ;

	vector<b_shadow>::iterator its ;
	vector<string>::iterator itd ;

	hlight.hl_language = detLang ;

	ll = data.size()-2 ;
	if ( topLine+zaread < ll ) { ll = topLine+zaread ; }

	for ( w = 0, dl = 1 ; dl <= ll ; ++dl )
	{
		if ( !shadow.at( dl ).bs_vShadow ) { break  ; }
		if (  shadow.at( dl ).bs_wShadow ) { w = dl ; }
	}

	if ( dl <= ll )
	{
		hlight.hl_oBrac1   = 0     ;
		hlight.hl_oBrac2   = 0     ;
		hlight.hl_oIf      = 0     ;
		hlight.hl_oDo      = 0     ;
		hlight.hl_ifLogic  = true  ;
		hlight.hl_doLogic  = true  ;
		hlight.hl_Paren    = true  ;
		hlight.hl_oComment = false ;
		for ( dl = w + 1 ; dl <= ll ; ++dl )
		{
			addHilight( lg, hlight, data[ dl ], shadow[ dl ].bs_Shadow ) ;
			if ( hlight.hl_abend ) { return ; }
			shadow[ dl ].bs_vShadow = true ;
			shadow[ dl ].bs_wShadow = ( hlight.hl_oBrac1 == 0 &&
						    hlight.hl_oBrac2 == 0 &&
						    hlight.hl_oIf    == 0 &&
						    hlight.hl_oDo    == 0 &&
						   !hlight.hl_continue    &&
						   !hlight.hl_oQuote      &&
						   !hlight.hl_oComment ) ;
		}
	}

	itd = data.begin() + topLine ;
	its = shadow.begin() + topLine ;
	i   = ( colsOn ) ? 1 : 0 ;

	for ( ; i < zaread && itd != data.end() ; ++i, ++its, ++itd )
	{
		if ( topLine == 0 && i == ( colsOn ) ? 1 : 0 ) { continue ; }
		if ( its->bs_Shadow.size() >= startCol )
		{
			it1 = zshadow.begin() + ( zareaw * i ) ;
			it2 = its->bs_Shadow.begin() + startCol - 1 ;
			w = its->bs_Shadow.size() - startCol + 1 ;
			if ( w > zareaw ) { w = zareaw ; }
			zshadow.replace( it1, it1+w, it2, it2+w ) ;
		}
		if ( hexOn ) { i += 3 ; }
	}
}


void PBRO01A::hilite_cursor()
{
	int i ;

	for ( i = curpos-1 ; i < zasize ; ++i )
	{
		if ( zarea[ i ] == ' ' || ( i / zareaw ) > ( data.size() - topLine - 2 ) ) { break ; }
		zshadow[ i ] = E_WHITE ;
	}
	for ( i = curpos-1 ; i >= 0 ; --i )
	{
		if ( zarea[ i ] == ' ' || ( topLine == 0 && ( i / zareaw ) == 0 ) ) { break ; }
		zshadow[ i ] = E_WHITE ;
	}
}


void PBRO01A::set_label( a_parms& parms )
{
	string label = parms.cmd.substr( 1 ) ;

	if ( parms.w2 != "" )
	{
		parms.errorid( "PBRO013" ) ;
		return ;
	}

	if ( !check_label( label, parms ) ) { return ; }

	vreplace( "ZVAL1", label ) ;
	vreplace( "ZVAL2", d2ds( topLine ) ) ;
	parms.errorid( ( labelList.count( label ) == 0 ) ? "PBRO011T" : "PBRO011U", 4 ) ;

	labelList[ label ] = topLine ;
}


void PBRO01A::action_ScrollUp()
{
	int amnt  ;
	int t = 0 ;

	if ( zscrolla == "MAX" )
	{
		topLine = 0 ;
	}
	else
	{
		amnt = ( hexOn && !datatype( zscrolla, 'W') ) ? 4 : 1 ;
		for ( ; topLine > 0 ; --topLine )
		{
			t += amnt ;
			if ( t > zscrolln ) { break ; }
		}
	}

	rebuildZAREA = true ;
}


void PBRO01A::action_ScrollDown()
{
	int amnt  ;
	int t = 0 ;

	if ( zscrolla == "MAX" )
	{
		topLine = data.size() - 1 ;
		amnt    = ( hexOn ) ? 4 : 1 ;
		for ( ; topLine > 0 ; --topLine )
		{
			t += amnt ;
			if ( t >= zaread ) { break ; }
		}
	}
	else
	{
		amnt = ( hexOn && !datatype( zscrolla, 'W') ) ? 4 : 1 ;
		for ( ; topLine < ( data.size() - 1 ) ; ++topLine )
		{
			t += amnt ;
			if ( t > zscrolln ) { break ; }
		}
	}

	rebuildZAREA = true ;
}


void PBRO01A::action_ScrollLeft()
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


void PBRO01A::action_ScrollRight()
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


void PBRO01A::action_BINARY( a_parms& parms )
{
	if ( parms.w2 != "" )
	{
		parms.errorid( "PBRO013" ) ;
		return ;
	}

	asBin        = true  ;
	binOn        = true  ;
	textOn       = false ;
	rebuildZAREA = true  ;
	read_file() ;
}


void PBRO01A::action_BROWSE( a_parms& parms )
{
	string w2 ;

	control( "ERRORS", "RETURN" ) ;
	w2 = word( parms.zcmd, 2 ) ;
	if ( w2 == "" )
	{
		select( "PGM(PPSP01A) PARM(BROWSEE) SCRNAME(BROWSE) SUSPEND" ) ;
	}
	else
	{
		if ( w2.front() != '/' )
		{
			w2 = zfile.substr( 0, zfile.find_last_of( '/' ) + 1 ) + w2 ;
		}
		browse( w2 ) ;
		if ( RC > 0 && isvalidName( ZRESULT ) )
		{
			parms.errorid( ZRESULT ) ;
		}
	}
	control( "ERRORS", "CANCEL" ) ;
}


void PBRO01A::action_EDIT( a_parms& parms )
{
	string w2 ;

	control( "ERRORS", "RETURN" ) ;
	w2 = word( parms.zcmd, 2 ) ;
	if ( w2 == "" )
	{
		select( "PGM(PPSP01A) PARM(EDITEE) SCRNAME(EDIT) SUSPEND" ) ;
	}
	else
	{
		if ( w2.front() != '/' )
		{
			w2 = zfile.substr( 0, zfile.find_last_of( '/' ) + 1 ) + w2 ;
		}
		edit( w2 ) ;
		if ( ZRESULT != "" && ( RC == 0 || RC == 4 ) )
		{
			setmsg( ZRESULT ) ;
		}
		else if ( RC > 11 && isvalidName( ZRESULT ) )
		{
			parms.errorid( ZRESULT ) ;
		}
	}
	control( "ERRORS", "CANCEL" ) ;
}


void PBRO01A::action_HEX( a_parms& parms )
{
	if ( parms.wds > 3 )
	{
		parms.errorid( "PBRO013" ) ;
		return ;
	}

	if ( parms.w2 == "" ||
	   ( parms.w2 == "ON" && ( parms.w3 == "" || parms.w3 == "VERT" ) ) ||
	   ( parms.w2 == "VERT" && parms.w3 == "" ) )
	{
		hexOn  = true ;
		vertOn = true ;
		rebuildZAREA = true ;
	}
	else if ( ( parms.w2 == "ON"   && parms.w3 == "DATA" ) ||
		  ( parms.w2 == "DATA" && parms.w3 == "" ) )
	{
		hexOn  = true  ;
		vertOn = false ;
		rebuildZAREA = true ;
	}
	else if ( parms.w2 == "OFF" && parms.w3 == "" )
	{
		hexOn = false ;
		rebuildZAREA = true ;
	}
	else
	{
		parms.errorid( "PBRO012" ) ;
	}
}


void PBRO01A::action_HILIGHT( a_parms& parms )
{
	if ( parms.w3 == "" )
	{
		parms.w3 = "AUTO" ;
	}

	if ( parms.w3 != "AUTO" && !addHilight( parms.w3 ) )
	{
		parms.errorid( "PBRO011M" ) ;
		return ;
	}

	entLang = parms.w3 ;
	if ( parms.w3 == "AUTO"  )
	{
		detLang = determineLang() ;
	}
	else
	{
		detLang = parms.w3 ;
	}

	if ( parms.w2 == "ON" )
	{
		for_each( shadow.begin(), shadow.end(),
		    [](b_shadow& a)
		{
			a.bs_wShadow = false ;
			a.bs_vShadow = false ;
		} ) ;
		hilightOn = true  ;
		rebuildZAREA = true ;
	}
	else if ( parms.w2 == "OFF" )
	{
		for_each( shadow.begin(), shadow.end(),
		    [](b_shadow& a)
		{
			a.bs_wShadow = false ;
			a.bs_vShadow = false ;
		} ) ;
		hilightOn = false ;
		rebuildZAREA = true ;
	}
}


void PBRO01A::action_COLS( a_parms& parms )
{
	if ( parms.w2 == "" || ( parms.w2 == "ON" && parms.w3 == "" ) )
	{
		colsOn = true ;
		rebuildZAREA = true ;
	}
	else if ( parms.w2 == "OFF" && parms.w3 == "" )
	{
		colsOn = false ;
		rebuildZAREA = true ;
	}
	else
	{
		parms.errorid( "PBRO012" ) ;
	}
}


void PBRO01A::action_FIND( a_parms& parms )
{
	if ( setFind( parms ) > 0 ) { return ; }

	if ( zcurfld == "ZAREA" )
	{
		offset = zcurpos ;
		if ( ( offset % zareaw ) == 1  )
		{
			++offset ;
		}
	}
	else
	{
		offset = 0 ;
	}

	performFind( topLine, offset ) ;
	if ( find_parms.f_line > 0 )
	{
		zaline = find_parms.f_line - topLine ;
		if ( hexOn ) { zaline = 4*zaline ; }
		if ( zaline < 0 || zaline > zaread-1 )
		{
			topLine = find_parms.f_line - 1 ;
		}
		if ( find_parms.f_offset < startCol-1 || find_parms.f_offset > zareaw + startCol - 1 )
		{
			startCol = find_parms.f_offset - 13 ;
			if ( startCol < 1 )
			{
				startCol = 1 ;
			}
		}
		cursor.set( "ZAREA", find_parms.f_line, find_parms.f_offset ) ;
		type  = typeList[ find_parms.f_mtch ] ;
		str   = find_parms.f_estring ;
		occ   = d2ds( find_parms.f_occurs ) ;
		lines = d2ds( find_parms.f_lines  ) ;
		if ( find_parms.f_dir == 'A' )
		{
			parms.errorid( "PBRO011G", 4 ) ;
			find_parms.f_dir = 'N' ;
		}
		else
		{
			parms.errorid( "PBRO011F", 4 ) ;
		}
	}
	else
	{
		cursor.home() ;
		type   = typeList[ find_parms.f_mtch ] ;
		str    = find_parms.f_estring ;
		parms.errorid( "PBRO011E", 4 ) ;
		return ;
	}
	rebuildZAREA = true ;
}


void PBRO01A::action_RFIND()
{
	if ( !find_parms.f_fset )
	{
		msg = "PBRO011A" ;
		return ;
	}

	if ( zcurfld == "ZAREA" )
	{
		offset = zcurpos ;
		if ( ( offset % zareaw ) == 1 )
		{
			++offset ;
		}
	}
	else
	{
		offset = 0 ;
	}

	performFind( topLine, offset ) ;
	if ( find_parms.f_line > 0 )
	{
		zaline = find_parms.f_line - topLine ;
		if ( hexOn ) { zaline = 4*zaline ; }
		if ( zaline < 0 || zaline > zaread-1 )
		{
			topLine = find_parms.f_line - 1 ;
		}
		if ( find_parms.f_offset < startCol-1 || find_parms.f_offset > zareaw + startCol - 1 )
		{
			startCol = find_parms.f_offset - 13 ;
			if ( startCol < 1 )
			{
				startCol = 1 ;
			}
		}
		cursor.set( "ZAREA", find_parms.f_line, find_parms.f_offset ) ;
		type = typeList[ find_parms.f_mtch ] ;
		str  = find_parms.f_estring ;
		msg  = "PBRO011F" ;
	}
	else
	{
		if ( find_parms.f_dir == 'N' || find_parms.f_dir == 'F' )
		{
			find_parms.f_dir = 'F' ;
			msg = "PBRO016" ;
		}
		else
		{
			find_parms.f_dir = 'L' ;
			msg = "PBRO017" ;
		}
		return ;
	}
	rebuildZAREA = true ;
}


void PBRO01A::action_LOCATE( a_parms& parms )
{
	string label ;

	if ( parms.w2 == "" )
	{
		parms.errorid( "PBRO011Q" ) ;
		return ;
	}

	if ( parms.w3 != "" )
	{
		parms.errorid( "PBRO013" ) ;
		return ;
	}

	if ( datatype( parms.w2, 'W' ) )
	{
		topLine = ds2d( parms.w2 ) ;
		if ( topLine >= maxLines )
		{
			topLine = maxLines - 1 ;
		}
		rebuildZAREA = true ;
	}
	else
	{
		label = ( parms.w2.front() == '.' ) ? parms.w2.substr( 1 ) : parms.w2 ;
		if ( !check_label( label, parms ) ) { return ; }
		if ( labelList.count( label ) > 0 )
		{
			topLine = labelList[ label ] ;
			vreplace( "ZVAL1", label ) ;
			vreplace( "ZVAL2", d2ds( topLine ) ) ;
			parms.errorid( "PBRO011R", 4 ) ;
			rebuildZAREA = true ;
		}
		else
		{
			parms.errorid( "PBRO011S" ) ;
		}
	}
}


void PBRO01A::action_RESET( a_parms& parms )
{
	if ( parms.w2 != "" )
	{
		parms.errorid( "PBRO013" ) ;
		return ;
	}

	colsOn       = false ;
	binOn        = false ;
	textOn       = false ;
	rebuildZAREA = true  ;
}


void PBRO01A::action_TEXT( a_parms& parms )
{
	if ( parms.w2 != "" )
	{
		parms.errorid( "PBRO013" ) ;
		return ;
	}

	asBin        = false ;
	binOn        = false ;
	textOn       = true  ;
	rebuildZAREA = true  ;
	read_file()          ;
}


int PBRO01A::setFind( a_parms& parms )
{
	int i  ;
	int ws ;

	char c  ;
	char c1 ;
	char c2 ;

	bool alnum  ;
	bool alnum1 ;
	bool alnum2 ;

	string delim ;
	string com  ;
	string ucmd ;
	string w1   ;
	string pic  ;
	string pre1 ;
	string pre2 ;
	string suf1 ;
	string rcmd ;

	size_t p1 ;
	size_t p2 ;

	const char quote('\"') ;
	const char apost('\'') ;
	const string pic_chars   = "=@#$¬.-<>" ;
	const string regex_chars = "\\^$.|?*+()[{" ;

	b_find t ;

	parms.msg = "" ;
	com = " " + subword( parms.zcmd, 2 ) + " " ;

	p1 = com.find( quote ) ;
	p2 = com.find( apost ) ;

	if      ( p1 == string::npos ) { delim = string( 1, apost ) ; }
	else if ( p2 == string::npos ) { delim = string( 1, quote ) ; }
	else if ( p1 < p2 )            { delim = string( 1, quote ) ; }
	else                           { delim = string( 1, apost ) ; }

	if ( ( p1 = pos( delim, com ) ) )
	{
		p2  = pos( delim, com,  p1+1 ) ;
		if ( p2 == 0 ) { parms.errorid( "PBRO011H" ) ; return 20 ; }
		c1 = toupper( com[ p1-2 ] ) ;
		c2 = toupper( com[ p2   ] ) ;
		if ( c1 == ' ' && c2 == ' ' ) { t.f_text = true ; }
		else if ( c1 == 'T' && c2 == ' ' ) { t.f_text   = true ; }
		else if ( c1 == 'C' && c2 == ' ' ) { t.f_asis   = true ; }
		else if ( c1 == 'X' && c2 == ' ' ) { t.f_hex    = true ; }
		else if ( c1 == 'P' && c2 == ' ' ) { t.f_pic    = true ; }
		else if ( c1 == 'R' && c2 == ' ' ) { t.f_rreg   = true ; }
		else if ( c2 == 'T' && c1 == ' ' ) { t.f_text   = true ; }
		else if ( c2 == 'C' && c1 == ' ' ) { t.f_asis   = true ; }
		else if ( c2 == 'X' && c1 == ' ' ) { t.f_hex    = true ; }
		else if ( c2 == 'P' && c1 == ' ' ) { t.f_pic    = true ; }
		else if ( c2 == 'R' && c1 == ' ' ) { t.f_rreg   = true ; }
		else                               { parms.errorid( "PBRO011I" ) ; return 20 ; }
		t.f_estring = substr( com, (p1+1), (p2-p1-1) ) ;
		t.f_string  = t.f_estring ;
		if ( t.f_text )
		{
			t.f_string = upper( t.f_estring ) ;
		}
		com = delstr( com, (p1-1), (p2-p1+3) ) ;
	}

	if ( t.f_rreg ) { t.f_regreq = true ; }

	ucmd = upper( com ) ;
	if ( ( p1 = wordpos( "NEXT", ucmd ) ) )
	{
		t.f_dir = 'N' ;
		p1   = wordindex( ucmd, p1 ) ;
		ucmd = delstr( ucmd, p1, 4 ) ;
	}
	else if ( ( p1 = wordpos( "PREV", ucmd ) ) )
	{
		t.f_dir = 'P' ;
		p1   = wordindex( ucmd, p1 ) ;
		ucmd = delstr( ucmd, p1, 4 ) ;
	}
	else if ( ( p1 = wordpos( "FIRST", ucmd ) ) )
	{
		t.f_dir = 'F' ;
		p1   = wordindex( ucmd, p1 ) ;
		ucmd = delstr( ucmd, p1, 5 ) ;
	}
	else if ( ( p1 = wordpos( "LAST", ucmd ) ) )
	{
		t.f_dir = 'L' ;
		p1   = wordindex( ucmd, p1 ) ;
		ucmd = delstr( ucmd, p1, 4 ) ;
	}
	else if ( ( p1 = wordpos( "ALL", ucmd ) ) )
	{
		t.f_dir = 'A' ;
		p1   = wordindex( ucmd, p1 ) ;
		ucmd = delstr( ucmd, p1, 3 ) ;
	}

	if ( !t.f_rreg )
	{
		if ( ( p1 = wordpos( "CHARS", ucmd ) ) )
		{
			t.f_mtch  = 'C'  ;
			p1   = wordindex( ucmd, p1 ) ;
			ucmd = delstr( ucmd, p1, 5 ) ;
		}
		else if ( ( p1 = wordpos( "PRE", ucmd ) ) )
		{
			t.f_mtch   = 'P'  ;
			t.f_regreq = true ;
			p1   = wordindex( ucmd, p1 ) ;
			ucmd = delstr( ucmd, p1, 3 ) ;
		}
		else if ( ( p1 = wordpos( "PREFIX", ucmd ) ) )
		{
			t.f_mtch   = 'P'  ;
			t.f_regreq = true ;
			p1   = wordindex( ucmd, p1 ) ;
			ucmd = delstr( ucmd, p1, 6 ) ;
		}
		else if ( ( p1 = wordpos( "SUF", ucmd ) ) )
		{
			t.f_mtch   = 'S'  ;
			t.f_regreq = true ;
			p1   = wordindex( ucmd, p1 ) ;
			ucmd = delstr( ucmd, p1, 3 ) ;
		}
		else if ( ( p1 = wordpos( "SUFFIX", ucmd ) ) )
		{
			t.f_mtch   = 'S'  ;
			t.f_regreq = true ;
			p1   = wordindex( ucmd, p1 ) ;
			ucmd = delstr( ucmd, p1, 6 ) ;
		}
		else if ( ( p1 = wordpos( "WORD", ucmd ) ) )
		{
			t.f_mtch   = 'W'  ;
			t.f_regreq = true ;
			p1   = wordindex( ucmd, p1 ) ;
			ucmd = delstr( ucmd, p1, 4 ) ;
		}
	}

	ws   = words( ucmd ) ;
	rcmd = "" ;
	for ( i = 1 ; i <= ws ; ++i )
	{
		w1 = word( ucmd, i ) ;
		if ( datatype( w1, 'W' ) )
		{
			rcmd += " " + w1 ;
		}
		else
		{
			if ( w1 == "*" )
			{
				if ( find_parms.f_string == "" )
				{
					parms.errorid( "PBRO011C" ) ;
					return 20 ;
				}
				else
				{
					if ( t.f_string != "" )
					{
						parms.errorid( "PBRO015" ) ;
						return 20 ;
					}
					w1 = find_parms.f_string ;
				}
			}
			if ( t.f_string != "" )
			{
				str = w1 ;
				parms.errorid( "PBRO018" ) ;
				return 20 ;
			}
			t.f_text    = true ;
			t.f_estring = w1   ;
			t.f_string  = upper( w1 ) ;
		}
	}

	if ( t.f_string == "" )
	{
		if ( rcmd == "" )
		{
			parms.errorid( "PBRO011D" ) ;
			return 20 ;
		}
		if ( words( rcmd ) > 1 )
		{
			parms.errorid( "PBRO011V" ) ;
			return 20 ;
		}
		t.f_string  = strip( rcmd ) ;
		t.f_estring = t.f_string ;
		rcmd = "" ;
	}

	i = words( rcmd ) ;
	if ( i > 0 )
	{
		t.f_scol = ds2d( word( rcmd, 1 ) ) ;
		if ( t.f_scol < 1 || t.f_scol > 65535 )
		{
			parms.errorid( "PBRO011J" ) ;
			return 20 ;
		}
		if ( i == 1 )
		{
			t.f_oncol = true ;
		}
		else if ( i == 2 )
		{
			t.f_ecol = ds2d( word( rcmd, 2 ) ) ;
			if ( t.f_ecol < 1 || t.f_ecol > 65535 )
			{
				parms.errorid( "PBRO011J" ) ;
				return 20 ;
			}
			if ( t.f_scol > t.f_ecol )
			{
				swap( t.f_scol, t.f_ecol ) ;
			}
			if ( !t.f_rreg && t.f_estring.size() > ( t.f_ecol - t.f_scol + 1 ) )
			{
				str = t.f_estring ;
				parms.errorid( "PBRO014" ) ;
				return 20 ;
			}
		}
		else
		{
			parms.errorid( "PBRO019" ) ;
			return 20 ;
		}
	}

	if ( t.f_hex )
	{
		if ( !ishex( t.f_string ) )
		{
			parms.errorid( "PBRO011K" ) ;
			return 20 ;
		}
		t.f_string = xs2cs( t.f_string ) ;
		t.f_asis   = true                ;
	}

	// For picture strings:

	// =  any character                   .  invalid characters (dot, x'2E')
	// @  alphabetic characters           -  non-numeric characters
	// #  numeric characters              <  lower case alphabetics
	// $  special characters              >  upper case alphabetics
	// ¬  non-blank characters (x'AC')

	if ( t.f_pic || t.f_mtch == 'P' || t.f_mtch == 'S' || t.f_mtch == 'W' )
	{
		t.f_regreq = true ;
		pic   = "" ;
		alnum = false ;
		for ( int i = 0 ; i < t.f_string.size() ; ++i )
		{
			c = t.f_string[ i ] ;
			if ( i == 0 || i == t.f_string.size() - 1 )
			{
				alnum = isalnum( c ) ;
			}
			if ( t.f_pic && pic_chars.find( c ) != string::npos )
			{
				switch ( c )
				{
					case '=':
						alnum = true ;
						pic += "." ;
						break ;

					case '@':
						alnum = true ;
						pic += "[[:alpha:]]" ;
						break ;

					case '#':
						alnum = true ;
						pic += "[[:digit:]]" ;
						break ;

					case '$':
						alnum = false ;
						pic += "[^[:blank:]^[:alpha:]^[:digit:]]" ;
						break ;

					case '¬':
						alnum = false ;
						pic += "[^[:blank:]]" ;
						break ;

					case '.':
						alnum = false ;
						pic += "[^[:print:]]" ;
						break ;

					case '-':
						alnum = true ;
						pic += "[^[:digit:]]" ;
						break ;

					case '<':
						alnum = true ;
						pic += "(?-i)[a-z](?i)" ;
						break ;

					case '>':
						alnum = true ;
						pic += "(?-i)[A-Z](?i)" ;
						break ;

				}
			}
			else if ( regex_chars.find( c ) != string::npos )
			{
				pic.push_back( '\\' ) ;
				pic.push_back( c ) ;
			}
			else
			{
				pic.push_back( c ) ;
			}
			if ( i == 0 )
			{
				alnum1 = alnum ;
			}
			if ( i == t.f_string.size() - 1 )
			{
				alnum2 = alnum ;
			}
		}
		t.f_string = pic ;
	}

	switch ( t.f_mtch )
	{
		case 'P':
			pre1 = ( alnum1 ) ? "\\b" : "(^|\\B)" ;
			t.f_string = pre1 + t.f_string + "\\w" ;
			break ;

		case 'S':
			suf1 = ( alnum2 ) ? "\\b" : "($|\\B)" ;
			t.f_string = "\\w" + t.f_string + suf1 ;
			break ;

		case 'W':
			pre1 = ( alnum1 ) ? "\\b" : "(^|\\B)" ;
			suf1 = ( alnum2 ) ? "\\b" : "($|\\B)" ;
			t.f_string = pre1 + t.f_string + suf1 ;
	}

	if ( t.f_regreq )
	{
		try
		{
			if ( t.f_asis )
			{
				t.f_regexp.assign( t.f_string ) ;
			}
			else
			{
				t.f_regexp.assign( t.f_string, boost::regex_constants::icase ) ;
			}
		}
		catch  ( boost::regex_error& e )
		{
			parms.errorid( "PBRO011N" ) ;
			return 20 ;
		}
	}

	t.f_fset = true ;

	find_parms = t  ;

	if ( t.f_dir == 'A' ) { t.f_dir = 'F' ; }
	Global_bfind_parms[ ds2d( zscrnum ) ] = t ;

	return 0 ;
}


void PBRO01A::performFind( int spos, int offset )
{
	int dl ;
	int c1 ;
	int c2 ;
	int oX ;
	int oY ;

	size_t p1 ;

	bool found  ;
	bool found1 ;
	bool skip   ;

	string::const_iterator itss ;
	string::const_iterator itse ;

	smatch results ;

	find_parms.f_occurs = 0 ;
	find_parms.f_line   = 0 ;
	find_parms.f_lines  = 0 ;
	find_parms.f_offset = 0 ;

	if ( find_parms.f_dir == 'F' || find_parms.f_dir == 'A' || find_parms.f_dir == 'L' )
	{
		offset = 0 ;
	}

	if ( offset > zareaw && colsOn )
	{
		offset = offset - zareaw ;
	}

	if ( offset > 0 )
	{
		oX = (offset % zareaw)+startCol-2 ;
		oY = offset / zareaw ;
	}
	else
	{
		oX = 0 ;
		oY = 0 ;
	}

	if ( oX == -1 )
	{
		oX = zareaw-1 ;
		--oY ;
	}

	if      ( find_parms.f_dir == 'F' ) { dl = 1             ; }
	else if ( find_parms.f_dir == 'A' ) { dl = 1             ; }
	else if ( find_parms.f_dir == 'L' ) { dl = data.size()-2 ; }
	else                                { dl = spos + oY     ; }

	if ( dl > data.size() - 2 )
	{
		dl = spos ;
	}

	if ( dl == 0 )
	{
		dl = 1 ;
	}

	while ( true )
	{
		skip = false ;
		c1   = 0 ;
		c2   = data.at( dl ).size() -1 ;

		if ( find_parms.f_scol > 0 )
		{
			c1 = find_parms.f_scol - 1 ;
		}
		if ( find_parms.f_ecol > 0 && c2 > find_parms.f_ecol-1 )
		{
			c2 = find_parms.f_ecol - 1 ;
		}

		if ( oX > 0 )
		{
			if ( oX > data.at( dl ).size()-1 )
			{
				oX = data.at( dl ).size()-1 ;
			}
			if ( find_parms.f_dir == 'P' || find_parms.f_dir == 'L' )
			{
				if ( oX < c1 )        { skip = true ; }
				else if ( oX <= c2 )  { c2   = oX-1 ; }
			}
			else
			{
				if ( oX > c2 )       { skip = true ; }
				else if ( oX >= c1 ) { c1   = oX+1 ; }
			}
		}
		oX = 0 ;
		if ( skip || c1 >= c2 )
		{
			if ( find_parms.f_dir == 'L' || find_parms.f_dir == 'P' ) { --dl ; }
			else { ++dl ; }
			if ( dl < 1 || dl > data.size()-2 ) { break ; }
			continue ;
		}

		if ( find_parms.f_regreq )
		{
			found1 = true  ;
			found  = false ;
			itss   = data.at( dl ).begin() + c1 ;
			itse   = itss + ( c2 - c1 + 1 ) ;
			if ( find_parms.f_oncol )
			{
				if ( regex_search( itss, itse, results, find_parms.f_regexp ) )
				{
					if ( itss == results[0].first )
					{
						found = true ;
						p1 = find_parms.f_scol-1 ;
					}
				}
			}
			else
			{
				if ( find_parms.f_dir == 'P' || find_parms.f_dir == 'L' || find_parms.f_dir == 'A' )
				{
					while ( regex_search( itss, itse, results, find_parms.f_regexp ) )
					{
						found = true ;
						for ( p1 = c1 ; itss != results[0].first ; ++itss ) { ++p1 ; }
						c1   = p1 + 1 ;
						itss = results[0].first ;
						++itss ;
						if ( found1 )
						{
							found1 = false ;
							++find_parms.f_lines ;
						}
						++find_parms.f_occurs ;
						if ( find_parms.f_dir == 'A' && find_parms.f_line == 0 )
						{
							find_parms.f_line = dl ;
							find_parms.f_offset = p1 ;
						}
					}
				}
				else
				{
					if ( regex_search( itss, itse, results, find_parms.f_regexp ) )
					{
						found = true ;
						for ( p1 = c1 ; itss != results[0].first ; ++itss ) { ++p1 ; }
					}
				}
			}
			if ( found && find_parms.f_line == 0 )
			{
				find_parms.f_line = dl ;
				find_parms.f_offset = p1 ;
			}
		}
		else
		{
			found1 = true ;
			while ( true )
			{
				found  = false ;
				if ( find_parms.f_dir == 'P' || find_parms.f_dir == 'L' )
				{
					if ( find_parms.f_asis )
					{
						p1 = data.at( dl ).rfind( find_parms.f_string, c2 ) ;
					}
					else
					{
						p1 = upper( data.at( dl ) ).rfind( find_parms.f_string, c2 ) ;
					}
					c2 = p1 - 1 ;
				}
				else
				{
					if ( find_parms.f_asis )
					{
						p1 = data.at( dl ).find( find_parms.f_string, c1 ) ;
					}
					else
					{
						p1 = upper( data.at( dl ) ).find( find_parms.f_string, c1 ) ;
					}
					c1 = p1 + 1 ;
				}
				if ( p1 != string::npos )
				{
					if ( find_parms.f_oncol )
					{
						if ( p1 == find_parms.f_scol-1 )
						{
							found = true ;
						}
					}
					else if ( find_parms.f_dir == 'P' || find_parms.f_dir == 'L' )
					{
						if ( p1 >= c1 )
						{
							found = true ;
						}
					}
					else
					{
						if ( p1 <= c2 )
						{
							found = true ;
						}
					}
				}
				if ( found && find_parms.f_line == 0 )
				{
					find_parms.f_line   = dl ;
					find_parms.f_offset = p1 ;
				}
				if ( find_parms.f_dir != 'A' || !found ) { break ; }
				++find_parms.f_occurs ;
				if ( found1 )
				{
					found1 = false ;
					++find_parms.f_lines ;
				}
			}
		}
		if      ( find_parms.f_dir == 'A' ) { ++dl ; }
		else if ( find_parms.f_dir == 'F' ) { if ( found ) { find_parms.f_dir = 'N' ; break ; } ; ++dl ; }
		else if ( find_parms.f_dir == 'L' ) { if ( found ) { find_parms.f_dir = 'P' ; break ; } ; --dl ; }
		else if ( find_parms.f_dir == 'N' ) { if ( found ) { break ; } ; ++dl ; }
		else if ( find_parms.f_dir == 'P' ) { if ( found ) { break ; } ; --dl ; }
		if ( dl < 1 || dl > data.size()-2 ) { break ; }
	}
}


bool PBRO01A::check_label( const string& label, a_parms& parms )
{
	if ( !isvalidName( label ) || label.size() > 7 )
	{
		vreplace( "ZVAL1", label ) ;
		parms.errorid( "PBRO011P" ) ;
		return false ;
	}
	return true ;
}


string PBRO01A::determineLang()
{
	// Try to determine the language, first from the extension, then the directory containing the source,
	// then the contents of the file.
	// Limit the scan to the first 100 lines of code

	// Returned language must exist in eHilight (hiRoutine function map) or an exception will occur.

	int i ;

	size_t p ;

	string s ;
	string t ;
	string w ;

	p = zfile.find_last_of( '.' ) ;
	if ( p != string::npos )
	{
		s = zfile.substr( p+1 ) ;
		if ( findword( s, "c cpp h hpp" ) ) { return "CPP"  ; }
		if ( findword( s, "rex rexx rx" ) ) { return "REXX" ; }
	}

	if ( zfile.find( "/rexx/" ) != string::npos ) { return "REXX"  ; }
	if ( zfile.find( "/tmp/"  ) != string::npos ) { return "OTHER" ; }

	for ( i = 1 ; i < data.size() && i < 100 ; ++i )
	{
		t = data.at( i ) ;
		if ( t.size() == 0 ) { continue       ; }
		if ( t[ 0 ] == '*' ) { return "ASM"   ; }
		if ( t[ 0 ] == ')' ) { return "PANEL" ; }
		w = word( t, 1 ) ;
		if ( findword( w, "TITLE CSECT DSECT MACRO START COPY" ) )
		{
			return "ASM"  ;
		}
		if ( i == 0 && t.find( "rexx" ) != string::npos )
		{
			return "REXX" ;
		}
		if ( w == "/*"    )  { return "CPP" ; }
	}
	return "DEFAULT" ;
}


// ============================================================================================ //

extern "C" { pApplication* maker() { return new PBRO01A ; } }
extern "C" { void destroy(pApplication* p) { delete p ; } }
