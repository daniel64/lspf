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

IFSTMNT::IFSTMNT( string s )
{
	// Format of the IF panel statement
	// IF ( &AAA=&BBBB )
	// IF ( &AAA = VALUE1,VALUE2 )
	// IF ( &AAA NE 'Hello','Goodbye' )
	// IF (.CURSOR = ZCMD)
	// IF (.MSG    = PSYS011)
	// IF ( &Z EQ .TRUE )
	// IF ( &Z EQ .FALSE ) .TRUE = "1" and .FALSE = "0"
	// rhs value lists only for EQ and NE (EQ only one needs to be true, NE all need to be true)


	int p1 ;
	int p2 ;

	bool  f_end ;

	string t    ;
	string comp ;

	if_RC    = 0     ;
	if_lhs   = ""    ;
	if_rhs.clear()   ;
	if_isvar.clear() ;
	if_stmnt = 0     ;
	if_true  = false ;
	if_else  = false ;
	if_istb  = false ;
	if_eq    = false ;
	if_ne    = false ;
	if_gt    = false ;
	if_lt    = false ;
	if_ge    = false ;
	if_le    = false ;
	if_ng    = false ;
	if_nl    = false ;

	p1 = s.find( '(' ) ;
	if ( p1 == string::npos ) { if_RC = 20 ; return ; }
	t = upper( strip( s.substr( 0, p1 ) ) ) ;
	if ( t != "IF" ) { if_RC = 20 ; return ; }
	s = strip( s.erase( 0, p1+1 ) ) ;

	p1 = s.find_first_of( "=><!" ) ;
	if ( p1 == string::npos )
	{
		p2 = s.find( ' ' ) ;
		if ( p2 == string::npos ) { if_RC = 20 ; return ; }
		if_lhs = upper( strip( s.substr( 0, p2 ) ) ) ;
		p1 = s.find_first_not_of( ' ', p2 ) ;
		if ( p1 == string::npos ) { if_RC = 20 ; return ; }
		p2 = s.find( ' ', p1 ) ;
		if ( p2 == string::npos ) { if_RC = 20 ; return ; }
	}
	else
	{
		p2 = s.find_first_not_of( "=><!", p1 ) ;
		if ( p2 == string::npos ) { if_RC = 20 ; return ; }
		if_lhs = upper( strip( s.substr( 0, p1 ) ) ) ;
	}

	comp = s.substr( p1, p2-p1 ) ;
	s    = strip( s.erase( 0, p2 ) ) ;

	if ( words( if_lhs ) != 1 ) { if_RC = 20 ; return ; }
	if      ( if_lhs    == ".CURSOR" ) {}
	else if ( if_lhs    == ".MSG"    ) {}
	else if ( if_lhs[0] != '&' ) { if_RC = 20 ; return ; }
	else
	{
		if_lhs.erase( 0, 1 ) ;
		if ( !isvalidName( if_lhs ) ) { if_RC = 20 ; return ; }
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
	else                     { if_RC = 20 ; return ; }

	f_end = false ;

	while ( true )
	{
		if ( s[ 0 ] == '&' )
		{
			p1 = s.find( ',' ) ;
			if ( p1 == string::npos )
			{
				p1 = s.find( ')' ) ;
				if ( p1 == string::npos ) { if_RC = 20 ; return ; }
				f_end = true ;
			}
			t = upper( strip( s.substr( 0, p1 ) ) ) ;
			t.erase( 0, 1 ) ;
			if ( !isvalidName( t ) ) { if_RC = 20 ; return ; }
			if_isvar.push_back( true ) ;
		}
		else if ( s[ 0 ]  == '\'' )
		{
			s.erase( 0, 1 ) ;
			p1 = s.find( '\'' ) ;
			if ( p1 == string::npos ) { if_RC = 20 ; return ; }
			t  = s.substr( 0, p1 ) ;
			s  = strip( s.erase( 0, p1+1 ) ) ;
			p1 = s.find( ',' ) ;
			if ( p1 == string::npos )
			{
				p1 = s.find( ')' ) ;
				if ( p1 == string::npos ) { if_RC = 20 ; return ; }
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
				if ( p1 == string::npos ) { if_RC = 20 ; return ; }
				f_end = true ;
			}
			t = upper( strip( s.substr( 0, p1 ) ) ) ;
			if      ( t == "TRUE" )  { t = "1" ; }
			else if ( t == "FALSE" ) { t = "0" ; }
			else    { if_RC = 20 ; return ; }
			if_isvar.push_back( false ) ;
		}
		else
		{
			p1 = s.find( ',' ) ;
			if ( p1 == string::npos )
			{
				p1 = s.find( ')' ) ;
				if ( p1 == string::npos ) { if_RC = 20 ; return ; }
				f_end = true ;
			}
			t = upper( strip( s.substr( 0, p1 ) ) ) ;
			if ( t == "" || (t.find_first_of( " )" ) != string::npos) ) { if_RC = 20 ; return ; }
			if_isvar.push_back( false ) ;
		}
		if_rhs.push_back( t ) ;
		s = strip( s.erase( 0, p1+1 ) ) ;
		if ( s == "" && !f_end )  { if_RC = 20 ; return ; }
		if ( f_end ) { break ; }
	}
	if ( s != "" )  { if_RC = 20 ; return ; }
	if ( (!if_eq && !if_ne) && if_rhs.size() > 1 ) { if_RC = 20 ; return ; }
}


ASSGN::ASSGN( string s )
{
	// Format of the assignment panel statement
	// &AAA = &BBBB
	// &AAA = VALUE
	// &AAA = 'Quoted Value'
	// &AAA = .TRAIL | .HELP | .MSG | .CURSOR
	// .HELP | .MSG | .CURSOR = &BBB | VALUE | 'Quoted Value'
	// &AAA = UPPER( ABC )
	// &AAA = LENGTH( ABC )
	// &AAA = WORDS( ABC )  Number of words in the value of ABC
	// &A   = EXISTS( ABC ) True if file/directory in variable ABC exists
	// &A   = FILE( ABC )   True if file in variable ABC exists
	// &A   = DIR( ABC )    True if directory in variable ABC exists

	int p  ;
	int p1 ;

	as_RC      = 0     ;
	as_lhs     = ""    ;
	as_rhs     = ""    ;
	as_isvar   = false ;
	as_isattr  = false ;
	as_istb    = false ;
	as_retlen  = false ;
	as_upper   = false ;
	as_words   = false ;
	as_chkexst = false ;
	as_chkdir  = false ;
	as_chkfile = false ;

	p = s.find( '=' ) ;
	if ( p == string::npos ) { as_RC = 20 ; return ; }
	as_lhs = upper( strip( s.substr( 0, p ) ) ) ;
	if ( words( as_lhs ) != 1 ) { as_RC = 20 ; return ; }
	if      ( findword( as_lhs, ".AUTOSEL .CURSOR .CSRROW .HELP .MSG .NRET" ) ) {}
	else if ( as_lhs.substr( 0, 6 ) == ".ATTR(" )
	{
		p1 = as_lhs.find( ')' ) ;
		if ( p1 == string::npos ) { as_RC = 20 ; return ; }
		as_lhs = strip( as_lhs.substr( 6, p1-6 ) ) ;
		if ( !isvalidName( as_lhs ) ) { as_RC = 20 ; return ; }
		as_isattr = true ;
	}
	else if ( as_lhs[0] == '&' )
	{
		as_lhs.erase( 0, 1 ) ;
		if ( !isvalidName( as_lhs ) ) { as_RC = 20 ; return ; }
	}
	else  { as_RC = 20 ; return ; }
	s = strip( s.erase( 0, p+1 ) ) ;
	if ( s[ 0 ] == '&' )
	{

		if ( words( s ) != 1 ) { as_RC = 20 ; return ; }
		s.erase( 0, 1 ) ;
		s = upper( s )  ;
		if ( !isvalidName( s ) ) { as_RC = 20 ; return ; }
		as_rhs   = s   ;
		as_isvar = true ;
	}
	else if ( s[ 0 ]  == '\'' )
	{
		s.erase( 0, 1 ) ;
		p = s.find( '\'' ) ;
		if ( p == string::npos ) { as_RC = 20 ; return ; }
		as_rhs = s.substr( 0, p ) ;
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { as_RC = 20 ; return ; }
	}
	else if ( upper( s.substr( 0, 4 ) ) == "DIR(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 4 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { as_RC = 20 ; return ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { as_RC = 20 ; return ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { as_RC = 20 ; return ; }
		as_isvar  = true ;
		as_chkdir = true ;
	}
	else if ( upper( s.substr( 0, 7 ) ) == "EXISTS(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 7 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { as_RC = 20 ; return ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { as_RC = 20 ; return ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { as_RC = 20 ; return ; }
		as_isvar   = true ;
		as_chkexst = true ;
	}
	else if ( upper( s.substr( 0, 5 ) ) == "FILE(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 5 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { as_RC = 20 ; return ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { as_RC = 20 ; return ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { as_RC = 20 ; return ; }
		as_isvar   = true ;
		as_chkfile = true ;
	}
	else if ( upper( s.substr( 0, 7 ) ) == "LENGTH(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 7 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { as_RC = 20 ; return ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { as_RC = 20 ; return ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { as_RC = 20 ; return ; }
		as_isvar  = true ;
		as_retlen = true ;
	}
	else if ( upper( s.substr( 0, 6 ) ) == "WORDS(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 6 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { as_RC = 20 ; return ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { as_RC = 20 ; return ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { as_RC = 20 ; return ; }
		as_isvar = true ;
		as_words = true ;
	}
	else if ( upper( s.substr( 0, 6 ) ) == "UPPER(" )
	{
		s = upper( s )  ;
		s = strip( s.erase( 0, 6 ) ) ;
		p = s.find( ')' ) ;
		if ( p == string::npos ) { as_RC = 20 ; return ; }
		as_rhs = strip( s.substr( 0, p ) ) ;
		if ( !isvalidName( as_rhs ) ) { as_RC = 20 ; return ; }
		s = strip( s.erase( 0, p+1 ) ) ;
		if ( s != "" )  { as_RC = 20 ; return ; }
		as_isvar = true ;
		as_upper = true ;
	}
	else
	{
		if ( words( s ) != 1 ) { as_RC = 20 ; return ; }
		s = upper( s ) ;
		if ( s[ 0 ]  == '.' && !findword( s, ".TRAIL .HELP .MSG .CURSOR" ) ) { as_RC = 20 ; return ; }
		as_rhs = s ;
	}
}


VPUTGET::VPUTGET( string s )
{
	int i  ;
	int j  ;
	int ws ;

	string w    ;
	string w1   ;
	string w2   ;
	string vars ;

	vpg_RC   = 0     ;
	vpg_vput = false ;
	vpg_vget = false ;
	vpg_pool = ASIS  ;


	s = upper( s )      ;
	w1 = word( s, 1 )   ;
	w2 = word( s, 2 )   ;
	w = subword( s, 2 ) ;

	if ( substr( w, 1, 1) == "(" )
	{
		i = pos( ")", w ) ;
		if ( i == 0 )  { vpg_RC = 20 ; return ; }
		vars = substr( w, 2, i-2 ) ;
		w    = strip( substr( w, i+1 ) ) ;
		ws   = words( vars ) ;
	}
	else
	{
		vars = w2 ;
		w    = subword( s, 3 ) ;
		ws   = 1  ;
	}
	for( j = 1; j <= ws ; j++ )
	{
		if ( !isvalidName( word( vars, j ) ) )  { vpg_RC = 20 ; return ; }
	}
	if ( w1 == "VPUT" ) { vpg_vput = true ; }
	else                { vpg_vget = true ; }
	vpg_vars = vars ;

	if ( w == "ASIS" || w == "" ) { vpg_pool  = ASIS     ; }
	else if ( w == "SHARED" )     { vpg_pool  = SHARED   ; }
	else if ( w == "PROFILE" )    { vpg_pool  = PROFILE  ; }
	else                          { vpg_RC = 20 ; return ; }
}


VERIFY::VERIFY( string s )
{
	// VER (&VAR LIST A B C D)
	// VER (&VAR,LIST,A,B,C,D)
	// VER (&VAR NB LIST A B C D)
	// VER (&VAR NONBLANK LIST A B C D)
	// VER (&VAR PICT ABCD)
	// VER (&VAR HEX)
	// VER (&VAR OCT)

	int i     ;
	int p     ;
	string w  ;
	string w1 ;
	string w2 ;

	ver_RC      = 0     ;
	ver_nblank  = false ;
	ver_numeric = false ;
	ver_list    = false ;
	ver_pict    = false ;
	ver_hex     = false ;
	ver_octal   = false ;
	ver_tbfield = false ;

	s  = upper( s )      ;
	s  = subword( s, 2 ) ;
	w1 = word( s, 1 ) ;

	p = 0 ;
	while ( true )
	{
		p = s.find( ',', p ) ;
		if ( p == string::npos ) { break ; }
		s[ p ] = ' ' ;
		p++ ;
	}

	if ( w1.size() == 0 ) { ver_RC = 20 ; return ; }
	else if ( w1.size() == 1 )
	{
		if ( w1 != "(" ) { ver_RC = 20 ; return ; }
		s = subword( s, 2 )  ;
		if ( s[ 0 ] != '&' ) { ver_RC = 20 ; return ; }
		s = substr( s, 2 )  ;
	}
	else
	{
		if ( w1.substr( 0, 2 ) != "(&" ) { ver_RC = 20 ; return ; }
		s = substr( s, 3 ) ;
	}
	if ( s.back() != ')' ) { ver_RC = 20 ; return ; }
	s = substr( s, 1, s.size()-1 ) ;

	ver_field = word( s, 1 ) ;
	if ( !isvalidName( ver_field ) ) { ver_RC = 20 ; return ; }

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
		else if ( substr( w, 1, 4 ) == "MSG=" ) { ver_msgid = substr( w, 5 ) ; }
		else if ( ver_pict ) { if ( ver_value != "" ) { ver_RC = 20 ; return ; } else ver_value = w ; }
		else if ( ver_list ) { ver_value = ver_value + " " + w ; }
		else    { ver_RC = 20 ; return ; }
		i++ ;
	}
	if ( !ver_nblank && !ver_numeric && !ver_list && !ver_pict && !ver_hex && !ver_octal )
	{
		ver_RC = 20 ;
		return      ;
	}
	if ( (ver_list || ver_pict) && ver_value == "" )
	{
		ver_RC = 20 ;
		return      ;
	}
}

