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
		void     define( int &, string, string *, nameCHCK check=CHECK ) ;
		void     define( int &, string, int * ) ;
		bool     ifexists( int &, string, nameCHCK check=CHECK ) ;
		dataType getType( int &, string, nameCHCK check=CHECK )  ;
		string * vlocate( int &, int, string, nameCHCK check=CHECK )  ;
		void     put( int &, int, string, string, nameCHCK check=CHECK ) ;
		void     put( int &, int, string, int    ) ;
		string   get( int &, int, string, nameCHCK check=CHECK ) ;
		int      get( int &, int, dataType, string ) ;
		void     setmask( int &, string, string ) ;
		void     dlete( int &, string, nameCHCK check=CHECK ) ;
		void     reset() ;
		string   vilist( int &, vdType ) ;
		string   vslist( int &, vdType ) ;

	private:
		map< string, stack< fVAR> > POOL ;
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
		map< string, pVAR > POOL ;

		int    refCount ;
		bool   readOnly ;
		bool   changed  ;
		bool   sysPROF  ;
		string path     ;

		void   put( int &, string, string, vTYPE =USER ) ;
		string get( int &, string )      ;
		string * vlocate( int &, string ) ;
		void   load( int &, string , string ) ;
		void   save( int &, string )     ;

		void   erase( int &, string )    ;
		bool   isSystem( int &, string ) ;
		void   setreadOnly()  { readOnly = true  ; }
		void   resetChanged() { changed  = false ; }
		void   createGenEntries() ;

	friend class poolMGR ;
} ;


class poolMGR
{
	public:
		poolMGR() ;

		string getAPPLID()   { return currAPPLID ; }
		string getshrdPool() { return shrdPool   ; }
		void   setAPPLID( int & RC, string )     ;
		void   setshrdPool( int & RC, string )   ;
		string vlist( int & RC, poolType, int )  ;

		void   createPool( int & RC, poolType, string path="" ) ;
		void   destroyPool( int & RC, poolType ) ;
		void   setPOOLsreadOnly( int & RC ) ;
		void   defaultVARs( int & RC, string name, string value, poolType ) ;
		void   statistics() ;
		void   snap() ;

		void   put( int &, string, string, poolType = ASIS, vTYPE =USER ) ;
		string get( int &, string, poolType=ASIS ) ;
		string * vlocate( int &, string, poolType=ASIS ) ;
		void   locateSubPool( int &, map<string, pVPOOL>::iterator &, string, poolType = ASIS ) ;
		void   erase( int &, string, poolType = ASIS ) ;
		bool   ifexists( int &, string ) ;

	private:
		string currAPPLID ;
		string shrdPool   ;
		int    shrdPooln  ;
		map< string, pVPOOL > POOLs_shared      ;
		map< string, pVPOOL > POOLs_profile     ;
} ;
