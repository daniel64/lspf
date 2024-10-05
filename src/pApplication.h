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


enum s_paths
{
	s_ZFILE,
	s_ZLLIB,
	s_ZMLIB,
	s_ZPLIB,
	s_ZSLIB,
	s_ZTABL,
	s_ZTLIB
} ;


enum CTL_LIST
{
	CTL_ABENDRTN,
	CTL_CUA,
	CTL_DISPLAY,
	CTL_ERRORS,
	CTL_HISTORY,
	CTL_NONDISPL,
	CTL_NOTIFY,
	CTL_PASSTHRU,
	CTL_REFLIST,
	CTL_SIGTERM,
	CTL_SIGUSR1,
	CTL_SPLIT
} ;


class guardInt
{
	public:
		guardInt( int& RC ) : rc1( &RC ), rc2( RC )
		{}

		~guardInt()
		{
			*rc1 = rc2 ;
		}

	private:
		int* rc1 ;
		int  rc2;
} ;


class zlibdef
{
	public:
		zlibdef( const string& p,
			 const string& l )
		{
			paths  = p ;
			ddname = l ;
		}

		zlibdef( const string& p ) : zlibdef()
		{
			paths = p ;
		}

		zlibdef()
		{
			ddname = "" ;
		}

		bool library() const
		{
			return ( ddname != "" ) ;
		}

		string paths ;
		string ddname ;

} ;


class zaltlib
{
	public:
		zaltlib( const string& p,
			 const string& l,
			 bool f )
		{
			paths  = p ;
			ddname = l ;
			tofree = f ;
		}

		zaltlib( const string& p,
			 const string& l ) : zaltlib()
		{
			paths  = p ;
			ddname = l ;
		}

		zaltlib( const string& p ) : zaltlib()
		{
			ddname = p ;
		}

		zaltlib()
		{
			paths  = "" ;
			tofree = true ;
		}

		bool dynalloc() const
		{
			return ( paths != "" ) ;
		}

		string paths ;
		string ddname ;
		bool   tofree ;

} ;


class pApplication
{
	public:
		pApplication() ;
		virtual ~pApplication() ;

		void run() ;

		virtual void application() = 0 ;
		virtual void isredit( const string& ) {} ;
		virtual void action_signal() {} ;

		static vector<string>notifies ;

		static logger* lg ;

		static uint pflgToken ;

		static bool controlNonDispl ;
		static bool controlDisplayLock ;
		static bool controlSplitEnable ;
		static bool lineInOutDone ;

		static pApplication* self_ptr() { return self    ; }
		static void clr_self()          { self = nullptr ; }

		int    RC ;
		int    ZRC ;
		int    ZRSN ;
		string ZRESULT ;
		string PARM ;

		bool   errPanelissued ;
		bool   propagateEnd ;
		bool   jumpEntered ;
		bool   busyAppl ;
		bool   terminateAppl ;
		bool   applicationEnded ;
		bool   abnormalEnd ;
		bool   abnormalEndForced ;
		bool   abnormalTimeout ;
		bool   abnormalNoMsg ;
		bool   reloadCUATables ;
		bool   interrupted ;
		int    shrdPool ;
		bool   SEL ;
		bool   newpool ;
		string newappl ;
		bool   passlib ;
		bool   suspend ;
		bool   setMessage ;
		string rexxName ;
		string reffield ;
		string outBuffer ;
		string inBuffer ;

		boost::thread* pThread ;
		pApplication*  uAppl ;

		void (* lspfCallback)( lspfCommand& ) ;

		pPanel* currPanel ;
		pPanel* currtbPanel ;

		uint   taskid()   { return taskId ; }
		bool   abending() { return abending1 ; }

		void   init_phase1( selobj&,
				    int,
				    fPOOL*,
				    void (* lspfCallback)( lspfCommand& ) ) ;

		void   init_phase2() ;
		void   info() ;

		void*  get_userdata()           { return ApplUserData[ stid ] ; }
		void*  get_userdata( uint t )   { return ApplUserData[ t ]   ; }
		void   set_userdata( void* v )  { ApplUserData[ taskId ] = v ; }
		virtual string sysexec() ;

