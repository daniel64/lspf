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

class pPanel
{
	public:
		string cmdfield ;
		bool   showLMSG ;

		static uint panel_intens  ;
		static poolMGR* p_poolMGR ;
		static logger* lg         ;

		pPanel()  ;
		~pPanel() ;

		void   init( errblock& err ) ;

		void   field_edit( uint row, uint col, char ch, bool Istr, bool& prot ) ;
		void   field_delete_char( uint row, uint col, bool& prot ) ;
		void   field_backspace( uint& row, uint& col, bool& prot ) ;
		void   field_erase_eof( uint row, uint col, bool& prot )   ;
		void   field_tab_down( uint& row, uint& col ) ;
		void   field_tab_next( uint& row, uint& col ) ;
		const string& field_getvalue( const string& field ) ;
		const string& field_getrawvalue( const string& field ) ;
		bool   field_valid( const string& field ) ;
		fieldExc field_getexec( const string& )   ;
		void   field_setvalue( const string& field, const string& value ) ;
		string field_getname( uint row, uint col ) ;
		bool   field_get_row_col( const string& fld, uint& row, uint& col ) ;
		int    field_get_col( const string& fld ) ;
		bool   field_nonblank( const string&, uint ) ;
		bool   keep_cmd() ;

		void   cursor_placement( errblock& ) ;
		void   cursor_to_cmdfield( unsigned int =1 ) ;
		void   cursor_to_next_field( const string& name, uint& row, uint& col ) ;
		void   cursor_eof( uint& row, uint& col )  ;
		void   get_cursor( uint& row, uint& col ) { row   = p_row + win_row ; col   = p_col + win_col ; }
		void   set_cursor( uint row, uint col )   { p_row = row - win_row   ; p_col = col - win_col   ; }

		int    on_abline( uint row ) { if ( ab.size() > 0 ) return (win_row == row) ; else return false ; }
		bool   pd_active()           { return pdActive ; }
		void   get_pd_home( uint& row, uint& col ) ;
		bool   display_pd( errblock&, uint row, uint col, string& ) ;
		void   display_pd( errblock& ) ;
		void   display_current_pd( errblock&, uint row, uint col ) ;
		void   hide_popup() ;
		void   hide_pd()    ;
		void   remove_pd()  ;
		bool   jump_field( uint, uint, string& ) ;
		bool   cursor_on_pulldown( uint, uint ) ;
		void   display_next_pd( errblock&, const string&, string& ) ;
		void   display_msg( errblock& ) ;
		void   display_area_si() ;
		pdc    retrieve_choice( errblock&, string& ) ;
		void   toggle_fscreen( bool, int, int ) ;

		void   redraw_fields( errblock& ) ;
		void   refresh_fields( errblock& ) ;

		const string& cmd_getvalue() ;
		void   cmd_setvalue( const string& ) ;

		bool    get_tbscan()                   { return tb_scan    ; }

		bool   is_cmd_inactive( const string& value ) ;

		bool   has_command_field()             { return fieldList.count( cmdfield ) > 0 ; }
		bool   on_border_line( uint, uint )  ;
		bool   hide_msg_window( uint, uint ) ;
		void   selPanel( bool x ) { selectPanel = x ; }

		void   set_pfpressed( const string& s="" ) { pfkey = s    ; }
		const string& get_pfpressed()              { return pfkey ; }

		string get_keylist( int ) ;
		void   point_and_shoot( uint, uint ) ;

		void   reset_cmd() ;

		const string get_zprim() { return primaryMenu ? "YES" : "NO" ; }

