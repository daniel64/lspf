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

//////////////////////////////// Utility Functions ///////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// REXX-like string handling functions

bool abbrev( string s1, string s2 )
{
	// s2 an abbreviation of s1
	unsigned int l1 = s1.length() ;
	unsigned int l2 = s2.length() ;

	if ( l1 < l2 ) return false ;

	for ( unsigned int i = 0 ; i < l2 ; i++ )
	{
		if ( s1[ i ] != s2[ i ] ) return false ;
	}
	return true ;
}



bool abbrev( string s1, string s2, unsigned int n )
{
	// s2 an abbreviation of s1 with a minumum length n
	unsigned int l1 = s1.length() ;
	unsigned int l2 = s2.length() ;

	if ( n == 0 )  return true  ;
	if ( l2 < n )  return false ;
	if ( l1 < l2 ) return false ;

	for ( unsigned int i = 0 ; i < l2 ; i++ )
	{
		if ( s1[ i ] != s2[ i ] ) return false ;
	}
	return true ;
}



string centre( string s, unsigned int n, char c )
{
	int j1 ;
	int j2 ;

	unsigned int l = s.length() ;

	if ( n > l )
	{
		j1 = ( n - l ) / 2 ;
		j2 = n - l - j1    ;
		return string( j1, c ) + s + string( j2, c ) ;
	}
	else
	{
		j1 = ( l - n ) / 2 ;
		return s.substr( j1 , n ) ;
	}
}



string center( string s, unsigned int n )
{
	return centre( s, n ) ;
}



string copies( string s, unsigned int n )
{
	string t1 = "" ;

	for ( ; n > 0 ; n-- )
	{
		t1 = t1 + s ;
	}
	return t1 ;
}



bool datatype( string s, char type )
{
	if ( type == 'W' )
	{
		for( unsigned int i = 0 ; i < s.length() ; i++ )
		{
			if ( !isdigit( s[ i ] ) ) return false ;
		}
		return true ;
	}
	return false ;
}



string delstr( string s, unsigned int n )
{
	if ( n > s.length() ) return s ;

	return s.erase( n - 1 ) ;
}



string delstr( string s, unsigned int n, unsigned int l )
{
	if ( n > s.length() ) return s ;

	if ( (n + l) > s.length() )
	{
		return s.erase( n - 1 ) ;
	}
	else
	{
		return s.erase( n - 1, l ) ;
	}
}



string delword( string s, unsigned int w )
{
	int i = 0 ;
	int j = 0 ;

	for ( ; w > 0 ; w-- )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) return s ;
		j = s.find_first_of( ' ', i ) ;
		if ( j == string::npos )
		{
			if ( w == 1 ) return s.substr( 0 , i ) ;
			else return s ;
		}
	}
	return s.erase( i ) ;
}



string delword( string s, unsigned int w, unsigned int n )
{
	int i = 0 ;
	int j = 0 ;
	int k = 0 ;

	if ( n == 0 ) return s ;

	for ( ; w > 0 ; w-- )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) return s ;
		j = s.find_first_of( ' ', i ) ;
		if ( j == string::npos )
		{
			if ( w == 1 ) return s.substr( 0 , i ) ;
			else return s ;
		}
	}

	for ( ; n > 0 ; n-- )
	{
		k = s.find_first_not_of( ' ', j ) ;
		if ( k == string::npos ) return s.erase( i ) ;
		j = s.find_first_of( ' ', k ) ;
		if ( j == string::npos ) return s.erase( i ) ;
	}
	return s.erase( i, k-i ) ;
}



string insert( string s1, string s2, int n, char c )
{
	// Insert s1 into s2 at n
	int l2 = s2.length() ;

	if ( n > l2 )
	{
		return s2 + string( (n - l2 - 1), c ) + s1 ;
	}
	else
	{
		return s2.insert( (n-1), s1 ) ;
	}
}




