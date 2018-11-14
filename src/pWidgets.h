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

class dynArea
{
	public:
		dynArea(){
				dynArea_dataInsp  = false ;
				dynArea_userModsp = false ;
				dynArea_dataModsp = false ;
				dynArea_Attrs     = ""    ;
				dynArea_inAttrs   = ""    ;
			 }

	private:
		uint   dynArea_row       ;
		uint   dynArea_col       ;
		uint   dynArea_width     ;
		uint   dynArea_depth     ;
		bool   dynArea_dataInsp  ;
		bool   dynArea_userModsp ;
		bool   dynArea_dataModsp ;
		char   dynArea_UserMod   ;
		char   dynArea_DataMod   ;
		string dynArea_Attrs     ;
		string dynArea_inAttrs   ;
		string dynArea_shadow_name ;

		void dynArea_init( errblock& err,
				   int maxw,
				   int maxd,
				   const string& line,
				   const string& da_dataIn,
				   const string& da_dataOut ) ;

		void setsize( int, int, int, int ) ;

       friend class pPanel ;
       friend class field  ;
} ;


class field
{
	public:
		static char field_paduchar ;
		static bool field_nulls    ;
		static uint field_intens   ;

	private:
		field() {
				field_pwd          = false ;
				field_changed      = false ;
				field_active       = true  ;
				field_cua          = NONE  ;
				field_colour1      = 0     ;
				field_colour2      = 0     ;
				field_attr_once    = false ;
				field_skip         = true  ;
				field_caps         = false ;
				field_paduser      = false ;
				field_padchar      = ' '   ;
				field_just         = 'L'   ;
				field_numeric      = false ;
				field_input        = false ;
				field_dynArea      = NULL  ;
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
		attType      field_cua          ;
		uint         field_colour1      ;
		uint         field_colour2      ;
		bool         field_attr_once    ;
		bool         field_skip         ;
		bool         field_caps         ;
		bool         field_paduser      ;
		char         field_padchar      ;
		char         field_just         ;
		bool         field_numeric      ;
		bool         field_input        ;
		dynArea*     field_dynArea      ;
		bool         field_tb           ;
		bool         field_scrollable   ;
		unsigned int field_scroll_start ;
		string       field_shadow_value ;
		set<size_t>  field_usermod      ;

		void field_init( errblock& err, int maxw, int maxd, const string& line ) ;
		void field_opts( errblock& err, string& )  ;
		void field_reset() ;
		bool cursor_on_field( uint row, uint col ) ;

		void display_field( WINDOW*,
				    map<unsigned char, uint>&,
				    map<unsigned char, uint>& ) ;

		bool edit_field_insert( WINDOW* win,
					char ch,
					char schar,
					int row,
					map<unsigned char, uint>&,
					map<unsigned char, uint>& ) ;

		bool edit_field_replace( WINDOW* win,
					 char ch,
					 char schar,
					 int row,
					 map<unsigned char, uint>&,
					 map<unsigned char, uint>& ) ;

		void edit_field_delete( WINDOW* win,
					int row,
					map<unsigned char, uint>&,
					map<unsigned char, uint>& ) ;

		int  edit_field_backspace( WINDOW* win,
					   int row,
					   map<unsigned char, uint>&,
					   map<unsigned char, uint>& ) ;

		void field_remove_nulls_da()    ;
		void field_blank( WINDOW* win ) ;
		void field_clear( WINDOW* win ) ;

		void field_erase_eof( WINDOW* win,
				      unsigned int col,
				      map<unsigned char, uint>&,
				      map<unsigned char, uint>& ) ;

		bool field_dyna_input( uint col )  ;
		int  field_dyna_input_offset( uint col )  ;
		void field_update_datamod_usermod( string*,
						   int ) ;
		void field_attr( errblock& err,
				 string attrs,
				 bool =false ) ;

		void field_attr() ;
		void field_prep_input()   ;
		void field_prep_display() ;
		void field_set_caps()     ;
		int  end_of_field( WINDOW* win,
				   uint col ) ;

       friend class pPanel ;
} ;


class literal
{
	public:
		static uint literal_intens ;

