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
		virtual ~pApplication() ;

		void run() ;

		virtual void application() = 0 ;
		virtual void isredit( const string& ) {} ;

		static map<int, void*>ApplUserData ;
		static vector<enqueue>g_enqueues ;
		static vector<string>notifies ;
		static poolMGR*  p_poolMGR  ;
		static tableMGR* p_tableMGR ;
		static logger*   lg         ;

		static bool ControlNonDispl ;
		static bool ControlDisplayLock ;
		static bool ControlSplitEnable ;
		static bool lineInOutDone ;

		int    RC                 ;
		int    ZRC                ;
		int    ZRSN               ;
		string ZRESULT            ;
		string PARM               ;

		bool   errPanelissued     ;
		bool   testMode           ;
		bool   propagateEnd       ;
		bool   jumpEntered        ;
		bool   busyAppl           ;
		bool   terminateAppl      ;
		bool   applicationEnded   ;
		bool   abnormalEnd        ;
		bool   abnormalEndForced  ;
		bool   abnormalTimeout    ;
		bool   abnormalNoMsg      ;
		bool   reloadCUATables    ;
		int    shrdPool           ;
		bool   SEL                ;
		bool   newpool            ;
		string newappl            ;
		bool   passlib            ;
		bool   suspend            ;
		bool   setMessage         ;
		string rexxName           ;
		string reffield           ;
		string outBuffer          ;
		string inBuffer           ;

		boost::thread* pThread    ;
		pApplication*  uAppl      ;

		void (* lspfCallback)( lspfCommand& ) ;

		pPanel* currPanel   ;
		pPanel* currtbPanel ;

		uint   taskid()         { return taskId ; }
		void   init_phase1( selobj&, int, void (* lspfCallback)( lspfCommand& ) ) ;
		void   init_phase2() ;
		void   info() ;

		string get_zsel() ;
		const string get_trail() ;
		selobj get_select_cmd() { return selct ; }
		string get_help_member( int, int ) ;
		string get_current_panelDesc()  ;
		string get_current_screenName() ;
		string get_panelid() ;
		string get_command_buffer() { return cbuffer ; }
		bool   simulate_enter()     { return ( ControlNonDispl || ControlDisplayLock ) ; }
		bool   simulate_end()       { return ( ControlNonDispl && ControlNonDisplEnd ) ; }
		bool   show_help_member() ;

		void set_userAction( USR_ACTIONS a ) ;

		void  set_zlibd( bool, pApplication* ) ;
		const map<string,stack<string>>& get_zlibd() { return zlibd ; }

		string get_applid() ;

		const string& get_jobkey() ;

		void  set_appdesc( const string& s )    { zappdesc = s ; }
		void  set_appver(  const string& s )    { zappver  = s ; }
		void  set_apphelp( const string& s )    { zapphelp = s ; }
		const string& get_appname()             { return zappname ; }
		const string& get_appdesc()             { return zappdesc ; }
		const string& get_appver()              { return zappver  ; }

		void   control( const string&, const string&, const string& ="" ) ;
		void   control( const string&, void (pApplication::*)() ) ;
		void   libdef( const string&, const string& ="", const string& ="", const string& ="UNCOND" ) ;
		void   rdisplay( const string&, bool =true ) ;
		void   pull( const string& ) ;
		void   pull( string* ) ;

		void   display( string p_name = "",
				const string& p_msg = "",
				const string& p_cursor = "",
				int p_curpos = 0,
				const string& p_buffer = "",
				const string& p_retbuf = "" ) ;

		void   pquery( const string& p_name,
			       const string& a_name,
			       const string& t = "",
			       const string& w = "",
			       const string& d = "",
			       const string& r= "",
			       const string& c = "" ) ;

		bool   notify() ;
		void   notify( const string&, bool =true ) ;
		bool   notify_pending() ;

		void   select( const string& ) ;
		void   select( const selobj& ) ;

		void   submit( const string& ) ;
		void   submit( const selobj& ) ;

		void   qlibdef( const string&, const string& ="" , const string& ="" ) ;

		void   vcopy( const string&, string&, vcMODE=MOVE ) ;
		void   vcopy( const string&, string*&, vcMODE=LOCATE ) ;

		void   vdefine( const string&,
				string*,
				string* =NULL,
				string* =NULL,
				string* =NULL,
				string* =NULL,
				string* =NULL,
				string* =NULL,
				string* =NULL,
				string* =NULL,
				string* =NULL ) ;

		void   vdefine( const string&,
				int*,
				int* =NULL,
				int* =NULL,
				int* =NULL,
				int* =NULL,
				int* =NULL,
				int* =NULL,
				int* =NULL,
				int* =NULL,
				int* =NULL ) ;

		void   vdelete( const string& ) ;
		void   verase( const string& var, poolType =ASIS ) ;
		void   vget( const string& var, poolType =ASIS )   ;
		void   vput( const string& var, poolType =ASIS )   ;
		set<string>& vlist( poolType pType, int lvl ) ;
		set<string>& vilist( vdType =ALL ) ;
		set<string>& vslist( vdType =ALL ) ;
		void   vmask( const string& var, const string& type, const string& mask ) ;
		void   vreplace( const string&, const string& ) ;
		void   vreplace( const string&, int )           ;
		void   vreset() ;

		void   log( const string& msg ) ;
		void   qtabopen( const string& tb_list ) ;

		void   tbadd( const string& tb_name,
			      string tb_namelst="",
			      const string& tb_order="",
			      int tb_num_of_rows=0 ) ;

		void   tbbottom( const string& tb_name,
				 const string& tb_savenm="",
				 const string& tb_rowid_vn="",
				 const string& tb_noread="",
				 const string& tb_crp_name="" ) ;

		void   tbclose( const string& tb_name, const string& new_name="", string path="" ) ;

		void   tbcreate( const string& tb_name,
				 string keys,
				 string names,
				 tbWRITE =NOWRITE,
				 tbREP =NOREPLACE,
				 string path="",
				 tbDISP =NON_SHARE ) ;

		void   tbdelete( const string& tb_name ) ;

		void   tbdispl( const string& tb_name,
				string p_name = "",
				const string& p_msg = "",
				string p_cursor = "",
				int p_csrrow = 0,
				int p_csrpos = 1,
				string p_autosel = "YES",
				const string& p_crp_name = "",
				const string& p_rowid_nm = "",
				const string& p_msgloc = "" ) ;

		void   tbend( const string& tb_name ) ;

		void   tberase( const string& tb_name, string tb_path="" ) ;

		void   tbexist( const string& tb_name ) ;

		void   tbget( const string& tb_name,
			      const string& tb_savenm="",
			      const string& tb_rowid_vn="",
			      const string& tb_noread="",
			      const string& tb_crp_name="" ) ;

		void   tbmod( const string& tb_name,
			      string tb_namelst="",
			      const string& tb_order="" ) ;

		void   tbopen( const string& tb_name,
			       tbWRITE WRITE,
			       string path="",
			       tbDISP c=NON_SHARE ) ;

		void   tbput( const string& tb_name,
			      string tb_namelst="",
			      const string& tb_order="" ) ;

		void   tbquery( const string& tb_name,
				const string& tb_keyn="",
				const string& tb_varn="",
				const string& tb_rownn="",
				const string& tb_keynn="",
				const string& tb_namenn="",
				const string& tb_crpn="",
				const string& tb_sirn="",
				const string& tb_lstn="",
				const string& tb_condn="",
				const string& tb_dirn="" ) ;

		void   tbsarg( const string& tb_name,
			       const string& tb_namelst="",
			       const string& tb_dir="NEXT",
			       const string& tb_cond_pairs="" ) ;

		void   tbsave( const string& tb_name,
			       const string& new_name="",
			       string path="" ) ;

		void   tbscan( const string& tb_name,
			       const string& tb_namelst="",
			       const string& tb_savenm="",
			       const string& tb_rowid_vn="",
			       const string& tb_dir="NEXT",
			       const string& tb_read="",
			       const string& tb_crp_name="",
			       const string& tb_condlst="" ) ;

		void   tbskip( const string& tb_name,
			       int num=1,
			       const string& tb_savenm="",
			       const string& tb_rowid_vn="",
			       const string& tb_rowid="",
			       const string& tb_noread="",
			       const string& tb_crp_name="" ) ;

		void   tbsort( const string& tb_name,
			       string tb_fields ) ;

		void   tbtop( const string& tb_name ) ;

		void   tbvclear( const string& tb_name ) ;

		void   browse( const string& m_file,
			       const string& m_panel="" ) ;

		void   edit( const string& m_file,
			     const string& m_panel="",
			     const string& m_macro="",
			     const string& m_profile="",
			     const string& m_lcmds="",
			     const string& m_confirm="",
			     const string& m_preserv="" ) ;

		void   view( const string& m_file,
			     const string& m_panel="",
			     const string& m_macro="",
			     const string& m_profile="",
			     const string& m_lcmds="",
			     const string& m_confirm="",
			     const string& m_chgwarn="" ) ;

		void   edrec( const string& m_parm ) ;

		void   setmsg( const string& msg, msgSET sType=UNCOND  ) ;
		void   getmsg( const string&,
			       const string&,
			       const string& ="",
			       const string& ="",
			       const string& ="",
			       const string& ="",
			       const string& ="" ) ;

		void   addpop( const string& ="", int =0, int =0 ) ;
		void   rempop( const string& ="" ) ;
		void   movepop( int, int ) ;

		void   set_cursor( int row, int col ) ;
		void   set_cursor_home() ;

		bool   inputInhibited() ;
		bool   msgInhibited()   ;
		void   display_pd( errblock& ) ;
		void   display_id()     ;
		void   toggle_fscreen() ;
		bool   isprimMenu()     ;
		void   get_home( uint & row, uint & col ) ;
		void   get_cursor( uint & row, uint & col ) ;
		void   set_msg( const string& ) ;
		void   set_msg1( const slmsg&, const string& ) ;
		void   display_setmsg() ;
		void   clear_msg() ;
		slmsg  getmsg1()                 { return zmsg1   ; }
		string getmsgid1()               { return zmsgid1 ; }
		void   msgResponseOK() ;
		bool   nretriev_on()   ;
		string get_nretfield() ;
		string& get_zparm()              { return zparm ; }
		void   (pApplication::*pcleanup)() = &pApplication::cleanup_default ;
		bool   cleanupRunning()          { return !abended ; }
		void   abend() ;
		void   abend( const string& ) ;
		void   abend( const string&, const string& ) ;
		void   uabend() ;
		void   uabend( const string&, int = -1 ) ;
		void   uabend( const string&, const string&, int = -1 ) ;
		void   uabend( const string&, const string&, const string&, int = -1 ) ;
		void   uabend( const string&, const string&, const string&, const string&, int = -1 ) ;

		void   enq( const string&, const string&, enqDISP =EXC, enqSCOPE =GLOBAL ) ;
		void   deq( const string&, const string&, enqSCOPE =GLOBAL ) ;
		void   qscan( const string&, const string&, enqDISP =EXC, enqSCOPE =GLOBAL ) ;

		void   set_forced_abend()  ;
		void   set_timeout_abend() ;
		void   loadCommandTable()  ;
		void   store_scrname() ;
		void   restore_Zvars( int ) ;
		void   reload_keylist( pPanel* ) ;
		bool   errorsReturn() ;
		void   setTestMode()  ;
		bool   selectPanel()             { return selPanel ; }

		void   set_addpop( pApplication* ) ;

		bool   do_refresh_lscreen()      { return refreshlScreen  ; }
		void   refresh_lscreen_done()    { refreshlScreen = false ; }
		bool   line_output_pending()     { return lineOutPending  ; }
		bool   line_input_pending()      { return lineInPending   ; }

		void   redraw_panel( errblock& ) ;

		bool   is_nested()               { return nested  ; }
		void*  get_options()             { return options ; }
		string get_status() ;

		void   show_enqueues() ;

		bool   background()              { return backgrd ; }

		bool   error_msg_issued() ;

		void   save_errblock()    ;
		void   restore_errblock() ;
		void   ispexec( const string& ) ;

		errblock get_errblock()          { return errBlock ; }

		string sub_vars( string ) ;


	private:
		static boost::mutex mtx ;

		boost::condition* cond_mstr ;
		boost::mutex mutex ;

		vector<enqueue>l_enqueues ;

		fPOOL funcPOOL    ;

		pPanel* prevPanel ;

		int  addpop_row  ;
		int  addpop_col  ;
		int  lscreen     ;
		int  lscreen_num ;

		uint  taskId     ;
		uint  ptid       ;

		bool backgrd ;
		bool notifyEnded ;
		bool addpop_active  ;
		bool refreshlScreen ;
		bool ControlErrorsReturn ;
		bool ControlPassLRScroll ;
		bool ControlNonDisplEnd  ;
		bool selPanel ;
		bool abending ;
		bool abended  ;
		bool lineOutPending ;
		bool lineInPending  ;
		bool cmdTableLoaded ;
		bool nested     ;
		void* options   ;

		selobj selct    ;

		string zjobkey  ;

		string zparm    ;
		string zappname ;
		string zappdesc ;
		string zappver  ;
		string zapphelp ;

		string zmsgid   ;
		string zmsgid1  ;
		slmsg  zmsg     ;
		slmsg  zmsg1    ;
		string zzmlib   ;
		string zzplib   ;
		string zztlib   ;
		string zztabl   ;
		string zzmusr   ;
		string zzpusr   ;
		string zztusr   ;
		string zztabu   ;
		string zscrname ;
		string zerr1    ;
		string zerr2    ;
		string zerr3    ;
		string zerr4    ;
		string zerr5    ;
		string zerr6    ;
		string zerr7    ;
		string zerr8    ;

		string startDate ;
		string startTime ;
		string cbuffer   ;

		WAIT_REASON waiting_on ;
		USR_ACTIONS usr_action ;

		void get_message( const string& ) ;
		int  check_message_id( const string& ) ;
		bool load_message( const string& ) ;
		bool sub_message_vars( slmsg & ) ;
		void set_screenName() ;
		void set_ZVERB_panel_resp() ;
		void set_ZVERB_panel_resp_re_init() ;
		void set_panel_zvars() ;
		bool end_pressed() ;

		void set_nondispl_enter() ;
		void set_nondispl_end() ;
		void clr_nondispl() ;

		int    edrec_init( const string& m_parm,
				   const string& qname,
				   const string& rname,
				   const string& uprof,
				   const string& tabName,
				   const string& v_list ) ;
		int    edrec_query( const string& m_parm,
				    const string& qname,
				    const string& rname,
				    const string& uprof,
				    const string& tabName,
				    const string& v_list ) ;
		int    edrec_process( const string& m_parm,
				      const string& qname,
				      const string& rname,
				      const string& uprof,
				      const string& tabName,
				      const string& v_list ) ;
		int    edrec_cancel( const string& m_parm,
				     const string& qname,
				     const string& rname,
				     const string& uprof,
				     const string& tabName,
				     const string& v_list ) ;
		int    edrec_defer( const string& qname,
				    const string& rname ) ;

		void   edit_rec( const edit_parms& ) ;

		void   check_qrname( const string&,
				     const string&,
				     const string& ) ;

		errblock errBlock ;
		errblock serBlock ;

		map<string, slmsg> msgList ;
		map<string, pPanel*> panelList ;

		stack<pPanel*> tbpanel_stk ;
		stack<string> stk_str ;
		stack<int> stk_int  ;
		stack<popup> popups ;
		stack<stack<string>> urid_stk ;

		map<string,stack<string>> zlibd ;

		string get_search_path( s_paths ) ;
		string get_search_path( const string& ) ;

		void load_keylist( pPanel* ) ;
		map<string, pPanel*>::iterator createPanel( const string& p_name ) ;
		void actionSelect()   ;

		void checkRCode() ;
		void checkRCode( errblock ) ;
		void splitZerrlm( string )  ;
		void xabend( const string&, int = -1 ) ;
		void cleanup_default()      ;

		bool tableNameOK( const string& tb_name, const string& err ) ;

		class xTerminate {} ;

		void abendexc() ;
		void abend_nothrow() ;

		void wait_event( WAIT_REASON ) ;
} ;

#undef  MOD_NAME
#define MOD_NAME APPL

#undef llog
#undef debug1
#undef debug2


#define llog(t, s) \
{ \
lg->lock() ; \
(*lg) << microsec_clock::local_time() << \
" " << left( quotes(MOD_NAME), 10 ) << \
" " << d2ds( taskid(), 5 ) << " " << t << " " << s ; \
lg->unlock() ; \
}

#ifdef DEBUG1
#define debug1( s ) \
{ \
lg->lock() ; \
(*lg) << microsec_clock::local_time() << \
" " << left( quotes(MOD_NAME), 10 ) << \
" " << d2ds( taskid(), 5 ) << \
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
" " << left( quotes(MOD_NAME), 10 ) << \
" " << d2ds( taskid(), 5 ) << \
" D line: "  << __LINE__  << \
" >>L2 Function: " << __FUNCTION__ << \
" -  " << s ; \
lg->unlock() ; \
}
#else
#define debug2( s )
#endif
