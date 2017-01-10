/* Compile with ::                                                                          */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPFLST0A.so -o libPFLST0A.so PFLST0A.cpp      */

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


/* Display a file list for a directory and invoke browse if a file is selected or list the new directory    */
/* Use system variable ZFLSTPGM to refer to this                                                            */

/* If invoked with a PARM of BROWSE file, browse file                                                       */
/* If invoked with a PARM of EDIT file, edit file                                                           */
/* If invoked with a PARM of VIEW file, view file                                                           */
/* If invoked with a PARM of path, list path                                                                */
/* If invoked with a PARM of INFO, list info for entry                                                      */
/* If invoked with a PARM of EXPAND, will expand the passed directory name to the next level                */
/*                SUBPARM of ALL - return all types (fully qualified)                                       */
/*                SUBPARM of DO1 - return only directories                                                  */
/*                SUBPARM of FO1 - return only fully qualified files                                        */
/*                SUBPARM of FO2 - return only the file names (without the directory name)                  */
/* If invoked with a PARM of EXPAND1, will expand field according to subtype                                */
/*                SUBPARM of PNL  - panel files in ZPLIB                                                    */
/*                SUBPARM of REXX - REXX programs in ZORXPATH                                               */
/*                SUBPARM of PGM  - C++ programs in ZDLPATH                                                 */

/* CD /xxx to change to directory /xxx                                                                      */
/* CD xxx to change to directory xxx under the current directory listing                                    */
/* BACK or S to go back one directory level                                                                 */
/* S xxx to select file or directory xxx from the current list                                              */
/* L xxx FIRST|LAST|PREV to scroll the list to entry xxx (NEXT is the default)                              */
/* O  - filter list                                                                                         */
/* REF - refresh directory list                                                                             */
/* MKDIR  - Make a directory (under current or specify full path)                                           */
/* SEARCH - Show a list containing only files that match a search word (within the file).  Uses grep        */
/* TOUCH  - Make a file (under current or specify full path)                                                */

/* Line Commands:                                                                                           */
/* B  - Browse file                                                                                         */
/* C  - Copy file/directory/symlink                                                                         */
/* D  - Delete file                                                                                         */
/* E  - Edit file                                                                                           */
/* EX - Execute file as OOREXX EXEC                                                                         */
/* I  - Information on the entry                                                                            */
/* L  - List directory                                                                                      */
/* M  - Modify entry attributes                                                                             */
/* R  - Rename pathname                                                                                     */
/* S  - Select directory or file (list or browse)                                                           */
/* T  - Display a directory tree                                                                            */
/* TT - Display a directory tree in BROWSE                                                                  */
/* X  - List link directory                                                                                 */
/* =  - repeat the last-entered line command                                                                */

/* nano - invoke nano editor on the file                                                                    */
/* vi   - invoke vi editor on the file                                                                      */
/*        CONTROL TIMEOUT DISABLE for nano and vi or lspf will chop the application after ZMAXWAIT          */
/*        as it does not return until reset_prog_mode                                                       */
/*                                                                                                          */
/*                                                                                                          */
/* ZRC/ZRSN/ZRESULT codes                                                                                   */
/*                                                                                                          */
/* For PARM=LIST                                                                                            */
/*     ZRC=8/ZRSN=12 - Entry passed does not exist                                                          */
/*     ZRC=8/ZRSN=16 - Permission denied                                                                    */
/*     ZRC=8/ZRSN=20 - Unkown error occured                                                                 */

#include <boost/filesystem.hpp>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

#include "../lspf.h"
#include "../utilities.h"
#include "../classes.h"
#include "../pWidgets.h"
#include "../pVPOOL.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"
#include "PFLST0A.h"

using namespace std ;
using namespace boost::filesystem ;

#undef LOGOUT
#undef MOD_NAME

#define LOGOUT aplog
#define MOD_NAME PFLST0A