TRUNC::TRUNC( string s )
{
	// Format of the TRUNC panel statement
	// &AAA = TRUNC( &BBB,'.' )
	// &AAA = TRUNC( &BBB, 3  )

	int    p ;
	string t ;

	trnc_RC   = 0   ;
	trnc_char = ' ' ;
	trnc_len  = 0   ;

	s = upper( s )    ;
	p = s.find( '=' ) ;
	if ( p == string::npos ) { trnc_RC = 20 ; return ; }
	trnc_field1 = strip( s.substr( 0, p ) ) ;
	if ( trnc_field1 == "" )     { trnc_RC = 20 ; return ; }
	if ( trnc_field1[0] != '&' ) { trnc_RC = 20 ; return ; }
	trnc_field1.erase( 0, 1 ) ;

	s = strip( s.erase( 0, p+1 ) ) ;
	if ( s.substr( 0, 6 ) != "TRUNC(" ) { trnc_RC = 20 ; return ; }
	s = strip( s.erase( 0, 6 ) ) ;
	if ( s == "" ) { trnc_RC = 20 ; return ; }
	p = s.find( ',' ) ;
	if ( p == string::npos ) { trnc_RC = 20 ; return ; }
	trnc_field2 = strip( s.substr( 0, p ) ) ;

	if ( trnc_field2 == "" )     { trnc_RC = 20 ; return ; }
	if ( trnc_field2[0] != '&' ) { trnc_RC = 20 ; return ; }
	trnc_field2.erase( 0, 1 ) ;
	s = strip( s.erase( 0, p+1 ) ) ;
	if ( s == "" ) { trnc_RC = 20 ; return ; }

	if ( s[0] == '\'' )
	{
		if ( s.size() < 4 ) { trnc_RC = 20 ; return ; }
		trnc_char = s[ 1 ] ;
		if ( s[ 2 ] != '\'' ) { trnc_RC = 20 ; return ; }
		s = strip( s.erase( 0, 3 ) ) ;
		if ( s != ")" ) { trnc_RC = 20 ; return ; }
	}
	else
	{
		if ( s.size() < 2 ) { trnc_RC = 20 ; return ; }
		p = s.find( ')' ) ;
		if ( p == string::npos ) { trnc_RC = 20 ; return ; }
		t = strip( s.substr( 0, p ) ) ;
		s = strip( s.erase( 0, p ) )  ;
		if ( !datatype( t, 'W' ) ) { trnc_RC = 20 ; return ; }
		trnc_len = ds2d( t ) ;
		if ( trnc_len <= 0 ) { trnc_RC = 20 ; return ; }
		if ( s != ")" ) { trnc_RC = 20 ; return ; }
	}
	if ( !isvalidName( trnc_field1 ) || !isvalidName( trnc_field2 ) ) { trnc_RC = 20 ; return ; }

	return ;
}


