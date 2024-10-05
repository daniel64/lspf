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

/*******************************************************************************************************/
/*                                                                                                     */
/* Simple routine to add a bit of hilighting to C++ programs and various other types of files.         */
/* Match brackets and braces and hilight mis-matches.                                                  */
/*                                                                                                     */
/* Currently supported file-types:                                                                     */
/*   Mainframe assembler                                                                               */
/*   C++                                                                                               */
/*   Rexx                                                                                              */
/*   Other (pseudo-PL/1 syntax)                                                                        */
/*   lspf panels                                                                                       */
/*   skeleton                                                                                          */
/*   diff (unified)                                                                                    */
/*   diff (context)                                                                                    */
/*   COBOL                                                                                             */
/*   JCL                                                                                               */
/*   RUST                                                                                              */
/*   TOML                                                                                              */
/*   ANSI (use imbedded ANSI colour code sequences)                                                    */
/*   Default is green                                                                                  */
/*                                                                                                     */
/*******************************************************************************************************/

using namespace std ;

#include "ehilight.h"

int taskid()     { return 0 ; }
string modname() { return "HILIGHT" ; }


void addHilight( logger* lg,
		 hilight& h,
		 const string& line,
		 string& shadow )
{
	try
	{
		hiRoutine[ h.hl_language ]( h, line, shadow ) ;
	}
	catch (...)
	{
		llog( "E", "An exception has occured hilighting line "<< line <<endl) ;
		llog( "E", "Exception: " <<  boost::current_exception_diagnostic_information() <<endl ) ;
		llog( "E", "Hilighting disabled" <<endl) ;
		h.hl_abend = true ;
	}
}


bool addHilight( const string& lang )
{
	return ( hiRoutine.count( lang ) > 0 ) ;
}


void addCppHilight( hilight& h,
		    const string& line,
		    string& shadow )
{
	//
	// Special characters (yellow) must also be in the delims list.
	//

	uint start ;

	size_t j  ;
	size_t ln ;
	size_t p1 ;

	string w ;
	const string delims( " (){}=;<>+-*/|:!%#[]&,\\\"'" ) ;

	bool oQuote = false ;
	char Quote ;

	map<string, keyw>::iterator it ;

	int  oBrac1   = h.hl_oBrac1   ;
	int  oBrac2   = h.hl_oBrac2   ;
	bool oComment = h.hl_oComment ;

	ln = line.size() ;
	if ( ln == 0 ) { shadow = "" ; return ; }

	shadow = string( ln, H_DEFAULT ) ;

	start = 0 ;

	for ( j = 0 ; j < ln ; ++j )
	{
		if ( !oQuote && ln > 1 && j < ln-1 )
		{
			if ( line.compare( j, 2, "/*" ) == 0 )
			{
				oComment = true ;
				shadow.replace( j, 2, 2, H_COMMENTS ) ;
				++j ;
				continue ;
			}
			if ( oComment && line.compare( j, 2, "*/" ) == 0 )
			{
				oComment = false ;
				shadow.replace( j, 2, 2, H_COMMENTS ) ;
				++j ;
				continue ;
			}
			if ( !oComment && line.compare( j, 2, "//" ) == 0 )
			{
				shadow.replace( j, ln-j, ln-j, H_COMMENTS ) ;
				break ;
			}
		}

		if ( oComment )
		{
			shadow[ j ] = H_COMMENTS ;
			continue ;
		}

		if ( oQuote && ln > 1 && j < ln-1 && ( line.compare( j, 2, "\\\\" ) == 0 ) )
		{
			++j ;
			continue ;
		}

		if ( ln > 1 && j < ln-1 && ( line.compare( j, 2, "\\'" ) == 0 || line.compare( j, 2, "\\\"" ) == 0 ) )
		{
			++j ;
			continue ;
		}

		if ( !oQuote )
		{
			if ( line[ j ] == ' '  ) { continue ; }
			if ( line[ j ] == '"' || line[ j ] == '\'' )
			{
				oQuote = true ;
				Quote  = line[ j ] ;
				start  = j ;
				continue   ;
			}
		}
		else if ( line[ j ] == Quote )
		{
			oQuote = false ;
			shadow.replace( start, j-start+1, j-start+1, H_QUOTED ) ;
			continue ;
		}
		if ( oQuote )
		{
			if ( j == ln-1 )
			{
				shadow.replace( start, j-start+1, j-start+1, R_WHITE ) ;
			}
			continue ;
		}
		if ( j == 0 )
		{
			j = line.find_first_not_of( ' ' ) ;
			if ( j == string::npos )
			{
				break ;
			}
			if ( line[ j ] == '#' )
			{
				p1 = line.find( ' ', j ) ;
				if ( p1 == string::npos )
				{
					shadow.replace( 0, ln, ln, H_COMPDIR ) ;
					break ;
				}
				else if ( p1 == j + 1 )
				{
					p1 = line.find_first_not_of( ' ', p1 + 1 ) ;
					if ( p1 == string::npos )
					{
						shadow.replace( 0, ln, ln, H_COMPDIR ) ;
						break ;
					}
					else
					{
						p1 = line.find( ' ', p1 ) ;
						if ( p1 == string::npos )
						{
							shadow.replace( 0, ln, ln, H_COMPDIR ) ;
							break ;
						}
						shadow.replace( 0, p1, p1, H_COMPDIR ) ;
						j = p1 ;
					}
				}
				else
				{
					shadow.replace( 0, p1, p1, H_COMPDIR ) ;
					j = p1 ;
				}
				continue ;
			}
		}
		if ( h.hl_specials.find( line[ j ] ) != string::npos )
		{
			shadow[ j ] = H_SPECIAL ;
			continue ;
		}
		p1 = line.find_first_of( delims, j ) ;
		if ( p1 != j || p1 == string::npos )
		{
			if ( p1 == string::npos ) { p1 = ln ; }
			w  = line.substr( j, p1-j ) ;
			it = kw_cpp.find( w ) ;
			if ( it != kw_cpp.end() )
			{
				shadow.replace( j, it->second.kw_len, it->second.kw_len, it->second.kw_col ) ;
			}
			if ( h.hl_ifLogic )
			{
				if ( w == "if" )
				{
					++h.hl_oIf ;
				}
				else if ( w == "else" )
				{
					if ( h.hl_oIf == 0 )
					{
						shadow.replace( j, 4, 4, R_MAGENTA ) ;
						h.hl_mismatch = true ;
					}
					else if ( h.hl_oIf > 0 )
					{
						--h.hl_oIf ;
					}
				}
			}
			j = p1 - 1 ;
			continue   ;
		}
		if ( h.hl_Paren )
		{
			if ( line[ j ] == '(' ) { ++oBrac1 ; shadow[ j ] = oBrac1 % 5 + 11 ; continue ; }
			if ( line[ j ] == ')' )
			{
				if ( oBrac1 == 0 ) { shadow[ j ] = R_MAGENTA ; h.hl_mismatch = true ; }
				else               { shadow[ j ] = oBrac1 % 5 + 11 ; --oBrac1 ; }
				continue ;
			}
		}
		if ( h.hl_doLogic )
		{
			if ( line[ j ] == '{' )
			{
				++oBrac2 ;
				shadow[ j ] = oBrac2 % 5 + 11 ;
				continue ;
			}
			else if ( line[ j ] == '}' )
			{
				if ( oBrac2 == 0 )
				{
					shadow[ j ]   = R_MAGENTA ;
					h.hl_mismatch = true ;
				}
				else if ( oBrac2 > 0 )
				{
					shadow[ j ] = oBrac2 % 5 + 11 ;
					--oBrac2 ;
					if ( oBrac2 == 0 ) { h.hl_oIf = 0 ; }
				}
				continue ;
			}
		}
	}
	h.hl_oBrac1   = oBrac1 ;
	h.hl_oBrac2   = oBrac2 ;
	h.hl_oComment = oComment ;
}


