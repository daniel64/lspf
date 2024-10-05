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

#define _PANEL    10
#define _ATTR     20
#define _ABC      30
#define _ABCINIT  40
#define _ABCPROC  50
#define _BODY     60
#define _AREA     70
#define _INIT     80
#define _REINIT   90
#define _PROC    100
#define _HELP    110
#define _PNTS    110
#define _FIELD   110


struct exitInfo1
{
	//
	// PANEXIT REXX and *REXX statement exit structure.
	//

	char   type ;
	string panelid ;
	string psection ;
	string exdata ;
	string zrxrc  ;
	string zrxmsg ;
	void*  panel ;
	void*  pAppl ;
	errblock* err ;
	set<string>* vars ;
} ;


class pPanel
{
	public:
		string cmdfield ;
		bool   showLMSG ;

		static uint panel_intens ;
		static uint pfkgToken ;
		static poolMGR* p_poolMGR ;
		static logger* lg ;
		static bool tabpas ;

		pPanel()  ;
		pPanel( errblock& err,
			fPOOL*,
			void*,
			bool,
			bool,
			bool,
			uint,
			const string& ) ;

		~pPanel() ;

		void   field_edit( uint,
				   uint,
				   char,
				   bool,
				   bool& ) ;

		void   field_delete_char( uint,
					  uint,
					  bool& ) ;

		void   field_backspace( uint&,
					uint&,
					bool& ) ;

		void   field_erase_eof( uint,
					uint,
					bool&,
					bool = false ) ;

		void   field_erase_spaces( uint,
					   uint,
					   bool& ) ;

		void   field_erase_word( uint,
					 uint,
					 bool& ) ;

		void   field_tab_down( uint&,
				       uint& ) ;

		void   field_tab_next( uint&,
				       uint& ) ;

		void   field_tab_prev( uint&,
				       uint& ) ;

		const string& field_getvalue( const string& ) ;
		const string& field_getvalue_caps( const string& ) ;
		const string& field_getrawvalue( const string& ) ;
		bool   field_exists( const string& ) ;
		fieldXct field_getexec( const string& )   ;
		void   field_setvalue( const string&,
				       const string& ) ;
		void   field_setvalue( field*,
				       const string& ) ;
		string field_getname( uint,
				      uint ) ;
		string field_getname_input( uint,
					    uint ) ;
		string field_getname_nocmd() ;
		bool   field_get_row_col( const string& ,
					  uint&,
					  uint& ) ;
		bool   cursor_on_field( const string&,
					uint,
					uint ) ;
		bool   field_is_input( const string& ) ;

		int    field_get_col( const string& ) ;
		uint   field_get_scroll_pos( const string& ) ;
		bool   field_nonblank( const string&, uint ) ;
		bool   keep_cmd() ;

		void   cursor_placement( errblock&,
					 const string&,
					 int ) ;
		void   cursor_placement_display( errblock& ) ;
		void   cursor_placement_tbdispl( errblock& ) ;
		void   cursor_to_cmdfield( unsigned int = 1 ) ;
		void   cursor_to_next_field( const string&,
					     uint&,
					     uint& ) ;

		void   cursor_eof( uint,
				   uint& ) ;

		void   cursor_sof( uint,
				   uint& ) ;

		void   cursor_first_sof( uint,
					 uint& ) ;

		void   cursor_top_unprot( uint&,
					  uint& ) ;

		bool   cursor_next_word( uint,
					 uint& ) ;

		bool   cursor_prev_word( uint,
					 uint& ) ;

		bool   cursor_first_word( uint,
					  uint& ) ;

		bool   cursor_last_word( uint,
					 uint& ) ;

		void   get_pcursor( uint& row, uint& ) ;
		void   set_pcursor( uint row, uint ) ;
		void   get_lcursor( uint& row, uint& ) ;
		void   set_lcursor( uint row, uint ) ;

		void   cursor( uint,
			       uint ) ;

