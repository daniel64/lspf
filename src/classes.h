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


using namespace lspf ;

#define INC_IMBLK     0b00000001
#define INC_ALPHA     0b00000010
#define INC_ALPHANUM  0b00001010
#define INC_ALPHAB    0b00000100
#define INC_ALPHABNUM 0b00001100
#define INC_NUM       0b00001000
#define INC_ALL3      0b00001110


enum TOKEN_TYPES
{
	TT_STRING_QUOTED,
	TT_STRING_UNQUOTED,
	TT_VARIABLE,
	TT_OTHER,
	TT_EOT
} ;


enum TOKEN_SUBTYPES
{
	TS_NAME,
	TS_AMPR_VAR_VALID,
	TS_AMPR_VAR_INVALID,
	TS_CTL_VAR_VALID,
	TS_CTL_VAR_INVALID,
	TS_COMPARISON_OP,
	TS_OPEN_BRACKET,
	TS_CLOSE_BRACKET,
	TS_COMMA,
	TS_EQUALS,
	TS_NONE
} ;


enum STATEMENT_TYPE
{
	ST_IF,
	ST_ELSE,
	ST_ERROR,
	ST_VGET,
	ST_VPUT,
	ST_ASSIGN,
	ST_GOTO,
	ST_EXIT,
	ST_REFRESH,
	ST_VEDIT,
	ST_PANEXIT,
	ST_PREXX,
	ST_VERIFY,
	ST_EOF
} ;


enum RL_COND
{
	RL_EQ,
	RL_NE,
	RL_GT,
	RL_GE,
	RL_LE,
	RL_LT
} ;


enum VER_TYPE
{
	VER_ALPHA,
	VER_ALPHAB,
	VER_BIT,
	VER_ENUM,
	VER_HEX,
	VER_INCLUDE,
	VER_IPADDR4,
	VER_IPADDR6,
	VER_LEN,
	VER_LIST,
	VER_LISTX,
	VER_LISTV,
	VER_LISTVX,
	VER_NA,
	VER_NAME,
	VER_NUMERIC,
	VER_OCT,
	VER_PICT,
	VER_PICTCN,
	VER_RANGE,
	VER_STDDATE,
	VER_STDTIME
} ;


enum VL_COND
{
	VL_EQ,
	VL_NE,
	VL_GT,
	VL_GE,
	VL_LE,
	VL_LT
} ;


enum PN_FUNCTION
{
	PN_DIR,
	PN_EXISTS,
	PN_FILE,
	PN_LENGTH,
	PN_LVLINE,
	PN_PFK,
	PN_TRANS,
	PN_TRUNC,
	PN_REVERSE,
	PN_WORDS,
	PN_UPPER,
	PN_NONE
} ;


enum CV_CONTROL
{
	CV_ALARM,
	CV_AUTOSEL,
	CV_BROWSE,
	CV_CSRPOS,
	CV_CSRROW,
	CV_CURSOR,
	CV_EDIT,
	CV_FALSE,
	CV_HELP,
	CV_HHELP,
	CV_HIST,
	CV_MSG,
	CV_NRET,
	CV_PFKEY,
	CV_RESP,
	CV_TRUE,
	CV_TRAIL,
	CV_ZVARS
} ;


enum PS_SECT
{
	PS_INIT,
	PS_REINIT,
	PS_PROC,
	PS_ABCINIT,
	PS_ABCPROC
} ;


enum PGM_TYPE
{
	PGM_NONE,
	PGM_PANEL,
	PGM_REXX,
	PGM_SHELL,
} ;


enum FT_TYPE
{
	FT_BLANK,
	FT_DEFAULT,
	FT_DO,
	FT_ENDDO,
	FT_DOT,
	FT_ENDDOT,
	FT_ELSE,
	FT_IF,
	FT_IM,
	FT_ITERATE,
	FT_LEAVE,
	FT_NOP,
	FT_REXX,
	FT_SEL,
	FT_ENDSEL,
	FT_SET,
	FT_SETF,
	FT_TB,
	FT_TBA
} ;


enum tTOKEN
{
	tINTEGER,
	tPLUS,
	tMINUS,
	tMUL,
	tDIV,
	tPOWER,
	tLPAREN,
	tRPAREN,
	tEOF,
	tNONE
} ;


enum ECAPS
{
	CAPS_ON,
	CAPS_OFF,
	CAPS_IN,
	CAPS_OUT
} ;


enum SR_COND
{
	S_EQ,
	S_NE,
	S_LE,
	S_LT,
	S_GE,
	S_GT
} ;


enum LM_DISP
{
	LM_SHR,
	LM_EXCLU
} ;


enum ENT_TYPE
{
	ENT_BLOCK,
	ENT_CHAR,
	ENT_DIR,
	ENT_FIFO,
	ENT_FILE,
	ENT_SOCKET,
	ENT_SYML,
	ENT_UNKNOWN,
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************     Variable parm class       ********************************* */
/* ************************************ ***************************** ********************************* */


class vparm
{
	public:
		vparm()
		{
			clear() ;
		}

		void clear()
		{
			value   = "" ;
			subtype = TS_NONE ;
		}

		string value ;
		TOKEN_SUBTYPES subtype ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************           Token class         ********************************* */
/* ************************************ ***************************** ********************************* */


class token
{
	public:
		explicit token() : token( TT_OTHER )
		{
			idx = -1 ;
		}

		explicit token( TOKEN_TYPES tt )
		{
			value1  = "" ;
			value2  = "" ;
			type    = tt ;
			subtype = TS_NONE ;
		}

		void clear()
		{
			value1  = "" ;
			value2  = "" ;
			type    = TT_OTHER ;
			subtype = TS_NONE ;
		}

		string value1 ;
		string value2 ;
		int    idx ;
		TOKEN_TYPES    type ;
		TOKEN_SUBTYPES subtype ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************          PARSER class         ********************************* */
/* ************************************ ***************************** ********************************* */


class parser
{
	public:
		parser()
		{
			idx      = 0 ;
			optUpper = false ;
		}

		void    parse( errblock&,
			       const string& ) ;
		token&  getFirstToken() ;
		token&  getNextToken() ;
		token   getToken( uint ) ;
		int     getEntries() ;
		void    eraseTokens( int ) ;
		void    optionUpper()   { optUpper = true ; }
		bool    getNextIfCurrent( const string&  ) ;
		bool    getNextIfCurrent( TOKEN_TYPES    ) ;
		bool    getNextIfCurrent( TOKEN_SUBTYPES ) ;
		string  peekNextValue() ;
		TOKEN_SUBTYPES peekNextsubType() ;
		token&  getCurrentToken() ;
		const string& getCurrentValue() ;
		void    getNameList( errblock&, string& ) ;
		bool    isCurrentType( TOKEN_TYPES ) ;
		bool    isCurrentSubType( TOKEN_SUBTYPES ) ;
		void    getNextString( errblock& err,
				       string::const_iterator&,
				       const string& s,
				       string& r,
				       bool& ) ;
		STATEMENT_TYPE getStatementType() ;

