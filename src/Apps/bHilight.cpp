#include <string>
using namespace std ;

/* ************************************************************************************************************************************************************ */
/* VERY simple routine to add a bit of hilighting to C++ programs and various other types of files.  Matches brackets on the same line                          */
/* ************************************************************************************************************************************************************ */

#include "bHilight.h"

#undef LOGOUT
#undef MOD_NAME

#define LOGOUT aplog
#define MOD_NAME HILIGHT

void addHilight( vector<string> & data, string fileType, int iline, int startCol, int ZAREAW, int ZAREAD, string & ZSHADOW )
{
        if      ( fileType == "text/x-panel" ) { addPanelHilight( data, iline, startCol, ZAREAW, ZAREAD, ZSHADOW ) ; }
        else if ( fileType == "text/x-c++"  ||
                  fileType == "text/x-c"     ) { addCppHilight( data, iline, startCol, ZAREAW, ZAREAD, ZSHADOW )   ; }
        else if ( fileType == "text/x-rexx"  ) { addRexxHilight( data, iline, startCol, ZAREAW, ZAREAD, ZSHADOW )  ; }
        else if ( fileType == "text"         ) { addNoHilight( data, iline, startCol, ZAREAW, ZAREAD, ZSHADOW )    ; }
        else                                   { addDefHilight( data, iline, startCol, ZAREAW, ZAREAD, ZSHADOW )   ; }
}


void addPanelHilight( vector<string> & data, int iline, int startCol, int ZAREAW, int ZAREAD, string & ZSHADOW )
{
        int i, j, k, l ;
        int ln, start, stop ;
        int Area ;
        int oBra ;
        string w ;
        string ZTEMP  ;
        bool oQuote   ;
        bool oComment ;
        char Quote    ;

        Area     = ZAREAD * ZAREAW ;
        ZTEMP    = ""              ;
        ZTEMP.resize( Area, N_GREEN ) ;
        oComment = false ;

        l = 0 ;
        for ( i = iline ; i < data.size()-1 ; i++ )
        {
                l++ ;
                if ( i == 0 ) continue ;
                oQuote   = false ;
                oBra = 0 ;
                if ( l > ZAREAD ) break ;
                ln = data[ i ].size() ;
                if ( startCol > ln ) continue ;
                j  = 0 ;

                ZTEMP = string( ln, N_GREEN ) ;

                w = word(  data[ i ], 1 ) ;
                if ( w == ")PANEL" )           { ZTEMP.replace( 0, ln, ln, B_RED )    ; j = ln ; }
                if ( w == ")COMMENT" )         { ZTEMP.replace( 0, 8, 8, N_RED )      ; oComment = true  ; j = ln ; }
                else if ( w == ")ENDCOMMENT" ) { ZTEMP.replace( 0, 11, 11, N_YELLOW ) ; oComment = false ; j = ln ; }
                else if ( oComment ) { ZTEMP.replace( 0, ln, ln, N_WHITE ) ; j = ln ; }
                if ( w == ")BODY" )            { ZTEMP.replace( 0, 5, 5, N_RED )      ; j = ln ; }
                else if ( w == ")ENDBODY" )    { ZTEMP.replace( 0, 8, 8, N_YELLOW )   ; j = ln ; }
                else if ( w == ")COMMAND" )    { ZTEMP.replace( 0, 8, 8, N_RED )      ; j = ln ; }
                else if ( w == ")ENDCOMMAND" ) { ZTEMP.replace( 0, 11, 11, N_YELLOW ) ; j = ln ; }
                else if ( w == ")PROC" )       { ZTEMP.replace( 0, 5, 5, N_RED )      ; j = ln ; }
                else if ( w == ")ENDPROC" )    { ZTEMP.replace( 0, 8, 8, N_YELLOW )   ; j = ln ; }

                for ( ; j < ln ; j++ )
                {
                        if ( data[ i ][ j ] == ' '   & !oQuote ) continue ;
                        if ( data[ i ][ j ] == '"'   & !oQuote ) { oQuote = true  ; Quote = '"'  ; start = j ; continue ; }
                        if ( data[ i ][ j ] == '\''  & !oQuote ) { oQuote = true  ; Quote = '\'' ; start = j ; continue ; }
                        if ( data[ i ][ j ] == Quote &  oQuote ) { oQuote = false ; stop  = j ; ZTEMP.replace( start, stop-start+1, stop-start+1, N_YELLOW ) ; continue ; }
                        if ( oQuote ) continue ;
                        if ( ( j < ln - 1) & data[ i ][ j ] == '-' & data[ i ][ j+1 ] == '-' ) { ZTEMP.replace( j, ln-j+1, ln-j+1, N_BLUE ) ; break ; }
                        if ( isalnum( data[ i ][ j ] ) | data[ i ][ j ] == '#' | data[ i ][ j ] == '<' )
                        {
                                start = j ;
                                for ( ; j < data[ i ].size() ; j++ )
                                {
                                          if ( j > (ZAREAW + startCol) ) break ;
                                          stop = j ;
                                          if ( data[ i ][ j ] == ' ' | data[ i ][ j ] == '\t' | data[ i ][ j ] == '(' | data[ i ][ j ] == ')' ) { break ; }
                                }
                                if ( j == data[ i ].size() ) stop++ ;
                                w = upper( data[ i ].substr( start, stop-start ) ) ;
                                if      ( w == "LITERAL" ) ZTEMP.replace( start, stop-start, stop-start, N_TURQ )   ;
                                else if ( w == "FIELD" )   ZTEMP.replace( start, stop-start, stop-start, N_TURQ )   ;
                                else if ( w == "DYNAREA" ) ZTEMP.replace( start, stop-start, stop-start, N_TURQ )   ;
                                else if ( w == "PDC" )     ZTEMP.replace( start, stop-start, stop-start, N_TURQ )   ;
                                else                       ZTEMP.replace( start, stop-start, stop-start, N_WHITE )  ;
                        }
                }
                if ( startCol > 1 ) { ZTEMP.erase( 0, startCol-1 ) ; }
                ZTEMP.resize( ZAREAW, N_GREEN ) ;
                ZSHADOW.replace( ( l-1)*ZAREAW, ZAREAW, ZTEMP ) ;
        }

}


