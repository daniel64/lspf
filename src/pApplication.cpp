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

#undef  MOD_NAME
#undef  LOGOUT

#define MOD_NAME pApplic
#define LOGOUT   aplog

int  ispexeci( pApplication *, const string & ) ;

#include <stdexcept>

pApplication::pApplication()
{
	currApplication        = this   ;
	testMode               = false  ;
	propagateEnd           = false  ;
	jumpEntered            = false  ;
	resumeTime             = boost::posix_time::second_clock::universal_time() ;
	ControlDisplayLock     = false  ;
	ControlNonDispl        = false  ;
	ControlErrorsReturn    = false  ;
	ControlPassLRScroll    = false  ;
	ControlSplitEnable     = true   ;
	ControlRefUpdate       = true   ;
	errPanelissued         = false  ;
	abending               = false  ;
	addpop_active          = false  ;
	addpop_row             = 0      ;
	addpop_col             = 0      ;
	noTimeOut              = false  ;
	busyAppl               = false  ;
	terminateAppl          = false  ;
	abnormalEnd            = false  ;
	abnormalEndForced      = false  ;
	abnormalTimeout        = false  ;
	rawOutput              = false  ;
	reloadCUATables        = false  ;
	rexxName               = ""     ;
	NEWPOOL                = false  ;
	PASSLIB                = false  ;
	SUSPEND                = false  ;
	libdef_muser           = false  ;
	libdef_puser           = false  ;
	libdef_tuser           = false  ;
	SEL                    = false  ;
	setMSG                 = false  ;
	reffield               = ""     ;
	PANELID                = ""     ;
	PPANELID               = ""     ;
	ZHELP                  = ""     ;
	ZAHELP                 = ""     ;
	ZTDROWS                = 0      ;
	ZTDTOP                 = 0      ;
	ZCURFLD                = ""     ;
	ZCURPOS                = 1      ;
	ZRC                    = 0      ;
	ZRSN                   = 0      ;
	ZRESULT                = ""     ;
	ZERR1                  = ""     ;
	ZERR2                  = ""     ;
	ZERR3                  = ""     ;
	ZERR4                  = ""     ;
	ZERR5                  = ""     ;
	ZERR6                  = ""     ;
	ZERR7                  = ""     ;
	ZERR8                  = ""     ;
	ZERR9                  = ""     ;
	ZERR10                 = ""     ;
	vdefine( "ZCURFLD",  &ZCURFLD ) ;
	vdefine( "ZCURPOS",  &ZCURPOS ) ;
	vdefine( "ZTDMARK",  &ZTDMARK ) ;
	vdefine( "ZTDDEPTH", &ZTDDEPTH) ;
	vdefine( "ZTDROWS",  &ZTDROWS ) ;
	vdefine( "ZTDSELS",  &ZTDSELS ) ;
	vdefine( "ZTDTOP",   &ZTDTOP  ) ;
	vdefine( "ZTDVROWS", &ZTDVROWS) ;
	vdefine( "ZAPPNAME", &ZAPPNAME) ;
	rmsgs.clear()                   ;
}


pApplication::~pApplication()
{
	map<string, pPanel *>::iterator it;
	for ( it = panelList.begin() ; it != panelList.end() ; it++ )
	{
		delete it->second ;
	}
	busyAppl = false ;
}


void pApplication::init()
{
	// Before being dispatched in its own thread, set the search paths, table display BOD mark and
	// addpop status if this has not been invoked with the SUSPEND option on the SELECT command

	int scrNum ;

	ZPLIB    = p_poolMGR->get( RC, "ZPLIB", PROFILE ) ;
	ZTLIB    = p_poolMGR->get( RC, "ZTLIB", PROFILE ) ;
	ZMLIB    = p_poolMGR->get( RC, "ZMLIB", PROFILE ) ;
	ZORXPATH = p_poolMGR->get( RC, "ZORXPATH", PROFILE ) ;

	ZTDMARK  = centre( " Bottom of Data ", ds2d( p_poolMGR->get( RC, "ZSCRMAXW", SHARED ) ), '*' ) ;

	if ( !SUSPEND )
	{
		scrNum     = ds2d(p_poolMGR->get( RC, "ZSCRNUM", SHARED ) ) ;
		addpop_row = ds2d( p_poolMGR->get( RC, scrNum, "ZPROW" ) ) ;
		addpop_col = ds2d( p_poolMGR->get( RC, scrNum, "ZPCOL" ) ) ;
		if ( addpop_row != 0 || addpop_col != 0 )
		{
			addpop_active = true ;
		}
	}

	log( "I", "Initialisation complete" << endl ; )
}


void pApplication::wait_event()
{
	busyAppl = false ;
	while ( true )
	{
		if ( terminateAppl ) { RC = 20 ; log( "E", "Application terminating.  Cancelling wait_event" << endl ) ; abend() ; }
		if ( !busyAppl )
		{
			boost::mutex::scoped_lock lk(mutex) ;
			cond_appl.wait(lk) ;
			lk.unlock() ;
		}
		if ( busyAppl ) { return ; }
	}
}


void pApplication::createPanel( const string & p_name )
{
	string paths ;

	RC = 0 ;

	if ( panelList.count( p_name ) > 0 ) { return ; }
	if ( !isvalidName( p_name ) ) { RC = 20 ; checkRCode( "Invalid panel name '"+ p_name +"'" ) ; return ; }

	pPanel * p_panel    = new pPanel ;
	p_panel->p_poolMGR  = p_poolMGR  ;
	p_panel->p_funcPOOL = &funcPOOL  ;
	p_panel->LRScroll   = ControlPassLRScroll ;
	p_panel->REXX       = (rexxName != "" )   ;
	p_panel->init( RC )              ;

	if ( libdef_puser ) { paths = mergepaths( ZPUSER, ZPLIB ) ; }
	else                { paths = ZPLIB                       ; }

	RC = p_panel->loadPanel( p_name, paths ) ;

	if ( RC == 0 )
	{
		panelList[ p_name ] = p_panel ;
		load_keylist( p_panel )       ;

	}
	else if ( RC == 12 )
	{
		ZERR2 = "Panel not found" ;
		delete p_panel            ;

	}
	else
	{
		ZERR2  = p_panel->PERR1 ;
		ZERR3  = p_panel->PERR2 ;
		if ( p_panel->PERR3 != "" )
		{
			ZERR9  = "Panel line where error was detected:" ;
			ZERR10 = p_panel->PERR3 ;
		}
		delete p_panel ;
	}
}


bool pApplication::isprimMenu()
{
	if ( panelList.size() == 0 ) { return false                  ; }
	else                         { return currPanel->primaryMenu ; }
}


void pApplication::get_home( uint & row, uint & col )
{
	if ( panelList.size() == 0 ) { row = 0 ; col = 0               ; }
	else                         { currPanel->get_home( row, col ) ; }
}


void pApplication::get_cursor( uint & row, uint & col )
{
	if ( panelList.size() == 0 ) { row = 0 ; col = 0                 ; }
	else                         { currPanel->get_cursor( row, col ) ; }
}


string pApplication::get_current_panelDescr()
{
	if ( panelList.size() == 0 ) { return ""                          ; }
	else                         { return currPanel->get_panelDescr() ; }
}


string pApplication::get_current_screenName()
{
	// This is called from lspf thread, so we need to set the correct shared pool for this
	// application thread when getting ZSCRNAME.  Must be set back after this call in lspf

	int RCode      ;
	string scrname ;

	p_poolMGR->setShrdPool( RCode, shrdPool ) ;
	return p_poolMGR->get( RCode, "ZSCRNAME", SHARED ) ;
}


bool pApplication::inputInhibited()
{
	if ( panelList.size() > 0 ) { return currPanel->inputInhibited() ; }
	return false ;
}


bool pApplication::msgInhibited()
{
	if ( panelList.size() > 0 ) { return currPanel->msgInhibited() ; }
	return false ;
}


void pApplication::msgResponseOK()
{
	if ( panelList.size() > 0 ) { currPanel->msgResponseOK() ; }
}


void pApplication::store_scrname()
{
	int RC1 ;

	ZSCRNAME = p_poolMGR->get( RC1, "ZSCRNAME", SHARED ) ;
}


void pApplication::restore_Zvars( int screenID )
{
	int RC1 ;

	if ( panelList.size() > 0 ) { currPanel->update_keylist_vars() ; }

	if ( p_poolMGR->get( RC1, screenID, "ZSCRNAM2" ) == "PERM" )
	{
			ZSCRNAME = p_poolMGR->get( RC1, screenID, "ZSCRNAME" ) ;
	}
	p_poolMGR->put( RC1, "ZSCRNAME", ZSCRNAME, SHARED ) ;

	p_poolMGR->put( RC, screenID, "ZPROW", d2ds( addpop_row ) ) ;
	p_poolMGR->put( RC, screenID, "ZPCOL", d2ds( addpop_col ) ) ;
}


void pApplication::refresh_id()
{
	if ( panelList.size() > 0 ) { currPanel->display_id() ; }
}


void pApplication::set_msg( const string & msg_id )
{
	// Display a message on current panel using msg_id

	// This is called from mainline lspf, so set CONTROL ERRORS RETURN and restore
	// original CONTROL ERROR settings on exit.  This is so lspf does not hang on an error.

	bool errorStatus ;

	if ( panelList.size() == 0 ) { return ; }

	errorStatus = ControlErrorsReturn ;

	ControlErrorsReturn = true ;
	get_Message( msg_id ) ;
	if ( RC == 0 )
	{
		currPanel->clear_msg_loc() ;
		currPanel->set_panel_msg( MSG, MSGID ) ;
		currPanel->display_msg()   ;
	}
	ControlErrorsReturn = errorStatus  ;
}


void pApplication::set_msg1( const slmsg & t, string msgid, bool Immed )
{
	// Propogate setmsg() to the current panel using the short-long-message object.
	// If immed, display the message now rather than allow the application DISPLAY()/TBDISPL() service to it.

	// MSG1 and MSGID1 are used to store messages issued by the setmsg() service after variable substitution.

	MSG1   = t     ;
	MSGID1 = msgid ;
	setMSG = true  ;

	if ( Immed && panelList.size() > 0 )
	{
		currPanel->set_panel_msg( MSG1, MSGID1 ) ;
		currPanel->display_msg() ;
		setMSG = false ;
	}
}


bool pApplication::nretriev_on()
{
	if ( panelList.size() == 0 ) { return false                     ; }
	else                         { return currPanel->get_nretriev() ; }
}


string pApplication::get_nretfield()
{
	if ( panelList.size() == 0 ) { return ""                         ; }
	else                         { return currPanel->get_nretfield() ; }
}