	private:
		uint idx ;
		bool optUpper ;
		set<string> ctl_valid = { { ".ALARM"   },
					  { ".AUTOSEL" },
					  { ".BROWSE"  },
					  { ".CSRPOS"  },
					  { ".CSRROW"  },
					  { ".CURSOR"  },
					  { ".EDIT"    },
					  { ".FALSE"   },
					  { ".HELP"    },
					  { ".HHELP"   },
					  { ".HIST"    },
					  { ".MSG"     },
					  { ".NRET"    },
					  { ".PFKEY"   },
					  { ".RESP"    },
					  { ".TRUE"    },
					  { ".TRAIL"   },
					  { ".ZVARS"   } } ;

		map<string, STATEMENT_TYPE> statement_types =
		{ { "*REXX",     ST_PREXX   },
		  { ".ATTR",     ST_ASSIGN  },
		  { ".ATTRCHAR", ST_ASSIGN  },
		  { "ELSE",      ST_ELSE    },
		  { "EXIT",      ST_EXIT    },
		  { "GOTO",      ST_GOTO    },
		  { "PANEXIT",   ST_PANEXIT },
		  { "REFRESH",   ST_REFRESH },
		  { "VEDIT",     ST_VEDIT   },
		  { "VER",       ST_VERIFY  },
		  { "VGET",      ST_VGET    },
		  { "VPUT",      ST_VPUT    },
		  { "IF",        ST_IF      },
		  { "",          ST_EOF     } } ;

		set<string> compare_ops =
		{ { "!=" },
		  { ">"  },
		  { "<"  },
		  { ">=" },
		  { "!<" },
		  { "<=" },
		  { "!>" } } ;

		token current_token ;
		vector<token>tokens ;
} ;

/* ************************************ ***************************** ********************************* */
/* ************************************           tToken class        ********************************* */
/* ************************************ ***************************** ********************************* */


class ttoken
{
	public:
		ttoken()
		{
			type  = tEOF ;
			value = ""   ;
		}

		ttoken( tTOKEN a, const string& b )
		{
			type  = a ;
			value = b ;
		}

	string value ;
	tTOKEN type  ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************           Lexer class         ********************************* */
/* ************************************ ***************************** ********************************* */

class Lexer
{
	public:

		Lexer()
		{
		}

		Lexer( const string& s )
		{
			text = s ;
			pos  = 0 ;
			current_char  = text[ pos ] ;
			current_token = ttoken( tNONE, "" ) ;
		}

		size_t pos  ;
		string text ;
		ttoken current_token ;
		char   current_char  ;

		void   advance() ;
		void   skip_whitespace() ;
		string str() ;
		string integer() ;
		ttoken get_next_token( errblock& ) ;
} ;



/* ************************************ ***************************** ********************************* */
/* ************************************       Interpreter class       ********************************* */
/* ************************************ ***************************** ********************************* */

class Interpreter
{
	public:
		Interpreter( errblock& err,
			     const Lexer& l )
		{
			lexer = l ;
			current_token = lexer.get_next_token( err ) ;
		}

		Lexer  lexer ;
		ttoken current_token ;

		void eat( errblock&,
			  tTOKEN ) ;

		int  factor( errblock& ) ;
		int  term( errblock& ) ;
		int  expr( errblock& ) ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************     Point-and-shoot class     ********************************* */
/* ************************************ ***************************** ********************************* */


class pnts
{
	public:
		pnts()
		{
			pnts_fvar = false ;
		}

		pnts( errblock& err,
		      const string& s ) : pnts()
		{
			parse( err, s ) ;
		}

		string pnts_field ;
		string pnts_var ;
		string pnts_val ;

		bool   pnts_fvar ;

	private:
		void parse( errblock&, string ) ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************           Help class          ********************************* */
/* ************************************ ***************************** ********************************* */


class help
{
	public:
		help()
		{
			help_passthru = false ;
		}

		help( const string& rp ) : help()
		{
			help_rp      = rp ;
			help_missing = true ;
		}

		help( errblock& err,
		      const string& s ) : help()
		{
			parse( err, s ) ;
		}

		string help_field ;
		string help_panel ;
		string help_msg ;
		string help_rp ;
		bool   help_missing ;
		bool   help_passthru ;

	 private:
		void parse( errblock&, string ) ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************           TRUNC class         ********************************* */
/* ************************************ ***************************** ********************************* */


class TRUNC
{
	public:
		TRUNC()
		{
			trnc_field.clear() ;
			trnc_char = ' ' ;
			trnc_len  = 0 ;
		}

		void parse( errblock&,
			    parser&,
			    bool = true ) ;

		vparm  trnc_field ;
		char   trnc_char ;
		int    trnc_len ;

} ;


/* ************************************ ***************************** ********************************* */
/* ************************************           TRANS class         ********************************* */
/* ************************************ ***************************** ********************************* */


class TRANS
{
	public:
		TRANS()
		{
			trns_field.clear() ;
			trns_msgid   = "" ;
			trns_default = "" ;
			trns_tbfield = false ;
			trns_trunc   = nullptr ;
		}

		~TRANS()
		{
			delete trns_trunc ;
		}
		void parse( errblock&,
			    parser&,
			    bool = true ) ;

		TRUNC* trns_trunc ;
		vparm  trns_field ;
		string trns_msgid ;
		string trns_default ;
		bool   trns_tbfield ;
		vector<pair<string,string>> trns_list ;
} ;



/* ************************************ ***************************** ********************************* */
/* ************************************       Assignment class        ********************************* */
/* ************************************ ***************************** ********************************* */


class ASSGN
{
	public :
		ASSGN()
		{
			as_lhs.clear() ;
			as_rhs.clear() ;
			as_isattr  = false ;
			as_isattc  = false ;
			as_trunc   = nullptr ;
			as_trans   = nullptr ;
			as_func    = PN_NONE ;
		}

		~ASSGN()
		{
			delete as_trunc ;
			delete as_trans ;
		}

		void parse( errblock&,
			    parser& ) ;

		vparm  as_lhs ;
		vparm  as_rhs ;
		TRUNC* as_trunc ;
		TRANS* as_trans ;
		PN_FUNCTION as_func ;
		bool   as_isattr ;
		bool   as_isattc ;
} ;



/* ************************************ ***************************** ********************************* */
/* ************************************           VEDIT class         ********************************* */
/* ************************************ ***************************** ********************************* */


class VEDIT
{
	public:
		VEDIT()
		{
			ved_var   = "" ;
			ved_msgid = "" ;
		}

		void parse( errblock&,
			    parser& ) ;

