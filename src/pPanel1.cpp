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

pPanel::pPanel()
{
	p_row       = 0      ;
	p_col       = 0      ;
	abc_pos     = 2      ;
	showLMSG    = false  ;
	primaryMenu = false  ;
	selectPanel = false  ;
	scrollOn    = false  ;
	pdActive    = false  ;
	msgResp     = false  ;
	nretriev    = false  ;
	forEdit     = false  ;
	forBrowse   = false  ;
	nretfield   = ""     ;
	KEYLISTN    = ""     ;
	KEYAPPL     = ""     ;
	KEYHELPN    = ""     ;
	ALARM       = false  ;
	ZPRIM       = ""     ;
	panelTitle  = ""     ;
	panelDescr  = ""     ;
	abIndex     = 0      ;
	opt_field   = 0      ;
	tb_model    = false  ;
	tb_depth    = 0      ;
	tb_curidx   = -1     ;
	tb_csrrow   = -1     ;
	tb_fields   = ""     ;
	tb_clear    = ""     ;
	tb_scan     = false  ;
	tb_autosel  = false  ;
	tb_lcol     = 0      ;
	tb_lsz      = 0      ;
	full_screen = false  ;
	win_addpop  = false  ;
	msg_and_cmd = false  ;
	win_row     = 0      ;
	win_col     = 0      ;
	win_width   = 0      ;
	win_depth   = 0      ;
	dyn_depth   = 0      ;
	ZPHELP      = ""     ;
	cmdfield    = ""     ;
	Home        = ""     ;
	scroll      = "ZSCROLL" ;
	fwin        = NULL   ;
	pwin        = NULL   ;
	bwin        = NULL   ;
	idwin       = NULL   ;
	smwin       = NULL   ;
	lmwin       = NULL   ;
}


pPanel::~pPanel()
{
	// iterate over the 4 panel widget types, literal, field, dynArea, boxes and delete them.
	// Delete panel language statements in )INIT, )REINIT, )PROC, )ABCINIT and )ABCPROC sections.
	// Delete the main window/panel, popup panel and any message windows/panels created (free any userdata first)

	for ( auto it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		delete it->second ;
	}

	for ( auto it = dynAreaList.begin() ; it != dynAreaList.end() ; it++ )
	{
		delete it->second ;
	}

	for_each( literalList.begin(), literalList.end(),
		[](literal * a)
		{
			delete a ;
		} ) ;

	for_each( boxes.begin(), boxes.end(),
		[](Box * a)
		{
			delete a ;
		} ) ;
	for_each( initstmnts.begin(), initstmnts.end(),
		[](panstmnt * a)
		{
			delete a ;
		} ) ;
	for_each( reinstmnts.begin(), reinstmnts.end(),
		[](panstmnt * a)
		{
			delete a ;
		} ) ;
	for_each( procstmnts.begin(), procstmnts.end(),
		[](panstmnt * a)
		{
			delete a ;
		} ) ;
	for ( auto it = abc_initstmnts.begin() ; it != abc_initstmnts.end() ; it++ )
	{
		for_each( it->second.begin(), it->second.end(),
			[](panstmnt * a)
			{
				delete a ;
			} ) ;
	}
	for ( auto it = abc_procstmnts.begin() ; it != abc_procstmnts.end() ; it++ )
	{
		for_each( it->second.begin(), it->second.end(),
			[](panstmnt * a)
			{
				delete a ;
			} ) ;
	}

	if ( bwin )
	{
		panel_cleanup( bpanel ) ;
		del_panel( bpanel ) ;
		delwin( bwin )      ;
	}
	if ( fwin )
	{
		panel_cleanup( panel ) ;
		del_panel( panel )     ;
		delwin( fwin ) ;
	}
	if ( pwin )
	{
		delwin( pwin ) ;
	}
	if ( smwin )
	{
		panel_cleanup( smpanel ) ;
		del_panel( smpanel ) ;
		delwin( smwin )      ;
	}
	if ( lmwin )
	{
		panel_cleanup( lmpanel ) ;
		del_panel( lmpanel ) ;
		delwin( lmwin )      ;
	}
	if ( idwin )
	{
		panel_cleanup( idpanel ) ;
		del_panel( idpanel ) ;
		delwin( idwin )      ;
	}
}


void pPanel::init( errblock& err )
{
	err.setRC( 0 ) ;

	ZSCRMAXD = ds2d( p_poolMGR->get( err, "ZSCRMAXD", SHARED ) ) ;
	if ( err.getRC() > 0 )
	{
		err.seterrid( "PSYE015L", "GET", "ZSCRMAXD" ) ;
		return ;
	}

	ZSCRMAXW = ds2d( p_poolMGR->get( err, "ZSCRMAXW", SHARED ) ) ;
	if ( err.getRC() > 0 )
	{
		err.seterrid( "PSYE015L", "GET", "ZSCRMAXW" ) ;
		return ;
	}

	ZSCRNUM = ds2d(p_poolMGR->get( err, "ZSCRNUM", SHARED ) ) ;
	if ( err.getRC() > 0 )
	{
		err.seterrid( "PSYE015L", "GET", "ZSCRNUM" ) ;
		return ;
	}

	WSCRMAXD = ZSCRMAXD ;
	WSCRMAXW = ZSCRMAXW ;
	taskId   = err.taskid ;
}


string pPanel::getDialogueVar( errblock& err, const string& var )
{
	// Return the value of a dialogue variable (always as a string so convert int->string if necessary)
	// Search order is:
	//   Function pool defined
	//   Function pool implicit
	//   SHARED pool
	//   PROFILE pool
	// Function pool variables of type int are converted to string

	// Selection panels do not use the function pool.  Retrieve from shared or profile pools only

	// If the variable is not found, create a null implicit entry in the function pool

	// RC =  0 Normal completion
	// RC =  8 Variable not found
	// RC = 20 Severe error

	string * p_str    ;
	dataType var_type ;

	err.taskid = taskId ;

	if ( selectPanel )
	{
		return p_poolMGR->get( err, var, ASIS ) ;
	}

	var_type = p_funcPOOL->getType( err, var, NOCHECK ) ;
	if ( err.error() ) { return "" ; }

	if ( err.RC0() )
	{
		if ( var_type == INTEGER )
		{
			return d2ds( p_funcPOOL->get( err, 0, var_type, var ) ) ;
		}
		else
		{
			return p_funcPOOL->get( err, 0, var, NOCHECK ) ;
		}
	}
	else
	{
		p_str = p_poolMGR->vlocate( err, var ) ;
		if ( err.error() ) { return "" ; }
		switch ( err.getRC() )
		{
			case 0:
				 return *p_str ;
			case 4:
				 return p_poolMGR->get( err, var ) ;
			case 8:
				 p_funcPOOL->put( err, var, "" ) ;
		}
	}
	return "" ;
}


void pPanel::putDialogueVar( errblock& err, const string& var, const string& val )
{
	// Store data for a dialogue variable in the function pool.
	// Creates an implicit function pool variable if one does not already exist.

	// Selection panels do not use the function pool.  Put in SHARED or PROFILE
	// wherever it resides in ( or SHARED if not found )

	// RC =  0 Normal completion
	// RC = 20 Severe error

	if ( selectPanel )
	{
		p_poolMGR->put( err, var, val, ASIS ) ;
	}
	else
	{
		p_funcPOOL->put( err, var, val ) ;
	}
}


void pPanel::syncDialogueVar( errblock& err, const string& var )
{
	// Relevant for REXX procedures only and called for panel variables not in the function pool

	// Copy the dialogue variable to the function pool so REXX has access to it without doing a
	// VGET first.
	// This means variables that change in the SHARED/PROFILE pool, will be retrieved from the
	// function pool with the old values (VGET required first, in this case).

	// If the variable does not exist in any pool, create a null entry in the function pool so
	// a default value of blanks is used instead of the REXX variable name.

	// BUG:  ZPANELID and ZPFKEY are not sync'd as they are not in the SHARED pool at this point !!
	//       Old values are therefore always blank !!

	p_funcPOOL->put( err, var, p_poolMGR->get( err, var, ASIS ) ) ;
}


void pPanel::set_popup( int sp_row, int sp_col )
{
	win_addpop = true ;
	win_row    = (sp_row + win_depth + 1) < ZSCRMAXD ? sp_row : (ZSCRMAXD - win_depth - 1) ;
	win_col    = (sp_col + win_width + 1) < ZSCRMAXW ? sp_col : (ZSCRMAXW - win_width - 1) ;
}


void pPanel::move_popup()
{
	mvwin( win, win_row, win_col )      ;
	mvwin( bwin, win_row-1, win_col-1 ) ;
}


void pPanel::remove_popup()
{
	win_addpop = false ;
	win_row    = 0     ;
	win_col    = 0     ;
}


void pPanel::show_popup()
{
	if ( win != pwin ) { return ; }

	top_panel( bpanel ) ;
	top_panel( panel )  ;
}


void pPanel::hide_popup()
{
	if ( win != pwin ) { return ; }

	hide_panel( bpanel ) ;
	hide_panel( panel )  ;
}


void pPanel::toggle_fscreen( bool sp_popup, int sp_row, int sp_col )
{
	full_screen = !full_screen ;

	if ( !full_screen && sp_popup )
	{
		set_popup( sp_row, sp_col ) ;
	}
}


void pPanel::display_panel( errblock& err )
{
	// fwin - created unconditionally and is the full-screen window
	// pwin - pop-up window only created if the panel contains a WINDOW(w,d) statement
	// (associate ncurses panel 'panel' with whichever is active and set WSCRMAX?)

	// Use fwin if no pwin exists, or no ADDPOP() has been done and remove the popup
	// if no pwin exists, even if an ADDPOP() has been done (not exactly how real ISPF works).
	// The RESIZE lspf command can also toggle full screen mode.

	string winttl ;
	string panttl ;

	err.setRC( 0 ) ;

	update_keylist_vars( err ) ;

	if ( win_addpop && pwin && !full_screen )
	{
		win = pwin ;
		if ( win != panel_window( panel ) )
		{
			replace_panel( panel, win ) ;
		}
		mvwin( win, win_row, win_col )      ;
		mvwin( bwin, win_row-1, win_col-1 ) ;
		wattrset( bwin, cuaAttr[ AWF ] ) ;
		box( bwin, 0, 0 ) ;
		winttl = getDialogueVar( err, "ZWINTTL" ) ;
		if ( err.error() ) { return ; }
		if ( winttl != "" )
		{
			winttl = " "+ winttl +" " ;
			mvwaddstr( bwin, 0, ( win_width - winttl.size() ) / 2, winttl.c_str() ) ;
		}
		wattroff( bwin, cuaAttr[ AWF ] ) ;
		top_panel( bpanel ) ;
		top_panel( panel ) ;
		WSCRMAXW = win_width ;
		WSCRMAXD = win_depth ;
	}
	else
	{
		if ( win_addpop ) { remove_popup() ; }
		win = fwin ;
		if ( bwin )
		{
			hide_panel( bpanel ) ;
		}
		if ( win != panel_window( panel ) )
		{
			replace_panel( panel, win ) ;
		}
		top_panel( panel )  ;
		WSCRMAXW = ZSCRMAXW ;
		WSCRMAXD = ZSCRMAXD ;
	}

	werase( win ) ;

	panttl = sub_vars( panelTitle ) ;
	wattrset( win, cuaAttr[ PT ] ) ;
	mvwaddstr( win, ab.size() > 0 ? 2 : 0, ( WSCRMAXW - panttl.size() ) / 2, panttl.c_str() ) ;
	wattroff( win, cuaAttr[ PT ] ) ;

	if ( tb_model )
	{
		display_tb_mark_posn( err )    ;
		set_tb_fields_act_inact( err ) ;
	}

	display_ab() ;
	display_literals()    ;
	display_fields( err ) ;
	display_boxes() ;
	hide_pd()       ;
	display_pd( err )  ;
	display_msg( err ) ;
	display_id( err )  ;

	msg_and_cmd = ( msgid != "" && cmd_getvalue() != "" ) ;

	if ( ALARM ) { beep() ; }
}


void pPanel::redraw_fields( errblock& err )
{
	// Re-draw all fields in a window (eg after a .SHOW/.HIDE NULLS command)

	display_fields( err ) ;
}


void pPanel::refresh()
{
	// Refresh the screen by showing the current ncurses panel
	// (win always references the actual window in use, either fwin, or pwin)

	if ( win == pwin )
	{
		top_panel( bpanel ) ;
		touchwin( panel_window( bpanel ) ) ;
	}
	top_panel( panel ) ;
	touchwin( win ) ;
	return ;
}


