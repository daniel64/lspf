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

class fVAR
{
	public:
		fVAR()
		{
			fVAR_int        = 0 ;
			fVAR_string_ptr = &fVAR_string ;
			fVAR_int_ptr    = &fVAR_int ;
			fVAR_vtype      = VED_NONE ;
			nullstr         = "" ;
			fVAR_zconv      = 0 ;
		}

		fVAR( string* addr,
		      int zconv ) : fVAR()
		{
			fVAR_string_ptr = addr ;
			fVAR_type       = STRING ;
			fVAR_defined    = true ;
			fVAR_zconv      = zconv ;
		}

		fVAR( int* addr,
		      int zconv ) : fVAR()
		{
			fVAR_int_ptr = addr ;
			fVAR_type    = INTEGER ;
			fVAR_defined = true ;
			fVAR_zconv   = zconv ;
		}

		fVAR( const string& s,
		      int zconv ) : fVAR()
		{
			fVAR_string  = s ;
			fVAR_type    = STRING ;
			fVAR_defined = false ;
			fVAR_zconv   = zconv ;
		}

		fVAR( int i,
		      int zconv ) : fVAR()
		{
			fVAR_int     = i ;
			fVAR_type    = INTEGER ;
			fVAR_defined = false ;
			fVAR_zconv   = zconv ;
		}

		string& sget( const string& ) ;

		int iget( errblock&,
			  const string& ) ;

		void put( errblock&,
			  const string&,
			  const string& ) ;

		void put( int,
			  int = 0 ) ;

		bool integer() const ;

		bool hasmask() const ;

		bool hasmask( string& m,
			      VEDIT_TYPE& t ) const ;

	private:
		string*  fVAR_string_ptr ;
		int*     fVAR_int_ptr ;
		string   fVAR_string ;
		int      fVAR_int  ;
		string   fVAR_mask ;
		dataType fVAR_type ;
		bool     fVAR_defined ;
		VEDIT_TYPE fVAR_vtype ;
		int      fVAR_zconv ;

		string nullstr ;

	friend class fPOOL ;
} ;


class fPOOL
{
	public:

		fPOOL()
		{
			nullstr = "" ;
		}

		~fPOOL() ;

		void define( errblock&,
			     const string&,
			     string*,
			     bool check = true ) ;

		void define( errblock&,
			     const char*,
			     string* ) ;

		void define( errblock&,
			     const string&,
			     int* addr,
			     bool check = true ) ;

		void define( errblock&,
			     const char*,
			     int* addr ) ;

	private:
		map<string, stack<fVAR*>> pool_1 ;
		map<string, string*> pool_2 ;

		string nullstr ;
		set<string> varList ;

		bool ifexists( errblock&,
			       const string& ) ;

		fVAR* getfVAR( errblock&,
			       const string& ) ;

		string* vlocate( errblock&,
				 const string& ) ;

		void put1( errblock&,
			   const string&,
			   const string&,
			   bool check = true ) ;

		void put1( errblock&,
			   const string&,
			   const char* ) ;

		void put1( errblock&,
			   const string&,
			   int,
			   bool check = true ) ;

		void put1( errblock&,
			   const char*,
			   int ) ;

		void put2( errblock&,
			   const string&,
			   const string& ) ;

		void put2( errblock&,
			   const string&,
			   int value ) ;

		void put3( const string&,
			   const string& ) ;

		const string& get1( errblock&,
				    int,
				    const string&,
				    bool check = true ) ;

		const string& get1( errblock&,
				    int,
				    const char* ) ;

		int  get1( errblock&,
			   int,
			   dataType,
			   const string&,
			   bool check = true ) ;

		int  get1( errblock&,
			   int,
			   dataType,
			   const char* ) ;

		const string& get2( errblock&,
				    int,
				    const string& ) ;

		int get2( errblock&,
			  int,
			  dataType,
			  const string& ) ;

		const string& get3( errblock&,
				    const string& ) ;

		void setmask( errblock&,
			      const string&,
			      const string&,
			      VEDIT_TYPE ) ;

		void getmask( errblock&,
			      const string&,
			      string&,
			      VEDIT_TYPE& ) ;

		bool hasmask( const string& ) ;

		bool hasmask( const string&,
			      string&,
			      VEDIT_TYPE& ) ;

		void del( errblock&,
			  const string& ) ;

		void reset( errblock& ) ;

		set<string>& vlist( int&,
				    dataType,
				    vdType ) ;

		string modname() { return "FPOOL" ; }

		map<string, int> zint2str = { { "ZTDTOP",   6 },
					      { "ZTDDEPTH", 6 },
					      { "ZTDROWS",  6 },
					      { "ZTDVROWS", 6 },
					      { "ZTDSELS",  4 },
					      { "ZCURPOS",  4 },
					      { "ZSBTASK",  8 },
					      { "ZCURINX",  8 } } ;

#ifdef HAS_REXX_SUPPORT
	friend int REXXENTRY rxterExit_panel1( RexxExitContext*,
					       int,
					       int,
					       PEXIT ) ;

	friend int REXXENTRY rxiniExit_panel1( RexxExitContext*,
					       int,
					       int,
					       PEXIT ) ;
	friend int REXXENTRY rxterExit_ft( RexxExitContext*,
					   int,
					   int,
					   PEXIT ) ;

	friend int REXXENTRY rxiniExit_ft( RexxExitContext*,
					   int,
					   int,
					   PEXIT ) ;
#endif