		string ved_var ;
		string ved_msgid ;

} ;


/* ************************************ ***************************** ********************************* */
/* ************************************          VERIFY class         ********************************* */
/* ************************************ ***************************** ********************************* */


class VERIFY
{
	public:
		VERIFY()
		{
			ver_var     = "" ;
			ver_type    = VER_NA ;
			ver_msgid   = "" ;
			ver_len     = -1 ;
			ver_nblank  = false ;
			ver_tbfield = false ;
			ver_pnfield = false ;
			ver_include = 0x00 ;
		}

		void parse( errblock&,
			    parser&,
			    bool = true ) ;

		string   ver_var ;
		string   ver_msgid ;
		size_t   ver_len ;
		VER_TYPE ver_type ;
		bool     ver_nblank ;
		bool     ver_tbfield ;
		bool     ver_pnfield ;
		char     ver_include ;
		VL_COND  ver_cond ;

		vector<string> ver_vlist ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************        VGET/VPUT class        ********************************* */
/* ************************************ ***************************** ********************************* */


class VPUTGET
{
	public:
		VPUTGET()
		{
			vpg_pool = ASIS ;
		}

		void parse( errblock&,
			    parser& ) ;
		bool     vpg_vput ;
		string   vpg_vars ;
		poolType vpg_pool ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************      IF Statement class       ********************************* */
/* ************************************ ***************************** ********************************* */


class IFSTMNT
{
	public :
		IFSTMNT()
		{
			if_lhs.clear() ;
			if_rhs.clear() ;
			if_true   = false ;
			if_AND    = false ;
			if_else   = false ;
			if_verify = nullptr ;
			if_trunc  = nullptr ;
			if_trans  = nullptr ;
			if_func   = PN_NONE ;
			if_next   = nullptr ;
		}
		~IFSTMNT()
		{
			delete if_verify ;
			delete if_trunc ;
			delete if_trans ;
			delete if_next ;
		}

		void parse( errblock&,
			    parser& ) ;

		vparm    if_lhs ;
		vector<vparm> if_rhs ;
		VERIFY*  if_verify ;
		TRUNC*   if_trunc ;
		TRANS*   if_trans ;
		IFSTMNT* if_next ;
		PN_FUNCTION if_func ;
		bool     if_true ;
		bool     if_AND ;
		bool     if_else ;
		RL_COND  if_cond ;

	private:
		void parse_cond( errblock&,
				 parser& ) ;

		void parse_cond_continue( errblock&,
					  parser& ) ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************     *REXX Statement class     ********************************* */
/* ************************************ ***************************** ********************************* */


class PREXX
{
	public :
		PREXX()
		{
			all_vars = false ;
			rexx_var = false ;
			vars_var = false ;
		}

		bool rexx_inline()
		{
			return ( rexx == "" ) ;
		}

		bool rexx_variable()
		{
			return rexx_var ;
		}

		bool vars_variable()
		{
			return vars_var ;
		}

		void parse( errblock&,
			    parser& ) ;

		string      rexx ;
		set<string> vars ;
		bool        all_vars ;
		bool        rexx_var ;
		bool        vars_var ;

	private:
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************   PANEXIT Statement class     ********************************* */
/* ************************************ ***************************** ********************************* */

class PANEXIT : public PREXX
{
	public :
		PANEXIT() : PREXX()
		{
			is_rexx = true ;
		}

		void parse( errblock&,
			    parser& ) ;

		string msgid ;
		string exdata ;
		bool   is_rexx ;

	private:
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************     Panel statement class     ********************************* */
/* ************************************ ***************************** ********************************* */


class panstmnt
{
	public :
		panstmnt()
		{
			ps_label   = "" ;
			ps_rlist   = "" ;
			ps_column  = 0 ;
			ps_exit    = false ;
			ps_goto    = false ;
			ps_noop    = false ;
			ps_refresh = false ;
			ps_assign  = nullptr ;
			ps_if      = nullptr ;
			ps_else    = nullptr ;
			ps_panexit = nullptr ;
			ps_prexx   = nullptr ;
			ps_vedit   = nullptr ;
			ps_ver     = nullptr ;
			ps_vputget = nullptr ;
		}

		panstmnt( size_t column ) : panstmnt()
		{
			ps_column = column ;
		}

		~panstmnt()
		{
			delete ps_assign ;
			delete ps_if ;
			delete ps_panexit ;
			delete ps_prexx ;
			delete ps_vedit ;
			delete ps_ver ;
			delete ps_vputget ;
		}

		string   ps_label ;
		string   ps_rlist ;
		int      ps_column ;
		bool     ps_exit ;
		bool     ps_goto ;
		bool     ps_noop  ;
		bool     ps_refresh ;
		ASSGN*   ps_assign ;
		IFSTMNT* ps_if ;
		IFSTMNT* ps_else ;
		PANEXIT* ps_panexit ;
		PREXX*   ps_prexx ;
		VEDIT*   ps_vedit ;
		VERIFY*  ps_ver ;
		VPUTGET* ps_vputget ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************        TBSEARCH class         ********************************* */
/* ************************************ ***************************** ********************************* */


class tbsearch
{
	public:
		explicit tbsearch( const string& var,
				   const string& val,
				   int i ) : tbsearch()
		{
			tbs_var      = var ;
			tbs_val      = val ;
			tbs_position = i ;
			if ( tbs_val.size() > 0 && tbs_val.back() == '*' )
			{
				tbs_generic = true ;
				tbs_size    = tbs_val.size() - 1 ;
				tbs_val.pop_back() ;
			}
		}

		tbsearch()
		{
			tbs_position  = -1 ;
			tbs_generic   = false ;
			tbs_condition = S_EQ ;
			tbs_size      = 0 ;
			tbs_y         = 0 ;
		}

		bool set_condition( const string& cond )
		{
			string t1 ;
			if ( cond.size() == 4 && cond[ 2 ] == 'Y' && std::isdigit( cond[ 3 ] ) )
			{
				t1    = cond.substr( 0, 2 ) ;
				tbs_y = ds2d( cond.substr( 3, 1 ) ) ;
				if ( tbs_y < 1 || tbs_y > 7 ) { return false ; }
				if      ( t1 == "LE" ) { tbs_condition = S_LE ; }
				else if ( t1 == "LT" ) { tbs_condition = S_LT ; }
				else if ( t1 == "GE" ) { tbs_condition = S_GE ; }
				else if ( t1 == "GT" ) { tbs_condition = S_GT ; }
				else                   { return false ; }
				if ( tbs_generic )
				{
					tbs_size += 2 ;
				}
			}
			else if ( cond == ""   ) { tbs_condition = S_EQ ; }
			else if ( cond == "EQ" ) { tbs_condition = S_EQ ; }
			else if ( cond == "NE" ) { tbs_condition = S_NE ; }
			else if ( cond == "LE" ) { tbs_condition = S_LE ; }
			else if ( cond == "LT" ) { tbs_condition = S_LT ; }
			else if ( cond == "GE" ) { tbs_condition = S_GE ; }
			else if ( cond == "GT" ) { tbs_condition = S_GT ; }
			else                     { return false ; }
			return true ;
		}

