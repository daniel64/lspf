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

enum RR_PARMS
{
	RR_PL1,
	RR_PL2,
	RR_PL3,
	RR_PL4,
	RR_PLA,
	RR_PLR,
	RR_PLF,
	RR_NR1,
	RR_MTC,
	RR_US1
} ;


class plrflst1 : public pApplication
{
	public:
		plrflst1() ;
		void application() ;

	private:
		void OpenTableRO() ;
		void OpenTableUP() ;
		void CloseTable()  ;

		void   OpenActiveRefList( const string&,
					  const string& ) ;
		void   AddReferralEntry( string ) ;
		void   AddReflistEntry( string& )  ;
		void   AddFilelistEntry( const string& ) ;
		void   PersonalFileList( const string& = "" ) ;
		void   OpenFileList( const string& )  ;
		void   EditFileList( const string& )  ;
		void   EditNewFileList( const string& = "" ) ;
		void   RetrieveMatchEntry( const string&,
					   const string&,
					   uint = 1 ) ;
		void   RetrieveEntry( string ) ;
		void   createDefaultTable() ;
		void   createReflistEntry() ;
		void   saveListEntry( const string&,
				      const string&,
				      const string ) ;

		void   loadTBTable( const string& ) ;
		void   loadTBTable_all( const string& ) ;

		void   add_files( const string&,
				  const string&,
				  vector<string>&,
				  vector<size_t>&,
				  set<string>& ) ;

		void userSettings() ;

		string get_shared_var( const string& ) ;
		string get_profile_var( const string& ) ;

		string zuser    ;
		string zcmd     ;
		string zscreen  ;

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

		map<string, RR_PARMS> pgm_parms = { { "PL1", RR_PL1 },
						    { "PL2", RR_PL2 },
						    { "PL3", RR_PL3 },
						    { "PL4", RR_PL4 },
						    { "PLA", RR_PLA },
						    { "PLR", RR_PLR },
						    { "PLF", RR_PLF },
						    { "NR1", RR_NR1 },
						    { "MTC", RR_MTC },
						    { "US1", RR_US1 } } ;
} ;
