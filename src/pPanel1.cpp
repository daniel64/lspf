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
	bypassCur   = false  ;
	redisplay   = false  ;
	nretfield   = ""     ;
	keylistn    = ""     ;
	keyappl     = ""     ;
	keyhelpn    = ""     ;
	pos_smsg    = ""     ;
	pos_lmsg    = ""     ;
	panelTitle  = ""     ;
	panelDesc   = ""     ;
	Area1       = ""     ;
	abIndex     = 0      ;
	da_dataIn   = ""     ;
	da_dataOut  = ""     ;
	tb_model    = false  ;
	tb_depth    = 0      ;
	tb_curidr   = -1     ;
	tb_curidx   = -1     ;
	tb_csrrow   = 0      ;
	tb_crp      = -1     ;
	tb_scan     = false  ;
	tb_autosel  = false  ;
	tb_lcol     = 0      ;
	tb_lsz      = 0      ;
	full_screen = false  ;
	win_addpop  = false  ;
	error_msg   = false  ;
	win_row     = 0      ;
	win_col     = 0      ;
	win_width   = 0      ;
	win_depth   = 0      ;
	dyn_depth   = 0      ;
	cmdfield    = ""     ;
	home        = ""     ;
	dTrail      = ""     ;
	scroll      = "ZSCROLL" ;
	fieldMap    = NULL   ;
	fieldAddrs  = NULL   ;
	fwin        = NULL   ;
	pwin        = NULL   ;
	bwin        = NULL   ;
	idwin       = NULL   ;
	smwin       = NULL   ;
	lmwin       = NULL   ;
	control_vars = {
	{ ".ALARM",   CV_ALARM   },
	{ ".AUTOSEL", CV_AUTOSEL },
	{ ".BROWSE",  CV_BROWSE  },
	{ ".CSRPOS",  CV_CSRPOS  },
	{ ".CSRROW",  CV_CSRROW  },
	{ ".CURSOR",  CV_CURSOR  },
	{ ".EDIT",    CV_EDIT    },
	{ ".FALSE",   CV_FALSE   },
	{ ".HELP",    CV_HELP    },
	{ ".MSG",     CV_MSG     },
	{ ".NRET",    CV_NRET    },
	{ ".PFKEY",   CV_PFKEY   },
	{ ".RESP",    CV_RESP    },
	{ ".TRUE",    CV_TRUE    },
	{ ".TRAIL",   CV_TRAIL   },
	{ ".ZVARS",   CV_ZVARS   } } ;
}


pPanel::~pPanel()
{
	// Iterate over the 5 panel widget types, text, field, Area, dynArea, boxes action-bar-choices and delete.
	// Delete panel language statements in )INIT, )REINIT, )PROC, )ABCINIT and )ABCPROC sections.
	// Delete the main window/panel, popup panel and any message windows/panels created (free any userdata first)

	for ( auto it = fieldList.begin() ; it != fieldList.end() ; ++it )
	{
		delete it->second ;
	}

	for ( auto it = AreaList.begin() ; it != AreaList.end() ; ++it )
	{
		delete it->second ;
	}

	for ( auto it = dynAreaList.begin() ; it != dynAreaList.end() ; ++it )
	{
		delete it->second ;
	}

	for_each( textList.begin(), textList.end(),
		[](text* a)
		{
			delete a ;
		} ) ;

	for_each( boxes.begin(), boxes.end(),
		[](Box* a)
		{
			delete a ;
		} ) ;
	for_each( ab.begin(), ab.end(),
		[](abc* a)
		{
			delete a ;
		} ) ;
	for_each( initstmnts.begin(), initstmnts.end(),
		[](panstmnt* a)
		{
			delete a ;
		} ) ;
	for_each( reinstmnts.begin(), reinstmnts.end(),
		[](panstmnt* a)
		{
			delete a ;
		} ) ;
	for_each( procstmnts.begin(), procstmnts.end(),
		[](panstmnt* a)
		{
			delete a ;
		} ) ;
	for ( auto it = abc_initstmnts.begin() ; it != abc_initstmnts.end() ; ++it )
	{
		for_each( it->second.begin(), it->second.end(),
			[](panstmnt* a)
			{
				delete a ;
			} ) ;
	}
	for ( auto it = abc_procstmnts.begin() ; it != abc_procstmnts.end() ; ++it )
	{
		for_each( it->second.begin(), it->second.end(),
			[](panstmnt* a)
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

	delete[] fieldMap ;
	delete[] fieldAddrs ;

	update_panels() ;
}


void pPanel::init( errblock& err )
{
	err.setRC( 0 ) ;

	zscrmaxd = ds2d( p_poolMGR->get( err, "ZSCRMAXD", SHARED ) ) ;
	if ( err.getRC() > 0 )
	{
		err.seterrid( "PSYE015L", "GET", "ZSCRMAXD" ) ;
		return ;
	}

	zscrmaxw = ds2d( p_poolMGR->get( err, "ZSCRMAXW", SHARED ) ) ;
	if ( err.getRC() > 0 )
	{
		err.seterrid( "PSYE015L", "GET", "ZSCRMAXW" ) ;
		return ;
	}

	zscrnum = ds2d(p_poolMGR->get( err, "ZSCRNUM", SHARED ) ) ;
	if ( err.getRC() > 0 )
	{
		err.seterrid( "PSYE015L", "GET", "ZSCRNUM" ) ;
		return ;
	}

	wscrmaxd = zscrmaxd ;
	wscrmaxw = zscrmaxw ;
	taskId   = err.taskid ;
}


void pPanel::init_control_variables()
{
	// Initialise panel control variables (and related) when the display service first receives control

	dAlarm  = "NO" ;
	dCursor = "" ;
	dCsrpos = 0  ;
	dHelp   = "" ;
	dMsg    = "" ;
	dResp   = "" ;

	iCursor = "" ;
	iCsrpos = 1  ;
	iMsg    = "" ;

	msgloc  = "" ;
	end_pressed = false ;
	bypassCur   = false ;
}


void pPanel::reset_control_variables()
{
	// Reset panel control variables (and related) before )PROC processing

	dCursor = "" ;
	dCsrpos = 0  ;
	dMsg    = "" ;
	dResp   = "" ;

	iCursor = "" ;
	iCsrpos = 1  ;
	iMsg    = "" ;

	msgloc  = "" ;
}


string pPanel::getDialogueVar( errblock& err,
			       const string& var )
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

	string* p_str     ;
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
				 p_funcPOOL->put1( err, var, "" ) ;
		}
	}
	return "" ;
}


void pPanel::putDialogueVar( errblock& err,
			     const string& var,
			     const string& val )
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
		p_funcPOOL->put1( err, var, val ) ;
	}
}


void pPanel::syncDialogueVar( errblock& err,
			      const string& var )
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

	p_funcPOOL->put1( err, var, p_poolMGR->get( err, var, ASIS ) ) ;
}


void pPanel::set_popup( bool addpop_active, int& addpop_row, int& addpop_col )
{
	if ( addpop_active )
	{
		win_addpop = true ;
		win_row    = (addpop_row + win_depth + 1) < zscrmaxd ? addpop_row : (zscrmaxd - win_depth - 1) ;
		win_col    = (addpop_col + win_width + 1) < zscrmaxw ? addpop_col : (zscrmaxw - win_width - 1) ;
		addpop_row = win_row ;
		addpop_col = win_col ;
	}
	else
	{
		remove_popup() ;
	}
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
	if ( !win || win != pwin ) { return ; }

	top_panel( bpanel ) ;
	top_panel( panel )  ;
}


void pPanel::hide_popup()
{
	if ( !win || win != pwin ) { return ; }

	hide_panel( bpanel ) ;
	hide_panel( panel )  ;
}


void pPanel::create_panels( popup& p )
{
	// If we are being displayed in a popup, create another set of
	// ncurses windows/panels on a subsequent ADDPOP call and store
	// original windows/panels in the popup structure.

	if ( !win || win != pwin ) { return ; }

	p.pan1 = panel ;
	p.pan2 = bpanel ;
	p.panl = this ;

	pwin   = newwin( win_depth, win_width, 0, 0 ) ;
	panel  = new_panel( pwin ) ;
	set_panel_userptr( panel, new panel_data( zscrnum, this ) ) ;

	bwin   = newwin( win_depth+2, win_width+2, 0, 0 ) ;
	bpanel = new_panel( bwin ) ;
	set_panel_userptr( bpanel, new panel_data( zscrnum, true, this ) ) ;

	win = pwin ;
}


void pPanel::delete_panels( const popup& p )
{
	// Delete the set of ncurses windows/panels in the popup structure on a REMPOP call

	WINDOW* w ;

	w = panel_window( p.pan1 ) ;
	panel_cleanup( p.pan1 ) ;
	del_panel( p.pan1 ) ;
	delwin( w ) ;

	w = panel_window( p.pan2 ) ;
	panel_cleanup( p.pan2 ) ;
	del_panel( p.pan2 ) ;
	delwin( w ) ;
}


void pPanel::decolourise( WINDOW* w, uint colour1, uint colour2, uint intens )
{
	// Remove the colour from the panel window w, using mvwchgat().

	// Redraw the action bar divider and boxes as ncurses line drawing is an attribute and
	// removing the attributes results in the line being changed to the mapping character
	// (eg. 'q' for ACS_HLINE).

	for ( uint i = 0 ; i < wscrmaxd ; ++i )
	{
		mvwchgat( w, i, 0, -1, intens | panel_intens, colour1, NULL ) ;
	}

	if ( ab.size() > 0 )
	{
		wattrset( w, colour2 | intens | panel_intens ) ;
		mvwhline( w, 1, 0, ACS_HLINE, wscrmaxw ) ;
	}

	for ( auto it = boxes.begin() ; it != boxes.end() ; ++it )
	{
		(*it)->display_box( win, sub_vars( (*it)->box_title ), colour2, intens ) ;
	}
}


void pPanel::redraw_panel( errblock& err )
{
	// Redraw the entire panel if it has been decolourised, or just the panel frame
	// if decolourisation has been set off.

	if ( is_panel_decolourised() )
	{
		display_panel( err ) ;
		if ( bwin )
		{
			set_panel_frame_act() ;
		}
	}
	else if ( bwin && is_panel_frame_inact() )
	{
		wattrset( bwin, cuaAttr[ AWF ] | panel_intens ) ;
		draw_frame( err ) ;
		set_panel_frame_act() ;
	}
}


void pPanel::toggle_fscreen( bool sp_popup,
			     int sp_row,
			     int sp_col )
{
	full_screen = !full_screen ;

	if ( !full_screen && sp_popup )
	{
		set_popup( sp_popup, sp_row, sp_col ) ;
	}
	clear_msg() ;
}


void pPanel::display_panel( errblock& err )
{
	// fwin - created unconditionally and is the full-screen window
	// pwin - pop-up window only created if the panel contains a WINDOW(w,d) statement
	// (associate ncurses panel 'panel' with whichever is active and set WSCRMAX?)

	// Use fwin if no pwin exists, or no ADDPOP() has been done and remove the popup
	// if no pwin exists, even if an ADDPOP() has been done (not exactly how real ISPF works).
	// The RESIZE lspf command can also toggle full screen mode.

	string panttl ;

	err.setRC( 0 ) ;

	update_keylist_vars( err ) ;
	if ( err.error() ) { return ; }

	if ( win_addpop && pwin && !full_screen )
	{
		win = pwin ;
		if ( win != panel_window( panel ) )
		{
			replace_panel( panel, win ) ;
		}
		mvwin( win, win_row, win_col )      ;
		mvwin( bwin, win_row-1, win_col-1 ) ;
		wattrset( bwin, cuaAttr[ AWF ] | panel_intens ) ;
		draw_frame( err ) ;
		if ( err.error() ) { return ; }
		top_panel( bpanel ) ;
		top_panel( panel ) ;
		wscrmaxw = win_width ;
		wscrmaxd = win_depth ;
		unset_panel_fscreen() ;
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
		wscrmaxw = zscrmaxw ;
		wscrmaxd = zscrmaxd ;
		set_panel_fscreen() ;
	}

	werase( win ) ;
	unset_panel_decolourised() ;

	panttl = sub_vars( panelTitle ) ;
	wattrset( win, cuaAttr[ PT ] | panel_intens ) ;
	mvwaddstr( win, ( ab.size() > 0 ) ? 2 : 0, ( wscrmaxw - panttl.size() ) / 2, panttl.c_str() ) ;

	if ( tb_model )
	{
		display_tb_mark_posn( err ) ;
		if ( err.error() ) { return ; }
		set_tb_fields_act_inact( err ) ;
		if ( err.error() ) { return ; }
	}

	display_ab() ;
	display_text() ;

	display_fields( err, true ) ;
	if ( err.error() ) { return ; }

	display_boxes() ;

	hide_pd() ;

	display_area_si() ;

	display_pd( err )  ;
	if ( err.error() ) { return ; }

	display_msg( err ) ;
	if ( err.error() ) { return ; }

	display_id( err )  ;
	if ( err.error() ) { return ; }

	error_msg = ( dMsg != "" && MSG.type != IMT ) ;

	if ( dAlarm == "YES" ) { beep() ; }
}


