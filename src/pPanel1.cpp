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
	scrollOn    = false  ;
	pdActive    = false  ;
	msgResp     = false  ;
	nretriev    = false  ;
	nretfield   = ""     ;
	KEYLISTN    = ""     ;
	KEYAPPL     = ""     ;
	panelTitle  = ""     ;
	panelDescr  = ""     ;
	abIndex     = 0      ;
	opt_field   = 0      ;
	tb_model    = false  ;
	tb_depth    = 0      ;
	tb_fields   = ""     ;
	win_addpop  = false  ;
	smp_created = false  ;
	lmp_created = false  ;
	win_row     = 0      ;
	win_col     = 0      ;
	win_width   = 0      ;
	win_depth   = 0      ;
	dyn_depth   = 0      ;
	ZPHELP      = ""     ;
	PERR        = ""     ;
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
	if ( bwin != NULL )
	{
		panel_cleanup( bpanel ) ;
		del_panel( bpanel ) ;
		delwin( bwin )      ;
	}
	if ( fwin != NULL )
	{
		panel_cleanup( panel ) ;
		del_panel( panel )     ;
		delwin( fwin ) ;
	}
	if ( pwin != NULL )
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
	if ( idwin != NULL )
	{
		panel_cleanup( idpanel ) ;
		del_panel( idpanel ) ;
		delwin( idwin )      ;
	}
}


void pPanel::init( int & RC1 )
{
	ZSCRMAXD = ds2d( p_poolMGR->get( RC1, "ZSCRMAXD", SHARED ) ) ;
	if ( RC1 > 0 ) { PERR = "ZSCRMAXD poolMGR.get failed" ; log("C", PERR ) ; RC1 = 20 ; return ; }
	ZSCRMAXW = ds2d( p_poolMGR->get( RC1, "ZSCRMAXW", SHARED ) ) ;
	if ( RC1 > 0 ) { PERR = "ZSCRMAXW poolMGR.get failed" ; log("C", PERR ) ; RC1 = 20 ; return ; }
	ZSCRNUM  = ds2d(p_poolMGR->get( RC1, "ZSCRNUM", SHARED ) ) ;
	if ( RC1 > 0 ) { PERR = "ZSCRNUM poolMGR.get failed"  ; log("C", PERR ) ; RC1 = 20 ; return ; }

	WSCRMAXD = ZSCRMAXD ;
	WSCRMAXW = ZSCRMAXW ;
}


string pPanel::getDialogueVar( string var )
{
	// Return the value of a dialogue variable (always as a string so convert int->string if necessary)
	// Search order is:
	//   Function pool defined
	//   Function pool implicit
	//   SHARED pool
	//   PROFILE pool
	// Function pool variables of type int are converted to string

	dataType var_type ;

	if ( p_funcPOOL->ifexists( RC, var, NOCHECK ) )
	{
		var_type = p_funcPOOL->getType( RC, var, NOCHECK ) ;
		if ( RC == 0 )
		{
			switch ( var_type )
			{
			case INTEGER:
				return d2ds( p_funcPOOL->get( RC, 0, var_type, var ) ) ;
				break ;
			case STRING:
				return p_funcPOOL->get( RC, 0, var, NOCHECK ) ;
				break ;
			}
		}
		else
		{
			log( "E", "Non-zero return code received retrieving variable type for " << var << " from function pool" << endl ) ;
			RC = 20   ;
			return "" ;
		}
	}
	else
	{
		return p_poolMGR->get( RC, var ) ;
	}
	return "" ;
}


void pPanel::putDialogueVar( string var, string val )
{
	// Upate a dialogue variable where it resides in the standard search order.
	// If not found, create an implicit function pool variable

	if      ( p_funcPOOL->ifexists( RC, var ) ) { p_funcPOOL->put( RC, 0, var, val )   ; }
	else if ( p_poolMGR->ifexists( RC, var ) )  { p_poolMGR->put( RC, var, val, ASIS ) ; }
	else                                        { p_funcPOOL->put( RC, 0, var, val )   ; }
}


void pPanel::syncDialogueVar( string var )
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

	if ( !p_poolMGR->ifexists( RC, var ) )
	{
		p_funcPOOL->put( RC, 0, var, "" ) ;
	}
	else
	{
		p_funcPOOL->put( RC, 0, var, p_poolMGR->get( RC, var, ASIS ) ) ;
	}
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


void pPanel::display_panel( int & RC1 )
{
	// fwin - created unconditionally and is the full-screen window
	// pwin - pop-up window only created if the panel contains a WINDOW(w,d) statement
	// (associate ncurses panel 'panel' with whichever is active and set WSCRMAX?)

	// Use fwin if no pwin exists, or no ADDPOP() has been done (not exactly how real ISPF works)

	string winttl ;
	string panttl ;

	RC1 = 0 ;

	update_keylist_vars() ;

	if ( win_addpop && pwin != NULL )
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
		winttl = getDialogueVar( "ZWINTTL" ) ;
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
		if ( bwin != NULL )
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
		p_funcPOOL->put( RC1, 0, "ZTDSELS", 0 ) ;
		display_tb_mark_posn() ;
		tb_fields_active_inactive() ;
	}

	display_ab()       ;
	display_literals() ;
	update_field_values( RC1 ) ;

	display_fields() ;
	display_boxes()  ;
	hide_pd()        ;
	display_pd()     ;
	display_msg()    ;
	display_id()     ;
}


void pPanel::redraw_fields()
{
	// Re-draw all fields in a window (eg after a .SHOW/.HIDE nulls command)

	display_fields() ;
}


