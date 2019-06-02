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

bool abbrev( const string& s1, const string& s2 )
{
	// Return true if s2 is an abbreviation of s1, else false

	unsigned int l1 = s1.length() ;
	unsigned int l2 = s2.length() ;

	if ( l1 < l2 ) { return false ; }

	return s1.compare( 0, l2, s2 ) == 0 ;
}



bool abbrev( const string& s1, const string& s2, unsigned int n )
{
	// Return true if s2 an abbreviation of s1 with a minumum length n, else false

	unsigned int l1 = s1.length() ;
	unsigned int l2 = s2.length() ;

	if ( n == 0 )  { return true  ; }
	if ( l2 < n )  { return false ; }
	if ( l1 < l2 ) { return false ; }

	return s1.compare( 0, l2, s2 ) == 0 ;
}



string centre( const string& s, unsigned int n, char c )
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
		return s.substr( j1, n ) ;
	}
}



string center( const string& s, unsigned int n )
{
	return centre( s, n ) ;
}



string copies( const string& s, unsigned int n )
{
	unsigned int l ;
	string t1 ;

	l = s.size()     ;
	t1.resize( n*l ) ;

	for ( unsigned int i = 0 ; i < n ; ++i )
	{
		t1.replace( i*l, l, s ) ;
	}
	return t1 ;
}



bool datatype( const string& s, char type )
{
	// A - only alphanumeric characters (a-z, A-Z, 0-9)
	// L - only lower case alpha characters (a-z)
	// M - only mixed case characters (a-z, A-Z)
	// W - only whole numbers
	// U - only upper case alpha characters (A-Z)

	if ( s == "" ) { return false ; }

	switch ( type )
	{
	case 'A':
		for ( unsigned int i = 0 ; i < s.length() ; ++i )
		{
			if ( !isalnum( s[ i ] ) ) { return false ; }
		}
		break ;

	case 'L':
		for ( unsigned int i = 0 ; i < s.length() ; ++i )
		{
			if ( !islower( s[ i ] ) ) { return false ; }
		}
		break ;

	case 'M':
		for ( unsigned int i = 0 ; i < s.length() ; ++i )
		{
			if ( !islower( s[ i ] ) &&
			     !isupper( s[ i ] ) ) { return false ; }
		}
		break ;

	case 'W':
		for ( unsigned int i = 0 ; i < s.length() ; ++i )
		{
			if ( !isdigit( s[ i ] ) ) { return false ; }
		}
		break ;

	case 'U':
		for ( unsigned int i = 0 ; i < s.length() ; ++i )
		{
			if ( !isupper( s[ i ] ) ) { return false ; }
		}
		break ;

	default:
		return false ;
	}

	return true ;
}



string delstr( string s, unsigned int n )
{
	if ( n > s.length() ) { return s ; }

	return s.erase( n - 1 ) ;
}



string delstr( string s, unsigned int n, unsigned int l )
{
	if ( n > s.length() ) { return s ; }

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
	// Delete all words starting at w.  Keep leading spaces

	size_t i = 0 ;
	size_t j = 0 ;

	for ( ; w > 0 ; --w )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) { return s ; }
		j = s.find( ' ', i ) ;
		if ( j == string::npos )
		{
			if ( w == 1 ) { return s.substr( 0, i ) ; }
			else          { return s                ; }
		}
	}
	return s.erase( i ) ;
}



string delword( string s, unsigned int w, unsigned int n )
{
	// Delete words starting at w for n words.  Keep leading spaces but remove
	// trailing spaces on last word deleted

	size_t i = 0 ;
	size_t j = 0 ;
	size_t k = 0 ;

	if ( n == 0 ) { return s ; }

	for ( ; w > 0 ; --w )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) { return s ; }
		j = s.find( ' ', i ) ;
		if ( j == string::npos )
		{
			if ( w == 1 ) { return s.substr( 0, i ) ; }
			else          { return s                 ; }
		}
	}

	for ( ; n > 0 ; --n )
	{
		k = s.find_first_not_of( ' ', j ) ;
		if ( k == string::npos ) { return s.erase( i ) ; }
		j = s.find( ' ', k ) ;
		if ( j == string::npos ) { return s.erase( i, k-i ) ; }
	}
	return s.erase( i, k-i ) ;
}