void pApplication::display( string p_name, const string & p_msg, const string & p_cursor, int p_csrpos )
{
	string ZZVERB ;

	int  scrNum   ;
	bool doReinit ;

	RC       = 0     ;
	doReinit = false ;

	if ( p_name == "" )
	{
		if ( PANELID == "" ) { RC = 20 ; checkRCode( "No panel specified" ) ; return ; }
		p_name   = PANELID ;
		doReinit = true    ;
	}

	if ( p_cursor != "" && !isvalidName( p_cursor ) )
	{
		RC = 20 ;
		checkRCode( "Invalid CURSOR position "+ p_cursor ) ;
		return ;
	}

	createPanel( p_name ) ;
	if ( RC > 0 ) { checkRCode( "Panel '"+ p_name +"' error during DISPLAY" ) ; return ; }

	if ( propagateEnd )
	{
		if ( PPANELID == p_name ) { propagateEnd = false ;                }
		else                      { PPANELID = p_name ; RC = 8 ; return ; }
	}

	currPanel = panelList[ p_name ] ;

	if ( setMSG )
	{
		if ( !ControlNonDispl )
		{
			currPanel->set_panel_msg( MSG1, MSGID1 ) ;
			setMSG = false ;
		}
	}
	else if ( p_msg != "" )
	{
		get_Message( p_msg ) ;
		if ( RC > 0 ) { return ; }
		currPanel->set_panel_msg( MSG, MSGID ) ;
	}
	else
	{
		currPanel->clear_msg() ;
	}
	PANELID           = p_name   ;
	currPanel->CURFLD = p_cursor ;
	currPanel->CURPOS = p_csrpos ;
	if ( addpop_active ) { currPanel->set_popup( addpop_row, addpop_col ) ; }
	else                 { currPanel->remove_popup()                      ; }

	p_poolMGR->put( RC, "ZPANELID", p_name, SHARED, SYSTEM ) ;

	if ( doReinit )
	{
		currPanel->display_panel_reinit( RC ) ;
		if ( RC > 0 ) { ZERR2 = currPanel->PERR1 ; checkRCode( "Error processing )REINIT section of panel "+ p_name ) ; return ; }
	}
	else
	{
		currPanel->display_panel_init( RC ) ;
		if ( RC > 0 ) { ZERR2 = currPanel->PERR1 ; checkRCode( "Error processing )INIT section of panel "+ p_name ) ; return ; }
		currPanel->update_field_values( RC ) ;
		if ( RC > 0 ) { ZERR2 = currPanel->PERR1 ; checkRCode( "Error updating field values of panel "+ p_name ) ; return ; }
	}

	scrNum = ds2d(p_poolMGR->get( RC, "ZSCRNUM", SHARED ) ) ;
	if ( p_poolMGR->get( RC, "ZSCRNAM1", SHARED ) == "ON" &&
	     p_poolMGR->get( RC, scrNum, "ZSCRNAM2" ) == "PERM" )
	{
			p_poolMGR->put( RC, "ZSCRNAME", p_poolMGR->get( RC, scrNum, "ZSCRNAME" ), SHARED ) ;
	}

	while ( true )
	{
		currPanel->display_panel( RC ) ;
		if ( RC > 0 ) { ZERR2 = currPanel->PERR1 ; checkRCode( "Panel error displaying "+ p_name ) ; break ; }
		currPanel->cursor_to_field( RC ) ;
		if ( RC > 0 ) { checkRCode( "Cursor field '"+ currPanel->CURFLD +"' not found on panel or invalid for DISPLAY" ) ; break ; }
		wait_event() ;
		ControlDisplayLock = false ;
		ControlNonDispl    = false ;
		currPanel->display_panel_update( RC ) ;

		currPanel->display_panel_proc( RC, 0 ) ;
		if ( RC > 0 ) { ZERR2 = currPanel->PERR1 ; checkRCode( "Error processing )PROC section of panel "+ p_name ) ; break ; }

		ZZVERB = p_poolMGR->get( RC, "ZVERB", SHARED ) ;
		if ( RC > 0 ) { RC = 20 ; checkRCode( "PoolMGR get of ZVERB failed" ) ; }
		if ( ZZVERB == "RETURN" )                    { propagateEnd = true ; }
		if ( findword( ZZVERB, "END EXIT RETURN" ) ) { RC = 8 ; return     ; }

		if ( currPanel->MSGID != "" )
		{
			get_Message( currPanel->MSGID ) ;
			if ( RC > 0 ) { break ; }
			currPanel->set_panel_msg( MSG, MSGID ) ;
			currPanel->display_panel_reinit( RC ) ;
			if ( RC > 0 ) { ZERR2 = currPanel->PERR1 ; checkRCode( "Error processing )REINIT section of panel "+ p_name ) ; return ; }
			continue ;
		}
		break ;
	}
	currPanel->clear_msg()  ;
	currPanel->resetAttrs() ;
}


void pApplication::refresh()
{
	RC = 0 ;
	map<string, pPanel *>::iterator it ;

	it = panelList.find( PANELID ) ;
	if ( it != panelList.end() )
	{
		it->second->refresh() ;
	}
}


void pApplication::libdef( const string & lib, const string & type )
{
	// libdef - Add/remove a list of paths to the search order for panels, messages and tables

	// RC = 0   Normal completion
	// RC = 4   Removing a libdef that was not in effect
	// RC = 16  No paths in the corresponding ZxUSER variable
	// RC = 20  Severe error

	if ( wordpos ( lib, "ZMUSER ZPUSER ZTUSER" ) == 0 ) { RC = 20 ; checkRCode( "Invalid variable name on libdef" ) ; return ; }
	if ( type == "" )
	{
		if ( lib == "ZMUSER" )
		{
			if ( !libdef_muser ) { RC = 4 ; log( "E", "No LIBDEF for messages is active" << endl ) ; return ; }
			libdef_muser = false ;
		}
		else if ( lib == "ZPUSER" )
		{
			if ( !libdef_puser ) { RC = 4 ; log( "E", "No LIBDEF for panels is active" << endl ) ; return ; }
			libdef_puser = false ;
		}
		else
		{
			if ( !libdef_tuser ) { RC = 4 ; log( "E", "No LIBDEF for tables is active" << endl ) ; return ; }
			libdef_tuser = false ;
		}
	}
	else if ( type == "FILE" )
	{
		if ( lib == "ZMUSER" )
		{
			if ( ZMUSER == ""   ) { RC = 16 ; checkRCode( "ZMUSER has not been assigned one or more paths for user message search" ) ; return ; }
			libdef_muser = true ;
		}
		else if ( lib == "ZPUSER" )
		{
			if ( ZPUSER == ""   ) { RC = 16 ; checkRCode( "ZPUSER has not been assigned one or more paths for user panel search" ) ; return ; }
			libdef_puser = true ;
		}
		else
		{
			if ( ZTUSER == ""   ) { RC = 16 ; checkRCode( "ZTUSER has not been assigned one or more paths for user table search" ) ; return ; }
			libdef_tuser = true ;
		}
	}
	else
	{
		RC = 20 ;
		checkRCode( "Invalid type on LIBDEF" ) ;
	}
}


string pApplication::get_select_cmd( const string & opt )
{
	return panelList[ PANELID ]->return_command( opt ) ;
}


void pApplication::set_cursor( int row, int col )
{
	if ( panelList.size() > 0 ) { currPanel->set_cursor( row, col ) ; }
}


void pApplication::vdefine( const string & names, int * i_ad1, int * i_ad2, int * i_ad3, int * i_ad4, int * i_ad5, int * i_ad6, int * i_ad7, int * i_ad8 )
{
	// RC = 0  Normal completion
	// RC = 20 Severe Error
	// (funcPOOL.define returns 0 or 20)

	int w ;

	string name ;

	const string e1( "Too many variables on VDEFINE statement" ) ;
	const string e2( "Address is null on VDEFINE statement" )    ;
	const string e3( "Error in function pool define for " )      ;

	w = words( names ) ;
	if ( ( w > 8 ) || ( w < 1 ) ) { RC = 20 ; checkRCode( e1 ) ; return ; }

	RC = 0 ;

	if ( i_ad1 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
	name = word( names, 1 ) ;
	funcPOOL.define( RC, name, i_ad1 ) ;
	if ( RC > 0 ) { checkRCode( e3 + name ) ; return ; }

	if ( w > 1 )
	{
		if ( i_ad2 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 2 ) ;
		funcPOOL.define( RC, name , i_ad2 ) ;
		if ( RC > 0 ) { checkRCode( e3 + name ) ; return ; }
	}
	if ( w > 2 )
	{
		if ( i_ad3 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 3 ) ;
		funcPOOL.define( RC, name, i_ad3 ) ;
		if ( RC > 0 ) { checkRCode( e3 + name ) ; return ; }
	}
	if ( w > 3 )
	{
		if ( i_ad4 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 4 ) ;
		funcPOOL.define( RC, name, i_ad4 ) ;
		if ( RC > 0 ) { checkRCode( e3 + name ) ; return ; }
	}
	if ( w > 4 )
	{
		if ( i_ad5 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 5 ) ;
		funcPOOL.define( RC, name, i_ad5 ) ;
		if ( RC > 0 ) { checkRCode( e3 + name ) ; return ; }
	}
	if ( w > 5 )
	{
		if ( i_ad6 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 6 ) ;
		funcPOOL.define( RC, name, i_ad6 ) ;
		if ( RC > 0 ) { checkRCode( e3 + name ) ; return ; }
	}

	if ( w > 6 )
	{
		if ( i_ad7 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 7 ) ;
		funcPOOL.define( RC, name, i_ad7 ) ;
		if ( RC > 0 ) { checkRCode( e3 + name ) ; return ; }
	}

	if ( w > 7 )
	{
		if ( i_ad8 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 8 ) ;
		funcPOOL.define( RC, name, i_ad8 ) ;
		if ( RC > 0 ) { checkRCode( e3 + name ) ; return ; }
	}
}



void pApplication::vdefine( const string & names, string * s_ad1, string * s_ad2, string * s_ad3, string * s_ad4, string * s_ad5, string * s_ad6, string * s_ad7, string * s_ad8 )
{
	// RC = 0  Normal completion
	// RC = 20 Severe Error
	// (funcPOOL.define returns 0 or 20)

	int w  ;

	string name ;

	const string e1( "Too many variables on VDEFINE statement" ) ;
	const string e2( "Address is null on VDEFINE statement" )    ;
	const string e3( "Error in function pool define for " )      ;

	RC = 0 ;

	w = words( names ) ;
	if ( ( w > 8 ) | ( w < 1 ) ) { RC = 20 ; checkRCode( e1 ) ; return ; }

	if ( s_ad1 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
	name = word( names, 1 ) ;
	funcPOOL.define( RC, name, s_ad1 ) ;
	if ( RC > 0 ) { checkRCode( e3 + name ) ; return ; }

	if ( w > 1 )
	{
		if ( s_ad2 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 2 ) ;
		funcPOOL.define( RC, name, s_ad2 ) ;
		if ( RC > 0 ) { checkRCode( e3 + name ) ; return ; }
	}
	if ( w > 2 )
	{
		if ( s_ad3 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 3 ) ;
		funcPOOL.define( RC, name, s_ad3 ) ;
		if ( RC > 0 ) { checkRCode( e3 + name ) ; return ; }
	}
	if ( w > 3 )
	{
		if ( s_ad4 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 4 ) ;
		funcPOOL.define( RC, name, s_ad4 ) ;
		if ( RC > 0 ) { checkRCode( e3 + name ) ; return ; }
	}
	if ( w > 4 )
	{
		if ( s_ad5 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 5 ) ;
		funcPOOL.define( RC, name, s_ad5 ) ;
		if ( RC > 0 ) { checkRCode( e3 + name ) ; return ; }
	}
	if ( w > 5 )
	{
		if ( s_ad6 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 6 ) ;
		funcPOOL.define( RC, name, s_ad6 ) ;
		if ( RC > 0 ) { checkRCode( e3 + name ) ; return ; }
	}
	if ( w > 6 )
	{
		if ( s_ad7 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 7 ) ;
		funcPOOL.define( RC, name, s_ad7 ) ;
		if ( RC > 0 ) { checkRCode( e3 + name ) ; return ; }
	}
	if ( w > 7 )
	{
		if ( s_ad8 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 8 ) ;
		funcPOOL.define( RC, name, s_ad8 ) ;
		if ( RC > 0 ) { checkRCode( e3 + name ) ; return ; }
	}
}


void pApplication::vdelete( const string & names )
{
	// RC = 0  Normal completion
	// RC = 8  At least one variable not found
	// RC = 20 Severe error
	// (funcPOOL.dlete returns 0, 8 or 20)

	int i       ;
	int ws      ;
	int maxRC   ;

	string name ;

	RC    = 0 ;
	maxRC = 0 ;

	ws = words( names ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		name = word( names, i ) ;
		funcPOOL.dlete( RC, name ) ;
		if ( RC > 8 ) { checkRCode( "VDELETE failed for "+ name ) ; }
		maxRC = max( maxRC, RC ) ;
		if ( RC > 8 ) { return   ; }
	}
	RC = maxRC ;
}


void pApplication::vmask( const string & name, const string & type, const string & mask )
{
	// Set a mask for a function pool variable (must be vdefined first)
	// Partial implementation as no VEDIT panel statement yet so this is never used

	// RC = 0  Normal completion
	// RC = 8  Variable not found
	// RC = 20 Severe error
	// (funcPOOL.setmask returns 0, 8 or 20)

	int i ;
	const string fmask = "IDATE STDDATE ITIME STDTIME JDATE JSTD" ;

	if ( type == "FORMAT" )
	{
		if ( wordpos( mask, fmask ) == 0 ) { RC = 20 ; }
	}
	else if ( type == "USER" )
	{
		if ( mask.size() > 20 ) { RC = 20 ; }
		else
		{
			for ( i = 0 ; i < mask.size() ; i++ )
			{
				if ( mask[i] != 'A' && mask[i] != 'B' && mask[i] != '9' &&
				     mask[i] != 'H' && mask[i] != 'N' && mask[i] != 'V' &&
				     mask[i] != 'S' && mask[i] != 'X' && mask[i] != '(' &&
				     mask[i] != ')' && mask[i] != '-' && mask[i] != '/' &&
				     mask[i] != ',' && mask[i] != '.' ) { RC = 20 ; break ; }
			}
		}
	}
	else { RC = 20 ; }
	if ( RC > 0 ) { checkRCode( "VMASK invalid format for '"+ name +"'.  Mask: "+ mask ) ; return ; }

	funcPOOL.setmask( RC, name, mask ) ;
	if ( RC > 8 ) { checkRCode( "VMASK failed for "+ name ) ; }
}


void pApplication::vreset()
{
	// RC = 0  Normal completion
	// RC = 20 Severe error
	// (funcPOOL.reset returns 0)

	funcPOOL.reset() ;
}


void pApplication::vreplace( const string & name, const string & s_val )
{
	// RC = 0  Normal completion
	// RC = 20 Severe error
	// (funcPOOL.put returns 0, 20)

	RC = 0 ;
	funcPOOL.put( RC, 0, name, s_val ) ;
	if ( RC > 0 ) { checkRCode( "VREPLACE failed for '"+ name +"'" ) ; }
}


void pApplication::vreplace( const string & name, int i_val )
{
	// RC = 0  Normal completion
	// RC = 20 Severe error
	// (funcPOOL.put returns 0 or 20)

	RC = 0 ;
	funcPOOL.put( RC, 0, name, i_val ) ;
	if ( RC > 0 ) { checkRCode( "VREPLACE failed for '"+ name +"'" ) ; }
}


void pApplication::vget( const string & names, poolType pType )
{
	// RC = 0  Normal completion
	// RC = 8  Variable not found
	// RC = 20 Severe error
	// (funcPOOL.put returns 0 or 20)
	// (funcPOOL.getType returns 0, 8 or 20.  For RC = 8 create implicit function pool variable)
	// (poolMGR.get return 0, 8 or 20)

	string val  ;
	string name ;

	int maxRC   ;
	int ws      ;
	int i       ;

	dataType var_type ;

	RC    = 0 ;
	maxRC = 0 ;

	ws = words( names ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		name = word( names, i ) ;
		val  = p_poolMGR->get( RC, name, pType ) ;
		if ( RC  > 8 ) { checkRCode( "Pool manager get failed for "+ name ) ; }
		if ( RC == 0 )
		{
			var_type = funcPOOL.getType( RC, name ) ;
			if ( RC  > 8 ) { checkRCode( "Function pool getType failed for "+ name ) ; }
			if ( RC == 0 )
			{
				switch ( var_type )
				{
				case INTEGER:
					funcPOOL.put( RC, 0, name, ds2d( val ) ) ;
					break ;
				case STRING:
					funcPOOL.put( RC, 0, name, val ) ;
				}
				if ( RC > 0 ) { checkRCode( "Function pool put failed for "+ name ) ; }
			}
			else if ( RC == 8 )
			{
				funcPOOL.put( RC, 0, name, val ) ;
				if ( RC > 0 ) { checkRCode( "Function pool put failed creating implicit variable for "+ name ) ; }
			}
		}
		maxRC = max( maxRC, RC ) ;
		if ( RC > 8 ) { return   ; }
	}
	RC = maxRC ;
}


void pApplication::vput( const string & names, poolType pType )
{
	// RC = 0  Normal completion
	// RC = 8  Variable not found in function pool
	// RC = 12 Read-only variable
	// RC = 16 Truncation occured
	// RC = 20 Severe error
	// (funcPOOL.get returns 0, 8 or 20)
	// (funcPOOL.getType returns 0, 8 or 20)
	// (poolMGR.put return 0, 12, 16 or 20)

	int i     ;
	int ws    ;
	int maxRC ;

	string s_val, name ;
	dataType var_type  ;

	RC    = 0 ;
	maxRC = 0 ;

	ws = words( names ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		name     = word( names, i ) ;
		var_type = funcPOOL.getType( RC, name ) ;
		if ( RC  > 8 ) { checkRCode( "Function pool getType failed for "+ name ) ; }
		if ( RC == 0 )
		{
			switch ( var_type )
			{
			case INTEGER:
				s_val = d2ds( funcPOOL.get( RC, 0, var_type, name ) ) ;
				break ;
			case STRING:
				s_val = funcPOOL.get( RC, 0, name ) ;
			}
			if ( RC > 0 ) { checkRCode( "Function pool get failed for "+ name ) ; }
			p_poolMGR->put( RC, name, s_val, pType ) ;
			if ( RC > 0 ) { checkRCode( "Pool manager put failed for "+ name +" RC="+ d2ds( RC ) ) ; }
		}
		maxRC = max( maxRC, RC ) ;
		if ( RC > 8 ) { return   ; }
	}
	RC = maxRC ;
}


void pApplication::vcopy( const string & var, string & val, vcMODE mode )
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

	dataType var_type  ;

	RC = 0 ;

	switch ( mode )
	{
	case LOCATE:
		RC = 20 ;
		log( "E", "LOCATE invalid.  Pointer parameter required, string passed" << endl ) ;
		break ;
	case MOVE:
		var_type = funcPOOL.getType( RC, var ) ;
		if ( RC  > 8 ) { checkRCode( "Function pool getType failed for "+ var ) ; }
		if ( RC == 0 )
		{
			switch ( var_type )
			{
			case INTEGER:
				val = d2ds( funcPOOL.get( RC, 0, var_type, var ) ) ;
				break ;
			case STRING:
				val = funcPOOL.get( RC, 0, var ) ;
			}
			if ( RC  > 0 ) { checkRCode( "Function pool get failed for "+ var ) ; }
		}
		else if ( RC == 8 )
		{
			val = p_poolMGR->get( RC, var, ASIS ) ;
			if ( RC  > 8 ) { checkRCode( "Pool get failed for "+ var ) ; }
		}
	}
}


void pApplication::vcopy( const string & var, string * & p_val, vcMODE mode )
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
	// (poolMGR.get return 0, 8 or 20)

	dataType var_type  ;

	RC = 0 ;

	switch ( mode )
	{
	case LOCATE:
		var_type = funcPOOL.getType( RC, var ) ;
		if ( RC  > 8 ) { checkRCode( "Function pool getType failed for "+ var ) ; }
		if ( RC == 0 )
		{
			switch ( var_type )
			{
			case INTEGER:
				RC = 20 ;
				log( "E", "LOCATE option invalid for integer values" << endl ) ;
				break ;
			case STRING:
				p_val = funcPOOL.vlocate( RC, 0, var, CHECK ) ;
			}
			if ( RC  > 0 ) { checkRCode( "Function pool vlocate failed for "+ var ) ; }
		}
		else if ( RC == 8 )
		{
			p_val = p_poolMGR->vlocate( RC, var, ASIS ) ;
			if ( RC  > 8 ) { checkRCode( "Pool vlocate failed for "+ var ) ; }
		}
		break ;
	case MOVE:
		RC = 20 ;
		log( "E", "MOVE invalid.  String parameter required, pointer passed" << endl ) ;
	}
}


