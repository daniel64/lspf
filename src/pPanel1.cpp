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
#define MOD_NAME pPanel1
#define LOGOUT   aplog


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
	panelTitle  = ""     ;
	panelDescr  = ""     ;
	abIndex     = 0      ;
	opt_field   = 0      ;
	tb_model    = false  ;
	tb_depth    = 0      ;
	tb_fields   = ""     ;
	tb_clear    = ""     ;
	tb_scan     = false  ;
	tb_lcol     = 0      ;
	tb_lsz      = 0      ;
	win_addpop  = false  ;
	smp_created = false  ;
	lmp_created = false  ;
	win_row     = 0      ;
	win_col     = 0      ;
	win_width   = 0      ;
	win_depth   = 0      ;
	dyn_depth   = 0      ;
	ZPHELP      = ""     ;
	CMDfield    = "ZCMD" ;
	Home        = "ZCMD" ;
	fwin        = NULL   ;
	pwin        = NULL   ;
	bwin        = NULL   ;
	idwin       = NULL   ;
}


pPanel::~pPanel()
{
	// iterate over the 4 panel widget types, literal, field, dynArea, boxes and delete them.
	// Delete the main window/panel, popup panel and any message windows/panels created (free any userdata first)

	map<string, field *>::iterator it1;
	map<string, dynArea *>::iterator it2;


	for ( it1 = fieldList.begin() ; it1 != fieldList.end() ; it1++ )
	{
		delete it1->second ;
	}

	for ( it2 = dynAreaList.begin() ; it2 != dynAreaList.end() ; it2++ )
	{
		delete it2->second ;
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
	if ( smp_created )
	{
		panel_cleanup( smpanel ) ;
		del_panel( smpanel ) ;
		delwin( smwin )      ;
	}
	if ( lmp_created )
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

	// If the variable is not found, create a null implicit entry in the function pool

	// RC =  0 Normal completion
	// RC =  8 Variable not found
	// RC = 20 Severe error

	string * p_str    ;
	dataType var_type ;

	var_type = p_funcPOOL->getType( err, var, NOCHECK ) ;
	if ( err.error() ) { return "" ; }

	if ( err.RC0() )
	{
		switch ( var_type )
		{
			case INTEGER:
				return d2ds( p_funcPOOL->get( err, 0, var_type, var ) ) ;
			case STRING:
				return p_funcPOOL->get( err, 0, var, NOCHECK ) ;
		}
	}
	else
	{
		p_str = p_poolMGR->vlocate( err, var ) ;
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

	// RC =  0 Normal completion
	// RC = 20 Severe error

	p_funcPOOL->put( err, var, val ) ;
}


void pPanel::syncDialogueVar( const string& var )
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

	errblock err ;

	p_funcPOOL->put( err, var, p_poolMGR->get( err, var, ASIS ) ) ;
}


void pPanel::set_popup( int sp_row, int sp_col )
{
	win_addpop = true ;
	win_row    = (sp_row + win_depth + 1) < ZSCRMAXD ? sp_row : (ZSCRMAXD - win_depth - 1) ;
	win_col    = (sp_col + win_width + 1) < ZSCRMAXW ? sp_col : (ZSCRMAXW - win_width - 1) ;
}


void pPanel::remove_popup()
{
	win_addpop = false ;
	win_row    = 0     ;
	win_col    = 0     ;
}


void pPanel::move_popup()
{
	mvwin( win, win_row, win_col )      ;
	mvwin( bwin, win_row-1, win_col-1 ) ;
}


void pPanel::display_panel( errblock& err )
{
	// fwin - created unconditionally and is the full-screen window
	// pwin - pop-up window only created if the panel contains a WINDOW(w,d) statement
	// (associate ncurses panel 'panel' with whichever is active and set WSCRMAX?)

	// Use fwin if no pwin exists, or no ADDPOP() has been done (not exactly how real ISPF works)

	string winttl ;
	string panttl ;

	err.setRC( 0 ) ;

	update_keylist_vars() ;

	if ( win_addpop && pwin )
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
		if ( winttl != "" )
		{
			winttl = " "+ winttl +" " ;
			mvwaddstr( bwin, 0, ( WSCRMAXW - winttl.size() ) / 2, winttl.c_str() ) ;
		}
		wattroff( bwin, cuaAttr[ AWF ] ) ;
		top_panel( bpanel ) ;
		top_panel( panel ) ;
		WSCRMAXW = win_width ;
		WSCRMAXD = win_depth ;
	}
	else
	{
		win = fwin ;
		if ( bwin )
		{
			hide_panel( bpanel ) ;
		}
		if ( win != panel_window( panel ) )
		{
			replace_panel( panel, win ) ;
		}
		top_panel( panel ) ;
		WSCRMAXW = ZSCRMAXW ;
		WSCRMAXD = ZSCRMAXD ;
	}

	werase( win ) ;

	panttl = sub_vars( panelTitle ) ;
	wattrset( win, cuaAttr[ PT ] ) ;
	mvwaddstr( win, 2, ( WSCRMAXW - panttl.size() ) / 2, panttl.c_str() ) ;
	wattroff( win, cuaAttr[ PT ] ) ;

	if ( tb_model )
	{
		p_funcPOOL->put( err, "ZTDSELS", 0 ) ;
		display_tb_mark_posn() ;
		tb_fields_active_inactive() ;
	}

	display_ab()     ;
	display_literals() ;
	display_fields() ;
	display_boxes()  ;
	hide_pd()        ;
	display_pd()     ;
	display_msg()    ;
	display_id()     ;

	if ( ALARM ) { beep() ; }

}


void pPanel::redraw_fields()
{
	// Re-draw all fields in a window (eg after a .SHOW/.HIDE NULLS command)

	display_fields() ;
}