		string get_zsel() ;
		const string get_trail() ;
		selobj get_select_cmd()  { return selct ; }
		string get_current_panelDesc()  ;
		string get_current_screenName() ;
		string get_panelid() ;
		string get_keyshelp() ;
		string get_exhelp() ;
		string get_help() ;
		string get_command_buffer( errblock& ) ;
		void   set_command_retbuf( errblock&, const string& ) ;
		bool   simulate_enter()     { return ( controlNonDispl || controlDisplayLock ) ; }
		bool   simulate_end()       { return ( controlNonDispl && controlNonDisplEnd ) ; }
		bool   show_help_member() ;
		string get_swb_name() ;

		void set_userAction( USR_ACTIONS a ) ;

		void set_zlibd_altlib( pApplication* ) ;
		void set_zlibd_altlib( bool, pApplication* ) ;

		const map<string,stack<zlibdef>>& get_zlibd() { return zlibd ; }
		const map<char,stack<zaltlib>>& get_zaltl()   { return zaltl ; }

		string get_applid() ;

		const string& get_jobkey() ;

		void  set_appdesc( const string& s )    { zappdesc = s ; }
		void  set_appver( const string& s )     { zappver  = s ; }

		const string& get_appname()             { return zappname ; }
		const string& get_appdesc()             { return zappdesc ; }
		const string& get_appver()              { return zappver  ; }
		const string& modname()                 { return zappname ; }

		bool  zhelppgm_home() ;

		void  control( const string&,
			       const string&,
			       const string& ="" ) ;
		void  control( const string&,
			       void (pApplication::*)() ) ;

		void  libdef( const string&,
			      const string& = "",
			      const string& = "",
			      const string& = "" ) ;

		void  say( const string& = "",
			   bool = true ) ;
		void  rdisplay( const string& = "",
				bool = true ) ;
		void  pull( const string& ) ;
		void  pull( string* ) ;

		void  ftclose( const string&,
			       string = "",
			       string = "" ) ;
		void  fterase( const string&,
			       const string& = "" ) ;
		void  ftincl( const string&,
			      string = "" ) ;
		void  ftopen( string = "" ) ;

		void  dsinfo( const string& ) ;

		void  display( string p_name = "",
			       const string& = "",
			       const string& = "",
			       int = 0,
			       const string& = "",
			       const string& = "",
			       const string& = "" ) ;

		void  pquery( const string&,
			      const string&,
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "" ) ;

		void  lmclose( const string& ) ;

		void  lmddisp( const string& ) ;

		void  lmdfree( const string& ) ;

		void  lmdinit( const string&,
			       const string& ) ;

		void  lmfree( const string& ) ;

		void  lmget( const string&,
			     const string&,
			     const string&,
			     const string&,
			     size_t ) ;

		void  lminit( const string&,
			      const string&,
			      const string&,
			      LM_DISP = LM_SHR ) ;

		void  lminit( const string&,
			      const string&,
			      const string&,
			      const string& = "SHR" ) ;

		void  lmmadd( const string&,
			      const string& ) ;

		void  lmmdel( const string&,
			      const string& ) ;

		void  lmmfind( const string&,
			       const string&,
			       string = "NO" ) ;

		void  lmmlist( const string&,
			       string = "LIST",
			       const string& = "",
			       string = "NO",
			       const string& = "" ) ;

		void  lmmrep( const string&,
			      const string& ) ;

		void  lmopen( const string&,
			      const string&,
			      const string& = "" ) ;

		void  lmput( const string&,
			     const string&,
			     const string&,
			     int ) ;

		void  lmquery( const string&,
			       const string& = "",
			       const string& = "",
			       const string& = "",
			       const string& = "",
			       const string& = "" ) ;

		bool  notify() ;
		void  notify( const string&,
			      bool = true ) ;
		bool  notify_pending() ;

		void  select( const string& ) ;
		void  select( const selobj& ) ;

		void  submit( const string& ) ;
		void  submit( const selobj& ) ;

		void  qbaselib( const string&,
				const string& = "" ) ;

		void  qlibdef( const string&,
			       const string& = "",
			       const string& = "" ) ;

		void  queryenq( const string&,
				const string&,
				const string&,
				const string&,
				const string& = "",
				int  = 5000,
				const string& = "" ) ;

		void  vcopy( const string&,
			     string&,
			     vcMODE=MOVE ) ;
		void  vcopy( const string&,
			     string*&,
			     vcMODE=LOCATE ) ;