void addASMHilight( hilight& h,
		    const string& line,
		    string& shadow )
{
	uint i ;
	uint wd ;
	uint start ;
	uint j ;
	uint ln ;

	size_t p1 ;

	char Quote  = h.hl_Quote  ;
	bool oQuote = h.hl_oQuote ;

	ln = line.size() ;
	if ( ln == 0 ) { shadow = "" ; return ; }
	shadow = string( ln, H_DEFAULT ) ;
	if ( ln > 72 ) { ln = 72 ; }

	start = 0 ;

	if ( line[ 0 ] == '*' || ( ln > 1 && line.compare( 0, 2, ".*" ) == 0 ) )
	{
		shadow.replace( 0, ln, ln, H_COMMENTS ) ;
		return ;
	}

	j = 0 ;
	if ( h.hl_continue )
	{
		wd = 3 ;
		h.hl_continue = false ;
		if ( oQuote )
		{
			p1 = line.find_first_not_of( ' ' ) ;
			i  = ( ln < 16 ) ? ( ln - p1 ) : ( 16 - p1 ) ;
			if ( p1 < 15 )
			{
				shadow.replace( p1, i, i, R_RED ) ;
				j     = 15 ;
				start = 15 ;
			}
		}
	}
	else
	{
		if ( line[ 0 ] != ' ' ) { wd = 1 ; }
		else                    { wd = 2 ; }
		oQuote = false ;
	}

	for ( ; j < ln ; ++j )
	{
		if ( wd == 4 )
		{
			shadow.replace( j, ln-j, ln-j, H_COMMENTS ) ;
			break ;
		}
		if ( ln > 1 && j < ln-2 && line.compare( j, 2, "\\'" ) == 0 )
		{
			++j ;
			continue ;
		}

		if ( !oQuote && ln > 1 && j < ln-2 )
		{
			if ( line.compare( j, 2, "L\'" ) == 0 )
			{
				shadow.replace( j, 2, 2, H_QUOTED ) ;
				++j ;
				continue ;
			}
		}

		if ( !oQuote )
		{
			if ( line[ j ] == ' ' )
			{
				if ( j > 0 && line[ j - 1 ] != ' ' ) { ++wd ; }
				continue ;
			}
			if ( line[ j ] == '"' || line[ j ] == '\'' )
			{
				oQuote = true ;
				Quote  = line[ j ] ;
				start  = j ;
				continue   ;
			}
		}
		else if ( line[ j ] == Quote )
		{
			oQuote = false ;
			shadow.replace( start, j-start+1, j-start+1, H_QUOTED ) ;
			continue ;
		}
		if ( oQuote )
		{
			if ( j == ln-1 )
			{
				shadow.replace( start, j-start+1, j-start+1, R_WHITE ) ;
			}
			continue ;
		}
		if ( wd == 1 )
		{
			shadow[ j ] = H_COMMENTS ;
		}
		else if ( wd == 2 )
		{
			shadow[ j ] = N_RED ;
		}
	}
	if ( ln > 71 )
	{
		shadow.replace( 71, ln-71, ln-71, N_RED ) ;
		if ( line[ 71 ] != ' ' )
		{
			h.hl_continue = true ;
			if ( oQuote )
			{
				shadow.replace( start, 71-start, 71-start, H_QUOTED ) ;
			}
		}
	}
	h.hl_oQuote = oQuote ;
	h.hl_Quote  = Quote  ;
}


void addBshHilight( hilight& h,
		    const string& line,
		    string& shadow )
{
	//
	// Hilighting for bash and sh shells scripts.
	//

	uint j  ;
	uint ln ;

	size_t p1 ;

	int  oBrac1 = h.hl_oBrac1 ;

	string w ;
	map<string, keyw>::iterator it ;

	const string delims( " ()=,;<>+-*[]\"'/&|:%\\" ) ;

	char Quote ;
	bool oQuote = false ;

	ln = line.size() ;
	if ( ln == 0 ) { shadow = "" ; return ; }
	shadow = string( ln, H_DEFAULT ) ;

	j = 0 ;

	for ( ; j < ln ; ++j )
	{
		if ( !oQuote )
		{
			if ( line[ j ] == '#' )
			{
				shadow.replace( j, ln-j, ln-j, H_COMMENTS ) ;
				break ;
			}
			if ( line[ j ] == ' '  ) { continue ; }
			if ( line[ j ] == '"' || line[ j ] == '\'' )
			{
				oQuote = true ;
				Quote  = line[ j ] ;
				shadow.replace( j, 1, 1, H_QUOTED ) ;
				continue ;
			}
		}
		else if ( line[ j ] == Quote )
		{
			shadow.replace( j, 1, 1, H_QUOTED ) ;
			oQuote = false ;
			continue ;
		}
		if ( oQuote )
		{
			if ( line[ j ] == '$' )
			{
				shadow[ j ] = N_RED ;
				++j ;
				if ( j == ln ) { break ; }
				p1 = line.find_first_of( delims, j ) ;
				if ( p1 == string::npos ) { p1 = ln ; }
				shadow.replace( j, p1-j, p1-j, N_RED ) ;
				j = p1 - 1 ;
			}
			else
			{
				shadow.replace( j, 1, 1, H_QUOTED ) ;
			}
			continue ;
		}
		if ( h.hl_specials.find( line[ j ] ) != string::npos )
		{
			shadow[ j ] = H_SPECIAL ;
			if ( line[ j ] == '.' )
			{
				h.hl_oIf = 0 ;
				h.hl_oDo = 0 ;
			}
			continue ;
		}
		p1 = line.find_first_of( delims, j ) ;
		if ( p1 != j || p1 == string::npos )
		{
			if ( p1 == string::npos ) { p1 = ln ; }
			w     = line.substr( j, p1-j ) ;
			it    = kw_bash.find( w ) ;
			if ( it != kw_bash.end() )
			{
				shadow.replace( j, it->second.kw_len, it->second.kw_len, it->second.kw_col ) ;
			}
			if ( h.hl_ifLogic )
			{
				if ( w == "if" )
				{
					++h.hl_oIf ;
				}
				else if ( w == "else" )
				{
					if ( h.hl_oIf == 0 )
					{
						shadow.replace( j, 4, 4, R_MAGENTA ) ;
						h.hl_mismatch = true ;
					}
				}
				else if ( w == "fi" )
				{
					if ( h.hl_oIf == 0 )
					{
						shadow.replace( j, 6, 6, R_MAGENTA ) ;
						h.hl_mismatch = true ;
					}
					else if ( h.hl_oIf > 0 )
					{
						--h.hl_oIf ;
					}
				}
			}
			if ( h.hl_doLogic )
			{
				if ( w == "do" )
				{
					++h.hl_oDo ;
				}
				else if ( w == "done" )
				{
					if ( h.hl_oDo == 0 )
					{
						shadow.replace( j, 11, 11, R_MAGENTA ) ;
						h.hl_mismatch = true ;
					}
					else
					{
						--h.hl_oDo ;
					}
				}
			}
			j = p1 - 1 ;
			continue ;
		}
		if ( h.hl_Paren )
		{
			if ( line[ j ] == '(' )
			{
				++oBrac1 ;
				shadow[ j ] = oBrac1 % 5 + 11 ;
				continue ;
			}
			if ( line[ j ] == ')' )
			{
				if ( oBrac1 > 0 )
				{
					shadow[ j ] = oBrac1 % 5 + 11 ;
					--oBrac1 ;
				}
				continue ;
			}
		}
	}
}


