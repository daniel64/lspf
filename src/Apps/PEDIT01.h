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

enum P_CMDS
{
	PC_INVCMD,
	PC_AUTOSAVE,
	PC_BOUNDS,
	PC_BROWSE,
	PC_CANCEL,
	PC_CHANGE,
	PC_CAPS,
	PC_COLUMN,
	PC_COMPARE,
	PC_COPY,
	PC_CREATE,
	PC_CUT,
	PC_DEFINE,
	PC_DELETE,
	PC_EDIT,
	PC_EDITSET,
	PC_EXCLUDE,
	PC_FIND,
	PC_FLIP,
	PC_HEX,
	PC_HIDE,
	PC_HILIGHT,
	PC_IMACRO,
	PC_LOCATE,
	PC_NULLS,
	PC_PASTE,
	PC_PROFILE,
	PC_PRESERVE,
	PC_RECOVERY,
	PC_REDO,
	PC_REPLACE,
	PC_RESET,
	PC_SAVE,
	PC_SETUNDO,
	PC_SORT,
	PC_TABS,
	PC_UNDO,
	PC_XTABS
} ;

enum L_CMDS
{
	LC_A,
	LC_AK,
	LC_B,
	LC_BK,
	LC_BOUNDS,
	LC_C,
	LC_CC,
	LC_CHANGE,
	LC_COLS,
	LC_COPY,
	LC_D,
	LC_DD,
	LC_F,
	LC_HX,
	LC_HXX,
	LC_I,
	LC_L,
	LC_LC,
	LC_LCC,
	LC_M,
	LC_MM,
	LC_MASK,
	LC_MD,
	LC_MDD,
	LC_MN,
	LC_MNN,
	LC_O,
	LC_OK,
	LC_OO,
	LC_OOK,
	LC_PASTE,
	LC_R,
	LC_RR,
	LC_S,
	LC_SI,
	LC_SLC,
	LC_SLCC,
	LC_SLD,
	LC_SLDD,
	LC_SLTC,
	LC_SLTCC,
	LC_SRC,
	LC_SRCC,
	LC_SRD,
	LC_SRDD,
	LC_SRTC,
	LC_SRTCC,
	LC_TABS,
	LC_TJ,
	LC_TJJ,
	LC_TR,
	LC_TRR,
	LC_TS,
	LC_T,
	LC_TT,
	LC_TX,
	LC_TXX,
	LC_UC,
	LC_UCC,
	LC_W,
	LC_WW,
	LC_X,
	LC_XX,
	LC_XI
} ;


enum LN_TYPE
{
	LN_FILE,
	LN_ISRT,
	LN_NOTE,
	LN_PROF,
	LN_COL,
	LN_BNDS,
	LN_MASK,
	LN_TABS,
	LN_TOD,
	LN_BOD,
	LN_MSG,
	LN_INFO,
	LN_INVALID
} ;


enum LS_TYPE
{
	LS_NONE,
	LS_CHNG,
	LS_ERROR,
	LS_UNDO,
	LS_REDO
} ;


enum M_CMDS
{
	EM_INVCMD,
	EM_AUTOSAVE,
	EM_CANCEL,
	EM_CAPS,
	EM_CHANGE,
	EM_CHANGE_COUNTS,
	EM_CURSOR,
	EM_DATASET,
	EM_DATA_CHANGED,
	EM_DEFINE,
	EM_DELETE,
	EM_DISPLAY_COLS,
	EM_DISPLAY_LINES,
	EM_END,
	EM_EXCLUDE,
	EM_EXCLUDE_COUNTS,
	EM_FIND,
	EM_FIND_COUNTS,
	EM_FLIP,
	EM_HEX,
	EM_HIDE,
	EM_IMACRO,
	EM_INSERT,
	EM_LABEL,
	EM_LINE,
	EM_LINE_AFTER,
	EM_LINE_BEFORE,
	EM_LINENUM,
	EM_LOCATE,
	EM_MACRO,
	EM_MACRO_LEVEL,
	EM_MASKLINE,
	EM_NULLS,
	EM_PROCESS,
	EM_PROFILE,
	EM_RCHANGE,
	EM_RFIND,
	EM_SAVE,
	EM_SCAN,
	EM_SEEK,
	EM_SEEK_COUNTS,
	EM_SHIFT,
	EM_TABSLINE,
	EM_RESET,
	EM_XSTATUS
} ;


enum D_PARMS
{
	DF_MACRO,
	DF_ALIAS,
	DF_NOP,
	DF_DISABLED,
	DF_RESET,
} ;


class pcmd_entry
{
	public:
	       P_CMDS p_cmd    ;
	       string truename ;
} ;


class pcmd_format
{
	public:
	       int noptions1 ;
	       int noptions2 ;
} ;


class mcmd_format
{
	public:
	       M_CMDS m_cmd   ;
	       int qnvars1    ;
	       int qnvars2    ;
	       int qnkeyopts1 ;
	       int qnkeyopts2 ;
	       int ankeyopts1 ;
	       int ankeyopts2 ;
	       int anvalopts1 ;
	       int anvalopts2 ;
} ;
				  //       +----------------------------Query: # variables(min,max)
				  //       |       +--------------------Query: # key options(min,max)
				  //       |       |       +-----------Action: # key options(min,max
				  //       |       |       |       +---Action: # value options(min,max)
				  //       |       |       |       |             -1 = don't check
				  //       V       V       V       V
map<string,mcmd_format> EMServ =  //       ~~~~~   ~~~~~   ~~~~~   ~~~~~~
{ { "AUTOSAVE",       { EM_AUTOSAVE,       1,  2,  0,  0,  0,  0,  0,  2  } },
  { "CAPS",           { EM_CAPS,           1,  1,  0,  0,  0,  0,  1,  1  } },
  { "CHANGE_COUNTS",  { EM_CHANGE_COUNTS,  1,  2,  0,  0, -1, -1, -1, -1  } },
  { "CURSOR",         { EM_CURSOR,         1,  2,  0,  0,  0,  0,  1,  2  } },
  { "DATA_CHANGED",   { EM_DATA_CHANGED,   1,  1,  0,  0, -1, -1, -1, -1  } },
  { "DATASET",        { EM_DATASET,        1,  1,  0,  0, -1, -1, -1, -1  } },
  { "DISPLAY_COLS",   { EM_DISPLAY_COLS,   1,  2,  0,  0, -1, -1, -1, -1  } },
  { "DISPLAY_LINES",  { EM_DISPLAY_LINES,  1,  2,  0,  0, -1, -1, -1, -1  } },
  { "EXCLUDE_COUNTS", { EM_EXCLUDE_COUNTS, 1,  2,  0,  0, -1, -1, -1, -1  } },
  { "FIND_COUNTS",    { EM_FIND_COUNTS,    1,  2,  0,  0, -1, -1, -1, -1  } },
  { "HEX",            { EM_HEX,            1,  2,  0,  0, -1, -1, -1, -1  } },
  { "IMACRO",         { EM_IMACRO,         1,  1,  0,  0, -1, -1, -1, -1  } },
  { "LABEL",          { EM_LABEL,          1,  2,  1,  1,  1,  1,  1,  2  } },
  { "LINE",           { EM_LINE,           1,  1,  1,  1,  1,  1, -1, -1  } },
  { "LINE_AFTER",     { EM_LINE_AFTER,    -1, -1, -1, -1,  1,  1, -1, -1  } },
  { "LINE_BEFORE",    { EM_LINE_BEFORE,   -1, -1, -1, -1,  1,  1, -1, -1  } },
  { "LINENUM",        { EM_LINENUM,        1,  1,  1,  1, -1, -1,  1, -1  } },
  { "MACRO_LEVEL",    { EM_MACRO_LEVEL,    1,  1,  0,  0, -1, -1, -1, -1  } },
  { "MASKLINE",       { EM_MASKLINE,       1,  1,  0,  0,  0,  0, -1, -1  } },
  { "NULLS",          { EM_NULLS,          1,  2,  0,  0, -1, -1, -1, -1  } },
  { "PROFILE",        { EM_PROFILE,        1,  2,  0,  0, -1, -1, -1, -1  } },
  { "SCAN",           { EM_SCAN,           1,  1,  0,  0,  0,  0,  1,  1  } },
  { "SEEK_COUNTS",    { EM_SEEK_COUNTS,    1,  2,  0,  0, -1, -1, -1, -1  } },
  { "TABSLINE",       { EM_TABSLINE,       1,  1,  0,  0,  0,  0, -1, -1  } },
  { "XSTATUS",        { EM_XSTATUS,        1,  1,  1,  1,  1,  1,  1,  1  } } } ;


map<string,M_CMDS> EMCmds =
{ { "AUTOSAVE",     EM_AUTOSAVE },
  { "CANCEL",       EM_CANCEL   },
  { "CAPS",         EM_CAPS     },
  { "CHANGE",       EM_CHANGE   },
  { "DEFINE",       EM_DEFINE   },
  { "DELETE",       EM_DELETE   },
  { "END",          EM_END      },
  { "EXCLUDE",      EM_EXCLUDE  },
  { "FIND",         EM_FIND     },
  { "FLIP",         EM_FLIP     },
  { "HEX",          EM_HEX      },
  { "HIDE",         EM_HIDE     },
  { "IMACRO",       EM_IMACRO   },
  { "INSERT",       EM_INSERT   },
  { "LOCATE",       EM_LOCATE   },
  { "NULLS",        EM_NULLS    },
  { "PROCESS",      EM_PROCESS  },
  { "PROFILE",      EM_PROFILE  },
  { "RCHANGE",      EM_RCHANGE  },
  { "RESET",        EM_RESET    },
  { "RFIND",        EM_RFIND    },
  { "SAVE",         EM_SAVE     },
  { "SCAN",         EM_SCAN,    },
  { "SEEK",         EM_SEEK     },
  { "SHIFT",        EM_SHIFT    } } ;


