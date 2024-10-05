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

#undef  TASKID
#define TASKID() 0


enum TB_SERVICES
{
	TB_ADD,
	TB_BOTTOM,
	TB_CREATE,
	TB_DELETE,
	TB_EXISTS,
	TB_GET,
	TB_MOD,
	TB_NONE,
	TB_OPEN,
	TB_PUT,
	TB_QUERY,
	TB_SARG,
	TB_SAVE,
	TB_SCAN,
	TB_SKIP,
	TB_SORT,
	TB_STATS,
	TB_TOP,
	TB_VCLEAR
} ;


class Table
{
	public:
		static uint pflgToken ;

		Table()
		{
			CRP           = 0     ;
			max_urid      = 0     ;
			max_rid       = 0     ;
			lastcc        = 0     ;
			rowcreat      = 0     ;
			tableupd      = 0     ;
			utime         = 0     ;
			firstMult     = true  ;
			changed       = false ;
			updated       = false ;
			pr_create     = false ;
			tab_cmds      = false ;
			tab_klst      = false ;
			tab_ipath     = ""    ;
			tab_opath     = ""    ;
			tab_user      = ""    ;
			sa_namelst    = ""    ;
			sa_cond_pairs = ""    ;
			sa_dir        = "NEXT";
			sort_ir       = ""    ;
			tab_service   = TB_NONE ;
		}

		Table( const string& name,
		       const string& keys,
		       const string& flds,
		       tbWRITE tb_WRITE,
		       tbDISP  tb_DISP,
		       uint maxid ) : Table()
		{
			tab_name  = name ;
			id        = maxid ;
			tab_WRITE = tb_WRITE ;
			tab_DISP  = tb_DISP ;
			tab_keys1 = space( keys ) ;
			num_keys  = words( keys ) ;
			tab_flds  = space( flds ) ;
			num_flds  = words( flds ) ;
			tab_all1  = strip( tab_keys1 + " " + tab_flds ) ;
			num_all   = num_keys + num_flds ;
			tab_cmds  = ( tab_all1 == "ZCTVERB ZCTTRUNC ZCTACT ZCTDESC" ) ;
			tab_klst  = ( num_all  == 74 && strip( keys ) == "KEYLISTN" ) ;
			changed   = true ;
			for ( uint i = 1 ; i <= num_keys ; ++i )
			{
				string temp = word( tab_keys1, i ) ;
				tab_all2.push_back( temp ) ;
				tab_keys2.push_back( temp ) ;
			}
			for ( uint i = 1 ; i <= num_flds ; ++i )
			{
				tab_all2.push_back( word( tab_flds, i ) ) ;
			}
			time( &ctime ) ;
		}

		~Table() ;

	private:
		bool    tab_cmds      ;
		bool    tab_klst      ;
		string  tab_name      ;
		string  tab_keys1     ;
		string  tab_flds      ;
		string  tab_all1      ;
		string  tab_ipath     ;
		string  tab_opath     ;
		string  tab_user      ;
		bool    firstMult     ;
		bool    changed       ;
		bool    updated       ;
		bool    pr_create     ;
		uint    id            ;
		uint    max_urid      ;
		uint    max_rid       ;
		uint    num_keys      ;
		uint    num_flds      ;
		uint    num_all       ;
		uint    CRP           ;
		uint    CRPX          ;
		uint    lastcc        ;
		uint    rowcreat      ;
		uint    tableupd      ;
		string  sort_ir       ;
		string  sa_namelst    ;
		string  sa_cond_pairs ;
		string  sa_dir        ;
		tbDISP  tab_DISP      ;
		tbWRITE tab_WRITE     ;

		time_t ctime ;
		time_t utime ;

		TB_SERVICES tab_service ;

		vector<vector<string>*> table ;

		vector<tbsearch> sarg ;
		vector<string> tab_keys2 ;
		vector<string> tab_all2 ;

		map<uint,uint> urid2rid ;
		map<uint,uint> rid2urid ;

		map<uint,uint>openTasks ;

		int    taskid() { return 0 ; }

		bool   tableClosedforTask( const errblock& ) ;

		bool   tableOpenedforTask( const errblock& ) ;

		void   addTasktoTable( const errblock& ) ;

		void   removeTaskUsefromTable( errblock& ) ;

		void   removeTaskfromTable( errblock& ) ;

		bool   notInUse() ;

		string listTasks() ;

		uint   getid() { return id ; }

		void   set_path( const string& ) ;

		void loadRows( errblock&,
			       std::ifstream*,
			       const string&,
			       const string&,
			       const string&,
			       const string&,
			       uint,
			       uint,
			       uint ) ;

		void   saveTable( errblock&,
				  const string&,
				  const string& ) ;

