/*
  Copyright (c) 2022 Daniel John Erdos

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

/*******************************************************************************************************/
/*                                                                                                     */
/* TSO/E-like external funtions for REXX.                                                              */
/* Application must inherit from TSOENV instead of pApplication in order to use these servcies.        */
/* (Used mainly for writing REXX interface modules)                                                    */
/*                                                                                                     */
/* First check if the function is a builtin TSO-like function (ALLOC, FREE, LISTA, etc in GLServ).     */
/*                                                                                                     */
/* Currently implemented here:                                                                         */
/*  ALTLIB                                                                                             */
/*  DROP                                                                                               */
/*  EXECIO                                                                                             */
/*  LISTDSI                                                                                            */
/*  OUTTRAP                                                                                            */
/*  SYSVAR                                                                                             */
/*                                                                                                     */
/*******************************************************************************************************/

#include <boost/filesystem.hpp>
#include <boost/scope_exit.hpp>
#include <oorexxapi.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>


using namespace std ;
using namespace boost::filesystem ;


TSOENV::TSOENV()
{
	outtrapOn    = false ;
	outtrapValid = false ;
	outtrapLines = 0 ;
	outtrapIndex = 0 ;
	outtrapVar   = "" ;
}


TSOENV::~TSOENV()
{
}


int TSOENV::TSOEntry( RexxExitContext* context,
		      RexxStringObject command )
{
	//
	// Entry routine for TSO service commands.
	// Execute any builtin commands first and trap the output if outtrap is ON.
	//

	string s = context->CString( command ) ;

	string name ;

	size_t i ;

	int j ;
	int sRC ;

	vector<string> results ;

	if ( outtrapOn )
	{
		if ( builtin( s, results, context ) )
		{
			sRC = RC ;
			for ( i = 0, j = outtrapIndex + 1 ; i < results.size() && j <= outtrapLines ; ++i, ++j )
			{
				name = outtrapVar + d2ds( j ) ;
				context->SetContextVariable( name.c_str(), context->String( results[ i ].c_str() ) ) ;
				if ( outtrapValid && name.size() < 9 )
				{
					vreplace( name, results[ i ] ) ;
				}
			}
			outtrapIndex += i ;
			name = outtrapVar + "0" ;
			context->SetContextVariable( name.c_str(), context->String( d2ds( outtrapIndex ).c_str() ) ) ;
			return sRC ;
		}
	}
	else if ( builtin( s, context ) )
	{
		return RC ;
	}

	string w1 = upper( word( s, 1 ) ) ;

	auto it = commandfn.find( w1 ) ;
	if ( it != commandfn.end() )
	{
		return (this->*(it->second))( context, s ) ;
	}

	issueLineMessage( RXTSO_INVCMD, w1 ) ;

	return 20 ;
}


bool TSOENV::builtin( const string& p,
		      RexxExitContext* context )
{
	//
	// See if this is a builtin command and execute it if it is.
	// Display any output produced.
	//
	// RC is set by this routine.
	//

	vector<string> results ;

	bool retval = builtin( p, results, context ) ;

	for ( auto& r : results )
	{
		say( r ) ;
	}

	return retval ;
}


bool TSOENV::builtin( const string& p,
		      vector<string>& results,
		      RexxExitContext* context )
{
	//
	// See if this is a builtin command and execute it if it is.
	//
	// RC is set by this routine.
	//

	int rc = 0 ;

	string err ;

	string w1 =  upper( word( p, 1 ) ) ;

	if ( w1 == "ALTLIB" )
	{
		altlib( p, results ) ;
		return true ;
	}

	switch ( p_gls->builtin( w1 ) )
	{
	case GL_ALLOC:
		rc = p_gls->alloc( p, err ) ;
		break ;

	case GL_FREE:
		rc = p_gls->free( p, err ) ;
		break ;

	case GL_HELP:
		p_gls->help( results ) ;
		break ;

	case GL_LISTALC:
		p_gls->listalc( results ) ;
		break ;

	case GL_TIME:
		results.push_back( p_gls->get_time() ) ;
		break ;

	case GL_INVALID:
		RC = 20 ;
		return false ;
	}

	if ( err != "" )
	{
		results.push_back( err ) ;
	}

	RC = rc ;

	return true ;
}