string insert( string s1, string s2, unsigned int n, unsigned int l, char c )
{
	// Insert s1 into s2 at n for length l and pad with char c
	unsigned int l1 = s1.length() ;
	unsigned int l2 = s2.length() ;

	if ( l1 < l )
	{
		s1 = s1 + string( l - l1, c ) ;
	}
	else if ( l1 > l )
	{
		s1 = s1.substr( 0, l ) ;
	}

	if ( n > l2 )
	{
		return s2 + string( (n - l2 - 1), c ) + s1 ;
	}
	else
	{
		return s2.insert( (n - 1), s1 ) ;
	}
}



int lastpos( string s1, string s2 )
{
	int i ;
	int l1 = s1.length() ;
	int l2 = s2.length() ;

	for ( i = l2 ; i > 0 ; i-- )
	{
		if ( s1 == s2.substr( (i - 1), l1 ) ) return i ;
	}
	return 0 ;
}



int lastpos( string s1, string s2, unsigned int p )
{
	int i ;
	unsigned int l1 = s1.length() ;
	unsigned int l2 = s2.length() ;

	if ( p > l2 ) p = l2 ;

	for ( i = p ; i > 0 ; i-- )
	{
		if ( s1 == s2.substr( (i - 1), l1 ) ) return i ;
	}
	return 0 ;
}



string left( string s, unsigned int n, char c )
{
	s = strip( s , 'L' ) ;
	unsigned int l = s.length() ;

	if ( n > l )
	{
		return s + string( (n - l), c ) ;
	}
	else
	{
		return s.substr( 0, n ) ;
	}
}



int pos( string s1, string s2, unsigned int p )
{
	// Find s1 in s2 starting at p returning the offset to the first character
	int i  ;
	int lp ;
	int l1 = s1.length() ;
	int l2 = s2.length() ;

	if ( l1 == 0 || l2 == 0 ) return 0 ;

	lp = l2 - l1 + 1 ;

	if ( lp < 1 ) return 0 ;

	for ( i = p ; i <= lp ; i++ )
	{
		if ( s1 == s2.substr( i - 1, l1 ) ) return i ;
	}
	return 0 ;
}



string right( string s, unsigned int n, char c )
{
	s = strip( s , 'T' ) ;
	unsigned int l = s.length() ;

	if ( n > l )
	{
		return string( (n - l), c ) + s ;
	}
	else
	{
		return s.substr( l - n ) ;
	}
}



string space( string s, unsigned int n, char c )
{
	int i ;
	int w ;
	string pad( n, c ), t ;

	w = words( s ) ;
	if ( w == 1 ) return word( s , 1 ) ;

	t = word( s , 1 ) ;
	for ( i = 2 ; i <= w ; i++ )
	{
		t = t + pad + word( s, i ) ;
	}
	return t ;
}



string strip( string s, char opt, char c )
{
	int pos ;

	if ( (opt == 'B') || (opt == 'L') )
	{
		pos = s.find_first_not_of( c ) ;
		if ( pos != string::npos ) s = s.substr( pos ) ;
		else return "" ;
	}
	if ( (opt == 'B') || (opt == 'T') )
	{
		pos = s.find_last_not_of( c ) ;
		s   = s.substr( 0, pos+1 ) ;
	}
	return s ;
}



string substr( string s, unsigned int n )
{
	if ( n > s.length() ) return "" ;
	return s.substr( n - 1 ) ;
}



string substr( string s, unsigned int n, unsigned int l, char c )
{
	unsigned int l1 = s.length();

	if ( n > l1 )
	{
		return string( l, c ) ;
	}

	if ( (n + l - 1) > l1 )
	{
		return s.substr( n - 1 ) + string( l - (l1 - n + 1), c ) ;
	}
	else
	{
		return s.substr( (n - 1), l ) ;
	}
}



string subword( string s, unsigned int w )
{
	int i = 0 ;
	int j = 0 ;

	for ( ; w > 0 ; w-- )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) return "" ;
		j = s.find_first_of( ' ', i ) ;
		if ( j == string::npos )
		{
			if ( w == 1 ) return s.substr( i ) ;
			else return "" ;
		}
	}
	return strip( s.substr( i ) ) ;
}