void pPanel::display_panel_update( errblock& err )
{
	//  For all changed fields, remove the null character, apply attributes (upper case, left/right justified),
	//  and copy back to function pool.  Reset field for display.

	//  For dynamic areas, also update the shadow variable to indicate character deletes (0xFE) or nulls
	//  converted to spaced (0xFF)

	int fieldNum    ;
	int maxAmount   ;
	int Amnt        ;
	int p           ;
	int offset      ;
	int posn        ;

	string CMDVerb  ;
	string CMD      ;
	string fieldNam ;
	string sname    ;
	string msgfld   ;

	string * darea  ;
	string * shadow ;

	dataType var_type ;

	map<string, field *>::iterator it ;

	err.setRC( 0 ) ;

	fieldNum = -1 ;
	posn     = 1  ;
	msgid    = "" ;
	msgloc   = "" ;

	cursor_set  = false ;
	message_set = false ;

	p_funcPOOL->put( err, "ZERRMSG", "" ) ;
	if ( err.error() ) { return ; }

	CMDVerb = p_poolMGR->get( err, "ZVERB", SHARED ) ;
	if ( err.error() ) { return ; }

	end_pressed = findword( CMDVerb, "END EXIT RETURN" ) ;

	if ( scrollOn )
	{
		it = fieldList.find( scroll )  ;
		it->second->field_prep_input() ;
		it->second->field_set_caps()   ;
		it->second->field_changed = false ;
		if ( !isnumeric( it->second->field_value ) )
		{
			switch( it->second->field_value[ 0 ] )
			{
				case 'C': it->second->field_value = "CSR " ; break ;
				case 'D': it->second->field_value = "DATA" ; break ;
				case 'H': it->second->field_value = "HALF" ; break ;
				case 'P': it->second->field_value = "PAGE" ; break ;
				default:  set_message_cond( "PSYS011I" ) ;
					  set_cursor_cond( scroll )      ;
			}
		}
		p_funcPOOL->put( err, it->first, it->second->field_value ) ;
		if ( err.error() ) { return ; }
	}

	p_funcPOOL->put( err, "ZCURFLD", "" ) ;
	if ( err.error() ) { return ; }
	p_funcPOOL->put( err, "ZCURPOS", 1  ) ;
	if ( err.error() ) { return ; }

	tb_curidx = -1 ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( it->second->field_changed )
		{
			if ( it->second->field_dynArea )
			{
				it->second->field_remove_nulls_da() ;
				p        = it->first.find( '.' )    ;
				fieldNam = it->first.substr( 0, p ) ;
				fieldNum = ds2d( it->first.substr( p+1 ) ) ;
				darea    = p_funcPOOL->vlocate( err, fieldNam ) ;
				if ( err.error() ) { return ; }
				offset   = fieldNum*it->second->field_length      ;
				if ( it->second->field_dynArea->dynArea_DataModsp )
				{
					it->second->field_DataMod_to_UserMod( darea, offset ) ;
				}
				darea->replace( offset, it->second->field_length, it->second->field_value ) ;
				sname    = it->second->field_dynArea->dynArea_shadow_name ;
				shadow   = p_funcPOOL->vlocate( err, sname ) ;
				if ( err.error() ) { return ; }
				shadow->replace( offset, it->second->field_length, it->second->field_shadow_value ) ;
			}
			else
			{
				it->second->field_prep_input() ;
				it->second->field_set_caps()   ;
				var_type = p_funcPOOL->getType( err, it->first, NOCHECK ) ;
				if ( err.error() ) { return ; }
				if ( err.RC0() && var_type == INTEGER )
				{
					if ( datatype( it->second->field_value, 'W' ) )
					{
						p_funcPOOL->put( err, it->first, ds2d( it->second->field_value ) ) ;
					}
					else
					{
						set_message_cond( "PSYS011G" ) ;
						set_cursor_cond( it->first )   ;
					}
				}
				else if ( selectPanel )
				{
					p_poolMGR->put( err, it->first, it->second->field_value, ASIS ) ;
				}
				else
				{
					p_funcPOOL->put( err, it->first, it->second->field_value, NOCHECK ) ;
				}
				if ( err.error() ) { return ; }
			}
		}
		if ( it->second->field_active && it->second->cursor_on_field( p_row, p_col ) )
		{
			posn = p_col - it->second->field_col + 1 ;
			if ( it->second->field_dynArea )
			{
				p        = it->first.find( '.' )    ;
				fieldNam = it->first.substr( 0, p ) ;
				fieldNum = ds2d( it->first.substr( p+1 ) )  ;
				p_funcPOOL->put( err, "ZCURFLD", fieldNam ) ;
				if ( err.error() ) { return ; }
				p_funcPOOL->put( err, "ZCURPOS", ( fieldNum*it->second->field_length + posn ) ) ;
				if ( err.error() ) { return ; }
			}
			else
			{
				if ( it->second->field_tb )
				{
					p        = it->first.find( '.' )    ;
					fieldNam = it->first.substr( 0, p ) ;
					fieldNum = ds2d( it->first.substr( p+1 ) )  ;
					p_funcPOOL->put( err, "ZCURFLD", fieldNam ) ;
					if ( err.error() ) { return ; }
					tb_curidx = ds2d( it->first.substr( p+1 ) ) ;
				}
				else
				{
					p_funcPOOL->put( err, "ZCURFLD", it->first ) ;
					if ( err.error() ) { return ; }
				}
				p_funcPOOL->put( err, "ZCURPOS", posn ) ;
				if ( err.error() ) { return ; }
			}
		}
	}

	p_poolMGR->put( err, "ZSCROLLA", "",  SHARED ) ;
	if ( err.error() ) { return ; }

	p_poolMGR->put( err, "ZSCROLLN", "0", SHARED ) ;
	if ( err.error() ) { return ; }

	if ( scrollOn && findword( CMDVerb, "UP DOWN LEFT RIGHT" ) )
	{
		CMD    = upper( cmd_getvalue() ) ;
		msgfld = cmdfield ;
		if ( CMD == "" || ( !findword( CMD, "M MAX C CSR D DATA H HALF P PAGE" ) && !isnumeric( CMD ) ) )
		{
			CMD    = fieldList[ scroll ]->field_value ;
			msgfld = scroll ;
		}
		else
		{
			cmd_setvalue( err, "" ) ;
			p_funcPOOL->put( err, cmdfield, "" )  ;
			if ( err.error() ) { return ; }
		}
		if      ( tb_model )                          { maxAmount = tb_depth  ; }
		else if ( findword( CMDVerb, "LEFT RIGHT" ) ) { maxAmount = dyn_width ; }
		else                                          { maxAmount = dyn_depth ; }
		if ( isnumeric( CMD ) )
		{
			if ( CMD.size() > 6 )
			{
				set_message_cond( "PSYS011I" ) ;
				set_cursor_cond( msgfld )      ;
			}
			else
			{
				p_poolMGR->put( err, "ZSCROLLA", CMD, SHARED ) ;
				if ( err.error() ) { return ; }
				p_poolMGR->put( err, "ZSCROLLN", CMD, SHARED ) ;
				if ( err.error() ) { return ; }
			}
		}
		else if ( CMD.front() == 'M' )
		{
			p_poolMGR->put( err, "ZSCROLLA", "MAX", SHARED ) ;
			if ( err.error() ) { return ; }
		}
		else if ( CMD.front() == 'C' )
		{
			if ( fieldNum == -1 )
			{
				Amnt = maxAmount ;
			}
			else if ( CMDVerb.front() == 'U' )
			{
				Amnt = maxAmount - fieldNum - 1 ;
			}
			else if ( CMDVerb.front() == 'D' )
			{
				Amnt = fieldNum ;
			}
			else if ( CMDVerb.front() == 'L' )
			{
				Amnt = maxAmount - posn ;
			}
			else
			{
				Amnt = posn - 1 ;
			}
			p_poolMGR->put( err, "ZSCROLLN", Amnt == 0 ? d2ds( maxAmount ) : d2ds( Amnt ), SHARED ) ;
			if ( err.error() ) { return ; }
			p_poolMGR->put( err, "ZSCROLLA", "CSR", SHARED ) ;
			if ( err.error() ) { return ; }
		}
		else if ( CMD.front() == 'D' )
		{
			p_poolMGR->put( err, "ZSCROLLN", d2ds( maxAmount - 1 ), SHARED ) ;
			if ( err.error() ) { return ; }
			p_poolMGR->put( err, "ZSCROLLA", "DATA", SHARED ) ;
			if ( err.error() ) { return ; }
		}
		else if ( CMD.front() == 'H' )
		{
			p_poolMGR->put( err, "ZSCROLLN", d2ds( maxAmount/2 ), SHARED ) ;
			if ( err.error() ) { return ; }
			p_poolMGR->put( err, "ZSCROLLA", "HALF", SHARED ) ;
			if ( err.error() ) { return ; }
		}
		else
		{
			p_poolMGR->put( err, "ZSCROLLN", d2ds( maxAmount ), SHARED ) ;
			if ( err.error() ) { return ; }
			p_poolMGR->put( err, "ZSCROLLA", "PAGE", SHARED ) ;
			if ( err.error() ) { return ; }
		}
	}
}



void pPanel::display_panel_init( errblock& err )
{
	// Perform panel )INIT processing

	// Probably not correct, but call for each line on the screen if a table display,
	// or just call once if not.

	int ln( 0 ) ;

	cursor_set  = false ;
	message_set = false ;

	set_pfpressed( "" ) ;
	putDialogueVar( err, "ZPRIM", "" ) ;

	err.setRC( 0 ) ;
	do
	{
		process_panel_stmnts( err, ln, initstmnts ) ;
		if ( err.error() ) { break ; }
		ln++ ;
	} while ( ln < tb_depth ) ;
}


void pPanel::display_panel_reinit( errblock& err, int ln )
{
	// Perform panel )REINIT processing

	cursor_set  = false ;
	message_set = false ;

	set_pfpressed( "" ) ;

	err.setRC( 0 ) ;
	process_panel_stmnts( err, ln, reinstmnts ) ;
}


void pPanel::display_panel_proc( errblock& err, int ln )
{
	// Perform panel )PROC processing

	// If cursor on a point-and-shoot field (PS), set variable as in the )PNTS panel section if defined there
	// If a selection panel, check .TRAIL if not NOCHECK on the ZSEL variable and issue error

	int p ;

	string zsel     ;
	string fieldNam ;

	err.setRC( 0 ) ;

	fieldNam = p_funcPOOL->get( err, 0, "ZCURFLD" ) ;
	if ( err.error() ) { return ; }

	auto it2 = pntsTable.end() ;
	if ( fieldNam != "" )
	{
		auto it = fieldList.find( fieldNam ) ;
		if ( it != fieldList.end() && it->second->field_cua == PS )
		{
			it2 = pntsTable.find( fieldNam ) ;
		}
	}
	else
	{
		for ( auto it = literalPS.begin() ; it != literalPS.end() ; it++ )
		{
			if ( (*it)->cursor_on_literal( p_row, p_col ) )
			{
				it2 = pntsTable.find( (*it)->literal_name ) ;
				break ;
			}
		}
	}
	if ( it2 != pntsTable.end() )
	{
		putDialogueVar( err, it2->second.pnts_var, it2->second.pnts_val ) ;
		if ( err.error() ) { return ; }
	}

	process_panel_stmnts( err, ln, procstmnts ) ;
	if ( err.error() ) { return ; }

	if ( selectPanel )
	{
		zsel = getDialogueVar( err, "ZSEL" ) ;
		if ( err.error() ) { return ; }
		if ( zsel != "" )
		{
			p = wordpos( "NOCHECK", zsel ) ;
			if ( zsel.compare( 0, 5, "PANEL" )  != 0  &&
			     getControlVar( err, ".TRAIL" ) != "" && p == 0  )
			{
				zsel = "?" ;
			}
			else if ( p > 0 )
			{
				idelword( zsel, p, 1 ) ;
			}
			putDialogueVar( err, "ZSEL", zsel ) ;
			if ( err.error() ) { return ; }
		}
	}
}


void pPanel::abc_panel_init( errblock& err, const string& abc_desc )
{
	// Perform panel )ABCINIT processing

	cursor_set  = false ;
	message_set = false ;
	msgid       = ""    ;
	setControlVar( err, 0, ".ZVARS", "" ) ;

	process_panel_stmnts( err, 0, abc_initstmnts[ abc_desc ] ) ;
}