		int    cursor_on_ab( uint row ) { return ( ab.size() > 0 ) ? ( win_row == row ) : false ; }
		bool   pd_active()              { return pdActive ; }
		void   get_pd_home( uint&,
				    uint& ) ;
		bool   display_pd( errblock&,
				   uint,
				   uint,
				   string&,
				   uint = 0 ) ;
		void   display_pd( errblock& ) ;
		void   display_current_pd( errblock&,
					   uint,
					   uint ) ;
		void   hide_popup() ;
		void   hide_pd( errblock& ) ;
		void   remove_pd( errblock& ) ;

		bool   jump_field( uint,
				   uint,
				   string& ) ;

		bool   cursor_on_pulldown( uint,
					   uint ) ;

		void   display_next_pd( errblock&,
					const string&,
					string&,
					string& ) ;

		void   display_prev_pd( errblock&,
					const string&,
					string&,
					string& ) ;

		void   display_msg( errblock& ) ;
		void   display_lmsg( errblock& ) ;
		void   display_area_si() ;
		pdc    retrieve_choice( errblock&,
					string& ) ;
		void   toggle_fscreen( errblock&,
				       bool,
				       int,
				       int ) ;

		void   redraw_fields( errblock& ) ;
		void   refresh_fields( errblock& ) ;

		const string& cmd_getvalue() ;
		void   cmd_setvalue( const string& ) ;
		bool   cmd_nonblank() ;

		bool   get_tbscan()  { return tb_scan ; }

		bool   is_cmd_inactive( const string& value,
					uint,
					uint ) ;

		bool   has_command_field()  { return fieldList.count( cmdfield ) > 0 ; }

		bool   cursor_on_border_line( uint,
					      uint ) ;

		bool   cursor_in_window_border( uint,
						uint ) ;

		bool   hide_msg_window( uint,
					uint ) ;

		bool   cursor_on_msg_window( uint,
					     uint ) ;

		void   set_pfpressed( const string& s = "" ) { pfkey = s    ; }

		const string& get_pfpressed() { return pfkey ; }

		void   build_pfkeys( errblock&,
				     bool = false ) ;
		string get_pfkey( errblock&,
				  int ) ;

		void   add_linecmd( errblock&,
				    uint,
				    uint,
				    string ) ;

		void   reset_cmd() ;

		bool   error_msg_issued() { return error_msg ; }

		bool   is_popup()         { return win && win == pwin ; }

		bool   simulate_enter()   { return dResp == "ENTER" ; }

		help  get_field_help( uint row,
				      uint col ) ;
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
		string keytype     ;
		bool   keyshr      ;
		string keyhelpn    ;
		uint   pfklToken   ;
		string zzcmd       ;
		string pos_smsg    ;
		string pos_lmsg    ;
		string da_dataIn   ;
		string da_dataOut  ;
		string Area1       ;
		string rxpath      ;
		bool   tb_model    ;
		bool   tb_fixed    ;
		bool   tb_scan     ;
		bool   tb_autosel  ;
		bool   tb_dvars    ;
		int    tb_start    ;
		int    tb_modlines ;
		int    tb_pdepth   ;
		int    tb_rdepth   ;
		int    tb_curidx   ;
		int    tb_csrrow   ;
		int    tb_crp      ;
		bool   pdActive    ;
		uint   abc_pos     ;
		bool   primaryMenu ;
		bool   selPanel    ;
		bool   scrollOn    ;
		bool   zshowpfk    ;
		bool   pfk_built   ;
		string panelTitle  ;
		string panelDesc   ;
		uint   abIndex     ;
		uint   tb_toprow   ;
		int    cvd_lines   ;
		bool   msgResp     ;
		bool   nretriev    ;
		bool   lrScroll    ;
		bool   forEdit     ;
		bool   forBrowse   ;
		bool   bypassCur   ;
		bool   redisplay   ;
		bool   ex_attrchar ;
		string zdatef      ;
		string nretfield   ;
		string row_text    ;
		uint   dyns_toprow ;
		uint   dyns_depth  ;
		uint   dyns_width  ;
		uint   zscrnum     ;
		uint   zscrmaxd    ;
		uint   zscrmaxw    ;
		uint   wscrmaxd    ;
		uint   wscrmaxw    ;
		uint   l_row       ;
		uint   l_col       ;
		uint   s_row       ;
		uint   s_col       ;
		uint   p_version   ;
		uint   p_format    ;
		string msgloc      ;
		bool   win_addpop  ;
		bool   end_pressed ;
		bool   full_screen ;
		bool   error_msg   ;
		bool   tutorial    ;
		uint   win_width   ;
		uint   win_depth   ;
		uint   win_row     ;
		uint   win_col     ;
		bool   term_resize ;
		bool   displayed   ;
		char   inv_schar   ;
		WINDOW* win        ;
		WINDOW* owin       ;
		WINDOW* fwin       ;
		WINDOW* pwin       ;
		WINDOW* bwin       ;
		WINDOW* smwin      ;
		WINDOW* lmwin      ;
		WINDOW* idwin      ;
		WINDOW* pfkwin     ;
		PANEL*  panel      ;
		PANEL*  bpanel     ;
		PANEL*  smpanel    ;
		PANEL*  lmpanel    ;
		PANEL*  idpanel    ;
		PANEL*  pfkpanel   ;

