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
		void application() ;

	private:
		void setup()       ;
		void OpenTableRO() ;
		void OpenTableUP() ;
		void CloseTable()  ;

		void OpenActiveFList( string ) ;
		void AddReflistEntry( string ) ;
		void PersonalFList() ;
		void OpenFileList( string )  ;
		void EditFileList( string )  ;
		void RetrieveEntry( string ) ;

		void userSettings() ;

		string ZCURTB   ;
		string FLADESCP ;
		string FLAPET01 ;
		string FLAPET02 ;
		string FLAPET03 ;
		string FLAPET04 ;
		string FLAPET05 ;
		string FLAPET06 ;
		string FLAPET07 ;
		string FLAPET08 ;
		string FLAPET09 ;
		string FLAPET10 ;
		string FLAPET11 ;
		string FLAPET12 ;
		string FLAPET13 ;
		string FLAPET14 ;
		string FLAPET15 ;
		string FLAPET16 ;
		string FLAPET17 ;
		string FLAPET18 ;
		string FLAPET19 ;
		string FLAPET20 ;
		string FLAPET21 ;
		string FLAPET22 ;
		string FLAPET23 ;
		string FLAPET24 ;
		string FLAPET25 ;
		string FLAPET26 ;
		string FLAPET27 ;
		string FLAPET28 ;
		string FLAPET29 ;
		string FLAPET30 ;
		string FLACTIME ;
		string FLAUTIME ;

		string UPROF    ;
		string TABFLDS  ;




} ;