void pPanel::abc_panel_proc( errblock& err, const string& abc_desc )
{
	// Perform panel )ABCPROC processing

	cursor_set  = false ;
	message_set = false ;
	msgid       = ""    ;

	process_panel_stmnts( err, 0, abc_procstmnts[ abc_desc ] ) ;
}


void pPanel::process_panel_stmnts( errblock& err, int ln, vector<panstmnt* >& panstmnts )
{
	// General routine to process panel statements.  Pass address of the panel statements vector for the
	// panel section being processed.

	// For table display fields, .ATTR and .CURSOR apply to fields on line ln

	// If the panel statment has a ps_if address and a ps_xxx address, this is for statements coded
	// inline on the if-statement so test and execute if the if-statement is true.

	int if_column   ;

	string wd       ;
	string l        ;
	string s        ;
	string t        ;
	string u1       ;
	string val      ;
	string vars     ;
	string var      ;
	string fieldNam ;
	string fieldVal ;
	string g_label  ;

	bool if_skip    ;

	g_label     = ""    ;
	if_column   = 0     ;
	if_skip     = false ;

	for ( auto ips = panstmnts.begin() ; ips != panstmnts.end() ; ips++ )
	{
		if ( if_skip )
		{
			if ( if_column < (*ips)->ps_column )
			{
				continue ;
			}
			if_skip = false ;
		}
		if ( (*ips)->ps_if )
		{
			process_panel_if( err,
					  ln,
					  (*ips)->ps_if ) ;
			if ( err.error() ) { return ; }
			if_skip = !(*ips)->ps_if->if_true ;
			if ( if_skip )
			{
				if_column = (*ips)->ps_column ;
				continue ;
			}
		}
		else if ( (*ips)->ps_else )
		{
			if_skip = (*ips)->ps_else->if_true ;
			if ( if_skip )
			{
				if_column = (*ips)->ps_column ;
				continue ;
			}
		}
		if ( (*ips)->ps_exit ) { return ; }
		else if ( (*ips)->ps_goto )
		{
			g_label = (*ips)->ps_label ;
			for ( ; ips != panstmnts.end() ; ips++ )
			{
				if ( !(*ips)->ps_goto && (*ips)->ps_label == g_label )
				{
					g_label = "" ;
					break        ;
				}
				if ( (*ips)->ps_if )
				{
					(*ips)->ps_if->if_true = true ;
				}
				else if ( (*ips)->ps_else )
				{
					(*ips)->ps_else->if_true = false ;
				}
				continue ;
			}
			if ( g_label != "" )
			{
				err.seterrid( "PSYE041F", g_label ) ;
				return ;
			}
		}
		else if ( (*ips)->ps_refresh )
		{
			refresh_fields( err, (*ips)->ps_rlist ) ;
		}
		else if ( (*ips)->ps_vputget )
		{
			process_panel_vputget( err,
					       (*ips)->ps_vputget ) ;
			if ( err.error() ) { return ; }
		}
		else if ( (*ips)->ps_assgn )
		{
			process_panel_assignment( err,
						  ln,
						  (*ips)->ps_assgn ) ;
			if ( err.error() ) { return ; }
		}
		else if ( (*ips)->ps_ver )
		{
			if ( !message_set )
			{
				process_panel_verify( err,
						      ln,
						      (*ips)->ps_ver ) ;
				if ( err.error() ) { return ; }
			}
		}
	}
}


void pPanel::process_panel_if( errblock& err, int ln, IFSTMNT* ifstmnt )
{
	// Group AND statements as these have a higher precedence than OR

	bool if_AND     ;
	bool group_true ;

	process_panel_if_cond( err, ln, ifstmnt ) ;
	if ( err.error() || !ifstmnt->if_next ) { return ; }

	IFSTMNT* if_stmnt = ifstmnt ;

	group_true = ifstmnt->if_true ;
	while ( ifstmnt->if_next )
	{
		if_AND  = ifstmnt->if_AND  ;
		ifstmnt = ifstmnt->if_next ;
		process_panel_if_cond( err, ln, ifstmnt ) ;
		if ( err.error() ) { return ; }
		if ( if_AND )
		{
			group_true = group_true && ifstmnt->if_true ;
			while ( !group_true && ifstmnt->if_AND )
			{
				ifstmnt = ifstmnt->if_next ;
			}
		}
		else
		{
			if ( group_true )
			{
				if_stmnt->if_true = true ;
				return ;
			}
			group_true = ifstmnt->if_true ;
		}
	}

	if_stmnt->if_true = group_true ;
}


void pPanel::process_panel_if_cond( errblock& err, int ln, IFSTMNT* ifstmnt )
{
	int j ;

	bool l_break ;

	string rhs_val  ;
	string lhs_val  ;
	string fieldVal ;
	string fieldNam ;

	vector<string>::iterator it ;

	ifstmnt->if_true = ( ifstmnt->if_cond != IF_EQ ) ;

	if ( ifstmnt->if_lhs.front() == '.' )
	{
		lhs_val = getControlVar( err, ifstmnt->if_lhs ) ;
		if ( err.error() ) { return ; }
	}
	else if ( ifstmnt->if_verify )
	{
		fieldVal = getDialogueVar( err, ifstmnt->if_verify->ver_var ) ;
		if ( err.error() ) { return ; }
		if ( ifstmnt->if_verify->ver_nblank )
		{
			ifstmnt->if_true = ( fieldVal != "" ) ;
		}
		else if ( fieldVal == "" )
		{
			ifstmnt->if_true = true ;
		}
		if ( fieldVal != "" )
		{
			switch ( ifstmnt->if_verify->ver_type )
			{
			case VER_NUMERIC:
				ifstmnt->if_true = isnumeric( fieldVal ) ;
				break ;

			case VER_LIST:
			case VER_LISTX:
				for ( it  = ifstmnt->if_verify->ver_vlist.begin() ;
				      it != ifstmnt->if_verify->ver_vlist.end()  ; it++ )
				{
					if ( sub_vars( (*it) ) == fieldVal ) { break ; }
				}
				if ( ifstmnt->if_verify->ver_type == VER_LIST )
				{
					ifstmnt->if_true = ( it != ifstmnt->if_verify->ver_vlist.end() ) ;
				}
				else
				{
					ifstmnt->if_true = ( it == ifstmnt->if_verify->ver_vlist.end() ) ;
				}
				break ;

			case VER_PICT:
				ifstmnt->if_true = ispict( fieldVal, ifstmnt->if_verify->ver_vlist[ 0 ] ) ;
				break ;

			case VER_HEX:
				ifstmnt->if_true = ishex( fieldVal ) ;
				break ;

			case VER_OCT:
				ifstmnt->if_true = isoctal( fieldVal ) ;
			}
		}
		return ;
	}
	else if ( ifstmnt->if_trunc )
	{
		lhs_val = process_panel_trunc( err, ifstmnt->if_trunc ) ;
		if ( err.error() ) { return ; }
	}
	else
	{
		lhs_val = sub_vars( ifstmnt->if_lhs ) ;
	}
	for ( j = 0 ; j < ifstmnt->if_rhs.size() ; j++ )
	{
		l_break = false ;
		if ( ifstmnt->if_rhs[ j ].front() == '.' )
		{
			rhs_val = getControlVar( err, ifstmnt->if_rhs[ j ] ) ;
			if ( err.error() ) { return ; }
		}
		else
		{
			rhs_val = sub_vars( ifstmnt->if_rhs[ j ] ) ;
		}
		switch ( ifstmnt->if_cond )
		{
		case IF_EQ:
			ifstmnt->if_true = ifstmnt->if_true || ( lhs_val == rhs_val ) ;
			l_break = ifstmnt->if_true ;
			break ;

		case IF_NE:
			ifstmnt->if_true = ifstmnt->if_true && ( lhs_val != rhs_val ) ;
			l_break = !ifstmnt->if_true ;
			break ;

		case IF_GT:
			ifstmnt->if_true = ( lhs_val >  rhs_val ) ;
			break ;

		case IF_GE:
			ifstmnt->if_true = ( lhs_val >= rhs_val ) ;
			break ;

		case IF_LE:
			ifstmnt->if_true = ( lhs_val <= rhs_val ) ;
			break ;

		case IF_LT:
			ifstmnt->if_true = ( lhs_val <  rhs_val ) ;
			break ;

		}
		if ( l_break ) { break ; }
	}
}


string pPanel::process_panel_trunc( errblock& err, TRUNC* trunc )
{
	int p ;

	string t      ;
	string dTrail ;

	setControlVar( err, 0, ".TRAIL", "" ) ;

	if ( trunc->trnc_field.front() == '.' )
	{
		t = getControlVar( err, trunc->trnc_field.substr( 1 ) ) ;
	}
	else
	{
		t = getDialogueVar( err, trunc->trnc_field.substr( 1 ) ) ;
	}
	if ( err.error() ) { return "" ; }

	p = trunc->trnc_len ;
	if ( p > 0 )
	{
		if ( p > t.size() )
		{
			dTrail = "" ;
		}
		else
		{
			dTrail = strip( t.substr( p ) ) ;
			t      = t.substr( 0, p ) ;
		}
	}
	else
	{
		p = t.find( trunc->trnc_char ) ;
		if ( p == string::npos )
		{
			dTrail = "" ;
		}
		else
		{
			dTrail = strip( t.substr( p+1 ) ) ;
			t      = t.substr( 0, p ) ;
		}
	}

	setControlVar( err, 0, ".TRAIL", dTrail ) ;
	if ( err.error() ) { return "" ; }

	return t ;
}


string pPanel::process_panel_trans( errblock& err, int ln, TRANS* trans, const string& as_lhs )
{
	string fieldNam ;
	string t ;

	vector<pair<string,string>>::iterator itt ;

	if ( trans->trns_trunc )
	{
		t = process_panel_trunc( err, trans->trns_trunc ) ;
		if ( err.error() ) { return "" ; }
	}
	else
	{
		t = getDialogueVar( err, trans->trns_field ) ;
	}
	if ( err.error() ) { return "" ; }

	for ( itt = trans->trns_list.begin() ; itt != trans->trns_list.end() ; itt++ )
	{
		if ( t == sub_vars( itt->first ) )
		{
			t = sub_vars( itt->second ) ;
			break ;
		}
	}

	if ( itt == trans->trns_list.end() )
	{
		if ( trans->trns_default == "" )
		{
			t = "" ;
			if ( trans->trns_msgid != "" && !message_set )
			{
				set_message_cond( sub_vars( trans->trns_msgid ) ) ;
				if ( fieldList.count( trans->trns_field ) > 0 )
				{
					set_cursor_cond( trans->trns_field, ln ) ;
				}
				else if ( fieldList.count( as_lhs ) > 0 )
				{
					set_cursor_cond( as_lhs, ln ) ;
				}
			}
		}
		else if ( trans->trns_default != "*" )
		{
			t = sub_vars( trans->trns_default ) ;
		}
	}

	return t ;
}


