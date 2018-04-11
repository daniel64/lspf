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

using namespace std;


using namespace boost::filesystem ;

class PFLST0A : public pApplication
{
	public:
		PFLST0A()          ;
		void application() ;

	private:
		void createFileList1( string ="" ) ;
		void createFileList2( const string &, string ="" ) ;
		void createSearchList( const string & ) ;

		string showListing() ;
		void   showInfo( const string& ) ;
		int    processPrimCMD()   ;
		void   copyDirs( const string&, const string&, const string&, bool & ) ;
		void   modifyAttrs( const string& )  ;
		string expandDir( const string& )  ;
		string expandFld1( const string& ) ;
		void   browseTree( const string& ) ;
		string getAppName( string ) ;
		string expandName( const string& ) ;
		void   AddPath( const string&, const string&, vector<path>& ) ;
		string createEntry( const string&, const string& ) ;

		vector<string> SearchList ;
		bool   UseSearch ;
		bool   UseList   ;

		int    crp ;
		string msg ;
		string rsn ;
		string PssList ;

		string zcmd    ;
		string zverb   ;
		string zhome   ;
		string zscreen ;
		string zuser   ;
		string zpath   ;
		string zdir    ;

		string zcurfld  ;
		int    zcurinx  ;
		int    ztdtop   ;
		int    ztdvrows ;
		int    ztdsels  ;
		int    ztddepth ;

		string dslist ;
		string exgen  ;
		string afhidden ;

		string sel      ;
		string ENTRY    ;
		string message  ;
		string TYPE     ;
		string permiss  ;
		string size     ;
		string stcdate  ;
		string moddate  ;
		string moddates ;
		string ientry   ;
		string itype    ;
		string iowner   ;
		string igroup   ;
		string iinode   ;
		string inlnks   ;
		string irlnk    ;
		string ipermiss ;
		string isize    ;
		string istcdate ;
		string imoddate ;
		string iaccdate ;
		string imaj     ;
		string imin     ;
		string iblksize ;
		string isetuid  ;
		string isetgid  ;
		string isticky  ;
		string iownern  ;
		string igroupn  ;
} ;
