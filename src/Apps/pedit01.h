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


//                   ----+----1----+----2----+----3--
#define ID_FILE    0b10000000000000000000000000000000
#define ID_MOVEC   0b01000000000000000000000000000000
#define ID_COPYC   0b00100000000000000000000000000000
#define ID_REPTC   0b00100000000000000000000000000000
#define ID_MOVEM   0b00010000000000000000000000000000
#define ID_COPYRM  0b00001000000000000000000000000000
#define ID_TE      0b00000100000000000000000000000000
#define ID_ISRT    0b00000010000000000000000000000000

//                   ----+----1----+----2----+----3--
#define ID_OWRITE  0b00000001100000000000000000000000
#define ID_CHNGO   0b00000001010000000000000000000000
#define ID_CSHIFT  0b00000001001000000000000000000000
#define ID_DSHIFT  0b00000001000100000000000000000000
#define ID_TFTS    0b00000001000010000000000000000000
#define ID_RENUM   0b00000001000001000000000000000000


enum RECV_STATUS
{
	RECV_RUNNING,
	RECV_STOPPING,
	RECV_STOPPED
} ;

enum P_CMDS
{
	PC_INVCMD,
	PC_AUTONUM,
	PC_AUTOSAVE,
	PC_BACKUP,
	PC_BOUNDS,
	PC_BROWSE,
	PC_CANCEL,
	PC_CAPS,
	PC_CHANGE,
	PC_COLUMN,
	PC_COMPARE,
	PC_COPY,
	PC_CREATE,
	PC_CUT,
	PC_DEFINE,
	PC_DELETE,
	PC_EDIT,
	PC_EDITSET,
	PC_END,
	PC_EXCLUDE,
	PC_FIND,
	PC_FLIP,
	PC_HEX,
	PC_HIDE,
	PC_HILITE,
	PC_IMACRO,
	PC_LEVEL,
	PC_LOCATE,
	PC_MOVE,
	PC_NONUMBER,
	PC_NOTES,
	PC_NULLS,
	PC_NUMBER,
	PC_PASTE,
	PC_PRESERVE,
	PC_PROFILE,
	PC_RCHANGE,
	PC_RECOVERY,
	PC_REDO,
	PC_RENUM,
	PC_REPLACE,
	PC_RESET,
	PC_RFIND,
	PC_SAVE,
	PC_SETUNDO,
	PC_SORT,
	PC_STATS,
	PC_TABS,
	PC_UNDO,
	PC_UNNUMBER,
	PC_VIEW,
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
	LC_LMAC,
	LC_M,
	LC_MASK,
	LC_MD,
	LC_MDD,
	LC_MM,
	LC_MN,
	LC_MNN,
	LC_MOVE,
	LC_NOP,
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
	LC_T,
	LC_TABS,
	LC_TFLOW,
	LC_TJ,
	LC_TJJ,
	LC_TR,
	LC_TRR,
	LC_TS,
	LC_TT,
	LC_TX,
	LC_TXX,
	LC_UC,
	LC_UCC,
	LC_W,
	LC_WW,
	LC_X,
	LC_XI,
	LC_XX
} ;


enum LN_TYPE
{
	LN_INVCMD,
	LN_BNDS,
	LN_BOD,
	LN_COL,
	LN_FILE,
	LN_INFO,
	LN_ISRT,
	LN_MASK,
	LN_MSG,
	LN_NOTE,
	LN_PROF,
	LN_TABS,
	LN_TOD
} ;


enum LS_TYPE
{
	LS_CHNG,
	LS_ERROR,
	LS_NONE,
	LS_REDO,
	LS_UNDO
} ;


enum M_CMDS
{
	EM_AUTONUM,
	EM_AUTOSAVE,
	EM_BACKUP,
	EM_BOUNDS,
	EM_BUILTIN,
	EM_CANCEL,
	EM_CAPS,
	EM_CHANGE,
	EM_CHANGE_COUNTS,
	EM_COMPARE,
	EM_COPY,
	EM_CREATE,
	EM_CURSOR,
	EM_CUT,
	EM_DATAID,
	EM_DATASET,
	EM_DATA_CHANGED,
	EM_DATA_WIDTH,
	EM_DEFINE,
	EM_DELETE,
	EM_DISPLAY_COLS,
	EM_DISPLAY_LINES,
	EM_DOWN,
	EM_END,
	EM_EXCLUDE,
	EM_EXCLUDE_COUNTS,
	EM_FIND,
	EM_FIND_COUNTS,
	EM_FLIP,
	EM_FLOW_COUNTS,
	EM_HEX,
	EM_HIDE,
	EM_HILITE,
	EM_IMACRO,
	EM_INSERT,
	EM_INVCMD,
	EM_LABEL,
	EM_LEFT,
	EM_LINE,
	EM_LINENUM,
	EM_LINE_AFTER,
	EM_LINE_BEFORE,
	EM_LINE_STATUS,
	EM_LOCATE,
	EM_LRECL,
	EM_MACRO,
	EM_MACRO_LEVEL,
	EM_MACRO_MSG,
	EM_MASKLINE,
	EM_MOVE,
	EM_NONUMBER,
	EM_NOTES,
	EM_NULLS,
	EM_NUMBER,
	EM_PATH,
	EM_PROCESS,
	EM_PROFILE,
	EM_RANGE_CMD,
	EM_RCHANGE,
	EM_RECFM,
	EM_RECOVERY,
	EM_RENUM,
	EM_REPLACE,
	EM_RESET,
	EM_RFIND,
	EM_RIGHT,
	EM_SAVE,
	EM_SAVE_LENGTH,
	EM_SCAN,
	EM_SEEK,
	EM_SEEK_COUNTS,
	EM_SESSION,
	EM_SETUNDO,
	EM_SHIFT,
	EM_SORT,
	EM_STATS,
	EM_TABS,
	EM_TABSLINE,
	EM_TFLOW,
	EM_UNNUMBER,
	EM_UP,
	EM_USER_STATE,
	EM_XSTATUS,
	EM_XTABS
} ;


enum D_PARMS
{
	DF_ALIAS,
	DF_DISABLED,
	DF_MACRO,
	DF_PGM,
	DF_CMD,
	DF_NOP,
	DF_RESET
} ;


enum CSR_POS
{
	CSR_FC_DATA,
	CSR_FC_LCMD,
	CSR_FNB_DATA,
	CSR_LC_OR_FNB_DATA,
	CSR_OFF_DATA,
	CSR_OFF_LCMD
} ;


enum SS_TYPE
{
	SS_ALL,
	SS_AUTONUM,
	SS_AUTOSAVE,
	SS_CAPS,
	SS_HEX,
	SS_HILITE,
	SS_IMACRO,
	SS_NOTES,
	SS_NULLS,
	SS_NUMBER,
	SS_PROFILE,
	SS_STATS,
	SS_TABS,
	SS_XTABS
} ;


enum CA_TYPE
{
	CA_NONE,
	CA_CAPS_OFF,
	CA_CAPS_ON,
	CA_NUM_OFF,
	CA_NUM_ON,
	CA_NUM_CBL_ON,
	CA_NUM_CBL_OFF,
	CA_NUM_STD_ON,
	CA_NUM_STD_OFF,
	CA_PSWITCH,
	CA_SETU_OFF,
	CA_XTABS_OFF,
	CA_XTABS_ON
} ;


class pcmd_entry

{
	public:
	       P_CMDS p_cmd ;
	       string truename ;
} ;


class pcmd_format
{
	public:
	       int  noptions1 ;
	       int  noptions2 ;
	       bool ucase ;
} ;


class mcmd_format
{
	public:
	       M_CMDS m_cmd  ;
	       bool csvalid  ;
	       bool qasvalid ;
	       bool aasvalid ;
	       int  qnvars1  ;
	       int  qnvars2  ;
	       int  qnkeyopts1 ;
	       int  qnkeyopts2 ;
	       int  ankeyopts1 ;
	       int  ankeyopts2 ;
	       int  anvalopts1 ;
	       int  anvalopts2 ;
} ;


class profile
{
	public:
		string profName       ;
		bool   profAutoSave   ;
		bool   profSaveP      ;
		bool   profNullA      ;
		bool   profNulls      ;
		bool   profLock       ;
		bool   profCaps       ;
		bool   profHex        ;
		bool   profTabs       ;
		bool   profATabs      ;
		bool   profXTabs      ;
		int    profXTabz      ;
		bool   profRecovery   ;
		bool   profBackup     ;
		bool   profHilight    ;
		bool   profIfLogic    ;
		bool   profDoLogic    ;
		bool   profLogic      ;
		bool   profParen      ;
		bool   profCutReplace ;
		bool   profPasteKeep  ;
		bool   profPosFcx     ;
		string profLang       ;
		string profIMACRO     ;
		bool   profVert       ;
		bool   profFindPhrase ;
		bool   profCsrPhrase  ;
		bool   profUndoKeep   ;
		bool   profNum        ;
		bool   profNumSTD     ;
		bool   profNumCBL     ;
		bool   profNumDisp    ;
		bool   profAutoNum    ;
		bool   profNotes      ;
		bool   profStats      ;

		bool operator != ( const profile& rhs ) const
		{
			return ( profAutoSave   != rhs.profAutoSave   ||
				 profATabs      != rhs.profATabs      ||
				 profAutoNum    != rhs.profAutoNum    ||
				 profCaps       != rhs.profCaps       ||
				 profCsrPhrase  != rhs.profCsrPhrase  ||
				 profCutReplace != rhs.profCutReplace ||
				 profDoLogic    != rhs.profDoLogic    ||
				 profFindPhrase != rhs.profFindPhrase ||
				 profHex        != rhs.profHex        ||
				 profHilight    != rhs.profHilight    ||
				 profIfLogic    != rhs.profIfLogic    ||
				 profLock       != rhs.profLock       ||
				 profLogic      != rhs.profLogic      ||
				 profNotes      != rhs.profNotes      ||
				 profNullA      != rhs.profNullA      ||
				 profNulls      != rhs.profNulls      ||
				 profNum        != rhs.profNum        ||
				 profNumCBL     != rhs.profNumCBL     ||
				 profNumDisp    != rhs.profNumDisp    ||
				 profNumSTD     != rhs.profNumSTD     ||
				 profParen      != rhs.profParen      ||
				 profPasteKeep  != rhs.profPasteKeep  ||
				 profPosFcx     != rhs.profPosFcx     ||
				 profRecovery   != rhs.profRecovery   ||
				 profBackup     != rhs.profBackup     ||
				 profSaveP      != rhs.profSaveP      ||
				 profTabs       != rhs.profTabs       ||
				 profUndoKeep   != rhs.profUndoKeep   ||
				 profVert       != rhs.profVert       ||
				 profXTabs      != rhs.profXTabs      ||
				 profXTabz      != rhs.profXTabz      ||
				 profStats      != rhs.profStats      ||
				 profIMACRO     != rhs.profIMACRO     ||
				 profLang       != rhs.profLang       ||
				 profName       != rhs.profName       ) ;
		}

		bool operator == ( const profile& rhs ) const
		{
			return ( profAutoSave   == rhs.profAutoSave   &&
				 profATabs      == rhs.profATabs      &&
				 profAutoNum    == rhs.profAutoNum    &&
				 profCaps       == rhs.profCaps       &&
				 profCsrPhrase  == rhs.profCsrPhrase  &&
				 profCutReplace == rhs.profCutReplace &&
				 profDoLogic    == rhs.profDoLogic    &&
				 profFindPhrase == rhs.profFindPhrase &&
				 profHex        == rhs.profHex        &&
				 profHilight    == rhs.profHilight    &&
				 profIfLogic    == rhs.profIfLogic    &&
				 profLock       == rhs.profLock       &&
				 profLogic      == rhs.profLogic      &&
				 profNotes      == rhs.profNotes      &&
				 profNullA      == rhs.profNullA      &&
				 profNulls      == rhs.profNulls      &&
				 profNum        == rhs.profNum        &&
				 profNumCBL     == rhs.profNumCBL     &&
				 profNumDisp    == rhs.profNumDisp    &&
				 profNumSTD     == rhs.profNumSTD     &&
				 profParen      == rhs.profParen      &&
				 profPasteKeep  == rhs.profPasteKeep  &&
				 profPosFcx     == rhs.profPosFcx     &&
				 profRecovery   == rhs.profRecovery   &&
				 profBackup     == rhs.profBackup     &&
				 profSaveP      == rhs.profSaveP      &&
				 profTabs       == rhs.profTabs       &&
				 profUndoKeep   == rhs.profUndoKeep   &&
				 profVert       == rhs.profVert       &&
				 profXTabs      == rhs.profXTabs      &&
				 profXTabz      == rhs.profXTabz      &&
				 profStats      == rhs.profStats      &&
				 profIMACRO     == rhs.profIMACRO     &&
				 profLang       == rhs.profLang       &&
				 profName       == rhs.profName       ) ;
		}
} ;


class caution
{
	public:
		caution( CA_TYPE catype,
			 const string& msg ) : caution( catype )
		{
			message = msg ;
		}

		caution( CA_TYPE catype = CA_NONE )
		{
			type = catype ;
		}

