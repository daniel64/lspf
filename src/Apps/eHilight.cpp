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


void addHilight( hilight & h, const string & line, string & shadow )
{
	hiRoutine[ h.hl_language ]( h, line, shadow ) ;
}


bool addHilight( string lang )
{
	return findword( lang, "ASM CPP DEFAULT OTHER PANEL REXX" ) ;
}


void addCppHilight( hilight & h, const string & line, string & shadow )
{
	int ln ;
	int p1 ;
	int start  ;
	int stop   ;
	int oBrac1 ;
	int oBrac2 ;

	uint j ;

	string w ;
	const string delims( " (){}=;><+-*[]" ) ;

	bool oQuote   ;
	bool oComment ;

	char Quote    ;

	map<string, keyw>::iterator it ;

	oQuote   = false ;

	oBrac1   = h.hl_oBrac1   ;
	oBrac2   = h.hl_oBrac2   ;
	oComment = h.hl_oComment ;

	ln = line.size() ;
	if ( ln == 0 ) { return ; }

	shadow = string( ln, N_GREEN ) ;
	start  = 0 ;
	stop   = 0 ;
	p1     = 0 ;

	for ( j = 0 ; j < ln ; j++ )
	{
		if ( !oQuote && ln > 1 && j < ln-1 && line.compare( j, 2, "/*") == 0 )
		{
			oComment = true ;
			shadow.replace( j, 2, 2, N_TURQ ) ;
			j++ ;
			continue ;
		}
		if ( !oQuote && ln > 1 && oComment && j < ln-1 && line.compare( j, 2, "*/" ) == 0 )
		{
			oComment = false ;
			shadow.replace( j, 2, 2, N_TURQ ) ;
			j++ ;
			continue ;
		}

		if ( ln > 1 && j < ln-1 && line.compare( j, 2, "\\'" ) == 0 )
		{
			j++ ;
			continue ;
		}

		if ( oComment )
		{
			shadow[ j ] = N_TURQ ;
			continue ;
		}

		if ( line[ j ] == ' '   && !oQuote ) { continue ; }
		if ( line[ j ] == '"'   && !oQuote ) { oQuote = true  ; Quote = '"'  ; start = j ; continue ; }
		if ( line[ j ] == '\''  && !oQuote ) { oQuote = true  ; Quote = '\'' ; start = j ; continue ; }
		if ( line[ j ] == Quote &&  oQuote )
		{
			oQuote = false ;
			stop   = j ;
			shadow.replace( start, stop-start+1, stop-start+1, N_WHITE ) ;
			continue ;
		}
		if ( oQuote )   { continue ; }
		if ( j > ln-1 ) { break    ; }
		p1 = line.find_first_of( delims, j ) ;
		if ( p1 != j || p1 == string::npos )
		{
			if ( p1 == string::npos ) { p1 = ln ; }
			start = j ;
			stop  = p1 - 1 ;
			w     = line.substr( start, stop-start+1 ) ;
			it    = keywList1.find( w ) ;
			if ( h.hl_ifLogic )
			{
				if ( w == "if" )
				{
					h.hl_oIf++ ;
				}
				else if ( w == "else" )
				{
					if ( h.hl_oIf == 0 )
					{
					}
					else { h.hl_oIf-- ; }
				}
			}
			if ( it != keywList1.end() )
			{
				shadow.replace( start, it->second.kw_len, it->second.kw_len, it->second.kw_col ) ;
			}
			else if ( w == "//" )
			{
				shadow.replace( start, ln-j+2, ln-j+2, N_TURQ ) ;
				j = ln ;
			}
			else { j = p1 ; }
		}
		if ( line[ j ] == '(' ) { oBrac1++ ; shadow[ j ] = oBrac1 % 7 + 7 ; continue ; }
		if ( line[ j ] == ')' )
		{
			if ( oBrac1 == 0 ) { shadow[ j ] = R_WHITE ; }
			else               { shadow[ j ] = oBrac1 % 7 + 7 ; oBrac1-- ; }
			continue ;
		}
		if ( line[ j ] == '{' ) { oBrac2++ ; shadow[ j ] = oBrac2 % 7 + 7 ; continue ; }
		if ( line[ j ] == '}' )
		{
			if ( h.hl_doLogic )
			{
				if ( oBrac2 == 0 ) { shadow[ j ] = R_WHITE ; }
				else               { shadow[ j ] = oBrac2 % 7 + 7 ; oBrac2-- ; }
			}
			continue ;
		}
		if ( line[ j ] == '=' ) { shadow[ j ] = B_YELLOW ; continue ; }
	}
	h.hl_oBrac1   = oBrac1 ;
	h.hl_oBrac2   = oBrac2 ;
	h.hl_oComment = oComment ;
}


