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

class PPSP01A : public pApplication
{
	public:
		void application() ;

	private:
		void show_log( string fileName ) ;

		void read_file( string fileName ) ;
		bool file_has_changed( string fileName, int & fsize ) ;
		void fill_dynamic_area() ;
		void set_excludes()      ;
		void exclude_all()       ;
		void find_lines( string );

		int firstLine ;
		int maxLines  ;
		int startCol  ;

		vector<string> data     ;
		vector<bool>   excluded ;

		string ZCOL1   ;
		string ZROW1   ;
		string ZROW2   ;
		string ZAREA   ;
		string ZAREAT  ;
		int    ZAREAW  ;
		int    ZAREAD  ;
		string ZSHADOW ;
		int    maxCol  ;

		string filteri  ;
		string filterx  ;
		bool   Xon      ;
		bool   showDate ;
		bool   showTime ;
		bool   showMod  ;
		bool   showTask ;

		void pfkeySettings()  ;
		void colourSettings() ;
		int  setScreenAttrs( string, int, string &, string &, string & ) ;
		void setISPSVar( string, string ) ;
		void todoList()       ;
		void poolVariables()  ;
		void getpoolVariables( string ) ;
		void showPaths()         ;
		void showCommandTables() ;
		void showLoadedClasses() ;
		void showSavedFileList() ;
		void showTasks()         ;
		void updateTasks( string ) ;

		string VARLST, SEL, VAR, VPOOL, VPLVL, VAL, MESSAGE ;
		string MODLST, APPL, MOD, MODPATH;


};