int TSOENV::drop( RexxCallContext* context,
		  const string& s )
{
	//
	// Drop a variable.
	//
	//   This routine drops the rexx variable and sets the variable value to its
	//   name in lspf - synchronising the lspf and REXX variable pools will not
	//   achieve this after a REXX drop statement.
	//
	//   This version is called from the lspf REXX function package.  IE. y = drop( "var1 var2 ..." )
	//
	//   Return code:
	//    0 Normal completion.
	//    8 Not a valid lspf variable name.  Not dropped.
	//   20 Severe error.
	//
	// RC is not set by this method.
	//

	int ws = words( s ) ;

	int rc = 0 ;

	string name ;

	guardInt t( RC ) ;

	for ( int i = 1 ; i <= ws ; ++i )
	{
		name = upper( word( s, i ) ) ;
		if ( isvalidName( name ) )
		{
			context->DropContextVariable( name.c_str() ) ;
			vreplace( name, name ) ;
			rc = max( rc, RC ) ;
		}
		else
		{
			rc = max( rc, 8 ) ;
		}
	}


	return rc ;
}


int TSOENV::Drop( RexxExitContext* context,
		  const string& s )
{
	//
	// Drop a variable.
	//
	//   This routine drops the rexx variable and sets the variable value to its
	//   name in lspf - synchronising the lspf and REXX variable pools will not
	//   achieve this after a REXX drop statement.
	//
	//   This version is called from the TSO address environment  IE. address TSO "drop var1 var2 ...".
	//
	//   RETURN:
	//    0 Normal completion.
	//    8 Not a valid lspf variable name.  Not dropped.
	//   20 Severe error.
	//
	// RC is not set by this method.
	//

	int ws = words( s ) ;

	int rc = 0 ;

	string name ;

	guardInt t( RC ) ;

	for ( int i = 2 ; i <= ws ; ++i )
	{
		name = upper( word( s, i ) ) ;
		if ( isvalidName( name ) )
		{
			context->DropContextVariable( name.c_str() ) ;
			vreplace( name, name ) ;
			rc = max( rc, RC ) ;
		}
		else
		{
			rc = max( rc, 8 ) ;
		}
	}


	return rc ;
}


