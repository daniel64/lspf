/*  Compile with ::                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libpdiff0a.so -o libpdiff0a.so pdiff0a.cpp */

/*
  Copyright (c) 2020 Daniel John Erdos

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

/**********************************************************************************/
/*                                                                                */
/* Compare files or directories.                                                  */
/* Calls diff to do the real work.                                                */
/*                                                                                */
/**********************************************************************************/


#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <utime.h>
#include <pwd.h>
#include <grp.h>

#include "../lspfall.h"
#include "pdiff0a.h"

using namespace std ;
using namespace boost ;
using namespace boost::filesystem ;

LSPF_APP_MAKER( pdiff0a )


pdiff0a::pdiff0a()
{
	STANDARD_HEADER( "Compare files and directories", "1.0.0" )

	patch_file = "diff -u %s %s 2>/dev/null" ;
	patch_dir  = "diff -Naur %s %s %s 2>/dev/null" ;

	const string vlist1 = "ENTRYA ENTRYB SHOWI SHOWA SHOWB RECUR1" ;
	const string vlist2 = "CMPINS CMPIGD CMPIGB CMPITS CMPIGT EXCLPATT IGNREGEX PROCOPTS" ;
	const string vlist3 = "SEL ENTRY INA INB MOD ISD MDATEA SIZEA MDATEB SIZEB" ;
	const string vlist4 = "DFFUTT DFCNTX ZSTR1 ZUSER ZVERB ZSCREEN" ;
	const string vlist5 = "ISFILE1 ISDIR1 MDATEAO MDATEBO CDIR LIT1 LIT2 LIT3" ;
	const string vlist6 = "F1LEN F2COL F2LEN F3COL F3LEN F4COL F4LEN" ;
	const string vlist7 = "F5LEN F6COL F6LEN F7COL F7LEN F8COL F8LEN F9COL F9LEN" ;
	const string vlist8 = "ZCURINX ZTDTOP ZTDSELS" ;
	const string vlist9 = "CRP ZSCREEND ZSCREENW" ;

	vdefine( vlist1, &entrya, &entryb, &showi, &showa, &showb, &recur1 ) ;
	vdefine( vlist2, &cmpins, &cmpigd, &cmpigb, &cmpits, &cmpigt, &exclpatt, &ignregex, &procopts ) ;
	vdefine( vlist3, &sel, &entry, &inA, &inB, &mod, &isd, &mdatea, &sizea, &mdateb, &sizeb ) ;
	vdefine( vlist4, &dffutt, &dfcntx, &zstr1, &zuser, &zverb, &zscreen ) ;
	vdefine( vlist5, &isfile1, &isdir1, &mdateao, &mdatebo, &cdir, &lit1, &lit2, &lit3 ) ;
	vdefine( vlist6, &f1len, &f2col, &f2len, &f3col, &f3len, &f4col, &f4len ) ;
	vdefine( vlist7, &f5len, &f6col, &f6len, &f7col, &f7len, &f8col, &f8len, &f9col, &f9len ) ;
	vdefine( vlist8, &zcurinx, &ztdtop, &ztdsels ) ;
	vdefine( vlist9, &crp, &zscreend, &zscreenw ) ;
}


