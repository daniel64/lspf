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
/*  BIN/BINARY                                                                            */
/*      Open file as binary                                                               */
/*  HEX ON | OFF                                                                          */
/*      Display data as hex                                                               */
/*  COLS ON | OFF                                                                         */
/*      Display columns on top line of data                                               */
/*  F/FIND                                                                                */
/*      Find string                                                                       */
/*  RES/RESET                                                                             */
/*      Reset options ( COLS OFF )                                                        */
/*  TEXT                                                                                  */
/*      Open file as text (0A will be interpreted as newline)                             */
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

b_find PBRO01A::Global_bfind_parms ;


PBRO01A::PBRO01A()
{
	set_appdesc( "Browser for lspf" ) ;
	set_appver( "1.0.1" ) ;

	vdefine( "ZCMD ZVERB ZROW1 ZCURFLD", &zcmd, &zverb, &zrow1, &zcurfld ) ;
	vdefine( "ZAREA ZSHADOW ZAREAT ZDSN", &zarea, &zshadow, &zareat, &zdsn ) ;
	vdefine( "ZSCROLLN ZAREAW ZAREAD ZCURPOS", &zscrolln, &zareaw, &zaread, &zcurpos ) ;
	vdefine( "ZSCROLLA ZCOL1 ZCOL2 TYPE STR", &zscrolla, &zcol1, &zcol2, &type, &str ) ;
	vdefine( "OCC LINES CMD ZZSTR1", &occ, &lines, &cmd, &zzstr1 ) ;

	XRC  = 0 ;
	XRSN = 0 ;
}


void PBRO01A::application()
{
	Browse() ;

	ZRC  = XRC ;
	ZRSN = XRSN ;
}


