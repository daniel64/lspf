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

#include <cstdarg>

pApplication::pApplication()
{
	currApplication        = this   ;
	testMode               = false  ;
	propagateEnd           = false  ;
	jumpEntered            = false  ;
	resumeTime             = boost::posix_time::second_clock::universal_time() ;
	Insert                 = false  ;
	ControlDisplayLock     = false  ;
	ControlErrorsReturn    = false  ;
	ControlSplitEnable     = true   ;
        errPanelissued         = false  ;
	noTimeOut              = false  ;
	busyAppl               = false  ;
	terminateAppl          = false  ;
	abnormalEnd            = false  ;
	abnormalEndForced      = false  ;
	reloadCUATables        = false  ;
	NEWPOOL                = false  ;
	PASSLIB                = false  ;
	libdef_muser           = false  ;
	libdef_puser           = false  ;
	libdef_tuser           = false  ;
	SEL                    = false  ;
	setMSG                 = false  ;
	PPANELID               = ""     ;
	ZHELP                  = ""     ;
	ZAHELP                 = ""     ;
	ZMHELP                 = ""     ;
	ZTDROWS                = 0      ;
	ZTDTOP                 = 0      ;
	ZCURFLD                = ""     ;
	ZCURPOS                = 1      ;
	ZRC                    = 0      ;
	ZRSN                   = 0      ;
	ZRESULT                = ""     ;
	ZERR1                  = ""     ;
	ZERR2                  = ""     ;
	vdefine( "ZCURFLD",  &ZCURFLD ) ;
	vdefine( "ZCURPOS",  &ZCURPOS ) ;
	vdefine( "ZTDMARK",  &ZTDMARK ) ;
	vdefine( "ZTDDEPTH", &ZTDDEPTH) ;
	vdefine( "ZTDROWS",  &ZTDROWS ) ;
	vdefine( "ZTDSELS",  &ZTDSELS ) ;
	vdefine( "ZTDTOP",   &ZTDTOP  ) ;
	vdefine( "ZTDVROWS", &ZTDVROWS) ;
	vdefine( "ZERR1",    &ZERR1   ) ;
	vdefine( "ZERR2",    &ZERR2   ) ;
	vdefine( "ZAPPNAME", &ZAPPNAME) ;
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
	ZPLIB    = p_poolMGR->get( RC, "ZPLIB", PROFILE ) ;
	ZTLIB    = p_poolMGR->get( RC, "ZTLIB", PROFILE ) ;
	ZMLIB    = p_poolMGR->get( RC, "ZMLIB", PROFILE ) ;
	ZORXPATH = p_poolMGR->get( RC, "ZORXPATH", PROFILE ) ;

	ZTDMARK = centre( " BOTTOM OF DATA ", ds2d( p_poolMGR->get( RC, "ZSCRMAXW", SHARED ) ), '*' ) ;
	log( "I", "Initialisation complete" << endl ; )
}


void pApplication::wait_event()
{
	busyAppl = false ;
	while ( true )
	{
		if ( !busyAppl )
		{
			boost::mutex::scoped_lock lk(mutex) ;
			cond_appl.wait(lk) ;
			lk.unlock() ;
		}
		if ( busyAppl ) return ;
	}
}


