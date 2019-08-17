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

void parser::parse( errblock& err, const string& s )
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
	if ( i < tokens.size() )
	{
		return tokens[ i ] ;
	}
	return token( TT_EOT ) ;
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

	return tokens.size() > i ? tokens[ i ].value1 : "" ;
}


token& parser::getCurrentToken()
{
	return current_token ;
}


string& parser::getCurrentValue()
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
	// Return the panel language statement type, ST_EOF if empty or ST_ERROR if not recognised.

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



void parser::getNextString( errblock& err, string::const_iterator& it, const string& s, string& r, bool& quoted )
{
	// Get the next word in string s and place in r.
	// Words can be delimited by space or special chars "()=,<>!+|;:" (also returned as words).

	int i ;
	int j ;
	int quotes ;

	const string delims = " (),=<>!|;:" ;
	const string compar = "=<>!" ;

	string::const_iterator its ;

	err.setRC( 0 ) ;

	quoted = false ;
	r      = ""    ;

	while ( it != s.end() && (*it) == ' ' ) { ++it ; }
	if ( it == s.end() )
	{
		return ;
	}

	if ( compar.find( (*it) ) != string::npos )
	{
		its = it ;
		++it ;
		while ( it != s.end() && ( compar.find( (*it) ) != string::npos ) ) { ++it ; }
		r.assign( its, it ) ;
	}
	else if ( delims.find( (*it) ) != string::npos )
	{
		r = (*it) ;
		++it ;
	}
	else if ( (*it) == '\'' )
	{
		++it ;
		its = it ;
		while ( it != s.end() )
		{
			quotes = 0 ;
			while ( it != s.end() && (*it) == '\'' ) { ++quotes ; ++it ; }
			if ( quotes == 0 && it != s.end() )
			{
				r.push_back( (*it) ) ;
				++it ;
				continue ;
			}
			i = quotes / 2 ;
			j = quotes % 2 ;
			r = r + string( i, '\'' ) ;
			if ( j == 1 )
			{
				if ( it != s.end() && delims.find( (*it) ) == string::npos )
				{
					err.seterrid( "PSYE037G" ) ;
					return ;
				}
				break ;
			}
			else if ( j == 0 && it == s.end() )
			{
				err.seterrid( "PSYE033F" ) ;
				return ;
			}
		}
		quoted = true ;
	}
	else
	{
		its = it ;
		while ( it != s.end() && ( delims.find( (*it) ) == string::npos ) ) { ++it ; }
		r.assign( its, it ) ;
	}
}