int TSOENV::listdsi( RexxCallContext* context,
		     const string& dsn,
		     const string& type )
{
	//
	// Return file information.
	//
	//   This is called from the lspf REXX function package.  IE. y = listdsi( name, "FILE" )
	//   DSN can be a dialogue variable.
	//
	//   RETURN:
	//     0  Normal completion.
	//     4  Normal completion.  Some data unavailable.
	//    16  File not found.
	//    20  Severe error.
	//
	// RC is not set by this method.
	//

	string file ;

	string sysdsname ;
	string sysdsorg ;
	string syschange ;
	string sysmodify ;
	string sysaccess ;
	string sysowner ;
	string sysgroup ;
	string sysused ;
	string sysinode ;
	string sysmajor ;
	string sysminor ;
	string sysblksize ;
	string syslink ;
	string sysmsglvl1 ;
	string sysmsglvl2 ;

	string sysreason = "0000" ;

	string irlnk ;

	struct stat results ;

	struct tm* time_info ;

	guardInt t( RC ) ;

	int rc = 0 ;

	char buf[ 20 ] ;
	char* buffer   ;

	size_t bufferSize = 255 ;

	if ( type == "FILE" )
	{
		file = p_gls->get_allocated_path( dsn ) ;
		if ( file == "" )
		{
			clear_sysvars( context ) ;
			context->SetContextVariable( "SYSREASON",  context->String( "0016" ) ) ;
			context->SetContextVariable( "SYSMSGLVL1", context->String( "File not allocated" ) ) ;
			return 16 ;
		}
	}
	else if ( isvalidName( upper( dsn ) ) )
	{
		vcopy( upper( dsn ), file ) ;
		if ( RC > 0 )
		{
			clear_sysvars( context ) ;
			context->SetContextVariable( "SYSREASON",  context->String( "0016" ) ) ;
			context->SetContextVariable( "SYSMSGLVL1", context->String( "Dialogue variable not found" ) ) ;
			return 16 ;
		}
		if ( file == "" )
		{
			clear_sysvars( context ) ;
			context->SetContextVariable( "SYSREASON",  context->String( "0016" ) ) ;
			context->SetContextVariable( "SYSMSGLVL1", context->String( "Dialogue variable does not contain a valid file name" ) ) ;
			return 16 ;
		}
	}
	else
	{
		file = dsn ;
	}

	try
	{
		if ( !exists( file ) )
		{
			throw runtime_error( "File does not exist" ) ;
		}
		sysdsname = file ;
		if ( lstat( file.c_str(), &results ) == 0 )
		{
			sysdsorg = ( S_ISDIR( results.st_mode )  ) ? "DIR"    :
				   ( S_ISREG( results.st_mode )  ) ? "FILE"   :
				   ( S_ISCHR( results.st_mode )  ) ? "CHAR"   :
				   ( S_ISBLK( results.st_mode )  ) ? "BLOCK"  :
				   ( S_ISFIFO( results.st_mode ) ) ? "FIFO"   :
				   ( S_ISSOCK( results.st_mode ) ) ? "SOCKET" :
				   ( S_ISLNK( results.st_mode )  ) ? "SYML"   : "" ;

			sysused = d2ds( results.st_size ) ;
			time_info = gmtime( &(results.st_ctime) ) ;
			strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ; syschange = buf ;

			time_info = gmtime( &(results.st_mtime) ) ;
			strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ; sysmodify = buf ;

			time_info = gmtime( &(results.st_atime) ) ;
			strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ; sysaccess = buf ;

			sysinode = d2ds( results.st_ino ) ;

			struct passwd* pw = getpwuid( results.st_uid ) ;
			if ( pw )
			{
				sysowner = pw->pw_name ;
			}

			struct group* gr = getgrgid( results.st_gid ) ;
			if ( gr )
			{
				sysgroup = gr->gr_name ;
			}

			sysmajor   = d2ds( major( results.st_dev ) ) ;
			sysminor   = d2ds( minor( results.st_dev ) ) ;
			sysblksize = d2ds( results.st_blksize ) ;

			irlnk = "" ;
			if ( S_ISLNK( results.st_mode ) )
			{
				while ( true )
				{
					buffer = new char[ bufferSize ] ;
					int rc = readlink( file.c_str(), buffer, bufferSize ) ;
					if ( rc == -1 )
					{
						delete[] buffer ;
						if ( errno == ENAMETOOLONG ) { bufferSize += 255; }
						else                         { break ;            }
					}
					else
					{
						irlnk = string( buffer, rc ) ;
						delete[] buffer ;
						syslink = ( irlnk.front() != '/' ) ?
							    file.substr( 0, file.find_last_of( '/' ) + 1 ) + irlnk : irlnk ;
						break ;
					}
				}
			}
		}
		else
		{
			throw runtime_error( "lstat gave a non-zero return code" ) ;
		}
	}
	catch ( runtime_error& e )
	{
		clear_sysvars( context ) ;
		sysreason  = "0016" ;
		sysmsglvl1 = e.what() ;
		sysmsglvl2 = strerror( errno ) ;
		return 20 ;
	}
	catch ( exception& e )
	{
		clear_sysvars( context ) ;
		sysreason  = "0016" ;
		sysmsglvl1 = e.what() ;
		return 20 ;
	}
	catch (...)
	{
		clear_sysvars( context ) ;
		sysreason  = "0016" ;
		sysmsglvl1 = "An unknown error has occured" ;
		return 20 ;
	}

	context->SetContextVariable( "SYSDSNAME",  context->String( sysdsname.c_str() ) ) ;
	context->SetContextVariable( "SYSDSORG",   context->String( sysdsorg.c_str() ) ) ;
	context->SetContextVariable( "SYSUSED",    context->String( sysused.c_str() ) ) ;
	context->SetContextVariable( "SYSINODE",   context->String( sysinode.c_str() ) ) ;
	context->SetContextVariable( "SYSCHANGE",  context->String( syschange.c_str() ) ) ;
	context->SetContextVariable( "SYSACCESS",  context->String( sysaccess.c_str() ) ) ;
	context->SetContextVariable( "SYSMODIFY",  context->String( sysmodify.c_str() ) ) ;
	context->SetContextVariable( "SYSOWNER",   context->String( sysowner.c_str() ) ) ;
	context->SetContextVariable( "SYSGROUP",   context->String( sysgroup.c_str() ) ) ;
	context->SetContextVariable( "SYSMAJOR",   context->String( sysmajor.c_str() ) ) ;
	context->SetContextVariable( "SYSMINOR",   context->String( sysminor.c_str() ) ) ;
	context->SetContextVariable( "SYSLINK",    context->String( syslink.c_str() ) ) ;
	context->SetContextVariable( "SYSBLKSIZE", context->String( sysblksize.c_str() ) ) ;

	context->SetContextVariable( "SYSREASON",  context->String( sysreason.c_str() ) ) ;
	context->SetContextVariable( "SYSMSGLVL1", context->String( sysmsglvl1.c_str() ) ) ;
	context->SetContextVariable( "SYSMSGLVL2", context->String( sysmsglvl2.c_str() ) ) ;

	return rc ;
}


