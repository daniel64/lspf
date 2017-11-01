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
		string * fVAR_string_ptr ;
		int    * fVAR_int_ptr    ;
		string   fVAR_string     ;
		string   fVAR_mask       ;
		int      fVAR_int        ;
		dataType fVAR_type       ;
		bool     fVAR_defined    ;
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
		void     define( errblock& err,
				 const string& name ,
				 string * addr,
				 nameCHCK check=CHECK ) ;

		void     define( errblock& err,
				 const string& name,
				 int * addr ) ;
	private:
		string nullstr ;
		string varList ;
		bool     ifexists( errblock& err,
				   const string& name,
				   nameCHCK check=CHECK ) ;

		dataType getType( errblock& err,
				  const string& name,
				  nameCHCK check=CHECK )  ;

		string * vlocate( errblock& err,
				  const string& name,
				  nameCHCK check=CHECK )  ;

		void     put( errblock& err,
			      const string& name,
			      const string& value,
			      nameCHCK check=CHECK ) ;

		void     put( errblock& err,
			      const string& name,
			      int value ) ;

		const string& get( errblock& err,
				   int maxRC,
				   const string& name,
				   nameCHCK check=CHECK ) ;

		int      get( errblock& err,
			      int maxRC,
			      dataType dataType,
			      const string& name ) ;

		void     setmask( errblock& err,
				  const string& name,
				  const string& mask ) ;

		void     dlete( errblock& err,
				const string& name,
				nameCHCK check=CHECK ) ;

		void     reset( errblock& err ) ;

		const string& vilist( int& RC,
				      vdType defn ) ;

		const string& vslist( int&RC,
				      vdType defn ) ;

		map<string, stack<fVAR*>> POOL ;

	friend class pApplication ;
	friend class Table  ;
	friend class pPanel ;
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
			refCount = 1     ;
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

		string * vlocate( errblock& err,
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
		void   incRefCount()  { refCount++          ; }
		void   decRefCount()  { refCount--          ; }
		void   sysProfile()   { sysProf = true      ; }
		bool   issysProfile() { return sysProf      ; }
		bool   inUse()        { return refCount > 0 ; }
		void   resetChanged() { changed  = false    ; }
		void   createGenEntries() ;

	friend class poolMGR ;
} ;


class poolMGR
{
	public:
		poolMGR()  ;
		~poolMGR() ;

		void   createPool( errblock& err,
				   poolType pType,
				   string path="" ) ;

		void   destroyPool( errblock& err,
				    poolType pType ) ;

		void   destroyPool( int ls ) ;

		void   setApplid( errblock& err,
				  const string& applid )   ;

		void   setShrdPool( errblock& err,
				    const string& shrPool ) ;

		void   put( errblock& err,
			    const string& name,
			    const string& value,
			    poolType = ASIS,
			    vTYPE =USER ) ;

		void   put( errblock& err,
			    int ls,
			    const string& name,
			    const string& value ) ;

		string get( errblock& err,
			    const string& name,
			    poolType=ASIS ) ;

		string get( errblock& err,
			    int ls,
			    const string& name ) ;

		void   defaultVARs( errblock& err,
				    const string& name,
				    const string& value,
				    poolType ) ;

		string getApplid()   { return currApplid ; }
		string getShrdPool() { return shrdPool   ; }

		void   setPOOLsReadOnly() ;
		void   snap() ;
		void   statistics() ;

	private:
		const string& vlist( int& RC,
				     poolType pType,
				     int lvl )  ;

		void   createPool( int ls ) ;

		string * vlocate( errblock& err,
				  const string& name,
				  poolType=ASIS ) ;

		void   locateSubPool( errblock& err,
				      map<string, pVPOOL*>::iterator& p_it,
				      map<string, pVAR*>::iterator& v_it,
				      const string& name,
				      poolType=ASIS ) ;

		void   erase( errblock& err,
			      const string& name,
			      poolType=ASIS ) ;

		string currApplid ;
		string shrdPool   ;
		string varList    ;
		int    shrdPooln  ;

		map<string, pVPOOL*> POOLs_shared  ;
		map<string, pVPOOL*> POOLs_profile ;
		map<int,    pVPOOL*> POOLs_lscreen ;

	friend class pApplication ;
	friend class pPanel       ;
} ;
