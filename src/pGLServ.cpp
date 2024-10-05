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
/* Global lspf services.                                                                                              */
/* Services which are common to all lspf applications and global in scope.                                            */
/*                                                                                                                    */
/* Current functions:                                                                                                 */
/* ALLOCATE                                                                                                           */
/* FREE                                                                                                               */
/* HELP                                                                                                               */
/* LISTALC                                                                                                            */
/* TIME                                                                                                               */
/*                                                                                                                    */
/* Global ENQ/DEQ                                                                                                     */
/*                                                                                                                    */
/* Central store of input/output streams (one writer, many readers).                                                  */
/*                                                                                                                    */
/* Any additions should be added to the the builtin() TSOENV method so they can be used from REXX and the             */
/* command line.                                                                                                      */
/*                                                                                                                    */
/* ****************************************************************************************************************** */


gls::gls()
{
        null_alloc = new tso_alloc() ;
}


gls::~gls()
{
        //
        // Delete any remaining allocate structures.
        //

        for ( const auto& a : allocs )
        {
                delete a.second ;
        }

        delete null_alloc ;
}


GL_BUILTIN gls::builtin( const string& cmd )
{
        //
        // Return the GL_BUILTIN enum for a builtin command.
        //

        auto it = builtin_cmds.find( cmd ) ;

        return ( it == builtin_cmds.end() ) ? GL_INVALID : it->second ;
}


/* *************************************** ********************************************* **************************************** */
/* ***************************************          Builtin Command Processing           **************************************** */
/* *************************************** ********************************************* **************************************** */


int gls::alloc( const string& s,
                string& err )
{
        //
        // Allocate a file or files to a DDNAME.
        //
        //   RETURN:
        //    0 Allocation successful.
        //   12 Allocation not successful.  Error message has been issued.
        //

        size_t j ;

        tso_alloc* alloc = new tso_alloc( err, s ) ;
        if ( err != "" )
        {
                delete alloc ;
                return 12 ;
        }

        if ( is_open( alloc->ddn ) )
        {
                err = issueLineMessage( RXTSO_FILE_OPEN ) ;
                delete alloc ;
                return 12 ;
        }

        if ( !alloc->reuse && allocs.count( alloc->ddn ) > 0 )
        {
                err = issueLineMessage( RXTSO_ALLOCATED ) ;
                delete alloc ;
                return 12 ;
        }

        if ( alloc->create )
        {
                boost::filesystem::path temp =
                boost::filesystem::temp_directory_path() / boost::filesystem::unique_path( "alloc_temp-%%%%-%%%%" ) ;
                alloc->setpath( temp.native() ) ;
                if ( alloc->directory() )
                {
                        create_directory( temp ) ;
                }
        }
        else if ( alloc->dummy )
        {
                alloc->setpath( "/dev/null" ) ;
        }
        else
        {
                for ( j = 0 ; j < alloc->entries() ; ++j )
                {
                        if ( is_path_allocated( alloc->ddn, alloc->getpath( j ), alloc->shr ) )
                        {
                                err = issueLineMessage( RXTSO_ALLOCATED_OTHER ) ;
                                delete alloc ;
                                return 12 ;
                        }
                }
        }

        if ( !alloc->dummy && !add_dsn_enqueues( alloc ) )
        {
                  err = issueLineMessage( RXTSO_ENQFAIL ) ;
                  delete alloc ;
                  return 12 ;
        }

        if ( alloc->reuse )
        {
                auto it = allocs.find( alloc->ddn ) ;
                if ( it != allocs.end() )
                {
                        if ( it->second->is_open() )
                        {
                                err = issueLineMessage( RXTSO_FILE_OPEN ) ;
                                delete alloc ;
                                return 12 ;
                        }
                        delete it->second ;
                        allocs.erase( it ) ;
                }
        }

        allocs[ alloc->ddn ] = alloc ;

        return 0 ;
}


