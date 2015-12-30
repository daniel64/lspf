/*
  Copyright (c) 2015 Daniel John Erdos

  This program is free software; you can redistribute it and/or modify x
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
#include <string>
using namespace std ;

/* *************************************************************************************************** */
/* *VERY* simple routine to add a bit of hilighting to C++ programs and various other types of files.  */
/* Match brackets and braces and hilight mis-matches                                                   */
/* *************************************************************************************************** */

#include "eHilight.h"

#undef LOGOUT
#undef MOD_NAME

#define LOGOUT aplog
#define MOD_NAME HILIGHT


void addHilight( hilight & h, string line, string & shadow )
{
	if      ( h.hl_language == "text/x-panel" ) { }
	else if ( h.hl_language == "text/x-c++"  ||
		  h.hl_language == "CPP"         ||
		  h.hl_language == "text/x-c"     ) { addCppHilight( h, line, shadow ) ; }
	else if ( h.hl_language == "text/x-rexx"  ) { }
	else if ( h.hl_language == "text"         ) { }
	else                                        { }
}


bool addHilight( string lang )
{
	if ( wordpos( lang, "AUTO CPP PANEL" ) == 0 ) { return false ; }
	return true ;
}


void addCppHilight( hilight & h, string line, string & shadow )
{
	int ln ;
	int start  ;
	int stop   ;
	int oBrac1 ;
	int oBrac2 ;
	int p1     ;

	uint j ;

	string w ;
	const string delims( "(){}= ;><+-*[]" ) ;

	bool oQuote   ;
	bool oComment ;

	char Quote    ;

	oQuote   = false ;
	oBrac1   = h.hl_oBrac1   ;
	oBrac2   = h.hl_oBrac2   ;
	oComment = h.hl_oComment ;

	ln = line.size() ;
	if ( ln == 0 ) { return ; }

	shadow = string( ln, N_GREEN ) ;
	start = 0 ;
	stop  = 0 ;
	p1    = 0 ;

	for ( j = 0 ; j < ln ; j++ )
	{
		if ( line[ j ] == ' '   && !oQuote ) { continue ; }
		if ( line[ j ] == '"'   && !oQuote ) { oQuote = true  ; Quote = '"'  ; start = j ; continue ; }
		if ( line[ j ] == '\''  && !oQuote ) { oQuote = true  ; Quote = '\'' ; start = j ; continue ; }
		if ( line[ j ] == Quote &&  oQuote )
		{
			oQuote = false ;
			stop  = j ;
			shadow.replace( start, stop-start+1, stop-start+1, N_YELLOW ) ;
			continue ;
		}
		if ( oQuote )   { continue ; }
		if ( j > ln-1 ) { break    ; }
		if ( ln > 1 && j < ln-1 && line.compare( j, 2, "/*") == 0 )
		{
			oComment = true ;
			shadow.replace( j, 2, 2, B_BLUE ) ;
			j++ ;
			continue ;
		}
		if ( ln > 1 && oComment && j < ln-1 && line.compare( j, 2, "*/" ) == 0 )
		{
			oComment = false ;
			shadow.replace( j, 2, 2, B_BLUE ) ;
			j++ ;
			continue ;
		}
		if ( oComment )
		{
			shadow.replace( j, 1, 1, B_BLUE ) ;
			continue ;
		}

		p1 = line.find_first_of( delims, j ) ;
		if ( p1 != j || p1 == string::npos )
		{
			if ( p1 == string::npos ) { p1 = ln - 1 ; }
			start = j ;
			stop  = p1 - 1 ;
			w     = line.substr( start, stop-start+1 ) ;
			if      ( w == "if" )       shadow.replace( start, 2, 2, B_WHITE ) ;
			else if ( w == "else" )     shadow.replace( start, 4, 4, B_RED   ) ;
			else if ( w == "for" )      shadow.replace( start, 3, 3, B_TURQ  ) ;
			else if ( w == "while" )    shadow.replace( start, 5, 5, B_WHITE ) ;
			else if ( w == "#include" ) shadow.replace( start, 8, 8, B_TURQ  ) ;
			else if ( w == "#define" )  shadow.replace( start, 7, 7, B_TURQ  ) ;
			else if ( w == "#undef" )   shadow.replace( start, 6, 6, B_TURQ  ) ;
			else if ( w == "void" )     shadow.replace( start, 4, 4, B_BLUE  ) ;
			else if ( w == "int" )      shadow.replace( start, 3, 3, B_BLUE  ) ;
			else if ( w == "uint" )     shadow.replace( start, 4, 4, B_BLUE  ) ;
			else if ( w == "bool" )     shadow.replace( start, 4, 4, B_BLUE  ) ;
			else if ( w == "string" )   shadow.replace( start, 6, 6, B_BLUE  ) ;
			else if ( w == "char" )     shadow.replace( start, 4, 4, B_BLUE  ) ;
			else if ( w == "//" )
			{
				shadow.replace( start, ln-j+2, ln-j+2, B_BLUE ) ;
				j = ln ;
			}
			else { j = p1  ; }
		}
		if ( line[ j ] == '(' ) { oBrac1++ ; shadow.replace( j, 1, 1, oBrac1 % 7 + 7 ) ; continue ; }
		if ( line[ j ] == ')' )
		{
			if ( oBrac1 == 0 ) { shadow.replace( j, 1, 1, R_WHITE ) ; }
			else               { shadow.replace( j, 1, 1, oBrac1 % 7 + 7 ) ; oBrac1-- ; }
			continue ;
		}
		if ( line[ j ] == '{' ) { oBrac2++ ; shadow.replace( j, 1, 1, oBrac2 % 7 + 7 ) ; continue ; }
		if ( line[ j ] == '}' )
		{
			if ( oBrac2 == 0 ) { shadow.replace( j, 1, 1, R_WHITE ) ; }
			else               { shadow.replace( j, 1, 1, oBrac2 % 7 + 7 ) ; oBrac2-- ; }
			continue ;
		}
		if ( line[ j ] == '=' ) { shadow.replace( j, 1, 1, B_WHITE ) ; continue ; }
	}
	h.hl_oBrac1   = oBrac1 ;
	h.hl_oBrac2   = oBrac2 ;
	h.hl_oComment = oComment ;
}

