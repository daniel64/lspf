/* Compile with ::                                                                                          */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPFLST0A.so -o libPFLST0A.so PFLST0A.cpp                      */

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
/* If invoked with a PARM of path, list path                                                                */
/* If invoked with a PARM of INFO, list info for entry                                                      */

/* CD /xxx to change to directory /xxx                                                                      */
/* CD xxx to change to directory xxx under the current directory listing                                    */
/* BACK or S to go back one directory level                                                                 */
/* S xxx to select file or directory xxx from the current list                                              */
/* L xxx FIRST|LAST|PREV to scroll the list to entry xxx (NEXT is the default)                              */ 
/* O  - filter list                                                                                         */
/* REF - refresh directory list                                                                             */
/* MKDIR - Make a directory (under current or specify full path)                                            */
/* TOUCH - Make a file (under current or specify full path)                                                 */

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
/* X  - List link directory                                                                                 */
/* =  - repeat the last-entered line command                                                                */

/* nano - invoke nano editor on the file                                                                    */
/* vi   - invoke vi editor on the file                                                                      */
/*        noTimeOut set for nano and vi or lspf will chop the application after ZMAXWAIT as it does not     */
/*        return until reset_prog_mode                                                                      */ 
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
	int    j        ;
	int    RCode    ;
	int    num      ;

	bool   del      ;
	bool   errs     ;

	string entry    ;
	string OPATH    ;
	string OSEL     ;
	string lp       ;
	string PGM      ;
	string w1       ;
	string w2       ;
	string w3       ;
	string t        ;

	string CONDOFF  ;
	string NEMPTOK  ;
	string DIRREC   ;
	string NEWENTRY ;
	string FREPL    ;
	
	struct stat results ;
	char * buffer       ;
	size_t bufferSize = 255 ;
	size_t rc               ;

	boost::system::error_code ec ;

	vcopy( "ZUSER", ZUSER, MOVE ) ;
	vcopy( "ZSCREEN", ZSCREEN, MOVE ) ;
	
	ofstream of ;

	vdefine( "SEL ENTRY MESSAGE TYPE PERMISS SIZE STCDATE MODDATE", &SEL, &ENTRY, &MESSAGE, &TYPE, &PERMISS, &SIZE, &STCDATE, &MODDATE ) ; 
	vdefine( "MODDATES ZVERB ZHOME ZCMD ZPATH CONDOFF NEWENTRY FREPL", &MODDATES, &ZVERB, &ZHOME, &ZCMD, &ZPATH, &CONDOFF, &NEWENTRY, &FREPL ) ;
	vdefine( "RSN NEMPTOK DIRREC", &RSN, &NEMPTOK, &DIRREC ) ;
	vdefine( "CRP", &CRP ) ;
	
	vget( "ZHOME", SHARED ) ;

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
					if ( w1 == "BROWSE" ) { browse( ZPATH ) ;  cleanup() ; return ;  }
					else                  { edit( ZPATH )   ;  cleanup() ; return ;  }
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
		else { ZPATH = PARM ; }
		if( !is_directory( ZPATH ) ) ZPATH = ZHOME ;
	}

	if ( !is_directory( ZPATH ) ) ZPATH = ZHOME ;

	OPATH  = ZPATH ;
	OSEL   = ""    ;
	DSLIST = "DSLST" + right( d2ds( taskid() ), 3, '0' ) ;
	MSG    = ""  ;

	RC       = 0 ;
	i        = 1 ;
	ZTDVROWS = 1 ;

	createFileList() ;
	if ( RC > 0 ) { setmsg( "FLST01F" ) ; cleanup() ; return ; }

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
		if ( MSG == "" ) { ZCMD  = "" ; }
		tbdispl( DSLIST, "PFLST0A1", MSG, "ZCMD" ) ;
		if ( RC  > 8 ) { abend() ; }
		if ( RC == 8 ) { break   ; }
		MSG = "" ;
		w1  = upper( word( ZCMD, 1 ) ) ;
		w2  = word( ZCMD, 2 ) ;
		w3  = word( ZCMD, 3 ) ;
		if ( w1 == "REF" && w2 == "" )           { tbend( DSLIST ) ; createFileList()     ; continue ; }
		if ( w1 == "O" && w2 != "" && w3 == "" ) { tbend( DSLIST ) ; createFileList( w2 ) ; continue ; }
		i = ZTDTOP ;
		vget( "ZVERB", SHARED ) ;
		CRP    = 0  ;
		RCode  = processPrimCMD() ;
		if ( RCode == 8 ) { MSG = "PSYS018" ; continue ; }
		if ( CRP > 0 ) { i = CRP  ; }
		if ( RCode == 4 ) { continue ; }
		if ( ZPATH == "" ) ZPATH = ZHOME ;
		if ( OPATH != ZPATH )
		{
			if ( ( !exists( ZPATH ) || !is_directory( ZPATH ) ) )
			{
				MSG = "BRENT012" ;
			}
			else
			{
				tbend( DSLIST )  ;
				createFileList() ;
				i     = 1        ;
				OPATH = ZPATH    ;
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
				if ( ZRESULT != "" ) { MESSAGE = ZRESULT    ; MSG = "PSYS01M" ; }
				else                 { MESSAGE = "Executed" ;                   }
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
					MSG     = "FLST0110"     ;
					MESSAGE = "Invalid Type" ;
					tbput( DSLIST ) ;
					continue        ;
				}
				display( "PFLST0A5", MSG, "ZCMD" ) ;
				if ( RC > 8  ) { abend() ; }
				if ( ZCMD == "CAN" || ZCMD == "CANCEL" || RC == 8 )
				{
					ZCMD    = ""          ;
					MSG     = "FLST01W"   ;
					MESSAGE = "Cancelled" ;
				}
				else
				{
					if ( NEWENTRY[ 0 ] != '/' ) { NEWENTRY = substr( entry, 1, lastpos( "/", entry ) ) + NEWENTRY ; }
					if ( exists( NEWENTRY ) )
					{
						if ( is_directory( entry ) || is_symlink( entry ) )
						{
							MSG     = "FLST01Y" ;
							MESSAGE = "Invalid" ;
							tbput( DSLIST ) ;
							continue        ;
						}
						if ( FREPL != "/" )
						{
							MSG     = "FLST01X"     ;
							MESSAGE = "File Exists" ;
							tbput( DSLIST ) ;
							continue        ;
						}
						if ( !is_regular_file( NEWENTRY ) )
						{
							MSG     = "FLST01Y"      ;
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
							MSG     = "FLST0116" ;
							MESSAGE = "Errors"   ;
						}
						else
						{
							MSG     = "FLST01U" ;
							MESSAGE = "Copied"  ;
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
						MSG     = "FLST01U" ;
						MESSAGE = "Copied"  ;
					}
					else
					{
						MSG     = "FLST01V"    ;
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
						MSG     = "FLST01P"   ;
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
							MSG     = "FLST01N" ;
							RSN     = d2ds( num ) + " files deleted" ;
							MESSAGE = "Deleted" ;
						}
						else
						{
							MSG     = "FLST01O"    ;
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
							MSG     = "FLST01N"         ;
							RSN     = "1 entry deleted" ;
							MESSAGE = "Deleted"         ;
						}
						else
						{
							MSG     = "FLST01O"    ;
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
					MSG     = "FLST01S"   ;
					MESSAGE = "Cancelled" ;
				}
				else
				{
					if ( NEWENTRY[ 0 ] != '/' ) { NEWENTRY = substr( entry, 1, lastpos( "/", entry ) ) + NEWENTRY ; }
					if ( rename( entry.c_str(), NEWENTRY.c_str() ) == 0 )
					{
						MSG     = "FLST01Q" ;
						MESSAGE = "Renamed" ;
					}
					else
					{
						if ( errno == EXDEV )
						{
							MSG     = "FLST01Z"  ;
							MESSAGE = "Use COPY" ;
						}
						else
						{
							MSG     = "FLST01R"         ;
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
				select( "PGM(" + PGM + ") PARM(porexx2.rex " + entry + ")" ) ;
				SEL = "" ;
				if ( ZRESULT != "" ) { MESSAGE = ZRESULT    ; MSG = "FLST01M" ; }
				else                 { MESSAGE = "Executed" ;                   }
				if ( ZRC == 0 ) { browse( "/tmp/porexx2.say" ) ; }
				tbput( DSLIST ) ;
			}
			else if ( is_regular_file( entry ) && (SEL == "S" || SEL == "B" || SEL == "E" || SEL == "L" ) )
			{
				if ( SEL == "E" )
				{
					edit( entry ) ;
					SEL = "" ;
					if ( ZRESULT != "" ) { MESSAGE = ZRESULT  ; }
					else                 { MESSAGE = "Edited" ; }
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
			}
			else if ( is_regular_file( entry ) && (SEL == "NANO" ) )
			{
				t = "nano " + entry ;
				noTimeOut = true    ;
				def_prog_mode()     ;
				endwin()            ;
				system( t.c_str() ) ;
				reset_prog_mode()   ;
				refresh()           ;
				noTimeOut = false   ;
				SEL     = ""        ;
				MESSAGE = "nano"    ;
				tbput( DSLIST )     ;
			}
			else if ( is_regular_file( entry ) && (SEL == "VI" ) )
			{
				t = "vi " + entry   ;
				noTimeOut = true    ;
				def_prog_mode()     ;
				endwin()            ;
				system( t.c_str() ) ;
				reset_prog_mode()   ;
				refresh()           ;
				noTimeOut = false   ;
				SEL     = ""        ;
				MESSAGE = "vi"      ;
				tbput( DSLIST )     ;
			}
			else if ( SEL == "T" )
			{
				SEL = "" ;
				boost::filesystem::path temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path( ZUSER + "-" + ZSCREEN + "-%%%%-%%%%" ) ;
				string tname = temp.native() ;
				of.open( tname ) ;
				recursive_directory_iterator eIt ; 
				recursive_directory_iterator dIt( entry, ec ) ;
				if ( ec.value() != boost::system::errc::success )
				{
					MSG     = "FLST0117"   ;
					RSN     = ec.message() ;
					MESSAGE = ec.message() ;
					tbput( DSLIST )        ;
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
						}
					} 
				}
				catch ( const filesystem_error& ex )
				{
					MSG     = "FLST0117" ;
					RSN     = ex.what()  ;
					MESSAGE = ex.what()  ;
					tbput( DSLIST )      ;
				}
				of.close() ;
				browse( tname ) ;
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


void PFLST0A::createFileList( string filter )
{
	int i    ;
	string p ;
	struct stat results   ;
	struct tm * time_info ;
	char buf[ 20 ]        ;

	typedef vector< path > vec ;

	tbcreate( DSLIST, "", "MESSAGE SEL ENTRY TYPE PERMISS SIZE STCDATE MODDATE MODDATES", NOWRITE ) ;

	MESSAGE = ""    ;
	SEL     = ""    ;
	ENTRY   = "."   ;
	TYPE    = "Dir" ;
	tbadd( DSLIST ) ;
	ENTRY   = ".."  ;
	tbadd( DSLIST ) ;

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
		if ( filter != "" && !abbrev( ENTRY, filter ) ) { continue ; }
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
		if ( S_ISDIR(results.st_mode) )  { PERMISS.replace(0, 1, 1, 'd' ) ; }
		if ( S_ISLNK(results.st_mode) )  { PERMISS.replace(0, 1, 1, 'l' ) ; }
		if ( results.st_mode & S_IRUSR ) { PERMISS.replace(1, 1, 1, 'r' ) ; }
		if ( results.st_mode & S_IWUSR ) { PERMISS.replace(2, 1, 1, 'w' ) ; }
		if ( results.st_mode & S_IXUSR ) { PERMISS.replace(3, 1, 1, 'x' ) ; }
		if ( results.st_mode & S_IRGRP ) { PERMISS.replace(4, 1, 1, 'r' ) ; }
		if ( results.st_mode & S_IWGRP ) { PERMISS.replace(5, 1, 1, 'w' ) ; }
		if ( results.st_mode & S_IXGRP ) { PERMISS.replace(6, 1, 1, 'x' ) ; }
		if ( results.st_mode & S_IROTH ) { PERMISS.replace(7, 1, 1, 'r' ) ; }
		if ( results.st_mode & S_IWOTH ) { PERMISS.replace(8, 1, 1, 'w' ) ; }
		if ( results.st_mode & S_IXOTH ) { PERMISS.replace(9, 1, 1, 'x' ) ; }
		SIZE      = d2ds( results.st_size )   ;
		MODDATES  = d2ds( results.st_mtime )  ;
		time_info = gmtime( &(results.st_mtime) ) ;
		strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info )  ; buf[ 19 ]  = 0x00 ; MODDATE = buf ;
		SEL       = ""  ;
		tbadd( DSLIST ) ;
	}
	tbtop( DSLIST ) ;
}



void PFLST0A::showInfo( string p )
{
	int s ;

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
	if ( results.st_mode & S_IRUSR ) { IPERMISS.replace(0, 1, 1, 'r' ) ; }
	if ( results.st_mode & S_IWUSR ) { IPERMISS.replace(1, 1, 1, 'w' ) ; }
	if ( results.st_mode & S_IXUSR ) { IPERMISS.replace(2, 1, 1, 'x' ) ; }
	if ( results.st_mode & S_IRGRP ) { IPERMISS.replace(3, 1, 1, 'r' ) ; }
	if ( results.st_mode & S_IWGRP ) { IPERMISS.replace(4, 1, 1, 'w' ) ; }
	if ( results.st_mode & S_IXGRP ) { IPERMISS.replace(5, 1, 1, 'x' ) ; }
	if ( results.st_mode & S_IROTH ) { IPERMISS.replace(6, 1, 1, 'r' ) ; }
	if ( results.st_mode & S_IWOTH ) { IPERMISS.replace(7, 1, 1, 'w' ) ; }
	if ( results.st_mode & S_IXOTH ) { IPERMISS.replace(8, 1, 1, 'x' ) ; }

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
				else			    { break ;		 }
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

	ofstream of ;
	boost::system::error_code ec ;

	cw    = upper( word( ZCMD, 1 ) ) ;
	ws    = subword( ZCMD, 2 ) ;
	w2    = word( ZCMD, 2 )    ;
	w3    = word( ZCMD, 3 )    ;

	if ( cw == "" ) return  0 ;
	else if ( cw == "SORT" )
	{
		w2 = upper( w2 ) ;
		w3 = upper( w3 ) ;
		if ( w2 == "" ) { w2 = "ENTRY" ; }
		if ( w3 == "" ) { w3 = "A"     ; }
		if      ( w2 == "ENTRY"   ) { tbsort( DSLIST, "ENTRY,C,"+w3   )  ; }
		else if ( w2 == "TYPE"    ) { tbsort( DSLIST, "TYPE,C,"+w3    )  ; }
		else if ( w2 == "PERMISS" ) { tbsort( DSLIST, "PERMISS,C,"+w3 )  ; }
		else if ( w2 == "SIZE"    ) { tbsort( DSLIST, "SIZE,N,"+w3    )  ; }
		else if ( w2 == "MOD"     ) { tbsort( DSLIST, "MODDATES,N,"+w3 ) ; }
		else                        { return 8 ;                           }
		if ( RC > 0 ) { MSG = "FLST01H" ; return 4 ; }
		return 0 ;
	}
	else if ( (cw == "S" || cw == "B" || cw == "E" ) && ws != "" )
	{
		p = ZPATH + "/" + ws ;
		if ( is_directory( p ) && (cw == "S" || cw == "B" || cw == "CD" ) )
		{
			vcopy( "ZFLSTPGM", PGM, MOVE ) ;
			select( "PGM(" + PGM + ") PARM(" + p + ")" ) ;
			ZCMD = "" ;
		}
		else if ( is_regular_file( p ) && (cw == "S" || cw == "B" || cw == "E" || cw == "L" ) )
		{
			if ( cw == "E" ) { edit( p )   ; }
			else 		 { browse( p ) ; }
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
		if ( ZPATH.back() == '/' ) { ZPATH = ZPATH.substr( 0, ZPATH.size()-1) ; }
		if ( substr( ws, 1, 1 ) != "/" ) { ZPATH = ZPATH + "/" + ws ; }
		else                             { ZPATH = ws               ; }
		return 0 ;
	}
	else if ( cw == "L" )
	{
		tbvclear( DSLIST ) ;
		ENTRY = word( ws, 1 ) ;
		if ( ENTRY.back() != '*' ) { ENTRY.push_back( '*' ) ; }
		if ( w3 == "PREV" )
		{
			tbsarg( DSLIST, "", "PREVIOUS", "ENTRY,LE" ) ;
			tbscan( DSLIST, "", "", "", "", "", "CRP"  ) ;
		}
		else
		{
			if ( w3 == "FIRST" )
			{
				tbsarg( DSLIST, "", "NEXT", "ENTRY,GE" ) ;
				tbtop( DSLIST )  ;
				tbscan( DSLIST, "", "", "", "", "", "CRP" ) ;
			}
			else
			{
				if ( w3 == "LAST" )
				{
					tbsarg( DSLIST, "", "PREVIOUS", "ENTRY,LE" ) ;
					tbbottom( DSLIST ) ;
					tbscan( DSLIST, "", "", "", "", "", "CRP"  ) ;
				}
				else
				{
					tbsarg( DSLIST, "", "NEXT", "ENTRY,GE" ) ;
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
			MSG = "FLST0111" ;
		}
		else
		{
			MSG = "FLST0112"   ;
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
				MSG = "FLST0114" ;
			}
			else { MSG = "FLST0115" ; }
		}
		else
		{
			MSG = "FLST0113" ;
		}
		ZCMD = "" ;
		return 4  ;
	}
	return 8 ;
}


void PFLST0A::copyDirs( string src, string dest, string DIRREC, bool & errs )
{
	boost::system::error_code ec ;

	copy_directory( src, dest, ec ) ;
	if ( ec.value() != boost::system::errc::success )
	{
		errs    = true         ;
		MSG     = "FLST01V"    ;
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


void PFLST0A::modifyAttrs( string p )
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
	struct tm * time_info ;
	char buf [ 20 ] ;
	char * buffer   ;
	size_t bufferSize = 255 ;
	size_t rc               ;
	
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

	pwd = getpwuid( results.st_uid ) ;
	grp = getgrgid( results.st_gid ) ;

	IOWNER  = pwd->pw_name ;
	IOWNERN = d2ds( pwd->pw_uid ) ;
	IGROUP  = grp->gr_name ;
	IGROUPN = d2ds( grp->gr_gid ) ;

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
		MSG     = "FLST01I"   ;
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
			if ( chmod( p.c_str(), t ) != 0 ) { MSG = "FLST01J" ; }
		}
		if ( MSG == "" && (IOWNERN != OOWNERN) )
		{
			changed = true ;
			pwd = getpwuid( ds2d( IOWNERN ) ) ;
			if ( pwd == NULL )
			{
				MSG = "FLST01K" ;
			}
			else
			{
				changed = true    ;
				uid = pwd->pw_uid ;
				if ( chown( p.c_str(), uid, -1 ) == -1 ) { MSG = "FLST0118" ; }
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
				if ( chown( p.c_str(), uid, -1 ) == -1 ) { MSG = "FLST0118" ; }
			}
		}
		if ( MSG == "" && (IGROUPN != OGROUPN) )
		{
			changed = true ;
			grp = getgrgid( ds2d( IGROUPN ) ) ;
			if ( grp == NULL )
			{
				MSG = "FLST01L" ;
			}
			else
			{
				gid = grp->gr_gid ;
				if ( chown( p.c_str(), -1, gid ) == -1 ) { MSG = "FLST0119" ; }
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
				if ( chown( p.c_str(), -1, gid ) == -1 ) { MSG = "FLST0119" ; }
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

// ============================================================================================ //

extern "C" { pApplication *maker() { return new PFLST0A ; } }
extern "C" { void destroy(pApplication *p) { delete p ; } }