int gls::free( const string& s,
               string& err )
{
        //
        // Free a DDNAME.
        //
        //   RETURN:
        //    0 Free successful.
        //   12 Free not successful.  Error message has been issued.
        //

        bool freed ;

        int sRC = 0 ;

        tso_free f( err, s ) ;
        if ( err != "" )
        {
                return 12 ;
        }

        if ( f.all )
        {
                if ( allocs.size() == 0 )
                {
                        err = issueLineMessage( RXTSO_NOFILES ) ;
                        return 8 ;
                }
                for ( auto it = allocs.begin() ; it != allocs.end() ; )
                {
                        if ( !it->second->dynalloc() )
                        {
                                ++it ;
                        }
                        else if ( it->second->is_open() || is_open( it->second->ddn ) )
                        {
                                err = issueLineMessage( RXTSO_FILE_OPEN ) ;
                                sRC = 12 ;
                                ++it ;
                        }
                        else
                        {

                                release_dsn_enqueues( it->second ) ;
                                delete it->second ;
                                it = allocs.erase( it ) ;
                        }
                }
                return sRC ;
        }

        for ( const auto& ddn : f.ddns )
        {
                if ( is_open( ddn ) )
                {
                        err = issueLineMessage( RXTSO_FILE_OPEN ) ;
                        return 12 ;
                }
                auto it = allocs.find( ddn ) ;
                if ( it != allocs.end() )
                {
                        if ( it->second->is_open() )
                        {
                                err = issueLineMessage( RXTSO_FILE_OPEN ) ;
                                return 12 ;
                        }
                        release_dsn_enqueues( it->second ) ;
                        delete it->second ;
                        allocs.erase( it ) ;
                }
                else
                {
                        err = issueLineMessage( RXTSO_NOTALLOC ) ;
                        return 12 ;
                }
        }

        for ( const auto& path : f.paths )
        {
                freed = false ;
                for ( auto it = allocs.begin() ; it != allocs.end() ; )
                {
                        if ( it->second->entries() > 1 && it->second->find( path ) )
                        {
                                err = issueLineMessage( RXTSO_CONCAT ) ;
                                return 12 ;
                        }
                        if ( path == it->second->path )
                        {
                                if ( it->second->is_open() || is_open( it->first ) )
                                {
                                        err = issueLineMessage( RXTSO_FILE_OPEN ) ;
                                        return 12 ;
                                }
                                release_dsn_enqueues( it->second ) ;
                                delete it->second ;
                                it = allocs.erase( it ) ;
                                freed = true ;
                        }
                        else
                        {
                                ++it ;
                        }
                }
                if ( !freed )
                {
                        err = issueLineMessage( RXTSO_NOTALLOC ) ;
                        sRC = 12 ;
                }
        }

        return sRC ;
}


void gls::help( vector<string>& v )
{
        //
        // List some very simple help for the builtin commands.
        //

        v.push_back( "ALLOCATE     Allocate a file to a ddname") ;
        v.push_back( "ALLOC" ) ;
        v.push_back( "FREE         Free a file allocation") ;
        v.push_back( "LISTALC      List allocation status") ;
        v.push_back( "LISTA" ) ;
        v.push_back( "TIME         Display the time and date") ;
}


void gls::listalc( vector<string>& v )
{
        //
        // Return the allocation status.
        //

        string disp ;

        for ( const auto& al : allocs )
        {
                disp = ( al.second->create ) ? "NEW" :
                       ( al.second->shr    ) ? "SHR" : "OLD" ;
                if ( al.second->del )
                {
                        disp += ",DELETE" ;
                }
                v.push_back( ( al.second->dummy ) ? "DUMMY" : al.second->path ) ;
                v.push_back( "  " + left( al.first, 9 ) + disp ) ;
        }
}


string gls::get_time()
{
        //
        // Return the current date and time.
        //

        char buf[ 60 ] ;

        time_t rawtime ;
        struct tm* time_info = nullptr ;

        time( &rawtime ) ;

        time_info = localtime( &rawtime ) ;

        strftime( buf, sizeof( buf ), "The time is %H:%M:%S on %A, %d %B %Y", time_info ) ;

        return buf ;
}


/* *************************************** ********************************************* **************************************** */
/* ***************************************              Allocate Functions               **************************************** */
/* *************************************** ********************************************* **************************************** */


const tso_alloc* gls::get_alloc( const string& ddn )
{
        //
        // Retrieve the alloc structure address for an allocation by DDNAME.
        //

        auto it = allocs.find( upper( ddn ) ) ;

        return ( it != allocs.end() ) ? it->second : null_alloc ;
}


const string& gls::get_allocated_path( const string& ddn )
{
        //
        // Retrieve the file for an allocation by DDNAME.
        //

        auto it = allocs.find( upper( ddn ) ) ;

        return ( it != allocs.end() ) ? it->second->path : null_alloc->path ;
}