void pPanel::process_panel_verify( errblock& err, int ln, VERIFY* verify )
{
	int i ;

	string fieldNam ;
	string fieldVal ;
	string msg ;
	string t   ;

	vector<string>::iterator it ;

	fieldNam = verify->ver_var ;

	fieldVal = getDialogueVar( err, verify->ver_var ) ;
	if ( err.error() ) { return ; }

	if ( verify->ver_nblank )
	{
		if ( fieldVal == "" )
		{
			set_message_cond( verify->ver_msgid == "" ? "PSYS019" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
	}
	if ( fieldVal == "" )
	{
		return ;
	}
	switch ( verify->ver_type )
	{
	case VER_NUMERIC:
		if ( !isnumeric( fieldVal ) )
		{
			set_message_cond( verify->ver_msgid == "" ? "PSYS011A" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_LIST:
	case VER_LISTX:
		for ( it = verify->ver_vlist.begin() ; it != verify->ver_vlist.end() ; it++ )
		{
			if ( sub_vars( (*it) ) == fieldVal ) { break ; }
		}
		if ( ( verify->ver_type == VER_LIST  && it == verify->ver_vlist.end() ) ||
		     ( verify->ver_type == VER_LISTX && it != verify->ver_vlist.end() ) )
		{
			if ( verify->ver_vlist.size() == 1 )
			{
				t  = "value is " ;
				t += sub_vars( verify->ver_vlist[ 0 ] ) ;
			}
			else
			{
				t = "values are " ;
				for ( i = 0 ; i < verify->ver_vlist.size() - 2 ; i++ )
				{
					t += sub_vars( verify->ver_vlist[ i ] ) + ", " ;
				}
				t += sub_vars( verify->ver_vlist[ i ] ) ;
				t += " and " ;
				i++ ;
				t += sub_vars( verify->ver_vlist[ i ] ) ;
			}
			msg = verify->ver_type == VER_LIST ? "PSYS011B" : "PSYS012Q" ;
			set_message_cond( verify->ver_msgid == "" ? msg : sub_vars( verify->ver_msgid ) ) ;
			if ( msgid == msg )
			{
				p_poolMGR->put( err, "ZZSTR1", t, SHARED ) ;
				if ( err.error() ) { return ; }
			}
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_PICT:
		if ( !ispict( fieldVal, verify->ver_vlist[ 0 ] ) )
		{
			p_poolMGR->put( err, "ZZSTR1", d2ds( verify->ver_vlist[ 0 ].size() ), SHARED ) ;
			if ( err.error() ) { return ; }
			p_poolMGR->put( err, "ZZSTR2", verify->ver_vlist[ 0 ], SHARED ) ;
			if ( err.error() ) { return ; }
			set_message_cond( verify->ver_msgid == "" ? "PSYS011N" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_HEX:
		if ( !ishex( fieldVal ) )
		{
			set_message_cond( verify->ver_msgid == "" ? "PSYS011H" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_OCT:
		if ( !isoctal( fieldVal ) )
		{
			set_message_cond( verify->ver_msgid == "" ? "PSYS011F" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
	}
}


void pPanel::process_panel_vputget( errblock& err, VPUTGET* vputget )
{
	// For select panels: VGET from PROFILE deletes from SHARED, otherwise has no effect
	//                    VPUT to SHARED  copies from PROFILE to SHARED (or null if not found in either)
	//                    VPUT to PROFILE copies from SHARED to PROFILE (or null) and deletes SHARED
	//                    BUG: VPUT has been a bit of a guess - probably not correct!

	int j  ;
	int ws ;

	string var ;
	string val ;

	ws = words( vputget->vpg_vars ) ;
	if ( vputget->vpg_vput )
	{
		for ( j = 1 ; j <= ws ; j++ )
		{
			var = sub_vars( word( vputget->vpg_vars, j ) ) ;
			if ( selectPanel )
			{
				if ( vputget->vpg_pool == SHARED )
				{
					val = p_poolMGR->get( err, var, PROFILE ) ;
					if ( err.RC0() )
					{
						p_poolMGR->put( err, var, val, SHARED ) ;
					}
					else if ( err.RC8() )
					{
						p_poolMGR->vlocate( err, var, SHARED ) ;
						if ( err.RC8() )
						{
							p_poolMGR->put( err, var, "", SHARED ) ;
						}
					}
					if ( err.error() ) { return ; }
				}
				else if ( vputget->vpg_pool == PROFILE )
				{
					val = p_poolMGR->get( err, var, SHARED ) ;
					if ( err.RC0() )
					{
						p_poolMGR->put( err, var, val, PROFILE ) ;
					}
					else if ( err.RC8() )
					{
						p_poolMGR->vlocate( err, var, PROFILE ) ;
						if ( err.RC8() )
						{
							p_poolMGR->put( err, var, "", PROFILE ) ;
						}
					}
					if ( err.error() ) { return ; }
				}
				else if ( vputget->vpg_pool == ASIS )
				{
					p_poolMGR->vlocate( err, var, ASIS ) ;
					if ( err.RC8() )
					{
						p_poolMGR->put( err, var, "", SHARED ) ;
					}
					if ( err.error() ) { return ; }
				}
			}
			else
			{
				val = p_funcPOOL->get( err, 8, var ) ;
				if ( err.RC0() )
				{
					p_poolMGR->put( err, var, val, vputget->vpg_pool ) ;
				}
				if ( err.error() ) { return ; }
			}
		}
	}
	else
	{
		for ( j = 1 ; j <= ws ; j++ )
		{
			var = sub_vars( word( vputget->vpg_vars, j ) ) ;
			if ( selectPanel )
			{
				if ( vputget->vpg_pool == PROFILE )
				{
					p_poolMGR->erase( err, var, SHARED ) ;
					if ( err.error() ) { return ; }
				}
			}
			else
			{
				val = p_poolMGR->get( err, var, vputget->vpg_pool ) ;
				if ( err.error() ) { return ; }
				if ( err.RC0() )
				{
					p_funcPOOL->put( err, var, val ) ;
				}
			}
		}
	}
	if ( err.RC8() ) { err.setRC( 0 ) ; }
}


void pPanel::process_panel_assignment( errblock& err, int ln, ASSGN* assgn )
{
	string t ;
	string fieldNam ;

	if ( assgn->as_function == AS_TRANS )
	{
		t = process_panel_trans( err, ln, assgn->as_trans, assgn->as_lhs ) ;
		if ( err.error() ) { return ; }
	}
	else if ( assgn->as_function == AS_TRUNC )
	{
		t = process_panel_trunc( err, assgn->as_trunc ) ;
		if ( err.error() ) { return ; }
	}
	else
	{
		if ( assgn->as_rhs.front() == '.' )
		{
			t = getControlVar( err, assgn->as_rhs ) ;
			if ( err.error() ) { return ; }
		}
		else if ( assgn->as_isvar )
		{
			t = getDialogueVar( err, assgn->as_rhs ) ;
			if ( err.error() ) { return ; }
		}
		else
		{
			t = sub_vars( assgn->as_rhs ) ;
		}
		switch ( assgn->as_function )
		{
		case AS_LENGTH:
			t = d2ds( t.size() ) ;
			break ;

		case AS_REVERSE:
			reverse( t.begin(), t.end() ) ;
			break ;

		case AS_UPPER:
			iupper( t ) ;
			break ;

		case AS_WORDS:
			t = d2ds( words( t ) ) ;
			break ;

		case AS_EXISTS:
			t = exists( t ) ? "1" : "0" ;
			break ;

		case AS_FILE:
			t = is_regular_file( t ) ? "1" : "0" ;
			break ;

		case AS_DIR:
			t = is_directory( t ) ? "1" : "0" ;
		}
	}

	if ( assgn->as_lhs.front() == '.' )
	{
		setControlVar( err, ln, assgn->as_lhs, t ) ;
		if ( err.error() ) { return ; }
	}
	else if ( assgn->as_isattr )
	{
		fieldNam = assgn->as_lhs ;
		if ( assgn->as_istb ) { fieldNam += "." + d2ds( ln ) ; }
		fieldList[ fieldNam ]->field_attr( err, t ) ;
		if ( err.error() ) { return ; }
		attrList.push_back( fieldNam ) ;
	}
	else
	{
		putDialogueVar( err, assgn->as_lhs, t ) ;
		if ( err.error() ) { return ; }
		if ( assgn->as_lhs == "ZPRIM" )
		{
			ZPRIM = t ;
			primaryMenu = ( t == "YES" ) ;
		}
	}
}


string pPanel::getControlVar( errblock& err, const string& svar )
{
	if ( svar == ".ALARM" )
	{
		return ALARM ? "YES" : "NO" ;
	}
	else if ( svar == ".AUTOSEL" )
	{
		return tb_autosel ? "YES" : "NO" ;
	}
	else if ( svar == ".BROWSE" )
	{
		return forBrowse ? "YES" : "NO" ;
	}
	else if ( svar == ".CSRPOS" )
	{
		return d2ds( p_funcPOOL->get( err, 0, INTEGER, "ZCURPOS" ), 4 ) ;
	}
	else if ( svar == ".CSRROW" )
	{
		return d2ds( p_funcPOOL->get( err, 0, INTEGER, "ZCURINX" ), 8 ) ;
	}
	else if ( svar == ".CURSOR" )
	{
		return p_funcPOOL->get( err, 0, "ZCURFLD" ) ;
	}
	else if ( svar == ".EDIT" )
	{
		return forEdit ? "YES" : "NO" ;
	}
	else if ( svar == ".FALSE" )
	{
		return "0" ;
	}
	else if ( svar == ".HELP" )
	{
		return ZPHELP ;
	}
	else if ( svar == ".MSG" )
	{
		return msgid ;
	}
	else if ( svar == ".NRET" )
	{
		return nretriev ? "ON" : "OFF" ;
	}
	else if ( svar == ".PFKEY" )
	{
		return get_pfpressed() ;
	}
	else if ( svar == ".RESP" )
	{
		return end_pressed ? "END" : "ENTER" ;
	}
	else if ( svar == ".TRUE" )
	{
		return "1" ;
	}
	else if ( svar == ".TRAIL" )
	{
		return p_funcPOOL->get( err, 8, ".TRAIL", NOCHECK ) ;
	}
	else if ( svar == ".ZVARS" )
	{
		return p_funcPOOL->get( err, 8, ".ZVARS", NOCHECK ) ;
	}
	return svar ;
}


void pPanel::setControlVar( errblock& err, int ln, const string& svar, const string& sval )
{
	if ( svar == ".ALARM" )
	{
		if ( sval == "YES" )
		{
			ALARM = true ;
		}
		else if ( sval == "NO" || sval == "" )
		{
			ALARM = false ;
		}
		else
		{
			err.seterrid( "PSYE041G" ) ;
		}
	}
	else if ( svar == ".AUTOSEL" )
	{
		if ( sval == "YES" || sval == "" )
		{
			tb_autosel = true ;
		}
		else if ( sval == "NO" )
		{
			tb_autosel = false ;
		}
		else
		{
			err.seterrid( "PSYE042O" ) ;
		}
	}
	else if ( svar == ".BROWSE" )
	{
		forBrowse = ( sval == "YES" ) ;
	}
	else if ( svar == ".CSRPOS" )
	{
		if ( !isnumeric( sval ) || ds2d( sval ) < 1 )
		{
			err.seterrid( "PSYE041J" ) ;
			return ;
		}
		curpos = ds2d( sval ) ;
		p_funcPOOL->put( err, "ZCURPOS", curpos ) ;
		if ( err.error() ) { return ; }
	}
	else if ( svar == ".CSRROW" )
	{
		if ( !isnumeric( sval ) || ds2d( sval ) < 0 )
		{
			err.seterrid( "PSYE041H" ) ;
			return ;
		}
		tb_csrrow = ds2d( sval ) ;
		p_funcPOOL->put( err, "ZCURINX", tb_csrrow ) ;
		if ( err.error() ) { return ; }
	}
	else if ( svar == ".CURSOR" )
	{
		if ( !cursor_set )
		{
			p_funcPOOL->put( err, "ZCURFLD", sval ) ;
			if ( err.error() ) { return ; }
			curfld = wordpos( sval, tb_fields ) == 0 ? sval : sval +"."+ d2ds( ln ) ;
			curpos = 1 ;
			cursor_set = true ;
		}
	}
	else if ( svar == ".EDIT" )
	{
		forEdit = ( sval == "YES" ) ;
	}
	else if ( svar == ".HELP" )
	{
		ZPHELP = sval ;
	}
	else if ( svar == ".NRET" )
	{
		if      ( sval == "ON" )        { nretriev  = true  ; }
		else if ( isvalidName( sval ) ) { nretfield = sval  ; }
		else                            { nretriev  = false ; }
	}
	else if ( svar == ".MSG" )
	{
		set_message_cond( sval ) ;
	}
	else if ( svar == ".RESP" )
	{
		if ( sval == "ENTER" )
		{
			p_poolMGR->put( err, "ZVERB", "", SHARED ) ;
			if ( err.error() ) { return ; }
			end_pressed = false ;
		}
		else if ( sval == "END" )
		{
			p_poolMGR->put( err, "ZVERB", "END", SHARED ) ;
			if ( err.error() ) { return ; }
			end_pressed = true ;
		}
		else
		{
			err.seterrid( "PSYE041I" ) ;
		}
	}
	else if ( svar == ".TRAIL" )
	{
		p_funcPOOL->put( err, ".TRAIL", sval, NOCHECK ) ;
	}
	else if ( svar == ".ZVARS" )
	{
		if ( sval != "" && !isvalidName( sval ) )
		{
			err.seterrid( "PSYE013A", sval, ".ZVARS )ABCINIT statement" ) ;
			return  ;
		}
		p_funcPOOL->put( err, ".ZVARS", sval, NOCHECK ) ;
	}
}


void pPanel::set_message_cond( const string& msg )
{
	errblock err ;

	if ( !message_set )
	{
		msgid       = msg  ;
		message_set = true ;
		p_funcPOOL->put( err, "ZERRMSG", msg ) ;
	}
}


void pPanel::set_cursor_cond( const string& csr, int i )
{
	errblock err ;

	if ( !cursor_set )
	{
		curfld = csr ;
		msgloc = csr ;
		curidx = i   ;
		p_funcPOOL->put( err, "ZCURFLD", csr ) ;
		cursor_set = true ;
	}
}


void pPanel::set_cursor( const string& csr, int p )
{
	curfld = csr ;
	curpos = p   ;
}


void pPanel::set_cursor_home()
{
	curfld = Home ;
	curpos = 1    ;
}


string pPanel::get_cursor()
{
	if ( findword( curfld, tb_fields ) )
	{
		return curfld + "." + d2ds( curidx ) ;
	}
	return curfld ;
}


string pPanel::get_msgloc()
{
	if ( findword( msgloc, tb_fields ) )
	{
		return msgloc + "." + d2ds( curidx ) ;
	}
	return msgloc ;
}


void pPanel::refresh_fields( errblock& err, const string& fields )
{
	// Update the field value from the dialogue variable.  Apply any field justification defined.

	int j ;
	int k ;

	string sname    ;
	string * darea  ;
	string * shadow ;

	map<string, field *>::iterator   itf ;
	map<string, dynArea *>::iterator itd ;

	for ( itf = fieldList.begin() ; itf != fieldList.end() ; itf++ )
	{
		if ( fields != "*" && !findword( itf->first, fields ) ) { continue ; }
		if ( itf->second->field_dynArea ) { continue ; }
		itf->second->field_value = getDialogueVar( err, itf->first ) ;
		if ( err.error() ) { return ; }
		itf->second->field_prep_display() ;
	}

	for ( itd = dynAreaList.begin() ; itd != dynAreaList.end() ; itd++ )
	{
		if ( fields != "*" && !findword( itd->first, fields ) ) { continue ; }
		k      = itd->second->dynArea_width ;
		darea  = p_funcPOOL->vlocate( err, itd->first ) ;
		if ( err.error() ) { return ; }
		sname  = itd->second->dynArea_shadow_name ;
		shadow = p_funcPOOL->vlocate( err, sname ) ;
		if ( err.error() ) { return ; }
		for ( int i = 0 ; i < itd->second->dynArea_depth ; i++ )
		{
			j   = i * itd->second->dynArea_width ;
			itf = fieldList.find( itd->first +"."+ d2ds( i ) ) ;
			itf->second->field_value        = darea->substr( j, k )  ;
			itf->second->field_shadow_value = shadow->substr( j, k ) ;
		}
	}
}


void pPanel::refresh_fields( errblock& err )
{
	// Update all field values from their dialogue variables and display.

	int j   ;
	int k   ;

	string sname    ;
	string * darea  ;
	string * shadow ;

	map<string, field *>::iterator   itf ;
	map<string, dynArea *>::iterator itd ;

	bool snulls = ( p_poolMGR->get( err, "ZNULLS", SHARED ) == "YES" ) ;
	if ( err.error() ) { return ; }
	char pad    = p_poolMGR->get( err, "ZPADC", PROFILE ).front() ;
	if ( err.error() ) { return ; }

	for ( itf = fieldList.begin() ; itf != fieldList.end() ; itf++ )
	{
		if ( itf->second->field_dynArea ) { continue ; }
		itf->second->field_value = getDialogueVar( err, itf->first ) ;
		if ( err.error() ) { return ; }
		itf->second->field_prep_display() ;
		itf->second->display_field( win, pad, snulls ) ;
	}

	for ( itd = dynAreaList.begin() ; itd != dynAreaList.end() ; itd++ )
	{
		k      = itd->second->dynArea_width ;
		darea  = p_funcPOOL->vlocate( err, itd->first ) ;
		if ( err.error() ) { return ; }
		sname  = itd->second->dynArea_shadow_name ;
		shadow = p_funcPOOL->vlocate( err, sname ) ;
		if ( err.error() ) { return ; }
		for ( int i = 0 ; i < itd->second->dynArea_depth ; i++ )
		{
			j   = i * itd->second->dynArea_width ;
			itf = fieldList.find( itd->first +"."+ d2ds( i ) ) ;
			itf->second->field_value        = darea->substr( j, k )  ;
			itf->second->field_shadow_value = shadow->substr( j, k ) ;
			itf->second->display_field( win, pad, snulls ) ;
		}
	}
}


void pPanel::create_tbfield( errblock& err, const string& pline )
{
	// Default is JUST(ASIS) for fields of a TB model, so change from the default of JUST(LEFT)
	// Create implicit function pool variables for the TB field.

	int tlen ;
	int tcol ;
	int ws   ;

	string w2   ;
	string w3   ;
	string w4   ;
	string opts ;
	string name ;
	string nidx ;

	cuaType fType ;

	field   a   ;
	field * fld ;

	ws = words( pline ) ;
	if ( ws < 6 )
	{
		err.seterrid( "PSYE035P" ) ;
		return ;
	}
	w2   = word( pline, 2 ) ;
	w3   = word( pline, 3 ) ;
	w4   = word( pline, 4 ) ;

	opts = upper( subword( pline, 5, ws-5 ) ) ;
	name = word( pline, ws ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE031F", name, "TBFIELD" ) ;
		return  ;
	}

	if ( findword( name, tb_fields ) )
	{
		err.seterrid( "PSYE041A", name ) ;
		return  ;
	}

	if ( w2.size() > 1 && w2.front() == '+' )
	{
		if ( w2.size() > 2 && w2[ 1 ] == '+' )
		{
			tcol = tb_lcol + tb_lsz + ds2d( substr( w2, 3 ) ) ;
		}
		else
		{
			tcol = tb_lcol + ds2d( substr( w2, 2 ) ) ;
		}
	}
	else
	{
		if      ( isnumeric( w2 ) )                 { tcol = ds2d( w2 ) ; }
		else if ( w2 == "MAX" )                     { tcol = WSCRMAXW   ; }
		else if ( w2.compare( 0, 4, "MAX-" ) == 0 ) { tcol = WSCRMAXW - ds2d( substr( w2, 5 ) ) ; }
		else
		{
			err.seterrid( "PSYE031B", w2 ) ;
			return ;
		}
	}

	if      ( isnumeric( w3 ) )                 { tlen = ds2d( w3 ) ; }
	else if ( w3 == "MAX" )                     { tlen = WSCRMAXW - tcol + 1 ; }
	else if ( w3.compare( 0, 4, "MAX-" ) == 0 ) { tlen = WSCRMAXW - tcol - ds2d( substr( w3, 5 ) ) + 1 ; }
	else
	{
		err.seterrid( "PSYE031B", w3 ) ;
		return ;
	}

	tb_lcol = tcol ;
	tb_lsz  = tlen ;

	if ( cuaAttrName.count( w4 ) == 0 )
	{
		err.seterrid( "PSYE032F", w4 ) ;
		return ;
	}
	fType = cuaAttrName[ w4 ] ;

	a.field_just = 'A'        ;
	a.field_opts( err, opts ) ;
	if ( err.error() ) { return ; }

	a.field_cua    = fType    ;
	a.field_col    = tcol - 1 ;
	a.field_length = tlen     ;
	a.field_cole   = tcol - 1 + tlen ;
	a.field_input  = ( cuaAttrUnprot.count( fType ) > 0 ) ;
	a.field_tb     = true ;

	for ( int i = 0 ; i < tb_depth ; i++ )
	{
		fld  = new field ;
		*fld = a ;
		fld->field_row = tb_row + i ;
		nidx = name +"."+ d2ds( i ) ;
		fieldList[ nidx ] = fld ;
		p_funcPOOL->put( err, nidx, "", NOCHECK ) ;
		if ( err.error() ) { return ; }
	}
	tb_fields += " " + name ;

}


void pPanel::create_pdc( errblock& err, const string& abc_desc, const string& pline )
{
	// ab is a vector list of action-bar-choices (abc objects)
	// Each action-bar-choice is a vector list of pull-down-choices (pdc objects)

	int i ;
	int p ;

	string head ;
	string tail ;

	string pdc_desc ;
	string pdc_run  ;
	string pdc_parm ;
	string pdc_unavail ;

	abc t_abc( p_funcPOOL, selectPanel ) ;

	p = wordpos( "ACTION", pline ) ;
	if ( p > 0 )
	{
		head = subword( pline, 2, p-2 ) ;
		tail = subword( pline, p+1 ) ;
	}
	else
	{
		head = subword( pline, 2 ) ;
	}

	pdc_desc = extractKWord( err, head, "DESC()" ) ;
	if ( err.error() ) { return ; }

	pdc_unavail = parseString( err, head, "UNAVAIL()" ) ;
	if ( err.error() ) { return ; }

	if ( pdc_unavail != "" && !isvalidName( pdc_unavail ) )
	{
		err.seterrid( "PSYE031D", pdc_unavail ) ;
		return ;
	}

	if ( head != "" )
	{
		err.seterrid( "PSYE032H", head ) ;
		return ;
	}

	pdc_run = parseString( err, tail, "RUN()" ) ;
	if ( err.error() ) { return ; }
	pdc_parm = extractKWord( err, tail, "PARM()" ) ;
	if ( err.error() ) { return ; }

	if ( tail != "" )
	{
		err.seterrid( "PSYE032H", tail ) ;
		return ;
	}

	for ( i = 0 ; i < ab.size() ; i++ )
	{
		if ( ab.at( i ).abc_desc == abc_desc ) { break ; }
	}

	if ( i == ab.size() )
	{
		t_abc.abc_desc = abc_desc ;
		t_abc.abc_col  = abc_pos  ;
		abc_pos        = abc_pos + abc_desc.size() + 2 ;
		ab.push_back( t_abc ) ;
	}

	ab.at( i ).add_pdc( pdc( pdc_desc, pdc_run, pdc_parm, pdc_unavail ) ) ;
}


void pPanel::update_field_values( errblock& err )
{
	// Update field_values from the dialogue variables (may not exist so treat RC=8 from getDialogueVar as normal completion)

	// Treat dynamic areas differently - they must reside in the function pool.  Use vlocate to get the dynamic area variables
	// via their addresses to avoid large string copies

	int j ;
	int k ;

	string sname    ;
	string * darea  ;
	string * shadow ;

	map<string, field   *>::iterator itf ;
	map<string, dynArea *>::iterator itd ;

	err.setRC( 0 ) ;

	for ( itf = fieldList.begin() ; itf != fieldList.end() ; itf++ )
	{
		if ( !itf->second->field_dynArea )
		{
			itf->second->field_value = getDialogueVar( err, itf->first ) ;
			if ( err.error() ) { return ; }
		}
		itf->second->field_changed = false ;
	}
	if ( err.RC8() ) { err.setRC( 0 ) ; }

	for ( itd = dynAreaList.begin() ; itd != dynAreaList.end() ; itd++ )
	{
		darea = p_funcPOOL->vlocate( err, itd->first ) ;
		if ( !err.RC0() )
		{
			err.seterrid( "PSYE032G", itd->first ) ;
			return ;
		}
		sname  = itd->second->dynArea_shadow_name ;
		shadow = p_funcPOOL->vlocate( err, sname ) ;
		if ( !err.RC0() )
		{
			err.seterrid( "PSYE032I", sname ) ;
			return ;
		}
		if ( darea->size() > shadow->size() )
		{
			llog( "W", "Shadow variable '"+ sname +"' size is smaller than the data variable " << itd->first << " size.  Results are unpredictable" << endl ) ;
			llog( "W", "Data variable size   = " << darea->size() << endl ) ;
			llog( "W", "Shadow variable size = " << shadow->size() << endl ) ;
		}
		darea->resize( itd->second->dynArea_width * itd->second->dynArea_depth, ' ' )   ;
		shadow->resize( itd->second->dynArea_width * itd->second->dynArea_depth, 0xFF ) ;
		k = itd->second->dynArea_width ;
		for ( int i = 0 ; i < itd->second->dynArea_depth ; i++ )
		{
			j   = i * itd->second->dynArea_width ;
			itf = fieldList.find( itd->first +"."+ d2ds( i ) ) ;
			itf->second->field_value        = darea->substr( j, k )  ;
			itf->second->field_shadow_value = shadow->substr( j, k ) ;
		}
	}
}


void pPanel::display_literals()
{
	// Substitute dialogue variables and place result in literal_xvalue.  If there are no
	// dialogue variables present, set literal_dvars to false and clear literal_value ;

	for ( auto it = literalList.begin() ; it != literalList.end() ; it++ )
	{
		if ( (*it)->literal_dvars )
		{
			(*it)->literal_xvalue = sub_vars( (*it)->literal_value, (*it)->literal_dvars ) ;
			if ( !(*it)->literal_dvars )
			{
				(*it)->literal_value = "" ;
			}
		}
		(*it)->literal_display( win ) ;
	}
}


void pPanel::display_fields( errblock& err )
{
	map<string, field *>::iterator it;

	bool snulls = ( p_poolMGR->get( err, "ZNULLS", SHARED ) == "YES" ) ;
	if ( err.error() ) { return ; }
	char pad    = p_poolMGR->get( err, "ZPADC", PROFILE ).front() ;
	if ( err.error() ) { return ; }

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( !it->second->field_dynArea )
		{
			it->second->field_prep_display() ;
		}
		it->second->display_field( win, pad, snulls ) ;
	}
}


void pPanel::display_ab()
{
	if ( ab.size() == 0 ) { return ; }

	for ( int i = 0 ; i < ab.size() ; i++ )
	{
		ab[ i ].display_abc_unsel( win ) ;
	}

	wattrset( win, cuaAttr[ ABSL ] ) ;
	mvwhline( win, 1, 0, ACS_HLINE, WSCRMAXW ) ;
	wattroff( win, cuaAttr[ ABSL ] ) ;
}


void pPanel::resetAttrs()
{
	int i ;

	for ( i = 0 ; i < attrList.size() ; i++ )
	{
		fieldList[ attrList[ i ] ]->field_attr() ;
	}
	attrList.clear() ;
}


void pPanel::cursor_to_cmdfield( int& RC1, int f_pos )
{
	if ( cmdfield != "" ) { cursor_to_field( RC1, cmdfield, f_pos ) ; }
}


void pPanel::cursor_to_field( int& RC1, string f_name, int f_pos )
{
	int oX ;
	int oY ;

	map<string, field   *>::iterator itf ;
	map<string, dynArea *>::iterator itd ;

	RC1 = 0 ;

	if ( f_name == "" )
	{
		f_name = get_cursor() ;
		f_pos  = curpos ;
		if ( f_name == "" ) { return ; }
	}

	itf = fieldList.find( f_name ) ;
	if ( itf == fieldList.end() )
	{
		itd = dynAreaList.find( f_name ) ;
		if ( itd == dynAreaList.end() )
		{
			p_col = 0 ;
			p_row = 0 ;
			RC1   = !isvalidName( f_name ) ? 20 : 12 ;
		}
		else
		{
			if ( f_pos < 1 ) { f_pos = 1 ; }
			f_pos-- ;
			oX = f_pos % itd->second->dynArea_width ;
			oY = f_pos / itd->second->dynArea_width ;
			if ( oY >= itd->second->dynArea_depth )
			{
				oX = 0 ;
				oY = 0 ;
			}
			p_col = itd->second->dynArea_col + oX ;
			p_row = itd->second->dynArea_row + oY ;
		}
	}
	else
	{
		if ( f_pos < 1 || f_pos > itf->second->field_length ) { f_pos = 1 ; }
		p_col = itf->second->field_col + f_pos - 1 ;
		p_row = itf->second->field_row ;
	}
}


void pPanel::get_home( uint& row, uint& col )
{
	// Return the physical position on the screen

	map<string, field *>::iterator it ;

	it = fieldList.find( Home ) ;
	if ( it == fieldList.end() )
	{
		row = 0 ;
		col = 0 ;
	}
	else
	{
		row = it->second->field_row ;
		col = it->second->field_col ;
	}
	row += win_row ;
	col += win_col ;
}


string& pPanel::field_getvalue( const string& f_name )
{
	map<string, field *>::iterator it ;

	it = fieldList.find( f_name )   ;
	it->second->field_prep_input()  ;
	return  it->second->field_value ;
}


void pPanel::field_setvalue( errblock& err, const string& f_name, const string& f_value )
{
	bool snulls = ( p_poolMGR->get( err, "ZNULLS", SHARED ) == "YES" ) ;
	if ( err.error() ) { return ; }
	char pad    = p_poolMGR->get( err, "ZPADC", PROFILE ).front() ;
	if ( err.error() ) { return ; }

	map<string, field *>::iterator it ;

	it = fieldList.find( f_name ) ;
	it->second->field_value   = f_value ;
	it->second->field_changed = true    ;
	it->second->field_prep_display()    ;
	it->second->display_field( win, pad, snulls ) ;
}


string& pPanel::cmd_getvalue()
{
	if ( fieldList.find( cmdfield ) == fieldList.end() )
	{
		return ZZCMD ;
	}
	return field_getvalue( cmdfield ) ;
}


void pPanel::cmd_setvalue( errblock& err, const string& v )
{
	if ( fieldList.find( cmdfield ) == fieldList.end() )
	{
		ZZCMD = v ;
	}
	else
	{
		field_setvalue( err, cmdfield, v ) ;
	}
}


bool pPanel::is_cmd_inactive( const string& cmd )
{
	// Some commands may be inactive for this type of panel, so return true for these.
	// Not sure how real ISPF does this without setting ZCTACT to NOP !! but this works okay.

	// Control variable .EDIT=YES activates RFIND and RCHANGE for a panel
	// Control variable .BROWSE=YES activates RFIND for a panel

	if ( findword( cmd, "LEFT RIGHT" ) )
	{
		if ( LRScroll ) { return false ; }
		if ( tb_model ) { return true  ; }
	}
	else if ( cmd == "NRETRIEV" && !nretriev ) { return true ; }
	else if ( cmd == "RFIND"    && ( !forEdit && !forBrowse ) ) { return true ; }
	else if ( cmd == "RCHANGE"  && !forEdit   ) { return true ; }

	if ( !scrollOn && findword( cmd, "UP DOWN LEFT RIGHT" ) ) { return true ; }
	return false ;
}


bool pPanel::field_valid( const string& f_name )
{
	return fieldList.find( f_name ) != fieldList.end() ;
}


string pPanel::field_getname( uint row, uint col )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field

	map<string, field *>::iterator it ;

	row -= win_row ;
	col -= win_col ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( it->second->cursor_on_field( row, col ) )
		{
			if ( !it->second->field_active ) { return "" ; }
			return it->first ;
		}
	}
	return "" ;
}


bool pPanel::field_get_row_col( const string& fld, uint& row, uint& col )
{
	// If field found on panel (by name), return true and its position, else return false
	// Return the physical position on the screen, so add the window offsets to field_row/col

	auto it = fieldList.find( fld ) ;
	if ( it == fieldList.end() || !it->second->field_active ) { return false ; }

	row = it->second->field_row + win_row ;
	col = it->second->field_col + win_col ;

	return true ;
}


void pPanel::field_get_col( const string& fld, uint& col )
{
	// If field found on panel (by name), return column position.
	// Return the physical position on the screen, so add the window offsets to field_col

	auto it = fieldList.find( fld ) ;
	if ( it != fieldList.end() )
	{
		if ( it->second->field_active )
		{
			col = it->second->field_col + win_col ;
		}
	}
}


fieldExc pPanel::field_getexec( const string& field )
{
	// If passed field is in the field execute table, return the structure fieldExc
	// for that field as defined in )FIELD panel section.

	if ( fieldExcTable.find( field ) == fieldExcTable.end() ) { return fieldExc() ; }
	return fieldExcTable[ field ] ;
}


void pPanel::field_edit( errblock& err, uint row, uint col, char ch, bool Isrt, bool& prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field
	// field_tab_next also needs the physical position, so addjust before and after the call.

	bool snulls = ( p_poolMGR->get( err, "ZNULLS", SHARED ) == "YES" ) ;
	if ( err.error() ) { return ; }
	char pad    = p_poolMGR->get( err, "ZPADC", PROFILE ).front() ;
	if ( err.error() ) { return ; }

	row  -= win_row ;
	col  -= win_col ;
	p_row = row ;
	p_col = col ;

	prot = true ;
	for ( auto it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( it->second->cursor_on_field( p_row, p_col ) )
		{
			if ( !it->second->field_active ) { return ; }
			if (  it->second->field_numeric && ch != ' ' && !isdigit( ch ) ) { return ; }
			if ( !it->second->field_dynArea && !it->second->field_input ) { return ; }
			if (  it->second->field_dynArea && !it->second->field_dyna_input( col ) ) { return ; }
			if ( Isrt )
			{
				if ( !it->second->edit_field_insert( win, ch, col, pad, snulls ) ) { return ; }
			}
			else
			{
				if ( !it->second->edit_field_replace( win, ch, col, pad, snulls ) ) { return ; }
			}
			prot = false ;
			++p_col ;
			if ( p_col == it->second->field_cole && it->second->field_skip )
			{
				p_row += win_row ;
				p_col += win_col ;
				field_tab_next( p_row, p_col ) ;
				p_row -= win_row ;
				p_col -= win_col ;
			}
			return ;
		}
	}
}


void pPanel::field_backspace( errblock& err, uint& row, uint& col, bool& prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field
	// Return physical position of the cursor in row/col

	uint trow ;
	uint tcol ;
	uint p    ;

	bool snulls = ( p_poolMGR->get( err, "ZNULLS", SHARED ) == "YES" ) ;
	if ( err.error() ) { return ; }
	char pad    = p_poolMGR->get( err, "ZPADC", PROFILE ).front() ;
	if ( err.error() ) { return ; }

	map<string, field *>::iterator it;

	trow = row - win_row ;
	tcol = col - win_col ;

	prot = true ;
	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( it->second->cursor_on_field( trow, tcol ) )
		{
			if ( tcol == it->second->field_col ) { return ; }
			if ( !it->second->field_active )     { return ; }
			if ( !it->second->field_dynArea && !it->second->field_input ) { return ; }
			if (  it->second->field_dynArea && !it->second->field_dyna_input( tcol ) ) { return ; }
			p    = tcol  ;
			tcol = it->second->edit_field_backspace( win, tcol, pad, snulls ) ;
			if ( p == tcol ) { return ; }
			prot = false ;
			row  = trow + win_row ;
			col  = tcol + win_col ;
			return ;
		}
	}
}


void pPanel::field_delete_char( errblock& err, uint row, uint col, bool& prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field
	// Return physical position of the cursor in row/col

	uint trow ;
	uint tcol ;

	map<string, field *>::iterator it ;

	bool snulls = ( p_poolMGR->get( err, "ZNULLS", SHARED ) == "YES" ) ;
	if ( err.error() ) { return ; }
	char pad    = p_poolMGR->get( err, "ZPADC", PROFILE ).front() ;
	if ( err.error() ) { return ; }

	trow = row - win_row ;
	tcol = col - win_col ;

	prot = true ;
	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( it->second->cursor_on_field( trow, tcol ) )
		{
			if ( !it->second->field_active ) { return ; }
			if ( !it->second->field_dynArea && !it->second->field_input ) { return ; }
			if (  it->second->field_dynArea && !it->second->field_dyna_input( tcol ) ) { return ; }
			it->second->edit_field_delete( win, tcol, pad, snulls ) ;
			prot = false ;
			return ;
		}
	}
}


void pPanel::field_erase_eof( errblock& err, uint row, uint col, bool& prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field

	map<string, field *>::iterator it;

	bool snulls = ( p_poolMGR->get( err, "ZNULLS", SHARED ) == "YES" ) ;
	if ( err.error() ) { return ; }
	char pad    = p_poolMGR->get( err, "ZPADC", PROFILE ).front() ;
	if ( err.error() ) { return ; }

	row -= win_row ;
	col -= win_col ;

	prot = true ;
	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( it->second->cursor_on_field( row, col ) )
		{
			if ( !it->second->field_active ) { return ; }
			if ( !it->second->field_dynArea && !it->second->field_input ) { return ; }
			if (  it->second->field_dynArea && !it->second->field_dyna_input( col ) ) { return ; }
			it->second->field_erase_eof( win, col, pad, snulls ) ;
			prot = false ;
			return ;
		}
	}
}


void pPanel::cursor_eof( uint& row, uint& col )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field
	// Return physical position of the cursor in row/col

	uint trow ;
	uint tcol ;

	map<string, field *>::iterator it;

	trow = row - win_row ;
	tcol = col - win_col ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( it->second->cursor_on_field( trow, tcol ) )
		{
			if ( !it->second->field_active ) { return ; }
			if ( !it->second->field_dynArea && !it->second->field_input ) { return ; }
			if (  it->second->field_dynArea && !it->second->field_dyna_input( tcol ) ) { return ; }
			col = it->second->end_of_field( win, tcol ) + win_col ;
			return ;
		}
	}
}


void pPanel::field_tab_down( uint& row, uint& col )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field
	// Return physical position of the cursor in row/col
	// (get_home returns physical position on the screen)

	int t_offset ;
	int m_offset ;
	int c_offset ;
	int d_offset ;
	int o_row    ;

	uint trow ;
	uint tcol ;

	trow = row - win_row ;
	tcol = col - win_col ;

	bool cursor_moved(false) ;
	map<string, field *>::iterator it;

	c_offset = trow * WSCRMAXW + tcol ;
	m_offset = WSCRMAXD * WSCRMAXW    ;
	o_row    = trow                   ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( !it->second->field_active) { continue ; }
		if ( !it->second->field_dynArea && !it->second->field_input ) { continue ; }
		if (  it->second->field_row <= o_row ) { continue ; }
		d_offset = 0 ;
		if ( it->second->field_dynArea && it->second->field_dynArea->dynArea_DataInsp )
		{
			d_offset = it->second->field_dyna_input_offset( 0 ) ;
			if ( d_offset == -1 ) { continue ; }
		}
		t_offset = it->second->field_row * WSCRMAXW + it->second->field_col + d_offset ;
		if ( (t_offset > c_offset) && (t_offset < m_offset) )
		{
			m_offset     = t_offset ;
			row          = it->second->field_row + win_row ;
			col          = it->second->field_col + d_offset + win_col ;
			cursor_moved = true ;
		}
	}
	if ( !cursor_moved ) { get_home( row, col ) ; }
}



void pPanel::field_tab_next( uint& row, uint& col )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field
	// Return physical position of the cursor in row/col
	// (get_home returns physical position on the screen)

	uint t_offset ;
	uint m_offset ;
	uint c_offset ;
	uint d_offset ;
	uint o_row ;
	uint o_col ;
	uint trow  ;
	uint tcol  ;

	trow = row - win_row ;
	tcol = col - win_col ;

	bool cursor_moved(false) ;
	map<string, field *>::iterator it;

	c_offset = trow * WSCRMAXW + tcol ;
	m_offset = WSCRMAXD * WSCRMAXW    ;
	o_row    = trow                   ;
	o_col    = tcol                   ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( !it->second->field_active) { continue ; }
		if ( !it->second->field_dynArea && !it->second->field_input ) { continue ; }
		d_offset = 0 ;
		if ( it->second->field_dynArea && it->second->field_dynArea->dynArea_DataInsp )
		{
			if ( o_row == it->second->field_row ) { d_offset = it->second->field_dyna_input_offset( o_col ) ; }
			else                                  { d_offset = it->second->field_dyna_input_offset( 0 )     ; }
			if ( d_offset == -1 ) { continue ; }
		}
		t_offset = it->second->field_row * WSCRMAXW + it->second->field_col + d_offset ;
		if ( (t_offset > c_offset) && (t_offset < m_offset) )
		{
			m_offset     = t_offset ;
			row          = it->second->field_row + win_row ;
			col          = it->second->field_col + d_offset + win_col ;
			cursor_moved = true ;
		}
	}
	if ( !cursor_moved ) { get_home( row, col ) ; }
}