		void   vdefine( const string&,
				string*,
				string* = nullptr,
				string* = nullptr,
				string* = nullptr,
				string* = nullptr,
				string* = nullptr,
				string* = nullptr,
				string* = nullptr,
				string* = nullptr,
				string* = nullptr,
				string* = nullptr,
				string* = nullptr ) ;

		void   vdefine( const string&,
				int*,
				int* = nullptr,
				int* = nullptr,
				int* = nullptr,
				int* = nullptr,
				int* = nullptr,
				int* = nullptr,
				int* = nullptr,
				int* = nullptr,
				int* = nullptr,
				int* = nullptr,
				int* = nullptr ) ;

		void   vdelete( const string& ) ;
		void   vdelete( const string&,
				const string& ) ;
		void   vdelete( const string&,
				const string&,
				const string& ) ;
		void   vdelete( const string&,
				const string&,
				const string&,
				const string& ) ;
		void   verase( const string&,
			       poolType = ASIS ) ;
		void   vget( const string&,
			     poolType =ASIS ) ;
		void   vput( const string&,
			     poolType =ASIS ) ;
		set<string>& vlist( poolType pType,
				    int lvl ) ;

		set<string>& vilist( vdType = ALL ) ;
		set<string>& vslist( vdType = ALL ) ;

		void   vmask( const string&,
			      const string&,
			      const string& ) ;
		void   vreplace( const string&,
				 const string& ) ;
		void   vreplace( const string&,
				 int ) ;
		void   vreset() ;

		void   list( const string& ) ;
		void   log( const string& ) ;
		void   qtabopen( const string& ) ;

		void   tbadd( const string&,
			      string = "",
			      const string& = "",
			      int = 0 ) ;

		void   tbbottom( const string&,
				 const string& = "",
				 const string& = "",
				 const string& = "",
				 const string& = "" ) ;

		void   tbclose( const string&,
				const string& = "",
				string = "" ) ;

		void   tbcreate( const string&,
				 string,
				 string,
				 tbWRITE = NOWRITE,
				 tbREP = NOREPLACE,
				 string = "",
				 tbDISP = NON_SHARE ) ;

		void   tbdelete( const string& tb_name ) ;

		void   tbdispl( const string&,
				string = "",
				const string& = "",
				string = "",
				int = 0,
				int = 1,
				string = "YES",
				const string& = "",
				const string& = "",
				const string& = "" ) ;

		void   tbend( const string& ) ;

		void   tberase( const string&,
				string = "" ) ;

		void   tbexist( const string& ) ;

		void   tbget( const string&,
			      const string& = "",
			      const string& = "",
			      const string& = "",
			      const string& = "" ) ;

		void   tbmod( const string&,
			      string = "",
			      const string& = "" ) ;

		void   tbopen( const string&,
			       tbWRITE,
			       string = "",
			       tbDISP = NON_SHARE ) ;

		void   tbput( const string&,
			      string = "",
			      const string& = "" ) ;

		void   tbquery( const string&,
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "" ) ;

		void   tbsarg( const string&,
			       const string& = "",
			       const string& = "NEXT",
			       const string& = "" ) ;

		void   tbsave( const string&,
			       const string& = "",
			       string = "" ) ;

		void   tbscan( const string&,
			       const string& = "",
			       const string& = "",
			       const string& = "",
			       const string& = "NEXT",
			       const string& = "",
			       const string& = "",
			       const string& = "" ) ;

		void   tbskip( const string&,
			       int = 1,
			       const string& = "",
			       const string& = "",
			       const string& = "",
			       const string& = "",
			       const string& = "" ) ;

		void   tbsort( const string&,
			       string ) ;

		void   tbstats( const string&,
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				const string& = "",
				string = "",
				const string& = "",
				const string& = "",
				const string& = "" ) ;

		void   tbtop( const string& ) ;

		void   tbvclear( const string& ) ;

		void   browse( const string&,
			       const string& = "",
			       const string& = "",
			       int = 0 ) ;

		void   edit( const string&,
			     const string& = "",
			     const string& = "",
			     const string& = "",
			     const string& = "",
			     const string& = "",
			     const string& = "",
			     const string& = "",
			     const string& = "",
			     int = 0 ) ;