string insert( const string& s1, string s2, unsigned int n, char c )
{
	// Insert s1 into s2 at n

	unsigned int l2 = s2.length() ;

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



int lastpos( const string& s1, const string& s2 )
{
	int i ;
	unsigned int l1 = s1.length() ;
	unsigned int l2 = s2.length() ;

	for ( i = l2 - 1 ; i >= 0 ; --i )
	{
		if ( s2.compare( i, l1, s1 ) == 0 ) { return i+1 ; }
	}
	return 0 ;
}



int lastpos( const string& s1, const string& s2, unsigned int p )
{
	int i ;
	unsigned int l1 = s1.length() ;
	unsigned int l2 = s2.length() ;

	if ( p > l2 ) { p = l2 ; }

	for ( i = p - 1 ; i >= 0 ; --i )
	{
		if ( s2.compare( i, l1, s1 ) == 0 ) { return i+1 ; }
	}
	return 0 ;
}



string left( string s, unsigned int n, char c )
{
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



int pos( const string& s1, const string& s2, unsigned int p )
{
	// Find s1 in s2 starting at p returning the offset to the first character

	int i  ;
	int lp ;
	unsigned int l1 = s1.length() ;
	unsigned int l2 = s2.length() ;

	if ( l1 == 0 || l2 == 0 ) { return 0 ; }

	lp = l2 - l1 + 1 ;

	if ( lp < 1 ) { return 0 ; }

	for ( i = p ; i <= lp ; ++i )
	{
		if ( s2.compare( i-1, l1, s1 ) == 0 ) { return i ; }
	}
	return 0 ;
}



string reverse( string s )
{
	reverse( s.begin(), s.end() ) ;
	return s ;
}



string right( string s, unsigned int n, char c )
{
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



string space( const string& s, unsigned int n, char c )
{
	int i ;
	int w ;
	string t ;
	string pad( n, c ) ;

	w = words( s ) ;
	if ( w == 1 ) { return strip( s ) ; }

	t = word( s, 1 ) ;
	for ( i = 2 ; i <= w ; ++i )
	{
		t = t + pad + word( s, i ) ;
	}
	return t ;
}



string strip( string s, char opt, char c )
{
	if ( opt == 'B' || opt == 'L' )
	{
		s.erase( 0, s.find_first_not_of( c ) ) ;
	}
	if ( opt == 'B' || opt == 'T' )
	{
		s.erase( s.find_last_not_of( c ) + 1 ) ;
	}
	return s ;
}



string substr( const string& s, unsigned int n )
{
	if ( n > s.length() ) { return "" ; }
	return s.substr( n - 1 ) ;
}



string substr( const string& s, unsigned int n, unsigned int l, char c )
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



string subword( const string& s, unsigned int w )
{
	size_t i = 0 ;
	size_t j = 0 ;

	for ( ; w > 0 ; --w )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) { return "" ; }
		j = s.find( ' ', i ) ;
		if ( j == string::npos )
		{
			if ( w == 1 ) { return s.substr( i ) ; }
			else          { return ""            ; }
		}
	}
	return strip( s.substr( i ) ) ;
}



string subword( const string& s, unsigned int w, unsigned int n )
{
	size_t i = 0 ;
	size_t j = 0 ;
	size_t k = 0 ;

	for ( ; w > 0 ; --w )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) { return "" ; }
		j = s.find( ' ', i ) ;
		if ( j == string::npos )
		{
			if ( w == 1 ) { return s.substr( i ) ; }
			else          { return ""            ; }
		}
	}

	for ( ; n > 1 ; --n )
	{
		k = s.find_first_not_of( ' ', j ) ;
		if ( k == string::npos ) { return strip( s.substr( i ) ) ; }
		j = s.find( ' ', k ) ;
		if ( j == string::npos ) { return s.substr( i ) ; }
	}
	return strip( s.substr( i, j-i ) ) ;
}



string word( const string& s, unsigned int w )
{
	size_t i = 0 ;
	size_t j = 0 ;

	for ( ; w > 0 ; --w )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) { return "" ; }
		j = s.find( ' ', i ) ;
		if ( j == string::npos )
		{
			if ( w == 1 ) { return s.substr( i ) ; }
			else          { return ""            ; }
		}
	}
	return s.substr( i, j-i ) ;
}



int wordindex( const string& s, unsigned int w )
{
	size_t i = 0 ;
	size_t j = 0 ;

	for ( ; w > 0 ; --w )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) { return 0 ; }
		j = s.find( ' ', i ) ;
		if ( j == string::npos )
		{
			if ( w == 1 ) { break    ; }
			else          { return 0 ; }
		}
	}
	return ++i ;
}



int wordlength( const string& s, unsigned int n )
{
	return word( s, n ).length() ;
}