		void* pAppl ;

		int taskid() { return taskId ; }

		USR_ACTIONS usr_action ;

		string  dAlarm  ;
		string  dCursor ;
		int     dCsrpos ;
		string  dHelp   ;
		string  dHHelp  ;
		string  dHist   ;
		string  dMsg    ;
		string  dResp   ;
		string  dTrail  ;
		string  dZvars  ;

		string  iMsg    ;
		string  iCursor ;
		int     iCsrpos ;

		int     ztdtop ;
		int     ztdrows ;
		int     ztdvrows ;

		string  table ;

		short int* fieldMap   ;
		field**    fieldAddrs ;

		exitInfo1 exitinfo1 ;
		exitInfo2 exitinfo2 ;

		const string validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#$@" ;

		fPOOL* funcPool ;

		void   init_control_variables() ;
		void   reset_control_variables() ;
		void   load_panel( errblock&,
				   const string&,
				   const string& ) ;

		void   load_panel_help( errblock&,
					string& ) ;

		void   load_panel_pnts( errblock&,
					const string& ) ;

		void   load_panel_field( errblock&,
					 string& ) ;

		void   load_panel_attr( errblock&,
					string&,
					const string&,
					string& ) ;

		void   read_source( errblock&,
				    vector<string>&,
				    string,
				    const string&,
				    const string& ) ;

		void   preprocess_source( errblock&,
					  vector<string>&,
					  vector<string>&,
					  const string& ) ;

		void   convert_source( errblock&,
				       vector<string>& ) ;

		bool   reload() ;

		void   build_jumpList() ;
		void   build_fieldMap() ;
		void   build_Areas( errblock& ) ;

		void   rebuild_after_area_scroll( Area* ) ;
		void   scroll_all_areas_to_top() ;
		void   scroll_all_fields_to_start( errblock& ) ;

		field* field_getaddr_any( uint row,
					  uint col ) ;

		string field_getname_any( field* ) ;

		void   check_overlapping_fields( errblock&,
						 bool = false ) ;

		void   createPanel_Refresh( errblock&,
					    parser&,
					    panstmnt* ) ;

		void   createPanel_Vputget( errblock&,
					    parser&,
					    panstmnt* ) ;

		void   createPanel_Assign( errblock&,
					   parser&,
					   panstmnt*,
					   bool ) ;

		void   createPanel_Vedit( errblock&,
					  parser&,
					  panstmnt* ) ;

		void   createPanel_prexx( errblock&,
					  parser&,
					  panstmnt* ) ;

		void   createPanel_Verify( errblock&,
					   parser&,
					   panstmnt* ) ;

		void   createPanel_If( errblock&,
				       parser&,
				       panstmnt*,
				       bool,
				       bool ) ;

		void   createPanel_Else( errblock&,
					 parser&,
					 panstmnt*,
					 vector<panstmnt*>*,
					 bool,
					 bool ) ;

		void   createPanel_REXX( errblock&,
					 parser&,
					 panstmnt* ) ;

		void   createPanel_PANEXIT( errblock&,
					    parser&,
					    panstmnt* ) ;