		void   view( const string&,
			     const string& = "",
			     const string& = "",
			     const string& = "",
			     const string& = "",
			     const string& = "",
			     const string& = "",
			     const string& = "",
			     const string& = "" ) ;

		void   edrec( const string& ) ;

		void   setmsg( const string&,
			       msgSET = UNCOND,
			       const string& = "" ) ;

		void   getmsg( const string&,
			       const string&,
			       const string& ="",
			       const string& ="",
			       const string& ="",
			       const string& ="",
			       const string& ="" ) ;

		void   addpop( const string& ="",
			       int = 0,
			       int = 0 ) ;
		void   rempop( const string& ="" ) ;
		void   movepop( int, int ) ;

		void   set_pcursor( uint,
				    uint ) ;
		void   set_lcursor( uint,
				    uint ) ;
		void   set_lcursor_home() ;

		bool   inputInhibited() ;
		bool   msgInhibited()   ;
		void   display_pd( errblock& ) ;
		void   display_id()   ;
		void   build_pfkeys( bool = false ) ;
		void   display_pfkeys() ;
		void   toggle_fscreen() ;
		bool   isprimMenu()     ;
		void   get_home2( uint&,
				  uint& ) ;
		void   get_pcursor( uint&,
				    uint& ) ;
		void   get_lcursor( uint&,
				    uint& ) ;
		void   set_msg( const string& ) ;
		void   set_msg1( const slmsg&,
				 const string& ) ;
		void   display_setmsg() ;
		void   clear_msg() ;
		slmsg  getmsg1()                 { return zmsg1   ; }
		string getmsgid1()               { return zmsgid1 ; }
		void   msgResponseOK() ;
		bool   nretriev_on() ;

		const string& get_zparm()        { return zparm   ; }
		const string& get_nretfield() ;
		const string& get_history_fields() ;

		void   (pApplication::*pcleanup)() = &pApplication::cleanup_default ;
		void   (pApplication::*psigusr1)() = &pApplication::handle_sigusr1  ;
		void   (pApplication::*psigterm)() = &pApplication::handle_sigterm  ;

		bool   cleanupRunning()          { return !abended ; }

		void   abend() ;
		void   abend( const string&,
			      const string& = "",
			      const string& = "",
			      const string& = "" ) ;

		void   uabend() ;
		void   uabend( const string&,
			       int = -1 ) ;
		void   uabend( const string&,
			       const string&,
			       int = -1 ) ;
		void   uabend( const string&,
			       const string&,
			       const string&,
			       int = -1 ) ;
		void   uabend( const string&,
			       const string&,
			       const string&,
			       const string&,
			       int = -1 ) ;

		void   enqv( const string&,
			     const string&,
			     enqDISP = EXC,
			     enqSCOPE = GLOBAL ) ;
		void   enq( const string&,
			    const string&,
			    enqDISP = EXC,
			    enqSCOPE = GLOBAL ) ;
		void   deq( const string&,
			    const string&,
			    enqSCOPE = GLOBAL ) ;
		void   deqv( const string&,
			     const string&,
			     enqSCOPE = GLOBAL ) ;
		void   qscanv( const string&,
			       const string&,
			       enqDISP = EXC,
			       enqSCOPE = GLOBAL ) ;

		void   qscan( const string&,
			      const string&,
			      enqDISP = EXC,
			      enqSCOPE = GLOBAL ) ;

		void   set_forced_abend()  ;
		void   set_timeout_abend() ;
		void   loadCommandTable() ;
		void   store_Zvars() ;
		void   restore_Zvars() ;
		void   reload_keylist() ;
		bool   errorsReturn() ;
		void   setDebugMode() ;
		void   setDebugLevel( const string& ) ;
		void   clearDebugMode() ;
		bool   selectPanel()             { return selPanel ; }

		void   set_addpop( pApplication* ) ;
		void   get_addpop( uint&,
				   uint& ) ;

		bool   do_refresh_lscreen()      { return refreshlScreen  ; }
		void   refresh_lscreen_done()    { refreshlScreen = false ; }
		bool   line_output_pending()     { return lineOutPending  ; }
		bool   line_input_pending()      { return lineInPending   ; }

		void   redraw_panel( errblock& ) ;

		bool   is_nested()               { return nested  ; }
		bool   is_pfkey()                { return pfkey   ; }
		void*  get_options()             { return options ; }
		string get_status() ;