void PFLST0A::application()
{
	log( "I", "Application PFLST0A starting with PARM of " << PARM << endl ) ;

	int    i        ;
	int    RCode    ;
	int    num      ;

	bool   del      ;
	bool   errs     ;

	string entry    ;
	string OPATH    ;
	string AFHIDDEN ;
	string OHIDDEN  ;
	string OSEL     ;
	string lp       ;
	string PGM      ;
	string w1       ;
	string w2       ;
	string w3       ;
	string t        ;
	string filter   ;

	string CONDOFF  ;
	string NEMPTOK  ;
	string DIRREC   ;
	string NEWENTRY ;
	string FREPL    ;

	char * buffer       ;
	size_t bufferSize = 255 ;
	size_t rc               ;

	boost::system::error_code ec ;

	vcopy( "ZUSER", ZUSER, MOVE ) ;
	vcopy( "ZSCREEN", ZSCREEN, MOVE ) ;

	std::ofstream of ;

	vdefine( "SEL ENTRY MESSAGE TYPE PERMISS SIZE STCDATE MODDATE", &SEL, &ENTRY, &MESSAGE, &TYPE, &PERMISS, &SIZE, &STCDATE, &MODDATE ) ;
	vdefine( "MODDATES ZVERB ZHOME ZCMD ZPATH CONDOFF NEWENTRY FREPL", &MODDATES, &ZVERB, &ZHOME, &ZCMD, &ZPATH, &CONDOFF, &NEWENTRY, &FREPL ) ;
	vdefine( "RSN NEMPTOK DIRREC AFHIDDEN", &RSN, &NEMPTOK, &DIRREC, &AFHIDDEN ) ;
	vdefine( "CRP", &CRP ) ;

	vget( "ZHOME", SHARED ) ;
	vget( "AFHIDDEN", PROFILE ) ;

	if ( PARM == "" ) { vget( "ZPATH", PROFILE ) ; }
	else
	{
		w1 = word( PARM, 1 ) ;
		if ( w1 == "BROWSE" || w1 == "EDIT" )
		{
			ZPATH = subword( PARM, 2 ) ;
			try
			{
				if ( is_regular_file( ZPATH ) )
				{
					if      ( w1 == "BROWSE" ) { browse( ZPATH ) ;  cleanup() ; return ;  }
					else if ( w1 == "VIEW"   ) { view( ZPATH )   ;  cleanup() ; return ;  }
					else                       { edit( ZPATH )   ;  cleanup() ; return ;  }
				}
			}
			catch ( const filesystem_error& ex )
			{
				setmsg( "FLST011" ) ;
				cleanup() ;
				return    ;
			}
		}
		else if ( w1 == "INFO" )
		{
			showInfo( subword( PARM, 2 ) ) ;
			cleanup() ;
			return    ;
		}
		else if ( w1 == "EXPAND" )
		{
			ZRESULT = expandDir( subword( PARM, 2 ) ) ;
			if ( word( PARM, 2 ) == "FO2" && ZRC == 0 )
			{
				ZRESULT = substr( ZRESULT, lastpos( "/", ZRESULT)+1 ) ;
			}
			cleanup() ;
			return    ;
		}
		else if ( w1 == "EXPAND1" )
		{
			ZRESULT = expandFld1( subword( PARM, 2 ) ) ;
			cleanup() ;
			return    ;
		}
		else { ZPATH = PARM ; }
		if( !is_directory( ZPATH ) ) ZPATH = ZHOME ;
	}

	if ( !is_directory( ZPATH ) ) ZPATH = ZHOME ;

	OSEL   = ""    ;
	DSLIST = "DSLST" + right( d2ds( taskid() ), 3, '0' ) ;
	MSG    = ""  ;


	RC        = 0  ;
	i         = 1  ;
	ZTDVROWS  = 1  ;
	filter    = "" ;
	UseSearch = false ;

	createFileList1() ;
	if ( RC > 0 ) { setmsg( "FLST015" ) ; }

	while ( true )
	{
		if ( ZTDVROWS > 0 )
		{
			tbtop( DSLIST )     ;
			tbskip( DSLIST, i ) ;
		}
		else
		{
			tbbottom( DSLIST ) ;
			tbskip( DSLIST, - (ZTDDEPTH-2) ) ;
			if ( RC > 0 ) { tbtop( DSLIST ) ; }
		}
		OPATH   = ZPATH    ;
		OHIDDEN = AFHIDDEN ;
		if ( MSG == "" ) { ZCMD  = "" ; }
		tbdispl( DSLIST, "PFLST0A1", MSG, "ZCMD" ) ;
		if ( RC  > 8 ) { abend() ; }
		if ( RC == 8 ) { break   ; }
		MSG = "" ;
		w1  = upper( word( ZCMD, 1 ) ) ;
		w2  = word( ZCMD, 2 ) ;
		w3  = word( ZCMD, 3 ) ;
		if ( (w1 == "REFRESH" || w1 == "RESET" ) && w2 == "" )
		{
			if ( w1 == "RESET" ) { filter = "" ; UseSearch = false ; }
			if ( filter == "" ) { vreplace( "FMSG1", "" ) ; }
			if ( !UseSearch   ) { vreplace( "FMSG2", "" ) ; }
			tbend( DSLIST ) ;
			createFileList1( filter ) ;
			continue ;
		}
		if ( w1 == "O" && w2 != "" && w3 == "" )
		{
			filter = w2 ;
			vreplace( "FMSG1", "Filtered on file name" ) ;
			tbend( DSLIST ) ;
			createFileList1( filter ) ;
			continue ;
		}
		if ( ( w1 == "SEARCH" || w1 == "SRCHFOR" ) && w2 != "" && w3 == "" )
		{
			UseSearch = true ;
			vreplace( "FMSG2", "Filtered on contents" ) ;
			createSearchList( w2 ) ;
			tbend( DSLIST ) ;
			createFileList1( filter ) ;
			continue ;
		}
		i = ZTDTOP ;
		vget( "ZVERB", SHARED ) ;
		CRP    = 0 ;
		RCode  = processPrimCMD() ;
		if ( RCode == 8 )  { MSG = "PSYS018" ; continue ; }
		if ( CRP > 0 )     { i = CRP       ; }
		if ( RCode == 4 )  { continue      ; }
		if ( ZPATH == "" ) { ZPATH = ZHOME ; }
		if ( OHIDDEN != AFHIDDEN )
		{
			filter = ""       ;
			tbend( DSLIST )   ;
			createFileList1() ;
			i = 1    ;
			continue ;
		}
		if ( OPATH != ZPATH )
		{
			if ( ( !exists( ZPATH ) || !is_directory( ZPATH ) ) )
			{
				MSG = "PSYS012A" ;
			}
			else
			{
				filter = ""       ;
				tbend( DSLIST )   ;
				createFileList1() ;
				i = 1 ;
			}
			continue ;
		}
		CONDOFF = "" ;
		NEMPTOK = "" ;
		DIRREC  = "" ;
		while ( ZTDSELS > 0 )
		{
			if ( SEL == "=" && OSEL != "" ) { SEL = OSEL ; }
			OSEL = SEL ;
			if ( ZPATH.back() == '/' ) { entry = ZPATH + ENTRY       ; }
			else                       { entry = ZPATH + "/" + ENTRY ; }
			if ( SEL == ""  ) {}
			else if ( SEL == "I" )
			{
				showInfo( entry )       ;
				SEL     = ""            ;
				MESSAGE = "Information" ;
				tbput( DSLIST )         ;
			}
			else if ( SEL == "EX" )
			{
				vcopy( "ZOREXPGM", PGM, MOVE ) ;
				select( "PGM(" + PGM + ") PARM(" + entry + ")" ) ;
				SEL = "" ;
				if ( ZRESULT != "" ) { MESSAGE = ZRESULT    ; MSG = "PSYS011M" ; }
				else                 { MESSAGE = "Executed" ;                    }
				tbput( DSLIST ) ;
			}
			else if ( SEL == "X" )
			{
				SEL = "" ;
				lp  = "" ;
				while ( true )
				{
					buffer = new char[ bufferSize ] ;
					rc     = readlink( entry.c_str(), buffer, bufferSize ) ;
					if ( rc == -1 )
					{
						delete[] buffer ;
						if( errno == ENAMETOOLONG )
						{
							bufferSize += 255;
						}
						else
						{
							MESSAGE = "Not a link" ;
							break ;
						}
					}
					else
					{
						lp = string( buffer, rc ) ;
						delete[] buffer ;
						if ( substr( lp, 1, 1 ) != "/" ) { lp = ZPATH + lp ; }
						if ( !is_directory( lp ) )
						{
							MESSAGE = "Not a directory" ;
							lp      = ""    ;
						}
						break ;
					}
				}
				if ( lp == "" ) { tbput( DSLIST ) ; continue ; }
				vcopy( "ZFLSTPGM", PGM, MOVE ) ;
				select( "PGM(" + PGM + ") PARM(" + lp + ")" ) ;
				if ( ZRESULT != "" ) { MESSAGE = ZRESULT  ; }
				else                 { MESSAGE = "Linked" ; }
				tbput( DSLIST ) ;
				continue        ;
			}
			else if ( SEL == "C" )
			{
				ZCMD     = "" ;
				SEL      = "" ;
				NEWENTRY = "" ;
				FREPL    = "" ;
				if ( !is_directory( entry ) && !is_symlink( entry ) && !is_regular_file( entry ) )
				{
					MSG     = "FLST012A"     ;
					MESSAGE = "Invalid Type" ;
					tbput( DSLIST ) ;
					continue        ;
				}
				display( "PFLST0A5", MSG, "ZCMD" ) ;
				if ( RC > 8  ) { abend() ; }
				if ( ZCMD == "CAN" || ZCMD == "CANCEL" || RC == 8 )
				{
					ZCMD    = ""          ;
					MSG     = "FLST011W"  ;
					MESSAGE = "Cancelled" ;
				}
				else
				{
					if ( NEWENTRY[ 0 ] != '/' ) { NEWENTRY = substr( entry, 1, lastpos( "/", entry ) ) + NEWENTRY ; }
					if ( exists( NEWENTRY ) )
					{
						if ( is_directory( entry ) || is_symlink( entry ) )
						{
							MSG     = "FLST011Y" ;
							MESSAGE = "Invalid"  ;
							tbput( DSLIST ) ;
							continue        ;
						}
						if ( FREPL != "/" )
						{
							MSG     = "FLST011X"    ;
							MESSAGE = "File Exists" ;
							tbput( DSLIST ) ;
							continue        ;
						}
						if ( !is_regular_file( NEWENTRY ) )
						{
							MSG     = "FLST011Y"     ;
							MESSAGE = "Invalid Type" ;
							tbput( DSLIST ) ;
							continue        ;
						}
					}
					if ( is_directory( entry ) )
					{
						errs = false ;
						copyDirs( entry, NEWENTRY, DIRREC, errs ) ;
						if ( errs )
						{
							MSG     = "FLST012G" ;
							MESSAGE = "Errors"   ;
						}
						else
						{
							MSG     = "FLST011U" ;
							MESSAGE = "Copied"   ;
						}
						tbput( DSLIST ) ;
						continue ;
					}
					else if  ( is_regular_file( entry ) )
					{
						copy_file( entry, NEWENTRY, copy_option::overwrite_if_exists, ec ) ;
					}
					else if  ( is_symlink( entry ) )
					{
						copy_symlink( entry, NEWENTRY, ec ) ;
					}
					if ( ec.value() == boost::system::errc::success )
					{
						MSG     = "FLST011U" ;
						MESSAGE = "Copied"   ;
					}
					else
					{
						MSG     = "FLST011V"   ;
						RSN     = ec.message() ;
						MESSAGE = ec.message() ;
						log( "E", "Copy of " + entry + " to " + NEWENTRY + " failed with " + ec.message() << endl ) ;
					}
				}
				tbput( DSLIST ) ;
			}
			else if ( SEL == "D" )
			{
				ZCMD = "" ;
				SEL  = "" ;
				if ( CONDOFF == "/" ) { del = true  ; }
				else                  { del = false ; }
				if ( !del )
				{
					display( "PFLST0A3", MSG, "ZCMD" ) ;
					if ( RC > 8  ) { abend() ; }
					if ( ZCMD == "CAN" || ZCMD == "CANCEL" || RC == 8 )
					{
						ZCMD    = ""          ;
						MSG     = "FLST011P"  ;
						MESSAGE = "Cancelled" ;
					}
					else { del = true ; }
				}
				if ( del )
				{
					if ( NEMPTOK == "/" )
					{
						num = remove_all( entry.c_str( ), ec ) ;
						if ( ec.value() == boost::system::errc::success )
						{
							MSG     = "FLST011N" ;
							RSN     = d2ds( num ) + " files deleted" ;
							MESSAGE = "Deleted" ;
						}
						else
						{
							MSG     = "FLST011O"   ;
							RSN     = ec.message() ;
							MESSAGE = ec.message() ;
							log( "E", "Delete of " + entry + " failed with " + ec.message() << " " << num <<" messages deleted "<< endl ) ;
						}
					}
					else
					{
						remove( entry.c_str( ), ec ) ;
						if ( ec.value() == boost::system::errc::success )
						{
							MSG     = "FLST011N"        ;
							RSN     = "1 entry deleted" ;
							MESSAGE = "Deleted"         ;
						}
						else
						{
							MSG     = "FLST011O"   ;
							RSN     = ec.message() ;
							MESSAGE = ec.message() ;
							log( "E", "Delete of " + entry + " failed with " + ec.message() << endl ) ;
						}
					}
				}
				tbput( DSLIST ) ;
			}
			else if ( SEL == "M" )
			{
				ZCMD     = "" ;
				SEL      = "" ;
				modifyAttrs( entry ) ;
				tbput( DSLIST ) ;
			}
			else if ( SEL == "R" )
			{
				ZCMD     = "" ;
				SEL      = "" ;
				NEWENTRY = "" ;
				display( "PFLST0A4", MSG, "ZCMD" ) ;
				if ( RC > 8  ) { abend() ; }
				if ( ZCMD == "CAN" || ZCMD == "CANCEL" || RC == 8 )
				{
					ZCMD    = ""          ;
					MSG     = "FLST011S"  ;
					MESSAGE = "Cancelled" ;
				}
				else
				{
					if ( NEWENTRY[ 0 ] != '/' ) { NEWENTRY = substr( entry, 1, lastpos( "/", entry ) ) + NEWENTRY ; }
					if ( rename( entry.c_str(), NEWENTRY.c_str() ) == 0 )
					{
						MSG     = "FLST011Q" ;
						MESSAGE = "Renamed"  ;
					}
					else
					{
						if ( errno == EXDEV )
						{
							MSG     = "FLST011Z" ;
							MESSAGE = "Use COPY" ;
						}
						else
						{
							MSG     = "FLST011R"        ;
							RSN     = strerror( errno ) ;
							MESSAGE = strerror( errno ) ;
						}
						log( "E", "Rename of " + entry + " to " + NEWENTRY + " failed with " + strerror( errno ) << endl ) ;
					}
				}
				tbput( DSLIST ) ;
			}
			else if ( is_directory( entry ) && (SEL == "S" || SEL == "B" || SEL == "L") )
			{
				vcopy( "ZFLSTPGM", PGM, MOVE ) ;
				select( "PGM(" + PGM + ") PARM(" + entry + ")" ) ;
				SEL = "" ;
				if ( ZRESULT != "" ) { MESSAGE = ZRESULT  ; }
				else                 { MESSAGE = "Listed" ; }
				tbput( DSLIST )     ;
			}
			else if ( is_regular_file( entry ) && (SEL == "FMT") )
			{
				vcopy( "ZOREXPGM", PGM, MOVE ) ;
				select( "PGM(" + PGM + ") PARM(porexx2 " + entry + ")" ) ;
				SEL = "" ;
				if ( ZRESULT != "" ) { MESSAGE = ZRESULT    ; MSG = "FLST01M" ; }
				else                 { MESSAGE = "Executed" ;                   }
				if ( ZRC == 0 ) { browse( "/tmp/porexx2.say" ) ; }
				tbput( DSLIST ) ;
			}
			else if ( is_regular_file( entry ) && findword( SEL, "B S E L V" ) )
			{
				if ( SEL == "E" )
				{
					edit( entry ) ;
					SEL = "" ;
					if ( ZRESULT != "" ) { MESSAGE = ZRESULT  ; }
					else                 { MESSAGE = "Edited" ; }
					tbput( DSLIST )     ;
				}
				else if ( SEL == "V" )
				{
					view( entry ) ;
					SEL = "" ;
					if ( ZRESULT != "" ) { MESSAGE = ZRESULT  ; }
					else                 { MESSAGE = "Viewed" ; }
					tbput( DSLIST )     ;
				}
				else
				{
					browse( entry ) ;
					SEL = "" ;
					if ( ZRESULT != "" ) { MESSAGE = ZRESULT   ; }
					else                 { MESSAGE = "Browsed" ; }
					tbput( DSLIST )     ;
				}
				vcopy( "ZRFURL", t, MOVE ) ;
				if ( t == "YES" )
				{
					vcopy( "ZRFLPGM", PGM, MOVE ) ;
					select( "PGM("+PGM+") PARM(PLA "+entry+")" ) ;
				}
			}
			else if ( is_regular_file( entry ) && (SEL == "NANO" ) )
			{
				t = "nano " + entry ;
				control( "TIMEOUT", "DISABLE" ) ;
				def_prog_mode()     ;
				endwin()            ;
				system( t.c_str() ) ;
				reset_prog_mode()   ;
				refresh()           ;
				control( "TIMEOUT", "ENABLE" ) ;
				SEL     = ""        ;
				MESSAGE = "nano"    ;
				tbput( DSLIST )     ;
			}
			else if ( is_regular_file( entry ) && (SEL == "VI" ) )
			{
				t = "vi " + entry   ;
				control( "TIMEOUT", "DISABLE" ) ;
				def_prog_mode()     ;
				endwin()            ;
				system( t.c_str() ) ;
				reset_prog_mode()   ;
				refresh()           ;
				control( "TIMEOUT", "ENABLE" ) ;
				SEL     = ""        ;
				MESSAGE = "vi"      ;
				tbput( DSLIST )     ;
			}
			else if ( SEL == "T" || SEL == "TT" )
			{
				RC = 0 ;
				boost::filesystem::path temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path( ZUSER + "-" + ZSCREEN + "-%%%%-%%%%" ) ;
				string tname = temp.native() ;
				of.open( tname ) ;
				recursive_directory_iterator eIt ;
				recursive_directory_iterator dIt( entry, ec ) ;
				if ( ec.value() != boost::system::errc::success )
				{
					MSG     = "FLST012H"   ;
					RSN     = ec.message() ;
					MESSAGE = ec.message() ;
					SEL     = ""           ;
					tbput( DSLIST )        ;
					of.close()             ;
					remove( tname )        ;
					continue               ;
				}
				of << substr( entry, lastpos( "/", entry )+1 ) << endl ;
				try
				{
					for( ; dIt != eIt ; ++dIt )
					{
						path current( (*dIt) ) ;
						if ( is_regular_file( current ) || is_directory( current ) )
						{
							of << copies( "|   ", dIt.level()) << "|-- " ;
							of << strip( current.filename().string(), 'B', '"' ) << endl ;
							if ( SEL == "T" )
							{
								of << strip( current.string(), 'B', '"' ) << endl ;
							}
						}
					}
				}
				catch ( const filesystem_error& ex )
				{
					MSG     = "FLST012H" ;
					RSN     = ex.what()  ;
					MESSAGE = ex.what()  ;
					SEL     = ""         ;
					tbput( DSLIST )      ;
					of.close()           ;
					remove( tname )      ;
					continue             ;
				}
				of.close()      ;
				if ( SEL == "T" )
				{
					browseTree( tname ) ;
				}
				else
				{
					browse( tname ) ;
				}
				if ( RC > 0 )
				{
					MSG     = "FLST012H"      ;
					MESSAGE = "Unknown Error" ;
					tbput( DSLIST )           ;
				}
				remove( tname ) ;
			}
			else
			{
				SEL     = ""        ;
				MESSAGE = "Invalid" ;
				tbput( DSLIST )     ;
			}
			if ( ZTDSELS > 1 )
			{
				tbdispl( DSLIST ) ;
				if ( RC > 4 ) break ;
			}
			else { ZTDSELS = 0 ; }
		}
	}
	tbend( DSLIST ) ;
	cleanup() ;
	return    ;
}


