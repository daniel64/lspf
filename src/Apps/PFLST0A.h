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

enum LN_CMDS
{
	LN_ADD,
	LN_BROWSE,
	LN_COPY,
	LN_DELETE,
	LN_EDIT,
	LN_EXECUTE,
	LN_FORMAT,
	LN_INFO,
	LN_LIST,
	LN_MODIFY,
	LN_NANO,
	LN_RENAME,
	LN_SUBMIT,
	LN_TREE,
	LN_TTREE,
	LN_VIEW,
	LN_VI,
	LN_LINK
} ;


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
		string lcmtab   ;
		string eprof    ;
		string eimac    ;
		string eccan    ;
		string epresv   ;
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

		map<string,LN_CMDS> line_cmds  = { { "ADD",      LN_ADD       },
						   { "B",        LN_BROWSE    },
						   { "C",        LN_COPY      },
						   { "D",        LN_DELETE    },
						   { "E",        LN_EDIT      },
						   { "EX",       LN_EXECUTE   },
						   { "FMT",      LN_FORMAT    },
						   { "I",        LN_INFO      },
						   { "L",        LN_LIST      },
						   { "M",        LN_MODIFY    },
						   { "NANO",     LN_NANO      },
						   { "R",        LN_RENAME    },
						   { "SUB",      LN_SUBMIT    },
						   { "T",        LN_TREE      },
						   { "TT",       LN_TTREE     },
						   { "V",        LN_VIEW      },
						   { "VI",       LN_VI        },
						   { "X",        LN_LINK      } } ;

} ;