int TSOENV::Execio( RexxExitContext* context,
		    const string& s )
{
	//
	// Control the reading and writing of data from/to a file.
	//
	//   RETURN:
	//    0 Normal completion.
	//    1 Data truncation during DISKW operation.
	//    2 End of file reached on DISKR before number of lines read.
	//   20 Severe error.  Error message has been issued.
	//
	// DISKW * with STEM option stops at first uninitialised variable - STEM.0 is not used.
	//
	// If reading or writing to the rexx queue, use env variable RXQUEUE to get the queue name or use SESSION if not set.
	// In REXX: x = value( "RXQUEUE", "MYSTACK", "ENVIRONMENT" )   (old value returned in x).
	//
	// Allow any number of readers of a ddname, or just one writer.
	//
	// RC is not set by this method.
	//

	int i ;
	int l ;
	int rc ;
	int queuemode ;

	int sRC = 0 ;

	string val ;
	string path ;
	string name ;
	string line ;

	bool validstem ;

	const char* quename ;

	CONSTRXSTRING queuedata ;
	RXSTRING      result = { 0, nullptr } ;

	RexxObjectPtr optr ;
	RexxStringObject str ;

	string err ;

	guardInt t( RC ) ;

	execio xio( err, s ) ;
	if ( err != "" )
	{
		say( err ) ;
		return 20 ;
	}

	path = p_gls->get_allocated_path( xio.ddn ) ;
	if ( path == "" )
	{
		issueLineMessage( RXTSO_NOTALLOC ) ;
		return 20 ;
	}

	if ( xio.read )
	{
		if ( p_gls->is_open_output( xio.ddn ) )
		{
			issueLineMessage( RXTSO_FILE_OPENOUT ) ;
			return 20 ;
		}
		std::ifstream* fin = p_gls->is_open_input( xio.ddn, taskid() ) ;
		if ( !fin )
		{
			fin = new std::ifstream( path.c_str() ) ;
			if ( !fin->is_open() )
			{
				delete fin ;
				issueLineMessage( RXTSO_OPEN_FAIL ) ;
				return 20 ;
			}
			p_gls->add_open_input( xio.ddn, fin, taskid() ) ;
		}
		if ( xio.fifo || xio.lifo )
		{
			quename = getenv( "RXQUEUE" ) ;
			if ( !quename )
			{
				quename = "SESSION" ;
			}
			queuemode = ( xio.fifo ) ? RXQUEUE_FIFO : RXQUEUE_LIFO ;
		}
		else
		{
			validstem = ( xio.stem.size() < 8 && isvalidName( xio.stem ) ) ;
		}
		l = 0 ;
		i = 0 ;
		while ( ( xio.all || ++l <= xio.num ) && getline( *fin, line ) )
		{
			if ( !xio.skip )
			{
				if ( xio.fifo || xio.lifo )
				{
					MAKERXSTRING( queuedata, line.c_str(), line.size() ) ;
					rc = RexxAddQueue( quename, &queuedata, queuemode ) ;
					if ( rc != RXQUEUE_OK )
					{
						return 20 ;
					}
				}
				else
				{
					name = xio.stem + d2ds( ++i ) ;
					context->SetContextVariable( name.c_str(), context->String( line.c_str(), line.size() ) ) ;
					if ( validstem && name.size() <= 8 )
					{
						vreplace( name, line ) ;
					}
				}
			}
		}
		if ( !xio.fifo && !xio.lifo )
		{
			name = xio.stem + "0" ;
			val  = ( xio.skip ) ? "0" : d2ds( i ) ;
			context->SetContextVariable( name.c_str(), context->String( val.c_str() ) ) ;
			if ( validstem )
			{
				vreplace( name, val ) ;
			}
		}
		if ( fin->bad() )
		{
			sRC = 20 ;
		}
		else if ( fin->eof() && !xio.all && i < xio.num )
		{
			sRC = 2 ;
		}
		if ( xio.finis )
		{
			p_gls->remove_open_input( xio.ddn, taskid() ) ;
		}
	}
	else
	{
		if ( p_gls->is_open_input( xio.ddn ) )
		{
			issueLineMessage( RXTSO_FILE_OPENIN ) ;
			return 20 ;
		}
		std::ofstream* fout = p_gls->is_open_output( xio.ddn, taskid() ) ;
		if ( !fout )
		{
			if ( p_gls->is_open_output( xio.ddn ) )
			{
				issueLineMessage( RXTSO_FILE_OPENOUT ) ;
				return 20 ;
			}
			fout = new std::ofstream( path.c_str() ) ;
			if ( !fout->is_open() )
			{
				delete fout ;
				issueLineMessage( RXTSO_OPEN_FAIL ) ;
				return 20 ;
			}
			p_gls->add_open_output( xio.ddn, fout, taskid() ) ;
		}
		if ( xio.fifo || xio.lifo )
		{
			quename = getenv( "RXQUEUE" ) ;
			if ( !quename )
			{
				quename = "SESSION" ;
			}
			i = 0 ;
			while ( xio.all || ++i <= xio.num )
			{
				rc = RexxPullFromQueue( quename, &result, nullptr, RXQUEUE_NOWAIT ) ;
				if ( rc != RXQUEUE_OK )
				{
					RexxFreeMemory( result.strptr ) ;
					break ;
				}
				*fout << string( result.strptr, result.strlength ) << endl ;
				RexxFreeMemory( result.strptr ) ;
				result = { 0, nullptr } ;
				if ( fout->bad() )
				{
					sRC = 20 ;
					break ;
				}
			}
		}
		else
		{
			for ( i = 1 ; ( xio.all || i <= xio.num ) ; ++i )
			{
				name = xio.stem + d2ds( i ) ;
				optr = context->GetContextVariable( name.c_str() ) ;
				if ( optr == NULLOBJECT ) { break ; }
				str = context->ObjectToString( optr ) ;
				*fout << string( context->StringData( str ), context->StringLength( str ) ) << endl ;
				if ( fout->bad() )
				{
					sRC = 20 ;
					break ;
				}
			}
		}
		if ( xio.finis )
		{
			p_gls->remove_open_output( xio.ddn ) ;
		}
	}

	return sRC ;
}