void pPanel::redisplay_panel()
{
	// Refresh the screen by re-drawing all the elements

	string panttl ;
	errblock err  ;

	werase( win ) ;

	panttl = sub_vars( panelTitle ) ;
	wattrset( win, cuaAttr[ PT ] ) ;
	mvwaddstr( win, 2, ( WSCRMAXW - panttl.size() ) / 2, panttl.c_str() ) ;
	wattroff( win, cuaAttr[ PT ] ) ;

	if ( tb_model )
	{
		p_funcPOOL->put( err, "ZTDSELS", 0 ) ;
		display_tb_mark_posn() ;
		tb_fields_active_inactive() ;
	}

	display_ab()     ;
	display_literals() ;
	display_fields() ;
	display_boxes()  ;
	hide_pd()        ;
	display_pd()     ;
	display_msg()    ;
	display_id()     ;
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

	//  If END entered with pull down displayed, remove the pull down and clear ZVERB.
	//  For dynamic areas, also update the shadow variable to indicate character deletes (0xFF).

	int fieldNum    ;
	int scrollAmt   ;
	int curpos      ;
	int p           ;
	int offset      ;

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

	fieldNum = 0  ;
	curpos   = 1  ;
	MSGID    = "" ;
	MSGLOC   = "" ;

	cursor_set  = false ;
	message_set = false ;

	CMDVerb     = p_poolMGR->get( err, "ZVERB", SHARED ) ;
	end_pressed = findword( CMDVerb, "END EXIT RETURN" ) ;

	if ( CMDVerb == "END" && pdActive )
	{
		p_poolMGR->put( err, "ZVERB", "",  SHARED ) ;
		end_pressed = false ;
		pdActive    = false ;
	}

	if ( pdActive )
	{
		hide_pd()        ;
		pdActive = false ;
	}

	if ( scrollOn )
	{
		it = fieldList.find( "ZSCROLL" ) ;
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
				default:  setMessageCond( "PSYS011I" ) ;
					  setCursorCond( "ZSCROLL" )   ;
			}
		}
		p_funcPOOL->put( err, it->first, it->second->field_value ) ;
	}

	p_funcPOOL->put( err, "ZCURFLD", "" ) ;
	p_funcPOOL->put( err, "ZCURPOS", 1  ) ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( it->second->field_changed )
		{
			if ( it->second->field_dynArea )
			{
				it->second->field_remove_nulls_da()        ;
				p        = it->first.find_first_of( '.' )  ;
				fieldNam = it->first.substr( 0, p )        ;
				fieldNum = ds2d( it->first.substr( p+1 ) ) ;
				darea    = p_funcPOOL->vlocate( err, fieldNam ) ;
				offset   = fieldNum*it->second->field_length      ;
				if ( it->second->field_dynArea_ptr->dynArea_DataModsp )
				{
					it->second->field_DataMod_to_UserMod( darea, offset ) ;
				}
				darea->replace( offset, it->second->field_length, it->second->field_value ) ;
				sname    = it->second->field_dynArea_ptr->dynArea_shadow_name ;
				shadow   = p_funcPOOL->vlocate( err, sname ) ;
				shadow->replace( offset, it->second->field_length, it->second->field_shadow_value ) ;
			}
			else
			{
				it->second->field_prep_input() ;
				it->second->field_set_caps()   ;
				var_type = p_funcPOOL->getType( err, it->first ) ;
				if ( err.RC0() && var_type == INTEGER )
				{
					if ( datatype( it->second->field_value, 'W' ) )
					{
						p_funcPOOL->put( err, it->first, ds2d( it->second->field_value ) ) ;
						if ( err.error() ) { return ; }
					}
					else
					{
						setMessageCond( "PSYS011G" ) ;
						setCursorCond( it->first )   ;
					}
				}
				else
				{
					p_funcPOOL->put( err, it->first, it->second->field_value, NOCHECK ) ;
					if ( err.error() ) { return ; }
				}
			}
		}
		if ( it->second->cursor_on_field( p_row, p_col ) )
		{
			curpos = p_col - it->second->field_col + 1 ;
			if ( it->second->field_dynArea )
			{
				p        = it->first.find_first_of( '.' )  ;
				fieldNam = it->first.substr( 0, p )        ;
				fieldNum = ds2d( it->first.substr( p+1 ) ) ;
				p_funcPOOL->put( err, "ZCURFLD", fieldNam ) ;
				p_funcPOOL->put( err, "ZCURPOS", ( fieldNum*it->second->field_length + curpos ) ) ;
				fieldNum++ ;
			}
			else
			{
				p_funcPOOL->put( err, "ZCURFLD", it->first ) ;
				p_funcPOOL->put( err, "ZCURPOS", curpos )    ;
			}
		}
	}

	p_poolMGR->put( err, "ZSCROLLA", "",  SHARED ) ;
	p_poolMGR->put( err, "ZSCROLLN", "0", SHARED ) ;

	if ( scrollOn && findword( CMDVerb, "UP DOWN LEFT RIGHT" ) )
	{
		CMD    = upper( fieldList[ CMDfield ]->field_value ) ;
		msgfld = CMDfield ;
		if ( CMD == "" || ( !findword( CMD, "M MAX C CSR D DATA H HALF P PAGE" ) && !isnumeric( CMD ) ) )
		{
			CMD    = fieldList[ "ZSCROLL" ]->field_value ;
			msgfld = "ZSCROLL" ;
		}
		else
		{
			fieldList[ CMDfield ]->field_value = "" ;
			p_funcPOOL->put( err, CMDfield, "" )  ;
		}
		if      ( tb_model )                          { scrollAmt = tb_depth  ; }
		else if ( findword( CMDVerb, "LEFT RIGHT" ) ) { scrollAmt = dyn_width ; }
		else                                          { scrollAmt = dyn_depth ; }
		if ( isnumeric( CMD ) )
		{
			if ( CMD.size() > 6 )
			{
				setMessageCond( "PSYS011I" ) ;
				setCursorCond( msgfld )      ;
			}
			else
			{
				p_poolMGR->put( err, "ZSCROLLA", CMD, SHARED ) ;
				p_poolMGR->put( err, "ZSCROLLN", CMD, SHARED ) ;
			}
		}
		else if ( CMD[ 0 ] == 'M' )
		{
			p_poolMGR->put( err, "ZSCROLLA", "MAX", SHARED ) ;
		}
		else if ( CMD[ 0 ] == 'C' )
		{
			if ( fieldNum == 0 )
			{
				p_poolMGR->put( err, "ZSCROLLN", d2ds( scrollAmt ),  SHARED ) ;
			}
			else if ( CMDVerb[ 0 ] == 'U' )
			{
				p_poolMGR->put( err, "ZSCROLLN", d2ds( dyn_depth-fieldNum ), SHARED ) ;
			}
			else if ( CMDVerb[ 0 ] == 'D' )
			{
				p_poolMGR->put( err, "ZSCROLLN", d2ds( fieldNum-1 ), SHARED ) ;
			}
			else if ( CMDVerb[ 0 ] == 'L' )
			{
				p_poolMGR->put( err, "ZSCROLLN", d2ds( dyn_width-curpos ), SHARED ) ;
			}
			else
			{
				p_poolMGR->put( err, "ZSCROLLN", d2ds( curpos ), SHARED ) ;
			}
			p_poolMGR->put( err, "ZSCROLLA", "CSR", SHARED ) ;
		}
		else if ( CMD[ 0 ] == 'D' )
		{
			p_poolMGR->put( err, "ZSCROLLN", d2ds( scrollAmt ), SHARED ) ;
			p_poolMGR->put( err, "ZSCROLLA", "DATA", SHARED ) ;
		}
		else if ( CMD[ 0 ] == 'H' )
		{
			p_poolMGR->put( err, "ZSCROLLN", d2ds( scrollAmt/2 ), SHARED ) ;
			p_poolMGR->put( err, "ZSCROLLA", "HALF", SHARED ) ;
		}
		else
		{
			p_poolMGR->put( err, "ZSCROLLN", d2ds( scrollAmt ), SHARED ) ;
			p_poolMGR->put( err, "ZSCROLLA", "PAGE", SHARED ) ;
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

	err.setRC( 0 ) ;
	process_panel_stmnts( err, ln, reinstmnts ) ;
}


void pPanel::display_panel_proc( errblock& err, int ln )
{
	// Perform panel )PROC processing

	// If cursor on a point-and-shoot field (PS), set variable as in the )PNTS panel section if defined there

	int i ;
	string fieldNam ;
	map<string, field *>::iterator it;

	err.setRC( 0 ) ;
	process_panel_stmnts( err, ln, procstmnts ) ;
	if ( err.error() ) { return ; }

	fieldNam = p_funcPOOL->get( err, 0, "ZCURFLD" ) ;
	if ( fieldNam != "" )
	{
		it = fieldList.find( fieldNam ) ;
		if ( it != fieldList.end() )
		{
			if ( it->second->field_cua == PS )
			{
				if ( pntsTable.count( fieldNam ) > 0 )
				{
					p_funcPOOL->put( err, pntsTable[ fieldNam ].pnts_var, pntsTable[ fieldNam ].pnts_val ) ;
				}
			}
		}
	}
	else
	{
		for ( i = 0 ; i < literalList.size() ; i++ )
		{
			if ( !literalList.at( i )->cursor_on_literal( p_row, p_col ) ) { continue ; }
			fieldNam = literalList.at( i )->literal_name ;
			if ( fieldNam == "" ) { break ; }
			if ( pntsTable.count( fieldNam ) > 0 )
			{
				p_funcPOOL->put( err, pntsTable[ fieldNam ].pnts_var, pntsTable[ fieldNam ].pnts_val ) ;
			}
			break ;
		}
	}
}


void pPanel::process_panel_stmnts( errblock& err, int ln, vector<panstmnt* >& panstmnts )
{
	// General routine to process panel statements.  Pass address of the panel statements vector for the
	// panel section being processed.

	// Set RC1=8 from VGET/VPUT to 0 as it isn't an error in this case
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

	vector<panstmnt* >::iterator ips ;

	g_label     = ""    ;
	if_column   = 0     ;
	if_skip     = false ;

	for ( ips = panstmnts.begin() ; ips != panstmnts.end() ; ips++ )
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
			refresh_fields( (*ips)->ps_rlist ) ;
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
			if ( ifstmnt->if_verify->ver_numeric )
			{
				ifstmnt->if_true = isnumeric( fieldVal ) ;
			}
			else if ( ifstmnt->if_verify->ver_list || ifstmnt->if_verify->ver_listx )
			{
				for ( it  = ifstmnt->if_verify->ver_vlist.begin() ;
				      it != ifstmnt->if_verify->ver_vlist.end()  ; it++ )
				{
					if ( sub_vars( (*it) ) == fieldVal ) { break ; }
				}
				if ( ifstmnt->if_verify->ver_list )
				{
					ifstmnt->if_true = ( it != ifstmnt->if_verify->ver_vlist.end() ) ;
				}
				else
				{
					ifstmnt->if_true = ( it == ifstmnt->if_verify->ver_vlist.end() ) ;
				}
			}
			else if ( ifstmnt->if_verify->ver_pict )
			{
				ifstmnt->if_true = ispict( fieldVal, ifstmnt->if_verify->ver_vlist[ 0 ] ) ;
			}
			else if ( ifstmnt->if_verify->ver_hex )
			{
				ifstmnt->if_true = ishex( fieldVal ) ;
			}
			else if ( ifstmnt->if_verify->ver_octal )
			{
				ifstmnt->if_true = isoctal( fieldVal ) ;
			}
		}
		return ;
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

	if ( trunc->trnc_field.front() == '.' )
	{
		t = getControlVar( err, trunc->trnc_field ) ;
	}
	else
	{
		t = getDialogueVar( err, trunc->trnc_field ) ;
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

	p_funcPOOL->put( err, ".TRAIL", dTrail, NOCHECK ) ;
	if ( err.error() ) { return "" ; }

	return t ;
}


string pPanel::process_panel_trans( errblock& err, int ln, TRANS* trans )
{
	string fieldNam ;
	string t ;

	vector<pair<string,string>>::iterator itt ;

	t = getDialogueVar( err, trans->trns_field ) ;
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
				setMessageCond( sub_vars( trans->trns_msgid ) ) ;
				if ( trans->trns_pnfield )
				{
					fieldNam = trans->trns_field ;
					if ( trans->trns_tbfield )
					{
						fieldNam += "." + d2ds( ln ) ;
					}
					setCursorCond( fieldNam ) ;
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
	if ( verify->ver_tbfield ) { fieldNam += "." + d2ds( ln ) ; }

	fieldVal = getDialogueVar( err, verify->ver_var ) ;
	if ( err.error() ) { return ; }

	if ( verify->ver_nblank )
	{
		if ( fieldVal == "" )
		{
			setMessageCond( verify->ver_msgid == "" ? "PSYS019" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				setCursorCond( fieldNam ) ;
			}
		}
	}
	if ( fieldVal == "" )
	{
		return ;
	}
	if ( verify->ver_numeric )
	{
		if ( !isnumeric( fieldVal ) )
		{
			setMessageCond( verify->ver_msgid == "" ? "PSYS011A" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				setCursorCond( fieldNam ) ;
			}
		}
	}
	else if ( verify->ver_list || verify->ver_listx )
	{
		for ( it = verify->ver_vlist.begin() ; it != verify->ver_vlist.end() ; it++ )
		{
			if ( sub_vars( (*it) ) == fieldVal ) { break ; }
		}
		if ( ( verify->ver_list  && it == verify->ver_vlist.end() ) ||
		     ( verify->ver_listx && it != verify->ver_vlist.end() ) )
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
			msg = verify->ver_list ? "PSYS011B" : "PSYS012Q" ;
			setMessageCond( verify->ver_msgid == "" ? msg : sub_vars( verify->ver_msgid ) ) ;
			if ( MSGID == msg )
			{
				p_poolMGR->put( err, "ZZSTR1", t, SHARED ) ;
				if ( err.error() ) { return ; }
			}
			if ( verify->ver_pnfield )
			{
				setCursorCond( fieldNam ) ;
			}
		}
	}
	else if ( verify->ver_pict )
	{
		if ( !ispict( fieldVal, verify->ver_vlist[ 0 ] ) )
		{
			p_poolMGR->put( err, "ZZSTR1", d2ds( verify->ver_vlist[ 0 ].size() ), SHARED ) ;
			if ( err.error() ) { return ; }
			p_poolMGR->put( err, "ZZSTR2", verify->ver_vlist[ 0 ], SHARED ) ;
			if ( err.error() ) { return ; }
			setMessageCond( verify->ver_msgid == "" ? "PSYS011N" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				setCursorCond( fieldNam ) ;
			}
		}
	}
	else if ( verify->ver_hex )
	{
		if ( !ishex( fieldVal ) )
		{
			setMessageCond( verify->ver_msgid == "" ? "PSYS011H" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				setCursorCond( fieldNam ) ;
			}
		}
	}
	else if ( verify->ver_octal )
	{
		if ( !isoctal( fieldVal ) )
		{
			setMessageCond( verify->ver_msgid == "" ? "PSYS011F" : sub_vars( verify->ver_msgid ) ) ;
			if ( verify->ver_pnfield )
			{
				setCursorCond( fieldNam ) ;
			}
		}
	}
}


void pPanel::process_panel_vputget( errblock& err, VPUTGET* vputget )
{
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
			val = p_funcPOOL->get( err, 8, var ) ;
			if ( err.error() ) { return ; }
			if ( err.RC0() )
			{
				p_poolMGR->put( err, var, val, vputget->vpg_pool ) ;
				if ( err.error() ) { return ; }
			}
		}
	}
	else
	{
		for ( j = 1 ; j <= ws ; j++ )
		{
			var = sub_vars( word( vputget->vpg_vars, j ) ) ;
			val = p_poolMGR->get( err, var, vputget->vpg_pool ) ;
			if ( err.error() ) { return ; }
			if ( err.RC0() )
			{
				p_funcPOOL->put( err, var, val ) ;
			}
		}
	}
	if ( err.RC8() ) { err.setRC( 0 ) ; }
}