void pPanel::redisplay_panel()
{
	// Refresh the screen by re-drawing all the elements

	string panttl ;

	werase( win ) ;

	panttl = sub_vars( panelTitle ) ;
	wattrset( win, cuaAttr[ PT ] ) ;
	mvwaddstr( win, 2, ( WSCRMAXW - panttl.size() ) / 2, panttl.c_str() ) ;
	wattroff( win, cuaAttr[ PT ] ) ;

	if ( tb_model )
	{
		p_funcPOOL->put( RC, 0, "ZTDSELS", 0 ) ;
		display_tb_mark_posn() ;
		tb_fields_active_inactive() ;
	}

	display_ab()       ;
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


void pPanel::display_panel_update( int & RC1 )
{
	//  For all changed fields, remove the null character, apply attributes (upper case, left/right justified),
	//  and copy back to function pool
	//  If END entered with pull down displayed, remove the pull down and clear ZVERB
	//  If cursor on a point-and-shoot field (PS), set variable as in the )PNTS panel section if defined there
	//  When copying to the function pool, change value as follows:
	//      JUST(LEFT/RIGHT) leading and trailing spaces are removed
	//      JUST(ASIS) Only trailing spaces are removed

	//  For dynamic areas, also update the shadow variable to indicate character deletes (0xFF)

	//  Clear error message if END pressed so panel is not re-displaed

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

	RC1      = 0  ;
	fieldNum = 0  ;
	MSGID    = "" ;
	MSGLOC   = "" ;

	CMDVerb = p_poolMGR->get( RC1, "ZVERB", SHARED ) ;
	if ( CMDVerb == "END" && pdActive )
	{
		pdActive = false ;
		p_poolMGR->put( RC1, "ZVERB", "",  SHARED ) ;
	}

	if ( pdActive )
	{
		hide_pd()        ;
		pdActive = false ;
	}

	if ( scrollOn )
	{
		it = fieldList.find( "ZSCROLL" ) ;
		it->second->field_remove_nulls() ;
		it->second->field_value = upper( strip( it->second->field_value ) ) ;
		if ( !isnumeric( it->second->field_value ) )
		{
			switch( it->second->field_value[ 0 ] )
			{
				case 'C': it->second->field_value = "CSR " ; break ;
				case 'D': it->second->field_value = "DATA" ; break ;
				case 'H': it->second->field_value = "HALF" ; break ;
				case 'P': it->second->field_value = "PAGE" ; break ;
				default:  MSGID  = "PSYS011I" ;
					  CURFLD = "ZSCROLL"  ;
					  MSGLOC = CURFLD     ;
			}
		}
		p_funcPOOL->put( RC1, 0, it->first, it->second->field_value ) ;
		it->second->field_changed = false ;
	}

	p_funcPOOL->put( RC1, 0, "ZCURFLD", "" ) ;
	p_funcPOOL->put( RC1, 0, "ZCURPOS", 1  ) ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( it->second->field_changed )
		{
			it->second->field_remove_nulls() ;
			if ( !it->second->field_dynArea )
			{
				switch( it->second->field_just )
				{
					case 'L':
					case 'R': it->second->field_value = strip( it->second->field_value, 'B', ' ' ) ; break ;
					case 'A': it->second->field_value = strip( it->second->field_value, 'T', ' ' ) ; break ;
				}
				if ( it->second->field_caps )
				{
					it->second->field_value = upper( it->second->field_value ) ;
				}
				var_type = p_funcPOOL->getType( RC1, it->first ) ;
				if ( RC1 == 0 && var_type == INTEGER )
				{
					if ( datatype( it->second->field_value, 'W' ) )
					{
						p_funcPOOL->put( RC1, 0, it->first, ds2d( it->second->field_value ) ) ;
					}
					else
					{
						MSGID  = "PSYS011G" ;
						CURFLD = it->first  ;
						MSGLOC = CURFLD     ;
					}
				}
				else
				{
					p_funcPOOL->put( RC1, 0, it->first, it->second->field_value, NOCHECK ) ;
				}
				if ( RC1 > 0 ) continue ;
			}
			else
			{
				p        = it->first.find_first_of( '.' )  ;
				fieldNam = it->first.substr( 0, p )        ;
				fieldNum = ds2d( it->first.substr( p+1 ) ) ;
				darea    = p_funcPOOL->vlocate( RC1, 0, fieldNam ) ;
				offset   = fieldNum*it->second->field_length      ;
				if ( it->second->field_dynArea_ptr->dynArea_DataModsp )
				{
					it->second->field_DataMod_to_UserMod( darea, offset ) ;
				}
				darea->replace( offset, it->second->field_length, it->second->field_value ) ;
				sname    = it->second->field_dynArea_ptr->dynArea_shadow_name ;
				shadow   = p_funcPOOL->vlocate( RC1, 0, sname )   ;
				shadow->replace( offset, it->second->field_length, it->second->field_shadow_value ) ;
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
				p_funcPOOL->put( RC1, 0, "ZCURFLD", fieldNam ) ;
				p_funcPOOL->put( RC1, 0, "ZCURPOS", ( fieldNum*it->second->field_length + curpos ) ) ;
				fieldNum++ ;
			}
			else
			{
				p_funcPOOL->put( RC1, 0, "ZCURFLD", it->first ) ;
				p_funcPOOL->put( RC1, 0, "ZCURPOS", curpos )    ;
			}
		}
	}

	p_poolMGR->put( RC1, "ZSCROLLA", "",  SHARED ) ;
	p_poolMGR->put( RC1, "ZSCROLLN", "0", SHARED ) ;

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
			p_funcPOOL->put( RC1, 0, CMDfield, "" )  ;
		}
		if      ( tb_model )                          { scrollAmt = tb_depth  ; }
		else if ( findword( CMDVerb, "LEFT RIGHT" ) ) { scrollAmt = dyn_width ; }
		else                                          { scrollAmt = dyn_depth ; }
		if ( isnumeric( CMD ) )
		{
			if ( CMD.size() > 6 )
			{
				MSGID  = "PSYS011I" ;
				CURFLD = msgfld     ;
				MSGLOC = CURFLD     ;
			}
			p_poolMGR->put( RC1, "ZSCROLLA", CMD, SHARED ) ;
			p_poolMGR->put( RC1, "ZSCROLLN", CMD, SHARED ) ;
		}
		else if ( CMD[ 0 ] == 'M' )
		{
			p_poolMGR->put( RC1, "ZSCROLLA", "MAX", SHARED ) ;
		}
		else if ( CMD[ 0 ] == 'C' )
		{
			if ( fieldNum == 0 )
			{
				p_poolMGR->put( RC1, "ZSCROLLN", d2ds( scrollAmt ),  SHARED ) ;
			}
			else if ( CMDVerb[ 0 ] == 'U' )
			{
				p_poolMGR->put( RC1, "ZSCROLLN", d2ds( dyn_depth-fieldNum ), SHARED ) ;
			}
			else if ( CMDVerb[ 0 ] == 'D' )
			{
				p_poolMGR->put( RC1, "ZSCROLLN", d2ds( fieldNum-1 ), SHARED ) ;
			}
			else if ( CMDVerb[ 0 ] == 'L' )
			{
				p_poolMGR->put( RC1, "ZSCROLLN", d2ds( dyn_width-curpos ), SHARED ) ;
			}
			else
			{
				p_poolMGR->put( RC1, "ZSCROLLN", d2ds( curpos ), SHARED ) ;
			}
			p_poolMGR->put( RC1, "ZSCROLLA", "CSR", SHARED ) ;
		}
		else if ( CMD[ 0 ] == 'D' )
		{
			p_poolMGR->put( RC1, "ZSCROLLN", d2ds( scrollAmt ), SHARED ) ;
			p_poolMGR->put( RC1, "ZSCROLLA", "DATA", SHARED ) ;
		}
		else if ( CMD[ 0 ] == 'H' )
		{
			p_poolMGR->put( RC1, "ZSCROLLN", d2ds( scrollAmt/2 ), SHARED ) ;
			p_poolMGR->put( RC1, "ZSCROLLA", "HALF", SHARED ) ;
		}
		else
		{
			p_poolMGR->put( RC1, "ZSCROLLN", d2ds( scrollAmt ), SHARED ) ;
			p_poolMGR->put( RC1, "ZSCROLLA", "PAGE", SHARED ) ;
		}
	}
	if ( findword( CMDVerb, "END EXIT RETURN" ) ) { MSGID = "" ; }
}