int wordpos( const string& s1, const string& s2 )
{
	// Returns the word number of the first word in s1 found in s2, where words in s1 are found
	// in string s2 in the same sequence, else return 0
	// The number of spaces in and around words in s1 and s2 is not relevant

	size_t i = 0 ;
	size_t j = 0 ;

	int l  = 0 ;
	int w  = 0 ;
	int sj = 0 ;
	int sl = 0 ;

	uint k = 0 ;

	string s  ;

	unsigned int wds1 = words( s1 ) ;
	unsigned int wds2 ;

	vector<string> ws1 ;

	if ( wds1 == 0 ) { return 0 ; }
	if ( wds1 == 1 )
	{
		s = strip( s1 ) ;
		while ( true )
		{
			i = s2.find_first_not_of( ' ', j ) ;
			if ( i == string::npos ) { return 0 ; }
			j = s2.find( ' ', i ) ;
			++k ;
			if ( j == string::npos )
			{
				if ( s2.compare( i, s2.length()-i, s ) == 0 ) { return k ; }
				else                                          { return 0 ; }
			}
			if ( s2.compare( i, j-i, s ) == 0 ) { return k ; }
		}
	}
	else
	{
		wds2 = words( s2 ) ;
		if ( wds1 > wds2 ) { return 0 ; }
		for ( i = 1 ; i <= wds1 ; ++i )
		{
			ws1.push_back( word( s1, i ) ) ;
		}
		l = ( wds2 - wds1 + 1 ) ;
		while ( true )
		{
			i = s2.find_first_not_of( ' ', j ) ;
			if ( i == string::npos ) { return 0 ; }
			j = s2.find( ' ', i ) ;
			if ( j == string::npos )
			{
				if ( s2.compare( i, s2.length()-1, ws1.at( k ) ) == 0 ) { ++k ; }
				if ( k == wds1 ) { return w ; }
				else             { return 0 ; }
			}
			if ( k == 0 ) { sj = j ; sl = l ; ++w ; }
			if ( s2.compare( i, j-i, ws1.at( k ) ) == 0 ) { ++k   ; }
			else                                          { k = 0 ; }
			if ( k == wds1 ) { return w ; }
			if ( k == 0 )
			{
				j = sj ;
				l = sl - 1 ;
				if ( l < 1 ) { return 0 ; }
			}
		}
	}
}



int words( const string& s )
{
	size_t i = 0 ;
	unsigned int w = 0 ;

	for ( ; ; ++w )
	{
		i = s.find_first_not_of( ' ', i ) ;
		if ( i == string::npos ) { break ; }
		i = s.find( ' ', i ) ;
	}
	return w ;
}


// End of REXX-like functions


string bs2xs( string s )
{
	int l ;

	string reslt = "" ;
	string t     ;

	l = s.length() % CHAR_BIT ;
	if ( l > 0 ) { s = string( CHAR_BIT-l, '0' ) + s ; }

	reslt.reserve( l/4 ) ;

	for ( unsigned int i = 0 ; i < s.length() ; i += 4 )
	{
		t = s.substr( i, 4 ) ;
		if      ( t == "0000" ) { reslt.push_back( '0' ) ; }
		else if ( t == "0001" ) { reslt.push_back( '1' ) ; }
		else if ( t == "0010" ) { reslt.push_back( '2' ) ; }
		else if ( t == "0011" ) { reslt.push_back( '3' ) ; }
		else if ( t == "0100" ) { reslt.push_back( '4' ) ; }
		else if ( t == "0101" ) { reslt.push_back( '5' ) ; }
		else if ( t == "0110" ) { reslt.push_back( '6' ) ; }
		else if ( t == "0111" ) { reslt.push_back( '7' ) ; }
		else if ( t == "1000" ) { reslt.push_back( '8' ) ; }
		else if ( t == "1001" ) { reslt.push_back( '9' ) ; }
		else if ( t == "1010" ) { reslt.push_back( 'A' ) ; }
		else if ( t == "1011" ) { reslt.push_back( 'B' ) ; }
		else if ( t == "1100" ) { reslt.push_back( 'C' ) ; }
		else if ( t == "1101" ) { reslt.push_back( 'D' ) ; }
		else if ( t == "1110" ) { reslt.push_back( 'E' ) ; }
		else if ( t == "1111" ) { reslt.push_back( 'F' ) ; }
		else                    { continue               ; }
	}
	return reslt ;
}



