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

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// Utility Functions ///////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// REXX-like string handling functions

namespace lspf {

bool abbrev( const string& s1,
	     const string& s2 )
{
	//
	// Return true if s2 is an abbreviation of s1, else false.
	//

	unsigned int l1 = s1.size() ;
	unsigned int l2 = s2.size() ;

	if ( l1 < l2 ) { return false ; }

	return s1.compare( 0, l2, s2 ) == 0 ;
}



bool abbrev( const string& s1,
	     const string& s2,
	     unsigned int n )
{
	//
	// Return true if s2 an abbreviation of s1 with a minumum length n, else false.
	//

	unsigned int l1 = s1.size() ;
	unsigned int l2 = s2.size() ;

	if ( n == 0 )  { return true  ; }
	if ( l2 < n )  { return false ; }
	if ( l1 < l2 ) { return false ; }

	return s1.compare( 0, l2, s2 ) == 0 ;
}



string centre( const string& s,
	       unsigned int n,
	       char c )
{
	int j1 ;
	int j2 ;

	unsigned int l = s.size() ;

	if ( n > l )
	{
		j1 = ( n - l ) / 2 ;
		j2 = n - l - j1 ;
		return string( j1, c ) + s + string( j2, c ) ;
	}
	else
	{
		j1 = ( l - n ) / 2 ;
		return s.substr( j1, n ) ;
	}
}



string center( const string& s,
	       unsigned int n )
{
	return centre( s, n ) ;
}



string copies( const string& s,
	       unsigned int n )
{
	unsigned int l ;
	string t1 ;

	l = s.size() ;
	t1.resize( n*l ) ;

	for ( unsigned int i = 0 ; i < n ; ++i )
	{
		t1.replace( i*l, l, s ) ;
	}

	return t1 ;
}



string copies( const char c,
	       unsigned int n )
{
	return string( n, c ) ;
}



bool datatype( const string& s,
	       char type )
{
	//
	// A - only alphanumeric characters (a-z, A-Z, 0-9).
	// L - only lower case alpha characters (a-z).
	// M - only mixed case characters (a-z, A-Z).
	// W - only whole numbers, including an optional leading '+' or '-'.
	// U - only upper case alpha characters (A-Z).
	//

	int j = 0 ;

	if ( s == "" ) { return false ; }

	switch ( type )
	{
	case 'A':
		for ( unsigned int i = 0 ; i < s.size() ; ++i )
		{
			if ( !isalnum( s[ i ] ) ) { return false ; }
		}
		break ;

	case 'L':
		for ( unsigned int i = 0 ; i < s.size() ; ++i )
		{
			if ( !islower( s[ i ] ) ) { return false ; }
		}
		break ;

	case 'M':
		for ( unsigned int i = 0 ; i < s.size() ; ++i )
		{
			if ( !std::isalpha( s[ i ] ) ) { return false ; }
		}
		break ;

	case 'W':
		if ( s[ 0 ] == '+' || s[ 0 ] == '-' )
		{
			if ( s.size() == 1 ) { return false ; }
			j = 1 ;
		}
		for ( unsigned int i = j ; i < s.size() ; ++i )
		{
			if ( !std::isdigit( s[ i ] ) ) { return false ; }
		}
		break ;

	case 'U':
		for ( unsigned int i = 0 ; i < s.size() ; ++i )
		{
			if ( !isupper( s[ i ] ) ) { return false ; }
		}
		break ;

	default:
		return false ;
	}

	return true ;
}



string delstr( string s,
	       unsigned int n )
{
	return ( n > s.size() ) ? s : s.erase( n - 1 ) ;
}



string delstr( string s,
	       unsigned int n,
	       unsigned int l )
{
	if ( n > s.size() ) { return s ; }

	if ( ( n + l ) > s.size() )
	{
		return s.erase( n - 1 ) ;
	}
	else
	{
		return s.erase( n - 1, l ) ;
	}
}



string delword( string s,
		unsigned int w )
{
	//
	// Delete all words starting at w.  Keep leading spaces.
	//

	size_t i ;
	size_t j ;

	for ( i = 0, j = 0 ; w > 0 ; --w )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) { return s ; }
		j = s.find( ' ', i ) ;
		if ( j == string::npos )
		{
			return ( w == 1 ) ? s.substr( 0, i ) : s ;
		}
	}

	return s.erase( i ) ;
}



string delword( string s,
		unsigned int w,
		unsigned int n )
{
	//
	// Delete words starting at w for n words.  Keep leading spaces but remove
	// trailing spaces on last word deleted.
	//

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
			else          { return s                ; }
		}
	}

	for ( ; n > 0 ; --n )
	{
		k = s.find_first_not_of( ' ', j ) ;
		if ( k == string::npos ) { return s.erase( i ) ; }
		j = s.find( ' ', k ) ;
		if ( j == string::npos ) { return ( n == 1 ) ? s.erase( i, k-i ) : s.erase( i ) ; }
	}

	return s.erase( i, k-i ) ;
}



string insert( const string& s1,
	       string s2,
	       unsigned int n,
	       char c )
{
	//
	// Insert s1 into s2 at n.
	//

	return ( n > s2.size() ) ? s2 + string( ( n - s2.size() - 1 ), c ) + s1 :
				   s2.insert( ( n - 1 ), s1 ) ;
}



string insert( string s1,
	       string s2,
	       unsigned int n,
	       unsigned int l,
	       char c )
{
	//
	// Insert s1 into s2 at n for length l and pad with char c.
	//

	size_t l1 = s1.size() ;
	size_t l2 = s2.size() ;

	if ( l1 < l )
	{
		s1 += string( l - l1, c ) ;
	}
	else if ( l1 > l )
	{
		s1.resize( l ) ;
	}

	return ( n > l2 ) ? s2 + string( ( n - l2 - 1 ), c ) + s1 :
			    s2.insert( ( n - 1 ), s1 ) ;
}



string justify( const string& s,
		size_t l,
		char c )
{
	//
	// JUSTIFY is a non-SAA function provided by TSO/E.
	//

	int ws ;
	int in ;

	string t = space( s ) ;

	if ( t.size() > l )
	{
		t.resize( l ) ;
		trim( t ) ;
	}

	ws = words( t ) ;

	if ( ws < 2 )
	{
		return left( t, l, c ) ;
	}

	in = ( l + ws - t.size() - 1 ) / ( ws - 1 ) ;

	t = space( t, in ) ;

	for ( int j = 2 ; j <= ws && t.size() < l ; ++j )
	{
		t.insert( ( wordindex( t, j ) - 1 ), 1, c ) ;
	}

	if ( c != ' ' )
	{
		replace( t.begin(), t.end(), ' ', c ) ;
	}

	return t ;
}



int lastpos( const string& s1,
	     const string& s2 )
{
	int i ;
	unsigned int l1 = s1.size() ;
	unsigned int l2 = s2.size() ;

	for ( i = l2 - 1 ; i >= 0 ; --i )
	{
		if ( s2.compare( i, l1, s1 ) == 0 ) { return i + 1 ; }
	}

	return 0 ;
}



int lastpos( const string& s1,
	     const string& s2,
	     unsigned int p )
{
	int i ;
	unsigned int l1 = s1.size() ;
	unsigned int l2 = s2.size() ;

	if ( p > l2 ) { p = l2 ; }

	for ( i = p - 1 ; i >= 0 ; --i )
	{
		if ( s2.compare( i, l1, s1 ) == 0 ) { return i + 1 ; }
	}

	return 0 ;
}



string left( string s,
	     unsigned int n,
	     char c )
{
	unsigned int l = s.size() ;

	return ( n > l ) ? s + string( ( n - l ), c ) :
			   s.substr( 0, n ) ;
}



int pos( const string& s1,
	 const string& s2,
	 unsigned int p )
{
	//
	// Find s1 in s2 starting at p returning the offset to the first character.
	//

	int i  ;
	int lp ;
	unsigned int l1 = s1.size() ;
	unsigned int l2 = s2.size() ;

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



string right( string s,
	      unsigned int n,
	      char c )
{
	return ( n > s.size() ) ? string( ( n - s.size() ), c ) + s :
				  s.substr( s.size() - n ) ;
}



string space( const string& s,
	      unsigned int n,
	      char c )
{
	int i ;
	int w ;

	string t ;
	string pad( n, c ) ;

	w = words( s ) ;
	if ( w == 0 ) { return "" ; }
	if ( w == 1 ) { return word( s, 1 ) ; }

	t = word( s, 1 ) ;
	for ( i = 2 ; i <= w ; ++i )
	{
		t += pad + word( s, i ) ;
	}

	return t ;
}



string strip( string s,
	      char opt,
	      char c )
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



string substr( const string& s,
	       unsigned int n )
{
	return ( n > s.size() ) ? "" : s.substr( n - 1 ) ;
}



string substr( const string& s,
	       unsigned int n,
	       unsigned int l,
	       char c )
{
	unsigned int l1 = s.size();

	if ( n > l1 )
	{
		return string( l, c ) ;
	}

	if ( ( n + l - 1 ) > l1 )
	{
		return s.substr( n - 1 ) + string( l - ( l1 - n + 1 ), c ) ;
	}
	else
	{
		return s.substr( ( n - 1 ), l ) ;
	}
}



string subword( const string& s,
		unsigned int w )
{
	size_t i ;
	size_t j ;

	for ( i = 0, j = 0 ; w > 0 ; --w )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) { return "" ; }
		j = s.find( ' ', i ) ;
		if ( j == string::npos )
		{
			return ( w == 1 ) ? s.substr( i ) : "" ;
		}
	}

	return strip( s.substr( i ) ) ;
}



