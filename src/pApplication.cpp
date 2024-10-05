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

#include <sys/sysmacros.h>
#include <pwd.h>
#include <grp.h>


#define MAX_PANELS 20

namespace lspf {

void ispexeci( pApplication*,
	       const string&,
	       errblock& ) ;

}

boost::mutex pApplication::mtx ;

pApplication::pApplication()
{
	funcPool            = new fPOOL ;
	uAppl               = nullptr ;
	initc               = false  ;
	debugMode           = false  ;
	propagateEnd        = false  ;
	jumpEntered         = false  ;
	controlErrorsReturn = false  ;
	controlPassLRScroll = false  ;
	controlNonDisplEnd  = false  ;
	lineOutPending      = false  ;
	lineInPending       = false  ;
	cmdTableLoaded      = false  ;
	nested              = false  ;
	pfkey               = false  ;
	errPanelissued      = false  ;
	abending1           = false  ;
	abending2           = false  ;
	abended             = false  ;
	addpop_active       = false  ;
	addpop_row          = 0      ;
	addpop_col          = 0      ;
	taskId              = 0      ;
	backgrd             = false  ;
	notifyEnded         = false  ;
	busyAppl            = true   ;
	terminateAppl       = false  ;
	applicationEnded    = false  ;
	abnormalEnd         = false  ;
	abnormalEndForced   = false  ;
	abnormalTimeout     = false  ;
	abnormalNoMsg       = false  ;
	reloadCUATables     = false  ;
	refreshlScreen      = false  ;
	interrupted         = false  ;
	zjobkey             = ""     ;
	rexxName            = ""     ;
	pfllToken           = 0      ;
	newpool             = false  ;
	newappl             = ""     ;
	passlib             = false  ;
	suspend             = true   ;
	SEL                 = false  ;
	selPanel            = false  ;
	setMessage          = false  ;
	clearMessage        = false  ;
	zstored             = false  ;
	service             = false  ;
	selopt1             = false  ;
	nolss               = true   ;
	inBuffer            = ""     ;
	outBuffer           = ""     ;
	reffield            = ""     ;
	PARM                = ""     ;
	currPanel           = nullptr ;
	currtbPanel         = nullptr ;
	FTailor             = nullptr ;
	p_lss               = nullptr ;
	zappver             = ""     ;
	initRC              = 0      ;
	RC                  = 0      ;
	ZRC                 = 0      ;
	ZRSN                = 0      ;
	ZRESULT             = ""     ;
	waiting_on          = WAIT_NONE ;
	zerr1               = ""     ;
	zerr2               = ""     ;
	zerr3               = ""     ;
	zerr4               = ""     ;
	zerr5               = ""     ;
	zerr6               = ""     ;
	zerr7               = ""     ;
	zerr8               = ""     ;

	funcPool->define( errblk, "ZRC", &ZRC ) ;
	funcPool->define( errblk, "ZRSN", &ZRSN ) ;
	funcPool->define( errblk, "ZRESULT", &ZRESULT ) ;

	lsig_action.sa_handler = handle_signal ;
	sigfillset( &lsig_action.sa_mask ) ;
	lsig_action.sa_flags = 0 ;
}


pApplication::~pApplication()
{
	while ( !popups.empty() )
	{
		PANEL* pan1 = popups.top().pan1 ;
		PANEL* pan2 ;
		if ( pan1 )
		{
			WINDOW* w = panel_window( pan1 ) ;
			const void* vptr = panel_userptr( pan1 ) ;
			if ( vptr )
			{
				delete static_cast<const panel_data*>(vptr) ;
			}
			del_panel( pan1 ) ;
			delwin( w ) ;
			pan2 = popups.top().pan2 ;
			w    = panel_window( pan2 ) ;
			vptr = panel_userptr( pan2 ) ;
			if ( vptr )
			{
				delete static_cast<const panel_data*>(vptr) ;
			}
			del_panel( pan2 ) ;
			delwin( w ) ;
		}
		popups.pop() ;
		update_panels() ;
	}

	for ( auto& panel : panelList )
	{
		delete panel.second ;
	}

	if ( !nofunc )
	{
		delete funcPool ;
	}
	else
	{
		funcPool->put2( errblk, "ZSBTASK",  stk_nofunc.top() ) ;
		stk_nofunc.pop() ;
		funcPool->put2( errblk, "ZCURINX",  stk_nofunc.top() ) ;
		stk_nofunc.pop() ;
		funcPool->put2( errblk, "ZCURPOS",  stk_nofunc.top() ) ;
		stk_nofunc.pop() ;
		funcPool->put2( errblk, "ZTDVROWS", stk_nofunc.top() ) ;
		stk_nofunc.pop() ;
		funcPool->put2( errblk, "ZTDROWS",  stk_nofunc.top() ) ;
		stk_nofunc.pop() ;
		funcPool->put2( errblk, "ZTDDEPTH", stk_nofunc.top() ) ;
		stk_nofunc.pop() ;
		funcPool->put2( errblk, "ZTDSELS",  stk_nofunc.top() ) ;
		stk_nofunc.pop() ;
		funcPool->put2( errblk, "ZTDTOP",   stk_nofunc.top() ) ;
		stk_nofunc.pop() ;
	}

	if ( !nolss )
	{
		delete p_lss ;
	}

	delete FTailor ;
}


void pApplication::init_phase1( selobj& sel,
				int taskid,
				fPOOL* x_funcPool,
				void (* Callback)( lspfCommand& ) )
{
	//
	// Setup various program parameters.
	// Variable services are not available at this time.
	//

	zappname = sel.pgm  ;
	zparm    = sel.parm ;
	PARM     = sel.parm ;
	nofunc   = sel.nofunc ;
	zexpand  = sel.zexpand ;
	nocheck  = sel.nocheck ;
	passlib  = sel.passlib ;
	newappl  = sel.newappl ;
	newpool  = sel.newpool ;
	suspend  = sel.suspend ;
	selPanel = sel.selPanel() ;
	nested   = sel.nested  ;
	pfkey    = sel.pfkey   ;
	options  = sel.options ;
	backgrd  = sel.backgrd ;
	selopt   = sel.selopt  ;
	nollog   = sel.nollog  ;

	if ( selopt != "" )
	{
		selopt1 = true ;
	}

	if ( nofunc )
	{
		delete funcPool ;
		funcPool = x_funcPool ;
		stk_nofunc.push( funcPool->get2( errblk, 8, INTEGER, "ZTDTOP" ) ) ;
		stk_nofunc.push( funcPool->get2( errblk, 8, INTEGER, "ZTDSELS" ) ) ;
		stk_nofunc.push( funcPool->get2( errblk, 8, INTEGER, "ZTDDEPTH" ) ) ;
		stk_nofunc.push( funcPool->get2( errblk, 8, INTEGER, "ZTDROWS" ) ) ;
		stk_nofunc.push( funcPool->get2( errblk, 8, INTEGER, "ZTDVROWS" ) ) ;
		stk_nofunc.push( funcPool->get2( errblk, 8, INTEGER, "ZCURPOS" ) ) ;
		stk_nofunc.push( funcPool->get2( errblk, 8, INTEGER, "ZCURINX" ) ) ;
		stk_nofunc.push( funcPool->get2( errblk, 8, INTEGER, "ZSBTASK" ) ) ;
	}

	if ( sel.service )
	{
		service = true ;
		controlErrorsReturn = sel.errret ;
	}

	taskId = taskid ;
	ptid   = ( sel.ptid == 0 ) ? taskId : sel.ptid ;
	stid   = sel.stid ;
	errblk.taskid = taskid ;
	errblk.ptid   = ptid   ;
	lspfCallback  = Callback ;
}


void pApplication::init_phase2()
{
	//
	// Before being dispatched in its own thread, set the search paths and
	// create implicit function pool variables defined as INTEGER (kept across a VRESET).
	//

	zzplib = get_ddname_search_paths( "ZPLIB" ) ;
	zzmlib = get_ddname_search_paths( "ZMLIB" ) ;
	zzslib = get_ddname_search_paths( "ZSLIB" ) ;
	zztlib = get_ddname_search_paths( "ZTLIB" ) ;
	zzllib = get_ddname_search_paths( "ZLLIB" ) ;
	zztabl = get_ddname_search_paths( "ZTABL" ) ;
	zzfile = get_ddname_search_paths( "ZFILE" ) ;

	zzpusr = get_ddname_search_paths( "ZPUSR" ) ;
	zzmusr = get_ddname_search_paths( "ZMUSR" ) ;
	zzsusr = get_ddname_search_paths( "ZSUSR" ) ;
	zztusr = get_ddname_search_paths( "ZTUSR" ) ;
	zzlusr = get_ddname_search_paths( "ZLUSR" ) ;
	zztabu = get_ddname_search_paths( "ZTABU" ) ;
	zzfilu = get_ddname_search_paths( "ZFILU" ) ;

	zklpriv = p_poolMGR->get( errblk, "ZKLPRIV", PROFILE ) ;

	funcPool->put2( errblk, "ZTDTOP",   1 ) ;
	funcPool->put2( errblk, "ZTDSELS",  0 ) ;
	funcPool->put2( errblk, "ZTDDEPTH", 0 ) ;
	funcPool->put2( errblk, "ZTDROWS",  0 ) ;
	funcPool->put2( errblk, "ZTDVROWS", 0 ) ;
	funcPool->put2( errblk, "ZCURPOS",  1 ) ;
	funcPool->put2( errblk, "ZCURINX",  0 ) ;
	funcPool->put2( errblk, "ZSBTASK",  0 ) ;

	zscreen = ds2d( p_poolMGR->get( errblk, "ZSCREEN", SHARED ) ) ;
	zscrnum = ds2d( p_poolMGR->get( errblk, "ZSCRNUM", SHARED ) ) ;

	zdatef    = p_poolMGR->get( errblk, "ZDATEF", SHARED ) ;
	startTime = p_poolMGR->get( errblk, "ZTIMEL", SHARED ) ;
	startDate = p_poolMGR->get( errblk, "ZJ4DATE", SHARED ) ;

	errblk.setDebugMode( ds2d( p_poolMGR->get( errblk, zscrnum, "ZDEBUG" ) ) ) ;

	tutorial = ( zappname == p_poolMGR->get( errblk, "ZHELPPGM", PROFILE ) ) ;

	errblk.user = p_poolMGR->get( errblk, "ZUSER", SHARED ) ;

	cond_mstr = ( backgrd ) ? &cond_batch : &cond_lspf ;

	auto result = zaltl.insert( pair<char, stack<zaltlib>>( 'S', stack<zaltlib>() ) ) ;
	result.first->second.push( zaltlib( "SYSEXEC" ) ) ;

	if ( !p_lss )
	{
		p_lss = new lss ;
		nolss = false ;
	}

	llog( "I", "Phase 2 initialisation complete" << endl ; )
}


void pApplication::run()
{
	//
	// Start running the user application and catch any exceptions.
	// Not all exceptions are caught - eg. segmentation faults.
	//
	// Cleanup during termination:
	// 1) Close any tables opened by this application if a ptid.
	// 2) Close any files opened by this task.
	// 3) Unload the application command table if loaded by this application.
	// 4) Release any global enqueues held by this task.
	// 5) Send a notify if the batch job has ended/abended, if requested.
	// 6) FREE any new ALTLIB allocations if changes not passed back.
	//

	TRACE_FUNCTION() ;

	string t ;
	string err ;
	string ztime ;

	if ( initRC > 0 )
	{
		say( "Errors have occured during program constructor processing." ) ;
		say( "See application log for errors." ) ;
		say() ;
		terminateAppl    = true  ;
		applicationEnded = true  ;
		busyAppl         = false ;
		cond_mstr->notify_all()  ;
		t = ( backgrd ) ? " background " : " " ;
		llog( "I", "Shutting down"+ t +"application: " + zappname +" Taskid: " << taskId << endl ) ;
		return ;
	}

	sigaction( SIGTERM, &lsig_action, nullptr ) ;
	sigaction( SIGUSR1, &lsig_action, nullptr ) ;

	initc = true ;

	try
	{
		application() ;
	}
	catch ( pApplication::xTerminate )
	{
		llog( "E", "Application "+ zappname +" aborting..." << endl ) ;
	}
	catch (...)
	{
		try
		{
			abendexc() ;
		}
		catch (...)
		{
			llog( "E", "An abend has occured during abend processing" << endl ) ;
			llog( "E", "Calling abend() only to terminate application" << endl ) ;
			abend_nothrow() ;
		}
	}

	write_output() ;

	if ( ptid == taskId )
	{
		p_tableMGR->closeTablesforTask( errblk ) ;
	}

	p_gls->close_task_files( errblk.taskid ) ;

	if ( cmdTableLoaded )
	{
		p_tableMGR->destroyTable( errblk, get_applid() + "CMDS" ) ;
	}

	if ( passlib || newappl != "" )
	{
		auto ita = zaltl.find( 'A' ) ;
		if ( ita != zaltl.end() )
		{
			while ( !ita->second.empty() )
			{
				if ( ita->second.top().dynalloc() && ita->second.top().tofree )
				{
					p_gls->set_alloc_closed( ita->second.top().ddname ) ;
					p_gls->free( "F(" + ita->second.top().ddname + ")", err ) ;
					if ( err != "" )
					{
						llog( "E", "Free of ddname " + ita->second.top().ddname + " failed." << endl ) ;
						llog( "E", err << endl ) ;
					}
				}
				ita->second.pop() ;
			}
		}
	}

	if ( notifyEnded )
	{
		vget( "ZTIMEL", SHARED ) ;
		vcopy( "ZTIMEL", ztime, MOVE ) ;
		t = ( abnormalEnd ) ? " HAS ABENDED" : " HAS ENDED" ;
		notify( ztime.substr( 0, 8 ) + " JOB " + d2ds( taskid(), 5 ) + t ) ;
	}

	p_gls->release_enqueues_task( errblk.taskid ) ;

	t = ( backgrd ) ? " background " : " " ;
	llog( "I", "Shutting down"+ t +"application: " + zappname +" Taskid: " << taskId << endl ) ;

	terminateAppl    = true  ;
	applicationEnded = true  ;
	busyAppl         = false ;

	cond_mstr->notify_all() ;
}


void pApplication::wait_event( WAIT_REASON w )
{
	TRACE_FUNCTION() ;

	write_output() ;

	waiting_on = w ;
	busyAppl   = false ;

	boost::this_thread::sleep_for( boost::chrono::milliseconds( 1 ) ) ;

	cond_mstr->notify_all() ;

	while ( !busyAppl )
	{
		boost::mutex::scoped_lock lk( mutex ) ;
		cond_appl.wait( lk ) ;
		lk.unlock() ;
		if ( terminateAppl )
		{
			RC = 20 ;
			llog( "E", "Application terminating.  Cancelling wait_event" << endl ) ;
			abend() ;
		}
	}
	waiting_on = WAIT_NONE ;

	reload_keylist() ;
}


void pApplication::write_output()
{
	//
	// Write any re-directed cerr/cout output to the application log.
	//

	TRACE_FUNCTION() ;

	string text ;

	text = cerr_buffer.str() ;
	if ( text != "" )
	{
		llog( "E", text << endl ) ;
		cerr_buffer.clear() ;
		cerr_buffer.str( "" ) ;
	}

	text = cout_buffer.str() ;
	if ( text != "" )
	{
		llog( "O", text << endl ) ;
		cout_buffer.clear() ;
		cout_buffer.str( "" ) ;
	}
}


bool pApplication::isprimMenu()
{
	TRACE_FUNCTION() ;

	return ( currPanel ) ? currPanel->primaryMenu : false ;
}


void pApplication::remove_pd( pPanel* p )
{
	//
	// Remove the pulldown from the panel.
	//

	TRACE_FUNCTION() ;

	if ( valid_panel_addr( p ) )
	{
		p->remove_pd( errblk ) ;
	}
}


bool pApplication::valid_panel_addr( pPanel* p )
{
	//
	// Panel may have been deleted so check if it is still valid.
	//

	TRACE_FUNCTION() ;

	if ( p )
	{
		for ( auto it = panelList.begin() ; it != panelList.end() ; ++it )
		{
			if ( it->second == p )
			{
				return true ;
			}
		}
	}

	return false ;
}


void pApplication::get_home2( uint& row,
			      uint& col )
{
	TRACE_FUNCTION() ;

	if ( currPanel ) { currPanel->get_home2( row, col ) ; }
	else             { row = 0 ; col = 0                ; }
}


string pApplication::get_applid()
{
	TRACE_FUNCTION() ;

	return p_poolMGR->get( errblk, "ZAPPLID", SHARED ) ;
}


void pApplication::get_pcursor( uint& row,
				uint& col )
{
	TRACE_FUNCTION() ;

	if ( currPanel ) { currPanel->get_pcursor( row, col ) ; }
	else             { row = 0 ; col = 0                  ; }
}


void pApplication::get_lcursor( uint& row,
				uint& col )
{
	TRACE_FUNCTION() ;

	if ( currPanel ) { currPanel->get_lcursor( row, col ) ; }
	else             { row = 0 ; col = 0                  ; }
}


string pApplication::get_current_panelDesc()
{
	TRACE_FUNCTION() ;

	return ( currPanel ) ? currPanel->get_panelDesc() : "" ;
}


string pApplication::get_current_screenName()
{
	TRACE_FUNCTION() ;

	return upper( p_poolMGR->get( errblk, "ZSCRNAME", SHARED ) ) ;
}


string pApplication::get_panelid()
{
	TRACE_FUNCTION() ;

	return ( currPanel ) ? currPanel->panelid : "" ;
}


bool pApplication::inputInhibited()
{
	TRACE_FUNCTION() ;

	return ( currPanel ) ? currPanel->inputInhibited() : false ;
}


bool pApplication::msgInhibited()
{
	TRACE_FUNCTION() ;

	return ( currPanel ) ? currPanel->msgInhibited() : false ;
}


void pApplication::display_pd( errblock& err )
{
	TRACE_FUNCTION() ;

	if ( currPanel ) { currPanel->display_pd( err ) ; }
}


void pApplication::msgResponseOK()
{
	TRACE_FUNCTION() ;

	if ( currPanel ) { currPanel->msgResponseOK() ; }
}


bool pApplication::error_msg_issued()
{
	TRACE_FUNCTION() ;

	return ( currPanel ) ? currPanel->error_msg_issued() : false ;
}


void pApplication::set_userAction( USR_ACTIONS act )
{
	TRACE_FUNCTION() ;

	if ( currPanel ) { currPanel->set_userAction( act) ; }
	usr_action = act ;
}


string pApplication::get_command_buffer( errblock& err )
{
	TRACE_FUNCTION() ;

	err.clear() ;

	return ( currPanel && cbuffer != "" ) ? currPanel->getDialogueVar( err, cbuffer ) : "" ;
}


void pApplication::set_command_retbuf( errblock& err,
				       const string& rcmd )
{
	TRACE_FUNCTION() ;

	err.clear() ;

	if ( currPanel && rbuffer != "" ) { currPanel->putDialogueVar( err, rbuffer, rcmd ) ; }
}


bool pApplication::cmd_nonblank()
{
	//
	// Stop command stack execution if command line is non-blank, except in the
	// case the application has also issued a CONTROL NONDISPL to simulate END/ENTER.
	//

	TRACE_FUNCTION() ;

	return ( currPanel && !controlNonDispl ) ? currPanel->cmd_nonblank() : false ;
}


bool pApplication::end_pressed()
{
	TRACE_FUNCTION() ;

	return ( usr_action == USR_CANCEL ||
		 usr_action == USR_END    ||
		 usr_action == USR_EXIT   ||
		 usr_action == USR_RETURN ) ;
}


string pApplication::get_swb_name()
{
	TRACE_FUNCTION() ;

	string t = upper( p_poolMGR->get( errblk, "ZSCRNAME", SHARED ) ) ;

	return ( t == "" ) ? ( currPanel ) ? currPanel->panelid : "????????" : t ;
}


void pApplication::term_resize()
{
	//
	// Signal panels need to be reloaded after a terminal resize.
	//

	TRACE_FUNCTION() ;

	for ( auto& panel : panelList )
	{
		panel.second->term_resize = true ;
	}
}


bool pApplication::errorsReturn()
{
	TRACE_FUNCTION() ;

	return controlErrorsReturn ;
}


void pApplication::setDebugMode()
{
	//
	// Set debugging level 1.
	//

	TRACE_FUNCTION() ;

	debugMode = true ;
	errblk.setDebugMode( 1 ) ;
	p_poolMGR->put( errblk, zscrnum, "ZDEBUG", "1" ) ;
}


void pApplication::setDebugLevel( const string& dlevel )
{
	//
	// Set debugging mode and debugging level.
	// Debugging level of 0 is debugging off.
	//

	TRACE_FUNCTION() ;

	int lvl = min( 3, ds2d( dlevel ) ) ;

	if ( lvl == 0 )
	{
		clearDebugMode() ;
	}
	else
	{
		debugMode = true ;
		errblk.setDebugMode( lvl ) ;
		p_poolMGR->put( errblk, zscrnum, "ZDEBUG", d2ds( lvl, 2 ) ) ;
	}
}


void pApplication::clearDebugMode()
{
	//
	// Set debugging mode off and debugging level 0.
	//

	TRACE_FUNCTION() ;

	debugMode = false ;
	errblk.clearDebugMode() ;
	p_poolMGR->put( errblk, zscrnum, "ZDEBUG", "0" ) ;
}


bool pApplication::zhelppgm_home()
{
	//
	// Don't set the cursor to the home position if it is the help program that has been started
	// via a pfkey.
	//

	TRACE_FUNCTION() ;

	return ( !tutorial || !pfkey ) ;
}


void pApplication::redraw_panel( errblock& err )
{
	TRACE_FUNCTION() ;

	if ( currPanel ) { currPanel->redraw_panel( err ) ; }
}


void pApplication::store_Zvars()
{
	//
	// Store various SHARED pool variables.
	//

	TRACE_FUNCTION() ;

	zscrname = p_poolMGR->get( errblk, "ZSCRNAME", SHARED ) ;
	zstored  = true ;
}


void pApplication::restore_Zvars()
{
	//
	// Restore various SHARED pool variables after stacked or called application has terminated.
	//
	// ZKLNAME
	// ZKLAPPL
	// ZKLTYPE
	// ZPRIM
	// ZSCRNAME
	//

	TRACE_FUNCTION() ;

	if ( currPanel )
	{
		currPanel->update_keylist_vars( errblk ) ;
		currPanel->update_zprim_var( errblk ) ;
	}

	if ( p_poolMGR->get( errblk, zscrnum, "ZSCRNAM2" ) == "PERM" )
	{
		zscrname = p_poolMGR->get( errblk, zscrnum, "ZSCRNAME" ) ;
		zstored  = true ;
	}

	if ( zstored )
	{
		p_poolMGR->put( errblk, "ZSCRNAME", zscrname, SHARED ) ;
	}
}


void pApplication::display_id()
{
	TRACE_FUNCTION() ;

	if ( currPanel ) { currPanel->display_id( errblk ) ; }
}


void pApplication::build_pfkeys( bool force_build )
{
	TRACE_FUNCTION() ;

	if ( currPanel ) { currPanel->build_pfkeys( errblk, force_build ) ; }
}


void pApplication::display_pfkeys()
{
	TRACE_FUNCTION() ;

	if ( currPanel ) { currPanel->display_pfkeys( errblk ) ; }
}


void pApplication::set_nondispl_enter()
{
	TRACE_FUNCTION() ;

	controlNonDispl    = true  ;
	controlNonDisplEnd = false ;
}


void pApplication::set_nondispl_end()
{
	TRACE_FUNCTION() ;

	controlNonDispl    = true ;
	controlNonDisplEnd = true ;
}


void pApplication::clr_nondispl()
{
	TRACE_FUNCTION() ;

	controlNonDispl    = false ;
	controlNonDisplEnd = false ;
}


void pApplication::set_msg( const string& msg_id )
{
	//
	// Display a message on current panel using msg_id.
	//

	TRACE_FUNCTION() ;

	if ( !currPanel ) { return ; }

	get_message( msg_id ) ;
	if ( RC == 0 )
	{
		currPanel->clear_msgloc() ;
		currPanel->set_panel_msg( zmsg, zmsgid ) ;
		currPanel->display_msg( errblk ) ;
	}
}


void pApplication::set_msg1( const slmsg& t,
			     const string& msgid )
{
	//
	// Propagate setmsg() to the current application using the short-long-message object.
	// zmsg1 and zmsgid1 are used to store messages issued by the setmsg() service after variable substitution.
	//

	TRACE_FUNCTION() ;

	zmsg1      = t ;
	zmsgid1    = msgid ;
	setMessage = true ;
}


void pApplication::display_setmsg()
{
	//
	// Display the message stored by the setmsg() service.
	//
	// Ignore message location if it does not exist on the panel.
	//

	TRACE_FUNCTION() ;

	const string e1 = "SETMSG Service Error" ;

	if ( setMessage && currPanel )
	{
		if ( currPanel->field_exists( zmsg1.msgloc ) )
		{
			currPanel->set_msgloc( errblk, zmsg1.msgloc ) ;
			CHECK_ERROR_SETCALL_RETURN( e1 )
		}
		else
		{
			currPanel->clear_msgloc() ;
		}
		currPanel->set_panel_msg( zmsg1, zmsgid1 ) ;
		currPanel->display_msg( errblk ) ;
		setMessage = false ;
	}
}


void pApplication::clear_msg()
{
	TRACE_FUNCTION() ;

	if ( currPanel ) { currPanel->clear_msg() ; }
}


bool pApplication::show_help_member()
{
	TRACE_FUNCTION() ;

	return ( !setMessage && ( currPanel->get_msg() == "" || currPanel->showLMSG || currPanel->MSG.lmsg == "" ) ) ;
}


void pApplication::save_errblock()
{
	TRACE_FUNCTION() ;

	serblk = errblk ;
	errblk.clear() ;
	errblk.setServiceCall() ;
}


void pApplication::restore_errblock()
{
	TRACE_FUNCTION() ;

	errblk = serblk ;
}


bool pApplication::nretriev_on()
{
	TRACE_FUNCTION() ;

	return ( currPanel ) ? currPanel->get_nretriev() : false ;
}


const string& pApplication::get_nretfield()
{
	TRACE_FUNCTION() ;

	return ( currPanel ) ? currPanel->get_nretfield() : nullstr ;
}


const string& pApplication::get_history_fields()
{
	TRACE_FUNCTION() ;

	return ( currPanel ) ? currPanel->get_history_fields() : nullstr ;
}


void pApplication::toggle_fscreen()
{
	TRACE_FUNCTION() ;

	currPanel->toggle_fscreen( errblk, addpop_active, addpop_row, addpop_col ) ;
	currPanel->display_panel( errblk, false ) ;
}


void pApplication::set_addpop( pApplication* p )
{
	TRACE_FUNCTION() ;

	addpop_row    = p->addpop_row ;
	addpop_col    = p->addpop_col ;
	addpop_active = p->addpop_active ;
}


void pApplication::get_addpop( uint& row,
			       uint& col )
{
	TRACE_FUNCTION() ;

	row = addpop_row ;
	col = addpop_col ;
}


void pApplication::set_zlibd_altlib( bool passlib,
				     pApplication* p )
{
	//
	// Copy forward the LIBDEF and ALTLIB libraries:
	//   For PASSLIB only make the current set available.  Any changes are lost when
	//     the application terminates.
	//   For NEWAPPL, the libraries are not available to the new application.
	//   For neither, make the current set available and any changes are kept after
	//     the application terminates.
	//
	// Only called for PASSLIB or NEWAPPL = null ie.  No isolation.
	//
	// For ALTLIB where the changes are not passed back after program termination,
	// set tofree to false so we don't free allocations needed when original ALTLIB
	// is restored.  Allocations done during this application will be freed on
	// program termination.
	//

	TRACE_FUNCTION() ;

	const char user = 'U' ;
	const char appl = 'A' ;
	const char syst = 'S' ;

	pair<map<char, stack<zaltlib>>::iterator, bool> result ;

	if ( !passlib )
	{
		zlibd = p->zlibd ;
		zaltl = p->zaltl ;
	}
	else
	{
		for ( auto it = p->zlibd.begin() ; it != p->zlibd.end() ; ++it )
		{
			zlibd[ it->first ].push( it->second.top() ) ;
		}
		zaltl.clear() ;
		auto itu = p->zaltl.find( user ) ;
		auto ita = p->zaltl.find( appl ) ;
		auto its = p->zaltl.find( syst ) ;
		if ( itu != p->zaltl.end() )
		{
			result = zaltl.insert( pair<char, stack<zaltlib>>( user, stack<zaltlib>() ) ) ;
			result.first->second.push( zaltlib( itu->second.top().paths, itu->second.top().ddname, false ) ) ;
		}
		if ( ita != p->zaltl.end() )
		{
			result = zaltl.insert( pair<char, stack<zaltlib>>( appl, stack<zaltlib>() ) ) ;
			result.first->second.push( zaltlib( ita->second.top().paths, ita->second.top().ddname, false ) ) ;
		}
		if ( its != p->zaltl.end() )
		{
			result = zaltl.insert( pair<char, stack<zaltlib>>( syst, stack<zaltlib>() ) ) ;
			result.first->second.push( zaltlib( its->second.top().paths, its->second.top().ddname, false ) ) ;
		}
	}
}


void pApplication::set_zlibd_altlib( pApplication* p )
{
	//
	// Copy back the LIBDEF and ALTLIB libraries on program termination.
	//
	// Only called for NEWAPPL = null ie.  No isolation and changed libraries are passed back.
	//

	TRACE_FUNCTION() ;

	zlibd = p->zlibd ;
	zaltl = p->zaltl ;
}


