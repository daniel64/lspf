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

bool   abbrev( const string& s1, const string& s2 )        ;
bool   abbrev( const string& s1, const string& s2, unsigned int n ) ;

string centre( const string& s, unsigned int n, char c = ' ' ) ;
string center( const string& s, unsigned int n, char c = ' ' ) ;

string copies( const string& s, unsigned int n ) ;

bool   datatype( const string& s, char type ) ;

string delstr( string s, unsigned int n )  ;
string delstr( string s, unsigned int n, unsigned int l ) ;

string delword( string s, unsigned int w ) ;
string delword( string s, unsigned int w, unsigned int n ) ;

string insert( const string& s1, string s2, unsigned int n = 0, char c = ' ' )    ;
string insert( string s1, string s2, unsigned int, unsigned int l, char c = ' ' ) ;

int    lastpos( const string& s1, const string& s2 )                 ;
int    lastpos( const string& s1, const string& s2, unsigned int p ) ;

string left( string s, unsigned int l, char c = ' ' ) ;

int    pos( const string& s1, const string& s2, unsigned int p = 1 ) ;

string reverse( string s ) ;

string right( string s, unsigned int l, char c = ' ' ) ;

string space( const string& s, unsigned int n = 1, char c = ' ' ) ;

string strip( string s, char opt = 'B', char c = ' ' ) ;

string substr( const string& s, unsigned int n  )        ;
string substr( const string& s, unsigned int n, unsigned int l, char pad = ' '  ) ;

string subword( const string& s, unsigned int w )                 ;
string subword( const string& s, unsigned int w, unsigned int n ) ;

string word( const string& s, unsigned int w )   ;

int wordindex( const string& s, unsigned int w )  ;

int wordlength( const string& s, unsigned int n ) ;

int wordpos( const string& s1, const string& s2 )  ;

int words( const string& s ) ;

// These convert to/from a displayable binary/hex/decimal or character string
// bs - displayable binary string eg "01000010"
// cs - raw character string
// ds - displayable decimal string eg "66"
// xs - displayable hex string eg "42"
// d  - integer

// Not all exist yet !
// b2x
// c2d c2x
// d2c d2x
// x2b x2c x2d

string bs2xs( string s ) ;            // eg "01000010" -> "42"     ( REXX B2X("11000011")    ->   "C3" )

string cs2bs( const string& s ) ;     // eg "B" -> "01000010"
int    cs2d( const string& s )  ;     // eg "B" -> int66           ( REXX C2D("a")        ->       97 )
string cs2xs( const string& s ) ;     // eg "A" -> "41"            ( REXX C2X("0123"X)    ->    "0123" )
string cs2xs( char c   ) ;            // eg "B" -> "42"            ( same )
string i2bs( uint i ) ;               // eg 1   -> 00000000000000000000000000000001
string i2xs( uint i ) ;               // eg 1   -> 00000001

string d2cs( int i ) ;                // eg int66 -> "B"           ( REXX D2C(65)      ->   "A" )
string d2ds( int i ) ;                // eg int66 -> "66"          ( not required for REXX but is for C++)
string d2ds( int i, int j ) ;         // eg int66 -> "66"          ( not required for REXX but is for C++)
string d2xs( int i ) ;                // eg int66 -> "42"          ( REXX D2X(129)       ->    "81" )

int    ds2d( const string& s )  ;     // eg "66"  -> int66         ( not required for REXX but is for C++)

string xs2bs( const string& s ) ;     // eg "42" -> "01000010"     ( REXX X2B("C3")        ->  "11000011" )
string xs2cs( const string& s ) ;     // eg "42" -> "B"            ( REXX X2C("4865 6c6c 6f") ->  "Hello"  )
int    xs2d( const string& s )  ;     // eg "A0" -> int160         ( REXX X2D("0E")        ->    14 )


string& trim_left( string& s )  ;
string& trim_right( string& s ) ;
string& trim( string& s )       ;

string& dquote( char c, string& s ) ;

bool   findword( const string& s1, const string& s2 )  ;
int    countc( const string& s, char c )       ;

string& iupper( string& s ) ;
char&   iupper( char& c   ) ;
string& ilower( string& s ) ;
char&   ilower( char& c ) ;
string  upper( string s ) ;
string  lower( string s ) ;

string& idelstr( string& s, unsigned int n )  ;
string& idelstr( string& s, unsigned int n, unsigned int l ) ;

string& idelword( string& s, unsigned int w ) ;
string& idelword( string& s, unsigned int w, unsigned int n ) ;

string& istrip( string& s, char opt = 'B', char c = ' ' ) ;

bool   isnumeric( const string& s ) ;
string d2size( int )          ;
string hex2print( const string& t )  ;

string addCommas( string t )  ;
string addCommas( string t, int prec ) ;

bool   isvalidName( const string& s )  ;
bool   isvalidName4( const string& s ) ;
bool   ishex( const string& s ) ;
bool   ishex( char c )           ;
bool   isoctal( const string& s ) ;
bool   isoctal( char c )        ;
bool   ispict( const string&, const string& ) ;

int    getpaths( const string& p ) ;
string getpath( const string& s, int p ) ;
string mergepaths( const string& p1, const string& p2 ) ;
string mergepaths( const string& p1, const char* c1 )   ;
string mergepaths( const string& p1, const char* c1, const char* c2 ) ;
string mergepaths( const string& p1, const string& p2, const string& p3 ) ;

string  parseString( errblock& err, string& s, string p ) ;
string& getNameList( errblock& err, string& s ) ;

string extractKWord( errblock& err, string& s, string p ) ;
