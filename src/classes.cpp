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

void parser::parseStatement( errblock& err, const string& s )
{
	bool quoted ;
	char c1     ;

	string u ;

	token  t ;

	string::const_iterator it ;

	tokens.clear() ;

	err.setRC( 0 ) ;
	idx = 0        ;
	it  = s.begin() ;

	while ( it != s.end() )
	{
		getNextString( err, it, s, u, quoted ) ;
		if ( err.error() ) { return ; }
		if ( u == "" ) { break ; }
		c1 = u.front() ;
		t.clear() ;
		if ( quoted )
		{
			t.type  = TT_STRING_QUOTED ;
			t.value = u ;
		}
		else if ( u.size() > 1 && ( c1 == '=' || c1 == '<' || c1 =='>' || c1 == '!' ) )
		{
			t.subtype = TS_COMPARISON_OP ;
			t.value   = u ;
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
			t.value = u ;
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
			t.value = u ;
		}
		else if ( u == "(" )
		{
			t.subtype = TS_OPEN_BRACKET ;
			t.value   = "(" ;
		}
		else if ( u == ")" )
		{
			t.subtype = TS_CLOSE_BRACKET ;
			t.value   = ")" ;
		}
		else if ( u == "," )
		{
			t.subtype = TS_COMMA ;
			t.value   = "," ;
		}
		else if ( u == "=" )
		{
			t.subtype = TS_EQUALS ;
			t.value   = "=" ;
		}
		else
		{
			if ( optUpper ) { iupper( u ) ; }
			if ( isvalidName( u ) )
			{
				t.subtype = TS_NAME ;
			}
			t.type  = TT_STRING_UNQUOTED ;
			t.value = u ;
		}
		t.idx = tokens.size() ;
		tokens.push_back( t ) ;
	}
	tokens.size() > 0 ? current_token = tokens[ 0 ] : current_token = token( TT_EOT ) ;
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

	for ( int j = 0 ; j < tokens.size() ; j++ )
	{
		tokens[ j ].idx = j ;
	}
}


token parser::getToken( int i )
{
	if ( i < tokens.size() )
	{
		return tokens[ i ] ;
	}
	return token( TT_EOT ) ;
}


token parser::getFirstToken()
{
	idx = 0 ;

	tokens.size() > 0 ? current_token = tokens[ 0 ] : current_token = token( TT_EOT ) ;
	return current_token ;
}


token parser::getNextToken()
{
	idx++ ;

	tokens.size() > idx ? current_token = tokens[ idx ] : current_token = token( TT_EOT ) ;
	return current_token ;
}


string parser::peekNextValue()
{
	int i = idx + 1 ;

	return tokens.size() > i ? tokens[ i ].value : "" ;
}


token parser::getCurrentToken()
{
	return current_token ;
}


string parser::getCurrentValue()
{
	return current_token.value ;
}


bool parser::isCurrentType( TOKEN_TYPES t )
{
	return ( t == current_token.type ) ;
}


bool parser::isCurrentSubType( TOKEN_SUBTYPES t )
{
	return ( t == current_token.subtype ) ;
}


bool parser::getNextIfCurrent( TOKEN_TYPES tok )
{
	if ( current_token.type == tok )
	{
		idx++ ;
		idx < tokens.size() ? current_token = tokens[ idx ] : current_token = token( TT_EOT ) ;
		return true ;
	}
	return false ;
}


bool parser::getNextIfCurrent( TOKEN_SUBTYPES tok )
{
	if ( current_token.subtype == tok )
	{
		idx++ ;
		idx < tokens.size() ? current_token = tokens[ idx ] : current_token = token( TT_EOT ) ;
		return true ;
	}
	return false ;
}


STATEMENT_TYPE parser::getStatementType()
{
	// Return the panel language statement type, ST_EOF if empty or ST_ERROR if not recognised.

	if ( tokens.size() == 0 )
	{
		return ST_EOF ;
	}

	token t = getToken( 0 ) ;

	if      ( t.value == "IF" )      { return ST_IF      ; }
	else if ( t.value == "ELSE" )    { return ST_ELSE    ; }
	else if ( t.value == "VGET" )    { return ST_VGET    ; }
	else if ( t.value == "VPUT" )    { return ST_VPUT    ; }
	else if ( t.value == "GOTO" )    { return ST_GOTO    ; }
	else if ( t.value == "EXIT" )    { return ST_EXIT    ; }
	else if ( t.value == "VER" )     { return ST_VERIFY  ; }
	else if ( t.value == "REFRESH" ) { return ST_REFRESH ; }
	else if ( t.value == ".ATTR" )
	{
		if ( tokens.size() < 6 ) { return ST_ERROR ; }
		if ( tokens[ 4 ].subtype == TS_EQUALS ) { return ST_ASSIGN ; }
	}

	if ( tokens.size() < 2 ) { return ST_ERROR ; }

	if ( tokens[ 1 ].subtype == TS_EQUALS ) { return ST_ASSIGN ; }

	return ST_ERROR ;
}



void parser::getNextString( errblock& err, string::const_iterator& it, const string& s, string& r, bool& quoted )
{
	// Get the next word in string s and place in r.
	// Words can be delimited by space or "()=,<>!".  "()=,<>!" are also returned as words.

	const string delims = " (),=<>!" ;
	const string compar = "=<>!"     ;

	string::const_iterator itt ;

	err.setRC( 0 ) ;

	quoted = false ;
	r      = ""    ;

	while ( it != s.end() && (*it) == ' ' ) { it++ ; }
	if ( it == s.end() )
	{
		return ;
	}

	if ( (*it) == '=' )
	{
		it++ ;
		if ( it == s.end() || ( (*it) != '<' && (*it) !='>' && (*it) != '!' ) )
		{
			r = "=" ;
			return ;
		}
	}

	if ( (*it) == '=' || (*it) == '<' || (*it) =='>' || (*it) == '!' )
	{
		itt = it ;
		it++ ;
		while ( it != s.end() && ( compar.find( (*it) ) != string::npos ) ) { it++ ; }
		r.assign( itt, it ) ;
		return ;
	}
	else if ( (*it) == ',' || (*it) == '(' || (*it) == ')' )
	{
		r = (*it) ;
		it++ ;
	}
	else if ( (*it) == '\'' )
	{
		it++ ;
		itt = it ;
		while ( it != s.end() && (*it) != '\'' ) { it++ ; }
		if ( it == s.end() )
		{
			err.seterrid( "PSYE033F" ) ;
			return ;
		}
		quoted = true ;
		r.assign( itt, it ) ;
		it++ ;
	}
	else
	{
		itt = it ;
		while ( it != s.end() && ( delims.find( (*it) ) == string::npos ) ) { it++ ; }
		r.assign( itt, it ) ;
	}
}