string TSOENV::outtrap( const string& var,
			int l )
{
	//
	// Start/stop outtrapping of builtin commands.
	//
	//   This is called from the lspf REXX function package.  EG. y = outtrap( VAR, 9 )
	//
	//   RETURN:
	//     Variable name or OFF.
	//

	string v = upper( var ) ;

	if ( v == "OFF" )
	{
		outtrapLines = 0 ;
		outtrapIndex = 0 ;
		outtrapOn    = false ;
		outtrapValid = false ;
	}
	else
	{
		outtrapLines = ( l == -1 ) ? INT_MAX : l ;
		outtrapIndex = 0 ;
		outtrapOn    = true ;
		outtrapVar   = v ;
		outtrapValid = isvalidName( outtrapVar ) ;
	}

	return v ;
}


string TSOENV::sysvar( const string& s )
{
	//
	// SYSVAR function
	// Return current session information.
	//
	// RC is not set by this method.
	//

	string type = upper( s ) ;

	errblk.setRC( 0 ) ;

	guardInt t( RC ) ;

	if ( type == "SYSENV" )
	{
		return ( backgrd ) ? "BACK" : "FORE" ;
	}
	else if ( type == "SYSUID" )
	{
		return p_poolMGR->get( errblk, "ZUSER", SHARED ) ;
	}
	else if ( type == "SYSNEST" )
	{
		return ( nested ) ? "YES" : "NO" ;
	}
	else if ( type == "SYSLTERM" )
	{
		return ( p_poolMGR->get( errblk, "ZSCRMAXD", SHARED ) ) ;
	}
	else if ( type == "SYSWTERM" )
	{
		return ( p_poolMGR->get( errblk, "ZSCRMAXW", SHARED ) ) ;
	}
	else if ( type == "SYSNODE" )
	{
		return p_poolMGR->get( errblk, "ZNODNAME", SHARED ) ;
	}
	else if ( type == "SYSISPF" )
	{
		return "ACTIVE" ;
	}

	return "" ;
}