	private:
		string home        ;
		string scroll      ;
		string panelid     ;
		int    taskId      ;
		int    tb_curidr   ;
		int    curpos      ;
		slmsg  MSG         ;
		string pfkey       ;
		string keylistn    ;
		string keyappl     ;
		string keyhelpn    ;
		bool   Rexx        ;
		string zzcmd       ;
		string pos_smsg    ;
		string pos_lmsg    ;
		string da_dataIn   ;
		string da_dataOut  ;
		string Area1       ;
		int    tb_start    ;
		int    tb_depth    ;
		int    tb_curidx   ;
		int    tb_csrrow   ;
		int    tb_crp      ;
		bool   tb_scan     ;
		bool   tb_autosel  ;
		bool   pdActive    ;
		int    abc_pos     ;
		bool   primaryMenu ;
		bool   selectPanel ;
		bool   scrollOn    ;
		string panelTitle  ;
		string panelDesc   ;
		uint   abIndex     ;
		uint   tb_toprow   ;
		int    tb_lcol     ;
		int    tb_lsz      ;
		bool   msgResp     ;
		bool   nretriev    ;
		bool   lrScroll    ;
		bool   forEdit     ;
		bool   forBrowse   ;
		bool   bypassCur   ;
		bool   redisplay   ;
		string nretfield   ;
		uint   dyn_depth   ;
		uint   dyn_width   ;
		uint   zscrnum     ;
		uint   zscrmaxd    ;
		uint   zscrmaxw    ;
		uint   wscrmaxd    ;
		uint   wscrmaxw    ;
		uint   p_row       ;
		uint   p_col       ;
		uint   p_version   ;
		uint   p_format    ;
		string msgloc      ;
		bool   tb_model    ;
		bool   win_addpop  ;
		bool   end_pressed ;
		bool   full_screen ;
		bool   error_msg   ;
		uint   win_width   ;
		uint   win_depth   ;
		uint   win_row     ;
		uint   win_col     ;
		char   inv_schar   ;
		WINDOW* win        ;
		WINDOW* fwin       ;
		WINDOW* pwin       ;
		WINDOW* bwin       ;
		WINDOW* smwin      ;
		WINDOW* lmwin      ;
		WINDOW* idwin      ;
		PANEL*  panel      ;
		PANEL*  bpanel     ;
		PANEL*  smpanel    ;
		PANEL*  lmpanel    ;
		PANEL*  idpanel    ;

		USR_ACTIONS usr_action ;

		string  dAlarm  ;
		string  dCursor ;
		int     dCsrpos ;
		string  dHelp   ;
		string  dMsg    ;
		string  dResp   ;
		string  dTrail  ;
		string  dZvars  ;

		string  iMsg    ;
		string  iCursor ;
		int     iCsrpos ;

		short int* fieldMap   ;
		field**    fieldAddrs ;

		const string validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#$@" ;

		fPOOL* p_funcPOOL ;

		void   init_control_variables() ;
		void   reset_control_variables() ;
		void   loadPanel( errblock&, const string&, const string& ) ;
		void   readPanel( errblock&, vector<string>&, const string&, const string&, string ) ;

		void   build_jumpList() ;
		void   build_fieldMap() ;
		void   build_Areas( errblock& ) ;
		void   rebuild_after_area_scroll( Area* ) ;

		void   check_overlapping_fields( errblock& ) ;

		void   createPanel_Refresh( errblock&, parser&, panstmnt* ) ;
		void   createPanel_Vputget( errblock&, parser&, panstmnt* ) ;
		void   createPanel_Assign( errblock&, parser&, panstmnt*, bool ) ;
		void   createPanel_Verify( errblock&, parser&, panstmnt* ) ;
		void   createPanel_If( errblock&, parser&, panstmnt*, bool, bool ) ;
		void   createPanel_Else( errblock&, parser&, panstmnt*, vector<panstmnt*>*, bool, bool ) ;

		void   display_panel( errblock& ) ;
		void   display_panel_update( errblock& ) ;
		void   display_panel_init( errblock& )   ;
		void   display_panel_attrs( errblock& )  ;
		void   display_panel_reinit( errblock&, int ln=0 ) ;
		void   display_panel_proc( errblock&, int ln )     ;

