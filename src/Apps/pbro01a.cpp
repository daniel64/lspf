/*  Compile with ::                                                                            */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libpbro01a.so -o libpbro01a.so -lmagic pbro01a.cpp */

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
/*                                                                                        */
/* PDF-like browser program                                                               */
/*                                                                                        */
/*  ZRC/ZRSN exit codes (RC=ZRC in the calling program)                                   */
/*    0/0     Normal completion                                                           */
/*   12/0     File empty                                                                  */
/*   14/0     File does not exist                                                         */
/*   20/0     Severe error.                                                               */
/*   20/4     Cannot open file.  Permission denied.                                       */
/*   20/8     Cannot open file.  Invalid file type.                                       */
/*   20/12    Cannot open file.  Unknown open error.                                      */
/*                                                                                        */
/* ZRESULT contains the relevant message id for RC > 0                                    */
/*                                                                                        */
/* Commands:                                                                              */
/*  ANSI ON | OFF                                                                         */
/*      Use ANSI colour code sequences to set text colour.                                */
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
/*      Open file as text (0A and 0D0A will be interpreted as newline)                    */
/*  .label                                                                                */
/*      Set label (must be alphnumeric, max 8 chars, case insensitive)                    */
/*  &cmd                                                                                  */
/*      Keeps command on the command line (including on SPLIT/SWAP)                       */
/*                                                                                        */
/******************************************************************************************/

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>

#include <vector>

#include "../lspfall.h"
#include <magic.h>
#include "ehilight.cpp"
#include "pbro01a.h"

#define CLRTABLE   "EDITCLRS"

using namespace boost ;
using namespace std   ;
using namespace boost::filesystem ;

map<int,b_find> pbro01a::Global_bfind_parms ;

LSPF_APP_MAKER( pbro01a )

pbro01a::pbro01a()
{
	STANDARD_HEADER( "Browser for lspf", "1.0.4" )

	vdefine( "ZCMD ZVERB ZROW1 ZCURFLD", &zcmd, &zverb, &zrow1, &zcurfld ) ;
	vdefine( "ZAREA ZSHADOW ZDSN", &zarea, &zshadow, &zfile ) ;
	vdefine( "ZSCROLLN ZAREAW ZSCREEND ZSCREENW", &zscrolln, &zareaw, &zscreend, &zscreenw ) ;
	vdefine( "ZAREAD ZLVLINE ZCURPOS", &zaread, &zlvline, &zcurpos ) ;
	vdefine( "ZSCROLLA ZCOL1 ZCOL2 TYPE STR", &zscrolla, &zcol1, &zcol2, &type, &str ) ;
	vdefine( "OCC LINES CMD ZZSTR1 FSCROLL", &occ, &lines, &cmd, &zzstr1, &fscroll ) ;

	vdefine( "ZEDCLRD ZEDCLRC ZEDCLRK ZEDCLRQ ZEDCLRV ZEDCLRS", &zedclrd, &zedclrc, &zedclrk, &zedclrq, &zedclrv, &zedclrs ) ;
	vdefine( "ZEDHID ZEDHIC ZEDHIK ZEDHIQ ZEDHIV ZEDHIS", &zedhid, &zedhic, &zedhik, &zedhiq, &zedhiv, &zedhis ) ;

	langSpecials[ "ALL"     ] = "" ;
	langSpecials[ "ASM"     ] = "+-*/=<>&|:,#" ;
	langSpecials[ "BASH"    ] = "=<>+-*[]/&|:,;%\\" ;
	langSpecials[ "COBOL"   ] = "." ;
	langSpecials[ "CPP"     ] = "+-*/=<>&,|:!;%#[]\\" ;
	langSpecials[ "DEFAULT" ] = "" ;
	langSpecials[ "JCL"     ] = ",|<>&=" ;
	langSpecials[ "OTHER"   ] = "+-*/=<>&|:" ;
	langSpecials[ "PANEL"   ] = "=,&.<>!*|" ;
	langSpecials[ "REXX"    ] = "=<>+-*[]/&|:,;%\\" ;
	langSpecials[ "RUST"    ] = "+-*/%^!&|<>=@_.;:,#$?~[]" ;
	langSpecials[ "SKEL"    ] = "&?!<|>" ;
	langSpecials[ "TOML"    ] = ".=" ;

	XRC  = 0 ;
	XRSN = 0 ;
}


void pbro01a::application()
{
	string listid ;

	browse_parms* b_parms = static_cast<browse_parms*>( get_options() ) ;

	if ( !b_parms ) { return ; }

	zfile  = b_parms->browse_file ;
	panel  = b_parms->browse_panel ;
	reclen = b_parms->browse_reclen ;

	if ( zfile == "" )
	{
		uabend( "PBRO011Y" ) ;
	}

	try
	{
		if ( is_directory( zfile ) )
		{
			vdefine( "LID", &listid ) ;
			lmdinit( "LID", zfile ) ;
			lmddisp( listid ) ;
			lmdfree( listid ) ;
			vdelete( "LID" ) ;
			return ;
		}
	}
	catch (...)
	{
		uabend( "PBRO015" ) ;
	}

	initialise() ;

	Browse() ;

	ZRC  = XRC ;
	ZRSN = XRSN ;
}


void pbro01a::initialise()
{
	vcopy( "ZSCRNUM", zscrnum, MOVE ) ;

	vget( "ZSCREEND ZSCREENW", SHARED ) ;

	if ( panel == "" ) { panel = "PBRO01A2" ; }

	pquery( panel, "ZAREA", "", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 )
	{
		uabend() ;
	}

	zasize = zareaw * zaread ;

	s1b = string( zareaw, N_BLUE   ) ;
	s1g = string( zareaw, N_GREEN  ) ;
	s1y = string( zareaw, N_YELLOW ) ;
	s1w = string( zareaw, N_WHITE  ) ;
	div = string( zareaw, '-' ) ;

	optNoConvTabs = ( get_shared_var( "ZEDTABSS" ) == "YES" ) ;
}