		CA_TYPE type ;
		string  message ;
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


//                                                        +----------Minimum number of parameters
//                                                        |   +------Maximum number of parameters
//                                                        |   |      value of -1 means don't check
//                                                        |   |
//                                                        |   |  +---Upper case on error.
//                                                        |   |  |
//                                                        V   V  V
map<P_CMDS, pcmd_format> pcmdFormat = { { PC_AUTONUM,  {  0,  1, true  } },
					{ PC_AUTOSAVE, {  0,  2, true  } },
					{ PC_BACKUP,   { -1, -1, false } },
					{ PC_BOUNDS,   {  0,  2, true  } },
					{ PC_BROWSE,   { -1, -1, false } },
					{ PC_CANCEL,   {  0,  0, true  } },
					{ PC_CHANGE,   { -1, -1, false } },
					{ PC_CAPS,     {  0,  1, true  } },
					{ PC_COLUMN,   {  0,  1, true  } },
					{ PC_COMPARE,  { -1, -1, false } },
					{ PC_COPY,     {  0,  1, false } },
					{ PC_CREATE,   { -1, -1, false } },
					{ PC_CUT,      { -1, -1, true  } },
					{ PC_DEFINE,   {  2,  3, true  } },
					{ PC_DELETE,   { -1, -1, true  } },
					{ PC_EDIT,     { -1, -1, false } },
					{ PC_EDITSET,  {  0,  0, true  } },
					{ PC_END,      {  0,  0, true  } },
					{ PC_EXCLUDE,  { -1, -1, true  } },
					{ PC_FIND,     { -1, -1, false } },
					{ PC_FLIP,     {  0,  2, true  } },
					{ PC_HEX,      {  0,  2, true  } },
					{ PC_HIDE,     {  1,  1, true  } },
					{ PC_HILITE,   { -1, -1, true  } },
					{ PC_IMACRO,   {  1,  1, true  } },
					{ PC_LEVEL,    {  1,  1, true  } },
					{ PC_LOCATE,   { -1, -1, true  } },
					{ PC_MOVE,     {  0,  1, false } },
					{ PC_NULLS,    {  0,  2, true  } },
					{ PC_NONUMBER, {  0,  0, true  } },
					{ PC_NOTES,    {  0,  1, true  } },
					{ PC_NUMBER,   {  0,  4, true  } },
					{ PC_PASTE,    { -1, -1, true  } },
					{ PC_PROFILE,  { -1, -1, true  } },
					{ PC_PRESERVE, {  0,  1, true  } },
					{ PC_RCHANGE,  {  0,  0, true  } },
					{ PC_RECOVERY, { -1, -1, true  } },
					{ PC_REDO,     {  0,  1, true  } },
					{ PC_RENUM,    {  0,  3, true  } },
					{ PC_REPLACE,  { -1, -1, false } },
					{ PC_RESET,    { -1, -1, true  } },
					{ PC_RFIND,    {  0,  0, true  } },
					{ PC_SAVE,     {  0,  0, true  } },
					{ PC_SETUNDO,  {  1,  1, true  } },
					{ PC_SORT,     { -1, -1, true  } },
					{ PC_STATS,    {  0,  1, true  } },
					{ PC_TABS,     { -1, -1, true  } },
					{ PC_UNDO,     {  0,  1, true  } },
					{ PC_UNNUMBER, {  0,  0, true  } },
					{ PC_VIEW,     { -1, -1, true  } },
					{ PC_XTABS,    {  1,  1, true  } } } ;

//                                         +------------------Command syntax valid
//                                         |      +-----------Query assignment syntax valid
//                                         |      |      +----Action assignment syntax valid
//                                         |      |      |       +------------Query: # variables(min,max)
//                                         |      |      |       |       +----Query: # key options(min,max)
//                                         |      |      |       |       |       +----------Action: # key options(min,max)
//                                         |      |      |       |       |       |       +--Action: # value options(min,max)
//                                         |      |      |       |       |       |       |            -1 = don't check
//                                         V      V      V       V       V       V       V
map<string, mcmd_format> emService = //    ~~~~   ~~~~~  ~~~~~   ~~~~~   ~~~~~   ~~~~~   ~~~~~~
{ { "AUTONUM",        { EM_AUTONUM,        true,  true,  true,   1,  1,  0,  0,  0,  0,  1,  1 } },
  { "AUTOSAVE",       { EM_AUTOSAVE,       true,  true,  true,   1,  2,  0,  0,  0,  0,  0,  2 } },
  { "BACKUP",         { EM_BACKUP,         true,  true,  true,   1,  2,  0,  0, -1, -1, -1, -1 } },
  { "BOUNDS",         { EM_BOUNDS,         true,  true,  true,   1,  2,  0,  0,  0,  0,  1,  2 } },
  { "CANCEL",         { EM_CANCEL,         true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "CAPS",           { EM_CAPS,           true,  true,  true,   1,  1,  0,  0,  0,  0,  1,  1 } },
  { "CHANGE",         { EM_CHANGE,         true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "CHANGE_COUNTS",  { EM_CHANGE_COUNTS,  false, true,  false,  1,  2,  0,  0, -1, -1, -1, -1 } },
  { "COMPARE",        { EM_COMPARE,        true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "COPY",           { EM_COPY,           true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "CREATE",         { EM_CREATE,         true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "CURSOR",         { EM_CURSOR,         false, true,  true,   1,  2,  0,  0,  0,  0,  1,  2 } },
  { "CUT",            { EM_CUT,            true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "DATAID",         { EM_DATAID,         false, true,  false,  1,  1,  0,  0, -1, -1, -1, -1 } },
  { "DATASET",        { EM_DATASET,        false, true,  false,  1,  1,  0,  0, -1, -1, -1, -1 } },
  { "DATA_CHANGED",   { EM_DATA_CHANGED,   false, true,  false,  1,  1,  0,  0, -1, -1, -1, -1 } },
  { "DATA_WIDTH",     { EM_DATA_WIDTH,     false, true,  false,  1,  1,  0,  0, -1, -1, -1, -1 } },
  { "DEFINE",         { EM_DEFINE,         true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "DELETE",         { EM_DELETE,         true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "DISPLAY_COLS",   { EM_DISPLAY_COLS,   false, true,  false,  1,  2,  0,  0, -1, -1, -1, -1 } },
  { "DISPLAY_LINES",  { EM_DISPLAY_LINES,  false, true,  false,  1,  2,  0,  0, -1, -1, -1, -1 } },
  { "DOWN",           { EM_DOWN,           true,  false, false,  1,  1, -1, -1, -1, -1, -1, -1 } },
  { "END",            { EM_END,            true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "EXCLUDE",        { EM_EXCLUDE,        true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "EXCLUDE_COUNTS", { EM_EXCLUDE_COUNTS, false, true,  false,  1,  2,  0,  0, -1, -1, -1, -1 } },
  { "FIND",           { EM_FIND,           true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "FIND_COUNTS",    { EM_FIND_COUNTS,    false, true,  false,  1,  2,  0,  0, -1, -1, -1, -1 } },
  { "FLIP",           { EM_FLIP,           true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "FLOW_COUNTS",    { EM_FLOW_COUNTS,    false, true,  false,  1,  2,  0,  0, -1, -1, -1, -1 } },
  { "HEX",            { EM_HEX,            true,  true,  true,   1,  2,  0,  0, -1, -1, -1, -1 } },
  { "HIDE",           { EM_HIDE,           true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "HILITE",         { EM_HILITE,         true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "IMACRO",         { EM_IMACRO,         true,  true,  true,   1,  1,  0,  0, -1, -1, -1, -1 } },
  { "INSERT",         { EM_INSERT,         true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "LABEL",          { EM_LABEL,          false, true,  true,   1,  2,  1,  1,  1,  1,  1,  2 } },
  { "LEFT",           { EM_LEFT,           true,  false, false,  1,  1, -1, -1, -1, -1, -1, -1 } },
  { "LINE",           { EM_LINE,           false, true,  true,   1,  1,  1,  1,  1,  1, -1, -1 } },
  { "LINENUM",        { EM_LINENUM,        false, true,  false,  1,  1,  1,  1, -1, -1,  1, -1 } },
  { "LINE_AFTER",     { EM_LINE_AFTER,     false, false, true,  -1, -1, -1, -1,  1,  1, -1, -1 } },
  { "LINE_BEFORE",    { EM_LINE_BEFORE,    false, false, true,  -1, -1, -1, -1,  1,  1, -1, -1 } },
  { "LINE_STATUS",    { EM_LINE_STATUS,    false, true,  false,  1,  1,  1,  1, -1, -1, -1, -1 } },
  { "LOCATE",         { EM_LOCATE,         true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "LRECL",          { EM_LRECL,          false, true,  false,  1,  1,  0,  0, -1, -1, -1, -1 } },
  { "MACRO_LEVEL",    { EM_MACRO_LEVEL,    false, true,  false,  1,  1,  0,  0, -1, -1, -1, -1 } },
  { "MACRO_MSG",      { EM_MACRO_MSG,      false, true,  true,   1,  1,  0,  0,  0,  0,  1,  1 } },
  { "MASKLINE",       { EM_MASKLINE,       false, true,  true,   1,  1,  0,  0,  0,  0, -1, -1 } },
  { "MOVE",           { EM_MOVE,           true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "NONUMBER",       { EM_NONUMBER,       true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "NOTES",          { EM_NOTES,          true,  true,  true,   1,  1,  0,  0,  0,  0,  1,  1 } },
  { "NULLS",          { EM_NULLS,          true,  true,  true,   1,  2,  0,  0, -1, -1, -1, -1 } },
  { "NUMBER",         { EM_NUMBER,         true,  true,  true,   0,  2,  0,  0, -1, -1, -1, -1 } },
  { "REPLACE",        { EM_REPLACE,        true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "PATH",           { EM_PATH,           false, true,  true,   1,  1,  0,  0,  0,  0,  1,  1 } },
  { "PROCESS",        { EM_PROCESS,        true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "PROFILE",        { EM_PROFILE,        true,  true,  false,  1,  2,  0,  0, -1, -1, -1, -1 } },
  { "RANGE_CMD",      { EM_RANGE_CMD,      false, true,  false,  1,  1,  0,  0, -1, -1, -1, -1 } },
  { "RCHANGE",        { EM_RCHANGE,        true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "RECFM",          { EM_RECFM,          false, true,  false,  1,  2,  0,  0, -1, -1, -1, -1 } },
  { "RECOVERY",       { EM_RECOVERY,       true,  true,  true,   1,  2,  0,  0, -1, -1, -1, -1 } },
  { "RENUM",          { EM_RENUM,          true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "RESET",          { EM_RESET,          true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "RFIND",          { EM_RFIND,          true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "RIGHT",          { EM_RIGHT,          true,  false, false,  1,  1, -1, -1, -1, -1, -1, -1 } },
  { "SAVE",           { EM_SAVE,           true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "SAVE_LENGTH",    { EM_SAVE_LENGTH,    false, true,  true,   1,  1,  1,  1,  1,  1,  1,  1 } },
  { "SCAN",           { EM_SCAN,           true,  true,  true,   1,  1,  0,  0,  0,  0,  1,  1 } },
  { "SEEK",           { EM_SEEK,           true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "SEEK_COUNTS",    { EM_SEEK_COUNTS,    false, true,  false,  1,  2,  0,  0, -1, -1, -1, -1 } },
  { "SESSION",        { EM_SESSION,        false, true,  false,  1,  2,  0,  0, -1, -1, -1, -1 } },
  { "SETUNDO",        { EM_SETUNDO,        true,  true,  true,   1,  1,  0,  0,  0,  0,  1,  1 } },
  { "SHIFT",          { EM_SHIFT,          true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "SORT",           { EM_SORT,           true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "STATS",          { EM_STATS,          true,  true,  true,   1,  1,  0,  0,  0,  0,  1,  1 } },
  { "TABS",           { EM_TABS,           true,  true,  true,   1,  2,  0,  0, -1, -1, -1, -1 } },
  { "TABSLINE",       { EM_TABSLINE,       false, true,  true,   1,  1,  0,  0,  0,  0, -1, -1 } },
  { "TFLOW",          { EM_TFLOW,          true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "UNNUMBER",       { EM_UNNUMBER,       true,  false, false, -1, -1, -1, -1, -1, -1, -1, -1 } },
  { "UP",             { EM_UP,             true,  false, false,  1,  1, -1, -1, -1, -1, -1, -1 } },
  { "USER_STATE",     { EM_USER_STATE,     false, true,  true,   1,  1,  0,  0,  0,  0,  1,  1 } },
  { "XSTATUS",        { EM_XSTATUS,        false, true,  true,   1,  1,  1,  1,  1,  1,  1,  1 } },
  { "XTABS",          { EM_XTABS,          true,  true,  true,   1,  2,  0,  0, -1, -1, -1, -1 } } } ;


map<string, pcmd_entry> PrimCMDS =
{ { "AUTONUM",  { PC_AUTONUM,  "AUTONUM"  } },
  { "AUTOSAVE", { PC_AUTOSAVE, "AUTOSAVE" } },
  { "BACKUP",   { PC_BACKUP,   "BACKUP"   } },
  { "BACKU",    { PC_BACKUP,   "BACKUP"   } },
  { "BND",      { PC_BOUNDS,   "BOUNDS"   } },
  { "BNDS",     { PC_BOUNDS,   "BOUNDS"   } },
  { "BOU",      { PC_BOUNDS,   "BOUNDS"   } },
  { "BOUND",    { PC_BOUNDS,   "BOUNDS"   } },
  { "BOUNDS",   { PC_BOUNDS,   "BOUNDS"   } },
  { "BROWSE",   { PC_BROWSE,   "BROWSE"   } },
  { "CANCEL",   { PC_CANCEL,   "CANCEL"   } },
  { "CAPS",     { PC_CAPS,     "CAPS"     } },
  { "C",        { PC_CHANGE,   "CHANGE"   } },
  { "CHA",      { PC_CHANGE,   "CHANGE"   } },
  { "CHANGE",   { PC_CHANGE,   "CHANGE"   } },
  { "CHG",      { PC_CHANGE,   "CHANGE"   } },
  { "COL",      { PC_COLUMN,   "COLUMN"   } },
  { "COLS",     { PC_COLUMN,   "COLUMN"   } },
  { "COLUMN",   { PC_COLUMN,   "COLUMN"   } },
  { "COMP",     { PC_COMPARE,  "COMPARE"  } },
  { "COMPARE",  { PC_COMPARE,  "COMPARE"  } },
  { "COPY",     { PC_COPY,     "COPY"     } },
  { "CRE",      { PC_CREATE,   "CREATE"   } },
  { "CREATE",   { PC_CREATE,   "CREATE"   } },
  { "CUT",      { PC_CUT,      "CUT"      } },
  { "DEF",      { PC_DEFINE,   "DEFINE"   } },
  { "DEFINE",   { PC_DEFINE,   "DEFINE"   } },
  { "DEL",      { PC_DELETE,   "DELETE"   } },
  { "DELETE",   { PC_DELETE,   "DELETE"   } },
  { "EDIT",     { PC_EDIT,     "EDIT"     } },
  { "EDITSET",  { PC_EDITSET,  "EDITSET"  } },
  { "EDSET",    { PC_EDITSET,  "EDITSET"  } },
  { "END",      { PC_END,      "END"      } },
  { "X",        { PC_EXCLUDE,  "EXCLUDE"  } },
  { "EX",       { PC_EXCLUDE,  "EXCLUDE"  } },
  { "EXC",      { PC_EXCLUDE,  "EXCLUDE"  } },
  { "EXCLUDE",  { PC_EXCLUDE,  "EXCLUDE"  } },
  { "EXCLUDED", { PC_EXCLUDE,  "EXCLUDE"  } },
  { "F",        { PC_FIND,     "FIND"     } },
  { "FIND",     { PC_FIND,     "FIND"     } },
  { "FLIP",     { PC_FLIP,     "FLIP"     } },
  { "HEX",      { PC_HEX,      "HEX"      } },
  { "HIDE",     { PC_HIDE,     "HIDE"     } },
  { "HI",       { PC_HILITE,   "HILITE"   } },
  { "HILIGHT",  { PC_HILITE,   "HILITE"   } },
  { "HILITE",   { PC_HILITE,   "HILITE"   } },
  { "IMAC",     { PC_IMACRO,   "IMACRO"   } },
  { "IMACRO",   { PC_IMACRO,   "IMACRO"   } },
  { "LEVEL",    { PC_LEVEL,    "LEVEL"    } },
  { "L",        { PC_LOCATE,   "LOCATE"   } },
  { "LOC",      { PC_LOCATE,   "LOCATE"   } },
  { "LOCATE",   { PC_LOCATE,   "LOCATE"   } },
  { "MOVE",     { PC_MOVE,     "MOVE"     } },
  { "NONUM",    { PC_NONUMBER, "NONUMBER" } },
  { "NONUMB",   { PC_NONUMBER, "NONUMBER" } },
  { "NONUMBER", { PC_NONUMBER, "NONUMBER" } },
  { "NONUMBR",  { PC_NONUMBER, "NONUMBER" } },
  { "NOTE",     { PC_NOTES,    "NOTES"    } },
  { "NOTES",    { PC_NOTES,    "NOTES"    } },
  { "NUL",      { PC_NULLS,    "NULLS"    } },
  { "NULL",     { PC_NULLS,    "NULLS"    } },
  { "NULLS",    { PC_NULLS,    "NULLS"    } },
  { "NUM",      { PC_NUMBER,   "NUMBER"   } },
  { "NUMB",     { PC_NUMBER,   "NUMBER"   } },
  { "NUMBER",   { PC_NUMBER,   "NUMBER"   } },
  { "PASTE",    { PC_PASTE,    "PASTE"    } },
  { "PRESERVE", { PC_PRESERVE, "PRESERVE" } },
  { "PR",       { PC_PROFILE,  "PROFILE"  } },
  { "PRO",      { PC_PROFILE,  "PROFILE"  } },
  { "PROF",     { PC_PROFILE,  "PROFILE"  } },
  { "PROFILE",  { PC_PROFILE,  "PROFILE"  } },
  { "RCHANGE",  { PC_RCHANGE,  "RCHANGE"  } },
  { "RECOV",    { PC_RECOVERY, "RECOVERY" } },
  { "RECOVER",  { PC_RECOVERY, "RECOVERY" } },
  { "RECOVERY", { PC_RECOVERY, "RECOVERY" } },
  { "RECOVRY",  { PC_RECOVERY, "RECOVERY" } },
  { "RECVR",    { PC_RECOVERY, "RECOVERY" } },
  { "RECVRY",   { PC_RECOVERY, "RECOVERY" } },
  { "REDO",     { PC_REDO,     "REDO"     } },
  { "REN",      { PC_RENUM,    "RENUM"    } },
  { "RENUM",    { PC_RENUM,    "RENUM"    } },
  { "REP",      { PC_REPLACE,  "REPLACE"  } },
  { "REPL",     { PC_REPLACE,  "REPLACE"  } },
  { "REPLACE",  { PC_REPLACE,  "REPLACE"  } },
  { "RES",      { PC_RESET,    "RESET"    } },
  { "RESET",    { PC_RESET,    "RESET"    } },
  { "RFIND",    { PC_RFIND,    "RFIND"    } },
  { "SAVE",     { PC_SAVE,     "SAVE"     } },
  { "SETU",     { PC_SETUNDO,  "SETUNDO"  } },
  { "SETUNDO",  { PC_SETUNDO,  "SETUNDO"  } },
  { "SORT",     { PC_SORT,     "SORT"     } },
  { "STATS",    { PC_STATS,    "STATS"    } },
  { "TAB",      { PC_TABS,     "TABS"     } },
  { "TABS",     { PC_TABS,     "TABS"     } },
  { "UNDO",     { PC_UNDO,     "UNDO"     } },
  { "UNN",      { PC_UNNUMBER, "UNNUMBER" } },
  { "UNNUM",    { PC_UNNUMBER, "UNNUMBER" } },
  { "UNNUMB",   { PC_UNNUMBER, "UNNUMBER" } },
  { "UNNUMBER", { PC_UNNUMBER, "UNNUMBER" } },
  { "VIEW",     { PC_VIEW,     "VIEW"     } },
  { "XTABS",    { PC_XTABS,    "XTABS"    } } } ;


map<string, D_PARMS> DefParms =
{ { "MACRO",    DF_MACRO    },
  { "PGM",      DF_PGM      },
  { "CMD",      DF_CMD      },
  { "ALIAS",    DF_ALIAS    },
  { "NOP",      DF_NOP      },
  { "DISABLED", DF_DISABLED },
  { "RESET",    DF_RESET    } } ;


class idata
{
	public:
		idata( const idata& ca )
		{
			id_level  = ca.id_level ;
			id_delete = ca.id_delete ;
			id_status = ca.id_status ;
			id_data   = ca.id_data ;
		}

		idata( const idata& ca, int level )
		{
			id_level  = level ;
			id_delete = ca.id_delete ;
			id_status = ca.id_status ;
			id_data   = ca.id_data ;
		}

		idata operator | ( const idata& rhs )
		{
			id_status |= rhs.id_status ;

			return *this ;
		}

		idata operator = ( const idata& rhs )
		{
			id_level   = rhs.id_level ;
			id_delete  = rhs.id_delete ;
			id_status |= rhs.id_status ;
			id_data    = rhs.id_data ;

			return *this ;
		}

	private:
		idata( uint32_t status ) : idata()
		{
			id_status = status ;
		}

		idata( const string& s,
		       uint32_t status ) : idata()
		{
			id_status = status ;
			id_data   = s ;
		}

		idata( const string& s,
		       uint32_t status,
		       int level ) : idata()
		{
			id_level  = level ;
			id_status = status ;
			id_data   = s ;
		}

		idata()
		{
			id_level  = 0  ;
			id_delete = false ;
			id_status = 0  ;
			id_data   = "" ;
		}

		void add_status( uint32_t s )
		{
			id_status |= s ;
		}

		int      id_level  ;
		uint32_t id_status ;
		bool     id_delete ;
		string   id_data   ;

	friend class iline ;
} ;


class icond
{
	private:
		icond( LS_TYPE c, int lvl )
		{
			ic_level  = lvl ;
			ic_status = c ;
		}

		int     ic_level ;
		LS_TYPE ic_status ;

	friend class iline ;
} ;


class iexcl
{
	private:
		iexcl() : iexcl( 0, false )
		{
		}

		iexcl( bool b, int lvl )
		{
			ix_level = lvl ;
			ix_excl  = b   ;
		}

		int  ix_level ;
		bool ix_excl  ;

	friend class iline ;
} ;


class ilabel
{
	private:
		ilabel()
		{
			iy_level = 0 ;
		}

		bool empty()
		{
			return iy_label.empty() ;
		}

		int iy_level ;
		map<int, string> iy_label ;

	friend class iline ;
} ;


class istatus
{
	public:
		istatus()
		{
			is_level = 0 ;
			is_bnd1  = 0 ;
			is_bnd2  = 0 ;
			is_sbnd1 = false ;
			is_sbnd2 = false ;
		}

		istatus( SS_TYPE type,
			 const profile& prof ) : istatus()
		{
			is_type = type ;
			is_prof = prof ;
		}

		istatus( SS_TYPE type,
			 const profile& prof,
			 int bnd1,
			 int bnd2,
			 bool sbnd1,
			 bool sbnd2 ) : istatus()
		{
			is_type  = type ;
			is_bnd1  = bnd1 ;
			is_bnd2  = bnd2 ;
			is_sbnd1 = bnd1 ;
			is_sbnd2 = bnd2 ;
			is_prof  = prof ;
		}

		int     is_level ;
		int     is_bnd1  ;
		int     is_bnd2  ;
		bool    is_sbnd1 ;
		bool    is_sbnd2 ;
		SS_TYPE is_type  ;
		profile is_prof  ;

		bool operator != ( const istatus& rhs ) const
		{
			return ( is_prof != rhs.is_prof ) ;
		}
} ;


class ipline
{
	private:
		ipline( const string& s ) : ipline()
		{
			ip_data = s ;
		}

		ipline( LN_TYPE t ) : ipline()
		{
			ip_type = t ;
		}

		ipline( LN_TYPE t,
			const string& s ) : ipline()
		{
			ip_type = t ;
			ip_data = s ;
		}

		ipline()
		{
			ip_type   = LN_INVCMD ;
			ip_excl   = false ;
			ip_hex    = false ;
			ip_prof   = false ;
			ip_profln = 0     ;
			ip_label  = ""    ;
			ip_data   = ""    ;
			ip_status = 0     ;
		}

		bool is_file() const
		{
			return ( ip_type == LN_FILE ) ;
		}

		bool is_isrt() const
		{
			return ( ip_type == LN_ISRT ) ;
		}

		LN_TYPE  ip_type   ;
		bool     ip_excl   ;
		bool     ip_hex    ;
		bool     ip_prof   ;
		int      ip_profln ;
		string   ip_data   ;
		string   ip_label  ;
		uint32_t ip_status ;

	friend class pedit01 ;
} ;


class iline
{
	private:
		static map<int, stack<int>>Global_Undo ;
		static map<int, stack<int>>Global_Redo ;
		static map<int, stack<int>>Global_File_level ;
		static map<int, stack<istatus>>Global_status ;
		static map<int, stack<istatus>>Global_status_redo ;
		static map<int, bool>setUNDO   ;
		static map<int, bool>Redo_data ;
		static map<int, bool>Redo_other ;
		static map<int, bool>File_save ;
		static map<int, bool>changed_icond ;
		static map<int, bool>changed_ilabel ;
		static map<int, bool>changed_xclud ;
		static map<int, string>src_file ;
		static map<int, string>dst_file ;
		static map<int, bool>file_changed ;
		static map<int, bool>file_inserts ;

		static void init_Globals( int task,
					  bool vmode,
					  const string& f1 = "",
					  const string& f2 = "" )
		{
			src_file[ task ]  = f1 ;
			dst_file[ task ]  = f2 ;
			File_save[ task ] = ( vmode ) ? true : false ;
			file_changed[ task ]   = false ;
			file_inserts[ task ]   = false ;
			changed_ilabel[ task ] = false ;
			changed_xclud[ task ]  = false ;
		}

		static void set_Globals( int task,
					 const string& f1 = "",
					 const string& f2 = "" )
		{
			src_file[ task ]  = f1 ;
			dst_file[ task ]  = f2 ;
		}

		static bool is_File_save( int task )
		{
			return File_save[ task ] ;
		}

		static void copyFile( int task )
		{
			boost::system::error_code ec ;
			if ( src_file[ task ].compare( 0, 5, "/tmp/" ) == 0 )
			{
				File_save[ task ] = true ;
			}
			else if ( src_file[ task ] != "" && dst_file[ task ] != "" )
			{
				copy_file( src_file[ task ], dst_file[ task ], ec ) ;
				File_save[ task ] = true ;
			}
		}

		static int get_Global_Undo_Size( int task )
		{
			return Global_Undo[ task ].size() ;
		}

		static int get_Global_Redo_Size( int task )
		{
			return Global_Redo[ task ].size() ;
		}

		static int get_Global_File_Size( int task )
		{
			return Global_File_level[ task ].size() ;
		}

		static void clear_Global_Undo( int task )
		{
			while ( !Global_Undo[ task ].empty() )
			{
				Global_Undo[ task ].pop() ;
			}
		}

		static void clear_Global_Redo( int task )
		{
			while ( !Global_Redo[ task ].empty() )
			{
				Global_Redo[ task ].pop() ;
			}
		}

		static bool has_Redo_data( int task )
		{
			return Redo_data[ task ] ;
		}

		static void reset_Redo_data( int task )
		{
			Redo_data[ task ] = false ;
		}

		static bool has_Redo_other( int task )
		{
			return Redo_other[ task ] ;
		}

		static void reset_Redo_other( int task )
		{
			Redo_other[ task ] = false ;
		}

		static void reset_Global_Undo( int task )
		{
			Global_Undo[ task ] = Global_File_level[ task ] ;
		}

		static void clear_Global_File_level( int task )
		{
			while ( !Global_File_level[ task ].empty() )
			{
				Global_File_level[ task ].pop() ;
			}
		}

		static int get_Global_File_level( int task )
		{
			if ( Global_File_level[ task ].empty() )
			{
				return 0 ;
			}
			else
			{
				return Global_File_level[ task ].top() ;
			}
		}

		static void set_Global_File_level( int task )
		{
			if ( Global_File_level[ task ].empty() ||
			     Global_File_level[ task ].top() != Global_Undo[ task ].top() )
			{
				Global_File_level[ task ].push( Global_Undo[ task ].top() ) ;
			}
		}

		static void remove_Global_File_level( int task )
		{
			Global_File_level[ task ].pop() ;
		}

		static int get_Global_Undo_level( int task )
		{
			if ( !setUNDO[ task ] || Global_Undo[ task ].empty() )
			{
				return 0 ;
			}
			return Global_Undo[ task ].top() ;
		}

		static int get_Global_Redo_level( int task )
		{
			if ( !setUNDO[ task ] || Global_Redo[ task ].empty() )
			{
				return 0 ;
			}
			return Global_Redo[ task ].top() ;
		}

		static void move_Global_Undo2Redo( int task )
		{
			if ( setUNDO[ task ] && !Global_Undo[ task ].empty() )
			{
				Global_Redo[ task ].push( Global_Undo[ task ].top() ) ;
				Global_Undo[ task ].pop() ;
			}
		}

		static void move_Global_Redo2Undo( int task )
		{
			if ( setUNDO[ task ] && !Global_Redo[ task ].empty() )
			{
				Global_Undo[ task ].push( Global_Redo[ task ].top() ) ;
				Global_Redo[ task ].pop() ;
			}
		}

		static bool data_Changed( int task )
		{
			return file_changed[ task ] ;
		}

		static void reset_changed_icond( int task )
		{
			changed_icond[ task ] = false ;
		}

		static bool has_changed_icond( int task )
		{
			return changed_icond[ task ] ;
		}

		static void reset_changed_ilabel( int task )
		{
			changed_ilabel[ task ] = false ;
		}

		static bool has_changed_ilabel( int task )
		{
			return changed_ilabel[ task ] ;
		}

		static void reset_changed_xclud( int task )
		{
			changed_xclud[ task ] = false ;
		}

		static bool has_changed_xclud( int task )
		{
			return changed_xclud[ task ] ;
		}

		static void status_put( int task,
					SS_TYPE type,
					const profile& prof,
					int bnd1,
					int bnd2,
					bool sbnd1,
					bool sbnd2 )
		{
			istatus istats( type, prof, bnd1, bnd2, sbnd1, sbnd2 ) ;
			if ( setUNDO[ task ] )
			{
				int lvl ;
				if ( Global_status[ task ].empty() )
				{
					lvl = 0 ;
				}
				else if ( Global_Undo[ task ].empty() )
				{
					lvl = 1 ;
				}
				else
				{
					lvl = get_Global_Undo_level( task ) ;
				}
				istats.is_level = lvl ;
				if ( Global_status[ task ].empty() )
				{
					Global_status[ task ].push( istats ) ;
				}
				else if ( Global_status[ task ].top() != istats )
				{
					Global_status[ task ].push( istats ) ;
				}
				else
				{
					lvl = 0 ;
				}
				if ( lvl > 0 && ( Global_Undo[ task ].empty() || Global_Undo[ task ].top() != lvl ) )
				{
					Global_Undo[ task ].push( lvl ) ;
				}
			}
			else if ( Global_status[ task ].empty() )
			{
				Global_status[ task ].push( istats ) ;
			}
			else
			{
				Global_status[ task ].top() = istats ;
			}
		}

		static void status_update( int task,
					   int bnd1,
					   int bnd2,
					   bool sbnd1,
					   bool sbnd2 )
		{
			if ( !Global_status[ task ].empty() )
			{
				istatus& istats = Global_status[ task ].top() ;
				istats.is_bnd1  = bnd1 ;
				istats.is_bnd2  = bnd2 ;
				istats.is_sbnd1 = sbnd1 ;
				istats.is_sbnd2 = sbnd2 ;
			}
		}

		static SS_TYPE status_get_type( int task )
		{
			return Global_status[ task ].top().is_type ;
		}

		static const profile& status_get_profile( int task )
		{
			return Global_status[ task ].top().is_prof ;
		}

		static int status_get_LeftBnd( int task )
		{
			return Global_status[ task ].top().is_bnd1 ;
		}

		static int status_get_RightBnd( int task )
		{
			return Global_status[ task ].top().is_bnd2 ;
		}

		static bool status_get_LeftBndSet( int task )
		{
			return Global_status[ task ].top().is_sbnd1 ;
		}

		static bool status_get_RightBndSet( int task )
		{
			return Global_status[ task ].top().is_sbnd2 ;
		}

		static void status_undo( int task )
		{
			if ( setUNDO[ task ] )
			{
				Global_status_redo[ task ].push( Global_status[ task ].top() ) ;
				Global_status[ task ].pop() ;
				Redo_other[ task ] = true  ;
			}
		}

		static void status_redo( int task )
		{
			if ( setUNDO[ task ] )
			{
				Global_status[ task ].push( Global_status_redo[ task ].top() ) ;
				Global_status_redo[ task ].pop() ;
				Redo_other[ task ] = true ;
			}
		}

		static int get_status_level( int task )
		{
			if ( setUNDO[ task ] )
			{
				return Global_status[ task ].top().is_level ;
			}
			return 0 ;
		}

		static int get_status_redo_level( int task )
		{
			if ( setUNDO[ task ] )
			{
				if ( !Global_status_redo[ task ].empty() )
				{
					return Global_status_redo[ task ].top().is_level ;
				}
			}
			return 0 ;
		}

		static void remove_redo_status( int task )
		{
			while ( !Global_status_redo[ task ].empty() )
			{
				Global_status_redo[ task ].pop() ;
			}
		}

		static void clear_status( int task )
		{
			while ( !Global_status[ task ].empty() )
			{
				Global_status[ task ].pop() ;
			}
			while ( !Global_status_redo[ task ].empty() )
			{
				Global_status_redo[ task ].pop() ;
			}
		}

		explicit iline( int taskid,
				LN_TYPE type,
				int lnumSize ) : iline( taskid, type )
		{
			set_file_insert( lnumSize ) ;
		}

		explicit iline( int taskid,
				CA_TYPE catype ) : iline( taskid, LN_MSG )
		{
			il_caution = catype ;
		}

		explicit iline( int taskid,
				LN_TYPE type = LN_INVCMD )
		{
			il_next     = nullptr ;
			il_prev     = nullptr ;
			il_seqn     = 0       ;
			il_type     = type    ;
			il_status   = LS_NONE ;
			il_caution  = CA_NONE ;
			il_hex      = false  ;
			il_mark     = false  ;
			il_deleted  = false  ;
			il_prof     = false  ;
			il_lnumreq  = false  ;
			il_lcc      = ""     ;
			il_profln   = 0      ;
			il_taskid   = taskid ;
			il_Shadow   = ""     ;
			il_vShadow  = false  ;
			il_wShadow  = false  ;
			il_nsect    = false  ;
			il_xclud.push( iexcl() ) ;
		}

		explicit iline()
		{
			il_next = nullptr ;
			il_prev = nullptr ;
		}

		iline*   il_prev ;
		iline*   il_next ;
		uint     il_seqn ;
		LN_TYPE  il_type ;
		LS_TYPE  il_status  ;
		CA_TYPE  il_caution ;
		bool     il_hex     ;
		bool     il_mark    ;
		bool     il_deleted ;
		bool     il_prof    ;
		bool     il_lnumreq ;
		string   il_lcc     ;
		int      il_profln  ;
		int      il_taskid  ;
		string   il_Shadow  ;
		bool     il_vShadow ;
		bool     il_wShadow ;
		bool     il_nsect   ;
		stack  <icond>  il_cond ;
		stack  <icond>  il_cond_redo ;
		stack  <iexcl>  il_xclud ;
		stack  <iexcl>  il_xclud_redo ;
		stack  <ilabel> il_label ;
		stack  <ilabel> il_label_redo ;
		stack  <idata>  il_idata ;
		stack  <idata>  il_idata_redo ;

		void set_file_insert( int lnumSize )
		{
			if ( il_type == LN_FILE && lnumSize > 0 )
			{
				file_inserts[ il_taskid ] = true ;
				il_lnumreq = true ;
			}
		}

		bool is_file() const
		{
			return ( il_type == LN_FILE ) ;
		}

		bool is_valid_file() const
		{
			return ( !il_deleted && il_type == LN_FILE ) ;
		}

		bool not_valid_file() const
		{
			return ( il_deleted || il_type != LN_FILE ) ;
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
			return ( !il_deleted && il_type == LN_NOTE ) ;
		}

		bool is_info() const
		{
			return ( !il_deleted && il_type == LN_INFO ) ;
		}

		bool is_msg() const
		{
			return ( !il_deleted && il_type == LN_MSG ) ;
		}

		bool is_col() const
		{
			return ( !il_deleted && il_type == LN_COL ) ;
		}

		bool is_prof() const
		{
			return ( !il_deleted && il_prof ) ;
		}

		bool is_prof_prof() const
		{
			return ( !il_deleted && il_prof && il_type == LN_PROF ) ;
		}

		bool is_caution( CA_TYPE catype ) const
		{
			return ( !il_deleted && il_type == LN_MSG && il_caution == catype ) ;
		}

		bool is_tabs() const
		{
			return ( !il_deleted && il_type == LN_TABS ) ;
		}

		bool is_mask() const
		{
			return ( !il_deleted && il_type == LN_MASK ) ;
		}

		bool is_bnds() const
		{
			return ( !il_deleted && il_type == LN_BNDS ) ;
		}

		bool is_isrt() const
		{
			return ( il_type == LN_ISRT ) ;
		}

		bool has_condition() const
		{
			return ( !il_cond.empty() ) ;
		}

		LS_TYPE get_condition() const
		{
			return ( il_cond.empty() ? LS_NONE : il_cond.top().ic_status ) ;
		}

		bool is_chng() const
		{
			return ( !il_cond.empty() && il_cond.top().ic_status == LS_CHNG ) ;
		}

		bool is_error() const
		{
			return ( !il_cond.empty() && il_cond.top().ic_status == LS_ERROR ) ;
		}

		bool is_undo() const
		{
			return ( il_status == LS_UNDO ) ;
		}

		bool is_redo() const
		{
			return ( il_status == LS_REDO ) ;
		}

		bool marked() const
		{
			return il_mark ;
		}

		bool is_excluded() const
		{
			return ( il_xclud.top().ix_excl ) ;
		}

		bool is_not_excluded() const
		{
			return ( !il_xclud.top().ix_excl ) ;
		}

		void set_excluded( bool b,
				   int lvl )
		{
			if ( il_xclud.top().ix_excl != b )
			{
				if ( setUNDO[ il_taskid ] )
				{
					il_xclud.push( iexcl( b, lvl ) ) ;
					if ( Global_Undo[ il_taskid ].empty() ||
					     Global_Undo[ il_taskid ].top() != lvl )
					{
						Global_Undo[ il_taskid ].push( lvl ) ;
					}
					changed_xclud[ il_taskid ] = true ;
				}
				else
				{
					il_xclud.top().ix_excl = b ;
				}
			}
		}

		void set_excluded( int lvl )
		{
			set_excluded( true, lvl ) ;
		}

		void set_unexcluded( int lvl )
		{
			set_excluded( false, lvl ) ;
		}

		void toggle_excluded( int lvl )
		{
			set_excluded( !is_excluded(), lvl ) ;
		}

		void resetFilePrefix( int lvl )
		{
			il_status  = LS_NONE ;
			il_mark    = false ;
			set_excluded( false, lvl ) ;
			removeCondition() ;
		}

		void resetFileStatus()
		{
			il_status = LS_NONE ;
			removeCondition() ;
		}

		void clrChngStatus()
		{
			if ( is_chng() )
			{
				il_cond.pop() ;
			}
		}

		void clrErrorStatus()
		{
			if ( is_error() )
			{
				il_cond.pop() ;
			}
		}

		void clrUndoStatus()
		{
			if ( il_status == LS_UNDO ) { il_status = LS_NONE ; }
		}

		void clrRedoStatus()
		{
			if ( il_status == LS_REDO ) { il_status = LS_NONE ; }
		}

		void setChngStatus( int lvl )
		{
			set_condition( LS_CHNG, lvl ) ;
		}

		void setErrorStatus( int lvl )
		{
			set_condition( LS_ERROR, lvl ) ;
		}

		void set_condition( LS_TYPE t,
				    int lvl )
		{
			if ( !il_cond.empty() && il_cond.top().ic_level  == lvl &&
						 il_cond.top().ic_status == t )
			{
				return ;
			}
			if ( il_cond.empty() && t == LS_NONE )
			{
				return ;
			}
			if ( setUNDO[ il_taskid ] )
			{
				il_cond.push( icond( t, lvl ) ) ;
				if ( Global_Undo[ il_taskid ].empty() ||
					Global_Undo[ il_taskid ].top() != lvl )
				{
					Global_Undo[ il_taskid ].push( lvl ) ;
				}
				changed_icond[ il_taskid ] = true ;
			}
			else
			{
				if ( il_cond.empty() )
				{
					il_cond.push( icond( t, lvl ) ) ;
				}
				else
				{
					il_cond.top().ic_status = t ;
				}
			}
		}

		void removeCondition()
		{
			while ( !il_cond.empty() )
			{
				il_cond.pop() ;
			}
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

		bool is_special() const
		{
			return ( is_note() || is_prof() || is_col()  || is_bnds() ||
				 is_mask() || is_tabs() || is_info() || is_msg()  ) ;
		}

		void clearLcc()
		{
			il_lcc = "" ;
		}

		void remove_back()
		{
			if ( is_valid_file() )
			{
				il_idata.top().id_data.pop_back() ;
			}
		}

		bool has_invChars()
		{
			if ( is_valid_file() )
			{
				if ( any_of( il_idata.top().id_data.begin(), il_idata.top().id_data.end(),
					[]( char c )
					{
						return ( !isprint( c ) ) ;
					} ) )
				{
					return true ;
				}
			}
			return false ;
		}

		bool has_label( int lvl=0 ) const
		{
			if ( il_deleted || il_label.size() == 0 ) { return false ; }

			for ( int i = lvl ; i >= 0 ; --i )
			{
				if ( il_label.top().iy_label.count( i ) > 0 ) { return true ; }
			}
			return false ;
		}

		bool specialLabel( int lvl=0 )
		{
			if ( il_deleted || il_label.size() == 0 ) { return false ; }

			if ( il_label.top().iy_label.count( lvl ) > 0 )
			{
				const string& t = il_label.top().iy_label[ lvl ] ;
				return ( t.size() == 6 && t.compare( 0, 2, ".O" ) == 0 ) ;
			}
			return false ;
		}

		string get_label( int lvl=0 )
		{
			if ( il_deleted || il_label.size() == 0 ) { return "" ; }

			if ( il_label.top().iy_label.count( lvl ) > 0 )
			{
				return il_label.top().iy_label[ lvl ] ;
			}
			return "" ;
		}

		void getLabelInfo( int lvl,
				   string& label,
				   int& foundLvl )
		{
			label    = "" ;
			foundLvl = 0  ;

			if ( il_deleted || il_label.size() == 0 ) { return ; }

			for ( int i = lvl ; i >= 0 ; --i )
			{
				if ( il_label.top().iy_label.count( i ) > 0 )
				{
					label    = il_label.top().iy_label[ i ] ;
					foundLvl = i ;
					break ;
				}
			}
			return ;
		}

		int setLabel( const string& s,
			      int level,
			      int lvl = 0 )
		{
			//
			// Set line label for level.
			//
			// rc = 0 Normal completion.
			// rc = 4 Line had a label at this level.
			//
			int rc = 0 ;

			ilabel t ;

			if ( il_label.size() > 0 )
			{
				if ( il_label.top().iy_label.count( lvl ) > 0 )
				{
					if ( il_label.top().iy_label[ lvl ] == s )
					{
						return 4 ;
					}
					rc = 4 ;
				}
				if ( setUNDO[ il_taskid ] )
				{
					t = il_label.top() ;
				}
			}
			if ( setUNDO[ il_taskid ] )
			{
				t.iy_label[ lvl ] = s ;
				t.iy_level = level ;
				if ( il_label.empty() || il_label.top().iy_level != level )
				{
					il_label.push( t ) ;
				}
				else
				{
					il_label.top() = t ;
				}
				if ( Global_Undo[ il_taskid ].empty() ||
				     Global_Undo[ il_taskid ].top() != level )
				{
					Global_Undo[ il_taskid ].push( level ) ;
				}
				changed_ilabel[ il_taskid ] = true ;
			}
			else if ( il_label.size() > 0 )
			{
				il_label.top().iy_label[ lvl ] = s ;
			}
			else
			{
				t.iy_label[ lvl ] = s ;
				il_label.push( t ) ;
			}
			return rc ;
		}

		void clearLabel( int level,
				 int lvl = 0 )
		{
			ilabel t ;
			if ( il_deleted ||
			     il_label.size() == 0 ||
			     il_label.top().iy_label.count( lvl ) == 0 )
			{
				return ;
			}
			if ( setUNDO[ il_taskid ] )
			{
				t = il_label.top() ;
				t.iy_label.erase( lvl ) ;
				t.iy_level = level ;
				if ( il_label.top().iy_level != level )
				{
					il_label.push( t ) ;
				}
				else
				{
					il_label.top() = t ;
				}
				if ( Global_Undo[ il_taskid ].empty() ||
				     Global_Undo[ il_taskid ].top() != level )
				{
					Global_Undo[ il_taskid ].push( level ) ;
				}
				changed_ilabel[ il_taskid ] = true ;
			}
			else
			{
				il_label.top().iy_label.erase( lvl ) ;
			}
			if ( il_label.size() == 1 && il_label.top().empty() )
			{
				il_label.pop() ;
			}
			return ;
		}

		bool clearLabel( const string& s,
				 int level,
				 int lvl=0 )
		{
			if ( il_deleted ||
			     il_label.size() == 0 ||
			     il_label.top().iy_label.count( lvl ) == 0 ||
			     il_label.top().iy_label[ lvl ] != s )
			{
				return false ;
			}
			clearLabel( level, lvl ) ;
			return true ;
		}

		void clearSpecialLabel( int level,
					int lvl=0 )
		{
			if ( il_deleted ||
			     il_label.size() == 0 ||
			     il_label.top().iy_label.count( lvl )  == 0 ||
			     il_label.top().iy_label[ lvl ].size() != 6 ||
			     il_label.top().iy_label[ lvl ].compare( 0, 2, ".O" ) != 0 )
			{
				return ;
			}
			clearLabel( level, lvl ) ;
		}

		bool compareLabel( const string& s,
				   int lvl )
		{
			if ( il_deleted ||
			     il_label.size() == 0 ||
			     il_label.top().iy_label.count( lvl ) == 0 ||
			     il_label.top().iy_label[ lvl ] != s )
			{
				return false ;
			}
			return true ;
		}

		bool put_idata( size_t len1,
				size_t pos,
				const string& s,
				int level,
				uint32_t status = 0,
				bool comp = true )
		{
			//
			// Update idata.
			// String contains only the data without the line numbers - use existing numbers.
			// Return true if a valid file.
			//

			string temp ;

			if ( len1 > 0 )
			{
				if ( pos > 0 )
				{
					temp = get_linenum1( len1 ) + s + get_linenum2( pos ) ;
				}
				else
				{
					temp = get_linenum1( len1 ) + s ;
				}
			}
			else if ( pos > 0 )
			{
				temp = s + get_linenum2( pos ) ;
			}
			else
			{
				temp = s ;
			}

			return put_idata( temp, level, status, comp ) ;
		}

		bool put_idata( const string& s,
				int level,
				uint32_t status = 0,
				bool comp = true )
		{
			//
			// Update idata.
			// Return true if a valid file.
			//

			if ( comp && !il_idata.empty() && s == il_idata.top().id_data )
			{
				return false ;
			}
			if ( is_file() && !is_File_save( il_taskid ) )
			{
				copyFile( il_taskid ) ;
			}
			if ( is_file() )
			{
				file_changed[ il_taskid ] = true ;
			}
			if ( !setUNDO[ il_taskid ] )
			{
				if ( il_idata.empty() ) { il_idata.push( idata( s, status ) ) ; }
				else                    { il_idata.top() = idata( s, status ) ; }
			}
			else
			{
				if ( il_idata.empty() )
				{
					il_idata.push( idata( s, status, level ) ) ;
				}
				else if ( il_idata.top().id_level != level )
				{
					il_idata.push( idata( s, status, level ) | il_idata.top() ) ;
				}
				else
				{
					il_idata.top() = idata( s, status, level ) ;
				}
				if ( Global_Undo[ il_taskid ].empty() ||
				     Global_Undo[ il_taskid ].top() != level )
				{
					Global_Undo[ il_taskid ].push( level ) ;
				}
				if ( is_file() )
				{
					set_Global_File_level( il_taskid) ;
				}
			}
			il_vShadow = false ;
			if ( is_valid_file() )
			{
				clear_Global_Redo( il_taskid ) ;
				remove_redo_idata() ;
			}

			return is_valid_file() ;
		}

		bool put_idata( const string& s,
				uint32_t status = 0 )
		{
			if ( il_idata.empty() )
			{
				il_idata.push( idata( s, status ) ) ;
			}
			else
			{
				if ( s == il_idata.top().id_data )
				{
					return false ;
				}
				il_idata.top() = idata( s, status, il_idata.top().id_level ) ;
			}
			il_vShadow = false ;

			return is_valid_file() ;
		}

		void update_lnummod( size_t len1,
				     size_t pos,
				     const string& mod )
		{
			if ( len1 == 8 || pos > 0 )
			{
				il_idata.top().id_data.replace( pos+6, 2, mod ) ;
			}
		}

		void set_idata_minsize( size_t m )
		{
			if ( il_idata.top().id_data.size() < m )
			{
				il_idata.top().id_data.resize( m, ' ' ) ;
			}
		}

		bool set_idata_upper( int level,
				      uint32_t status = 0 )
		{
			if ( any_of( il_idata.top().id_data.begin(), il_idata.top().id_data.end(),
				   []( char c )
				   {
					return ( islower( c ) ) ;
				   } ) )
			{
				put_idata( upper( il_idata.top().id_data ), level, status ) ;
				return is_file() ;
			}
			return false ;
		}

		bool set_idata_upper( int level,
				      int l,
				      int r,
				      uint32_t status = 0 )
		{
			string t = il_idata.top().id_data ;
			iupper( t, l-1, (r == 0) ? t.size()-1 : r-1 ) ;
			if ( t != il_idata.top().id_data )
			{
				put_idata( t, level, status ) ;
				return is_file() ;
			}
			return false ;
		}

		bool set_idata_lower( int level,
				      int l,
				      int r,
				      uint32_t status = 0 )
		{
			string t = il_idata.top().id_data ;
			ilower( t, l-1, (r == 0) ? t.size()-1 : r-1 ) ;
			if ( t != il_idata.top().id_data )
			{
				put_idata( t, level ) ;
				return is_file() ;
			}
			return false ;
		}

		bool set_idata_trim( int level,
				     uint32_t status = 0 )
		{
			if ( il_idata.top().id_data.size() > 0 && il_idata.top().id_data.back() == ' ' )
			{
				put_idata( strip( il_idata.top().id_data, 'T', ' ' ), level, status ) ;
				return is_file() ;
			}
			return false ;
		}

		void set_idata_trim()
		{
			trim_right( il_idata.top().id_data ) ;
		}

		bool add_linenum( int i,
				  size_t len1,
				  size_t len2,
				  size_t pos,
				  const string& mod,
				  int level )
		{
			//
			// Add line number and mod level.
			//
			// Update line status with ID_RENUM.
			//
			// Return true if data changed.
			//
			// len2 will only be set for fixed length records.
			//

			string t0 ;
			string t1 ;
			string t2 ;

			if ( len1 == 6 )
			{
				t1 = d2ds( i, 6 ) ;
			}
			else if ( len1 == 8 )
			{
				t1 = d2ds( i, 6 ) + mod ;
			}
			if ( len2 == 8 )
			{
				t2 = d2ds( i, 6 ) + mod ;
			}

			bool changed = false ;

			il_lnumreq = false ;
			if ( len1 > 0 && il_idata.top().id_data.size() < len1 )
			{
				put_idata( t1, level, ID_RENUM ) ;
				return true ;
			}

			t0 = il_idata.top().id_data ;
			if ( len1 > 0 && t0.compare( 0, len1, t1 ) != 0 )
			{
				t0.replace( 0, len1, t1 ) ;
				changed = true ;
			}

			if ( len2 > 0 && t0.compare( pos, len2, t2 ) != 0 )
			{
				t0.replace( pos, len2, t2 ) ;
				changed = true ;
			}

			if ( changed )
			{
				put_idata( t0, level, ID_RENUM ) ;
			}

			return changed ;
		}

		bool add_linenum( size_t len1,
				  size_t len2,
				  size_t pos,
				  size_t& pseq1,
				  size_t& pseq2,
				  int level )
		{
			//
			// Check and add line number if missing or out of sequence.
			// Don't add mod level - done later by upd_modlevel() method.
			//
			// pseq1 is the previous seqence number of lhs STD/COBOL number.
			// pseq2 is the previous seqence number of rhs STD number.
			//
			// Update line status with ID_RENUM.
			//
			// RETURN:  true if any changes made to the data.
			//

			string lnum ;

			string t0 ;
			string t1 ;
			string t2 ;

			uint i ;

			bool changed1 = false ;
			bool changed2 = false ;

			t0 = il_idata.top().id_data ;

			il_lnumreq = false ;

			if ( len1 > 0 )
			{
				lnum = get_linenum1( len1 ) ;
				if ( lnum == "" )
				{
					++pseq1 ;
					changed1 = true ;
				}
				else
				{
					if ( len1 == 8 )
					{
						lnum.erase( 6, 2 ) ;
					}
					i = ds2d( lnum ) ;
					if ( i <= pseq1 )
					{
						++pseq1 ;
						changed1 = true ;
					}
					else
					{
						pseq1 = i ;
					}
				}
				if ( changed1 )
				{
					t1 = d2ds( pseq1, 6 ) ;
					if ( len1 == 8 ) { t1 = t1 + "  " ; }
					if ( il_idata.top().id_data.size() < len1 )
					{
						t0 = t1 ;
					}
					else
					{
						t0.replace( 0, len1, t1 ) ;
					}
				}
			}
			if ( len2 > 0 )
			{
				lnum = get_linenum2( pos ) ;
				if ( lnum == "" )
				{
					++pseq2 ;
					changed2 = true ;
				}
				else
				{
					lnum.erase( 6, 2 ) ;
					i = ds2d( lnum ) ;
					if ( i <= pseq2 )
					{
						++pseq2 ;
						changed2 = true ;
					}
					else
					{
						pseq2 = i ;
					}
				}
				if ( changed2 )
				{
					t2 = d2ds( pseq2, 6 ) ;
					if ( il_idata.top().id_data.size() < ( pos + len2 ) )
					{
						t0.resize( ( pos + len2 ), ' ' ) ;
					}
					t0.replace( pos, 6, t2 ) ;
				}
			}

			if ( changed1 || changed2 )
			{
				put_idata( t0, level, ID_RENUM ) ;
			}

			return ( changed1 || changed2 ) ;
		}

		void upd_modlevel( size_t pos )
		{
			//
			// Update mod level to 01 in-place (done after NUM ON).
			//

			il_idata.top().id_data.replace( pos+6, 2, "01" ) ;
		}

		bool upd_linenum( int i,
				  size_t len1,
				  size_t len2,
				  size_t pos,
				  const string& mod,
				  int level )
		{
			//
			// Update the line number, leaving the mod level unchanged.
			// Add any missing line numbers by calling add_linenum().
			//
			// Update line status with ID_RENUM.
			//
			// Return true if line updated.
			//

			string t1 ;
			string lnum ;

			if ( len2 > 0 )
			{
				t1 = get_linenum2( pos ) ;
				if ( t1 == "" ) { return add_linenum( i, len1, len2, pos, mod, level ) ; }
			}
			if ( len1 > 0 )
			{
				t1 = get_linenum1( len1 ) ;
				if ( t1 == "" ) { return add_linenum( i, len1, len2, pos, mod, level ) ; }
			}

			bool changed = false ;
			il_lnumreq   = false ;

			lnum = d2ds( i, 6 ) ;

			t1 = il_idata.top().id_data ;

			if ( len1 > 0 && t1.compare( 0, 6, lnum ) != 0 )
			{
				t1.replace( 0, 6, lnum ) ;
				changed = true ;
			}
			if ( len2 > 0 && t1.compare( pos, 6, lnum ) != 0 )
			{
				t1.replace( pos, 6, lnum ) ;
				changed = true ;
			}
			if ( changed )
			{
				put_idata( t1, level, ID_RENUM ) ;
			}

			return changed ;
		}

		int get_linenum( size_t len1,
				 size_t len2,
				 size_t pos ) const
		{
			//
			// Return the line number giving preference to STD, left then right.
			// Don't return the mod level - just the 6 byte number.
			//
			// Return -1 if no valid number found.
			//

			if ( len2 > 0 )
			{
				const string& data = il_idata.top().id_data ;
				if ( data.size() >= ( pos + len2 ) )
				{
					if ( datatype( data.substr( pos, 6 ), 'W' ) )
					{
						return ds2d( data.substr( pos, 6 ) ) ;
					}
				}
			}

			if ( len1 > 0 )
			{
				const string& data = il_idata.top().id_data ;
				if ( data.size() >= len1 )
				{
					if ( datatype( data.substr( 0, 6 ), 'W' ) )
					{
						return ds2d( data.substr( 0, 6 ) ) ;
					}
				}
			}

			return -1 ;
		}

		int get_linenum( size_t len2,
				 size_t pos ) const
		{
			//
			// Return the full STD line number including the update level.
			//
			// Return -1 if no valid number found.
			//

			const string& data = il_idata.top().id_data ;
			if ( data.size() >= ( pos + len2 ) )
			{
				if ( datatype( data.substr( pos, 8 ), 'W' ) )
				{
					return ds2d( data.substr( pos, 8 ) ) ;
				}
			}

			return -1 ;
		}

		string get_linenum1( size_t len1 )
		{
			//
			// Return a valid left hand side line number.  Blank if invalid.
			// len1 is either 8 (STD) or 6 (CBL).
			//

			string t ;

			if ( il_idata.top().id_data.size() >= len1 )
			{
				t = il_idata.top().id_data.substr( 0, len1 ) ;
				return ( datatype( t, 'W' ) ) ? t : "" ;
			}

			return "" ;
		}

		string get_linenum2( size_t pos )
		{
			//
			// Return a valid right hand side line number.  Blank if invalid.
			// len2 is always 8 (STD).
			//

			string t ;

			if ( il_idata.top().id_data.size() >= ( pos + 8 ) )
			{
				t = il_idata.top().id_data.substr( pos, 8 ) ;
				return ( datatype( t, 'W' ) ) ? t : "" ;
			}

			return "" ;
		}

		string get_linenum_str( size_t len1,
					size_t len2,
					size_t pos )
		{
			//
			// Return the line number as a string, giving preference to STD, left then right.
			// Don't return the mod level - just the 6 byte number.
			//

			int i = get_linenum( len1, len2, pos ) ;

			return ( i > -1 ) ? d2ds( i, 6 ) : "??????" ;
		}

		void remove_linenum( size_t len1,
				     size_t pos,
				     int level )
		{
			//
			// Overwrite the line number(s) with blanks.
			// Set file status to ID_RENUM.
			//

			string t1 = il_idata.top().id_data ;
			if ( len1 > 0 )
			{
				t1.replace( 0, len1, len1, ' ' ) ;
			}
			if ( pos > 0 )
			{
				t1.replace( pos, 8, 8, ' ' ) ;
			}
			put_idata( t1, level, ID_RENUM ) ;
		}

		void set_deleted( int level )
		{
			if ( !is_tod_or_bod() && !il_deleted )
			{
				if ( setUNDO[ il_taskid ] )
				{
					put_idata( "", level, 0, false ) ;
					il_idata.top().id_delete = true ;
					il_deleted               = true ;
				}
				else
				{
					il_deleted = true ;
				}
			}
		}

		void convert_to_file( int level,
				      size_t lnumSize )
		{
			//
			// Called when ISRT line overwritten to create a new file line.
			//
			// Add file status ID_ISRT | ID_OWRITE.
			//
			il_type = LN_FILE ;
			il_idata.top().add_status( ID_ISRT | ID_OWRITE ) ;
			if ( !is_File_save( il_taskid ) )
			{
				copyFile( il_taskid ) ;
			}
			file_changed[ il_taskid ] = true ;
			file_inserts[ il_taskid ] = true ;
			if ( lnumSize > 0 )
			{
				il_lnumreq = true ;
			}
			if ( setUNDO[ il_taskid ] )
			{
				il_idata.top().id_level = level ;
				if ( Global_Undo[ il_taskid ].empty() ||
				     Global_Undo[ il_taskid ].top() != level )
				{
					Global_Undo[ il_taskid ].push( level ) ;
				}
				set_Global_File_level( il_taskid ) ;
				clear_Global_Redo( il_taskid ) ;
				remove_redo_idata() ;
			}
		}

		const string& get_idata() const
		{
			return il_idata.top().id_data ;
		}

		string get_idata( size_t len1,
				  size_t pos ) const
		{
			//
			// Return the data section without line numbers (STD and COBOL).
			//
			if ( len1 == 0 )
			{
				if ( pos == 0 )
				{
					return il_idata.top().id_data ;
				}
				else
				{
					return il_idata.top().id_data.substr( 0, pos ) ;
				}
			}
			else if ( pos == 0 )
			{
				return il_idata.top().id_data.substr( len1 ) ;
			}
			else
			{
				return il_idata.top().id_data.substr( len1, pos - len1 ) ;
			}
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

		string* get_idata_ptr()
		{
			return &(il_idata.top().id_data) ;
		}

		bool idata_is_empty() const
		{
			return il_idata.empty() ;
		}

		void undo_idata()
		{
			if ( setUNDO[ il_taskid ] )
			{
				if ( il_idata.top().id_delete )
				{
					il_deleted = false ;
				}
				il_idata_redo.push( il_idata.top() ) ;
				il_idata.pop() ;
				if ( il_idata.empty() )
				{
					il_deleted = true ;
				}
				Redo_data[ il_taskid ] = true ;
			}
		}

		void redo_idata()
		{
			if ( setUNDO[ il_taskid ] )
			{
				il_deleted = il_idata_redo.top().id_delete ;
				il_idata.push( il_idata_redo.top() ) ;
				il_idata_redo.pop() ;
			}
		}

		void undo_icond()
		{
			if ( setUNDO[ il_taskid ] )
			{
				il_cond_redo.push( il_cond.top() ) ;
				il_cond.pop() ;
				Redo_other[ il_taskid ] = true ;
			}
		}

		void redo_icond()
		{
			if ( setUNDO[ il_taskid ] )
			{
				il_cond.push( il_cond_redo.top() ) ;
				il_cond_redo.pop() ;
			}
		}

		void undo_ilabel()
		{
			if ( setUNDO[ il_taskid ] )
			{
				il_label_redo.push( il_label.top() ) ;
				il_label.pop() ;
				Redo_other[ il_taskid ] = true ;
			}
		}

		void redo_ilabel()
		{
			if ( setUNDO[ il_taskid ] )
			{
				il_label.push( il_label_redo.top() ) ;
				il_label_redo.pop() ;
			}
		}

		void undo_iexcl()
		{
			if ( setUNDO[ il_taskid ] )
			{
				il_xclud_redo.push( il_xclud.top() ) ;
				il_xclud.pop() ;
				Redo_other[ il_taskid ] = true ;
			}
		}

		void redo_iexcl()
		{
			if ( setUNDO[ il_taskid ] )
			{
				il_xclud.push( il_xclud_redo.top() ) ;
				il_xclud_redo.pop() ;
			}
		}

		void flatten_idata()
		{
			idata d( il_idata.top(), 0 ) ;
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

		void flatten_cond()
		{
			if ( il_cond.size() > 0 )
			{
				icond c    = il_cond.top() ;
				c.ic_level = 0 ;
				while ( !il_cond.empty() )
				{
					il_cond.pop() ;
				}
				while ( !il_cond_redo.empty() )
				{
					il_cond_redo.pop() ;
				}
				il_cond.push( c ) ;
			}
		}

		void flatten_label()
		{
			if ( il_label.size() > 0 )
			{
				ilabel l   = il_label.top() ;
				l.iy_level = 0 ;
				while ( !il_label.empty() )
				{
					il_label.pop() ;
				}
				while ( !il_label_redo.empty() )
				{
					il_label_redo.pop() ;
				}
				il_label.push( l ) ;
			}
		}

		void flatten_excl()
		{
			iexcl x    = il_xclud.top() ;
			x.ix_level = 0 ;
			while ( !il_xclud.empty() )
			{
				il_xclud.pop() ;
			}
			while ( !il_xclud_redo.empty() )
			{
				il_xclud_redo.pop() ;
			}
			il_xclud.push( x ) ;
		}

		void remove_redo_idata()
		{
			while ( !il_idata_redo.empty() )
			{
				il_idata_redo.pop() ;
			}
		}

		void remove_redo_cond()
		{
			while ( !il_cond_redo.empty() )
			{
				il_cond_redo.pop() ;
			}
		}

		void remove_redo_label()
		{
			while ( !il_label_redo.empty() )
			{
				il_label_redo.pop() ;
			}
		}

		void remove_redo_excl()
		{
			while ( !il_xclud_redo.empty() )
			{
				il_xclud_redo.pop() ;
			}
		}

		int get_idata_level() const
		{
			return ( il_idata.empty() ) ? 0 :
				 il_idata.top().id_level ;
		}

		int get_idata_Redo_level() const
		{
			return ( il_idata_redo.empty() ) ? 0 :
				 il_idata_redo.top().id_level ;
		}

		int get_icond_level() const
		{
			return ( il_cond.empty() ) ? 0 :
				 il_cond.top().ic_level ;
		}

		int get_icond_Redo_level() const
		{
			return ( il_cond_redo.empty() ) ? 0 :
				 il_cond_redo.top().ic_level ;
		}

		int get_ilabel_level() const
		{
			return ( il_label.empty() ) ? 0 :
				 il_label.top().iy_level ;
		}

		int get_ilabel_Redo_level() const
		{
			return ( il_label_redo.empty() ) ? 0 :
				 il_label_redo.top().iy_level ;
		}

		int get_iexcl_level() const
		{
			return il_xclud.top().ix_level ;
		}

		int get_iexcl_Redo_level() const
		{
			return ( il_xclud_redo.empty() ) ? 0 :
				 il_xclud_redo.top().ix_level ;
		}

		uint32_t get_idstatus() const
		{
			return ( il_idata.empty() ) ? 0 : il_idata.top().id_status ;
		}


	friend class Data ;
	friend class line_data ;
	friend class pedit01 ;
} ;


class Data
{
	public:
		Data()
		{
			ln_bottom = nullptr ;
			ln_end    = new iline() ;
			ln_begin  = ln_end ;
			reseq     = false ;
			tot       = 0 ;
		}

		~Data()
		{
			for ( iline* i = ln_end->il_prev ; i ; i = i->il_prev )
			{
				delete i->il_next ;
			}
			delete ln_begin ;
		}

	struct Iterator
	{
		using iterator_category = std::bidirectional_iterator_tag ;
		using difference_type   = std::ptrdiff_t ;
		using value_type        = iline ;
		using pointer           = iline* ;
		using reference         = iline& ;

		Iterator( pointer ptr ) : ln_current( ptr ) {}

		reference operator * () const { return *ln_current ; }
		pointer   operator -> ()   { return ln_current ; }
		Iterator& operator ++ ()   { ln_current = ln_current->il_next ; return *this ; }
		Iterator operator ++ (int) { Iterator tmp = *this ; ++(*this) ; return tmp ; }
		Iterator& operator -- ()   { ln_current = ln_current->il_prev ; return *this ; }
		Iterator operator -- (int) { Iterator tmp = *this ; --(*this) ; return tmp ; }
		friend bool operator == ( const Iterator& a, const Iterator& b) { return a.ln_current == b.ln_current ; }
		friend bool operator != ( const Iterator& a, const Iterator& b) { return a.ln_current != b.ln_current ; }

		Iterator& operator += ( int i )
		{
			for ( ; i > 0 ; --i )
			{
				ln_current = ln_current->il_next ;
			}
			return *this ;
		}

		Iterator& operator -= ( int i )
		{
			for ( ; i > 0 ; --i )
			{
				ln_current = ln_current->il_prev ;
			}
			return *this ;
		}

		const Iterator operator + ( const int i )
		{
			Iterator out( *this ) ;
			out += i ;
			return out ;
		}

		const Iterator operator - ( const int i )
		{
			Iterator out( *this ) ;
			out -= i ;
			return out ;
		}

		private:
			pointer ln_current ;


	} ;

	Iterator begin() { return Iterator( ln_begin ) ; }
	Iterator end()   { return Iterator( ln_end )   ; }

	iline* top()     { return ln_begin  ; }
	iline* bottom()  { return ln_bottom ; }

	iline* operator [] ( int index ) const
	{
		if ( index < tot && index >= 0 )
		{
			iline* elem ;
			for ( iline* it = ln_begin ; it != ln_end && index >= 0 ; it = it->il_next, --index )
			{
				elem = it ;
			}
			if ( index == -1 )
			{
				return elem ;
			}
		}
		return nullptr ;
	}

	iline* push_back( iline* ln,
			  const string& s = "" )
	{
		//
		// Only used for the initial load.  idata set with level = 0, status = ID_FILE
		//

		ln->put_idata( s, 0, ID_FILE ) ;

		push_backchain( ln ) ;

		++tot ;

		return ln ;
	}

	iline* insert( iline* pos,
		       iline* ln )
	{
		//
		// Add ln before position pos.  Return newly inserted address.
		//

		assert( pos != ln_begin && pos != ln_end ) ;

		add_midchain( pos, ln ) ;

		++tot ;

		return ln ;
	}


	iline* insert( Data::Iterator it,
		       iline* ln )
	{
		//
		// Add ln before position it.  Return newly inserted address.
		//

		iline* pos = &( *it ) ;

		assert( pos != ln_begin && pos != ln_end ) ;

		add_midchain( pos, ln ) ;

		++tot ;

		return ln ;
	}


	iline* insert( iline* pos,
		       iline* ln,
		       const string& s,
		       uint32_t status = 0 )
	{
		//
		// Add ln before position pos.  Return newly inserted address.
		// Set idata (no undo level).
		//

		assert( pos != ln_begin && pos != ln_end ) ;

		ln->put_idata( s, status ) ;

		add_midchain( pos, ln ) ;

		++tot ;

		return ln ;
	}


	iline* insert( Data::Iterator it,
		       iline* ln,
		       const string& s,
		       uint32_t status = 0 )
	{
		//
		// Add ln before position pos.  Return newly inserted address.
		// Set idata (no undo level).
		//

		iline* pos = &( *it ) ;

		assert( pos != ln_begin && pos != ln_end ) ;

		ln->put_idata( s, status ) ;

		add_midchain( pos, ln ) ;

		++tot ;

		return ln ;
	}


	iline* insert( iline* pos,
		       iline* ln,
		       const string& s,
		       int level,
		       uint32_t status = 0 )
	{
		//
		// Add ln before position pos.  Return newly inserted address.
		// Set idata.
		//

		assert( pos != ln_begin && pos != ln_end ) ;

		ln->put_idata( s, level, status ) ;

		add_midchain( pos, ln ) ;

		++tot ;

		return ln ;
	}


	iline* insert( Data::Iterator it,
		       iline* ln,
		       const string& s,
		       int level,
		       uint32_t status = 0 )
	{
		//
		// Add ln before position pos.  Return newly inserted address.
		// Set idata.
		//

		iline* pos = &( *it ) ;

		assert( pos != ln_begin && pos != ln_end ) ;

		ln->put_idata( s, level, status ) ;

		add_midchain( pos, ln ) ;

		++tot ;

		return ln ;
	}


	iline* erase( iline* ln )

	{
		//
		// Remove element pointed to by address ln from the chain and delete it.
		// Return the address of the next element in the chain.
		//

		assert( ln != ln_end && ln != ln_begin && ln != ln_bottom ) ;

		iline* next = ln->il_next ;

		remove_midchain( ln ) ;

		delete ln ;

		--tot ;

		return next ;
	}


	iline* erase( Data::Iterator it )
	{
		//
		// Remove element pointed to by iterator it from the chain and delete it.
		// Return the address of the next element in the chain.
		//

		iline* ln = &( *it ) ;

		assert( ln != ln_end && ln != ln_begin && ln != ln_bottom ) ;

		iline* next = ln->il_next ;

		remove_midchain( ln ) ;

		delete ln ;

		--tot ;

		return next ;
	}


	bool lt( Data::Iterator a,
		 Data::Iterator b )
	{
		//
		// Return true if 'a' is at a position less than 'b'.
		//

		resequence() ;
		return ( a->il_seqn < b->il_seqn ) ;
	}


	bool le( Data::Iterator a,
		 Data::Iterator b )
	{
		//
		// Return true if 'a' is at a position less than, or equal to, 'b'.
		//

		resequence() ;
		return ( a->il_seqn <= b->il_seqn ) ;
	}


	bool gt( Data::Iterator a,
		 Data::Iterator b )
	{
		//
		// Return true if 'a' is at a position greater than 'b'.
		//

		resequence() ;
		return ( a->il_seqn > b->il_seqn ) ;
	}


	bool ge( Data::Iterator a,
		 Data::Iterator b )
	{
		//
		// Return true if 'a' is at a position greater than, or equal to, 'b'.
		//

		resequence() ;
		return ( a->il_seqn >= b->il_seqn ) ;
	}


	Data::Iterator at( int i )
	{
		//
		// Return the iterator for data line i.
		// First line is 0.
		//

		Data::Iterator it = Data::begin() ;

		for ( ; it != Data::end() && i > 0 ; ++it )
		{
			--i ;
		}

		return it ;
	}


	int at( Data::Iterator ite )
	{
		//
		// Return the data line index for iterator.
		// First line is 0.
		//

		int i = 0 ;

		Data::Iterator its = Data::begin() ;

		for ( ; its != Data::end() && its != ite ; ++its )
		{
			++i ;
		}

		return i ;
	}


	int size() { return tot ; }


	void clear()
	{
		for ( iline* i = ln_end->il_prev ; i ; i = i->il_prev )
		{
			delete i->il_next ;
		}
		delete ln_begin ;

		ln_bottom = nullptr ;
		ln_end    = new iline() ;
		ln_begin  = ln_end ;
		tot       = 0 ;
		reseq     = false ;
	}


	private:
		int tot ;

		iline* ln_begin ;
		iline* ln_bottom ;
		iline* ln_end ;

		bool reseq ;


	void push_backchain( iline* ln_new )
	{
		//
		// Add line ln_new at the end of the chain.
		// Add a sequence number (1000 more than previous sequence number).
		//

		if ( ln_bottom )
		{
			ln_bottom->il_next = ln_new ;
			ln_new->il_prev    = ln_bottom ;
		}
		else
		{
			ln_begin = ln_new ;
		}


		ln_end->il_prev = ln_new ;
		ln_new->il_next = ln_end ;

		ln_bottom = ln_new ;

		if ( ln_new->il_prev )
		{
			ln_new->il_seqn = ln_new->il_prev->il_seqn + 1000 ;
		}
	}


	void add_midchain( iline* ln_isrt,
			   iline* ln_new )
	{
		//
		// Add line ln_new before ln_isrt.
		// Add a sequence number (half difference), or flag for resequencing.
		//

		uint diff ;

		iline* ln_prev = ln_isrt->il_prev ;

		ln_prev->il_next = ln_new ;
		ln_isrt->il_prev = ln_new ;

		ln_new->il_next = ln_isrt ;
		ln_new->il_prev = ln_prev ;

		if ( !reseq )
		{
			diff = ( ln_isrt->il_seqn - ln_prev->il_seqn ) / 2 ;
			if ( diff > 0 )
			{
				ln_new->il_seqn = ln_prev->il_seqn + diff ;
			}
			else
			{
				reseq = true ;
			}
		}
	}


	void remove_midchain( iline* ln_curr )
	{
		//
		// Remove ln_curr from the iline chain
		//

		ln_curr->il_next->il_prev = ln_curr->il_prev ;
		ln_curr->il_prev->il_next = ln_curr->il_next ;
	}

	void resequence()
	{
		//
		// Reseqence the sequence number on the iline record by ( max uint )/data.size().
		//

		uint i ;
		uint maxi ;

		if ( reseq )
		{
			maxi = UINT_MAX / tot ;
			i    = 0 ;
			for ( auto it = Data::begin() ; it != Data::end() ; ++it, i += maxi )
			{
				it->il_seqn = i ;
			}
			reseq = false ;
		}
	}
} ;


class extData
{
	public:
		extData()
		{
			move     = false ;
			copy     = false ;
			replace  = false ;
			create   = false ;
			filename = "" ;
		}

		bool is_move()    { return move    ; }
		bool is_copy()    { return copy    ; }
		bool is_replace() { return replace ; }
		bool is_create()  { return create  ; }

		void set_move( const string& f )    { move    = true ; filename = f ; }
		void set_copy( const string& f )    { copy    = true ; filename = f ; }
		void set_replace( const string& f ) { replace = true ; filename = f ; }
		void set_create( const string& f )  { create  = true ; filename = f ; }

		const string& name() { return filename ; }

		void reset()
		{
			move     = false ;
			copy     = false ;
			replace  = false ;
			create   = false ;
			filename = "" ;
		}

	private:
		bool move ;
		bool copy ;
		bool replace ;
		bool create ;

		string filename ;
} ;


class lcmd
{
	private:
		lcmd()
		{
			lcmd_clear() ;
		}

		void lcmd_clear()
		{
			lcmd_cmdstr  = " " ;
			lcmd_lcname  = ""  ;
			lcmd_sADDR   = nullptr ;
			lcmd_eADDR   = nullptr ;
			lcmd_dADDR   = nullptr ;
			lcmd_lADDR   = nullptr ;
			lcmd_ABOW    = ' ' ;
			lcmd_ABRpt   = 0   ;
			lcmd_Rpt     = 0   ;
			lcmd_procd   = false ;
			lcmd_cut     = false ;
			lcmd_create  = false ;
			extdata.reset() ;
		}

		void set( const extData& e )
		{
			extdata = e ;
		}

		L_CMDS lcmd_cmd    ;
		string lcmd_cmdstr ;
		string lcmd_lcname ;
		iline* lcmd_sADDR  ;
		iline* lcmd_eADDR  ;
		iline* lcmd_dADDR  ;
		iline* lcmd_lADDR  ;
		int    lcmd_Rpt    ;
		int    lcmd_ABRpt  ;
		char   lcmd_ABOW   ;
		bool   lcmd_procd  ;
		bool   lcmd_cut    ;
		bool   lcmd_create ;

		extData extdata ;

	friend class pedit01 ;
} ;

map<int, bool> iline::setUNDO ;
map<int, bool> iline::Redo_data ;
map<int, bool> iline::Redo_other ;
map<int, bool> iline::File_save ;
map<int, stack<int>>iline::Global_Undo ;
map<int, stack<int>>iline::Global_Redo ;
map<int, stack<int>>iline::Global_File_level ;
map<int, stack<istatus>>iline::Global_status ;
map<int, stack<istatus>>iline::Global_status_redo ;
map<int, string> iline::src_file ;
map<int, string> iline::dst_file ;
map<int, bool> iline::file_changed ;
map<int, bool> iline::file_inserts ;
map<int, bool> iline::changed_icond ;
map<int, bool> iline::changed_ilabel ;
map<int, bool> iline::changed_xclud ;


class ipos
{
	public:
		ipos( uint w = 0 )
		{
			ipos_addr    = nullptr ;
			ipos_hex     = 0 ;
			ipos_lchar   = w ;
			ipos_oupdate = "N" ;
		}

		void clear( uint w )
		{
			ipos_addr    = nullptr ;
			ipos_hex     = 0 ;
			ipos_lchar   = w ;
			ipos_oupdate = "N" ;
		}

		bool ipos_hex_top() const
		{
			return ( ipos_hex == 1 ) ;
		}

		bool ipos_hex_bottom() const
		{
			return ( ipos_hex == 2 ) ;
		}

		bool ipos_div() const
		{
			return ( ipos_hex == 3 ) ;
		}

		bool ipos_hex_line() const
		{
			return ( ipos_hex > 0 ) ;
		}

		void set_null()
		{
			ipos_addr = nullptr ;
		}

		void set_ovalue( uint w,
				 size_t lnum2,
				 const string& t,
				 char c )
		{
			ipos_ovalue = t.substr( w ) ;
			if ( lnum2 > 0 )
			{
				if ( ipos_ovalue.size() <= lnum2 )
				{
					ipos_ovalue = "" ;
				}
				else
				{
					ipos_ovalue.erase( ipos_ovalue.size() - lnum2 ) ;
				}
			}
			ipos_oshadow = string( ipos_ovalue.size(), c ) ;
			ipos_oupdate = "N" ;
		}

		void set_shadow( const string& t )
		{
			ipos_oshadow = t ;
			ipos_oshadow.resize( ipos_ovalue.size(), N_GREEN ) ;
		}

		void clear_value()
		{
			ipos_ovalue  = "" ;
			ipos_oshadow = "" ;
			ipos_oupdate = "N" ;
		}

		ipos( const ipos& ca )
		{
			ipos_addr  = ca.ipos_addr  ;
			ipos_hex   = ca.ipos_hex   ;
			ipos_lchar = ca.ipos_lchar ;

		}

		ipos operator = ( const ipos& rhs )
		{
			ipos_addr  = rhs.ipos_addr  ;
			ipos_hex   = rhs.ipos_hex   ;
			ipos_lchar = rhs.ipos_lchar ;

			return *this ;
		}

		iline* ipos_addr  ;
		uint   ipos_hex   ;
		uint   ipos_lchar ;
		string ipos_ovalue ;
		string ipos_oshadow ;
		string ipos_oupdate ;
} ;


class cmd_range
{
	private:
		cmd_range()
		{
			c_vlab  = false ;
			c_vcol  = false ;
			c_vlnm  = false ;
			c_slab  = ""    ;
			c_elab  = ""    ;
			c_sidx  = nullptr ;
			c_eidx  = nullptr ;
			c_scol  = 0     ;
			c_ecol  = 0     ;
			c_ocol  = false ;
			c_rest  = ""    ;
		}

		void cmd_range_clear()
		{
			c_slab  = ""    ;
			c_elab  = ""    ;
			c_sidx  = nullptr ;
			c_eidx  = nullptr ;
			c_scol  = 0     ;
			c_ecol  = 0     ;
			c_ocol  = false ;
			c_rest  = ""    ;
		}

		bool   c_vlab  ;
		bool   c_vcol  ;
		bool   c_vlnm  ;
		string c_slab  ;
		string c_elab  ;
		iline* c_sidx  ;
		iline* c_eidx  ;
		int    c_scol  ;
		int    c_ecol  ;
		bool   c_ocol  ;
		string c_rest  ;

	friend class pedit01 ;
} ;


class edit_find
{
	public:
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
			f_rc       = 0     ;
			f_ADDR     = nullptr ;
			f_pADDR    = nullptr ;
			f_pCol     = 0     ;
			f_ptopLine = nullptr ;
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
			f_acol     = 0     ;
			f_bcol     = 0     ;
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

		~edit_find()
		{
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
			f_rc       = 0  ;
			f_chnincr  = 0  ;
			f_ADDR     = nullptr ;
			f_lines    = 0  ;
			f_offset   = 0  ;
		}

		void set_rstring( const string& s )
		{
			if ( f_rstring == "" ) { f_rstring = s ; }
		}

		void f_set_match( const string& s )
		{
			f_mtch = s.front() ;
		}

		bool f_prefix() const
		{
			return ( f_mtch == 'P' ) ;
		}

		void f_set_prefix()
		{
			f_mtch = 'P' ;
		}

		bool f_suffix() const
		{
			return ( f_mtch == 'S' ) ;
		}

		void f_set_suffix()
		{
			f_mtch = 'S' ;
		}

		bool f_word() const
		{
			return ( f_mtch == 'W' ) ;
		}

		void f_set_word()
		{
			f_mtch = 'W' ;
		}

		bool f_word_prefix() const
		{
			return ( f_mtch == 'W' || f_mtch == 'P' ) ;
		}

		bool f_word_suffix() const
		{
			return ( f_mtch == 'W' || f_mtch == 'S' ) ;
		}

		bool f_word_prefix_suffix() const
		{
			return ( f_mtch == 'W' || f_mtch == 'P' || f_mtch == 'S' ) ;
		}

		bool f_chars() const
		{
			return ( f_mtch == 'C' ) ;
		}

		void f_set_chars()
		{
			f_mtch = 'C' ;
		}

		string f_mtch_str() const
		{
			switch ( f_mtch )
			{
				case 'C': return "CHARS"  ;
				case 'P': return "PREFIX" ;
				case 'S': return "SUFFIX" ;
				case 'W': return "WORD"   ;
			}
			return "??????" ;
		}

		void f_set_dir( const string& s )
		{
			f_dir = s.front() ;
		}

		bool f_first() const
		{
			return ( f_dir == 'F' ) ;
		}

		void f_set_first()
		{
			f_dir = 'F' ;
		}

		bool f_all() const
		{
			return ( f_dir == 'A' ) ;
		}

		void f_set_all()
		{
			f_dir = 'A' ;
		}

		bool f_last() const
		{
			return ( f_dir == 'L' ) ;
		}

		void f_set_last()
		{
			f_dir = 'L' ;
		}

		bool f_next() const
		{
			return ( f_dir == 'N' ) ;
		}

		void f_set_next()
		{
			f_dir = 'N' ;
		}

		bool f_prev() const
		{
			return ( f_dir == 'P' ) ;
		}

		void f_set_prev()
		{
			f_dir = 'P' ;
		}

		bool f_reverse() const
		{
			return ( f_dir == 'L' || f_dir == 'P' ) ;
		}

		bool f_x() const
		{
			return ( f_excl == 'X' ) ;
		}

		bool f_nx() const
		{
			return ( f_excl == 'N' ) ;
		}

		void f_set_excl( char x )
		{
			f_excl = x ;
		}

		void f_set_nx()
		{
			f_excl = 'N' ;
		}

		void f_incr_occurs()
		{
			++f_occurs ;
		}

		void f_incr_lines()
		{
			++f_lines ;
		}

		void f_incr_change_occurs()
		{
			++f_ch_occs ;
		}

		void f_incr_change_errors()
		{
			++f_ch_errs ;
		}

		void set_seek_counts()
		{
			f_sk_occs = f_occurs ;
			f_sk_lnes = f_lines ;
		}

		void set_seek_counts( int a,
				      int b )
		{
			f_sk_occs = a ;
			f_sk_lnes = b ;
		}

		void set_find_counts()
		{
			f_fd_occs = f_occurs ;
			f_fd_lnes = f_lines ;
		}

		void set_find_counts( int a,
				      int b )
		{
			f_fd_occs = a ;
			f_fd_lnes = b ;
		}

		void set_exclude_counts()
		{
			f_ex_occs = f_occurs ;
			f_ex_lnes = f_lines ;
		}

		void set_exclude_counts( int a,
					 int b )
		{
			f_ex_occs = a ;
			f_ex_lnes = b ;
		}

		void set_change_counts( int a,
					int b )
		{
			f_ch_occs = a ;
			f_ch_errs = b ;
		}

		string getColumnsString()
		{
			if ( f_ocol )
			{
				return " on column " + d2ds( f_scol ) ;
			}
			else if ( f_acol > 0 )
			{
				if ( f_bcol > 1 )
				{
					return " within columns " + d2ds( f_acol ) + " to "+ d2ds( f_bcol ) ;
				}
				else
				{
					return " from column " + d2ds( f_acol ) + " to end of data" ;
				}
			}

			return "" ;
		}

		string getXNXString()
		{
			return ( f_excl == 'X' ) ? " excluded"     :
			( f_excl == 'N' ) ? " non-excluded" : "" ;
		}

		string getRangeString()
		{
			return ( f_slab != "" ) ? " from " + f_slab + " to " + f_elab : "" ;
		}

		string setFoundString()
		{
			//
			// Set the found/notfound string in find/change/exclude messages.
			//

			int sz ;

			string s ;

			if ( f_rstring == "" )
			{
				if ( f_hex )
				{
					return "X'" + f_ostring + "'" ;
				}
				s = f_ostring ;
			}
			else
			{
				if ( f_prefix() )
				{
					sz = ( f_hex ) ? f_ostring.size() / 2 : f_ostring.size() ;
					s  = f_rstring.substr( 0, sz ) ;
				}
				else if ( f_suffix() )
				{
					s = f_rstring.substr( 1, f_ostring.size() ) ;
				}
				else
				{
					s = f_rstring ;
				}
				if ( f_hex )
				{
					return "X'" + cs2xs( s ) + "'" ;
				}
			}

			if ( any_of( s.begin(), s.end(),
				[]( char c )
				{
					return ( !isprint( c ) ) ;
				} ) )
			{
				s = "X'" + cs2xs( s ) + "'" ;
			}
			else
			{
				s = "'" + s + "'" ;
			}

			return s ;
		}

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
		int    f_rc      ;
		iline* f_ADDR    ;
		iline* f_pADDR   ;
		int    f_pCol    ;
		iline* f_ptopLine;
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
		int    f_acol    ;
		int    f_bcol    ;
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
		boost::regex f_regexp ;
} ;


class user_state
{
	public:
		user_state( const  profile& prof,
			    const  edit_find& find,
			    const  string& maskline,
			    const  string& tabline,
			    int    lbounds,
			    int    rbounds,
			    iline* mRow1,
			    int    mRow2,
			    int    mCol,
			    iline* topline1,
			    int    topline2,
			    bool   cursorset )
		{
			ustate_prof      = prof ;
			ustate_find      = find ;
			ustate_maskline  = maskline ;
			ustate_tabline   = tabline ;
			ustate_lbounds   = lbounds ;
			ustate_rbounds   = rbounds ;
			ustate_mRow1     = mRow1 ;
			ustate_mRow2     = mRow2 ;
			ustate_mCol      = mCol,
			ustate_topline1  = topline1 ;
			ustate_topline2  = topline2 ;
			ustate_cursorset = cursorset ;
		}

		string get_key( int seed )
		{
			static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
						       "abcdefghijklmnopqrstuvwxyz!$%^&*()_-+={}[]:;@~#<>?,./" ;
			string s ;
			s.reserve( 36 ) ;

			srand( time( nullptr ) + seed ) ;

			for ( int i = 0 ; i < 36 ; ++i )
			{
				s.push_back( alphanum[ rand() % ( sizeof( alphanum ) - 1 ) ] ) ;
			}

			return s ;
		}

		profile   ustate_prof ;
		edit_find ustate_find ;

		int    ustate_lbounds ;
		int    ustate_rbounds ;
		int    ustate_mCol ;
		iline* ustate_mRow1 ;
		int    ustate_mRow2 ;
		iline* ustate_topline1 ;
		int    ustate_topline2 ;
		bool   ustate_cursorset ;
		string ustate_tabline ;
		string ustate_maskline ;
} ;


class defName
{
	public:
		defName()
		{
			name     = "" ;
			alias    = false ;
			nop      = false ;
			disabled = false ;
			exp      = false ;
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

		string name ;
		bool   alias ;
		bool   nop ;
		bool   disabled ;
		bool   exp ;
		bool   cmd ;
		bool   pgm ;
} ;


class Cursorp
{
	private:
		Cursorp()
		{
			placeHome  = false ;
			placeUsing = 1 ;
			placeADDR  = nullptr ;
			placeRow   = 0 ;
			placeType  = CSR_FC_LCMD ;
			placeOff   = 0 ;
		}

		Cursorp( iline* addr,
			 CSR_POS pt,
			 int offset = 0 ,
			 bool lcmd = false ) : Cursorp()
		{
			//
			// placeUsing: Use 'addr' to place cursor.
			//
			// placeType       CSR_FC_LCMD        First char of the line command area.
			// (CSR_POS)       CSR_FC_DATA        First char of data area.
			//                 CSR_FNB_DATA       First non-blank char of data area (after startCol).
			//                 CSR_OFF_DATA       Use position in placeOff as start of data line.
			//                 CSR_OFF_LCMD       Use position in placeOff as start of zdataw.
			//                 CSR_LC_OR_FNB_DATA First char of line command or data area depeding on where the cursor is.
			//
			// placeOff  Offset from start of the data line (adjust for line command and startCol later).
			//

			if ( !addr )
			{
				placeHome = true ;
				return ;
			}

			placeADDR = addr ;

			if ( pt == CSR_LC_OR_FNB_DATA )
			{
				if ( !lcmd ) { return ; }
				placeType = CSR_FC_LCMD ;
			}
			else
			{
				placeType = pt ;
			}

			placeOff = offset ;
		}

		Cursorp( uint row,
			 CSR_POS pt,
			 int offset = false ) : Cursorp()
		{
			//
			// placeUsing: Use screen row to place cursor ( first row = 1 ).
			//

			placeUsing = 2 ;
			placeRow   = row ;
			placeType  = pt ;
			placeOff   = offset ;
		}


		Cursorp( iline* addr,
			 int offset ) : Cursorp()
		{
			//
			// Store current cursor position using placeType=CSR_OFF_DATA (position in placeOff).
			//

			if ( addr ) { placeADDR = addr ; }
			else        { placeHome = true ; }
			placeType = CSR_OFF_DATA ;
			placeOff  = offset ;
		}

		void clear()
		{
			placeHome  = false ;
			placeUsing = 1 ;
			placeADDR  = nullptr ;
			placeRow   = 0 ;
			placeType  = CSR_FC_LCMD ;
			placeOff   = 0 ;
		}

		void home()
		{
			//
			// Place cursor at the home position.
			//

			placeHome = true ;
		}


		iline* getAddr()
		{
			return placeADDR ;
		}

		bool    placeHome ;
		int     placeUsing ;
		iline*  placeADDR ;
		int     placeRow ;
		CSR_POS placeType ;
		int     placeOff ;

		bool operator != ( const Cursorp& rhs ) const
		{
			return ( placeHome  != rhs.placeHome  ||
				placeUsing != rhs.placeUsing ||
				placeADDR  != rhs.placeADDR  ||
				placeRow   != rhs.placeRow   ||
				placeType  != rhs.placeType  ||
				placeOff   != rhs.placeOff   ) ;
		}

	friend class Cursor  ;
	friend class pedit01 ;
} ;


class Cursor
{
	//
	// Keep 3 cursors:
	// a - first choice.
	// b - second choice if cursor a is not on the screen (default to cursor c).
	// c - stored cursor after DISPLAY service returns.
	//

	public:
		Cursor()
		{
			_fixed = false ;
			a      = nullptr ;
			b      = nullptr ;
			c      = nullptr ;
		}

		~Cursor()
		{
			delete a ;
			delete b ;
			delete c ;
		}

		void clear()
		{
			_fixed = false ;

			delete a ;
			delete b ;

			a = nullptr ;
			b = ( c ) ? new Cursorp( *c ) : nullptr ;
		}

		Cursorp* get()
		{
			return ( a ) ? a : b ;
		}

		Cursorp* def()
		{
			return c ;
		}

		bool isdef( Cursorp* x )
		{
			return ( c == x ) ;
		}

		void set( iline* addr,
			  CSR_POS pt,
			  int offset = 0 ,
			  bool lcmd = false )
		{
			//
			// placeUsing: Use 'addr' to place cursor.
			//

			move_a2b() ;

			a = new Cursorp( addr, pt, offset, lcmd ) ;

			if ( !addr )
			{
				a->home() ;
				return ;
			}

			fix() ;
		}


		void set( iline* addr1,
			  iline* addr2,
			  CSR_POS pt )
		{
			//
			// placeUsing: Use 'addr' to place cursor.
			// addr1 - primary position.
			// addr2 - secondary position.
			//

			delete a ;
			delete b ;

			a = new Cursorp( addr1, pt ) ;
			b = new Cursorp( addr2, pt ) ;

			fix() ;
		}


		void set( uint row,
			  CSR_POS pt,
			  int offset = 0 )
		{
			//
			// placeUsing: Use screen row to place cursor ( first row = 1 ).
			//

			move_a2b() ;

			a = new Cursorp( row, pt, offset ) ;

			fix() ;
		}


		void store( iline* addr,
			    int offset )
		{
			//
			// Store current cursor position using PlaceType=CSR_OFF_DATA (position in PlaceOff).
			//

			_fixed = false ;

			delete a ;
			delete b ;
			delete c ;

			a = nullptr ;
			b = nullptr ;

			c = ( addr ) ? new Cursorp( addr, offset ) : nullptr ;
		}

		void home()
		{
			//
			// Place cursor at the home position.
			//

			delete a ;
			delete b ;

			a = new Cursorp ;
			b = nullptr ;

			a->placeHome = true ;

			fix() ;
		}

		void fix()
		{
			_fixed = true ;

			if ( !a && c )
			{
				a = new Cursorp( *c ) ;
			}
		}

		bool not_fixed()
		{
			return !_fixed ;
		}

		bool addr( const iline* addr )
		{
			return ( a && a->placeADDR == addr ) ||
			       ( b && b->placeADDR == addr ) ||
			       ( c && c->placeADDR == addr ) ;
		}

		void pop_front()
		{
			delete a ;
			a = b ;
			b = nullptr ;
		}

		void replace( iline* addr1,
			      iline* addr2 )
		{
			if ( a && a->placeADDR == addr1 )
			{
				a->placeADDR = addr2 ;
			}
			if ( b && b->placeADDR == addr1 )
			{
				b->placeADDR = addr2 ;
			}
			if ( c && c->placeADDR == addr1 )
			{
				c->placeADDR = addr2 ;
			}
		}

	private:
		void move_a2b()
		{
			delete b ;
			b = a ;
			a = nullptr ;
		}

		Cursorp* a ;
		Cursorp* b ;
		Cursorp* c ;

		bool _fixed ;
} ;


class cmdblock
{
	public:
		cmdblock()
		{
			clear_all() ;
		}

		void clear_all()
		{
			cmd     = "" ;
			cmdf    = "" ;
			ocmd    = "" ;
			zverb   = "" ;
			parms   = "" ;
			p_cmd   = PC_INVCMD ;
			msgid   = "" ;
			cchar   = "" ;
			udata   = "" ;
			val1    = "" ;
			val2    = "" ;
			val3    = "" ;
			RC      = 0  ;
			mRC     = 0  ;
			RSN     = 0  ;
			cwds    = 0  ;
			alias   = false ;
			seek    = false ;
			actiond = false ;
			macro   = false ;
			expl    = false ;
			keep    = false ;
			deact   = false ;
			ucase   = false ;
		}

		void clear()
		{
			msgid = ""    ;
			RC    = 0     ;
			mRC   = 0     ;
			RSN   = 0     ;
			zverb = ""    ;
			parms = ""    ;
			cchar = ""    ;
			udata = ""    ;
			val1  = ""    ;
			val2  = ""    ;
			val3  = ""    ;
			macro = true  ;
			expl  = false ;
			seek  = false ;
			keep  = false ;
			alias = false ;
			deact = false ;
			ucase = false ;
		}

		void set_cmd( const string& s1,
			      const string& s2,
			      map<string, stack<defName>>& defNames,
			      const string& zzverb = "" )
		{
			set_cmd( s1 + " " + s2, defNames, zzverb ) ;
		}

		void set_cmd( const string& s,
			      map<string,stack<defName>>& defNames,
			      const string& zzverb = "" )
		{
			string w ;

			map<string,stack<defName>>::const_iterator ita ;
			map<P_CMDS,pcmd_format>::const_iterator itp ;

			if ( zzverb == "RFIND" || zzverb == "RCHANGE" )
			{
				clear_all() ;
				zverb = zzverb ;
				parms = s ;
				return ;
			}

			cmd = strip( s ) ;
			if ( cmd == "" ) { clear_all() ; return ; }

			clear() ;

			if ( cmd.front() == '&' )
			{
				cmd.erase( 0, 1 ) ;
				if ( cmd == "" ) { return ; }
				keep  = true ;
				cchar = "&"  ;
			}
			else if ( cmd.front() == ':' )
			{
				iupper( cmd.erase( 0, 1 ) ) ;
				cmd.erase( remove( cmd.begin(), cmd.end(), ' ' ), cmd.end() ) ;
				if ( cmd == "" ) { return ; }
				macro = false ;
				cchar = ":"   ;
			}
			if ( cmd.front() == '%' )
			{
				cmd.erase( 0, 1 ) ;
				if ( cmd == "" ) { return ; }
				expl   = true ;
				cchar += "%"  ;
			}
			else
			{
				w = upper( word( cmd, 1 ) ) ;
				if ( w == "BUILTIN" )
				{
					if ( trim( cmd ) == "BUILTIN" )
					{
						set_msg( "PEDT015F", 20 ) ;
						keep = true ;
						return ;
					}
					w = get_truename( upper( word( cmd, 2 ) ) ) ;
					if ( PrimCMDS.count( w ) == 0 )
					{
						set_msg( "PEDM012O", 20 ) ;
						return ;
					}
					idelword( cmd, 1, 1 ) ;
				}
				else
				{
					w   = get_truename( w ) ;
					ita = defNames.find( w ) ;
					if ( ita != defNames.end() )
					{
						const defName* dn = &ita->second.top() ;
						if ( dn->deactive() )
						{
							set_msg( "PEDT015R", w, 12 ) ;
							deact = true ;
							return ;
						}
						if ( dn->macro() )
						{
							expl = true ;
						}
						else if ( dn->alias )
						{
							w   = get_truename( dn->name ) ;
							ita = defNames.find( w ) ;
							if ( ita != defNames.end() && ita->second.top().deactive() )
							{
								set_msg( "PEDT015R", w, 12 ) ;
								deact = true ;
								return ;
							}
							ocmd  = cmd ;
							cmd   = w + " " + idelword( cmd, 1, 1 ) ;
							trim( cmd )  ;
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
			cwds = words( cmd ) ;
			cmdf = upper( word( cmd, 1 ) ) ;
			itp  = pcmdFormat.find( p_cmd ) ;
			if ( itp != pcmdFormat.end() )
			{
				if ( cwds < ( itp->second.noptions1 + 1 ) )
				{
					set_msg( "PEDT015E", 20 ) ;
					return ;
				}
				if ( itp->second.noptions2 != -1 && cwds > ( itp->second.noptions2 + 1 ) )
				{
					set_msg( "PEDT015D", 20 ) ;
					return ;
				}
				ucase = itp->second.ucase ;
			}
			actiond = false ;
			parms   = subword( cmd, 2 ) ;
		}

		void set_cmd( const string& s )
		{
			cmd = s ;
		}

		void set_cmd( const string& s1,
			      const string& s2 )
		{
			cmd = s1 + " " + s2 ;
		}

		void set_cmd_keep( const string& s )
		{
			cmd   = s ;
			keep  = true ;
			cchar = "&" ;
		}

		void set_zverb_cmd()
		{
			if ( zverb != "" )
			{
				cmd = zverb + " " + parms ;
			}
		}

		const string& get_parms()
		{
			return parms ;
		}

		void pop_parm1()
		{
			idelword( parms, 1, 1 ) ;
		}

		string condget_cmd() const
		{
			if ( error() || keep ) { return macro ? cchar + cmd :
							ucase ? alias ? cchar + upper( ocmd )  : cchar + upper( cmd )
							      : alias ? cchar + upper1( ocmd ) : cchar + upper1( cmd ) ; }
			return "" ;
		}

		int get_cmd_words() const
		{
			return cwds ;
		}

		const string& get_cmd() const
		{
			return cmd ;
		}

		string get_cmd1() const
		{
			return word( cmd, 1 ) ;
		}

		const string& get_cmdf() const
		{
			return cmdf ;
		}

		bool cmdf_equals( const string& s ) const
		{
			return ( cmdf == s ) ;
		}

		bool cmdf_equals( const string& s1,
				  const string& s2 ) const
		{
			return ( cmdf == s1 || cmdf == s2 ) ;
		}

		bool cmdf_not_equals( const string& s ) const
		{
			return ( cmdf != s ) ;
		}

		int is_macro() const
		{
			return macro ;
		}

		void actioned()
		{
			actiond = true ;
		}

		bool is_actioned() const
		{
			return actiond ;
		}

		bool not_actioned() const
		{
			return !actiond ;
		}

		int is_seek() const
		{
			return seek ;
		}

		int isLine_cmd() const
		{
			return ( cchar == ":" ) ;
		}

		void reset()
		{
			cmd     = "" ;
			cchar   = "" ;
			udata   = "" ;
			p_cmd   = PC_INVCMD ;
			actiond = false ;
			seek    = false ;
			macro   = false ;
			expl    = false ;
			keep    = false ;
		}

		void cond_reset()
		{
			if ( !keep ) { reset() ; }
		}

		void clear_msg()
		{
			msgid = "" ;
			RC    = 0  ;
			mRC   = 0  ;
			RSN   = 0  ;
			val1  = "" ;
			val2  = "" ;
			val3  = "" ;
		}

		void set_msg( const string& m,
			      int rc1,
			      int rc2 = -1 )
		{
			msgid = m   ;
			RC    = rc1 ;
			mRC   = ( rc2 == -1 ) ? rc1 : rc2 ;
			val1  = ""  ;
			val2  = ""  ;
			val3  = ""  ;
		}

		void set_msg( const string& m,
			      const string& v1,
			      int rc1,
			      int rc2 = -1 )
		{
			msgid = m   ;
			RC    = rc1 ;
			mRC   = ( rc2 == -1 ) ? rc1 : rc2 ;
			val1  = v1  ;
			val2  = ""  ;
			val3  = ""  ;
		}

		void set_msg( const string& m,
			      const string& v1,
			      const string& v2,
			      int rc1,
			      int rc2 = -1 )
		{
			msgid = m   ;
			RC    = rc1 ;
			mRC   = ( rc2 == -1 ) ? rc1 : rc2 ;
			val1  = v1  ;
			val2  = v2  ;
			val3  = ""  ;
		}

		void set_msg( const string& m,
			      const string& v1,
			      const string& v2,
			      const string& v3,
			      int rc1,
			      int rc2 = -1 )
		{
			msgid = m   ;
			RC    = rc1 ;
			mRC   = ( rc2 == -1 ) ? rc1 : rc2 ;
			val1  = v1  ;
			val2  = v2  ;
			val3  = v3  ;
		}

		void set_msg_cond( const string& m,
				   int rc1,
				   int rc2 = -1 )
		{
			if ( msgid == "" )
			{
				msgid = m   ;
				RC    = rc1 ;
				mRC   = ( rc2 == -1 ) ? rc1 : rc2 ;
				val1  = ""  ;
				val2  = ""  ;
				val3  = ""  ;
			}
		}

		void setRC( int rc )
		{
			RC = rc ;
		}

		const string& get_msg() const
		{
			return msgid ;
		}

		bool msgset() const
		{
			return ( msgid != "" ) ;
		}

		bool cmdset() const
		{
			return ( cmd != "" ) ;
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
			return ( RC >= 12 ) ;
		}

		bool ok() const
		{
			return ( RC < 12 ) ;
		}

		bool deactive() const
		{
			return deact ;
		}

		void keep_cmd()
		{
			keep = true ;
		}

		bool has_msg_val1()
		{
			return val1 != "" ;
		}

		bool has_msg_val2()
		{
			return val2 != "" ;
		}

		bool has_msg_val3()
		{
			return val3 != "" ;
		}

		const string& get_msg_val1()
		{
			return val1 ;
		}

		const string& get_msg_val2()
		{
			return val2 ;
		}

		const string& get_msg_val3()
		{
			return val3 ;
		}

		const string& get_truename( const string& w ) const
		{
			auto it = PrimCMDS.find( w ) ;
			return ( it == PrimCMDS.end() ) ? w : it->second.truename ;
		}

		string cmd   ;
		string cmdf  ;
		string ocmd  ;
		string zverb ;
		string parms ;
		P_CMDS p_cmd ;
		string msgid ;
		string cchar ;
		string udata ;
		string val1  ;
		string val2  ;
		string val3  ;
		int    RC    ;
		int    mRC   ;
		int    RSN   ;
		int    cwds  ;
		bool   alias ;
		bool   seek  ;
		bool   actiond ;
		bool   macro ;
		bool   expl  ;
		bool   keep  ;
		bool   deact ;
		bool   ucase ;

	friend class pedit01 ;
	friend class pedrxm1 ;
} ;


class miblock
{
	public:
		miblock()
		{
			clear_all() ;
		}

		void clear_all()
		{
			emacro    = ""    ;
			rmacro    = ""    ;
			mfile     = ""    ;
			editAppl  = nullptr ;
			macAppl   = nullptr ;
			sttment   = ""    ;
			m_cmd     = EM_INVCMD ;
			rxpath1   = ""    ;
			rxpath2   = ""    ;
			keyword   = ""    ;
			keyopts   = ""    ;
			value     = ""    ;
			parms     = ""    ;
			var1      = ""    ;
			var2      = ""    ;
			msgid     = ""    ;
			val1      = ""    ;
			val2      = ""    ;
			val3      = ""    ;
			zerrmsg   = ""    ;
			zerrsm    = ""    ;
			zerrlm    = ""    ;
			zerralrm  = ""    ;
			zerrhm    = ""    ;
			zerrtype  = ""    ;
			zerrwind  = ""    ;
			zldpath   = ""    ;
			lcmd      = ""    ;
			setzerr   = false ;
			mfound    = false ;
			macro     = false ;
			messageOn = false ;
			imacro    = false ;
			process   = true  ;
			processed = false ;
			cursorset = false ;
			fatal     = false ;
			assign    = false ;
			query     = false ;
			scan      = true  ;
			runmacro  = false ;
			cmd_macro = false ;
			pgm_macro = false ;
			eended    = false ;
			cancel    = false ;
			editserv  = false ;
			errorsRet = false ;
			rexxError = false ;
			isrError  = false ;
			keep      = false ;
			ustatekey = 0     ;
			etaskid   = 0     ;
			sttwds    = 0     ;
			nestlvl   = 0     ;
			RC        = 0     ;
			RSN       = 0     ;
			exRC      = 0     ;
		}

		void clear()
		{
			macAppl   = nullptr ;
			sttment   = ""    ;
			m_cmd     = EM_INVCMD ;
			keyword   = ""    ;
			keyopts   = ""    ;
			value     = ""    ;
			parms     = ""    ;
			var1      = ""    ;
			var2      = ""    ;
			msgid     = ""    ;
			val1      = ""    ;
			val2      = ""    ;
			val3      = ""    ;
			zerrmsg   = ""    ;
			zerrsm    = ""    ;
			zerrlm    = ""    ;
			zerralrm  = ""    ;
			zerrhm    = ""    ;
			zerrtype  = ""    ;
			zerrwind  = ""    ;
			setzerr   = false ;
			mfound    = false ;
			macro     = false ;
			process   = true  ;
			cursorset = false ;
			assign    = false ;
			query     = false ;
			scan      = true  ;
			runmacro  = false ;
			rexxError = false ;
			sttwds    = 0     ;
			nestlvl   = 0     ;
			RC        = 0     ;
			RSN       = 0     ;
			exRC      = 0     ;
		}

		void reset()
		{
			m_cmd     = EM_INVCMD ;
			sttment   = ""    ;
			keyword   = ""    ;
			keyopts   = ""    ;
			value     = ""    ;
			var1      = ""    ;
			var2      = ""    ;
			msgid     = ""    ;
			val1      = ""    ;
			val2      = ""    ;
			val3      = ""    ;
			zerrmsg   = ""    ;
			zerrsm    = ""    ;
			zerrlm    = ""    ;
			zerralrm  = ""    ;
			zerrhm    = ""    ;
			zerrtype  = ""    ;
			zerrwind  = ""    ;
			setzerr   = false ;
			assign    = false ;
			query     = false ;
			runmacro  = false ;
			sttwds    = 0     ;
			RC        = 0     ;
			RSN       = 0     ;
		}

		void seterror( const string& e1,
			       int i1,
			       int i2 = 0 )
		{
			msgid = e1 ;
			RC    = i1 ;
			RSN   = i2 ;
			fatal = ( RC >= 12 ) ;
		}

		void seterror( const string& e1,
			       const string& e2,
			       int i1,
			       int i2 = 0 )
		{
			msgid = e1 ;
			val1  = e2 ;
			RC    = i1 ;
			RSN   = i2 ;
			fatal = ( RC >= 12 ) ;
		}

		void seterror( const string& e1,
			       const string& e2,
			       const string& e3,
			       int i1,
			       int i2 = 0 )
		{
			msgid = e1 ;
			val1  = e2 ;
			val2  = e3 ;
			RC    = i1 ;
			RSN   = i2 ;
			fatal = ( RC >= 12 ) ;
		}

		void seterror( const string& e1,
			       const string& e2,
			       const string& e3,
			       const string& e4,
			       int i1,
			       int i2 = 0 )
		{
			msgid = e1 ;
			val1  = e2 ;
			val2  = e3 ;
			val3  = e4 ;
			RC    = i1 ;
			RSN   = i2 ;
			fatal = ( RC >= 12 ) ;
		}

		void seterror( const cmdblock& cmd )
		{
			msgid = cmd.msgid ;
			if ( cmd.mRC >= 12 )
			{
				val1 = cmd.val1 ;
				val2 = cmd.val2 ;
				val3 = cmd.val3 ;
			}
			RC    = cmd.mRC ;
			RSN   = cmd.RSN ;
			fatal = ( RC >= 12 ) ;
		}

		void seterror( const cmdblock& cmd,
			       int rc )
		{
			msgid = cmd.msgid ;
			if ( cmd.mRC >= 12 )
			{
				val1 = cmd.val1 ;
				val2 = cmd.val2 ;
				val3 = cmd.val3 ;
			}
			RC    = rc ;
			RSN   = cmd.RSN ;
			fatal = ( RC >= 12 ) ;
		}

		bool error()
		{
			return ( RC > 11 ) ;
		}

		bool ok()
		{
			return ( RC < 12 ) ;
		}

		void set_cmd( const cmdblock& cmd )
		{
			keep   = cmd.keep ;
			rmacro = word( cmd.cmd, 1 ) ;
		}

		void exitRC( const string& s )
		{
			if ( datatype( s, 'W' ) )
			{
				exRC = ds2d( s ) ;
			}
			else
			{
				seterror( "PEDM012L", 20 ) ;
				exRC = 28 ;
			}
		}

		void exitRC( int i )
		{
			exRC = i ;
		}

		int exitRC() const
		{
			return exRC ;
		}

		void setRC( int i )
		{
			RC = i ;
			if ( RC >= 12 )
			{
				fatal = true ;
			}
			else
			{
				fatal = false ;
				msgid = "" ;
			}
		}

		void setRC_noError( int i )
		{
			setRC( i ) ;
			msgid = ""  ;
			fatal = false ;
		}

		void addRC_noError( int i )
		{
			setRC( min( 20, ( RC + i ) ) ) ;
			fatal = false ;
			msgid = "" ;
		}

		bool msgset() const
		{
			return ( msgid != "" ) ;
		}

		void replace_vars( string& s )
		{
			int i ;
			int j ;
			int ws ;

			char qt ;

			bool quote ;

			string var  ;
			string vars ;
			string val  ;
			string vals ;

			string::const_iterator it ;

			vector<int> ob ;
			vector<int> cb ;

			for ( i = 0, quote = false, it = s.begin() ; it != s.end() ; ++it, ++i )
			{
				if ( !quote && ( *it == '"' || *it == '\'' ) )
				{
					quote = true ;
					qt    = *it ;
				}
				else if ( quote )
				{
					if ( *it == qt ) { quote = false ; }
				}
				else if ( *it == '(' )
				{
					ob.push_back( i ) ;
					if ( ob.size() != cb.size() + 1 )
					{
						seterror( "PEDM012N", 20 ) ;
						return ;
					}
				}
				else if ( *it == ')' )
				{
					cb.push_back( i ) ;
					if ( ob.size() != cb.size() )
					{
						seterror( "PEDM012N", 20 ) ;
						return ;
					}
				}
			}

			if ( ob.size() != cb.size() )
			{
				seterror( "PEDM012N", 20 ) ;
				return ;
			}

			for ( i = ( ob.size() - 1 ) ; i >= 0 ; --i )
			{
				vars = s.substr( ob[ i ] + 1, ( cb[ i ] - ob[ i ] - 1 ) ) ;
				vals = "" ;
				replace( vars.begin(), vars.end(), ',', ' ' ) ;
				ws = words( vars ) ;
				if ( ws == 0 )
				{
					seterror( "PEDM011G", 20 ) ;
					return ;
				}
				for ( j = 1 ; j <= ws ; ++j )
				{
					var = upper( word( vars, j ) ) ;
					if ( !isvalidName( var ) )
					{
						seterror( "PEDM011D", var, 20 ) ;
						return ;
					}
					macAppl->vcopy( var, val ) ;
					if ( val.find( ' ' ) != string::npos )
					{
						val = "'" + val + "'" ;
					}
					vals += " " + val ;
				}
				s.replace( ob[ i ], ( cb[ i ] - ob[ i ] + 1 ), vals ) ;
			}
			trim( s ) ;
		}

		void parse( const string& s,
			    map<string, stack<defName>>& defNames )
		{
			char qt ;

			size_t p1 = string::npos ;
			size_t p2 = 0 ;

			string w ;
			string t ;

			bool quote = false ;
			bool asis  ;
			bool cfind ;
			bool cline ;

			string::iterator it ;

			const string asis_list  = "COMPARE CHANGE CREATE COPY MOVE REPLACE PATH " ;
			const string cfind_list = "FIND CHANGE SEEK" ;
			const string cline_list = "LINE LINE_BEFORE LINE_AFTER" ;

			if ( eended )
			{
				seterror( "PEDM013D", 12 ) ;
				eended = false ;
				return ;
			}

			reset() ;

			sttment = s ;
			trim( sttment ) ;

			w = get_truename( upper( word( sttment, 1 ) ) ) ;
			t = w + " " + subword( sttment, 2 ) ;

			asis  = ( findword( w, asis_list ) ) ;
			cfind = ( findword( w, cfind_list ) ) ;
			cline = ( findword( w, cline_list ) ) ;

			for ( it = t.begin() ; it != t.end() ; ++it, ++p2 )
			{
				if ( !quote && ( *it == '"' || *it == '\'' ) )
				{
					quote = true ;
					qt    = *it ;
					continue ;
				}
				if ( quote )
				{
					if ( *it == qt ) { quote = false ; }
					continue ;
				}
				if ( p1 == string::npos && *it == '=' && !cfind )
				{
					p1     = p2   ;
					assign = true ;
				}
				if ( !asis && ( !cline || !assign ) )
				{
					*it = toupper( *it ) ;
				}
			}

			if ( w == "MACRO" )
			{
				if ( macro )
				{
					seterror( "PEDM011B", 12 ) ;
				}
				else
				{
					m_cmd = EM_MACRO ;
					parse_MACRO()    ;
				}
				return ;
			}

			if ( !macro )
			{
				seterror( "PEDM011E", 12 ) ;
				return ;
			}

			if ( sttment == "" )
			{
				seterror( "PEDM012P", 12 ) ;
				return ;
			}

			sttwds = ( w == "BUILTIN" ) ? words( sttment ) - 1 : words( sttment ) ;

			if ( t.front() == '(' )
			{
				if ( p1 == string::npos )
				{
					seterror( "PEDM011L", 12 ) ;
					return ;
				}
				query = true ;
				parse_query( t, p1 ) ;
			}
			else
			{
				parse_action( t, p1, defNames ) ;
			}
		}

		void parse_MACRO()
		{
			int i  ;
			int ws ;

			size_t p1 ;

			string t ;
			string var ;

			t = upper( sttment ) ;
			trim( t.erase( 0, 6 ) ) ;
			if ( t != "" && t.front() == '(' )
			{
				p1 = t.find( ')' ) ;
				if ( p1 == string::npos )
				{
					seterror( "PEDM011C", 12 ) ;
					return ;
				}
				keyopts = t.substr( 1, p1-1 ) ;
				trim( t.erase( 0, p1+2 ) ) ;
				replace( keyopts.begin(), keyopts.end(), ',', ' ' ) ;
				iupper( keyopts ) ;
				for ( ws = words( keyopts ), i = 1 ; i <= ws ; ++i )
				{
					var = word( keyopts, i ) ;
					if ( !isvalidName( var ) )
					{
						seterror( "PEDM011D", var, 20 ) ;
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
				seterror( "PEDM011P", 12 ) ;
				return ;
			}
			macro = true ;
		}

		void parse_query( const string& t,
				  size_t p1 )
		{
			uint miss ;
			uint nvars ;
			uint nkeyopts ;

			string var ;
			string kphrase ;

			string* pvar ;

			map<string,mcmd_format>::const_iterator it ;

			value   = t.substr( 0, p1 ) ;
			kphrase = t.substr( p1+1 ) ;

			replace_vars( kphrase ) ;
			if ( fatal ) { return ; }

			keyword = get_truename( word( kphrase, 1 ) ) ;

			it = emService.find( keyword ) ;
			if ( it == emService.end() )
			{
				seterror( "PEDM011N", 12 ) ;
				return ;
			}

			const mcmd_format* mform = &it->second ;

			if ( !mform->qasvalid )
			{
				seterror( "PEDM012R", 20 ) ;
				return ;
			}
			m_cmd = mform->m_cmd ;

			keyopts  = subword( kphrase, 2 ) ;
			nkeyopts = words( keyopts ) ;
			trim( value ) ;

			p1 = value.find( ')' ) ;
			if ( p1 == string::npos )
			{
				seterror( "PEDM011F", 12 ) ;
				return ;
			}
			if ( value.back() != ')' )
			{
				seterror( "PEDM011I", 20 ) ;
				return ;
			}
			value = value.substr( 1, value.size() - 2 ) ;
			if ( trim( value ) == "" )
			{
				seterror( "PEDM011G", 12 ) ;
				return ;
			}
			if ( value.front() == ',' )
			{
				miss = 1 ;
				pvar = &var2 ;
			}
			else
			{
				miss = 0 ;
				pvar = &var1 ;
			}

			replace( value.begin(), value.end(), ',', ' ' ) ;

			nvars = words( value ) ;
			if ( nvars > 2 || ( nvars > 1 && pvar == &var2 ) )
			{
				seterror( "PEDM011H", 20 ) ;
				return ;
			}

			for ( uint i = 1 ; i <= nvars ; ++i )
			{
				var = word( value, i ) ;
				if ( !isvalidName( var ) )
				{
					seterror( "PEDM011D", var, 20 ) ;
					return ;
				}
				*pvar = var   ;
				pvar  = &var2 ;
			}

			if ( mform->qnvars1 != -1 && ( nvars + miss ) < mform->qnvars1 )
			{
				seterror( "PEDM012B", 12 ) ;
				return ;
			}

			if ( mform->qnvars2 != -1 && ( nvars + miss ) > mform->qnvars2 )
			{
				seterror( "PEDM012C", 20 ) ;
				return ;
			}

			if ( mform->qnkeyopts1 != -1 && nkeyopts < mform->qnkeyopts1 )
			{
				seterror( "PEDM011X", 20 ) ;
				return ;
			}

			if ( mform->qnkeyopts2 != -1 && nkeyopts > mform->qnkeyopts2 )
			{
				seterror( "PEDM011Y", 20 ) ;
				return ;
			}
		}

		void parse_action( string& t,
				   size_t p1,
				   map<string, stack<defName>>& defNames )
		{
			int nkeyopts ;
			int nvalopts ;

			string kphrase ;

			map<string,mcmd_format>::const_iterator it ;
			map<string,stack<defName>>::const_iterator ita ;

			if ( assign )
			{
				kphrase = t.substr( 0, p1 ) ;
				value   = t.substr( p1+1 ) ;
			}
			else
			{
				kphrase = t ;
				value   = subword( t, 2 ) ;
			}

			keyword = word( kphrase, 1 ) ;
			if ( keyword == "BUILTIN" )
			{
				keyword = get_truename( word( kphrase, 2 ) ) ;
				if ( keyword == "" )
				{
					seterror( "PEDM011K", 20 ) ;
					return ;
				}
				it = emService.find( keyword ) ;
				if ( it == emService.end() )
				{
					seterror( "PEDM012O", 20 ) ;
				}
				else if ( assign )
				{
					keyword = subword( kphrase, 1, 2 ) ;
					m_cmd   = EM_BUILTIN ;
					replace_vars( value ) ;
				}
				else if ( keyword == "END" || keyword == "CANCEL" || keyword == "SAVE" )
				{
					idelword( sttment, 1, 1 ) ;
					m_cmd = ( keyword == "END" )    ? EM_END :
						( keyword == "CANCEL" ) ? EM_CANCEL : EM_SAVE ;
				}
				else
				{
					keyword = "" ;
					value   = sttment ;
					m_cmd   = EM_BUILTIN ;
					if ( it->second.qasvalid || it->second.aasvalid )
					{
						replace_vars( value ) ;
					}
				}
				return ;
			}

			keyword = get_truename( keyword ) ;
			ita     = defNames.find( keyword ) ;
			if ( ita != defNames.end() )
			{
				const defName* dn = &ita->second.top() ;
				if ( dn->deactive() )
				{
					seterror( "PEDT015S", 12 ) ;
					return ;
				}
				if ( dn->macro() )
				{
					runmacro = true ;
					cmd_macro = dn->cmd ;
					pgm_macro = dn->pgm ;
					return ;
				}
				if ( dn->alias )
				{
					keyword = dn->name ;
					sttment = keyword + " " + idelword( sttment, 1, 1 ) ;
				}
			}

			it = emService.find( keyword ) ;
			if ( it == emService.end() )
			{
				runmacro = true ;
				if ( keyword.front() == '%' )
				{
					cmd_macro = true ;
				}
				else if ( keyword.front() == '!' )
				{
					pgm_macro = true ;
				}
				return ;
			}

			const mcmd_format* mform = &it->second ;

			if ( assign && !mform->aasvalid )
			{
				if ( !mform->qasvalid )
				{
					seterror( "PEDM012R", 20 ) ;
				}
				else
				{
					seterror( "PEDM011T", keyword, 20 ) ;
				}
				return ;
			}

			if ( !assign && !mform->csvalid )
			{

				seterror( "PEDM012S", 20 ) ;
				return ;
			}

			if ( mform->m_cmd == EM_PROCESS && words( sttment ) > 5 )
			{
				seterror( "PEDM011P", 12 ) ;
				return ;
			}

			m_cmd = mform->m_cmd ;
			if ( mform->qasvalid || mform->aasvalid )
			{
				replace_vars( value ) ;
				if ( fatal ) { return ; }
			}

			if ( !assign )
			{
				keyopts = value ;
				return ;
			}

			keyopts = subword( kphrase, 2 ) ;
			if ( mform->qasvalid || mform->aasvalid )
			{
				replace_vars( keyopts ) ;
				if ( fatal ) { return ; }
			}

			nvalopts = words( value ) ;
			nkeyopts = words( keyopts ) ;

			if ( mform->ankeyopts1 != -1 && nkeyopts < mform->ankeyopts1 )
			{
				seterror( "PEDM011X", 20 ) ;
				return ;
			}

			if ( mform->ankeyopts2 != -1 && nkeyopts > mform->ankeyopts2 )
			{
				seterror( "PEDM011Y", 20 ) ;
				return ;
			}

			if ( mform->anvalopts1 != -1 && nvalopts < mform->anvalopts1 )
			{
				seterror( "PEDM011Z", 20 ) ;
				return ;
			}

			if ( mform->anvalopts2 != -1 && nvalopts > mform->anvalopts2 )
			{
				seterror( "PEDM012A", 20 ) ;
				return ;
			}
		}

		void set_macro( const string& s,
				map<string, stack<defName>>& defNames,
				bool force_pgmmac = false )
		{
			defName dn ;

			rmacro    = emacro ;
			cmd_macro = false ;
			pgm_macro = force_pgmmac ;

			if ( s.front() == '%' )
			{
				cmd_macro = true ;
				emacro    = s.substr( 1 ) ;
				auto it1 = defNames.find( emacro ) ;
				if ( it1 == defNames.end() )
				{
					it1 = defNames.find( upper( emacro ) ) ;
					if ( it1 != defNames.end() )
					{
						iupper( emacro ) ;
					}
				}
				if ( it1 == defNames.end() || !it1->second.top().exp )
				{
					dn.cmd = true ;
					defNames[ emacro ].push( dn ) ;
				}
			}
			else if ( s.front() == '!' )
			{
				pgm_macro = true ;
				emacro    = upper( s.substr( 1 ) ) ;
				auto it1 = defNames.find( emacro ) ;
				if ( it1 == defNames.end() || !it1->second.top().exp )
				{
					dn.pgm = true ;
					defNames[ emacro ].push( dn ) ;
				}
			}
			else
			{
				auto it1 = defNames.find( s ) ;
				if ( it1 != defNames.end() )
				{
					pgm_macro = it1->second.top().pgm ;
					cmd_macro = it1->second.top().cmd ;
				}
				else
				{
					auto it2 = defNames.find( upper( s ) ) ;
					if ( it2 != defNames.end() )
					{
						pgm_macro = it2->second.top().pgm ;
						cmd_macro = it2->second.top().cmd ;
					}
				}
				emacro = ( pgm_macro ) ? upper( s ) : s ;
			}
		}

		bool getMacroFileName( const string& paths )
		{
			setRC( 0 ) ;
			locator loc( paths, emacro ) ;
			loc.locate() ;
			if ( loc.errors() )
			{
				setRC( 28 ) ;
				return false ;
			}
			if ( loc.not_found() )
			{
				setRC( 8 ) ;
				return false ;
			}
			mfile  = loc.entry() ;
			mfound = true ;
			return true ;
		}

		bool scanOn() const
		{
			return scan ;
		}

		bool iskeep() const
		{
			return keep ;
		}

		bool editEnded() const
		{
			return eended ;
		}

		bool lcmacro() const
		{
			return ( lcmd != "" ) ;
		}

		int get_sttment_words() const
		{
			return sttwds ;
		}

		const string& get_truename( const string& w ) const
		{
			auto it = PrimCMDS.find( w ) ;
			return ( it == PrimCMDS.end() ) ? w : it->second.truename ;
		}

		string emacro    ;
		string rmacro    ;
		string mfile     ;
		string rxpath1   ;
		string rxpath2   ;
		pApplication* editAppl ;
		pApplication* macAppl ;
		string sttment   ;
		M_CMDS m_cmd     ;
		string keyword   ;
		string keyopts   ;
		string value     ;
		string parms     ;
		string var1      ;
		string var2      ;
		string msgid     ;
		string val1      ;
		string val2      ;
		string val3      ;
		string zerrmsg   ;
		string zerrsm    ;
		string zerrlm    ;
		string zerralrm  ;
		string zerrhm    ;
		string zerrtype  ;
		string zerrwind  ;
		string zldpath   ;
		string lcmd      ;
		bool   setzerr   ;
		bool   mfound    ;
		bool   macro     ;
		bool   messageOn ;
		bool   imacro    ;
		bool   process   ;
		bool   processed ;
		bool   cursorset ;
		bool   fatal     ;
		bool   assign    ;
		bool   query     ;
		bool   scan      ;
		bool   runmacro  ;
		bool   cmd_macro ;
		bool   pgm_macro ;
		bool   eended    ;
		bool   cancel    ;
		bool   editserv  ;
		bool   errorsRet ;
		bool   rexxError ;
		bool   isrError  ;
		bool   keep      ;
		int    ustatekey ;
		int    sttwds    ;
		int    etaskid   ;
		int    nestlvl   ;
		int    RC        ;
		int    RSN       ;
		int    exRC      ;

	friend class pedit01 ;
	friend class pedrxm1 ;
} ;


class lmac
{
	public:
		lmac()
		{
			lcname = "" ;
			lcmac  = "" ;
			lcpgm  = false ;
			lcdst  = false ;
		}

		string lcname ;
		string lcmac ;
		bool   lcpgm ;
		bool   lcdst ;
} ;


class setTrue
{
	public:
		setTrue( bool& b )
		{
			b = true ;
			s = &b ;
		}
		~setTrue()
		{
			*s = false ;
		}
		bool* s ;
} ;


class Cut
{
	public:

		Cut( cmdblock& pcmd,
		     bool profReplace,
		     string& s,
		     Data::Iterator it1,
		     Data::Iterator it2 ) : Cut()
		{
			parseString2( s, "DEFAULT" ) ;

			replace = profReplace ;
			mark    = parseString2( s, "MARKED" ) ;
			only_x  = parseString2( s, "X" ) ;
			only_nx = parseString2( s, "NX" ) ;

			if ( only_x && only_nx )
			{
				pcmd.set_msg( "PEDM012X", "X", "NX", "CUT", 12 ) ;
				return ;
			}
			if ( parseString2( s, "APPEND" ) )
			{
				replace = false ;
			}
			else if ( parseString2( s, "REPLACE R" ) )
			{
				replace = true ;
			}

			if ( s != "" )
			{
				if ( words( s ) == 1 )
				{
					clip = trim( s ) ;
				}
				else
				{
					pcmd.set_msg( "PEDT013A", 12 ) ;
					return ;
				}
			}

			its = it1 ;
			ite = it2 ;
			if ( ite != nullptr )
			{
				++ite ;
			}
		}

		Cut()
		{
			clip    = "DEFAULT" ;
			mark    = false ;
			replace = false ;
			only_x  = false ;
			only_nx = false ;
			its     = nullptr ;
			ite     = nullptr ;
		}

		void iterators( Data::Iterator i,
				Data::Iterator j )
		{
			its = i ;
			ite = j ;
		}

		bool repl()    { return replace ; }
		bool x()       { return only_x  ; }
		bool nx()      { return only_nx ; }
		bool marked()  { return mark    ; }
		bool norange() { return its == nullptr ; }

		Data::Iterator begin() { return its ; }
		Data::Iterator end()   { return ite ; }

		const string& clipBoard() { return clip ; }

	private:
		string clip ;

		bool mark ;
		bool replace ;
		bool only_x ;
		bool only_nx ;

		Data::Iterator its = nullptr ;
		Data::Iterator ite = nullptr ;
} ;


class Del
{
	public:
		Del( const string& c ) : Del()
		{
			del_ws = words( c ) ;
			if ( del_ws == 0 )
			{
				seterror( "PEDT013J", "DELETE" ) ;
				return ;
			}

			for ( int i = 1 ; i <= del_ws ; ++i )
			{
				string w = word( c, i ) ;
				if ( w == "ALL" )
				{
					if ( del_ALL )
					{
						seterror( "PEDT013J", "DELETE" ) ;
						return ;
					}
					del_ALL = true ;
				}
				else if ( w == "X" )
				{
					if ( del_X || del_NX )
					{
						seterror( "PEDT013J", "DELETE" ) ;
						return ;
					}
					del_X = true ;
				}
				else if ( w == "NX" )
				{
					if ( del_NX || del_X )
					{
						seterror( "PEDT013J", "DELETE" ) ;
						return ;
					}
					del_NX = true ;
				}
				else if ( w.front() == '.' )
				{
					if ( w == ".ZF" )
					{
						w = ".ZFIRST" ;
					}
					else if ( w == ".ZL" )
					{
						w = ".ZLAST" ;
					}
					if ( del_labela == "" )
					{
						del_labela = w ;
					}
					else if ( del_labelb == "" )
					{
						del_labelb = w ;
					}
					else
					{
						seterror( "PEDT013J", "DELETE" ) ;
						return ;
					}
				}
				else if ( isnumeric( w ) )
				{
					if ( del_linenum1 == 0 )
					{
						del_linenum1 = ds2d( w ) ;
						if ( del_linenum1 < 1 )
						{
							seterror( "PEDT013J", "DELETE" ) ;
							return ;
						}
					}
					else if ( del_linenum2 == 0 )
					{
						del_linenum2 = ds2d( w ) ;
						if ( del_linenum2 < 1 )
						{
							seterror( "PEDT013J", "DELETE" ) ;
							return ;
						}
						if ( del_linenum1 > del_linenum2 )
						{
							swap( del_linenum1, del_linenum2 ) ;
						}
					}
					else
					{
						seterror( "PEDT013J", "DELETE" ) ;
						return ;
					}
				}
				else
				{
					seterror( "PEDT013J", "DELETE" ) ;
					return ;
				}
			}
			if ( del_labela != "" && del_labelb != "" && del_linenum1 > 0 )
			{
				seterror( "PEDT013J", "DELETE" ) ;
				return ;
			}
			else if ( del_linenum1 > 0 && del_linenum2 > 0 && del_labela != "" )
			{
				seterror( "PEDT013J", "DELETE" ) ;
				return ;
			}
		}

		Del()
		{
			del_ws       = 0 ;
			del_ALL      = false ;
			del_X        = false ;
			del_NX       = false ;
			del_labela   = "" ;
			del_labelb   = "" ;
			del_linenum1 = 0  ;
			del_linenum2 = 0  ;
			del_RC       = 0  ;
			del_error    = "" ;
			del_val1     = "" ;
			del_validCSR = true ;
		}

		void seterror( const string& e,
			       int RC = 20 )
		{
			del_error = e ;
			del_val1  = "" ;
			del_RC    = RC ;
		}

		void seterror( const string& e,
			       const string& v,
			       int RC = 20 )
		{
			del_error = e ;
			del_val1  = v ;
			del_RC    = RC ;
		}

		int    del_ws  ;
		bool   del_ALL ;
		bool   del_X   ;
		bool   del_NX  ;
		string del_labela ;
		string del_labelb ;
		int    del_linenum1 ;
		int    del_linenum2 ;
		int    del_RC    ;
		string del_error ;
		string del_val1  ;
		bool   del_validCSR ;

		Data::Iterator del_its = nullptr ;
		Data::Iterator del_ite = nullptr ;
} ;


class line_data
{
	public:
		line_data()
		{
			line_type = LS_NONE ;
		}

		void set_string( iline& d )
		{
			line_str   = d.get_idata() ;
			line_len   = line_str.size() ;
			line_type  = LS_NONE ;
			line_label = "" ;
			if ( d.has_condition() )
			{
				line_type = d.get_condition() ;
			}
			if ( d.has_label() )
			{
				line_label = d.get_label() ;
			}
			line_excl = d.is_excluded() ;
		}

		void resize( size_t l )
		{
			line_str.resize( l, 0x00 ) ;
		}

		string get_string()
		{
			return line_str.substr( 0, line_len ) ;
		}

		size_t get_string_len()
		{
			return line_len ;
		}

		size_t  line_len ;
		LS_TYPE line_type ;
		bool    line_excl ;
		string  line_label ;
		string  line_str ;
} ;


class pedit01 : public pApplication
{
	public:
		pedit01() ;

		void application() ;

		void isredit( const string& ) ;

		map<string, stack<defName>> defNames ;
		cmdblock pcmd ;

	private:
		static map<int, edit_find>Global_efind_parms ;

		miblock miBlock ;

		Cursor cursor ;

		Cut cut ;

		extData extdata ;

		map<string, lang_colours> langColours ;
		map<string, string> langSpecials ;

		boost::condition cond_recov ;

		boost::thread* bThread ;

		void Edit() ;

		void addEditRecovery() ;
		void removeEditRecovery() ;

		void loadEditProfile( const string& ) ;
		void swapEditProfile( const string&,
				      vector<caution>& ) ;
		void saveEditProfile( const string& ) ;
		void createDefaultEditProfile( const string&,
					       const string&,
					       const string,
					       const string ) ;
		void createEditProfile( const string&,
					const string&,
					const string,
					const string ) ;

		bool existsEditProfile( const string&,
					const string&,
					const string& ) ;
		void resetEditProfile() ;

		void readLineCommandTable() ;
		void cleanup_custom()     ;
		void cleanup()            ;
		void initialise()         ;
		bool termOK()             ;
		void viewWarning1()       ;
		bool viewWarning2()       ;
		void readFile()           ;
		bool saveFile()           ;
		bool confirmCancel()      ;
		bool confirmMove( const string& ) ;
		bool confirmReplace( const string& ) ;
		void startRecoveryTask()  ;
		void stopRecoveryTask()   ;
		void updateRecoveryData() ;
		void fill_dynamic_area()  ;
		void fill_hilight_shadow();
		void clr_hilight_shadow() ;
		void protNonDisplayChars();
		void addNulls()           ;
		void getZAREAchanges()    ;
		void set_language_fvars( const string& ) ;
		void load_language_colours() ;
		void save_language_colours() ;
		bool set_language_colours( const string& ) ;

		string getColumnLine( int =0 ) ;

		void updateData() ;

		void issue_cautions( bool,
				     bool,
				     bool,
				     bool,
				     bool,
				     bool ) ;

		string updateLine( const ipos&,
				   iline*,
				   string,
				   string,
				   size_t,
				   int,
				   int ) ;

		void applyTabs( iline*,
				vector<size_t>&,
				string&,
				const string&,
				size_t,
				size_t = 0 ) ;

		void hiliteFindPhrase() ;

		void actionFind() ;
		bool actionFind_regex( iline*,
				       int,
				       int ) ;

		int actionFind_regex_wps( iline*,
					  int,
					  string::const_iterator,
					  string::const_iterator,
					  string::const_iterator,
					  string::const_iterator ) ;

		bool actionFind_nonregex( iline*,
					  int,
					  int,
					  int ) ;

		void actionChange() ;

		void check_delete( Del& ) ;

		void set_zhivars() ;

		int set_zdest() ;

		int set_zranges( const string&,
				 const string& ) ;

		bool storeLineCommands()  ;
		void actionPrimCommand1() ;
		void actionPrimCommand2() ;
		void actionPrimCommand3() ;
		void actionLineCommands() ;
		void actionLineCommand( lcmd&,
					bool&,
					bool& ) ;

		void run_imacro( const string&,
				 bool = false ) ;

		void run_macro( const string&,
				bool = false,
				bool = false,
				bool = false,
				const string& = "" ) ;

		void set_macro_cursor() ;

		void actionService() ;
		void querySetting() ;

		void clr_zedimsgs() ;
		void set_zedimsgs() ;
		void set_zedimsgs( const string& ) ;

		void action_ScrollLeft()  ;
		void action_ScrollRight() ;
		void action_ScrollUp()   ;
		void action_ScrollDown() ;
		void action_RFIND()   ;
		void action_RCHANGE() ;
		bool action_UNDO() ;
		bool action_REDO() ;

		int  maxCol() ;

		void load_profile( profile& ) ;

		void store_status( SS_TYPE ) ;
		void update_status() ;
		void restore_status( SS_TYPE ) ;

		void setLineLabels()  ;
		string genNextLabel( string ) ;
		const string& get_truename( const string& ) ;

		void removeRecoveryData() ;

		Data::Iterator getFirstEX( Data::Iterator ) ;
		Data::Iterator getLastEX( Data::Iterator ) ;
		int  getExclBlockSize( Data::Iterator ) ;
		int  getDataBlockSize( Data::Iterator ) ;

		iline* getLastADDR( Data::Iterator, int ) ;

		Data::Iterator getDataLine( int )  ;
		int    getFileLine( Data::Iterator ) ;

		iline* getFileLineZLABEL( const string& ) ;
		iline* getFileLineZFIRST() ;
		iline* getFileLineZLAST() ;

		iline* getDataLineNext( Data::Iterator ) ;

		Data::Iterator getFileLineNext( Data::Iterator ) ;

		Data::Iterator getNextDataLine( Data::Iterator ) ;
		Data::Iterator getNextFileLine( Data::Iterator ) ;

		Data::Iterator getPrevDataLine( Data::Iterator ) ;

		iline* getNextDataLine( iline* ) ;
		iline* getPrevDataLine( iline* ) ;

		iline* getNextFileLine( iline* ) ;
		iline* getPrevFileLine( iline* ) ;

		iline* getFileLinePrev( iline* ) ;
		iline* getFileLineNext( iline* ) ;

		string formLineData( Data::Iterator = nullptr ) ;

		int  isValidiline( iline* ) ;

		bool ADDROnScreen( iline*,
				   iline* ) ;

		void moveTopline( iline*,
				  bool = false ) ;

		uint countVisibleLines( Data::Iterator ) ;
		void moveDownLines( int = 1 ) ;

		string mergeLine( const string&,
				  Data::Iterator ) ;

		void cleanupRedoStacks() ;
		void updateProfLines( vector<string>& ) ;
		void buildProfLines( vector<string>&,
				     int i = 5 ) ;
		void removeProfLines() ;
		void removeSpecialLines( Data::Iterator,
					 Data::Iterator ) ;
		void processNewInserts() ;

		string convertTabs( string ) ;

		string& convertiTabs( string& ) ;

		void copyFileData( vector<line_data>&,
				   bool,
				   bool,
				   Data::Iterator,
				   Data::Iterator,
				   size_t& ) ;

		void reflowData( vector<string>&,
				 int,
				 int,
				 int ) ;

		bool is_line_blank( const string& ) ;

		string overlay1( string,
				 string,
				 bool& ) ;
		string overlay2( const string&,
				 string ) ;
		string templat( string,
				string ) ;
		bool formLineCmd1( const string&,
				   string&, int& ) ;
		bool formLineCmd2( const string&,
				   string&, int& ) ;

		void create_copy( Cut&,
				  vector<ipline>& ) ;

		bool copyToClipboard( vector<ipline>& ) ;
		void getClipboard( vector<ipline>& ) ;
		void clearClipboard( string ) ;

		void manageClipboard() ;
		void manageClipboard_create( vector<string>& ) ;
		void manageClipboard_descr( const string&,
					    const string& ) ;
		void manageClipboard_browse( const string& ) ;
		void manageClipboard_toggle( int,
					     const string& ) ;
		void manageClipboard_edit( const string&,
					   const string& ) ;
		void manageClipboard_rename( const string&,
					     const string& ) ;
		void manageClipboard_delete( const string& ) ;
		void manageClipboard_islocked( const string& ) ;

		string displayPanel( const string&,
				     const string&,
				     const string&,
				     bool = true,
				     bool = true ) ;

		void create_file( const string&,
				  Data::Iterator,
				  Data::Iterator ) ;

		void create_file( const string&,
				  iline*,
				  iline* ) ;

		void copyIn( const string&,
			     Data::Iterator,
			     int = -1,
			     int = -1 ) ;

		void positionCursor() ;
		void moveColumn( int = 0 ) ;
		void moveCursorEnter() ;

		void moveCursorLine( int,
				     iline* ) ;
		void moveCursorLine( int,
				     iline*,
				     size_t,
				     size_t ) ;
		void moveCursorLine( Data::Iterator ) ;

		bool moveCursorNB( int,
				   iline* ) ;

		bool moveCursorNB( int,
				   iline*,
				   size_t,
				   size_t ) ;

		bool moveCursorNB( iline*,
				   iline* ) ;

		int  get_minsize() ;
		int  get_overflow_size() ;

		bool setFindChangeExcl( char,
					bool = false ) ;
		void setFoundMsg() ;
		void setChangedMsg() ;
		void setChangedError() ;
		void setExcludedMsg() ;
		void setNotFoundMsg() ;
		void setCursorFind() ;
		void setCursorFindChange() ;
		void setCursorChange() ;

		bool extract_range( string,
				    cmd_range& ) ;
		bool extract_labels( string&,
				     iline*&,
				     iline*& ) ;
		int extract_lptr( string&,
				  Data::Iterator&,
				  Data::Iterator&,
				  bool = true,
				  bool = true ) ;

		iline* getNextSpecial( iline*,
				       Data::Iterator,
				       Data::Iterator,
				       char,
				       char ) ;

		iline* locateCBLlinenum( iline*,
					 const string& ) ;

		iline* locateSTDlinenum( iline*,
					 const string& ) ;

		int getLinePtrIterator( const string&,
					Data::Iterator& ) ;

		Data::Iterator getLabelIterator( const string&,
						 int& ) ;

		iline* getLabelAddress( const string&,
					int& ) ;

		int  getRow( iline* ) ;

		int  get_datawidth() ;

		bool checkLabel1( const string&,
				  int = 0 ) ;
		bool checkLabel2( const string&,
				  int = 0 ) ;

		bool getSoftwareTabLocation( size_t&,
					     size_t&,
					     bool& ) ;

		void getLogicalTabLocations( vector<size_t>& ) ;
		bool onHardwareTab( size_t ) ;

		string expandFileName( const string& ) ;

		void copyPrefix( ipline&,
				 Data::Iterator ) ;
		void copyPrefix( Data::Iterator,
				 const ipline&,
				 bool = false ) ;

		Data::Iterator addSpecial( LN_TYPE,
					   Data::Iterator,
					   vector<string>& ) ;
		void addSpecial( Data::Iterator,
				 vector<caution>& ) ;

		Data::Iterator addSpecial( LN_TYPE,
					   Data::Iterator,
					   const string& ) ;

		void removeSpecial( CA_TYPE ) ;

		void removeSpecial( CA_TYPE,
				    CA_TYPE ) ;

		void removeSpecial( CA_TYPE,
				    CA_TYPE,
				    CA_TYPE ) ;

		string getMaskLine1() ;
		string getMaskLine2() ;

		void sort_data( const string& ) ;

		string rshiftCols( int,
				   const string* ) ;
		string lshiftCols( int,
				   const string* ) ;

		bool   rshiftData( int,
				   const string*,
				   string& ) ;
		bool   lshiftData( int,
				   const string*,
				   string& ) ;

		bool   textSplitData( const string&,
				      string&,
				      string& ) ;

		void   compare_files( string ) ;
		void   compare_exclude( const string& ) ;

		string createTempName() ;
		string determineLang()  ;

		void   check_number_parm( const string&,
					  set<string>&, const string& ) ;

		void   set_default_bounds() ;
		void   set_num_parameters() ;
		void   unnum_data() ;
		void   renum_data() ;
		void   add_line_numbers() ;
		void   add_missing_line_numbers() ;

		bool   is_defName( const string& ) ;
		void   set_msg_variables() ;

		void   qstrings( const string&,
				 vector<string>&,
				 int ) ;

		void   get_profile_var( const string&,
					string& ) ;

		string get_profile_var( const string& ) ;

		void   set_profile_var( const string&,
					string& ) ;

		void   get_shared_var( const string&,
				       string& ) ;

		string get_shared_var( const string& ) ;

		void   set_shared_var( const string&,
				       const string& ) ;

		void   set_dialogue_var( const string&,
					 const string& ) ;

		void   set_macro_var( const string&,
				      const string& ) ;

		void   clear_user_states() ;

		void   updateStatistics() ;
		void   removeStatistics() ;

		void clearMacroLabels( int mlvl )
		{
			int lvl = level ;
			for_each( data.begin(), data.end(),
				[ lvl, mlvl ](iline& a)
				{
					a.clearLabel( lvl, mlvl ) ;
				} ) ;
		}


		void term_resize() ;

		bool is_pgmmacro( const string& ) ;

		string file_recfm() { return ( reclen == 0 ) ? "U" : "F" ; }
		string file_lrecl() { return ( reclen == 0 ) ? "" : d2ds( reclen ) ; }

		iline* topLine           ;
		iline* modLine           ;
		iline* ptopLine          ;
		int  XRC                 ;
		int  XRSN                ;
		int  startCol            ;
		iline* mRow              ;
		int  mCol                ;
		int  aRow                ;
		int  aCol                ;
		int  aOff                ;
		iline* aADDR             ;
		bool aLCMD               ;
		bool aDATA               ;
		bool macroRunning        ;
		int  level               ;
		int  saveLevel           ;
		int  nestLevel           ;
		int  XTabz               ;
		int  targetLine          ;
		size_t lnumSize          ;
		size_t lnumSize1         ;
		size_t lnumSize2         ;
		size_t lnumS2pos         ;

		bool tabsOnRead          ;
		bool abendComplete       ;

		bool   profAutoSave      ;
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
		bool   profRecovery      ;
		bool   profBackup        ;
		bool   profHilight       ;
		bool   profIfLogic       ;
		bool   profDoLogic       ;
		bool   profLogic         ;
		bool   profParen         ;
		bool   profCutReplace    ;
		bool   profPasteKeep     ;
		bool   profPosFcx        ;
		string profLang          ;
		string profIMACRO        ;
		bool   profVert          ;
		bool   profFindPhrase    ;
		bool   profCsrPhrase     ;
		bool   profUndoKeep      ;
		bool   profNum           ;
		bool   profNumSTD        ;
		bool   profNumCBL        ;
		bool   profNumDisp       ;
		bool   profAutoNum       ;
		bool   profNotes         ;
		bool   profStats         ;

		string detLang           ;

		string dataid1           ;
		string dataid2           ;

		string spfedit           ;

		string zcmd              ;
		string zverb             ;
		string zpfkey            ;
		string zapplid           ;
		string zuser             ;
		string zscreen           ;
		string zscrnum           ;
		string zhome             ;
		string zuprof            ;
		string zedbfile          ;
		string zeduser           ;
		string zedoclr           ;
		string zedfclr           ;
		string zedfhlt           ;
		string zedpclr           ;

		string zedclrd           ;
		string zedclrc           ;
		string zedclrk           ;
		string zedclrq           ;
		string zedclrv           ;
		string zedclrs           ;

		string zedhid           ;
		string zedhic           ;
		string zedhik           ;
		string zedhiq           ;
		string zedhiv           ;
		string zedhis           ;

		string zedphlt           ;
		string zedlctab          ;
		string zvmode            ;
		string zusermc1          ;

		int    zscreend          ;
		int    zscreenw          ;

		bool   zedvmode          ;
		bool   zedcwarn          ;

		bool optNoConvTabs       ;
		bool optPreserve         ;
		bool optConfCancel       ;
		bool optFindPhrase       ;

		bool pfkeyPressed        ;
		bool termEdit            ;
		bool stripST             ;
		bool convTabs            ;
		bool convSpaces          ;
		bool cutActive           ;
		bool cutReplace          ;
		bool pasteActive         ;
		bool copyActive          ;
		bool moveActive          ;
		bool pasteKeep           ;
		bool hideExcl            ;
		bool scrollData          ;
		bool editRecovery        ;
		bool explProfile         ;

		bool edrec_done          ;

		int  LeftBnd             ;
		int  RightBnd            ;
		bool LeftBndSet          ;
		bool RightBndSet         ;

		string lnummod           ;

		string maskLine          ;
		bool   zchanged          ;
		bool   ichgwarn          ;
		bool   dataUpdated       ;
		bool   colsOn            ;
		string tabsLine          ;
		char   tabsChar          ;
		string backupLoc         ;
		iline* misADDR           ;

		Data data ;

		vector<ipos> s2data ;

		vector<bool> lChanged ;
		vector<bool> dChanged ;
		vector<bool> dTouched ;

		vector<lcmd> lcmds ;

		set<iline*> noAttrTabs ;

		edit_find fcx_parms ;
		hilight hlight      ;

		bool rebuildZAREA  ;
		bool rebuildShadow ;
		bool fileChanged   ;

		string curfld  ;
		int    curpos  ;

		string panel   ;
		string zfile   ;
		string bfile   ;
		string zcol1   ;
		string zcol2   ;
		string zarea   ;
		string zshadow ;
		int    zareaw  ;
		int    zaread  ;
		int    zlvline ;
		int    zolen   ;
		uint   zdataw  ;
		uint   zasize  ;
		string carea   ;
		string cshadow ;
		string xarea   ;
		string marea   ;
		string fscroll ;

		string zscrolla ;
		int    zscrolln ;
		string zcurfld  ;
		int    zcurpos  ;

		iline* zdest    ;
		iline* zfrange  ;
		iline* zlrange  ;
		string zrange_cmd ;

		int    tflow1   ;
		int    tflow2   ;

		string zedprof  ;
		string zedproft ;

		string zedptype ;
		string zedplrcl ;
		string zedprcfm ;
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

		string edimac   ;
		string edprof   ;
		string edlmac   ;
		string edmparm  ;

		int    reclen   ;

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
		string slr  ;
		string slb  ;
		string slw  ;

		string slgu ;
		string slru ;

		string type ;
		string str  ;
		string str2 ;
		string occ  ;
		string lines;

		RECV_STATUS recvStatus ;

		bool recovSusp ;
		bool canBackup ;

		map<string, user_state*> user_states ;

		map<string, void(pedit01::*)()>scrollList = { { "UP",    &pedit01::action_ScrollUp    },
							      { "DOWN",  &pedit01::action_ScrollDown  },
							      { "LEFT",  &pedit01::action_ScrollLeft  },
							      { "RIGHT", &pedit01::action_ScrollRight } } ;

		map<string, void(pedit01::*)()>zverbList  = { { "RFIND",   &pedit01::action_RFIND   },
							      { "RCHANGE", &pedit01::action_RCHANGE } } ;

		map<char, string>fcxList = { { 'F', "FIND"    },
					     { 'C', "CHANGE"  },
					     { 'X', "EXCLUDE" } } ;

		map<string, char>locList = { { "CHA",  'C' },
					     { "CMD",  'K' },
					     { "ERR",  'E' },
					     { "INFO", 'I' },
					     { "LAB",  'L' },
					     { "MSG",  'M' },
					     { "NOTE", 'N' },
					     { "REDO", 'U' },
					     { "SPE",  'S' },
					     { "T",    'T' },
					     { "UNDO", 'U' },
					     { "X",    'X' } } ;

		map<bool, string>OnOff   = { { true,  "ON"  },
					     { false, "OFF" } } ;

		map<bool, string>YesNo   = { { true,  "YES" },
					     { false, "NO"  } } ;

		map<bool, char>ZeroOne   = { { true,  '1'   },
					     { false, '0'   } } ;

		map<string,L_CMDS> lineCmds  = { { "A",    LC_A      },
						 { "AK",   LC_AK     },
						 { "B",    LC_B      },
						 { "BK",   LC_BK     },
						 { "BNDS", LC_BOUNDS },
						 { "C",    LC_C      },
						 { "CC",   LC_CC     },
						 { "COLS", LC_COLS   },
						 { "D",    LC_D      },
						 { "DD",   LC_DD     },
						 { "F",    LC_F      },
						 { "HX",   LC_HX     },
						 { "HXX",  LC_HXX    },
						 { "I",    LC_I      },
						 { "L",    LC_L      },
						 { "LC",   LC_LC     },
						 { "LCC",  LC_LCC    },
						 { "M",    LC_M      },
						 { "MM",   LC_MM     },
						 { "MASK", LC_MASK   },
						 { "MD",   LC_MD     },
						 { "MDD",  LC_MDD    },
						 { "MN",   LC_MN     },
						 { "MNN",  LC_MNN    },
						 { "O",    LC_O      },
						 { "OK",   LC_OK     },
						 { "OO",   LC_OO     },
						 { "OOK",  LC_OOK    },
						 { "R",    LC_R      },
						 { "RR",   LC_RR     },
						 { "S",    LC_S      },
						 { "SI",   LC_SI     },
						 { "TABS", LC_TABS   },
						 { "TF",   LC_TFLOW  },
						 { "TJ",   LC_TJ     },
						 { "TJJ",  LC_TJJ    },
						 { "TR",   LC_TR     },
						 { "TRR",  LC_TRR    },
						 { "TS",   LC_TS     },
						 { "T",    LC_T      },
						 { "TT",   LC_TT     },
						 { "TX",   LC_TX     },
						 { "TXX",  LC_TXX    },
						 { "UC",   LC_UC     },
						 { "UCC",  LC_UCC    },
						 { "W",    LC_W      },
						 { "WW",   LC_WW     },
						 { "X",    LC_X      },
						 { "XX",   LC_XX     },
						 { "XI",   LC_XI     },
						 { ")",    LC_SRC    },
						 { "))",   LC_SRCC   },
						 { "(",    LC_SLC    },
						 { "((",   LC_SLCC   },
						 { ">",    LC_SRD    },
						 { ">>",   LC_SRDD   },
						 { "<",    LC_SLD    },
						 { "<<",   LC_SLDD   },
						 { "]",    LC_SRTC   },
						 { "]]",   LC_SRTCC  },
						 { "[",    LC_SLTC   },
						 { "[[",   LC_SLTCC  } } ;

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

		map<string,string> aliasNames = { { "CHAR",     "CHARS"    },
						  { "CHG",      "CHA"      },
						  { "CHANGE",   "CHA"      },
						  { "CMDS",     "CMD"      },
						  { "COB",      "COBOL"    },
						  { "COM",      "CMD"      },
						  { "COMMAND",  "CMD"      },
						  { "COMMANDS", "CMD"      },
						  { "CURSOR",   "CUR"      },
						  { "DIS",      "DISABLED" },
						  { "DISAB",    "DISABLED" },
						  { "DISABLE",  "DISABLED" },
						  { "DISP",     "DISPLAY"  },
						  { "DISPL",    "DISPLAY"  },
						  { "DO",       "DOLOGIC"  },
						  { "ERROR",    "ERR"      },
						  { "ERRORS",   "ERR"      },
						  { "EX",       "X"        },
						  { "EXC",      "X"        },
						  { "EXCLUDE",  "X"        },
						  { "EXCLUDED", "X"        },
						  { "IF",       "IFLOGIC"  },
						  { "INFOLINE", "INFO"     },
						  { "HIDE",     "H"        },
						  { "LABEL",    "LAB"      },
						  { "LABELS",   "LAB"      },
						  { "NOCOB",    "NOCOBOL"  },
						  { "MRK",      "T"        },
						  { "MARK",     "T"        },
						  { "MARKED",   "T"        },
						  { "MSGLINE",  "MSG"      },
						  { "NOCOB",    "NOCOBOL"  },
						  { "NOTELINE", "NOTE"     },
						  { "PRE",      "PREFIX"   },
						  { "PROF",     "PROFILE"  },
						  { "REC",      "RECOVER"  },
						  { "RECOVERY", "RECOVER"  },
						  { "SPECIAL",  "SPE"      },
						  { "STD",      "STANDARD" },
						  { "STG",      "STORAGE"  },
						  { "STO",      "STORAGE"  },
						  { "STOR",     "STORAGE"  },
						  { "STORE",    "STORAGE"  },
						  { "SUF",      "SUFFIX"   },
						  { "VERT",     "VERTICAL" } } ;

		map<string, string> abokLCMDS = { { "AK",  "A"  },
						  { "BK",  "B"  },
						  { "OK",  "O"  },
						  { "OOK", "OO" } } ;

		set<string> blkCmds  = { { "CC"   },
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

		set<string> todCmds  = { { "A"    },
					 { "I"    } } ;

		set<string> bodCmds  = { { "B"    },
					 { "COLS" },
					 { "BNDS" },
					 { "MASK" },
					 { "TABS" } } ;

		set<string> abokReq  = { { "C"    },
					 { "CC"   },
					 { "M"    },
					 { "MM"   } } ;

		set<string> checkDst = { { "C"    },
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

		set<string> abowList = { { "A"    },
					 { "AK"   },
					 { "B"    },
					 { "BK"   },
					 { "O"    },
					 { "OK"   },
					 { "OO"   },
					 { "OOK"  },
					 { "W"    },
					 { "WW"   } } ;

		set<string> owBlock  = { { "OO"   },
					 { "WW"   } } ;

		set<string> reptOk   = { { "A"    },
					 { "AK"   },
					 { "B"    },
					 { "BK"   },
					 { "C"    },
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
					 { "TF"   },
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

		map<string, lmac> lmacs ;

		void iincr( iline*& ) ;
		void idecr( iline*& ) ;

		iline* itr2ptr( Data::Iterator it )
		{
			assert ( it != nullptr ) ;

			return ( it == data.end() ) ? nullptr : &( *it ) ;
		}

		Data::Iterator delete_line( Data::Iterator,
					    bool = false ) ;


		friend class pedrxm1 ;
		friend class pedmcp1 ;
} ;