void pPanel::display_panel_init( int & RC1 )
{
	// Perform panel )INIT processing

	// Set RC1=8 from VGET/VPUT to 0 as it isn't an error in this case
	// For table display fields, .ATTR applies to all fields but .CURSOR applies only to the top field, .0

	int i  ;
	int j  ;
	int ws ;
	int i_vputget ;
	int i_assign  ;

	string vars ;
	string var  ;
	string val  ;
	string t    ;
	string fieldNam ;

	map<string, field *>::iterator it;

	i_vputget = 0  ;
	i_assign  = 0  ;

	RC1 = 0 ;
	for ( i = 0 ; i < initstmnts.size() ; i++ )
	{
		if ( initstmnts.at( i ).ps_assign )
		{
			if ( assgnListi.at( i_assign ).as_isvar )
			{
				if ( assgnListi.at( i_assign ).as_rhs == "Z" )
				{
					t = "" ;
				}
				else
				{
					t = getDialogueVar( assgnListi.at( i_assign ).as_rhs ) ;
					if ( assgnListi.at( i_assign ).as_retlen )       { t = d2ds( t.size() )   ; }
					else if ( assgnListi.at( i_assign ).as_upper )   { t = upper( t )         ; }
					else if ( assgnListi.at( i_assign ).as_words )   { t = d2ds( words( t ) ) ; }
					else if ( assgnListi.at( i_assign ).as_chkexst ) { t = exists( t ) ? "1" : "0"          ; }
					else if ( assgnListi.at( i_assign ).as_chkfile ) { t = is_regular_file( t ) ? "1" : "0" ; }
					else if ( assgnListi.at( i_assign ).as_chkdir  ) { t = is_directory( t )    ? "1" : "0" ; }

				}
			}
			else if ( assgnListi.at( i_assign ).as_rhs == ".HELP" )
			{
				t = ZPHELP ;
			}
			else if ( assgnListi.at( i_assign ).as_rhs == ".MSG" )
			{
				t = MSGID ;
			}
			else if ( assgnListi.at( i_assign ).as_rhs == ".CURSOR" )
			{
				t = p_funcPOOL->get( RC1, 0, "ZCURFLD" ) ;
			}
			else
			{
				t = assgnListi.at( i_assign ).as_rhs ;
			}
			if ( assgnListi.at( i_assign ).as_lhs == ".AUTOSEL" )
			{
				if ( t == "" ) { t = "YES" ; }
				if ( t != "YES" && t != "NO" ) { RC1 = 20 ; PERR = "Invalid .AUTOSEL value.  Must be YES, NO or blank" ; return ; }
				p_funcPOOL->put( RC1, 0, ".AUTOSEL", t, NOCHECK ) ;
			}
			else if ( assgnListi.at( i_assign ).as_lhs == ".HELP" )
			{
				ZPHELP = t ;
			}
			else if ( assgnListi.at( i_assign ).as_lhs == ".NRET" )
			{
				if      ( t == "ON" )        { nretriev  = true  ; }
				else if ( isvalidName( t ) ) { nretfield = t     ; }
				else                         { nretriev  = false ; }
			}
			else if ( assgnListi.at( i_assign ).as_lhs == ".MSG" )
			{
				MSGID = t ;
			}
			else if ( assgnListi.at( i_assign ).as_lhs == ".CSRROW" )
			{
				if ( !isnumeric( t ) ) { RC1 = 20 ; PERR = "Invalid .CSRROW value.  Must be numeric" ; return ; }
				p_funcPOOL->put( RC1, 0, ".CSRROW", t, NOCHECK ) ;
			}
			else if ( assgnListi.at( i_assign ).as_lhs == ".CURSOR" )
			{
				if ( wordpos( assgnListi.at( i_assign ).as_rhs, tb_fields ) > 0 ) { t = t + ".0" ; }
				CURFLD = t ;
			}
			else if ( assgnListi.at( i_assign ).as_isattr )
			{
				fieldNam = assgnListi.at( i_assign ).as_lhs ;
				if ( assgnListi.at( i_assign ).as_istb )
				{
					for ( j = 0 ; j < tb_depth ; j++ )
					{
						if ( fieldList[ fieldNam +"."+ d2ds( j ) ]->field_attr( t ) > 0 )
						{
							PERR = "Error setting attribute "+ t +" for table display field " +fieldNam ;
							RC1  = 20 ;
							return    ;
						}
					}
				}
				else
				{
					if ( fieldList[ fieldNam ]->field_attr( t ) > 0 )
					{
						PERR = "Error setting attribute "+ t +" for field "+ fieldNam ;
						RC1  = 20 ;
						return    ;
					}
				}
			}
			else
			{
				putDialogueVar( assgnListi.at( i_assign ).as_lhs, t ) ;
				if ( fieldList.find( assgnListi.at( i_assign ).as_lhs ) != fieldList.end() )
				{
					it = fieldList.find( assgnListi.at( i_assign ).as_lhs ) ;
					it->second->field_value = t ;
				}
			}
			i_assign++ ;
		}
		else if ( initstmnts.at( i ).ps_vputget )
		{
			vars = vpgListi.at( i_vputget ).vpg_vars ;
			ws   = words( vars ) ;
			if ( vpgListi.at( i_vputget ).vpg_vget )
			{
				for ( j = 1 ; j <= ws ; j++ )
				{
					var = word( vars, j ) ;
					val = p_poolMGR->get( RC1, var, vpgListi.at( i_vputget ).vpg_pool ) ;
					if ( RC1 == 0 )
					{
						p_funcPOOL->put( RC1, 0, var, val ) ;
					}
				}
			}
			if ( vpgListi.at( i_vputget ).vpg_vput )
			{
				for ( j = 1 ; j <= ws ; j++ )
				{
					var = word( vars, j ) ;
					val = p_funcPOOL->get( RC1, 8, var ) ;
					if ( RC1 == 0 )
					{
						p_poolMGR->put( RC1, var, val, vpgListi.at( i_vputget ).vpg_pool ) ;
					}
				}
			}
			if ( RC1 == 8 ) { RC1 = 0 ; }
			i_vputget++ ;
		}
	}
}


void pPanel::display_panel_reinit( int & RC1, int ln )
{
	// Perform panel )REINIT processing

	// Set RC1=8 from VGET/VPUT to 0 as it isn't an error in this case
	// For table display fields, .ATTR and .CURSOR apply to fields on line ln

	int i   ;
	int j   ;
	int ws  ;
	int i_vputget ;
	int i_assign  ;

	RC1 = 0 ;

	map<string, field *>::iterator it ;

	i_vputget = 0 ;
	i_assign  = 0 ;

	string t   ;
	string val ;
	string var ;
	string vars ;
	string fieldNam ;

	for ( i = 0 ; i < reinstmnts.size() ; i++ )
	{
		if ( reinstmnts.at( i ).ps_assign )
		{
			if ( assgnListr.at( i_assign ).as_isvar )
			{
				if ( assgnListr.at( i_assign ).as_rhs == "Z" )
				{
					t = "" ;
				}
				else
				{
					t = getDialogueVar( assgnListr.at( i_assign ).as_rhs ) ;
					if ( assgnListr.at( i_assign ).as_retlen )       { t = d2ds( t.size() )   ; }
					else if ( assgnListr.at( i_assign ).as_upper )   { t = upper( t )         ; }
					else if ( assgnListr.at( i_assign ).as_words )   { t = d2ds( words( t ) ) ; }
					else if ( assgnListr.at( i_assign ).as_chkexst ) { t = exists( t ) ? "1" : "0"          ; }
					else if ( assgnListr.at( i_assign ).as_chkfile ) { t = is_regular_file( t ) ? "1" : "0" ; }
					else if ( assgnListr.at( i_assign ).as_chkdir  ) { t = is_directory( t )    ? "1" : "0" ; }
				}
			}
			else if ( assgnListr.at( i_assign ).as_rhs == ".HELP" )
			{
				t = ZPHELP ;
			}
			else if ( assgnListr.at( i_assign ).as_rhs == ".MSG" )
			{
				t = MSGID ;
			}
			else if ( assgnListr.at( i_assign ).as_lhs == ".NRET" )
			{
				if      ( t == "ON" )        { nretriev  = true  ; }
				else if ( isvalidName( t ) ) { nretfield = t     ; }
				else                         { nretriev  = false ; }
			}
			else if ( assgnListr.at( i_assign ).as_rhs == ".CURSOR" )
			{
				t = p_funcPOOL->get( RC1, 0, "ZCURFLD" ) ;
			}
			else
			{
				t = assgnListr.at( i_assign ).as_rhs ;
			}
			if ( assgnListr.at( i_assign ).as_lhs == ".AUTOSEL" )
			{
				if ( t == "" ) { t = "YES" ; }
				if ( t != "YES" && t != "NO" ) { RC1 = 20 ; PERR = "Invalid .AUTOSEL value.  Must be YES, NO or blank" ; return ; }
				p_funcPOOL->put( RC1, 0, ".AUTOSEL", t, NOCHECK ) ;
			}
			else if ( assgnListr.at( i_assign ).as_lhs == ".HELP" )
			{
				ZPHELP = t ;
			}
			else if ( assgnListr.at( i_assign ).as_lhs == ".MSG" )
			{
				MSGID  = t ;
			}
			else if ( assgnListr.at( i_assign ).as_lhs == ".CSRROW" )
			{
				if ( !isnumeric( t ) ) { RC1 = 20 ; PERR = "Invalid .CSRROW value.  Must be numeric" ; return ; }
				p_funcPOOL->put( RC1, 0, ".CSRROW", t, NOCHECK ) ;
			}
			else if ( assgnListr.at( i_assign ).as_lhs == ".CURSOR" )
			{
				if ( wordpos( assgnListr.at( i_assign ).as_rhs, tb_fields ) > 0 ) { t = t + "."+ d2ds( ln ) ; }
				CURFLD = t ;
			}
			else if ( assgnListr.at( i_assign ).as_isattr )
			{
				fieldNam = assgnListr.at( i_assign ).as_lhs ;
				if ( assgnListr.at( i_assign ).as_istb )
				{
					if ( fieldList[ fieldNam +"."+ d2ds( ln ) ]->field_attr( t ) > 0 )
					{
						PERR = "Error setting attribute "+ t +" for table display field " +fieldNam ;
						RC1  = 20 ;
						return    ;
					}
					attrList.push_back( fieldNam +"."+ d2ds( ln ) ) ;
				}
				else
				{
					if ( fieldList[ fieldNam ]->field_attr( t ) > 0 )
					{
						PERR = "Error setting attribute "+ t +" for field "+ fieldNam ;
						RC1  = 20 ;
						return    ;
					}
					attrList.push_back( fieldNam ) ;
				}
			}
			else
			{
				putDialogueVar( assgnListr.at( i_assign ).as_lhs, t ) ;
				if ( fieldList.find( assgnListr.at( i_assign ).as_lhs ) != fieldList.end() )
				{
					it = fieldList.find( assgnListr.at( i_assign ).as_lhs ) ;
					it->second->field_value = t ;
				}
			}
			i_assign++ ;
		}
		else if ( reinstmnts.at( i ).ps_vputget )
		{
			vars = vpgListr.at( i_vputget ).vpg_vars ;
			ws   = words( vars ) ;
			if ( vpgListr.at( i_vputget ).vpg_vget )
			{
				for ( j = 1 ; j <= ws ; j++ )
				{
					var = word( vars, j ) ;
					val = p_poolMGR->get( RC1, var, vpgListr.at( i_vputget ).vpg_pool ) ;
					if ( RC1 == 0 )
					{
						p_funcPOOL->put( RC1, 0, var, val ) ;
					}
				}
			}
			if ( vpgListr.at( i_vputget ).vpg_vput )
			{
				for ( j = 1 ; j <= ws ; j++ )
				{
					var = word( vars, j ) ;
					val = p_funcPOOL->get( RC1, 8, var ) ;
					if ( RC1 == 0 )
					{
						p_poolMGR->put( RC1, var, val, vpgListr.at( i_vputget ).vpg_pool ) ;
					}
				}
			}
			if ( RC1 == 8 ) { RC1 = 0 ; }
			i_vputget++ ;
		}
	}
}


