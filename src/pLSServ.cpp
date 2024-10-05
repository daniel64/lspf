/*
  Copyright (c) 2023 Daniel John Erdos

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


/* ****************************************************************************************************************** */
/* Logical Screen Services.                                                                                           */
/* Services which are common to a logical screen:                                                                     */
/*                                                                                                                    */
/* Currently implemented:                                                                                             */
/*                                                                                                                    */
/* LMCLOSE                                                                                                            */
/* LMDDISP                                                                                                            */
/* LMDFREE                                                                                                            */
/* LMDINIT                                                                                                            */
/* LMFREE                                                                                                             */
/* LMGET                                                                                                              */
/* LMINIT                                                                                                             */
/* LMMADD                                                                                                             */
/* LMMDEL                                                                                                             */
/* LMMFIND                                                                                                            */
/* LMMLIST                                                                                                            */
/* LMMREP (calls LMMADD)                                                                                              */
/* LMOPEN                                                                                                             */
/* LMPUT                                                                                                              */
/* LMQUERY                                                                                                            */
/*                                                                                                                    */
/* ****************************************************************************************************************** */

#include <sys/stat.h>
#include <boost/regex.hpp>

#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

using namespace boost ;


lss::~lss()
{
	for ( auto lmm : lmmlist_entries )
	{
		delete lmm.second ;
	}
}


void lss::lmclose( errblock& err,
		   fPOOL* funcPool,
		   const string& dataid )
{
	//
	// LMCLOSE
	//
	// Close a dataid created by the LMINIT service.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 File not open. PSYZ002 describes error.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 20 Severe error.
	//

	err.setRC( 0 ) ;

	string ddn ;
	string dsn ;

	map<string, lmentry>::iterator it ;

	if ( !exists_dataid( err, funcPool, it, dataid ) )
	{
		return ;
	}

	lmentry* lm = &it->second ;

	if ( !lm->lmopened() )
	{
		funcPool->put2( err, "ZEDSMSG", "File not open" ) ;
		funcPool->put2( err, "ZEDLMSG", "File for DATAID '" + dataid + "' has not been opened" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 8 ) ;
		return ;
	}

	ddn = lm->get_ddname() ;
	dsn = get_dsn_dataid( dataid ) ;

	if ( lm->for_input() )
	{
		if ( p_gls->is_open_input( ddn, err.taskid ) )
		{
			p_gls->remove_open_input( ddn, err.taskid ) ;
		}
	}
	else if ( p_gls->is_open_output( ddn, err.taskid ) )
	{
		p_gls->remove_open_output( ddn ) ;
	}

	if ( lm->using_dsname() )
	{
		if ( lm->is_directory() )
		{
			dsn = full_name( dsn, lm->get_tfile() ) ;
			if ( exists( dsn ) )
			{
				remove( dsn ) ;
			}
		}
	}
	else if ( p_gls->alloc_is_directory( ddn ) )
	{
		dsn = full_name( dsn, lm->get_tfile() ) ;
		if ( exists( dsn ) )
		{
			remove( dsn ) ;
		}
	}

	p_gls->set_alloc_closed( ddn ) ;

	lm->set_lmclosed() ;
}


void lss::lmddisp( errblock& err,
		   fPOOL* funcPool,
		   const string& listid )
{
	//
	// LMDDISP
	//
	// Display directory list created by the LMDINIT service.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 Error creating list.  PSYZ002 describes error.
	//   RC = 10 LISTID not found.  PSYZ002 describes error.
	//   RC = 12 Incorrect keyword.
	//   RC = 20 Severe error.
	//

	map<string, ldentry>::iterator it ;

	if ( !exists_listid( err, funcPool, it, listid ) )
	{
		return ;
	}
}


void lss::lmdfree( errblock& err,
		   fPOOL* funcPool,
		   const string& listid )
{
	//
	// LMDFREE
	//
	// Remove a listid created by the LMDINIT service.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 Free of listid failed.  PSYZ002 describes error.
	//   RC = 10 listid not found.  PSYZ002 describes error.
	//   RC = 20 Severe error.
	//

	err.setRC( 0 ) ;

	map<string, ldentry>::iterator it ;

	if ( !exists_listid( err, funcPool, it, listid ) )
	{
		return ;
	}

	ldentries.erase( it ) ;
}


void lss::lmdinit( errblock& err,
		   fPOOL* funcPool,
		   const string& listidv,
		   const string& level )
{
	//
	// LMDINIT
	//
	// Generate a listid for a directory.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 LISTID not created.  PSYZ002 describes error.
	//   RC = 12 Parameter is invalid.  PSYZ002 describes error.
	//   RC = 20 Severe error.
	//

	err.setRC( 0 ) ;

	try
	{
		if ( !exists( level ) )
		{
			throw runtime_error( "File does not exist" ) ;
		}
		if ( !is_directory( level ) )
		{
			throw runtime_error( "Entry is not a directory" ) ;
		}
	}
	catch ( runtime_error& e )
	{
		funcPool->put2( err, "ZEDSMSG", e.what() ) ;
		funcPool->put2( err, "ZEDLMSG", strerror( errno ) ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 8 ) ;
		return ;
	}
	catch (...)
	{
		funcPool->put2( err, "ZEDSMSG", "Entry cannot be accessed" ) ;
		funcPool->put2( err, "ZEDLMSG", strerror( errno ) ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 8 ) ;
		return ;
	}

	string id = "ISR" + d2ds( ++ndataid, 5 ) ;

	ldentries[ id ] = ldentry( level ) ;

	funcPool->put2( err, listidv, id ) ;
	if ( err.error() ) { return ; }
}