void pPanel::draw_frame( PANEL* p, uint col )
{
	WINDOW* w = panel_window( p ) ;

	string winttl = get_panel_ttl( p ) ;

	wattrset( w, col | panel_intens ) ;
	box( w, 0, 0 ) ;

	if ( winttl != "" )
	{
		mvwaddstr( w, 0, ( win_width - winttl.size() ) / 2, winttl.c_str() ) ;
	}
}


void pPanel::draw_frame( errblock& err )
{
	string winttl = getDialogueVar( err, "ZWINTTL" ) ;
	if ( err.error() ) { return ; }

	box( bwin, 0, 0 ) ;

	if ( winttl != "" )
	{
		winttl = " "+ winttl +" " ;
		mvwaddstr( bwin, 0, ( win_width - winttl.size() ) / 2, winttl.c_str() ) ;
		set_panel_ttl( winttl ) ;
	}
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
}


bool pPanel::do_redisplay()
{
	return redisplay ;
}


void pPanel::display_panel_update( errblock& err )
{
	// For all changed fields, remove the null character, apply attributes (upper case, left/right justified),
	// and copy back to function pool.  Reset field for display.

	// For dynamic areas, also update the shadow variable to indicate character deletes (0xFE) or nulls
	// converted to spaces (0xFF)

	int fieldNum    ;
	int maxAmount   ;
	int Amnt        ;
	int p           ;
	int offset      ;
	int posn        ;

	string cmdVerb  ;
	string cmd      ;
	string fieldNam ;
	string msgfld   ;

	const string scrollAmounts = "M MAX C CSR D DATA H HALF P PAGE" ;

	string* darea   ;
	string* shadow  ;

	dataType var_type ;

	map<string, field*>::iterator it ;
	map<string, Area*>::iterator ita ;

	err.setRC( 0 ) ;

	fieldNum = -1 ;
	posn     = 1  ;
	MSG.clear()   ;

	reset_attrs_once() ;
	reset_control_variables() ;

	redisplay = false ;
	bypassCur = false ;

	end_pressed = ( usr_action == USR_CANCEL ||
			usr_action == USR_END    ||
			usr_action == USR_EXIT   ||
			usr_action == USR_RETURN ) ;

	cmdVerb = p_poolMGR->get( err, "ZVERB", SHARED ) ;
	if ( err.error() ) { return ; }

	if ( Area1 != "" && ( cmdVerb == "UP" || cmdVerb == "DOWN" ) )
	{
		for ( ita = AreaList.begin() ; ita != AreaList.end() ; ++ita )
		{
			if ( ita->second->cursor_on_area( p_row, p_col ) )
			{
				break ;
			}
		}
		if ( ita == AreaList.end() && not scrollOn ) { ita = AreaList.find( Area1 ) ; }
		if ( ita != AreaList.end() )
		{
			if ( cmdVerb == "UP" )
			{
				p = ita->second->scroll_up( p_row, p_col ) ;
				if ( p == -1 )
				{
					set_message_cond( "PSYS013K" ) ;
				}
				else
				{
					rebuild_after_area_scroll( ita->second ) ;
					p_row += p ;
				}
			}
			else
			{
				p = ita->second->scroll_down( p_row, p_col ) ;
				if ( p == -1 )
				{
					set_message_cond( "PSYS013L" ) ;
				}
				else
				{
					rebuild_after_area_scroll( ita->second ) ;
					p_row -= p ;
				}
			}
			cmd = cmd_getvalue() ;
			if ( findword( upper( cmd ), scrollAmounts ) || ( isnumeric( cmd ) && cmd.size() <= 6 ) )
			{
				cmd_setvalue( "" ) ;
			}
			redisplay = true ;
			bypassCur = true ;
			return ;
		}
	}

	if ( scrollOn )
	{
		it = fieldList.find( scroll ) ;
		if ( trim( it->second->field_value ) == "" )
		{
			it->second->field_value = p_poolMGR->get( err, scroll, PROFILE ) ;
			if ( err.error() ) { return ; }
			if ( it->second->field_value == "" )
			{
				it->second->field_value = getDialogueVar( err, "ZSCROLLD" ) ;
				if ( err.error() ) { return ; }
			}
		}
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
					  dCsrpos = 1 ;
			}
		}
		p_funcPOOL->put2( err, it->first, it->second->field_value ) ;
		if ( err.error() ) { return ; }
	}

	tb_curidx = -1 ;
	tb_curidr = -1 ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; ++it )
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
				offset   = fieldNum*it->second->field_length ;
				it->second->field_update_datamod_usermod( darea, offset ) ;
				darea->replace( offset, it->second->field_length, it->second->field_value ) ;
				shadow = p_funcPOOL->vlocate( err, it->second->field_dynArea->dynArea_shadow_name ) ;
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
						p_funcPOOL->put1( err, it->first, ds2d( it->second->field_value ) ) ;
						if ( err.error() ) { return ; }
					}
					else
					{
						set_message_cond( "PSYS011G" ) ;
						set_cursor_cond( it->first )   ;
						dCsrpos = 1 ;
					}
				}
				else if ( selectPanel )
				{
					p_poolMGR->put( err, it->first, it->second->field_value, ASIS ) ;
				}
				else if ( it->second->field_tb )
				{
					p_funcPOOL->put3( err, it->first, it->second->field_value ) ;
				}
				else
				{
					p_funcPOOL->put2( err, it->first, it->second->field_value ) ;
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
				iCursor  = fieldNam ;
				iCsrpos  = fieldNum*it->second->field_length + posn ;
			}
			else
			{
				if ( it->second->field_tb )
				{
					p        = it->first.find( '.' )    ;
					fieldNam = it->first.substr( 0, p ) ;
					fieldNum = ds2d( it->first.substr( p+1 ) )  ;
					iCursor  = fieldNam ;
				}
				else
				{
					iCursor = it->first ;
				}
				iCsrpos = posn ;
			}
		}
	}

	if ( tb_model && p_row >= tb_toprow && p_row < ( tb_toprow + tb_depth ) )
	{
		tb_curidx = p_row - tb_toprow ;
	}

	p_poolMGR->put( err, "ZSCROLLA", "",  SHARED ) ;
	if ( err.error() ) { return ; }

	p_poolMGR->put( err, "ZSCROLLN", "0", SHARED ) ;
	if ( err.error() ) { return ; }

	if ( scrollOn && findword( cmdVerb, "UP DOWN LEFT RIGHT" ) )
	{
		cmd    = upper( cmd_getvalue() ) ;
		msgfld = cmdfield ;
		if ( cmd == "" || ( !findword( cmd, scrollAmounts ) && !isnumeric( cmd ) ) )
		{
			cmd    = fieldList[ scroll ]->field_value ;
			msgfld = scroll ;
		}
		else
		{
			cmd_setvalue( "" ) ;
			p_funcPOOL->put2( err, cmdfield, "" ) ;
			if ( err.error() ) { return ; }
		}
		if      ( tb_model )                          { maxAmount = tb_depth  ; }
		else if ( findword( cmdVerb, "LEFT RIGHT" ) ) { maxAmount = dyn_width ; }
		else                                          { maxAmount = dyn_depth ; }
		if ( isnumeric( cmd ) )
		{
			if ( cmd.size() > 6 )
			{
				set_message_cond( "PSYS011I" ) ;
				set_cursor_cond( msgfld )      ;
				dCsrpos = 1 ;
			}
			else
			{
				p_poolMGR->put( err, "ZSCROLLA", cmd, SHARED ) ;
				if ( err.error() ) { return ; }
				p_poolMGR->put( err, "ZSCROLLN", cmd, SHARED ) ;
				if ( err.error() ) { return ; }
			}
		}
		else if ( cmd.front() == 'M' )
		{
			p_poolMGR->put( err, "ZSCROLLA", "MAX", SHARED ) ;
			if ( err.error() ) { return ; }
		}
		else if ( cmd.front() == 'C' )
		{
			if ( fieldNum == -1 )
			{
				Amnt = maxAmount ;
			}
			else if ( cmdVerb.front() == 'U' )
			{
				Amnt = maxAmount - fieldNum - 1 ;
			}
			else if ( cmdVerb.front() == 'D' )
			{
				Amnt = fieldNum ;
			}
			else if ( cmdVerb.front() == 'L' )
			{
				Amnt = maxAmount - posn ;
			}
			else
			{
				Amnt = posn - 1 ;
			}
			p_poolMGR->put( err, "ZSCROLLN", ( Amnt == 0 ) ? d2ds( maxAmount ) : d2ds( Amnt ), SHARED ) ;
			if ( err.error() ) { return ; }
			p_poolMGR->put( err, "ZSCROLLA", "CSR", SHARED ) ;
			if ( err.error() ) { return ; }
		}
		else if ( cmd.front() == 'D' )
		{
			p_poolMGR->put( err, "ZSCROLLN", d2ds( maxAmount - 1 ), SHARED ) ;
			if ( err.error() ) { return ; }
			p_poolMGR->put( err, "ZSCROLLA", "DATA", SHARED ) ;
			if ( err.error() ) { return ; }
		}
		else if ( cmd.front() == 'H' )
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


void pPanel::display_panel_attrs( errblock& err )
{
	// Go through the list of character attributes and perform any
	// variable substitution after )INIT processing has completed.

	string t ;

	for ( auto it = char_attrlist.begin() ; it != char_attrlist.end() ; ++it )
	{
		if ( it->second.has_dvars() )
		{
			t = sub_vars( it->second.get() ) ;
			it->second.update( err, t ) ;
			if ( err.error() ) { return ; }
			colour_attrlist[ it->first ] = it->second.get_colour() ;
			if ( ddata_map.count( it->first ) > 0 )
			{
				ddata_map[ it->first ] = it->second.get_colour() ;
			}
			else if ( schar_map.count( it->first ) > 0 )
			{
				schar_map[ it->first ] = it->second.get_colour() ;
			}
		}
	}
}


void pPanel::display_panel_init( errblock& err )
{
	// Perform panel )INIT processing

	set_pfpressed( "" ) ;
	err.setRC( 0 ) ;

	putDialogueVar( err, "ZPRIM", "" ) ;
	if ( err.error() ) { return ; }

	reset_attrs() ;

	process_panel_stmnts( err,
			      0,
			      initstmnts,
			      PS_INIT ) ;
}


void pPanel::display_panel_reinit( errblock& err, int ln )
{
	// Perform panel )REINIT processing

	set_pfpressed( "" ) ;
	err.setRC( 0 ) ;

	if ( dCursor != "" )
	{
		iCursor = dCursor ;
		dCursor = "" ;
	}

	process_panel_stmnts( err,
			      ln,
			      reinstmnts,
			      PS_REINIT ) ;
}


void pPanel::display_panel_proc( errblock& err, int ln )
{
	// Perform panel )PROC processing
	// If a selection panel, check .TRAIL if not NOCHECK on the ZSEL variable and issue error

	int p ;

	string zsel ;

	err.setRC( 0 ) ;

	process_panel_stmnts( err,
			      ln,
			      procstmnts,
			      PS_PROC ) ;
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


void pPanel::abc_panel_init( errblock& err,
			     const string& abc_desc )
{
	// Perform panel )ABCINIT processing

	err.setRC( 0 ) ;

	dCursor = "" ;
	dMsg    = "" ;
	setControlVar( err, ".ZVARS", "", PS_ABCINIT ) ;

	process_panel_stmnts( err,
			      0,
			      abc_initstmnts[ abc_desc ],
			      PS_ABCINIT ) ;
}


void pPanel::abc_panel_proc( errblock& err,
			     const string& abc_desc )
{
	// Perform panel )ABCPROC processing

	err.setRC( 0 ) ;

	dCursor = "" ;
	dMsg    = "" ;

	process_panel_stmnts( err,
			      0,
			      abc_procstmnts[ abc_desc ],
			      PS_ABCPROC ) ;
}


void pPanel::process_panel_stmnts( errblock& err,
				   int ln,
				   vector<panstmnt* >& panstmnts,
				   PS_SECT ps_sect )
{
	// General routine to process panel statements.  Pass address of the panel statements vector for the
	// panel section being processed.

	// For table display fields, .ATTR and .CURSOR apply to fields on line ln

	// If the panel statment has a ps_if address and a ps_xxx address, this is for statements coded
	// inline on the if-statement so test and execute if the if-statement is true.

	int if_column ;

	bool if_skip ;

	string g_label ;

	g_label   = "" ;
	if_column = 0  ;
	if_skip   = false ;

	set<string>attr_sect ;
	set<string>attrchar_sect ;

	for ( auto ips = panstmnts.begin() ; ips != panstmnts.end() ; ++ips )
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
			for ( ; ips != panstmnts.end() ; ++ips )
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
			refresh_fields( err, ln, (*ips)->ps_rlist ) ;
			if ( err.error() ) { return ; }
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
						  (*ips)->ps_assgn,
						  attr_sect,
						  attrchar_sect,
						  ps_sect ) ;
			if ( err.error() ) { return ; }
		}
		else if ( (*ips)->ps_ver )
		{
			if ( dMsg == "" )
			{
				process_panel_verify( err,
						      ln,
						      (*ips)->ps_ver ) ;
				if ( err.error() ) { return ; }
			}
		}
	}
}