void PFLST0A::createFileList1( string filter )
{
	int i    ;
	string p ;
	string t ;

	bool fGen ;

	vector<string>::iterator itv ;

	struct stat results   ;
	struct tm * time_info ;
	char buf[ 20 ]        ;

	typedef vector< path > vec ;

	tbcreate( DSLIST, "", "MESSAGE SEL ENTRY TYPE PERMISS SIZE STCDATE MODDATE MODDATES", NOWRITE ) ;

	MESSAGE = ""    ;
	SEL     = ""    ;
	iupper( filter ) ;

	fGen = ( filter.find_first_of( "?*" ) != string::npos ) ;

	vcopy( "AFHIDDEN", t, MOVE ) ;

	vec v;

	try
	{
		copy( directory_iterator( ZPATH ), directory_iterator(), back_inserter( v ) ) ;
	}
	catch ( const filesystem_error& ex )
	{
		ZRESULT = "List Error" ;
		RC      = 16 ;
		log( "E", "Error listing directory " << ex.what() << endl ) ;
		return ;
	}
	sort( v.begin(), v.end() ) ;

	for ( vec::const_iterator it (v.begin()) ; it != v.end() ; ++it )
	{
		ENTRY   = (*it).string() ;
		p       = ENTRY          ;
		i       = lastpos( "/", ENTRY ) + 1 ;
		ENTRY   = substr( ENTRY, i ) ;
		if ( t != "/" && ENTRY[ 0 ] == '.' ) { continue ; }
		if ( fGen )
		{
			if ( !matchpattern( filter, upper( ENTRY ) ) ) { continue ; }
		}
		else
		{
			if ( filter != "" && pos( filter, upper( ENTRY ) ) == 0 ) { continue ; }
		}
		if ( UseSearch )
		{
			itv = find( SearchList.begin(), SearchList.end(), (*it).string() ) ;
			if ( itv == SearchList.end() ) { continue ; }
		}
		MESSAGE = "";
		lstat( p.c_str(), &results ) ;
		if ( S_ISDIR( results.st_mode ) )       TYPE = "Dir"     ;
		else if ( S_ISREG( results.st_mode ) )  TYPE = "File"    ;
		else if ( S_ISCHR( results.st_mode ) )  TYPE = "Char"    ;
		else if ( S_ISBLK( results.st_mode ) )  TYPE = "Block"   ;
		else if ( S_ISFIFO( results.st_mode ) ) TYPE = "Fifo"    ;
		else if ( S_ISSOCK( results.st_mode ) ) TYPE = "Socket"  ;
		else if ( S_ISLNK(results.st_mode ) )   TYPE = "Syml"    ;
		else                                    TYPE = "Unknown" ;
		PERMISS = string( 10, '-' ) ;
		if ( S_ISDIR(results.st_mode) )  { PERMISS[ 0 ] = 'd' ; }
		if ( S_ISLNK(results.st_mode) )  { PERMISS[ 0 ] = 'l' ; }
		if ( results.st_mode & S_IRUSR ) { PERMISS[ 1 ] = 'r' ; }
		if ( results.st_mode & S_IWUSR ) { PERMISS[ 2 ] = 'w' ; }
		if ( results.st_mode & S_IXUSR ) { PERMISS[ 3 ] = 'x' ; }
		if ( results.st_mode & S_IRGRP ) { PERMISS[ 4 ] = 'r' ; }
		if ( results.st_mode & S_IWGRP ) { PERMISS[ 5 ] = 'w' ; }
		if ( results.st_mode & S_IXGRP ) { PERMISS[ 6 ] = 'x' ; }
		if ( results.st_mode & S_IROTH ) { PERMISS[ 7 ] = 'r' ; }
		if ( results.st_mode & S_IWOTH ) { PERMISS[ 8 ] = 'w' ; }
		if ( results.st_mode & S_IXOTH ) { PERMISS[ 9 ] = 'x' ; }
		SIZE      = d2ds( results.st_size )   ;
		MODDATES  = d2ds( results.st_mtime )  ;
		time_info = gmtime( &(results.st_mtime) ) ;
		strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info )  ; buf[ 19 ]  = 0x00 ; MODDATE = buf ;
		SEL       = ""  ;
		tbadd( DSLIST ) ;
	}
	tbtop( DSLIST ) ;
}