string pPanel::get_field_help( uint row, uint col )
{
	string fld = field_getname( row, col ) ;

	if ( fieldHList.count( fld ) == 0 ) { return "" ; }
	return fieldHList[ fld ] ;
}


void pPanel::tb_set_linesChanged( errblock& err, string& asURID )
{
	//  Store changed lines for processing by the application if requested via tbdispl with no panel name
	//  Format is a list of line-number/URID pairs

	//  If AUTOSEL(YES)/CSRROW specified and the line is on the screen, add row even if it has not changed

	string URID  ;

	map<string, field *>::iterator it ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( !it->second->field_tb ) { continue ; }
		if ( it->second->field_changed )
		{
			URID = p_funcPOOL->get( err, 0, ".ZURID."+ d2ds( it->second->field_row - tb_row ), NOCHECK ) ;
			if ( err.error() ) { return ; }
			tb_linesChanged[ it->second->field_row - tb_row ] = URID ;
		}
		else if ( asURID != "" )
		{
			URID = p_funcPOOL->get( err, 0, ".ZURID."+ d2ds( it->second->field_row - tb_row ), NOCHECK ) ;
			if ( err.error() ) { return ; }
			if ( URID == asURID )
			{
				tb_linesChanged[ it->second->field_row - tb_row ] = URID ;
				asURID = "" ;
			}
		}
	}
	p_funcPOOL->put( err, "ZTDSELS", tb_linesChanged.size() ) ;
}


