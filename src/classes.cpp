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

void IFSTMNT::parse( errblock& err, string s )
{
	// Format of the IF panel statement

	// IF ( &AAA=&BBBB )
	// IF ( &AAA = VALUE1,VALUE2 )
	// IF ( &AAA NE 'Hello','Goodbye' )
	// IF (.CURSOR = ZCMD)
	// IF (.MSG    = PSYS011)
	// IF (.RESP   = ENTER)
	// IF (.RESP   = END)
	// IF ( &Z EQ .TRUE )
	// IF ( &Z EQ .FALSE ) .TRUE = "1" and .FALSE = "0"
	// rhs value lists only for EQ and NE (EQ only one needs to be true, NE all need to be true)
	// IF ( VER (&A...) )

	int p1 ;
	int p2 ;

	bool f_end ;

	string t    ;
	string comp ;

	p1 = s.find( '(' ) ;
	if ( p1 == string::npos )
	{
		err.seterrid( "PSYE033D" ) ;
		return ;
	}
	trim( s.erase( 0, p1+1 ) ) ;

	p1 = s.find( '(' ) ;
	if ( p1 != string::npos )
	{
		t = strip( s.substr( 0, p1 ) ) ;
		if ( t == "VER" )
		{
			if ( s.back() != ')' )
			{
				err.seterrid( "PSYE032D" ) ;
				return ;
			}
			s.pop_back() ;
			if_verify.parse( err, s ) ;
			if ( err.error() )
			{
				err.seterrid( "PSYE033B" ) ;
				return ;
			}
			if ( if_verify.ver_msgid != "" )
			{
				err.seterrid( "PSYE033A" ) ;
				return ;
			}
			if_ver = true ;
			return ;
		}
	}

	p1 = s.find_first_of( "=><!" ) ;
	if ( p1 == string::npos )
	{
		p2 = s.find( ' ' ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( "PSYE033B" ) ;
			return ;
		}
		if_lhs = upper( strip( s.substr( 0, p2 ) ) ) ;
		p1 = s.find_first_not_of( ' ', p2 ) ;
		if ( p1 == string::npos )
		{
			err.seterrid( "PSYE033B" ) ;
			return ;
		}
		p2 = s.find( ' ', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( "PSYE033B" ) ;
			return ;
		}
	}
	else
	{
		p2 = s.find_first_not_of( "=><!", p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( "PSYE033B" ) ;
			return ;
		}
		if_lhs = upper( strip( s.substr( 0, p1 ) ) ) ;
	}

	comp = s.substr( p1, p2-p1 ) ;
	trim( s.erase( 0, p2 ) ) ;

	if ( words( if_lhs ) != 1 )
	{
		err.seterrid( "PSYE033C" ) ;
		return ;
	}
	if      ( if_lhs == ".CURSOR" ) {}
	else if ( if_lhs == ".MSG"    ) {}
	else if ( if_lhs == ".RESP"   ) {}
	else if ( if_lhs.front() != '&' )
	{
		err.seterrid( "PSYE033I" ) ;
		return ;
	}
	else
	{
		if_lhs.erase( 0, 1 ) ;
		if ( !isvalidName( if_lhs ) )
		{
			err.seterrid( "PSYE031D", if_lhs ) ;
			return ;
		}
	}

	iupper( comp ) ;
	if      ( comp == "="  ) { if_eq = true ; }
	else if ( comp == "EQ" ) { if_eq = true ; }
	else if ( comp == "!=" ) { if_ne = true ; }
	else if ( comp == "NE" ) { if_ne = true ; }
	else if ( comp == ">"  ) { if_gt = true ; }
	else if ( comp == "GT" ) { if_gt = true ; }
	else if ( comp == "<"  ) { if_lt = true ; }
	else if ( comp == "LT" ) { if_lt = true ; }
	else if ( comp == ">=" ) { if_ge = true ; }
	else if ( comp == "=>" ) { if_ge = true ; }
	else if ( comp == "GE" ) { if_ge = true ; }
	else if ( comp == "<=" ) { if_le = true ; }
	else if ( comp == "=<" ) { if_le = true ; }
	else if ( comp == "LE" ) { if_le = true ; }
	else if ( comp == "!>" ) { if_ng = true ; }
	else if ( comp == "NG" ) { if_ng = true ; }
	else if ( comp == "!<" ) { if_nl = true ; }
	else if ( comp == "NL" ) { if_nl = true ; }
	else
	{
		err.seterrid( "PSYE033E", comp ) ;
		return ;
	}

	f_end = false ;

	while ( true )
	{
		if ( s.front() == '&' )
		{
			p1 = s.find( ',' ) ;
			if ( p1 == string::npos )
			{
				p1 = s.find( ')' ) ;
				if ( p1 == string::npos )
				{
					err.seterrid( "PSYE032D" ) ;
					return ;
				}
				f_end = true ;
			}
			t = upper( strip( s.substr( 0, p1 ) ) ) ;
			t.erase( 0, 1 ) ;
			if ( !isvalidName( t ) )
			{
				err.seterrid( "PSYE031D", t ) ;
				return ;
			}
			if_isvar.push_back( true ) ;
		}
		else if ( s.front()  == '\'' )
		{
			s.erase( 0, 1 ) ;
			p1 = s.find( '\'' ) ;
			if ( p1 == string::npos )
			{
				err.seterrid( "PSYE033F" ) ;
				return ;
			}
			t  = s.substr( 0, p1 ) ;
			trim( s.erase( 0, p1+1 ) ) ;
			p1 = s.find( ',' ) ;
			if ( p1 == string::npos )
			{
				p1 = s.find( ')' ) ;
				if ( p1 == string::npos )
				{
					err.seterrid( "PSYE032D" ) ;
					return ;
				}
				f_end = true ;
			}
			if_isvar.push_back( false ) ;
		}
		else if ( s.front() == '.' )
		{
			s.erase( 0, 1 ) ;
			p1 = s.find( ',' ) ;
			if ( p1 == string::npos )
			{
				p1 = s.find( ')' ) ;
				if ( p1 == string::npos )
				{
					err.seterrid( "PSYE032D" ) ;
					return ;
				}
				f_end = true ;
			}
			t = upper( strip( s.substr( 0, p1 ) ) ) ;
			if      ( t == "TRUE" )  { t = "1" ; }
			else if ( t == "FALSE" ) { t = "0" ; }
			else
			{
				err.seterrid( "PSYE033G", t ) ;
				return ;
			}
			if_isvar.push_back( false ) ;
		}
		else
		{
			p1 = s.find( ',' ) ;
			if ( p1 == string::npos )
			{
				p1 = s.find( ')' ) ;
				if ( p1 == string::npos )
				{
					err.seterrid( "PSYE032D" ) ;
					return ;
				}
				f_end = true ;
			}
			t = upper( strip( s.substr( 0, p1 ) ) ) ;
			if ( t == "" || (t.find_first_of( " )" ) != string::npos) )
			{
				err.seterrid( "PSYE033B" ) ;
				return ;
			}
			if_isvar.push_back( false ) ;
		}
		if_rhs.push_back( t ) ;
		trim( s.erase( 0, p1+1 ) ) ;
		if ( s == "" && !f_end )
		{
			err.seterrid( "PSYE033B" ) ;
			return ;
		}
		if ( f_end ) { break ; }
	}
	if ( s != "" )
	{
		err.seterrid( "PSYE032H", s ) ;
		return ;
	}
	if ( ( !if_eq && !if_ne ) && if_rhs.size() > 1 )
	{
		err.seterrid( "PSYE033H" ) ;
		return ;
	}
	return ;
}


