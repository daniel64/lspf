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

#ifdef HAS_REXX_SUPPORT
int REXXENTRY rxiniExit_panel1( RexxExitContext*,
				int,
				int,
				PEXIT ) ;

int REXXENTRY rxterExit_panel1( RexxExitContext*,
				int,
				int,
				PEXIT ) ;

int REXXENTRY rxsioExit_panel1( RexxExitContext*,
				int,
				int,
				PEXIT ) ;
#endif


pPanel::pPanel()
{
	l_row       = 0     ;
	l_col       = 0     ;
	s_row       = 0     ;
	s_col       = 0     ;
	abc_pos     = 2     ;
	showLMSG    = false ;
	primaryMenu = false ;
	selPanel    = false ;
	scrollOn    = false ;
	pdActive    = false ;
	msgResp     = false ;
	nretriev    = false ;
	forEdit     = false ;
	forBrowse   = false ;
	bypassCur   = false ;
	redisplay   = false ;
	nretfield   = ""    ;
	keylistn    = ""    ;
	keyappl     = ""    ;
	keytype     = ""    ;
	keyshr      = false ;
	keyhelpn    = ""    ;
	pos_smsg    = ""    ;
	pos_lmsg    = ""    ;
	panelTitle  = ""    ;
	panelDesc   = ""    ;
	Area1       = ""    ;
	abIndex     = 0     ;
	da_dataIn   = ""    ;
	da_dataOut  = ""    ;
	tb_model    = false ;
	tb_fixed    = false ;
	tb_scan     = false ;
	tb_autosel  = false ;
	tb_dvars    = false ;
	tb_modlines = 0     ;
	tb_pdepth   = 0     ;
	tb_rdepth   = 0     ;
	tb_curidr   = -1    ;
	tb_curidx   = -1    ;
	tb_csrrow   = 0     ;
	tb_crp      = -1    ;
	cvd_lines   = 0     ;
	full_screen = false ;
	win_addpop  = false ;
	error_msg   = false ;
	zshowpfk    = false ;
	pfk_built   = false ;
	ex_attrchar = false ;
	win_row     = 0     ;
	win_col     = 0     ;
	win_width   = 0     ;
	win_depth   = 0     ;
	dyns_toprow = 0     ;
	dyns_depth  = 0     ;
	dyns_width  = 0     ;
	ztdtop      = 1     ;
	ztdrows     = 0     ;
	ztdvrows    = 0     ;
	table       = ""    ;
	cmdfield    = ""    ;
	home        = ""    ;
	dTrail      = ""    ;
	scroll      = ""    ;
	fieldMap    = nullptr ;
	fieldAddrs  = nullptr ;
	term_resize = false   ;
	displayed   = false   ;
	win         = nullptr ;
	fwin        = nullptr ;
	pwin        = nullptr ;
	bwin        = nullptr ;
	owin        = nullptr ;
	idwin       = nullptr ;
	smwin       = nullptr ;
	lmwin       = nullptr ;
	pfkwin      = nullptr ;
	pfklToken   = 0 ;
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
	{ ".HHELP",   CV_HHELP   },
	{ ".HIST",    CV_HIST    },
	{ ".MSG",     CV_MSG     },
	{ ".NRET",    CV_NRET    },
	{ ".PFKEY",   CV_PFKEY   },
	{ ".RESP",    CV_RESP    },
	{ ".TRUE",    CV_TRUE    },
	{ ".TRAIL",   CV_TRAIL   },
	{ ".ZVARS",   CV_ZVARS   } } ;

#ifdef HAS_REXX_SUPPORT
	instance = nullptr ;
#endif

}


pPanel::pPanel( errblock& err,
		fPOOL* x_funcPOOL,
		void* x_pAppl,
		bool x_lrScroll,
		bool x_selPanel,
		bool x_tutor,
		uint x_zscrnum,
		const string& x_zdatef ) : pPanel()
{
	err.setRC( 0 ) ;

	zscrmaxd = ds2d( p_poolMGR->get( err, "ZSCRMAXD", SHARED ) ) ;
	if ( err.getRC() > 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE015L", "GET", "ZSCRMAXD" ) ;
		return ;
	}

	zscrmaxw = ds2d( p_poolMGR->get( err, "ZSCRMAXW", SHARED ) ) ;
	if ( err.getRC() > 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE015L", "GET", "ZSCRMAXW" ) ;
		return ;
	}

	funcPool = x_funcPOOL ;
	lrScroll = x_lrScroll ;
	selPanel = x_selPanel ;
	zscrnum  = x_zscrnum ;
	zdatef   = x_zdatef ;
	pAppl    = x_pAppl ;
	tutorial = x_tutor ;

	wscrmaxd = zscrmaxd ;
	wscrmaxw = zscrmaxw ;
	taskId   = err.taskid ;
}


pPanel::~pPanel()
{
	//
	// Iterate over the 9 panel types: text, field, Area, dynArea, boxes, tbfields, action-bar-choices,
	// help entries and attributes and delete.
	//
	// Delete panel language statements in )INIT, )REINIT, )PROC, )ABCINIT and )ABCPROC sections.
	// Delete the main window/panel, popup panel and any message windows/panels created (free any userdata first).
	//

	for ( auto& f : fieldList )
	{
		delete f.second ;
	}

	for ( auto& a : AreaList )
	{
		delete a.second ;
	}

	for ( auto& d : dynAreaList )
	{
		delete d.second ;
	}

	for ( auto& h : field_help )
	{
		delete h.second ;
	}

	for ( auto& f : field_xct_table )
	{
		delete f.second ;
	}

	for ( auto& f : sfieldsList )
	{
		delete f ;
	}

	for_each( textList.begin(), textList.end(),
		[](text* a)
		{
			delete a ;
		} ) ;

	for_each( tbtextList.begin(), tbtextList.end(),
		[](text* a)
		{
			delete a ;
		} ) ;

	for_each( boxes.begin(), boxes.end(),
		[](Box* a)
		{
			delete a ;
		} ) ;

	for ( auto tbf : tbfields )
	{
		delete tbf ;
	}

	for_each( ab.begin(), ab.end(),
		[](abc* a)
		{
			delete a ;
		} ) ;

	for_each( char_attrlist.begin(), char_attrlist.end(),
		[](const pair<unsigned char, char_attrs*> attrs )
		{
			delete attrs.second ;
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
		delwin( bwin ) ;
	}
	if ( fwin )
	{
		panel_cleanup( panel ) ;
		del_panel( panel ) ;
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
		delwin( smwin ) ;
	}
	if ( lmwin )
	{
		panel_cleanup( lmpanel ) ;
		del_panel( lmpanel ) ;
		delwin( lmwin ) ;
	}
	if ( idwin )
	{
		panel_cleanup( idpanel ) ;
		del_panel( idpanel ) ;
		delwin( idwin ) ;
	}
	if ( pfkwin )
	{
		panel_cleanup( pfkpanel ) ;
		del_panel( pfkpanel ) ;
		delwin( pfkwin ) ;
	}

	delete[] fieldMap ;
	delete[] fieldAddrs ;

	update_panels() ;

#ifdef HAS_REXX_SUPPORT
	if ( instance )
	{
		instance->Terminate() ;
	}
#endif
}


void pPanel::init_control_variables()
{
	//
	// Initialise panel control variables (and related) when the display service first receives control (INIT processing only).
	//

	TRACE_FUNCTION() ;

	dAlarm  = "NO" ;
	dCursor = "" ;
	dCsrpos = 0  ;
	dHelp   = "" ;
	dHist   = "" ;
	dMsg    = "" ;
	dResp   = "" ;

	iCursor = "" ;
	iCsrpos = 1  ;
	iMsg    = "" ;

	histories.clear() ;

	msgloc      = "" ;
	end_pressed = false ;
	bypassCur   = false ;
}


void pPanel::reset_control_variables()
{
	//
	// Reset panel control variables (and related) before )PROC processing.
	//

	TRACE_FUNCTION() ;

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
			       const string& var,
			       field* pfield )
{
	//
	// Return the value of a dialogue variable (always as a string so convert int->string if necessary).
	//
	// Search order is:
	//   Function pool defined.
	//   Function pool implicit.
	//   SHARED pool.
	//   PROFILE pool.
	// Function pool variables of type 'int' are converted to 'string'.
	//
	// Selection panels do not use the function pool.  Retrieve from SHARED or PROFILE pools only.
	//
	// If the variable is not found, create a null implicit entry in the function pool.
	//
	// If the variable is fetched from the function pool pool and is associated with a
	// screen input field (non-SELECT panel), replace variable after conversion (CAPS, etc).
	//
	// If the variable is fetched from the SHARED or PROFILE pool and is associated with a
	// screen input field (non-SELECT panel), place variable in the function pool after conversion.
	//
	// RC =  0 Normal completion.
	// RC =  8 Variable not found.
	// RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	string temp ;

	string* p_str ;

	fVAR* pvar ;

	err.taskid = taskId ;

	if ( selPanel )
	{
		return p_poolMGR->get( err, var, ASIS ) ;
	}

	pvar = funcPool->getfVAR( err, var ) ;
	if ( err.error() ) { return "" ; }

	if ( pvar )
	{
		if ( pvar->integer() )
		{
			return pvar->sget( var ) ;
		}
		else
		{
			if ( pfield && pfield->field_is_input() )
			{
				temp = pvar->sget( var ) ;
				if ( temp != "" )
				{
					pvar->put( err, var, pfield->convert_value( temp ) ) ;
				}
				return temp ;
			}
			return pvar->sget( var ) ;
		}
	}
	else
	{
		p_str = p_poolMGR->vlocate( err, var ) ;
		if ( err.error() ) { return "" ; }
		switch ( err.getRC() )
		{
			case 0:
				if ( pfield && pfield->field_is_input() )
				{
					temp = *p_str ;
					if ( temp != "" )
					{
						pfield->convert_value( temp ) ;
					}
					funcPool->put2( err, var, temp ) ;
					return temp ;
				}
				return *p_str ;

			case 4:
				err.setRC( 0 ) ;
				if ( pfield && pfield->field_is_input() )
				{
					temp = p_poolMGR->get( err, var ) ;
					funcPool->put2( err, var, temp ) ;
					return temp ;
				}
				return p_poolMGR->get( err, var ) ;

			case 8:
				funcPool->put1( err, var, "" ) ;
		}
	}

	return "" ;
}


void pPanel::putDialogueVar( errblock& err,
			     const string& var,
			     const string& val )
{
	//
	// Store data for a dialogue variable in the function pool.
	// Creates an implicit function pool variable if one does not already exist.
	//
	// If the variable has been vdefined as an INTEGER, convert to integer and put.
	//
	// Selection panels do not use the function pool.  Put in SHARED or PROFILE
	// wherever it resides in (or SHARED if not found).
	//
	// RC =  0 Normal completion.
	// RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	fVAR* pvar ;

	if ( selPanel )
	{
		p_poolMGR->put( err, var, val, ASIS ) ;
	}
	else
	{
		pvar = funcPool->getfVAR( err, var ) ;
		if ( err.error() ) { return ; }
		if ( pvar )
		{
			pvar->put( err, var, val ) ;
		}
		else
		{
			funcPool->put2( err, var, val ) ;
		}
	}
}


void pPanel::set_popup( errblock& err,
			bool addpop_active,
			int& addpop_row,
			int& addpop_col )
{
	TRACE_FUNCTION() ;

	int i ;

	if ( addpop_active )
	{
		i = ( p_poolMGR->sysget( err, "ZSWPBR", PROFILE ) == "Y" && wscrmaxd < zscrmaxd - 2 ) ? 2 : 1 ;
		win_addpop = true ;
		win_row    = ( addpop_row + win_depth + i ) < zscrmaxd ? addpop_row : ( zscrmaxd - win_depth - i ) ;
		win_col    = ( addpop_col + win_width + 1 ) < zscrmaxw ? addpop_col : ( zscrmaxw - win_width - 1 ) ;
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
	TRACE_FUNCTION() ;

	mvwin( win, win_row, win_col ) ;
	mvwin( bwin, win_row-1, win_col-1 ) ;
}


void pPanel::remove_popup()
{
	TRACE_FUNCTION() ;

	win_addpop = false ;
	win_row    = 0 ;
	win_col    = 0 ;
}


void pPanel::show_popup()
{
	TRACE_FUNCTION() ;

	if ( !win || win != pwin ) { return ; }

	top_panel( bpanel ) ;
	top_panel( panel ) ;
}


void pPanel::hide_popup()
{
	TRACE_FUNCTION() ;

	if ( !win || win != pwin ) { return ; }

	hide_panel( bpanel ) ;
	hide_panel( panel ) ;

	if ( pfkwin )
	{
		hide_panel( pfkpanel ) ;
	}
}


string pPanel::get_help()
{
	//
	// For the HELP command, return the .HELP control variable unless
	// it is requested by the tutorial program, in which case return .HHELP
	// (or the help tutorial panel lsph0000 if blank).
	//

	TRACE_FUNCTION() ;

	return ( tutorial )     ?
	       ( dHHelp != "" ) ? dHHelp : "LSPH0000" : dHelp ;
}


string pPanel::get_exhelp( errblock& err )
{
	//
	// For the EXHELP command, return the .HELP control variable unless
	// it is blank, in which case return ZHTOP.
	//

	TRACE_FUNCTION() ;

	return ( dHelp == "" ) ? getDialogueVar( err, "ZHTOP" ) : dHelp ;
}


void pPanel::create_panels( popup& p )
{
	//
	// If we are being displayed in a popup, create another set of
	// ncurses windows/panels on a subsequent ADDPOP call and store
	// original windows/panels in the popup structure (if panel has
	// been written to, ie displayed).
	//

	TRACE_FUNCTION() ;

	if ( !displayed || !win || win != pwin ) { return ; }

	p.pan1 = panel ;
	p.pan2 = bpanel ;
	p.panl = this ;

	pwin   = newwin( win_depth, win_width, 0, 0 ) ;
	panel  = new_panel( pwin ) ;
	set_panel_userptr( panel, new panel_data( zscrnum, this ) ) ;

	bwin   = newwin( win_depth+2, win_width+2, 0, 0 ) ;
	bpanel = new_panel( bwin ) ;
	set_panel_userptr( bpanel, new panel_data( zscrnum, true, this ) ) ;

	update_panels() ;

	win       = pwin ;
	displayed = false ;
}


void pPanel::pop_panels( const popup& p )
{
	//
	// Delete the current set of ncurses windows/panels and restore the top popup in
	// the popup structure on a REMPOP call.
	//
	// The popup 'p' may be empty if the window was never displayed before another ADDPOP.
	//

	TRACE_FUNCTION() ;

	WINDOW* w ;

	if ( !p.pan1 )
	{
		return ;
	}

	w = panel_window( panel ) ;
	panel_cleanup( panel ) ;
	del_panel( panel ) ;
	delwin( w ) ;

	w = panel_window( bpanel ) ;
	panel_cleanup( bpanel ) ;
	del_panel( bpanel ) ;
	delwin( w ) ;

	update_panels() ;

	panel  = p.pan1 ;
	bpanel = p.pan2 ;

	pwin = panel_window( panel ) ;
	bwin = panel_window( bpanel ) ;
}


void pPanel::decolourise( WINDOW* w,
			  uint colour1,
			  uint colour2,
			  uint intens )
{
	//
	// Remove the colour from the panel window w and any displayed messages, using mvwchgat().
	//
	// Redraw the action bar divider and boxes as ncurses line drawing is an attribute and
	// removing the attributes results in the line being changed to the mapping character
	// (eg. 'q' for ACS_HLINE).
	//

	TRACE_FUNCTION() ;

	int maxy ;
	int maxx ;

	for ( uint i = 0 ; i < wscrmaxd ; ++i )
	{
		mvwchgat( w, i, 0, -1, intens | panel_intens, colour1, nullptr ) ;
	}

	if ( ab.size() > 0 )
	{
		wattrset( w, colour2 | intens | panel_intens ) ;
		mvwhline( w, 1, 0, ACS_HLINE, wscrmaxw ) ;
	}

	for ( auto& box : boxes )
	{
		box->display_box( win, sub_vars( box->box_title ), colour2, intens ) ;
	}

	if ( smwin && !panel_hidden( smpanel) )
	{
		getmaxyx( smwin, maxy, maxx ) ;
		maxx = -1 ;
		if ( is_inbox( smpanel ) )
		{
			for ( int i = 1 ; i < ( maxy - 1 ) ; ++i )
			{
				mvwchgat( smwin, i, 1, maxx, intens | panel_intens, colour1, nullptr ) ;
			}
			wattrset( smwin, colour2 | intens | panel_intens ) ;
			box( smwin, 0, 0 ) ;
		}
		else
		{
			mvwchgat( smwin, 0, 0, maxx, intens | panel_intens, colour1, nullptr ) ;
		}
	}

	if ( lmwin && !panel_hidden( lmpanel) )
	{
		getmaxyx( lmwin, maxy, maxx ) ;
		maxx = -1 ;
		if ( is_inbox( lmpanel ) )
		{
			for ( int i = 1 ; i < ( maxy - 1 ) ; ++i )
			{
				mvwchgat( lmwin, i, 1, maxx, intens | panel_intens, colour1, nullptr ) ;
			}
			wattrset( lmwin, colour2 | intens | panel_intens ) ;
			box( lmwin, 0, 0 ) ;
		}
		else
		{
			mvwchgat( lmwin, 0, 0, maxx, intens | panel_intens, colour1, nullptr ) ;
		}
	}
}


void pPanel::redraw_panel( errblock& err )
{
	//
	// Redraw the entire panel if it has been decolourised, or just the panel frame
	// if decolourisation has been set off.
	//

	TRACE_FUNCTION() ;

	if ( is_panel_decolourised() )
	{
		display_panel( err, false ) ;
		if ( bwin )
		{
			set_panel_frame_act() ;
		}
	}
	else
	{
		if ( bwin && is_panel_frame_inact() )
		{
			wattrset( bwin, cuaAttr[ AWF ] | panel_intens ) ;
			draw_frame( err ) ;
			set_panel_frame_act() ;
		}
		draw_msgframes() ;
	}
}


void pPanel::toggle_fscreen( errblock& err,
			     bool sp_popup,
			     int sp_row,
			     int sp_col )
{
	TRACE_FUNCTION() ;

	full_screen = !full_screen ;

	if ( !full_screen && sp_popup )
	{
		set_popup( err, sp_popup, sp_row, sp_col ) ;
	}
	clear_msg() ;
}


void pPanel::display_panel( errblock& err,
			    bool reset )
{
	//
	// fwin - created unconditionally and is the full-screen window.
	// pwin - pop-up window only created if the panel contains a WINDOW(w,d) statement.
	// (associate ncurses panel 'panel' with whichever is active and set WSCRMAX?).
	//
	// Use fwin if no pwin exists, or no ADDPOP() has been done and remove the popup
	// if no pwin exists, even if an ADDPOP() has been done (not exactly how real ISPF works).
	// The RESIZE lspf command can also toggle full screen mode.
	//

	TRACE_FUNCTION() ;

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
		mvwin( win, win_row, win_col ) ;
		mvwin( bwin, win_row-1, win_col-1 ) ;
		wattrset( bwin, cuaAttr[ AWF ] | panel_intens ) ;
		draw_frame( err ) ;
		if ( err.error() ) { return ; }
		top_panel( bpanel ) ;
		top_panel( panel ) ;
		wscrmaxw  = win_width ;
		wscrmaxd  = win_depth ;
		displayed = true ;
		unset_panel_fscreen() ;
	}
	else
	{
		if ( win_addpop ) { remove_popup() ; }
		win = fwin ;
		if ( bwin )
		{
			hide_panel( bpanel ) ;
			wscrmaxw = win_width ;
			wscrmaxd = win_depth ;
		}
		else
		{
			wscrmaxw = zscrmaxw ;
			wscrmaxd = zscrmaxd ;
		}
		if ( win != panel_window( panel ) )
		{
			replace_panel( panel, win ) ;
		}
		top_panel( panel ) ;
		set_panel_fscreen() ;
	}

	if ( win != owin )
	{
		owin = win ;
		build_pfkeys( err, true ) ;
	}

	werase( win ) ;
	unset_panel_decolourised() ;

	panttl = sub_vars( panelTitle ) ;
	wattrset( win, cuaAttr[ PT ] | panel_intens ) ;
	mvwaddstr( win, ( ab.size() > 0 ) ? 2 : 0, ( wscrmaxw - panttl.size() ) / 2, panttl.c_str() ) ;

	if ( tb_model )
	{
		display_ztdmark( err ) ;
		if ( err.error() ) { return ; }
		set_tb_fields_act_inact( err ) ;
		if ( err.error() ) { return ; }
		if ( !reset )
		{
			display_row_indicator( err ) ;
		}
	}

	display_ab( err ) ;
	display_text() ;
	display_tbtext() ;

	display_fields( err, reset ) ;
	if ( err.error() ) { return ; }

	display_boxes() ;

	hide_pd( err ) ;

	display_area_si() ;

	display_pd( err )  ;
	if ( err.error() ) { return ; }

	build_pfkeys( err ) ;
	if ( err.error() ) { return ; }

	display_pfkeys( err ) ;
	if ( err.error() ) { return ; }

	display_msg( err ) ;
	if ( err.error() ) { return ; }

	display_id( err )  ;
	if ( err.error() ) { return ; }

	error_msg = ( dMsg != "" && MSG.type != IMT ) ;

	if ( dAlarm == "YES" ) { beep() ; }
}


void pPanel::draw_frame( PANEL* p,
			 uint col )
{
	//
	// Re-draw frame and message frames, colour col.
	//

	TRACE_FUNCTION() ;

	WINDOW* w = panel_window( p ) ;

	string winttl = get_panel_ttl( p ) ;

	wattrset( w, col | panel_intens ) ;
	box( w, 0, 0 ) ;

	if ( winttl != "" )
	{
		mvwaddstr( w, 0, ( win_width - winttl.size() + 2 ) / 2, winttl.c_str() ) ;
	}

	draw_msgframes( col ) ;
}


void pPanel::draw_frame( errblock& err )
{
	//
	// Set frame active.
	//

	TRACE_FUNCTION() ;

	string winttl = getDialogueVar( err, "ZWINTTL" ) ;
	if ( err.error() ) { return ; }

	box( bwin, 0, 0 ) ;

	if ( winttl != "" )
	{
		winttl = " "+ winttl +" " ;
		mvwaddstr( bwin, 0, ( win_width - winttl.size() + 2 ) / 2, winttl.c_str() ) ;
		set_panel_ttl( winttl ) ;
	}

	set_panel_frame_act() ;
}


void pPanel::draw_msgframes( uint col )
{
	TRACE_FUNCTION() ;

	if ( smwin && is_inbox( smpanel ) && !panel_hidden( smpanel) )
	{
		wattrset( smwin, col | panel_intens ) ;
		box( smwin, 0, 0 ) ;
	}

	if ( lmwin && is_inbox( lmpanel ) && !panel_hidden( lmpanel) )
	{
		wattrset( lmwin, col | panel_intens ) ;
		box( lmwin, 0, 0 ) ;
	}
}


void pPanel::draw_msgframes()
{
	TRACE_FUNCTION() ;

	uint col ;

	if ( smwin && is_inbox( smpanel ) && !panel_hidden( smpanel) )
	{
		col = mvwinch( smwin, 1, 2 ) & A_COLOR ;
		wattrset( smwin, col | panel_intens ) ;
		box( smwin, 0, 0 ) ;
	}

	if ( lmwin && is_inbox( lmpanel ) && !panel_hidden( lmpanel) )
	{
		col = mvwinch( lmwin, 1, 2 ) & A_COLOR ;
		wattrset( lmwin, col | panel_intens ) ;
		box( lmwin, 0, 0 ) ;
	}
}


void pPanel::redraw_fields( errblock& err )
{
	//
	// Re-draw all fields in a window (eg after a .SHOW/.HIDE NULLS command).
	//

	TRACE_FUNCTION() ;

	display_fields( err ) ;
}


void pPanel::refresh()
{
	//
	// Refresh the screen by showing the current ncurses panel
	// (win always references the actual window in use, either fwin, or pwin).
	//

	TRACE_FUNCTION() ;

	if ( win == pwin )
	{
		top_panel( bpanel ) ;
		touchwin( panel_window( bpanel ) ) ;
	}

	top_panel( panel ) ;
	touchwin( win ) ;
}


void pPanel::restore()
{
	//
	// Performed during CONTROL DISPLAY RESTORE processing.
	//
	// A previous TBDISPL may have displayed a panel with the same
	// table variable name(s) which will have destroyed the function pool value.
	// Restore these from the field value in case this has happened.
	//

	TRACE_FUNCTION() ;

	field* pfield ;

	map<string, field*>::iterator it ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; ++it )
	{
		pfield = it->second ;
		if ( pfield->field_is_tbdispl() )
		{
			funcPool->put3( it->first, pfield->field_get_value() ) ;
		}
	}
}


bool pPanel::do_redisplay()
{
	TRACE_FUNCTION() ;

	return redisplay ;
}


void pPanel::update_field_values( errblock& err )
{
	//
	// This is executed before a panel is displayed.
	//
	// Update field_values from the dialogue variables.
	// Dialogue variable may not exist so treat RC=8 from getDialogueVar() as normal completion.
	//
	// For internal field names, only retrieve from the function pool and set to nulls if not found.
	//
	// For scrollable fields, first update the relevant dialogue variables from the sfield structure (IND, LIND, RIND, etc)
	//
	// If dynamic area is not in the function pool, copy from the SHARED or PROFILE pool
	// to the function pool first. Resize area or shadow values if smaller than the dynamic area size.
	//

	TRACE_FUNCTION() ;

	uint i ;
	uint j ;
	uint l ;
	uint m ;

	uint area ;
	uint width ;

	string val ;
	string var ;
	string mask ;
	string shadow ;

	string* darea ;
	string* dshadow ;
	string* strptr1 ;
	string* strptr2 ;

	dynArea* da ;

	field* pfield ;

	field_ext1* dx ;

	map<string, field*>::iterator itf ;
	map<string, dynArea*>::iterator itd ;

	VEDIT_TYPE vtype ;

	err.setRC( 0 ) ;

	for ( auto& sf : sfields )
	{
		pfield = sf.first ;
		val    = ( pfield->field_is_tbdispl() ) ? funcPool->get3( err, sf.second.first ) :
							  getDialogueVar( err, sf.second.first, pfield ) ;
		if ( err.error() ) { return ; }
		pfield->field_put_value( val ) ;
		if ( pfield->field_has_lenvar() )
		{
			update_lenvar( err, pfield ) ;
			if ( err.error() ) { return ; }
		}
		if ( funcPool->hasmask( sf.second.first, mask, vtype ) )
		{
			val = pfield->field_get_value() ;
			pfield->field_put_value( add_vmask( val, zdatef, mask, vtype ) ) ;
		}
		if ( pfield->field_has_scroll_var() )
		{
			pfield->field_set_scroll_parm( upper( getDialogueVar( err, pfield->field_get_scroll_var() ) ) ) ;
			if ( err.error() ) { return ; }
		}
		if ( err.error() ) { return ; }
		if ( pfield->field_has_lcol_var() )
		{
			var = pfield->field_get_lcol_var() ;
			val = getDialogueVar( err, var ) ;
			if ( err.error() ) { return ; }
			l = 1 ;
			if ( isnumeric( val ) )
			{
				m = ds2d( val ) ;
				if ( m >= 1 && m <= 32767 )
				{
					l = m ;
				}
			}
			pfield->field_set_lcol( l ) ;
		}
		update_scroll_inds( err, pfield, false ) ;
		if ( err.error() ) { return ; }
		update_scroll_rcol( err, pfield, false ) ;
		if ( err.error() ) { return ; }
	}

	auto pitr = lcolmmap.begin() ;

	for ( auto citr = lcolmmap.begin() ; citr != lcolmmap.end() ; ++citr )
	{
		if ( pitr->first != citr->first )
		{
			putDialogueVar( err, pitr->first, d2ds( max_scroll_lcol( pitr->second ) ) ) ;
			if ( err.error() ) { return ; }
			pitr = citr ;
		}
	}
	if ( pitr != lcolmmap.end() )
	{
		putDialogueVar( err, pitr->first, d2ds( max_scroll_lcol( pitr->second ) ) ) ;
		if ( err.error() ) { return ; }
	}

	for ( itf = fieldList.begin() ; itf != fieldList.end() ; ++itf )
	{
		pfield = itf->second ;
		if ( !pfield->field_is_dynamic() && !pfield->field_is_scrollable() )
		{
			if ( pfield->field_validname )
			{
				pfield->field_put_value( getDialogueVar( err, itf->first, pfield ) ) ;
				if ( err.error() ) { return ; }
				if ( funcPool->hasmask( itf->first, mask, vtype ) )
				{
					val = pfield->field_get_value() ;
					pfield->field_put_value( add_vmask( val, zdatef, mask, vtype ) ) ;
					continue ;
				}
			}
			else
			{
				pfield->field_put_value( funcPool->get3( err, itf->first ) ) ;
				if ( err.RC8() )
				{
					pfield->field_put_value( "" ) ;
					funcPool->put3( itf->first, "" ) ;
				}
			}
		}
		pfield->field_changed = false ;
	}
	if ( err.RC8() ) { err.setRC( 0 ) ; }

	for ( itd = dynAreaList.begin() ; itd != dynAreaList.end() ; ++itd )
	{
		darea = funcPool->vlocate( err, itd->first ) ;
		if ( err.error() ) { return ; }
		if ( err.RC8() )
		{
			funcPool->put2( err, itd->first, p_poolMGR->get( err, itd->first, ASIS ) ) ;
			if ( err.error() ) { return ; }
			darea = funcPool->vlocate( err, itd->first ) ;
			if ( !err.RC0() )
			{
				err.seterrid( TRACE_INFO(), "PSYE032G", itd->first ) ;
				return ;
			}
		}
		da       = itd->second ;
		shadow   = da->dynArea_shadow_name ;
		dshadow  = funcPool->vlocate( err, shadow ) ;
		if ( err.error() ) { return ; }
		if ( err.RC8() )
		{
			funcPool->put2( err, shadow, p_poolMGR->get( err, shadow, ASIS ) ) ;
			if ( err.error() ) { return ; }
			dshadow = funcPool->vlocate( err, shadow ) ;
			if ( !err.RC0() )
			{
				err.seterrid( TRACE_INFO(), "PSYE032I", shadow ) ;
				return ;
			}
		}
		width = da->dynArea_width ;
		area  = da->dynArea_area  ;
		if ( darea->size() < area )
		{
			darea->resize( area, ' ' ) ;
		}
		if ( dshadow->size() < area )
		{
			dshadow->resize( area, 0xFF ) ;
		}
		i = 0 ;
		for ( auto f : da->fieldList )
		{
			pfield = static_cast<field*>( f ) ;
			j      = i * width ;
			pfield->field_value = darea->substr( j, width ) ;
			dx = pfield->field_da_ext ;
			dx->field_ext1_shadow = dshadow->substr( j, width ) ;
			if ( dx->field_ext1_has_overflow() )
			{
				strptr1 = funcPool->vlocate( err, dx->field_ext1_overflow_vname ) ;
				if ( err.error() ) { return ; }
				if ( err.RC0() )
				{
					strptr2 = funcPool->vlocate( err, dx->field_ext1_overflow_sname ) ;
					if ( err.error() ) { return ; }
					if ( err.RC0() )
					{
						dx->field_ext1_load( strptr1, strptr2 ) ;
					}
				}
				if ( err.RC8() )
				{
					dx->field_ext1_clear() ;
					err.setRC( 0 ) ;
				}
			}
			++i ;
		}
	}
}