void parser::getNameList( errblock& err, string& r )
{
	// Return a list of valid variable names (type TS_NAME or TS_AMPR_VAR_VALID)
	// separated by spaces or commas and between brackets or a single entry.

	err.setRC( 0 ) ;

	if ( getNextIfCurrent( TS_OPEN_BRACKET ) )
	{
		if ( getNextIfCurrent( TS_CLOSE_BRACKET ) )
		{
			err.seterrid( "PSYE031G" ) ;
			return ;
		}
		while ( true )
		{
			if ( getNextIfCurrent( TS_COMMA ) ) { continue ; }
			if ( getNextIfCurrent( TS_CLOSE_BRACKET ) ) { break ; }
			if ( current_token.subtype == TS_NAME ||
			     current_token.subtype == TS_AMPR_VAR_VALID )
			{
				r += " " + current_token.value ;
				getNextToken() ;
			}
			else if ( current_token.type == TT_EOT )
			{
				err.seterrid( "PSYE032D", current_token.value ) ;
				return ;
			}
			else
			{
				err.seterrid( "PSYE031D", current_token.value ) ;
				return ;
			}
		}
	}
	else if ( current_token.subtype == TS_NAME ||
		  current_token.subtype == TS_AMPR_VAR_VALID )
	{
		r = current_token.value ;
		getNextToken() ;
	}
	else
	{
		err.seterrid( "PSYE031D", current_token.value ) ;
		return ;
	}
}


void IFSTMNT::parse( errblock& err, parser& v )
{
	// Format of the IF panel statement

	// IF ( COND ) opt-statement

	token t ;

	v.getFirstToken() ;
	v.getNextToken()  ;

	err.setRC( 0 ) ;

	if ( !v.getNextIfCurrent( TS_OPEN_BRACKET ) )
	{
		err.seterrid( "PSYE033D" ) ;
		return ;
	}

	parse_cond( err, v ) ;
	if ( err.error() ) { return ; }

	if ( if_lhs == "" && if_trunc == NULL && if_verify == NULL )
	{
		err.seterrid( "PSYE031G" ) ;
		return ;
	}

	t = v.getCurrentToken()  ;
	if ( t.subtype != TS_CLOSE_BRACKET )
	{
		err.seterrid( "PSYE032D" ) ;
		return ;
	}
	v.eraseTokens( t.idx ) ;
	v.getFirstToken()      ;
}