string cs2bs( const string& s )
{
	string reslt ;

	reslt.reserve( 8*s.size() ) ;
	reslt = "" ;
	for ( unsigned int i = 0 ; i < s.size() ; ++i )
	{
		for ( int j = 7; j >= 0; --j )
		{
			( s[ i ] && (1 << j) ) ? reslt.push_back( '1' ) : reslt.push_back( '0' ) ;
		}
	}
	return reslt ;
}



int cs2d( const string& s )
{
	int j ;
	int k ;

	k = 0 ;
	for ( unsigned int i = 0 ; i < s.size() ; ++i )
	{
		j = static_cast< int >( s[ i ] ) ;
		if ( j < 0 ) { j = 256 + j ; }
		k = k + j* pow(256, i ) ;
	}
	return k ;
}



string cs2xs( const string& s )
{
	const char hexdigits[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' } ;

	char x ;
	char y ;

	int  j ;

	string reslt ;

	unsigned int l = s.length()  ;
	reslt = ""          ;
	reslt.resize( 2*l ) ;

	j = 0 ;
	for ( unsigned int i = 0 ; i < l ; ++i )
	{
		x = hexdigits[ s[ i ] >> 4 & 0x0F ] ;
		y = hexdigits[ s[ i ] & 0x0F ] ;
		reslt[ j ] = x ;
		++j ;
		reslt[ j ] = y ;
		++j ;
	}
	return reslt ;
}



string cs2xs( char c )
{
	const char hexdigits[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' } ;
	char x, y ;
	string s ;

	s = "" ;
	x = hexdigits[ c >> 4 & 0x0F ] ;
	y = hexdigits[ c & 0x0F ]      ;
	s.push_back( x ) ;
	s.push_back( y ) ;
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



string d2ds( int i, int j )
{
	ostringstream stream ;
	stream << i          ;
	return right( stream.str(), j, '0' ) ;
}



string d2xs( int i )
{
	ostringstream stream ;
	stream << std::hex << i ;
	return stream.str() ;
}



int ds2d( const string& s )
{
	int i ;

	if ( s == "" ) { return 0 ; }

	istringstream stream ( s ) ;

	stream >> i ;
	return i    ;
}



string xs2bs( const string& s )
{
	string reslt = "" ;

	reslt.reserve( 4*s.size() ) ;

	for ( unsigned int i = 0 ; i < s.size() ; ++i )
	{
		switch ( s[ i ] )
		{
			case '0': reslt += "0000" ; break ;
			case '1': reslt += "0001" ; break ;
			case '2': reslt += "0010" ; break ;
			case '3': reslt += "0011" ; break ;
			case '4': reslt += "0100" ; break ;
			case '5': reslt += "0101" ; break ;
			case '6': reslt += "0110" ; break ;
			case '7': reslt += "0111" ; break ;
			case '8': reslt += "1000" ; break ;
			case '9': reslt += "1001" ; break ;
			case 'a':
			case 'A': reslt += "1010" ; break ;
			case 'b':
			case 'B': reslt += "1011" ; break ;
			case 'c':
			case 'C': reslt += "1100" ; break ;
			case 'd':
			case 'D': reslt += "1101" ; break ;
			case 'e':
			case 'E': reslt += "1110" ; break ;
			case 'f':
			case 'F': reslt += "1111" ; break ;
		}
	}
	return reslt ;
}



string i2bs( uint i )
{
	string reslt = "" ;

	reslt.reserve( CHAR_BIT * sizeof( i ) ) ;

	for ( int j = CHAR_BIT * sizeof( i ) - 1 ; j >= 0 ; --j )
	{
		if ( ( i >> j ) & 1 )
		{
			reslt += "1" ;
		}
		else
		{
			reslt += "0" ;
		}
	}
	return reslt ;
}


string i2xs( uint i )
{
	stringstream ss ;

	ss << setfill( '0' ) << setw( sizeof( i ) * 2 ) << hex << i ;
	return ss.str() ;
}


string xs2cs( const string& s )
{
	int j ;
	int k ;

	string reslt = "" ;
	reslt.reserve( s.size()/2 ) ;

	j = 0 ;
	k = 0 ;
	for ( unsigned int i = 0 ; i < s.size() ; ++i )
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
		if ( ( i % 2 ) == 1 )  { reslt += string( 1, k*16 + j ) ; }
		k = j ;
	}
	return reslt ;
}



int xs2d( const string& s )
{
	int x ;

	stringstream stream;
	stream << std::hex << s ;
	stream >> x ;
	return x ;
}



string& trim_right( string& s )
{
	// Trim spaces on the right.  String modified in-place.
	// Return string& so it can be used in expressions

	return s.erase( s.find_last_not_of( ' ' ) + 1 ) ;
}



string& trim_left( string& s )
{
	// Trim spaces on the left.  String modified in-place.
	// Return string& so it can be used in expressions

	return s.erase( 0, s.find_first_not_of( ' ' ) ) ;
}



string& trim( string& s )
{
	// Trim spaces on the left and right.  String modified in-place.
	// Return string& so it can be used in expressions

	s.erase( 0, s.find_first_not_of( ' ' ) ) ;
	return s.erase( s.find_last_not_of( ' ' ) + 1 ) ;
}


string dquote( errblock& err, char c, string s )
{
	int i ;
	int j ;
	int quotes ;

	string r = "" ;

	string::const_iterator it ;

	trim( s ) ;
	it = s.begin() + 1 ;

	err.setRC( 0 ) ;
	while ( it != s.end() )
	{
		quotes = 0 ;
		while ( it != s.end() && (*it) == c ) { ++quotes ; ++it ; }
		if ( quotes == 0 && it != s.end() )
		{
			r.push_back( (*it) ) ;
			++it ;
			continue ;
		}
		i = quotes / 2 ;
		j = quotes % 2 ;
		r = r + string( i, c ) ;
		if ( j == 1 )
		{
			if ( it != s.end() )
			{
				err.seterrid( "PSYE037G" ) ;
				return "" ;
			}
			break ;
		}
		else if ( j == 0 && it == s.end() )
		{
			err.seterrid( "PSYE033F" ) ;
			return "" ;
		}
	}
	return r ;
}


bool findword( const string& s1, const string& s2 )
{
	// Return true if words in s1 are found in string s2 in the same sequence, else false

	return wordpos( s1, s2 ) != 0 ;
}



int countc( const string& s, char c )
{
	unsigned int l = s.length() ;
	int n = 0 ;

	for ( unsigned int i = 0 ; i < l ; ++i )
	{
		if ( s[ i ] == c ) { ++n ; }
	}
	return n ;
}



string& iupper( string& s )
{
	// Convert to upper case in-place

	for ( unsigned int i = 0 ; i < s.length() ; ++i )
	{
		s[ i ] = toupper( s[ i ] ) ;
	}
	return s ;
}



string& iupper( string& s, unsigned int i, unsigned int e )
{
	// Convert to upper case in-place between range (start pos, end pos)

	if ( e >= s.size() ) { e = s.size() - 1 ; }

	for ( ; i < s.length() && i <= e ; ++i )
	{
		s[ i ] = toupper( s[ i ] ) ;
	}
	return s ;
}



char& iupper( char& c )
{
	// Convert to upper case in-place

	c = toupper( c ) ;
	return c ;
}



string& ilower( string& s )
{
	// Convert to lower case in-place

	for ( unsigned int i = 0 ; i < s.length() ; ++i )
	{
		s[ i ] = tolower( s[ i ] ) ;
	}
	return s ;
}



string& ilower( string& s, unsigned int i, unsigned int e )
{
	// Convert to lower case in-place between range (start pos, end pos)

	if ( e >= s.size() ) { e = s.size() - 1 ; }

	for ( ; i < s.length() && i <= e ; ++i )
	{
		s[ i ] = tolower( s[ i ] ) ;
	}
	return s ;
}



char& ilower( char& c )
{
	// Convert to lower case in-place

	c = tolower( c ) ;
	return c ;
}



string upper( string s )
{
	for ( unsigned int i = 0 ; i < s.length() ; ++i )
	{
		s[ i ] = toupper( s[ i ] ) ;
	}
	return s ;
}



string lower( string s )
{
	for ( unsigned int i = 0 ; i < s.length() ; ++i )
	{
		s[ i ] = tolower( s[ i ] ) ;
	}
	return s ;
}



string& idelstr( string& s, unsigned int n )
{
	if ( n > s.length() ) { return s ; }

	return s.erase( n - 1 ) ;
}



string& idelstr( string& s, unsigned int n, unsigned int l )
{
	if ( n > s.length() ) { return s ; }

	if ( (n + l) > s.length() )
	{
		return s.erase( n - 1 ) ;
	}
	else
	{
		return s.erase( n - 1, l ) ;
	}
}



string& idelword( string& s, unsigned int w )
{
	// Delete word w to end of string in-place and return reference
	// Keep leading spaces on word w, but remove spaces at end of string

	size_t i = 0 ;
	size_t j = 0 ;

	for ( ; w > 0 ; --w )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) { return s ; }
		j = s.find( ' ', i ) ;
		if ( j == string::npos )
		{
			if ( w == 1 ) { return s.erase( i ) ; }
			else          { return s            ; }
		}
	}
	return s.erase( i ) ;
}