void pApplication::panel_create( string p_name )
{
	string paths ;

	RC = 0 ;
	if ( panelList.find( p_name ) != panelList.end() ) return ;
	if ( !isvalidName( p_name ) ) { RC = 20 ; checkRCode( "Invalid panel name >>" + p_name + "<<" ) ; return ; }

	pPanel * p_panel    = new pPanel ;
	p_panel->p_poolMGR  = p_poolMGR  ;
	p_panel->p_funcPOOL = &funcPOOL  ;
	p_panel->init( RC )              ;

	if ( libdef_puser ) paths = mergepaths( ZPUSER, ZPLIB ) ;
	else                paths = ZPLIB                       ;

	RC = p_panel->loadPanel( p_name, paths ) ;

	if ( RC == 0 ) { panelList[ p_name ] = p_panel          ; }
	else           { ZERR2 = p_panel->PERR ; delete p_panel ; }
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



void pApplication::set_msg( string SMSG, string LMSG, cuaType MSGTYPE, bool MSGALRM )
{
	if ( panelList.size() == 0 ) { return ; }
	else                         { currPanel->set_msg( SMSG, LMSG, MSGTYPE, MSGALRM ) ; }
}


void pApplication::display( string p_name, string p_msg, string p_cursor, int p_csrpos )
{
	string  MSG    ;
	string  ZZVERB ;

	RC = 0 ;

	panel_create( p_name ) ;
	if ( RC > 0 ) { checkRCode( "Panel >>" + p_name + "<< not found or invalid for DISPLAY service" ) ; return ; }

	if ( propagateEnd )
	{
		if ( PPANELID == p_name ) { propagateEnd = false ;                }
		else                      { PPANELID = p_name ; RC = 8 ; return ; }
	}

	currPanel = panelList[ p_name ] ;

	if ( p_msg != "" )
	{
		read_Message( p_msg ) ;
		if ( RC > 0 ) { return ; }
		currPanel->set_msg( ZSMSG, ZLMSG, ZMSGTYPE, ZMSGALRM ) ;
		MSGID = p_msg ;
		if ( ZSMSG == "" ) { currPanel->showLMSG = true ; }
	}
	else
	{
		if ( setMSG )
		{
			currPanel->set_msg( ZSMSG, ZLMSG, ZMSGTYPE, ZMSGALRM ) ;
		}
		else
		{
			currPanel->clear_msg()       ;
			ZMHELP = ""                  ;
			currPanel->showLMSG = false  ;
		}
	}

	setMSG            = false    ;
        PANELID           = p_name   ;
	currPanel->CURFLD = p_cursor ;
	currPanel->CURPOS = p_csrpos ;
	
	p_poolMGR->put( RC, "ZPANELID", p_name, SHARED, SYSTEM ) ;
	currPanel->display_panel_init( RC ) ;
	if ( RC > 0 ) { ZERR2 = currPanel->PERR ; checkRCode( "Error processing )INIT section of panel " + p_name ) ; return ; }
	while ( true )
	{
		currPanel->display_panel( RC ) ;
		if ( RC > 0 ) { checkRCode( "Panel error displaying " + p_name ) ; break ; }
		currPanel->cursor_to_field( RC ) ;
		if ( RC > 0 ) { checkRCode( "Cursor field >>" + currPanel->CURFLD + "<< not found on panel or invalid for DISPLAY service" ) ; break ; }
		wait_event() ;
		currPanel->display_panel_update( RC ) ;
		currPanel->display_panel_proc( RC, 0 ) ;
		if ( RC > 0 ) { ZERR2 = currPanel->PERR ; checkRCode( "Error processing )PROC section of panel " + p_name ) ; break ; }

		ZZVERB = p_poolMGR->get( RC, "ZVERB", SHARED ) ;
		if ( RC > 0 ) { RC = 20 ; checkRCode( "PoolMGR get of ZVERB failed" ) ; }
		if ( ZZVERB == "RETURN" )                    { propagateEnd = true  ; }
		if ( ZZVERB == "END" || ZZVERB == "RETURN" ) { RC = 8 ; return      ; }

		if ( !currPanel->scrollOn )
		{
			if      ( ZZVERB == "UP" )    { currPanel->MSGID = "PSYS011" ; }
			else if ( ZZVERB == "DOWN" )  { currPanel->MSGID = "PSYS012" ; }
			else if ( ZZVERB == "RIGHT" ) { currPanel->MSGID = "PSYS013" ; }
			else if ( ZZVERB == "LEFT" )  { currPanel->MSGID = "PSYS014" ; }
		}
		if ( currPanel->MSGID != "" )
		{
			read_Message( currPanel->MSGID ) ;
			if ( RC > 0 ) { break ; }
			currPanel->set_msg( ZSMSG, ZLMSG, ZMSGTYPE, ZMSGALRM ) ;
			MSGID = currPanel->MSGID  ;
			currPanel->display_panel_reinit( RC, 0 ) ;
			if ( RC > 0 ) { ZERR2 = currPanel->PERR ; checkRCode( "Error processing )REINIT section of panel " + p_name ) ; return ; }
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
	if ( it == panelList.end() )
	{
		RC = 20 ; checkRCode( "Panel >>" + PANELID + "<< not found during REFRESH" ) ;
	}
	else
	{
		it->second->refresh( RC ) ;
	}
}


void pApplication::libdef( string lib, string type )
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
		checkRCode( "Invalid type on LIBDEF service" ) ;
	}
}


string pApplication::get_select_cmd( string opt )
{
	return panelList[ PANELID ]->return_command( opt ) ;
}


void pApplication::set_cursor( int row, int col )
{
	currPanel->set_cursor( row, col ) ;
}


void pApplication::vdefine( string names, int * i_ad1, int * i_ad2, int * i_ad3, int * i_ad4, int * i_ad5, int * i_ad6, int * i_ad7, int * i_ad8 )
{
	// RC = 0  Normal completion
	// RC = 20 Severe Error
	// (funcPOOL.define returns 0 or 20)

	int w ;

	string name ;
	
	string e1( "Too many variables on VDEFINE statement" ) ;
	string e2( "Address is null on VDEFINE statement" )    ;
	string e3( "Error in function pool define for " )      ;

	w = words( names ) ;
	if ( ( w > 8 ) || ( w < 1 ) ) { RC = 20 ; checkRCode( e1 ) ; return ; }

	RC = 0 ;

	if ( i_ad1 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
	name = word( names, 1 ) ;
	funcPOOL.define( RC, name, i_ad1 ) ;
	if ( RC > 0 ) checkRCode( e3 + name ) ;

	if ( w > 1 )
	{
		if ( i_ad2 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 2 ) ;
		funcPOOL.define( RC, name , i_ad2 ) ;
		if ( RC > 0 ) checkRCode( e3 + name ) ;
	}
	if ( w > 2 )
	{
		if ( i_ad3 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 3 ) ;
		funcPOOL.define( RC, name, i_ad3 ) ;
		if ( RC > 0 ) checkRCode( e3 + name ) ;
	}
	if ( w > 3 )
	{
		if ( i_ad4 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 4 ) ;
		funcPOOL.define( RC, name, i_ad4 ) ;
		if ( RC > 0 ) checkRCode( e3 + name ) ;
	}
	if ( w > 4 )
	{
		if ( i_ad5 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 5 ) ;
		funcPOOL.define( RC, name, i_ad5 ) ;
		if ( RC > 0 ) checkRCode( e3 + name ) ;
	}
	if ( w > 5 )
	{
		if ( i_ad6 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 6 ) ;
		funcPOOL.define( RC, name, i_ad6 ) ;
		if ( RC > 0 ) checkRCode( e3 + name ) ;
	}

	if ( w > 6 )
	{
		if ( i_ad7 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 7 ) ;
		funcPOOL.define( RC, name, i_ad7 ) ;
		if ( RC > 0 ) checkRCode( e3 + name ) ;
	}

	if ( w > 7 )
	{
		if ( i_ad8 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 8 ) ;
		funcPOOL.define( RC, name, i_ad8 ) ;
		if ( RC > 0 ) checkRCode( e3 + name ) ;
	}
	if ( RC > 0 ) { return ; }
}



void pApplication::vdefine( string names, string * s_ad1, string * s_ad2, string * s_ad3, string * s_ad4, string * s_ad5, string * s_ad6, string * s_ad7, string * s_ad8 )
{
	// RC = 0  Normal completion
	// RC = 20 Severe Error
	// (funcPOOL.define returns 0 or 20)

	int w  ;

	string name ;

	string e1( "Too many variables on VDEFINE statement" ) ;
	string e2( "Address is null on VDEFINE statement" )    ;
	string e3( "Error in function pool define for " )      ;

	RC = 0 ;

	w = words( names ) ;
	if ( ( w > 8 ) | ( w < 1 ) ) { RC = 20 ; checkRCode( e1 ) ; return ; }

	if ( s_ad1 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
	name = word( names, 1 ) ;
	funcPOOL.define( RC, name, s_ad1 ) ;
	if ( RC > 0 ) checkRCode( e3 + name ) ;

	if ( w > 1 )
	{
		if ( s_ad2 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 2 ) ;
		funcPOOL.define( RC, name, s_ad2 ) ;
		if ( RC > 0 ) checkRCode( e3 + name ) ;
	}
	if ( w > 2 )
	{
		if ( s_ad3 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 3 ) ;
		funcPOOL.define( RC, name, s_ad3 ) ;
		if ( RC > 0 ) checkRCode( e3 + name ) ;
	}
	if ( w > 3 )
	{
		if ( s_ad4 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 4 ) ;
		funcPOOL.define( RC, name, s_ad4 ) ;
		if ( RC > 0 ) checkRCode( e3 + name ) ;
	}
	if ( w > 4 )
	{
		if ( s_ad5 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 5 ) ;
		funcPOOL.define( RC, name, s_ad5 ) ;
		if ( RC > 0 ) checkRCode( e3 + name ) ;
	}
	if ( w > 5 )
	{
		if ( s_ad6 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 6 ) ;
		funcPOOL.define( RC, name, s_ad6 ) ;
		if ( RC > 0 ) checkRCode( e3 + name ) ;
	}
	if ( w > 6 )
	{
		if ( s_ad7 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 7 ) ;
		funcPOOL.define( RC, name, s_ad7 ) ;
		if ( RC > 0 ) checkRCode( e3 + name ) ;
	}
	if ( w > 7 )
	{
		if ( s_ad8 == NULL ) { RC = 20 ; checkRCode( e2 ) ; return ; }
		name = word( names, 8 ) ;
		funcPOOL.define( RC, name, s_ad8 ) ;
		if ( RC > 0 ) checkRCode ( e3 + name ) ;
	}
	if ( RC > 0 ) { return ; }
}


void pApplication::vdelete( string names )
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
		if ( RC > 8 ) checkRCode( "VDELETE failed for " + name ) ;
		if ( RC > maxRC ) { maxRC = RC ; }
		if ( RC > 8 )     { return     ; }
	}
	RC = maxRC ;
}


void pApplication::vreset()
{
	// RC = 0  Normal completion
	// RC = 20 Severe error
	// (funcPOOL.reset returns 0)

	funcPOOL.reset() ;
}


void pApplication::vreplace( string name, string s_val )
{
	// RC = 0  Normal completion
	// RC = 20 Severe error
	// (funcPOOL.put returns 0, 20)

	RC = 0 ;
	funcPOOL.put( RC, 0, name, s_val ) ;
	if ( RC > 8 ) checkRCode( "Function pool put failed for " + name ) ;
}


void pApplication::vreplace( string name, int i_val )
{
	// RC = 0  Normal completion
	// RC = 20 Severe error
	// (funcPOOL.put returns 0 or 20)

	RC = 0 ;
	funcPOOL.put( RC, 0, name, i_val ) ;
	if ( RC > 8 ) checkRCode( "VREPLACE failed for " + name ) ;
}


void pApplication::vget( string names, poolType pType )
{
	// RC = 0  Normal completion
	// RC = 8  Variable not found
	// RC = 20 Severe error
	// (funcPOOL.put returns 0 or 20)
	// (funcPOOL.getType returns 0, 8 or 20.  For RC = 8 create implicit function pool variable)
	// (poolMGR.get return 0, 8 or 20)

	string val        ;
	string name       ;

	int maxRC         ;
	int ws            ;
	int i             ;

	dataType var_type ;

	RC    = 0 ;
	maxRC = 0 ;

	ws = words( names ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		name = word( names, i ) ;
		val  = p_poolMGR->get( RC, name, pType ) ;
		if ( RC  > 8 ) checkRCode( "Pool manager get failed for " + name ) ;
		if ( RC == 0 )
		{
			var_type = funcPOOL.getType( RC, name ) ;
			if ( RC  > 8 ) checkRCode( "Function pool getType failed for " + name ) ;
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
				if ( RC > 0 ) checkRCode( "Function pool put failed for " + name ) ;
			}
			else if ( RC == 8 )
			{
				funcPOOL.put( RC, 0, name, val ) ;
				if ( RC > 0 ) checkRCode( "Function pool put failed creating implicit variable for " + name ) ;
			}
		}
		if ( RC > maxRC ) { maxRC = RC ; }
		if ( RC > 8     ) { return     ; }
	}
	RC = maxRC ;
}


