/* Compile with ::                                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPFLST0A.so -lboost_regex -o libPFLST0A.so PFLST0A.cpp  */

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
/* If invoked with a PARM of LIST, use passed list to create a file list instead of listing a directory     */
/* If invoked with a PARM of EXPAND, will expand the passed directory name to the next level                */
/*                SUBPARM of ALL - return all types (fully qualified)                                       */
/*                SUBPARM of DO1 - return only directories                                                  */
/*                SUBPARM of FO1 - return only fully qualified files                                        */
/*                SUBPARM of FO2 - return only the file names (without the directory name)                  */
/* If invoked with a PARM of EXPAND1, will expand field according to subtype                                */
/*                SUBPARM of PNL  - panel files in ZPLIB                                                    */
/*                SUBPARM of REXX - REXX programs in ZORXPATH                                               */
/*                SUBPARM of PGM  - C++ programs in ZDLPATH                                                 */
/*                SUBPARM of CMD  - commands in ZORXPATH and PATH                                           */

/* Primary Commands:                                                                                        */
/* CD /xxx to change to directory /xxx                                                                      */
/* CD xxx to change to directory xxx under the current directory listing                                    */
/* BACK or S to go back one directory level                                                                 */
/* S xxx to select file or directory xxx from the current list                                              */
/* L xxx FIRST|LAST|PREV to scroll the list to entry xxx (NEXT is the default)                              */
/* O  - filter list                                                                                         */
/* REF - refresh directory list                                                                             */
/* MKDIR  - Make a directory (under current or specify full path)                                           */
/* SEARCH - Show a list containing only files that match a search word (within the file).  Uses grep        */
/* SRCHFOR- Same as SEARCH                                                                                  */
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
#include <boost/regex.hpp>
#include <vector>

#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

#include "../lspf.h"
#include "../utilities.h"
#include "../classes.h"
#include "../pVPOOL.h"
#include "../pWidgets.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"
#include "PFLST0A.h"

using namespace std ;
using namespace boost ;
using namespace boost::filesystem ;

#undef  MOD_NAME
#define MOD_NAME PFLST0A


PFLST0A::PFLST0A()
{
	vdefine( "ZCURFLD", &zcurfld ) ;
	vdefine( "ZCURINX  ZTDTOP ZTDVROWS ZTDSELS ZTDDEPTH", &zcurinx, &ztdtop, &ztdvrows, &ztdsels, &ztddepth ) ;
	vdefine( "ZEDLMACT ZEDEPROF ZEDIMACA", &lcmtab, &eprof, &eimac ) ;
}