string& idelword( string& s, unsigned int w, unsigned int n )
{
	// Delete word w for n words, in-place and return reference
	// Keep leading spaces on word w, but remove trailing spaces on last word

	size_t i = 0 ;
	size_t j = 0 ;
	size_t k = 0 ;

	if ( n == 0 ) { return s ; }

	for ( ; w > 0 ; --w )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) { return s ; }
		j = s.find( ' ', i ) ;
		if ( j == string::npos )
		{
			if ( w == 1 ) { return s.erase( i ) ; }
			else          { return s            ; }
		}
	}

	for ( ; n > 0 ; --n )
	{
		k = s.find_first_not_of( ' ', j ) ;
		if ( k == string::npos ) { return s.erase( i ) ; }
		j = s.find( ' ', k ) ;
		if ( j == string::npos ) { return s.erase( i, k-i ) ; }
	}
	return s.erase( i, k-i ) ;
}



string& istrip( string& s, char opt, char c )
{
	if ( opt == 'B' || opt == 'L' )
	{
		s.erase( 0, s.find_first_not_of( c ) ) ;
	}
	if ( opt == 'B' || opt == 'T' )
	{
		s.erase( s.find_last_not_of( c ) + 1 ) ;
	}
	return s ;
}



bool isnumeric( const string& s )
{
	if ( s.length() == 0 ) { return false ; }

	for ( unsigned int i = 0 ; i < s.length() ; ++i )
	{
		if ( !isdigit( s[ i ] ) ) { return false ; }
	}
	return true ;
}