void addASMHilight( hilight & h, const string & line, string & shadow )
{
	int ln ;
	int wd ;
	int p1 ;
	int start  ;
	int stop   ;
	int oBrac1 ;
	int oBrac2 ;

	uint j ;

	string w ;
	const string delims( " (){}=;><+-*[]" ) ;

	bool oQuote   ;
	bool oComment ;

	char Quote    ;

	oQuote   = false ;
	oBrac1   = h.hl_oBrac1   ;
	oBrac2   = h.hl_oBrac2   ;
	oComment = h.hl_oComment ;

	ln = line.size() ;
	if ( ln == 0 ) { return ; }

	start = 0 ;
	stop  = 0 ;
	p1    = 0 ;
	if ( line[ 0 ] == '*' )
	{
		shadow = string( ln, N_TURQ ) ;
		return ;
	}
	shadow = string( ln, N_GREEN ) ;

	if ( line[ 0 ] == ' ' ) { wd = 1 ; }
	else                    { wd = 0 ; }
	for ( j = 0 ; j < ln ; j++ )
	{
		if ( ln > 1 && j < ln-2 && line.compare( j, 2, "\\'" ) == 0 )
		{
			j++ ;
			continue ;
		}

		if ( line[ j ] == ' '   && !oQuote ) { continue ; }
		if ( line[ j ] == '"'   && !oQuote ) { oQuote = true  ; Quote = '"'  ; start = j ; continue ; }
		if ( line[ j ] == '\''  && !oQuote ) { oQuote = true  ; Quote = '\'' ; start = j ; continue ; }
		if ( line[ j ] == Quote &&  oQuote )
		{
			oQuote = false ;
			stop  = j ;
			shadow.replace( start, stop-start+1, stop-start+1, N_WHITE ) ;
			continue ;
		}
		if ( oQuote )   { continue ; }
		if ( j > ln-1 ) { break    ; }
		if ( line[ j ] != ' ' )
		{
			start = j ;
			p1 = line.find_first_of( ' ', j ) ;
			if ( p1 == string::npos ) { p1 = ln ; }
			wd++ ;
			if ( wd == 1 )
			{
				shadow.replace( start, p1-start+1, p1-start+1, N_TURQ ) ;
			}
			else if ( wd == 2 )
			{
				shadow.replace( start, p1-start+1, p1-start+1, N_RED ) ;
			}
			else if ( wd == 3 )
			{
				shadow.replace( start, p1-start+1, p1-start+1, N_GREEN ) ;
			}
			else
			{
				shadow.replace( start, p1-start+1, p1-start+1, N_TURQ ) ;
			}
			j = p1 ;
			if ( j > ln-1 ) { break ; }
		}
	}
	if ( ln > 71 )
	{
		shadow.replace( 71, ln-71, ln-71, B_WHITE ) ;
	}
}