void pApplication::verase( const string & names, poolType pType )
{
	// RC = 0  Normal completion
	// RC = 8  Variable not found
	// RC = 12 Read-only variable
	// RC = 20 Severe error
	// (poolMGR.erase return 0, 8 12 or 20)


	int i       ;
	int ws      ;
	int maxRC   ;

	string name ;

	RC    = 0 ;
	maxRC = 0 ;

	ws = words( names ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		name = word( names, i ) ;
		p_poolMGR->erase( RC, name, pType ) ;
		if ( RC > 8 ) { checkRCode( "Pool erase failed for "+ name) ; }
		maxRC = max( maxRC, RC ) ;
		if ( RC > 8 ) { return   ; }
	}
	RC = maxRC ;
}


string pApplication::vlist( poolType pType, int lvl )
{
	return p_poolMGR->vlist( RC, pType, lvl ) ;
}


string pApplication::vilist( vdType defn )
{
	return funcPOOL.vilist( RC, defn ) ;
}


string pApplication::vslist( vdType defn )
{
	return funcPOOL.vslist( RC, defn ) ;
}


void pApplication::addpop( const string & a_fld, int a_row, int a_col )
{
	//  Create pop-up window and set row/col for the next panel display.  If addpop() is already active, store old values for next rempop()
	//  Position of addpop is relative to row=1, col=3 or the previous addpop() position for this logical screen.
	//  Defaults are 0,0 giving row=1, col=3 (or 2,4 when starting at 1,1)

	//  RC = 0  Normal completion
	//  RC = 12 No panel displayed before addpop() service when using field parameter
	//  RC = 20 Severe error

	int  scrNum     ;

	uint p_row( 0 ) ;
	uint p_col( 0 ) ;

	RC = 0 ;

	scrNum = ds2d(p_poolMGR->get( RC, "ZSCRNUM", SHARED ) ) ;
	if ( a_fld != "" )
	{
		if ( panelList.size() == 0 )
		{
			RC = 12 ;
			checkRCode( "No prior DISPLAY PANEL before ADDPOP" ) ;
			return ;
		}
		if ( !currPanel->field_get_row_col( a_fld, p_row, p_col ) )
		{
			RC = 20 ;
			checkRCode( "Field "+ a_fld +" not found or invalid on ADDPOP" ) ;
			return ;
		}
		a_row += p_row ;
		a_col += p_col - 4 ;
	}
	else
	{
		a_row += ds2d( p_poolMGR->get( RC, scrNum, "ZPROW" ) ) ;
		a_col += ds2d( p_poolMGR->get( RC, scrNum, "ZPCOL" ) ) ;
	}
	if ( addpop_active )
	{
		addpop_stk.push( addpop_row ) ;
		addpop_stk.push( addpop_col ) ;
	}

	addpop_active = true ;
	addpop_row = (a_row <  0 ) ? 1 : a_row + 2 ;
	addpop_col = (a_col < -1 ) ? 2 : a_col + 4 ;
	p_poolMGR->put( RC, scrNum, "ZPROW", d2ds( addpop_row ) ) ;
	p_poolMGR->put( RC, scrNum, "ZPCOL", d2ds( addpop_col ) ) ;
}


void pApplication::rempop( const string & r_all )
{
	//  Remove pop-up window.  Restore previous rempop() if there is one (push order row,col).

	//  RC = 0  Normal completion
	//  RC = 16 No pop-up window exists at this level
	//  RC = 20 Severe error

	int  scrNum ;

	RC = 0 ;

	if ( !addpop_active ) { RC = 16 ; checkRCode( "No pop-up window exists at this level" ) ; return ; }

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
	else if ( r_all == "ALL" )
	{
		while ( !addpop_stk.empty() )
		{
			addpop_stk.pop() ;
		}
		addpop_active = false ;
		addpop_row    = 0 ;
		addpop_col    = 0 ;
	}
	else { RC = 20 ; checkRCode( "Invalid parameter on REMPOP.  Must be ALL or blank" ) ; return ; }

	scrNum = ds2d(p_poolMGR->get( RC, "ZSCRNUM", SHARED ) ) ;
	p_poolMGR->put( RC, scrNum, "ZPROW", d2ds( addpop_row ) ) ;
	p_poolMGR->put( RC, scrNum, "ZPCOL", d2ds( addpop_col ) ) ;
}


void pApplication::movepop()
{
	if ( addpop_active )
	{
		currPanel->set_popup( addpop_row, addpop_col ) ;
		currPanel->move_popup() ;
	}
}


