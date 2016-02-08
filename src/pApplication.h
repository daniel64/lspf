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

class pApplication
{
	public:
		pApplication()  ;
		~pApplication() ;

		virtual void application() = 0 ;

		static pApplication * currApplication ;

		int    taskID             ;
		string PANELID            ;
		string PPANELID           ;
		string MSGID              ;
		bool   ControlDisplayLock ;
		bool   ControlNonDispl    ;
		bool   ControlSplitEnable ;
		bool   ControlRefUpdate   ;
		bool   errPanelissued     ;
		bool   testMode           ;
		bool   propagateEnd       ;
		bool   jumpEntered        ;
		bool   noTimeOut          ;
		bool   busyAppl           ;
		bool   terminateAppl      ;
		bool   abnormalEnd        ;
		bool   abnormalEndForced  ;
		bool   reloadCUATables    ;
		bool   rawOutput          ;
		bool   libdef_muser       ;
		bool   libdef_puser       ;
		bool   libdef_tuser       ;
		string rexxName           ;
		bool   SEL                ;
		string SEL_PGM            ;
		string SEL_PARM           ;
		string SEL_NEWAPPL        ;
		bool   SEL_NEWPOOL        ;
		bool   SEL_PASSLIB        ;
		string field_name         ;
		bool   NEWPOOL            ;
		bool   PASSLIB            ;
		bool   setMSG             ;
		string shrdPool           ;
		int    RC                 ;
		vector<string>rmsgs       ;

		boost::posix_time::ptime resumeTime ;
		boost::thread            * pThread  ;

		string PARM   ;

		fPOOL      funcPOOL   ;
		poolMGR  * p_poolMGR  ;
		tableMGR * p_tableMGR ;

		pPanel * currPanel   ;
		pPanel * currtbPanel ;
		map<string,  pPanel *>  panelList ;
		stack<pPanel *> SRpanelStack      ;

		int    taskid()  { return taskID ; }
		void   init() ;
		void   info() ;
		void   refresh()  ;
		void   nrefresh() ;
		void   save_screen()    ;
		void   restore_screen() ;
		void   panel_create( string p_name ) ;
		bool   isRawOutput() { return rawOutput ; }

		string get_select_cmd( string ) ;
		string get_help_member( int, int ) ;
		string sub_vars( string ) ;

		void   control( string, string ) ;
		void   control( string, void (pApplication::*)() ) ;
		void   libdef( string, string = "" ) ;
		void   rdisplay( string ) ;
		void   display( string p_name, string p_msg = "", string p_cursor = "", int p_curpos = 0 ) ;
		void   pquery( string p_name, string a_name, string t = "", string w = "", string d = "", string r = "", string c = "" ) ;
		void   select( string )  ;
		void   select( string pgm, string parm, string newappl, bool newpool, bool passlib ) ;
		void   attr( string, string ) ;

		void   vcopy( string, string &, vcMODE=MOVE )   ;
		void   vcopy( string, string * &, vcMODE=LOCATE ) ;
		void   vdefine( string, string *, string * =NULL, string * =NULL, string * =NULL, string * =NULL, string * =NULL, string * =NULL, string * =NULL ) ;
		void   vdefine( string, int *, int * =NULL, int * =NULL, int * =NULL, int * =NULL, int * =NULL, int * =NULL, int * =NULL ) ;
		void   vdelete( string ) ;
		void   verase( string var, poolType =ASIS ) ;
		void   vget( string var, poolType =ASIS )   ;
		void   vput( string var, poolType =ASIS )   ;
		string vlist( poolType pType, int lvl )     ;
		string vilist()                             ;
		string vslist()                             ;
		void   vmask( string var, string type, string mask ) ;
		void   vreplace( string, string )           ;
		void   vreplace( string, int )              ;
		void   vreset() ;

		map<string,  bool> tablesOpen   ;
		map<string,  bool> tablesUpdate ;

