/* Compile with ::                                                                      */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libpflst0a.so -o libpflst0a.so pflst0a.cpp  */

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


/************************************************************************************************************/
/*                                                                                                          */
/* Display a file list for a directory and invoke browse if a file is selected or list the new directory    */
/* Use system variable ZFLSTPGM to refer to this.                                                           */
/*                                                                                                          */
/* If invoked with a PARM of path, list path                                                                */
/* If invoked with a PARM of DIR, list directory                                                            */
/* If invoked with a PARM of DIRR, list directory recursively                                               */
/* If invoked with a PARM of INFO, display info for entry and then exit.                                    */
/* If invoked with a PARM of LIST, use passed list to create a file list instead of listing a directory     */
/* If invoked with a PARM of EXPAND, will expand the passed directory name to the next level                */
/*                SUBPARM of ALL - return all types (fully qualified)                                       */
/*                SUBPARM of DO1 - return only directories                                                  */
/*                SUBPARM of FO1 - return only fully qualified files                                        */
/*                SUBPARM of FO2 - return only the file names (without the directory name)                  */
/* If invoked with a PARM of EXPAND1, will expand field according to subtype                                */
/*                SUBPARM of PNL  - panel files in ZPLIB                                                    */
/*                SUBPARM of REXX - REXX programs in ALTLIB                                                 */
/*                SUBPARM of PGM  - C++ programs in ZDLPATH                                                 */
/*                SUBPARM of CMD  - commands in ALTLIB paths and PATH                                       */
/* If invoked with a PARM of EXPAND2, will display directory listing.                                       */
/* If invoked with a PARM of QLIB, will expand a path variable.                                             */
/*                                                                                                          */
/* ZRC/ZRSN/ZRESULT codes                                                                                   */
/*                                                                                                          */
/* For PARM=LIST                                                                                            */
/*     ZRC=8/ZRSN=12 - Entry passed does not exist                                                          */
/*     ZRC=8/ZRSN=16 - Permission denied                                                                    */
/*     ZRC=8/ZRSN=20 - Unkown error occured                                                                 */
/*                                                                                                          */
/************************************************************************************************************/

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/circular_buffer.hpp>
#include <vector>

#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <utime.h>
#include <sys/xattr.h>
#include <pwd.h>
#include <grp.h>

#include <mntent.h>
#include <sys/statvfs.h>

#include "../lspfall.h"
#include "pflst0a.h"

#define _PATH_PROC_MOUNTS       "/proc/mounts"

using namespace std ;
using namespace boost ;
using namespace boost::filesystem ;

LSPF_APP_MAKER( pflst0a )


pflst0a::pflst0a()
{
	STANDARD_HEADER( "File list application for lspf", "1.0.2" )

	vdefine( "ZCURFLD", &zcurfld ) ;
	vdefine( "ZCURINX  ZTDTOP ZTDVROWS ZTDSELS ZTDDEPTH", &zcurinx, &ztdtop, &ztdvrows, &ztdsels, &ztddepth ) ;
	vdefine( "LOGERR PFLSCRL PFLEVEP PFLUREF PFLSDEF", &log_error, &pflscrl, &pflevep, &pfluref, &pflsdef ) ;
	vdefine( "SEL ENTRY MESSAGE TYPE PERMISS", &sel, &entry, &message, &type, &permiss ) ;
	vdefine( "SIZE STCDATE STCDATES MODDATE MODDATES", &size, &stcdate, &stcdates, &moddate, &moddates ) ;
	vdefine( "ACCDATE ACCDATES OWNER GROUP", &accdate, &accdates, &owner, &group ) ;
	vdefine( "ZVERB ZHOME ZCMD ZPATH ZSCRNAME", &zverb, &zhome, &zcmd, &zpath, &zscrname ) ;
	vdefine( "ZUPROF RSN AFHIDDEN AFFULL EXGEN FLDIRS", &zuprof, &rsn, &afhidden, &afsfull, &exgen, &fldirs ) ;
	vdefine( "FMSG1 FMSG2 FMSG3 FMSG4 TIMEOUT", &fmsg1, &fmsg2, &fmsg3, &fmsg4, &timeout ) ;
	vdefine( "PFLSCMD PFLHBAR ZHOTBARD ZHOTBARS CCPATH", &pflscmd, &pflhbar, &zhotbard, &zhotbars, &ccpath ) ;
	vdefine( "CRP ZHOTBARW ZCURPOS", &crp, &zhotbarw, &zcurpos ) ;

	ztdvrows  = 1 ;
	ztdsels   = 0 ;
	ztdtop    = 0 ;
	msg       = "" ;
	ccpath    = "" ;
	findstr   = "" ;
	timeout   = "/" ;
	useList   = false ;
	recursv   = false ;
	rebuild1  = false ;
	rebuild2  = false ;
	include   = true  ;
	initsort  = true  ;
	log_error = "OFF" ;
	sort_parm = "(ENTRY,C,A)" ;
}