const string& gls::get_allocated_ddname( const string& file )
{
        //
        // Retrieve the ddname for an allocation by file name.
        //

        for ( auto it = allocs.begin() ; it != allocs.end() ; ++it )
        {
                if ( it->second->find( file ) )
                {
                        return it->first ;
                }
        }

        return null_alloc->ddn ;
}


void gls::set_alloc_open( const string& ddn )
{
        //
        // Set the ddname open.
        //

        auto it = allocs.find( upper( ddn ) ) ;
        if ( it != allocs.end() )
        {
                it->second->set_open() ;
        }
}


void gls::set_alloc_closed( const string& ddn )
{
        //
        // Set the ddname closed.
        //

        auto it = allocs.find( upper( ddn ) ) ;
        if ( it != allocs.end() )
        {
                it->second->set_closed() ;
        }
}


void gls::set_alloc_nodynalloc( const string& ddn )
{
        //
        // Set the ddname not dynamically allocated.
        //

        auto it = allocs.find( upper( ddn ) ) ;
        if ( it != allocs.end() )
        {
                it->second->set_nodynalloc() ;
        }
}


bool gls::is_ddname_allocated( const string& ddn )
{
        //
        // Return true if the ddname is allocated.
        //

        return ( allocs.find( upper( ddn ) ) != allocs.end() ) ;
}


bool gls::is_path_allocated( const string& path )
{
        //
        // Return true if the path is allocated.
        //

        return ( get_allocated_ddname( path ) != "" ) ;
}


bool gls::is_path_allocated( const string& ddn,
                             const string& path,
                             bool shr )
{
        //
        // Return true if the path is allocated somewhere other than ddn,
        // and has an incompatible disposition (either requested OLD or held OLD).
        //

        tso_alloc* alloc ;

        for ( auto it = allocs.begin() ; it != allocs.end() ; ++it )
        {
                alloc = it->second ;
                if ( alloc->ddn != ddn )
                {
                        if ( alloc->find( path ) && ( !shr || alloc->old ) )
                        {
                                return true ;
                        }
                }
        }

        return false ;
}


bool gls::alloc_is_directory( const string& ddn )
{
        //
        // Return true if all entries for ddname ddn are directories.
        //

        auto it = allocs.find( upper( ddn ) ) ;

        return it->second->directory() ;
}


bool gls::alloc_is_regular_file( const string& ddn )
{
        //
        // Return true if entry for ddname ddn is a file.
        //

        auto it = allocs.find( upper( ddn ) ) ;

        return it->second->file() ;
}


bool gls::add_dsn_enqueues( const tso_alloc* alloc )
{
        //
        // Add SYSDSN enqueue for each file in an allocation.  Return false if any fail.
        //

        const string sysdsn = "SYSDSN" ;

        enqDISP disp = ( alloc->shr ) ? SHR : EXC ;

        for ( size_t j = 0 ; j < alloc->entries() ; ++j )
        {
                if ( enq( sysdsn, alloc->getpath( j ), 0, disp, GLOBAL ) > 0 )
                {
                        return false ;
                }
        }

        return true ;
}


void gls::release_dsn_enqueues( const tso_alloc* alloc )
{
        //
        // Release SYSDSN enqueue for each file in an allocation.
        //

        for ( size_t i = 0 ; i < alloc->entries() ; ++i )
        {
                const string& path = alloc->getpath( i ) ;
                if ( !has_other_allocs( alloc->ddn, path ) )
                {
                        deq( "SYSDSN", path, 0, GLOBAL ) ;
                }
        }
}


bool gls::has_other_allocs( const string& ddn,
                            const string& path )
{
        //
        // Return true if the path is allocated other than to ddn.
        //

        for ( auto it = allocs.begin() ; it != allocs.end() ; ++it )
        {
                if ( ddn != it->first && it->second->find( path ) )
                {
                        return true ;
                }
        }

        return false ;
}


/* *************************************** ********************************************* **************************************** */
/* ***************************************             I/O Stream Functions              **************************************** */
/* *************************************** ********************************************* **************************************** */


bool gls::is_open( const string& ddn )
{
        //
        // Return true if the file for ddname ddn is open, either input or output.
        //

        return ( is_open_input( ddn ) || is_open_output( ddn ) ) ;
}