map<string,pcmd_entry> PrimCMDS =
{ { "AUTOSAVE", { PC_AUTOSAVE, "AUTOSAVE" } },
  { "BND",      { PC_BOUNDS,   "BOUNDS"   } },
  { "BNDS",     { PC_BOUNDS,   "BOUNDS"   } },
  { "BOU",      { PC_BOUNDS,   "BOUNDS"   } },
  { "BOUND",    { PC_BOUNDS,   "BOUNDS"   } },
  { "BOUNDS",   { PC_BOUNDS,   "BOUNDS"   } },
  { "BROWSE",   { PC_BROWSE,   "BROWSE"   } },
  { "CANCEL",   { PC_CANCEL,   "CANCEL"   } },
  { "CAN",      { PC_CANCEL,   "CANCEL"   } },
  { "CHANGE",   { PC_CHANGE,   "CHANGE"   } },
  { "C",        { PC_CHANGE,   "CHANGE"   } },
  { "CHA",      { PC_CHANGE,   "CHANGE"   } },
  { "CHG",      { PC_CHANGE,   "CHANGE"   } },
  { "CAPS",     { PC_CAPS,     "CAPS"     } },
  { "COLUMN",   { PC_COLUMN,   "COLUMN"   } },
  { "COL",      { PC_COLUMN,   "COLUMN"   } },
  { "COLS",     { PC_COLUMN,   "COLUMN"   } },
  { "COMPARE",  { PC_COMPARE,  "COMPARE"  } },
  { "COMP",     { PC_COMPARE,  "COMPARE"  } },
  { "COPY",     { PC_COPY,     "COPY"     } },
  { "CREATE",   { PC_CREATE,   "CREATE"   } },
  { "CRE",      { PC_CREATE,   "CREATE"   } },
  { "CUT",      { PC_CUT,      "CUT"      } },
  { "DEFINE",   { PC_DEFINE,   "DEFINE"   } },
  { "DEF",      { PC_DEFINE,   "DEFINE"   } },
  { "DELETE",   { PC_DELETE,   "DELETE"   } },
  { "DEL",      { PC_DELETE,   "DELETE"   } },
  { "EDIT",     { PC_EDIT,     "EDIT"     } },
  { "EDITSET",  { PC_EDITSET,  "EDITSET"  } },
  { "EDSET",    { PC_EDITSET,  "EDITSET"  } },
  { "EXCLUDE",  { PC_EXCLUDE,  "EXCLUDE"  } },
  { "EXCLUDED", { PC_EXCLUDE,  "EXCLUDE"  } },
  { "EX",       { PC_EXCLUDE,  "EXCLUDE"  } },
  { "EXC",      { PC_EXCLUDE,  "EXCLUDE"  } },
  { "X",        { PC_EXCLUDE,  "EXCLUDE"  } },
  { "FIND",     { PC_FIND,     "FIND"     } },
  { "F",        { PC_FIND,     "FIND"     } },
  { "FLIP",     { PC_FLIP,     "FLIP"     } },
  { "HEX",      { PC_HEX,      "HEX"      } },
  { "HIDE",     { PC_HIDE,     "HIDE"     } },
  { "HILIGHT",  { PC_HILIGHT,  "HILIGHT"  } },
  { "HI",       { PC_HILIGHT,  "HILITE"   } },
  { "HILITE",   { PC_HILIGHT,  "HILITE"   } },
  { "IMACRO",   { PC_IMACRO,   "IMACRO"   } },
  { "IMAC",     { PC_IMACRO,   "IMACRO"   } },
  { "LOCATE",   { PC_LOCATE,   "LOCATE"   } },
  { "LOC",      { PC_LOCATE,   "LOCATE"   } },
  { "L",        { PC_LOCATE,   "LOCATE"   } },
  { "NULLS",    { PC_NULLS,    "NULLS"    } },
  { "NULL",     { PC_NULLS,    "NULLS"    } },
  { "NUL",      { PC_NULLS,    "NULLS"    } },
  { "PASTE",    { PC_PASTE,    "PASTE"    } },
  { "PRESERVE", { PC_PRESERVE, "PRESERVE" } },
  { "PROFILE",  { PC_PROFILE,  "PROFILE"  } },
  { "PROF",     { PC_PROFILE,  "PROFILE"  } },
  { "PRO",      { PC_PROFILE,  "PROFILE"  } },
  { "PR",       { PC_PROFILE,  "PROFILE"  } },
  { "RECVR",    { PC_RECOVERY, "RECOVERY" } },
  { "RECVRY",   { PC_RECOVERY, "RECOVERY" } },
  { "RECOV",    { PC_RECOVERY, "RECOVERY" } },
  { "RECOVRY",  { PC_RECOVERY, "RECOVERY" } },
  { "RECOVER",  { PC_RECOVERY, "RECOVERY" } },
  { "RECOVERY", { PC_RECOVERY, "RECOVERY" } },
  { "REDO",     { PC_REDO,     "REDO"     } },
  { "REPLACE",  { PC_CREATE,   "REPLACE"  } },
  { "REP",      { PC_CREATE,   "REPLACE"  } },
  { "REPL",     { PC_CREATE,   "REPLACE"  } },
  { "RESET",    { PC_RESET,    "RESET"    } },
  { "RES",      { PC_RESET,    "RESET"    } },
  { "SAVE",     { PC_SAVE,     "SAVE"     } },
  { "SETUNDO",  { PC_SETUNDO,  "SETUNDO"  } },
  { "SETU",     { PC_SETUNDO,  "SETUNDO"  } },
  { "SORT",     { PC_SORT,     "SORT"     } },
  { "TABS",     { PC_TABS,     "TABS"     } },
  { "TAB",      { PC_TABS,     "TABS"     } },
  { "UNDO",     { PC_UNDO,     "UNDO"     } },
  { "XTABS",    { PC_XTABS,    "XTABS"    } }} ;


map<string,D_PARMS> DefParms =
{ { "MACRO",        DF_MACRO    },
  { "ALIAS",        DF_ALIAS    },
  { "NOP",          DF_NOP      },
  { "DISABLED",     DF_DISABLED },
  { "RESET",        DF_RESET    } } ;


class lcmd
{
	private:
		L_CMDS lcmd_CMD    ;
		string lcmd_CMDSTR ;
		int    lcmd_sURID  ;
		int    lcmd_eURID  ;
		int    lcmd_dURID  ;
		int    lcmd_lURID  ;
		int    lcmd_Rpt    ;
		char   lcmd_ABOW   ;
		bool   lcmd_swap   ;
		bool   lcmd_cut    ;
		bool   lcmd_create ;
		lcmd()
		{
			lcmd_CMDSTR  = " "   ;
			lcmd_sURID   = 0     ;
			lcmd_eURID   = 0     ;
			lcmd_dURID   = 0     ;
			lcmd_lURID   = 0     ;
			lcmd_ABOW    = ' '   ;
			lcmd_Rpt     = 0     ;
			lcmd_swap    = false ;
			lcmd_cut     = false ;
			lcmd_create  = false ;
		}
		void lcmd_clear()
		{
			lcmd_CMDSTR  = " "   ;
			lcmd_sURID   = 0     ;
			lcmd_eURID   = 0     ;
			lcmd_dURID   = 0     ;
			lcmd_lURID   = 0     ;
			lcmd_ABOW    = ' '   ;
			lcmd_Rpt     = 0     ;
			lcmd_swap    = false ;
			lcmd_cut     = false ;
			lcmd_create  = false ;
		}
	friend class PEDIT01 ;
} ;


class idata
{
	private:
		int    id_level   ;
		bool   id_deleted ;
		string id_data    ;
		idata()
		{
			id_level   = 0     ;
			id_deleted = false ;
			id_data    = ""    ;
		}
	friend class iline ;
} ;


class ipos
{
	public:
		uint   ipos_dl   ;
		uint   ipos_hex  ;
		int    ipos_URID ;
		bool   ipos_div  ;
		ipos()
		{
			ipos_dl   = 0 ;
			ipos_hex  = 0 ;
			ipos_URID = 0 ;
			ipos_div  = false ;
		}
		void clear()
		{
			ipos_dl   = 0 ;
			ipos_hex  = 0 ;
			ipos_URID = 0 ;
			ipos_div  = false ;
		}
} ;


class ipline
{
	private:
		LN_TYPE ip_type   ;
		LS_TYPE ip_status ;
		bool    ip_excl   ;
		bool    ip_hex    ;
		int     ip_profln ;
		string  ip_data   ;
		string  ip_label  ;
		ipline()
		{
			ip_type   = LN_INVALID ;
			ip_status = LS_NONE    ;
			ip_excl   = false ;
			ip_hex    = false ;
			ip_profln = 0     ;
			ip_label  = ""    ;
			ip_data   = ""    ;
		}
		bool is_file() const
		{
			return ( ip_type == LN_FILE ) ;
		}
		bool is_isrt() const
		{
			return ( ip_type == LN_ISRT ) ;
		}

	friend class PEDIT01 ;
} ;


class iline
{
	private:
		static map<int, stack<int>>Global_Undo ;
		static map<int, stack<int>>Global_Redo ;
		static map<int, stack<int>>Global_File_level ;
		static map<int, int>maxURID    ;
		static map<int, bool>setUNDO   ;
		static map<int, bool>Redo_data ;

		LN_TYPE il_type    ;
		LS_TYPE il_status  ;
		bool    il_excl    ;
		bool    il_hex     ;
		bool    il_mark    ;
		bool    il_deleted ;
		string  il_lcc     ;
		int     il_profln  ;
		int     il_URID    ;
		int     il_taskid  ;
		string  il_Shadow  ;
		bool    il_vShadow ;
		bool    il_wShadow ;
		map    <int, string>il_label ;
		stack  <idata> il_idata      ;
		stack  <idata> il_idata_redo ;