void PBRO01A::Browse()
{
	int i       ;
	int lchng   ;
	int curpos  ;
	uint offset ;

	string w2     ;
	string w3     ;
	string file   ;
	string panel  ;
	string curfld ;
	string zbralt ;

	bool rebuildZAREA ;

	map<string, int>labelList ;

	errblock err ;

	msg = "" ;

	file = parseString( err, PARM, "FILE()" ) ;
	if ( err.error() || file == "" )
	{
		llog( "E", "Invalid parameter format passed to PBRO01A" << endl ; )
		abend() ;
		return  ;
	}

	panel = parseString( err, PARM, "PANEL()" ) ;
	if ( err.error() )
	{
		llog( "E", "Invalid parameter format passed to PBRO01A" << endl ; )
		abend() ;
		return  ;
	}
	if ( panel == "" ) { panel = "PBRO01A2" ; }

	zdsn = file ;

	rebuildZAREA = true  ;
	hexOn        = false ;
	colsOn       = false ;
	Asbin        = false ;
	binOn        = false ;
	textOn       = false ;
	hilightOn    = true  ;
	entLang      = ""    ;

	typList[ 'C' ] = "CHARS"  ;
	typList[ 'P' ] = "PREFIX" ;
	typList[ 'S' ] = "SUFFIX" ;
	typList[ 'W' ] = "WORD"   ;

	startCol = 1 ;
	maxCol   = 1 ;

	pquery( panel, "ZAREA", "ZAREAT", "ZAREAW", "ZAREAD" ) ;
	if ( RC > 0 ) { abend() ; }

	read_file( file ) ;
	if ( XRC > 0 ) { return ; }

	msg    = ""     ;
	curfld = "ZCMD" ;
	curpos = 1      ;

	if ( Global_bfind_parms.f_fset )
	{
		find_parms = Global_bfind_parms ;
	}

	zasize = zareaw * zaread ;

	zbralt = "" ;
	vget( "ZBRALT", SHARED ) ;
	if ( RC == 0 )
	{
		vcopy( "ZBRALT", zbralt, MOVE ) ;
	}
	if ( zbralt == "" ) { vreplace( "ZBRALT", zdsn ) ; }

	while ( true )
	{
		if ( rebuildZAREA ) { fill_dynamic_area() ; }
		else                { zshadow = cshadow   ; }
		if ( curfld == "ZAREA" )
		{
			for ( i = curpos-1 ; i < zasize ; ++i )
			{
				if ( zarea[ i ] == ' ' ) { break ; }
				if ( ( i / zareaw ) > ( data.size() - topLine - 2 ) ) { break ; }
				zshadow[ i ] = E_WHITE ;
			}
			for ( i = curpos-1 ; i >= 0 ; --i )
			{
				if ( zarea[ i ] == ' ' ) { break ; }
				if ( topLine == 0 && ( i / zareaw ) == 0 ) { break ; }
				zshadow[ i ] = E_WHITE ;
			}
		}

		zrow1 = d2ds( topLine, 8 )  ;
		zcol1 = d2ds( startCol, 5 ) ;
		zcol2 = d2ds( startCol+zareaw-1, 5 ) ;
		if ( msg == "" ) { zcmd = "" ; }

		display( panel, msg, curfld, curpos ) ;
		if ( RC == 8 ) { break ; }

		msg          = ""    ;
		rebuildZAREA = false ;

		vget( "ZVERB ZSCROLLA ZSCROLLN", SHARED ) ;

		cmd  = upper( word( zcmd, 1 ) ) ;
		w2   = upper( word( zcmd, 2 ) ) ;
		w3   = upper( word( zcmd, 3 ) ) ;

		if ( zcurfld == "ZAREA" )
		{
			if ( (colsOn && zcurpos <= zareaw) ) { curfld = "ZCMD" ; curpos = 1 ; }
			else if ( topLine == 0 && ( zcurpos <= zareaw || (colsOn && zcurpos <= 2*zareaw))) { curfld = "ZCMD" ; curpos = 1 ; }
			else { curfld = "ZAREA" ; curpos = zcurpos ; }
		}
		else
		{
			curfld = "ZCMD" ;
			curpos = 1      ;
		}

		if ( cmd == "" )
		{
			;
		}
		else if ( cmd == "BIN" || cmd == "BINARY" )
		{
			if ( w2 != "" ) { msg = "PBRO013" ; continue ; }
			Asbin        = true  ;
			binOn        = true  ;
			textOn       = false ;
			rebuildZAREA = true  ;
			read_file( file ) ;
			if ( XRC > 0 ) { return ; }
		}
		else if ( cmd == "HEX" )
		{
			if      ( (w2 == "" ) || (w2 == "ON" && w3 == "") ) { hexOn = true  ; rebuildZAREA = true ; }
			else if ( (w2 == "OFF" && w3 == "") )               { hexOn = false ; rebuildZAREA = true ; }
			else    { msg = "PBRO012" ; continue ; }
		}
		else if ( findword( cmd, "HILITE HILIGHT HI" ) )
		{
			if ( w3 == "" ) { w3 = "AUTO" ; }
			if ( w3 != "AUTO" && !addHilight( w3 ) ) { msg = "PBRO011M" ; continue ; }
			entLang = w3 ;
			if      ( w3 == "AUTO"  ) { detLang = determineLang( file ) ; }
			else                      { detLang = w3                    ; }
			if      ( w2 == "ON" )
			{
				for_each( shadow.begin(), shadow.end(),
					  [](b_shadow& a) { a.bs_wShadow = false ; a.bs_vShadow = false ; } ) ;
				hilightOn = true  ; rebuildZAREA = true ;
			}
			else if ( w2 == "OFF" )
			{
				for_each( shadow.begin(), shadow.end(),
					  [](b_shadow& a) { a.bs_wShadow = false ; a.bs_vShadow = false ; } ) ;
				hilightOn = false ; rebuildZAREA = true ;
			}
		}
		else if ( cmd == "COLS" )
		{
			if      ( (w2 == "" ) || (w2 == "ON" && w3 == "") ) { colsOn = true  ; rebuildZAREA = true ; }
			else if ( (w2 == "OFF" && w3 == "") )               { colsOn = false ; rebuildZAREA = true ; }
			else    { msg = "PBRO012" ; continue ; }
		}
		else if ( cmd == "F" || cmd == "FIND" )
		{
			if ( setFind() > 0 ) { continue ; }
			zcmd = ""  ;
			if ( zcurfld == "ZAREA" ) { offset = zcurpos ; if ( ( offset % zareaw ) == 1  ) { ++offset ; } }
			else                      { offset = 0       ; }
			actionFind( topLine, offset ) ;
			if ( find_parms.f_line > 0 )
			{
				zaline = find_parms.f_line - topLine ;
				if ( zaline < 0 || zaline > zaread-1 )
				{
					topLine = find_parms.f_line - 1 ;
				}
				curfld = "ZAREA" ;
				if ( find_parms.f_offset < startCol-1 || find_parms.f_offset > zareaw + startCol - 1 )
				{
					startCol = find_parms.f_offset - 13 ;
					if ( startCol < 1 ) { startCol = 1 ; }
				}
				curpos = ( find_parms.f_line - topLine ) * zareaw + find_parms.f_offset - startCol + 2 ;
				if ( colsOn ) { curpos += zareaw ; }
				type   = typList[ find_parms.f_mtch ] ;
				str    = find_parms.f_estring ;
				occ    = d2ds( find_parms.f_occurs ) ;
				lines  = d2ds( find_parms.f_lines  ) ;
				if ( find_parms.f_dir == 'A' ) { msg = "PBRO011G" ; find_parms.f_dir = 'N' ; }
				else                           { msg = "PBRO011F" ;                          }
			}
			else
			{
				curfld = "ZCMD" ;
				curpos = 1 ;
				type = typList[ find_parms.f_mtch ] ;
				str = find_parms.f_estring ;
				msg = "PBRO011E" ;
				continue         ;
			}
			rebuildZAREA = true ;
			continue            ;
		}
		else if ( cmd == "RES" || cmd == "RESET" )
		{
			if ( w2 != "" ) { msg = "PBRO013" ; continue ; }
			colsOn       = false ;
			binOn        = false ;
			textOn       = false ;
			rebuildZAREA = true  ;
		}
		else if ( cmd == "TEXT" )
		{
			if ( w2 != "" ) { msg = "PBRO013" ; continue ; }
			Asbin        = false ;
			binOn        = false ;
			textOn       = true  ;
			rebuildZAREA = true  ;
			read_file( file )    ;
			if ( XRC > 0 ) { return ; }
		}
		else if ( cmd.size() > 1 && cmd[ 0 ] == '.' )
		{
			labelList[ cmd ] = topLine ;
		}
		else if ( findword( cmd, "L LOC LOCATE" ) && w2.size() > 1 && w2[ 0 ] == '.' )
		{
			if ( labelList.count( w2 ) > 0 )
			{
				topLine = labelList[ w2 ] ;
				rebuildZAREA = true ;
			}
		}
		else
		{
			msg = "PBRO011" ;
			continue ;
		}

		if ( zverb == "" )
		{
			;
		}
		else if ( zverb == "DOWN" )
		{
			lchng = topLine ;
			rebuildZAREA = true ;
			if ( zscrolla == "MAX" )
			{
				topLine = maxLines - zaread ;
			}
			else
			{
				topLine = topLine + zscrolln  ;
				if ( topLine >= maxLines ) { topLine = maxLines - 1 ; }
			}
			if ( curfld == "ZAREA" && lchng != topLine )
			{
				lchng  = topLine - lchng ;
				curpos = curpos - lchng * zareaw ;
				if ( curpos < 0 ) { curpos = 1 ; curfld = "ZCMD" ; }
			}
		}
		else if ( zverb == "UP" )
		{
			lchng = topLine ;
			rebuildZAREA = true ;
			if ( zscrolla == "MAX" )
			{
				topLine = 0 ;
			}
			else
			{
				topLine = topLine - zscrolln  ;
				if ( topLine < 0 ) { topLine = 0 ; }
			}
			if ( curfld == "ZAREA" && lchng != topLine )
			{
				lchng  = topLine - lchng ;
				curpos = curpos - lchng * zareaw ;
				if ( curpos > zasize ) { curpos = 1 ; curfld = "ZCMD" ; }
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
		else if ( zverb == "RFIND" )
		{
			if ( find_parms.f_fset )
			{
				if ( zcurfld == "ZAREA" ) { offset = zcurpos ; if ( ( offset % zareaw ) == 1  ) { ++offset ; } }
				else                      { offset = 0       ; }
				actionFind( topLine, offset ) ;
				if ( find_parms.f_line > 0 )
				{
					zaline = find_parms.f_line - topLine ;
					if ( zaline < 0 || zaline > zaread-1 )
					{
						topLine = find_parms.f_line - 1 ;
					}
					curfld  = "ZAREA" ;
					if ( find_parms.f_offset < startCol-1 || find_parms.f_offset > zareaw + startCol - 1 )
					{
						startCol = find_parms.f_offset - 13 ;
						if ( startCol < 1 ) { startCol = 1 ; }
					}
					curpos = ( find_parms.f_line - topLine ) * zareaw + find_parms.f_offset - startCol + 2 ;
					if ( colsOn ) { curpos += zareaw ; }
					type    = typList[ find_parms.f_mtch ] ;
					str     = find_parms.f_estring ;
					msg     = "PBRO011F" ;
				}
				else
				{
					if ( find_parms.f_dir == 'N' ||
					     find_parms.f_dir == 'F' ) { find_parms.f_dir = 'F' ; msg = "PBRO016" ; }
					else                           { find_parms.f_dir = 'L' ; msg = "PBRO017" ; }
					continue ;
				}
				rebuildZAREA = true ;
			}
			else { msg = "PBRO011A" ; continue ; }
		}
		else
		{
			msg = "PBRO011" ;
			continue ;
		}
		if ( topLine < 0 ) topLine = 0 ;
	}
	verase( "ZBRALT", SHARED ) ;
	vput( "ZSCROLL", PROFILE ) ;
	Global_bfind_parms = find_parms ;
}


void PBRO01A::read_file( string file )
{
	string inLine ;
	string line1  ;
	string ext    ;
	string w1     ;

	b_shadow t    ;

	int rc ;
	int i   ;
	int j   ;

	size_t p1 ;

	char x  ;

	RC      = 0 ;
	topLine = 0 ;

	std::ifstream fin ;

	try
	{
		if ( !exists( file ) )
		{
			vreplace( "ZVAL1", file ) ;
			vput( "ZVAL1", SHARED ) ;
			XRC  = 14 ;
			XRSN = 0  ;
			ZRESULT = "PSYS011S" ;
			return    ;
		}
	}
	catch ( const filesystem_error& ex )
	{
		vreplace( "ZVAL1", file ) ;
		vput( "ZVAL1", SHARED ) ;
		XRC  = 20 ;
		XRSN = 4  ;
		ZRESULT = "PSYS011T" ;
		return    ;
	}

	if ( !is_regular_file( file.c_str() ) )
	{
		vreplace( "ZVAL1", file ) ;
		vput( "ZVAL1", SHARED ) ;
		XRC  = 20 ;
		XRSN = 8  ;
		ZRESULT = "PSYS011R" ;
		return    ;
	}

	fin.open( file.c_str() ) ;

	if ( !fin.is_open() )
	{
		vreplace( "ZVAL1", file ) ;
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
		fileType = magic_file( cookie, file.c_str() ) ;
		if ( fileType != "" )
		{
			if ( findword( "charset=binary", fileType ) )
			{
				Asbin = true ;
			}
			else
			{
				p1 = fileType.find( ';' ) ;
				if ( p1 != string::npos ) { fileType = fileType.substr( 0, p1 ) ; }
			}
		}
	}
	magic_close( cookie ) ;

	maxLines = 1 ;
	data.clear()   ;
	shadow.clear() ;
	data.push_back( centre( " Top of Data ", zareaw, '*' ) ) ;
	shadow.push_back( t ) ;

	if      ( binOn )  { Asbin = true ; }
	else if ( textOn ) { Asbin = false ; }

	if ( Asbin )
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
				j = 8 - (p1 % 8 ) ;
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
			detLang = determineLang( file ) ;
		}
		else
		{
			detLang = entLang ;
		}
		hilightOn = true  ;
	}
	if ( maxLines == 1 )
	{
		vreplace( "ZVAL1", file ) ;
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
	string s1b ;
	string s1g ;
	string s1y ;
	string s1w ;
	string div ;
	string col ;

	int i  ;
	int l  ;
	int t  ;
	int ln ;

	s1b = string( zareaw, E_BLUE   ) ;
	s1g = string( zareaw, E_GREEN  ) ;
	s1y = string( zareaw, E_YELLOW ) ;
	s1w = string( zareaw, E_WHITE  ) ;
	div = string( zareaw, '-' )      ;

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
				if ( ln > zareaw ) ln = zareaw       ;
				t1 = substr( data.at( k ), startCol, zareaw ) ;
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
				zarea   += t1  + t3  + t4  + div ;
				zshadow += s1y + s1b + s1b + s1w ;
			}
			else
			{
				zarea   += substr( data.at( k ), 1, zareaw ) ;
				zshadow += s1b ;
			}
			if ( zarea.size() >= zasize ) { break ; }
			if ( k >= data.size() - 1 )   { break ; }
		}
	}
	else
	{
		for ( int k = topLine ; k < (topLine + zaread) ; ++k )
		{
			if ( k >  0 && k < data.size()-1 ) { t1 = substr( data.at( k ), startCol, zareaw ) ; }
			else                               { t1 = substr( data.at( k ), 1, zareaw )        ; }
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

	string ztemp ;

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
		if ( its->bs_Shadow.size() >= startCol )
		{
			it1 = zshadow.begin() + ( zareaw * i ) ;
			it2 = its->bs_Shadow.begin() + ( startCol - 1 ) ;
			w = its->bs_Shadow.size() - ( startCol - 1 ) ;
			if ( w > zareaw ) { w = zareaw ; }
			zshadow.replace( it1, it1+w, it2, it2+w ) ;
		}
	}
}