bool gls::is_open( const string& ddn,
                   int taskid )
{
        //
        // Return true if the file for ddname ddn is open for taskid, either input or output.
        //

        return ( is_open_input( ddn, taskid ) || is_open_output( ddn, taskid ) ) ;
}


bool gls::is_open_input( const string& ddn )
{
        //
        // Return true if the file for ddname ddn is open input anywhere.
        //

        return ( if_opened.count( ddn ) > 0 ) ;
}


std::ifstream* gls::is_open_input( const string& ddn,
                                   int taskid )
{
        //
        // Return ifstream address if the file for ddname ddn is open input for taskid.
        //

        auto it = if_opened.find( ddn ) ;

        return ( it == if_opened.end() ) ? nullptr : it->second.get_ifstream( taskid ) ;
}


std::ofstream* gls::is_open_output( const string& ddn )
{
        //
        // Return ofstream address if the file for ddname ddn is open output.
        //

        auto it = of_opened.find( ddn ) ;

        return ( it == of_opened.end() ) ? nullptr : it->second.get_ofstream() ;
}


std::ofstream* gls::is_open_output( const string& ddn,
                                    int taskid )
{
        //
        // Return ofstream address if the file for ddname ddn is open output for taskid.
        //

        auto it = of_opened.find( ddn ) ;

        return ( it == of_opened.end() ) ? nullptr : it->second.get_ofstream( taskid ) ;
}


void gls::add_open_input( const string& ddn,
                          std::ifstream* fin,
                          int taskid )
{
        //
        // Add ifstream and task to if_opened map.
        //

        auto it = if_opened.find( ddn ) ;

        if ( it == if_opened.end() )
        {
                auto result = if_opened.insert( pair<string, input_task>( make_pair( ddn, input_task() ) ) ) ;
                it = result.first ;
        }

        it->second.add( taskid, fin ) ;

        set_alloc_open( ddn ) ;
}


void gls::add_open_output( const string& ddn,
                           std::ofstream* fout,
                           int taskid )
{
        //
        // Add ofstream and task to of_opened map.
        //

        auto result = of_opened.insert( pair<string, output_task>( make_pair( ddn, output_task() ) ) ) ;
        result.first->second.set( taskid, fout ) ;

        set_alloc_open( ddn ) ;
}


void gls::remove_open_input( const string& ddn,
                             int taskid )
{
        //
        // Remove ifstream for ddn/taskid.
        //
        // NOTE: The remove() method closes and deletes the open file handle.
        //       The input_task destructor closes and deletes all remaining open file handles.
        //

        auto it = if_opened.find( ddn ) ;

        if ( it != if_opened.end() )
        {
                if ( it->second.remove( taskid ) )
                {
                        set_alloc_closed( ddn ) ;
                        if ( it->second.empty() )
                        {
                                if_opened.erase( it ) ;
                        }
                }
        }
}


void gls::remove_open_output( const string& ddn )
{
        //
        // Remove ofstream for ddn.
        //
        // NOTE: The output_task destructor closes and deletes the open file handle.
        //

        of_opened.erase( ddn ) ;

        set_alloc_closed( ddn ) ;
}


void gls::close_task_files( int taskid )
{
        //
        // Close all files and delete ifstream/ofstream for taskid.
        //

        for ( auto it = if_opened.begin() ; it != if_opened.end() ; )
        {
                if ( it->second.remove( taskid ) )
                {
                        set_alloc_closed( it->first ) ;
                        if ( it->second.empty() )
                        {
                                it = if_opened.erase( it ) ;
                        }
                        else
                        {
                                ++it ;
                        }
                }
                else
                {
                        ++it ;
                }
        }

        for ( auto it = of_opened.begin() ; it != of_opened.end() ; )
        {
                if ( it->second.match( taskid ) )
                {
                        set_alloc_closed( it->first ) ;
                        it = of_opened.erase( it ) ;
                }
                else
                {
                        ++it ;
                }
        }
}


/* *************************************** ********************************************* **************************************** */
/* ***************************************           Global Enqueue Processing           **************************************** */
/* *************************************** ********************************************* **************************************** */