		void   setup_panelREXX( errblock&,
					vector<panstmnt*>*,
					PREXX*,
					vector<string>&,
					vector<string>::iterator& ) ;

		void   display_panel( errblock&,
				      bool = true ) ;
		void   display_panel_update( errblock& ) ;
		void   display_panel_init( errblock& )   ;
		void   display_panel_attrs( errblock& )  ;
		void   display_panel_reinit( errblock&, int = 0 ) ;
		void   display_panel_proc( errblock&, int ) ;

		int   get_csrpos()         { return ( dCsrpos == 0  ) ? iCsrpos : dCsrpos ; }
		const string& get_cursor() { return ( dCursor == "" ) ? iCursor : dCursor ; }
		const string& get_msg()    { return ( trim( dMsg ) == "" ) ? trim( iMsg ) : dMsg ; }
		const string& get_resp()   { return dResp   ; }
		const string& get_trail()  { return dTrail  ; }
		const string& get_zvars()  { return dZvars  ; }

		string get_help() ;
		string get_exhelp( errblock& ) ;

		void   refresh() ;
		void   refresh_field( errblock&,
				      field*,
				      const string& ) ;
		void   refresh_fields( errblock&,
				       int,
				       const string& ) ;

		void   abc_panel_init( errblock&,
				       const string& ) ;
		void   abc_panel_proc( errblock&,
				       const string& ) ;

		void   restore() ;

		void   display_id( errblock& ) ;

		void   display_pfkeys( errblock& ) ;

		void   set_cursor( const string&,
				   int ) ;

		void   clear_cursor_cond() ;

		void   point_and_shoot( errblock& ) ;

		string get_tbCursor( const string& ) ;
		string get_tbCursor( const string&,
				     int ) ;

		void   set_msg( const string& m ) { iMsg = m ; }
		string get_msgloc() ;
		void   set_msgloc( errblock&,
				   const string& ) ;

		bool   do_redisplay() ;

		void   set_cursor_idr( int i ) { tb_curidr = i ; }
		void   set_lcursor_home() ;
		void   set_lcursor_scroll() ;

		void   get_home1( uint& row,
				  uint& col ) ;

		void   get_home2( uint& row,
				  uint& col ) ;

		void   set_popup( errblock&,
				  bool,
				  int&,
				  int& ) ;
		void   remove_popup() ;
		void   move_popup() ;
		void   show_popup() ;
		void   create_panels( popup& ) ;
		void   pop_panels( const popup& ) ;

		void   set_userAction( USR_ACTIONS a ) { usr_action = a ; }

		void   put_keylist( int,
				    const string* ) ;
		void   put_keyattr( int,
				    const string* ) ;
		void   put_keylabl( int,
				    const string* ) ;

		string get_panelDesc()  { return ( panelDesc != "" ) ? sub_vars( panelDesc ) : sub_vars( panelTitle ) ; }

		void   update_field_values( errblock& ) ;

		void   get_panel_info( errblock& err,
				       const string&,
				       const string& = "",
				       const string& = "",
				       const string& = "",
				       const string& = "",
				       const string& = "" ) ;

		field* get_field_address( uint,
					  uint ) ;

		void   update_scroll_inds( errblock&,
					   field*,
					   bool = true ) ;

		uint   max_scroll_lcol( field* ) ;

		void   update_scroll_lcol( errblock&,
					   field* ) ;

		void   update_scroll_rcol( errblock&,
					   field*,
					   bool = true ) ;

		void   update_lenvar( errblock&,
				      field* ) ;

		void   set_tblen() ;

		void   tb_set_csrrow( int i )   { tb_csrrow  = i    ; }
		int    tb_get_csrrow()          { return tb_csrrow  ; }
		void   tb_set_autosel( bool b ) { tb_autosel = b    ; }
		void   tb_set_crp( int i )      { tb_crp     = i    ; }
		void   tb_set_linesChanged( errblock& ) ;
		void   tb_add_autosel_line( errblock& ) ;
		bool   tb_get_lineChanged( errblock&,
					   int&,
					   string& ) ;
		bool   tb_linesPending() ;
		void   tb_clear_linesChanged( errblock& )  ;
		void   tb_remove_lineChanged() ;