string subword( const string& s,
		unsigned int w,
		unsigned int n )
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



string translate( const string& s )
{
	//
	// Translate with just one parameter is the same as upper().
	//

	return upper( s ) ;
}



string translate( string s,
		  string otab,
		  const string& itab,
		  char pad )
{
	//
	// Translate characters in string s using the character in otab
	// at the position the character is found in itab.  Pad otab if smaller
	// than itab.
	//

	size_t p ;

	if ( otab.size() < itab.size() )
	{
		otab.resize( itab.size(), pad ) ;
	}

	for ( size_t i = 0 ; i < s.size() ; ++i )
	{
		p = itab.find( s[ i ] ) ;
		if ( p != string::npos )
		{
			s[ i ] = otab[ p ] ;
		}
	}

	return s ;
}



string translate( string s,
		  const char x,
		  const char y )
{
	//
	// Translate characters in string s changing all occurences of y to x.
	//

	std::replace( s.begin(), s.end(), y, x ) ;

	return s ;
}



string word( const string& s,
	     unsigned int w )
{
	size_t i ;
	size_t j ;

	for ( i = 0, j = 0 ; w > 0 ; --w )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) { return "" ; }
		j = s.find( ' ', i ) ;
		if ( j == string::npos )
		{
			return ( w == 1 ) ? s.substr( i ) : "" ;
		}
	}

	return s.substr( i, j-i ) ;
}



int wordindex( const string& s,
	       unsigned int w )
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



int wordlength( const string& s,
		unsigned int n )
{
	return word( s, n ).size() ;
}