int gls::enq( const string& maj,
              const string& min,
              int taskid,
              enqDISP disp,
              enqSCOPE scope )
{
        //
        // Enqueue a resource with QNAME/RNAME
        //
        //   RETURN:
        //    0 Normal completion.
        //      Not held by this task and added.
        //      Already held by this task for this disposition.
        //      Only held by this task and disposition upgraded from SHR to EXCL.
        //    8 Enqueue already held.
        //   20 Severe error.
        //

        boost::lock_guard<boost::mutex> lock( mtx ) ;

        for ( auto it = g_enqueues.begin() ; it != g_enqueues.end() ; ++it )
        {
                if ( it->maj_name == maj && it->min_name == min )
                {
                        if ( it->tasks.count( taskid ) > 0 )
                        {
                                if ( it->disp == disp )
                                {
                                        return 0 ;
                                }
                                if ( it->tasks.size() == 1 && it->disp == SHR )
                                {
                                        g_enqueues.erase( it ) ;
                                        g_enqueues.push_back( enqueue( maj, min, taskid, disp ) ) ;
                                        return 0 ;
                                }
                        }
                        else if ( it->disp == SHR && disp == SHR )
                        {
                                it->tasks.insert( taskid ) ;
                                return 0 ;
                        }
                        return 8 ;
                }
        }

        g_enqueues.push_back( enqueue( maj, min, taskid, disp ) ) ;

        return 0 ;
}


int gls::deq( const string& maj,
              const string& min,
              int taskid,
              enqSCOPE scope )
{
        //
        // Denqueue a resource with QNAME/RNAME
        //
        //   RETURN:
        //    0 Normal completion.
        //    8 Enqueue not held by this task.
        //   20 Severe error.
        //

        int rc = 8 ;

        boost::lock_guard<boost::mutex> lock( mtx ) ;

        for ( auto it = g_enqueues.begin() ; it != g_enqueues.end() ; ++it )
        {
                if ( it->maj_name == maj && it->min_name == min )
                {
                        if ( it->tasks.count( taskid ) > 0 )
                        {
                                it->tasks.erase( taskid ) ;
                                if ( it->tasks.size() == 0 )
                                {
                                        g_enqueues.erase( it ) ;
                                }
                                rc = 0 ;
                        }
                        break ;
                }
        }

        return rc ;
}


int gls::qscan( const string& maj,
                const string& min,
                enqDISP disp,
                enqSCOPE scope )
{
        //
        // Scan for an enqueue.  Match on qname, rname, disposition and scope.
        //
        //   RETURN:
        //    0 Enqueue held.
        //    8 Enqueue is not held.
        //   20 Severe error.
        //

        int rc = 8 ;

        boost::lock_guard<boost::mutex> lock( mtx ) ;

        for ( auto it = g_enqueues.begin() ; it != g_enqueues.end() ; ++it )
        {
                if ( it->maj_name == maj && it->min_name == min )
                {
                        if ( it->disp == disp )
                        {
                                rc = 0 ;
                        }
                        return rc ;
                }
        }

        return rc ;
}


void gls::release_enqueues_task( int taskid )
{
        //
        // Release all enqueues for taskid.
        //

        for ( auto it = g_enqueues.begin() ; it != g_enqueues.end() ; )
        {
                if ( it->tasks.count( taskid ) > 0 )
                {
                        it->tasks.erase( taskid ) ;
                        if ( it->tasks.size() == 0 )
                        {
                                it = g_enqueues.erase( it ) ;
                                continue ;
                        }
                }
                ++it ;
        }
}


int gls::queryenq( vector<vector<string>>& v,
                   string qname,
                   string rname,
                   const string& req,
                   bool wait,
                   int limit )
{
        //
        // Get a list of enqueues that match QNAME and RNAME and place in table v.
        //
        //   RETURN:
        //    0 Normal completion
        //    4 Table truncated
        //
        // QNAME and RNAME may be prefixes.
        //

        int recs = 0 ;

        bool qn_prefix = false ;
        bool rn_prefix = false ;

        vector<string> row ;

        if ( qname.back() == '*' )
        {
                qn_prefix = true ;
                qname.pop_back() ;
        }

        if ( rname.back() == '*' )
        {
                rn_prefix = true ;
                rname.pop_back() ;
        }

        auto it = g_enqueues.begin() ;
        for ( ; it != g_enqueues.end() && ( limit == 0 || recs < limit ) ; ++it )
        {
                if ( qn_prefix )
                {
                        if ( qname != "" && !abbrev( it->maj_name, qname ) )
                        {
                                continue ;
                        }
                }
                else if ( qname != it->maj_name )
                {
                        continue ;
                }
                if ( rn_prefix )
                {
                        if ( rname != "" && !abbrev( it->min_name, rname ) )
                        {
                                continue ;
                        }
                }
                else if ( rname != it->min_name )
                {
                        continue ;
                }
                for ( auto itt = it->tasks.begin() ; itt != it->tasks.end() ; ++itt )
                {
                        row.push_back( d2ds( *itt, 8 ) ) ;
                        row.push_back( it->maj_name ) ;
                        row.push_back( it->min_name ) ;
                        row.push_back( ( ( it->disp == EXC ) ? "EXCLUSIVE" : "SHARE" ) ) ;
                        ++recs ;
                }
                v.push_back( row ) ;
                row.clear() ;
        }

        return ( it != g_enqueues.end() ? 4 : 0 ) ;
}