void pdiff0a::application()
{
	string zcmd ;

	const string vlist = "ZCMD" ;

	char buf[ 2048 ] ;

	initialise() ;

	vdefine( vlist, &zcmd ) ;

	while ( true )
	{
		display( "PDIFF0A1" ) ;
		if ( RC == 8 ) { break ; }
		if ( isfile1 == "1" )
		{
			if ( zcmd == "/" )
			{
				entry = entrya.substr( entrya.find_last_of( '/' ) + 1 ) ;
				addpop( "", 2, 5 ) ;
				display( "PDIFF0A5" ) ;
				rempop() ;
				zcmd = "" ;
				if ( sel == "" ) { continue ; }
				if ( sel.front() == 'S' || sel.front() == 'V' )
				{
					compare_files( entrya,
						       entryb,
						       entry,
						       sel.front() == 'V' ) ;
				}
				else if ( sel == "EA" )
				{
					edit_file( entrya ) ;
				}
				else if ( sel == "EB" )
				{
					edit_file( entryb ) ;
				}
				else if ( sel == "CA" )
				{
					cdir = "(A->B)" ;
					copy_entry( entrya,
						    entryb ) ;
				}
				else if ( sel == "CB" || sel == "C" )
				{
					cdir = "(B->A)" ;
					copy_entry( entryb,
						    entrya ) ;
				}
				else if ( sel == "MA" || sel == "M" )
				{
					merge_files( entrya,
						     entryb ) ;
				}
				else if ( sel == "MB" )
				{
					merge_files( entryb,
						     entrya ) ;
				}
				else if ( sel == "P" )
				{
					snprintf( buf,
						  sizeof( buf ),
						  patch_file,
						  entrya.c_str(),
						  entryb.c_str() ) ;
					create_patch( buf, entry ) ;
				}
				sel = "" ;
			}
			else
			{
				compare_files( entrya, entryb ) ;
			}
		}
		else
		{
			compare_dirs( dir( entrya ), dir( entryb ) ) ;
		}
	}

	vdelete( vlist ) ;
}


void pdiff0a::initialise()
{
	vget( "ZUSER ZSCREEN ZSCREENW ZSCREEND", SHARED ) ;
}