void pApplication::control( const string & parm1, const string & parm2, const string & parm3 )
{
	// CONTROL ERRORS CANCEL - abend for RC >= 12
	// CONTROL ERRORS RETURN - return to application for any RC

	// CONTROL DISPLAY SAVE/RESTORE - SAVE/RESTORE status for a TBDISPL
	//         SAVE/RESTORE saves/restores the six ZTD* variables in the function pool as well
	//         as the currtbPanel pointer for retrieving other model sets via tbdispl with no panel specified
	//         Only necessary if a tbdispl invokes another tbdispl in the same task

	// CONTROL SPLIT DISABLE - RC=8 if screen already split

	// CONTROL PASSTHRU LRSCROLL  PASON | PASOFF | PASQUERY

	// CONTROL REFLIST UPDATE
	// CONTROL REFLIST NOUPDATE

	// lspf extensions:
	// CONTROL TIMEOUT  ENABLE  - Enable application timeouts after ZWAITMAX ms (default).
	// CONTROL TIMEOUT  DISABLE - Disable forced abend of applications if ZWAITMAX exceeded.
	// CONTROL ABENDRTN DEFAULT - Reset abend routine to the default, pApplication::cleanup_default
	// CONTROL RDISPLAY FLUSH   - Flush raw output to the screen

	RC = 0 ;

	map<string, pPanel *>::iterator it;

	if ( parm3 != "" && parm1 != "PASSTHRU" )
	{
		RC = 20 ;
		checkRCode( "Error in control service" ) ;
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
			refresh() ;
		}
		else if ( parm2 == "SAVE" )
		{
			stk_str.push( ZTDMARK  ) ;
			stk_int.push( ZTDDEPTH ) ;
			stk_int.push( ZTDROWS  ) ;
			stk_int.push( ZTDSELS  ) ;
			stk_int.push( ZTDTOP   ) ;
			stk_int.push( ZTDVROWS ) ;
			SRpanelStack.push( currtbPanel ) ;
		}
		else if ( parm2 == "RESTORE" )
		{
			if ( SRpanelStack.empty() ) { ZERR2 = "No previous CONTROL ERRORS SAVE performed" ; RC = 20  ; }
			else
			{
				ZTDVROWS = stk_int.top() ;
				stk_int.pop() ;
				ZTDTOP   = stk_int.top() ;
				stk_int.pop() ;
				ZTDSELS  = stk_int.top() ;
				stk_int.pop() ;
				ZTDROWS  = stk_int.top() ;
				stk_int.pop() ;
				ZTDDEPTH = stk_int.top() ;
				stk_int.pop() ;
				ZTDMARK  = stk_str.top() ;
				stk_str.pop() ;
				currtbPanel = SRpanelStack.top() ;
				SRpanelStack.pop() ;
			}
		}
		else { RC = 20 ; }
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
		else { RC = 20 ; }
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
					it->second->LRScroll = true ;
				}
			}
			else if ( parm3 == "PASOFF" )
			{
				ControlPassLRScroll = false ;
				for ( it = panelList.begin() ; it != panelList.end() ; it++ )
				{
					it->second->LRScroll = false ;
				}
			}
			else if ( parm3 == "PASQUERY" )
			{
				RC = ControlPassLRScroll ? 1 : 0 ;
			}
			else { RC = 20 ; }
		}
		else { RC = 20 ; }
	}
	else if ( parm1 == "RDISPLAY" )
	{
		if ( parm2 == "FLUSH" )
		{
			rawOutput = true  ;
			wait_event()      ;
			rawOutput = false ;
		}
		else { RC = 20 ; }
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
		else { RC = 20 ; }
	}
	else if ( parm1 == "SPLIT" )
	{
		if ( parm2 == "ENABLE" )
		{
			ControlSplitEnable = true ;
		}
		else if ( parm2 == "DISABLE" )
		{
			if ( p_poolMGR->get( RC, "ZSPLIT", SHARED ) == "YES" )
			{
				RC = 8 ;
			}
			else
			{
				ControlSplitEnable = false ;
			}
		}
		else { RC = 20 ; }
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
		else { RC = 20 ; }
	}
	else if ( parm1 == "ABENDRTN" )
	{
		if ( parm2 == "DEFAULT" )
		{
			pcleanup = &pApplication::cleanup_default ;
		}
		else { RC = 20 ; }
	}
	else { RC = 20 ; }
	if ( RC > 4 ) { checkRCode( "Error in control service" ) ; }
}


void pApplication::control( const string & parm1, void (pApplication::*pFunc)() )
{
	// lspf extensions:
	// CONTROL ABENDRTN ptr_to_routine - Set the routine to get control during an abend

	RC = 0 ;

	if ( parm1 == "ABENDRTN" )
	{
		pcleanup = pFunc ;
	}
	else { RC = 20 ; }
	if ( RC > 0 ) { checkRCode( "Error in control service" ) ; }
}


void pApplication::tbadd( const string & tb_name, const string & tb_namelst, const string & tb_order, int tb_num_of_rows )
{
	// Add a row to a table

	// RC = 0   Normal completion
	// RC = 8   For keyed tables only, row already exists
	// RC = 12  Table not open
	// RC = 20  Severe error

	RC = 0 ;

	if ( tb_order  != "" && tb_order != "ORDER" )       { RC = 20 ; checkRCode( "Invalid ORDER parameter specified on TBADD" )          ; return ; }
	if ( tb_num_of_rows < 0 || tb_num_of_rows > 65535 ) { RC = 20 ; checkRCode( "Invalid number-of-rows parameter specified on TBADD" ) ; return ; }

	if ( !isTableUpdate( tb_name, "TBADD" ) ) { return ; }

	p_tableMGR->tbadd( RC, funcPOOL, tb_name, tb_namelst, tb_order, tb_num_of_rows ) ;
	if ( RC > 8 ) { checkRCode( "TBADD gave return code of "+ d2ds( RC ) ) ; }
}


void pApplication::tbbottom( const string & tb_name, const string & tb_savenm, const string & tb_rowid_vn, const string & tb_noread, const string & tb_crp_name )
{
	// Move row pointer to the bottom

	// RC = 0   Normal completion
	// RC = 8   Table is empty.  CRP is set to 0
	// RC = 12  Table not open
	// RC = 20  Severe error

	RC = 0 ;
	if ( !isTableOpen( tb_name, "TBBOTTOM" ) ) { return ; }

	p_tableMGR->tbbottom( RC, funcPOOL, tb_name, tb_savenm, tb_rowid_vn, tb_noread, tb_crp_name  ) ;
	if ( RC > 8 ) { checkRCode( "TBBOTTOM gave return code of "+ d2ds( RC ) ) ; }
}


void pApplication::tbclose( const string & tb_name, const string & tb_newname, const string & tb_path )
{
	// Save and close the table (calls saveTable and destroyTable routines).
	// Do not report error on tbclose() of a temporary table (pass false as last parm, to saveTable() in this case)
	// Error occurs only on tbsave() in this case

	// saveTable() returns RC4 if a table has not been changed (and so no save is performed).  Ignore in this case

	// RC = 0   Normal completion
	// RC = 12  Table not open
	// RC = 16  Path error
	// RC = 20  Severe error

	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBCLOSE" ) ) { return ; }

	if ( tablesUpdate.find( tb_name ) != tablesUpdate.end() )
	{
		p_tableMGR->saveTable( RC, taskid(), tb_name, tb_newname, tb_path, false ) ;
		if ( RC == 4 ) { RC = 0 ; }
	}
	if ( RC == 0 )
	{
		p_tableMGR->destroyTable( RC, taskid(), tb_name ) ;
		if ( RC == 0 )
		{
			tablesOpen.erase( tb_name ) ;
			tablesUpdate.erase( tb_name ) ;
		}
	}
	if ( RC > 8 ) { checkRCode( "TBCLOSE gave return code of "+ d2ds( RC ) ) ; }
}


void pApplication::tbcreate( const string & tb_name, const string & keys, const string & names, tbSAVE m_SAVE, tbREP m_REP, string m_path, tbDISP m_DISP )
{
	// Create a new table.  For permanent tables without a path specified, save to first path entry in ZTLIB

	// RC = 0   Normal completion
	// RC = 4   Normal completion - Table exists and REPLACE speified
	// RC = 8   Table exists and REPLACE not specified or REPLACE specified and opend in SHARE mode or REPLACE specified and opened in EXCLUSIVE mode but not the owning task
	// RC = 12  Table in use
	// RC = 16  ??
	// RC = 20  Severe error

	int ws ;
	bool temp ;

	string w ;

	RC   = 0    ;
	temp = true ;

	if ( !isvalidName( tb_name ) ) { RC = 20 ; checkRCode( "Invalid table name on TBCREATE '"+ tb_name +"'" ) ; return ; }
	if ( ( tablesOpen.find( tb_name ) != tablesOpen.end() ) && m_REP != REPLACE )
	{
		RC = 8  ;
		log( "E", "Table '" << tb_name << "' has already been created under this task and REPLACE not specified" << endl ) ;
		return  ;
	}

	if ( m_SAVE == WRITE )
	{
		temp = false ;
		if ( m_path == "" ) { m_path = getpath( ZTLIB, 1 ) ; }
	}

	ws = words( keys ) ;
	for ( int i = 1 ; i <= ws ; i++ )
	{
		w = word( keys, i ) ;
		if ( !isvalidName( w ) ) { RC = 20 ; checkRCode( "Invalid key name '"+ w +"'" ) ; return ; }
	}

	ws = words( names ) ;
	for ( int i = 1; i <= ws ; i++ )
	{
		w = word( names, i ) ;
		if ( !isvalidName( w ) ) { RC = 20 ; checkRCode( "Invalid field name '"+ w +"'" ) ; return ; }
	}

	p_tableMGR->createTable( RC, taskid(), tb_name, keys, names, temp, m_REP, m_path, m_DISP ) ;
	if ( RC > 8 ) { checkRCode( "TBCREATE gave return code of "+ d2ds( RC ) ) ; }

	if ( RC < 8 )
	{
		tablesOpen[ tb_name ]   = true ;
		tablesUpdate[ tb_name ] = true ;
	}
}


void pApplication::tbdelete( const string & tb_name )
{
	// Delete a row in the table.  For keyed tables, the table is searched with the current key.  For non-keyed tables the current CRP is used.

	// RC = 0   Normal completion
	// RC = 8   Row does not exist for a keyed table or for non-keyed table, CRP was at TOP(zero)
	// RC = 12  Table not open
	// RC = 20  Severe error

	RC = 0 ;
	if ( !isTableUpdate( tb_name, "TBDELETE" ) ) { return ; }

	p_tableMGR->tbdelete( RC, funcPOOL, tb_name ) ;
	if ( RC > 8 ) { checkRCode( "TBDELETE gave return code of "+ d2ds( RC ) ) ; }
}


