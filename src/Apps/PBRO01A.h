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

class a_parms
{
	public:
		string zcmd ;
		string cmd  ;
		string w2   ;
		string w3   ;
		string msg  ;

		int    wds  ;
		int    RC   ;

	a_parms()
	{
		clear() ;
	}

	void clear()
	{
		zcmd = "" ;
		cmd  = "" ;
		w2   = "" ;
		w3   = "" ;
		msg  = "" ;
		wds  = 0  ;
		RC   = 0  ;
	}

	void set( const string& s1, const string& s2 )
	{
		zcmd = s1 ;
		cmd  = s2 ;
		w2   = upper( word( s1, 2 ) ) ;
		w3   = upper( word( s1, 3 ) ) ;
		msg  = "" ;
		RC   = 0  ;
		wds  = words( s1 ) ;

	}

	void errorid( const string& id, int rcode=20 )
	{
		msg = id ;
		RC  = rcode ;
	}

	bool error()
	{
		return ( RC > 8 ) ;
	}
} ;


class a_cursor
{
	public:
		string field  ;
		int    line   ;
		int    offset ;
		int    pos    ;

	a_cursor()
	{
		clear() ;
	}

	void clear()
	{
		field  = "" ;
		line   = 0  ;
		offset = 0  ;
		pos    = 1  ;
	}

	void set( const string& a, int b )
	{
		field = a ;
		pos   = b ;
	}

	void set( const string& a, int b, int c, int topLine, bool hexOn, bool colsOn )
	{
		field  = a ;
		line   = b ;
		offset = c ;
		if ( colsOn ) { --line ; }
		if ( hexOn  )
		{
			line = line / 4 + 1 ;
			if ( topLine == 0 ) { ++line ; }
		}
		line += topLine - 1 ;
		if ( line < 1 ) { home() ; }
	}

	void set( const string& a, int b, int c )
	{
		field  = a ;
		line   = b ;
		offset = c ;
	}

	void set( int b )
	{
		pos = b ;
	}

	void home()
	{
		field = "ZCMD" ;
		pos   = 1 ;
	}

	void placecursor( int depth, int width, int top, int scol, bool hexOn, bool colsOn )
	{
		int i ;
		int j ;

		if ( field == "ZAREA" )
		{
			i = line - top ;
			if ( hexOn )
			{
				i = 4*i ;
				if ( top == 0 ) { i = i - 3 ; }
			}
			if ( colsOn ) { ++i ; }
			if ( i < 0 || i >= depth )
			{
				home() ;
			}
			else
			{
				j = offset - scol + 2 ;
				if ( j < 0 ) { j = 0 ; }
				else if ( j >= width ) { j = width - 1 ; }
				pos = i * width + j ;
			}
		}
	}

	const string& curfld()
	{
		return field ;
	}

	int curpos()
	{
		return pos ;
	}
} ;


class b_shadow
{
	private:
		string bs_Shadow ;
		bool   bs_vShadow ;
		bool   bs_wShadow ;
	b_shadow()
	{
		bs_Shadow  = ""    ;
		bs_vShadow = false ;
		bs_wShadow = false ;
	}
	friend class PBRO01A ;
} ;


class b_find
{
	public:
		string f_string  ;
		string f_estring ;
		int    f_occurs  ;
		int    f_line    ;
		int    f_lines   ;
		int    f_offset  ;
		bool   f_regreq  ;
		bool   f_text    ;
		bool   f_asis    ;
		bool   f_hex     ;
		bool   f_pic     ;
		bool   f_rreg    ;
		int    f_scol    ;
		int    f_ecol    ;
		bool   f_oncol   ;
		bool   f_fset    ;
		char   f_dir     ;
		char   f_mtch    ;
		boost::regex f_regexp ;
	b_find()
	{
		f_string  = ""    ;
		f_estring = ""    ;
		f_occurs  = 0     ;
		f_line    = 0     ;
		f_lines   = 0     ;
		f_offset  = 0     ;
		f_regreq  = false ;
		f_text    = false ;
		f_asis    = false ;
		f_hex     = false ;
		f_pic     = false ;
		f_rreg    = false ;
		f_scol    = 0     ;
		f_ecol    = 0     ;
		f_oncol   = false ;
		f_fset    = false ;
		f_dir     = 'N'   ;
		f_mtch    = 'C'   ;
	}
} ;


class PBRO01A : public pApplication
{
	public:
		PBRO01A() ;
		void application() ;