pPanel* pApplication::create_panel( const string& p_name )
{
	//
	// Create a new panel object and load from source.
	//
	// If running in DEBUG mode or the panel has requested a reload, create a new panel
	// object each time and delete the old one.
	//
	// Don't keep too many panels or they may give storage problems.
	//
	// If reloading panel, address may be stored in the popup structure and panel_data so adjust.
	//

	TRACE_FUNCTION() ;

	const string e1 = "Error creating panel " + p_name ;

	pPanel* o_panel = nullptr ;

	errblk.setRC( 0 ) ;

	if ( !isvalidName( p_name ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE021A", p_name ) ;
		checkRCode( errblk ) ;
		return nullptr ;
	}

	if ( panelList.size() > MAX_PANELS )
	{
		for ( auto it = panelList.begin() ; it != panelList.end() ; )
		{
			if ( it->second != currPanel && it->second != currtbPanel )
			{
				adjust_popups( it->second ) ;
				delete it->second ;
				it = panelList.erase( it ) ;
			}
			else
			{
				++it ;
			}
		}
	}

	auto it = panelList.find( p_name ) ;
	if ( it != panelList.end() )
	{
		if ( debugMode || it->second->reload() )
		{
			o_panel = it->second ;
			panelList.erase( it ) ;
		}
		else
		{
			return it->second ;
		}
	}

	pPanel* p_panel = new pPanel( errblk,
				      funcPool,
				      (void *)this,
				      controlPassLRScroll,
				      selPanel,
				      tutorial,
				      zscrnum,
				      zdatef ) ;
	if ( errblk.error() )
	{
		if ( o_panel )
		{
			if ( currPanel   == o_panel ) { currPanel   = nullptr ; }
			if ( currtbPanel == o_panel ) { currtbPanel = nullptr ; }
			adjust_popups( o_panel ) ;
			delete o_panel ;
		}
		delete p_panel ;
		errblk.setcall( e1 ) ;
		checkRCode( errblk ) ;
		return nullptr ;
	}

	p_panel->load_panel( errblk,
			     p_name,
			     get_libdef_search_paths( s_ZPLIB ) ) ;
	if ( errblk.RC0() )
	{
		if ( o_panel )
		{
			if ( currPanel   == o_panel ) { currPanel   = p_panel ; }
			if ( currtbPanel == o_panel ) { currtbPanel = p_panel ; }
			adjust_popups( o_panel, p_panel ) ;
			delete o_panel ;
		}
		panelList[ p_name ] = p_panel ;
		load_keylist( p_panel ) ;
	}
	else
	{
		if ( o_panel )
		{
			if ( currPanel   == o_panel ) { currPanel   = nullptr ; }
			if ( currtbPanel == o_panel ) { currtbPanel = nullptr ; }
			adjust_popups( o_panel ) ;
			delete o_panel ;
		}
		delete p_panel ;
		p_panel = nullptr ;
		errblk.setcall( e1 ) ;
		checkRCode( errblk ) ;
	}
	errblk.clearsrc() ;

	return p_panel ;
}


void pApplication::adjust_popups( pPanel* p1,
				  pPanel* p2 )
{
	//
	// If we are deleting a panel contained in a popup structure, remove or adjust the address.
	// If updating the popup, also update the panel_userdata to contain the correct pPanel address.
	//
	// If p2 is null, remove popup entry from the popups stack.  This will cause the popup to dispappear from
	// the screen (dummy popups entry still required for REMPOP).
	// Otherwise replace p1 with p2.
	//

	TRACE_FUNCTION() ;

	if ( popups.empty() )
	{
		return ;
	}

	const void* vptr ;

	stack<popup> temp ;

	while ( !popups.empty() )
	{
		temp.push( popups.top() ) ;
		popups.pop() ;
	}

	while ( !temp.empty() )
	{
		popup& pop = temp.top() ;
		if ( pop.panl == p1 )
		{
			if ( p2 )
			{
				pop.panl = p2 ;
				popups.push( pop ) ;
				vptr = panel_userptr( pop.pan1 ) ;
				if ( vptr )
				{
					const panel_data* pd = static_cast<const panel_data*>(vptr) ;
					pd->update( p2 ) ;
				}
				vptr = panel_userptr( pop.pan2 ) ;
				if ( vptr )
				{
					const panel_data* pd = static_cast<const panel_data*>(vptr) ;
					pd->update( p2 ) ;
				}
			}
			else
			{
				PANEL* pan1 = pop.pan1 ;
				PANEL* pan2 ;
				if ( pan1 )
				{
					WINDOW* w = panel_window( pan1 ) ;
					vptr = panel_userptr( pan1 ) ;
					if ( vptr )
					{
						delete static_cast<const panel_data*>(vptr) ;
					}
					del_panel( pan1 ) ;
					delwin( w ) ;
					pan2 = pop.pan2 ;
					w    = panel_window( pan2 ) ;
					vptr = panel_userptr( pan2 ) ;
					if ( vptr )
					{
						delete static_cast<const panel_data*>(vptr) ;
					}
					del_panel( pan2 ) ;
					delwin( w ) ;
					update_panels() ;
				}
				popups.push( popup() ) ;
			}
		}
		else
		{
			popups.push( pop ) ;
		}
		temp.pop() ;
	}
}


/* *********************************************** ***************************** ************************************************ */
/* ***********************************************     Start of DM Services      ************************************************ */
/* *********************************************** ***************************** ************************************************ */


void pApplication::addpop( const string& a_fld,
			   int a_row,
			   int a_col )
{
	//
	// Create pop-up window and set row/col for the next panel display.
	// If addpop() is already active, store old values for next rempop().
	//
	// Position of addpop is relative to row=1, col=3 or the previous addpop() position for this logical screen.
	// Defaults are 0,0 giving row=1, col=3.
	//
	// RC =  0 Normal completion.
	// RC = 12 No panel displayed before addpop() service when using field parameter.
	// RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "ADDPOP Service Error" ;

	popup p ;

	uint p_row = 0 ;
	uint p_col = 0 ;

	RC = 0 ;

	if ( a_fld != "" )
	{
		if ( !currPanel )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022L", 12 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		if ( !currPanel->field_get_row_col( a_fld, p_row, p_col ) )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022M", a_fld, currPanel->panelid, 20 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		a_row += p_row ;
		a_col += p_col - 3 ;
	}

	if ( addpop_active )
	{
		if ( currPanel )
		{
			currPanel->create_panels( p ) ;
		}
		popups.push( p.set( addpop_row, addpop_col ) ) ;
		a_row += addpop_row ;
		a_col += addpop_col ;
	}

	addpop_active = true ;
	addpop_row    = ( a_row <  0 ) ? 1 : a_row + 2 ;
	addpop_col    = ( a_col < -1 ) ? 2 : a_col + 4 ;

	if ( currPanel )
	{
		currPanel->show_popup() ;
	}
}


void pApplication::browse( const string& m_file,
			   const string& m_panel,
			   const string& m_dataid,
			   int m_reclen )
{
	TRACE_FUNCTION() ;

	const string e1 = "BROWSE Service Error" ;

	browse_parms b ;

	if ( isvalidName( upper( m_file ) ) )
	{
		vcopy( upper( m_file ), b.browse_file, MOVE ) ;
	}
	else
	{
		b.browse_file = m_file ;
	}

	if ( m_dataid != "" )
	{
		b.browse_file = get_file_for_dataid( m_dataid ) ;
		if ( b.browse_file == "" )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE054I", m_dataid ) ;
			checkRCode( errblk ) ;
			return ;
		}
	}

	b.browse_panel  = m_panel ;
	b.browse_reclen = m_reclen ;

	selct.clear() ;
	selct.pgm     = p_poolMGR->get( errblk, "ZBRPGM", PROFILE ) ;
	selct.parm    = "" ;
	selct.newappl = "" ;
	selct.newpool = false ;
	selct.passlib = false ;
	selct.suspend = true  ;
	selct.options = (void*)&b ;
	selct.ptid    = ptid  ;
	selct.stid    = taskId ;
	selct.service = true  ;
	selct.errret  = controlErrorsReturn ;
	selct.backgrd = backgrd ;
	actionSelect() ;

	RC = ZRC ;
	if ( ZRC > 11 && isvalidName( ZRESULT ) )
	{
		errblk.setcall( TRACE_INFO(), "BROWSE Error", ZRESULT, get_shared_var( "ZVAL1" ), ZRC ) ;
		checkRCode( errblk ) ;
	}
}