void pPanel::process_panel_if( errblock& err,
			       int ln,
			       IFSTMNT* ifstmnt )
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


void pPanel::process_panel_if_cond( errblock& err,
				    int ln,
				    IFSTMNT* ifstmnt )
{
	uint j ;

	bool l_break ;
	bool numeric ;

	string rhs_val  ;
	string lhs_val  ;
	string fieldVal ;
	string fieldNam ;

	vector<string>::iterator it ;

	ifstmnt->if_true = ( ifstmnt->if_cond == IF_NE ) ;

	if ( ifstmnt->if_verify )
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
				      it != ifstmnt->if_verify->ver_vlist.end()  ; ++it )
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

			case VER_NAME:
				ifstmnt->if_true = isvalidName( fieldVal ) ;
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

	if ( ifstmnt->if_func == PN_TRANS )
	{
		lhs_val = process_panel_trans( err, ln, ifstmnt->if_trans, ifstmnt->if_lhs.value ) ;
		if ( err.error() ) { return ; }
	}
	else if ( ifstmnt->if_func == PN_TRUNC )
	{
		lhs_val = process_panel_trunc( err, ifstmnt->if_trunc ) ;
		if ( err.error() ) { return ; }
	}
	else if ( ifstmnt->if_lhs.subtype == TS_CTL_VAR_VALID ||
		  ifstmnt->if_lhs.subtype == TS_AMPR_VAR_VALID )
	{
		lhs_val = getVariable( err, ifstmnt->if_lhs ) ;
		if ( err.error() ) { return ; }
		process_panel_function( ifstmnt->if_func, lhs_val ) ;
	}
	else
	{
		lhs_val = sub_vars( ifstmnt->if_lhs.value ) ;
	}

	for ( j = 0 ; j < ifstmnt->if_rhs.size() ; ++j )
	{
		l_break = false ;
		if ( ifstmnt->if_rhs[ j ].subtype == TS_CTL_VAR_VALID )
		{
			rhs_val = getControlVar( err, ifstmnt->if_rhs[ j ].value ) ;
			if ( err.error() ) { return ; }
		}
		else if ( ifstmnt->if_rhs[ j ].subtype == TS_AMPR_VAR_VALID )
		{
			rhs_val = getDialogueVar( err, ifstmnt->if_rhs[ j ].value ) ;
			if ( err.error() ) { return ; }
		}
		else
		{
			rhs_val = sub_vars( ifstmnt->if_rhs[ j ].value ) ;
		}
		numeric = isnumeric( lhs_val ) && isnumeric( rhs_val ) ;
		switch ( ifstmnt->if_cond )
		{
		case IF_EQ:
			if ( numeric )
			{
				ifstmnt->if_true = ifstmnt->if_true || ( ds2d( lhs_val ) == ds2d( rhs_val ) ) ;
			}
			else
			{
				ifstmnt->if_true = ifstmnt->if_true || ( lhs_val == rhs_val ) ;
			}
			l_break = ifstmnt->if_true ;
			break ;

		case IF_NE:
			if ( numeric )
			{
				ifstmnt->if_true = ifstmnt->if_true && ( ds2d( lhs_val ) != ds2d( rhs_val ) ) ;
			}
			else
			{
				ifstmnt->if_true = ifstmnt->if_true && ( lhs_val != rhs_val ) ;
			}
			l_break = !ifstmnt->if_true ;
			break ;

		case IF_GT:
			if ( numeric )
			{
				ifstmnt->if_true = ifstmnt->if_true || ( ds2d( lhs_val ) > ds2d( rhs_val ) ) ;
			}
			else
			{
				ifstmnt->if_true = ifstmnt->if_true || ( lhs_val > rhs_val ) ;
			}
			break ;

		case IF_GE:
			if ( numeric )
			{
				ifstmnt->if_true = ( ds2d( lhs_val ) >= ds2d( rhs_val ) ) ;
			}
			else
			{
				ifstmnt->if_true = ( lhs_val >= rhs_val ) ;
			}
			break ;

		case IF_LE:
			if ( numeric )
			{
				ifstmnt->if_true = ( ds2d( lhs_val ) <= ds2d( rhs_val ) ) ;
			}
			else
			{
				ifstmnt->if_true = ( lhs_val <= rhs_val ) ;
			}
			break ;

		case IF_LT:
			if ( numeric )
			{
				ifstmnt->if_true = ( ds2d( lhs_val ) < ds2d( rhs_val ) ) ;
			}
			else
			{
				ifstmnt->if_true = ( lhs_val < rhs_val ) ;
			}
			break ;

		}
		if ( l_break ) { break ; }
	}
}


string pPanel::process_panel_trunc( errblock& err, TRUNC* trunc )
{
	size_t p ;

	string t ;
	string rest ;

	setControlVar( err, ".TRAIL", "", PS_PROC ) ;
	if ( err.error() ) { return "" ; }

	t = getVariable( err, trunc->trnc_field ) ;
	if ( err.error() ) { return "" ; }

	p = trunc->trnc_len ;
	if ( p > 0 )
	{
		if ( p > t.size() )
		{
			rest = "" ;
		}
		else
		{
			rest = strip( t.substr( p ) ) ;
			t    = t.substr( 0, p ) ;
		}
	}
	else
	{
		p = t.find( trunc->trnc_char ) ;
		if ( p == string::npos )
		{
			rest = "" ;
		}
		else
		{
			rest = strip( t.substr( p+1 ) ) ;
			t    = t.substr( 0, p ) ;
		}
	}

	setControlVar( err, ".TRAIL", rest, PS_PROC ) ;
	if ( err.error() ) { return "" ; }

	return t ;
}