void TSOENV::clear_sysvars( RexxCallContext* context )
{
	context->SetContextVariable( "SYSDSNAME",  context->String( "" ) ) ;
	context->SetContextVariable( "SYSDSORG",   context->String( "" ) ) ;
	context->SetContextVariable( "SYSUSED",    context->String( "" ) ) ;
	context->SetContextVariable( "SYSINODE",   context->String( "" ) ) ;
	context->SetContextVariable( "SYSCHANGE",  context->String( "" ) ) ;
	context->SetContextVariable( "SYSACCESS",  context->String( "" ) ) ;
	context->SetContextVariable( "SYSMODIFY",  context->String( "" ) ) ;
	context->SetContextVariable( "SYSOWNER",   context->String( "" ) ) ;
	context->SetContextVariable( "SYSGROUP",   context->String( "" ) ) ;
	context->SetContextVariable( "SYSMAJOR",   context->String( "" ) ) ;
	context->SetContextVariable( "SYSMINOR",   context->String( "" ) ) ;
	context->SetContextVariable( "SYSLINK",    context->String( "" ) ) ;
	context->SetContextVariable( "SYSBLKSIZE", context->String( "" ) ) ;
	context->SetContextVariable( "SYSREASON",  context->String( "" ) ) ;
	context->SetContextVariable( "SYSMSGLVL1", context->String( "" ) ) ;
	context->SetContextVariable( "SYSMSGLVL2", context->String( "" ) ) ;
}