void IFSTMNT::parse_cond( errblock& err, parser& v )
{
	// Format of the IF-condition panel statement

	// &AAA=&BBBB
	// &AAA = VALUE1,VALUE2
	// &AAA NE 'Hello','Goodbye'
	// &AAA=&BBBB & &CCC = DDDD AND &EEEE = FFFFF
	// &AAA=&BBBB | &CCC = DDDD OR  &EEEE = FFFFF
	// VER (&A...) | &A = 'CAN'
	// TRUNC(...) = 'VAL1','VAL2' AND VER(...)

	token t ;

	t = v.getCurrentToken() ;
	if ( t.subtype == TS_CLOSE_BRACKET )
	{
		err.seterrid( "PSYE031J" ) ;
		return ;
	}
	else if ( t.type == TT_EOT )
	{
		err.seterrid( "PSYE031K" ) ;
		return ;
	}

	if ( t.value == "VER" && v.peekNextValue() == "(" )
	{
		if_verify = new VERIFY ;
		v.eraseTokens( t.idx - 1 ) ;
		if_verify->parse( err, v, false ) ;
		if ( err.error() ) { return ; }
		if ( if_verify->ver_msgid != "" )
		{
			err.seterrid( "PSYE033A" ) ;
			return ;
		}
		t = v.getCurrentToken() ;
		if ( t.subtype == TS_CLOSE_BRACKET || t.type == TT_EOT ) { return ; }
		if ( t.type    != TT_STRING_QUOTED && findword( t.value, "& | AND OR" ) )
		{
			if_AND  = ( t.value == "&" || t.value == "AND" ) ;
			if_next = new IFSTMNT ;
			v.getNextToken() ;
			if_next->parse_cond( err, v ) ;
			if ( err.error() ) { return ; }
			if ( if_next->if_lhs == "" && if_next->if_trunc == NULL && if_next->if_verify == NULL )
			{
				err.seterrid( "PSYE031G" ) ;
			}
		}
		else
		{
			err.seterrid( "PSYE032H", t.value ) ;
		}
		return ;
	}
	else if ( t.value == "TRUNC" && v.peekNextValue() == "(" )
	{
		if_trunc = new TRUNC ;
		if_trunc->parse( err, v, false ) ;
		if ( err.error() ) { return ; }
		t = v.getCurrentToken() ;
	}
	else
	{
		if_lhs = t.value ;
		if ( if_lhs == "" )
		{
			err.seterrid( "PSYE033B" ) ;
			return ;
		}
		else if ( t.subtype == TS_CTL_VAR_INVALID )
		{
			err.seterrid( "PSYE033G", if_lhs ) ;
			return ;
		}
		else if ( t.subtype == TS_AMPR_VAR_INVALID )
		{
			err.seterrid( "PSYE031D", t.value ) ;
			return ;
		}
		t = v.getNextToken() ;
	}

	if      ( t.value == "="  ) { if_cond = IF_EQ ; }
	else if ( t.value == "EQ" ) { if_cond = IF_EQ ; }
	else if ( t.value == "!=" ) { if_cond = IF_NE ; }
	else if ( t.value == "NE" ) { if_cond = IF_NE ; }
	else if ( t.value == ">"  ) { if_cond = IF_GT ; }
	else if ( t.value == "GT" ) { if_cond = IF_GT ; }
	else if ( t.value == "<"  ) { if_cond = IF_LT ; }
	else if ( t.value == "LT" ) { if_cond = IF_LT ; }
	else if ( t.value == ">=" ) { if_cond = IF_GE ; }
	else if ( t.value == "=>" ) { if_cond = IF_GE ; }
	else if ( t.value == "GE" ) { if_cond = IF_GE ; }
	else if ( t.value == "!<" ) { if_cond = IF_GE ; }
	else if ( t.value == "NL" ) { if_cond = IF_GE ; }
	else if ( t.value == "<=" ) { if_cond = IF_LE ; }
	else if ( t.value == "=<" ) { if_cond = IF_LE ; }
	else if ( t.value == "LE" ) { if_cond = IF_LE ; }
	else if ( t.value == "!>" ) { if_cond = IF_LE ; }
	else if ( t.value == "NG" ) { if_cond = IF_LE ; }
	else
	{
		err.seterrid( "PSYE033E", t.value ) ;
		return ;
	}

	t = v.getNextToken() ;
	if ( t.subtype == TS_CTL_VAR_INVALID )
	{
		err.seterrid( "PSYE033G", t.value ) ;
		return ;
	}
	else if ( t.subtype == TS_AMPR_VAR_INVALID )
	{
		err.seterrid( "PSYE031D", t.value ) ;
		return ;
	}

	if ( t.type == TT_OTHER )
	{
		err.seterrid( "PSYE033Q", t.value ) ;
		return ;
	}

	if_rhs.push_back( t.value ) ;

	while ( true )
	{
		t = v.getNextToken() ;
		if ( t.type != TT_STRING_QUOTED && findword( t.value, "& | AND OR" ) && v.peekNextValue() != "," )
		{
			break ;
		}
		else if ( t.type == TT_EOT )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		else if ( t.subtype == TS_COMMA )
		{
			if ( v.peekNextValue() == "," || v.peekNextValue() == ")" )
			{
				if_rhs.push_back( "" ) ;
			}
			continue ;
		}
		else if ( t.subtype == TS_CLOSE_BRACKET )
		{
			break ;
		}
		else if ( t.subtype == TS_CTL_VAR_INVALID )
		{
			err.seterrid( "PSYE033G", t.value ) ;
			return ;
		}
		else if ( t.subtype == TS_AMPR_VAR_INVALID )
		{
			err.seterrid( "PSYE031D", t.value ) ;
			return ;
		}
		if_rhs.push_back( t.value ) ;
	}

	if ( if_rhs.size() == 0 )
	{
		err.seterrid( "PSYE031K" ) ;
		return ;
	}

	if ( ( if_cond != IF_EQ && if_cond != IF_NE ) && if_rhs.size() > 1 )
	{
		err.seterrid( "PSYE033H" ) ;
		return ;
	}

	if ( t.type != TT_STRING_QUOTED && findword( t.value, "& | AND OR" ) )
	{
		if_AND  = ( t.value == "&" || t.value == "AND" ) ;
		if_next = new IFSTMNT ;
		v.getNextToken() ;
		if_next->parse_cond( err, v ) ;
		if ( err.error() ) { return ; }
		if ( if_next->if_lhs == "" && if_next->if_trunc == NULL && if_next->if_verify == NULL )
		{
			err.seterrid( "PSYE031G" ) ;
			return ;
		}
	}
}