string pPanel::process_panel_trans( errblock& err,
				    int ln,
				    TRANS* trans,
				    const string& as_lhs )
{
	string t ;

	vector<pair<string,string>>::iterator it ;

	if ( trans->trns_trunc )
	{
		t = process_panel_trunc( err, trans->trns_trunc ) ;
		if ( err.error() ) { return "" ; }
	}
	else
	{
		t = getVariable( err, trans->trns_field ) ;
		if ( err.error() ) { return "" ; }
	}

	for ( it = trans->trns_list.begin() ; it != trans->trns_list.end() ; ++it )
	{
		if ( t == sub_vars( it->first ) )
		{
			t = sub_vars( it->second ) ;
			break ;
		}
	}

	if ( it == trans->trns_list.end() )
	{
		if ( trans->trns_default == "" )
		{
			t = "" ;
			if ( trans->trns_msgid != "" && dMsg == "" )
			{
				set_message_cond( sub_vars( trans->trns_msgid ) ) ;
				if ( fieldList.count( trans->trns_field.value ) > 0 )
				{
					set_cursor_cond( trans->trns_field.value, ln ) ;
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


void pPanel::process_panel_verify( errblock& err,
				   int ln,
				   VERIFY* verify )
{
	uint i ;

	string t   ;
	string msg ;
	string fieldNam ;
	string fieldVal ;

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
		for ( it = verify->ver_vlist.begin() ; it != verify->ver_vlist.end() ; ++it )
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
				for ( i = 0 ; i < verify->ver_vlist.size() - 2 ; ++i )
				{
					t += sub_vars( verify->ver_vlist[ i ] ) + ", " ;
				}
				t += sub_vars( verify->ver_vlist[ i ] ) ;
				t += " and " ;
				++i ;
				t += sub_vars( verify->ver_vlist[ i ] ) ;
			}
			msg = ( verify->ver_type == VER_LIST ) ? "PSYS011B" : "PSYS012Q" ;
			set_message_cond( verify->ver_msgid == "" ? msg : sub_vars( verify->ver_msgid ) ) ;
			if ( dMsg == msg )
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

	case VER_NAME:
		if ( !isvalidName( fieldVal ) )
		{
			set_message_cond( verify->ver_msgid == "" ? "PSYS012U" : sub_vars( verify->ver_msgid ) ) ;
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
		for ( j = 1 ; j <= ws ; ++j )
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
		for ( j = 1 ; j <= ws ; ++j )
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
					p_funcPOOL->put1( err, var, val ) ;
					if ( err.error() ) { return ; }
				}
			}
		}
	}
	if ( err.RC8() ) { err.setRC( 0 ) ; }
}


void pPanel::process_panel_assignment( errblock& err,
				       int ln,
				       ASSGN* assgn,
				       set<string>& attr_sect,
				       set<string>& attrchar_sect,
				       PS_SECT ps_sect )
{
	// Assignment processing.
	// Only action the first .ATTR or .ATTRCHAR statement in a panel section for the same field.

	string rhs ;
	string lhs ;

	if ( assgn->as_func == PN_TRANS )
	{
		rhs = process_panel_trans( err, ln, assgn->as_trans, assgn->as_lhs.value ) ;
		if ( err.error() ) { return ; }
	}
	else if ( assgn->as_func == PN_TRUNC )
	{
		rhs = process_panel_trunc( err, assgn->as_trunc ) ;
		if ( err.error() ) { return ; }
	}
	else if ( assgn->as_rhs.subtype == TS_AMPR_VAR_VALID ||
		  assgn->as_rhs.subtype == TS_CTL_VAR_VALID )
	{
		rhs = getVariable( err, assgn->as_rhs ) ;
		if ( err.error() ) { return ; }
		process_panel_function( assgn->as_func, rhs ) ;
	}
	else
	{
		rhs = sub_vars( assgn->as_rhs.value ) ;
	}

	if ( !assgn->as_isattr && assgn->as_lhs.subtype == TS_CTL_VAR_VALID )
	{
		setControlVar( err, assgn->as_lhs.value, upper( rhs ), ps_sect ) ;
		if ( err.error() ) { return ; }
	}
	else if ( assgn->as_isattr )
	{
		process_panel_attr( err,
				    ln,
				    rhs,
				    assgn,
				    attr_sect,
				    ps_sect ) ;
		if ( err.error() ) { return ; }
	}
	else if ( assgn->as_isattc )
	{
		process_panel_attrchar( err,
					rhs,
					assgn,
					attrchar_sect ) ;
		if ( err.error() ) { return ; }
	}
	else
	{
		putDialogueVar( err, assgn->as_lhs.value, rhs ) ;
		if ( err.error() ) { return ; }
		if ( assgn->as_lhs.value == "ZPRIM" )
		{
			primaryMenu = ( rhs == "YES" ) ;
		}
	}
}


void pPanel::process_panel_attr( errblock& err,
				 int ln,
				 const string& rhs,
				 ASSGN* assgn,
				 set<string>& attr_sect,
				 PS_SECT ps_sect )
{
	// .ATTR() processing.  For a TB model field in the )INIT section, perform for every
	// model line on the screen.

	int i ;

	string t   ;
	string lhs ;

	if ( assgn->as_lhs.subtype == TS_AMPR_VAR_VALID || assgn->as_lhs.subtype == TS_CTL_VAR_VALID )
	{
		lhs = getVariable( err, assgn->as_lhs ) ;
		if ( lhs == "" || err.error() ) { return ; }
	}
	else
	{
		lhs = assgn->as_lhs.value ;
	}

	if ( fieldList.count( lhs ) == 0 && not tb_field( lhs ) )
	{
		err.seterrid( "PSYE041S", lhs ) ;
		return ;
	}

	if ( attr_sect.count( lhs ) > 0 ) { return ; }

	attr_sect.insert( lhs ) ;
	if ( ps_sect == PS_INIT && tb_field( lhs ) )
	{
		i = 0 ;
		do
		{
			t = lhs + "." + d2ds( i ) ;
			fieldList[ t ]->field_attr( err, rhs, false ) ;
			if ( err.error() ) { return ; }
			attrList.push_back( t ) ;
			++i ;
		} while ( i < tb_depth ) ;
	}
	else
	{
		if ( tb_field( lhs ) )
		{
			lhs += "." + d2ds( ln ) ;
		}
		fieldList[ lhs ]->field_attr( err, rhs, ( ps_sect == PS_PROC || ps_sect == PS_REINIT ) ) ;
		if ( err.error() ) { return ; }
		attrList.push_back( lhs ) ;
	}
}


void pPanel::process_panel_attrchar( errblock& err,
				     string& rhs,
				     ASSGN* assgn,
				     set<string>& attrchar_sect )
{
	// .ATTRCHAR() processing.

	string lhs ;

	lhs = assgn->as_lhs.value ;
	if ( assgn->as_lhs.subtype == TS_AMPR_VAR_VALID )
	{
		lhs = getDialogueVar( err, lhs ) ;
		if ( lhs == "" || err.error() ) { return ; }
		if ( lhs.size() > 2 || ( lhs.size() == 2 && not ishex( lhs ) ) )
		{
			err.seterrid( "PSYE041B" ) ;
			return ;
		}
		if ( lhs.size() == 2 ) { lhs = xs2cs( lhs ) ; }
	}

	if ( attrchar_sect.count( lhs ) > 0 ) { return ; }

	attrchar_sect.insert( lhs ) ;
	attrchar( err, lhs.front(), rhs ) ;
	if ( err.error() ) { return ; }
}


void pPanel::process_panel_function( PN_FUNCTION func, string& s )
{
	switch ( func )
	{
	case PN_LENGTH:
		s = d2ds( s.size() ) ;
		break ;

	case PN_REVERSE:
		reverse( s.begin(), s.end() ) ;
		break ;

	case PN_UPPER:
		iupper( s ) ;
		break ;

	case PN_WORDS:
		s = d2ds( words( s ) ) ;
		break ;

	case PN_EXISTS:
		try
		{
			s = exists( s ) ? "1" : "0" ;
		}
		catch (...)
		{
			s = "0" ;
		}
		break ;

	case PN_FILE:
		try
		{
			s = is_regular_file( s ) ? "1" : "0" ;
		}
		catch (...)
		{
			s = "0" ;
		}
		break ;

	case PN_DIR:
		try
		{
			s = is_directory( s ) ? "1" : "0" ;
		}
		catch (...)
		{
			s = "0" ;
		}
	}
}


string pPanel::getVariable( errblock& err, const vparm& svar )
{
	if ( svar.subtype == TS_CTL_VAR_VALID )
	{
		return getControlVar( err, svar.value ) ;
	}
	else
	{
		return getDialogueVar( err, svar.value ) ;
	}
}


string pPanel::getControlVar( errblock& err, const string& svar )
{
	auto it = control_vars.find( svar ) ;
	if ( it == control_vars.end() )
	{
		err.seterrid( "PSYE033G", svar ) ;
		return "" ;
	}

	switch ( it->second )
	{
	case CV_ALARM:
		return dAlarm ;

	case CV_AUTOSEL:
		return tb_autosel ? "YES" : "NO" ;

	case CV_BROWSE:
		return forBrowse ? "YES" : "NO" ;

	case CV_CSRPOS:
		return d2ds( get_csrpos(), 4 ) ;

	case CV_CSRROW:
		return d2ds( tb_csrrow, 8 ) ;

	case CV_CURSOR:
		return get_cursor() ;

	case CV_EDIT:
		return forEdit ? "YES" : "NO" ;

	case CV_FALSE:
		return "0" ;

	case CV_HELP:
		return dHelp ;

	case CV_MSG:
		return get_msg() ;

	case CV_NRET:
		return nretriev ? "ON" : "OFF" ;

	case CV_PFKEY:
		return get_pfpressed() ;

	case CV_RESP:
		return end_pressed ? "END" : "ENTER" ;

	case CV_TRUE:
		return "1" ;

	case CV_TRAIL:
		return dTrail ;

	case CV_ZVARS:
		return dZvars ;
	}

	return svar ;
}


void pPanel::setControlVar( errblock& err,
			    const string& svar,
			    const string& sval,
			    PS_SECT ps_sect )
{
	auto it = control_vars.find( svar ) ;
	if ( it == control_vars.end() )
	{
		err.seterrid( "PSYE033G", svar ) ;
		return ;
	}

	switch ( it->second )
	{
	case CV_ALARM:
		if ( sval == "YES" )
		{
			dAlarm = "YES" ;
		}
		else if ( sval == "NO" || sval == "" )
		{
			dAlarm = "NO" ;
		}
		else
		{
			err.seterrid( "PSYE041G" ) ;
		}
		break ;

	case CV_AUTOSEL:
		if ( ps_sect != PS_INIT && ps_sect != PS_REINIT ) { break ; }
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
		break ;

	case CV_BROWSE:
		forBrowse = ( sval == "YES" ) ;
		break ;

	case CV_CSRPOS:
		if ( dCsrpos == 0 )
		{
			if ( !isnumeric( sval ) || ds2d( sval ) < 1 )
			{
				err.seterrid( "PSYE041J" ) ;
				return ;
			}
			dCsrpos = ds2d( sval ) ;
		}
		break ;

	case CV_CSRROW:
		if ( !isnumeric( sval ) || ds2d( sval ) < 0 )
		{
			err.seterrid( "PSYE041H" ) ;
			return ;
		}
		tb_csrrow = ds2d( sval ) ;
		break ;

	case CV_CURSOR:
		if ( dCursor == "" )
		{
			dCursor = sval ;
			msgloc  = ""   ;
			if ( dCsrpos == 0 )
			{
				iCsrpos = 1 ;
			}
		}
		break ;

	case CV_EDIT:
		forEdit = ( sval == "YES" ) ;
		break ;

	case CV_HELP:
		dHelp = sval ;
		break ;

	case CV_NRET:
		if      ( sval == "ON" )        { nretriev  = true  ; }
		else if ( isvalidName( sval ) ) { nretfield = sval  ; }
		else                            { nretriev  = false ; }
		break ;

	case CV_MSG:
		set_message_cond( sval ) ;
		break ;

	case CV_RESP:
		if ( sval == "ENTER" )
		{
			dResp = sval ;
			end_pressed = false ;
		}
		else if ( sval == "END" )
		{
			dResp = sval ;
			end_pressed = true ;
		}
		else
		{
			err.seterrid( "PSYE041I" ) ;
		}
		break ;

	case CV_TRAIL:
		dTrail = sval ;
		break ;

	case CV_ZVARS:
		if ( sval != "" && !isvalidName( sval ) )
		{
			err.seterrid( "PSYE013A", sval, ".ZVARS )ABCINIT statement" ) ;
			return  ;
		}
		dZvars = sval ;
	}
}


void pPanel::set_message_cond( const string& msg )
{
	if ( dMsg == "" )
	{
		dMsg = msg ;
	}
}


void pPanel::set_cursor_cond( const string& csr, int i )
{
	if ( dCursor == "" )
	{
		dCursor   = csr ;
		iCsrpos   = 1   ;
		msgloc    = csr ;
		tb_curidr = i   ;
	}
}


void pPanel::set_cursor( const string& csr, int p )
{
	iCursor = csr ;
	iCsrpos = max( 1, p ) ;
}


string pPanel::get_tbCursor( const string& cursor )
{
	// This is only called for a table display field.
	// Return cursor position as set by get_cursor(), tb_curidr and tb_csrrow.

	// Use tb_curidr if set, else csrrow.
	// If visible, return the internal field name otherwise the command field.

	int p    ;
	int vrow ;

	errblock err ;

	if ( tb_curidr > -1 )
	{
		return cursor + "." + d2ds( tb_curidr ) ;
	}
	else if ( tb_csrrow > 0 )
	{
		vrow = p_funcPOOL->get( err, 0, INTEGER, "ZTDVROWS", NOCHECK ) ;
		if ( err.error() ) { return "" ; }
		p    = tb_csrrow - p_funcPOOL->get( err, 0, INTEGER, "ZTDTOP", NOCHECK ) ;
		if ( err.error() ) { return "" ; }
		if ( p >= 0 && p <= vrow )
		{
			return cursor + "." + d2ds( p ) ;
		}
	}
	return cmdfield ;
}


string pPanel::get_tbCursor( const string& cursor, int crp )
{
	// This is only called for a table display field.

	// Return cursor position as set by get_cursor() and the crp.
	// If visible, return the internal field name otherwise the command field.

	int p    ;
	int vrow ;

	errblock err ;

	if ( crp > 0 )
	{
		vrow = p_funcPOOL->get( err, 0, INTEGER, "ZTDVROWS", NOCHECK ) ;
		if ( err.error() ) { return "" ; }
		p    = crp - p_funcPOOL->get( err, 0, INTEGER, "ZTDTOP", NOCHECK ) ;
		if ( err.error() ) { return "" ; }
		if ( p >= 0 && p <= vrow )
		{
			return cursor + "." + d2ds( p ) ;
		}
	}
	return cmdfield ;
}


string pPanel::get_msgloc()
{
	int p    ;
	int vrow ;

	errblock err ;

	if ( tb_field( msgloc ) )
	{
		if ( tb_curidr > -1 )
		{
			return msgloc + "." + d2ds( tb_curidr ) ;
		}
		else if ( tb_csrrow > 0 )
		{
			vrow = p_funcPOOL->get( err, 0, INTEGER, "ZTDVROWS", NOCHECK ) ;
			if ( err.error() ) { return "" ; }
			p    = tb_csrrow - p_funcPOOL->get( err, 0, INTEGER, "ZTDTOP", NOCHECK ) ;
			if ( err.error() ) { return "" ; }
			if ( p >= 0 && p <= vrow )
			{
				return msgloc + "." + d2ds( p ) ;
			}
		}
		return "" ;
	}
	return msgloc ;
}


bool pPanel::tb_field( const string& f )
{
	return ( tb_fields.count( f ) > 0 ) ;
}


void pPanel::point_and_shoot( uint row, uint col )
{
	// If the cursor is on a point-and-shoot field or text, set the pnts field, if blank, to the pnts value.

	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find entry.

	row -= win_row ;
	col -= win_col ;

	map<string, pnts>::iterator itp = pntsTable.end() ;
	map<string, field*>::iterator itf ;
	vector<text*>::iterator itt ;

	if ( field_pas.empty() && text_pas.empty() ) { return ; }

	for ( itf = field_pas.begin() ; itf != field_pas.end() ; ++itf )
	{
		if ( itf->second->field_visible  &&
		     itf->second->field_active   &&
		     itf->second->cursor_on_field( row, col ) )
		{
			itp = pntsTable.find( itf->first ) ;
			break ;
		}
	}

	if ( itf == field_pas.end() )
	{
		for ( itt = text_pas.begin() ; itt != text_pas.end() ; ++itt )
		{
			if ( (*itt)->text_visible && (*itt)->cursor_on_text( row, col ) )
			{
				itp = pntsTable.find( (*itt)->text_name ) ;
				break ;
			}
		}
	}

	if ( itp != pntsTable.end() )
	{
		itf = fieldList.find( itp->second.pnts_var ) ;
		if ( strip( itf->second->field_value ) == "" )
		{
			itf->second->field_value = itp->second.pnts_val ;
		}
	}
}


void pPanel::refresh_fields( errblock& err, int ln, const string& fields )
{
	// Update the field value from the dialogue variable.  Apply any field justification defined.
	// For REFRESH * and table display fields, only refresh variables on line ln.

	int i  ;
	int j  ;
	int k  ;
	int p  ;
	int ws ;

	string temp    ;
	string* darea  ;
	string* shadow ;

	map<string, field*>::iterator   itf ;
	map<string, dynArea*>::iterator itd ;

	map<string,string> fconv ;

	if ( tb_model && fields != "*" )
	{
		for ( ws = words( fields ), i = 1 ; i <= ws ; ++i )
		{
			temp = word( fields, i ) ;
			if ( tb_field( temp ) )
			{
				fconv[ temp + "." + d2ds( ln ) ] = temp ;
			}
		}
	}

	for ( itf = fieldList.begin() ; itf != fieldList.end() ; ++itf )
	{
		if ( not itf->second->field_dynArea &&
		   ( fields == "*" || findword( itf->first, fields ) || fconv.count( itf->first ) > 0 ) )
		{
			if ( fconv.count( itf->first ) > 0 )
			{
				refresh_field( err, itf, fconv[ itf->first ] ) ;
				if ( err.error() ) { return ; }
			}
			else if ( fields == "*" && itf->second->field_tb )
			{
				p    = itf->first.find( '.' ) ;
				temp = itf->first.substr( 0, p ) ;
				if ( ds2d( itf->first.substr( p+1 ) ) == ln )
				{
					refresh_field( err, itf, temp ) ;
					if ( err.error() ) { return ; }
				}
			}
			else
			{
				refresh_field( err, itf, itf->first ) ;
				if ( err.error() ) { return ; }
			}
		}
	}

	for ( itd = dynAreaList.begin() ; itd != dynAreaList.end() ; ++itd )
	{
		if ( fields == "*" || findword( itd->first, fields ) )
		{
			k     = itd->second->dynArea_width ;
			darea = p_funcPOOL->vlocate( err, itd->first ) ;
			if ( err.error() ) { return ; }
			shadow = p_funcPOOL->vlocate( err, itd->second->dynArea_shadow_name ) ;
			if ( err.error() ) { return ; }
			for ( unsigned int i = 0 ; i < itd->second->dynArea_depth ; ++i )
			{
				j   = i * itd->second->dynArea_width ;
				itf = fieldList.find( itd->first + "." + d2ds( i ) ) ;
				itf->second->field_value        = darea->substr( j, k )  ;
				itf->second->field_shadow_value = shadow->substr( j, k ) ;
			}
		}
	}
}


void pPanel::refresh_fields( errblock& err )
{
	// Update all field values from their dialogue variables and display.

	int j ;
	int k ;

	string* darea  ;
	string* shadow ;

	map<string, field*>::iterator   itf ;
	map<string, dynArea*>::iterator itd ;

	for ( itf = fieldList.begin() ; itf != fieldList.end() ; ++itf )
	{
		if ( not itf->second->field_dynArea )
		{
			refresh_field( err, itf, itf->first ) ;
			if ( err.error() ) { return ; }
			itf->second->display_field( win, inv_schar, ddata_map, schar_map ) ;
		}
	}

	for ( itd = dynAreaList.begin() ; itd != dynAreaList.end() ; ++itd )
	{
		k      = itd->second->dynArea_width ;
		darea  = p_funcPOOL->vlocate( err, itd->first ) ;
		if ( err.error() ) { return ; }
		shadow = p_funcPOOL->vlocate( err, itd->second->dynArea_shadow_name ) ;
		if ( err.error() ) { return ; }
		for ( unsigned int i = 0 ; i < itd->second->dynArea_depth ; ++i )
		{
			j   = i * itd->second->dynArea_width ;
			itf = fieldList.find( itd->first +"."+ d2ds( i ) ) ;
			itf->second->field_value        = darea->substr( j, k )  ;
			itf->second->field_shadow_value = shadow->substr( j, k ) ;
			itf->second->display_field( win, inv_schar, ddata_map, schar_map ) ;
		}
	}
}


void pPanel::refresh_field( errblock& err, map<string, field*>::iterator it, const string& var )
{
	// Update the field value from the dialogue variable var and
	// apply any field justification defined.

	it->second->field_value = getDialogueVar( err, var ) ;
	if ( err.error() ) { return ; }

	it->second->field_prep_display() ;
	it->second->field_set_caps() ;
}


void pPanel::create_tbfield( errblock& err, const string& pline )
{
	// Default is JUST(ASIS) for fields of a TB model, so change from the default of JUST(LEFT)
	// Create implicit function pool variables for the TB field.

	int tlen ;
	int ws   ;

	uint tcol ;

	string w2   ;
	string w3   ;
	string w4   ;
	string opts ;
	string name ;
	string nidx ;

	attType fType ;

	field  a   ;
	field* fld ;

	ws = words( pline ) ;
	if ( ws < 6 )
	{
		err.seterrid( "PSYE035Q" ) ;
		return ;
	}
	w2 = word( pline, 2 ) ;
	w3 = word( pline, 3 ) ;
	w4 = word( pline, 4 ) ;

	opts = upper( subword( pline, 5, ws-5 ) ) ;
	name = word( pline, ws ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE031F", name, "TBFIELD" ) ;
		return  ;
	}

	if ( tb_field( name ) )
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
		else if ( w2 == "MAX" )                     { tcol = wscrmaxw   ; }
		else if ( w2.compare( 0, 4, "MAX-" ) == 0 ) { tcol = wscrmaxw - ds2d( substr( w2, 5 ) ) ; }
		else
		{
			err.seterrid( "PSYE031S", w2 ) ;
			return ;
		}
	}

	if      ( isnumeric( w3 ) )                 { tlen = ds2d( w3 ) ; }
	else if ( w3 == "MAX" )                     { tlen = wscrmaxw - tcol + 1 ; }
	else if ( w3.compare( 0, 4, "MAX-" ) == 0 ) { tlen = wscrmaxw - tcol - ds2d( substr( w3, 5 ) ) + 1 ; }
	else
	{
		err.seterrid( "PSYE031S", w3 ) ;
		return ;
	}

	if ( tcol > wscrmaxw )
	{
		err.seterrid( "PSYE031B", d2ds( tcol ), d2ds( wscrmaxw ) ) ;
		return ;
	}

	tb_lcol = tcol ;
	tb_lsz  = tlen ;

	if ( cuaAttrName.count( w4 ) == 0 && noncuaAttrName.count( w4 ) == 0 )
	{
		err.seterrid( "PSYE032F", w4 ) ;
		return ;
	}
	fType = cuaAttrName[ w4 ] ;

	a.field_just = 'A'        ;
	a.field_opts( err, opts ) ;
	if ( err.error() ) { return ; }

	if ( cuaAttrName.count( w4 ) > 0 )
	{
		a.field_cua     = fType ;
		a.field_colour1 = cuaAttr[ fType ] ;
	}
	else
	{
		a.field_cua     = NONE ;
		a.field_colour1 = ( w4 == "INPUT" ) ? RED : WHITE ;
	}
	a.field_col     = tcol - 1 ;
	a.field_length  = tlen     ;
	a.field_endcol  = tcol + tlen - 2 ;
	a.field_input   = ( attrUnprot.count( fType ) > 0 ) ;
	a.field_tb      = true ;

	for ( int i = 0 ; i < tb_depth ; ++i )
	{
		fld  = new field ;
		*fld = a ;
		fld->field_row = tb_toprow + i ;
		nidx = name +"."+ d2ds( i ) ;
		fieldList[ nidx ] = fld ;
		p_funcPOOL->put3( err, nidx, "" ) ;
		if ( err.error() ) { return ; }
	}
	tb_fields.insert( name ) ;

}