void TSOENV::altlib( const string& s,
		     vector<string>& output )
{
	//
	// Set the REXX library search order.
	//
	//   RETURN CODE:
	//    0 Normal completion.
	//    4 Deactivate request did not find alternate library.
	//    8 Application level library already exits - COND specified.
	//   16 Required DDNAME not allocated.
	//   20 Severe error.  See messages.
	//
	// Informational messages are in IKJADM, IKJADM1-99.
	//

	int rc ;
	int sRC = 0 ;

	const char user = 'U' ;
	const char appl = 'A' ;
	const char syst = 'S' ;

	string err ;

	const string sysuexec = "SYSUEXEC" ;
	const string sysexec  = "SYSEXEC" ;

	stack<zaltlib> temp ;

	pair<map<char, stack<zaltlib>>::iterator, bool> result ;

	tso_altlib* altlib = new tso_altlib( err, s ) ;
	if ( err != "" )
	{
		set_shared_var( "IKJADM", "0" ) ;
		delete altlib ;
		say( err ) ;
		RC = 20 ;
		return ;
	}

	BOOST_SCOPE_EXIT( &RC, &sRC, &altlib, &output, &s, &outtrapOn, this_ )
	{
		int i ;
		vector<string>::iterator it ;
		if ( altlib->quiet )
		{
			this_->set_shared_var( "IKJADM", d2ds( output.size() + 1 ) ) ;
			this_->set_shared_var( "IKJADM1", s ) ;
			for ( it = output.begin(), i = 2 ; it != output.end() && i < 100 ; ++it, ++i )
			{
				this_->set_shared_var( "IKJADM" + d2ds( i ), *it ) ;
			}
			output.clear() ;
		}
		else if ( !outtrapOn )
		{
			for ( auto& l : output )
			{
				this_->say( l ) ;
			}
			output.clear() ;
		}

		delete altlib ;
		RC = sRC ;
	}
	BOOST_SCOPE_EXIT_END

	if ( altlib->disp )
	{
		output.push_back( "Current search order (by DDNAME) is:" ) ;
		auto itu = zaltl.find( user ) ;
		auto ita = zaltl.find( appl ) ;
		auto its = zaltl.find( syst ) ;
		if ( itu == zaltl.end() && ita == zaltl.end() && its == zaltl.end() )
		{
			output.push_back( "No levels are currently being searched" ) ;
			return ;
		}
		if ( itu != zaltl.end() )
		{
			output.push_back( "User-level EXEC         DDNAME=" + itu->second.top().ddname ) ;
		}
		if ( ita != zaltl.end() )
		{
			temp = ita->second ;
			output.push_back( "Application-level EXEC  DDNAME=" + temp.top().ddname ) ;
			temp.pop() ;
			while ( !temp.empty() )
			{
				output.push_back( "               Stacked  DDNAME=" + temp.top().ddname ) ;
				temp.pop() ;
			}
		}
		if ( its != zaltl.end() )
		{
			output.push_back( "System-level EXEC       DDNAME=" + its->second.top().ddname ) ;
		}
		return ;
	}

	if ( altlib->reset )
	{
		auto itu = zaltl.find( user ) ;
		auto ita = zaltl.find( appl ) ;
		auto its = zaltl.find( syst ) ;
		if ( itu != zaltl.end() )
		{
			zaltl.erase( itu ) ;
		}
		if ( ita != zaltl.end() )
		{
			while ( !ita->second.empty() )
			{
				if ( ita->second.top().dynalloc() && ita->second.top().tofree )
				{
					p_gls->set_alloc_closed( ita->second.top().ddname ) ;
					p_gls->free( "F(" + ita->second.top().ddname + ")", err ) ;
				}
				ita->second.pop() ;
			}
			zaltl.erase( ita ) ;
		}
		if ( its == zaltl.end() )
		{
			if ( !p_gls->is_ddname_allocated( sysexec ) )
			{
				output.push_back( "DDNAME SYSEXEC is not allocated" ) ;
				output.push_back( "SYSEXEC must be preallocated before calling ALTLIB" ) ;
				sRC = 16 ;
				return ;
			}
			result = zaltl.insert( pair<char, stack<zaltlib>>( syst, stack<zaltlib>() ) ) ;
			result.first->second.push( zaltlib( sysexec ) ) ;
		}
		return ;
	}

	if ( altlib->act )
	{
		if ( altlib->user )
		{
			auto itu = zaltl.find( user ) ;
			if ( itu == zaltl.end() )
			{
				if ( !p_gls->is_ddname_allocated( sysuexec ) )
				{
					output.push_back( "DDNAME SYSUEXEC is not allocated" ) ;
					output.push_back( "SYSUEXEC must be preallocated before calling ALTLIB" ) ;
					sRC = 16 ;
					return ;
				}
				result = zaltl.insert( pair<char, stack<zaltlib>>( user, stack<zaltlib>() ) ) ;
				result.first->second.push( zaltlib( sysuexec ) ) ;
			}
		}
		else if ( altlib->appl )
		{
			auto ita = zaltl.find( appl ) ;
			if ( ita != zaltl.end() && altlib->cond )
			{
				sRC = 8 ;
				return ;
			}
			if ( altlib->path != "" )
			{
				altlib->ddn = p_gls->gn_sysddn() ;
				rc = p_gls->alloc( "ALLOC F(" + altlib->ddn + ") DS(" + altlib->path + ") SHR", err ) ;
				if ( rc > 0 )
				{
					output.push_back( "DDNAME " + altlib->ddn + "cannot be allocated" ) ;
					output.push_back( err ) ;
					sRC = 20 ;
					return ;
				}
			}
			else if ( !p_gls->is_ddname_allocated( altlib->ddn ) )
			{
				output.push_back( "DDNAME " + altlib->ddn + " is not allocated" ) ;
				output.push_back( altlib->ddn + " must be preallocated before calling ALTLIB" ) ;
				sRC = 16 ;
				return ;
			}
			p_gls->set_alloc_open( altlib->ddn ) ;
			if ( ita == zaltl.end() )
			{
				result = zaltl.insert( pair<char, stack<zaltlib>>( appl, stack<zaltlib>() ) ) ;
				ita    = result.first ;
			}
			ita->second.push( zaltlib( altlib->path, altlib->ddn ) ) ;
		}
		else
		{
			auto its = zaltl.find( syst ) ;
			if ( its == zaltl.end() )
			{
				if ( !p_gls->is_ddname_allocated( sysexec ) )
				{
					output.push_back( "DDNAME SYSEXEC is not allocated" ) ;
					output.push_back( "SYSEXEC must be preallocated before calling ALTLIB" ) ;
					sRC = 16 ;
					return ;
				}
				result = zaltl.insert( pair<char, stack<zaltlib>>( syst, stack<zaltlib>() ) ) ;
				result.first->second.push( zaltlib( sysexec ) ) ;
			}
		}
	}
	else
	{
		auto itu = zaltl.find( user ) ;
		auto ita = zaltl.find( appl ) ;
		auto its = zaltl.find( syst ) ;
		if ( altlib->all )
		{
			if ( itu == zaltl.end() && ita == zaltl.end() && its == zaltl.end() )
			{
				sRC = 4 ;
				return ;
			}
			if ( itu != zaltl.end() )
			{
				zaltl.erase( itu ) ;
			}
			if ( ita != zaltl.end() )
			{
				while ( !ita->second.empty() )
				{
					if ( ita->second.top().dynalloc() && ita->second.top().tofree )
					{
						p_gls->set_alloc_closed( ita->second.top().ddname ) ;
						p_gls->free( "F(" + ita->second.top().ddname + ")", err ) ;
					}
					ita->second.pop() ;
				}
				zaltl.erase( ita ) ;
			}
			if ( its != zaltl.end() )
			{
				zaltl.erase( its ) ;
			}
			output.push_back( "No levels are currently being searched" ) ;
		}
		else if ( altlib->user )
		{
			if ( itu == zaltl.end() )
			{
				sRC = 4 ;
				return ;
			}
			zaltl.erase( itu ) ;
		}
		else if ( altlib->appl )
		{
			if ( ita == zaltl.end() )
			{
				sRC = 4 ;
				return ;
			}
			if ( ita->second.top().dynalloc() && ita->second.top().tofree )
			{
				p_gls->set_alloc_closed( ita->second.top().ddname ) ;
				p_gls->free( "F(" + ita->second.top().ddname + ")", err ) ;
			}
			ita->second.pop() ;
			if ( ita->second.empty() )
			{
				zaltl.erase( ita ) ;
			}
		}
		else if ( its == zaltl.end() )
		{
			sRC = 4 ;
		}
		else
		{
			zaltl.erase( its ) ;
		}
	}
}