void pApplication::vput( string names, poolType pType )
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
		if ( RC  > 8 ) checkRCode( "Function pool getType failed for " + name ) ;
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
			if ( RC > 0 ) checkRCode( "Function pool get failed for " + name ) ;
			p_poolMGR->put( RC, name, s_val, pType ) ;
			if ( RC > 0 ) checkRCode( "Pool manager put failed for " + name + " RC=" + d2ds( RC ) ) ;
		}
		if ( RC > maxRC ) { maxRC = RC ; }
		if ( RC > 8     ) { return     ; }
	}
	RC = maxRC ;
}


void pApplication::vcopy( string name, string & var_name, vcMODE mode )
{
	// RC = 0  Normal completion
	// RC = 8  Variable not found
	// RC = 16 Truncation occured
	// RC = 20 Severe error
	// (funcPOOL.get returns 0, 8 or 20)
	// (funcPOOL.getType returns 0, 8 or 20)
	// (poolMGR.get return 0, 8 or 20)

	string s_val ;

	dataType var_type  ;

	switch ( mode )
	{
	case LOCATE:
		RC = 20 ;
		log( "N", "LOCATE mode for VCOPY not yet implemented" << endl ) ;
		break ;
	case MOVE:
		var_type = funcPOOL.getType( RC, name ) ;
		if ( RC  > 8 ) checkRCode( "Function pool getType failed for " + name ) ;
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
			if ( RC  > 0 ) checkRCode( "Function pool get failed for " + name ) ;
			if ( RC == 0 ) { var_name = s_val ; }
		}
		else if ( RC == 8 )
		{
			s_val = p_poolMGR->get( RC, name, ASIS ) ;
			if ( RC  > 8 ) { checkRCode( "Pool failed for " + name) ; }
			if ( RC == 0 ) { var_name = s_val                       ; }
		}
	}
}


void pApplication::verase( string names, poolType pType )
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
		if ( RC > 0 ) checkRCode( "Pool erase failed for " + name) ;
		if ( RC > maxRC ) { maxRC = RC ; }
		if ( RC > 8     ) { return     ; }
	}
	RC = maxRC ;
}


string pApplication::vlist( poolType pType, int lvl )
{
	return p_poolMGR->vlist( RC, pType, lvl ) ;
}