		string get_condition()
		{
			switch ( tbs_condition )
			{
			case S_EQ:
			      return "EQ" ;

			case S_NE:
			      return "NE" ;

			case S_LE:
			      return "LE" ;

			case S_LT:
			      return "LT" ;

			case S_GE:
			      return "GE" ;

			case S_GT:
			      return "GT" ;

			default:
			      return "??" ;
			}
		}

		string  tbs_var ;
		string  tbs_val ;
		SR_COND tbs_condition ;
		bool    tbs_generic ;
		int     tbs_position ;
		int     tbs_size ;
		size_t  tbs_y ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************      Field Execute class      ********************************* */
/* ************************************ ***************************** ********************************* */


class fieldXct
{
	public:
		fieldXct()
		{
			fieldXct_field   = "" ;
			fieldXct_command = "" ;
			fieldXct_passed  = "" ;
			fieldXct_pnum    = 0  ;
		}

		fieldXct( errblock& err,
			  const string& s ) : fieldXct()
		{
			parse( err, s ) ;
		}

		string fieldXct_field ;
		string fieldXct_command ;
		string fieldXct_passed ;
		uint   fieldXct_pnum ;

	private:
		void parse( errblock&,
			    string ) ;

} ;


/* ************************************ ***************************** ********************************* */
/* ************************************     Scrollable Field class    ********************************* */
/* ************************************ ***************************** ********************************* */


class sfield
{
	public:
		sfield()
		{
			len = 0 ;

			rind2 = "+" ;
			lind2 = "-" ;
			sind2 = "<->" ;

			s2ind2 = " +" ;
			s3ind2 = "- " ;
			s4ind2 = "-+" ;

			scroll_on = true ;
			scroll_lr = true ;
			scrollvar = false ;
		}

		sfield( errblock& err,
			const string& s ) : sfield()
		{
			parse( err, s ) ;
		}

		string field ;

		string ind1  ;
		string rind1 ;
		string rind2 ;
		string lind1 ;
		string lind2 ;
		string sind1 ;
		string sind2 ;

		string lcol ;
		string rcol ;
		string scale ;
		string scroll ;

		string s2ind2 ;
		string s3ind2 ;
		string s4ind2 ;

		string lenvar ;

		size_t len ;

		bool scroll_on ;
		bool scroll_lr ;
		bool scrollvar ;

	private:
		void parse( errblock&,
			    string ) ;

} ;


/* ************************************ ***************************** ********************************* */
/* ************************************   Short/Long message class    ********************************* */
/* ************************************ ***************************** ********************************* */


class slmsg
{
	public:
		slmsg()
		{
			clear() ;
		}

		void clear()
		{
			smsg   = "" ;
			lmsg   = "" ;
			hlp    = "" ;
			dvwin  = "" ;
			dvtype = "" ;
			dvalm  = "" ;
			dvhlp  = "" ;
			msgloc = "" ;
			type   = IMT ;
			alm    = false ;
			resp   = false ;
			smwin  = false ;
			lmwin  = false ;
		}

		int parse( const string&,
			   const string& ) ;

		string  smsg ;
		string  lmsg ;
		string  hlp ;
		string  dvwin ;
		string  dvtype ;
		string  dvalm ;
		string  dvhlp ;
		string  msgloc ;
		attType type ;
		bool    alm ;
		bool    resp ;
		bool    smwin ;
		bool    lmwin ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************      Window Popup class       ********************************* */
/* ************************************ ***************************** ********************************* */


class popup
{
	public:
		popup()
		{
			row  = 0 ;
			col  = 0 ;
			panl = nullptr ;
			pan1 = nullptr ;
			pan2 = nullptr ;
		}

		popup& set( uint r,
			    uint c )
		{
			row  = r ;
			col  = c ;
			return *this ;
		}

		uint    row ;
		uint    col ;
		void*   panl ;
		PANEL*  pan1 ;
		PANEL*  pan2 ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************      SELECT object class      ********************************* */
/* ************************************ ***************************** ********************************* */


class selobj
{
	public:
		selobj()
		{
			clear() ;
		}

		void clear()
		{
			pgm     = "" ;
			parm    = "" ;
			newappl = "" ;
			rsn     = 0 ;
			ptid    = 0 ;
			stid    = 0 ;
			newpool = false ;
			passlib = false ;
			suspend = false ;
			scrname = "" ;
			selopt  = "" ;
			selPanl = false ;
			backgrd = false ;
			nested  = false ;
			pgmtype = PGM_NONE ;
			quiet   = false ;
			nollog  = false ;
			errors  = false ;
			sync    = true ;
			fstack  = false ;
			errret  = false ;
			service = false ;
			pfkey   = false ;
			nofunc  = false ;
			zexpand = false ;
			nocheck = false ;
			options = nullptr ;
		}

		void def( const string& p )
		{
			clear() ;
			pgm     = p ;
			newappl = "ISR" ;
			newpool = true ;
			suspend = true ;
			selPanl = true ;
			nested  = true ;
			nocheck = true ;
			fstack  = false ;
		}

		bool parse( errblock&,
			    string ) ;

		bool selPanel() { return selPanl ; }

		string pgm ;
		string parm ;
		string newappl ;
		int    rsn ;
		uint   ptid ;
		uint   stid ;
		bool   newpool ;
		bool   passlib ;
		bool   suspend ;
		string scrname ;
		string selopt  ;
		bool   selPanl ;
		bool   backgrd ;
		bool   nested  ;
		bool   fstack  ;
		bool   errret  ;
		bool   service ;
		PGM_TYPE pgmtype ;
		bool   quiet ;
		bool   nollog ;
		bool   pfkey ;
		bool   nofunc ;
		bool   zexpand ;
		bool   nocheck ;
		bool   errors ;
		bool   sync ;
		void*  options ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************     Edit parameter class      ********************************* */
/* ************************************ ***************************** ********************************* */


class edit_parms
{
	public:
		edit_parms()
		{
			edit_viewmode = false ;
			edit_recovery = false ;
			edit_reclen   = 0 ;
		}

		string edit_file ;
		string edit_panel ;
		string edit_macro ;
		string edit_profile ;
		string edit_lcmds ;
		string edit_bfile ;
		string edit_confirm ;
		string edit_preserve ;
		string edit_chgwarn ;
		string edit_parm ;
		string edit_dataid ;
		int    edit_reclen ;
		bool   edit_viewmode ;
		bool   edit_recovery ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************     Browse parameter class    ********************************* */
/* ************************************ ***************************** ********************************* */


class browse_parms
{
	public:
		browse_parms()
		{
			browse_reclen = 0 ;
		}