void pdiff0a::compare_dirs( const string& dir1,
			    const string& dir2 )
{
	int rc ;
	int sRC ;

	int csrrow = 0 ;
	int crpx   = 0 ;
	int ppos   = 0 ;

	bool cur2sel = false ;

	const string vlist1 = "ZCMD" ;

	string msg ;
	string zcmd ;
	string panl ;
	string temp ;
	string cursor ;
	string msgloc ;
	string excl ;
	string opts ;

	string autosel = "YES" ;

	char buf[ 2048 ] ;

	if ( dir1 == dir2 )
	{
		setmsg( "DIFF011J" ) ;
		return ;
	}

	vdefine( vlist1, &zcmd ) ;

	opts  = "-q -s" ;
	opts += ( exclpatt == "" ) ? "" : " -x '"+ exclpatt +"'" ;
	opts += ( ignregex == "" ) ? "" : " -I '"+ ignregex +"'" ;

	string cmd = create_diff_cmd( dir1, dir2, opts ) ;

	vector<string> results ;
	set<entry_info> outl ;

	execute_cmd( rc, cmd, results ) ;
	if ( rc == 0 )
	{
		setmsg( "DIFF011O" ) ;
		vdelete( vlist1 ) ;
		return ;
	}
	else if ( rc != 1 )
	{
		llog( "E", "Diff return code = "<< rc <<endl) ;
		llog( "E", "Failing command: "<< cmd <<endl) ;
		setmsg( "DIFF011L" ) ;
		vdelete( vlist1 ) ;
		return ;
	}

	parse_diff_output( dir1, dir2, results, outl ) ;

	if ( outl.size() == 0 )
	{
		setmsg( "DIFF011O" ) ;
		vdelete( vlist1 ) ;
		return ;
	}

	table = "DFF" + d2ds( taskid(), 5 ) ;

	create_table( outl ) ;

	msg     = "" ;
	cursor  = "" ;
	msgloc  = "" ;
	ztdtop  = 0  ;
	ztdsels = 0  ;
	zcurinx = 0  ;

	vget( "PPOS", SHARED ) ;
	if ( RC == 0 )
	{
		vcopy( "PPOS", temp ) ;
		verase( "PPOS", SHARED ) ;
		ppos = ds2d( temp ) ;
	}

	while ( true )
	{
		if ( msg == "" && ztdsels > 0 )
		{
			--ztdsels ;
		}
		if ( ztdsels == 0 )
		{
			tbtop( table ) ;
			tbskip( table, ztdtop ) ;
			panl = ( ppos == 0 ) ? "PDIFF0A2" :
			       ( ppos == 1 ) ? "PDIFF0A3" : "PDIFF0A4" ;
		}
		else
		{
			panl = "" ;
		}
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
		control( "PASSTHRU", "LRSCROLL", "PASON" ) ;
		tbdispl( table,
			 panl,
			 msg,
			 cursor,
			 csrrow,
			 1,
			 autosel,
			 "CRP",
			 msgloc ) ;
		sRC = RC ;
		control( "PASSTHRU", "LRSCROLL", "PASOFF" ) ;
		if ( sRC == 8 ) { break ; }
		if ( zcmd == "REFRESH" )
		{
			set_shared_var( "PPOS", ppos ) ;
			control( "NONDISPL", "ENTER" ) ;
			break ;
		}
		if ( is_term_resized( zscreend+2, zscreenw ) )
		{
			vget( "ZSCREENW ZSCREEND", SHARED ) ;
		}
		msg     = "" ;
		cursor  = "" ;
		msgloc  = "" ;
		csrrow  = 0  ;
		crpx    = crp ;
		autosel = "YES" ;
		if ( zcmd == "PATCH" )
		{
			excl = ( exclpatt == "" ) ? "" : "-x '"+ exclpatt +"'" ;
			zcmd = "" ;
			snprintf( buf,
				  sizeof( buf ),
				  patch_dir,
				  entrya.c_str(),
				  entryb.c_str(),
				  excl.c_str() ) ;
			create_patch( buf, "*ALL*" ) ;
		}
		else if ( zcmd != "" )
		{
			tbsort( table, "(" + zcmd + ",C,D)" ) ;
			zcmd = "" ;
			continue ;
		}
		vget( "ZVERB", SHARED ) ;
		if ( zverb == "LEFT" && ppos > 0 )
		{
			if ( ppos == 2 && zscreenw < 120 ) { --ppos ; }
			--ppos ;
			continue ;
		}
		else if ( zverb == "RIGHT" && ppos < 2 )
		{
			if ( ppos == 0 && zscreenw < 120 ) { ++ppos ; }
			++ppos ;
			continue ;
		}
		if ( ztdsels == 0 && zcurinx > 0 )
		{
			tbtop( table ) ;
			tbskip( table, zcurinx ) ;
			ztdsels = 1 ;
			sel     = "S" ;
			crpx    = zcurinx ;
		}
		if ( sel != "" )
		{
			cur2sel = true ;
		}
		if ( sel == "/" )
		{
			addpop( "", 2, 5 ) ;
			display( "PDIFF0A5" ) ;
			rempop() ;
		}
		if ( sel == "" ) { continue ; }
		if ( ( sel == "EA" && inA == "Y" ) ||
		     ( sel == "EB" && inB == "Y" ) ||
		     ( sel == "CA" && inA == "Y" ) ||
		     ( sel == "C"  && inB == "Y" ) ||
		     ( sel == "CB" && inB == "Y" ) )
		{
		}
		else if ( inA != "Y" || inB != "Y" || isd == "Y" )
		{
			msg    = "DIFF011C" ;
			cursor = "SEL" ;
			msgloc = "SEL" ;
			csrrow = crp ;
			sel    = "" ;
			continue ;
		}
		if ( mod == "N" && findword( sel.front(), "S V C M" ) )
		{
			msg    = "DIFF011N" ;
			cursor = "SEL" ;
			msgloc = "SEL" ;
			csrrow = crp ;
			continue ;
		}
		if ( sel.front() == 'S' || sel.front() == 'V' )
		{
			compare_files( full_name( entrya, entry ),
				       full_name( entryb, entry ),
				       entry,
				       sel.front() == 'V' ) ;
		}
		else if ( sel == "EA" )
		{
			edit_file( full_name( entrya, entry ) ) ;
		}
		else if ( sel == "EB" )
		{
			edit_file( full_name( entryb, entry ) ) ;
		}
		else if ( sel == "CA" )
		{
			cdir = "(A->B)" ;
			copy_entry( full_name( entrya, entry ),
				    full_name( entryb, entry ) ) ;
		}
		else if ( sel == "CB" || sel == "C" )
		{
			cdir = "(B->A)" ;
			copy_entry( full_name( entryb, entry ),
				    full_name( entrya, entry ) ) ;
		}
		else if ( sel == "MA" || sel == "M" )
		{
			merge_files( full_name( entrya, entry ),
				     full_name( entryb, entry ) ) ;
		}
		else if ( sel == "MB" )
		{
			merge_files( full_name( entryb, entry ),
				     full_name( entrya, entry ) ) ;
		}
		else if ( sel == "P" )
		{
			snprintf( buf,
				  sizeof( buf ),
				  patch_file,
				  full_name( entrya, entry ).c_str(),
				  full_name( entryb, entry ).c_str() ) ;
			create_patch( buf, entry ) ;
		}
		sel = "" ;
	}
	tbend( table ) ;

	vdelete( vlist1 ) ;
}