bool pPanel::tb_get_lineChanged( errblock& err, int& ln, string& URID )
{
	//  Retrieve the next changed line on the tbdispl.  Return screen line number and URID of the table record
	//  Don't remove the pair from the list but update ZTDSELS

	map<int, string>::iterator it ;

	ln   = 0  ;
	URID = "" ;

	if ( tb_linesChanged.size() == 0 ) { return false ; }

	it   = tb_linesChanged.begin() ;
	ln   = it->first  ;
	URID = it->second ;

	p_funcPOOL->put( err, "ZTDSELS", tb_linesChanged.size() ) ;
	if ( err.error() ) { return  false ; }
	return true ;
}


void pPanel::tb_clear_linesChanged( errblock& err )
{
	//  Clear all stored changed lines on a tbdispl with panel name and set ZTDSELS to zero

	tb_linesChanged.clear() ;
	p_funcPOOL->put( err, "ZTDSELS", 0 ) ;
}


void pPanel::tb_remove_lineChanged()
{
	//  Remove the processed line from the list of changed lines

	tb_linesChanged.erase( tb_linesChanged.begin() ) ;
}


void pPanel::display_tb_mark_posn( errblock& err )
{
	int rows  ;
	int top   ;
	int size  ;

	string mark ;
	string posn ;

	rows = p_funcPOOL->get( err, 0, INTEGER, "ZTDROWS" ) ;
	if ( err.error() ) { return ; }
	top  = p_funcPOOL->get( err, 0, INTEGER, "ZTDTOP"  ) ;
	if ( err.error() ) { return ; }
	size = rows - top + 1 ;
	p_funcPOOL->put( err, "ZTDVROWS", size ) ;
	if ( err.error() ) { return ; }

	wattrset( win, WHITE ) ;
	if ( size < tb_depth )
	{
		mark = p_funcPOOL->get( err, 8, "ZTDMARK" ) ;
		if ( err.error() ) { return ; }
		if ( err.RC8() )
		{
			mark = p_poolMGR->get( err, "ZTDMARK" ) ;
			if ( err.error() ) { return ; }
			if ( err.RC8() )
			{
				mark = centre( " Bottom of Data ", WSCRMAXW, '*' ) ;
			}
		}
		if ( mark.size() > WSCRMAXW )
		{
			mark.resize( WSCRMAXW ) ;
		}
		mvwaddstr( win, tb_row + size, 0, mark.c_str() ) ;
		p_funcPOOL->put( err, "ZTDVROWS", size ) ;
		if ( err.error() ) { return ; }
	}
	else
	{
		p_funcPOOL->put( err, "ZTDVROWS", tb_depth ) ;
		if ( err.error() ) { return ; }
	}

	posn = "" ;
	if ( top <= rows )
	{
		posn = "Row "+ d2ds( top ) +" of "+ d2ds( rows ) ;
	}
	mvwaddstr( win, 2, WSCRMAXW - posn.length(), posn.c_str() ) ;
	wattroff( win, WHITE ) ;
}