		void   show_enqueues() ;

		void   remove_pd( pPanel* ) ;

		bool   background()              { return backgrd ; }

		bool   error_msg_issued() ;
		bool   cmd_nonblank() ;
		void   set_clear_msg( bool b )   { clearMessage = b ; }
		bool   get_selopt1()             { bool x = selopt1 ; selopt1 = false ; return x ; }
		const  string& get_selopt()      { return selopt ; }

		void   set_selopt( const string& s ) { if ( selPanel ) { selopt = s ; } }

		void   save_errblock() ;

		void   set_self()                { self = this ; }
		void   restore_errblock() ;
		void   ispexec( const string& ) ;

		errblock get_errblock()          { return errblk ; }

		string sub_vars( string ) ;


	private:
		static boost::mutex mtx ;
		static pApplication* self ;

		static poolMGR*  p_poolMGR  ;
		static tableMGR* p_tableMGR ;
		static gls*      p_gls ;

		static map<int, void*>ApplUserData ;

		struct sigaction lsig_action ;

		boost::condition* cond_mstr ;
		boost::mutex mutex ;

		static std::stringstream cout_buffer ;
		static std::stringstream cerr_buffer ;

		vector<enqueue>l_enqueues ;

		fPOOL* funcPool ;

		lss* p_lss ;

		pFTailor* FTailor ;

		int  initRC ;

		int  addpop_row ;
		int  addpop_col ;
		int  zscreen ;
		int  zscrnum ;

		uint  taskId ;
		uint  ptid ;
		uint  stid ;

		map<string, uint> table_id ;

		bool initc ;
		bool backgrd ;
		bool notifyEnded ;
		bool addpop_active  ;
		bool refreshlScreen ;
		bool controlErrorsReturn ;
		bool controlPassLRScroll ;
		bool controlNonDisplEnd  ;
		bool selPanel ;
		bool abending1 ;
		bool abending2 ;
		bool abended  ;
		bool debugMode ;
		bool lineOutPending ;
		bool lineInPending  ;
		bool cmdTableLoaded ;
		bool clearMessage ;
		bool zstored ;
		bool nested ;
		bool pfkey ;
		bool nolss ;
		bool nofunc ;
		bool zexpand ;
		bool nocheck ;
		bool tutorial ;
		bool service ;
		bool selopt1 ;
		bool nollog ;
		void* options ;

		selobj selct ;

		string zjobkey ;
		string selopt  ;

		uint pfllToken ;

		string dHelp    ;

		string zparm    ;
		string zappname ;
		string zappdesc ;
		string zappver  ;

		string zscrname ;

		string zmsgid   ;
		string zmsgid1  ;
		slmsg  zmsg     ;
		slmsg  zmsg1    ;
		string zzmlib   ;
		string zzplib   ;
		string zzslib   ;
		string zztlib   ;
		string zzllib   ;
		string zztabl   ;
		string zzfile   ;
		string zzmusr   ;
		string zzpusr   ;
		string zzsusr   ;
		string zztusr   ;
		string zzlusr   ;
		string zztabu   ;
		string zzfilu   ;
		string zdatef   ;
		string zklpriv  ;
		string zerr1    ;
		string zerr2    ;
		string zerr3    ;
		string zerr4    ;
		string zerr5    ;
		string zerr6    ;
		string zerr7    ;
		string zerr8    ;

		string nullstr ;

		string startDate ;
		string startTime ;
		string cbuffer   ;
		string rbuffer   ;

		WAIT_REASON waiting_on ;
		USR_ACTIONS usr_action ;

		void get_message( const string& ) ;
		void get_message( const string&,
				  string&,
				  string& ) ;
		int  check_message_id( const string& ) ;
		bool load_message( const string& ) ;
		bool sub_message_vars( slmsg & ) ;
		void set_screenName() ;
		void set_panelName( const string& ) ;
		string get_old_panelName() ;

		void display_row_indicator() ;

		void update_ztdfvars( int,
				      int,
				      int ) ;

		void set_ZVERB_panel_resp() ;
		void set_ZVERB_panel_resp_re_init() ;
		void set_ZVERB( const string& ) ;

		void set_panel_zvars() ;
		bool end_pressed() ;

		string get_file_for_dataid( const string& ) ;

		string get_file_for_listid( const string& ) ;