void parser::getNameList( errblock& err, string& r )
{
	// Return a list of valid variable names (subtype TS_NAME or TS_AMPR_VAR_VALID)
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
				r += " " + current_token.value1 ;
				getNextToken() ;
			}
			else if ( current_token.type == TT_EOT )
			{
				err.seterrid( "PSYE031V", current_token.value1 ) ;
				return ;
			}
			else
			{
				err.seterrid( "PSYE031D", current_token.value1 ) ;
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
		err.seterrid( "PSYE031D", current_token.value1 ) ;
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

	if ( if_lhs.value == ""   &&
	     if_trunc  == NULL &&
	     if_trans  == NULL &&
	     if_verify == NULL )
	{
		err.seterrid( "PSYE031G" ) ;
		return ;
	}

	t = v.getCurrentToken()  ;
	if ( t.type == TT_EOT )
	{
		err.seterrid( "PSYE031V" ) ;
		return ;
	}
	else if ( t.subtype != TS_CLOSE_BRACKET )
	{
		err.seterrid( "PSYE032Z", t.value1 ) ;
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
	// .CURSOR=ZCMD
	// VER (&A...) | &A = 'CAN'
	// TRUNC(...) = 'VAL1','VAL2' AND VER(...)
	// LENGTH(ABC) = 'VAL1','VAL2' AND VER(...)
	// fn(ABC) = 'VAL1','VAL2' AND VER(...)
	// EXISTS(FILENAME)

	// If comparison is optional ( DIR(), FILE(), EXISTS() ), default to 'EQ .TRUE' )

	bool comp_opt = false ;

	token t ;
	vparm p ;

	t = v.getCurrentToken() ;
	if ( t.subtype == TS_CLOSE_BRACKET )
	{
		err.seterrid( "PSYE031J" ) ;
		return ;
	}
	else if ( t.type == TT_EOT )
	{
		err.seterrid( "PSYE031K", "Missing variable" ) ;
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
				err.seterrid( "PSYE033A", "VERIFY" ) ;
				return ;
			}
			parse_cond_continue( err, v ) ;
			return ;
		}
		auto it = if_functions.find( t.value1 ) ;
		if ( it == if_functions.end() )
		{
			err.seterrid( "PSYE033U", t.value1 ) ;
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
				err.seterrid( "PSYE033A", "TRANS" ) ;
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
				err.seterrid( "PSYE031G" ) ;
				return ;
			}
			else if ( t.subtype == TS_CTL_VAR_INVALID )
			{
				err.seterrid( "PSYE033G", t.value1 ) ;
				return ;
			}
			else
			{
				err.seterrid( "PSYE031Q" ) ;
				return ;
			}
			if_lhs.value = t.value1 ;
			v.getNextToken() ;
			if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
			{
				err.seterrid( "PSYE031V" ) ;
				return ;
			}
		}
		t = v.getCurrentToken() ;
	}
	else
	{
		if ( t.value1 == "" )
		{
			err.seterrid( "PSYE033B" ) ;
			return ;
		}
		else if ( t.type == TT_VARIABLE )
		{
			if_lhs.subtype = t.subtype ;
			if_lhs.value   = ( t.subtype == TS_AMPR_VAR_VALID ) ? t.value1.substr( 1 ) : t.value1 ;
		}
		else if ( t.subtype == TS_CTL_VAR_INVALID )
		{
			err.seterrid( "PSYE033G", t.value1 ) ;
			return ;
		}
		else if ( t.subtype == TS_AMPR_VAR_INVALID )
		{
			err.seterrid( "PSYE031D", t.value1 ) ;
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
		err.seterrid( "PSYE031K", "Missing comparison operator" ) ;
		return ;
	}

	p.clear() ;
	auto it = if_conds.find( t.value1 ) ;
	if ( it == if_conds.end() )
	{
		if ( comp_opt )
		{
			if_cond = IF_EQ ;
			p.value = "1" ;
			if_rhs.push_back( p ) ;
			parse_cond_continue( err, v ) ;
		}
		else
		{
			err.seterrid( "PSYE033E", t.value1 ) ;
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
		err.seterrid( "PSYE033B" ) ;
		return ;
	}
	else if ( t.subtype == TS_CTL_VAR_INVALID )
	{
		err.seterrid( "PSYE033G", t.value1 ) ;
		return ;
	}
	else if ( t.subtype == TS_AMPR_VAR_INVALID )
	{
		err.seterrid( "PSYE031D", t.value1 ) ;
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
			err.seterrid( "PSYE031V" ) ;
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
			err.seterrid( "PSYE033G", t.value1 ) ;
			return ;
		}
		else if ( t.subtype == TS_AMPR_VAR_INVALID )
		{
			err.seterrid( "PSYE031D", t.value1 ) ;
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
		err.seterrid( "PSYE031K", "Missing variable" ) ;
		return ;
	}

	if ( ( if_cond != IF_EQ && if_cond != IF_NE ) && if_rhs.size() > 1 )
	{
		err.seterrid( "PSYE033H" ) ;
		return ;
	}

	parse_cond_continue( err, v ) ;
}


void IFSTMNT::parse_cond_continue( errblock& err, parser& v )
{
	// Check if the if_condition is continued with a boolean operator

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
		     if_next->if_trunc  == NULL &&
		     if_next->if_trans  == NULL &&
		     if_next->if_verify == NULL )
		{
			err.seterrid( "PSYE031G" ) ;
			return ;
		}
	}
}