void PFLST0A::application()
{
	llog( "I", "Application PFLST0A starting with PARM of " << PARM << endl ) ;

	int i      ;
	int rc     ;
	int RCode  ;
	int num    ;
	int csrrow ;
	int crpx   ;

	size_t p1  ;

	bool del   ;
	bool errs  ;

	string entry    ;
	string opath    ;
	string ohidden  ;
	string oexgen   ;
	string lp       ;
	string pgm      ;
	string w1       ;
	string w2       ;
	string w3       ;
	string t        ;
	string filter   ;
	string panel    ;
	string csr      ;
	string msgloc   ;

	string condoff  ;
	string nemptok  ;
	string dirrec   ;
	string newentry ;
	string frepl    ;

	char* buffer    ;
	size_t bufferSize = 255 ;

	boost::system::error_code ec ;

	vdefine( "SEL ENTRY MESSAGE TYPE PERMISS SIZE STCDATE MODDATE", &sel, &ENTRY, &message, &TYPE, &permiss, &size, &stcdate, &moddate ) ;
	vdefine( "MODDATES ZVERB ZHOME ZCMD ZPATH CONDOFF NEWENTRY FREPL", &moddates, &zverb, &zhome, &zcmd, &zpath, &condoff, &newentry, &frepl ) ;
	vdefine( "RSN NEMPTOK DIRREC AFHIDDEN", &rsn, &nemptok, &dirrec, &afhidden ) ;
	vdefine( "EXGEN OEXGEN", &exgen, &oexgen ) ;
	vdefine( "CRP", &crp ) ;

	vcopy( "ZUSER", zuser, MOVE ) ;
	vcopy( "ZSCREEN", zscreen, MOVE ) ;

	std::ofstream of ;

	vget( "ZHOME ZEDLMACT ZEDEPROF ZEDIMACA", SHARED ) ;
	vget( "AFHIDDEN EXGEN", PROFILE ) ;

	UseList = false ;

	if ( PARM == "" )
	{
		vget( "ZPATH", PROFILE ) ;
	}
	else
	{
		w1 = word( PARM, 1 ) ;
		if ( w1 == "BROWSE" || w1 == "EDIT" )
		{
			zpath = subword( PARM, 2 ) ;
			try
			{
				if ( is_regular_file( zpath ) )
				{
					if ( w1 == "BROWSE" )
					{
						browse( zpath ) ;
						cleanup() ;
						return ;
					}
					else if ( w1 == "VIEW" )
					{
						view( zpath ) ;
						cleanup() ;
						return ;
					}
					else
					{
						edit( zpath, "", eimac, eprof, lcmtab ) ;
						cleanup() ;
						return ;
					}
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
		else if ( w1 == "LIST" )
		{
			UseList = true ;
			vreplace( "LSTNAME", word( PARM, 2 ) ) ;
			PssList = word( PARM, 3 ) ;
		}
		else
		{
			zpath = PARM ;
		}
	}

	filter = "" ;
	p1 = zpath.find_last_of( '/' ) ;
	if ( p1 != string::npos )
	{
		if ( zpath.find_first_of( "?*[", p1 ) != string::npos )
		{
			filter = zpath.substr( p1+1 ) ;
			zpath.erase( p1 ) ;
		}
	}
	if ( !is_directory( zpath ) ) { zpath = zhome ; }

	dslist = "DSLST" + d2ds( taskid(), 3 ) ;

	RC        = 0 ;
	i         = 1 ;
	ztdvrows  = 1 ;
	ztdsels   = 0 ;
	ztdtop    = 0 ;
	csrrow    = 0 ;
	crpx      = 0 ;
	csr       = "ZCMD" ;
	msgloc    = ""    ;
	msg       = ""    ;
	UseSearch = false ;

	createFileList1( filter ) ;
	if ( RC > 0 ) { setmsg( "FLST015" ) ; }

	while ( true )
	{
		if ( msg == "" && ztdsels > 0 )
		{
			ztdsels-- ;
		}
		if ( ztdsels == 0 )
		{
			if ( ztdvrows > 0 )
			{
				tbtop( dslist )     ;
				tbskip( dslist, i ) ;
			}
			else
			{
				tbbottom( dslist ) ;
				tbskip( dslist, - (ztddepth-2) ) ;
				if ( RC > 0 ) { tbtop( dslist ) ; }
			}
			panel = UseList ? "PFLST0A9" : "PFLST0A1" ;
			condoff = "" ;
			nemptok = "" ;
			dirrec  = "" ;
		}
		else
		{
			panel = "" ;
		}
		opath   = zpath    ;
		ohidden = afhidden ;
		oexgen  = exgen    ;
		if ( msg != "" && csr == "" )
		{
			csr    = "SEL" ;
			msgloc = "SEL" ;
			csrrow = crpx  ;
		}
		else
		{
			csr    = "ZCMD" ;
		}

		tbdispl( dslist, panel, msg, csr, csrrow, 1, "YES", "CRP", "", msgloc ) ;
		if ( RC == 8 ) { break ; }

		msg    = "" ;
		csr    = "ZCMD" ;
		msgloc = "" ;
		csrrow = 0  ;
		w1  = upper( word( zcmd, 1 ) ) ;
		w2  = word( zcmd, 2 ) ;
		w3  = word( zcmd, 3 ) ;
		if ( (w1 == "REFRESH" || w1 == "RESET" ) && w2 == "" )
		{
			if ( w1 == "RESET" ) { filter = "" ; UseSearch = false ; }
			if ( filter == "" )  { vreplace( "FMSG1", "" ) ; }
			if ( !UseSearch   )  { vreplace( "FMSG2", "" ) ; }
			tbend( dslist ) ;
			createFileList1( filter ) ;
			zcmd = "" ;
			continue  ;
		}
		if ( w1 == "O" && w2 != "" && w3 == "" )
		{
			filter = w2 ;
			vreplace( "FMSG1", "Filtered on file name" ) ;
			tbend( dslist ) ;
			createFileList1( filter ) ;
			zcmd = "" ;
			continue  ;
		}
		if ( ( w1 == "SEARCH" || w1 == "SRCHFOR" ) && w2 != "" && w3 == "" )
		{
			UseSearch = true ;
			vreplace( "FMSG2", "Filtered on contents" ) ;
			createSearchList( w2 ) ;
			tbend( dslist ) ;
			createFileList1( filter ) ;
			zcmd = "" ;
			continue  ;
		}
		i = ztdtop ;
		vget( "ZVERB", SHARED ) ;
		crpx  = crp ;
		crp   = 0 ;
		RCode = processPrimCMD() ;
		zcmd  = "" ;
		if ( RCode == 8 )  { msg = "PSYS018" ; continue ; }
		if ( crp > 0 )     { i = crp       ; }
		if ( RCode == 4 )  { continue      ; }
		if ( zpath == "" ) { zpath = zhome ; }
		if ( ohidden != afhidden || oexgen != exgen )
		{
			filter = ""       ;
			tbend( dslist )   ;
			createFileList1() ;
			i = 1    ;
			continue ;
		}
		if ( opath != zpath )
		{
			if ( ( !exists( zpath ) || !is_directory( zpath ) ) )
			{
				msg = "PSYS012A" ;
			}
			else
			{
				filter = ""       ;
				tbend( dslist )   ;
				createFileList1() ;
				i = 1 ;
			}
			continue ;
		}
		csr = "" ;
		if ( ztdsels == 0 && zcurinx != 0 )
		{
			tbtop( dslist ) ;
			tbskip( dslist, zcurinx ) ;
			entry = UseList ? ENTRY : createEntry( zpath, ENTRY ) ;
			if ( zcurfld == "ENTRY" )
			{
				if ( is_directory( entry ) )
				{
					sel = "L" ;
				}
				else if ( is_regular_file( entry ) )
				{
					sel = "E" ;
				}
				else
				{
					continue ;
				}
			}
			else if ( zcurfld == "TYPE" )
			{
				sel = "I" ;
			}
			else
			{
				continue ;
			}
		}
		entry = UseList ? ENTRY : createEntry( zpath, ENTRY ) ;
		if ( sel != "" && !exists( entry ) )
		{
			sel     = ""          ;
			msg     = "FLST012L"  ;
			message = "Not Found" ;
			tbput( dslist )       ;
			continue ;
		}
		auto it = line_cmds.find( sel ) ;
		sel = "" ;
		if ( it == line_cmds.end() ) { continue ; }

		switch ( it->second )
		{
		case LN_EDIT:
		case LN_VIEW:
		case LN_BROWSE:
		case LN_FORMAT:
		case LN_NANO:
		case LN_VI:
			if ( !is_regular_file( entry ) )
			{
				msg     = "FLST012M" ;
				message = "Invalid Entry" ;
				tbput( dslist ) ;
				continue ;
			}
			vcopy( "ZRFURL", t, MOVE ) ;
			if ( t == "YES" )
			{
				vcopy( "ZRFLPGM", pgm, MOVE ) ;
				select( "PGM("+pgm+") PARM(PLA "+entry+") BACK" ) ;
			}
			break ;
		}

		switch ( it->second )
		{
		case LN_INFO:
			showInfo( entry )       ;
			message = "Information" ;
			tbput( dslist )         ;
			break ;

		case LN_EXECUTE:
			control( "ERRORS", "RETURN" ) ;
			select( "CMD(%" + entry + ") LANG(REXX)" ) ;
			control( "ERRORS", "CANCEL" ) ;
			if ( ZRESULT != "" )
			{
				message = ZRESULT    ;
				msg     = "PSYS011M" ;
				vreplace( "STR", "RC="+ d2ds( ZRC ) +"  RSN="+ d2ds( ZRSN ) ) ;
			}
			else
			{
				message = "Executed" ;
			}
			tbput( dslist ) ;
			break ;

		case LN_LINK:
			lp = "" ;
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
						message = "Not a link" ;
						vreplace( "ZEDSMSG", message ) ;
						vreplace( "ZEDLMSG", ""      ) ;
						msg     = "PSYZ002" ;
						lp      = "" ;
						break ;
					}
				}
				else
				{
					lp = string( buffer, rc ) ;
					delete[] buffer ;
					if ( substr( lp, 1, 1 ) != "/" ) { lp = zpath + lp ; }
					if ( !is_directory( lp ) )
					{
						message = "Not a directory" ;
						vreplace( "ZEDSMSG", message ) ;
						vreplace( "ZEDLMSG", ""      ) ;
						msg     = "PSYZ002" ;
						lp      = "" ;
					}
					break ;
				}
			}
			if ( lp == "" ) { tbput( dslist ) ; continue ; }
			vcopy( "ZFLSTPGM", pgm, MOVE ) ;
			select( "PGM(" + pgm + ") PARM(" + lp + ")" ) ;
			if ( ZRESULT != "" )
			{
				message = ZRESULT ;
				vreplace( "ZEDSMSG", message ) ;
				vreplace( "ZEDLMSG", ""      ) ;
				msg     = "PSYZ002" ;
			}
			else
			{
				message = "Linked" ;
			}
			tbput( dslist ) ;
			break ;

		case LN_COPY:
			zcmd     = "" ;
			newentry = "" ;
			frepl    = "" ;
			if ( !is_directory( entry ) && !is_symlink( entry ) && !is_regular_file( entry ) )
			{
				msg     = "FLST012A"     ;
				message = "Invalid Type" ;
				tbput( dslist ) ;
				continue        ;
			}
			display( "PFLST0A5" ) ;
			if ( RC == 8 )
			{
				setmsg( "FLST011W" )  ;
				message = "Cancelled" ;
				tbput( dslist ) ;
				break ;
			}
			if ( newentry[ 0 ] != '/' ) { newentry = substr( entry, 1, lastpos( "/", entry ) ) + newentry ; }
			if ( exists( newentry ) )
			{
				if ( is_directory( entry ) || is_symlink( entry ) )
				{
					msg     = "FLST011Y" ;
					message = "Invalid"  ;
					tbput( dslist ) ;
					continue        ;
				}
				if ( frepl != "/" )
				{
					msg     = "FLST011X"    ;
					message = "File Exists" ;
					tbput( dslist ) ;
					continue        ;
				}
				if ( !is_regular_file( newentry ) )
				{
					msg     = "FLST011Y"     ;
					message = "Invalid Type" ;
					tbput( dslist ) ;
					continue        ;
				}
			}
			if ( is_directory( entry ) )
			{
				errs = false ;
				copyDirs( entry, newentry, dirrec, errs ) ;
				if ( errs )
				{
					msg     = "FLST012G" ;
					message = "Errors"   ;
				}
				else
				{
					setmsg( "FLST011U" ) ;
					message = "Copied"   ;
				}
				tbput( dslist ) ;
				continue ;
			}
			else if ( is_regular_file( entry ) )
			{
				copy_file( entry, newentry, copy_option::overwrite_if_exists, ec ) ;
			}
			else if ( is_symlink( entry ) )
			{
				copy_symlink( entry, newentry, ec ) ;
			}
			if ( ec.value() == boost::system::errc::success )
			{
				setmsg( "FLST011U" ) ;
				message = "Copied"   ;
			}
			else
			{
				msg     = "FLST011V"   ;
				rsn     = ec.message() ;
				message = ec.message() ;
				llog( "E", "Copy of " + entry + " to " + newentry + " failed with " + ec.message() << endl ) ;
			}
			tbput( dslist ) ;
			break ;

		case LN_DELETE:
			del = ( condoff == "/" ) ;
			if ( !del )
			{
				addpop( "", 5, 5 ) ;
				display( "PFLST0A3" ) ;
				if ( RC == 8 )
				{
					setmsg( "FLST011P" )  ;
					message = "Cancelled" ;
				}
				else
				{
					del = true ;
				}
				rempop() ;
			}
			if ( del )
			{
				if ( nemptok == "/" )
				{
					num = remove_all( entry.c_str( ), ec ) ;
					if ( ec.value() == boost::system::errc::success )
					{
						rsn     = d2ds( num ) + " files deleted" ;
						setmsg( "FLST011N" ) ;
						message = "Deleted" ;
					}
					else
					{
						msg     = "FLST011O"   ;
						rsn     = ec.message() ;
						message = ec.message() ;
						llog( "E", "Delete of " + entry + " failed with " + ec.message() << " " << num <<" messages deleted "<< endl ) ;
					}
				}
				else
				{
					remove( entry.c_str( ), ec ) ;
					if ( ec.value() == boost::system::errc::success )
					{
						rsn     = "1 entry deleted" ;
						setmsg( "FLST011N" ) ;
						message = "Deleted"         ;
					}
					else
					{
						msg     = "FLST011O"   ;
						rsn     = ec.message() ;
						message = ec.message() ;
						llog( "E", "Delete of " + entry + " failed with " + ec.message() << endl ) ;
					}
				}
			}
			tbput( dslist ) ;
			break ;

		case LN_MODIFY:
			modifyAttrs( entry ) ;
			tbput( dslist ) ;
			break ;

		case LN_RENAME:
			newentry = "" ;
			display( "PFLST0A4", msg, "ZCMD" ) ;
			if ( RC == 8 )
			{
				zcmd    = ""          ;
				setmsg( "FLST011S" )  ;
				message = "Cancelled" ;
			}
			else
			{
				if ( newentry[ 0 ] != '/' ) { newentry = substr( entry, 1, lastpos( "/", entry ) ) + newentry ; }
				if ( rename( entry.c_str(), newentry.c_str() ) == 0 )
				{
					setmsg( "FLST011Q" ) ;
					message = "Renamed"  ;
				}
				else
				{
					if ( errno == EXDEV )
					{
						msg     = "FLST011Z" ;
						message = "Use COPY" ;
					}
					else
					{
						msg     = "FLST011R"        ;
						rsn     = strerror( errno ) ;
						message = strerror( errno ) ;
					}
					llog( "E", "Rename of " + entry + " to " + newentry + " failed with " + strerror( errno ) << endl ) ;
				}
			}
			tbput( dslist ) ;
			break ;

		case LN_EDIT:
			edit( entry, "", eimac, eprof, lcmtab ) ;
			if ( ZRESULT != "" )
			{
				message = ZRESULT ;
			}
			else
			{
				message = "Edited" ;
			}
			tbput( dslist ) ;
			break ;

		case LN_VIEW:
			view( entry ) ;
			if ( ZRESULT != "" )
			{
				message = ZRESULT ;
			}
			else
			{
				message = "Viewed" ;
			}
			tbput( dslist ) ;
			break ;

		case LN_BROWSE:
			browse( entry ) ;
			if ( ZRESULT != "" )
			{
				message = ZRESULT ;
			}
			else
			{
				message = "Browsed" ;
			}
			tbput( dslist ) ;
			break ;

		case LN_LIST:
			vcopy( "ZFLSTPGM", pgm, MOVE ) ;
			select( "PGM(" + pgm + ") PARM(" + entry + ")" ) ;
			if ( ZRESULT != "" )
			{
				message = ZRESULT ;
			}
			else
			{
				message = "Listed" ;
			}
			tbput( dslist ) ;
			break ;

		case LN_FORMAT:
			select( "CMD(%porexx2 " + entry + ") LANG(REXX)" ) ;
			if ( ZRESULT != "" )
			{
				message = ZRESULT ;
			}
			else
			{
				message = "Executed" ;
			}
			if ( ZRC == 0 ) { browse( "/tmp/porexx2.say" ) ; }
			tbput( dslist ) ;
			break ;

		case LN_NANO:
			t = "nano " + entry ;
			control( "TIMEOUT", "DISABLE" ) ;
			def_prog_mode()     ;
			endwin()            ;
			std::system( t.c_str() ) ;
			reset_prog_mode()   ;
			refresh()           ;
			control( "TIMEOUT", "ENABLE" ) ;
			message = "nano"    ;
			tbput( dslist )     ;
			break ;

		case LN_VI:
			t = "vi " + entry   ;
			control( "TIMEOUT", "DISABLE" ) ;
			def_prog_mode()     ;
			endwin()            ;
			std::system( t.c_str() ) ;
			reset_prog_mode()   ;
			refresh()           ;
			control( "TIMEOUT", "ENABLE" ) ;
			message = "vi"      ;
			tbput( dslist )     ;
			break ;

		case LN_TREE:
		case LN_TTREE:
			RC = 0 ;
			boost::filesystem::path temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;
			string tname = temp.native() ;
			of.open( tname ) ;
			recursive_directory_iterator eIt ;
			recursive_directory_iterator dIt( entry, ec ) ;
			if ( ec.value() != boost::system::errc::success )
			{
				msg     = "FLST012H"   ;
				rsn     = ec.message() ;
				message = ec.message() ;
				tbput( dslist )        ;
				of.close()             ;
				remove( tname )        ;
				continue               ;
			}
			of << substr( entry, lastpos( "/", entry )+1 ) << endl ;
			try
			{
				for ( ; dIt != eIt ; ++dIt )
				{
					path current( (*dIt) ) ;
					if ( is_regular_file( current ) || is_directory( current ) )
					{
						of << copies( "|   ", dIt.level()) << "|-- " ;
						of << strip( current.filename().string(), 'B', '"' ) << endl ;
						if ( it->second == LN_TREE )
						{
							of << strip( current.string(), 'B', '"' ) << endl ;
						}
					}
				}
			}
			catch ( const filesystem_error& ex )
			{
				msg     = "FLST012H" ;
				rsn     = ex.what()  ;
				message = ex.what()  ;
				tbput( dslist )      ;
				of.close()           ;
				remove( tname )      ;
				continue             ;
			}
			of.close() ;
			( it->second == LN_TREE ) ? browseTree( tname ) : browse( tname ) ;
			if ( RC > 0 )
			{
				msg     = "FLST012H"      ;
				message = "Unknown Error" ;
				tbput( dslist )           ;
			}
			remove( tname ) ;
			break ;
		}
	}
	tbend( dslist ) ;
	if ( UseList ) { remove( PssList ) ; }

	cleanup() ;
	return    ;
}