void pPanel::set_tb_fields_act_inact( errblock& err )
{
	int rows ;
	int top  ;
	int size ;
	int ws   ;
	int i    ;
	int j    ;

	string s ;
	string suf ;

	rows = p_funcPOOL->get( err, 0, INTEGER, "ZTDROWS" ) ;
	if ( err.error() ) { return ; }
	top  = p_funcPOOL->get( err, 0, INTEGER, "ZTDTOP"  ) ;
	if ( err.error() ) { return ; }

	size = rows - top + 1 ;

	for ( ws = words( tb_fields ), i = 0 ; i < tb_depth ; i++ )
	{
		suf = "." + d2ds( i ) ;
		for ( j = 1 ; j <= ws ; j++ )
		{
			s = word( tb_fields, j ) + suf ;
			fieldList[ s ]->field_active = ( i < size ) ;
		}
	}
}


void pPanel::get_pd_home( uint& row, uint& col )
{
	// Return the physical home position of a pull-down

	row = win_row + 2 ;
	col = win_col + 2 + ab[ abIndex ].get_pd_col() ;
}


bool pPanel::cursor_on_pulldown( uint row, uint col )
{
	// row/col is the pysical position on the screen.

	return ab[ abIndex ].cursor_on_pulldown( row, col ) ;
}


bool pPanel::display_pd( errblock& err, uint row, uint col, string& msg )
{
	// row/col is the pysical position on the screen.  Correct by subtracting the window column position

	// Call relevant )ABCINIT and display the pull-down at the cursor position.

	row -= win_row ;
	col -= win_col ;
	hide_pd() ;

	err.setRC( 0 ) ;

	for ( int i = 0 ; i < ab.size() ; i++ )
	{
		if ( col >= ab[ i ].abc_col && col < ( ab[ i ].abc_col + ab[ i ].abc_desc.size() ) )
		{
			abIndex = i ;
			auto it = ab.begin() ;
			advance( it, abIndex ) ;
			clear_msg() ;
			abc_panel_init( err, it->get_abc_desc() ) ;
			if ( err.error() )
			{
				pdActive = false ;
				return false ;
			}
			msg = msgid ;
			it->display_abc_sel( win ) ;
			it->display_pd( err, win_row, win_col, row ) ;
			pdActive = true ;
			p_col    = it->abc_col + 2 ;
			p_row    = 2 ;
			return true  ;
		}
	}
	return false ;
}


void pPanel::display_pd( errblock& err )
{
	if ( !pdActive ) { return ; }

	ab.at( abIndex ).display_abc_sel( win ) ;
	ab.at( abIndex ).display_pd( err, win_row, win_col, 0 ) ;
}


void pPanel::display_current_pd( errblock& err, uint row, uint col )
{
	// row/col is the pysical position on the screen.  Correct by subtracting the window column position

	// Re-display the pull-down if the cursor is placed on it (to update current choice and hilite)

	auto it = ab.begin() ;
	advance( it, abIndex ) ;

	row -= win_row ;
	col -= win_col ;

	if ( col >= it->abc_col && col < ( it->abc_col + it->abc_maxw + 10 ) )
	{
		it->display_pd( err, win_row, win_col, row ) ;
	}
}


void pPanel::display_next_pd( errblock& err, string& msg )
{
	err.setRC( 0 ) ;

	if ( ab.size() == 0 ) { return ; }

	hide_pd() ;
	if ( !pdActive || ++abIndex == ab.size() ) { abIndex = 0 ; }

	auto it = ab.begin() ;
	advance( it, abIndex ) ;

	clear_msg() ;

	pdActive = true ;

	abc_panel_init( err, it->get_abc_desc() ) ;
	if ( err.error() )
	{
		pdActive = false ;
		return ;
	}
	msg = msgid ;
	it->display_abc_sel( win ) ;
	it->display_pd( err, win_row, win_col, 2 ) ;
	p_col = it->abc_col + 2 ;
	p_row = 2 ;
}


void pPanel::remove_pd()
{
	hide_pd() ;
	pdActive = false ;
}


void pPanel::hide_pd()
{
	if ( !pdActive ) { return ; }

	ab.at( abIndex ).hide_pd() ;
	ab.at( abIndex ).display_abc_unsel( win ) ;
}


pdc pPanel::retrieve_choice( errblock& err, string& msg )
{
	pdc t_pdc ;

	auto it = ab.begin() ;
	advance( it, abIndex ) ;

	t_pdc = it->retrieve_choice( err ) ;

	if ( !t_pdc.pdc_inact )
	{
		abc_panel_proc( err, it->get_abc_desc() ) ;
		msg = msgid ;
	}

	return t_pdc ;
}


void pPanel::display_boxes()
{
	for ( auto it = boxes.begin() ; it != boxes.end() ; it++ )
	{
		(*it)->display_box( win, sub_vars( (*it)->box_title ) ) ;
	}
}


