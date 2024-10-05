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

using namespace boost::filesystem ;

/* ************************************ ***************************** ********************************* */
/* ************************************         PARSER class          ********************************* */
/* ************************************ ***************************** ********************************* */


void parser::parse( errblock& err,
		    const string& s )
{
	bool quoted ;
	char c1 ;

	string u ;

	token t ;

	string::const_iterator it ;

	tokens.clear() ;

	err.setRC( 0 ) ;
	it  = s.begin() ;
	idx = 0 ;

	while ( it != s.end() )
	{
		getNextString( err, it, s, u, quoted ) ;
		if ( err.error() ) { return ; }
		if ( u == "" )
		{
			if ( !quoted ) { break ; }
		}
		else
		{
			c1 = u.front() ;
		}
		t.clear() ;
		if ( quoted )
		{
			t.type   = TT_STRING_QUOTED ;
			t.value1 = u ;
		}
		else if ( compare_ops.count( u ) > 0 )
		{
			t.subtype = TS_COMPARISON_OP ;
			t.value1  = u ;
		}
		else if ( c1 == '&' && u.size() > 1 )
		{
			if ( optUpper ) { iupper( u ) ; }
			if ( isvalidName( u.substr( 1 ) ) )
			{
				t.type    = TT_VARIABLE ;
				t.subtype = TS_AMPR_VAR_VALID ;
			}
			else
			{
				t.subtype = TS_AMPR_VAR_INVALID ;
			}
			t.value1 = u ;
		}
		else if ( c1 == '.' && u.size() > 1 )
		{
			if ( optUpper ) { iupper( u ) ; }
			if ( ctl_valid.find( u ) != ctl_valid.end() )
			{
				t.type    = TT_VARIABLE ;
				t.subtype = TS_CTL_VAR_VALID ;
			}
			else
			{
				t.subtype = TS_CTL_VAR_INVALID ;
			}
			t.value1 = u ;
		}
		else if ( c1 == '(' )
		{
			t.subtype = TS_OPEN_BRACKET ;
			t.value1  = "(" ;
		}
		else if ( c1 == ')' )
		{
			t.subtype = TS_CLOSE_BRACKET ;
			t.value1  = ")" ;
		}
		else if ( c1 == ',' )
		{
			t.subtype = TS_COMMA ;
			t.value1  = "," ;
		}
		else if ( u == "=" )
		{
			t.subtype = TS_EQUALS ;
			t.value1  = "=" ;
		}
		else
		{
			t.value2 = u ;
			if ( optUpper ) { iupper( u ) ; }
			if ( isvalidName( u ) )
			{
				t.subtype = TS_NAME ;
			}
			t.type   = TT_STRING_UNQUOTED ;
			t.value1 = u ;
		}
		t.idx = tokens.size() ;
		tokens.push_back( t ) ;
	}

	current_token = ( tokens.size() > 0 ) ? tokens[ 0 ] : token( TT_EOT ) ;
}


int parser::getEntries()
{
	return tokens.size() ;
}


void parser::eraseTokens( int i )
{
	if ( i == -1 )
	{
		tokens.clear() ;
		return ;
	}

	tokens.erase( tokens.begin(), tokens.begin() + i + 1 ) ;

	for ( unsigned int j = 0 ; j < tokens.size() ; ++j )
	{
		tokens[ j ].idx = j ;
	}
}


token parser::getToken( unsigned int i )
{
	return ( i < tokens.size() ) ? tokens[ i ] : token( TT_EOT ) ;
}


token& parser::getFirstToken()
{
	idx = 0 ;

	current_token = ( tokens.size() > 0 ) ? tokens[ 0 ] : token( TT_EOT ) ;

	return current_token ;
}


token& parser::getNextToken()
{
	++idx ;

	current_token = ( tokens.size() > idx ) ? tokens[ idx ] : token( TT_EOT ) ;

	return current_token ;
}


string parser::peekNextValue()
{
	unsigned int i = idx + 1 ;

	return ( tokens.size() > i ) ? tokens[ i ].value1 : "" ;
}


TOKEN_SUBTYPES parser::peekNextsubType()
{
	unsigned int i = idx + 1 ;

	return ( tokens.size() > i ) ? tokens[ i ].subtype : TS_NONE ;
}


token& parser::getCurrentToken()
{
	return current_token ;
}


const string& parser::getCurrentValue()
{
	return current_token.value1 ;
}


bool parser::isCurrentType( TOKEN_TYPES t )
{
	return ( t == current_token.type ) ;
}


bool parser::isCurrentSubType( TOKEN_SUBTYPES t )
{
	return ( t == current_token.subtype ) ;
}


bool parser::getNextIfCurrent( const string& tok )
{
	if ( current_token.value1 == tok )
	{
		++idx ;
		current_token = ( idx < tokens.size() ) ? tokens[ idx ] : token( TT_EOT ) ;
		return true ;
	}

	return false ;
}


bool parser::getNextIfCurrent( TOKEN_TYPES tok )
{
	if ( current_token.type == tok )
	{
		++idx ;
		current_token = ( idx < tokens.size() ) ? tokens[ idx ] : token( TT_EOT ) ;
		return true ;
	}

	return false ;
}


bool parser::getNextIfCurrent( TOKEN_SUBTYPES tok )
{
	if ( current_token.subtype == tok )
	{
		++idx ;
		current_token = ( idx < tokens.size() ) ? tokens[ idx ] : token( TT_EOT ) ;
		return true ;
	}

	return false ;
}


STATEMENT_TYPE parser::getStatementType()
{
	//
	// Return the panel language statement type, ST_EOF if empty or ST_ERROR if not recognised.
	//

	token t = getToken( 0 ) ;

	if ( t.type == TT_STRING_QUOTED ) { return ST_ERROR ; }

	auto it = statement_types.find( t.value1 ) ;
	if ( it != statement_types.end() )
	{
		return it->second ;
	}

	if ( tokens.size() < 2 ) { return ST_ERROR ; }

	if ( tokens[ 1 ].subtype == TS_EQUALS ) { return ST_ASSIGN ; }

	return ST_ERROR ;
}



void parser::getNextString( errblock& err,
			    string::const_iterator& it,
			    const string& s,
			    string& r,
			    bool& quoted )
{
	//
	// Get the next word in string s and place in r.
	// Words can be delimited by spaces, single quotes or special chars "()=,<>!+|;:" (also returned as words).
	// Two single quotes within a quoted string are reduced to a single quote.
	//

	int i ;
	int j ;
	int quotes ;

	const string delims = " (),=<>!|;:" ;
	const string compar = "=<>!" ;

	string::const_iterator its ;

	err.setRC( 0 ) ;

	quoted = false ;
	r      = ""    ;

	while ( it != s.end() && *it == ' ' ) { ++it ; }
	if ( it == s.end() )
	{
		return ;
	}

	if ( compar.find( *it ) != string::npos )
	{
		its = it ;
		++it ;
		while ( it != s.end() && ( compar.find( *it ) != string::npos ) ) { ++it ; }
		r.assign( its, it ) ;
	}
	else if ( delims.find( *it ) != string::npos )
	{
		r = *it ;
		++it ;
	}
	else if ( *it == '\'' )
	{
		++it ;
		its = it ;
		if ( its != s.end() && *its == '\'' )
		{
			++its ;
			if ( its == s.end() || delims.find( *its ) != string::npos )
			{
				++it ;
				quoted = true ;
				return ;
			}
		}
		its = it ;
		while ( it != s.end() )
		{
			quotes = 0 ;
			while ( it != s.end() && *it == '\'' ) { ++quotes ; ++it ; }
			if ( quotes == 0 && it != s.end() )
			{
				r.push_back( *it ) ;
				++it ;
				continue ;
			}
			i = quotes / 2 ;
			j = quotes % 2 ;
			r = r + string( i, '\'' ) ;
			if ( j == 1 )
			{
				if ( it != s.end() && delims.find( *it ) == string::npos )
				{
					err.seterrid( TRACE_INFO(), "PSYE037G" ) ;
					return ;
				}
				break ;
			}
			else if ( j == 0 && it == s.end() )
			{
				err.seterrid( TRACE_INFO(), "PSYE033F" ) ;
				return ;
			}
		}
		quoted = true ;
	}
	else
	{
		its = it ;
		while ( it != s.end() && ( delims.find( *it ) == string::npos ) ) { ++it ; }
		r.assign( its, it ) ;
	}
}


void parser::getNameList( errblock& err,
			  string& r )
{
	//
	// Return a list of valid variable names (subtype TS_NAME or TS_AMPR_VAR_VALID)
	// separated by spaces or commas and between brackets or a single entry.
	//

	err.setRC( 0 ) ;

	if ( getNextIfCurrent( TS_OPEN_BRACKET ) )
	{
		if ( getNextIfCurrent( TS_CLOSE_BRACKET ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
			return ;
		}
		while ( true )
		{
			if ( getNextIfCurrent( TS_COMMA ) ) { continue ; }
			if ( getNextIfCurrent( TS_CLOSE_BRACKET ) ) { break ; }
			if ( current_token.subtype == TS_NAME ||
			     current_token.subtype == TS_AMPR_VAR_VALID )
			{
				r += " " + current_token.value1 ;
				getNextToken() ;
			}
			else if ( current_token.type == TT_EOT )
			{
				err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
				return ;
			}
			else
			{
				err.seterrid( TRACE_INFO(), "PSYE031D", current_token.value1 ) ;
				return ;
			}
		}
	}
	else if ( current_token.subtype == TS_NAME ||
		  current_token.subtype == TS_AMPR_VAR_VALID )
	{
		r = current_token.value1 ;
		getNextToken() ;
	}
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE031D", current_token.value1 ) ;
		return ;
	}
}


/* ************************************ ***************************** ********************************* */
/* ************************************           Lexer class         ********************************* */
/* ************************************ ***************************** ********************************* */


void Lexer::advance()
{
	if ( ++pos >= text.size() )
	{
		current_token = ttoken() ;
		return ;
	}

	current_char = text[ pos ] ;
}


void Lexer::skip_whitespace()
{
	while ( current_token.type != tEOF && isspace( current_char ) )
	{
		advance() ;
	}
}


string Lexer::str()
{
	string result ;

	while ( current_token.type != tEOF )
	{
		result += current_char ;
		advance() ;
	}

	return result ;
}


string Lexer::integer()
{
	string result ;

	while ( current_token.type != tEOF && isdigit( current_char ) )
	{
		result += current_char ;
		advance() ;
	}

	return result ;
}


ttoken Lexer::get_next_token( errblock& err )
{
	if ( pos >= text.size() )
	{
		return ttoken() ;
	}

	current_char = text[ pos ] ;

	while ( current_token.type != tEOF )
	{
		if ( isspace( current_char ) )
		{
			skip_whitespace() ;
			continue ;
		}
		else if ( isdigit( current_char ) )
		{
			return ttoken( tINTEGER, integer() ) ;
		}
		else if ( current_char == '+' )
		{
			advance() ;
			return ttoken( tPLUS, "+" ) ;
		}
		else if ( current_char == '-' )
		{
			advance() ;
			return ttoken( tMINUS, "-" ) ;
		}
		else if ( current_char == '*' )
		{
			advance() ;
			return ttoken( tMUL, "*" ) ;
		}
		else if ( current_char == '/' )
		{
			advance() ;
			return ttoken( tDIV, "/" ) ;
		}
		else if ( current_char == '(' )
		{
			advance() ;
			return ttoken( tLPAREN, "(" ) ;
		}
		else if ( current_char == ')' )
		{
			advance() ;
			return ttoken( tRPAREN, ")" ) ;
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE028A", str() ) ;
			return ttoken() ;
		}
	}

	return ttoken() ;
}


/* ************************************ ***************************** ********************************* */
/* ************************************       Interpreter class       ********************************* */
/* ************************************ ***************************** ********************************* */


void Interpreter::eat( errblock& err,
		       tTOKEN tok )
{
	if ( current_token.type == tok )
	{
		current_token = lexer.get_next_token( err ) ;
	}
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE028B" ) ;
	}
}


int Interpreter::factor( errblock& err )
{
	int result ;

	ttoken t = current_token ;

	if ( t.type == tINTEGER )
	{
		eat( err, tINTEGER ) ;
		if ( err.error() ) { return 0 ; }
		return ds2d( t.value ) ;
	}
	else if ( t.type == tLPAREN )
	{
		eat( err, tLPAREN ) ;
		if ( err.error() ) { return 0 ; }
		result = expr( err ) ;
		if ( err.error() ) { return 0 ; }
		eat( err, tRPAREN ) ;
		if ( err.error() ) { return 0 ; }
		return result ;
	}

	return 0 ;
}


int Interpreter::term( errblock& err )
{
	int result ;
	ttoken t   ;

	result = factor( err ) ;
	if ( err.error() ) { return 0 ; }

	while ( current_token.type == tMUL || current_token.type == tDIV )
	{
		t = current_token ;
		if ( t.type == tMUL )
		{
			eat( err, tMUL ) ;
			if ( err.error() ) { return 0 ; }
			result = result * factor( err ) ;
			if ( err.error() ) { return 0 ; }
		}
		else if ( t.type == tDIV )
		{
			eat( err, tDIV ) ;
			if ( err.error() ) { return 0 ; }
			result = result / factor( err ) ;
			if ( err.error() ) { return 0 ; }
		}
	}

	return result ;
}


int Interpreter::expr( errblock& err )
{
	int result ;
	ttoken t ;

	result = term( err ) ;
	if ( err.error() ) { return 0 ; }

	while ( current_token.type == tPLUS || current_token.type == tMINUS )
	{
		t = current_token ;
		if ( t.type == tPLUS )
		{
			eat( err, tPLUS ) ;
			if ( err.error() ) { return 0 ; }
			result = result + term( err ) ;
			if ( err.error() ) { return 0 ; }
		}
		else if ( t.type == tMINUS )
		{
			eat( err, tMINUS ) ;
			if ( err.error() ) { return 0 ; }
			result = result - term( err ) ;
			if ( err.error() ) { return 0 ; }
		}
	}

	return result ;
}


/* ************************************ ***************************** ********************************* */
/* ************************************      IF Statement class       ********************************* */
/* ************************************ ***************************** ********************************* */


void IFSTMNT::parse( errblock& err,
		     parser& v )
{
	//
	// Format of the IF panel statement.
	//
	// IF ( COND ) opt-statement.
	//

	token t ;

	v.getFirstToken() ;
	v.getNextToken()  ;

	err.setRC( 0 ) ;

	if ( !v.getNextIfCurrent( TS_OPEN_BRACKET ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE033D" ) ;
		return ;
	}

	parse_cond( err, v ) ;
	if ( err.error() ) { return ; }

	if ( if_lhs.value == ""      &&
		if_trunc  == nullptr &&
		if_trans  == nullptr &&
		if_verify == nullptr )
	{
		err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
		return ;
	}

	t = v.getCurrentToken()  ;
	if ( t.type == TT_EOT )
	{
		err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
		return ;
	}
	else if ( t.subtype != TS_CLOSE_BRACKET )
	{
		err.seterrid( TRACE_INFO(), "PSYE032Z", t.value1 ) ;
		return ;
	}

	v.eraseTokens( t.idx ) ;
	v.getFirstToken()      ;
}


void IFSTMNT::parse_cond( errblock& err,
			  parser& v )
{
	//
	// Format of the IF-condition panel statement.
	//
	// &AAA=&BBBB
	// &AAA = VALUE1,VALUE2
	// &AAA NE 'Hello','Goodbye'
	// &AAA=&BBBB & &CCC = DDDD AND &EEEE = FFFFF
	// &AAA=&BBBB | &CCC = DDDD OR  &EEEE = FFFFF
	// .CURSOR=ZCMD
	// VER (&A...) | &A = 'CAN'
	// TRUNC(...) = 'VAL1','VAL2' AND VER(...)
	// LENGTH(ABC) = 'VAL1','VAL2' AND VER(...)
	// fn(ABC) = 'VAL1','VAL2' AND VER(...)
	// EXISTS(FILENAME)
	//
	// If comparison is optional ( DIR(), FILE(), EXISTS() ), default to 'EQ .TRUE' ).
	//

	bool comp_opt = false ;

	token t ;
	vparm p ;

	map<string, RL_COND> if_conds =
	{ { "=",  RL_EQ },
	  { "EQ", RL_EQ },
	  { "!=", RL_NE },
	  { "NE", RL_NE },
	  { ">",  RL_GT },
	  { "GT", RL_GT },
	  { "<",  RL_LT },
	  { "LT", RL_LT },
	  { ">=", RL_GE },
	  { "GE", RL_GE },
	  { "!<", RL_GE },
	  { "NL", RL_GE },
	  { "<=", RL_LE },
	  { "LE", RL_LE },
	  { "!>", RL_LE },
	  { "NG", RL_LE } } ;

	map<string, PN_FUNCTION> if_functions =
	{ { "DIR",     PN_DIR     },
	  { "EXISTS",  PN_EXISTS  },
	  { "FILE",    PN_FILE    },
	  { "LENGTH",  PN_LENGTH  },
	  { "LVLINE",  PN_LVLINE  },
	  { "PFK",     PN_PFK     },
	  { "TRANS",   PN_TRANS   },
	  { "TRUNC",   PN_TRUNC   },
	  { "REVERSE", PN_REVERSE },
	  { "WORDS",   PN_WORDS   },
	  { "UPPER",   PN_UPPER   } } ;

	t = v.getCurrentToken() ;
	if ( t.subtype == TS_CLOSE_BRACKET )
	{
		err.seterrid( TRACE_INFO(), "PSYE031J" ) ;
		return ;
	}
	else if ( t.type == TT_EOT )
	{
		err.seterrid( TRACE_INFO(), "PSYE031K", "Missing variable" ) ;
		return ;
	}

	if ( v.peekNextValue() == "(" )
	{
		if ( t.value1 == "VER" )
		{
			if_verify = new VERIFY ;
			v.eraseTokens( t.idx - 1 ) ;
			if_verify->parse( err, v, false ) ;
			if ( err.error() ) { return ; }
			if ( if_verify->ver_msgid != "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE033A", "VERIFY" ) ;
				return ;
			}
			parse_cond_continue( err, v ) ;
			return ;
		}
		auto it = if_functions.find( t.value1 ) ;
		if ( it == if_functions.end() )
		{
			err.seterrid( TRACE_INFO(), "PSYE033U", t.value1 ) ;
			return ;
		}
		if_func = it->second ;
		switch ( if_func )
		{
		case PN_TRUNC:
			if_trunc = new TRUNC ;
			if_trunc->parse( err, v, false ) ;
			if ( err.error() ) { return ; }
			break ;

		case PN_TRANS:
			if_trans = new TRANS ;
			if_trans->parse( err, v, false ) ;
			if ( err.error() ) { return ; }
			if ( if_trans->trns_msgid != "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE033A", "TRANS" ) ;
				return ;
			}
			break ;

		case PN_PFK:
			v.getNextToken() ;
			t = v.getNextToken() ;
			if ( t.subtype == TS_AMPR_VAR_VALID )
			{
				if_lhs.subtype = TS_AMPR_VAR_VALID ;
				if_lhs.value   = t.value1.substr( 1 ) ;
			}
			else
			{
				if_lhs.value = t.value1 ;
			}
			t = v.getNextToken() ;
			if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
				return ;
			}
			break ;

		case PN_DIR:
		case PN_EXISTS:
		case PN_FILE:
			comp_opt = true ;
		default:
			v.getNextToken() ;
			t = v.getNextToken() ;
			if ( if_func == PN_LVLINE && t.subtype != TS_NAME )
			{
				err.seterrid( TRACE_INFO(), "PSYE032D" ) ;
				return ;
			}
			if ( t.subtype == TS_NAME )
			{
				if_lhs.subtype = TS_AMPR_VAR_VALID ;
			}
			else if ( t.subtype == TS_CTL_VAR_VALID )
			{
				if_lhs.subtype = TS_CTL_VAR_VALID ;
			}
			else if ( t.subtype == TS_CLOSE_BRACKET )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			else if ( t.subtype == TS_CTL_VAR_INVALID )
			{
				err.seterrid( TRACE_INFO(), "PSYE033G", t.value1 ) ;
				return ;
			}
			else
			{
				err.seterrid( TRACE_INFO(), "PSYE031Q" ) ;
				return ;
			}
			if_lhs.value = t.value1 ;
			v.getNextToken() ;
			if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
				return ;
			}
		}
		t = v.getCurrentToken() ;
	}
	else
	{
		if ( t.value1 == "" )
		{
			if ( t.type != TT_STRING_QUOTED )
			{
				err.seterrid( TRACE_INFO(), "PSYE033B" ) ;
				return ;
			}
		}
		else if ( t.type == TT_VARIABLE )
		{
			if_lhs.subtype = t.subtype ;
			if_lhs.value   = ( t.subtype == TS_AMPR_VAR_VALID ) ? t.value1.substr( 1 ) : t.value1 ;
		}
		else if ( t.subtype == TS_CTL_VAR_INVALID )
		{
			err.seterrid( TRACE_INFO(), "PSYE033G", t.value1 ) ;
			return ;
		}
		else if ( t.subtype == TS_AMPR_VAR_INVALID )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", t.value1 ) ;
			return ;
		}
		else
		{
			if_lhs.subtype = TS_NONE ;
			if_lhs.value = ( t.type == TT_STRING_UNQUOTED ) ? t.value2 : t.value1 ;

		}
		t = v.getNextToken() ;
	}

	if ( t.type == TT_EOT )
	{
		err.seterrid( TRACE_INFO(), "PSYE031K", "Missing comparison operator" ) ;
		return ;
	}

	p.clear() ;
	auto it = if_conds.find( t.value1 ) ;
	if ( it == if_conds.end() )
	{
		if ( comp_opt )
		{
			if_cond = RL_EQ ;
			p.value = "1" ;
			if_rhs.push_back( p ) ;
			parse_cond_continue( err, v ) ;
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE033E", t.value1 ) ;
		}
		return ;
	}
	if_cond = it->second ;

	t = v.getNextToken() ;
	if ( t.type == TT_VARIABLE )
	{
		p.subtype = t.subtype ;
		p.value   = ( t.subtype == TS_AMPR_VAR_VALID ) ? t.value1.substr( 1 ) : t.value1 ;
	}
	else if ( t.value1 == "" )
	{
		if ( t.type != TT_STRING_QUOTED )
		{
			err.seterrid( TRACE_INFO(), "PSYE033B" ) ;
			return ;
		}
	}
	else if ( t.subtype == TS_CTL_VAR_INVALID )
	{
		err.seterrid( TRACE_INFO(), "PSYE033G", t.value1 ) ;
		return ;
	}
	else if ( t.subtype == TS_AMPR_VAR_INVALID )
	{
		err.seterrid( TRACE_INFO(), "PSYE031D", t.value1 ) ;
		return ;
	}
	else
	{
		p.subtype = TS_NONE ;
		p.value = ( t.type == TT_STRING_UNQUOTED ) ? t.value2 : t.value1 ;

	}
	if_rhs.push_back( p ) ;

	while ( true )
	{
		t = v.getNextToken() ;
		p.clear() ;
		if ( t.type != TT_STRING_QUOTED &&
		     findword( t.value1, "& | AND OR" ) &&
		     v.peekNextValue() != "," )
		{
			break ;
		}
		else if ( t.type == TT_EOT )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return ;
		}
		else if ( t.subtype == TS_COMMA )
		{
			if ( v.peekNextValue() == "," || v.peekNextValue() == ")" )
			{
				if_rhs.push_back( p ) ;
			}
			continue ;
		}
		else if ( t.subtype == TS_CLOSE_BRACKET )
		{
			break ;
		}
		else if ( t.type == TT_VARIABLE )
		{
			p.value   = ( t.subtype == TS_AMPR_VAR_VALID ) ? t.value1.substr( 1 ) : t.value1 ;
			p.subtype = t.subtype ;
		}
		else if ( t.subtype == TS_CTL_VAR_INVALID )
		{
			err.seterrid( TRACE_INFO(), "PSYE033G", t.value1 ) ;
			return ;
		}
		else if ( t.subtype == TS_AMPR_VAR_INVALID )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", t.value1 ) ;
			return ;
		}
		else
		{
			p.value = ( t.type == TT_STRING_UNQUOTED ) ? t.value2 : t.value1 ;
			p.subtype = TS_NONE ;
		}
		if_rhs.push_back( p ) ;
	}

	if ( if_rhs.size() == 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE031K", "Missing variable" ) ;
		return ;
	}

	if ( ( if_cond != RL_EQ && if_cond != RL_NE ) && if_rhs.size() > 1 )
	{
		err.seterrid( TRACE_INFO(), "PSYE033H" ) ;
		return ;
	}

	parse_cond_continue( err, v ) ;
}