		string browse_file ;
		string browse_panel ;
		int    browse_reclen ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************      Char Attribute class     ********************************* */
/* ************************************ ***************************** ********************************* */


class char_attrs
{
	public:
		char_attrs()
		{
			type    = INPUT ;
			cuadyn  = NONE ;

			caps1   = CAPS_OFF ;
			caps2   = CAPS_OFF ;
			just1   = 'L' ;
			just2   = 'A' ;
			nojump  = false ;
			numeric = false ;
			padc    = false ;
			padchar = ' ' ;
			paduser = false ;
			skip    = true ;

			colour  = 0 ;
			intens  = 0 ;
			hilite  = 0 ;

			avail   = true ;
			cua     = false ;
			dvars   = false ;
			input   = true ;
			once    = false ;
			output  = false ;
			pas     = false ;
			passwd  = false ;
			text    = false ;

			ncolour = true ;
			nintens = true ;
			nhilite = true ;

			ovrd_attrs = nullptr ;
		}

		char_attrs( attType t ) : char_attrs()
		{
			set_cuatype( t ) ;
		}

		char_attrs( errblock& err,
			    const string& attrs ) : char_attrs()
		{
			setattr( err, attrs ) ;
		}

		~char_attrs()
		{
			delete ovrd_attrs ;
		}

		char_attrs( const char_attrs& ca )
		{
			type    = ca.type ;
			cuadyn  = ca.cuadyn ;

			caps1   = ca.caps1 ;
			caps2   = ca.caps2 ;
			just1   = ca.just1 ;
			just2   = ca.just2 ;
			nojump  = ca.nojump ;
			numeric = ca.numeric ;
			padc    = ca.padc ;
			padchar = ca.padchar ;
			paduser = ca.paduser ;
			skip    = ca.skip ;

			colour  = ca.colour ;
			intens  = ca.intens ;
			hilite  = ca.hilite ;

			avail   = ca.avail ;
			cua     = ca.cua ;
			dvars   = ca.dvars ;
			input   = ca.input ;
			once    = false ;
			output  = ca.output ;
			pas     = ca.pas ;
			passwd  = ca.passwd ;
			text    = ca.text ;

			ncolour = ca.ncolour ;
			nintens = ca.nintens ;
			nhilite = ca.nhilite ;

			ovrd_attrs = nullptr ;
		}

		char_attrs operator = ( const char_attrs& rhs )
		{
			type    = rhs.type ;
			cuadyn  = rhs.cuadyn ;

			caps1   = rhs.caps1 ;
			caps2   = rhs.caps2 ;
			just1   = rhs.just1 ;
			just2   = rhs.just2 ;
			nojump  = rhs.nojump ;
			numeric = rhs.numeric ;
			padc    = rhs.padc ;
			padchar = rhs.padchar ;
			paduser = rhs.paduser ;
			skip    = rhs.skip ;

			colour  = rhs.colour ;
			intens  = rhs.intens ;
			hilite  = rhs.hilite ;

			avail   = rhs.avail ;
			cua     = rhs.cua ;
			dvars   = rhs.dvars ;
			input   = rhs.input ;
			once    = false ;
			output  = rhs.output ;
			pas     = rhs.pas ;
			passwd  = rhs.passwd ;
			text    = rhs.text ;

			ncolour = rhs.ncolour ;
			nintens = rhs.nintens ;
			nhilite = rhs.nhilite ;

			delete ovrd_attrs ;
			ovrd_attrs = nullptr ;
			return *this ;
		}

		void clear()
		{
			type    = INPUT ;
			cuadyn  = NONE ;

			caps1   = CAPS_OFF ;
			caps2   = CAPS_OFF ;
			just1   = 'L' ;
			just2   = 'A' ;
			nojump  = false ;
			numeric = false ;
			padc    = false ;
			padchar = ' ' ;
			paduser = false ;
			skip    = true ;

			colour  = 0 ;
			intens  = 0 ;
			hilite  = 0 ;

			avail   = true ;
			cua     = false ;
			input   = true ;
			once    = false ;
			output  = false ;
			pas     = false ;
			passwd  = false ;
			text    = false ;

			ncolour = true ;
			nintens = true ;
			nhilite = true ;

			delete ovrd_attrs ;
			ovrd_attrs = nullptr ;
		}

		void update( errblock&,
			     const string& ) ;

		void update( errblock&,
			     const char_attrs* ) ;

		void override_attrs( errblock&,
				     const string&,
				     bool,
				     bool = false ) ;

		void remove_override() ;

		void remove_override_once() ;

		const string& get_entry1() ;

		char get_padchar() const ;
		char get_just( bool ) const ;

		bool is_cua_input() const ;
		bool is_input_attr() const ;
		bool is_output_attr() const ;
		bool is_text_attr() const ;

		bool is_caps( bool ) const ;
		bool is_caps_in( bool ) const ;
		bool is_caps_out( bool ) const ;
		bool is_input() const ;
		bool is_input_pas() const ;
		bool is_nojump() const ;
		bool is_numeric() const ;
		bool is_once() const ;
		bool is_padc() const ;
		bool is_paduser() const ;
		bool is_pas() const ;
		bool is_passwd() const ;
		bool is_intens_non() const ;
		bool is_skip() const ;
		bool is_text() const ;

		uint get_colour() const ;

		void set_cuatype( attType ) ;

		attType get_type() const ;

		bool has_dvars() const  { return dvars ; }

	private:

		attType type ;
		attType cuadyn ;

		char just1 ;
		char just2 ;
		char padchar ;

		string entry1 ;
		string entry2 ;

		unsigned int colour ;
		unsigned int intens ;
		unsigned int hilite ;

		ECAPS caps1 ;
		ECAPS caps2 ;

		bool nojump ;
		bool numeric ;
		bool padc ;
		bool paduser ;
		bool skip ;

		bool avail ;
		bool cua ;
		bool dvars ;
		bool input ;
		bool once ;
		bool output ;
		bool pas ;
		bool passwd ;
		bool text ;

		bool ncolour ;
		bool nintens ;
		bool nhilite ;

		char_attrs* ovrd_attrs ;

		void parse_attrs( errblock&,
				  string,
				  bool = false,
				  bool = false,
				  bool = false ) ;

		void parse_attrs_type( errblock&,
				       string&,
				       bool = false,
				       bool = false,
				       bool = false ) ;

		void parse_attrs_colour( errblock&,
					 string&,
					 bool = false ) ;

		void parse_attrs_intens( errblock&,
					 string&,
					 bool = false ) ;

		void parse_attrs_hilite( errblock&,
					 string&,
					 bool = false ) ;

		void parse_attrs_unavail( errblock&,
					  string&,
					  bool = false ) ;

		void parse_attrs_pas( errblock&,
				      string&,
				      bool = false ) ;

		void parse_attrs_cuadyn( errblock&,
					 string&,
					 bool = false ) ;

		void parse_attrs_caps( errblock&,
				       string&,
				       bool = false ) ;

		void parse_attrs_just( errblock&,
				       string&,
				       bool = false ) ;

		void parse_attrs_numeric( errblock&,
					  string&,
					  bool = false ) ;

		void parse_attrs_pad( errblock&,
				      string&,
				      bool = false ) ;