void addRxxHilight( hilight & h, const string & line, string & shadow )
{
	int ln ;
	int p1 ;
	int start  ;
	int stop   ;
	int oBrac1 ;
	int oBrac2 ;

	uint j ;

	string w ;
	const string delims( " (){}=;><+-*[]" ) ;

	bool oQuote   ;
	bool oComment ;

	char Quote    ;

	map<string, keyw>::iterator it ;

	oQuote   = false ;

	oBrac1   = h.hl_oBrac1   ;
	oBrac2   = h.hl_oBrac2   ;
	oComment = h.hl_oComment ;

	ln = line.size() ;
	if ( ln == 0 ) { return ; }

	shadow = string( ln, N_GREEN ) ;
	start  = 0 ;
	stop   = 0 ;
	p1     = 0 ;

	for ( j = 0 ; j < ln ; j++ )
	{
		if ( !oQuote && ln > 1 && j < ln-1 && line.compare( j, 2, "/*") == 0 )
		{
			oComment = true ;
			shadow.replace( j, 2, 2, N_TURQ ) ;
			j++ ;
			continue ;
		}
		if ( !oQuote && ln > 1 && oComment && j < ln-1 && line.compare( j, 2, "*/" ) == 0 )
		{
			oComment = false ;
			shadow.replace( j, 2, 2, N_TURQ ) ;
			j++ ;
			continue ;
		}

		if ( ln > 1 && j < ln-1 && line.compare( j, 2, "\\'" ) == 0 )
		{
			j++ ;
			continue ;
		}

		if ( oComment )
		{
			shadow[ j ] = N_TURQ ;
			continue ;
		}

		if ( line[ j ] == ' '   && !oQuote ) { continue ; }
		if ( line[ j ] == '"'   && !oQuote ) { oQuote = true  ; Quote = '"'  ; start = j ; continue ; }
		if ( line[ j ] == '\''  && !oQuote ) { oQuote = true  ; Quote = '\'' ; start = j ; continue ; }
		if ( line[ j ] == Quote &&  oQuote )
		{
			oQuote = false ;
			stop   = j ;
			shadow.replace( start, stop-start+1, stop-start+1, N_WHITE ) ;
			continue ;
		}
		if ( oQuote )   { continue ; }
		if ( j > ln-1 ) { break    ; }
		p1 = line.find_first_of( delims, j ) ;
		if ( p1 != j || p1 == string::npos )
		{
			if ( p1 == string::npos ) { p1 = ln ; }
			start = j ;
			stop  = p1 - 1 ;
			w     = line.substr( start, stop-start+1 ) ;
			w     = upper( w ) ;
			it    = keywList2.find( w ) ;
			if ( it != keywList1.end() )
			{
				shadow.replace( start, it->second.kw_len, it->second.kw_len, it->second.kw_col ) ;
			}
			else if ( w == "//" )
			{
				shadow.replace( start, ln-j+2, ln-j+2, N_TURQ ) ;
				j = ln ;
			}
			else { j = p1 ; }
			if ( h.hl_ifLogic )
			{
				if ( w == "IF" )
				{
					h.hl_oIf++ ;
				}
				else if ( w == "ELSE" )
				{
					if ( h.hl_oIf == 0 )
					{
						shadow.replace( start, 4, 4, R_YELLOW ) ;
					}
					else { h.hl_oIf-- ; }
				}
			}
		}
		if ( line[ j ] == '(' ) { oBrac1++ ; shadow[ j ] = oBrac1 % 7 + 7 ; continue ; }
		if ( line[ j ] == ')' )
		{
			if ( oBrac1 == 0 ) { shadow[ j ] = R_WHITE ; }
			else               { shadow[ j ] = oBrac1 % 7 + 7 ; oBrac1-- ; }
			continue ;
		}
		if ( line[ j ] == '{' ) { oBrac2++ ; shadow[ j ] = oBrac2 % 7 + 7 ; continue ; }
		if ( line[ j ] == '}' )
		{
			if ( h.hl_doLogic )
			{
				if ( oBrac2 == 0 ) { shadow[ j ] = R_WHITE ; }
				else               { shadow[ j ] = oBrac2 % 7 + 7 ; oBrac2-- ; }
			}
			continue ;
		}
		if ( line[ j ] == '=' ) { shadow[ j ] = B_YELLOW ; continue ; }
	}
	h.hl_oBrac1   = oBrac1 ;
	h.hl_oBrac2   = oBrac2 ;
	h.hl_oComment = oComment ;
}


void addOthHilight( hilight & h, const string & line, string & shadow )
{
	int ln ;

	ln = line.size() ;
	if ( ln == 0 ) { return ; }
	shadow = string( ln, N_GREEN ) ;
}


void addDefHilight( hilight & h, const string & line, string & shadow )
{
	int ln ;

	ln = line.size() ;
	if ( ln == 0 ) { return ; }
	shadow = string( ln, N_GREEN ) ;
}