	friend class pApplication ;
	friend class tableMGR ;
	friend class lss      ;
	friend class Table    ;
	friend class pPanel   ;
	friend class pFTailor ;
	friend class abc      ;
} ;


enum pVType
{
	pV_VALUE,
	pV_ZTIME,
	pV_ZTIMEL,
	pV_ZDATE,
	pV_ZDAY,
	pV_ZDAYOFWK,
	pV_ZDATESTD,
	pV_ZMONTH,
	pV_ZJDATE,
	pV_ZJ4DATE,
	pV_ZYEAR,
	pV_ZSTDYEAR,
	pV_ZDEBUG,
	pV_ZTASKID
} ;


class pVAR
{
		pVAR()
		{
			pVAR_value  = "" ;
			pVAR_system = true ;
			pVAR_type   = pV_VALUE ;
		}

		pVAR( const pVAR& v )
		{
			pVAR_value  = v.pVAR_value ;
			pVAR_system = v.pVAR_system ;
			pVAR_type   = v.pVAR_type ;
		}

		pVAR( pVType t ) : pVAR()
		{
			pVAR_type = t ;
		}

	private:
		string pVAR_value  ;
		bool   pVAR_system ;
		pVType pVAR_type   ;

	friend class pVPOOL ;
} ;


class pVPOOL
{
	public:
		static uint pfkgToken ;

		pVPOOL()
		{
			refCount = 0     ;
			readOnly = false ;
			changed  = false ;
			profile  = false ;
			sysProf  = false ;
			path     = ""    ;
		}

		~pVPOOL() ;

	private:
		map<string, pVAR*> POOL ;

		int    refCount ;
		bool   readOnly ;
		bool   changed ;
		bool   profile ;
		bool   sysProf ;
		string path ;

		int    taskid() { return 0 ; }

		void   put( errblock&,
			    const string&,
			    const string&,
			    vTYPE = USER ) ;

		void   put( errblock&,
			    map<string, pVAR*>::iterator,
			    const string&,
			    vTYPE = USER ) ;

		string get( errblock&,
			    map<string, pVAR*>::iterator ) ;

		string* vlocate( errblock&,
				 map<string, pVAR*>::iterator ) ;

		void   load( errblock&,
			     const string&,
			     const string& ) ;

		void   save( errblock&,
			     const string& ) ;

		void   erase( errblock&,
			      map<string, pVAR*>::iterator ) ;

		bool   isSystem( map<string, pVAR*>::iterator ) ;

		void   setReadOnly()  { readOnly = true     ; }
		void   incRefCount()  { ++refCount          ; }
		void   decRefCount()  { --refCount          ; }
		void   setProfile()   { profile = true      ; }
		void   sysProfile()   { sysProf = true      ; }
		bool   issysProfile() { return sysProf      ; }
		bool   inUse()        { return refCount > 0 ; }
		void   resetChanged() { changed = false     ; }
		void   createGenEntries() ;

		string modname() { return "VPOOL" ; }

	friend class poolMGR ;
} ;


class poolMGR
{
	public:
		poolMGR()  ;
		~poolMGR() ;

		static logger* lg ;

		void   connect( int,
				const string&,
				int ) ;
		void   disconnect( int ) ;
		void   setProfilePath( errblock&,
				       const string& ) ;

		void   setPools( errblock& ) ;

		void   createProfilePool( errblock&,
					  const string&,
					  bool = false ) ;

		int    createSharedPool() ;

		void   destroySystemPool( errblock& ) ;

		void   destroyPool( int ) ;

		void   sysput( errblock&,
			       const string&,
			       const string&,
			       poolType ) ;

		void   put( errblock&,
			    const string&,
			    const string&,
			    poolType = ASIS,
			    vTYPE = USER ) ;

		void   put( errblock&,
			    int ls,
			    const string&,
			    const string& ) ;

		string sysget( errblock&,
			       const string&,
			       poolType ) ;

		string get( errblock&,
			    const string&,
			    poolType = ASIS ) ;

		string get( errblock&,
			    int ls,
			    const string& ) ;

		void   setPOOLsReadOnly() ;
		void   snap() ;
		void   statistics() ;

	private:
		int    shrdPool ;
		int    _shared  ;
		string _applid  ;
		string ppath    ;

		set<string> varList ;
		map<int, pair<string,int>> task_table ;

		map<string, pVPOOL*> POOLs_shared  ;
		map<string, pVPOOL*> POOLs_profile ;
		map<int,    pVPOOL*> POOLs_lscreen ;

		boost::mutex mtx ;

		int    taskid() { return 0 ; }

		void lock()    { mtx.lock()   ; }
		void unlock()  { mtx.unlock() ; }

		set<string>& vlist( errblock&,
				    int&,
				    poolType,
				    int ) ;

		map<int, pVPOOL*>::iterator createPool( int ) ;

		string* vlocate( errblock&,
				 const string&,
				 poolType = ASIS ) ;

		void   locateSubPool( errblock&,
				      map<string, pVPOOL*>::iterator&,
				      map<string, pVPOOL*>::iterator&,
				      map<string, pVAR*>::iterator&,
				      const string&,
				      const string& ) ;

		void   locateSubPool( errblock&,
				      map<string, pVPOOL*>::iterator&,
				      map<string, pVPOOL*>::iterator&,
				      map<string, pVAR*>::iterator&,
				      int,
				      const string& ) ;

		void   erase( errblock&,
			      const string&,
			      poolType = ASIS ) ;

		string modname() { return "POOLMGR" ; }

	friend class pApplication ;
	friend class pPanel ;
	friend class pFTailor ;
	friend class abc ;
} ;