		void   tbadd( string tb_name, string tb_namelst="", string tb_order="", int tb_num_of_rows=0 ) ;
		void   tbbottom( string tb_name, string tb_savenm="", string tb_rowid_vn="", string tb_noread="", string tb_crp_name="" ) ;
		void   tbclose( string tb_name, string new_name="", string path="" ) ;
		void   tbcreate( string tb_name, string keys, string names, tbSAVE a=NOWRITE, tbREP b=NOREPLACE, string path="", tbDISP c=EXCLUSIVE ) ;
		void   tbdelete( string tb_name ) ;
		void   tbdispl( string tb_name, string p_name="", string p_msg="", string p_cursor="", int p_csrrow=0, int p_csrpos=1, string p_autosel="YES", string p_crp_name="", string p_rowid_nm="" ) ;
		void   tbend( string tb_name ) ;
		void   tberase( string tb_name, string tb_path="" ) ;
		void   tbexist( string tb_name ) ;
		void   tbget( string tb_name, string tb_savenm="", string tb_rowid_vn="", string tb_noread="", string tb_crp_name="" ) ;
		void   tbmod( string tb_name, string tb_namelst="", string tb_order="" ) ;
		void   tbopen( string tb_name, tbSAVE WRITE, string path="", tbDISP c=EXCLUSIVE ) ;
		void   tbput( string tb_name, string tb_namelst="", string tb_order="" ) ;
		void   tbquery( string tb_name, string tb_keyn="", string tb_varn="", string tb_rownn="", string tb_keynn="", string tb_namenn="",string tb_crpn="", string tb_sirn="", string tb_lstn="", string tb_condn="", string tb_dirn="" ) ;
		void   tbsarg( string tb_name, string tb_namelst="", string tb_dir="NEXT", string tb_cond_pairs="" ) ;
		void   tbsave( string tb_name, string new_name="", string path="") ;
		void   tbscan( string tb_name, string tb_namelst="", string tb_savenm="", string tb_rowid_vn="", string tb_dir="NEXT", string tb_read="", string tb_crp_name="", string tb_condlst="" ) ;
		void   tbskip( string tb_name, int num=1, string tb_savenm="", string tb_rowid_vn="", string tb_rowid="", string tb_noread="", string tb_crp_name=""  ) ;
		void   tbsort( string tb_name, string tb_fields ) ;
		void   tbtop( string tb_name ) ;
		void   tbvclear( string tb_name ) ;

		bool   isTableOpen( string tb_name, string func ) ;
		bool   isTableUpdate( string tb_name, string func ) ;

		void   browse( string m_file, string m_panel="" ) ;
		void   edit( string m_file, string m_panel=""   ) ;
		void   view( string m_file, string m_panel=""   ) ;
		void   setmsg( string msg, msgSET sType=UNCOND  ) ;

		void   addpop( string ="", int =0, int =0 ) ;
		void   rempop( string ="" ) ;

		void   wait_event() ;

		void   checkRCode( string ="" ) ;

		void   set_cursor( int row, int col ) ;

		bool   isprimMenu()  ;
		bool   popupDisplayed() { return addpop_active ; }
		void   get_home( uint & row, uint & col ) ;
		void   get_cursor( uint & row, uint & col ) ;
		void   set_msg( string, string, cuaType, bool ) ;
		bool   nretriev_on()   ;
		string get_nretfield() ;
		void   cleanup()       ;
		void   cleanup_default() ;
		void   (pApplication::*pcleanup)() = &pApplication::cleanup_default ;
		void   abend()       ;
		void   abendexc()    ;
		void   set_forced_abend() ;
		void   closeTables() ;
		void   closeLog()    ;

		string ZAPPNAME   ;
		string ZAPPDESC   ;
		string ZMLIB      ;
		string ZPLIB      ;
		string ZTLIB      ;
		string ZZAPPLID   ;
		string ZCURFLD    ;
		int    ZCURPOS    ;
		string ZTDMARK    ;
		int    ZTDDEPTH   ;
		int    ZTDROWS    ;
		int    ZTDSELS    ;
		int    ZTDTOP     ;
		int    ZTDVROWS   ;
		int    ZCURINX    ;
		string ZCMD       ;
		string ZAPPLID    ;
		string ZEDLMSG    ;
		string ZEDSMSG    ;
		string ZHELP      ;
		string ZAHELP     ;
		string ZMHELP     ;
		string ZHOME      ;
		string ZORXPATH   ;
		string ZPFKEY     ;
		string ZSEL       ;
		string ZSELPARM   ;
		string ZSCREEN    ;
		string ZSCROLL    ;
		int    ZSCRMAXD   ;
		int    ZSCRMAXW   ;
		int    ZSCROLLN   ;
		string ZSCROLLA   ;
		string ZSMSG      ;
		string ZLMSG      ;
		cuaType ZMSGTYPE  ;
		bool   ZMSGALRM   ;
		string ZUSER      ;
		string ZVERB      ;
		string ZMUSER     ;
		string ZPUSER     ;
		string ZTUSER     ;
		int    ZRC        ;
		int    ZRSN       ;
		string ZRESULT    ;
		string ZERR1      ;
		string ZERR2      ;

	private:
		boost::mutex mutex ;

		int    addpop_row          ;
		int    addpop_col          ;

		string dumpFile            ;

		bool   addpop_active       ;

		bool   ControlErrorsReturn ;
		bool   abending            ;

		stack<string> stk_str      ;
		stack<int> stk_int         ;
		stack<int> addpop_stk      ;

		void read_Message( string ) ;
		void load_keylist( pPanel * ) ;
} ;