void pApplication::control( string parm1, string parm2 )
{
	// CONTROL ERRORS CANCEL - abend for RC >= 12
	// CONTROL ERRORS RETURN - return to application for any RC
	// CONTROL DISPLAY SAVE/RESTORE - SAVE/RESTORE status for a TBDISPL
	//         SAVE/RESTORE saves/restores the six ZTD* variables in the function pool as well
	//         as the currtbPanel pointer for retrieving other model sets via tbdispl with no panel specified
	//         Only necessary if a tbdispl invokes another tbdispl in the same task

	RC = 0 ;

	if ( parm1 == "DISPLAY" )
	{
		if ( parm2 == "LOCK" )
		{
			ControlDisplayLock = true ;
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
	else if ( parm1 == "SPLIT" )
	{
		if ( parm2 == "ENABLE" )
		{
			ControlSplitEnable = true ;
		}
		else if ( parm2 == "DISABLE" )
		{
			ControlSplitEnable = false ;
		}
		else { RC = 20 ; }
	}
	else { RC = 20 ; }
	if ( RC > 0 ) checkRCode( "Error in control service" ) ;
}


void pApplication::tbadd( string tb_name, string tb_namelst, string tb_order, int tb_num_of_rows ) 
{
	// Add a row to a table
	// RC = 0   Normal completion
	// RC = 8   For keyed tables only, row already exists
	// RC = 12  Table not open
	// RC = 20  Severe error

	RC = 0 ;

	if ( tb_order  != "" && tb_order != "ORDER" )       { RC = 20 ; checkRCode( "Invalid ORDER parameter specified on TBADD" )          ; return ; }
	if ( tb_num_of_rows < 0 || tb_num_of_rows > 65535 ) { RC = 20 ; checkRCode( "Invalid number-of-rows parameter specified on TBADD" ) ; return ; }

	if ( !isTableOpen( tb_name, "TBADD" ) ) { return ; }

	p_tableMGR->tbadd( RC, funcPOOL, tb_name, tb_namelst, tb_order, tb_num_of_rows ) ;
	if ( RC > 8 ) { checkRCode( "TBADD gave return code of " + d2ds( RC ) ) ; }
}


void pApplication::tbbottom( string tb_name )
{
	// Move row pointer to the bottom
	// RC = 0   Normal completion
	// RC = 8   Table is empty.  CRP is set to 0
	// RC = 12  Table not open
	// RC = 20  Severe error

	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBBOTTOM" ) ) { return ; }

	p_tableMGR->tbbottom( RC, tb_name ) ;
	if ( RC > 8 ) { checkRCode( "TBBOTTOM gave return code of " + d2ds( RC ) ) ; }
}


void pApplication::tbclose( string tb_name, string tb_newname, string tb_path )
{
	// Save and close the table (calls saveTableifWRITE and destroyTable routines).  If NOWRITE specified, just remove table from storage.
	// RC = 0   Normal completion
	// RC = 12  Table not open
	// RC = 16  Path error
	// RC = 20  Severe error

	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBCLOSE" ) ) { return ; }

	p_tableMGR->saveTableifWRITE( RC, taskid(), tb_name, tb_newname, tb_path ) ;
	if ( RC == 0 )
	{
		p_tableMGR->destroyTable( RC, taskid(), tb_name ) ;
		if ( RC == 0 )
		{
			tablesOpen.erase( tb_name ) ;
		}
	}
	if ( RC > 8 ) { checkRCode( "TBCLOSE gave return code of " + d2ds( RC ) ) ; }
}


void pApplication::tbcreate( string tb_name, string keys, string names, tbSAVE m_SAVE, tbREP m_REP, string m_path, tbDISP m_DISP )
{
	// Create a new table
	// RC = 0   Normal completion
	// RC = 4   Normal completion - Table exists and REPLACE speified
	// RC = 8   Table exists and REPLACE not specified or REPLACE specified and opend in SHARE mode or REPLACE specified and opened in EXCLUSIVE mode but not the owning task
	// RC = 12  Table in use
	// RC = 16  ??
	// RC = 20  Severe error

	int ws ;

	string w ;

	RC = 0 ;

	if ( !isvalidName( tb_name ) ) { RC = 20 ; checkRCode( "Invalid table name on TBCREATE >>" + tb_name + "<<" ) ; return ; }
	if ( ( tablesOpen.find( tb_name ) != tablesOpen.end() ) && m_REP != REPLACE )
	{
		RC = 8  ;
		log( "E", "Table >>" << tb_name << "<< has already been created under this task and REPLACE not specified" << endl ) ;
		return  ;
	}

	if ( (m_SAVE == WRITE) & (m_path == "") ) { m_path = ZTLIB ; }

	ws = words( keys ) ;
	for ( int i = 1 ; i <= ws ; i++ )
	{
		w = word( keys, i ) ;
		if ( !isvalidName( w ) ) { RC = 20 ; checkRCode( "Invalid key name >>" + w + "<<" ) ; return ; }
	}

	ws = words( names ) ;
	for ( int i = 1; i <= ws ; i++ )
	{
		w = word( names, i ) ;
		if ( !isvalidName( w ) ) { RC = 20 ; checkRCode( "Invalid field name >>" + w + "<<" ) ; return ; }
	}

	p_tableMGR->createTable( RC, taskid(), tb_name, keys, names, m_SAVE, m_REP, m_path, m_DISP ) ;
	if ( RC > 8 ) { checkRCode( "TBCREATE gave return code of " + d2ds( RC ) ) ; }

	if ( RC < 8 ) tablesOpen[ tb_name ] = true ;
}


void pApplication::tbdelete( string tb_name )
{
	// Delete a row in the table.  For keyed tables, the table is searched with the current key.  For non-keyed tables the current CRP is used.
	// RC = 0   Normal completion
	// RC = 8   Row does not exist for a keyed table or for non-keyed table, CRP was at TOP(zero)
	// RC = 12  Table not open
	// RC = 20  Severe error

	RC = 0 ;
	if ( !isTableOpen( tb_name, "TBDELETE" ) ) { return ; }

	p_tableMGR->tbdelete( RC, funcPOOL, tb_name ) ;
	if ( RC > 8 ) { checkRCode( "TBDELETE gave return code of " + d2ds( RC ) ) ; }
}


void pApplication::tbdispl( string tb_name, string p_name, string p_msg, string p_cursor, int p_csrrow, int p_csrpos, string p_autosel, string p_crp_name, string p_rowid_nm )
{
	// tbdispl with panel, no message - clear previous pending lines, rebuild scrollable area and display panel
	// tbdispl with panel, message    - rebuild scrollable area and display panel and message
	// tbdispl no panel, no message   - retrieve next pending line.  If none, display panel
	// tbdispl no panel, message      - display panel with message.  No rebuilding of the scrollable area
	
	// Set CRP to first changed line, autoselected line, or tbtop if there are no selected lines

	// If .AUTOSEL and .CSRROW set in panel, override the parameters p_autosel and p_csrrow
	
	// Autoselect if the p_csrpos CRP is visible

	int EXITRC  ;
	int ws      ;
	int i       ;
	int ln      ;
	int posn    ;
	int csrvrow ;

	string CMD    ;
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

	if ( p_cursor   != "" && !isvalidName( p_cursor   ) ) { RC = 20 ; checkRCode( "Invalid CURSOR position" ) ; return ; }
	if ( p_autosel  != "YES" && p_autosel != "NO"     )   { RC = 20 ; checkRCode( "Invalid AUTOSEL parameter.  Must be YES or NO" ) ; return ; }
	if ( p_crp_name != "" && !isvalidName( p_crp_name ) ) { RC = 20 ; checkRCode( "Invalid CRP variable name" )   ; return ; }
	if ( p_rowid_nm != "" && !isvalidName( p_rowid_nm ) ) { RC = 20 ; checkRCode( "Invalid ROWID variable name" ) ; return ; }

	if ( p_name != "" )
	{
		panel_create( p_name ) ;
		if ( RC > 0 ) { checkRCode( "Panel >>" + p_name + "<< not found or invalid for TBDISPL" ) ; return ; }
		currtbPanel = panelList[ p_name ] ;
		currPanel   = currtbPanel         ;
		PANELID     = p_name ;
		p_poolMGR->put( RC, "ZPANELID", p_name, SHARED, SYSTEM ) ;
		if ( p_msg == "" )
		{
			currtbPanel->clear_tb_linesChanged( RC ) ;
			if ( RC > 0 ) { ZERR2 = currtbPanel->PERR ; checkRCode( "Panel >>" + p_name + "<< error during TBDISPL" ) ; return ; }
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
			currtbPanel->clear_URID_tb_lineChanged( RC, URID ) ;
			if ( RC > 0 ) { ZERR2 = currtbPanel->PERR ; checkRCode( "Panel >>" + currtbPanel->PANELID + "<< error during TBDISPL" ) ; return ; }
		}
		else
		{
			p_name    = currtbPanel->PANELID ;
			currPanel = currtbPanel ;
			PANELID   = p_name      ;
			p_poolMGR->put( RC, "ZPANELID", p_name, SHARED, SYSTEM ) ;
		}
	}

	if ( p_msg != "" )
	{
		read_Message( p_msg ) ;
		if ( RC > 0 ) { return ; }
		currtbPanel->set_msg( ZSMSG, ZLMSG, ZMSGTYPE, ZMSGALRM ) ;
		MSGID = p_msg    ;
		if ( ZSMSG == "" ) { currtbPanel->showLMSG = true ; }
	}
	else
	{
		if ( setMSG )
		{
			currtbPanel->set_msg( ZSMSG, ZLMSG, ZMSGTYPE, ZMSGALRM ) ;
		}
		else
		{
			currtbPanel->clear_msg()       ;
			ZMHELP = ""                    ;
			currtbPanel->showLMSG = false  ;
		}
	}
	setMSG = false ;
	if ( p_cursor == "" ) { p_cursor = currtbPanel->Home ; }

	currtbPanel->display_panel_init( RC ) ;
	if ( RC > 0 ) { ZERR2 = currtbPanel->PERR ; checkRCode( "Error processing )INIT section of panel " + p_name ) ; return ; }

	t = funcPOOL.get( RC, 8, ".AUTOSEL", NOCHECK ) ;
	if ( RC == 0 ) { p_autosel = t ; }
	t = funcPOOL.get( RC, 8, ".CSRROW", NOCHECK ) ;
	if ( RC == 0 ) { p_csrrow = ds2d( t ) ; }

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
			if ( RC > 0 ) { checkRCode( "Cursor field >>" + currtbPanel->CURFLD + "<< not found on panel or invalid for TBDISP service" ) ; break ; }
			wait_event() ;
			currtbPanel->CURFLD = "" ;
			currtbPanel->display_panel_update( RC ) ;
			currtbPanel->set_tb_linesChanged() ;
		}
 
		ZZVERB = p_poolMGR->get( RC, "ZVERB" ) ;
                if ( RC > 0 ) { RC = 20 ; checkRCode( "PoolMGR get of ZVERB failed" ) ; }
		if ( ZZVERB == "RETURN" ) { propagateEnd = true  ; }
		if ( ZZVERB == "END" || ZZVERB == "EXIT" || ZZVERB == "RETURN" ) { RC = 8 ; return ; }

		EXITRC = 0  ;
		if ( currtbPanel->tb_lineChanged( ln, URID ) )
		{
			tbskip( tb_name, 0, "", p_rowid_nm, URID, "", p_crp_name ) ;
			ws = words( currtbPanel->tb_fields ) ;
			for ( i = 1 ; i <= ws ; i++ )
			{
				s = word( currtbPanel->tb_fields, i ) ;
				funcPOOL.put( RC, 0, s, funcPOOL.get( RC, 0, s + "." + d2ds( ln ), NOCHECK ) ) ;
			}
			if ( ZTDSELS > 1 ) { EXITRC = 4; }
		}

		currtbPanel->display_panel_proc( RC, ln ) ;
		if ( RC > 0 ) { ZERR2 = currtbPanel->PERR ; checkRCode( "Error processing )PROC section of panel " + p_name ) ; return ; }
		t = funcPOOL.get( RC, 8, ".CSRROW", NOCHECK ) ;
		if ( RC == 0 ) { p_csrrow = ds2d( t ) ; }

		if ( currtbPanel->MSGID != "" )
		{
			read_Message( currtbPanel->MSGID ) ;
			if ( RC > 0 ) { return ; }
			currtbPanel->set_msg( ZSMSG, ZLMSG, ZMSGTYPE, ZMSGALRM ) ;
			MSGID = currtbPanel->MSGID  ;
			if ( p_name == "" )
			{
				p_name    = currtbPanel->PANELID ;
				currPanel = currtbPanel ;
				PANELID   = p_name      ;
				p_poolMGR->put( RC, "ZPANELID", p_name, SHARED, SYSTEM ) ;
			}
			currtbPanel->display_panel_reinit( RC, ln ) ;
			if ( RC > 0 ) { ZERR2 = currtbPanel->PERR ; checkRCode( "Error processing )REINIT section of panel " + p_name ) ; break ; }
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
	RC = EXITRC ;
}


void pApplication::tbend( string tb_name )
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
	}
	if ( RC > 8 ) { checkRCode( "TBEND gave return code of " + d2ds( RC ) ) ; }
}