void pPanel::process_panel_assignment( errblock& err, int ln, ASSGN* assgn )
{
	string t ;
	string fieldNam ;

	if ( assgn->as_isvar )
	{
		if ( assgn->as_rhs.front() == '.' )
		{
			t = getControlVar( err, assgn->as_rhs ) ;
			if ( err.error() ) { return ; }
		}
		else
		{
			t = getDialogueVar( err, assgn->as_rhs ) ;
			if ( err.error() ) { return ; }
		}
		t = sub_vars( t ) ;
	}

	if      ( assgn->as_trans  )  { t = process_panel_trans( err, ln, assgn->as_trans ) ; }
	else if ( assgn->as_trunc  )  { t = process_panel_trunc( err, assgn->as_trunc )     ; }
	else if ( assgn->as_retlen )  { t = d2ds( t.size() )                 ; }
	else if ( assgn->as_reverse ) { reverse( t.begin(), t.end() )        ; }
	else if ( assgn->as_upper  )  { iupper( t )                          ; }
	else if ( assgn->as_words  )  { t = d2ds( words( t ) )               ; }
	else if ( assgn->as_chkexst ) { t = exists( t ) ? "1" : "0"          ; }
	else if ( assgn->as_chkfile ) { t = is_regular_file( t ) ? "1" : "0" ; }
	else if ( assgn->as_chkdir )  { t = is_directory( t )    ? "1" : "0" ; }
	else if ( assgn->as_rhs.front() == '.' )
	{
		t = getControlVar( err, assgn->as_rhs ) ;
		if ( err.error() ) { return ; }
	}
	else
	{
		t = sub_vars( assgn->as_rhs ) ;
	}

	if ( assgn->as_lhs.front() == '.' )
	{
		setControlVar( err, ln, assgn->as_lhs, t ) ;
		if ( err.error() ) { return ; }
	}
	else if ( assgn->as_isattr )
	{
		fieldNam = assgn->as_lhs ;
		if ( assgn->as_istb )
		{
			fieldList[ fieldNam +"."+ d2ds( ln ) ]->field_attr( err, t ) ;
			if ( err.error() ) { return ; }
			attrList.push_back( fieldNam +"."+ d2ds( ln ) ) ;
		}
		else
		{
			fieldList[ fieldNam ]->field_attr( err, t ) ;
			if ( err.error() ) { return ; }
			attrList.push_back( fieldNam ) ;
		}
	}
	else
	{
		putDialogueVar( err, assgn->as_lhs, t ) ;
		if ( err.error() ) { return ; }
		if ( assgn->as_lhs == "ZPRIM" )
		{
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
	else if ( svar == ".CSRPOS" )
	{
		return d2ds( p_funcPOOL->get( err, 0, INTEGER, "ZCURPOS" ) ) ;
	}
	else if ( svar == ".CURSOR" )
	{
		return p_funcPOOL->get( err, 0, "ZCURFLD" ) ;
	}
	else if ( svar == ".HELP" )
	{
		return ZPHELP ;
	}
	else if ( svar == ".MSG" )
	{
		return MSGID ;
	}
	else if ( svar == ".RESP" )
	{
		return end_pressed ? "END" : "ENTER" ;
	}
	else if ( svar == ".TRAIL" )
	{
		return p_funcPOOL->get( err, 8, ".TRAIL", NOCHECK ) ;
	}
	return svar ;
}


void pPanel::setControlVar( errblock& err, int ln, const string& svar, const string& sval )
{
	if ( svar == ".AUTOSEL" )
	{
		// ???
	}
	else if ( svar == ".ALARM" )
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
			return ;
		}
	}
	else if ( svar == ".BROWSE" )
	{
		forBrowse = ( sval == "YES" ) ;
	}
	else if ( svar == ".CURSOR" )
	{
		if ( !cursor_set )
		{
			p_funcPOOL->put( err, "ZCURFLD", sval ) ;
			if ( err.error() ) { return ; }
			CURFLD = wordpos( sval, tb_fields ) == 0 ? sval : sval +"."+ d2ds( ln ) ;
			CURPOS = 1 ;
			cursor_set = true ;
		}
	}
	else if ( svar == ".CSRPOS" )
	{
		if ( !isnumeric( sval ) )
		{
			err.seterrid( "PSYE041J" ) ;
			return ;
		}
		CURPOS = ds2d( sval ) ;
		p_funcPOOL->put( err, "ZCURPOS", CURPOS ) ;
		if ( err.error() ) { return ; }
	}
	else if ( svar == ".CSRROW" )
	{
		if ( !isnumeric( sval ) )
		{
			err.seterrid( "PSYE041H" ) ;
			return ;
		}
		p_funcPOOL->put( err, ".CSRROW", sval, NOCHECK ) ;
		if ( err.error() ) { return ; }
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
		setMessageCond( sval ) ;
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
			return ;
		}
	}
}