	private:
		literal() {
				literal_colour = 0  ;
				literal_name   = "" ;
				literal_dvars  = true ;
			  }
		uint    literal_row    ;
		uint    literal_col    ;
		uint    literal_cole   ;
		attType literal_cua    ;
		uint    literal_colour ;
		string  literal_value  ;
		string  literal_xvalue ;
		string  literal_name   ;
		bool    literal_dvars  ;

		void literal_init( errblock& err, int maxw, int maxd, int& opt_field, const string& line ) ;
		void literal_display( WINDOW* ) ;
		bool cursor_on_literal( uint row, uint col ) ;

       friend class pPanel ;
} ;


class pdc
{
	public:
		static uint pdc_intens ;

		pdc()   {
				pdc_desc    = "" ;
				pdc_xdesc   = "" ;
				pdc_dvars   = true ;
				pdc_run     = "" ;
				pdc_parm    = "" ;
				pdc_unavail = "" ;
				pdc_inact   = true ;
			} ;
		~pdc() {} ;
		pdc( const string& a, const string& b, const string& c, const string& d )
			{
				pdc_desc    = a  ;
				pdc_xdesc   = "" ;
				pdc_dvars   = true ;
				pdc_run     = b  ;
				pdc_parm    = c  ;
				pdc_unavail = d  ;
				pdc_inact   = false ;
			} ;
		string pdc_desc  ;
		string pdc_xdesc ;
		bool   pdc_dvars ;
		string pdc_run   ;
		string pdc_parm  ;
		string pdc_unavail ;
		bool   pdc_inact ;
		void   display_pdc_avail( WINDOW*, attType, int ) ;
		void   display_pdc_unavail( WINDOW*, attType, int ) ;
} ;


class abc
{
	public:
		static uint abc_intens ;
		abc()   {
				abc_maxh   = 0  ;
				abc_maxw   = 0  ;
				currChoice = 0  ;
				choiceVar  = "" ;
				pd_created = false ;
			} ;

		abc( fPOOL* p, bool b )
			{
				p_funcPOOL = p  ;
				selPanel   = b  ;
				abc_maxh   = 0  ;
				abc_maxw   = 0  ;
				currChoice = 0  ;
				choiceVar  = "" ;
				pd_created = false ;
			} ;

		~abc()  {
			if ( pd_created )
			{
				del_panel( panel ) ;
				delwin( win )      ;
			}
			} ;

		string       abc_desc ;
		unsigned int abc_col  ;
		unsigned int abc_maxh ;
		unsigned int abc_maxw ;

		static poolMGR* p_poolMGR ;

		void   add_pdc( const pdc& )         ;
		void   display_abc_sel( WINDOW* )    ;
		void   display_abc_unsel( WINDOW* )  ;
		void   display_pd( errblock&, uint, uint, uint ) ;
		void   hide_pd()    ;
		int    get_pd_col() ;
		void   get_msg_position( uint&, uint& ) ;
		const  string& get_abc_desc() ;
		pdc    retrieve_choice( errblock& ) ;
		bool   cursor_on_pulldown( uint, uint ) ;

	private:
		fPOOL* p_funcPOOL ;
		bool   pd_created ;
		bool   selPanel   ;
		int    currChoice ;
		string choiceVar  ;

		void   putDialogueVar( errblock&, const string&, const string& ) ;
		string getDialogueVar( errblock&, const string& ) ;
		string sub_vars( errblock&, string, bool& ) ;
		void   create_window( uint, uint ) ;

		vector<pdc> pdcList ;
		WINDOW* win  ;
		PANEL* panel ;
} ;


class Box
{
	public:
		static uint box_intens ;
		Box()   {
				box_title = "" ;
			}

		void box_init( errblock& err, int maxw, int maxd, const string& line ) ;
		void display_box( WINDOW*, string ) ;

		string box_title ;
	private:
		uint box_row    ;
		uint box_col    ;
		uint box_width  ;
		uint box_depth  ;
		uint box_colour ;
} ;

