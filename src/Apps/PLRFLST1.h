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

class PLRFLST1 : public pApplication
{
	public:
		PLRFLST1()         ;
		void application() ;

	private:
		void OpenTableRO() ;
		void OpenTableUP() ;
		void CloseTable()  ;

		void   OpenActiveFList( const string& ) ;
		void   AddReflistEntry( string& )     ;
		void   AddFilelistEntry( const string& ) ;
		void   PersonalFList( const string& ) ;
		void   OpenFileList( const string& )  ;
		string StoreFileList( const string& ) ;
		void   EditFileList( const string& )  ;
		void   RetrieveMatchEntry( string )   ;
		void   RetrieveEntry( string ) ;
		void   createDefaultTable() ;

		void userSettings() ;

		string zuser    ;
		string zcmd     ;
		string zscreen  ;
		string uprof    ;

		int    zcurinx  ;
		int    ztdtop   ;
		int    ztdsels  ;

		string zcurtb   ;
		string fladescp ;
		string flapet01 ;
		string flapet02 ;
		string flapet03 ;
		string flapet04 ;
		string flapet05 ;
		string flapet06 ;
		string flapet07 ;
		string flapet08 ;
		string flapet09 ;
		string flapet10 ;
		string flapet11 ;
		string flapet12 ;
		string flapet13 ;
		string flapet14 ;
		string flapet15 ;
		string flapet16 ;
		string flapet17 ;
		string flapet18 ;
		string flapet19 ;
		string flapet20 ;
		string flapet21 ;
		string flapet22 ;
		string flapet23 ;
		string flapet24 ;
		string flapet25 ;
		string flapet26 ;
		string flapet27 ;
		string flapet28 ;
		string flapet29 ;
		string flapet30 ;
		string flactime ;
		string flautime ;

		string table    ;
		string tabflds  ;
} ;
