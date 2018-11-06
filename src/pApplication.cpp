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

void ispexeci( pApplication*, const string&, errblock& ) ;

map<int, void*>pApplication::ApplUserData ;

pApplication::pApplication()
{
	testMode            = false  ;
	propagateEnd        = false  ;
	jumpEntered         = false  ;
	ControlDisplayLock  = false  ;
	ControlNonDispl     = false  ;
	ControlErrorsReturn = false  ;
	ControlPassLRScroll = false  ;
	ControlSplitEnable  = true   ;
	ControlRefUpdate    = true   ;
	lineOutDone         = false  ;
	lineOutPending      = false  ;
	nested              = false  ;
	errPanelissued      = false  ;
	abending            = false  ;
	abended             = false  ;
	addpop_active       = false  ;
	addpop_row          = 0      ;
	addpop_col          = 0      ;
	taskId              = 0      ;
	backgrd             = false  ;
	noTimeOut           = false  ;
	busyAppl            = true   ;
	terminateAppl       = false  ;
	applicationEnded    = false  ;
	abnormalEnd         = false  ;
	abnormalEndForced   = false  ;
	abnormalTimeout     = false  ;
	abnormalNoMsg       = false  ;
	reloadCUATables     = false  ;
	refreshlScreen      = false  ;
	rexxName            = ""     ;
	newpool             = false  ;
	newappl             = ""     ;
	passlib             = false  ;
	suspend             = true   ;
	SEL                 = false  ;
	selPanel            = false  ;
	setMessage          = false  ;
	lineBuffer          = ""     ;
	reffield            = ""     ;
	PARM                = ""     ;
	currPanel           = NULL   ;
	prevPanel           = NULL   ;
	currtbPanel         = NULL   ;
	zappver             = ""     ;
	zapphelp            = ""     ;
	ZRC                 = 0      ;
	ZRSN                = 0      ;
	ZRESULT             = ""     ;
	zerr1               = ""     ;
	zerr2               = ""     ;
	zerr3               = ""     ;
	zerr4               = ""     ;
	zerr5               = ""     ;
	zerr6               = ""     ;
	zerr7               = ""     ;
	zerr8               = ""     ;
}


pApplication::~pApplication()
{
	map<string, pPanel*>::iterator it;
	for ( it = panelList.begin() ; it != panelList.end() ; it++ )
	{
		delete it->second ;
	}
	busyAppl = false ;
}


void pApplication::init_phase1( selobj& sel, int taskid, void (* Callback)( lspfCommand& ) )
{
	// Setup various program parameters.
	// Variable services are not available at this time.

	zappname = sel.pgm  ;
	PARM     = sel.parm ;
	passlib  = sel.passlib ;
	newappl  = sel.newappl ;
	newpool  = sel.newpool ;
	suspend  = sel.suspend ;
	selPanel = sel.selPanel() ;
	nested   = sel.nested  ;
	options  = sel.options ;
	backgrd  = sel.backgrd ;
	taskId   = taskid      ;
	errBlock.taskid = taskid ;
	lspfCallback    = Callback ;
}


void pApplication::init_phase2()
{
	// Before being dispatched in its own thread, set the search paths and
	// create implicit function pool variables defined as INTEGER.

	// ZZ variables are for internal use only.  Don't use in applications.

	zzplib = p_poolMGR->get( errBlock, "ZPLIB", PROFILE ) ;
	zzmlib = p_poolMGR->get( errBlock, "ZMLIB", PROFILE ) ;
	zztlib = p_poolMGR->get( errBlock, "ZTLIB", PROFILE ) ;
	zztabl = p_poolMGR->get( errBlock, "ZTABL", PROFILE ) ;

	zzpusr = p_poolMGR->get( errBlock, "ZPUSR", PROFILE ) ;
	zzmusr = p_poolMGR->get( errBlock, "ZMUSR", PROFILE ) ;
	zztusr = p_poolMGR->get( errBlock, "ZTUSR", PROFILE ) ;
	zztabu = p_poolMGR->get( errBlock, "ZTABU", PROFILE ) ;

	funcPOOL.put( errBlock, "ZTDTOP",   0 ) ;
	funcPOOL.put( errBlock, "ZTDSELS",  0 ) ;
	funcPOOL.put( errBlock, "ZTDDEPTH", 0 ) ;
	funcPOOL.put( errBlock, "ZTDROWS",  0 ) ;
	funcPOOL.put( errBlock, "ZTDVROWS", 0 ) ;
	funcPOOL.put( errBlock, "ZCURPOS",  1 ) ;
	funcPOOL.put( errBlock, "ZCURINX",  0 ) ;
	funcPOOL.put( errBlock, "ZZCRP",    0 ) ;

	llog( "I", "Phase 2 initialisation complete" << endl ; )
}


void pApplication::run()
{
	// Start running the user application and catch any exceptions.
	// Not all exceptions are caught - eg. segmentation faults

	// Cleanup during termination:
	// 1) Close any tables opened by this application
	// 2) Unload the application command table

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

	for ( auto it = tablesOpen.begin() ; it != tablesOpen.end() ; it++ )
	{
		debug1( "Closing table " << *it << endl ) ;
		p_tableMGR->destroyTable( errBlock, *it ) ;
	}

	p_tableMGR->destroyTable( errBlock, get_applid() + "CMDS" ) ;

	if ( backgrd )
	{
		llog( "I", "Shutting down background application: " + zappname +" Taskid: " << taskId << endl ) ;
	}
	else
	{
		llog( "I", "Shutting down application: " + zappname +" Taskid: " << taskId << endl ) ;
		llog( "I", "Returning to calling program." << endl ) ;
	}

	terminateAppl    = true  ;
	applicationEnded = true  ;
	busyAppl         = false ;
}


void pApplication::wait_event()
{
	busyAppl = false ;

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
}


void pApplication::createPanel( const string& p_name )
{
	errBlock.setRC( 0 ) ;

	if ( panelList.count( p_name ) > 0 ) { return ; }

	if ( !isvalidName( p_name ) )
	{
		errBlock.seterrid( "PSYE021A", p_name ) ;
		return ;
	}

	pPanel* p_panel     = new pPanel ;
	p_panel->p_funcPOOL = &funcPOOL  ;
	p_panel->lrScroll   = ControlPassLRScroll ;
	p_panel->Rexx       = ( rexxName != "" )  ;
	p_panel->selPanel( selPanel ) ;
	p_panel->init( errBlock ) ;
	if ( errBlock.error() )
	{
		delete p_panel ;
		return ;
	}

	p_panel->loadPanel( errBlock, p_name, get_search_path( s_ZPLIB ) ) ;

	if ( errBlock.RC0() )
	{
		panelList[ p_name ] = p_panel ;
		load_keylist( p_panel ) ;
	}
	else
	{
		delete p_panel ;
		errBlock.setcall( "Error loading panel "+ p_name ) ;
		checkRCode( errBlock ) ;
	}
}


bool pApplication::isprimMenu()
{
	if ( currPanel ) { return currPanel->primaryMenu ; }
	return false ;
}


void pApplication::get_home( uint& row, uint& col )
{
	if ( currPanel ) { currPanel->get_home( row, col ) ; }
	else             { row = 0 ; col = 0               ; }
}


string pApplication::get_applid()
{
	return p_poolMGR->get( errBlock, "ZAPPLID", SHARED ) ;
}


void pApplication::get_cursor( uint& row, uint& col )
{
	if ( currPanel ) { currPanel->get_cursor( row, col ) ; }
	else             { row = 0 ; col = 0                 ; }
}


string pApplication::get_current_panelDesc()
{
	if ( currPanel ) { return currPanel->get_panelDesc() ; }
	return "" ;
}


string pApplication::get_current_screenName()
{
	return p_poolMGR->get( errBlock, "ZSCRNAME", SHARED ) ;
}


string pApplication::get_panelid()
{
	if ( currPanel ) { return currPanel->panelid ; }
	return "" ;
}


bool pApplication::inputInhibited()
{
	if ( currPanel ) { return currPanel->inputInhibited() ; }
	return false ;
}


bool pApplication::msgInhibited()
{
	if ( currPanel ) { return currPanel->msgInhibited() ; }
	return false ;
}


void pApplication::display_pd( errblock& err )
{
	if ( currPanel ) { currPanel->display_pd( err ) ; }
}


void pApplication::msgResponseOK()
{
	if ( currPanel ) { currPanel->msgResponseOK() ; }
}


bool pApplication::msg_issued_with_cmd()
{
	if ( currPanel ) { return currPanel->msg_issued_with_cmd() ; }
	return false ;
}


void pApplication::store_scrname()
{
	zscrname = p_poolMGR->get( errBlock, "ZSCRNAME", SHARED ) ;
}


bool pApplication::errorsReturn()
{
	return ControlErrorsReturn ;
}


void pApplication::setTestMode()
{
	testMode = true ;
	errBlock.setDebugMode() ;
}


void pApplication::restore_Zvars( int screenid )
{
	// Restore various variables after stacked application has terminated

	if ( currPanel )
	{
		currPanel->update_keylist_vars( errBlock ) ;
		p_poolMGR->put( errBlock, "ZPRIM", currPanel->get_zprim(), SHARED ) ;
	}

	if ( p_poolMGR->get( errBlock, screenid, "ZSCRNAM2" ) == "PERM" )
	{
		zscrname = p_poolMGR->get( errBlock, screenid, "ZSCRNAME" ) ;
	}

	p_poolMGR->put( errBlock, "ZSCRNAME", zscrname, SHARED ) ;
}


void pApplication::display_id()
{
	if ( currPanel ) { currPanel->display_id( errBlock ) ; }
}


void pApplication::set_msg( const string& msg_id )
{
	// Display a message on current panel using msg_id

	if ( !currPanel ) { return ; }

	get_message( msg_id ) ;
	if ( RC == 0 )
	{
		currPanel->clear_msg_loc() ;
		currPanel->set_panel_msg( zmsg, zmsgid ) ;
		currPanel->display_msg( errBlock )       ;
	}
}


void pApplication::set_msg1( const slmsg& t, const string& msgid, bool Immed )
{
	// Propogate setmsg() to the current panel using the short-long-message object.
	// If immed, display the message now rather than allow the application DISPLAY()/TBDISPL() service to it.

	// zmsg1 and zmsgid1 are used to store messages issued by the setmsg() service after variable substitution.

	zmsg1      = t     ;
	zmsgid1    = msgid ;
	setMessage = true  ;

	if ( Immed && currPanel )
	{
		currPanel->set_panel_msg( zmsg1, zmsgid1 ) ;
		currPanel->display_msg( errBlock ) ;
		setMessage = false ;
	}
}


void pApplication::clear_msg()
{
	if ( currPanel ) { currPanel->clear_msg() ; }
}


void pApplication::save_errblock()
{
	serBlock = errBlock ;
	errBlock.clear()    ;
	errBlock.setServiceCall() ;
}


void pApplication::restore_errblock()
{
	errBlock = serBlock ;
}


bool pApplication::nretriev_on()
{
	if ( currPanel ) { return currPanel->get_nretriev() ; }
	return false ;
}


string pApplication::get_nretfield()
{
	if ( currPanel ) { return currPanel->get_nretfield() ; }
	return "" ;
}


void pApplication::toggle_fscreen()
{
	currPanel->toggle_fscreen( addpop_active, addpop_row, addpop_col ) ;
	currPanel->display_panel( errBlock ) ;
	return ;
}


