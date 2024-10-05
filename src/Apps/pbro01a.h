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


enum BHLT_STATUS
{
	BHLT_RUNNING,
	BHLT_STOPPING,
	BHLT_STOPPED
} ;

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

	void set( const string& s1,
		  const string& s2 )
	{
		zcmd = s1 ;
		cmd  = s2 ;
		w2   = upper( word( s1, 2 ) ) ;
		w3   = upper( word( s1, 3 ) ) ;
		msg  = "" ;
		RC   = 0  ;
		wds  = words( s1 ) ;

	}

	void errorid( const string& id,
		      int rcode=20 )
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

	void set( const string& a,
		  int b )
	{
		field = a ;
		pos   = b ;
	}

	void set( const string& a,
		  int b,
		  int c,
		  int topLine,
		  bool hexOn,
		  bool colsOn )
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

	void set( const string& a,
		  int b,
		  int c )
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

	void placecursor( int depth,
			  int width,
			  int top,
			  int scol,
			  bool hexOn,
			  bool colsOn )
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
				else if ( j > width ) { j = width ; }
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
		string bs_line ;
		bool   bs_vShadow ;
		bool   bs_wShadow ;
		bool   bs_nsect   ;
	b_shadow()
	{
		bs_Shadow  = ""    ;
		bs_line    = ""    ;
		bs_vShadow = false ;
		bs_wShadow = false ;
		bs_nsect   = false ;
	}
	friend class pbro01a ;
} ;


class b_find
{
	public:
		string f_string  ;
		string f_ostring ;
		string f_rstring ;
		int    f_occurs  ;
		int    f_line    ;
		int    f_lines   ;
		int    f_offset  ;
		int    f_pRow    ;
		int    f_pCol    ;
		int    f_ptopLine;
		bool   f_regreq  ;
		bool   f_text    ;
		bool   f_asis    ;
		bool   f_hex     ;
		bool   f_pic     ;
		bool   f_rreg    ;
		bool   f_bottom  ;
		bool   f_top     ;
		int    f_scol    ;
		int    f_ecol    ;
		bool   f_oncol   ;
		bool   f_fset    ;
		char   f_dir     ;
		char   f_mtch    ;
		boost::regex f_regexp ;

	b_find()
	{
		f_string   = ""    ;
		f_ostring  = ""    ;
		f_rstring  = ""    ;
		f_occurs   = 0     ;
		f_line     = 0     ;
		f_lines    = 0     ;
		f_offset   = 0     ;
		f_pRow     = 0     ;
		f_pCol     = 0     ;
		f_ptopLine = 0     ;
		f_regreq   = false ;
		f_text     = false ;
		f_asis     = false ;
		f_hex      = false ;
		f_pic      = false ;
		f_rreg     = false ;
		f_bottom   = false ;
		f_top      = false ;
		f_scol     = 0     ;
		f_ecol     = 0     ;
		f_oncol    = false ;
		f_fset     = false ;
		f_dir      = 'N'   ;
		f_mtch     = 'C'   ;
	}

	void set_rstring( const string& s )
	{
		if ( f_rstring == "" ) { f_rstring = s ; }
	}

} ;


class lang_colours
{
	public:
		lang_colours( const string& sp ) : lang_colours()
		{
			specials = sp ;
		}

		lang_colours( const string& cold,
			      const string& colc,
			      const string& colk,
			      const string& colq,
			      const string& colv,
			      const string& cols,
			      const string& _hid,
			      const string& _hic,
			      const string& _hik,
			      const string& _hiq,
			      const string& _hiv,
			      const string& _his,
			      const string& specials ) : cold( cold ), colc( colc ), colk( colk ), colq( colq ), colv( colv ), cols( cols ),
							 specials( specials )
		{
			hid = ( _hid != "NORMAL" ) ? _hid : "" ;
			hic = ( _hic != "NORMAL" ) ? _hic : "" ;
			hik = ( _hik != "NORMAL" ) ? _hik : "" ;
			hiq = ( _hiq != "NORMAL" ) ? _hiq : "" ;
			hiv = ( _hiv != "NORMAL" ) ? _hiv : "" ;
			his = ( _his != "NORMAL" ) ? _his : "" ;
		}

		lang_colours()
		{
			cold = "GREEN" ;
			colc = "TURQ" ;
			colk = "RED" ;
			colq = "WHITE" ;
			colv = "BLUE" ;
			cols = "YELLOW" ;
		}

		lang_colours operator << ( const lang_colours& rhs )
		{
			cold = rhs.cold ;
			colc = rhs.colc ;
			colk = rhs.colk ;
			colq = rhs.colq ;
			colv = rhs.colv ;
			cols = rhs.cols ;
			hid  = rhs.hid ;
			hic  = rhs.hic ;
			hik  = rhs.hik ;
			hiq  = rhs.hiq ;
			hiv  = rhs.hiv ;
			his  = rhs.his ;

			return *this ;
		}