void addRxxHilight( hilight& h,
		    const string& line,
		    string& shadow )
{
	//
	// Special characters (yellow) must also be in the delims list.
	//

	uint j     ;
	uint ln    ;
	uint start ;

	size_t p1 ;
	size_t p2 ;
	int    p3 ;

	string w ;
	string t ;

	const string delims( " ()=,;<>+-*[]\"'/&|:%\\" ) ;

	bool oQuote  = false ;
	bool newline = true  ;
	char Quote ;

	map<string, RX_INS>::iterator itp ;
	map<string, keyw>::iterator it ;

	int  oBrac1   = h.hl_oBrac1   ;
	bool oComment = h.hl_oComment ;

	ln = line.size() ;
	if ( ln == 0 )
	{
		shadow = "" ;
		h.hl_continue = false ;
		return ;
	}

	shadow = string( ln, H_DEFAULT ) ;

	start = 0 ;

	for ( j = 0 ; j < ln ; ++j )
	{
		if ( !oQuote && ln > 1 && j < ln-1 )
		{
			if ( line.compare( j, 2, "/*" ) == 0 )
			{
				oComment = true ;
				shadow.replace( j, 2, 2, H_COMMENTS ) ;
				++j ;
				continue ;
			}
			if ( oComment && line.compare( j, 2, "*/" ) == 0 )
			{
				oComment = false ;
				shadow.replace( j, 2, 2, H_COMMENTS ) ;
				++j ;
				continue ;
			}
			if ( !oComment && line.compare( j, 2, "--" ) == 0 )
			{
				shadow.replace( j, ln-j, ln-j, H_COMMENTS ) ;
				break ;
			}
		}

		if ( ln > 1 && j < ln-1 && line.compare( j, 2, "\\'" ) == 0 )
		{
			++j ;
			continue ;
		}

		if ( oComment )
		{
			shadow[ j ] = H_COMMENTS ;
			continue ;
		}

		if ( !oQuote )
		{
			if ( line[ j ] == ' '  ) { continue ; }
			if ( line[ j ] == '"' || line[ j ] == '\'' )
			{
				oQuote = true ;
				Quote  = line[ j ] ;
				start  = j ;
				continue   ;
			}
		}
		else if ( line[ j ] == Quote )
		{
			oQuote = false ;
			shadow.replace( start, j-start+1, j-start+1, H_QUOTED ) ;
			continue ;
		}
		if ( oQuote )
		{
			if ( j == ln-1 )
			{
				shadow.replace( start, j-start+1, j-start+1, R_WHITE ) ;
			}
			continue ;
		}
		if ( h.hl_specials.find( line[ j ] ) != string::npos )
		{
			shadow[ j ] = H_SPECIAL ;
			continue ;
		}
		p1 = line.find_first_of( delims, j ) ;
		if ( p1 != j || p1 == string::npos )
		{
			if ( p1 == string::npos ) { p1 = ln ; }
			w  = upper( line.substr( j, p1-j ) ) ;
			it = kw_rexx.find( w ) ;
			if ( it != kw_rexx.end() )
			{
				shadow.replace( j, it->second.kw_len, it->second.kw_len, it->second.kw_col ) ;
			}
			if ( h.hl_ifLogic )
			{
				while ( newline && !h.hl_istack.empty() && !h.hl_continue &&
				       ( h.hl_istack.top() == RX_INSTR1 || h.hl_istack.top() == RX_END1 ) &&
					 w != "ELSE" )
				{
					addRxxHilight_unwind_if( h ) ;
					h.hl_if = ( h.hl_istack.empty() ) ? RX_NONE : h.hl_istack.top() ;
				}
				newline = false ;
				itp = rexx_if.find( w ) ;
				if ( itp != rexx_if.end() )
				{
					switch ( itp->second )
					{
					case RX_IF:
						h.hl_if = RX_IF ;
						h.hl_istack.push( RX_IF ) ;
						++h.hl_oIf ;
						if ( !h.hl_DoIf.empty() )
						{
							++h.hl_DoIf.top() ;
						}
						shadow.replace( j, 2, 2, ( h.hl_oIf % 5 + 7 ) ) ;
						break ;

					case RX_THEN:
						shadow.replace( j, 4, 4, N_GREEN ) ;
						if ( h.hl_if == RX_IF )
						{
							h.hl_if = RX_THEN ;
							h.hl_istack.push( RX_THEN ) ;
							if ( p1 == ln ) { break ; }
							t  = upper( line.substr( p1 ) ) ;
							p2 = t.find( ';' ) ;
							if ( p2 != string::npos )
							{
								t.insert( p2, 1, ' ' ) ;
								t.insert( p2+2, 1, ' ' ) ;
								p3 = wordpos( "ELSE", t ) ;
								if ( p3 > 0 )
								{
									if ( ( wordpos( ";", t ) + 1 ) != p3 )
									{
										addRxxHilight_unwind_if( h ) ;
									}
								}
								else if ( trim( t.erase( 0, p2+2 ) ) != "" )
								{
									addRxxHilight_unwind_if( h ) ;
								}
							}
							else if ( wordpos( "ELSE", t ) > 0 )
							{
								addRxxHilight_unwind_if( h ) ;
							}
						}
						break ;

					case RX_ELSE:
						if ( !h.hl_istack.empty() && h.hl_istack.top() == RX_ELSE )
						{
							addRxxHilight_unwind_if( h ) ;
						}
						h.hl_if = RX_ELSE ;
						h.hl_istack.push( RX_ELSE ) ;
						if ( h.hl_oIf > 0 )
						{
							if ( !h.hl_DoIf.empty() && h.hl_DoIf.top() == 0 )
							{
								shadow.replace( j, 4, 4, R_MAGENTA ) ;
								h.hl_mismatch = true ;
							}
							else
							{
								shadow.replace( j, 4, 4, ( h.hl_oIf % 5 + 7 ) ) ;
							}
						}
						else
						{
							shadow.replace( j, 4, 4, R_MAGENTA ) ;
							h.hl_mismatch = true ;
						}
						break ;

					case RX_DO:
						if ( h.hl_if == RX_THEN )
						{
							h.hl_if = RX_DO1 ;
						}
						else if ( h.hl_if == RX_ELSE )
						{
							h.hl_if = RX_DO2 ;
						}
						else
						{
							h.hl_if = RX_DO3 ;
						}
						h.hl_DoIf.push( 0 ) ;
						h.hl_istack.push( h.hl_if ) ;
						break ;

					case RX_END:
						if ( h.hl_if == RX_DO1 )
						{
							h.hl_if = RX_END1 ;
							h.hl_istack.push( RX_END1 ) ;
						}
						else if ( h.hl_if == RX_DO2 )
						{
							h.hl_if = RX_END2 ;
							h.hl_istack.push( RX_END2 ) ;
							addRxxHilight_unwind_if( h ) ;
							h.hl_if = ( h.hl_istack.empty() ) ? RX_NONE : h.hl_istack.top() ;
						}
						else
						{
							addRxxHilight_unwind_end3( h ) ;
							h.hl_if = ( h.hl_istack.empty() ) ? RX_NONE : h.hl_istack.top() ;
						}
						if ( !h.hl_DoIf.empty() )
						{
							h.hl_DoIf.pop() ;
						}
						break ;

					case RX_SELECT:
						h.hl_if = RX_DO3 ;
						h.hl_istack.push( RX_DO3 ) ;
						h.hl_DoIf.push( 0 ) ;
						break ;

					case RX_OTHERWISE:
						shadow.replace( j, 9, 9, N_GREEN ) ;
						break ;

					case RX_WHEN:
						shadow.replace( j, 4, 4, N_GREEN ) ;
						break ;

					}
				}
				else if ( h.hl_if == RX_THEN )
				{
					h.hl_istack.push( RX_INSTR1 ) ;
				}
				else if ( h.hl_if == RX_ELSE )
				{
					h.hl_istack.push( RX_INSTR2 ) ;
					addRxxHilight_unwind_if( h ) ;
					h.hl_if = ( h.hl_istack.empty() ) ? RX_NONE : h.hl_istack.top() ;
				}
			}
			if ( h.hl_doLogic )
			{
				if ( w == "DO" )
				{
					++h.hl_oDo ;
					shadow.replace( j, 2, 2, ( h.hl_oDo % 5 + 11 ) ) ;
				}
				else if ( w == "SELECT" )
				{
					++h.hl_oDo ;
					shadow.replace( j, 6, 6, ( h.hl_oDo % 5 + 11 ) ) ;
				}
				else if ( w == "END" )
				{
					if ( h.hl_oDo > 0 )
					{
						shadow.replace( j, 3, 3, ( h.hl_oDo % 5 + 11 ) ) ;
						--h.hl_oDo ;
					}
					else
					{
						shadow.replace( j, 3, 3, R_MAGENTA ) ;
						h.hl_mismatch = true ;
					}
				}
			}
			j = p1 - 1 ;
			continue ;
		}
		if ( h.hl_Paren )
		{
			if ( line[ j ] == '(' ) { ++oBrac1 ; shadow[ j ] = oBrac1 % 5 + 11 ; continue ; }
			if ( line[ j ] == ')' )
			{
				if ( oBrac1 == 0 ) { shadow[ j ] = R_MAGENTA ; h.hl_mismatch = true ; }
				else               { shadow[ j ] = oBrac1 % 5 + 11 ; --oBrac1 ; }
				continue ;
			}
		}
	}

	h.hl_oBrac1   = oBrac1 ;
	h.hl_oComment = oComment ;
	p1            = line.find_last_not_of( ' ' ) ;
	h.hl_continue = ( p1 != string::npos && line[ p1 ] == ',' ) ;
}