		explicit iline( int taskid )
		{
			il_type    = LN_INVALID ;
			il_status  = LS_NONE    ;
			il_excl    = false  ;
			il_hex     = false  ;
			il_mark    = false  ;
			il_deleted = false  ;
			il_lcc     = ""     ;
			il_profln  = 0      ;
			il_taskid  = taskid ;
			il_URID    = ++maxURID[ taskid ] ;
			il_Shadow  = ""     ;
			il_vShadow = false  ;
			il_wShadow = false  ;
		}
		bool is_file() const
		{
			return ( il_type == LN_FILE ) ;
		}
		bool is_tod() const
		{
			return ( il_type == LN_TOD ) ;
		}
		bool is_bod() const
		{
			return ( il_type == LN_BOD ) ;
		}
		bool is_tod_or_bod() const
		{
			return ( il_type == LN_TOD || il_type == LN_BOD ) ;
		}
		bool is_note() const
		{
			return ( il_type == LN_NOTE ) ;
		}
		bool is_info() const
		{
			return ( il_type == LN_INFO ) ;
		}
		bool is_msg() const
		{
			return ( il_type == LN_MSG ) ;
		}
		bool is_col() const
		{
			return ( il_type == LN_COL ) ;
		}
		bool is_prof() const
		{
			return ( il_type == LN_PROF ) ;
		}
		bool is_tabs() const
		{
			return ( il_type == LN_TABS ) ;
		}
		bool is_mask() const
		{
			return ( il_type == LN_MASK ) ;
		}
		bool is_bnds() const
		{
			return ( il_type == LN_BNDS ) ;
		}
		bool is_isrt() const
		{
			return ( il_type == LN_ISRT ) ;
		}
		bool is_chng() const
		{
			return ( il_status == LS_CHNG ) ;
		}
		bool is_error() const
		{
			return ( il_status == LS_ERROR ) ;
		}
		bool is_undo() const
		{
			return ( il_status == LS_UNDO ) ;
		}
		bool is_redo() const
		{
			return ( il_status == LS_REDO ) ;
		}
		void resetFilePrefix()
		{
			il_status = LS_NONE ;
			il_excl   = false ;
			il_mark   = false ;
		}
		void resetFileStatus()
		{
			il_status = LS_NONE ;
		}
		void setChngStatus()
		{
			il_status = LS_CHNG ;
		}
		void clrChngStatus()
		{
			if ( il_status == LS_CHNG ) { il_status = LS_NONE ; }
		}
		void clrErrorStatus()
		{
			if ( il_status == LS_ERROR ) { il_status = LS_NONE ; }
		}
		void clrUndoStatus()
		{
			if ( il_status == LS_UNDO ) { il_status = LS_NONE ; }
		}
		void clrRedoStatus()
		{
			if ( il_status == LS_REDO ) { il_status = LS_NONE ; }
		}
		void setErrorStatus()
		{
			il_status = LS_ERROR ;
		}
		void setUndoStatus()
		{
			il_status = LS_UNDO ;
		}
		void setRedoStatus()
		{
			il_status = LS_REDO ;
		}
		void resetMarked()
		{
			il_mark = false ;
		}
		void toggleMarked()
		{
			il_mark = !il_mark ;
		}
		bool isValidFile() const
		{
			return il_type == LN_FILE && !il_deleted ;
		}
		bool isSpecial() const
		{
			return is_note() || is_prof() || is_col()  || is_bnds() ||
			       is_mask() || is_tabs() || is_info() || is_msg()  ;
		}
		void clearLcc()
		{
			il_lcc = "" ;
		}
		bool hasLabel( int lvl=0 ) const
		{
			for ( int i = lvl ; i >= 0 ; i-- )
			{
				if ( il_label.count( i ) > 0 ) { return true ; }
			}
			return false ;
		}
		bool specialLabel( int lvl=0 )
		{
			if ( il_label.count( lvl ) > 0 )
			{
				return ( il_label[ lvl ].compare( 0, 2, ".O" ) == 0 ) ;
			}
			return false ;
		}
		string getLabel( int lvl=0 )
		{
			if ( il_label.count( lvl ) > 0 )
			{
				return il_label[ lvl ] ;
			}
			return "" ;
		}
		void getLabelInfo( int lvl, string& label, int& foundLvl )
		{
			label    = "" ;
			foundLvl = 0  ;

			for ( int i = lvl ; i >= 0 ; i-- )
			{
				if ( il_label.count( i ) > 0 )
				{
					label    = il_label[ i ] ;
					foundLvl = i ;
					break ;
				}
			}
			return ;
		}
		int setLabel( const string& s, int lvl=0 )
		{
			int rc = 0 ;

			if ( il_label.count( lvl ) > 0 ) { rc = 4 ; }
			il_label[ lvl ] = s ;
			return rc ;
		}
		void clearLabel( int lvl=0 )
		{
			if ( il_label.count( lvl ) > 0 )
			{
				il_label.erase( lvl ) ;
			}
		}
		bool clearLabel( const string& s, int lvl=0 )
		{
			if ( il_label.count( lvl ) > 0 && il_label[ lvl ] == s )
			{
				il_label.erase( lvl ) ;
				return true ;
			}
			return false ;
		}
		bool compareLabel( const string& s, int lvl )
		{
			if ( il_label.count( lvl ) > 0 && il_label[ lvl ] == s ) { return true ; }
			return false ;
		}
		bool put_idata( const string& s, int level, bool comp = true )
		{
			idata d ;

			if ( comp && !il_idata.empty() && s == il_idata.top().id_data )
			{
				return false ;
			}
			if ( !setUNDO[ il_taskid ] )
			{
				d.id_data = s ;
				if ( il_idata.empty() ) { il_idata.push( d ) ; }
				else                    { il_idata.top() = d ; }
			}
			else
			{
				d.id_level = level ;
				d.id_data  = s     ;
				il_idata.push( d ) ;
				if ( Global_Undo[ il_taskid ].empty() ||
				     Global_Undo[ il_taskid ].top() != level )
				{
					Global_Undo[ il_taskid ].push( level ) ;
				}
				if ( il_type == LN_FILE )
				{
					set_Global_File_level() ;
				}
			}
			il_vShadow = false  ;
			clear_Global_Redo() ;
			remove_redo_idata() ;
			return il_type == LN_FILE && !il_deleted ;
		}
		bool put_idata( const string& s )
		{
			idata d ;

			if ( il_idata.empty() )
			{
				d.id_data = s ;
				il_idata.push( d ) ;
			}
			else
			{
				if ( s == il_idata.top().id_data ) { return false ; }
				il_idata.top().id_data = s ;
			}
			il_vShadow = false  ;
			return il_type == LN_FILE && !il_deleted ;
		}
		void set_idata_minsize( int i )
		{
			il_idata.top().id_data.resize( i, ' ' ) ;
		}
		bool set_idata_upper( int Level )
		{
			if ( any_of( il_idata.top().id_data.begin(), il_idata.top().id_data.end(),
				   []( char c )
				   {
					return ( islower( c ) ) ;
				   } ) )
			{
				put_idata( upper( il_idata.top().id_data ), Level ) ;
				return il_type == LN_FILE ;
			}
			return false ;
		}
		bool set_idata_lower( int Level )
		{
			if ( any_of( il_idata.top().id_data.begin(), il_idata.top().id_data.end(),
				   []( char c )
				   {
					return ( isupper( c ) ) ;
				   } ) )
			{
				put_idata( lower( il_idata.top().id_data ), Level ) ;
				return il_type == LN_FILE ;
			}
			return false ;
		}
		bool set_idata_trim( int Level )
		{
			if ( il_idata.top().id_data.size() > 0 && il_idata.top().id_data.back() == ' ' )
			{
				put_idata( strip( il_idata.top().id_data, 'T', ' ' ), Level ) ;
				return il_type == LN_FILE ;
			}
			return false ;
		}
		void set_idata_trim()
		{
			trim_right( il_idata.top().id_data ) ;
		}
		void set_deleted( int Level )
		{
			if ( il_deleted ) { return ; }
			clearLabel() ;
			put_idata( "", Level, false ) ;
			il_idata.top().id_deleted = true ;
			il_deleted                = true ;
		}
		void convert_to_file( int level )
		{
			il_type = LN_FILE ;
			if ( !setUNDO[ il_taskid ] ) { return ; }
			il_idata.top().id_level = level ;
			if ( Global_Undo[ il_taskid ].empty()     ||
			     Global_Undo[ il_taskid ].top() != level )
			{
				Global_Undo[ il_taskid ].push( level ) ;
			}
			set_Global_File_level() ;
			clear_Global_Redo() ;
			remove_redo_idata() ;
		}
		const string& get_idata() const
		{
			return il_idata.top().id_data ;
		}
		int get_idata_len() const
		{
			return il_idata.top().id_data.size() ;
		}
		string::const_iterator get_idata_begin() const
		{
			return il_idata.top().id_data.begin() ;
		}
		string::const_iterator get_idata_end() const
		{
			return il_idata.top().id_data.end() ;
		}
		string * get_idata_ptr()
		{
			return &il_idata.top().id_data ;
		}
		bool idata_is_empty() const
		{
			return il_idata.empty() ;
		}
		void undo_idata()
		{
			if ( !setUNDO[ il_taskid ] ) { return ; }
			if ( il_idata.top().id_deleted )
			{
				il_deleted = false ;
			}
			il_idata_redo.push( il_idata.top() ) ;
			il_idata.pop()                       ;
			if ( il_idata.empty() )
			{
				il_deleted = true ;
			}
			il_excl = false ;
			Redo_data[ il_taskid ] = true ;
		}
		void redo_idata()
		{
			if ( !setUNDO[ il_taskid ] ) { return ; }
			il_deleted = il_idata_redo.top().id_deleted ;
			il_idata.push( il_idata_redo.top() ) ;
			il_idata_redo.pop() ;
			il_excl = false ;
		}
		void clear_Global_Undo()
		{
			while ( !Global_Undo[ il_taskid ].empty() )
			{
				Global_Undo[ il_taskid ].pop() ;
			}
		}
		void clear_Global_Redo()
		{
			while ( !Global_Redo[ il_taskid ].empty() )
			{
				Global_Redo[ il_taskid ].pop() ;
			}
		}
		bool is_Redo_data()
		{
			return Redo_data[ il_taskid ] ;
		}
		void reset_Redo_data()
		{
			Redo_data[ il_taskid ] = false ;
		}
		void reset_Global_Undo()
		{
			Global_Undo[ il_taskid ] = Global_File_level[ il_taskid ] ;
		}
		void clear_Global_File_level()
		{
			while ( !Global_File_level[ il_taskid ].empty() )
			{
				Global_File_level[ il_taskid ].pop() ;
			}
		}
		int get_Global_File_level() const
		{
			if ( Global_File_level[ il_taskid ].empty() )
			{
				return 0 ;
			}
			else
			{
				return Global_File_level[ il_taskid ].top() ;
			}
		}
		void set_Global_File_level()
		{
			if ( Global_File_level[ il_taskid ].empty() ||
			     Global_File_level[ il_taskid ].top() != Global_Undo[ il_taskid ].top() )
			{
				Global_File_level[ il_taskid ].push( Global_Undo[ il_taskid ].top() ) ;
			}
		}
		void remove_Global_File_level()
		{
			Global_File_level[ il_taskid ].pop() ;
		}
		int get_Global_Undo_level() const
		{
			if ( !setUNDO[ il_taskid ] ||
			     Global_Undo[ il_taskid ].empty() ) { return 0 ; }
			return Global_Undo[ il_taskid ].top() ;
		}
		int get_Global_Redo_level() const
		{
			if ( !setUNDO[ il_taskid ] ||
			     Global_Redo[ il_taskid ].empty() ) { return 0 ; }
			return Global_Redo[ il_taskid ].top() ;
		}
		void move_Global_Undo2Redo()
		{
			if ( !setUNDO[ il_taskid ] ) { return ; }
			Global_Redo[ il_taskid ].push ( Global_Undo[ il_taskid ].top() ) ;
			Global_Undo[ il_taskid ].pop() ;
		}
		void move_Global_Redo2Undo()
		{
			if ( !setUNDO[ il_taskid ] ) { return ; }
			Global_Undo[ il_taskid ].push( Global_Redo[ il_taskid ].top() ) ;
			Global_Redo[ il_taskid ].pop() ;
		}
		void flatten_idata()
		{
			idata d = il_idata.top() ;
			while ( !il_idata.empty() )
			{
				il_idata.pop() ;
			}
			while ( !il_idata_redo.empty() )
			{
				il_idata_redo.pop() ;
			}
			il_idata.push( d ) ;
		}
		void remove_redo_idata()
		{
			while ( !il_idata_redo.empty() )
			{
				il_idata_redo.pop() ;
			}
		}
		int get_idata_level() const
		{
			if ( il_idata.empty() ) { return 0 ; }
			return il_idata.top().id_level ;
		}
		int get_idata_Redo_level() const
		{
			if ( il_idata_redo.empty() ) { return 0 ; }
			return il_idata_redo.top().id_level ;
		}
		int get_Global_Undo_Size() const
		{
			return Global_Undo[ il_taskid ].size() ;
		}
		int get_Global_Redo_Size() const
		{
			return Global_Redo[ il_taskid ].size() ;
		}
		int get_Global_File_Size() const
		{
			return Global_File_level[ il_taskid ].size() ;
		}