void gls::show_enqueues( errblock& err )
{
        //
        // Display Enqueues.
        //

        boost::lock_guard<boost::mutex> lock( mtx ) ;

        llog( "I", ".ENQ" << endl ) ;
        llog( "-", "*************************************************************************************************************" << endl ) ;
        llog( "-", "Global enqueue vector size is "<< g_enqueues.size() << endl ) ;

        llog( "-", "" << endl ) ;
        llog( "-", "Global enqueues held by task "<< d2ds( err.taskid, 8 ) << endl ) ;
        llog( "-", "Exc/Share  Major Name  Minor Name "<< endl ) ;
        llog( "-", "---------  ----------  ---------- "<< endl ) ;

        for ( auto it = g_enqueues.begin() ; it != g_enqueues.end() ; ++it )
        {
                if ( it->tasks.count( err.taskid ) == 0 ) { continue ; }
                llog( "-", "" << setw( 11 ) << std::left << ( it->disp == EXC ? "EXCLUSIVE" : "SHARE" )
                              << setw( 8 )  << std::left << it->maj_name << "    " << it->min_name << endl ) ;
        }

        llog( "-", ""<< endl ) ;
        llog( "-", "All global enqueues"<< endl ) ;
        llog( "-", "Task      Exc/Share  Major Name  Minor Name "<< endl ) ;
        llog( "-", "--------  ---------  ----------  ---------- "<< endl ) ;

        for ( auto it = g_enqueues.begin() ; it != g_enqueues.end() ; ++it )
        {
                for ( auto itt = it->tasks.begin() ; itt != it->tasks.end() ; ++itt )
                {
                        llog( "-", "" << d2ds( *itt, 8 ) << "  "
                                      << setw( 11 ) << std::left << ( it->disp == EXC ? "EXCLUSIVE" : "SHARE" )
                                      << setw( 8 )  << it->maj_name << "    " << it->min_name << endl ) ;
                }
        }

        llog( "-", "*************************************************************************************************************" << endl ) ;
}


/* *************************************** ********************************************* **************************************** */
/* ***************************************           Error Message Processing            **************************************** */
/* *************************************** ********************************************* **************************************** */


string gls::issueLineMessage( int msgid,
                              const string& t )
{
        switch ( msgid )
        {
                case RXTSO_INVCMD:
                        return t + " is an invalid command" ;

                case RXTSO_NOTALLOC:
                        return "File not allocated" ;

                case RXTSO_ALLOCATED:
                        return "File is already allocated" ;

                case RXTSO_ALLOCATED_OTHER:
                        return "File is already allocated to another ddname" ;

                case RXTSO_FILE_OPEN:
                        return "File is open" ;

                case RXTSO_FILE_OPENOUT:
                        return "File is already opened for output" ;

                case RXTSO_FILE_OPENIN:
                        return "File is already opened for input" ;

                case RXTSO_OPEN_FAIL:
                        return "File cannot be opened" ;

                case RXTSO_NOFILES:
                        return "No files allocated" ;

                case RXTSO_NOEXIST_DIR:
                        return "A directory in the concatination does not exist" ;

                case RXTSO_NOTDIRECTORY:
                        return "Entry is not a directory" ;

                case RXTSO_ACCERR:
                        return "Error accessing entry" ;

                case RXTSO_ENQFAIL:
                        return "Failure enqueuing file" ;

                case RXTSO_CONCAT:
                        return "Path cannot be freed - is part of one or more a concatinated allocation." ;

        }

        return "" ;
}