void addRxxHilight_unwind_if( hilight& h )
{
	//
	// Unwind the instruction-stack for completed 'if' statements.
	//

	while ( !h.hl_istack.empty() )
	{
		if ( h.hl_istack.top() == RX_IF )
		{
			if ( h.hl_oIf > 0 )
			{
				--h.hl_oIf ;
				if ( !h.hl_DoIf.empty() && h.hl_DoIf.top() > 0 )
				{
					--h.hl_DoIf.top() ;
				}
			}
			h.hl_istack.pop() ;
			if ( !h.hl_istack.empty() && h.hl_istack.top() == RX_ELSE )
			{
				addRxxHilight_unwind_if( h ) ;
			}
			break ;
		}
		h.hl_istack.pop() ;
	}

	if ( !h.hl_istack.empty() && h.hl_istack.top() == RX_THEN )
	{
		h.hl_istack.push( RX_INSTR1 ) ;
	}
}


void addRxxHilight_unwind_end3( hilight& h )
{
	//
	// Unwind the instruction-stack for completed 'do...end' statements.
	//

	int oDo = 1 ;

	while ( !h.hl_istack.empty() && oDo > 0 )
	{
		if ( h.hl_istack.top() == RX_DO3 )
		{
			--oDo ;
		}
		else if ( h.hl_istack.top() == RX_END3 )
		{
			++oDo ;
		}
		h.hl_istack.pop() ;
	}
}


void addOthHilight( hilight& h,
		    const string& line,
		    string& shadow )
{
	//
	// Highlight as a pseudo-PL/1 language.
	// Special characters (yellow) must also be in the delims list.
	//

	uint ln ;
	uint start ;
	uint j  ;

	size_t p1 ;

	string w ;

	const string delims( " +-*/=<>()&|:\"'" ) ;

	bool oQuote = false ;
	char Quote ;

	map<string, keyw>::iterator it ;

	int  oBrac1   = h.hl_oBrac1   ;
	bool oComment = h.hl_oComment ;

	ln = line.size() ;
	if ( ln == 0 ) { shadow = "" ; return ; }

	shadow = string( ln, H_DEFAULT ) ;

	start  = 0 ;

	for ( j = 0 ; j < ln ; ++j )
	{
		if ( !oQuote && ln > 1 && j < ln-1 )
		{
			if ( line.compare( j, 2, "/*" ) == 0 )
			{
				oComment = true ;
				shadow.replace( j, 2, 2, H_COMMENTS ) ;
				++j ;
				continue ;
			}
			if ( oComment && line.compare( j, 2, "*/" ) == 0 )
			{
				oComment = false ;
				shadow.replace( j, 2, 2, H_COMMENTS ) ;
				++j ;
				continue ;
			}
		}

		if ( oComment )
		{
			shadow[ j ] = H_COMMENTS ;
			continue ;
		}

		if ( !oQuote )
		{
			if ( line[ j ] == ' '  ) { continue ; }
			if ( line[ j ] == '"' || line[ j ] == '\'' )
			{
				oQuote = true ;
				Quote  = line[ j ] ;
				start  = j ;
				continue   ;
			}
		}
		else if ( line[ j ] == Quote )
		{
			oQuote = false ;
			shadow.replace( start, j-start+1, j-start+1, H_QUOTED ) ;
			continue ;
		}
		if ( oQuote )
		{
			if ( j == ln-1 )
			{
				shadow.replace( start, j-start+1, j-start+1, R_WHITE ) ;
			}
			continue ;
		}
		if ( h.hl_specials.find( line[ j ] ) != string::npos )
		{
			shadow[ j ] = H_SPECIAL ;
			continue ;
		}
		p1 = line.find_first_of( delims, j ) ;
		if ( p1 != j || p1 == string::npos )
		{
			if ( p1 == string::npos ) { p1 = ln ; }
			w     = upper( line.substr( j, p1-j ) ) ;
			it    = kw_other.find( w ) ;
			if ( it != kw_other.end() )
			{
				shadow.replace( j, it->second.kw_len, it->second.kw_len, it->second.kw_col ) ;
			}
			if ( h.hl_ifLogic )
			{
				if ( w == "IF" )
				{
					++h.hl_oIf ;
				}
				else if ( w == "ELSE" )
				{
					if ( h.hl_oIf == 0 )
					{
						shadow.replace( j, 4, 4, R_MAGENTA ) ;
						h.hl_mismatch = true ;
					}
					else if ( h.hl_oIf > 0 )
					{
						--h.hl_oIf ;
					}
				}
			}
			if ( h.hl_doLogic )
			{
				if ( w == "DO" )
				{
					++h.hl_oDo ;
				}
				else if ( w == "END" )
				{
					if ( h.hl_oDo == 0 )
					{
						shadow.replace( j, 3, 3, R_MAGENTA ) ;
						h.hl_mismatch = true ;
					}
					else
					{
						--h.hl_oDo ;
					}
				}
			}
			j = p1 - 1 ;
			continue ;
		}
		if ( h.hl_Paren )
		{
			if ( line[ j ] == '(' ) { ++oBrac1 ; shadow[ j ] = oBrac1 % 5 + 11 ; continue ; }
			if ( line[ j ] == ')' )
			{
				if ( oBrac1 == 0 ) { shadow[ j ] = R_MAGENTA ; h.hl_mismatch = true ; }
				else               { shadow[ j ] = oBrac1 % 5 + 11 ; --oBrac1 ; }
				continue ;
			}
		}
	}
	h.hl_oBrac1   = oBrac1 ;
	h.hl_oComment = oComment ;
}


void addDefHilight( hilight& h,
		    const string& line,
		    string& shadow )
{
	//
	// Highlight in a single colour, green.
	//

	size_t ln ;

	ln = line.size() ;
	if ( ln == 0 ) { shadow = "" ; return ; }

	shadow = string( ln, H_DEFAULT ) ;
}