void pApplication::tbdispl( const string & tb_name, string p_name, const string & p_msg, string p_cursor, int p_csrrow, int p_csrpos, string p_autosel, const string & p_crp_name, const string & p_rowid_nm )
{
	// tbdispl with panel, no message - clear previous pending lines, rebuild scrollable area and display panel
	// tbdispl with panel, message    - rebuild scrollable area and display panel and message
	// tbdispl no panel, no message   - retrieve next pending line.  If none, display panel
	// tbdispl no panel, message      - display panel with message.  No rebuilding of the scrollable area

	// Set CRP to first changed line, autoselected line, or tbtop if there are no selected lines

	// If .AUTOSEL and .CSRROW set in panel, override the parameters p_autosel and p_csrrow

	// Autoselect if the p_csrpos CRP is visible

	// Use separate pointer currtbPanel for tb displays so that a CONTROL DISPLAY SAVE/RESTORE is only necessary when a tbdispl issues another tbdispl and not
	// for a display of an ordinary panel

	int exitRC  ;
	int ws      ;
	int i       ;
	int ln      ;
	int posn    ;
	int csrvrow ;
	int scrNum  ;

	string ZZVERB ;
	string URID   ;
	string s      ;
	string t      ;

	RC = 0 ;
	ln = 0 ;

	if ( propagateEnd )
	{
		if ( p_name == "" )       { RC = 8 ; return      ; }
		if ( PPANELID == p_name ) { propagateEnd = false ;                }
		else                      { PPANELID = p_name ; RC = 8 ; return ; }
	}

	if ( !isTableOpen( tb_name, "TBDISPL" ) ) { return ; }
	if ( p_autosel == "" ) { p_autosel = "YES" ; }

	if ( p_cursor   != ""    && !isvalidName( p_cursor   ) ) { RC = 20 ; checkRCode( "Invalid CURSOR position" )     ; return ; }
	if ( p_autosel  != "YES" &&  p_autosel != "NO"         ) { RC = 20 ; checkRCode( "Invalid AUTOSEL parameter.  Must be YES or NO" ) ; return ; }
	if ( p_crp_name != ""    && !isvalidName( p_crp_name ) ) { RC = 20 ; checkRCode( "Invalid CRP variable name" )   ; return ; }
	if ( p_rowid_nm != ""    && !isvalidName( p_rowid_nm ) ) { RC = 20 ; checkRCode( "Invalid ROWID variable name" ) ; return ; }

	if ( p_name != "" )
	{
		createPanel( p_name ) ;
		if ( RC > 0 ) { checkRCode( "Panel '"+ p_name +"' error during TBDISPL" ) ; return ; }
		currtbPanel = panelList[ p_name ] ;
		currPanel   = currtbPanel         ;
		PANELID     = p_name ;
		p_poolMGR->put( RC, "ZPANELID", p_name, SHARED, SYSTEM ) ;
		if ( p_msg == "" )
		{
			currtbPanel->clear_tb_linesChanged( RC ) ;
			if ( RC > 0 ) { ZERR2 = currtbPanel->PERR1 ; checkRCode( "Panel '"+ p_name +"' error during TBDISPL" ) ; return ; }
		}
		posn = p_tableMGR->getCRP( RC, tb_name ) ;
		if ( posn == 0 ) { posn = 1 ; }
		p_tableMGR->fillfVARs( RC, funcPOOL, tb_name, currtbPanel->tb_depth, posn ) ;
	}
	else
	{
		p_poolMGR->put( RC, "ZVERB", "",  SHARED ) ;
		if ( p_msg == "" && currtbPanel->tb_lineChanged( ln, URID ) )
		{
			currtbPanel->remove_tb_lineChanged() ;
		}
		else
		{
			p_name    = currtbPanel->PANELID ;
			PANELID   = currtbPanel->PANELID ;
			currPanel = currtbPanel ;
			p_poolMGR->put( RC, "ZPANELID", p_name, SHARED, SYSTEM ) ;
		}
	}

	if ( setMSG )
	{
		if ( !ControlNonDispl && p_name != "" )
		{
			currtbPanel->set_panel_msg( MSG1, MSGID1 ) ;
			setMSG = false ;
		}
	}
	else if ( p_msg != "" )
	{
		get_Message( p_msg ) ;
		if ( RC > 0 ) { return ; }
		currtbPanel->set_panel_msg( MSG, MSGID ) ;
	}
	else
	{
		currtbPanel->clear_msg() ;
	}
	if ( p_cursor == "" ) { p_cursor = currtbPanel->Home ; }
	if ( addpop_active )  { currtbPanel->set_popup( addpop_row, addpop_col ) ; }
	else                  { currtbPanel->remove_popup()                      ; }

	currtbPanel->display_panel_init( RC ) ;
	if ( RC > 0 ) { ZERR2 = currtbPanel->PERR1 ; checkRCode( "Error processing )INIT section of panel "+ p_name ) ; return ; }

	currPanel->update_field_values( RC ) ;
	if ( RC > 0 ) { ZERR2 = currPanel->PERR1 ; checkRCode( "Error updating field values of panel "+ p_name ) ; return; }

	t = funcPOOL.get( RC, 8, ".AUTOSEL", NOCHECK ) ;
	if ( RC == 0 ) { p_autosel = t ; }
	t = funcPOOL.get( RC, 8, ".CSRROW", NOCHECK ) ;
	if ( RC == 0 ) { p_csrrow = ds2d( t ) ; }

	scrNum = ds2d(p_poolMGR->get( RC, "ZSCRNUM", SHARED ) ) ;
	if ( p_poolMGR->get( RC, "ZSCRNAM1", SHARED ) == "ON" &&
	     p_poolMGR->get( RC, scrNum, "ZSCRNAM2" ) == "PERM" )
	{
			p_poolMGR->put( RC, "ZSCRNAME", p_poolMGR->get( RC, scrNum, "ZSCRNAME" ), SHARED ) ;
	}

	while ( true )
	{
		if ( p_name != "" )
		{
			currtbPanel->display_panel( RC ) ;
			if ( currtbPanel->CURFLD == "" )
			{
				if ( wordpos( p_cursor, currtbPanel->tb_fields ) > 0 )
				{
					csrvrow = p_csrrow - ZTDTOP + 1 ;
					if ( csrvrow > 0 && csrvrow <= ZTDVROWS ) { i = csrvrow - 1 ; }
					else if ( ln > 0 ) { i = ln ; }
					else               { i = 1  ; }
					currtbPanel->CURFLD = p_cursor + "." + d2ds( i ) ;
				}
				else { currtbPanel->CURFLD = p_cursor ; }
				currtbPanel->CURPOS = p_csrpos ;
			}
			currtbPanel->cursor_to_field( RC ) ;
			if ( RC > 0 ) { checkRCode( "Cursor field '"+ currtbPanel->CURFLD +"' not found on panel or invalid for TBDISP" ) ; break ; }
			wait_event() ;
			ControlDisplayLock = false ;
			ControlNonDispl    = false ;
			currtbPanel->clear_msg() ;
			currtbPanel->CURFLD = "" ;
			currtbPanel->display_panel_update( RC ) ;
			currtbPanel->set_tb_linesChanged() ;
		}

		exitRC = 0  ;
		if ( currtbPanel->tb_lineChanged( ln, URID ) )
		{
			tbskip( tb_name, 0, "", p_rowid_nm, URID, "", p_crp_name ) ;
			ws = words( currtbPanel->tb_fields ) ;
			for ( i = 1 ; i <= ws ; i++ )
			{
				s = word( currtbPanel->tb_fields, i ) ;
				funcPOOL.put( RC, 0, s, funcPOOL.get( RC, 0, s + "." + d2ds( ln ), NOCHECK ) ) ;
			}
			if ( ZTDSELS > 1 ) { exitRC = 4; }
		}

		currtbPanel->display_panel_proc( RC, ln ) ;
		if ( RC > 0 ) { ZERR2 = currtbPanel->PERR1 ; checkRCode( "Error processing )PROC section of panel "+ p_name ) ; return ; }

		ZZVERB = p_poolMGR->get( RC, "ZVERB" ) ;
		if ( RC > 0 ) { RC = 20 ; checkRCode( "PoolMGR get of ZVERB failed" ) ; }
		if ( ZZVERB == "RETURN" )                    { propagateEnd = true ; }
		if ( findword( ZZVERB, "END EXIT RETURN" ) ) { RC = 8 ; return     ; }

		t = funcPOOL.get( RC, 8, ".CSRROW", NOCHECK ) ;
		if ( RC == 0 ) { p_csrrow = ds2d( t ) ; }
		if ( currtbPanel->MSGID != "" )
		{
			get_Message( currtbPanel->MSGID ) ;
			if ( RC > 0 ) { return ; }
			currtbPanel->set_panel_msg( MSG, MSGID ) ;
			if ( p_name == "" )
			{
				p_name    = currtbPanel->PANELID ;
				currPanel = currtbPanel ;
				PANELID   = p_name      ;
				p_poolMGR->put( RC, "ZPANELID", p_name, SHARED, SYSTEM ) ;
			}
			currtbPanel->display_panel_reinit( RC, ln ) ;
			if ( RC > 0 ) { ZERR2 = currtbPanel->PERR1 ; checkRCode( "Error processing )REINIT section of panel "+ p_name ) ; break ; }
			t = funcPOOL.get( RC, 8, ".AUTOSEL", NOCHECK ) ;
			if ( RC == 0 ) { p_autosel = t ; }
			t = funcPOOL.get( RC, 8, ".CSRROW", NOCHECK ) ;
			if ( RC == 0 ) { p_csrrow = ds2d( t ) ; }
			continue ;
		}
		if ( ZZVERB == "UP" || ZZVERB == "DOWN" )
		{
			if ( ZZVERB == "UP" )
			{
				ZSCROLLA = p_poolMGR->get( RC, "ZSCROLLA", SHARED ) ;
				if ( ZSCROLLA != "MAX" )
				{
					ZSCROLLN = ds2d( p_poolMGR->get( RC, "ZSCROLLN", SHARED ) ) ;
					ZTDTOP > ZSCROLLN ? ( ZTDTOP = ZTDTOP - ZSCROLLN ) : ZTDTOP = 1 ;
				}
				else { ZTDTOP = 1 ; }
				ZCMD = "" ;
			}
			else
			{
				ZSCROLLA = p_poolMGR->get( RC, "ZSCROLLA", SHARED ) ;
				if ( ZSCROLLA == "MAX" )
				{
					ZTDTOP = ZTDROWS + 1 ;
				}
				else
				{
					ZSCROLLN = ds2d( p_poolMGR->get( RC, "ZSCROLLN", SHARED ) ) ;
					ZSCROLLN + ZTDTOP > ZTDROWS ? ( ZTDTOP = ZTDROWS+1 ) : ZTDTOP = ZTDTOP + ZSCROLLN ;
				}
				ZCMD = "" ;
			}
			p_tableMGR->fillfVARs( RC, funcPOOL, tb_name, currtbPanel->tb_depth, ZTDTOP ) ;
			currPanel->update_field_values( RC ) ;
			if ( RC > 0 ) { ZERR2 = currPanel->PERR1 ; checkRCode( "Error updating field values of panel "+ p_name ) ; return ; }
			continue ;
		}
		break ;
	}
	if ( URID == "" )
	{
		csrvrow = p_csrrow - ZTDTOP + 1 ;
		if ( p_autosel == "YES" && csrvrow > 0 && csrvrow <= ZTDVROWS )
		{
			tbtop( tb_name ) ;
			tbskip( tb_name, p_csrrow, "", p_rowid_nm, "", "", p_crp_name ) ;
			ws = words( currtbPanel->tb_fields ) ;
			for ( i = 1 ; i <= ws ; i++ )
			{
				s = word( currtbPanel->tb_fields, i ) ;
				funcPOOL.put( RC, 0, s, funcPOOL.get( RC, 0, s + "." + d2ds( csrvrow-1 ), NOCHECK ) ) ;
			}
			ZTDSELS++ ;
		}
		else
		{
			tbtop( tb_name ) ;
			if ( p_crp_name != "" ) { funcPOOL.put( RC, 0, p_crp_name, 0  ) ; }
			if ( p_rowid_nm != "" ) { funcPOOL.put( RC, 0, p_rowid_nm, "" ) ; }
		}
	}
	currtbPanel->resetAttrs() ;
	RC = exitRC ;
}


void pApplication::tbend( const string & tb_name )
{
	// Close a table without saving (calls destroyTable routine).  If opened share, use count is reduced and table deleted when use count = 0

	// RC = 0   Normal completion
	// RC = 12  Table not open
	// RC = 20  Severe error

	RC = 0 ;
	if ( !isTableOpen( tb_name, "TBEND" ) ) { return ; }

	p_tableMGR->destroyTable( RC, taskid(), tb_name ) ;
	if ( RC == 0 )
	{
		tablesOpen.erase( tb_name ) ;
		tablesUpdate.erase( tb_name ) ;
	}
	if ( RC > 8 ) { checkRCode( "TBEND gave return code of "+ d2ds( RC ) ) ; }
}


void pApplication::tberase( const string & tb_name, string tb_path )
{
	// Erase a table file from path

	// RC = 0   Normal completion
	// RC = 8   Table does not exist
	// RC = 12  Table in use
	// RC = 16  Path does not exist
	// RC = 20  Severe error

	RC = 0 ;
	if ( !isvalidName( tb_name ) ) { RC = 20 ; checkRCode( "Invalid table name on TBERASE" ) ; return ; }

	if ( tb_path == "" ) { tb_path = ZTLIB ; }
	p_tableMGR->tberase( RC, tb_name, tb_path ) ;
	if ( RC > 8 ) { checkRCode( "TBERASE gave return code of "+ d2ds( RC ) ) ; }
}


void pApplication::tbexist( const string & tb_name )
{
	// Test for the existance of a row in a keyed table

	// RC = 0   Normal completion
	// RC = 8   Row does not exist or not a keyed table
	// RC = 12  Table not open
	// RC = 20  Severe error

	RC = 0 ;
	if ( !isTableOpen( tb_name, "TBEXIST" ) ) { return ; }

	p_tableMGR->tbexist( RC, funcPOOL, tb_name ) ;
	if ( RC > 8 ) { checkRCode( "TBEXISTS gave return code of "+ d2ds( RC ) ) ; }
}



void pApplication::tbget( const string & tb_name, const string & tb_savenm, const string & tb_rowid_vn, const string & tb_noread, const string & tb_crp_name )
{
	RC = 0 ;
	if ( !isTableOpen( tb_name, "TBGET" ) ) { return ; }

	p_tableMGR->tbget( RC, funcPOOL, tb_name, tb_savenm, tb_rowid_vn, tb_noread, tb_crp_name  ) ;
	if ( RC > 8 ) { checkRCode( "TBGET gave return code of "+ d2ds( RC ) ) ; }
}


void pApplication::tbmod( const string & tb_name, const string & tb_namelst, const string & tb_order )
{
	// Update a row in a table

	RC = 0 ;

	if ( tb_order  != "" && tb_order != "ORDER" ) { RC = 20 ; checkRCode( "Invalid ORDER parameter specified on TBMOD" ) ; return ; }

	if ( !isTableUpdate( tb_name, "TBMOD" ) ) { return ; }

	p_tableMGR->tbmod( RC, funcPOOL, tb_name, tb_namelst, tb_order ) ;
	if ( RC > 8 ) { checkRCode( "TBMOD gave return code of "+ d2ds( RC ) ) ; }
}


void pApplication::tbopen( const string & tb_name, tbSAVE m_SAVE, string m_paths, tbDISP m_DISP )
{
	// Open an existing table, reading it from a file.  If aleady opened in SHARE/NOWRITE, increment use count

	// RC = 0   Normal completion
	// RC = 8   Table does not exist in search path
	// RC = 12  Table already open by this or another task
	// RC = 16  path does not exist
	// RC = 20  Severe error

	RC = 0 ;

	if ( !isvalidName( tb_name ) ) { RC = 20 ; checkRCode( "Invalid table name on TBOPEN" ) ; return ; }

	if ( tablesOpen.find( tb_name ) != tablesOpen.end() )
	{
		RC = 12 ;
		checkRCode( "Table "+ tb_name +" already open on TBOPEN" ) ;
		return  ;
	}

	if ( m_paths == "" )
	{
		if ( libdef_tuser ) { m_paths = mergepaths( ZTUSER, ZTLIB ) ; }
		else                { m_paths = ZTLIB                       ; }
	}

	p_tableMGR->loadTable( RC, taskid(), tb_name, m_DISP, m_paths ) ;

	if ( RC > 8 ) { checkRCode( "TBOPEN gave return code of "+ d2ds( RC ) ) ; }
	if ( RC < 8 )
	{
		tablesOpen[ tb_name ] = true ;
		if ( m_SAVE == WRITE )
		{
			tablesUpdate[ tb_name ] = true ;
		}
	}
}