void pApplication::display( string p_name,
			    const string& p_msg,
			    const string& p_cursor,
			    int p_curpos )
{
	string zzverb ;

	const string e1 = "Error during DISPLAY of panel " + p_name ;
	const string e2 = "Error processing )INIT section of panel "   ;
	const string e3 = "Error processing )REINIT section of panel " ;
	const string e4 = "Error processing )PROC section of panel "   ;
	const string e5 = "Error during update of panel " ;
	const string e6 = "Error updating field values of panel " ;
	const string e7 = "Error processing )ATTR section of panel " ;

	bool doReinit = false ;

	RC = 0 ;

	if ( p_name == "" )
	{
		if ( !currPanel )
		{
			errBlock.setcall( e1, "PSYE021C" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		p_name   = currPanel->panelid ;
		doReinit = true ;
	}

	if ( p_cursor != "" && !isvalidName( p_cursor ) )
	{
		errBlock.setcall( e1, "PSYE023I", p_cursor ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	createPanel( p_name ) ;
	if ( !errBlock.RC0() )
	{
		errBlock.setcall( e1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	prevPanel = currPanel ;
	currPanel = panelList[ p_name ] ;

	if ( propagateEnd )
	{
		if ( prevPanel && prevPanel->panelid == p_name )
		{
			propagateEnd = false ;
		}
		else
		{
			ControlNonDispl = true ;
		}
	}

	currPanel->msgid = p_msg ;

	currPanel->set_cursor( p_cursor, p_curpos ) ;
	currPanel->set_msgloc( "" ) ;

	if ( addpop_active ) { currPanel->set_popup( addpop_row, addpop_col ) ; }
	else                 { currPanel->remove_popup()                      ; }

	p_poolMGR->put( errBlock, "ZPANELID", p_name, SHARED, SYSTEM ) ;

	if ( doReinit )
	{
		currPanel->display_panel_reinit( errBlock ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e3 + p_name ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}
	else
	{
		currPanel->display_panel_init( errBlock ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e2 + p_name ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		currPanel->display_panel_attrs( errBlock ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e7 + p_name ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		currPanel->update_field_values( errBlock ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e6 + p_name ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		RC = errBlock.getRC() ;
	}

	if ( setMessage )
	{
		if ( !ControlNonDispl )
		{
			currPanel->set_panel_msg( zmsg1, zmsgid1 ) ;
			setMessage = false ;
		}
	}
	else if ( currPanel->msgid != "" )
	{
		get_message( currPanel->msgid ) ;
		if ( RC > 0 ) { return ; }
		currPanel->set_panel_msg( zmsg, zmsgid ) ;
	}
	else
	{
		currPanel->clear_msg() ;
	}

	set_screenName() ;

	while ( true )
	{
		if ( lineOutDone )
		{
			refreshlScreen = true  ;
			wait_event() ;
			lineOutDone    = false ;
			refreshlScreen = false ;
		}
		currPanel->display_panel( errBlock ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e1 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		currPanel->cursor_placement( errBlock ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e1 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		wait_event() ;
		ControlDisplayLock = false ;
		ControlNonDispl    = false ;
		refreshlScreen     = false ;
		currPanel->hide_popup()    ;
		currPanel->resetAttrs_once() ;
		currPanel->display_panel_update( errBlock ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e5 + p_name ) ;
			checkRCode( errBlock ) ;
			return ;
		}

		currPanel->display_panel_proc( errBlock, 0 ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e4 + p_name ) ;
			checkRCode( errBlock ) ;
			return ;
		}

		zzverb = p_poolMGR->get( errBlock, "ZVERB", SHARED ) ;
		if ( !errBlock.RC0() )
		{
			errBlock.setcall( e1, "PSYE015L", "GET", "ZVERB" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		if ( propagateEnd || findword( zzverb, "END EXIT RETURN" ) )
		{
			if ( zzverb == "RETURN" ) { propagateEnd = true ; }
			RC = 8 ;
			return ;
		}

		if ( currPanel->msgid == "" ) { break ; }

		get_message( currPanel->msgid ) ;
		if ( RC > 0 ) { return ; }
		currPanel->set_panel_msg( zmsg, zmsgid ) ;
		currPanel->display_panel_reinit( errBlock ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e3 + p_name ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		RC = errBlock.getRC() ;
	}
	currPanel->clear_msg() ;
}


void pApplication::libdef( const string& lib,
			   const string& type,
			   const string& id,
			   const string& procopt )
{
	// libdef - Add/remove a list of paths to the search order for panels, messages and tables

	// Format:
	//         Application-level libraries
	//         LIBDEF ZxLIB                    - remove LIBDEF for search
	//         LIBDEF ZxLIB PATH ID(path-list) - add path-list to the search path
	//
	//         LIBDEF ZTABL                    - remove LIBDEF
	//         LIBDEF ZTABL PATH ID(path-list) - add path to LIBDEF output lib-type
	//
	// X - M, P or T
	// Path-list is a colon-separated list of directory names

	// Search order, user-level, application-level, system-level

	// RC = 0   Normal completion
	// RC = 4   Removing a LIBDEF that was not in effect
	//          STKADD specified but no stack in effect
	// RC = 8   COND specified but a LIBDEF is already in effect
	// RC = 16  No paths in the ID() parameter
	// RC = 20  Severe error

	const string e1 = "LIBDEF Error" ;

	int i ;
	int p ;

	bool proc_cond   ;
	bool proc_uncond ;
	bool proc_stack  ;
	bool proc_stkadd ;

	string dirname   ;

	stack<string>* zxlib ;

	RC = 0 ;

	if ( procopt != "" && !findword( procopt, "COND UNCOND STACK STKADD" ) )
	{
		errBlock.setcall( e1, "PSYE022I", procopt ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( not findword( lib, "ZMLIB ZPLIB ZTLIB ZTABL" ) )
	{
		errBlock.setcall( e1, "PSYE022H", lib ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	zxlib = &zlibd[ lib ] ;

	proc_cond   = ( procopt == "COND"   ) ;
	proc_uncond = ( procopt == "UNCOND" ) ;
	proc_stack  = ( procopt == "STACK" || procopt == "" ) ;
	proc_stkadd = ( procopt == "STKADD" ) ;

	if ( type == "" )
	{
		if ( id != "" || procopt != "" )
		{
			errBlock.setcall( e1, "PSYE023K" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		if ( zxlib->empty() )
		{
			RC = 4 ;
			return ;
		}
		zxlib->pop() ;
	}
	else if ( type == "PATH" )
	{
		if ( id == "" )
		{
			errBlock.setcall( e1, "PSYE022F", 16 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		for ( p = getpaths( id ), i = 1 ; i <= p ; i++ )
		{
			dirname = getpath( id, i ) ;
			try
			{
				if ( !exists( dirname ) || !is_directory( dirname ) )
				{
					errBlock.setcall( e1, "PSYE023L", dirname ) ;
					checkRCode( errBlock ) ;
					return ;
				}
			}
			catch ( boost::filesystem::filesystem_error &e )
			{
				errBlock.setcall( e1, "PSYS012C", e.what() ) ;
				checkRCode( errBlock ) ;
				return ;
			}
			catch (...)
			{
				errBlock.setcall( e1, "PSYS012C", "Entry: "+ dirname ) ;
				checkRCode( errBlock ) ;
				return ;
			}
		}
		if ( proc_cond && !zxlib->empty() )
		{
			RC = 8 ;
			return ;
		}
		else if ( proc_stkadd && zxlib->empty() )
		{
			RC = 4 ;
			return ;
		}
		if ( proc_cond || proc_uncond )
		{
			if ( zxlib->empty() ) { zxlib->push( id ) ; }
			else                  { zxlib->top() = id ; }
		}
		else if ( proc_stack )
		{
			zxlib->push( id ) ;
		}
		else
		{
			zxlib->top() = mergepaths( id, zxlib->top() ) ;
		}
	}
	else
	{
		errBlock.setcall( e1, "PSYE022G", type ) ;
		checkRCode( errBlock ) ;
	}
}


void pApplication::qlibdef( const string& lib, const string& type_var, const string& id_var )
{
	// query libdef status for lib-type lib

	const string e1 = "QLIBDEF Error" ;

	stack<string>* zxlib ;

	RC = 0 ;

	if ( not findword( lib, "ZMLIB ZPLIB ZTLIB ZTABL" ) )
	{
		errBlock.setcall( e1, "PSYE022H", lib ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	zxlib = &zlibd[ lib ] ;

	if ( zxlib->empty() )
	{
		RC = 4 ;
		return ;
	}

	if ( type_var != "" )
	{
		funcPOOL.put( errBlock, type_var, "PATH" ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e1 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}

	if ( id_var != "" )
	{
		funcPOOL.put( errBlock, id_var, zxlib->top() ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e1 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}
}


string pApplication::get_search_path( s_paths p )
{
	// Return the search path depending on the LIBDEFs in effect

	// Search Order:
	//                No LIBDEF       LIBDEF
	//                ----------------------
	//                                ZMUSR
	// Messages       ZMLIB           LIBDEF
	//                                ZMLIB
	//                ----------------------
	//                                ZPUSR
	// Panels         ZPLIB           LIBDEF
	//                                ZPLIB
	//                ----------------------
	//                                ZTUSR
	// Table Input    ZTLIB           LIBDEF
	//                                ZTLIB
	//                ----------------------
	//                                ZTABU
	// Table Output   ZTABL           LIBDEF
	//                                ZTABL
	//                ----------------------

	string* zzxusr ;
	string* zzxlib ;

	stack<string>* zxlib ;

	switch ( p )
	{
	case s_ZMLIB:
		zzxusr = &zzmusr ;
		zxlib  = &zlibd[ "ZMLIB" ] ;
		zzxlib = &zzmlib ;
		break ;

	case s_ZPLIB:
		zzxusr = &zzpusr ;
		zxlib  = &zlibd[ "ZPLIB" ] ;
		zzxlib = &zzplib ;
		break ;

	case s_ZTLIB:
		zzxusr = &zztusr ;
		zxlib  = &zlibd[ "ZTLIB" ] ;
		zzxlib = &zztlib ;
		break ;

	case s_ZTABL:
		zzxusr = &zztabu ;
		zxlib  = &zlibd[ "ZTABL" ] ;
		zzxlib = &zztabl ;
		break ;
	}

	if ( zxlib->empty() )
	{
		return *zzxlib ;
	}
	else if ( *zzxusr == "" )
	{
		return mergepaths( zxlib->top(), *zzxlib ) ;
	}
	else
	{
		return mergepaths( *zzxusr, zxlib->top(), *zzxlib ) ;
	}
}


void pApplication::set_screenName()
{
	if ( p_poolMGR->get( errBlock, "ZSCRNAM1", SHARED ) == "ON" &&
	     p_poolMGR->get( errBlock, lscreen_num, "ZSCRNAM2" ) == "PERM" )
	{
		p_poolMGR->put( errBlock, "ZSCRNAME", p_poolMGR->get( errBlock, lscreen_num, "ZSCRNAME" ), SHARED ) ;
	}
}


string pApplication::get_zsel()
{
	string zsel ;

	zsel = p_poolMGR->get( errBlock, "ZSEL" ) ;
	p_poolMGR->put( errBlock, "ZSEL", "" ) ;

	return zsel ;
}


string pApplication::get_dTRAIL()
{
	return funcPOOL.get( errBlock, 0, ".TRAIL", NOCHECK ) ;
}


void pApplication::set_cursor( int row, int col )
{
	if ( currPanel ) { currPanel->set_cursor( row, col ) ; }
}


void pApplication::set_cursor_home()
{
	if ( currPanel ) { currPanel->set_cursor_home() ; }
}


void pApplication::vdefine( const string& names,
			    int* i_ad1,
			    int* i_ad2,
			    int* i_ad3,
			    int* i_ad4,
			    int* i_ad5,
			    int* i_ad6,
			    int* i_ad7,
			    int* i_ad8 )
{
	// RC = 0  Normal completion
	// RC = 20 Severe Error
	// (funcPOOL.define returns 0 or 20)

	int w ;

	const string e1 = "VDEFINE Error" ;

	string name ;

	RC = 0 ;

	w = words( names ) ;
	if ( ( w > 8 ) || ( w < 1 ) )
	{
		errBlock.setcall( e1, "PSYE022D" ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( i_ad1 == NULL )
	{
		errBlock.setcall( e1, "PSYE022E" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	name = word( names, 1 ) ;
	funcPOOL.define( errBlock, name, i_ad1 ) ;
	if ( errBlock.error() ) { checkRCode( errBlock ) ; return ; }

	if ( w > 1 )
	{
		if ( i_ad2 == NULL )
		{
			errBlock.setcall( e1, "PSYE022E" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		name = word( names, 2 ) ;
		funcPOOL.define( errBlock, name, i_ad2 ) ;
		if ( errBlock.error() ) { checkRCode( errBlock ) ; return ; }
	}
	if ( w > 2 )
	{
		if ( i_ad3 == NULL )
		{
			errBlock.setcall( e1, "PSYE022E" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		name = word( names, 3 ) ;
		funcPOOL.define( errBlock, name, i_ad3 ) ;
		if ( errBlock.error() ) { checkRCode( errBlock ) ; return ; }
	}
	if ( w > 3 )
	{
		if ( i_ad4 == NULL )
		{
			errBlock.setcall( e1, "PSYE022E" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		name = word( names, 4 ) ;
		funcPOOL.define( errBlock, name, i_ad4 ) ;
		if ( errBlock.error() ) { checkRCode( errBlock ) ; return ; }
	}
	if ( w > 4 )
	{
		if ( i_ad5 == NULL )
		{
			errBlock.setcall( e1, "PSYE022E" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		name = word( names, 5 ) ;
		funcPOOL.define( errBlock, name, i_ad5 ) ;
		if ( errBlock.error() ) { checkRCode( errBlock ) ; return ; }
	}
	if ( w > 5 )
	{
		if ( i_ad6 == NULL )
		{
			errBlock.setcall( e1, "PSYE022E" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		name = word( names, 6 ) ;
		funcPOOL.define( errBlock, name, i_ad6 ) ;
		if ( errBlock.error() ) { checkRCode( errBlock ) ; return ; }
	}

	if ( w > 6 )
	{
		if ( i_ad7 == NULL )
		{
			errBlock.setcall( e1, "PSYE022E" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		name = word( names, 7 ) ;
		funcPOOL.define( errBlock, name, i_ad7 ) ;
		if ( errBlock.error() ) { checkRCode( errBlock ) ; return ; }
	}

	if ( w > 7 )
	{
		if ( i_ad8 == NULL )
		{
			errBlock.setcall( e1, "PSYE022E" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		name = word( names, 8 ) ;
		funcPOOL.define( errBlock, name, i_ad8 ) ;
		if ( errBlock.error() ) { checkRCode( errBlock ) ; return ; }
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
			    string* s_ad8 )
{
	// RC = 0  Normal completion
	// RC = 20 Severe Error
	// (funcPOOL.define returns 0 or 20)

	int w  ;

	string name ;
	const string e1 = "VDEFINE Error" ;

	RC = 0 ;

	w = words( names ) ;
	if ( ( w > 8 ) || ( w < 1 ) )
	{
		errBlock.setcall( e1, "PSYE022D" ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( s_ad1 == NULL )
	{
		errBlock.setcall( e1, "PSYE022E" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	name = word( names, 1 ) ;
	funcPOOL.define( errBlock, name, s_ad1 ) ;
	if ( errBlock.error() ) { checkRCode( errBlock ) ; return ; }

	if ( w > 1 )
	{
		if ( s_ad2 == NULL )
		{
			errBlock.setcall( e1, "PSYE022E" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		name = word( names, 2 ) ;
		funcPOOL.define( errBlock, name, s_ad2 ) ;
		if ( errBlock.error() ) { checkRCode( errBlock ) ; return ; }
	}
	if ( w > 2 )
	{
		if ( s_ad3 == NULL )
		{
			errBlock.setcall( e1, "PSYE022E" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		name = word( names, 3 ) ;
		funcPOOL.define( errBlock, name, s_ad3 ) ;
		if ( errBlock.error() ) { checkRCode( errBlock ) ; return ; }
	}
	if ( w > 3 )
	{
		if ( s_ad4 == NULL )
		{
			errBlock.setcall( e1, "PSYE022E" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		name = word( names, 4 ) ;
		funcPOOL.define( errBlock, name, s_ad4 ) ;
		if ( errBlock.error() ) { checkRCode( errBlock ) ; return ; }
	}
	if ( w > 4 )
	{
		if ( s_ad5 == NULL )
		{
			errBlock.setcall( e1, "PSYE022E" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		name = word( names, 5 ) ;
		funcPOOL.define( errBlock, name, s_ad5 ) ;
		if ( errBlock.error() ) { checkRCode( errBlock ) ; return ; }
	}
	if ( w > 5 )
	{
		if ( s_ad6 == NULL )
		{
			errBlock.setcall( e1, "PSYE022E" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		name = word( names, 6 ) ;
		funcPOOL.define( errBlock, name, s_ad6 ) ;
		if ( errBlock.error() ) { checkRCode( errBlock ) ; return ; }
	}
	if ( w > 6 )
	{
		if ( s_ad7 == NULL )
		{
			errBlock.setcall( e1, "PSYE022E" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		name = word( names, 7 ) ;
		funcPOOL.define( errBlock, name, s_ad7 ) ;
		if ( errBlock.error() ) { checkRCode( errBlock ) ; return ; }
	}
	if ( w > 7 )
	{
		if ( s_ad8 == NULL )
		{
			errBlock.setcall( e1, "PSYE022E" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		name = word( names, 8 ) ;
		funcPOOL.define( errBlock, name, s_ad8 ) ;
		if ( errBlock.error() ) { checkRCode( errBlock ) ; return ; }
	}
}


void pApplication::vdelete( const string& names )
{
	// RC = 0  Normal completion
	// RC = 8  At least one variable not found
	// RC = 20 Severe error
	// (funcPOOL.dlete returns 0, 8 or 20)

	int i  ;
	int ws ;

	errBlock.setmaxRC( 0 ) ;

	ws = words( names ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		funcPOOL.dlete( errBlock, word( names, i ) ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( "VDELETE failed" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		errBlock.setmaxRC() ;
	}
	RC = errBlock.getmaxRC() ;
}


void pApplication::vmask( const string& name, const string& type, const string& mask )
{
	// Set a mask for a function pool variable (must be vdefined first)
	// Partial implementation as no VEDIT panel statement yet so this is never used

	// RC = 0  Normal completion
	// RC = 8  Variable not found
	// RC = 20 Severe error
	// (funcPOOL.setmask returns 0, 8 or 20)

	const string e1    = "VMASK Error" ;
	const string fmask = "IDATE STDDATE ITIME STDTIME JDATE JSTD" ;

	if ( type == "FORMAT" )
	{
		if ( wordpos( mask, fmask ) == 0 )
		{
			errBlock.setcall( e1, "PSYE022P", mask, fmask ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}
	else if ( type == "USER" )
	{
		if ( mask.size() > 20 )
		{
			errBlock.setcall( e1, "PSYE022Q" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		else
		{
			for ( unsigned int i = 0 ; i < mask.size() ; i++ )
			{
				if ( mask[i] != 'A' && mask[i] != 'B' && mask[i] != '9' &&
				     mask[i] != 'H' && mask[i] != 'N' && mask[i] != 'V' &&
				     mask[i] != 'S' && mask[i] != 'X' && mask[i] != '(' &&
				     mask[i] != ')' && mask[i] != '-' && mask[i] != '/' &&
				     mask[i] != ',' && mask[i] != '.' )
				{
					errBlock.setcall( e1, "PSYE022S", mask ) ;
					checkRCode( errBlock ) ;
					return ;
				}
			}
		}
	}
	else
	{
		errBlock.setcall( e1, "PSYE022R", type ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	funcPOOL.setmask( errBlock, name, mask ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( e1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::vreset()
{
	// RC = 0  Normal completion
	// RC = 20 Severe error
	// (funcPOOL.reset returns 0)

	funcPOOL.reset( errBlock ) ;
	RC = errBlock.getRC() ;
}


void pApplication::vreplace( const string& name, const string& s_val )
{
	// RC = 0  Normal completion
	// RC = 20 Severe error
	// (funcPOOL.put returns 0, 20)

	funcPOOL.put( errBlock, name, s_val ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "VREPLACE failed" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::vreplace( const string& name, int i_val )
{
	// RC = 0  Normal completion
	// RC = 20 Severe error
	// (funcPOOL.put returns 0 or 20)

	funcPOOL.put( errBlock, name, i_val ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "VREPLACE failed" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::vget( const string& names, poolType pType )
{
	// RC = 0  Normal completion
	// RC = 8  Variable not found
	// RC = 20 Severe error
	// (funcPOOL.put returns 0 or 20)
	// (funcPOOL.getType returns 0, 8 or 20.  For RC = 8 create implicit function pool variable)
	// (poolMGR.get return 0, 8 or 20)

	int ws      ;
	int i       ;
	string val  ;
	string name ;

	const string e1 = "VGET Error" ;

	dataType var_type ;

	errBlock.setmaxRC( 0 ) ;

	ws = words( names ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		name = word( names, i ) ;
		val  = p_poolMGR->get( errBlock, name, pType ) ;
		if ( errBlock.RC0() )
		{
			var_type = funcPOOL.getType( errBlock, name ) ;
			if ( errBlock.RC0() )
			{
				if ( var_type == INTEGER )
				{
					funcPOOL.put( errBlock, name, ds2d( val ) ) ;
				}
				else
				{
					funcPOOL.put( errBlock, name, val ) ;
				}
			}
			else if ( errBlock.RC8() )
			{
				funcPOOL.put( errBlock, name, val ) ;
			}
		}
		if ( errBlock.error() )
		{
			errBlock.setcall( e1 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		errBlock.setmaxRC() ;
	}
	RC = errBlock.getmaxRC() ;
}


void pApplication::vput( const string& names, poolType pType )
{
	// RC = 0  Normal completion
	// RC = 8  Variable not found in function pool
	// RC = 12 Read-only variable
	// RC = 16 Truncation occured
	// RC = 20 Severe error
	// (funcPOOL.get returns 0, 8 or 20)
	// (funcPOOL.getType returns 0, 8 or 20)
	// (poolMGR.put return 0, 12, 16 or 20)

	int i  ;
	int ws ;

	string s_val ;
	string name  ;

	const string e1 = "VPUT Error" ;

	dataType var_type ;

	errBlock.setmaxRC( 0 ) ;

	ws = words( names ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		name     = word( names, i ) ;
		var_type = funcPOOL.getType( errBlock, name ) ;
		if ( errBlock.RC0() )
		{
			if ( var_type == INTEGER )
			{
				s_val = d2ds( funcPOOL.get( errBlock, 0, var_type, name ) ) ;
			}
			else
			{
				s_val = funcPOOL.get( errBlock, 0, name ) ;
			}
			if ( errBlock.error() )
			{
				errBlock.setcall( e1 ) ;
				checkRCode( errBlock ) ;
				return ;
			}
			p_poolMGR->put( errBlock, name, s_val, pType ) ;
		}
		if ( errBlock.error() )
		{
			errBlock.setcall( e1 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		errBlock.setmaxRC() ;
	}
	RC = errBlock.getmaxRC() ;
}


void pApplication::vcopy( const string& var, string& val, vcMODE mode )
{
	// Retrieve a copy of a dialogue variable name in var and move to variable val
	// (normal dialogue variable search order)
	// This routine is only valid for MODE=MOVE

	// RC = 0  Normal completion
	// RC = 8  Variable not found
	// RC = 16 Truncation occured
	// RC = 20 Severe error

	// (funcPOOL.get returns 0, 8 or 20)
	// (funcPOOL.getType returns 0, 8 or 20)
	// (poolMGR.get return 0, 8 or 20)

	dataType var_type ;

	const string e1 = "VCOPY Error" ;

	switch ( mode )
	{
	case LOCATE:
		errBlock.setcall( e1, "PSYE022A" ) ;
		checkRCode( errBlock ) ;
		return ;

	case MOVE:
		var_type = funcPOOL.getType( errBlock, var ) ;
		if ( errBlock.RC0() )
		{
			if ( var_type == INTEGER )
			{
				val = d2ds( funcPOOL.get( errBlock, 0, var_type, var ) ) ;
			}
			else
			{
				val = funcPOOL.get( errBlock, 0, var ) ;
			}
			if ( errBlock.error() )
			{
				errBlock.setcall( e1 ) ;
				checkRCode( errBlock ) ;
				return ;
			}
		}
		else if ( errBlock.RC8() )
		{
			val = p_poolMGR->get( errBlock, var, ASIS ) ;
			if ( errBlock.error() )
			{
				errBlock.setcall( e1 ) ;
				checkRCode( errBlock ) ;
				return ;
			}
		}
		else
		{
			errBlock.setcall( e1 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}
	RC = errBlock.getRC() ;
}


void pApplication::vcopy( const string& var, string* & p_val, vcMODE mode )
{
	// Return the address of a dialogue variable name in var, in p_val
	// (normal dialogue variable search order)
	// This routine is only valid for MODE=LOCATE
	// MODE=LOCATE not valid for integer pointers as these may be in the variable pools as strings

	// RC = 0  Normal completion
	// RC = 8  Variable not found
	// RC = 16 Truncation occured
	// RC = 20 Severe error

	// (funcPOOL.get returns 0, 8 or 20)
	// (funcPOOL.getType returns 0, 8 or 20)
	// (funcPOOL.vlocate returns 0, 8 or 20)
	// (poolMGR.get return 0, 8 or 20)

	dataType var_type  ;

	const string e1 = "VCOPY Error" ;

	switch ( mode )
	{
	case LOCATE:
		var_type = funcPOOL.getType( errBlock, var ) ;
		if ( errBlock.RC0() )
		{
			if ( var_type == INTEGER )
			{
				errBlock.setcall( e1, "PSYE022C" ) ;
			}
			else
			{
				p_val = funcPOOL.vlocate( errBlock, var, CHECK ) ;
			}
			if ( errBlock.error() )
			{
				errBlock.setcall( e1 ) ;
				checkRCode( errBlock ) ;
				return ;
			}
		}
		else if ( errBlock.RC8() )
		{
			p_val = p_poolMGR->vlocate( errBlock, var, ASIS ) ;
			if ( errBlock.error() )
			{
				errBlock.setcall( e1 ) ;
				checkRCode( errBlock ) ;
				return ;
			}
		}
		else
		{
			errBlock.setcall( e1 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		break ;
	case MOVE:
		errBlock.setcall( e1, "PSYE022B" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::verase( const string& names, poolType pType )
{
	// RC = 0  Normal completion
	// RC = 8  Variable not found
	// RC = 12 Read-only variable
	// RC = 20 Severe error
	// (poolMGR.erase return 0, 8 12 or 20)


	int i  ;
	int ws ;

	string name ;

	errBlock.setmaxRC( 0 ) ;

	ws = words( names ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		name = word( names, i ) ;
		p_poolMGR->erase( errBlock, name, pType ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( "VERASE failed" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		errBlock.setmaxRC() ;
	}
	RC = errBlock.getmaxRC() ;
}


set<string>& pApplication::vlist( poolType pType, int lvl )
{
	return p_poolMGR->vlist( errBlock, RC, pType, lvl ) ;
}


set<string>& pApplication::vilist( vdType defn )
{
	return funcPOOL.vilist( RC, defn ) ;
}


set<string>& pApplication::vslist( vdType defn )
{
	return funcPOOL.vslist( RC, defn ) ;
}


void pApplication::addpop( const string& a_fld, int a_row, int a_col )
{
	//  Create pop-up window and set row/col for the next panel display.
	//  If addpop() is already active, store old values for next rempop()

	//  Position of addpop is relative to row=1, col=3 or the previous addpop() position for this logical screen.
	//  Defaults are 0,0 giving row=1, col=3 (or 2,4 when starting at 1,1)

	//  Force a refresh of the screen in case the same window has been displayed after another addpop.  This can
	//  leave parts of the old window on the screen. (save/restore panel stack by the logical screen)

	//  RC = 0  Normal completion
	//  RC = 12 No panel displayed before addpop() service when using field parameter
	//  RC = 20 Severe error

	const string e1 = "ADDPOP Error" ;

	uint p_row = 0 ;
	uint p_col = 0 ;

	RC = 0 ;

	if ( a_fld != "" )
	{
		if ( !currPanel )
		{
			errBlock.setcall( e1, "PSYE022L", 12 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		if ( !currPanel->field_get_row_col( a_fld, p_row, p_col ) )
		{
			errBlock.setcall( e1, "PSYE022M", a_fld, currPanel->panelid, 20 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		a_row += p_row ;
		a_col += p_col - 4 ;
	}

	if ( addpop_active )
	{
		addpop_stk.push( addpop_row ) ;
		addpop_stk.push( addpop_col ) ;
		a_row += addpop_row ;
		a_col += addpop_col ;
	}

	addpop_active = true ;
	addpop_row = (a_row <  0 ) ? 1 : a_row + 2 ;
	addpop_col = (a_col < -1 ) ? 2 : a_col + 4 ;

	if ( currPanel ) { currPanel->show_popup() ; }
	refreshlScreen = true ;
}


void pApplication::rempop( const string& r_all )
{
	//  Remove pop-up window.  Restore previous rempop() if there is one (push order row,col).

	//  RC = 0  Normal completion
	//  RC = 16 No pop-up window exists at this level
	//  RC = 20 Severe error

	const string e1 = "REMPOP Error" ;

	RC = 0 ;

	if ( r_all != "" && r_all != "ALL" )
	{
		errBlock.setcall( e1, "PSYE022U", r_all ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( !addpop_active )
	{
		errBlock.setcall( e1, "PSYE022T", 16 ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( r_all == "" )
	{
		if ( !addpop_stk.empty() )
		{
			addpop_col = addpop_stk.top() ;
			addpop_stk.pop() ;
			addpop_row = addpop_stk.top() ;
			addpop_stk.pop() ;
		}
		else
		{
			addpop_active = false ;
			addpop_row    = 0 ;
			addpop_col    = 0 ;
		}
	}
	else
	{
		while ( !addpop_stk.empty() )
		{
			addpop_stk.pop() ;
		}
		addpop_active = false ;
		addpop_row    = 0 ;
		addpop_col    = 0 ;
	}
}


void pApplication::movepop()
{
	if ( addpop_active )
	{
		currPanel->set_popup( addpop_row, addpop_col ) ;
		currPanel->move_popup() ;
	}
}


void pApplication::control( const string& parm1, const string& parm2, const string& parm3 )
{
	// CONTROL ERRORS CANCEL - abend for RC >= 12
	// CONTROL ERRORS RETURN - return to application for any RC

	// CONTROL DISPLAY SAVE/RESTORE - SAVE/RESTORE status for a TBDISPL
	//         SAVE/RESTORE saves/restores the function pool variables associated with a tb display
	//         (the six ZTD* variables and the .ZURID.ln variables), and also the currtbPanel
	//         pointer for retrieving other pending sets via a tbdispl with no panel specified.
	//         Only necessary if a tbdispl invokes another tbdispl within the same task

	// CONTROL SPLIT DISABLE - RC=8 if screen already split

	// CONTROL PASSTHRU LRSCROLL  PASON | PASOFF | PASQUERY

	// CONTROL REFLIST UPDATE
	// CONTROL REFLIST NOUPDATE

	// lspf extensions:
	// ----------------
	//
	// CONTROL ABENDRTN DEFAULT - Reset abend routine to the default, pApplication::cleanup_default
	// CONTROL RELOAD   CUA     - Reload the CUA tables
	// CONTROL TIMEOUT  ENABLE  - Enable application timeouts after ZWAITMAX ms (default).
	// CONTROL TIMEOUT  DISABLE - Disable forced abend of applications if ZWAITMAX exceeded.
	// CONTROL REFLIST  ON      - REFLIST retrieve is on for this application.  ZRESULT will replace field.
	// CONTROL REFLIST  OFF     - REFLIST retrieve is off for this application.

	int i ;

	const string e1 = "CONTROL Error" ;

	map<string, pPanel*>::iterator it;

	errBlock.setRC( 0 ) ;

	if ( parm3 != "" && parm1 != "PASSTHRU" )
	{
		errBlock.setcall( e1, "PSYE022V", parm1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( parm1 == "DISPLAY" )
	{
		if ( parm2 == "LOCK" )
		{
			ControlDisplayLock = true ;
		}
		else if ( parm2 == "NONDISPL" )
		{
			ControlNonDispl = true ;
		}
		else if ( parm2 == "REFRESH" )
		{
			refreshlScreen = true ;
		}
		else if ( parm2 == "SAVE" )
		{
			stk_int.push( funcPOOL.get( errBlock, 0, INTEGER, "ZTDDEPTH" ) ) ;
			stk_int.push( funcPOOL.get( errBlock, 0, INTEGER, "ZTDROWS"  ) ) ;
			stk_int.push( funcPOOL.get( errBlock, 0, INTEGER, "ZTDSELS"  ) ) ;
			stk_int.push( funcPOOL.get( errBlock, 0, INTEGER, "ZTDTOP"   ) ) ;
			stk_int.push( funcPOOL.get( errBlock, 0, INTEGER, "ZTDVROWS" ) ) ;
			tbpanel_stk.push( currtbPanel ) ;
			if ( currtbPanel && currtbPanel->tb_depth > 0 )
			{
				urid_stk.push( stack<string>() ) ;
				for ( i = 0 ; i < currtbPanel->tb_depth ; i++ )
				{
					urid_stk.top().push( funcPOOL.get( errBlock, 0, ".ZURID."+d2ds( i ), NOCHECK ) ) ;
				}
			}
		}
		else if ( parm2 == "RESTORE" )
		{
			if ( tbpanel_stk.empty() )
			{
				errBlock.setcall( e1, "PSYE022W" ) ;
				checkRCode( errBlock ) ;
				return ;
			}
			funcPOOL.put( errBlock, "ZTDVROWS", stk_int.top() ) ;
			stk_int.pop() ;
			funcPOOL.put( errBlock, "ZTDTOP", stk_int.top() ) ;
			stk_int.pop() ;
			funcPOOL.put( errBlock, "ZTDSELS", stk_int.top() ) ;
			stk_int.pop() ;
			funcPOOL.put( errBlock, "ZTDROWS", stk_int.top() ) ;
			stk_int.pop() ;
			funcPOOL.put( errBlock, "ZTDDEPTH", stk_int.top() ) ;
			stk_int.pop() ;
			currtbPanel = tbpanel_stk.top() ;
			tbpanel_stk.pop() ;
			if ( currtbPanel && currtbPanel->tb_depth > 0 && !urid_stk.empty() )
			{
				stack<string>* ptr_stk = &urid_stk.top() ;
				i = ptr_stk->size() ;
				while ( !ptr_stk->empty() )
				{
					i-- ;
					funcPOOL.put( errBlock, ".ZURID."+d2ds( i ), ptr_stk->top(), NOCHECK ) ;
					ptr_stk->pop() ;
				}
				urid_stk.pop() ;
			}
		}
		else
		{
			errBlock.setcall( e1, "PSYE022X", "DISPLAY", parm2 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}
	else if ( parm1 == "ERRORS" )
	{
		if ( parm2 == "RETURN" )
		{
			ControlErrorsReturn = true ;
		}
		else if ( parm2 == "CANCEL" )
		{
			ControlErrorsReturn = false ;
		}
		else
		{
			errBlock.setcall( e1, "PSYE022X", "ERRORS", parm2 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}
	else if ( parm1 == "PASSTHRU" )
	{
		if ( parm2 == "LRSCROLL" )
		{
			if ( parm3 == "PASON" )
			{
				ControlPassLRScroll = true ;
				for ( it = panelList.begin() ; it != panelList.end() ; it++ )
				{
					it->second->lrScroll = true ;
				}
			}
			else if ( parm3 == "PASOFF" )
			{
				ControlPassLRScroll = false ;
				for ( it = panelList.begin() ; it != panelList.end() ; it++ )
				{
					it->second->lrScroll = false ;
				}
			}
			else if ( parm3 == "PASQUERY" )
			{
				errBlock.setRC( ControlPassLRScroll ? 1 : 0 ) ;
			}
			else
			{
				errBlock.setcall( e1, "PSYE022X", "PASSTHRU LRSCROLL", parm3 ) ;
				checkRCode( errBlock ) ;
				return ;
			}
		}
		else
		{
			errBlock.setcall( e1, "PSYE022X", "PASSTHRU", parm2 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}
	else if ( parm1 == "RELOAD" )
	{
		if ( parm2 == "CUA" )
		{
			reloadCUATables = true ;
		}
		else
		{
			errBlock.setcall( e1, "PSYE022X", "RELOAD", parm2 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}
	else if ( parm1 == "REFLIST" )
	{
		if ( parm2 == "UPDATE" )
		{
			ControlRefUpdate = true ;
		}
		else if ( parm2 == "NOUPDATE" )
		{
			ControlRefUpdate = false ;
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
			errBlock.setcall( e1, "PSYE022X", "REFLIST", parm2 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}
	else if ( parm1 == "SPLIT" )
	{
		if ( parm2 == "ENABLE" )
		{
			ControlSplitEnable = true ;
		}
		else if ( parm2 == "DISABLE" )
		{
			if ( p_poolMGR->get( errBlock, "ZSPLIT", SHARED ) == "YES" )
			{
				errBlock.setRC( 8 ) ;
			}
			else
			{
				ControlSplitEnable = false ;
			}
		}
		else
		{
			errBlock.setcall( e1, "PSYE022X", "SPLIT", parm2 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}
	else if ( parm1 == "TIMEOUT" )
	{
		if ( parm2 == "ENABLE" )
		{
			noTimeOut = false ;
		}
		else if ( parm2 == "DISABLE" )
		{
			noTimeOut = true ;
		}
		else
		{
			errBlock.setcall( e1, "PSYE022X", "TIMEOUT", parm2 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}
	else if ( parm1 == "ABENDRTN" )
	{
		if ( parm2 == "DEFAULT" )
		{
			pcleanup = &pApplication::cleanup_default ;
		}
		else
		{
			errBlock.setcall( e1, "PSYE022X", "ABENDTRN", parm2 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}
	else
	{
		errBlock.setcall( e1, "PSYE022Y", parm1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	RC = errBlock.getRC() ;
}


void pApplication::control( const string& parm1, void (pApplication::*pFunc)() )
{
	// lspf extensions:
	//
	// CONTROL ABENDRTN ptr_to_routine - Set the routine to get control during an abend

	const string e1 = "CONTROL Error" ;

	errBlock.setRC( 0 ) ;

	if ( parm1 == "ABENDRTN" )
	{
		pcleanup = pFunc ;
	}
	else
	{
		errBlock.setcall( e1, "PSYE022Y", parm1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	RC = errBlock.getRC() ;
}


void pApplication::log( const string& msgid )
{
	// RC = 0   Normal completion
	// RC = 12  Message not found or message syntax error
	// RC = 20  Severe error

	string t ;

	getmsg( msgid, "ZERRSM", "ZERRLM" ) ;

	if ( RC == 0 )
	{
		vcopy( "ZERRSM", t, MOVE ) ;
		llog( "L", t << endl )     ;
		vcopy( "ZERRLM", t, MOVE ) ;
		llog( "L", t << endl )     ;
	}
}


void pApplication::qtabopen( const string& tb_list )
{
	// RC = 0   Normal completion
	// RC = 12  Variable name prefix too long (max 7 characters)
	// RC = 20  Severe error

	p_tableMGR->qtabopen( errBlock, funcPOOL, tb_list ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "QTABOPEN error" ) ;
		checkRCode( errBlock ) ;
	}
}


void pApplication::tbadd( const string& tb_name,
			  string tb_namelst,
			  const string& tb_order,
			  int tb_num_of_rows )
{
	// Add a new row to a table

	// RC = 0   Normal completion
	// RC = 8   For keyed tables only, row already exists
	// RC = 12  Table not open
	// RC = 20  Severe error

	const string e1 = "TBADD Error" ;

	if ( tb_order != "" && tb_order != "ORDER" )
	{
		errBlock.setcall( e1, "PSYE023A", "TBADD", tb_order ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( tb_num_of_rows < 0 || tb_num_of_rows > 65535 )
	{
		errBlock.setcall( e1, "PSYE023B", "TBADD", d2ds( tb_num_of_rows ) ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	getNameList( errBlock, tb_namelst ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( e1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( !isTableOpen( tb_name, "TBADD" ) ) { return ; }

	p_tableMGR->tbadd( errBlock, funcPOOL, tb_name, tb_namelst, tb_order, tb_num_of_rows ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( e1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::tbbottom( const string& tb_name,
			     const string& tb_savenm,
			     const string& tb_rowid_vn,
			     const string& tb_noread,
			     const string& tb_crp_name )
{
	// Move row pointer to the bottom

	// RC = 0   Normal completion
	// RC = 8   Table is empty.  CRP is set to 0
	// RC = 12  Table not open
	// RC = 20  Severe error

	if ( !isTableOpen( tb_name, "TBBOTTOM" ) ) { return ; }

	p_tableMGR->tbbottom( errBlock, funcPOOL, tb_name, tb_savenm, tb_rowid_vn, tb_noread, tb_crp_name  ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "TBBOTTOM error" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::tbclose( const string& tb_name, const string& tb_newname, string tb_paths )
{
	// Save and close the table (calls saveTable and destroyTable routines).
	// If table opened in NOWRITE mode, just remove table from storage.

	// If path is not specified, use ZTABL as the output path or, if blank, the first path in ZTLIB

	// RC = 0   Normal completion (RC=0 or RC=4 from destroyTable() )
	// RC = 12  Table not open
	// RC = 16  Path error
	// RC = 20  Severe error

	const string e1 = "TBCLOSE Error" ;

	if ( !isTableOpen( tb_name, "TBCLOSE" ) ) { return ; }

	if ( tb_newname != "" && !isvalidName( tb_newname ) )
	{
		errBlock.setcall( e1, "PSYE022J", "table", tb_newname ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( p_tableMGR->writeableTable( errBlock, tb_name ) )
	{
		if ( tb_paths == "" )
		{
			tb_paths = get_search_path( s_ZTABL ) ;
			if ( tb_paths == "" )
			{
				errBlock.setcall( e1, "PSYE013C", 16 ) ;
				checkRCode( errBlock ) ;
				return ;
			}
		}
		p_tableMGR->saveTable( errBlock, tb_name, tb_newname, tb_paths ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e1 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}
	if ( errBlock.RC0() )
	{
		p_tableMGR->destroyTable( errBlock, tb_name ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e1 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		else if ( errBlock.RC0() )
		{
			tablesOpen.erase( tb_name ) ;
		}
		errBlock.setRC( 0 ) ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::tbcreate( const string& tb_name,
			     string tb_keys,
			     string tb_names,
			     tbWRITE tb_WRITE,
			     tbREP tb_REP,
			     string tb_paths,
			     tbDISP tb_DISP )
{
	// Create a new table.
	// tb_paths is an input library to check if the table already exists if opened in WRITE mode.
	// Default to ZTLIB if blank (one must be set for WRITE mode)

	// RC = 0   Normal completion
	// RC = 4   Normal completion - Table exists and REPLACE speified
	// RC = 8   Table exists and REPLACE not specified or REPLACE specified and opend in SHARE mode or
	//          REPLACE specified and opened in EXCLUSIVE mode but not the owning task.
	// RC = 12  Table in use
	// RC = 16  WRITE specified but input library not specified
	// RC = 20  Severe error

	int ws ;
	int i  ;

	string w ;

	const string e1 = "TBCREATE Error" ;

	RC = 0 ;

	if ( !isvalidName( tb_name ) )
	{
		errBlock.setcall( e1, "PSYE022J", "table", tb_name ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( tb_paths == "" )
	{
		tb_paths = get_search_path( s_ZTLIB ) ;
		if ( tb_paths == "" && tb_WRITE == WRITE )
		{
			errBlock.setcall( e1, "PSYE022K", 16 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}

	getNameList( errBlock, tb_keys ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( e1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	for ( ws = words( tb_keys ), i = 1 ; i <= ws ; i++ )
	{
		w = word( tb_keys, i ) ;
		if ( !isvalidName( w ) )
		{
			errBlock.setcall( e1, "PSYE022J", "key", w ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}

	getNameList( errBlock, tb_names ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( e1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	for ( ws = words( tb_names ), i = 1; i <= ws ; i++ )
	{
		w = word( tb_names, i ) ;
		if ( !isvalidName( w ) )
		{
			errBlock.setcall( e1, "PSYE022J", "field", w ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}

	p_tableMGR->createTable( errBlock, tb_name, tb_keys, tb_names, tb_REP, tb_WRITE, tb_paths, tb_DISP ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( e1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	RC = errBlock.getRC() ;
	if ( RC < 8 )
	{
		tablesOpen.insert( tb_name ) ;
	}
}


void pApplication::tbdelete( const string& tb_name )
{
	// Delete a row in the table.  For keyed tables, the table is searched with the current key.  For non-keyed tables the current CRP is used.

	// RC = 0   Normal completion
	// RC = 8   Row does not exist for a keyed table or for non-keyed table, CRP was at TOP(zero)
	// RC = 12  Table not open
	// RC = 20  Severe error

	if ( !isTableOpen( tb_name, "TBDELETE" ) ) { return ; }

	p_tableMGR->tbdelete( errBlock, funcPOOL, tb_name ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "TBDELETE error" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::tbdispl( const string& tb_name,
			    string p_name,
			    const string& p_msg,
			    string p_cursor,
			    int p_csrrow,
			    int p_curpos,
			    string p_autosel,
			    const string& p_crp_name,
			    const string& p_rowid_nm,
			    const string& p_msgloc )
{
	// tbdispl with panel, no message - clear pending lines, rebuild scrollable area and display panel
	// tbdispl with panel, message    - clear pending lines, rebuild scrollable area and display panel and message
	// tbdispl no panel, no message   - retrieve next pending line.  If none, display panel
	// tbdispl no panel, message      - display panel with message.  No rebuilding of the scrollable area

	// Set CRP to first changed line or tbtop if there are no selected lines
	// ln is the tb screen line of the table CRP when invoking )REINIT and )PROC sections (CRP-ZTDTOP)

	// If .AUTOSEL and .CSRROW set in panel, override the parameters p_autosel and p_csrrow
	// Autoselect if the p_curpos CRN is visible

	// Store panel pointer in currtbPanel so that a CONTROL DISPLAY SAVE/RESTORE is only necessary
	// when a TBDISPL issues another TBDISPL and not for a display of an ordinary panel.

	// RC =  0  Normal completion
	// RC =  4  More than 1 row selected
	// RC =  8  End pressed
	// RC = 12  Panel, message or cursor field not found
	// RC = 20  Severe error

	int ztdtop  ;
	int ztdrows ;
	int ztdsels ;
	int exitRC  ;
	int i       ;
	int ln      ;
	int idr     ;

	int    zscrolln ;
	string zscrolla ;

	bool tbscan ;
	bool rebuild = true ;
	bool wpanel  = true ;

	string zzverb ;
	string URID   ;
	string s      ;

	const string e1 = "Error during TBDISPL of panel "+ p_name ;
	const string e2 = "Error processing )INIT section of panel "   ;
	const string e3 = "Error processing )REINIT section of panel " ;
	const string e4 = "Error processing )PROC section of panel "   ;
	const string e5 = "Error during update of panel " ;
	const string e6 = "Error updating field values of panel " ;
	const string e7 = "Error processing )ATTR section of panel " ;

	RC = 0 ;
	ln = 0 ;

	prevPanel = currPanel ;

	if ( propagateEnd )
	{
		if ( !currtbPanel )
		{
			propagateEnd = false ;
		}
		if ( prevPanel && prevPanel->panelid == p_name )
		{
			propagateEnd = false ;
		}
		else
		{
			ControlNonDispl = true ;
		}
	}

	if ( !isTableOpen( tb_name, "TBDISPL" ) ) { return ; }

	if ( p_cursor != "" && !isvalidName( p_cursor ) )
	{
		errBlock.setcall( e1, "PSYE023I", p_cursor ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( p_autosel != "YES" && p_autosel != "NO" && p_autosel != "" )
	{
		errBlock.setcall( e1, "PSYE023J", p_autosel ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( p_crp_name != "" && !isvalidName( p_crp_name ) )
	{
		errBlock.setcall( e1, "PSYE022O", "CRP", p_crp_name ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( p_rowid_nm != "" && !isvalidName( p_rowid_nm ) )
	{
		errBlock.setcall( e1, "PSYE022O", "ROW", p_rowid_nm ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( p_msgloc != "" && !isvalidName( p_msgloc ) )
	{
		errBlock.setcall( e1, "PSYE031L", p_msgloc, "MSGLOC()" ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( p_name != "" )
	{
		createPanel( p_name ) ;
		if ( !errBlock.RC0() )
		{
			errBlock.setcall( e1 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		currPanel   = panelList[ p_name ] ;
		currtbPanel = currPanel ;
		currPanel->tb_clear_linesChanged( errBlock ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e1 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}
	else
	{
		wpanel = false ;
		p_poolMGR->put( errBlock, "ZVERB", "",  SHARED ) ;
		if ( currtbPanel == NULL )
		{
			errBlock.setcall( e1, "PSYE021C" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		currPanel = currtbPanel ;
	}


	currPanel->msgid = p_msg ;
	currPanel->tb_set_autosel( p_autosel == "YES" || p_autosel == "" ) ;
	currPanel->tb_set_csrrow( p_csrrow ) ;
	currPanel->set_msgloc( p_msgloc ) ;
	currPanel->set_cursor( p_cursor, p_curpos ) ;

	if ( addpop_active ) { currPanel->set_popup( addpop_row, addpop_col ) ; }
	else                 { currPanel->remove_popup()                      ; }

	if ( wpanel )
	{
		currPanel->display_panel_init( errBlock ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e2 + p_name ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		currPanel->display_panel_attrs( errBlock ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e7 + p_name ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}
	else
	{
		if ( currPanel->tb_get_lineChanged( errBlock, ln, URID ) )
		{
			tbskip( tb_name, 0, "", p_rowid_nm, URID, "", p_crp_name ) ;
			for ( auto it = currPanel->tb_fields.begin() ; it != currPanel->tb_fields.end() ; it++ )
			{
				s = *it ;
				funcPOOL.put( errBlock, s, funcPOOL.get( errBlock, 0, s+"."+ d2ds( ln ), NOCHECK ) ) ;
				if ( errBlock.error() )
				{
					checkRCode( errBlock ) ;
					return ;
				}
			}
		}
		tbquery( tb_name, "", "", "", "", "", "ZZCRP" ) ;
		i = funcPOOL.get( errBlock, 0, INTEGER, "ZZCRP" ) ;
		if ( i == 0 ) { i = 1 ; }
		ln = i - funcPOOL.get( errBlock, 0, INTEGER, "ZTDTOP" ) ;
		currPanel->display_panel_reinit( errBlock, ln ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e3 + p_name ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		currPanel->tb_remove_lineChanged() ;
		currPanel->tb_get_lineChanged( errBlock, ln, URID ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e1 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		if ( p_msg != "" || URID == "" )
		{
			rebuild = false ;
			p_name  = currPanel->panelid ;
			currPanel->tb_add_autosel_line( errBlock ) ;
		}
	}

	if ( setMessage )
	{
		if ( !ControlNonDispl && p_name != "" )
		{
			currPanel->set_panel_msg( zmsg1, zmsgid1 ) ;
			setMessage = false ;
		}
	}
	else if ( currPanel->msgid != "" )
	{
		get_message( currPanel->msgid ) ;
		if ( RC > 0 ) { RC = 12 ; return ; }
		currPanel->set_panel_msg( zmsg, zmsgid ) ;
	}
	else
	{
		currPanel->clear_msg() ;
	}

	if ( rebuild && p_name != "" )
	{
		tbscan = currPanel->get_tbscan() ;
		p_tableMGR->fillfVARs( errBlock, funcPOOL, tb_name, currPanel->get_tb_clear(), tbscan, currPanel->tb_depth, -1, currPanel->tb_get_csrrow(), idr ) ;
		currPanel->set_cursor_idr( idr ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e1 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}

	currPanel->update_field_values( errBlock ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( e6 + p_name ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	set_screenName() ;

	while ( true )
	{
		if ( p_name != "" )
		{
			if ( lineOutDone )
			{
				refreshlScreen = true  ;
				wait_event() ;
				lineOutDone    = false ;
				refreshlScreen = false ;
			}
			p_poolMGR->put( errBlock, "ZPANELID", p_name, SHARED, SYSTEM ) ;
			tbquery( tb_name, "", "", "", "", "", "ZZCRP" ) ;
			currPanel->tb_set_crp( funcPOOL.get( errBlock, 0, INTEGER, "ZZCRP" ) ) ;
			currPanel->display_panel( errBlock ) ;
			if ( errBlock.error() )
			{
				errBlock.setcall( e1 ) ;
				checkRCode( errBlock ) ;
				return ;
			}
			currPanel->cursor_placement( errBlock ) ;
			if ( errBlock.error() )
			{
				errBlock.setcall( e1, "PSYE022N", currPanel->curfld ) ;
				checkRCode( errBlock ) ;
				return ;
			}
			wait_event() ;
			ControlDisplayLock = false ;
			ControlNonDispl    = false ;
			refreshlScreen     = false ;
			currPanel->hide_popup() ;
			currPanel->clear_msg() ;
			currPanel->curfld = "" ;
			currPanel->resetAttrs_once() ;
			currPanel->display_panel_update( errBlock ) ;
			if ( errBlock.error() )
			{
				errBlock.setcall( e5 ) ;
				checkRCode( errBlock ) ;
				return ;
			}
			currPanel->tb_set_linesChanged( errBlock ) ;
		}

		exitRC = 0  ;
		if ( currPanel->tb_curidx > -1 )
		{
			URID = funcPOOL.get( errBlock, 0, ".ZURID."+d2ds( currPanel->tb_curidx ), NOCHECK ) ;
			if ( errBlock.error() )
			{
				checkRCode( errBlock ) ;
				return ;
			}
			if ( URID != "" )
			{
				tbskip( tb_name, 0, "", "", URID, "NOREAD", "ZCURINX" ) ;
			}
			else
			{
				funcPOOL.put( errBlock, "ZCURINX", 0 ) ;
			}
		}
		else
		{
			funcPOOL.put( errBlock, "ZCURINX", 0 ) ;
		}
		currPanel->tb_set_csrrow( funcPOOL.get( errBlock, 0, INTEGER, "ZCURINX" ) ) ;
		if ( currPanel->tb_get_lineChanged( errBlock, ln, URID ) )
		{
			tbskip( tb_name, 0, "", p_rowid_nm, URID, "", p_crp_name ) ;
			for ( auto it = currPanel->tb_fields.begin() ; it != currPanel->tb_fields.end() ; it++ )
			{
				s = *it ;
				funcPOOL.put( errBlock, s, funcPOOL.get( errBlock, 0, s+"."+ d2ds( ln ), NOCHECK ) ) ;
				if ( errBlock.error() )
				{
					checkRCode( errBlock ) ;
					return ;
				}
			}
			ztdsels = funcPOOL.get( errBlock, 0, INTEGER, "ZTDSELS" ) ;
			if ( ztdsels > 1 ) { exitRC = 4; }
		}

		currPanel->display_panel_proc( errBlock, ln ) ;
		if ( errBlock.error() )
		{
			errBlock.setcall( e4 + p_name ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		zzverb = p_poolMGR->get( errBlock, "ZVERB" ) ;
		if ( !errBlock.RC0() )
		{
			errBlock.setcall( e1, "PSYE015L", "GET", "ZVERB" ) ;
			checkRCode( errBlock ) ;
			return ;
		}
		if ( propagateEnd || findword( zzverb, "END EXIT RETURN" ) )
		{
			if ( zzverb == "RETURN" ) { propagateEnd = true ; }
			RC = 8 ;
			return ;
		}

		if ( currPanel->msgid != "" )
		{
			get_message( currPanel->msgid ) ;
			if ( RC > 0 ) { RC = 12 ; return ; }
			currPanel->set_panel_msg( zmsg, zmsgid ) ;
			if ( p_name == "" )
			{
				p_name = currPanel->panelid ;
				p_poolMGR->put( errBlock, "ZPANELID", p_name, SHARED, SYSTEM ) ;
			}
			currPanel->display_panel_reinit( errBlock, ln ) ;
			if ( errBlock.error() )
			{
				errBlock.setcall( e3 + p_name ) ;
				checkRCode( errBlock ) ;
				return ;
			}
			continue ;
		}
		ztdsels = funcPOOL.get( errBlock, 0, INTEGER, "ZTDSELS" ) ;
		if ( ztdsels == 0 && ( zzverb == "UP" || zzverb == "DOWN" ) )
		{
			zscrolla = p_poolMGR->get( errBlock, "ZSCROLLA", SHARED ) ;
			if ( zzverb == "UP" )
			{
				if ( zscrolla == "MAX" )
				{
					ztdtop = 1 ;
				}
				else
				{
					ztdtop   = funcPOOL.get( errBlock, 0, INTEGER, "ZTDTOP" ) ;
					zscrolln = ds2d( p_poolMGR->get( errBlock, "ZSCROLLN", SHARED ) ) ;
					ztdtop   = ( ztdtop > zscrolln ) ? ( ztdtop - zscrolln ) : 1 ;
				}
			}
			else
			{
				if ( zscrolla == "MAX" )
				{
					ztdtop = funcPOOL.get( errBlock, 0, INTEGER, "ZTDROWS" ) + 1 ;
				}
				else
				{
					ztdtop   = funcPOOL.get( errBlock, 0, INTEGER, "ZTDTOP"  ) ;
					ztdrows  = funcPOOL.get( errBlock, 0, INTEGER, "ZTDROWS" ) ;
					zscrolln = ds2d( p_poolMGR->get( errBlock, "ZSCROLLN", SHARED ) ) ;
					ztdtop   = ( zscrolln + ztdtop > ztdrows ) ? ( ztdrows + 1 ) : ztdtop + zscrolln ;
				}
			}
			p_tableMGR->fillfVARs( errBlock, funcPOOL, tb_name, currPanel->get_tb_clear(), tbscan, currPanel->tb_depth, ztdtop, 0, idr ) ;
			if ( errBlock.error() )
			{
				errBlock.setcall( e6 + p_name ) ;
				checkRCode( errBlock ) ;
				return ;
			}
			currPanel->update_field_values( errBlock ) ;
			if ( errBlock.error() )
			{
				errBlock.setcall( e6 + p_name ) ;
				checkRCode( errBlock ) ;
				return ;
			}
			currPanel->set_cursor_home() ;
			continue ;
		}
		break ;
	}

	if ( funcPOOL.get( errBlock, 0, INTEGER, "ZTDSELS" ) == 0 )
	{
		tbtop( tb_name ) ;
		if ( p_crp_name != "" )
		{
			funcPOOL.put( errBlock, p_crp_name, 0 ) ;
			if ( errBlock.error() )
			{
				errBlock.setcall( e1 ) ;
				checkRCode( errBlock ) ;
				return ;
			}
		}
		if ( p_rowid_nm != "" )
		{
			funcPOOL.put( errBlock, p_rowid_nm, "" ) ;
			if ( errBlock.error() )
			{
				errBlock.setcall( e1 ) ;
				checkRCode( errBlock ) ;
				return ;
			}
		}
	}

	RC = exitRC ;
}


void pApplication::tbend( const string& tb_name )
{
	// Close a table without saving (calls destroyTable routine).
	// If opened share, use count is reduced and table deleted when use count = 0.

	// RC = 0   Normal completion (RC=0 or RC=4 from destroyTable() )
	// RC = 12  Table not open
	// RC = 20  Severe error

	if ( !isTableOpen( tb_name, "TBEND" ) ) { return ; }

	p_tableMGR->destroyTable( errBlock, tb_name ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "TBEND error" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	else if ( errBlock.RC0() )
	{
		tablesOpen.erase( tb_name ) ;
	}
	RC = 0 ;
}


void pApplication::tberase( const string& tb_name, string tb_paths )
{
	// Erase a table file from the table output library

	// If tb_paths is not specified, use ZTABL as the output library.  Error if blank.

	// RC = 0   Normal completion
	// RC = 8   Table does not exist
	// RC = 12  Table in use
	// RC = 16  Path does not exist
	// RC = 20  Severe error

	const string e1 = "TBERASE Error" ;

	if ( !isvalidName( tb_name ) )
	{
		errBlock.setcall( e1, "PSYE022J", "table", tb_name ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( tb_paths == "" )
	{
		tb_paths = get_search_path( s_ZTABL ) ;
		if ( tb_paths == "" )
		{
			errBlock.setcall( e1, "PSYE013C", 16 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}

	p_tableMGR->tberase( errBlock, tb_name, tb_paths ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( e1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::tbexist( const string& tb_name )
{
	// Test for the existance of a row in a keyed table

	// RC = 0   Normal completion
	// RC = 8   Row does not exist or not a keyed table
	// RC = 12  Table not open
	// RC = 20  Severe error

	if ( !isTableOpen( tb_name, "TBEXIST" ) ) { return ; }

	p_tableMGR->tbexist( errBlock, funcPOOL, tb_name ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "TBEXISTS error" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}



void pApplication::tbget( const string& tb_name,
			  const string& tb_savenm,
			  const string& tb_rowid_vn,
			  const string& tb_noread,
			  const string& tb_crp_name )
{
	if ( !isTableOpen( tb_name, "TBGET" ) ) { return ; }

	p_tableMGR->tbget( errBlock, funcPOOL, tb_name, tb_savenm, tb_rowid_vn, tb_noread, tb_crp_name  ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "TBGET error" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::tbmod( const string& tb_name,
			  string tb_namelst,
			  const string& tb_order )
{
	// Update/add a row in a table
	// Tables with keys        : Same as tbadd if row not found
	// Tables with without keys: Same as tbadd

	// RC = 0   Okay.  Keyed tables - row updated.  Non-keyed tables new row added
	// RC = 8   Row did not match - row added for keyed tables
	// RC = 16  Numeric conversion error
	// RC = 20  Severe error

	const string e1 = "TBMOD Error" ;

	if ( tb_order != "" && tb_order != "ORDER" )
	{
		errBlock.setcall( e1, "PSYE023A", "TBMOD", tb_order ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( !isTableOpen( tb_name, "TBMOD" ) ) { return ; }

	getNameList( errBlock, tb_namelst ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( e1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	p_tableMGR->tbmod( errBlock, funcPOOL, tb_name, tb_namelst, tb_order ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( e1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::tbopen( const string& tb_name,
			   tbWRITE tb_WRITE,
			   string tb_paths,
			   tbDISP tb_DISP )
{
	// Open an existing table, reading it from a file.
	// If aleady opened in SHARE mode, increment use count

	// RC = 0   Normal completion
	// RC = 8   Table does not exist in search path
	// RC = 12  Table already open by this or another task
	// RC = 16  paths not allocated
	// RC = 20  Severe error

	const string e1 = "TBOPEN Error" ;

	if ( !isvalidName( tb_name ) )
	{
		errBlock.setcall( e1, "PSYE022J", "table", tb_name ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( tb_paths == "" )
	{
		tb_paths = get_search_path( s_ZTLIB ) ;
		if ( tb_paths == "" )
		{
			errBlock.setcall( e1, "PSYE013D", 16 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}

	p_tableMGR->loadTable( errBlock, tb_name, tb_WRITE, tb_paths, tb_DISP, true ) ;

	if ( errBlock.error() )
	{
		errBlock.setcall( e1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( errBlock.RC0() )
	{
		tablesOpen.insert( tb_name ) ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::tbput( const string& tb_name, string tb_namelst, const string& tb_order )
{
	// Update the current row in a table
	// Tables with keys        : Keys must match CRP row
	// Tables with without keys: CRP row updated

	// RC = 0   Normal completion
	// RC = 8   Keyed tables - key does not match current row.  CRP set to top (0)
	//          Non-keyed tables - CRP at top
	// RC = 12  Table not open
	// RC = 16  Numeric conversion error for sorted tables
	// RC = 20  Severe error

	const string e1 = "TBPUT Error" ;

	RC = 0 ;

	if ( tb_order != "" && tb_order != "ORDER" )
	{
		errBlock.setcall( e1, "PSYE023A", "TBPUT", tb_order ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( !isTableOpen( tb_name, "TBPUT" ) ) { return ; }

	getNameList( errBlock, tb_namelst ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( e1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	p_tableMGR->tbput( errBlock, funcPOOL, tb_name, tb_namelst, tb_order ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( e1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
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
	if ( !isTableOpen( tb_name, "TBQUERY" ) ) { return ; }

	p_tableMGR->tbquery( errBlock, funcPOOL, tb_name, tb_keyn, tb_varn, tb_rownn, tb_keynn, tb_namenn, tb_crpn, tb_sirn, tb_lstn, tb_condn, tb_dirn ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "TBQUERY error" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::tbsarg( const string& tb_name,
			   const string& tb_namelst,
			   const string& tb_dir,
			   const string& tb_cond_pairs )
{
	if ( !isTableOpen( tb_name, "TBSARG" ) ) { return ; }

	p_tableMGR->tbsarg( errBlock, funcPOOL, tb_name, tb_namelst, tb_dir, tb_cond_pairs ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "TBSARG error" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::tbsave( const string& tb_name, const string& tb_newname, string tb_paths )
{
	// Save the table to disk (calls saveTable routine).  Table remains open for processing.
	// Table must be open in WRITE mode.

	// If tb_paths is not specified, use ZTABL as the output path.  Error if blank.

	// RC = 0   Normal completion
	// RC = 12  Table not open or not open WRITE
	// RC = 16  Alternate name save error
	// RC = 20  Severe error

	RC = 0 ;

	const string e1 = "TBSAVE Error" ;

	if ( !isTableOpen( tb_name, "TBSAVE" ) ) { return ; }

	if ( tb_newname != "" && !isvalidName( tb_newname ) )
	{
		errBlock.setcall( e1, "PSYE022J", "table", tb_newname ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( !p_tableMGR->writeableTable( errBlock, tb_name ) )
	{
		errBlock.setcall( e1 ) ;
		if ( !errBlock.error() )
		{
			errBlock.seterrid( "PSYE014S", tb_name, 12 ) ;
		}
		checkRCode( errBlock ) ;
		return ;
	}

	if ( tb_paths == "" )
	{
		tb_paths = get_search_path( s_ZTABL ) ;
		if ( tb_paths == "" )
		{
			errBlock.setcall( e1, "PSYE013C", 16 ) ;
			checkRCode( errBlock ) ;
			return ;
		}
	}

	p_tableMGR->saveTable( errBlock, tb_name, tb_newname, tb_paths ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( e1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::tbscan( const string& tb_name,
			   const string& tb_namelst,
			   const string& tb_savenm,
			   const string& tb_rowid_vn,
			   const string& tb_dir,
			   const string& tb_read,
			   const string& tb_crp_name,
			   const string& tb_condlst )
{
	if ( !isTableOpen( tb_name, "TBSCAN" ) ) { return ; }

	p_tableMGR->tbscan( errBlock, funcPOOL, tb_name, tb_namelst, tb_savenm, tb_rowid_vn, tb_dir, tb_read, tb_crp_name, tb_condlst ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "TBSCAN error" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}



void pApplication::tbskip( const string& tb_name,
			   int num,
			   const string& tb_savenm,
			   const string& tb_rowid_vn,
			   const string& tb_rowid,
			   const string& tb_noread,
			   const string& tb_crp_name )
{
	if ( !isTableOpen( tb_name, "TBSKIP" ) ) { return ; }

	p_tableMGR->tbskip( errBlock, funcPOOL, tb_name, num, tb_savenm, tb_rowid_vn, tb_rowid, tb_noread, tb_crp_name ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "TBSKIP error" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::tbsort( const string& tb_name, string tb_fields )
{
	if ( !isTableOpen( tb_name, "TBSORT" ) ) { return ; }

	getNameList( errBlock, tb_fields ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "TBSORT error" ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	p_tableMGR->tbsort( errBlock, tb_name, tb_fields ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "TBSORT error" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}



void pApplication::tbtop( const string& tb_name )
{
	if ( !isTableOpen( tb_name, "TBTOP" ) ) { return ; }

	p_tableMGR->tbtop( errBlock, tb_name ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "TBTOP error" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::tbvclear( const string& tb_name )
{
	if ( !isTableOpen( tb_name, "TBVCLEAR" ) ) { return ; }

	p_tableMGR->tbvclear( errBlock, funcPOOL, tb_name ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "TBVCLEAR error" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


bool pApplication::isTableOpen( const string& tb_name, const string& func )
{
	RC = 0 ;
	errBlock.setRC( 0 ) ;

	if ( tb_name == "" )
	{
		errBlock.seterrid( "PSYE013H", func ) ;
	}
	else if ( !isvalidName( tb_name ) )
	{
		errBlock.seterrid( "PSYE014Q", tb_name ) ;
	}
	else if ( tablesOpen.count( tb_name ) == 0 )
	{
		errBlock.seterrid( "PSYE013G", func, tb_name, 12 ) ;
	}
	if ( errBlock.error() )
	{
		errBlock.setcall( func + " error" ) ;
		checkRCode( errBlock ) ;
		return false ;
	}
	return true ;
}


void pApplication::edit( const string& m_file,
			 const string& m_panel,
			 const string& m_macro,
			 const string& m_profile,
			 const string& m_lcmds )
{
	selct.clear() ;
	selct.pgm     = p_poolMGR->get( errBlock, "ZEDITPGM", PROFILE ) ;
	selct.parm    = "FILE("+ m_file +") "+
			"PANEL("+ m_panel +") "+
			"MACRO("+ m_macro +") "+
			"PROFILE("+ m_profile+ ") "+
			"LINECMDS("+ m_lcmds+ ")" ;
	selct.newappl = ""      ;
	selct.newpool = false   ;
	selct.passlib = false   ;
	selct.suspend = true    ;
	selct.scrname = "EDIT"  ;
	actionSelect() ;
}


void pApplication::browse( const string& m_file, const string& m_panel )
{
	selct.clear() ;
	selct.pgm     = p_poolMGR->get( errBlock, "ZBRPGM", PROFILE ) ;
	selct.parm    = "FILE("+ m_file +") PANEL("+ m_panel +")" ;
	selct.newappl = ""       ;
	selct.newpool = false    ;
	selct.passlib = false    ;
	selct.suspend = true     ;
	selct.scrname = "BROWSE" ;
	actionSelect() ;
}


void pApplication::view( const string& m_file, const string& m_panel )
{
	selct.clear() ;
	selct.pgm     = p_poolMGR->get( errBlock, "ZVIEWPGM", PROFILE ) ;
	selct.parm    = "FILE("+ m_file +") PANEL("+ m_panel +")" ;
	selct.newappl = ""      ;
	selct.newpool = false   ;
	selct.passlib = false   ;
	selct.suspend = true    ;
	selct.scrname = "VIEW"  ;
	actionSelect() ;
}


void pApplication::select( const string& cmd )
{
	// SELECT a function or panel in keyword format for use in applications,
	// ie PGM(abc) CMD(oorexx) PANEL(xxx) PARM(zzz) NEWAPPL PASSLIB SCRNAME(abc) etc.

	// No variable substitution is done at this level.

	if ( !selct.parse( errBlock, cmd ) )
	{
		errBlock.setcall( "Error in SELECT command "+cmd ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	actionSelect() ;
}


void pApplication::select( const selobj& sel )
{
	// SELECT a function or panel using a SELECT object (internal use only)

	selct = sel    ;
	actionSelect() ;
}


void pApplication::actionSelect()
{
	// RC =  0  Normal completion of the selection panel or function.  END was entered.
	// RC =  4  Normal completion.  RETURN was entered or EXIT specified on the selection panel
	// RC = 20  Abnormal termination of the called task.

	// If the application has abended, propogate back (only set if not controlErrorsReturn).

	// If abnormal termination in the selected task:
	// ZRC = 20  RSN = 999  Application program abended.
	// ZRC = 20  RSN = 998  SELECT PGM not found.
	// ZRC = 20  RSN = 997  SELECT CMD not found.
	// (don't percolate these codes back to the calling program - ZRSN = 0)

	RC  = 0    ;
	SEL = true ;

	wait_event() ;

	if ( RC == 4 )
	{
		propagateEnd = true ;
		currPanel    = NULL ;
	}

	SEL = false ;

	if ( abnormalEnd )
	{
		abnormalNoMsg = true ;
		if ( ZRC == 20 && ZRESULT == "Not Found" )
		{
			if ( ZRSN == 997 )
			{
				errBlock.setcall( "Error in SELECT command", "PSYS012X", word( selct.parm, 1 ) ) ;
			}
			else if ( ZRSN == 998 )
			{
				errBlock.setcall( "Error in SELECT command", "PSYS012W", selct.pgm ) ;
			}
			ZRSN = 0 ;
			checkRCode( errBlock ) ;
		}
		llog( "E", "Percolating abend to calling application.  Taskid: "<< taskId <<endl ) ;
		abend() ;
	}
	selct.clear() ;
}


void pApplication::pquery( const string& p_name,
			   const string& a_name,
			   const string& t_name,
			   const string& w_name,
			   const string& d_name,
			   const string& r_name,
			   const string& c_name )
{
	const string e1 = "PQUERY Error for panel "+p_name ;

	RC = 0 ;

	if ( !isvalidName( p_name ) )
	{
		errBlock.setcall( e1, "PSYE021A", p_name ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( a_name != "" && !isvalidName( a_name ) )
	{
		errBlock.setcall( e1, "PSYE023C", "area", a_name ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( t_name != "" && !isvalidName( t_name ) )
	{
		errBlock.setcall( e1, "PSYE023C", "area type", t_name ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( w_name != "" && !isvalidName( w_name ) )
	{
		errBlock.setcall( e1, "PSYE023C", "area width", w_name ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( d_name != "" && !isvalidName( d_name ) )
	{
		errBlock.setcall( e1, "PSYE023C", "area depth", d_name ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( r_name != "" && !isvalidName( r_name ) )
	{
		errBlock.setcall( e1, "PSYE023C", "area row number", r_name ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( c_name != "" && !isvalidName( c_name ) )
	{
		errBlock.setcall( e1, "PSYE023C", "area column number", c_name ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	createPanel( p_name ) ;
	if ( !errBlock.RC0() )
	{
		errBlock.setcall( e1 ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	panelList[ p_name ]->get_panel_info( RC, a_name, t_name, w_name, d_name, r_name, c_name ) ;
}


void pApplication::attr( const string& field, const string& attrs )
{
	if ( !currPanel )
	{
		errBlock.setcall( "ATTR change error", "PSYE023D" ) ;
		checkRCode( errBlock ) ;
		return  ;
	}
	currPanel->attr( errBlock, field, attrs ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "ATTR change error" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	RC = errBlock.getRC() ;
}


void pApplication::reload_keylist( pPanel* p )
{
	// Does an unconditional reload every time, but need to find a way to detect a change (TODO)
	// Alternatively, don't preload pfkeys into the panel object but pass back requested key from pApplication.

	load_keylist( p ) ;
}

void pApplication::load_keylist( pPanel* p  )
{
	bool klfail ;

	string tabName  ;
	string tabField ;

	string uprof ;


	if ( p->keylistn == "" || p_poolMGR->get( errBlock, "ZKLUSE", PROFILE ) != "Y" )
	{
		return ;
	}

	tabName = p->keyappl + "KEYP" ;
	klfail  = ( p_poolMGR->get( errBlock, "ZKLFAIL", PROFILE ) == "Y" ) ;

	vcopy( "ZUPROF", uprof, MOVE ) ;
	tbopen( tabName, NOWRITE, uprof, SHARE ) ;
	if ( RC > 0 )
	{
		if ( !klfail )
		{
			RC = 0 ;
			llog( "W", "Open of keylist table '"+ tabName +"' failed" << endl ) ;
			return ;
		}
		errBlock.setcall( "KEYLIST error", "PSYE023E", tabName ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	tbvclear( tabName ) ;
	vreplace( "KEYLISTN", p->keylistn ) ;
	tbget( tabName ) ;
	if ( RC > 0 )
	{
		tbend( tabName ) ;
		if ( !klfail )
		{
			RC = 0 ;
			llog( "W", "Keylist '"+ p->keylistn +"' not found in keylist table "+ tabName << endl ) ;
			return ;
		}
		errBlock.setcall( "KEYLIST error", "PSYE023F", p->keylistn, tabName ) ;
		checkRCode( errBlock ) ;
		return  ;
	}

	vcopy( "KEY1DEF",  tabField, MOVE ) ; p->put_keylist( KEY_F(1),  tabField ) ;
	vcopy( "KEY2DEF",  tabField, MOVE ) ; p->put_keylist( KEY_F(2),  tabField ) ;
	vcopy( "KEY3DEF",  tabField, MOVE ) ; p->put_keylist( KEY_F(3),  tabField ) ;
	vcopy( "KEY4DEF",  tabField, MOVE ) ; p->put_keylist( KEY_F(4),  tabField ) ;
	vcopy( "KEY5DEF",  tabField, MOVE ) ; p->put_keylist( KEY_F(5),  tabField ) ;
	vcopy( "KEY6DEF",  tabField, MOVE ) ; p->put_keylist( KEY_F(6),  tabField ) ;
	vcopy( "KEY7DEF",  tabField, MOVE ) ; p->put_keylist( KEY_F(7),  tabField ) ;
	vcopy( "KEY8DEF",  tabField, MOVE ) ; p->put_keylist( KEY_F(8),  tabField ) ;
	vcopy( "KEY9DEF",  tabField, MOVE ) ; p->put_keylist( KEY_F(9),  tabField ) ;
	vcopy( "KEY10DEF", tabField, MOVE ) ; p->put_keylist( KEY_F(10), tabField ) ;
	vcopy( "KEY11DEF", tabField, MOVE ) ; p->put_keylist( KEY_F(11), tabField ) ;
	vcopy( "KEY12DEF", tabField, MOVE ) ; p->put_keylist( KEY_F(12), tabField ) ;
	vcopy( "KEY13DEF", tabField, MOVE ) ; p->put_keylist( KEY_F(13), tabField ) ;
	vcopy( "KEY14DEF", tabField, MOVE ) ; p->put_keylist( KEY_F(14), tabField ) ;
	vcopy( "KEY15DEF", tabField, MOVE ) ; p->put_keylist( KEY_F(15), tabField ) ;
	vcopy( "KEY16DEF", tabField, MOVE ) ; p->put_keylist( KEY_F(16), tabField ) ;
	vcopy( "KEY17DEF", tabField, MOVE ) ; p->put_keylist( KEY_F(17), tabField ) ;
	vcopy( "KEY18DEF", tabField, MOVE ) ; p->put_keylist( KEY_F(18), tabField ) ;
	vcopy( "KEY19DEF", tabField, MOVE ) ; p->put_keylist( KEY_F(19), tabField ) ;
	vcopy( "KEY20DEF", tabField, MOVE ) ; p->put_keylist( KEY_F(20), tabField ) ;
	vcopy( "KEY21DEF", tabField, MOVE ) ; p->put_keylist( KEY_F(21), tabField ) ;
	vcopy( "KEY22DEF", tabField, MOVE ) ; p->put_keylist( KEY_F(22), tabField ) ;
	vcopy( "KEY23DEF", tabField, MOVE ) ; p->put_keylist( KEY_F(23), tabField ) ;
	vcopy( "KEY24DEF", tabField, MOVE ) ; p->put_keylist( KEY_F(24), tabField ) ;
	vcopy( "KEYHELPN", p->keyhelpn, MOVE ) ;

	tbend( tabName ) ;
}


void pApplication::rdisplay( const string& msg, bool subVars )
{
	// Display line mode output on the screen.  Cancel any screen refreshes as this will be done
	// as part of returning to full screen mode.

	RC = 0 ;

	lineBuffer = subVars ? sub_vars( msg ) : msg ;

	lineOutDone    = true  ;
	lineOutPending = true  ;
	refreshlScreen = false ;
	wait_event() ;
	lineOutPending = false ;
}


void pApplication::setmsg( const string& msg, msgSET sType )
{
	// Retrieve message and store in zmsg1 and zmsgid1.

	RC = 0 ;

	if ( ( sType == COND ) && setMessage ) { return ; }

	get_message( msg ) ;
	if ( RC > 0 )
	{
		RC = 20 ;
		return  ;
	}

	zmsg1      = zmsg  ;
	zmsgid1    = msg   ;
	setMessage = true  ;
}


void pApplication::getmsg( const string& msg,
			   const string& smsg,
			   const string& lmsg,
			   const string& alm,
			   const string& hlp,
			   const string& typ,
			   const string& wndo )
{
	// Load message msg and substitute variables

	// RC = 0   Normal completion
	// RC = 12  Message not found or message syntax error
	// RC = 20  Severe error

	const string e1 = "GETMSG Error" ;

	slmsg tmsg ;

	if ( smsg != "" && !isvalidName( smsg ) )
	{
		errBlock.setcall( e1, "PSYE022O", "SHORT MESSAGE", smsg ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( lmsg != "" && !isvalidName( lmsg ) )
	{
		errBlock.setcall( e1, "PSYE022O", "LONG MESSAGE", lmsg ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( alm != "" && !isvalidName( alm ) )
	{
		errBlock.setcall( e1, "PSYE022O", "ALARM", alm ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( hlp != "" && !isvalidName( hlp ) )
	{
		errBlock.setcall( e1, "PSYE022O", "HELP", hlp ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( typ != "" && !isvalidName( typ ) )
	{
		errBlock.setcall( e1, "PSYE022O", "TYPE", typ ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	if ( wndo != "" && !isvalidName( wndo ) )
	{
		errBlock.setcall( e1, "PSYE022O", "WINDOW", wndo ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( !load_message( msg ) ) { return ; }

	tmsg      = msgList[ msg ] ;
	tmsg.smsg = sub_vars( tmsg.smsg ) ;
	tmsg.lmsg = sub_vars( tmsg.lmsg ) ;

	if ( !sub_message_vars( tmsg ) )
	{
		errBlock.seterror( "Invalid variable value" ) ;
		checkRCode( errBlock ) ;
		return ;
	}

	if ( smsg != "" )
	{
		funcPOOL.put( errBlock, smsg, tmsg.smsg ) ;
		if ( errBlock.error() )
		{
			errBlock.seterror( e1 ) ;
			checkRCode( errBlock )  ;
			return ;
		}
	}
	if ( lmsg != "" )
	{
		funcPOOL.put( errBlock, lmsg, tmsg.lmsg ) ;
		if ( errBlock.error() )
		{
			errBlock.seterror( e1 ) ;
			checkRCode( errBlock )  ;
			return ;
		}
	}
	if ( alm != "" )
	{
		funcPOOL.put( errBlock, alm, tmsg.alm ? "YES" : "NO" ) ;
		if ( errBlock.error() )
		{
			errBlock.seterror( e1 ) ;
			checkRCode( errBlock )  ;
			return ;
		}
	}
	if ( typ != "" )
	{
		switch ( tmsg.type )
		{
		case IMT: funcPOOL.put( errBlock, typ, "NOTIFY" )   ;
				break ;

		case WMT: funcPOOL.put( errBlock, typ, "WARNING" )  ;
				break ;

		case AMT: funcPOOL.put( errBlock, typ, "CRITICAL" ) ;
		}
		if ( errBlock.error() )
		{
			errBlock.seterror( e1 ) ;
			checkRCode( errBlock )  ;
			return ;
		}
	}
	if ( hlp != "" )
	{
		funcPOOL.put( errBlock, hlp, tmsg.hlp ) ;
		if ( errBlock.error() )
		{
			errBlock.seterror( e1 ) ;
			checkRCode( errBlock )  ;
			return ;
		}
	}
	if ( wndo != "" )
	{
		funcPOOL.put( errBlock, wndo, tmsg.resp ? "RESP" : "NORESP" ) ;
		if ( errBlock.error() )
		{
			errBlock.seterror( e1 ) ;
			checkRCode( errBlock )  ;
			return ;
		}
	}
	RC = errBlock.getRC() ;
}


string pApplication::get_help_member( int row, int col )
{
	RC = 0 ;

	return "M("+ zmsg.hlp+ ") " +
	       "F("+ currPanel->get_field_help( row, col )+ ") " +
	       "P("+ currPanel->zphelp +") " +
	       "A("+ zapphelp +") " +
	       "K("+ currPanel->keyhelpn +") "+
	       "PATHS("+ get_search_path( s_ZPLIB ) +")" ;
}


void pApplication::get_message( const string& p_msg )
{
	// Load messages from message library and copy slmsg object to zmsg for the message requested

	// Substitute any dialogue variables in the short and long messages
	// Substitute any dialogue varibles in .TYPE, .WINDOW, .HELP and .ALARM parameters
	// Set zmsgid

	RC = 0 ;

	if ( !load_message( p_msg ) ) { return ; }

	zmsg      = msgList[ p_msg ]      ;
	zmsg.smsg = sub_vars( zmsg.smsg ) ;
	zmsg.lmsg = sub_vars( zmsg.lmsg ) ;

	if ( !sub_message_vars( zmsg ) ) { RC = 20 ; return ; }

	zmsgid = p_msg ;
}


bool pApplication::load_message( const string& p_msg )
{
	// Message format: 1-5 alph char prefix
	//                 3 numeric chars
	//                 1 alph char suffix (optional and only if prefix is less than 5)

	// Read messages and store in msgList map (no variable substitution done at this point)
	// Return false if message not found in member or there is an error (but still store individual messages from member)
	// Error on duplicate message-id in the file member

	// The message file name is determined by truncating the message ID after the second digit of the number.
	// AB123A file AB12
	// G012 file G01

	// If in test mode, reload message each time it is requested
	// Routine sets the return code:
	// RC =  0  Normal completion
	// RC = 12  Message not found or syntax error
	// RC = 20  Severe error

	int i  ;
	int j  ;

	char c ;

	string p_msg_fn ;
	string filename ;
	string mline    ;
	string paths    ;
	string tmp      ;
	string msgid    ;
	string smsg     ;
	string lmsg     ;

	bool lcontinue = false ;

	slmsg t ;

	RC = 0  ;

	if ( testMode ) { msgList.clear() ; }
	else if ( msgList.count( p_msg ) > 0 ) { return true ; }

	i = check_message_id( p_msg ) ;

	if ( i == 0 || ( p_msg.size() - i > 3 && !isalpha( p_msg.back() ) ) )
	{
		RC = 12 ;
		zerr1 = "Message-id format invalid (1): "+ p_msg ;
		checkRCode() ;
		return false ;
	}

	p_msg_fn = p_msg.substr( 0, i+2 ) ;

	paths = get_search_path( s_ZMLIB ) ;

	for ( i = getpaths( paths ), j = 1 ; j <= i ; j++ )
	{
		filename = getpath( paths, j ) + p_msg_fn ;
		try
		{
			if ( exists( filename ) )
			{
				if ( !is_regular_file( filename ) )
				{
					RC = 20 ;
					zerr1 = "Message file "+ filename +" is not a regular file" ;
					checkRCode() ;
					return false ;
				}
				break ;
			}
		}
		catch ( boost::filesystem::filesystem_error &e )
		{
			RC = 20 ;
			zerr1 = "I/O error accessing entry" ;
			zerr2 = e.what() ;
			checkRCode() ;
			return false ;
		}
		catch (...)
		{
			RC = 20 ;
			zerr1 = "I/O error accessing entry" ;
			zerr2 = filename ;
			checkRCode() ;
			return false ;
		}
	}
	if ( j > i )
	{
		RC = 12 ;
		zerr1 = "Message file "+ p_msg_fn +" not found in ZMLIB for message-id "+ p_msg ;
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
		checkRCode() ;
		return false ;
	}

	while ( getline( messages, mline ) )
	{
		trim( mline ) ;
		if ( mline == "" || mline.front() == '*' ) { continue ; }
		if ( mline.compare( 0, 2, "/*" ) == 0 )    { continue ; }
		if ( mline.compare( 0, p_msg_fn.size(), p_msg_fn ) == 0 )
		{
			if ( msgid != "" )
			{
				if ( lcontinue || !t.parse( smsg, lmsg ) )
				{
					RC = 12 ;
					messages.close() ;
					zerr1 = "Error (1) in message-id "+ msgid ;
					checkRCode() ;
					return false ;
				}
				if ( msgList.count( msgid ) > 0 )
				{
					RC = 20 ;
					messages.close() ;
					zerr1 = "Duplicate message-id found: "+ msgid ;
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
				RC = 12 ;
				zerr1 = "Message-id format invalid (2): "+ msgid ;
				checkRCode() ;
				return false ;
			}
		}
		else
		{
			if ( msgid == "" || ( lmsg != "" && !lcontinue ) )
			{
				RC = 12 ;
				messages.close() ;
				zerr1 = "Extraeneous data: "+ mline ;
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
					RC = 12 ;
					messages.close() ;
					zerr1 = "Error (2) in message-id "+ msgid ;
					checkRCode() ;
					return false ;
				}
				tmp = mline.substr( 1, mline.size() - 2 ) ;
				dquote( c, tmp ) ;
			}
			else
			{
				if ( words( mline ) > 1 )
				{
					RC = 12 ;
					messages.close() ;
					zerr1 = "Error (3) in message-id "+ msgid ;
					checkRCode() ;
					return false ;
				}
				tmp = mline ;
			}
			lmsg = lmsg + tmp ;
			if ( lmsg.size() > 512 )
			{
				RC = 12 ;
				messages.close() ;
				zerr1 = "Long message size exceeds 512 bytes for message-id "+ msgid ;
				checkRCode() ;
				return false ;
			}
		}
	}
	if ( messages.bad() )
	{
		RC = 20 ;
		messages.close() ;
		zerr1 = "Error while reading message file "+ filename ;
		checkRCode() ;
		return false ;
	}
	messages.close() ;

	if ( smsg != "" )
	{
		if ( lcontinue || !t.parse( smsg, lmsg ) )
		{
			RC = 12 ;
			messages.close() ;
			zerr1 = "Error (4) in message-id "+ msgid ;
			checkRCode() ;
			return false ;
		}
		if ( msgList.count( msgid ) > 0 )
		{
			RC = 20 ;
			messages.close() ;
			zerr1 = "Duplicate message-id found: "+ msgid ;
			checkRCode() ;
			return false ;
		}
		msgList[ msgid ] = t ;
	}

	if ( msgList.count( p_msg ) == 0 )
	{
		RC = 12 ;
		zerr1 = "Message-id "+ p_msg +" not found in message file "+ p_msg_fn ;
		checkRCode() ;
		return false ;
	}
	return true ;
}


bool pApplication::sub_message_vars( slmsg& t )
{
	// Get the dialogue variable value specified in message .T, .A, .H and .W options

	// Error if the dialgue variable is blank
	// This routine does not set the return code.  Set in the calling routine.

	// Use defaults for invalid values

	// .TYPE overrides .WINDOW and .ALARM

	string val ;

	if ( t.dvwin != "" )
	{
		t.lmwin = true ;
		vcopy( t.dvwin, val, MOVE ) ;
		trim( val ) ;
		if      ( val == "RESP"    || val == "R"  ) { t.smwin = true  ; t.resp = true ; }
		else if ( val == "NORESP"  || val == "N"  ) { t.smwin = true  ; }
		else if ( val == "LRESP"   || val == "LR" ) { t.resp  = true  ; }
		else if ( val == "LNORESP" || val == "LN" ) {                   }
		else if ( val == "" ) { return false ; }
	}
	if ( t.dvalm != "" )
	{
		vcopy( t.dvalm, val, MOVE ) ;
		trim( val ) ;
		if      ( val == "YES" ) { t.alm = true  ; }
		else if ( val == "NO"  ) { t.alm = false ; }
		else if ( val == ""    ) { return false  ; }
	}
	if ( t.dvtype != "" )
	{
		vcopy( t.dvtype, val, MOVE ) ;
		trim( val ) ;
		if      ( val == "N" ) { t.type = IMT ; t.alm = false ; }
		else if ( val == "W" ) { t.type = WMT ; t.alm = true  ; }
		else if ( val == "A" ) { t.type = AMT ; t.alm = true  ; }
		else if ( val == "C" ) { t.type = AMT ; t.alm = true  ; t.resp = true ; t.smwin = true ; t.lmwin = true ; }
		else if ( val == ""  ) { return false ; }
	}
	if ( t.dvhlp != "" )
	{
		vcopy( t.dvhlp, t.hlp, MOVE ) ;
	}
	return true ;
}


int pApplication::check_message_id( const string& msgid )
{
	// Return 0 if message-id format is incorrect, else the offset to the first numeric triplet

	int i ;
	int l ;

	l = msgid.size() ;

	if ( l < 4 || !isvalidName( msgid ) )
	{
		return 0 ;
	}

	l = l - 2 ;
	for ( i = 1 ; i < l ; i++ )
	{
		if ( isdigit( msgid[ i ] ) && isdigit( msgid[ i + 1 ] ) && isdigit( msgid[ i + 2 ] ) )
		{
			return i ;
		}
	}
	return 0 ;
}


string pApplication::sub_vars( string s )
{
	// In string, s, substitute variables starting with '&' for their dialogue value
	// The variable is delimited by any non-valid variable characters.
	// Rules: A '.' at the end of a variable, concatinates the value with the string following the dot
	//        .. reduces to .
	//        && reduces to & with no variable substitution

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
		p1++ ;
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


void pApplication::info()
{
	llog( "-", "*************************************************************************************************************" << endl ) ;
	llog( "-", "Application Information for "<< zappname << endl ) ;
	llog( "-", "                   Task ID: "<< taskId << endl ) ;
	llog( "-", "          Shared Pool Name: "<< d2ds( shrdPool, 8 ) << endl ) ;
	llog( "-", "         Profile Pool Name: "<< p_poolMGR->get( errBlock, "ZAPPLID", SHARED ) << endl ) ;
	llog( "-", " " << endl ) ;
	llog( "-", "Application Description . : "<< zappdesc << endl ) ;
	llog( "-", "Application Version . . . : "<< zappver  << endl ) ;
	llog( "-", "Last Panel Displayed. . . : "<< currPanel->panelid << endl ) ;
	llog( "-", "Last Message Displayed. . : "<< zmsgid << endl ) ;
	llog( "-", "Number of Panels Loaded . : "<< panelList.size() << endl )  ;
	llog( "-", "Number of Open Tables . . : "<< tablesOpen.size() << endl ) ;
	if ( rexxName != "" )
	{
		llog( "-", "Application running REXX. : "<< rexxName << endl ) ;
	}
	llog( "-", " " << endl ) ;
	if ( testMode )
	{
		llog( "-", "Application running in test mode"<< endl ) ;
	}
	if ( noTimeOut )
	{
		llog( "-", "Application has disabled timeouts"<< endl ) ;
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


void pApplication::loadCommandTable()
{
	//  Load application command table in the application task so it can be unloaded on task termination
	//  This is done during SELECT processing so LIBDEFs must be active with PASSLIB specified,
	//  if being used to find the table.

	p_tableMGR->loadTable( errBlock, get_applid() + "CMDS", NOWRITE, get_search_path( s_ZTLIB ), SHARE ) ;
}


void pApplication::ispexec( const string& s )
{
	ispexeci( this, s, errBlock ) ;
	if ( errBlock.error() )
	{
		errBlock.setcall( "ISPEXEC Interface Error" ) ;
		checkRCode( errBlock ) ;
		return ;
	}
	errBlock.clearsrc() ;
}


void pApplication::checkRCode()
{
	// If the error panel is to be displayed, cancel CONTROL DISPLAY LOCK and remove any popup's

	// If this is issued as a result of a service call (a call from another thread ie. lspf), just return.
	// The calling thread needs to check further for errors as there is not much that can be done here.

	int RC1 ;

	if ( errBlock.ServiceCall() )
	{
		errBlock.seterror() ;
		return ;
	}

	RC1 = RC ;

	if ( zerr1 != "" ) { llog( "E", zerr1 << endl ) ; }
	if ( zerr2 != "" ) { llog( "E", zerr2 << endl ) ; }

	if ( not ControlErrorsReturn && RC >= 12 )
	{
		llog( "E", "RC="<< RC <<" CONTROL ERRORS CANCEL is in effect.  Aborting"<< endl ) ;
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
		ControlDisplayLock  = false ;
		ControlErrorsReturn = true  ;
		selPanel            = false ;
		if ( addpop_active ) { rempop( "ALL" ) ; }
		display( "PSYSER1" )  ;
		if ( RC <= 8 ) { errPanelissued = true ; }
		abend() ;
	}
}


void pApplication::checkRCode( errblock err )
{
	// If the error panel is to be displayed, cancel CONTROL DISPLAY LOCK and remove any popup's

	// Format: msg1   header - call description resulting in the error
	//         short  msg
	//         longer description

	// Set RC to the error code in the error block if we are returning to the program (CONTROL ERRORS RETURN)
	// and clear errBlock (will have been re-used for services run from this routine)

	// Terminate processing if this routing is called during error processing.

	// If this is issued as a result of a service call (a call from another thread ie. lspf), just return.
	// The calling thread needs to check further for errors as there is not much that can be done here.

	string t ;

	if ( err.ServiceCall() )
	{
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
		llog( "E", "Error ZVAL1: "<< err.val1 << endl )  ;
		llog( "E", "Error ZVAL2: "<< err.val2 << endl )  ;
		llog( "E", "Error ZVAL3: "<< err.val3 << endl )  ;
		llog( "E", "Source     : "<< err.getsrc() << endl ) ;
		abend() ;
	}

	if ( ControlErrorsReturn )
	{
		RC = err.getRC() ;
		errBlock.clear() ;
		return ;
	}

	errBlock.setAbending() ;

	if ( err.val1 != "" ) { vreplace( "ZVAL1", err.val1 ) ; }
	if ( err.val2 != "" ) { vreplace( "ZVAL2", err.val2 ) ; }
	if ( err.val3 != "" ) { vreplace( "ZVAL3", err.val3 ) ; }

	getmsg( err.msgid, "ZERRSM", "ZERRLM" ) ;

	vreplace( "ZERRMSG", err.msgid )  ;
	vreplace( "ZERRRC", d2ds( err.getRC() ) ) ;

	llog( "E", err.msg1 << endl ) ;

	vcopy( "ZERRSM", t, MOVE ) ;
	if ( t != "" ) { llog( "E", t << endl ) ; }
	vcopy( "ZERRLM", t, MOVE ) ;
	if ( t != "" )
	{
		llog( "E", t << endl ) ;
		splitZerrlm( t ) ;
	}

	llog( "E", "RC="<< err.getRC() <<" CONTROL ERRORS CANCEL is in effect.  Aborting" << endl ) ;

	vreplace( "ZAPPNAME", zappname ) ;
	vreplace( "ZERRRX", rexxName )  ;
	vreplace( "ZERR1",  err.msg1  ) ;

	vreplace( "ZERR2", "" ) ;
	vreplace( "ZERR3", err.getsrc() ) ;

	if ( err.getsrc() != "" )
	{
		if ( err.dialogsrc() )
		{
			vreplace( "ZERR2", "Current dialogue statement:" ) ;
		}
		else
		{
			if ( err.panelsrc() )
			{
				vreplace( "ZERR2",  "Panel statement where error was detected:" ) ;
			}
			else
			{
				vreplace( "ZERR2",  "Line where error was detected:" ) ;
			}
		}
	}

	ControlDisplayLock  = false ;
	ControlErrorsReturn = true  ;
	selPanel            = false ;

	if ( addpop_active ) { rempop( "ALL" ) ; }
	errBlock.clear() ;

	display( "PSYSER2" )  ;
	if ( RC <= 8 ) { errPanelissued = true ; }
	abend() ;
}


void pApplication::splitZerrlm( string t )
{
	size_t i ;

	int l    ;
	int maxw ;

	vdefine( "ZSCRMAXW", &maxw ) ;
	vget( "ZSCRMAXW", SHARED ) ;

	maxw = maxw - 6 ;
	l    = 0        ;
	do
	{
		l++ ;
		if ( t.size() > size_t( maxw ) )
		{
			i = t.find_last_of( ' ', maxw ) ;
			i = ( i == string::npos ) ? maxw : i + 1 ;
			vreplace( "ZERRLM"+ d2ds( l ), t.substr( 0, i ) ) ;
			t.erase( 0, i ) ;
		}
		else
		{
			vreplace( "ZERRLM"+d2ds( l ), t ) ;
			t = "" ;
		}
	} while ( t.size() > 0 ) ;
}


/* ************************************ ***************************** ********************************* */
/* ************************************ Abnormal Termination Routines ********************************* */
/* ************************************ ***************************** ********************************* */

void pApplication::cleanup_default()
{
	// Dummy routine.  Override in the application so the customised one is called on an exception condition.
	// Use CONTROL ABENDRTN ptr_to_routine to set

	// Called on: abend()
	//            abendexc()
	//            set_forced_abend()
	//            set_timeout_abend()
}


void pApplication::abend()
{
	abend_nothrow() ;
	throw( pApplication::xTerminate() ) ;
}


void pApplication::abend_nothrow()
{
	llog( "E", "Shutting down application: "+ zappname +" Taskid: " << taskId << " due to an abnormal condition" << endl ) ;
	abnormalEnd   = true  ;
	terminateAppl = true  ;
	SEL           = false ;
	(this->*pcleanup)()   ;
	abended       = true  ;
}


void pApplication::uabend()
{
	// Abend application with no messages.

	abnormalEnd   = true  ;
	abnormalNoMsg = true  ;
	terminateAppl = true  ;
	SEL           = false ;
	(this->*pcleanup)()   ;
	abended       = true  ;
	throw( pApplication::xTerminate() ) ;
}


void pApplication::uabend( const string& msgid, int callno )
{
	// Abend application with error screen.
	// Screen will show short and long messages from msgid, and the return code RC
	// Optional variables can be coded after the message id and are placed in ZVAR1, ZVAR2 etc.

	vreplace( "ZERRRC", d2ds( RC ) ) ;
	xabend( msgid, callno ) ;
}


void pApplication::uabend( const string& msgid, const string& val1, int callno )
{
	vreplace( "ZERRRC", d2ds( RC ) ) ;
	vreplace( "ZVAL1", val1 ) ;
	xabend( msgid, callno )   ;
}


void pApplication::uabend( const string& msgid, const string& val1, const string& val2, int callno )
{
	vreplace( "ZERRRC", d2ds( RC ) ) ;
	vreplace( "ZVAL1", val1 ) ;
	vreplace( "ZVAL2", val2 ) ;
	xabend( msgid, callno )   ;
}


void pApplication::uabend( const string& msgid, const string& val1, const string& val2, const string& val3, int callno )
{
	vreplace( "ZERRRC", d2ds( RC ) ) ;
	vreplace( "ZVAL1", val1 ) ;
	vreplace( "ZVAL3", val2 ) ;
	vreplace( "ZVAL4", val3 ) ;
	xabend( msgid, callno )   ;
}


void pApplication::xabend( const string& msgid, int callno )
{
	string t ;

	getmsg( msgid, "ZERRSM", "ZERRLM" ) ;
	vcopy( "ZERRLM", t, MOVE ) ;
	splitZerrlm( t ) ;

	t = "A user abend has occured in application "+ zappname ;
	if ( callno != -1 ) { t += " at call number " + d2ds( callno ) ; }

	llog( "E", "Shutting down application: "+ zappname +" Taskid: "<< taskId <<" due to a user abend" << endl ) ;

	vreplace( "ZAPPNAME", zappname ) ;
	vreplace( "ZERRRX", rexxName ) ;
	vreplace( "ZERRMSG", msgid ) ;
	vreplace( "ZERR1",  t  ) ;
	vreplace( "ZERR2",  "" ) ;
	vreplace( "ZERR3",  "" ) ;
	ControlDisplayLock  = false ;
	ControlErrorsReturn = true  ;
	selPanel            = false ;

	display( "PSYSER2" ) ;
	if ( RC <= 8 ) { errPanelissued = true ; }

	abnormalEnd   = true  ;
	terminateAppl = true  ;
	SEL           = false ;
	(this->*pcleanup)()   ;
	abended       = true  ;
	throw( pApplication::xTerminate() ) ;
}


void pApplication::abendexc()
{
	llog( "E", "An unhandled exception has occured in application: "+ zappname +" Taskid: " << taskId << endl ) ;
	if ( !abending )
	{
		abending = true     ;
		(this->*pcleanup)() ;
	}
	else
	{
		llog( "E", "An abend has occured during abend processing.  Cleanup will not be called" << endl ) ;
	}
	exception_ptr ptr = current_exception() ;
	llog( "E", "Exception: " << (ptr ? ptr.__cxa_exception_type()->name() : "Unknown" ) << endl ) ;
	llog( "E", "Shutting down application: " << zappname << " Taskid: " << taskId << endl ) ;
	abnormalEnd   = true  ;
	terminateAppl = true  ;
	SEL           = false ;
	abended       = true  ;
}


void pApplication::set_forced_abend()
{
	llog( "E", "Shutting down application: "+ zappname +" Taskid: "<< taskId <<" due to a forced condition" << endl ) ;
	abnormalEnd       = true  ;
	abnormalEndForced = true  ;
	terminateAppl     = true  ;
	SEL               = false ;
	(this->*pcleanup)()       ;
	abended           = true  ;
}


void pApplication::set_timeout_abend()
{
	llog( "E", "Shutting down application: "+ zappname +" Taskid: "<< taskId <<" due to a timeout condition" << endl ) ;
	abnormalEnd       = true  ;
	abnormalEndForced = true  ;
	abnormalTimeout   = true  ;
	terminateAppl     = true  ;
	SEL               = false ;
	(this->*pcleanup)()       ;
	abended           = true  ;
	busyAppl          = false ;
}