TRANS::TRANS( string s )
{
	// Format of the TRANS panel statement ( change val1 to val2, * is everything else )

	// &AAA = TRANS( &BBB  val1,val2 ...  *,* )
	// &AAA = TRANS( &BBB  val1,val2 ...  *,'?' )
	// &AAA = TRANS( &BBB  val1,val2 ...  ) non-matching results in &AAA being set to null

	int    p  ;
	int    p1 ;
	int    p2 ;
	int    i  ;
	int    j  ;
	string v1 ;
	string v2 ;

	trns_RC   = 0   ;

	s = upper( s )    ;
	p = s.find( '=' ) ;
	if ( p == string::npos ) { trns_RC = 20 ; return ; }
	trns_field1 = strip( s.substr( 0, p ) ) ;
	if ( trns_field1 == "" )     { trns_RC = 20 ; return ; }
	if ( trns_field1[0] != '&' ) { trns_RC = 20 ; return ; }
	trns_field1.erase( 0, 1 ) ;

	s = strip( s.erase( 0, p+1 ) ) ;
	if ( s.substr( 0, 6 ) != "TRANS(" ) { trns_RC = 20 ; return ; }
	s = strip( s.erase( 0, 6 ) ) ;
	if ( s == "" ) { trns_RC = 20 ; return ; }
	p = s.find( ' ' ) ;
	if ( p == string::npos ) { trns_RC = 20 ; return ; }
	trns_field2 = strip( s.substr( 0, p ) ) ;

	if ( trns_field2 == "" )     { trns_RC = 20 ; return ; }
	if ( trns_field2[0] != '&' ) { trns_RC = 20 ; return ; }
	trns_field2.erase( 0, 1 ) ;

	s = strip( s.erase( 0, p+1 ) ) ;
	if ( s == "" ) { trns_RC = 20 ; return ; }
	if ( s.back() != ')' ) { trns_RC = 20 ; return ; }
	s = strip( s.erase( s.size()-1, 1 ) ) ;

	j = countc( s, ',' ) ;
	if ( j == 0 )  { trns_RC = 20 ; return ; }

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
		if ( tlst.find( v1 ) != tlst.end() ) { trns_RC = 20 ; return ; }
		if ( words( v1 ) != 1 ) { trns_RC = 20 ; return ; }
		if ( words( v2 ) != 1 ) { trns_RC = 20 ; return ; }
		tlst[ v1 ] = v2 ;
	}
	if ( strip( s ) != "" ) { trns_RC = 20 ; return ; }
	if ( !isvalidName( trns_field1 ) || !isvalidName( trns_field2 ) ) { trns_RC = 20 ; return ; }
}