void addPanHilight( hilight& h,
		    const string& line,
		    string& shadow )
{
	//
	// Special characters (yellow) must also be in the delims list.
	//

	uint start ;
	uint j ;

	size_t ln ;
	size_t p1 ;

	string w ;

	const string delims( " =,&.()=<>!+-*|\"'" ) ;

	bool oQuote = false ;
	char Quote ;

	map<string, keyw>::iterator it ;

	int  oBrac1   = h.hl_oBrac1   ;
	bool oComment = h.hl_oComment ;

	ln = line.size() ;
	if ( ln == 0 ) { shadow = "" ; return ; }

	shadow = string( ln, H_DEFAULT ) ;

	for ( start = 0, j = 0 ; j < ln ; ++j )
	{
		if ( !oQuote && ln > 1 && j < ln-1 )
		{
			if ( line.compare( j, 2, "/*" ) == 0 )
			{
				shadow.replace( j, ln-j, ln-j, H_COMMENTS ) ;
				break ;
			}
			if ( oComment && j == 0 && upper(line).compare( 0, 11, ")ENDCOMMENT" ) == 0 )
			{
				oComment = false ;
				shadow.replace( 0, 11, 11, H_COMMENTS ) ;
				break ;
			}
			if ( !oComment && ( line.compare( j, 2, "--" ) == 0 ) )
			{
				shadow.replace( j, ln-j, ln-j, H_COMMENTS ) ;
				break ;
			}
		}

		if ( ln > 1 && j < ln-1 && line.compare( j, 2, "\\'" ) == 0 )
		{
			++j ;
			continue ;
		}

		if ( oComment )
		{
			shadow[ j ] = H_COMMENTS ;
			continue ;
		}

		if ( !oQuote )
		{
			if ( line[ j ] == ' '  ) { continue ; }
			if ( line[ j ] == '"' || line[ j ] == '\'' )
			{
				oQuote = true ;
				Quote  = line[ j ] ;
				start  = j ;
				continue   ;
			}
		}
		else if ( line[ j ] == Quote )
		{
			oQuote = false ;
			shadow.replace( start, j-start+1, j-start+1, H_QUOTED ) ;
			continue ;
		}
		if ( oQuote )
		{
			if ( j == ln-1 )
			{
				shadow.replace( start, j-start+1, j-start+1, R_WHITE ) ;
			}
			continue ;
		}
		if ( j == 0 && line.front() == ')' )
		{
			w  = upper( word( line, 1 ) ) ;
			if ( w == ")COMMENT" )
			{
				shadow.replace( 0, 8, 8, H_COMMENTS ) ;
				oComment = true ;
				break ;
			}
			it = kw_panel.find( w ) ;
			if ( it != kw_panel.end() )
			{
				shadow.replace( 0, it->second.kw_len, it->second.kw_len, it->second.kw_col ) ;
				j = it->second.kw_len ;
			}
			continue ;
		}
		if ( h.hl_specials.find( line[ j ] ) != string::npos )
		{
			shadow[ j ] = H_SPECIAL ;
			continue ;
		}
		p1 = line.find_first_of( delims, j ) ;
		if ( p1 != j || p1 == string::npos )
		{
			if ( p1 == string::npos ) { p1 = ln ; }
			w     = upper( line.substr( j, p1-j ) ) ;
			it    = kw_panel.find( w ) ;
			if ( it != kw_panel.end() )
			{
				shadow.replace( j, it->second.kw_len, it->second.kw_len, it->second.kw_col ) ;
			}
			if ( h.hl_ifLogic )
			{
				if ( w == "IF" )
				{
					++h.hl_oIf ;
				}
				else if ( w == "ELSE" )
				{
					if ( h.hl_oIf == 0 )
					{
						shadow.replace( j, 4, 4, R_MAGENTA ) ;
						h.hl_mismatch = true ;
					}
					else if ( h.hl_oIf > 0 )
					{
						--h.hl_oIf ;
					}
				}
			}
			j = p1 - 1 ;
			continue ;
		}
		if ( h.hl_Paren )
		{
			if ( line[ j ] == '(' ) { ++oBrac1 ; shadow[ j ] = oBrac1 % 5 + 11 ; continue ; }
			if ( line[ j ] == ')' )
			{
				if ( oBrac1 == 0 ) { shadow[ j ] = R_MAGENTA ; h.hl_mismatch = true ; }
				else               { shadow[ j ] = oBrac1 % 5 + 11 ; --oBrac1 ; }
				continue ;
			}
		}
	}
	h.hl_oBrac1   = oBrac1 ;
	h.hl_oComment = oComment ;
}


void addSklHilight( hilight& h,
		    const string& line,
		    string& shadow )
{
	//
	// Special characters (yellow) must also be in the delims list.
	//

	uint ln ;

	size_t p1 ;

	string w ;

	ln = line.size() ;
	if ( ln == 0 ) { shadow = "" ; return ; }

	shadow.reserve( ln ) ;

	shadow = string( ln, H_DEFAULT ) ;

	if ( line[ 0 ] == ')' )
	{
		w = upper( word( line, 1 ) ) ;
		if ( w == ")CM" )
		{
			shadow.replace( 0, ln, ln, H_COMMENTS ) ;
		}
		else if ( w == ")DO" )
		{
			++h.hl_oDO ;
			shadow.replace( 0, ln, ln, ( h.hl_oDO % 5 + 11 ) ) ;
		}
		else if ( w == ")ENDDO" )
		{
			if ( h.hl_oDO > 0 )
			{
				shadow.replace( 0, ln, ln, ( h.hl_oDO % 5 + 11 ) ) ;
				--h.hl_oDO ;
			}
			else
			{
				shadow.replace( 0, ln, ln, R_MAGENTA ) ;
			}
		}
		else if ( w == ")DOT" )
		{
			++h.hl_oDOT ;
			shadow.replace( 0, ln, ln, ( h.hl_oDOT % 5 + 11 ) ) ;
		}
		else if ( w == ")ENDDOT" )
		{
			if ( h.hl_oDOT > 0 )
			{
				shadow.replace( 0, ln, ln, ( h.hl_oDOT % 5 + 11 ) ) ;
				--h.hl_oDOT ;
			}
			else
			{
				shadow.replace( 0, ln, ln, R_MAGENTA ) ;
			}
		}
		else if ( w == ")SEL" )
		{
			++h.hl_oSEL ;
			shadow.replace( 0, ln, ln, ( h.hl_oSEL % 5 + 11 ) ) ;
		}
		else if ( w == ")ENDSEL" )
		{
			if ( h.hl_oSEL > 0 )
			{
				shadow.replace( 0, ln, ln, ( h.hl_oSEL % 5 + 11 ) ) ;
				--h.hl_oSEL ;
			}
			else
			{
				shadow.replace( 0, ln, ln, R_MAGENTA ) ;
			}
		}
		else if ( w == ")ITERATE" )
		{
			if ( h.hl_oDO > 0 )
			{
				shadow.replace( 0, ln, ln, ( h.hl_oDO % 5 + 11 ) ) ;
			}
			else
			{
				shadow.replace( 0, ln, ln, R_MAGENTA ) ;
			}
		}
		else if ( w == ")LEAVE" )
		{
			if ( upper( word( line, 2 ) ) == "DOT" )
			{
				if ( h.hl_oDOT > 0 )
				{
					shadow.replace( 0, ln, ln, ( h.hl_oDOT % 5 + 11 ) ) ;
				}
				else
				{
					shadow.replace( 0, ln, ln, R_MAGENTA ) ;
				}
			}
			else if ( h.hl_oDO > 0 )
			{
				shadow.replace( 0, ln, ln, ( h.hl_oDO % 5 + 11 ) ) ;
			}
			else
			{
				shadow.replace( 0, ln, ln, R_MAGENTA ) ;
			}
		}
		else
		{
			shadow = string( ln, N_RED ) ;
		}
		p1 = line.find_first_of( h.hl_specials ) ;
		while ( p1 != string::npos )
		{
			shadow[ p1 ] = H_SPECIAL ;
			p1 = line.find_first_of( h.hl_specials, p1+1 ) ;
		}
	}
	else
	{
		shadow = string( ln, N_GREEN ) ;
	}
}