void pPanel::create_pdc( errblock& err,
			 const string& abc_desc,
			 uint abc_mnem,
			 const string& pline )
{
	// ab is a vector list of action-bar-choices (abc objects)
	// Each action-bar-choice is a vector list of pull-down-choices (pdc objects)

	uint p ;

	string head ;
	string tail ;

	string pdc_desc ;
	string pdc_run  ;
	string pdc_parm ;
	string pdc_unavail ;

	abc* t_abc ;

	vector<abc*>::iterator it ;

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

	for ( it = ab.begin() ; it != ab.end() ; ++it )
	{
		if ( (*it)->abc_desc == abc_desc ) { break ; }
	}

	if ( it == ab.end() )
	{
		t_abc = new abc( p_funcPOOL, selectPanel ) ;
		t_abc->abc_desc = abc_desc ;
		if ( abc_mnem > 0 )
		{
			t_abc->abc_mnem1 = abc_mnem ;
			t_abc->abc_mnem2 = toupper( abc_desc[ abc_mnem - 1 ] ) ;
		}
		t_abc->abc_col  = abc_pos ;
		abc_pos        += abc_desc.size() + 2 ;
		ab.push_back( t_abc ) ;
		it = ab.begin() + ab.size() - 1 ;
	}

	(*it)->add_pdc( pdc( pdc_desc, pdc_run, pdc_parm, pdc_unavail ) ) ;
}


void pPanel::update_field_values( errblock& err )
{
	// Update field_values from the dialogue variables.
	// May not exist so treat RC=8 from getDialogueVar as normal completion.

	// Treat dynamic areas differently - they must reside in the function pool.
	// Use vlocate to get the dynamic area variables via their addresses to avoid
	// large string copies.

	int j ;
	int k ;

	string  sname  ;
	string* darea  ;
	string* shadow ;

	map<string, field*>::iterator itf ;
	map<string, dynArea*>::iterator itd ;

	err.setRC( 0 ) ;

	for ( itf = fieldList.begin() ; itf != fieldList.end() ; ++itf )
	{
		if ( !itf->second->field_dynArea )
		{
			itf->second->field_value = getDialogueVar( err, itf->first ) ;
			if ( err.error() ) { return ; }
		}
		itf->second->field_changed = false ;
	}
	if ( err.RC8() ) { err.setRC( 0 ) ; }

	for ( itd = dynAreaList.begin() ; itd != dynAreaList.end() ; ++itd )
	{
		darea = p_funcPOOL->vlocate( err, itd->first, NOCHECK ) ;
		if ( !err.RC0() )
		{
			err.seterrid( "PSYE032G", itd->first ) ;
			return ;
		}
		sname  = itd->second->dynArea_shadow_name ;
		shadow = p_funcPOOL->vlocate( err, sname, NOCHECK ) ;
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
		for ( unsigned int i = 0 ; i < itd->second->dynArea_depth ; ++i )
		{
			j   = i * itd->second->dynArea_width ;
			itf = fieldList.find( itd->first +"."+ d2ds( i ) ) ;
			itf->second->field_value        = darea->substr( j, k )  ;
			itf->second->field_shadow_value = shadow->substr( j, k ) ;
		}
	}
}


void pPanel::display_text()
{
	// Substitute dialogue variables and place result in text_xvalue.  If there are no
	// dialogue variables present, set text_dvars to false and clear text_value.

	for ( auto it = textList.begin() ; it != textList.end() ; ++it )
	{
		if ( (*it)->text_dvars )
		{
			(*it)->text_xvalue = sub_vars( (*it)->text_value, (*it)->text_dvars ) ;
			if ( !(*it)->text_dvars )
			{
				(*it)->text_value = "" ;
			}
		}
		(*it)->text_display( win ) ;
	}
}


void pPanel::display_fields( errblock& err, bool reset )
{
	map<string, field*>::iterator it ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; ++it )
	{
		if ( !it->second->field_dynArea )
		{
			it->second->field_prep_display() ;
			it->second->field_set_caps()     ;
		}
		else if ( reset )
		{
			it->second->field_reset() ;
		}
		it->second->display_field( win, inv_schar, ddata_map, schar_map ) ;
	}
}


void pPanel::display_ab()
{
	if ( ab.size() == 0 ) { return ; }

	for ( auto it = ab.begin() ; it != ab.end() ; ++it )
	{
		(*it)->display_abc_unsel( win ) ;
	}

	wattrset( win, cuaAttr[ ABSL ] | panel_intens ) ;
	mvwhline( win, 1, 0, ACS_HLINE, wscrmaxw ) ;
}



void pPanel::reset_attrs()
{
	for ( unsigned int i = 0 ; i < attrList.size() ; ++i )
	{
		fieldList[ attrList[ i ] ]->field_attr() ;
	}
	attrList.clear() ;
}


void pPanel::reset_attrs_once()
{
	// Reset attributes for fields that have been marked as a temporary attribute change,
	// ie. .ATTR() in the )REINIT and )PROC panel sections that are valid for only one redisplay.

	for ( auto ita = attrList.begin() ; ita != attrList.end() ; )
	{
		auto itf = fieldList.find( *ita ) ;
		if ( itf->second->field_attr_once )
		{
			itf->second->field_attr() ;
			ita = attrList.erase( ita ) ;
			continue ;
		}
		++ita ;
	}
}