void pApplication::tbput( const string & tb_name, const string & tb_namelst, const string & tb_order )
{
	// Update the current row in a table

	// RC = 0   Normal completion
	// RC = 8   Keyed tables - key does not match current row.  CRP set to top (0)
	//          Non-keyed tables - CRP at top
	// RC = 12  Table not open
	// RC = 16  Numeric conversion error for sorted tables
	// RC = 20  Severe error

	RC = 0 ;

	if ( tb_order  != "" && tb_order != "ORDER" ) { RC = 20 ; checkRCode( "Invalid ORDER parameter specified on TBPUT" ) ; return ; }

	if ( !isTableUpdate( tb_name, "TBPUT" ) ) { return ; }

	p_tableMGR->tbput( RC, funcPOOL, tb_name, tb_namelst, tb_order ) ;
	if ( RC > 8 ) { checkRCode( "TBPUT gave return code of "+ d2ds( RC ) ) ; }
}


void pApplication::tbquery( const string & tb_name, const string & tb_keyn, const string & tb_varn, const string & tb_rownn, const string & tb_keynn, const string & tb_namenn, const string & tb_crpn, const string & tb_sirn, const string & tb_lstn, const string & tb_condn, const string & tb_dirn )
{
	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBQUERY" ) ) { return ; }

	p_tableMGR->tbquery( RC, funcPOOL, tb_name, tb_keyn, tb_varn, tb_rownn, tb_keynn, tb_namenn, tb_crpn, tb_sirn, tb_lstn, tb_condn, tb_dirn ) ;
	if ( RC > 8 ) { checkRCode( "TBQUERY gave return code of "+ d2ds( RC ) ) ; }
}


void pApplication::tbsarg( const string & tb_name, const string & tb_namelst, const string & tb_dir, const string & tb_cond_pairs )
{
	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBSARG" ) ) { return ; }

	p_tableMGR->tbsarg( RC, funcPOOL, tb_name, tb_namelst, tb_dir, tb_cond_pairs ) ;
	if ( RC > 8 ) { checkRCode( "TBSARG gave return code of "+ d2ds( RC ) ) ; }
}


void pApplication::tbsave( const string & tb_name, const string & tb_newname, const string & path )
{
	// Save the table to disk (calls saveTable routine).  Table remains open for processing.  Table must have the WRITE attribute
	// saveTable() returns RC4 if a table has not been changed (and so no save is performed).  Ignore in this case

	// RC = 0   Normal completion
	// RC = 12  Table not open or not open WRITE
	// RC = 16  Alternate name save error
	// RC = 20  Severe error

	RC = 0 ;

	if ( !isTableUpdate( tb_name, "TBSAVE" ) ) { return ; }

	p_tableMGR->saveTable( RC, taskid(), tb_name, tb_newname, path ) ;
	if ( RC == 4 ) { RC = 0 ; }
	if ( RC > 8 ) { checkRCode( "TBSAVE gave return code of "+ d2ds( RC ) ) ; }
}


void pApplication::tbscan( const string & tb_name, const string & tb_namelst, const string & tb_savenm, const string & tb_rowid_vn, const string & tb_dir, const string & tb_read, const string & tb_crp_name, const string & tb_condlst )
{
	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBSCAN" ) ) { return ; }

	p_tableMGR->tbscan( RC, funcPOOL, tb_name, tb_namelst, tb_savenm, tb_rowid_vn, tb_dir, tb_read, tb_crp_name, tb_condlst ) ;
	if ( RC > 8 ) { checkRCode( "TBSCAN gave return code of "+ d2ds( RC ) ) ; }
}



void pApplication::tbskip( const string & tb_name, int num, const string & tb_savenm, const string & tb_rowid_vn, const string & tb_rowid, const string & tb_noread, const string & tb_crp_name )
{
	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBSKIP" ) ) { return ; }

	p_tableMGR->tbskip( RC, funcPOOL, tb_name, num, tb_savenm, tb_rowid_vn, tb_rowid, tb_noread, tb_crp_name ) ;
	if ( RC > 8 ) { checkRCode( "TBSKIP gave return code of "+ d2ds( RC ) ) ; }
}


void pApplication::tbsort( const string & tb_name, const string & tb_fields )
{
	RC = 0 ;

	if ( !isTableUpdate( tb_name, "TBSORT" ) ) { return ; }

	p_tableMGR->tbsort( RC, tb_name, tb_fields ) ;
	if ( RC > 8 ) { checkRCode( "TBSORT gave return code of "+ d2ds( RC ) ) ; }
}



void pApplication::tbtop( const string & tb_name )
{
	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBTOP" ) ) { return ; }

	p_tableMGR->tbtop( RC, tb_name ) ;
	if ( RC > 8 ) { checkRCode( "TBTOP gave return code of "+ d2ds( RC ) ) ; }
}


void pApplication::tbvclear( const string & tb_name )
{
	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBVCLEAR" ) ) { return ; }

	p_tableMGR->tbvclear( RC, funcPOOL, tb_name ) ;
	if ( RC > 8 ) { checkRCode( "TBVCLEAR gave return code of "+ d2ds( RC ) ) ; }
}


bool pApplication::isTableOpen( const string & tb_name, const string & func )
{

	RC = 0 ;

	if ( !isvalidName( tb_name ) ) { RC = 20 ; checkRCode( "Invalid table name on "+ func +" '"+ tb_name +"'" ) ; return false ; }
	if ( tablesOpen.find( tb_name ) == tablesOpen.end() )
	{
		RC = 12      ;
		checkRCode( "Table "+ tb_name +" not open on "+ func ) ;
		return false ;
	}
	return true ;
}


bool pApplication::isTableUpdate( const string & tb_name, const string & func )
{

	RC = 0 ;

	if ( !isvalidName( tb_name ) ) { RC = 20 ; checkRCode( "Invalid table name on "+ func +" "+ tb_name ) ; return false ; }
	if ( tablesOpen.find( tb_name ) == tablesOpen.end() )
	{
		RC = 12      ;
		checkRCode( "Table "+ tb_name +" not open on "+ func ) ;
		return false ;
	}
	if ( tablesUpdate.find( tb_name ) == tablesUpdate.end() )
	{
		RC = 12      ;
		checkRCode( "Table "+ tb_name +" open in read-only mode on "+ func ) ;
		return false ;
	}
	return true ;
}


void pApplication::edit( const string & m_file, const string & m_panel )
{
	SELCT.clear() ;
	SELCT.PGM     = p_poolMGR->get( RC, "ZEDITPGM", PROFILE ) ;
	SELCT.PARM    = "FILE("+ m_file +") PANEL("+ m_panel +")" ;
	SELCT.NEWAPPL = ""      ;
	SELCT.NEWPOOL = false   ;
	SELCT.PASSLIB = false   ;
	SELCT.SUSPEND = true    ;
	SELCT.SCRNAME = "EDIT"  ;
	actionSelect() ;
}


void pApplication::browse( const string & m_file, const string & m_panel )
{
	SELCT.clear() ;
	SELCT.PGM     = p_poolMGR->get( RC, "ZBRPGM", PROFILE ) ;
	SELCT.PARM    = "FILE("+ m_file +") PANEL("+ m_panel +")" ;
	SELCT.NEWAPPL = ""       ;
	SELCT.NEWPOOL = false    ;
	SELCT.PASSLIB = false    ;
	SELCT.SUSPEND = true     ;
	SELCT.SCRNAME = "BROWSE" ;
	actionSelect() ;
}


void pApplication::view( const string & m_file, const string & m_panel )
{
	SELCT.clear() ;
	SELCT.PGM     = p_poolMGR->get( RC, "ZVIEWPGM", PROFILE ) ;
	SELCT.PARM    = "FILE("+ m_file +") PANEL("+ m_panel +")" ;
	SELCT.NEWAPPL = ""      ;
	SELCT.NEWPOOL = false   ;
	SELCT.PASSLIB = false   ;
	SELCT.SUSPEND = true    ;
	SELCT.SCRNAME = "VIEW"  ;
	actionSelect() ;
}


void pApplication::select( const string & cmd )
{
	// SELECT a function or panel in keyword format for use in applications,
	// ie PGM(abc) CMD(oorexx) PANEL(xxx) PARM(zzz) NEWAPPL PASSLIB SCRNAME(abc) etc.

	// No variable substitution is done at this level.

	if ( !SELCT.parse( cmd ) )
	{
		RC = 20 ;
		checkRCode( "Error in SELECT commmand "+ cmd ) ;
		return ;
	}
	actionSelect() ;
}


void pApplication::select( selobj sel )
{
	// SELECT a function or panel using a SELECT object (internal use only)

	SELCT = sel    ;
	actionSelect() ;
}


void pApplication::actionSelect()
{
	// RC=0  Normal completion of the selection panel or function.  END was entered.
	// RC=4  Normal completion.  RETURN was entered or EXIT specified on the selection panel

	SEL      = true  ;
	busyAppl = false ;

	wait_event()    ;
	debug1( "SELECT returned.  RC=" << RC << endl ) ;

	if ( RC == 4 ) { propagateEnd = true ; }

	SEL = false   ;
	SELCT.clear() ;
}


void pApplication::pquery( const string & p_name, const string & a_name, const string & t_name, const string & w_name, const string & d_name, const string & r_name, const string & c_name )
{
	RC = 0 ;

	if ( !isvalidName( p_name ) ) { RC = 20 ; checkRCode( "Invalid panel name '"+ p_name +"' on PQUERY" ) ; return ; }
	if ( !isvalidName( a_name ) ) { RC = 20 ; checkRCode( "Invalid area name '"+ a_name +"' on PQUERY" ) ; return ; }

	if ( (t_name != "") && !isvalidName( t_name ) ) { RC = 20 ; checkRCode( "Invalid area type name '"+ t_name +"' on PQUERY" ) ; return ; }
	if ( (w_name != "") && !isvalidName( w_name ) ) { RC = 20 ; checkRCode( "Invalid area width name '"+ w_name +"' on PQUERY" ) ; return ; }
	if ( (d_name != "") && !isvalidName( d_name ) ) { RC = 20 ; checkRCode( "Invalid area depth name '"+ d_name +"' on PQUERY" ) ; return ; }
	if ( (r_name != "") && !isvalidName( r_name ) ) { RC = 20 ; checkRCode( "Invalid area row number name '"+ r_name +"' on PQUERY" ) ; return ; }
	if ( (c_name != "") && !isvalidName( c_name ) ) { RC = 20 ; checkRCode( "Invalid area column number name '"+ c_name +"' on PQUERY" ) ; return ; }

	createPanel( p_name ) ;
	if ( panelList.count( p_name ) == 0 )
	{
		RC = 20 ;
		checkRCode( "Panel '"+ p_name +"' error during PQUERY" ) ;
		return  ;
	}
	panelList[ p_name ]->get_panel_info( RC, a_name, t_name, w_name, d_name, r_name, c_name ) ;
}


void pApplication::attr( const string & field, const string & attrs )
{
	if ( PANELID == "" )
	{
		RC = 20 ;
		checkRCode( "No panel has yet been loaded to change field attributes" ) ;
		return  ;
	}
	panelList[ PANELID ]->attr( RC, field, attrs ) ;
}


void pApplication::reload_keylist( pPanel * p )
{
	// Does an unconditional reload every time, but need to find a way to detect a change (TODO)
	// Alternatively, don't preload pfkeys into the panel object but pass back requested key from pApplication.

	load_keylist( p ) ;
}


