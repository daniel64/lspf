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


#define RXTSO_OK              0
#define RXTSO_INVCMD          1
#define RXTSO_INVPARM         2
#define RXTSO_NOTALLOC        3
#define RXTSO_ALLOCATED       4
#define RXTSO_ALLOCATED_OTHER 5
#define RXTSO_FILE_OPEN       6
#define RXTSO_FILE_OPENIN     7
#define RXTSO_FILE_OPENOUT    8
#define RXTSO_OPEN_FAIL       9
#define RXTSO_NOFILES        10
#define RXTSO_NOEXIST_DIR    11
#define RXTSO_NOTDIRECTORY   12
#define RXTSO_ACCERR         13
#define RXTSO_ENQFAIL        14
#define RXTSO_CONCAT         15

#undef TASKID
#define TASKID() err.taskid


enum GL_BUILTIN
{
	GL_INVALID,
	GL_ALLOC,
	GL_FREE,
	GL_HELP,
	GL_LISTALC,
	GL_TIME
} ;


class input_task
{
	public:
		input_task()
		{
			opened.clear() ;
		}

		~input_task()
		{
			for ( const auto& f : opened )
			{
				f.second->close() ;
				delete f.second ;
			}
		}

		bool empty()
		{
			return opened.empty() ;
		}

		std::ifstream* get_ifstream( int taskid )
		{
			auto it = opened.find( taskid ) ;
			return ( it != opened.end() ) ? it->second : nullptr ;
		}

		void add( int taskid,
			  std::ifstream* fin )
		{
			opened[ taskid ] = fin ;
		}

		bool remove( int taskid )
		{
			auto it = opened.find( taskid ) ;
			if ( it != opened.end() )
			{
				it->second->close() ;
				delete it->second ;
				opened.erase( it ) ;
				return true ;
			}
			return false ;
		}

	private:
		map<int, std::ifstream*> opened ;
} ;


class output_task
{
	public:
		output_task()
		{
			taskid = 0 ;
			fout   = nullptr ;
		}

		~output_task()
		{
			if ( fout )
			{
				fout->close() ;
				delete fout ;
			}
		}

		std::ofstream* get_ofstream()
		{
			return fout ;
		}

		std::ofstream* get_ofstream( int t )
		{
			return ( taskid == t ) ? fout : nullptr ;
		}

		void set( int i,
			  std::ofstream* j )
		{
			taskid = i ;
			fout   = j ;
		}

		bool match( int t )
		{
			return ( taskid == t ) ;
		}


	private:
		int taskid ;

		std::ofstream* fout ;
} ;


class gls
{
	public:
		gls() ;
		~gls() ;

		static logger* lg ;
		static unsigned int ispddn ;
		static unsigned int sysddn ;

	private:
		boost::mutex mtx ;

		GL_BUILTIN builtin( const string& ) ;

		void help( vector<string>& ) ;

		void listalc( vector<string>& ) ;

		string get_time() ;

		int alloc( const string&,
			   string& ) ;

		const tso_alloc* get_alloc( const string& ) ;

		const string& get_allocated_path( const string& ) ;
		const string& get_allocated_ddname( const string& ) ;

		void  set_alloc_open( const string& ) ;
		void  set_alloc_closed( const string& ) ;

		void  set_alloc_nodynalloc( const string& ) ;

		bool  is_path_allocated( const string& ) ;
		bool  is_path_allocated( const string&,
					 const string&,
					 bool ) ;
		bool  is_ddname_allocated( const string& ) ;

		bool  add_dsn_enqueues( const tso_alloc* ) ;
		void  release_dsn_enqueues( const tso_alloc* ) ;

		bool  has_other_allocs( const string&,
					const string& ) ;

		bool  is_open( const string& ) ;
		bool  is_open( const string&,
			       int ) ;

		bool           is_open_input( const string& ) ;
		std::ifstream* is_open_input( const string&,
					      int ) ;

		std::ofstream* is_open_output( const string& ) ;
		std::ofstream* is_open_output( const string&,
					       int ) ;

		void add_open_input( const string&,
				     std::ifstream*,
				     int ) ;

		void add_open_output( const string&,
				      std::ofstream*,
				      int ) ;

		void remove_open_input( const string&,
					int ) ;
		void remove_open_output( const string& ) ;

		void close_task_files( int ) ;

		bool alloc_is_directory( const string& ) ;
		bool alloc_is_regular_file( const string& ) ;

		int free( const string&,
			  string& ) ;

		string issueLineMessage( int,
					 const string& = "" ) ;

		int enq( const string&,
			 const string&,
			 int,
			 enqDISP  = EXC,
			 enqSCOPE = GLOBAL ) ;

		int deq( const string&,
			 const string&,
			 int,
			 enqSCOPE = GLOBAL ) ;

		int qscan( const string&,
			   const string&,
			   enqDISP  = EXC,
			   enqSCOPE = GLOBAL ) ;

		void release_enqueues_task( int ) ;

		void show_enqueues( errblock& ) ;

		int queryenq( vector<vector<string>>&,
			      string,
			      string,
			      const string&,
			      bool,
			      int ) ;

		string gn_sysddn() { return "SYS" + d2ds( sysddn++, 5 ) ; }
		string gn_ispddn() { return "ISP" + d2ds( ispddn--, 5 ) ; }

		tso_alloc* null_alloc ;

		map<string, input_task> if_opened ;
		map<string, output_task> of_opened ;

		map<string, tso_alloc*> allocs ;

		vector<enqueue>g_enqueues ;

		string modname() { return "GLSERV" ; }

		map<string, GL_BUILTIN> builtin_cmds = { { "ALLOCATE", GL_ALLOC   },
							 { "ALLOC",    GL_ALLOC   },
							 { "FREE",     GL_FREE    },
							 { "HELP",     GL_HELP    },
							 { "LISTA",    GL_LISTALC },
							 { "LISTALC",  GL_LISTALC },
							 { "TIME",     GL_TIME    } } ;

		friend class lss ;
		friend class pApplication ;
		friend class TSOENV ;

		friend void createSystemAlloc( const string&,
					       const string& ) ;
} ;