void pdiff0a::create_patch( const char* cmd,
			    string entry )
{
	int rc ;

	string str1 ;
	string line ;
	string pfile ;
	string ffile ;
	string rafile ;
	string differr ;

	const string vlist = "ENTRY PFILE FFILE RAFILE DIFFERR STR1" ;

	boost::system::error_code ec ;

	std::ifstream fin ;
	std::ofstream fout ;

	string tfile = create_tempname() ;

	execute_cmd( rc, cmd, tfile ) ;
	if ( rc == 0 )
	{
		setmsg( "DIFF011N" ) ;
		remove( tfile ) ;
		return ;
	}
	else if ( rc != 1 )
	{
		llog( "E", "Diff return code = "<< rc <<endl) ;
		llog( "E", "Failing command: "<< cmd <<endl) ;
		setmsg( "DIFF011M" ) ;
		remove( tfile ) ;
		return ;
	}

	vdefine( vlist, &entry, &pfile, &ffile, &rafile, &differr, &str1 ) ;

	addpop( "", 2, 5 ) ;
	while ( true )
	{
		display( "PDIFF0A7" ) ;
		if ( RC == 8 )
		{
			setmsg( "DIFF011P" ) ;
			break ;
		}
		ffile = ( pfile.front() == '/' ) ? pfile : full_name( get_shared_var( "ZHOME" ), pfile ) ;
		if ( rafile == "1" )
		{
			copy_file( tfile, ffile, copy_options::overwrite_existing, ec ) ;
			if ( ec.value() == boost::system::errc::success )
			{
				setmsg( "DIFF011Q" ) ;
				break ;
			}
			else
			{
				differr = ec.message() ;
				setmsg( "DIFF011R" ) ;
				continue ;
			}
		}
		else
		{
			fout.open( ffile, std::ios::app ) ;
			if ( fout.fail() )
			{
				differr = strerror( errno ) ;
				setmsg( "DIFF011R" ) ;
				continue ;
			}
			fin.open( tfile ) ;
			if ( fin.fail() )
			{
				str1    = tfile ;
				differr = strerror( errno ) ;
				setmsg( "DIFF011S" ) ;
				continue ;
			}
			while ( getline( fin, line ) )
			{
				fout << line << endl ;
			}
			fin.close() ;
			fout.close() ;
			setmsg( "DIFF011Q" ) ;
		}
		break ;
	}

	rempop() ;
	remove( tfile ) ;
	vdelete( vlist ) ;
}