string TSOENV::sysexec()
{
	//
	// Return the search order for REXX commands.
	//   Order is:
	//     1) User          SYSUEXEC
	//     2) Application   File/dataset operand
	//     3) System        SYSEXEC
	//

	string paths ;

	auto itu = zaltl.find( 'U' ) ;
	auto ita = zaltl.find( 'A' ) ;
	auto its = zaltl.find( 'S' ) ;

	if ( itu != zaltl.end() )
	{
		paths = p_gls->get_allocated_path( "SYSUEXEC" ) ;
	}

	if ( ita != zaltl.end() )
	{
		paths = mergepaths( paths, p_gls->get_allocated_path( ita->second.top().ddname ) ) ;
	}

	if ( its != zaltl.end() )
	{
		paths = mergepaths( paths, p_gls->get_allocated_path( "SYSEXEC" ) ) ;
	}

	return paths ;
}


void TSOENV::set_shared_var( const string& var,
			     const string& val )
{
	//
	// Set the dialogue variable in the SHARED pool.
	//

	string t = val ;

	vdefine( var, &t ) ;
	vput( var, SHARED ) ;
	vdelete( var ) ;
}


void TSOENV::issueLineMessage( int msgid,
			       const string& t )
{
	switch ( msgid )
	{
		case RXTSO_INVCMD:
			say( t + " is an invalid command" ) ;
			break ;

		case RXTSO_INVPARM:
			say( "Invalid parameter passed" ) ;
			break ;

		case RXTSO_NOTALLOC:
			say( "File not allocated" ) ;
			break ;

		case RXTSO_ALLOCATED:
			say( "File is already allocated" ) ;
			break ;

		case RXTSO_FILE_OPEN:
			say( "File is open" ) ;
			break ;

		case RXTSO_FILE_OPENOUT:
			say( "File is already opened for output" ) ;
			break ;

		case RXTSO_FILE_OPENIN:
			say( "File is already opened for input" ) ;
			break ;

		case RXTSO_OPEN_FAIL:
			say( "File cannot be opened" ) ;
			break ;
	}
}