void PFLST0A::createSearchList( const string& w )
{
	int p1 ;

	string cmd ;
	string t1  ;
	string t2  ;
	string dparms ;
	string result ;
	string file   ;
	string v_list ;
	string tname1 ;
	string inLine ;
	string t      ;

	path temp1 ;

	char buffer[256] ;
	string quote( 1, '\"' ) ;

	vector<string>::iterator it ;

	vcopy( "ZUSER", ZUSER, MOVE ) ;
	vcopy( "ZSCREEN", ZSCREEN, MOVE ) ;

	temp1  = temp_directory_path() / unique_path( ZUSER + "-" + ZSCREEN + "-%%%%-%%%%" ) ;
	tname1 = temp1.native() ;

	if ( ZPATH.back() != '/' ) { ZPATH = ZPATH + "/" ; }
	cmd = "grep -i -s \"" + w + "\" " + ZPATH + "*" ;

	std::ofstream of ;
	of.open( tname1 ) ;
	FILE* pipe{popen( cmd.c_str(), "r" ) } ;
	while( fgets( buffer, sizeof( buffer ), pipe ) != nullptr )
	{
		file   = buffer ;
		result = file.substr(0, file.size() - 1 ) ;
		of << result << endl ;
	}
	pclose( pipe ) ;
	of.close()     ;

	SearchList.clear() ;
	std::ifstream fin( tname1.c_str() ) ;
	while ( getline( fin, inLine ) )
	{
		p1 = inLine.find( ':' ) ;
		if ( p1 == string::npos ) { continue ; }
		t  = inLine.substr( 0, p1 ) ;
		it = find( SearchList.begin(), SearchList.end(), t ) ;
		if ( it == SearchList.end() )
		{
			SearchList.push_back( t ) ;
		}
	}
	fin.close() ;
	remove( tname1 ) ;
}


void PFLST0A::showInfo( const string& p )
{
	struct stat results   ;
	struct tm * time_info ;
	char buf [ 20 ] ;
	char * buffer   ;
	size_t bufferSize = 255 ;
	size_t rc               ;

	try
	{
		if ( !exists( p ) )
		{
			ZRC  = 8  ;
			ZRSN = 12 ;
			return    ;
		}
	}
	catch ( const filesystem_error& ex )
	{
		ZRC  = 8  ;
		ZRSN = 16 ;
		setmsg( "FLST014" ) ;
		return    ;
	}

	if ( lstat( p.c_str(), &results ) != 0 )
	{
		ZRC  = 12 ;
		ZRSN = 20 ;
		return    ;
	}

	vdefine( "IENTRY ITYPE  IINODE INLNKS IPERMISS ISIZE    ISTCDATE IMODDATE", &IENTRY, &ITYPE,  &IINODE, &INLNKS, &IPERMISS, &ISIZE,    &ISTCDATE, &IMODDATE ) ;
	vdefine( "IRLNK  IOWNER IGROUP IMAJ   IMIN     IBLKSIZE IACCDATE ISETUID",  &IRLNK,  &IOWNER, &IGROUP, &IMAJ,   &IMIN,     &IBLKSIZE, &IACCDATE, &ISETUID  ) ;
	vdefine( "ISETGID ISTICKY", &ISETGID, &ISTICKY ) ;

	IENTRY = p ;
	if ( S_ISDIR( results.st_mode ) )       ITYPE = "Directory"     ;
	else if ( S_ISREG( results.st_mode ) )  ITYPE = "File"          ;
	else if ( S_ISCHR( results.st_mode ) )  ITYPE = "Character"     ;
	else if ( S_ISBLK( results.st_mode ) )  ITYPE = "Block"         ;
	else if ( S_ISFIFO( results.st_mode ) ) ITYPE = "Fifo"          ;
	else if ( S_ISSOCK( results.st_mode ) ) ITYPE = "Socket"        ;
	else if ( S_ISLNK(results.st_mode ) )   ITYPE = "Symbolic link" ;
	else                                    ITYPE = "Unknown"       ;

	IOWNER = "" ;
	IGROUP = "" ;
	struct passwd *pw = getpwuid( results.st_uid ) ;
	if ( pw != NULL)
	{
		IOWNER = pw->pw_name ;
	}

	struct group  *gr = getgrgid( results.st_gid ) ;
	if ( gr != NULL)
	{
		IGROUP = gr->gr_name ;
	}

	IPERMISS = string( 9, '-' ) ;
	if ( results.st_mode & S_IRUSR ) { IPERMISS[ 0 ] = 'r' ; }
	if ( results.st_mode & S_IWUSR ) { IPERMISS[ 1 ] = 'w' ; }
	if ( results.st_mode & S_IXUSR ) { IPERMISS[ 2 ] = 'x' ; }
	if ( results.st_mode & S_IRGRP ) { IPERMISS[ 3 ] = 'r' ; }
	if ( results.st_mode & S_IWGRP ) { IPERMISS[ 4 ] = 'w' ; }
	if ( results.st_mode & S_IXGRP ) { IPERMISS[ 5 ] = 'x' ; }
	if ( results.st_mode & S_IROTH ) { IPERMISS[ 6 ] = 'r' ; }
	if ( results.st_mode & S_IWOTH ) { IPERMISS[ 7 ] = 'w' ; }
	if ( results.st_mode & S_IXOTH ) { IPERMISS[ 8 ] = 'x' ; }

	if ( results.st_mode & S_ISUID ) { ISETUID = "YES"; }
	else                             { ISETUID = "NO "; }
	if ( results.st_mode & S_ISGID ) { ISETGID = "YES"; }
	else                             { ISETGID = "NO "; }
	if ( results.st_mode & S_ISVTX ) { ISTICKY = "YES"; }
	else                             { ISTICKY = "NO "; }

	IINODE = d2ds( results.st_ino )   ;
	INLNKS = d2ds( results.st_nlink ) ;
	ISIZE  = d2ds( results.st_size )  ;

	time_info = gmtime( &(results.st_ctime) ) ;
	strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info )  ; buf[ 19 ]  = 0x00 ; ISTCDATE = buf ;

	time_info = gmtime( &(results.st_mtime) ) ;
	strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info )  ; buf[ 19 ]  = 0x00 ; IMODDATE = buf ;

	time_info = gmtime( &(results.st_atime) ) ;
	strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info )  ; buf[ 19 ]  = 0x00 ; IACCDATE = buf ;


	IRLNK = "" ;
	if ( S_ISLNK(results.st_mode ) )
	{
		while ( true )
		{
			buffer = new char[ bufferSize ] ;
			rc     = readlink( p.c_str(), buffer, bufferSize ) ;
			if ( rc == -1 )
			{
				delete[] buffer ;
				if( errno == ENAMETOOLONG ) { bufferSize += 255; }
				else                        { break ;            }
			}
			else
			{
				IRLNK = string( buffer, rc ) ;
				delete[] buffer ;
				break           ;
			}
		}
	}

	IMAJ     = d2ds( major( results.st_dev ) ) ;
	IMIN     = d2ds( minor( results.st_dev ) ) ;
	IBLKSIZE = d2ds( results.st_blksize )      ;

	while ( true )
	{
		display( "PFLST0A2" ) ;
		if ( RC  > 8 ) { abend() ;         }
		if ( RC == 8 ) { RC = 0  ; break ; }
	}
	vdelete( "IENTRY   ITYPE    IINODE  INLNKS  IPERMISS ISIZE ISTCDATE IMODDATE IOWNER IGROUP IRLNK IMAJ IMIN" ) ;
	vdelete( "IBLKSIZE IACCDATE ISETUID ISETGID ISTICKY" ) ;
}