void pbro01a::Browse()
{
	int aRow ;
	int aCol ;

	int i ;

	string zxcmd ;

	a_parms action_parms ;

	rebuildZAREA = true  ;
	hexOn        = false ;
	vertOn       = true  ;
	colsOn       = false ;
	asBin        = false ;
	binOn        = false ;
	textOn       = false ;
	hilightOn    = true  ;
	hiComplete   = false ;
	ansi_on      = true  ;
	entLang      = ""    ;
	msg          = ""    ;
	zxcmd        = ""    ;
	startCol     = 1     ;
	maxCol       = 1     ;
	hltdl        = 0     ;
	bhltStatus   = BHLT_STOPPED ;

	read_file() ;
	if ( XRC > 0 ) { return ; }

	boost::thread* bThread = new boost::thread( &pbro01a::hilightData, this ) ;
	while ( bhltStatus != BHLT_RUNNING )
	{
		boost::this_thread::sleep_for( boost::chrono::milliseconds( 1 ) ) ;
	}

	boost::this_thread::sleep_for( boost::chrono::milliseconds( 2 ) ) ;
	cond_hlt.notify_all() ;

	auto it = Global_bfind_parms.find( ds2d( zscrnum ) ) ;
	if ( it != Global_bfind_parms.end() )
	{
		find_parms = it->second ;
	}

	vget( "ZBRALT", SHARED ) ;
	if ( RC == 8 )
	{
		vreplace( "ZBRALT", zfile ) ;
	}
	else
	{
		verase( "ZBRALT", SHARED ) ;
	}

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

		cursor.placecursor( zlvline, zareaw, topLine, startCol, hexOn, colsOn ) ;
		curfld = cursor.curfld() ;
		curpos = cursor.curpos() ;

		if ( curfld == "ZAREA" )
		{
			hilite_cursor() ;
		}

		display( panel, msg, curfld, curpos ) ;
		if ( RC == 8 ) { break ; }

		if ( is_term_resized( zscreend+2, zscreenw ) )
		{
			term_resize() ;
			rebuildZAREA = true ;
			continue ;
		}

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

	if ( bhltStatus == BHLT_RUNNING )
	{
		bhltStatus = BHLT_STOPPING ;
		cond_hlt.notify_all() ;
	}

	i = 0 ;
	while ( bhltStatus != BHLT_STOPPED && ++i < 200 )
	{
		boost::this_thread::sleep_for( boost::chrono::milliseconds( 5 ) ) ;
	}

	delete bThread ;

	vput( "ZSCROLL", PROFILE ) ;
}


