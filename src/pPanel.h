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
		string msgid    ;
		string cmdfield ;
		bool   showLMSG ;

		static poolMGR * p_poolMGR ;
		static logger  * lg        ;

		pPanel()  ;
		~pPanel() ;

		void   init( errblock& err ) ;

		void   field_edit( errblock&, uint row, uint col, char ch, bool Istr, bool& prot ) ;
		void   field_delete_char( errblock&, uint row, uint col, bool& prot ) ;
		void   field_backspace( errblock&, uint& row, uint& col, bool& prot ) ;
		void   field_erase_eof( errblock&, uint row, uint col, bool& prot )   ;
		void   field_tab_down( uint& row, uint& col ) ;
		void   field_tab_next( uint& row, uint& col ) ;
		string& field_getvalue( const string& field ) ;
		bool   field_valid( const string& field ) ;
		fieldExc field_getexec( const string& )   ;
		void   field_setvalue( errblock&, const string& field, const string& value ) ;
		string field_getname( uint row, uint col ) ;
		bool   field_get_row_col( const string& fld, uint& row, uint& col ) ;
		void   field_get_col( const string& fld, uint& col ) ;

		void   cursor_to_field( int& RC, string ="", int =1 ) ;
		void   cursor_to_cmdfield( int& RC, int =1 ) ;
		void   cursor_to_next_field ( const string& name, uint& row, uint& col )  ;
		void   cursor_eof( uint& row, uint& col )  ;
		void   get_cursor( uint& row, uint& col ) { row   = p_row + win_row ; col   = p_col + win_col ; }
		void   set_cursor( uint row, uint col )   { p_row = row - win_row   ; p_col = col - win_col   ; }

		int    get_abline()      { if ( ab.size() > 0 ) return win_row ; else return -1 ; }
		bool   pd_active()       { return pdActive ; }
		void   get_pd_home( uint& row, uint& col ) ;
		bool   display_pd( errblock&, uint row, uint col, string& ) ;
		void   display_pd( errblock& ) ;
		void   display_current_pd( errblock&, uint row, uint col ) ;
		void   hide_popup()       ;
		void   hide_pd()          ;
		void   remove_pd()        ;
		bool   cursor_on_pulldown( uint, uint ) ;
		void   display_next_pd( errblock&, string& ) ;
		void   display_msg( errblock& ) ;
		pdc    retrieve_choice( errblock&, string& ) ;
		void   toggle_fscreen( bool, int, int ) ;

		void   display_panel( errblock& ) ;
		void   redraw_fields( errblock& ) ;
		void   refresh()         ;
		void   refresh_fields( errblock& ) ;
		void   refresh_fields( errblock&, const string& ) ;

		string& cmd_getvalue()                  ;
		void    cmd_setvalue( errblock&, const string& ) ;

		bool    get_tbscan()                   { return tb_scan  ; }
		string& get_tb_clear()                 { return tb_clear ; }

		bool   is_cmd_inactive( const string& value ) ;

		bool   has_command_field()             { return fieldList.count( cmdfield ) > 0 ; }
		bool   on_border_line( uint, uint )  ;
		bool   hide_msg_window( uint, uint ) ;
		void   selPanel( bool x ) { selectPanel = x ; }

		void   set_pfpressed( const string& s ) { pfkey = s    ; }
		string get_pfpressed()                  { return pfkey ; }

		string get_keylist( int ) ;

		const string& get_zprim() { return ZPRIM ; }

	private:
		string Home        ;
		string scroll      ;
		string panelid     ;
		bool   ALARM       ;
		string curfld      ;
		int    taskId      ;
		int    curidx      ;
		int    curpos      ;
		slmsg  MSG         ;
		string pfkey       ;
		string KEYLISTN    ;
		string KEYAPPL     ;
		string KEYHELPN    ;
		bool   REXX        ;
		string ZPHELP      ;
		string ZZCMD       ;
		string ZPRIM       ;
		string tb_fields   ;
		string tb_clear    ;
		int    tb_depth    ;
		int    tb_curidx   ;
		int    tb_csrrow   ;
		bool   tb_scan     ;
		bool   tb_autosel  ;
		bool   pdActive    ;
		int    abc_pos     ;
		bool   primaryMenu ;
		bool   selectPanel ;
		bool   scrollOn    ;
		string panelTitle  ;
		string panelDescr  ;
		int    abIndex     ;
		int    tb_row      ;
		int    tb_lcol     ;
		int    tb_lsz      ;
		bool   msgResp     ;
		bool   nretriev    ;
		bool   LRScroll    ;
		bool   forEdit     ;
		bool   forBrowse   ;
		string nretfield   ;
		int    opt_field   ;
		int    dyn_depth   ;
		int    dyn_width   ;
		int    ZSCRNUM     ;
		int    ZSCRMAXD    ;
		int    ZSCRMAXW    ;
		int    WSCRMAXD    ;
		int    WSCRMAXW    ;
		uint   p_row       ;
		uint   p_col       ;
		string msgloc      ;
		bool   tb_model    ;
		bool   win_addpop  ;
		bool   end_pressed ;
		bool   message_set ;
		bool   cursor_set  ;
		bool   full_screen ;
		bool   msg_and_cmd ;
		int    win_width   ;
		int    win_depth   ;
		int    win_row     ;
		int    win_col     ;
		WINDOW * win       ;
		WINDOW * fwin      ;
		WINDOW * pwin      ;
		WINDOW * bwin      ;
		WINDOW * smwin     ;
		WINDOW * lmwin     ;
		WINDOW * idwin     ;
		PANEL  * panel     ;
		PANEL  * bpanel    ;
		PANEL  * smpanel   ;
		PANEL  * lmpanel   ;
		PANEL  * idpanel   ;

		fPOOL  * p_funcPOOL ;

		void   loadPanel( errblock&, const string&, const string& ) ;
		void   readPanel( errblock&, vector<string>&, const string&, const string&, string ) ;

		void   createPanel_Refresh( errblock&, parser&, panstmnt* ) ;
		void   createPanel_Vputget( errblock&, parser&, panstmnt* ) ;
		void   createPanel_Assign( errblock&, parser&, panstmnt* ) ;
		void   createPanel_Verify( errblock&, parser&, panstmnt* ) ;
		void   createPanel_If( errblock&, parser&, panstmnt*, bool ) ;
		void   createPanel_Else( errblock&, parser&, panstmnt*, vector<panstmnt* >*, bool )  ;

		void   display_panel_update( errblock& ) ;
		void   display_panel_init( errblock& )   ;
		void   display_panel_reinit( errblock&, int ln=0 ) ;
		void   display_panel_proc( errblock&, int ln )     ;

		void   abc_panel_init( errblock&, const string& ) ;
		void   abc_panel_proc( errblock&, const string& ) ;

		void   display_id( errblock& ) ;

		void   set_cursor( const string&, int ) ;
		string get_cursor() ;
		string get_msgloc() ;

		void   set_cursor_idx( int i ) { curidx = i ; }
		void   set_cursor_home() ;

		void   set_popup( int, int ) ;
		void   remove_popup() ;
		void   move_popup()   ;
		void   show_popup()   ;

		void   put_keylist( int, const string& ) ;
		string get_panelDescr()  { return panelDescr != "" ? sub_vars( panelDescr ) : sub_vars( panelTitle ) ; }

		void   update_field_values( errblock& ) ;

		void   get_panel_info( int& RC, const string& a_name, const string& t = "", const string& w = "", const string& d = "", const string& r = "", const string& c = "" ) ;
		void   attr( int& RC, const string& field, const string& attrs ) ;
		void   get_home( uint& row, uint& col ) ;

		void   tb_set_csrrow( int i )   { tb_csrrow  = i    ; }
		int    tb_get_csrrow()          { return tb_csrrow  ; }
		void   tb_set_autosel( bool b ) { tb_autosel = b    ; }
		bool   tb_is_autosel()          { return tb_autosel ; }
		void   tb_set_linesChanged( errblock&, string& ) ;
		bool   tb_get_lineChanged( errblock&, int&, string& ) ;
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
		void   set_cursor_cond( const string&, int =0 ) ;

		void   resetAttrs() ;

		void   syncDialogueVar( errblock&, const string& ) ;
		string getDialogueVar( errblock&, const string& )   ;
		void   putDialogueVar( errblock&, const string&, const string& ) ;
		string getControlVar( errblock&, const string& ) ;
		void   setControlVar( errblock&, int, const string&, const string& ) ;

		void   create_tbfield( errblock&, int col, int size, cuaType cuaFT, const string& name, const string& opts ) ;
		void   create_tbfield( errblock&, const string& ) ;
		void   create_pdc( errblock&, const string&, const string& ) ;

		void   display_boxes()  ;

		void   display_tb_mark_posn( errblock& )    ;
		void   set_tb_fields_act_inact( errblock& ) ;

		string sub_vars( string s ) ;
		string sub_vars( string s, bool& ) ;

		vector<string>attrList ;
		map<int, string> tb_linesChanged ;

		vector<literal *> literalList      ;
		vector<literal *> literalPS        ;
		vector<abc> ab                     ;
		vector<Box *> boxes                ;
		map<string, field *> fieldList     ;
		map<string, dynArea *> dynAreaList ;
		map<string, pnts> pntsTable        ;
		map<string, string> fieldHList     ;
		map<int, string> Keylistl          ;

		map<string, fieldExc> fieldExcTable ;

		map<string, vector<panstmnt*>> abc_initstmnts ;
		map<string, vector<panstmnt*>> abc_procstmnts ;

		vector<panstmnt* > procstmnts ;
		vector<panstmnt* > initstmnts ;
		vector<panstmnt* > reinstmnts ;

		void   display_literals() ;
		void   display_ab()       ;
		void   display_fields( errblock& ) ;

		void   process_panel_stmnts( errblock& err, int ln, vector<panstmnt* >& stmnts ) ;
		void   process_panel_assignment( errblock& err, int ln, ASSGN* assgn ) ;
		void   process_panel_vputget( errblock& err, VPUTGET* vputget ) ;
		void   process_panel_verify( errblock& err, int ln, VERIFY* verify ) ;
		void   process_panel_if( errblock& err, int ln, IFSTMNT* ifstmnt ) ;
		void   process_panel_if_cond( errblock& err, int ln, IFSTMNT* ifstmnt ) ;
		string process_panel_trunc( errblock& err, TRUNC* trunc ) ;
		string process_panel_trans( errblock& err, int ln, TRANS* trans, const string& )  ;

		bool   msg_issued_with_cmd()    { return msg_and_cmd ; }

		void   get_msgwin( string, int&, int&, int&, int&, vector<string>& ) ;
		void   panel_cleanup( PANEL * ) ;

		void   update_keylist_vars( errblock& ) ;

		friend class pApplication ;
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
