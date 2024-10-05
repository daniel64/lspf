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

#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem ;

enum UT_PARMS
{
	UT_ALOG,
	UT_BROWSEE,
	UT_CMDS,
	UT_CTLKEYS,
	UT_CTU,
	UT_CUAATTR,
	UT_DSL,
	UT_DSX,
	UT_EDITEE,
	UT_FKA,
	UT_GCL,
	UT_GOPTS,
	UT_KEYL,
	UT_KEYS,
	UT_KLIST,
	UT_KLISTS,
	UT_LIBDEFS,
	UT_MODS,
	UT_MOUSE,
	UT_OUTLIST,
	UT_PATHS,
	UT_PFK,
	UT_PSYSER1,
	UT_PSYSER2,
	UT_PSYSER3,
	UT_RUN,
	UT_SAVELST,
	UT_SETVAR,
	UT_SLOG,
	UT_SPKEYS,
	UT_TBU,
	UT_TODO,
	UT_UTPGMS,
	UT_VARS,
	UT_ZEXPAND
} ;

class ut_format

{
	public:
	       int ut_min ;
	       int ut_max ;
} ;


map<string, UT_PARMS> utils = { { "AL",      UT_ALOG    },
				{ "BROWSEE", UT_BROWSEE },
				{ "CMDS",    UT_CMDS    },
				{ "CTLKEYS", UT_CTLKEYS },
				{ "CTU",     UT_CTU     },
				{ "CUAATTR", UT_CUAATTR },
				{ "DSL",     UT_DSL     },
				{ "DSX",     UT_DSX     },
				{ "EDITEE",  UT_EDITEE  },
				{ "FKA",     UT_FKA     },
				{ "GCL",     UT_GCL     },
				{ "GOPTS",   UT_GOPTS   },
				{ "KEYL",    UT_KEYL    },
				{ "KEYS",    UT_KEYS    },
				{ "KLIST",   UT_KLIST   },
				{ "KLISTS",  UT_KLISTS  },
				{ "LIBDEFS", UT_LIBDEFS },
				{ "MODS",    UT_MODS    },
				{ "MOUSE",   UT_MOUSE   },
				{ "OUTLIST", UT_OUTLIST },
				{ "PATHS",   UT_PATHS   },
				{ "PFK",     UT_PFK     },
				{ "PSYSER1", UT_PSYSER1 },
				{ "PSYSER2", UT_PSYSER2 },
				{ "PSYSER3", UT_PSYSER3 },
				{ "RUN",     UT_RUN     },
				{ "SAVELST", UT_SAVELST },
				{ "SETVAR",  UT_SETVAR  },
				{ "SL",      UT_SLOG    },
				{ "SPKEYS",  UT_SPKEYS  },
				{ "TBU",     UT_TBU     },
				{ "TODO",    UT_TODO    },
				{ "UTPGMS",  UT_UTPGMS  },
				{ "VARS",    UT_VARS    },
				{ "ZEXPAND", UT_ZEXPAND } } ;


map<UT_PARMS, ut_format> utFormat = { { UT_ALOG,     {  1,  1 } },
				      { UT_BROWSEE,  {  1, -1 } },
				      { UT_CMDS,     {  1,  1 } },
				      { UT_CUAATTR,  {  1,  1 } },
				      { UT_CTLKEYS,  {  1,  1 } },
				      { UT_CTU,      {  1,  1 } },
				      { UT_DSL,      {  1, -1 } },
				      { UT_DSX,      {  1, -1 } },
				      { UT_EDITEE,   {  1, -1 } },
				      { UT_FKA,      {  1,  2 } },
				      { UT_GCL,      {  1,  1 } },
				      { UT_GOPTS,    {  1,  1 } },
				      { UT_KEYL,     {  1,  1 } },
				      { UT_KEYS,     {  1,  1 } },
				      { UT_KLIST,    {  1,  2 } },
				      { UT_KLISTS,   {  1,  1 } },
				      { UT_LIBDEFS,  {  1,  2 } },
				      { UT_MODS,     {  1,  1 } },
				      { UT_MOUSE,    {  1,  1 } },
				      { UT_OUTLIST,  {  1,  1 } },
				      { UT_PATHS,    {  1,  1 } },
				      { UT_PFK,      {  1,  2 } },
				      { UT_PSYSER1,  {  1,  1 } },
				      { UT_PSYSER2,  {  2,  2 } },
				      { UT_PSYSER3,  {  2,  2 } },
				      { UT_RUN,      {  2,  2 } },
				      { UT_SAVELST,  {  1,  1 } },
				      { UT_SETVAR,   {  3,  3 } },
				      { UT_SLOG,     {  1,  1 } },
				      { UT_SPKEYS,   {  1,  1 } },
				      { UT_TBU,      {  1,  1 } },
				      { UT_TODO,     {  1,  1 } },
				      { UT_UTPGMS,   {  1,  1 } },
				      { UT_VARS,     {  1,  2 } },
				      { UT_ZEXPAND,  {  1, -1 } } } ;


class ppsp01a : public pApplication
{
	public:
		ppsp01a() ;
		void application() ;

	private:
		static boost::mutex mtx ;

		void show_log( const string& ) ;

		void read_file( const string& ) ;
		bool file_has_changed( const string&,
				       int& ) ;
		void fill_dynamic_area() ;
		void set_excludes()      ;
		void exclude_all()       ;
		void find_lines( string );

