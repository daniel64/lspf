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
	fVAR()
	{
		fVAR_string_ptr = &fVAR_string ;
		fVAR_int_ptr    = &fVAR_int    ;
	}
	private:
		string*  fVAR_string_ptr ;
		int*     fVAR_int_ptr    ;
		string   fVAR_string     ;
		string   fVAR_mask       ;
		int      fVAR_int        ;
		dataType fVAR_type       ;
		bool     fVAR_defined    ;
		bool     fVAR_valid      ;

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
		void define( errblock& err,
			     const string& name ,
			     string* addr ) ;

		void define( errblock& err,
			     const string& name,
			     int* addr ) ;
	private:
		map<string, stack<fVAR*>> POOL ;

		string nullstr ;
		set<string> varList ;

		bool ifexists( errblock& err,
			       const string& name ) ;

		dataType getType( errblock& err,
				  const string& name,
				  nameCHCK check=CHECK ) ;

		string* vlocate( errblock& err,
				 const string& name ) ;

		void put( errblock& err,
			  const string& name,
			  const string& value,
			  nameCHCK check=CHECK ) ;

		void put( errblock& err,
			  const string& name,
			  int value ) ;

		const string& get( errblock& err,
				   int maxRC,
				   const string& name,
				   nameCHCK check=CHECK ) ;

		int  get( errblock& err,
			  int maxRC,
			  dataType dataType,
			  const string& name ) ;

		void setmask( errblock& err,
			      const string& name,
			      const string& mask ) ;

		void dlete( errblock& err,
			    const string& name ) ;

		void reset( errblock& err ) ;

		set<string>& vilist( int& RC,
				     vdType defn ) ;

		set<string>& vslist( int&RC,
				     vdType defn ) ;

	friend class pApplication ;
	friend class tableMGR ;
	friend class Table    ;
	friend class pPanel   ;
	friend class abc      ;
} ;


enum pVType
{
	pV_VALUE,
	pV_ZTIME,
	pV_ZTIMEL,
	pV_ZDATE,
	pV_ZDATEL,
	pV_ZDAY,
	pV_ZDAYOFWK,
	pV_ZDATESTD,
	pV_ZMONTH,
	pV_ZJDATE,
	pV_ZJ4DATE,
	pV_ZYEAR,
	pV_ZTASKID,
	pV_ZSTDYEAR
} ;


class pVAR
{
	private:
		string pVAR_value  ;
		bool   pVAR_system ;
		pVType pVAR_type   ;

	friend class pVPOOL ;
} ;


class pVPOOL
{
	public:
		pVPOOL()
		{
			refCount = 0     ;
			readOnly = false ;
			changed  = false ;
			sysProf  = false ;
			path     = ""    ;
		}
		~pVPOOL() ;

	private:
		map<string, pVAR*> POOL ;

		int    refCount ;
		bool   readOnly ;
		bool   changed  ;
		bool   sysProf  ;
		string path     ;

		void   put( errblock& err,
			    const string& name,
			    const string& value,
			    vTYPE =USER ) ;

		void   put( errblock& err,
			    map<string, pVAR*>::iterator v_it,
			    const string& value,
			    vTYPE =USER ) ;

		string get( errblock& err,
			    map<string, pVAR*>::iterator v_it ) ;

		string* vlocate( errblock& err,
				 map<string, pVAR*>::iterator v_it ) ;

		void   load( errblock& err,
			     const string& applid,
			     const string& path ) ;

		void   save( errblock& err,
			     const string& applid ) ;

		void   erase( errblock& err,
			      map<string, pVAR*>::iterator v_it ) ;

		bool   isSystem( map<string, pVAR*>::iterator v_it ) ;

		void   setReadOnly()  { readOnly = true     ; }
		void   incRefCount()  { ++refCount          ; }
		void   decRefCount()  { --refCount          ; }
		void   sysProfile()   { sysProf = true      ; }
		bool   issysProfile() { return sysProf      ; }
		bool   inUse()        { return refCount > 0 ; }
		void   resetChanged() { changed = false     ; }
		void   createGenEntries() ;

	friend class poolMGR ;
} ;


class poolMGR
{
	public:
		poolMGR()  ;
		~poolMGR() ;

		static logger* lg ;

		void   connect( int taskid, const string&, int ) ;
		void   disconnect( int taskid ) ;
		void   setProfilePath( errblock& err,
				       const string& ) ;

		void   setPools( errblock& ) ;

		void   createProfilePool( errblock& err,
					  const string& appl ) ;

		int    createSharedPool() ;

		void   destroySystemPool( errblock& err ) ;

		void   destroyPool( int ls ) ;

		void   sysput( errblock& err,
			       const string& name,
			       const string& value,
			       poolType ) ;

		void   put( errblock& err,
			    const string& name,
			    const string& value,
			    poolType = ASIS,
			    vTYPE =USER ) ;

		void   put( errblock& err,
			    int ls,
			    const string& name,
			    const string& value ) ;

		string sysget( errblock& err,
			       const string& name,
			       poolType ) ;

		string get( errblock& err,
			    const string& name,
			    poolType=ASIS ) ;

		string get( errblock& err,
			    int ls,
			    const string& name ) ;

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

		void lock()    { mtx.lock()   ; }
		void unlock()  { mtx.unlock() ; }

		set<string>& vlist( errblock& err,
				    int& RC,
				    poolType pType,
				    int lvl ) ;

		map<int, pVPOOL*>::iterator createPool( int ls ) ;

		string* vlocate( errblock& err,
				 const string& name,
				 poolType=ASIS ) ;

		void   locateSubPool( errblock& err,
				      map<string, pVPOOL*>::iterator& pp_it,
				      map<string, pVPOOL*>::iterator& p_it,
				      map<string, pVAR*>::iterator& v_it,
				      const string& pool,
				      const string& name ) ;

		void   locateSubPool( errblock& err,
				      map<string, pVPOOL*>::iterator& sp_it,
				      map<string, pVPOOL*>::iterator& p_it,
				      map<string, pVAR*>::iterator& v_it,
				      int pool,
				      const string& name ) ;

		void   erase( errblock& err,
			      const string& name,
			      poolType=ASIS ) ;

	friend class pApplication ;
	friend class pPanel       ;
	friend class abc          ;
} ;

#undef llog
#undef debug1
#undef debug2


#define llog(t, s) \
{ \
lg->lock() ; \
(*lg) << microsec_clock::local_time() << \
" POOL      " << \
" " << d2ds( err.taskid, 5 ) << " " << t << " " << s ; \
lg->unlock() ; \
}

#ifdef DEBUG1
#define debug1( s ) \
{ \
lg->lock() ; \
(*lg) << microsec_clock::local_time() << \
" POOL      " << \
" " << d2ds( err.taskid, 5 ) << \
" D line: "  << __LINE__  << \
" >>L1 Function: " << __FUNCTION__ << \
" -  " << s ; \
lg->unlock() ; \
}
#else
#define debug1( s )
#endif


#ifdef DEBUG2
#define debug2( s ) \
{ \
lg->lock() ; \
(*lg) << microsec_clock::local_time() << \
" POOL      " << \
" " << d2ds( err.taskid, 5 ) << \
" D line: "  << __LINE__  << \
" >>L2 Function: " << __FUNCTION__ << \
" -  " << s ; \
lg->unlock() ; \
}
#else
#define debug2( s )
#endif