void pApplication::load_keylist( pPanel * p )
{
	string tabName  ;
	string tabField ;

	string UPROF    ;
	string kerr     ;

	bool   klfail   ;

	if ( p->KEYLISTN == "" ) { return ; }

	tabName = p->KEYAPPL + "KEYP" ;
	klfail  = ( p_poolMGR->get( RC, "ZKLFAIL", PROFILE ) == "Y" ) ;

	vcopy( "ZUPROF", UPROF, MOVE ) ;
	tbopen( tabName, NOWRITE, UPROF, SHARE ) ;
	if ( RC  > 0 )
	{
		kerr = "Open of keylist table '"+ tabName +"' failed" ;
		if ( !klfail ) { RC = 0 ; log( "W", kerr << endl ) ; return ; }
		RC = 20 ;
		checkRCode( kerr ) ;
	}

	tbvclear( tabName ) ;
	vreplace( "KEYLISTN", p->KEYLISTN ) ;
	tbget( tabName ) ;
	if ( RC > 0 )
	{
		tbend( tabName ) ;
		kerr = "Keylist '"+ p->KEYLISTN +"' not found in keylist table "+ tabName ;
		if ( !klfail ) { RC = 0 ; log( "W", kerr << endl ) ; return ; }
		RC = 20 ;
		checkRCode( "Keylist '"+ p->KEYLISTN +"' not found in keylist table "+ tabName ) ;
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
	vcopy( "KEYHELPN", p->KEYHELPN, MOVE ) ;

	tbend( tabName ) ;
}


void pApplication::rdisplay( const string & msg )
{
	RC = 0 ;
	rmsgs.push_back( sub_vars( msg ) ) ;
	if ( rmsgs.size() == ds2d( p_poolMGR->get( RC, "ZSCRMAXD", SHARED ) ) - 1 )
	{
		rawOutput = true  ;
		wait_event()      ;
		rawOutput = false ;
	}
}


void pApplication::rdisplay1( const string & msg )
{
	// Same as RDISPLAY, except no variable substitution is performed.  It is used in the
	// ISPEXEC interface which does the substitution itself.

	RC = 0 ;
	rmsgs.push_back( msg ) ;
	if ( rmsgs.size() == ds2d( p_poolMGR->get( RC, "ZSCRMAXD", SHARED ) ) - 1 )
	{
		rawOutput = true  ;
		wait_event()      ;
		rawOutput = false ;
	}
}


void pApplication::setmsg( const string & msg, msgSET sType )
{
	// Retrieve message and store in MSG1 and MSGID1.

	RC = 0 ;

	if ( ( sType == COND ) && setMSG ) { return ; }

	get_Message( msg ) ;
	if ( RC > 0 ) { RC = 20 ; return ; }
	MSG1   = MSG   ;
	MSGID1 = msg   ;
	setMSG = true  ;
}


void pApplication::getmsg( const string & msg, const string & smsg, const string & lmsg, const string & alm, const string & hlp, const string & typ, const string & wndo )
{
	// Load message msg and substitute variables

	slmsg tmsg ;

	RC = 0 ;

	if ( smsg != "" && !isvalidName( smsg ) ) { RC = 20 ; checkRCode( "Invalid SHORT MESSAGE variable name" ) ; return ; }
	if ( lmsg != "" && !isvalidName( lmsg ) ) { RC = 20 ; checkRCode( "Invalid LONG MESSAGE variable name" )  ; return ; }
	if ( alm  != "" && !isvalidName( alm  ) ) { RC = 20 ; checkRCode( "Invalid ALARM variable name" )         ; return ; }
	if ( hlp  != "" && !isvalidName( hlp  ) ) { RC = 20 ; checkRCode( "Invalid HELP variable name" )          ; return ; }
	if ( typ  != "" && !isvalidName( typ  ) ) { RC = 20 ; checkRCode( "Invalid TYPE variable name" )          ; return ; }
	if ( wndo != "" && !isvalidName( wndo ) ) { RC = 20 ; checkRCode( "Invalid WINDOW variable name" )        ; return ; }

	if ( !load_Message( msg ) ) { return ; }

	tmsg      = msgList[ msg ] ;
	tmsg.smsg = sub_vars( tmsg.smsg ) ;
	tmsg.lmsg = sub_vars( tmsg.lmsg ) ;

	if ( !sub_Message_vars( tmsg ) )
	{
		RC = 20 ;
		checkRCode( "Invalid variable value" ) ;
		return ;
	}

	if ( smsg != "" ) { funcPOOL.put( RC, 0, smsg, tmsg.smsg ) ; }
	if ( lmsg != "" ) { funcPOOL.put( RC, 0, lmsg, tmsg.lmsg ) ; }
	if (  alm != "" )
	{
		if ( tmsg.alm ) { funcPOOL.put( RC, 0, alm, "YES" ) ; }
		else            { funcPOOL.put( RC, 0, alm, "NO" )  ; }
	}
	if (  typ != "" )
	{
		switch ( tmsg.type )
		{
			case IMT: funcPOOL.put( RC, 0, typ, "NOTIFY" )   ; break ;
			case WMT: funcPOOL.put( RC, 0, typ, "WARNING" )  ; break ;
			case AMT: funcPOOL.put( RC, 0, typ, "CRITICAL" ) ;
		}
	}
	if ( hlp  != "" ) { funcPOOL.put( RC, 0, hlp, tmsg.hlp ) ; }
	if ( wndo != "" )
	{
		if   ( tmsg.resp ) { funcPOOL.put( RC, 0, wndo, "RESP" )   ; }
		else               { funcPOOL.put( RC, 0, wndo, "NORESP" ) ; }
	}
}


string pApplication::get_help_member( int row, int col )
{
	string fld ;
	string paths ;

	RC = 0 ;

	fld = currPanel->field_getname( row, col ) ;

	if ( libdef_puser ) { paths = mergepaths( ZPUSER, ZPLIB ) ; }
	else                { paths = ZPLIB                       ; }

	return "M("+ MSG.hlp+ ") " +
	       "F("+ currPanel->get_field_help( fld )+ ") " +
	       "P("+ currPanel->ZPHELP +") " +
	       "A("+ ZAHELP +") " +
	       "K("+ currPanel->KEYHELPN +") "+
	       "PATHS("+paths+")" ;
}


void pApplication::get_Message( const string & p_msg )
{
	// Load messages from message library and copy slmsg object to MSG for the message requested

	// Substitute any dialogue variables in the short and long messages
	// Substitute any dialogue varibles in .TYPE, .WINDOW, .HELP and .ALARM parameters
	// Set MSGID

	RC = 0 ;

	if ( !load_Message( p_msg ) ) { return ; }

	MSG      = msgList[ p_msg ]     ;
	MSG.smsg = sub_vars( MSG.smsg ) ;
	MSG.lmsg = sub_vars( MSG.lmsg ) ;

	if ( !sub_Message_vars( MSG ) ) { return ; }

	MSGID    = p_msg ;
}


bool pApplication::load_Message( const string & p_msg )
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

	int i  ;
	int j  ;
	int p1 ;

	string p_msg_fn ;
	string filename ;
	string mline    ;
	string paths    ;
	string tmp      ;
	string msgid    ;

	bool found      ;
	bool lcontinue  ;

	map<string,bool>MMsgs ;

	slmsg t ;

	if ( !testMode && msgList.count( p_msg ) > 0 ) { return true ; }

	i = chk_Message_id( p_msg ) ;

	if ( i == 0 || ((p_msg.size() - i) > 3 && !isalpha( p_msg.back()) ) )
	{
		RC = 20 ;
		checkRCode( "Message-id format invalid: "+ p_msg ) ;
		return false ;
	}

	p_msg_fn = p_msg.substr( 0, i+2 ) ;

	if ( libdef_muser ) { paths = mergepaths( ZMUSER, ZMLIB ) ; }
	else                { paths = ZMLIB                       ; }

	found = false ;
	i = getpaths( paths ) ;
	for ( j = 1 ; j <= i ; j++ )
	{
		filename = getpath( paths, j ) + p_msg_fn ;
		if ( exists( filename ) )
		{
			if ( !is_regular_file( filename ) )
			{
				RC = 20 ;
				checkRCode( "Message file "+ filename +" is not a regular file" ) ;
				return false ;
			}
			else
			{
				found = true ;
				break        ;
			}
		}
	}
	if ( !found )
	{
		RC = 12 ;
		checkRCode( "Message file "+ p_msg_fn +" not found in ZMLIB for message-id "+ p_msg ) ;
		return false ;
	}

	msgid  = ""    ;
	t.smsg = ""    ;
	t.lmsg = ""    ;
	t.cont = false ;

	std::ifstream messages( filename.c_str() ) ;
	if ( !messages.is_open() )
	{
		RC = 20 ;
		checkRCode( "Error opening message file "+ filename ) ;
		return false ;
	}
	while ( getline( messages, mline ) )
	{
		mline = strip( mline ) ;
		if ( mline == "" || mline[ 0 ] == '*' ) { continue ; }
		if ( mline.compare( 0, 2, "/*" ) == 0 ) { continue ; }
		if ( mline.compare( 0, p_msg_fn.size(), p_msg_fn ) == 0 )
		{
			if ( msgid != "" )
			{
				if ( t.cont || !parse_Message( t ) )
				{
					RC = 20 ;
					messages.close() ;
					checkRCode( "Error in message-id "+ msgid ) ;
					return false ;
				}
				if ( MMsgs.count( msgid ) > 0 )
				{
					RC = 20 ;
					messages.close() ;
					checkRCode( "Duplicate message-id found: "+ msgid ) ;
					return false ;
				}
				msgList[ msgid ] = t    ;
				MMsgs[ msgid ]   = true ;
			}
			msgid  = word( mline, 1 )    ;
			t.smsg = subword( mline, 2 ) ;
			t.lmsg = "" ;
			i = chk_Message_id( msgid ) ;
			if ( i == 0 || ((msgid.size() - i) > 3 && !isalpha( msgid.back()) ) )
			{
				RC = 20 ;
				checkRCode( "Message-id format invalid: "+ msgid ) ;
				return false ;
			}
		}
		else
		{
			if ( msgid == "" || ( t.lmsg != "" && !t.cont ) )
			{
				RC = 20 ;
				messages.close() ;
				checkRCode( "Extraeneous data: "+ mline ) ;
				return false ;
			}
			lcontinue = false ;
			if ( mline.back() == '+' )
			{
				mline.erase( mline.size()-1 ) ;
				mline = strip( mline )        ;
				lcontinue = true ;
			}
			if ( mline[ 0 ] == '\'' || mline[ 0 ] == '"' )
			{
				p1 = mline.find_first_of( mline[ 0 ], 1 ) ;
				if ( p1 == string::npos || p1 != mline.size() - 1 )
				{
					RC = 20 ;
					messages.close() ;
					checkRCode( "Error in message-id "+ msgid ) ;
					return false ;
				}
				tmp = mline.substr( 1, p1-1 ) ;
			}
			else
			{
				if ( words( mline ) > 1 )
				{
					RC = 20 ;
					messages.close() ;
					checkRCode( "Error in message-id "+ msgid ) ;
					return false ;
				}
				tmp = mline ;
			}
			t.cont ? t.lmsg = t.lmsg + " " + tmp : t.lmsg = tmp ;
			t.cont = lcontinue ;
		}
	}
	if ( messages.bad() )
	{
		RC = 20 ;
		messages.close() ;
		checkRCode( "Error while reading message file "+ filename ) ;
		return false ;
	}
	messages.close() ;

	if ( t.smsg != "" )
	{
		if ( t.cont || !parse_Message( t ) )
		{
			RC = 20 ;
			messages.close() ;
			checkRCode( "Error in message-id "+ msgid ) ;
			return false ;
		}
		if ( MMsgs.count( msgid ) > 0 )
		{
			RC = 20 ;
			messages.close() ;
			checkRCode( "Duplicate message-id found: "+ msgid ) ;
			return false ;
		}
		msgList[ msgid ] = t ;
	}

	if ( msgList.count( p_msg ) == 0 )
	{
		RC = 12 ;
		checkRCode( "Message-id "+ p_msg +" not found in message file "+ p_msg_fn ) ;
		return false ;
	}
	return true ;
}