		void    get_ztdvars( int&,
				     int&,
				     int& ) ;

		void    set_ztdvars( int,
				     int,
				     int ) ;

		bool  get_nretriev()               { return nretriev  ; }
		const string& get_nretfield()      { return nretfield ; }
		const string& get_history_fields() { return dHist ; }

		void   set_panel_msg( const slmsg&, const string& ) ;
		void   clear_msg() ;
		void   clear_msgloc()   { msgloc = ""     ; }
		bool   inputInhibited() { return ( pdActive || msgResp ) ; }
		bool   msgInhibited()   { return msgResp  ; }
		void   msgResponseOK()  { msgResp = false ; }

		void   set_message_cond( const string& ) ;
		void   set_message_cond( errblock&,
					 const string&,
					 const string& ) ;
		void   set_message_cond( errblock&,
					 const string&,
					 const string&,
					 const string& ) ;
		void   set_message_cond( errblock&,
					 const string&,
					 const string&,
					 const string&,
					 const string& ) ;
		void   set_cursor_cond( const string&,
					int = -1,
					int =  1 ) ;

		bool   is_history_field( const string& ) ;

		void   set_zzstr_shared( errblock&,
					 const string& ) ;
		void   set_zzstr_shared( errblock&,
					 const string&,
					 const string& ) ;

		void   reset_attrs() ;
		void   reset_attrs_once() ;

		void   set_panel_fscreen() ;
		void   unset_panel_fscreen() ;

		void   unset_panel_decolourised() ;
		bool   is_panel_decolourised() ;

		void   set_panel_frame_act()  ;
		bool   is_panel_frame_inact() ;

		bool   is_inbox( PANEL* ) ;

		void   set_panel_ttl( const string& ) ;
		const string get_panel_ttl( PANEL* ) ;

		void   redraw_panel( errblock& ) ;

		void   draw_frame( PANEL*, uint ) ;
		void   draw_frame( errblock& ) ;

		void   draw_msgframes( uint ) ;
		void   draw_msgframes() ;

		string getDialogueVar( errblock&,
				       const string&,
				       field* = nullptr );
		void   putDialogueVar( errblock&,
				       const string&,
				       const string& ) ;
		string getControlVar( errblock&,
				      const string& ) ;
		void   setControlVar( errblock&,
				      const string&,
				      const string&,
				      PS_SECT,
				      int = 0 ) ;
		string getVariable( errblock&,
				    const vparm& ) ;

		string getVariable( errblock&,
				    const string& ) ;

		void   create_tbfield( errblock&,
				       const string&,
				       uint&,
				       uint&,
				       uint&,
				       bool& ) ;

		void   create_tbtext( errblock&,
				      const string&,
				      uint&,
				      uint&,
				      uint&,
				      bool& ) ;

		void   create_pdc( errblock&,
				   const string&,
				   uint,
				   const string& ) ;

		void   display_boxes() ;

		void   display_ztdmark( errblock& ) ;
		void   display_row_indicator( errblock& ) ;
		void   display_row_indicator( errblock&,
					      const string& ) ;
		void   set_tb_fields_act_inact( errblock& ) ;

		uint   get_tbred_lines() ;
		uint   get_areared_lines() ;
		uint   get_areared_lines( dynArea* ) ;
		uint   get_areared_lines( Area* ) ;
		uint   get_field_length( const string& ) ;
		uint   get_display_field_length( const string& ) ;
		void   put_field_value( const string&,
					const string& ) ;

		void   put_field_value( const string&,
					uint ) ;

		void   decolourise( WINDOW*,
				    uint,
				    uint,
				    uint ) ;

		int    get_value( errblock&,
				  string,
				  const string&,
				  string&,
				  int = 0 ) ;

		string sub_vars( string ) ;
		string sub_vars( string,
				 bool& ) ;

		void   add_field( const string&,
				  field* ) ;

		bool   tb_field( const string& ) ;

		bool   cursor_in_window( uint,
					 uint ) ;

