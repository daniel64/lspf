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

using namespace std ;
using namespace boost::filesystem ;

enum LN_CMDS
{
	LN_ADD,
	LN_BROWSE,
	LN_CCOPY,
	LN_COPY,
	LN_DELETE,
	LN_EDIT,
	LN_EXCLUDE,
	LN_EXECUTE1,
	LN_EXECUTE2,
	LN_FS,
	LN_INFO,
	LN_LINK,
	LN_LIST,
	LN_LISTX,
	LN_MODIFY,
	LN_NANO,
	LN_RENAME,
	LN_SUBMIT,
	LN_TREE,
	LN_TTREE,
	LN_UNZIP,
	LN_VI,
	LN_VIEW,
	LN_ZIP
} ;


using namespace boost::filesystem ;

class pflst0a : public pApplication
{
	public:
		pflst0a() ;
		void application() ;

	private:
		void cleanup_custom() ;
		int  setup_parameters() ;
		void setup_hotbar() ;
		void process_hotbar() ;
		void load_path_vector( vector<path>& ) ;
		void load_searchList( vector<string>& ) ;
		void create_filelist1( bool = false ) ;
		void updateFileList1() ;
		void create_filelist2() ;
		void createSearchList() ;

		void set_scrname( const string& ) ;
		void restore_scrname() ;

		string get_profile_var( const string&,
					const string& = "" ) ;

		void   put_profile_var( const string&,
					const string& ) ;

		string showListing() ;
		void   show_info( const string&,
				  bool& ) ;
		void   show_FileSystem( const string&,
					bool& ) ;
		int    actionPrimaryCommand1() ;
		int    actionPrimaryCommand2() ;
		void   actionLineCommand( const string&,
					  int&,
					  bool& ) ;
		void   copy_directory( const string&,
				       const string&,
				       bool,
				       bool,
				       bool,
				       bool&,
				       bool& ) ;
		int    edit_entry( const string&,
				   bool& ) ;
		int    copy_entry( const string&,
				   bool&,
				   bool = false ) ;
		void   view_entry( const string&,
				   bool& ) ;
		void   list_entry( const string&,
				   bool& ) ;
		void   list_entry_recursive( const string&,
					     bool& ) ;
		void   modify_entry( const string&,
				     bool& ) ;
		void   delete_entry( const string&,
				     bool& ) ;
		void   browse_entry( const string&,
				     bool& ) ;
		void   rename_entry( const string&,
				     bool& ) ;

		string expand_dir1( const string& ) ;
		string expand_dir2( const string& ) ;
		string expand_field1( const string& ) ;
		void   browseTree( const string& ) ;
		string expand_name( const string& ) ;
		string getAppName( string ) ;
		string getPFLName() ;
		void   add_paths( const string&,
				  const string&,
				  vector<path>& ) ;

		int    editRecovery( const string& ) ;
		bool   isRexx( string ) ;
		string get_tempname() ;
		bool   path_change( const string&,
				    const string& ) ;
		void   getFileAttributes()  ;
		void   getFilePermissions() ;
		void   getFileUIDGID() ;
		void   copy_file_attributes( const string&,
					     const string& ) ;

		void   set_filter_i( const string&,
				     bool = false ) ;
		bool   match_filter_i( const string& ) ;
		void   clear_filter_i() ;

		void   set_filter_x( const string&,
				     bool = false ) ;
		bool   match_filter_x( const string& ) ;
		void   clear_filter_x() ;

		void   set_filter( const string&,
				   vector<string>&,
				   map<string, boost::regex>&,
				   bool ) ;

		void   execute_cmd( int&,
				    const string&,
				    vector<string>& ) ;

		void   set_search( const string& ) ;
		void   clear_search() ;

		void   set_fmsgs() ;
		void   confirm_cmd( string&,
				    bool&,
				    bool& ) ;

		void   confirm_shell( string&,
				      bool&,
				      bool& ) ;

		string full_name( const string& ,
				  const string& ) ;

		string full_dir( const string& ) ;

		string file_name( const string& ) ;
		string file_directory( const string& ) ;

		void listDirectory( string& ) ;

		void log_filesystem_error( const string& ) ;
		void log_filesystem_error( const filesystem_error& ) ;
		void log_filesystem_error( const boost::system::error_code&,
					     const string& ) ;

		void update_reflist( const string& ) ;

		string get_dialogue_var( const string& ) ;

		void   set_dialogue_var( const string&,
					 const string& ) ;

		const string& get_truename( const string& ) ;

		string get_target( const string& ) ;

		string add_path( const string&,
				 const string& ) ;

		bool is_subpath( const path& p1,
				 const path& p2 ) ;
		bool is_subpath( const path& p1,
				 const path& p2,
				 int& ) ;

		string qstring( int&,
				const string& ) ;
		string qstring1( int&,
				 const string& ) ;