void pdiff0a::compare_files( const string& file1,
			     const string& file2,
			     const string& entry,
			     bool view_mode )
{
	int rc ;

	string cmd = create_diff_cmd( file1, file2, ( view_mode ) ? procopts : procopts + " --color=always -t" ) ;

	string lit = ( entry == "" ) ? file1 + " " + file2 :
				       "a/" + entry + " b/" + entry ;

	string tfile = create_tempname() ;

	execute_cmd( rc, cmd, tfile ) ;
	if ( rc == 0 )
	{
		setmsg( "DIFF011N" ) ;
		remove( tfile ) ;
		return ;
	}
	else if ( rc != 1 )
	{
		llog( "E", "Diff return code = "<< rc <<endl) ;
		llog( "E", "Failing command: "<< cmd <<endl) ;
		setmsg( "DIFF011M" ) ;
		remove( tfile ) ;
		return ;
	}

	if ( view_mode )
	{
		set_shared_var( "ZEDALT", "COMPARE: " + lit ) ;
		view( tfile ) ;
		verase( "ZEDALT", SHARED ) ;
	}
	else
	{
		set_shared_var( "ZBRALT", "COMPARE: " + lit ) ;
		control( "ERRORS", "RETURN" ) ;
		browse( tfile ) ;
		if ( isvalidName( ZRESULT ) )
		{
			setmsg( ZRESULT ) ;
		}
		control( "ERRORS", "CANCEL" ) ;
		verase( "ZBRALT", SHARED ) ;
	}

	remove( tfile ) ;
}


void pdiff0a::edit_file( const string& file1 )
{
	string emsg ;

	control( "ERRORS", "RETURN" ) ;

	edit( file1 ) ;
	if ( RC == 0 || RC == 4 || RC == 14 )
	{
		setmsg( ( ZRESULT != "" ) ? ZRESULT : "DIFF011E" ) ;
	}
	else if ( RC > 11 )
	{
		control( "ERRORS", "CANCEL" ) ;
		if ( isvalidName( ZRESULT ) )
		{
			uabend( ZRESULT ) ;
		}
		else
		{
			vcopy( "ZERRMSG", emsg ) ;
			if ( emsg != "" )
			{
				uabend( emsg ) ;
			}
		}
		uabend( "DIFF011T" ) ;
	}
	control( "ERRORS", "CANCEL" ) ;
}


void pdiff0a::copy_entry( const string& file1,
			  const string& file2 )
{
	struct stat results1 = get_lstat( file1 ) ;
	struct stat results2 = get_lstat( file2 ) ;

	string wmsg ;
	string entry1 = file1 ;
	string entry2 = file2 ;
	string mdate1 = moddate( results1 ) ;
	string mdate2 = moddate( results2 ) ;
	string fsize1 = filesiz( results1 ) ;
	string fsize2 = filesiz( results2 ) ;

	const string vlist = "ENTRY1 ENTRY2 MODDATE1 MODDATE2 FSIZE1 FSIZE2 WMSG" ;

	boost::system::error_code ec ;

	vdefine( vlist, &entry1, &entry2, &mdate1, &mdate2, &fsize1, &fsize2, &wmsg ) ;

	if ( moddate_greater( results2, results1 ) )
	{
		wmsg = "Warning:  File being copied is older that receiving file" ;
	}

	addpop( "", 2, 5 ) ;
	display( "PDIFF0A6" ) ;
	if ( RC == 8 )
	{
		setmsg( "DIFF011F" ) ;
		rempop() ;
		vdelete( vlist ) ;
		return ;
	}

	copy_file( file1, file2, copy_options::overwrite_existing, ec ) ;
	if ( ec.value() != boost::system::errc::success )
	{
		setmsg( "DIFF011G" ) ;
		zstr1 = ec.message() ;
		llog( "E", "Copy of file " << file1 << " to " << file2 << " failed." << endl ) ;
		llog( "E", ec.message() << endl ) ;
	}
	else
	{
		copy_file_attributes( results1, file1, file2 ) ;
		setmsg( "DIFF011K" ) ;
	}

	rempop() ;
	vdelete( vlist ) ;
}