int PBRO01A::setFind()
{
	int i        ;
	int j        ;
	int ws       ;
	char c1      ;
	char c2      ;
	string delim ;
	string com   ;
	string ucmd  ;
	string w1    ;
	string pic   ;

	size_t p1    ;
	size_t p2    ;

	const char quote('\"') ;
	const char apost('\'') ;

	b_find t ;

	msg = ""  ;
	com = " " + subword( zcmd, 2 ) + " " ;

	p1 = com.find( quote ) ;
	p2 = com.find( apost ) ;

	if      ( p1 == string::npos ) { delim = string( 1, apost ) ; }
	else if ( p2 == string::npos ) { delim = string( 1, quote ) ; }
	else if ( p1 < p2 )            { delim = string( 1, quote ) ; }
	else                           { delim = string( 1, apost ) ; }

	if ( ( p1 = pos( delim, com ) ) )
	{
		p2  = pos( delim, com,  p1+1 ) ;
		if ( p2 == 0 ) { msg = "PBRO011H" ; return 20 ; }
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
		else                               { msg = "PBRO011I" ; return 20 ; }
		t.f_estring = substr( com, (p1+1), (p2-p1-1) ) ;
		t.f_string  = t.f_estring ;
		if ( t.f_text ) { t.f_string = upper( t.f_estring ) ; }
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

	ws = words( ucmd ) ;
	for ( i = 1 ; i <= ws ; ++i )
	{
		w1 = word( ucmd, i ) ;
		if ( datatype( w1, 'W' ) )
		{
			if ( t.f_scol != 0 && t.f_ecol != 0 ) { msg = "PBRO019" ; return 20 ; }
			j = ds2d( w1 ) ;
			if ( j < 1 || j > 65535 ) { msg = "PBRO011J" ; return 20 ; }
			if ( t.f_scol == 0 ) { t.f_scol = j ; }
			else                 { t.f_ecol = j ; }
		}
		else
		{
			if ( w1 == "*" )
			{
				if ( find_parms.f_string == "" ) { msg = "PBRO011C" ; return 20 ; }
				else
				{
					if ( t.f_string != "" )  { msg = "PBRO015"  ; return 20 ; }
					w1 = find_parms.f_string ;
				}
			}
			if ( t.f_string != "" ) { str = w1 ; msg = "PBRO018" ; return 20 ; }
			t.f_text    = true ;
			t.f_estring = w1   ;
			t.f_string  = upper( w1 ) ;
		}
	}

	if ( t.f_scol>0 && t.f_ecol>0 && (t.f_scol > t.f_ecol) )
	{
		i        = t.f_scol ;
		t.f_scol = t.f_ecol ;
		t.f_ecol = i ;
	}

	if ( t.f_scol>0 && t.f_ecol>0 && ((t.f_ecol - t.f_scol) < t.f_estring.size()) ) { msg = "PBRO014" ; return 20 ; }

	if ( t.f_scol>0 && t.f_ecol==0 ) { t.f_oncol = true ; }

	if ( t.f_hex )
	{
		if ( !ishex( t.f_string ) )  { msg = "PBRO011K" ; return 20 ; }
		t.f_string = xs2cs( t.f_string ) ;
		t.f_asis   = true                ;
	}

	if ( t.f_pic )
	{
		pic = "" ;
		j   = 0  ;
		// =  any character                   .  invalid characters
		// @  alphabetic characters           -  non-numeric characters
		// #  numeric characters              <  lower case alphabetics
		// $  special characters              >  upper case alphabetics
		// ?  non-blank characters            *  any number of non-blank characters
		for ( i = 0 ; i < t.f_string.size() ; ++i )
		{
			switch ( t.f_string[ i ] )
			{
				case '*':
					pic = pic + "[^[:blank:]]*" ;
					break ;
				case '=':
					pic = pic + "." ;
					break ;
				case '@':
					pic = pic + "[[:alpha:]]" ;
					break ;
				case '#':
					pic = pic + "[[:digit:]]" ;
					break ;
				case '$':
					pic = pic + "[^[:blank:]^[:alpha:]^[:digit:]]" ;
					break ;
				case '?':
					pic = pic + "[^[:blank:]]" ;
					break ;
				case '.':
					pic = pic + "[^[:print:]]" ;
					break ;
				case '-':
					pic = pic + "[^[:digit:]]" ;
					break ;
				case '<':
					pic += "(?-i)[a-z](?i)" ;
					break ;
				case '>':
					pic += "(?-i)[A-Z](?i)" ;
					break ;
				default:
					pic += t.f_string[ i ] ;
					break ;
			}
		}
		t.f_string = pic  ;
		t.f_regreq = true ;
	}

	if ( t.f_regreq && !t.f_rreg )
	{
		switch ( t.f_mtch )
		{
			case 'P':
				t.f_string = "\\b" + t.f_string + "\\w" ;
				break ;
			case 'S':
				t.f_string = "\\w" + t.f_string + "\\b" ;
				break ;
			case 'W':
				t.f_string = "\\b" + t.f_string + "\\b" ;
				break ;
		}
	}

	t.f_fset = true ;
	find_parms = t  ;
	return 0 ;
}


void PBRO01A::actionFind( int spos, int offset )
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

	regex regexp ;
	smatch results ;

	find_parms.f_occurs = 0 ;
	find_parms.f_line   = 0 ;
	find_parms.f_lines  = 0 ;
	find_parms.f_offset = 0 ;

	if ( find_parms.f_dir == 'F' || find_parms.f_dir == 'A' || find_parms.f_dir == 'L' ) { offset = 0 ; }

	if ( offset > zareaw && colsOn ) { offset = offset - zareaw ; }
	if ( offset > 0 ) { oX = (offset % zareaw)+startCol-2 ; oY = offset / zareaw ; }
	else              { oX = 0        ; oY = 0 ; }
	if ( oX == -1 )   { oX = zareaw-1 ; --oY   ; }

	try
	{
		if ( find_parms.f_regreq )
		{
			if ( find_parms.f_asis )
			{
				regexp.assign( find_parms.f_string )   ;
			}
			else
			{
				regexp.assign( find_parms.f_string, boost::regex::ECMAScript|boost::regex::icase ) ;
			}
		}
	}
	catch  (boost::regex_error& e)
	{
		if ( find_parms.f_regreq )
		{
			msg = "PBRO011N" ;
			return           ;
		}
	}

	if      ( find_parms.f_dir == 'F' ) { dl = 1             ; }
	else if ( find_parms.f_dir == 'A' ) { dl = 1             ; }
	else if ( find_parms.f_dir == 'L' ) { dl = data.size()-2 ; }
	else                                { dl = spos + oY     ; }

	if ( dl > data.size() - 2 ) { dl = spos ; }
	if ( dl == 0 ) { dl = 1 ; }

	while ( true )
	{
		skip = false ;
		c1   = 0 ;
		c2   = data.at( dl ).size() -1 ;

		if ( find_parms.f_scol > 0 ) { c1 = find_parms.f_scol - 1 ; }
		if ( find_parms.f_ecol > 0 && c2 > find_parms.f_ecol-1 ) { c2 = find_parms.f_ecol - 1 ; }

		if ( oX > 0 )
		{
			if ( oX > data.at( dl ).size()-1 ) { oX = data.at( dl ).size()-1 ; }
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
				if ( regex_search( itss, itse, results, regexp ) )
				{
					if ( itss == results[0].first ) { found = true ; p1 = find_parms.f_scol-1 ; }
				}
			}
			else
			{
				if ( find_parms.f_dir == 'P' || find_parms.f_dir == 'L' || find_parms.f_dir == 'A' )
				{
					while ( regex_search( itss, itse, results, regexp ) )
					{
						found = true ;
						for ( p1 = c1 ; itss != results[0].first ; ++itss ) { ++p1 ; }
						c1 = p1 + 1 ;
						itss  = results[0].first ;
						++itss ;
						if ( found1 ) { found1 = false ; ++find_parms.f_lines ; }
						++find_parms.f_occurs ;
						if ( find_parms.f_dir == 'A' && find_parms.f_line == 0 ) { find_parms.f_line = dl ; find_parms.f_offset = p1 ; }
					}
				}
				else
				{
					if ( regex_search( itss, itse, results, regexp ) )
					{
						found = true ;
						for ( p1 = c1 ; itss != results[0].first ; ++itss ) { ++p1 ; }
					}
				}
			}
			if ( found && find_parms.f_line == 0 ) { find_parms.f_line = dl ; find_parms.f_offset = p1 ; }
		}
		else
		{
			found1 = true ;
			while ( true )
			{
				found  = false ;
				if ( find_parms.f_dir == 'P' || find_parms.f_dir == 'L' )
				{
					if ( find_parms.f_asis ) { p1 = data.at( dl ).rfind( find_parms.f_string, c2 )       ; }
					else                     { p1 = upper( data.at( dl ) ).rfind( find_parms.f_string, c2 ) ; }
					c2 = p1 - 1 ;
				}
				else
				{
					if ( find_parms.f_asis ) { p1 = data.at( dl ).find( find_parms.f_string, c1 )          ; }
					else                     { p1 = upper( data.at( dl ) ).find( find_parms.f_string, c1 ) ; }
					c1 = p1 + 1 ;
				}
				if ( p1 != string::npos )
				{
					if ( find_parms.f_oncol )
					{
						if ( p1 == find_parms.f_scol-1 ) { found = true ; }
					}
					else if ( find_parms.f_dir == 'P' || find_parms.f_dir == 'L' )
					{
						if ( p1 >= c1 ) { found = true ; }
					}
					else
					{
						if ( p1 <= c2 ) { found = true ; }
					}
				}
				if ( found && find_parms.f_line == 0 )   { find_parms.f_line = dl ; find_parms.f_offset = p1 ; }
				if ( find_parms.f_dir != 'A' || !found ) { break ; }
				++find_parms.f_occurs ;
				if ( found1 ) { found1 = false ; ++find_parms.f_lines ; }
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

string PBRO01A::determineLang( string zfile )
{
	// Try to determine the language, first from the extension, then the directory containing the source,
	// then the contents of the file.
	// Limit the scan to the first 100 lines of code

	// Returned language must exist in eHilight (hiRoutine function map) or an exception will occur

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