bool pApplication::parse_Message( slmsg & t )
{
	// Parse message and fill the slmsg object.  Long message already processed at this stage.

	// .TYPE overrides .WINDOW and .ALARM

	int p1 ;
	int p2 ;
	int ln ;

	string rest ;
	string tmp  ;

	t.hlp   = ""    ;
	t.type  = IMT   ;
	t.smwin = false ;
	t.lmwin = false ;
	t.resp  = false ;
	t.alm   = false ;

	if ( t.smsg[ 0 ] == '\'' || t.smsg[ 0 ] == '"' )
	{
		p1 = t.smsg.find_first_of( t.smsg[ 0 ], 1 ) ;
		if ( p1 == string::npos ) { return false ; }
		rest   = substr( t.smsg, p1+2 )   ;
		t.smsg = t.smsg.substr( 1, p1-1 ) ;
	}
	else
	{
		if ( t.smsg[ 0 ] == '.' )
		{
			rest   = t.smsg ;
			t.smsg = ""     ;
		}
		else
		{
			rest   = subword( t.smsg, 2 ) ;
			t.smsg = word( t.smsg, 1 )    ;
		}
	}

	p1 = pos( ".HELP=", rest ) ;
	if ( p1 == 0 )
	{
		p1 = pos( ".H=", rest ) ;
		ln = 3 ;
	}
	else
	{
		ln = 6 ;
	}
	if ( p1 > 0 )
	{
		p2 = pos( " ", rest, p1 ) ;
		if ( p2 == 0 ) { t.hlp = substr( rest, p1+ln )           ; rest = delstr( rest, p1 )        ; }
		else           { t.hlp = substr( rest, p1+ln, p2-p1-ln ) ; rest = delstr( rest, p1, p2-p1 ) ; }
		if ( t.hlp.size() == 0 ) { return false ; }
		if ( t.hlp[ 0 ] == '&')
		{
			t.hlp.erase( 0, 1 ) ;
			if ( !isvalidName( t.hlp ) ) { return false ; }
			t.dvhlp = t.hlp ;
		}
	}

	p1 = pos( ".WINDOW=", rest ) ;
	if ( p1 == 0 )
	{
		p1 = pos( ".W=", rest ) ;
		ln = 3 ;
	}
	else
	{
		ln = 8 ;
	}
	if ( p1 > 0 )
	{
		t.lmwin = true ;
		p2 = pos( " ", rest, p1 ) ;
		if ( p2 == 0 ) { tmp = substr( rest, p1+ln )           ; rest = delstr( rest, p1 )        ; }
		else           { tmp = substr( rest, p1+ln, p2-p1-ln ) ; rest = delstr( rest, p1, p2-p1 ) ; }
		if ( tmp.size() == 0 ) { return false ; }
		if ( tmp[ 0 ] == '&')
		{
			tmp.erase( 0, 1 ) ;
			if ( !isvalidName( tmp ) ) { return false ; }
			t.dvwin = tmp ;
		}
		else if ( tmp == "RESP"    || tmp == "R"  ) { t.smwin = true  ; t.resp = true ; }
		else if ( tmp == "NORESP"  || tmp == "N"  ) { t.smwin = true  ; }
		else if ( tmp == "LRESP"   || tmp == "LR" ) { t.resp  = true  ; }
		else if ( tmp == "LNORESP" || tmp == "LN" ) {                   }
		else    { return false  ; }
	}

	p1 = pos( ".ALARM=", rest ) ;
	if ( p1 == 0 )
	{
		p1 = pos( ".A=", rest ) ;
		ln = 3 ;
	}
	else
	{
		ln = 7 ;
	}
	if ( p1 > 0 )
	{
		p2 = pos( " ", rest, p1 ) ;
		if ( p2 == 0 ) { tmp = substr( rest, p1+ln )           ; rest = delstr( rest, p1 )        ; }
		else           { tmp = substr( rest, p1+ln, p2-p1-ln ) ; rest = delstr( rest, p1, p2-p1 ) ; }
		if ( tmp.size() == 0 ) { return false ; }
		if ( tmp[ 0 ] == '&')
		{
			tmp.erase( 0, 1 ) ;
			if ( !isvalidName( tmp ) ) { return false ; }
			t.dvalm = tmp ;
		}
		else if ( tmp == "YES" ) { t.alm = true  ; }
		else if ( tmp == "NO"  ) { t.alm = false ; }
		else                     { return false  ; }
	}

	p1 = pos( ".TYPE=", rest ) ;
	if ( p1 == 0 )
	{
		p1 = pos( ".T=", rest ) ;
		ln = 3 ;
	}
	else
	{
		ln = 6 ;
	}
	if ( p1 > 0 )
	{
		p2 = pos( " ", rest, p1 ) ;
		if ( p2 == 0 ) { tmp = substr( rest, p1+ln )           ; rest = delstr( rest, p1 )        ; }
		else           { tmp = substr( rest, p1+ln, p2-p1-ln ) ; rest = delstr( rest, p1, p2-p1 ) ; }
		if ( tmp.size() == 0 ) { return false ; }
		if ( tmp[ 0 ] == '&')
		{
			tmp.erase( 0, 1 ) ;
			if ( !isvalidName( tmp ) ) { return false ; }
			t.dvtype = tmp ;
		}
		else if ( tmp == "N" ) { t.type = IMT ; t.alm = false ; }
		else if ( tmp == "W" ) { t.type = WMT ; t.alm = true  ; }
		else if ( tmp == "A" ) { t.type = AMT ; t.alm = true  ; }
		else if ( tmp == "C" ) { t.type = AMT ; t.alm = true  ; t.resp = true ; t.smwin = true ; t.lmwin = true ; }
		else                   { return false                 ; }
	}

	if ( strip( rest ) != "" ) { return false ; }

	if ( t.smwin && t.smsg != "" ) { t.lmsg = t.smsg +" - "+ t.lmsg ; }
	return true ;
}


bool pApplication::sub_Message_vars( slmsg & t )
{
	// Get the dialogue variable value specified in message .T, .A, .H and .W options

	// Error if the dialgue variable is blank
	// Use defaults for invalid values

	// .TYPE overrides .WINDOW and .ALARM

	string val ;

	if ( t.dvwin != "" )
	{
		t.lmwin = true ;
		vcopy( t.dvwin, val, MOVE ) ;
		val = strip( val ) ;
		if      ( val == "RESP"    || val == "R"  ) { t.smwin = true  ; t.resp = true ; }
		else if ( val == "NORESP"  || val == "N"  ) { t.smwin = true  ; }
		else if ( val == "LRESP"   || val == "LR" ) { t.resp  = true  ; }
		else if ( val == "LNORESP" || val == "LN" ) {                   }
		else if ( val == "" ) { return false ; }
	}
	if ( t.dvalm != "" )
	{
		vcopy( t.dvalm, val, MOVE ) ;
		val = strip( val ) ;
		if      ( val == "YES" ) { t.alm = true  ; }
		else if ( val == "NO"  ) { t.alm = false ; }
		else if ( val == ""    ) { return false  ; }
	}
	if ( t.dvtype != "" )
	{
		vcopy( t.dvtype, val, MOVE ) ;
		val = strip( val ) ;
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


int pApplication::chk_Message_id( const string & msgid )
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

	int p1 ;
	int p2 ;

	string var ;
	string val ;

	const string validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#$@" ;
	p1 = 0 ;
	p2 = 0 ;

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
	int i ;
	int p ;

	log( "-", "*************************************************************************************************************" << endl ) ;
	log( "-", "Application Information for " << ZAPPNAME << endl ) ;
	log( "-", "                   Task ID: " << taskID << endl ) ;
	log( "-", "          Shared Pool Name: " << shrdPool << endl ) ;
	log( "-", "         Profile Pool Name: " << ZZAPPLID << endl ) ;
	log( "-", " " << endl ) ;
	log( "-", "Application Description . : " << ZAPPDESC << endl ) ;
	log( "-", "Last Panel Displayed. . . : " << PANELID << endl ) ;
	log( "-", "Last Message Displayed. . : " << MSGID << endl )   ;
	log( "-", "Number of Panels Loaded . : " << panelList.size() << endl )  ;
	log( "-", "Number of Open Tables . . : " << tablesOpen.size() << endl ) ;
	if ( rexxName != "" )
	{
		log( "-", "Application running REXX. : " << rexxName << endl ) ;
	}
	log( "-", " " << endl ) ;
	if ( testMode )
	{
		log( "-", "Application running in test mode" << endl ) ;
	}
	if ( noTimeOut )
	{
		log( "-", "Application has disabled timeouts" << endl ) ;
	}
	if ( PASSLIB )
	{
		log( "-", "Application started with PASSLIB option" << endl ) ;
	}
	if ( NEWPOOL )
	{
		log( "-", "Application started with NEWPOOL option" << endl ) ;
	}
	if ( libdef_muser )
	{
		log( "-", "LIBDEF active for user message search" << endl ) ;
		p = getpaths( ZMUSER ) ;
		for ( i = 1 ; i <= p ; i++ )
		{
			log( "-", "       Path. . . . . . . " << getpath( ZMUSER, i ) << endl ) ;
		}

	}
	if ( libdef_puser )
	{
		log( "-", "LIBDEF active for user panel search" << endl ) ;
		p = getpaths( ZPUSER ) ;
		for ( i = 1 ; i <= p ; i++ )
		{
			log( "-", "       Path. . . . . . . " << getpath( ZPUSER, i ) << endl ) ;
		}
	}
	if ( libdef_tuser )
	{
		log( "-", "LIBDEF active for user table search" << endl ) ;
		p = getpaths( ZTUSER ) ;
		for ( i = 1 ; i <= p ; i++ )
		{
			log( "-", "       Path. . . . . . . " << getpath( ZTUSER, i ) << endl ) ;
		}
	}
	log( "-", "*************************************************************************************************************" << endl ) ;
}


void pApplication::closeTables()
{
	map<string, bool>::iterator it;

	for ( it = tablesOpen.begin() ; it != tablesOpen.end() ; it++ )
	{
		debug1( "Closing table " << it->first << endl ) ;
		p_tableMGR->destroyTable( RC, taskid(), it->first ) ;
	}
}


void pApplication::ispexec( const string & s )
{
	RC = ispexeci( this, s ) ;
}


void pApplication::checkRCode( const string & s )
{
	// If the error panel is to be displayed, cancel CONTROL DISPLAY LOCK

	log( "E", s << endl ) ;
	if ( ZERR2 != "" ) { log( "E", ZERR2 << endl ) ; }

	if ( !ControlErrorsReturn && RC >= 12 )
	{
		log( "E", "RC="<< RC <<" CONTROL ERRORS CANCEL is in effect.  Aborting" << endl ) ;
		vreplace( "ZERR1",  s ) ;
		vreplace( "ZERR2",  ZERR2  ) ;
		vreplace( "ZERR3",  ZERR3  ) ;
		vreplace( "ZERR4",  ZERR4  ) ;
		vreplace( "ZERR5",  ZERR5  ) ;
		vreplace( "ZERR6",  ZERR6  ) ;
		vreplace( "ZERR7",  ZERR7  ) ;
		vreplace( "ZERR8",  ZERR8  ) ;
		vreplace( "ZERR9",  ZERR9  ) ;
		vreplace( "ZERR10", ZERR10 ) ;
		ControlDisplayLock  = false  ;
		ControlErrorsReturn = true   ;
		display( "PSYSER1" ) ;
		if ( RC <= 8 ) { errPanelissued = true ; }
		abend() ;
	}
}


void pApplication::cleanup_default()
{
	// Dummy routine.  Override in the application so the customised one is called on an exception condition.
	// Use CONTROL ABENDRTN ptr_to_routine

	// Called on: abend()
	//            abendexc()
	//            set_forced_abend()
	//            set_timeout_abend()
}


void pApplication::cleanup()
{
	log( "I", "Shutting down application: " << ZAPPNAME << " Taskid: " << taskID << endl ) ;
	terminateAppl = true  ;
	busyAppl      = false ;
	log( "I", "Returning to calling program." << endl ) ;
	return ;
}


void pApplication::abend()
{
	log( "E", "Shutting down application: " << ZAPPNAME << " Taskid: " << taskID << " due to an abnormal condition" << endl ) ;
	abnormalEnd   = true  ;
	terminateAppl = true  ;
	busyAppl      = false ;
	SEL           = false ;
	(this->*pcleanup)()   ;
	log( "E", "Application entering wait state" << endl ) ;
	boost::this_thread::sleep_for(boost::chrono::seconds(31536000)) ;
}


void pApplication::abendexc()
{
	log( "E", "An unhandled exception has occured in application: " << ZAPPNAME << " Taskid: " << taskID << endl ) ;
	if ( !abending )
	{
		(this->*pcleanup)() ;
		abending = true     ;
	}
	else
	{
		log( "E", "An abend has occured during abend processing.  Cleanup will not be called" << endl ) ;
	}
	exception_ptr ptr = current_exception() ;
	log( "E", "Exception: " << (ptr ? ptr.__cxa_exception_type()->name() : "Unknown" ) << endl ) ;
	log( "E", "Shutting down application: " << ZAPPNAME << " Taskid: " << taskID << endl ) ;
	abnormalEnd   = true  ;
	terminateAppl = true  ;
	busyAppl      = false ;
	SEL           = false ;
	log( "E", "Application entering wait state" << endl ) ;
	boost::this_thread::sleep_for(boost::chrono::seconds(31536000)) ;
}


void pApplication::set_forced_abend()
{
	log( "E", "Shutting down application: " << ZAPPNAME << " Taskid: " << taskID << " due to a forced condition" << endl ) ;
	abnormalEnd       = true  ;
	abnormalEndForced = true  ;
	terminateAppl     = true  ;
	busyAppl          = false ;
	SEL               = false ;
	(this->*pcleanup)()       ;
}


void pApplication::set_timeout_abend()
{
	log( "E", "Shutting down application: " << ZAPPNAME << " Taskid: " << taskID << " due to a timeout condition" << endl ) ;
	abnormalEnd       = true  ;
	abnormalEndForced = true  ;
	abnormalTimeout   = true  ;
	terminateAppl     = true  ;
	busyAppl          = false ;
	SEL               = false ;
	(this->*pcleanup)()       ;
}


void pApplication::closeLog()
{
	log( "I", "Closing application log" << endl ) ;
	aplog.close() ;
}