void ASSGN::parse( errblock& err, string s )
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

	int p  ;
	int p1 ;

	const string lhs_control = ".ALARM .AUTOSEL .BROWSE .CURSOR .CSRROW .CSRPOS .EDIT .HELP .MSG .NRET .RESP" ;
	const string rhs_control = ".ALARM .CSRPOS .CURSOR .HELP .MSG .TRAIL .RESP" ;

	p = s.find( '=' ) ;
	if ( p == string::npos )
	{
		err.seterrid( "PSYE033O" ) ;
		return ;
	}
	as_lhs = upper( strip( s.substr( 0, p ) ) ) ;
	if ( words( as_lhs ) != 1 )
	{
		err.seterrid( "PSYE033P" ) ;
		return ;
	}
	if ( findword( as_lhs, lhs_control ) ) {}
	else if ( as_lhs.substr( 0, 6 ) == ".ATTR(" )
	{
		p1 = as_lhs.find( ')' ) ;
		if ( p1 == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		as_lhs = strip( as_lhs.substr( 6, p1-6 ) ) ;
		if ( !isvalidName( as_lhs ) )
		{
			err.seterrid( "PSYE031D", as_lhs ) ;
			return ;
		}
		as_isattr = true ;
	}
	else if ( as_lhs.front() == '&' )
	{
		as_lhs.erase( 0, 1 ) ;
		if ( !isvalidName( as_lhs ) )
		{
			err.seterrid( "PSYE031D", as_lhs ) ;
			return ;
		}
	}
	else
	{
		err.seterrid( "PSYE033Q", as_lhs ) ;
		return ;
	}
	trim( s.erase( 0, p+1 ) ) ;
	if ( s.front() == '&' )
	{
		if ( words( s ) != 1 )
		{
			err.seterrid( "PSYE033R" ) ;
			return ;
		}
		s.erase( 0, 1 ) ;
		s = upper( s )  ;
		if ( !isvalidName( s ) )
		{
			err.seterrid( "PSYE031D", s ) ;
			return ;
		}
		as_rhs   = s   ;
		as_isvar = true ;
	}
	else if ( s.front() == '\'' )
	{
		s.erase( 0, 1 ) ;
		p = s.find( '\'' ) ;
		if ( p == string::npos )
		{
			err.seterrid( "PSYE033F" ) ;
			return ;
		}
		as_rhs = s.substr( 0, p ) ;
		trim( s.erase( 0, p+1 ) ) ;
		if ( s != "" )
		{
			err.seterrid( "PSYE032H", s ) ;
			return ;
		}
	}
	else if ( upper( s.substr( 0, 4 ) ) == "DIR(" )
	{
		iupper( s )  ;
		trim( s.erase( 0, 4 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) )
		{
			err.seterrid( "PSYE031D", as_rhs ) ;
			return ;
		}
		trim( s.erase( 0, p+1 ) ) ;
		if ( s != "" )
		{
			err.seterrid( "PSYE032H", s ) ;
			return ;
		}
		as_isvar  = true ;
		as_chkdir = true ;
	}
	else if ( upper( s.substr( 0, 7 ) ) == "EXISTS(" )
	{
		iupper( s )  ;
		trim( s.erase( 0, 7 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) )
		{
			err.seterrid( "PSYE031D", as_rhs ) ;
			return ;
		}
		trim( s.erase( 0, p+1 ) ) ;
		if ( s != "" )
		{
			err.seterrid( "PSYE032H", s ) ;
			return ;
		}
		as_isvar   = true ;
		as_chkexst = true ;
	}
	else if ( upper( s.substr( 0, 5 ) ) == "FILE(" )
	{
		iupper( s )  ;
		trim( s.erase( 0, 5 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) )
		{
			err.seterrid( "PSYE031D", as_rhs ) ;
			return ;
		}
		trim( s.erase( 0, p+1 ) ) ;
		if ( s != "" )
		{
			err.seterrid( "PSYE032H", s ) ;
			return ;
		}
		as_isvar   = true ;
		as_chkfile = true ;
	}
	else if ( upper( s.substr( 0, 7 ) ) == "LENGTH(" )
	{
		iupper( s )  ;
		trim( s.erase( 0, 7 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) )
		{
			err.seterrid( "PSYE031D", as_rhs ) ;
			return ;
		}
		trim( s.erase( 0, p+1 ) ) ;
		if ( s != "" )
		{
			err.seterrid( "PSYE032H", s ) ;
			return ;
		}
		as_isvar  = true ;
		as_retlen = true ;
	}
	else if ( upper( s.substr( 0, 8 ) ) == "REVERSE(" )
	{
		iupper( s )  ;
		trim( s.erase( 0, 8 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) )
		{
			err.seterrid( "PSYE031D", as_rhs ) ;
			return ;
		}
		trim( s.erase( 0, p+1 ) ) ;
		if ( s != "" )
		{
			err.seterrid( "PSYE032H", s ) ;
			return ;
		}
		as_isvar   = true ;
		as_reverse = true ;
	}
	else if ( upper( s.substr( 0, 6 ) ) == "WORDS(" )
	{
		iupper( s )  ;
		trim( s.erase( 0, 6 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) )
		{
			err.seterrid( "PSYE031D", as_rhs ) ;
			return ;
		}
		trim( s.erase( 0, p+1 ) ) ;
		if ( s != "" )
		{
			err.seterrid( "PSYE032H", s ) ;
			return ;
		}
		as_isvar = true ;
		as_words = true ;
	}
	else if ( upper( s.substr( 0, 6 ) ) == "UPPER(" )
	{
		iupper( s )  ;
		trim( s.erase( 0, 6 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) )
		{
			err.seterrid( "PSYE031D", as_rhs ) ;
			return ;
		}
		trim( s.erase( 0, p+1 ) ) ;
		if ( s != "" )
		{
			err.seterrid( "PSYE032H", s ) ;
			return ;
		}
		as_isvar = true ;
		as_upper = true ;
	}
	else
	{
		if ( words( s ) != 1 )
		{
			err.seterrid( "PSYE032H", subword( s, 2 ) ) ;
			return ;
		}
		iupper( s ) ;
		if ( s.front() == '.' && !findword( s, rhs_control ) )
		{
			err.seterrid( "PSYE033S", s ) ;
			return ;
		}
		as_rhs = s ;
	}
	return ;
}