	private:
		static map<int, b_find>Global_bfind_parms ;

		map<string, int>labelList ;

		a_cursor cursor ;

		void Browse()               ;
		void initialise()           ;
		void read_file()            ;
		void fill_dynamic_area()    ;
		void fill_hilight_shadow()  ;
		void set_label( a_parms& )  ;
		bool check_label( const string&, a_parms& );

		int  setFind( a_parms& ) ;
		void performFind( int, int ) ;

		void action_BINARY( a_parms& ) ;
		void action_BROWSE( a_parms& ) ;
		void action_COLS( a_parms& )  ;
		void action_EDIT( a_parms& )  ;
		void action_FIND( a_parms& )  ;
		void action_HEX( a_parms& )   ;
		void action_HILIGHT( a_parms& ) ;
		void action_LOCATE( a_parms& );
		void action_RESET( a_parms& ) ;
		void action_TEXT( a_parms& )  ;

		void action_ScrollLeft()  ;
		void action_ScrollRight() ;
		void action_ScrollUp()   ;
		void action_ScrollDown() ;
		void action_RFIND() ;

		string determineLang() ;
		void   hilite_cursor() ;

		int  topLine  ;
		uint maxLines ;
		int  startCol ;

		uint maxCol  ;

		vector<string> data     ;
		vector<b_shadow> shadow ;

		b_find find_parms ;
		hilight hlight    ;

		int  XRC  ;
		int  XRSN ;

		bool hexOn  ;
		bool vertOn ;
		bool colsOn ;
		bool asBin  ;
		bool binOn  ;
		bool textOn ;
		bool hilightOn ;
		bool rebuildZAREA ;

		string zcmd    ;
		string zverb   ;
		string zcurfld ;
		int    zcurpos ;
		int    curpos  ;
		int    offset  ;

		string zfile   ;
		string panel   ;
		string msg     ;
		string cmd     ;
		string curfld  ;
		string zrow1   ;
		string zcol1   ;
		string zcol2   ;
		string zarea   ;
		string zshadow ;
		string cshadow ;
		string zscrnum ;
		int    zareaw  ;
		int    zaread  ;
		uint   zasize  ;
		int    zaline  ;
		string zscrolla ;
		int    zscrolln ;

		string s1b ;
		string s1g ;
		string s1y ;
		string s1w ;
		string div ;

		string type  ;
		string str   ;
		string occ   ;
		string lines ;

		string zzstr1   ;
		string fileType ;
		string detLang  ;
		string entLang  ;

		map<char, string>typeList = { { 'C', "CHARS"  },
					      { 'P', "PREFIX" },
					      { 'S', "SUFFIX" },
					      { 'W', "WORD"   } } ;

		map<string, void(PBRO01A::*)()>zverbList = { { "UP",    &PBRO01A::action_ScrollUp    },
							     { "DOWN",  &PBRO01A::action_ScrollDown  },
							     { "LEFT",  &PBRO01A::action_ScrollLeft  },
							     { "RIGHT", &PBRO01A::action_ScrollRight },
							     { "RFIND", &PBRO01A::action_RFIND       } } ;

		map<string, void(PBRO01A::*)( a_parms& )>cmdList = { { "BIN",     &PBRO01A::action_BINARY  },
								     { "BINARY",  &PBRO01A::action_BINARY  },
								     { "BROWSE",  &PBRO01A::action_BROWSE  },
								     { "COLS",    &PBRO01A::action_COLS    },
								     { "COLUMNS", &PBRO01A::action_COLS    },
								     { "EDIT",    &PBRO01A::action_EDIT    },
								     { "F",       &PBRO01A::action_FIND    },
								     { "FIND",    &PBRO01A::action_FIND    },
								     { "HEX",     &PBRO01A::action_HEX     },
								     { "HI",      &PBRO01A::action_HILIGHT },
								     { "HILIGHT", &PBRO01A::action_HILIGHT },
								     { "HILIITE", &PBRO01A::action_HILIGHT },
								     { "L",       &PBRO01A::action_LOCATE  },
								     { "LOC",     &PBRO01A::action_LOCATE  },
								     { "LOCATE",  &PBRO01A::action_LOCATE  },
								     { "RES",     &PBRO01A::action_RESET   },
								     { "RESET",   &PBRO01A::action_RESET   },
								     { "TEXT",    &PBRO01A::action_TEXT    } } ;
} ;