void pPanel::display_panel_proc( int & RC1, int ln )
{
	//  Process )PROC statements

	//  Set RC1=8 from VGET/VPUT to 0 as it isn't an error in this case
	//  For table display fields, .ATTR and .CURSOR apply to fields on line ln
	//  If cursor on a point-and-shoot field (PS), set variable as in the )PNTS panel section if defined there

	int i, j, ws    ;
	int k           ;
	int p           ;
	int sz          ;
	int i_if        ;
	int i_trunc     ;
	int i_trans     ;
	int i_vputget   ;
	int i_verify    ;
	int i_assign    ;
	int if_column   ;

	string wd       ;
	string l        ;
	string t        ;
	string val      ;
	string vars     ;
	string var      ;
	string dTRAIL   ;
	string fieldNam ;

	bool found      ;
	bool if_skip    ;

	map<string, field *>::iterator it;

	RC1       = 0  ;
	i_if      = 0  ;
	i_trunc   = 0  ;
	i_trans   = 0  ;
	i_vputget = 0  ;
	i_verify  = 0  ;
	i_assign  = 0  ;
	i_if      = 0  ;
	dTRAIL    = "" ;
	if_column = 0  ;
	if_skip   = false ;

	for ( i = 0 ; i < procstmnts.size() ; i++ )
	{
		if ( if_skip )
		{
			if ( if_column < procstmnts.at( i ).ps_column )
			{
				if      ( procstmnts.at( i ).ps_if      ) { i_if++      ; }
				else if ( procstmnts.at( i ).ps_else    ) { i_if++      ; }
				else if ( procstmnts.at( i ).ps_assign  ) { i_assign++  ; }
				else if ( procstmnts.at( i ).ps_verify  ) { i_verify++  ; }
				else if ( procstmnts.at( i ).ps_vputget ) { i_vputget++ ; }
				else if ( procstmnts.at( i ).ps_trunc   ) { i_trunc++   ; }
				else if ( procstmnts.at( i ).ps_trans   ) { i_trans++   ; }
				continue ;
			}
			else { if_skip = false ; }
		}
		if ( procstmnts.at( i ).ps_exit ) { return ; }
		else if ( procstmnts.at( i ).ps_if )
		{
			if ( ifList.at( i_if ).if_eq ) { ifList.at( i_if ).if_true = false ; }
			if ( ifList.at( i_if ).if_ne ) { ifList.at( i_if ).if_true = true  ; }
			if ( ifList.at( i_if ).if_lhs == ".CURSOR" )
			{
				val = p_funcPOOL->get( RC1, 0, "ZCURFLD" ) ;
			}
			else if ( ifList.at( i_if ).if_lhs == ".MSG" )
			{
				val = MSGID ;
			}
			else
			{
				val = getDialogueVar( ifList.at( i_if ).if_lhs ) ;
			}
			for ( j = 0 ; j < ifList.at( i_if ).if_rhs.size() ; j++ )
			{
				if ( ifList.at( i_if ).if_isvar[ j ] )
				{
					if ( ifList.at( i_if ).if_rhs[ j ] == "Z" )
					{
						t = "" ;
					}
					else
					{
						t = getDialogueVar( ifList.at( i_if ).if_rhs[ j ] ) ;
					}
				}
				else
				{
					t = ifList.at( i_if ).if_rhs[ j ] ;
				}
				if ( ifList.at( i_if ).if_eq )
				{
					ifList.at( i_if ).if_true = ifList.at( i_if ).if_true || ( val == t ) ;
					if ( ifList.at( i_if ).if_true ) { break ; }
				}
				else if ( ifList.at( i_if ).if_ne )
				{
					ifList.at( i_if ).if_true = ifList.at( i_if ).if_true && ( val != t ) ;
					if ( !ifList.at( i_if ).if_true ) { break ; }
				}
				else
				{
					if ( (ifList.at( i_if ).if_gt && ( val >  t )) ||
					     (ifList.at( i_if ).if_lt && ( val <  t )) ||
					     (ifList.at( i_if ).if_ge && ( val >= t )) ||
					     (ifList.at( i_if ).if_le && ( val <= t )) ||
					     (ifList.at( i_if ).if_ng && ( val <= t )) ||
					     (ifList.at( i_if ).if_nl && ( val >= t )) )
					{
						ifList.at( i_if ).if_true = true ;

					}
					else
					{
						ifList.at( i_if ).if_true = false ;
					}
				}
			}
			if ( ifList.at( i_if ).if_true )
			{
				if_skip = false ;
			}
			else
			{
				if_skip   = true ;
				if_column = procstmnts.at( i ).ps_column ;
			}
			i_if++ ;
		}
		else if ( procstmnts.at( i ).ps_else )
		{
			if ( ifList.at( ifList.at( i_if ).if_stmnt ).if_true )
			{
				if_skip = true ;
				if_column = procstmnts.at( i ).ps_column ;
			}
			else
			{
				if_skip = false ;
			}
			i_if++ ;
		}
		else if ( procstmnts.at( i ).ps_trunc )
		{
			t = getDialogueVar( truncList.at( i_trunc ).trnc_field2 ) ;
			if ( truncList.at( i_trunc ).trnc_len > 0 )
			{
				p = truncList.at( i_trunc ).trnc_len ;
				if ( p > t.size() ) { dTRAIL = "" ; }
				else
				{
					dTRAIL = strip( t.substr( p, t.size() - p ) ) ;
					t      = t.substr( 0, p ) ;
				}
			}
			else
			{
				p = t.find( truncList.at( i_trunc ).trnc_char ) ;
				if ( p == string::npos ) { dTRAIL = "" ; }
				else
				{
					dTRAIL = strip( t.substr( p+1, t.size() - p - 1 ) ) ;
					t      = t.substr( 0, p ) ;
				}
			}
			putDialogueVar( truncList.at( i_trunc ).trnc_field1, t ) ;
			if ( fieldList.find( truncList.at( i_trunc ).trnc_field1 ) != fieldList.end() )
			{
				it = fieldList.find( truncList.at( i_trunc ).trnc_field1 ) ;
				it->second->field_value = t ;
			}
			i_trunc++ ;
		}
		else if ( procstmnts.at( i ).ps_trans )
		{
			t = getDialogueVar( transList.at( i_trans ).trns_field2 ) ;
			if ( transList.at( i_trans ).tlst.find( t ) != transList.at( i_trans ).tlst.end() )
			{
				t = transList.at( i_trans ).tlst[ t ] ;
			}
			else
			{
				if ( transList.at( i_trans ).tlst.find( "*" ) != transList.at( i_trans ).tlst.end() )
				{
					if ( transList.at( i_trans ).tlst[ "*" ] != "*" )
					{
						t = transList.at( i_trans ).tlst[ "*" ] ;
					}
				}
				else { t = "" ; }
			}
			putDialogueVar( transList.at( i_trans ).trns_field1, t ) ;
			if ( fieldList.find( transList.at( i_trans ).trns_field1 ) != fieldList.end() )
			{
				it = fieldList.find( transList.at( i_trans ).trns_field1 ) ;
				it->second->field_value = t ;
			}
			i_trans++ ;
		}
		else if ( procstmnts.at( i ).ps_vputget )
		{
			vars = vpgList.at( i_vputget ).vpg_vars ;
			ws   = words( vars ) ;
			if ( vpgList.at( i_vputget ).vpg_vget )
			{
				for ( j = 1 ; j <= ws ; j++ )
				{
					var = word( vars, j ) ;
					val = p_poolMGR->get( RC1, var, vpgList.at( i_vputget ).vpg_pool ) ;
					if ( RC1 == 0 )
					{
						p_funcPOOL->put( RC1, 0, var, val ) ;
					}
				}
			}
			if ( vpgList.at( i_vputget ).vpg_vput )
			{
				for ( j = 1 ; j <= ws ; j++ )
				{
					var = word( vars, j ) ;
					val = p_funcPOOL->get( RC1, 8, var ) ;
					if ( RC1 == 0 )
					{
						p_poolMGR->put( RC1, var, val, vpgList.at( i_vputget ).vpg_pool ) ;
					}
				}
			}
			if ( RC1 == 8 ) { RC1 = 0 ; }
			i_vputget++ ;
		}
		else if ( procstmnts.at( i ).ps_assign )
		{
			if ( assgnList.at( i_assign ).as_isvar )
			{
				if ( assgnList.at( i_assign ).as_rhs == "Z" )
				{
					t = "" ;
				}
				else
				{
					t = getDialogueVar( assgnList.at( i_assign ).as_rhs ) ;
					if      ( assgnList.at( i_assign ).as_retlen  ) { t = d2ds( t.size() )   ; }
					else if ( assgnList.at( i_assign ).as_upper   ) { t = upper( t )         ; }
					else if ( assgnList.at( i_assign ).as_words   ) { t = d2ds( words( t ) ) ; }
					else if ( assgnList.at( i_assign ).as_chkexst ) { t = exists( t ) ? "1" : "0"          ; }
					else if ( assgnList.at( i_assign ).as_chkfile ) { t = is_regular_file( t ) ? "1" : "0" ; }
					else if ( assgnList.at( i_assign ).as_chkdir  ) { t = is_directory( t )    ? "1" : "0" ; }
				}
			}
			else if ( assgnList.at( i_assign ).as_rhs == ".CURSOR" )
			{
				t = p_funcPOOL->get( RC1, 0, "ZCURFLD" ) ;
			}
			else if ( assgnList.at( i_assign ).as_rhs == ".HELP" )
			{
				t = ZPHELP ;
			}
			else if ( assgnList.at( i_assign ).as_rhs == ".MSG" )
			{
				t = MSGID ;
			}
			else if ( assgnList.at( i_assign ).as_rhs == ".TRAIL" )
			{
				t = dTRAIL ;
			}
			else
			{
				t = assgnList.at( i_assign ).as_rhs ;
			}
			if ( assgnList.at( i_assign ).as_lhs == ".AUTOSEL" )
			{
			}
			else if ( assgnList.at( i_assign ).as_lhs == ".CURSOR" )
			{
				p_funcPOOL->put( RC1, 0, "ZCURFLD", t ) ;
				if ( wordpos( assgnList.at( i_assign ).as_rhs, tb_fields ) > 0 ) { t = t +"."+ d2ds( ln ) ; }
				CURFLD = t ;
			}
			else if ( assgnList.at( i_assign ).as_lhs == ".HELP" )
			{
				ZPHELP = t ;
			}
			else if ( assgnList.at( i_assign ).as_lhs == ".NRET" )
			{
				if      ( t == "ON" )        { nretriev  = true  ; }
				else if ( isvalidName( t ) ) { nretfield = t     ; }
				else                         { nretriev  = false ; }
			}
			else if ( assgnList.at( i_assign ).as_lhs == ".MSG" )
			{
				MSGID = t ;
			}
			else if ( assgnList.at( i_assign ).as_lhs == ".CSRROW" )
			{
				if ( !isnumeric( t ) ) { RC1 = 20 ; PERR = "Invalid .CSRROW value.  Must be numeric" ; return ; }
				p_funcPOOL->put( RC1, 0, ".CSRROW", t, NOCHECK ) ;
			}
			else if ( assgnList.at( i_assign ).as_isattr )
			{
				fieldNam = assgnList.at( i_assign ).as_lhs ;
				if ( assgnList.at( i_assign ).as_istb )
				{
					if ( fieldList[ fieldNam +"."+ d2ds( ln ) ]->field_attr( t ) > 0 )
					{
						PERR = "Error setting attribute "+ t +" for table display field " +fieldNam ;
						RC1  = 20 ;
						return    ;
					}
					attrList.push_back( fieldNam +"."+ d2ds( ln ) ) ;
				}
				else
				{
					if ( fieldList[ fieldNam ]->field_attr( t ) > 0 )
					{
						PERR = "Error setting attribute "+ t +" for field "+ fieldNam ;
						RC1  = 20 ;
						return    ;
					}
					attrList.push_back( fieldNam ) ;
				}
			}
			else
			{
				putDialogueVar( assgnList.at( i_assign ).as_lhs, t ) ;
				if ( fieldList.find( assgnList.at( i_assign ).as_lhs ) != fieldList.end() )
				{
					it = fieldList.find( assgnList.at( i_assign ).as_lhs ) ;
					it->second->field_value = t ;
				}
			}
			i_assign++ ;
		}
		else if ( procstmnts.at( i ).ps_verify )
		{
			fieldNam = verList.at( i_verify ).ver_field ;
			if ( verList.at( i_verify ).ver_tbfield ) { fieldNam = fieldNam +"."+ d2ds( ln ) ; }
			it = fieldList.find( fieldNam ) ;
			if ( verList.at( i_verify ).ver_nblank )
			{
				if ( it->second->field_value == "" )
				{
					MSGID  = verList.at( i_verify ).ver_msgid  ;
					if (MSGID == "" ) { MSGID = "PSYS019" ; }
					CURFLD = fieldNam ;
					MSGLOC = CURFLD   ;
					return            ;
				}
			}
			if ( it->second->field_value == "" )
			{
				i_verify++ ;
				continue   ;
			}
			if ( verList.at( i_verify ).ver_numeric )
			{
				if ( !isnumeric( it->second->field_value ) )
				{
					MSGID  = verList.at( i_verify ).ver_msgid  ;
					if (MSGID == "" ) { MSGID = "PSYS011A" ; }
					CURFLD = fieldNam ;
					MSGLOC = CURFLD   ;
					return            ;
				}
			}
			else if ( verList.at( i_verify ).ver_list )
			{
				found = false ;
				p     = verList.at( i_verify ).ver_value.find( "::" ) ;
				if ( p == string::npos )
				{
					if ( wordpos( it->second->field_value, verList.at( i_verify ).ver_value ) > 0 ) { found = true ; }
				}
				else
				{
					ws = words( verList.at( i_verify ).ver_value ) ;
					for ( k = 1 ; k <= ws ; k++ )
					{
						wd = word( verList.at( i_verify ).ver_value, k ) ;
						sz = wd.size()       ;
						p  = wd.find( "::" ) ;
						if ( p == string::npos )
						{
							if ( it->second->field_value == verList.at( i_verify ).ver_value ) { found = true ; break ; }
						}
						else
						{
							l  = wd.substr( p+2, sz-p-2 ) ;
							wd = wd.substr( 0 , p ) ;
							if ( l == "" ) { break ; }
							if ( abbrev( wd, it->second->field_value, ds2d( l ) ) )
							{
								it->second->field_value = wd ;
								p_funcPOOL->put( RC1, 0, it->first, it->second->field_value, NOCHECK ) ;
								found = true ;
								break ;
							}
						}
					}
				}
				if ( !found )
				{
					MSGID  = verList.at( i_verify ).ver_msgid  ;
					if (MSGID == "" ) { MSGID = "PSYS011B" ; }
					CURFLD = fieldNam ;
					MSGLOC = CURFLD   ;
					return            ;
				}
			}
			else if ( verList.at( i_verify ).ver_pict )
			{
				if ( !ispict( it->second->field_value, verList.at( i_verify ).ver_value ) )
				{
					p_poolMGR->put( RC1, "ZZSTR1", d2ds( verList.at( i_verify ).ver_value.size() ), SHARED ) ;
					p_poolMGR->put( RC1, "ZZSTR2", verList.at( i_verify ).ver_value, SHARED ) ;
					MSGID  = verList.at( i_verify ).ver_msgid  ;
					if (MSGID == "" ) { MSGID = "PSYS011N" ; }
					CURFLD = fieldNam ;
					MSGLOC = CURFLD   ;
					return            ;
				}
			}
			else if ( verList.at( i_verify ).ver_hex )
			{
				if ( !ishex( it->second->field_value ) )
				{
					MSGID  = verList.at( i_verify ).ver_msgid  ;
					if (MSGID == "" ) { MSGID = "PSYS011H" ; }
					CURFLD = fieldNam ;
					MSGLOC = CURFLD   ;
					return            ;
				}
			}
			else if ( verList.at( i_verify ).ver_octal )
			{
				if ( !isoctal( it->second->field_value ) )
				{
					MSGID  = verList.at( i_verify ).ver_msgid  ;
					if (MSGID == "" ) { MSGID = "PSYS011F" ; }
					CURFLD = fieldNam ;
					MSGLOC = CURFLD   ;
					return            ;
				}
			}
			i_verify++ ;
		}
	}

	fieldNam = p_funcPOOL->get( RC1, 0, "ZCURFLD" ) ;
	if ( fieldNam != "" )
	{
		it = fieldList.find( fieldNam ) ;
		if ( it != fieldList.end() )
		{
			if ( it->second->field_cua == PS )
			{
				if ( pntsTable.count( fieldNam ) > 0 )
				{
					p_funcPOOL->put( RC1, 0, pntsTable[ fieldNam ].pnts_var, pntsTable[ fieldNam ].pnts_val ) ;
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
				p_funcPOOL->put( RC1, 0, pntsTable[ fieldNam ].pnts_var, pntsTable[ fieldNam ].pnts_val ) ;
			}
			break ;
		}
	}
}


void pPanel::create_tbfield( int col, int length, cuaType cuaFT, string name, string opts )
{
	// Default is JUST(ASIS) for fields of a TB model, so change from the default of JUST(LEFT)

	field * m_fld ;

	RC = 0 ;
	if ( !isvalidName( name ) )
	{
		RC = 20 ;
		PERR = "Invalid field name "+ name +" entered for TB field" ;
		return  ;
	}

	tb_fields = tb_fields +" "+ name ;

	for ( int i = 0 ; i < tb_depth ; i++ )
	{
		m_fld               = new field  ;
		m_fld->field_cua    = cuaFT      ;
		m_fld->field_prot   = cuaAttrProt [ cuaFT ] ;
		m_fld->field_row    = tb_row + i ;
		m_fld->field_col    = col - 1    ;
		m_fld->field_length = length     ;
		m_fld->field_cole   = col - 1 + length ;
		m_fld->field_just   = 'A'        ;
		m_fld->field_input  = ( cuaFT == CEF || cuaFT == DATAIN || cuaFT == NEF ) ;
		m_fld->field_tb     = true ;
		fieldList[ name +"."+ d2ds( i ) ] = m_fld ;

		fieldOptsParse( RC, opts, m_fld->field_caps, m_fld->field_just, m_fld->field_numeric, m_fld->field_padchar, m_fld->field_skip ) ;
		if ( RC > 0 )
		{
			RC   = 20 ;
			PERR = "Error in options for field "+ name +".  Entry is "+ opts ;
			return    ;
		}

	}
}


void pPanel::create_pdc( string abc_name, string pdc_name, string pdc_run, string pdc_parm, string pdc_unavail )
{
	// ab is a vector list of action-bar-choices (abc objects)
	// Each action-bar-choice is a vector list of pull-down-choices (pdc objects)

	int i      ;
	bool found ;

	abc t_abc  ;

	RC    = 0     ;
	found = false ;

	for ( i = 0 ; i < ab.size() ; i++ )
	{
		if ( ab.at( i ).abc_name == abc_name ) { found = true ; break ; }
	}
	if ( !found )
	{
		t_abc.abc_name = abc_name ;
		t_abc.abc_col  = abc_pos  ;
		abc_pos        = abc_pos + abc_name.size() + 2 ;
		ab.push_back( t_abc ) ;
		i = ab.size()-1       ;
	}
	ab.at( i ).add_pdc( pdc_name, pdc_run, pdc_parm, pdc_unavail ) ;
}


void pPanel::update_field_values( int & RC1 )
{
	// Update field_values from the dialogue variables (may not exist so treat RC1=8 from getDialogueVar as normal completion)

	// Apply just(right|left|asis) to non-dynamic area input/output fields
	//     JUST(LEFT)  strip off leading and trailing spaces
	//     JUST(RIGHT) strip off trailing spaces only and pad to the left with spaces to size field_length
	//     JUST(ASIS) no change
	// Treat dynamic areas differently - they must reside in the function pool.  Use vlocate to get the dynamic area variables
	// via their addresses to avoid large string copies

	string sname    ;
	string * darea  ;
	string * shadow ;

	map<string, field   *>::iterator it1 ;
	map<string, dynArea *>::iterator it2 ;

	RC1 = 0 ;

	for ( it1 = fieldList.begin() ; it1 != fieldList.end() ; it1++ )
	{
		if ( !it1->second->field_dynArea )
		{
			it1->second->field_value = getDialogueVar( it1->first ) ;
			switch( it1->second->field_just )
			{
				case 'L': it1->second->field_value = strip( it1->second->field_value, 'B', ' ' ) ;
					  break ;
				case 'R': it1->second->field_value = strip( it1->second->field_value, 'T', ' ' ) ;
					  it1->second->field_value = right( it1->second->field_value, it1->second->field_length, 0x00 ) ;
					  break ;
			}
		}
		it1->second->field_changed = false ;
	}
	if ( RC1 == 8 ) { RC1 = 0 ; }

	for ( it2 = dynAreaList.begin() ; it2 != dynAreaList.end() ; it2++ )
	{
		darea = p_funcPOOL->vlocate( RC1, 0, it2->first, NOCHECK ) ;
		if ( RC1 > 0 ) { RC1 = 20 ; PERR = "Dynamic area variable has not been defined in the function pool" ; return ; }
		sname  = it2->second->dynArea_shadow_name ;
		shadow = p_funcPOOL->vlocate( RC1, 0, sname, NOCHECK ) ;
		if ( RC1 > 0 ) { RC1 = 20 ; PERR = "Dynamic area shadow variable has not been defined in the function pool" ; return ; }
		if ( darea->size() > shadow->size() )
		{
			log( "W", "Shadow variable " << sname << " size is smaller than the data variable " << it2->first << " size.  Results may be unpredictable" << endl ) ;
			log( "W", "Data variable size   = " << darea->size() << endl ) ;
			log( "W", "Shadow variable size = " << shadow->size() << endl ) ;
		}
		darea->resize( it2->second->dynArea_width * it2->second->dynArea_depth, ' ' )   ;
		shadow->resize( it2->second->dynArea_width * it2->second->dynArea_depth, 0xFF ) ;
		for ( int i = 0 ; i < it2->second->dynArea_depth ; i++ )
		{
			fieldList[ it2->first +"."+ d2ds( i ) ]->field_value        = darea->substr( i * it2->second->dynArea_width, it2->second->dynArea_width )  ;
			fieldList[ it2->first +"."+ d2ds( i ) ]->field_shadow_value = shadow->substr( i * it2->second->dynArea_width, it2->second->dynArea_width ) ;
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
	map<string, field *>::iterator it;

	bool snulls = ( p_poolMGR->get( RC, "ZNULLS", SHARED ) == "YES" ) ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( !it->second->field_active ) { continue ; }
		it->second->display_field( win, snulls ) ;
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
		RC = fieldList[ attrList[ i ] ]->field_attr( "RESET" ) ;
	}
	attrList.clear() ;
}


void pPanel::cursor_to_field( int & RC1, string f_name, int f_pos )
{
	int oX ;
	int oY ;

	RC1 = 0 ;
	if ( f_name == "" ) { f_name = CURFLD ; f_pos = CURPOS ; }
	if ( f_name == "" ) { return                           ; }

	if ( fieldList.find( f_name ) == fieldList.end() )
	{
		if ( dynAreaList.count( f_name ) == 0 )
		{
			p_col = 0  ;
			p_row = 0  ;
			RC1   = 12 ;
			if ( !isvalidName( f_name ) ) { RC1 = 20 ; }
			return     ;
		}
		else
		{
			if ( f_pos < 1 ) f_pos = 1 ;
			f_pos-- ;
			oX = f_pos % ( dynAreaList[ f_name ]->dynArea_width ) ;
			oY = f_pos / ( dynAreaList[ f_name ]->dynArea_width ) ;
			if ( oY >= dynAreaList[ f_name ]->dynArea_depth )
			{
				oX = 0 ;
				oY = 0 ;
			}
			p_col = dynAreaList[ f_name ]->dynArea_col + oX ;
			p_row = dynAreaList[ f_name ]->dynArea_row + oY ;
		}
	}
	else
	{
		if ( f_pos > fieldList[ f_name ]->field_length ) f_pos = 1 ;
		if ( f_pos < 1 ) f_pos = 1 ;
		p_col = fieldList[ f_name ]->field_col + f_pos - 1 ;
		p_row = fieldList[ f_name ]->field_row             ;
	}
}


void pPanel::get_home( uint & row, uint & col )
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


string pPanel::field_getvalue( string f_name )
{
	fieldList[ f_name ]->field_remove_nulls() ;
	return  fieldList[ f_name ]->field_value  ;
}


void pPanel::field_setvalue( string f_name, string f_value )
{
	bool snulls = ( p_poolMGR->get( RC, "ZNULLS", SHARED ) == "YES" ) ;

	if ( f_value.size() > fieldList[ f_name ]->field_length ) { f_value.substr( 0, f_value.size() - 1 ) ; }
	fieldList[ f_name ]->field_value   = f_value ;
	fieldList[ f_name ]->field_changed = true    ;
	fieldList[ f_name ]->display_field( win, snulls ) ;
}


string pPanel::cmd_getvalue()
{
	fieldList[ CMDfield ]->field_remove_nulls() ;
	return fieldList[ CMDfield ]->field_value   ;
}


void pPanel::cmd_setvalue( string f_value )
{
	bool snulls = ( p_poolMGR->get( RC, "ZNULLS", SHARED ) == "YES" ) ;

	if ( f_value.size() > fieldList[ CMDfield ]->field_length ) { f_value.substr( 0, f_value.size() - 1 ) ; }
	fieldList[ CMDfield ]->field_value   = f_value ;
	fieldList[ CMDfield ]->field_changed = true    ;
	fieldList[ CMDfield ]->display_field( win, snulls ) ;
}


bool pPanel::is_cmd_inactive( string cmd )
{
	// Some commands may be inactive for this type of panel, so return true for these.

	// Not sure how real ISPF does this without setting ZCTACT to NOP !! but this works okay.

	if ( findword( cmd, "LEFT RIGHT" ) )
	{
		if ( LRScroll ) { return false ; }
		if ( tb_model ) { return true  ; }
	}
	else if ( cmd == "NRETRIEV" && !nretriev ) { return true ; }

	if ( !scrollOn && findword( cmd, "UP DOWN LEFT RIGHT" ) ) { return true ; }
	return false ;
}


bool pPanel::field_valid( string f_name )
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
			if ( !it->second->field_active ) return "" ;
			return it->first ;
		}
	}
	return "" ;
}


