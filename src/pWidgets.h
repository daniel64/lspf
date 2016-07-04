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

ofstream aplog(ALOG) ;


class dynArea
{
	public:
		int    dynArea_row       ;
		int    dynArea_col       ;
		int    dynArea_width     ;
		int    dynArea_depth     ;
		bool   dynArea_DataInsp  ;
		bool   dynArea_DataOutsp ;
		bool   dynArea_UserModsp ;
		bool   dynArea_DataModsp ;
		char   dynArea_DataIn    ;
		char   dynArea_DataOut   ;
		char   dynArea_UserMod   ;
		char   dynArea_DataMod   ;
		string dynArea_Field     ;
		string dynArea_FieldIn   ;
		string dynArea_shadow_name ;

		dynArea(){
				dynArea_DataInsp  = false ;
				dynArea_DataOutsp = false ;
				dynArea_UserModsp = false ;
				dynArea_DataModsp = false ;
				dynArea_Field     = ""    ;
				dynArea_FieldIn   = ""    ;
			 }

		int  dynArea_init( int MAXW, int MAXD, string line ) ;
		void setsize( int, int, int, int ) ;
} ;


class field
{
	public:
		unsigned int field_row          ;
		unsigned int field_col          ;
		unsigned int field_length       ;
		string       field_value        ;
		bool         field_pwd          ;
		bool         field_changed      ;
		bool         field_active       ;
		cuaType      field_cua          ;
		bool         field_usecua       ;
		uint         field_colour       ;
		bool         field_prot         ;
		bool         field_skip         ;
		bool         field_caps         ;
		char         field_padchar      ;
		char         field_just         ;
		bool         field_numeric      ;
		bool         field_input        ;
		bool         field_dynArea      ;
		dynArea *    field_dynArea_ptr  ;
		bool         field_tb           ;
		bool         field_scrollable   ;
		unsigned int field_scroll_start ;
		string       field_shadow_value ;

		field() {
				field_pwd          = false ;
				field_changed      = false ;
				field_active       = true  ;
				field_usecua       = true  ;
				field_colour       = 0     ;
				field_prot         = true  ;
				field_skip         = true  ;
				field_caps         = false ;
				field_padchar      = ' '   ;
				field_just         = 'L'   ;
				field_numeric      = false ;
				field_input        = false ;
				field_dynArea      = false ;
				field_tb           = false ;
				field_scrollable   = false ;
				field_scroll_start = 1     ;
				field_shadow_value = ""    ;
			} ;

		int  field_init( int MAXW, int MAXD, string line ) ;
		void display_field( WINDOW *, bool ) ;
		bool edit_field_insert( WINDOW * win, char ch, int row, bool, bool ) ;
		void edit_field_delete( WINDOW * win, int row, bool ) ;
		int  edit_field_backspace( WINDOW * win, int col, bool ) ;
		void field_remove_nulls()        ;
		void field_blank( WINDOW * win ) ;
		void field_clear( WINDOW * win ) ;
		void field_erase_eof( WINDOW * win, unsigned int col, bool ) ;
		bool field_dyna_input( uint col )  ;
		int  field_dyna_input_offset( uint col )  ;
		void field_DataMod_to_UserMod( string *, int ) ;
		int  field_attr( string attrs ) ;
		int  end_of_field( WINDOW * win, uint col )   ;
} ;


class literal
{
	public:
		literal() {
				literal_colour = 0  ;
				literal_name   = "" ;
			  }
		int     literal_row    ;
		int     literal_col    ;
		int     literal_length ;
		cuaType literal_cua    ;
		uint    literal_colour ;
		string  literal_value  ;
		string  literal_name   ;

		int  literal_init( int MAXW, int MAXD, int & opt_field, string line ) ;
		void literal_display( WINDOW * ) ;
} ;


class pdc
{
	public:
		string pdc_name ;
		string pdc_run  ;
		string pdc_parm ;
		string pdc_unavail ;

		pdc()   {
				pdc_name    = "" ;
				pdc_run     = "" ;
				pdc_parm    = "" ;
				pdc_unavail = "" ;
			} ;
		~pdc() {} ;
} ;


class abc
{
	public:
		string       abc_name ;
		unsigned int abc_col  ;
		unsigned int abc_maxh ;
		unsigned int abc_maxw ;

		abc()   {
				abc_maxh   = 0 ;
				abc_maxw   = 0 ;
				pd_created = false ;
			} ;

		~abc() {
			if ( pd_created )
			{
				del_panel( panel ) ;
				update_panels()    ;
			}
			} ;

		void add_pdc( string, string, string, string ) ;
		void display_abc_sel( WINDOW * )   ;
		void display_abc_unsel( WINDOW * ) ;
		void display_pd() ;
		void hide_pd()    ;
		pdc  retrieve_pdChoice( unsigned int row, unsigned int col ) ;

	private:
		bool   pd_created ;
		vector< pdc > pdcList ;
		WINDOW * win   ;
		PANEL  * panel ;
} ;


class Box
{
	public:
		int  box_init( int MAXW, int MAXD, string line ) ;
		void display_box( WINDOW * )      ;

	private:
		int    box_row    ;
		int    box_col    ;
		int    box_width  ;
		int    box_depth  ;
		uint   box_colour ;
		string box_title  ;
		int    box_title_offset ;
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


class pnts
{
	public:
		pnts( string ) ;
		pnts() {}      ;

	int    pnts_RC    ;
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
		IFSTMNT( string ) ;
		IFSTMNT()
		{
			if_RC    = 0     ;
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
	int    if_RC    ;
	string if_lhs   ;
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
		ASSGN( string ) ;
		ASSGN()
		{
			as_RC      = 0     ;
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
	int    as_RC      ;
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
		VERIFY( string ) ;
		VERIFY(){
				ver_RC      = 0     ;
				ver_nblank  = false ;
				ver_numeric = false ;
				ver_list    = false ;
				ver_pict    = false ;
				ver_hex     = false ;
				ver_octal   = false ;
				ver_tbfield = false ;
			} ;

		string ver_field   ;
		int    ver_RC      ;
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
		VPUTGET( string ) ;
		VPUTGET()
			{
				vpg_RC   = 0 ;
				vpg_vput = false ;
				vpg_vget = false ;
				vpg_pool = ASIS  ;
			} ;
		int      vpg_RC   ;
		bool     vpg_vput ;
		bool     vpg_vget ;
		string   vpg_vars ;
		poolType vpg_pool ;
} ;


class TRUNC
{
	public:
		TRUNC( string ) ;
		TRUNC() {
				trnc_RC   = 0   ;
				trnc_char = ' ' ;
				trnc_len  = 0   ;
			} ;

	int    trnc_RC     ;
	string trnc_field1 ;
	string trnc_field2 ;
	char   trnc_char   ;
	int    trnc_len    ;

} ;


class TRANS
{
	public:
		TRANS( string ) ;
		TRANS() {
				trns_RC   = 0   ;
			} ;

	map<string, string> tlst ;

	int    trns_RC     ;
	string trns_field1 ;
	string trns_field2 ;
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
