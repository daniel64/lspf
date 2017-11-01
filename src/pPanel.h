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
		string MSGID    ;
		string CMDfield ;
		bool   showLMSG ;

		pPanel()  ;
		~pPanel() ;

		void   init( errblock& err ) ;

		void   field_edit( uint row, uint col, char ch, bool Istr, bool& prot ) ;
		void   field_delete_char( uint row, uint col, bool& prot ) ;
		void   field_backspace( uint& row, uint& col, bool& prot ) ;
		void   field_erase_eof( uint row, uint col, bool& prot )   ;
		void   field_tab_down( uint& row, uint& col ) ;
		void   field_tab_next( uint& row, uint& col ) ;
		void   field_clear( const string& field )    ;
		string field_getvalue( const string& field ) ;
		bool   field_valid( const string& field ) ;
		fieldExc field_getexec( const string& )   ;
		void   field_setvalue( const string& field, const string& value ) ;
		string field_getname( uint row, uint col ) ;
		bool   field_get_row_col( const string& fld, uint& row, uint& col ) ;

		void   cursor_to_field( int& RC, string ="", int =1 ) ;
		void   cursor_to_next_field ( const string& name, uint& row, uint& col )  ;
		void   cursor_eof( uint& row, uint& col )  ;
		void   get_cursor( uint& row, uint& col ) { row   = p_row + win_row ; col   = p_col + win_col ; }
		void   set_cursor( uint row, uint col )   { p_row = row - win_row   ; p_col = col - win_col   ; }

		int    get_abline()      { if ( ab.size() > 0 ) return win_row ; else return -1 ; }
		bool   pd_Active()       { return pdActive ; }
		bool   display_pd( uint col ) ;
		void   display_pd()       ;
		void   hide_pd()          ;
		void   remove_pd()        ;
		void   display_next_pd()  ;
		void   display_msg()      ;
		pdc    retrieve_pdChoice( int row, int col ) ;
		void   toggle_fscreen()   { ff_screen = !ff_screen ; }

		void   display_panel( errblock& ) ;
		void   redisplay_panel() ;
		void   redraw_fields()   ;
		void   refresh()         ;
		void   refresh_fields()  ;
		void   refresh_fields( const string& ) ;

		string cmd_getvalue()                  { return field_getvalue( CMDfield ) ; }
		void   cmd_setvalue( const string& v ) { field_setvalue( CMDfield, v )     ; }

		bool    get_tbscan()                   { return tb_scan  ; }
		string& get_tb_clear()                 { return tb_clear ; }

		bool   is_cmd_inactive( const string& value ) ;

		bool   on_border_line( uint, uint )  ;
		bool   hide_msg_window( uint, uint ) ;
		void   selPanel( bool x ) { selectPanel = x ; }

		string get_keylist( int ) ;

	private:
		int    RC          ;
		string Home        ;
		string PANELID     ;
		bool   ALARM       ;
		string CURFLD      ;
		int    CURPOS      ;
		slmsg  MSG         ;
		string KEYLISTN    ;
		string KEYAPPL     ;
		string KEYHELPN    ;
		bool   REXX        ;
		string ZPHELP      ;
		string tb_fields   ;
		string tb_clear    ;
		int    tb_depth    ;
		bool   tb_scan     ;
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
		string MSGLOC      ;
		bool   tb_model    ;
		bool   win_addpop  ;
		bool   end_pressed ;
		bool   message_set ;
		bool   cursor_set  ;
		bool   ff_screen   ;
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

		poolMGR * p_poolMGR  ;
		fPOOL   * p_funcPOOL ;

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

		void   display_id() ;

		void   set_popup( int, int ) ;
		void   remove_popup()        ;
		void   move_popup()          ;

		void   put_keylist( int, const string& ) ;
		string get_panelDescr()  { return panelDescr != "" ? sub_vars( panelDescr ) : sub_vars( panelTitle ) ; }

		void   update_field_values( errblock& ) ;

		void   get_panel_info( int& RC, const string& a_name, const string& t = "", const string& w = "", const string& d = "", const string& r = "", const string& c = "" ) ;
		void   attr( int& RC, const string& field, const string& attrs ) ;
		void   get_home( uint& row, uint& col ) ;

		void   tb_set_linesChanged() ;
		bool   tb_lineChanged( int&, string& ) ;
		void   tb_clear_linesChanged( errblock& ) ;
		void   tb_remove_lineChanged() ;

		string  get_field_help( uint row, uint col ) ;
		bool    get_nretriev()  { return nretriev  ; }
		string& get_nretfield() { return nretfield ; }

		void   set_panel_msg( slmsg, const string& ) ;
		void   clear_msg() ;
		void   clear_msg_loc()  { MSGLOC = ""     ; }
		bool   inputInhibited() { return ( pdActive || msgResp ) ; }
		bool   msgInhibited()   { return msgResp  ; }
		void   msgResponseOK()  { msgResp = false ; }

		void   setMessageCond( const string& ) ;
		void   setCursorCond( const string& )  ;

		string return_command( const string& ) ;
		void   resetAttrs() ;

		void   syncDialogueVar( const string& ) ;
		string getDialogueVar( errblock&, const string& )   ;
		void   putDialogueVar( errblock&, const string&, const string& ) ;
		string getControlVar( errblock&, const string& ) ;
		void   setControlVar( errblock&, int, const string&, const string& ) ;

		void   create_tbfield( errblock&, int col, int size, cuaType cuaFT, const string& name, const string& opts ) ;
		void   create_tbfield( errblock&, const string& ) ;
		void   create_pdc( errblock&, const string& ) ;

		void   display_boxes()  ;

		void   display_tb_mark_posn() ;
		void   tb_fields_active_inactive() ;
		string sub_vars( string s ) ;

		vector<string>attrList ;
		map<int, string> tb_linesChanged ;

		vector<literal *> literalList      ;
		vector<abc> ab                     ;
		vector<Box *> boxes                ;
		map<string, field *> fieldList     ;
		map<string, dynArea *> dynAreaList ;
		map<string, string> commandTable   ;
		map<string, pnts> pntsTable        ;
		map<string, string> fieldHList     ;
		map<int, string> Keylistl          ;

		map<string, fieldExc> fieldExcTable ;

		vector<panstmnt* > procstmnts ;
		vector<panstmnt* > initstmnts ;
		vector<panstmnt* > reinstmnts ;

		void   display_literals() ;
		void   display_ab()       ;
		void   display_fields()   ;

		void  process_panel_stmnts( errblock& err, int ln, vector<panstmnt* >& stmnts ) ;
		void  process_panel_assignment( errblock& err, int ln, ASSGN* assgn ) ;
		void  process_panel_vputget( errblock& err, VPUTGET* vputget ) ;
		void  process_panel_verify( errblock& err, int ln, VERIFY* verify ) ;
		void  process_panel_if( errblock& err, int ln, IFSTMNT* ifstmnt ) ;
		void  process_panel_if_cond( errblock& err, int ln, IFSTMNT* ifstmnt ) ;
		string process_panel_trunc( errblock& err, TRUNC* trunc ) ;
		string process_panel_trans( errblock& err, int ln, TRANS* trans )  ;

		void   get_msgwin( string, int&, int&, int&, int&, vector<string>& ) ;
		void   panel_cleanup( PANEL * ) ;

		void   update_keylist_vars() ;

		friend class pApplication ;
} ;