	friend class PEDIT01 ;
	friend class PEDRXM1 ;
} ;


class cmd_range
{
	private:
		bool   c_vlab  ;
		bool   c_vcol  ;
		string c_slab  ;
		string c_elab  ;
		uint   c_sidx  ;
		uint   c_eidx  ;
		int    c_scol  ;
		int    c_ecol  ;
		bool   c_ocol  ;
		string c_rest  ;
	cmd_range()
	{
		c_vlab  = false ;
		c_vcol  = false ;
		c_slab  = ""    ;
		c_elab  = ""    ;
		c_sidx  = -1    ;
		c_eidx  = -1    ;
		c_scol  = 0     ;
		c_ecol  = 0     ;
		c_ocol  = false ;
		c_rest  = ""    ;
	}
	void cmd_range_clear()
	{
		c_slab  = ""    ;
		c_elab  = ""    ;
		c_sidx  = -1    ;
		c_eidx  = -1    ;
		c_scol  = 0     ;
		c_ecol  = 0     ;
		c_ocol  = false ;
		c_rest  = ""    ;
	}
	friend class PEDIT01 ;
} ;


class edit_find
{
	private:
		string f_string  ;
		string f_ostring ;
		string f_cstring ;
		string f_rstring ;
		bool   f_success ;
		bool   f_error   ;
		bool   f_top     ;
		bool   f_bottom  ;
		bool   f_exclude ;
		bool   f_searched;
		char   f_dir     ;
		char   f_mtch    ;
		int    f_ssize   ;
		int    f_occurs  ;
		int    f_URID    ;
		int    f_pURID   ;
		int    f_pCol    ;
		int    f_ptopLine;
		int    f_dl      ;
		int    f_lines   ;
		int    f_offset  ;
		char   f_excl    ;
		bool   f_regreq  ;
		bool   f_text    ;
		bool   f_asis    ;
		bool   f_hex     ;
		bool   f_pic     ;
		bool   f_reg     ;
		bool   f_ctext   ;
		bool   f_casis   ;
		bool   f_chex    ;
		bool   f_cpic    ;
		bool   f_creg    ;
		string f_slab    ;
		string f_elab    ;
		int    f_scol    ;
		int    f_ecol    ;
		int    f_ocol    ;
		bool   f_fset    ;
		bool   f_cset    ;
		bool   f_chngall ;
		int    f_chnincr ;
		int    f_fd_occs ;
		int    f_fd_lnes ;
		int    f_ch_occs ;
		int    f_ch_errs ;
		int    f_sk_occs ;
		int    f_sk_lnes ;
		int    f_ex_occs ;
		int    f_ex_lnes ;
	edit_find()
	{
		f_string   = ""    ;
		f_ostring  = ""    ;
		f_cstring  = ""    ;
		f_rstring  = ""    ;
		f_success  = true  ;
		f_error    = false ;
		f_top      = false ;
		f_bottom   = false ;
		f_exclude  = false ;
		f_searched = false ;
		f_dir      = 'N'   ;
		f_mtch     = 'C'   ;
		f_ssize    = 0     ;
		f_occurs   = 0     ;
		f_URID     = 0     ;
		f_pURID    = 0     ;
		f_pCol     = 0     ;
		f_ptopLine = 0     ;
		f_dl       = 0     ;
		f_lines    = 0     ;
		f_offset   = 0     ;
		f_excl     = 'A'   ;
		f_regreq   = false ;
		f_text     = false ;
		f_asis     = false ;
		f_hex      = false ;
		f_pic      = false ;
		f_reg      = false ;
		f_ctext    = false ;
		f_casis    = false ;
		f_chex     = false ;
		f_cpic     = false ;
		f_creg     = false ;
		f_slab     = ""    ;
		f_elab     = ""    ;
		f_scol     = 0     ;
		f_ecol     = 0     ;
		f_ocol     = 0     ;
		f_fset     = false ;
		f_cset     = false ;
		f_chngall  = false ;
		f_chnincr  = 0     ;
		f_fd_occs  = 0     ;
		f_fd_lnes  = 0     ;
		f_ch_occs  = 0     ;
		f_ch_errs  = 0     ;
		f_sk_occs  = 0     ;
		f_sk_lnes  = 0     ;
		f_ex_occs  = 0     ;
		f_ex_lnes  = 0     ;
	}
	void reset()
	{
		f_success  = false ;
		f_error    = false ;
		f_top      = false ;
		f_bottom   = false ;
		f_searched = false ;
		f_rstring  = "" ;
		f_occurs   = 0  ;
		f_chnincr  = 0  ;
		f_URID     = 0  ;
		f_dl       = 0  ;
		f_lines    = 0  ;
		f_offset   = 0  ;
	}
	void setrstring( const string& s )
	{
		if ( f_rstring == "" ) { f_rstring = s ; }
	}
	bool findReverse() const
	{
		return ( f_dir == 'L' || f_dir == 'P' ) ;
	}
	friend class PEDIT01 ;
} ;


class defName
{
	public:
		string name     ;
		bool   alias    ;
		bool   nop      ;
		bool   disabled ;
		bool   cmd      ;
		bool   pgm      ;
       defName()
       {
		name     = ""    ;
		alias    = false ;
		nop      = false ;
		disabled = false ;
		cmd      = false ;
		pgm      = false ;
       }
       void clear()
       {
		name     = ""    ;
		alias    = false ;
		nop      = false ;
		disabled = false ;
		cmd      = false ;
		pgm      = false ;
       }
       bool deactive() const
       {
		return ( nop || disabled ) ;
       }
       bool macro() const
       {
		return ( pgm || cmd ) ;
       }
} ;


class cmdblock
{
	public:
		string CMD   ;
		string OCMD  ;
		P_CMDS p_cmd ;
		string MSG   ;
		string cchar ;
		string udata ;
		int    RC    ;
		int    RSN   ;
		int    cwds  ;
		bool   alias    ;
		bool   seek     ;
		bool   actioned ;
		bool   macro    ;
		bool   expl     ;
		bool   lcmd     ;
		bool   keep     ;
		bool   deact    ;