		string get_ddname_search_paths( const string& ) ;

		bool valid_panel_addr( pPanel* ) ;

		void set_nondispl_enter() ;
		void set_nondispl_end() ;
		void clr_nondispl() ;

		void tbskip( const string&,
			     const string&,
			     const string&,
			     const string&,
			     const string& ) ;

		void tbquery( const string&,
			      uint& ) ;

		int  edrec_init( const string&,
				 const string&,
				 const string&,
				 const string&,
				 const string&,
				 const string& ) ;

		int  edrec_query( const string&,
				  const string&,
				  const string&,
				  const string&,
				  const string&,
				  const string& ) ;

		int  edrec_process( const string&,
				    const string&,
				    const string&,
				    const string&,
				    const string&,
				    const string& ) ;

		int  edrec_cancel( const string&,
				   const string&,
				   const string&,
				   const string&,
				   const string&,
				   const string& ) ;

		int  edrec_defer( const string&,
				  const string& ) ;

		void edit_rec( const edit_parms& ) ;

		void check_qrname( const string&,
				   const string&,
				   const string& ) ;

		fPOOL* get_fpool_addr() { return funcPool ; }

		bool   get_zexpand()    { return zexpand ; }

		bool   get_nocheck()    { return nocheck ; }

		void   set_exhelp( const string& ) ;

		bool   do_clear_msg()   { return clearMessage ; }

		errblock errblk ;
		errblock serblk ;

		map<string, slmsg> msgList ;
		map<string, pPanel*> panelList ;

		stack<pPanel*> tbpanel_stk ;
		stack<string> stk_str ;
		stack<int> stk_int ;
		stack<int> stk_nofunc ;
		stack<popup> popups ;
		stack<stack<string>> urid_stk ;

		map<string,stack<zlibdef>> zlibd ;
		map<char,stack<zaltlib>> zaltl ;

		string get_search_paths( string,
					 s_paths ) ;
		string get_libdef_search_paths( s_paths ) ;
		string get_libdef_search_paths( const string& ) ;

		bool has_libdef( const string& ) ;

		void load_keylist( pPanel* ) ;
		pPanel* create_panel( const string& p_name ) ;
		void adjust_popups( pPanel*,
				    pPanel* = nullptr ) ;
		void actionSelect() ;

		void checkRCode() ;
		void checkRCode( errblock ) ;
		void splitZerrlm( string ) ;
		void xabend( const string&,
			     int = -1 ) ;
		void cleanup_default() ;
		void handle_sigusr1() ;
		void handle_sigterm() ;

		bool tableNameOK( const string& tb_name,
				  const string& err ) ;

		string full_name( const string&,
				  const string& ) ;

		string get_shared_var( const string& ) ;

		void ft_cleanup() ;

		void term_resize() ;

		class xTerminate {} ;

		void abendexc() ;
		void abend_nothrow() ;

		void wait_event( WAIT_REASON ) ;

		void write_output() ;

		map<string, CTL_LIST> controls = { { "ABENDRTN",  CTL_ABENDRTN },
						   { "CUA",       CTL_CUA      },
						   { "DISPLAY",   CTL_DISPLAY  },
						   { "ERRORS",    CTL_ERRORS   },
						   { "HISTORY",   CTL_HISTORY  },
						   { "NONDISPL",  CTL_NONDISPL },
						   { "NOTIFY",    CTL_NOTIFY   },
						   { "PASSTHRU",  CTL_PASSTHRU },
						   { "REFLIST",   CTL_REFLIST  },
						   { "SIGTERM",   CTL_SIGTERM  },
						   { "SIGUSR1",   CTL_SIGUSR1  },
						   { "SPLIT",     CTL_SPLIT    } } ;

		map<string, string> ddaliases = { { "ISPFILE", "ZFILE" },
						  { "ISPLLIB", "ZLLIB" },
						  { "ISPMLIB", "ZMLIB" },
						  { "ISPPLIB", "ZPLIB" },
						  { "ISPSLIB", "ZSLIB" },
						  { "ISPTABL", "ZTABL" },
						  { "ISPTLIB", "ZTLIB" } } ;

		friend int main( int argc,
				 char** argv ) ;

		friend void startApplication( selobj&,
					      bool,
					      bool ) ;

		friend void terminateApplicationBack( pApplication* ) ;