void addCobHilight( hilight& h,
		    const string& line,
		    string& shadow )
{
	//
	// Special characters (yellow) must also be in the delims list.
	//

	uint ln ;
	uint start ;
	uint j  ;

	size_t p1 ;

	string w ;

	const string delims( " .()\"'" ) ;

	bool oQuote = false ;
	char Quote ;

	map<string, keyw>::iterator it ;

	int oBrac1 = h.hl_oBrac1 ;

	ln = line.size() ;
	if ( ln == 0 ) { shadow = "" ; return ; }

	shadow.reserve( ln ) ;

	if ( ln < 7 )
	{
		shadow = string( ln, N_YELLOW ) ;
		return ;
	}
	shadow = string( 6, N_YELLOW ) + string( ln - 6, H_DEFAULT ) ;
	if ( ln == 80 && isnumeric( line.substr( 72 ) ) )
	{
		ln = 72 ;
	}
	else if ( ln > 72 )
	{
		for ( j = 72 ; j < ln ; ++j )
		{
			if ( line[ j ] != ' ' )
			{
				shadow[ j ] = R_RED ;
			}
		}
		ln = 72 ;
	}

	start = 0 ;

	for ( j = 6 ; j < ln ; ++j )
	{
		if ( j == 6 )
		{
			if ( line[ 6 ] == '*' || line[ 6 ] == '/' )
			{
				shadow.replace( 6, ln-6, ln-6, H_COMMENTS ) ;
				break ;
			}
			if ( line[ 6 ] != ' ' )
			{
				shadow[ 6 ] = N_WHITE ;
				continue ;
			}
		}
		if ( !oQuote )
		{
			if ( line[ j ] == ' '  ) { continue ; }
			if ( line[ j ] == '"' || line[ j ] == '\'' )
			{
				oQuote = true ;
				Quote  = line[ j ] ;
				start  = j ;
				continue   ;
			}
		}
		else if ( line[ j ] == Quote )
		{
			oQuote = false ;
			shadow.replace( start, j-start+1, j-start+1, H_QUOTED ) ;
			continue ;
		}
		if ( oQuote )
		{
			if ( j == ln-1 )
			{
				shadow.replace( start, j-start+1, j-start+1, H_QUOTED ) ;
			}
			continue ;
		}
		if ( h.hl_specials.find( line[ j ] ) != string::npos )
		{
			shadow[ j ] = H_SPECIAL ;
			if ( line[ j ] == '.' )
			{
				h.hl_oIf = 0 ;
				h.hl_oDo = 0 ;
			}
			continue ;
		}
		p1 = line.find_first_of( delims, j ) ;
		if ( p1 != j || p1 == string::npos )
		{
			if ( p1 == string::npos ) { p1 = ln ; }
			w     = upper( line.substr( j, p1-j ) ) ;
			it    = kw_cobol.find( w ) ;
			if ( it != kw_cobol.end() )
			{
				shadow.replace( j, it->second.kw_len, it->second.kw_len, it->second.kw_col ) ;
			}
			if ( h.hl_ifLogic )
			{
				if ( w == "IF" )
				{
					++h.hl_oIf ;
				}
				else if ( w == "ELSE" )
				{
					if ( h.hl_oIf == 0 )
					{
						shadow.replace( j, 4, 4, R_MAGENTA ) ;
						h.hl_mismatch = true ;
					}
				}
				else if ( w == "END-IF" )
				{
					if ( h.hl_oIf == 0 )
					{
						shadow.replace( j, 6, 6, R_MAGENTA ) ;
						h.hl_mismatch = true ;
					}
					else if ( h.hl_oIf > 0 )
					{
						--h.hl_oIf ;
					}
				}
			}
			if ( h.hl_doLogic )
			{
				if ( w == "PERFORM" )
				{
					++h.hl_oDo ;
				}
				else if ( w == "END-PERFORM" )
				{
					if ( h.hl_oDo == 0 )
					{
						shadow.replace( j, 11, 11, R_MAGENTA ) ;
						h.hl_mismatch = true ;
					}
					else
					{
						--h.hl_oDo ;
					}
				}
			}
			j = p1 - 1 ;
			continue ;
		}
		if ( h.hl_Paren )
		{
			if ( line[ j ] == '(' ) { ++oBrac1 ; shadow[ j ] = oBrac1 % 5 + 11 ; continue ; }
			if ( line[ j ] == ')' )
			{
				if ( oBrac1 == 0 ) { shadow[ j ] = R_MAGENTA ; h.hl_mismatch = true ; }
				else               { shadow[ j ] = oBrac1 % 5 + 11 ; --oBrac1 ; }
				continue ;
			}
		}
	}
	h.hl_oBrac1 = oBrac1 ;
}


void addJclHilight( hilight& h,
		    const string& line,
		    string& shadow )
{
	uint ln ;
	uint start ;
	uint j  ;
	uint ws ;

	size_t p1 ;
	size_t c1 = 0 ;
	size_t c2 = 0 ;

	string w ;

	const string delims( " (),|<>&='" ) ;

	bool oQuote = false ;
	bool contin = true ;

	map<string, keyw>::iterator it ;

	int  oBrac1 = h.hl_oBrac1 ;

	ln = line.size() ;
	if ( ln == 0 ) { shadow = "" ; return ; }

	shadow = string( ln, H_DEFAULT ) ;
	if ( ln > 72 ) { ln = 72 ; }

	start = 0 ;
	ws    = 0 ;

	for ( j = 0 ; j < ln ; ++j )
	{
		if ( j == 0 && line.compare( 0, 2, "//" ) != 0 && line.compare( 0, 2, "/*" ) != 0 )
		{
			shadow.replace( 0, ln, ln, H_DATA ) ;
			break ;
		}
		if ( j == 0 && ln > 2 && line.compare( 0, 3, "//*" ) == 0 )
		{
			shadow.replace( 0, ln, ln, H_COMMENTS ) ;
			break ;
		}
		if ( !oQuote )
		{
			if ( line[ j ] == '\'' )
			{
				oQuote = true ;
				start  = j ;
				continue   ;
			}
		}
		else if ( line[ j ] == '\'' )
		{
			oQuote = false ;
			shadow.replace( start, j-start+1, j-start+1, H_QUOTED ) ;
			continue ;
		}
		if ( oQuote )
		{
			if ( j == ln-1 )
			{
				shadow.replace( start, j-start+1, j-start+1, H_QUOTED ) ;
			}
			continue ;
		}
		if ( h.hl_specials.find( line[ j ] ) != string::npos )
		{
			shadow[ j ] = H_SPECIAL ;
			continue ;
		}
		p1 = line.find_first_of( delims, j ) ;
		if ( p1 != j || p1 == string::npos )
		{
			if ( p1 == string::npos )
			{
				p1 = ln ;
			}
			w  = upper( line.substr( j, p1-j ) ) ;
			it = kw_jcl.find( w ) ;
			if ( it != kw_jcl.end() )
			{
				shadow.replace( j, it->second.kw_len, it->second.kw_len, it->second.kw_col ) ;
				contin = false ;
			}
			if ( h.hl_ifLogic )
			{
				if ( w == "IF" )
				{
					++h.hl_oIf ;
				}
				else if ( w == "ELSE" )
				{
					if ( h.hl_oIf == 0 )
					{
						shadow.replace( j, 4, 4, R_MAGENTA ) ;
						h.hl_mismatch = true ;
					}
				}
				else if ( w == "ENDIF" )
				{
					if ( h.hl_oIf == 0 )
					{
						shadow.replace( j, 6, 6, R_MAGENTA ) ;
						h.hl_mismatch = true ;
					}
					else if ( h.hl_oIf > 0 )
					{
						--h.hl_oIf ;
					}
				}
			}
			j = p1 - 1 ;
			continue ;
		}
		else if ( line[ j ] == ' ' )
		{
			++ws ;
			while ( j < ln && line[ j ] == ' ' ) { ++j ; }
			if ( j == ln ) { break ; }
			if ( ws == 2 ) { c1 = j ; }
			else if ( ws == 3 ) { c2 = j ; }
			--j ;
		}
		if ( h.hl_Paren )
		{
			if ( line[ j ] == '(' ) { ++oBrac1 ; shadow[ j ] = oBrac1 % 5 + 11 ; continue ; }
			if ( line[ j ] == ')' )
			{
				if ( oBrac1 == 0 ) { shadow[ j ] = R_MAGENTA ; h.hl_mismatch = true ; }
				else               { shadow[ j ] = oBrac1 % 5 + 11 ; --oBrac1 ; }
				continue ;
			}
		}
	}

	if ( ( contin && c1 > 0 ) )
	{
		shadow.replace( c1, ln-c1, ln-c1, H_COMMENTS ) ;
	}
	else if ( ( !contin && c2 > 0 ) )
	{
		shadow.replace( c2, ln-c2, ln-c2, H_COMMENTS ) ;
	}

	h.hl_oBrac1 = oBrac1 ;
}