void pPanel::set_panel_msg( const slmsg& t, const string& m )
{
	errblock err ;

	clear_msg() ;
	MSG   = t   ;
	msgid = m   ;

	if ( MSG.smsg == "" && MSG.lmsg == "" )
	{
		msgid = "" ;
		return     ;
	}
	if ( p_poolMGR->get( err, ZSCRNUM, "ZSHMSGID" ) == "Y" ) { MSG.lmsg = msgid +" "+ MSG.lmsg ; }
	if ( err.error() ) { return ; }
	showLMSG = ( MSG.smsg == "" || MSG.lmsg == "" ) ;
}


void pPanel::clear_msg()
{
	MSG.clear() ;
	msgid    = "" ;
	showLMSG = false ;

	if ( smwin )
	{
		panel_cleanup( smpanel ) ;
		del_panel( smpanel ) ;
		delwin( smwin )      ;
		smwin = NULL         ;
	}

	if ( lmwin )
	{
		panel_cleanup( lmpanel ) ;
		del_panel( lmpanel ) ;
		delwin( lmwin )      ;
		lmwin = NULL         ;
	}
}


void pPanel::put_keylist( int entry, const string& keyv )
{
	Keylistl[ entry ] = keyv ;
}


string pPanel::get_keylist( int entry )
{
	if ( KEYLISTN == "" || Keylistl.count( entry ) == 0 ) { return "" ; }
	return Keylistl[ entry ] ;
}


void pPanel::display_msg( errblock& err )
{
	int i     ;
	int x_row ;
	int w_row ;
	int w_col ;
	int w_depth ;
	int w_width ;

	vector<string> v ;

	if ( msgid == "" ) { return ; }

	msgResp = MSG.resp ;
	if ( msgid != "PSYS012L" )
	{
		p_poolMGR->put( err, ZSCRNUM, "ZMSGID", msgid ) ;
		if ( err.error() ) { return ; }
	}

	if ( MSG.smsg != "" )
	{
		if ( smwin )
		{
			panel_cleanup( smpanel ) ;
			del_panel( smpanel ) ;
			delwin( smwin )      ;
		}
		if ( MSG.smwin || pd_active() )
		{
			get_msgwin( MSG.smsg, w_row, w_col, w_depth, w_width, v ) ;
			smwin = newwin( w_depth, w_width, w_row+win_row, w_col+win_col ) ;
			wattrset( smwin, cuaAttr[ MSG.type ] ) ;
			box( smwin, 0, 0 ) ;
			for ( i = 0 ; i < v.size() ; i++ )
			{
				mvwaddstr( smwin, i+1, 2, v[i].c_str() ) ;
			}
		}
		else
		{
			x_row = ab.size() > 0 ? 2 : 0 ;
			mvwaddstr( win, x_row, (WSCRMAXW - 25), "                         " ) ;
			smwin = newwin( 1, MSG.smsg.size(), x_row+win_row, (WSCRMAXW - MSG.smsg.size() + win_col) ) ;
			wattrset( smwin, cuaAttr[ MSG.type ] ) ;
			mvwaddstr( smwin, 0, 0, MSG.smsg.c_str() ) ;
		}
		smpanel = new_panel( smwin )  ;
		set_panel_userptr( smpanel, new panel_data( ZSCRNUM ) ) ;
		wattroff( smwin, cuaAttr[ MSG.type ] ) ;
		if ( MSG.alm )
		{
			beep() ;
			MSG.alm = false ;
		}
	}
	if ( showLMSG && MSG.lmsg != "" )
	{
		if ( lmwin )
		{
			panel_cleanup( lmpanel ) ;
			del_panel( lmpanel ) ;
			delwin( lmwin )      ;
		}
		if ( MSG.lmwin || p_poolMGR->get( err, "ZLMSGW", PROFILE ) == "Y" || pd_active() )
		{
			get_msgwin( MSG.lmsg, w_row, w_col, w_depth, w_width, v ) ;
			lmwin = newwin( w_depth, w_width, w_row+win_row, w_col+win_col ) ;
			wattrset( lmwin, cuaAttr[ MSG.type ] )  ;
			box( lmwin, 0, 0 ) ;
			for ( i = 0 ; i < v.size() ; i++ )
			{
				mvwaddstr( lmwin, i+1, 2, v[i].c_str() ) ;
			}
		}
		else
		{
			lmwin = newwin( 1, MSG.lmsg.size(), 4+win_row, 1+win_col ) ;
			wattrset( lmwin, cuaAttr[ MSG.type ] )  ;
			mvwaddstr( lmwin, 0, 0, MSG.lmsg.c_str() ) ;
		}
		lmpanel = new_panel( lmwin )  ;
		set_panel_userptr( lmpanel, new panel_data( ZSCRNUM ) ) ;
		wattroff( lmwin, cuaAttr[ MSG.type ] )  ;
	}
}


void pPanel::get_msgwin( string m, int& t_row, int& t_col, int& t_depth, int& t_width, vector<string>& v )
{
	// Split message into separate lines if necessary and put into vector v.
	// Calculate the message window position and size.

	int w  ;
	int mw ;
	int h  ;
	int p  ;

	map<string, field *>::iterator it ;

	it = fieldList.find( get_msgloc() ) ;

	h = m.size() / WSCRMAXW + 1 ;
	w = m.size() / h            ;
	if ( it != fieldList.end() && it->second->field_col > WSCRMAXW/2 )
	{
		w = w / 3 ;
	}
	if ( w > WSCRMAXW - 6 ) { w = WSCRMAXW - 6 ; }

	v.clear() ;
	mw = 0    ;

	while ( m != "" )
	{
		if ( m.size() <= w )
		{
			if ( m.size() > mw ) { mw = m.size() ; }
			v.push_back( m ) ;
			break ;
		}
		p = m.find_last_of( ' ', w ) ;
		if ( p == string::npos ) { p = w ; }
		v.push_back( m.substr( 0, p ) ) ;
		trim_right( v.back() ) ;
		if ( v.back().size() > mw ) { mw = v.back().size() ; }
		m.erase( 0, p ) ;
		trim_left( m )  ;
	}

	t_depth = v.size() + 2 ;
	t_width = mw + 4       ;

	if ( pd_active() )
	{
		ab.at( abIndex ).get_msg_position( t_row, t_col ) ;
		t_row += 1 ;
	}
	else if ( it != fieldList.end() )
	{
		t_row = it->second->field_row + 1 ;
		t_col = it->second->field_col - 2 ;
		if ( t_col < 0 ) { t_col = 0 ; }
		if ( (t_row + t_depth) > WSCRMAXD )
		{
			t_row = WSCRMAXD - t_depth ;
			t_col = t_col + it->second->field_length + 2 ;
		}
		if ( (t_col + t_width) > WSCRMAXW )
		{
			t_col = WSCRMAXW - t_width ;
		}
	}
	else
	{
		t_row =  WSCRMAXD - t_depth    ;
		t_col = (WSCRMAXW - t_width)/2 ;
	}
}


void pPanel::display_id( errblock& err )
{
	string scrname ;
	string panarea ;

	panarea = "" ;

	if ( idwin )
	{
		panel_cleanup( idpanel ) ;
		del_panel( idpanel ) ;
		delwin( idwin )      ;
		idwin = NULL         ;
	}

	if ( p_poolMGR->get( err, ZSCRNUM, "ZSHUSRID" ) == "Y" )
	{
		panarea = p_poolMGR->get( err, "ZUSER", SHARED ) + " " ;
	}

	if ( p_poolMGR->get( err, ZSCRNUM, "ZSHPANID" ) == "Y" )
	{
		panarea += panelid + " " ;
	}

	if ( p_poolMGR->get( err, "ZSCRNAM1", SHARED ) == "ON" )
	{
		scrname = p_poolMGR->get( err, "ZSCRNAME", SHARED ) ;
		if ( scrname != "" )
		{
			panarea += scrname + " " ;
		}
	}

	if ( panarea != "" )
	{
		idwin   = newwin( 1, panarea.size(), ab.size() > 0 ? win_row+2 : win_row, win_col+1 ) ;
		idpanel = new_panel( idwin )  ;
		set_panel_userptr( idpanel, new panel_data( ZSCRNUM ) ) ;
		wattrset( idwin, cuaAttr[ PI ] ) ;
		mvwaddstr( idwin, 0, 0, panarea.c_str() ) ;
		wattroff( idwin, cuaAttr[ PI ] ) ;
		top_panel( idpanel ) ;
	}
}


void pPanel::get_panel_info( int& RC1, const string& a_name, const string& t_name, const string& w_name, const string& d_name, const string& r_name, const string& c_name )
{
	map<string, dynArea *>::iterator it ;

	errblock err ;

	RC1 = 0 ;

	it = dynAreaList.find( a_name ) ;
	if ( it == dynAreaList.end() )
	{
		llog( "E", "PQUERY.  Dynamic area '"+ a_name +"' not found" << endl ) ;
		RC1 = 8 ;
		return  ;
	}

	if ( t_name != "" ) { p_funcPOOL->put( err, t_name, "DYNAMIC" ) ; }
	if ( w_name != "" ) { p_funcPOOL->put( err, w_name, it->second->dynArea_width ) ; }
	if ( d_name != "" ) { p_funcPOOL->put( err, d_name, it->second->dynArea_depth ) ; }
	if ( r_name != "" ) { p_funcPOOL->put( err, r_name, it->second->dynArea_row )   ; }
	if ( c_name != "" ) { p_funcPOOL->put( err, c_name, it->second->dynArea_col )   ; }
}


void pPanel::attr( int& RC1, const string& field, const string& attrs )
{
	RC1 = 0 ;

	errblock err ;

	if ( fieldList.count( field ) == 0 )
	{
		llog( "E", "ATTR.  Field '"+ field +"' not found" << endl ) ;
		RC1 = 8 ;
	}
	else
	{
		fieldList[ field ]->field_attr( err, attrs ) ;
		if ( err.error() )
		{
			RC1 = 20 ;
		}
	}
}


void pPanel::panel_cleanup( PANEL * p )
{
	const void* vptr = panel_userptr( p ) ;

	if ( vptr )
	{
		delete static_cast<const panel_data *>(vptr) ;
	}
}


bool pPanel::on_border_line( uint r, uint c )
{
	// Return true if the cursor is on the window border (to start a window move)

	if ( win != pwin ) { return false ; }
	return ( ((r == win_row-1 || r == win_row+WSCRMAXD) &&
		  (c >= win_col-1 && c <= win_col+WSCRMAXW))  ||
		 ((c == win_col-1 || c == win_col+WSCRMAXW) &&
		  (r >= win_row-1 && r <= win_row+WSCRMAXD)) ) ;
}


void pPanel::update_keylist_vars( errblock& err )
{
	p_poolMGR->put( err, "ZKLNAME", KEYLISTN, SHARED, SYSTEM ) ;
	p_poolMGR->put( err, "ZKLAPPL", KEYAPPL,  SHARED, SYSTEM ) ;
	p_poolMGR->put( err, "ZKLTYPE", "P",      SHARED, SYSTEM ) ;
}


bool pPanel::hide_msg_window( uint r, uint c )
{
	// If the cursor is on a message window, hide the window and return true
	// The underlying panel is not submitted for processing in this case.

	int ht  ;
	int len ;
	int w_row ;
	int w_col ;

	if ( smwin && !panel_hidden( smpanel) )
	{
		getmaxyx( smwin, len, ht ) ;
		if ( ht > 1 )
		{
			getbegyx( smwin, w_row, w_col ) ;
			if ( ( r >= w_row && r < (w_row + len) )  &&
			     ( c >= w_col && c < (w_col + ht ) ) )
			{
				hide_panel( smpanel ) ;
				return true ;
			}
		}
	}
	if ( lmwin && !panel_hidden( lmpanel) )
	{
		getmaxyx( lmwin, len, ht ) ;
		if ( ht > 1 )
		{
			getbegyx( lmwin, w_row, w_col ) ;
			if ( ( r >= w_row && r < (w_row + len) )  &&
			     ( c >= w_col && c < (w_col + ht ) ) )
			{
				hide_panel( lmpanel ) ;
				return true ;
			}
		}
	}
	return false ;
}


string pPanel::sub_vars( string s )
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

	errblock err ;

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
			val = getDialogueVar( err, var ) ;
			if ( !err.error() )
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
	return s ;
}


string pPanel::sub_vars( string s, bool& dvars )
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

	errblock err ;

	const string validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#$@" ;
	p1 = 0 ;

	dvars = false ;
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
			val = getDialogueVar( err, var ) ;
			if ( !err.error() )
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
				dvars = true ;
			}
		}
	}
	return s ;
}