void PFLST0A::createFileList1( string filter )
{
	int i ;

	size_t p1 ;

	string p ;
	string t ;
	string pat  ;
	string str  ;
	string rest ;

	char c    ;

	bool fGen ;

	struct stat results  ;
	struct tm* time_info ;
	char buf[ 20 ]       ;

	regex expression ;

	tbcreate( dslist, "", "(MESSAGE,SEL,ENTRY,TYPE,PERMISS,SIZE,STCDATE,MODDATE,MODDATES)", NOWRITE ) ;

	message = ""  ;
	sel     = ""  ;

	iupper( filter ) ;

	fGen = ( filter.find_first_of( "?*[" ) != string::npos ) ;

	vector<path> v;
	vector<path>::const_iterator it ;

	if ( UseList )
	{
		std::ifstream fin( PssList.c_str() ) ;
		while ( getline( fin, t ) )
		{
			if ( exgen == "/" )
			{
				p1 = t.find_last_of( '/' ) ;
				if ( p1 != string::npos )
				{
					if ( t.find_first_of( "?*[", p1 ) != string::npos )
					{
						str = t.substr( p1+1) ;
						AddPath( t.erase(p1), str, v ) ;
					}
					else
					{
						v.push_back( t ) ;
					}
				}
				else
				{
					v.push_back( t ) ;
				}
			}
			else
			{
				v.push_back( t ) ;
			}
		}
		fin.close() ;
	}
	else
	{
		try
		{
			copy( directory_iterator( zpath ), directory_iterator(), back_inserter( v ) ) ;
		}
		catch ( const filesystem_error& ex )
		{
			ZRESULT = "List Error" ;
			RC      = 16 ;
			llog( "E", "Error listing directory " << ex.what() << endl ) ;
			return ;
		}
		sort( v.begin(), v.end() ) ;
	}

	if ( fGen )
	{
		pat = "" ;
		for ( uint i = 0 ; i < filter.size() ; i++ )
		{
			c = toupper( filter[ i ] ) ;
			if      ( c == '*' ) { pat += "[^[:blank:]]*" ; }
			else if ( c == '?' ) { pat += "[^[:blank:]]"  ; }
			else                 { pat.push_back( c )     ; }
		}
		try
		{
			expression.assign( pat ) ;
		}
		catch  ( boost::regex_error& e )
		{
			setmsg( "PSYS012P" ) ;
			return ;
		}
	}

	for ( it = v.begin() ; it != v.end() ; ++it )
	{
		tbvclear( dslist ) ;
		ENTRY = (*it).string() ;
		p     = ENTRY          ;
		i     = lastpos( "/", ENTRY ) + 1 ;
		rest  = ENTRY.substr( 0, i-1 ) ;
		ENTRY = substr( ENTRY, i )   ;
		if ( !UseList && afhidden != "/" && ENTRY[ 0 ] == '.' ) { continue ; }
		if ( fGen )
		{
			str = upper( ENTRY ) ;
			if ( !regex_match( str.begin(), str.end(), expression ) ) { continue ; }
		}
		else
		{
			if ( UseList )
			{
				if ( filter != "" && pos( filter, upper( p ) ) == 0 ) { continue ; }
			}
			else
			{
				if ( filter != "" && pos( filter, upper( ENTRY ) ) == 0 ) { continue ; }
			}
		}
		if ( UseSearch )
		{
			if ( find( SearchList.begin(), SearchList.end(), (*it).string() ) == SearchList.end() )
			{
				continue ;
			}
		}
		if ( lstat( p.c_str(), &results ) != 0 )
		{
			TYPE    = "----" ;
			permiss = string( 10, '-' ) ;
		}
		else
		{
			if ( S_ISDIR( results.st_mode ) )       { TYPE = "Dir"     ; }
			else if ( S_ISREG( results.st_mode ) )  { TYPE = "File"    ; }
			else if ( S_ISCHR( results.st_mode ) )  { TYPE = "Char"    ; }
			else if ( S_ISBLK( results.st_mode ) )  { TYPE = "Block"   ; }
			else if ( S_ISFIFO( results.st_mode ) ) { TYPE = "Fifo"    ; }
			else if ( S_ISSOCK( results.st_mode ) ) { TYPE = "Socket"  ; }
			else if ( S_ISLNK(results.st_mode ) )   { TYPE = "Syml"    ; }
			else                                    { TYPE = "Unknown" ; }
			permiss = string( 10, '-' ) ;
			if ( S_ISDIR(results.st_mode) )  { permiss[ 0 ] = 'd' ; }
			if ( S_ISLNK(results.st_mode) )  { permiss[ 0 ] = 'l' ; }
			if ( results.st_mode & S_IRUSR ) { permiss[ 1 ] = 'r' ; }
			if ( results.st_mode & S_IWUSR ) { permiss[ 2 ] = 'w' ; }
			if ( results.st_mode & S_IXUSR ) { permiss[ 3 ] = 'x' ; }
			if ( results.st_mode & S_IRGRP ) { permiss[ 4 ] = 'r' ; }
			if ( results.st_mode & S_IWGRP ) { permiss[ 5 ] = 'w' ; }
			if ( results.st_mode & S_IXGRP ) { permiss[ 6 ] = 'x' ; }
			if ( results.st_mode & S_IROTH ) { permiss[ 7 ] = 'r' ; }
			if ( results.st_mode & S_IWOTH ) { permiss[ 8 ] = 'w' ; }
			if ( results.st_mode & S_IXOTH ) { permiss[ 9 ] = 'x' ; }
			size      = d2ds( results.st_size )   ;
			moddates  = d2ds( results.st_mtime )  ;
			time_info = gmtime( &(results.st_mtime) ) ;
			strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info )  ;
			buf[ 19 ] = 0x00 ;
			moddate   = buf ;
		}
		if ( UseList ) { ENTRY = rest + ENTRY ; }
		tbadd( dslist ) ;
	}
	tbtop( dslist ) ;
}


