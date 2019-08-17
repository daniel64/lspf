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
		dynArea()
		{
			dynArea_dataInsp  = false ;
			dynArea_userModsp = false ;
			dynArea_dataModsp = false ;
			dynArea_scroll    = false ;
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
		bool   dynArea_scroll    ;
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
		field()
		{
			field_pwd          = false ;
			field_changed      = false ;
			field_active       = true  ;
			field_cua          = NONE  ;
			field_colour1      = 0     ;
			field_colour2      = 0     ;
			field_attr_once    = false ;
			field_skip         = true  ;
			field_caps         = false ;
			field_nojump       = false ;
			field_paduser      = false ;
			field_padchar      = ' '   ;
			field_just         = 'L'   ;
			field_numeric      = false ;
			field_input        = false ;
			field_dynArea      = NULL  ;
			field_tb           = false ;
			field_scrollable   = false ;
			field_visible      = true  ;
			field_area         = 0     ;
			field_shadow_value = ""    ;
		} ;

		unsigned int field_row          ;
		unsigned int field_col          ;
		unsigned int field_area_row     ;
		unsigned int field_area_col     ;
		unsigned int field_endcol       ;
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
		bool         field_nojump       ;
		bool         field_paduser      ;
		char         field_padchar      ;
		char         field_just         ;
		bool         field_numeric      ;
		bool         field_input        ;
		dynArea*     field_dynArea      ;
		bool         field_tb           ;
		bool         field_scrollable   ;
		bool         field_visible      ;
		unsigned int field_area         ;
		string       field_shadow_value ;
		set<size_t>  field_usermod      ;

		void field_init( errblock& err,
				 int maxw,
				 int maxd,
				 const string& line,
				 uint = 0,
				 uint = 0 ) ;

		void field_opts( errblock& err, string& )  ;
		void field_reset() ;
		bool cursor_on_field( uint row, uint col ) ;

		void display_field( WINDOW*,
				    char schar,
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
					char schar,
					map<unsigned char, uint>&,
					map<unsigned char, uint>& ) ;

		int  edit_field_backspace( WINDOW* win,
					   int row,
					   char schar,
					   map<unsigned char, uint>&,
					   map<unsigned char, uint>& ) ;

		void field_remove_nulls_da()    ;
		void field_blank( WINDOW* win ) ;
		void field_clear( WINDOW* win ) ;

		void field_erase_eof( WINDOW* win,
				      unsigned int col,
				      char schar,
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
		uint field_get_colour1()  { return field_colour1 ; }
		uint field_get_row()      { return field_row     ; }

		int  end_of_field( WINDOW* win,
				   uint col ) ;

	friend class pPanel ;
	friend class Area   ;
} ;


class text
{
	public:
		static uint text_intens ;

	private:
		text()
		{
			text_colour  = 0  ;
			text_name    = "" ;
			text_visible = true ;
			text_dvars   = true ;
		}

		uint    text_row ;
		uint    text_col ;
		uint    text_area_row ;
		uint    text_area_col ;
		uint    text_endcol  ;
		attType text_cua     ;
		uint    text_colour  ;
		string  text_value   ;
		string  text_xvalue  ;
		string  text_name    ;
		bool    text_visible ;
		bool    text_dvars   ;

		void text_init( errblock& err,
				int maxw,
				int maxd,
				uint& opt_field,
				const string& line,
				uint = 0,
				uint = 0 ) ;
		void text_display( WINDOW* ) ;
		bool cursor_on_text( uint row, uint col ) ;

	friend class pPanel ;
	friend class Area   ;
} ;


class Area
{
	public:
		Area() :
		si1( "" ),
		si2( "More:     +" ),
		si3( "More:   -  " ),
		si4( "More:   - +" )
		{
			maxRow = 0 ;
			maxPos = 0 ;
			pos    = 0 ;
		}


	public:
		void Area_init( errblock& err,
				int maxw,
				int maxd,
				uint num,
				const string& line ) ;

		void add( text* ) ;
		void add( const string&, field* ) ;

		void get_info( uint&, uint&, uint& ) ;
		void get_info( uint&, uint&, uint&, uint& ) ;

		uint get_width()   { return Area_width ; }
		uint get_depth()   { return Area_depth ; }
		uint get_col()     { return Area_col   ; }
		uint get_num()     { return Area_num   ; }

		bool not_defined() { return fieldList.empty() && textList.empty() ; }
		void make_visible( field* ) ;

		bool cursor_on_area( uint, uint ) ;

		void check_overlapping_fields( errblock&, const string& ) ;

		void update_area() ;

		int  scroll_up( uint, uint ) ;
		int  scroll_down( uint, uint ) ;

		const char* get_scroll_indicator() ;

	private:
		uint pos ;
		uint maxRow ;
		uint maxPos ;

		uint Area_num ;
		uint Area_row ;
		uint Area_col ;
		uint Area_width ;
		uint Area_depth ;

		const char* si1 ;
		const char* si2 ;
		const char* si3 ;
		const char* si4 ;

		void update_fields() ;
		void update_text() ;

		map<string, field*> fieldList ;
		vector<text*> textList ;

	friend class field ;
} ;


class pdc
{
	public:
		static uint pdc_intens ;

		pdc()
		{
			pdc_desc    = "" ;
			pdc_xdesc   = "" ;
			pdc_dvars   = true ;
			pdc_run     = "" ;
			pdc_parm    = "" ;
			pdc_unavail = "" ;
			pdc_inact   = true ;
		} ;

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
		static poolMGR* p_poolMGR ;

		abc()
		{
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
			abc_mnem1  = 0  ;
			choiceVar  = "" ;
			pd_created = false ;
		} ;

		~abc()
		{
			if ( pd_created )
			{
				del_panel( panel ) ;
				delwin( win )      ;
			}
		} ;

		string       abc_desc  ;
		char         abc_mnem2 ;
		unsigned int abc_mnem1 ;
		unsigned int abc_col   ;
		unsigned int abc_maxh  ;
		unsigned int abc_maxw  ;

		void   add_pdc( const pdc& )         ;
		void   display_abc_sel( WINDOW* )    ;
		void   display_abc_unsel( WINDOW* )  ;
		void   display_pd( errblock&, const string&, uint, uint, uint ) ;
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

		Box()
		{
			box_title = "" ;
		}

		void box_init( errblock& err, int maxw, int maxd, const string& line ) ;
		void display_box( WINDOW*, string ) ;
		void display_box( WINDOW*, string, uint, uint ) ;

		string box_title ;
	private:
		uint box_row    ;
		uint box_col    ;
		uint box_width  ;
		uint box_depth  ;
		uint box_colour ;

		void draw_box( WINDOW*, string& ) ;
} ;