void lss::lmfree( errblock& err,
		  fPOOL* funcPool,
		  const string& dataid )
{
	//
	// LMFREE
	//
	// Remove a dataid created by the LMINIT service.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 Free of dataid failed.  PSYZ002 describes error.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 20 Severe error.
	//

	err.setRC( 0 ) ;

	int rc ;

	string errs ;
	string ddn ;

	map<string, lmentry>::iterator it ;

	if ( !exists_dataid( err, funcPool, it, dataid ) )
	{
		return ;
	}

	lmentry* lm  = &it->second ;

	ddn = lm->get_ddname() ;

	if ( p_gls->is_open_output( ddn, err.taskid ) )
	{
		p_gls->remove_open_output( ddn ) ;
	}

	if ( p_gls->is_open_input( ddn, err.taskid ) )
	{
		p_gls->remove_open_input( ddn, err.taskid ) ;
	}


	if ( lm->using_dsname() )
	{
		rc = p_gls->free( "F(" + lm->get_ddname() + ")", errs ) ;
		if ( rc > 0 )
		{
			funcPool->put2( err, "ZEDSMSG", "Deallocation failed" ) ;
			funcPool->put2( err, "ZEDLMSG", errs ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 8 ) ;
		}
	}

	if ( lm->lmopened() )
	{
		p_gls->set_alloc_closed( ddn ) ;
	}

	lmentries.erase( it ) ;
}


