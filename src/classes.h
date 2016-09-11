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



class pnts
{
	public:
		pnts() {} ;
		bool parse( string ) ;

		string pnts_field ;
		string pnts_var   ;
		string pnts_val   ;
} ;


class panstmnt
{
	public :
		panstmnt()
		{
			ps_column  = 0     ;
			ps_if      = false ;
			ps_else    = false ;
			ps_assign  = false ;
			ps_verify  = false ;
			ps_vputget = false ;
			ps_trunc   = false ;
			ps_trans   = false ;
			ps_exit    = false ;
		}

		int  ps_column  ;
		bool ps_if      ;
		bool ps_else    ;
		bool ps_assign  ;
		bool ps_verify  ;
		bool ps_vputget ;
		bool ps_trunc   ;
		bool ps_trans   ;
		bool ps_exit    ;
} ;


class IFSTMNT
{
	public :
		IFSTMNT()
		{
			if_lhs   = ""    ;
			if_rhs.clear()   ;
			if_stmnt = 0     ;
			if_isvar.clear() ;
			if_true  = false ;
			if_else  = false ;
			if_istb  = false ;
			if_eq    = false ;
			if_ne    = false ;
			if_gt    = false ;
			if_lt    = false ;
			if_ge    = false ;
			if_le    = false ;
			if_ng    = false ;
			if_nl    = false ;
		}
		bool parse( string ) ;

		string if_lhs           ;
		vector<string> if_rhs   ;
		vector<bool>   if_isvar ;
		int    if_stmnt ;
		bool   if_true  ;
		bool   if_else  ;
		bool   if_istb  ;
		bool   if_eq    ;
		bool   if_ne    ;
		bool   if_gt    ;
		bool   if_lt    ;
		bool   if_ge    ;
		bool   if_le    ;
		bool   if_ng    ;
		bool   if_nl    ;
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
			as_upper   = false ;
			as_words   = false ;
			as_chkexst = false ;
			as_chkfile = false ;
			as_chkdir  = false ;
		}
		bool parse( string ) ;

		string as_lhs     ;
		string as_rhs     ;
		bool   as_isvar   ;
		bool   as_isattr  ;
		bool   as_istb    ;
		bool   as_retlen  ;
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
				ver_nblank  = false ;
				ver_numeric = false ;
				ver_list    = false ;
				ver_pict    = false ;
				ver_hex     = false ;
				ver_octal   = false ;
				ver_tbfield = false ;
			} ;

		bool parse( string ) ;

		string ver_field   ;
		bool   ver_nblank  ;
		bool   ver_numeric ;
		bool   ver_list    ;
		bool   ver_pict    ;
		bool   ver_hex     ;
		bool   ver_octal   ;
		bool   ver_tbfield ;
		string ver_value   ;
		string ver_msgid   ;
} ;


class VPUTGET
{
	public:
		VPUTGET()
			{
				vpg_vput = false ;
				vpg_vget = false ;
				vpg_pool = ASIS  ;
			} ;

		bool parse( string ) ;
		bool     vpg_vput ;
		bool     vpg_vget ;
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

		bool parse( string ) ;

		string trnc_field1 ;
		string trnc_field2 ;
		char   trnc_char   ;
		int    trnc_len    ;

} ;


class TRANS
{
	public:
		TRANS() {} ;
		bool parse( string ) ;

		map<string, string> tlst ;
		string trns_field1 ;
		string trns_field2 ;
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
				else return false ;
				return true       ;
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
			SCRNAME = "" ;
		}
		void clear()
		{
			PGM     = "" ;
			PARM    = "" ;
			NEWAPPL = "" ;
			NEWPOOL = false ;
			PASSLIB = false ;
			SCRNAME = "" ;
		}
		bool parse( string ) ;

		string PGM     ;
		string PARM    ;
		string NEWAPPL ;
		bool   NEWPOOL ;
		bool   PASSLIB ;
		string SCRNAME ;
} ;