		void parse_attrs_skip( errblock&,
				       string&,
				       bool = false ) ;

		void parse_attrs_nojump( errblock&,
					 string&,
					 bool = false ) ;

		void parse_attrs_passwd( errblock&,
					 string&,
					 bool = false ) ;

		void parse_inline( errblock&,
				   string ) ;

		void setattr( errblock&,
			      const string& ) ;

		void set_defaults() ;

	friend class field ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************      Text Attribute class     ********************************* */
/* ************************************ ***************************** ********************************* */


class text_attrs
{
	public:
		text_attrs( attType t )
		{
			type = t ;
		}

		text_attrs()
		{
			type = VOI ;
		}

		uint    get_colour() ;
		attType get_type() ;

		attType type ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************     File Tailoring classes    ********************************* */
/* ************************************ ***************************** ********************************* */


class ftc_base
{
	public:
		ftc_base()
		{
		}

	protected:
		int ln ;
		const string* name ;

		string src_info()
		{
			return *name + " record-" + to_string( ln ) ;
		}

		string src_info_line( const string& s )
		{
			return src_info() + ": " + s ;
		}

} ;


class ftc_def : public ftc_base
{
	public:
		ftc_def()
		{
		}

		ftc_def( errblock& err,
			 const string& s1,
			 const string* s2,
			 int l ) : ftc_def()
		{
			ln   = l ;
			name = s2 ;
			parse( err, s1 ) ;
		}

       private:
		void parse( errblock&,
			    const string& ) ;

		string get() const
		{
			return defs ;
		}

		char char_ctrl() const
		{
			return defs[ 0 ] ;
		}

		char char_var() const
		{
			return defs[ 1 ] ;
		}

		string defs ;

	friend class pFTailor ;
} ;


class ftc_do : public ftc_base
{
	public:
		ftc_do()
		{
			l_loop  = 1 ;
			l_to    = 0 ;
			l_by    = 0 ;
			l_for   = 0 ;
			e_loop  = false ;
			e_to    = false ;
			e_by    = false ;
			e_for   = false ;
			e_while = false ;
			e_until = false ;
			l_type1 = true  ;
			l_type2 = false ;
			l_type3 = false ;
		}

		ftc_do( errblock& err,
			const string& s1,
			const string* s2,
			int l ) : ftc_do()
		{
			ln   = l ;
			name = s2 ;
			parse( err, s1 ) ;
		}

       private:
		void parse( errblock&,
			    const string& ) ;

		void parse_t( errblock&,
			      const string&,
			      token&,
			      string&,
			      int& ) ;

		bool l_type1 ;
		bool l_type2 ;
		bool l_type3 ;

		bool e_while ;
		bool e_until ;

		bool e_loop ;
		bool e_to ;
		bool e_by ;
		bool e_for ;

		int l_loop ;
		int l_to ;
		int l_by ;
		int l_for ;

		int l_rhs ;
		int l_lhs ;

		string v_loop ;
		string v_to ;
		string v_by ;
		string v_for ;

		string v_rhs ;
		string v_lhs ;

		RL_COND cond ;

		string var ;

	friend class pFTailor ;
} ;


class ftc_dot : public ftc_base
{
	public:
		ftc_dot()
		{
			sarg = "" ;
			scan = false ;
		}

		ftc_dot( errblock& err,
			 const string& s1,
			 const string* s2,
			 int l ) : ftc_dot()
		{
			ln   = l ;
			name = s2 ;
			parse( err, s1 ) ;
		}

       private:
		void parse( errblock&,
			    string ) ;

		string table ;
		string sarg  ;
		bool   scan  ;
		bool   opt   ;

	friend class pFTailor ;
} ;


class ftc_leave : public ftc_base
{
	public :
		ftc_leave()
		{
			dot = false ;
		}

		ftc_leave( errblock& err,
			   const string& s1,
			   const string* s2,
			   int l ) : ftc_leave()
		{
			ln   = l ;
			name = s2 ;
			parse( err, s1 ) ;
			if ( err.error() ) { return ; }
		}

		void parse( errblock&,
			    const string& ) ;

	private:

		bool dot ;

	friend class pFTailor ;
} ;


class ftc_rexx : public ftc_base, public PREXX
{
	public :
		ftc_rexx() : PREXX()
		{
		}

		ftc_rexx( errblock& err,
			  const string& s1,
			  const string* s2,
			  int l ) : ftc_rexx()
		{
			ln   = l ;
			name = s2 ;
			parse( err, s1 ) ;
			if ( err.error() ) { return ; }
		}

		void parse( errblock&,
			    const string& ) ;

	private:

	friend class pFTailor ;
} ;


class ftc_sel : public ftc_base
{
	public:
		ftc_sel()
		{
			lvar    = false ;
			rvar    = false ;
			sel_AND = true  ;
			ft_sel_next = nullptr ;
		}

		~ftc_sel()
		{
			delete ft_sel_next ;
		}

		ftc_sel( errblock& err,
			 const string& s1,
			 const string* s2,
			 const char char_var,
			 int l ) : ftc_sel()
		{
			ln   = l ;
			name = s2 ;
			parse( err, s1, char_var ) ;
		}

       private:

		void parse( errblock&,
			    string,
			    const char ) ;

		ftc_sel* ft_sel_next ;

		bool   lvar ;
		bool   rvar ;
		bool   sel_AND ;

		string lval ;
		string rval ;

		RL_COND cond ;

	friend class pFTailor ;
} ;


class ftc_set : public ftc_base
{
	public:
		ftc_set()
		{
		}

		ftc_set( errblock& err,
			 const string& s1,
			 const string* s2,
			 int l ) : ftc_set()
		{
			ln   = l ;
			name = s2 ;
			parse( err, s1 ) ;
		}

       private:
		void parse( errblock&,
			    const string& ) ;

		string var ;
		string expr ;

	friend class pFTailor ;
} ;


class ftc_tb : public ftc_base
{
	public:
		ftc_tb()
		{
		}

		ftc_tb( errblock& err,
			const string& s1,
			const string* s2,
			int l ) : ftc_tb()
		{
			ln     = l ;
			name   = s2 ;
			parse( err, s1 ) ;
		}

       protected:
		void parse( errblock&,
			    const string& ) ;

		size_t get_tabpos( size_t ) ;

		bool ft_tba ;

		vector<bool> ft_tbalt ;
		vector<size_t> ft_tabs ;

	friend class pFTailor ;
} ;


class ftc_main : public ftc_base
{
	public:
		ftc_main()
		{
			ft_next    = nullptr ;
			ft_prev    = nullptr ;
			ft_jump    = nullptr ;
			ft_def     = nullptr ;
			ft_sel     = nullptr ;
			ft_set     = nullptr ;
			ft_dot     = nullptr ;
			ft_do      = nullptr ;
			ft_line1   = nullptr ;
			ft_line2   = nullptr ;
			ft_tb      = nullptr ;
			ft_rexx    = nullptr ;
			ft_leave   = nullptr ;
			ft_enddot  = false ;
			ft_endsel  = false ;
			ft_enddo   = false ;
			ft_blank   = false ;
			ft_noft    = false ;
			ft_nop     = false ;
			ft_iterate = false ;
		}

