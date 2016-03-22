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
		string PANELID     ;
		string Home        ;
		string CMDfield    ;
		string MSGID       ;
		string CURFLD      ;
		int    CURPOS      ;
		string SMSG        ;
		string LMSG        ;
		string PERR        ;
		string KEYLISTN    ;
		string KEYAPPL     ;
		cuaType MSGTYPE    ;
		bool   MSGALRM     ;
		bool   REXX        ;
		string ZPHELP      ;
		string tb_fields   ;
		int    tb_depth    ;
		int    abc_pos     ;
		bool   abActive    ;
		bool   showLMSG    ;
		bool   primaryMenu ;
		bool   scrollOn    ;

		pPanel()  ;
		~pPanel() ;

		poolMGR * p_poolMGR  ;
		fPOOL   * p_funcPOOL ;

		void   init( int & RC ) ;

		int    loadPanel( string p_name, string paths ) ;

		map<int, string > tb_linesChanged ;

		void   display_panel( int & RC )  ;
		void   display_panel_update( int & RC ) ;
		void   display_panel_init( int & RC )   ;
		void   display_panel_reinit( int & RC, int ln ) ;
		void   display_panel_proc( int & RC, int ln )   ;
		void   display_msg() ;

		void   set_popup( int, int ) ;
		void   remove_popup()        ;

		void   put_keylist( int, string ) ;
		string get_keylist( int )         ;

		void   update_field_values( int & RC ) ;

		int    get_abline()      { return win_row ; } ;
		bool   display_pd( uint row, uint col ) ;
		void   hide_pd()         ;
		bool   is_pd_displayed() ;
		void   display_pd_next() ;

		void   clear() ;
		void   refresh( int & RC ) ;
		void   nrefresh()          ;

		pdc    retrieve_pdChoice( int row, int col ) ;
		void   get_panel_info( int & RC, string a_name, string t = "", string w = "", string d = "", string r = "", string c = "" ) ;
		void   attr( int & RC, string field, string attrs ) ;
		void   get_home( uint & row, uint & col ) ;

		void   set_tb_linesChanged() ;
		bool   tb_lineChanged( int &, string & ) ;
		void   clear_tb_linesChanged( int & ) ;
		void   remove_tb_lineChanged( int & ) ;

		void   cursor_to_field( int & RC, string name="", int pos = 1 ) ;
		void   cursor_eof( uint & row, uint & col )  ;
		void   cursor_to_next_field ( string name, uint & row, uint & col )  ;
		void   get_cursor( uint & row, uint & col )     { row   = p_row + win_row ; col   = p_col + win_col ; } ;
		void   set_cursor( uint row, uint col )         { p_row = row - win_row   ; p_col = col - win_col   ; } ;

		void   field_edit( uint row, uint col, char ch, bool Istr, bool & prot ) ;
		void   field_delete_char( uint row, uint col, bool & prot )   ;
		void   field_backspace( uint & row, uint & col, bool & prot ) ;
		void   field_erase_eof( uint row, uint col, bool & prot )     ;
		void   field_tab_down( uint & row, uint & col ) ;
		void   field_tab_next( uint & row, uint & col ) ;
		void   field_clear( string field )    ;
		string field_getvalue( string field ) ;
		fieldExc field_getexec( string ) ;
		void   field_setvalue( string field, string value ) ;

		string cmd_getvalue() ;
		void   cmd_setvalue( string value ) ;

		string field_getname( uint row, uint col ) ;
		bool   field_get_row_col( string fld, uint & row, uint & col ) ;
		string get_field_help( string fld ) ;
		bool   get_nretriev()  { return nretriev  ; }
		string get_nretfield() { return nretfield ; }

		void   set_msg( string, string, cuaType, bool ) ;
		void   clear_msg() ;

		string return_command( string ) ;

		vector<string>attrList ;
		void   resetAttrs()    ;

	private:
		string PanelTitle  ;
		int    abIndex     ;
		int    RC          ;
		int    tb_row      ;
		bool   nretriev    ;
		string nretfield   ;
		int    opt_field   ;
		int    dyn_depth   ;
		int    dyn_width   ;
		int    ZSCRMAXD    ;
		int    ZSCRMAXW    ;
		int    WSCRMAXD    ;
		int    WSCRMAXW    ;
		uint   p_row       ;
		uint   p_col       ;
		bool   tb_model    ;
		bool   win_addpop  ;
		bool   win_created ;
		bool   pan_created ;
		int    win_width   ;
		int    win_depth   ;
		int    win_row     ;
		int    win_col     ;
		WINDOW * win       ;
		WINDOW * bwin      ;
		PANEL  * panel     ;
		PANEL  * bpanel    ;

		void   syncDialogueVar( string ) ;
		string getDialogueVar( string )  ;
		void   putDialogueVar( string, string ) ;

		void   create_tbfield( int col, int size, cuaType cuaFT, string name, string opts ) ;
		void   create_pdc( string, string, string, string, string ) ;

		void   display_boxes() ;

		void   display_tb_mark_posn() ;
		void   tb_fields_active_inactive() ;

		vector<literal *> literalList ;
		map<string, field   *> fieldList ;
		map<string, dynArea *> dynAreaList ;
		vector<abc > ab ;
		vector<Box *> boxes ;
		map<string, string > commandTable ;
		map<string, pnts > pntsTable      ;
		map<string, string> fieldHList    ;
		map<int, string> Keylistl         ;

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
} ;
