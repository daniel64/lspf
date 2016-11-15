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
		bool   pdActive ;
		bool   showLMSG ;

		pPanel()  ;
		~pPanel() ;

		void   init( int & RC ) ;

		void   field_edit( uint row, uint col, char ch, bool Istr, bool & prot ) ;
		void   field_delete_char( uint row, uint col, bool & prot )   ;
		void   field_backspace( uint & row, uint & col, bool & prot ) ;
		void   field_erase_eof( uint row, uint col, bool & prot )     ;
		void   field_tab_down( uint & row, uint & col ) ;
		void   field_tab_next( uint & row, uint & col ) ;
		void   field_clear( string field )    ;
		string field_getvalue( string field ) ;
		bool   field_valid( string field ) ;
		fieldExc field_getexec( string )   ;
		void   field_setvalue( string field, string value ) ;
		string field_getname( uint row, uint col ) ;
		bool   field_get_row_col( string fld, uint & row, uint & col ) ;

		void   cursor_to_field( int & RC, string = "", int = 1 ) ;
		void   cursor_to_next_field ( string name, uint & row, uint & col )  ;
		void   cursor_eof( uint & row, uint & col )  ;
		void   get_cursor( uint & row, uint & col )     { row   = p_row + win_row ; col   = p_col + win_col ; } ;
		void   set_cursor( uint row, uint col )         { p_row = row - win_row   ; p_col = col - win_col   ; } ;

		void   display_panel( int & RC ) ;
		int    get_abline()      { return win_row  ; }
		bool   is_pd_displayed() { return pdActive ; }
		bool   display_pd( uint col ) ;
		void   hide_pd()          ;
		void   display_first_pd() ;
		void   display_next_pd()  ;
		void   display_msg()      ;
		pdc    retrieve_pdChoice( int row, int col ) ;

		void   redisplay_panel() ;
		void   redraw_fields()   ;
		void   refresh() ;

		string cmd_getvalue() ;
		void   cmd_setvalue( string value )    ;
		bool   is_cmd_inactive( string value ) ;

		bool   on_border_line( uint, uint )  ;
		bool   hide_msg_window( uint, uint ) ;

		string get_keylist( int ) ;

	private:
		int    RC          ;
		string Home        ;
		string PANELID     ;
		string CURFLD      ;
		int    CURPOS      ;
		slmsg  MSG         ;
		string PERR        ;
		string KEYLISTN    ;
		string KEYAPPL     ;
		bool   REXX        ;
		string ZPHELP      ;
		string tb_fields   ;
		int    tb_depth    ;
		int    abc_pos     ;
		bool   primaryMenu ;
		bool   scrollOn    ;
		string panelTitle  ;
		string panelDescr  ;
		int    abIndex     ;
		int    tb_row      ;
		bool   msgResp     ;
		bool   nretriev    ;
		bool   LRScroll    ;
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
		bool   smp_created ;
		bool   lmp_created ;
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

		int    loadPanel( string p_name, string paths ) ;

		void   display_panel_update( int & RC ) ;
		void   display_panel_init( int & RC )   ;
		void   display_panel_reinit( int & RC, int ln ) ;
		void   display_panel_proc( int & RC, int ln )   ;
		void   display_id()  ;

		void   set_popup( int, int ) ;
		void   remove_popup()        ;
		void   move_popup()          ;

		void   put_keylist( int, string ) ;
		string get_panelDescr()  { return panelDescr != "" ? sub_vars( panelDescr ) : sub_vars( panelTitle ) ; }

		void   update_field_values( int & RC ) ;

		void   get_panel_info( int & RC, string a_name, string t = "", string w = "", string d = "", string r = "", string c = "" ) ;
		void   attr( int & RC, string field, string attrs ) ;
		void   get_home( uint & row, uint & col ) ;

		void   set_tb_linesChanged() ;
		bool   tb_lineChanged( int &, string & ) ;
		void   clear_tb_linesChanged( int & ) ;
		void   remove_tb_lineChanged()        ;

		string get_field_help( string fld ) ;
		bool   get_nretriev()  { return nretriev  ; }
		string get_nretfield() { return nretfield ; }

		void   set_panel_msg( slmsg, string ) ;
		void   clear_msg() ;
		void   clear_msg_loc()  { MSGLOC = ""     ; }
		bool   inputInhibited() { return ( pdActive || msgResp ) ; }
		bool   msgInhibited()   { return msgResp  ; }
		void   msgResponseOK()  { msgResp = false ; }

		string return_command( string ) ;
		void   resetAttrs()             ;

		void   syncDialogueVar( string ) ;
		string getDialogueVar( string )  ;
		void   putDialogueVar( string, string ) ;

		void   create_tbfield( int col, int size, cuaType cuaFT, string name, string opts ) ;
		void   create_pdc( string, string, string, string, string ) ;

		void   display_boxes() ;

		void   display_tb_mark_posn() ;
		void   tb_fields_active_inactive() ;
		string sub_vars( string s ) ;

		vector<string>attrList ;
		map<int, string> tb_linesChanged ;

		vector<literal *> literalList      ;
		vector<abc > ab                    ;
		vector<Box *> boxes                ;
		map<string, field   *> fieldList   ;
		map<string, dynArea *> dynAreaList ;
		map<string, string > commandTable  ;
		map<string, pnts > pntsTable       ;
		map<string, string> fieldHList     ;
		map<int, string> Keylistl          ;

		map<string, fieldExc> fieldExcTable ;

		vector<panstmnt> procstmnts ;
		vector<panstmnt> initstmnts ;
		vector<panstmnt> reinstmnts ;

		vector<IFSTMNT>  ifList ;
		vector<VERIFY>  verList ;
		vector<VPUTGET> vpgList ;
		vector<TRUNC> truncList ;
		vector<TRANS> transList ;
		vector<ASSGN> assgnList ;

		vector<VPUTGET> vpgListi ;
		vector<ASSGN> assgnListi ;

		vector<VPUTGET> vpgListr ;
		vector<ASSGN> assgnListr ;

		void   display_literals() ;
		void   display_fields()   ;
		void   display_ab()       ;
		void   display_pd()       ;

		void   get_msgwin( int, int &, int &, int &, int & ) ;
		void   panel_cleanup( PANEL * ) ;

		void   update_keylist_vars() ;

		friend class pApplication ;
} ;