	cmdblock()
	{
		CMD      = "" ;
		OCMD     = "" ;
		p_cmd    = PC_INVCMD ;
		MSG      = "" ;
		cchar    = "" ;
		udata    = "" ;
		RC       = 0  ;
		RSN      = 0  ;
		cwds     = 0  ;
		alias    = false ;
		seek     = false ;
		actioned = false ;
		macro    = false ;
		expl     = false ;
		lcmd     = false ;
		keep     = false ;
		deact    = false ;
	}
	void clear()
	{
		CMD      = "" ;
		OCMD     = "" ;
		p_cmd    = PC_INVCMD ;
		MSG      = "" ;
		cchar    = "" ;
		udata    = "" ;
		RC       = 0  ;
		RSN      = 0  ;
		cwds     = 0  ;
		alias    = false ;
		seek     = false ;
		actioned = false ;
		macro    = false ;
		expl     = false ;
		lcmd     = false ;
		keep     = false ;
		deact    = false ;
	}
	void set_cmd( const string& s, map<string,stack<defName>>& defNames )
	{
		string w ;
		map<string,stack<defName>>::iterator ita ;

		CMD = strip( s ) ;
		if ( CMD == "" ) { clear() ; return ; }

		MSG      = ""    ;
		RC       = 0     ;
		RSN      = 0     ;
		cchar    = ""    ;
		udata    = ""    ;
		macro    = true  ;
		expl     = false ;
		seek     = false ;
		lcmd     = false ;
		keep     = false ;
		alias    = false ;
		deact    = false ;
		if ( CMD.front() == '&' )
		{
			CMD.erase( 0, 1 ) ;
			if ( CMD == "" ) { return ; }
			keep  = true ;
			cchar = "&"  ;
		}
		if ( CMD.front() == '%' )
		{
			CMD.erase( 0, 1 ) ;
			if ( CMD == "" ) { return ; }
			expl   = true ;
			cchar += "%"  ;
		}
		else if ( CMD.front() == ':' && CMD.size() < 8 )
		{
			iupper( CMD.erase( 0, 1 ) ) ;
			if ( CMD == "" ) { return ; }
			lcmd  = true  ;
			macro = false ;
			cchar = ":"   ;
		}
		else
		{
			w = upper( word( CMD, 1 ) ) ;
			if ( w == "BUILTIN" )
			{
				idelword( CMD, 1, 1 ) ;
				if ( trim( CMD ) == "" )
				{
					set_msg( "PEDT015F" ) ;
					CMD  = "BUILTIN" ;
					keep = true      ;
					return ;
				}
				if ( PrimCMDS.count( w ) > 0 ) { w = PrimCMDS[ w ].truename ; }
				w = upper( word( CMD, 1 ) ) ;
			}
			else
			{
				if ( PrimCMDS.count( w ) > 0 ) { w = PrimCMDS[ w ].truename ; }
				ita = defNames.find( w ) ;
				if ( ita != defNames.end() )
				{
					if ( ita->second.top().deactive() )
					{
						set_msg( "PEDT015R", 8 ) ;
						deact = true ;
						return ;
					}
					else if ( ita->second.top().macro() )
					{
						expl = true ;
					}
					if ( ita->second.top().alias )
					{
						w   = ita->second.top().name ;
						if ( PrimCMDS.count( w ) > 0 ) { w = PrimCMDS[ w ].truename ; }
						ita = defNames.find( w ) ;
						if ( ita != defNames.end() && ita->second.top().deactive() )
						{
							set_msg( "PEDT015R", 8 ) ;
							deact = true ;
							return ;
						}
						OCMD  = CMD ;
						CMD   = w + " " + idelword( CMD, 1, 1 ) ;
						trim( CMD )  ;
						alias = true ;
					}
				}
			}
			if ( !expl )
			{
				if ( w == "SEEK" )
				{
					p_cmd = PC_FIND ;
					seek  = true  ;
					macro = false ;
				}
				else if ( PrimCMDS.count( w ) > 0 )
				{
					p_cmd = PrimCMDS[ w ].p_cmd ;
					macro = false ;
				}
			}
		}
		cwds     = words( CMD ) ;
		actioned = false ;
	}
	string condget_cmd() const
	{
		if ( error() || keep ) { return alias ? cchar+OCMD : cchar+CMD ; }
		return "" ;
	}
	int get_cmd_words() const
	{
		return cwds ;
	}
	const string& get_cmd() const
	{
		return CMD ;
	}
	bool cmd_not( const string& s ) const
	{
		return ( CMD != s ) ;
	}
	int isMacro() const
	{
		return macro ;
	}
	void setActioned()
	{
		actioned = true ;
	}
	bool isActioned() const
	{
		return actioned ;
	}
	int isSeek() const
	{
		return seek ;
	}
	int isLine_cmd() const
	{
		return lcmd ;
	}
	void reset()
	{
		CMD      = ""    ;
		cchar    = ""    ;
		udata    = ""    ;
		p_cmd    = PC_INVCMD ;
		actioned = false ;
		seek     = false ;
		macro    = false ;
		expl     = false ;
		lcmd     = false ;
		keep     = false ;
	}
	void cond_reset()
	{
		if ( !keep ) { reset() ; }
	}
	void clear_msg()
	{
		MSG = "" ;
		RC  = 0  ;
		RSN = 0  ;
	}
	void set_msg( const string& m, int rc =12 )
	{
		MSG = m  ;
		RC  = rc ;
	}
	void setRC( int rc )
	{
		RC = rc ;
	}
	const string& get_msg() const
	{
		return MSG ;
	}
	bool msgset() const
	{
		return ( MSG != "" ) ;
	}
	bool cmdset() const
	{
		return ( CMD != "" ) ;
	}
	void set_userdata( const string& s )
	{
		udata = s ;
	}
	const string& get_userdata() const
	{
		return udata ;
	}
	bool error() const
	{
		return ( RC > 8 ) ;
	}
	bool info() const
	{
		return ( RC < 9 ) ;
	}
	bool deactive() const
	{
		return deact ;
	}
	friend class PEDIT01 ;
	friend class PEDRXM1 ;
} ;


class miblock
{
	public:
		string emacro    ;
		string mfile     ;
		string rxpath1   ;
		string rxpath2   ;
		pApplication * editAppl ;
		pApplication * macAppl  ;
		string sttment   ;
		M_CMDS m_cmd     ;
		string keyword   ;
		string kphrase   ;
		string keyopts   ;
		string value     ;
		string parms     ;
		string var1      ;
		string var2      ;
		string msgid     ;
		string val1      ;
		bool   mfound    ;
		bool   macro     ;
		bool   imacro    ;
		bool   process   ;
		bool   processed ;
		bool   setcursor ;
		bool   fatal     ;
		bool   assign    ;
		bool   query     ;
		bool   scan      ;
		bool   runmacro  ;
		bool   eended    ;
		int    sttwds    ;
		int    etaskid   ;
		int    nestlvl   ;
		int    nvars     ;
		int    nkeyopts  ;
		int    nvalopts  ;
		int    RC        ;
		int    RSN       ;
		int    exitRC    ;