		void   loadRow( errblock&,
				vector<string>* ) ;

		void   reserveSpace( int ) ;

		void   reset_changed() { changed = false ; }

		void   storeIntValue( errblock&,
				      fPOOL*,
				      const string&,
				      int,
				      int = 8 ) ;

		void   fillfVARs( errblock&,
				  fPOOL*,
				  int,
				  const set<string>&,
				  const set<string>&,
				  map<string, pair<string,uint>>&,
				  bool,
				  int,
				  int,
				  int&,
				  int&,
				  int&,
				  int&,
				  char = ' ',
				  char = ' ',
				  int  = 0,
				  int  = 0 ) ;

		bool   matchsarg( int,
				  vector<string>* ) ;

		int    setscroll( int,
				  bool,
				  char,
				  char,
				  int,
				  int,
				  int,
				  int ) ;


		void   cmdsearch( errblock&,
				  fPOOL*,
				  const string& ) ;


		vector<vector<string>*>::iterator getKeyItr( errblock&,
							     fPOOL* ) ;

		void   loadfuncPool( errblock&,
				     fPOOL*,
				     const string& ) ;

		void   storeExtensionVarNames( errblock&,
					       fPOOL*,
					       const string& ) ;

		void   loadFields( errblock&,
				   fPOOL*,
				   const string&,
				   vector<string>* ) ;

		void   storeFields( errblock&,
				    fPOOL*,
				    const string&,
				    const string&,
				    const string&,
				    const string& ) ;

		void   tbadd( errblock&,
			      fPOOL*,
			      const string&,
			      const string&,
			      int )  ;

		void   tbbottom( errblock&,
				 fPOOL*,
				 const string&,
				 const string&,
				 const string&,
				 const string& ) ;

		void   tbdelete( errblock&,
				 fPOOL* ) ;

		void   tbexist( errblock&,
				fPOOL* ) ;

		void   tbget( errblock&,
			      fPOOL*,
			      const string&,
			      const string&,
			      const string&,
			      const string& ) ;

		void   tbmod( errblock&,
			      fPOOL*,
			      const string&,
			      const string& ) ;

		void   tbput( errblock&,
			      fPOOL*,
			      const string&,
			      const string& ) ;

		void   tbquery( errblock&,
				fPOOL*,
				const string&,
				const string&,
				const string&,
				const string&,
				const string&,
				const string&,
				const string&,
				const string&,
				const string&,
				const string& ) ;

		void   tbquery( bool& ) ;

		void   tbquery( uint& ) ;

		void   tbsarg( errblock&,
			       fPOOL*,
			       string,
			       const string&,
			       string ) ;

		void   setscan( errblock&,
				fPOOL*,
				vector<tbsearch>&,
				set<string>&,
				const string&,
				const string& ) ;

		void   tbscan( errblock&,
			       fPOOL*,
			       int,
			       string,
			       const string&,
			       const string&,
			       const string&,
			       const string&,
			       const string&,
			       string ) ;

		void   tbskip( errblock&,
			       fPOOL*,
			       int,
			       const string&,
			       const string&,
			       const string&,
			       const string&,
			       const string& ) ;

		void   tbskip( errblock&,
			       fPOOL*,
			       const string&,
			       const string&,
			       const string&,
			       const string& ) ;

		void   tbsort( errblock&,
			       string ) ;

		void tbstats( errblock&,
			      fPOOL*,
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "" ) ;

		void   tbtop( errblock& ) ;

		void   tbvclear( errblock&,
				 fPOOL* ) ;

		void   add_rid( uint,
				uint ) ;

		void   del_rid( vector<vector<string>*>::iterator ) ;

		int    get_virtsize() ;

		string create_header() ;

		void  parse_header( const string& ) ;

		bool  has_extvars( uint ) ;

		bool  has_extvars( vector<string>* ) ;

		const string& get_extvar_list( uint ) ;

		const string& get_extvar_list( vector<string>* ) ;

		int   get_extvar_pos( uint,
				      const string& ) ;

		int   get_extvar_pos( vector<string>*,
				      const string& ) ;

		void  set_service_cc( TB_SERVICES a,
				      uint b )
		{
			tab_service = a ;
			lastcc      = b ;
		}

		string modname() { return "TABLE" ; }

		friend class tableMGR ;
} ;


class tableMGR
{
	public:
		tableMGR( int ) ;
		tableMGR() ;
		~tableMGR() ;

		static logger* lg ;

		Table* tbopen( errblock&,
			       const string&,
			       tbWRITE = WRITE,
			       const string& = "",
			       tbDISP = NON_SHARE ) ;