bool pPanel::field_get_row_col( string fld, uint & row, uint & col )
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


fieldExc pPanel::field_getexec( string field )
{
	// If passed field is in the field execute table, return the structure fieldExc for that field as defined in )FIELD panel section.

	if ( fieldExcTable.find( field ) == fieldExcTable.end() ) { return fieldExc() ; }
	return fieldExcTable[ field ] ;
}


void pPanel::field_clear( string f_name )
{
	fieldList[ f_name ]->field_clear( win ) ;
}


void pPanel::field_edit( uint row, uint col, char ch, bool Isrt, bool & prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field

	map<string, field *>::iterator it;

	bool snulls = ( p_poolMGR->get( RC, "ZNULLS", SHARED ) == "YES" ) ;

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
			if ( !it->second->edit_field_insert( win, ch, col, Isrt, snulls ) ) { return ; }
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


void pPanel::field_backspace( uint & row, uint & col, bool & prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field
	// Return physical position of the cursor in row/col

	uint trow ;
	uint tcol ;
	uint p    ;

	bool snulls = ( p_poolMGR->get( RC, "ZNULLS", SHARED ) == "YES" ) ;
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
			tcol = it->second->edit_field_backspace( win, tcol, snulls ) ;
			if ( p == tcol ) { return ; }
			prot = false ;
			row  = trow + win_row ;
			col  = tcol + win_col ;
			return ;
		}
	}
}


