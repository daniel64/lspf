/*
  Copyright (c) 2015 Daniel John Erdos

  This program is free software; you can redistribute it and/or modifyf
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

bool IFSTMNT::parse( string s )
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

	bool  f_end ;

	string t    ;
	string comp ;

	p1 = s.find( '(' ) ;
	if ( p1 == string::npos ) { return false ; }
	s = strip( s.erase( 0, p1+1 ) ) ;

	p1 = s.find( '(' ) ;
	if ( p1 != string::npos )
	{
		t = strip( s.substr( 0, p1 ) ) ;
		if ( t == "VER" )
		{
			p2 = s.find_last_of( ")" ) ;
			if ( p2 == string::npos ) { return false ; }
			t = s.substr( 0, p2 ) ;
			if ( !if_verify.parse( t ) ) { return false ; }
			if ( if_verify.ver_msgid != "" ) { return false ; }
			if ( strip( s.substr( p2+1 ) ) != "" ) { return false ; }
			if_ver = true ;
			return true ;
		}
	}

	p1 = s.find_first_of( "=><!" ) ;
	if ( p1 == string::npos )
	{
		p2 = s.find( ' ' ) ;
		if ( p2 == string::npos ) { return false ; }
		if_lhs = upper( strip( s.substr( 0, p2 ) ) ) ;
		p1 = s.find_first_not_of( ' ', p2 ) ;
		if ( p1 == string::npos ) { return false ; }
		p2 = s.find( ' ', p1 ) ;
		if ( p2 == string::npos ) { return false ; }
	}
	else
	{
		p2 = s.find_first_not_of( "=><!", p1 ) ;
		if ( p2 == string::npos ) { return false ; }
		if_lhs = upper( strip( s.substr( 0, p1 ) ) ) ;
	}

	comp = s.substr( p1, p2-p1 ) ;
	s    = strip( s.erase( 0, p2 ) ) ;

	if ( words( if_lhs ) != 1 ) { return false ; }
	if      ( if_lhs    == ".CURSOR" ) {}
	else if ( if_lhs    == ".MSG"    ) {}
	else if ( if_lhs    == ".RESP"   ) {}
	else if ( if_lhs[0] != '&' ) { return false ; }
	else
	{
		if_lhs.erase( 0, 1 ) ;
		if ( !isvalidName( if_lhs ) ) { return false ; }
	}

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
	else                     { return false ; }

	f_end = false ;

	while ( true )
	{
		if ( s[ 0 ] == '&' )
		{
			p1 = s.find( ',' ) ;
			if ( p1 == string::npos )
			{
				p1 = s.find( ')' ) ;
				if ( p1 == string::npos ) { return false ; }
				f_end = true ;
			}
			t = upper( strip( s.substr( 0, p1 ) ) ) ;
			t.erase( 0, 1 ) ;
			if ( !isvalidName( t ) ) { return false ; }
			if_isvar.push_back( true ) ;
		}
		else if ( s[ 0 ]  == '\'' )
		{
			s.erase( 0, 1 ) ;
			p1 = s.find( '\'' ) ;
			if ( p1 == string::npos ) { return false ; }
			t  = s.substr( 0, p1 ) ;
			s  = strip( s.erase( 0, p1+1 ) ) ;
			p1 = s.find( ',' ) ;
			if ( p1 == string::npos )
			{
				p1 = s.find( ')' ) ;
				if ( p1 == string::npos ) { return false ; }
				f_end = true ;
			}
			if_isvar.push_back( false ) ;
		}
		else if ( s[ 0 ]  == '.' )
		{
			s.erase( 0, 1 ) ;
			p1 = s.find( ',' ) ;
			if ( p1 == string::npos )
			{
				p1 = s.find( ')' ) ;
				if ( p1 == string::npos ) { return false ; }
				f_end = true ;
			}
			t = upper( strip( s.substr( 0, p1 ) ) ) ;
			if      ( t == "TRUE" )  { t = "1" ; }
			else if ( t == "FALSE" ) { t = "0" ; }
			else    { return false             ; }
			if_isvar.push_back( false ) ;
		}
		else
		{
			p1 = s.find( ',' ) ;
			if ( p1 == string::npos )
			{
				p1 = s.find( ')' ) ;
				if ( p1 == string::npos ) { return false ; }
				f_end = true ;
			}
			t = upper( strip( s.substr( 0, p1 ) ) ) ;
			if ( t == "" || (t.find_first_of( " )" ) != string::npos) ) { return false ; }
			if_isvar.push_back( false ) ;
		}
		if_rhs.push_back( t ) ;
		s = strip( s.erase( 0, p1+1 ) ) ;
		if ( s == "" && !f_end )  { return false ; }
		if ( f_end ) { break ; }
	}
	if ( s != "" )  { return false ; }
	if ( (!if_eq && !if_ne) && if_rhs.size() > 1 ) { return false ; }
	return true ;
}


bool ASSGN::parse( string s )
{
	// Format of the assignment panel statement

	// &AAA = &BBBB
	// &AAA = VALUE
	// &AAA = 'Quoted Value'
	// &AAA = .ALARM | .TRAIL | .HELP | .MSG | .CURSOR | .RESP
	// .ALARM .RESP | .HELP | .MSG | .CURSOR = &BBB | VALUE | 'Quoted Value'
	// &AAA = UPPER( ABC )
	// &AAA = LENGTH( ABC )
	// &AAA = REVERSE( ABC )
	// &AAA = WORDS( ABC )  Number of words in the value of ABC
	// &A   = EXISTS( ABC ) True if file/directory in variable ABC exists
	// &A   = FILE( ABC )   True if file in variable ABC exists
	// &A   = DIR( ABC )    True if directory in variable ABC exists

	int p  ;
	int p1 ;

	p = s.find( '=' ) ;
	if ( p == string::npos ) { return false ; }
	as_lhs = upper( strip( s.substr( 0, p ) ) ) ;
	if ( words( as_lhs ) != 1 ) {  return false ; }
	if      ( findword( as_lhs, ".ALARM .AUTOSEL .CURSOR .CSRROW .HELP .MSG .NRET .RESP" ) ) {}
	else if ( as_lhs.substr( 0, 6 ) == ".ATTR(" )
	{
		p1 = as_lhs.find( ')' ) ;
		if ( p1 == string::npos ) { return false ; }
		as_lhs = strip( as_lhs.substr( 6, p1-6 ) ) ;
		if ( !isvalidName( as_lhs ) ) { return false ; }
		as_isattr = true ;
	}
	else if ( as_lhs[0] == '&' )
	{
		as_lhs.erase( 0, 1 ) ;
		if ( !isvalidName( as_lhs ) ) { return false ; }
	}
	else  { return false ; }
	s = strip( s.erase( 0, p+1 ) ) ;
	if ( s[ 0 ] == '&' )
	{

		if ( words( s ) != 1 ) { return false ; }
		s.erase( 0, 1 ) ;
		s = upper( s )  ;
		if ( !isvalidName( s ) ) { return false ; }
		as_rhs   = s   ;
		as_isvar = true ;
	}
	else if ( s[ 0 ]  == '\'' )
	{
		s.erase( 0, 1 ) ;
		p = s.find( '\'' ) ;
		if ( p == string::npos ) { return false ; }
		as_rhs = s.substr( 0, p ) ;
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { return false ; }
	}
	else if ( upper( s.substr( 0, 4 ) ) == "DIR(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 4 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { return false ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { return false ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { return false ; }
		as_isvar  = true ;
		as_chkdir = true ;
	}
	else if ( upper( s.substr( 0, 7 ) ) == "EXISTS(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 7 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { return false ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { return false ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { return false ; }
		as_isvar   = true ;
		as_chkexst = true ;
	}
	else if ( upper( s.substr( 0, 5 ) ) == "FILE(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 5 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { return false ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { return false ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { return false ; }
		as_isvar   = true ;
		as_chkfile = true ;
	}
	else if ( upper( s.substr( 0, 7 ) ) == "LENGTH(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 7 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { return false ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { return false ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { return false ; }
		as_isvar  = true ;
		as_retlen = true ;
	}
	else if ( upper( s.substr( 0, 8 ) ) == "REVERSE(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 8 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { return false ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { return false ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { return false ; }
		as_isvar   = true ;
		as_reverse = true ;
	}
	else if ( upper( s.substr( 0, 6 ) ) == "WORDS(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 6 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { return false ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { return false ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { return false ; }
		as_isvar = true ;
		as_words = true ;
	}
	else if ( upper( s.substr( 0, 6 ) ) == "UPPER(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 6 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { return false ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { return false ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { return false ; }
		as_isvar = true ;
		as_upper = true ;
	}
	else
	{
		if ( words( s ) != 1 ) { return false ; }
		s = upper( s ) ;
		if ( s[ 0 ]  == '.' && !findword( s, ".ALARM .CURSOR .HELP .MSG .TRAIL .RESP" ) ) { return false ; }
		as_rhs = s ;
	}
	return true ;
}


bool VPUTGET::parse( string s )
{
	// VGET ABC
	// VGET(ABC) PROFILE

	int i  ;
	int j  ;
	int ws ;
	int p1 ;

	string w1 ;

	s  = upper( s )    ;
	p1 = s.find( '(' ) ;
	if ( p1 == string::npos ) { w1 = word( s, 1 )               ; s = subword( s, 2 )           ; }
	else                      { w1 = strip( s.substr( 0, p1 ) ) ; s = strip( s.erase( 0, p1 ) ) ; }

	if ( s == "" ) { return false ; }

	( w1 == "VPUT" ) ? vpg_vput = true : vpg_vget = true ;

	if ( s[ 0 ] == '(' )
	{
		i = s.find( ')', 1 ) ;
		if ( i == string::npos )  { return false ; }
		vpg_vars = s.substr( 1, i-1 ) ;
		replace( vpg_vars.begin(), vpg_vars.end(), ',', ' ' ) ;
		for( ws = words( vpg_vars ), j = 1 ; j <= ws ; j++ )
		{
			if ( !isvalidName( word( vpg_vars, j ) ) )  { return false ; }
		}
		s.erase( 0, i+1 ) ;
	}
	else
	{
		vpg_vars = word( s, 1 ) ;
		if ( !isvalidName( vpg_vars ) ) { return false ; }
		s = subword( s, 2 ) ;
	}

	s = strip( s ) ;
	if ( s == "ASIS" || s == "" ) { vpg_pool  = ASIS    ; }
	else if ( s == "SHARED" )     { vpg_pool  = SHARED  ; }
	else if ( s == "PROFILE" )    { vpg_pool  = PROFILE ; }
	else                          { return false        ; }
	return true ;
}


bool VERIFY::parse( string s )
{
	// VER (&VAR LIST A B C D)
	// VER (&VAR,LIST,A,B,C,D)
	// VER (&VAR NB LIST A B C D)
	// VER(&VAR NONBLANK LIST A B C D)
	// VER(&VAR PICT ABCD)
	// VER(&VAR HEX)
	// VER(&VAR OCT)


	int i     ;
	int p     ;
	string t  ;
	string w  ;
	string w1 ;
	string w2 ;

	p = s.find( '(' ) ;
	if ( p == string::npos ) { return false ; }
	t = upper( strip( s.substr( 0, p ) ) ) ;
	if ( t != "VER" ) { return false ; }
	s = upper( strip( s.erase( 0, p ) ) ) ;
	w1 = word( s, 1 ) ;

	p = 0 ;
	replace( s.begin(), s.end(), ',', ' ' ) ;

	if ( w1.size() == 0 ) { return false ; }
	else if ( w1.size() == 1 )
	{
		if ( w1 != "(" ) { return false ; }
		s = subword( s, 2 )  ;
		if ( s[ 0 ] != '&' ) { return false ; }
		s = substr( s, 2 )  ;
	}
	else
	{
		if ( w1.substr( 0, 2 ) != "(&" ) { return false ; }
		s = substr( s, 3 ) ;
	}
	if ( s.back() != ')' ) { return false ; }
	s = substr( s, 1, s.size()-1 ) ;

	ver_var = word( s, 1 ) ;
	if ( !isvalidName( ver_var ) ) { return false ; }

	w2 = word( s, 2 ) ;

	if ( w2 == "NB" || w2 == "NONBLANK" ) { ver_nblank = true ; i = 3 ; }
	else                                  { i = 2 ;                     }

	while ( true )
	{
		w = word( s, i ) ;
		if ( w == "" ) break ;
		if      ( w == "NUM" )  { ver_numeric = true  ; }
		else if ( w == "LIST" ) { ver_list    = true  ; }
		else if ( w == "PICT" ) { ver_pict    = true  ; }
		else if ( w == "HEX" )  { ver_hex     = true  ; }
		else if ( w == "OCT" )  { ver_octal   = true  ; }
		else if ( substr( w, 1, 4 ) == "MSG=" )
		{
			ver_msgid = strip( substr( w, 5 ) ) ;
			if ( ver_msgid == "" ) { return false ; }
		}
		else if ( ver_pict ) { if ( ver_value != "" ) return false ; else ver_value = w ; }
		else if ( ver_list ) { ver_value = ver_value + " " + w ; }
		else    { return false ; }
		i++ ;
	}
	if ( !ver_nblank && !ver_numeric && !ver_list && !ver_pict && !ver_hex && !ver_octal )
	{
		return false ;
	}
	if ( (ver_list || ver_pict) && ver_value == "" )
	{
		return false ;
	}
	return true ;
}


bool TRUNC::parse( string s )
{
	// Format of the TRUNC panel statement
	// &AAA = TRUNC( &BBB,'.' )
	// &AAA = TRUNC ( &BBB, 3  )

	int    p ;
	string t ;

	s = upper( s )    ;
	p = s.find( '=' ) ;
	if ( p == string::npos ) { return false ; }
	trnc_field1 = strip( s.substr( 0, p ) ) ;
	if ( trnc_field1 == "" )       { return false ; }
	if ( trnc_field1[ 0 ] != '&' ) { return false ; }
	trnc_field1.erase( 0, 1 ) ;

	s = strip( s.erase( 0, p+1 ) ) ;
	if ( s.substr( 0, 5) != "TRUNC" ) { return false ; }
	s = strip( s.erase( 0, 5 ) ) ;
	p = s.find( '(' ) ;
	if ( p != 0 ) { return false ; }
	s.erase( 0, 1 ) ;
	p = s.find( ',' ) ;
	if ( p == string::npos ) { return false ; }
	trnc_field2 = strip( s.substr( 0, p ) ) ;

	if ( trnc_field2 == "" )     { return false ; }
	if ( trnc_field2[0] != '&' ) { return false ; }
	trnc_field2.erase( 0, 1 ) ;
	s = strip( s.erase( 0, p+1 ) ) ;
	if ( s == "" ) { return false ; }

	if ( s[0] == '\'' )
	{
		if ( s.size() < 4 ) { return false ; }
		trnc_char = s[ 1 ] ;
		if ( s[ 2 ] != '\'' ) { return false ; }
		s = strip( s.erase( 0, 3 ) ) ;
		if ( s != ")" ) { return false ; }
	}
	else
	{
		if ( s.size() < 2 ) { return false ; }
		p = s.find( ')' ) ;
		if ( p == string::npos ) { return false ; }
		t = strip( s.substr( 0, p ) ) ;
		s = strip( s.erase( 0, p ) )  ;
		if ( !datatype( t, 'W' ) ) { return false ; }
		trnc_len = ds2d( t ) ;
		if ( trnc_len <= 0 ) { return false ; }
		if ( s != ")" ) { return false ; }
	}
	if ( !isvalidName( trnc_field1 ) || !isvalidName( trnc_field2 ) ) { return false ; }
	return true ;
}


bool TRANS::parse( string s )
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

	s = upper( s )    ;
	p = s.find( '=' ) ;
	if ( p == string::npos ) { return false ; }
	trns_field1 = strip( s.substr( 0, p ) ) ;
	if ( trns_field1 == "" )       { return false ; }
	if ( trns_field1[ 0 ] != '&' ) { return false ; }
	trns_field1.erase( 0, 1 ) ;
	if ( !isvalidName( trns_field1 ) ) { return false ; }

	s = strip( s.erase( 0, p+1 ) ) ;
	if ( s.substr( 0, 5 ) != "TRANS" ) { return false ; }
	s = strip( s.erase( 0, 5 ) ) ;
	p = s.find( '(' ) ;
	if ( p != 0 ) { return false ; }
	s = strip( s.erase( 0, 1 ) ) ;
	p = s.find( ' ' ) ;
	if ( p == string::npos ) { return false ; }
	trns_field2 = strip( s.substr( 0, p ) ) ;

	if ( trns_field2 == "" )       { return false ; }
	if ( trns_field2[ 0 ] != '&' ) { return false ; }
	trns_field2.erase( 0, 1 ) ;
	if ( !isvalidName( trns_field2 ) ) { return false ; }

	s = strip( s.erase( 0, p+1 ) ) ;
	if ( s == "" )         { return false ; }
	if ( s.back() != ')' ) { return false ; }
	s.pop_back() ;

	j = countc( s, ',' ) ;
	if ( j == 0 )  { return false ; }

	for ( i = 1 ; i <= j ; i++ )
	{
		p1 = s.find( ',' ) ;
		v1 = strip( s.substr( 0, p1 ) ) ;
		s  = strip( s.erase( 0, p1+1 ) ) ;
		p2 = s.find( ' ' ) ;
		if ( p2 == string::npos )
		{
			v2 = s  ;
			s  = "" ;
		}
		else
		{
			v2 = strip( s.substr( 0, p2 ) ) ;
			s  = strip( s.erase( 0, p2+1 ) ) ;
		}
		if ( trns_list.find( v1 ) != trns_list.end() ) { return false ; }
		if ( words( v1 ) != 1 ) { return false ; }
		if ( words( v2 ) != 1 ) { return false ; }
		trns_list[ v1 ] = v2 ;
	}
	s = strip( s ) ;
	if ( substr( s, 1, 4 ) == "MSG=" )
	{
		trns_msg = strip( substr( s, 5 ) ) ;
		if ( !isvalidName( trns_msg ) ) { return false ; }
	}
	else
	{
		if ( s != "" ) { return false ; }
	}
	return true ;
}


bool pnts::parse( string s )
{
	// Format of the PNTS panel entry (point-and-shoot entries)

	// FIELD(fld) VAR(var) VAL(value)
	// fld and var must be defined in the panel

	int p1 ;
	int p2 ;

	s  = upper( s ) ;

	p1 = pos( "FIELD(", s ) ;
	if ( p1 == 0 ) { return false ; }

	p2 = pos( ")", s, p1 ) ;
	if ( p2 == 0 ) { return false ; }
	pnts_field = strip( substr( s, (p1 + 6), (p2 - (p1 + 6)) ) ) ;
	if ( !isvalidName( pnts_field ) ) { return false ; }
	s = delstr( s, p1, (p2 - p1 + 1) ) ;

	p1 = pos( "VAR(", s ) ;
	if ( p1 == 0 ) { return false ; }

	p2 = pos( ")", s, p1 ) ;
	if ( p2 == 0 ) { return false ; }
	pnts_var = strip( substr( s, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
	if ( !isvalidName( pnts_var ) ) { return false ; }
	s = delstr( s, p1, (p2 - p1 + 1) ) ;

	p1 = pos( "VAL(", s ) ;
	if ( p1 == 0 ) { return false ; }

	p2 = pos( ")", s, p1 ) ;
	if ( p2 == 0 ) { return false ; }
	pnts_val = strip( substr( s, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
	s = strip( delstr( s, p1, (p2 - p1 + 1) ) ) ;

	if ( s != "" || pnts_field == "" || pnts_var == "" || pnts_val == "" ) { return false ; }
	return true ;
}


bool selobj::parse( string SELSTR )
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

	// Match brackets for PARM and CMD as these may contain brackets

	int ob ;
	int p1 ;
	int p2 ;

	string lang ;
	string str  ;

	clear() ;
	str = upper( SELSTR )     ;
	p1  = pos( "PARM(", str ) ;
	if ( p1 > 0 )
	{
		ob = 1 ;
		for ( p2 = p1+4 ; p2 < SELSTR.size() ; p2++ )
		{
			if ( SELSTR.at( p2 ) == '(' ) { ob++  ; }
			if ( SELSTR.at( p2 ) == ')' )
			{
				ob-- ;
				if ( ob == 0 ) { break ; }
			}
		}
		if ( ob != 0 ) { return false ; }
		p2++ ;
		PARM   = strip( substr( SELSTR, (p1 + 5), (p2 - (p1 + 5)) ) ) ;
		PARM   = strip( PARM, 'B', '"' ) ;
		SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
		str    = upper( SELSTR ) ;
	}

	p1 = pos( "PGM(", str ) ;
	if ( p1 > 0 )
	{
		p2     = pos( ")", SELSTR, p1 ) ;
		if ( p2 == 0 ) { return false ; }
		PGM    = strip( substr( str, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
		SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
		str    = upper( SELSTR ) ;
		if ( PGM[ 0 ] == '&' )
		{
			if ( !isvalidName( substr( PGM, 2 ) ) ) { return false ; }
		}
		else
		{
			if ( !isvalidName( PGM ) ) { return false ; }
		}
	}
	else
	{
		p1 = pos( "PANEL(", str ) ;
		if ( p1 > 0 )
		{
			if ( PARM != "" ) { return false ; }
			p2 = pos( ")", SELSTR, p1 ) ;
			if ( p2 == 0 ) { return false ; }
			PARM = strip( substr( str, (p1 + 6), (p2 - (p1 + 6)) ) ) ;
			if ( !isvalidName( PARM ) ) { return false ; }
			PGM    = "&ZPANLPGM"  ;
			SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
			str    = upper( SELSTR ) ;
			p1 = pos( "OPT(", str ) ;
			if ( p1 > 0 )
			{
				p2 = pos( ")", SELSTR, p1 ) ;
				if ( p2 == 0 ) { return false ; }
				PARM = PARM + " " + strip( substr( str, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
				SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
				str    = upper( SELSTR ) ;
			}
		}
		else
		{
			p1 = pos( "CMD(", str ) ;
			if ( p1 > 0 )
			{
				if ( PARM != "" ) { return false ; }
				ob = 1 ;
				for ( p2 = p1+3 ; p2 < SELSTR.size() ; p2++ )
				{
					if ( SELSTR.at( p2 ) == '(' ) { ob++  ; }
					if ( SELSTR.at( p2 ) == ')' )
					{
						ob-- ;
						if ( ob == 0 ) { break ; }
					}
				}
				if ( ob != 0 ) { return false ; }
				p2++ ;
				PARM   = strip( substr( SELSTR, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
				SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
				str    = upper( SELSTR ) ;
				lang   = "" ;
				p1     = pos( "LANG(", str ) ;
				if ( p1 > 0 )
				{
					p2     = pos( ")", SELSTR, p1 ) ;
					if ( p2 == 0 ) { return false ; }
					lang   = strip( substr( str, (p1 + 5), (p2 - (p1 + 5)) ) ) ;
					SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
					str    = upper( SELSTR ) ;
				}
				if ( lang == "" || lang == "REXX" )
				{
					PGM = "&ZOREXPGM"  ;
				}
				else { return false ; }
			}
		}
	}

	p1 = pos( "SCRNAME(", str ) ;
	if ( p1 > 0 )
	{
		p2      = pos( ")", str, p1 ) ;
		if ( p2 == 0 ) { return false ; }
		SCRNAME = strip( substr( str, (p1 + 8), (p2 - (p1 + 8)) ) ) ;
		str     = delstr( str, p1, (p2 - p1 + 1) ) ;
		if ( !isvalidName( SCRNAME ) || findword( SCRNAME, "LIST NEXT PREV" ) ) { return false ; }
	}

	p1 = pos( "NEWAPPL(", str ) ;
	if ( p1 > 0 )
	{
		p2      = pos( ")", str, p1 ) ;
		if ( p2 == 0 ) { return false ; }
		NEWAPPL = strip( substr( str, (p1 + 8), (p2 - (p1 + 8)) ) ) ;
		NEWPOOL = true ;
		str     = delstr( str, p1, (p2 - p1 + 1) ) ;
		if ( !isvalidName4( NEWAPPL ) ) { return false ; }
	}
	else
	{
		p1 = wordpos( "NEWAPPL", str ) ;
		if ( p1 > 0 )
		{
			NEWAPPL = "ISP";
			NEWPOOL = true ;
			str     = delword( str, p1, 1 ) ;
		}
	}

	p1 = wordpos( "NEWPOOL", str ) ;
	if ( p1 > 0 )
	{
		NEWPOOL = true ;
		str     = delword( str, p1, 1 ) ;
	}

	p1 = wordpos( "SUSPEND", str ) ;
	if ( p1 > 0 )
	{
		SUSPEND = true ;
		str     = delword( str, p1, 1 ) ;
	}

	p1 = wordpos( "PASSLIB", str ) ;
	if ( p1 > 0 )
	{
		if ( NEWAPPL == "" ) { return false ; }
		PASSLIB = true ;
		str     = delword( str, p1, 1 ) ;
	}

	if ( PGM == "" || strip( str ) != "" ) { return false ; }

	return true ;
}