void pPanel::cursor_placement( errblock& err )
{
	// Position cursor

	// Cursor placement for TBDISPL:
	//        CURSOR/CSRROW    - Place on row with CRN of CSRROW
	//        CURSOR/no CSRROW - Use CRP to indicate row.  If CRP=0, place on the command line.
	//     no CURSOR/CSRROW    - First field of row
	//     no CURSOR/no CSRROW - Place on the command line

	uint   f_pos = 1 ;
	string f_name ;

	if ( bypassCur ) { return ; }

	int    csrpos = get_csrpos() ;
	string cursor = get_cursor() ;

	uint oX ;
	uint oY ;

	map<uint, Area*>::iterator ita ;
	map<string, field*>::iterator itf ;
	map<string, dynArea*>::iterator itd ;

	p_row = 0 ;
	p_col = 0 ;

	if ( tb_model )
	{
		if ( cursor == "" )
		{
			if ( tb_csrrow > 0 )
			{
				f_name = get_tbCursor( get_first_tb_field() ) ;
			}
			else
			{
				f_name = cmdfield ;
			}
		}
		else if ( fieldList.count( cursor ) > 0 )
		{
			f_name = cursor ;
			f_pos  = csrpos ;
		}
		else if ( not tb_field( cursor ) )
		{
			err.seterrid( "PSYE022N", cursor, !isvalidName( cursor ) ? 20 : 12 ) ;
			return ;
		}
		else if ( tb_csrrow > 0 )
		{
			f_name = get_tbCursor( cursor ) ;
		}
		else if ( tb_crp > 0 )
		{
			f_name = get_tbCursor( cursor, tb_crp ) ;
		}
		else
		{
			f_name = cmdfield ;
		}
	}
	else
	{
		f_name = cursor ;
		f_pos  = csrpos ;
	}

	if ( f_name == "" ) { return ; }

	itf = fieldList.find( f_name ) ;
	if ( itf == fieldList.end() )
	{
		itd = dynAreaList.find( f_name ) ;
		if ( itd == dynAreaList.end() )
		{
			err.seterrid( "PSYE022N", f_name, !isvalidName( f_name ) ? 20 : 12 ) ;
			return ;
		}
		else
		{
			if ( f_pos < 1 ) { f_pos = 1 ; }
			--f_pos ;
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
		if ( f_pos < 1 || f_pos > itf->second->field_length )
		{
			f_pos = 1 ;
		}
		if ( itf->second->field_area > 0 && not itf->second->field_visible )
		{
			ita = AreaNum.find( itf->second->field_area ) ;
			ita->second->make_visible( itf->second ) ;
			rebuild_after_area_scroll( ita->second ) ;
		}
		p_col = itf->second->field_col + f_pos - 1 ;
		p_row = itf->second->field_row ;
	}
}


string pPanel::get_first_tb_field()
{
	uint i = wscrmaxw ;

	string t = "" ;

	for ( auto it1 = tb_fields.begin() ; it1 != tb_fields.end() ; ++it1 )
	{
		auto it2 = fieldList.find( *it1 + ".0" ) ;
		if ( it2->second->field_col < i )
		{
			i = it2->second->field_col ;
			t = *it1 ;
		}
	}
	return t ;
}


void pPanel::cursor_to_cmdfield( unsigned int f_pos )
{
	map<string, field*>::iterator it ;

	p_row = 0 ;
	p_col = 0 ;

	if ( cmdfield == "" ) { return ; }

	it = fieldList.find( cmdfield ) ;

	if ( f_pos < 1 || f_pos > it->second->field_length ) { f_pos = 1 ; }

	p_col = it->second->field_col + f_pos - 1 ;
	p_row = it->second->field_row ;
}


void pPanel::get_home( uint& row, uint& col )
{
	// Return the physical home position on the screen

	map<string, field*>::iterator it ;

	it = fieldList.find( home ) ;
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


void pPanel::set_cursor_home()
{
	// Set the cursor to the relative home position

	map<string, field*>::iterator it ;

	dCursor = home ;
	dCsrpos = 1    ;

	it = fieldList.find( home ) ;
	if ( it == fieldList.end() )
	{
		p_row = 0 ;
		p_col = 0 ;
	}
	else
	{
		p_row = it->second->field_row ;
		p_col = it->second->field_col ;
	}
}


field* pPanel::get_field_address( uint row, uint col )
{
	return fieldAddrs[ fieldMap[ row * zscrmaxw + col ] ] ;
}


const string& pPanel::field_getvalue( const string& f_name )
{
	map<string, field*>::iterator it = fieldList.find( f_name ) ;

	it->second->field_prep_input() ;
	return it->second->field_value ;
}


const string& pPanel::field_getrawvalue( const string& f_name )
{
	map<string, field*>::iterator it = fieldList.find( f_name ) ;

	return it->second->field_value ;
}


void pPanel::field_setvalue( const string& f_name, const string& f_value )
{
	map<string, field*>::iterator it = fieldList.find( f_name ) ;

	it->second->field_value   = f_value ;
	it->second->field_changed = true ;
	it->second->field_prep_display() ;
	it->second->display_field( win, inv_schar, ddata_map, schar_map ) ;
}


const string& pPanel::cmd_getvalue()
{
	if ( cmdfield == "" )
	{
		return zzcmd ;
	}
	return field_getvalue( cmdfield ) ;
}


void pPanel::cmd_setvalue( const string& v )
{
	if ( cmdfield == "" )
	{
		zzcmd = v ;
	}
	else
	{
		field_setvalue( cmdfield, v ) ;
	}
}


bool pPanel::cmd_nonblank()
{
	return ( strip( cmd_getvalue() ) != "" ) ;
}


bool pPanel::keep_cmd()
{
	// Return true to keep the command line and/or displayed message.
	// Also set error_msg to false so old errors do not stop the command stack from executing.

	// Errors from primary command - always clear
	// else command line blank - keep
	//      command line non-blank - clear

	map<string, field*>::iterator it ;

	if ( error_msg )
	{
		error_msg = false ;
		if ( get_cursor() == cmdfield )
		{
			return false ;
		}
	}

	if ( cmdfield == "" )
	{
		return true ;
	}

	it = fieldList.find( cmdfield ) ;

	it->second->field_prep_input() ;
	if ( it->second->field_value.size() == 0 )
	{
		return true ;
	}

	it->second->field_set_caps() ;
	it->second->display_field( win, inv_schar, ddata_map, schar_map ) ;

	return ( it->second->field_value.front() == '&' ) ;
}


void pPanel::reset_cmd()
{
	errblock err ;

	if ( cmdfield != "" )
	{
		field_setvalue( cmdfield, getDialogueVar( err, cmdfield ) ) ;
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
		if ( lrScroll ) { return false ; }
		if ( tb_model ) { return true  ; }
		if ( scrollOn ) { return false ; }
		return true ;
	}
	else if ( findword( cmd, "UP DOWN" ) )
	{
		if ( not AreaList.empty() || scrollOn ) { return false ; }
		return true ;
	}
	else if ( cmd == "NRETRIEV" && !nretriev ) { return true ; }
	else if ( cmd == "RFIND"    && ( !forEdit && !forBrowse ) ) { return true ; }
	else if ( cmd == "RCHANGE"  && !forEdit   ) { return true ; }

	return false ;
}


bool pPanel::field_valid( const string& f_name )
{
	return ( fieldList.find( f_name ) != fieldList.end() ) ;
}


string pPanel::field_getname( uint row, uint col )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field

	map<string, field*>::iterator it ;

	if ( row < win_row || col < win_col ) { return "" ; }

	row -= win_row ;
	col -= win_col ;

	field* fld = get_field_address( row, col ) ;
	if ( fld && fld->field_active )
	{
		for ( it = fieldList.begin() ; it != fieldList.end() ; ++it )
		{
			if ( fld == it->second )
			{
				return it->first ;
			}
		}
	}
	return "" ;
}


bool pPanel::field_get_row_col( const string& fld, uint& row, uint& col )
{
	// If field found on panel (by name), return true and its position, else return false.
	// Return the physical position on the screen, so add the window offsets to field_row/col

	auto it = fieldList.find( fld ) ;
	if ( it == fieldList.end() || !it->second->field_active ) { return false ; }

	row = it->second->field_row + win_row ;
	col = it->second->field_col + win_col ;

	return true ;
}


int pPanel::field_get_col( const string& fld )
{
	// If field found on panel (by name), return column position.
	// Return the physical position on the screen, so add the window offset to field_col

	auto it = fieldList.find( fld ) ;
	if ( it != fieldList.end() && it->second->field_active )
	{
		return it->second->field_col + win_col ;
	}
	return 0 ;
}


bool pPanel::field_nonblank( const string& field, uint p )
{
	// Return true if there is a nonblank character at position p in the field value.

	if ( p == 0 ) { return true ; }

	auto it = fieldList.find( field ) ;
	if ( it->second->field_value.size() > p )
	{
		return ( it->second->field_value.compare( p, 1, " " ) > 0 ) ;
	}
	return false ;
}


fieldExc pPanel::field_getexec( const string& field )
{
	// If passed field is in the field execute table, return the structure fieldExc
	// for that field as defined in )FIELD panel section.

	if ( fieldExcTable.find( field ) == fieldExcTable.end() ) { return fieldExc() ; }
	return fieldExcTable[ field ] ;
}


void pPanel::field_edit( uint row,
			 uint col,
			 char ch,
			 bool Isrt,
			 bool& prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	// field_tab_next also needs the physical position, so addjust before and after the call.

	prot = true ;

	if ( row < win_row || col < win_col )
	{
		p_row = row - win_row ;
		p_col = col - win_col ;
		return ;
	}

	row  -= win_row ;
	col  -= win_col ;
	p_row = row ;
	p_col = col ;

	field* fld = get_field_address( row, col ) ;
	if ( fld && fld->field_active )
	{
		if ( (  fld->field_numeric && ch != ' ' && !isdigit( ch ) ) ||
		     ( !fld->field_dynArea && !fld->field_input )           ||
		     (  fld->field_dynArea && !fld->field_dyna_input( col ) ) ) { return ; }
		if ( Isrt )
		{
			if ( !fld->edit_field_insert( win, ch, inv_schar, col, ddata_map, schar_map ) )
			{
				return ;
			}
		}
		else
		{
			if ( !fld->edit_field_replace( win, ch, inv_schar, col, ddata_map, schar_map ) )
			{
				return ;
			}
		}
		prot = false ;
		++p_col ;
		if ( fld->field_skip && p_col == ( fld->field_endcol + 1 ) )
		{
			p_row += win_row ;
			p_col += win_col ;
			field_tab_next( p_row, p_col ) ;
			p_row -= win_row ;
			p_col -= win_col ;
		}
	}
}


void pPanel::field_backspace( uint& row,
			      uint& col,
			      bool& prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field
	// Return physical position of the cursor in row/col

	uint trow ;
	uint tcol ;
	uint p    ;

	prot = true ;

	if ( row < win_row || col < win_col ) { return ; }

	trow = row - win_row ;
	tcol = col - win_col ;

	field* fld = get_field_address( trow, tcol ) ;
	if ( fld && fld->field_active && tcol != fld->field_col )
	{
		if ( ( !fld->field_dynArea && !fld->field_input ) ||
		     (  fld->field_dynArea && !fld->field_dyna_input( tcol ) ) ) { return ; }
		p    = tcol ;
		tcol = fld->edit_field_backspace( win, tcol, inv_schar, ddata_map, schar_map ) ;
		if ( p != tcol )
		{
			prot = false ;
			row  = trow + win_row ;
			col  = tcol + win_col ;
		}
	}
}


void pPanel::field_delete_char( uint row, uint col, bool& prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field
	// Return physical position of the cursor in row/col

	uint trow ;
	uint tcol ;

	prot = true ;

	if ( row < win_row || col < win_col ) { return ; }

	trow = row - win_row ;
	tcol = col - win_col ;

	field* fld = get_field_address( trow, tcol ) ;
	if ( fld && fld->field_active )
	{
		if ( ( !fld->field_dynArea && !fld->field_input ) ||
		     (  fld->field_dynArea && !fld->field_dyna_input( tcol ) ) ) { return ; }
		fld->edit_field_delete( win, tcol, inv_schar, ddata_map, schar_map ) ;
		prot = false ;
	}
}


void pPanel::field_erase_eof( uint row, uint col, bool& prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field

	prot = true ;

	if ( row < win_row || col < win_col ) { return ; }

	row -= win_row ;
	col -= win_col ;

	field* fld = get_field_address( row, col ) ;
	if ( fld && fld->field_active )
	{
		if ( ( !fld->field_dynArea && !fld->field_input ) ||
		     (  fld->field_dynArea && !fld->field_dyna_input( col ) ) ) { return ; }
		fld->field_erase_eof( win, col, inv_schar, ddata_map, schar_map ) ;
		prot = false ;
	}
}


void pPanel::cursor_eof( uint& row, uint& col )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field
	// Return physical position of the cursor in row/col

	uint trow ;
	uint tcol ;

	if ( row < win_row || col < win_col ) { return ; }

	trow = row - win_row ;
	tcol = col - win_col ;

	field* fld = get_field_address( trow, tcol ) ;
	if ( fld && fld->field_active )
	{
		if ( ( !fld->field_dynArea && !fld->field_input ) ||
		     (  fld->field_dynArea && !fld->field_dyna_input( tcol ) ) ) { return ; }
		col = fld->end_of_field( win, tcol ) + win_col ;
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

	uint o_row   ;

	uint trow ;
	uint tcol ;

	trow = row - win_row ;
	tcol = col - win_col ;

	bool cursor_moved(false) ;
	map<string, field*>::iterator it;

	c_offset = trow * wscrmaxw + tcol ;
	m_offset = wscrmaxd * wscrmaxw    ;
	o_row    = trow                   ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; ++it )
	{
		if ( !it->second->field_active) { continue ; }
		if ( !it->second->field_dynArea && !it->second->field_input ) { continue ; }
		if (  it->second->field_row <= o_row ) { continue ; }
		d_offset = 0 ;
		if ( it->second->field_dynArea )
		{
			if ( !it->second->field_dynArea->dynArea_dataInsp )
			{
				continue ;
			}
			d_offset = it->second->field_dyna_input_offset( 0 ) ;
			if ( d_offset == -1 ) { continue ; }
		}
		t_offset = it->second->field_row * wscrmaxw + it->second->field_col + d_offset ;
		if ( t_offset > c_offset && t_offset < m_offset )
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
	int  d_offset ;

	uint o_row ;
	uint o_col ;
	uint trow  ;
	uint tcol  ;

	trow = row - win_row ;
	tcol = col - win_col ;

	bool cursor_moved(false) ;
	map<string, field*>::iterator it;

	c_offset = trow * wscrmaxw + tcol ;
	m_offset = wscrmaxd * wscrmaxw    ;
	o_row    = trow                   ;
	o_col    = tcol                   ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; ++it )
	{
		if ( !it->second->field_active) { continue ; }
		if ( !it->second->field_dynArea && !it->second->field_input ) { continue ; }
		d_offset = 0 ;
		if ( it->second->field_dynArea )
		{
			if ( !it->second->field_dynArea->dynArea_dataInsp )
			{
				continue ;
			}
			if ( o_row == it->second->field_row )
			{
				d_offset = it->second->field_dyna_input_offset( o_col ) ;
			}
			else
			{
				d_offset = it->second->field_dyna_input_offset( 0 ) ;
			}
			if ( d_offset == -1 ) { continue ; }
		}
		t_offset = it->second->field_row * wscrmaxw + it->second->field_col + d_offset ;
		if ( t_offset > c_offset && t_offset < m_offset )
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


void pPanel::tb_set_linesChanged( errblock& err )
{
	// Store changed lines for processing by the application if requested via tbdispl with no panel name
	// Format is a list of line-number/URID pairs

	int idr ;
	string URID ;

	map<string, field*>::iterator it ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; ++it )
	{
		if ( it->second->field_tb && it->second->field_changed )
		{
			it->second->field_changed = false ;
			idr = it->second->field_row - tb_toprow ;
			if ( tb_linesChanged.count( idr ) == 0 )
			{
				URID = p_funcPOOL->get( err, 0, ".ZURID."+ d2ds( idr ), NOCHECK ) ;
				if ( err.error() ) { return ; }
				tb_linesChanged[ idr ] = URID ;
			}
		}
	}
	p_funcPOOL->put2( err, "ZTDSELS", tb_linesChanged.size() ) ;
}


void pPanel::tb_add_autosel_line( errblock& err )
{
	// Add auto-selected line to list of changed lines

	int idr ;

	string URID ;

	err.setRC( 0 ) ;

	if ( tb_autosel && tb_csrrow > 0 )
	{
		idr = tb_csrrow - p_funcPOOL->get( err, 0, INTEGER, "ZTDTOP", NOCHECK ) ;
		if ( err.error() ) { return ; }
		if ( idr >= 0 && idr < tb_depth && tb_linesChanged.count( idr ) == 0 )
		{
			URID = p_funcPOOL->get( err, 0, ".ZURID."+ d2ds( idr ), NOCHECK ) ;
			if ( err.error() ) { return ; }
			if ( URID != "" )
			{
				tb_linesChanged[ idr ] = URID ;
				p_funcPOOL->put2( err, "ZTDSELS", tb_linesChanged.size() ) ;
			}
		}
	}
}


bool pPanel::tb_get_lineChanged( errblock& err, int& ln, string& URID )
{
	// Retrieve the next changed line on the tbdispl.  Return screen line number and URID of the table record
	// Don't remove the pair from the list but update ZTDSELS

	map<int, string>::iterator it ;

	ln   = 0  ;
	URID = "" ;

	err.setRC( 0 ) ;

	if ( tb_linesChanged.size() == 0 ) { return false ; }

	it   = tb_linesChanged.begin() ;
	ln   = it->first  ;
	URID = it->second ;

	p_funcPOOL->put2( err, "ZTDSELS", tb_linesChanged.size() ) ;
	if ( err.error() ) { return false ; }
	return true ;
}


bool pPanel::tb_linesPending()
{
	return ( tb_linesChanged.size() > 0 ) ;
}


void pPanel::tb_clear_linesChanged( errblock& err )
{
	// Clear all stored changed lines on a tbdispl with panel name and set ZTDSELS to zero

	tb_linesChanged.clear() ;
	p_funcPOOL->put2( err, "ZTDSELS", 0 ) ;
}


void pPanel::tb_remove_lineChanged()
{
	// Remove the processed line from the list of changed lines

	if ( !tb_linesChanged.empty() )
	{
		tb_linesChanged.erase( tb_linesChanged.begin() ) ;
	}
}


void pPanel::display_tb_mark_posn( errblock& err )
{
	int rows ;
	int top  ;
	int size ;

	string mark ;
	string posn ;

	rows = p_funcPOOL->get( err, 0, INTEGER, "ZTDROWS", NOCHECK ) ;
	if ( err.error() ) { return ; }

	top  = p_funcPOOL->get( err, 0, INTEGER, "ZTDTOP", NOCHECK ) ;
	if ( err.error() ) { return ; }

	size = rows - top + 1 ;

	wattrset( win, cuaAttr[ SI ] | panel_intens ) ;
	if ( size < tb_depth )
	{
		mark = p_funcPOOL->get( err, 8, "ZTDMARK", NOCHECK ) ;
		if ( err.error() ) { return ; }
		if ( err.RC8() )
		{
			mark = p_poolMGR->get( err, "ZTDMARK" ) ;
			if ( err.error() ) { return ; }
			if ( err.RC8() )
			{
				mark = centre( " Bottom of Data ", wscrmaxw, '*' ) ;
			}
		}
		if ( mark.size() > wscrmaxw )
		{
			mark.resize( wscrmaxw ) ;
		}
		mvwaddstr( win, tb_toprow + size, 0, mark.c_str() ) ;
		p_funcPOOL->put2( err, "ZTDVROWS", size ) ;
		if ( err.error() ) { return ; }
	}
	else
	{
		p_funcPOOL->put2( err, "ZTDVROWS", tb_depth ) ;
		if ( err.error() ) { return ; }
	}

	posn = "" ;
	if ( top <= rows )
	{
		posn = "Row "+ d2ds( top ) +" of "+ d2ds( rows ) ;
	}
	mvwaddstr( win, ( ab.size() > 0 ) ? 2 : 0, wscrmaxw - posn.length(), posn.c_str() ) ;
}


void pPanel::set_tb_fields_act_inact( errblock& err )
{
	int rows ;
	int top  ;
	int size ;
	int i    ;

	string suf ;

	set<string>::iterator it ;

	rows = p_funcPOOL->get( err, 0, INTEGER, "ZTDROWS", NOCHECK ) ;
	if ( err.error() ) { return ; }
	top  = p_funcPOOL->get( err, 0, INTEGER, "ZTDTOP", NOCHECK ) ;
	if ( err.error() ) { return ; }

	size = rows - top + 1 ;

	for ( i = 0 ; i < tb_depth ; ++i )
	{
		suf = "." + d2ds( i ) ;
		for ( it = tb_fields.begin() ; it != tb_fields.end() ; ++it )
		{
			fieldList[ *it + suf ]->field_active = ( i < size ) ;
		}
	}
}


void pPanel::rebuild_after_area_scroll( Area* a )
{
	// Update the fieldList, textList, fieldMap and fieldAddrs after
	// an AREA scroll

	a->update_area() ;

	build_fieldMap() ;
}


void pPanel::get_pd_home( uint& row, uint& col )
{
	// Return the physical home position of a pull-down

	row = win_row + 2 ;
	col = win_col + 2 + ab[ abIndex ]->get_pd_col() ;
}


bool pPanel::cursor_on_pulldown( uint row, uint col )
{
	// row/col is the pysical position on the screen.

	return ab[ abIndex ]->cursor_on_pulldown( row, col ) ;
}


bool pPanel::display_pd( errblock& err, uint row, uint col, string& msg )
{
	// row/col is the pysical position on the screen.  Correct by subtracting the window column position

	// Call relevant )ABCINIT and display the pull-down at the cursor position.

	uint i ;

	vector<abc*>::iterator it ;

	row -= win_row ;
	col -= win_col ;
	hide_pd() ;

	err.setRC( 0 ) ;

	for ( i = 0, it = ab.begin() ; it != ab.end() ; ++it, ++i )
	{
		if ( col >= (*it)->abc_col && col < ( (*it)->abc_col + (*it)->abc_desc.size() ) )
		{
			abIndex = i ;
			clear_msg() ;
			abc_panel_init( err, (*it)->get_abc_desc() ) ;
			if ( err.error() )
			{
				pdActive = false ;
				return false ;
			}
			msg = dMsg ;
			(*it)->display_abc_sel( win ) ;
			(*it)->display_pd( err, dZvars, win_row, win_col, row ) ;
			pdActive = true ;
			p_col    = (*it)->abc_col + 2 ;
			p_row    = 2 ;
			return true  ;
		}
	}
	return false ;
}


void pPanel::display_pd( errblock& err )
{
	if ( !pdActive ) { return ; }

	auto it = ab.begin() + abIndex ;

	(*it)->display_abc_sel( win ) ;
	(*it)->display_pd( err, dZvars, win_row, win_col, 0 ) ;
}


void pPanel::display_current_pd( errblock& err, uint row, uint col )
{
	// row/col is the pysical position on the screen.  Correct by subtracting the window column position

	// Re-display the pull-down if the cursor is placed on it (to update current choice and hilite)

	auto it = ab.begin() + abIndex ;

	row -= win_row ;
	col -= win_col ;

	if ( col >= (*it)->abc_col && col < ( (*it)->abc_col + (*it)->abc_maxw + 10 ) )
	{
		(*it)->display_pd( err, dZvars, win_row, win_col, row ) ;
	}
}


void pPanel::display_next_pd( errblock& err, const string& mnemonic, string& msg )
{
	uint i ;

	vector<abc*>::iterator it ;

	err.setRC( 0 ) ;

	if ( ab.size() == 0 ) { return ; }

	hide_pd() ;
	if ( !pdActive )
	{
		abIndex = 0 ;
		if ( mnemonic.size() == 1 )
		{
			for ( i = 0, it = ab.begin() ; it != ab.end() ; ++it, ++i )
			{
				if ( (*it)->abc_mnem2 == mnemonic.front() )
				{
					abIndex = i ;
					break ;
				}
			}
		}
	}
	else if ( ++abIndex == ab.size() )
	{
		abIndex = 0 ;
	}

	it = ab.begin() + abIndex ;

	clear_msg() ;

	pdActive = true ;

	abc_panel_init( err, (*it)->get_abc_desc() ) ;
	if ( err.error() )
	{
		pdActive = false ;
		return ;
	}

	msg = dMsg ;
	(*it)->display_abc_sel( win ) ;
	(*it)->display_pd( err, dZvars, win_row, win_col, 2 ) ;
	p_col = (*it)->abc_col + 2 ;
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

	auto it = ab.begin() + abIndex ;

	(*it)->hide_pd() ;
	(*it)->display_abc_unsel( win ) ;
}


void pPanel::display_area_si()
{
	// Display the area scroll indicator.

	uint row ;
	uint col ;
	uint width ;

	for ( auto it = AreaList.begin() ; it != AreaList.end() ; ++it )
	{
		it->second->get_info( row, col, width ) ;
		wattrset( win, cuaAttr[ SI ] | panel_intens ) ;
		mvwaddstr( win, row, col+width-12, it->second->get_scroll_indicator() ) ;
	}
}


bool pPanel::jump_field( uint row, uint col, string& fvalue )
{
	// Return true if cursor is on a jump field and a jump command has been entered.
	// Also return field value up to the cursor position or first space (remove nulls and leading spaces)
	// Replace field value with function pool value.

	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.

	size_t p1 ;
	size_t p2 ;

	errblock err ;

	const char nulls = 0x00 ;

	if ( jumpList.size() == 0 ) { return false ; }

	row -= win_row ;
	col -= win_col ;

	for ( auto it = jumpList.begin() ; it != jumpList.end() ; ++it )
	{
		if ( it->second->cursor_on_field( row, col ) )
		{
			fvalue = it->second->field_value.substr( 0, col - it->second->field_col ) ;
			p1 = 0 ;
			while ( true )
			{
				p1 = fvalue.find( nulls, p1 ) ;
				if ( p1 == string::npos ) { break ; }
				p2 = fvalue.find_first_not_of( nulls, p1 ) ;
				if ( p2 == string::npos )
				{
					fvalue.erase( p1 ) ;
					break ;
				}
				else
				{
					fvalue.erase( p1, p2-p1 ) ;
				}
			}
			fvalue = word( fvalue, 1 ) ;
			if ( fvalue.size() > 1 && fvalue.front() == '=' )
			{
				it->second->field_value = getDialogueVar( err, it->first ) ;
				return true ;
			}
			return false ;
		}
	}
	return false ;
}


pdc pPanel::retrieve_choice( errblock& err, string& msg )
{
	auto it = ab.begin() + abIndex ;
	pdc t_pdc = (*it)->retrieve_choice( err ) ;

	if ( !t_pdc.pdc_inact )
	{
		abc_panel_proc( err, (*it)->get_abc_desc() ) ;
		msg = dMsg ;
	}

	return t_pdc ;
}


void pPanel::display_boxes()
{
	for ( auto it = boxes.begin() ; it != boxes.end() ; ++it )
	{
		(*it)->display_box( win, sub_vars( (*it)->box_title ) ) ;
	}
}


void pPanel::set_panel_msg( const slmsg& t, const string& m )
{
	errblock err ;

	clear_msg() ;
	MSG  = t ;
	dMsg = m ;

	if ( MSG.smsg == "" && MSG.lmsg == "" )
	{
		dMsg = "" ;
	}
	else
	{
		showLMSG = ( MSG.smsg == "" || MSG.lmsg == "" ) ;
	}
}


void pPanel::clear_msg()
{
	MSG.clear() ;

	dMsg      = "" ;
	showLMSG  = false ;
	error_msg = false ;

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
	if ( keylistn == "" || Keylistl.count( entry ) == 0 ) { return "" ; }
	return Keylistl[ entry ] ;
}


void pPanel::display_msg( errblock& err )
{
	uint i ;

	uint w_row ;
	uint w_col ;
	uint w_depth ;
	uint w_width ;
	uint f_colour1 = 0 ;

	uint m_row = 0 ;
	uint m_col = 0 ;

	string t ;
	string msg = get_msg() ;

	bool inWindow1 = false ;
	bool inWindow2 = false ;

	vector<string> v ;

	if ( msg == "" ) { return ; }

	msgResp = MSG.resp ;
	if ( msg != "PSYS012L" )
	{
		p_poolMGR->put( err, zscrnum, "ZMSGID", msg ) ;
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
		inWindow1 = ( MSG.smwin || pd_active() ) ;
		if ( not inWindow1 && pos_smsg != "" )
		{
			auto it = fieldList.find( pos_smsg ) ;
			if ( MSG.smsg.size() > it->second->field_length )
			{
				inWindow1 = true ;
			}
			else
			{
				m_row     = it->second->field_row + win_row ;
				m_col     = it->second->field_col + win_col ;
				f_colour1 = it->second->field_get_colour1() ;
			}
		}
		if ( inWindow1 )
		{
			get_msgwin( err, MSG.smsg, w_row, w_col, w_depth, w_width, v ) ;
			if ( err.error() ) { return ; }
			smwin = newwin( w_depth, w_width, w_row+win_row, w_col+win_col ) ;
			wattrset( smwin, cuaAttr[ MSG.type ] | panel_intens ) ;
			box( smwin, 0, 0 ) ;
			for ( i = 0 ; i < v.size() ; ++i )
			{
				mvwaddstr( smwin, i+1, 2, v[i].c_str() ) ;
			}
		}
		else
		{
			wstandend( win ) ;
			t = MSG.smsg ;
			if ( pos_smsg == "" )
			{
				if ( t.size() < 24 ) { t = right( t, 24 ) ; }
				m_row = ( ab.size() > 0 ) ? 2 : 0 ;
				m_row = win_row + m_row ;
				m_col = win_col + wscrmaxw - t.size() - 1 ;
				smwin = newwin( 1, t.size()+1, m_row, m_col) ;
				wattrset( smwin, cuaAttr[ MSG.type ] | panel_intens ) ;
				mvwaddstr( smwin, 0, 1, t.c_str() ) ;
			}
			else
			{
				smwin = newwin( 1, t.size(), m_row, m_col) ;
				wattrset( smwin, f_colour1 | panel_intens ) ;
				mvwaddstr( smwin, 0, 0, t.c_str() ) ;
			}
		}
		smpanel = new_panel( smwin )  ;
		set_panel_userptr( smpanel, new panel_data( zscrnum ) ) ;
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
		inWindow2 = ( MSG.lmwin || p_poolMGR->get( err, "ZLMSGW", PROFILE ) == "Y" || pd_active() ) ;
		if ( not inWindow2 && pos_lmsg != "" )
		{
			auto it = fieldList.find( pos_lmsg ) ;
			if ( MSG.lmsg.size() > it->second->field_length )
			{
				inWindow2 = true ;
			}
			else
			{
				m_row     = it->second->field_row + win_row ;
				m_col     = it->second->field_col + win_col ;
				f_colour1 = it->second->field_get_colour1() ;
			}
		}
		if ( inWindow2 )
		{
			if ( inWindow1 )
			{
				t = ( MSG.smsg != "" ) ? MSG.smsg +" - "+ MSG.lmsg : MSG.lmsg ;
				if ( smwin )
				{
					panel_cleanup( smpanel ) ;
					del_panel( smpanel ) ;
					delwin( smwin ) ;
					smwin = NULL ;
				}
			}
			else
			{
				t = MSG.lmsg ;
			}
			if ( p_poolMGR->get( err, zscrnum, "ZSHMSGID" ) == "Y" )
			{
				t = msg + " " + t ;
			}
			get_msgwin( err, t, w_row, w_col, w_depth, w_width, v ) ;
			if ( err.error() ) { return ; }
			lmwin = newwin( w_depth, w_width, w_row+win_row, w_col+win_col ) ;
			wattrset( lmwin, cuaAttr[ MSG.type ] | panel_intens ) ;
			box( lmwin, 0, 0 ) ;
			for ( i = 0 ; i < v.size() ; ++i )
			{
				mvwaddstr( lmwin, i+1, 2, v[ i ].c_str() ) ;
			}
		}
		else
		{
			t = MSG.lmsg ;
			if ( p_poolMGR->get( err, zscrnum, "ZSHMSGID" ) == "Y" )
			{
				t = msg + " " + t ;
			}
			if ( pos_lmsg == "" )
			{
				auto it = fieldList.find( cmdfield ) ;
				if ( it != fieldList.end() )
				{
					m_row = win_row + it->second->field_get_row() + 1 ;
					m_col = win_col + 1 ;
				}
				else
				{
					m_row = win_row + 4 ;
					m_col = win_col + 1 ;
				}
				f_colour1 = cuaAttr[ MSG.type ] ;
			}
			lmwin = newwin( 1, min( t.size(), size_t( wscrmaxw-1) ), m_row, m_col ) ;
			wattrset( lmwin, f_colour1 | panel_intens ) ;
			mvwaddstr( lmwin, 0, 0, t.c_str() ) ;
		}
		lmpanel = new_panel( lmwin ) ;
		set_panel_userptr( lmpanel, new panel_data( zscrnum ) ) ;
	}
}


void pPanel::get_msgwin( errblock& err,
			 string m,
			 uint& t_row,
			 uint& t_col,
			 uint& t_depth,
			 uint& t_width,
			 vector<string>& v )
{
	// Split message into separate lines if necessary and put into vector v.
	// Calculate the message window position and size.

	uint w  ;
	uint mw ;
	uint h  ;
	uint t  ;

	size_t p ;

	string loc = get_msgloc() ;

	auto it = fieldList.find( loc ) ;

	if ( it == fieldList.end() )
	{
		if ( loc != "" )
		{
			err.seterrid( "PSYE021D", loc ) ;
			return ;
		}
		h = ( m.size() / wscrmaxw ) + 1 ;
		w = ( m.size() / h )            ;
	}
	else if ( ( it->second->field_col + m.size() + 2 ) > wscrmaxw )
	{
		t = wscrmaxw - it->second->field_col - 2 ;
		if ( t < 30 )
		{
			t = 3 * sqrt( m.size() ) ;
			if ( t < 30 ) { t = 30 ; }
		}
		h = ( m.size() / t ) + 1 ;
		w = m.size() / h  ;
	}
	else
	{
		w = m.size() ;
	}

	if ( w > wscrmaxw - 6 ) { w = wscrmaxw - 6     ; }
	if ( w > wscrmaxw / 2 ) { w = wscrmaxw * 2 / 3 ; }

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
		ab.at( abIndex )->get_msg_position( t_row, t_col ) ;
		++t_row ;
		if ( (t_row + t_depth + win_row ) > zscrmaxd )
		{
			t_row = zscrmaxd - win_row - t_depth ;
		}
	}
	else if ( it != fieldList.end() )
	{
		t_row = it->second->field_row + 1 ;
		t_col = it->second->field_col > 2 ? it->second->field_col - 2 : 0 ;
		if ( (t_row + t_depth) > wscrmaxd )
		{
			t_row = wscrmaxd - t_depth ;
			t_col = t_col + it->second->field_length + 2 ;
		}
		if ( (t_col + t_width) > wscrmaxw )
		{
			t_col = wscrmaxw - t_width ;
		}
	}
	else if ( win == pwin )
	{
		t_row = win_depth + 1 ;
		t_col = -1 ;
		if ( (t_row + t_depth + win_row ) > zscrmaxd )
		{
			t_row = zscrmaxd - win_row - t_depth ;
		}
	}
	else
	{
		t_row =  wscrmaxd - t_depth      ;
		t_col = (wscrmaxw - t_width) / 2 ;
	}
}