void pApplication::control( const string& parm1,
			    const string& parm2,
			    const string& parm3 )
{
	//
	// CONTROL ERRORS CANCEL - abend for RC >= 12.
	// CONTROL ERRORS RETURN - return to application for any RC.
	//
	// CONTROL DISPLAY SAVE/RESTORE - SAVE/RESTORE status for a TBDISPL.
	//         SAVE/RESTORE saves/restores the function pool variables associated with a tb display
	//         (the six ZTD* variables and the .ZURID.ln variables), and also the currtbPanel
	//         pointer for retrieving other pending sets via a tbdispl with no panel specified.
	//         Also restore table field function pool variables in pool_2.
	//         Only necessary if a tbdispl invokes another tbdispl within the same task.
	//
	// CONTROL SPLIT  DISABLE - RC=8 if screen already split.
	//
	// CONTROL PASSTHRU LRSCROLL  PASON | PASOFF | PASQUERY.
	//
	// CONTROL REFLIST UPDATE.
	// CONTROL REFLIST NOUPDATE.
	//
	// lspf extensions:
	// ----------------
	//
	// CONTROL CUA      RELOAD   - Reload the CUA tables.
	// CONTROL CUA      NORELOAD - Stop reloading the CUA tables.
	//
	// CONTROL ABENDRTN DEFAULT  - Reset abend routine to the default, pApplication::cleanup_default.
	//
	// CONTROL REFLIST  ON       - REFLIST retrieve is on for this application.  ZRESULT will replace field.
	// CONTROL REFLIST  OFF      - REFLIST retrieve is off for this application.
	//
	// CONTROL HISTORY  UPDATE   - Update field history for this application.
	// CONTROL HISTORY  NOUPDATE - Don't update field history for this application.
	//
	// CONTROL NOTIFY   JOBEND   - Send notify message when background job ends.
	//
	// CONTROL SIGTERM  DEFAULT  - Reset default SIGTERM signal handler routine, pApplication::handle_sigterm.
	// CONTROL SIGTERM  IGNORE   - Ignore SIGTERM signal
	// CONTROL SIGUSR1  DEFAULT  - Reset default SIGUSR1 signal handler routine, pApplication::handle_sigusr1.
	// CONTROL SIGUSR1  IGNORE   - Ignore SIGUSR1 signal
	//
	// CONTROL ERRORS   STATUS   - RC=0 for ERRORS CANCEL
	//                           - RC=4 for ERRORS RETURN
	//

	TRACE_FUNCTION() ;

	int i ;

	const string e1 = "CONTROL Service Error" ;

	errblk.setRC( 0 ) ;

	if ( parm3 != "" && parm1 != "PASSTHRU" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022V", parm1 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	auto itc = controls.find( parm1 ) ;
	if ( itc == controls.end() )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022Y", parm1 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	switch ( itc->second )
	{
	case CTL_CUA:
		if ( parm2 == "RELOAD" )
		{
			reloadCUATables = true ;
		}
		else if ( parm2 == "NORELOAD" )
		{
			reloadCUATables = false ;
		}
		else
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022X", "CUA", parm2 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		break ;

	case CTL_DISPLAY:
		if ( parm2 == "LOCK" )
		{
			controlDisplayLock = true ;
		}
		else if ( parm2 == "REFRESH" )
		{
			refreshlScreen = true ;
		}
		else if ( parm2 == "SAVE" )
		{
			stk_int.push( funcPool->get1( errblk, 0, INTEGER, "ZTDDEPTH" ) ) ;
			stk_int.push( funcPool->get1( errblk, 0, INTEGER, "ZTDROWS"  ) ) ;
			stk_int.push( funcPool->get1( errblk, 0, INTEGER, "ZTDSELS"  ) ) ;
			stk_int.push( funcPool->get1( errblk, 0, INTEGER, "ZTDTOP"   ) ) ;
			stk_int.push( funcPool->get1( errblk, 0, INTEGER, "ZTDVROWS" ) ) ;
			tbpanel_stk.push( currtbPanel ) ;
			if ( currtbPanel && currtbPanel->tb_rdepth > 0 )
			{
				urid_stk.push( stack<string>() ) ;
				stack<string>* ptr_stk = &urid_stk.top() ;
				for ( i = 0 ; i < currtbPanel->tb_rdepth ; ++i )
				{
					ptr_stk->push( funcPool->get3( errblk, ".ZURID."+d2ds( i ) ) ) ;
				}
			}
		}
		else if ( parm2 == "RESTORE" )
		{
			if ( tbpanel_stk.empty() )
			{
				errblk.setcall( TRACE_INFO(), e1, "PSYE022W" ) ;
				checkRCode( errblk ) ;
				return ;
			}
			funcPool->put2( errblk, "ZTDVROWS", stk_int.top() ) ;
			stk_int.pop() ;
			funcPool->put2( errblk, "ZTDTOP", stk_int.top() ) ;
			stk_int.pop() ;
			funcPool->put2( errblk, "ZTDSELS", stk_int.top() ) ;
			stk_int.pop() ;
			funcPool->put2( errblk, "ZTDROWS", stk_int.top() ) ;
			stk_int.pop() ;
			funcPool->put2( errblk, "ZTDDEPTH", stk_int.top() ) ;
			stk_int.pop() ;
			currtbPanel = tbpanel_stk.top() ;
			tbpanel_stk.pop() ;
			if ( !valid_panel_addr( currtbPanel ) )
			{
				currtbPanel = nullptr ;
			}
			else
			{
				if ( currtbPanel->tb_rdepth > 0 && !urid_stk.empty() )
				{
					stack<string>* ptr_stk = &urid_stk.top() ;
					i = ptr_stk->size() ;
					while ( !ptr_stk->empty() )
					{
						funcPool->put3( ".ZURID."+d2ds( --i ), ptr_stk->top() ) ;
						ptr_stk->pop() ;
					}
					urid_stk.pop() ;
				}
				currtbPanel->restore() ;
			}
		}
		else
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022X", "DISPLAY", parm2 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		break ;

	case CTL_HISTORY:
		if ( parm2 == "UPDATE" )
		{
			p_poolMGR->put( errblk, zscrnum, "ZFHUPDT", "Y" ) ;
		}
		else if ( parm2 == "NOUPDATE" )
		{
			p_poolMGR->put( errblk, zscrnum, "ZFHUPDT", "N" ) ;
		}
		else
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022X", "HISTORY", parm2 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		break ;

	case CTL_NONDISPL:
		if ( parm2 == "ENTER" || parm2 == "" )
		{
			set_nondispl_enter() ;
		}
		else if ( parm2 == "END" )
		{
			set_nondispl_end() ;
		}
		else
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022X", "NONDISPL", parm2 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		break ;

	case CTL_ERRORS:
		if ( parm2 == "RETURN" )
		{
			controlErrorsReturn = true ;
		}
		else if ( parm2 == "CANCEL" )
		{
			controlErrorsReturn = false ;
		}
		else if ( parm2 == "STATUS" )
		{
			errblk.setRC( ( controlErrorsReturn ) ? 4 : 0 ) ;
		}
		else
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022X", "ERRORS", parm2 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		break ;

	case CTL_NOTIFY:
		if ( parm2 == "JOBEND" )
		{
			if ( backgrd ) { notifyEnded = true ; }
		}
		else
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022X", "NOTIFY", parm2 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		break ;

	case CTL_PASSTHRU:
		if ( parm2 == "LRSCROLL" )
		{
			if ( parm3 == "PASON" )
			{
				controlPassLRScroll = true ;
				for ( auto& panel : panelList )
				{
					panel.second->lrScroll = true ;
				}
			}
			else if ( parm3 == "PASOFF" )
			{
				controlPassLRScroll = false ;
				for ( auto& panel : panelList )
				{
					panel.second->lrScroll = false ;
				}
			}
			else if ( parm3 == "PASQUERY" )
			{
				errblk.setRC( ( controlPassLRScroll ) ? 1 : 0 ) ;
			}
			else
			{
				errblk.setcall( TRACE_INFO(), e1, "PSYE022X", "PASSTHRU LRSCROLL", parm3 ) ;
				checkRCode( errblk ) ;
				return ;
			}
		}
		else
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022X", "PASSTHRU", parm2 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		break ;

	case CTL_REFLIST:
		if ( parm2 == "UPDATE" )
		{
			p_poolMGR->put( errblk, zscrnum, "ZREFUPDT", "Y" ) ;
		}
		else if ( parm2 == "NOUPDATE" )
		{
			p_poolMGR->put( errblk, zscrnum, "ZREFUPDT", "N" ) ;
		}
		else if ( parm2 == "ON" )
		{
			reffield = "#REFLIST" ;
		}
		else if ( parm2 == "OFF" )
		{
			reffield = "" ;
		}
		else
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022X", "REFLIST", parm2 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		break ;

	case CTL_SPLIT:
		if ( parm2 == "ENABLE" )
		{
			controlSplitEnable = true ;
		}
		else if ( parm2 == "DISABLE" )
		{
			if ( p_poolMGR->get( errblk, "ZSPLIT", SHARED ) == "YES" )
			{
				errblk.setRC( 8 ) ;
			}
			else
			{
				controlSplitEnable = false ;
			}
		}
		else
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022X", "SPLIT", parm2 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		break ;

	case CTL_ABENDRTN:
		if ( parm2 == "DEFAULT" )
		{
			pcleanup = &pApplication::cleanup_default ;
		}
		else
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022X", "ABENDTRN", parm2 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		break ;

	case CTL_SIGTERM:
		if ( parm2 == "DEFAULT" )
		{
			psigterm = &pApplication::handle_sigterm ;
			lsig_action.sa_handler = handle_signal ;
			sigaction( SIGTERM, &lsig_action, nullptr ) ;
		}
		else if ( parm2 == "IGNORE" )
		{
			signal( SIGTERM, SIG_IGN ) ;
			lsig_action.sa_handler = nullptr ;
		}
		else
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022X", "ABENDTRN", parm2 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		break ;

	case CTL_SIGUSR1:
		if ( parm2 == "DEFAULT" )
		{
			psigusr1 = &pApplication::handle_sigusr1 ;
			lsig_action.sa_handler = handle_signal ;
			sigaction( SIGUSR1, &lsig_action, nullptr ) ;
		}
		else if ( parm2 == "IGNORE" )
		{
			signal( SIGUSR1, SIG_IGN ) ;
			lsig_action.sa_handler = nullptr ;
		}
		else
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022X", "ABENDTRN", parm2 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		break ;
	}

	RC = errblk.getRC() ;
}


void pApplication::control( const string& parm1,
			    void (pApplication::*pFunc)() )
{
	//
	// lspf extensions:
	//
	// CONTROL ABENDRTN ptr_to_routine - Set the routine to get control during an abend.
	// CONTROL SIGTERM  ptr_to_routine - Set the routine to get control on a SIGTERM signal.
	// CONTROL SIGUSR1  ptr_to_routine - Set the routine to get control on a SIGUSR1 signal.
	//

	TRACE_FUNCTION() ;

	const string e1 = "CONTROL Service Error" ;

	errblk.setRC( 0 ) ;

	auto itc = controls.find( parm1 ) ;
	if ( itc == controls.end() )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022Y", parm1 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	switch ( itc->second )
	{
	case CTL_ABENDRTN:
		pcleanup = pFunc ;
		break ;

	case CTL_SIGTERM:
		psigterm = pFunc ;
		lsig_action.sa_handler = handle_signal ;
		sigaction( SIGTERM, &lsig_action, nullptr ) ;
		break ;

	case CTL_SIGUSR1:
		psigusr1 = pFunc ;
		lsig_action.sa_handler = handle_signal ;
		sigaction( SIGUSR1, &lsig_action, nullptr ) ;
		break ;
	}

	RC = errblk.getRC() ;
}


void pApplication::dsinfo( const string& file )
{
	//
	// Get file information.
	//
	// RC =  0  Normal completion.
	// RC =  8  User requested information unavailable.
	// RC = 12  Error obtaining file information.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	string zdsorg ;
	string zdsblk ;
	string zdsdsnt ;
	string zdsused ;
	string zdslink ;
	string zdsowner ;
	string zdsgroup ;
	string zdscdate ;
	string zdsmdate ;
	string zdsadate ;
	string zdsmajor ;
	string zdsminor ;
	string zdsinode ;

	string irlnk ;

	struct stat results ;

	struct tm* time_info ;

	char buf[ 20 ] ;
	char* buffer   ;

	size_t bufferSize = 255 ;

	try
	{
		if ( !exists( file ) )
		{
			throw runtime_error( "File does not exist" ) ;
		}
		zdsdsnt = file ;
		if ( lstat( file.c_str(), &results ) == 0 )
		{
			zdsorg = ( S_ISDIR( results.st_mode )  ) ? "DIR"    :
				 ( S_ISREG( results.st_mode )  ) ? "FILE"   :
				 ( S_ISCHR( results.st_mode )  ) ? "CHAR"   :
				 ( S_ISBLK( results.st_mode )  ) ? "BLOCK"  :
				 ( S_ISFIFO( results.st_mode ) ) ? "FIFO"   :
				 ( S_ISSOCK( results.st_mode ) ) ? "SOCKET" :
				 ( S_ISLNK( results.st_mode )  ) ? "SYML"   : "" ;

			zdsused = d2ds( results.st_size ) ;
			time_info = gmtime( &(results.st_ctime) ) ;
			strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ; zdscdate = buf ;

			time_info = gmtime( &(results.st_mtime) ) ;
			strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ; zdsmdate = buf ;

			time_info = gmtime( &(results.st_atime) ) ;
			strftime( buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", time_info ) ; zdsadate = buf ;

			zdsinode = d2ds( results.st_ino ) ;

			struct passwd* pw = getpwuid( results.st_uid ) ;
			if ( pw )
			{
				zdsowner = pw->pw_name ;
			}

			struct group* gr = getgrgid( results.st_gid ) ;
			if ( gr )
			{
				zdsgroup = gr->gr_name ;
			}

			zdsmajor = d2ds( major( results.st_dev ) ) ;
			zdsminor = d2ds( minor( results.st_dev ) ) ;
			zdsblk   = d2ds( results.st_blksize ) ;

			irlnk = "" ;
			if ( S_ISLNK( results.st_mode ) )
			{
				while ( true )
				{
					buffer = new char[ bufferSize ] ;
					int rc = readlink( file.c_str(), buffer, bufferSize ) ;
					if ( rc == -1 )
					{
						delete[] buffer ;
						if ( errno == ENAMETOOLONG ) { bufferSize += 255; }
						else                         { break ;            }
					}
					else
					{
						irlnk = string( buffer, rc ) ;
						delete[] buffer ;
						zdslink = ( irlnk.front() != '/' ) ?
							    file.substr( 0, file.find_last_of( '/' ) + 1 ) + irlnk : irlnk ;
						break ;
					}
				}
			}
		}
		else
		{
			throw runtime_error( "lstat gave a non-zero return code" ) ;
		}
	}
	catch ( runtime_error& e )
	{
		RC = 8 ;
		return ;
	}
	catch (...)
	{
		errblk.seterrid( TRACE_INFO(), "PSYS012C", "Unknown error occured" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	vreplace( "ZDSORG",   zdsorg ) ;
	vreplace( "ZDSBLK",   zdsblk ) ;
	vreplace( "ZDSUSED",  zdsused ) ;
	vreplace( "ZDSDSNT",  zdsdsnt ) ;
	vreplace( "ZDSLINK",  zdslink ) ;
	vreplace( "ZDSCDATE", zdscdate ) ;
	vreplace( "ZDSMDATE", zdsmdate ) ;
	vreplace( "ZDSADATE", zdsadate ) ;
	vreplace( "ZDSMAJOR", zdsmajor ) ;
	vreplace( "ZDSMINOR", zdsminor ) ;
	vreplace( "ZDSOWNER", zdsowner ) ;
	vreplace( "ZDSGROUP", zdsgroup ) ;
	vreplace( "ZDSINODE", zdsinode ) ;
}


void pApplication::display( string p_name,
			    const string& p_msg,
			    const string& p_cursor,
			    int   p_curpos,
			    const string& p_buffer,
			    const string& p_retbuf,
			    const string& p_msgloc )
{
	//
	// Display panel.
	//
	// If no panel name has been passed, default to the previous panel name and only perform )REINIT processing.
	//
	// RC =  0  Normal completion.
	// RC =  8  END or RETURN entered.
	// RC = 12  Panel, message, message location field or cursor field not found.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "Error during DISPLAY of panel " + p_name ;
	const string e2 = "Error processing )INIT section of panel "   ;
	const string e3 = "Error processing )REINIT section of panel " ;
	const string e4 = "Error processing )PROC section of panel "   ;
	const string e5 = "Error during update of panel " ;
	const string e6 = "Error updating field values of panel " ;
	const string e7 = "Error processing )ATTR section of panel " ;
	const string e8 = "Background job attempted to display panel " ;

	bool doReinit = false ;

	RC = 0 ;

	if ( !busyAppl )
	{
		llog( "E", "Invalid method state" <<endl ) ;
		llog( "E", "Application is in a wait state.  Method cannot invoke display services." <<endl ) ;
		RC = 20 ;
		return ;
	}

	if ( backgrd )
	{
		llog( "B", e8 + p_name <<endl ) ;
		RC = 8 ;
		return ;
	}

	if ( currPanel )
	{
		currPanel->hide_popup() ;
	}

	if ( p_name == "" )
	{
		if ( !currPanel )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE021C" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		p_name   = currPanel->panelid ;
		doReinit = true ;
	}

	if ( p_cursor != "" && !isvalidName( p_cursor ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE023I", p_cursor ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( p_buffer != "" && !isvalidName( p_buffer ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE023Q", p_buffer ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( p_retbuf != "" && !isvalidName( p_retbuf ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE023R", p_retbuf ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( p_msgloc != "" && !isvalidName( p_msgloc ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE031L", p_msgloc, "MSGLOC()" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	cbuffer = p_buffer ;
	rbuffer = p_retbuf ;

	currPanel = create_panel( p_name ) ;
	if ( errblk.error() ) { return ; }

	if ( cbuffer != "" )
	{
		currPanel->getDialogueVar( errblk, cbuffer ) ;
		CHECK_ERROR_SETCALL_RETURN( e1 )
		if ( errblk.RC8() )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE021G", cbuffer ) ;
			checkRCode( errblk ) ;
			return ;
		}
	}

	if ( propagateEnd )
	{
		if ( currPanel && currPanel->panelid == get_old_panelName() )
		{
			propagateEnd = false ;
		}
		else
		{
			set_nondispl_enter() ;
		}
	}

	if ( !doReinit )
	{
		currPanel->init_control_variables() ;
	}

	currPanel->set_msg( p_msg ) ;
	currPanel->set_cursor( p_cursor, p_curpos ) ;

	currPanel->set_msgloc( errblk, p_msgloc ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	currPanel->set_popup( errblk, addpop_active, addpop_row, addpop_col ) ;

	p_poolMGR->put( errblk, "ZPANELID", p_name, SHARED, SYSTEM ) ;

	usr_action = USR_ENTER ;

	if ( doReinit )
	{
		currPanel->display_panel_reinit( errblk ) ;
		CHECK_ERROR_SETCALL_RETURN( e3 + p_name )
		set_ZVERB_panel_resp_re_init() ;
		set_panel_zvars() ;
	}
	else
	{
		currPanel->display_panel_init( errblk ) ;
		CHECK_ERROR_SETCALL_RETURN( e2 + p_name )
		currPanel->display_panel_attrs( errblk ) ;
		CHECK_ERROR_SETCALL_RETURN( e7 + p_name )
		set_ZVERB_panel_resp_re_init() ;
		set_panel_zvars() ;
		currPanel->update_field_values( errblk ) ;
		CHECK_ERROR_SETCALL_RETURN( e6 + p_name )
		RC = errblk.getRC() ;
	}

	if ( currPanel->get_msg() != "" )
	{
		get_message( currPanel->get_msg() ) ;
		if ( RC > 0 ) { return ; }
		currPanel->set_panel_msg( zmsg, zmsgid ) ;
	}

	set_screenName() ;
	set_panelName( p_name ) ;

	while ( true )
	{
		currPanel->cursor_placement_display( errblk ) ;
		CHECK_ERROR_SETCALL_RETURN( e1 )
		currPanel->display_panel( errblk ) ;
		CHECK_ERROR_SETCALL_RETURN( e1 )
		if ( lineInOutDone && !controlNonDispl )
		{
			refreshlScreen = true ;
			wait_event( WAIT_USER ) ;
			lineInOutDone  = false ;
			refreshlScreen = false ;
		}
		else if ( !propagateEnd )
		{
			update_panels() ;
		}
		if ( currPanel->get_msg() == "" && !controlNonDispl )
		{
			currPanel->clear_msg() ;
		}
		if ( !propagateEnd )
		{
			set_ZVERB( "" ) ;
		}
		wait_event( WAIT_USER ) ;
		if ( usr_action == USR_RETURN )
		{
			propagateEnd = true ;
		}
		controlDisplayLock = false ;
		refreshlScreen     = false ;
		reloadCUATables    = false ;
		currPanel->display_panel_update( errblk ) ;
		CHECK_ERROR_SETCALL_RETURN( e5 + p_name )
		if ( currPanel->get_msg() != "" )
		{
			get_message( currPanel->get_msg() ) ;
			if ( RC > 0 ) { return ; }
			currPanel->set_panel_msg( zmsg, zmsgid ) ;
			if ( !propagateEnd && !end_pressed() )
			{
				continue ;
			}
		}
		if ( currPanel->do_redisplay() ) { continue ; }

		currPanel->display_panel_proc( errblk, 0 ) ;
		clr_nondispl() ;
		CHECK_ERROR_SETCALL_RETURN( e4 + p_name )
		set_ZVERB_panel_resp() ;
		set_panel_zvars() ;
		if ( propagateEnd || end_pressed() )
		{
			if ( usr_action == USR_RETURN )
			{
				propagateEnd = true ;
			}
			RC = 8 ;
			return ;
		}

		if ( currPanel->get_msg() == "" ) { break ; }

		get_message( currPanel->get_msg() ) ;
		if ( RC > 0 ) { return ; }
		currPanel->set_panel_msg( zmsg, zmsgid ) ;
		currPanel->display_panel_reinit( errblk ) ;
		CHECK_ERROR_SETCALL_RETURN( e3 + p_name )
		RC = errblk.getRC() ;
		set_ZVERB_panel_resp_re_init() ;
		set_panel_zvars() ;
	}
}


void pApplication::edit( const string& m_file,
			 const string& m_panel,
			 const string& m_macro,
			 const string& m_profile,
			 const string& m_dataid,
			 const string& m_lcmds,
			 const string& m_confirm,
			 const string& m_preserve,
			 const string& m_parm,
			 int m_reclen )
{
	//
	// Edit a file.
	//
	// RC =  0  Normal completion.  Data was saved.
	// RC =  4  Normal completion.  Data was not saved.
	//          No changes made or CANCEL entered.
	// RC = 14  File in use.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "EDIT Service Error" ;

	edit_parms e ;

	if ( isvalidName( upper( m_file ) ) )
	{
		vcopy( upper( m_file ), e.edit_file, MOVE ) ;
	}
	else
	{
		e.edit_file = m_file ;
	}

	if ( m_dataid != "" )
	{
		e.edit_dataid = m_dataid ;
		e.edit_file   = get_file_for_dataid( m_dataid ) ;
		if ( e.edit_file == "" )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE054I", m_dataid ) ;
			checkRCode( errblk ) ;
			return ;
		}
	}

	e.edit_panel    = m_panel ;
	e.edit_macro    = m_macro ;
	e.edit_profile  = m_profile ;
	e.edit_lcmds    = m_lcmds  ;
	e.edit_confirm  = m_confirm ;
	e.edit_preserve = m_preserve ;
	if ( m_parm != "" )
	{
		if ( m_macro == "" )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE023S", m_parm ) ;
			checkRCode( errblk ) ;
			return ;
		}
		vcopy( m_parm, e.edit_parm, MOVE ) ;
		if ( RC > 0 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE023T", m_parm ) ;
			checkRCode( errblk ) ;
			return ;
		}
	}
	e.edit_reclen = m_reclen ;

	selct.clear() ;
	selct.pgm     = p_poolMGR->get( errblk, "ZEDITPGM", PROFILE ) ;
	selct.newappl = ""    ;
	selct.newpool = false ;
	selct.passlib = false ;
	selct.suspend = true  ;
	selct.options = (void*)&e ;
	selct.ptid    = ptid  ;
	selct.stid    = taskId ;
	selct.service = true  ;
	selct.errret  = controlErrorsReturn ;
	selct.backgrd = backgrd ;
	actionSelect() ;

	RC = ZRC ;
	if ( ZRC > 11 && isvalidName( ZRESULT ) )
	{
		errblk.setcall( TRACE_INFO(), "EDIT Error", ZRESULT, get_shared_var( "ZVAL1" ), ZRC ) ;
		checkRCode( errblk ) ;
	}
}


void pApplication::edrec( const string& m_parm )
{
	//
	// Edit recovery services.
	//
	// RC =  0  INIT    - Edit recovery table created.
	//          QUERY   - Recovery not pending.
	//          PROCESS - Recovery completed and data saved.
	// RC =  4  INIT    - Edit recovery table already exists for this application.
	//          QUERY   - Entry found in the Edit Recovery Table, recovery pending.
	//          PROCESS - Recovery completed but user did not save data.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "EDREC Service Error" ;

	int xRC ;

	string uprof ;

	const string qname   = "ISRRECOV" ;
	const string rname   = "*in progress*" ;
	const string tabName = get_applid() + "EDRT" ;
	const string vlist   = "ZEDSTAT ZEDTFILE ZEDBFILE ZEDMODE ZEDOPTS ZEDUSER ZEDRECFM ZEDLRECL" ;

	vcopy( "ZUPROF", uprof, MOVE ) ;

	errblk.setRC( 0 ) ;

	if ( m_parm == "INIT" )
	{
		xRC = edrec_init( m_parm,
				  qname,
				  rname,
				  uprof,
				  tabName,
				  vlist ) ;
	}
	else if ( m_parm == "QUERY" )
	{
		xRC = edrec_query( m_parm,
				   qname,
				   rname,
				   uprof,
				   tabName,
				   vlist ) ;
	}
	else if ( m_parm == "PROCESS" )
	{
		xRC = edrec_process( m_parm,
				     qname,
				     rname,
				     uprof,
				     tabName,
				     vlist ) ;
	}
	else if ( m_parm == "CANCEL" )
	{
		xRC = edrec_cancel( m_parm,
				    qname,
				    rname,
				    uprof,
				    tabName,
				    vlist ) ;
	}
	else if ( m_parm == "DEFER" )
	{
		xRC = edrec_defer( qname, rname ) ;
	}
	else
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE023M", m_parm ) ;
		checkRCode( errblk ) ;
		xRC = RC ;
	}

	if ( xRC >= 12 )
	{
		errblk.setcall( e1 ) ;
		checkRCode( errblk ) ;
		xRC = RC ;
	}

	RC = xRC ;
}


void pApplication::ftclose( const string& ft_member,
			    string ft_paths,
			    string ft_norepl )
{
	//
	// Terminate file tailoring.
	//
	// ft_paths can be a directory, allocation or lib-type LIBDEF.
	//
	// RC =  0  Normal completion.
	// RC =  4  File exists and NOREPL specified.  File unchanged.
	// RC =  8  File not open.  FTOPEN not used before FTCLOSE.
	// RC = 12  Output file in use.  ENQ failure.
	// RC = 16  Skeleton library or output file not allocated.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "FTCLOSE Service Error" ;

	const string ispfile = "ISPFILE" ;

	string zuser ;
	string ztempf ;
	string ft_path ;

	string lib ;

	RC = 0 ;

	if ( !FTailor )
	{
		RC = 8 ;
		return ;
	}

	iupper( ft_norepl ) ;

	if ( ft_norepl != "" && ft_norepl != "NOREPL" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE025E" ) ;
		checkRCode( errblk ) ;
		ft_cleanup() ;
		return ;
	}

	if ( FTailor->is_temp() )
	{
		zuser = p_poolMGR->get( errblk, "ZUSER", SHARED ) ;
		boost::filesystem::path temp = boost::filesystem::temp_directory_path() /
		       boost::filesystem::unique_path( zuser + "-" + d2ds( zscreen, 2 ) + "-%%%%-%%%%" ) ;
		ztempf = temp.native() ;
		p_poolMGR->put( errblk, "ZTEMPF", ztempf, SHARED ) ;
		if ( errblk.error() )
		{
			errblk.setcall( e1 ) ;
			checkRCode( errblk ) ;
			ft_cleanup() ;
			return ;
		}
	}
	else
	{
		if ( ft_paths == "" )
		{
			ft_paths = get_ddname_search_paths( ispfile ) ;
			if ( ft_paths == "" && has_libdef( ispfile ) )
			{
				ft_paths = get_libdef_search_paths( ispfile ) ;
				if ( errblk.error() )
				{
					ft_cleanup() ;
					return ;
				}
			}
		}
		else if ( ft_paths.size() < 9 && ft_paths.find( '/' ) == string::npos )
		{
			lib      = upper( ft_paths ) ;
			ft_paths = get_ddname_search_paths( lib ) ;
			if ( ft_paths == "" )
			{
				ft_paths = get_libdef_search_paths( lib ) ;
				if ( errblk.error() )
				{
					ft_cleanup() ;
					return ;
				}
			}
		}
		if ( ft_paths == "" )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE025C", 16 ) ;
			checkRCode( errblk ) ;
			ft_cleanup() ;
			return ;
		}
		if ( getpaths( ft_paths ) == 1 )
		{
			try
			{
				if ( is_directory( ft_paths ) )
				{
					if ( ft_member == "" )
					{
						errblk.setcall( TRACE_INFO(), e1, "PSYE025F" ) ;
						checkRCode( errblk ) ;
						ft_cleanup() ;
						return ;
					}
					ztempf = full_name( ft_paths, ft_member ) ;
				}
				else if ( is_regular_file( ft_paths ) || !exists( ft_paths ) )
				{
					if ( ft_member != "" )
					{
						errblk.setcall( TRACE_INFO(), e1, "PSYE025Q" ) ;
						checkRCode( errblk ) ;
						ft_cleanup() ;
						return ;
					}
					ztempf = ft_paths ;
				}
				else
				{
					errblk.setcall( TRACE_INFO(), e1, "PSYE025G" ) ;
					checkRCode( errblk ) ;
					ft_cleanup() ;
					return ;
				}
			}
			catch ( boost::filesystem::filesystem_error &e )
			{
				errblk.setcall( TRACE_INFO(), e1, "PSYS012C", e.what() ) ;
				checkRCode( errblk ) ;
				ft_cleanup() ;
				return ;
			}
		}
		else
		{
			if ( ft_member == "" )
			{
				errblk.setcall( TRACE_INFO(), e1, "PSYE025F" ) ;
				checkRCode( errblk ) ;
				ft_cleanup() ;
				return ;
			}
			locator loc( ft_paths, ft_member ) ;
			loc.locate() ;
			if ( loc.errors() )
			{
				errblk.setcall( TRACE_INFO(), e1, loc.msgid(), loc.mdata() ) ;
				ft_cleanup() ;
				return ;
			}
			ft_path = loc.entry() ;
			if ( ft_path == "" )
			{
				ztempf = getpath( ft_paths, 1 ) + ft_member ;
			}
		}
	}

	if ( ft_norepl == "NOREPL" && exists( ztempf ) )
	{
		ft_cleanup() ;
		RC = 4 ;
		return ;
	}

	enq( "SPFEDIT", ztempf ) ;
	if ( RC == 8 )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE025D", 12 ) ;
		checkRCode( errblk ) ;
		ft_cleanup() ;
		return ;
	}

	FTailor->ftclose( errblk, ztempf ) ;
	if ( errblk.error() )
	{
		errblk.setcall( e1 ) ;
		checkRCode( errblk ) ;
		ft_cleanup() ;
		return ;
	}

	deq( "SPFEDIT", ztempf ) ;

	ft_cleanup() ;
}


void pApplication::fterase( const string& ft_name,
			    const string& ft_paths )
{
	//
	// Erase tailoring file from output library.
	//
	// ft_paths can be a directory, allocation or lib-type LIBDEF.
	//
	// RC =  0  Normal completion.
	// RC =  8  File does not exist.
	// RC = 12  Output file in use.  ENQ failure.
	// RC = 16  Skeleton library or output file not allocated.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "FTERASE Service Error" ;

	const string ispfile = "ISPFILE" ;

	string delfile ;
	string paths ;

	if ( ft_name == "" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE025J" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( ft_paths == "" )
	{
		paths = get_ddname_search_paths( ispfile ) ;
		if ( paths == "" && has_libdef( ispfile ) )
		{
			paths = get_libdef_search_paths( ispfile ) ;
			if ( errblk.error() ) { return ; }
		}
	}
	else if ( ft_paths.size() < 9 && ft_paths.find( '/' ) == string::npos )
	{
		paths = get_ddname_search_paths( upper( ft_paths ) ) ;
		if ( paths == "" )
		{
			paths = get_libdef_search_paths( upper( ft_paths ) ) ;
			if ( errblk.error() ) { return ; }
		}
	}

	if ( paths == "" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE025C", 16 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( getpaths( paths ) == 1 )
	{
		try
		{
			if ( is_directory( paths ) )
			{
				delfile = full_name( paths, ft_name ) ;
			}
			else
			{
				errblk.setcall( TRACE_INFO(), e1, "PSYE025O" ) ;
				checkRCode( errblk ) ;
				return ;
			}
		}
		catch ( boost::filesystem::filesystem_error &e )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYS012C", e.what() ) ;
			checkRCode( errblk ) ;
			return ;
		}
	}
	else
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE025O" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( !exists( delfile ) )
	{
		RC = 8 ;
		return ;
	}

	enq( "SPFEDIT", delfile ) ;
	if ( RC == 8 )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE025D", 12 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	try
	{
		if ( !remove( delfile ) )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE025P", strerror( errno ), 20 ) ;
			checkRCode( errblk ) ;
			return ;
		}
	}
	catch ( boost::filesystem::filesystem_error &e )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE025P", e.what(), 20 ) ;
		checkRCode( errblk ) ;
		return ;
	}
}


void pApplication::ftopen( string ft_temp )
{
	//
	// Start file tailoring.
	//
	// RC =  0  Normal completion.
	// RC =  8  File tailoring already in progress.
	// RC = 16  Skeleton library or output file not allocated.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "FTOPEN Service Error" ;

	RC = 0 ;

	if ( FTailor )
	{
		RC = 8 ;
		return ;
	}

	iupper( ft_temp ) ;
	if ( ft_temp != "" && ft_temp != "TEMP" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE025A" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	FTailor = new pFTailor( errblk,
				funcPool,
				this,
				( ft_temp == "TEMP" ) ) ;
	if ( errblk.error() )
	{
		errblk.setcall( e1 ) ;
		checkRCode( errblk ) ;
		ft_cleanup() ;
		return ;
	}
}


void pApplication::ftincl( const string& ft_skel,
			   string ft_noft )
{
	//
	// Include skeleton.
	//
	// RC =  0  Normal completion.
	// RC =  8  Skeleton does not exist.
	// RC = 16  Skeleton library or output file not allocated.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "FTINCL Service Error" ;

	RC = 0 ;

	if ( !FTailor )
	{
		ftopen() ;
	}

	iupper( ft_noft ) ;
	if ( ft_noft != "" && ft_noft != "NOFT" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE025B" ) ;
		checkRCode( errblk ) ;
		ft_cleanup() ;
		return ;
	}

	FTailor->ftincl( errblk,
			 ft_skel,
			 ( ft_noft == "NOFT" ) ) ;
	if ( errblk.error() )
	{
		errblk.setcall( e1 ) ;
		checkRCode( errblk ) ;
		ft_cleanup() ;
		return ;
	}

	if ( FTailor->zftxrc == 8 )
	{
		setmsg( ( FTailor->zftxmsg == "" ) ? "PSYE034R" : FTailor->zftxmsg ) ;
	}

	RC = errblk.getRC() ;
}


void pApplication::getmsg( const string& msg,
			   const string& smsg,
			   const string& lmsg,
			   const string& alm,
			   const string& hlp,
			   const string& typ,
			   const string& wndo )
{
	//
	// Load message msg and substitute variables.
	//
	// RC =  0  Normal completion.
	// RC = 12  Message not found or message syntax error.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "GETMSG Service Error" ;

	errblk.setRC( 0 ) ;

	slmsg tmsg ;

	if ( smsg != "" && !isvalidName( smsg ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022O", "SHORT MESSAGE", smsg ) ;
		checkRCode( errblk ) ;
		return ;
	}
	if ( lmsg != "" && !isvalidName( lmsg ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022O", "LONG MESSAGE", lmsg ) ;
		checkRCode( errblk ) ;
		return ;
	}
	if ( alm != "" && !isvalidName( alm ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022O", "ALARM", alm ) ;
		checkRCode( errblk ) ;
		return ;
	}
	if ( hlp != "" && !isvalidName( hlp ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022O", "HELP", hlp ) ;
		checkRCode( errblk ) ;
		return ;
	}
	if ( typ != "" && !isvalidName( typ ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022O", "TYPE", typ ) ;
		checkRCode( errblk ) ;
		return ;
	}
	if ( wndo != "" && !isvalidName( wndo ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022O", "WINDOW", wndo ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( !load_message( msg ) ) { return ; }

	tmsg = msgList[ msg ] ;

	tmsg.smsg = sub_vars( tmsg.smsg ) ;
	tmsg.lmsg = sub_vars( tmsg.lmsg ) ;

	if ( tmsg.smsg.size() > 34  ) { tmsg.smsg.erase( 34  ) ; }
	if ( tmsg.lmsg.size() > 512 ) { tmsg.lmsg.erase( 512 ) ; }

	if ( !sub_message_vars( tmsg ) )
	{
		errblk.seterrid( TRACE_INFO(), "PSYE019D", "Invalid variable value" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( smsg != "" )
	{
		funcPool->put1( errblk, smsg, tmsg.smsg ) ;
		CHECK_ERROR_SETERROR_RETURN( e1 )
	}
	if ( lmsg != "" )
	{
		funcPool->put1( errblk, lmsg, tmsg.lmsg ) ;
		CHECK_ERROR_SETERROR_RETURN( e1 )
	}
	if ( alm != "" )
	{
		funcPool->put1( errblk, alm, ( tmsg.alm ) ? "YES" : "NO" ) ;
		CHECK_ERROR_SETERROR_RETURN( e1 )
	}
	if ( typ != "" )
	{
		switch ( tmsg.type )
		{
		case IMT: funcPool->put1( errblk, typ, "NOTIFY" )   ;
				break ;

		case WMT: funcPool->put1( errblk, typ, "WARNING" )  ;
				break ;

		case AMT: funcPool->put1( errblk, typ, "CRITICAL" ) ;
		}
		CHECK_ERROR_SETERROR_RETURN( e1 )
	}
	if ( hlp != "" )
	{
		funcPool->put1( errblk, hlp, tmsg.hlp ) ;
		CHECK_ERROR_SETERROR_RETURN( e1 )
	}
	if ( wndo != "" )
	{
		funcPool->put1( errblk, wndo, ( tmsg.resp ) ? "RESP" : "NORESP" ) ;
		CHECK_ERROR_SETERROR_RETURN( e1 )
	}
	RC = errblk.getRC() ;
}


void pApplication::libdef( const string& lib,
			   const string& type,
			   const string& id,
			   const string& procopt )
{
	//
	// LIBDEF - Add/remove a list of paths to the search order for panels, messages, skeletons and tables
	//          or a generic type.
	//
	//   RETURN CODE:
	//    0  Normal completion.
	//    4  Removing a LIBDEF that was not in effect.
	//       STKADD specified but no stack in effect.  Paths added.
	//    8  COND specified but a LIBDEF is already in effect.
	//   16  No paths in the ID() parameter, or invalid file name.
	//   20  Severe error.
	//
	// Format:
	//         Application-level libraries
	//         LIBDEF ZxLIB                       - remove LIBDEF for search
	//         LIBDEF ZxLIB PATH ID(path-list)    - add path-list to the search path
	//         LIBDEF ZxLIB LIBRARY ID(ddname)    - add ddname to the search path
	//         X - M, P, S or T
	//
	//         LIBDEF ZTABL                       - remove LIBDEF
	//         LIBDEF ZTABL PATH ID(path-list)    - add path to LIBDEF lib-type
	//         LIBDEF ZTABL LIBRARY ID(ddname)    - add ddname to the LIBDEF lib-type
	//
	//         LIBDEF MYLIB                       - remove generic LIBDEF
	//         LIBDEF MYLIB PATH ID(path-list)    - add path to generic LIBDEF lib-type
	//         LIBDEF MYLIB LIBRARY ID(ddname)    - add ddname to the generic LIBDEF lib-type
	//
	// Aliases ISPxLIB can be used for ZxLIB (Also ISPTABL and ISPFILE).
	//
	// For PATH:
	//   Path-list is a colon-separated list of directory names that must exist.
	//
	// For LIBRARY:
	//   ID() contains an allocated DDNAME of directory names that must exist.
	//
	// Search order for non-generic lib-types: user-level, application-level, system-level.
	//
	// See get_libdef_search_paths() for search order.
	//

	TRACE_FUNCTION() ;

	const string e1 = "LIBDEF Service Error" ;

	int i ;
	int p ;

	bool proc_cond   ;
	bool proc_uncond ;
	bool proc_stack  ;
	bool proc_stkadd ;

	string dirname ;
	string ddname ;
	string paths ;
	string llib ;

	map<string,stack<zlibdef>>::iterator it ;
	pair<map<string, stack<zlibdef>>::iterator, bool> result ;

	RC = 0 ;

	if ( !isvalidName( lib ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYS012U" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( procopt != "" && !findword( procopt, "COND UNCOND STACK STKADD" ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022I", procopt ) ;
		checkRCode( errblk ) ;
		return ;
	}

	proc_cond   = ( procopt == "COND"   ) ;
	proc_uncond = ( procopt == "UNCOND" ) ;
	proc_stack  = ( procopt == "STACK" || procopt == "" ) ;
	proc_stkadd = ( procopt == "STKADD" ) ;

	auto ita = ddaliases.find( lib ) ;
	llib = ( ita == ddaliases.end() ) ? lib : ita->second ;

	if ( type == "" )
	{
		if ( id != "" || procopt != "" )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE023K" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		it = zlibd.find( llib ) ;
		if ( it == zlibd.end() )
		{
			RC = 4 ;
			return ;
		}
		if ( it->second.top().library() )
		{
			p_gls->set_alloc_closed( it->second.top().ddname ) ;
		}
		it->second.pop() ;
		if ( it->second.empty() )
		{
			zlibd.erase( it ) ;
		}
		return ;
	}

	if ( type == "LIBRARY" )
	{
		if ( proc_stkadd )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE025R", 20 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		ddname = upper( id ) ;
		if ( !isvalidName( ddname ) )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE055B", 20 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		paths = get_ddname_search_paths( ddname ) ;
	}
	else if ( type == "PATH" )
	{
		paths = id ;
	}
	else
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022G", type ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p = getpaths( paths ) ;
	if ( p == 0 )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022F", 16 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	for ( i = 1 ; i <= p ; ++i )
	{
		dirname = getpath( paths, i ) ;
		try
		{
			if ( !exists( dirname ) || !is_directory( dirname ) )
			{
				errblk.setcall( TRACE_INFO(), e1, "PSYE023L", dirname, 16 ) ;
				checkRCode( errblk ) ;
				return ;
			}
		}
		catch ( boost::filesystem::filesystem_error &e )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYS012C", e.what() ) ;
			checkRCode( errblk ) ;
			return ;
		}
	}

	it = zlibd.find( llib ) ;

	if ( proc_cond && it != zlibd.end() )
	{
		RC = 8 ;
		return ;
	}
	else if ( proc_stkadd && it == zlibd.end() )
	{
		RC = 4 ;
	}

	if ( it == zlibd.end() )
	{
		result = zlibd.insert( pair<string, stack<zlibdef>>( llib, stack<zlibdef>() ) ) ;
		it     = result.first ;
	}

	if ( proc_cond || proc_uncond )
	{
		if ( it->second.empty() )
		{
			it->second.push( zlibdef( paths, ddname ) ) ;
		}
		else
		{
			it->second.top() = zlibdef( paths, ddname ) ;
		}
	}
	else if ( proc_stack || it->second.empty() )
	{
		it->second.push( zlibdef( paths, ddname ) ) ;
	}
	else
	{
		if ( it->second.top().library() )
		{
			p_gls->set_alloc_closed( it->second.top().ddname ) ;
		}
		it->second.top() = zlibdef( mergepaths( paths, it->second.top().paths ) ) ;
	}

	if ( type == "LIBRARY" )
	{
		p_gls->set_alloc_open( ddname ) ;
	}
}


void pApplication::list( const string& var )
{
	//
	// Write data to the lspf application log.
	//
	// RC =  0  Normal completion.
	// RC = 12  Message not found or message syntax error.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	string d ;

	vcopy( var, d, MOVE ) ;
	if ( RC == 0 )
	{
		llog( "L", d << endl ) ;
	}
}


void pApplication::lmclose( const string& dataid )
{
	//
	// LMCLOSE
	//
	// Close a dataid created by the LMINIT service.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 File not open.  PSYZ002 describes error.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "LMCLOSE Service Error" ;

	int exitRC ;

	if ( !isvalidName( dataid ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054A" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_lss->lmclose( errblk,
			funcPool,
			dataid ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	exitRC = errblk.getRC() ;

	if ( exitRC > 0 && errblk.msgid != "" )
	{
		vreplace( "ZERRMSG", errblk.msgid ) ;
		getmsg( errblk.msgid,
			"ZERRSM",
			"ZERRLM" ) ;
	}

	RC = exitRC ;
}


void pApplication::lmddisp( const string& listid )
{
	//
	// LMDDISP
	//
	// Display directory list created by the LMDINIT service.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 Error creating list.  PSYZ002 describes error.
	//   RC = 10 LISTID not found.  PSYZ002 describes error.
	//   RC = 12 Incorrect keyword.
	//   RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "LMDDISP Service Error" ;

	int exitRC ;

	if ( !isvalidName( listid ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054A" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_lss->lmddisp( errblk, funcPool, listid ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	selct.clear() ;
	selct.pgm     = p_poolMGR->get( errblk, "ZFLSTPGM", PROFILE ) ;
	selct.parm    = "DIR " + get_file_for_listid( listid ) ;
	selct.newappl = ""    ;
	selct.newpool = false ;
	selct.passlib = false ;
	selct.suspend = true  ;
	selct.ptid    = ptid  ;
	selct.stid    = taskId ;
	selct.errret  = controlErrorsReturn ;
	selct.backgrd = backgrd ;
	actionSelect() ;

	CHECK_ERROR_SETCALL_RETURN( e1 )

	exitRC = errblk.getRC() ;

	if ( exitRC > 0 && errblk.msgid != "" )
	{
		vreplace( "ZERRMSG", errblk.msgid ) ;
		getmsg( errblk.msgid,
			"ZERRSM",
			"ZERRLM" ) ;
	}

	RC = exitRC ;
}


void pApplication::lmdfree( const string& listid )
{
	//
	// LMDFREE
	//
	// Remove a listid created by the LMDINIT service.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 Free of listid failed.  PSYZ002 describes error.
	//   RC = 10 listid not found.  PSYZ002 describes error.
	//   RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "LMDFREE Service Error" ;

	int exitRC ;

	if ( !isvalidName( listid ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054A" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_lss->lmdfree( errblk, funcPool, listid ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	exitRC = errblk.getRC() ;

	if ( exitRC > 0 && errblk.msgid != "" )
	{
		vreplace( "ZERRMSG", errblk.msgid ) ;
		getmsg( errblk.msgid,
			"ZERRSM",
			"ZERRLM" ) ;
	}

	RC = exitRC ;
}


void pApplication::lmdinit( const string& listidv,
			    const string& level )
{
	//
	// LMDINIT
	//
	// Generate a listid for a directory.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 LISTID not created.  PSYZ002 describes error.
	//   RC = 12 Parameter is invalid.  PSYZ002 describes error.
	//   RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	int exitRC ;

	const string e1 = "LMDINIT Service Error" ;

	if ( listidv == "" || !isvalidName( listidv ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054B", "LISTID" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( level == "" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054K" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_lss->lmdinit( errblk,
			funcPool,
			listidv,
			level ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	exitRC = errblk.getRC() ;

	if ( exitRC > 0 && errblk.msgid != "" )
	{
		vreplace( "ZERRMSG", errblk.msgid ) ;
		getmsg( errblk.msgid,
			"ZERRSM",
			"ZERRLM" ) ;
	}

	RC = exitRC ;
}


void pApplication::lmfree( const string& dataid )
{
	//
	// LMFREE
	//
	// Remove a dataid created by the LMINIT service.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 Free of dataid failed.  PSYZ002 describes error.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "LMFREE Service Error" ;

	int exitRC ;

	if ( !isvalidName( dataid ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054A" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_lss->lmfree( errblk, funcPool, dataid ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	exitRC = errblk.getRC() ;

	if ( exitRC > 0 && errblk.msgid != "" )
	{
		vreplace( "ZERRMSG", errblk.msgid ) ;
		getmsg( errblk.msgid,
			"ZERRSM",
			"ZERRLM" ) ;
	}

	RC = exitRC ;
}


void pApplication::lmget( const string& dataid,
			  const string& mode,
			  const string& dataloc,
			  const string& datalenv,
			  size_t maxlen )
{
	//
	// LMGET
	//
	// Read a record from the file for the given DATAID.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 End of data.  No message formated.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 12 File not open for input, LMMFIND not done or parameter error.
	//   RC = 16 Error access dialogue variables.
	//   RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "LMGET Service Error" ;

	string data ;

	int exitRC ;

	if ( !isvalidName( dataid ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054A" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( mode != "INVAR" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054E" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( dataloc == "" || !isvalidName( dataloc ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054B", "DATALOC" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( datalenv == "" || !isvalidName( datalenv ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054B", "DATALEN" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_lss->lmget( errblk,
		      funcPool,
		      dataid,
		      data,
		      datalenv,
		      maxlen ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	exitRC = errblk.getRC() ;

	vreplace( dataloc, data ) ;

	if ( exitRC > 0 && errblk.msgid != "" )
	{
		vreplace( "ZERRMSG", errblk.msgid ) ;
		getmsg( errblk.msgid,
			"ZERRSM",
			"ZERRLM" ) ;
	}

	RC = exitRC ;
}


void pApplication::lminit( const string& dataidv,
			   const string& dsn,
			   const string& ddn,
			   const string& disp )
{
	//
	// LMINIT
	//
	// Associate a dataid with a file/directory.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 Dataid not created.  PSYZ002 describes error.
	//   RC = 12 Parameter is invalid.  PSYZ002 describes error.
	//   RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "LMINIT Service Error" ;

	LM_DISP lmdisp ;

	if ( disp == "SHR" )
	{
		lmdisp = LM_SHR ;
	}
	else if ( disp == "EXCLU" )
	{
		lmdisp = LM_EXCLU ;
	}
	else
	{
		funcPool->put2( errblk, "ZEDSMSG", "Invalid disposition specified." ) ;
		funcPool->put2( errblk, "ZEDLMSG", "Disposition must be either SHR or EXCLU" ) ;
		errblk.setcall( TRACE_INFO(), e1, "PSYZ002", 12 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	lminit( dataidv, dsn, ddn, lmdisp ) ;
}


void pApplication::lminit( const string& dataidv,
			   const string& dsn,
			   const string& ddn,
			   LM_DISP disp )
{
	//
	// LMINIT
	//
	// Associate a dataid with a file/directory.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 Dataid not created.  PSYZ002 describes error.
	//   RC = 12 Parameter is invalid.  PSYZ002 describes error.
	//   RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	int exitRC ;

	const string e1 = "LMINIT Service Error" ;

	if ( dataidv == "" || !isvalidName( dataidv ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054B", "DATAID" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( dsn == "" && ddn == "" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054C" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( dsn != "" && ddn != "" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054D" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_lss->lminit( errblk,
		       funcPool,
		       dataidv,
		       dsn,
		       ddn,
		       disp ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	exitRC = errblk.getRC() ;

	if ( exitRC > 0 && errblk.msgid != "" )
	{
		vreplace( "ZERRMSG", errblk.msgid ) ;
		getmsg( errblk.msgid,
			"ZERRSM",
			"ZERRLM" ) ;
	}

	RC = exitRC ;
}


void pApplication::lmmadd( const string& dataid,
			   const string& member )
{
	//
	// LMMADD
	//
	//   Add a member to a library.
	//
	//   RC =  0 Normal completion.
	//   RC =  4 Directory already contains the specified name.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 12 Parameter invalid, file not open for output, DSORG invalid.
	//   RC = 14 No records written for the member to be added.
	//   RC = 16 Truncation accessing dialgue variables.
	//   RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "LMMADD Service Error" ;

	int exitRC ;

	if ( !isvalidName( dataid ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054A" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( member == "" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054F", "MEMBER", "LMMADD", 12 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_lss->lmmadd( errblk,
		       funcPool,
		       dataid,
		       member ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	exitRC = errblk.getRC() ;

	if ( exitRC > 0 && errblk.msgid != "" )
	{
		vreplace( "ZERRMSG", errblk.msgid ) ;
		getmsg( errblk.msgid,
			"ZERRSM",
			"ZERRLM" ) ;
	}

	RC = exitRC ;
}


void pApplication::lmmdel( const string& dataid,
			   const string& member )
{
	//
	// LMMDEL
	//
	//   Delete a member from a library.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 Member was not found.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 12 Parameter invalid, file not open for output, DSORG invalid.
	//   RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "LMMDEL Service Error" ;

	int exitRC ;

	if ( !isvalidName( dataid ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054A" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( member == "" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054F", "MEMBER", "LMMDEL", 12 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_lss->lmmdel( errblk,
		       funcPool,
		       dataid,
		       member ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	exitRC = errblk.getRC() ;

	if ( exitRC > 0 && errblk.msgid != "" )
	{
		vreplace( "ZERRMSG", errblk.msgid ) ;
		getmsg( errblk.msgid,
			"ZERRSM",
			"ZERRLM" ) ;
	}

	RC = exitRC ;
}


void pApplication::lmmfind( const string& dataid,
			    const string& member,
			    string stats )
{
	//
	// LMMFIND
	//
	//   Find a member in a directory.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 Member not found.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 12 Parameter invalid, file not open for input, DSORG invalid.
	//   RC = 14 No records written for the member to be added.
	//   RC = 16 Truncation accessing dialgue variables.
	//   RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "LMMFIND Service Error" ;

	int exitRC ;

	if ( stats == "" ) { stats = "NO"   ; }

	if ( !isvalidName( dataid ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054A" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( member == "" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054F", "MEMBER", "LMMFIND", 12 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( stats != "YES" && stats != "NO" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054J", "STATS", "YES or NO", 20 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_lss->lmmfind( errblk,
			funcPool,
			dataid,
			member,
			stats ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	exitRC = errblk.getRC() ;

	if ( exitRC > 0 && errblk.msgid != "" )
	{
		vreplace( "ZERRMSG", errblk.msgid ) ;
		getmsg( errblk.msgid,
			"ZERRSM",
			"ZERRLM" ) ;
	}

	RC = exitRC ;
}


void pApplication::lmmlist( const string& dataid,
			    string opt,
			    const string& member,
			    string stats,
			    const string& pattern )
{
	//
	// LMMLIST
	//
	//   List members in a directory.
	//
	//   RC =  0 Normal completion.
	//   RC =  4 Empty member list.
	//   RC =  8 LIST - End of member list.
	//           FREE - Member list not found.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 12 Parameter invalid, file not open for input, DSORG invalid.
	//   RC = 16 Truncation accessing dialgue variables.
	//   RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "LMMLIST Service Error" ;

	int exitRC ;

	if ( opt   == "" ) { opt   = "LIST" ; }
	if ( stats == "" ) { stats = "NO"   ; }

	if ( !isvalidName( dataid ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054A" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( opt != "LIST" && opt != "FREE" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054J", "OPTION", "LIST or FREE", 20 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( opt == "LIST" && !isvalidName( member ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054B", "MEMBER", 12 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( stats != "YES" && stats != "NO" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054J", "STATS", "YES or NO", 20 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_lss->lmmlist( errblk,
			funcPool,
			dataid,
			opt,
			member,
			stats,
			pattern ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	exitRC = errblk.getRC() ;

	if ( exitRC > 0 && errblk.msgid != "" )
	{
		vreplace( "ZERRMSG", errblk.msgid ) ;
		getmsg( errblk.msgid,
			"ZERRSM",
			"ZERRLM" ) ;
	}

	RC = exitRC ;
}


void pApplication::lmmrep( const string& dataid,
			   const string& member )
{
	//
	// LMMREP
	//
	//   Replace a member in a directory.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 Member is added - it did not exist.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 12 Parameter invalid, file not open for output, DSORG invalid.
	//   RC = 14 No records written for the member to be added.
	//   RC = 16 Truncation accessing dialgue variables.
	//   RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "LMMREP Service Error" ;

	int exitRC ;

	if ( !isvalidName( dataid ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054A" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( member == "" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054F", "MEMBER", "LMMREP", 12 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_lss->lmmadd( errblk,
		       funcPool,
		       dataid,
		       member,
		       true ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	exitRC = errblk.getRC() ;

	if ( exitRC > 0 && errblk.msgid != "" )
	{
		vreplace( "ZERRMSG", errblk.msgid ) ;
		getmsg( errblk.msgid,
			"ZERRSM",
			"ZERRLM" ) ;
	}

	RC = exitRC ;
}


void pApplication::lmopen( const string& dataid,
			   const string& option,
			   const string& orgv )
{
	//
	// LMOPEN
	//
	// Open a file for input or output processing.
	//
	//   RC =  0 Normal completion.
	//   RC =  8 File could not be opened.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 12 Parameter invalid or file already open.
	//   RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "LMOPEN Service Error" ;

	int exitRC ;

	if ( !isvalidName( dataid ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054A" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( option != "" && option != "INPUT" && option != "OUTPUT" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054G", 12 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( orgv != "" && !isvalidName( orgv ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054B", "DSORG", 12 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_lss->lmopen( errblk,
		       funcPool,
		       dataid,
		       option,
		       orgv ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	exitRC = errblk.getRC() ;

	if ( exitRC > 0 && errblk.msgid != "" )
	{
		vreplace( "ZERRMSG", errblk.msgid ) ;
		getmsg( errblk.msgid,
			"ZERRSM",
			"ZERRLM" ) ;
	}

	RC = exitRC ;
}


void pApplication::lmput( const string& dataid,
			  const string& mode,
			  const string& dataloc,
			  int datalen )
{
	//
	// LMPUT
	//
	// Write a record to the file/directory for the given DATAID.
	//
	//   RC =  0 Normal completion.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 12 File not open for output or parameter error.
	//   RC = 16 Error access dialogue variables.
	//   RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "LMPUT Service Error" ;

	string data ;

	int exitRC ;

	if ( !isvalidName( dataid ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054A" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( mode != "INVAR" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054E", 12 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( dataloc == "" || !isvalidName( dataloc ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054B", "DATALOC", 12 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	vcopy( dataloc, data, MOVE ) ;
	if ( RC > 0 )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054H", dataloc, 16 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	data.resize( datalen, ' ' ) ;

	p_lss->lmput( errblk,
		      funcPool,
		      dataid,
		      data ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	exitRC = errblk.getRC() ;

	if ( exitRC > 0 && errblk.msgid != "" )
	{
		vreplace( "ZERRMSG", errblk.msgid ) ;
		getmsg( errblk.msgid,
			"ZERRSM",
			"ZERRLM" ) ;
	}

	RC = exitRC ;
}


void pApplication::lmquery( const string& dataid,
			    const string& dsnv,
			    const string& ddnv,
			    const string& enqv,
			    const string& openv,
			    const string& dsorgv )
{
	//
	// LMQUERY
	//
	// Return values specified for the LMINIT service.
	//
	//   RC =  0 Normal completion.
	//   RC =  4 Some data not available.  Blanks returned.
	//   RC = 10 Dataid not found.  PSYZ002 describes error.
	//   RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "LMQUERY Service Error" ;

	int exitRC ;

	string w ;

	if ( !isvalidName( dataid ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE054A" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( namesNotValid( w, dsnv, ddnv, enqv, openv, dsorgv ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022J", w, "variable" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_lss->lmquery( errblk,
			funcPool,
			dataid,
			dsnv,
			ddnv,
			enqv,
			openv,
			dsorgv ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	exitRC = errblk.getRC() ;

	if ( exitRC > 0 && errblk.msgid != "" )
	{
		vreplace( "ZERRMSG", errblk.msgid ) ;
		getmsg( errblk.msgid,
			"ZERRSM",
			"ZERRLM" ) ;
	}

	RC = exitRC ;
}


void pApplication::log( const string& msgid )
{
	//
	// Write a message to the lspf application log.
	//
	// RC =  0  Normal completion.
	// RC = 12  Message not found or message syntax error.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "LOG Service Error" ;

	string zzsmsg ;
	string zzlmsg ;

	get_message( msgid,
		     zzsmsg,
		     zzlmsg ) ;

	if ( RC == 0 )
	{
		llog( "L", zzsmsg << endl ) ;
		llog( "L", zzlmsg << endl ) ;
	}
}


void pApplication::notify( const string& msg,
			   bool subVars )
{
	TRACE_FUNCTION() ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	notifies.push_back( subVars ? sub_vars( msg ) : msg ) ;
}


void pApplication::pquery( const string& p_name,
			   const string& a_name,
			   const string& t_name,
			   const string& w_name,
			   const string& d_name,
			   const string& r_name,
			   const string& c_name )
{
	//
	// Return information about a specified area on a specified panel.
	//
	// RC =  0  Normal completion.
	// RC =  8  Specified area not found on panel.
	// RC = 20  Severe error.
	//
	// If panel not already loaded, load and delete afterwards to prevent a blank screen in some cases.
	// EG.  Editor -> IMACRO that displays a panel - editor panel blank.
	// (On ncurses panel stack for the logical screen but has never been processed via pPanel::display_panel()).
	//

	TRACE_FUNCTION() ;

	const string e1 = "PQUERY Service Error" ;
	const string e2 = "PQUERY Error for panel " + p_name ;

	bool p_preloaded ;

	pPanel* panel ;

	errblk.setRC( 0 ) ;

	if ( p_name == "" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE019C", "PANEL" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( !isvalidName( p_name ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE021A", p_name ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( a_name == "" )
	{
		errblk.setcall( TRACE_INFO(), e2, "PSYE019C", "area name" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( !isvalidName( a_name ) )
	{
		errblk.setcall( TRACE_INFO(), e2, "PSYE023C", "area", a_name ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( t_name != "" && !isvalidName( t_name ) )
	{
		errblk.setcall( TRACE_INFO(), e2, "PSYE023C", "area type", t_name ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( w_name != "" && !isvalidName( w_name ) )
	{
		errblk.setcall( TRACE_INFO(), e2, "PSYE023C", "area width", w_name ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( d_name != "" && !isvalidName( d_name ) )
	{
		errblk.setcall( TRACE_INFO(), e2, "PSYE023C", "area depth", d_name ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( r_name != "" && !isvalidName( r_name ) )
	{
		errblk.setcall( TRACE_INFO(), e2, "PSYE023C", "area row number", r_name ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( c_name != "" && !isvalidName( c_name ) )
	{
		errblk.setcall( TRACE_INFO(), e2, "PSYE023C", "area column number", c_name ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_preloaded = ( panelList.find( p_name ) != panelList.end() ) ;

	panel = create_panel( p_name ) ;
	if ( errblk.error() ) { return ; }

	panel->get_panel_info( errblk,
			       a_name,
			       t_name,
			       w_name,
			       d_name,
			       r_name,
			       c_name ) ;
	RC = errblk.getRC() ;

	if ( !p_preloaded )
	{
		delete panel ;
		panelList.erase( p_name ) ;
	}
}


void pApplication::qbaselib( const string& lib,
			     const string& id_var )
{
	//
	// Query base library information.
	//
	// RC =  0 Normal completion.
	// RC =  4 Specified DD-name not defined.
	// RC = 20 Severe Error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "QBASELIB Service Error" ;

	string list ;

	RC = 0 ;

	if ( !isvalidName( lib ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYS012U" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	list = get_ddname_search_paths( lib ) ;

	if ( list == "" )
	{
		RC = 4 ;
		return ;
	}

	if ( id_var != "" )
	{
		funcPool->put1( errblk, id_var, list ) ;
		CHECK_ERROR_SETCALL_RETURN( e1 )
	}
}


void pApplication::qlibdef( const string& lib,
			    const string& type_var,
			    const string& id_var )
{
	//
	// Query LIBDEF status for lib-type lib.
	//
	// RC =  0 Normal completion.  LIBDEF exists and information returned.
	// RC =  4 Specified lib-type does not have an active LIBDEF definition.
	// RC = 12 Invalid lib-type of ZUPROF specified.
	// RC = 20 Severe Error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "QLIBDEF Service Error" ;

	map<string,stack<zlibdef>>::iterator it ;

	RC = 0 ;

	if ( !isvalidName( lib ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYS012U" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( lib == "ZUPROF" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYS014I", 12 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	it = zlibd.find( lib ) ;
	if ( it == zlibd.end() )
	{
		RC = 4 ;
		return ;
	}

	if ( type_var != "" )
	{
		funcPool->put1( errblk, type_var, ( ( it->second.top().library() ) ? "LIBRARY" : "PATH" ) ) ;
		CHECK_ERROR_SETCALL_RETURN( e1 )
	}

	if ( id_var != "" )
	{
		funcPool->put1( errblk, id_var, it->second.top().paths ) ;
		CHECK_ERROR_SETCALL_RETURN( e1 )
	}
}


void pApplication::qtabopen( const string& tb_list )
{
	//
	// Obtain a list of currently open lspf tables.
	//
	// RC =  0  Normal completion.
	// RC =  4  Incomplete list.  Insufficient space to construct variable name.
	// RC = 12  Variable name prefix too long (max 7 characters).
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "QTABOPEN Error" ;

	p_tableMGR->qtabopen( errblk, funcPool, tb_list ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )
}


void pApplication::queryenq( const string& table,
			     const string& qnamev,
			     const string& rnamev,
			     const string& req,
			     const string& wait,
			     int limit,
			     const string& save )
{
	//
	// Query system enqueues.
	//
	// RC =  0 Normal completion.  Table written.
	// RC =  4 Table written but truncated.
	// RC =  8 No enqueues satisfy request.
	// RC = 12 Table create failed.
	// RC = 20 Severe Error.
	//

	TRACE_FUNCTION() ;

	string fname ;

	string qname ;
	string rname ;

	int rc ;

	const string e1 = "QUERYENQ Service Error" ;

	const string vlist1 = "ZENJOB ZENQNAME ZENRNAME ZENDISP ZENHOLD ZENSCOPE ZENSTEP ZENGLOBL ZENSYST ZENRESV" ;

	vector<vector<string>> v ;

	std::ofstream* fout ;

	if ( !isvalidName( qnamev ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE031D", qnamev ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( !isvalidName( rnamev ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE031D", rnamev ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( wait != "" && wait != "WAIT" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE013M", "WAIT", wait ) ;
		checkRCode( errblk ) ;
		return ;
	}

	vcopy( qnamev, qname ) ;
	vcopy( rnamev, rname ) ;

	if ( qname == "" )
	{
		qname = "*" ;
	}

	if ( rname == "" )
	{
		rname = "*" ;
	}

	rc = p_gls->queryenq( v, qname, rname, req, ( wait == "WAIT" ), limit ) ;

	p_tableMGR->tbcreate( errblk,
			      table,
			      "",
			      vlist1 ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	if ( v.size() == 0 )
	{
		RC = 8 ;
		return ;
	}

	if ( save != "" )
	{
		fname = p_poolMGR->get( errblk, "ZHOME", SHARED ) + "/" + save + ".enqlist" ;
		fout  = new std::ofstream( fname.c_str() ) ;
		if ( !fout->is_open() )
		{
			delete fout ;
			errblk.setcall( TRACE_INFO(), e1, "PSYE055A", fname ) ;
			checkRCode( errblk ) ;
			return ;
		}
	}

	for ( auto it = v.begin() ; it != v.end() ; ++it )
	{
		tbvclear( table ) ;
		vreplace( "ZENJOB", it->at( 0 ) ) ;
		vreplace( "ZENQNAME", it->at( 1 ) ) ;
		vreplace( "ZENRNAME", it->at( 2 ) ) ;
		vreplace( "ZENDISP", it->at( 3 ) ) ;
		tbadd( table ) ;
		if ( save != "" )
		{
			*fout << it->at( 0 ) << " " ;
			*fout << it->at( 3 ) << " " ;
			*fout << it->at( 1 ) << " " ;
			*fout << it->at( 2 ) << endl ;
		}
	}

	if ( save != "" )
	{
		fout->close() ;
		delete fout ;
	}

	RC = rc ;
}


void pApplication::say( const string& msg,
			bool subVars )
{
	//
	// Display line mode output.  Difference with rdisplay is that the
	// return code is not changed by this method.
	//

	TRACE_FUNCTION() ;

	guardInt t( RC ) ;

	rdisplay( msg, subVars ) ;
}


void pApplication::rdisplay( const string& msg,
			     bool subVars )
{
	//
	// Display line mode output on the screen.  Cancel any screen refreshes as this will be done
	// as part of returning to full screen mode.
	// If running in the background, log message to the application log.
	//
	// RC =  0  Normal completion.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	RC = 0 ;

	outBuffer = ( subVars ) ? sub_vars( msg ) : msg ;

	if ( backgrd )
	{
		llog( "B", outBuffer << endl ) ;
		return ;
	}

	if ( !busyAppl )
	{
		llog( "E", "Invalid method state" <<endl ) ;
		llog( "E", "Application is in a wait state.  Method cannot issue line output" <<endl ) ;
		RC = 20 ;
		return ;
	}

	lineInOutDone  = true  ;
	lineOutPending = true  ;
	refreshlScreen = false ;
	wait_event( WAIT_OUTPUT ) ;
	if ( abnormalEndForced )
	{
		uabend() ;
	}
	lineOutPending = false ;
}


void pApplication::pull( const string& var )
{
	//
	// Pull data from user in raw mode and put in the function pool variable 'var'.
	//

	TRACE_FUNCTION() ;

	string str ;
	const string e1 = "PULL Service Error" ;

	RC = 0 ;

	if ( !isvalidName( var ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYS012U" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	pull( &str ) ;
	funcPool->put2( errblk, var, str ) ;
}


void pApplication::pull( string* str )
{
	//
	// Pull data from user in raw mode and put in the passed string pointer.
	//

	TRACE_FUNCTION() ;

	RC = 0 ;

	if ( backgrd )
	{
		llog( "E", "Cannot pull data in batch mode" << endl ) ;
		RC = 20 ;
		return ;
	}

	if ( !busyAppl )
	{
		llog( "E", "Invalid method state" <<endl ) ;
		llog( "E", "Application is in a wait state.  Method cannot pull input" <<endl ) ;
		RC = 20 ;
		return ;
	}

	lineInOutDone = true ;
	lineInPending = true ;
	wait_event( WAIT_USER ) ;
	if ( abnormalEndForced )
	{
		uabend() ;
	}
	lineInPending = false ;
	*str = inBuffer ;
}


void pApplication::rempop( const string& rem_all )
{
	//
	// Remove pop-up window.  Restore previous addpop() if there is one.
	//
	// RC =  0 Normal completion.
	// RC = 16 No pop-up window exists at this level.
	// RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "REMPOP Service Error" ;

	pPanel* panl ;

	RC = 0 ;

	if ( rem_all != "" && rem_all != "ALL" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022U", rem_all ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( !addpop_active )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022T", 16 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( rem_all == "ALL" )
	{
		while ( !popups.empty() )
		{
			popup& pop = popups.top() ;
			panl = static_cast<pPanel*>( pop.panl ) ;
			if ( panl )
			{
				panl->pop_panels( pop ) ;
			}
			popups.pop() ;
		}
	}

	if ( !popups.empty() )
	{
		popup& pop = popups.top() ;
		if ( pop.panl )
		{
			addpop_col = pop.col ;
			addpop_row = pop.row ;
			panl       = static_cast<pPanel*>( pop.panl ) ;
			panl->pop_panels( pop ) ;
		}
		popups.pop() ;
	}
	else
	{
		addpop_active = false ;
		addpop_row    = 0 ;
		addpop_col    = 0 ;
	}
}


void pApplication::select( const string& cmd )
{
	//
	// SELECT a function or panel in keyword format for use in applications,
	// ie PGM(abc) CMD(oorexx) PANEL(xxx) PARM(zzz) NEWAPPL PASSLIB SCRNAME(abc) etc.
	//
	// No variable substitution is done at this level.
	//
	// RC =  0 Normal completion.
	// RC =  4 Normal completion.  RETURN or EXIT entered from the selection panel.
	// RC = 12 Specified panel could not be found.
	// RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "Error in SELECT command " ;

	if ( !selct.parse( errblk, cmd ) )
	{
		errblk.setcall( e1 + cmd ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( backgrd && selct.pgmtype == PGM_PANEL )
	{
		errblk.setcall( TRACE_INFO(), e1 + cmd, "PSYE039T" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	selct.backgrd = backgrd ;
	selct.sync    = true ;
	selct.ptid    = ptid ;
	selct.stid    = taskId ;

	actionSelect() ;
}


void pApplication::setmsg( const string& msg,
			   msgSET sType,
			   const string& msgloc )
{
	//
	// Retrieve message and store in zmsg1 and zmsgid1.
	//
	// Ignore error if message field not found.
	//
	// RC =  0 Normal completion.
	// RC =  4 COND specified and a SETMSG request was pending.
	// RC = 12 Specified message could not be found.
	// RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "SETMSG Service Error" ;

	RC = 0 ;

	if ( setMessage && sType == COND )
	{
		RC = 4 ;
		return ;
	}

	if ( msgloc != "" && !isvalidName( msgloc ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE031L", msgloc, "MSGLOC()" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	get_message( msg ) ;
	if ( RC > 0 )
	{
		return  ;
	}

	zmsg1        = zmsg ;
	zmsg1.msgloc = msgloc ;
	zmsgid1      = msg  ;
	setMessage   = true ;
}


void pApplication::submit( const string& cmd )
{
	//
	// In the background, SELECT a function in keyword format for use in applications,
	// ie PGM(abc) CMD(oorexx) PARM(zzz) NEWAPPL PASSLIB etc.
	//
	// No variable substitution is done at this level.
	//

	TRACE_FUNCTION() ;

	const string e1 = "Error in SUBMIT command" ;

	if ( !selct.parse( errblk, cmd ) )
	{
		errblk.setcall( e1 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( selct.pgmtype == PGM_PANEL )
	{
		errblk.setcall( TRACE_INFO(), e1 + cmd, "PSYE039T" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	selct.backgrd = true  ;
	selct.sync    = false ;
	actionSelect() ;
}


void pApplication::tbadd( const string& tb_name,
			  string tb_namelst,
			  const string& tb_order,
			  int tb_num_of_rows )
{
	//
	// Add a new row to a table.
	//
	// RC =  0  Normal completion.
	// RC =  8  For keyed tables only, row already exists.
	// RC = 12  Table not open.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBADD Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	if ( tb_order != "" && tb_order != "ORDER" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE023A", "TBADD", tb_order ) ;
		checkRCode( errblk ) ;
		return ;
	}
	if ( tb_num_of_rows < 0 || tb_num_of_rows > 65535 )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE023B", "TBADD", d2ds( tb_num_of_rows ) ) ;
		checkRCode( errblk ) ;
		return ;
	}

	getNameList( errblk, tb_namelst ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	p_tableMGR->tbadd( errblk,
			   funcPool,
			   tb_name,
			   tb_namelst,
			   tb_order,
			   tb_num_of_rows ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbbottom( const string& tb_name,
			     const string& tb_save_name,
			     const string& tb_rowid_name,
			     const string& tb_noread,
			     const string& tb_crp_name )
{
	//
	// Move row pointer to the bottom.
	//
	// RC =  0  Normal completion.
	// RC =  8  Table is empty.  CRP is set to 0.
	// RC = 12  Table not open.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBBOTTOM Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	p_tableMGR->tbbottom( errblk,
			      funcPool,
			      tb_name,
			      tb_save_name,
			      tb_rowid_name,
			      tb_noread,
			      tb_crp_name ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbclose( const string& tb_name,
			    const string& tb_newname,
			    string tb_paths )
{
	//
	// Save and close the table (calls saveTable and destroyTable routines).
	//
	// If table opened in NOWRITE mode, just remove table from storage.
	//
	// If tb_paths is not specified, use ZTABL as the output path.  Error if blank.
	//
	// RC =  0  Normal completion.
	// RC = 12  Table not open.
	// RC = 16  Path error.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBCLOSE Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	if ( tb_newname != "" && !isvalidName( tb_newname ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022J", tb_newname, "table" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( p_tableMGR->writeableTable( errblk, tb_name, "TBCLOSE" ) )
	{
		tb_paths = get_search_paths( tb_paths, s_ZTABL ) ;
		if ( errblk.error() ) { return ; }
		if ( tb_paths == "" )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE013C", 16 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		p_tableMGR->saveTable( errblk,
				       "TBCLOSE",
				       tb_name,
				       tb_newname,
				       tb_paths ) ;
		CHECK_ERROR_SETCALL_RETURN( e1 )
	}
	if ( errblk.RC0() )
	{
		p_tableMGR->destroyTable( errblk, tb_name, "TBCLOSE" ) ;
		CHECK_ERROR_SETCALL_RETURN( e1 )
	}
	RC = errblk.getRC() ;
}


void pApplication::tbcreate( const string& tb_name,
			     string tb_keys,
			     string tb_names,
			     tbWRITE tb_WRITE,
			     tbREP tb_REP,
			     string tb_paths,
			     tbDISP tb_DISP )
{
	//
	// Create a new table.
	//
	// tb_paths is an input library to check if the table already exists if opened in WRITE mode.
	// Default to ZTLIB if blank.
	//
	// RC =  0  Normal completion.
	// RC =  4  Normal completion - Table exists and REPLACE speified.
	// RC =  8  REPLACE not specified and table exists,
	//          or REPLACE specified with table open in SHARE or SHARE requested,
	//          or SHARE specified but a different task has it open NON_SHARE.
	// RC = 12  Table in use.
	// RC = 16  WRITE specified but input library not specified or allocated.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBCREATE Service Error" ;

	RC = 0 ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	tb_paths = get_search_paths( tb_paths, s_ZTLIB ) ;
	if ( errblk.error() ) { return ; }

	getNameList( errblk, tb_keys ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	getNameList( errblk, tb_names ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	if ( tb_paths == "" && tb_WRITE == WRITE )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE013V", 16 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_tableMGR->tbcreate( errblk,
			      tb_name,
			      tb_keys,
			      tb_names,
			      tb_WRITE,
			      tb_REP,
			      tb_paths,
			      tb_DISP ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbdelete( const string& tb_name )
{
	//
	// Delete a row in the table.
	//
	// For keyed tables, the table is searched with the current key.
	// For non-keyed tables the current CRP is used.
	//
	// RC =  0  Normal completion.
	// RC =  8  Row does not exist for a keyed table or for non-keyed table, CRP was at TOP(zero).
	// RC = 12  Table not open.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBDELETE Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	p_tableMGR->tbdelete( errblk, funcPool, tb_name ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbdispl( const string& tb_name,
			    string p_name,
			    const string& p_msg,
			    string p_cursor,
			    int p_csrrow,
			    int p_curpos,
			    string p_autosel,
			    const string& p_crp_name,
			    const string& p_rowid_name,
			    const string& p_msgloc )
{
	//
	// Display a table.
	//
	// tbdispl with panel, no message - clear pending lines, rebuild scrollable area and display panel.
	// tbdispl with panel, message    - clear pending lines, rebuild scrollable area and display panel and message.
	// tbdispl no panel, no message   - retrieve next pending line.  If none, display panel.
	// tbdispl no panel, message      - display panel with message.  No rebuilding of the scrollable area.
	//
	// Set CRP to first changed line or tbtop if there are no selected lines.
	// ln is the tb screen line of the table CRP when invoking )REINIT and )PROC sections.
	//
	// If .AUTOSEL and .CSRROW set in panel, override the parameters p_autosel and p_csrrow.
	// Autoselect if the p_curpos CRN is visible.
	//
	// Store panel pointer in currtbPanel so that a CONTROL DISPLAY SAVE/RESTORE is only necessary
	// when a TBDISPL issues another TBDISPL and not for a display of an ordinary panel.
	//
	//
	// RC =  0  Normal completion.
	// RC =  4  More than 1 row selected.
	// RC =  8  End pressed.
	// RC = 12  Panel, message or cursor field not found.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	int ln ;
	int idr ;
	int exitRC ;

	int ztdtop ;
	int ztdsels ;
	int ztdrows ;
	int ztdvrows ;

	int    zscrolln ;
	string zscrolla ;

	bool rebuild = true ;

	uint id ;
	uint lcrp ;

	string zzverb ;

	string URID ;

	const string e1 = "Error during TBDISPL" ;
	const string e2 = "Error during TBDISPL of panel "+ p_name ;
	const string e3 = "Error processing )INIT section of panel " ;
	const string e4 = "Error processing )REINIT section of panel " ;
	const string e5 = "Error processing )PROC section of panel " ;
	const string e6 = "Error during update of panel " ;
	const string e7 = "Error updating field values of panel " ;
	const string e8 = "Error processing )ATTR section of panel " ;
	const string e9 = "Background job attempted to display panel " ;

	if ( !busyAppl )
	{
		llog( "E", "Invalid method state" <<endl ) ;
		llog( "E", "Application is in a wait state.  Method cannot invoke display services." <<endl ) ;
		RC = 20 ;
		return ;
	}

	if ( backgrd )
	{
		llog( "B", e9 + p_name <<endl ) ;
		RC = 8 ;
		return ;
	}

	RC = 0 ;
	ln = 0 ;

	if ( propagateEnd )
	{
		if ( !currtbPanel || currtbPanel->panelid == get_old_panelName() )
		{
			propagateEnd = false ;
		}
		else
		{
			set_nondispl_enter() ;
		}
	}

	if ( !tableNameOK( tb_name, e2 ) ) { return ; }

	if ( p_cursor != "" && !isvalidName( p_cursor ) )
	{
		errblk.setcall( TRACE_INFO(), e2, "PSYE023I", p_cursor ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( p_autosel != "YES" && p_autosel != "NO" && p_autosel != "" )
	{
		errblk.setcall( TRACE_INFO(), e2, "PSYE023J", p_autosel ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( p_crp_name != "" && !isvalidName( p_crp_name ) )
	{
		errblk.setcall( TRACE_INFO(), e2, "PSYE022O", "CRP", p_crp_name ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( p_rowid_name != "" && !isvalidName( p_rowid_name ) )
	{
		errblk.setcall( TRACE_INFO(), e2, "PSYE022O", "ROW", p_rowid_name ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( p_msgloc != "" && !isvalidName( p_msgloc ) )
	{
		errblk.setcall( TRACE_INFO(), e2, "PSYE031L", p_msgloc, "MSGLOC()" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( p_name != "" )
	{
		if ( currPanel )
		{
			currPanel->hide_popup() ;
		}
		currPanel = create_panel( p_name ) ;
		if ( errblk.error() ) { return ; }
		if ( !currPanel->tb_model )
		{
			errblk.setcall( TRACE_INFO(), e2, "PSYE021E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		currtbPanel = currPanel ;
		table_id[ tb_name ] = p_tableMGR->getid( errblk, tb_name ) ;
		CHECK_ERROR_SETCALL_RETURN( e2 )
		currPanel->tb_clear_linesChanged( errblk ) ;
	}
	else if ( currtbPanel && currtbPanel->table != tb_name )
	{
		currtbPanel->tb_clear_linesChanged( errblk ) ;
		currPanel = currtbPanel ;
	}
	else
	{
		id = p_tableMGR->getid( errblk, tb_name ) ;
		CHECK_ERROR_SETCALL_RETURN( e1 )
		auto it = table_id.find( tb_name ) ;
		if ( it != table_id.end() && id != it->second )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE021F", tb_name ) ;
			checkRCode( errblk ) ;
			return ;
		}
		if ( !currtbPanel )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE021C" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		currPanel = currtbPanel ;
	}

	if ( p_name != "" )
	{
		currPanel->init_control_variables() ;
	}

	if ( currPanel->table != tb_name )
	{
		currPanel->set_ztdvars( 0, 0, 0 ) ;
	}

	currPanel->get_ztdvars( ztdtop, ztdrows, ztdvrows ) ;

	update_ztdfvars( ztdtop, ztdrows, ztdvrows ) ;

	currPanel->set_msg( p_msg ) ;
	currPanel->tb_set_autosel( p_autosel == "YES" || p_autosel == "" ) ;
	currPanel->tb_set_csrrow( p_csrrow ) ;

	currPanel->set_msgloc( errblk, p_msgloc ) ;
	CHECK_ERROR_SETCALL_RETURN( e2 )

	currPanel->set_cursor( p_cursor, p_curpos ) ;

	currPanel->set_popup( errblk, addpop_active, addpop_row, addpop_col ) ;

	usr_action = USR_ENTER ;

	if ( p_name != "" )
	{
		currPanel->display_panel_init( errblk ) ;
		CHECK_ERROR_SETCALL_RETURN( e3 + p_name )
		currPanel->display_panel_attrs( errblk ) ;
		CHECK_ERROR_SETCALL_RETURN( e8 + p_name )
		set_ZVERB_panel_resp_re_init() ;
		set_panel_zvars() ;
	}
	else
	{
		if ( currPanel->tb_get_lineChanged( errblk, ln, URID ) )
		{
			currPanel->display_panel_reinit( errblk, ln ) ;
			CHECK_ERROR_SETCALL_RETURN( e4 + currPanel->panelid )
			currPanel->tb_remove_lineChanged() ;
			set_ZVERB_panel_resp_re_init() ;
			set_panel_zvars() ;
		}
		if ( !currPanel->tb_linesPending() || p_msg != "" )
		{
			rebuild = false ;
			p_name  = currPanel->panelid ;
		}
	}

	if ( currPanel->get_msg() != "" )
	{
		get_message( currPanel->get_msg() ) ;
		if ( RC > 0 ) { RC = 12 ; return ; }
		currPanel->set_panel_msg( zmsg, zmsgid ) ;
	}

	if ( currPanel->table != tb_name )
	{
		rebuild = true ;
		currPanel->table = tb_name ;
	}

	if ( rebuild && p_name != "" )
	{
		ztdtop = -1 ;
		p_tableMGR->fillfVARs( errblk,
				       funcPool,
				       tb_name,
				       currPanel->get_tb_fields(),
				       currPanel->get_tb_clear(),
				       currPanel->tb_lenvars(),
				       currPanel->get_tbscan(),
				       currPanel->tb_rdepth,
				       currPanel->tb_get_csrrow(),
				       ztdtop,
				       ztdrows,
				       ztdvrows,
				       idr ) ;
		if ( errblk.error() )
		{
			errblk.setcall( e2 ) ;
			errblk.setval( tb_name, p_name ) ;
			checkRCode( errblk ) ;
			return ;
		}
		currPanel->set_tblen() ;
		currPanel->set_ztdvars( ztdtop, ztdrows, ztdvrows ) ;
		update_ztdfvars( ztdtop, ztdrows, ztdvrows ) ;
		currPanel->set_cursor_idr( idr ) ;
		currPanel->update_field_values( errblk ) ;
		CHECK_ERROR_SETCALL_RETURN( e7 + p_name )
	}

	set_screenName() ;
	if ( p_name != "" )
	{
		set_panelName( p_name ) ;
	}

	while ( true )
	{
		if ( p_name != "" )
		{
			p_poolMGR->put( errblk, "ZPANELID", p_name, SHARED, SYSTEM ) ;
			tbquery( tb_name, lcrp ) ;
			currPanel->tb_set_crp( lcrp ) ;
			currPanel->display_panel( errblk ) ;
			CHECK_ERROR_SETCALL_RETURN( e2 )
			currPanel->cursor_placement_tbdispl( errblk ) ;
			CHECK_ERROR_SETCALL_RETURN( e2 )
			display_row_indicator() ;
			CHECK_ERROR_SETCALL_RETURN( e2 )
			if ( lineInOutDone && !controlNonDispl )
			{
				refreshlScreen = true ;
				wait_event( WAIT_USER ) ;
				lineInOutDone  = false ;
				refreshlScreen = false ;
			}
			else if ( !propagateEnd )
			{
				update_panels() ;
			}
			if ( currPanel->get_msg() == "" && !controlNonDispl )
			{
				clear_msg() ;
			}
			if ( !propagateEnd )
			{
				set_ZVERB( "" ) ;
			}
			wait_event( WAIT_USER ) ;
			if ( usr_action == USR_RETURN )
			{
				propagateEnd = true ;
			}
			controlDisplayLock = false ;
			refreshlScreen     = false ;
			reloadCUATables    = false ;
			currPanel->display_panel_update( errblk ) ;
			CHECK_ERROR_SETCALL_RETURN( e6 )
			if ( currPanel->get_msg() != "" )
			{
				get_message( currPanel->get_msg() ) ;
				if ( RC > 0 ) { return ; }
				currPanel->set_panel_msg( zmsg, zmsgid ) ;
				if ( !propagateEnd && !end_pressed() )
				{
					continue ;
				}
			}
			if ( currPanel->do_redisplay() ) { continue ; }
			currPanel->tb_set_linesChanged( errblk ) ;
			CHECK_ERROR_SETCALL_RETURN( e2 )
		}

		exitRC = 0  ;
		if ( currPanel->tb_curidx > -1 )
		{
			URID = funcPool->get3( errblk, ".ZURID." + d2ds( currPanel->tb_curidx ) ) ;
			CHECK_ERROR_RETURN()
			if ( URID != "" )
			{
				tbskip( tb_name, "", URID, "NOREAD", "ZCURINX" ) ;
			}
			else
			{
				funcPool->put2( errblk, "ZCURINX", 0 ) ;
			}
		}
		else
		{
			funcPool->put2( errblk, "ZCURINX", 0 ) ;
		}
		if ( currPanel->tb_get_lineChanged( errblk, ln, URID ) )
		{
			tbskip( tb_name, p_rowid_name, URID, "", p_crp_name ) ;
			for ( auto it = currPanel->tb_fields.begin() ; it != currPanel->tb_fields.end() ; ++it )
			{
				funcPool->put2( errblk, *it, funcPool->get3( errblk, *it + "." + d2ds( ln ) ) ) ;
				CHECK_ERROR_RETURN()
			}
		}

		ztdsels = funcPool->get1( errblk, 0, INTEGER, "ZTDSELS" ) ;
		if ( ztdsels > 1 ) { exitRC = 4 ; }

		zzverb = p_poolMGR->get( errblk, "ZVERB", SHARED ) ;
		if ( ztdsels == 0 && ( zzverb == "UP" || zzverb == "DOWN" ) )
		{
			zscrolla = p_poolMGR->get( errblk, "ZSCROLLA", SHARED ) ;
			zscrolln = ds2d( p_poolMGR->get( errblk, "ZSCROLLN", SHARED ) ) ;
			p_tableMGR->fillfVARs( errblk,
					       funcPool,
					       tb_name,
					       currPanel->get_tb_fields(),
					       currPanel->get_tb_clear(),
					       currPanel->tb_lenvars(),
					       currPanel->get_tbscan(),
					       currPanel->tb_rdepth,
					       0,
					       ztdtop,
					       ztdrows,
					       ztdvrows,
					       idr,
					       zzverb.front(),
					       ( zscrolla == "MAX" ) ? 'M' : ' ',
					       zscrolln,
					       ( ( currPanel->tb_pdepth - currPanel->get_tbred_lines() - 1 ) / currPanel->tb_modlines ) ) ;
			CHECK_ERROR_SETCALL_RETURN( e7 + p_name )
			currPanel->set_tblen() ;
			currPanel->set_ztdvars( ztdtop, ztdrows, ztdvrows ) ;
			update_ztdfvars( ztdtop, ztdrows, ztdvrows ) ;
			currPanel->update_field_values( errblk ) ;
			CHECK_ERROR_SETCALL_RETURN( e7 + p_name )
			currPanel->clear_cursor_cond() ;
			continue ;
		}

		currPanel->display_panel_proc( errblk, ln ) ;
		clr_nondispl() ;
		CHECK_ERROR_SETCALL_RETURN( e5 + p_name )
		set_ZVERB_panel_resp() ;
		set_panel_zvars() ;
		if ( propagateEnd || end_pressed() )
		{
			if ( usr_action == USR_RETURN )
			{
				propagateEnd = true ;
			}
			RC = 8 ;
			return ;
		}

		if ( currPanel->get_msg() != "" )
		{
			get_message( currPanel->get_msg() ) ;
			if ( RC > 0 ) { RC = 12 ; return ; }
			currPanel->set_panel_msg( zmsg, zmsgid ) ;
			if ( p_name == "" )
			{
				p_name = currPanel->panelid ;
				p_poolMGR->put( errblk, "ZPANELID", p_name, SHARED, SYSTEM ) ;
			}
			currPanel->display_panel_reinit( errblk, ln ) ;
			CHECK_ERROR_SETCALL_RETURN( e4 + p_name )
			set_ZVERB_panel_resp_re_init() ;
			set_panel_zvars() ;
			continue ;
		}
		break ;
	}

	if ( ztdsels == 0 )
	{
		tbtop( tb_name ) ;
		if ( p_crp_name != "" )
		{
			funcPool->put2( errblk, p_crp_name, 0 ) ;
			CHECK_ERROR_SETCALL_RETURN( e2 )
		}
		if ( p_rowid_name != "" )
		{
			funcPool->put2( errblk, p_rowid_name, "" ) ;
			CHECK_ERROR_SETCALL_RETURN( e2 )
		}
	}

	RC = exitRC ;
}


void pApplication::tbend( const string& tb_name )
{
	//
	// Close a table without saving (calls destroyTable routine).
	//
	// If opened share, use count is reduced and table removed from storage when use count = 0.
	//
	// RC =  0  Normal completion.
	// RC = 12  Table not open.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBEND Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	p_tableMGR->destroyTable( errblk, tb_name, "TBEND" ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )
	RC = 0 ;
}


void pApplication::tberase( const string& tb_name,
			    string tb_paths )
{
	//
	// Erase a table file from the table output library.
	//
	// If tb_paths is not specified, use ZTABL as the output library.  Error if blank.
	//
	// RC =  0  Normal completion.
	// RC =  8  Table does not exist.
	// RC = 12  Table in use.
	// RC = 16  Path does not exist.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBERASE Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	tb_paths = get_search_paths( tb_paths, s_ZTABL ) ;
	if ( errblk.error() ) { return ; }

	if ( tb_paths == "" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE013C", 16 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_tableMGR->tberase( errblk, tb_name, tb_paths ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbexist( const string& tb_name )
{
	//
	// Test for the existance of a row in a keyed table.
	//
	// RC =  0  Normal completion.
	// RC =  8  Row does not exist or not a keyed table.
	// RC = 12  Table not open.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBEXISTS Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	p_tableMGR->tbexist( errblk, funcPool, tb_name ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}



void pApplication::tbget( const string& tb_name,
			  const string& tb_save_name,
			  const string& tb_rowid_name,
			  const string& tb_noread,
			  const string& tb_crp_name )
{
	//
	// Access a row in a table.
	//
	// For table with keys, use the current value of the key in the dialogue variable.
	// For non-keyed tables, use the CRP.
	//
	// RC =  0  Normal completion.
	// RC =  8  CRP was at TOP(zero) for non-keyed tables or key not found for keyed tables.
	// RC = 20  Severe error.
	//
	// ROWID name and CRP name are for output only (not used for finding the record).
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBGET Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	p_tableMGR->tbget( errblk,
			   funcPool,
			   tb_name,
			   tb_save_name,
			   tb_rowid_name,
			   tb_noread,
			   tb_crp_name  ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbmod( const string& tb_name,
			  string tb_namelst,
			  const string& tb_order )
{
	//
	// Update/add a row in a table.
	//
	// Tables with keys        : Same as tbadd if row not found.
	// Tables with without keys: Same as tbadd.
	//
	// RC =  0  Normal completion. Keyed tables - row updated.  Non-keyed tables new row added.
	// RC =  8  Row did not match - row added for keyed tables.
	// RC = 16  Numeric conversion error.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBMOD Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	if ( tb_order != "" && tb_order != "ORDER" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE023A", "TBMOD", tb_order ) ;
		checkRCode( errblk ) ;
		return ;
	}

	getNameList( errblk, tb_namelst ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	p_tableMGR->tbmod( errblk,
			   funcPool,
			   tb_name,
			   tb_namelst,
			   tb_order ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbopen( const string& tb_name,
			   tbWRITE tb_WRITE,
			   string tb_paths,
			   tbDISP tb_DISP )
{
	//
	// Open an existing table, reading it from a file.
	//
	// If aleady opened in SHARE mode, increment use count.
	//
	// RC =  0  Normal completion.
	// RC =  8  Table does not exist in search path.
	// RC = 12  Table already open by this or another task.
	// RC = 16  paths not allocated.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBOPEN Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	tb_paths = get_search_paths( tb_paths, s_ZTLIB ) ;
	if ( errblk.error() ) { return ; }

	p_tableMGR->tbopen( errblk,
			    tb_name,
			    tb_WRITE,
			    tb_paths,
			    tb_DISP ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbput( const string& tb_name,
			  string tb_namelst,
			  const string& tb_order )
{
	//
	// Update the current row in a table
	//
	// Tables with keys        : Keys must match CRP row.
	// Tables with without keys: CRP row updated.
	//
	// RC =  0  Normal completion.
	// RC =  8  Keyed tables - key does not match current row.  CRP set to top (0).
	//          Non-keyed tables - CRP at top.
	// RC = 12  Table not open.
	// RC = 16  Numeric conversion error for sorted tables.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBPUT Service Error" ;

	RC = 0 ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	if ( tb_order != "" && tb_order != "ORDER" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE023A", "TBPUT", tb_order ) ;
		checkRCode( errblk ) ;
		return ;
	}

	getNameList( errblk, tb_namelst ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	p_tableMGR->tbput( errblk,
			   funcPool,
			   tb_name,
			   tb_namelst,
			   tb_order ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbquery( const string& tb_name,
			    const string& tb_keyn,
			    const string& tb_varn,
			    const string& tb_rownn,
			    const string& tb_keynn,
			    const string& tb_namenn,
			    const string& tb_crpn,
			    const string& tb_sirn,
			    const string& tb_lstn,
			    const string& tb_condn,
			    const string& tb_dirn )
{
	//
	// Return information about the specified table.
	//
	// RC =  0  Normal completion.
	// RC = 12  Table not open.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBQUERY Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	p_tableMGR->tbquery( errblk,
			     funcPool,
			     tb_name,
			     tb_keyn,
			     tb_varn,
			     tb_rownn,
			     tb_keynn,
			     tb_namenn,
			     tb_crpn,
			     tb_sirn,
			     tb_lstn,
			     tb_condn,
			     tb_dirn ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbquery( const string& tb_name,
			    uint& lcrp )
{
	//
	// Return CRP for the specified table (Internal use only).
	//
	// RC =  0  Normal completion.
	// RC = 12  Table not open.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBQUERY Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	p_tableMGR->tbquery( errblk,
			     tb_name,
			     lcrp ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbsarg( const string& tb_name,
			   const string& tb_namelst,
			   const string& tb_dir,
			   const string& tb_cond_pairs )
{
	//
	// Establish search arguments for scanning a table.
	//
	// RC =  0  Normal completion.
	// RC =  8  No search arguments set (all column variables null and namelst not specified).
	// RC = 12  Table not open.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBSARG Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	p_tableMGR->tbsarg( errblk,
			    funcPool,
			    tb_name,
			    tb_namelst,
			    tb_dir,
			    tb_cond_pairs ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbsave( const string& tb_name,
			   const string& tb_newname,
			   string tb_paths )
{
	//
	// Save the table to disk (calls saveTable routine).  Table remains open for processing.
	//
	// Table must be open in WRITE mode.
	//
	// If tb_paths is not specified, use ZTABL as the output path.  Error if blank.
	//
	// RC =  0  Normal completion.
	// RC = 12  Table not open or not open WRITE.
	// RC = 16  Alternate name save error.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	RC = 0 ;

	const string e1 = "TBSAVE Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	if ( tb_newname != "" && !isvalidName( tb_newname ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022J", tb_newname, "table" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( !p_tableMGR->writeableTable( errblk, tb_name, "TBSAVE" ) )
	{
		errblk.setcall( e1 ) ;
		if ( !errblk.error() )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE014S", tb_name, 12 ) ;
		}
		checkRCode( errblk ) ;
		return ;
	}

	tb_paths = get_search_paths( tb_paths, s_ZTABL ) ;
	if ( errblk.error() ) { return ; }

	if ( tb_paths == "" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE013C", 16 ) ;
		checkRCode( errblk ) ;
		return ;
	}

	p_tableMGR->saveTable( errblk,
			       "TBSAVE",
			       tb_name,
			       tb_newname,
			       tb_paths ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbscan( const string& tb_name,
			   const string& tb_namelst,
			   const string& tb_save_name,
			   const string& tb_rowid_name,
			   const string& tb_dir,
			   const string& tb_noread,
			   const string& tb_crp_name,
			   const string& tb_condlst )
{
	//
	// Scan table from current CRP according to parameters tb_namelst/tb_condlst/tb_dir if specified
	// or the search parameters set by a previous TBSARG call.
	//
	// RC =  0  Normal completion.
	// RC =  8  Row not found.  CRP set to top (zero).
	// RC = 12  Table not open.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBSCAN Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	p_tableMGR->tbscan( errblk,
			    funcPool,
			    tb_name,
			    tb_namelst,
			    tb_save_name,
			    tb_rowid_name,
			    tb_dir,
			    tb_noread,
			    tb_crp_name,
			    tb_condlst ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}



void pApplication::tbskip( const string& tb_name,
			   int num,
			   const string& tb_save_name,
			   const string& tb_rowid_name,
			   const string& tb_row,
			   const string& tb_noread,
			   const string& tb_crp_name )
{
	//
	// Move CRP to a position in the table and read the row into the dialogue variables.
	//
	// Position using tb_row if specified, then use num.
	//
	// RC =  0  Normal completion.
	// RC =  8  CRP would be outside the table or ROWID not found. CRP set to top (zero).
	// RC = 12  Table not open.
	// RC = 16  Truncation has occured.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBSKIP Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	p_tableMGR->tbskip( errblk,
			    funcPool,
			    tb_name,
			    num,
			    tb_save_name,
			    tb_rowid_name,
			    tb_row,
			    tb_noread,
			    tb_crp_name ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbskip( const string& tb_name,
			   const string& tb_rowid_name,
			   const string& tb_urid,
			   const string& tb_noread,
			   const string& tb_crp_name )
{
	//
	// Move CRP to a position in the table and read the row into the dialogue variables.
	//
	// Position using URID (Internal use only).
	//
	// RC =  0  Normal completion.
	// RC =  8  URID not found. CRP set to top (zero).
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBSKIP Service Error" ;

	p_tableMGR->tbskip( errblk,
			    funcPool,
			    tb_name,
			    tb_rowid_name,
			    tb_urid,
			    tb_noread,
			    tb_crp_name ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbsort( const string& tb_name,
			   string tb_fields )
{
	//
	// Sort a table and store the order in the sort information record.
	//
	// RC =  0  Normal completion.
	// RC = 12  Table not open.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBSORT Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	getNameList( errblk, tb_fields ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	p_tableMGR->tbsort( errblk,
			    tb_name,
			    tb_fields ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbstats( const string& tb_name,
			    const string& tb_cdate,
			    const string& tb_ctime,
			    const string& tb_udate,
			    const string& tb_utime,
			    const string& tb_user,
			    const string& tb_rowcreat,
			    const string& tb_rowcurr,
			    const string& tb_rowupd,
			    const string& tb_tableupd,
			    const string& tb_service,
			    const string& tb_retcode,
			    const string& tb_status1,
			    const string& tb_status2,
			    const string& tb_status3,
			    string tb_library,
			    const string& tb_virtsize,
			    const string& tb_cdate4d,
			    const string& tb_udate4d )
{
	//
	// Return table statistics.
	//
	// RC =  0  Normal completion (even if the table does not exist).
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBSTATS Service Error" ;

	string tb_paths ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	trim( iupper( tb_library ) ) ;

	if ( tb_library == "" )
	{
		tb_paths = get_libdef_search_paths( s_ZTLIB ) ;
	}
	else if ( tb_library.size() < 9 && tb_library.find( '/' ) == string::npos )
	{
		iupper( tb_library ) ;
		tb_paths = get_ddname_search_paths( tb_library ) ;
		if ( tb_paths == "" && has_libdef( tb_library ) )
		{
			tb_paths = get_libdef_search_paths( tb_library ) ;
			if ( errblk.error() ) { return ; }
		}
	}

	p_tableMGR->tbstats( errblk,
			     funcPool,
			     tb_name,
			     tb_cdate,
			     tb_ctime,
			     tb_udate,
			     tb_utime,
			     tb_user,
			     tb_rowcreat,
			     tb_rowcurr,
			     tb_rowupd,
			     tb_tableupd,
			     tb_service,
			     tb_retcode,
			     tb_status1,
			     tb_status2,
			     tb_status3,
			     tb_paths,
			     tb_virtsize,
			     tb_cdate4d,
			     tb_udate4d ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbtop( const string& tb_name )
{
	//
	// Set the current row pointer (CRP) to the top of the table, ahead of the first row.
	//
	// RC =  0  Normal completion.
	// RC = 12  Table not open.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBTOP Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	p_tableMGR->tbtop( errblk, tb_name ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::tbvclear( const string& tb_name )
{
	//
	// Set all dialogue variables for table columns to null.
	//
	// RC =  0  Normal completion.
	// RC = 12  Table not open.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	const string e1 = "TBVCLEAR Service Error" ;

	if ( !tableNameOK( tb_name, e1 ) ) { return ; }

	p_tableMGR->tbvclear( errblk, funcPool, tb_name ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::vcopy( const string& var,
			  string& val,
			  vcMODE mode )
{
	//
	// Retrieve a copy of a dialogue variable name in var and move to variable val.
	// (normal dialogue variable search order).
	// This routine is only valid for MODE=MOVE.
	//
	// RC =  0 Normal completion (add vmask if defined).
	// RC =  8 Variable not found.
	// RC = 16 Truncation occured.
	// RC = 20 Severe error.
	//
	// (funcPOOL.get returns 0, 8 or 20).
	// (funcPOOL.getfVAR returns 0, 8 or 20).
	// (poolMGR.get return 0, 8 or 20).
	//

	TRACE_FUNCTION() ;

	string mask ;

	fVAR* pvar ;

	VEDIT_TYPE vtype ;

	const string e1 = "VCOPY Service Error" ;

	switch ( mode )
	{
	case LOCATE:
		errblk.setcall( TRACE_INFO(), e1, "PSYE022A" ) ;
		checkRCode( errblk ) ;
		return ;

	case MOVE:
		pvar = funcPool->getfVAR( errblk, var ) ;
		if ( pvar )
		{
			val = pvar->sget( var ) ;
			if ( pvar->hasmask( mask, vtype ) )
			{
				add_vmask( val, zdatef, mask, vtype ) ;
			}
		}
		else if ( errblk.RC8() )
		{
			val = p_poolMGR->get( errblk, var, ASIS ) ;
			CHECK_ERROR_SETCALL_RETURN( e1 )
		}
		else
		{
			errblk.setcall( e1 ) ;
			checkRCode( errblk ) ;
			return ;
		}
	}
	RC = errblk.getRC() ;
}


void pApplication::vcopy( const string& var,
			  string*& p_val,
			  vcMODE mode )
{
	//
	// Return the address of a dialogue variable name in var, in p_val pointer.
	// (normal dialogue variable search order).
	// This routine is only valid for MODE=LOCATE.
	// MODE=LOCATE not valid for integer pointers as these may be in the variable pools as strings.
	//
	// RC =  0 Normal completion.
	// RC =  8 Variable not found.  p_val set to NULL.
	// RC = 16 Truncation occured. p_val set to NULL.
	// RC = 20 Severe error. p_val set to NULL.
	//
	// (funcPOOL.get returns 0, 8 or 20).
	// (funcPOOL.getfVAR returns 0, 8 or 20).
	// (funcPOOL.vlocate returns 0, 8 or 20).
	// (poolMGR.get return 0, 8 or 20).
	//

	TRACE_FUNCTION() ;

	const string e1 = "VCOPY Service Error" ;

	fVAR* pvar ;

	p_val = nullptr ;

	switch ( mode )
	{
	case LOCATE:
		pvar = funcPool->getfVAR( errblk, var ) ;
		if ( pvar )
		{
			if ( pvar->integer() )
			{
				errblk.setcall( TRACE_INFO(), e1, "PSYE022C" ) ;
				checkRCode( errblk ) ;
				return ;
			}
			if ( pvar->hasmask() )
			{
				errblk.setcall( TRACE_INFO(), e1, "PSYE044D", var ) ;
				checkRCode( errblk ) ;
				return ;
			}
			p_val = &pvar->sget( var ) ;
		}
		else if ( errblk.RC8() )
		{
			p_val = p_poolMGR->vlocate( errblk, var, ASIS ) ;
			CHECK_ERROR_SETCALL_RETURN( e1 )
		}
		else
		{
			errblk.setcall( e1 ) ;
			checkRCode( errblk ) ;
			return ;
		}
		break ;

	case MOVE:
		errblk.setcall( TRACE_INFO(), e1, "PSYE022B" ) ;
		checkRCode( errblk ) ;
		return ;

	}
	RC = errblk.getRC() ;
}


void pApplication::vdefine( const string& names,
			    int* i_ad1,
			    int* i_ad2,
			    int* i_ad3,
			    int* i_ad4,
			    int* i_ad5,
			    int* i_ad6,
			    int* i_ad7,
			    int* i_ad8,
			    int* i_ad9,
			    int* i_ad10,
			    int* i_ad11,
			    int* i_ad12 )
{
	//
	// Define a variable to the function pool.
	//
	// RC =  0 Normal completion.
	// RC = 20 Severe Error.
	//
	// (funcPOOL.define returns 0 or 20).
	//

	TRACE_FUNCTION() ;

	int w ;

	const string e1 = "VDEFINE Service Error" ;

	string name ;

	RC = 0 ;

	w = words( names ) ;
	if ( ( w > 12 ) || ( w < 1 ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022D" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( !i_ad1 )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
		checkRCode( errblk ) ;
		return ;
	}
	name = word( names, 1 ) ;
	funcPool->define( errblk, name, i_ad1 ) ;
	CHECK_ERROR_RETURN()

	if ( w > 1 )
	{
		if ( !i_ad2 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 2 ) ;
		funcPool->define( errblk, name, i_ad2 ) ;
		CHECK_ERROR_RETURN()
	}
	if ( w > 2 )
	{
		if ( !i_ad3 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 3 ) ;
		funcPool->define( errblk, name, i_ad3 ) ;
		CHECK_ERROR_RETURN()
	}
	if ( w > 3 )
	{
		if ( !i_ad4 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 4 ) ;
		funcPool->define( errblk, name, i_ad4 ) ;
		CHECK_ERROR_RETURN()
	}
	if ( w > 4 )
	{
		if ( !i_ad5 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 5 ) ;
		funcPool->define( errblk, name, i_ad5 ) ;
		CHECK_ERROR_RETURN()
	}
	if ( w > 5 )
	{
		if ( !i_ad6 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 6 ) ;
		funcPool->define( errblk, name, i_ad6 ) ;
		CHECK_ERROR_RETURN()
	}

	if ( w > 6 )
	{
		if ( !i_ad7 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 7 ) ;
		funcPool->define( errblk, name, i_ad7 ) ;
		CHECK_ERROR_RETURN()
	}

	if ( w > 7 )
	{
		if ( !i_ad8 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 8 ) ;
		funcPool->define( errblk, name, i_ad8 ) ;
		CHECK_ERROR_RETURN()
	}

	if ( w > 8 )
	{
		if ( !i_ad9 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 9 ) ;
		funcPool->define( errblk, name, i_ad9 ) ;
		CHECK_ERROR_RETURN()
	}

	if ( w > 9 )
	{
		if ( !i_ad10 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 10 ) ;
		funcPool->define( errblk, name, i_ad10 ) ;
		CHECK_ERROR_RETURN()
	}

	if ( w > 10 )
	{
		if ( !i_ad11 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 11 ) ;
		funcPool->define( errblk, name, i_ad11 ) ;
		CHECK_ERROR_RETURN()
	}

	if ( w > 11 )
	{
		if ( !i_ad12 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 12 ) ;
		funcPool->define( errblk, name, i_ad12 ) ;
		CHECK_ERROR_RETURN()
	}
}



void pApplication::vdefine( const string& names,
			    string* s_ad1,
			    string* s_ad2,
			    string* s_ad3,
			    string* s_ad4,
			    string* s_ad5,
			    string* s_ad6,
			    string* s_ad7,
			    string* s_ad8,
			    string* s_ad9,
			    string* s_ad10,
			    string* s_ad11,
			    string* s_ad12 )
{
	//
	// Define a variable to the function pool.
	//
	// RC =  0 Normal completion.
	// RC = 20 Severe Error.
	//
	// (funcPOOL.define returns 0 or 20).
	//

	TRACE_FUNCTION() ;

	int w  ;

	string name ;
	const string e1 = "VDEFINE Service Error" ;

	RC = 0 ;

	w = words( names ) ;
	if ( ( w > 12 ) || ( w < 1 ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022D" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( !s_ad1 )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
		checkRCode( errblk ) ;
		return ;
	}
	name = word( names, 1 ) ;
	funcPool->define( errblk, name, s_ad1 ) ;
	CHECK_ERROR_RETURN()

	if ( w > 1 )
	{
		if ( !s_ad2 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 2 ) ;
		funcPool->define( errblk, name, s_ad2 ) ;
		CHECK_ERROR_RETURN()
	}
	if ( w > 2 )
	{
		if ( !s_ad3 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 3 ) ;
		funcPool->define( errblk, name, s_ad3 ) ;
		CHECK_ERROR_RETURN()
	}
	if ( w > 3 )
	{
		if ( !s_ad4 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 4 ) ;
		funcPool->define( errblk, name, s_ad4 ) ;
		CHECK_ERROR_RETURN()
	}
	if ( w > 4 )
	{
		if ( !s_ad5 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 5 ) ;
		funcPool->define( errblk, name, s_ad5 ) ;
		CHECK_ERROR_RETURN()
	}
	if ( w > 5 )
	{
		if ( !s_ad6 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 6 ) ;
		funcPool->define( errblk, name, s_ad6 ) ;
		CHECK_ERROR_RETURN()
	}
	if ( w > 6 )
	{
		if ( !s_ad7 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 7 ) ;
		funcPool->define( errblk, name, s_ad7 ) ;
		CHECK_ERROR_RETURN()
	}
	if ( w > 7 )
	{
		if ( !s_ad8 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 8 ) ;
		funcPool->define( errblk, name, s_ad8 ) ;
		CHECK_ERROR_RETURN()
	}
	if ( w > 8 )
	{
		if ( !s_ad9 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 9 ) ;
		funcPool->define( errblk, name, s_ad9 ) ;
		CHECK_ERROR_RETURN()
	}
	if ( w > 9 )
	{
		if ( !s_ad10 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 10 ) ;
		funcPool->define( errblk, name, s_ad10 ) ;
		CHECK_ERROR_RETURN()
	}
	if ( w > 10 )
	{
		if ( !s_ad11 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 11 ) ;
		funcPool->define( errblk, name, s_ad11 ) ;
		CHECK_ERROR_RETURN()
	}
	if ( w > 11 )
	{
		if ( !s_ad12 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022E" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		name = word( names, 12 ) ;
		funcPool->define( errblk, name, s_ad12 ) ;
		CHECK_ERROR_RETURN()
	}
}


void pApplication::vdelete( const string& names )
{
	//
	// Remove defined variables from the function pool.
	//
	// RC =  0 Normal completion.
	// RC =  8 At least one variable not found.
	// RC = 20 Severe error.
	//
	// (funcPOOL.del returns 0, 8 or 20).
	//

	TRACE_FUNCTION() ;

	const string e1 = "VDELETE Service Error" ;

	int i ;
	int ws = words( names ) ;

	if ( ws == 0 )
	{
		RC = 0 ;
		return ;
	}

	errblk.setmaxRC( 0 ) ;

	for ( i = 1 ; i <= ws ; ++i )
	{
		funcPool->del( errblk, word( names, i ) ) ;
		CHECK_ERROR_SETCALL_RETURN( e1 )
		errblk.setmaxRC() ;
	}

	RC = errblk.getmaxRC() ;
}


void pApplication::vdelete( const string& names1,
			    const string& names2 )
{
	//
	// Remove defined variables from the function pool.
	//
	// RC =  0 Normal completion.
	// RC =  8 At least one variable not found.
	// RC = 20 Severe error.
	//
	// (funcPOOL.del returns 0, 8 or 20).
	//

	TRACE_FUNCTION() ;

	int maxRC ;

	vdelete( names1 ) ;
	if ( errblk.error() ) { return ; }

	maxRC = RC ;

	vdelete( names2 ) ;
	RC = max( maxRC, RC ) ;
}


void pApplication::vdelete( const string& names1,
			    const string& names2,
			    const string& names3 )
{
	//
	// Remove defined variables from the function pool.
	//
	// RC =  0 Normal completion.
	// RC =  8 At least one variable not found.
	// RC = 20 Severe error.
	//
	// (funcPOOL.del returns 0, 8 or 20).
	//

	TRACE_FUNCTION() ;

	int maxRC ;

	vdelete( names1 ) ;
	if ( errblk.error() ) { return ; }
	maxRC = RC ;

	vdelete( names2 ) ;
	if ( errblk.error() ) { return ; }
	maxRC = max( maxRC, RC ) ;

	vdelete( names3 ) ;
	RC = max( maxRC, RC ) ;
}


void pApplication::vdelete( const string& names1,
			    const string& names2,
			    const string& names3,
			    const string& names4 )
{
	//
	// Remove defined variables from the function pool.
	//
	// RC =  0 Normal completion.
	// RC =  8 At least one variable not found.
	// RC = 20 Severe error.
	//
	// (funcPOOL.del returns 0, 8 or 20).
	//

	TRACE_FUNCTION() ;

	int maxRC ;

	vdelete( names1 ) ;
	if ( errblk.error() ) { return ; }
	maxRC = RC ;

	vdelete( names2 ) ;
	if ( errblk.error() ) { return ; }
	maxRC = max( maxRC, RC ) ;

	vdelete( names3 ) ;
	if ( errblk.error() ) { return ; }
	maxRC = max( maxRC, RC ) ;

	vdelete( names4 ) ;
	RC = max( maxRC, RC ) ;
}


void pApplication::verase( const string& names,
			   poolType pType )
{
	//
	// Remove variables from the profile or shared pool.
	//
	// RC =  0 Normal completion.
	// RC =  8 Variable not found.
	// RC = 12 Read-only variable.
	// RC = 20 Severe error.
	//
	// (poolMGR.erase return 0, 8 12 or 20).
	//

	TRACE_FUNCTION() ;

	int i  ;
	int ws ;

	const string e1 = "VERASE Service Error" ;
	string name ;

	errblk.setmaxRC( 0 ) ;

	ws = words( names ) ;
	for ( i = 1 ; i <= ws ; ++i )
	{
		name = word( names, i ) ;
		p_poolMGR->erase( errblk, name, pType ) ;
		CHECK_ERROR_SETCALL_RETURN( e1 )
		errblk.setmaxRC() ;
	}
	RC = errblk.getmaxRC() ;
}


void pApplication::vget( const string& names,
			 poolType pType )
{
	//
	// Copy dialogue variables from the profile or shared pool to the function pool.
	//
	// RC =  0 Normal completion (remove vmask if defined).
	// RC =  8 Variable not found (set function pool var to 0/null).
	// RC = 12 Validation failed.
	// RC = 20 Severe error.
	//
	// (funcPOOL.put returns 0 or 20).
	// (funcPOOL.getfVAR returns 0, 8 or 20.  For RC = 8 create implicit function pool variable).
	// (poolMGR.get return 0, 8 or 20).
	//

	TRACE_FUNCTION() ;

	int ws ;
	int i  ;

	bool found ;

	string val  ;
	string name ;
	string mask ;

	const string e1 = "VGET Service Error" ;

	VEDIT_TYPE vtype ;

	fVAR* pvar ;

	errblk.setmaxRC( 0 ) ;

	for ( ws = words( names ), i = 1 ; i <= ws ; ++i )
	{
		name = word( names, i ) ;
		val  = p_poolMGR->get( errblk, name, pType ) ;
		CHECK_ERROR_SETCALL_RETURN( e1 )
		errblk.setmaxRC() ;
		found = errblk.RC0() ;
		pvar  = funcPool->getfVAR( errblk, name ) ;
		CHECK_ERROR_SETCALL_RETURN( e1 )
		if ( pvar )
		{
			if ( pvar->hasmask( mask, vtype ) )
			{
				remove_vmask( errblk, zdatef, name, val, mask, vtype ) ;
				CHECK_ERROR_SETCALL_RETURN( e1 )
			}
			if ( pvar->integer() )
			{
				pvar->put( errblk, name, ( found ) ? val : "0" ) ;
			}
			else
			{
				pvar->put( errblk, name, val );
			}
			CHECK_ERROR_SETCALL_RETURN( e1 )
		}
		else
		{
			funcPool->put2( errblk, name, val ) ;
		}
		CHECK_ERROR_SETCALL_RETURN( e1 )
	}
	RC = errblk.getmaxRC() ;
}


void pApplication::view( const string& m_file,
			 const string& m_panel,
			 const string& m_macro,
			 const string& m_profile,
			 const string& m_dataid,
			 const string& m_lcmds,
			 const string& m_confirm,
			 const string& m_chgwarn,
			 const string& m_parm )
{
	TRACE_FUNCTION() ;

	const string e1 = "VIEW Service Service Error" ;

	edit_parms e ;

	if ( isvalidName( upper( m_file ) ) )
	{
		vcopy( upper( m_file ), e.edit_file, MOVE ) ;
	}
	else
	{
		e.edit_file = m_file ;
	}

	e.edit_panel    = m_panel ;
	e.edit_macro    = m_macro ;
	e.edit_profile  = m_profile ;
	e.edit_lcmds    = m_lcmds   ;
	e.edit_confirm  = m_confirm ;
	e.edit_chgwarn  = m_chgwarn ;
	if ( m_parm != "" )
	{
		if ( m_macro == "" )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE023S", m_parm ) ;
			checkRCode( errblk ) ;
			return ;
		}
		vcopy( m_parm, e.edit_parm, MOVE ) ;
		if ( RC > 0 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE023T", m_parm ) ;
			checkRCode( errblk ) ;
			return ;
		}
	}

	if ( m_dataid != "" )
	{
		e.edit_dataid = m_dataid ;
		e.edit_file   = get_file_for_dataid( m_dataid ) ;
		if ( e.edit_file == "" )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE054I", m_dataid ) ;
			checkRCode( errblk ) ;
			return ;
		}
	}

	e.edit_viewmode = true ;
	selct.clear() ;
	selct.pgm     = p_poolMGR->get( errblk, "ZVIEWPGM", PROFILE ) ;
	selct.newappl = ""    ;
	selct.newpool = false ;
	selct.passlib = false ;
	selct.suspend = true  ;
	selct.options = (void*)&e ;
	selct.ptid    = ptid  ;
	selct.stid    = taskId ;
	selct.service = true  ;
	selct.errret  = controlErrorsReturn ;
	selct.backgrd = backgrd ;
	actionSelect() ;

	RC = ZRC ;
	if ( ZRC > 11 && isvalidName( ZRESULT ) )
	{
		errblk.setcall( TRACE_INFO(), "VIEW Error", ZRESULT, get_shared_var( "ZVAL1" ), ZRC ) ;
		checkRCode( errblk ) ;
	}
}


void pApplication::vmask( const string& names,
			  const string& type,
			  const string& mask )
{
	//
	// Set a mask for a function pool variable (must be vdefined first).
	//
	// RC =  0 Normal completion.
	// RC =  8 Variable not found.
	// RC = 20 Severe error.
	//
	// (funcPOOL.setmask returns 0, 8 or 20).
	//

	TRACE_FUNCTION() ;

	string name ;

	int ws ;
	int p  ;
	int i  ;

	VEDIT_TYPE vtype ;

	const string e1    = "VMASK Service Error" ;
	const string fmask = "IDATE STDDATE ITIME STDTIME JDATE JSTD" ;
	const string valid_uchars = "AB9HNVSX()-/,." ;
	const string mndry_uchars = "A9HNX" ;

	errblk.setmaxRC( 0 ) ;

	if ( type == "FORMAT" )
	{
		p = wordpos( mask, fmask ) ;
		switch ( p )
		{
		case 1:
			vtype = VED_IDATE ;
			break ;

		case 2:
			vtype = VED_STDDATE ;
			break ;

		case 3:
			vtype = VED_ITIME ;
			break ;

		case 4:
			vtype = VED_STDTIME ;
			break ;

		case 5:
			vtype = VED_JDATE ;
			break ;

		case 6:
			vtype = VED_JSTD ;
			break ;

		default:
			errblk.setcall( TRACE_INFO(), e1, "PSYE022P", mask, fmask ) ;
			checkRCode( errblk ) ;
			return ;
		}
	}
	else if ( type == "USER" )
	{
		vtype = VED_USER ;
		if ( mask.size() > 20 || mask.size() == 0 )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022Q" ) ;
			checkRCode( errblk ) ;
			return ;
		}
		else
		{
			if ( mask.find_first_not_of( valid_uchars ) != string::npos ||
			     mask.find_first_of( mndry_uchars ) == string::npos ||
			     countc( mask, 'S' ) > 1 ||
			   ( mask.find( 'S' ) != string::npos && mask.front() != 'S' ) )
			{
				errblk.setcall( TRACE_INFO(), e1, "PSYE022S", mask ) ;
				checkRCode( errblk ) ;
				return ;
			}
		}
	}
	else
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE022R", type ) ;
		checkRCode( errblk ) ;
		return ;
	}

	ws = words( names ) ;
	for ( i = 1 ; i <= ws ; ++i )
	{
		name = word( names, i ) ;
		funcPool->setmask( errblk, name, mask, vtype ) ;
		CHECK_ERROR_SETCALL_RETURN( e1 )
		errblk.setmaxRC() ;
	}
	RC = errblk.getmaxRC() ;
}


void pApplication::vput( const string& names,
			 poolType pType )
{
	//
	// Copy dialogue variables from the function pool to the profile or shared pool.
	//
	// RC =  0 Normal completion (add vmask if defined).
	// RC =  8 Variable not found in function pool.
	// RC = 12 Read-only variable.
	// RC = 16 Truncation occured.
	// RC = 20 Severe error.
	//
	// (funcPOOL.get returns 0, 8 or 20).
	// (funcPOOL.getfVAR returns 0, 8 or 20).
	// (poolMGR.put return 0, 12, 16 or 20).
	//

	TRACE_FUNCTION() ;

	int i  ;
	int ws ;

	string s_val ;
	string name ;
	string mask ;

	const string e1 = "VPUT Service Error" ;

	VEDIT_TYPE vtype ;

	fVAR* pvar ;

	errblk.setmaxRC( 0 ) ;

	ws = words( names ) ;
	for ( i = 1 ; i <= ws ; ++i )
	{
		name = word( names, i ) ;
		pvar = funcPool->getfVAR( errblk, name ) ;
		if ( pvar )
		{
			s_val = pvar->sget( name ) ;
			if ( pvar->hasmask( mask, vtype ) )
			{
				add_vmask( s_val, zdatef, mask, vtype ) ;
			}
			p_poolMGR->put( errblk, name, s_val, pType ) ;
		}
		CHECK_ERROR_SETCALL_RETURN( e1 )
		errblk.setmaxRC() ;
	}
	RC = errblk.getmaxRC() ;
}


void pApplication::vreplace( const string& name,
			     const string& s_val )
{
	//
	// Update the contents of the function pool.
	//
	// RC =  0 Normal completion.
	// RC = 20 Severe error.
	//
	// (funcPOOL.put returns 0, 20).
	//

	TRACE_FUNCTION() ;

	const string e1 = "VREPLACE Service Error" ;

	funcPool->put1( errblk, name, s_val ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::vreplace( const string& name,
			     int i_val )
{
	//
	// Update the contents of the function pool.
	//
	// RC =  0 Normal completion.
	// RC = 20 Severe error.
	//
	// (funcPOOL.put returns 0 or 20).
	//

	TRACE_FUNCTION() ;

	const string e1 = "VREPLACE Service Error" ;

	funcPool->put1( errblk, name, i_val ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	RC = errblk.getRC() ;
}


void pApplication::vreset()
{
	//
	// Remove implicit and defined variables from the function pool.
	// Redefine funtion pool system variables (unless NOFUNC has been specified).
	//
	// RC =  0 Normal completion.
	// RC = 20 Severe error.
	//
	// (funcPOOL.reset returns 0).
	//

	TRACE_FUNCTION() ;

	funcPool->reset( errblk ) ;
	RC = errblk.getRC() ;

	if ( !nofunc )
	{
		funcPool->define( errblk, "ZRC", &ZRC ) ;
		funcPool->define( errblk, "ZRSN", &ZRSN ) ;
		funcPool->define( errblk, "ZRESULT", &ZRESULT ) ;
	}

	funcPool->put2( errblk, "ZTDTOP",   1 ) ;
	funcPool->put2( errblk, "ZTDSELS",  0 ) ;
	funcPool->put2( errblk, "ZTDDEPTH", 0 ) ;
	funcPool->put2( errblk, "ZTDROWS",  0 ) ;
	funcPool->put2( errblk, "ZTDVROWS", 0 ) ;
	funcPool->put2( errblk, "ZCURPOS",  1 ) ;
	funcPool->put2( errblk, "ZCURINX",  0 ) ;
	funcPool->put2( errblk, "ZSBTASK",  0 ) ;
}


/* *********************************************** ***************************** ************************************************ */
/* ***********************************************      End of DM Services       ************************************************ */
/* *********************************************** ***************************** ************************************************ */


/* *************************************** ********************************************* **************************************** */
/* ***************************************      Start of other Application Services      **************************************** */
/* *************************************** ********************************************* **************************************** */


void pApplication::enqv( const string& qnamev,
			 const string& rnamev,
			 enqDISP disp,
			 enqSCOPE scope )
{
	//
	// ENQ a resource.
	//
	// QNAME and RNAME are passed via dialogue variables.
	//
	// RC =  0 Normal completion.
	// RC =  8 Enqueue already held by this, or another task if exclusive requested.
	// RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	errblk.setRC( 0 ) ;

	const string e1 = "ENQ Service Error" ;

	RC = 0 ;

	string qname ;
	string rname ;

	if ( !isvalidName( qnamev ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE031D", qnamev ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( !isvalidName( rnamev ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE031D", rnamev ) ;
		checkRCode( errblk ) ;
		return ;
	}

	vcopy( qnamev, qname ) ;
	vcopy( rnamev, rname ) ;

	enq( qname, rname, disp, scope ) ;
}


void pApplication::enq( const string& maj,
			const string& min,
			enqDISP disp,
			enqSCOPE scope )
{
	//
	// ENQ a resource.
	//
	// RC =  0 Normal completion.
	// RC =  8 Enqueue already held by this, or another task if exclusive requested.
	// RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	errblk.setRC( 0 ) ;

	const string e1 = "ENQ Service Error" ;

	RC = 0 ;

	check_qrname( e1, maj, min ) ;
	if ( errblk.error() ) { return ; }

	if ( scope == GLOBAL )
	{
		RC = p_gls->enq( maj, min, taskid(), disp, scope ) ;
		return ;
	}

	for ( auto it = l_enqueues.begin() ; it != l_enqueues.end() ; ++it )
	{
		if ( it->maj_name == maj && it->min_name == min )
		{
			if ( it->disp == SHR && disp == SHR && it->tasks.count( taskId ) == 0 )
			{
				it->tasks.insert( taskId ) ;
				return ;
			}
			RC = 8 ;
			return ;
		}
	}

	l_enqueues.push_back( enqueue( maj, min, taskId, disp ) ) ;
}


void pApplication::deqv( const string& qnamev,
			 const string& rnamev,
			 enqSCOPE scope )
{
	//
	// DEQ a resource.
	//
	// QNAME and RNAME are passed via dialogue variables.
	//
	// RC =  0 Normal completion.
	// RC =  8 Enqueue not held by this task.
	// RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	errblk.setRC( 0 ) ;

	const string e1 = "DEQ Service Error" ;

	RC = 0 ;

	string qname ;
	string rname ;

	if ( !isvalidName( qnamev ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE031D", qnamev ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( !isvalidName( rnamev ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE031D", rnamev ) ;
		checkRCode( errblk ) ;
		return ;
	}

	vcopy( qnamev, qname ) ;
	vcopy( rnamev, rname ) ;

	deq( qname, rname, scope ) ;
}


void pApplication::deq( const string& maj,
			const string& min,
			enqSCOPE scope )
{
	//
	// DEQ a resource.
	//
	// RC =  0 Normal completion.
	// RC =  8 Enqueue not held by this task.
	// RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	errblk.setRC( 0 ) ;

	const string e1 = "DEQ Service Error" ;

	check_qrname( e1, maj, min ) ;
	if ( errblk.error() ) { return ; }

	if ( scope == GLOBAL )
	{
		RC = p_gls->deq( maj, min, taskid(), scope ) ;
		return ;
	}

	RC = 8 ;
	for ( auto it = l_enqueues.begin() ; it != l_enqueues.end() ; ++it )
	{
		if ( it->maj_name == maj && it->min_name == min )
		{
			if ( it->tasks.count( taskId ) > 0 )
			{
				it->tasks.erase( taskId ) ;
				if ( it->tasks.size() == 0 )
				{
					l_enqueues.erase( it ) ;
				}
				RC = 0 ;
			}
			break ;
		}
	}
}


void pApplication::qscanv( const string& qnamev,
			   const string& rnamev,
			   enqDISP disp,
			   enqSCOPE scope )
{
	//
	// Scan for an enqueue.  Match on qname, rname, disposition and scope.
	//
	// QNAME and RNAME are passed via dialogue variables.
	//
	// RC =  0 Enqueue held.
	// RC =  8 Enqueue is not held.
	// RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	errblk.setRC( 0 ) ;

	const string e1 = "QSCAN Service Error" ;

	RC = 0 ;

	string qname ;
	string rname ;

	if ( !isvalidName( qnamev ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE031D", qnamev ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( !isvalidName( rnamev ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE031D", rnamev ) ;
		checkRCode( errblk ) ;
		return ;
	}

	vcopy( qnamev, qname ) ;
	vcopy( rnamev, rname ) ;

	qscan( qname, rname, disp, scope ) ;
}


void pApplication::qscan( const string& maj,
			  const string& min,
			  enqDISP disp,
			  enqSCOPE scope )
{
	//
	// Scan for an enqueue.  Match on qname, rname, disposition and scope.
	//
	// RC =  0 Enqueue held.
	// RC =  8 Enqueue is not held.
	// RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	errblk.setRC( 0 ) ;

	const string e1 = "QSCAN Service Error" ;

	RC = 8 ;

	check_qrname( e1, maj, min ) ;
	if ( errblk.error() ) { return ; }

	if ( scope == GLOBAL )
	{
		RC = p_gls->qscan( maj, min, disp, scope ) ;
		return ;
	}

	for ( auto it = l_enqueues.begin() ; it != l_enqueues.end() ; ++it )
	{
		if ( it->maj_name == maj && it->min_name == min )
		{
			if ( it->disp == disp )
			{
				RC = 0 ;
			}
			return ;
		}
	}
}


/* *************************************** ********************************************* **************************************** */
/* ***************************************       End of other Application Services       **************************************** */
/* *************************************** ********************************************* **************************************** */


void pApplication::set_ZVERB_panel_resp_re_init()
{
	TRACE_FUNCTION() ;

	string dotResp = currPanel->get_resp() ;

	if ( currPanel->get_msg() == "" )
	{
		if ( dotResp == "ENTER" )
		{
			set_ZVERB( "" ) ;
			set_userAction( USR_ENTER ) ;
			set_nondispl_enter() ;
			propagateEnd = false ;
		}
		else if ( dotResp == "END" )
		{
			set_ZVERB( "END" ) ;
			set_userAction( USR_END ) ;
			set_nondispl_end() ;
			propagateEnd = false ;
		}
	}
	else if ( dotResp == "ENTER" || dotResp == "END" )
	{
		clr_nondispl() ;
		propagateEnd = false ;
	}
}


void pApplication::set_ZVERB_panel_resp()
{
	TRACE_FUNCTION() ;

	const string& dotResp = currPanel->get_resp() ;

	if ( dotResp == "ENTER" )
	{
		set_ZVERB( "" ) ;
		set_userAction( USR_ENTER ) ;
		propagateEnd = false ;
	}
	else if ( dotResp == "END" )
	{
		set_ZVERB( "END" ) ;
		set_userAction( USR_END ) ;
		propagateEnd = false ;
	}
}


void pApplication::set_ZVERB( const string& zverb )
{
	TRACE_FUNCTION() ;

	p_poolMGR->put( errblk, "ZVERB", zverb, SHARED ) ;
}


void pApplication::set_panel_zvars()
{
	TRACE_FUNCTION() ;

	funcPool->put2( errblk, "ZCURFLD", currPanel->get_cursor() ) ;
	funcPool->put2( errblk, "ZCURPOS", currPanel->get_csrpos() ) ;
}


void pApplication::update_ztdfvars( int ztdtop,
				    int ztdrows,
				    int ztdvrows )
{
	TRACE_FUNCTION() ;

	funcPool->put2( errblk, "ZTDTOP", ztdtop ) ;
	funcPool->put2( errblk, "ZTDROWS", ztdrows ) ;
	funcPool->put2( errblk, "ZTDVROWS", ztdvrows ) ;
}


void pApplication::display_row_indicator()
{
	//
	// Set the row indicator message for table displays.
	// Use ZTDMSG.  If not set, use PSYZ003 for ROWS(ALL) and PSYZ005 for ROWS(SCAN)
	//

	TRACE_FUNCTION() ;

	string ztdmsg ;
	string ztdsmsg ;
	string ztdlmsg ;

	vcopy( "ZTDMSG", ztdmsg ) ;

	get_message( ( ztdmsg == "" ) ? ( currPanel->get_tbscan() ) ? "PSYZ005" : "PSYZ003" : ztdmsg, ztdsmsg, ztdlmsg ) ;

	currPanel->display_row_indicator( errblk, ( ztdsmsg == "" ) ? ztdlmsg : ztdsmsg ) ;
}


string pApplication::get_search_paths( string tb_paths,
				       s_paths p )
{
	//
	// Resolve the search paths.
	//
	// If no path specified, get the default libdef search path, p.
	// If path specified and not a path, return ddname paths, or libdef paths.
	//

	TRACE_FUNCTION() ;

	string lib ;

	if ( tb_paths == "" )
	{
		tb_paths = get_libdef_search_paths( p ) ;
	}
	else if ( tb_paths.size() < 9 && tb_paths.find( '/' ) == string::npos )
	{
		lib      = upper( tb_paths ) ;
		tb_paths = get_ddname_search_paths( lib ) ;
		if ( tb_paths == "" )
		{
			tb_paths = get_libdef_search_paths( lib ) ;
			if ( errblk.error() ) { return "" ; }
		}
	}

	return tb_paths ;
}


string pApplication::get_libdef_search_paths( s_paths p )
{
	//
	// Return the search path depending on the LIBDEFs in effect.
	//
	// Search Order:
	//                No LIBDEF       LIBDEF       LIBDEF
	//                                with PATH    with LIBRARY
	//                -----------------------------------------
	//                                ZMUSR
	// Messages       ZMLIB           LIBDEF       LIBDEF
	//                                ZMLIB        ZMLIB
	//                -----------------------------------------
	//                                ZPUSR
	// Panels         ZPLIB           LIBDEF       LIBDEF
	//                                ZPLIB        ZPLIB
	//                -----------------------------------------
	//                                ZSUSR
	// Skeleton Input ZSLIB           LIBDEF       LIBDEF
	//                                ZSLIB        ZSLIB
	//                -----------------------------------------
	//                                ZLUSR
	// Load           ZLLIB           LIBDEF       LIBDEF
	//                                ZLLIB        ZLLIB
	//                -----------------------------------------
	//                                ZTUSR
	// Table Input    ZTLIB           LIBDEF       LIBDEF
	//                                ZTLIB        ZTLIB
	//                -----------------------------------------
	//                                ZTABU
	// Table Output   ZTABL           LIBDEF       LIBDEF
	//                -----------------------------------------
	//                                ZFILU
	// FT Output      ZFILE           LIBDEF       LIBDEF
	//                -----------------------------------------
	//

	TRACE_FUNCTION() ;

	string* zzxusr = nullptr ;
	string* zzxlib = nullptr ;

	map<string,stack<zlibdef>>::iterator it ;

	switch ( p )
	{
	case s_ZMLIB:
		zzxusr = &zzmusr ;
		it     = zlibd.find( "ZMLIB" ) ;
		zzxlib = &zzmlib ;
		break ;

	case s_ZPLIB:
		zzxusr = &zzpusr ;
		it     = zlibd.find( "ZPLIB" ) ;
		zzxlib = &zzplib ;
		break ;

	case s_ZSLIB:
		zzxusr = &zzsusr ;
		it     = zlibd.find( "ZSLIB" ) ;
		zzxlib = &zzslib ;
		break ;

	case s_ZLLIB:
		zzxusr = &zzlusr ;
		it     = zlibd.find( "ZLLIB" ) ;
		zzxlib = &zzllib ;
		break ;

	case s_ZTLIB:
		zzxusr = &zztusr ;
		it     = zlibd.find( "ZTLIB" ) ;
		zzxlib = &zztlib ;
		break ;

	case s_ZTABL:
		zzxusr = &zztabu ;
		it     = zlibd.find( "ZTABL" ) ;
		if ( it == zlibd.end() )
		{
			zzxlib = &zztabl ;
		}
		break ;

	case s_ZFILE:
		zzxusr = &zzfilu ;
		it     = zlibd.find( "ZFILE" ) ;
		if ( it == zlibd.end() )
		{
			zzxlib = &zzfile ;
		}
		break ;
	}

	if ( it == zlibd.end() )
	{
		return *zzxlib ;
	}

	const zlibdef& ldef = it->second.top() ;

	if ( !zzxlib )
	{
		return ( ldef.library() ) ? ldef.paths : mergepaths( *zzxusr, ldef.paths ) ;
	}
	else if ( *zzxusr == "" || ldef.library() )
	{
		return mergepaths( ldef.paths, *zzxlib ) ;
	}
	else
	{
		return mergepaths( *zzxusr, ldef.paths, *zzxlib ) ;
	}
}


string pApplication::get_libdef_search_paths( const string& lib )
{
	//
	// Return the search path depending on the LIBDEFs in effect.
	// ISPxLIB is an alias for ZxLIB.
	//
	// Generic libraries can only be used for table input, table output and file tailoring output.
	//
	//                No LIBDEF       LIBDEF       LIBDEF
	//                                with PATH    with LIBRARY
	//                -----------------------------------------
	// Table Input    Allocated
	// (LIBRARY)      library         Unchanged    LIBDEF
	//                -----------------------------------------
	// Table Output   Allocated
	// (LIBRARY)      library         LIBDEF       Unchanged
	//                -----------------------------------------
	// FT Output      Allocated
	// (LIBRARY)      library         LIBDEF       Unchanged
	//                -----------------------------------------
	//

	TRACE_FUNCTION() ;

	string ddlibs ;

	const string e1 = "LIBRARY Service Error" ;

	if ( !isvalidName( lib ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYS012U" ) ;
		checkRCode( errblk ) ;
		return "" ;
	}

	if      ( lib == "ZMLIB" || lib == "ISPMLIB" ) { return get_libdef_search_paths( s_ZMLIB ) ; }
	else if ( lib == "ZPLIB" || lib == "ISPPLIB" ) { return get_libdef_search_paths( s_ZPLIB ) ; }
	else if ( lib == "ZSLIB" || lib == "ISPSLIB" ) { return get_libdef_search_paths( s_ZSLIB ) ; }
	else if ( lib == "ZLLIB" || lib == "ISPLLIB" ) { return get_libdef_search_paths( s_ZLLIB ) ; }
	else if ( lib == "ZTLIB" || lib == "ISPTLIB" ) { return get_libdef_search_paths( s_ZTLIB ) ; }
	else if ( lib == "ZTABL" || lib == "ISPTABL" ) { return get_libdef_search_paths( s_ZTABL ) ; }
	else if ( lib == "ZFILE" || lib == "ISPFILE" ) { return get_libdef_search_paths( s_ZFILE ) ; }

	ddlibs  = get_ddname_search_paths( lib ) ;
	auto it = zlibd.find( lib ) ;

	if ( ddlibs == "" )
	{
		if ( it == zlibd.end() )
		{
			errblk.setcall( TRACE_INFO(), e1, "PSYE022H", lib ) ;
			checkRCode( errblk ) ;
			return "" ;
		}
	}

	return ( ddlibs == "" ) ? it->second.top().paths : ddlibs ;
}


bool pApplication::has_libdef( const string& lib )
{
	//
	// Check if there is a LIBDEF active for this lib.
	// Used to avoid getting error PSYE022H on get_libdef_search_paths()
	//

	TRACE_FUNCTION() ;

	return ( zlibd.find( lib ) != zlibd.end() ) ;
}


string pApplication::get_ddname_search_paths( const string& ddn )
{
	//
	// Get search paths allocated to a DDNAME.
	//

	TRACE_FUNCTION() ;

	return p_gls->get_allocated_path( ddn ) ;
}


string pApplication::get_file_for_dataid( const string& dataid )
{
	//
	// Get file for a DATAID.
	//

	TRACE_FUNCTION() ;

	return p_lss->get_dsn_dataid( dataid ) ;
}


string pApplication::get_file_for_listid( const string& listid )
{
	//
	// Get file for a LISTID.
	//

	TRACE_FUNCTION() ;

	return p_lss->get_dsn_listid( listid ) ;
}


void pApplication::set_screenName()
{
	TRACE_FUNCTION() ;

	if ( p_poolMGR->get( errblk, zscrnum, "ZSCRNAM2" ) == "PERM" )
	{
		p_poolMGR->put( errblk, "ZSCRNAME", p_poolMGR->get( errblk, zscrnum, "ZSCRNAME" ), SHARED ) ;
	}
}


void pApplication::set_panelName( const string& p )
{
	TRACE_FUNCTION() ;

	p_poolMGR->put( errblk, zscrnum, "ZSCRNAM3", p ) ;
}


string pApplication::get_old_panelName()
{
	TRACE_FUNCTION() ;

	return p_poolMGR->get( errblk, zscrnum, "ZSCRNAM3" ) ;
}


string pApplication::get_zsel()
{
	TRACE_FUNCTION() ;

	string zsel = p_poolMGR->get( errblk, "ZSEL", SHARED ) ;
	p_poolMGR->put( errblk, "ZSEL", "", SHARED ) ;

	return zsel ;
}


const string pApplication::get_trail()
{
	TRACE_FUNCTION() ;

	return ( currPanel ) ? currPanel->get_trail() : "" ;
}


void pApplication::set_pcursor( uint row,
				uint col )
{
	TRACE_FUNCTION() ;

	if ( currPanel ) { currPanel->set_pcursor( row, col ) ; }
}


void pApplication::set_lcursor( uint row,
				uint col )
{
	TRACE_FUNCTION() ;

	if ( currPanel ) { currPanel->set_lcursor( row, col ) ; }
}


void pApplication::set_lcursor_home()
{
	TRACE_FUNCTION() ;

	if ( currPanel ) { currPanel->set_lcursor_home() ; }
}


set<string>& pApplication::vlist( poolType pType,
				  int lvl )
{
	TRACE_FUNCTION() ;

	return p_poolMGR->vlist( errblk, RC, pType, lvl ) ;
}


set<string>& pApplication::vilist( vdType defn )
{
	TRACE_FUNCTION() ;

	return funcPool->vlist( RC, INTEGER, defn ) ;
}


set<string>& pApplication::vslist( vdType defn )
{
	TRACE_FUNCTION() ;

	return funcPool->vlist( RC, STRING, defn ) ;
}


void pApplication::movepop( int row,
			    int col )
{
	TRACE_FUNCTION() ;

	if ( addpop_active )
	{
		row = row - 1 ;
		col = col - 3 ;
		addpop_row = (row <  0 ) ? 1 : row + 2 ;
		addpop_col = (col < -1 ) ? 2 : col + 4 ;
		currPanel->set_popup( errblk, true, addpop_row, addpop_col ) ;
		currPanel->move_popup() ;
	}
}


void pApplication::ft_cleanup()
{
	TRACE_FUNCTION() ;

	delete FTailor ;

	FTailor = nullptr ;
}


bool pApplication::tableNameOK( const string& tb_name,
				const string& err )
{
	TRACE_FUNCTION() ;

	RC = 0 ;
	errblk.setRC( 0 ) ;

	if ( tb_name == "" )
	{
		errblk.setcall( TRACE_INFO(), err, "PSYE013H" ) ;
	}
	else if ( !isvalidName( tb_name ) )
	{
		errblk.setcall( TRACE_INFO(), err, "PSYE014Q", tb_name ) ;
	}
	CHECK_ERROR_RETURN_FALSE()

	return true ;
}


int pApplication::edrec_init( const string& m_parm,
			      const string& qname,
			      const string& rname,
			      const string& uprof,
			      const string& tabName,
			      const string& vlist )
{
	//
	// Initialise an edit recovery table.
	//
	// RC =  0  Edit recovery table created.
	// RC =  4  Edit recovery table already exists for this application.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	int i ;
	int xRC ;

	string zedstat  ;
	string zedtfile ;
	string zedbfile ;
	string zeduser  ;
	string zedmode  ;
	string zedopts  ;
	string zedrecfm ;
	string zedlrecl ;

	const string e1 = "EDREC INIT Service Error" ;

	qscan( qname, rname, EXC, LOCAL ) ;
	if ( RC == 0 )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE023O", "INIT" ) ;
		checkRCode( errblk ) ;
		return RC ;
	}

	vdefine( vlist, &zedstat, &zedtfile, &zedbfile, &zedmode, &zedopts, &zeduser, &zedrecfm, & zedlrecl ) ;
	if ( RC > 0 ) { return 20 ; }

	tbopen( tabName, WRITE, uprof ) ;
	if ( RC == 0 )
	{
		tbend( tabName ) ;
		xRC = 4 ;
	}
	else if ( RC == 8 )
	{
		tbcreate( tabName,
			  "",
			  "(ZEDSTAT,ZEDTFILE,ZEDBFILE,ZEDMODE,ZEDRECFM,ZEDLRECL,ZEDOPTS,ZEDUSER)",
			  WRITE,
			  NOREPLACE,
			  uprof ) ;
		if ( RC > 0 )
		{
			vdelete( vlist ) ;
			return 20 ;
		}
		tbvclear( tabName ) ;
		zedstat = "0" ;
		for ( i = 0 ; i < EDREC_SZ ; ++i )
		{
			tbadd( tabName ) ;
			if ( RC > 0 )
			{
				tbend( tabName ) ;
				vdelete( vlist ) ;
				return 20 ;
			}
		}
		tbclose( tabName, "", uprof ) ;
		if ( RC > 0 )
		{
			vdelete( vlist ) ;
			return 20 ;
		}
		xRC = 0 ;
	}
	else
	{
		xRC = 20 ;
	}

	vdelete( vlist ) ;
	return xRC ;
}


int pApplication::edrec_query( const string& m_parm,
			       const string& qname,
			       const string& rname,
			       const string& uprof,
			       const string& tabName,
			       const string& vlist )
{
	//
	// Query recovery pending.
	//
	// RC =  0  Recovery not pending.
	// RC =  4  Entry found in the Edit Recovery Table, recovery pending.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	int row ;

	string zedstat  ;
	string zedtfile ;
	string zedbfile ;
	string zeduser  ;
	string zedmode  ;
	string zedopts  ;
	string zedrecfm ;
	string zedlrecl ;

	const string e1 = "EDREC QUERY Service Error" ;

	qscan( qname, rname, EXC, LOCAL ) ;
	if ( RC == 0 )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE023O", "QUERY" ) ;
		checkRCode( errblk ) ;
		return RC ;
	}

	row = funcPool->get1( errblk, 8, INTEGER, "ZEDROW" ) ;

	vdefine( vlist, &zedstat, &zedtfile, &zedbfile, &zedmode, &zedopts, &zeduser, &zedrecfm, &zedlrecl ) ;
	if ( RC > 0 ) { return 20 ; }

	tbopen( tabName, WRITE, uprof ) ;
	if ( RC == 8 )
	{
		vdelete( vlist ) ;
		return 0 ;
	}

	tbskip( tabName, row ) ;

	while ( true )
	{
		tbskip( tabName, 1 ) ;
		if ( RC > 0 ) { break ; }
		++row ;
		if ( zedstat == "0" ) { continue ; }
		enq( "SPFEDIT", zedbfile ) ;
		if ( RC == 8 ) { continue ; }
		tbend( tabName ) ;
		vdelete( vlist ) ;
		funcPool->put2( errblk, "ZEDTFILE", zedtfile ) ;
		funcPool->put2( errblk, "ZEDBFILE", zedbfile ) ;
		funcPool->put2( errblk, "ZEDOPTS",  zedopts ) ;
		funcPool->put2( errblk, "ZEDMODE",  zedmode ) ;
		funcPool->put2( errblk, "ZEDRECFM", zedrecfm ) ;
		funcPool->put2( errblk, "ZEDLRECL", zedlrecl ) ;
		funcPool->put2( errblk, "ZEDUSER",  zeduser ) ;
		funcPool->put2( errblk, "ZEDROW",   row ) ;
		p_poolMGR->put( errblk, "ZEDUSER", zeduser, SHARED ) ;
		enq( qname, rname, EXC, LOCAL ) ;
		return 4 ;
	}

	tbend( tabName ) ;
	vdelete( vlist ) ;
	funcPool->put2( errblk, "ZEDROW", 0 ) ;

	return 0 ;
}


int pApplication::edrec_process( const string& m_parm,
				 const string& qname,
				 const string& rname,
				 const string& uprof,
				 const string& tabName,
				 const string& vlist )
{
	//
	// Process edit recovery.
	//
	// RC =  0  PROCESS - Recovery completed and data saved.
	// RC =  4  PROCESS - Recovery completed but user did not save data.
	// RC = 20  Severe error.
	//
	// ZEDOPTS  - byte 0 - confirm cancel.
	//          - byte 1 - preserve trailing spaces.
	//

	TRACE_FUNCTION() ;

	int xRC ;
	int row ;

	string zedstat  ;
	string zedtfile ;
	string zedbfile ;
	string zeduser  ;
	string zedmode  ;
	string zedrecfm ;
	string zedlrecl ;
	string zedopts  ;

	edit_parms e ;

	const string e1 = "EDREC PROCESS Service Error" ;

	qscan( qname, rname, EXC, LOCAL ) ;
	if ( RC == 8 )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE023N", "PROCESS" ) ;
		checkRCode( errblk ) ;
		return RC ;
	}

	zedtfile = funcPool->get2( errblk, 0, "ZEDTFILE" ) ;
	zedbfile = funcPool->get2( errblk, 0, "ZEDBFILE" ) ;
	zedopts  = funcPool->get2( errblk, 0, "ZEDOPTS"  ) ;
	zedmode  = funcPool->get2( errblk, 0, "ZEDMODE"  ) ;
	zedrecfm = funcPool->get2( errblk, 0, "ZEDRECFM" ) ;
	zedlrecl = funcPool->get2( errblk, 0, "ZEDLRECL" ) ;
	row      = funcPool->get1( errblk, 0, INTEGER, "ZEDROW" ) ;

	try
	{
		if ( !exists( zedbfile ) )
		{
			throw runtime_error( "" ) ;
		}
	}
	catch (...)
	{
		deq( qname, zedtfile ) ;
		errblk.setcall( TRACE_INFO(), e1, "PSYE023P", zedbfile ) ;
		checkRCode( errblk ) ;
		return RC ;
	}

	deq( qname, rname, LOCAL ) ;

	e.edit_recovery = true ;
	e.edit_viewmode = ( zedmode == "V" ) ;
	e.edit_file     = zedtfile ;
	e.edit_bfile    = zedbfile ;
	e.edit_reclen   = ( zedlrecl == "" ) ? 0 : ds2d( zedlrecl ) ;
	e.edit_confirm  = ( zedopts.size() > 0 && zedopts[ 0 ] == '1' ) ? "YES" : "NO" ;
	e.edit_preserve = ( zedopts.size() > 1 && zedopts[ 1 ] == '1' ) ? "PRESERVE" : "" ;

	edit_rec( e ) ;
	xRC = ( RC < 5 ) ? RC : 20 ;

	deq( qname, zedtfile ) ;
	deq( "SPFEDIT", zedbfile ) ;

	remove( zedbfile ) ;

	vdefine( vlist, &zedstat, &zedtfile, &zedbfile, &zedmode, &zedopts, &zeduser, &zedrecfm, &zedlrecl ) ;
	if ( RC > 0 ) { return 20 ; }

	tbopen( tabName, WRITE, uprof ) ;
	if ( RC > 0 )
	{
		vdelete( vlist ) ;
		return 20 ;
	}

	tbskip( tabName, row ) ;
	if ( RC > 0 )
	{
		tbend( tabName ) ;
		vdelete( vlist ) ;
		return 20 ;
	}

	tbvclear( tabName ) ;
	zedstat = "0" ;
	tbput( tabName ) ;
	if ( RC > 0 )
	{
		tbend( tabName ) ;
		vdelete( vlist ) ;
		return 20 ;
	}

	tbclose( tabName, "", uprof ) ;
	if ( RC > 0 )
	{
		vdelete( vlist ) ;
		return 20 ;
	}

	vdelete( vlist ) ;

	return xRC ;
}


int pApplication::edrec_cancel( const string& m_parm,
				const string& qname,
				const string& rname,
				const string& uprof,
				const string& tabName,
				const string& vlist )
{
	//
	// Cancel edit recovery.
	//
	// RC =  0  Normal completion.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	string zedstat  ;
	string zedtfile ;
	string zedbfile ;
	string zeduser  ;
	string zedrecfm ;
	string zedlrecl ;
	string zedmode  ;
	string zedopts  ;

	const string e1 = "EDREC CANCEL Service Error" ;

	deq( qname, rname, LOCAL ) ;
	if ( RC == 8 )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE023N", "CANCEL" ) ;
		checkRCode( errblk ) ;
		return RC ;
	}

	int row = funcPool->get1( errblk, 8, INTEGER, "ZEDROW" ) ;

	zedtfile = funcPool->get2( errblk, 8, "ZEDTFILE" ) ;
	zedbfile = funcPool->get2( errblk, 8, "ZEDBFILE" ) ;

	deq( qname, zedtfile ) ;
	deq( "SPFEDIT", zedbfile ) ;

	remove( zedbfile ) ;

	vdefine( vlist, &zedstat, &zedtfile, &zedbfile, &zedmode, &zedopts, &zeduser, &zedrecfm, &zedlrecl ) ;
	if ( RC > 0 ) { return 20 ; }

	tbopen( tabName, WRITE, uprof ) ;
	if ( RC > 0 )
	{
		vdelete( vlist ) ;
		return 20 ;
	}

	tbskip( tabName, row ) ;
	if ( RC > 0 )
	{
		tbend( tabName ) ;
		vdelete( vlist ) ;
		return 20 ;
	}

	tbvclear( tabName ) ;
	zedstat = "0" ;
	tbput( tabName ) ;
	if ( RC > 0 )
	{
		tbend( tabName ) ;
		vdelete( vlist ) ;
		return 20 ;
	}

	tbclose( tabName, "", uprof ) ;
	if ( RC > 0 )
	{
		vdelete( vlist ) ;
		return 20 ;
	}

	vdelete( vlist ) ;
	if ( RC > 0 )
	{
		return 20 ;
	}

	funcPool->put2( errblk, "ZEDTFILE", "" ) ;
	funcPool->put2( errblk, "ZEDBFILE", "" ) ;

	return 0 ;
}


int pApplication::edrec_defer( const string& qname,
			       const string& rname )
{
	//
	// Defer edit recovery.
	//
	// RC =  0  Normal completion.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	string zedtfile ;
	string zedbfile ;

	const string e1 = "EDREC DEFER Service Error" ;

	deq( qname, rname, LOCAL ) ;
	if ( RC == 8 )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE023N", "DEFER" ) ;
		checkRCode( errblk ) ;
		return RC ;
	}

	zedtfile = funcPool->get2( errblk, 8, "ZEDTFILE" ) ;
	zedbfile = funcPool->get2( errblk, 8, "ZEDBFILE" ) ;

	deq( qname, zedtfile ) ;
	deq( "SPFEDIT", zedbfile ) ;

	return 0 ;
}


void pApplication::edit_rec( const edit_parms& e )
{
	//
	// Used internally to call edit or view during edit recovery.
	//
	// RC =  0  Normal completion.  Data was saved.
	// RC =  4  Normal completion.  Data was not saved.
	//          No changes made or CANCEL entered.
	// RC = 14  File in use.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	selct.clear() ;
	selct.pgm     = p_poolMGR->get( errblk, "ZEDITPGM", PROFILE ) ;
	selct.newappl = ""    ;
	selct.newpool = false ;
	selct.passlib = false ;
	selct.suspend = true  ;
	selct.options = (void*)&e ;
	selct.ptid    = ptid  ;
	selct.stid    = taskId ;
	selct.service = true  ;
	selct.errret  = controlErrorsReturn ;
	selct.backgrd = backgrd ;
	actionSelect() ;

	RC = ZRC ;
	if ( ZRC > 11 && isvalidName( ZRESULT ) )
	{
		errblk.setcall( TRACE_INFO(), "EDIT Error", ZRESULT, get_shared_var( "ZVAL1" ), ZRC ) ;
		checkRCode( errblk ) ;
	}
}


void pApplication::select( const selobj& sel )
{
	//
	// SELECT a function or panel using a SELECT object (internal use only).
	//

	TRACE_FUNCTION() ;

	const string e1 = "Error in SELECT command" ;

	if ( backgrd && sel.pgmtype == PGM_PANEL )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE039T" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	selct = sel ;
	selct.backgrd = backgrd ;
	selct.sync    = true ;
	selct.ptid    = ptid ;
	selct.stid    = taskId ;
	actionSelect() ;
}


void pApplication::submit( const selobj& sel )
{
	//
	// Submit for background processing a function using a SELECT object (internal use only).
	//

	TRACE_FUNCTION() ;

	const string e1 = "Error in SUBMIT command" ;

	if ( sel.pgmtype == PGM_PANEL )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYE039T" ) ;
		checkRCode( errblk ) ;
		return ;
	}

	selct = sel ;
	selct.backgrd = true  ;
	selct.sync    = false ;
	actionSelect() ;
}


void pApplication::actionSelect()
{
	//
	// RC =  0  Normal completion of the selection panel or function.  END was entered.
	// RC =  4  Normal completion.  RETURN was entered or EXIT specified on the selection panel.
	// RC = 20  Abnormal termination of the called task.
	//
	// If the application has abended, propagate back (only set if not controlErrorsReturn).
	//
	// If abnormal termination in the selected task:
	// ZRC = 20  ZRSN = 999  Application program abended.
	// ZRC = 20  ZRSN = 998  SELECT PGM not found.
	// ZRC = 20  ZRSN = 997  SELECT CMD not found.
	// ZRC = 20  ZRSN = 996  Errors loading program.
	// Don't percolate these codes back to the calling program (set ZRSN = 0) so it doesn't
	// appear to be abending with these codes (20/999/Abended instead).
	//
	// BUG: selct.pgm will be blank for PANEL/CMD/SHELL in error messages (resolved in lspf.cpp).
	//

	TRACE_FUNCTION() ;

	int exitRC ;

	if ( !busyAppl )
	{
		llog( "E", "Invalid method state" <<endl ) ;
		llog( "E", "Application is in a wait state.  Method cannot invoke SELECT services." <<endl ) ;
		RC = 20 ;
		return ;
	}

	RC  = 0 ;
	SEL = true ;

	wait_event( WAIT_SELECT ) ;

	if ( RC == 4 )
	{
		propagateEnd = true ;
		if ( !selct.selPanl )
		{
			RC = 0 ;
		}
	}

	SEL = false ;

	if ( abnormalEnd )
	{
		abnormalNoMsg = true ;
		if ( ZRC == 20 && ZRESULT == "PSYS013J" )
		{
			switch ( ZRSN )
			{
			case 996:
				errblk.setcall( TRACE_INFO(), "Error in SELECT command", "PSYS013H", selct.pgm ) ;
				break ;

			case 997:
				errblk.setcall( TRACE_INFO(), "Error in SELECT command", "PSYS012X", word( selct.parm, 1 ) ) ;
				break ;

			case 998:
				errblk.setcall( TRACE_INFO(), "Error in SELECT command", "PSYS012W", selct.pgm ) ;
				break ;

			}
			ZRSN = 0 ;
			checkRCode( errblk ) ;
		}
		llog( "E", "Percolating abend to calling application.  Taskid: "<< taskId <<endl ) ;
		abend() ;
	}

	exitRC = RC ;
	if ( ZRC == 20 && ZRESULT == "PSYS013J" )
	{
		switch ( ZRSN )
		{
		case 996:
			vreplace( "ZVAL1", selct.pgm ) ;
			ZRESULT = "PSYS013H" ;
			break ;

		case 997:
			vreplace( "ZVAL1", word( selct.parm, 1 ) ) ;
			ZRESULT = "PSYS012X" ;
			break ;

		case 998:
			vreplace( "ZVAL1", selct.pgm ) ;
			ZRESULT = "PSYS012W" ;
			break ;

		}
	}

	if ( !abnormalEnd && ZRC >= 12 && ZRSN == 999 && ZRESULT == "*0123456789NOABEND" )
	{
		vget( "ZERRMSG ZERRSM ZERRLM", SHARED ) ;
		vcopy( "ZERRMSG", ZRESULT ) ;
		exitRC  = ZRC ;
		ZRESULT = "" ;
	}

	selct.clear() ;
	RC = exitRC ;
}


void pApplication::reload_keylist()
{
	TRACE_FUNCTION() ;

	pPanel* p ;

	string tklpriv = p_poolMGR->get( errblk, "ZKLPRIV", PROFILE ) ;

	if ( !panelList.empty() && ( pfllToken != pflgToken || zklpriv != tklpriv ) )
	{
		zklpriv = tklpriv ;
		for ( auto it = panelList.begin() ; it != panelList.end() ; ++it )
		{
			p = it->second ;
			load_keylist( p ) ;
			p->build_pfkeys( errblk ) ;
		}
		currPanel->update_keylist_vars( errblk ) ;
		currPanel->display_pfkeys( errblk ) ;
	}
}


void pApplication::load_keylist( pPanel* p )
{
	//
	// If ZKLPRIV = N or p->keyshr, search only shared.
	// If ZKLPRIV = Y, search private then shared.
	//
	// If keylist not found for the application id and keyappl not specified on the )PANEL statement,
	// search ISP (if not already searched).
	//
	// PRIVATE keys are read from ZUPROF.
	// SHARED keys are read from ZTLIB.
	//

	TRACE_FUNCTION() ;

	int ws1 ;
	int ws2 ;

	string suffix ;
	string suffixes ;

	string tabName ;
	string appName ;
	string appNames ;

	bool tab_open  = false ;
	bool key_found = false ;

	pfllToken = pflgToken ;

	if ( p->keylistn == "" )
	{
		return ;
	}

	appNames = ( p->keyappl == "" ) ? get_applid() : p->keyappl ;
	suffixes = ( zklpriv == "N" || p->keyshr ) ? "KEYS" : "KEYP KEYS" ;

	if ( p->keyappl == "" && appNames != "ISP" )
	{
		appNames += " ISP" ;
	}

	ws1 = words( appNames ) ;
	ws2 = words( suffixes ) ;

	for ( int i = 1 ; i <= ws1 && !key_found ; ++i )
	{
		appName = word( appNames, i ) ;
		for ( int j = 1 ; j <= ws2 ; ++j )
		{
			suffix  = word( suffixes, j ) ;
			tabName = appName + suffix ;
			tbopen( tabName,
				NOWRITE,
				( suffix == "KEYP" ) ? "ZUPROF" : "",
				SHARE ) ;
			if ( RC == 0 )
			{
				tab_open = true ;
				tbvclear( tabName ) ;
				vreplace( "KEYLISTN", p->keylistn ) ;
				tbget( tabName ) ;
				if ( RC == 0 )
				{
					p->keytype = suffix.substr( 3, 1 ) ;
					p_poolMGR->put( errblk, "ZKLTYPE", p->keytype, SHARED, SYSTEM ) ;
					if ( p->keyappl == "" )
					{
						p->keyappl = appName ;
						p_poolMGR->put( errblk, "ZKLAPPL", p->keyappl, SHARED, SYSTEM ) ;
					}
					key_found = true ;
					break ;
				}
				else
				{
					tbend( tabName ) ;
				}
			}
		}
	}

	if ( !tab_open )
	{
		errblk.setcall( TRACE_INFO(), "KEYLIST Error", "PSYE023E", tabName ) ;
		checkRCode( errblk ) ;
		return ;
	}

	if ( !key_found )
	{
		errblk.setcall( TRACE_INFO(), "KEYLIST Error", "PSYE023F", p->keylistn, tabName ) ;
		checkRCode( errblk ) ;
		return  ;
	}

	p->put_keylist( KEY_F(1),  funcPool->vlocate( errblk, "KEY1DEF"  ) ) ;
	p->put_keylist( KEY_F(2),  funcPool->vlocate( errblk, "KEY2DEF"  ) ) ;
	p->put_keylist( KEY_F(3),  funcPool->vlocate( errblk, "KEY3DEF"  ) ) ;
	p->put_keylist( KEY_F(4),  funcPool->vlocate( errblk, "KEY4DEF"  ) ) ;
	p->put_keylist( KEY_F(5),  funcPool->vlocate( errblk, "KEY5DEF"  ) ) ;
	p->put_keylist( KEY_F(6),  funcPool->vlocate( errblk, "KEY6DEF"  ) ) ;
	p->put_keylist( KEY_F(7),  funcPool->vlocate( errblk, "KEY7DEF"  ) ) ;
	p->put_keylist( KEY_F(8),  funcPool->vlocate( errblk, "KEY8DEF"  ) ) ;
	p->put_keylist( KEY_F(9),  funcPool->vlocate( errblk, "KEY9DEF"  ) ) ;
	p->put_keylist( KEY_F(10), funcPool->vlocate( errblk, "KEY10DEF" ) ) ;
	p->put_keylist( KEY_F(11), funcPool->vlocate( errblk, "KEY11DEF" ) ) ;
	p->put_keylist( KEY_F(12), funcPool->vlocate( errblk, "KEY12DEF" ) ) ;
	p->put_keylist( KEY_F(13), funcPool->vlocate( errblk, "KEY13DEF" ) ) ;
	p->put_keylist( KEY_F(14), funcPool->vlocate( errblk, "KEY14DEF" ) ) ;
	p->put_keylist( KEY_F(15), funcPool->vlocate( errblk, "KEY15DEF" ) ) ;
	p->put_keylist( KEY_F(16), funcPool->vlocate( errblk, "KEY16DEF" ) ) ;
	p->put_keylist( KEY_F(17), funcPool->vlocate( errblk, "KEY17DEF" ) ) ;
	p->put_keylist( KEY_F(18), funcPool->vlocate( errblk, "KEY18DEF" ) ) ;
	p->put_keylist( KEY_F(19), funcPool->vlocate( errblk, "KEY19DEF" ) ) ;
	p->put_keylist( KEY_F(20), funcPool->vlocate( errblk, "KEY20DEF" ) ) ;
	p->put_keylist( KEY_F(21), funcPool->vlocate( errblk, "KEY21DEF" ) ) ;
	p->put_keylist( KEY_F(22), funcPool->vlocate( errblk, "KEY22DEF" ) ) ;
	p->put_keylist( KEY_F(23), funcPool->vlocate( errblk, "KEY23DEF" ) ) ;
	p->put_keylist( KEY_F(24), funcPool->vlocate( errblk, "KEY24DEF" ) ) ;

	p->put_keyattr( KEY_F(1),  funcPool->vlocate( errblk, "KEY1ATR"  ) ) ;
	p->put_keyattr( KEY_F(2),  funcPool->vlocate( errblk, "KEY2ATR"  ) ) ;
	p->put_keyattr( KEY_F(3),  funcPool->vlocate( errblk, "KEY3ATR"  ) ) ;
	p->put_keyattr( KEY_F(4),  funcPool->vlocate( errblk, "KEY4ATR"  ) ) ;
	p->put_keyattr( KEY_F(5),  funcPool->vlocate( errblk, "KEY5ATR"  ) ) ;
	p->put_keyattr( KEY_F(6),  funcPool->vlocate( errblk, "KEY6ATR"  ) ) ;
	p->put_keyattr( KEY_F(7),  funcPool->vlocate( errblk, "KEY7ATR"  ) ) ;
	p->put_keyattr( KEY_F(8),  funcPool->vlocate( errblk, "KEY8ATR"  ) ) ;
	p->put_keyattr( KEY_F(9),  funcPool->vlocate( errblk, "KEY9ATR"  ) ) ;
	p->put_keyattr( KEY_F(10), funcPool->vlocate( errblk, "KEY10ATR" ) ) ;
	p->put_keyattr( KEY_F(11), funcPool->vlocate( errblk, "KEY11ATR" ) ) ;
	p->put_keyattr( KEY_F(12), funcPool->vlocate( errblk, "KEY12ATR" ) ) ;
	p->put_keyattr( KEY_F(13), funcPool->vlocate( errblk, "KEY13ATR" ) ) ;
	p->put_keyattr( KEY_F(14), funcPool->vlocate( errblk, "KEY14ATR" ) ) ;
	p->put_keyattr( KEY_F(15), funcPool->vlocate( errblk, "KEY15ATR" ) ) ;
	p->put_keyattr( KEY_F(16), funcPool->vlocate( errblk, "KEY16ATR" ) ) ;
	p->put_keyattr( KEY_F(17), funcPool->vlocate( errblk, "KEY17ATR" ) ) ;
	p->put_keyattr( KEY_F(18), funcPool->vlocate( errblk, "KEY18ATR" ) ) ;
	p->put_keyattr( KEY_F(19), funcPool->vlocate( errblk, "KEY19ATR" ) ) ;
	p->put_keyattr( KEY_F(20), funcPool->vlocate( errblk, "KEY20ATR" ) ) ;
	p->put_keyattr( KEY_F(21), funcPool->vlocate( errblk, "KEY21ATR" ) ) ;
	p->put_keyattr( KEY_F(22), funcPool->vlocate( errblk, "KEY22ATR" ) ) ;
	p->put_keyattr( KEY_F(23), funcPool->vlocate( errblk, "KEY23ATR" ) ) ;
	p->put_keyattr( KEY_F(24), funcPool->vlocate( errblk, "KEY24ATR" ) ) ;

	p->put_keylabl( KEY_F(1),  funcPool->vlocate( errblk, "KEY1LAB"  ) ) ;
	p->put_keylabl( KEY_F(2),  funcPool->vlocate( errblk, "KEY2LAB"  ) ) ;
	p->put_keylabl( KEY_F(3),  funcPool->vlocate( errblk, "KEY3LAB"  ) ) ;
	p->put_keylabl( KEY_F(4),  funcPool->vlocate( errblk, "KEY4LAB"  ) ) ;
	p->put_keylabl( KEY_F(5),  funcPool->vlocate( errblk, "KEY5LAB"  ) ) ;
	p->put_keylabl( KEY_F(6),  funcPool->vlocate( errblk, "KEY6LAB"  ) ) ;
	p->put_keylabl( KEY_F(7),  funcPool->vlocate( errblk, "KEY7LAB"  ) ) ;
	p->put_keylabl( KEY_F(8),  funcPool->vlocate( errblk, "KEY8LAB"  ) ) ;
	p->put_keylabl( KEY_F(9),  funcPool->vlocate( errblk, "KEY9LAB"  ) ) ;
	p->put_keylabl( KEY_F(10), funcPool->vlocate( errblk, "KEY10LAB" ) ) ;
	p->put_keylabl( KEY_F(11), funcPool->vlocate( errblk, "KEY11LAB" ) ) ;
	p->put_keylabl( KEY_F(12), funcPool->vlocate( errblk, "KEY12LAB" ) ) ;
	p->put_keylabl( KEY_F(13), funcPool->vlocate( errblk, "KEY13LAB" ) ) ;
	p->put_keylabl( KEY_F(14), funcPool->vlocate( errblk, "KEY14LAB" ) ) ;
	p->put_keylabl( KEY_F(15), funcPool->vlocate( errblk, "KEY15LAB" ) ) ;
	p->put_keylabl( KEY_F(16), funcPool->vlocate( errblk, "KEY16LAB" ) ) ;
	p->put_keylabl( KEY_F(17), funcPool->vlocate( errblk, "KEY17LAB" ) ) ;
	p->put_keylabl( KEY_F(18), funcPool->vlocate( errblk, "KEY18LAB" ) ) ;
	p->put_keylabl( KEY_F(19), funcPool->vlocate( errblk, "KEY19LAB" ) ) ;
	p->put_keylabl( KEY_F(20), funcPool->vlocate( errblk, "KEY20LAB" ) ) ;
	p->put_keylabl( KEY_F(21), funcPool->vlocate( errblk, "KEY21LAB" ) ) ;
	p->put_keylabl( KEY_F(22), funcPool->vlocate( errblk, "KEY22LAB" ) ) ;
	p->put_keylabl( KEY_F(23), funcPool->vlocate( errblk, "KEY23LAB" ) ) ;
	p->put_keylabl( KEY_F(24), funcPool->vlocate( errblk, "KEY24LAB" ) ) ;

	p->keyhelpn = funcPool->get1( errblk, 0, "KEYHELPN" ) ;

	tbend( tabName ) ;
}


bool pApplication::notify_pending()
{
	TRACE_FUNCTION() ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	return ( notifies.size() > 0 && p_poolMGR->get( errblk, "ZNOTIFY", PROFILE ) == "Y" ) ;
}


bool pApplication::notify()
{
	TRACE_FUNCTION() ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	if ( notifies.size() == 0 || p_poolMGR->get( errblk, "ZNOTIFY", PROFILE ) != "Y" )
	{
		return false ;
	}

	auto it = notifies.begin() ;

	outBuffer      = *it  ;
	lineInOutDone  = true ;
	notifies.erase( it )  ;
	return true ;
}


string pApplication::get_help()
{
	TRACE_FUNCTION() ;

	string t = currPanel->get_help() ;

	return ( t == currPanel->panelid ) ? "*" : t ;
}


string pApplication::get_exhelp()
{
	//
	// Return the extended help panel.
	// If the tutorial is running and there is no .HELP, return the .HELP value from the previous application
	// stored in dHelp.
	//

	TRACE_FUNCTION() ;

	return ( !currPanel ) ? "" :
	       ( tutorial && dHelp != "" && currPanel->dHelp == "" ) ?
				     ( dHelp == currPanel->panelid ) ? "*" + dHelp : dHelp
										   : currPanel->get_exhelp( errblk ) ;
}


void pApplication::set_exhelp( const string& s )
{
	//
	// Save the previous help panel's .HELP value if the tutorial is running.
	// This is so EXHELP from HELP will show .HELP from the original panel displayed.
	//

	TRACE_FUNCTION() ;

	if ( tutorial )
	{
		dHelp = ( s.size() > 0 && s.front() == '*' ) ? s.substr( 1 ) : s ;
	}
}


string pApplication::get_keyshelp()
{
	//
	// Return keys help for the current keylist, or ZKEYHELP if not in use.
	// Return '*' if the keys help panel is already being displayed.
	//

	TRACE_FUNCTION() ;

	string t = currPanel->keyhelpn ;

	if ( t == currPanel->panelid )
	{
		return "*" ;
	}

	if ( t == "" || p_poolMGR->get( errblk, "ZKLUSE", PROFILE ) == "N" )
	{
		vcopy( "ZKEYHELP", t, MOVE ) ;
	}

	return t ;
}


void pApplication::get_message( const string& p_msg,
				string& smsg,
				string& lmsg )
{
	//
	// Load messages from message library and copy short and long messages to smsg and lmsg.
	//
	// Substitute any dialogue variables in the short and long messages.
	//
	// RC =  0  Normal completion.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	RC = 0 ;

	if ( !load_message( p_msg ) ) { return ; }

	smsg = sub_vars( msgList[ p_msg ].smsg ) ;
	lmsg = sub_vars( msgList[ p_msg ].lmsg ) ;

	if ( smsg.size() > 34  ) { smsg.erase( 34  ) ; }
	if ( lmsg.size() > 512 ) { lmsg.erase( 512 ) ; }
}


void pApplication::get_message( const string& p_msg )
{
	//
	// Load messages from message library and copy slmsg object to zmsg for the message requested.
	//
	// Substitute any dialogue variables in the short and long messages.
	// Substitute any dialogue varibles in .TYPE, .WINDOW, .HELP and .ALARM parameters.
	// Set zmsgid.
	//
	// RC =  0  Normal completion.
	// RC = 12  Message not found.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	RC = 0 ;

	if ( !load_message( p_msg ) ) { return ; }

	zmsg = msgList[ p_msg ] ;

	zmsg.smsg = sub_vars( zmsg.smsg ) ;
	zmsg.lmsg = sub_vars( zmsg.lmsg ) ;

	if ( zmsg.smsg.size() > 34  ) { zmsg.smsg.erase( 34  ) ; }
	if ( zmsg.lmsg.size() > 512 ) { zmsg.lmsg.erase( 512 ) ; }

	if ( !sub_message_vars( zmsg ) ) { RC = 20 ; return ; }

	zmsgid = p_msg ;
}


bool pApplication::load_message( const string& p_msg )
{
	//
	// Message format: 1-5 alpha char prefix.
	//                 3 numeric chars.
	//                 1 alpha char suffix (optional and only if prefix is less than 5).
	//
	// Read messages and store in msgList map (no variable substitution done at this point).
	// Return false if message not found in member or there is an error (but still store individual messages from member).
	// Error on duplicate message-id in the file member.
	//
	// The message file name is determined by truncating the message ID after the second digit of the number.
	// AB123A file AB12.
	// G012 file G01.
	// Message file name can be upper (first search) or lower case.
	//
	// If in test mode, reload message each time it is requested.
	//
	// Routine sets the return code:
	// RC =  0  Normal completion.
	// RC = 12  Message not found.
	// RC = 20  Severe error.
	//

	TRACE_FUNCTION() ;

	int i ;
	int j ;

	char c ;

	string p_msgfn1 ;
	string p_msgfn2 ;
	string filename ;
	string mline ;
	string paths ;
	string tmp   ;
	string msgid ;
	string smsg  ;
	string lmsg  ;

	bool lcontinue = false ;
	bool found     = false ;

	slmsg t ;

	RC = 0 ;

	if ( debugMode )
	{
		msgList.clear() ;
	}
	else if ( msgList.count( p_msg ) > 0 )
	{
		return true ;
	}

	i = check_message_id( p_msg ) ;

	if ( i == 0 || ( p_msg.size() - i > 3 && !isalpha( p_msg.back() ) ) )
	{
		RC = 20 ;
		zerr1 = "Error in message "+ p_msg ;
		zerr2 = "Message-id format invalid (1)." ;
		checkRCode() ;
		return false ;
	}

	p_msgfn1 = p_msg.substr( 0, i + 2 ) ;
	p_msgfn2 = p_msgfn1 ;

	paths = get_libdef_search_paths( s_ZMLIB ) ;

	while ( true )
	{
		for ( i = getpaths( paths ), j = 1 ; j <= i ; ++j )
		{
			filename = getpath( paths, j ) + p_msgfn2 ;
			try
			{
				if ( exists( filename ) )
				{
					if ( !is_regular_file( filename ) )
					{
						RC = 20 ;
						zerr1 = "Error in message "+ p_msg ;
						zerr2 = "Message file "+ filename +" is not a regular file" ;
						checkRCode() ;
						return false ;
					}
					found = true ;
					break ;
				}
			}
			catch ( boost::filesystem::filesystem_error &e )
			{
				RC = 20 ;
				zerr1 = "Error accessing entry" ;
				zerr2 = filename + " " + e.what() ;
				checkRCode() ;
				return false ;
			}
		}
		if ( found || ( p_msgfn2 == lower( p_msgfn2 ) ) ) { break ; }
		ilower( p_msgfn2 ) ;
	}

	if ( !found )
	{
		RC    = 12 ;
		zerr1 = "Error loading message "+ p_msg ;
		zerr2 = "Message file "+ p_msgfn1 +" not found in ZMLIB." ;
		checkRCode() ;
		return false ;
	}

	msgid = "" ;
	smsg  = "" ;
	lmsg  = "" ;
	tmp   = "" ;

	std::ifstream messages( filename.c_str() ) ;
	if ( !messages.is_open() )
	{
		RC = 20 ;
		zerr1 = "Error opening message file "+ filename ;
		zerr2 = strerror( errno ) ;
		checkRCode() ;
		return false ;
	}

	while ( getline( messages, mline ) )
	{
		trim( mline ) ;
		if ( mline == "" || mline.front() == '*' ) { continue ; }
		if ( mline.compare( 0, 2, "/*" ) == 0 )    { continue ; }
		if ( mline.compare( 0, p_msgfn1.size(), p_msgfn1 ) == 0 )
		{
			if ( msgid != "" )
			{
				if ( lcontinue )
				{
					RC = 20 ;
					messages.close() ;
					zerr1 = "Error in message "+ msgid ;
					zerr2 = "Invalid continuation found." ;
					checkRCode() ;
					return false ;
				}
				if ( t.parse( smsg, lmsg ) > 0 )
				{
					RC = 20 ;
					messages.close() ;
					zerr1 = "Error in message "+ msgid ;
					zerr2 = "Error (1) in message-id." ;
					checkRCode() ;
					return false ;
				}
				if ( msgList.count( msgid ) > 0 )
				{
					RC = 20 ;
					messages.close() ;
					zerr1 = "Error in message "+ msgid ;
					zerr2 = "Duplicate message-id found." ;
					checkRCode() ;
					return false ;
				}
				msgList[ msgid ] = t ;
			}
			msgid = word( mline, 1 )    ;
			smsg  = subword( mline, 2 ) ;
			lmsg  = "" ;
			tmp   = "" ;
			i = check_message_id( msgid ) ;
			if ( i == 0 || ( msgid.size() - i > 3 && !isalpha( msgid.back() ) ) )
			{
				RC = 20 ;
				zerr1 = "Error in message "+ msgid ;
				zerr2 = "Message-id format invalid (2). " ;
				checkRCode() ;
				return false ;
			}
		}
		else
		{
			if ( msgid == "" || ( lmsg != "" && !lcontinue ) )
			{
				RC = 20 ;
				messages.close() ;
				zerr1 = "Error in message "+ msgid ;
				zerr2 = "Extraeneous data: "+ mline ;
				checkRCode() ;
				return false ;
			}
			lcontinue = ( mline.back() == '+' ) ;
			if ( lcontinue )
			{
				mline.pop_back() ;
				trim( mline )    ;
			}
			c = mline.front() ;
			if ( c == '\'' || c == '"' )
			{
				if ( mline.back() != c )
				{
					RC = 20 ;
					messages.close() ;
					zerr1 = "Error in message "+ msgid ;
					zerr2 = "No ending quote found in message." ;
					checkRCode() ;
					return false ;
				}
				tmp = dquote( errblk, c, mline ) ;
				if ( errblk.error() )
				{
					RC = 20 ;
					messages.close() ;
					zerr1 = "Error in message "+ msgid ;
					zerr2 = "Error (3) in message-id." ;
					checkRCode() ;
					return false ;
				}
			}
			else
			{
				if ( words( mline ) > 1 )
				{
					RC = 20 ;
					messages.close() ;
					zerr1 = "Error in message "+ msgid ;
					zerr2 = "Error (4) in message-id." ;
					checkRCode() ;
					return false ;
				}
				tmp = mline ;
			}
			lmsg = lmsg + tmp ;
		}
	}
	if ( messages.bad() )
	{
		RC = 20 ;
		messages.close() ;
		zerr1 = "Error loading "+ msgid ;
		zerr2 = "Error reading message file "+ filename ;
		checkRCode() ;
		return false ;
	}

	messages.close() ;

	if ( smsg != "" )
	{
		if ( lcontinue )
		{
			RC = 20 ;
			messages.close() ;
			zerr1 = "Error in message "+ msgid ;
			zerr2 = "Invalid continuation found." ;
			checkRCode() ;
			return false ;
		}
		if ( t.parse( smsg, lmsg ) > 0 )
		{
			RC = 20 ;
			messages.close() ;
			zerr1 = "Error in message "+ msgid ;
			zerr2 = "Error (5) in message-id "+ msgid ;
			checkRCode() ;
			return false ;
		}
		if ( msgList.count( msgid ) > 0 )
		{
			RC = 20 ;
			messages.close() ;
			zerr1 = "Error in message "+ msgid ;
			zerr2 = "Duplicate message-id found: "+ msgid ;
			checkRCode() ;
			return false ;
		}
		msgList[ msgid ] = t ;
	}

	if ( msgList.count( p_msg ) == 0 )
	{
		RC = 12 ;
		zerr1 = "Error in message "+ p_msg ;
		zerr2 = "Message-id not found in message file "+ p_msgfn1 ;
		checkRCode() ;
		return false ;
	}

	return true ;
}


bool pApplication::sub_message_vars( slmsg& t )
{
	//
	// Get the dialogue variable value specified in message .T, .A, .H and .W options.
	//
	// Error if the dialgue variable is blank.
	// This routine does not set the return code.  Set in the calling routine.
	//
	// Use defaults for invalid values.
	//
	// .TYPE overrides .WINDOW and .ALARM.
	//

	TRACE_FUNCTION() ;

	string val ;

	if ( t.dvwin != "" )
	{
		t.lmwin = true ;
		vcopy( t.dvwin, val, MOVE ) ;
		trim( val ) ;
		if ( val == "RESP" || val == "R"  )
		{
			t.smwin = true ;
			t.resp  = true ;
		}
		else if ( val == "NORESP" || val == "N" || val == "NR" )
		{
			t.smwin = true ;
		}
		else if ( val == "LRESP" || val == "LR" )
		{
			t.resp  = true ;
		}
		else if ( val != "LNORESP" && val == "LN" )
		{
		}
		else if ( val == "" )
		{
			return false ;
		}
	}

	if ( t.dvalm != "" )
	{
		vcopy( t.dvalm, val, MOVE ) ;
		trim( val ) ;
		if ( val == "YES" )
		{
			t.alm = true ;
		}
		else if ( val == "NO" )
		{
			t.alm = false ;
		}
		else if ( val == "" )
		{
			return false ;
		}
	}

	if ( t.dvtype != "" )
	{
		vcopy( t.dvtype, val, MOVE ) ;
		trim( val ) ;
		if ( val == "N" || val == "NOTIFY" || val == "INFO" )
		{
			t.type = IMT ;
			t.alm = false ;
		}
		else if ( val == "W" || val == "WARNING" || val == "ERROR" )
		{
			t.type = WMT ;
			t.alm = true ;
		}
		else if ( val == "A" || val == "ACTION" )
		{
			t.type = AMT ;
			t.alm = true ;
		}
		else if ( val == "C" || val == "CRITICAL" )
		{
			t.type  = AMT ;
			t.alm   = true ;
			t.resp  = true ;
			t.smwin = true ;
			t.lmwin = true ;
		}
		else if ( val == "" )
		{
			return false ;
		}
	}

	if ( t.dvhlp != "" )
	{
		vcopy( t.dvhlp, t.hlp, MOVE ) ;
	}

	return true ;
}


int pApplication::check_message_id( const string& msgid )
{
	//
	// Return 0 if message-id format is incorrect, else the offset to the first numeric triplet.
	//

	TRACE_FUNCTION() ;

	int l = msgid.size() ;

	if ( l < 4 || !isvalidName( msgid ) )
	{
		return 0 ;
	}

	l -= 2 ;
	for ( int i = 1 ; i < l ; ++i )
	{
		if ( isdigit( msgid[ i ] )     &&
		     isdigit( msgid[ i + 1 ] ) &&
		     isdigit( msgid[ i + 2 ] ) )
		{
			return i ;
		}
	}

	return 0 ;
}


string pApplication::sub_vars( string s )
{
	//
	// In string, s, substitute variables starting with '&' for their dialogue value.
	// The variable is delimited by any non-valid variable characters.
	// Rules: A '.' at the end of a variable, concatinates the value with the string following the dot
	//        .. reduces to .
	//        && reduces to & with no variable substitution.
	//

	TRACE_FUNCTION() ;

	size_t p1 ;
	size_t p2 ;

	string var ;
	string val ;

	const string validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#$@" ;
	p1 = 0 ;

	while ( true )
	{
		p1 = s.find( '&', p1 ) ;
		if ( p1 == string::npos || p1 == s.size() - 1 ) { break ; }
		++p1 ;
		if ( s[ p1 ] == '&' )
		{
			s.erase( p1, 1 ) ;
			p1 = s.find_first_not_of( '&', p1 ) ;
			continue ;
		}
		p2  = s.find_first_not_of( validChars, p1 ) ;
		if ( p2 == string::npos ) { p2 = s.size() ; }
		var = upper( s.substr( p1, p2-p1 ) ) ;
		if ( isvalidName( var ) )
		{
			val = "" ;
			vcopy( var, val, MOVE ) ;
			if ( RC <= 8 )
			{
				trim_right( val ) ;
				if ( p2 < s.size() && s[ p2 ] == '.' )
				{
					s.replace( p1-1, var.size()+2, val ) ;
				}
				else
				{
					s.replace( p1-1, var.size()+1, val ) ;
				}
				p1 = p1 + val.size() - 1 ;
			}
		}
	}
	RC = 0   ;
	return s ;
}


void pApplication::check_qrname( const string& e1,
				 const string& maj,
				 const string& min )
{
	TRACE_FUNCTION() ;

	if ( !isvalidName( maj ) )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYS013F" ) ;
		checkRCode( errblk ) ;
	}
	else if ( min == "" )
	{
		errblk.setcall( TRACE_INFO(), e1, "PSYS013G" ) ;
		checkRCode( errblk ) ;
	}
}


string pApplication::sysexec()
{
	//
	// Return the basic search path for REXX commands.
	//

	TRACE_FUNCTION() ;

	return get_ddname_search_paths( "SYSEXEC" ) ;
}


void pApplication::show_enqueues()
{
	TRACE_FUNCTION() ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	llog( "I", ".ENQ" << endl ) ;
	llog( "-", "*************************************************************************************************************" << endl ) ;
	llog( "-", "Local enqueues held by task "<< d2ds( taskId, 8 ) << endl ) ;
	llog( "-", "Exc/Share  Major Name  Minor Name "<< endl ) ;
	llog( "-", "---------  ----------  ---------- "<< endl ) ;

	for ( auto it = l_enqueues.begin() ; it != l_enqueues.end() ; ++it )
	{
		llog( "-", "" << setw( 11 ) << std::left << ( it->disp == EXC ? "EXCLUSIVE" : "SHARE" )
			      << setw( 8 )  << std::left << it->maj_name << "    " << it->min_name << endl ) ;
	}

	llog( "-", "*************************************************************************************************************" << endl ) ;

	p_gls->show_enqueues( errblk ) ;
}


void pApplication::info()
{
	TRACE_FUNCTION() ;

	llog( "I", ".INFO" << endl ) ;
	llog( "-", "*************************************************************************************************************" << endl ) ;
	llog( "-", "Application Information for "<< zappname << endl ) ;
	llog( "-", "                 Thread ID: "<< pThread->get_id() << endl ) ;
	llog( "-", "                   Task ID: "<< d2ds( taskId, 8 ) << endl ) ;
	llog( "-", "            Parent Task ID: "<< d2ds( ptid, 8 ) << endl ) ;
	llog( "-", "          Shared Pool Name: "<< d2ds( shrdPool, 8 ) << endl ) ;
	llog( "-", "         Profile Pool Name: "<< get_applid() << endl ) ;
	llog( "-", " " << endl ) ;
	llog( "-", "Application Description . : "<< zappdesc << endl ) ;
	llog( "-", "Application Version . . . : "<< zappver  << endl ) ;
	llog( "-", "Current Application Status: "<< get_status() << endl ) ;
	llog( "-", "Last Panel Displayed. . . : "<< currPanel->panelid << endl ) ;
	llog( "-", "Last Message Displayed. . : "<< zmsgid << endl ) ;
	llog( "-", "Number of Panels Loaded . : "<< panelList.size() << endl )  ;
	if ( rexxName != "" )
	{
		llog( "-", "Application running REXX. : "<< rexxName << endl ) ;
	}
	llog( "-", " " << endl ) ;
	if ( debugMode )
	{
		llog( "-", "Application running in debugging mode"<< endl ) ;
	}
	if ( passlib )
	{
		llog( "-", "Application started with PASSLIB option"<< endl ) ;
	}
	if ( newpool )
	{
		llog( "-", "Application started with NEWPOOL option"<< endl ) ;
	}
	llog( "-", "*************************************************************************************************************" << endl ) ;
}


string pApplication::get_status()
{
	TRACE_FUNCTION() ;

	string t ;

	if ( applicationEnded )
	{
		t = ( abnormalEnd ) ? "Abended" : "Ended" ;
	}
	else
	{
		switch ( waiting_on )
		{
		case WAIT_NONE:
			t = ( abnormalEnd ) ? "Abending" : "Running" ;
			break ;

		case WAIT_OUTPUT:
			t = "Waiting on output" ;
			break ;

		case WAIT_SELECT:
			t = "Waiting on SELECT" ;
			break ;

		case WAIT_USER:
			t = "Waiting on user" ;
			break ;
		}
	}

	return t ;
}


const string& pApplication::get_jobkey()
{
	//
	// Create a string that uniquely identifies this job.
	// Of the form: yyyyddd-hhmmsstt-nnnnn
	// where nnnn is the taskid.
	//
	// Used for creating spool file names, etc.
	//

	TRACE_FUNCTION() ;

	string ztimel ;
	string zj4date ;

	if ( zjobkey == "" )
	{
		ztimel  = startTime ;
		zj4date = startDate ;
		zj4date.erase( 4, 1 ) ;
		ztimel.erase( 8, 1 ) ;
		ztimel.erase( 5, 1 ) ;
		ztimel.erase( 2, 1 ) ;
		zjobkey = zj4date + "-" + ztimel + "-" + d2ds( taskid(), 5 ) ;
	}

	return zjobkey ;
}


void pApplication::loadCommandTable()
{
	//
	// Load application command table in the application task so it can be unloaded on task termination.
	// This is done during SELECT processing so LIBDEFs must be active with PASSLIB specified,
	// if being used to find the table.
	//

	TRACE_FUNCTION() ;

	p_tableMGR->tbopen( errblk,
			    get_applid() + "CMDS",
			    NOWRITE,
			    get_libdef_search_paths( s_ZTLIB ),
			    SHARE ) ;
	if ( errblk.RC0() )
	{
		cmdTableLoaded = true ;
	}
}


void pApplication::ispexec( const string& s )
{
	TRACE_FUNCTION() ;

	const string e1 = "ISPEXEC Interface Error" ;

	lspf::ispexeci( this, s, errblk ) ;
	CHECK_ERROR_SETCALL_RETURN( e1 )

	errblk.clearsrc() ;
}


void pApplication::checkRCode()
{
	//
	// If the error panel is to be displayed, cancel CONTROL DISPLAY LOCK and remove any popup's.
	//
	// If this is issued as a result of a service call (a call from another thread ie. lspf), just return.
	// The calling thread needs to check further for errors as there is not much that can be done here.
	//

	TRACE_FUNCTION() ;

	int RC1 ;

	if ( errblk.ServiceCall() )
	{
		errblk.seterror() ;
		return ;
	}

	RC1 = RC ;

	if ( zerr1 != "" ) { llog( "E", zerr1 << endl ) ; }
	if ( zerr2 != "" ) { llog( "E", zerr2 << endl ) ; }

	if ( !controlErrorsReturn && RC >= 12 )
	{
		llog( "E", "RC="<< RC <<" CONTROL ERRORS CANCEL is in effect.  Aborting..."<< endl ) ;
		vreplace( "ZAPPNAME", zappname ) ;
		vreplace( "ZERRRX", rexxName ) ;
		vreplace( "ZERR1",  zerr1 ) ;
		vreplace( "ZERR2",  zerr2 ) ;
		vreplace( "ZERR3",  zerr3 ) ;
		vreplace( "ZERR4",  zerr4 ) ;
		vreplace( "ZERR5",  zerr5 ) ;
		vreplace( "ZERR6",  zerr6 ) ;
		vreplace( "ZERR7",  zerr7 ) ;
		vreplace( "ZERR8",  zerr8 ) ;
		vreplace( "ZERRRC", d2ds( RC1 ) ) ;
		controlDisplayLock  = false ;
		controlErrorsReturn = true  ;
		selPanel            = false ;
		if ( addpop_active ) { rempop( "ALL" ) ; }
		display( "PSYSER1" )  ;
		if ( RC <= 8 ) { errPanelissued = true ; }
		abend() ;
	}
}


void pApplication::checkRCode( errblock err )
{
	//
	// If the error panel is to be displayed, cancel CONTROL DISPLAY LOCK/NONDISPL and remove any popup's.
	//
	// Format: msg1   header - call description resulting in the error.
	//         short  msg.
	//         longer description.
	//
	// Set RC to the error code in the error block if we are returning to the program (CONTROL ERRORS RETURN).
	//
	// Terminate processing if this routing is called during error processing.
	//
	// If this is issued as a result of a service call (a call from another thread ie. lspf), just return.
	// The calling thread needs to check further for errors as there is not much that can be done here.
	//

	TRACE_FUNCTION() ;

	string t ;

	if ( err.ServiceCall() )
	{
		return ;
	}

	if ( !initc )
	{
		llog( "E", "Errors have occured during program constructor processing."<<endl ) ;
		llog( "E", "Error msg  : "<< err.msg1 << endl )  ;
		llog( "E", "Error RC   : "<< err.getRC() << endl ) ;
		llog( "E", "Error id   : "<< err.msgid << endl ) ;
		llog( "E", "Error mod  : "<< err.mod << endl ) ;
		llog( "E", "Error ZVAL1: "<< err.val1 << endl )  ;
		llog( "E", "Error ZVAL2: "<< err.val2 << endl )  ;
		llog( "E", "Error ZVAL3: "<< err.val3 << endl )  ;
		initRC = 20 ;
		return ;
	}

	if ( err.abending() )
	{
		llog( "E", "Errors have occured during error processing.  Terminating application."<<endl ) ;
		llog( "E", "Error Appl : "<< zappname << endl )  ;
		llog( "E", "Error REXX : "<< rexxName << endl )  ;
		llog( "E", "Error msg  : "<< err.msg1 << endl )  ;
		llog( "E", "Error RC   : "<< err.getRC() << endl ) ;
		llog( "E", "Error id   : "<< err.msgid << endl ) ;
		llog( "E", "Error mod  : "<< err.mod << endl ) ;
		llog( "E", "Error ZVAL1: "<< err.val1 << endl )  ;
		llog( "E", "Error ZVAL2: "<< err.val2 << endl )  ;
		llog( "E", "Error ZVAL3: "<< err.val3 << endl )  ;
		llog( "E", "Source     : "<< err.getsrc() << endl ) ;
		abend() ;
	}

	errblk.setAbending() ;

	if ( err.val1 != "" ) { vreplace( "ZVAL1", err.val1 ) ; }
	if ( err.val2 != "" ) { vreplace( "ZVAL2", err.val2 ) ; }
	if ( err.val3 != "" ) { vreplace( "ZVAL3", err.val3 ) ; }

	getmsg( err.msgid,
		"ZERRSM",
		"ZERRLM" ) ;

	vreplace( "ZERRMSG", err.msgid ) ;
	vreplace( "ZERRMOD", err.mod ) ;
	vreplace( "ZERRRC", d2ds( err.getRC() ) ) ;

	vcopy( "ZERRSM", t, MOVE ) ;
	if ( t != "" ) { llog( "E", t << endl ) ; }
	vcopy( "ZERRLM", t, MOVE ) ;
	if ( t != "" )
	{
		llog( "E", t << endl ) ;
		splitZerrlm( t ) ;
	}

	vreplace( "ZERR1", err.msg1 ) ;
	vreplace( "ZERR2", "" ) ;
	vreplace( "ZERR3", err.getsrc() ) ;

	vreplace( "ZAPPNAME", zappname ) ;
	vreplace( "ZERRRX", rexxName )  ;

	if ( err.getsrc() != "" )
	{
		if ( err.dialogsrc() )
		{
			vreplace( "ZERR2", "Current dialogue statement:" ) ;
		}
		else if ( err.ftsrc() )
		{
			vreplace( "ZERR2",  "Skeleton statement where error was detected:" ) ;
		}
		else if ( err.panelsrc() )
		{
			vreplace( "ZERR2",  "Panel statement where error was detected:" ) ;
		}
		else
		{
			vreplace( "ZERR2",  "Line where error was detected:" ) ;
		}
	}

	if ( controlErrorsReturn )
	{
		if ( service )
		{
			abending1 = true ;
			(this->*pcleanup)() ;
			ZRC     = 20 ;
			ZRSN    = 999 ;
			ZRESULT = "*0123456789NOABEND" ;
			vput( "ZERRMSG ZERRSM ZERRLM ZVAL1 ZVAL2 ZVAL3 ZERR1 ZERR2 ZERR3", SHARED ) ;
			throw pApplication::xTerminate() ;
		}
		RC = err.getRC() ;
		errblk = err ;
		return ;
	}

	llog( "E", err.msg1 << endl ) ;
	llog( "E", "Detecting module " << err.mod << endl ) ;
	llog( "E", "RC="<< err.getRC() <<" CONTROL ERRORS CANCEL is in effect.  Aborting..." << endl ) ;

	clr_nondispl() ;

	controlDisplayLock  = false ;
	controlErrorsReturn = true  ;
	selPanel            = false ;

	if ( addpop_active ) { rempop( "ALL" ) ; }
	errblk.clear() ;

	display( "PSYSER2" ) ;
	if ( RC <= 8 ) { errPanelissued = true ; }

	abend() ;
}


void pApplication::splitZerrlm( string t )
{
	TRACE_FUNCTION() ;

	size_t i ;

	int l ;
	int maxw ;

	maxw = ds2d( p_poolMGR->get( errblk, "ZSCRMAXW", SHARED ) ) - 6 ;
	l    = 0 ;

	do
	{
		++l ;
		if ( t.size() > size_t( maxw ) )
		{
			i = t.find_last_of( ' ', maxw ) ;
			i = ( i == string::npos ) ? maxw : i + 1 ;
			vreplace( "ZERRLM" + d2ds( l ), t.substr( 0, i ) ) ;
			t.erase( 0, i ) ;
		}
		else
		{
			vreplace( "ZERRLM" + d2ds( l ), t ) ;
			t = "" ;
		}
	} while ( t.size() > 0 ) ;

}


string pApplication::full_name( const string& a,
				const string& b )
{
	TRACE_FUNCTION() ;

	return ( a.back() == '/' ) ? a + b : a + "/" + b ;
}


string pApplication::get_shared_var( const string& var )
{
	TRACE_FUNCTION() ;

	return p_poolMGR->get( errblk, var, SHARED ) ;
}


/* ************************************ ***************************** ********************************* */
/* ************************************ Abnormal Termination Routines ********************************* */
/* ************************************ ***************************** ********************************* */

void pApplication::cleanup_default()
{
	//
	// Dummy routine.  Override in the application so the customised one is called on an exception condition.
	// Use CONTROL ABENDRTN ptr_to_routine to set.
	//     Example: control( "ABENDRTN", static_cast<void (pApplication::*)()>(&pedit01::cleanup_custom) ) ;
	//
	// Called on: abend()
	//            abendexc()
	//            set_timeout_abend()
	//
	// When this routine is called (or the override), abending1 is set to true.
	//
}


void pApplication::handle_sigterm()
{
	//
	// Override in the application so the customised one is called on a SIGTERM signal.
	// Use CONTROL SIGTERM ptr_to_routine to set.
	//

	TRACE_FUNCTION() ;

	abend( "PSYS014A" ) ;
}


void pApplication::handle_sigusr1()
{
	//
	// Override in the application so the customised one is called on a SIGUSR1 signal.
	// Use CONTROL SIGUSR1 ptr_to_routine to set.
	//

	TRACE_FUNCTION() ;

	interrupted = true ;
}


void pApplication::abend()
{
	TRACE_FUNCTION() ;

	abend_nothrow() ;
	throw pApplication::xTerminate() ;
}


void pApplication::abend( const string& msgid,
			  const string& val1,
			  const string& val2,
			  const string& val3 )
{
	TRACE_FUNCTION() ;

	errblk.setabend( msgid, val1, val2, val3 ) ;
	checkRCode( errblk ) ;
}


void pApplication::abend_nothrow()
{
	TRACE_FUNCTION() ;

	llog( "E", "Shutting down application: "+ zappname +" Taskid: " << taskId << " due to an abnormal condition" << endl ) ;
	abnormalEnd   = true  ;
	abending1     = true  ;
	terminateAppl = true  ;
	SEL           = false ;

	controlErrorsReturn = true ;
	(this->*pcleanup)() ;
	abended = true ;
}


void pApplication::uabend()
{
	//
	// Abend application with no messages.
	//

	TRACE_FUNCTION() ;

	abnormalEnd   = true  ;
	abnormalNoMsg = true  ;
	terminateAppl = true  ;
	abending1     = true  ;
	SEL           = false ;

	controlErrorsReturn = true ;
	(this->*pcleanup)() ;
	abended = true ;
	throw pApplication::xTerminate() ;
}


void pApplication::uabend( const string& msgid,
			   int callno )
{
	//
	// Abend application with error screen.
	// Screen will show short and long messages from msgid, and the return code RC.
	// Optional variables can be coded after the message id and are placed in ZVAL1, ZVAL2 etc.
	//

	TRACE_FUNCTION() ;

	vreplace( "ZERRRC", d2ds( RC ) ) ;
	xabend( msgid, callno ) ;
}


void pApplication::uabend( const string& msgid,
			   const string& val1,
			   int callno )
{
	TRACE_FUNCTION() ;

	vreplace( "ZERRRC", d2ds( RC ) ) ;
	vreplace( "ZVAL1", val1 ) ;
	xabend( msgid, callno ) ;
}


void pApplication::uabend( const string& msgid,
			   const string& val1,
			   const string& val2,
			   int callno )
{
	TRACE_FUNCTION() ;

	vreplace( "ZERRRC", d2ds( RC ) ) ;
	vreplace( "ZVAL1", val1 ) ;
	vreplace( "ZVAL2", val2 ) ;
	xabend( msgid, callno ) ;
}


void pApplication::uabend( const string& msgid,
			   const string& val1,
			   const string& val2,
			   const string& val3,
			   int callno )
{
	TRACE_FUNCTION() ;

	vreplace( "ZERRRC", d2ds( RC ) ) ;
	vreplace( "ZVAL1", val1 ) ;
	vreplace( "ZVAL3", val2 ) ;
	vreplace( "ZVAL4", val3 ) ;
	xabend( msgid, callno ) ;
}


void pApplication::xabend( const string& msgid,
			   int callno )
{
	//
	// Called after uabend() setup.
	//

	TRACE_FUNCTION() ;

	string t ;

	getmsg( msgid,
		"ZERRSM",
		"ZERRLM" ) ;

	if ( controlErrorsReturn )
	{
		if ( service )
		{
			abending1 = true ;
			(this->*pcleanup)() ;
			ZRC     = 20 ;
			ZRSN    = 999 ;
			ZRESULT = "*0123456789NOABEND" ;
			vreplace( "ZERRMSG", msgid )  ;
			vput( "ZERRMSG ZERRSM ZERRLM", SHARED ) ;
			throw pApplication::xTerminate() ;
		}
		RC = 20 ;
		return ;
	}

	vcopy( "ZERRLM", t, MOVE ) ;
	splitZerrlm( t ) ;

	t = "A user abend has occured in application "+ zappname ;
	if ( callno != -1 ) { t += " at call number " + d2ds( callno ) ; }

	llog( "E", "Shutting down application: "+ zappname +" Taskid: "<< taskId <<" due to a user abend" << endl ) ;

	vreplace( "ZAPPNAME", zappname ) ;
	vreplace( "ZERRRX", rexxName ) ;
	vreplace( "ZERRMSG", msgid ) ;
	vreplace( "ZERRMOD", "" ) ;
	vreplace( "ZERR1",  t  ) ;
	vreplace( "ZERR2",  "" ) ;
	vreplace( "ZERR3",  "" ) ;
	controlDisplayLock  = false ;
	controlErrorsReturn = true  ;
	selPanel            = false ;

	display( "PSYSER2" ) ;
	if ( RC <= 8 ) { errPanelissued = true ; }

	abnormalEnd   = true  ;
	terminateAppl = true  ;
	abending1     = true  ;
	SEL           = false ;

	controlErrorsReturn = true ;
	(this->*pcleanup)() ;
	abended = true ;
	throw pApplication::xTerminate() ;
}


void pApplication::abendexc()
{
	TRACE_FUNCTION() ;

	const char eol = 0x0A ;

	llog( "E", "An unhandled exception has occured in application: "+ zappname +" Taskid: " << taskId << endl ) ;
	if ( !abending2 )
	{
		controlErrorsReturn = true ;
		abending1 = true ;
		abending2 = true ;
		(this->*pcleanup)() ;
	}
	else
	{
		llog( "E", "An abend has occured during abend processing.  Cleanup will not be called" << endl ) ;
	}

	string t = boost::current_exception_diagnostic_information() ;

	size_t p = t.find( eol ) ;
	while ( p != string::npos )
	{
		llog( "E", "Exception: "+ t.substr( 0, p ) << endl ) ;
		t.erase( 0, p+1 ) ;
		p = t.find( eol ) ;
	}

	llog( "E", "Exception: "+ t << endl ) ;

	llog( "E", "Shutting down application: "+ zappname +" Taskid: " << taskId << endl ) ;
	abnormalEnd   = true  ;
	terminateAppl = true  ;
	SEL           = false ;
	abended       = true  ;
}


void pApplication::set_forced_abend()
{
	TRACE_FUNCTION() ;

	llog( "E", "Shutting down application: "+ zappname +" Taskid: "<< taskId <<" due to a forced condition" << endl ) ;
	abnormalEnd       = true  ;
	abnormalEndForced = true  ;
	terminateAppl     = true  ;
	SEL               = false ;
	abended           = true  ;
}


void pApplication::set_timeout_abend()
{
	TRACE_FUNCTION() ;

	llog( "E", "Shutting down application: "+ zappname +" Taskid: "<< taskId <<" due to a timeout condition" << endl ) ;
	abnormalEnd       = true  ;
	abnormalEndForced = true  ;
	abnormalTimeout   = true  ;
	terminateAppl     = true  ;
	abending1         = true  ;
	backgrd           = true  ;
	SEL               = false ;

	for ( auto& panel : panelList )
	{
		delete panel.second ;
	}
	panelList.clear() ;

	controlErrorsReturn = true ;
	(this->*pcleanup)() ;
	abended  = true  ;
	busyAppl = false ;
}