		friend void terminateApplication() ;

		friend void setGlobalClassVars() ;

		friend class pLScreen ;
		friend class pPanel ;
		friend class pFTailor ;

		friend class TSOENV ;
} ;


void handle_signal( int signal )
{
	pApplication* p = pApplication::self_ptr() ;

	if ( p )
	{
		if ( signal == SIGTERM )
		{
			(p->*(p->psigterm))() ;
		}
		else if ( signal == SIGUSR1 )
		{
			(p->*(p->psigusr1))() ;
		}
		pApplication::clr_self() ;
	}
}

#define tlog(s)                            \
{                                          \
lg->lock() ;                               \
(*lg) << microsec_clock::local_time() <<   \
" " << left( modname(), 10 ) <<            \
" " << d2ds( taskid(), 5 ) << " T " << s ; \
lg->unlock() ;                             \
}

#ifdef DEBUG2
#define TRACE_FUNCTION() trace_logger l_##x##_scope( __FUNCTION__, __PRETTY_FUNCTION__, __LINE__, taskid(), modname() ) ;
#else
#define TRACE_FUNCTION()
#endif

class trace_logger
{
	public:
		static logger* lg ;

		thread_local static int indent ;

		trace_logger( const string& fn1,
			      const string& fn2,
			      int ln,
			      int t,
			      const string& m ) : taskId( t ), fn1( fn1 ), fn2( fn2 ), ln( ln ), zmodname( m )
		{
			startTime = std::chrono::steady_clock::now() ;

			ssize = max( 1, ( max( 30, int( fn1.size() ) ) - indent - int( fn1.size() ) ) ) ;

			tlog( "Enter method --> " << d2ds( indent, 2 ) << " "
						  << string( indent++, ' ' ) << fn1
						  << string( ssize, ' ' ) << "line: " << d2ds( ln, 6 ) << "   sig: "<< fn2 <<endl ) ;
		}

		~trace_logger()
		{
			auto endTime = std::chrono::steady_clock::now() ;

			auto resp = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime ).count() ;

			tlog( "Exit ----------> " << d2ds( --indent, 2 ) << " "
						  << string( indent, ' ' ) << fn1
						  << string( ssize, ' ' ) << "line: "<< d2ds( ln, 6 )
						  << " Response (ms): " << resp << endl ) ;
		}

		int   taskid()           { return taskId   ; }
		const string& modname()  { return zmodname ; }

		int   taskId ;

		string fn1 ;
		string fn2 ;

		int ln ;

		string zmodname ;

		int ssize ;

		std::chrono::steady_clock::time_point startTime ;
} ;


thread_local int trace_logger::indent = 0 ;


#define STANDARD_HEADER( a, b )  \
set_appdesc( a ) ;               \
set_appver( b ) ;


#define CHECK_ERROR_RETURN()     \
if ( errblk.error() )            \
{                                \
	checkRCode( errblk ) ;   \
	return ;                 \
}


#define CHECK_ERROR_RETURN_FALSE() \
if ( errblk.error() )            \
{                                \
	checkRCode( errblk ) ;   \
	return false ;           \
}


#define CHECK_ERROR_SETCALL_RETURN( a )  \
if ( errblk.error() )            \
{                                \
	errblk.setcall( a ) ;    \
	checkRCode( errblk ) ;   \
	return ;                 \
}


#define CHECK_ERROR_SETERROR_RETURN1( a ) \
if ( errblk.error() )            \
{                                \
	errblk.seterrid( TRACE_INFO(), "PSYE019D", a ) ; \
	checkRCode( errblk ) ;   \
	return ;                 \
}


#define CHECK_ERROR_SETERROR_RETURN2( a, b ) \
if ( errblk.error() )            \
{                                \
	errblk.seterrid( TRACE_INFO(), "PSYE019D", a, b ) ; \
	checkRCode( errblk ) ;   \
	return ;                 \
}


#define EXPAND( x ) x
#define GET_MACRO( _1, _2, NAME, ... ) NAME
#define CHECK_ERROR_SETERROR_RETURN( ... ) \
	EXPAND( GET_MACRO( __VA_ARGS__, CHECK_ERROR_SETERROR_RETURN2, CHECK_ERROR_SETERROR_RETURN1 )( __VA_ARGS__ ) )