void pApplication::tberase( string tb_name, string tb_path )
{
	// Erase a table file from path
	// RC = 0   Normal completion
	// RC = 8   Table does not exist
	// RC = 12  Table in use
	// RC = 16  Path does not exist
	// RC = 20  Severe error

	RC = 0 ;
	if ( !isvalidName( tb_name ) ) { RC = 20 ; checkRCode( "Invalid table name on TBERASE" ) ; return ; }

	if ( tb_path == "" ) tb_path = ZTLIB ;
	p_tableMGR->tberase( RC, tb_name, tb_path ) ;
	if ( RC > 8 ) { checkRCode( "TBERASE gave return code of " + d2ds( RC ) ) ; }
}


void pApplication::tbexist( string tb_name )
{
	// Test for the existance of a row in a keyed table
	// RC = 0   Normal completion
	// RC = 8   Row does not exist or not a keyed table
	// RC = 12  Table not open
	// RC = 20  Severe error

	RC = 0 ;
	if ( !isTableOpen( tb_name, "TBEXIST" ) ) { return ; }

	p_tableMGR->tbexist( RC, funcPOOL, tb_name ) ;
	if ( RC > 8 ) { checkRCode( "TBEXISTS gave return code of " + d2ds( RC ) ) ; }
}



void pApplication::tbget( string tb_name, string tb_savenm, string tb_rowid_vn, string tb_noread, string tb_crp_name )
{
	RC = 0 ;
	if ( !isTableOpen( tb_name, "TBGET" ) ) { return ; }

	p_tableMGR->tbget( RC, funcPOOL, tb_name, tb_savenm, tb_rowid_vn, tb_noread, tb_crp_name  ) ;
	if ( RC > 8 ) { checkRCode( "TBGET gave return code of " + d2ds( RC ) ) ; }
}


void pApplication::tbmod( string tb_name, string tb_namelst, string tb_order )
{
	// Update a row in a table

	RC = 0 ;

	if ( tb_namelst != "" )                             { RC = 20 ; checkRCode( "Name list not yet implemented for TBMOD" ) ; return ; }
	if ( tb_order  != "" && tb_order != "ORDER" )       { RC = 20 ; checkRCode( "Invalid ORDER parameter specified on TBMOD" ) ; return ; }

	if ( !isTableOpen( tb_name, "TBMOD" ) ) { return ; }

	p_tableMGR->tbmod( RC, funcPOOL, tb_name, tb_namelst, tb_order ) ;
	if ( RC > 8 ) { checkRCode( "TBMOD gave return code of " + d2ds( RC ) ) ; }
}