		int   get_csrpos()         { return ( dCsrpos == 0  ) ? iCsrpos : dCsrpos ; }
		const string& get_cursor() { return ( dCursor == "" ) ? iCursor : dCursor ; }
		const string& get_help()   { return dHelp   ; }
		const string& get_msg()    { return ( dMsg == "" ) ? iMsg : dMsg ; }
		const string& get_resp()   { return dResp   ; }
		const string& get_trail()  { return dTrail  ; }
		const string& get_zvars()  { return dZvars  ; }

		void   refresh() ;
		void   refresh_field( errblock&, map<string, field*>::iterator, const string& ) ;
		void   refresh_fields( errblock&, int, const string& ) ;

		void   abc_panel_init( errblock&, const string& ) ;
		void   abc_panel_proc( errblock&, const string& ) ;

		void   display_id( errblock& ) ;

		void   set_cursor( const string&, int ) ;
		string get_tbCursor( const string& ) ;
		string get_tbCursor( const string&, int ) ;
		void   set_msg( const string& m ) { iMsg = m ; }
		string get_msgloc() ;
		void   set_msgloc( const string& m ) { msgloc = m ; }

		bool   do_redisplay() ;

		void   set_cursor_idr( int i ) { tb_curidr = i ; }
		void   set_cursor_home() ;
		void   get_home( uint& row, uint& col ) ;

		void   set_popup( bool, int&, int& ) ;
		void   remove_popup() ;
		void   move_popup()   ;
		void   show_popup()   ;
		void   create_panels( popup& ) ;
		void   delete_panels( const popup& ) ;

		void   set_userAction( USR_ACTIONS a ) { usr_action = a ; }

		void   put_keylist( int, const string& ) ;
		string get_panelDesc()  { return panelDesc != "" ? sub_vars( panelDesc ) : sub_vars( panelTitle ) ; }

		void   update_field_values( errblock& ) ;

		void   get_panel_info( errblock& err,
				       const string& a_name,
				       const string& t = "",
				       const string& w = "",
				       const string& d = "",
				       const string& r = "",
				       const string& c = "" ) ;

		field* get_field_address( uint, uint ) ;

		void   attr( errblock&, const string& field, const string& attrs ) ;
		void   attrchar( errblock&, const char, string& attrs ) ;


		void   tb_set_csrrow( int i )   { tb_csrrow  = i    ; }
		int    tb_get_csrrow()          { return tb_csrrow  ; }
		void   tb_set_autosel( bool b ) { tb_autosel = b    ; }
		void   tb_set_crp( int i )      { tb_crp     = i    ; }
		void   tb_set_linesChanged( errblock& ) ;
		void   tb_add_autosel_line( errblock& ) ;
		bool   tb_get_lineChanged( errblock&, int&, string& ) ;
		bool   tb_linesPending() ;
		void   tb_clear_linesChanged( errblock& )  ;
		void   tb_remove_lineChanged() ;

		string  get_field_help( uint row, uint col ) ;
		bool    get_nretriev()  { return nretriev  ; }
		string& get_nretfield() { return nretfield ; }

		void   set_panel_msg( const slmsg&, const string& ) ;
		void   clear_msg() ;
		void   clear_msg_loc()  { msgloc = ""     ; }
		bool   inputInhibited() { return ( pdActive || msgResp ) ; }
		bool   msgInhibited()   { return msgResp  ; }
		void   msgResponseOK()  { msgResp = false ; }

		void   set_message_cond( const string& ) ;
		void   set_cursor_cond( const string&, int =-1 ) ;

		void   reset_attrs() ;
		void   reset_attrs_once() ;

		void   set_panel_fscreen() ;
		void   unset_panel_fscreen() ;

		void   unset_panel_decolourised() ;
		bool   is_panel_decolourised() ;

		void   set_panel_frame_act()  ;
		bool   is_panel_frame_inact() ;

		void   set_panel_ttl( const string& ) ;
		const string get_panel_ttl( PANEL* ) ;

		void   redraw_panel( errblock& ) ;

		void   draw_frame( PANEL*, uint ) ;
		void   draw_frame( errblock& ) ;