void addCppHilight( vector<string> & data, int iline, int startCol, int ZAREAW, int ZAREAD, string & ZSHADOW )
{
        int i, j, k, l ;
        int ln, start, stop ;
        int Area ;
        int oBrac1 ;
        int oBrac2 ;
        int p1   ;
        int p2   ;

        string w ;
        string line  ;
        string ZTEMP ;

        const string delims( "(){}= ;><+-*[]" ) ;

        bool oQuote   ;
        bool oComment ;

        char Quote    ;

        Area     = ZAREAD * ZAREAW ;
        ZSHADOW  = "" ;
        ZSHADOW.resize( Area, N_GREEN ) ;
        oComment = false ;
        oQuote   = false ;

        l      = 0 ;
        oBrac1 = 0 ;
        oBrac2 = 0 ;
        for ( i = iline ; i < data.size()-1 ; i++ )
        {
                l++ ;
                if ( i == 0 ) continue ;

                if ( l > ZAREAD ) break ;
                ln = data[ i ].size() ;

                ZTEMP = "" ;
                ZTEMP.resize( ln, N_GREEN ) ;
                start = 0 ;
                stop  = 0 ;
                p1    = 0 ;
                line  = data[ i ] ;
                for ( j = 0 ; j < ln ; j++ )
                {
                        if ( line[ j ] == ' '   && !oQuote ) { continue ; }
                        if ( line[ j ] == '"'   && !oQuote ) { oQuote = true  ; Quote = '"'  ; start = j ; continue ; }
                        if ( line[ j ] == '\''  && !oQuote ) { oQuote = true  ; Quote = '\'' ; start = j ; continue ; }
                        if ( line[ j ] == Quote &&  oQuote )
                        {
                                oQuote = false ;
                                stop  = j ;
                                ZTEMP.replace( start, stop-start+1, stop-start+1, N_YELLOW ) ;
                                continue ;
                        }
                        if ( oQuote )   { continue ; }
                        if ( j > ln-1 ) { break    ; }
                        if ( ln > 1 && j < ln-1 && line.compare( j, 2, "/*") == 0 )
                        {
                                oComment = true ;
                                ZTEMP.replace( j, 2, 2, B_BLUE ) ;
                                j++ ;
                                continue ;
                        }
                        if ( ln > 1 && oComment && j < ln-1 && line.compare( j, 2, "*/" ) == 0 )
                        {
                                oComment = false ;
                                ZTEMP.replace( j, 2, 2, B_BLUE ) ;
                                j++ ;
                                continue ;
                        }
                        if ( oComment )
                        {
                                ZTEMP.replace( j, 1, 1, B_BLUE ) ;
                                continue ;
                        }

                        p1 = line.find_first_of( delims, j ) ;
                        if ( p1 != j || p1 == string::npos )
                        {
                                if ( p1 == string::npos ) { p1 = ln - 1 ; }
                                start = j ;
                                stop  = p1 - 1 ;
                                w     = line.substr( start, stop-start+1 ) ;
                                if      ( w == "if" )       ZTEMP.replace( start, 2, 2, B_WHITE ) ;
                                else if ( w == "else" )     ZTEMP.replace( start, 4, 4, B_RED   ) ;
                                else if ( w == "for" )      ZTEMP.replace( start, 3, 3, B_TURQ  ) ;
                                else if ( w == "while" )    ZTEMP.replace( start, 5, 5, B_WHITE ) ;
                                else if ( w == "#include" ) ZTEMP.replace( start, 8, 8, B_TURQ  ) ;
                                else if ( w == "#define" )  ZTEMP.replace( start, 7, 7, B_TURQ  ) ;
                                else if ( w == "#undef" )   ZTEMP.replace( start, 6, 6, B_TURQ  ) ;
                                else if ( w == "void" )     ZTEMP.replace( start, 4, 4, B_BLUE  ) ;
                                else if ( w == "int" )      ZTEMP.replace( start, 3, 3, B_BLUE  ) ;
                                else if ( w == "uint" )     ZTEMP.replace( start, 4, 4, B_BLUE  ) ;
                                else if ( w == "bool" )     ZTEMP.replace( start, 4, 4, B_BLUE  ) ;
                                else if ( w == "string" )   ZTEMP.replace( start, 6, 6, B_BLUE  ) ;
                                else if ( w == "char" )     ZTEMP.replace( start, 4, 4, B_BLUE  ) ;
                                else if ( w == "//" )
                                {
                                        ZTEMP.replace( start, ln-j+2, ln-j+2, B_BLUE ) ;
                                        j = ln ;
                                }
                                else { j = p1  ; }
                        }
                        if ( line[ j ] == '(' ) { oBrac1++ ; ZTEMP.replace( j, 1, 1, oBrac1 % 7 + 7 ) ; continue ; }
                        if ( line[ j ] == ')' )
                        {
                                if ( oBrac1 == 0 ) { ZTEMP.replace( j, 1, 1, R_WHITE ) ; }
                                else               { ZTEMP.replace( j, 1, 1, oBrac1 % 7 + 7 ) ; oBrac1-- ; }
                                continue ;
                        }
                        if ( line[ j ] == '{' ) { oBrac2++ ; ZTEMP.replace( j, 1, 1, oBrac2 % 7 + 7 ) ; continue ; }
                        if ( line[ j ] == '}' )
                        {
                                if ( oBrac2 == 0 ) { ZTEMP.replace( j, 1, 1, R_WHITE ) ; }
                                else               { ZTEMP.replace( j, 1, 1, oBrac2 % 7 + 7 ) ; oBrac2-- ; }
                                continue ;
                        }
                        if ( line[ j ] == '=' ) { ZTEMP.replace( j, 1, 1, B_WHITE ) ; continue ; }
                }
                if ( startCol > 1 ) { ZTEMP.erase( 0, startCol-1 ) ; }
                ZTEMP.resize( ZAREAW, N_GREEN ) ;
                ZSHADOW.replace( ( l-1)*ZAREAW, ZAREAW, ZTEMP ) ;
        }
}