void pPanel::setMessageCond( const string& msg )
{
	if ( !message_set )
	{
		MSGID       = msg  ;
		message_set = true ;
	}
}


void pPanel::setCursorCond( const string& csr )
{
	errblock err ;

	if ( !cursor_set )
	{
		CURFLD = csr ;
		MSGLOC = csr ;
		p_funcPOOL->put( err, "ZCURFLD", csr ) ;
		cursor_set = true ;
	}
}


void pPanel::refresh_fields( const string& fields )
{
	// Update the field value from the dialogue variable.  Apply any field justification defined.

	int j ;
	int k ;

	errblock err ;

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
		itf->second->field_prep_display() ;
	}

	for ( itd = dynAreaList.begin() ; itd != dynAreaList.end() ; itd++ )
	{
		if ( fields != "*" && !findword( itd->first, fields ) ) { continue ; }
		k      = itd->second->dynArea_width ;
		darea  = p_funcPOOL->vlocate( err, itd->first ) ;
		sname  = itd->second->dynArea_shadow_name ;
		shadow = p_funcPOOL->vlocate( err, sname ) ;
		for ( int i = 0 ; i < itd->second->dynArea_depth ; i++ )
		{
			j   = i * itd->second->dynArea_width ;
			itf = fieldList.find( itd->first +"."+ d2ds( i ) ) ;
			itf->second->field_value        = darea->substr( j, k )  ;
			itf->second->field_shadow_value = shadow->substr( j, k ) ;
		}
	}
}