	miblock()
	{
		emacro    = ""    ;
		mfile     = ""    ;
		rxpath1   = ""    ;
		rxpath2   = ""    ;
		editAppl  = NULL  ;
		macAppl   = NULL  ;
		sttment   = ""    ;
		m_cmd     = EM_INVCMD ;
		kphrase   = ""    ;
		keyword   = ""    ;
		keyopts   = ""    ;
		value     = ""    ;
		parms     = ""    ;
		var1      = ""    ;
		var2      = ""    ;
		msgid     = ""    ;
		val1      = ""    ;
		mfound    = false ;
		macro     = false ;
		imacro    = false ;
		process   = true  ;
		processed = false ;
		setcursor = false ;
		fatal     = false ;
		assign    = false ;
		query     = false ;
		scan      = true  ;
		runmacro  = false ;
		eended    = false ;
		etaskid   = 0     ;
		sttwds    = 0     ;
		nestlvl   = 0     ;
		nvars     = 0     ;
		nkeyopts  = 0     ;
		nvalopts  = 0     ;
		RC        = 0     ;
		RSN       = 0     ;
		exitRC    = 0     ;
	}
	void clear()
	{
		emacro    = ""    ;
		mfile     = ""    ;
		editAppl  = NULL  ;
		macAppl   = NULL  ;
		sttment   = ""    ;
		m_cmd     = EM_INVCMD ;
		kphrase   = ""    ;
		keyword   = ""    ;
		keyopts   = ""    ;
		value     = ""    ;
		parms     = ""    ;
		var1      = ""    ;
		var2      = ""    ;
		msgid     = ""    ;
		val1      = ""    ;
		mfound    = false ;
		macro     = false ;
		imacro    = false ;
		process   = true  ;
		processed = false ;
		setcursor = false ;
		fatal     = false ;
		assign    = false ;
		query     = false ;
		scan      = true  ;
		runmacro  = false ;
		etaskid   = 0     ;
		sttwds    = 0     ;
		nestlvl   = 0     ;
		nvars     = 0     ;
		nkeyopts  = 0     ;
		nvalopts  = 0     ;
		RC        = 0     ;
		RSN       = 0     ;
		exitRC    = 0     ;
	}
	void reset()
	{
		m_cmd     = EM_INVCMD ;
		sttment   = ""    ;
		kphrase   = ""    ;
		keyword   = ""    ;
		keyopts   = ""    ;
		value     = ""    ;
		var1      = ""    ;
		var2      = ""    ;
		msgid     = ""    ;
		val1      = ""    ;
		assign    = false ;
		query     = false ;
		runmacro  = false ;
		sttwds    = 0     ;
		nvars     = 0     ;
		nkeyopts  = 0     ;
		nvalopts  = 0     ;
		RC        = 0     ;
		RSN       = 0     ;
	}
	void seterror( const string& e1, int i1=12, int i2 =0 )
	{
		msgid = e1 ;
		RC    = i1 ;
		RSN   = i2 ;
		fatal = ( RC >= 12 ) ;
	}
	void seterror( const string& e1, const string& e2, int i1 =12, int i2 =0 )
	{
		msgid = e1 ;
		val1  = e2 ;
		RC    = i1 ;
		RSN   = i2 ;
		fatal = ( RC >= 12 ) ;
	}
	void seterror( cmdblock& cmd )
	{
		msgid = cmd.MSG ;
		RC    = cmd.RC  ;
		RSN   = cmd.RSN ;
		fatal = ( RC >= 12 ) ;
	}
	void setExitRC( const string& s )
	{
		if ( datatype( s, 'W' ) )
		{
			exitRC = ds2d( s ) ;
		}
		else
		{
			seterror( "PEDM012L", 20 ) ;
			exitRC = 28 ;
		}
	}
	void setExitRC( int i )
	{
		exitRC = i ;
	}
	int getExitRC() const
	{
		return exitRC ;
	}
	void setRC( int i )
	{
		RC = i ;
		if ( RC >= 12 ) { fatal = true ; }
		else
		{
			fatal = false ;
			msgid = "" ;
		}
	}
	bool msgset() const
	{
		return ( msgid != "" ) ;
	}
	void parseMACRO()
	{
		int i  ;
		int ws ;

		size_t p1 ;

		string t ;

		t = upper( sttment )    ;
		trim( t.erase( 0, 6 ) ) ;
		if ( t.front() == '(' )
		{
			p1 = t.find( ')' ) ;
			if ( p1 == string::npos )
			{
				seterror( "PEDM011C" ) ;
				return ;
			}
			keyopts = t.substr( 1, p1-1 ) ;
			trim( t.erase( 0, p1+2 ) ) ;
			replace( keyopts.begin(), keyopts.end(), ',', ' ' ) ;
			iupper( keyopts ) ;
			for( ws = words( keyopts ), i = 1 ; i <= ws ; i++ )
			{
				var1 = word( keyopts, i ) ;
				if ( !isvalidName( var1 ) )
				{
					seterror( "PEDM011D", var1, 20 ) ;
					return ;
				}
			}
		}
		if ( t == "NOPROCESS" )
		{
			process = false ;
		}
		else if ( t != "" && t != "PROCESS" )
		{
			seterror( "PEDM011P" ) ;
			return                 ;
		}
		macro = true ;
		return       ;
	}
	void parseStatement( const string& s, map<string,stack<defName>>& defNames )
	{
		int  i     ;
		int  miss  ;
		char qt    ;

		size_t p1  ;

		string var ;
		string w   ;
		string t   ;

		bool builtin ;
		bool quote   ;
		bool isvar   ;

		map<string,mcmd_format>::iterator it     ;
		map<string,stack<defName>>::iterator ita ;
		string::iterator its ;

		if ( eended )
		{
			seterror( "PEDM013D" ) ;
			eended = false ;
			return ;
		}
		reset() ;

		sttment = s ;
		trim( sttment ) ;
		t = sttment ;

		for ( quote = false, its = t.begin() ; its != t.end() ; its++ )
		{
			if ( !quote && ((*its) == '"' || (*its) == '\'' ) ) { quote = true ; qt = (*its) ; continue ; }
			if (  quote &&  (*its) == qt ) { quote = false ; continue ; }
			if ( quote ) { continue  ; }
			(*its) = toupper( *its ) ;
		}

		builtin = false ;
		w       = word( t, 1 ) ;

		if ( w == "MACRO" )
		{
			if ( macro )
			{
				seterror( "PEDM011B" ) ;
			}
			else
			{
				m_cmd = EM_MACRO ;
				parseMACRO()     ;
			}
			return ;
		}
		if ( !macro )
		{
			seterror( "PEDM011E" ) ;
			return                 ;
		}
		if ( sttment == "" )
		{
			seterror( "PEDM012P" ) ;
			return                 ;
		}
		if ( w == "BUILTIN" )
		{
			idelword( sttment, 1, 1 ) ;
			idelword( t, 1, 1 )       ;
			builtin = true ;
		}
		sttwds = words( sttment ) ;
		if ( t.front() == '(' )
		{
			p1 = t.find( '=' ) ;
			if ( p1 == string::npos )
			{
				seterror( "PEDM011K" ) ;
				return ;
			}
			assign  = true ;
			value   = t.substr( 0, p1 ) ;
			kphrase = t.substr( p1+1  ) ;
			query   = true ;
			keyword = word( kphrase, 1 ) ;
			if ( EMServ.count( keyword ) == 0 )
			{
				seterror( "PEDM011N" ) ;
				return ;
			}
		}
		else
		{
			p1      = t.find( '=' ) ;
			keyword = word( t, 1 )  ;
			if ( p1 != string::npos ) { assign  = true ; }
			if ( PrimCMDS.count( keyword ) > 0 ) { keyword = PrimCMDS[ keyword ].truename ; }
			if ( assign && !builtin )
			{
				ita = defNames.find( keyword ) ;
				if ( ita != defNames.end() )
				{
					if ( ita->second.top().deactive() )
					{
						seterror( "PEDT015S" ) ;
						return ;
					}
					keyword = ita->second.top().name ;
					sttment = keyword + " " + idelword( sttment, 1, 1 ) ;
				}
			}
			if ( EMServ.count( keyword ) == 0 )
			{
				if ( EMCmds.count( keyword ) == 0 )
				{
					if ( builtin )
					{
						seterror( "PEDM012J" ) ;
					}
					else
					{
						runmacro = true ;
					}
					return ;
				}
				m_cmd = EMCmds[ keyword ] ;
				if ( m_cmd == EM_PROCESS && processed && !imacro )
				{
					seterror( "PEDM012K" ) ;
				}
				return ;
			}
			if ( !assign )
			{
				if ( EMServ.count( keyword ) > 0 && EMCmds.count( keyword ) == 0 )
				{
					seterror( "PEDM012X" ) ;
					return ;
				}
				keyopts = subword( t, 2 ) ;
				m_cmd   = EMCmds[ keyword ] ;
				if ( m_cmd == EM_PROCESS && processed && !imacro )
				{
					seterror( "PEDM012K" ) ;
				}
				return ;
			}
			kphrase = t.substr( 0, p1 ) ;
			value   = t.substr( p1+1  ) ;
		}
		keyopts  = subword( kphrase, 2 ) ;
		nkeyopts = words( keyopts ) ;
		isvar    = false ;
		m_cmd    = EMServ[ keyword ].m_cmd ;
		trim( value )    ;
		if ( !query && (m_cmd == EM_LINE || m_cmd == EM_LINE_BEFORE || m_cmd == EM_LINE_AFTER ) ) {}
		else if ( value.front() == '(' )
		{
			isvar = true ;
			p1 = value.find( ')' ) ;
			if ( p1 == string::npos )
			{
				seterror( "PEDM011F" ) ;
				return ;
			}
			if ( value.back() != ')' )
			{
				seterror( "PEDM011H", 20 ) ;
				return ;
			}
			value = value.substr( 1, value.size() - 2 ) ;
			if ( trim( value ) == "" )
			{
				seterror( "PEDM011G" ) ;
				return ;
			}
		}

		miss = 0 ;
		if ( isvar )
		{
			if ( value.front() == ',' ) { miss = 1 ; }
			replace( value.begin(), value.end(), ',', ' ' ) ;
			iupper( trim( value ) ) ;
			for ( nvars = words( value ), i = 1 ; i <= nvars ; i++ )
			{
				var = word( value, i ) ;
				if ( !isvalidName( var ) )
				{
					seterror( "PEDM011D", var, 20 ) ;
					return ;
				}
				if      ( i+miss == 1 ) { var1 = var ; }
				else if ( i+miss == 2 ) { var2 = var ; }
				else    { seterror( "PEDM011H", 20 ) ; fatal = true ; return ;  }
			}
		}

		it = EMServ.find( keyword ) ;
		if ( query )
		{
			if ( (it->second.qnvars1 != -1 ) )
			{
				if ( (nvars + miss) < EMServ[ keyword ].qnvars1 )
				{
					seterror( "PEDM012B" ) ;
					return ;
				}
			}
			if ( (it->second.qnvars2 != -1 ) )
			{
				if ( (nvars + miss) > EMServ[ keyword ].qnvars2 )
				{
					seterror( "PEDM012C", 20 ) ;
					return ;
				}
			}
			if ( it->second.qnkeyopts1 != -1 )
			{
				if ( nkeyopts < it->second.qnkeyopts1 )
				{
					seterror( "PEDM011X", 20 ) ;
					return ;
				}
			}
			if ( it->second.qnkeyopts2 != -1 )
			{
				if ( nkeyopts > it->second.qnkeyopts2 )
				{
					seterror( "PEDM011Y", 20 ) ;
					return ;
				}
			}
		}
		else
		{
			nvalopts = words( value ) ;
			if ( it->second.ankeyopts1 != -1 )
			{
				if ( nkeyopts < it->second.ankeyopts1 )
				{
					seterror( "PEDM011X", 20 ) ;
					return ;
				}
			}
			if ( it->second.ankeyopts2 != -1 )
			{
				if ( nkeyopts > it->second.ankeyopts2 )
				{
					seterror( "PEDM011Y", 20 ) ;
					return ;
				}
			}
			if ( it->second.anvalopts1 != -1 )
			{
				if ( nvalopts < it->second.anvalopts1 )
				{
					seterror( "PEDM011Z", 20 ) ;
					return ;
				}
			}
			if ( it->second.anvalopts2 != -1 )
			{
				if ( nvalopts > it->second.anvalopts2 )
				{
					seterror( "PEDM012A", 20 ) ;
					return ;
				}
			}
		}
	}
	void setMacro( const string& s )
	{
		emacro = s ;
	}
	bool getMacroFileName( const string& paths )
	{
		int i ;
		int j ;
		setRC( 0 ) ;
		for ( j = getpaths( paths ), i = 1 ; i <= j ; i++ )
		{
			mfile = getpath( paths, i ) + emacro ;
			if ( !exists( mfile ) ) { continue ; }
			if ( is_regular_file( mfile ) ) { mfound = true ; return true ; }
			setRC( 28 )  ;
			return false ;
		}
		setRC( 8 )   ;
		return false ;
	}
	bool scanOn() const
	{
		return scan ;
	}
	bool editEnded() const
	{
		return eended ;
	}
	int get_sttment_words() const
	{
		return sttwds ;
	}
	friend class PEDIT01 ;
	friend class PEDRXM1 ;
} ;


class PEDIT01 : public pApplication
{
	public:
		PEDIT01()  ;

		void application()   ;

		void actionService() ;
		void querySetting()  ;
		void isredit( const string& ) ;

		miblock miBlock ;
		cmdblock pcmd   ;

		map<string,stack<defName>> defNames ;

	private:
		static set<string>EditList       ;
		static edit_find Global_efind_parms ;

		void showEditEntry()      ;
		void showEditRecovery()   ;
		void Edit()               ;
		void getEditProfile( const string& )  ;
		void delEditProfile( const string& )  ;
		void saveEditProfile( const string& ) ;
		void createEditProfile( const string&, const string& ) ;
		void resetEditProfile()   ;
		void cleanup_custom()     ;
		void initialise()         ;
		bool termOK()             ;
		void readFile()           ;
		bool saveFile()           ;
		bool showConfirmCancel()  ;
		void fill_dynamic_area()  ;
		void fill_hilight_shadow();
		void clr_hilight_shadow() ;
		void protNonDisplayChars();
		void releaseDynamicStorage() ;
		void getZAREAchanges()    ;
		void updateData()         ;
		string setFoundString()   ;
		string getColumnLine( int =0 ) ;

		void hiliteFindPhrase()   ;
		void actionFind()         ;
		void actionChange()       ;

		bool checkLineCommands()  ;
		void actionPrimCommand1() ;
		void actionPrimCommand2() ;
		void actionLineCommands() ;
		void actionLineCommand( vector<lcmd>::iterator ) ;

		void run_macro( bool =false ) ;

		void actionZVERB()        ;
		void setLineLabels()      ;

		bool actionUNDO()         ;
		bool actionREDO()         ;
		void removeRecoveryData() ;

		uint getLine( int )       ;
		uint getFirstEX( uint )   ;
		int  getLastEX( vector<iline *>::iterator ) ;
		uint getLastEX( uint )    ;
		int  getExBlockSize( uint )   ;
		int  getDataBlockSize( uint ) ;
		bool URIDOnScreen( int, int ) ;
		void moveTopline( int )  ;
		int  getLastURID( vector<iline *>::iterator, int ) ;

