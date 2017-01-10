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
		void     define( int& RC,
				 const string& name ,
				 string * addr,
				 nameCHCK check=CHECK ) ;

		void     define( int& RC,
				 const string& name,
				 int * addr ) ;
	private:
		bool     ifexists( int& RC,
				   const string& name,
				   nameCHCK check=CHECK ) ;

		dataType getType( int& RC,
				  const string& name,
				  nameCHCK check=CHECK )  ;

		string * vlocate( int& RC,
				  int,
				  const string& name,
				  nameCHCK check=CHECK )  ;

		void     put( int& RC,
			      int maxRC,
			      const string& name,
			      const string& value,
			      nameCHCK check=CHECK ) ;

		void     put( int& RC,
			      int maxRC,
			      const string& name,
			      int value ) ;

		string   get( int& RC,
			      int maxRC,
			      const string& name,
			      nameCHCK check=CHECK ) ;

		int      get( int& RC,
			      int maxRC,
			      dataType dataType,
			      const string& name ) ;

		void     setmask( int& RC,
				  const string& name,
				  const string& mask ) ;

		void     dlete( int& RC,
				const string& name,
				nameCHCK check=CHECK ) ;

		void     reset() ;

		string   vilist( int& RC,
				 vdType defn ) ;

		string   vslist( int&RC,
				 vdType defn ) ;

		map<string, stack< fVAR>> POOL ;
	friend class pApplication  ;
	friend class Table ;
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
			refCount = 0     ;
			readOnly = false ;
			changed  = false ;
			sysPROF  = false ;
			path     = ""    ;
		}

	private:
		map<string, pVAR> POOL ;

		int    refCount ;
		bool   readOnly ;
		bool   changed  ;
		bool   sysPROF  ;
		string path     ;

		void   put( int& RC,
			    const string& name,
			    const string& value,
			    vTYPE =USER ) ;

		string get( int& RC,
			    const string& name ) ;

		string * vlocate( int& RC,
				  const string& name ) ;

		void   load( int& RC,
			     const string& applid,
			     const string& path ) ;

		void   save( int& RC,
			     const string& applid ) ;

		void   erase( int& RC,
			      const string& name ) ;

		bool   isSystem( int& RC,
				 const string& name ) ;

		void   setReadOnly()  { readOnly = true  ; }
		void   resetChanged() { changed  = false ; }
		void   createGenEntries() ;

	friend class poolMGR ;
} ;


class poolMGR
{
	public:
		poolMGR() ;
		void   createPool( int& RC,
				   poolType pType,
				   string path="" ) ;

		void   destroyPool( int& RC,
				    poolType pType ) ;

		void   destroyPool( int ls ) ;

		void   setAPPLID( int& RC,
				  const string& applid )   ;

		void   setShrdPool( int& RC,
				    const string& shrPool ) ;

		void   put( int& RC,
			    const string& name,
			    const string& value,
			    poolType = ASIS,
			    vTYPE =USER ) ;

		void   put( int& RC,
			    int ls,
			    const string& name,
			    const string& value ) ;

		string get( int& RC,
			    const string& name,
			    poolType=ASIS ) ;

		string get( int& RC,
			    int ls,
			    const string& name ) ;

		void   defaultVARs( int& RC,
				    const string& name,
				    const string& value,
				    poolType ) ;

		string getAPPLID()   { return currAPPLID ; }
		string getShrdPool() { return shrdPool   ; }

		void   setPOOLsReadOnly( int& RC ) ;
		void   snap() ;
		void   statistics() ;

		bool   ifexists( int& RC,
				 const string& name ) ;

	private:
		string vlist( int& RC,
			      poolType pType,
			      int lvl )  ;

		void   createPool( int ls ) ;

		string * vlocate( int& RC,
				  const string& name,
				  poolType=ASIS ) ;

		void   locateSubPool( int& RC,
				      map<string, pVPOOL>::iterator& vpool,
				      const string& name,
				      poolType=ASIS ) ;

		void   erase( int& RC,
			      const string& name,
			      poolType=ASIS ) ;

		string currAPPLID ;
		string shrdPool   ;
		int    shrdPooln  ;
		map<string, pVPOOL> POOLs_shared  ;
		map<string, pVPOOL> POOLs_profile ;
		map<int,    pVPOOL> POOLs_lscreen ;

	friend class pApplication ;
	friend class pPanel       ;
} ;