void pApplication::tbopen( string tb_name, tbSAVE m_SAVE, string m_paths, tbDISP m_DISP )
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
		checkRCode( "Table " + tb_name + " already open on TBOPEN" ) ;
		return  ;
	}

	if ( m_paths == "" )
	{
	  	if ( libdef_tuser ) m_paths = mergepaths( ZTUSER, ZTLIB ) ;
		else                m_paths = ZTLIB                       ;
	}
	if( m_SAVE == NOWRITE ) { m_DISP = SHARE  ; }
	p_tableMGR->loadTable( RC, taskid(), tb_name, m_SAVE, m_DISP, m_paths ) ;

	if ( RC > 8 ) { checkRCode( "TBOPEN gave return code of " + d2ds( RC ) ) ; }
	if ( RC < 8 ) tablesOpen[ tb_name ] = true ;
}


void pApplication::tbput( string tb_name, string tb_namelst, string tb_order )
{
	// Update the current row in a table
	// RC = 0   Normal completion
	// RC = 8   Keyed tables - key does not match current row.  CRP set to top (0)
	//          Non-keyed tables - CRP at top
	// RC = 12  Table not open
	// RC = 16  Numeric conversion error for sorted tables
	// RC = 20  Severe error

	RC = 0 ;

	if ( tb_namelst != "" )                             { RC = 20 ; checkRCode( "Name list not yet implemented for TBPUT"  ) ; return ; }
	if ( tb_order  != "" && tb_order != "ORDER" )       { RC = 20 ; checkRCode( "Invalid ORDER parameter specified on TBPUT" ) ; return ; }

	if ( !isTableOpen( tb_name, "TBPUT" ) ) { return ; }

	p_tableMGR->tbput( RC, funcPOOL, tb_name, tb_namelst, tb_order ) ;
	if ( RC > 8 ) { checkRCode( "TBPUT gave return code of " + d2ds( RC ) ) ; }
}


void pApplication::tbquery( string tb_name, string tb_keyn, string tb_varn, string tb_rownn, string tb_keynn, string tb_namenn, string tb_crpn, string tb_sirn, string tb_lstn, string tb_condn, string tb_dirn )
{
	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBQUERY" ) ) { return ; }

	p_tableMGR->tbquery( RC, funcPOOL, tb_name, tb_keyn, tb_varn, tb_rownn, tb_keynn, tb_namenn, tb_crpn, tb_sirn, tb_lstn, tb_condn, tb_dirn ) ;
	if ( RC > 8 ) { checkRCode( "TBQUERY gave return code of " + d2ds( RC ) ) ; }
}


void pApplication::tbsarg( string tb_name, string tb_namelst, string tb_dir, string tb_cond_pairs )
{
	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBSARG" ) ) { return ; }

	p_tableMGR->tbsarg( RC, funcPOOL, tb_name, tb_namelst, tb_dir, tb_cond_pairs ) ;
	if ( RC > 8 ) { checkRCode( "TBSARG gave return code of " + d2ds( RC ) ) ; }
}


void pApplication::tbsave( string tb_name, string tb_newname, string path )
{
	// Save the table to disk (calls saveTable routine).  Table remains open for processing.  Table must have the WRITE attribute
	// RC = 0   Normal completion
	// RC = 12  Table not open or not open WRITE
	// RC = 16  Alternate name save error
	// RC = 20  Severe error

	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBSAVE" ) ) { return ; }

	p_tableMGR->saveTable( RC, taskid(), tb_name, tb_newname, path ) ;
	if ( RC > 8 ) { checkRCode( "TBSAVE gave return code of " + d2ds( RC ) ) ; }
}


void pApplication::tbscan( string tb_name, string tb_namelst, string tb_savenm, string tb_rowid_vn, string tb_dir, string tb_read, string tb_crp_name, string tb_condlst )
{
	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBSCAN" ) ) { return ; }

	p_tableMGR->tbscan( RC, funcPOOL, tb_name, tb_namelst, tb_savenm, tb_rowid_vn, tb_dir, tb_read, tb_crp_name, tb_condlst ) ;
	if ( RC > 8 ) { checkRCode( "TBSCAN gave return code of " + d2ds( RC ) ) ; }
}



void pApplication::tbskip( string tb_name, int num, string tb_savenm, string tb_rowid_vn, string tb_rowid, string tb_noread, string tb_crp_name )
{
	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBSKIP" ) ) { return ; }

	p_tableMGR->tbskip( RC, funcPOOL, tb_name, num, tb_savenm, tb_rowid_vn, tb_rowid, tb_noread, tb_crp_name ) ;
	if ( RC > 8 ) { checkRCode( "TBSKIP gave return code of " + d2ds( RC ) ) ; }
}


void pApplication::tbsort( string tb_name, string tb_fields )
{
	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBSORT" ) ) { return ; }

	p_tableMGR->tbsort( RC, funcPOOL, tb_name, tb_fields ) ;
	if ( RC > 8 ) { checkRCode( "TBSORT gave return code of " + d2ds( RC ) ) ; }
}



void pApplication::tbtop( string tb_name )
{
	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBTOP" ) ) { return ; }

	p_tableMGR->tbtop( RC, tb_name ) ;
	if ( RC > 8 ) { checkRCode( "TBTOP gave return code of " + d2ds( RC ) ) ; }
}


void pApplication::tbvclear( string tb_name )
{
	RC = 0 ;

	if ( !isTableOpen( tb_name, "TBVCLEAR" ) ) { return ; }

	p_tableMGR->tbvclear( RC, funcPOOL, tb_name ) ;
	if ( RC > 8 ) { checkRCode( "TBVCLEAR gave return code of " + d2ds( RC ) ) ; }
}


bool pApplication::isTableOpen( string tb_name, string func )
{

	RC = 0 ;

	if ( !isvalidName( tb_name ) ) { RC = 20 ; checkRCode( "Invalid table name on " + func + " >>" + tb_name + "<<" ) ; return false ; }
	if ( tablesOpen.find( tb_name ) == tablesOpen.end() )
	{
		RC = 12      ;
		checkRCode( "Table " + tb_name + " not open on " + func ) ;
		return false ;
	}
	return true ;
}



void pApplication::edit( string m_file, string m_panel )
{
	select( p_poolMGR->get( RC, "ZEDITPGM", PROFILE ), "FILE(" + m_file + ") PANEL(" + m_panel + ")", "", false, false ) ;
}


void pApplication::browse( string m_file, string m_panel )
{
	select( p_poolMGR->get( RC, "ZBRPGM", PROFILE ), "FILE(" + m_file + ") PANEL(" + m_panel + ")", "", false, false ) ;
}


void pApplication::view( string m_file, string m_panel )
{
	select( p_poolMGR->get( RC, "ZVIEWPGM", PROFILE ), "FILE(" + m_file + ") PANEL(" + m_panel + ")", "", false, false ) ;
}