		string cold ;
		string colc ;
		string colk ;
		string colq ;
		string colv ;
		string cols ;

		string hid ;
		string hic ;
		string hik ;
		string hiq ;
		string hiv ;
		string his ;

		string specials ;
} ;


class pbro01a : public pApplication
{
	public:
		pbro01a() ;
		void application() ;

	private:
		static map<int, b_find>Global_bfind_parms ;
		const string ansi_start = "\x1B[" ;
		const char   ansi_end   = 'm' ;
		const string ansi_codes = "0123456789; " ;

		map<string, int>labelList ;

		a_cursor cursor ;

		BHLT_STATUS bhltStatus ;

		boost::condition cond_hlt ;

		void Browse() ;
		void initialise() ;
		void read_file() ;
		void fill_dynamic_area() ;
		void fill_hilight_shadow() ;
		void fill_zshadow() ;
		void set_label( a_parms& ) ;
		bool check_label( const string&, a_parms& ) ;
		void load_language_colours() ;
		void set_language_fvars( const string& lang ) ;

		string get_profile_var( const string& ) ;

		void term_resize() ;

		int  setFind( a_parms& ) ;
		void performFind() ;
		void setNotFoundMsg( a_parms& ) ;

		string setFoundString() ;
		string getColumnLine() ;

		void hilightData() ;

		void action_ANSI( a_parms& )  ;
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

		string get_shared_var( const string& ) ;

		string determineLang() ;
		void   hilite_cursor() ;

		int  topLine  ;
		uint maxLines ;
		int  startCol ;

		uint maxCol  ;

		map<string, lang_colours> langColours ;
		map<string, string> langSpecials ;

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
		bool ansi_on ;
		bool hilightOn ;
		bool hiComplete ;
		bool rebuildZAREA ;

		bool optNoConvTabs ;

		string zcmd    ;
		string zverb   ;
		string zcurfld ;
		int    zcurpos ;
		int    curpos  ;

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
		int    reclen  ;
		int    zareaw  ;
		int    zaread  ;
		int    zscreend ;
		int    zscreenw ;
		int    yaread  ;
		int    zlvline ;
		uint   zasize  ;
		int    zaline  ;
		string fscroll ;
		string zscrolla ;
		int    zscrolln ;
		int    hltdl ;

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

		string zedclrd ;
		string zedclrc ;
		string zedclrk ;
		string zedclrq ;
		string zedclrv ;
		string zedclrs ;

		string zedhid ;
		string zedhic ;
		string zedhik ;
		string zedhiq ;
		string zedhiv ;
		string zedhis ;

		map<char, string>typeList = { { 'C', "CHARS"  },
					      { 'P', "PREFIX" },
					      { 'S', "SUFFIX" },
					      { 'W', "WORD"   } } ;

		map<string, void(pbro01a::*)()>zverbList = { { "UP",    &pbro01a::action_ScrollUp    },
							     { "DOWN",  &pbro01a::action_ScrollDown  },
							     { "LEFT",  &pbro01a::action_ScrollLeft  },
							     { "RIGHT", &pbro01a::action_ScrollRight },
							     { "RFIND", &pbro01a::action_RFIND       } } ;

		map<string, void(pbro01a::*)( a_parms& )>cmdList = { { "ANSI",    &pbro01a::action_ANSI    },
								     { "BIN",     &pbro01a::action_BINARY  },
								     { "BINARY",  &pbro01a::action_BINARY  },
								     { "BROWSE",  &pbro01a::action_BROWSE  },
								     { "COLS",    &pbro01a::action_COLS    },
								     { "COLUMNS", &pbro01a::action_COLS    },
								     { "EDIT",    &pbro01a::action_EDIT    },
								     { "F",       &pbro01a::action_FIND    },
								     { "FIND",    &pbro01a::action_FIND    },
								     { "HEX",     &pbro01a::action_HEX     },
								     { "HI",      &pbro01a::action_HILIGHT },
								     { "HILIGHT", &pbro01a::action_HILIGHT },
								     { "HILIITE", &pbro01a::action_HILIGHT },
								     { "L",       &pbro01a::action_LOCATE  },
								     { "LOC",     &pbro01a::action_LOCATE  },
								     { "LOCATE",  &pbro01a::action_LOCATE  },
								     { "RES",     &pbro01a::action_RESET   },
								     { "RESET",   &pbro01a::action_RESET   },
								     { "TEXT",    &pbro01a::action_TEXT    } } ;
} ;