void pPanel::display_id( errblock& err )
{
	string scrname ;
	string panarea = "" ;

	if ( idwin )
	{
		panel_cleanup( idpanel ) ;
		del_panel( idpanel ) ;
		delwin( idwin ) ;
		idwin = NULL ;
	}

	if ( p_poolMGR->get( err, zscrnum, "ZSHUSRID" ) == "Y" )
	{
		panarea = p_poolMGR->get( err, "ZUSER", SHARED ) + " " ;
	}

	if ( p_poolMGR->get( err, zscrnum, "ZSHPANID" ) == "Y" )
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
		idwin   = newwin( 1, panarea.size(), ( ab.size() > 0 ) ? win_row+2 : win_row, win_col+1 ) ;
		idpanel = new_panel( idwin )  ;
		set_panel_userptr( idpanel, new panel_data( zscrnum ) ) ;
		wattrset( idwin, cuaAttr[ PI ] | panel_intens ) ;
		mvwaddstr( idwin, 0, 0, panarea.c_str() ) ;
		top_panel( idpanel ) ;
	}
}


void pPanel::get_panel_info( errblock& err,
			     const string& a_name,
			     const string& t_name,
			     const string& w_name,
			     const string& d_name,
			     const string& r_name,
			     const string& c_name )
{
	// RC =  0  Normal completion
	// RC =  8  Area type not found on panel
	// RC = 20  Severe error

	// Only AREATYPE(DYNAMIC) currently supported

	map<string, dynArea*>::iterator it ;

	it = dynAreaList.find( a_name ) ;
	if ( it == dynAreaList.end() )
	{
		err.setRC( 8 ) ;
		return ;
	}

	if ( t_name != "" )
	{
		p_funcPOOL->put2( err, t_name, "DYNAMIC" ) ;
		if ( err.error() ) { return ; }
	}

	if ( w_name != "" )
	{
		p_funcPOOL->put2( err, w_name, it->second->dynArea_width ) ;
		if ( err.error() ) { return ; }
	}

	if ( d_name != "" )
	{
		p_funcPOOL->put2( err, d_name, it->second->dynArea_depth ) ;
		if ( err.error() ) { return ; }
	}

	if ( r_name != "" )
	{
		p_funcPOOL->put2( err, r_name, it->second->dynArea_row ) ;
		if ( err.error() ) { return ; }
	}

	if ( c_name != "" )
	{
		p_funcPOOL->put2( err, c_name, it->second->dynArea_col ) ;
		if ( err.error() ) { return ; }
	}
}