void pdiff0a::merge_files( const string& file1,
			   const string& file2 )
{
	string emsg ;

	vreplace( "MDFILE", file2 ) ;

	control( "ERRORS", "RETURN" ) ;

	edit( file1,
	      "",
	      "mergemac",
	      "",
	      "",
	      "",
	      "",
	      "",
	      "MDFILE" ) ;
	if ( RC == 0 || RC == 4 || RC == 14 )
	{
		setmsg( ( ZRESULT != "" ) ? ZRESULT : "DIFF011E" ) ;
	}
	else if ( RC > 11 )
	{
		control( "ERRORS", "CANCEL" ) ;
		if ( isvalidName( ZRESULT ) )
		{
			uabend( ZRESULT ) ;
		}
		else
		{
			vcopy( "ZERRMSG", emsg ) ;
			if ( emsg != "" )
			{
				uabend( emsg ) ;
			}
		}
		uabend( "DIFF011T" ) ;
	}
	control( "ERRORS", "CANCEL" ) ;
}


string pdiff0a::create_diff_cmd( const string& e1,
				 const string& e2,
				 const string& opts )
{
	string dparms ;
	string utt ;

	if ( cmpins  == "/" ) { dparms += " -i " ; }
	if ( cmpigd  == "/" ) { dparms += " -b " ; }
	if ( cmpigb  == "/" ) { dparms += " -B " ; }
	if ( cmpits  == "/" ) { dparms += " -Z " ; }
	if ( cmpigt  == "/" ) { dparms += " -E " ; }
	if ( recur1  == "/" ) { dparms += " -r " ; }

	utt = ( dfcntx != "" && ( dffutt == "-c" || dffutt == "-u" ) ) ? upper( dffutt ) + " " + dfcntx : dffutt ;

	return "diff " + dparms + " " + opts + " " + utt + " " + e1 + " " + e2 + " 2>/dev/null" ;

}


void pdiff0a::execute_cmd( int& rc,
			   const string& cmd,
			   vector<string>& results )
{
	string line ;

	char buffer[ 8192 ] ;

	results.clear() ;

	FILE* pipe { popen( cmd.c_str(), "r" ) } ;

	if ( !pipe )
	{
		rc = 3 ;
		setmsg( "DIFF011I" ) ;
		llog( "E", "POPEN failed.  Command string size="<< cmd.size() <<endl ) ;
		return ;
	}

	while ( fgets( buffer, sizeof( buffer ), pipe ) )
	{
		line = buffer ;
		if ( line != "" && line.back() == 0x0a )
		{
			line.pop_back() ;
		}
		results.push_back( line ) ;
	}

	rc = WEXITSTATUS( pclose( pipe ) ) ;
}


void pdiff0a::execute_cmd( int& rc,
			   const string& cmd,
			   const string& file )
{
	string line ;

	char buffer[ 8192 ] ;

	std::ofstream of ;

	FILE* pipe { popen( cmd.c_str(), "r" ) } ;

	if ( !pipe )
	{
		rc = 3 ;
		setmsg( "DIFF011I" ) ;
		llog( "E", "POPEN failed.  Command string size="<< cmd.size() <<endl ) ;
		return ;
	}

	of.open( file ) ;
	while ( fgets( buffer, sizeof( buffer ), pipe ) )
	{
		line = buffer ;
		if ( line != "" && line.back() == 0x0a )
		{
			line.pop_back() ;
		}
		of << line << endl ;
	}

	rc = WEXITSTATUS( pclose( pipe ) ) ;
	of.close() ;
}


void pdiff0a::create_table( set<entry_info>& outl )
{
	size_t maxl = 40 ;

	tbcreate( table,
		  "",
		  "(SEL,ENTRY,INA,INB,MOD,ISD,MDATEA,SIZEA,MDATEB,SIZEB,MDATEAO,MDATEBO)",
		  NOWRITE,
		  REPLACE ) ;

	for ( auto it = outl.begin() ; it != outl.end() ; ++it )
	{
		sel     = "" ;
		entry   = it->file ;
		inA     = ( it->inA ) ? "Y" : "N" ;
		inB     = ( it->inB ) ? "Y" : "N" ;
		mod     = (!it->inA || !it->inB || it->isd ) ? "" :
			  ( it->mod ) ? "Y" : "N" ;
		isd     = ( it->isd ) ? "Y" : "N" ;
		mdatea  = it->ma ;
		sizea   = it->sa ;
		mdateb  = it->mb ;
		sizeb   = it->sb ;
		mdateao = it->oa ;
		mdatebo = it->ob ;
		maxl    = max( maxl, min( entry.size(), size_t( zscreenw-22 ) ) ) ;
		tbadd( table ) ;
	}

	tbsort( table, "(ENTRY,C,A)" ) ;

	set_tb_variables( maxl ) ;
}