		void   syncDialogueVar( errblock&, const string& ) ;
		string getDialogueVar( errblock&, const string& )   ;
		void   putDialogueVar( errblock&, const string&, const string& ) ;
		string getControlVar( errblock&, const string& ) ;
		void   setControlVar( errblock&, const string&, const string&, PS_SECT ) ;
		string getVariable( errblock&, const vparm& ) ;

		void   create_tbfield( errblock&, int col,
				       int size,
				       attType cuaFT,
				       const string& name,
				       const string& opts ) ;

		void   create_tbfield( errblock&, const string& ) ;
		void   create_pdc( errblock&,
				   const string&,
				   uint,
				   const string& ) ;

		void   display_boxes() ;

		void   display_tb_mark_posn( errblock& )    ;
		void   set_tb_fields_act_inact( errblock& ) ;

		void   decolourise( WINDOW*, uint, uint, uint ) ;

		string sub_vars( string s ) ;
		string sub_vars( string s, bool& ) ;

		bool   tb_field( const string& ) ;

		vector<string>attrList ;
		map<int, string> tb_linesChanged ;

		vector<text*> textList ;
		vector<text*> text_pas ;
		vector<abc*> ab                   ;
		vector<Box*> boxes                ;
		map<string, field*> fieldList     ;
		map<string, field*> jumpList      ;
		map<string, field*> field_pas     ;
		map<string, Area*> AreaList       ;
		map<uint, Area*> AreaNum          ;
		map<string, dynArea*> dynAreaList ;
		map<string, pnts> pntsTable       ;
		map<string, string> fieldHList    ;
		map<int, string> Keylistl         ;

		map<string, CV_CONTROL> control_vars ;
		map<string, fieldExc> fieldExcTable  ;

		map<string, vector<panstmnt*>> abc_initstmnts ;
		map<string, vector<panstmnt*>> abc_procstmnts ;

		vector<panstmnt*> procstmnts ;
		vector<panstmnt*> initstmnts ;
		vector<panstmnt*> reinstmnts ;

		set<string> tb_fields ;
		set<string> tb_clear  ;

		set<string>& get_tb_fields() { return tb_fields ; }
		set<string>& get_tb_clear()  { return tb_clear ; }

		string get_first_tb_field() ;

		map<unsigned char, char_attrs> char_attrlist ;
		map<unsigned char, unsigned int> colour_attrlist ;
		map<unsigned char, unsigned int> ddata_map ;
		map<unsigned char, unsigned int> schar_map ;

		void   display_text() ;
		void   display_ab()   ;
		void   display_fields( errblock&, bool =false ) ;

		void   process_panel_stmnts( errblock& err,
					     int ln,
					     vector<panstmnt*>& stmnts,
					     PS_SECT ) ;

		void   process_panel_assignment( errblock& err,
						 int ln,
						 ASSGN* assgn,
						 set<string>& attr,
						 set<string>& attrchar,
						 PS_SECT ) ;

		void   process_panel_attr( errblock& err,
					   int ln,
					   const string& rhs,
					   ASSGN* assgn,
					   set<string>& attr,
					   PS_SECT ) ;

		void   process_panel_attrchar( errblock& err,
					       string& rhs,
					       ASSGN* assgn,
					       set<string>& attrchar ) ;

		void   process_panel_vputget( errblock& err,
					      VPUTGET* vputget ) ;

		void   process_panel_verify( errblock& err,
					     int ln,
					     VERIFY* verify ) ;

		void   process_panel_if( errblock& err,
					 int ln,
					 IFSTMNT* ifstmnt ) ;

		void   process_panel_if_cond( errblock& err,
					      int ln,
					      IFSTMNT* ifstmnt ) ;

		string process_panel_trunc( errblock& err,
					    TRUNC* trunc ) ;

		string process_panel_trans( errblock& err,
					    int ln,
					    TRANS* trans,
					    const string& ) ;