void pApplication::select( string cmd )
{
	// SELECT a function or panel in keyword format for use in applications, ie PGM(abc) CMD(oorexx) PANEL(xxx) PARM(zzz) NEWAPPL PASSLIB etc.
	// No variable substitution is done at this level.
	// RC=0  Normal completion of the selection panel or function.  END was entered.
	// RC=4  Normal completion.  RETURN was entered or EXIT specified on the selection panel

	selectParse( RC, cmd, SEL_PGM, SEL_PARM, SEL_NEWAPPL, SEL_NEWPOOL, SEL_PASSLIB) ;

	if ( RC > 0 )
	{
		checkRCode( "Error in SELECT commmand format.  RC=" + d2ds( RC ) + "  Command passed was " + cmd ) ; 
		return ;
	}
	SEL         = true  ;
	busyAppl    = false ;

	wait_event()    ;
	debug1( "SELECT returned.  RC=" << RC << endl ) ;

	if ( RC == 4 ) propagateEnd = true ;

	SEL         = false ;
	SEL_PGM     = ""    ;
	SEL_PARM    = ""    ;
	SEL_NEWAPPL = ""    ;
	SEL_NEWPOOL = false ;
	SEL_PASSLIB = false ;
}


void pApplication::select( string pgm, string parm, string newappl, bool newpool, bool passlib )
{
	// SELECT function - this format is for internal use only.  Use keyword format of the SELECT for application programs. 
	// No variable substitution is done at this level.
	// RC=0  Normal completion of the selection panel or function.  END was entered.
	// RC=4  Normal completion.  RETURN was entered or EXIT specified on the selection panel

	SEL_PGM  = pgm  ;
	SEL_PARM = parm ;
	if ( !isvalidName( SEL_PGM ) ) { RC = 20 ; checkRCode( "Invalid program name passed to SELECT >>" + SEL_PGM + "<<" ) ; return ; }
	if ( (newappl != "") & !isvalidName4( newappl ) ) { RC = 20 ; checkRCode( "Invalid NEWAPPL pool name passed to SELECT >>" + newappl + "<<" ) ; return ; }
	SEL_NEWAPPL = newappl ;
	SEL_NEWPOOL = newpool ;
	SEL_PASSLIB = passlib ;

	SEL         = true  ;
	busyAppl    = false ;

	wait_event() ;

	if ( RC == 4 ) propagateEnd = true ;

	SEL         = false ;
	SEL_PGM     = ""    ;
	SEL_PARM    = ""    ;
	SEL_NEWAPPL = ""    ;
	SEL_NEWPOOL = false ;
	SEL_PASSLIB = false ;
}


void pApplication::pquery( string p_name, string a_name, string t_name, string w_name, string d_name, string r_name, string c_name )
{
	RC = 0 ;

	if ( !isvalidName( p_name ) ) { RC = 20 ; checkRCode( "Invalid panel name " + p_name + " on PQUERY" ) ; return ; }
	if ( !isvalidName( a_name ) ) { RC = 20 ; checkRCode( "Invalid area name " + a_name + " on PQUERY" ) ; return ; }

	if ( (t_name != "") && !isvalidName( t_name ) ) { RC = 20 ; checkRCode( "Invalid area type name " + t_name + " on PQUERY" ) ; return ; }
	if ( (w_name != "") && !isvalidName( w_name ) ) { RC = 20 ; checkRCode( "Invalid area width name " + w_name + " on PQUERY" ) ; return ; }
	if ( (d_name != "") && !isvalidName( d_name ) ) { RC = 20 ; checkRCode( "Invalid area depth name " + d_name + " on PQUERY" ) ; return ; }
	if ( (r_name != "") && !isvalidName( r_name ) ) { RC = 20 ; checkRCode( "Invalid area row number name " + r_name + " on PQUERY" ) ; return ; }
	if ( (c_name != "") && !isvalidName( c_name ) ) { RC = 20 ; checkRCode( "Invalid area column number name " + c_name + " on PQUERY" ) ; return ; }

	panel_create( p_name ) ;
	if ( panelList.find( p_name ) == panelList.end() )
	{
		RC = 20 ;
		checkRCode( "Panel " + p_name + " not found or invalid for PQUERY" ) ; return ;
		return  ;
	}
	panelList[ p_name ]->get_panel_info( RC, a_name, t_name, w_name, d_name, r_name, c_name ) ;
}


void pApplication::attr( string field, string attrs )
{
	if ( PANELID == "" )
	{
		RC = 20 ;
		checkRCode( "No panel has yet been loaded to change field attributes" ) ;
		return  ;
	}
	panelList[ PANELID ]->attr( RC, field, attrs ) ;
}


void pApplication::setmsg( string msg, msgSET sType )
{
	RC = 0 ;

	if ( ( sType == COND ) & setMSG ) return ;

	read_Message( msg ) ;
	if ( RC > 0 ) { RC = 20 ; return ; }
	MSGID  = msg  ;
	setMSG = true ;
}


string pApplication::get_help_member( int row, int col )
{
	string fld ;
	string hlp ;
	string paths ;

	RC  = 0  ;
	fld = currPanel->field_getname( row, col ) ;
	if ( fld != "" ) { hlp = currPanel->get_field_help( fld ) ; }

	if ( libdef_puser ) paths = mergepaths( ZPUSER, ZPLIB ) ;
	else                paths = ZPLIB                       ;

	return "M("+ZMHELP+") F("+currPanel->get_field_help( fld )+") P("+currPanel->ZPHELP+") A("+ZAHELP+") PATHS("+paths+")" ;
}


