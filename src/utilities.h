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

bool   abbrev(  string s1, string s2 )        ;
bool   abbrev(  string s1, string s2, unsigned int n ) ;

string centre(  string s, unsigned int n, char c = ' ' ) ;
string center(  string s, unsigned int n, char c = ' ' ) ;

string copies(  string s, unsigned int n ) ;

bool   datatype( string s, char type ) ;

string delword( string s, unsigned int w ) ;

string delstr(  string s, unsigned int n )                 ;
string delstr(  string s, unsigned int n, unsigned int l ) ;

string delword( string s, unsigned int w )        ;
string delword( string s, unsigned int w, unsigned int n ) ;

string insert( string s1, string s2, int n = 0, char c = ' ' )           ;
string insert( string s1, string s2, unsigned int, unsigned int l, char c = ' ' ) ;

int lastpos( string s1, string s2 )                 ;
int lastpos( string s1, string s2, unsigned int p ) ;

string left( string s, unsigned int l, char c = ' ' ) ;

int pos( string s1, string s2, unsigned int p = 1 ) ;

string right( string s, unsigned int l, char c = ' ' ) ;

string space( string s, unsigned int n = 1, char c = ' ' ) ;

string strip( string s, char opt = 'B', char c = ' ' ) ;

string substr( string s, unsigned int n  )        ;
string substr( string s, unsigned int n, unsigned int l, char pad = ' '  ) ;

string subword( string s, unsigned int n )                 ;
string subword( string s, unsigned int n, unsigned int l ) ;

string word(  string s, unsigned int w )   ;

int wordindex( string s, unsigned int w )  ;

int wordlength( string s, unsigned int n ) ;

int wordpos( string s1, string s2 )  ;

int words( string s ) ;

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

string bs2xs( string s ) ;     // eg "01000010" -> "42"     ( REXX B2X("11000011")    ->   "C3" )

string cs2bs( string s ) ;     // eg "B" -> "01000010"
int    cs2d( string s )  ;     // eg "B" -> int66           ( REXX C2D("a")        ->       97 )
string cs2xs( string s ) ;     // eg "A" -> "41"            ( REXX C2X("0123"X)    ->    "0123" )
string cs2xs( char c   ) ;     // eg "B" -> "42"            ( same )

string d2cs( int i )    ;      // eg int66 -> "B"           ( REXX D2C(65)      ->   "A" )
string d2ds( int i )    ;      // eg int66 -> "66"          ( not required for REXX but is for C++)
string d2xs( int i )    ;      // eg int66 -> "42"          ( REXX D2X(129)       ->    "81" )

int    ds2d( string s ) ;      // eg "66"  -> int66         ( not required for REXX but is for C++)

string xs2bs( string s ) ;     // eg "42" -> "01000010"     ( REXX X2B("C3")        ->  "11000011" )
string xs2cs( string s ) ;     // eg "42" -> "B"            ( REXX X2C("4865 6c6c 6f") ->  "Hello"  )
int    xs2d( string s )  ;     // eg "A0" -> int160         ( REXX X2D("0E")        ->    14 )


bool   findword( string s1, string s2 ) ;
int    countc( string s, char c )       ;
bool   matchpattern( string s1, string s2 ) ;
string upper( string s )      ;
string lower( string s )      ;
bool   isnumeric( string s )    ;
string d2size( int )          ;
string hex2print( string t )  ;
string addCommas( string t )  ;
string addCommas( string t, int prec ) ;
bool   isvalidName( string s )  ;
bool   isvalidName4( string s ) ;
bool   isvalidHex( string s )   ;
bool   ishex( char c )          ;
bool   isoctal( string s )      ;
bool   isoctal( char c )        ;
bool   ispict( string, string ) ;
int    getpaths( string )       ;
string getpath( string, int ) ;
string mergepaths( string, string ) ;
void   selectParse( int &, string, string &, string &, string &, bool &, bool & ) ;
void   fieldOptsParse(int &, string, bool &, char &, bool &, char &, bool & ) ;
string parseString( bool & rlt, string & s, string p ) ;
