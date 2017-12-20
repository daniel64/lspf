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
		void show_log( const string& fileName ) ;

		void read_file( const string& fileName ) ;
		bool file_has_changed( const string& fileName, int& fsize ) ;
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
		int    task    ;
		int    lprefix ;

		string VARLST  ;
		string SEL     ;
		string VAR     ;
		string VAL     ;
		string VPOOL   ;
		string VPLVL   ;
		string MESSAGE ;

		string MODLST  ;
		string APPL    ;
		string MOD     ;
		string MODPATH ;

		char   filteri  ;
		char   filterx  ;
		bool   Xon      ;
		bool   showDate ;
		bool   showTime ;
		bool   showMod  ;
		bool   showTask ;

		void dsList( string ) ;
		void lspfSettings()   ;
		void pfkeySettings()  ;
		void colourSettings() ;
		void globalColours()  ;
		int  setScreenAttrs( const string&, int, string, string, string ) ;
		void setISPSVar( const string&, string ) ;
		void todoList()       ;
		void poolVariables( const string& )    ;
		void getpoolVariables( const string& ) ;
		void runApplication( const string& )   ;
		void showPaths()         ;
		void showCommandTables() ;
		void showLoadedClasses() ;
		void showSavedFileList() ;
		void showTasks()         ;
		void updateTasks( const string&, const string& ="", const string& ="" ) ;
		void utilityPrograms()   ;
		void keylistTables()     ;
		void keylistTable( string="", string="", string="" ) ;
		void editKeylist( const string&, const string& ) ;
		void viewKeylist( const string&, const string& ) ;
		void createKeyTable( string )  ;

} ;