pnts::pnts( string s )
{
	// Format of the PNTS panel entry (point-and-shoot entries)
	// FIELD(fld) VAR(var) VAL(value)
	// fld and var must be defined in the panel

	int p1 ;
	int p2 ;

	pnts_RC = 0 ;

	s  = upper( s ) ;

	p1 = pos( "FIELD(", s ) ;
	if ( p1 == 0 ) { pnts_RC = 20 ; return ; }

	p2 = pos( ")", s, p1 ) ;
	if ( p2 == 0 ) { pnts_RC = 20 ; return ; }
	pnts_field = strip( substr( s, (p1 + 6), (p2 - (p1 + 6)) ) ) ;
	if ( !isvalidName( pnts_field ) ) {  pnts_RC = 20 ; return ; }
	s = delstr( s, p1, (p2 - p1 + 1) ) ;

	p1 = pos( "VAR(", s ) ;
	if ( p1 == 0 ) { pnts_RC = 20 ; return ; }

	p2 = pos( ")", s, p1 ) ;
	if ( p2 == 0 ) { pnts_RC = 20 ; return ; }
	pnts_var = strip( substr( s, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
	if ( !isvalidName( pnts_var ) ) {  pnts_RC = 20 ; return ; }
	s = delstr( s, p1, (p2 - p1 + 1) ) ;

	p1 = pos( "VAL(", s ) ;
	if ( p1 == 0 ) { pnts_RC = 20 ; return ; }

	p2 = pos( ")", s, p1 ) ;
	if ( p2 == 0 ) { pnts_RC = 20 ; return ; }
	pnts_val = strip( substr( s, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
	s = strip( delstr( s, p1, (p2 - p1 + 1) ) ) ;

	if ( s != "" || pnts_field == "" || pnts_var == "" || pnts_val == "" ) { pnts_RC = 20 ; return ; }
}


bool selobj::parse( string SELSTR )
{
	// Valid keyword formats:
	// PGM(abc) PARM(xyz) NEWAPPL(abcd) NEWPOOL PASSLIB
	// PGM(abc) PARM(xyz) NEWAPPL NEWPOOL PASSLIB
	// CMD(abc def) LANG(REXX) - translates to PGM(&ZOREXPGM) PARM(abc def)
	// CMD(abc def)            - translates to PGM(&ZOREXPGM) PARM(abc def)
	// PANEL(def)   - translates to PGM(&ZPANLPGM) PARM(def)

	// + SCRNAME(ghi) - give the function a screen name

	// Match brackets for PARM and CMD as these may contain brackets

	int ob ;
	int p1 ;
	int p2 ;

	string lang ;

	PGM     = ""    ;
	PARM    = ""    ;
	NEWAPPL = ""    ;
	NEWPOOL = false ;
	PASSLIB = false ;
	SCRNAME = ""    ;

	p1 = pos( "PARM(", SELSTR ) ;
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
	}

	p1 = pos( "PGM(", SELSTR ) ;
	if ( p1 > 0 )
	{
		p2     = pos( ")", SELSTR, p1 ) ;
		if ( p2 == 0 ) { return false ; }
		PGM    = strip( substr( SELSTR, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
		SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
		if ( PGM[ 0 ] == '&' )
		{
			if ( !isvalidName( substr( PGM, 2 ) ) ) { return false ; }
		}
		else
			if ( !isvalidName( PGM ) ) { return false ; }
	}
	else
	{
		p1 = pos( "PANEL(", SELSTR ) ;
		if ( p1 > 0 )
		{
			if ( PARM != "" ) { return false ; }
			p2 = pos( ")", SELSTR, p1 ) ;
			if ( p2 == 0 ) { return false ; }
			PARM = strip( substr( SELSTR, (p1 + 6), (p2 - (p1 + 6)) ) ) ;
			if ( !isvalidName( PARM ) ) { return false ; }
			PGM    = "&ZPANLPGM"  ;
			SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
			p1 = pos( "OPT(", SELSTR ) ;
			if ( p1 > 0 )
			{
				p2 = pos( ")", SELSTR, p1 ) ;
				if ( p2 == 0 ) { return false ; }
				PARM = PARM + " " + strip( substr( SELSTR, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
				SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
			}
		}
		else
		{
			p1 = pos( "CMD(", SELSTR ) ;
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
				lang   = "" ;
				p1     = pos( "LANG(", SELSTR ) ;
				if ( p1 > 0 )
				{
					p2     = pos( ")", SELSTR, p1 ) ;
					if ( p2 == 0 ) { return false ; }
					lang   = strip( substr( SELSTR, (p1 + 5), (p2 - (p1 + 5)) ) ) ;
					SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
				}
				if ( lang == "" || lang == "REXX" )
				{
					PGM = "&ZOREXPGM"  ;
				}
				else { return false ; }
			}
		}
	}

	p1 = pos( "SCRNAME(", SELSTR ) ;
	if ( p1 > 0 )
	{
		p2      = pos( ")", SELSTR, p1 ) ;
		if ( p2 == 0 ) { return false ; }
		SCRNAME = strip( substr( SELSTR, (p1 + 8), (p2 - (p1 + 8)) ) ) ;
		SELSTR  = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
		if ( !isvalidName( SCRNAME ) ) { return false ; }
	}

	p1 = pos( "NEWAPPL(", SELSTR ) ;
	if ( p1 > 0 )
	{
		p2      = pos( ")", SELSTR, p1 ) ;
		if ( p2 == 0 ) { return false ; }
		NEWAPPL = strip( substr( SELSTR, (p1 + 8), (p2 - (p1 + 8)) ) ) ;
		NEWPOOL = true ;
		SELSTR  = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
		if ( !isvalidName4( NEWAPPL ) ) { return false ; }
	}
	else
	{
		p1 = wordpos( "NEWAPPL", SELSTR ) ;
		if ( p1 > 0 )
		{
			NEWAPPL = "ISP";
			NEWPOOL = true ;
			SELSTR  = delword( SELSTR, p1, 1 ) ;
		}
	}

	p1 = wordpos( "NEWPOOL", SELSTR ) ;
	if ( p1 > 0 )
	{
		NEWPOOL = true ;
		SELSTR  = delword( SELSTR, p1, 1 ) ;
	}

	p1 = wordpos( "PASSLIB", SELSTR ) ;
	if ( p1 > 0 )
	{
		if ( NEWAPPL == "" ) { return false ; }
		PASSLIB = true ;
		SELSTR  = delword( SELSTR, p1, 1 ) ;
	}

	if ( PGM == "" )             { return false ; }
	if ( strip( SELSTR ) != "" ) { return false ; }
	return true ;
}