void ASSGN::parse( errblock& err, parser& v )
{
	// Format of the assignment panel statement

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
	// .ATTR(ZCMD)    = 'xxxx'
	// .ATTR(.CURSOR) = 'xxxx'
	// .ATTR(&VAR)    = 'xxxx'
	// .ATTRCHAR(+)   = 'xxxx'
	// .ATTRCHAR(92)  = 'xxxx'
	// .ATTRCHAR(&VAR)= 'xxxx'

	token t ;

	map<string, PN_FUNCTION>::iterator it ;

	err.setRC( 0 ) ;

	t = v.getFirstToken() ;
	if ( t.type == TT_STRING_QUOTED )
	{
		err.seterrid( "PSYE031H" ) ;
		return ;
	}

	if ( findword( t.value1, ".FALSE .PFKEY .TRUE .TRAIL" ) )
	{
		err.seterrid( "PSYE033S", t.value1 ) ;
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
			err.seterrid( "PSYE033D" ) ;
			return ;
		}
		if ( v.isCurrentSubType( TS_CLOSE_BRACKET ) )
		{
			err.seterrid( "PSYE031G" ) ;
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
			err.seterrid( "PSYE031O", as_lhs.value ) ;
			return ;
		}
		if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
		{
			err.seterrid( "PSYE031V" ) ;
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
			err.seterrid( "PSYE033D" ) ;
			return ;
		}
		if ( v.isCurrentSubType( TS_CLOSE_BRACKET ) )
		{
			err.seterrid( "PSYE031G" ) ;
			return ;
		}
		t = v.getCurrentToken() ;
		if ( t.subtype != TS_NAME  &&
		     t.subtype != TS_AMPR_VAR_VALID )
		{
			err.seterrid( "PSYE031P", as_lhs.value ) ;
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
				err.seterrid( "PSYE041B" ) ;
				return ;
			}
			if ( as_lhs.value.size() == 2 ) { as_lhs.value = xs2cs( as_lhs.value ) ; }
		}
		v.getNextToken() ;
		if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
		{
			err.seterrid( "PSYE031V" ) ;
			return ;
		}
		as_isattc = true ;
	}
	else if ( t.subtype == TS_CTL_VAR_INVALID )
	{
		err.seterrid( "PSYE033G", t.value1 ) ;
		return ;
	}
	else if ( t.subtype == TS_AMPR_VAR_INVALID )
	{
		err.seterrid( "PSYE031D", t.value1 ) ;
		return ;
	}
	else
	{
		err.seterrid( "PSYE033Q", t.value1 ) ;
		return ;
	}

	if ( !v.getNextIfCurrent( TS_EQUALS ) )
	{
		err.seterrid( "PSYE033O" ) ;
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
		err.seterrid( "PSYE033G", t.value1 ) ;
		return ;
	}
	else if ( t.subtype == TS_AMPR_VAR_INVALID )
	{
		err.seterrid( "PSYE031D", t.value1 ) ;
		return ;
	}
	else if ( v.peekNextValue() == "(" )
	{
		it = assign_functions.find( t.value1 ) ;
		if ( it == assign_functions.end() )
		{
			err.seterrid( "PSYE033U", t.value1 ) ;
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

		default:
			v.getNextToken() ;
			t = v.getNextToken() ;
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
				err.seterrid( "PSYE031G" ) ;
				return ;
			}
			else if ( t.subtype == TS_CTL_VAR_INVALID )
			{
				err.seterrid( "PSYE033G", t.value1 ) ;
				return ;
			}
			else
			{
				err.seterrid( "PSYE031Q" ) ;
				return ;
			}
			as_rhs.value = t.value1 ;
			t = v.getNextToken() ;
			if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
			{
				err.seterrid( "PSYE031V" ) ;
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
		err.seterrid( "PSYE033Z", t.value1 ) ;
		return ;
	}

	t = v.getNextToken() ;
	if ( t.type != TT_EOT )
	{
		err.seterrid( "PSYE032H", t.value1 ) ;
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
	ver_var = t.value1.substr( 1 ) ;

	if ( !v.getNextIfCurrent( TS_AMPR_VAR_VALID ) )
	{
		err.seterrid( "PSYE033I", t.value1 ) ;
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
				err.seterrid( "PSYE032H", v.getCurrentValue() ) ;
				return ;
			}
			return ;
		}
		v.getNextIfCurrent( TS_COMMA ) ;
		t = v.getCurrentToken() ;
	}

	if      ( t.value1 == "NUM"   ) { ver_type = VER_NUMERIC ; }
	else if ( t.value1 == "LIST"  ) { ver_type = VER_LIST    ; }
	else if ( t.value1 == "LISTX" ) { ver_type = VER_LISTX ; ver_nblank = true ; }
	else if ( t.value1 == "PICT"  ) { ver_type = VER_PICT ; }
	else if ( t.value1 == "HEX"   ) { ver_type = VER_HEX  ; }
	else if ( t.value1 == "NAME"  ) { ver_type = VER_NAME ; }
	else if ( t.value1 == "OCT"   ) { ver_type = VER_OCT  ; }
	else if ( t.value1 == "MSG"   ) {                     ; }
	else
	{
		err.seterrid( "PSYE033L" ) ;
		return ;
	}

	if ( t.value1 != "MSG" )
	{
		v.getNextToken()  ;
		v.getNextIfCurrent( TS_COMMA ) ;
		t = v.getCurrentToken() ;
	}

	if ( ver_type == VER_PICT )
	{
		ver_vlist.push_back( t.value1 ) ;
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
			err.seterrid( "PSYE031V" ) ;
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
		else if ( t.value1 == "MSG" )
		{
			break ;
		}
		else if ( t.subtype == TS_AMPR_VAR_INVALID )
		{
			err.seterrid( "PSYE031D", t.value1 ) ;
			return ;
		}
		else
		{
			ver_vlist.push_back( t.value1 ) ;
		}
		t = v.getNextToken() ;
	}

	if ( t.value1 == "MSG" )
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
			ver_msgid = t.value1 ;
		}
		else
		{
			err.seterrid( "PSYE031I", t.value1 ) ;
			return ;
		}
	}

	if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
	{
		err.seterrid( "PSYE031V" ) ;
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
		if ( t.value1 == ".TRAIL" )
		{
			err.seterrid( "PSYE038G" ) ;
			return ;
		}
		trnc_field.subtype = t.subtype ;
		trnc_field.value = ( t.subtype == TS_CTL_VAR_VALID ) ? t.value1 : t.value1.substr( 1 ) ;
	}
	else if ( t.subtype == TS_CTL_VAR_INVALID )
	{
		err.seterrid( "PSYE033G", t.value1 ) ;
		return ;
	}
	else if ( t.subtype == TS_AMPR_VAR_INVALID )
	{
		err.seterrid( "PSYE031D", t.value1 ) ;
		return ;
	}
	else
	{
		err.seterrid( ( t.value1 == "" ) ? "PSYE038A" : "PSYE031Q" ) ;
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
	   ( t.type == TT_STRING_UNQUOTED && !datatype( t.value1, 'W' ) ) )
	{
		if ( t.value1.size() != 1 )
		{
			err.seterrid( "PSYE038F", t.value1 ) ;
			return ;
		}
		trnc_char = t.value1.front() ;
	}
	else
	{
		if ( !datatype( t.value1, 'W' ) )
		{
			err.seterrid( "PSYE019E", t.value1 ) ;
			return ;
		}
		trnc_len = ds2d( t.value1 ) ;
		if ( trnc_len <= 0 )
		{
			err.seterrid( "PSYE019F" ) ;
			return ;
		}
	}

	v.getNextToken() ;
	if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
	{
		err.seterrid( "PSYE031V" ) ;
		return ;
	}

	if ( check && !v.isCurrentType( TT_EOT ) )
	{
		err.seterrid( "PSYE032H", v.getCurrentValue() ) ;
		return ;
	}
	return ;
}