void ASSGN::parse( errblock& err, parser& v )
{
	// Format of the assignment panel statement

	// .ATTR(ZCMD) = 'xxxx'
	// &AAA = &BBBB
	// &AAA = VALUE
	// &AAA = 'Quoted Value'
	// &AAA = .ALARM | .TRAIL | .HELP | .MSG | .CSRPOS | .CURSOR | .RESP
	// .ALARM .RESP | .HELP | .MSG | .CSRPOS | .CURSOR = &BBB | VALUE | 'Quoted Value'
	// &AAA = UPPER( ABC )
	// &AAA = LENGTH( ABC )
	// &AAA = REVERSE( ABC )
	// &AAA = WORDS( ABC )  Number of words in the value of ABC
	// &A   = EXISTS( ABC ) True if file/directory in variable ABC exists
	// &A   = FILE( ABC )   True if file in variable ABC exists
	// &A   = DIR( ABC )    True if directory in variable ABC exists
	// &A   = TRUNC()
	// &A   = TRANS()

	token t ;

	map<string, AS_FUNCTION> assign_functions =
	{ { "DIR",     AS_DIR     },
	  { "EXISTS",  AS_EXISTS  },
	  { "FILE",    AS_FILE    },
	  { "LENGTH",  AS_LENGTH  },
	  { "TRANS",   AS_TRANS   },
	  { "TRUNC",   AS_TRUNC   },
	  { "REVERSE", AS_REVERSE },
	  { "WORDS",   AS_WORDS   },
	  { "UPPER",   AS_UPPER   } } ;

	const string functn_list = "DIR EXISTS FILE LENGTH REVERSE WORDS UPPER" ;

	map<string, AS_FUNCTION>::iterator it;

	err.setRC( 0 ) ;

	t = v.getFirstToken() ;
	if ( t.type == TT_STRING_QUOTED )
	{
		err.seterrid( "PSYE031H" ) ;
		return ;
	}

	if ( v.getNextIfCurrent( TS_AMPR_VAR_VALID ) )
	{
		as_lhs = t.value.substr( 1 ) ;
	}
	else if ( t.value == ".ATTR" )
	{
		v.getNextToken() ;
		if ( !v.getNextIfCurrent( TS_OPEN_BRACKET ) )
		{
			err.seterrid( "PSYE033D" ) ;
			return ;
		}
		if ( v.isCurrentSubType( TS_CLOSE_BRACKET ) )
		{
			err.seterrid( "PSYE031G" ) ;
			return ;
		}
		as_lhs = v.getCurrentValue() ;
		if ( !v.getNextIfCurrent( TS_NAME ) )
		{
			err.seterrid( "PSYE031D", as_lhs ) ;
			return ;
		}
		if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		as_isattr = true ;
	}
	else if ( findword( t.value, ".FALSE .PFKEY .TRUE .TRAIL" ) )
	{
		err.seterrid( "PSYE033S", t.value ) ;
		return ;
	}
	else if ( t.subtype == TS_CTL_VAR_INVALID )
	{
		err.seterrid( "PSYE033G", t.value ) ;
		return ;
	}
	else if ( t.subtype == TS_AMPR_VAR_INVALID )
	{
		err.seterrid( "PSYE031D", t.value ) ;
		return ;
	}
	else if ( t.subtype == TS_CTL_VAR_VALID )
	{
		as_lhs = t.value ;
		t      = v.getNextToken() ;
	}
	else
	{
		err.seterrid( "PSYE033Q", t.value ) ;
		return ;
	}

	if ( !v.getNextIfCurrent( TS_EQUALS ) )
	{
		err.seterrid( "PSYE033O" ) ;
		return ;
	}

	t = v.getCurrentToken() ;
	if ( t.subtype == TS_CTL_VAR_INVALID )
	{
		err.seterrid( "PSYE033G", t.value ) ;
		return ;
	}
	else if ( t.subtype == TS_AMPR_VAR_INVALID )
	{
		err.seterrid( "PSYE031D", t.value ) ;
		return ;
	}
	else if ( v.getNextIfCurrent( TS_CTL_VAR_VALID ) )
	{
		as_rhs = t.value ;
	}
	else if ( v.peekNextValue() == "(" )
	{
		it = assign_functions.find( t.value ) ;
		if ( it == assign_functions.end() )
		{
			err.seterrid( "PSYE033U", t.value ) ;
			return ;
		}
		as_function = it->second ;
		switch ( as_function )
		{
		case AS_TRUNC:
			as_trunc = new TRUNC ;
			as_trunc->parse( err, v ) ;
			if ( err.error() ) { return ; }
			break ;

		case AS_TRANS:
			as_trans = new TRANS ;
			as_trans->parse( err, v ) ;
			if ( err.error() ) { return ; }
			break ;

		default:
			t = v.getNextToken() ;
			t = v.getNextToken() ;
			if ( v.isCurrentSubType( TS_CLOSE_BRACKET ) )
			{
				err.seterrid( "PSYE031G" ) ;
				return ;
			}
			as_rhs = v.getCurrentValue() ;
			if ( !v.getNextIfCurrent( TS_NAME ) && !v.getNextIfCurrent( TS_CTL_VAR_VALID ) )
			{
				err.seterrid( as_rhs.front() == '.' ? "PSYE033G" : "PSYE031D", as_rhs ) ;
				return ;
			}
			if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
			{
				err.seterrid( "PSYE032D" ) ;
				return ;
			}
			as_isvar = true ;
		}
	}
	else
	{
		as_rhs = t.value ;
		if ( as_isattr ) { iupper( as_rhs ) ; }
		v.getNextToken() ;
	}

	if ( !v.isCurrentType( TT_EOT ) )
	{
		err.seterrid( "PSYE032H", v.getCurrentValue() ) ;
		return ;
	}
}


void VPUTGET::parse( errblock& err, parser& v )
{
	// VGET ABC
	// VGET(ABC) PROFILE

	token t ;

	err.setRC( 0 ) ;

	t = v.getFirstToken() ;
	vpg_vput = ( t.value == "VPUT" ) ;

	v.getNextToken() ;

	v.getNameList( err, vpg_vars ) ;
	if ( err.error() ) { return ; }

	t = v.getCurrentToken() ;
	if ( t.value == "ASIS" ||
	     t.type  == TT_EOT )         { vpg_pool = ASIS    ; }
	else if ( t.value == "SHARED" )  { vpg_pool = SHARED  ; }
	else if ( t.value == "PROFILE" ) { vpg_pool = PROFILE ; }
	else
	{
		err.seterrid( "PSYE033Z", t.value ) ;
		return ;
	}

	t = v.getNextToken() ;
	if ( t.type != TT_EOT )
	{
		err.seterrid( "PSYE032H", t.value ) ;
		return ;
	}
}