		ftc_main( errblock& err,
			  const string& s1,
			  const string* s2,
			  const char char_var,
			  int l,
			  bool nf = false ) : ftc_main()
		{
			ln       = l  ;
			name     = s2 ;
			ft_noft  = nf ;
			ft_line2 = &s1 ;
			parse( err, s1, char_var ) ;
		}

		ftc_main( const string* ps,
			  const string* s2,
			  int l,
			  bool nf = false ) : ftc_main()
		{
			ft_line1 = ps ;
			ln       = l  ;
			name     = s2 ;
			ft_noft  = nf ;
		}

		~ftc_main()
		{
			delete ft_def ;
			delete ft_sel ;
			delete ft_set ;
			delete ft_dot ;
			delete ft_do  ;
			delete ft_tb  ;
			delete ft_rexx ;
			delete ft_leave ;
		}

       private:
		ftc_main* ft_next ;
		ftc_main* ft_prev ;
		ftc_main* ft_jump ;

		ftc_def*   ft_def ;
		ftc_set*   ft_set ;
		ftc_sel*   ft_sel ;
		ftc_dot*   ft_dot ;
		ftc_do*    ft_do  ;
		ftc_tb*    ft_tb  ;
		ftc_rexx*  ft_rexx ;
		ftc_leave* ft_leave ;

		const string* ft_line1 ;
		const string* ft_line2 ;

		bool ft_blank ;
		bool ft_enddo ;
		bool ft_enddot ;
		bool ft_endsel ;
		bool ft_noft ;
		bool ft_nop ;
		bool ft_iterate ;

		int  ft_num ;

		void parse( errblock&,
			    const string&,
			    const char ) ;

	map<string, FT_TYPE> statement_types =
		   { { "BLANK",   FT_BLANK   },
		     { "DEFAULT", FT_DEFAULT },
		     { "DO",      FT_DO      },
		     { "ENDDO",   FT_ENDDO   },
		     { "DOT",     FT_DOT     },
		     { "ENDDOT",  FT_ENDDOT  },
		     { "ELSE",    FT_ELSE    },
		     { "IF",      FT_IF      },
		     { "ITERATE", FT_ITERATE },
		     { "LEAVE",   FT_LEAVE   },
		     { "NOP",     FT_NOP     },
		     { "REXX",    FT_REXX    },
		     { "SEL",     FT_SEL     },
		     { "ENDSEL",  FT_ENDSEL  },
		     { "SET",     FT_SET     },
		     { "SETF",    FT_SETF    },
		     { "TB",      FT_TB      },
		     { "TBA",     FT_TBA     } } ;

	friend class pFTailor ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************         Logger class          ********************************* */
/* ************************************ ***************************** ********************************* */


class logger
{
	public:
		logger() ;
		~logger() ;

		ofstream& operator << ( const string& s )
		{
			of << s ;
			return of ;
		}

		ofstream& operator << ( const ptime& t )
		{
			of << t ;
			return of ;
		}

		void lock()    { mtx.lock() ; }
		void unlock()  { mtx.unlock() ; }

		bool open( const string& = "",
			   bool = false ) ;

		bool set( const string& ) ;

		const string& logname() { return *currfl ; }

	private:
		bool logOpen ;

		string* currfl ;
		string  tmpfl ;
		string  logfl ;

		void close() ;

		ofstream of ;
		boost::recursive_mutex mtx ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************       tempfile class          ********************************* */
/* ************************************ ***************************** ********************************* */


class tempfile
{
	public:
		tempfile( const string& s )
		{
			boost::filesystem::path temp =
			boost::filesystem::temp_directory_path() / boost::filesystem::unique_path( s + "-%%%%-%%%%" ) ;
			tfile = temp.native() ;
		}

		~tempfile()
		{
			boost::filesystem::remove( tfile ) ;
		}

		void open()
		{
			of.open( tfile ) ;
		}

		void close()
		{
			of.close() ;
		}

		ofstream& operator << ( const string& s )
		{
			of << s ;
			return of ;
		}

		const string& name()
		{
			return tfile ;
		}

		const char* name_cstr()
		{
			return tfile.c_str() ;
		}

	private:
		ofstream of ;

		string tfile ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************         locator class         ********************************* */
/* ************************************ ***************************** ********************************* */


class locator
{
	public:
		locator( const string& dd,
			 const string& fn,
			 const string& suf ) : locator()
		{
			paths    = dd ;
			filename = fn + suf ;
		}

		locator( const string& dd,
			 const string& fn ) : locator()
		{
			paths    = dd ;
			filename = fn ;
		}

		locator()
		{
			type = 'B' ;
		}

		void locate() ;

		bool found()
		{
			return ( ent != "" ) ;
		}

		bool not_found()
		{
			return ( ent == "" ) ;
		}

		bool errors()
		{
			return ( mid != "" ) ;
		}

		const string& msgid()
		{
			return mid ;
		}

		const string& mdata()
		{
			return data ;
		}

		const string& name()
		{
			return filename ;
		}

		const string& entry()
		{
			return ent ;
		}

		locator& asis()
		{
			type = 'A' ;
			return *this ;
		}

		locator& both()
		{
			type = 'B' ;
			return *this ;
		}

	private:
		char type ;

		string mid ;
		string ent ;
		string data ;
		string paths ;
		string filename ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************     Dynamic Loader class      ********************************* */
/* ************************************ ***************************** ********************************* */


class dynloader
{
	public:
		dynloader( const string&,
			   int = RTLD_NOW ) ;

		~dynloader() ;

		void  open() ;
		void  close() ;
		void* lookup( const char* ) ;
		bool  errors() { return ( errstring != "" ) ; }
		void  clear()  { errstring = "" ; }
		const string& errmsg() { return errstring ; }

	private:
		string errstring ;
		string filename ;
		int flags ;
		void* handle ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************      PANEXIT LOAD class       ********************************* */
/* ************************************ ***************************** ********************************* */


struct exitInfo2
{
	//
	// PANEXIT LOAD exit structure.
	//

	string msgid ;
	string panelid ;
	string psection ;
	string exdata ;
	size_t numvars ;
	vector<string> vars ;
	vector<string>* vals ;
} ;


class panload
{
	public:
		panload() {} ;
		virtual ~panload() {} ;
		virtual int go( exitInfo2& ) = 0 ;

	private:
} ;

/* ************************************ ***************************** ********************************* */
/* ************************************           ENQ class           ********************************* */
/* ************************************ ***************************** ********************************* */


class enqueue
{
	public:
		enqueue()
		{
			maj_name = "" ;
			min_name = "" ;
			disp     = EXC ;
		}