		Table* loadTable( errblock&,
				  const string&,
				  const string&,
				  tbWRITE = WRITE,
				  tbDISP = NON_SHARE,
				  bool = false ) ;

		void   saveTable( errblock&,
				  const string&,
				  const string&,
				  const string&,
				  const string& ) ;

		void   tbadd( errblock&,
			      fPOOL*,
			      const string&,
			      const string&,
			      const string&,
			      int ) ;

		void tbcreate( errblock&,
			       const string&,
			       const string&,
			       const string&,
			       tbWRITE = NOWRITE,
			       tbREP = NOREPLACE,
			       const string& = "",
			       tbDISP = NON_SHARE ) ;

		void tbsort( errblock&,
			     const string&,
			     const string& ) ;

		void tbstats( errblock&,
			      fPOOL*,
			      const string&,
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "" ) ;

		void statistics() ;

		void cmdsearch( errblock&,
				fPOOL*,
				string,
				const string&,
				const string&,
				bool ) ;

	private:
		int zswind ;

		uint maxId ;

		multimap<string, Table*> tables ;

		map<Table*, boost::filesystem::path> table_enqs ;

		boost::recursive_mutex mtx ;

		multimap<string, Table*>::iterator getTableIterator1( errblock&,
								      const string& ) ;

		multimap<string, Table*>::iterator getTableIterator2( errblock&,
								      const string& ) ;

		Table* getTableAddress1( errblock&,
					 const string& ) ;

		Table* getTableAddress2( errblock&,
					 const string& ) ;

		Table* createTable( errblock&,
				    const string&,
				    const string&,
				    const string&,
				    tbWRITE,
				    tbDISP ) ;

		void   enq( Table*,
			    const string& ) ;

		void   deq( Table* ) ;

		Table* qscan( const string& ) ;

		uint   getid( errblock&,
			      const string& ) ;

		void   fillfVARs( errblock&,
				  fPOOL*,
				  const string&,
				  const set<string>&,
				  const set<string>&,
				  map<string, pair<string,uint>>&,
				  bool,
				  int,
				  int,
				  int&,
				  int&,
				  int&,
				  int&,
				  char = ' ',
				  char = ' ',
				  int = 0,
				  int = 0 ) ;

		void   destroyTable( errblock&,
				     const string&,
				     const string& = "" ) ;

		void   closeTablesforTask( errblock& ) ;

		void   qtabopen( errblock&,
				 fPOOL*,
				 const string& ) ;

		void   tbbottom( errblock&,
				 fPOOL*,
				 const string&,
				 const string&,
				 const string&,
				 const string&,
				 const string& ) ;

		void   tbdelete( errblock&,
				 fPOOL*,
				 const string& ) ;

		void   tberase( errblock&,
				const string&,
				const string& ) ;

		void   tbexist( errblock&,
				fPOOL*,
				const string& ) ;

		void   tbget( errblock&,
			      fPOOL*,
			      const string&,
			      const string&,
			      const string&,
			      const string&,
			      const string&  ) ;

		void   tbmod( errblock&r,
			      fPOOL*,
			      const string&,
			      const string&,
			      const string& ) ;

		void   tbput( errblock&,
			      fPOOL*,
			      const string&,
			      const string&,
			      const string& ) ;

		void   tbquery( errblock&,
				fPOOL*,
				const string&,
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "" ) ;

		void   tbquery( errblock&,
				const string&,
				bool& ) ;

		void   tbquery( errblock&,
				const string&,
				uint& ) ;

		void   tbsarg( errblock&,
			       fPOOL*,
			       const string&,
			       const string&,
			       const string&,
			       const string& ) ;

		void   tbskip( errblock&,
			       fPOOL*,
			       const string&,
			       int,
			       const string& = "",
			       const string& = "",
			       const string& = "",
			       const string& = "",
			       const string& = "" ) ;

		void   tbskip( errblock&,
			       fPOOL*,
			       const string&,
			       const string&,
			       const string&,
			       const string&,
			       const string& ) ;

		void   tbscan( errblock&,
			       fPOOL*,
			       const string&,
			       const string& = "",
			       const string& = "",
			       const string& = "",
			       const string& = "",
			       const string& = "",
			       const string& = "",
			       const string& = "" ) ;

		void   tbtop( errblock&,
			      const string& ) ;

		void   tbvclear( errblock&,
				 fPOOL*,
				 const string& ) ;

		bool   writeableTable( errblock&,
				       const string&,
				       const string& ) ;

		string locate( errblock&,
			       const string&,
			       const string& ) ;

		bool status3( errblock&,
			      const string& ) ;

		string modname() { return "TABLEMGR" ; }

		int    taskid() { return 0 ; }

		friend class pApplication ;
		friend class pFTailor ;
} ;