void pPanel::field_delete_char( uint row, uint col, bool & prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field
	// Return physical position of the cursor in row/col

	uint trow ;
	uint tcol ;

	map<string, field *>::iterator it ;

	bool snulls = ( p_poolMGR->get( RC, "ZNULLS", SHARED ) == "YES" ) ;
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
			it->second->edit_field_delete( win, tcol, snulls ) ;
			prot = false ;
			row  = trow + win_row ;
			col  = tcol + win_col ;
			return ;
		}
	}
}


void pPanel::field_erase_eof( uint row, uint col, bool & prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field

	map<string, field *>::iterator it;

	bool snulls = ( p_poolMGR->get( RC, "ZNULLS", SHARED ) == "YES" ) ;
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
			it->second->field_erase_eof( win, col, snulls ) ;
			prot = false ;
			return ;
		}
	}
}


void pPanel::cursor_eof( uint & row, uint & col )
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


void pPanel::field_tab_down( uint & row, uint & col )
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



void pPanel::field_tab_next( uint & row, uint & col )
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


string pPanel::get_field_help( string fld )
{
	if ( fieldHList.count( fld ) == 0 ) { return "" ; }
	return fieldHList[ fld ] ;
}


void pPanel::set_tb_linesChanged()
{
	//  Store changed lines for processing by the application if requested via tbdispl with no panel name
	//  Format is a list of line-number/URID pairs

	string URID ;

	map<string, field *>::iterator it ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( it->second->field_tb && it->second->field_changed )
		{
			URID = p_funcPOOL->get( RC, 0, "ZURID."+ d2ds( it->second->field_row - tb_row ), NOCHECK ) ;
			tb_linesChanged[ it->second->field_row - tb_row ] = URID ;
		}
	}
	p_funcPOOL->put( RC, 0, "ZTDSELS", tb_linesChanged.size() ) ;
}


