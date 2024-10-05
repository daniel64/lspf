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


// REXX-like string handling functions.

namespace lspf {

bool abbrev( const string&,
	     const string& ) ;
bool abbrev( const string&,
	     const string&,
	     unsigned int ) ;

string centre( const string&,
	       unsigned int,
	       char = ' ' ) ;
string center( const string&,
	       unsigned int,
	       char = ' ' ) ;

string copies( const string&,
	       unsigned int ) ;

string copies( const char,
	       unsigned int ) ;

bool datatype( const string&,
	       char type ) ;

string delstr( string,
	       unsigned int )  ;
string delstr( string,
	       unsigned int,
	       unsigned int ) ;

string delword( string,
		unsigned int ) ;
string delword( string,
		unsigned int,
		unsigned int ) ;

string insert( const string&,
	       string,
	       unsigned int = 0,
	       char = ' ' ) ;
string insert( string,
	       string,
	       unsigned int,
	       unsigned int,
	       char = ' ' ) ;

string justify( const string&,
		size_t,
		char = ' ' ) ;

int lastpos( const string&,
	     const string& ) ;
int lastpos( const string&,
	     const string&,
	     unsigned int ) ;

string left( string,
	     unsigned int,
	     char = ' ' ) ;

int pos( const string&,
	 const string&,
	 unsigned int = 1 ) ;

string reverse( string ) ;

string right( string,
	      unsigned int,
	      char = ' ' ) ;

string space( const string&,
	      unsigned int = 1,
	      char = ' ' ) ;

string strip( string,
	      char = 'B',
	      char = ' ' ) ;

string substr( const string&,
	       unsigned int ) ;
string substr( const string&,
	       unsigned int,
	       unsigned int,
	       char = ' '  ) ;

string subword( const string&,
		unsigned int ) ;
string subword( const string&,
		unsigned int,
		unsigned int ) ;

string translate( const string& ) ;

string translate( string,
		  string,
		  const string& = "",
		  char = ' ' ) ;

string translate( string,
		  const char,
		  const char ) ;

string word( const string&,
	     unsigned int ) ;

int wordindex( const string&,
	       unsigned int ) ;

int wordlength( const string&,
		unsigned int ) ;

int wordpos( const string&,
	     const string& )  ;

unsigned int words( const string& ) ;

// These convert to/from a displayable binary/hex/decimal or character string.
// bs - displayable binary string eg "01000010".
// cs - raw character string.
// ds - displayable decimal string eg "66".
// xs - displayable hex string eg "42".
// d  - integer.
//
// Not all exist yet !
// b2x
// c2d c2x
// d2c d2x
// x2b x2c x2d

string bs2xs( string ) ;                      // eg "01000010" -> "42"     ( REXX B2X("11000011")    ->   "C3" )

string cs2bs( const string& ) ;               // eg "B" -> "01000010"
int cs2d( const string& ) ;                   // eg "B" -> int66           ( REXX C2D("a")        ->       97 )
string cs2xs( const string& ) ;               // eg "A" -> "41"            ( REXX C2X("0123"X)    ->    "0123" )
string cs2xs1( const string&,                 // eg "A" -> "4"
	       uint = 0,
	       size_t = string::npos ) ;
string cs2xs2( const string&,                 // eg "A" -> "1"
	       uint = 0,
	       size_t = string::npos ) ;
string c2xs( char ) ;                         // eg "B" -> "42"            ( same )
string i2bs( uint ) ;                         // eg 1   -> 00000000000000000000000000000001
string i2xs( uint ) ;                         // eg 1   -> 00000001

string d2cs( int ) ;                          // eg int66 -> "B"           ( REXX D2C(65)      ->   "A" )
string d2ds( int,                             // eg int66 -> "66"          ( not required for REXX but is for C++)
	     int = 0,
	     char = '0' ) ;
string ui2ds( uint,                           // eg int66 -> "66"          ( not required for REXX but is for C++)
	      int = 0 ) ;
string d2xs( int ) ;                          // eg int66 -> "42"          ( REXX D2X(129)       ->    "81" )

int ds2d( const string& ) ;                   // eg "66"  -> int66         ( not required for REXX but is for C++)
size_t  ds2size_t( const string& ) ;          // eg "66"  -> int66         ( not required for REXX but is for C++)
ssize_t ds2ssize_t( const string& ) ;         // eg "66"  -> int66         ( not required for REXX but is for C++)

string xs2bs( const string& ) ;               // eg "42" -> "01000010"     ( REXX X2B("C3")        ->  "11000011" )
string xs2cs( const string& ) ;               // eg "42" -> "B"            ( REXX X2C("4865 6c6c 6f") ->  "Hello"  )
int xs2d( const string& ) ;                   // eg "A0" -> int160         ( REXX X2D("0E")        ->    14 )

string addr2str( void *) ;

string strip_left( string ) ;
string strip_right( string ) ;

string& trim_left( string& ) ;
string& trim_right( string& ) ;
string& trim( string& ) ;

string dquote( errblock&,
	       char,
	       string ) ;

bool findword( const string&,
	       const string& ) ;

bool findword( const char,
	       const string& ) ;

uint countc( const string&,
	     char ) ;

string& iupper( string& ) ;
string& iupper( string&,
		unsigned int,
		unsigned int ) ;
char& iupper( char& ) ;

string& iupper1( string& ) ;

string& ilower( string& ) ;
string& ilower( string&,
		unsigned int,
		unsigned int ) ;
char& ilower( char& ) ;

string upper( string ) ;
string lower( string ) ;

string upper1( string ) ;

bool ualpha( const string&,
	     size_t ) ;
bool lalpha( const string&,
	     size_t ) ;

string& idelstr( string&,
		 unsigned int ) ;
string& idelstr( string&,
		 unsigned int,
		 unsigned int ) ;

string& idelword( string&,
		  unsigned int ) ;
string& idelword( string&,
		  unsigned int,
		  unsigned int ) ;

string& istrip( string&,
		char = 'B',
		char = ' ' ) ;

bool isnumeric( const string& ) ;
string d2size( uint64_t,
	       int = 2 ) ;
string hex2print( const string& ) ;

string addCommas( int ) ;
string addCommas( string ) ;
string addCommas( string,
		  int ) ;

bool isvalidName( const string& ) ;
bool isvalidName4( const string& ) ;

bool isalpha( const string& ) ;
bool isalphab( const string& ) ;
bool isalphanum( const string& ) ;
bool isalphabnum( const string& ) ;

bool isbit( const string& ) ;

bool ishex( const string& ) ;
bool ishex( char ) ;

bool isoctal( const string& ) ;
bool isoctal( char ) ;

bool ispict( const string&,
	     const string& ) ;

void ispictcn( errblock&,
	       const string&,
	       const char,
	       string&,
	       const string& ) ;

void isenum( errblock&,
	     const string& ) ;

int getpaths( const string& ) ;

string getpath( const string&,
		int ) ;

string mergepaths( const string&,
		   const string& ) ;
string mergepaths( const string&,
		   const char* ) ;
string mergepaths( const string&,
		   const char*,
		   const char* ) ;
string mergepaths( const string&,
		   const string&,
		   const string& ) ;

string parseString1( errblock&,
		    string&,
		    string ) ;
bool parseString2( string&,
		   string ) ;
string parseString3( errblock&,
		    string& ) ;

string& getNameList( errblock&,
		     string& ) ;

string getKeywords1( errblock&,
		     string& ) ;

string getKeywords2( errblock&,
		     string ) ;

string extractKWord( errblock&,
		     string&,
		     string ) ;

void qwords( errblock&,
	     const string&,
	     vector<string>& ) ;

string qstring( errblock&,
		const string& ) ;

string qstring1( errblock&,
		 const string& ) ;

void word( const string&,
	   vector<string>& ) ;

bool check_date( const string&,
		 const string&,
		 const string& ) ;

bool check_date( const string&,
		 const string& ) ;

string& add_vmask( string&,
		   const string&,
		   const string&,
		   VEDIT_TYPE ) ;

void remove_vmask( errblock&,
		   const string&,
		   const string&,
		   string&,
		   const string&,
		   VEDIT_TYPE ) ;

bool namesNotValid( string&,
		    const string&,
		    const string& = "",
		    const string& = "",
		    const string& = "",
		    const string& = "" ) ;

bool allNumeric( const string& ) ;

bool ibrackets( const string&,
		char ) ;

bool suffix( const string&,
	     const string& ) ;

bool prefix( const string&,
	     const string& ) ;

void rem_comments( string& ) ;

size_t findnq( const string&,
	       char,
	       size_t = 0 ) ;

size_t findnq( const string&,
	       const string&,
	       size_t = 0 ) ;

string conv_regex( const string&,
		   char,
		   char ) ;

string conv_regex_any( const string&,
		       char,
		       char ) ;

string delta_time( uint32_t ) ;

} ;