		int  getFileLine( uint ) ;
		uint getDataLine( int )  ;

		vector<iline * >::iterator getValidDataLineNext( vector<iline * >::iterator ) ;
		uint getValidFileLinePrev( uint ) ;
		uint getValidFileLineNext( uint ) ;

		uint getNextDataLine( uint ) ;
		vector<iline * >::iterator getNextDataLine( vector<iline * >::iterator ) ;
		uint getNextFileLine( uint ) ;
		vector<iline * >::iterator getNextFileLine( vector<iline * >::iterator ) ;

		uint getPrevDataLine( uint ) ;
		uint getPrevFileLine( uint ) ;
		vector<iline * >::iterator getPrevFileLine( vector<iline * >::iterator ) ;
		void getLineData( vector<iline *>::iterator ) ;
		string mergeLine( const string&, vector<iline *>::iterator ) ;

		void cleanupData()       ;
		void cleanupRedoStacks() ;
		void updateProfLines( vector<string>& ) ;
		void buildProfLines( vector<string>& )  ;
		void removeProfLines( )  ;
		void removeSpecialLines( vector<iline * >::iterator, vector<iline * >::iterator ) ;
		void processNewInserts() ;

		string removeTabs( string )  ;
		bool   getVariables( int, string&, string& ) ;

		void copyFileData( vector<string> &, int, int  ) ;

		string overlay1( string, string, bool& ) ;
		string overlay2( string, string )        ;
		string templat( string, string )         ;
		bool formLineCmd( const string&, string&, int& ) ;

		bool copyToClipboard( vector<ipline>& vip ) ;
		void getClipboard( vector<ipline>& vip ) ;
		void clearClipboard( string ) ;
		void manageClipboard() ;
		void manageClipboard_create( vector<string>& ) ;
		void manageClipboard_descr( const string&, const string& )  ;
		void manageClipboard_browse( const string& ) ;
		void manageClipboard_toggle( int, const string& ) ;
		void manageClipboard_edit( const string&, const string& )   ;
		void manageClipboard_rename( const string&, const string& ) ;
		void manageClipboard_delete( const string& ) ;
		void manageClipboard_islocked( const string& ) ;

		void createFile( uint, uint ) ;

		void clearCursor() ;
		void fixCursor()   ;
		void storeCursor(  int, int=0 ) ;
		void placeCursor(  int, int, int=0 ) ;
		void placeCursor( uint, int, int=0 ) ;
		void positionCursor()     ;
		void moveColumn( int =0 ) ;
		void moveCursorEnter()    ;

		vector<iline * >::iterator getLineItr( int )  ;
		vector<iline * >::iterator getLineItr( uint ) ;
		vector<iline * >::iterator getLineItr( int, vector<iline *>::iterator ) ;

		bool setFindChangeExcl( char ) ;
		void setFoundMsg()             ;
		void setChangedMsg()           ;
		void setExcludedMsg()          ;
		void setNotFoundMsg()          ;
		bool setCommandRange( string, cmd_range& ) ;
		int  getNextSpecial( int, int, char, char ) ;
		bool getLabelItr( const string&, vector<iline * >::iterator &, uint& ) ;
		int  getLabelLine( const string& )  ;
		int  getLabelIndex( const string& ) ;
		bool checkLabel1( const string&, int =0 ) ;
		bool checkLabel2( const string&, int =0 ) ;

		bool getTabLocation( size_t& ) ;
		void copyPrefix( ipline &, iline *& ) ;
		void copyPrefix( iline * &,ipline&, bool =false ) ;
		void addSpecial( char, int, vector<string>& ) ;
		void addSpecial( char, int, const string& ) ;
		void addSpecial( char, vector<iline * >::iterator, const string& ) ;

		string getMaskLine() ;
		string rshiftCols( int, const string * ) ;
		string lshiftCols( int, const string * ) ;
		bool   rshiftData( int, const string *, string& ) ;
		bool   lshiftData( int, const string *, string& ) ;
		bool   textSplitData( const string&, string&, string& ) ;

		void   compareFiles( const string& ) ;
		string createTempName()  ;
		string determineLang()   ;

		uint topLine             ;
		uint ptopLine            ;
		uint maxCol              ;
		int  startCol            ;
		int  mRow                ;
		int  mCol                ;
		int  aRow                ;
		int  aCol                ;
		int  aURID               ;
		bool aLCMD               ;
		bool macroRunning        ;
		int  Level               ;
		int  saveLevel           ;
		int  nestLevel           ;
		int  XTabz               ;

		bool tabsOnRead          ;
		bool abendRecovery       ;
		bool abendComplete       ;

		bool cursorPlaceHome     ;
		bool cursorFixed         ;
		int  cursorPlaceUsing    ;
		int  cursorPlaceURID     ;
		int  cursorPlaceURIDO    ;
		int  cursorPlaceRow      ;
		int  cursorPlaceType     ;
		int  cursorPlaceOff      ;
		int  cursorPlaceOffO     ;

		bool   profASave         ;
		bool   profSaveP         ;
		bool   profNullA         ;
		bool   profNulls         ;
		bool   profLock          ;
		bool   profCaps          ;
		bool   profHex           ;
		bool   profTabs          ;
		bool   profATabs         ;
		bool   profXTabs         ;
		int    profXTabz         ;
		bool   profRecover       ;
		bool   profHilight       ;
		bool   profIfLogic       ;
		bool   profDoLogic       ;
		bool   profLogic         ;
		bool   profParen         ;
		bool   profCutReplace    ;
		bool   profPasteKeep     ;
		bool   profPosFcx        ;
		string profLang          ;
		string profIMAC          ;
		bool   profVert          ;
		bool   profFindPhrase    ;
		bool   profCsrPhrase     ;
		bool   profUndoKeep      ;

		string detLang           ;
		string creFile           ;

		string zcmd              ;
		string zverb             ;
		string zapplid           ;
		string zuser             ;
		string zscreen           ;
		string zhome             ;

		bool optNoConvTabs       ;
		bool optPreserve         ;
		bool optConfCancel       ;
		bool optFindPhrase       ;
		string optProfile        ;
		string optMacro          ;

		bool termEdit            ;
		bool stripST             ;
		bool convTabs            ;
		bool convSpaces          ;
		bool creActive           ;
		bool cutActive           ;
		bool cutReplace          ;
		bool pasteActive         ;
		bool copyActive          ;
		bool pasteKeep           ;
		bool hideExcl            ;

		int  LeftBnd             ;
		int  RightBnd            ;

		string maskLine          ;
		bool   colsOn            ;
		string tabsLine          ;
		char   tabsChar          ;
		string recoverLoc        ;

		vector<iline *> data     ;
		map<int, ipos> s2data    ;
		map<int, bool> sChanged  ;
		map<int, bool> sTouched  ;
		map<int, int>lchar       ;
		vector<lcmd> lcmds       ;

		edit_find fcx_parms ;
		hilight hlight      ;

		bool rebuildZAREA  ;
		bool rebuildShadow ;
		bool fileChanged   ;

		string curfld  ;
		int    curpos  ;

		string zfile   ;
		string zcol1   ;
		string zcol2   ;
		string zarea   ;
		string zshadow ;
		string zareat  ;
		int    zareaw  ;
		int    zaread  ;
		uint   zdataw  ;
		uint   zasize  ;
		string carea   ;
		string cshadow ;
		string xarea   ;

		string zscrolla ;
		int    zscrolln ;
		string zcurfld  ;
		int    zcurpos  ;

		string zedprof  ;
		string zedproft ;

		string zedptype ;
		string zedpflag ;
		string zedpmask ;
		string zedpbndl ;
		string zedpbndr ;
		string zedptabc ;
		string zedptabs ;
		string zedptabz ;
		string zedprclc ;
		string zedphllg ;
		string zedpimac ;
		string zedpflg2 ;
		string zedpflg3 ;

		string eeimac   ;
		string eeprof   ;
		string eetabss  ;
		string eeprsps  ;
		string eeccan   ;

		string fileType  ;
		string clipBoard ;
		string clipTable ;

		string sdg  ;
		string sdy  ;
		string sdyh ;
		string sdw  ;
		string sdr  ;
		string sdrh ;
		string sdb  ;
		string sdmk ;

		string div  ;

		string slg  ;
		string sly  ;
		string slyh ;
		string slw  ;
		string slr  ;
		string slrh ;
		string slb  ;

		string type ;
		string str  ;
		string occ  ;
		string lines;

		map<char, string>typList    = { { 'C', "CHARS"  },
						{ 'P', "PREFIX" },
						{ 'S', "SUFFIX" },
						{ 'W', "WORD"   } } ;

		map<bool, string>OnOff      = { { true,  "ON"   },
						{ false, "OFF"  } } ;

		map<bool, char>ZeroOne      = { { true,  '1'    },
						{ false, '0'    } } ;


						      //
						      //               +----------Minimum number of parameters
						      //               |   +------Maximum number of parameters
						      //               |   |
						      //               |   |      value of -1 means don't check
						      //               V   V
		map<P_CMDS,pcmd_format> PCMDform = { { PC_AUTOSAVE, {  0,  2 } },
						     { PC_BOUNDS,   {  0,  2 } },
						     { PC_BROWSE,   { -1, -1 } },
						     { PC_CANCEL,   {  0,  0 } },
						     { PC_CHANGE,   { -1, -1 } },
						     { PC_CAPS,     {  0,  2 } },
						     { PC_COLUMN,   {  0,  1 } },
						     { PC_COMPARE,  { -1, -1 } },
						     { PC_COPY,     {  0,  1 } },
						     { PC_CREATE,   { -1, -1 } },
						     { PC_CUT,      { -1, -1 } },
						     { PC_DEFINE,   {  2,  3 } },
						     { PC_DELETE,   { -1, -1 } },
						     { PC_EDIT,     { -1, -1 } },
						     { PC_EDITSET,  {  0,  0 } },
						     { PC_EXCLUDE,  { -1, -1 } },
						     { PC_FIND,     { -1, -1 } },
						     { PC_FLIP,     { -1, -1 } },
						     { PC_HEX,      {  0,  2 } },
						     { PC_HIDE,     {  1,  1 } },
						     { PC_HILIGHT,  { -1, -1 } },
						     { PC_IMACRO,   {  1,  1 } },
						     { PC_LOCATE,   { -1, -1 } },
						     { PC_NULLS,    {  0,  2 } },
						     { PC_PASTE,    { -1, -1 } },
						     { PC_PROFILE,  { -1, -1 } },
						     { PC_PRESERVE, {  0,  1 } },
						     { PC_RECOVERY, { -1, -1 } },
						     { PC_REDO,     {  0,  1 } },
						     { PC_REPLACE,  { -1, -1 } },
						     { PC_RESET,    { -1, -1 } },
						     { PC_SAVE,     {  0,  0 } },
						     { PC_SETUNDO,  {  1,  1 } },
						     { PC_SORT,     { -1, -1 } },
						     { PC_TABS,     { -1, -1 } },
						     { PC_UNDO,     {  0,  1 } },
						     { PC_XTABS,    {  0,  1 } } } ;