string d2size( int size )
{
	int div = 0 ;

	float t ;
	stringstream ss ;
	string units[] = { "B", "KB", "MB", "GB", "TB" } ;

	t = size ;

	while ( t >= 1024 && div < 5 )
	{
		t = t / 1024 ;
		++div ;
	}
	ss << fixed << setprecision( 2 ) << t << " " << units[ div ] ;
	return ss.str() ;
}


string hex2print( const string& t )
{
	string s = "" ;

	unsigned int l = t.length() ;

	for ( unsigned int i = 0 ; i < l ; ++i )
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

	size_t j ;

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

	size_t j ;

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


bool isvalidName( const string& s )
{
	unsigned int l = s.length() ;

	if ( l > 8 ) { return false ; }
	if ( l < 1 ) { return false ; }
	if ( isdigit( s[ 0 ] ) ) { return false ; }

	for ( unsigned int i = 0 ; i < l ; ++i )
	{
		if ( (!isdigit( s[ i ] )) &&
		     (!isupper( s[ i ] )) &&
		     (s[ i ] != '#') &&
		     (s[ i ] != '$') &&
		     (s[ i ] != '@') ) { return false ; }
	}
	return true ;
}


bool isvalidName4( const string& s )
{
	unsigned int l = s.length() ;

	if ( l > 4 ) { return false ; }
	if ( l < 1 ) { return false ; }
	if ( isdigit( s[ 0 ] ) ) { return false ; }

	for ( unsigned int i = 0 ; i < l ; ++i )
	{
		if ( (!isdigit( s[ i ] )) &&
		     (!isupper( s[ i ] )) &&
		     (s[ i ] != '#') &&
		     (s[ i ] != '$') &&
		     (s[ i ] != '@') ) { return false ; }
	}
	return true ;
}


bool ishex( const string& s )
{
	unsigned int l = s.length() ;

	if ( (l % 2) != 0 ) { return false ; }
	if ( l < 1 ) { return false ; }

	for ( unsigned int i = 0 ; i < l ; ++i )
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



bool isoctal( const string& s )
{
	unsigned int l = s.length() ;

	if ( l < 1 ) { return false ; }

	for ( unsigned int i = 0 ; i < l ; ++i )
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


bool ispict( const string& s, const string& picts )
{
	// C - any char
	// A - A-Z, a-z, #, $, @
	// N - any numeric character (0-9)
	// 9 - any numeric character (same as N)
	// X - any hexadecimal character (0-9, A-F, a-f)
	// O - any octal char

	unsigned int l1 = s.length()     ;
	unsigned int l2 = picts.length() ;

	if ( l1 != l2 ) { return false ; }

	for ( unsigned int i = 0 ; i < l1 ; ++i )
	{
		switch ( picts[ i ] )
		{
			case 'C': break ;
			case 'A': if ((!isalpha( s[ i ] )) &&
				      (s[ i ] != '#')      &&
				      (s[ i ] != '$')      &&
				      (s[ i ] != '@') ) { return false ; }
				  break ;
			case 'N':
			case '9': if ( !isdigit( s[ i ] ) )   { return false ; }
				  break ;
			case 'X': if ( !ishex( s[ i ] ) )     { return false ; }
				  break ;
			case 'O': if ( !isoctal( s[ i ] ) )   { return false ; }
				  break ;
			default:  if ( picts[ i ] != s[ i ] ) { return false ; }
		}
	}
	return true ;
}



int getpaths( const string& p )
{
	if ( strip( p ) == "" ) { return 0 ; }

	return countc( p, ':' ) + 1 ;
}

string getpath( const string& s, int p )
{
	// Return path p in a concatenation.  Add ending '/' if missing and remove surrounding spaces

	int p1 ;
	int p2 ;

	string path ;

	p1 = 1 ;

	for ( int i = 1 ; i < p ; ++i )
	{
		p1 = pos( ":", s, p1 ) ;
		if ( p1 == 0 ) { return "" ; }
		++p1 ;
	}
	p2 = pos( ":", s, p1 ) ;
	if ( p2 == 0 ) { path = substr( s, p1, s.size()-p1+1 ) ; }
	else           { path = substr( s, p1, p2-p1 )         ; }

	if ( trim( path ) != "" && path.back() != '/' )
	{
		path.push_back( '/' ) ;
	}

	return path ;
}


string mergepaths( const string& p1, const string& p2 )
{
	if      ( p1 == "" ) { return p2 ; }
	else if ( p2 == "" ) { return p1 ; }

	return p1 + ':' + p2 ;
}


string mergepaths( const string& p1, const string& p2, const string& p3 )
{
	return mergepaths( mergepaths( p1, p2 ), p3 ) ;
}


string mergepaths( const string& p1, const char* c1 )
{
	if ( c1 )
	{
		return mergepaths( p1, string( c1 ) ) ;
	}

	return p1 ;
}


string mergepaths( const string& p1, const char* c1, const char* c2 )
{
	return mergepaths( mergepaths( p1, c1 ), c2 ) ;
}


string parseString( errblock& err, string& s, string p )
{
	// Return value of keyword parameter p, or null if not entered
	// Return everything between the brackets
	// Leading and trailing spaces are removed from the parameter value

	// err - errblock to hold any errors
	// s   - entered string (on exit, minus the keyword parameter, p and trimmed)
	// p   - parameter to find (case insensitive)

	int ob ;

	size_t p1 ;
	size_t p2 ;

	string us ;
	string t  ;

	err.setRC( 0 ) ;

	iupper( trim( p ) ) ;

	if ( p.size() == 0 )
	{
		err.seterrid( "PSYE037F" ) ;
		return "" ;
	}

	if ( p.back() != ')' )
	{
		err.seterrid( "PSYE037I", p ) ;
		return "" ;
	}

	p.pop_back() ;
	us = upper( s ) ;

	if ( us.compare( 0, p.size(), p ) == 0 )
	{
		p1 = 0 ;
	}
	else
	{
		p1 = us.find( " " + p ) ;
		if ( p1 == string::npos ) { return "" ; }
		++p1 ;
	}

	ob = 1 ;
	for ( p2 = p1+p.size() ; p2 < s.size() ; ++p2 )
	{
		if ( s.at( p2 ) == '(' ) { ++ob ; }
		if ( s.at( p2 ) == ')' )
		{
			ob-- ;
			if ( ob == 0 ) { break ; }
		}
	}
	if ( ob > 0 )
	{
		err.seterrid( "PSYE032D" ) ;
		return "" ;
	}
	if ( p2 < s.size()-1 && s.at( p2+1 ) != ' ' )
	{
		err.seterrid( "PSYE037H" ) ;
		return "" ;
	}

	t = s.substr( p1+p.size(), p2-p1-p.size() ) ;
	trim( s.erase( p1, p2-p1+1 ) ) ;

	return trim( t ) ;
}


bool parseString1( string& s, string p )
{
	// Return true if keyword p found in string s (keyword does not contain brackets).
	// Remove keyword from string s and trim.

	// s - entered string
	// p - parameter to find (case insensitive)

	size_t p1 ;

	string us ;

	iupper( trim( p ) ) ;

	if ( p.size() == 0 )
	{
		return false ;
	}

	us = upper( s ) ;

	p1 = wordpos( p, us ) ;
	if ( p1 > 0 )
	{
		trim( idelword( s, p1, 1 ) ) ;
		return true ;
	}
	return false ;
}


string& getNameList( errblock& err, string& s )
{
	// return the name list contained in s.  Change in-place if there are no errors, else return original string.
	// err - errblock to hold any errors

	string tmp = s ;

	trim( s ) ;

	bool bracket = false ;

	err.setRC( 0 ) ;

	if ( s == "" ) { return s ; }

	if ( s.front() == '(' )
	{
		if ( s.back() != ')' )
		{
			err.seterrid( "PSYE032D" ) ;
			return s ;
		}
		else
		{
			s = s.substr( 1, s.size() - 2 ) ;
			bracket = true ;
		}
	}
	std::replace( s.begin(), s.end(), ',', ' ' ) ;
	if ( !bracket && words( s ) > 1 )
	{
		err.seterrid( "PSYE039U" ) ;
		s = tmp ;
	}
	return s ;
}


string extractKWord( errblock& err, string& s, string p )
{
	// return value of keyword parameter p, or null if not entered.  Value can be quoted (removed).

	// err - errblock to hold any errors
	// s   - entered string (on exit, minus the keyword parameter, p and trimmed)
	// p   - parameter to find (case insensitive)

	size_t p1 ;
	size_t p2 ;

	char q ;

	bool quote = false ;

	string us ;
	string t  ;

	err.setRC( 0 ) ;

	iupper( trim( p ) ) ;

	if ( p.size() == 0 )
	{
		err.seterrid( "PSYE037F" ) ;
		return "" ;
	}

	us = upper( s ) ;
	if ( p.back() != ')' || p.size() < 3 )
	{
		err.seterrid( "PSYE037I" ) ;
		return "" ;
	}

	p.pop_back() ;
	if ( p.back() != '(' )
	{
		err.seterrid( "PSYE037I" ) ;
		return "" ;
	}

	if ( us.compare( 0, p.size(), p ) == 0 )
	{
		p1 = 0 ;
	}
	else
	{
		p1 = us.find( " " + p ) ;
		if ( p1 == string::npos ) { return "" ; }
		++p1 ;
	}

	p2 = p1 + p.size() ;
	p2 = s.find_first_not_of( ' ', p2 ) ;
	if ( p2 == string::npos )
	{
		err.seterrid( "PSYE037H" ) ;
		return "" ;
	}

	if ( s[ p2 ] == '"' || s[ p2 ] == '\'' )
	{
		quote  = true ;
		q      = s[ p2 ] ;
		++p2 ;
	}

	for ( ; p2 < s.size() ; ++p2 )
	{
		if ( quote && s[ p2 ] == q ) { ++p2 ; break ; }
		if ( quote ) { continue ; }
		if ( s[ p2 ] == ')' ) { break ; }
	}

	p2 = s.find_first_not_of( ' ', p2 ) ;
	if ( quote && p2 == string::npos )
	{
		err.seterrid( "PSYE033F" ) ;
		return "" ;
	}

	if ( p2 == string::npos || s[ p2 ] != ')' )
	{
		err.seterrid( "PSYE037H" ) ;
		return "" ;
	}

	if ( p2 < s.size()-1 && s[ p2+1 ] != ' ' )
	{
		err.seterrid( "PSYE037H" ) ;
		return "" ;
	}

	t = s.substr( p1+p.size(), p2-p1-p.size() ) ;
	trim( s.erase( p1, p2-p1+1 ) ) ;

	trim( t ) ;
	if ( quote )
	{
		t.erase( 0, 1 ) ;
		t.pop_back() ;
	}

	return t ;
}


void qwords( errblock& err, const string& s, vector<string>& v )
{
	// Split a string up into words and place into string vector v.
	// Quotes can be used (single or double) if words contain spaces (quotes are removed)

	size_t i = 0 ;
	size_t j = 0 ;

	char quote ;

	v.clear() ;

	while ( true )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) { return ; }
		if ( s[ i ] == '\'' || s[ i ] == '"' )
		{
			quote = s[ i ] ;
			++i ;
			j = s.find( quote, i ) ;
			if ( j == string::npos )
			{
				err.seterrid( "PSYE033F" ) ;
				return ;
			}
			++j ;
			if ( j < s.size() && s[ j ] != ' ' )
			{
				err.seterrid( "PSYE037G" ) ;
				return ;
			}
			v.push_back( s.substr( i, j-i-1 ) ) ;
		}
		else
		{
			j = s.find( ' ', i ) ;
			if ( j == string::npos )
			{
				v.push_back( s.substr( i ) ) ;
				return ;
			}
			v.push_back( s.substr( i, j-i ) ) ;
		}
	}
}