bool pPanel::tb_lineChanged( int & ln, string & URID )
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

	RC = 0 ;
	p_funcPOOL->put( RC, 0, "ZTDSELS", tb_linesChanged.size() ) ;
	return true ;
}


void pPanel::clear_tb_linesChanged( int & RC1 )
{
	//  Clear all stored changed lines on a tbdispl with panel name and set ZTDSELS to zero

	RC1 = 0 ;

	tb_linesChanged.clear() ;
	p_funcPOOL->put( RC1, 0, "ZTDSELS", 0 ) ;
}


void pPanel::remove_tb_lineChanged()
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

	rows = p_funcPOOL->get( RC, 0, INTEGER, "ZTDROWS" ) ;
	top  = p_funcPOOL->get( RC, 0, INTEGER, "ZTDTOP"  ) ;
	size = rows - top + 1 ;
	p_funcPOOL->put( RC, 0, "ZTDVROWS", size ) ;

	wattrset( win, WHITE ) ;
	if ( size < tb_depth )
	{
		mark = p_funcPOOL->get( RC, 0, "ZTDMARK" ) ;
		mvwaddstr( win, tb_row + size, 0, mark.c_str() ) ;
		p_funcPOOL->put( RC, 0, "ZTDVROWS", size ) ;
	}
	else
	{
		p_funcPOOL->put( RC, 0, "ZTDVROWS", tb_depth ) ;
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

	rows = p_funcPOOL->get( RC, 0, INTEGER, "ZTDROWS" ) ;
	top  = p_funcPOOL->get( RC, 0, INTEGER, "ZTDTOP"  ) ;
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



string pPanel::return_command( string opt )
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


void pPanel::set_panel_msg( slmsg t, string msgid )
{
	int i ;

	clear_msg()    ;
	MSG    = t     ;
	MSGID  = msgid ;

	if ( MSG.smsg == "" && MSG.lmsg == "" )
	{
		MSGID = "" ;
		return     ;
	}
	if ( p_poolMGR->get( i, ZSCRNUM, "ZSHMSGID" ) == "Y" ) { MSG.lmsg = msgid +" "+ MSG.lmsg ; }
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


void pPanel::put_keylist( int entry, string keyv )
{
	Keylistl[ entry ] = keyv ;
}


string pPanel::get_keylist( int entry )
{
	if ( KEYLISTN == ""  || Keylistl.count( entry ) == 0 ) { return "" ; }
	return Keylistl[ entry ] ;
}


void pPanel::display_msg()
{
	int w_row ;
	int w_col ;
	int w_depth ;
	int w_width ;

	if ( MSGID == "" ) { return ; }

	msgResp = MSG.resp ;
	if ( MSGID != "PSYS012L" )
	{
		p_poolMGR->put( RC, ZSCRNUM, "ZMSGID", MSGID ) ;
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
			get_msgwin( MSG.smsg.size(), w_row, w_col, w_depth, w_width ) ;
			smwin = newwin( w_depth, w_width, w_row+win_row, w_col+win_col ) ;
			wattrset( smwin, cuaAttr[ MSG.type ] ) ;
			box( smwin, 0, 0 ) ;
			mvwaddstr( smwin, 1, 2, MSG.smsg.c_str() ) ;
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
		if ( MSG.lmwin || p_poolMGR->get( RC, "ZLMSGW", PROFILE ) == "Y" )
		{
			get_msgwin( MSG.lmsg.size(), w_row, w_col, w_depth, w_width ) ;
			lmwin = newwin( w_depth, w_width, w_row+win_row, w_col+win_col ) ;
			wattrset( lmwin, cuaAttr[ MSG.type ] )  ;
			box( lmwin, 0, 0 ) ;
			mvwaddstr( lmwin, 1, 2, MSG.lmsg.c_str() ) ;
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


void pPanel::get_msgwin( int t_size, int & t_row, int & t_col, int & t_depth, int & t_width )
{
	// Calculate the message window position and size
	// TODO: multi-line messages

	int i  ;
	int w1 ;

	if ( MSGLOC != "" && fieldList.count( MSGLOC ) > 0 )
	{
		t_row = fieldList[ MSGLOC ]->field_row + 1 ;
		t_col = fieldList[ MSGLOC ]->field_col - 2 ;
		w1    = WSCRMAXD - t_col ;
		t_depth = 3 ;
		t_width = t_size + 4 ;
	}
	else
	{
		t_width = t_size + 4 ;
		i       = t_width / WSCRMAXW ;
		t_width = t_width / (i + 1 ) ;
		t_depth = i + 3              ;
		t_row   = WSCRMAXD - t_depth - 1 ;
		t_col   = (WSCRMAXW - t_width)/2 ;
	}
}


void pPanel::display_id()
{
	string scrname ;
	string panarea ;

	RC = 0 ;

	panarea = "" ;

	if ( idwin != NULL )
	{
		panel_cleanup( idpanel ) ;
		del_panel( idpanel ) ;
		delwin( idwin )      ;
		idwin = NULL         ;
	}

	if ( p_poolMGR->get( RC, ZSCRNUM, "ZSHPANID" ) == "Y" )
	{
		panarea = PANELID + " " ;
	}

	if ( p_poolMGR->get( RC, "ZSCRNAM1", SHARED ) == "ON" )
	{
		scrname = p_poolMGR->get( RC, "ZSCRNAME", SHARED ) ;
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


void pPanel::get_panel_info( int & RC1, string a_name, string t_name, string w_name, string d_name, string r_name, string c_name )
{
	map<string, dynArea *>::iterator it ;

	RC1 = 0 ;

	it = dynAreaList.find( a_name ) ;
	if ( it == dynAreaList.end() )
	{
		log( "E", "PQUERY.  Dynamic area " << a_name << " not found" << endl ) ;
		RC1 = 8 ;
		return  ;
	}

	if ( t_name != "" ) { p_funcPOOL->put( RC1, 0, t_name, "DYNAMIC" ) ; }
	if ( w_name != "" ) { p_funcPOOL->put( RC1, 0, w_name, it->second->dynArea_width ) ; }
	if ( d_name != "" ) { p_funcPOOL->put( RC1, 0, d_name, it->second->dynArea_depth ) ; }
	if ( r_name != "" ) { p_funcPOOL->put( RC1, 0, r_name, it->second->dynArea_row )   ; }
	if ( c_name != "" ) { p_funcPOOL->put( RC1, 0, c_name, it->second->dynArea_col )   ; }
}


void pPanel::attr( int & RC1, string field, string attrs )
{
	RC1 = 0 ;
	if ( fieldList.count( field ) == 0 )
	{
		log( "E", "ATTR.  Field " << field << "not found" << endl ) ;
		RC1 = 8  ;
	}
	else { RC1 = fieldList[ field ]->field_attr( attrs ) ; }
}


void pPanel::panel_cleanup( PANEL * p )
{
	const void * vptr ;

	vptr = panel_userptr( p ) ;
	if ( vptr != NULL )
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
	p_poolMGR->put( RC, "ZKLNAME", KEYLISTN, SHARED ) ;
	p_poolMGR->put( RC, "ZKLAPPL", KEYAPPL,  SHARED ) ;
	p_poolMGR->put( RC, "ZKLTYPE", "P",      SHARED ) ;
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
			val = getDialogueVar( var ) ;
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
	return s ;
}