void pPanel::refresh_fields()
{
	// Update all field values from their dialogue variables and display.

	int j   ;
	int k   ;

	string sname    ;
	string * darea  ;
	string * shadow ;

	errblock err ;

	map<string, field *>::iterator   itf ;
	map<string, dynArea *>::iterator itd ;

	bool snulls = ( p_poolMGR->get( err, "ZNULLS", SHARED ) == "YES" ) ;
	char pad    = p_poolMGR->get( err, "ZPADC", PROFILE ).front() ;

	for ( itf = fieldList.begin() ; itf != fieldList.end() ; itf++ )
	{
		if ( itf->second->field_dynArea ) { continue ; }
		itf->second->field_value = getDialogueVar( err, itf->first ) ;
		itf->second->field_prep_display() ;
		itf->second->display_field( win, pad, snulls ) ;
	}

	for ( itd = dynAreaList.begin() ; itd != dynAreaList.end() ; itd++ )
	{
		k      = itd->second->dynArea_width ;
		darea  = p_funcPOOL->vlocate( err, itd->first ) ;
		sname  = itd->second->dynArea_shadow_name ;
		shadow = p_funcPOOL->vlocate( err, sname ) ;
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
	a.field_input  = !cuaAttrProt[ fType ] ;
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


void pPanel::create_pdc( errblock& err, const string& pline )
{
	// ab is a vector list of action-bar-choices (abc objects)
	// Each action-bar-choice is a vector list of pull-down-choices (pdc objects)

	int i  ;
	int p1 ;

	string abc_name ;

	string pdc_name ;
	string pdc_run  ;
	string pdc_parm ;
	string pdc_unavail ;
	string rest ;

	abc t_abc ;

	abc_name = word( pline, 2 ) ;
	rest     = subword( pline, 3 ) ;

	if ( rest.size() > 1 && rest.front() == '\"' )
	{
		p1 = rest.find( '\"', 1 ) ;
		if ( p1 == string::npos )
		{
			err.seterrid( "PSYE033F" ) ;
			return ;
		}
		pdc_name = rest.substr( 1, p1-1 ) ;
		rest.erase( 0, p1+1 ) ;
	}
	else
	{
		pdc_name = word( rest, 1 ) ;
		idelword( rest, 1, 1 ) ;
	}

	if ( word( rest, 1 ) != "ACTION" )
	{
		err.seterrid( "PSYE035J" ) ;
		return ;
	}

	idelword( rest, 1, 1 ) ;

	pdc_run = parseString( err, rest, "RUN()" ) ;
	if ( err.error() ) { return ; }

	pdc_parm = parseString( err, rest, "PARM()" ) ;
	if ( err.error() ) { return ; }

	if ( pdc_parm.size() > 1 && pdc_parm.front() == '"' )
	{
		if ( pdc_parm.back() != '"' )
		{
			err.seterrid( "PSYE033F" ) ;
			return ;
		}
		pdc_parm.erase( 0, 1 ) ;
		pdc_parm.pop_back()    ;
	}

	pdc_unavail = parseString( err, rest, "UNAVAIL()" ) ;
	if ( err.error() ) { return ; }

	if ( rest != "" )
	{
		err.seterrid( "PSYE032H", rest ) ;
		return ;
	}

	for ( i = 0 ; i < ab.size() ; i++ )
	{
		if ( ab.at( i ).abc_name == abc_name )
		{
			if ( ab.at( i ).pdc_exists( pdc_name ) )
			{
				err.seterrid( "PSYE041B", pdc_name, abc_name ) ;
				return ;
			}
			break ;
		}
	}

	if ( i == ab.size() )
	{
		t_abc.abc_name = abc_name ;
		t_abc.abc_col  = abc_pos  ;
		abc_pos        = abc_pos + abc_name.size() + 2 ;
		ab.push_back( t_abc ) ;
	}

	pdc t_pdc( pdc_name, pdc_run, pdc_parm, pdc_unavail ) ;

	ab.at( i ).add_pdc( t_pdc ) ;
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
			llog( "W", "Shadow variable '"+ sname +"' size is smaller than the data variable " << itd->first << " size.  Results are be unpredictable" << endl ) ;
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
	vector<literal *>::iterator it ;

	for ( it = literalList.begin() ; it != literalList.end() ; it++ )
	{
		(*it)->literal_display( win, sub_vars( (*it)->literal_value ) ) ;
	}

}


void pPanel::display_fields()
{
	errblock err ;
	map<string, field *>::iterator it;

	bool snulls = ( p_poolMGR->get( err, "ZNULLS", SHARED ) == "YES" ) ;
	char pad    = p_poolMGR->get( err, "ZPADC", PROFILE ).front() ;

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
	int i ;

	for ( i = 0 ; i < ab.size() ; i++ )
	{
		ab.at( i ).display_abc_unsel( win ) ;
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


void pPanel::cursor_to_field( int& RC1, string f_name, int f_pos )
{
	int oX ;
	int oY ;

	map<string, field   *>::iterator itf ;
	map<string, dynArea *>::iterator itd ;

	RC1 = 0 ;

	if ( f_name == "" )
	{
		if ( CURFLD == "" ) { return ; }
		f_name = CURFLD ;
		f_pos  = CURPOS ;
	}

	itf = fieldList.find( f_name ) ;
	if ( itf == fieldList.end() )
	{
		itd = dynAreaList.find( f_name ) ;
		if ( itd == dynAreaList.end() )
		{
			p_col = 0 ;
			p_row = 0 ;
			if ( !isvalidName( f_name ) ) { RC1 = 20 ; }
			else                          { RC1 = 12 ; }
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
	row = row + win_row ;
	col = col + win_col ;
}


string pPanel::field_getvalue( const string& f_name )
{
	map<string, field *>::iterator it ;

	it = fieldList.find( f_name )   ;
	it->second->field_prep_input()  ;
	return  it->second->field_value ;
}


void pPanel::field_setvalue( const string& f_name, const string& f_value )
{
	errblock err ;
	bool snulls = ( p_poolMGR->get( err, "ZNULLS", SHARED ) == "YES" ) ;
	char pad    = p_poolMGR->get( err, "ZPADC", PROFILE ).front() ;

	map<string, field *>::iterator it ;

	it = fieldList.find( f_name ) ;
	it->second->field_value   = f_value ;
	it->second->field_changed = true    ;
	it->second->field_prep_display()    ;
	it->second->display_field( win, pad, snulls ) ;
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

	row = row - win_row ;
	col = col - win_col ;

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

	map<string, field *>::iterator it ;

	it = fieldList.find( fld ) ;
	if ( it == fieldList.end() ) { return false ; }

	if ( !it->second->field_active ) { return false ; }

	row = it->second->field_row + win_row ;
	col = it->second->field_col + win_col ;

	return true ;
}


fieldExc pPanel::field_getexec( const string& field )
{
	// If passed field is in the field execute table, return the structure fieldExc
	// for that field as defined in )FIELD panel section.

	if ( fieldExcTable.find( field ) == fieldExcTable.end() ) { return fieldExc() ; }
	return fieldExcTable[ field ] ;
}


void pPanel::field_clear( const string& f_name )
{
	errblock err ;
	char pad = p_poolMGR->get( err, "ZPADC", PROFILE ).front() ;

	fieldList[ f_name ]->field_clear( win, pad ) ;
}


void pPanel::field_edit( uint row, uint col, char ch, bool Isrt, bool& prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field

	errblock err ;
	map<string, field *>::iterator it;

	bool snulls = ( p_poolMGR->get( err, "ZNULLS", SHARED ) == "YES" ) ;
	char pad    = p_poolMGR->get( err, "ZPADC", PROFILE ).front() ;

	row   = row - win_row ;
	col   = col - win_col ;
	p_row = row ;
	p_col = col ;

	prot = true ;
	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
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
			if ( (p_col == it->second->field_cole) && (it->second->field_skip) )
			{
				field_tab_next( p_row, p_col ) ;
			}
			return ;
		}
	}
}


void pPanel::field_backspace( uint& row, uint& col, bool& prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field
	// Return physical position of the cursor in row/col

	uint trow ;
	uint tcol ;
	uint p    ;

	errblock err ;

	bool snulls = ( p_poolMGR->get( err, "ZNULLS", SHARED ) == "YES" ) ;
	char pad    = p_poolMGR->get( err, "ZPADC", PROFILE ).front() ;
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


void pPanel::field_delete_char( uint row, uint col, bool& prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field
	// Return physical position of the cursor in row/col

	uint trow ;
	uint tcol ;

	errblock err ;

	map<string, field *>::iterator it ;

	bool snulls = ( p_poolMGR->get( err, "ZNULLS", SHARED ) == "YES" ) ;
	char pad    = p_poolMGR->get( err, "ZPADC", PROFILE ).front() ;

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


void pPanel::field_erase_eof( uint row, uint col, bool& prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field

	errblock err ;

	map<string, field *>::iterator it;

	bool snulls = ( p_poolMGR->get( err, "ZNULLS", SHARED ) == "YES" ) ;
	char pad    = p_poolMGR->get( err, "ZPADC", PROFILE ).front() ;
	row = row - win_row ;
	col = col - win_col ;

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
		if ( it->second->field_dynArea && it->second->field_dynArea_ptr->dynArea_DataInsp )
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
		if ( it->second->field_dynArea && it->second->field_dynArea_ptr->dynArea_DataInsp )
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


string pPanel::get_field_help( const string& fld )
{
	if ( fieldHList.count( fld ) == 0 ) { return "" ; }
	return fieldHList[ fld ] ;
}


void pPanel::tb_set_linesChanged()
{
	//  Store changed lines for processing by the application if requested via tbdispl with no panel name
	//  Format is a list of line-number/URID pairs

	string URID  ;
	errblock err ;

	map<string, field *>::iterator it ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( it->second->field_tb && it->second->field_changed )
		{
			URID = p_funcPOOL->get( err, 0, ".ZURID."+ d2ds( it->second->field_row - tb_row ), NOCHECK ) ;
			tb_linesChanged[ it->second->field_row - tb_row ] = URID ;
		}
	}
	p_funcPOOL->put( err, "ZTDSELS", tb_linesChanged.size() ) ;
}


bool pPanel::tb_lineChanged( int& ln, string& URID )
{
	//  Retrieve the next changed line on the tbdispl.  Return screen line number and URID of the table record
	//  Don't remove the pair from the list but update ZTDSELS

	map<int, string>::iterator it ;

	errblock err ;

	ln   = 0  ;
	URID = "" ;

	if ( tb_linesChanged.size() == 0 ) { return false ; }

	it   = tb_linesChanged.begin() ;
	ln   = it->first  ;
	URID = it->second ;

	RC = 0 ;
	p_funcPOOL->put( err, "ZTDSELS", tb_linesChanged.size() ) ;
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


void pPanel::display_tb_mark_posn()
{
	int rows  ;
	int top   ;
	int size  ;

	string mark ;
	string posn ;

	errblock err ;

	rows = p_funcPOOL->get( err, 0, INTEGER, "ZTDROWS" ) ;
	top  = p_funcPOOL->get( err, 0, INTEGER, "ZTDTOP"  ) ;
	size = rows - top + 1 ;
	p_funcPOOL->put( err, "ZTDVROWS", size ) ;

	wattrset( win, WHITE ) ;
	if ( size < tb_depth )
	{
		mark = p_funcPOOL->get( err, 0, "ZTDMARK" ) ;
		mvwaddstr( win, tb_row + size, 0, mark.c_str() ) ;
		p_funcPOOL->put( err, "ZTDVROWS", size ) ;
	}
	else
	{
		p_funcPOOL->put( err, "ZTDVROWS", tb_depth ) ;
	}

	posn = "" ;
	if ( top <= rows )
	{
		posn = "Row "+ d2ds( top ) +" of "+ d2ds( rows ) ;
	}
	mvwaddstr( win, 2, WSCRMAXW - posn.length(), posn.c_str() ) ;
	wattroff( win, WHITE ) ;
}


void pPanel::tb_fields_active_inactive()
{
	int rows ;
	int top  ;
	int size ;
	int ws   ;
	int i    ;
	int j    ;

	string s ;

	errblock err ;

	rows = p_funcPOOL->get( err, 0, INTEGER, "ZTDROWS" ) ;
	top  = p_funcPOOL->get( err, 0, INTEGER, "ZTDTOP"  ) ;
	size = rows - top + 1 ;

	ws = words( tb_fields ) ;
	for ( i = 0 ; i < tb_depth ; i++ )
	{
		for ( j = 1 ; j <= ws ; j++ )
		{
			s = word( tb_fields, j ) +"."+ d2ds( i ) ;
			fieldList[ s ]->field_active = ( i < size ) ;
		}
	}
}



string pPanel::return_command( const string& opt )
{
	if ( commandTable.count( opt ) == 0 ) { return "" ; }
	return commandTable.at( opt ) ;
}


bool pPanel::display_pd( uint col )
{
	// col is the pysical position on the screen.  Correct by subtracting the window column position

	int i ;

	col = col - win_col ;
	hide_pd() ;
	for ( i = 0 ; i < ab.size() ; i++ )
	{
		if ( (col >= ab.at(i).abc_col) && (col < (ab.at(i).abc_col + ab.at(i).abc_name.size()) ) )
		{
			ab.at(i).display_abc_sel( win ) ;
			ab.at(i).display_pd( win_row, win_col ) ;
			pdActive = true ;
			abIndex  = i ;
			p_col    = ab.at(i).abc_col + 2 ;
			p_row    = 2 ;
			return true  ;
		}
	}
	return false ;
}


void pPanel::display_pd()
{
	if ( !pdActive ) { return ; }
	ab.at( abIndex ).display_pd( win_row, win_col ) ;
}


void pPanel::display_first_pd()
{
	if ( ab.size() == 0 ) { return ; }

	ab.at( 0 ).display_abc_sel( win ) ;
	ab.at( 0 ).display_pd( win_row, win_col ) ;
	pdActive = true ;
	abIndex  = 0    ;
	p_col    = ab.at( 0 ).abc_col + 2 ;
	p_row    = 2    ;
}


void pPanel::display_next_pd()
{
	if ( !pdActive ) { return ; }

	if ( ++abIndex == ab.size() ) { abIndex = 0 ; }
	ab.at( abIndex ).display_abc_sel( win ) ;
	ab.at( abIndex ).display_pd( win_row, win_col ) ;
	p_col = ab.at( abIndex ).abc_col + 2 ;
	p_row = 2 ;
}


void pPanel::hide_pd()
{
	if ( !pdActive ) { return ; }

	ab.at( abIndex ).hide_pd() ;
	ab.at( abIndex ).display_abc_unsel( win ) ;
}


pdc pPanel::retrieve_pdChoice( int row, int col )
{
	if ( !pdActive ) { return pdc() ; }

	ab.at( abIndex ).hide_pd() ;
	ab.at( abIndex ).display_abc_unsel( win ) ;
	pdActive = false ;
	return ab.at( abIndex ).retrieve_pdChoice( row-win_row, col-win_col ) ;
}


void pPanel::display_boxes()
{
	vector<Box *>::iterator it ;

	for ( it = boxes.begin() ; it != boxes.end() ; it++ )
	{
		(*it)->display_box( win ) ;
	}
}


void pPanel::set_panel_msg( slmsg t, const string& msgid )
{
	errblock err ;

	clear_msg()   ;
	MSG   = t     ;
	MSGID = msgid ;

	if ( MSG.smsg == "" && MSG.lmsg == "" )
	{
		MSGID = "" ;
		return     ;
	}
	if ( p_poolMGR->get( err, ZSCRNUM, "ZSHMSGID" ) == "Y" ) { MSG.lmsg = msgid +" "+ MSG.lmsg ; }
	showLMSG = ( MSG.smsg == "" || MSG.lmsg == "" ) ;
}


void pPanel::clear_msg()
{
	MSG.clear() ;
	showLMSG = false ;
	if ( smp_created )
	{
		panel_cleanup( smpanel ) ;
		del_panel( smpanel ) ;
		delwin( smwin )      ;
		smp_created = false  ;
	}
	if ( lmp_created )
	{
		panel_cleanup( lmpanel ) ;
		del_panel( lmpanel ) ;
		delwin( lmwin )      ;
		lmp_created = false  ;
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


void pPanel::display_msg()
{
	int i     ;
	int w_row ;
	int w_col ;
	int w_depth ;
	int w_width ;

	vector<string> v ;

	errblock err ;

	if ( MSGID == "" ) { return ; }

	msgResp = MSG.resp ;
	if ( MSGID != "PSYS012L" )
	{
		p_poolMGR->put( err, ZSCRNUM, "ZMSGID", MSGID ) ;
	}

	if ( MSG.smsg != "" )
	{
		if ( smp_created )
		{
			panel_cleanup( smpanel ) ;
			del_panel( smpanel ) ;
			delwin( smwin )      ;
		}
		if ( MSG.smwin )
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
			smwin = newwin( 1, MSG.smsg.size()+1, 1+win_row, (WSCRMAXW - MSG.smsg.size() - 1 + win_col) ) ;
			wattrset( smwin, cuaAttr[ MSG.type ] ) ;
			mvwaddstr( smwin, 0, 0, MSG.smsg.c_str() ) ;
		}
		smpanel = new_panel( smwin )  ;
		set_panel_userptr( smpanel, new panel_data( ZSCRNUM ) ) ;
		wattroff( smwin, cuaAttr[ MSG.type ] ) ;
		smp_created = true ;
		if ( MSG.alm )
		{
			beep() ;
			MSG.alm = false ;
		}
	}
	if ( showLMSG && MSG.lmsg != "" )
	{
		if ( lmp_created )
		{
			panel_cleanup( lmpanel ) ;
			del_panel( lmpanel ) ;
			delwin( lmwin )      ;
		}
		if ( MSG.lmwin || p_poolMGR->get( err, "ZLMSGW", PROFILE ) == "Y" )
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
		lmp_created = true ;
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

	it = fieldList.find( MSGLOC ) ;

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

	if ( it != fieldList.end() )
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


void pPanel::display_id()
{
	string scrname ;
	string panarea ;

	RC = 0 ;

	errblock err ;

	panarea = "" ;

	if ( idwin )
	{
		panel_cleanup( idpanel ) ;
		del_panel( idpanel ) ;
		delwin( idwin )      ;
		idwin = NULL         ;
	}

	if ( p_poolMGR->get( err, ZSCRNUM, "ZSHPANID" ) == "Y" )
	{
		panarea = PANELID + " " ;
	}

	if ( p_poolMGR->get( err, "ZSCRNAM1", SHARED ) == "ON" )
	{
		scrname = p_poolMGR->get( err, "ZSCRNAME", SHARED ) ;
		if ( scrname != "" )
		{
			panarea  = panarea + scrname + " " ;
		}
	}

	if ( panarea != "" )
	{
		idwin   = newwin( 1, panarea.size(), win_row+2, win_col+0 ) ;
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


void pPanel::update_keylist_vars()
{
	errblock err ;

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

	if ( smp_created && !panel_hidden( smpanel) )
	{
		getmaxyx( smwin, len, ht ) ;
		if ( ht > 1 )
		{
			getbegyx( smwin, w_col, w_row ) ;
			if ( ( r >= w_col && r < (w_col + len) )  &&
			     ( c >= w_row && c < (w_row + ht ) ) )
			{
				hide_panel( smpanel ) ;
				return true ;
			}
		}
	}
	if ( lmp_created && !panel_hidden( lmpanel) )
	{
		getmaxyx( lmwin, len, ht ) ;
		if ( ht > 1 )
		{
			getbegyx( lmwin, w_col, w_row ) ;
			if ( ( r >= w_col && r < (w_col + len) )  &&
			     ( c >= w_row && c < (w_row + ht ) ) )
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