void lss::lmget( errblock& err,
		 fPOOL* funcPool,
		 const string& dataid,
		 string& data,
		 const string& datalenv,
		 size_t maxlen )
{
	//
	// LMGET
	//
	// Read a record from the file for the given DATAID.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 End of data.  No message formated.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 12 File not open for input, LMMFIND not done or parameter error.
	//   RC = 16 Error access dialogue variables.
	//   RC = 20 Severe error.
	//

	err.setRC( 0 ) ;

	string ddn ;
	string dsn ;

	std::ifstream* fin ;

	map<string, lmentry>::iterator it ;

	if ( !exists_dataid( err, funcPool, it, dataid ) )
	{
		return ;
	}

	lmentry* lm = &it->second ;

	if ( lm->using_dsname() )
	{
		if ( lm->is_directory() && !lm->done_lmmfind() )
		{
			funcPool->put2( err, "ZEDSMSG", "LMMFIND not done" ) ;
			funcPool->put2( err, "ZEDLMSG", "LMMFIND has not been done before LMGET on a directory DATAID" ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
			return ;
		}
	}
	else if ( p_gls->alloc_is_directory( lm->get_ddname() ) && !lm->done_lmmfind() )
	{
		funcPool->put2( err, "ZEDSMSG", "LMMFIND not done" ) ;
		funcPool->put2( err, "ZEDLMSG", "LMMFIND has not been done before LMGET on a directory DATAID" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
		return ;
	}

	dsn = get_dsn_dataid( dataid ) ;
	ddn = lm->get_ddname() ;

	fin = p_gls->is_open_input( ddn, err.taskid ) ;
	if ( !fin )
	{
		if ( p_gls->is_open( ddn ) )
		{
			funcPool->put2( err, "ZEDSMSG", "Open for input failed" ) ;
			funcPool->put2( err, "ZEDLMSG", "File open for DATAID '" + dataid + "' failed.  File open in another task." ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 20 ) ;
			return ;
		}
		if ( lm->using_dsname() )
		{
			if ( lm->is_directory() )
			{
				dsn = full_name( dsn, lm->get_lmmfind() ) ;
			}
		}
		else if ( p_gls->alloc_is_directory( ddn ) )
		{
			dsn = full_name( dsn, lm->get_lmmfind() ) ;
		}
		fin = new std::ifstream( dsn.c_str() ) ;
		if ( !fin->is_open() )
		{
			funcPool->put2( err, "ZEDSMSG", "Open for input failed" ) ;
			funcPool->put2( err, "ZEDLMSG", "File open for DATAID '" + dataid + "' failed" ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 20 ) ;
			delete fin ;
			return ;
		}
		p_gls->add_open_input( ddn, fin, err.taskid ) ;
	}

	if ( !getline( *fin, data ) )
	{
		err.setRC( 8 ) ;
	}
	else
	{
		if ( data.size() > maxlen )
		{
			data.resize( maxlen ) ;
		}
		funcPool->put2( err, datalenv, d2ds( data.size(), 8 ) ) ;
	}
}


void lss::lminit( errblock& err,
		  fPOOL* funcPool,
		  const string& dataidv,
		  string dsn,
		  string ddn,
		  LM_DISP disp )
{
	//
	// LMINIT
	//
	// Associate a dataid with a file/directory.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 Dataid not created.  PSYZ002 describes error.
	//   RC = 12 Parameter is invalid.  PSYZ002 describes error.
	//   RC = 20 Severe error.
	//

	ENT_TYPE type = ENT_UNKNOWN ;

	int rc ;

	struct stat results ;

	string errs ;

	err.setRC( 0 ) ;

	if ( ddn != "" )
	{
		const tso_alloc* alloc = p_gls->get_alloc( ddn ) ;
		if ( alloc->path == "" )
		{
			funcPool->put2( err, "ZEDSMSG", "File not allocated" ) ;
			funcPool->put2( err, "ZEDLMSG", "Allocate file '" + ddn + "' before issuing LMINIT." ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 8 ) ;
			return ;
		}
	}
	else
	{
		try
		{
			if ( !exists( dsn ) )
			{
				throw runtime_error( "" ) ;
			}
			if ( lstat( dsn.c_str(), &results ) == 0 )
			{
				type = ( S_ISDIR( results.st_mode )  ) ? ENT_DIR    :
				       ( S_ISREG( results.st_mode )  ) ? ENT_FILE   :
				       ( S_ISCHR( results.st_mode )  ) ? ENT_CHAR   :
				       ( S_ISBLK( results.st_mode )  ) ? ENT_BLOCK  :
				       ( S_ISFIFO( results.st_mode ) ) ? ENT_FIFO   :
				       ( S_ISSOCK( results.st_mode ) ) ? ENT_SOCKET :
				       ( S_ISLNK( results.st_mode )  ) ? ENT_SYML   : ENT_UNKNOWN ;
			}
			else
			{
				throw runtime_error( "" ) ;
			}
		}
		catch (...)
		{
			funcPool->put2( err, "ZEDSMSG", "File not found or cannot be accessed" ) ;
			funcPool->put2( err, "ZEDLMSG", "Check file exists and has the correct permissions." ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 8 ) ;
			return ;
		}
	}

	if ( ddn == "" )
	{
		while ( true )
		{
			ddn = p_gls->gn_ispddn() ;
			if ( !p_gls->is_ddname_allocated( ddn ) )
			{
				rc = p_gls->alloc( "ALLOC F(" + ddn + ") DS(" + dsn + ") " + ( ( disp == LM_SHR ) ? "SHR" : "OLD" ), errs ) ;
				if ( rc > 0 )
				{
					funcPool->put2( err, "ZEDSMSG", "File cannot be allocated" ) ;
					funcPool->put2( err, "ZEDLMSG", errs ) ;
					err.seterrid( TRACE_INFO(), "PSYZ002", 8 ) ;
					return ;
				}
				break ;
			}
		}
	}


	string id = "ISR" + d2ds( ++ndataid, 5 ) ;

	lmentries[ id ] = lmentry( dsn, ddn, disp, type ) ;

	funcPool->put2( err, dataidv, id ) ;
	if ( err.error() ) { return ; }
}


void lss::lmmadd( errblock& err,
		  fPOOL* funcPool,
		  const string& dataid,
		  const string& member,
		  bool repl )
{
	//
	// LMMADD
	//
	//   Add a member to a library.
	//
	//   RC =  0 Normal completion.
	//   RC =  4 Directory already contains the specified name.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 12 Parameter invalid, file not open for output, DSORG invalid.
	//   RC = 14 No records written for the member to be added.
	//   RC = 16 Truncation accessing dialgue variables.
	//   RC = 20 Severe error.
	//
	//
	// LMMREP
	//
	//   Replace a member in a library.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 Member is added - it did not exist.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 12 Parameter invalid, file not open for output, DSORG invalid.
	//   RC = 14 No records written for the member to be added.
	//   RC = 16 Truncation accessing dialgue variables.
	//   RC = 20 Severe error.
	//

	err.setRC( 0 ) ;

	string ddn ;
	string dsn ;
	string fname1 ;
	string fname2 ;

	std::ofstream* fout ;

	map<string, lmentry>::iterator it ;

	if ( !exists_dataid( err, funcPool, it, dataid ) )
	{
		return ;
	}

	lmentry* lm = &it->second ;

	if ( !lm->lmopened() || lm->for_input() )
	{
		funcPool->put2( err, "ZEDSMSG", "DATAID not open output" ) ;
		funcPool->put2( err, "ZEDLMSG", "DATAID '" + dataid + "' has not been opened for output" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
		return ;
	}

	if ( lm->using_dsname() )
	{
		if ( !lm->is_directory() )
		{
			funcPool->put2( err, "ZEDSMSG", "DATAID is not a directory" ) ;
			funcPool->put2( err, "ZEDLMSG", "DATAID '" + dataid + "' is not a directory" ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
			return ;
		}
	}
	else if ( !p_gls->alloc_is_directory( lm->get_ddname() ) )
	{
		funcPool->put2( err, "ZEDSMSG", "DATAID is not a directory" ) ;
		funcPool->put2( err, "ZEDLMSG", "DATAID '" + dataid + "' is not a directory" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
		return ;
	}

	if ( lm->get_records() == 0 )
	{
		funcPool->put2( err, "ZEDSMSG", "No records written to DATAID" ) ;
		funcPool->put2( err, "ZEDLMSG", "No records have been written to DATAID '" + dataid + "'" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
		return ;
	}

	dsn    = get_dsn_dataid( dataid ) ;
	fname1 = full_name( dsn, member ) ;

	if ( exists( fname1 ) )
	{
		if ( !repl )
		{
			funcPool->put2( err, "ZEDSMSG", "Member already exits" ) ;
			funcPool->put2( err, "ZEDLMSG", "Member specified already exists in DATAID '" + dataid + "'" ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 4 ) ;
			return ;
		}
	}
	else if ( repl )
	{
		funcPool->put2( err, "ZEDSMSG", "Member added" ) ;
		funcPool->put2( err, "ZEDLMSG", "Member added to directory.  It did not previously exist." ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 8 ) ;
	}

	ddn = lm->get_ddname() ;

	fout = p_gls->is_open_output( ddn, err.taskid ) ;
	if ( !fout || !fout->is_open() )
	{
		funcPool->put2( err, "ZEDSMSG", "File for DATAID not open output" ) ;
		funcPool->put2( err, "ZEDLMSG", "File for DATAID '" + dataid + "' not open for output" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 20 ) ;
		return ;
	}

	p_gls->remove_open_output( ddn ) ;

	fname2 = full_name( dsn, lm->get_tfile() ) ;

	if ( repl && exists( fname1 ) )
	{
		remove( fname1 ) ;
	}

	rename( fname2, fname1 ) ;

	lm->clr_records() ;
}


void lss::lmmdel( errblock& err,
		  fPOOL* funcPool,
		  const string& dataid,
		  const string& member )
{
	//
	// LMMDEL
	//
	//   Delete a member from a library.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 Member was not found.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 12 Parameter invalid, file not open for output, DSORG invalid.
	//   RC = 20 Severe error.
	//
	//   Remove any associated symlinks in the same directory.
	//

	err.setRC( 0 ) ;

	string dsn ;
	string pat ;
	string fname ;
	string tname ;

	size_t p ;

	int deletes = 0 ;

	bool useRegex ;

	vector<path> vt ;

	regex expression ;

	map<string, lmentry>::iterator it ;

	if ( !exists_dataid( err, funcPool, it, dataid ) )
	{
		return ;
	}

	lmentry* lm = &it->second ;

	if ( !lm->lmopened() || lm->for_input() )
	{
		funcPool->put2( err, "ZEDSMSG", "DATAID not open output" ) ;
		funcPool->put2( err, "ZEDLMSG", "DATAID '" + dataid + "' has not been opened for output" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
		return ;
	}

	if ( lm->using_dsname() )
	{
		if ( !lm->is_directory() )
		{
			funcPool->put2( err, "ZEDSMSG", "DATAID is not a directory" ) ;
			funcPool->put2( err, "ZEDLMSG", "DATAID '" + dataid + "' is not a directory" ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
			return ;
		}
	}
	else if ( !p_gls->alloc_is_directory( lm->get_ddname() ) )
	{
		funcPool->put2( err, "ZEDSMSG", "DATAID is not a directory" ) ;
		funcPool->put2( err, "ZEDLMSG", "DATAID '" + dataid + "' is not a directory" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
		return ;
	}

	dsn   = get_dsn_dataid( dataid ) ;
	fname = full_name( dsn, member ) ;

	useRegex = ( member.find_first_of( "%*" ) != string::npos ) ;

	if ( useRegex )
	{
		pat = conv_regex( member, '%', '*' ) ;
		try
		{
			expression.assign( pat ) ;
		}
		catch  (...)
		{
			funcPool->put2( err, "ZEDSMSG", "Invalid pattern" ) ;
			funcPool->put2( err, "ZEDLMSG", "Entered member is not a valid pattern" ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 20 ) ;
			return ;
		}
		try
		{
			copy( directory_iterator( dsn ), directory_iterator(), back_inserter( vt ) ) ;
		}
		catch (...)
		{
			funcPool->put2( err, "ZEDSMSG", "Error accessing directory" ) ;
			funcPool->put2( err, "ZEDLMSG", "Check permissions and directory is accessible" ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 20 ) ;
			return ;
		}
		for ( auto& ent : vt )
		{
			fname = ent.string() ;
			p     = fname.find_last_of( '/' ) ;
			if ( is_symlink( fname ) && !regex_match( fname.begin() + p + 1, fname.end(), expression ) )
			{
				tname = get_target( fname ) ;
				p     = tname.find_last_of( '/' ) ;
				if ( p != string::npos )
				{
					if ( tname.compare( 0, p, dsn, 0, p ) != 0 )
					{
						continue ;
					}
					else
					{
						tname.erase( 0, p + 1 ) ;
					}
				}
				if ( regex_match( tname.begin(), tname.end(), expression ) )
				{
					try
					{
						remove( fname ) ;
						++deletes ;
					}
					catch ( boost::filesystem::filesystem_error &e )
					{
						err.seterrid( TRACE_INFO(), "PSYS012C", e.what() ) ;
						return ;
					}
				}
			}
			else if ( regex_match( fname.begin() + p + 1, fname.end(), expression ) )
			{
				try
				{
					if ( is_regular_file( fname ) || is_symlink( fname ) )
					{
						remove( fname ) ;
						++deletes ;
					}
				}
				catch ( boost::filesystem::filesystem_error &e )
				{
					err.seterrid( TRACE_INFO(), "PSYS012C", e.what() ) ;
					return ;
				}
			}
		}
		if ( deletes == 0 )
		{
			funcPool->put2( err, "ZEDSMSG", "No members found" ) ;
			funcPool->put2( err, "ZEDLMSG", "No Members found that match the entered pattern" ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 8 ) ;
			return ;
		}
	}
	else
	{
		if ( !is_symlink( fname ) && !exists( fname ) )
		{
			funcPool->put2( err, "ZEDSMSG", "Member not found" ) ;
			funcPool->put2( err, "ZEDLMSG", "Member does not exist in the directory" ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 8 ) ;
			return ;
		}
		if ( !is_symlink( fname ) )
		{
			try
			{
				copy( directory_iterator( dsn ), directory_iterator(), back_inserter( vt ) ) ;
			}
			catch (...)
			{
				funcPool->put2( err, "ZEDSMSG", "Error accessing directory" ) ;
				funcPool->put2( err, "ZEDLMSG", "Check permissions and directory is accessible" ) ;
				err.seterrid( TRACE_INFO(), "PSYZ002", 20 ) ;
				return ;
			}
			for ( auto& ent : vt )
			{
				if ( is_symlink( ent.string() ) )
				{
					tname = get_target( ent.string() ) ;
					p     = tname.find_last_of( '/' ) ;
					if ( p == string::npos )
					{
						tname = full_name( dsn, tname ) ;
					}
					if ( tname == fname )
					{
						try
						{
							remove( ent.string() ) ;
						}
						catch ( boost::filesystem::filesystem_error &e )
						{
							err.seterrid( TRACE_INFO(), "PSYS012C", e.what() ) ;
							return ;
						}
					}
				}
			}
		}
		try
		{
			remove( fname ) ;
		}
		catch ( boost::filesystem::filesystem_error &e )
		{
			err.seterrid( TRACE_INFO(), "PSYS012C", e.what() ) ;
			return ;
		}
	}
}


void lss::lmmfind( errblock& err,
		   fPOOL* funcPool,
		   const string& dataid,
		   const string& member,
		   const string& stats )
{
	//
	// LMMFIND
	//
	//   Find a member in a directory.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 Member not found.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 12 Parameter invalid, file not open for input, DSORG invalid.
	//   RC = 14 No records written for the member to be added.
	//   RC = 16 Truncation accessing dialgue variables.
	//   RC = 20 Severe error.
	//

	err.setRC( 0 ) ;

	string dsn ;
	string fname ;

	map<string, lmentry>::iterator it ;

	if ( !exists_dataid( err, funcPool, it, dataid ) )
	{
		return ;
	}

	lmentry* lm = &it->second ;

	if ( !lm->lmopened() || !lm->for_input() )
	{
		funcPool->put2( err, "ZEDSMSG", "DATAID not open input" ) ;
		funcPool->put2( err, "ZEDLMSG", "DATAID '" + dataid + "' has not been opened for input" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
		return ;
	}

	if ( lm->using_dsname() )
	{
		if ( !lm->is_directory() )
		{
			funcPool->put2( err, "ZEDSMSG", "DATAID is not a directory" ) ;
			funcPool->put2( err, "ZEDLMSG", "DATAID '" + dataid + "' is not a directory" ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
			return ;
		}
	}
	else if ( !p_gls->alloc_is_directory( lm->get_ddname() ) )
	{
		funcPool->put2( err, "ZEDSMSG", "DATAID is not a directory" ) ;
		funcPool->put2( err, "ZEDLMSG", "DATAID '" + dataid + "' is not a directory" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
		return ;
	}

	dsn   = get_dsn_dataid( dataid ) ;
	fname = full_name( dsn, member ) ;

	if ( !exists( fname ) )
	{
		funcPool->put2( err, "ZEDSMSG", "Member does not exit" ) ;
		funcPool->put2( err, "ZEDLMSG", "Member specified does not exists in directory DATAID '" + dataid + "'" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 8 ) ;
		return ;
	}

	lm->set_lmmfind( member ) ;

	if ( stats == "YES" )
	{
		set_stats( err, funcPool, member, dsn ) ;
	}

}


void lss::lmmlist( errblock& err,
		   fPOOL* funcPool,
		   const string& dataid,
		   const string& opt,
		   const string& member,
		   const string& stats,
		   const string& pattern )
{
	//
	// LMMLIST
	//
	//   List members in a directory.
	//
	//   RC =  0 Normal completion.
	//   RC =  4 Empty member list.
	//   RC =  8 LIST - End of member list.
	//           FREE - Member list not found.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 12 Parameter invalid, file not open for input, DSORG invalid.
	//   RC = 16 Truncation accessing dialgue variables.
	//   RC = 20 Severe error.
	//
	//   Note:  Only regular files are retrieved.
	//          No recursion into sub-directories.
	//          Pattern is case sensitive.
	//

	err.setRC( 0 ) ;

	uint i ;
	uint j ;

	string p ;
	string dsn ;
	string mem ;

	vector<string>* vec ;

	map<string, lmentry>::iterator it ;

	boost::system::error_code ec ;

	regex expression ;

	if ( !exists_dataid( err, funcPool, it, dataid ) )
	{
		return ;
	}

	lmentry* lm = &it->second ;

	if ( !lm->lmopened() || !lm->for_input() )
	{
		funcPool->put2( err, "ZEDSMSG", "DATAID not open input" ) ;
		funcPool->put2( err, "ZEDLMSG", "DATAID '" + dataid + "' has not been opened for input" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
		return ;
	}

	if ( opt == "FREE" )
	{
		auto itl = lmmlist_entries.find( dataid ) ;
		if ( itl != lmmlist_entries.end() )
		{
			lmmlist_posn.erase( itl->second ) ;
			delete itl->second ;
			lmmlist_entries.erase( itl ) ;
		}
		else
		{
			err.setRC( 8 ) ;
		}
		return ;
	}

	if ( lm->using_dsname() )
	{
		if ( !lm->is_directory() )
		{
			funcPool->put2( err, "ZEDSMSG", "DATAID is not a directory" ) ;
			funcPool->put2( err, "ZEDLMSG", "DATAID '" + dataid + "' is not a directory" ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
			return ;
		}
	}
	else if ( !p_gls->alloc_is_directory( lm->get_ddname() ) )
	{
		funcPool->put2( err, "ZEDSMSG", "DATAID is not a directory" ) ;
		funcPool->put2( err, "ZEDLMSG", "DATAID '" + dataid + "' is not a directory" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
		return ;
	}

	dsn = get_dsn_dataid( dataid ) ;

	auto itl = lmmlist_entries.find( dataid ) ;

	if ( itl == lmmlist_entries.end() )
	{
		if ( pattern != "" )
		{
			try
			{
				expression.assign( conv_regex( pattern, '%', '*' ) ) ;
			}
			catch  ( boost::regex_error& e )
			{
				funcPool->put2( err, "ZEDSMSG", "Invalid pattern" ) ;
				funcPool->put2( err, "ZEDLMSG", "Pattern parameter cannot be converted to a REGEX" ) ;
				err.seterrid( TRACE_INFO(), "PSYZ002", 20 ) ;
				return ;
			}
		}
		vec = new vector<string> ;
		for ( j = getpaths( dsn ), i = 1 ; i <= j ; ++i )
		{
			p = getpath( dsn, i ) ;
			directory_iterator zcurr( p, ec ) ;
			directory_iterator zend ;
			while ( zcurr != zend )
			{
				path curr( *zcurr ) ;
				if ( is_regular_file( curr.string() ) )
				{
					mem = curr.filename().string() ;
					if ( pattern == "" || regex_match( mem.begin(), mem.end(), expression ) )
					{
						vec->push_back( mem ) ;
					}
				}
				zcurr.increment( ec ) ;
				if ( ec )
				{
					funcPool->put2( err, "ZEDSMSG", "Error accessing directory" ) ;
					funcPool->put2( err, "ZEDLMSG", "An error occured accessing the directory." ) ;
					err.seterrid( TRACE_INFO(), "PSYZ002", 20 ) ;
					delete vec ;
					return ;
				}
			}
		}
		if ( vec->empty() )
		{
			delete vec ;
			err.setRC( 4 ) ;
			return ;
		}
		sort( vec->begin(), vec->end() ) ;
		vec->erase( unique( vec->begin(), vec->end() ), vec->end() ) ;
		lmmlist_entries[ dataid ] = vec ;
	}
	else
	{
		vec = itl->second ;
	}

	auto itc = lmmlist_posn.find( vec ) ;

	if ( itc == lmmlist_posn.end() )
	{
		i   = 0 ;
		mem = funcPool->get2( err, 8, member ) ;
		if ( err.error() ) { return ; }
		if ( mem != "" )
		{
		     for ( ; i < vec->size() && vec->at( i ) < mem ; ++i )
		     {
		     }
		}
	}
	else
	{
		i = itc->second + 1 ;
	}

	if ( i == vec->size() )
	{
		err.setRC( 8 ) ;
		return ;
	}

	funcPool->put2( err, member, vec->at( i ) ) ;
	if ( err.error() ) { return ; }

	if ( stats == "YES" )
	{
		set_stats( err, funcPool, vec->at( i ), dsn ) ;
	}

	lmmlist_posn[ vec ] = i ;
}


void lss::lmopen( errblock& err,
		  fPOOL* funcPool,
		  const string& dataid,
		  const string& option,
		  const string& orgv )
{
	//
	// LMOPEN
	//
	// Open a file for input or output processing.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 File could not be opened.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 12 Parameter invalid or file already open.
	//   RC = 20 Severe error.
	//

	err.setRC( 0 ) ;

	string ddn ;
	string dsn ;

	map<string, lmentry>::iterator it ;

	if ( !exists_dataid( err, funcPool, it, dataid ) )
	{
		return ;
	}

	lmentry* lm = &it->second ;

	if ( lm->lmopened() )
	{
		funcPool->put2( err, "ZEDSMSG", "DATAID already opened" ) ;
		funcPool->put2( err, "ZEDLMSG", "DATAID '" + dataid + "' already opened" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
		return ;
	}

	if ( option == "OUTPUT" && lm->lminit_shr() )
	{
		funcPool->put2( err, "ZEDSMSG", "DATAID opened SHR" ) ;
		funcPool->put2( err, "ZEDLMSG", "OUTPUT requires DATAID '" + dataid + "' to be open EXCLU" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
		return ;
	}

	if ( orgv != "" )
	{
		funcPool->put2( err, orgv, lm->get_type_str() ) ;
		if ( err.error() ) { return ; }
	}

	dsn = get_dsn_dataid( dataid ) ;
	ddn = lm->get_ddname() ;

	if ( option == "OUTPUT" )
	{
		if ( p_gls->is_open( ddn ) )
		{
			funcPool->put2( err, "ZEDSMSG", "File for DATAID already opened" ) ;
			funcPool->put2( err, "ZEDLMSG", "File for DATAID '" + dataid + "' already opened input or output" ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
			return ;
		}
	}
	else
	{
		if ( p_gls->is_open_output( ddn ) )
		{
			funcPool->put2( err, "ZEDSMSG", "File for DATAID already opened" ) ;
			funcPool->put2( err, "ZEDLMSG", "File for DATAID '" + dataid + "' already opened output" ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 12 ) ;
			return ;
		}
		lm->set_input() ;
	}

	p_gls->set_alloc_open( ddn ) ;

	lm->set_lmopened() ;
}


void lss::lmput( errblock& err,
		 fPOOL* funcPool,
		 const string& dataid,
		 const string& data )
{
	//
	// LMPUT
	//
	// Write a record to the file/directory for the given DATAID.
	//
	// If the dataid is for a directory, write to a temporary file in the
	// directory - LMMADD will then create the correct file name.
	//
	//   RC =  0 Normal completion.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 12 File not open for output or parameter error.
	//   RC = 16 Error access dialogue variables.
	//   RC = 20 Severe error.
	//

	err.setRC( 0 ) ;

	string ddn ;
	string dsn ;

	std::ofstream* fout ;

	map<string, lmentry>::iterator it ;

	if ( !exists_dataid( err, funcPool, it, dataid ) )
	{
		return ;
	}

	lmentry* lm = &it->second ;

	dsn = get_dsn_dataid( dataid ) ;
	ddn = lm->get_ddname() ;

	fout = p_gls->is_open_output( ddn, err.taskid ) ;
	if ( !fout )
	{
		if ( p_gls->is_open( ddn ) )
		{
			funcPool->put2( err, "ZEDSMSG", "Open for output failed" ) ;
			funcPool->put2( err, "ZEDLMSG", "File open for DATAID '" + dataid + "' failed.  File open in another task." ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 20 ) ;
			return ;
		}
		if ( lm->using_dsname() )
		{
			if ( lm->is_directory() )
			{
				dsn = full_name( dsn, lm->get_tfile() ) ;
			}
		}
		else if ( p_gls->alloc_is_directory( ddn ) )
		{
			dsn = full_name( dsn, lm->get_tfile() ) ;
		}
		fout = new std::ofstream( dsn.c_str() ) ;
		if ( !fout->is_open() )
		{
			funcPool->put2( err, "ZEDSMSG", "Open for output failed" ) ;
			funcPool->put2( err, "ZEDLMSG", "File open for DATAID '" + dataid + "' failed" ) ;
			err.seterrid( TRACE_INFO(), "PSYZ002", 20 ) ;
			delete fout ;
			return ;
		}
		p_gls->add_open_output( ddn, fout, err.taskid ) ;
	}

	*fout << data << endl ;

	lm->inc_records() ;
}


void lss::lmquery( errblock& err,
		   fPOOL* funcPool,
		   const string& dataid,
		   const string& dsnv,
		   const string& ddnv,
		   const string& enqv,
		   const string& openv,
		   const string& dsorgv )
{
	//
	// LMQUERY
	//
	// Return values specified for the LMINIT service.
	//
	//   RC =  0 Normal completion.
	//   RC =  4 Some data not available.  Blanks returned.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 20 Severe error.
	//

	err.setRC( 0 ) ;

	map<string, lmentry>::iterator it ;

	if ( !exists_dataid( err, funcPool, it, dataid ) )
	{
		return ;
	}

	lmentry* lm = &it->second ;

	if ( dsnv != "" )
	{
		const string& val = lm->get_entry() ;
		funcPool->put2( err, dsnv, val ) ;
		if ( err.error() ) { return ; }
		if ( val == "" )
		{
			err.setRC( 4 ) ;
		}
	}

	if ( ddnv != "" )
	{
		const string& val = lm->get_ddname() ;
		funcPool->put2( err, ddnv, val ) ;
		if ( err.error() ) { return ; }
		if ( val == "" )
		{
			err.setRC( 4 ) ;
		}
	}

	if ( enqv != "" )
	{
		const string& val = lm->get_disp_str() ;
		funcPool->put2( err, enqv, val ) ;
		if ( err.error() ) { return ; }
		if ( val == "" )
		{
			err.setRC( 4 ) ;
		}
	}

	if ( openv != "" )
	{
		if ( !lm->lmopened() )
		{
			funcPool->put2( err, openv, "" ) ;
			if ( err.error() ) { return ; }
			err.setRC( 4 ) ;
		}
		else
		{
			funcPool->put2( err, openv, ( lm->for_input() ) ? "INPUT" : "OUTPUT" ) ;
			if ( err.error() ) { return ; }
		}
	}

	if ( dsorgv != "" )
	{
		const string& val = lm->get_type_str() ;
		funcPool->put2( err, dsorgv, val ) ;
		if ( err.error() ) { return ; }
		if ( val == "" )
		{
			err.setRC( 4 ) ;
		}
	}
}


void lss::set_stats( errblock& err,
		     fPOOL* funcPool,
		     const string& mem,
		     const string& paths )
{
	//
	// Set statistics for member mem in the function pool.
	//

	uint i ;
	uint j ;

	string p ;
	string ent ;

	string zlcdsname ;
	string zlcdsorg ;
	string zlcchange ;
	string zlcmodify ;
	string zlcaccess ;
	string zlcowner ;
	string zlcgroup ;
	string zlcused ;
	string zlcinode ;
	string zlcmajor ;
	string zlcminor ;
	string zlcblksize ;
	string zlclink ;

	struct stat results ;

	struct tm* time_info ;

	char buf[ 20 ] ;

	for ( i = 1, j = getpaths( paths ) ; i <= j ; ++i )
	{
		p   = getpath( paths, i ) ;
		ent = full_name( p, mem ) ;
		if ( exists( ent ) )
		{
			funcPool->put2( err, "ZLCLIB", p ) ;
			if ( err.error() ) { return ; }
			break ;
		}
	}

	zlcdsname = ent ;
	try
	{
		if ( lstat( ent.c_str(), &results ) == 0 )
		{
			zlcdsorg = ( S_ISDIR( results.st_mode )  ) ? "DIR"    :
				   ( S_ISREG( results.st_mode )  ) ? "FILE"   :
				   ( S_ISCHR( results.st_mode )  ) ? "CHAR"   :
				   ( S_ISBLK( results.st_mode )  ) ? "BLOCK"  :
				   ( S_ISFIFO( results.st_mode ) ) ? "FIFO"   :
				   ( S_ISSOCK( results.st_mode ) ) ? "SOCKET" :
				   ( S_ISLNK( results.st_mode )  ) ? "SYML"   : "" ;

			zlcused = d2ds( results.st_size ) ;
			time_info = gmtime( &(results.st_ctime) ) ;
			strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ; zlcchange = buf ;

			time_info = gmtime( &(results.st_mtime) ) ;
			strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ; zlcmodify = buf ;

			time_info = gmtime( &(results.st_atime) ) ;
			strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ; zlcaccess = buf ;

			zlcinode = d2ds( results.st_ino ) ;

			struct passwd* pw = getpwuid( results.st_uid ) ;
			if ( pw )
			{
				zlcowner = pw->pw_name ;
			}

			struct group* gr = getgrgid( results.st_gid ) ;
			if ( gr )
			{
				zlcgroup = gr->gr_name ;
			}

			zlcmajor   = d2ds( major( results.st_dev ) ) ;
			zlcminor   = d2ds( minor( results.st_dev ) ) ;
			zlcblksize = d2ds( results.st_blksize ) ;

			if ( S_ISLNK( results.st_mode ) )
			{
				zlclink = get_target( ent ) ;
			}
		}
	}
	catch (...)
	{
		zlcdsorg   = "" ;
		zlcchange  = "" ;
		zlcmodify  = "" ;
		zlcaccess  = "" ;
		zlcowner   = "" ;
		zlcgroup   = "" ;
		zlcused    = "" ;
		zlcinode   = "" ;
		zlcmajor   = "" ;
		zlcminor   = "" ;
		zlcblksize = "" ;
		zlclink    = "" ;
	}

	funcPool->put2( err, "ZLCDSNAM", zlcdsname  ) ;
	funcPool->put2( err, "ZLCDSORG", zlcdsorg   ) ;
	funcPool->put2( err, "ZLCCHNG",  zlcchange  ) ;
	funcPool->put2( err, "ZLCMOD",   zlcmodify  ) ;
	funcPool->put2( err, "ZLCACC",   zlcaccess  ) ;
	funcPool->put2( err, "ZLCOWNER", zlcowner   ) ;
	funcPool->put2( err, "ZLCGROUP", zlcgroup   ) ;
	funcPool->put2( err, "ZLCUSED",  zlcused    ) ;
	funcPool->put2( err, "ZLCINODE", zlcinode   ) ;
	funcPool->put2( err, "ZLCMAJOR", zlcmajor   ) ;
	funcPool->put2( err, "ZLCMINOR", zlcminor   ) ;
	funcPool->put2( err, "ZLCBLKSZ", zlcblksize ) ;
	funcPool->put2( err, "ZLCLINK",  zlclink    ) ;
}


bool lss::exists_dataid( errblock& err,
			 fPOOL* funcPool,
			 map<string, lmentry>::iterator& it,
			 const string& dataid )
{
	//
	// Check if the dataid exists and set PSYZ002 if not.
	//

	it = lmentries.find( dataid ) ;

	if ( it == lmentries.end() )
	{
		funcPool->put2( err, "ZEDSMSG", "DATAID not set" ) ;
		funcPool->put2( err, "ZEDLMSG", "DATAID '" + dataid + "' not found" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 10 ) ;
		return false ;
	}

	return true ;
}


bool lss::exists_listid( errblock& err,
			 fPOOL* funcPool,
			 map<string, ldentry>::iterator& it,
			 const string& listid )
{
	//
	// Check if the listid exists and set PSYZ002 if not.
	//

	it = ldentries.find( listid ) ;

	if ( it == ldentries.end() )
	{
		funcPool->put2( err, "ZEDSMSG", "LISTID not set" ) ;
		funcPool->put2( err, "ZEDLMSG", "LISTID '" + listid + "' not found" ) ;
		err.seterrid( TRACE_INFO(), "PSYZ002", 10 ) ;
		return false ;
	}

	return true ;
}


string lss::get_dsn_dataid( const string& dataid )
{
	//
	// Return the file name or concatination associated with a dataid.
	// If the LMINIT has been done with a DDNAME, return the
	// allocated file name.
	//

	auto it = lmentries.find( dataid ) ;

	if ( it == lmentries.end() )
	{
		return "" ;
	}

	lmentry* lm = &it->second ;

	return ( lm->using_ddname() ) ? p_gls->get_allocated_path( lm->get_ddname() ) : lm->get_entry() ;
}


string lss::get_dsn_listid( const string& listid )
{
	//
	// Return the file name associated with a listid.
	//

	auto it = ldentries.find( listid ) ;

	return ( it == ldentries.end() ) ? "" : (&it->second)->get_entry() ;
}


string lss::get_target( const string& l )
{
	//
	// Return the pathname of a link target.
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
			break ;
		}
	}

	return lp ;
}


string lss::full_name( const string& a,
		       const string& b )
{
	return ( a.back() == '/' ) ? a + b : a + "/" + b ;
}