int PFLST0A::processPrimCMD()
{
	string cw  ;
	string ws  ;
	string w2  ;
	string w3  ;
	string PGM ;
	string p   ;
	string t   ;

	std::ofstream of ;
	boost::system::error_code ec ;

	cw    = upper( word( ZCMD, 1 ) ) ;
	ws    = subword( ZCMD, 2 ) ;
	w2    = word( ZCMD, 2 )    ;
	w3    = word( ZCMD, 3 )    ;

	if ( cw == "" ) { return  0 ; }
	else if ( cw == "SORT" )
	{
		iupper( w2 ) ;
		iupper( w3 ) ;
		if ( w2 == "" ) { w2 = "ENTRY" ; }
		if ( w3 == "" ) { w3 = "A"     ; }
		if      ( w2 == "ENTRY"   ) { tbsort( DSLIST, "ENTRY,C,"+w3   )  ; }
		else if ( w2 == "TYPE"    ) { tbsort( DSLIST, "TYPE,C,"+w3    )  ; }
		else if ( w2 == "PERMISS" ) { tbsort( DSLIST, "PERMISS,C,"+w3 )  ; }
		else if ( w2 == "SIZE"    ) { tbsort( DSLIST, "SIZE,N,"+w3    )  ; }
		else if ( w2 == "MOD"     ) { tbsort( DSLIST, "MODDATES,N,"+w3 ) ; }
		else if ( w2 == "CHA"     ) { tbsort( DSLIST, "MODDATES,N,"+w3 ) ; }
		else                        { return 8 ;                           }
		if ( RC > 0 ) { MSG = "FLST016" ; return 4 ; }
		return 0 ;
	}
	else if ( (cw == "S" || cw == "B" || cw == "E" ) && ws != "" )
	{
		p = ZPATH + "/" + ws ;
		if ( is_directory( p ) && (cw == "S" || cw == "B" ) )
		{
			vcopy( "ZFLSTPGM", PGM, MOVE ) ;
			select( "PGM(" + PGM + ") PARM(" + p + ")" ) ;
			ZCMD = "" ;
		}
		else if ( is_regular_file( p ) && (cw == "S" || cw == "B" || cw == "E" || cw == "L" ) )
		{
			if ( cw == "E" ) { edit( p )   ; }
			else             { browse( p ) ; }
			ZCMD = "" ;
		}
		return 0 ;
	}
	else if ( cw == "BACK" | cw == "S" )
	{
		if ( ZPATH.back() == '/' ) { ZPATH = ZPATH.substr( 0, ZPATH.size()-1) ; }
		ZPATH = substr( ZPATH, 1, lastpos( "/", ZPATH)-1 ) ;
		return 0 ;
	}
	else if ( cw == "CD" )
	{
		if ( ws == "" ) { ws = ZHOME ; }
		t = ZPATH ;
		if ( t.back() == '/' ) { t = t.erase( t.size()-1, 1) ; }
		if ( substr( ws, 1, 1 ) != "/" ) { t += "/" + ws ; }
		else                             { t  = ws       ; }
		if ( t.find_first_of( "*?" ) != string::npos ) { t = expandName( t ) ; }
		if ( t == "" ) { return 8 ; }
		if ( is_directory( t ) ) { ZPATH = t ; return 0 ; }
		else { return 8 ; }
	}
	else if ( cw == "L" )
	{
		tbvclear( DSLIST ) ;
		ENTRY = word( ws, 1 ) ;
		if ( ENTRY.back() != '*' ) { ENTRY.push_back( '*' ) ; }
		if ( w3 == "PREV" )
		{
			tbsarg( DSLIST, "", "PREVIOUS", "(ENTRY,LE)" ) ;
			tbscan( DSLIST, "", "", "", "", "", "CRP"  ) ;
		}
		else
		{
			if ( w3 == "FIRST" )
			{
				tbsarg( DSLIST, "", "NEXT", "(ENTRY,GE)" ) ;
				tbtop( DSLIST )  ;
				tbscan( DSLIST, "", "", "", "", "", "CRP" ) ;
			}
			else
			{
				if ( w3 == "LAST" )
				{
					tbsarg( DSLIST, "", "PREVIOUS", "(ENTRY,LE)" ) ;
					tbbottom( DSLIST ) ;
					tbscan( DSLIST, "", "", "", "", "", "CRP"  ) ;
				}
				else
				{
					tbsarg( DSLIST, "", "NEXT", "(ENTRY,GE)" ) ;
					tbscan( DSLIST, "", "", "", "", "", "CRP" ) ;
				}
			}
		}
		return 4 ;
	}
	else if ( cw == "MKDIR" )
	{
		if ( ws[ 0 ] != '/' ) { ws = ZPATH + "/" + ws ; }
		create_directory( ws, ec ) ;
		if ( ec.value() == boost::system::errc::success )
		{
			MSG = "FLST012B" ;
		}
		else
		{
			MSG = "FLST012C"   ;
			RSN = ec.message() ;
			log( "E", "Create of direcotry " + ws + " failed with " + ec.message() << endl ) ;
		}
		ZCMD = "" ;
		return 4  ;
	}
	else if ( cw == "TOUCH" )
	{
		if ( ws[ 0 ] != '/' ) { ws = ZPATH + "/" + ws ; }
		if ( !exists( ws ) )
		{
			of.open( ws.c_str() ) ;
			if ( !of.fail() )
			{
				of << endl ;
				of.close() ;
				MSG = "FLST012E" ;
			}
			else { MSG = "FLST012F" ; }
		}
		else
		{
			MSG = "FLST012D" ;
		}
		ZCMD = "" ;
		return 4  ;
	}
	return 8 ;
}


void PFLST0A::copyDirs( const string& src, const string& dest, const string& DIRREC, bool& errs )
{
	boost::system::error_code ec ;

	copy_directory( src, dest, ec ) ;
	if ( ec.value() != boost::system::errc::success )
	{
		errs    = true         ;
		MSG     = "FLST011V"   ;
		RSN     = ec.message() ;
		MESSAGE = ec.message() ;
		log( "E", "Copy of directory" + src + " to " + dest + " failed with " + ec.message() << endl ) ;
		return ;
	}

	for( directory_iterator file( src, ec ) ; file != directory_iterator() ; ++file )
	{
		if ( ec.value() != boost::system::errc::success )
		{
			errs = true ;
			log( "E", "Listing directory " << src << " failed with " + ec.message() << endl ) ;
			return ;
		}
		path current( file->path() ) ;
		if( is_directory( current ) )
		{
			if ( DIRREC == "/" )
			{
				copyDirs( current.string(), dest + "/" + current.filename().string(), DIRREC, errs ) ;
			}
			else
			{
				copy_directory( current, dest / current.filename(), ec ) ;
				if ( ec.value() != boost::system::errc::success )
				{
					errs = true ;
					log( "E", "Copy of directory " << current.filename() << " failed with " + ec.message() << endl ) ;
				}
			}
		}
		else if ( is_regular_file( current ) )
		{
			copy_file( current, dest / current.filename(), ec ) ;
			if ( ec.value() != boost::system::errc::success )
			{
				errs = true ;
				log( "E", "Copy of file " << current.filename() << " failed with " + ec.message() << endl ) ;
			}
		}
		else if ( is_symlink( current ) )
		{
			copy_symlink( current, dest / current.filename(), ec ) ;
			if ( ec.value() != boost::system::errc::success )
			{
				errs = true ;
				log( "E", "Copy of symlink " << current.filename() << " failed with " + ec.message() << endl ) ;
			}
		}
		else
		{
			errs = true ;
			log( "E", "Ignoring entry " << current.filename() << " Not a regular file, directory or symlink" << endl ) ;
		}
	}
}