void VERIFY::parse( errblock& err, parser& v, bool check )
{
	// VER (&VAR LIST A B C D)
	// VER (&VAR,LIST,A,B,C,D)
	// VER (&VAR NB LIST A B C D)
	// VER(&VAR NONBLANK LIST A B C D)
	// VER(&VAR PICT ABCD MSG = PSYS011A )
	// VER(&VAR HEX)
	// VER(&VAR OCT)

	token t ;

	err.setRC( 0 ) ;

	v.getFirstToken() ;
	v.getNextToken()  ;

	if ( !v.getNextIfCurrent( TS_OPEN_BRACKET ) )
	{
		err.seterrid( "PSYE033D" ) ;
		return ;
	}

	t = v.getCurrentToken() ;
	ver_var = t.value.substr( 1 ) ;

	if ( !v.getNextIfCurrent( TS_AMPR_VAR_VALID ) )
	{
		err.seterrid( "PSYE033I", t.value ) ;
	}

	v.getNextIfCurrent( TS_COMMA ) ;
	t = v.getCurrentToken() ;

	if ( findword( t.value, "NB NONBLANK" ) )
	{
		ver_nblank = true ;
		v.getNextToken()  ;
		if ( v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
		{
			if ( !v.isCurrentType( TT_EOT ) )
			{
				err.seterrid( "PSYE032H", v.getCurrentValue() ) ;
				return ;
			}
			return ;
		}
		v.getNextIfCurrent( TS_COMMA ) ;
		t = v.getCurrentToken() ;
	}

	if      ( t.value == "NUM"   ) { ver_type = VER_NUMERIC ; }
	else if ( t.value == "LIST"  ) { ver_type = VER_LIST    ; }
	else if ( t.value == "LISTX" ) { ver_type = VER_LISTX ; ver_nblank = true ; }
	else if ( t.value == "PICT"  ) { ver_type = VER_PICT ; }
	else if ( t.value == "HEX"   ) { ver_type = VER_HEX  ; }
	else if ( t.value == "OCT"   ) { ver_type = VER_OCT  ; }
	else if ( t.value == "MSG"   ) {                     ; }
	else
	{
		err.seterrid( "PSYE033L" ) ;
		return ;
	}

	if ( t.value != "MSG" )
	{
		v.getNextToken()  ;
		v.getNextIfCurrent( TS_COMMA ) ;
		t = v.getCurrentToken() ;
	}

	if ( ver_type == VER_PICT )
	{
		ver_vlist.push_back( t.value ) ;
		v.getNextToken() ;
		v.getNextIfCurrent( TS_COMMA ) ;
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
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		else if ( t.subtype == TS_COMMA )
		{
			if ( v.peekNextValue() == "," || v.peekNextValue() == ")" )
			{
				ver_vlist.push_back( "" ) ;
			}
			t = v.getNextToken() ;
			continue ;
		}
		else if ( t.value == "MSG" )
		{
			break ;
		}
		else if ( t.subtype == TS_AMPR_VAR_INVALID )
		{
			err.seterrid( "PSYE031D", t.value ) ;
			return ;
		}
		else
		{
			ver_vlist.push_back( t.value ) ;
		}
		t = v.getNextToken() ;
	}

	if ( t.value == "MSG" )
	{
		v.getNextToken() ;
		if ( !v.getNextIfCurrent( TS_EQUALS ) )
		{
			err.seterrid( "PSYE033O" ) ;
			return ;
		}
		t = v.getCurrentToken() ;
		if ( v.getNextIfCurrent( TS_NAME ) ||
		     v.getNextIfCurrent( TS_AMPR_VAR_VALID ) )
		{
			ver_msgid = t.value ;
		}
		else
		{
			err.seterrid( "PSYE031I", t.value ) ;
			return ;
		}
	}

	if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
	{
		err.seterrid( "PSYE032D" ) ;
		return ;
	}

	if ( check && !v.isCurrentType( TT_EOT ) )
	{
		err.seterrid( "PSYE032H", v.getCurrentValue() ) ;
		return ;
	}

	if ( ( ver_type == VER_LIST || ver_type == VER_LISTX || ver_type == VER_PICT ) && ver_vlist.empty() )
	{
		err.seterrid( "PSYE033N" ) ;
		return ;
	}
}


void TRUNC::parse( errblock& err, parser& v, bool check )
{
	// Format of the TRUNC panel statement
	// TRUNC( &BBB,'.' )
	// TRUNC ( &BBB, 3  )

	token t ;

	err.setRC( 0 ) ;

	v.getNextToken() ;

	if ( !v.getNextIfCurrent( TS_OPEN_BRACKET ) )
	{
		err.seterrid( "PSYE033D" ) ;
		return ;
	}

	t = v.getCurrentToken() ;

	if ( t.type == TT_VARIABLE )
	{
		if ( t.value == ".TRAIL" )
		{
			err.seterrid( "PSYE038G" ) ;
			return ;
		}
		trnc_field = t.value ;
	}
	else if ( t.subtype == TS_CTL_VAR_INVALID )
	{
		err.seterrid( "PSYE033G", t.value ) ;
		return ;
	}
	else
	{
		t.value == "" ? err.seterrid( "PSYE038A" ) : err.seterrid( "PSYE033I", t.value ) ;
		return ;
	}

	v.getNextToken() ;
	if ( !v.getNextIfCurrent( TS_COMMA ) )
	{
		err.seterrid( "PSYE038C" ) ;
		return ;
	}

	t = v.getCurrentToken() ;
	if ( t.type == TT_STRING_QUOTED ||
	   ( t.type == TT_STRING_UNQUOTED && !datatype( t.value, 'W' ) ) )
	{
		if ( t.value.size() != 1 )
		{
			err.seterrid( "PSYE038F", t.value ) ;
			return ;
		}
		trnc_char = t.value.front() ;
	}
	else
	{
		if ( !datatype( t.value, 'W' ) )
		{
			err.seterrid( "PSYE019E", t.value ) ;
			return ;
		}
		trnc_len = ds2d( t.value ) ;
		if ( trnc_len <= 0 )
		{
			err.seterrid( "PSYE019F" ) ;
			return ;
		}
	}

	v.getNextToken() ;
	if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
	{
		err.seterrid( "PSYE032D" ) ;
		return ;
	}

	if ( check && !v.isCurrentType( TT_EOT ) )
	{
		err.seterrid( "PSYE032H", v.getCurrentValue() ) ;
		return ;
	}
	return ;
}


void TRANS::parse( errblock& err, parser& v )
{
	// Format of the TRANS panel statement ( change val1 to val2, * is everything else.  Issue message. )

	// TRANS( &BBB  val1,val2 ...  *,* )
	// TRANS ( &BBB  val1,val2 ...  *,'?' )
	// TRANS ( &BBB  val1,val2 ...  ) non-matching results in &AAA being set to null
	// TRANS ( &AAA  val1,val2 ...  MSG=msgid ) issue message if no match
	// TRANS ( &AAA  &v1,&v2 ...  MSG = msgid ) issue message if no match
	// TRANS ( TRUNC(...)  &v1,&v2 ...  MSG = msgid ) issue message if no match

	string v1 ;
	string v3 ;

	bool first ;

	token t ;

	err.setRC( 0 ) ;

	t = v.getNextToken() ;

	if ( !v.getNextIfCurrent( TS_OPEN_BRACKET ) )
	{
		err.seterrid( "PSYE033D" ) ;
		return ;
	}

	trns_field = v.getCurrentValue() ;
	if ( trns_field == "TRUNC" && v.peekNextValue() == "(" )
	{
		trns_trunc = new TRUNC ;
		trns_trunc->parse( err, v, false ) ;
		if ( err.error() ) { return ; }
	}
	else if ( !v.getNextIfCurrent( TS_AMPR_VAR_VALID ) )
	{
		trns_field == "" ? err.seterrid( "PSYE038A" ) : err.seterrid( "PSYE033I", trns_field ) ;
		return ;
	}
	else
	{
		trns_field.erase( 0, 1 ) ;
	}

	first = true ;
	while ( true )
	{
		t = v.getCurrentToken() ;
		if ( t.type == TT_EOT )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		else if ( v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
		{
			if ( !first )
			{
				err.seterrid( "PSYE039H" ) ;
				return ;
			}
			break ;
		}
		if ( first && t.value == "MSG" && v.peekNextValue() == "=" )
		{
			t = v.getNextToken() ;
			t = v.getNextToken() ;
			if ( v.getNextIfCurrent( TS_NAME ) ||
			     v.getNextIfCurrent( TS_AMPR_VAR_VALID ) )
			{
				trns_msgid = t.value ;
			}
			else
			{
				err.seterrid( "PSYE031D", t.value ) ;
				return ;
			}
			if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
			{
				err.seterrid( "PSYE032D" ) ;
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
				v1 = t.value ;
				first = false ;
				v.getNextToken() ;
				continue ;
			}
			if ( v1 == "*" )
			{
				trns_default = t.value ;
			}
			else
			{
				trns_list.push_back( make_pair( v1, t.value )  ) ;
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
		err.seterrid( "PSYE032H", v.getCurrentValue() ) ;
		return ;
	}

	if ( !v.isCurrentType( TT_EOT ) )
	{
		err.seterrid( "PSYE032H", v.getCurrentValue() ) ;
		return ;
	}
	return ;
}


void pnts::parse( errblock& err, string s )
{
	// Format of the PNTS panel entry (point-and-shoot entries)

	// FIELD(fld) VAR(var) VAL(value)
	// fld and var must be defined in the panel

	iupper( s ) ;

	err.setRC( 0 ) ;

	pnts_field = parseString( err, s, "FIELD()" ) ;
	if ( pnts_field == "" )
	{
		err.seterrid( "PSYE031C", "FIELD" ) ;
		return ;
	}
	if ( !isvalidName( pnts_field ) )
	{
		err.seterrid( "PSYE031D", pnts_field ) ;
		return ;
	}

	pnts_var = parseString( err, s, "VAR()" ) ;
	if ( pnts_var == "" )
	{
		err.seterrid( "PSYE031C", "VAR" ) ;
		return ;
	}

	if ( !isvalidName( pnts_var ) )
	{
		err.seterrid( "PSYE031D", pnts_var ) ;
		return ;
	}

	pnts_val = parseString( err, s, "VAL()" ) ;
	if ( pnts_val == "" )
	{
		err.seterrid( "PSYE031C", "VAL" ) ;
		return ;
	}

	if ( pnts_field == "" || pnts_var == "" || pnts_val == "" )
	{
		err.seterrid( "PSYE037A" ) ;
		return ;
	}

	if ( trim( s ) != "" )
	{
		err.seterrid( "PSYE032H", s ) ;
		return ;
	}
	return ;
}


void fieldExc::parse( errblock& err, string s )
{
	fieldExc_field = parseString( err, s, "FIELD()" ) ;
	if ( err.error() ) { return ; }
	if ( fieldExc_field == "" )
	{
		err.seterrid( "PSYE042G" ) ;
		return ;
	}

	fieldExc_command = parseString( err, s, "EXEC()" ) ;
	if ( err.error() ) { return ; }
	if ( fieldExc_command == "" )
	{
		err.seterrid( "PSYE042H" ) ;
		return ;
	}
	istrip( fieldExc_command, 'B', '\'' ) ;

	fieldExc_passed = parseString( err, s, "PASS()" ) ;
	if ( err.error() ) { return ; }
	if ( s != "" )
	{
		err.seterrid( "PSYE032H", s ) ;
		return ;
	}
}


bool slmsg::parse( const string& s, const string& l )
{
	// Parse message and fill the slmsg object.
	// .TYPE overrides .WINDOW and .ALARM

	int p1 ;
	int p2 ;
	int ln ;

	char c ;

	string rest ;
	string tmp  ;

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
		if ( p1 == string::npos ) { return false ; }
		rest = substr( smsg, p1+2 )   ;
		smsg = smsg.substr( 1, p1-1 ) ;
		dquote( c, smsg ) ;
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

	p1 = pos( ".HELP=", rest ) ;
	if ( p1 == 0 )
	{
		p1 = pos( ".H=", rest ) ;
		ln = 3 ;
	}
	else
	{
		ln = 6 ;
	}
	if ( p1 > 0 )
	{
		p2 = pos( " ", rest, p1 ) ;
		if ( p2 == 0 ) { hlp = substr( rest, p1+ln )           ; idelstr( rest, p1 )        ; }
		else           { hlp = substr( rest, p1+ln, p2-p1-ln ) ; idelstr( rest, p1, p2-p1 ) ; }
		if ( hlp.size()  == 0 ) { return false ; }
		if ( hlp.front() == '&')
		{
			hlp.erase( 0, 1 ) ;
			if ( !isvalidName( hlp ) ) { return false ; }
			dvhlp = hlp ;
		}
	}

	p1 = pos( ".WINDOW=", rest ) ;
	if ( p1 == 0 )
	{
		p1 = pos( ".W=", rest ) ;
		ln = 3 ;
	}
	else
	{
		ln = 8 ;
	}
	if ( p1 > 0 )
	{
		lmwin = true ;
		p2 = pos( " ", rest, p1 ) ;
		if ( p2 == 0 ) { tmp = substr( rest, p1+ln )           ; idelstr( rest, p1 )        ; }
		else           { tmp = substr( rest, p1+ln, p2-p1-ln ) ; idelstr( rest, p1, p2-p1 ) ; }
		if ( tmp.size()  == 0 ) { return false ; }
		if ( tmp.front() == '&')
		{
			tmp.erase( 0, 1 ) ;
			if ( !isvalidName( tmp ) ) { return false ; }
			dvwin = tmp ;
		}
		else if ( tmp == "RESP"    || tmp == "R"  ) { smwin = true  ; resp = true ; }
		else if ( tmp == "NORESP"  || tmp == "N"  ) { smwin = true  ; }
		else if ( tmp == "LRESP"   || tmp == "LR" ) { resp  = true  ; }
		else if ( tmp == "LNORESP" || tmp == "LN" ) {                 }
		else    { return false  ; }
	}

	p1 = pos( ".ALARM=", rest ) ;
	if ( p1 == 0 )
	{
		p1 = pos( ".A=", rest ) ;
		ln = 3 ;
	}
	else
	{
		ln = 7 ;
	}
	if ( p1 > 0 )
	{
		p2 = pos( " ", rest, p1 ) ;
		if ( p2 == 0 ) { tmp = substr( rest, p1+ln )           ; idelstr( rest, p1 )        ; }
		else           { tmp = substr( rest, p1+ln, p2-p1-ln ) ; idelstr( rest, p1, p2-p1 ) ; }
		if ( tmp.size()  == 0 ) { return false ; }
		if ( tmp.front() == '&')
		{
			tmp.erase( 0, 1 ) ;
			if ( !isvalidName( tmp ) ) { return false ; }
			dvalm = tmp ;
		}
		else if ( tmp == "YES" ) { alm = true   ; }
		else if ( tmp == "NO"  ) { alm = false  ; }
		else                     { return false ; }
	}

	p1 = pos( ".TYPE=", rest ) ;
	if ( p1 == 0 )
	{
		p1 = pos( ".T=", rest ) ;
		ln = 3 ;
	}
	else
	{
		ln = 6 ;
	}
	if ( p1 > 0 )
	{
		p2 = pos( " ", rest, p1 ) ;
		if ( p2 == 0 ) { tmp = substr( rest, p1+ln )           ; idelstr( rest, p1 )        ; }
		else           { tmp = substr( rest, p1+ln, p2-p1-ln ) ; idelstr( rest, p1, p2-p1 ) ; }
		if ( tmp.size()  == 0 ) { return false ; }
		if ( tmp.front() == '&')
		{
			tmp.erase( 0, 1 ) ;
			if ( !isvalidName( tmp ) ) { return false ; }
			dvtype = tmp ;
		}
		else if ( tmp == "N" ) { type = IMT ; alm = false ; }
		else if ( tmp == "W" ) { type = WMT ; alm = true  ; }
		else if ( tmp == "A" ) { type = AMT ; alm = true  ; }
		else if ( tmp == "C" ) { type = AMT ; alm = true  ; resp = true ; smwin = true ; lmwin = true ; }
		else                   { return false             ; }
	}

	if ( trim( rest ) != "" ) { return false ; }

	if ( smwin && smsg != "" ) { lmsg = smsg +" - "+ lmsg ; }

	trim_right( smsg ) ;
	trim_right( lmsg ) ;

	return true ;
}


bool selobj::parse( errblock& err, string SELSTR )
{
	// Case insensitive except for PARM() and CMD()

	// Valid keyword formats:
	// PGM(abc) PARM(xyz) NEWAPPL(abcd) NEWPOOL PASSLIB
	// PGM(abc) PARM(xyz) NEWAPPL NEWPOOL PASSLIB
	// CMD(abc def) LANG(REXX) - translates to PGM(&ZOREXPGM) PARM(abc def)
	// CMD(abc def)            - translates to PGM(&ZOREXPGM) PARM(abc def)
	// PANEL(def)   - translates to PGM(&ZPANLPGM) PARM(def)

	// + SCRNAME(ghi) - give the function a screen name (valid name but not LIST, NEXT, PREV)
	// + SUSPEND      - Suspend any popup windows

	// Match brackets for PARM and CMD as these may contain brackets.  These can also be enclosed in
	// double quotes if needed, that are then removed.

	int ob ;
	int p1 ;
	int p2 ;

	bool oquote ;

	string lang ;
	string str  ;

	err.setRC( 0 ) ;

	clear() ;
	str = upper( SELSTR ) ;
	p1  = pos( "PARM(", str ) ;
	if ( p1 > 0 )
	{
		ob     = 1 ;
		oquote = false ;
		for ( p2 = p1+4 ; p2 < SELSTR.size() ; p2++ )
		{
			if ( SELSTR.at( p2 ) == '"' ) { oquote = !oquote ; }
			if ( oquote ) { continue ; }
			if ( SELSTR.at( p2 ) == '(' ) { ob++  ; }
			if ( SELSTR.at( p2 ) == ')' )
			{
				ob-- ;
				if ( ob == 0 ) { break ; }
			}
		}
		if ( ob != 0 || oquote )
		{
			err.seterrid( "PSYE033F" ) ;
			return false ;
		}
		p2++ ;
		PARM   = strip( substr( SELSTR, (p1 + 5), (p2 - (p1 + 5)) ) ) ;
		PARM   = strip( PARM, 'B', '"' ) ;
		SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
		str    = upper( SELSTR ) ;
	}

	p1 = pos( "PGM(", str ) ;
	if ( p1 > 0 )
	{
		p2 = pos( ")", SELSTR, p1 ) ;
		if ( p2 == 0 )
		{
			err.seterrid( "PSYE032D" ) ;
			return false ;
		}
		PGM    = strip( substr( str, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
		SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
		str    = upper( SELSTR ) ;
		if ( !PGM.empty() && PGM.front() == '&' )
		{
			if ( !isvalidName( substr( PGM, 2 ) ) )
			{
				err.seterrid( "PSYE031D", substr( PGM, 2 ) ) ;
				return false ;
			}
		}
		else
		{
			if ( !isvalidName( PGM ) )
			{
				err.seterrid( "PSYE031E", "PROGRAM", PGM ) ;
				return false ;
			}
		}
	}
	else
	{
		p1 = pos( "PANEL(", str ) ;
		if ( p1 > 0 )
		{
			if ( PARM != "" )
			{
				err.seterrid( "PSYE039M", "PANEL" ) ;
				return false ;
			}
			p2 = pos( ")", SELSTR, p1 ) ;
			if ( p2 == 0 )
			{
				err.seterrid( "PSYE032D" ) ;
				return false ;
			}
			PARM = strip( substr( str, (p1 + 6), (p2 - (p1 + 6)) ) ) ;
			if ( !isvalidName( PARM ) )
			{
				err.seterrid( "PSYE031E", "PANEL", PARM ) ;
				return false ;
			}
			PGM    = "&ZPANLPGM" ;
			SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
			str    = upper( SELSTR ) ;
			p1 = pos( "OPT(", str ) ;
			if ( p1 > 0 )
			{
				p2 = pos( ")", SELSTR, p1 ) ;
				if ( p2 == 0 )
				{
					err.seterrid( "PSYE032D" ) ;
					return false ;
				}
				PARM  += " " + strip( substr( str, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
				SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
				str    = upper( SELSTR ) ;
			}
			selPanl = true ;
		}
		else
		{
			p1 = pos( "CMD(", str ) ;
			if ( p1 > 0 )
			{
				if ( PARM != "" )
				{
					err.seterrid( "PSYE039M", "CMD" ) ;
					return false ;
				}
				ob     = 1 ;
				oquote = false ;
				for ( p2 = p1+3 ; p2 < SELSTR.size() ; p2++ )
				{
					if ( SELSTR.at( p2 ) == '"' ) { oquote = !oquote ; }
					if ( oquote ) { continue ; }
					if ( SELSTR.at( p2 ) == '(' ) { ob++  ; }
					if ( SELSTR.at( p2 ) == ')' )
					{
						ob-- ;
						if ( ob == 0 ) { break ; }
					}
				}
				if ( ob != 0 || oquote )
				{
					err.seterrid( "PSYE033F" ) ;
					return false ;
				}
				p2++ ;
				PARM   = strip( substr( SELSTR, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
				PARM   = strip( PARM, 'B', '"' ) ;
				SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
				str    = upper( SELSTR ) ;
				lang   = "" ;
				p1     = pos( "LANG(", str ) ;
				if ( p1 > 0 )
				{
					p2 = pos( ")", SELSTR, p1 ) ;
					if ( p2 == 0 )
					{
						err.seterrid( "PSYE032D" ) ;
						return false ;
					}
					lang   = strip( substr( str, (p1 + 5), (p2 - (p1 + 5)) ) ) ;
					SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
					str    = upper( SELSTR ) ;
				}
				if ( lang == "" || lang == "REXX" )
				{
					PGM = "&ZOREXPGM"  ;
				}
				else
				{
					err.seterrid( "PSYE039N", lang ) ;
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
			err.seterrid( "PSYE032D" ) ;
			return false ;
		}
		SCRNAME = strip( substr( str, (p1 + 8), (p2 - (p1 + 8)) ) ) ;
		str     = delstr( str, p1, (p2 - p1 + 1) ) ;
		if ( findword( SCRNAME, "LIST NEXT PREV" ) )
		{
			err.seterrid( "PSYE039O" ) ;
			return false ;
		}
		if ( !isvalidName( SCRNAME ) )
		{
			err.seterrid( "PSYE039P", SCRNAME ) ;
			return false ;
		}
	}

	p1 = pos( "NEWAPPL(", str ) ;
	if ( p1 > 0 )
	{
		p2 = pos( ")", str, p1 ) ;
		if ( p2 == 0 )
		{
			err.seterrid( "PSYE032D" ) ;
			return false ;
		}
		NEWAPPL = strip( substr( str, (p1 + 8), (p2 - (p1 + 8)) ) ) ;
		NEWPOOL = true ;
		str     = delstr( str, p1, (p2 - p1 + 1) ) ;
		if ( !isvalidName4( NEWAPPL ) )
		{
			err.seterrid( "PSYE031E", "NEWAPPL", NEWAPPL ) ;
			return false ;
		}
	}
	else
	{
		p1 = wordpos( "NEWAPPL", str ) ;
		if ( p1 > 0 )
		{
			NEWAPPL = "ISP";
			NEWPOOL = true ;
			idelword( str, p1, 1 ) ;
		}
	}

	p1 = wordpos( "NEWPOOL", str ) ;
	if ( p1 > 0 )
	{
		NEWPOOL = true ;
		idelword( str, p1, 1 ) ;
	}

	p1 = wordpos( "SUSPEND", str ) ;
	if ( p1 > 0 )
	{
		SUSPEND = true ;
		idelword( str, p1, 1 ) ;
	}

	p1 = wordpos( "PASSLIB", str ) ;
	if ( p1 > 0 )
	{
		if ( NEWAPPL == "" )
		{
			err.seterrid( "PSYE039Q" ) ;
			return false ;
		}
		PASSLIB = true ;
		idelword( str, p1, 1 ) ;
	}

	if ( PGM == "" )
	{
		err.seterrid( "PSYE039R" ) ;
		return false ;
	}

	if ( trim( str ) != "" )
	{
		err.seterrid( "PSYE032H", str ) ;
		return false ;
	}

	return true ;
}


logger::logger()
{
	logfl   = ""    ;
	currfl  = NULL  ;
	logOpen = false ;

	boost::filesystem::path temp = \
	boost::filesystem::temp_directory_path() / boost::filesystem::unique_path( "lspf-%%%%-%%%%" ) ;
	tmpfl  = temp.native() ;
}


logger::~logger()
{
	if ( logOpen ) { close() ; }
}


bool logger::open( const string& dest, bool append )
{
	// Open log file for output.  Write to temporary file if destination not set

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

	if ( of.is_open() ) { logOpen = true ; }
	return logOpen ;
}


void logger::close()
{
	lock() ;
	of.close() ;
	logOpen = false ;
	unlock()   ;
}


bool logger::set( const string& dest )
{
	// Move log records to a new destination and continue recording.  Remove the old file.

	string* t ;

	bool res = false ;

	boost::system::error_code ec ;

	if ( dest == *currfl ) { return true ; }

	lock() ;
	of.close() ;
	logOpen = false ;

	if ( currfl && !exists( *currfl ) )
	{
		res = open( dest, true ) ;
	}
	else if ( !exists( dest ) || is_regular_file( dest ) )
	{
		copy_file( *currfl, dest, copy_option::overwrite_if_exists, ec ) ;
		if ( ec.value() == boost::system::errc::success )
		{
			t   = currfl ;
			res = open( dest, true ) ;
			remove( *t ) ;
		}
	}

	unlock()   ;
	return res ;
}