void PFLST0A::AddPath( const string& p, const string& f, vector<path>& v )
{
	// Add paths to path vector v, from path p using generic f.  Ignore duplicates.

	char c ;

	string pat ;
	string ent ;
	string pth ;

	vector<path> vt;
	vector<path>::const_iterator it ;

	regex expression ;

	try
	{
		copy( directory_iterator( p ), directory_iterator(), back_inserter( vt ) ) ;
	}
	catch ( const filesystem_error& ex )
	{
	}

	sort( vt.begin(), vt.end() ) ;

	pat = "" ;
	for ( uint i = 0 ; i < f.size() ; i++ )
	{
		c = toupper( f[ i ] ) ;
		if      ( c == '*' ) { pat += "[^[:blank:]]*" ; }
		else if ( c == '?' ) { pat += "[^[:blank:]]"  ; }
		else                 { pat.push_back( c )     ; }
	}

	try
	{
		expression.assign( pat ) ;
	}
	catch  ( boost::regex_error& e )
	{
		v.push_back( pth ) ;
		return ;
	}

	for ( it = vt.begin() ; it != vt.end() ; ++it )
	{
		ent = (*it).string() ;
		pth = ent            ;
		ent = substr( ent, lastpos( "/", ent ) + 1 ) ;
		iupper( ent ) ;
		if ( !regex_match( ent.begin(), ent.end(), expression ) ) { continue ; }
		if ( find( v.begin(), v.end(), pth ) == v.end() )
		{
			v.push_back( pth ) ;
		}
	}
}


void PFLST0A::createSearchList( const string& w )
{
	size_t p1 ;

	string cmd ;
	string result ;
	string file   ;
	string tname  ;
	string inLine ;
	string t      ;

	char buffer[256] ;

	std::ofstream of ;

	path temp ;

	vcopy( "ZUSER", zuser, MOVE ) ;
	vcopy( "ZSCREEN", zscreen, MOVE ) ;

	temp  = temp_directory_path() / unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;
	tname = temp.native() ;

	if ( zpath.back() != '/' ) { zpath += "/" ; }

	cmd = "grep -i -s \"" + w + "\" " + zpath + "* 2>&1" ;

	of.open( tname ) ;
	FILE* pipe { popen( cmd.c_str(), "r" ) } ;

	while ( fgets( buffer, sizeof( buffer ), pipe ) != nullptr )
	{
		file   = buffer ;
		result = file.substr(0, file.size() - 1 ) ;
		of << result << endl ;
	}

	pclose( pipe ) ;
	of.close()     ;

	SearchList.clear() ;
	std::ifstream fin( tname.c_str() ) ;

	while ( getline( fin, inLine ) )
	{
		p1 = inLine.find( ':' ) ;
		if ( p1 == string::npos ) { continue ; }
		t  = inLine.substr( 0, p1 ) ;
		if ( find( SearchList.begin(), SearchList.end(), t ) == SearchList.end() )
		{
			SearchList.push_back( t ) ;
		}
	}

	fin.close() ;
	remove( tname ) ;
}