void VPUTGET::parse( errblock& err, string s )
{
	// VGET ABC
	// VGET(ABC) PROFILE

	int i  ;
	int j  ;
	int ws ;
	int p1 ;

	string w1 ;
	string t  ;

	iupper( s ) ;
	p1 = s.find( '(' ) ;
	if ( p1 == string::npos )
	{
		w1 = word( s, 1 ) ;
		s  = subword( s, 2 ) ;
	}
	else
	{
		w1 = strip( s.substr( 0, p1 ) ) ;
		trim( s.erase( 0, p1 ) ) ;
	}

	if ( s == "" )
	{
		err.seterrid( "PSYE033T" ) ;
		return ;
	}

	( w1 == "VPUT" ) ? vpg_vput = true : vpg_vget = true ;

	if ( s.front() == '(' )
	{
		i = s.find( ')', 1 ) ;
		if ( i == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		vpg_vars = s.substr( 1, i-1 ) ;
		replace( vpg_vars.begin(), vpg_vars.end(), ',', ' ' ) ;
		for( ws = words( vpg_vars ), j = 1 ; j <= ws ; j++ )
		{
			t = word( vpg_vars, j ) ;
			if ( !isvalidName( t ) )
			{
				err.seterrid( "PSYE031D", t ) ;
				return ;
			}
		}
		s.erase( 0, i+1 ) ;
	}
	else
	{
		vpg_vars = word( s, 1 ) ;
		if ( !isvalidName( vpg_vars ) )
		{
			err.seterrid( "PSYE031D", vpg_vars ) ;
			return ;
		}
		s = subword( s, 2 ) ;
	}

	trim( s ) ;
	if ( s == "ASIS" || s == "" ) { vpg_pool = ASIS    ; }
	else if ( s == "SHARED" )     { vpg_pool = SHARED  ; }
	else if ( s == "PROFILE" )    { vpg_pool = PROFILE ; }
	else
	{
		err.seterrid( "PSYE033U", s ) ;
		return ;
	}
	return ;
}


void VERIFY::parse( errblock& err, string s )
{
	// VER (&VAR LIST A B C D)
	// VER (&VAR,LIST,A,B,C,D)
	// VER (&VAR NB LIST A B C D)
	// VER(&VAR NONBLANK LIST A B C D)
	// VER(&VAR PICT ABCD)
	// VER(&VAR HEX)
	// VER(&VAR OCT)


	int i     ;
	int p1    ;
	int p2    ;
	int ws    ;
	string t  ;
	string w  ;

	p1 = s.find( '(' ) ;
	if ( p1 == string::npos )
	{
		err.seterrid( "PSYE033D" ) ;
		return ;
	}
	p2 = s.find( ')', p1 ) ;
	if ( p2 == string::npos )
	{
		err.seterrid( "PSYE032D" ) ;
		return ;
	}

	t = strip( s.substr( p2+1 ) ) ;
	if ( t != "" )
	{
		err.seterrid( "PSYE032H", t ) ;
		return ;
	}
	s = upper( strip( s.substr( p1+1, p2-p1-1 ) ) ) ;

	replace( s.begin(), s.end(), ',', ' ' ) ;
	w = word( s, 1 ) ;

	if ( w == "" )
	{
		err.seterrid( "PSYE033J" ) ;
		return ;
	}
	if ( w.front() != '&' )
	{
		err.seterrid( "PSYE033I" ) ;
		return ;
	}

	ver_var = w.substr( 1 ) ;
	if ( !isvalidName( ver_var ) )
	{
		err.seterrid( "PSYE031D", ver_var ) ;
		return ;
	}

	ws = words( s )    ;
	w  = word( s, ws ) ;
	if ( w.substr( 0, 4 ) == "MSG=" )
	{
		ver_msgid = w.substr( 4 ) ;
		if ( ver_msgid == "" )
		{
			err.seterrid( "PSYE033K" ) ;
			return ;
		}
		s = subword( s, 1, ws-1 ) ;
	}

	w = word( s, 2 ) ;
	if ( w == "NB" || w == "NONBLANK" ) { i = 3 ; ver_nblank = true ; }
	else                                { i = 2 ;                     }

	w = word( s, i ) ;
	if ( ver_nblank && w == "" )
	{
		return ;
	}

	if      ( w == "NUM"  ) { ver_numeric = true ; }
	else if ( w == "LIST" ) { ver_list    = true ; }
	else if ( w == "PICT" ) { ver_pict    = true ; }
	else if ( w == "HEX"  ) { ver_hex     = true ; }
	else if ( w == "OCT"  ) { ver_octal   = true ; }
	else
	{
		err.seterrid( "PSYE033L" ) ;
		return ;
	}

	while ( ver_list )
	{
		i++ ;
		w = word( s, i ) ;
		if ( w == ""  ) { break ; }
		if ( w == ")" )
		{
			err.seterrid( "PSYE033M" ) ;
			return ;
		}
		ver_value += " " + w ;
	}

	if ( ver_pict )
	{
		i++ ;
		w = word( s, i ) ;
		if ( w == "" || w == ")" )
		{
			err.seterrid( "PSYE033M" ) ;
			return ;
		}
		ver_value = w ;
	}

	if ( ( ver_list || ver_pict ) && ver_value == "" )
	{
		err.seterrid( "PSYE033N" ) ;
		return ;
	}

	i++ ;
	if ( word( s, i ) != "" )
	{
		err.seterrid( "PSYE032H", word( s, i ) ) ;
		return ;
	}
	return ;
}


void TRUNC::parse( errblock& err, string s )
{
	// Format of the TRUNC panel statement
	// &AAA = TRUNC( &BBB,'.' )
	// &AAA = TRUNC ( &BBB, 3  )

	int    p ;
	string t ;

	iupper( s )    ;
	p = s.find( '=' ) ;
	if ( p == string::npos )
	{
		err.seterrid( "PSYE033O" ) ;
		return ;
	}
	trnc_field1 = strip( s.substr( 0, p ) ) ;
	if ( trnc_field1 == "" )
	{
		err.seterrid( "PSYE038A" ) ;
		return ;
	}
	if ( trnc_field1.front() != '&' )
	{
		err.seterrid( "PSYE033I" ) ;
		return ;
	}
	trnc_field1.erase( 0, 1 ) ;

	trim( s.erase( 0, p+1 ) ) ;
	if ( s.compare( 0, 5, "TRUNC" ) != 0 )
	{
		err.seterrid( "PSYE038B" ) ;
		return ;
	}
	trim( s.erase( 0, 5 ) ) ;
	p = s.find( '(' ) ;
	if ( p != 0 )
	{
		err.seterrid( "PSYE033D" ) ;
		return ;
	}
	s.erase( 0, 1 ) ;
	p = s.find( ',' ) ;
	if ( p == string::npos )
	{
		err.seterrid( "PSYE038C" ) ;
		return ;
	}
	trnc_field2 = strip( s.substr( 0, p ) ) ;

	if ( trnc_field2 == "" )
	{
		err.seterrid( "PSYE038D" ) ;
		return ;
	}
	if ( trnc_field2.front() != '&' )
	{
		err.seterrid( "PSYE033I" ) ;
		return ;
	}
	trnc_field2.erase( 0, 1 ) ;
	trim( s.erase( 0, p+1 ) ) ;
	if ( s == "" )
	{
		err.seterrid( "PSYE038E" ) ;
		return ;
	}

	if ( s.front() == '\'' )
	{
		if ( s.size() < 4 )
		{
			err.seterrid( "PSYE038F", s ) ;
			return ;
		}
		trnc_char = s[ 1 ] ;
		if ( s[ 2 ] != '\'' )
		{
			err.seterrid( "PSYE033F" ) ;
			return ;
		}
		trim( s.erase( 0, 3 ) ) ;
		if ( s != ")" )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
	}
	else
	{
		if ( s.size() < 2 )
		{
			err.seterrid( "PSYE038F", s ) ;
			return ;
		}
		p = s.find( ')' ) ;
		if ( p == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		t = strip( s.substr( 0, p ) ) ;
		trim( s.erase( 0, p ) )  ;
		if ( !datatype( t, 'W' ) )
		{
			err.seterrid( "PSYE019E" ) ;
			return ;
		}
		trnc_len = ds2d( t ) ;
		if ( trnc_len <= 0 )
		{
			err.seterrid( "PSYE019F" ) ;
			return ;
		}
		if ( s != ")" )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
	}
	if ( !isvalidName( trnc_field1 ) )
	{
		err.seterrid( "PSYE031D", trnc_field1 ) ;
		return ;
	}
	if ( !isvalidName( trnc_field2 ) )
	{
		err.seterrid( "PSYE031D", trnc_field2 ) ;
		return ;
	}
	return ;
}


void TRANS::parse( errblock& err, string s )
{
	// Format of the TRANS panel statement ( change val1 to val2, * is everything else.  Issue message. )

	// &AAA = TRANS( &BBB  val1,val2 ...  *,* )
	// &AAA = TRANS ( &BBB  val1,val2 ...  *,'?' )
	// &AAA = TRANS ( &BBB  val1,val2 ...  ) non-matching results in &AAA being set to null
	// &AAA = TRANS ( &AAA  val1,val2 ...  MSG=msgid ) issue message if no match

	int    p  ;
	int    p1 ;
	int    p2 ;
	int    i  ;
	int    j  ;
	string v1 ;
	string v2 ;

	iupper( s )    ;
	p = s.find( '=' ) ;
	if ( p == string::npos )
	{
		err.seterrid( "PSYE033O" ) ;
		return ;
	}

	trns_field1 = strip( s.substr( 0, p ) ) ;
	if ( trns_field1 == "" )
	{
		err.seterrid( "PSYE039A" ) ;
		return ;
	}

	if ( trns_field1.front() != '&' )
	{
		err.seterrid( "PSYE033I" ) ;
		return ;
	}

	trns_field1.erase( 0, 1 ) ;
	if ( !isvalidName( trns_field1 ) )
	{
		err.seterrid( "PSYE031D", trns_field1 ) ;
		return ;
	}

	trim( s.erase( 0, p+1 ) ) ;
	if ( s.compare( 0, 5, "TRANS" ) != 0 )
	{
		err.seterrid( "PSYE039B" ) ;
		return ;
	}

	trim( s.erase( 0, 5 ) ) ;
	if ( s.front() != '(' )
	{
		err.seterrid( "PSYE033D" ) ;
		return ;
	}

	trim( s.erase( 0, 1 ) ) ;
	p = s.find( ' ' ) ;
	if ( p == string::npos )
	{
		err.seterrid( "PSYE033C" ) ;
		return ;
	}

	trns_field2 = strip( s.substr( 0, p ) ) ;
	if ( trns_field2 == "" )
	{
		err.seterrid( "PSYE039D" ) ;
		return ;
	}

	if ( trns_field2.front() != '&' )
	{
		err.seterrid( "PSYE033I" ) ;
		return ;
	}

	trns_field2.erase( 0, 1 ) ;
	if ( !isvalidName( trns_field2 ) )
	{
		err.seterrid( "PSYE031D", trns_field2 ) ;
		return ;
	}

	trim( s.erase( 0, p+1 ) ) ;
	if ( s == "" )
	{
		err.seterrid( "PSYE039E" ) ;
		return ;
	}

	if ( s.back() != ')' )
	{
		err.seterrid( "PSYE032D" ) ;
		return ;
	}
	s.pop_back() ;

	j = countc( s, ',' ) ;
	if ( j == 0 )
	{
		err.seterrid( "PSYE039F" ) ;
		return ;
	}

	for ( i = 1 ; i <= j ; i++ )
	{
		p1 = s.find( ',' ) ;
		v1 = strip( s.substr( 0, p1 ) ) ;
		trim( s.erase( 0, p1+1 ) ) ;
		p2 = s.find( ' ' ) ;
		if ( p2 == string::npos )
		{
			v2 = s  ;
			s  = "" ;
		}
		else
		{
			v2 = strip( s.substr( 0, p2 ) ) ;
			trim( s.erase( 0, p2+1 ) )      ;
		}
		if ( trns_list.count( v1 ) > 0 )
		{
			err.seterrid( "PSYE039G", v1 ) ;
			return ;
		}
		if ( words( v1 ) != 1 )
		{
			err.seterrid( "PSYE039H", v1 ) ;
			return ;
		}
		if ( words( v2 ) != 1 )
		{
			err.seterrid( "PSYE039H", v2 ) ;
			return ;
		}
		trns_list[ v1 ] = v2 ;
	}
	trim( s ) ;
	if ( s.compare( 0, 4, "MSG=" ) == 0 )
	{
		trns_msg = strip( substr( s, 5 ) ) ;
		if ( !isvalidName( trns_msg ) )
		{
			err.seterrid( "PSYE031D", trns_msg ) ;
			return ;
		}
	}
	else
	{
		if ( s != "" )
		{
			err.seterrid( "PSYE032H", s ) ;
			return ;
		}
	}
	return ;
}


void pnts::parse( errblock& err, string s )
{
	// Format of the PNTS panel entry (point-and-shoot entries)

	// FIELD(fld) VAR(var) VAL(value)
	// fld and var must be defined in the panel

	int p1 ;
	int p2 ;

	iupper( s ) ;

	p1 = s.find( "FIELD(" ) ;
	if ( p1 == string::npos )
	{
		err.seterrid( "PSYE031C", "FIELD" ) ;
		return ;
	}

	p2 = s.find( ")", p1 ) ;
	if ( p2 == string::npos )
	{
		err.seterrid( "PSYE032D" ) ;
		return ;
	}

	pnts_field = strip( s.substr( p1+6, p2-p1-6  ) ) ;
	if ( !isvalidName( pnts_field ) )
	{
		err.seterrid( "PSYE031D", pnts_field ) ;
		return ;
	}
	s.erase( p1, p2-p1+1) ;

	p1 = s.find( "VAR(" ) ;
	if ( p1 == string::npos )
	{
		err.seterrid( "PSYE031C", "VAR" ) ;
		return ;
	}

	p2 = s.find( ")", p1 ) ;
	if ( p2 == string::npos )
	{
		err.seterrid( "PSYE032D" ) ;
		return ;
	}

	pnts_var = strip( s.substr( p1+4, p2-p1-4  ) ) ;
	if ( !isvalidName( pnts_var ) )
	{
		err.seterrid( "PSYE031D", pnts_var ) ;
		return ;
	}
	s.erase( p1, p2-p1+1) ;

	p1 = s.find( "VAL(" ) ;
	if ( p1 == string::npos )
	{
		err.seterrid( "PSYE031C", "VAL" ) ;
		return ;
	}

	p2 = s.find( ")", p1 ) ;
	if ( p2 == string::npos )
	{
		err.seterrid( "PSYE032D" ) ;
		return ;
	}

	pnts_val = strip( s.substr( p1+4, p2-p1-4  ) ) ;
	s.erase( p1, p2-p1+1) ;

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