void TRANS::parse( errblock& err, parser& v, bool check )
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
		err.seterrid( "PSYE033G", t.value1 ) ;
		return ;
	}
	else if ( t.subtype == TS_AMPR_VAR_INVALID )
	{
		err.seterrid( "PSYE031D", t.value1 ) ;
		return ;
	}
	else
	{
		err.seterrid( ( t.value1 == "" ) ? "PSYE038A" : "PSYE031Q" ) ;
		return ;
	}

	first = true ;
	while ( true )
	{
		t = v.getCurrentToken() ;
		if ( t.type == TT_EOT )
		{
			err.seterrid( "PSYE031V" ) ;
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
				err.seterrid( "PSYE031D", t.value1 ) ;
				return ;
			}
			if ( !v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
			{
				err.seterrid( "PSYE031V" ) ;
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
				trns_list.push_back( make_pair( v1, t.value1 ) ) ;
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
		err.seterrid( "PSYE031R", "TRANS", v.getCurrentValue() ) ;
		return ;
	}

	if ( check && !v.isCurrentType( TT_EOT ) )
	{
		err.seterrid( "PSYE032H", v.getCurrentValue() ) ;
		return ;
	}
}


void pnts::parse( errblock& err, string s )
{
	// Format of the PNTS panel entry (point-and-shoot entries)

	// FIELD(fld) VAR(var) VAL(value)
	// fld and var must be defined in the panel

	iupper( s ) ;

	err.setRC( 0 ) ;

	pnts_field = parseString( err, s, "FIELD()" ) ;
	if ( err.error() ) { return ; }
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
	if ( err.error() ) { return ; }
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

	pnts_val = extractKWord( err, s, "VAL()" ) ;
	if ( err.error() ) { return ; }
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

	if ( s != "" )
	{
		err.seterrid( "PSYE032H", s ) ;
		return ;
	}
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

	size_t p1 ;
	size_t p2 ;

	int ln ;

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
		if ( p1 == string::npos ) { return false ; }
		rest = substr( smsg, p1+2 ) ;
		smsg = smsg.substr( 0, p1+1 ) ;
		smsg = dquote( err, c, smsg ) ;
		if ( err.error() ) { return false ; }
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

	trim_right( smsg ) ;
	trim_right( lmsg ) ;

	return true ;
}


bool selobj::parse( errblock& err, string selstr )
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
	// single quotes if needed, that are then removed.

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
			err.seterrid( "PSYE031V" ) ;
			return false ;
		}
		if ( oquote )
		{
			err.seterrid( "PSYE033F" ) ;
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
			err.seterrid( "PSYE031V" ) ;
			return false ;
		}
		pgm    = strip( substr( str, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
		selstr = delstr( selstr, p1, (p2 - p1 + 1) ) ;
		str    = upper( selstr ) ;
		if ( !pgm.empty() && pgm.front() == '&' )
		{
			if ( !isvalidName( substr( pgm, 2 ) ) )
			{
				err.seterrid( "PSYE031D", substr( pgm, 2 ) ) ;
				return false ;
			}
		}
		else
		{
			if ( !isvalidName( pgm ) )
			{
				err.seterrid( "PSYE031E", "PROGRAM", pgm ) ;
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
				err.seterrid( "PSYE039M", "PANEL" ) ;
				return false ;
			}
			p2 = pos( ")", selstr, p1 ) ;
			if ( p2 == 0 )
			{
				err.seterrid( "PSYE031V" ) ;
				return false ;
			}
			parm = strip( substr( str, (p1 + 6), (p2 - (p1 + 6)) ) ) ;
			if ( !isvalidName( parm ) )
			{
				err.seterrid( "PSYE031E", "PANEL", parm ) ;
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
					err.seterrid( "PSYE031V" ) ;
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
					err.seterrid( "PSYE039M", "CMD" ) ;
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
					err.seterrid( "PSYE031V" ) ;
					return false ;
				}
				if ( oquote )
				{
					err.seterrid( "PSYE033F" ) ;
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
						err.seterrid( "PSYE031V" ) ;
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
			err.seterrid( "PSYE031V" ) ;
			return false ;
		}
		scrname = strip( substr( str, (p1 + 8), (p2 - (p1 + 8)) ) ) ;
		str     = delstr( str, p1, (p2 - p1 + 1) ) ;
		if ( findword( scrname, "LIST NEXT PREV" ) )
		{
			err.seterrid( "PSYE039O" ) ;
			return false ;
		}
		if ( !isvalidName( scrname ) )
		{
			err.seterrid( "PSYE039P", scrname ) ;
			return false ;
		}
	}

	p1 = pos( "NEWAPPL(", str ) ;
	if ( p1 > 0 )
	{
		p2 = pos( ")", str, p1 ) ;
		if ( p2 == 0 )
		{
			err.seterrid( "PSYE031V" ) ;
			return false ;
		}
		newappl = strip( substr( str, (p1 + 8), (p2 - (p1 + 8)) ) ) ;
		newpool = true ;
		str     = delstr( str, p1, (p2 - p1 + 1) ) ;
		if ( !isvalidName4( newappl ) )
		{
			err.seterrid( "PSYE031E", "NEWAPPL", newappl ) ;
			return false ;
		}
		if ( newappl == "ISPS" )
		{
			err.seterrid( "PSYE039S" ) ;
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
			err.seterrid( "PSYE039Q" ) ;
			return false ;
		}
		passlib = true ;
		idelword( str, p1, 1 ) ;
	}

	if ( pgm == "" && pgmtype == PGM_NONE )
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


void char_attrs::setattr( errblock& err, string& attrs )
{
	// Create new values using attrs.
	// Keep a copy of the original string if it contains dialogue variables.

	err.setRC( 0 ) ;
	char_attrs_clear() ;

	dvars = ( attrs.find( '&' ) != string::npos ) ;
	if ( dvars ) { entry = attrs ; }

	parse( err, attrs ) ;
}


void char_attrs::update( errblock& err, string& attrs )
{
	// Update existing values with what is in attrs.
	// Used for attrchar() and variable substitution after )INIT processing

	err.setRC( 0 ) ;

	parse( err, attrs ) ;
}


const string& char_attrs::get()
{
	// Return the entry (only set if it contains substitutable variables).

	return entry ;
}


void char_attrs::parse( errblock& err, string& attrs )
{
	// Parse the attribute type statement
	// Currently supported parameters:
	// 1) TYPE
	// 2) COLOUR
	// 3) INTENS
	// 4) HILITE
	// 5) CUADYN - syntax checked but only used on types DATAIN/DATAOUT

	// CUA types cannot have COLOUR, INTENS or HILITE parameters coded
	// CUADYN for DATAIN/DATAOUT. Ignored for everything else.

	string temp ;

	iupper( attrs ) ;

	temp = parseString( err, attrs, "TYPE()" ) ;
	if ( err.error() ) { return ; }

	if ( temp != "" )
	{
		auto it = type_map.find( temp ) ;
		if ( it == type_map.end() )
		{
			err.seterrid( "PSYE035B", temp, "TYPE" ) ;
			return ;
		}
		if ( none_cua_map.count( temp ) == 0 )
		{
			if ( attrs != "" )
			{
				err.seterrid( "PSYE035D", attrs ) ;
				return ;
			}
			typecua = true ;
		}
		type       = it->second ;
		typeChange = true ;
	}

	temp = parseString( err, attrs, "COLOUR()" ) ;
	if ( err.error() ) { return ; }
	if ( temp == "" )
	{
		temp = parseString( err, attrs, "COLOR()" ) ;
		if ( err.error() ) { return ; }
	}


	if ( temp != "" )
	{
		if ( temp.front() == '&' )
		{
			if ( not isvalidName( temp.substr( 1 ) ) )
			{
				err.seterrid( "PSYE031D", temp.substr( 1 ) ) ;
				return ;
			}
		}
		else
		{
			auto it = colour_map.find( temp ) ;
			if ( it == colour_map.end() )
			{
				err.seterrid( "PSYE035B", temp, "COLOUR" ) ;
				return ;
			}
			colour = it->second ;
		}
		colourChange = true ;
	}

	temp = parseString( err, attrs, "INTENS()" ) ;
	if ( err.error() ) { return ; }

	if ( temp != "" )
	{
		if ( temp.front() == '&' )
		{
			if ( not isvalidName( temp.substr( 1 ) ) )
			{
				err.seterrid( "PSYE031D", temp.substr( 1 ) ) ;
				return ;
			}
		}
		else
		{
			auto it = intens_map.find( temp ) ;
			if ( it == intens_map.end() )
			{
				err.seterrid( "PSYE035B", temp, "INTENS" ) ;
				return ;
			}
			intens = it->second ;
		}
		intensChange = true ;
	}

	temp = parseString( err, attrs, "HILITE()" ) ;
	if ( err.error() ) { return ; }
	if ( err.RSN0() )
	{
		if ( temp == "" )
		{
			hilite = 0 ;
		}
		else if ( temp.front() == '&' )
		{
			if ( not isvalidName( temp.substr( 1 ) ) )
			{
				err.seterrid( "PSYE031D", temp.substr( 1 ) ) ;
				return ;
			}
		}
		else
		{
			auto it = hilite_map.find( temp ) ;
			if ( it == hilite_map.end() )
			{
				err.seterrid( "PSYE035B", temp, "HILITE" ) ;
				return ;
			}
			hilite = it->second ;
		}
		hiliteChange = true ;
	}

	temp = parseString( err, attrs, "CUADYN()" ) ;
	if ( err.error() ) { return ; }

	if ( temp != "" )
	{
		if ( temp.front() == '&' )
		{
			if ( not isvalidName( temp.substr( 1 ) ) )
			{
				err.seterrid( "PSYE031D", temp.substr( 1 ) ) ;
				return ;
			}
		}
		else
		{
			auto it1 = type_map.find( temp ) ;
			auto it2 = none_cua_map.find( temp ) ;
			if ( it1 == type_map.end() || it2 != none_cua_map.end() )
			{
				err.seterrid( "PSYE035B", temp, "HILITE" ) ;
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

	if ( trim( attrs ) != "" )
	{
		err.seterrid( "PSYE032H", attrs ) ;
		return ;
	}
}


uint char_attrs::get_colour()
{
	return ( cuadyn == NONE ) ? ( colour | intens | hilite ) : lspfc::cuaAttr[ cuadyn ] ;
}


attType char_attrs::get_type()
{
	return type ;
}


logger::logger()
{
	logfl   = ""    ;
	currfl  = NULL  ;
	logOpen = false ;

	boost::filesystem::path temp = \
	boost::filesystem::temp_directory_path() / boost::filesystem::unique_path( "lspf-%%%%-%%%%" ) ;
	tmpfl = temp.native() ;
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
	boost::lock_guard<boost::mutex> lock( mtx ) ;

	of.close() ;
	logOpen = false ;
}


bool logger::set( const string& dest )
{
	// Move log records to a new destination and continue recording.
	// Overwrite the new file and remove the old file.

	string* t ;

	bool res = false ;

	boost::system::error_code ec ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	if ( dest == *currfl ) { return true ; }

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

	return res ;
}