void PFLST0A::showInfo( const string& p )
{
	int rc ;

	struct stat results  ;
	struct tm* time_info ;

	char buf [ 20 ] ;
	char* buffer    ;

	size_t bufferSize = 255 ;

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

	vdefine( "IENTRY ITYPE  IINODE INLNKS IPERMISS ISIZE    ISTCDATE IMODDATE", &ientry, &itype,  &iinode, &inlnks, &ipermiss, &isize,    &istcdate, &imoddate ) ;
	vdefine( "IRLNK  IOWNER IGROUP IMAJ   IMIN     IBLKSIZE IACCDATE ISETUID",  &irlnk,  &iowner, &igroup, &imaj,   &imin,     &iblksize, &iaccdate, &isetuid  ) ;
	vdefine( "ISETGID ISTICKY", &isetgid, &isticky ) ;

	ientry = p ;
	if ( S_ISDIR( results.st_mode ) )       { itype = "Directory"     ; }
	else if ( S_ISREG( results.st_mode ) )  { itype = "File"          ; }
	else if ( S_ISCHR( results.st_mode ) )  { itype = "Character"     ; }
	else if ( S_ISBLK( results.st_mode ) )  { itype = "Block"         ; }
	else if ( S_ISFIFO( results.st_mode ) ) { itype = "Fifo"          ; }
	else if ( S_ISSOCK( results.st_mode ) ) { itype = "Socket"        ; }
	else if ( S_ISLNK(results.st_mode ) )   { itype = "Symbolic link" ; }
	else                                    { itype = "Unknown"       ; }

	iowner = "" ;
	igroup = "" ;

	struct passwd* pw = getpwuid( results.st_uid ) ;
	if ( pw )
	{
		iowner = pw->pw_name ;
	}

	struct group* gr = getgrgid( results.st_gid ) ;
	if ( gr )
	{
		igroup = gr->gr_name ;
	}

	ipermiss = string( 9, '-' ) ;
	if ( results.st_mode & S_IRUSR ) { ipermiss[ 0 ] = 'r' ; }
	if ( results.st_mode & S_IWUSR ) { ipermiss[ 1 ] = 'w' ; }
	if ( results.st_mode & S_IXUSR ) { ipermiss[ 2 ] = 'x' ; }
	if ( results.st_mode & S_IRGRP ) { ipermiss[ 3 ] = 'r' ; }
	if ( results.st_mode & S_IWGRP ) { ipermiss[ 4 ] = 'w' ; }
	if ( results.st_mode & S_IXGRP ) { ipermiss[ 5 ] = 'x' ; }
	if ( results.st_mode & S_IROTH ) { ipermiss[ 6 ] = 'r' ; }
	if ( results.st_mode & S_IWOTH ) { ipermiss[ 7 ] = 'w' ; }
	if ( results.st_mode & S_IXOTH ) { ipermiss[ 8 ] = 'x' ; }

	if ( results.st_mode & S_ISUID ) { isetuid = "YES"; }
	else                             { isetuid = "NO "; }
	if ( results.st_mode & S_ISGID ) { isetgid = "YES"; }
	else                             { isetgid = "NO "; }
	if ( results.st_mode & S_ISVTX ) { isticky = "YES"; }
	else                             { isticky = "NO "; }

	iinode = d2ds( results.st_ino )   ;
	inlnks = d2ds( results.st_nlink ) ;
	isize  = d2ds( results.st_size )  ;

	time_info = gmtime( &(results.st_ctime) ) ;
	strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ; buf[ 19 ]  = 0x00 ; istcdate = buf ;

	time_info = gmtime( &(results.st_mtime) ) ;
	strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ; buf[ 19 ]  = 0x00 ; imoddate = buf ;

	time_info = gmtime( &(results.st_atime) ) ;
	strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ; buf[ 19 ]  = 0x00 ; iaccdate = buf ;


	irlnk = "" ;
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
				irlnk = string( buffer, rc ) ;
				delete[] buffer ;
				break           ;
			}
		}
	}

	imaj     = d2ds( major( results.st_dev ) ) ;
	imin     = d2ds( minor( results.st_dev ) ) ;
	iblksize = d2ds( results.st_blksize )      ;

	while ( true )
	{
		display( "PFLST0A2" ) ;
		if ( RC == 8 ) { break ; }
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
	string pgm ;
	string p   ;
	string t   ;

	std::ofstream of ;
	boost::system::error_code ec ;

	cw = upper( word( zcmd, 1 ) ) ;

	if ( cw == "" ) { return 0 ; }

	ws = subword( zcmd, 2 ) ;
	w2 = word( zcmd, 2 )    ;
	w3 = word( zcmd, 3 )    ;

	if ( cw == "SORT" )
	{
		iupper( w2 ) ;
		iupper( w3 ) ;
		if ( w2 == "" ) { w2 = "ENTRY" ; }
		if ( w3 == "" && w2 == "CHA" ) { w3 = "D" ; }
		else if ( w3 == "" ) { w3 = "A"     ; }
		if      ( w2 == "ENTRY"   ) { tbsort( dslist, "(ENTRY,C,"+w3+")"   )  ; }
		else if ( w2 == "TYPE"    ) { tbsort( dslist, "(TYPE,C,"+w3+")"    )  ; }
		else if ( w2 == "PERMISS" ) { tbsort( dslist, "(PERMISS,C,"+w3+")" )  ; }
		else if ( w2 == "SIZE"    ) { tbsort( dslist, "(SIZE,N,"+w3+")"    )  ; }
		else if ( w2 == "MOD"     ) { tbsort( dslist, "(MODDATES,N,"+w3+")" ) ; }
		else if ( w2 == "CHA"     ) { tbsort( dslist, "(MODDATES,N,"+w3+")" ) ; }
		else                        { return 8 ;                                }
		if ( RC > 0 ) { msg = "FLST016" ; return 4 ; }
		return 0 ;
	}
	else if ( findword( cw, "S B E EDIT" ) && ws != "" )
	{
		p = zpath + "/" + ws ;
		if ( is_directory( p ) && (cw == "S" || cw == "B" ) )
		{
			vcopy( "ZFLSTPGM", pgm, MOVE ) ;
			select( "PGM(" + pgm + ") PARM(" + p + ")" ) ;
			zcmd = "" ;
		}
		else if ( is_regular_file( p ) )
		{
			if ( findword( cw, "E EDIT" ) )
			{
				edit( p, "", eimac, eprof, lcmtab ) ;
			}
			else
			{
				browse( p ) ;
			}
			zcmd = "" ;
		}
		return 0 ;
	}
	else if ( cw == "BACK" || cw == "S" )
	{
		if ( zpath.back() == '/' ) { zpath = zpath.substr( 0, zpath.size()-1) ; }
		zpath = substr( zpath, 1, lastpos( "/", zpath)-1 ) ;
		return 0 ;
	}
	else if ( cw == "CD" )
	{
		if ( ws == "" ) { ws = zhome ; }
		t = zpath ;
		if ( t.back() == '/' ) { t.erase( t.size()-1, 1) ; }
		if ( substr( ws, 1, 1 ) != "/" ) { t += "/" + ws ; }
		else                             { t  = ws       ; }
		if ( t.find_first_of( "?*[" ) != string::npos ) { t = expandName( t ) ; }
		if ( t == "" ) { return 8 ; }
		if ( is_directory( t ) ) { zpath = t ; return 0 ; }
		else { return 8 ; }
	}
	else if ( cw == "L" )
	{
		tbvclear( dslist ) ;
		ENTRY = word( ws, 1 ) ;
		if ( ENTRY.back() != '*' ) { ENTRY.push_back( '*' ) ; }
		if ( w3 == "PREV" )
		{
			tbsarg( dslist, "", "PREVIOUS", "(ENTRY,LE)" ) ;
			tbscan( dslist, "", "", "", "", "", "CRP"  ) ;
		}
		else
		{
			if ( w3 == "FIRST" )
			{
				tbsarg( dslist, "", "NEXT", "(ENTRY,GE)" ) ;
				tbtop( dslist )  ;
				tbscan( dslist, "", "", "", "", "", "CRP" ) ;
			}
			else
			{
				if ( w3 == "LAST" )
				{
					tbsarg( dslist, "", "PREVIOUS", "(ENTRY,LE)" ) ;
					tbtop( dslist ) ;
					tbscan( dslist, "", "", "", "", "", "CRP"  ) ;
				}
				else
				{
					tbsarg( dslist, "", "NEXT", "(ENTRY,GE)" ) ;
					tbscan( dslist, "", "", "", "", "", "CRP" ) ;
				}
			}
		}
		return 4 ;
	}
	else if ( cw == "MKDIR" )
	{
		if ( ws[ 0 ] != '/' ) { ws = zpath + "/" + ws ; }
		create_directory( ws, ec ) ;
		if ( ec.value() == boost::system::errc::success )
		{
			msg = "FLST012B" ;
		}
		else
		{
			msg = "FLST012C"   ;
			rsn = ec.message() ;
			llog( "E", "Create of directory " + ws + " failed with " + ec.message() << endl ) ;
		}
		zcmd = "" ;
		return 4  ;
	}
	else if ( cw == "TOUCH" )
	{
		if ( ws[ 0 ] != '/' ) { ws = zpath + "/" + ws ; }
		if ( !exists( ws ) )
		{
			of.open( ws.c_str() ) ;
			if ( !of.fail() )
			{
				of << endl ;
				of.close() ;
				msg = "FLST012E" ;
			}
			else { msg = "FLST012F" ; }
		}
		else
		{
			msg = "FLST012D" ;
		}
		zcmd = "" ;
		return 4  ;
	}
	return 8 ;
}