void addDfuHilight( hilight& h,
		    const string& line,
		    string& shadow )
{
	uint ln = line.size() ;
	if ( ln == 0 ) { shadow = "" ; return ; }

	char c = N_WHITE ;

	string w = word( line, 1 ) ;

	if ( w == "---" || w == "+++" )
	{
		c = N_GREEN ;
	}
	else if ( w == "@@" )
	{
		c = N_YELLOW ;
	}
	else if ( line[ 0 ] == '-' )
	{
		c = N_RED ;
	}
	else if ( line[ 0 ] == '+' )
	{
		c = N_TURQ ;
	}

	shadow = string( ln, c ) ;
}


void addDfcHilight( hilight& h,
		    const string& line,
		    string& shadow )
{
	uint ln = line.size() ;
	if ( ln == 0 ) { shadow = "" ; return ; }

	char c = N_WHITE ;

	string w = word( line, 1 ) ;

	if ( w == "---" || w == "***" )
	{
		c = N_GREEN ;
	}
	else if ( w == "***************" )
	{
		c = N_YELLOW ;
	}
	else if ( line[ 0 ] == '!' )
	{
		c = N_BLUE ;
	}
	else if ( line[ 0 ] == '-' )
	{
		c = N_RED ;
	}
	else if ( line[ 0 ] == '+' )
	{
		c = N_TURQ ;
	}

	shadow = string( ln, c ) ;
}


void addANSHilight( hilight& h,
		    const string& line,
		    string& shadow )
{
	int j ;

	size_t p1 ;
	size_t p2 ;
	size_t p3 ;
	size_t p4 ;

	size_t d1 = 0 ;
	size_t d2 = 0 ;

	string t ;
	string temp ;

	map<int, int>::iterator it ;

	bool bold = false ;
	bool rev  = false ;

	const string ansi_start = "\x1b[" ;
	const char   ansi_end   = 'm' ;
	const char   ansi_delm  = ';' ;
	const string ansi_codes = "0123456789; " ;

	uint ln = line.size() ;
	if ( ln == 0 ) { shadow = "" ; return ; }

	shadow = string( ln, h.hl_defc ) ;

	p1 = line.find( ansi_start ) ;
	while ( p1 != string::npos )
	{
		p2 = line.find_first_not_of( ansi_codes, p1+2 ) ;
		if ( p2 == string::npos ) { break ; }
		if ( line[ p2 ] != ansi_end )
		{
			p1 = line.find( ansi_start, p2 ) ;
			continue ;
		}
		temp = line.substr( p1+2, p2-p1-2 ) ;
		d2   = p2-p1+1 ;
		p3   = temp.find( ansi_delm ) ;
		while ( temp != "" )
		{
			if ( p3 == string::npos ) { p3 = temp.size() ; }
			t  = temp.substr( 0, p3 ) ;
			if ( datatype( t, 'W' ) )
			{
				j = ds2d( t ) ;
				if ( j == 0 )
				{
					bold = false ;
					rev  = false ;
					h.hl_defc = N_WHITE ;
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
						h.hl_defc = it->second ;
					}
				}
			}
			if ( p3 == temp.size() )
			{
				temp = "" ;
				p4   = p1 - d1 ;
				shadow.replace( p4, ln-p4, ln-p4, h.hl_defc ) ;
			}
			else
			{
				temp.erase( 0, p3+1 ) ;
				p3 = temp.find( ansi_delm ) ;
			}
		}
		d1 += d2 ;
		p1 = line.find( ansi_start, p2+1 ) ;
	}
}