void pApplication::read_Message( string p_msg )
{
	// The message file name is determined by truncating the message ID after the second digit of the number.
	// AB123A file AB12
	// G012 file G01

	int i  ;
	int j  ;
	int p1 ;
	int p2 ;

	string s        ;
	string t        ;
	string p_msg_fn ;
	string filename ;
	string line2    ;
	string paths    ;
	string w1       ;
	string ws       ;
	string rest     ;
	bool found      ;
	char line1[ 256 ] ;

	RC       = 0     ;

	ZMHELP   = ""    ;
	ZMSGTYPE = WMT   ;
	ZMSGALRM = true  ;

	if ( !isvalidName( p_msg ) || p_msg.size() < 4 ) { RC = 20 ; checkRCode( "Invalid message format for " + p_msg ) ; return ; }

	found = false ;
	for ( i = 1 ; i < p_msg.size() - 2 ; i++ )
	{
		j = i + 1 ;
		if ( isdigit( p_msg[ i ] ) && isdigit( p_msg[ j ] ) )
		{
			p_msg_fn = substr( p_msg, 1, j + 1 ) ;
			found    = true ;
			break           ;
		}
	}

	if ( !found ) { RC = 20 ; checkRCode( "Message " + p_msg + " has invalid format" ) ; return ; }

	if ( libdef_muser ) paths = mergepaths( ZMUSER, ZMLIB ) ;
	else                paths = ZMLIB                       ;

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
				checkRCode( "Message file " + filename + " is not a regular file" ) ;
				return  ;
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
		RC = 20 ;
		checkRCode( "Message file " + p_msg_fn + " not found for message id " + p_msg ) ;
		return  ;
	}
	found = false ;
	ifstream messages ;
	messages.open( filename.c_str() ) ;
	while ( true )
	{
		messages.getline( line1, 256 ) ;
		if ( messages.fail() != 0 ) break ;

		line2.assign( line1, messages.gcount() - 1 ) ;

		w1 = word( line2, 1 )    ;
		ws = subword( line2, 2 ) ;

		if ( found )
		{
			p1 = pos( "\"", line2 ) ;
			if ( p1 > 0 )
			{
				p2 = pos( "\"", line2, p1+1 ) ;
				if ( p2 == 0 ) { RC = 20 ; break ; }
				ZLMSG = substr( line2, p1+1, p2-p1-1 ) ;
				rest  = delstr( line2, p1, p2-p1+1 )   ;
			}
			else
			{
				ZLMSG = word( line2, 1 )    ;
				rest  = subword( line2, 2 ) ;
			}
			if ( strip( rest ) != "" ) { RC = 20 ; break ; }
			ZLMSG  = sub_vars( ZLMSG ) ;
			break ;
		}
		if ( w1 == p_msg )
		{
			p1 = pos( "\"", line2 ) ;
			if ( p1 > 0 )
			{
				p2 = pos( "\"", line2, p1+1 ) ;
				if ( p2 == 0 ) { RC = 20 ; break ; }
				ZSMSG = " " + substr( line2, p1+1, p2-p1-1 ) ;
				rest  = substr( line2, p2+1) ;
			}
			else
			{
				ZSMSG = " " + word( line2, 2 ) ;
				rest  = subword( line2, 3 )    ;
			}
			p1 = pos( ".HELP=", rest ) ;
			if ( p1 > 0 )
			{
				p2 = pos( " ", rest, p1 ) ;
				if ( p2 == 0 ) { ZMHELP = substr( rest, p1+6 )          ; rest = delstr( rest, p1 )        ; }
				else           { ZMHELP = substr( rest, p1+6, p2-p1-6 ) ; rest = delstr( rest, p1, p2-p1 ) ; }
			}
			p1 = pos( ".TYPE=", rest ) ;
			if ( p1 > 0 )
			{
				p2 = pos( " ", rest, p1 ) ;
				if ( p2 == 0 ) { t  = substr( rest, p1+6 )          ; rest = delstr( rest, p1 )        ; }
				else           { t  = substr( rest, p1+6, p2-p1-6 ) ; rest = delstr( rest, p1, p2-p1 ) ; }
				if      ( t == "N" ) { ZMSGTYPE = IMT   ; ZMSGALRM = false ; }
				else if ( t == "W" ) { ZMSGTYPE = WMT   ; }
				else if ( t == "A" ) { ZMSGTYPE = AMT   ; }
				else if ( t == "C" ) { ZMSGTYPE = AMT   ; }
				else                 { RC = 20 ; break  ; }
			}
			p1 = pos( ".ALARM=", rest ) ;
			if ( p1 > 0 )
			{
				p2 = pos( " ", rest, p1 ) ;
				if ( p2 == 0 ) { t  = substr( rest, p1+7 )          ; rest = delstr( rest, p1 )        ; }
				else           { t  = substr( rest, p1+7, p2-p1-7 ) ; rest = delstr( rest, p1, p2-p1 ) ; }
				if      ( t == "YES" ) { ZMSGALRM = true  ; }
				else if ( t == "NO" )  { ZMSGALRM = false ; }
				else                   { RC = 20 ; break  ; }
			}

			if ( strip( rest ) != "" ) { RC = 20 ; break ; }
			ZSMSG = sub_vars( ZSMSG ) ;
			ZLMSG = ""   ;
			found = true ;
		}
	}
	if ( RC > 0 )
	{
		checkRCode( "Error in message format of " + p_msg + " found in message file " + p_msg_fn ) ;
	}
	else if ( !found )
	{
		RC = 20 ;
		checkRCode( "Message " + p_msg + " not found in message file " + p_msg_fn ) ;
	}
	messages.close() ;
	return ;
}


string pApplication::sub_vars( string s )
{
	int p1 ;
	int p2 ;

	string var ;
	string val ;

	p1 = 0 ;
	p2 = 0 ;

	while ( true )
	{
		p1 = s.find( '&', p1 ) ;
		if ( p1 == string::npos ) { break ; }
		p1++ ;
		if ( s[ p1 ] == '&' )
		{
			s.erase( p1,1 ) ;
			p1 = s.find_first_not_of( '&', p1 ) ;
			continue        ;
		}
		p2 = s.find_first_of( "& .'", p1 ) ;
		if ( p2 == string::npos )  { p2 = s.size() ; }
		var = s.substr( p1, (p2-p1) ) ;
		if ( isvalidName( var ) )
		{
			val = "" ;
			vcopy( var, val, MOVE ) ;
			if ( RC <= 8 )
			{
				if ( s[ p2 ] == '.' ) { s.replace( p1-1, var.size()+2, val ) ; }
				else                  { s.replace( p1-1, var.size()+1, val ) ; }
				if ( RC == 8 ) { p1-- ; }
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
	log( "-", "         Profile Pool Name: " << p_poolMGR->get( RC, "ZAPPLID", SHARED ) << endl ) ;
	log( "-", " " << endl ) ;
	log( "-", "Application Description . : " << ZAPPDESC << endl ) ;
	log( "-", "Last Panel Displayed. . . : " << PANELID << endl ) ;
	log( "-", "Last Message Displayed. . : " << MSGID << endl )   ;
	log( "-", "Number of Panels Loaded . : " << panelList.size() << endl )  ;
	log( "-", "Number of Open Tables . . : " << tablesOpen.size() << endl ) ;
	log( "-", " " << endl ) ;
	if ( testMode )
	{
		log( "-", "Application running in test mode" << endl ) ;
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


void pApplication::checkRCode( string s )
{
	log( "E", s << endl ) ;
	if ( ZERR2 != "" ) { log( "E", ZERR2 << endl ) ; }

	if ( (!ControlErrorsReturn) && (RC >= 12) )
	{
		log( "E", "RC=" << RC << " CONTROL ERRORS CANCEL is in effect.  Aborting" << endl ) ;
		ZERR1 = s ;
		control( "ERRORS", "RETURN" ) ;
		display( "PSYSER1" ) ;
		if ( RC <= 8 ) { errPanelissued = true ; }
		abend() ;
	}
}


void pApplication::cleanup()
{
	log( "I", "Shutting down application." << endl ) ;
	terminateAppl = true  ;
	busyAppl      = false ;
	log( "I", "Returning to calling program." << endl ) ;
	return ;
}


void pApplication::abend()
{
	log( "W", "Shutting down application due to an abnormal condition." << endl ) ;
	abnormalEnd   = true  ;
	terminateAppl = true  ;
	busyAppl      = false ;
	log( "W", "Application entering wait state" << endl ) ;
	boost::this_thread::sleep(boost::posix_time::seconds(31536000)) ;
}


void pApplication::set_forced_abend()
{
	log( "W", "Shutting down application due to a forced condition." << endl ) ;
	abnormalEnd       = true  ;
	abnormalEndForced = true  ;
	terminateAppl     = true  ;
	busyAppl          = false ;
}


void pApplication::closeLog()
{
	log( "I", "Closing application log" << endl ) ; 
	aplog.close() ;
}