		map<string,L_CMDS> LineCMDS = { { "A",        LC_A        },
						{ "AK",       LC_AK       },
						{ "B",        LC_B        },
						{ "BK",       LC_BK       },
						{ "BNDS",     LC_BOUNDS   },
						{ "C",        LC_C        },
						{ "CC",       LC_CC       },
						{ "COLS",     LC_COLS     },
						{ "D",        LC_D        },
						{ "DD",       LC_DD       },
						{ "F",        LC_F        },
						{ "HX",       LC_HX       },
						{ "HXX",      LC_HXX      },
						{ "I",        LC_I        },
						{ "L",        LC_L        },
						{ "LC",       LC_LC       },
						{ "LCC",      LC_LCC      },
						{ "M",        LC_M        },
						{ "MM",       LC_MM       },
						{ "MASK",     LC_MASK     },
						{ "MD",       LC_MD       },
						{ "MDD",      LC_MDD      },
						{ "MN",       LC_MN       },
						{ "MNN",      LC_MNN      },
						{ "O",        LC_O        },
						{ "OK",       LC_OK       },
						{ "OO",       LC_OO       },
						{ "OOK",      LC_OOK      },
						{ "R",        LC_R        },
						{ "RR",       LC_RR       },
						{ "S",        LC_S        },
						{ "SI",       LC_SI       },
						{ "TABS",     LC_TABS     },
						{ "TJ",       LC_TJ       },
						{ "TJJ",      LC_TJJ      },
						{ "TR",       LC_TR       },
						{ "TRR",      LC_TRR      },
						{ "TS",       LC_TS       },
						{ "T",        LC_T        },
						{ "TT",       LC_TT       },
						{ "TX",       LC_TX       },
						{ "TXX",      LC_TXX      },
						{ "UC",       LC_UC       },
						{ "UCC",      LC_UCC      },
						{ "W",        LC_W        },
						{ "WW",       LC_WW       },
						{ "X",        LC_X        },
						{ "XX",       LC_XX       },
						{ "XI",       LC_XI       },
						{ ")",        LC_SRC      },
						{ "))",       LC_SRCC     },
						{ "(",        LC_SLC      },
						{ "((",       LC_SLCC     },
						{ ">",        LC_SRD      },
						{ ">>",       LC_SRDD     },
						{ "<",        LC_SLD      },
						{ "<<",       LC_SLDD     },
						{ "]",        LC_SRTC     },
						{ "]]",       LC_SRTCC    },
						{ "[",        LC_SLTC     },
						{ "[[",       LC_SLTCC    } } ;

		map<string,string> aliasLCMDS = { { "COL",    "COLS" },
						  { "BND",    "BNDS" },
						  { "BOU",    "BNDS" },
						  { "BOUND",  "BNDS" },
						  { "BOUNDS", "BNDS" },
						  { "LLC",    "LCC"  },
						  { "LCLC",   "LCC"  },
						  { "HHX",    "HXX"  },
						  { "HXHX",   "HXX"  },
						  { "MMD",    "MDD"  },
						  { "MDMD",   "MDD"  },
						  { "MMN",    "MNN"  },
						  { "MNMN",   "MNN"  },
						  { "TAB",    "TABS" },
						  { "TTJ",    "TJJ"  },
						  { "TJTJ",   "TJJ"  },
						  { "TTR",    "TRR"  },
						  { "TRTR",   "TRR"  },
						  { "TTX",    "TXX"  },
						  { "TXTX",   "TXX"  },
						  { "UUC",    "UCC"  },
						  { "UCUC",   "UCC"  } } ;

		map<string,string> aliasNames = { { "CHAR",     "CHARS"     },
						  { "CHG",      "CHA"       },
						  { "CHANGE",   "CHA"       },
						  { "CMDS",     "CMD"       },
						  { "COM",      "CMD"       },
						  { "COMMAND",  "CMD"       },
						  { "COMMANDS", "CMD"       },
						  { "CURSOR",   "CUR"       },
						  { "DIS",      "DISABLED"  },
						  { "DISAB",    "DISABLED"  },
						  { "DISABLE",  "DISABLED"  },
						  { "DISP",     "DISPLAY"   },
						  { "DISPL",    "DISPLAY"   },
						  { "DO",       "DOLOGIC"   },
						  { "ERROR",    "ERR"       },
						  { "ERRORS",   "ERR"       },
						  { "EX",       "X"         },
						  { "EXC",      "X"         },
						  { "EXCLUDE",  "X"         },
						  { "EXCLUDED", "X"         },
						  { "IF",       "IFLOGIC"   },
						  { "INFOLINE", "INFO"      },
						  { "HIDE",     "H"         },
						  { "LABEL",    "LAB"       },
						  { "LABELS",   "LAB"       },
						  { "MRK",      "T"         },
						  { "MARK",     "T"         },
						  { "MARKED",   "T"         },
						  { "MSGLINE",  "MSG"       },
						  { "NOTELINE", "NOTE"      },
						  { "PRE",      "PREFIX"    },
						  { "REC",      "RECOVER"   },
						  { "RECOVERY", "RECOVER"   },
						  { "SPECIAL",  "SPE"       },
						  { "STD",      "STANDARD"  },
						  { "STG",      "STORAGE"   },
						  { "STO",      "STORAGE"   },
						  { "STOR",     "STORAGE"   },
						  { "STORE",    "STORAGE"   },
						  { "SUF",      "SUFFIX"    },
						  { "VERT",     "VERTICAL"  } } ;

		map<string,string> abokLCMDS  = { { "AK",  "A"  },
						  { "BK",  "B"  },
						  { "OK",  "O"  },
						  { "OOK", "OO" } } ;

		set<string> BLKcmds  = { { "CC"   },
					 { "DD"   },
					 { "MM"   },
					 { "HXX"  },
					 { "LCC"  },
					 { "MDD"  },
					 { "MNN"  },
					 { "OO"   },
					 { "OOK"  },
					 { "RR"   },
					 { "TJJ"  },
					 { "TRR"  },
					 { "TT"   },
					 { "TXX"  },
					 { "XX"   },
					 { "WW"   },
					 { "UCC"  },
					 { "(("   },
					 { "))"   },
					 { "<<"   },
					 { ">>"   },
					 { "[["   },
					 { "]]"   } } ;

		set<string> SPLcmds  = { { "A"    },
					 { "AK"   },
					 { "B"    },
					 { "BK"   },
					 { "C"    },
					 { "CC"   },
					 { "COLS" },
					 { "D"    },
					 { "DD"   },
					 { "F"    },
					 { "I"    },
					 { "L"    },
					 { "M"    },
					 { "MM"   },
					 { "MD"   },
					 { "MDD"  },
					 { "MN"   },
					 { "MNN"  },
					 { "R"    },
					 { "RR"   },
					 { "S"    },
					 { "SI"   },
					 { "TX"   },
					 { "TXX"  },
					 { "X"    },
					 { "XX"   },
					 { "XI"   } } ;

		set<string> TODcmds  = { { "A"    },
					 { "I"    } } ;

		set<string> BODcmds  = { { "B"    },
					 { "COLS" },
					 { "BNDS" },
					 { "MASK" },
					 { "TABS" } } ;

		set<string> ABOKReq  = { { "C"    },
					 { "CC"   },
					 { "M"    },
					 { "MM"   } } ;

		set<string> Chkdist  = { { "C"    },
					 { "D"    },
					 { "M"    },
					 { "HX"   },
					 { "LC"   },
					 { "MD"   },
					 { "MN"   },
					 { "O"    },
					 { "OK"   },
					 { "TJ"   },
					 { "TR"   },
					 { "TX"   },
					 { "UC"   },
					 { "W"    },
					 { "X"    } } ;

		set<string> ABOWList = { { "A"    },
					 { "AK"   },
					 { "B"    },
					 { "BK"   },
					 { "O"    },
					 { "OK"   },
					 { "OO"   },
					 { "OOK"  },
					 { "W"    },
					 { "WW"   } } ;

		set<string> OWBlock  = { { "OO"   },
					 { "WW"   } } ;

		set<string> ReptOK   = { { "C"    },
					 { "D"    },
					 { "F"    },
					 { "HX"   },
					 { "I"    },
					 { "L"    },
					 { "M"    },
					 { "MD"   },
					 { "MN"   },
					 { "O"    },
					 { "OK"   },
					 { "R"    },
					 { "UC"   },
					 { "LC"   },
					 { "RR"   },
					 { "S"    },
					 { "TJ"   },
					 { "TR"   },
					 { "T"    },
					 { "TX"   },
					 { "W"    },
					 { "X"    },
					 { "(("   },
					 { "))"   },
					 { "("    },
					 { ")"    },
					 { "<<"   },
					 { ">>"   },
					 { "<"    },
					 { ">"    },
					 { "[["   },
					 { "]]"   },
					 { "["    },
					 { "]"    } } ;

		set<string> XBlock   = { { "R"    },
					 { "(("   },
					 { "))"   },
					 { "("    },
					 { ")"    },
					 { "<<"   },
					 { ">>"   },
					 { "<"    },
					 { ">"    },
					 { "[["   },
					 { "]]"   },
					 { "["    },
					 { "]"    } } ;

		friend class PEDRXM1 ;
} ;