void IFSTMNT::parse_cond_continue( errblock& err,
				   parser& v )
{
	//
	// Check if the if_condition is continued with a boolean operator.
	//

	token t ;

	t = v.getCurrentToken() ;
	if ( t.type != TT_STRING_QUOTED && findword( t.value1, "& | AND OR" ) )
	{
		if_AND  = ( t.value1 == "&" || t.value1 == "AND" ) ;
		if_next = new IFSTMNT ;
		v.getNextToken() ;
		if_next->parse_cond( err, v ) ;
		if ( err.error() ) { return ; }
		if ( if_next->if_lhs.value == ""   &&
		     if_next->if_trunc  == nullptr &&
		     if_next->if_trans  == nullptr &&
		     if_next->if_verify == nullptr )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
			return ;
		}
	}
}


/* ************************************ ***************************** ********************************* */
/* ************************************     *REXX Statement class     ********************************* */
/* ************************************ ***************************** ********************************* */


void PREXX::parse( errblock& err,
		   parser& v )
{
	//
	// Format of the *REXX panel statement:
	//
	// *REXX
	// *REXX ()
	// *REXX (*)
	// *REXX (*,(rexx))
	// *REXX (var1,var2,(rexx))
	// *REXX (*,var1,var2,(rexx))
	// *REXX (&a,(&b))
	//
	// Variable names may be separated by a blank or a comma.
	// REXX procedure and list of passed variables can be contained in a variable.
	//

	err.setRC( 0 ) ;

	token t ;

	t = v.getNextToken() ;

	if ( t.type == TT_EOT )
	{
		return ;
	}

	if ( !v.getNextIfCurrent( TS_OPEN_BRACKET ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE033D" ) ;
		return ;
	}

	t = v.getCurrentToken() ;
	while ( t.subtype != TS_CLOSE_BRACKET && t.subtype != TS_OPEN_BRACKET )
	{
		if ( t.value1 == "*" )
		{
			all_vars = true ;
		}
		else if ( t.subtype != TS_NAME && t.subtype != TS_AMPR_VAR_VALID )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", t.value1 ) ;
			return ;
		}
		else
		{
			if ( t.subtype == TS_AMPR_VAR_VALID )
			{
				vars_var = true ;
			}
			vars.insert( t.value1 ) ;
		}
		t = v.getNextToken() ;
		if ( t.subtype != TS_COMMA && t.subtype != TS_CLOSE_BRACKET && t.subtype != TS_NAME )
		{
			err.seterrid( TRACE_INFO(), "PSYE034J", t.value1 ) ;
			return ;
		}
		if ( t.subtype == TS_COMMA )
		{
			t = v.getNextToken() ;
		}
	}

	if ( t.subtype == TS_OPEN_BRACKET )
	{
		t  = v.getNextToken() ;
		if ( t.subtype == TS_CLOSE_BRACKET )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
			return ;
		}
		else if ( t.type == TT_EOT )
		{
			err.seterrid( TRACE_INFO(), "PSYE034K" ) ;
			return ;
		}
		if ( t.subtype == TS_AMPR_VAR_VALID )
		{
			rexx_var = true ;
			rexx     = t.value1.substr( 1 ) ;
		}
		else
		{
			rexx = t.value2 ;
		}
		t = v.getNextToken() ;
		if ( t.subtype != TS_CLOSE_BRACKET )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return ;
		}
		t = v.getNextToken() ;
	}

	if ( t.subtype != TS_CLOSE_BRACKET )
	{
		err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
		return ;
	}

	t = v.getNextToken() ;

	if ( t.type != TT_EOT )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", t.value1 ) ;
		return ;
	}
}


/* ************************************ ***************************** ********************************* */
/* ************************************   PANEXIT Statement class     ********************************* */
/* ************************************ ***************************** ********************************* */


void PANEXIT::parse( errblock& err,
		     parser& v )
{
	//
	// Format of the PANEXIT panel statement:
	//
	// PANEXIT((VAR1),REXX|LOAD,name)
	// PANEXIT((VAR1,VAR2),REXX|LOAD,name)
	// PANEXIT((VAR1,VAR2),REXX|LOAD,name,MSG=msgid)
	// PANEXIT((VAR1,VAR2),REXX|LOAD,name,exdata,MSG=msgid)
	//
	// Variable names may be separated by a blank or a comma.
	// REXX procedure, LOAD name and list of passed variables can be contained in a variable.
	//

	err.setRC( 0 ) ;

	token t ;

	v.getNextToken() ;

	if ( !v.getNextIfCurrent( TS_OPEN_BRACKET ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE033D" ) ;
		return ;
	}

	if ( !v.getNextIfCurrent( TS_OPEN_BRACKET ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE033D" ) ;
		return ;
	}

	t = v.getCurrentToken() ;
	while ( t.subtype != TS_CLOSE_BRACKET )
	{
		if ( t.subtype != TS_NAME && t.subtype != TS_AMPR_VAR_VALID )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", t.value1 ) ;
			return ;
		}
		else
		{
			if ( t.subtype == TS_AMPR_VAR_VALID )
			{
				vars_var = true ;
			}
			vars.insert( t.value1 ) ;
		}
		t = v.getNextToken() ;
		if ( t.subtype != TS_COMMA && t.subtype != TS_CLOSE_BRACKET && t.subtype != TS_NAME )
		{
			err.seterrid( TRACE_INFO(), "PSYE034J", t.value1 ) ;
			return ;
		}
		if ( t.subtype == TS_COMMA )
		{
			t = v.getNextToken() ;
		}
	}

	if ( vars.empty() )
	{
		err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
		return ;
	}

	v.getNextToken() ;
	if ( !v.getNextIfCurrent( TS_COMMA ) )
	{
		t = v.getCurrentToken() ;
		err.seterrid( TRACE_INFO(), "PSYE038N", t.value2 ) ;
		return ;
	}

	t = v.getCurrentToken() ;
	if ( t.value1 != "REXX" && t.value1 != "LOAD" )
	{
		err.seterrid( TRACE_INFO(), "PSYE038O", t.value2 ) ;
		return ;
	}

	is_rexx = ( t.value1 == "REXX" ) ;

	t = v.getNextToken() ;
	if ( t.subtype != TS_COMMA )
	{
		err.seterrid( TRACE_INFO(), "PSYE038N", t.value2 ) ;
		return ;
	}

	t = v.getNextToken() ;
	if ( t.subtype == TS_CLOSE_BRACKET || t.type == TT_EOT )
	{
		err.seterrid( TRACE_INFO(), "PSYE038P" ) ;
		return ;
	}

	if ( t.subtype == TS_AMPR_VAR_VALID )
	{
		rexx_var = true ;
		rexx     = t.value1.substr( 1 ) ;
	}
	else
	{
		rexx = t.value2 ;
	}

	t = v.getNextToken() ;
	if ( t.subtype == TS_CLOSE_BRACKET )
	{
		t = v.getNextToken() ;
		if ( t.type != TT_EOT )
		{
			err.seterrid( TRACE_INFO(), "PSYE032H", t.value1 ) ;
		}
		return ;
	}
	else if ( t.type == TT_EOT )
	{
		err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
		return ;
	}
	else if ( t.subtype != TS_COMMA )
	{
		err.seterrid( TRACE_INFO(), "PSYE034J", t.value1 ) ;
		return ;
	}

	t = v.getNextToken() ;
	if ( t.subtype == TS_AMPR_VAR_VALID )
	{
		exdata = t.value1.substr( 1 ) ;
		t = v.getNextToken() ;
		if ( t.subtype == TS_CLOSE_BRACKET )
		{
			t = v.getNextToken() ;
			if ( t.type != TT_EOT )
			{
				err.seterrid( TRACE_INFO(), "PSYE032H", t.value1 ) ;
			}
			return ;
		}
		else if ( t.type == TT_EOT )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return ;
		}
		else if ( t.subtype != TS_COMMA )
		{
			err.seterrid( TRACE_INFO(), "PSYE034J", t.value1 ) ;
			return ;
		}
		t = v.getNextToken() ;
	}

	if ( t.value1 != "MSG" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", t.value1 ) ;
		return ;
	}

	t = v.getNextToken() ;
	if ( t.value1 != "=" )
	{
		err.seterrid( TRACE_INFO(), "PSYE038Q" ) ;
		return ;
	}

	t = v.getNextToken() ;
	if ( t.subtype != TS_NAME )
	{
		err.seterrid( TRACE_INFO(), "PSYE038Q" ) ;
		return ;
	}

	msgid = t.value1 ;

	t = v.getNextToken() ;

	if ( t.subtype != TS_CLOSE_BRACKET )
	{
		err.seterrid( TRACE_INFO(), "PSYE038R" ) ;
		return ;
	}

	t = v.getNextToken() ;

	if ( t.type != TT_EOT )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", t.value1 ) ;
		return ;
	}
}


/* ************************************ ***************************** ********************************* */
/* ************************************       Assignment class        ********************************* */
/* ************************************ ***************************** ********************************* */