void PFLST0A::modifyAttrs( const string& p )
{
	int i  ;
	int i1 ;
	int i2 ;
	int i3 ;

	bool changed    ;

	string OPERMISS ;
	string OSETUID  ;
	string OSETGID  ;
	string OSTICKY  ;
	string OOWNER   ;
	string OGROUP   ;
	string OOWNERN  ;
	string OGROUPN  ;

	mode_t t  ;
	uid_t uid ;
	gid_t gid ;

	struct stat results   ;

	vdefine( "IENTRY ITYPE  IPERMISS ", &IENTRY, &ITYPE, &IPERMISS ) ;
	vdefine( "IOWNER IGROUP ISETUID", &IOWNER, &IGROUP, &ISETUID  ) ;
	vdefine( "ISETGID ISTICKY IOWNERN IGROUPN", &ISETGID, &ISTICKY, &IOWNERN, &IGROUPN ) ;

	lstat( p.c_str(), &results ) ;

	IENTRY = p ;
	if ( S_ISDIR( results.st_mode ) )       ITYPE = "Directory"     ;
	else if ( S_ISREG( results.st_mode ) )  ITYPE = "File"          ;
	else if ( S_ISCHR( results.st_mode ) )  ITYPE = "Character"     ;
	else if ( S_ISBLK( results.st_mode ) )  ITYPE = "Block"         ;
	else if ( S_ISFIFO( results.st_mode ) ) ITYPE = "Fifo"          ;
	else if ( S_ISSOCK( results.st_mode ) ) ITYPE = "Socket"        ;
	else if ( S_ISLNK(results.st_mode ) )   ITYPE = "Symbolic link" ;
	else                                    ITYPE = "Unknown"       ;

	struct passwd *pwd ;
	struct group  *grp ;

	IOWNER  = "" ;
	IOWNERN = "" ;
	pwd = getpwuid( results.st_uid ) ;
	if ( pwd != NULL)
	{
		IOWNER  = pwd->pw_name ;
		IOWNERN = d2ds( pwd->pw_uid ) ;
	}

	IGROUP  = "" ;
	IGROUPN = "" ;
	grp = getgrgid( results.st_gid ) ;
	if ( grp != NULL)
	{
		IGROUP  = grp->gr_name ;
		IGROUPN = d2ds( grp->gr_gid ) ;
	}

	i1 = 0 ;
	i2 = 0 ;
	i3 = 0 ;
	if ( results.st_mode & S_IRUSR ) { i1 = i1 + 4 ; }
	if ( results.st_mode & S_IWUSR ) { i1 = i1 + 2 ; }
	if ( results.st_mode & S_IXUSR ) { i1 = i1 + 1 ; }
	if ( results.st_mode & S_IRGRP ) { i2 = i2 + 4 ; }
	if ( results.st_mode & S_IWGRP ) { i2 = i2 + 2 ; }
	if ( results.st_mode & S_IXGRP ) { i2 = i2 + 1 ; }
	if ( results.st_mode & S_IROTH ) { i3 = i3 + 4 ; }
	if ( results.st_mode & S_IWOTH ) { i3 = i3 + 2 ; }
	if ( results.st_mode & S_IXOTH ) { i3 = i3 + 1 ; }

	IPERMISS = d2ds( i1 ) + d2ds( i2 ) + d2ds( i3 ) ;

	if ( results.st_mode & S_ISUID ) { ISETUID = "/" ; }
	else                             { ISETUID = ""  ; }
	if ( results.st_mode & S_ISGID ) { ISETGID = "/" ; }
	else                             { ISETGID = ""  ; }
	if ( results.st_mode & S_ISVTX ) { ISTICKY = "/" ; }
	else                             { ISTICKY = ""  ; }

	OSETUID  = ISETUID  ;
	OSETGID  = ISETGID  ;
	OSTICKY  = ISTICKY  ;
	OPERMISS = IPERMISS ;
	OOWNER   = IOWNER   ;
	OGROUP   = IGROUP   ;
	OOWNERN  = IOWNERN  ;
	OGROUPN  = IGROUPN  ;
	MSG      = ""       ;
	t        = results.st_mode ;
	changed  = false    ;

	display( "PFLST0A6", MSG, "ZCMD" ) ;
	if ( RC  > 8 ) { abend() ; }
	if ( ZCMD == "CANCEL" || RC == 8 )
	{
		ZCMD    = ""          ;
		MSG     = "FLST017"   ;
		MESSAGE = "Cancelled" ;
	}
	else
	{
		if ( IPERMISS != OPERMISS )
		{
			i = ds2d( string( 1, IPERMISS[ 0 ] ) ) ;
			if ( i >= 4 ) { t = t |  S_IRUSR ; i = i - 4 ; }
			else          { t = t & ~S_IRUSR ;             }
			if ( i >= 2 ) { t = t |  S_IWUSR ; i = i - 2 ; }
			else          { t = t & ~S_IWUSR ;             }
			if ( i == 1 ) { t = t |  S_IXUSR ; }
			else          { t = t & ~S_IXUSR ; }
			i = ds2d( string( 1, IPERMISS[ 1 ] ) ) ;
			if ( i >= 4 ) { t = t |  S_IRGRP ; i = i - 4 ; }
			else          { t = t & ~S_IRGRP ;             }
			if ( i >= 2 ) { t = t |  S_IWGRP ; i = i - 2 ; }
			else          { t = t & ~S_IWGRP ;             }
			if ( i == 1 ) { t = t |  S_IXGRP ; }
			else          { t = t & ~S_IXGRP ; }
			i = ds2d( string( 1, IPERMISS[ 2 ] ) ) ;
			if ( i >= 4 ) { t = t |  S_IROTH ; i = i - 4 ; }
			else          { t = t & ~S_IROTH ;             }
			if ( i >= 2 ) { t = t |  S_IWOTH ; i = i - 2 ; }
			else          { t = t & ~S_IWOTH ;             }
			if ( i == 1 ) { t = t |  S_IXOTH ; }
			else          { t = t & ~S_IXOTH ; }
		}
		if ( ISETUID != OSETUID )
		{
			if ( ISETUID == "/" ) { t = t |  S_ISUID ; }
			else                  { t = t & ~S_ISUID ; }
		}
		if ( ISETGID != OSETGID )
		{
			if ( ISETGID == "/" ) { t = t |  S_ISGID ; }
			else                  { t = t & ~S_ISGID ; }
		}
		if ( ISTICKY != OSTICKY )
		{
			if ( ISTICKY == "/" ) { t = t |  S_ISVTX ; }
			else                  { t = t & ~S_ISVTX ; }
		}
		if ( t != results.st_mode )
		{
			changed = true ;
			if ( chmod( p.c_str(), t ) != 0 ) { MSG = "FLST018" ; }
		}
		if ( MSG == "" && (IOWNERN != OOWNERN) )
		{
			changed = true ;
			pwd = getpwuid( ds2d( IOWNERN ) ) ;
			if ( pwd == NULL )
			{
				MSG = "FLST019" ;
			}
			else
			{
				changed = true    ;
				uid = pwd->pw_uid ;
				if ( chown( p.c_str(), uid, -1 ) == -1 ) { MSG = "FLST012I" ; }
			}
		}
		else if ( MSG == "" && (IOWNER != OOWNER) )
		{
			changed = true ;
			pwd = getpwnam( IOWNER.c_str() ) ;
			if ( pwd == NULL )
			{
				MSG = "FLST011B" ;
			}
			else
			{
				uid = pwd->pw_uid ;
				if ( chown( p.c_str(), uid, -1 ) == -1 ) { MSG = "FLST012I" ; }
			}
		}
		if ( MSG == "" && (IGROUPN != OGROUPN) )
		{
			changed = true ;
			grp = getgrgid( ds2d( IGROUPN ) ) ;
			if ( grp == NULL )
			{
				MSG = "FLST011L" ;
			}
			else
			{
				gid = grp->gr_gid ;
				if ( chown( p.c_str(), -1, gid ) == -1 ) { MSG = "FLST012J" ; }
			}
		}
		else if ( MSG == "" && (IGROUP != OGROUP) )
		{
			changed = true ;
			grp = getgrnam( IGROUP.c_str() ) ;
			if ( grp == NULL )
			{
				MSG = "FLST011C" ;
			}
			else
			{
				gid = grp->gr_gid ;
				if ( chown( p.c_str(), -1, gid ) == -1 ) { MSG = "FLST012J" ; }
			}
		}
		stat( p.c_str(), &results ) ;
		if ( changed )
		{
			if ( MSG != "" ) { MESSAGE = "Modify Failed" ; }
			else             { MESSAGE = "Modified"      ; MSG = "FLST011A" ; }
		}
	}
	vdelete( "IENTRY ITYPE  IPERMISS IOWNER IGROUP IOWNERN IGROUPN ISETUID ISETGID ISTICKY" ) ;
}


