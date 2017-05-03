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


enum TOKEN_TYPES
{
	TT_STRING_QUOTED,
	TT_STRING_UNQUOTED,
	TT_AMPR_VAR_VALID,
	TT_AMPR_VAR_INVALID,
	TT_VAR_VALID,
	TT_CTL_VAR_VALID,
	TT_CTL_VAR_INVALID,
	TT_COMPARISON_OP,
	TT_OPEN_BRACKET,
	TT_CLOSE_BRACKET,
	TT_COMMA,
	TT_EQUALS,
	TT_EOF
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
	ST_TRUNC,
	ST_TRANS,
	ST_REFRESH,
	ST_VERIFY,
	ST_EOF
} ;

class token
{
	public:
		token()
		{
			value = ""     ;
			idx   = -1     ;
			type  = TT_EOF ;
		}
		string      value ;
		int         idx   ;
		TOKEN_TYPES type  ;
} ;


class parser
{
	public:
		parser()
		{
			idx      = 0     ;
			optUpper = false ;
		}
		void parseStatement( errblock&, string s ) ;
		token  getFirstToken() ;
		token  getNextToken()  ;
		token  getToken( int ) ;
		int    getEntries()    ;
		void   eraseTokens( int ) ;
		void   optionUpper()   { optUpper = true ; }
		bool   getNextIfCurrent( TOKEN_TYPES ) ;
		token  getCurrentToken() ;
		string getCurrentValue() ;
		void   getNameList( errblock&, string& ) ;
		bool   isCurrentType( TOKEN_TYPES ) ;
		void   getNextString( errblock& err, string::const_iterator&, const string& s, string& r, bool& ) ;
		STATEMENT_TYPE getStatementType() ;

	private:
		int   idx ;
		bool  optUpper ;
		token current_token ;
		vector<token>tokens ;
} ;

class pnts
{
	public:
		pnts() {} ;
		void parse( errblock&, string ) ;

		string pnts_field ;
		string pnts_var   ;
		string pnts_val   ;
} ;


class ASSGN
{
	public :
		ASSGN()
		{
			as_lhs     = ""    ;
			as_rhs     = ""    ;
			as_isvar   = false ;
			as_isattr  = false ;
			as_istb    = false ;
			as_retlen  = false ;
			as_reverse = false ;
			as_upper   = false ;
			as_words   = false ;
			as_chkexst = false ;
			as_chkfile = false ;
			as_chkdir  = false ;
		}
		void parse( errblock&, parser& ) ;

		string as_lhs     ;
		string as_rhs     ;
		bool   as_isvar   ;
		bool   as_isattr  ;
		bool   as_istb    ;
		bool   as_retlen  ;
		bool   as_reverse ;
		bool   as_upper   ;
		bool   as_words   ;
		bool   as_chkexst ;
		bool   as_chkfile ;
		bool   as_chkdir  ;
} ;



class VERIFY
{
	public:
		VERIFY(){
				ver_var     = ""    ;
				ver_msgid   = ""    ;
				ver_nblank  = false ;
				ver_numeric = false ;
				ver_list    = false ;
				ver_pict    = false ;
				ver_hex     = false ;
				ver_octal   = false ;
				ver_tbfield = false ;
				ver_pnfield = false ;
			} ;

		void parse( errblock&, parser&, bool =false ) ;

		string ver_var     ;
		string ver_msgid   ;
		bool   ver_nblank  ;
		bool   ver_numeric ;
		bool   ver_list    ;
		bool   ver_pict    ;
		bool   ver_hex     ;
		bool   ver_octal   ;
		bool   ver_tbfield ;
		bool   ver_pnfield ;
		vector<string> ver_vlist ;
} ;


class VPUTGET
{
	public:
		VPUTGET()
			{
				vpg_pool = ASIS ;
			} ;

		void parse( errblock&, parser& ) ;
		bool     vpg_vput ;
		string   vpg_vars ;
		poolType vpg_pool ;
} ;


class TRUNC
{
	public:
		TRUNC() {
				trnc_char = ' ' ;
				trnc_len  = 0   ;
			} ;

		void parse( errblock&, parser& ) ;

		string trnc_field1 ;
		string trnc_field2 ;
		char   trnc_char   ;
		int    trnc_len    ;

} ;


class TRANS
{
	public:
		TRANS() {
				trns_msgid    = ""    ;
				trns_default  = ""    ;
				trns_tbfield2 = false ;
				trns_pnfield2 = false ;
			} ;
		void parse( errblock&, parser& ) ;

		string trns_field1   ;
		string trns_field2   ;
		string trns_msgid    ;
		string trns_default  ;
		bool   trns_tbfield2 ;
		bool   trns_pnfield2 ;
		vector<pair<string,string>> trns_list ;
} ;


class IFSTMNT
{
	public :
		IFSTMNT()
		{
			if_lhs    = ""    ;
			if_rhs.clear()    ;
			if_true   = false ;
			if_else   = false ;
			if_eq     = false ;
			if_ne     = false ;
			if_gt     = false ;
			if_lt     = false ;
			if_ge     = false ;
			if_le     = false ;
			if_ng     = false ;
			if_nl     = false ;
			if_verify = NULL  ;
		}
		~IFSTMNT()
		{
			delete if_verify ;
		}
		void parse( errblock&, parser& ) ;