		bool   cursor_not_in_window( uint,
					     uint ) ;

		void   make_field_visible( errblock&,
					   field* ) ;

		void   update_tbfields( errblock& ) ;

		bool check_stdtime( const string& ) ;
		bool check_stddate( const string& ) ;

		vector<string>attrList ;
		map<int, string> tb_linesChanged ;

		map<string, string> modvars ;
		map<string, int> winvars ;

		vector<string> klist;
		vector<text*> textList ;
		vector<text*> tbtextList ;
		vector<text*> text_pas ;
		vector<text*> text_rp ;
		vector<abc*> ab ;
		vector<Box*> boxes ;
		vector<tbfield*> tbfields ;
		map<string, field*> fieldList ;
		map<field*, string> addr2field ;
		map<string, field*> jumpList ;
		map<string, Area*> AreaList ;
		map<uint, Area*> AreaNum ;
		map<string, dynArea*> dynAreaList ;
		map<string, pnts> pntsTable ;
		map<string, help*> field_help ;
		map<int, string> keylistl ;
		map<int, string> keyattrl ;
		map<int, string> keylabll ;

		map<PREXX*, vector<string>> rexx_inlines ;

		map<string, CV_CONTROL> control_vars ;
		map<string, fieldXct*> field_xct_table ;
		map<field*, pair<string, sfield*>> sfields ;
		map<string, pair<string, uint>> tblenvars ;
		map<string, pair<string, uint>>& tb_lenvars() { return tblenvars ; }
		vector<sfield*> sfieldsList ;
		set<field*> histories ;

		multimap<string, field*> lcolmmap ;

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

		map<unsigned char, char_attrs*> char_attrlist ;
		map<unsigned char, unsigned int> ddata_map ;
		map<unsigned char, unsigned int> schar_map ;

		void   display_text() ;
		void   display_tbtext() ;
		void   display_ab( errblock& ) ;
		void   display_fields( errblock&,
				       bool = false ) ;

		void   process_panel_stmnts( errblock&,
					     int,
					     vector<panstmnt*>&,
					     PS_SECT ) ;

		void   process_panel_assignment( errblock&,
						 int,
						 ASSGN*,
						 set<string>&,
						 set<string>&,
						 PS_SECT ) ;

		void   process_panel_attr( errblock& err,
					   int ln,
					   const string& rhs,
					   ASSGN* assgn,
					   set<string>& attr,
					   PS_SECT ) ;

		void   process_panel_attrchar( errblock& err,
					       const string& rhs,
					       ASSGN* assgn,
					       set<string>& attrchar,
					       PS_SECT ) ;

		void   process_panel_vput( errblock& err,
					   VPUTGET* vputget ) ;

		void   process_panel_vget( errblock& err,
					   VPUTGET* vputget ) ;

		void   process_panel_vedit( errblock&,
					    int,
					    VEDIT*,
					    PS_SECT ) ;

		void   process_panel_prexx( errblock&,
					    PREXX* ) ;

		void   process_panel_panexit( errblock&,
					      PANEXIT*,
					      PS_SECT ) ;

		void   process_panel_panexit_rexx( errblock&,
						   PANEXIT*,
						   PS_SECT ) ;

		void   process_panel_panexit_load( errblock&,
						   PANEXIT*,
						   PS_SECT ) ;

		void   process_panel_verify( errblock&,
					     int,
					     VERIFY* ) ;

		void   process_panel_if( errblock&,
					 int,
					 IFSTMNT* ) ;

		void   process_panel_if_cond( errblock& err,
					      int ln,
					      IFSTMNT* ifstmnt ) ;

		string process_panel_trunc( errblock& err,
					    TRUNC* trunc ) ;

		string process_panel_trans( errblock& err,
					    int ln,
					    TRANS* trans,
					    const string& ) ;

		void process_panel_function( errblock& err,
					     PN_FUNCTION,
					     const string&,
					     string& ) ;

		void get_msgwin( errblock&,
				 string,
				 uint&,
				 uint&,
				 uint&,
				 uint&,
				 vector<string>& ) ;

		void panel_cleanup( PANEL* ) ;