void pPanel::update_tbfields( errblock& err )
{
	//
	// Procedure to update the internal fields for tbfield statements if they contain variables for the columns and/or lengths.
	//

	TRACE_FUNCTION() ;

	uint i ;
	uint j ;

	string t ;

	bool tb_changed = false ;

	for ( auto& tb : tbfields )
	{
		i = -1 ;
		j = -1 ;
		if ( tb->tbfield_col_var != "" )
		{
			t = getDialogueVar( err, tb->tbfield_col_var ) ;
			if ( !isnumeric( t ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE042B", tb->tbfield_name, tb->tbfield_col_var, t ) ;
				return ;
			}
			i = ds2d( t ) ;
			if ( i > wscrmaxw )
			{
				err.seterrid( TRACE_INFO(), "PSYE031B", tb->tbfield_name, d2ds( i ), d2ds( wscrmaxw ) ) ;
				return ;
			}
		}
		if ( tb->tbfield_len_var != "" )
		{
			t = getDialogueVar( err, tb->tbfield_len_var ) ;
			if ( !isnumeric( t ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE042B", tb->tbfield_name, tb->tbfield_len_var, t ) ;
				return ;
			}
			j = ds2d( t ) ;
		}
		if ( tb->set_tbfield( i, j ) )
		{
			if ( ( tb->tbfield_col + tb->tbfield_len - 1 ) > wscrmaxw )
			{
				err.seterrid( TRACE_INFO(), "PSYE031X", tb->tbfield_name ) ;
				return ;
			}
			tb->update_fields() ;
			tb_changed = true ;
		}
	}

	if ( tb_changed )
	{
		check_overlapping_fields( err, true ) ;
		if ( err.error() ) { return ; }
		build_fieldMap() ;
	}
}


void pPanel::set_tblen()
{
	//
	// For scrollable fields on a table display, the field display length will be
	// the maximum of all instances on the current display.
	//

	TRACE_FUNCTION() ;

	field* pfield ;

	if ( tblenvars.size() > 0 )
	{
		for ( auto it = tblenvars.begin() ; it != tblenvars.end() ; ++it )
		{
			for ( int i = 0 ; i < tb_rdepth ; ++i )
			{
				pfield = fieldList[ it->first + "." + d2ds( i ) ] ;
				pfield->field_set_tblen( it->second.second ) ;
			}
		}
	}
}


void pPanel::display_panel_attrs( errblock& err )
{
	//
	// Go through the list of character attributes and perform any
	// variable substitution after )INIT processing has completed.
	//

	TRACE_FUNCTION() ;

	string t ;

	char_attrs* attrchar ;

	for ( auto it = char_attrlist.begin() ; it != char_attrlist.end() ; ++it )
	{
		attrchar = it->second ;
		if ( attrchar->has_dvars() )
		{
			t = sub_vars( attrchar->get_entry1() ) ;
			attrchar->update( err, t ) ;
			if ( err.error() ) { return ; }
			auto it1 = ddata_map.find( it->first ) ;
			if ( it1 != ddata_map.end() )
			{
				it1->second = attrchar->get_colour() ;
			}
			else
			{
				auto it2 = schar_map.find( it->first ) ;
				if ( it2 != schar_map.end() )
				{
					it2->second = attrchar->get_colour() ;
				}
			}
		}
	}
}


void pPanel::display_panel_init( errblock& err )
{
	//
	// Perform panel )INIT processing.
	//

	TRACE_FUNCTION() ;

	dynArea* da ;

	string val ;

	s_row = 0 ;
	s_col = 0 ;

	set_pfpressed( "" ) ;
	err.setRC( 0 ) ;

	putDialogueVar( err, "ZPRIM", "" ) ;
	if ( err.error() ) { return ; }

	scroll_all_areas_to_top() ;
	scroll_all_fields_to_start( err ) ;
	if ( err.error() ) { return ; }

	reset_attrs() ;

	process_panel_stmnts( err,
			      0,
			      initstmnts,
			      PS_INIT ) ;
	if ( err.error() ) { return ; }

	if ( tb_model && tb_dvars )
	{
		update_tbfields( err ) ;
		if ( err.error() ) { return ; }
	}

	if ( tblenvars.size() > 0 )
	{
		for ( auto it = tblenvars.begin() ; it != tblenvars.end() ; ++it )
		{
			if ( it->second.first != "" )
			{
				funcPool->put2( err, it->second.first, d2ds( it->second.second ) ) ;
				if ( err.error() ) { return ; }
			}
		}
	}

	for ( auto itd = dynAreaList.begin() ; itd != dynAreaList.end() ; ++itd )
	{
		da = itd->second ;
		if ( da->dynArea_olenvar != "" )
		{
			val = getDialogueVar( err, da->dynArea_olenvar ) ;
			if ( err.error() ) { return ; }
			if ( val.size() > 5 || !datatype( val, 'W' ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE033K", val ) ;
				return ;
			}
			da->dynArea_olen = ds2d( val ) ;
			if ( da->dynArea_olen > 65535 )
			{
				err.seterrid( TRACE_INFO(), "PSYE033K", val ) ;
				return ;
			}
		}
	}
}


void pPanel::display_panel_reinit( errblock& err,
				   int ln )
{
	//
	// Perform panel )REINIT processing.
	//

	TRACE_FUNCTION() ;

	s_row = 0 ;
	s_col = 0 ;

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


void pPanel::display_panel_proc( errblock& err,
				 int ln )
{
	//
	// Perform panel )PROC processing.
	// If a selection panel, check .TRAIL if not NOCHECK on the ZSEL variable and issue error.
	//

	TRACE_FUNCTION() ;

	int p ;

	string zsel ;

	err.setRC( 0 ) ;

	process_panel_stmnts( err,
			      ln,
			      procstmnts,
			      PS_PROC ) ;
	if ( err.error() ) { return ; }

	if ( selPanel )
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
			putDialogueVar( err, "ZSEL", zsel ) ;
			if ( err.error() ) { return ; }
		}
	}
}


void pPanel::abc_panel_init( errblock& err,
			     const string& abc_desc )
{
	//
	// Perform panel )ABCINIT processing.
	//

	TRACE_FUNCTION() ;

	err.setRC( 0 ) ;

	dCursor = "" ;
	dMsg    = "" ;
	dResp   = "" ;
	setControlVar( err, ".ZVARS", "", PS_ABCINIT ) ;

	process_panel_stmnts( err,
			      0,
			      abc_initstmnts[ abc_desc ],
			      PS_ABCINIT ) ;
}


void pPanel::abc_panel_proc( errblock& err,
			     const string& abc_desc )
{
	//
	// Perform panel )ABCPROC processing.
	//

	TRACE_FUNCTION() ;

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
	//
	// General routine to process panel statements.  Pass address of the panel statements vector for the
	// panel section being processed.
	//
	// For table display fields, .ATTR and .CURSOR apply to fields on line ln.
	//
	// If the panel statment has a ps_if address and a ps_xxx address, this is for statements coded
	// inline on the if-statement so test and execute if the if-statement is true.
	//

	TRACE_FUNCTION() ;

	int if_column ;

	bool if_skip ;

	string g_label ;

	if_column = 0 ;
	if_skip   = false ;

	set<string> attr_sect ;
	set<string> attrchar_sect ;

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
					break ;
				}
				if ( (*ips)->ps_if )
				{
					(*ips)->ps_if->if_true = true ;
				}
				else if ( (*ips)->ps_else )
				{
					(*ips)->ps_else->if_true = false ;
				}
			}
			if ( g_label != "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE041F", g_label ) ;
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
			if ( (*ips)->ps_vputget->vpg_vput )
			{
				process_panel_vput( err,
						    (*ips)->ps_vputget ) ;
			}
			else
			{
				process_panel_vget( err,
						   (*ips)->ps_vputget ) ;
			}
			if ( err.error() ) { return ; }
		}
		else if ( (*ips)->ps_assign )
		{
			process_panel_assignment( err,
						  ln,
						  (*ips)->ps_assign,
						  attr_sect,
						  attrchar_sect,
						  ps_sect ) ;
			if ( err.error() ) { return ; }
		}
		else if ( (*ips)->ps_vedit )
		{
			process_panel_vedit( err,
					     ln,
					     (*ips)->ps_vedit,
					     ps_sect ) ;
			if ( err.error() ) { return ; }
		}
		else if ( (*ips)->ps_prexx )
		{
			process_panel_prexx( err,
					     (*ips)->ps_prexx ) ;
			if ( err.error() ) { return ; }
		}
		else if ( (*ips)->ps_panexit )
		{
			process_panel_panexit( err,
					       (*ips)->ps_panexit,
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
	//
	// Group AND statements as these have a higher precedence than OR.
	//

	TRACE_FUNCTION() ;

	bool if_AND ;
	bool group_true ;

	process_panel_if_cond( err,
			       ln,
			       ifstmnt ) ;
	if ( err.error() || !ifstmnt->if_next ) { return ; }

	IFSTMNT* if_stmnt = ifstmnt ;

	group_true = ifstmnt->if_true ;
	while ( ifstmnt->if_next )
	{
		if_AND  = ifstmnt->if_AND  ;
		ifstmnt = ifstmnt->if_next ;
		process_panel_if_cond( err,
				       ln,
				       ifstmnt ) ;
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
	//
	// Process IF condition.
	// Ignore trailing spaces when performing the comparison.
	//

	TRACE_FUNCTION() ;

	int i  ;
	int r1 ;
	int r2 ;
	int lhs_ival = 0 ;

	uint j ;

	VERIFY* verify = ifstmnt->if_verify ;

	bool l_break ;
	bool numeric ;
	bool numeric_lhs ;

	string t1 ;
	string temp ;
	string rhs_val ;
	string lhs_val ;
	string fieldVal ;

	char   pictcn1 ;
	string pictcn2 ;
	string pictcn3 ;

	const string picts = "CAN9Xcanx" ;

	vector<string> qw ;

	vector<string>::iterator it ;

	struct sockaddr_in sa ;

	ifstmnt->if_true = ( ifstmnt->if_cond == RL_NE ) ;

	if ( verify )
	{
		fieldVal = getDialogueVar( err, verify->ver_var ) ;
		trim_right( fieldVal ) ;
		if ( err.error() ) { return ; }
		if ( verify->ver_nblank )
		{
			ifstmnt->if_true = ( fieldVal != "" ) ;
		}
		else if ( fieldVal == "" )
		{
			ifstmnt->if_true = true ;
		}
		if ( fieldVal != "" || verify->ver_type == VER_LEN )
		{
			switch ( verify->ver_type )
			{
			case VER_ALPHA:
				ifstmnt->if_true = isalpha( fieldVal ) ;
				break ;

			case VER_ALPHAB:
				ifstmnt->if_true = isalphab( fieldVal ) ;
				break ;

			case VER_BIT:
				ifstmnt->if_true = isbit( fieldVal ) ;
				break ;

			case VER_ENUM:
				trim_left( fieldVal ) ;
				isenum( err, fieldVal ) ;
				ifstmnt->if_true = ( err.getRC() == 0 ) ;
				break ;

			case VER_HEX:
				ifstmnt->if_true = ishex( fieldVal ) ;
				break ;

			case VER_INCLUDE:
				if ( ( verify->ver_include & INC_IMBLK ) == INC_IMBLK )
				{
					fieldVal.erase( std::remove( fieldVal.begin(), fieldVal.end(), ' ' ), fieldVal.end() ) ;
				}
				if ( ( verify->ver_include & INC_ALPHANUM ) == INC_ALPHANUM )
				{
					ifstmnt->if_true = isalphanum( fieldVal ) ;
				}
				else if ( ( verify->ver_include & INC_ALPHABNUM ) == INC_ALPHABNUM )
				{
					ifstmnt->if_true = isalphabnum( fieldVal ) ;
				}
				else if ( ( verify->ver_include & INC_ALPHA ) == INC_ALPHA )
				{
					ifstmnt->if_true = isalpha( fieldVal ) ;
				}
				else if ( ( verify->ver_include & INC_ALPHAB ) == INC_ALPHAB )
				{
					ifstmnt->if_true = isalphab( fieldVal ) ;
				}
				else
				{
					ifstmnt->if_true = isnumeric( fieldVal ) ;
				}
				break ;

			case VER_IPADDR4:
				ifstmnt->if_true = ( inet_pton( AF_INET, fieldVal.c_str(), &(sa.sin_addr) ) == 1 ) ;
				break ;

			case VER_IPADDR6:
				ifstmnt->if_true = ( inet_pton( AF_INET6, fieldVal.c_str(), &(sa.sin_addr) ) == 1 ) ;
				break ;

			case VER_LEN:
				if ( verify->ver_vlist.empty() )
				{
					j = verify->ver_len ;
				}
				else
				{
					temp = getDialogueVar( err, verify->ver_vlist[ 0 ] ) ;
					if ( err.error() ) { return ; }
					if ( !datatype( temp, 'W' ) )
					{
						err.seterrid( TRACE_INFO(), "PSYE052A", temp, "verify LEN" ) ;
						return ;
					}
					j = ds2d( temp ) ;
				}
				switch ( verify->ver_cond )
				{
				case VL_EQ:
					ifstmnt->if_true = ( fieldVal.size() == j ) ;
					break ;

				case VL_NE:
					ifstmnt->if_true = ( fieldVal.size() != j ) ;
					break ;

				case VL_GT:
					ifstmnt->if_true = ( fieldVal.size() > j  ) ;
					break ;

				case VL_GE:
					ifstmnt->if_true = ( fieldVal.size() >= j ) ;
					break ;

				case VL_LE:
					ifstmnt->if_true = ( fieldVal.size() <= j ) ;
					break ;

				case VL_LT:
					ifstmnt->if_true = ( fieldVal.size() < j  ) ;
					break ;

				}
				break ;

			case VER_LIST:
			case VER_LISTX:
				for ( it = verify->ver_vlist.begin() ; it != verify->ver_vlist.end() ; ++it )
				{
					if ( strip_right( sub_vars( *it ) ) == fieldVal ) { break ; }
				}
				if ( verify->ver_type == VER_LIST )
				{
					ifstmnt->if_true = ( it != verify->ver_vlist.end() ) ;
				}
				else
				{
					ifstmnt->if_true = ( it == verify->ver_vlist.end() ) ;
				}
				break ;

			case VER_LISTV:
			case VER_LISTVX:
				temp = getDialogueVar( err, verify->ver_vlist.front() ) ;
				if ( err.error() ) { return ; }
				getlist( err, temp, qw ) ;
				if ( err.error() || qw.size() == 0 ) { return ; }
				for ( it = qw.begin() ; it != qw.end() ; ++it )
				{
					if ( strip_right( sub_vars( *it ) ) == fieldVal ) { break ; }
				}
				if ( verify->ver_type == VER_LISTV )
				{
					ifstmnt->if_true = ( it != qw.end() ) ;
				}
				else
				{
					ifstmnt->if_true = ( it == qw.end() ) ;
				}
				break ;

			case VER_NAME:
				ifstmnt->if_true = isvalidName( fieldVal ) ;
				break ;

			case VER_NUMERIC:
				trim_left( fieldVal ) ;
				ifstmnt->if_true = isnumeric( fieldVal ) ;
				break ;

			case VER_OCT:
				ifstmnt->if_true = isoctal( fieldVal ) ;
				break ;

			case VER_PICT:
				ifstmnt->if_true = ispict( fieldVal, verify->ver_vlist[ 0 ] ) ;
				break ;

			case VER_PICTCN:
				temp = getVariable( err, verify->ver_vlist[ 0 ] ) ;
				if ( temp.size() != 1 )
				{
					err.seterrid( TRACE_INFO(), "PSYE052R" ) ;
					return ;
				}
				pictcn1 = temp[ 0 ] ;
				if ( picts.find( pictcn1 ) != string::npos )
				{
					err.seterrid( TRACE_INFO(), "PSYE052R" ) ;
					return ;
				}
				pictcn2 = getVariable( err, verify->ver_vlist[ 1 ] ) ;
				if ( pictcn2.find( pictcn1 ) == string::npos )
				{
					err.seterrid( TRACE_INFO(), "PSYE052S" ) ;
					return ;
				}
				pictcn3 = getVariable( err, verify->ver_vlist[ 2 ] ) ;
				ispictcn( err, fieldVal, pictcn1, pictcn2, pictcn3 ) ;
				if ( err.error() ) { return ; }
				ifstmnt->if_true = ( err.getRC() == 0 ) ;
				break ;

			case VER_RANGE:
				if ( !datatype( fieldVal, 'W' ) )
				{
					ifstmnt->if_true = false ;
					break ;
				}
				temp = verify->ver_vlist[ 0 ] ;
				if  ( !datatype( temp, 'W' ) )
				{
					t1   = temp ;
					temp = getDialogueVar( err, temp ) ;
					if ( err.error() ) { return ; }
					if ( !datatype( temp, 'W' ) )
					{
						err.seterrid( TRACE_INFO(), "PSYE052A", t1, "verify RANGE" ) ;
						return ;
					}
				}
				r1   = ds2d( temp ) ;
				temp = verify->ver_vlist[ 1 ] ;
				if  ( !datatype( temp, 'W' ) )
				{
					t1   = temp ;
					temp = getDialogueVar( err, temp ) ;
					if ( err.error() ) { return ; }
					if ( !datatype( temp, 'W' ) )
					{
						err.seterrid( TRACE_INFO(), "PSYE052A", t1, "verify RANGE" ) ;
						return ;
					}
				}
				r2 = ds2d( temp ) ;
				i  = ds2d( fieldVal ) ;
				ifstmnt->if_true = ( i >= r1 && i <= r2 ) ;
				break ;

			case VER_STDDATE:
				ifstmnt->if_true = check_stddate( fieldVal ) ;
				break ;

			case VER_STDTIME:
				ifstmnt->if_true = check_stdtime( fieldVal ) ;
				break ;
			}
		}
		return ;
	}

	if ( ifstmnt->if_func == PN_TRANS )
	{
		lhs_val = process_panel_trans( err,
					       ln,
					       ifstmnt->if_trans,
					       ifstmnt->if_lhs.value ) ;
		if ( err.error() ) { return ; }
	}
	else if ( ifstmnt->if_func == PN_TRUNC )
	{
		lhs_val = process_panel_trunc( err,
					       ifstmnt->if_trunc ) ;
		if ( err.error() ) { return ; }
	}
	else if ( ifstmnt->if_func == PN_PFK )
	{
		if ( ifstmnt->if_lhs.subtype == TS_AMPR_VAR_VALID )
		{
			lhs_val = getVariable( err, ifstmnt->if_lhs ) ;
			if ( err.error() ) { return ; }
		}
		else
		{
			lhs_val = ifstmnt->if_lhs.value ;
		}
		iupper( lhs_val ) ;
		process_panel_function( err,
					ifstmnt->if_func,
					ifstmnt->if_lhs.value,
					lhs_val ) ;
		if ( err.error() ) { return ; }
	}
	else if ( ifstmnt->if_lhs.subtype == TS_CTL_VAR_VALID ||
		  ifstmnt->if_lhs.subtype == TS_AMPR_VAR_VALID )
	{
		lhs_val = getVariable( err, ifstmnt->if_lhs ) ;
		if ( err.error() ) { return ; }
		process_panel_function( err,
					ifstmnt->if_func,
					ifstmnt->if_lhs.value,
					lhs_val ) ;
		if ( err.error() ) { return ; }
	}
	else
	{
		lhs_val = sub_vars( ifstmnt->if_lhs.value ) ;
	}

	trim_right( lhs_val ) ;

	numeric_lhs = isnumeric( lhs_val ) ;
	if ( numeric_lhs )
	{
		lhs_ival = ds2d( lhs_val ) ;
	}

	for ( l_break = false, j = 0 ; j < ifstmnt->if_rhs.size() && !l_break ; ++j )
	{
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
		trim_right( rhs_val ) ;
		numeric = numeric_lhs && isnumeric( rhs_val ) ;
		switch ( ifstmnt->if_cond )
		{
		case RL_EQ:
			if ( numeric )
			{
				ifstmnt->if_true = ifstmnt->if_true || ( lhs_ival == ds2d( rhs_val ) ) ;
			}
			else
			{
				ifstmnt->if_true = ifstmnt->if_true || ( lhs_val == rhs_val ) ;
			}
			l_break = ifstmnt->if_true ;
			break ;

		case RL_NE:
			if ( numeric )
			{
				ifstmnt->if_true = ifstmnt->if_true && ( lhs_ival != ds2d( rhs_val ) ) ;
			}
			else
			{
				ifstmnt->if_true = ifstmnt->if_true && ( lhs_val != rhs_val ) ;
			}
			l_break = !ifstmnt->if_true ;
			break ;

		case RL_GT:
			if ( numeric )
			{
				ifstmnt->if_true = ( lhs_ival > ds2d( rhs_val ) ) ;
			}
			else
			{
				ifstmnt->if_true = ( lhs_val > rhs_val ) ;
			}
			break ;

		case RL_GE:
			if ( numeric )
			{
				ifstmnt->if_true = ( lhs_ival >= ds2d( rhs_val ) ) ;
			}
			else
			{
				ifstmnt->if_true = ( lhs_val >= rhs_val ) ;
			}
			break ;

		case RL_LE:
			if ( numeric )
			{
				ifstmnt->if_true = ( lhs_ival <= ds2d( rhs_val ) ) ;
			}
			else
			{
				ifstmnt->if_true = ( lhs_val <= rhs_val ) ;
			}
			break ;

		case RL_LT:
			if ( numeric )
			{
				ifstmnt->if_true = ( lhs_ival < ds2d( rhs_val ) ) ;
			}
			else
			{
				ifstmnt->if_true = ( lhs_val < rhs_val ) ;
			}
			break ;
		}
	}
}


string pPanel::process_panel_trunc( errblock& err,
				    TRUNC* trunc )
{
	TRACE_FUNCTION() ;

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
	TRACE_FUNCTION() ;

	string t ;

	vector<pair<string,string>>::iterator it ;

	if ( trans->trns_trunc )
	{
		t = process_panel_trunc( err,
					 trans->trns_trunc ) ;
		if ( err.error() ) { return "" ; }
	}
	else
	{
		t = getVariable( err, trans->trns_field ) ;
		if ( err.error() ) { return "" ; }
	}

	if ( tutorial )
	{
		int i = 0 ;
		for ( auto& trns : trans->trns_list )
		{
			if ( trns.second.size() > 0 )
			{
				funcPool->put2( err, "ZHTRAN" + d2ds( ++i, 2 ), upper( trns.second ) ) ;
			}
		}
		funcPool->put2( err, "ZHTRAN00", d2ds( i, 4 ) ) ;
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


void pPanel::process_panel_vedit( errblock& err,
				  int ln,
				  VEDIT* vedit,
				  PS_SECT ps_sect )
{
	//
	// VEDIT processing.
	// Remove the function pool mask from the field value and copy value to the function pool.
	//
	// This routine only issues a message for mask errors.
	//

	TRACE_FUNCTION() ;

	string mask ;
	string val ;

	field* pfield ;

	VEDIT_TYPE vtype ;

	err.setRC( 0 ) ;

	if ( ps_sect != PS_PROC )
	{
		return ;
	}

	funcPool->getmask( err, vedit->ved_var, mask, vtype ) ;
	if ( err.error() ) { return ; }

	auto it = fieldList.find( vedit->ved_var ) ;

	pfield = it->second ;

	pfield->field_prep_input() ;
	pfield->field_apply_caps_in() ;

	val = pfield->field_get_value() ;

	remove_vmask( err,
		      zdatef,
		      vedit->ved_var,
		      val,
		      mask,
		      vtype ) ;

	if ( err.error() )
	{
		set_message_cond( err, ( vedit->ved_msgid == "" ) ? err.geterrid() : sub_vars( vedit->ved_msgid ), vedit->ved_var ) ;
		if ( err.error() ) { return ; }
		set_cursor_cond( vedit->ved_var ) ;
	}
	else
	{
		funcPool->put2( err, vedit->ved_var, val ) ;
	}
}


#ifdef HAS_REXX_SUPPORT
void pPanel::process_panel_prexx( errblock& err,
				  PREXX* prexx )
{
	//
	// Process REXX panel statement.
	//
	//   ZRXRC:
	//    0 Normal completion.
	//    8 Panel REXX-defined failure.  lspf sets .MSG and displays/redisplays panel.
	//   20 Severe error in the panel rexx.
	//   -- Any other value also gives a severe error.
	//
	// ZRXMSG - message id used to set .MSG when ZRXRC = 8 (PSYE034N if blank) or ZERRMSG if a severe error (PSYE032M if blank).
	//

	TRACE_FUNCTION() ;

	int i ;
	int j ;
	int zrxrc ;

	string w ;
	string msg ;
	string tmp ;

	string rxname ;

	const string msg_source = "panel" ;

	set<string> vars ;
	set<string>* vars_ptr = &prexx->vars ;

	tempfile tfile( "lspf-inlrexx" ) ;

	err.setRC( 0 ) ;

	if ( prexx->vars_variable() )
	{
		for ( auto& var : prexx->vars )
		{
			if ( var.front() == '&' )
			{
				tmp = getDialogueVar( err, var.substr( 1 ) ) ;
				if ( err.error() ) { return ; }
				replace( tmp.begin(), tmp.end(), ',', ' ' ) ;
				iupper( tmp ) ;
				for ( i = 1, j = words( tmp ) ; i <= j ; ++i )
				{
					w = word( tmp, i ) ;
					if ( !isvalidName( w ) )
					{
						err.seterrid( TRACE_INFO(), "PSYE013A", "*REXX statement", w ) ;
						return ;
					}
					vars.insert( w ) ;
				}
			}
			else
			{
				vars.insert( var ) ;
			}
		}
		vars_ptr = &vars ;
	}

	exitinfo1.vars    = vars_ptr ;
	exitinfo1.err     = &err ;
	exitinfo1.type    = 'R'  ;
	exitinfo1.panelid = panelid ;
	exitinfo1.panel   = (void *)this ;
	exitinfo1.pAppl   = pAppl ;

	if ( prexx->rexx_inline() )
	{
		tfile.open() ;
		auto it1 = rexx_inlines.find( prexx ) ;
		for ( auto it2 = it1->second.begin() ; it2 != it1->second.end() ; ++it2 )
		{
			tfile << *it2 << endl ;
		}
		tfile.close() ;
		rxname = tfile.name() ;
	}
	else
	{
		rxname = ( prexx->rexx_variable() ) ? getDialogueVar( err, prexx->rexx ) : prexx->rexx ;
		if ( rxname == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE034O", prexx->rexx ) ;
			return ;
		}
		if ( rxname.front() == '%' ) { rxname.erase( 0, 1 ) ; }
	}

	if ( !instance )
	{
		start_rexx_interpreter( err ) ;
		if ( err.error() ) { return ; }
	}

	args = threadContext->NewArray( 1 ) ;
	threadContext->CallProgram( rxname.c_str(), args ) ;

	if ( threadContext->CheckCondition() )
	{
		cond = threadContext->GetConditionInfo() ;
		threadContext->DecodeConditionInfo( cond, &condition ) ;
		llog( "E", "*REXX error running REXX. .: " << ( ( prexx->rexx_inline() ) ? "*inline*" : rxname ) << endl ) ;
		llog( "E", "   Condition Code . . . . .: " << condition.code << endl ) ;
		llog( "E", "   Condition Error Text . .: " << threadContext->CString( condition.errortext ) << endl ) ;
		llog( "E", "   Condition Message. . . .: " << threadContext->CString( condition.message ) << endl ) ;
		llog( "E", "   Line Error Occured . . .: " << condition.position << endl ) ;
		if ( prexx->rexx_inline() )
		{
			size_t i = 1 ;
			llog( "E", "***** Failing inline REXX code **************************************************************************" << endl ) ;
			auto it1 = rexx_inlines.find( prexx ) ;
			for ( auto it2 = it1->second.begin() ; it2 != it1->second.end() ; ++it2, ++i )
			{
				llog( "-", d2ds( i, 5 ) << ( ( condition.position == i ) ? "*> " : " | " ) << *it2 << endl ) ;
			}
			llog( "E", "***** End of REXX code **********************************************************************************" << endl ) ;
		}
		if ( condition.code == Rexx_Error_Program_unreadable_notfound )
		{
			err.seterrid( TRACE_INFO(), "PSYE034L", msg_source, rxname ) ;
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE034M" ) ;
		}
		instance->Terminate() ;
		instance = nullptr ;
		return ;
	}

	if ( err.error() )
	{
		instance->Terminate() ;
		instance = nullptr ;
		return ;
	}

	if ( !datatype( exitinfo1.zrxrc, 'W' ) )
	{
		msg = ( exitinfo1.zrxmsg == "" ) ? "PSYE034M" : exitinfo1.zrxmsg ;
		putDialogueVar( err, "ZERRMSG", msg ) ;
		if ( err.error() ) { return ; }
		err.seterrid( TRACE_INFO(), msg ) ;
		llog( "E", "Panel REXX returned a non-numeric value for ZRXRC.  Value=" << exitinfo1.zrxrc << endl ) ;
	}
	else
	{
		zrxrc = ds2d( exitinfo1.zrxrc ) ;
		if ( zrxrc != 0 && zrxrc != 8 )
		{
			msg = ( exitinfo1.zrxmsg == "" ) ? "PSYE034M" : exitinfo1.zrxmsg ;
			putDialogueVar( err, "ZERRMSG", msg ) ;
			if ( err.error() ) { return ; }
			err.seterrid( TRACE_INFO(), msg ) ;
			llog( "E", "Panel REXX returned a value for ZRXRC that is not 0 or 8.  Value=" << exitinfo1.zrxrc << endl ) ;
		}
		else if ( zrxrc == 8 )
		{
			set_message_cond( ( exitinfo1.zrxmsg == "" ) ? "PSYE034N" : exitinfo1.zrxmsg ) ;
		}
	}

	instance->Terminate() ;
	instance = nullptr ;
}


void pPanel::process_panel_panexit( errblock& err,
				    PANEXIT* pexit,
				    PS_SECT ps_sect )
{
	//
	// Process PANEXIT statement.
	//

	TRACE_FUNCTION() ;

	if ( pexit->is_rexx )
	{
		process_panel_panexit_rexx( err, pexit, ps_sect ) ;
	}
	else
	{
		process_panel_panexit_load( err, pexit, ps_sect ) ;
	}
}


void pPanel::process_panel_panexit_load( errblock& err,
					 PANEXIT* pexit,
					 PS_SECT ps_sect )
{
	//
	// Process PANEXIT statement for LOAD.
	//
	//   Return codes:
	//    0 Normal completion.
	//    8 Panel LOAD defined failure.  lspf sets .MSG and displays/redisplays panel.
	//   20 Severe error in the PANEXIT module.
	//   -- Any other value also gives a severe error.
	//
	// The module is located using the LIBDEF ISPLLIB.
	//
	// MSGID - message id used to set .MSG when RC = 8 (MSG= or PSYE038X if blank) or ZERRMSG if a severe error (MSG= or PSYE038Y if blank).
	//
	// On exit, MSGID is contained in msgid and the LOAD exit value is the go() return value.
	//

	TRACE_FUNCTION() ;

	int i ;
	int j ;
	int rc ;

	void* maker ;
	void* destroyer ;

	string w ;
	string tmp ;
	string msg ;

	panload* panLoad ;

	vector<string> vars ;
	vector<string> vals ;

	if ( pexit->vars_variable() )
	{
		for ( auto& var : pexit->vars )
		{
			if ( var.front() == '&' )
			{
				tmp = getDialogueVar( err, var.substr( 1 ) ) ;
				if ( err.error() ) { return ; }
				replace( tmp.begin(), tmp.end(), ',', ' ' ) ;
				iupper( tmp ) ;
				for ( i = 1, j = words( tmp ) ; i <= j ; ++i )
				{
					w = word( tmp, i ) ;
					if ( !isvalidName( w ) )
					{
						err.seterrid( TRACE_INFO(), "PSYE013A", "PANEXIT statement", w ) ;
						return ;
					}
					vars.push_back( w ) ;
				}
			}
			else
			{
				vars.push_back( var ) ;
			}
		}
	}
	else for ( auto& var : pexit->vars )
	{
		vars.push_back( var ) ;
	}

	for ( auto& var : vars )
	{
		tmp = getDialogueVar( err, var ) ;
		if ( err.error() ) { return ; }
		vals.push_back( tmp ) ;
	}

	string ldname = ( pexit->rexx_variable() ) ? getDialogueVar( err, pexit->rexx ) : pexit->rexx ;

	pApplication* appl = static_cast<pApplication*>(pAppl) ;

	locator loc( appl->get_libdef_search_paths( s_ZLLIB ), ldname, ".so" ) ;

	loc.locate() ;

	if ( loc.not_found() )
	{
		if ( loc.errors() )
		{
			err.seterrid( TRACE_INFO(), loc.msgid(), loc.mdata(), 20 ) ;
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE038V", loc.name(), 20 ) ;
		}
		return ;
	}

	dynloader loader( loc.entry() ) ;

	loader.open() ;
	if ( loader.errors() )
	{
		err.seterrid( TRACE_INFO(), "PSYE038W", "open", ldname, loader.errmsg(), 20 ) ;
		return ;
	}

	maker = loader.lookup( "pan_make" ) ;
	if ( loader.errors() )
	{
		err.seterrid( TRACE_INFO(), "PSYE038W", "lookup", "symbol pan_make", loader.errmsg(), 20 ) ;
		return ;
	}

	destroyer = loader.lookup( "pan_destroy" ) ;
	if ( loader.errors() )
	{
		err.seterrid( TRACE_INFO(), "PSYE038W", "lookup", "symbol pan_destroy", loader.errmsg(), 20 ) ;
		return ;
	}

	exitinfo2.msgid    = "" ;
	exitinfo2.numvars  = vars.size() ;
	exitinfo2.exdata   = ( pexit->exdata != "" ) ? getDialogueVar( err, pexit->exdata ) : "" ;
	exitinfo2.vars     = vars ;
	exitinfo2.vals     = &vals ;
	exitinfo2.panelid  = panelid ;
	exitinfo2.psection = ( ps_sect == PS_INIT   ) ? "I" :
			     ( ps_sect == PS_REINIT ) ? "R" : "P" ;

	panLoad = (( panload*(*)() )( maker ))() ;

	rc = panLoad->go( exitinfo2 ) ;

	(( void(*)( panload*) )( destroyer ))( panLoad ) ;

	if ( rc == 8 )
	{
		set_message_cond( err, ( exitinfo2.msgid == "" ) ?
				       ( pexit->msgid    == "" ) ? "PSYE038X" : pexit->msgid : exitinfo2.msgid, ldname ) ;
	}
	else if ( rc != 0 )
	{
		msg = ( exitinfo2.msgid == "" ) ?
		      ( pexit->msgid    == "" ) ? "PSYE038Y" : pexit->msgid : exitinfo2.msgid ;
		putDialogueVar( err, "ZERRMSG", msg ) ;
		if ( err.error() ) { return ; }
		err.seterrid( TRACE_INFO(), msg, ldname ) ;
		llog( "E", "PANEXIT LOAD routine returned a value that is not 0 or 8.  Value=" << rc << endl ) ;
		return ;
	}

	for ( uint i = 0 ; i < vars.size() && i < vals.size() ; ++i )
	{
		putDialogueVar( err, vars[ i ], vals[ i ] ) ;
		if ( err.error() ) { return ; }
	}
}



void pPanel::process_panel_panexit_rexx( errblock& err,
					 PANEXIT* pexit,
					 PS_SECT ps_sect )
{
	//
	// Process PANEXIT statement for REXX.
	//
	//   RESULT:
	//    0 Normal completion.
	//    8 Panel REXX-defined failure.  lspf sets .MSG and displays/redisplays panel.
	//   20 Severe error in the PANEXIT rexx.
	//   -- Any other value also gives a severe error.
	//
	// MSGID - message id used to set .MSG when RESULT = 8 (MSG= or PSYE038T if blank) or ZERRMSG if a severe error (MSG= or PSYE038S if blank).
	//
	// On exit, MSGID is contained in zrxmsg and the REXX exit/return value is in zrxrc.
	//

	TRACE_FUNCTION() ;

	int i ;
	int j ;
	int zrxrc = 0 ;

	string w ;
	string msg ;
	string tmp ;

	string rxname ;

	const string msg_source = "panel" ;

	set<string> vars ;
	set<string>* vars_ptr = &pexit->vars ;

	RexxObjectPtr result ;

	err.setRC( 0 ) ;

	if ( pexit->vars_variable() )
	{
		for ( auto& var : pexit->vars )
		{
			if ( var.front() == '&' )
			{
				tmp = getDialogueVar( err, var.substr( 1 ) ) ;
				if ( err.error() ) { return ; }
				replace( tmp.begin(), tmp.end(), ',', ' ' ) ;
				iupper( tmp ) ;
				for ( i = 1, j = words( tmp ) ; i <= j ; ++i )
				{
					w = word( tmp, i ) ;
					if ( !isvalidName( w ) )
					{
						err.seterrid( TRACE_INFO(), "PSYE013A", "*REXX statement", w ) ;
						return ;
					}
					vars.insert( w ) ;
				}
			}
			else
			{
				vars.insert( var ) ;
			}
		}
		vars_ptr = &vars ;
	}

	exitinfo1.vars     = vars_ptr ;
	exitinfo1.err      = &err ;
	exitinfo1.type     = 'P'  ;
	exitinfo1.panelid  = panelid ;
	exitinfo1.exdata   = ( pexit->exdata != "" ) ? getDialogueVar( err, pexit->exdata ) : "" ;
	exitinfo1.panel    = (void *)this ;
	exitinfo1.pAppl    = pAppl ;
	exitinfo1.psection = ( ps_sect == PS_INIT   ) ? "I" :
			     ( ps_sect == PS_REINIT ) ? "R" : "P" ;

	rxname = ( pexit->rexx_variable() ) ? getDialogueVar( err, pexit->rexx ) : pexit->rexx ;
	if ( rxname == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE034O", pexit->rexx ) ;
		return ;
	}

	if ( rxname.front() == '%' ) { rxname.erase( 0, 1 ) ; }

	if ( !instance )
	{
		start_rexx_interpreter( err ) ;
		if ( err.error() ) { return ; }
	}

	args   = threadContext->NewArray( 1 ) ;
	result = threadContext->CallProgram( rxname.c_str(), args ) ;

	if ( threadContext->CheckCondition() )
	{
		cond = threadContext->GetConditionInfo() ;
		threadContext->DecodeConditionInfo( cond, &condition ) ;
		llog( "E", "PANEXIT error running REXX.: " << rxname << endl ) ;
		llog( "E", "   Condition Code . . . . .: " << condition.code << endl ) ;
		llog( "E", "   Condition Error Text . .: " << threadContext->CString( condition.errortext ) << endl ) ;
		llog( "E", "   Condition Message. . . .: " << threadContext->CString( condition.message ) << endl ) ;
		llog( "E", "   Line Error Occured . . .: " << condition.position << endl ) ;
		if ( condition.code == Rexx_Error_Program_unreadable_notfound )
		{
			err.seterrid( TRACE_INFO(), "PSYE034L", msg_source, rxname ) ;
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE038S" ) ;
		}
	}
	else if ( err.ok() && result != NULLOBJECT )
	{
		w = threadContext->CString( result ) ;
		zrxrc = ( datatype( w, 'W' ) ) ? ds2d( w ) : 20 ;
		if ( zrxrc != 0 && zrxrc != 8 )
		{
			msg = ( exitinfo1.zrxmsg == "" ) ?
			      ( pexit->msgid     == "" ) ? "PSYE038S" : pexit->msgid : exitinfo1.zrxmsg ;
			putDialogueVar( err, "ZERRMSG", msg ) ;
			if ( err.error() )
			{
				instance->Terminate() ;
				instance = nullptr ;
				return ;
			}
			err.seterrid( TRACE_INFO(), msg ) ;
			llog( "E", "PANEXIT REXX routine returned a value that is not 0 or 8.  Value=" << w << endl ) ;
		}
		else if ( zrxrc == 8 )
		{
			set_message_cond( ( exitinfo1.zrxmsg == "" ) ?
					  ( pexit->msgid     == "" ) ? "PSYE038T" : pexit->msgid : exitinfo1.zrxmsg ) ;
		}
	}

	instance->Terminate() ;
	instance = nullptr ;
}


void pPanel::start_rexx_interpreter( errblock& err )
{
	//
	// Start the OOREXX interpreter.
	//
	// Setup REXX initialisation exit RXINI and termination exit RXTER to handle
	// setting REXX/lspf variables before and after REXX program execution.
	//
	// Setup REXX exit RXSIO (subfunction RXSIOSAY and RXSIOTRC) to redirect say/trace output to the rdisplay() method
	// and RXSIOTRD to call the pull() method for the rexx PULL/ARG PULL statements and
	// RXSIODTR to call the pull() method for interactive debug input.
	//

	TRACE_FUNCTION() ;

	pApplication* appl = static_cast<pApplication*>(pAppl) ;
	rxpath             = appl->sysexec() ;

	options[ 0 ].optionName = APPLICATION_DATA ;
	options[ 0 ].option     = (void*)&exitinfo1 ;
	options[ 1 ].optionName = DIRECT_EXITS ;
	options[ 1 ].option     = (void*)exits ;
	options[ 2 ].optionName = EXTERNAL_CALL_PATH ;
	options[ 2 ].option     = rxpath.c_str() ;
	options[ 3 ].optionName = "" ;

	exits[ 0 ].sysexit_code = RXINI ;
	exits[ 0 ].handler      = rxiniExit_panel1 ;
	exits[ 1 ].sysexit_code = RXTER ;
	exits[ 1 ].handler      = rxterExit_panel1 ;
	exits[ 2 ].sysexit_code = RXSIO ;
	exits[ 2 ].handler      = rxsioExit_panel1 ;
	exits[ 3 ].sysexit_code = RXENDLST ;
	exits[ 3 ].handler      = nullptr ;

	if ( !RexxCreateInterpreter( &instance, &threadContext, options ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE034T" ) ;
	}
}


int REXXENTRY rxiniExit_panel1( RexxExitContext* context,
				int exitnumber,
				int subfunction,
				PEXIT parmblock )

{
	//
	// OOREXX initialisation exit to set REXX variables at the start of program execution.
	// Also set special variables ZRXRC and ZRXMSG in the REXX variable pool.
	//
	// type = 'R' when called for *REXX statement.
	// type = 'P' when called for PANEXIT statement.
	//
	// If the function pool variable has been defined as an integer, always return character format.
	//

	void* vptr ;

	string val ;
	string name ;

	vptr = context->GetApplicationData() ;

	exitInfo1* exitinfo1 = static_cast<exitInfo1*>(vptr) ;

	pPanel* panel = static_cast<pPanel*>(exitinfo1->panel) ;

	if ( exitinfo1->type == 'R' )
	{
		context->SetContextVariable( "ZRXRC", context->String( "0" ) ) ;
		context->SetContextVariable( "ZRXMSG", context->String( "" ) ) ;
	}
	else
	{
		context->SetContextVariable( "PANELNAME", context->String( exitinfo1->panelid.c_str() ) ) ;
		context->SetContextVariable( "PANELSECTION", context->String( exitinfo1->psection.c_str() ) ) ;
		context->SetContextVariable( "VARNAMES.0", context->String( d2ds( exitinfo1->vars->size(), 8 ).c_str() ) ) ;
		context->SetContextVariable( "VARVALS.0", context->String( d2ds( exitinfo1->vars->size(), 8 ).c_str() ) ) ;
		context->SetContextVariable( "VARLENS.0", context->String( d2ds( exitinfo1->vars->size(), 8 ).c_str() ) ) ;
		context->SetContextVariable( "MSGID", context->String( "" ) ) ;
		context->SetContextVariable( "EXDATA", context->String( exitinfo1->exdata.c_str(), exitinfo1->exdata.size() ) ) ;
		int i = 1 ;
		for ( auto& var : *exitinfo1->vars )
		{
			name = "VARNAMES."+d2ds( i ) ;
			context->SetContextVariable( name.c_str(), context->String( var.c_str() ) ) ;
			name = "VARVALS."+d2ds( i ) ;
			val = panel->getDialogueVar( *exitinfo1->err, var ) ;
			context->SetContextVariable( name.c_str(), context->String( val.c_str(), val.size() ) ) ;
			name = "VARLENS."+d2ds( i ) ;
			context->SetContextVariable( name.c_str(), context->String( d2ds( val.size(), 5 ).c_str() ) ) ;
			++i ;
		}
	}

	for ( auto& var : *exitinfo1->vars )
	{
		val = panel->getDialogueVar( *exitinfo1->err, var ) ;
		if ( exitinfo1->err->error() ) { return RXEXIT_HANDLED ; }
		context->SetContextVariable( var.c_str(), context->String( val.c_str(), val.size() ) ) ;
	}

	return RXEXIT_HANDLED ;
}


int REXXENTRY rxterExit_panel1( RexxExitContext* context,
				int exitnumber,
				int subfunction,
				PEXIT parmblock )

{
	//
	// OOREXX termination exit to set dialogue variables at the end of program execution.
	// Also set special variables ZRXRC and ZRXMSG in the lspf function pool.
	//
	// type = 'R' when called for *REXX statement.
	// type = 'P' when called for PANEXIT statement.
	//
	// Any passed variable dropped in the REXX procedure will have its value set to its name.
	//
	// If the function pool variable has been defined as an integer, convert from character data.
	//

	void* vptr ;

	string val ;

	RexxObjectPtr optr ;
	RexxStringObject str ;

	vptr = context->GetApplicationData() ;

	exitInfo1* exitinfo1 = static_cast<exitInfo1*>(vptr) ;

	pPanel* panel = static_cast<pPanel*>(exitinfo1->panel) ;

	for ( auto& var : *exitinfo1->vars )
	{
		optr = context->GetContextVariable( var.c_str() ) ;
		if ( optr == NULLOBJECT )
		{
			val = var ;
		}
		else
		{
			str = context->ObjectToString( optr ) ;
			val = string( context->StringData( str ), context->StringLength( str ) ) ;
		}
		panel->putDialogueVar( *exitinfo1->err, var, val ) ;
		if ( exitinfo1->err->error() ) { return RXEXIT_HANDLED ; }
	}

	if ( exitinfo1->type == 'R' )
	{
		optr = context->GetContextVariable( "ZRXRC" ) ;
		exitinfo1->zrxrc = ( optr != NULLOBJECT ) ? context->CString( optr ) : "ZRXRC" ;
		panel->putDialogueVar( *exitinfo1->err, "ZRXRC", exitinfo1->zrxrc ) ;
		if ( exitinfo1->err->error() ) { return RXEXIT_HANDLED ; }
		optr = context->GetContextVariable( "ZRXMSG" ) ;
		exitinfo1->zrxmsg = ( optr != NULLOBJECT ) ? context->CString( optr ) : "ZRXMSG" ;
		panel->putDialogueVar( *exitinfo1->err, "ZRXMSG", exitinfo1->zrxmsg ) ;
		if ( exitinfo1->err->error() ) { return RXEXIT_HANDLED ; }
	}
	else
	{
		optr = context->GetContextVariable( "MSGID" ) ;
		exitinfo1->zrxmsg = ( optr != NULLOBJECT ) ? context->CString( optr ) : "MSGID" ;
		panel->putDialogueVar( *exitinfo1->err, "MSGID", exitinfo1->zrxmsg ) ;
	}

	return RXEXIT_HANDLED ;
}


int REXXENTRY rxsioExit_panel1( RexxExitContext* context,
				int exitnumber,
				int subfunction,
				PEXIT ParmBlock )

{
	//
	// REXX exit RXSIO.  Handle subfunction RXSIOSAY and RXSIOTRC to call rdisplay() method for say/trace output.
	//                   Handle subfunction RXSIOTRD to call pull() method to get user data for pull/arg pull requests.
	//                   Handle subfunction RXSIODTR to call pull() method for interactive debug input.
	//

	void* vptr ;

	switch ( subfunction )
	{
	case RXSIOSAY:
	case RXSIOTRC:
	{
		vptr = context->GetApplicationData() ;
		exitInfo1* exitinfo1    = static_cast<exitInfo1*>(vptr) ;
		pApplication* appl      = static_cast<pApplication*>(exitinfo1->pAppl) ;
		RXSIOSAY_PARM* say_parm = (RXSIOSAY_PARM *)ParmBlock ;
		appl->rdisplay( string( say_parm->rxsio_string.strptr, say_parm->rxsio_string.strlength ), false ) ;
		return RXEXIT_HANDLED ;
	}

	case RXSIOTRD:
	{
		string ans ;
		vptr = context->GetApplicationData() ;
		exitInfo1* exitinfo1    = static_cast<exitInfo1*>(vptr) ;
		pApplication* appl      = static_cast<pApplication*>(exitinfo1->pAppl) ;
		RXSIOTRD_PARM* trd_parm = (RXSIOTRD_PARM *)ParmBlock ;
		appl->pull( &ans ) ;
		if ( ans.size() >= trd_parm->rxsiotrd_retc.strlength )
		{
			trd_parm->rxsiotrd_retc.strptr = (char *)RexxAllocateMemory( ans.size() + 1 ) ;
		}
		strcpy( trd_parm->rxsiotrd_retc.strptr, ans.c_str() ) ;
		trd_parm->rxsiotrd_retc.strlength = ans.size() ;
		return RXEXIT_HANDLED ;
	}

	case RXSIODTR:
	{
		string ans ;
		vptr = context->GetApplicationData() ;
		exitInfo1* exitinfo1    = static_cast<exitInfo1*>(vptr) ;
		pApplication* appl      = static_cast<pApplication*>(exitinfo1->pAppl) ;
		RXSIODTR_PARM* dtr_parm = (RXSIODTR_PARM *)ParmBlock ;
		appl->pull( &ans ) ;
		if ( ans.size() >= dtr_parm->rxsiodtr_retc.strlength )
		{
			dtr_parm->rxsiodtr_retc.strptr = (char *)RexxAllocateMemory( ans.size() + 1 ) ;
		}
		strcpy( dtr_parm->rxsiodtr_retc.strptr, ans.c_str() ) ;
		dtr_parm->rxsiodtr_retc.strlength = ans.size() ;
		return RXEXIT_HANDLED ;
	}
	}

	return RXEXIT_NOT_HANDLED ;
}


#else
void pPanel::process_panel_prexx( errblock& err,
				  PREXX* prexx )
{
	//
	// Dummy routine for when REXX has not been enabled.
	//
}


void pPanel::process_panel_panexit( errblock& err,
				    PANEXIT* pexit,
				    PS_SECT ps_sect )
{
	//
	// Dummy routine for when REXX has not been enabled.
	//
}
#endif


void pPanel::process_panel_verify( errblock& err,
				   int ln,
				   VERIFY* verify )
{
	TRACE_FUNCTION() ;

	int j  ;
	int r1 ;
	int r2 ;

	uint i ;

	bool error ;

	string t1  ;
	string t2  ;
	string msg ;
	string temp ;
	string errtext ;
	string fieldNam ;
	string fieldVal ;

	char   pictcn1 ;
	string pictcn2 ;
	string pictcn3 ;

	const string picts = "CAN9Xcanx" ;

	struct sockaddr_in sa ;

	vector<string> qw ;

	vector<string>::iterator it ;

	fieldNam = verify->ver_var ;
	fieldVal = getDialogueVar( err, verify->ver_var ) ;
	if ( err.error() ) { return ; }

	trim_right( fieldVal ) ;

	if ( verify->ver_nblank )
	{
		if ( fieldVal == "" )
		{
			set_message_cond( ( verify->ver_msgid == "" ) ? "PSYS019" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
	}

	if ( fieldVal == "" && verify->ver_type != VER_LEN )
	{
		return ;
	}

	switch ( verify->ver_type )
	{
	case VER_ALPHA:
		if ( !isalpha( fieldVal ) )
		{
			set_message_cond( ( verify->ver_msgid == "" ) ? "PSYE052D" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_ALPHAB:
		if ( !isalphab( fieldVal ) )
		{
			set_message_cond( ( verify->ver_msgid == "" ) ? "PSYE052E" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_BIT:
		if ( !isbit( fieldVal ) )
		{
			set_message_cond( ( verify->ver_msgid == "" ) ? "PSYE052F" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_ENUM:
		trim_left( fieldVal ) ;
		isenum( err, fieldVal ) ;
		if ( err.getRC() > 0 )
		{
			set_message_cond( ( verify->ver_msgid == "" ) ? err.geterrid() : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln, err.getcsrpos() ) ;
			}
		}
		break ;

	case VER_HEX:
		if ( !ishex( fieldVal ) )
		{
			set_message_cond( ( verify->ver_msgid == "" ) ? "PSYS011H" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_INCLUDE:
		if ( ( verify->ver_include & INC_IMBLK ) == INC_IMBLK )
		{
			fieldVal.erase( std::remove( fieldVal.begin(), fieldVal.end(), ' ' ), fieldVal.end() ) ;
			errtext = "Value can also include imbedded blank characters." ;
		}
		if ( ( verify->ver_include & INC_ALPHANUM ) == INC_ALPHANUM )
		{
			if ( !isalphanum( fieldVal ) )
			{
				set_message_cond( err, ( verify->ver_msgid == "" ) ? "PSYE052G" : sub_vars( verify->ver_msgid ), errtext ) ;
				if ( err.error() ) { return ; }
				if ( verify->ver_pnfield )
				{
					set_cursor_cond( fieldNam, ln ) ;
				}
			}
		}
		else if ( ( verify->ver_include & INC_ALPHABNUM ) == INC_ALPHABNUM )
		{
			if ( !isalphabnum( fieldVal ) )
			{
				set_message_cond( err, ( verify->ver_msgid == "" ) ? "PSYE052H" : sub_vars( verify->ver_msgid ), errtext ) ;
				if ( err.error() ) { return ; }
				if ( verify->ver_pnfield )
				{
					set_cursor_cond( fieldNam, ln ) ;
				}
			}
		}
		else if ( ( verify->ver_include & INC_ALPHA ) == INC_ALPHA )
		{
			if ( !isalpha( fieldVal ) )
			{
				set_message_cond( err, ( verify->ver_msgid == "" ) ? "PSYE052I" : sub_vars( verify->ver_msgid ), errtext ) ;
				if ( err.error() ) { return ; }
				if ( verify->ver_pnfield )
				{
					set_cursor_cond( fieldNam, ln ) ;
				}
			}
		}
		else if ( ( verify->ver_include & INC_ALPHAB ) == INC_ALPHAB )
		{
			if ( !isalphab( fieldVal ) )
			{
				set_message_cond( err, ( verify->ver_msgid == "" ) ? "PSYE052J" : sub_vars( verify->ver_msgid ), errtext ) ;
				if ( err.error() ) { return ; }
				if ( verify->ver_pnfield )
				{
					set_cursor_cond( fieldNam, ln ) ;
				}
			}
		}
		else
		{
			trim_left( fieldVal ) ;
			if ( !isnumeric( fieldVal ) )
			{
				set_message_cond( err, ( verify->ver_msgid == "" ) ? "PSYE052K" : sub_vars( verify->ver_msgid ), errtext ) ;
				if ( err.error() ) { return ; }
				if ( verify->ver_pnfield )
				{
					set_cursor_cond( fieldNam, ln ) ;
				}
			}
		}
		break ;

	case VER_IPADDR4:
		if ( inet_pton( AF_INET, fieldVal.c_str(), &(sa.sin_addr) ) != 1 )
		{
			set_message_cond( ( verify->ver_msgid == "" ) ? "PSYE052T" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_IPADDR6:
		if ( inet_pton( AF_INET6, fieldVal.c_str(), &(sa.sin_addr) ) != 1 )
		{
			set_message_cond( ( verify->ver_msgid == "" ) ? "PSYE052U" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_LEN:
		if ( verify->ver_vlist.empty() )
		{
			i = verify->ver_len ;
		}
		else
		{
			t1 = getDialogueVar( err, verify->ver_vlist[ 0 ] ) ;
			if ( err.error() ) { return ; }
			if ( !datatype( t1, 'W' ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE052A", t1, "verify LEN" ) ;
				return ;
			}
			i = ds2d( t1 ) ;
		}
		error = true ;
		switch ( verify->ver_cond )
		{
		case VL_EQ:
			if ( fieldVal.size() == i )
			{
				error = false ;
			}
			else
			{
				t1 = "equal to" ;
			}
			break ;

		case VL_NE:
			if ( fieldVal.size() != i )
			{
				error = false ;
			}
			else
			{
				t1 = "not equal to" ;
			}
			break ;

		case VL_GT:
			if ( fieldVal.size() > i )
			{
				error = false ;
			}
			else
			{
				t1 = "greater than" ;
			}
			break ;

		case VL_GE:
			if ( fieldVal.size() >= i )
			{
				error = false ;
			}
			else
			{
				t1 = "greater than or equal to" ;
			}
			break ;

		case VL_LE:
			if ( fieldVal.size() <= i )
			{
				error = false ;
			}
			else
			{
				t1 = "less than or equal to" ;
			}
			break ;

		case VL_LT:
			if ( fieldVal.size() < i )
			{
				error = false ;
			}
			else
			{
				t1 = "less than" ;
			}
			break ;

		}
		if ( error )
		{
			set_zzstr_shared( err, t1, d2ds( i ) ) ;
			if ( err.error() ) { return ; }
			set_message_cond( ( verify->ver_msgid == "" ) ? "PSYS013P" : sub_vars( verify->ver_msgid ) ) ;
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
			if ( strip_right( sub_vars( *it ) ) == fieldVal ) { break ; }
		}
		if ( ( verify->ver_type == VER_LIST  && it == verify->ver_vlist.end() ) ||
		     ( verify->ver_type == VER_LISTX && it != verify->ver_vlist.end() ) )
		{
			if ( verify->ver_vlist.size() == 1 )
			{
				t1 = "value is " ;
				t1 += sub_vars( verify->ver_vlist[ 0 ] ) ;
			}
			else
			{
				t1 = "values are " ;
				for ( i = 0 ; i < verify->ver_vlist.size() - 2 ; ++i )
				{
					t1 += sub_vars( verify->ver_vlist[ i ] ) + ", " ;
				}
				t1 += sub_vars( verify->ver_vlist[ i ] ) ;
				t1 += " and " ;
				++i ;
				t1 += sub_vars( verify->ver_vlist[ i ] ) ;
			}
			msg = ( verify->ver_type == VER_LIST ) ? "PSYS011B" : "PSYS012Q" ;
			set_message_cond( ( verify->ver_msgid == "" ) ? msg : sub_vars( verify->ver_msgid ) ) ;
			if ( dMsg == msg )
			{
				set_zzstr_shared( err, t1 ) ;
				if ( err.error() ) { return ; }
			}
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_LISTV:
	case VER_LISTVX:
		temp = getDialogueVar( err, verify->ver_vlist.front() ) ;
		if ( err.error() ) { return ; }
		getlist( err, temp, qw ) ;
		if ( err.error() || qw.size() == 0 ) { return ; }
		for ( it = qw.begin() ; it != qw.end() ; ++it )
		{
			if ( strip_right( sub_vars( *it ) ) == fieldVal ) { break ; }
		}
		if ( ( verify->ver_type == VER_LISTV  && it == qw.end() ) ||
		     ( verify->ver_type == VER_LISTVX && it != qw.end() ) )
		{
			if ( qw.size() == 1 )
			{
				t1 = "value is " ;
				t1 += sub_vars( qw[ 0 ] ) ;
			}
			else
			{
				t1 = "values are " ;
				for ( i = 0 ; i < qw.size() - 2 ; ++i )
				{
					t1 += sub_vars( qw[ i ] ) + ", " ;
				}
				t1 += sub_vars( qw[ i ] ) ;
				t1 += " and " ;
				++i ;
				t1 += sub_vars( qw[ i ] ) ;
			}
			msg = ( verify->ver_type == VER_LISTV ) ? "PSYS011B" : "PSYS012Q" ;
			set_message_cond( ( verify->ver_msgid == "" ) ? msg : sub_vars( verify->ver_msgid ) ) ;
			if ( dMsg == msg )
			{
				set_zzstr_shared( err, t1 ) ;
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
			set_message_cond( ( verify->ver_msgid == "" ) ? "PSYS012U" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_NUMERIC:
		trim_left( fieldVal ) ;
		if ( !isnumeric( fieldVal ) )
		{
			set_message_cond( ( verify->ver_msgid == "" ) ? "PSYS011A" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_OCT:
		if ( !isoctal( fieldVal ) )
		{
			set_message_cond( ( verify->ver_msgid == "" ) ? "PSYS011F" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_PICT:
		if ( !ispict( fieldVal, verify->ver_vlist[ 0 ] ) )
		{
			set_zzstr_shared( err, d2ds( verify->ver_vlist[ 0 ].size() ), verify->ver_vlist[ 0 ] ) ;
			if ( err.error() ) { return ; }
			set_message_cond( ( verify->ver_msgid == "" ) ? "PSYS011N" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_PICTCN:
		temp = getVariable( err, verify->ver_vlist[ 0 ] ) ;
		if ( temp.size() != 1 )
		{
			err.seterrid( TRACE_INFO(), "PSYE052R" ) ;
			return ;
		}
		pictcn1 = temp[ 0 ] ;
		if ( picts.find( pictcn1 ) != string::npos )
		{
			err.seterrid( TRACE_INFO(), "PSYE052R" ) ;
			return ;
		}
		pictcn2 = getVariable( err, verify->ver_vlist[ 1 ] ) ;
		if ( pictcn2.find( pictcn1 ) == string::npos )
		{
			err.seterrid( TRACE_INFO(), "PSYE052S" ) ;
			return ;
		}
		pictcn3 = getVariable( err, verify->ver_vlist[ 2 ] ) ;
		ispictcn( err, fieldVal, pictcn1, pictcn2, pictcn3 ) ;
		if ( err.error() ) { return ; }
		if ( err.getRC() > 0 )
		{
			p_poolMGR->put( err, "ZVAL1", err.val1 ) ;
			if ( err.error() ) { return ; }
			p_poolMGR->put( err, "ZVAL2", err.val2 ) ;
			if ( err.error() ) { return ; }
			set_message_cond( ( verify->ver_msgid == "" ) ? err.geterrid() : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_RANGE:
		t1 = verify->ver_vlist[ 0 ] ;
		if ( !datatype( t1, 'W' ) )
		{
			t1 = getDialogueVar( err, t1 ) ;
			if ( err.error() ) { return ; }
			if ( !datatype( t1, 'W' ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE052A", verify->ver_vlist[ 0 ], "verify RANGE" ) ;
				return ;
			}
		}
		r1 = ds2d( t1 ) ;
		t2 = verify->ver_vlist[ 1 ] ;
		if ( !datatype( t2, 'W' ) )
		{
			t2 = getDialogueVar( err, t2 ) ;
			if ( err.error() ) { return ; }
			if ( !datatype( t2, 'W' ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE052A", verify->ver_vlist[ 1 ], "verify RANGE" ) ;
				return ;
			}
		}
		r2 = ds2d( t2 ) ;
		if ( !datatype( fieldVal, 'W' ) )
		{
			set_zzstr_shared( err, d2ds( r1 ), d2ds( r2 ) ) ;
			if ( err.error() ) { return ; }
			set_message_cond( ( verify->ver_msgid == "" ) ? "PSYS013N" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
			break ;
		}
		j = ds2d( fieldVal ) ;
		if ( j < r1 || j > r2 )
		{
			set_zzstr_shared( err, d2ds( r1 ), d2ds( r2 ) ) ;
			if ( err.error() ) { return ; }
			set_message_cond( ( verify->ver_msgid == "" ) ? "PSYS013O" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_STDDATE:
		if ( !check_stddate( fieldVal ) )
		{
			set_message_cond( ( verify->ver_msgid == "" ) ? "PSYE052B" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;

	case VER_STDTIME:
		if ( !check_stdtime( fieldVal ) )
		{
			set_message_cond( ( verify->ver_msgid == "" ) ? "PSYE052C" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				set_cursor_cond( fieldNam, ln ) ;
			}
		}
		break ;
	}
}


bool pPanel::check_stddate( const string& s )
{
	TRACE_FUNCTION() ;

	char sep = zdatef[ 2 ] ;

	string yy ;
	string mm ;
	string dd ;

	if ( s.size() != 10 )
	{
		return false ;
	}

	if ( zdatef.compare( 0, 2, "DD" ) == 0 )
	{
		dd = s.substr( 0, 2 ) ;
		mm = s.substr( 3, 2 ) ;
		yy = s.substr( 6, 4 ) ;
		if ( s[ 2 ] != sep || s[ 5 ] != sep )
		{
			return false ;
		}
	}
	else
	{
		yy = s.substr( 0, 4 ) ;
		mm = s.substr( 5, 2 ) ;
		dd = s.substr( 8, 2 ) ;
		if ( s[ 4 ] != sep || s[ 7 ] != sep )
		{
			return false ;
		}
	}

	return check_date( yy, mm, dd ) ;
}


bool pPanel::check_stdtime( const string& s )
{
	TRACE_FUNCTION() ;

	string hh ;
	string mm ;
	string ss ;

	if ( s.size() != 8 || s[ 2 ] != ':' || s[ 5 ] != ':' ) { return false ; }

	hh = s.substr( 0, 2 ) ;
	mm = s.substr( 3, 2 ) ;
	ss = s.substr( 6, 2 ) ;

	if ( !isnumeric( hh ) ||
	     !isnumeric( mm ) ||
	     !isnumeric( ss ) )
	{
		return false ;
	}

	return ( ds2d( hh ) < 24 && ds2d( mm ) < 60 && ds2d( ss ) < 60 ) ;
}


void pPanel::process_panel_vput( errblock& err,
				 VPUTGET* vput )
{
	//
	// For select panels: VPUT to SHARED  copies from PROFILE to SHARED (or null if not found in either).
	//                    VPUT to PROFILE copies from SHARED to PROFILE (or null) and deletes SHARED.
	//                    BUG: VPUT has been a bit of a guess - probably not correct!
	//

	TRACE_FUNCTION() ;

	int j  ;
	int ws ;

	string var ;
	string val ;
	string mask ;

	fVAR* pvar ;

	VEDIT_TYPE vtype ;

	ws = words( vput->vpg_vars ) ;
	for ( j = 1 ; j <= ws ; ++j )
	{
		var = sub_vars( word( vput->vpg_vars, j ) ) ;
		if ( selPanel )
		{
			if ( vput->vpg_pool == SHARED )
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
			else if ( vput->vpg_pool == PROFILE )
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
			else if ( vput->vpg_pool == ASIS )
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
			pvar = funcPool->getfVAR( err, var ) ;
			if ( err.error() ) { return ; }
			if ( pvar )
			{
				val = pvar->sget( var ) ;
				if ( pvar->hasmask( mask, vtype ) )
				{
					add_vmask( val, zdatef, mask, vtype ) ;
				}
				p_poolMGR->put( err, var, val, vput->vpg_pool ) ;
				if ( err.error() ) { return ; }
			}
		}
	}
	if ( err.RC8() ) { err.setRC( 0 ) ; }
}


void pPanel::process_panel_vget( errblock& err,
				 VPUTGET* vget )
{
	//
	// For select panels: VGET from PROFILE deletes from SHARED, otherwise has no effect.
	//

	TRACE_FUNCTION() ;

	int j  ;
	int ws ;

	string var ;
	string val ;
	string mask ;

	bool found ;

	VEDIT_TYPE vtype ;

	fVAR* pvar ;

	ws = words( vget->vpg_vars ) ;
	for ( j = 1 ; j <= ws ; ++j )
	{
		var = sub_vars( word( vget->vpg_vars, j ) ) ;
		if ( selPanel )
		{
			if ( vget->vpg_pool == PROFILE )
			{
				p_poolMGR->erase( err, var, SHARED ) ;
				if ( err.error() ) { return ; }
			}
		}
		else
		{
			val = p_poolMGR->get( err, var, vget->vpg_pool ) ;
			if ( err.error() ) { return ; }
			found = err.RC0() ;
			pvar  = funcPool->getfVAR( err, var ) ;
			if ( err.error() ) { return ; }
			if ( pvar )
			{
				if ( pvar->hasmask( mask, vtype ) )
				{
					if ( !found ) { continue ; }
					remove_vmask( err, zdatef, var, val, mask, vtype ) ;
					if ( err.error() ) { return ; }
				}
				if ( pvar->integer() )
				{
					pvar->put( err, var, ( found ) ? val : "0" ) ;
				}
				else
				{
					pvar->put( err, var, val ) ;
				}
				if ( err.error() ) { return ; }
			}
			else
			{
				funcPool->put2( err, var, val ) ;
				if ( err.error() ) { return ; }
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
	//
	// Assignment processing.
	// Only action the first .ATTR or .ATTRCHAR statement in a panel section for the same field.
	// Ignore trailing spaces.
	//

	TRACE_FUNCTION() ;

	string rhs ;

	if ( assgn->as_func == PN_TRANS )
	{
		rhs = process_panel_trans( err,
					   ln,
					   assgn->as_trans,
					   assgn->as_lhs.value ) ;
		if ( err.error() ) { return ; }
	}
	else if ( assgn->as_func == PN_TRUNC )
	{
		rhs = process_panel_trunc( err,
					   assgn->as_trunc ) ;
		if ( err.error() ) { return ; }
	}
	else if ( assgn->as_func == PN_PFK )
	{
		if ( assgn->as_rhs.subtype == TS_AMPR_VAR_VALID )
		{
			rhs = getVariable( err, assgn->as_rhs ) ;
			if ( err.error() ) { return ; }
		}
		else
		{
			rhs = assgn->as_rhs.value ;
		}
		iupper( rhs ) ;
		process_panel_function( err,
					assgn->as_func,
					assgn->as_rhs.value,
					rhs ) ;
		if ( err.error() ) { return ; }
	}
	else if ( assgn->as_rhs.subtype == TS_AMPR_VAR_VALID ||
		  assgn->as_rhs.subtype == TS_CTL_VAR_VALID )
	{
		rhs = getVariable( err, assgn->as_rhs ) ;
		if ( err.error() ) { return ; }
		process_panel_function( err,
					assgn->as_func,
					assgn->as_rhs.value,
					rhs ) ;
		if ( err.error() ) { return ; }
	}
	else
	{
		rhs = sub_vars( assgn->as_rhs.value ) ;
	}

	trim_right( rhs ) ;

	if ( !assgn->as_isattr && assgn->as_lhs.subtype == TS_CTL_VAR_VALID )
	{
		setControlVar( err, assgn->as_lhs.value, upper( rhs ), ps_sect, ln ) ;
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
					attrchar_sect,
					ps_sect ) ;
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
	//
	// .ATTR() processing.  For a TB model field in the )INIT section, perform for every
	// model line on the screen.
	//

	TRACE_FUNCTION() ;

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
		err.seterrid( TRACE_INFO(), "PSYE041S", lhs ) ;
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
			fieldList[ t ]->field_attr( err, rhs ) ;
			if ( err.error() ) { return ; }
			attrList.push_back( t ) ;
			++i ;
		} while ( i < tb_rdepth ) ;
	}
	else
	{
		if ( tb_field( lhs ) )
		{
			lhs += "." + d2ds( ln ) ;
		}
		fieldList[ lhs ]->field_attr( err,
					      rhs,
					      ( ps_sect == PS_PROC || ps_sect == PS_REINIT ) ) ;
		if ( err.error() ) { return ; }
		attrList.push_back( lhs ) ;
	}
}


void pPanel::process_panel_attrchar( errblock& err,
				     const string& rhs,
				     ASSGN* assgn,
				     set<string>& attrchar_sect,
				     PS_SECT ps_sect )
{
	//
	// .ATTRCHAR() processing.
	//
	// If there is a field entry that has a .ATTR() active referencing this )ATTR entry,
	// update the field_inline_attrs entry with the current override values of the )ATTR entry.
	//

	TRACE_FUNCTION() ;

	char ch ;

	string lhs ;

	char_attrs* attrchar ;

	field* pfield ;

	err.setRC( 0 ) ;

	lhs = assgn->as_lhs.value ;
	if ( assgn->as_lhs.subtype == TS_AMPR_VAR_VALID )
	{
		lhs = getDialogueVar( err, lhs ) ;
		if ( lhs == "" || err.error() ) { return ; }
		if ( lhs.size() > 2 || ( lhs.size() == 2 && not ishex( lhs ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE041B" ) ;
			return ;
		}
		if ( lhs.size() == 2 ) { lhs = xs2cs( lhs ) ; }
	}

	if ( attrchar_sect.count( lhs ) > 0 ) { return ; }

	attrchar_sect.insert( lhs ) ;

	ch = lhs.front() ;

	auto it = char_attrlist.find( ch ) ;

	if ( it == char_attrlist.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE031N", isprint( ch ) ? string( 1, ch ) : c2xs( ch ) ) ;
		return ;
	}

	attrchar = it->second ;
	attrchar->override_attrs( err,
				  rhs,
				  ( ps_sect == PS_PROC || ps_sect == PS_REINIT ) ) ;
	if ( err.error() ) { return ; }

	ex_attrchar = true ;

	for ( auto ita = attrList.begin() ; ita != attrList.end() ; ++ita )
	{
		auto itf = fieldList.find( *ita ) ;
		pfield = itf->second ;
		if ( attrchar == pfield->field_char_attrs && pfield->field_inline_attrs )
		{
			pfield->field_inline_attrs->update( err, attrchar ) ;
			if ( err.error() ) { return ; }
		}
	}
}


void pPanel::process_panel_function( errblock& err,
				     PN_FUNCTION func,
				     const string& s1,
				     string& s2 )
{
	TRACE_FUNCTION() ;

	map<string, field*>::iterator itf ;
	map<string, dynArea*>::iterator itd ;

	int c ;
	int i ;
	int r ;

	switch ( func )
	{
	case PN_LENGTH:
		itf = fieldList.find( s1 ) ;
		if ( itf != fieldList.end() && itf->second->field_scroll_on() )
		{
			s2 = d2ds( itf->second->field_get_display_len() ) ;
		}
		else
		{
			s2 = d2ds( s2.size() ) ;
		}
		break ;

	case PN_LVLINE:
		itd = dynAreaList.find( s1 ) ;
		if ( itd == dynAreaList.end() )
		{
			err.seterrid( TRACE_INFO(), "PSYE043I", s1 ) ;
			return ;
		}
		s2 = d2ds( ( itd->second->dynArea_depth - get_areared_lines( itd->second ) ), 4 ) ;
		break ;

	case PN_PFK:
		if ( s2 == "" ) { return ; }
		if ( s2.size() < 3 && isnumeric( s2 ) )
		{
			c = ds2d( s2 ) ;
			if ( c > 0 && c < 25 )
			{
				s2 = "" ;
				if ( p_poolMGR->sysget( err, "ZKLUSE", PROFILE ) == "Y" || keyshr )
				{
					s2 = ( keylistn == "" || keylistl.count( KEY_F( c ) ) == 0 ) ? "" : keylistl[ KEY_F( c ) ] ;
				}
				if ( s2 == "" )
				{
					s2 = p_poolMGR->get( err, "ZPF" + d2ds( c, 2 ), PROFILE ) ;
				}
				return ;
			}
		}

		r = ( p_poolMGR->sysget( err, "ZPRIKEYS", PROFILE ) == "LOW" ) ? 1 : 13 ;
		if ( p_poolMGR->sysget( err, "ZKLUSE", PROFILE ) == "Y" || keyshr )
		{
			for ( i = 0 ; i < 24 ; ++i )
			{
				c = r + i ;
				if ( c > 24 ) { c -= 24 ; }
				if ( s2 == ( ( keylistn == "" || keylistl.count( KEY_F( c ) ) == 0 ) ? "" : upper( keylistl[ KEY_F( c ) ] ) ) )
				{
					s2 = "F" + d2ds( c ) ;
					return ;
				}
			}
		}
		for ( i = 0 ; i < 24 ; ++i )
		{
			c = r + i ;
			if ( c > 24 ) { c -= 24 ; }
			if ( s2 == upper( p_poolMGR->get( err, "ZPF" + d2ds( c, 2 ), PROFILE ) ) )
			{
				s2 = "F" + d2ds( c ) ;
				return ;
			}
		}
		s2 = "" ;
		break ;

	case PN_REVERSE:
		reverse( s2.begin(), s2.end() ) ;
		break ;

	case PN_UPPER:
		iupper( s2 ) ;
		break ;

	case PN_WORDS:
		s2 = d2ds( words( s2 ) ) ;
		break ;

	case PN_EXISTS:
		try
		{
			s2 = exists( s2 ) ? "1" : "0" ;
		}
		catch (...)
		{
			s2 = "0" ;
		}
		break ;

	case PN_FILE:
		try
		{
			s2 = is_regular_file( s2 ) ? "1" : "0" ;
		}
		catch (...)
		{
			s2 = "0" ;
		}
		break ;

	case PN_DIR:
		try
		{
			s2 = is_directory( s2 ) ? "1" : "0" ;
		}
		catch (...)
		{
			s2 = "0" ;
		}
	}
}


string pPanel::getVariable( errblock& err,
			    const vparm& svar )
{
	TRACE_FUNCTION() ;

	return ( svar.subtype == TS_CTL_VAR_VALID ) ? getControlVar( err, svar.value )  :
						      getDialogueVar( err, svar.value ) ;
}


string pPanel::getVariable( errblock& err,
			    const string& var )
{
	//
	// Return the value of a variable if first character is an '&' or just the
	// variable name if not.  Variable name has already been checked.
	//

	TRACE_FUNCTION() ;

	return ( var.front() == '&' ) ? getDialogueVar( err, var.substr( 1 ) ) : var ;
}


string pPanel::getControlVar( errblock& err,
			      const string& svar )
{
	TRACE_FUNCTION() ;

	auto it = control_vars.find( svar ) ;
	if ( it == control_vars.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE033G", svar ) ;
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
		return funcPool->get2( err, 0, "ZCURINX" ) ;

	case CV_CURSOR:
		return get_cursor() ;

	case CV_EDIT:
		return forEdit ? "YES" : "NO" ;

	case CV_FALSE:
		return "0" ;

	case CV_HELP:
		return dHelp ;

	case CV_HHELP:
		return dHHelp ;

	case CV_HIST:
		return ( dHist != "" ) ? "ON" : "OFF" ;

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
			    PS_SECT ps_sect,
			    int ln )
{
	TRACE_FUNCTION() ;

	int i ;
	int ws ;

	string t1 ;
	string t2 ;

	field* pfield  ;

	auto it = control_vars.find( svar ) ;
	if ( it == control_vars.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE033G", svar ) ;
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
			err.seterrid( TRACE_INFO(), "PSYE041G" ) ;
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
			err.seterrid( TRACE_INFO(), "PSYE042O" ) ;
		}
		break ;

	case CV_BROWSE:
		forBrowse = ( sval == "YES" ) ;
		break ;

	case CV_CSRPOS:
		if ( dCsrpos == 0 )
		{
			if ( !isnumeric( sval ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE041J" ) ;
				return ;
			}
			i       = ds2d( sval ) ;
			dCsrpos = ( i > 0 ) ? i : 1 ;
		}
		break ;

	case CV_CSRROW:
		if ( !isnumeric( sval ) || ds2d( sval ) < 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE041H" ) ;
			return ;
		}
		tb_csrrow = ds2d( sval ) ;
		break ;

	case CV_CURSOR:
		if ( dCursor == "" )
		{
			dCursor   = strip( sval ) ;
			msgloc    = "" ;
			tb_curidr = ln  ;
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

	case CV_HHELP:
		dHHelp = sval ;
		break ;

	case CV_HIST:
		if ( sval == "*" )
		{
			for ( auto fld : fieldList )
			{
				pfield = fld.second ;
				if ( fld.first != cmdfield &&
				     fld.first != scroll &&
				     pfield->field_length > 1 &&
				     pfield->field_is_input() &&
				    !pfield->field_is_passwd() &&
				    !pfield->field_is_tbdispl() &&
				    !pfield->field_is_dynamic() )
				{
					histories.insert( fld.second ) ;
					dHist = dHist + " " + fld.first ;
				}
			}
		}
		else
		{
			t1 = translate( sval, ' ', ',' ) ;
			for ( i = 1, ws = words( t1 ) ; i <= ws ; ++i )
			{
				t2 = word( t1, i ) ;
				auto it = fieldList.find( t2 ) ;
				if ( it == fieldList.end() )
				{
					err.seterrid( TRACE_INFO(), "PSYE031M", ".HIST", t2 ) ;
					break ;
				}
				pfield = it->second ;
				if ( histories.count( pfield ) > 0 )
				{
					err.seterrid( TRACE_INFO(), "PSYE042F", t2 ) ;
					break ;
				}
				if ( pfield->field_is_tbdispl() || pfield->field_is_dynamic() || pfield->field_is_passwd() )
				{
					err.seterrid( TRACE_INFO(), "PSYE043P", t2 ) ;
					break ;
				}
				histories.insert( pfield ) ;
			}
			dHist = dHist + " " + t1 ;
		}
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
			err.seterrid( TRACE_INFO(), "PSYE041I" ) ;
		}
		break ;

	case CV_TRAIL:
		dTrail = sval ;
		break ;

	case CV_ZVARS:
		if ( sval != "" && !isvalidName( sval ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE013A", ".ZVARS )ABCINIT statement", sval ) ;
			return  ;
		}
		dZvars = sval ;
	}
}


void pPanel::set_message_cond( const string& msg )
{
	TRACE_FUNCTION() ;

	if ( dMsg == "" )
	{
		dMsg = msg ;
	}
}


void pPanel::set_message_cond( errblock& err,
			       const string& msg,
			       const string& zval1 )
{
	TRACE_FUNCTION() ;

	if ( dMsg == "" )
	{
		dMsg = msg ;
		funcPool->put2( err, "ZVAL1", zval1 ) ;
	}
}


void pPanel::set_message_cond( errblock& err,
			       const string& msg,
			       const string& zval1,
			       const string& zval2 )
{
	TRACE_FUNCTION() ;

	if ( dMsg == "" )
	{
		dMsg = msg ;
		funcPool->put2( err, "ZVAL1", zval1 ) ;
		funcPool->put2( err, "ZVAL2", zval2 ) ;
	}
}


void pPanel::set_message_cond( errblock& err,
			       const string& msg,
			       const string& zval1,
			       const string& zval2,
			       const string& zval3 )
{
	TRACE_FUNCTION() ;

	if ( dMsg == "" )
	{
		dMsg = msg ;
		funcPool->put2( err, "ZVAL1", zval1 ) ;
		funcPool->put2( err, "ZVAL2", zval2 ) ;
		funcPool->put2( err, "ZVAL3", zval2 ) ;
	}
}


bool pPanel::is_history_field( const string& field )
{
	TRACE_FUNCTION() ;

	return findword( field, dHist ) ;
}


void pPanel::set_zzstr_shared( errblock& err,
			       const string& zzstr1 )
{
	TRACE_FUNCTION() ;

	p_poolMGR->put( err, "ZZSTR1", zzstr1, SHARED ) ;
}


void pPanel::set_zzstr_shared( errblock& err,
			       const string& zzstr1,
			       const string& zzstr2 )
{
	TRACE_FUNCTION() ;

	p_poolMGR->put( err, "ZZSTR1", zzstr1, SHARED ) ;
	if ( err.error() ) { return ; }

	p_poolMGR->put( err, "ZZSTR2", zzstr2, SHARED ) ;
}


void pPanel::set_cursor_cond( const string& csr,
			      int i,
			      int p )
{
	TRACE_FUNCTION() ;

	if ( dCursor == "" )
	{
		dCursor   = csr ;
		iCsrpos   = p   ;
		msgloc    = csr ;
		tb_curidr = i   ;
	}
}


void pPanel::set_cursor( const string& csr,
			 int p )
{
	TRACE_FUNCTION() ;

	iCursor = csr ;
	iCsrpos = max( 1, p ) ;
}


void pPanel::clear_cursor_cond()
{
	//
	// Clear the cursor, if not on a table display field.
	//

	TRACE_FUNCTION() ;

	if ( !tb_field( iCursor ) )
	{
		iCursor = "" ;
	}
}


string pPanel::get_tbCursor( const string& cursor )
{
	//
	// This is only called for a table display field.
	// Return cursor position as set by get_cursor(), tb_curidr and tb_csrrow.
	//
	// Use tb_curidr if set, else csrrow.
	// If visible, return the internal field name otherwise the command field.
	//

	TRACE_FUNCTION() ;

	int p ;

	if ( ztdvrows > 0 )
	{
		if ( tb_curidr > -1 )
		{
			return cursor + "." + d2ds( tb_curidr ) ;
		}
		else if ( tb_csrrow > 0 )
		{
			p = tb_modlines * ( tb_csrrow - ztdtop ) ;
			if ( p >= 0 && p < int( tb_pdepth - get_tbred_lines() ) && ( tb_csrrow - ztdtop ) < ztdvrows )
			{
				return cursor + "." + d2ds( p / tb_modlines ) ;
			}
		}
	}

	return cmdfield ;
}


string pPanel::get_tbCursor( const string& cursor,
			     int crp )
{
	//
	// This is only called for a table display field.
	//
	// Return cursor position as set by get_cursor() and the CRP.
	// If visible, return the internal field name otherwise the command field.
	//

	TRACE_FUNCTION() ;

	int p ;

	if ( crp > 0 )
	{
		p = tb_modlines * ( crp - ztdtop ) ;
		if ( p >= 0 && p < int( tb_pdepth - get_tbred_lines() ) && ( crp - ztdtop ) < ztdvrows )
		{
			return cursor + "." + d2ds( p / tb_modlines ) ;
		}
	}

	return cmdfield ;
}


void pPanel::set_msgloc( errblock& err,
			 const string& p_msgloc )
{
	TRACE_FUNCTION() ;

	err.setRC( 0 ) ;

	if ( p_msgloc == "" || tb_field( p_msgloc ) )
	{
		msgloc = p_msgloc ;
		return ;
	}

	auto it = fieldList.find( p_msgloc ) ;
	if ( it == fieldList.end() || !it->second->field_active )
	{
		err.seterrid( TRACE_INFO(), "PSYE022M", p_msgloc, panelid, 12 ) ;
	}
	else
	{
		msgloc = p_msgloc ;
	}
}


string pPanel::get_msgloc()
{
	TRACE_FUNCTION() ;

	int p ;

	if ( tb_field( msgloc ) )
	{
		if ( ztdvrows > 0 )
		{
			if ( tb_curidr > -1 )
			{
				return msgloc + "." + d2ds( tb_curidr ) ;
			}
			else if ( tb_csrrow > 0 )
			{
				p = tb_modlines * ( tb_csrrow - ztdtop ) ;
				if ( p >= 0 && p < int( tb_pdepth - get_tbred_lines() ) && ( tb_csrrow - ztdtop ) < ztdvrows )
				{
					return msgloc + "." + d2ds( p / tb_modlines ) ;
				}
			}
		}
		return "" ;
	}

	return msgloc ;
}


bool pPanel::tb_field( const string& f )
{
	TRACE_FUNCTION() ;

	return ( tb_fields.count( f ) > 0 ) ;
}


void pPanel::refresh_fields( errblock& err,
			     int ln,
			     const string& fields )
{
	//
	// Update the field value from the dialogue variable.  Apply any field justification defined.
	// For REFRESH * and table display fields, only refresh fields on line ln and the
	// dialogue variable for the internal field name.
	//

	TRACE_FUNCTION() ;

	int width ;

	int i  ;
	int j  ;
	int ws ;

	string name    ;

	string* darea  ;
	string* shadow ;

	field* pfield  ;

	dynArea* da ;

	field_ext1* dx ;

	map<string, field*>::iterator   itf ;
	map<string, dynArea*>::iterator itd ;

	map<string,string> fconv ;

	if ( tb_model && fields != "*" )
	{
		for ( ws = words( fields ), i = 1 ; i <= ws ; ++i )
		{
			name = word( fields, i ) ;
			if ( tb_field( name ) )
			{
				fconv[ name + "." + d2ds( ln ) ] = name ;
			}
		}
	}

	for ( itf = fieldList.begin() ; itf != fieldList.end() ; ++itf )
	{
		pfield = itf->second ;
		if ( !pfield->field_is_dynamic() &&
		   ( fields == "*" || findword( itf->first, fields ) || fconv.count( itf->first ) > 0 ) )
		{
			if ( fconv.count( itf->first ) > 0 )
			{
				refresh_field( err, pfield, fconv[ itf->first ] ) ;
				if ( err.error() ) { return ; }
				funcPool->put3( itf->first, funcPool->get2( err, 0, fconv[ itf->first ] ) ) ;
			}
			else if ( fields == "*" && pfield->field_is_tbdispl() )
			{
				name = pfield->field_get_name() ;
				if ( pfield->field_get_index() == ln )
				{
					refresh_field( err, pfield, name ) ;
					if ( err.error() ) { return ; }
					funcPool->put3( itf->first, funcPool->get2( err, 0, name ) ) ;
				}
			}
			else
			{
				refresh_field( err, pfield, itf->first ) ;
				if ( err.error() ) { return ; }
			}
		}
	}

	for ( itd = dynAreaList.begin() ; itd != dynAreaList.end() ; ++itd )
	{
		da = itd->second ;
		if ( fields == "*" || findword( itd->first, fields ) )
		{
			width = da->dynArea_width ;
			darea = funcPool->vlocate( err, itd->first ) ;
			if ( err.error() ) { return ; }
			shadow = funcPool->vlocate( err, da->dynArea_shadow_name ) ;
			if ( err.error() ) { return ; }
			darea->resize( da->dynArea_area, ' ' ) ;
			shadow->resize( da->dynArea_area, ' ' ) ;
			i = 0 ;
			for ( auto f : da->fieldList )
			{
				pfield = static_cast<field*>( f ) ;
				dx     = pfield->field_da_ext ;
				j      = i * da->dynArea_width ;
				pfield->field_value   = darea->substr( j, width )  ;
				dx->field_ext1_shadow = shadow->substr( j, width ) ;
				++i ;
			}
		}
	}
}


void pPanel::refresh_fields( errblock& err )
{
	//
	// Update all field values from their dialogue variables
	// including dynamic area overflow variables and display.
	//

	TRACE_FUNCTION() ;

	int width ;

	uint i ;
	uint j ;

	string* darea  ;
	string* shadow ;
	string* oflow1 ;
	string* oflow2 ;
	string* oflow3 ;

	field* pfield  ;

	dynArea* da ;

	field_ext1* dx ;

	map<string, field*>::iterator   itf ;
	map<string, dynArea*>::iterator itd ;

	for ( itf = fieldList.begin() ; itf != fieldList.end() ; ++itf )
	{
		pfield = itf->second ;
		if ( !pfield->field_is_dynamic() )
		{
			refresh_field( err, pfield, itf->first ) ;
			if ( err.error() ) { return ; }
			pfield->display_field( win, inv_schar, ddata_map, schar_map ) ;
		}
	}

	for ( itd = dynAreaList.begin() ; itd != dynAreaList.end() ; ++itd )
	{
		da     = itd->second ;
		width  = da->dynArea_width ;
		darea  = funcPool->vlocate( err, itd->first ) ;
		if ( err.error() ) { return ; }
		shadow = funcPool->vlocate( err, da->dynArea_shadow_name ) ;
		if ( err.error() ) { return ; }
		darea->resize( da->dynArea_area, ' ' ) ;
		shadow->resize( da->dynArea_area, ' ' ) ;
		i = 0 ;
		for ( auto f : da->fieldList )
		{
			pfield = static_cast<field*>( f ) ;
			dx     = pfield->field_da_ext ;
			j      = i * da->dynArea_width ;
			pfield->field_value   = darea->substr( j, width )  ;
			dx->field_ext1_shadow = shadow->substr( j, width ) ;
			if ( dx->field_ext1_has_overflow() )
			{
				oflow1 = funcPool->vlocate( err, dx->field_ext1_overflow_vname ) ;
				if ( err.error() ) { return ; }
				if ( err.RC0() )
				{
					oflow2 = funcPool->vlocate( err, dx->field_ext1_overflow_sname ) ;
					if ( err.error() ) { return ; }
					if ( err.RC0() )
					{
						oflow3 = funcPool->vlocate( err, dx->field_ext1_overflow_uname ) ;
						if ( err.error() ) { return ; }
						if ( err.RC0() )
						{
							dx->field_ext1_load( oflow1, oflow2, oflow3 ) ;
						}
					}
				}
			}
			pfield->display_field( win, inv_schar, ddata_map, schar_map ) ;
			++i ;
		}
	}
}


void pPanel::refresh_field( errblock& err,
			    field* pfield,
			    const string& var )
{
	//
	// Update the field value from the dialogue variable var and
	// apply any field justification defined.
	//
	// If vdefined with a mask, add mask to the field value.
	//
	// If field is scrollable, update tblen, associated indicators etc..
	//

	TRACE_FUNCTION() ;

	string mask ;
	string val ;

	size_t len = string::npos ;

	VEDIT_TYPE vtype ;

	if ( pfield->field_is_scrollable() )
	{
		len = pfield->field_get_value_size() ;
	}

	pfield->field_put_value( getDialogueVar( err, var ) ) ;
	if ( err.error() ) { return ; }

	pfield->field_prep_display() ;
	pfield->field_apply_caps_out() ;

	if ( funcPool->hasmask( var, mask, vtype ) )
	{
		val = pfield->field_get_value() ;
		pfield->field_put_value( add_vmask( val, zdatef, mask, vtype ) ) ;
	}

	if ( pfield->field_is_scrollable() )
	{
		if ( pfield->field_is_tbdispl() )
		{
			if ( pfield->field_update_tblen_max() )
			{
				update_scroll_inds( err, pfield ) ;
				if ( err.error() ) { return ; }
				update_scroll_lcol( err, pfield ) ;
				if ( err.error() ) { return ; }
				update_scroll_rcol( err, pfield ) ;
				if ( err.error() ) { return ; }
			}
		}
		else if ( len == string::npos || len != pfield->field_get_value_size() )
		{
			update_scroll_inds( err, pfield ) ;
			if ( err.error() ) { return ; }
			update_scroll_lcol( err, pfield ) ;
			if ( err.error() ) { return ; }
			update_scroll_rcol( err, pfield ) ;
			if ( err.error() ) { return ; }
		}
	}
}


void pPanel::display_panel_update( errblock& err )
{
	//
	// This is executed after a panel is displayed.
	//
	// For all changed input fields, remove the null character, apply attributes (upper case, left/right
	// justified), and copy back to function pool (unless it is a masked variable - VEDIT does it in this case ).
	// Reset field for display.
	//
	// Process point_and_shoot fields at the end so they have priority over changed fields.
	//
	// If panel is displayed by the TUTORIAL program, Enter scrolls down a scroll area, UP/DOWN are passed through.
	//
	// For dynamic areas, also update the shadow variable to indicate character deletes (0xFE) or nulls
	// converted to spaces (0xFF).
	//

	TRACE_FUNCTION() ;

	int i ;
	int j ;
	int p ;

	int fieldNum ;
	int maxAmount ;
	int offset ;
	int posn ;
	int Amnt ;

	uint l ;

	string cmdVerb ;
	string cmd ;
	string fieldNam ;
	string msgfld ;

	string t ;

	bool zcont = false ;
	bool cmd_nonblank ;

	const string scrollAmounts = "M MAX C CSR D DATA H HALF P PAGE" ;

	string* darea ;
	string* shadow ;

	field* pfield ;
	field* tfield ;

	Area* parea ;

	field_ext1* dx ;

	fVAR* pvar ;

	map<string, field*>::iterator it ;

	err.setRC( 0 ) ;

	fieldNum = -1 ;
	posn     = 1 ;
	MSG.clear() ;

	reset_control_variables() ;

	redisplay = false ;
	bypassCur = false ;

	tb_fixed = false ;

	end_pressed = ( usr_action == USR_CANCEL ||
			usr_action == USR_END    ||
			usr_action == USR_EXIT   ||
			usr_action == USR_RETURN ) ;

	cmdVerb = p_poolMGR->get( err, "ZVERB", SHARED ) ;
	if ( err.error() ) { return ; }

	cmd = upper( cmd_getvalue() ) ;

	cmd_nonblank = ( cmd != "" ) ;

	if ( Area1 != "" && ( ( !tutorial && ( cmdVerb == "UP" || cmdVerb == "DOWN" ) ) ||
			      (  tutorial && cmd == "" && ( cmdVerb == "" || cmdVerb == "RIGHT" || cmdVerb == "LEFT" ) ) ) )
	{
		zcont = false ;
		parea = nullptr ;
		for ( auto& area : AreaList )
		{
			if ( area.second->cursor_on_area( l_row, l_col ) )
			{
				parea = area.second ;
				break ;
			}
		}
		if ( !parea && !scrollOn )
		{
			parea = AreaList[ Area1 ] ;
		}
		if ( parea )
		{
			if ( ( !tutorial || parea->can_scroll() ) && ( cmdVerb == "UP" || cmdVerb == "LEFT" ) )
			{
				p = parea->scroll_up( l_row, l_col ) ;
				if ( p == -1 )
				{
					if ( tutorial && parea->is_tod() )
					{
						zcont = true ;
					}
					else
					{
						set_message_cond( "PSYS013K" ) ;
						parea->tod_issued() ;
					}
				}
				else
				{
					rebuild_after_area_scroll( parea ) ;
					l_row += p ;
					parea->reset_tod() ;
				}
				parea->reset_eod() ;
			}
			else if ( !tutorial || parea->can_scroll() )
			{
				p = parea->scroll_down( l_row, l_col ) ;
				if ( p == -1 )
				{
					if ( tutorial && parea->is_eod() )
					{
						zcont = true ;
					}
					else
					{
						set_message_cond( "PSYS013L" ) ;
						parea->eod_issued() ;
					}
				}
				else
				{
					rebuild_after_area_scroll( parea ) ;
					l_row -= p ;
					parea->reset_eod() ;
				}
				parea->reset_tod() ;
			}
			else
			{
				zcont = true ;
			}
			cmd = cmd_getvalue() ;
			if ( !tutorial && ( findword( upper( cmd ), scrollAmounts ) || ( isnumeric( cmd ) && cmd.size() <= 6 ) ) )
			{
				cmd_setvalue( "" ) ;
			}
			if ( !zcont )
			{
				redisplay = true ;
				bypassCur = true ;
				return ;
			}
		}
	}

	if ( sfields.size() > 0 && ( cmdVerb == "LEFT" || cmdVerb == "RIGHT" || cmdVerb == "ZCLRSFLD" ) )
	{
		pfield = field_getaddr_any( ( l_row + win_row ), ( l_col + win_col ) ) ;
		if ( pfield && pfield->field_scroll_on() )
		{
			l = 0 ;
			if ( scrollOn && ( cmdVerb == "RIGHT" || cmdVerb == "LEFT" ) )
			{
				cmd = upper( cmd_getvalue() ) ;
				t   = "" ;
				if ( cmd == "" )
				{
				}
				else if ( isnumeric( cmd ) )
				{
					if ( cmd.size() < 6 )
					{
						t = cmd ;
						cmd_setvalue( "" ) ;
					}
				}
				else
				{
					switch ( cmd.front() )
					{
					case 'H':
					case 'D':
					case 'P':
					case 'C':
					case 'M':
						  t = string( 1, cmd.front() ) ;
						  cmd_setvalue( "" ) ;
						  break ;
					}
				}
				if ( t == "" )
				{
					if ( scroll != "" )
					{
						t = p_poolMGR->get( err, scroll, PROFILE ) ;
						if ( err.error() ) { return ; }
					}
					if ( t == "" )
					{
						t = getDialogueVar( err, "ZSCROLLD" ) ;
						if ( err.error() ) { return ; }
						if ( t == "" )
						{
							t = "P" ;
						}
					}
				}
				if ( !isnumeric( t ) )
				{
					switch( t.front() )
					{
						case 'C': l = l_col - pfield->field_col ;
							break ;

						case 'D':
							l = pfield->field_length - 1;
							break ;

						case 'P':
							l = pfield->field_length ;
							break ;

						case 'H':
							l = pfield->field_length / 2 ;
							break ;

						case 'M':
							l = 32767 ;
							break ;
					}
				}
				else
				{
					l = ds2d( t ) ;
				}
			}
			if ( cmdVerb == "RIGHT" || cmdVerb == "LEFT" )
			{
				if ( cmdVerb == "RIGHT" )
				{
					if ( !pfield->field_scroll_right( l ) )
					{
						set_message_cond( "PSYS021A" ) ;
						redisplay = true ;
						bypassCur = true ;
						return ;
					}
				}
				else if ( !pfield->field_scroll_left( l ) )
				{
					set_message_cond( "PSYS021B" ) ;
					redisplay = true ;
					bypassCur = true ;
					return ;
				}
				if ( pfield->field_is_tbdispl() )
				{
					uint ss  = pfield->field_get_start() ;
					fieldNam = pfield->field_tb_ext->field_ext2_name ;
					j        = pfield->field_tb_ext->field_ext2_index ;
					for ( i = 0 ; i < tb_rdepth ; ++i )
					{
						if ( i != j )
						{
							auto it = fieldList.find( fieldNam + "." + d2ds( i ) ) ;
							it->second->field_scroll_to_pos( ss ) ;
						}
					}
				}
				else if ( pfield->field_has_lcol_var() )
				{
					l = pfield->field_get_start() ;
					auto ret = lcolmmap.equal_range( pfield->field_get_lcol_var() ) ;
					for ( auto it = ret.first ; it != ret.second ; ++it )
					{
						tfield = it->second ;
						if ( tfield != pfield && tfield->field_scroll_on() )
						{
							if ( tfield->field_scroll_to_pos( l ) )
							{
								update_scroll_inds( err, tfield ) ;
								if ( err.error() ) { return ; }
								update_scroll_lcol( err, tfield ) ;
								if ( err.error() ) { return ; }
								update_scroll_rcol( err, tfield ) ;
								if ( err.error() ) { return ; }
							}
						}
					}
				}
				l_col = pfield->field_col ;
			}
			else if ( pfield->field_is_input() )
			{

				pfield->field_put_value( "" ) ;
				pfield->field_changed = true ;
				redisplay = true ;
				bypassCur = true ;
				return ;
			}
			update_scroll_inds( err, pfield ) ;
			if ( err.error() ) { return ; }
			if ( cmdVerb != "ZCLRSFLD" )
			{
				update_scroll_lcol( err, pfield ) ;
				if ( err.error() ) { return ; }
				update_scroll_rcol( err, pfield ) ;
				if ( err.error() ) { return ; }
			}
			redisplay = true ;
			bypassCur = true ;
			return ;
		}
	}

	reset_attrs_once() ;

	if ( scrollOn && scroll != "" )
	{
		it = fieldList.find( scroll ) ;
		pfield = it->second ;
		if ( trim( pfield->field_value ) == "" )
		{
			pfield->field_put_value( p_poolMGR->get( err, scroll, PROFILE ) ) ;
			if ( err.error() ) { return ; }
			if ( pfield->field_value_blank() )
			{
				pfield->field_put_value( getDialogueVar( err, "ZSCROLLD" ) ) ;
				if ( err.error() ) { return ; }
			}
		}
		pfield->field_prep_input() ;
		pfield->field_apply_caps_uncond() ;
		pfield->field_changed = false ;
		if ( !isnumeric( pfield->field_get_value() ) )
		{
			switch( pfield->field_get_value().front() )
			{
				case 'C': pfield->field_put_value( "CSR " ) ; break ;
				case 'D': pfield->field_put_value( "DATA" ) ; break ;
				case 'H': pfield->field_put_value( "HALF" ) ; break ;
				case 'P': pfield->field_put_value( "PAGE" ) ; break ;
				default:  set_message_cond( "PSYS011I" ) ;
					  set_cursor_cond( scroll ) ;
					  dCsrpos = 1 ;
			}
		}
		funcPool->put2( err, it->first, pfield->field_value ) ;
		if ( err.error() ) { return ; }
	}

	tb_curidx = -1 ;
	tb_curidr = -1 ;

	for ( auto& f : fieldList )
	{
		pfield = f.second ;
		if ( pfield->field_changed )
		{
			if ( pfield->field_is_dynamic() )
			{
				pfield->field_remove_nulls_da() ;
				dx       = pfield->field_da_ext ;
				fieldNam = pfield->field_get_name() ;
				fieldNum = pfield->field_get_index() ;
				darea    = funcPool->vlocate( err, fieldNam ) ;
				if ( err.error() ) { return ; }
				offset   = fieldNum * pfield->field_length ;
				pfield->field_update_datamod_usermod( darea, offset ) ;
				darea->replace( offset, pfield->field_length, pfield->field_value ) ;
				shadow = funcPool->vlocate( err, pfield->field_da_ext->field_ext1_dynArea->dynArea_shadow_name ) ;
				if ( err.error() ) { return ; }
				shadow->replace( offset, pfield->field_length, dx->field_ext1_shadow ) ;
				if ( dx->field_ext1_changed )
				{
					funcPool->put2( err, dx->field_ext1_overflow_vname, dx->field_ext1_overflow_value ) ;
					if ( err.error() ) { return ; }
					funcPool->put2( err, dx->field_ext1_overflow_sname, dx->field_ext1_overflow_shadow ) ;
					if ( err.error() ) { return ; }
					funcPool->put2( err, dx->field_ext1_overflow_uname, "Y" ) ;
					if ( err.error() ) { return ; }
				}
			}
			else if ( pfield->field_is_input() )
			{
				if ( tb_model && !pfield->field_is_tbdispl() )
				{
					tb_fixed = true ;
				}
				pfield->field_prep_input() ;
				( scroll == f.first ) ? pfield->field_apply_caps_uncond() : pfield->field_apply_caps_in() ;
				if ( pfield->field_is_tbdispl() )
				{
					funcPool->put3( f.first, pfield->field_get_value() ) ;
				}
				else if ( selPanel )
				{
					p_poolMGR->put( err, f.first, pfield->field_get_value(), ASIS ) ;
				}
				else
				{
					pvar = funcPool->getfVAR( err, f.first ) ;
					if ( err.error() ) { return ; }
					if ( pvar )
					{
						if ( pvar->hasmask() )
						{
							continue ;
						}
						if ( pvar->integer() )
						{
							if ( datatype( pfield->field_get_value(), 'W' ) )
							{
								pvar->put( err, f.first, pfield->field_get_value() ) ;
								if ( err.error() ) { return ; }
							}
							else
							{
								set_message_cond( "PSYS011G" ) ;
								set_cursor_cond( f.first ) ;
								dCsrpos = 1 ;
							}
						}
						else
						{
							pvar->put( err, f.first, pfield->field_get_value() ) ;
							if ( err.error() ) { return ; }
						}
					}
					else
					{
						funcPool->put2( err, f.first, pfield->field_get_value() ) ;
						if ( err.error() ) { return ; }
					}
				}
			}
		}
		if ( pfield->field_active && pfield->cursor_on_field( l_row, l_col ) )
		{
			posn = l_col - pfield->field_col + 1 ;
			if ( pfield->field_is_dynamic() )
			{
				fieldNam = pfield->field_get_name() ;
				fieldNum = pfield->field_get_index() ;
				iCursor  = fieldNam ;
				iCsrpos  = ( fieldNum * pfield->field_length ) + posn ;
			}
			else if ( pfield->field_is_tbdispl() )
			{
				fieldNam = pfield->field_get_name() ;
				fieldNum = pfield->field_get_index() ;
				iCursor  = fieldNam ;
				iCsrpos  = posn ;
			}
			else
			{
				iCursor = f.first ;
				iCsrpos = posn ;
			}
		}
	}

	for ( auto& sf : sfields )
	{
		pfield = sf.first ;
		if ( pfield->field_has_lenvar() )
		{
			update_lenvar( err, pfield ) ;
			if ( err.error() ) { return ; }
			funcPool->put2( err, pfield->field_get_lenvar(), pfield->field_get_display_len() ) ;
			if ( err.error() ) { return ; }
		}
	}

	if ( tb_model && l_row >= tb_toprow && l_row < ( tb_toprow + tb_pdepth - get_tbred_lines() ) )
	{
		tb_curidx = ( l_row - tb_toprow ) / tb_modlines ;
	}

	p_poolMGR->put( err, "ZSCROLLA", "", SHARED ) ;
	if ( err.error() ) { return ; }

	p_poolMGR->put( err, "ZSCROLLN", "0", SHARED ) ;
	if ( err.error() ) { return ; }

	if ( ( tutorial || scrollOn ) && findword( cmdVerb, "UP DOWN LEFT RIGHT" ) )
	{
		cmd    = upper( cmd_getvalue() ) ;
		msgfld = cmdfield ;
		if ( cmd == "" || ( !findword( cmd, scrollAmounts ) && !isnumeric( cmd ) ) )
		{
			if ( scroll != "" )
			{
				cmd = fieldList[ scroll ]->field_get_value() ;
				msgfld = scroll ;
			}
			else
			{
				cmd = getDialogueVar( err, "ZSCROLLD" ) ;
				if ( err.error() ) { return ; }
				if ( cmd == "" )
				{
					cmd = "DATA" ;
				}
			}
		}
		else
		{
			cmd_setvalue( "" ) ;
			if ( cmdfield != "" )
			{
				funcPool->put2( err, cmdfield, "" ) ;
				if ( err.error() ) { return ; }
			}
		}
		maxAmount = ( tb_model ) ? ( ( tb_pdepth - get_tbred_lines() ) / tb_modlines ) :
			    ( findword( cmdVerb, "LEFT RIGHT" ) ) ? dyns_width : ( dyns_depth - get_areared_lines() ) ;
		if ( tb_model && cmd_nonblank && tb_curidx == -1 )
		{
			set_lcursor_home() ;
		}
		if ( cmd == "" )
		{
		}
		else if ( isnumeric( cmd ) )
		{
			if ( cmd.size() > 6 )
			{
				set_message_cond( "PSYS011I" ) ;
				set_cursor_cond( msgfld ) ;
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
			Amnt = ( tb_model )               ?
			       ( tb_curidx == -1 )        ? maxAmount :
			       ( cmdVerb.front() == 'U' ) ? maxAmount - tb_curidx - 1 :
			       ( cmdVerb.front() == 'D' ) ? tb_curidx :
			       ( cmdVerb.front() == 'L' ) ? maxAmount - posn : posn - 1 :
			       ( fieldNum == -1 )         ? maxAmount :
			       ( cmdVerb.front() == 'U' ) ? maxAmount - fieldNum - 1 :
			       ( cmdVerb.front() == 'D' ) ? fieldNum :
			       ( cmdVerb.front() == 'L' ) ? maxAmount - posn : posn - 1 ;
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

	point_and_shoot( err ) ;
}


void pPanel::display_text()
{
	//
	// Display TEXT.
	//
	// Substitute dialogue variables and place result in text_xvalue.  If there are no
	// dialogue variables present, set text_dvars to false and clear text_value.
	//

	TRACE_FUNCTION() ;

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


void pPanel::display_tbtext()
{
	//
	// Display TBTEXT.
	//
	// Substitute dialogue variables and place result in text_xvalue.  If there are no
	// dialogue variables present, set text_dvars to false and clear text_value.
	//

	TRACE_FUNCTION() ;

	for ( auto it = tbtextList.begin() ; it != tbtextList.end() ; ++it )
	{
		if ( (*it)->text_row < ( tb_toprow + ( ztdvrows * tb_modlines ) ) )
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
}


void pPanel::display_fields( errblock& err,
			     bool reset )
{
	//
	// Display all fields.
	// Also reset the list of touched attribute positions for dynamic areas if requested.
	//

	TRACE_FUNCTION() ;

	map<string, field*>::iterator it ;

	field* pfield ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; ++it )
	{
		pfield = it->second ;
		if ( !pfield->field_is_dynamic() )
		{
			pfield->field_prep_display() ;
			pfield->field_apply_caps_out() ;
		}
		else if ( reset )
		{
			pfield->field_reset() ;
		}
		pfield->display_field( win, inv_schar, ddata_map, schar_map ) ;
	}
}


void pPanel::display_ab( errblock& err )
{
	TRACE_FUNCTION() ;

	bool hi_mnemonic ;

	if ( ab.size() == 0 ) { return ; }

	hi_mnemonic = ( p_poolMGR->get( err, "ZHIABMN", PROFILE ) == "Y" ) ;

	for ( auto it = ab.begin() ; it != ab.end() ; ++it )
	{
		(*it)->display_abc_unsel( win, hi_mnemonic ) ;
	}

	wattrset( win, cuaAttr[ ABSL ] | panel_intens ) ;
	mvwhline( win, 1, 0, ACS_HLINE, wscrmaxw ) ;
}


void pPanel::point_and_shoot( errblock& err )
{
	//
	// If the cursor is on a point-and-shoot field or text, set the pnts variable (if field blank), to the pnts value.
	//

	TRACE_FUNCTION() ;

	field* pfield ;

	map<string, pnts>::iterator itp = pntsTable.end() ;
	vector<text*>::iterator itt ;

	if ( pntsTable.empty() || cursor_not_in_window( l_row + win_row, l_col + win_col ) )
	{
		return ;
	}

	pfield = get_field_address( l_row, l_col ) ;
	if ( pfield && pfield->field_active && pfield->field_is_pas() )
	{
		if ( pfield->field_is_tbdispl() )
		{
			itp = pntsTable.find( pfield->field_tb_ext->field_ext2_name ) ;
		}
		else
		{
			itp = pntsTable.find( field_getname_any( pfield ) ) ;
		}
	}

	if ( !pfield )
	{
		for ( itt = text_pas.begin() ; itt != text_pas.end() ; ++itt )
		{
			if ( (*itt)->text_visible && (*itt)->cursor_on_text( l_row, l_col ) )
			{
				itp = pntsTable.find( (*itt)->text_name ) ;
				break ;
			}
		}
	}

	if ( itp != pntsTable.end() )
	{
		if ( !itp->second.pnts_fvar || strip( fieldList[ itp->second.pnts_var ]->field_value ) == "" )
		{
			putDialogueVar( err, itp->second.pnts_var, itp->second.pnts_val ) ;
		}
	}
}


void pPanel::reset_attrs()
{
	//
	// Reset attributes for fields/attributes that have had an attribute change,
	// ie. .ATTR() or .ATTRCHAR() in the )INIT, )REINIT and )PROC panel sections.
	//

	TRACE_FUNCTION() ;

	for ( auto ita = attrList.begin() ; ita != attrList.end() ; ++ita )
	{
		auto itf = fieldList.find( *ita ) ;
		itf->second->field_attr_reset() ;
	}
	attrList.clear() ;

	if ( ex_attrchar )
	{
		for ( auto it = char_attrlist.begin() ; it != char_attrlist.end() ; ++it )
		{
			it->second->remove_override() ;
		}
		ex_attrchar = false ;
	}
}


void pPanel::reset_attrs_once()
{
	//
	// Reset attributes for fields/attributes that have been marked as a temporary attribute change,
	// ie. .ATTR() or .ATTRCHAR() in the )REINIT and )PROC panel sections that are only valid
	// for one redisplay.
	//

	TRACE_FUNCTION() ;

	for ( auto ita = attrList.begin() ; ita != attrList.end() ; )
	{
		auto itf = fieldList.find( *ita ) ;
		if ( itf->second->field_attr_reset_once() )
		{
			ita = attrList.erase( ita ) ;
			continue ;
		}
		++ita ;
	}

	if ( ex_attrchar )
	{
		for ( auto it = char_attrlist.begin() ; it != char_attrlist.end() ; ++it )
		{
			it->second->remove_override_once() ;
		}
	}
}


void pPanel::cursor_placement_display( errblock& err )
{
	//
	// Position cursor for DISPLAY service.
	//
	// Default cursor placement for DISPLAY:
	//   If not set, placed on the first input field that meets these conditions starting from the top:
	//     a) First input field on a line.
	//     b) Field must be blank.
	//     c) Must not have a field name of ZCMD.
	//   First, non-scrollable areas are searched, then scrollable areas.
	//   If these conditions are still not met, cursor placed on the first input field.
	//   If there are no input fields, cursor is placed in the upper-left corner of the screen.
	//

	TRACE_FUNCTION() ;

	size_t i ;

	if ( bypassCur ) { return ; }

	uint zz_row ;
	uint zz_col ;

	field* pfield ;

	int    f_pos  = get_csrpos() ;
	string f_name = get_cursor() ;

	l_row = 0 ;
	l_col = 0 ;

	if ( f_name == "" )
	{
		for ( i = 0 ; i <= AreaList.size() && f_name == "" ; ++i )
		{
			zz_row = -1 ;
			zz_col = -1 ;
			for ( auto it = fieldList.begin() ; it != fieldList.end() ; ++it )
			{
				pfield = it->second ;
				if ( !pfield->field_active     ||
				     !pfield->field_visible    ||
				     !pfield->field_is_input() ||
				      it->first == "ZCMD"      ||
				      pfield->field_area  != i ||
				     !pfield->field_value_blank() )
				{
					continue ;
				}
				if ( ( zz_row >  pfield->field_row ) ||
				     ( zz_row == pfield->field_row &&
				       zz_col >  pfield->field_col ) )
				{
					zz_row = pfield->field_row ;
					zz_col = pfield->field_col ;
					f_name = it->first ;
				}
			}
		}
		if ( f_name == "" )
		{
			zz_row = -1 ;
			zz_col = -1 ;
			for ( auto it = fieldList.begin() ; it != fieldList.end() ; ++it )
			{
				pfield = it->second ;
				if ( !pfield->field_active  ||
				     !pfield->field_visible ||
				     !pfield->field_is_input() )
				{
					continue ;
				}
				if ( ( zz_row >  pfield->field_row ) ||
				     ( zz_row == pfield->field_row &&
				       zz_col >  pfield->field_col ) )
				{
					zz_row = pfield->field_row ;
					zz_col = pfield->field_col ;
					f_name = it->first ;
				}
			}
		}
	}

	if ( f_name != "" )
	{
		cursor_placement( err, f_name, f_pos ) ;
	}
}


void pPanel::cursor_placement_tbdispl( errblock& err )
{
	//
	// Position cursor for TBDISPL service.
	//
	// Cursor placement for TBDISPL:
	//        CURSOR/CSRROW    - Place on row with CRN of CSRROW.
	//        CURSOR/no CSRROW - Use CRP to indicate row.  If CRP=0, place on the command line.
	//     no CURSOR/CSRROW    - First field of row.
	//     no CURSOR/no CSRROW - Place on the command line.
	//
	// If no command line, use the first top left unprotected field.
	//

	TRACE_FUNCTION() ;

	int    f_pos = 1 ;
	string f_name ;

	if ( bypassCur ) { return ; }

	int    csrpos = get_csrpos() ;
	string cursor = get_cursor() ;

	l_row = 0 ;
	l_col = 0 ;

	if ( cursor == "" )
	{
		if ( tb_csrrow > 0 )
		{
			f_name = get_tbCursor( get_first_tb_field() ) ;
		}
		else
		{
			f_name = cmdfield ;
			f_pos  = 1 ;
		}
	}
	else if ( fieldList.count( cursor ) > 0 )
	{
		f_name = cursor ;
		f_pos  = csrpos ;
	}
	else if ( !tb_field( cursor ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE022N", cursor, !isvalidName( cursor ) ? 20 : 12 ) ;
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
	else if ( cmdfield != "" )
	{
		f_name = cmdfield ;
	}
	else if ( ztdvrows > 0 )
	{
		f_name = get_tbCursor( get_first_tb_field(), 1 ) ;
	}

	if ( f_name != "" )
	{
		cursor_placement( err, f_name, f_pos ) ;
	}
	else
	{
		cursor_top_unprot( l_row, l_col ) ;
	}
}


void pPanel::cursor_placement( errblock& err,
			       const string& f_name,
			       int f_pos )
{
	//
	// Calculate the panel l_row and l_col co-ordinates from the passed field name and offset.
	//

	TRACE_FUNCTION() ;

	uint oX ;
	uint oY ;

	field* pfield ;

	dynArea* da ;

	Area* pArea ;

	map<string, field*>::iterator itf ;
	map<string, dynArea*>::iterator itd ;

	itf = fieldList.find( f_name ) ;
	if ( itf == fieldList.end() )
	{
		itd = dynAreaList.find( f_name ) ;
		if ( itd == dynAreaList.end() )
		{
			err.seterrid( TRACE_INFO(), "PSYE022N", f_name, !isvalidName( f_name ) ? 20 : 12 ) ;
			return ;
		}
		da = itd->second ;
		if ( f_pos < 1 ) { f_pos = 1 ; }
		--f_pos ;
		oX = f_pos % da->dynArea_width ;
		oY = f_pos / da->dynArea_width ;
		if ( oY >= da->dynArea_depth )
		{
			oX = 0 ;
			oY = 0 ;
		}
		l_col = da->dynArea_col + oX ;
		l_row = da->dynArea_row + oY ;
	}
	else
	{
		pfield = itf->second ;
		if ( f_pos < 1 || size_t( f_pos ) > pfield->field_length )
		{
			f_pos = 1 ;
		}
		if ( pfield->field_area > 0 )
		{
			pArea = AreaNum[ pfield->field_area ] ;
			if ( !pfk_built )
			{
				build_pfkeys( err, true ) ;
			}
			pArea->set_visible_depth( get_areared_lines( pArea ) ) ;
			if ( !pfield->field_visible || !pArea->in_visible_area( pfield ) )
			{
				pArea->make_visible( pfield ) ;
				rebuild_after_area_scroll( pArea ) ;
			}
		}
		l_row = pfield->field_row ;
		l_col = pfield->field_col + f_pos - 1 ;
	}
}


string pPanel::get_first_tb_field()
{
	//
	// Return the left-most field in a TBMODEL set.
	//

	TRACE_FUNCTION() ;

	uint i = -1 ;

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
	TRACE_FUNCTION() ;

	map<string, field*>::iterator it ;

	l_row = 0 ;
	l_col = 0 ;

	field* pfield ;

	if ( cmdfield == "" ) { return ; }

	it = fieldList.find( cmdfield ) ;

	pfield = it->second ;

	if ( f_pos < 1 || f_pos > pfield->field_length ) { f_pos = 1 ; }

	l_col = pfield->field_col + f_pos - 1 ;
	l_row = pfield->field_row ;
}


void pPanel::get_home1( uint& row,
			uint& col )
{
	//
	// Return the physical home position on the screen.
	// If none defined, find the first top/left unprotected field.
	//

	TRACE_FUNCTION() ;

	map<string, field*>::iterator it ;

	field* pfield ;

	if ( home == "" )
	{
		cursor_top_unprot( row, col ) ;
	}
	else
	{
		it     = fieldList.find( home ) ;
		pfield = it->second ;
		row    = pfield->field_row ;
		col    = pfield->field_col ;
	}

	row += win_row ;
	col += win_col ;
}


void pPanel::get_home2( uint& row,
			uint& col )
{
	//
	// Toggle between the physical home position on the screen and the previous position when home was first pressed.
	//

	TRACE_FUNCTION() ;

	uint i ;
	uint j ;

	get_home1( i, j ) ;

	if ( row == i && col == j )
	{
		if ( s_row > 0 && s_col > 0 )
		{
			row = s_row + win_row ;
			col = s_col + win_col ;
		}
	}
	else
	{
		s_row = row - win_row ;
		s_col = col - win_col ;
		row   = i ;
		col   = j ;
	}

}


void pPanel::set_lcursor_home()
{
	//
	// Set the cursor to the relative home position.
	//

	TRACE_FUNCTION() ;

	map<string, field*>::iterator it ;

	field* pfield ;

	dCursor = home ;
	dCsrpos = 1 ;

	if ( home == "" )
	{
		cursor_top_unprot( l_row, l_col ) ;
	}
	else
	{
		it = fieldList.find( home ) ;
		pfield = it->second ;
		l_row  = pfield->field_row ;
		l_col  = pfield->field_col ;
	}
}


void pPanel::set_lcursor_scroll()
{
	//
	// Set the cursor to the relative scroll position.
	//

	TRACE_FUNCTION() ;

	map<string, field*>::iterator it ;

	field* pfield ;

	if ( scroll == "" ) { return ; }

	dCursor = scroll ;
	dCsrpos = 1 ;

	it     = fieldList.find( scroll ) ;
	pfield = it->second ;
	l_row  = pfield->field_row ;
	l_col  = pfield->field_col ;
}


field* pPanel::get_field_address( uint row,
				  uint col )
{
	//
	// Return the field address at position row,col.
	//

	TRACE_FUNCTION() ;

	return fieldAddrs[ fieldMap[ row * zscrmaxw + col ] ] ;
}


const string& pPanel::field_getvalue( const string& f_name )
{
	TRACE_FUNCTION() ;

	field* pfield ;

	map<string, field*>::iterator it = fieldList.find( f_name ) ;

	pfield = it->second ;

	pfield->field_prep_input() ;

	return pfield->field_get_value() ;
}


const string& pPanel::field_getvalue_caps( const string& f_name )
{
	TRACE_FUNCTION() ;

	field* pfield ;

	map<string, field*>::iterator it = fieldList.find( f_name ) ;

	pfield = it->second ;

	pfield->field_prep_input() ;
	pfield->field_apply_caps() ;

	return pfield->field_get_value() ;
}


const string& pPanel::field_getrawvalue( const string& f_name )
{
	TRACE_FUNCTION() ;

	map<string, field*>::iterator it = fieldList.find( f_name ) ;

	field* pfield = it->second ;

	if ( pfield->field_is_scrollable() )
	{
		pfield->field_update_ssvalue() ;
	}

	return pfield->field_get_value() ;
}


void pPanel::field_setvalue( const string& f_name,
			     const string& f_value )
{
	TRACE_FUNCTION() ;

	field* pfield ;

	map<string, field*>::iterator it = fieldList.find( f_name ) ;

	pfield = it->second ;

	if ( pfield->field_is_input() )
	{
		pfield->field_put_value( f_value ) ;
		pfield->field_changed = true ;
		pfield->field_prep_display() ;
		pfield->display_field( win, inv_schar, ddata_map, schar_map ) ;
	}
}


void pPanel::field_setvalue( field* pfield,
			     const string& f_value )
{
	TRACE_FUNCTION() ;

	pfield->field_put_value( f_value ) ;
	pfield->field_changed = true ;
	pfield->field_prep_display() ;
	pfield->display_field( win, inv_schar, ddata_map, schar_map ) ;
}


const string& pPanel::cmd_getvalue()
{
	TRACE_FUNCTION() ;

	return ( cmdfield == "" ) ? zzcmd : field_getvalue( cmdfield ) ;
}


void pPanel::cmd_setvalue( const string& v )
{
	TRACE_FUNCTION() ;

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
	TRACE_FUNCTION() ;

	return ( strip( cmd_getvalue() ) != "" ) ;
}


bool pPanel::keep_cmd()
{
	//
	// Return true to keep the command line and/or displayed message.
	// Also set error_msg to false so old errors do not stop the command stack from executing.
	//
	// Command starts with '&' - keep
	// else errors from primary command - clear
	// else command line blank, SWAP x or SPLIT x - keep
	// else command line non-blank - clear.
	//

	TRACE_FUNCTION() ;

	string w1 ;

	if ( cmdfield == "" )
	{
		return true ;
	}

	field* pfield = fieldList[ cmdfield ] ;

	pfield->field_prep_input() ;
	if ( !pfield->field_value_blank() && pfield->field_get_value().front() == '&' )
	{
		return true ;
	}

	if ( error_msg )
	{
		error_msg = false ;
		if ( get_cursor() == cmdfield )
		{
			return false ;
		}
	}

	w1 = upper( word( pfield->field_get_value(), 1 ) ) ;
	if ( w1 == "" || w1 == "SPLIT" || w1 == "SWAP" )
	{
		field_setvalue( pfield, "" ) ;
		return true ;
	}

	return false ;
}


void pPanel::reset_cmd()
{
	TRACE_FUNCTION() ;

	errblock err ;

	if ( cmdfield != "" )
	{
		field_setvalue( cmdfield, getDialogueVar( err, cmdfield ) ) ;
	}
}


bool pPanel::is_cmd_inactive( const string& cmd,
			      uint row,
			      uint col )
{
	//
	// Some SETVERB commands may be inactive for this type of panel/field, so return true for these.
	// Not sure how real ISPF does this without setting ZCTACT to NOP !! but this works okay.
	//
	// Control variable .EDIT=YES activates RFIND and RCHANGE for a panel.
	// Control variable .BROWSE=YES activates RFIND for a panel.
	//

	TRACE_FUNCTION() ;

	field* pfield ;

	if ( pd_active() )
	{
		return ( !findword( cmd, "END RETURN" ) ) ;
	}

	if ( tutorial && findword( cmd, "UP DOWN LEFT RIGHT" ) )
	{
		return false ;
	}

	if ( sfields.size() > 0 && findword( cmd, "LEFT RIGHT ZEXPAND ZCLRSFLD" ) )
	{
		pfield = field_getaddr_any( row, col ) ;
		if ( pfield && pfield->field_scroll_on() )
		{
			return false ;
		}
	}

	if ( !histories.empty() && cmd == "ZHISTORY" )
	{
		pfield = field_getaddr_any( row, col ) ;
		if ( pfield && histories.count( pfield ) > 0 )
		{
			return false ;
		}
	}

	if ( findword( cmd, "LEFT RIGHT" ) )
	{
		if ( lrScroll ) { return false ; }
		if ( tb_model ) { return true  ; }
		if ( scrollOn ) { return false ; }
		return true ;
	}
	else if ( findword( cmd, "UP DOWN" ) )
	{
		if ( !AreaList.empty() || scrollOn ) { return false ; }
		return true ;
	}
	else if ( findword( cmd, "ZEXPAND ZCLRSFLD ZHISTORY" ) )
	{
		return true ;
	}
	else if ( cmd == "NRETRIEV" && !nretriev ) { return true ; }
	else if ( cmd == "RFIND"    && ( !forEdit && !forBrowse ) ) { return true ; }
	else if ( cmd == "RCHANGE"  && !forEdit   ) { return true ; }

	return false ;
}


bool pPanel::field_exists( const string& name )
{
	//
	// Return true if field exists.
	//

	TRACE_FUNCTION() ;

	return ( fieldList.count( name ) > 0 ) ;
}


string pPanel::field_getname( uint row,
			      uint col )
{
	//
	// Get field name the cursor is on (just normal input/output fields are selected, including for table displays).
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	//

	TRACE_FUNCTION() ;

	if ( cursor_not_in_window( row, col ) ) { return "" ; }

	row -= win_row ;
	col -= win_col ;

	field* pfield = get_field_address( row, col ) ;

	return ( pfield && pfield->field_active && !pfield->field_is_dynamic() ) ? addr2field[ pfield ] : "" ;
}


string pPanel::field_getname_input( uint row,
				    uint col )
{
	//
	// Get field name the cursor is on (just normal input fields are selected).
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	//

	TRACE_FUNCTION() ;

	if ( cursor_not_in_window( row, col ) ) { return "" ; }

	row -= win_row ;
	col -= win_col ;

	field* pfield = get_field_address( row, col ) ;

	return ( pfield && pfield->field_active && pfield->field_is_input() && !pfield->field_is_dynamic() && !pfield->field_is_tbdispl() ) ?
		 addr2field[ pfield ] : "" ;
}


field* pPanel::field_getaddr_any( uint row,
				  uint col )
{
	//
	// Get field address the cursor is on (field just needs to be active).
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	//

	TRACE_FUNCTION() ;

	if ( cursor_not_in_window( row, col ) ) { return nullptr ; }

	row -= win_row ;
	col -= win_col ;

	field* pfield = get_field_address( row, col ) ;

	return ( pfield && pfield->field_active ) ? pfield : nullptr ;
}


string pPanel::field_getname_any( field* addr )
{
	//
	// Get field name for address addr (field just needs to be active).
	//

	TRACE_FUNCTION() ;

	return ( addr->field_active ) ? addr2field[ addr ] : "" ;
}


string pPanel::field_getname_nocmd()
{
	//
	// Return the field the application cursor is on (blank if it is on the command field).
	//

	TRACE_FUNCTION() ;

	string temp ;

	field* pfield = get_field_address( l_row, l_col ) ;

	if ( pfield && pfield->field_active && pfield->field_is_input() && !pfield->field_is_dynamic() && !pfield->field_is_tbdispl() )
	{
		temp = addr2field[ pfield ] ;
		if ( temp == cmdfield ) { temp = "" ; }
	}

	return temp ;
}


bool pPanel::cursor_on_field( const string& name,
			      uint row,
			      uint col )
{
	//
	// Return true if field exists and cursor is placed on it.
	// Passed row/col is the physical position on the screen.
	//

	TRACE_FUNCTION() ;

	auto it = fieldList.find( name ) ;
	if ( it == fieldList.end() || !it->second->field_active ) { return false ; }

	return ( it->second->cursor_on_field( row - win_row, col - win_col ) ) ;
}


bool pPanel::field_is_input( const string& name )
{
	//
	// Return true if field is an input field.
	//

	TRACE_FUNCTION() ;

	auto it = fieldList.find( name ) ;
	if ( it == fieldList.end() ) { return false ; }

	return it->second->field_is_input() ;
}


bool pPanel::field_get_row_col( const string& name,
				uint& row,
				uint& col )
{
	//
	// If field found on panel (by name), return true and its position, else return false.
	// Return the physical position on the screen, so add the window offsets to field_row/col.
	//

	TRACE_FUNCTION() ;

	field* pfield ;

	auto it = fieldList.find( name ) ;
	if ( it == fieldList.end() || !it->second->field_active ) { return false ; }

	pfield = it->second ;

	row = pfield->field_row + win_row ;
	col = pfield->field_col + win_col ;

	return true ;
}


int pPanel::field_get_col( const string& name )
{
	//
	// If field found on panel (by name) and is active, return column position.
	// Return the physical position on the screen, so add the window offset to field_col.
	//

	TRACE_FUNCTION() ;

	auto it = fieldList.find( name ) ;
	if ( it != fieldList.end() && it->second->field_active )
	{
		return it->second->field_col + win_col ;
	}

	return 0 ;
}


uint pPanel::field_get_scroll_pos( const string& name )
{
	//
	// If field is scrollable, return the start position.
	//

	TRACE_FUNCTION() ;

	auto it = fieldList.find( name ) ;
	if ( it != fieldList.end() && it->second->field_active && it->second->field_is_scrollable() )
	{
		return it->second->field_get_start() ;
	}

	return 0 ;
}


bool pPanel::field_nonblank( const string& name,
			     uint p )
{
	//
	// Return true if there is a nonblank character at position p in the field value.
	//

	TRACE_FUNCTION() ;

	if ( p == 0 ) { return true ; }

	auto it = fieldList.find( name ) ;
	field* pfield = it->second ;

	if ( pfield->field_value.size() > p )
	{
		return ( pfield->field_value.compare( p, 1, " " ) > 0 ) ;
	}

	return false ;
}


fieldXct pPanel::field_getexec( const string& field )
{
	//
	// If passed field is in the field execute table, return the structure fieldXct
	// for that field as defined in )FIELD panel section.
	//

	TRACE_FUNCTION() ;

	auto it = field_xct_table.find( field ) ;

	return ( it == field_xct_table.end() ) ? fieldXct() : *it->second ;
}


void pPanel::field_edit( uint row,
			 uint col,
			 char ch,
			 bool Isrt,
			 bool& prot )
{
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	// field_tab_next also needs the physical position, so adjust before and after the call.
	//

	TRACE_FUNCTION() ;

	prot = true ;

	if ( row < win_row || col < win_col )
	{
		l_row = row - win_row ;
		l_col = col - win_col ;
		return ;
	}

	row  -= win_row ;
	col  -= win_col ;

	if ( row >= zscrmaxd || col >= zscrmaxw )
	{
		return ;
	}

	l_row = row ;
	l_col = col ;

	if ( ( win == pwin && l_row >= win_depth - cvd_lines ) ||
	     ( l_row >= zscrmaxd - cvd_lines ) ||
	     ( cursor_on_msg_window( row+win_row, col+win_col ) ) )
	{
		return ;
	}

	field* pfield = get_field_address( row, col ) ;
	if ( pfield && pfield->field_active )
	{
		if ( (  pfield->field_is_numeric() && ch != ' ' && !isdigit( ch ) ) ||
		     ( !pfield->field_is_dynamic() &&  pfield->field_is_prot( tabpas )  ) ||
		     (  pfield->field_is_dynamic() && !pfield->field_dyna_input( col ) ) ) { return ; }
		if ( Isrt )
		{
			if ( !pfield->edit_field_insert( win, ch, inv_schar, col, prot, ddata_map, schar_map ) )
			{
				return ;
			}
		}
		else
		{
			if ( !pfield->edit_field_replace( win, ch, inv_schar, col, ddata_map, schar_map ) )
			{
				return ;
			}
		}
		prot = false ;
		++l_col ;
		if ( ( pfield->field_is_dynamic() || pfield->field_is_skip() ) && l_col == ( pfield->field_endcol + 1 ) )
		{
			l_row += win_row ;
			l_col += win_col ;
			field_tab_next( l_row, l_col ) ;
			l_row -= win_row ;
			l_col -= win_col ;
		}
	}
}


void pPanel::field_backspace( uint& row,
			      uint& col,
			      bool& prot )
{
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	// Return physical position of the cursor in row/col.
	//

	TRACE_FUNCTION() ;

	uint trow ;
	uint tcol ;
	uint p    ;

	prot = true ;

	if ( cursor_not_in_window( row, col ) )
	{
		return ;
	}

	trow = row - win_row ;
	tcol = col - win_col ;

	if ( ( win == pwin && trow >= win_depth - cvd_lines ) ||
	     ( trow >= zscrmaxd - cvd_lines ) ||
	     ( cursor_on_msg_window( row, col ) ) )
	{
		return ;
	}

	field* pfield = get_field_address( trow, tcol ) ;
	if ( pfield && pfield->field_active && tcol != pfield->field_col )
	{
		if ( ( !pfield->field_is_dynamic() &&  pfield->field_is_prot( tabpas ) ) ||
		     (  pfield->field_is_dynamic() && !pfield->field_dyna_input( tcol ) ) ) { return ; }
		p    = tcol ;
		tcol = pfield->edit_field_backspace( win, tcol, inv_schar, ddata_map, schar_map ) ;
		if ( p != tcol )
		{
			prot = false ;
			row  = trow + win_row ;
			col  = tcol + win_col ;
		}
	}
}


void pPanel::field_delete_char( uint row,
				uint col,
				bool& prot )
{
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	// Return physical position of the cursor in row/col.
	//

	TRACE_FUNCTION() ;

	uint trow ;
	uint tcol ;

	prot = true ;

	if ( cursor_not_in_window( row, col ) )
	{
		return ;
	}

	trow = row - win_row ;
	tcol = col - win_col ;

	if ( ( win == pwin && trow >= win_depth - cvd_lines ) ||
	     ( trow >= zscrmaxd - cvd_lines ) ||
	     ( cursor_on_msg_window( row, col ) ) )
	{
		return ;
	}

	field* pfield = get_field_address( trow, tcol ) ;
	if ( pfield && pfield->field_active )
	{
		if ( ( !pfield->field_is_dynamic() &&  pfield->field_is_prot( tabpas ) ) ||
		     (  pfield->field_is_dynamic() && !pfield->field_dyna_input( tcol ) ) ) { return ; }
		pfield->edit_field_delete( win, tcol, inv_schar, ddata_map, schar_map ) ;
		prot = false ;
	}
}


void pPanel::field_erase_eof( uint row,
			      uint col,
			      bool& prot,
			      bool end_os )
{
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	//

	TRACE_FUNCTION() ;

	prot = true ;

	if ( cursor_not_in_window( row, col ) )
	{
		return ;
	}

	row -= win_row ;
	col -= win_col ;

	if ( ( win == pwin && row >= win_depth - cvd_lines ) ||
	     ( row >= zscrmaxd - cvd_lines ) ||
	     ( cursor_on_msg_window( row+win_row, col+win_col ) ) )
	{
		return ;
	}

	field* pfield = get_field_address( row, col ) ;
	if ( pfield && pfield->field_active )
	{
		if ( ( !pfield->field_is_dynamic() &&  pfield->field_is_prot( tabpas ) ) ||
		     (  pfield->field_is_dynamic() && !pfield->field_dyna_input( col ) ) ) { return ; }
		pfield->field_erase_eof( win, col, inv_schar, ddata_map, schar_map, end_os ) ;
		prot = false ;
	}
}


void pPanel::field_erase_spaces( uint row,
				 uint col,
				 bool& prot )
{
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	//

	TRACE_FUNCTION() ;

	prot = true ;

	if ( cursor_not_in_window( row, col ) )
	{
		return ;
	}

	row -= win_row ;
	col -= win_col ;

	if ( ( win == pwin && row >= win_depth - cvd_lines ) ||
	     ( row >= zscrmaxd - cvd_lines ) ||
	     ( cursor_on_msg_window( row+win_row, col+win_col ) ) )
	{
		return ;
	}

	field* pfield = get_field_address( row, col ) ;
	if ( pfield && pfield->field_active )
	{
		if ( ( !pfield->field_is_dynamic() &&  pfield->field_is_prot( tabpas ) ) ||
		     (  pfield->field_is_dynamic() && !pfield->field_dyna_input( col ) ) ) { return ; }
		pfield->field_erase_spaces( win, col, inv_schar, ddata_map, schar_map ) ;
		prot = false ;
	}
}


void pPanel::field_erase_word( uint row,
			       uint col,
			       bool& prot )
{
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	//

	TRACE_FUNCTION() ;

	prot = true ;

	if ( cursor_not_in_window( row, col ) )
	{
		return ;
	}

	row -= win_row ;
	col -= win_col ;

	if ( ( win == pwin && row >= win_depth - cvd_lines ) ||
	     ( row >= zscrmaxd - cvd_lines ) ||
	     ( cursor_on_msg_window( row+win_row, col+win_col ) ) )
	{
		return ;
	}

	field* pfield = get_field_address( row, col ) ;
	if ( pfield && pfield->field_active )
	{
		if ( ( !pfield->field_is_dynamic() &&  pfield->field_is_prot( tabpas ) ) ||
		     (  pfield->field_is_dynamic() && !pfield->field_dyna_input( col ) ) ) { return ; }
		pfield->field_erase_word( win, col, inv_schar, ddata_map, schar_map ) ;
		prot = false ;
	}
}


void pPanel::cursor_eof( uint row,
			 uint& col )
{
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	// Return physical position of the cursor in row/col.
	//

	TRACE_FUNCTION() ;

	uint trow ;
	uint tcol ;

	if ( cursor_not_in_window( row, col ) )
	{
		return ;
	}

	trow = row - win_row ;
	tcol = col - win_col ;

	if ( ( win == pwin && trow >= win_depth - cvd_lines ) ||
	     ( trow >= zscrmaxd - cvd_lines ) ||
	     ( cursor_on_msg_window( row, col ) ) )
	{
		return ;
	}

	field* pfield = get_field_address( trow, tcol ) ;
	if ( pfield && pfield->field_active )
	{
		if ( ( !pfield->field_is_dynamic() &&  pfield->field_is_prot( tabpas ) ) ||
		     (  pfield->field_is_dynamic() && !pfield->field_dyna_input( tcol ) ) ) { return ; }
		col = pfield->end_of_field( win, tcol ) + win_col ;
	}
}


void pPanel::cursor_sof( uint row,
			 uint& col )
{
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	// Return physical position of the cursor in row/col.
	//

	TRACE_FUNCTION() ;

	uint trow ;
	uint tcol ;

	if ( cursor_not_in_window( row, col ) )
	{
		return ;
	}

	trow = row - win_row ;
	tcol = col - win_col ;

	if ( ( win == pwin && trow >= win_depth - cvd_lines ) ||
	     ( trow >= zscrmaxd - cvd_lines ) ||
	     ( cursor_on_msg_window( row, col ) ) )
	{
		return ;
	}

	field* pfield = get_field_address( trow, tcol ) ;
	if ( pfield && pfield->field_active )
	{
		if ( ( !pfield->field_is_dynamic() &&  pfield->field_is_prot( tabpas ) ) ||
		     (  pfield->field_is_dynamic() && !pfield->field_dyna_input( tcol ) ) ) { return ; }
		col = pfield->start_of_field( win, tcol ) + win_col ;
	}
}


void pPanel::cursor_first_sof( uint row,
			       uint& col )
{
	//
	// Get the cursor position of the start of the first input field on a line.
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	// Return physical column position of the cursor in col.
	//

	TRACE_FUNCTION() ;

	field* pfield ;
	field* lfield = nullptr ;

	uint zz_col = -1 ;

	int i ;

	if ( cursor_not_in_window( row, col ) )
	{
		return ;
	}

	row -= win_row ;

	for ( auto& f : fieldList )
	{
		pfield = f.second ;
		if ( pfield->field_row == row   &&
		     pfield->field_col < zz_col &&
		     pfield->field_active       &&
		     pfield->field_visible      &&
		   ( pfield->field_is_input() || pfield->field_is_dynamic() ) )
		{
			zz_col = pfield->field_col ;
			lfield = pfield ;
		}
	}

	if ( !lfield )
	{
		return ;
	}

	if ( lfield->field_is_dynamic() )
	{
		i = lfield->field_dyna_input_offset( zz_col ) ;
		if ( i != -1 )
		{
			col = lfield->field_col + win_col + i ;
		}
	}
	else
	{
		col = lfield->field_col + win_col ;
	}
}


void pPanel::cursor_top_unprot( uint& row,
				uint& col )
{
	//
	// Find the relative row/col of the top unprotected field on a panel.
	// If none, return 0,0.
	//

	TRACE_FUNCTION() ;

	int j ;

	uint depth = ( ( win == pwin ) ? win_depth - cvd_lines : zscrmaxd - cvd_lines ) ;

	field* pfield ;

	col = -1 ;

	for ( uint i = 0 ; i < depth ; ++i )
	{
		for ( auto& f : fieldList )
		{
			pfield = f.second ;
			if ( pfield->field_row == i  &&
			     pfield->field_col < col &&
			     pfield->field_active    &&
			     pfield->field_visible )
			{
				if ( pfield->field_is_input() )
				{
					col = pfield->field_col ;
					row = i ;
				}
				else if ( pfield->field_is_dynamic() )
				{
					j = pfield->field_dyna_input_offset( 0 ) ;
					if ( j != -1 )
					{
						col = pfield->field_col + j ;
						row = i ;
					}
				}
			}
		}
		if ( col != uint ( -1 ) )
		{
			return ;
		}
	}

	row = 0 ;
	col = 0 ;
}


bool pPanel::cursor_next_word( uint row,
			       uint& col )
{
	//
	// Move cursor to the next word in a field.
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	// Return physical position of the cursor in row/col.
	//
	// Return:  True if cursor has moved, false otherwise.
	//

	TRACE_FUNCTION() ;

	uint trow ;
	uint tcol ;

	if ( cursor_not_in_window( row, col ) )
	{
		return false ;
	}

	trow = row - win_row ;
	tcol = col - win_col ;

	if ( ( win == pwin && trow >= win_depth - cvd_lines ) ||
	     ( trow >= zscrmaxd - cvd_lines ) ||
	     ( cursor_on_msg_window( row, col ) ) )
	{
		return false ;
	}

	field* pfield = get_field_address( trow, tcol ) ;
	if ( pfield && pfield->field_active )
	{
		if ( ( !pfield->field_is_dynamic() &&  pfield->field_is_prot( tabpas ) ) ||
		     (  pfield->field_is_dynamic() && !pfield->field_dyna_input( tcol ) ) ) { return false ; }
		col = pfield->field_next_word( win, tcol ) + win_col ;
	}

	return ( col != tcol ) ;
}


bool pPanel::cursor_prev_word( uint row,
			       uint& col )
{
	//
	// Move cursor to the previous word in a field.
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	// Return physical position of the cursor in row/col.
	//
	// Return:  True if cursor has moved, false otherwise.
	//

	TRACE_FUNCTION() ;

	uint trow ;
	uint tcol ;

	if ( cursor_not_in_window( row, col ) )
	{
		return false ;
	}

	trow = row - win_row ;
	tcol = col - win_col ;

	if ( ( win == pwin && trow >= win_depth - cvd_lines ) ||
	     ( trow >= zscrmaxd - cvd_lines ) ||
	     ( cursor_on_msg_window( row, col ) ) )
	{
		return false ;
	}

	field* pfield = get_field_address( trow, tcol ) ;
	if ( pfield && pfield->field_active )
	{
		if ( ( !pfield->field_is_dynamic() &&  pfield->field_is_prot( tabpas ) ) ||
		     (  pfield->field_is_dynamic() && !pfield->field_dyna_input( tcol ) ) ) { return false ; }
		col = pfield->field_prev_word( win, tcol ) + win_col ;
	}

	return ( col != tcol ) ;
}


bool pPanel::cursor_first_word( uint row,
				uint& col )
{
	//
	// Move cursor to the first word in a field.
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	// Return physical position of the cursor in row/col.
	//
	// Return:  True if cursor has moved, false otherwise.
	//

	TRACE_FUNCTION() ;

	uint trow ;
	uint tcol ;

	if ( cursor_not_in_window( row, col ) )
	{
		return false ;
	}

	trow = row - win_row ;
	tcol = col - win_col ;

	if ( ( win == pwin && trow >= win_depth - cvd_lines ) ||
	     ( trow >= zscrmaxd - cvd_lines ) ||
	     ( cursor_on_msg_window( row, col ) ) )
	{
		return false ;
	}

	field* pfield = get_field_address( trow, tcol ) ;
	if ( pfield && pfield->field_active )
	{
		if ( ( !pfield->field_is_dynamic() &&  pfield->field_is_prot( tabpas ) ) ||
		     (  pfield->field_is_dynamic() && !pfield->field_dyna_input( tcol ) ) ) { return false ; }
		col = pfield->field_first_word( win, tcol ) + win_col ;
	}

	return ( col != tcol ) ;
}


bool pPanel::cursor_last_word( uint row,
			       uint& col )
{
	//
	// Move cursor to the last word in a field.
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	// Return physical position of the cursor in row/col.
	//
	// Return:  True if cursor has moved, false otherwise.
	//

	TRACE_FUNCTION() ;

	uint trow ;
	uint tcol ;

	if ( cursor_not_in_window( row, col ) )
	{
		return false ;
	}

	trow = row - win_row ;
	tcol = col - win_col ;

	if ( ( win == pwin && trow >= win_depth - cvd_lines ) ||
	     ( trow >= zscrmaxd - cvd_lines ) ||
	     ( cursor_on_msg_window( row, col ) ) )
	{
		return false ;
	}

	field* pfield = get_field_address( trow, tcol ) ;
	if ( pfield && pfield->field_active )
	{
		if ( ( !pfield->field_is_dynamic() &&  pfield->field_is_prot( tabpas ) ) ||
		     (  pfield->field_is_dynamic() && !pfield->field_dyna_input( tcol ) ) ) { return false ; }
		col = pfield->field_last_word( win, tcol ) + win_col ;
	}

	return ( col != tcol ) ;
}


void pPanel::set_pcursor( uint row,
			  uint col )
{
	//
	// Set the logical position of the cursor from the physical position.
	// If not inside the window and not on the border, set to the home position.
	//

	TRACE_FUNCTION() ;

	if ( cursor_in_window_border( row, col ) )
	{
		l_row = row - win_row ;
		l_col = col - win_col ;
	}
	else
	{
		set_lcursor_home() ;
	}
}


void pPanel::get_pcursor( uint& row,
			  uint& col )
{
	//
	// Retrieve the physical position of the cursor.
	//

	TRACE_FUNCTION() ;

	row = l_row + win_row ;
	col = l_col + win_col ;
}


void pPanel::set_lcursor( uint row,
			  uint col )
{
	//
	// Set the logical position of the cursor.  If not within the window, set it to the home position.
	//

	TRACE_FUNCTION() ;

	if ( row < wscrmaxd && col < wscrmaxw )
	{
		l_row = row ;
		l_col = col ;
	}
	else
	{
		set_lcursor_home() ;
	}
}


void pPanel::get_lcursor( uint& row,
			  uint& col )
{
	//
	// Retrieve the logical position of the cursor.
	//

	TRACE_FUNCTION() ;

	row = l_row ;
	col = l_col ;
}


void pPanel::cursor( uint row,
		     uint col )
{
	//
	// Move cursor to primary input field or scroll field if already on the primary input field.
	// row/col is the physical position on the screen.
	//

	TRACE_FUNCTION() ;

	uint h_row ;
	uint h_col ;

	get_home1( h_row, h_col ) ;

	if ( h_row == 0 && h_col == 0 ) { return ; }

	if ( h_row == row && h_col == col )
	{
		set_lcursor_scroll() ;
	}
	else
	{
		set_lcursor_home() ;
	}
}


void pPanel::field_tab_down( uint& row,
			     uint& col )
{
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	// Return physical position of the cursor in row/col.
	// (get_home returns physical position on the screen).
	//

	TRACE_FUNCTION() ;

	int t_offset ;
	int m_offset ;
	int c_offset ;
	int d_offset ;

	uint o_row ;

	uint trow ;
	uint tcol ;

	bool cursor_moved = false ;

	field* pfield ;

	trow = row - win_row ;
	tcol = col - win_col ;

	map<string, field*>::iterator it;

	c_offset = trow * wscrmaxw + tcol ;
	m_offset = wscrmaxd * wscrmaxw ;
	o_row    = trow ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; ++it )
	{
		pfield = it->second ;
		if ( !pfield->field_active) { continue ; }
		if ( !pfield->field_is_dynamic() && pfield->field_is_prot( tabpas ) ) { continue ; }
		if (  pfield->field_row <= o_row ) { continue ; }
		if ( ( win == pwin && pfield->field_row >= ( win_depth - cvd_lines ) ) ||
		     ( pfield->field_row >= zscrmaxd - cvd_lines ) ||
		     ( cursor_on_msg_window( pfield->field_row + win_row, pfield->field_col + win_col  ) ) )
		{
			continue ;
		}
		d_offset = 0 ;
		if ( pfield->field_is_dynamic() )
		{
			if ( pfield->field_da_ext->field_ext1_dynArea->dynArea_inAttrs == "" )
			{
				continue ;
			}
			d_offset = pfield->field_dyna_input_offset( 0 ) ;
			if ( d_offset == -1 ) { continue ; }
		}
		t_offset = pfield->field_row * wscrmaxw + pfield->field_col + d_offset ;
		if ( t_offset > c_offset && t_offset < m_offset )
		{
			m_offset     = t_offset ;
			row          = pfield->field_row + win_row ;
			col          = pfield->field_col + d_offset + win_col ;
			cursor_moved = true ;
		}
	}

	if ( !cursor_moved ) { get_home1( row, col ) ; }
}



void pPanel::field_tab_next( uint& row,
			     uint& col )
{
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	// Return physical position of the cursor in row/col.
	// (get_home returns physical position on the screen).
	//

	TRACE_FUNCTION() ;

	uint t_offset ;
	uint m_offset ;
	uint c_offset ;
	int  d_offset ;

	uint o_row ;
	uint o_col ;
	uint trow ;
	uint tcol ;

	bool cursor_moved = false ;

	field* pfield ;

	trow = row - win_row ;
	tcol = col - win_col ;

	map<string, field*>::iterator it;

	c_offset = trow * wscrmaxw + tcol ;
	m_offset = wscrmaxd * wscrmaxw ;
	o_row    = trow ;
	o_col    = tcol ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; ++it )
	{
		pfield = it->second ;
		if ( !pfield->field_active) { continue ; }
		if ( !pfield->field_is_dynamic() && pfield->field_is_prot( tabpas ) ) { continue ; }
		if ( ( win == pwin && pfield->field_row >= ( win_depth - cvd_lines ) ) ||
		     ( pfield->field_row >= zscrmaxd - cvd_lines ) ||
		     ( cursor_on_msg_window( pfield->field_row + win_row, pfield->field_col + win_col  ) ) )
		{
			continue ;
		}
		d_offset = 0 ;
		if ( pfield->field_is_dynamic() )
		{
			if ( pfield->field_da_ext->field_ext1_dynArea->dynArea_inAttrs == "" )
			{
				continue ;
			}
			if ( o_row == pfield->field_row )
			{
				d_offset = pfield->field_dyna_input_offset( o_col ) ;
			}
			else
			{
				d_offset = pfield->field_dyna_input_offset( 0 ) ;
			}
			if ( d_offset == -1 ) { continue ; }
		}
		t_offset = pfield->field_row * wscrmaxw + pfield->field_col + d_offset ;
		if ( t_offset > c_offset && t_offset < m_offset )
		{
			m_offset     = t_offset ;
			row          = pfield->field_row + win_row ;
			col          = pfield->field_col + d_offset + win_col ;
			cursor_moved = true ;
		}
	}

	if ( !cursor_moved ) { get_home1( row, col ) ; }
}


void pPanel::field_tab_prev( uint& row,
			     uint& col )
{
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	// Return physical position of the cursor in row/col.
	// (get_home returns physical position on the screen).
	//

	TRACE_FUNCTION() ;

	uint t_offset  ;
	uint mf_offset ;
	uint mb_offset ;
	uint c_offset  ;

	int  d_offset ;

	uint o_row ;
	uint o_col ;
	uint trow ;
	uint tcol ;

	uint row1 = 0 ;
	uint row2 = 0 ;
	uint col1 = 0 ;
	uint col2 = 0 ;

	bool forward  = false ;
	bool backward = false ;

	field* pfield ;

	trow = row - win_row ;
	tcol = col - win_col ;

	map<string, field*>::iterator it;

	c_offset  = trow * wscrmaxw + tcol ;
	mf_offset = 0 ;
	mb_offset = 0 ;
	o_row     = trow ;
	o_col     = tcol ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; ++it )
	{
		pfield = it->second ;
		if ( !pfield->field_active) { continue ; }
		if ( !pfield->field_is_dynamic() && pfield->field_is_prot( tabpas ) ) { continue ; }
		if ( ( win == pwin && pfield->field_row >= ( win_depth - cvd_lines ) ) ||
		     ( pfield->field_row >= zscrmaxd - cvd_lines ) ||
		     ( cursor_on_msg_window( pfield->field_row + win_row, pfield->field_col + win_col  ) ) )
		{
			continue ;
		}
		d_offset = 0 ;
		if ( pfield->field_is_dynamic() )
		{
			if ( pfield->field_da_ext->field_ext1_dynArea->dynArea_inAttrs == "" )
			{
				continue ;
			}
			if ( o_row == pfield->field_row )
			{
				d_offset = pfield->field_dyna_input_offset_prev( o_col ) ;
			}
			else
			{
				d_offset = pfield->field_dyna_input_offset_prev( pfield->field_endcol ) ;
			}
			if ( d_offset == -1 ) { continue ; }
		}
		t_offset = pfield->field_row * wscrmaxw + pfield->field_col + d_offset ;
		if ( t_offset > c_offset && t_offset > mf_offset )
		{
			mf_offset = t_offset ;
			row1      = pfield->field_row ;
			col1      = pfield->field_col + d_offset ;
			forward   = true ;
		}
		if ( t_offset < c_offset && t_offset > mb_offset )
		{
			mb_offset = t_offset ;
			row2      = pfield->field_row ;
			col2      = pfield->field_col + d_offset ;
			backward  = true ;
		}
	}

	if ( backward )
	{
		row = row2 + win_row ;
		col = col2 + win_col ;
	}
	else if ( forward )
	{
		row = row1 + win_row ;
		col = col1 + win_col ;
	}
	else
	{
		get_home1( row, col ) ;
	}
}


help pPanel::get_field_help( uint row,
			     uint col )
{
	//
	// Retrieve the help object associated with the field name the cursor is on.
	//
	// Field name is determined as follows:
	//   If on the action bar - create name of the form ZABCxx
	//   If on a pull-down    - create name of the form ZPDCxxyy
	//   If on a field or RP text, use the assigned name.
	//
	// row/col is the physical position on the screen.
	//

	TRACE_FUNCTION() ;

	int i ;

	string fld ;
	string rp ;

	abc* pabc ;

	row -= win_row ;
	col -= win_col ;

	vector<abc*>::iterator it ;

	if ( pd_active() && cursor_on_pulldown( row, col ) )
	{
		pabc = ab[ abIndex ] ;
		fld  = "ZPDC" + d2ds( abIndex + 1, 2 ) + d2ds( pabc->retrieve_choice_number() + 1, 2 ) ;
	}
	else if ( cursor_on_ab( row ) )
	{
		col -= win_col ;
		for ( i = 1, it = ab.begin() ; it != ab.end() && fld == "" ; ++it, ++i )
		{
			if ( (*it)->cursor_on_abc( col ) )
			{
				fld = "ZABC" + d2ds( i, 2 ) ;
			}
		}
	}

	if ( fld == "" )
	{
		fld = field_getname_input( row, col ) ;
	}

	if ( fld == "" )
	{
		for ( auto it = text_rp.begin() ; it != text_rp.end() && fld == "" ; ++it )
		{
			if ( (*it)->text_visible && (*it)->cursor_on_text( row, col ) )
			{
				fld = (*it)->text_name ;
				rp  = fld ;
			}
		}
	}

	auto ith = field_help.find( fld ) ;

	return ( ith == field_help.end() ) ? help( rp ) : *ith->second ;
}


void pPanel::tb_set_linesChanged( errblock& err )
{
	//
	// Store changed lines for processing by the application if requested via tbdispl with no panel name.
	// Format is a list of line-number/URID pairs.
	//

	TRACE_FUNCTION() ;

	int idr ;

	string URID ;

	field* pfield ;

	map<string, field*>::iterator it ;

	tb_add_autosel_line( err ) ;
	if ( err.error() ) { return ; }

	for ( it = fieldList.begin() ; it != fieldList.end() ; ++it )
	{
		pfield = it->second ;
		if ( pfield->field_is_tbdispl() && pfield->field_changed )
		{
			pfield->field_changed = false ;
			idr = ( pfield->field_row - tb_toprow ) / tb_modlines ;
			if ( tb_linesChanged.count( idr ) == 0 )
			{
				URID = funcPool->get3( err, ".ZURID."+ d2ds( idr ) ) ;
				tb_linesChanged[ idr ] = URID ;
			}
		}
	}

	funcPool->put2( err, "ZTDSELS", tb_linesChanged.size() ) ;
}


void pPanel::tb_add_autosel_line( errblock& err )
{
	//
	// Add auto-selected line to list of changed lines.
	//

	TRACE_FUNCTION() ;

	int idr ;

	string URID ;

	err.setRC( 0 ) ;

	if ( tb_autosel && tb_csrrow > 0 )
	{
		idr = tb_csrrow - ztdtop ;
		if ( idr >= 0 && idr < tb_rdepth && tb_linesChanged.count( idr ) == 0 )
		{
			URID = funcPool->get3( err, ".ZURID."+ d2ds( idr ) ) ;
			if ( URID != "" )
			{
				tb_linesChanged[ idr ] = URID ;
				funcPool->put2( err, "ZTDSELS", tb_linesChanged.size() ) ;
			}
		}
	}
}


bool pPanel::tb_get_lineChanged( errblock& err,
				 int& ln,
				 string& URID )
{
	//
	// Retrieve the next changed line on the tbdispl.  Return screen line number and URID of the table record.
	// Don't remove the pair from the list but update ZTDSELS.
	//

	TRACE_FUNCTION() ;

	map<int, string>::iterator it ;

	ln   = 0  ;
	URID = "" ;

	err.setRC( 0 ) ;

	if ( tb_linesChanged.size() == 0 ) { return false ; }

	it   = tb_linesChanged.begin() ;
	ln   = it->first  ;
	URID = it->second ;

	funcPool->put2( err, "ZTDSELS", tb_linesChanged.size() ) ;
	if ( err.error() ) { return false ; }

	return true ;
}


bool pPanel::tb_linesPending()
{
	TRACE_FUNCTION() ;

	return ( tb_linesChanged.size() > 0 ) ;
}


void pPanel::tb_clear_linesChanged( errblock& err )
{
	//
	// Clear all stored changed lines on a tbdispl with panel name and set ZTDSELS to zero.
	//

	TRACE_FUNCTION() ;

	tb_linesChanged.clear() ;
	funcPool->put2( err, "ZTDSELS", 0 ) ;
}


void pPanel::tb_remove_lineChanged()
{
	//
	// Remove the processed line from the list of changed lines.
	//

	TRACE_FUNCTION() ;

	if ( !tb_linesChanged.empty() )
	{
		tb_linesChanged.erase( tb_linesChanged.begin() ) ;
	}
}


void pPanel::display_ztdmark( errblock& err )
{
	TRACE_FUNCTION() ;

	int size = ztdvrows * tb_modlines ;

	string mark ;

	wattrset( win, cuaAttr[ SI ] | panel_intens ) ;
	if ( size < tb_pdepth )
	{
		mark = funcPool->get2( err, 8, "ZTDMARK" ) ;
		if ( err.error() ) { return ; }
		if ( err.RC8() )
		{
			mark = p_poolMGR->get( err, "ZTDMARK" ) ;
			if ( err.error() ) { return ; }
			if ( err.RC8() )
			{
				mark = centre( " Bottom of Data ", wscrmaxw-1, '*' ) ;
			}
		}
		if ( mark.size() > wscrmaxw-1 )
		{
			mark.resize( wscrmaxw-1 ) ;
		}
		wattrset( win, cuaAttr[ CH ] | panel_intens ) ;
		mvwaddstr( win, tb_toprow + size, 1, mark.c_str() ) ;
	}

	if ( ztdtop <= ztdrows )
	{
		putDialogueVar( err, "ZZTDTOP", d2ds( ztdtop ) ) ;
		putDialogueVar( err, "ZZTDROWS", d2ds( ztdrows ) ) ;
		putDialogueVar( err, "ZZTDBROW", d2ds( ztdtop + ztdvrows - 1 ) ) ;
	}
}


void pPanel::get_ztdvars( int& zztdtop,
			  int& zztdrows,
			  int& zztdvrows )
{
	TRACE_FUNCTION() ;

	zztdtop   = ztdtop ;
	zztdrows  = ztdrows ;
	zztdvrows = ztdvrows ;
}


void pPanel::set_ztdvars( int zztdtop,
			  int zztdrows,
			  int zztdvrows )
{
	TRACE_FUNCTION() ;

	ztdtop   = zztdtop ;
	ztdrows  = zztdrows ;
	ztdvrows = zztdvrows ;
}


void pPanel::display_row_indicator( errblock& err )
{
	TRACE_FUNCTION() ;

	if ( ztdtop <= ztdrows )
	{
		wattrset( win, WHITE | panel_intens ) ;
		mvwaddstr( win, ( ab.size() > 0 ) ? 2 : 0, wscrmaxw - row_text.size(), row_text.c_str() ) ;
	}
}


void pPanel::display_row_indicator( errblock& err,
				    const string& text )
{
	TRACE_FUNCTION() ;

	row_text = text ;

	if ( ztdtop <= ztdrows )
	{
		wattrset( win, WHITE | panel_intens ) ;
		mvwaddstr( win, ( ab.size() > 0 ) ? 2 : 0, wscrmaxw - text.size(), text.c_str() ) ;
	}
}


void pPanel::set_tb_fields_act_inact( errblock& err )
{
	TRACE_FUNCTION() ;

	string suf ;

	set<string>::iterator it ;

	for ( int i = 0 ; i < tb_rdepth ; ++i )
	{
		suf = "." + d2ds( i ) ;
		for ( it = tb_fields.begin() ; it != tb_fields.end() ; ++it )
		{
			fieldList[ *it + suf ]->field_active = ( i < ztdvrows ) ;
		}
	}
}


void pPanel::scroll_all_areas_to_top()
{
	//
	// Scroll all areas back to the top before )INIT processing and rebuild fieldMap
	// if any have been scrolled.
	//

	TRACE_FUNCTION() ;

	bool any_scrolled = false ;

	for ( auto it = AreaList.begin() ; it != AreaList.end() ; ++it )
	{
		if ( it->second->scroll_to_top() )
		{
			it->second->update_area() ;
			any_scrolled = true ;
		}
	}

	if ( any_scrolled )
	{
		build_fieldMap() ;
	}
}


void pPanel::scroll_all_fields_to_start( errblock& err )
{
	//
	// Scroll all scrollable fields to the start before )INIT processing if no LCOL defined.
	// Also update RCOL value and SCROLL indicators (ie. IND(), LIND(), RIND(), SIND() and SCALE())
	//

	TRACE_FUNCTION() ;

	field* pfield ;

	for ( auto& sf : sfields )
	{
		pfield = sf.first ;
		if ( !pfield->field_has_lcol_var() )
		{
			pfield->field_scroll_to_start() ;
			update_scroll_inds( err, pfield, false ) ;
			if ( err.error() ) { return ; }
			update_scroll_rcol( err, pfield, false ) ;
			if ( err.error() ) { return ; }
		}
	}
}


void pPanel::update_scroll_inds( errblock& err,
				 field* pfield,
				 bool update_field )
{
	//
	// Update IND(), LIND(), RIND(), SIND() and SCALE() of scrollable fields.
	//

	TRACE_FUNCTION() ;

	int l ;

	if ( pfield->field_has_ind() )
	{
		putDialogueVar( err, pfield->field_get_ind1(), pfield->field_get_ind2() ) ;
		if ( err.error() ) { return ; }
		if ( update_field )
		{
			put_field_value( pfield->field_get_ind1(), pfield->field_get_ind2() ) ;
		}
	}

	if ( pfield->field_has_lind() )
	{
		putDialogueVar( err, pfield->field_get_lind1(), pfield->field_get_lind2() ) ;
		if ( err.error() ) { return ; }
		if ( update_field )
		{
			put_field_value( pfield->field_get_lind1(), pfield->field_get_lind2() ) ;
		}
	}

	if ( pfield->field_has_rind() )
	{
		putDialogueVar( err, pfield->field_get_rind1(), pfield->field_get_rind2() ) ;
		if ( err.error() ) { return ; }
		if ( update_field )
		{
			put_field_value( pfield->field_get_rind1(), pfield->field_get_rind2() ) ;
		}
	}

	if ( pfield->field_has_sind() )
	{
		l = get_field_length( pfield->field_get_sind1() ) ;
		putDialogueVar( err, pfield->field_get_sind1(), pfield->field_get_sind2( l ) ) ;
		if ( err.error() ) { return ; }
		if ( update_field )
		{
			put_field_value( pfield->field_get_sind1(), pfield->field_get_sind2( l ) ) ;
		}
		put_field_value( pfield->field_get_sind1(), pfield->field_get_sind2( l ) ) ;
	}

	if ( pfield->field_has_scale() )
	{
		l = get_field_length( pfield->field_get_scale1() ) ;
		putDialogueVar( err, pfield->field_get_scale1(), pfield->field_get_scale2( l ) ) ;
		if ( err.error() ) { return ; }
		if ( update_field )
		{
			put_field_value( pfield->field_get_scale1(), pfield->field_get_scale2( l ) ) ;
		}
	}
}


void pPanel::update_scroll_lcol( errblock& err,
				 field* pfield )
{
	//
	// Update LCOL() for a scrollable field.
	// If more than one field has the same LCOL variable, use the maximum.
	//

	TRACE_FUNCTION() ;

	uint maxlcol ;

	if ( pfield->field_has_lcol_var() )
	{
		maxlcol = max_scroll_lcol( pfield ) ;
		putDialogueVar( err, pfield->field_get_lcol_var(), d2ds( maxlcol ) ) ;
		if ( err.error() ) { return ; }
		put_field_value( pfield->field_get_lcol_var(), maxlcol ) ;
	}
}


void pPanel::update_scroll_rcol( errblock& err,
				 field* pfield,
				 bool update_field )
{
	//
	// Update RCOL() for a scrollable field.
	//

	TRACE_FUNCTION() ;

	if ( pfield->field_has_rcol_var() )
	{
		putDialogueVar( err, pfield->field_get_rcol_var(), d2ds( pfield->field_get_rcol() ) ) ;
		if ( err.error() ) { return ; }
		if ( update_field )
		{
			put_field_value( pfield->field_get_rcol_var(), pfield->field_get_rcol() ) ;
		}
	}
}


uint pPanel::max_scroll_lcol( field* pfield )
{
	//
	// Return max value of LCOL() for a number of scrollable fields with a common LCOL variable name.
	//

	TRACE_FUNCTION() ;

	uint i = 0 ;

	auto ret = lcolmmap.equal_range( pfield->field_get_lcol_var() ) ;

	for ( auto it = ret.first ; it != ret.second ; ++it )
	{
		i = max( i, it->second->field_get_lcol() ) ;
	}

	return i ;
}


void pPanel::update_lenvar( errblock& err,
			    field* pfield )
{
	//
	// Update LEN value.
	// Set to 0 if the variable contains an invalid value.  This will set LEN to the minimum.
	//

	TRACE_FUNCTION() ;

	string val = getDialogueVar( err, pfield->field_get_lenvar() ) ;
	if ( err.error() ) { return ; }

	uint l = 0 ;
	uint m ;

	if ( isnumeric( val ) )
	{
		m = ds2d ( val ) ;
		if ( m >= 1 && m <= 32767 )
		{
			l = m ;
		}
	}

	pfield->field_set_len( l ) ;
}


void pPanel::rebuild_after_area_scroll( Area* a )
{
	//
	// Update the fieldList, textList, fieldMap and fieldAddrs after
	// an AREA scroll.
	//

	TRACE_FUNCTION() ;

	a->update_area() ;

	build_fieldMap() ;
}


void pPanel::get_pd_home( uint& row,
			  uint& col )
{
	//
	// Return the physical home position of a pull-down.
	//

	TRACE_FUNCTION() ;

	abc* pabc = ab[ abIndex ] ;

	pabc->get_pd_home( row, col ) ;
}


bool pPanel::cursor_on_pulldown( uint row,
				 uint col )
{
	//
	// row/col is the pysical position on the screen.
	//

	TRACE_FUNCTION() ;

	return ab[ abIndex ]->cursor_on_pulldown( row, col ) ;
}


bool pPanel::display_pd( errblock& err,
			 uint row,
			 uint col,
			 string& msg,
			 uint home )
{
	//
	// row/col is the pysical position on the screen.  Correct by subtracting the window column position for col.
	//
	// Call relevant )ABCINIT and display the pull-down at the cursor position.
	//

	TRACE_FUNCTION() ;

	uint i ;

	vector<abc*>::iterator it ;

	abc* pabc ;

	col -= win_col ;
	hide_pd( err ) ;

	err.setRC( 0 ) ;

	for ( i = 0, it = ab.begin() ; it != ab.end() ; ++it, ++i )
	{
		pabc = *it ;
		if ( pabc->cursor_on_abc( col ) )
		{
			abIndex = i ;
			clear_msg() ;
			abc_panel_init( err,
					pabc->get_abc_desc() ) ;
			if ( err.error() )
			{
				pdActive = false ;
				return false ;
			}
			msg = dMsg ;
			pabc->display_pd( err,
					  zscrmaxd,
					  zscrmaxw,
					  dZvars,
					  win_row,
					  win_col,
					  row,
					  home ) ;
			if ( err.error() )
			{
				pdActive = false ;
				return false ;
			}
			pabc->display_abc_sel( win ) ;
			pabc->get_pd_home( l_row, l_col ) ;
			l_row   -= win_row ;
			l_col   -= win_col ;
			msg      = dMsg ;
			pdActive = true ;
			return true ;
		}
	}

	return false ;
}


void pPanel::display_pd( errblock& err )
{
	TRACE_FUNCTION() ;

	if ( !pdActive ) { return ; }

	abc* pabc = ab[ abIndex ] ;

	pabc->display_abc_sel( win ) ;
	pabc->display_pd( err,
			  zscrmaxd,
			  zscrmaxw,
			  dZvars,
			  win_row,
			  win_col,
			  0 ) ;
}


void pPanel::display_current_pd( errblock& err,
				 uint row,
				 uint col )
{
	//
	// row/col is the pysical position on the screen.
	//
	// Re-display the pull-down if the cursor is placed on it (to update current choice and hilite).
	//

	TRACE_FUNCTION() ;

	abc* pabc = ab[ abIndex ] ;

	if ( col >= pabc->abc_col1 && col < ( pabc->abc_col1 + pabc->abc_maxw + 10 ) )
	{
		pabc->display_pd( err,
				  zscrmaxd,
				  zscrmaxw,
				  dZvars,
				  win_row,
				  win_col,
				  row,
				  0 ) ;
	}
}


void pPanel::display_next_pd( errblock& err,
			      const string& mnemonic,
			      string& posn,
			      string& msg )
{
	TRACE_FUNCTION() ;

	uint i ;
	uint p1 ;
	uint p2 = 0 ;
	uint p3 = 0 ;

	uint p_home = 1 ;

	vector<abc*>::iterator it ;

	err.setRC( 0 ) ;

	if ( ab.size() == 0 ) { return ; }

	hide_pd( err ) ;

	if ( !pdActive && mnemonic.size() != 1 )
	{
		abIndex  = 0 ;
		pdActive = true ;
	}
	else if ( !pdActive || mnemonic.size() == 1 )
	{
		if ( !pdActive )
		{
			abIndex = 0 ;
		}
		for ( i = 0, it = ab.begin() ; it != ab.end() ; ++it, ++i )
		{
			if ( (*it)->abc_mnem2 == mnemonic.front() )
			{
				abIndex = i ;
				break ;
			}
		}
		pdActive = true ;
	}
	else if ( ++abIndex == ab.size() )
	{
		abIndex = 0 ;
	}

	abc* pabc = ab[ abIndex ] ;

	clear_msg() ;

	abc_panel_init( err,
			pabc->get_abc_desc() ) ;
	if ( err.error() )
	{
		pdActive = false ;
		return ;
	}

	if ( posn != "" && posn.size() < 4 && datatype( posn, 'W' ) )
	{
		p1 = ds2d( posn ) ;
		if ( p1 > 0 && p1 <= pabc->get_pd_rows() )
		{
			p2     = p1 - 1 ;
			p3     = p1 + win_row + 1,
			p_home = p1 ;
		}
	}

	msg = dMsg ;
	pabc->display_pd( err,
			  zscrmaxd,
			  zscrmaxw,
			  dZvars,
			  win_row,
			  win_col,
			  p3,
			  p_home ) ;
	if ( err.error() )
	{
		pdActive = false ;
		return ;
	}

	pabc->display_abc_sel( win ) ;
	pabc->get_pd_home( l_row, l_col ) ;
	l_row -= win_row - p2 ;
	l_col -= win_col ;
}


void pPanel::display_prev_pd( errblock& err,
			      const string& mnemonic,
			      string& posn,
			      string& msg )
{
	TRACE_FUNCTION() ;

	uint p1 ;
	uint p2 = 0 ;
	uint p3 = 0 ;

	uint p_home = 1 ;

	err.setRC( 0 ) ;

	hide_pd( err ) ;

	if ( abIndex == 0 )
	{
		abIndex = ab.size() - 1 ;
	}
	else
	{
		--abIndex ;
	}

	abc* pabc = ab[ abIndex ] ;

	clear_msg() ;

	abc_panel_init( err,
			pabc->get_abc_desc() ) ;
	if ( err.error() )
	{
		pdActive = false ;
		return ;
	}

	if ( posn != "" && posn.size() < 4 && datatype( posn, 'W' ) )
	{
		p1 = ds2d( posn ) ;
		if ( p1 > 0 && p1 <= pabc->get_pd_rows() )
		{
			p2     = p1 - 1 ;
			p3     = p1 + win_row + 1,
			p_home = p1 ;
		}
	}

	msg = dMsg ;
	pabc->display_pd( err,
			  zscrmaxd,
			  zscrmaxw,
			  dZvars,
			  win_row,
			  win_col,
			  p3,
			  p_home ) ;
	if ( err.error() )
	{
		pdActive = false ;
		return ;
	}

	pabc->display_abc_sel( win ) ;
	pabc->get_pd_home( l_row, l_col ) ;
	l_row -= win_row - p2 ;
	l_col -= win_col ;
}


void pPanel::remove_pd( errblock& err )
{
	TRACE_FUNCTION() ;

	if ( pdActive )
	{
		hide_pd( err ) ;
		pdActive = false ;
		set_lcursor_home() ;
	}
}


void pPanel::hide_pd( errblock& err )
{
	TRACE_FUNCTION() ;

	bool hi_mnemonic ;

	if ( !pdActive ) { return ; }

	abc* pabc = ab[ abIndex ] ;

	hi_mnemonic = ( p_poolMGR->get( err, "ZHIABMN", PROFILE ) == "Y" ) ;

	pabc->hide_pd() ;
	pabc->display_abc_unsel( win, hi_mnemonic ) ;
}


void pPanel::display_area_si()
{
	//
	// Display the area scroll indicator.
	//

	TRACE_FUNCTION() ;

	uint row ;
	uint col ;
	uint width ;

	Area* parea ;

	for ( auto it = AreaList.begin() ; it != AreaList.end() ; ++it )
	{
		parea = it->second ;
		parea->set_visible_depth( get_areared_lines( parea ) ) ;
		parea->get_info( row, col, width ) ;
		wattrset( win, cuaAttr[ SI ] | panel_intens ) ;
		mvwaddstr( win, row, ( col + width - 12 ), parea->get_scroll_indicator() ) ;
	}
}


bool pPanel::jump_field( uint row,
			 uint col,
			 string& fvalue )
{
	//
	// Return true if cursor is on a jump field and a jump command has been entered.
	// Also return field value up to the cursor position or first space (remove nulls and leading spaces).
	// Replace field value with function pool value.
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field.
	//

	TRACE_FUNCTION() ;

	size_t p1 ;
	size_t p2 ;

	errblock err ;

	field* pfield ;

	const char nulls = 0x00 ;

	if ( jumpList.size() == 0 ) { return false ; }

	row -= win_row ;
	col -= win_col ;

	for ( auto it = jumpList.begin() ; it != jumpList.end() ; ++it )
	{
		pfield = it->second ;
		if ( pfield->cursor_on_field( row, col ) )
		{
			fvalue = pfield->field_value.substr( 0, col - pfield->field_col ) ;
			p1     = fvalue.find( nulls ) ;
			while ( p1 != string::npos )
			{
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
				p1 = fvalue.find( nulls, p1 ) ;
			}
			fvalue = word( fvalue, 1 ) ;
			if ( fvalue.size() > 1 && fvalue.front() == '=' )
			{
				pfield->field_put_value( getDialogueVar( err, it->first ) ) ;
				return true ;
			}
			return false ;
		}
	}

	return false ;
}


pdc pPanel::retrieve_choice( errblock& err,
			     string& msg )
{
	TRACE_FUNCTION() ;

	abc* pabc = ab[ abIndex ] ;

	pdc t_pdc = pabc->retrieve_choice( err ) ;

	if ( !t_pdc.pdc_inact )
	{
		abc_panel_proc( err, pabc->get_abc_desc() ) ;
		msg = dMsg ;
	}

	return t_pdc ;
}


void pPanel::display_boxes()
{
	TRACE_FUNCTION() ;

	for ( auto it = boxes.begin() ; it != boxes.end() ; ++it )
	{
		(*it)->display_box( win, sub_vars( (*it)->box_title ) ) ;
	}
}


void pPanel::set_panel_msg( const slmsg& t,
			    const string& m )
{
	TRACE_FUNCTION() ;

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
	TRACE_FUNCTION() ;

	MSG.clear() ;

	dMsg      = "" ;
	showLMSG  = false ;
	error_msg = false ;

	if ( smwin )
	{
		panel_cleanup( smpanel ) ;
		del_panel( smpanel ) ;
		delwin( smwin ) ;
		smwin = nullptr ;
	}

	if ( lmwin )
	{
		panel_cleanup( lmpanel ) ;
		del_panel( lmpanel ) ;
		delwin( lmwin ) ;
		lmwin = nullptr ;
	}
}


void pPanel::put_keylist( int entry,
			  const string* keyv )
{
	TRACE_FUNCTION() ;

	keylistl[ entry ] = *keyv ;
}


void pPanel::put_keyattr( int entry,
			  const string* attr )
{
	TRACE_FUNCTION() ;

	keyattrl[ entry ] = *attr ;
}


void pPanel::put_keylabl( int entry,
			  const string* label )
{
	TRACE_FUNCTION() ;

	keylabll[ entry ] = *label ;
}


string pPanel::get_pfkey( errblock& err,
			  int c )
{
	TRACE_FUNCTION() ;

	string key ;
	string pfcmd ;

	if ( p_poolMGR->sysget( err, "ZKLUSE", PROFILE ) == "Y" || keyshr )
	{
		pfcmd = ( keylistn == "" || keylistl.count( c ) == 0 ) ? "" : keylistl[ c ] ;
	}

	if ( pfcmd == "" )
	{
		key   = "ZPF" + d2ds( ( c - KEY_F( 0 ) ), 2 ) ;
		pfcmd = p_poolMGR->get( err, key, PROFILE ) ;
		if ( err.RC8() )
		{
			p_poolMGR->put( err, key, "", PROFILE ) ;
		}
	}

	key = "PF" + d2ds( c - KEY_F( 0 ), 2 ) ;
	p_poolMGR->put( err, "ZPFKEY", key, SHARED, SYSTEM ) ;
	set_pfpressed( key ) ;

	return pfcmd ;
}


void pPanel::add_linecmd( errblock& err,
			  uint row,
			  uint col,
			  string lcmd )
{
	//
	// Place lcmd in the first input field on passed row location.
	//
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find line.
	//
	// If the field is in a scrollable area but covered by pfkeys/swapbar, scroll the area so the field
	// is visible (may not have an effect if )INIT display processing then performed).
	//
	// RC =  0 Normal completion.
	// RC =  4 First field is a dynamic area.
	// RC =  8 Cursor not in window.
	// RC = 20 Error - message id set.
	//

	TRACE_FUNCTION() ;

	field* pfield ;
	field* lfield = nullptr ;

	uint zz_col = -1 ;

	err.setRC( 0 ) ;

	if ( cursor_not_in_window( row, col ) )
	{
		err.setRC( 8 ) ;
		return ;
	}

	row -= win_row ;

	for ( auto& f : fieldList )
	{
		pfield = f.second ;
		if ( pfield->field_row == row   &&
		     pfield->field_col < zz_col &&
		     pfield->field_active       &&
		     pfield->field_visible      &&
		   ( pfield->field_is_input() || pfield->field_is_dynamic() ) )
		{
			zz_col = pfield->field_col ;
			lfield = pfield ;
		}
	}

	if ( !lfield )
	{
		err.seterrid( TRACE_INFO(), "PSYS014F" ) ;
		return ;
	}

	if ( lfield->field_is_dynamic() )
	{
		err.setRC( 4 ) ;
		return ;
	}

	if ( cmdfield != "" && lfield == fieldList.find( cmdfield )->second )
	{
		err.seterrid( TRACE_INFO(), "PSYS014H" ) ;
		return ;
	}

	if ( trim( lfield->field_value ) != "" )
	{
		if ( lfield->field_area > 0 )
		{
			make_field_visible( err, lfield ) ;
		}
		err.seterrid( TRACE_INFO(), "PSYS014G" ) ;
		return ;
	}

	if ( lcmd.size() > lfield->field_length )
	{
		lcmd.erase( lfield->field_length ) ;
	}
	field_setvalue( lfield, lcmd ) ;

	if ( lfield->field_area > 0 )
	{
		make_field_visible( err, lfield ) ;
	}
}


void pPanel::display_lmsg( errblock& err )
{
	TRACE_FUNCTION() ;

	showLMSG = true ;
	display_msg( err ) ;
}


void pPanel::display_msg( errblock& err )
{
	TRACE_FUNCTION() ;

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

	field* pfield ;

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
			pfield  = it->second ;
			if ( MSG.smsg.size() > pfield->field_length )
			{
				inWindow1 = true ;
			}
			else
			{
				m_row     = pfield->field_row + win_row ;
				m_col     = pfield->field_col + win_col ;
				f_colour1 = pfield->field_get_colour() ;
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
				m_row = ( ab.size() > 0 ) ? 2 : 0 ;
				int l = wscrmaxw - 1 - t.size() ;
				int j = max( 0, ( l - 8 ) ) ;
				while ( l >= j && char( mvwinch( win, m_row, l ) & A_CHARTEXT ) != ' ' )
				{
					--l ;
				}
				t     = right( t, ( wscrmaxw - l - 1 ) ) ;
				m_col = win_col + wscrmaxw - t.size() - 1 ;
				smwin = newwin( 1, ( t.size() + 1 ), ( win_row + m_row ), m_col) ;
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
		smpanel = new_panel( smwin ) ;
		set_panel_userptr( smpanel, new panel_data( zscrnum, inWindow1 ) ) ;
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
			delwin( lmwin ) ;
		}
		inWindow2 = ( MSG.lmwin || p_poolMGR->get( err, "ZLMSGW", PROFILE ) == "Y" || pd_active() ) ;
		if ( not inWindow2 && pos_lmsg != "" )
		{
			auto it = fieldList.find( pos_lmsg ) ;
			pfield  = it->second ;
			if ( MSG.lmsg.size() > pfield->field_length )
			{
				inWindow2 = true ;
			}
			else
			{
				m_row     = pfield->field_row + win_row ;
				m_col     = pfield->field_col + win_col ;
				f_colour1 = pfield->field_get_colour() ;
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
					smwin = nullptr ;
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
		set_panel_userptr( lmpanel, new panel_data( zscrnum, inWindow2 ) ) ;
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
	//
	// Split message into separate lines if necessary and put into vector v.
	// Calculate the message window position and size.
	//
	// If the message window has a location field, try not to cover any part of the
	// field with the message window (move above if there is space).
	//

	TRACE_FUNCTION() ;

	uint w ;
	uint h ;
	uint t ;
	uint corr ;

	size_t mw ;

	size_t p ;

	string loc = get_msgloc() ;

	field* pfield = nullptr ;

	abc* pabc ;

	auto it = fieldList.find( loc ) ;

	w = m.size() ;

	if ( it == fieldList.end() )
	{
		if ( loc != "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE021D", loc ) ;
			return ;
		}
		if ( pd_active() )
		{
			t = zscrmaxw - ab.at( abIndex )->get_pd_col1() - 3 ;
			if ( t < 20 ) { t = 20 ; }
			w = min( w, t ) ;
		}
		else if ( win == fwin && w > ( zscrmaxw * 2 / 3 ) )
		{
			w = zscrmaxw * 2 / 3 ;
		}
	}
	else
	{
		pfield = it->second ;
		if ( ( pfield->field_col + m.size() + 2 ) > wscrmaxw )
		{
			t = wscrmaxw - pfield->field_col - 2 ;
			if ( t < 30 )
			{
				t = 3 * sqrt( m.size() ) ;
				if ( t < 30 ) { t = 30 ; }
			}
			h = ( m.size() / t ) + 1 ;
			w = m.size() / h  ;
		}
		else if ( w > ( zscrmaxw / 2 ) )
		{
			w = zscrmaxw / 2 ;
		}
	}

	if ( w > wscrmaxw - 4 ) { w = wscrmaxw - 4 ; }

	v.clear() ;
	mw = 0    ;

	while ( m != "" )
	{
		if ( m.size() <= w )
		{
			mw = max( mw, m.size() ) ;
			v.push_back( m ) ;
			break ;
		}
		p = m.find_last_of( ' ', w ) ;
		if ( p == string::npos ) { p = w ; }
		v.push_back( strip( m.substr( 0, p ), 'T', ' ' ) ) ;
		mw = max( mw, v.back().size() ) ;
		m.erase( 0, p ) ;
		trim_left( m )  ;
	}

	t_depth = v.size() + 2 ;
	t_width = mw + 4 ;

	if ( pd_active() )
	{
		pabc = ab[ abIndex ] ;
		pabc->get_msg_position( t_row, t_col, win_col ) ;
		++t_row ;
		if ( ( t_row + t_depth + win_row ) > zscrmaxd )
		{
			t_row = pabc->get_pd_row1() - t_depth - win_row ;
			if ( ( t_row + win_row ) > zscrmaxd )
			{
				t_row = zscrmaxd - t_depth - win_row ;
			}
		}
		if ( ( t_col + t_width + win_col ) > zscrmaxw )
		{
			t_col = zscrmaxw - win_col - t_width ;
		}
	}
	else if ( pfield )
	{
		t_row = pfield->field_row + 1 ;
		t_col = pfield->field_col > 2 ? pfield->field_col - 2 : 0 ;
		if ( t_row + t_depth + win_row > zscrmaxd - 1 )
		{
			t_row = zscrmaxd - t_depth - win_row - 1 ;
			t_col = t_col + pfield->field_length + 2 ;
			if ( t_row > zscrmaxd ) { ++t_row ; }
		}
		if ( ( t_col + t_width + win_col ) > zscrmaxw )
		{
			t_col = zscrmaxw - t_width - win_col ;
		}
		if ( ( pfield->field_row >= t_row )           &&
		     ( pfield->field_row <= t_row + t_depth ) &&
		     ( pfield->field_row + win_row >= t_depth ) )
		{
			for ( uint i = 0 ; i < pfield->field_length ; ++i )
			{
				if ( pfield->field_col + i > t_col &&
				     pfield->field_col + i < t_col + t_width )
				{
					t_row = pfield->field_row - t_depth ;
					break ;
				}
			}
		}
	}
	else if ( win == pwin )
	{
		corr  = ( p_poolMGR->sysget( err, "ZSWPBR",  PROFILE ) == "Y" ) ? 1 : 0 ;
		t_row = win_depth + 1 ;
		t_col = -1 ;
		if ( ( t_row + t_depth + win_row + corr ) > zscrmaxd )
		{
			t_row = zscrmaxd - t_depth - win_row - corr ;
		}
	}
	else
	{
		t_row = wscrmaxd - t_depth - 1 ;
		t_col = ( wscrmaxw - t_width - 4 ) / 2 ;
		if ( t_col > wscrmaxw ) { t_col = 0 ; }
	}
}


void pPanel::display_id( errblock& err )
{
	TRACE_FUNCTION() ;

	uint row ;
	uint maxx ;
	uint maxy ;
	uint w_row ;
	uint w_col ;

	string scrname ;
	string panarea = "" ;

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
			panarea += upper( scrname ) + " " ;
		}
	}

	if ( panarea != "" )
	{
		row = ( ab.size() > 0 ) ? win_row + 2 : win_row ;
		if ( idwin )
		{
			getmaxyx( idwin, maxy, maxx ) ;
			if ( maxx != panarea.size() || maxy != 1 )
			{
				panel_cleanup( idpanel ) ;
				del_panel( idpanel ) ;
				delwin( idwin ) ;
				idwin = nullptr ;
			}
			else
			{
				getbegyx( idwin, w_row, w_col ) ;
				if ( w_row != row || w_col != win_col+1 )
				{
					mvwin( idwin, row, win_col+1 ) ;
				}
			}
		}
		if ( !idwin )
		{
			idwin   = newwin( 1, panarea.size(), row, win_col+1 ) ;
			idpanel = new_panel( idwin )  ;
			set_panel_userptr( idpanel, new panel_data( zscrnum ) ) ;
		}
		wattrset( idwin, cuaAttr[ PI ] | panel_intens ) ;
		mvwaddstr( idwin, 0, 0, panarea.c_str() ) ;
		top_panel( idpanel ) ;
		update_panels() ;
	}
	else if ( idwin )
	{
		hide_panel( idpanel ) ;
	}
}


void pPanel::display_pfkeys( errblock& err )
{
	TRACE_FUNCTION() ;

	uint row ;
	uint maxx ;
	uint maxy ;
	uint w_row ;
	uint w_col ;

	bool swapbar = ( p_poolMGR->sysget( err, "ZSWPBR",  PROFILE ) == "Y" && ( win == fwin ) && !( pwin && win_depth < zscrmaxd ) ) ;

	row = ( swapbar ) ? ( wscrmaxd + win_row - klist.size() - 1 ) :
			    ( wscrmaxd + win_row - klist.size() ) ;

	if ( pfkwin )
	{
		hide_panel( pfkpanel ) ;
	}

	if ( klist.size() > 0 )
	{
		if ( pfkwin )
		{
			getmaxyx( pfkwin, maxy, maxx ) ;
			if ( maxx != wscrmaxw || maxy != klist.size() )
			{
				panel_cleanup( pfkpanel ) ;
				del_panel( pfkpanel ) ;
				delwin( pfkwin ) ;
				pfkwin = nullptr ;
			}
			else
			{
				getbegyx( pfkwin, w_row, w_col ) ;
				if ( w_row != row || w_col != win_col )
				{
					mvwin( pfkwin, row, win_col ) ;
				}
			}
		}
		if ( !pfkwin )
		{
			pfkwin   = newwin( klist.size(), wscrmaxw, row, win_col ) ;
			pfkpanel = new_panel( pfkwin ) ;
			set_panel_userptr( pfkpanel, new panel_data( zscrnum ) ) ;
		}
		wattrset( pfkwin, cuaAttr[ FK ] | panel_intens ) ;
		for ( uint i = 0 ; i < klist.size() ; ++i )
		{
			mvwaddstr( pfkwin, i, 1, klist[ i ].c_str() ) ;
		}
		top_panel( pfkpanel ) ;
		if ( smwin && !panel_hidden( smpanel) )
		{
			top_panel( smpanel ) ;
		}
		if ( lmwin && !panel_hidden( lmpanel) )
		{
			top_panel( lmpanel ) ;
		}
		update_panels() ;
	}

	display_area_si() ;
}


void pPanel::build_pfkeys( errblock& err,
			   bool force_build )
{
	//
	// cvd_lines: lines covering the bottom of the display.
	//            = number of pfkey lines for popup windows.
	//            = number of pfkey lines + swapbar line for fullscreen windows.
	//

	TRACE_FUNCTION() ;

	int diff ;

	uint row ;
	uint col ;

	bool showpfk = ( p_poolMGR->sysget( err, "ZPFSHOW", PROFILE ) == "ON" ) ;
	uint maxlnes = ( p_poolMGR->sysget( err, "ZPFSET", PROFILE )  == "ALL" ) ? 4 : 2 ;

	if ( zshowpfk != showpfk )
	{
		force_build = true ;
		zshowpfk    = showpfk ;
	}

	if ( pfklToken == pfkgToken && !force_build ) { return ; }

	if ( force_build && !pfk_built && !win )
	{
		if ( win_addpop && pwin && !full_screen )
		{
			win = pwin ;
			wscrmaxw = win_width ;
			wscrmaxd = win_depth ;
		}
		else
		{
			win = fwin ;
		}
	}

	bool swapbar  = ( p_poolMGR->sysget( err, "ZSWPBR",  PROFILE ) == "Y" && ( win == fwin ) && !( pwin && win_depth < zscrmaxd ) ) ;
	bool klinuse  = ( p_poolMGR->sysget( err, "ZKLUSE",  PROFILE ) == "Y" || keyshr ) ;
	bool fkashort = ( p_poolMGR->sysget( err, "ZFKA",    PROFILE ) == "SHORT" ) ;

	uint kfirst = 1 ;
	uint klast  = 24 ;

	string t1 ;
	string t2 ;
	string t3 ;
	string t4 ;
	string zpfset ;
	string zprikeys ;

	klist.clear() ;

	if ( showpfk )
	{
		zpfset   = p_poolMGR->get( err, "ZPFSET", PROFILE ) ;
		zprikeys = p_poolMGR->get( err, "ZPRIKEYS", PROFILE ) ;
		if ( ( zpfset == "PRI" && zprikeys == "LOW" ) ||
		     ( zpfset == "ALT" && zprikeys == "UPP" ) )
		{
			klast  = 12 ;
		}
		else if ( ( zpfset == "PRI" && zprikeys == "UPP" ) ||
			  ( zpfset == "ALT" && zprikeys == "LOW" ) )
		{
			kfirst = 13 ;
		}
		t4 = "" ;
		for ( uint i = kfirst ; i <= klast && klist.size() < maxlnes ; ++i )
		{
			if ( klinuse && keylistl[ KEY_F( i ) ] != "" )
			{
				if ( keyattrl[ KEY_F( i ) ] == "NO" )
				{
					continue ;
				}
				else if ( keyattrl[ KEY_F( i ) ] == "SHORT" )
				{
					t3 = right( "F" + d2ds( i ) + "=", 4 ) + left( keylabll[ KEY_F( i ) ], 8 ) ;
				}
				else if ( fkashort )
				{
					continue ;
				}
				else
				{
					t3 = right( "F" + d2ds( i ) + "=", 4 ) + left( keylistl[ KEY_F( i ) ], 8 ) ;
				}
			}
			else
			{
				t2 = p_poolMGR->get( err, "ZPFL" + d2ds( i, 2 ), PROFILE ) ;
				if ( upper( t2 ) == "NOSHOW" ) { continue ; }
				t1 = p_poolMGR->get( err, "ZPF" + d2ds( i, 2 ), PROFILE ) ;
				t3  = right( "F" + d2ds( i ) + "=", 4 ) + left( ( ( t2 == "" ) ? t1 : t2 ), 8 ) ;
			}
			if ( ( t4.size() + t3.size() ) > ( wscrmaxw - 2 ) )
			{
				klist.push_back( t4 ) ;
				t4 = "" ;
			}
			t4 += left( t3, 13 ) ;
		}
		if ( t4 != "" && klist.size() < maxlnes )
		{
			klist.push_back( t4 ) ;
		}
	}

	if ( showpfk && field_get_row_col( cmdfield, row, col ) )
	{
		diff = row - win_row - ( wscrmaxd - klist.size() - ( ( swapbar ) ? 2 : 1 ) ) ;
		while ( diff > 0 && klist.size() > 0 )
		{
			klist.pop_back() ;
			--diff ;
		}
	}

	cvd_lines = klist.size() ;

	if ( swapbar )
	{
		++cvd_lines ;
	}

	pfk_built = true ;
	pfklToken = pfkgToken ;
}


uint pPanel::get_tbred_lines()
{
	//
	// Return the number of lines the tb display has been reduced by because of
	// SWAPBAR and PFSHOW commands.
	//

	TRACE_FUNCTION() ;

	return max( 0, int( cvd_lines + tb_pdepth + tb_toprow - wscrmaxd ) ) ;
}


uint pPanel::get_areared_lines()
{
	//
	// Return the number of lines the scrollable dynamic area display has been reduced by because of
	// SWAPBAR and PFSHOW commands.
	//

	TRACE_FUNCTION() ;

	return max( 0, int( cvd_lines + dyns_depth + dyns_toprow - wscrmaxd ) ) ;
}


uint pPanel::get_areared_lines( dynArea* a )
{
	//
	// Return the number of lines the dynamic area display has been reduced by because of
	// SWAPBAR and PFSHOW commands.
	//

	TRACE_FUNCTION() ;

	return max( 0, int( cvd_lines + a->dynArea_depth + a->dynArea_row - wscrmaxd ) ) ;
}


uint pPanel::get_areared_lines( Area* a )
{
	//
	// Return the number of lines the scroll area display has been reduced by because of
	// SWAPBAR and PFSHOW commands.
	//

	TRACE_FUNCTION() ;

	uint row ;
	uint depth ;

	a->get_info( row, depth ) ;

	return max( 0, int( cvd_lines + depth + 1 + row - wscrmaxd ) ) ;
}


uint pPanel::get_display_field_length( const string& field )
{
	//
	// Return display field length for a scrollable field.
	//

	TRACE_FUNCTION() ;

	auto it = fieldList.find( field ) ;

	return it->second->field_get_display_len() ;
}


uint pPanel::get_field_length( const string& field )
{
	//
	// Return field length for field.
	//

	TRACE_FUNCTION() ;

	auto it = fieldList.find( field ) ;

	return it->second->field_length ;
}


void pPanel::put_field_value( const string& field,
			      const string& value )
{
	//
	// Update a field value (string).
	//

	TRACE_FUNCTION() ;

	auto it = fieldList.find( field ) ;

	it->second->field_put_value( value ) ;
}


void pPanel::put_field_value( const string& field,
			      uint value )
{
	//
	// Update a field value (uint).
	//

	TRACE_FUNCTION() ;

	auto it = fieldList.find( field ) ;

	it->second->field_put_value( d2ds( value ) ) ;
}


void pPanel::get_panel_info( errblock& err,
			     const string& a_name,
			     const string& t_name,
			     const string& w_name,
			     const string& d_name,
			     const string& r_name,
			     const string& c_name )
{
	//
	// RC =  0  Normal completion.
	// RC =  8  Area type not found on panel.
	// RC = 20  Severe error.
	//
	// Only AREATYPE(DYNAMIC) currently supported.
	//

	TRACE_FUNCTION() ;

	dynArea* da ;

	map<string, dynArea*>::iterator it ;

	it = dynAreaList.find( a_name ) ;
	if ( it == dynAreaList.end() )
	{
		err.setRC( 8 ) ;
		return ;
	}

	if ( t_name != "" )
	{
		funcPool->put2( err, t_name, "DYNAMIC" ) ;
		if ( err.error() ) { return ; }
	}

	da = it->second ;
	if ( w_name != "" )
	{
		funcPool->put2( err, w_name, da->dynArea_width ) ;
		if ( err.error() ) { return ; }
	}

	if ( d_name != "" )
	{
		funcPool->put2( err, d_name, da->dynArea_depth ) ;
		if ( err.error() ) { return ; }
	}

	if ( r_name != "" )
	{
		funcPool->put2( err, r_name, da->dynArea_row ) ;
		if ( err.error() ) { return ; }
	}

	if ( c_name != "" )
	{
		funcPool->put2( err, c_name, da->dynArea_col ) ;
		if ( err.error() ) { return ; }
	}
}


void pPanel::panel_cleanup( PANEL* p )
{
	TRACE_FUNCTION() ;

	const void* vptr = panel_userptr( p ) ;

	if ( vptr )
	{
		delete static_cast<const panel_data*>(vptr) ;
	}
}


void pPanel::set_panel_fscreen()
{
	TRACE_FUNCTION() ;

	const void* vptr = panel_userptr( panel ) ;

	if ( vptr )
	{
		const panel_data* pd = static_cast<const panel_data*>(vptr) ;
		pd->set_fscreen() ;
	}
}


void pPanel::unset_panel_fscreen()
{
	TRACE_FUNCTION() ;

	const void* vptr = panel_userptr( panel ) ;

	if ( vptr )
	{
		const panel_data* pd = static_cast<const panel_data*>(vptr) ;
		pd->unset_fscreen() ;
	}
}


void pPanel::unset_panel_decolourised()
{
	TRACE_FUNCTION() ;

	const void* vptr = panel_userptr( panel ) ;

	if ( vptr )
	{
		const panel_data* pd = static_cast<const panel_data*>(vptr) ;
		pd->unset_decolourised() ;
	}
}


bool pPanel::is_panel_decolourised()
{
	TRACE_FUNCTION() ;

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
	TRACE_FUNCTION() ;

	const void* vptr = panel_userptr( bpanel ) ;

	if ( vptr )
	{
		const panel_data* pd = static_cast<const panel_data*>(vptr) ;
		pd->set_frame_act() ;
	}
}


bool pPanel::is_panel_frame_inact()
{
	TRACE_FUNCTION() ;

	const void* vptr = panel_userptr( bpanel ) ;

	if ( vptr )
	{
		const panel_data* pd = static_cast<const panel_data*>(vptr) ;
		return pd->is_frame_inact() ;
	}

	return true ;
}


bool pPanel::is_inbox( PANEL* p )
{
	TRACE_FUNCTION() ;

	const void* vptr = panel_userptr( p ) ;

	if ( vptr )
	{
		const panel_data* pd = static_cast<const panel_data*>(vptr) ;
		return pd->is_inbox() ;
	}

	return false ;
}


void pPanel::set_panel_ttl( const string& winttl )
{
	TRACE_FUNCTION() ;

	const void* vptr = panel_userptr( bpanel ) ;
	if ( vptr )
	{
		const panel_data* pd = static_cast<const panel_data*>(vptr) ;
		pd->set_winttl( winttl ) ;
	}
}


const string pPanel::get_panel_ttl( PANEL* p )
{
	TRACE_FUNCTION() ;

	const void* vptr = panel_userptr( p ) ;
	if ( vptr )
	{
		const panel_data* pd = static_cast<const panel_data*>(vptr) ;
		return pd->get_winttl() ;
	}

	return "" ;
}


void pPanel::update_keylist_vars( errblock& err )
{
	TRACE_FUNCTION() ;

	if ( p_poolMGR->sysget( err, "ZKLUSE", PROFILE ) == "Y" || keyshr )
	{
		p_poolMGR->put( err, "ZKLNAME", keylistn, SHARED, SYSTEM ) ;
		p_poolMGR->put( err, "ZKLAPPL", keyappl,  SHARED, SYSTEM ) ;
		p_poolMGR->put( err, "ZKLTYPE", keytype,  SHARED, SYSTEM ) ;
	}
	else
	{
		p_poolMGR->put( err, "ZKLNAME", "", SHARED, SYSTEM ) ;
		p_poolMGR->put( err, "ZKLAPPL", "", SHARED, SYSTEM ) ;
		p_poolMGR->put( err, "ZKLTYPE", "", SHARED, SYSTEM ) ;
	}
}


void pPanel::update_zprim_var( errblock& err )
{
	TRACE_FUNCTION() ;

	p_poolMGR->put( err, "ZPRIM", ( primaryMenu ) ? "YES" : "NO", SHARED ) ;
}


void pPanel::make_field_visible( errblock& err,
				 field* pfield )
{
	TRACE_FUNCTION() ;

	Area* pArea = AreaNum[ pfield->field_area ] ;

	pArea->set_visible_depth( get_areared_lines( pArea ) ) ;

	if ( !pfield->field_visible || !pArea->in_visible_area( pfield ) )
	{
		pArea->make_visible( pfield ) ;
		rebuild_after_area_scroll( pArea ) ;
		display_panel( err ) ;
	}
}


bool pPanel::cursor_in_window( uint row,
			       uint col )
{
	//
	// Return true if the cursor is within a window, not including the border.
	// row/col is the physical position on the screen.
	//

	TRACE_FUNCTION() ;

	return ( win && ( win == pwin ) ) ? ( wenclose( win, row, col ) ) : true ;
}


bool pPanel::cursor_not_in_window( uint row,
				   uint col )
{
	//
	// Return false if the cursor is within a window, not including the border.
	// row/col is the physical position on the screen.
	//

	TRACE_FUNCTION() ;

	return ( !cursor_in_window( row, col ) ) ;
}


bool pPanel::cursor_in_window_border( uint row,
				      uint col )
{
	//
	// Return true if the cursor is on or within the window border.
	// row/col is the physical position on the screen.
	//

	TRACE_FUNCTION() ;

	return ( win && ( win == pwin ) ) ? wenclose( bwin, row, col ) : true ;
}


bool pPanel::cursor_on_border_line( uint row,
				    uint col )
{
	//
	// Return true if the cursor is on the window border (to start a window move).
	// row/col is the physical position on the screen.
	//

	TRACE_FUNCTION() ;

	return ( win && ( win == pwin ) ) ? ( wenclose( bwin, row, col ) && !wenclose( win, row, col ) ) : false ;
}


bool pPanel::cursor_on_msg_window( uint row,
				   uint col )
{
	//
	// Return true if the cursor is on a message window.
	//

	TRACE_FUNCTION() ;

	if ( smwin && !panel_hidden( smpanel) && is_inbox( smpanel ) )
	{
		return wenclose( smwin, row, col ) ;
	}

	if ( lmwin && !panel_hidden( lmpanel) )
	{
		return wenclose( lmwin, row, col ) ;
	}

	return false ;
}


bool pPanel::hide_msg_window( uint row,
			      uint col )
{
	//
	// If the cursor is on a message window, hide the window and return true.
	// The underlying panel is not submitted for processing in this case.
	//

	TRACE_FUNCTION() ;

	bool result = false ;

	if ( smwin && !panel_hidden( smpanel) && is_inbox( smpanel ) )
	{
		if ( wenclose( smwin, row, col ) )
		{
			hide_panel( smpanel ) ;
			result = true ;
		}
	}

	if ( lmwin && !panel_hidden( lmpanel) && is_inbox( lmpanel ) )
	{
		if ( wenclose( lmwin, row, col ) )
		{
			hide_panel( lmpanel ) ;
			result = true ;
		}
	}

	return result ;
}


void pPanel::getlist( errblock& err,
		      string& s,
		      vector<string>& v )
{
	//
	// Split a string up into words and place into string vector v.  Words may be separated by
	// blanks or a comma.
	// Single quotes can be used if words contain spaces or commas (quotes are removed) and two single
	// quotes within quotes represent a single quote.
	//

	TRACE_FUNCTION() ;

	size_t i = 0 ;
	size_t j = 0 ;

	v.clear() ;

	while ( true )
	{
		i = s.find_first_not_of( " ,", j ) ;
		if ( i == string::npos ) { return ; }
		if ( s[ i ] == '\'' )
		{
			++i ;
			j = s.find( '\'', i ) ;
			if ( j == string::npos )
			{
				err.seterrid( TRACE_INFO(), "PSYE033F" ) ;
				return ;
			}
			while ( s.compare( j, 2, "\'\'" ) == 0 )
			{
				s.erase( j, 1 ) ;
				++j ;
				if ( j == s.size() )
				{
					err.seterrid( TRACE_INFO(), "PSYE033F" ) ;
					return ;
				}
				j = s.find( '\'', j ) ;
				if ( j == string::npos )
				{
					err.seterrid( TRACE_INFO(), "PSYE033F" ) ;
					return ;
				}
			}
			++j ;
			if ( j < s.size() && s[ j ] != ' ' && s[ j ] != ',' )
			{
				err.seterrid( TRACE_INFO(), "PSYE037G" ) ;
				return ;
			}
			v.push_back( s.substr( i, j-i-1 ) ) ;
		}
		else
		{
			j = s.find_first_of( " ,", i ) ;
			if ( j == string::npos )
			{
				v.push_back( s.substr( i ) ) ;
				return ;
			}
			v.push_back( s.substr( i, j-i ) ) ;
			if ( v.back().find_first_of( "<(+|);->:=" ) != string::npos )
			{
				err.seterrid( TRACE_INFO(), "PSYE037J" ) ;
				return ;
			}
		}
	}
}


int pPanel::get_value( errblock& err,
		       string s,
		       const string& text,
		       string& var,
		       int maxval )
{
	//
	// Resolve string s, either as a numeric value, MAX (if maxval entered), or
	// a dialogue variable containing a numeric value.
	//
	// Also return the variable name, if used.
	//

	TRACE_FUNCTION() ;

	var = "" ;

	if ( s.front() == '&' )
	{
		var = s.substr( 1 ) ;
		if ( !isvalidName( var ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE011Y", s, text ) ;
			return 0 ;
		}
		s = getDialogueVar( err, var ) ;
		if ( err.error() ) { return 0 ; }
		if ( !isnumeric( s ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE011Z", s ) ;
			return 0 ;
		}
	}
	else if ( maxval > 0 && s == "MAX" )
	{
		return ( maxval - 2 ) ;
	}
	else if ( !isnumeric( s ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE011Z", s ) ;
		return 0 ;
	}

	return ds2d( s ) ;
}


string pPanel::sub_vars( string s )
{
	//
	// In string, s, substitute variables starting with '&' for their dialogue value.
	// The variable is delimited by any non-valid variable characters.
	// Rules: A '.' at the end of a variable, concatinates the value with the string following the dot
	//        .. reduces to .
	//        && reduces to & with no variable substitution.
	//

	TRACE_FUNCTION() ;

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

	return s ;
}


string pPanel::sub_vars( string s,
			 bool& dvars )
{
	//
	// In string, s, substitute variables starting with '&' for their dialogue value.
	// The variable is delimited by any non-valid variable characters.
	// Rules: A '.' at the end of a variable, concatinates the value with the string following the dot
	//        .. reduces to .
	//        && reduces to & with no variable substitution.
	//

	TRACE_FUNCTION() ;

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
				dvars = true ;
			}
		}
	}

	return s ;
}