int wordpos( const string& s1,
	     const string& s2 )
{
	//
	// Returns the word number of the first word in s1 found in s2, where words in s1 are found
	// in string s2 in the same sequence, else return 0.
	// The number of spaces in and around words in s1 and s2 is not relevant.
	//

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
				if ( s2.compare( i, s2.size()-i, s ) == 0 ) { return k ; }
				else                                        { return 0 ; }
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
				if ( s2.compare( i, s2.size()-1, ws1.at( k ) ) == 0 ) { ++k ; }
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


unsigned int words( const string& s )
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


// End of REXX-like functions.


string bs2xs( string s )
{
	int l = s.size() % CHAR_BIT ;

	string reslt = "" ;
	string t ;

	if ( l > 0 )
	{
		s = string( CHAR_BIT-l, '0' ) + s ;
	}

	reslt.reserve( s.size() / 4 ) ;

	for ( unsigned int i = 0 ; i < s.size() ; i += 4 )
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
	char c ;

	string reslt = "" ;

	reslt.reserve( 8 * s.size() ) ;

	for ( unsigned int i = 0 ; i < s.size() ; ++i )
	{
		c = s[ i ] ;
		for ( int j = 7 ; j >= 0 ; --j )
		{
			( c && ( 1 << j ) ) ? reslt.push_back( '1' ) : reslt.push_back( '0' ) ;
		}
	}

	return reslt ;
}



int cs2d( const string& s )
{
	int j ;
	int k = 0 ;

	for ( unsigned int i = 0 ; i < s.size() ; ++i )
	{
		j = static_cast< int >( s[ i ] ) ;
		if ( j < 0 ) { j = 256 + j ; }
		k = k + j * pow( 256, i ) ;
	}

	return k ;
}



string cs2xs( const string& s )
{
	const char hexdigits[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' } ;

	string reslt = "" ;

	unsigned int l = s.size() ;

	reslt.reserve( 2*l ) ;

	for ( unsigned int i = 0 ; i < l ; ++i )
	{
		reslt.push_back( hexdigits[ s[ i ] >> 4 & 0x0F ] ) ;
		reslt.push_back( hexdigits[ s[ i ] & 0x0F ] ) ;
	}

	return reslt ;
}



string cs2xs1( const string& s,
	       uint i,
	       size_t l )
{
	//
	// Convert first 4-bits of each byte to a string of displayable characters starting from i, length l.
	// (each 4-bits -> 1 displayable character).
	//

	const char hexdigits[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' } ;

	string reslt = "" ;

	l = min( l, s.size() ) ;

	reslt.reserve( l ) ;

	for ( ; i < l ; ++i )
	{
		reslt.push_back( hexdigits[ s[ i ] >> 4 & 0x0F ] ) ;
	}

	return reslt ;
}



string cs2xs2( const string& s,
	       uint i,
	       size_t l )
{
	//
	// Convert last 4-bits of each byte to a string of displayable characters starting from i, length l.
	// (each 4-bits -> 1 displayable character).
	//

	const char hexdigits[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' } ;

	string reslt = "" ;

	l = min( l, s.size() ) ;

	reslt.reserve( l ) ;

	for ( ; i < l ; ++i )
	{
		reslt.push_back( hexdigits[ s[ i ] & 0x0F ] ) ;
	}

	return reslt ;
}



string c2xs( char c )
{
	const char hexdigits[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' } ;

	string s ;

	s.push_back( hexdigits[ c >> 4 & 0x0F ] ) ;
	s.push_back( hexdigits[ c & 0x0F ] ) ;

	return s ;
}



string d2cs( int i )
{
	char c ;

	string t ;

	for ( uint j = 0 ; j < sizeof( int ) ; ++j )
	{
		c = i ;
		t = string( 1, c ) + t ;
		i = i >> 8 ;
	}

	return t ;
}



string d2ds( int i,
	     int j,
	     char c )
{
	ostringstream stream ;
	stream << i ;

	return ( j == 0 ) ? stream.str() : right( stream.str(), j, c ) ;
}



string ui2ds( uint i,
	      int j )
{
	ostringstream stream ;
	stream << i ;

	return ( j == 0 ) ? stream.str() : right( stream.str(), j, '0' ) ;
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

	istringstream stream( s ) ;

	stream >> i ;

	return i ;
}



size_t ds2size_t( const string& s )
{
	size_t i ;

	if ( s == "" ) { return 0 ; }

	istringstream stream( s ) ;

	stream >> i ;

	return i ;
}



ssize_t ds2ssize_t( const string& s )
{
	ssize_t i ;

	if ( s == "" ) { return 0 ; }

	istringstream stream( s ) ;

	stream >> i ;

	return i ;
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
	int j = 0 ;
	int k = 0 ;

	string reslt = "" ;
	reslt.reserve( s.size()/2 ) ;

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
		if ( ( i % 2 ) == 1 ) { reslt += string( 1, k*16 + j ) ; }
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



string addr2str( void* addr )
{
	stringstream stream ;
	stream << addr ;

	return stream.str() ;
}



string strip_right( string s )
{
	//
	// Trim spaces on the right.
	//

	return s.erase( s.find_last_not_of( ' ' ) + 1 ) ;
}



string strip_left( string s )
{
	//
	// Trim spaces on the left.
	//

	return s.erase( 0, s.find_first_not_of( ' ' ) ) ;
}



string& trim_right( string& s )
{
	//
	// Trim spaces on the right.  String modified in-place.
	// Return string& so it can be used in expressions.
	//

	return s.erase( s.find_last_not_of( ' ' ) + 1 ) ;
}



string& trim_left( string& s )
{
	//
	// Trim spaces on the left.  String modified in-place.
	// Return string& so it can be used in expressions.
	//

	return s.erase( 0, s.find_first_not_of( ' ' ) ) ;
}



string& trim( string& s )
{
	//
	// Trim spaces on the left and right.  String modified in-place.
	// Return string& so it can be used in expressions.
	//

	s.erase( 0, s.find_first_not_of( ' ' ) ) ;
	return s.erase( s.find_last_not_of( ' ' ) + 1 ) ;
}


string dquote( errblock& err,
	       char c,
	       string s )
{
	//
	// Replace two quotes with a single quote.
	//

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
		while ( it != s.end() && *it == c ) { ++quotes ; ++it ; }
		if ( quotes == 0 && it != s.end() )
		{
			r.push_back( *it ) ;
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
				err.seterrid( TRACE_INFO(), "PSYE037G" ) ;
				return "" ;
			}
			break ;
		}
		else if ( j == 0 && it == s.end() )
		{
			err.seterrid( TRACE_INFO(), "PSYE033F" ) ;
			return "" ;
		}
	}
	return r ;
}


bool findword( const string& s1,
	       const string& s2 )
{
	//
	// Return true if words in s1 are found in string s2 in the same sequence, else false.
	//

	return wordpos( s1, s2 ) != 0 ;
}



bool findword( const char c1,
	       const string& s2 )
{
	//
	// Return true if word c1 is found in string s2, else false.
	//

	return wordpos( string( 1, c1 ), s2 ) != 0 ;
}



unsigned int countc( const string& s,
		     char c )
{
	unsigned int l = s.size() ;

	int n = 0 ;

	for ( unsigned int i = 0 ; i < l ; ++i )
	{
		if ( s[ i ] == c ) { ++n ; }
	}

	return n ;
}



string& iupper( string& s )
{
	//
	// Convert to upper case in-place.
	//

	for ( unsigned int i = 0 ; i < s.size() ; ++i )
	{
		char& c = s[ i ] ;
		c = toupper( c ) ;
	}

	return s ;
}



string& iupper( string& s,
		unsigned int i,
		unsigned int e )
{
	//
	// Convert to upper case in-place between range (start pos, end pos).
	//

	if ( e >= s.size() ) { e = s.size() - 1 ; }

	for ( ; i < s.size() && i <= e ; ++i )
	{
		char& c = s[ i ] ;
		c = toupper( c ) ;
	}

	return s ;
}



char& iupper( char& c )
{
	//
	// Convert to upper case in-place.
	//

	c = toupper( c ) ;
	return c ;
}



string& iupper1( string& s )
{
	//
	// Convert the first word in string s to upper case in-place.
	//

	size_t p = s.find_first_not_of( ' ' ) ;

	for ( size_t i = p ; i < s.size() && s[ i ] != ' ' ; ++i )
	{
		char& c = s[ i ] ;
		c = toupper( c ) ;
	}

	return s ;
}



string& ilower( string& s )
{
	//
	// Convert to lower case in-place.
	//

	for ( unsigned int i = 0 ; i < s.size() ; ++i )
	{
		char& c = s[ i ] ;
		c = tolower( c ) ;
	}

	return s ;
}



string& ilower( string& s,
		unsigned int i,
		unsigned int e )
{
	//
	// Convert to lower case in-place between range (start pos, end pos).
	//

	if ( e >= s.size() ) { e = s.size() - 1 ; }

	for ( ; i < s.size() && i <= e ; ++i )
	{
		char& c = s[ i ] ;
		c = tolower( c ) ;
	}

	return s ;
}



char& ilower( char& c )
{
	//
	// Convert to lower case in-place.
	//

	c = tolower( c ) ;
	return c ;
}



string upper( string s )
{
	for ( unsigned int i = 0 ; i < s.size() ; ++i )
	{
		char& c = s[ i ] ;
		c = toupper( c ) ;
	}

	return s ;
}



string upper1( string s )
{
	//
	// Convert the first word in string s to upper case.
	//

	size_t p = s.find_first_not_of( ' ' ) ;

	for ( size_t i = p ; i < s.size() && s[ i ] != ' ' ; ++i )
	{
		char& c = s[ i ] ;
		c = toupper( c ) ;
	}

	return s ;
}



string lower( string s )
{
	for ( unsigned int i = 0 ; i < s.size() ; ++i )
	{
		char& c = s[ i ] ;
		c = tolower( c ) ;
	}

	return s ;
}



bool ualpha( const string& s,
	     size_t p )
{
	//
	// Return true if all alpha characters at word p are uppercase.
	//

	size_t p1 ;
	size_t p2 ;

	p1 = s.find_last_of( ' ', p ) ;
	p2 = s.find_first_of( ' ', p ) ;

	if ( p1 == string::npos ) { p1 = 0 ; }
	if ( p2 == string::npos ) { p2 = s.size() - 1 ; }

	for ( ; p1 < p2 ; ++p1 )
	{
		if ( std::isalpha( s[ p1 ] ) && !isupper( s[ p1 ] ) ) { return false ; }
	}

	return true ;
}



bool lalpha( const string& s,
	     size_t p )
{
	//
	// Return true if all alpha characters at word p are lowercase.
	//

	size_t p1 ;
	size_t p2 ;

	p1 = s.find_last_of( ' ', p ) ;
	p2 = s.find_first_of( ' ', p ) ;

	if ( p1 == string::npos ) { p1 = 0 ; }
	if ( p2 == string::npos ) { p2 = s.size() - 1 ; }

	for ( ; p1 < p2 ; ++p1 )
	{
		if ( std::isalpha( s[ p1 ] ) && !islower( s[ p1 ] ) ) { return false ; }
	}

	return true ;
}



string& idelstr( string& s,
		 unsigned int n )
{
	return ( n > s.size() ) ? s : s.erase( n - 1 ) ;
}



string& idelstr( string& s,
		 unsigned int n,
		 unsigned int l )
{
	if ( n > s.size() ) { return s ; }

	if ( ( n + l ) > s.size() )
	{
		return s.erase( n - 1 ) ;
	}
	else
	{
		return s.erase( n - 1, l ) ;
	}
}



string& idelword( string& s,
		  unsigned int w )
{
	//
	// Delete word w to end of string in-place and return reference.
	// Keep leading spaces on word w, but remove spaces at end of string.
	//

	size_t i = 0 ;
	size_t j = 0 ;

	for ( ; w > 0 ; --w )
	{
		i = s.find_first_not_of( ' ', j ) ;
		if ( i == string::npos ) { return s ; }
		j = s.find( ' ', i ) ;
		if ( j == string::npos )
		{
			return ( w == 1 ) ? s.erase( i ) : s ;
		}
	}

	return s.erase( i ) ;
}



string& idelword( string& s,
		  unsigned int w,
		  unsigned int n )
{
	//
	// Delete word w for n words, in-place and return reference.
	// Keep leading spaces on word w, but remove trailing spaces on last word.
	//

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
			return ( w == 1 ) ? s.erase( i ) : s ;
		}
	}

	for ( ; n > 0 ; --n )
	{
		k = s.find_first_not_of( ' ', j ) ;
		if ( k == string::npos ) { return s.erase( i ) ; }
		j = s.find( ' ', k ) ;
		if ( j == string::npos ) { return ( n == 1 ) ? s.erase( i, k-i ) : s.erase( i ) ; }
	}

	return s.erase( i, k-i ) ;
}



string& istrip( string& s,
		char opt,
		char c )
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
	if ( s.size() == 0 ) { return false ; }

	for ( unsigned int i = 0 ; i < s.size() ; ++i )
	{
		if ( !std::isdigit( s[ i ] ) ) { return false ; }
	}

	return true ;
}


string d2size( uint64_t size,
	       int prec )
{
	int div = 0 ;

	double t = size ;

	const string units[] = { "", "K", "M", "G", "T", "P" } ;

	while ( t >= 1024 && div < 5 )
	{
		t /= 1024 ;
		++div ;
	}

	stringstream ss ;
	ss << fixed << setprecision( prec ) << t << " " << units[ div ] ;

	return ss.str() ;
}


string hex2print( const string& t )
{
	string s = "" ;

	unsigned int l = t.size() ;

	for ( unsigned int i = 0 ; i < l ; ++i )
	{
		if ( isprint( t[ i ] ) ) s = s + t[ i ] ;
		else                     s = s + ' '    ;
	}

	return s ;
}


string addCommas( int i )
{
	return addCommas( d2ds( i ) ) ;
}


string addCommas( string t )
{
	string nsign ;

	int i ;

	size_t j ;

	if ( t == "" ) { return 0 ; }

	j = t.find( '.' ) ;
	if ( j != string::npos )
	{
		t = t.substr( 0, j ) ;
	}

	if ( t[ 0 ] == '-' ) { t.erase( 0, 1 ) ; nsign = "-" ; }
	for ( i = t.size() - 3 ; i > 0 ; i -= 3 )
	{
		t.insert( i, "," ) ;
	}

	return nsign + t ;
}


string addCommas( string t,
		  int prec )
{
	string s1 ;
	string s2 ;
	string nsign ;

	int i ;

	size_t j ;

	if ( t[ 0 ] == '-' )
	{
		t.erase( 0, 1 ) ;
		nsign = "-" ;
	}

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

	if ( t == "" )
	{
		return "0." + s2 ;
	}

	for ( i = s1.size() - 3 ; i > 0 ; i -= 3 )
	{
		s1.insert( i, "," ) ;
	}

	return nsign + s1 + "." + s2 ;
}


bool isvalidName( const string& s )
{
	unsigned int l = s.size() ;

	char c ;

	if ( l < 1 || l > 8 ) { return false ; }

	if ( std::isdigit( s[ 0 ] ) ) { return false ; }

	for ( unsigned int i = 0 ; i < l ; ++i )
	{
		c = s[ i ] ;
		if ( !std::isdigit( c ) &&
		     !isupper( c ) &&
		     c != '#' &&
		     c != '$' &&
		     c != '@' ) { return false ; }
	}

	return true ;
}


bool isvalidName4( const string& s )
{
	unsigned int l = s.size() ;

	char c ;

	if ( l < 1 || l > 4 ) { return false ; }

	if ( std::isdigit( s[ 0 ] ) ) { return false ; }

	for ( unsigned int i = 0 ; i < l ; ++i )
	{
		c = s[ i ] ;
		if ( !std::isdigit( c ) &&
		     !isupper( c ) &&
		     c != '#' &&
		     c != '$' &&
		     c != '@' ) { return false ; }
	}

	return true ;
}


bool isalpha( const string& s )
{
	unsigned int l = s.size() ;

	char c ;

	for ( unsigned int i = 0 ; i < l ; ++i )
	{
		c = s[ i ] ;
		if ( !std::isalpha( c ) &&
		     c != '#' &&
		     c != '$' &&
		     c != '@' ) { return false ; }
	}

	return true ;
}


bool isalphab( const string& s )
{
	unsigned int l = s.size() ;

	for ( unsigned int i = 0 ; i < l ; ++i )
	{
		if ( !std::isalpha( s[ i ] ) ) { return false ; }
	}

	return true ;
}


bool isalphanum( const string& s )
{
	unsigned int l = s.size() ;

	char c ;

	for ( unsigned int i = 0 ; i < l ; ++i )
	{
		c = s[ i ] ;
		if ( !std::isalpha( c ) &&
		     !std::isdigit( c ) &&
		     c != '#' &&
		     c != '$' &&
		     c != '@' ) { return false ; }
	}

	return true ;
}


bool isalphabnum( const string& s )
{
	unsigned int l = s.size() ;

	char c ;

	for ( unsigned int i = 0 ; i < l ; ++i )
	{
		c = s[ i ] ;
		if ( !std::isalpha( c ) &&
		     !std::isdigit( c ) ) { return false ; }
	}

	return true ;
}


bool isbit( const string& s )
{
	unsigned int l = s.size() ;

	char c ;

	for ( unsigned int i = 0 ; i < l ; ++i )
	{
		c = s[ i ] ;
		if ( c != '0' && c != '1' ) { return false ; }
	}

	return true ;
}


bool ishex( const string& s )
{
	unsigned int l = s.size() ;

	if ( l < 1 || ( ( l % 2 ) != 0 ) ) { return false ; }

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
	unsigned int l = s.size() ;

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


bool ispict( const string& s,
	     const string& picts )
{
	//
	// Picture string characters:
	// C - any character.
	// A - A-Z, a-z, #, $, @.
	// N - any numeric character (0-9).
	// 9 - any numeric character (same as N).
	// X - any hexadecimal character (0-9, A-F, a-f).
	//

	unsigned int l1 = s.size() ;
	unsigned int l2 = picts.size() ;

	char c ;

	if ( l1 != l2 ) { return false ; }

	for ( unsigned int i = 0 ; i < l1 ; ++i )
	{
		c = s[ i ] ;
		switch ( picts[ i ] )
		{
			case 'c':
			case 'C': break ;

			case 'a':
			case 'A': if ( !std::isalpha( c ) &&
				       c != '#' &&
				       c != '$' &&
				       c != '@' )
				  {
					return false ;
				  }
				  break ;

			case 'n':
			case 'N':
			case '9': if ( !std::isdigit( c ) )
				  {
					return false ;
				  }
				  break ;

			case 'x':
			case 'X': if ( !ishex( c ) )
				  {
					return false ;
				  }
				  break ;

			default:  if ( picts[ i ] != c )
				  {
					return false ;
				  }
		}
	}

	return true ;
}


void ispictcn( errblock& err,
	       const string& str,
	       const char mask_char,
	       string& field_mask,
	       const string& string_picts )
{
	//
	// str          - string to audit.
	// mask_char    - mask character.
	// field_mask   - field mask (non-mask characters are not checked).
	// string_picts - constants and picture string characters.
	//
	// Picture string characters:
	// C - any character.
	// A - A-Z, a-z, #, $, @.
	// N - any numeric character (0-9).
	// 9 - any numeric character (same as N).
	// X - any hexadecimal character (0-9, A-F, a-f).
	//
	// Examples:
	// VER (&fld1,PICTCN,'!','V!!R!!M!!',VNNRNNMNN)
	// matches V10R20M00 but not V10R20M0Y
	//
	// VER (&fld1,PICTCN,*,OS*****,OSNNNAN)
	// matches OS390R8 but not OS39018
	//
	// RC =  0  Match.
	// RC =  8  No match.  Error message set.
	// RC = 20  Severe error.  Error message set.
	//

	unsigned int l1 = str.size() ;
	unsigned int l2 = field_mask.size() ;
	unsigned int l3 = string_picts.size() ;

	char c ;

	err.setRC( 0 ) ;

	if ( l1 != l3 )
	{
		err.seterrid( TRACE_INFO(), "PSYE052L", d2ds( l3 ), string_picts, 8 ) ;
		return ;
	}

	if ( l2 < l3 )
	{
		field_mask.resize( l3, 0x00 ) ;
	}

	for ( unsigned int i = 0 ; i < l3 ; ++i )
	{
		if ( field_mask[ i ] != mask_char )
		{
			c = string_picts[ i ] ;
			if ( c != str[ i ] )
			{
				err.seterrid( TRACE_INFO(), "PSYE052M", d2ds( i+1 ), string( 1, c ), 8 ) ;
				return ;
			}
		}
		else
		{
			c = str[ i ] ;
			switch ( string_picts[ i ] )
			{
				case 'c': break ;
				case 'C': break ;

				case 'a':
				case 'A': if ( !std::isalpha( c ) &&
					       c != '#' &&
					       c != '$' &&
					       c != '@' )
					{
						err.seterrid( TRACE_INFO(), "PSYE052N", d2ds( i+1 ), 8 ) ;
						return ;
					}
					break ;

				case 'n':
				case 'N':
				case '9': if ( !std::isdigit( c ) )
					{
						err.seterrid( TRACE_INFO(), "PSYE052O", d2ds( i+1 ), 8 ) ;
						return ;
					}
					break ;

				case 'x':
				case 'X': if ( !ishex( c ) )
					{
						err.seterrid( TRACE_INFO(), "PSYE052P", d2ds( i+1 ), 8 ) ;
						return ;
					}
					break ;

				default:
					err.seterrid( TRACE_INFO(), "PSYE052Q", 20 ) ;
					return ;
			}
		}
	}
}


void isenum( errblock& err,
	     const string& str )
{
	//
	// Check string 'str' is in ENUM format.
	//
	// RC = 0  String passes verification.
	// RC = 8  Error in verification.  Message id and cursor position set.
	//

	int j = 0 ;
	int k = 0 ;
	int l = 0 ;

	size_t p ;

	size_t n  = 0 ;
	size_t nl = str.size() - 1 ;

	size_t lim1 ;
	size_t lim2 ;

	bool bra1 = false ;
	bool bra2 = false ;
	bool ign1 = false ;
	bool ign2 = false ;
	bool dgts = false ;

	err.setRC( 0 ) ;
	err.setcsrpos( 1 ) ;

	p = str.find_first_not_of( ' ' ) ;

	if ( p == string::npos )
	{
		return ;
	}

	for ( n = p ; n < str.size() ; ++n )
	{
		switch ( str[ n ] )
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				  dgts = true ;
				  break ;

			case '+':
				  if ( n != p )
				  {
					err.seterrid( TRACE_INFO(), "PSYE053A", 8 ) ;
					err.setcsrpos( n + 1 ) ;
					return ;
				  }
				  ign1 = true ;
				  break ;

			case '-':
				  if ( j > 0 || ( n != p && n != nl ) )
				  {
					err.seterrid( TRACE_INFO(), "PSYE053B", 8 ) ;
					err.setcsrpos( n + 1 ) ;
					return ;
				  }
				  ( n == p ) ? ign1 = true : ign2 = true ;
				  ++j ;
				  break ;

			case '.':
				  if ( k > 0 )
				  {
					err.seterrid( TRACE_INFO(), "PSYE053C", 8 ) ;
					err.setcsrpos( n + 1 ) ;
					return ;
				  }
				  ++k ;
				  lim2 = n - 1 ;
				  dgts = false ;
				  break ;

			case ',':
				  if ( k > 0 )
				  {
					err.seterrid( TRACE_INFO(), "PSYE053D", 8 ) ;
					err.setcsrpos( n + 1 ) ;
					return ;
				  }
				  ++l ;
				  break ;

			case '(':
				  if ( n != p )
				  {
					err.seterrid( TRACE_INFO(), "PSYE053E", 8 ) ;
					err.setcsrpos( n + 1 ) ;
					return ;
				  }
				  bra1 = true ;
				  ign1 = true ;
				  break ;

			case ')':
				  if ( !bra1 || n != nl )
				  {
					err.seterrid( TRACE_INFO(), "PSYE053E", 8 ) ;
					err.setcsrpos( n + 1 ) ;
					return ;
				  }
				  bra2 = true ;
				  ign2 = true ;
				  break ;

			default:
				  err.seterrid( TRACE_INFO(), "PSYE053F", 8 ) ;
				  err.setcsrpos( n + 1 ) ;
				  return ;
		}
	}

	if ( !dgts )
	{
		err.seterrid( TRACE_INFO(), "PSYE053G", 8 ) ;
		return ;
	}

	if ( bra1 && !bra2 )
	{
		err.seterrid( TRACE_INFO(), "PSYE053H", 8 ) ;
		err.setcsrpos( nl + 2 ) ;
		return ;
	}

	if ( l > 0 )
	{
		lim1 = ( ign1 ) ? p : p - 1 ;
		if ( k == 0 )
		{
			lim2 = ( ign2 ) ? ( nl - 1 ) : nl ;
		}
		for ( k = 0, n = lim2 ; n != lim1 ; --n )
		{
			if ( std::isdigit( str[ n ] ) )
			{
				++k ;
				if ( k > 3 )
				{
					err.seterrid( TRACE_INFO(), "PSYE053I", 8 ) ;
					err.setcsrpos( n + 2 ) ;
					return ;
				}
			}
			else if ( k == 3 && n > ( lim1 + 1 ) )
			{
				if ( str[ n ] != ',' )
				{
					err.seterrid( TRACE_INFO(), "PSYE053I", 8 ) ;
					err.setcsrpos( n + 2 ) ;
					return ;
				}
				k = 0 ;
			}
			else
			{
				err.seterrid( TRACE_INFO(), "PSYE053I", 8 ) ;
				err.setcsrpos( n + 1 ) ;
				return ;
			}
		}
		lim1 = ( ign1 ) ? p + 1 : p ;
		if ( str.find_first_not_of( "0,", lim1 ) > str.find_first_of( ',' ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE053I", 8 ) ;
			err.setcsrpos( lim1 + 1 ) ;
			return ;
		}
	}
}


int getpaths( const string& p )
{
	return ( strip( p ) == "" ) ? 0 : countc( p, ':' ) + 1 ;
}


string getpath( const string& str,
		int p )
{
	//
	// Return path p in a concatenation.  Add ending '/' if missing and remove surrounding spaces.
	//

	int p1 ;
	int p2 ;

	string path ;

	p1 = 1 ;

	for ( int i = 1 ; i < p ; ++i )
	{
		p1 = pos( ":", str, p1 ) ;
		if ( p1 == 0 ) { return "" ; }
		++p1 ;
	}

	p2 = pos( ":", str, p1 ) ;

	path = ( p2 == 0 ) ? substr( str, p1, str.size()-p1+1 ) : path = substr( str, p1, p2-p1 ) ;

	if ( trim( path ) != "" && path.back() != '/' )
	{
		path.push_back( '/' ) ;
	}

	return path ;
}


string mergepaths( const string& p1,
		   const string& p2 )
{
	return ( p1 == "" ) ? p2 :
	       ( p2 == "" ) ? p1 : p1 + ':' + p2 ;
}


string mergepaths( const string& p1,
		   const string& p2,
		   const string& p3 )
{
	return mergepaths( mergepaths( p1, p2 ), p3 ) ;
}


string mergepaths( const string& p1,
		   const char* c1 )
{
	return ( c1 ) ? mergepaths( p1, string( c1 ) ) : p1 ;
}


string mergepaths( const string& p1,
		   const char* c1,
		   const char* c2 )
{
	return mergepaths( mergepaths( p1, c1 ), c2 ) ;
}


string parseString1( errblock& err,
		    string& s,
		    string p )
{
	//
	// Return value of keyword parameter p, or null if not entered.
	// Return everything between the brackets.  Brackets within quotes are ignored.
	// Leading and trailing spaces are removed from the parameter value.
	//
	// String can be single quoted.  Two single quotes within quotes results
	// in a single quote.
	//
	// Spaces are allowed between the keyword and the opening bracket.
	//
	// err - errblock to hold any errors.
	// s   - entered string (on exit, minus the keyword parameter, p and trimmed).
	// p   - parameter to find (case insensitive).
	//
	// RC = 0  Normal completion (RSN = 0 Keyword parameter found. RSN = 4 Keyword not found).
	// RC = 20 Severe error.
	//

	int ob ;

	size_t p1 ;
	size_t p2 ;
	size_t p3 ;

	size_t pf ;
	size_t sz ;
	size_t lc ;

	string us ;
	string t  ;

	bool quote = false ;

	iupper( trim( p ) ) ;

	if ( p == "()" )
	{
		return parseString3( err, s ) ;
	}

	err.setRC( 0 ) ;
	err.setRSN( 0 ) ;

	trim( s ) ;

	if ( p.size() == 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE037F" ) ;
		return "" ;
	}

	if ( s.size() == 0 || p.size() > s.size() )
	{
		err.setRSN( 4 ) ;
		return "" ;
	}

	if ( p.back() != ')' )
	{
		err.seterrid( TRACE_INFO(), "PSYE037I", p ) ;
		return "" ;
	}

	p.pop_back() ;

	if ( p == "" || p.back() != '(' )
	{
		err.seterrid( TRACE_INFO(), "PSYE037I", p + ")" ) ;
		return "" ;
	}

	p.pop_back() ;

	us = upper( s ) ;
	p1 = string::npos ;
	lc = us.size() - p.size() - 2 ;

	for ( size_t x = 0 ; x <= lc ; ++x )
	{
		if ( quote )
		{
			if ( s[ x ] == '\'' )
			{
				quote = false ;
			}
			continue ;
		}
		else if ( s[ x ] == '\'' )
		{
			quote = true ;
			continue ;
		}
		if ( ( x == 0 || us[ x - 1 ] == ' ' ) && us.compare( x, p.size(), p ) == 0 )
		{
			p2 = us.find_first_not_of( ' ', x + p.size() ) ;
			if ( p2 == string::npos )
			{
				err.setRSN( 4 ) ;
				return "" ;
			}
			else if ( us[ p2 ] == '(' )
			{
				pf = x ;
				p1 = p2 + 1 ;
				sz = p1 - x ;
				break ;
			}
		}
	}

	if ( p1 == string::npos )
	{
		err.setRSN( 4 ) ;
		return "" ;
	}

	ob    = 1 ;
	quote = false ;

	for ( p2 = p1 ; p2 < s.size() ; ++p2 )
	{
		if ( quote )
		{
			if ( s[ p2 ] == '\'' )
			{
				p3 = p2 + 1 ;
				if ( s.size() > p3 && s[ p3 ] == '\'' )
				{
					s.erase( p2, 1 ) ;
				}
				else
				{
					quote = false ;
				}
			}
			continue ;
		}
		else if ( s[ p2 ] == '\'' )
		{
			quote = true ;
			continue ;
		}
		if ( s.at( p2 ) == '(' ) { ++ob ; }
		if ( s.at( p2 ) == ')' )
		{
			ob-- ;
			if ( ob == 0 ) { break ; }
		}
	}

	if ( ob > 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
		return "" ;
	}

	if ( p2 < s.size()-1 && s.at( p2+1 ) != ' ' )
	{
		err.seterrid( TRACE_INFO(), "PSYE037H" ) ;
		return "" ;
	}

	t = s.substr( pf + sz, p2 - pf - sz ) ;
	trim( s.erase( pf, p2 - pf + 1 ) ) ;

	return trim( t ) ;
}


bool parseString2( string& s,
		   string p )
{
	//
	// Return true if keyword p found in string s (keyword does not contain brackets).
	// Remove keyword from string s and trim.
	//
	// s - entered string.
	// p - parameter to find (case insensitive).  Can be a space-separated list.
	//

	int p1 ;
	int ws ;
	int i ;

	string us ;

	iupper( p ) ;

	us = upper( s ) ;

	for ( ws = words( p ), i = 1 ; i <= ws ; ++i )
	{
		p1 = wordpos( word( p, i ), us ) ;
		if ( p1 > 0 )
		{
			trim( idelword( s, p1, 1 ) ) ;
			return true ;
		}
	}

	return false ;
}


string parseString3( errblock& err,
		    string& s )
{
	//
	// Return value between brackets.
	// Return everything between the brackets.  Brackets within quotes are ignored.
	// Leading and trailing spaces are removed from the value.
	//
	// String can be single quoted.  Two single quotes within quotes results
	// in a single quote.
	//
	// err - errblock to hold any errors.
	// s   - entered string (on exit, minus the keyword parameter, p and trimmed).
	//
	// RC = 0  Normal completion (RSN = 0 Keyword parameter found. RSN = 4 Keyword not found).
	// RC = 20 Severe error.
	//

	int ob ;

	size_t p1 ;
	size_t p2 ;

	size_t pf ;

	string t ;

	bool quote = false ;

	err.setRC( 0 ) ;
	err.setRSN( 0 ) ;

	trim( s ) ;

	if ( s.size() < 3 )
	{
		err.setRSN( 4 ) ;
		return "" ;
	}

	pf = string::npos ;

	for ( size_t x = 0 ; x < s.size() ; ++x )
	{
		if ( quote )
		{
			if ( s[ x ] == '\'' )
			{
				quote = false ;
			}
			continue ;
		}
		else if ( s[ x ] == '\'' )
		{
			quote = true ;
			continue ;
		}
		if ( ( x == 0 || s[ x - 1 ] == ' ' ) && s[ x ] == '(' )
		{
			pf = x ;
			break ;
		}
	}

	if ( pf == string::npos )
	{
		err.setRSN( 4 ) ;
		return "" ;
	}

	ob    = 1 ;
	quote = false ;

	for ( p1 = ( pf + 1 ) ; p1 < s.size() ; ++p1 )
	{
		if ( quote )
		{
			if ( s[ p1 ] == '\'' )
			{
				p2 = p1 + 1 ;
				if ( s.size() > p2 && s[ p2 ] == '\'' )
				{
					s.erase( p1, 1 ) ;
					continue ;
				}
				else
				{
					quote = false ;
					continue ;
				}
			}
			else
			{
				continue ;
			}
		}
		else if ( s[ p1 ] == '\'' )
		{
			quote = true ;
			continue ;
		}
		if ( s.at( p1 ) == '(' ) { ++ob ; }
		if ( s.at( p1 ) == ')' )
		{
			ob-- ;
			if ( ob == 0 ) { break ; }
		}
	}

	if ( ob > 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
		return "" ;
	}

	if ( p1 < s.size()-1 && s.at( p1+1 ) != ' ' )
	{
		err.seterrid( TRACE_INFO(), "PSYE037H" ) ;
		return "" ;
	}

	t = s.substr( pf + 1, p1 - pf - 1 ) ;
	trim( s.erase( pf, p1 - pf + 1 ) ) ;

	return trim( t ) ;
}


string& getNameList( errblock& err,
		     string& s )
{
	//
	// return the name list contained in s.  Change in-place if there are no errors, else return original string.
	// err - errblock to hold any errors.
	//

	string tmp = s ;

	trim( s ) ;

	bool bracket = false ;

	err.setRC( 0 ) ;

	if ( s == "" ) { return s ; }

	if ( s.front() == '(' )
	{
		if ( s.back() != ')' )
		{
			err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
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
		err.seterrid( TRACE_INFO(), "PSYE039U" ) ;
		s = tmp ;
	}

	return s ;
}


string getKeywords1( errblock& err,
		     string& s )
{
	//
	// Return all bracketed keywords that appear in the string (not including the brackets).
	// Ignore everything in quotes or brackets.
	//
	// "CMD XYZ A() B() C() DEF" returns "A B C"
	//
	// RC = 0  Normal completion.
	// RC = 20 Severe error.
	//

	int ob = 0 ;

	size_t p1 ;
	size_t p2 ;

	string keyws ;

	bool quote = false ;

	err.setRC( 0 ) ;
	err.setRSN( 0 ) ;

	trim( s ) ;

	if ( s.size() == 0 )
	{
		err.setRSN( 4 ) ;
		return "" ;
	}

	for ( p1 = 0 ; p1 < s.size() ; ++p1 )
	{
		if ( quote )
		{
			if ( s[ p1 ] == '\'' )
			{
				quote = false ;
			}
			continue ;
		}
		else if ( s[ p1 ] == '\'' )
		{
			quote = true ;
			continue ;
		}
		if ( s[ p1 ] == '(' )
		{
			if ( ob == 0 )
			{
				p2 = s.find_last_of( ' ', p1 ) ;
				if ( p2 != string::npos )
				{
					keyws += " " + s.substr( p2+1, p1-p2-1 ) ;
				}
				else if ( p1 > 0 )
				{
					keyws += " " + s.substr( 0, p1 ) ;
				}
			}
			++ob ;
		}
		if ( s[ p1 ] == ')' )
		{
			if ( ob == 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE031J" ) ;
				return "" ;
			}
			--ob ;
		}
	}

	if ( ob > 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
		return "" ;
	}

	return iupper( trim( keyws ) ) ;
}


string getKeywords2( errblock& err,
		     string s )
{
	//
	// Return all non-bracketed keywords that appear in the string.
	// Ignore everything in quotes or brackets.
	//
	// "CMD XYZ A() B() C() DEF GHI" returns "CMD XYZ DEF GHI"
	//
	// RC = 0  Normal completion.
	// RC = 20 Severe error.
	//

	int ob = 0 ;

	size_t p1 ;
	size_t p2 ;

	bool quote = false ;

	err.setRC( 0 ) ;
	err.setRSN( 0 ) ;

	trim( s ) ;

	if ( s.size() == 0 )
	{
		err.setRSN( 4 ) ;
		return "" ;
	}

	for ( p1 = 0 ; p1 < s.size() ; ++p1 )
	{
		if ( quote )
		{
			if ( s[ p1 ] == '\'' )
			{
				quote = false ;
			}
			s[ p1 ] = ' ' ;
			continue ;
		}
		else if ( s[ p1 ] == '\'' )
		{
			s[ p1 ] = ' ' ;
			quote = true ;
			continue ;
		}
		if ( s[ p1 ] == '(' )
		{
			if ( ob == 0 )
			{
				p2 = s.find_last_of( ' ', p1 ) ;
				if ( p2 != string::npos )
				{
					s.replace( p2+1, p1-p2-1, p1-p2-1, ' ' ) ;
				}
				else if ( p1 > 0 )
				{
					s.replace( 0, p1, p1, ' ' ) ;
				}
			}
			s[ p1 ] = ' ' ;
			++ob ;
		}
		if ( s[ p1 ] == ')' )
		{
			if ( ob == 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE031J" ) ;
				return "" ;
			}
			s[ p1 ] = ' ' ;
			--ob ;
		}
		if ( ob > 0 )
		{
			s[ p1 ] = ' ' ;
		}
	}

	if ( ob > 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE031V" ) ;
		return "" ;
	}

	return upper( space( s ) ) ;
}


string extractKWord( errblock& err,
		     string& s,
		     string p )
{
	//
	// return value of keyword parameter p, or null if not entered.  Value can be quoted (single or double - removed).
	//
	// err - errblock to hold any errors.
	// s   - entered string (on exit, minus the keyword parameter, p and trimmed).
	// p   - parameter to find (case insensitive).
	//
	// RC = 0  Normal completion (RSN = 0 Keyword parameter found. RSN = 4 Keyword not found).
	// RC = 20 Severe error.
	//

	size_t p1 ;
	size_t p2 ;
	size_t p3 ;

	char q ;

	bool quote = false ;

	string us ;
	string t  ;

	err.setRC( 0 ) ;
	err.setRSN( 0 ) ;

	iupper( trim( p ) ) ;

	if ( p.size() == 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE037F" ) ;
		return "" ;
	}

	us = upper( s ) ;
	if ( p.back() != ')' || p.size() < 3 )
	{
		err.seterrid( TRACE_INFO(), "PSYE037I" ) ;
		return "" ;
	}

	p.pop_back() ;
	if ( p.back() != '(' )
	{
		err.seterrid( TRACE_INFO(), "PSYE037I" ) ;
		return "" ;
	}

	if ( us.compare( 0, p.size(), p ) == 0 )
	{
		p1 = 0 ;
	}
	else
	{
		p1 = us.find( " " + p ) ;
		if ( p1 == string::npos )
		{
			err.setRSN( 4 ) ;
			return "" ;
		}
		++p1 ;
	}

	p2 = p1 + p.size() ;
	p2 = s.find_first_not_of( ' ', p2 ) ;
	if ( p2 == string::npos )
	{
		err.seterrid( TRACE_INFO(), "PSYE037H" ) ;
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
		if ( quote && s[ p2 ] == q )
		{
			p3 = p2 + 1 ;
			if ( s.size() > p3 && s[ p3 ] == q )
			{
				s.erase( p2, 1 ) ;
				continue ;
			}
			else
			{
				++p2 ;
				break ;
			}
		}
		if ( quote ) { continue ; }
		if ( s[ p2 ] == ')' ) { break ; }
	}

	p2 = s.find_first_not_of( ' ', p2 ) ;
	if ( quote && p2 == string::npos )
	{
		err.seterrid( TRACE_INFO(), "PSYE033F" ) ;
		return "" ;
	}

	if ( p2 == string::npos || s[ p2 ] != ')' )
	{
		err.seterrid( TRACE_INFO(), "PSYE037H" ) ;
		return "" ;
	}

	if ( p2 < s.size()-1 && s[ p2+1 ] != ' ' )
	{
		err.seterrid( TRACE_INFO(), "PSYE037H" ) ;
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


void qwords( errblock& err,
	     const string& s,
	     vector<string>& v )
{
	//
	// Split a string up into words and place into string vector v.
	// Quotes can be used (single or double) if words contain spaces (quotes are removed).
	//

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
				err.seterrid( TRACE_INFO(), "PSYE033F" ) ;
				return ;
			}
			++j ;
			if ( j < s.size() && s[ j ] != ' ' )
			{
				err.seterrid( TRACE_INFO(), "PSYE037G" ) ;
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


string qstring( errblock& err,
		const string& s )
{
	//
	// Remove quotes from string s.  Quotes can be either single or double.
	//
	// If single quotes, call qstring1, that removes quotes and also treats two single quotes
	// as a single quote that is part of the string.
	//
	// String must be quoted if it contains spaces.
	//

	size_t i = 0 ;

	if ( s[ 0 ] == '\'' )
	{
		return qstring1( err, s ) ;
	}
	else if ( s[ 0 ] == '"' )
	{
		i = s.find( '"', 1 ) ;
		if ( i == string::npos )
		{
			err.seterrid( TRACE_INFO(), "PSYE033F" ) ;
			return "" ;
		}
		if ( s.back() != '"' )
		{
			err.seterrid( TRACE_INFO(), "PSYE037G" ) ;
			return "" ;
		}
		return s.substr( 1, i-1 ) ;
	}
	else if ( words( s ) > 1 )
	{
		err.seterrid( TRACE_INFO(), "PSYE037K" ) ;
		return "" ;
	}

	return s ;
}


string qstring1( errblock& err,
		 const string& s )
{
	//
	// Remove quotes from string s.  Quotes are single.
	//
	// Two quotes within the string are reduced to one quote.
	// If string does not start with a single quote, return the string unchanged.
	//

	int i ;
	int quotes ;

	string r ;

	string::const_iterator it ;

	it = s.begin() ;

	if ( s.size() == 0 || *it != '\'' )
	{
		return s ;
	}

	if ( s.size() == 1 )
	{
		err.seterrid( TRACE_INFO(), "PSYE037G" ) ;
		return "" ;
	}

	if ( s.back() != '\'' )
	{
		err.seterrid( TRACE_INFO(), "PSYE033F" ) ;
		return "" ;
	}

	++it ;
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
		r += string( ( quotes / 2 ), '\'' ) ;
		i  = quotes % 2 ;
		if ( i == 1 && it != s.end() )
		{
			err.seterrid( TRACE_INFO(), "PSYE037G" ) ;
			return r ;
		}
		else if ( i == 0 && it == s.end() )
		{
			err.seterrid( TRACE_INFO(), "PSYE033F" ) ;
			return r ;
		}
	}

	return r ;
}


void word( const string& s,
	   vector<string>& v )
{
	//
	// Split a string up into words and place into string vector v.
	// More efficient that using rexx-like words()/word() if all words need to be retrieved.
	//
	// copy( istream_iterator<string>( ss ), istream_iterator<string>(), back_inserter( v ) )
	// is slightly slower than a simple v.push_back() of a temporary string.
	//

	istringstream ss( s ) ;

	v.clear() ;

	string t ;
	while ( ss >> t )
	{
		v.push_back( t ) ;
	}
}


bool check_date( const string& yy,
		 const string& mm,
		 const string& dd )
{
	int iyy ;
	int imm ;
	int idd ;

	if ( !isnumeric( yy ) || !isnumeric( mm ) || !isnumeric( dd ) )
	{
		return false ;
	}

	iyy = ds2d( yy ) ;
	imm = ds2d( mm ) ;
	idd = ds2d( dd ) ;

	if ( iyy <= 9999 )
	{
		switch ( imm )
		{
		case  1:
		case  3:
		case  5:
		case  7:
		case  8:
		case 10:
		case 12:
			return ( idd > 0 && idd <= 31 ) ;

		case  4:
		case  6:
		case  9:
		case 11:
			return ( idd > 0 && idd <= 30 ) ;

		case  2:
			if ( yy.size() == 2 )
			{
				iyy += 2000 ;
			}
			if ( ( iyy % 400 == 0 || ( iyy % 100 != 0 && iyy % 4 == 0 ) ) && idd > 0 && idd <= 29 )
			{
				return true ;
			}
			else if ( idd > 0 && idd <= 28 )
			{
				return true ;
			}
		}
	}

	return false ;
}


bool check_date( const string& yy,
		 const string& ddd )
{
	int iyy ;
	int iddd ;

	if ( !isnumeric( yy ) || !isnumeric( ddd ) )
	{
		return false ;
	}

	iyy  = ds2d( yy ) ;
	iddd = ds2d( ddd ) ;

	if ( yy.size() == 2 )
	{
		iyy += 2000 ;
	}

	if ( iyy <= 9999 )
	{
		if ( ( iyy % 400 == 0 || ( iyy % 100 != 0 && iyy % 4 == 0 ) ) && iddd > 0 && iddd <= 366 )
		{
			return true ;
		}
		else if ( iddd > 0 && iddd <= 365 )
		{
			return true ;
		}
	}

	return false ;
}


string& add_vmask( string& val,
		   const string& zdatef,
		   const string& mask,
		   VEDIT_TYPE vtype )
{
	//
	// FORMAT:
	//   IDATE     disp: zdatef    int: YYMMDD
	//   STDDATE   disp: zdatef    int: YYYYMMDD
	//   ITIME     disp: HH:MM     int: HHMM
	//   STDTIME   disp: HH:MM:SS  int: HHMMSS
	//   JDATE     disp: YY.DDD    int: YYDD
	//   JSTD      disp: YYYY.DDD  int: YYDDDD
	//
	// USER:
	//   Special characters to be added-
	//   ( ) - / , .
	//

	string yy ;
	string mm ;
	string dd ;

	string sep ;

	const string specials = "()-/,." ;

	switch ( vtype )
	{
	case VED_IDATE:
		val.resize( 6, ' ' ) ;
		yy  = val.substr( 0, 2 ) ;
		mm  = val.substr( 2, 2 ) ;
		dd  = val.substr( 4, 2 ) ;
		sep = string( 1, zdatef[ 2 ] ) ;
		if ( zdatef.compare( 0, 2, "DD" ) == 0 )
		{
			val = dd + sep + mm + sep + yy ;
		}
		else
		{
			val = yy + sep + mm + sep + dd ;
		}
		break ;

	case VED_STDDATE:
		val.resize( 8, ' ' ) ;
		yy  = val.substr( 0, 4 ) ;
		mm  = val.substr( 4, 2 ) ;
		dd  = val.substr( 6, 2 ) ;
		sep = string( 1, zdatef[ 2 ] ) ;
		if ( zdatef.compare( 0, 2, "DD" ) == 0 )
		{
			val = dd + sep + mm + sep + yy ;
		}
		else
		{
			val = yy + sep + mm + sep + dd ;
		}
		break ;

	case VED_ITIME:
		val.resize( 4, ' ' ) ;
		val.insert( 2, 1, ':' ) ;
		break ;

	case VED_STDTIME:
		val.resize( 6, ' ' ) ;
		val.insert( 2, 1, ':' ) ;
		val.insert( 5, 1, ':' ) ;
		break ;

	case VED_JDATE:
		val.resize( 5, ' ' ) ;
		val.insert( 2, 1, '.' ) ;
		break ;

	case VED_JSTD:
		val.resize( 7, ' ' ) ;
		val.insert( 4, 1, '.' ) ;
		break ;

	case VED_USER:
		val.resize( mask.size(), ' ' ) ;
		for ( size_t i = 0 ; i < mask.size() ; ++i )
		{
			if ( specials.find( mask[ i ] ) != string::npos )
			{
				val.insert( i, 1, mask[ i ] ) ;
			}
		}
		val.resize( mask.size(), ' ' ) ;
		break ;

	}

	return val ;
}


void remove_vmask( errblock& err,
		   const string& zdatef,
		   const string& name,
		   string& val,
		   const string& mask,
		   VEDIT_TYPE vtype )
{
	//
	// FORMAT:
	//   IDATE       int: YYMMDD    disp: zdatef
	//   STDDATE     int: YYYYMMDD  disp: zdatef
	//   ITIME       int: HHMM      disp: HH:MM
	//   STDTIME     int: HHMMSS    disp: HH:MM:SS
	//   JDATE       int: YYDDD     disp: YY.DDD
	//   JSTD        int: YYYYDDD   disp: YYYY.DDD
	//
	// USER:
	//   A  Any alphabetic character (A-Z, a-z)
	//   B  A blank space
	//   9  Any numeric character (0-9)
	//   H  Any hexadecimal digit (0-9, A-F, a-f)
	//   N  Any numeric or alphabetic character (0-9, A-Z, a-z)
	//   V  Location of the assumed decimal point
	//   S  The numeric data is signed
	//   X  Any allowable characters from the character set of the computer
	//      Special characters to be removed-
	//      ( ) - / , .
	//

	char sep ;

	string yy ;
	string dd ;

	string hh ;
	string mm ;
	string ss ;

	string t ;

	char c ;

	switch ( vtype )
	{
	case VED_IDATE:
		if ( val.size() != 8 )
		{
			err.seterrid( TRACE_INFO(), "PSYE044B", name, 12 ) ;
			return ;
		}
		sep = zdatef[ 2 ] ;
		if ( val[ 2 ] != sep || val[ 5 ] != sep )
		{
			err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
			return ;
		}
		mm = val.substr( 3, 2 ) ;
		if ( zdatef.compare( 0, 2, "DD" ) == 0 )
		{
			dd = val.substr( 0, 2 ) ;
			yy = val.substr( 6, 2 ) ;
		}
		else
		{
			yy = val.substr( 0, 2 ) ;
			dd = val.substr( 6, 2 ) ;
		}
		if ( !check_date( yy, mm, dd ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
			return ;
		}
		val = yy + mm + dd ;
		break ;

	case VED_STDDATE:
		if ( val.size() != 10 )
		{
			err.seterrid( TRACE_INFO(), "PSYE044B", name, 12 ) ;
			return ;
		}
		sep = zdatef[ 2 ] ;
		if ( zdatef.compare( 0, 2, "DD" ) == 0 )
		{
			dd = val.substr( 0, 2 ) ;
			mm = val.substr( 3, 2 ) ;
			yy = val.substr( 6, 4 ) ;
			if ( val[ 2 ] != sep || val[ 5 ] != sep )
			{
				err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
				return ;
			}
		}
		else
		{
			yy = val.substr( 0, 4 ) ;
			mm = val.substr( 5, 2 ) ;
			dd = val.substr( 8, 2 ) ;
			if ( val[ 4 ] != sep || val[ 7 ] != sep )
			{
				err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
				return ;
			}
		}
		if ( !check_date( yy, mm, dd ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
			return ;
		}
		val = yy + mm + dd ;
		break ;

	case VED_ITIME:
		if ( val.size() != 5 )
		{
			err.seterrid( TRACE_INFO(), "PSYE044B", name, 12 ) ;
			return ;
		}
		hh = val.substr( 0, 2 ) ;
		mm = val.substr( 3, 2 ) ;
		if ( val[ 2 ] != ':' || !isnumeric( hh ) || !isnumeric( mm ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
			return ;
		}
		if ( ds2d( hh ) > 23 || ds2d( mm ) > 59 )
		{
			err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
			return ;
		}
		val = hh + mm ;
		break ;

	case VED_STDTIME:
		if ( val.size() != 8 )
		{
			err.seterrid( TRACE_INFO(), "PSYE044B", name, 12 ) ;
			return ;
		}
		hh = val.substr( 0, 2 ) ;
		mm = val.substr( 3, 2 ) ;
		ss = val.substr( 6, 2 ) ;
		if ( val[ 2 ] != ':' || val[ 5 ] != ':' || !isnumeric( hh ) || !isnumeric( mm ) || !isnumeric( ss ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
			return ;
		}
		if ( ds2d( hh ) > 23 || ds2d( mm ) > 59 || ds2d( ss ) > 59 )
		{
			err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
			return ;
		}
		val = hh + mm + ss ;
		break ;

	case VED_JDATE:
		if ( val.size() != 6 )
		{
			err.seterrid( TRACE_INFO(), "PSYE044B", name, 12 ) ;
			return ;
		}
		yy = val.substr( 0, 2 ) ;
		dd = val.substr( 3, 3 ) ;
		if ( val[ 2 ] != '.' || !isnumeric( yy ) || !isnumeric( dd ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
			return ;
		}
		if ( !check_date( yy, dd ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
			return ;
		}
		val = yy + dd ;
		break ;

	case VED_JSTD:
		if ( val.size() != 8 )
		{
			err.seterrid( TRACE_INFO(), "PSYE044B", name, 12 ) ;
			return ;
		}
		yy = val.substr( 0, 4 ) ;
		dd = val.substr( 5, 3 ) ;
		if ( val[ 4 ] != '.' || !isnumeric( yy ) || !isnumeric( dd ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
			return ;
		}
		if ( !check_date( yy, dd ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
			return ;
		}
		val = yy + dd ;
		break ;

	case VED_USER:
		if ( val.size() != mask.size() )
		{
			err.seterrid( TRACE_INFO(), "PSYE044B", name, 12 ) ;
			return ;
		}
		for ( size_t i = 0 ; i < mask.size() ; ++i )
		{
			c = val[ i ] ;
			switch ( mask[ i ] )
			{
			case 'A':
				if ( !std::isalpha( c ) )
				{
					err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
					return ;
				}
				t.push_back( c ) ;
				break ;

			case 'B':
				if ( c != ' ' )
				{
					err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
					return ;
				}
				t.push_back( c ) ;
				break ;

			case 'H':
				if ( !ishex( c ) )
				{
					err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
					return ;
				}
				t.push_back( c ) ;
				break ;

			case 'N':
				if ( !isalnum( c ) )
				{
					err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
					return ;
				}
				t.push_back( c ) ;
				break ;

			case 'V':
				if ( c != '.' )
				{
					err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
					return ;
				}
				t.push_back( c ) ;
				break ;

			case 'S':
				if ( c != '+' && c != '-' )
				{
					err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
					return ;
				}
				t.push_back( c ) ;
				break ;

			case 'X':
				if ( !isprint( c ) )
				{
					err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
					return ;
				}
				t.push_back( c ) ;
				break ;

			case '9':
				if ( !std::isdigit( c ) )
				{
					err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
					return ;
				}
				t.push_back( c ) ;
				break ;

			case '(':
			case ')':
			case '/':
			case '-':
			case ',':
			case '.':
				if ( mask[ i ] != c )
				{
					err.seterrid( TRACE_INFO(), "PSYE044C", name, 12 ) ;
					return ;
				}
				break ;

			}
		}
		val = t ;
	}
}


bool namesNotValid( string& w,
		    const string& n1,
		    const string& n2,
		    const string& n3,
		    const string& n4,
		    const string& n5 )
{
	//
	// Check a series of names for validity.
	// Return true if any errors found and place the name in error in w.
	//

	if ( n1 != "" && !isvalidName( n1 ) )
	{
		w = n1 ;
		return true ;
	}

	if ( n2 != "" && !isvalidName( n2 ) )
	{
		w = n2 ;
		return true ;
	}

	if ( n3 != "" && !isvalidName( n3 ) )
	{
		w = n3 ;
		return true ;
	}

	if ( n4 != "" && !isvalidName( n4 ) )
	{
		w = n4 ;
		return true ;
	}

	if ( n5 != "" && !isvalidName( n5 ) )
	{
		w = n5 ;
		return true ;
	}

	return false ;
}


bool allNumeric( const string& n )
{
	//
	// Check a string for all valid numeric values.
	//

	int ws = words( n ) ;

	for ( int i = 1 ; i <= ws ; ++i )
	{
		if ( !datatype( word( n, i ), 'W' ) )
		{
			return false ;
		}
	}


	return true ;
}


bool ibrackets( const string& s,
		char c1 )
{
	//
	// Return true if there is an imbalance of brackets, else false.
	// Brackets inside quotes are not counted (single quotes only).
	//

	int ob = 0 ;

	bool oquote = false ;

	char ch ;
	char c2 = ( c1 == ')' ) ? '(' : ')' ;

	for ( uint i = 0 ; i < s.size() ; ++i )
	{
		ch = s[ i ] ;
		if ( oquote )
		{
			if ( ch == '\'' )
			{
				oquote = false ;
			}
		}
		else if ( ch == '\'' )
		{
			oquote = true ;
		}
		else if ( ch == c1 )
		{
			++ob ;
		}
		else if ( ch == c2 )
		{
			--ob ;
		}
	}

	return ( ob > 0 ) ;
}


bool suffix( const string& s1,
	     const string& s2 )
{
	//
	// Return true if s2 is a suffix of s1.
	//

	size_t l1 = s1.size() ;
	size_t l2 = s2.size() ;

	if ( l2 > l1 ) { return false ; }

	return ( s1.compare( l1 - l2, l2, s2 ) == 0 ) ;
}


bool prefix( const string& s1,
	     const string& s2 )
{
	//
	// Return true if s2 is a prefix of s1.
	//

	size_t l1 = s1.size() ;
	size_t l2 = s2.size() ;

	if ( l2 > l1 ) { return false ; }

	return ( s1.compare( 0, l2, s2 ) == 0 ) ;
}


void rem_comments( string& s )
{
	//
	// Remove /* ... */ comments from a string in place by replacing with spaces
	// or erasing to EOL if no ending comment.
	// It isn't a comment if it is inside single quotes.
	//

	bool quote = false ;

	size_t l = s.size() ;

	size_t i ;
	size_t j ;
	size_t n ;

	for ( i = 0 ; i < l ; ++i )
	{
		if ( s[ i ] == '\'' )
		{
			quote = !quote ;
		}
		else if ( !quote && s.compare( i, 2, "/*" ) == 0 )
		{
			j = s.find( "*/", i+2 ) ;
			if ( j == string::npos )
			{
				s.erase( i ) ;
				break ;
			}
			else
			{
				n = j - i + 2 ;
				s.replace( i, n, n, ' ' ) ;
				i = j + 1 ;
			}
		}
	}
}


size_t findnq( const string& s,
	       char c,
	       size_t p )
{
	//
	// Find char c in string s ignoring single quotes.
	//

	bool quote = false ;

	size_t i ;
	size_t l = s.size() ;

	for ( i = p ; i < l ; ++i )
	{
		if ( s[ i ] == '\'' )
		{
			quote = !quote ;
		}
		else if ( !quote && s[ i ] == c )
		{
			return i ;
		}
	}

	return string::npos ;
}


size_t findnq( const string& s1,
	       const string& s2,
	       size_t p )
{
	//
	// Find string s2 in string s1 ignoring single quotes.
	//

	bool quote = false ;

	size_t i ;

	size_t l1 = s1.size() ;
	size_t l2 = s2.size() ;

	size_t l3 ;

	if ( l2 > l1 )
	{
		return string::npos ;
	}

	l3 = l1 - l2 ;

	for ( i = p ; i <= l3 ; ++i )
	{
		if ( s1[ i ] == '\'' )
		{
			quote = !quote ;
		}
		else if ( !quote && s1.compare( i, l2, s2 ) == 0 )
		{
			return i ;
		}
	}

	return string::npos ;
}


string conv_regex( const string& s,
		   char c1,
		   char c2 )
{
	//
	// Convert non-regex string 's' to a regex pattern.
	// c1 is the char representing any single non-blank character.
	// c2 is the char representing any number of non-blank characters, including 0.
	//
	// Reduce multiple c2's to a single c2 as this can cause REGEX to fail.
	//

	bool mc2 = false ;

	string pat ;

	for ( size_t i = 0 ; i < s.size() ; ++i )
	{
		char c = s[ i ] ;
		if ( mc2 && c == c2 ) { continue ; }
		mc2 = false ;
		if ( c == c1 )
		{
			pat += "[^[:blank:]]" ;
		}
		else if ( c == c2 )
		{
			pat += "[^[:blank:]]*" ;
			mc2 = true ;
		}
		else
		{
			pat.push_back( c ) ;
		}
	}

	return pat ;
}


string conv_regex_any( const string& s,
		       char c1,
		       char c2 )
{
	//
	// Convert non-regex string 's' to a regex pattern.
	// c1 is the char representing any single character (including blanks)
	// c2 is the char representing any number of characters (including blanks), including 0.
	//
	// Also, escape any special regex characters so they are not interpreted as a part of
	// the regex expression.
	//
	// Reduce multiple c2's to a single c2 as this can cause REGEX to fail.
	//

	string pat ;

	bool mc2 = false ;

	const string regex_chars = "\\^$.+{[()|" ;

	for ( size_t i = 0 ; i < s.size() ; ++i )
	{
		char c = s[ i ] ;
		if ( mc2 && c == c2 ) { continue ; }
		mc2 = false ;
		if ( c == c1 )
		{
			pat += "." ;
		}
		else if ( c == c2 )
		{
			pat += ".*" ;
			mc2 = true ;
		}
		else if ( regex_chars.find( c ) != string::npos )
		{
			pat.push_back( '\\' ) ;
			pat.push_back( c ) ;
		}
		else
		{
			pat.push_back( c ) ;
		}
	}

	return pat ;
}


string delta_time( uint32_t seconds )
{
	//
	// Convert the time in seconds to a literal year/week/day h/m/s
	//

	string result ;

	if ( seconds >= 31536000 )
	{
		result = d2ds( seconds / 31536000) + " years ";
		seconds %= 31536000 ;
	}

	if ( seconds >= 604800 )
	{
		result += d2ds( seconds / 604800 ) + " weeks ";
		seconds %= 604800 ;
	}

	if ( seconds >= 86400 )
	{
		result += d2ds( seconds / 86400 ) + " days ";
		seconds %= 86400 ;
	}

	if ( seconds >= 3600 || result != "" )
	{
		result += d2ds( seconds / 3600, 2, ' ' ) + "h ";
		seconds %= 3600 ;
	}

	if ( seconds >= 60 || result != "" )
	{
		result += d2ds( seconds / 60, 2, ' ' ) + "m ";
		seconds %= 60 ;
	}

	if ( seconds < 1 )
	{
		result += " 0s";
	}
	else
	{
		result += d2ds( seconds, 2, ' ' ) + "s";
	}

	return result ;
}


} ;