		enqueue( const string& maj,
			 const string& min,
			 int t,
			 enqDISP d = EXC )
		{
			maj_name = maj ;
			min_name = min ;
			disp     = d ;
			tasks.insert( t ) ;
		}

		set<int> tasks ;
		enqDISP  disp ;

		string maj_name ;
		string min_name ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************          ALTLIB class         ********************************* */
/* ************************************ ***************************** ********************************* */


class tso_altlib
{
	public:
		tso_altlib( string& err,
			    const string& s ) : tso_altlib()
		{
			parse( err, s ) ;
		}

		tso_altlib()
		{
			ddn     = "" ;
			path    = "" ;
			user    = false ;
			syst    = false ;
			appl    = false ;
			all     = false ;
			reset   = false ;
			quiet   = false ;
			act     = false ;
			disp    = false ;
			cond    = false ;
		}

		~tso_altlib() ;

		void parse( string&,
			    string ) ;


		bool user ;
		bool syst ;
		bool appl ;
		bool all ;
		bool reset ;
		bool quiet ;
		bool act ;
		bool disp ;
		bool cond ;

		string ddn ;
		string path ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************          ALLOC class          ********************************* */
/* ************************************ ***************************** ********************************* */


class tso_alloc
{
	public:
		tso_alloc( string& err,
			   const string& s ) : tso_alloc()
		{
			parse( err, s ) ;
		}

		tso_alloc()
		{
			ddn     = "" ;
			path    = "" ;
			open    = 0 ;
			dir     = false ;
			shr     = false ;
			old     = false ;
			del     = false ;
			reuse   = false ;
			dummy   = false ;
			create  = false ;
			dynalc  = true ;
		}

		~tso_alloc() ;

		void parse( string&,
			    string ) ;

		void set_open()       { ++open ; }
		void set_closed()     { if ( open > 0 ) { --open ; } }
		void set_nodynalloc() { dynalc = false ; }

		bool is_open()        { return ( open > 0 ) ; }

		bool dynalloc()       { return dynalc ; }

		bool find( const string& f ) { return std::find( paths.begin(), paths.end(), f ) != paths.end() ; }

		const string& getpath( size_t i ) const { return paths[ i ] ; }

		bool directory()  { return  dir ; }
		bool file()       { return !dir ; }

		void setpath( const string& p ) { path = p ; paths.push_back( p ) ; }

		vector<string> paths ;

		string ddn ;
		string path ;

		int open ;

		size_t entries() const { return paths.size() ; }

		bool shr ;
		bool old ;
		bool del ;
		bool dir ;
		bool reuse ;
		bool dummy ;
		bool create ;
		bool dynalc ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************          FREE class           ********************************* */
/* ************************************ ***************************** ********************************* */


class tso_free
{
	public:
		tso_free( string& err,
			 const string& s ) : tso_free()
		{
			parse( err, s ) ;
		}

		tso_free()
		{
			all = false ;
		}

		void parse( string&,
			    string ) ;

		bool all ;

		vector<string> ddns ;
		vector<string> paths ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************         EXECIO class          ********************************* */
/* ************************************ ***************************** ********************************* */


class execio
{
	public:
		execio( string& err,
			const string& s ) : execio()
		{
			parse( err, s ) ;
		}

		execio()
		{
			all   = false ;
			read  = true ;
			fifo  = true ;
			lifo  = false ;
			skip  = false ;
			open  = true  ;
			num   = 0 ;
		}

		void parse( string&,
			    string ) ;

		string stem ;
		string ddn  ;
		bool   all  ;
		bool   read ;
		bool   finis ;
		bool   fifo ;
		bool   lifo ;
		bool   skip ;
		bool   open ;
		int    num ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************         LD entry class        ********************************* */
/* ************************************ ***************************** ********************************* */


class ldentry
{
       public:
		ldentry( const string& m_entry ) : ldentry()
		{
			entry = m_entry ;
		}

		ldentry()
		{
		}

		~ldentry()
		{}

		const string& get_entry()       { return entry ; }

       private:
		string entry ;
} ;


/* ************************************ ***************************** ********************************* */
/* ************************************         LM entry class        ********************************* */
/* ************************************ ***************************** ********************************* */


class lmentry
{
       public:
		lmentry( const string& m_entry,
			 const string& m_ddname,
			 LM_DISP m_disp,
			 ENT_TYPE m_type ) : lmentry()
		{
			entry    = m_entry ;
			ddname   = m_ddname ;
			disp     = m_disp ;
			type     = m_type ;
		}

		lmentry()
		{
			lmopen = false ;
			input  = false ;
			recs   = 0 ;
		}

		~lmentry()
		{}

		const string& get_entry()       { return entry     ; }
		const string& get_ddname()      { return ddname    ; }
		const string& get_lmmfind()     { return mfind     ; }
		int           get_records()     { return recs      ; }
		void          inc_records()     { ++recs           ; }
		void          clr_records()     { recs = 0         ; }
		bool          lmopened()        { return lmopen    ; }
		bool          for_input()       { return input     ; }
		void          set_input()       { input  = true    ; }
		void          set_lmopened()    { lmopen = true    ; recs = 0  ; }
		void          set_lmclosed()    { lmopen = false   ; }
		LM_DISP       get_disp()        { return disp      ; }
		bool          lminit_shr()      { return disp == LM_SHR ; }
		string        get_disp_str()    { return ( disp == LM_SHR ) ? "SHR" : "EXCLU" ; }
		ENT_TYPE      get_type()        { return type      ; }
		bool          using_ddname()    { return ( entry == "" )      ; }
		bool          using_dsname()    { return ( entry != "" )      ; }
		bool          done_lmmfind()    { return ( mfind != "" )      ; }
		bool          is_directory()    { return ( type == ENT_DIR  ) ; }
		bool          is_regular_file() { return ( type == ENT_FILE ) ; }
		string        get_type_str()    { return ( type == ENT_DIR    ) ? "DIR"     :
							 ( type == ENT_FILE   ) ? "FILE"    :
							 ( type == ENT_CHAR   ) ? "CHAR"    :
							 ( type == ENT_BLOCK  ) ? "BLOCK"   :
							 ( type == ENT_FIFO   ) ? "FIFO"    :
							 ( type == ENT_SOCKET ) ? "SOCKET"  :
							 ( type == ENT_SYML   ) ? "SYMLINK" : "" ; }
		const string& get_tfile()       {
							if ( tfile == "" )
							{
							       boost::filesystem::path temp = boost::filesystem::unique_path( ".lmput-%%%%-%%%%" ) ;
							       tfile = temp.native() ;
							}
							return tfile ;
						}

		void set_lmmfind( const string& s ) { mfind = s ; }

       private:
		string   entry  ;
		string   ddname ;
		string   tfile  ;
		string   mfind  ;
		bool     lmopen ;
		bool     input  ;
		int      recs   ;
		ENT_TYPE type   ;
		LM_DISP  disp   ;

} ;