void pflst0a::application()
{
	int RCode ;

	int crpx   = 0 ;
	int csrrow = 0 ;
	int ppos   = 0 ;

	bool terminat = false ;
	bool cur2sel  = false ;

	string cursor ;
	string entry1 ;
	string msgloc ;
	string oexgen ;
	string ohidden;
	string ofull  ;
	string opath  ;
	string panel  ;

	string autosel = "YES" ;

	vdefine( "OEXGEN", &oexgen ) ;

	vcopy( "ZUSER", zuser ) ;
	vcopy( "ZSCREEN", zscreen ) ;

	vget( "ZHOME", SHARED ) ;
	vget( "ZUPROF AFHIDDEN AFFULL EXGEN PFLSCMD PFLSCRL", PROFILE ) ;

	vget( "PFLHBAR", PROFILE ) ;
	if ( RC == 8 ) { pflhbar = "/" ; }

	vget( "PFLEVEP", PROFILE ) ;
	if ( RC == 8 ) { pflevep = "/" ; }

	vget( "PFLUREF", PROFILE ) ;
	if ( RC == 8 ) { pfluref = "/" ; }

	vget( "PFLSDEF", PROFILE ) ;
	if ( RC == 8 ) { pflsdef = "1" ; }

	stats = ( get_profile_var( "PFLSTTS", "ON" ) == "ON" ) ;

	affull = ( afsfull == "/" ) ;

	if ( pflscmd == "" ) { pflscmd = "OFF" ; }

	if ( setup_parameters() > 0 )
	{
		return ;
	}

	dslist = "DSL" + d2ds( taskid(), 5 ) ;

	top      = 1 ;
	ntop     = 1 ;
	ztdvrows = 1 ;
	retPos   = 0 ;
	msgloc   = "" ;
	opath    = zpath ;
	cursor   = "ZCMD" ;

	pnames.rset_capacity( 99 ) ;

	create_filelist1() ;
	if ( RC > 0 ) { setmsg( "FLST015" ) ; }

	setup_hotbar() ;

	while ( true )
	{
		if ( msg == "" && ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			if ( rebuild1 )
			{
				create_filelist1( true ) ;
				rebuild1 = false ;
			}
			if ( ztdvrows > 0 )
			{
				tbtop( dslist ) ;
				tbskip( dslist, ( pflscrl == "/" ) ? ntop : top ) ;
			}
			else
			{
				tbbottom( dslist ) ;
				tbskip( dslist, ( 2 - ztddepth ) ) ;
				if ( RC > 0 ) { tbtop( dslist ) ; }
			}
			panel = useList ? ( ppos == 0 ) ? "PFLST0A9" :
					  ( ppos == 1 ) ? "PFLST0AA" :
					  ( ppos == 2 ) ? "PFLST0AB" :
					  ( ppos == 3 ) ? "PFLST0AC" :
					  ( ppos == 4 ) ? "PFLST0AD" : "PFLST0AE" :
					  ( ppos == 0 ) ? "PFLST0A1" :
					  ( ppos == 1 ) ? "PFLST0AF" :
					  ( ppos == 2 ) ? "PFLST0AG" :
					  ( ppos == 3 ) ? "PFLST0AH" :
					  ( ppos == 4 ) ? "PFLST0AI" : "PFLST0AJ" ;
			vreplace( "CONFOFF", "" ) ;
			vreplace( "CONTERR", "" ) ;
			vreplace( "NEMPTOK", "" ) ;
		}
		opath   = zpath    ;
		ohidden = afhidden ;
		ofull   = afsfull  ;
		oexgen  = exgen    ;
		if ( msg != "" && cursor == "" )
		{
			cursor  = "SEL" ;
			msgloc  = "SEL" ;
			csrrow  = crpx  ;
			cur2sel = false ;
		}
		else if ( cur2sel )
		{
			cursor  = "SEL" ;
			msgloc  = "SEL" ;
			csrrow  = crpx  ;
			cur2sel = false ;
			autosel = "NO" ;
		}
		else
		{
			cursor = "ZCMD" ;
		}
		set_fmsgs() ;
		control( "PASSTHRU", "LRSCROLL", "PASON" ) ;
		tbdispl( dslist,
			 panel,
			 msg,
			 cursor,
			 csrrow,
			 1,
			 autosel,
			 "CRP",
			 "",
			 msgloc ) ;
		if ( RC == 8 ) { break ; }
		control( "PASSTHRU", "LRSCROLL", "PASOFF" ) ;
		autosel = "YES" ;
		if ( ztdsels > 0 && panel != "" && pflscrl == "/" )
		{
			ntop = crp ;
		}
		else if ( ztdsels == 0 )
		{
			ntop = ztdtop ;
		}
		cursor = "ZCMD" ;
		panel  = ""  ;
		msg    = ""  ;
		msgloc = ""  ;
		csrrow = 0   ;
		crpx   = crp ;
		crp    = 0   ;
		top    = ztdtop ;
		if ( zcmd == "" && zcurfld == "ZHOTBARD" && pflhbar == "/" )
		{
			process_hotbar() ;
		}
		RCode = actionPrimaryCommand1() ;
		vget( "ZVERB", SHARED ) ;
		if ( zverb == "LEFT" && ppos > 0 )
		{
			--ppos ;
			continue ;
		}
		else if ( zverb == "RIGHT" && ppos < 5 )
		{
			++ppos ;
			continue ;
		}
		if ( RCode == 4 )
		{
			zcmd     = "" ;
			ztdsels  = 0  ;
			ztdvrows = 1  ;
			top      = 0  ;
			continue ;
		}
		else if ( RCode == 8 )
		{
			zcmd = "" ;
			if ( crp > 0 )
			{
				top  = crp ;
				ntop = crp ;
			}
			continue ;
		}
		else if ( RCode == 12 )
		{
			if ( msg == "" ) { msg = "FLST016" ; }
			continue ;
		}
		else if ( RCode == 16 )
		{
			zcmd     = "" ;
			ztdsels  = 0  ;
			ztdvrows = 1  ;
			continue ;
		}
		zcmd = "" ;
		if ( crp > 0 )     { top = crp     ; }
		if ( zpath == "" ) { zpath = zhome ; }
		if ( rebuild2 || ohidden != afhidden || oexgen != exgen || ofull != afsfull )
		{
			affull = ( afsfull == "/" ) ;
			create_filelist1() ;
			rebuild2 = false ;
			ztdvrows = 1 ;
			top      = 0 ;
			continue ;
		}
		if ( !useList && path_change( opath, zpath ) )
		{
			include   = true ;
			recursv   = false ;
			sort_parm = "(ENTRY,C,A)" ;
			clear_search() ;
			clear_filter_i() ;
			clear_filter_x() ;
			create_filelist1() ;
			ztdvrows = 1 ;
			top      = 0 ;
			continue ;
		}
		cursor = "" ;
		if ( ztdvrows == 0 )
		{
			ztdsels = 1 ;
			msg     = "PSYZ006" ;
			continue ;
		}
		if ( ztdsels == 0 && zcurinx != 0 )
		{
			tbtop( dslist ) ;
			tbskip( dslist, zcurinx, "", "", "", "", "CRP" ) ;
			crpx = crp ;
			ntop = crp ;
			entry1 = ( useList || affull ) ? entry : full_name( zpath, entry ) ;
			if ( zcurfld == "ENTRY" || zcurfld == "SEL" )
			{
				if ( is_directory( entry1 ) )
				{
					sel = "L" ;
				}
				else if ( is_regular_file( entry1 ) )
				{
					sel = ( pflsdef == "1" ) ? "E" :
					      ( pflsdef == "2" ) ? "V" : "B" ;
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
		entry1 = ( useList || affull ) ? entry : full_name( zpath, entry ) ;
		if ( sel != "" )
		{
			cur2sel = true ;
			try
			{
				if ( is_symlink( entry1 ) && !exists( entry1 ) )
				{
					if ( sel != "I" && sel != "D" )
					{
						sel     = ""  ;
						msg     = "FLST012L" ;
						message = "Bad symlink" ;
						tbput( dslist ) ;
						continue ;
					}
				}
				else if ( !exists( entry1 ) )
				{
					sel     = "" ;
					msg     = "FLST012L"  ;
					message = "Not found" ;
					tbput( dslist ) ;
					continue ;
				}
			}
			catch ( const filesystem_error& ec )
			{
				sel = "" ;
				rsn = ec.what() ;
				msg = "FLST013M" ;
				message = "Error" ;
				tbput( dslist ) ;
				continue ;
			}
			actionLineCommand( entry1, top, terminat ) ;
			if ( terminat )
			{
				ztdsels = 0 ;
			}
		}
	}

	tbend( dslist ) ;
	if ( useList )
	{
		remove( pssList ) ;
	}

	vdelete( "OEXGEN" ) ;

	vget( "ZVERB", SHARED ) ;
	if ( zverb == "RETURN" )
	{
		control( "NONDISPL", "END" ) ;
		display( "PFLST0AZ" ) ;
	}
}


int pflst0a::setup_parameters()
{
	//
	// Return > 0 causes program to exit.
	//

	size_t p1 ;
	size_t p2 ;

	string w1 ;
	string t1 ;

	control( "ABENDRTN", static_cast<void (pApplication::*)()>(&pflst0a::cleanup_custom) ) ;

	bool unused ;

	if ( PARM == "" )
	{
		vget( "ZPATH", PROFILE ) ;
		if ( zpath == "" )
		{
			zpath = zhome ;
		}
	}
	else
	{
		w1 = word( PARM, 1 ) ;
		if ( w1 == "EXPAND" )
		{
			ZRESULT = expand_dir1( subword( PARM, 2 ) ) ;
			if ( word( PARM, 2 ) == "FO2" && ZRC == 0 )
			{
				ZRESULT.erase( 0, ZRESULT.find_last_of( '/' ) + 1 ) ;
			}
			return 4 ;
		}
		else if ( w1 == "EXPAND1" )
		{
			ZRESULT = expand_field1( subword( PARM, 2 ) ) ;
			return 4 ;
		}
		else if ( w1 == "EXPAND2" )
		{
			ZRESULT = expand_dir2( subword( PARM, 2 ) ) ;
			return 4 ;
		}
		else if ( w1 == "INFO" )
		{
			show_info( subword( PARM, 2 ), unused ) ;
			return 4 ;
		}
		else if ( w1 == "LIST" )
		{
			useList = true ;
			vreplace( "LSTNAME", word( PARM, 2 ) ) ;
			pssList = word( PARM, 3 ) ;
			set_filter_i( word( PARM, 4 ) ) ;
			include  = true  ;
			exgen    = "/"   ;
			initsort = false ;
		}
		else if ( w1 == "DIR" )
		{
			zpath   = full_dir( subword( PARM, 2 ) ) ;
			include = true ;
		}
		else if ( w1 == "DIRR" )
		{
			zpath   = full_dir( subword( PARM, 2 ) ) ;
			include = true ;
			recursv = true ;
			affull  = true ;
		}
		else if ( w1 == "QLIB" )
		{
			zpath   = "@" + subword( PARM, 2 ) ;
			include = true ;
			affull  = true ;
		}
		else
		{
			include = true ;
			iupper( w1 ) ;
			if ( abbrev( "#IMPORT", w1, 2 ) ||
			     abbrev( "#LOCATE", w1, 2 ) ||
			     abbrev( "#FIND", w1, 2 ) )
			{
				zpath = PARM ;
				return 0 ;
			}
			zpath = word( PARM, 1 ) ;
			set_filter_i( word( PARM, 2 ) ) ;
		}
	}

	p1 = zpath.find_last_of( '/' ) ;
	if ( p1 != string::npos && zpath.find( "**" ) == string::npos )
	{
		p2 = zpath.find_first_of( "?*[" ) ;
		if ( p2 != string::npos && p2 > p1 )
		{
			t1 = zpath.substr( p1 + 1 ) ;
			set_filter_i( affull ? "*/" + t1 : t1 ) ;
			zpath.erase( p1 ) ;
			include = true ;
		}
	}

	return 0 ;
}


void pflst0a::cleanup_custom()
{
	//
	// If we are abending, set the path in the profile to the home directory
	// so the same thing does not happen again when invoking the application.
	//

	zpath = zhome ;
	vput( "ZPATH", PROFILE ) ;
}


void pflst0a::setup_hotbar()
{
	int i ;

	string t1 ;
	string t2 ;

	if ( useList ) { return ; }

	pquery( "PFLST0A1", "ZHOTBARD", "", "ZHOTBARW" ) ;
	if ( RC > 0 ) { return ; }

	zhotbard = "" ;
	zhotbars = "" ;
	hotbar.clear() ;
	if ( pflhbar == "/" )
	{
		i        = 0 ;
		zhotbard = "Hotbar:" ;
		zhotbars = "WWWWWWW" ;
		hotbar[ "Hotbar:" ] = "HOTBAR?" ;
		while ( zhotbard.size() <= zhotbarw )
		{
			vget( "PFLENT" + d2ds( i, 2 ), PROFILE ) ;
			if ( RC > 0 ) { break ; }
			vcopy( "PFLENT" + d2ds( i, 2 ), t1, MOVE ) ;
			vget( "PFLCMD" + d2ds( i, 2 ), PROFILE ) ;
			if ( RC > 0 ) { break ; }
			vcopy( "PFLCMD" + d2ds( i, 2 ), t2, MOVE ) ;
			if ( t1 != "" && t2 != "" && ( zhotbard.size() + t1.size() ) < zhotbarw &&
			     t1.front() != '!' && hotbar.find( t1 ) == hotbar.end() )
			{
				hotbar[ t1 ] = t2 ;
				zhotbard += " " + t1 ;
				zhotbars += " " + string( t1.size(), 'Y' ) ;
			}
			++i ;
		}
	}
	zhotbard.resize( zhotbarw, ' ' ) ;
	zhotbars.resize( zhotbarw, 'Y' ) ;
}


void pflst0a::process_hotbar()
{
	size_t p1 = zhotbard.find( ' ', zcurpos - 1 ) ;
	size_t p2 = zhotbard.rfind( ' ', zcurpos - 1 ) ;

	string ent ;

	ent = ( p1 == string::npos ) ?
	      ( p2 == string::npos ) ? zhotbard : zhotbard.substr( p2 + 1 ) :
	      ( p2 == string::npos ) ? zhotbard.substr( 0, p1 ) : zhotbard.substr( p2 + 1, p1 - p2 - 1 ) ;

	zcmd = hotbar[ ent ] ;
}


void pflst0a::actionLineCommand( const string& entry1,
				 int& top,
				 bool& terminat )
{
	int rc ;

	bool execute  = false ;
	bool is_rexx  = false ;
	bool fc_rexx  = false ;
	bool fc_shell = false ;

	string confoff ;
	string conterr ;

	terminat = false ;

	string lp  ;
	string cmd ;
	string t   ;
	string osel ;

	char* buffer ;
	size_t bufferSize = 255 ;

	boost::system::error_code ec ;

	std::ofstream of ;

	if ( sel == "/" )
	{
		vreplace( "PENTRY1", entry1 ) ;
		addpop( "", 2, 5 ) ;
		display( "PFLST0AN", msg ) ;
		if ( RC == 8 )
		{
			rempop() ;
			return ;
		}
		rempop() ;
	}

	auto it = line_cmds.find( sel ) ;

	if ( it == line_cmds.end() || ( sel.size() > 1 && ( sel.front() == '%' || sel.front() == '>' ) ) )
	{
		vcopy( "CONFOFF", confoff, MOVE ) ;
		execute = ( confoff == "/" || pflscmd == "OFF" ) ;
		if ( sel.front() == '%' )
		{
			sel.erase( 0, 1 ) ;
			fc_rexx = true ;
		}
		else if ( sel.front() == '>' )
		{
			sel.erase( 0, 1 ) ;
			fc_shell = true ;
		}
		cmd = sel + " " + entry1 ;
		if ( sel == "." || ( pflscmd == "ON" && !execute ) )
		{
			is_rexx = !fc_shell && ( fc_rexx || isRexx( sel ) ) ;
			if ( is_rexx )
			{
				confirm_cmd( cmd, execute, terminat ) ;
			}
			else
			{
				confirm_shell( cmd, execute, terminat ) ;
			}
			sel = word( cmd, 1 ) ;
		}
		else
		{
			is_rexx = isRexx( sel ) ;
		}
		if ( execute && !terminat )
		{
			if ( is_rexx )
			{
				control( "ERRORS", "RETURN" ) ;
				select( "CMD(%" + cmd + ") LANG(REXX)" ) ;
				if ( RC == 20 )
				{
					message = "Failed" ;
					vcopy( "CONTERR", conterr, MOVE ) ;
					if ( conterr == "" )
					{
						msg = "FLST013N" ;
					}
				}
				else
				{
					message = "Executed" ;
				}
				control( "ERRORS", "CANCEL" ) ;
			}
			else
			{
				if ( timeout == "/" )
				{
					select( "PGM(PCMD0A) PARM("+ cmd +")" ) ;
				}
				else
				{
					select( "PGM(PCMD0A) PARM(--IMMED "+ cmd +")" ) ;
				}
				if ( ZRC == 127 )
				{
					vreplace( "ZVAL1", sel ) ;
					msg     = "PSYS012X" ;
					message = "Command not found" ;
				}
				else
				{
					message = "Executed" ;
				}
				osel = sel ;
			}
		}
		sel = "" ;
		tbput( dslist ) ;
		sel = osel ;
		return ;
	}

	osel = sel ;
	sel  = "" ;

	switch ( it->second )
	{
	case LN_EDIT:
	case LN_VIEW:
	case LN_BROWSE:
	case LN_NANO:
	case LN_VI:
		if ( !is_regular_file( entry1 ) )
		{
			msg     = "FLST012M" ;
			message = "Invalid entry" ;
			tbput( dslist ) ;
			return ;
		}
		update_reflist( entry1 ) ;
		break ;
	}

	switch ( it->second )
	{
	case LN_ADD:
		t = getPFLName() ;
		submit( "PGM("+ get_dialogue_var( "ZRFLPGM" ) +") PARM(PLF " + t + " "+ entry1 +")" ) ;
		message = "Added to "+ t ;
		tbput( dslist ) ;
		break ;

	case LN_EXECUTE1:
		control( "ERRORS", "RETURN" ) ;
		select( "CMD(EXEC " + entry1 + ") LANG(REXX)" ) ;
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

	case LN_EXECUTE2:
		if ( timeout == "/" )
		{
			select( "PGM(PCMD0A) PARM("+ entry1 +")" ) ;
		}
		else
		{
			select( "PGM(PCMD0A) PARM(--IMMED "+ entry1 +")" ) ;
		}
		if ( ZRC == 127 )
		{
			vreplace( "ZVAL1", sel ) ;
			msg     = "PSYS012X" ;
			message = "Command not found" ;
		}
		else
		{
			message = "Executed" ;
		}
		tbput( dslist ) ;
		break ;

	case LN_FS:
		show_FileSystem( entry1, terminat ) ;
		tbput( dslist ) ;
		break ;

	case LN_INFO:
		show_info( entry1, terminat ) ;
		tbput( dslist ) ;
		break ;

	case LN_LINK:
		lp = "" ;
		while ( true )
		{
			buffer = new char[ bufferSize ] ;
			rc     = readlink( entry1.c_str(), buffer, bufferSize ) ;
			if ( rc == -1 )
			{
				delete[] buffer ;
				if ( errno == ENAMETOOLONG )
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
		if ( lp == "" ) { tbput( dslist ) ; break ; }
		listDirectory( lp ) ;
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
	case LN_CCOPY:
		if ( !is_directory( entry1 ) && !is_symlink( entry1 ) && !is_regular_file( entry1 ) )
		{
			msg     = "FLST012A"     ;
			message = "Invalid type" ;
			tbput( dslist ) ;
		}
		else
		{
			rc = copy_entry( entry1, terminat, ( it->second == LN_CCOPY ) ) ;
			if ( rc == 0 && it->second == LN_COPY )
			{
				rebuild1 = true ;
			}
		}
		break ;

	case LN_DELETE:
		delete_entry( entry1, terminat ) ;
		break ;

	case LN_MODIFY:
		modify_entry( entry1, terminat ) ;
		break ;

	case LN_RENAME:
		rename_entry( entry1, terminat ) ;
		break ;

	case LN_EDIT:
		rc = edit_entry( entry1, terminat ) ;
		tbput( dslist ) ;
		if ( rc > 11 )
		{
			sel = osel ;
		}
		break ;

	case LN_VIEW:
		view_entry( entry1, terminat ) ;
		tbput( dslist ) ;
		break ;

	case LN_BROWSE:
		browse_entry( entry1, terminat ) ;
		tbput( dslist ) ;
		break ;

	case LN_LIST:
		list_entry( entry1, terminat ) ;
		break ;

	case LN_LISTX:
		list_entry_recursive( entry1, terminat ) ;
		break ;

	case LN_SUBMIT:
		submit( "CMD(%" + entry1 + ") LANG(REXX)" ) ;
		say( "JOB "+ right( get_dialogue_var( "ZSBTASK" ), 5, '0' ) + " SUBMITTED" ) ;
		message = "Submitted" ;
		tbput( dslist ) ;
		break ;

	case LN_NANO:
		t = "nano " + entry1;
		def_prog_mode()     ;
		endwin()            ;
		std::system( t.c_str() ) ;
		reset_prog_mode()   ;
		refresh()           ;
		message = "nano"    ;
		tbput( dslist )     ;
		break ;

	case LN_VI:
		t = "vi " + entry1 ;
		def_prog_mode()    ;
		endwin()           ;
		std::system( t.c_str() ) ;
		reset_prog_mode()  ;
		refresh()          ;
		message = "vi"     ;
		tbput( dslist )    ;
		break ;

	case LN_EXCLUDE:
		if ( include )
		{
			excludeList.insert( entry ) ;
		}
		else
		{
			excludeList.erase( entry ) ;
		}
		tbdelete( dslist ) ;
		break ;

	case LN_ZIP:
		break ;

	case LN_UNZIP:
		break ;

	case LN_TREE:
	case LN_TTREE:
		RC = 0 ;
		string tname = get_tempname() ;
		of.open( tname ) ;
		recursive_directory_iterator dIt( entry1, ec ) ;
		recursive_directory_iterator eIt ;
		if ( ec.value() != boost::system::errc::success )
		{
			msg     = "FLST012H" ;
			rsn     = ec.message() ;
			message = ec.message() ;
			tbput( dslist ) ;
			of.close() ;
			remove( tname ) ;
			break ;
		}
		of << entry1.substr( entry1.find_last_of( '/' ) + 1 ) << endl ;
		interrupted = false ;
		while ( dIt != eIt )
		{
			if ( interrupted )
			{
				setmsg( "FLST013E" ) ;
				break ;
			}
			path current( *dIt ) ;
			if ( access( current.c_str(), R_OK ) != 0 )
			{
				setmsg( "FLST012X" ) ;
				dIt.disable_recursion_pending() ;
				dIt.increment( ec ) ;
				continue;
			}
			if ( is_regular_file( current ) || is_directory( current ) )
			{
				of << copies( "|   ", dIt.depth()) << "|-- " ;
				of << current.filename().string() << endl ;
				if ( it->second == LN_TREE )
				{
					of << current.string() << endl ;
				}
			}
			dIt.increment( ec ) ;
			if ( ec )
			{
				setmsg( "FLST012X" ) ;
			}
		}
		of.close() ;
		control( "ERRORS", "RETURN" ) ;
		control( "DISPLAY", "SAVE" ) ;
		set_scrname( "BROWSE" ) ;
		( it->second == LN_TREE ) ? browseTree( tname ) : browse( tname ) ;
		control( "DISPLAY", "RESTORE" ) ;
		if ( RC > 0 )
		{
			msg     = "FLST012H"      ;
			message = "Unknown error" ;
			tbput( dslist )           ;
		}
		control( "ERRORS", "CANCEL" ) ;
		remove( tname ) ;
		restore_scrname() ;
		break ;

	}
}


void pflst0a::create_filelist1( bool keepMessages )
{
	bool hide ;
	bool match ;

	size_t p1 ;

	string entry1 ;

	map<string, string> messages ;

	if ( keepMessages )
	{
		control( "ERRORS", "RETURN" ) ;
		tbtop( dslist ) ;
		tbskip( dslist ) ;
		while ( RC == 0 )
		{
			if ( message != "" )
			{
				messages[ entry ] = message ;
			}
			tbskip( dslist ) ;
		}
		control( "ERRORS", "CANCEL" ) ;
	}

	tbcreate( dslist,
		  "",
		  "(MESSAGE,SEL,ENTRY,TYPE,PERMISS,SIZE,OWNER,GROUP,STCDATE,MODDATE,MODDATES,"
		  "STCDATE,STCDATES,ACCDATE,ACCDATES)",
		  NOWRITE,
		  REPLACE ) ;

	vector<path> v ;

	sel     = "" ;
	message = "" ;
	ztdsels = 0  ;
	pvalid  = false ;
	affull  = ( afsfull  == "/" ) ;
	hide    = ( afhidden != "/" ) ;

	load_path_vector( v ) ;

	auto itp = find( pnames.begin(), pnames.end(), zpath ) ;
	if ( itp != pnames.end() )
	{
		pnames.erase( itp ) ;
	}
	pnames.push_front( zpath ) ;
	retPos = 0 ;

	interrupted = false ;
	for ( auto& p : v )
	{
		if ( interrupted )
		{
			msg = "FLST013E" ;
			break ;
		}
		tbvclear( dslist ) ;
		entry = p.string() ;
		p1    = ( recursv ) ? zpath.size() : entry.find_last_of( '/' ) + 1 ;
		entry.erase( 0, p1 ) ;
		entry1 = ( useList || affull ) ? p.string() : entry ;
		if ( !useList && hide && entry.front() == '.' )
		{
			continue ;
		}
		match = match_filter_i( ( useList || affull ) ? p.string() : entry ) ;
		if ( match_filter_x( ( useList || affull ) ? p.string() : entry ) )
		{
			match = false ;
		}
		if ( !excludeList.empty() )
		{
			if ( excludeList.find( entry1 ) != excludeList.end() )
			{
				match = false ;
			}
		}
		if ( (  include && !match ) ||
		     ( !include &&  match ) )
		{
			continue ;
		}
		if ( stats && lstat( p.string().c_str(), &results ) == 0 )
		{
			getFileAttributes() ;
			getFilePermissions() ;
		}
		if ( stats )
		{
			getFileUIDGID() ;
		}
		entry = ( affull ) ? p.string() : entry1 ;
		if ( keepMessages )
		{
			auto it = messages.find( entry ) ;
			if ( it != messages.end() )
			{
				vreplace( "MESSAGE", it->second ) ;
			}
		}
		tbadd( dslist ) ;
	}

	if ( initsort )
	{
		tbsort( dslist, sort_parm ) ;
	}

	tbtop( dslist ) ;
}


void pflst0a::updateFileList1()
{
	bool match ;

	string entry1 ;

	tbtop( dslist ) ;
	tbskip( dslist ) ;

	while ( RC == 0 )
	{
		entry1 = ( useList || affull ) ? entry : full_name( zpath, entry ) ;
		match  = match_filter_i( ( useList || affull ) ? entry1 : entry ) ;
		if ( match_filter_x( ( useList || affull ) ? entry1 : entry ) )
		{
			match = false ;
		}
		if ( !search.empty() )
		{
			if ( searchList.find( entry1 ) == searchList.end() )
			{
				match = false ;
			}
		}
		if ( !excludeList.empty() )
		{
			if ( excludeList.find( entry1 ) != excludeList.end() )
			{
				match = false ;
			}
		}
		if ( (  include && !match ) ||
		     ( !include &&  match ) )
		{
			tbdelete( dslist ) ;
		}
		tbskip( dslist ) ;
	}

	tbtop( dslist ) ;
}


void pflst0a::load_path_vector( vector<path>& v )
{
	int i ;
	int n ;
	int rc ;

	char c ;

	size_t p1 ;
	size_t p2 ;
	size_t n1 ;
	size_t nb ;

	bool add_node   = true  ;
	bool d_asterisk = false ;

	string t ;
	string p ;
	string w1 ;
	string w2 ;
	string str ;
	string cmd ;
	string pat ;
	string mtext ;
	string mpath ;
	string inLine ;
	string tpath = zpath ;

	const string regex_chars = "\\^$.+{[()|" ;

	map<int, regex> nodes_filter ;

	vector<string> results ;

	regex expression ;

	boost::system::error_code ec ;

	path wd ;

	if ( useList )
	{
		std::ifstream fin( pssList.c_str() ) ;
		while ( getline( fin, t ) )
		{
			if ( exgen == "/" )
			{
				p1 = t.find_last_of( '/' ) ;
				if ( p1 != string::npos )
				{
					if ( t.find_first_of( "?*[", p1 ) != string::npos )
					{
						str = t.substr( p1 + 1 ) ;
						add_paths( t.erase( p1 ), str, v ) ;
						continue ;
					}
				}
			}
			try
			{
				if ( exists( t ) )
				{
					v.push_back( t ) ;
				}
			}
			catch (...)
			{
				msg = "FLST012X" ;
			}
		}
		fin.close() ;
		return ;
	}

	if ( tpath.front() == '#' )
	{
		tpath.erase( 0, 1 ) ;
		wd = boost::filesystem::current_path() ;
		w1 = upper( word( tpath, 1 ) ) ;
		w1 = ( abbrev( "IMPORT", w1 ) ) ? "import" :
		     ( abbrev( "LOCATE", w1 ) ) ? "locate" :
		     ( abbrev( "FIND", w1   ) ) ? "find"   : w1 ;
		if ( w1 != "import" && w1 != "locate" && w1 != "find" )
		{
			v.clear() ;
			msg = "FLST012Y" ;
			return ;
		}
		if ( w1 == "import" )
		{
			w2 = subword( tpath, 2 ) ;
			if ( w2 != "" && w2.front() != '/' )
			{
				w2 = wd.string() + "/" + w2 ;
			}
			try
			{
				if ( w2 == "" || !is_regular_file( w2 ) || ( access( w2.c_str(), R_OK ) != 0 ) )
				{
					v.clear() ;
					msg = "FLST011" ;
					return ;
				}
			}
			catch (...)
			{
				v.clear() ;
				msg = "FLST011" ;
				return ;
			}
			std::ifstream fin( w2.c_str() ) ;
			while ( getline( fin, inLine ) )
			{
				if ( inLine.size() > 0 && inLine.front() == '/' )
				{
					v.push_back( inLine ) ;
				}
				else if ( inLine.size() > 1 && inLine.compare( 0, 2, "./" ) == 0 )
				{
					inLine.erase( 0, 1 ) ;
					v.push_back( wd.string() + inLine ) ;
				}
			}
			fin.close() ;
			affull = true ;
			return ;
		}
		cmd = w1 + " " + subword( tpath, 2 ) + " 2>&1" ;
		execute_cmd( rc, cmd, results ) ;
		for ( auto& t : results )
		{
			if ( t.size() > 0 && t.front() == '/' )
			{
				v.push_back( t ) ;
			}
			else if ( t.size() > 1 && t.compare( 0, 2, "./" ) == 0 )
			{
				v.push_back( wd.string() + t.substr( 1 ) ) ;
			}
			else if ( rc != 0 && mtext == "" )
			{
				mtext = "(" + t + ")." ;
			}
		}
		affull = true ;
		if ( rc != 0 )
		{
			vreplace( "ZVAL1", mtext ) ;
			msg = "FLST012Z" ;
		}
		return ;
	}
	else if ( tpath.front() == '@' )
	{
		tpath.erase( 0, 1 ) ;
		iupper( tpath ) ;
		w1 = word( tpath, 1 ) ;
		if ( !isvalidName( w1 ) )
		{
			set_dialogue_var( "ZVAL1", w1 ) ;
			msg = "PSYE031D" ;
			return ;
		}
		vdefine( "MYLIB", &t ) ;
		qbaselib( w1, "MYLIB" ) ;
		for ( n = getpaths( t ), i = 1 ; i <= n ; ++i )
		{
			copy( directory_iterator( getpath( t, i ) ), directory_iterator(), back_inserter( v ) ) ;
		}
		vdelete( "MYLIB" ) ;
		affull = true ;
		return ;
	}

	if ( tpath.front() != '/' )
	{
		vreplace( "ZVAL1", tpath ) ;
		msg = "FLST012R" ;
		return ;
	}

	p1 = tpath.find_first_of( "?*[" ) ;
	if ( p1 != string::npos )
	{
		if ( tpath.back() == '/' ) { tpath.push_back( '*' ) ; }
		affull = true ;
		pat    = "" ;
		for ( uint i = 0 ; i < tpath.size() ; ++i )
		{
			c = tpath[ i ] ;
			if ( c == '*' )
			{
				if ( tpath.compare( i, 2, "**" ) == 0 )
				{
					pat += ".*" ;
					if ( add_node )
					{
						try
						{
							expression.assign( pat ) ;
						}
						catch ( boost::regex_error& e )
						{
							v.clear() ;
							vreplace( "ZZSTR", zpath ) ;
							msg = "PSYS012P" ;
							return ;
						}
						n1 = countc( tpath.substr( 0, i ), '/' ) ;
						nodes_filter[ n1 ] = expression ;
						add_node = false ;
					}
					++i ;
					d_asterisk = true ;
				}
				else
				{
					pat += "[^/]*" ;
				}
			}
			else if ( c == '?' )
			{
				pat += "[^[:blank:]]" ;
			}
			else if ( c == '/' )
			{
				t = tpath.substr( 0, i ) ;
				if ( t != "" && add_node )
				{
					if ( d_asterisk ) { add_node = false ; }
					try
					{
						expression.assign( pat ) ;
					}
					catch ( boost::regex_error& e )
					{
						v.clear() ;
						vreplace( "ZZSTR", t ) ;
						msg = "PSYS012P" ;
						return ;
					}
					nodes_filter[ countc( t, '/' ) ] = expression ;
				}
				pat.push_back( c ) ;
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
		try
		{
			expression.assign( pat ) ;
		}
		catch ( boost::regex_error& e )
		{
			v.clear() ;
			vreplace( "ZZSTR", zpath ) ;
			msg = "PSYS012P" ;
			return ;
		}
		if ( add_node )
		{
			nodes_filter[ countc( tpath, '/' ) ] = expression ;
		}
		p2    = tpath.find_last_of( '/', p1 ) ;
		mpath = ( p2 == 0 ) ? "/" : tpath.substr( 0, p2 ) ;
		nb    = ( mpath == "/" ) ? 1 : countc( mpath, '/' ) + 1 ;
		recursive_directory_iterator dIt( mpath, ec ) ;
		recursive_directory_iterator eIt ;
		interrupted = false ;
		while ( dIt != eIt )
		{
			if ( interrupted )
			{
				msg = "FLST013E" ;
				break ;
			}
			path curr( *dIt ) ;
			p  = curr.string() ;
			n1 = dIt.depth() + nb ;
			if ( !d_asterisk && n1 > nodes_filter.size() )
			{
				dIt.disable_recursion_pending() ;
			}
			else if ( regex_match( p.begin(), p.end(), expression ) )
			{
				v.push_back( p ) ;
				if ( recursv && is_directory( p ) )
				{
					recursive_directory_iterator drIt( p, ec ) ;
					recursive_directory_iterator erIt ;
					interrupted = false ;
					while ( drIt != erIt )
					{
						if ( interrupted )
						{
							msg = "FLST013E" ;
							break ;
						}
						path curr( *drIt ) ;
						v.push_back( curr.string() ) ;
						drIt.increment( ec ) ;
						if ( ec )
						{
							log_filesystem_error( ec, curr.string() ) ;
							msg = "FLST012X" ;
						}
					}
					dIt.disable_recursion_pending() ;
					if ( interrupted )
					{
						break ;
					}
				}
			}
			else if ( n1 <= nodes_filter.size() )
			{
				if ( !regex_match( p.begin(), p.end(), nodes_filter[ n1 ] ) )
				{
					dIt.disable_recursion_pending() ;
				}
			}
			else if ( access( p.c_str(), R_OK ) != 0 )
			{
				log_filesystem_error( p ) ;
				msg = "FLST012X" ;
				dIt.disable_recursion_pending() ;
			}
			dIt.increment( ec ) ;
			if ( ec )
			{
				log_filesystem_error( ec, curr.string() ) ;
				msg = "FLST012X" ;
			}
		}
		return ;
	}

	pvalid = true ;
	try
	{
		if ( !is_directory( zpath ) )
		{
			vreplace( "ZVAL1", zpath ) ;
			msg = "FLST012R" ;
			return ;
		}
		zpath = full_dir( zpath ) ;
		if ( recursv )
		{
			recursive_directory_iterator dIt( zpath, ec ) ;
			recursive_directory_iterator eIt ;
			interrupted = false ;
			while ( dIt != eIt )
			{
				if ( interrupted )
				{
					msg = "FLST013E" ;
					break ;
				}
				path curr( *dIt ) ;
				v.push_back( curr.string() ) ;
				dIt.increment( ec ) ;
				if ( ec )
				{
					log_filesystem_error( ec, curr.string() ) ;
					msg = "FLST012X" ;
				}
			}
		}
		else
		{
			copy( directory_iterator( zpath ), directory_iterator(), back_inserter( v ) ) ;
		}
	}
	catch ( const filesystem_error& ex )
	{
		log_filesystem_error( ex ) ;
		msg = "FLST012X" ;
	}
}


void pflst0a::add_paths( const string& pth,
			 const string& gen,
			 vector<path>& entries )
{
	//
	// Add paths to path vector entries, from path pth using generic gen. Ignore duplicates.
	//

	string ent ;

	vector<path> vt ;

	regex expression ;

	try
	{
		copy( directory_iterator( pth ), directory_iterator(), back_inserter( vt ) ) ;
	}
	catch ( const filesystem_error& ex )
	{
	}

	sort( vt.begin(), vt.end() ) ;

	try
	{
		expression.assign( conv_regex( upper( gen ), '?', '*' ), boost::regex_constants::icase ) ;
	}
	catch ( boost::regex_error& e )
	{
		return ;
	}

	for ( auto& p : vt )
	{
		ent = substr( p.string(), lastpos( "/", ent ) + 1 ) ;
		if ( regex_match( ent.begin(), ent.end(), expression ) )
		{
			if ( find( entries.begin(), entries.end(), p.string() ) == entries.end() )
			{
				entries.push_back( p.string() ) ;
			}
		}
	}
}


void pflst0a::createSearchList()
{
	//
	// Create a list of files that contain string 'search'.
	//

	int rc ;

	string cmd ;

	bool empty = true ;

	string tname = get_tempname() ;

	vector<string> results ;

	std::ofstream of ;

	searchList.clear() ;

	tbtop( dslist ) ;
	tbskip( dslist ) ;

	of.open( tname ) ;

	while ( RC == 0 )
	{
		if ( !useList && !affull )
		{
			entry = full_name( zpath, entry ) ;
		}
		try
		{
			if ( !is_regular_file( entry ) )
			{
				tbskip( dslist ) ;
				continue ;
			}
		}
		catch ( const filesystem_error& ex )
		{
			tbskip( dslist ) ;
			continue ;
		}
		of << " \"" + entry + "\"" << endl ;
		tbskip( dslist ) ;
		empty = false ;
	}

	of.close() ;

	if ( empty )
	{
		tbtop( dslist ) ;
		tbskip( dslist, ztdtop ) ;
		return ;
	}

	cmd = "xargs grep -i -l -s -F -e \"" + search[ 0 ] + "\" < " + tname + " 2>&1" ;
	execute_cmd( rc, cmd, results ) ;
	load_searchList( results ) ;

	for ( uint i = 1 ; i < search.size() ; ++i )
	{
		empty = true ;
		of.open( tname ) ;
		for ( auto it = searchList.begin() ; it != searchList.end() ; ++it )
		{
			of << " \"" << *it << "\"" << endl ;
			empty = false ;
		}
		of.close() ;
		if ( empty ) { break ; }
		cmd = "xargs grep -i -l -s -F -e \"" + search[ i ] + "\" < " + tname + " 2>&1" ;
		execute_cmd( rc, cmd, results ) ;
		load_searchList( results ) ;
	}

	remove( tname ) ;

	tbtop( dslist ) ;
	tbskip( dslist, ztdtop ) ;
}


void pflst0a::execute_cmd( int& rc,
			   const string& cmd,
			   vector<string>& results )
{
	string line ;

	char buffer[ 8192 ] ;

	results.clear() ;

	FILE* pipe { popen( cmd.c_str(), "r" ) } ;

	if ( !pipe )
	{
		llog( "E", "POPEN failed.  Command string size="<< cmd.size() << endl ) ;
		setmsg( "FLST013I" ) ;
		return ;
	}

	while ( fgets( buffer, sizeof( buffer ), pipe ) )
	{
		line = buffer ;
		if ( line != "" )
		{
			if ( line.back() == 0x0a )
			{
				line.pop_back() ;
			}
			results.push_back( line ) ;
		}
	}

	rc = WEXITSTATUS( pclose( pipe ) ) ;
}


void pflst0a::load_searchList( vector<string>& results )
{
	searchList.clear() ;

	for ( auto& r : results )
	{
		searchList.insert( r ) ;
	}

	results.clear() ;
}


int pflst0a::actionPrimaryCommand1()
{
	if ( zcmd == "" ) { return 0 ; }

	auto it = commandList1.find( get_truename( upper( word( zcmd, 1 ) ) ) ) ;
	if ( it != commandList1.end() )
	{
		return (this->*(it->second))() ;
	}

	msg = "PSYS018" ;

	return 12 ;
}


int pflst0a::action_Edit()
{
	int RCode = 0 ;

	string ws ;
	string p  ;

	bool terminat ;

	ws = subword( zcmd, 2 ) ;
	if ( ws == "" )
	{
		vreplace( "ZVAL1", "EDIT" ) ;
		return 12 ;
	}

	if ( !pvalid )
	{
		msg = "FLST013A" ;
		return 12 ;
	}

	p = full_name( zpath, ws ) ;

	if ( is_directory( p ) || is_regular_file( p ) || !exists( p ) )
	{
		edit_entry( p, terminat ) ;
	}

	return RCode ;
}


int pflst0a::action_Browse()
{
	string ws ;
	string p  ;

	ws = subword( zcmd, 2 ) ;
	if ( ws == "" )
	{
		vreplace( "ZVAL1", "BROWSE" ) ;
		return 12 ;
	}

	if ( !pvalid )
	{
		msg = "FLST013A" ;
		return 12 ;
	}

	p = full_name( zpath, ws ) ;

	if ( exists( p ) && ( is_regular_file( p ) || is_directory( p ) ) )
	{
		set_scrname( "BROWSE" ) ;
		control( "ERRORS", "RETURN" ) ;
		browse( p ) ;
		if ( isvalidName( ZRESULT ) )
		{
			msg = ZRESULT ;
		}
		control( "ERRORS", "CANCEL" ) ;
		restore_scrname() ;
		zcmd = "" ;
	}
	else
	{
		msg = "FLST012L" ;
		return 12 ;
	}

	return 0 ;
}


int pflst0a::action_View()
{
	string ws ;
	string p  ;

	bool terminat ;

	ws = subword( zcmd, 2 ) ;
	if ( ws == "" )
	{
		vreplace( "ZVAL1", "VIEW" ) ;
		return 12 ;
	}

	if ( !pvalid )
	{
		msg = "FLST013A" ;
		return 12 ;
	}

	p = full_name( zpath, ws ) ;

	if ( exists( p ) && ( is_directory( p ) || is_regular_file( p ) ) )
	{
		view_entry( p, terminat ) ;
		message = "" ;
		zcmd    = "" ;
	}
	else
	{
		msg = "FLST012L" ;
		return 12 ;
	}

	return 0 ;
}


int pflst0a::action_Setpath()
{
	zpath = subword( zcmd, 2 ) ;

	return 0 ;
}


int pflst0a::action_Sort()
{
	string numchar ;

	string w2 = upper( word( zcmd, 2 ) ) ;
	string w3 = upper( word( zcmd, 3 ) ) ;

	const string desc = "MESSAGE MODDATES STCDATES ACCDATES SIZE" ;
	const string numb = "MODDATES STCDATES ACCDATES SIZE" ;

	w2 = ( w2 == ""                      ) ? "ENTRY"    :
	     ( abbrev( "ENTRY", w2, 2 )      ) ? "ENTRY"    :
	     ( abbrev( "TYPE", w2, 2 )       ) ? "TYPE"     :
	     ( abbrev( "PERMISSION", w2, 2 ) ) ? "PERMISS"  :
	     ( abbrev( "OWNER", w2, 2 )      ) ? "OWNER"    :
	     ( abbrev( "GROUP", w2, 2 )      ) ? "GROUP"    :
	     ( abbrev( "SIZE", w2, 2 )       ) ? "SIZE"     :
	     ( abbrev( "MODIFIED", w2, 2 )   ) ? "MODDATES" :
	     ( abbrev( "CHANGED", w2, 2 )    ) ? "STCDATES" :
	     ( abbrev( "ACCESSED", w2, 2 )   ) ? "ACCDATES" :
	     ( abbrev( "MESSAGE", w2, 2 )    ) ? "MESSAGE"  :
	     ( w2 == "MSG" || w2 == "MSGS"   ) ? "MESSAGE"  : "" ;

	if ( w2 == "" )
	{
		vreplace( "ZVAL1", "SORT" ) ;
		return 12 ;
	}

	if ( w3 == "" )
	{
		w3 = ( findword( w2, desc ) ) ? "D" : "A" ;
	}


	if ( ( w3 != "A" && w3 != "D" ) || words( zcmd ) > 3 )
	{
		vreplace( "ZVAL1", "SORT" ) ;
		return 12 ;
	}

	numchar = ( findword( w2, numb ) ) ? "N" : "C" ;

	initsort  = true ;
	sort_parm = "(" + w2 + "," + numchar + "," + w3 + ")" ;

	tbsort( dslist, sort_parm ) ;

	return 4 ;
}


int pflst0a::action_Stats()
{
	stats = !stats ;

	if ( words( zcmd ) > 1 )
	{
		vreplace( "ZVAL1", "STATS" ) ;
		return 12 ;
	}

	put_profile_var( "PFLSTTS", ( stats ) ? "ON" : "OFF" ) ;

	action_Refresh() ;

	return 8 ;
}


int pflst0a::action_Touch()
{
	bool found ;

	string ws = subword( zcmd, 2 ) ;

	if ( words( zcmd ) == 1 )
	{
		vreplace( "ZVAL1", "TOUCH" ) ;
		return 12 ;
	}

	if ( ws.front() != '/' )
	{
		if ( !pvalid )
		{
			msg  = "FLST013A" ;
			return 12 ;
		}
		ws = zpath + "/" + ws ;
	}

	try
	{
		found = exists( ws ) ;
	}
	catch ( const filesystem_error& ex )
	{
		msg  = "FLST012F" ;
		zcmd = "" ;
		llog( "E", "Error accessing file." << endl ) ;
		llog( "E", ex.what() << endl ) ;
		return 12  ;
	}

	if ( !found )
	{
		try
		{
			std::ofstream of( ws.c_str() ) ;
			of.close() ;
			create_filelist1( true ) ;
			msg = "FLST012E" ;
		}
		catch ( const filesystem_error& ex )
		{
			msg = "FLST012F" ;
			llog( "E", "Error creating file." << endl ) ;
			llog( "E", ex.what() << endl ) ;
		}
	}
	else
	{
		msg = "FLST012D" ;
		return 12 ;
	}

	ztdsels = 0 ;

	return 8 ;
}


int pflst0a::action_Settings()
{
	addpop( "", 2, 5 ) ;

	while ( true )
	{
		display( "PFLST0AS" ) ;
		if ( RC > 0 ) { break ; }
	}

	setup_hotbar() ;

	rempop() ;

	return 8 ;
}


int pflst0a::action_Locate()
{
	int ws = words( zcmd ) ;

	string w3 = upper( word( zcmd, 3 ) ) ;

	if ( ws < 2 || ws > 3 )
	{
		vreplace( "ZVAL1", "LOCATE" ) ;
		return 12 ;
	}

	tbvclear( dslist ) ;

	entry = word( zcmd, 2 ) ;

	if ( entry.back() != '*' )
	{
		entry.push_back( '*' ) ;
	}

	if ( w3 == "PREV" )
	{
		tbsarg( dslist, "", "PREVIOUS", "(ENTRY,LE)" ) ;
	}
	else if ( w3 == "FIRST" )
	{
		tbsarg( dslist, "", "NEXT", "(ENTRY,GE)" ) ;
		tbtop( dslist ) ;
	}
	else if ( w3 == "LAST" )
	{
		tbsarg( dslist, "", "PREVIOUS", "(ENTRY,LE)" ) ;
		tbtop( dslist ) ;
	}
	else if ( w3 == "" || w3 == "NEXT" )
	{
		tbsarg( dslist, "", "NEXT", "(ENTRY,GE)" ) ;
	}
	else
	{
		vreplace( "ZVAL1", "LOCATE" ) ;
		return 12 ;
	}

	tbscan( dslist, "", "", "", "", "", "CRP" ) ;
	ztdsels = 0 ;

	return 8 ;
}


int pflst0a::action_Logerrs()
{
	log_error = ( log_error == "ON" ) ? "OFF" : "ON" ;

	return 8 ;
}


int pflst0a::action_Previous()
{
	if ( !pnames.empty() )
	{
		if ( ++retPos >= pnames.size() ) { retPos = 0 ; }
		clear_search() ;
		clear_filter_i() ;
		clear_filter_x() ;
		zpath    = pnames[ retPos ] ;
		rebuild2 = true ;
	}

	return 8 ;
}


int pflst0a::action_History()
{
	select( "PGM(" + get_dialogue_var( "ZFHSTPGM" ) + ") PARM(PLD PFLST0A1 0 0 5 17 ZPATH)" ) ;

	if ( ZRC == 0 )
	{
		zpath = ZRESULT ;
		clear_search() ;
		clear_filter_i() ;
		clear_filter_x() ;
		rebuild2 = true ;
		msg      = "FLST013B" ;
	}

	return 4 ;
}


int pflst0a::action_Back()
{
	if ( words( zcmd ) > 1 )
	{
		vreplace( "ZVAL1", "BACK" ) ;
		return 12 ;
	}

	if ( !pvalid )
	{
		msg = "FLST013A" ;
		return 12 ;
	}

	if ( zpath == "/" ) { return 0 ; }

	if ( zpath.back() == '/' )
	{
		zpath.pop_back() ;
	}

	zpath.erase( zpath.find_last_of( '/' ) + 1 ) ;

	return 0 ;
}


int pflst0a::action_Chngdir()
{
	string t  = zpath ;
	string ws = subword( zcmd, 2 ) ;

	if ( words( zcmd ) == 1 )
	{
		zpath = zhome ;
		return 0 ;
	}

	if ( ws.front() != '/' )
	{
		if ( !pvalid )
		{
			msg = "FLST013A" ;
			return 12 ;
		}
		t = full_name( t, ws ) ;
	}
	else
	{
		t = ws ;
	}

	if ( t.find_first_of( "?*[" ) != string::npos )
	{
		t = expand_name( t ) ;
	}

	if ( t == "" )
	{
		vreplace( "ZVAL1", "CD" ) ;
		return 12 ;
	}

	try
	{
		if ( !is_directory( t ) )
		{
			msg = "FLST012R" ;
			vreplace( "ZVAL1", t ) ;
			return 12 ;
		}
	}
	catch ( const filesystem_error& ex )
	{
		msg = "FLST014" ;
		llog( "E", "Error accessing directory." << endl ) ;
		llog( "E", ex.what() << endl ) ;
		return 12 ;
	}

	zpath = t ;

	return 0 ;
}


int pflst0a::action_Makedir()
{
	string ws = subword( zcmd, 2 ) ;

	boost::system::error_code ec ;

	if ( ws == "" )
	{
		vreplace( "ZVAL1", "MKDIR" ) ;
		return 12 ;
	}

	if ( ws.front() != '/' )
	{
		if ( !pvalid )
		{
			msg = "FLST013A" ;
			return 12 ;
		}
		ws = zpath + "/" + ws ;
	}

	create_directory( ws, ec ) ;
	if ( ec.value() == boost::system::errc::success )
	{
		msg = "FLST012B" ;
		create_filelist1( true ) ;
	}
	else
	{
		msg = "FLST012C"   ;
		rsn = ec.message() ;
		llog( "E", "Create of directory " + ws + " failed." << endl ) ;
		llog( "E", rsn << endl ) ;
		return 12 ;
	}

	return 8 ;
}


int pflst0a::action_Hotbar()
{
	pflhbar = ( pflhbar == "/" ) ? "" : "/" ;
	vput( "PFLHBAR", PROFILE ) ;

	setup_hotbar() ;

	return 4 ;
}


int pflst0a::action_Setupbar()
{
	addpop( "", 2, 5 ) ;
	while ( RC == 0 )
	{
		display( "PFLST0AP" ) ;
	}
	rempop() ;

	setup_hotbar() ;

	return 4 ;
}


int pflst0a::action_Include()
{
	string w2 = word( zcmd, 2 ) ;
	string w3 = word( zcmd, 3 ) ;

	if ( w2 == "" || w3 != "" )
	{
		vreplace( "ZVAL1", "I" ) ;
		return 12 ;
	}

	set_filter_i( w2 ) ;
	updateFileList1() ;

	return 4 ;
}


int pflst0a::action_Only()
{
	string w2 = word( zcmd, 2 ) ;
	string w3 = word( zcmd, 3 ) ;

	if ( w2 == "" || w3 != "" )
	{
		vreplace( "ZVAL1", "O" ) ;
		return 12 ;
	}

	clear_filter_i() ;
	clear_filter_x() ;
	clear_search() ;

	include = true ;

	set_filter_i( w2 ) ;
	create_filelist1() ;

	return 4 ;
}


int pflst0a::action_Print()
{
	int i ;

	size_t entrySize = 0 ;

	string t      ;
	string fname  ;
	string ztime  ;
	string zdatestd ;

	std::ofstream of ;

	vget( "ZDATESTD ZTIME", SHARED ) ;

	vcopy( "ZDATESTD", zdatestd ) ;
	vcopy( "ZTIME", ztime ) ;

	if ( zuprof.back() != '/' ) { zuprof += "/" ; }

	for ( i = 99 ; i >= 1 ; --i )
	{
		fname = zuprof + "spf" + d2ds( i ) + ".list" ;
		if ( exists( fname ) )
		{
			fname = zuprof + "spf" + d2ds( ++i ) + ".list" ;
			break ;
		}
	}

	if ( i > 99 )
	{
		msg = "FLST012T" ;
		return 12 ;
	}

	tbtop( dslist ) ;
	tbskip( dslist ) ;
	while ( RC == 0 )
	{
		entrySize = max( entrySize, entry.size() ) ;
		tbskip( dslist ) ;
	}

	if ( useList )
	{
		t = "Listing of referrence list "+ word( PARM, 2 ) ;
	}
	else
	{
		t = "Listing of files in directory "+ zpath ;
	}

	of.open( fname ) ;
	of << setw( entrySize + 45 ) << std::left << t  << "Date: " << zdatestd << endl ;
	of << setw( entrySize + 45 ) << std::left << "" << "Time: " << ztime  << endl ;
	of << endl ;
	of << endl ;

	of << setw( entrySize + 10 )
	   << std::left
	   << "Entry"
	   << "Type    "
	   << "Permission   "
	   << "Size       "
	   << "Modified"
	   << endl ;

	of << string( entrySize + 61, '-' ) << endl ;

	tbtop( dslist ) ;
	tbskip( dslist ) ;
	while ( RC == 0 )
	{
		of << setw( entrySize + 10 )
		   << std::left
		   << entry
		   << setw( 8 )
		   << std::left
		   << type
		   << setw( 13 )
		   << std::left
		   << permiss
		   << setw( 11 )
		   << std::left
		   << size
		   << moddate
		   << endl ;
		tbskip( dslist ) ;
	}

	of.close() ;

	vreplace( "ZVAL1", fname ) ;
	msg = "FLST012S" ;

	tbtop( dslist ) ;
	tbskip( dslist, ztdtop ) ;

	return 8 ;
}


int pflst0a::action_Expand()
{
	if ( words( zcmd ) > 1 )
	{
		vreplace( "ZVAL1", "RECURSIVE" ) ;
		return 12 ;
	}

	crp     = ztdtop ;
	recursv = !recursv ;

	if ( search.size() > 0 )
	{
		searchList.clear() ;
		create_filelist1() ;
		createSearchList() ;
		updateFileList1() ;
	}
	else
	{
		create_filelist1( true ) ;
	}

	return 8 ;
}


int pflst0a::action_Refresh()
{
	if ( words( zcmd ) > 1 )
	{
		vreplace( "ZVAL1", "REFRESH" ) ;
		return 12 ;
	}

	crp = ztdtop ;

	if ( search.size() > 0 )
	{
		searchList.clear() ;
		create_filelist1() ;
		createSearchList() ;
		updateFileList1() ;
	}
	else
	{
		create_filelist1( true ) ;
	}

	return 8 ;
}


int pflst0a::action_Reset()
{
	int ws = words( zcmd ) ;

	string t ;

	string w2 = upper( word( zcmd, 2 ) ) ;
	string w3 = upper( word( zcmd, 3 ) ) ;

	const string valid = "/X CC INCLUDE EXCLUDE SEARCH" ;

	vector<string> temp ;

	w2 = ( w2 == "I"                  ) ? "INCLUDE" :
	     ( abbrev( "INCLUDE", w2, 2 ) ) ? "INCLUDE" :
	     ( abbrev( "EXCLUDE", w2, 2 ) ) ? "EXCLUDE" :
	     ( w2 == "X"                  ) ? "EXCLUDE" :
	     ( abbrev( "SEARCH",  w2, 2 ) ) ? "SEARCH"  :
	     ( abbrev( "SRCHFOR", w2, 2 ) ) ? "SEARCH"  : w2 ;

	w3 = ( abbrev( "LAST", w3, 2 ) ) ? "LAST" : w3 ;

	if ( ws > 3 ||
	    ( ws  > 1 && !findword( w2, valid ) ) ||
	    ( ws == 3 && ( ( w2 != "INCLUDE" && w2 != "EXCLUDE" && w2 != "SEARCH" ) || w3 != "LAST" ) ) )
	{
		vreplace( "ZVAL1", "RESET" ) ;
		return 12 ;
	}

	if ( w2 == "/X" )
	{
		excludeList.clear() ;
		create_filelist1() ;
		return 4 ;
	}
	else if ( w2 == "CC" )
	{
		ccpath = "" ;
		return 4 ;
	}
	else if ( w2 == "INCLUDE" )
	{
		if ( filter_i.size() > 0 && w3 == "LAST" )
		{
			t = filter_i.back() ;
			auto it = filter_i_regex.find( t ) ;
			if ( it != filter_i_regex.end() )
			{
				filter_i_regex.erase( it ) ;
			}
			filter_i.pop_back() ;
			create_filelist1() ;
		}
		else if ( !filter_i.empty() )
		{
			clear_filter_i() ;
			create_filelist1() ;
		}
		return 4 ;
	}
	else if ( w2 == "EXCLUDE" )
	{
		if ( filter_x.size() > 0 && w3 == "LAST" )
		{
			t = filter_x.back() ;
			auto it = filter_x_regex.find( t ) ;
			if ( it != filter_x_regex.end() )
			{
				filter_x_regex.erase( it ) ;
			}
			filter_x.pop_back() ;
			create_filelist1() ;
		}
		else if ( !filter_x.empty() )
		{
			clear_filter_x() ;
			create_filelist1() ;
		}
		return 4 ;
	}
	else if ( w2 == "SEARCH" )
	{
		if ( !search.empty() && w3 == "LAST" )
		{
			search.pop_back() ;
			if ( !search.empty() )
			{
				temp = search ;
				clear_search() ;
				searchList.clear() ;
				create_filelist1() ;
				search = temp ;
				createSearchList() ;
				updateFileList1() ;
			}
			else
			{
				create_filelist1() ;
			}
		}
		else if ( !search.empty() )
		{
			clear_search() ;
			searchList.clear() ;
			create_filelist1() ;
		}
		return 4 ;
	}

	excludeList.clear() ;
	clear_filter_i() ;
	clear_filter_x() ;
	clear_search() ;
	ccpath    = "" ;
	ztdvrows  = 1  ;
	include   = true ;
	sort_parm = "(ENTRY,C,A)" ;

	create_filelist1() ;

	return 4 ;
}


int pflst0a::action_Search()
{
	//
	// Filter file list.
	//
	// Search string must be enclosed with quotes (single or double) if it
	// contains more than one word to be searched as a phrase.  Embeded
	// double quotes are not allowed.  Multiple non-quoted strings may also
	// be entered.
	//

	int i ;
	int j ;
	int rc ;

	string str = qstring( rc, subword( zcmd, 2 ) ) ;

	if ( rc > 4 || str == "" || str.find( '"' ) != string::npos )
	{
		vreplace( "ZVAL1", "SEARCH" ) ;
		return 12 ;
	}

	if ( rc == 0 )
	{
		set_search( str ) ;
	}
	else
	{
		if ( str.find( '"' ) != string::npos || str.find( '\'' ) != string::npos )
		{
			vreplace( "ZVAL1", "SEARCH" ) ;
			return 12 ;
		}
		for ( i = 1, j = words( str ) ; i <= j ; ++i )
		{
			set_search( word( str, i ) ) ;
		}
	}

	createSearchList() ;

	updateFileList1() ;

	return 4 ;
}


int pflst0a::action_Showcmd()
{
	string w2 = upper( word( zcmd, 2 ) ) ;

	if ( w2 == "ON" )
	{
		pflscmd = "ON" ;
	}
	else if ( w2 == "OFF" )
	{
		pflscmd = "OFF" ;
	}
	else if ( w2 == "" )
	{
		pflscmd = ( pflscmd == "ON" ) ? "OFF" : "ON" ;
	}
	else
	{
		vreplace( "ZVAL1", "SHOWCMD" ) ;
		return 12 ;
	}

	vput( "PFLSCMD", PROFILE ) ;

	return 4 ;
}


int pflst0a::action_Exclude()
{
	string w2 = word( zcmd, 2 ) ;
	string w3 = word( zcmd, 3 ) ;

	if ( w2 == "" || w3 != "" )
	{
		vreplace( "ZVAL1", "X" ) ;
		return 12 ;
	}

	set_filter_x( w2 ) ;
	updateFileList1() ;

	return 4 ;
}


int pflst0a::action_Zip()
{
	string w2 = word( zcmd, 2 ) ;
	string w3 = word( zcmd, 3 ) ;

	if ( w2 == "" || w3 != "" )
	{
		vreplace( "ZVAL1", "X" ) ;
		return 12 ;
	}

	return 4 ;
}


int pflst0a::action_Flip()
{
	string w2 = word( zcmd, 2 ) ;

	if ( w2 != "" )
	{
		vreplace( "ZVAL1", "X" ) ;
		return 12 ;
	}

	include = !include ;
	create_filelist1() ;

	return 4 ;
}


int pflst0a::action_Find()
{
	int ttop = ztdtop ;

	bool terminat ;
	bool first = false ;

	string w2 = upper( word( zcmd, 2 ) ) ;
	string w3 = upper( word( zcmd, 3 ) ) ;
	string w4 = upper( word( zcmd, 4 ) ) ;

	string entry1 ;

	w4 = ( abbrev( "FIRST", w4, 2 ) ) ? "FIRST" : w4 ;
	if ( w4 == "FIRST" )
	{
		idelword( zcmd, 4, 1 ) ;
		first = true ;
	}

	w3 = ( abbrev( "FIRST", w3, 2 ) ) ? "FIRST" : w3 ;
	if ( w3 == "FIRST" )
	{
		first = true ;
		w3 = upper( word( zcmd, 4 ) ) ;
		w4 = word( zcmd, 5 ) ;
	}
	else
	{
		w4 = word( zcmd, 4 ) ;
	}

	if ( ( w2 == "" && findstr == "" ) || ( w3 != "" && w3 != "E" && w3 != "B" ) || w4 != "" )
	{
		vreplace( "ZVAL1", "FIND" ) ;
		return 12 ;
	}

	if ( w2 != "" )
	{
		findstr = w2 ;
	}

	tbtop( dslist ) ;
	if ( first || ztdtop == 1 )
	{
		top = 0 ;
	}
	else
	{
		top = ztdtop ;
		tbskip( dslist, top ) ;
	}

	for ( uint i = 0 ; ( ztdtop == 1 ) ? ( i < 1 ) : ( i < 2 ) ; ++i )
	{
		tbskip( dslist ) ;
		while ( RC == 0 )
		{
			++top ;
			if ( upper( entry ).find( findstr ) != string::npos )
			{
				entry1 = ( useList || affull ) ? entry : full_name( zpath, entry ) ;
				if ( w3 == "E" )
				{
					edit_entry( entry1, terminat ) ;
				}
				else if ( w3 == "B" )
				{
					browse_entry( entry1, terminat ) ;
				}
				update_reflist( entry1 ) ;
				ntop = top ;
				return 16 ;
			}
			tbskip( dslist ) ;
		}
		top = 0 ;
		tbtop( dslist ) ;
	}

	top  = ttop ;
	ntop = ttop ;
	vreplace( "STR", findstr ) ;
	setmsg( "FLST013L" ) ;

	return 16 ;
}


int pflst0a::action_Colours()
{
	addpop( "", 2, 5 ) ;
	while ( RC == 0 )
	{
		display( "PFLST0AL", msg ) ;
	}
	rempop() ;

	return 4 ;
}


int pflst0a::action_Addpfl()
{
	addpop( "", 2, 5 ) ;
	while ( RC == 0 )
	{
		display( "PFLST0AM", msg ) ;
	}
	rempop() ;

	return 4 ;
}


int pflst0a::action_Block()
{
	int rc ;

	size_t wpos ;

	bool execute  = false ;
	bool terminat = false ;
	bool is_rexx  = false ;
	bool fc_rexx  = false ;
	bool fc_shell = false ;

	string cmd ;
	string xcmd ;
	string entry1 ;
	string confoff ;
	string conterr ;
	string w2 = word( zcmd, 2 ) ;
	string u2 = upper( w2 ) ;

	xcmd = subword( zcmd, 2 ) ;

	if ( w2 == "" )
	{
		vreplace( "ZVAL1", "*" ) ;
		return 12 ;
	}

	if ( xcmd.front() == '%' )
	{
		xcmd.erase( 0, 1 ) ;
		fc_rexx = true ;
	}
	else if ( xcmd.front() == '>' )
	{
		xcmd.erase( 0, 1 ) ;
		fc_shell = true ;
	}

	is_rexx = !fc_shell && ( fc_rexx || isRexx( w2 ) ) ;

	auto it = commandList3.find( u2 ) ;

	vreplace( "CONTERR", "" ) ;

	tbtop( dslist ) ;
	tbskip( dslist ) ;
	while ( RC == 0 )
	{
		entry1 = ( useList || affull ) ? entry : full_name( zpath, entry ) ;
		try
		{
			if ( is_symlink( entry1 ) && !exists( entry1 ) )
			{
				if ( u2 != "INFO" && u2 != "DELETE" )
				{
					tbskip( dslist ) ;
					continue ;
				}
			}
			else if ( !exists( entry1 ) )
			{
				tbskip( dslist ) ;
				continue ;
			}
		}
		catch (...)
		{
			tbskip( dslist ) ;
			continue ;
		}
		if ( it != commandList3.end() )
		{
			rc = (this->*(it->second))( entry1 ) ;
			if ( rc == 4 )
			{
				setmsg( "FLST012V" ) ;
				message = "Terminated" ;
				tbput( dslist ) ;
				break ;
			}
			else if ( rc > 4 )
			{
				break ;
			}
			tbskip( dslist ) ;
			continue ;
		}
		wpos = xcmd.find( '/' ) ;
		if ( wpos == string::npos )
		{
			cmd = xcmd + " " + entry1 ;
		}
		else
		{
			cmd = xcmd ;
			cmd.replace( wpos, 1, entry1 ) ;
		}
		vcopy( "CONFOFF", confoff, MOVE ) ;
		execute = ( confoff == "/" ) ;
		if ( !execute )
		{
			if ( is_rexx )
			{
				confirm_cmd( cmd, execute, terminat ) ;
				vcopy( "CONTERR", conterr, MOVE ) ;
			}
			else
			{
				confirm_shell( cmd, execute, terminat ) ;
			}
			if ( terminat )
			{
				break ;
			}
		}
		if ( execute )
		{
			if ( is_rexx )
			{
				control( "ERRORS", "RETURN" ) ;
				select( "CMD(%"+ cmd +") LANG(REXX)" ) ;
				if ( RC == 20 )
				{
					message  = "Failed" ;
					control( "ERRORS", "CANCEL" ) ;
					tbput( dslist ) ;
					if ( conterr == "" )
					{
						break ;
					}
				}
				control( "ERRORS", "CANCEL" ) ;
				message = "Executed" ;
			}
			else
			{
				if ( timeout == "/" )
				{
					select( "PGM(PCMD0A) PARM("+ cmd +")" ) ;
				}
				else
				{
					select( "PGM(PCMD0A) PARM(--IMMED "+ cmd +")" ) ;
				}
				if ( ZRC == 127 )
				{
					vreplace( "ZVAL1", word( cmd, 1 ) ) ;
					msg     = "PSYS012X" ;
					message = "Command not found" ;
				}
				else
				{
					message = "Executed" ;
				}
			}
		}
		tbput( dslist ) ;
		tbskip( dslist ) ;
	}

	tbtop( dslist ) ;
	tbskip( dslist, ztdtop ) ;

	return 8 ;
}


int pflst0a::action_Block_Add( const string& entry1 )
{
	string pfl ;

	pfl = upper( word( zcmd, 3 ) ) ;
	if ( pfl != "" )
	{
		if ( !isvalidName( pfl ) || pfl.size() < 3 )
		{
			setmsg( "FLST012W" ) ;
			return 8 ;
		}
	}
	else
	{
		pfl = getPFLName() ;
	}

	submit( "PGM("+ get_dialogue_var( "ZRFLPGM" ) +") PARM(PLF " + pfl + " "+ entry1 +")" ) ;

	message = "Added to "+ pfl ;
	tbput( dslist ) ;

	return 0 ;
}


int pflst0a::action_Block_Browse( const string& entry1 )
{
	bool terminat = false ;

	if ( !is_regular_file( entry1 ) )
	{
		message = "" ;
		tbput( dslist ) ;
		return 0 ;
	}

	browse_entry( entry1, terminat ) ;

	return ( terminat ) ? 4 : 0 ;
}


int pflst0a::action_Block_Copy( const string& entry1 )
{
	bool terminat = false ;

	if ( !is_directory( entry1 ) && !is_symlink( entry1 ) && !is_regular_file( entry1 ) )
	{
		message = "Invalid type" ;
		tbput( dslist ) ;
	}
	else
	{
		if ( copy_entry( entry1, terminat ) == 0 )
		{
			rebuild1 = true ;
		}
	}

	return ( terminat ) ? 4 : 0 ;
}


int pflst0a::action_Block_CCopy( const string& entry1 )
{
	bool terminat = false ;

	if ( !is_directory( entry1 ) && !is_symlink( entry1 ) && !is_regular_file( entry1 ) )
	{
		message = "Invalid type" ;
		tbput( dslist ) ;
	}
	else
	{
		copy_entry( entry1, terminat, true ) ;
	}

	return ( terminat ) ? 4 : 0 ;
}


int pflst0a::action_Block_Delete( const string& entry1 )
{
	bool terminat = false ;

	delete_entry( entry1, terminat ) ;

	return ( terminat ) ? 4 : 0 ;
}


int pflst0a::action_Block_Edit( const string& entry1 )
{
	bool terminat = false ;

	if ( !is_regular_file( entry1 ) )
	{
		message = "" ;
		tbput( dslist ) ;
		return 0 ;
	}

	edit_entry( entry1, terminat ) ;
	tbput( dslist ) ;

	return ( terminat ) ? 4 : 0 ;
}


int pflst0a::action_Block_View( const string& entry1 )
{
	bool terminat = false ;

	if ( !is_regular_file( entry1 ) )
	{
		message = "" ;
		tbput( dslist ) ;
		return 0 ;
	}

	view_entry( entry1, terminat ) ;

	return ( terminat ) ? 4 : 0 ;
}


int pflst0a::action_Block_Fsys( const string& entry1 )
{
	bool terminat = false ;

	show_FileSystem( entry1, terminat ) ;
	tbput( dslist ) ;

	return ( terminat ) ? 4 : 0 ;
}


int pflst0a::action_Block_Info( const string& entry1 )
{
	bool terminat = false ;

	show_info( entry1, terminat ) ;
	tbput( dslist ) ;

	return ( terminat ) ? 4 : 0 ;
}


int pflst0a::action_Block_List( const string& entry1 )
{
	bool terminat = false ;

	try
	{
		if ( !is_directory( entry1 ) )
		{
			message = "" ;
			tbput( dslist ) ;
			return 0 ;
		}
	}
	catch (...)
	{
		message = "" ;
		tbput( dslist ) ;
		return 0 ;
	}

	list_entry( entry1, terminat ) ;

	return ( terminat ) ? 4 : 0 ;
}


int pflst0a::action_Block_Modify( const string& entry1 )
{
	bool terminat = false ;

	modify_entry( entry1, terminat ) ;

	return ( terminat ) ? 4 : 0 ;
}


int pflst0a::action_Block_Rename( const string& entry1 )
{
	bool terminat = false ;

	rename_entry( entry1, terminat ) ;

	return ( terminat ) ? 4 : 0 ;
}


void pflst0a::delete_entry( const string& entry1,
			    bool& terminat )
{
	int num ;

	bool del ;
	bool delokay = false ;

	string zcmd1   ;
	string confoff ;
	string nemptok ;

	boost::system::error_code ec ;

	vdefine( "ZCMD1", &zcmd1 ) ;

	vcopy( "CONFOFF", confoff, MOVE ) ;
	del = ( confoff == "/" ) ;

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
		vcopy( "NEMPTOK", nemptok, MOVE ) ;
		if ( nemptok == "/" )
		{
			num = remove_all( entry1.c_str(), ec ) ;
			if ( ec.value() == boost::system::errc::success )
			{
				delokay = true ;
				rsn     = d2ds( num ) + " file(s) deleted" ;
				setmsg( "FLST011N" ) ;
				tbdelete( dslist ) ;
			}
			else
			{
				msg     = "FLST011O"   ;
				rsn     = ec.message() ;
				message = ec.message() ;
				llog( "E", "Delete of "+ entry1 +" failed."<< endl ) ;
				llog( "E",  message << " " << num << " file(s) deleted "<< endl ) ;
			}
		}
		else
		{
			remove( entry1.c_str(), ec ) ;
			if ( ec.value() == boost::system::errc::success )
			{
				delokay = true ;
				rsn     = "1 entry deleted" ;
				setmsg( "FLST011N" ) ;
				tbdelete( dslist ) ;
			}
			else
			{
				msg     = "FLST011O"   ;
				rsn     = ec.message() ;
				message = ec.message() ;
				llog( "E", "Delete of "+ entry1 +" failed." << endl ) ;
				llog( "E", message << endl ) ;
			}
		}
	}

	if ( zcmd1 == "QUIT" )
	{
		setmsg( "FLST012V" ) ;
		message  = "Terminated" ;
		terminat = true ;
	}

	if ( !delokay )
	{
		tbput( dslist ) ;
	}

	vdelete( "ZCMD1" ) ;
}


int pflst0a::edit_entry( const string& entry1,
			 bool& terminat )
{
	int RCode = 0 ;

	string zoname ;
	string zvmode ;
	string eeimac ;
	string eepanl ;
	string eeprof ;
	string eelmac ;
	string eerecl ;
	string eeccan ;
	string eeprsps ;
	string eetabss ;

	const string vlist1 = "EEIMAC EERECL EEPANL EEPROF EELMAC EECCAN EEPRSPS EETABSS ZONAME ZVMODE" ;
	const string vlist2 = "EEIMAC EERECL EEPANL" ;
	const string vlist3 = "EEPROF EELMAC EECCAN EEPRSPS EETABSS" ;

	if ( editRecovery( "EDIT" ) == 8 )
	{
		return 0 ;
	}

	msg = "" ;

	vdefine( vlist1, &eeimac, &eerecl, &eepanl, &eeprof, &eelmac, &eeccan, &eeprsps, &eetabss, &zoname, &zvmode ) ;
	vget( vlist2, SHARED ) ;
	vget( vlist3, PROFILE ) ;

	if ( pflevep == "/" )
	{
		zvmode = "EDIT" ;
		zoname = entry1 ;
		addpop( "", 2, 5 ) ;
		while ( true )
		{
			display( "PFLST0AT", msg ) ;
			if ( RC > 0 )
			{
				message = "Edit cancelled" ;
				rempop() ;
				vdelete( vlist1 ) ;
				return 0 ;
			}
			if ( eepanl != "" )
			{
				control( "ERRORS", "RETURN" ) ;
				pquery( eepanl, "ZAREA" ) ;
				if ( RC > 0 )
				{
					vreplace( "ZVAL1", eepanl ) ;
					msg = "PEDT013H" ;
					control( "ERRORS", "CANCEL" ) ;
					continue ;
				}
				control( "ERRORS", "CANCEL" ) ;
			}
			break ;
		}
		rempop() ;
	}

	set_scrname( "EDIT" ) ;

	control( "ERRORS", "RETURN" ) ;
	edit( entry1,
	      eepanl,
	      eeimac,
	      eeprof,
	      "",
	      eelmac,
	      ( eeccan  == "/" ? "YES" : "NO" ),
	      ( eeprsps == "/" ? "PRESERVE" : "" ),
	      "",
	      ( eerecl == "" ) ? 0 : ds2d( eerecl ) ) ;
	if ( RC == 0 || RC == 4 )
	{
		setmsg( ZRESULT != "" ? ZRESULT : "FLST012N" ) ;
		message = "Edited" ;
		if ( RC == 0 )
		{
			if ( stats && lstat( entry1.c_str(), &results ) == 0 )
			{
				getFileAttributes() ;
			}
		}
	}
	else if ( ( RC > 11 && RC < 20 ) || ( RC == 20 && ZRSN == 8 ) )
	{
		vcopy( "ZERRMSG", msg ) ;
		if ( msg != "" )
		{
			vcopy( "ZERRSM", message ) ;
			vget( "ZVAL1 ZVAL2 ZVAL3", SHARED ) ;
			setmsg( msg ) ;
		}
		else
		{
			message = "Error" ;
		}
		RCode = 12 ;
	}
	else if ( RC == 20 )
	{
		msg = get_dialogue_var( "ZERRMSG" ) ;
		getmsg( msg, "ZERRSM", "ZERRLM" ) ;
		vput( "ZERRSM ZERRLM", SHARED ) ;
		select( "PGM(PPSP01A) PARM(PSYSER3 "+ get_dialogue_var( "ZERRMSG" ) +")" ) ;
		message = "ABENDED" ;
		RCode = 12 ;
	}

	control( "ERRORS", "CANCEL" ) ;
	restore_scrname() ;

	vdelete( vlist1 ) ;

	return RCode ;
}


void pflst0a::browse_entry( const string& entry1,
			    bool& terminat )
{
	control( "ERRORS", "RETURN" ) ;

	set_scrname( "BROWSE" ) ;
	browse( entry1 ) ;
	if ( RC == 0 )
	{
		setmsg( "FLST012P" ) ;
		message = "Browsed" ;
	}
	else if ( RC == 12 )
	{
		message = "File empty" ;
		msg     = ZRESULT      ;
	}
	else
	{
		message = "Error" ;
		if ( isvalidName( ZRESULT ) )
		{
			msg = ZRESULT ;
		}
	}

	control( "ERRORS", "CANCEL" ) ;
	restore_scrname() ;
}


void pflst0a::view_entry( const string& entry1,
			  bool& terminat )
{
	string msg ;

	string zoname ;
	string zvmode ;
	string eeimac ;
	string eepanl ;
	string eeprof ;
	string eelmac ;
	string eeccan ;
	string becwarn ;
	string eetabss ;

	const string vlist1 = "EEIMAC EEPANL EEPROF EELMAC EECCAN EETABSS BECWARN ZONAME ZVMODE" ;
	const string vlist2 = "EEIMAC EEPANL" ;
	const string vlist3 = "EEPROF EELMAC EECCAN EETABSS BECWARN" ;

	if ( editRecovery( "VIEW" ) == 8 )
	{
		return ;
	}

	vdefine( vlist1, &eeimac, &eepanl, &eeprof, &eelmac, &eeccan, &eetabss, &becwarn, &zoname, &zvmode ) ;
	vget( vlist2, SHARED ) ;
	vget( vlist3, PROFILE ) ;

	if ( pflevep == "/" )
	{
		zvmode = "VIEW" ;
		zoname = entry1 ;
		addpop( "", 2, 5 ) ;
		while ( true )
		{
			display( "PFLST0AT", msg ) ;
			if ( RC > 0 )
			{
				message = "View cancelled" ;
				rempop() ;
				vdelete( vlist1 ) ;
				return ;
			}
			if ( eepanl != "" )
			{
				control( "ERRORS", "RETURN" ) ;
				pquery( eepanl, "ZAREA" ) ;
				if ( RC > 0 )
				{
					vreplace( "ZVAL1", eepanl ) ;
					msg = "PEDT013H" ;
					control( "ERRORS", "CANCEL" ) ;
					continue ;
				}
				control( "ERRORS", "CANCEL" ) ;
			}
			break ;
		}
		rempop() ;
	}

	set_scrname( "VIEW" ) ;

	control( "ERRORS", "RETURN" ) ;
	view( entry1,
	      eepanl,
	      eeimac,
	      eeprof,
	      "",
	      eelmac,
	      ( eeccan  == "/" ? "YES" : "NO" ),
	      ( becwarn == "/" ? "YES" : "NO" ) ) ;
	if ( RC == 0 || RC == 4 )
	{
		setmsg( ZRESULT != "" ? ZRESULT : "FLST012O" ) ;
		message = "Viewed" ;
	}
	else if ( RC > 11 && RC < 20 )
	{
		vcopy( "ZERRMSG", msg ) ;
		if ( msg != "" )
		{
			vcopy( "ZERRSM", message ) ;
			vget( "ZVAL1 ZVAL2 ZVAL3", SHARED ) ;
			setmsg( msg ) ;
		}
		else
		{
			message = "Error" ;
		}
	}
	else if ( RC == 20 )
	{
		msg = get_dialogue_var( "ZERRMSG" ) ;
		getmsg( msg, "ZERRSM", "ZERRLM" ) ;
		vput( "ZERRSM ZERRLM", SHARED ) ;
		select( "PGM(PPSP01A) PARM(PSYSER3 "+ msg +")" ) ;
		message = "ABENDED" ;
	}

	control( "ERRORS", "CANCEL" ) ;
	restore_scrname() ;

	vdelete( vlist1 ) ;
}


void pflst0a::show_FileSystem( const string& p,
			       bool& terminat )
{
	//
	// Display file system for this entry.
	//

	int match ;
	int match_max = 0 ;

	string ientry = p ;

	string mnt_dir ;
	string mnt_fsname ;
	string mnt_type ;
	string mnt_opts ;

	string immode ;
	string imsync ;
	string imiguid ;
	string ibsize ;
	string ibfree ;
	string ibavail ;
	string iblocks ;
	string ipcentu ;
	string ifiles ;
	string iffree ;
	string ifavail ;
	string ifsid ;

	const string vlist1 = "IFSNAME IDIR ITYPE IOPTS" ;
	const string vlist2 = "IENTRY IMMODE IMSYNC IMIGUID IBSIZE IBFREE IBAVAIL IBLOCKS IPCENTU" ;
	const string vlist3 = "IFILES IFFREE IFAVAIL IFSID" ;

	vdefine( vlist1, &mnt_fsname, &mnt_dir, &mnt_type, &mnt_opts ) ;
	vdefine( vlist2, &ientry, &immode, &imsync, &imiguid, &ibsize, &ibfree, &ibavail, &iblocks, &ipcentu ) ;
	vdefine( vlist3, &ifiles, &iffree, &ifavail, &ifsid ) ;

	struct mntent* ent ;

	FILE* afile = setmntent( _PATH_PROC_MOUNTS, "r" ) ;

	if ( afile )
	{
		while ( ( ent = getmntent( afile ) ) )
		{
			if ( string( ent->mnt_type ) != "autofs" && is_subpath( p, ent->mnt_dir, match ) )
			{
				if ( match > match_max )
				{
					mnt_dir    = ent->mnt_dir ;
					mnt_fsname = ent->mnt_fsname ;
					mnt_type   = ent->mnt_type ;
					mnt_opts   = ent->mnt_opts ;
					match_max  = match ;
				}
			}
		}
	}

	endmntent( afile ) ;

	struct statvfs buf ;

	if ( statvfs( mnt_dir.c_str(), &buf ) == -1 )
	{
		vreplace( "FLSTERR", strerror( errno ) ) ;
		setmsg( "FLST013O" ) ;
	}
	else
	{
		ibsize  = addCommas( buf.f_bsize ) ;
		ibfree  = addCommas( buf.f_bfree ) ;
		ibavail = addCommas( buf.f_bavail ) ;
		iblocks = addCommas( buf.f_blocks ) ;
		ifiles  = addCommas( buf.f_files ) ;
		iffree  = addCommas( buf.f_ffree ) ;
		ifavail = addCommas( buf.f_favail ) ;
		ifsid   = ui2ds( buf.f_fsid ) ;
		immode  = ( buf.f_flag & ST_RDONLY ) ? "RO" : "R/W" ;
		imsync  = ( buf.f_flag & ST_SYNCHRONOUS ) ? "YES" : "NO" ;
		imiguid = ( buf.f_flag & ST_NOSUID ) ? "YES" : "NO" ;
		if ( buf.f_blocks > 0 )
		{
			ipcentu = to_string( ( ( ( buf.f_blocks - buf.f_bavail ) * 1000 / buf.f_blocks ) + 5 ) / 10 ) + "%" ;
		}
	}

	RC = 0 ;
	while ( RC == 0 )
	{
		display( "PFLST0AU" ) ;
	}

	vdelete( vlist1, vlist2, vlist3 ) ;

	message = "File system information" ;

	if ( get_dialogue_var( "ZCMD1" ) == "QUIT" )
	{
		setmsg( "FLST012V" ) ;
		message = "Terminated" ;
		tbput( dslist ) ;
		terminat = true ;
	}
}


void pflst0a::show_info( const string& p,
			 bool& terminat )
{
	int rc ;

	string irlnk1 ;
	string irlnk2 ;

	const string vlist1 = "IENTRY ITYPE IINODE INLNKS IPERMISS ISIZE" ;
	const string vlist2 = "IRLNK1 IRLNK2 IOWNER IGROUP IMAJ IMIN IBLKSIZE" ;
	const string vlist3 = "ISTCDATE IMODDATE IACCDATE ISETUID" ;
	const string vlist4 = "ISETGID ISTICKY" ;

	struct tm* time_info ;

	char buf[ 20 ] ;
	char* buffer   ;

	size_t bufferSize = 255 ;

	try
	{
		if ( !is_symlink( p ) && !exists( p ) )
		{
			ZRC  = 8  ;
			ZRSN = 12 ;
			return  ;
		}
	}
	catch ( const filesystem_error& ec )
	{
		ZRC  = 8  ;
		ZRSN = 16 ;
		rsn  = ec.what() ;
		setmsg( "FLST013M" ) ;
		return ;
	}

	if ( lstat( p.c_str(), &results ) != 0 )
	{
		ZRC  = 12 ;
		ZRSN = 20 ;
		return ;
	}

	vdefine( vlist1, &ientry, &itype,  &iinode, &inlnks, &ipermiss, &isize ) ;
	vdefine( vlist2, &irlnk1, &irlnk2, &iowner, &igroup, &imaj, &imin, &iblksize ) ;
	vdefine( vlist3, &istcdate, &imoddate, &iaccdate, &isetuid ) ;
	vdefine( vlist4, &isetgid, &isticky ) ;

	ientry = p ;
	itype = ( S_ISDIR( results.st_mode )  ) ? "Directory"     :
		( S_ISREG( results.st_mode )  ) ? "File"          :
		( S_ISCHR( results.st_mode )  ) ? "Character"     :
		( S_ISBLK( results.st_mode )  ) ? "Block"         :
		( S_ISFIFO( results.st_mode ) ) ? "Fifo"          :
		( S_ISSOCK( results.st_mode ) ) ? "Socket"        :
		( S_ISLNK( results.st_mode )  ) ? "Symbolic link" : "Unknown" ;
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

	isetuid = ( results.st_mode & S_ISUID ) ? "YES" : "NO" ;
	isetgid = ( results.st_mode & S_ISGID ) ? "YES" : "NO" ;
	isticky = ( results.st_mode & S_ISVTX ) ? "YES" : "NO" ;

	iinode = to_string( results.st_ino ) ;
	inlnks = to_string( results.st_nlink ) ;
	isize  = to_string( results.st_size ) ;

	time_info = gmtime( &(results.st_ctime) ) ;
	strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ; istcdate = buf ;

	time_info = gmtime( &(results.st_mtime) ) ;
	strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ; imoddate = buf ;

	time_info = gmtime( &(results.st_atime) ) ;
	strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ; iaccdate = buf ;


	irlnk1 = "" ;
	irlnk2 = "" ;
	if ( S_ISLNK( results.st_mode ) )
	{
		while ( true )
		{
			buffer = new char[ bufferSize ] ;
			rc     = readlink( p.c_str(), buffer, bufferSize ) ;
			if ( rc == -1 )
			{
				delete[] buffer ;
				if ( errno == ENAMETOOLONG ) { bufferSize += 255; }
				else                         { break ;            }
			}
			else
			{
				irlnk1 = string( buffer, rc ) ;
				delete[] buffer ;
				irlnk2 = ( irlnk1.front() != '/' ) ?
					   p.substr( 0, p.find_last_of( '/' ) + 1 ) + irlnk1 : irlnk1 ;
				break ;
			}
		}
	}

	imaj     = to_string( major( results.st_dev ) ) ;
	imin     = to_string( minor( results.st_dev ) ) ;
	iblksize = to_string( results.st_blksize ) ;

	RC = 0 ;
	while ( RC == 0 )
	{
		display( "PFLST0A2" ) ;
	}

	vdelete( vlist1, vlist2, vlist3, vlist4 ) ;

	message = "Information" ;

	if ( get_dialogue_var( "ZCMD1" ) == "QUIT" )
	{
		setmsg( "FLST012V" ) ;
		message = "Terminated" ;
		tbput( dslist ) ;
		terminat = true ;
	}
}


void pflst0a::modify_entry( const string& p,
			    bool& terminat )
{
	int i  ;
	int i1 ;
	int i2 ;
	int i3 ;

	bool changed ;

	string zcmd1    ;
	string opermiss ;
	string osetuid  ;
	string osetgid  ;
	string osticky  ;
	string oowner   ;
	string ogroup   ;
	string oownern  ;
	string ogroupn  ;

	const string vlist1 = "IENTRY ITYPE IPERMISS" ;
	const string vlist2 = "IOWNER IGROUP ISETUID ISETGID" ;
	const string vlist3 = "ISTICKY IOWNERN IGROUPN ZCMD1" ;

	mode_t t  ;
	uid_t uid ;
	gid_t gid ;

	if ( lstat( p.c_str(), &results ) != 0 )
	{
		zcmd    = "" ;
		message = "lstat failed" ;
		vreplace( "FILE", p ) ;
		setmsg( "FLST011T" )  ;
		return ;
	}

	vdefine( vlist1, &ientry, &itype, &ipermiss ) ;
	vdefine( vlist2, &iowner, &igroup, &isetuid, &isetgid ) ;
	vdefine( vlist3, &isticky, &iownern, &igroupn, &zcmd1 ) ;

	ientry = p ;
	itype = ( S_ISDIR( results.st_mode )  ) ? "Directory"     :
		( S_ISREG( results.st_mode )  ) ? "File"          :
		( S_ISCHR( results.st_mode )  ) ? "Character"     :
		( S_ISBLK( results.st_mode )  ) ? "Block"         :
		( S_ISFIFO( results.st_mode ) ) ? "Fifo"          :
		( S_ISSOCK( results.st_mode ) ) ? "Socket"        :
		( S_ISLNK( results.st_mode )  ) ? "Symbolic link" : "Unknown" ;

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

	isetuid = ( results.st_mode & S_ISUID ) ? "/" : "" ;
	isetgid = ( results.st_mode & S_ISGID ) ? "/" : "" ;
	isticky = ( results.st_mode & S_ISVTX ) ? "/" : "" ;

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

	display( "PFLST0A6", msg, "ZCMD1" ) ;
	if ( RC == 8 )
	{
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
			if ( !pwd )
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
			if ( !pwd )
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
			if ( !grp )
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
			if ( !grp )
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
			if ( msg != "" )
			{
				message = "Modify failed" ;
			}
			else
			{
				if ( stats )
				{
					getFilePermissions() ;
					getFileUIDGID()      ;
				}
				message = "Modified" ;
				setmsg( "FLST011A" ) ;
			}
		}
	}

	if ( zcmd1 == "QUIT" )
	{
		setmsg( "FLST012V" ) ;
		message = "Terminated" ;
		tbput( dslist ) ;
		terminat = true ;
	}

	tbput( dslist ) ;

	vdelete( vlist1, vlist2, vlist3 ) ;
}


void pflst0a::getFileAttributes()
{
	char buf[ 20 ] ;

	struct tm* time_info ;

	size      = to_string( results.st_size ) ;
	moddates  = to_string( results.st_mtime ) ;
	accdates  = to_string( results.st_atime ) ;
	stcdates  = to_string( results.st_ctime ) ;

	time_info = gmtime( &(results.st_mtime) ) ;
	strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ;
	moddate   = buf ;
	time_info = gmtime( &(results.st_atime) ) ;
	strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ;
	accdate   = buf ;
	time_info = gmtime( &(results.st_ctime) ) ;
	strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ;
	stcdate   = buf ;
}


void pflst0a::getFilePermissions()
{
	type    = "----" ;
	permiss = string( 10, '-' ) ;

	type = ( S_ISDIR( results.st_mode )  ) ? "Dir"    :
	       ( S_ISREG( results.st_mode )  ) ? "File"   :
	       ( S_ISCHR( results.st_mode )  ) ? "Char"   :
	       ( S_ISBLK( results.st_mode )  ) ? "Block"  :
	       ( S_ISFIFO( results.st_mode ) ) ? "Fifo"   :
	       ( S_ISSOCK( results.st_mode ) ) ? "Socket" :
	       ( S_ISLNK( results.st_mode )  ) ? "Syml"   : "Unknown" ;

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
}


void pflst0a::getFileUIDGID()
{
	owner = "" ;
	group = "" ;

	struct passwd* pw = getpwuid( results.st_uid ) ;
	if ( pw )
	{
		owner = pw->pw_name ;
	}

	struct group* gr = getgrgid( results.st_gid ) ;
	if ( gr )
	{
		group = gr->gr_name ;
	}
}


void pflst0a::list_entry( const string& entry1,
			  bool& terminat )
{
	size_t p1 ;

	string t ;

	try
	{
		if ( is_directory( entry1 ) )
		{
			t = entry1 ;
		}
		else
		{
			p1 = entry1.find_last_of( '/' ) ;
			if ( p1 == string::npos )
			{
				msg = "FLST011M" ;
				return ;
			}
			t = entry1.substr( 0, p1 ) ;
		}
	}
	catch (...)
	{
		message = "Error listing directory" ;
		tbput( dslist ) ;
		return ;
	}

	listDirectory( t ) ;
	if ( ZRESULT != "" )
	{
		message = ZRESULT ;
	}
	else
	{
		message = "Listed" ;
	}

	tbput( dslist ) ;
}


void pflst0a::list_entry_recursive( const string& entry1,
				    bool& terminat )
{
	size_t p1 ;

	string t ;

	try
	{
		if ( is_directory( entry1 ) )
		{
			t = entry1 ;
		}
		else
		{
			p1 = entry1.find_last_of( '/' ) ;
			if ( p1 == string::npos )
			{
				msg = "FLST011M" ;
				return ;
			}
			t = entry1.substr( 0, p1 ) ;
		}
	}
	catch (...)
	{
		message = "Error listing directory" ;
		tbput( dslist ) ;
		return ;
	}

	select( "PGM(" + get_dialogue_var( "ZFLSTPGM" ) + ") PARM(DIRR " + t + ")" ) ;

	message = ( ZRESULT != "" ) ? ZRESULT : "Listed" ;

	tbput( dslist ) ;
}


void pflst0a::browseTree( const string& tname )
{
	int i ;
	int csrrow ;
	int crpx   ;

	bool terminat ;

	string tsel   ;
	string tfile  ;
	string tentry ;
	string panel  ;
	string cursor ;
	string tab    ;
	string line   ;
	string msgloc ;

	const string vlist1 = "TSEL TFILE TENTRY ZDIR" ;
	const string vlist2 = "CRP" ;

	std::ifstream fin ;
	fin.open( tname.c_str() ) ;
	if ( !fin.is_open() )
	{
		RC   = 16 ;
		llog( "E", "Error opening file " << tname << endl ) ;
		return ;
	}

	vdefine( vlist1, &tsel, &tfile, &tentry, &zdir ) ;
	vdefine( vlist2, &crp ) ;

	tab = "FTR" + d2ds( taskid(), 5 ) ;

	tbcreate( tab,
		  "",
		  "(TSEL,TFILE,TENTRY)",
		  NOWRITE ) ;

	tsel = "" ;
	i    = 1  ;
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
		++i ;

	}
	tbtop( tab ) ;

	ztdvrows  = 1 ;
	ztdsels   = 0 ;
	ztdtop    = 0 ;
	crpx      = 0 ;
	csrrow    = 0 ;
	msgloc    = "" ;
	msg       = "" ;
	cursor    = "ZCMD" ;

	while ( true )
	{
		if ( msg == "" && ztdsels > 0 )
		{
			--ztdsels ;
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
		if ( msg != "" && cursor == "" )
		{
			cursor = "TSEL" ;
			msgloc = "TSEL" ;
			csrrow = crpx ;
			panel  = ""   ;
		}
		else if ( msg != "" && cursor == "ZCMD" )
		{
			csrrow = crpx ;
			panel  = ""   ;
		}
		else
		{
			cursor = "ZCMD" ;
		}
		tbdispl( tab,
			 panel,
			 msg,
			 cursor,
			 csrrow,
			 1,
			 "YES",
			 "CRP",
			 "",
			 msgloc ) ;
		if ( RC == 8 ) { break ; }

		msg    = "" ;
		cursor = "" ;
		msgloc = "" ;
		crpx   = crp;
		csrrow = 0  ;
		if ( tsel == "S" )
		{
			set_scrname( "BROWSE" ) ;
			control( "ERRORS", "RETURN" ) ;
			browse( tfile ) ;
			if ( isvalidName( ZRESULT ) )
			{
				msg = ZRESULT ;
			}
			control( "ERRORS", "CANCEL" ) ;
			restore_scrname() ;
		}
		else if ( tsel == "I" )
		{
			show_info( tfile, terminat ) ;
		}
		else if ( tsel == "L" )
		{
			list_entry( tfile, terminat ) ;
		}
		else if ( tsel == "E" )
		{
			if ( editRecovery( "EDIT" ) == 8 ) { continue ; }
			set_scrname( "EDIT" ) ;
			control( "ERRORS", "RETURN" ) ;
			edit( tfile ) ;
			if ( RC == 0 || RC == 4 )
			{
				setmsg( ZRESULT != "" ? ZRESULT : "FLST012N" ) ;
			}
			else if ( RC > 11 )
			{
				msg = isvalidName( ZRESULT ) ? ZRESULT : "" ;
			}
			control( "ERRORS", "CANCEL" ) ;
			restore_scrname() ;
		}
	}

	tbend( tab ) ;
	vdelete( vlist1, vlist2 ) ;
}


int pflst0a::editRecovery( const string& zvmode )
{
	string msg   ;
	string zcmd  ;
	string zfile ;
	string zedmode ;

	const string vlist = "ZCMD ZFILE" ;

	vdefine( vlist, &zcmd, &zfile ) ;

	vreplace( "ZVMODE", zvmode ) ;

	edrec( "INIT" ) ;

	while ( true )
	{
		if ( msg == "" )
		{
			edrec( "QUERY" ) ;
			if ( RC == 0 ) { break ; }
			vcopy( "ZEDMODE", zedmode, MOVE ) ;
			if ( zvmode.compare( 0, 1, zedmode ) != 0 )
			{
				edrec( "DEFER" ) ;
				continue ;
			}
		}
		vcopy( "ZEDTFILE", zfile, MOVE ) ;
		display( "PEDIT014", msg, "ZCMD" ) ;
		if ( RC == 8 && zcmd != "CANCEL" )
		{
			edrec( "DEFER" ) ;
			vdelete( vlist ) ;
			return 8 ;
		}
		msg = "" ;
		if ( zcmd == "" )
		{
			control( "ERRORS", "RETURN" ) ;
			edrec( "PROCESS" ) ;
			if ( RC >= 12 )
			{
				vcopy( "ZERRMSG", msg, MOVE ) ;
			}
			control( "ERRORS", "CANCEL" ) ;
		}
		else if ( zcmd == "CANCEL" )
		{
			edrec( "CANCEL" ) ;
		}
		else
		{
			edrec( "DEFER" ) ;
		}
	}

	vdelete( vlist ) ;

	return 0 ;
}


void pflst0a::confirm_cmd( string& cmd,
			   bool& execute,
			   bool& terminat )
{
	string zcmd1 ;
	string pflcmd1 ;

	const string vlist1 = "ZCMD1 PFLCMD1" ;

	vdefine( vlist1, &zcmd1, &pflcmd1 ) ;

	pflcmd1 = cmd ;

	addpop( "", 5, 0 ) ;
	display( "PFLST0AK" ) ;
	if ( RC == 8 )
	{
		if ( zcmd1 == "QUIT" )
		{
			setmsg( "FLST012V" ) ;
			message = "Terminated" ;
			tbput( dslist ) ;
			terminat = true ;
		}
		else
		{
			setmsg( "FLST012U" ) ;
			message = "Cancelled" ;
			execute = false ;
		}
	}
	else
	{
		execute = true ;
		cmd     = pflcmd1 ;
	}

	rempop() ;
	vdelete( vlist1 ) ;
}


void pflst0a::confirm_shell( string& cmd,
			     bool& execute,
			     bool& terminat )
{
	string zcmd1 ;
	string pflcmd1 ;

	const string vlist1 = "ZCMD1 PFLCMD1" ;

	vdefine( vlist1, &zcmd1, &pflcmd1 ) ;

	pflcmd1 = cmd ;

	addpop( "", 5, 0 ) ;
	display( "PFLST0AR" ) ;
	if ( RC == 8 )
	{
		if ( zcmd1 == "QUIT" )
		{
			setmsg( "FLST012V" ) ;
			message = "Terminated" ;
			tbput( dslist ) ;
			terminat = true ;
		}
		else
		{
			setmsg( "FLST012U" ) ;
			message = "Cancelled" ;
			execute = false ;
		}
	}
	else
	{
		execute = true ;
		cmd     = pflcmd1 ;
	}

	rempop() ;
	vdelete( vlist1 ) ;
}


string pflst0a::expand_dir1( const string& parms )
{
	//
	// If passed directory begins with '?', display listing replacing '?' with '/' and using this
	// as the starting point.
	//
	// Return null if there is an error (directory does not exist or permission error).
	//
	// Cursor sensitive.  Only characters before the current cursor position (if > 1) in the field (ZFECSRP) will
	// be taken into account.
	//
	// If directory matches a listing entry, return the next entry (if not end-of-list).
	//
	// If directory is an abbreviation of one in the listing, return the current entry.
	//
	// If first parameter is ALL, all entries are used.
	// If first parameter is DO1, filter on directories.
	// If first parameter is FO2, filter on files.
	//

	int i ;

	size_t pos ;
	size_t p1 ;

	bool showl ;

	string type1 ;
	string dir1 ;
	string entry1 ;
	string dir ;
	string cpos ;
	string data1 ;

	ZRC   = 8  ;
	data1 = "" ;
	showl = false ;

	using vec = vector<path> ;

	vec v ;
	vec::iterator new_end ;

	type1 = word( parms, 1 ) ;

	vget( "ZFEDATA0 ZFECSRP", SHARED ) ;
	vcopy( "ZFEDATA0", dir, MOVE ) ;
	vcopy( "ZFECSRP", cpos, MOVE ) ;

	p1 = dir.find_first_not_of( ' ' ) ;
	if ( p1 == string::npos ) { p1 = 0 ; }
	pos = ds2d( cpos ) - p1 ;

	trim_left( dir ) ;

	if ( type1 == "FO2" && dir != "" )
	{
		if ( dir.front() == '?' )
		{
			showl = true ;
			dir   = ""   ;
		}
		vget( "ZFEDATA1", SHARED ) ;
		vcopy( "ZFEDATA1", data1, MOVE ) ;
		if ( trim( data1 ) == "" )
		{
			vget( "ZFEDATA2", SHARED ) ;
			vcopy( "ZFEDATA2", data1, MOVE ) ;
		}
		if ( data1 == "" )
		{
			RC = 8    ;
			return "" ;
		}
		if ( data1.back() == '/' ) { dir = data1 + dir ; --pos ; }
		else                       { dir = data1 + "/" + dir   ; }
		pos = pos + data1.size() + 1 ;
		if ( showl )
		{
		}
	}

	if ( dir == "" )
	{
		dir = zhome ;
		pos = zhome.size() ;
	}
	else if ( dir.front() == '?' )
	{
		if ( dir.size() > 1 )
		{
			zpath = dir ;
			zpath.front() = '/' ;
		}
		else
		{
			zpath = zhome ;
		}
		return showListing() ;
	}

	if ( pos > 1 && pos <= dir.size() )
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
		return "" ;
	}

	try
	{
		if ( type1 == "DO1" )
		{
			new_end = remove_if( v.begin(), v.end(),
				  []( const path& a )
				  {
					return !is_directory( a.string() ) ;
				  } ) ;
			v.erase( new_end, v.end() ) ;
		}
		else if ( type1 == "FO1" || type1 == "FO2" )
		{
			new_end = remove_if( v.begin(), v.end(),
				  []( const path& a )
				  {
					return !is_regular_file( a.string() ) ;
				  } ) ;
			v.erase( new_end, v.end() ) ;
		}
		else if ( type1 != "ALL" )
		{
			RC = 16 ;
			return "" ;
		}
	}
	catch ( const filesystem_error& ex )
	{
		return "" ;
	}

	sort( v.begin(), v.end() ) ;

	for ( vec::const_iterator it( v.begin() ) ; it != v.end() ; ++it )
	{
		entry1 = it->string() ;
		if ( !abbrev( entry1, dir ) ) { continue ; }
		ZRC = 0 ;
		if ( entry1 == dir )
		{
			++it ;
			if ( it != v.end() ) { return it->string() ; }
			else                 { return dir          ; }
		}
		break ;
	}

	return entry1 ;
}


string pflst0a::expand_dir2( const string& parms )
{
	//
	// Display a directory listing starting at parms.
	//

	zpath = parms ;

	return showListing() ;
}


string pflst0a::expand_field1( const string& parms )
{
	//
	// Expand field from listing of ZPLIB, ALTLIB, ZLDPATH, REXX_PATH or PATH.
	//
	// Cursor sensitive.  Only characters before the current cursor position (if > 1) in the field (ZFECSRP) will
	// be taken into account.
	//
	// If first parameter is PNL, panel files.
	// If first parameter is REXX, rexx programs.
	// If first parameter is PGM, programs (strip off "lib" and ".so" before comparison).
	// If first parameter is CMD, files in ALTLIB, REXX_PATH and PATH.
	//

	int i ;
	int n ;

	size_t pos ;
	size_t p1 ;

	string Paths ;
	string type1 ;
	string dir1 ;
	string entry1 ;
	string dir ;
	string cpos ;

	ZRC = 8 ;

	using vec = vector<path> ;

	vec v ;
	vec::iterator vec_end ;

	vector<string> mod ;
	vector<string>::iterator it ;
	vector<string>::iterator mod_end ;

	set<string>processed ;

	type1 = word( parms, 1 ) ;

	vget( "ZFEDATA0 ZFECSRP", SHARED ) ;
	vcopy( "ZFEDATA0", dir, MOVE ) ;
	vcopy( "ZFECSRP", cpos, MOVE ) ;

	p1 = dir.find_first_not_of( ' ' ) ;
	if ( p1 == string::npos ) { p1 = 0 ; }
	pos = ds2d( cpos ) - p1 ;
	trim_left( dir ) ;

	if ( pos > 1 && pos < dir.size() )
	{
		dir.erase( pos-1 ) ;
	}

	if ( type1 == "PNL" )
	{
		vcopy( "ZPLIB", Paths, MOVE ) ;
	}
	else if ( type1 == "REXX" )
	{
		Paths = sysexec() ;
	}
	else if ( type1 == "PGM" )
	{
		vcopy( "ZLDPATH", Paths, MOVE ) ;
	}
	else if ( type1 == "CMD" )
	{
		Paths = mergepaths( sysexec(), getenv( "REXX_PATH" ), getenv( "PATH" ) ) ;
	}
	else
	{
		return "" ;
	}

	v.clear() ;
	for ( n = getpaths( Paths ), i = 1 ; i <= n ; ++i )
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
			llog( "E", "Error listing directory." << endl ) ;
			llog( "E", ex.what() << endl ) ;
			return "" ;
		}
	}

	vec_end = remove_if( v.begin(), v.end(),
		  []( const path& a )
		  {
			return !is_regular_file( a.string() ) ;
		  } ) ;

	for_each( v.begin(), vec_end,
		[ &mod ]( const path& a )
		{
			mod.push_back( substr( a.string(), lastpos( "/", a.string() ) + 1 ) ) ;
		} ) ;

	mod_end = mod.end() ;
	if ( type1 == "PGM" )
	{
		mod_end = remove_if( mod.begin(), mod.end(),
			[]( const string& a )
			{
				return ( a.compare( a.size()-3,3 , ".so" ) != 0 ) ;
			} ) ;
	}

	sort( mod.begin(), mod_end ) ;
	mod_end = unique( mod.begin(), mod_end ) ;

	for ( it = mod.begin() ; it != mod_end ; ++it )
	{
		entry1 = *it ;
		if ( type1 == "PGM" ) { entry1 = getAppName( entry1 ) ; }
		if ( !abbrev( entry1, dir ) ) { continue ; }
		ZRC = 0 ;
		if ( entry1 == dir )
		{
			++it ;
			if ( it != mod_end )
			{
				if ( type1 == "PGM" ) { return getAppName( *it ) ; }
				return *it ;
			}
			else
			{
				return dir ;
			}
		}
		else
		{
			return entry1 ;
		}
	}

	return entry1 ;
}


string pflst0a::showListing()
{
	//
	// Show directory listing for EXPAND2 or ? request.
	//
	// Entry to pass back is selected using "/".
	//

	int crpx = 0 ;
	int RCode ;

	int csrrow = 0 ;

	bool cur2sel = false ;

	string zverb ;
	string panel ;
	string cursor ;
	string opath ;
	string rpath ;
	string ofldirs ;
	string flhidden ;
	string ohidden ;

	stack<pair<string,int>> dirlist ;

	const string vlist1 = "ZVERB SEL ENTRY TYPE FLHIDDEN" ;

	vdefine( vlist1, &zverb, &sel, &entry, &type, &flhidden ) ;

	vget( "FLDIRS FLHIDDEN", PROFILE ) ;

	dslist = "DSL" + d2ds( taskid(), 5 ) ;

	create_filelist2() ;

	ZRC      = 0 ;
	ztdvrows = 1 ;
	ztdsels  = 0 ;
	ztdtop   = 0 ;
	msg      = "" ;
	rpath    = "" ;
	pvalid   = true ;
	panel    = "PFLST0A7" ;

	addpop() ;
	while ( true )
	{
		if ( msg == "" && ztdsels > 0 )
		{
			--ztdsels ;
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
				tbskip( dslist, ( 2 - ztddepth ) ) ;
				if ( RC > 0 ) { tbtop( dslist ) ; }
			}
			panel = "PFLST0A7" ;
		}
		if ( cur2sel )
		{
			cursor  = "SEL" ;
			csrrow  = crpx  ;
			cur2sel = false ;
		}
		else
		{
			cursor = "ZCMD" ;
		}
		opath   = zpath ;
		ofldirs = fldirs ;
		ohidden = flhidden ;
		tbdispl( dslist,
			 panel,
			 msg,
			 cursor,
			 csrrow,
			 1,
			 "NO",
			 "CRP" ) ;
		if ( RC == 8 )
		{
			vget( "ZVERB", SHARED ) ;
			if ( dirlist.empty() || zverb == "CANCEL" || zverb == "EXIT" || zverb == "RETURN" )
			{
				ZRC = 8 ;
				break ;
			}
			zpath  = dirlist.top().first ;
			ztdtop = dirlist.top().second ;
			dirlist.pop() ;
			clear_filter_i() ;
			cur2sel = true ;
			include = true ;
			ztdsels = 0 ;
			crpx    = ztdtop ;
			create_filelist2() ;
			continue ;
		}
		msg   = "" ;
		panel = "" ;
		RCode = actionPrimaryCommand2() ;
		if ( RCode == 4 )
		{
			zcmd    = "" ;
			ztdsels = 0  ;
			ztdtop  = 0  ;
			continue ;
		}
		if ( RCode == 8 )
		{
			zcmd = "" ;
			if ( crp > 0 )
			{
				ztdtop = crp ;
			}
			continue ;
		}
		if ( RCode == 12 )
		{
			if ( msg == "" ) { msg = "FLST016" ; }
			continue ;
		}
		zcmd    = "" ;
		cursor  = "" ;
		csrrow  = 0  ;
		crpx    = crp ;
		if ( ofldirs != fldirs || ohidden != flhidden )
		{
			clear_filter_i() ;
			cur2sel = false ;
			include = true ;
			ztdtop  = 0 ;
			ztdsels = 0 ;
			create_filelist2() ;
			continue ;
		}
		else if ( path_change( opath, zpath ) )
		{
			if ( ( !exists( zpath ) || !is_directory( zpath ) ) )
			{
				msg = "PSYS012A" ;
			}
			else
			{
				clear_filter_i() ;
				cur2sel = false ;
				include = true ;
				ztdtop  = 0 ;
				ztdsels = 0 ;
				create_filelist2() ;
				dirlist.push( make_pair( opath, crp ) ) ;
			}
			continue ;
		}
		if ( ztdsels == 0 && zcurinx > 0 )
		{
			tbtop( dslist ) ;
			tbskip( dslist, zcurinx, "", "", "", "", "CRP" ) ;
			sel = ( type == "File" ) ? "B" : "S" ;
		}
		if ( sel != "" )
		{
			cur2sel = true ;
			crpx    = crp ;
			ztdtop  = ( pflscrl == "/" ) ? crp : ztdtop ;
		}
		if ( sel == "B" )
		{
			browse( full_name( zpath, entry ) ) ;
		}
		else if ( sel == "S" )
		{
			dirlist.push( make_pair( zpath, crp ) ) ;
			cur2sel = false ;
			ztdtop  = 0 ;
			ztdsels = 0 ;
			zpath   = full_name( zpath, entry ) ;
			create_filelist2() ;
		}
		else if ( sel == "/" )
		{
			rpath = full_name( zpath, entry ) ;
			break ;
		}
	}
	rempop() ;

	tbend( dslist ) ;
	vdelete( vlist1 ) ;

	return rpath ;
}


void pflst0a::create_filelist2()
{
	//
	// Create file list from zpath and add to table dslist.
	//

	using vec = vector<path> ;

	string* pstr ;

	bool hide ;
	bool match ;
	bool dirsonly = ( fldirs == "/" ) ;

	tbcreate( dslist,
		  "",
		  "(SEL,ENTRY,TYPE)",
		  NOWRITE,
		  REPLACE ) ;

	vec v ;

	if ( zpath == "" ) { zpath = "/" ; }

	vcopy( "FLHIDDEN", pstr ) ;

	hide = ( pstr && *pstr != "/" ) ;

	try
	{
		if ( is_regular_file( zpath ) || !exists( zpath ) )
		{
			zpath = file_directory( zpath ) ;
		}
		copy( directory_iterator( zpath ), directory_iterator(), back_inserter( v ) ) ;
	}
	catch ( const filesystem_error& ex )
	{
		ZRESULT = "List Error" ;
		RC      = 16 ;
		llog( "E", "Error listing directory." << endl ) ;
		llog( "E", ex.what() << endl ) ;
		return ;
	}

	sort( v.begin(), v.end() ) ;

	for ( auto& p : v )
	{
		sel   = "" ;
		entry = p.filename().string() ;
		if ( hide && entry.front() == '.' )
		{
			continue ;
		}
		match = match_filter_i( entry ) ;
		if ( (  include && !match ) ||
		     ( !include &&  match ) )
		{
			continue ;
		}
		if ( lstat( p.string().c_str(), &results ) != 0 )
		{
			type = "----" ;
			tbadd( dslist ) ;
			continue ;
		}
		type = ( S_ISDIR( results.st_mode )  ) ? "Dir"    :
		       ( S_ISREG( results.st_mode )  ) ? "File"   :
		       ( S_ISCHR( results.st_mode )  ) ? "Char"   :
		       ( S_ISBLK( results.st_mode )  ) ? "Block"  :
		       ( S_ISFIFO( results.st_mode ) ) ? "Fifo"   :
		       ( S_ISSOCK( results.st_mode ) ) ? "Socket" :
		       ( S_ISLNK( results.st_mode )  ) ? "Syml"   : "Unknown" ;
		if ( dirsonly && type != "Dir" )
		{
			continue ;
		}
		tbadd( dslist ) ;
	}

	tbtop( dslist ) ;
}


int pflst0a::actionPrimaryCommand2()
{
	if ( zcmd == "" ) { return 0 ; }

	auto it = commandList2.find( get_truename( upper( word( zcmd, 1 ) ) ) ) ;
	if ( it != commandList2.end() )
	{
		return (this->*(it->second))() ;
	}

	msg = "PSYS018" ;

	return 12 ;
}


int pflst0a::action_Touch1()
{
	bool found ;

	string ws = subword( zcmd, 2 ) ;

	if ( words( zcmd ) == 1 )
	{
		vreplace( "ZVAL1", "TOUCH" ) ;
		return 12 ;
	}

	if ( ws.front() != '/' )
	{
		ws = zpath + "/" + ws ;
	}

	try
	{
		found = exists( ws ) ;
	}
	catch ( const filesystem_error& ex )
	{
		msg  = "FLST012F" ;
		zcmd = "" ;
		llog( "E", "Error accessing file." << endl ) ;
		llog( "E", ex.what() << endl ) ;
		return 8  ;
	}

	if ( !found )
	{
		try
		{
			std::ofstream of( ws.c_str() ) ;
			of.close() ;
			create_filelist2() ;
			msg = "FLST012E" ;
		}
		catch ( const filesystem_error& ex )
		{
			msg = "FLST012F" ;
			llog( "E", "Error creating file." << endl ) ;
			llog( "E", ex.what() << endl ) ;
		}
	}
	else
	{
		msg = "FLST012D" ;
		return 12 ;
	}

	ztdsels = 0 ;

	return 8 ;
}


int pflst0a::action_Flip1()
{
	string w2 = word( zcmd, 2 ) ;

	if ( w2 != "" )
	{
		vreplace( "ZVAL1", "X" ) ;
		return 12 ;
	}

	if ( filter_i.empty() )
	{
		return 4 ;
	}

	include = !include ;
	create_filelist2() ;

	return 4 ;
}


int pflst0a::action_Makedir1()
{
	string ws = subword( zcmd, 2 ) ;

	boost::system::error_code ec ;

	if ( ws == "" )
	{
		vreplace( "ZVAL1", "MKDIR" ) ;
		return 12 ;
	}

	if ( ws.front() != '/' )
	{
		ws = zpath + "/" + ws ;
	}

	create_directory( ws, ec ) ;
	if ( ec.value() == boost::system::errc::success )
	{
		msg = "FLST012B" ;
		create_filelist2() ;
	}
	else
	{
		msg = "FLST012C"   ;
		rsn = ec.message() ;
		llog( "E", "Create of directory " + ws + " failed." << endl ) ;
		llog( "E", rsn << endl ) ;
		return 12 ;
	}

	return 8 ;
}

int pflst0a::action_Include1()
{
	string w2 = word( zcmd, 2 ) ;
	string w3 = word( zcmd, 3 ) ;

	if ( w2 == "" || w3 != "" )
	{
		vreplace( "ZVAL1", "I" ) ;
		return 12 ;
	}

	set_filter_i( w2 ) ;
	include = true ;
	create_filelist2() ;

	return 4 ;
}


int pflst0a::action_Refresh1()
{
	if ( words( zcmd ) > 1 )
	{
		vreplace( "ZVAL1", "REFRESH" ) ;
		return 12 ;
	}

	crp  = ztdtop ;
	zcmd = "" ;

	create_filelist2() ;

	return 4 ;
}


int pflst0a::action_Reset1()
{
	if ( words( zcmd ) > 1 )
	{
		vreplace( "ZVAL1", "RESET" ) ;
		return 12 ;
	}

	clear_filter_i() ;
	ztdvrows = 1  ;
	include  = true ;

	create_filelist2() ;

	return 4 ;
}


int pflst0a::action_Exclude1()
{
	string w2 = word( zcmd, 2 ) ;
	string w3 = word( zcmd, 3 ) ;

	if ( w2 == "" || w3 != "" )
	{
		vreplace( "ZVAL1", "X" ) ;
		return 12 ;
	}

	set_filter_i( w2 ) ;
	include = false ;
	create_filelist2() ;

	return 4 ;
}


void pflst0a::listDirectory( string& dir )
{
	//
	// List directory dir using the LMDDISP service.
	//

	string listid ;

	vdefine( "LID", &listid ) ;

	lmdinit( "LID", dir ) ;

	lmddisp( listid ) ;

	lmdfree( listid ) ;

	vdelete( "LID" ) ;
}


string pflst0a::getPFLName()
{
	string* t ;

	vget( "PFLADDL", PROFILE ) ;
	if ( RC == 8 )
	{
		return "FAV1" ;
	}

	vcopy( "PFLADDL", t, LOCATE ) ;

	return *t ;
}


string pflst0a::getAppName( string s )
{
	//
	// Remove "lib" at the front and ".so" at the end of a module name.
	//

	if ( s.size() > 6 )
	{
		s.erase( 0, 3 ) ;
		return s.erase( s.size() - 3 ) ;
	}

	return s ;
}


bool pflst0a::isRexx( string orexx )
{
	locator loc( sysexec(), orexx ) ;
	loc.locate() ;

	return loc.found() ;
}


string pflst0a::expand_name( const string& s )
{
	//
	// Resolve name if it contains *, ? or regex.  If more than one, return "".
	//

	uint i ;

	size_t p1 ;

	string dir1 ;
	string dir2 ;

	regex expression ;

	using vec = vector<path> ;

	vec v ;
	vec::iterator new_end  ;
	vec::const_iterator it ;

	p1 = s.find_last_of( "/" ) ;
	if ( p1 == string::npos ) { return "" ; }

	dir1 = s.substr( 0, p1 ) ;

	try
	{
		expression.assign( conv_regex( s, '?', '*' ) ) ;
	}
	catch ( boost::regex_error& e )
	{
		setmsg( "PSYS012P" ) ;
		return "" ;
	}

	try
	{
		copy( directory_iterator( dir1 ), directory_iterator(), back_inserter( v ) ) ;
	}
	catch ( const filesystem_error& ex )
	{
		return "" ;
	}

	new_end = remove_if( v.begin(), v.end(),
		  [](const path& a)
		  {
			return !is_directory( a.string() ) ;
		  } ) ;

	v.erase( new_end, v.end() ) ;

	for ( dir2 = "", i = 0, it = v.begin() ; it != v.end() ; ++it )
	{
		dir1 = it->string() ;
		if ( !regex_match( dir1.begin(), dir1.end(), expression ) )
		{
			continue ;
		}
		if ( ++i > 1 ) { return "" ; }
		dir2 = dir1 ;
	}

	return dir2 ;
}


string pflst0a::get_tempname()
{
	path temp = temp_directory_path() / unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;

	return temp.native() ;
}


bool pflst0a::path_change( const string& p1,
			   const string& p2 )
{
	return  ( p1 == "" || p2 == "" ) ? false :
		( p1.find_first_of( "?*[" ) != string::npos ) ? ( p1 != p2 ) :
		( ( p1.back() != '/' ) ? p1 + "/" : p1 ) !=
		( ( p2.back() != '/' ) ? p2 + "/" : p2 ) ;
}


void pflst0a::set_filter_i( const string& f, bool case_asis )
{
	//
	// Set and store the include filter.
	//

	set_filter( f, filter_i, filter_i_regex, case_asis ) ;
}


void pflst0a::clear_filter_i()
{
	//
	// Clear the include filters.
	//

	filter_i.clear() ;
	filter_i_regex.clear() ;
}


bool pflst0a::match_filter_i( const string& e1 )
{
	//
	// Return false if e1 matches any of the include filters set.
	//

	for ( uint i = 0 ; i < filter_i.size() ; ++i )
	{
		const string& t = filter_i[ i ] ;
		auto it = filter_i_regex.find( t ) ;
		if ( it != filter_i_regex.end() )
		{
			if ( !regex_match( e1.begin(), e1.end(), it->second ) )
			{
				return false ;
			}
		}
		else if ( pos( t, upper( e1 ) ) == 0 )
		{
			return false ;
		}
	}

	return true ;
}


void pflst0a::set_filter_x( const string& f, bool case_asis )
{
	//
	// Set and store the exclude filter.
	//

	set_filter( f, filter_x, filter_x_regex, case_asis ) ;
}


void pflst0a::clear_filter_x()
{
	//
	// Clear the exclude filters.
	//

	filter_x.clear() ;
	filter_x_regex.clear() ;
}


bool pflst0a::match_filter_x( const string& e1 )
{
	//
	// Return true if e1 matches any of the exclude filters set.
	//

	for ( size_t i = 0 ; i < filter_x.size() ; ++i )
	{
		const string& t = filter_x[ i ] ;
		auto it = filter_x_regex.find( t ) ;
		if ( it != filter_x_regex.end() )
		{
			if ( regex_match( e1.begin(), e1.end(), it->second ) )
			{
				return true ;
			}
		}
		else if ( pos( t, upper( e1 ) ) > 0 )
		{
			return true ;
		}
	}

	return false ;
}


void pflst0a::set_filter( const string& str,
			  vector<string>& filter,
			  map<string, boost::regex>& filter_regex,
			  bool case_asis )
{
	//
	// Set the filter and store the REGEX if applicable.
	//

	string t ;

	regex expression ;

	if ( str == "" || find( filter.begin(), filter.end(), str ) != filter.end() )
	{
		return ;
	}

	if ( case_asis || str.find_first_of( "?*[" ) != string::npos )
	{
		t = conv_regex( str, '?', '*' ) ;
		try
		{
			if ( case_asis )
			{
				expression.assign( t ) ;
			}
			else
			{
				expression.assign( t, boost::regex_constants::icase ) ;
			}
		}
		catch ( boost::regex_error& e )
		{
			vreplace( "ZZSTR", str ) ;
			setmsg( "PSYS012P" ) ;
			return ;
		}
		filter_regex[ str ] = expression ;
		filter.push_back( str ) ;
	}
	else
	{
		filter.push_back( upper( str ) ) ;
	}
}


void pflst0a::set_fmsgs()
{
	fmsg1 = "" ;
	fmsg2 = "" ;
	fmsg3 = "" ;
	fmsg4 = "" ;

	if ( filter_i.size() > 0 || filter_x.size() > 0 )
	{
		fmsg1 = "Name " ;
		if ( filter_i.size() > 0 )
		{
			if ( filter_x.size() > 0 )
			{
				fmsg1 += "(IX): " ;
			}
			else
			{
				fmsg1 += "(I) : " ;
			}
			fmsg1 += filter_i[ 0 ] ;
		}
		else
		{
			fmsg1 += "(X) : " ;
			fmsg1 += filter_x[ 0 ] ;
		}
	}

	if ( ( filter_i.size() + filter_x.size() ) > 1 )
	{
		fmsg1 += " (+)" ;
	}

	if ( search.size() > 0 )
	{
		fmsg2 = "Search   : " + search[ 0 ] ;
		if ( search.size() > 1 )
		{
			fmsg2 += " (+)" ;
		}
	}

	if ( !filter_i.empty() || !filter_x.empty() || !search.empty() || !excludeList.empty() )
	{
		fmsg3 = "*Filter*" ;
	}

	if ( !include )
	{
		fmsg3 += ( fmsg3 == "" ) ? "*Exclude*" : "Exclude*" ;
	}

	if ( pflscmd == "ON" )
	{
		fmsg4 = "*Show*" ;
	}

	if ( sort_parm != "(ENTRY,C,A)" )
	{
		fmsg4 += ( fmsg4 == "" ) ? "*Sort*" : "Sort*" ;
	}

	if ( recursv )
	{
		fmsg4 += ( fmsg4 == "" ) ? "*Expand*" : "Expand*" ;
	}

	if ( !stats )
	{
		fmsg4 += ( fmsg4 == "" ) ? "*Nostats*" : "Nostats*" ;
	}
}


void pflst0a::set_search( const string& s )
{
	if ( s == "" || find( search.begin(), search.end(), s ) != search.end() )
	{
		return ;
	}
	search.push_back( s ) ;
}


void pflst0a::clear_search()
{
	search.clear() ;
}


void pflst0a::rename_entry( const string& entry1,
			    bool& terminat )
{
	int result ;

	bool found ;

	string entry2 ;

	string msg      = "" ;
	string zcmd1    = "" ;
	string newentry = "" ;

	vdefine( "NEWENTRY ZCMD1", &newentry, &zcmd1 ) ;

	while ( true )
	{
		display( "PFLST0A4", msg, "ZCMD1" ) ;
		if ( RC == 8 )
		{
			setmsg( "FLST011S" )  ;
			message = "Cancelled" ;
			tbput( dslist ) ;
			break ;
		}
		msg = "" ;
		if ( newentry.front() != '/' )
		{
			entry2 = substr( entry1, 1, lastpos( "/", entry1 ) ) + newentry ;
		}
		else
		{
			entry2 = newentry ;
		}
		try
		{
			found = exists( entry2 ) ;
		}
		catch ( const filesystem_error& ex )
		{
			msg = "FLST011R" ;
			rsn = ex.what()  ;
			continue ;
		}
		catch (...)
		{
			msg = "FLST011R" ;
			rsn = "Unknown error" ;
			continue ;
		}
		if ( found )
		{
			setmsg( "FLST012Q" ) ;
			continue ;
		}
		try
		{
			result = rename( entry1.c_str(), entry2.c_str() ) ;
		}
		catch ( const filesystem_error& ex )
		{
			msg = "FLST011R" ;
			rsn = ex.what()  ;
			continue ;
		}
		catch (...)
		{
			rsn = "Unknown error" ;
			continue ;
		}
		if ( result == 0 )
		{
			setmsg( "FLST011Q" ) ;
			rebuild1 = true  ;
			break ;
		}
		if ( errno == EXDEV )
		{
			msg = "FLST011Z" ;
		}
		else
		{
			msg = "FLST011R" ;
			rsn = strerror( errno ) ;
		}
	}

	if ( zcmd1 == "QUIT" )
	{
		setmsg( "FLST012V" ) ;
		message = "Terminated" ;
		tbput( dslist ) ;
		terminat = true ;
	}

	vdelete( "NEWENTRY ZCMD1" ) ;
}


int pflst0a::copy_entry( const string& entry1,
			 bool& terminat,
			 bool  c_copy )
{
	int rc = 0 ;

	bool errs ;
	bool found ;
	bool skipped ;

	bool e1_is_file = false ;
	bool e1_is_dir  = false ;
	bool e2_is_file = false ;
	bool e2_is_dir  = false ;

	string msg      = "" ;
	string zcmd1    = "" ;
	string prattrs  = "" ;
	string frepl    = "" ;
	string dirrec   = "" ;

	string newentry ;
	string entry2 ;
	string panel ;

	const string vlist = "PRATTRS FREPL DIRREC NEWENTRY ZCMD1" ;

	boost::system::error_code ec ;

	vdefine( vlist, &prattrs, &frepl, &dirrec, &newentry, &zcmd1 ) ;

	panel    = ( c_copy ) ? "PFLST0AQ" : "PFLST0A5" ;
	newentry = ( c_copy ) ? ccpath : entry ;

	while ( true )
	{
		if ( c_copy && msg != "" )
		{
			setmsg( msg ) ;
			break ;
		}
		if ( newentry != "" && newentry.front() != '/' && filepre != "" )
		{
			newentry = filepre + newentry ;
		}
		if ( !c_copy || ccpath == "" )
		{
			display( panel, msg ) ;
			msg = "" ;
			if ( RC == 8 )
			{
				setmsg( "FLST011W" )  ;
				message = "Cancelled" ;
				tbput( dslist ) ;
				rc = 8 ;
				break ;
			}
		}
		else
		{
			vget( "PRATTRS FREPL DIRREC", SHARED ) ;
		}
		if ( c_copy )
		{
			entry2 = full_name( newentry, file_name( entry1 ) ) ;
		}
		else if ( newentry.front() != '/' )
		{
			entry2 = substr( entry1, 1, lastpos( "/", entry1 ) ) + newentry ;
		}
		else
		{
			filepre = substr( newentry, 1, lastpos( "/", newentry ) ) ;
			entry2  = newentry ;
		}
		try
		{
			e1_is_file = is_regular_file( entry1 ) ;
			if ( !e1_is_file )
			{
				e1_is_dir = is_directory( entry1 ) ;
			}
			found = exists( entry2 ) ;
			if ( found )
			{
				e2_is_file = is_regular_file( entry2 ) ;
				if ( !e2_is_file )
				{
					e2_is_dir = is_directory( entry2 ) ;
				}
			}
		}
		catch ( const filesystem_error& ex )
		{
			msg = "FLST011V" ;
			rsn = ex.what()  ;
			continue ;
		}
		catch (...)
		{
			msg = "FLST011V" ;
			rsn = "Unknown error" ;
			continue ;
		}
		if ( ( e1_is_dir && e2_is_dir ) )
		{
			if ( boost::filesystem::equivalent( entry1, entry2 ) )
			{
				msg     = "FLST013" ;
				message = "Same entry" ;
				tbput( dslist ) ;
				continue ;
			}
		}
		if ( ( e1_is_file && e2_is_dir ) ||
		     ( e1_is_dir  && e2_is_dir ) )
		{
			entry2 = full_name( entry2, entry1.substr( entry1.find_last_of( '/' ) + 1 ) ) ;
			found  = exists( entry2 ) ;
		}
		else if ( found )
		{
			if ( !e2_is_file && !e2_is_dir )
			{
				msg = "FLST011Y" ;
				continue ;
			}
		}
		if ( found && frepl != "/" )
		{
			msg     = "FLST011X" ;
			message = "Skipped" ;
			tbput( dslist ) ;
			continue ;
		}
		if ( e1_is_dir )
		{
			errs        = false ;
			skipped     = false ;
			interrupted = false ;
			if ( boost::filesystem::equivalent( entry1, entry2 ) )
			{
				msg     = "FLST013" ;
				message = "Same entry" ;
				tbput( dslist ) ;
				continue ;
			}
			copy_directory( entry1,
					entry2,
					( dirrec == "/" ),
					( frepl == "/" ),
					( prattrs == "/" ),
					skipped,
					errs ) ;
			if ( interrupted )
			{
				terminat = true ;
				message  = "Interrupted (SIGUSR1)" ;
				setmsg( "FLST013E" ) ;
				tbput( dslist ) ;
				vdelete( vlist ) ;
				return rc ;
			}
			if ( errs )
			{
				msg     = "FLST012G" ;
				message = "Errors" ;
				tbput( dslist ) ;
			}
			else if ( skipped )
			{
				setmsg( "FLST013F" ) ;
				message = "Partial copy" ;
				tbput( dslist ) ;
			}
			else
			{
				setmsg( "FLST011U" ) ;
				message = "Copied" ;
				tbput( dslist ) ;
			}
			break ;
		}
		else if ( e1_is_file )
		{
			if ( boost::filesystem::equivalent( entry1, entry2 ) )
			{
				msg     = "FLST013" ;
				message = "Same entry" ;
				tbput( dslist ) ;
				continue ;
			}
			copy_file( entry1, entry2, copy_options::overwrite_existing, ec ) ;
		}
		else if ( is_symlink( entry1 ) )
		{
			if ( boost::filesystem::equivalent( entry1, entry2 ) )
			{
				msg     = "FLST013" ;
				message = "Same entry" ;
				tbput( dslist ) ;
				continue ;
			}
			copy_symlink( entry1, entry2, ec ) ;
		}
		if ( ec.value() == boost::system::errc::success )
		{
			if ( prattrs == "/" )
			{
				copy_file_attributes( entry1, entry2 ) ;
			}
			setmsg( "FLST011U" ) ;
			message = "Copied" ;
			tbput( dslist ) ;
			break ;
		}
		else
		{
			msg = "FLST011V" ;
			rsn = ec.message() ;
		}
	}

	if ( zcmd1 == "QUIT" )
	{
		setmsg( "FLST012V" ) ;
		message = "Terminated" ;
		tbput( dslist ) ;
		terminat = true ;
	}

	vdelete( vlist ) ;

	return rc ;
}


void pflst0a::copy_directory( const string& src,
			      const string& dest,
			      bool  recursive,
			      bool  frepl,
			      bool  prattrs,
			      bool& skipped,
			      bool& errs )
{
	//
	// Copy directory src to dest.  Create dest (as src) if it does not exist.
	//
	// recursive - recursive copy.
	// frepl     - replace file if it already exists.
	// prattrs   - copy attributes.
	//

	string lsrc ;
	string ldest ;

	boost::system::error_code ec ;

	if ( interrupted )
	{
		return ;
	}

	if ( !exists( dest ) )
	{
		create_directory( dest, src, ec ) ;
		if ( ec.value() != boost::system::errc::success )
		{
			errs    = true         ;
			msg     = "FLST011V"   ;
			rsn     = ec.message() ;
			message = ec.message() ;
			llog( "E", "Copy of directory " + src + " to " + dest + " failed." << endl ) ;
			llog( "E", ec.message() << endl ) ;
			return ;
		}
	}

	if ( recursive && is_subpath( canonical( filesystem::path( dest ) ), canonical( filesystem::path( src  ) ) ) )
	{
		llog( "W", "Cannot recursively copy a directory, "<< src << ", into itself, "<< dest <<endl ) ;
		skipped = true ;
		return ;
	}


	for ( directory_iterator file( src, ec ) ; file != directory_iterator() ; ++file )
	{
		if ( interrupted )
		{
			setmsg( "FLST013E" ) ;
			break ;
		}
		if ( ec.value() != boost::system::errc::success )
		{
			errs = true ;
			llog( "E", "Listing directory " << src << " failed." << endl ) ;
			llog( "E", ec.message() << endl ) ;
			return ;
		}
		path current( file->path() ) ;
		lsrc  = current.string() ;
		ldest = dest + "/" + current.filename().string() ;
		if ( is_directory( current ) )
		{
			if ( recursive )
			{
				copy_directory( lsrc,
						ldest,
						recursive,
						frepl,
						prattrs,
						skipped,
						errs ) ;
			}
			else
			{
				if ( !exists( ldest ) )
				{
					create_directory( ldest, lsrc, ec ) ;
					if ( ec.value() != boost::system::errc::success )
					{
						errs = true ;
						llog( "E", "Copy of directory " << current.filename() <<
							   " failed." << endl ) ;
						llog( "E", ec.message() << endl ) ;
					}
				}
				if ( prattrs )
				{
					copy_file_attributes( lsrc, ldest ) ;
				}
			}
			continue ;
		}
		if ( !frepl && exists( ldest ) )
		{
			skipped = true ;
			continue ;
		}
		if ( is_regular_file( current ) )
		{
			copy_file( lsrc, ldest, copy_options::overwrite_existing, ec ) ;
			if ( ec.value() != boost::system::errc::success )
			{
				errs = true ;
				llog( "E", "Copy of file " << current.filename() << " failed." << endl ) ;
				llog( "E", ec.message() << endl ) ;
			}
			else if ( prattrs )
			{
				copy_file_attributes( lsrc, ldest ) ;
			}
		}
		else if ( is_symlink( current ) )
		{
			copy_symlink( lsrc, ldest, ec ) ;
			if ( ec.value() != boost::system::errc::success )
			{
				errs = true ;
				llog( "E", "Copy of symlink " << current.filename() << " failed." << endl ) ;
				llog( "E", ec.message() << endl ) ;
			}
			else if ( prattrs )
			{
				copy_file_attributes( lsrc, ldest ) ;
			}
		}
		else
		{
			errs = true ;
			llog( "E", "Ignoring entry " << current.filename() << endl ) ;
			llog( "E", "Not a regular file, directory or symlink" << endl ) ;
		}
	}

	if ( prattrs )
	{
		copy_file_attributes( src, dest ) ;
	}
}


void pflst0a::copy_file_attributes( const string& file1,
				    const string& file2 )
{
	//
	// Copy mode, ownership, timestamps and xattr from file1 to file2.
	//

	ssize_t buflen ;
	ssize_t vallen ;
	ssize_t keylen ;

	int flags = 0 ;

	char* buf ;
	char* key ;
	char* val ;

	const char* fl1 = file1.c_str() ;
	const char* fl2 = file2.c_str() ;

	struct utimbuf newtime ;

	struct passwd* pwd ;
	struct group*  grp ;

	struct stat fl1_stats ;

	uid_t uid ;
	gid_t gid ;

	if ( lstat( fl1, &fl1_stats ) == 0 )
	{
		newtime.modtime = fl1_stats.st_mtime;
		newtime.actime  = fl1_stats.st_atime;
		utime( fl2, &newtime ) ;
		pwd = getpwuid( fl1_stats.st_uid ) ;
		if ( pwd )
		{
			uid = pwd->pw_uid ;
			chown( fl2, uid, -1 ) ;
		}
		grp = getgrgid( fl1_stats.st_gid ) ;
		if ( grp )
		{
			gid = grp->gr_gid ;
			chown( fl2, -1, gid ) ;
		}
		chmod( fl2, fl1_stats.st_mode ) ;
	}


	buflen = listxattr( fl1, nullptr, 0 ) ;
	if ( buflen > 0 )
	{
		buf    = new char[ buflen ] ;
		key    = buf ;
		buflen = listxattr( fl1, key, buflen ) ;
		while ( buflen > 0 )
		{
			vallen = getxattr( fl1, key, nullptr, 0 ) ;
			if ( vallen >= 0 )
			{
				val    = new char[ vallen ] ;
				vallen = getxattr( fl1, key, val, vallen ) ;
				if ( vallen >= 0 )
				{
					setxattr( fl2, key, val, vallen, flags ) ;
				}
				delete[] val ;
			}
			keylen = strlen( key ) + 1 ;
			key    = key + keylen ;
			buflen = buflen - keylen ;
		}
		delete[] buf ;
	}
}


void pflst0a::update_reflist( const string& e )
{
	if ( get_dialogue_var( "ZRFURL" ) == "YES" && pfluref == "/" )
	{
		control( "ERRORS", "RETURN" ) ;
		submit( "PGM("+ get_dialogue_var( "ZRFLPGM" ) +") PARM(PLR "+ e +")" ) ;
		control( "ERRORS", "CANCEL" ) ;
	}
}


bool pflst0a::is_subpath( const path& p1,
			  const path& p2 )
{
	//
	// Return true if p2 is a subpath of p1.
	//

	for ( auto base = p1.begin(), sub = p2.begin() ; sub != p2.end() ; ++base, ++sub )
	{
		if ( base == p1.end() || *sub != *base )
		{
			return false ;
		}
	}

	return true ;
}


bool pflst0a::is_subpath( const path& p1,
			  const path& p2,
			  int& match )
{
	//
	// Return true if p2 is a subpath of p1.  Also return the number of directories that match.
	//

	match = 0 ;
	for ( auto base = p1.begin(), sub = p2.begin() ; sub != p2.end() ; ++base, ++sub )
	{
		if ( base == p1.end() || *sub != *base )
		{
			return false ;
		}
		++match ;
	}

	return true ;
}


string pflst0a::get_target( const string& l )
{
	//
	// Return the full pathname of a link target, even if it is a relative link.
	//

	int rc  ;

	string lp ;

	char* buffer ;
	size_t bufferSize = 255 ;

	while ( true )
	{
		buffer = new char[ bufferSize ] ;
		rc     = readlink( l.c_str(), buffer, bufferSize ) ;
		if ( rc == -1 )
		{
			delete[] buffer ;
			if ( errno == ENAMETOOLONG )
			{
				bufferSize += 255;
			}
			else
			{
				break ;
			}
		}
		else
		{
			lp = string( buffer, rc ) ;
			delete[] buffer ;
			if ( lp.front() != '/' )
			{
				lp = add_path( l, lp ) ;
			}
			break ;
		}
	}

	return lp ;
}


string pflst0a::add_path( const string& a,
			  const string& b )
{
	//
	// Add path of entry 'a' to entry 'b'.
	//

	return ( a.substr( 0, a.find_last_of( '/' ) + 1 ) + b ) ;
}


string pflst0a::full_name( const string& a,
			   const string& b )
{
	return ( a.back() == '/' ) ? a + b : a + "/" + b ;
}


string pflst0a::full_dir( const string& a )
{
	return ( a.back() == '/' ) ? a : a + "/" ;
}


string pflst0a::file_name( const string& a )
{
	size_t p = a.find_last_of( '/' ) ;

	return ( p == string::npos ) ? a : a.substr( p + 1 ) ;
}


string pflst0a::file_directory( const string& a )
{
	size_t p = a.find_last_of( '/' ) ;

	return ( p == string::npos ) ? a : a.substr( 0, p ) ;
}


const string& pflst0a::get_truename( const string& w )
{
	auto it = aliasNames.find( w ) ;

	return ( it == aliasNames.end() ) ? w : it->second ;
}


void pflst0a::log_filesystem_error( const string& p )
{
	if ( log_error == "ON" )
	{
		llog( "W", "File "+ p +" cannot be read." << endl ) ;
	}
}


void pflst0a::log_filesystem_error( const filesystem_error& ex )
{
	if ( log_error == "ON" )
	{
		llog( "W", ex.what() << endl ) ;
	}
}


void pflst0a::log_filesystem_error( const boost::system::error_code& ec,
				    const string& p )
{
	if ( log_error == "ON" )
	{
		llog( "W", "File "+ p +" - "+ ec.message() << endl ) ;
	}
}


void pflst0a::set_scrname( const string& name )
{
	vget( "ZSCRNAME", SHARED ) ;
	scrnames.push( zscrname ) ;

	zscrname = name ;
	vput( "ZSCRNAME", SHARED ) ;
}


void pflst0a::restore_scrname()
{
	if ( scrnames.empty() ) { return ; }

	zscrname = scrnames.top() ;
	vput( "ZSCRNAME", SHARED ) ;

	scrnames.pop() ;
}


string pflst0a::get_dialogue_var( const string& v )
{
	string t ;

	vcopy( v, t ) ;

	return t ;
}


void pflst0a::set_dialogue_var( const string& var,
				const string& val )
{
	vreplace( var, val ) ;
}


string pflst0a::get_profile_var( const string& var,
				 const string& def )
{
	//
	// VGET profile variable.  If it does not exist, set the default.
	//

	string temp ;

	vdefine( var, &temp ) ;
	vget( var, PROFILE ) ;
	if ( RC == 8 && def != "" )
	{
		temp = def ;
		vput( var, PROFILE ) ;
	}
	vdelete( var ) ;

	return temp ;
}


void pflst0a::put_profile_var( const string& var,
			       const string& val )
{
	//
	// VPUT profile variable.
	//

	string temp = val ;

	vdefine( var, &temp ) ;
	vput( var, PROFILE ) ;
	vdelete( var ) ;
}


string pflst0a::qstring( int& rc,
			 const string& s )
{
	//
	// Remove quotes from string s.  Quotes can be either single or double.
	//
	// If single quotes, call qstring1, that removes quotes and also treats two single quotes
	// as a single quote that is part of the string.
	//
	//   Return Code:
	//    0  Normal completion
	//    4  Multiple non-quoted words in string
	//   12  Severe error
	//

	rc = 0 ;

	size_t i = 0 ;

	if ( s.size() == 0 )
	{
		return "" ;
	}

	if ( s.front() == '\'' )
	{
		return qstring1( rc, s ) ;
	}

	if ( s.front() == '"' )
	{
		i = s.find( '"', 1 ) ;
		if ( i == string::npos || s.back() != '"' )
		{
			rc = 12 ;
			return "" ;
		}
		return s.substr( 1, i-1 ) ;
	}

	if ( words( s ) > 1 )
	{
		rc = 4 ;
	}

	return s ;
}


string pflst0a::qstring1( int& rc,
			  const string& s )
{
	//
	// Remove quotes from string s.  Quotes are single.
	//
	// Two quotes within the string are reduced to one quote.
	//
	//   Return Code:
	//    0  Normal completion
	//   12  Severe error
	//

	rc = 0 ;

	int i ;
	int quotes ;

	string r ;

	string::const_iterator it ;

	it = s.begin() ;

	if ( s.size() == 0 )
	{
		return "" ;
	}

	if ( s.size() == 1 || s.back() != '\'' )
	{
		rc = 12 ;
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
			rc = 12 ;
			return r ;
		}
		else if ( i == 0 && it == s.end() )
		{
			rc = 12 ;
			return r ;
		}
	}

	return r ;
}