string subword( string s, unsigned int w, unsigned int n )
{
	int i = 0 ;
	int j = 0 ;
	int k = 0 ;

	for ( ; w > 0 ; w-- )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) return "" ;
		j = s.find_first_of( ' ', i ) ;
		if ( j == string::npos )
		{
			if ( w == 1 ) return s.substr( i ) ;
			else return "" ;
		}
	}

	for ( ; n > 1 ; n-- )
	{
		k = s.find_first_not_of( ' ', j ) ;
		if ( k == string::npos ) return strip( s.substr( i ) ) ;

		j = s.find_first_of( ' ', k ) ;
		if ( j == string::npos ) return s.substr( i ) ;
	}
	return strip( s.substr( i , j-i ) ) ;
}



string word( string s, unsigned int w )
{
	int i = 0 ;
	int j = 0 ;

	for ( ; w > 0 ; w-- )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) return "" ;

		j = s.find_first_of( ' ', i ) ;
		if ( j == string::npos )
		{
			if ( w == 1 ) return s.substr( i ) ;
			else return "" ;
		}
	}
	return s.substr( i, j-i ) ;
}



int wordindex( string s, unsigned int w )
{
	int i = 0 ;
	int j = 0 ;

	for ( ; w > 0 ; w-- )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) return 0 ;

		j = s.find_first_of( ' ', i ) ;
		if ( j == string::npos )
		{
			if ( w == 1 ) return  i + 1  ;
			else return 0 ;
		}
	}
	return i + 1;
}



int wordlength( string s, unsigned int n )
{
	return word( s, n ).length() ;
}



int wordpos( string s1, string s2 )
{
	uint wds1 = words( s1 ) ;
	uint wds2 = words( s2 ) ;
	int i = 0 ;
	int j = 0 ;

	if ( wds1 == 0 ) return 0 ;
	if ( wds2 == 0 ) return 0 ;

	for ( i = 1 ; i <= wds2 ; i++ )
	{
		for ( j = 1 ; j <= wds1 ; j++ )
		{
			if ( word( s2, (i + j - 1) ) != word( s1, j ) ) break ;
		}
		if ( j > wds1 ) return i ;
	}
	return 0 ;
}



int words( string s )
{
	int i = 0 ;
	unsigned int w = 0 ;

	for ( ; ; w++ )
	{
		i = s.find_first_not_of( ' ', i ) ;
		if ( i == string::npos ) break ;
		i = s.find_first_of( ' ', i ) ;
	}
	return w ;
}



string bs2xs( string s )
{
	int i ;
	int l ;

	string reslt ;
	string t     ;

	l = s.length() % 8 ;
	if ( l > 0 ) { s = string( 8-l, '0' ) + s ; }

	for ( i = 0 ; i < s.length() ; i += 4 )
	{
		t = s.substr( i, 4 ) ;
		if      ( t == "0000" ) { reslt = reslt + '0' ; }
		else if ( t == "0001" ) { reslt = reslt + '1' ; }
		else if ( t == "0010" ) { reslt = reslt + '2' ; }
		else if ( t == "0011" ) { reslt = reslt + '3' ; }
		else if ( t == "0100" ) { reslt = reslt + '4' ; }
		else if ( t == "0101" ) { reslt = reslt + '5' ; }
		else if ( t == "0110" ) { reslt = reslt + '6' ; }
		else if ( t == "0111" ) { reslt = reslt + '7' ; }
		else if ( t == "1000" ) { reslt = reslt + '8' ; }
		else if ( t == "1001" ) { reslt = reslt + '9' ; }
		else if ( t == "1010" ) { reslt = reslt + 'A' ; }
		else if ( t == "1011" ) { reslt = reslt + 'B' ; }
		else if ( t == "1100" ) { reslt = reslt + 'C' ; }
		else if ( t == "1101" ) { reslt = reslt + 'D' ; }
		else if ( t == "1110" ) { reslt = reslt + 'E' ; }
		else if ( t == "1111" ) { reslt = reslt + 'F' ; }
		else                    { continue            ; }
	}
	return reslt ;
}



string cs2bs( string s )
{
	int i, j ;
	string reslt ;
	
	reslt = "" ;
	for ( i = 0 ; i < s.size() ; i++ )
	{
		for ( j = 7; j >= 0; --j)
		{
			s[ i ] & (1 << j) ? reslt = reslt + "1" : reslt = reslt + "0" ;
		}
	}
	return reslt ;
}