void PFLST0A::browseTree( const string& tname )
{
	int i ;

	string TSEL   ;
	string TFILE  ;
	string TENTRY ;
	string FTREE  ;

	string line   ;
	string PGM    ;

	std::ifstream fin ;
	fin.open( tname.c_str() ) ;
	if ( !fin.is_open() )
	{
		RC   = 16 ;
		log( "E", "Error opening file " << tname << endl ) ;
		return    ;
	}

	vdefine( "TSEL TFILE TENTRY ZDIR", &TSEL, &TFILE, &TENTRY, &ZDIR ) ;
	vcopy( "ZFLSTPGM", PGM, MOVE ) ;
	FTREE = "FTREE" + right( d2ds( taskid() ), 3, '0' ) ;

	tbcreate( FTREE, "", "TSEL TFILE TENTRY", NOWRITE ) ;

	TSEL = "" ;
	i    = 1  ;
	while ( getline( fin, line ) )
	{
		if ( i == 1 )
		{
			ZDIR = strip( line ) ;
		}
		else if ( i % 2 )
		{
			TFILE = strip( line ) ;
			tbadd( FTREE ) ;
		}
		else
		{
			TENTRY  = strip( line ) ;
		}
		i++ ;

	}
	tbtop( FTREE ) ;

	while ( true )
	{
		if ( ZTDVROWS > 0 )
		{
			tbtop( FTREE )     ;
			tbskip( FTREE, i ) ;
		}
		else
		{
			tbbottom( FTREE ) ;
			tbskip( FTREE, - (ZTDDEPTH-2) ) ;
			if ( RC > 0 ) { tbtop( FTREE ) ; }
		}
		if ( MSG == "" ) { ZCMD  = "" ; }
		tbdispl( FTREE, "PFLST0A8", MSG, "ZCMD" ) ;
		if ( RC  > 8 ) { abend() ; }
		if ( RC == 8 ) { break   ; }
		MSG = "" ;
		i = ZTDTOP ;
		while ( ZTDSELS > 0 )
		{
			if ( TSEL == ""  ) {}
			else if ( TSEL == "S" )
			{
				select( "PGM(" + PGM + ") PARM(BROWSE " + TFILE + ")" ) ;
			}
			else if ( TSEL == "I" )
			{
				select( "PGM(" + PGM + ") PARM(INFO " + TFILE + ")" ) ;
			}
			if ( ZTDSELS > 1 )
			{
				tbdispl( FTREE ) ;
				if ( RC > 4 ) break ;
			}
			else { ZTDSELS = 0 ; }
		}
	}
	tbend( FTREE ) ;
	return         ;

}


string PFLST0A::expandDir( const string& parms )
{
	// If passed directory begins with '?', display listing replacing '?' with '/' and using this
	// as the starting point

	// Cursor sensitive.  Only characters before the current cursor position (if > 1) in the field (ZFECSRP) will
	// be taken into account

	// If passed directory matches a listing entry, return the next entry (if not end-of-list)

	// If passed directory is an abbreviation of one in the listing, return the current entry

	// If first parameter is ALL, all entries are used
	// If first parameter is DO1, filter on directories
	// If first parameter is FO2, filter on files

	int i        ;
	int pos      ;

	bool showl   ;

	string type  ;
	string dir1  ;
	string entry ;
	string dir   ;
	string cpos  ;
	string data1 ;
	string data2 ;

	ZRC = 8 ;
	data1 = "" ;
	data2 = "" ;
	showl = false ;

	typedef vector< path > vec ;
	vec v ;
	vec::iterator new_end ;

	type = word( parms, 1 )    ;
	dir  = subword( parms, 2 ) ;

	vget( "ZFECSRP", SHARED ) ;
	vcopy( "ZFECSRP", cpos, MOVE ) ;
	pos = ds2d( cpos ) ;

	if ( type == "FO2" )
	{
		if ( dir[ 0 ] == '?' )
		{
			showl = true ;
			dir   = ""   ;
		}
		vget( "ZFEDATA1 ZFEDATA2", SHARED ) ;
		vcopy( "ZFEDATA1", data1, MOVE )    ;
		vcopy( "ZFEDATA2", data2, MOVE )    ;
		if ( data1 == "" )
		{
			if ( data2 == "" )
			{
				RC = 8    ;
				return "" ;
			}
			else
			{
				if ( data2.back() == '/' ) { dir = data2 + dir ; pos-- ; }
				else                       { dir = data2 + "/" + dir   ; }
				pos = pos + data2.size() + 1 ;
			}
		}
		else
		{
			if ( data1.back() == '/' ) { dir = data1 + dir ; pos-- ; }
			else                       { dir = data1 + "/" + dir   ; }
			pos = pos + data1.size() + 1 ;
		}
		if ( showl ) { ZPATH = dir ; return showListing() ; }
	}


	if ( dir == "" )
	{
		dir = ZHOME ;
		pos = ZHOME.size() ;
	}
	else if ( dir[ 0 ] == '?' )
	{
		if ( dir.size() > 1 )
		{
			ZPATH = dir ;
			ZPATH[ 0 ] = '/' ;
		}
		else
		{
			ZPATH = ZHOME ;
		}
		return showListing() ;
	}

	if ( pos > 1 && pos < dir.size() )
	{
		dir.erase( pos-1 ) ;
	}

	i    = lastpos( "/", dir ) ;
	dir1 = substr( dir, 1, i ) ;

	try
	{
		copy( directory_iterator( dir1 ), directory_iterator(), back_inserter( v ) ) ;
	}
	catch ( const filesystem_error& ex )
	{
		ZRESULT = "List Error" ;
		RC      = 16 ;
		log( "E", "Error listing directory " << ex.what() << endl ) ;
		return "" ;
	}

	try
	{
		if ( type == "ALL" ) {}
		else if ( type == "DO1" )
		{
			new_end = remove_if( v.begin(), v.end(), [](const path& a) { return !is_directory( a.string() ) ; } ) ;
			v.erase( new_end, v.end() ) ;
		}
		else if ( type == "FO1" || type == "FO2" )
		{
			new_end = remove_if( v.begin(), v.end(), [](const path& a) { return !is_regular_file( a.string() ) ; } ) ;
			v.erase( new_end, v.end() ) ;
		}
		else { RC = 16 ; return "" ; }
	}
	catch ( const filesystem_error& ex )
	{
		ZRESULT = "List Error" ;
		RC      = 16 ;
		log( "E", "Error listing directory " << ex.what() << endl ) ;
		return "" ;
	}

	sort( v.begin(), v.end() ) ;

	for ( vec::const_iterator it (v.begin()) ; it != v.end() ; ++it )
	{
		entry = (*it).string() ;
		if ( !abbrev( entry, dir ) ) { continue ; }
		ZRC = 0 ;
		if ( entry == dir )
		{
			it++ ;
			if ( it != v.end() ) { return (*it).string() ; }
			else                 { return dir            ; }
		}
		break ;
	}
	return entry ;
}


string PFLST0A::expandFld1( const string& parms )
{
	// Expand field from listing of ZPLIB, ZORXPATH or ZLDPATH

	// Cursor sensitive.  Only characters before the current cursor position (if > 1) in the field (ZFECSRP) will
	// be taken into account

	// If first parameter is PNL, panel files
	// If first parameter is REXX, rexx programs
	// If first parameter is PGM, programs (strip off "lib" and ".so" before comparison)

	int i        ;
	int n        ;
	int pos      ;

	string Paths ;
	string type  ;
	string dir1  ;
	string entry ;
	string dir   ;
	string cpos  ;

	ZRC = 8 ;

	typedef vector< path > vec ;
	vec v ;
	vec::iterator vec_end ;

	vector< string > mod ;
	vector< string >::iterator it ;
	vector< string >::iterator mod_end ;

	type = word( parms, 1 )    ;
	dir  = subword( parms, 2 ) ;

	vget( "ZFECSRP", SHARED ) ;
	vcopy( "ZFECSRP", cpos, MOVE ) ;
	pos = ds2d( cpos ) ;

	if ( pos > 1 && pos < dir.size() )
	{
		dir.erase( pos-1 ) ;
	}

	if ( type == "PNL" )
	{
		vcopy( "ZPLIB", Paths, MOVE ) ;
	}
	else if ( type == "REXX" )
	{
		vcopy( "ZORXPATH", Paths, MOVE ) ;
	}
	else if ( type == "PGM" )
	{
		vcopy( "ZLDPATH", Paths, MOVE ) ;
	}
	else { return "" ; }


	n = getpaths( Paths ) ;
	v.clear() ;
	for ( i = 1 ; i <= n ; i++ )
	{
		dir1 = getpath( Paths, i ) ;
		try
		{
			copy( directory_iterator( dir1 ), directory_iterator(), back_inserter( v ) ) ;
		}
		catch ( const filesystem_error& ex )
		{
			ZRESULT = "List Error" ;
			RC      = 16 ;
			log( "E", "Error listing directory " << ex.what() << endl ) ;
			return "" ;
		}
	}

	vec_end = remove_if( v.begin(), v.end(), [](const path& a) { return !is_regular_file( a.string() ) ; } ) ;

	for_each( v.begin(), vec_end,
		[&mod](const path& a) { mod.push_back( substr( a.string(), lastpos( "/", a.string())+1 ) ) ; } ) ;

	mod_end = mod.end() ;
	if ( type == "PGM" )
	{
		mod_end = remove_if( mod.begin(), mod.end(),
			[](const string& a) { return ( a.compare( a.size()-3,3 , ".so" ) > 0 ) ; } ) ;
	}

	sort( mod.begin(), mod_end ) ;

	mod_end = unique( mod.begin(), mod_end ) ;

	for ( it = mod.begin() ; it != mod_end ; ++it )
	{
		entry = (*it) ;
		if ( type == "PGM" ) { entry = getAppName( entry ) ; }
		if ( !abbrev( entry, dir ) ) { continue ; }
		ZRC = 0 ;
		if ( entry == dir )
		{
			it++ ;
			if ( it != mod_end )
			{
				if ( type == "PGM" ) { return getAppName( (*it) ) ; }
				return (*it) ;
			}
			else
			{
				return dir ;
			}
		}
		else { return entry ; }
	}
	return entry ;
}