		void   process_panel_function( PN_FUNCTION,
					       string& ) ;

		bool   error_msg_issued() { return error_msg ; }

		void   get_msgwin( errblock&,
				   string,
				   uint&,
				   uint&,
				   uint&,
				   uint&,
				   vector<string>& ) ;

		void   panel_cleanup( PANEL* ) ;

		void   update_keylist_vars( errblock& ) ;

		friend class pApplication ;
		friend class pLScreen     ;
} ;


class panel_data_ext
{
	public:
		explicit panel_data_ext()
		{
			winttl       = ""    ;
			fscreen      = false ;
			frame_inact  = false ;
			decolourised = false ;
		}

	private:
		string winttl ;

		bool fscreen ;
		bool frame_inact  ;
		bool decolourised ;

		friend class panel_data ;
} ;


class panel_data
{
	public:
		explicit panel_data()
		{
			data_ext = new panel_data_ext ;
		}

		explicit panel_data( uint x )
		{
			screenId = x ;
			ppanel   = NULL ;
			frame    = false ;
			data_ext = new panel_data_ext ;
			data_ext->fscreen = false ;
		}

		explicit panel_data( uint x, bool b, pPanel* p )
		{
			screenId = x ;
			ppanel   = p ;
			frame    = true ;
			data_ext = new panel_data_ext ;
			data_ext->fscreen = false ;
		}

		explicit panel_data( uint x, pPanel* p, bool f = false )
		{
			screenId = x ;
			ppanel   = p ;
			frame    = false ;
			data_ext = new panel_data_ext ;
			data_ext->fscreen = f ;
		}

		~panel_data()
		{
			delete data_ext ;
		}

		void set_fscreen() const
		{
			data_ext->fscreen = true ;
		}

		void unset_fscreen() const
		{
			data_ext->fscreen = false ;
		}

		bool is_fscreen() const
		{
			return data_ext->fscreen ;
		}

		void set_decolourised() const
		{
			data_ext->decolourised = true ;
		}

		void unset_decolourised() const
		{
			data_ext->decolourised = false ;
		}

		bool is_decolourised() const
		{
			return data_ext->decolourised ;
		}

		bool is_frame() const
		{
			return frame ;
		}

		void set_frame_inact() const
		{
			data_ext->frame_inact = true ;
		}

		void set_frame_act() const
		{
			data_ext->frame_inact = false ;
		}

		bool is_frame_act() const
		{
			return !data_ext->frame_inact ;
		}

		bool is_frame_inact() const
		{
			return data_ext->frame_inact ;
		}

		void set_winttl( const string& ttl ) const
		{
			data_ext->winttl = ttl ;
		}

		const string& get_winttl() const
		{
			return data_ext->winttl ;
		}


		uint screenId  ;

		bool frame     ;

		pPanel* ppanel ;

		panel_data_ext* data_ext ;
} ;


#undef llog
#undef debug1
#undef debug2


#define llog(t, s) \
{ \
lg->lock() ; \
(*lg) << microsec_clock::local_time() << \
" PANEL     " << \
" " << d2ds( taskId, 5 ) << " " << t << " " << s ; \
lg->unlock() ; \
}

#ifdef DEBUG1
#define debug1( s ) \
{ \
lg->lock() ; \
(*lg) << microsec_clock::local_time() << \
" PANEL     " << \
" " << d2ds( taskId, 5 ) << \
" D line: "  << __LINE__  << \
" >>L1 Function: " << __FUNCTION__ << \
" -  " << s ; \
lg->unlock() ; \
}
#else
#define debug1( s )
#endif


#ifdef DEBUG2
#define debug2( s ) \
{ \
lg->lock() ; \
(*lg) << microsec_clock::local_time() << \
" PANEL     " << \
" " << d2ds( taskId, 5 ) << \
" D line: "  << __LINE__  << \
" >>L2 Function: " << __FUNCTION__ << \
" -  " << s ; \
lg->unlock() ; \
}
#else
#define debug2( s )
#endif