		void update_keylist_vars( errblock& ) ;
		void update_zprim_var( errblock& ) ;

		void getlist( errblock& err,
			      string& s,
			      vector<string>& ) ;


		set<string> bodykws = { "ACTIONBAR",
					"AREA",
					"BOX",
					"DYNAREA",
					"FIELD",
					"LITERAL",
					"PANELDESC",
					"PANELTITLE",
					"TBFIELD",
					"TBTEXT",
					"TBMODEL",
					"TEXT" } ;

		string modname() { return "PANEL" ; }

#ifdef HAS_REXX_SUPPORT
		void start_rexx_interpreter( errblock& ) ;

		RexxInstance* instance   ;
		RexxThreadContext* threadContext ;
		RexxArrayObject args     ;
		RexxCondition condition  ;
		RexxDirectoryObject cond ;

		RexxOption options[ 4 ] ;
		RexxContextExit exits[ 4 ] ;

		friend int REXXENTRY rxterExit_panel1( RexxExitContext*,
						       int,
						       int,
						       PEXIT ) ;

		friend int REXXENTRY rxiniExit_panel1( RexxExitContext*,
						       int,
						       int,
						       PEXIT ) ;
#endif

		friend class pApplication ;

		friend class pLScreen ;

		friend void processAction( selobj&,
					   uint,
					   uint,
					   int,
					   bool&,
					   bool&,
					   bool&,
					   bool&,
					   bool& ) ;
} ;


class panel_data_ext
{
	public:
		explicit panel_data_ext()
		{
			winttl       = "" ;
			ppanel       = nullptr ;
			fscreen      = false ;
			inbox        = false ;
			frame_inact  = false ;
			decolourised = false ;
		}

	private:
		string winttl ;

		pPanel* ppanel ;

		bool fscreen ;
		bool inbox ;
		bool frame_inact ;
		bool decolourised ;

		friend class panel_data ;
} ;


class panel_data
{
	//
	// Make copy constructor and operator= private so this object cannot be copied
	// as it contains dynamic memory/resource allocations.
	//

	public:
		explicit panel_data( uint x,
				     bool b ) : panel_data( x )
		{
			data_ext->inbox = b ;
		}

		explicit panel_data( uint x,
				     bool b,
				     pPanel* p ) : panel_data( x )
		{
			frame            = true ;
			data_ext->ppanel = p ;
		}

		explicit panel_data( uint x,
				     pPanel* p,
				     bool f = false ) : panel_data( x )
		{
			data_ext->ppanel  = p ;
			data_ext->fscreen = f ;
		}

		explicit panel_data( uint x ) : panel_data()
		{
			screenId = x ;
		}

		explicit panel_data()
		{
			frame    = false ;
			data_ext = new panel_data_ext ;
			data_ext->fscreen = false ;
		}

		~panel_data()
		{
			delete data_ext ;
		}

		void set_fscreen() const
		{
			data_ext->fscreen = true ;
		}

		void update( pPanel* p ) const
		{
			data_ext->ppanel = p ;
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

		bool is_not_frame() const
		{
			return !frame ;
		}

		void set_frame_inact() const
		{
			data_ext->frame_inact = true ;
		}

		void set_frame_act() const
		{
			data_ext->frame_inact = false ;
		}

		bool is_frame_inact() const
		{
			return data_ext->frame_inact ;
		}

		bool is_frame_act() const
		{
			return !data_ext->frame_inact ;
		}

		bool is_inbox() const
		{
			return data_ext->inbox ;
		}

		void set_winttl( const string& ttl ) const
		{
			data_ext->winttl = ttl ;
		}

		const string& get_winttl() const
		{
			return data_ext->winttl ;
		}

		pPanel* ppanel() const
		{
			return data_ext->ppanel ;
		}

		uint screenid() const
		{
			return screenId ;
		}


	private:
		panel_data( const panel_data& pd )
		{
		      return ;
		}

		panel_data operator = ( const panel_data& rhs ) const
		{
		      return *this ;
		}

		bool frame ;
		uint screenId ;
		panel_data_ext* data_ext ;
} ;