void pbro01a::read_file()
{
	string inLine ;

	b_shadow t ;

	int rc ;
	int i  ;
	int j  ;

	bool cr = true ;

	size_t p1 ;
	size_t p2 ;
	size_t maxl = ( reclen == 0 ) ? zareaw : reclen ;

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
		vreplace( "ZVAL1", strerror( errno ) ) ;
		vreplace( "ZVAL2", zfile ) ;
		vput( "ZVAL1 ZVAL2", SHARED ) ;
		XRC  = 20 ;
		XRSN = 12 ;
		ZRESULT = "PSYS011U" ;
		return    ;
	}

	fileType = "text/plain" ;
	magic_t cookie = magic_open( MAGIC_CONTINUE | MAGIC_ERROR | MAGIC_MIME | MAGIC_SYMLINK ) ;
	rc = magic_load( cookie, nullptr ) ;

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

	detLang = "NONE" ;

	if ( asBin || reclen > 0 )
	{
		inLine = string( maxl, ' ' ) ;
		maxCol = maxl ;
		i      = 0    ;
		while ( true )
		{
			fin.get( x ) ;
			if ( fin.fail() != 0 ) { break ; } ;
			inLine[ i ] = x ;
			++i ;
			if ( ( reclen > 0 && i == reclen ) || ( reclen == 0 && i == zareaw ) )
			{
				data.push_back( inLine )     ;
				inLine = string( maxl, ' ' ) ;
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
			if ( !optNoConvTabs )
			{
				p1 = inLine.find( '\t' ) ;
				while ( p1 != string::npos )
				{
					j = 8 - ( p1 % 8 ) ;
					inLine.replace( p1, 1, j, ' ' ) ;
					p1 = inLine.find( '\t', p1 + 1 ) ;
				}
			}
			if ( cr && ( inLine.size() == 0 || inLine.back() != 0x0D ) )
			{
				cr = false ;
			}
			if ( maxCol < inLine.size() ) { maxCol = inLine.size() ; }
			data.push_back( inLine ) ;
			shadow.push_back( t ) ;
			++maxLines ;
		}
		if ( cr )
		{
			for ( auto it = data.begin() + 1 ; it != data.end() ; ++it )
			{
				it->pop_back() ;
			}
		}
		if ( entLang == "" || entLang == "AUTO" )
		{
			detLang = determineLang() ;
		}
		else
		{
			detLang = entLang ;
		}
		load_language_colours() ;
		set_language_fvars( detLang ) ;
		hilightOn = true ;
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

	fscroll = ( zfile.size() < ( zareaw - 41 ) ) ? "NO" : "YES" ;

	if ( detLang == "ANSI" )
	{
		for ( int i = 0 ; i < data.size() ; ++i )
		{
			shadow[ i ].bs_line = data[ i ] ;
			string& t = data[ i ] ;
			p1 = t.find( ansi_start ) ;
			while ( p1 != string::npos )
			{
				p2 = t.find_first_not_of( ansi_codes, p1+2 ) ;
				if ( p2 == string::npos ) { break ; }
				if ( t[ p2 ] != ansi_end )
				{
					p1 = t.find( ansi_start, p2 ) ;
					continue ;
				}
				t.erase( p1, p2-p1+1 ) ;
				p1 = t.find( ansi_start, p1 ) ;
			}
		}
	}
}


void pbro01a::fill_dynamic_area()
{
	//
	// Fill dynamic area from the data vector.
	//

	string t1 ;
	string t3 ;
	string t4 ;

	int ln ;

	if ( colsOn )
	{
		zarea   = getColumnLine() ;
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
			if ( k > 0 && k < data.size()-1 )
			{
				ln = data.at( k ).size() - startCol + 1 ;
				if ( ln > zareaw ) { ln = zareaw ; }
				t1 = substr( data.at( k ), startCol, zareaw ) ;
				if ( vertOn )
				{
					t3 = cs2xs1( t1, 0, ln ) ;
					t4 = cs2xs2( t1, 0, ln ) ;
					t3.resize( zareaw, ' ' ) ;
					t4.resize( zareaw, ' ' ) ;
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
	zshadow.resize( zasize, N_BLUE ) ;

	if ( hilightOn && !hlight.hl_abend )
	{
		fill_zshadow() ;
	}
	cshadow = zshadow ;
}


void pbro01a::fill_zshadow()
{
	//
	// Copy the relevant parts of the shadow data to the zshadow variable.
	//
	// If the background hilight task is still busy, wait until it has completed the
	// current screen of data.
	//

	int i  ;
	int w  ;

	while ( bhltStatus == BHLT_RUNNING && hltdl < ( topLine + zaread ) )
	{
		boost::this_thread::sleep_for( boost::chrono::milliseconds( 5 ) ) ;
	}

	vector<b_shadow>::iterator its ;
	vector<string>::iterator itd ;

	string::const_iterator it1 ;
	string::const_iterator it2 ;

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


void pbro01a::hilightData()
{
	//
	// Background task to build shadow data.
	//

	boost::mutex mutex ;

	string::const_iterator it1 ;
	string::const_iterator it2 ;

	bhltStatus = BHLT_RUNNING ;

	while ( bhltStatus == BHLT_RUNNING )
	{
		boost::mutex::scoped_lock lk( mutex ) ;
		cond_hlt.wait_for( lk, boost::chrono::milliseconds( 200 ) ) ;
		lk.unlock() ;
		if ( detLang == "NONE" || hiComplete ) { continue ; }
		hlight.hl_language = detLang ;
		hlight.hl_oBrac1   = 0 ;
		hlight.hl_oBrac2   = 0 ;
		hlight.hl_oIf      = 0 ;
		hlight.hl_oDo      = 0 ;
		hlight.hl_ifLogic  = true ;
		hlight.hl_doLogic  = true ;
		hlight.hl_Paren    = true ;
		hlight.hl_oComment = false ;

		for ( hltdl = 1 ; hltdl < data.size() - 1 && bhltStatus == BHLT_RUNNING ; ++hltdl )
		{
			auto& refs = shadow.at( hltdl ) ;
			auto& refd = ( detLang == "ANSI" ) ? refs.bs_line : data.at( hltdl ) ;
			addHilight( lg, hlight, refd, refs.bs_Shadow ) ;
			if ( hlight.hl_abend )
			{
				bhltStatus = BHLT_STOPPED ;
				return ;
			}
			refs.bs_vShadow = true ;
			refs.bs_wShadow = ( hlight.hl_oBrac1 == 0 &&
					    hlight.hl_oBrac2 == 0 &&
					    hlight.hl_oIf    == 0 &&
					    hlight.hl_oDo    == 0 &&
					   !hlight.hl_continue    &&
					   !hlight.hl_oQuote      &&
					   !hlight.hl_oComment ) ;
		}
		hiComplete = true ;
		hltdl     += zaread ;
	}
	bhltStatus = BHLT_STOPPED ;
}


void pbro01a::hilite_cursor()
{
	int i ;

	for ( i = curpos-1 ; i < zasize ; ++i )
	{
		if ( zarea[ i ] == ' ' || ( i / zareaw ) > ( data.size() - topLine - 2 ) ) { break ; }
		zshadow[ i ] = N_WHITE ;
	}
	for ( i = curpos-1 ; i >= 0 ; --i )
	{
		if ( zarea[ i ] == ' ' || ( topLine == 0 && ( i / zareaw ) == 0 ) ) { break ; }
		zshadow[ i ] = N_WHITE ;
	}
}


void pbro01a::set_label( a_parms& parms )
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


void pbro01a::action_ScrollUp()
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


void pbro01a::action_ScrollDown()
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
			if ( t >= zlvline ) { break ; }
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


void pbro01a::action_ScrollLeft()
{
	if ( zscrolla == "MAX" )
	{
		startCol = 1 ;
	}
	else
	{
		startCol = startCol - zscrolln ;
		if ( startCol < 1 ) { startCol = 1 ; }
	}
	rebuildZAREA = true ;
}


void pbro01a::action_ScrollRight()
{
	if ( zscrolla == "MAX" )
	{
		startCol = maxCol - zareaw ;
	}
	else
	{
		startCol = startCol + zscrolln ;
	}

	if ( startCol < 1 )
	{
		startCol = 1 ;
	}
	else if ( reclen > 0 && ( startCol + zareaw ) > reclen )
	{
		startCol = ( reclen <= zareaw ) ? 1 : ( reclen - zareaw + 1 ) ;
	}

	rebuildZAREA = true ;
}


void pbro01a::action_ANSI( a_parms& parms )
{
	int tTop = topLine ;

	if ( parms.w2 != "" && parms.w2 != "ON" && parms.w2 != "OFF" )
	{
		parms.errorid( "PBRO012" ) ;
		return ;
	}

	if ( parms.w2 == "" )
	{
		ansi_on = !ansi_on ;
	}
	else
	{
		ansi_on = ( parms.w2 == "ON" ) ;
	}

	rebuildZAREA = true ;
	read_file() ;
	topLine = tTop ;
}


void pbro01a::action_BINARY( a_parms& parms )
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


void pbro01a::action_BROWSE( a_parms& parms )
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


void pbro01a::action_EDIT( a_parms& parms )
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


void pbro01a::action_HEX( a_parms& parms )
{
	if ( parms.wds > 3 )
	{
		parms.errorid( "PBRO013" ) ;
		return ;
	}

	if ( parms.w2 == "" ||
	   ( parms.w2 == "ON"   && ( parms.w3 == "" || parms.w3 == "VERT" ) ) ||
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


void pbro01a::action_HILIGHT( a_parms& parms )
{
	if ( parms.w2 != "" && parms.w2 != "ON" && parms.w2 != "OFF" && parms.w3 == "" )
	{
		parms.w3 = parms.w2 ;
		parms.w2 = "ON" ;
	}

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
	load_language_colours() ;
	set_language_fvars( detLang ) ;

	if ( parms.w2 == "ON" )
	{
		shadow[ 1 ].bs_wShadow = false ;
		shadow[ 1 ].bs_vShadow = false ;
		hiComplete   = false ;
		hilightOn    = true ;
		rebuildZAREA = true ;
		hltdl        = 0 ;
		cond_hlt.notify_all() ;
	}
	else if ( parms.w2 == "OFF" )
	{
		shadow[ 1 ].bs_wShadow = false ;
		shadow[ 1 ].bs_vShadow = false ;
		hiComplete   = false ;
		hilightOn    = false ;
		rebuildZAREA = true ;
		hltdl        = 0 ;
		cond_hlt.notify_all() ;
	}
}


void pbro01a::action_COLS( a_parms& parms )
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


void pbro01a::action_FIND( a_parms& parms )
{
	if ( setFind( parms ) > 0 ) { return ; }

	performFind() ;
	if ( find_parms.f_line > 0 )
	{
		zaline = find_parms.f_line - topLine ;
		if ( hexOn ) { zaline = 4*zaline ; }
		if ( zaline < 0 || zaline > zlvline-1 )
		{
			topLine = find_parms.f_line - 1 ;
		}
		if ( find_parms.f_offset < startCol-1 || find_parms.f_offset >= zareaw + startCol - 2 )
		{
			startCol = find_parms.f_offset - 13 ;
			if ( startCol < 1 )
			{
				startCol = 1 ;
			}
		}
		cursor.set( "ZAREA", find_parms.f_line, find_parms.f_offset ) ;
		type  = typeList[ find_parms.f_mtch ] ;
		str   = setFoundString() ;
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
		type = typeList[ find_parms.f_mtch ] ;
		str  = setFoundString() ;
		setNotFoundMsg( parms ) ;
		return ;
	}
	rebuildZAREA = true ;
}


void pbro01a::action_RFIND()
{
	a_parms parms ;

	if ( !find_parms.f_fset )
	{
		msg = "PBRO011A" ;
		return ;
	}

	performFind() ;
	if ( find_parms.f_line > 0 )
	{
		zaline = find_parms.f_line - topLine ;
		if ( hexOn ) { zaline = 4*zaline ; }
		if ( zaline < 0 || zaline > zlvline-1 )
		{
			topLine = find_parms.f_line - 1 ;
		}
		if ( find_parms.f_offset < startCol-1 || find_parms.f_offset >= zareaw + startCol - 2 )
		{
			startCol = find_parms.f_offset - 13 ;
			if ( startCol < 1 )
			{
				startCol = 1 ;
			}
		}
		cursor.set( "ZAREA", find_parms.f_line, find_parms.f_offset ) ;
		type = typeList[ find_parms.f_mtch ] ;
		str  = setFoundString() ;
		msg  = "PBRO011F" ;
	}
	else
	{
		setNotFoundMsg( parms ) ;
		msg = parms.msg ;
		return ;
	}
	rebuildZAREA = true ;
}


void pbro01a::action_LOCATE( a_parms& parms )
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


void pbro01a::action_RESET( a_parms& parms )
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


void pbro01a::action_TEXT( a_parms& parms )
{
	if ( parms.w2 != "" )
	{
		parms.errorid( "PBRO013" ) ;
		return ;
	}

	asBin        = false ;
	binOn        = false ;
	textOn       = true ;
	rebuildZAREA = true ;
	read_file() ;
}


int pbro01a::setFind( a_parms& parms )
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
	string suf1 ;
	string rcmd ;
	string zxfind ;

	size_t p1 ;
	size_t p2 ;

	const char quote('\"') ;
	const char apost('\'') ;

	const string pic_chars   = "=@#$¬.-<>" ;
	const string str_quote   = string( 1, quote ) ;
	const string str_apost   = string( 1, apost ) ;
	const string regex_chars = "\\^$.|?*+()[{" ;

	b_find t ;

	parms.msg = "" ;
	com    = " " + subword( parms.zcmd, 2 ) + " " ;
	zxfind = parms.zcmd ;

	p1 = com.find( quote ) ;
	p2 = com.find( apost ) ;

	delim = ( p1 == string::npos ) ? str_apost :
		( p2 == string::npos ) ? str_quote :
		( p1 < p2 )            ? str_quote : str_apost ;

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
		t.f_ostring = substr( com, (p1+1), (p2-p1-1) ) ;
		t.f_string  = t.f_ostring ;
		if ( t.f_text )
		{
			iupper( t.f_string ) ;
		}
		com = delstr( com, (p1-1), (p2-p1+3) ) ;
	}

	if ( t.f_rreg ) { t.f_regreq = true ; }

	ucmd = upper( com ) ;

	if ( t.f_ostring == "" && words( ucmd ) == 1 )
	{
		istrip( ucmd ) ;
		t.f_ostring = ucmd ;
		t.f_string  = ucmd ;
		t.f_text    = true ;
		ucmd        = "" ;
	}

	if ( parseString2( ucmd, "NEXT" ) )
	{
		t.f_dir = 'N' ;
	}
	else if ( parseString2( ucmd, "PREV" ) )
	{
		t.f_dir = 'P' ;
	}
	else if ( parseString2( ucmd, "FIRST" ) )
	{
		t.f_dir = 'F' ;
	}
	else if ( parseString2( ucmd, "LAST" ) )
	{
		t.f_dir = 'L' ;
	}
	else if ( parseString2( ucmd, "ALL" ) )
	{
		t.f_dir = 'A' ;
	}

	if ( !t.f_rreg )
	{
		if ( parseString2( ucmd, "CHARS" ) )
		{
			t.f_mtch  = 'C'  ;
		}
		else if ( parseString2( ucmd, "PRE PREFIX" ) )
		{
			t.f_mtch   = 'P'  ;
			t.f_regreq = true ;
		}
		else if ( parseString2( ucmd, "SUF SUFFIX" ) )
		{
			t.f_mtch   = 'S'  ;
			t.f_regreq = true ;
		}
		else if ( parseString2( ucmd, "WORD" ) )
		{
			t.f_mtch   = 'W'  ;
			t.f_regreq = true ;
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
				w1 = find_parms.f_ostring ;
			}
			else
			{
				if ( t.f_string != "" )
				{
					str = w1 ;
					parms.errorid( "PBRO018" ) ;
					return 20 ;
				}
			}
			t.f_text    = true ;
			t.f_ostring = w1 ;
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
		t.f_ostring = t.f_string ;
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
			if ( !t.f_rreg && t.f_ostring.size() > ( t.f_ecol - t.f_scol + 1 ) )
			{
				str = t.f_ostring ;
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
		t.f_asis   = true ;
	}

	//
	// For picture strings:
	//
	// =  any character                   .  invalid characters (dot, x'2E')
	// @  alphabetic characters           -  non-numeric characters
	// #  numeric characters              <  lower case alphabetics
	// $  special characters              >  upper case alphabetics
	// ¬  non-blank characters (x'AC')
	//

	if ( t.f_pic || t.f_mtch == 'P' || t.f_mtch == 'S' || t.f_mtch == 'W' )
	{
		t.f_regreq = true ;
		pic   = "" ;
		alnum = false ;
		for ( size_t i = 0 ; i < t.f_string.size() ; ++i )
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
		catch ( boost::regex_error& e )
		{
			parms.errorid( "PBRO011N" ) ;
			return 20 ;
		}
	}

	t.f_fset = true ;

	find_parms = t ;

	if ( t.f_dir == 'A' ) { t.f_dir = 'F' ; }
	Global_bfind_parms[ ds2d( zscrnum ) ] = t ;

	return 0 ;
}


void pbro01a::performFind()
{
	int i  ;
	int dl ;
	int c1 ;
	int c2 ;
	int oX = -1 ;
	int oY = -1 ;

	size_t p1 ;

	bool skip ;
	bool found ;
	bool found1 ;

	string::const_iterator itss ;
	string::const_iterator itse ;

	smatch results ;

	find_parms.f_rstring = "" ;

	if ( find_parms.f_dir != 'F' && find_parms.f_dir != 'A' && find_parms.f_dir != 'L' )
	{
		if ( zcurfld == "ZAREA" )
		{
			i = zcurpos ;
			if ( i > zareaw && colsOn )
			{
				i -= zareaw ;
			}
			if ( i > 0 )
			{
				oX = ( i % zareaw ) + startCol - 2 ;
				oY = i / zareaw ;
			}
		}
	}

	if ( find_parms.f_line == 0 && ( find_parms.f_dir == 'N' || find_parms.f_dir == 'P' ) )
	{
		if ( find_parms.f_pRow     == ( ( oY > 0 ) ? ( oY + topLine  ) : topLine  ) &&
		     find_parms.f_pCol     == ( ( oX > 0 ) ? ( oX + startCol ) : startCol ) &&
		     find_parms.f_ptopLine == topLine )
		{
			find_parms.f_dir = ( find_parms.f_dir == 'N' ) ? 'F' : 'L' ;
		}
	}

	find_parms.f_pRow     = ( oY > 0 ) ? oY + topLine  : topLine  ;
	find_parms.f_pCol     = ( oX > 0 ) ? oX + startCol : startCol ;
	find_parms.f_ptopLine = topLine ;

	find_parms.f_occurs = 0 ;
	find_parms.f_line   = 0 ;
	find_parms.f_lines  = 0 ;
	find_parms.f_offset = 0 ;

	if ( find_parms.f_dir == 'F' || find_parms.f_dir == 'A' )
	{
		dl = 1 ;
		oX = -1 ;
	}
	else if ( ( find_parms.f_dir == 'L' ) ||
		  ( find_parms.f_dir == 'P' && topLine == 0 && ( oY == 0 || oY == -1 ) ) )
	{
		dl = data.size() - 2 ;
	}
	else if ( find_parms.f_dir == 'P' && oX == 0 )
	{
		if ( hexOn )
		{
			dl = ( topLine == 0 ) ? ( oY - 1 ) / 4 : topLine + ( oY / 4 ) - 1 ;
		}
		else
		{
			dl = topLine + oY - 1 ;
		}
	}
	else if ( oY > 0 )
	{
		if ( hexOn )
		{
			dl = ( topLine == 0 ) ? ( ( oY - 1 ) / 4 ) + 1 : topLine + ( oY / 4 ) ;
		}
		else
		{
			dl = topLine + oY ;
		}
	}
	else
	{
		dl = topLine ;
	}

	if ( dl > data.size() - 2 )
	{
		dl = topLine ;
	}
	else if ( dl < 1 )
	{
		dl = 1 ;
	}

	find_parms.f_top    = ( dl == 1               ) ;
	find_parms.f_bottom = ( dl == data.size() - 2 ) ;

	while ( true )
	{
		skip = false ;
		c1   = 0 ;
		c2   = data.at( dl ).size() - 1 ;

		if ( find_parms.f_scol > 0 )
		{
			c1 = find_parms.f_scol - 1 ;
		}
		if ( find_parms.f_oncol )
		{
			c1 = find_parms.f_scol - 1 ;
			if ( c1 > c2 ) { skip = true ; }
			else           { c2   = c1   ; }
		}
		else if ( find_parms.f_ecol > 0 && c2 > find_parms.f_ecol-1 )
		{
			c2 = find_parms.f_ecol - 1 ;
		}

		if ( oX > 0 )
		{
			if ( oX > data.at( dl ).size() )
			{
				oX = data.at( dl ).size() ;
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
		else if ( oX == 0 )
		{
			if ( c1 == 0 && find_parms.f_dir != 'P' && find_parms.f_dir != 'L' )
			{
				c1 = 1 ;
			}
		}
		if ( find_parms.f_oncol )
		{
			if ( c1 > find_parms.f_scol - 1 ) { skip = true ; }
		}
		oX = -1 ;
		if ( skip || c1 > c2 )
		{
			if ( find_parms.f_dir == 'L' || find_parms.f_dir == 'P' )
			{
				--dl ;
			}
			else
			{
				++dl ;
			}
			if ( dl < 1 || dl > data.size()-2 ) { break ; }
			continue ;
		}

		if ( find_parms.f_regreq )
		{
			found  = false ;
			found1 = true  ;
			if ( find_parms.f_mtch == 'S' && c1 > 0 ) { --c1 ; }
			itss   = data.at( dl ).begin() + c1 ;
			if ( find_parms.f_oncol )
			{
				itse = data.at( dl ).end() ;
				if ( regex_search( itss, itse, results, find_parms.f_regexp ) )
				{
					if ( itss == results[ 0 ].first )
					{
						p1    = find_parms.f_scol - 1 ;
						found = true ;
						++find_parms.f_lines  ;
						++find_parms.f_occurs ;
						find_parms.set_rstring( results[ 0 ] ) ;
					}
				}
			}
			else
			{
				itse = itss + ( c2 - c1 + 1 ) ;
				if ( find_parms.f_dir == 'P' || find_parms.f_dir == 'L' || find_parms.f_dir == 'A' )
				{
					while ( regex_search( itss, itse, results, find_parms.f_regexp ) )
					{
						found = true ;
						for ( p1 = c1 ; itss != results[ 0 ].first ; ++itss ) { ++p1 ; }
						c1   = p1 + 1 ;
						itss = results[0].first ;
						find_parms.set_rstring( results[ 0 ] ) ;
						++itss ;
						if ( found1 )
						{
							found1 = false ;
							++find_parms.f_lines ;
						}
						++find_parms.f_occurs ;
						if ( find_parms.f_dir == 'A' && find_parms.f_line == 0 )
						{
							if ( find_parms.f_mtch == 'S' ) { ++p1 ; }
							find_parms.f_line   = dl ;
							find_parms.f_offset = p1 ;
						}
					}
				}
				else
				{
					if ( regex_search( itss, itse, results, find_parms.f_regexp ) )
					{
						found = true ;
						for ( p1 = c1 ; itss != results[ 0 ].first ; ++itss ) { ++p1 ; }
						find_parms.set_rstring( results[ 0 ] ) ;
					}
				}
			}
			if ( found && find_parms.f_mtch == 'S' ) { ++p1 ; }
			if ( found && find_parms.f_line == 0 )
			{
				find_parms.f_line   = dl ;
				find_parms.f_offset = p1 ;
			}
		}
		else
		{
			found1 = true ;
			while ( true )
			{
				found = false ;
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
				if ( find_parms.f_dir != 'A' || !found )
				{
					break ;
				}
				++find_parms.f_occurs ;
				if ( found1 )
				{
					found1 = false ;
					++find_parms.f_lines ;
				}
			}
		}
		if ( find_parms.f_dir == 'A' )
		{
			++dl ;
		}
		else if ( find_parms.f_dir == 'F' || find_parms.f_dir == 'N' )
		{
			find_parms.f_dir = 'N' ;
			if ( found ) { break ; }
			++dl ;
		}
		else
		{
			find_parms.f_dir = 'P' ;
			if ( found ) { break ; }
			--dl ;
		}
		if ( dl < 1 || dl > data.size() - 2 )
		{
			break ;
		}
	}
}


void pbro01a::setNotFoundMsg( a_parms& parms )
{
	//
	// dir = N and not from top    - Bottom of Data reached.
	// dir = P and not from bottom - Top of Data reached.
	// dir = F/A/L/N-top/P-bottom  - Not found.
	//

	str  = setFoundString() ;
	type = typeList[ find_parms.f_mtch ] ;
	if ( find_parms.f_dir == 'P' || find_parms.f_dir == 'L' )
	{
		parms.errorid( find_parms.f_bottom ? "PBRO011E" : "PBRO017", 4 ) ;
	}
	else
	{
		parms.errorid( find_parms.f_top ? "PBRO011E" : "PBRO016", 4 ) ;
	}
}


string pbro01a::setFoundString()
{
	//
	// Called to set the found/notfound string in find messages.
	//

	int sz ;

	string s ;

	if ( find_parms.f_rstring == "" )
	{
		if ( find_parms.f_hex )
		{
			return "X'" + find_parms.f_ostring + "'" ;
		}
		s = find_parms.f_ostring ;
	}
	else
	{
		if ( find_parms.f_mtch == 'P' )
		{
			sz = ( find_parms.f_hex ) ? find_parms.f_ostring.size() / 2 : find_parms.f_ostring.size() ;
			s  = find_parms.f_rstring.substr( 0, sz ) ;
		}
		else if ( find_parms.f_mtch == 'S' )
		{
			s = find_parms.f_rstring.substr( 1, find_parms.f_ostring.size() ) ;
		}
		else
		{
			s = find_parms.f_rstring ;
		}
		if ( find_parms.f_hex )
		{
			return "X'" + cs2xs( s ) + "'" ;
		}
	}

	if ( any_of( s.begin(), s.end(),
		[]( char c )
		{
			return ( !isprint( c ) ) ;
		} ) )
	{
		s = "X'" + cs2xs( s ) + "'" ;
	}
	else
	{
		s = "'" + s + "'" ;
	}

	return s ;
}


string pbro01a::getColumnLine()
{
	//
	// Create column line.
	//

	int i ;
	int j ;
	int k ;
	int l ;
	int s ;

	string t = "" ;

	const string ruler = "----+----" ;

	s = startCol ;
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

	if ( reclen > 0 && ( reclen - startCol ) < zareaw )
	{
		l = reclen - startCol + 1 ;
		t = t.substr( 0, l ) ;
		t.resize( zareaw, ' ' ) ;
	}
	else
	{
		t = t.substr( 0, l ) ;
	}

	return t ;
}


string pbro01a::get_shared_var( const string& var )
{
	//
	// Return the dialogue variable from the SHARED pool.
	//

	string val ;

	vget( var, SHARED ) ;
	vcopy( var, val, MOVE ) ;

	return val ;
}


string pbro01a::get_profile_var( const string& var )
{
	//
	// Return the dialogue variable from the PROFILE pool.
	//

	string val ;

	vget( var, PROFILE ) ;
	vcopy( var, val, MOVE ) ;

	return val ;
}


bool pbro01a::check_label( const string& label,
			   a_parms& parms )
{
	if ( !isvalidName( label ) || label.size() > 7 )
	{
		vreplace( "ZVAL1", label ) ;
		parms.errorid( "PBRO011P" ) ;
		return false ;
	}
	return true ;
}


void pbro01a::term_resize()
{
	//
	// Get/set new size of variables after a terminal resize.
	//

	const string vlist1 = "ZSCREEND ZSCREENW" ;

	vget( vlist1, SHARED ) ;

	pquery( panel, "ZAREA", "", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 )
	{
		uabend() ;
	}

	zasize = zareaw * zaread ;

	s1b = string( zareaw, N_BLUE   ) ;
	s1g = string( zareaw, N_GREEN  ) ;
	s1y = string( zareaw, N_YELLOW ) ;
	s1w = string( zareaw, N_WHITE  ) ;
	div = string( zareaw, '-' ) ;
}


void pbro01a::load_language_colours()
{
	//
	// Load the language colours into the langColours map.
	// Create defaults if no table found.
	//

	int i ;
	int ws ;

	string temp ;

	string lang ;
	string cold ;
	string colc ;
	string colk ;
	string colq ;
	string colv ;
	string cols ;

	string hid ;
	string hic ;
	string hik ;
	string hiq ;
	string hiv ;
	string his ;

	string sp ;

	const string vlist1 = "LANG COLD COLC COLK COLQ COLV COLS" ;
	const string vlist2 = "HID HIC HIK HIQ HIV HIS SPECIALS" ;

	vdefine( vlist1, &lang, &cold, &colc, &colk, &colq, &colv, &cols ) ;
	vdefine( vlist2, &hid, &hic, &hik, &hiq, &hiv, &his, &sp ) ;

	tbopen( CLRTABLE, NOWRITE, "ZUPROF" ) ;
	if ( RC == 0 )
	{
		tbskip( CLRTABLE ) ;
		while ( RC == 0 )
		{
			langColours[ lang ] = lang_colours( cold,
							    colc,
							    colk,
							    colq,
							    colv,
							    cols,
							    hid,
							    hic,
							    hik,
							    hiq,
							    hiv,
							    his,
							    sp ) ;
			tbskip( CLRTABLE ) ;
		}
		tbend( CLRTABLE ) ;
	}

	if ( langColours.size() == 0 )
	{
		const string list = "ASM BASH COBOL CPP DEFAULT JCL OTHER PANEL RUST TOML SKEL REXX" ;
		for ( i = 1, ws = words( list ) ; i <= ws ; ++i )
		{
			temp = word( list, i ) ;
			langColours[ temp ] = lang_colours( langSpecials[ temp ] ) ;
		}
	}

	vdelete( vlist1, vlist2 ) ;
}


void pbro01a::set_language_fvars( const string& lang )
{
	//
	// Set the language colours in the function pool variables.
	// If it isn't found, create a default for the language and add to the map.
	//

	auto it = langColours.find( lang ) ;
	if ( it == langColours.end() )
	{
		auto result = langColours.insert( pair<string, lang_colours>( lang, lang_colours( langSpecials[ lang ] ) ) ) ;
		it = result.first ;
	}

	const lang_colours& c = it->second ;

	zedclrd = c.cold ;
	zedclrc = c.colc ;
	zedclrk = c.colk ;
	zedclrq = c.colq ;
	zedclrv = c.colv ;
	zedclrs = c.cols ;

	zedhid  = c.hid ;
	zedhic  = c.hic ;
	zedhik  = c.hik ;
	zedhiq  = c.hiq ;
	zedhiv  = c.hiv ;
	zedhis  = c.his ;

	hlight.hl_specials = c.specials ;
}


string pbro01a::determineLang()
{
	//
	// Try to determine the language, first from the extension, then the directory containing the source,
	// then the contents of the file.
	// Limit the scan to the first 100 lines of code.
	//
	// If the text contains ANSI colour code sequences, use this in preference.
	//
	// Returned language must exist in ehilight (hiRoutine function map) or an exception will occur.
	//

	int i ;
	int j ;

	bool diffu1 = false ;
	bool diffc1 = false ;

	size_t p1 ;
	size_t p2 ;

	string s ;
	string w ;

	const string cpp   = "CPP"   ;
	const string rexx  = "REXX"  ;
	const string cobol = "COBOL" ;
	const string assem = "ASM"   ;
	const string jcl   = "JCL"   ;
	const string bash  = "BASH"  ;
	const string other = "OTHER" ;
	const string panel = "PANEL" ;
	const string skel  = "SKEL"  ;
	const string diffu = "DIFFU" ;
	const string diffc = "DIFFC" ;
	const string def   = "DEFAULT" ;
	const string ansi  = "ANSI" ;

	if ( ansi_on )
	{
		for ( i = 1, j = 0 ; i < data.size() && i < 200 ; ++i )
		{
			string& t = data.at( i ) ;
			p1 = t.find( ansi_start ) ;
			while ( p1 != string::npos )
			{
				p2 = t.find_first_not_of( ansi_codes, p1+2 ) ;
				if ( p2 == string::npos )
				{
					break ;
				}
				if ( t[ p2 ] == ansi_end )
				{
					++j ;
					break ;
				}
				p1 = t.find( ansi_start, p2 ) ;
			}
		}
		if ( j > 0 )
		{
			return ansi ;
		}
	}

	p1 = zfile.find_last_of( '.' ) ;
	if ( p1 != string::npos )
	{
		s = zfile.substr( p1+1 ) ;
		if ( findword( s, "c cpp h hpp" ) )   { return cpp   ; }
		if ( findword( s, "rex rexx rx" ) )   { return rexx  ; }
		if ( findword( s, "cob cobol"   ) )   { return cobol ; }
		if ( findword( s, "asm assem"   ) )   { return assem ; }
		if ( findword( s, "output errors" ) ) { return def ; }
	}

	if ( zfile.find( "/rexx/" )  != string::npos ) { return rexx  ; }
	if ( zfile.find( "/cobol/" ) != string::npos ) { return cobol ; }

	for ( i = 1 ; i < data.size() && i < 100 ; ++i )
	{
		const string& t = data.at( i ) ;
		if ( t.size() == 0 ) { continue ; }
		w = word( t, 1 ) ;
		if ( findword( w, "#!/bin/sh #!/bin/bash" ) ||
		   ( w == "#!" && findword( word( t, 2 ), "/bin/sh /bin/bash" ) ) )
		{
			return bash ;
		}
		if ( i == 1 && w == "---" ) { diffu1 = true ; continue ; }
		if ( i == 1 && w == "***" ) { diffc1 = true ; continue ; }
		if ( i == 2 && diffu1 && w == "+++" ) { return diffu ; }
		if ( i == 2 && diffc1 && w == "---" ) { return diffc ; }
		if ( t[ 0 ] == '*' ) { return assem ; }
		if ( t[ 0 ] == ')' )
		{
			w = word( t, 1 ) ;
			if ( w == ")PANEL" || w == ")COMMENT" || w == ")ATTR" )
			{
				return panel ;
			}
			return skel ;
		}
		if ( findword( w, "TITLE CSECT DSECT MACRO START COPY" ) )
		{
			return assem ;
		}
		if ( ( t.size() > 23 && upper( subword( t.substr( 6 ), 1, 2 ) ) == "IDENTIFICATION DIVISION." ) ||
		     ( t.size() > 11 && upper( subword( t.substr( 6 ), 1, 2 ) ) == "ID DIVISION." ) )
		{
			return cobol ;
		}
		if ( i == 1 && t.size() > 2 && t[ 0 ] == '/' )
		{
			if ( t.compare( 1, 2, "/*" ) == 0 ) { return jcl ; }
			if ( t[ 1 ] == '/' )
			{
				w = word( t, 2 ) ;
				if ( findword( w, "JOB DD PROC EXEC MSG" ) ) { return jcl ; }
			}
			else if ( t.compare( 1, 9, "*PRIORITY" ) == 0 ) { return jcl ; }
		}
		if ( i == 1 && upper( t ).find( "REXX" ) != string::npos )
		{
			return rexx ;
		}
		if ( w == "/*" )  { return cpp ; }
	}
	return def ;
}