		string getColumnLine( int = 0 ) ;

		int firstLine ;
		int startCol  ;
		uint maxLines ;

		vector<string> data     ;
		vector<bool>   excluded ;

		string zcol1   ;
		string zrow1   ;
		string zrow2   ;
		string zarea   ;
		int    zareaw  ;
		int    zaread  ;
		int    zlvline ;
		uint   zasize  ;
		string zshadow ;
		uint   maxCol  ;
		int    task    ;
		int    lprefix ;

		string zscrolla ;
		int    zscrolln ;
		int    zcurinx  ;
		int    ztdtop   ;
		int    ztdvrows ;
		int    ztdsels  ;
		int    ztddepth ;

		string sel     ;
		string var     ;
		string val     ;
		string vpool   ;
		string vplvl   ;
		string message ;

		string zcmd    ;
		string zverb   ;
		string zapplid ;
		string zuser   ;
		string zscreen ;

		int    zscreend ;
		int    zscreenw ;

		string mod ;

		char filteri  ;
		char filterx  ;
		bool Xon      ;
		bool showDate ;
		bool showTime ;
		bool showMod  ;
		bool showTask ;

		bool colsOn ;

		void dsList( string ) ;
		void dxList( string ) ;
		void lspfSettings()   ;
		void pfkeySettings()  ;
		void keylistSettings() ;
		void cuaattrSettings();
		void globalColours()  ;
		void setRGBValues()   ;
		bool setScreenAttrs( const string&,
				     int ) ;
		void setISPSVar( const string&,
				 string ) ;
		void todoList()       ;
		void poolVariables( const string& )  ;
		void runApplication( const string& ) ;
		void getpoolVariables( const string&,
				       const string& ) ;
		void browseEntry( string& ) ;
		void editEntry( string& )   ;
		int  editRecovery( const string& ) ;

		string prompt_lmac() ;

		void showPaths()         ;
		void showCommandTables() ;
		void showLoadedClasses() ;
		void showSavedFileList() ;

		void mouseActions() ;
		void mouseActions_load( string** ) ;
		void specialKeys() ;

		void utilityPrograms() ;

		void keylistUtility() ;
		bool keylistUtility_display( string&,
					     const string&,
					     string&,
					     const string& ) ;
		void keylistUtility_load( const string&,
					  const string&,
					  const string&,
					  const string&,
					  const string&,
					  const string&,
					  const string&,
					  string&,
					  string&,
					  string&,
					  string&,
					  string& ) ;

		void keylistTables() ;
		void keylistTables_load( const string&,
					 const string&,
					 const string&,
					 const string&,
					 string&,
					 string&,
					 string&,
					 string&,
					 string&,
					 string& ) ;

		void keylistTable( string,
				   const string&,
				   string,
				   const string&,
				   string ) ;

		bool editKeylist( string&,
				  const string&,
				  const string&,
				  const string& = "" ) ;

		void viewKeylist( const string&,
				  const string&,
				  const string& = "" ) ;

		void createKeyTable( const string&,
				     const string& = "" ) ;

		void deleteKeylist( const string&,
				    const string&,
				    const string& ) ;

		void deleteKeyTable( const string&,
				     const string& ) ;

		void deleteEmptyKeyTable( const string&,
					 const string& ) ;

		void listDirectory( string& ) ;
		void controlKeys() ;

		void libdefStatus( const string& ) ;

		void showErrorScreen1() ;
		void showErrorScreen2( string ) ;
		void showErrorScreen3( string& ) ;

		void showHeldOutput() ;
		void showHeldOutput_build( const string&,
					   vector<path>&,
					   map<string, int>&,
					   set<string>& ) ;
		void showHeldOutput_display( const string&,
					     vector<path>& ) ;
		void showHeldOutput_purge( const string&,
					   vector<path>& ) ;

		void commandTableUtils() ;
		void commandTableDisplay( const string& ) ;
		bool commandTableEnqueue( const string&,
					  const string& ) ;
		void commandTableDequeue( const string&,
					  const string& ) ;
		void commandTableLoad( const string&,
				       const string&,
				       string&,
				       string&,
				       string&,
				       string&,
				       string&,
				       bool& ) ;

		void commandTableViewEntry() ;
		void commandTableEditEntry( const string&,
					    bool&,
					    string&,
					    string&,
					    string&,
					    string& ) ;

		void commandTableSave( const string&,
				       const string&,
				       const string&,
				       bool& ) ;

		void tableUtility() ;

		void tableUtility_list( string&,
					const string&,
					const string&,
					const string& ) ;

		void tableUtility_BrowseTable( string&,
					       const string&,
					       const string& ) ;

		void tableUtility_TableInfo( string&,
					     const string&,
					     const string& ) ;

		void tableUtility_TableStats( string&,
					      const string&,
					      const string& ) ;

		void pfshowUtility( string& ) ;

		void fkaUtility( string& ) ;

		void editString( const string& ) ;

		bool validTable( const string& ) ;

		void   update_reflist( const string& ) ;
		string get_dialogue_var( const string& ) ;
		void   set_dialogue_var( const string&,
					 const string& ) ;
		void   term_resize() ;

		string get_tempname() ;
		string get_filename( const string& ) ;
		string get_directory( const string& ) ;
		string get_columnLine( size_t ) ;

		void execute_cmd( int&,
				  const string&,
				  vector<string>& ) ;
} ;