int cs2d( string s )
{
	int i ;
	int j ;
	int k ;

	k = 0 ;
	for ( i = 0 ; i < s.size() ; i++ )
	{
		j = static_cast< int >( s[ i ] ) ;
		if ( j < 0 ) { j = 256 + j ; }
		k = k + j* pow(256, i ) ;
	}
	return k ;
}



string cs2xs( string s )
{
	const char hexdigits[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' } ;
	char x, y ;
	int  i, j ;
	string reslt ;

	int l = s.length()  ;
	reslt = ""          ;
	reslt.resize( 2*l ) ;

	j = 0 ;
	for ( i = 0 ; i < l ; i++ )
	{
		x = hexdigits[ s[ i ] >> 4 & 0x0F ] ;
		y = hexdigits[ s[ i ] & 0x0F ] ;
		reslt.replace( j, 1, 1, x ) ;
		j++ ;
		reslt.replace( j, 1, 1, y ) ;
		j++ ;
	}
	return reslt ;
}



string cs2xs( char c )
{
	const char hexdigits[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' } ;
	char x, y ;
	string s ;

	x = hexdigits[ c >> 4 & 0x0F ] ;
	y = hexdigits[ c & 0x0F ]      ;
	s = x + y ;
	return s ;
}



string d2cs( int i )
{
	ostringstream stream ;
	stream << char( i >> 8 ) ;
	stream << char( i )  ;
	return stream.str()  ;
}



string d2ds( int i )
{
	ostringstream stream ;
	stream << i          ;
	return stream.str()  ;
}



string d2xs( int i )
{
	ostringstream stream ;
	stream << std::hex << i ;
	return stream.str() ;
}



int ds2d( string s )
{
	if ( s == "" ) { return 0 ; }
	istringstream stream ( s ) ;
	int  i      ;

	stream >> i ;
	return i    ;
}



string xs2bs( string s )
{
	int i          ;
	string res("") ;

	for ( i = 0 ; i < s.size() ; i++ )
	{
		switch ( s[ i ] )
		{
			case '0': res = res + "0000" ; break ;
			case '1': res = res + "0001" ; break ;
			case '2': res = res + "0010" ; break ;
			case '3': res = res + "0011" ; break ;
			case '4': res = res + "0100" ; break ;
			case '5': res = res + "0101" ; break ;
			case '6': res = res + "0110" ; break ;
			case '7': res = res + "0111" ; break ;
			case '8': res = res + "1000" ; break ;
			case '9': res = res + "1001" ; break ;
			case 'a': 
			case 'A': res = res + "1010" ; break ;
			case 'b': 
			case 'B': res = res + "1011" ; break ;
			case 'c': 
			case 'C': res = res + "1100" ; break ;
			case 'd': 
			case 'D': res = res + "1101" ; break ;
			case 'e': 
			case 'E': res = res + "1110" ; break ;
			case 'f': 
			case 'F': res = res + "1111" ; break ;
		}
	}
	return res ;
}



string xs2cs( string s )
{
	int i ;
	int j ;
	int k ;

	string reslt("") ;

	for ( i = 0 ; i < s.size() ; i++ )
	{
		switch ( s[ i ] )
		{
			case '0': j = 0  ; break ;
			case '1': j = 1  ; break ;
			case '2': j = 2  ; break ;
			case '3': j = 3  ; break ;
			case '4': j = 4  ; break ;
			case '5': j = 5  ; break ;
			case '6': j = 6  ; break ;
			case '7': j = 7  ; break ;
			case '8': j = 8  ; break ;
			case '9': j = 9  ; break ;
			case 'a': 
			case 'A': j = 10 ; break ;
			case 'b': 
			case 'B': j = 11 ; break ;
			case 'c': 
			case 'C': j = 12 ; break ;
			case 'd': 
			case 'D': j = 13 ; break ;
			case 'e': 
			case 'E': j = 14 ; break ;
			case 'f': 
			case 'F': j = 15 ; break ;
		}
		if ( ( i % 2 ) == 1 )  { reslt = reslt + string( 1, k*16 + j ) ; }
		k = j ;
	}
	return reslt ;
}



int xs2d( string s )
{
	int x ;

	stringstream stream;
	stream << std::hex << s ;
	stream >> x ;
	return x ;
}



// End of REXX-like functions
//

bool findword( string s1, string s2 )
{
	int w ;

	for ( w = words( s2 ) ; w > 0 ; w-- )
	{
		if ( word( s2, w ) == s1 ) return true ;
	}
	return false ;
}



int countc( string s, char c )
{
	int i, l, n ;
	
	n = 0          ;
	l = s.length() ;
	
	for ( i = 0 ; i < l ; i++ )
	{
		if ( s[ i ] == c ) n++ ;
	}
	return n ;
}



bool matchpattern( string s1, string s2)
{
	int l1 = s1.length() - 1 ;
	int l2 = s2.length() - 1 ;
	int i  = 0  ;
	int j  = 0  ;

	for ( ; ; )
	{
		if ( s1[ i ] != '?' )
		{
			if ( s1[ i ] == '*' )
			{
				i++ ;
				if ( i > l1 ) return true ;
				for ( ; j <= l2 ; j++ )
				{
					if ( s1[ i ] == s2[ j ] ) break ;
				}
				if ( j > l2 ) return false ;
			}
			else
			{
				if ( s1[ i ] != s2[ j ] ) return false ;
			}
		}
		i++ ;
		j++ ;
		if ( (i > l1) | (j > l2) ) break ; 
	}
	if ( (i > l1) & (j > l2) ) return true ;
	return false ;
}



string upper( string s )
{
	for( unsigned int i = 0 ; i < s.length() ; i++ )
	{
		s[ i ] = toupper( s[ i ] ) ;
	}
	return s ;
}



string lower( string s )
{
	for( unsigned int i = 0 ; i < s.length() ; i++ )
	{
		s[ i ] = tolower( s[ i ] ) ;
	}
	return s ;
}



bool isnumeric( string s )
{
	if ( s.length() == 0 ) return false ;

	for( unsigned int i = 0 ; i < s.length() ; i++ )
	{
		if ( !isdigit( s[ i ] ) ) return false ;
	}
	return true ;
}


string d2size( int  size )
{                   
	int div = 0 ;

	float t ;
	stringstream ss ;
	string units[] = { "B", "KB", "MB", "GB", "TB" } ;

	t = size ;

	while ( t >= 1024 && div < 5 )
	{
		t = t / 1024 ;
		div++ ;   
	}
	ss << fixed << setprecision( 2 ) << t << " " << units[ div ] ;
	return ss.str() ;
}


string hex2print( string t )
{
	string s ;

	int l = t.length() ;
	s     = ""         ;

	for ( int i = 0 ; i < l ; i++ )
	{
		if ( isprint( t[ i ] ) ) s = s + t[ i ] ;
		else                     s = s + ' '    ;
	}
	return s ;
}


string addCommas( string t )
{
	string nsign("") ;

	int i ;
	int j ;

	if ( t == "" ) { return 0 ; }

	j = t.find( '.' ) ;
	if ( j != string::npos )
	{
		t = t.substr( 0, j ) ;
	}

	if ( t[ 0 ] == '-' ) { t.erase( 0, 1 ) ; nsign = "-" ; }
	for ( i = t.length() - 3 ; i > 0 ; i -= 3 )
	{
		t.insert( i, "," ) ;
	}
	return nsign + t ;
}


string addCommas( string t, int prec )
{
	string s1 ;
	string s2 ;
	string nsign("") ;

	int i ;
	int j ;

	if ( t[ 0 ] == '-' ) { t.erase( 0, 1 ) ; nsign = "-" ; }
	j = t.find( '.' ) ;
	if ( j == string::npos )
	{
		s1 = t ;
		s2 = string( prec, '0' ) ;
	}
	else
	{
		s1 = t.substr( 0, j ) ;
		s2 = t.substr( j+1 )  ;
		s2 = left( s2, prec, '0' ) ;
	}
	
	if ( t == "" ) { return "0." + s2 ; }

	for ( i = s1.length() - 3 ; i > 0 ; i -= 3 )
	{
		s1.insert( i, "," ) ;
	}
	return nsign + s1 + "." + s2 ;
}


bool isvalidName( string s )
{
	int l = s.length() ;

	if ( l > 8 ) return false ;
	if ( l < 1 ) return false ;
	if ( isdigit( s[ 0 ] ) ) return false ;

	for ( int i = 0 ; i < l ; i++ )
	{
		if ( (!isdigit( s[ i ] )) & (!isupper( s[ i ] )) & (s[ i ] != '#') & (s[ i ] != '$') & (s[ i ] != '@') ) return false ;
	}
	return true ;
}


bool isvalidName4( string s )
{
	int l = s.length() ;

	if ( l > 4 ) return false ;
	if ( l < 1 ) return false ;
	if ( isdigit( s[ 0 ] ) ) return false ;
	    
	for ( int i = 0 ; i < l ; i++ )
	{
		if ( (!isdigit( s[ i ] )) & (!isupper( s[ i ] )) & (s[ i ] != '#') & (s[ i ] != '$') & (s[ i ] != '@') ) return false ;
	}
	return true ;
}


bool isvalidHex( string s )
{
	int l = s.length() ;

	if ( (l % 2) != 0 ) return false ;
	if ( l < 1 ) return false ;
	    
	for ( int i = 0 ; i < l ; i++ )
	{
		switch ( s[ i ] )
		{
			case '0': break ;
			case '1': break ;
			case '2': break ;
			case '3': break ;
			case '4': break ;
			case '5': break ;
			case '6': break ;
			case '7': break ;
			case '8': break ;
			case '9': break ;
			case 'a': break ;
			case 'A': break ;
			case 'b': break ;
			case 'B': break ;
			case 'c': break ;
			case 'C': break ;
			case 'd': break ;
			case 'D': break ;
			case 'e': break ;
			case 'E': break ;
			case 'f': break ;
			case 'F': break ;
			default: return false ;
		}
	}
	return true ;
}


bool ishex( char c )
{
	switch ( c )
	{
		case '0': break ;
		case '1': break ;
		case '2': break ;
		case '3': break ;
		case '4': break ;
		case '5': break ;
		case '6': break ;
		case '7': break ;
		case '8': break ;
		case '9': break ;
		case 'a': break ;
		case 'A': break ;
		case 'b': break ;
		case 'B': break ;
		case 'c': break ;
		case 'C': break ;
		case 'd': break ;
		case 'D': break ;
		case 'e': break ;
		case 'E': break ;
		case 'f': break ;
		case 'F': break ;
		default: return false ;
	}
	return true ;
}



bool isoctal( string s )
{
	int l = s.length() ;

	if ( l < 1 ) return false ;
	    
	for ( int i = 0 ; i < l ; i++ )
	{
		switch ( s[ i ] )
		{
			case '0': break ;
			case '1': break ;
			case '2': break ;
			case '3': break ;
			case '4': break ;
			case '5': break ;
			case '6': break ;
			case '7': break ;
			default: return false ;
		}
	}
	return true ;
}


bool isoctal( char c )
{
	switch ( c )
	{
		case '0': break ;
		case '1': break ;
		case '2': break ;
		case '3': break ;
		case '4': break ;
		case '5': break ;
		case '6': break ;
		case '7': break ;
		default: return false ;
	}
	return true ;
}


bool ispict( string s, string picts )
{
	// C - any char
	// A -  (A-Z, a-z, #, $, @)
	// N - any numeric character (0-9)
	// 9 - any numeric character (same as N)
	// X - any hexadecimal character (0-9, A-F, a-f)
	// O - any octal char
	
	int l1 = s.length()     ;
	int l2 = picts.length() ;

	if ( l1 != l2 ) return false ;

	for ( int i = 0 ; i < l1 ; i++ )
	{
		switch ( picts[ i ] )
		{
			case 'C': break ;
			case 'A': if ( (!isdigit( s[ i ] )) & (!isupper( s[ i ] )) & (s[ i ] != '#') & (s[ i ] != '$') & (s[ i ] != '@') ) return false ;
				  break ;
			case 'N': 
			case '9': if ( !isdigit( s[ i ] ) ) { return false ; }
				  break ;
			case 'X': if ( !ishex( s[ i ] ) ) { return false ; }
				  break ; 
			case 'O': if ( !isoctal( s[ i ] ) ) { return false ; }
				  break ;
			default:  if ( picts[ i ] != s[ i ] ) { return false ; }
		}
	}
	return true ;
}



int getpaths( string s )
{
	if ( s == "" ) return 0 ;

	return countc( s, ':' ) + 1 ;
}


string getpath( string s, int p )
{
	// Return path p in a concatenation.  Add ending '/' if missing

	int i  ;
	int p1 ;
	int p2 ;
	string path ;
	
	p1 = 1 ;

	for ( i = 1 ; i < p ; i++ )
	{
		p1 = pos( ":", s, p1 ) ;
		if ( p1 == 0 ) { return "" ; }
		p1++ ;
	}
	p2 = pos( ":", s, p1 ) ;
	if ( p2 == 0 ) { path = substr( s, p1, s.size()-p1+1 ) ; }
	else           { path = substr( s, p1, p2-p1 )         ; }
	if ( path.back() == '/' ) { return path       ; }
	else                      { return path + '/' ; }
}


string mergepaths( string s1, string s2 )
{
	if      ( s1 == "" ) { return s2 ; }
	else if ( s2 == "" ) { return s1 ; }
	else                 { return s1 + ':' + s2 ; }
}


void selectParse( int & RC, string SELSTR, string & PGM, string & PARM, string & NEWAPPL, bool & NEWPOOL, bool & PASSLIB)
{
	// Valid keyword formats:
	// PGM(abc) PARM(xyz) NEWAPPL(abcd) NEWPOOL PASSLIB
	// PGM(abc) PARM(xyz) NEWAPPL NEWPOOL PASSLIB
	// CMD(abc def) - translates to PGM(&ZOREXPGM) PARM(abc def)
	// PANEL(def)   - translates to PGM(&ZPANLPGM) PARM(def)
	
	int p1 ;
	int p2 ;
	string t ;

	RC      = 0     ;
	PGM     = ""    ;
	PARM    = ""    ;
	NEWAPPL = ""    ;
	NEWPOOL = false ;
	PASSLIB = false ;

	p1 = pos( "PARM(", SELSTR ) ;
	if ( p1 > 0 )
	{
		p2     = pos( ")", SELSTR, p1 ) ;
		PARM   = strip( substr( SELSTR, (p1 + 5), (p2 - (p1 + 5)) ) ) ;
		PARM   = strip( PARM, 'B', '"' ) ;
		SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
	}

	p1 = pos( "PGM(", SELSTR ) ;
	if ( p1 > 0 )
	{
		p2     = pos( ")", SELSTR, p1 ) ;
		PGM    = strip( substr( SELSTR, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
		SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
		if ( substr( PGM, 1, 1 ) == "&" )
		{
			if ( !isvalidName( substr( PGM, 2 ) ) ) { RC = 20 ; return ; }
		}
		else
			if ( !isvalidName( PGM ) ) { RC = 20 ; return ; }
	}
	else
	{
		p1 = pos( "PANEL(", SELSTR ) ;
		if ( p1 > 0 )
		{
			if ( PARM != "" ) { RC = 20 ; return ; }
			p2 = pos( ")", SELSTR, p1 ) ;
			PARM = strip( substr( SELSTR, (p1 + 6), (p2 - (p1 + 6)) ) ) ;
			if ( !isvalidName( PARM ) ) { RC = 20 ; return ; }
			PGM    = "&ZPANLPGM"  ;
			SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
		}
		else
		{
			p1 = pos( "CMD(", SELSTR ) ;
			if ( p1 > 0 )
			{
				if ( PARM != "" ) { RC = 20 ; return ; }
				p2 = pos( ")", SELSTR, p1 ) ;
				PARM = strip( substr( SELSTR, (p1 + 4), (p2 - (p1 + 4)) ) ) ;
				PGM    = "&ZOREXPGM"  ;
				SELSTR = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
			}
		}
	}

	p1 = pos( "NEWAPPL(", SELSTR ) ;
	if ( p1 > 0 )
	{
		p2      = pos( ")", SELSTR, p1 ) ;
		NEWAPPL = strip( substr( SELSTR, (p1 + 8), (p2 - (p1 + 8)) ) ) ;
		NEWPOOL = true ;
		SELSTR  = delstr( SELSTR, p1, (p2 - p1 + 1) ) ;
		if ( !isvalidName4( NEWAPPL ) ) { RC = 20 ; return ; }
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
		if ( NEWAPPL == "" ) { RC = 20 ; }
		PASSLIB = true ;
		SELSTR  = delword( SELSTR, p1, 1 ) ;
	}
	
	if ( PGM == "" )             { RC = 20 ; }
	if ( strip( SELSTR ) != "" ) { RC = 20 ; }
	return ;
}


void fieldOptsParse( int & RC, string opts, bool & caps, char & just, bool & numeric, char & padchar, bool & skip )
{
//	CAPS(ON,OFF) 
//	JUST(LEFT,RIGHT,ASIS)
//	NUMERIC(ON,OFF)
//	PAD(char,NULL,USER)
//	SKIP(ON,OFF)
//
	int p1 ;
	int p2 ;
	string t     ;
	string uopts ;
	
	RC = 0 ;
	
	uopts = upper( opts ) ;
	if ( uopts == "NONE" ) return ;
	
	uopts = "," + uopts ;
	p1 = pos( ",CAPS(", uopts ) ;
	if ( p1 > 0 )
	{
		p2 = pos( ")", uopts, p1 ) ;
		t = strip( substr( uopts, (p1 + 6), (p2 - (p1 + 6)) ) ) ;
		if ( t == "ON" ) caps = true ;
		else    if ( t == "OFF" ) caps = false ;
			else { RC = 20 ; return ; }
		uopts = delstr( uopts, p1, (p2 - p1 + 1) ) ;
	}
	
	p1 = pos( ",JUST(", uopts ) ;
	if ( p1 > 0 )
	{
		p2 = pos( ")", uopts, p1 ) ;
		t = strip( substr( uopts, (p1 + 6), (p2 - (p1 + 6)) ) ) ;
		if ( t == "LEFT" ) just = 'L' ;
		else    if ( t == "RIGHT" ) just = 'R' ;
			else    if ( t == "ASIS" ) just = 'A' ;
				else { RC = 20 ; return ; }
		uopts = delstr( uopts, p1, (p2 - p1 + 1) ) ;
	}

	p1 = pos( ",NUMERIC(", uopts ) ;
	if ( p1 > 0 )
	{
		p2 = pos( ")", uopts, p1 ) ;
		t = strip( substr( uopts, (p1 + 9), (p2 - (p1 + 9)) ) ) ;
		if ( t == "ON" ) numeric = true ;
		else    if ( t == "OFF" ) numeric = false ;
			else { RC = 20 ; return ; }
		uopts = delstr( uopts, p1, (p2 - p1 + 1) ) ;
	}

	p1 = pos( ",PAD(", uopts ) ;
	if ( p1 > 0 )
	{
		p2 = pos( ")", uopts, p1 ) ;
		t = strip( substr( uopts, (p1 + 5), (p2 - (p1 + 5)) ) ) ;
		if ( t[ 0 ] == '\'' ) t = strip( t, 'B', '\'' ) ;
		else if ( t[ 0 ] == '"' ) t = strip( t, 'B', '"' ) ;
		if ( t.size() != 1 ) { RC = 20 ; return ; }
		padchar = t[ 0 ] ;
		uopts = delstr( uopts, p1, (p2 - p1 + 1) ) ;
	}

	p1 = pos( ",SKIP(", uopts ) ;
	if ( p1 > 0 )
	{
		p2 = pos( ")", uopts, p1 ) ;
		t = strip( substr( uopts, (p1 + 6), (p2 - (p1 + 6)) ) ) ;
		if ( t == "ON" ) skip = true ;
		else    if ( t == "OFF" ) skip = false ;
			else { RC = 20 ; return ; }
		uopts = delstr( uopts, p1, (p2 - p1 + 1) ) ;
	}
	
	if ( strip( uopts ) != "" ) { RC = 20 ; }
}