		int    action_Back() ;
		int    action_Browse() ;
		int    action_Chngdir() ;
		int    action_Addpfl() ;
		int    action_Edit() ;
		int    action_Exclude() ;
		int    action_Expand() ;
		int    action_Find() ;
		int    action_Flip() ;
		int    action_Locate() ;
		int    action_Logerrs() ;
		int    action_Makedir() ;
		int    action_Hotbar()  ;
		int    action_Setupbar() ;
		int    action_Include() ;
		int    action_Only() ;
		int    action_Print() ;
		int    action_Refresh() ;
		int    action_Reset() ;
		int    action_Search() ;
		int    action_Setpath() ;
		int    action_Sort() ;
		int    action_Stats() ;
		int    action_Touch() ;
		int    action_Settings() ;
		int    action_View() ;
		int    action_Colours() ;
		int    action_Block()   ;
		int    action_Showcmd() ;
		int    action_Previous() ;
		int    action_History() ;

		int    action_Makedir1() ;
		int    action_Include1() ;
		int    action_Refresh1() ;
		int    action_Reset1() ;
		int    action_Touch1() ;
		int    action_Exclude1() ;
		int    action_Flip1() ;
		int    action_Zip() ;

		int    action_Block_Add( const string& ) ;
		int    action_Block_Browse( const string& ) ;
		int    action_Block_Copy( const string& ) ;
		int    action_Block_CCopy( const string& ) ;
		int    action_Block_Delete( const string& ) ;
		int    action_Block_Edit( const string& ) ;
		int    action_Block_Fsys( const string& ) ;
		int    action_Block_Info( const string& ) ;
		int    action_Block_List( const string& ) ;
		int    action_Block_Modify( const string& ) ;
		int    action_Block_Rename( const string& ) ;
		int    action_Block_View( const string& ) ;

		set<string> searchList ;
		set<string> excludeList ;
		boost::circular_buffer<string> pnames ;

		bool   useList ;
		string log_error ;

		int    crp ;
		int    top ;
		int    ntop ;
		uint   retPos ;

		string msg ;
		string rsn ;
		string pssList ;

		string zcmd    ;
		string zverb   ;
		string zhome   ;
		string zscreen ;
		string zuser   ;
		string zpath   ;
		string zdir    ;
		string zuprof  ;
		string sort_parm ;

		string zscrname ;
		string zcurfld  ;
		int    zcurpos  ;
		int    zcurinx  ;
		int    ztdtop   ;
		int    ztdvrows ;
		int    ztdsels  ;
		int    ztddepth ;
		int    zhotbarw ;

		bool   include  ;
		bool   recursv  ;
		bool   rebuild1 ;
		bool   rebuild2 ;
		bool   initsort ;
		bool   affull   ;
		bool   pvalid   ;
		bool   stats    ;

		string dslist   ;
		string exgen    ;
		string afhidden ;
		string afsfull  ;
		string fldirs   ;
		string filepre  ;
		string timeout  ;

		string sel      ;
		string entry    ;
		string message  ;
		string type     ;
		string permiss  ;
		string size     ;
		string moddate  ;
		string moddates ;
		string accdate  ;
		string accdates ;
		string stcdate  ;
		string stcdates ;
		string owner    ;
		string group    ;
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

		string fmsg1    ;
		string fmsg2    ;
		string fmsg3    ;
		string fmsg4    ;

		string zhotbard ;
		string zhotbars ;

		string pflscmd  ;
		string pflhbar  ;
		string pflscrl  ;
		string pflevep  ;
		string pfluref  ;
		string pflsdef  ;

		string ccpath   ;

		string findstr  ;

		map<string, string> hotbar ;

		struct stat results ;

		vector<string> filter_i ;
		map<string, boost::regex> filter_i_regex ;

		vector<string> filter_x ;
		map<string, boost::regex> filter_x_regex ;

		vector<string> search ;

		stack<string> scrnames ;