string PFLST0A::showListing()
{
	string w1       ;
	string w2       ;
	string ws       ;
	string OPATH    ;
	string FLDIRS   ;
	string OFLDIRS  ;
	string FLHIDDEN ;
	string OHIDDEN  ;

	vdefine( "SEL ENTRY TYPE FLDIRS FLHIDDEN", &SEL, &ENTRY, &TYPE, &FLDIRS, &FLHIDDEN ) ;
	vget( "FLDIRS FLHIDDEN", PROFILE ) ;

	DSLIST = "DSLST" + right( d2ds( taskid() ), 3, '0' ) ;
	createFileList2( FLDIRS ) ;

	RC       = 0 ;
	ZTDVROWS = 1 ;
	ZRC      = 0 ;

	while ( true )
	{
		if ( ZTDVROWS > 0 )
		{
			tbtop( DSLIST )     ;
			tbskip( DSLIST, ZTDTOP ) ;
		}
		else
		{
			tbbottom( DSLIST ) ;
			tbskip( DSLIST, - (ZTDDEPTH-2) ) ;
			if ( RC > 0 ) { tbtop( DSLIST ) ; }
		}
		OPATH   = ZPATH    ;
		OFLDIRS = FLDIRS   ;
		OHIDDEN = FLHIDDEN ;
		if ( MSG == "" ) { ZCMD  = "" ; }
		tbdispl( DSLIST, "PFLST0A7", MSG, "ZCMD" ) ;
		if ( RC  > 8 ) { abend() ; }
		if ( RC == 8 ) { ZRC = 8 ; break ; }
		MSG = "" ;
		w1  = upper( word( ZCMD, 1 ) ) ;
		w2  = word( ZCMD, 2 )    ;
		ws  = subword( ZCMD, 2 ) ;
		if ( w1 == "REFRESH" )     { tbend( DSLIST ) ; createFileList2( FLDIRS )     ; continue ; }
		if ( w1 == "O" )           { tbend( DSLIST ) ; createFileList2( FLDIRS, w2 ) ; continue ; }
		if ( w1 == "S" && ZPATH != "/" )
		{
			if ( ZPATH.back() == '/' ) { ZPATH = ZPATH.substr( 0, ZPATH.size()-1) ; }
			ZPATH = substr( ZPATH, 1, lastpos( "/", ZPATH)-1 ) ;
			tbend( DSLIST )   ;
			createFileList2( FLDIRS ) ;
			continue ;
		}
		if ( w1 == "CD" )
		{
			if ( ws == "" ) { ws = ZHOME ; }
			if ( ZPATH.back() == '/' ) { ZPATH = ZPATH.substr( 0, ZPATH.size()-1) ; }
			if ( substr( ws, 1, 1 ) != "/" ) { ZPATH = ZPATH + "/" + ws ; }
			else                             { ZPATH = ws               ; }
			tbend( DSLIST ) ; createFileList2( FLDIRS ) ; continue ;
		}
		vget( "ZVERB", SHARED ) ;
		if ( OFLDIRS != FLDIRS )
		{
			tbend( DSLIST ) ;
			createFileList2( FLDIRS ) ;
			continue ;
		}
		if ( OHIDDEN != FLHIDDEN )
		{
			tbend( DSLIST ) ;
			createFileList2( FLDIRS ) ;
			continue ;
		}
		if ( OPATH != ZPATH )
		{
			if ( ( !exists( ZPATH ) || !is_directory( ZPATH ) ) )
			{
				MSG = "PSYS012A" ;
			}
			else
			{
				tbend( DSLIST ) ;
				createFileList2( FLDIRS ) ;
			}
			continue ;
		}
		while ( ZTDSELS > 0 )
		{
			if ( SEL == ""  ) {}
			else if ( SEL == "L" )
			{
				if ( ZPATH.back() == '/' ) { ZPATH = ZPATH + ENTRY       ; }
				else                       { ZPATH = ZPATH + "/" + ENTRY ; }
				tbend( DSLIST )   ;
				createFileList2( FLDIRS ) ;
			}
			else if ( SEL == "S" )
			{
				if ( ZPATH.back() == '/' ) { ZPATH = ZPATH + ENTRY       ; }
				else                       { ZPATH = ZPATH + "/" + ENTRY ; }
				tbend( DSLIST ) ;
				return ZPATH    ;
			}
			if ( ZTDSELS > 1 )
			{
				tbdispl( DSLIST ) ;
				if ( RC > 4 ) break ;
			}
			else { ZTDSELS = 0 ; }
		}
	}
	tbend( DSLIST ) ;
	return ""       ;
}


void PFLST0A::createFileList2( const string& FLDIRS, string filter )
{
	int i    ;

	string p ;
	string t ;
	struct stat results ;
	typedef vector< path > vec ;

	tbcreate( DSLIST, "", "SEL ENTRY TYPE", NOWRITE ) ;

	vec v;

	if ( ZPATH == "" ) { ZPATH = "/" ; }
	iupper( filter ) ;

	vcopy( "FLHIDDEN", t, MOVE ) ;

	try
	{
		copy( directory_iterator( ZPATH ), directory_iterator(), back_inserter( v ) ) ;
	}
	catch ( const filesystem_error& ex )
	{
		ZRESULT = "List Error" ;
		RC      = 16 ;
		log( "E", "Error listing directory " << ex.what() << endl ) ;
		return ;
	}

	sort( v.begin(), v.end() ) ;
	for ( vec::const_iterator it (v.begin()) ; it != v.end() ; ++it )
	{
		ENTRY   = (*it).string() ;
		p       = ENTRY          ;
		i       = lastpos( "/", ENTRY ) + 1 ;
		ENTRY   = substr( ENTRY, i ) ;
		if ( t != "/" && ENTRY[ 0 ] == '.' ) { continue ; }
		if ( filter != "" && pos( filter, upper( ENTRY ) ) == 0 ) { continue ; }
		lstat( p.c_str(), &results ) ;
		if ( S_ISDIR( results.st_mode ) )       { TYPE = "Dir"     ; }
		else if ( S_ISREG( results.st_mode ) )  { TYPE = "File"    ; }
		else if ( S_ISCHR( results.st_mode ) )  { TYPE = "Char"    ; }
		else if ( S_ISBLK( results.st_mode ) )  { TYPE = "Block"   ; }
		else if ( S_ISFIFO( results.st_mode ) ) { TYPE = "Fifo"    ; }
		else if ( S_ISSOCK( results.st_mode ) ) { TYPE = "Socket"  ; }
		else if ( S_ISLNK(results.st_mode ) )   { TYPE = "Syml"    ; }
		else                                    { TYPE = "Unknown" ; }
		SEL       = ""  ;
		if ( FLDIRS == "/" && TYPE != "Dir" ) { continue ; }
		tbadd( DSLIST ) ;
	}
	tbtop( DSLIST ) ;
}


string PFLST0A::getAppName( string s )
{
	// Remove "lib" at the front and ".so" at the end of a module name

	if ( s.size() > 6 )
	{
		s.erase( 0, 3 ) ;
		return s.erase( s.size() - 3 ) ;
	}
	return s ;
}


string PFLST0A::expandName( const string& s )
{
	// Resolve name if contains * or ?.  If more than one, return ""

	int i  ;
	int p1 ;
	string dir  ;
	string dir1 ;

	typedef vector< path > vec ;

	vec v ;
	vec::iterator new_end ;
	vec::const_iterator it ;

	p1 = s.find_last_of( "/" ) ;
	if ( p1 == string::npos ) { return "" ; }

	dir = s.substr( 0, p1 ) ;

	try
	{
		copy( directory_iterator( dir ), directory_iterator(), back_inserter( v ) ) ;
	}
	catch ( const filesystem_error& ex )
	{
		return "" ;
	}

	new_end = remove_if( v.begin(), v.end(),
		  [](const path& a) { return !is_directory( a.string() ) ; } ) ;
	v.erase( new_end, v.end() ) ;

	dir1 = "" ;
	i    = 0  ;
	for ( i = 0, it = v.begin() ; it != v.end() ; ++it )
	{
		dir = (*it).string() ;
		if ( !matchpattern( s, dir ) ) { continue ; }
		i++ ;
		if ( i > 1 ) { return "" ; }
		dir1 = dir ;
	}
	return dir1 ;
}


// ============================================================================================ //

extern "C" { pApplication *maker() { return new PFLST0A ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