void ASSGN::parse( errblock& err,
		   parser& v )
{
	//
	// Format of the assignment panel statement.
	//
	// &AAA = &BBBB
	// &AAA = VALUE
	// &AAA = 'Quoted Value'
	// &AAA = .ALARM | .TRAIL | .HELP | .MSG | .CSRPOS | .CURSOR | .RESP
	// .ALARM .RESP | .HELP | .MSG | .CSRPOS | .CURSOR = &BBB | VALUE | 'Quoted Value'
	// &AAA = UPPER( ABC )
	// &AAA = LENGTH( ABC )
	// &AAA = REVERSE( ABC )
	// &AAA = WORDS( ABC )  Number of words in the value of ABC.
	// &A   = EXISTS( ABC ) True if file/directory in variable ABC exists.
	// &A   = FILE( ABC )   True if file in variable ABC exists.
	// &A   = DIR( ABC )    True if directory in variable ABC exists.
	// &A   = TRUNC()
	// &A   = TRANS()
	// .ATTR(ZCMD)    = 'xxxx'
	// .ATTR(.CURSOR) = 'xxxx'
	// .ATTR(&VAR)    = 'xxxx'
	// .ATTRCHAR(+)   = 'xxxx'
	// .ATTRCHAR(92)  = 'xxxx'
	// .ATTRCHAR(&VAR)= 'xxxx'
	//

	token t ;

	map<string, PN_FUNCTION>::iterator it ;

	map<string, PN_FUNCTION> assign_functions =
	{ { "DIR",     PN_DIR     },
	  { "EXISTS",  PN_EXISTS  },
	  { "FILE",    PN_FILE    },
	  { "LENGTH",  PN_LENGTH  },
	  { "LVLINE",  PN_LVLINE  },
	  { "PFK",     PN_PFK     },
	  { "TRANS",   PN_TRANS   },
	  { "TRUNC",   PN_TRUNC   },
	  { "REVERSE", PN_REVERSE },
	  { "WORDS",   PN_WORDS   },
	  { "UPPER",   PN_UPPER   } } ;

	err.setRC( 0 ) ;

	t = v.getFirstToken() ;
	if ( t.type == TT_STRING_QUOTED )
	{
		err.seterrid( TRACE_INFO(), "PSYE031H" ) ;
		return ;
	}

	if ( findword( t.value1, ".FALSE .PFKEY .TRUE .TRAIL" ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE033S", t.value1 ) ;
		return ;
	}
	else if ( t.type == TT_VARIABLE )
	{
		as_lhs.value   = ( t.subtype == TS_AMPR_VAR_VALID ) ? t.value1.substr( 1 ) : t.value1 ;
		as_lhs.subtype = t.subtype ;
		t = v.getNextToken() ;
	}
	else if ( t.value1 == ".ATTR" )
	{
		v.getNextToken() ;
		if ( !v.getNextIfCurrent( TS_OPEN_BRACKET ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE033D" ) ;
			return ;
		}
		if ( v.isCurrentSubType( TS_CLOSE_BRACKET ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
			return ;
		}
		t = v.getCurrentToken() ;
		as_lhs.value = t.value1 ;
		if ( t.subtype == TS_AMPR_VAR_VALID )
		{
			as_lhs.value.erase( 0, 1 ) ;
		}
		if ( !v.getNextIfCurrent( ".CURSOR" ) &&
		     !v.getNextIfCurrent( TS_NAME )   &&
		     !v.getNextIfCurrent( TS_AMPR_VAR_VALID ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031O", as_lhs.value ) ;
			return ;
		}
		if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return ;
		}
		as_lhs.subtype = t.subtype ;
		as_isattr = true ;
	}
	else if ( t.value1 == ".ATTRCHAR" )
	{
		v.getNextToken() ;
		if ( !v.getNextIfCurrent( TS_OPEN_BRACKET ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE033D" ) ;
			return ;
		}
		if ( v.isCurrentSubType( TS_CLOSE_BRACKET ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
			return ;
		}
		t = v.getCurrentToken() ;
		if ( t.type    != TT_STRING_QUOTED   &&
		     t.type    != TT_STRING_UNQUOTED &&
		     t.subtype != TS_AMPR_VAR_VALID )
		{
			err.seterrid( TRACE_INFO(), "PSYE031P", as_lhs.value ) ;
			return ;
		}
		as_lhs.value = ( t.type == TT_STRING_UNQUOTED ) ? t.value2 : t.value1 ;
		as_lhs.subtype = t.subtype ;
		if ( t.subtype == TS_AMPR_VAR_VALID )
		{
			as_lhs.value.erase( 0, 1 ) ;
		}
		else
		{
			if ( as_lhs.value.size() > 2 || ( as_lhs.value.size() == 2 && not ishex( as_lhs.value ) ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE041B" ) ;
				return ;
			}
			if ( as_lhs.value.size() == 2 ) { as_lhs.value = xs2cs( as_lhs.value ) ; }
		}
		v.getNextToken() ;
		if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return ;
		}
		as_isattc = true ;
	}
	else if ( t.subtype == TS_CTL_VAR_INVALID )
	{
		err.seterrid( TRACE_INFO(), "PSYE033G", t.value1 ) ;
		return ;
	}
	else if ( t.subtype == TS_AMPR_VAR_INVALID )
	{
		err.seterrid( TRACE_INFO(), "PSYE031D", t.value1 ) ;
		return ;
	}
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE033Q", t.value1 ) ;
		return ;
	}

	if ( !v.getNextIfCurrent( TS_EQUALS ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE033V" ) ;
		return ;
	}

	if ( v.isCurrentType( TT_EOT ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE033W" ) ;
		return ;
	}

	t = v.getCurrentToken() ;
	if ( t.type == TT_VARIABLE )
	{
		as_rhs.subtype = t.subtype ;
		as_rhs.value   = ( t.subtype == TS_CTL_VAR_VALID ) ? t.value1 : t.value1.substr( 1 ) ;
		t = v.getNextToken() ;
	}
	else if ( t.subtype == TS_CTL_VAR_INVALID )
	{
		err.seterrid( TRACE_INFO(), "PSYE033G", t.value1 ) ;
		return ;
	}
	else if ( t.subtype == TS_AMPR_VAR_INVALID )
	{
		err.seterrid( TRACE_INFO(), "PSYE031D", t.value1 ) ;
		return ;
	}
	else if ( v.peekNextValue() == "(" )
	{
		it = assign_functions.find( t.value1 ) ;
		if ( it == assign_functions.end() )
		{
			err.seterrid( TRACE_INFO(), "PSYE033U", t.value1 ) ;
			return ;
		}
		as_func = it->second ;
		switch ( as_func )
		{
		case PN_TRUNC:
			as_trunc = new TRUNC ;
			as_trunc->parse( err, v ) ;
			if ( err.error() ) { return ; }
			break ;

		case PN_TRANS:
			as_trans = new TRANS ;
			as_trans->parse( err, v ) ;
			if ( err.error() ) { return ; }
			break ;

		case PN_PFK:
			v.getNextToken() ;
			t = v.getNextToken() ;
			if ( t.subtype == TS_AMPR_VAR_VALID )
			{
				as_rhs.subtype = TS_AMPR_VAR_VALID ;
				as_rhs.value   = t.value1.substr( 1 ) ;
			}
			else
			{
				as_rhs.value = t.value1 ;
			}
			t = v.getNextToken() ;
			if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
				return ;
			}
			break ;

		default:
			v.getNextToken() ;
			t = v.getNextToken() ;
			if ( as_func == PN_LVLINE && t.subtype != TS_NAME )
			{
				err.seterrid( TRACE_INFO(), "PSYE032D" ) ;
				return ;
			}
			if ( t.subtype == TS_NAME )
			{
				as_rhs.subtype = TS_AMPR_VAR_VALID ;
			}
			else if ( t.subtype == TS_CTL_VAR_VALID )
			{
				as_rhs.subtype = TS_CTL_VAR_VALID ;
			}
			else if ( t.subtype == TS_CLOSE_BRACKET )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			else if ( t.subtype == TS_CTL_VAR_INVALID )
			{
				err.seterrid( TRACE_INFO(), "PSYE033G", t.value1 ) ;
				return ;
			}
			else
			{
				err.seterrid( TRACE_INFO(), "PSYE031Q" ) ;
				return ;
			}
			as_rhs.value = t.value1 ;
			t = v.getNextToken() ;
			if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
				return ;
			}
		}
	}
	else
	{
		as_rhs.value = ( !as_isattr && !as_isattc && t.type == TT_STRING_UNQUOTED ) ? t.value2 : t.value1 ;
		v.getNextToken() ;
	}

	if ( !v.isCurrentType( TT_EOT ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", v.getCurrentValue() ) ;
		return ;
	}
}


/* ************************************ ***************************** ********************************* */
/* ************************************        VGET/VPUT class        ********************************* */
/* ************************************ ***************************** ********************************* */


void VPUTGET::parse( errblock& err,
		     parser& v )
{
	//
	// VGET ABC
	// VGET(ABC) PROFILE
	//

	token t ;

	err.setRC( 0 ) ;

	t = v.getFirstToken() ;
	vpg_vput = ( t.value1 == "VPUT" ) ;

	v.getNextToken() ;

	v.getNameList( err, vpg_vars ) ;
	if ( err.error() ) { return ; }

	t = v.getCurrentToken() ;
	if ( t.value1 == "ASIS" ||
	     t.type   == TT_EOT )         { vpg_pool = ASIS    ; }
	else if ( t.value1 == "SHARED" )  { vpg_pool = SHARED  ; }
	else if ( t.value1 == "PROFILE" ) { vpg_pool = PROFILE ; }
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE033Z", t.value1 ) ;
		return ;
	}

	t = v.getNextToken() ;
	if ( t.type != TT_EOT )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", t.value1 ) ;
		return ;
	}
}


/* ************************************ ***************************** ********************************* */
/* ************************************           VEDIT class         ********************************* */
/* ************************************ ***************************** ********************************* */


void VEDIT::parse( errblock& err,
		   parser& v )
{
	//
	// VEDIT (VAR)
	// VEDIT (VAR,MSG=msgid)
	//

	token t ;

	err.setRC( 0 ) ;

	v.getFirstToken() ;
	v.getNextToken() ;

	if ( !v.getNextIfCurrent( TS_OPEN_BRACKET ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE033D" ) ;
		return ;
	}

	t       = v.getCurrentToken() ;
	ved_var = t.value1 ;

	if ( !v.getNextIfCurrent( TS_NAME ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE031D", ved_var ) ;
		return ;
	}

	if ( v.getNextIfCurrent( TS_COMMA ) )
	{
		t = v.getCurrentToken() ;
		if ( t.value1 != "MSG" )
		{
			err.seterrid( TRACE_INFO(), "PSYE039K" ) ;
			return ;
		}
		v.getNextToken() ;
		if ( !v.getNextIfCurrent( TS_EQUALS ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE033V" ) ;
			return ;
		}
		t = v.getCurrentToken() ;
		if ( v.getNextIfCurrent( TS_NAME ) ||
		     v.getNextIfCurrent( TS_AMPR_VAR_VALID ) )
		{
			ved_msgid = t.value1 ;
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE031I", t.value1 ) ;
			return ;
		}
	}

	if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
	{
		if ( v.isCurrentType( TT_EOT ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE039L" ) ;
		}
		return ;
	}

	if ( !v.isCurrentType( TT_EOT ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", v.getCurrentValue() ) ;
		return ;
	}
}


/* ************************************ ***************************** ********************************* */
/* ************************************          VERIFY class         ********************************* */
/* ************************************ ***************************** ********************************* */


void VERIFY::parse( errblock& err,
		    parser& v,
		    bool check )
{
	//
	// VER (&VAR LIST A B C D)
	// VER (&VAR,LIST,A,B,C,D)
	// VER (&VAR NB LIST A B C D)
	// VER(&VAR NONBLANK LIST A B C D)
	// VER(&VAR PICT ABCD MSG = PSYS011A )
	// VER(&VAR HEX)
	// VER(&VAR OCT)
	//

	token t ;

	map<string, VL_COND> ver_conds =
	{ { "=",  VL_EQ },
	  { "EQ", VL_EQ },
	  { "!=", VL_NE },
	  { "NE", VL_NE },
	  { ">",  VL_GT },
	  { "GT", VL_GT },
	  { "<",  VL_LT },
	  { "LT", VL_LT },
	  { ">=", VL_GE },
	  { "GE", VL_GE },
	  { "!<", VL_GE },
	  { "NL", VL_GE },
	  { "<=", VL_LE },
	  { "LE", VL_LE },
	  { "!>", VL_LE },
	  { "NG", VL_LE } } ;

	const string picts = "CAN9Xcanx" ;

	err.setRC( 0 ) ;

	v.getFirstToken() ;
	v.getNextToken()  ;

	if ( !v.getNextIfCurrent( TS_OPEN_BRACKET ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE033D" ) ;
		return ;
	}

	t = v.getCurrentToken() ;
	ver_var = t.value1.substr( 1 ) ;

	if ( !v.getNextIfCurrent( TS_AMPR_VAR_VALID ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE033I", t.value1 ) ;
		return ;
	}

	v.getNextIfCurrent( TS_COMMA ) ;
	t = v.getCurrentToken() ;

	if ( findword( t.value1, "NB NONBLANK" ) )
	{
		ver_nblank = true ;
		v.getNextToken()  ;
		if ( v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
		{
			if ( !v.isCurrentType( TT_EOT ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE032H", v.getCurrentValue() ) ;
				return ;
			}
			return ;
		}
		v.getNextIfCurrent( TS_COMMA ) ;
		t = v.getCurrentToken() ;
	}

	if      ( t.value1 == "ALPHA"   ) { ver_type = VER_ALPHA   ; }
	else if ( t.value1 == "ALPHAB"  ) { ver_type = VER_ALPHAB  ; }
	else if ( t.value1 == "BIT"     ) { ver_type = VER_BIT     ; }
	else if ( t.value1 == "ENUM"    ) { ver_type = VER_ENUM    ; }
	else if ( t.value1 == "HEX"     ) { ver_type = VER_HEX     ; }
	else if ( t.value1 == "INCLUDE" ) { ver_type = VER_INCLUDE ; }
	else if ( t.value1 == "IPADDR4" ) { ver_type = VER_IPADDR4 ; }
	else if ( t.value1 == "IPADDR6" ) { ver_type = VER_IPADDR6 ; }
	else if ( t.value1 == "LEN"     ) { ver_type = VER_LEN     ; }
	else if ( t.value1 == "LIST"    ) { ver_type = VER_LIST    ; }
	else if ( t.value1 == "LISTX"   ) { ver_type = VER_LISTX   ; ver_nblank = true ; }
	else if ( t.value1 == "LISTV"   ) { ver_type = VER_LISTV   ; }
	else if ( t.value1 == "LISTVX"  ) { ver_type = VER_LISTVX  ; ver_nblank = true ; }
	else if ( t.value1 == "NAME"    ) { ver_type = VER_NAME    ; }
	else if ( t.value1 == "OCT"     ) { ver_type = VER_OCT     ; }
	else if ( t.value1 == "PICT"    ) { ver_type = VER_PICT    ; }
	else if ( t.value1 == "PICTCN"  ) { ver_type = VER_PICTCN  ; }
	else if ( t.value1 == "RANGE"   ) { ver_type = VER_RANGE   ; }
	else if ( t.value1 == "STDDATE" ) { ver_type = VER_STDDATE ; }
	else if ( t.value1 == "STDTIME" ) { ver_type = VER_STDTIME ; }
	else if ( t.value1 == "NUM"     ) { ver_type = VER_NUMERIC ; }
	else if ( t.value1 != "MSG"     )
	{
		err.seterrid( TRACE_INFO(), "PSYE051B", t.value1 ) ;
		return ;
	}

	if ( t.value1 != "MSG" )
	{
		v.getNextToken() ;
		v.getNextIfCurrent( TS_COMMA ) ;
		t = v.getCurrentToken() ;
	}

	if ( ver_type == VER_PICT )
	{
		ver_vlist.push_back( t.value1 ) ;
		t = v.getNextToken() ;
		if ( t.subtype == TS_COMMA )
		{
			if ( v.peekNextValue() != "MSG" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031W" ) ;
				return ;
			}
			v.getNextToken() ;
		}
	}

	if ( ver_type == VER_PICTCN )
	{
		if ( t.subtype == TS_AMPR_VAR_INVALID )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", t.value1.substr( 1 ) ) ;
			return ;
		}
		else if ( t.subtype != TS_AMPR_VAR_VALID && ( t.value1.size() != 1 || picts.find( t.value1[ 0 ] ) != string::npos ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE052R" ) ;
			return ;
		}
		ver_vlist.push_back( t.value1 ) ;
		t = v.getNextToken() ;
		if ( t.subtype != TS_COMMA )
		{
			err.seterrid( TRACE_INFO(), "PSYE038N", t.value2 ) ;
			return ;
		}
		t = v.getNextToken() ;
		if ( t.subtype == TS_AMPR_VAR_INVALID )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", t.value1.substr( 1 ) ) ;
			return ;
		}
		ver_vlist.push_back( t.value1 ) ;
		t = v.getNextToken() ;
		if ( t.subtype != TS_COMMA )
		{
			err.seterrid( TRACE_INFO(), "PSYE038N", t.value2 ) ;
			return ;
		}
		t = v.getNextToken() ;
		if ( t.subtype == TS_AMPR_VAR_INVALID )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", t.value1.substr( 1 ) ) ;
			return ;
		}
		ver_vlist.push_back( t.value1 ) ;
		t = v.getNextToken() ;
		if ( t.subtype == TS_COMMA )
		{
			if ( v.peekNextValue() != "MSG" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031W" ) ;
				return ;
			}
			v.getNextToken() ;
		}
		else if ( t.type == TT_EOT )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return ;
		}
		else if ( t.subtype != TS_CLOSE_BRACKET )
		{
			err.seterrid( TRACE_INFO(), "PSYE034J", t.value2 ) ;
			return ;
		}
	}

	t = v.getCurrentToken() ;
	while ( ver_type == VER_LIST || ver_type == VER_LISTX )
	{
		if ( t.subtype == TS_CLOSE_BRACKET )
		{
			break ;
		}
		else if ( t.type == TT_EOT )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return ;
		}
		else if ( t.subtype == TS_COMMA )
		{
			if ( v.peekNextValue() == "," ||
			     v.peekNextValue() == ")" )
			{
				ver_vlist.push_back( "" ) ;
			}
			t = v.getNextToken() ;
			continue ;
		}
		else if ( t.value1 == "MSG" )
		{
			break ;
		}
		else if ( t.subtype == TS_AMPR_VAR_INVALID )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", t.value1 ) ;
			return ;
		}
		else
		{
			ver_vlist.push_back( t.value1 ) ;
		}
		t = v.getNextToken() ;
	}

	if ( ver_type == VER_LISTV || ver_type == VER_LISTVX )
	{
		if ( t.subtype != TS_AMPR_VAR_VALID )
		{
			err.seterrid( TRACE_INFO(), "PSYE051G", t.value1.substr( 1 ) ) ;
			return ;
		}
		ver_vlist.push_back( t.value1.substr( 1 ) ) ;
		v.getNextToken() ;
		t = v.getCurrentToken() ;
		if ( t.subtype == TS_COMMA )
		{
			if ( v.peekNextValue() != "MSG" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031W" ) ;
				return ;
			}
			t = v.getNextToken() ;
		}
	}

	if ( ver_type == VER_RANGE )
	{
		if ( !datatype( t.value1, 'W' ) )
		{
			if ( t.subtype == TS_AMPR_VAR_VALID )
			{
				ver_vlist.push_back( t.value1.substr( 1 ) ) ;
			}
			else
			{
				err.seterrid( TRACE_INFO(), "PSYE051D" ) ;
				return ;
			}
		}
		else
		{
			ver_vlist.push_back( t.value1 ) ;
		}
		t = v.getNextToken() ;
		if ( v.getNextIfCurrent( TS_COMMA ) )
		{
			t = v.getCurrentToken() ;
		}
		if ( !datatype( t.value1, 'W' ) )
		{
			if ( t.subtype == TS_AMPR_VAR_VALID )
			{
				ver_vlist.push_back( t.value1.substr( 1 ) ) ;
			}
			else
			{
				err.seterrid( TRACE_INFO(), "PSYE051D" ) ;
				return ;
			}
		}
		else
		{
			ver_vlist.push_back( t.value1 ) ;
		}
		v.getNextToken() ;
		t = v.getCurrentToken() ;
		if ( t.subtype == TS_COMMA )
		{
			if ( v.peekNextValue() != "MSG" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031W" ) ;
				return ;
			}
			t = v.getNextToken() ;
		}
	}

	if ( ver_type == VER_LEN )
	{
		auto it = ver_conds.find( t.value1 ) ;
		if ( it == ver_conds.end() )
		{
			err.seterrid( TRACE_INFO(), "PSYE051E", t.value1 ) ;
			return ;
		}
		ver_cond = it->second ;
		t = v.getNextToken() ;
		if ( v.getNextIfCurrent( TS_COMMA ) )
		{
			t = v.getCurrentToken() ;
		}
		if ( t.subtype == TS_AMPR_VAR_VALID )
		{
			ver_vlist.push_back( t.value1.substr( 1 ) ) ;
		}
		else
		{
			if ( !datatype( t.value1, 'W' ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE051F" ) ;
				return ;
			}
			ver_len = ds2d( t.value1 ) ;
		}
		v.getNextToken() ;
		t = v.getCurrentToken() ;
		if ( t.subtype == TS_COMMA )
		{
			if ( v.peekNextValue() != "MSG" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031W" ) ;
				return ;
			}
			t = v.getNextToken() ;
		}
	}

	while ( ver_type == VER_INCLUDE )
	{
		if ( t.subtype == TS_CLOSE_BRACKET )
		{
			if ( ver_include == 0x00 )
			{
				err.seterrid( TRACE_INFO(), "PSYE051H" ) ;
				return ;
			}
			if ( ( ver_include & INC_ALL3 ) == INC_ALL3 )
			{
				err.seterrid( TRACE_INFO(), "PSYE051I" ) ;
				return ;
			}
			break ;
		}
		else if ( t.subtype == TS_COMMA )
		{
			t = v.getNextToken() ;
			continue ;
		}
		else if ( t.type == TT_EOT )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return ;
		}
		else if ( t.value1 == "IMBLK" )
		{
			if ( ( ver_include & INC_IMBLK ) != 0x00 )
			{
				err.seterrid( TRACE_INFO(), "PSYE051A", "IMBLK", "INCLUDE" ) ;
				return ;
			}
			if ( ver_include != 0x00 )
			{
				err.seterrid( TRACE_INFO(), "PSYE051J" ) ;
				return ;
			}
			ver_include |= INC_IMBLK ;
		}
		else if ( t.value1 == "ALPHA" )
		{
			if ( ( ver_include & INC_ALPHA ) != 0x00 )
			{
				err.seterrid( TRACE_INFO(), "PSYE051A", "ALPHA", "INCLUDE" ) ;
				return ;
			}
			ver_include |= INC_ALPHA ;
		}
		else if ( t.value1 == "ALPHAB" )
		{
			if ( ( ver_include & INC_ALPHAB ) != 0x00 )
			{
				err.seterrid( TRACE_INFO(), "PSYE051A", "ALPHAB", "INCLUDE" ) ;
				return ;
			}
			ver_include |= INC_ALPHAB ;
		}
		else if ( t.value1 == "NUM" )
		{
			if ( ( ver_include & INC_NUM ) != 0x00 )
			{
				err.seterrid( TRACE_INFO(), "PSYE051A", "NUM", "INCLUDE" ) ;
				return ;
			}
			ver_include |= INC_NUM ;
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE051K", t.value1 ) ;
			return ;
		}
		t = v.getNextToken() ;
	}

	if ( t.value1 == "MSG" )
	{
		v.getNextToken() ;
		if ( !v.getNextIfCurrent( TS_EQUALS ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE033V" ) ;
			return ;
		}
		t = v.getCurrentToken() ;
		if ( v.getNextIfCurrent( TS_NAME ) ||
		     v.getNextIfCurrent( TS_AMPR_VAR_VALID ) )
		{
			ver_msgid = t.value1 ;
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE031I", t.value1 ) ;
			return ;
		}
	}

	if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
	{
		if ( v.isCurrentType( TT_EOT ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE031W" ) ;
		}
		return ;
	}

	if ( check && !v.isCurrentType( TT_EOT ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", v.getCurrentValue() ) ;
		return ;
	}

	if ( ( ver_type == VER_LIST   ||
	       ver_type == VER_LISTX  ||
	       ver_type == VER_LISTV  ||
	       ver_type == VER_LISTVX ||
	       ver_type == VER_PICT )  &&
	       ver_vlist.empty() )
	{
		err.seterrid( TRACE_INFO(), "PSYE051C" ) ;
		return ;
	}
}


/* ************************************ ***************************** ********************************* */
/* ************************************           TRUNC class         ********************************* */
/* ************************************ ***************************** ********************************* */


void TRUNC::parse( errblock& err,
		   parser& v,
		   bool check )
{
	//
	// Format of the TRUNC panel statement.
	//
	// TRUNC( &BBB,'.' )
	// TRUNC ( &BBB, 3  )
	//

	token t ;

	err.setRC( 0 ) ;

	v.getNextToken() ;

	if ( !v.getNextIfCurrent( TS_OPEN_BRACKET ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE033D" ) ;
		return ;
	}

	t = v.getCurrentToken() ;

	if ( t.type == TT_VARIABLE )
	{
		if ( t.value1 == ".TRAIL" )
		{
			err.seterrid( TRACE_INFO(), "PSYE038G" ) ;
			return ;
		}
		trnc_field.subtype = t.subtype ;
		trnc_field.value = ( t.subtype == TS_CTL_VAR_VALID ) ? t.value1 : t.value1.substr( 1 ) ;
	}
	else if ( t.subtype == TS_CTL_VAR_INVALID )
	{
		err.seterrid( TRACE_INFO(), "PSYE033G", t.value1 ) ;
		return ;
	}
	else if ( t.subtype == TS_AMPR_VAR_INVALID )
	{
		err.seterrid( TRACE_INFO(), "PSYE031D", t.value1 ) ;
		return ;
	}
	else
	{
		err.seterrid( TRACE_INFO(), ( t.value1 == "" ) ? "PSYE038A" : "PSYE031Q" ) ;
		return ;
	}

	v.getNextToken() ;
	if ( !v.getNextIfCurrent( TS_COMMA ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE038C" ) ;
		return ;
	}

	t = v.getCurrentToken() ;

	if ( t.subtype == TS_CLOSE_BRACKET )
	{
		v.getNextToken() ;
		if ( check && !v.isCurrentType( TT_EOT ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE032H", v.getCurrentValue() ) ;
		}
		trnc_char = ' ' ;
		return ;
	}

	if ( datatype( t.value1, 'W' ) )
	{
		trnc_len = ds2d( t.value1 ) ;
		if ( trnc_len <= 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE019F" ) ;
			return ;
		}
	}
	else if ( t.value1.size() == 1 )
	{
		trnc_char = t.value1.front() ;
	}
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE038F", t.value1 ) ;
		return ;
	}

	v.getNextToken() ;
	if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
		return ;
	}

	if ( check && !v.isCurrentType( TT_EOT ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", v.getCurrentValue() ) ;
		return ;
	}
}


/* ************************************ ***************************** ********************************* */
/* ************************************           TRANS class         ********************************* */
/* ************************************ ***************************** ********************************* */


void TRANS::parse( errblock& err,
		   parser& v,
		   bool check )
{
	//
	// Format of the TRANS panel statement ( change val1 to val2, * is everything else.  Issue message. )
	//
	// TRANS( &BBB  val1,val2 ...  *,* )
	// TRANS ( &BBB  val1,val2 ...  *,'?' )
	// TRANS ( &BBB  val1,val2 ...  ) non-matching results in &AAA being set to null.
	// TRANS ( &AAA  val1,val2 ...  MSG=msgid ) issue message if no match.
	// TRANS ( &AAA  &v1,&v2 ...  MSG = msgid ) issue message if no match.
	// TRANS ( TRUNC(...)  &v1,&v2 ...  MSG = msgid ) issue message if no match.
	//

	string v1 ;

	bool first ;

	token t ;

	err.setRC( 0 ) ;

	t = v.getNextToken() ;

	if ( !v.getNextIfCurrent( TS_OPEN_BRACKET ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE033D" ) ;
		return ;
	}

	t = v.getCurrentToken() ;

	if ( t.value1 == "TRUNC" && v.peekNextValue() == "(" )
	{
		trns_trunc = new TRUNC ;
		trns_trunc->parse( err, v, false ) ;
		if ( err.error() ) { return ; }
	}
	else if ( t.type == TT_VARIABLE )
	{
		trns_field.subtype = t.subtype ;
		trns_field.value   = ( t.subtype == TS_CTL_VAR_VALID ) ? t.value1 : t.value1.substr( 1 ) ;
		t = v.getNextToken() ;
	}
	else if ( t.subtype == TS_CTL_VAR_INVALID )
	{
		err.seterrid( TRACE_INFO(), "PSYE033G", t.value1 ) ;
		return ;
	}
	else if ( t.subtype == TS_AMPR_VAR_INVALID )
	{
		err.seterrid( TRACE_INFO(), "PSYE031D", t.value1 ) ;
		return ;
	}
	else
	{
		err.seterrid( TRACE_INFO(), ( t.value1 == "" ) ? "PSYE038A" : "PSYE031Q" ) ;
		return ;
	}

	first = true ;
	while ( true )
	{
		t = v.getCurrentToken() ;
		if ( t.type == TT_EOT )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return ;
		}
		else if ( v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
		{
			if ( !first )
			{
				err.seterrid( TRACE_INFO(), "PSYE039H" ) ;
				return ;
			}
			break ;
		}
		if ( first && t.value1 == "MSG" && v.peekNextValue() == "=" )
		{
			v.getNextToken() ;
			t = v.getNextToken() ;
			if ( v.getNextIfCurrent( TS_NAME ) ||
			     v.getNextIfCurrent( TS_AMPR_VAR_VALID ) )
			{
				trns_msgid = t.value1 ;
			}
			else
			{
				err.seterrid( TRACE_INFO(), "PSYE031D", t.value1 ) ;
				return ;
			}
			if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
				return ;
			}
			break ;

		}
		if ( t.type    == TT_STRING_UNQUOTED ||
		     t.type    == TT_STRING_QUOTED   ||
		     t.subtype == TS_AMPR_VAR_VALID )
		{
			if ( first )
			{
				v1 = t.value1 ;
				first = false ;
				v.getNextToken() ;
				continue ;
			}
			if ( v1 == "*" )
			{
				trns_default = t.value1 ;
			}
			else
			{
				trns_list.push_back(
				make_pair( v1, ( t.type == TT_STRING_UNQUOTED ) ? t.value2 : t.value1 ) ) ;
			}
			v.getNextToken() ;
			first = true ;
			continue ;
		}
		if ( t.subtype == TS_COMMA )
		{
			if ( v.peekNextValue() == "," )
			{
				if ( first )
				{
					v1 = "" ;
					first = false ;
					v.getNextToken() ;
					continue ;
				}
				if ( v1 == "*" )
				{
					trns_default = "" ;
				}
				else
				{
					trns_list.push_back( make_pair( v1, "" )  ) ;
				}
				first = true ;
				v.getNextToken() ;
			}
			v.getNextToken() ;
			continue ;
		}
		err.seterrid( TRACE_INFO(), "PSYE031R", "TRANS", v.getCurrentValue() ) ;
		return ;
	}

	if ( check && !v.isCurrentType( TT_EOT ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", v.getCurrentValue() ) ;
		return ;
	}
}


/* ************************************ ***************************** ********************************* */
/* ************************************     Point-and-shoot class     ********************************* */
/* ************************************ ***************************** ********************************* */


void pnts::parse( errblock& err,
		  string s )
{
	//
	// Format of the PNTS panel entry (point-and-shoot entries).
	//
	// FIELD(fld) VAR(var) VAL(value)
	// fld and var must be defined in the panel.
	//

	iupper( s ) ;

	err.setRC( 0 ) ;

	pnts_field = parseString1( err, s, "FIELD()" ) ;
	if ( err.error() ) { return ; }
	if ( pnts_field == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE031C", "FIELD" ) ;
		return ;
	}
	if ( !isvalidName( pnts_field ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE031D", pnts_field ) ;
		return ;
	}

	pnts_var = parseString1( err, s, "VAR()" ) ;
	if ( err.error() ) { return ; }
	if ( pnts_var == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE031C", "VAR" ) ;
		return ;
	}

	if ( !isvalidName( pnts_var ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE031D", pnts_var ) ;
		return ;
	}

	pnts_val = extractKWord( err, s, "VAL()" ) ;
	if ( err.error() ) { return ; }
	if ( pnts_val == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE031C", "VAL" ) ;
		return ;
	}

	if ( s != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", s ) ;
		return ;
	}

	if ( pnts_field == "" || pnts_var == "" || pnts_val == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE037A" ) ;
		return ;
	}
}


/* ************************************ ***************************** ********************************* */
/* ************************************           Help class          ********************************* */
/* ************************************ ***************************** ********************************* */


void help::parse( errblock& err,
		  string s )
{
	//
	// Format of the HELP panel entry.
	//
	// FIELD(fld) PANEL(panel)
	// FIELD(fld) MSG(panel)
	// FIELD(fld) PASSTHRU
	//

	iupper( s ) ;

	err.setRC( 0 ) ;

	bool ex_msg   = false ;
	bool ex_panel = false ;

	help_field = parseString1( err, s, "FIELD()" ) ;
	if ( err.error() ) { return ; }
	if ( help_field == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE045A" ) ;
		return ;
	}
	if ( !isvalidName( help_field ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE045B", help_field ) ;
		return ;
	}

	help_panel = parseString1( err, s, "PANEL()" ) ;
	if ( err.error() ) { return ; }
	if ( err.RSN0() )
	{
		if ( help_panel == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
			return ;
		}
		if ( !isvalidName( help_panel ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE045B", help_panel ) ;
			return ;
		}
		ex_panel = true ;
	}

	help_msg = parseString1( err, s, "MSG()" ) ;
	if ( err.error() ) { return ; }
	if ( err.RSN0() )
	{
		if ( ex_panel )
		{
			err.seterrid( TRACE_INFO(), "PSYE045E" ) ;
			return ;
		}
		if ( help_msg == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
			return ;
		}
		if ( !isvalidName( help_msg ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE045B", help_msg ) ;
			return ;
		}
		ex_msg = true ;
	}

	help_passthru = parseString2( s, "PASSTHRU" ) ;

	if ( s != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", s ) ;
		return ;
	}

	if ( help_passthru && ( ex_panel || ex_msg ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE045E" ) ;
		return ;
	}

	if ( !help_passthru && !ex_panel && !ex_msg )
	{
		err.seterrid( TRACE_INFO(), "PSYE045D" ) ;
		return ;
	}
}


/* ************************************ ***************************** ********************************* */
/* ************************************      Field Execute class      ********************************* */
/* ************************************ ***************************** ********************************* */


void fieldXct::parse( errblock& err,
		      string s )
{
	err.setRC( 0 ) ;

	fieldXct_field = parseString1( err, s, "FIELD()" ) ;
	if ( err.error() ) { return ; }
	if ( fieldXct_field == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE042G" ) ;
		return ;
	}

	fieldXct_command = parseString1( err, s, "EXEC()" ) ;
	if ( err.error() ) { return ; }
	if ( fieldXct_command == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE042H" ) ;
		return ;
	}
	istrip( fieldXct_command, 'B', '\'' ) ;

	fieldXct_passed = parseString1( err, s, "PASS()" ) ;
	if ( err.error() ) { return ; }

	if ( s != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", s ) ;
		return ;
	}

	replace( fieldXct_passed.begin(), fieldXct_passed.end(), ',', ' ' ) ;

	fieldXct_pnum = words( fieldXct_passed ) ;

	if ( fieldXct_pnum > 9 )
	{
		err.seterrid( TRACE_INFO(), "PSYE042D" ) ;
		return ;
	}

}


/* ************************************ ***************************** ********************************* */
/* ************************************     Scrollable Field class    ********************************* */
/* ************************************ ***************************** ********************************* */


void sfield::parse( errblock& err,
		    string s )
{
	//
	// Scrollable field class.
	//
	// FIELD(name) IND(field,val) LIND(field,val) RIND(field,val) SIND(field,val) SCALE(field) LEN(nnnnn|var)
	//             LCOL(field) RCOL(field) SCROLL(ON|YES|NOLR|OFF|NO|var)
	//

	size_t p ;

	string t1 ;

	string ind2 ;

	err.setRC( 0 ) ;

	field = parseString1( err, s, "FIELD()" ) ;
	if ( err.error() ) { return ; }
	if ( field == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE042G" ) ;
		return ;
	}

	iupper( field ) ;

	t1 = parseString1( err, s, "IND()" ) ;
	if ( err.error() ) { return ; }
	if ( t1 != "" )
	{
		p = t1.find( ',' ) ;
		if ( p != string::npos )
		{
			ind1 = t1.substr( 0, p ) ;
			ind2 = t1.substr( p+1 ) ;
			trim( ind1 ) ;
			trim( ind2 ) ;
			if ( ind2.size() != 4 || ind2.find( ' ' ) != string::npos || ind2.front() != '\'' || ind2.back() != '\'' )
			{
				err.seterrid( TRACE_INFO(), "PSYE046A" ) ;
				return ;
			}
			s4ind2 = ind2.substr( 1, 2 ) ;
			s2ind2 = s4ind2 ;
			s2ind2[ 0 ] = ' ' ;
			s3ind2 = s4ind2 ;
			s3ind2[ 1 ] = ' ' ;
		}
		else
		{
			ind1 = t1 ;
		}
		iupper( ind1 ) ;
		if ( !isvalidName( ind1 ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", ind1 ) ;
			return ;
		}
	}

	t1 = parseString1( err, s, "LIND()" ) ;
	if ( err.error() ) { return ; }
	if ( t1 != "" )
	{
		p = t1.find( ',' ) ;
		if ( p != string::npos )
		{
			lind1 = t1.substr( 0, p ) ;
			lind2 = t1.substr( p+1 ) ;
			trim( lind1 ) ;
			trim( lind2 ) ;
			if ( lind2.size() != 3 || lind2.find( ' ' ) != string::npos || lind2.front() != '\'' || lind2.back() != '\'' )
			{
				err.seterrid( TRACE_INFO(), "PSYE046B" ) ;
				return ;
			}
			lind2 = lind2.substr( 1, 1 ) ;
		}
		else
		{
			lind1 = t1 ;
		}
		iupper( lind1 ) ;
		if ( !isvalidName( lind1 ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", lind1 ) ;
			return ;
		}
	}

	t1 = parseString1( err, s, "RIND()" ) ;
	if ( err.error() ) { return ; }
	if ( t1 != "" )
	{
		p = t1.find( ',' ) ;
		if ( p != string::npos )
		{
			rind1 = t1.substr( 0, p ) ;
			rind2 = t1.substr( p+1 ) ;
			trim( rind1 ) ;
			trim( rind2 ) ;
			if ( rind2.size() != 3 || rind2.find( ' ' ) != string::npos || rind2.front() != '\'' || rind2.back() != '\'' )
			{
				err.seterrid( TRACE_INFO(), "PSYE046C" ) ;
				return ;
			}
			rind2   = rind2.substr( 1, 1 ) ;
		}
		else
		{
			rind1 = t1 ;
		}
		iupper( rind1 ) ;
		if ( !isvalidName( rind1 ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", rind1 ) ;
			return ;
		}
	}

	t1 = parseString1( err, s, "SIND()" ) ;
	if ( err.error() ) { return ; }
	if ( t1 != "" )
	{
		p = t1.find( ',' ) ;
		if ( p != string::npos )
		{
			sind1 = t1.substr( 0, p ) ;
			sind2 = t1.substr( p+1 ) ;
			trim( sind1 ) ;
			trim( sind2 ) ;
			if ( sind2.size() != 5 || sind2.find( ' ' ) != string::npos || sind2.front() != '\'' || sind2.back() != '\'' )
			{
				err.seterrid( TRACE_INFO(), "PSYE046D" ) ;
				return ;
			}
			sind2 = sind2.substr( 1, 3 ) ;
		}
		else
		{
			sind1 = t1 ;
		}
		iupper( sind1 ) ;
		if ( !isvalidName( sind1 ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", sind1 ) ;
			return ;
		}
	}

	scale = parseString1( err, s, "SCALE()" ) ;
	if ( err.error() ) { return ; }
	if ( scale != "" )
	{
		iupper( scale ) ;
		if ( !isvalidName( scale ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", scale ) ;
			return ;
		}
	}

	lcol = parseString1( err, s, "LCOL()" ) ;
	if ( err.error() ) { return ; }
	if ( lcol != "" )
	{
		iupper( lcol ) ;
		if ( !isvalidName( lcol ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", lcol ) ;
			return ;
		}
	}

	rcol = parseString1( err, s, "RCOL()" ) ;
	if ( err.error() ) { return ; }
	if ( rcol != "" )
	{
		iupper( rcol ) ;
		if ( !isvalidName( rcol ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", rcol ) ;
			return ;
		}
	}

	t1 = parseString1( err, s, "LEN()" ) ;
	if ( err.error() ) { return ; }
	if ( t1 != "" )
	{
		iupper( t1 ) ;
		if ( isvalidName( t1 ) )
		{
			lenvar = t1 ;
		}
		else
		{
			if ( !isnumeric( t1 ) || t1.size() > 5 )
			{
				err.seterrid( TRACE_INFO(), "PSYE046E", t1 ) ;
				return ;
			}
			len = ds2d( t1 ) ;
			if ( len < 1 || len > 32767 )
			{
				err.seterrid( TRACE_INFO(), "PSYE046E", t1 ) ;
				return ;
			}
		}
	}

	scroll = parseString1( err, s, "SCROLL()" ) ;
	if ( err.error() ) { return ; }
	if ( scroll != "" )
	{
		iupper( scroll ) ;
		if ( scroll == "OFF" || scroll == "NO" )
		{
			scroll_on = false ;
		}
		else if ( scroll == "NOLR" )
		{
			scroll_lr = false ;
		}
		else if ( scroll != "ON" && scroll != "YES" )
		{
			if ( !isvalidName( scroll ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE046F", scroll ) ;
				return ;
			}
			scrollvar = true ;
		}
	}

	if ( s != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", s ) ;
		return ;
	}
}


/* ************************************ ***************************** ********************************* */
/* ************************************   Short/Long message class    ********************************* */
/* ************************************ ***************************** ********************************* */


int slmsg::parse( const string& s,
		  const string& l )
{
	//
	// Parse message and fill the slmsg object.
	// .TYPE overrides .WINDOW and .ALARM
	//
	// RC =  0 Normal completion.
	// RC =  4
	// RC = 20
	//

	size_t p1 ;
	size_t p2 ;

	char c ;

	string rest ;
	string tmp  ;

	errblock err ;

	hlp   = ""    ;
	type  = IMT   ;
	smwin = false ;
	lmwin = false ;
	resp  = false ;
	alm   = false ;

	smsg = s ;
	lmsg = l ;

	c = smsg.front() ;
	if ( c == '\'' || c == '"' )
	{
		p1 = smsg.find_last_of( c ) ;
		if ( p1 == 0 ) { return 4 ; }
		rest = substr( smsg, p1+2 ) ;
		smsg = smsg.substr( 0, p1+1 ) ;
		smsg = dquote( err, c, smsg ) ;
		if ( err.error() ) { return 4 ; }
	}
	else if ( smsg.front() == '.' )
	{
		rest = smsg ;
		smsg = ""   ;
	}
	else
	{
		rest = subword( smsg, 2 ) ;
		smsg = word( smsg, 1 )    ;
	}

	p1 = rest.find( ".HELP" ) ;
	if ( p1 == string::npos )
	{
		p1 = rest.find( ".H" ) ;
		if ( p1 != string::npos )
		{
			rest.erase( p1, 2 ) ;
		}
	}
	else
	{
		rest.erase( p1, 5 ) ;
	}

	if ( p1 != string::npos )
	{
		p1 = rest.find_first_not_of( ' ', p1 ) ;
		if ( p1 == string::npos ) { return 4 ; }
		if ( rest[ p1 ] != '=' ) { return 4 ; }
		rest.erase( p1, 1 ) ;
		p1   = rest.find_first_not_of( ' ', p1 ) ;
		p2   = rest.find( ' ', p1 ) ;
		hlp  = rest.substr( p1, p2-p1 ) ;
		if ( hlp.front() == '&' )
		{
			hlp.erase( 0, 1 ) ;
			if ( !isvalidName( hlp ) ) { return 4 ; }
			dvhlp = hlp ;
		}
		rest.erase( p1, p2-p1+1 ) ;
	}

	p1 = rest.find( ".WINDOW" ) ;
	if ( p1 == string::npos )
	{
		p1 = rest.find( ".W" ) ;
		if ( p1 != string::npos )
		{
			rest.erase( p1, 2 ) ;
		}
	}
	else
	{
		rest.erase( p1, 7 ) ;
	}
	if ( p1 != string::npos )
	{
		lmwin = true ;
		p1 = rest.find_first_not_of( ' ', p1 ) ;
		if ( p1 == string::npos ) { return 4 ; }
		if ( rest[ p1 ] != '=' ) { return 4 ; }
		rest.erase( p1, 1 ) ;
		p1  = rest.find_first_not_of( ' ', p1 ) ;
		p2  = rest.find( ' ', p1 ) ;
		tmp = rest.substr( p1, p2-p1 ) ;
		trim( tmp ) ;
		if ( tmp.front() == '&' )
		{
			tmp.erase( 0, 1 ) ;
			if ( !isvalidName( tmp ) ) { return 4 ; }
			dvwin = tmp ;
		}
		else if ( tmp == "RESP" || tmp == "R"  )
		{
			smwin = true ;
			resp  = true ;
		}
		else if ( tmp == "NORESP" || tmp == "N" || tmp == "NR" )
		{
			smwin = true ;
		}
		else if ( tmp == "LRESP" || tmp == "LR" )
		{
			resp  = true ;
		}
		else if ( tmp != "LNORESP" && tmp != "LN" )
		{
			return 4 ;
		}
		rest.erase( p1, p2-p1+1 ) ;
	}


	p1 = rest.find( ".ALARM" ) ;
	if ( p1 == string::npos )
	{
		p1 = rest.find( ".A" ) ;
		if ( p1 != string::npos )
		{
			rest.erase( p1, 2 ) ;
		}
	}
	else
	{
		rest.erase( p1, 6 ) ;
	}
	if ( p1 != string::npos )
	{
		p1 = rest.find_first_not_of( ' ', p1 ) ;
		if ( p1 == string::npos ) { return 4 ; }
		if ( rest[ p1 ] != '=' ) { return 4 ; }
		rest.erase( p1, 1 ) ;
		p1  = rest.find_first_not_of( ' ', p1 ) ;
		p2  = rest.find( ' ', p1 ) ;
		tmp = rest.substr( p1, p2-p1 ) ;
		trim( tmp ) ;
		if ( tmp.front() == '&' )
		{
			tmp.erase( 0, 1 ) ;
			if ( !isvalidName( tmp ) ) { return 4 ; }
			dvalm = tmp ;
		}
		else if ( tmp == "YES" )
		{
			alm = true ;
		}
		else if ( tmp == "NO"  )
		{
			alm = false ;
		}
		else
		{
			return 4 ;
		}
		rest.erase( p1, p2-p1+1 ) ;
	}

	p1 = rest.find( ".TYPE" ) ;
	if ( p1 == string::npos )
	{
		p1 = rest.find( ".T" ) ;
		if ( p1 != string::npos )
		{
			rest.erase( p1, 2 ) ;
		}
	}
	else
	{
		rest.erase( p1, 5 ) ;
	}
	if ( p1 != string::npos )
	{
		p1 = rest.find_first_not_of( ' ', p1 ) ;
		if ( p1 == string::npos ) { return 4 ; }
		if ( rest[ p1 ] != '=' ) { return 4 ; }
		rest.erase( p1, 1 ) ;
		p1  = rest.find_first_not_of( ' ', p1 ) ;
		p2  = rest.find( ' ', p1 ) ;
		tmp = rest.substr( p1, p2-p1 ) ;
		trim( tmp ) ;
		if ( tmp.front() == '&' )
		{
			tmp.erase( 0, 1 ) ;
			if ( !isvalidName( tmp ) ) { return 4 ; }
			dvtype = tmp ;
		}
		else if ( tmp == "N" || tmp == "NOTIFY" || tmp == "INFO" )
		{
			type = IMT ;
			alm  = false ;
		}
		else if ( tmp == "W" || tmp == "WARNING" || tmp == "ERROR" )
		{
			type = WMT ;
			alm  = true ;
		}
		else if ( tmp == "A" || tmp == "ACTION" )
		{
			type = AMT ;
			alm  = true ;
		}
		else if ( tmp == "C" || tmp == "CRITICAL" )
		{
			type  = AMT ;
			alm   = true ;
			resp  = true ;
			smwin = true ;
			lmwin = true ;
		}
		else
		{
			return 4 ;
		}
		rest.erase( p1, p2-p1+1 ) ;
	}

	trim( rest ) ;

	if ( rest != "" && rest != "NOKANA" ) { return 4 ; }

	trim_right( smsg ) ;
	trim_right( lmsg ) ;

	return 0 ;
}


/* ************************************ ***************************** ********************************* */
/* ************************************      SELECT object class      ********************************* */
/* ************************************ ***************************** ********************************* */


bool selobj::parse( errblock& err,
		    string selstr )
{
	//
	// Case insensitive except for PARM() and CMD().
	//
	// Valid keyword formats:
	//
	// PGM(abc) PARM(xyz) NEWAPPL(abcd) NEWPOOL PASSLIB
	// PGM(abc) PARM(xyz) NEWAPPL NEWPOOL PASSLIB
	// CMD(abc def) LANG(REXX) - translates to PGM(&ZOREXPGM) PARM(abc def).
	// CMD(abc def)            - translates to PGM(&ZOREXPGM) PARM(abc def).
	// PANEL(def)   - translates to PGM(&ZPANLPGM) PARM(def).
	//
	// + SCRNAME(ghi) - give the function a screen name (valid name but not LIST, NEXT, PREV).
	// + SUSPEND      - Suspend any popup windows.
	//
	// Match brackets for PARM and CMD as these may contain brackets.  These can also be enclosed in
	// single quotes if needed, that are then removed.
	//

	int ob ;

	size_t p1 ;
	size_t p2 ;

	bool oquote ;

	string lang ;
	string str  ;

	err.setRC( 0 ) ;

	clear() ;
	str = upper( selstr ) ;
	p1  = pos( "PARM(", str ) ;
	if ( p1 > 0 )
	{
		ob     = 1 ;
		oquote = false ;
		for ( p2 = p1+4 ; p2 < selstr.size() ; ++p2 )
		{
			if ( selstr.at( p2 ) == '\'' ) { oquote = !oquote ; }
			if ( oquote ) { continue ; }
			if ( selstr.at( p2 ) == '(' ) { ++ob  ; }
			if ( selstr.at( p2 ) == ')' )
			{
				ob-- ;
				if ( ob == 0 ) { break ; }
			}
		}
		if ( ob != 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return false ;
		}
		if ( oquote )
		{
			err.seterrid( TRACE_INFO(), "PSYE033F" ) ;
			return false ;
		}
		++p2 ;
		parm   = strip( substr( selstr, (p1 + 5), (p2 - (p1 + 5)) ) ) ;
		parm   = strip( parm, 'B', '\'' ) ;
		selstr = delstr( selstr, p1, (p2 - p1 + 1) ) ;
		str    = upper( selstr ) ;
	}

	p1 = pos( "PGM(", str ) ;
	if ( p1 > 0 )
	{
		p2 = pos( ")", selstr, p1 ) ;
		if ( p2 == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return false ;
		}
		pgm    = strip( substr( str, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
		selstr = delstr( selstr, p1, (p2 - p1 + 1) ) ;
		str    = upper( selstr ) ;
		if ( !pgm.empty() && pgm.front() == '&' )
		{
			if ( !isvalidName( substr( pgm, 2 ) ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE031D", substr( pgm, 2 ) ) ;
				return false ;
			}
		}
		else
		{
			if ( !isvalidName( pgm ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE031E", "PROGRAM", pgm ) ;
				return false ;
			}
		}
	}
	else
	{
		p1 = pos( "PANEL(", str ) ;
		if ( p1 > 0 )
		{
			if ( parm != "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE039M", "PANEL" ) ;
				return false ;
			}
			p2 = pos( ")", selstr, p1 ) ;
			if ( p2 == 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
				return false ;
			}
			parm = strip( substr( str, (p1 + 6), (p2 - (p1 + 6)) ) ) ;
			if ( !isvalidName( parm ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE031E", "PANEL", parm ) ;
				return false ;
			}
			pgmtype = PGM_PANEL ;
			selstr  = delstr( selstr, p1, (p2 - p1 + 1) ) ;
			str     = upper( selstr ) ;
			p1 = pos( "OPT(", str ) ;
			if ( p1 > 0 )
			{
				p2 = pos( ")", selstr, p1 ) ;
				if ( p2 == 0 )
				{
					err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
					return false ;
				}
				parm  += " " + strip( substr( str, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
				selstr = delstr( selstr, p1, (p2 - p1 + 1) ) ;
				str    = upper( selstr ) ;
			}
			selPanl = true ;
		}
		else
		{
			p1 = pos( "CMD(", str ) ;
			if ( p1 > 0 )
			{
				if ( parm != "" )
				{
					err.seterrid( TRACE_INFO(), "PSYE039M", "CMD" ) ;
					return false ;
				}
				ob     = 1 ;
				oquote = false ;
				for ( p2 = p1+3 ; p2 < selstr.size() ; ++p2 )
				{
					if ( selstr.at( p2 ) == '"' ) { oquote = !oquote ; }
					if ( oquote ) { continue ; }
					if ( selstr.at( p2 ) == '(' ) { ++ob  ; }
					if ( selstr.at( p2 ) == ')' )
					{
						ob-- ;
						if ( ob == 0 ) { break ; }
					}
				}
				if ( ob != 0 )
				{
					err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
					return false ;
				}
				if ( oquote )
				{
					err.seterrid( TRACE_INFO(), "PSYE033F" ) ;
					return false ;
				}
				++p2 ;
				parm   = strip( substr( selstr, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
				parm   = strip( parm, 'B', '"' ) ;
				selstr = delstr( selstr, p1, (p2 - p1 + 1) ) ;
				str    = upper( selstr ) ;
				lang   = "" ;
				p1     = pos( "LANG(", str ) ;
				if ( p1 > 0 )
				{
					p2 = pos( ")", selstr, p1 ) ;
					if ( p2 == 0 )
					{
						err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
						return false ;
					}
					lang   = strip( substr( str, (p1 + 5), (p2 - (p1 + 5)) ) ) ;
					selstr = delstr( selstr, p1, (p2 - p1 + 1) ) ;
					str    = upper( selstr ) ;
				}
				if ( lang == "" || lang == "REXX" )
				{
					pgmtype = PGM_REXX ;
				}
				else if ( lang == "SHELL" )
				{
					pgmtype = PGM_SHELL ;
				}
				else
				{
					err.seterrid( TRACE_INFO(), "PSYE039N", lang ) ;
					return false ;
				}
			}
		}
	}

	p1 = pos( "SCRNAME(", str ) ;
	if ( p1 > 0 )
	{
		p2 = pos( ")", str, p1 ) ;
		if ( p2 == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return false ;
		}
		scrname = strip( substr( str, (p1 + 8), (p2 - (p1 + 8)) ) ) ;
		str     = delstr( str, p1, (p2 - p1 + 1) ) ;
		if ( findword( scrname, "LIST NEXT PREV" ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE039O" ) ;
			return false ;
		}
		if ( !isvalidName( scrname ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE039P", scrname ) ;
			return false ;
		}
	}

	p1 = pos( "NEWAPPL(", str ) ;
	if ( p1 > 0 )
	{
		p2 = pos( ")", str, p1 ) ;
		if ( p2 == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return false ;
		}
		newappl = strip( substr( str, (p1 + 8), (p2 - (p1 + 8)) ) ) ;
		newpool = true ;
		str     = delstr( str, p1, (p2 - p1 + 1) ) ;
		if ( !isvalidName4( newappl ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031E", "NEWAPPL", newappl ) ;
			return false ;
		}
		if ( newappl == "ISPS" )
		{
			err.seterrid( TRACE_INFO(), "PSYE039S" ) ;
			return false ;
		}
	}
	else
	{
		p1 = wordpos( "NEWAPPL", str ) ;
		if ( p1 > 0 )
		{
			newappl = "ISP";
			newpool = true ;
			idelword( str, p1, 1 ) ;
		}
	}

	p1 = wordpos( "NEWPOOL", str ) ;
	if ( p1 > 0 )
	{
		newpool = true ;
		idelword( str, p1, 1 ) ;
	}

	p1 = wordpos( "NESTED", str ) ;
	if ( p1 > 0 )
	{
		nested = true ;
		idelword( str, p1, 1 ) ;
	}

	p1 = wordpos( "NOFUNC", str ) ;
	if ( p1 > 0 )
	{
		nofunc = true ;
		idelword( str, p1, 1 ) ;
	}

	p1 = wordpos( "SUSPEND", str ) ;
	if ( p1 > 0 )
	{
		suspend = true ;
		idelword( str, p1, 1 ) ;
	}

	p1 = wordpos( "PASSLIB", str ) ;
	if ( p1 > 0 )
	{
		if ( newappl == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE039Q" ) ;
			return false ;
		}
		passlib = true ;
		idelword( str, p1, 1 ) ;
	}

	if ( pgm == "" && pgmtype == PGM_NONE )
	{
		err.seterrid( TRACE_INFO(), "PSYE039R" ) ;
		return false ;
	}

	if ( trim( str ) != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", str ) ;
		return false ;
	}

	return true ;
}


/* ************************************ ***************************** ********************************* */
/* ************************************      Char Attribute class     ********************************* */
/* ************************************ ***************************** ********************************* */


void char_attrs::setattr( errblock& err,
			  const string& attrs )
{
	//
	// Create new values using attrs.
	// Keep a copy of the original string if it contains dialogue variables.
	//
	// Used in initial ATTR section setup during panel parsing.
	//

	if ( strip( attrs ) == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE035L" ) ;
		return ;
	}

	dvars = ( attrs.find( '&' ) != string::npos ) ;

	if ( dvars )
	{
		entry1 = attrs ;
	}

	parse_attrs( err, attrs, true ) ;
}


void char_attrs::update( errblock& err,
			 const string& attrs )
{
	//
	// Update existing values with what is in attrs.
	//
	// Only used for variable substitution after )INIT processing.
	//
	// TYPE can only be changed between input and output attributes.
	//

	bool is_text = is_text_attr() ;

	clear() ;

	parse_attrs( err, attrs ) ;

	if ( err.ok() )
	{
		if ( is_text != is_text_attr() )
		{
			err.seterrid( TRACE_INFO(), "PSYE035P" ) ;
			return ;
		}
	}
}


void char_attrs::update( errblock& err,
			 const char_attrs* attrchar )
{
	//
	// Update existing values with what is in attrchar if it has an override, and re-appy entry2.
	//
	// Used to update field inline ATTR() entry with the current ATTRCHAR() override.
	//

	bool temp ;

	if ( attrchar->ovrd_attrs )
	{
		temp  = once ;
		*this = *attrchar->ovrd_attrs ;
		once  = temp ;
		parse_attrs( err, entry2 ) ;
	}
}


const string& char_attrs::get_entry1()
{
	//
	// Return entry1 (only set if it contains substitutable variables).
	//

	return entry1 ;
}


void char_attrs::override_attrs( errblock& err,
				 const string& attrs,
				 bool chng_once,
				 bool save_attrs )
{
	//
	// Remove the override structure if there is one, and copy the existing
	// values to a new override structure and apply the passed attribute changes.
	//
	// If the TYPE has changed, check it is an allowed change.
	//
	// Used for .ATTR and .ATTRCHAR processing.
	//

	if ( save_attrs )
	{
		entry2 = attrs ;
	}

	delete ovrd_attrs ;
	ovrd_attrs = new char_attrs( *this ) ;

	ovrd_attrs->parse_attrs( err, attrs, false, true, cua ) ;
	if ( err.error() ) { return ; }

	if ( ( type != ovrd_attrs->type ) &&
	     ( ( is_text_attr() && ( ovrd_attrs->is_input_attr()  || ovrd_attrs->is_output_attr() ) ) ||
	       ( is_input_attr()  && ovrd_attrs->is_text_attr() ) ||
	       ( is_output_attr() && ovrd_attrs->is_text_attr() ) ||
	       ( !is_input_attr() && ovrd_attrs->type == EE ) ||
	       ( type == PS ) ||
	       ( type == RP ) ||
	       ( ovrd_attrs->type == PS ) ||
	       ( ovrd_attrs->type == RP ) ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE035E", attrs ) ;
		return ;
	}

	once = chng_once ;
}


void char_attrs::remove_override()
{
	//
	// Remove the override stucture.
	//

	if ( ovrd_attrs )
	{
		delete ovrd_attrs ;
		ovrd_attrs = nullptr ;
		once       = false ;
	}
}


void char_attrs::remove_override_once()
{
	//
	// Remove the override stucture set for one redisplay.
	//

	if ( ovrd_attrs && once )
	{
		delete ovrd_attrs ;
		ovrd_attrs = nullptr ;
		once       = false ;
	}
}


void char_attrs::parse_attrs( errblock& err,
			      string attrs,
			      bool p_init,
			      bool p_ovrride,
			      bool p_cua )
{
	//
	// Parser for the attribute type statement and .ATTR/.ATTRCHAR statements.
	//
	// Currently supported parameters:
	// 1)  TYPE    - CUA or basic type.
	// 2)  COLOUR  }
	// 3)  INTENS  } For type INPUT, OUTPUT and TEXT only.
	// 4)  HILITE  }
	// 5)  PAS     - define INPUT/OUTPUT field as a point-and-shoot field.
	// 6)  UNAVAIL - only used for CUA type SC (use SAC for available, SUC for unavailable).
	// 7)  CUADYN  - syntax checked but only used on types DATAIN/DATAOUT.
	// 8)  CAPS    - ON | OFF | IN | OUT
	// 9)  JUST    - LEFT | RIGHT | ASIS
	// 10) NUMERIC - ON | OFF
	// 11) PAD     - char | NULLS | USER
	// 12) PADC    - char | NULLS | USER
	// 13) SKIP    - ON | OFF
	// 14) NOJUMP  - ON | OFF
	// 15) PASSWD  - ON | OFF (type NEF and INPUT only)
	//
	// For overrides:
	//    CUA   -> basic type.  Only the type changes.
	//    basic -> CUA.  All attributes are now that of the CUA type.
	//
	// Keywords are processed in the order they appear (except for TYPE in an )ATTR section).
	//

	string temp ;

	bool ex_type    = false ;
	bool ex_colour  = false ;
	bool ex_intens  = false ;
	bool ex_hilite  = false ;
	bool ex_cuadyn  = false ;
	bool ex_unavail = false ;
	bool ex_pas     = false ;
	bool ex_pad     = false ;
	bool ex_padc    = false ;
	bool ex_skip    = false ;
	bool ex_nojump  = false ;
	bool ex_caps    = false ;
	bool ex_just    = false ;
	bool ex_numeric = false ;
	bool ex_passwd  = false ;

	err.setRC( 0 ) ;

	trim_left( iupper( attrs ) ) ;

	if ( p_init )
	{
		temp = parseString1( err, attrs, "TYPE()" ) ;
		if ( err.error() ) { return ; }
		if ( temp == "" )
		{
			if ( err.RSN0() )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
		}
		else
		{
			parse_attrs_type( err, temp, p_ovrride, p_cua, p_init ) ;
			if ( err.error() ) { return ; }
			ex_type = true ;
		}
		set_defaults() ;
	}

	while ( attrs != "" )
	{
		if ( attrs.compare( 0, 4, "TYPE" ) == 0 )
		{
			temp = parseString1( err, attrs, "TYPE()" ) ;
			if ( err.error() ) { return ; }
			if ( ex_type )
			{
				err.seterrid( TRACE_INFO(), "PSYE035F", "TYPE" ) ;
				return ;
			}
			if ( temp == "" ) { continue ; }
			parse_attrs_type( err, temp, p_ovrride, p_cua, p_init ) ;
			if ( err.error() ) { return ; }
			ex_type = true ;
		}
		else if ( attrs.compare( 0, 6, "COLOUR" ) == 0 )
		{
			if ( cua )
			{
				err.seterrid( TRACE_INFO(), "PSYE035D" ) ;
				return ;
			}
			temp = parseString1( err, attrs, "COLOUR()" ) ;
			if ( err.error() ) { return ; }
			if ( p_init && temp == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			if ( ex_colour )
			{
				err.seterrid( TRACE_INFO(), "PSYE035F", "COLOUR" ) ;
				return ;
			}
			if ( temp == "" ) { continue ; }
			parse_attrs_colour( err, temp, p_init ) ;
			if ( err.error() ) { return ; }
			ex_colour = true ;
		}
		else if ( attrs.compare( 0, 5, "COLOR" ) == 0 )
		{
			if ( cua )
			{
				err.seterrid( TRACE_INFO(), "PSYE035D" ) ;
				return ;
			}
			temp = parseString1( err, attrs, "COLOR()" ) ;
			if ( err.error() ) { return ; }
			if ( p_init && temp == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			if ( ex_colour )
			{
				err.seterrid( TRACE_INFO(), "PSYE035F", "COLOR" ) ;
				return ;
			}
			if ( temp == "" ) { continue ; }
			parse_attrs_colour( err, temp, p_init ) ;
			if ( err.error() ) { return ; }
			ex_colour = true ;
		}
		else if ( attrs.compare( 0, 6, "INTENS" ) == 0 )
		{
			temp = parseString1( err, attrs, "INTENS()" ) ;
			if ( cua && temp != "NON" )
			{
				err.seterrid( TRACE_INFO(), "PSYE035D" ) ;
				return ;
			}
			if ( err.error() ) { return ; }
			if ( p_init && temp == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			if ( ex_intens )
			{
				err.seterrid( TRACE_INFO(), "PSYE035F", "INTENS" ) ;
				return ;
			}
			if ( temp == "" ) { continue ; }
			parse_attrs_intens( err, temp, p_init ) ;
			if ( err.error() ) { return ; }
			ex_intens = true ;
		}
		else if ( attrs.compare( 0, 6, "HILITE" ) == 0 )
		{
			if ( cua )
			{
				err.seterrid( TRACE_INFO(), "PSYE035D" ) ;
				return ;
			}
			temp = parseString1( err, attrs, "HILITE()" ) ;
			if ( err.error() ) { return ; }
			if ( p_init && temp == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			if ( ex_hilite )
			{
				err.seterrid( TRACE_INFO(), "PSYE035F", "HILITE" ) ;
				return ;
			}
			if ( temp == "" ) { continue ; }
			parse_attrs_hilite( err, temp, p_init ) ;
			if ( err.error() ) { return ; }
			ex_hilite = true ;
		}
		else if ( attrs.compare( 0, 7, "UNAVAIL" ) == 0 )
		{
			temp = parseString1( err, attrs, "UNAVAIL()" ) ;
			if ( err.error() ) { return ; }
			if ( p_init && temp == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			if ( ex_unavail )
			{
				err.seterrid( TRACE_INFO(), "PSYE035F", "UNAVAIL" ) ;
				return ;
			}
			if ( temp == "" ) { continue ; }
			parse_attrs_unavail( err, temp, p_init ) ;
			if ( err.error() ) { return ; }
			ex_unavail = true ;
		}
		else if ( attrs.compare( 0, 6, "CUADYN" ) == 0 )
		{
			temp = parseString1( err, attrs, "CUADYN()" ) ;
			if ( err.error() ) { return ; }
			if ( p_init && temp == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			if ( ex_cuadyn )
			{
				err.seterrid( TRACE_INFO(), "PSYE035F", "CUADYN" ) ;
				return ;
			}
			if ( temp == "" ) { continue ; }
			parse_attrs_cuadyn( err, temp, p_init ) ;
			if ( err.error() ) { return ; }
			ex_cuadyn = true ;
		}
		else if ( attrs.compare( 0, 4, "CAPS" ) == 0 )
		{
			temp = parseString1( err, attrs, "CAPS()" ) ;
			if ( err.error() ) { return ; }
			if ( p_init && temp == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			if ( ex_caps )
			{
				err.seterrid( TRACE_INFO(), "PSYE035F", "CAPS" ) ;
				return ;
			}
			if ( temp == "" ) { continue ; }
			parse_attrs_caps( err, temp, p_init ) ;
			if ( err.error() ) { return ; }
			ex_caps = true ;
		}
		else if ( attrs.compare( 0, 4, "JUST" ) == 0 )
		{
			temp = parseString1( err, attrs, "JUST()" ) ;
			if ( err.error() ) { return ; }
			if ( p_init && temp == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			if ( ex_just )
			{
				err.seterrid( TRACE_INFO(), "PSYE035F", "JUST" ) ;
				return ;
			}
			if ( temp == "" ) { continue ; }
			parse_attrs_just( err, temp, p_init ) ;
			if ( err.error() ) { return ; }
			ex_just = true ;
		}
		else if ( attrs.compare( 0, 7, "NUMERIC" ) == 0 )
		{
			temp = parseString1( err, attrs, "NUMERIC()" ) ;
			if ( err.error() ) { return ; }
			if ( p_init && temp == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			if ( ex_numeric )
			{
				err.seterrid( TRACE_INFO(), "PSYE035F", "NUMERIC" ) ;
				return ;
			}
			if ( temp == "" ) { continue ; }
			parse_attrs_numeric( err, temp, p_init ) ;
			if ( err.error() ) { return ; }
			ex_numeric = true ;
		}
		else if ( attrs.compare( 0, 4, "PADC" ) == 0 )
		{
			temp = parseString1( err, attrs, "PADC()" ) ;
			if ( err.error() ) { return ; }
			if ( p_init && temp == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			if ( ex_pad )
			{
				err.seterrid( TRACE_INFO(), "PSYE035K", "PAD", "PADC" ) ;
				return ;
			}
			if ( ex_padc )
			{
				err.seterrid( TRACE_INFO(), "PSYE035F", "PADC" ) ;
				return ;
			}
			if ( temp == "" ) { continue ; }
			parse_attrs_pad( err, temp, p_init ) ;
			if ( err.error() ) { return ; }
			padc    = true ;
			ex_padc = true ;
		}
		else if ( attrs.compare( 0, 3, "PAD" ) == 0 )
		{
			temp = parseString1( err, attrs, "PAD()" ) ;
			if ( err.error() ) { return ; }
			if ( p_init && temp == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			if ( ex_pad )
			{
				err.seterrid( TRACE_INFO(), "PSYE035F", "PAD" ) ;
				return ;
			}
			if ( ex_padc )
			{
				err.seterrid( TRACE_INFO(), "PSYE035K", "PAD", "PADC" ) ;
				return ;
			}
			if ( temp == "" ) { continue ; }
			parse_attrs_pad( err, temp, p_init ) ;
			if ( err.error() ) { return ; }
			padc   = false ;
			ex_pad = true ;
		}
		else if ( attrs.compare( 0, 4, "SKIP" ) == 0 )
		{
			temp = parseString1( err, attrs, "SKIP()" ) ;
			if ( err.error() ) { return ; }
			if ( p_init && temp == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			if ( ex_skip )
			{
				err.seterrid( TRACE_INFO(), "PSYE035F", "SKIP" ) ;
				return ;
			}
			if ( temp == "" ) { continue ; }
			parse_attrs_skip( err, temp, p_init ) ;
			if ( err.error() ) { return ; }
			ex_skip = true ;
		}
		else if ( attrs.compare( 0, 6, "NOJUMP" ) == 0 )
		{
			temp = parseString1( err, attrs, "NOJUMP()" ) ;
			if ( err.error() ) { return ; }
			if ( p_init && temp == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			if ( ex_nojump )
			{
				err.seterrid( TRACE_INFO(), "PSYE035F", "NOJUMP" ) ;
				return ;
			}
			if ( temp == "" ) { continue ; }
			parse_attrs_nojump( err, temp, p_init ) ;
			if ( err.error() ) { return ; }
			ex_nojump = true ;
		}
		else if ( attrs.compare( 0, 6, "PASSWD" ) == 0 )
		{
			temp = parseString1( err, attrs, "PASSWD()" ) ;
			if ( err.error() ) { return ; }
			if ( p_init && temp == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			if ( ex_passwd )
			{
				err.seterrid( TRACE_INFO(), "PSYE035F", "PASSWD" ) ;
				return ;
			}
			if ( temp == "" ) { continue ; }
			parse_attrs_passwd( err, temp, p_init ) ;
			if ( err.error() ) { return ; }
			ex_passwd = true ;
		}
		else if ( attrs.compare( 0, 3, "PAS" ) == 0 )
		{
			temp = parseString1( err, attrs, "PAS()" ) ;
			if ( err.error() ) { return ; }
			if ( p_init && temp == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
				return ;
			}
			if ( ex_pas )
			{
				err.seterrid( TRACE_INFO(), "PSYE035F", "PAS" ) ;
				return ;
			}
			if ( temp == "" ) { continue ; }
			parse_attrs_pas( err, temp, p_init ) ;
			if ( err.error() ) { return ; }
			ex_pas = true ;
		}
		else if ( attrs != "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE032H", attrs ) ;
			return ;
		}
	}

	if ( !cua )
	{
		if ( type == CHAR )
		{
			if ( nintens )
			{
				intens = A_NORMAL ;
			}
			if ( ncolour )
			{
				colour = BLUE ;
			}
		}
		else
		{
			if ( nintens )
			{
				intens = ( colour == BLUE || colour == GREEN || colour == TURQ ) ? A_NORMAL : A_BOLD ;
			}
			if ( ncolour )
			{
				colour = ( input ) ? ( intens == A_BOLD ) ? RED   : GREEN :
						     ( intens == A_BOLD ) ? WHITE : BLUE ;
			}
		}
	}
}


void char_attrs::parse_attrs_type( errblock& err,
				   string& attr,
				   bool p_ovrride,
				   bool p_cua,
				   bool p_init )
{
	map<string, attType> type_map =
	{ { "CEF",     CEF    },
	  { "CH",      CH     },
	  { "CHAR",    CHAR   },
	  { "CT",      CT     },
	  { "DATAIN",  DATAIN },
	  { "DATAOUT", DATAOUT},
	  { "DT",      DT     },
	  { "EE",      EE     },
	  { "ET",      ET     },
	  { "FP",      FP     },
	  { "INPUT",   INPUT  },
	  { "LEF",     LEF    },
	  { "LI",      LI     },
	  { "LID",     LID    },
	  { "NEF",     NEF    },
	  { "NT",      NT     },
	  { "OUTPUT",  OUTPUT },
	  { "PIN",     PIN    },
	  { "PS",      PS     },
	  { "PT",      PT     },
	  { "RP",      RP     },
	  { "SAC",     SAC    },
	  { "SC",      SC     },
	  { "SI",      SI     },
	  { "SUC",     SUC    },
	  { "TEXT",    TEXT   },
	  { "VOI",     VOI    },
	  { "WASL",    WASL   },
	  { "WT",      WT     } } ;

	set<string> none_cua_map = { "CHAR",
				     "DATAIN",
				     "DATAOUT",
				     "INPUT",
				     "OUTPUT",
				     "TEXT" } ;


	if ( p_init && attr.front() == '&' )
	{
		if ( !isvalidName( attr.substr( 1 ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", attr.substr( 1 ) ) ;
		}
		return ;
	}

	auto it = type_map.find( attr ) ;
	if ( it == type_map.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE035B", attr, "TYPE" ) ;
		return ;
	}

	cua = ( none_cua_map.count( attr ) == 0 ) ;

	if ( findword( attr, "TEXT CH CT DT ET FP NT PIN PS PT RP SAC SI SUC WASL WT" ) )
	{
		text   = true  ;
		input  = false ;
		output = false ;
		nojump = false ;
		pas    = false ;
	}
	else
	{
		text   = false ;
		input  = findword( attr, "INPUT CEF EE LEF NEF" ) ;
		output = !input ;
		if ( !input )
		{
			numeric = false ;
		}
	}

	type = it->second ;

	if ( p_ovrride && !p_cua && cua )
	{
		char_attrs* temp = new char_attrs( type ) ;
		*this = *temp ;
		delete temp ;
	}
}


void char_attrs::parse_attrs_colour( errblock& err,
				     string& attr,
				     bool p_init )
{
	map<string, unsigned int> colour_map =
	{ { "RED",       RED     },
	  { "GREEN",     GREEN   },
	  { "YELLOW",    YELLOW  },
	  { "BLUE",      BLUE    },
	  { "MAGENTA",   MAGENTA },
	  { "PINK",      MAGENTA },
	  { "TURQ",      TURQ    },
	  { "TURQUOISE", TURQ    },
	  { "WHITE",     WHITE   } } ;

	if ( p_init && attr.front() == '&' )
	{
		if ( !isvalidName( attr.substr( 1 ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", attr.substr( 1 ) ) ;
			return ;
		}
	}
	else
	{
		auto it = colour_map.find( attr ) ;
		if ( it == colour_map.end() )
		{
			err.seterrid( TRACE_INFO(), "PSYE035B", attr, "COLOUR" ) ;
			return ;
		}
		colour  = it->second ;
		ncolour = false ;
	}
}


void char_attrs::parse_attrs_intens( errblock& err,
				     string& attr,
				     bool p_init )
{
	map<string, unsigned int> intens_map =
	{ { "HIGH", A_BOLD   },
	  { "LOW",  A_NORMAL },
	  { "NON",  A_INVIS  } } ;

	if ( p_init && attr.front() == '&' )
	{
		if ( !isvalidName( attr.substr( 1 ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", attr.substr( 1 ) ) ;
			return ;
		}
	}
	else
	{
		auto it = intens_map.find( attr ) ;
		if ( it == intens_map.end() )
		{
			err.seterrid( TRACE_INFO(), "PSYE035B", attr, "INTENS" ) ;
			return ;
		}
		intens  = it->second ;
		nintens = false ;
	}
}


void char_attrs::parse_attrs_hilite( errblock& err,
				     string& attr,
				     bool p_init )
{
	map<string, unsigned int> hilite_map =
	{ { "BLINK",   A_BLINK     },
	  { "REVERSE", A_REVERSE   },
	  { "USCORE",  A_UNDERLINE } } ;

	if ( p_init && attr.front() == '&' )
	{
		if ( !isvalidName( attr.substr( 1 ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", attr.substr( 1 ) ) ;
			return ;
		}
	}
	else
	{
		auto it = hilite_map.find( attr ) ;
		if ( it == hilite_map.end() )
		{
			err.seterrid( TRACE_INFO(), "PSYE035B", attr, "HILITE" ) ;
			return ;
		}
		hilite  = it->second ;
		nhilite = false ;
	}
}


void char_attrs::parse_attrs_unavail( errblock& err,
				      string& attr,
				      bool p_init )
{
	if ( p_init && attr.front() == '&' )
	{
		if ( !isvalidName( attr.substr( 1 ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", attr.substr( 1 ) ) ;
			return ;
		}
	}
	else if ( attr == "ON" )
	{
		if ( type == SC ) { avail = false ; }
	}
	else if ( attr == "OFF" )
	{
		if ( type == SC ) { avail = true ; }
	}
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE035C", "UNAVAIL" ) ;
		return ;
	}
}


void char_attrs::parse_attrs_pas( errblock& err,
				  string& attr,
				  bool p_init )
{
	if ( text )
	{
		err.seterrid( TRACE_INFO(), "PSYE035G" ) ;
		return ;
	}

	if ( p_init && attr.front() == '&' )
	{
		if ( !isvalidName( attr.substr( 1 ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", attr.substr( 1 ) ) ;
			return ;
		}
	}
	else if ( attr == "ON" )
	{
		pas = true ;
	}
	else if ( attr == "OFF" )
	{
		pas = false ;
	}
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE035C", "PAS" ) ;
		return ;
	}

	if ( pas && is_text() )
	{
		err.seterrid( TRACE_INFO(), "PSYE035J", "PAS(ON)" ) ;
		return ;
	}
}


void char_attrs::parse_attrs_cuadyn( errblock& err,
				     string& attr,
				     bool p_init )
{
	map<string, attType> type_map =
	{ { "CEF",     CEF    },
	  { "CH",      CH     },
	  { "CHAR",    CHAR   },
	  { "CT",      CT     },
	  { "DATAIN",  DATAIN },
	  { "DATAOUT", DATAOUT},
	  { "DT",      DT     },
	  { "EE",      EE     },
	  { "ET",      ET     },
	  { "FP",      FP     },
	  { "INPUT",   INPUT  },
	  { "LEF",     LEF    },
	  { "LI",      LI     },
	  { "LID",     LID    },
	  { "NEF",     NEF    },
	  { "NT",      NT     },
	  { "OUTPUT",  OUTPUT },
	  { "PIN",     PIN    },
	  { "PS",      PS     },
	  { "PT",      PT     },
	  { "RP",      RP     },
	  { "SAC",     SAC    },
	  { "SC",      SC     },
	  { "SI",      SI     },
	  { "SUC",     SUC    },
	  { "TEXT",    TEXT   },
	  { "VOI",     VOI    },
	  { "WASL",    WASL   },
	  { "WT",      WT     } } ;

	set<string> none_cua_map = { "CHAR",
				     "DATAIN",
				     "DATAOUT",
				     "INPUT",
				     "OUTPUT",
				     "TEXT" } ;

	if ( p_init && attr.front() == '&' )
	{
		if ( !isvalidName( attr.substr( 1 ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", attr.substr( 1 ) ) ;
			return ;
		}
	}
	else
	{
		auto it1 = type_map.find( attr ) ;
		auto it2 = none_cua_map.find( attr ) ;
		if ( it1 == type_map.end() || it2 != none_cua_map.end() )
		{
			err.seterrid( TRACE_INFO(), "PSYE035B", attr, "CUADYN" ) ;
			return ;
		}
		if ( type == DATAIN )
		{
			if ( it1->second == CEF ||
			     it1->second == EE  ||
			     it1->second == LEF ||
			     it1->second == NEF )
			{
				cuadyn = it1->second ;
			}
		}
		else if ( type == DATAOUT )
		{
			if ( it1->second == CH   ||
			     it1->second == CT   ||
			     it1->second == DT   ||
			     it1->second == ET   ||
			     it1->second == FP   ||
			     it1->second == LI   ||
			     it1->second == LID  ||
			     it1->second == NT   ||
			     it1->second == PIN  ||
			     it1->second == PT   ||
			     it1->second == SAC  ||
			     it1->second == SI   ||
			     it1->second == SUC  ||
			     it1->second == VOI  ||
			     it1->second == WASL ||
			     it1->second == WT )
			{
				cuadyn = it1->second ;
			}
		}
	}
}


void char_attrs::parse_attrs_caps( errblock& err,
				   string& attr,
				   bool p_init )
{
	//
	// ON  - Upper case translation to/from function pool
	// OFF - No translation
	// IN  - Upper case translation before being stored in the function pool
	// OUT - Upper case translation before being displayed.
	//

	if ( p_init && attr.front() == '&' )
	{
		if ( !isvalidName( attr.substr( 1 ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", attr.substr( 1 ) ) ;
			return ;
		}
	}
	else if ( attr == "ON" )
	{
		caps1 = CAPS_ON ;
	}
	else if ( attr == "IN" )
	{
		caps1 = CAPS_IN ;
	}
	else if ( attr == "OFF" )
	{
		caps1 = CAPS_OFF ;
	}
	else if ( attr == "OUT" )
	{
		caps1 = CAPS_OUT ;
	}
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE035O" ) ;
		return ;
	}
	caps2 = caps1 ;
}


void char_attrs::parse_attrs_just( errblock& err,
				   string& attr,
				   bool p_init )
{
	if ( p_init && attr.front() == '&' )
	{
		if ( !isvalidName( attr.substr( 1 ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", attr.substr( 1 ) ) ;
			return ;
		}
	}
	else if ( attr == "LEFT" )
	{
		just1 = 'L' ;
	}
	else if ( attr == "RIGHT" )
	{
		just1 = 'R' ;
	}
	else if ( attr == "ASIS" )
	{
		just1 = 'A' ;
	}
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE035M" ) ;
		return ;
	}
       just2 = just1 ;
}


void char_attrs::parse_attrs_numeric( errblock& err,
				      string& attr,
				      bool p_init )
{
	if ( p_init && attr.front() == '&' )
	{
		if ( !isvalidName( attr.substr( 1 ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", attr.substr( 1 ) ) ;
			return ;
		}
	}
	else if ( attr == "ON" )
	{
		numeric = true ;
	}
	else if ( attr == "OFF" )
	{
		numeric = false ;
	}
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE035C", "NUMERIC" ) ;
		return ;
	}

	if ( numeric && !is_input() )
	{
		err.seterrid( TRACE_INFO(), "PSYE035I", "NUMERIC(ON)" ) ;
		return ;
	}
}


void char_attrs::parse_attrs_pad( errblock& err,
				  string& attr,
				  bool p_init )
{
	char quote ;

	if ( p_init && attr.front() == '&' )
	{
		if ( !isvalidName( attr.substr( 1 ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", attr.substr( 1 ) ) ;
			return ;
		}
	}
	else if ( attr == "USER" )
	{
		paduser = true ;
	}
	else if ( attr == "NULLS" || attr == "\'\'" )
	{
		paduser = false ;
		padchar = 0x00 ;
	}
	else if ( attr == "\'\'\'\'" )
	{
		paduser = false ;
		padchar = 0x27 ;
	}
	else
	{
		quote = attr.front() ;
		if ( quote == '\'' || quote == '"' )
		{
			if ( attr.size() > 1 && attr.back() != quote )
			{
				err.seterrid( TRACE_INFO(), "PSYE033F" ) ;
				return ;
			}
			attr.pop_back() ;
			attr.erase( 0, 1 ) ;
		}
		if ( attr.size() != 1 )
		{
			err.seterrid( TRACE_INFO(), "PSYE035N", attr ) ;
			return ;
		}
		padchar = attr.front() ;
		paduser = false ;
	}
}


void char_attrs::parse_attrs_skip( errblock& err,
				   string& attr,
				   bool p_init )
{
	if ( p_init && attr.front() == '&' )
	{
		if ( !isvalidName( attr.substr( 1 ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", attr.substr( 1 ) ) ;
			return ;
		}
	}
	else if ( attr == "ON" )
	{
		skip = true ;
	}
	else if ( attr == "OFF" )
	{
		skip = false ;
	}
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE035C", "SKIP" ) ;
		return ;
	}
}


void char_attrs::parse_attrs_nojump( errblock& err,
				     string& attr,
				     bool p_init )
{
	if ( p_init && attr.front() == '&' )
	{
		if ( !isvalidName( attr.substr( 1 ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", attr.substr( 1 ) ) ;
			return ;
		}
	}
	else if ( attr == "ON" )
	{
		nojump = true ;
	}
	else if ( attr == "OFF" )
	{
		nojump = false ;
	}
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE035C", "NOJUMP" ) ;
		return ;
	}

	if ( nojump && is_text() )
	{
		err.seterrid( TRACE_INFO(), "PSYE036G", "NOJUMP(ON)" ) ;
		return ;
	}
}


void char_attrs::parse_attrs_passwd( errblock& err,
				     string& attr,
				     bool p_init )
{
	if ( type != NEF && type != INPUT )
	{
		err.seterrid( TRACE_INFO(), "PSYE035H" ) ;
		return ;
	}

	if ( p_init && attr.front() == '&' )
	{
		if ( !isvalidName( attr.substr( 1 ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE031D", attr.substr( 1 ) ) ;
			return ;
		}
	}
	else if ( attr == "ON" )
	{
		passwd = true ;
	}
	else if ( attr == "OFF" )
	{
		passwd = false ;
	}
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE035C", "PASSWD" ) ;
		return ;
	}
}


void char_attrs::parse_inline( errblock& err,
			       string opts )
{
	//
	// CAPS(ON,OFF,IN,OUT)
	// JUST(LEFT,RIGHT,ASIS)
	// NUMERIC(ON,OFF)
	// PAD(char,NULLS,USER)
	// PADC(char,NULLS,USER)
	// SKIP(ON,OFF)
	// NOJUMP(ON,OFF)
	//

	size_t p1 ;
	size_t p2 ;

	char quote ;

	string t   ;

	bool ex_pad = false ;

	err.setRC( 0 ) ;

	if ( opts == "NONE" || opts == "" ) { return ; }

	opts = "," + opts ;
	p1   = opts.find( ",CAPS(" ) ;
	if ( p1 != string::npos )
	{
		p2 = opts.find( ')', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return ;
		}
		t = opts.substr( p1+6, p2-p1-6 ) ;
		trim( t ) ;
		if ( t.size() == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
			return ;
		}
		if ( t == "ON" )
		{
			caps1 = CAPS_ON ;
		}
		else if ( t == "IN" )
		{
			caps1 = CAPS_IN ;
		}
		else if ( t == "OFF" )
		{
			caps1 = CAPS_OFF ;
		}
		else if ( t == "OUT" )
		{
			caps1 = CAPS_OUT ;
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE035O" ) ;
			return ;
		}
		caps2 = caps1 ;
		opts  = opts.erase( p1, p2-p1+1 ) ;
	}

	p1 = opts.find( ",JUST(" ) ;
	if ( p1 != string::npos )
	{
		p2 = opts.find( ')', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return ;
		}
		t = opts.substr( p1+6, p2-p1-6 ) ;
		trim( t ) ;
		if ( t.size() == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
			return ;
		}
		if      ( t == "LEFT"  ) { just1 = 'L' ; }
		else if ( t == "RIGHT" ) { just1 = 'R' ; }
		else if ( t == "ASIS"  ) { just1 = 'A' ; }
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE035M", t ) ;
			return ;
		}
		just2 = just1 ;
		opts  = opts.erase( p1, p2-p1+1 ) ;
	}

	p1 = opts.find( ",NUMERIC(" ) ;
	if ( p1 != string::npos )
	{
		p2 = opts.find( ')', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return ;
		}
		t = opts.substr( p1+9, p2-p1-9 ) ;
		trim( t ) ;
		if ( t.size() == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE035G" ) ;
			return ;
		}
		if      ( t == "ON"  ) { numeric = true  ; }
		else if ( t == "OFF" ) { numeric = false ; }
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE035C", "NUMERIC" ) ;
			return ;
		}
		opts = opts.erase( p1, p2-p1+1 ) ;
	}

	p1 = opts.find( ",PAD(" ) ;
	if ( p1 != string::npos )
	{
		p2 = opts.find( ')', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return ;
		}
		t = opts.substr( p1+5, p2-p1-5 ) ;
		trim( t ) ;
		if ( t == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
			return ;
		}
		else if ( t == "USER" )
		{
			paduser = true ;
		}
		else if ( t == "NULLS" )
		{
			padchar = 0x00 ;
		}
		else
		{
			quote = t.front() ;
			if ( quote == '\'' || quote == '"' )
			{
				if ( t.size() > 1 && t.back() != quote )
				{
					err.seterrid( TRACE_INFO(), "PSYE033F" ) ;
					return ;
				}
				t.pop_back()    ;
				t.erase( 0, 1 ) ;
			}
			if ( t.size() != 1 )
			{
				err.seterrid( TRACE_INFO(), "PSYE035N", t ) ;
				return ;
			}
			padchar = t.front() ;
		}
		opts   = opts.erase( p1, p2-p1+1 ) ;
		padc   = false ;
		ex_pad = true ;
	}

	p1 = opts.find( ",PADC(" ) ;
	if ( p1 != string::npos )
	{
		p2 = opts.find( ')', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return ;
		}
		if ( ex_pad )
		{
			err.seterrid( TRACE_INFO(), "PSYE035K", "PAD", "PADC" ) ;
			return ;
		}
		t = opts.substr( p1+6, p2-p1-6 ) ;
		trim( t ) ;
		if ( t == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
			return ;
		}
		else if ( t == "USER" )
		{
			paduser = true ;
		}
		else if ( t == "NULLS" )
		{
			padchar = 0x00 ;
		}
		else
		{
			quote = t.front() ;
			if ( quote == '\'' || quote == '"' )
			{
				if ( t.size() > 1 && t.back() != quote )
				{
					err.seterrid( TRACE_INFO(), "PSYE033F" ) ;
					return ;
				}
				t.pop_back()    ;
				t.erase( 0, 1 ) ;
			}
			if ( t.size() != 1 )
			{
				err.seterrid( TRACE_INFO(), "PSYE035N", t ) ;
				return ;
			}
			padchar = t.front() ;
		}
		opts = opts.erase( p1, p2-p1+1 ) ;
		padc = true ;
	}

	p1 = opts.find( ",SKIP(" ) ;
	if ( p1 != string::npos )
	{
		p2 = opts.find( ')', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return ;
		}
		t = opts.substr( p1+6, p2-p1-6 ) ;
		trim( t ) ;
		if ( t.size() == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
			return ;
		}
		if      ( t == "ON" )  { skip = true  ; }
		else if ( t == "OFF" ) { skip = false ; }
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE035C", "SKIP" ) ;
			return ;
		}
		opts = opts.erase( p1, p2-p1+1 ) ;
	}

	p1 = opts.find( ",NOJUMP(" ) ;
	if ( p1 != string::npos )
	{
		p2 = opts.find( ')', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
			return ;
		}
		t = opts.substr( p1+8, p2-p1-8 ) ;
		trim( t ) ;
		if ( t.size() == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
			return ;
		}
		if      ( t == "ON" )  { nojump = true  ; }
		else if ( t == "OFF" ) { nojump = false ; }
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE035C", "NOJUMP" ) ;
			return ;
		}
		opts = opts.erase( p1, p2-p1+1 ) ;
	}

	if ( trim( opts ) != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", opts ) ;
		return ;
	}

	if ( numeric && !is_input() )
	{
		err.seterrid( TRACE_INFO(), "PSYE035I", "NUMERIC(ON)" ) ;
		return ;
	}

	if ( nojump && is_text() )
	{
		err.seterrid( TRACE_INFO(), "PSYE036G", "NOJUMP(ON)" ) ;
		return ;
	}
}


uint char_attrs::get_colour() const
{
	//
	// For overrides:
	//    CUA -> basic type.  Use CUA colour/intens/hilite unless specified on the basic type.
	//

	const uint cl_mask = RED | GREEN | YELLOW | BLUE | MAGENTA | TURQ | WHITE ;

	const uint in_mask = A_NORMAL | A_BOLD | A_INVIS ;

	const uint hi_mask = A_BLINK | A_REVERSE | A_UNDERLINE ;

	uint l_colour ;

	if ( ovrd_attrs )
	{
		if ( cua && !ovrd_attrs->cua )
		{
			l_colour = lspfc::cuaAttr[ type ] ;
			if ( !ovrd_attrs->ncolour )
			{
				l_colour &= ~cl_mask ;
				l_colour |= ovrd_attrs->colour ;
			}
			if ( !ovrd_attrs->nintens )
			{
				l_colour &= ~in_mask ;
				l_colour |= ovrd_attrs->intens ;
			}
			if ( !ovrd_attrs->nhilite )
			{
				l_colour &= ~hi_mask ;
				l_colour |= ovrd_attrs->hilite ;
			}
			return l_colour ;
		}
		return ovrd_attrs->get_colour() ;
	}

	if ( cuadyn == NONE )
	{
		if ( pas )
		{
			return ( ncolour ) ? lspfc::cuaAttr[ PS ] : ( colour | intens | hilite ) ;
		}
		else if ( cua )
		{
			if ( type == SC )
			{
				return lspfc::cuaAttr[ ( avail ) ? SAC : SUC ] ;
			}
			return lspfc::cuaAttr[ type ] ;
		}
		return ( colour | intens | hilite ) ;
	}
	else
	{
	       return lspfc::cuaAttr[ cuadyn ] ;
	}
}


bool char_attrs::is_once() const
{
	return once ;
}


bool char_attrs::is_cua_input() const
{
	return ( type == CEF ||
		 type == EE  ||
		 type == LEF ||
		 type == NEF ) ;
}


bool char_attrs::is_input_attr() const
{
	return input ;
}


bool char_attrs::is_output_attr() const
{
	return output ;
}


bool char_attrs::is_text_attr() const
{
	return text ;
}


char char_attrs::get_padchar() const
{
	if ( ovrd_attrs ) { return ovrd_attrs->get_padchar() ; }

	return padchar ;
}


char char_attrs::get_just( bool is_tbfield ) const
{
	if ( ovrd_attrs ) { return ovrd_attrs->get_just( is_tbfield ) ; }

	return ( is_tbfield ) ? just2 : just1 ;
}


bool char_attrs::is_input() const
{
	if ( ovrd_attrs ) { return ovrd_attrs->is_input() ; }

	return input ;
}


bool char_attrs::is_input_pas() const
{
	if ( ovrd_attrs ) { return ovrd_attrs->is_input_pas() ; }

	return input || pas ;
}


bool char_attrs::is_text() const
{
	if ( ovrd_attrs ) { return ovrd_attrs->is_text() ; }

	return text ;
}


bool char_attrs::is_caps( bool is_tbfield ) const
{
	if ( ovrd_attrs ) { return ovrd_attrs->is_caps( is_tbfield ) ; }

	return ( is_tbfield ) ? ( caps2 == CAPS_ON ) : ( caps1 == CAPS_ON ) ;
}


bool char_attrs::is_caps_in( bool is_tbfield ) const
{
	if ( ovrd_attrs ) { return ovrd_attrs->is_caps_in( is_tbfield ) ; }

	return ( is_tbfield ) ? ( caps2 == CAPS_IN || caps2 == CAPS_ON ) : ( caps1 == CAPS_IN || caps1 == CAPS_ON ) ;
}


bool char_attrs::is_caps_out( bool is_tbfield ) const
{
	if ( ovrd_attrs ) { return ovrd_attrs->is_caps_out( is_tbfield ) ; }

	return ( is_tbfield ) ? ( caps2 == CAPS_OUT || caps2 == CAPS_ON ) : ( caps1 == CAPS_OUT || caps1 == CAPS_ON ) ;
}


bool char_attrs::is_skip() const
{
	if ( ovrd_attrs ) { return ovrd_attrs->is_skip() ; }

	return skip ;
}


bool char_attrs::is_nojump() const
{
	if ( ovrd_attrs ) { return ovrd_attrs->is_nojump() ; }

	return nojump ;
}


bool char_attrs::is_paduser() const
{
	if ( ovrd_attrs ) { return ovrd_attrs->is_paduser() ; }

	return paduser ;
}


bool char_attrs::is_numeric() const
{
	if ( ovrd_attrs ) { return ovrd_attrs->is_numeric() ; }

	return numeric ;
}


bool char_attrs::is_pas() const
{
	if ( ovrd_attrs ) { return ovrd_attrs->is_pas() ; }

	return pas ;
}


bool char_attrs::is_passwd() const
{
	if ( ovrd_attrs ) { return ovrd_attrs->is_passwd() ; }

	return passwd ;
}


bool char_attrs::is_intens_non() const
{
	if ( ovrd_attrs ) { return ovrd_attrs->is_intens_non() ; }

	return ( intens == A_INVIS ) ;
}


bool char_attrs::is_padc() const
{
	if ( ovrd_attrs ) { return ovrd_attrs->is_padc() ; }

	return padc ;
}


attType char_attrs::get_type() const
{
	if ( ovrd_attrs ) { return ovrd_attrs->type ; }

	return type ;
}


void char_attrs::set_cuatype( attType t )
{
	type = t ;
	cua  = true ;

	switch ( type )
	{
	case CH:
	case CT:
	case DT:
	case ET:
	case FP:
	case NT:
	case PIN:
	case PS:
	case PT:
	case RP:
	case SAC:
	case SI:
	case SUC:
	case WASL:
	case WT:
		text   = true  ;
		input  = false ;
		output = false ;
		break ;

	case CEF:
	case EE:
	case LEF:
	case NEF:
		text   = false ;
		input  = true  ;
		output = false ;
		break ;

	default:
		text   = false ;
		input  = false ;
		output = true  ;
	}

	set_defaults() ;
}


void char_attrs::set_defaults()
{
	switch ( type )
	{
	case LEF:
		padc = true ;
	case LI:
	case LID:
		just1 = 'A' ;
		break ;

	case CEF:
	case NEF:
		padc = true ;
		break ;

	case EE:
		padc    = true ;
		padchar = '_' ;
		break ;

	case INPUT:
		caps1   = CAPS_ON ;
		padc    = true ;
		paduser = true ;
		break ;

	case OUTPUT:
		caps1 = CAPS_ON ;
		break ;

	}
}


/* ************************************ ***************************** ********************************* */
/* ************************************      Text Attribute class     ********************************* */
/* ************************************ ***************************** ********************************* */


uint text_attrs::get_colour()
{
	return lspfc::cuaAttr[ type ] ;
}


attType text_attrs::get_type()
{
	return type ;
}

/* ************************************ ***************************** ********************************* */
/* ************************************     File Tailoring classes    ********************************* */
/* ************************************ ***************************** ********************************* */

void ftc_rexx::parse( errblock& err,
		      const string& s )
{
	//
	// Format of the )REXX file tailoring statement:
	//
	// )REXX VAR1,VAR2,VAR3
	// )REXX REXX=ftrexx
	// )REXX VAR1,VAR2,VAR3 REXX=ftrexx
	// )REXX &A REXX=&B
	//
	// Variable names may be separated by a blank or a comma.
	// REXX procedure and list of passed variables can be contained in a variable.
	//

	int i ;
	int j ;

	string t ;

	size_t p ;

	err.setRC( 0 ) ;

	p = upper( s ).find( "REXX=" ) ;
	if ( p != string::npos )
	{
		rexx = strip( s.substr( p+5 ) ) ;
		if ( words( rexx ) != 1 )
		{
			err.seterrid( TRACE_INFO(), "PSYE034S" ) ;
			return ;
		}
		t = upper( subword( s.substr( 0, p ), 2 ) ) ;
	}
	else
	{
		t = upper( subword( s, 2 ) ) ;
	}

	replace( t.begin(), t.end(), ',', ' ' ) ;

	for ( i = 1, j = words( t ) ; i <= j ; ++i )
	{
		vars.insert( word( t, i ) ) ;
	}
}


void ftc_sel::parse( errblock& err,
		     string s,
		     const char char_var )
{
	//
	// )SEL A op B && C op D | E op F
	//

	int ws = words( s ) ;

	string w1 ;

	err.setRC( 0 ) ;

	map<string, RL_COND> sel_conds =
	{ { "=",  RL_EQ },
	  { "EQ", RL_EQ },
	  { "!=", RL_NE },
	  { "NE", RL_NE },
	  { ">",  RL_GT },
	  { "GT", RL_GT },
	  { "<",  RL_LT },
	  { "LT", RL_LT },
	  { ">=", RL_GE },
	  { "GE", RL_GE },
	  { "!<", RL_GE },
	  { "NL", RL_GE },
	  { "<=", RL_LE },
	  { "LE", RL_LE },
	  { "!>", RL_LE },
	  { "NG", RL_LE } } ;

	if ( ws < 4 )
	{
		err.seterrid( TRACE_INFO(), "PSYE026D", ")SEL", src_info() ) ;
		return ;
	}

	string w3 = upper( word( s, 3 ) ) ;

	auto it = sel_conds.find( w3 ) ;
	if ( it == sel_conds.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE026P", w3, src_info() ) ;
		return ;
	}

	cond = it->second ;

	lval = upper( word( s, 2 ) ) ;
	rval = upper( word( s, 4 ) ) ;

	if ( lval.front() == char_var )
	{
		lvar = true ;
		lval.erase( 0, 1 ) ;
		if ( !isvalidName( lval ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE026U", lval, src_info() ) ;
			return ;
		}
	}

	if ( rval.front() == char_var )
	{
		rvar = true ;
		rval.erase( 0, 1 ) ;
		if ( !isvalidName( rval ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE026U", rval, src_info() ) ;
			return ;
		}
	}

	idelword( s, 1, 4 ) ;
	if ( words( s ) == 0 )
	{
		return ;
	}

	w1 = word( s, 1 ) ;
	if ( w1 != "&&" && w1 != "|" )
	{
		err.seterrid( TRACE_INFO(), "PSYE026O", src_info() ) ;
		return ;
	}

	sel_AND = ( w1 == "&&" ) ;

	ft_sel_next = new ftc_sel( err, s, name, char_var, ln ) ;
}


void ftc_def::parse( errblock& err,
		     const string& s )
{
	string t = word( s, 2 ) ;

	if ( words( s ) > 2 )
	{
		err.seterrid( TRACE_INFO(), "PSYE026T", subword( s, 3 ), src_info() ) ;
		return ;
	}

	if ( t.size() != 7 )
	{
		err.seterrid( TRACE_INFO(), "PSYE026Q", t, src_info() ) ;
		return ;
	}

	defs = t ;
}


void ftc_set::parse( errblock& err,
		     const string& s )
{
	//
	// )SET A = expr
	//

	int ws = words( s ) ;

	string w ;

	err.setRC( 0 ) ;

	if ( ws < 4 )
	{
		err.seterrid( TRACE_INFO(), "PSYE026D", ")SET", src_info() ) ;
		return ;
	}

	w = word( s, 3 ) ;
	if ( w != "=" )
	{
		err.seterrid( TRACE_INFO(), "PSYE026E", w, ")SET", src_info() ) ;
		return ;
	}

	var  = upper( word( s, 2 ) ) ;
	expr = subword( s, 4 ) ;

	if ( !isvalidName( var ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE026U", var, src_info() ) ;
		return ;
	}
}


void ftc_dot::parse( errblock& err,
		     string s )
{
	//
	// )DOT table OPT
	// )DOT table SCAN OPT
	// )DOT table SCAN(args) OPT
	//
	// Table name may be a variable.
	//

	int ws ;

	string t1 ;

	err.setRC( 0 ) ;

	scan = parseString2( s, "SCAN" ) ;

	sarg = parseString1( err, s, "SCAN()" ) ;
	if ( err.error() ) { return ; }
	if ( err.RSN0() )
	{
		if ( sarg == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE026W", "SCAN", src_info() ) ;
			return ;
		}
		if ( scan )
		{
			err.seterrid( TRACE_INFO(), "PSYE026X", "SCAN", src_info() ) ;
			return ;
		}
		scan = true ;
		sarg = "(" + sarg + ")" ;
	}

	ws = words( s ) ;
	if ( ws > 3 )
	{
		err.seterrid( TRACE_INFO(), "PSYE026T", subword( s, 4 ), src_info() ) ;
		return ;
	}

	if ( ws < 2 )
	{
		err.seterrid( TRACE_INFO(), "PSYE026D", ")DOT", src_info() ) ;
		return ;
	}

	table = word( s, 2 ) ;
	t1    = word( s, 3 ) ;

	if ( table == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE026B", src_info() ) ;
		return ;
	}

	if ( t1 != "" && t1 != "OPT" )
	{
		err.seterrid( TRACE_INFO(), "PSYE026E", t1, ")DOT", src_info() ) ;
		return ;
	}

	opt = ( t1 == "OPT" ) ;
}


void ftc_do::parse( errblock& err,
		    const string& s )
{
	//
	// )DO n
	// )DO FOREVER
	// )DO VAR = n to m by x for y WHILE | UNTIL expr
	//

	token t ;

	bool ex_while = false ;
	bool ex_until = false ;
	bool ex_loop  = false ;
	bool ex_to    = false ;
	bool ex_by    = false ;
	bool ex_for   = false ;

	parser t_do ;
	t_do.optionUpper() ;

	t_do.parse( err, s ) ;
	if ( err.error() )
	{
		err.setftsrc( src_info() ) ;
		return ;
	}

	err.setRC( 0 ) ;

	if ( t_do.getEntries() == 3 && t_do.getToken( 2 ).value1 == "FOREVER" )
	{
		l_type1 = false ;
		l_type3 = true ;
	}
	else if ( t_do.getEntries() > 3 )
	{
		l_type1 = false ;
		l_type2 = true ;
	}

	map<string, RL_COND> conds =
	{ { "=",  RL_EQ },
	  { "EQ", RL_EQ },
	  { "!=", RL_NE },
	  { "NE", RL_NE },
	  { ">",  RL_GT },
	  { "GT", RL_GT },
	  { "<",  RL_LT },
	  { "LT", RL_LT },
	  { ">=", RL_GE },
	  { "GE", RL_GE },
	  { "!<", RL_GE },
	  { "NL", RL_GE },
	  { "<=", RL_LE },
	  { "LE", RL_LE },
	  { "!>", RL_LE },
	  { "NG", RL_LE } } ;

	t_do.getFirstToken() ;
	t = t_do.getNextToken() ;
	t = t_do.getNextToken() ;

	if ( l_type1 )
	{
		if ( t.type != TT_EOT )
		{
			parse_t( err, s, t, v_loop, l_loop ) ;
			if ( err.error() ) { return ; }
			t = t_do.getNextToken() ;
			if ( t.type != TT_EOT )
			{
				err.seterrid( TRACE_INFO(), "PSYE026T", t.value2, src_info() ) ;
				return ;
			}
		}
	}
	else if ( l_type2 )
	{
		while ( t.type != TT_EOT )
		{
			if ( ( e_while = ( t.value1 == "WHILE" ) ) || ( ( e_until = ( t.value1 == "UNTIL" ) ) ) )
			{
				if ( ( ex_while || ex_until ) && ( e_while || e_until ) )
				{
					err.seterrid( TRACE_INFO(), "PSYE026X", "WHILE/UNTIL", src_info() ) ;
					return ;
				}
				t = t_do.getNextToken() ;
				parse_t( err, s, t, v_lhs, l_lhs ) ;
				if ( err.error() ) { return ; }
				t = t_do.getNextToken() ;
				auto it = conds.find( t.value1 ) ;
				if ( it == conds.end() )
				{
					err.seterrid( TRACE_INFO(), "PSYE027C", t.value1, ")DO", src_info() ) ;
					return ;
				}
				cond = it->second ;
				t = t_do.getNextToken() ;
				parse_t( err, s, t, v_rhs, l_rhs ) ;
				if ( err.error() ) { return ; }
				ex_while = e_while ;
				ex_until = e_until ;
			}
			else if ( t.value1 == "TO" )
			{
				if ( ex_to && e_to )
				{
					err.seterrid( TRACE_INFO(), "PSYE026X", "TO", src_info() ) ;
					return ;
				}
				t = t_do.getNextToken() ;
				parse_t( err, s, t, v_to, l_to ) ;
				if ( err.error() ) { return ; }
				e_to  = true ;
				ex_to = true ;
			}
			else if ( t.value1 == "BY" )
			{
				if ( ex_by && e_by )
				{
					err.seterrid( TRACE_INFO(), "PSYE026X", "BY", src_info() ) ;
					return ;
				}
				t = t_do.getNextToken() ;
				parse_t( err, s, t, v_by, l_by ) ;
				if ( err.error() ) { return ; }
				e_by  = true ;
				ex_by = true ;
			}
			else if ( t.value1 == "FOR" )
			{
				if ( ex_for && e_for )
				{
					err.seterrid( TRACE_INFO(), "PSYE026X", "FOR", src_info() ) ;
					return ;
				}
				t = t_do.getNextToken() ;
				parse_t( err, s, t, v_for, l_for ) ;
				if ( err.error() ) { return ; }
				e_for  = true ;
				ex_for = true ;
			}
			else if ( t.subtype == TS_NAME )
			{
				if ( ex_loop && e_loop )
				{
					err.seterrid( TRACE_INFO(), "PSYE026X", "Control variable", src_info() ) ;
					return ;
				}
				var = t.value1 ;
				t   = t_do.getNextToken() ;
				if ( t.value1 != "=" )
				{
					err.seterrid( TRACE_INFO(), "PSYE027B", "=", "control variable", src_info() ) ;
					return ;
				}
				t = t_do.getNextToken() ;
				parse_t( err, s, t, v_loop, l_loop ) ;
				if ( err.error() ) { return ; }
				e_loop  = true ;
				ex_loop = true ;
			}
			else
			{
				err.seterrid( TRACE_INFO(), "PSYE031D", t.value1 ) ;
				return ;
			}
			t = t_do.getNextToken() ;
		}
		if ( !e_loop && ( e_to || e_by ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE026D", "Control variable", src_info() ) ;
			return ;
		}
		if ( e_loop && !e_to && !e_while && !e_until )
		{
			err.seterrid( TRACE_INFO(), "PSYE026D", "TO, WHILE or UNTIL", src_info() ) ;
			return ;
		}
	}
}


void ftc_do::parse_t( errblock& err,
		      const string& s,
		      token& t,
		      string& v,
		      int& i )
{
	//
	// Check token for valid values and populate variables.
	//

	if ( t.type == TT_EOT )
	{
		err.seterrid( TRACE_INFO(), "PSYE027E", ")DO", src_info() ) ;
	}
	else if ( t.value1.size() > 1 && t.value1.size() <= 9 && isvalidName( t.value1.substr( 1 ) ) )
	{
		v = t.value1 ;
	}
	else if ( !datatype( t.value1, 'W' ) || t.value1.size() > 10 )
	{
		err.seterrid( TRACE_INFO(), "PSYE026E", t.value1, ")DO", src_info() ) ;
	}
	else
	{
		i = ds2d( t.value1 ) ;
	}
}


void ftc_leave::parse( errblock& err,
		       const string& s )
{
	//
	// )LEAVE
	// )LEAVE DOT
	//

	int ws = words( s ) ;

	if ( ws > 2 || ( ws == 2 && word( s, 2 ) != "DOT" ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE026T", subword( s, 3 ), src_info() ) ;
		return ;
	}

	dot = ( word( s, 2 ) == "DOT" ) ;
}


void ftc_tb::parse( errblock& err,
		    const string& s )
{
	//
	// )TB
	// )TB  n  n n  n
	// )TB  nA n nA n
	// )TBA n  n n  n
	//

	int ws = words( s ) ;

	string w1 = word( s, 1 ) ;

	bool tabalt ;

	size_t tab ;

	string t ;

	ft_tabs.clear() ;
	ft_tbalt.clear() ;

	ft_tba = ( w1 == ")TBA" ) ;

	for ( int i = 2 ; i <= ws ; ++i )
	{
		t = word( s, i ) ;
		if ( t.back() == 'A' )
		{
			if ( t.size() == 1 )
			{
				err.seterrid( TRACE_INFO(), "PSYE026R", w1, src_info() ) ;
				return ;
			}
			t.pop_back() ;
			tabalt = true ;
		}
		else
		{
			tabalt = ft_tba ;
		}
		if ( ( !datatype( t, 'W' ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE026R", t, src_info() ) ;
			return ;
		}
		tab = ds2d( t ) ;
		if ( ( tab > 255 ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE026S", t, src_info() ) ;
			return ;
		}
		ft_tabs.push_back( tab ) ;
		ft_tbalt.push_back( tabalt ) ;
	}

	if ( ft_tabs.empty() )
	{
		ft_tabs.push_back( 255 ) ;
		ft_tbalt.push_back( ft_tba ) ;
	}
}


size_t ftc_tb::get_tabpos( size_t p )
{
	int i ;

	size_t t = 0 ;

	vector<size_t>::iterator it ;

	for ( i = 0, it = ft_tabs.begin() ; it != ft_tabs.end() ; ++it, ++i )
	{
		if ( *it > ( ( ft_tbalt.at( i ) ) ? p : ( p + 1 ) ) )
		{
			t = *it ;
			break ;
		}
	}

	return t ;
}


void ftc_main::parse( errblock& err,
		      const string& s,
		      const char char_var )
{
	string w1 = word( s, 1 ) ;
	string w2 ;

	err.setRC( 0 ) ;

	auto it = statement_types.find( w1.substr( 1 ) ) ;
	if ( it == statement_types.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE026A", w1, src_info() ) ;
		return ;
	}

	switch ( it->second )
	{
	case FT_BLANK:
		if ( words( s ) > 2 )
		{
			err.seterrid( TRACE_INFO(), "PSYE026T", subword( s, 3 ), src_info() ) ;
			return ;
		}
		w2 = word( s, 2 ) ;
		if ( ( w2 != "" && !datatype( w2, 'W' ) ) || w2.size() > 4 )
		{
			err.seterrid( TRACE_INFO(), "PSYE026E", w2, ")BLANK", src_info() ) ;
			return ;
		}
		ft_blank = true ;
		ft_num   = ( w2 == "" ) ? 1 : ds2d( w2 ) ;
		break ;

	case FT_DEFAULT:
		ft_def = new ftc_def( err, s, name, ln ) ;
		break ;

	case FT_DO:
		ft_do = new ftc_do( err, s, name, ln ) ;
		break ;

	case FT_ENDDO:
		ft_enddo = true ;
		break ;

	case FT_DOT:
		ft_dot = new ftc_dot( err, s, name, ln ) ;
		break ;

	case FT_ENDDOT:
		ft_enddot = true ;
		break ;

	case FT_LEAVE:
		ft_leave = new ftc_leave( err, s, name, ln ) ;
		break ;

	case FT_ITERATE:
		ft_iterate = true ;
		break ;

	case FT_NOP:
		ft_nop = true ;
		break ;

	case FT_SEL:
		ft_sel = new ftc_sel( err, s, name, char_var, ln ) ;
		break ;

	case FT_ENDSEL:
		ft_endsel = true ;
		break ;

	case FT_REXX:
		ft_rexx = new ftc_rexx( err, s, name, ln ) ;
		break ;

	case FT_SET:
		ft_set = new ftc_set( err, s, name, ln ) ;
		break ;

	case FT_TB:
	case FT_TBA:
		ft_tb = new ftc_tb( err, s, name, ln ) ;
		break ;

	case FT_ELSE:
	case FT_IF:
	case FT_SETF:
		err.seterrid( TRACE_INFO(), "PSYE026I", src_info() ) ;
		break ;
	}
}


/* ************************************ ***************************** ********************************* */
/* ************************************         Logger class          ********************************* */
/* ************************************ ***************************** ********************************* */


logger::logger()
{
	logfl   = "" ;
	currfl  = nullptr ;
	logOpen = false ;

	boost::filesystem::path temp = \
	boost::filesystem::temp_directory_path() / boost::filesystem::unique_path( "lspf-%%%%-%%%%" ) ;
	tmpfl = temp.native() ;
}


logger::~logger()
{
	if ( logOpen ) { close() ; }
}


bool logger::open( const string& dest,
		   bool append )
{
	//
	// Open log file for output.  Write to temporary file if destination not set.
	//

	if ( logOpen ) { return true ; }

	logfl  = dest ;
	currfl = ( dest == "" ) ? &tmpfl : &logfl ;

	if ( append )
	{
		of.open( *currfl, std::fstream::app ) ;
	}
	else
	{
		of.open( *currfl ) ;
	}

	logOpen = of.is_open() ;

	return logOpen ;
}


void logger::close()
{
	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	of.close() ;
	logOpen = false ;
}


bool logger::set( const string& dest )
{
	//
	// Move log records to a new destination and continue recording.
	// Overwrite the new file and remove the old file.
	//

	string* t ;

	bool res = false ;

	boost::system::error_code ec ;

	boost::lock_guard<boost::recursive_mutex> lock( mtx ) ;

	if ( dest == *currfl ) { return true ; }

	of.close() ;
	logOpen = false ;

	if ( !exists( *currfl ) )
	{
		res = open( dest, true ) ;
	}
	else if ( !exists( dest ) || is_regular_file( dest ) )
	{
		copy_file( *currfl, dest, copy_options::overwrite_existing, ec ) ;
		if ( ec.value() == boost::system::errc::success )
		{
			t   = currfl ;
			res = open( dest, true ) ;
			remove( *t ) ;
		}
	}

	return res ;
}


/* ************************************ ***************************** ********************************* */
/* ************************************         locator class         ********************************* */
/* ************************************ ***************************** ********************************* */


void locator::locate()
{
	//
	// Locate a file in a concatination of paths.
	//
	// First, find the file name asis.  If not found, then, if requested and not already,
	// repeat the find with the file name in lower case.
	//

	int i ;
	int j ;

	string tempf = filename ;
	string tempe ;

	while ( true )
	{
		for ( i = getpaths( paths ), j = 1 ; j <= i ; ++j )
		{
			tempe = getpath( paths, j ) + tempf ;
			try
			{
				if ( boost::filesystem::exists( tempe ) )
				{
					if ( !boost::filesystem::is_regular_file( tempe ) )
					{
						mid  = "PSYE041L" ;
						data = tempe ;
						return ;
					}
					ent = tempe ;
					return ;
				}
			}
			catch ( boost::filesystem::filesystem_error &e )
			{
				mid  = "PSYS012C" ;
				data = e.what() ;
				return ;
			}
		}
		if ( type != 'B' || ( tempf == lower( tempf ) ) ) { break ; }
		ilower( tempf ) ;
	}
}



/* ************************************ ***************************** ********************************* */
/* ************************************     Dynamic Loader class      ********************************* */
/* ************************************ ***************************** ********************************* */


dynloader::dynloader( const string& name,
		      int f )

{
	filename = name ;
	flags    = f ;
	handle   = nullptr ;
}


dynloader::~dynloader()
{
	close() ;
}


void dynloader::open()
{
	handle = dlopen( filename.c_str(), flags ) ;

	if ( !handle )
	{
		errstring = string( dlerror() ) ;
	}
}


void dynloader::close()
{
	if ( handle )
	{
		dlclose( handle ) ;
		handle = nullptr ;
	}
}


void* dynloader::lookup( const char* symbol )
{
	void* func = dlsym( handle, symbol ) ;
	const char* dlsym_err = dlerror() ;

	if ( dlsym_err )
	{
		errstring = string( dlsym_err ) ;
	}

	return func ;
}



/* ************************************ ***************************** ********************************* */
/* ************************************         ALTLIB class          ********************************* */
/* ************************************ ***************************** ********************************* */


void tso_altlib::parse( string& err,
			string s )
{
	//
	// Format of the ALTLIB command:
	//   ALTLIB ACT USER/SYSTEM(EXEC) QUIET
	//   ALTLIB ACT APPLICATION(EXEC) DATASET()/DDNAME() COND/UNCOND QUIET
	//   ALTLIB DEACT USER/SYSTEM/APPLICATION(EXEC)/ALL
	//   ALTLIB DISPLAY
	//   ALTLIB RESET
	//
	//   '*' can be used in place of EXEC.
	//

	errblock errs ;

	int i ;
	int j ;
	int ws ;

	bool uncond = false ;

	string p ;
	string t ;
	string w ;
	string w2 ;
	string type ;
	string keyw1 ;

	w2 = upper( word( s, 2 ) ) ;

	if ( w2 == "ACT" || w2 == "ACTIVATE" )
	{
		act = true ;
	}
	else if ( w2 == "DEACT" || w2 == "DEACTIVATE" )
	{
		act = false ;
	}
	else if ( w2 == "DIS" || w2 == "DISP" || w2 == "DISPLAY" )
	{
		disp = true ;
	}
	else if ( w2 == "RES" || w2 == "RESET" )
	{
		reset = true ;
	}
	else
	{
		err = "Invalid action entered.  Enter ACTIVATE, DEACTIVATE, RESET or DISPLAY" ;
		return ;
	}

	quiet = parseString2( s, "QUIET" ) ;
	ws    = words( s ) ;

	if ( disp || reset )
	{
		if ( ws > 2 )
		{
			err = "Invalid number of parameters entered." ;
		}
		return ;
	}

	s = subword( s, 3 ) ;

	all    = parseString2( s, "ALL" ) ;
	reset  = parseString2( s, "RESET" ) ;
	cond   = parseString2( s, "COND" ) ;
	uncond = parseString2( s, "UNCOND" ) ;

	if ( !act && all )
	{
		if ( ws > 3 )
		{
			err = "Extaneous data '" + s + "' entered on an ALTLIB DEACT ALL command." ;
		}
		return ;
	}

	keyw1 = getKeywords1( errs, s ) ;
	if ( errs.error() )
	{
		err = "Error parsing ALLOCATE statatement" ;
		return ;
	}

	for ( i = 1, ws = words( keyw1 ) ; i <= ws ; ++i )
	{
		w = word( keyw1, i ) ;
		if ( abbrev( "USER", w, 2 ) )
		{
			w = w + "()" ;
			type = parseString1( errs, s, w ) ;
			user = true ;
		}
		else if ( abbrev( "SYSTEM", w, 2 ) )
		{
			w = w + "()" ;
			type = parseString1( errs, s, w ) ;
			syst = true ;
		}
		else if ( abbrev( "APPLICATION", w, 2 ) )
		{
			w = w + "()" ;
			type = parseString1( errs, s, w ) ;
			appl = true ;
		}
		else if ( abbrev( "DATASET", w, 2 ) || abbrev( "DSNAME", w, 2 ) )
		{
			w    = w + "()" ;
			path = parseString1( errs, s, w ) ;
			if ( path == "" )
			{
				err = "Missing path name on DSNAME keyword." ;
				return ;
			}
			if ( ddn != "" )
			{
				err = "Cannot specify both DDNAME and DSNAME on an ALTLIB ACTIVATE request" ;
				return ;
			}
		}
		else if ( abbrev( "DDNAME", w, 2 ) || abbrev( "FILE", w, 2 ) || abbrev( "LIBRARY", w, 2 ) )
		{
			w   = w + "()" ;
			ddn = upper( parseString1( errs, s, w ) ) ;
			if ( !isvalidName( ddn ) )
			{
				err = "Invalid DDNAME entered." ;
				return ;
			}
			if ( path != "" )
			{
				err = "Cannot specify both DDNAME and DSNAME on an ALTLIB ACTIVATE request" ;
				return ;
			}
		}
		else
		{
			err = "Extaneous keyword parameter " + w + " entered on an ALTLIB command." ;
			return ;
		}
	}

	if ( s != "" )
	{
		err = "Extaneous data '" + s + "' entered on an ALTLIB command." ;
		return ;
	}

	if ( ( user && ( syst || appl ) ) || ( syst && appl ) || ( !user && !syst && !appl ) )
	{
		err = "Specify only one of USER, SYSTEM and APPLICATION." ;
		return ;
	}

	if ( upper( type ) != "EXEC" && type != "*" )
	{
		err = "Invalid type entered.  Only EXEC and '*' are supported" ;
		return ;
	}

	if ( ( user || syst ) && ( ddn != "" || path != "" ) )
	{
		err = "Cannot specify DDNAME or path with an ALTLIB ACTIVATE USER/SYSTEM request" ;
		return ;
	}

	if ( appl && act && ddn == "" && path == "" )
	{
		err = "DDNAME or path must be specified on an ALTLIB ACTIVATE APPLICATION request" ;
		return ;
	}

	if ( !act && ( ddn != "" || path != "" ) )
	{
		err = "DDNAME or path must not be specified on an ALTLIB DEACTIVATE request" ;
		return ;
	}

	if ( act && all )
	{
		err = "Cannot specify ALL on an ALTLIB ACTIVATE request" ;
		return ;
	}

	if ( ( cond || uncond ) && ( !act || !appl ) )
	{
		err = "COND/UNCOND can only be specified for an ALTLIB ACTIVATE APPLICATION request" ;
		return ;
	}

	if ( cond && uncond )
	{
		err = "Cannot specify both COND and UNCOND on an ALTLIB request" ;
		return ;
	}

	try
	{
		t = "" ;
		j = getpaths( path ) ;
		for ( i = 1 ; i <= j ; ++i )
		{
			p = lspf::getpath( path, i ) ;
			if ( !exists( p ) || !is_directory( p ) )
			{
				err = "Only existing directories can be specified in an ALTLIB concatination." ;
				return ;
			}
			if ( p.size() > 1 && p.back() == '/' ) { p.pop_back() ; }
			t = ( t == "" ) ? p : t + ":" + p ;
		}
		path = t ;
	}
	catch (...)
	{
		err = "Error accessing file or directory" ;
		return ;
	}
}

tso_altlib::~tso_altlib()
{
}

/* ************************************ ***************************** ********************************* */
/* ************************************         ALLOC class           ********************************* */
/* ************************************ ***************************** ********************************* */


void tso_alloc::parse( string& err,
		       string s )
{
	//
	// Format of the ALLOC command:
	//
	// ALLOC FILE(ddn)   DSNAME(path) SHR REU
	// ALLOC FILE(ddn)   NEW DELETE
	// ALLOC FILE(ddn)   NEW DELETE DIR(1)
	// ALLOC FILE(ddn)   DUMMY
	// ALLOC DDNAME(ddn) DATASET(path) SHR REU
	// ALLOC DDNAME(ddn) PATH(path)
	//
	// For a single path, can be existing, new, file or directory.
	// For a concatination, must be all existing directories.
	// Convert existing paths to their canonical form.
	//

	errblock errs ;

	int i ;
	int j ;
	int ws ;

	string p ;
	string t ;
	string w ;

	bool path_spec = false ;

	string keyw1 = getKeywords1( errs, s ) ;
	if ( errs.error() )
	{
		err = "Error parsing ALLOCATE statatement" ;
		return ;
	}

	string keyw2 = getKeywords2( errs, subword( s, 2 ) ) ;
	if ( errs.error() )
	{
		err = "Error parsing ALLOCATE statatement" ;
		return ;
	}

	for ( i = 1, ws = words( keyw1 ) ; i <= ws ; ++i )
	{
		w = word( keyw1, i ) ;
		if ( abbrev( "FILE", w, 1 ) || abbrev( "DDNAME", w, 2 ) )
		{
			if ( ddn != "" )
			{
				err = "DDNAME can only be specified once" ;
				return ;
			}
			w   = w + "()" ;
			ddn = parseString1( errs, s, w ) ;
			if ( errs.error() ) { return ; }
			if ( ddn == "" )
			{
				err = "No DDNAME specified" ;
				return ;
			}
			iupper( ddn ) ;
			if ( !isvalidName( ddn ) )
			{
				err = "DDNAME is invalid" ;
				return ;
			}
		}
		else if ( abbrev( "DATASET", w, 2 ) || abbrev( "DSNAME", w, 2 ) || ( path_spec = ( w == "PATH" ) ) )
		{
			if ( path != "" )
			{
				err = "PATH can only be specified once" ;
			}
			w    = w + "()" ;
			path = parseString1( errs, s, w ) ;
			if ( errs.error() ) { return ; }
			if ( path == "" )
			{
				err = "No PATH specified" ;
				return ;
			}
			if ( path.front() == '\'' )
			{
				if ( path.size() < 3 )
				{
					err = "Invalid path specified, " + path ;
					return ;
				}
				if ( path.back() != '\'' )
				{
					err = "Missing end quote on path name" ;
					return ;
				}
				istrip( path, 'B', '\'' ) ;
			}
		}
		else if ( w == "DIR" )
		{
			dir = true ;
		}
		else
		{
			err = "Invalid keyword specified, " + w ;
			return ;
		}
	}

	if ( ddn == "" )
	{
		err = "DDNAME must be specified on an allocation request" ;
		return ;
	}

	for ( i = 1, ws = words( keyw2 ) ; i <= ws ; ++i )
	{
		w = word( keyw2, i ) ;
		if ( w == "SHR" || abbrev( "SHARE", w, 2 ) )
		{
			if ( old || create )
			{
				err = "Cannot specify SHR with OLD or NEW" ;
				return ;
			}
			shr = true ;
		}
		else if ( w == "OLD" )
		{
			if ( shr || create )
			{
				err = "Cannot specify OLD with SHR or NEW" ;
				return ;
			}
			old = true ;
		}
		else if ( w == "NEW" )
		{
			if ( old || shr )
			{
				err = "Cannot specify NEW with OLD or SHR" ;
				return ;
			}
			create = true ;
		}
		else if ( abbrev( "REUSE", w, 3 ) )
		{
			reuse = true ;
		}
		else if ( abbrev( "DELETE", w, 3 ) )
		{
			del = true ;
		}
		else if ( w == "DUMMY" )
		{
			dummy = true ;
		}
		else
		{
			err = "Invalid keyword specified, " + w ;
			return ;
		}
	}

	if ( create )
	{
		if ( !del )
		{
			err = "DELETE must be specified with the NEW keyword" ;
			return ;
		}
		if ( path != "" )
		{
			err = "Cannot specify NEW with a PATH/DATASET entry" ;
			return ;
		}
		if ( dummy )
		{
			err = "Cannot specify NEW with a DUMMY entry" ;
			return ;
		}
	}
	else if ( dummy )
	{
		if ( path != "" )
		{
			err = "Cannot specify PATH/DATASET with DUMMY" ;
			return ;
		}
		if ( shr || old || del )
		{
			err = "Cannot specify SHR, OLD or DELETE with DUMMY" ;
			return ;
		}
		if ( dir )
		{
			err = "Cannot specify DIR with DUMMY" ;
			return ;
		}
		shr = true ;
	}
	else if ( path == "" )
	{
		err = "Path must be specified if not NEW or DUMMY" ;
		return ;
	}

	if ( path_spec )
	{
		if ( shr || old || reuse )
		{
			err = "Cannot specify SHR, OLD or REUSE with a PATH entry" ;
			return ;
		}
		shr = true ;
	}

	try
	{
		j = getpaths( path ) ;
		if ( j == 1 )
		{
			if ( !exists( path ) )
			{
				if ( path.back() == '/' )
				{
					dir = true ;
				}
			}
			else
			{
				if ( is_directory( path ) )
				{
					dir = true ;
				}
				else if ( !is_regular_file( path ) )
				{
					err = "Only files and directories can be specified in an allocation." ;
					return ;
				}
				path = canonical( boost::filesystem::path( path ) ).string() ;
			}
			if ( path.size() > 1 && path.back() == '/' ) { path.pop_back() ; }
			paths.push_back( path ) ;
		}
		else if ( j > 1 )
		{
			t = "" ;
			for ( i = 1 ; i <= j ; ++i )
			{
				p = lspf::getpath( path, i ) ;
				if ( !exists( p ) || !is_directory( p ) )
				{
					err = "Only existing directories can be specified in an allocation concatination." ;
					return ;
				}
				if ( p.size() > 1 && p.back() == '/' ) { p.pop_back() ; }
				t = ( t == "" ) ? p : t + ":" + p ;
				paths.push_back( canonical( boost::filesystem::path( p ) ).string() ) ;
			}
			path = t ;
			dir  = true ;
		}
	}
	catch (...)
	{
		err = "Error accessing file or directory" ;
		return ;
	}
}


tso_alloc::~tso_alloc()
{
	try
	{
		if ( create && del && exists( path ) )
		{
			if ( dir )
			{
				remove_all( path ) ;
			}
			else
			{
				remove( path ) ;
			}
		}
	}
	catch (...)
	{
	}
}

/* ************************************ ***************************** ********************************* */
/* ************************************          FREE class           ********************************* */
/* ************************************ ***************************** ********************************* */


void tso_free::parse( string& err,
		      string s )
{
	//
	// Format of the FREE command:
	//
	// FREE FILE(ddname1,ddname2...) or DDNAME()
	// FREE DATASET(entry1,entry2...) or DSNAME()
	// FREE PATH(entry1,entry2...)
	// FREE ALL
	//
	// If freeing an existing path, convert to its canonical form.
	//

	int i ;
	int ws ;

	errblock errs ;

	string w ;
	string p ;
	string temp ;

	if ( findword( "ALL", upper( s ) ) )
	{
		if ( words( s ) > 2 )
		{
			err = "Invalid number of keywords" ;
			return ;
		}
		all = true ;
		return ;
	}

	string keyw1 = getKeywords1( errs, s ) ;
	if ( errs.error() ) { return ; }
	if ( words( keyw1 ) != 1 )
	{
		err = "Invalid number of keywords" ;
		return ;
	}

	string keyw2 = getKeywords2( errs, s ) ;
	if ( errs.error() ) { return ; }
	if ( words( keyw2 ) > 1 )
	{
		err = "Unknown keywords entered" ;
		return ;
	}

	if ( abbrev( "FILE", keyw1, 1 ) || abbrev( "DDNAME", keyw1, 2 ) )
	{
		keyw1 = keyw1 + "()" ;
		temp  = upper( parseString1( errs, s, keyw1 ) ) ;
		if ( errs.error() ) { return ; }
		if ( temp == "" )
		{
			err = "No value specified for " + keyw1 ;
			return ;
		}
		replace( temp.begin(), temp.end(), ',', ' ' ) ;
		for ( ws = words( temp ), i = 1 ; i <= ws ; ++i )
		{
			w = word( temp, i ) ;
			if ( !isvalidName( w ) )
			{
				err = "DDNAME is invalid" ;
			}
			ddns.push_back( w ) ;
		}
	}
	else if ( abbrev( "DATASET", keyw1, 2 ) || abbrev( "DSNAME", keyw1, 2 ) || abbrev( "PATH", keyw1, 4 ) )
	{
		keyw1 = keyw1 + "()" ;
		temp  = parseString1( errs, s, keyw1 ) ;
		if ( errs.error() ) { return ; }
		if ( temp == "" )
		{
			err = "No value specified for " + keyw1 ;
			return ;
		}
		replace( temp.begin(), temp.end(), ',', ' ' ) ;
		for ( ws = words( temp ), i = 1 ; i <= ws ; ++i )
		{
			p = word( temp, i ) ;
			try
			{
				if ( exists( p ) )
				{
					p = canonical( boost::filesystem::path( p ) ).string() ;
				}
			}
			catch (...)
			{
				err = "Error accessing file or directory" ;
				return ;
			}
			if ( p.back() == '/' ) { p.pop_back() ; }
			paths.push_back( p ) ;
		}
	}
	else
	{
		err = "Invalid keyword specified, " + keyw1 ;
	}
}


/* ************************************ ***************************** ********************************* */
/* ************************************         EXECIO class          ********************************* */
/* ************************************ ***************************** ********************************* */


void execio::parse( string& err,
		    string s )
{
	//
	// Format of the EXECIO command:
	//
	// EXECIO * DISKR ddname (LIFO|FILO|STEM stem. OPEN FINIS SKIP
	// EXECIO * DISKW ddname (STEM stem. OPEN FINIS
	//

	string w ;

	string head ;
	string tail ;

	size_t p1 ;
	size_t p2 ;
	size_t p3 ;

	iupper( s ) ;

	p1 = s.find( '(' ) ;
	if ( p1 == string::npos )
	{
		err = "Missing bracket" ;
		return ;
	}

	head = s.substr( 0, p1 ) ;
	tail = s.substr( p1 + 1 ) ;

	w = word( head, 2 ) ;
	if ( w == "*" )
	{
		all = true ;
	}
	else if ( isnumeric( w ) )
	{
		num = ds2d( w ) ;
	}
	else
	{
		err = "Invalid parameter.  Must be '*' or a number" ;
		return ;
	}

	w = word( head, 3 ) ;
	if ( w == "DISKW" )
	{
		read = false ;
	}
	else if ( w != "DISKR" )
	{
		err = "Invalid parameter.  Must be DISKR or DISKW" ;
		return ;
	}

	ddn = word( head, 4 ) ;
	if ( ddn == "" )
	{
		err = "DDNAME missing" ;
		return ;
	}

	open  = parseString2( tail, "OPEN" ) ;
	finis = parseString2( tail, "FINIS" ) ;
	skip  = parseString2( tail, "SKIP" ) ;

	p1 = wordpos( "FIFO", tail ) ;
	p2 = wordpos( "LIFO", tail ) ;
	p3 = wordpos( "STEM", tail ) ;

	if ( !read && ( p1 > 0 || p2 > 0 ) )
	{
		err = "FIFO and LIFO can only be specified with DISKR" ;
		return ;
	}

	if ( ( p1 > 0 && ( p2 > 0 || p3 > 0 ) ) || ( p2 > 0 && p3 > 0 ) )
	{
		err = "Specifiy only one of FIFO, LIFO or STEM" ;
		return ;
	}

	if ( skip && !read )
	{
		err = "SKIP can only be specified with DISKR" ;
		return ;
	}

	if ( p1 > 0 )
	{
		idelword( tail, p1, 1 ) ;
	}
	else if ( p2 > 0 )
	{
		lifo = true ;
		fifo = false ;
		idelword( tail, p2, 1 ) ;
	}
	else if ( p3 > 0 )
	{
		stem = word( tail, p3+1 ) ;
		if ( stem == "" )
		{
			err = "STEM variable must be specified after the STEM keyword" ;
			return ;
		}
		idelword( tail, p3, 2 ) ;
		fifo = false ;
	}

	trim( tail ) ;

	if ( tail != "" && tail != ")" )
	{
		err = "Unknown parameter " + tail ;
		return ;
	}
}
