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
		dynArea(){
				dynArea_DataInsp  = false ;
				dynArea_DataOutsp = false ;
				dynArea_UserModsp = false ;
				dynArea_DataModsp = false ;
				dynArea_Field     = ""    ;
				dynArea_FieldIn   = ""    ;
			 }

	private:
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

		int  dynArea_init( int MAXW, int MAXD, string line ) ;
		void setsize( int, int, int, int ) ;

       friend class pPanel ;
       friend class field  ;
} ;


class field
{
	private:
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

		unsigned int field_row          ;
		unsigned int field_col          ;
		unsigned int field_cole         ;
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

		int  field_init( int MAXW, int MAXD, string line ) ;
		bool cursor_on_field( uint row, uint col ) ;
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
		void field_attr() ;
		int  end_of_field( WINDOW * win, uint col )   ;

       friend class pPanel ;
} ;


class literal
{
	private:
		literal() {
				literal_colour = 0     ;
				literal_name   = ""    ;
			  }
		int     literal_row    ;
		int     literal_col    ;
		int     literal_cole   ;
		cuaType literal_cua    ;
		uint    literal_colour ;
		string  literal_value  ;
		string  literal_name   ;

		int  literal_init( int MAXW, int MAXD, int & opt_field, string line ) ;
		void literal_display( WINDOW *, const string & ) ;
		bool cursor_on_literal( uint row, uint col ) ;
       friend class pPanel ;
} ;


class pdc
{
	public:
		pdc()   {
				pdc_name    = "" ;
				pdc_run     = "" ;
				pdc_parm    = "" ;
				pdc_unavail = "" ;
			} ;
		~pdc() {} ;
		string pdc_name ;
		string pdc_run  ;
		string pdc_parm ;
		string pdc_unavail ;
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

		~abc()  {
			if ( pd_created )
			{
				del_panel( panel ) ;
				delwin( win )      ;
			}
			} ;

		void add_pdc( string, string, string, string ) ;
		void display_abc_sel( WINDOW * )   ;
		void display_abc_unsel( WINDOW * ) ;
		void display_pd( uint, uint ) ;
		void hide_pd() ;
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