void addDefHilight( vector<string> & data, int iline, int startCol, int ZAREAW, int ZAREAD, string & ZSHADOW )
{
        int Area ;

        Area     = ZAREAD * ZAREAW ;
        ZSHADOW  = "" ;
        ZSHADOW.resize( Area, N_GREEN ) ;
}


void addNoHilight( vector<string> & data, int iline, int startCol, int ZAREAW, int ZAREAD, string & ZSHADOW )
{
        int Area ;

        Area     = ZAREAD * ZAREAW ;
        ZSHADOW  = "" ;
        ZSHADOW.resize( Area, N_YELLOW ) ;
}


void addRexxHilight( vector<string> & data, int iline, int startCol, int ZAREAW, int ZAREAD, string & ZSHADOW )
{
        int i, j, k, l ;
        int p1 ;
        int p2 ;
        int ln, start, stop ;
        int oBra ;
        int Area ;

        string w ;
        string ZTEMP  ;
        string delims( "()= ;" ) ;

        bool oQuote   ;
        bool oComment ;

        char Quote    ;

        Area     = ZAREAD * ZAREAW ;
        ZSHADOW  = "" ;
        ZSHADOW.resize( Area, N_GREEN ) ;

        oComment = false ;
        l  = 0 ;
        for ( i = iline ; i < data.size()-1 ; i++ )
        {
                l++ ;
                if ( i == 0 ) continue ;
                oBra   = 0 ;
                oQuote = false ;
                if ( l > ZAREAD ) break ;
                ln = data[ i ].size() ;

                ZTEMP = "" ;
                ZTEMP.resize( ln, N_GREEN ) ;
                start = 0 ;
                stop  = 0 ;
                p1 = 0 ;
                j  = 0 ;
                while ( true )
                {
                        if ( oComment )
                        {
                                p2 = data[ i ].find( "*/" ) ;
                                if ( p2 == string::npos )
                                {
                                        ZTEMP.replace( 0, ln, ln, B_WHITE ) ;
                                        break ;
                                }
                                else
                                {
                                        ZTEMP.replace( 0, p2+2, p2+2, B_WHITE ) ;
                                        oComment = false ;
                                        j = p2 + 2 ;
                                        if ( j > ln-1 ) break ;
                                }
                        }
                        if ( data[ i ][ j ] == ' '   && !oQuote ) { j++ ; continue ; }
                        if ( data[ i ][ j ] == '"'   && !oQuote ) { oQuote = true  ; Quote = '"'  ; start = j ; j++ ; continue ; }
                        if ( data[ i ][ j ] == '\''  && !oQuote ) { oQuote = true  ; Quote = '\'' ; start = j ; j++ ; continue ; }
                        if ( data[ i ][ j ] == Quote &&  oQuote ) { oQuote = false ; stop  = j ; ZTEMP.replace( start, stop-start+1, stop-start+1, N_YELLOW ) ; j++ ; continue ; }
                        if ( oQuote ) { j++ ; continue ; }

                        if ( j > ln-1 ) { break ; }

                        p1 = data[ i ].find_first_of( delims, j ) ;
                        if ( p1 != j || p1 == string::npos )
                        {
                                if ( p1 == string::npos ) { p1 = ln ; }
                                start = j ;
                                stop  = p1 - 1 ;
                                j     = p1     ;
                                w     = upper( data[ i ].substr( start, stop-start+1 ) ) ;
                                debug1( " dje word is " << w << " len="<<stop-start+1<<" pos="<<start<< endl ) ;
                                if      ( w == "IF"   ) ZTEMP.replace( start, 2, 2, B_TURQ )   ;
                                else if ( w == "ELSE" ) ZTEMP.replace( start, 4, 4, B_TURQ )   ;
                                else if ( w == "DO"   ) ZTEMP.replace( start, 2, 2, B_TURQ )   ;
                                else if ( w == "THEN" ) ZTEMP.replace( start, 4, 4, B_RED  )   ;
                                else if ( w == "END"  ) ZTEMP.replace( start, 3, 3, B_TURQ )   ;
                                else if ( w == "/*"   )
                                {
                                        p2 = data[ i ].find( "*/", p1 ) ;
                                        if ( p2 == string::npos )
                                        {
                                                ZTEMP.replace( start, ln-j, ln-j, B_WHITE ) ;
                                                oComment = true ;
                                                continue        ;
                                        }
                                        else
                                        {
                                                ZTEMP.replace( start, p2-start+2, p2-start+2, B_WHITE ) ;
                                                j = p2 + 2 ;
                                        }
                                        continue ;
                                }
                        }
                        else
                        {
                                if ( data[ i ][ j ] == '(' ) { oBra++ ; ZTEMP.replace( j, 1, 1, oBra % 7 + 7 ) ; }
                                else if ( data[ i ][ j ] == ')' )
                                {
                                        if ( oBra == 0 ) { ZTEMP.replace( j, 1, 1, R_WHITE ) ; }
                                        else             { ZTEMP.replace( j, 1, 1, oBra % 7 + 7 ) ; oBra-- ; }
                                }
                                else if ( data[ i ][ j ] == '=' ) { oBra++ ; ZTEMP.replace( j, 1, 1, B_WHITE ) ; }
                                j++ ;
                        }
                }
                if ( startCol > 1 ) { ZTEMP.erase( 0, startCol-1 ) ; }
                ZTEMP.resize( ZAREAW, N_GREEN ) ;
                ZSHADOW.replace( ( l-1)*ZAREAW, ZAREAW, ZTEMP ) ;
        }
}

