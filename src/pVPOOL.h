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
		int      fVAR_int        ;
		dataType fVAR_type       ;
		bool     fVAR_explicit   ;
	friend class fPOOL ;
} ;


class fPOOL
{
	public:
		void     define( int &, string, string *, nameCHCK check=CHECK ) ;
		void     define( int &, string, int *    ) ;
		bool     ifexists( int &, string, nameCHCK check=CHECK ) ;
		dataType getType( int &, string, nameCHCK check=CHECK ) ;
		void     put( int &, int, string, string, nameCHCK check=CHECK ) ;
		void     put( int &, int, string, int    ) ;
		string   get( int &, int, string, nameCHCK check=CHECK ) ;
		int      get( int &, int, dataType, string ) ;
		void     dlete( int &, string, nameCHCK check=CHECK ) ;
		void     dlete_gen( int &, string ) ;
		void     reset() ;

	private:
		map< string, stack< fVAR> > POOL ;
} ;


class pVAR
{
	private:
		string   pVAR_value      ;
		bool     pVAR_system     ;
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
			path     = ""    ;
		}

	private:
		map< string, pVAR > POOL ;

		int    refCount ;
		bool   readOnly ;
		bool   changed  ;
		string path     ;

		void   put( int &, string, string, vTYPE =USER ) ;
		string get( int &, string )      ;
		void   load( int &, string , string ) ;
		void   save( int &, string )     ;

		void   erase( int &, string )    ;
		bool   isSystem( int &, string ) ;
		void   setreadOnly()  { readOnly = true  ; }
		void   resetChanged() { changed  = false ; }
		
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
		string get( int &, string, poolType = ASIS ) ;
		void   locateSubPool( int &, map<string, pVPOOL>::iterator &, string, poolType = ASIS ) ;
		void   erase( int &, string, poolType = ASIS ) ;
		bool   ifexists( int &, string ) ;

	private:
		int RC            ;
		string currAPPLID ;
		string shrdPool   ;
		int    shrdPooln  ;
		map< string, pVPOOL > POOLs_shared      ;
		map< string, pVPOOL > POOLs_profile     ;
} ;