void pPanel::attr( errblock& err, const string& field, const string& attrs )
{
	err.setRC( 0 ) ;

	auto it = fieldList.find( field ) ;

	if ( it == fieldList.end() )
	{
		err.seterrid( "PSYE031M", field ) ;
		return ;
	}
	it->second->field_attr( err, attrs ) ;
}


void pPanel::attrchar( errblock& err, const char ch, string& attrs )
{
	err.setRC( 0 ) ;

	char_attrs attrchar ;

	auto it = char_attrlist.find( ch ) ;

	if ( it == char_attrlist.end() )
	{
		err.seterrid( "PSYE031N", isprint( ch ) ? string( 1, ch ) : cs2xs( ch ) ) ;
		return ;
	}
	attrchar.update( err, attrs ) ;
}


void pPanel::panel_cleanup( PANEL* p )
{
	const void* vptr = panel_userptr( p ) ;

	if ( vptr )
	{
		delete static_cast<const panel_data*>(vptr) ;
	}
}


void pPanel::set_panel_fscreen()
{
	const void* vptr = panel_userptr( panel ) ;

	if ( vptr )
	{
		const panel_data* pd = static_cast<const panel_data*>(vptr) ;
		pd->set_fscreen() ;
	}
}


void pPanel::unset_panel_fscreen()
{
	const void* vptr = panel_userptr( panel ) ;

	if ( vptr )
	{
		const panel_data* pd = static_cast<const panel_data*>(vptr) ;
		pd->unset_fscreen() ;
	}
}


void pPanel::unset_panel_decolourised()
{
	const void* vptr = panel_userptr( panel ) ;

	if ( vptr )
	{
		const panel_data* pd = static_cast<const panel_data*>(vptr) ;
		pd->unset_decolourised() ;
	}
}


bool pPanel::is_panel_decolourised()
{
	const void* vptr = panel_userptr( panel ) ;

	if ( vptr )
	{
		const panel_data* pd = static_cast<const panel_data*>(vptr) ;
		return pd->is_decolourised() ;
	}
	return false ;
}


void pPanel::set_panel_frame_act()
{
	const void* vptr = panel_userptr( bpanel ) ;

	if ( vptr )
	{
		const panel_data* pd = static_cast<const panel_data*>(vptr) ;
		pd->set_frame_act() ;
	}
}


bool pPanel::is_panel_frame_inact()
{
	const void* vptr = panel_userptr( bpanel ) ;

	if ( vptr )
	{
		const panel_data* pd = static_cast<const panel_data*>(vptr) ;
		return pd->is_frame_inact() ;
	}
	return true ;
}


void pPanel::set_panel_ttl( const string& winttl )
{
	const void* vptr = panel_userptr( bpanel ) ;
	if ( vptr )
	{
		const panel_data* pd = static_cast<const panel_data*>(vptr) ;
		pd->set_winttl( winttl ) ;
	}
}


const string pPanel::get_panel_ttl( PANEL* p )
{
	const void* vptr = panel_userptr( p ) ;
	if ( vptr )
	{
		const panel_data* pd = static_cast<const panel_data*>(vptr) ;
		return pd->get_winttl() ;
	}
	return "" ;
}


bool pPanel::on_border_line( uint r, uint c )
{
	// Return true if the cursor is on the window border (to start a window move)

	if ( !win || win != pwin ) { return false ; }

	return ( ((r == win_row-1 || r == win_row+wscrmaxd) &&
		  (c >= win_col-1 && c <= win_col+wscrmaxw))  ||
		 ((c == win_col-1 || c == win_col+wscrmaxw) &&
		  (r >= win_row-1 && r <= win_row+wscrmaxd)) ) ;
}


void pPanel::update_keylist_vars( errblock& err )
{
	p_poolMGR->put( err, "ZKLNAME", keylistn, SHARED, SYSTEM ) ;
	p_poolMGR->put( err, "ZKLAPPL", keyappl,  SHARED, SYSTEM ) ;
	p_poolMGR->put( err, "ZKLTYPE", "P",      SHARED, SYSTEM ) ;
}


bool pPanel::hide_msg_window( uint r, uint c )
{
	// If the cursor is on a message window, hide the window and return true
	// The underlying panel is not submitted for processing in this case.

	int ht  ;
	int len ;

	bool reslt = false ;

	uint w_row ;
	uint w_col ;

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
				reslt = true ;
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
				reslt = true ;
			}
		}
	}

	return reslt ;
}


string pPanel::sub_vars( string s )
{
	// In string, s, substitute variables starting with '&' for their dialogue value
	// The variable is delimited by any non-valid variable characters.
	// Rules: A '.' at the end of a variable, concatinates the value with the string following the dot
	//        .. reduces to .
	//        && reduces to & with no variable substitution

	size_t p1 = 0 ;
	size_t p2 ;

	string var ;
	string val ;

	errblock err ;

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

	size_t p1 = 0 ;
	size_t p2 ;

	string var ;
	string val ;

	errblock err ;

	dvars = false ;
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