		map<string, int(pflst0a::*)()>commandList1 = { { "*",        &pflst0a::action_Block     },
							       { "ADDPFL",   &pflst0a::action_Addpfl    },
							       { "BACK",     &pflst0a::action_Back      },
							       { "BROWSE",   &pflst0a::action_Browse    },
							       { "CD",       &pflst0a::action_Chngdir   },
							       { "COLOURS",  &pflst0a::action_Colours   },
							       { "EDIT",     &pflst0a::action_Edit      },
							       { "EXPAND",   &pflst0a::action_Expand    },
							       { "FIND",     &pflst0a::action_Find      },
							       { "FLIP",     &pflst0a::action_Flip      },
							       { "HOTBAR",   &pflst0a::action_Hotbar    },
							       { "HOTBAR?",  &pflst0a::action_Setupbar  },
							       { "I",        &pflst0a::action_Include   },
							       { "LOCATE",   &pflst0a::action_Locate    },
							       { "LOGERRS",  &pflst0a::action_Logerrs   },
							       { "MKDIR",    &pflst0a::action_Makedir   },
							       { "O",        &pflst0a::action_Only      },
							       { "P?",       &pflst0a::action_History   },
							       { "PREVIOUS", &pflst0a::action_Previous  },
							       { "PRINT",    &pflst0a::action_Print     },
							       { "REFRESH",  &pflst0a::action_Refresh   },
							       { "RESET",    &pflst0a::action_Reset     },
							       { "SEARCH",   &pflst0a::action_Search    },
							       { "SETPATH",  &pflst0a::action_Setpath   },
							       { "SETTINGS", &pflst0a::action_Settings  },
							       { "SHOWCMD",  &pflst0a::action_Showcmd   },
							       { "SORT",     &pflst0a::action_Sort      },
							       { "STATS",    &pflst0a::action_Stats     },
							       { "TOUCH",    &pflst0a::action_Touch     },
							       { "VIEW",     &pflst0a::action_View      },
							       { "X",        &pflst0a::action_Exclude   },
							       { "ZIP",      &pflst0a::action_Zip       } } ;

		map<string, int(pflst0a::*)()>commandList2 = { { "BACK",    &pflst0a::action_Back     },
							       { "BROWSE",  &pflst0a::action_Browse   },
							       { "CD",      &pflst0a::action_Chngdir  },
							       { "EDIT",    &pflst0a::action_Edit     },
							       { "X",       &pflst0a::action_Exclude1 },
							       { "FLIP",    &pflst0a::action_Flip1    },
							       { "I",       &pflst0a::action_Include1 },
							       { "LOCATE",  &pflst0a::action_Locate   },
							       { "MKDIR",   &pflst0a::action_Makedir1 },
							       { "O",       &pflst0a::action_Only     },
							       { "REFRESH", &pflst0a::action_Refresh1 },
							       { "RESET",   &pflst0a::action_Reset1   },
							       { "SEARCH",  &pflst0a::action_Search   },
							       { "TOUCH",   &pflst0a::action_Touch1   },
							       { "VIEW",    &pflst0a::action_View     } } ;


		map<string, int(pflst0a::*)( const string& )>commandList3 =
						  { { "ADD",    &pflst0a::action_Block_Add     },
						    { "BROWSE", &pflst0a::action_Block_Browse  },
						    { "COPY",   &pflst0a::action_Block_Copy    },
						    { "CCOPY",  &pflst0a::action_Block_CCopy   },
						    { "DELETE", &pflst0a::action_Block_Delete  },
						    { "EDIT",   &pflst0a::action_Block_Edit    },
						    { "FS",     &pflst0a::action_Block_Fsys    },
						    { "INFO",   &pflst0a::action_Block_Info    },
						    { "LIST",   &pflst0a::action_Block_List    },
						    { "MODIFY", &pflst0a::action_Block_Modify  },
						    { "RENAME", &pflst0a::action_Block_Rename  },
						    { "VIEW",   &pflst0a::action_Block_View    } } ;


		map<string, LN_CMDS> line_cmds  = { { "ADD",   LN_ADD      },
						    { "B",     LN_BROWSE   },
						    { "C",     LN_COPY     },
						    { "CC",    LN_CCOPY    },
						    { "D",     LN_DELETE   },
						    { "E",     LN_EDIT     },
						    { "EX",    LN_EXECUTE1 },
						    { "<",     LN_EXECUTE2 },
						    { "FS",    LN_FS       },
						    { "I",     LN_INFO     },
						    { "L",     LN_LIST     },
						    { "LX",    LN_LISTX    },
						    { "M",     LN_MODIFY   },
						    { "NANO",  LN_NANO     },
						    { "R",     LN_RENAME   },
						    { "SUB",   LN_SUBMIT   },
						    { "T",     LN_TREE     },
						    { "TT",    LN_TTREE    },
						    { "V",     LN_VIEW     },
						    { "VI",    LN_VI       },
						    { "X",     LN_LINK     },
						    { "/X",    LN_EXCLUDE  },
						    { "ZIP",   LN_ZIP      },
						    { "UNZIP", LN_UNZIP    } } ;

		map<string,string> aliasNames = { { "S",       "BACK"     },
						  { "B",       "BROWSE"   },
						  { "E",       "EDIT"     },
						  { "EX",      "EXPAND"   },
						  { "EXP",     "EXPAND"   },
						  { "F",       "FIND"     },
						  { "L",       "LOCATE"   },
						  { "LOC",     "LOCATE"   },
						  { "P",       "PREVIOUS" },
						  { "PREV",    "PREVIOUS" },
						  { "REF",     "REFRESH"  },
						  { "RES",     "RESET"    },
						  { "SE",      "SEARCH"   },
						  { "SRCHFOR", "SEARCH"   },
						  { "SHOW",    "SHOWCMD"  },
						  { "STAT",    "STATS"    },
						  { "V",       "VIEW"     } } ;
} ;