void pdiff0a::parse_diff_output( const string& dir1,
				 const string& dir2,
				 vector<string>& results,
				 set<entry_info>& outl )
{
	int ws ;

	size_t p1 ;

	string w1 ;
	string wl ;
	string f1 ;
	string d1 ;
	string d2 ;

	struct stat lstat_a ;
	struct stat lstat_b ;

	set<entry_info> in ;

	entry_info info ;

	for ( uint i = 0 ; i < results.size() ; ++i )
	{
		info.clear() ;
		string& line = results[ i ] ;
		w1 = word( line, 1 ) ;
		if ( w1 == "Only" )
		{
			ws = words( line ) ;
			f1 = word( line, ws ) ;
			d1 = subword( line, 3, ws-3 ) ;
			d1.pop_back() ;
			if ( d1 == dir1 )
			{
				info.file = f1 ;
				info.dira = d1 ;
				info.inA  = true  ;
				in.insert( info ) ;
			}
			else if ( d1 == dir2 )
			{
				info.file = f1 ;
				info.dirb = d1 ;
				info.inB  = true  ;
				in.insert( info ) ;
			}
			else if ( d1.compare( 0, dir1.size(), dir1 ) == 0 )
			{
				info.file = d1.substr( dir1.size() ) + "/" + f1 ;
				info.dira = d1 ;
				info.inA  = true  ;
				in.insert( info ) ;
			}
			else if ( d1.compare( 0, dir2.size(), dir2 ) == 0 )
			{
				info.file = d1.substr( dir2.size() ) + "/" + f1 ;
				info.dirb = d1 ;
				info.inB  = true  ;
				in.insert( info ) ;
			}
		}
		else if ( w1 == "Files" )
		{
			d1 = subword( line, 2 ) ;
			p1 = d1.find( " and " ) ;
			d1 = d1.substr( 0, p1 ) ;
			if ( !is_regular_file( d1 ) ) { continue ; }
			f1 = d1.substr( dir1.size() ) ;
			info.dira = d1.substr( 0, d1.find_last_of( '/' ) ) ;

			p1 = line.find( " and " ) ;
			d2 = line.substr( p1+5 ) ;
			ws = words( d2 ) ;
			wl = word( d2, ws ) ;
			d2 = ( wl == "differ" ) ? subword( d2, 1, ws-1 ) : subword( d2, 1, ws-2 ) ;
			if ( !is_regular_file( d2 ) ) { continue ; }
			info.file = f1 ;
			info.dirb = d2.substr( 0, d2.find_last_of( '/' ) ) ;
			info.inA  = true ;
			info.inB  = true ;
			info.mod  = ( wl == "differ" ) ? true : false ;
			in.insert( info ) ;
		}
		else if ( w1 == "Common" )
		{
			p1 = line.find_last_of( '/' ) ;
			f1 = line.substr( p1+1 ) ;
			info.file = f1 ;
			info.inA  = true ;
			info.inB  = true ;
			info.isd  = true ;
			in.insert( info ) ;
		}
	}

	for ( auto it = in.begin() ; it != in.end() ; ++it )
	{
		if ( ( it->inA && it->inB && it->mod ) ||
		     ( showi == "/" && it->inA && it->inB && !it->mod ) ||
		     ( showa == "/" && !it->inB ) ||
		     ( showb == "/" && !it->inA ) )
		{
			info = *it ;
			if ( !it->isd && it->inA )
			{
				lstat_a = get_lstat( full_name( it->dira, file_name( it->file ) ) ) ;
				info.ma = moddate( lstat_a ) ;
				info.oa = moddats( lstat_a ) ;
				info.sa = filesiz( lstat_a ) ;
			}
			if ( !it->isd && it->inB )
			{
				lstat_b = get_lstat( full_name( it->dirb, file_name( it->file ) ) ) ;
				info.mb = moddate( lstat_b ) ;
				info.ob = moddats( lstat_b ) ;
				info.sb = filesiz( lstat_b ) ;
			}
			outl.insert( info ) ;
		}
	}
}