void PFLST0A::copyDirs( const string& src, const string& dest, const string& dirrec, bool& errs )
{
	boost::system::error_code ec ;

	copy_directory( src, dest, ec ) ;
	if ( ec.value() != boost::system::errc::success )
	{
		errs    = true         ;
		msg     = "FLST011V"   ;
		rsn     = ec.message() ;
		message = ec.message() ;
		llog( "E", "Copy of directory" + src + " to " + dest + " failed with " + ec.message() << endl ) ;
		return ;
	}

	for ( directory_iterator file( src, ec ) ; file != directory_iterator() ; ++file )
	{
		if ( ec.value() != boost::system::errc::success )
		{
			errs = true ;
			llog( "E", "Listing directory " << src << " failed with " + ec.message() << endl ) ;
			return ;
		}
		path current( file->path() ) ;
		if( is_directory( current ) )
		{
			if ( dirrec == "/" )
			{
				copyDirs( current.string(), dest + "/" + current.filename().string(), dirrec, errs ) ;
			}
			else
			{
				copy_directory( current, dest / current.filename(), ec ) ;
				if ( ec.value() != boost::system::errc::success )
				{
					errs = true ;
					llog( "E", "Copy of directory " << current.filename() << " failed with " + ec.message() << endl ) ;
				}
			}
		}
		else if ( is_regular_file( current ) )
		{
			copy_file( current, dest / current.filename(), ec ) ;
			if ( ec.value() != boost::system::errc::success )
			{
				errs = true ;
				llog( "E", "Copy of file " << current.filename() << " failed with " + ec.message() << endl ) ;
			}
		}
		else if ( is_symlink( current ) )
		{
			copy_symlink( current, dest / current.filename(), ec ) ;
			if ( ec.value() != boost::system::errc::success )
			{
				errs = true ;
				llog( "E", "Copy of symlink " << current.filename() << " failed with " + ec.message() << endl ) ;
			}
		}
		else
		{
			errs = true ;
			llog( "E", "Ignoring entry " << current.filename() << " Not a regular file, directory or symlink" << endl ) ;
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

	string opermiss ;
	string osetuid  ;
	string osetgid  ;
	string osticky  ;
	string oowner   ;
	string ogroup   ;
	string oownern  ;
	string ogroupn  ;

	mode_t t  ;
	uid_t uid ;
	gid_t gid ;

	struct stat results ;

	vdefine( "IENTRY ITYPE  IPERMISS ", &ientry, &itype, &ipermiss ) ;
	vdefine( "IOWNER IGROUP ISETUID", &iowner, &igroup, &isetuid  ) ;
	vdefine( "ISETGID ISTICKY IOWNERN IGROUPN", &isetgid, &isticky, &iownern, &igroupn ) ;

	lstat( p.c_str(), &results ) ;

	ientry = p ;
	if ( S_ISDIR( results.st_mode ) )       { itype = "Directory"     ; }
	else if ( S_ISREG( results.st_mode ) )  { itype = "File"          ; }
	else if ( S_ISCHR( results.st_mode ) )  { itype = "Character"     ; }
	else if ( S_ISBLK( results.st_mode ) )  { itype = "Block"         ; }
	else if ( S_ISFIFO( results.st_mode ) ) { itype = "Fifo"          ; }
	else if ( S_ISSOCK( results.st_mode ) ) { itype = "Socket"        ; }
	else if ( S_ISLNK(results.st_mode ) )   { itype = "Symbolic link" ; }
	else                                    { itype = "Unknown"       ; }

	struct passwd* pwd ;
	struct group*  grp ;

	iowner  = "" ;
	iownern = "" ;
	pwd = getpwuid( results.st_uid ) ;
	if ( pwd )
	{
		iowner  = pwd->pw_name ;
		iownern = d2ds( pwd->pw_uid ) ;
	}

	igroup  = "" ;
	igroupn = "" ;
	grp = getgrgid( results.st_gid ) ;
	if ( grp )
	{
		igroup  = grp->gr_name ;
		igroupn = d2ds( grp->gr_gid ) ;
	}

	i1 = 0 ;
	i2 = 0 ;
	i3 = 0 ;

	if ( results.st_mode & S_IRUSR ) { i1 += 4 ; }
	if ( results.st_mode & S_IWUSR ) { i1 += 2 ; }
	if ( results.st_mode & S_IXUSR ) { i1 += 1 ; }
	if ( results.st_mode & S_IRGRP ) { i2 += 4 ; }
	if ( results.st_mode & S_IWGRP ) { i2 += 2 ; }
	if ( results.st_mode & S_IXGRP ) { i2 += 1 ; }
	if ( results.st_mode & S_IROTH ) { i3 += 4 ; }
	if ( results.st_mode & S_IWOTH ) { i3 += 2 ; }
	if ( results.st_mode & S_IXOTH ) { i3 += 1 ; }

	ipermiss = d2ds( i1 ) + d2ds( i2 ) + d2ds( i3 ) ;

	if ( results.st_mode & S_ISUID ) { isetuid = "/" ; }
	else                             { isetuid = ""  ; }
	if ( results.st_mode & S_ISGID ) { isetgid = "/" ; }
	else                             { isetgid = ""  ; }
	if ( results.st_mode & S_ISVTX ) { isticky = "/" ; }
	else                             { isticky = ""  ; }

	osetuid  = isetuid  ;
	osetgid  = isetgid  ;
	osticky  = isticky  ;
	opermiss = ipermiss ;
	oowner   = iowner   ;
	ogroup   = igroup   ;
	oownern  = iownern  ;
	ogroupn  = igroupn  ;
	msg      = ""       ;
	t        = results.st_mode ;
	changed  = false    ;

	display( "PFLST0A6", msg, "ZCMD" ) ;
	if ( RC == 8 )
	{
		zcmd    = ""          ;
		setmsg( "FLST017" )   ;
		message = "Cancelled" ;
	}
	else
	{
		if ( ipermiss != opermiss )
		{
			i = ds2d( string( 1, ipermiss[ 0 ] ) ) ;
			if ( i >= 4 ) { t = t |  S_IRUSR ; i = i - 4 ; }
			else          { t = t & ~S_IRUSR ;             }
			if ( i >= 2 ) { t = t |  S_IWUSR ; i = i - 2 ; }
			else          { t = t & ~S_IWUSR ; }
			if ( i == 1 ) { t = t |  S_IXUSR ; }
			else          { t = t & ~S_IXUSR ; }
			i = ds2d( string( 1, ipermiss[ 1 ] ) ) ;
			if ( i >= 4 ) { t = t |  S_IRGRP ; i = i - 4 ; }
			else          { t = t & ~S_IRGRP ;             }
			if ( i >= 2 ) { t = t |  S_IWGRP ; i = i - 2 ; }
			else          { t = t & ~S_IWGRP ; }
			if ( i == 1 ) { t = t |  S_IXGRP ; }
			else          { t = t & ~S_IXGRP ; }
			i = ds2d( string( 1, ipermiss[ 2 ] ) ) ;
			if ( i >= 4 ) { t = t |  S_IROTH ; i = i - 4 ; }
			else          { t = t & ~S_IROTH ;             }
			if ( i >= 2 ) { t = t |  S_IWOTH ; i = i - 2 ; }
			else          { t = t & ~S_IWOTH ; }
			if ( i == 1 ) { t = t |  S_IXOTH ; }
			else          { t = t & ~S_IXOTH ; }
		}
		if ( isetuid != osetuid )
		{
			if ( isetuid == "/" ) { t = t |  S_ISUID ; }
			else                  { t = t & ~S_ISUID ; }
		}
		if ( isetgid != osetgid )
		{
			if ( isetgid == "/" ) { t = t |  S_ISGID ; }
			else                  { t = t & ~S_ISGID ; }
		}
		if ( isticky != osticky )
		{
			if ( isticky == "/" ) { t = t |  S_ISVTX ; }
			else                  { t = t & ~S_ISVTX ; }
		}
		if ( t != results.st_mode )
		{
			changed = true ;
			if ( chmod( p.c_str(), t ) != 0 ) { msg = "FLST018" ; }
		}
		if ( msg == "" && iownern != oownern )
		{
			changed = true ;
			pwd = getpwuid( ds2d( iownern ) ) ;
			if ( pwd == NULL )
			{
				msg = "FLST019" ;
			}
			else
			{
				changed = true    ;
				uid = pwd->pw_uid ;
				if ( chown( p.c_str(), uid, -1 ) == -1 ) { msg = "FLST012I" ; }
			}
		}
		else if ( msg == "" && iowner != oowner )
		{
			changed = true ;
			pwd = getpwnam( iowner.c_str() ) ;
			if ( pwd == NULL )
			{
				msg = "FLST011B" ;
			}
			else
			{
				uid = pwd->pw_uid ;
				if ( chown( p.c_str(), uid, -1 ) == -1 ) { msg = "FLST012I" ; }
			}
		}
		if ( msg == "" && igroupn != ogroupn )
		{
			changed = true ;
			grp = getgrgid( ds2d( igroupn ) ) ;
			if ( grp == NULL )
			{
				msg = "FLST011L" ;
			}
			else
			{
				gid = grp->gr_gid ;
				if ( chown( p.c_str(), -1, gid ) == -1 ) { msg = "FLST012J" ; }
			}
		}
		else if ( msg == "" && igroup != ogroup )
		{
			changed = true ;
			grp = getgrnam( igroup.c_str() ) ;
			if ( grp == NULL )
			{
				msg = "FLST011C" ;
			}
			else
			{
				gid = grp->gr_gid ;
				if ( chown( p.c_str(), -1, gid ) == -1 ) { msg = "FLST012J" ; }
			}
		}
		stat( p.c_str(), &results ) ;
		if ( changed )
		{
			if ( msg != "" ) { message = "Modify Failed" ; }
			else             { message = "Modified"      ; setmsg( "FLST011A" ) ; }
		}
	}
	vdelete( "IENTRY ITYPE IPERMISS IOWNER IGROUP IOWNERN IGROUPN ISETUID ISETGID ISTICKY" ) ;
}


void PFLST0A::browseTree( const string& tname )
{
	int i ;
	int csrrow ;


	string tsel   ;
	string tfile  ;
	string tentry ;
	string panel  ;
	string csr    ;
	string tab    ;
	string line   ;
	string pgm    ;

	std::ifstream fin ;
	fin.open( tname.c_str() ) ;
	if ( !fin.is_open() )
	{
		RC   = 16 ;
		llog( "E", "Error opening file " << tname << endl ) ;
		return    ;
	}

	vdefine( "TSEL TFILE TENTRY ZDIR", &tsel, &tfile, &tentry, &zdir ) ;
	vcopy( "ZFLSTPGM", pgm, MOVE ) ;
	tab = "FTREE" + d2ds( taskid(), 3 ) ;

	tbcreate( tab, "", "(TSEL,TFILE,TENTRY)", NOWRITE ) ;

	tsel   = "" ;
	i      = 1  ;
	while ( getline( fin, line ) )
	{
		if ( i == 1 )
		{
			zdir = strip( line ) ;
		}
		else if ( i % 2 )
		{
			tfile = strip( line ) ;
			tbadd( tab ) ;
		}
		else
		{
			tentry  = strip( line ) ;
		}
		i++ ;

	}
	tbtop( tab ) ;

	ztdvrows  = 1 ;
	ztdsels   = 0 ;
	ztdtop    = 0 ;
	csrrow    = 0 ;
	csr       = "ZCMD" ;

	while ( true )
	{
		if ( msg == "" && ztdsels > 0 )
		{
			ztdsels-- ;
		}
		if ( ztdsels == 0 )
		{
			if ( ztdvrows > 0 )
			{
				tbtop( tab ) ;
				tbskip( tab, ztdtop ) ;
			}
			else
			{
				tbbottom( tab ) ;
				tbskip( tab, - (ztddepth-2) ) ;
				if ( RC > 0 ) { tbtop( tab ) ; }
			}
			panel = "PFLST0A8" ;
		}
		else
		{
			panel = "" ;
		}
		if ( msg == "" ) { zcmd = "" ; }
		tbdispl( tab, panel, msg, csr, csrrow, 1, "YES", "CRP" ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		if ( tsel == "S" )
		{
			select( "PGM(" + pgm + ") PARM(BROWSE " + tfile + ")" ) ;
		}
		else if ( tsel == "I" )
		{
			showInfo( tfile ) ;
		}
		else if ( tsel == "E" )
		{
			select( "PGM(" + pgm + ") PARM(EDIT " + tfile + ")" ) ;
		}
	}
	tbend( tab ) ;
	return       ;

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
	size_t pos   ;

	bool showl   ;

	string type  ;
	string dir1  ;
	string entry ;
	string dir   ;
	string cpos  ;
	string data1 ;
	string data2 ;

	ZRC   = 8  ;
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
		if ( showl ) { zpath = dir ; return showListing() ; }
	}


	if ( dir == "" )
	{
		dir = zhome ;
		pos = zhome.size() ;
	}
	else if ( dir[ 0 ] == '?' )
	{
		if ( dir.size() > 1 )
		{
			zpath = dir ;
			zpath[ 0 ] = '/' ;
		}
		else
		{
			zpath = zhome ;
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
		llog( "E", "Error listing directory " << ex.what() << endl ) ;
		return "" ;
	}

	try
	{
		if ( type == "ALL" )
		{
			;
		}
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
		llog( "E", "Error listing directory " << ex.what() << endl ) ;
		return "" ;
	}

	sort( v.begin(), v.end() ) ;

	for ( vec::const_iterator it( v.begin() ) ; it != v.end() ; ++it )
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
	// Expand field from listing of ZPLIB, ZORXPATH, ZLDPATH, REXX_PATH or PATH

	// Cursor sensitive.  Only characters before the current cursor position (if > 1) in the field (ZFECSRP) will
	// be taken into account

	// If first parameter is PNL, panel files
	// If first parameter is REXX, rexx programs
	// If first parameter is PGM, programs (strip off "lib" and ".so" before comparison)
	// If first parameter is CMD, files in ZORXPATH, REXX_PATH and PATH

	int i        ;
	int n        ;

	size_t pos   ;

	string Paths ;
	string type  ;
	string dir1  ;
	string entry ;
	string dir   ;
	string cpos  ;

	ZRC = 8 ;

	typedef vector<path> vec ;
	vec v ;
	vec::iterator vec_end ;

	vector<string> mod ;
	vector<string>::iterator it ;
	vector<string>::iterator mod_end ;

	set<string>processed ;

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
	else if ( type == "CMD" )
	{
		vcopy( "ZORXPATH", Paths, MOVE ) ;
		Paths = mergepaths( Paths, getenv( "REXX_PATH" ), getenv( "PATH" ) ) ;
	}
	else
	{
		return "" ;
	}

	v.clear() ;
	for ( n = getpaths( Paths ), i = 1 ; i <= n ; i++ )
	{
		dir1 = getpath( Paths, i ) ;
		if ( processed.count( dir1 ) > 0 )
		{
			continue ;
		}
		processed.insert( dir1 ) ;
		if ( ( !exists( dir1 ) || !is_directory( dir1 ) ) )
		{
			continue ;
		}
		try
		{
			copy( directory_iterator( dir1 ), directory_iterator(), back_inserter( v ) ) ;
		}
		catch ( const filesystem_error& ex )
		{
			ZRESULT = "List Error" ;
			RC      = 16 ;
			llog( "E", "Error listing directory " << ex.what() << endl ) ;
			return "" ;
		}
	}

	vec_end = remove_if( v.begin(), v.end(),
		[](const path& a) { return !is_regular_file( a.string() ) ; } ) ;

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
		else
		{
			return entry ;
		}
	}
	return entry ;
}


string PFLST0A::showListing()
{
	int csrrow ;

	string w1       ;
	string w2       ;
	string ws       ;
	string opath    ;
	string fldirs   ;
	string ofldirs  ;
	string flhidden ;
	string ohidden  ;

	string panel    ;
	string csr      ;

	vdefine( "SEL ENTRY TYPE FLDIRS FLHIDDEN", &sel, &ENTRY, &TYPE, &fldirs, &flhidden ) ;
	vget( "FLDIRS FLHIDDEN", PROFILE ) ;

	dslist = "DSLST" + d2ds( taskid(), 3 ) ;
	createFileList2( fldirs ) ;

	RC       = 0 ;
	ZRC      = 0 ;
	ztdvrows = 1 ;
	ztdsels  = 0 ;
	ztdtop   = 0 ;
	csrrow   = 0 ;
	csr      = "ZCMD" ;
	msg      = "" ;

	while ( true )
	{
		if ( msg == "" && ztdsels > 0 )
		{
			ztdsels-- ;
		}
		if ( ztdsels == 0 )
		{
			if ( ztdvrows > 0 )
			{
				tbtop( dslist ) ;
				tbskip( dslist, ztdtop ) ;
			}
			else
			{
				tbbottom( dslist ) ;
				tbskip( dslist, - (ztddepth-2) ) ;
				if ( RC > 0 ) { tbtop( dslist ) ; }
			}
			panel = "PFLST0A7" ;
		}
		else
		{
			panel = "" ;
		}
		opath   = zpath    ;
		ofldirs = fldirs   ;
		ohidden = flhidden ;
		tbdispl( dslist, panel, msg, csr, csrrow, 1, "YES", "CRP" ) ;
		if ( RC == 8 ) { ZRC = 8 ; break ; }
		msg = "" ;
		csr = "ZCMD" ;
		w1  = upper( word( zcmd, 1 ) ) ;
		w2  = word( zcmd, 2 )    ;
		ws  = subword( zcmd, 2 ) ;
		if ( w1 == "REFRESH" ) { tbend( dslist ) ; createFileList2( fldirs )     ; continue ; }
		if ( w1 == "O" )       { tbend( dslist ) ; createFileList2( fldirs, w2 ) ; continue ; }
		if ( w1 == "S" && zpath != "/" )
		{
			if ( zpath.back() == '/' ) { zpath = zpath.substr( 0, zpath.size()-1) ; }
			zpath = substr( zpath, 1, lastpos( "/", zpath)-1 ) ;
			tbend( dslist )   ;
			createFileList2( fldirs ) ;
			continue ;
		}
		if ( w1 == "CD" )
		{
			if ( ws == "" ) { ws = zhome ; }
			if ( zpath.back() == '/' ) { zpath = zpath.substr( 0, zpath.size()-1) ; }
			if ( substr( ws, 1, 1 ) != "/" ) { zpath += "/" + ws ; }
			else                             { zpath  = ws       ; }
			tbend( dslist ) ; createFileList2( fldirs ) ; continue ;
		}
		vget( "ZVERB", SHARED ) ;
		if ( ofldirs != fldirs )
		{
			tbend( dslist ) ;
			createFileList2( fldirs ) ;
			continue ;
		}
		if ( ohidden != flhidden )
		{
			tbend( dslist ) ;
			createFileList2( fldirs ) ;
			continue ;
		}
		if ( opath != zpath )
		{
			if ( ( !exists( zpath ) || !is_directory( zpath ) ) )
			{
				msg = "PSYS012A" ;
			}
			else
			{
				tbend( dslist ) ;
				createFileList2( fldirs ) ;
			}
			continue ;
		}
		if ( sel == "S" )
		{
			zpath = createEntry( zpath, ENTRY ) ;
			tbend( dslist ) ;
			createFileList2( fldirs ) ;
		}
		else if ( sel == "/" )
		{
			zpath = createEntry( zpath, ENTRY ) ;
			tbend( dslist ) ;
			return zpath    ;
		}
	}
	tbend( dslist ) ;
	return ""       ;
}


void PFLST0A::createFileList2( const string& fldirs, string filter )
{
	int i    ;

	string p ;
	string t ;

	struct stat results ;
	typedef vector< path > vec ;

	tbcreate( dslist, "", "(SEL,ENTRY,TYPE)", NOWRITE ) ;

	vec v;

	if ( zpath == "" ) { zpath = "/" ; }
	iupper( filter ) ;

	vcopy( "FLHIDDEN", t, MOVE ) ;

	try
	{
		copy( directory_iterator( zpath ), directory_iterator(), back_inserter( v ) ) ;
	}
	catch ( const filesystem_error& ex )
	{
		ZRESULT = "List Error" ;
		RC      = 16 ;
		llog( "E", "Error listing directory " << ex.what() << endl ) ;
		return ;
	}

	sort( v.begin(), v.end() ) ;
	for ( vec::const_iterator it (v.begin()) ; it != v.end() ; ++it )
	{
		sel   = "" ;
		ENTRY = (*it).string() ;
		p     = ENTRY          ;
		i     = lastpos( "/", ENTRY ) + 1 ;
		ENTRY = substr( ENTRY, i ) ;
		if ( t != "/" && ENTRY[ 0 ] == '.' ) { continue ; }
		if ( filter != "" && pos( filter, upper( ENTRY ) ) == 0 ) { continue ; }
		if ( lstat( p.c_str(), &results ) != 0 )
		{
			TYPE = "----" ;
			tbadd( dslist ) ;
			continue ;
		}
		if      ( S_ISDIR( results.st_mode ) )  { TYPE = "Dir"     ; }
		else if ( S_ISREG( results.st_mode ) )  { TYPE = "File"    ; }
		else if ( S_ISCHR( results.st_mode ) )  { TYPE = "Char"    ; }
		else if ( S_ISBLK( results.st_mode ) )  { TYPE = "Block"   ; }
		else if ( S_ISFIFO( results.st_mode ) ) { TYPE = "Fifo"    ; }
		else if ( S_ISSOCK( results.st_mode ) ) { TYPE = "Socket"  ; }
		else if ( S_ISLNK(results.st_mode ) )   { TYPE = "Syml"    ; }
		else                                    { TYPE = "Unknown" ; }
		if ( fldirs == "/" && TYPE != "Dir" ) { continue ; }
		tbadd( dslist ) ;
	}
	tbtop( dslist ) ;
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


string PFLST0A::createEntry( const string& s1, const string& s2 )
{
	if ( s1.back() == '/' ) { return s1 + s2       ; }
	else                    { return s1 + "/" + s2 ; }
}


string PFLST0A::expandName( const string& s )
{
	// Resolve name if contains *, ? or regex.  If more than one, return ""

	uint i ;

	size_t p1 ;

	char c ;

	string dir  ;
	string dir1 ;
	string pat  ;

	regex expression ;

	typedef vector<path> vec ;

	vec v ;
	vec::iterator new_end  ;
	vec::const_iterator it ;

	p1 = s.find_last_of( "/" ) ;
	if ( p1 == string::npos ) { return "" ; }

	dir = s.substr( 0, p1 ) ;

	pat = "" ;
	for ( uint i = 0 ; i < s.size() ; i++ )
	{
		c = s[ i ] ;
		if      ( c == '*' ) { pat += "[^[:blank:]]*" ; }
		else if ( c == '?' ) { pat += "[^[:blank:]]"  ; }
		else                 { pat.push_back( c )     ; }
	}
	try
	{
		expression.assign( pat ) ;
	}
	catch  ( boost::regex_error& e )
	{
		setmsg( "PSYS012P" ) ;
		return "" ;
	}

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
	for ( i = 0, it = v.begin() ; it != v.end() ; ++it )
	{
		dir = (*it).string() ;
		if ( !regex_match( dir.begin(), dir.end(), expression ) ) { continue ; }
		i++ ;
		if ( i > 1 ) { return "" ; }
		dir1 = dir ;
	}
	return dir1 ;
}


// ============================================================================================ //

extern "C" { pApplication* maker() { return new PFLST0A ; } }
extern "C" { void destroy(pApplication* p) { delete p ; } }