void addRstHilight( hilight& h,
		    const string& line,
		    string& shadow )
{
	//
	// Special characters (yellow) must also be in the delims list.
	//

	uint start ;

	size_t j  ;
	size_t ln ;
	size_t p1 ;
	size_t p2 ;

	string w ;
	const string delims( " (){}=;<>+-*/|:!%#[]&,\\\"'" ) ;

	bool oQuote = false ;
	char Quote ;

	map<string, keyw>::iterator it ;

	int  oBrac1   = h.hl_oBrac1   ;
	int  oBrac2   = h.hl_oBrac2   ;
	bool oComment = h.hl_oComment ;

	ln = line.size() ;
	if ( ln == 0 ) { shadow = "" ; return ; }

	shadow = string( ln, H_DEFAULT ) ;

	start = 0 ;

	for ( j = 0 ; j < ln ; ++j )
	{
		if ( !oQuote && ln > 1 && j < ln-1 )
		{
			if ( line.compare( j, 2, "/*" ) == 0 )
			{
				oComment = true ;
				shadow.replace( j, 2, 2, H_COMMENTS ) ;
				++j ;
				continue ;
			}
			if ( oComment && line.compare( j, 2, "*/" ) == 0 )
			{
				oComment = false ;
				shadow.replace( j, 2, 2, H_COMMENTS ) ;
				++j ;
				continue ;
			}
			if ( !oComment && line.compare( j, 2, "//" ) == 0 )
			{
				shadow.replace( j, ln-j, ln-j, H_COMMENTS ) ;
				break ;
			}
		}

		if ( oComment )
		{
			shadow[ j ] = H_COMMENTS ;
			continue ;
		}

		if ( oQuote && ln > 1 && j < ln-1 && ( line.compare( j, 2, "\\\\" ) == 0 ) )
		{
			++j ;
			continue ;
		}

		if ( ln > 1 && j < ln-1 && ( line.compare( j, 2, "\\'" ) == 0 || line.compare( j, 2, "\\\"" ) == 0 ) )
		{
			++j ;
			continue ;
		}

		if ( !oQuote )
		{
			if ( line[ j ] == ' '  ) { continue ; }
			if ( line[ j ] == '"' || line[ j ] == '\'' )
			{
				oQuote = true ;
				Quote  = line[ j ] ;
				start  = j ;
				continue   ;
			}
		}
		else if ( line[ j ] == Quote )
		{
			oQuote = false ;
			shadow.replace( start, j-start+1, j-start+1, H_QUOTED ) ;
			continue ;
		}
		if ( oQuote )
		{
			if ( j == ln-1 )
			{
				shadow.replace( start, j-start+1, j-start+1, R_WHITE ) ;
			}
			continue ;
		}
		if ( j == 0 )
		{
			j = line.find_first_not_of( ' ' ) ;
			if ( j == string::npos )
			{
				break ;
			}
			if ( line[ j ] == '#' )
			{
				p1 = line.find( ' ', j ) ;
				if ( p1 == string::npos )
				{
					shadow.replace( 0, ln, ln, H_COMPDIR ) ;
					break ;
				}
				else if ( p1 == j + 1 )
				{
					p1 = line.find_first_not_of( ' ', p1 + 1 ) ;
					if ( p1 == string::npos )
					{
						shadow.replace( 0, ln, ln, H_COMPDIR ) ;
						break ;
					}
					else
					{
						p1 = line.find( ' ', p1 ) ;
						if ( p1 == string::npos )
						{
							shadow.replace( 0, ln, ln, H_COMPDIR ) ;
							break ;
						}
						shadow.replace( 0, p1, p1, H_COMPDIR ) ;
						j = p1 ;
					}
				}
				else
				{
					shadow.replace( 0, p1, p1, H_COMPDIR ) ;
					j = p1 ;
				}
				continue ;
			}
		}
		if ( line[ j ] == '!' && j > 0 && isalnum( line[ j-1 ] ) )
		{
			p2 = line.find_last_of( ' ', j ) ;
			if ( p2 == string::npos )
			{
				shadow.replace( 0, j+1, j+1, H_MACRO ) ;
			}
			else
			{
				shadow.replace( p2+1, j-p2, j-p2, H_MACRO ) ;
			}
			continue ;
		}
		if ( h.hl_specials.find( line[ j ] ) != string::npos )
		{
			shadow[ j ] = H_SPECIAL ;
			continue ;
		}
		p1 = line.find_first_of( delims, j ) ;
		if ( p1 != j || p1 == string::npos )
		{
			if ( p1 == string::npos ) { p1 = ln ; }
			w  = line.substr( j, p1-j ) ;
			it = kw_rust.find( w ) ;
			if ( it != kw_rust.end() )
			{
				shadow.replace( j, it->second.kw_len, it->second.kw_len, it->second.kw_col ) ;
			}
			if ( h.hl_ifLogic )
			{
				if ( w == "if" )
				{
					++h.hl_oIf ;
				}
				else if ( w == "else" )
				{
					if ( h.hl_oIf == 0 )
					{
						shadow.replace( j, 4, 4, R_MAGENTA ) ;
						h.hl_mismatch = true ;
					}
					else if ( h.hl_oIf > 0 )
					{
						--h.hl_oIf ;
					}
				}
			}
			j = p1 - 1 ;
			continue   ;
		}
		if ( h.hl_Paren )
		{
			if ( line[ j ] == '(' ) { ++oBrac1 ; shadow[ j ] = oBrac1 % 5 + 11 ; continue ; }
			if ( line[ j ] == ')' )
			{
				if ( oBrac1 == 0 ) { shadow[ j ] = R_MAGENTA ; h.hl_mismatch = true ; }
				else               { shadow[ j ] = oBrac1 % 5 + 11 ; --oBrac1 ; }
				continue ;
			}
		}
		if ( h.hl_doLogic )
		{
			if ( line[ j ] == '{' )
			{
				++oBrac2 ;
				shadow[ j ] = oBrac2 % 5 + 11 ;
				continue ;
			}
			else if ( line[ j ] == '}' )
			{
				if ( oBrac2 == 0 )
				{
					shadow[ j ]   = R_MAGENTA ;
					h.hl_mismatch = true ;
				}
				else if ( oBrac2 > 0 )
				{
					shadow[ j ] = oBrac2 % 5 + 11 ;
					--oBrac2 ;
					if ( oBrac2 == 0 ) { h.hl_oIf = 0 ; }
				}
				continue ;
			}
		}
	}
	h.hl_oBrac1   = oBrac1 ;
	h.hl_oBrac2   = oBrac2 ;
	h.hl_oComment = oComment ;
}


void addTmlHilight( hilight& h,
		    const string& line,
		    string& shadow )
{
	//
	// Special characters (yellow) must also be in the delims list.
	//

	uint start ;

	size_t j  ;
	size_t ln ;
	size_t p1 ;

	bool kword = true ;
	bool header ;

	int oBrac  = 0 ;
	int oBrac1 = h.hl_oBrac1 ;
	int oBrac2 = h.hl_oBrac2 ;

	string w ;
	const string delims( " =." ) ;

	bool oQuote = false ;
	char Quote ;

	map<string, keyw>::iterator it ;

	ln = line.size() ;
	if ( ln == 0 ) { shadow = "" ; return ; }

	shadow = string( ln, H_DEFAULT ) ;

	start = 0 ;

	p1 = line.find_first_not_of( ' ' ) ;
	if ( p1 == string::npos ) { return ; }

	header = ( line[ p1 ] == '[' ) ;

	for ( j = p1 ; j < ln ; ++j )
	{
		if ( !oQuote )
		{
			if ( line[ j ] == '#' )
			{
				shadow.replace( j, ln-j, ln-j, H_COMMENTS ) ;
				break ;
			}
			if ( line[ j ] == ' '  ) { continue ; }
			if ( line[ j ] == '"' || line[ j ] == '\'' )
			{
				oQuote = true ;
				Quote  = line[ j ] ;
				start  = j ;
				continue ;
			}
		}
		else if ( line[ j ] == Quote )
		{
			oQuote = false ;
			shadow.replace( start, j-start+1, j-start+1, H_QUOTED ) ;
			continue ;
		}
		if ( oQuote )
		{
			if ( j == ln-1 )
			{
				shadow.replace( start, j-start+1, j-start+1, R_WHITE ) ;
			}
			continue ;
		}
		if ( header )
		{
			if ( line[ j ] == '[' )
			{
				++oBrac ;
				shadow[ j ] = H_HEADER ;
			}
			else if ( line[ j ] == ']' )
			{
				if ( oBrac == 0 )
				{
					shadow[ j ]   = R_MAGENTA ;
					h.hl_mismatch = true ;
				}
				else
				{
					--oBrac ;
					shadow[ j ] = H_HEADER ;
				}
			}
			else if ( oBrac > 0 )
			{
				shadow[ j ] = H_HEADER ;
			}
			continue ;
		}
		if ( kword && line[ j ] == '=' )
		{
			shadow.replace( 0, j, j, H_KEYWORDS ) ;
			shadow[ j ] = H_SPECIAL ;
			kword       = false ;
			continue ;
		}
		if ( h.hl_specials.find( line[ j ] ) != string::npos )
		{
			shadow[ j ] = H_SPECIAL ;
			continue ;
		}
		p1 = line.find_first_of( delims, j ) ;
		if ( p1 != j || p1 == string::npos )
		{
			if ( p1 == string::npos ) { p1 = ln ; }
			w  = line.substr( j, p1-j ) ;
			it = kw_toml.find( w ) ;
			if ( it != kw_toml.end() )
			{
				shadow.replace( j, it->second.kw_len, it->second.kw_len, it->second.kw_col ) ;
			}
		}
		if ( line[ j ] == '[' )
		{
			++oBrac1 ;
			shadow[ j ] = oBrac1 % 5 + 11 ;
			continue ;
		}
		else if ( line[ j ] == ']' )
		{
			if ( oBrac1 == 0 )
			{
				shadow[ j ]   = R_MAGENTA ;
				h.hl_mismatch = true ;
			}
			else if ( oBrac1 > 0 )
			{
				shadow[ j ] = oBrac1 % 5 + 11 ;
				--oBrac1 ;
			}
			continue ;
		}
		if ( line[ j ] == '{' )
		{
			++oBrac2 ;
			shadow[ j ] = oBrac2 % 5 + 11 ;
			continue ;
		}
		else if ( line[ j ] == '}' )
		{
			if ( oBrac2 == 0 )
			{
				shadow[ j ]   = R_MAGENTA ;
				h.hl_mismatch = true ;
			}
			else if ( oBrac2 > 0 )
			{
				shadow[ j ] = oBrac2 % 5 + 11 ;
				--oBrac2 ;
			}
			continue ;
		}
	}

	h.hl_oBrac1 = oBrac1 ;
	h.hl_oBrac2 = oBrac2 ;
}