string pdiff0a::full_name( const string& a,
			   const string& b )
{
	return ( a.back() == '/' ) ? a + b : a + "/" + b ;
}


string pdiff0a::dir( const string& a )
{
	return ( a.back() == '/' ) ? a : a + "/" ;
}


string pdiff0a::file_name( const string& a )
{
	size_t p = a.find_last_of( '/' ) ;

	return ( p == string::npos ) ? a : a.substr( p + 1 ) ;
}


struct stat pdiff0a::get_lstat( const string& f )
{
	struct stat results ;

	if ( lstat( f.c_str(), &results ) != 0 )
	{
		results.st_mtime = 0 ;
	}

	return results ;
}


string pdiff0a::moddate( const struct stat& results )
{
	struct tm* time_info ;

	char buf[ 20 ] ;

	if ( results.st_mtime == 0 )
	{
		return "" ;
	}

	time_info = gmtime( &results.st_mtime ) ;
	strftime( buf, sizeof( buf ), "%d/%m/%Y %H:%M:%S", time_info ) ;

	return buf ;
}


string pdiff0a::moddats( const struct stat& results )
{
	struct tm* time_info ;

	char buf[ 16 ] ;

	if ( results.st_mtime == 0 )
	{
		return "" ;
	}

	time_info = gmtime( &results.st_mtime ) ;
	strftime( buf, sizeof( buf ), "%Y%m%d%H%M%S", time_info ) ;

	return buf ;
}


string pdiff0a::filesiz( const struct stat& results )
{
	return ( results.st_mtime == 0 ) ? "" : to_string( results.st_size ) ;
}


bool pdiff0a::moddate_greater( const struct stat& results1,
			       const struct stat& results2 )
{
	return ( results1.st_mtime > results2.st_mtime ) ;
}


void pdiff0a::copy_file_attributes( const struct stat& fl1_stats,
				    const string& file1,
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

	uid_t uid ;
	gid_t gid ;

	if ( fl1_stats.st_mtime != 0 )
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


void pdiff0a::set_tb_variables( uint l )
{
	//
	// Set the table display column positions and lengths according the the largest entry displayed.
	//

	uint k = min( l, uint( zscreenw-79 ) ) ;

	f1len = l ;

	f2col = l + 7 ;
	f2len = 3 ;

	f3col = l + 11 ;
	f3len = 3 ;

	f4col = l + 15 ;
	f4len = 3 ;

	f5len = k ;

	f6col = k + 7 ;
	f6len = 20 ;

	f7col = k + 28 ;
	f7len = 15 ;

	f8col = k + 44 ;
	f8len = 20 ;

	f9col = k + 65 ;
	f9len = 15 ;

	lit1 = string( ( f2col - 47 ), ' ' ) + "A   B   Modified" ;
	lit2 = string( ( f6col - 47 ), ' ' ) + "A:                                   B:" ;
	lit3 = string( ( f6col - 47 ), ' ' ) + "Modified date        Size            Modified date        Size" ;
}


string pdiff0a::get_shared_var( const string& var )
{
	string r ;

	vget( var, SHARED ) ;
	vcopy( var, r ) ;

	return r ;
}


void pdiff0a::set_shared_var( const string& var,
			      const string& val )
{
	vreplace( var, val ) ;
	vput( var, SHARED ) ;
}


void pdiff0a::set_shared_var( const string& var,
			      int val )
{
	vreplace( var, val ) ;
	vput( var, SHARED ) ;
}


string pdiff0a::create_tempname()
{
	path temp = temp_directory_path() / unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;

	return temp.native() ;
}