		string if_lhs      ;
		vector<string> if_rhs ;
		VERIFY*  if_verify ;
		bool     if_true   ;
		bool     if_else   ;
		bool     if_eq     ;
		bool     if_ne     ;
		bool     if_gt     ;
		bool     if_lt     ;
		bool     if_ge     ;
		bool     if_le     ;
		bool     if_ng     ;
		bool     if_nl     ;
} ;


class panstmnt
{
	public :
		panstmnt()
		{
			ps_label   = ""    ;
			ps_rlist   = ""    ;
			ps_column  = 0     ;
			ps_exit    = false ;
			ps_goto    = false ;
			ps_refresh = false ;
			ps_assgn   = NULL  ;
			ps_ver     = NULL  ;
			ps_if      = NULL  ;
			ps_else    = NULL  ;
			ps_vputget = NULL  ;
			ps_trunc   = NULL  ;
			ps_trans   = NULL  ;
		}
		~panstmnt()
		{
			delete ps_assgn ;
			delete ps_ver   ;
			delete ps_if    ;
			delete ps_vputget ;
			delete ps_trunc ;
			delete ps_trans ;
		}
		string ps_label   ;
		string ps_rlist   ;
		int    ps_column  ;
		bool   ps_exit    ;
		bool   ps_goto    ;
		bool   ps_refresh ;
		ASSGN*   ps_assgn ;
		VERIFY*  ps_ver   ;
		IFSTMNT* ps_if    ;
		IFSTMNT* ps_else  ;
		VPUTGET* ps_vputget ;
		TRUNC*   ps_trunc ;
		TRANS*   ps_trans ;
} ;


class tbsearch
{
	public:
		tbsearch()
			{
				tbs_ext  = false ;
				tbs_gen  = false ;
				tbs_cond = s_EQ  ;
			} ;

		bool tbsSetcon( string cond )
			{
				if ( cond == "" ) { cond = "GE" ; }
				tbs_scond = cond ;
				if      ( cond == "EQ" ) { tbs_cond = s_EQ ; }
				else if ( cond == "NE" ) { tbs_cond = s_NE ; }
				else if ( cond == "LE" ) { tbs_cond = s_LE ; }
				else if ( cond == "LT" ) { tbs_cond = s_LT ; }
				else if ( cond == "GE" ) { tbs_cond = s_GE ; }
				else if ( cond == "GT" ) { tbs_cond = s_GT ; }
				else                     { return false    ; }
				return true ;
			} ;

		string tbs_val   ;
		string tbs_scond ;
		srCOND tbs_cond  ;
		bool   tbs_ext   ;
		bool   tbs_gen   ;
		int    tbs_vsize ;
} ;


class fieldExc
{
	public:
		fieldExc()
		{
			fieldExc_command = "" ;
			fieldExc_passed  = "" ;
		} ;

		string fieldExc_command ;
		string fieldExc_passed  ;

} ;


class panel_data
{
	public:
		panel_data( int x )
		{
			screenID = x ;
		}

		int screenID ;
} ;


class slmsg
{
	public:
		slmsg()
		{
			smsg   = ""    ;
			lmsg   = ""    ;
			hlp    = ""    ;
			dvwin  = ""    ;
			dvtype = ""    ;
			dvalm  = ""    ;
			dvhlp  = ""    ;
			type   = IMT   ;
			alm    = false ;
			resp   = false ;
			smwin  = false ;
			lmwin  = false ;
			cont   = false ;
		}
		void clear()
		{
			smsg   = ""    ;
			lmsg   = ""    ;
			hlp    = ""    ;
			dvwin  = ""    ;
			dvtype = ""    ;
			dvalm  = ""    ;
			dvhlp  = ""    ;
			type   = IMT   ;
			alm    = false ;
			resp   = false ;
			smwin  = false ;
			lmwin  = false ;
			cont   = false ;
		}

		string  smsg   ;
		string  lmsg   ;
		string  hlp    ;
		string  dvwin  ;
		string  dvtype ;
		string  dvalm  ;
		string  dvhlp  ;
		cuaType type   ;
		bool    alm    ;
		bool    resp   ;
		bool    smwin  ;
		bool    lmwin  ;
		bool    cont   ;
} ;


class selobj
{
	public:
		selobj()
		{
			PGM     = "" ;
			PARM    = "" ;
			NEWAPPL = "" ;
			NEWPOOL = false ;
			PASSLIB = false ;
			SUSPEND = false ;
			SCRNAME = "" ;
			selPanl = false ;
		}
		void clear()
		{
			PGM     = "" ;
			PARM    = "" ;
			NEWAPPL = "" ;
			NEWPOOL = false ;
			PASSLIB = false ;
			SUSPEND = false ;
			SCRNAME = "" ;
			selPanl = false ;
		}
		bool parse( errblock&, string ) ;
		bool selPanel() { return selPanl ; }

		string PGM     ;
		string PARM    ;
		string NEWAPPL ;
		bool   NEWPOOL ;
		bool   PASSLIB ;
		bool   SUSPEND ;
		string SCRNAME ;

	private:
		bool   selPanl ;
} ;
