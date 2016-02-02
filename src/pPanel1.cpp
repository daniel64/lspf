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
	abActive    = false  ;
	nretriev    = false  ;
	nretfield   = ""     ;
	KEYLISTN    = ""     ;
	KEYAPPL     = ""     ;
	PanelTitle  = ""     ;
	abIndex     = 0      ;
	opt_field   = 0      ;
	tb_model    = false  ;
	tb_depth    = 0      ;
	tb_fields   = ""     ;
	win         = stdscr ;
	win_addpop  = false  ;
	win_created = false  ;
	pan_created = false  ;
	win_row     = 0      ;
	win_col     = 0      ;
	win_width   = 0      ;
	win_depth   = 0      ;
	dyn_depth   = 0      ;
	ZPHELP      = ""     ;
	PERR        = ""     ;
	CMDfield    = "ZCMD" ;
	Home        = "ZCMD" ;
}


pPanel::~pPanel()
{
	/* iterate over the 4 panel widget types, literal, field (inc tbfield, dynarea field), dynArea, boxes and delete them */

	int i ;
	map<string, field *>::iterator it1;
	map<string, dynArea *>::iterator it2;
	vector<Box *>::iterator it3;

	for ( i = 0 ; i < literalList.size() ; i++ )
	{
		delete literalList.at( i ) ;
	}

	for ( it1 = fieldList.begin() ; it1 != fieldList.end() ; it1++ )
	{
		delete it1->second ;
	}

	for ( it2 = dynAreaList.begin() ; it2 != dynAreaList.end() ; it2++ )
	{
		delete it2->second ;
	}
	for ( it3 = boxes.begin() ; it3 != boxes.end() ; it3++ )
	{
		delete (*it3) ;
	}

}


void pPanel::init( int & RC )
{
	RC = 0 ;

	ZSCRMAXD = ds2d( p_poolMGR->get( RC, "ZSCRMAXD", SHARED ) ) ;
	if ( RC > 0 ) { PERR = "ZSCRMAXD poolMGR.get failed" ; log("C", PERR ) ; RC = 20 ; return ; }
	ZSCRMAXW = ds2d( p_poolMGR->get( RC, "ZSCRMAXW", SHARED ) ) ;
	if ( RC > 0 ) { PERR = "ZSCRMAXW poolMGR.get failed" ; log("C", PERR ) ; RC = 20 ; return ; }
	WSCRMAXD = ZSCRMAXD ;
	WSCRMAXW = ZSCRMAXW ;

}


string pPanel::getDialogueVar( string var )
{
	// Return the value of a dialogue variable (always as a string so convert int->string if necessary)
	// Search order is:
	//   Function pool explicit
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
}


void pPanel::putDialogueVar( string var, string val )
{
	// Upate a dialogue variable where it resides in the standard search order.
	// If not found, create an implicit function pool variable

	if      ( p_funcPOOL->ifexists( RC, var ) ) { p_funcPOOL->put( RC, 0, var, val )   ; }
	else if ( p_poolMGR->ifexists( RC, var ) )  { p_poolMGR->put( RC, var, val, ASIS ) ; }
	else                                        { p_funcPOOL->put( RC, 0, var, val )   ; }
}


void pPanel::initDialogueVar( string var )
{
	// If a dialogue variable does not exist, initialise an enty in the function pool.
	// Required for REXX support so variables have a default value of blank, instead of the
	// REXX default value of the variable's name

	if      ( p_funcPOOL->ifexists( RC, var ) ) { return ; }
	else if ( p_poolMGR->ifexists( RC, var ) )  { return ; }
	else                                        { p_funcPOOL->put( RC, 0, var, "" ) ; }
}


void pPanel::set_popup( int sp_row, int sp_col )
{
	win_addpop= true   ;
	sp_row + win_depth + 1 < ZSCRMAXD ? win_row = sp_row : win_row = ZSCRMAXD - win_depth - 1 ;
	sp_col + win_width + 1 < ZSCRMAXW ? win_col = sp_col : win_col = ZSCRMAXW - win_width - 1 ;
}


void pPanel::remove_popup()
{
	win_addpop = false ;
	win_row    = 0     ;
	win_col    = 0     ;
}


void pPanel::display_panel( int & RC )
{
	debug1( "Displaying fields and action bar " << endl ) ;

	RC = 0 ;
	clear() ;

	if ( win_addpop && win_width > 0 )
	{
		if ( !pan_created )
		{
			mvwin( win, win_row, win_col )      ;
			mvwin( bwin, win_row-1, win_col-1 ) ;
			wattrset( bwin, cuaAttr[ AWF ] ) ;
			box( bwin, 0, 0 ) ;
			wattroff( bwin, cuaAttr[ AWF ] ) ;
			panel  = new_panel( win )  ;
			bpanel = new_panel( bwin ) ;
			pan_created = true ;
			update_panels()    ;

		}
		else
		{
			mvwin( win, win_row, win_col )      ;
			mvwin( bwin, win_row-1, win_col-1 ) ;
			show_panel( panel )  ;
			show_panel( bpanel ) ;
			update_panels()      ;
		}
	}

	wattrset( win, cuaAttr[ PT ] ) ;
	mvwaddstr( win, 2, ( WSCRMAXW - PanelTitle.size() ) / 2, PanelTitle.c_str() ) ;
	wattroff( win, cuaAttr[ PT ] ) ;

	if ( tb_model )
	{
		p_funcPOOL->put( RC, 0, "ZTDSELS", 0 ) ;
		display_tb_mark_posn() ;
		tb_fields_active_inactive() ;
	}

	display_ab()       ;
	display_literals() ;
	update_field_values( RC ) ;

	display_fields() ;
	display_boxes()  ;
	hide_pd()        ;
	display_pd()     ;
	display_MSG()    ;
	wrefresh( win ) ;
}


void pPanel::refresh( int & RC )
{
	debug1( "Refreshing panel fields and action bar " << endl ) ;

	RC = 0  ;
	clear() ;

	if ( pan_created )
	{
		show_panel( panel ) ;
		update_panels()     ;
	}

	wattrset( win, cuaAttr[ PT ] ) ;
	mvwaddstr( win, 2, ( WSCRMAXW - PanelTitle.size() ) / 2, PanelTitle.c_str() ) ;
	wattroff( win, cuaAttr[ PT ] ) ;

	display_ab()       ;
	display_literals() ;
	display_fields()   ;
	display_boxes()    ;
	hide_pd()          ;
	display_MSG()      ;
	display_pd()       ;
	if ( tb_model ) { display_tb_mark_posn() ; }
}


void pPanel::nrefresh()
{
	//  Refresh the ncurses window for panels only (if in a popup)

	if ( !pan_created ) { wrefresh( win ) ; }
}


void pPanel::display_panel_update( int & RC )
{
	//  For all changed fields, apply attributes (upper case, left/right justified), and copy back to function pool
	//  If END entered with pull down displayed, remove the pull down and clear ZVERB
	//  If cursor on a point-and-shoot field (PS), set variable as in the )PNTS panel section if defined there
	//  When copying to the function pool, change value as follows:
	//      JUST(LEFT/RIGHT) leading and trailing spaces are removed
	//      JUST(ASIS) Only trailing spaces are removed

	//  Clear error message if END pressed so panel is not re-displaed

	int fieldNum    ;
	int scrollAmt   ;
	int curpos      ;
	int p           ;

	string CMDVerb  ;
	string CMD      ;
	string t        ;
	string fieldNam ;
	string msgfld   ;

	dataType var_type ;

	map<string, field *>::iterator it ;

	RC       = 0  ;
	fieldNum = 0  ;
	MSGID    = "" ;

	CMDVerb = p_poolMGR->get( RC, "ZVERB", SHARED ) ;
	if ( CMDVerb == "END" && abActive )
	{
		abActive = false ;
		p_poolMGR->put( RC, "ZVERB", "",  SHARED ) ;
	}

	if ( abActive )
	{
		hide_pd()        ;
		abActive = false ;
	}

	if ( scrollOn )
	{
		it = fieldList.find( "ZSCROLL" ) ;
		it->second->field_value = upper( strip( it->second->field_value ) ) ;
		if ( !isnumeric( it->second->field_value ) )
		{
			switch( it->second->field_value[ 0 ] )
			{
				case 'C': it->second->field_value = "CSR " ; break ;
				case 'D': it->second->field_value = "DATA" ; break ;
				case 'H': it->second->field_value = "HALF" ; break ;
				case 'P': it->second->field_value = "PAGE" ; break ;
				default:  MSGID  = "PSYS01I" ; CURFLD = "ZSCROLL"  ;
			}
		}
		p_funcPOOL->put( RC, 0, it->first, it->second->field_value ) ;
		it->second->field_changed = false ;
	}

	p_funcPOOL->put( RC, 0, "ZCURFLD", "" ) ;
	p_funcPOOL->put( RC, 0, "ZCURPOS", 1  ) ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( it->second->field_changed )
		{
			debug1( "field has changed " << it->first << endl ) ;
			if ( !it->second->field_dynArea )
			{
				debug1( "Updating function pool " << it->first << ">>" << it->second->field_value << "<<" << endl ) ;
				switch( it->second->field_just )
				{
					case 'L':
					case 'R': it->second->field_value = strip( it->second->field_value, 'B', ' ' ) ; break ;
					case 'A': it->second->field_value = strip( it->second->field_value, 'T', ' ' ) ; break ;
				}
				if ( it->second->field_caps ) it->second->field_value = upper( it->second->field_value ) ;
				debug1( " (After applying attributes) " << it->first << ">>" << it->second->field_value << "<<" << endl ) ;
				var_type = p_funcPOOL->getType( RC, it->first ) ;
				if ( RC == 0 && var_type == INTEGER )
				{
					if ( datatype( it->second->field_value, 'W' ) )
					{
						p_funcPOOL->put( RC, 0, it->first, ds2d( it->second->field_value ) ) ;
					}
					else
					{
						MSGID  = "PSYS01G" ;
						CURFLD = it->first ;
					}
				}
				else
				{
					p_funcPOOL->put( RC, 0, it->first, it->second->field_value, NOCHECK ) ;
				}
				if ( RC > 0 ) continue ;
			}
			else
			{
				p        = it->first.find_first_of( '.' ) ;
				fieldNam = it->first.substr( 0, p )       ;
				fieldNum = ds2d( it->first.substr( (p + 1) , string::npos ) ) ;
				t    = p_funcPOOL->get( RC, 0, fieldNam ) ;
				t    = t.replace( fieldNum*it->second->field_length, it->second->field_length, it->second->field_value ) ;
				p_funcPOOL->put( RC, 0, fieldNam, t ) ;
				if ( RC > 0 ) continue ;
			}
		}
		if ( (it->second->field_row == p_row) && (p_col >=it->second->field_col) && (p_col < (it->second->field_col + it->second->field_length )) )
		{
			if ( it->second->field_dynArea )
			{
				p        = it->first.find_first_of( '.' ) ;
				fieldNam = it->first.substr( 0, p )       ;
				fieldNum = ds2d( it->first.substr( (p + 1) , string::npos ) ) ;
				p_funcPOOL->put( RC, 0, "ZCURFLD", fieldNam ) ;
				p_funcPOOL->put( RC, 0, "ZCURPOS", ( fieldNum * it->second->field_length + p_col - it->second->field_col + 1 ) ) ;
				curpos   = p_col - it->second->field_col + 1 ;
				fieldNum++ ;
			}
			else
			{
				p_funcPOOL->put( RC, 0, "ZCURFLD", it->first ) ;
				p_funcPOOL->put( RC, 0, "ZCURPOS", ( p_col - it->second->field_col + 1 ) ) ;
			}
		}
	}

	if ( scrollOn )
	{
		p_poolMGR->put( RC, "ZSCROLLA", "",  SHARED ) ;
		p_poolMGR->put( RC, "ZSCROLLN", "0", SHARED ) ;
		CMD      = fieldList[ CMDfield ]->field_value ;
		msgfld   = CMDfield ;
		if ( CMD == "" )
		{
			CMD    = fieldList[ "ZSCROLL" ]->field_value ;
			msgfld = "ZSCROLL" ;
		}
		if ( findword( CMDVerb, "UP DOWN LEFT RIGHT" ) )
		{
			if      ( tb_model )                          { scrollAmt = tb_depth  ; }
			else if ( findword( CMDVerb, "LEFT RIGHT" ) ) { scrollAmt = dyn_width ; }
			else                                          { scrollAmt = dyn_depth ; }
			if ( isnumeric( CMD ) )
			{
				if ( CMD.size() > 6 )
				{
					MSGID  = "PSYS01I" ;
					CURFLD = msgfld    ;
				}
				p_poolMGR->put( RC, "ZSCROLLA", CMD, SHARED ) ;
				p_poolMGR->put( RC, "ZSCROLLN", CMD, SHARED ) ;
			}
			else
			{
				CMD = upper( CMD ) ;
				if ( (CMD == "M") || (CMD == "MAX") )
				{
					p_poolMGR->put( RC, "ZSCROLLA", "MAX", SHARED ) ;
				}
				else
				{
					switch ( CMD[ 0 ] )
					{
					case 'C':
						if ( fieldNum == 0 )
						{
							p_poolMGR->put( RC, "ZSCROLLN", d2ds( scrollAmt ),  SHARED ) ;
						}
						else if ( CMDVerb == "UP" )
						{
							p_poolMGR->put( RC, "ZSCROLLN", d2ds( dyn_depth-fieldNum ), SHARED ) ;
						}
						else if ( CMDVerb == "DOWN"  )
						{
							p_poolMGR->put( RC, "ZSCROLLN", d2ds( fieldNum-1 ), SHARED ) ;
						}
						else if ( CMDVerb == "LEFT"  )
						{
							p_poolMGR->put( RC, "ZSCROLLN", d2ds( dyn_width-curpos ), SHARED ) ;
						}
						else
						{
							p_poolMGR->put( RC, "ZSCROLLN", d2ds( curpos ), SHARED ) ;
						}
						p_poolMGR->put( RC, "ZSCROLLA", "CSR", SHARED ) ;
						break ;
					case 'D':
						p_poolMGR->put( RC, "ZSCROLLN", d2ds( scrollAmt ), SHARED ) ;
						p_poolMGR->put( RC, "ZSCROLLA", "DATA", SHARED ) ;
						break ;
					case 'H':
						p_poolMGR->put( RC, "ZSCROLLN", d2ds( scrollAmt/2 ), SHARED ) ;
						p_poolMGR->put( RC, "ZSCROLLA", "HALF", SHARED ) ;
						break ;
					case 'P':
						p_poolMGR->put( RC, "ZSCROLLN", d2ds( scrollAmt ), SHARED ) ;
						p_poolMGR->put( RC, "ZSCROLLA", "PAGE", SHARED ) ;
						break ;
					default:
						MSGID  = "PSYS01I" ;
						CURFLD = msgfld    ;
					}
				}
			}
			fieldList[ CMDfield ]->field_value = "" ;
			p_funcPOOL->put( RC, 0, CMDfield, "" )  ;
		}
	}
	if ( findword( CMDVerb, "END EXIT RETURN" ) ) { MSGID = "" ; }
}



void pPanel::display_panel_init( int & RC )
{
	// Perform panel )INIT processing
	// Set RC=8 from VGET/VPUT to 0 as it isn't an error in this case
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

	RC = 0 ;
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
				t = p_funcPOOL->get( RC, 0, "ZCURFLD" ) ;
			}
			else
			{
				t = assgnListi.at( i_assign ).as_rhs ;
			}
			if ( assgnListi.at( i_assign ).as_lhs == ".AUTOSEL" )
			{
				if ( t == "" ) { t = "YES" ; }
				if ( t != "YES" && t != "NO" ) { RC = 20 ; PERR = "Invalid .AUTOSEL value.  Must be YES, NO or blank" ; return ; }
				p_funcPOOL->put( RC, 0, ".AUTOSEL", t, NOCHECK ) ;
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
				MSGID  = t ;
			}
			else if ( assgnListi.at( i_assign ).as_lhs == ".CSRROW" )
			{
				if ( !isnumeric( t ) ) { RC = 20 ; PERR = "Invalid .CSRROW value.  Must be numeric" ; return ; }
				p_funcPOOL->put( RC, 0, ".CSRROW", t, NOCHECK ) ;
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
						if ( fieldList[ fieldNam + "." + d2ds( j ) ]->field_attr( t ) > 0 )
						{
							PERR = "Error setting attribute " + t + " for table display field " +fieldNam ;
							RC   = 20 ;
							return    ;
						}
					}
				}
				else
				{
					if ( fieldList[ fieldNam ]->field_attr( t ) > 0 )
					{
						PERR = "Error setting attribute " + t + " for field " + fieldNam ;
						RC   = 20 ;
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
					val = p_poolMGR->get( RC, var, vpgListi.at( i_vputget ).vpg_pool ) ;
					if ( RC == 0 )
					{
						p_funcPOOL->put( RC, 0, var, val ) ;
					}
				}
			}
			if ( vpgListi.at( i_vputget ).vpg_vput )
			{
				for ( j = 1 ; j <= ws ; j++ )
				{
					var = word( vars, j ) ;
					val = p_funcPOOL->get( RC, 8, var ) ;
					if ( RC == 0 )
					{
						p_poolMGR->put( RC, var, val, vpgListi.at( i_vputget ).vpg_pool ) ;
					}
				}
			}
			if ( RC == 8 ) { RC = 0 ; }
			i_vputget++ ;
		}
	}
}


void pPanel::display_panel_reinit( int & RC, int ln )
{
	// Perform panel )REINIT processing
	// Set RC=8 from VGET/VPUT to 0 as it isn't an error in this case
	// For table display fields, .ATTR and .CURSOR apply to fields on line ln

	RC = 0 ;
	int i  ;
	int j ;
	int ws ;
	int i_vputget ;
	int i_assign  ;

	map<string, field *>::iterator it ;

	i_vputget = 0  ;
	i_assign  = 0  ;

	string t ;
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
				t = p_funcPOOL->get( RC, 0, "ZCURFLD" ) ;
			}
			else
			{
				t = assgnListr.at( i_assign ).as_rhs ;
			}
			if ( assgnListr.at( i_assign ).as_lhs == ".AUTOSEL" )
			{
				if ( t == "" ) { t = "YES" ; }
				if ( t != "YES" && t != "NO" ) { RC = 20 ; PERR = "Invalid .AUTOSEL value.  Must be YES, NO or blank" ; return ; }
				p_funcPOOL->put( RC, 0, ".AUTOSEL", t, NOCHECK ) ;
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
				if ( !isnumeric( t ) ) { RC = 20 ; PERR = "Invalid .CSRROW value.  Must be numeric" ; return ; }
				p_funcPOOL->put( RC, 0, ".CSRROW", t, NOCHECK ) ;
			}
			else if ( assgnListr.at( i_assign ).as_lhs == ".CURSOR" )
			{
				if ( wordpos( assgnListr.at( i_assign ).as_rhs, tb_fields ) > 0 ) { t = t + "." + d2ds( ln ) ; }
				CURFLD = t ;
			}
			else if ( assgnListr.at( i_assign ).as_isattr )
			{
				fieldNam = assgnListr.at( i_assign ).as_lhs ;
				if ( assgnListr.at( i_assign ).as_istb )
				{
					if ( fieldList[ fieldNam + "." + d2ds( ln ) ]->field_attr( t ) > 0 )
					{
						PERR = "Error setting attribute " + t + " for table display field " +fieldNam ;
						RC   = 20 ;
						return    ;
					}
					attrList.push_back( fieldNam + "." + d2ds( ln ) ) ;
				}
				else
				{
					if ( fieldList[ fieldNam ]->field_attr( t ) > 0 )
					{
						PERR = "Error setting attribute " + t + " for field " + fieldNam ;
						RC   = 20 ;
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
					val = p_poolMGR->get( RC, var, vpgListr.at( i_vputget ).vpg_pool ) ;
					if ( RC == 0 )
					{
						p_funcPOOL->put( RC, 0, var, val ) ;
					}
				}
			}
			if ( vpgListr.at( i_vputget ).vpg_vput )
			{
				for ( j = 1 ; j <= ws ; j++ )
				{
					var = word( vars, j ) ;
					val = p_funcPOOL->get( RC, 8, var ) ;
					if ( RC == 0 )
					{
						p_poolMGR->put( RC, var, val, vpgListr.at( i_vputget ).vpg_pool ) ;
					}
				}
			}
			if ( RC == 8 ) { RC = 0 ; }
			i_vputget++ ;
		}
	}
}


void pPanel::display_panel_proc( int & RC, int ln )
{
	//  Process )PROC statements
	//  Set RC=8 from VGET/VPUT to 0 as it isn't an error in this case
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

	RC        = 0  ;
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
				if      ( procstmnts.at( i ).ps_if     )  { i_if++      ; }
				else if ( procstmnts.at( i ).ps_else   )  { i_if++      ; }
				else if ( procstmnts.at( i ).ps_assign )  { i_assign++  ; }
				else if ( procstmnts.at( i ).ps_verify )  { i_verify++  ; }
				else if ( procstmnts.at( i ).ps_vputget ) { i_vputget++ ; }
				else if ( procstmnts.at( i ).ps_trunc )   { i_trunc++   ; }
				else if ( procstmnts.at( i ).ps_trans )   { i_trans++   ; }
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
				val = p_funcPOOL->get( RC, 0, "ZCURFLD" ) ;
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
			if ( transList.at( i_trans ).tlst.find( t ) !=  transList.at( i_trans ).tlst.end() )
			{
				t = transList.at( i_trans ).tlst[ t ] ;
			}
			else
			{
				if ( transList.at( i_trans ).tlst.find( "*" ) !=  transList.at( i_trans ).tlst.end() )
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
					val = p_poolMGR->get( RC, var, vpgList.at( i_vputget ).vpg_pool ) ;
					if ( RC == 0 )
					{
						p_funcPOOL->put( RC, 0, var, val ) ;
					}
				}
			}
			if ( vpgList.at( i_vputget ).vpg_vput )
			{
				for ( j = 1 ; j <= ws ; j++ )
				{
					var = word( vars, j ) ;
					val = p_funcPOOL->get( RC, 8, var ) ;
					if ( RC == 0 )
					{
						p_poolMGR->put( RC, var, val, vpgList.at( i_vputget ).vpg_pool ) ;
					}
				}
			}
			if ( RC == 8 ) { RC = 0 ; }
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
				t = p_funcPOOL->get( RC, 0, "ZCURFLD" ) ;
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
				p_funcPOOL->put( RC, 0, "ZCURFLD", t ) ;
				if ( wordpos( assgnList.at( i_assign ).as_rhs, tb_fields ) > 0 ) { t = t + "." + d2ds( ln ) ; }
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
				MSGID  = t ;
			}
			else if ( assgnList.at( i_assign ).as_lhs == ".CSRROW" )
			{
				if ( !isnumeric( t ) ) { RC = 20 ; PERR = "Invalid .CSRROW value.  Must be numeric" ; return ; }
				p_funcPOOL->put( RC, 0, ".CSRROW", t, NOCHECK ) ;
			}
			else if ( assgnList.at( i_assign ).as_isattr )
			{
				fieldNam = assgnList.at( i_assign ).as_lhs ;
				if ( assgnList.at( i_assign ).as_istb )
				{
					if ( fieldList[ fieldNam + "." + d2ds( ln ) ]->field_attr( t ) > 0 )
					{
						PERR = "Error setting attribute " + t + " for table display field " +fieldNam ;
						RC   = 20 ;
						return    ;
					}
					attrList.push_back( fieldNam + "." + d2ds( ln ) ) ;
				}
				else
				{
					if ( fieldList[ fieldNam ]->field_attr( t ) > 0 )
					{
						PERR = "Error setting attribute " + t + " for field " + fieldNam ;
						RC   = 20 ;
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
			if ( verList.at( i_verify ).ver_tbfield ) { fieldNam = fieldNam + "." + d2ds( ln ) ; }
			it = fieldList.find( fieldNam ) ;
			if ( verList.at( i_verify ).ver_nblank )
			{
				if ( it->second->field_value == "" )
				{
					MSGID  = verList.at( i_verify ).ver_msgid  ;
					if (MSGID == "" ) { MSGID = "PSYS019" ; }
					CURFLD = fieldNam ;
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
					if (MSGID == "" ) { MSGID = "PSYS01A" ; }
					CURFLD = fieldNam ;
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
								p_funcPOOL->put( RC, 0, it->first, it->second->field_value, NOCHECK ) ;
								found = true ;
								break ;
							}
						}
					}
				}
				if ( !found )
				{
					MSGID  = verList.at( i_verify ).ver_msgid  ;
					if (MSGID == "" ) { MSGID = "PSYS01B" ; }
					CURFLD = fieldNam ;
					return            ;
				}
			}
			else if ( verList.at( i_verify ).ver_pict )
			{
				if ( !ispict( it->second->field_value, verList.at( i_verify ).ver_value ) )
				{
					p_poolMGR->put( RC, "ZZSTR1", d2ds( verList.at( i_verify ).ver_value.size() ), SHARED ) ;
					p_poolMGR->put( RC, "ZZSTR2", verList.at( i_verify ).ver_value, SHARED ) ;
					MSGID  = verList.at( i_verify ).ver_msgid  ;
					if (MSGID == "" ) { MSGID = "PSYS01N" ; }
					CURFLD = fieldNam ;
					return            ;
				}
			}
			else if ( verList.at( i_verify ).ver_hex )
			{
				if ( !isvalidHex( it->second->field_value ) )
				{
					MSGID  = verList.at( i_verify ).ver_msgid  ;
					if (MSGID == "" ) { MSGID = "PSYS01H" ; }
					CURFLD = fieldNam ;
					return            ;
				}
			}
			else if ( verList.at( i_verify ).ver_octal )
			{
				if ( !isoctal( it->second->field_value ) )
				{
					MSGID  = verList.at( i_verify ).ver_msgid  ;
					if (MSGID == "" ) { MSGID = "PSYS01F" ; }
					CURFLD = fieldNam ;
					return            ;
				}
			}
			i_verify++ ;
		}
	}

	fieldNam = p_funcPOOL->get( RC, 0, "ZCURFLD" ) ;
	if ( fieldNam != "" )
	{
		it = fieldList.find( fieldNam ) ;
		if ( it != fieldList.end() )
		{
			if ( it->second->field_cua == PS )
			{
				if ( pntsTable.find( fieldNam ) != pntsTable.end() )
				{
					p_funcPOOL->put( RC, 0, pntsTable[ fieldNam ].pnts_var, pntsTable[ fieldNam ].pnts_val ) ;
				}
			}
		}
	}
	else
	{
		for ( i = 0 ; i < literalList.size() ; i++ )
		{
			fieldNam = literalList.at( i )->literal_name ;
			if ( fieldNam == "" ) { continue ; }
			if ( (literalList.at( i )->literal_row == p_row) && (p_col >=literalList.at( i )->literal_col) &&
			     (p_col < (literalList.at( i )->literal_col + literalList.at( i )->literal_length )) )
			{
				if ( pntsTable.find( fieldNam ) != pntsTable.end() )
				{
					p_funcPOOL->put( RC, 0, pntsTable[ fieldNam ].pnts_var, pntsTable[ fieldNam ].pnts_val ) ;
				}
				break ;
			}
		}
	}
}


void pPanel::clear()
{
	RC = 0 ;
	for ( int i = 0 ; i < WSCRMAXD ; i ++ )
	{
		wmove( win, i, 0 ) ;
		clrtoeol()   ;
	}
}


void pPanel::create_tbfield( int col, int length, cuaType cuaFT, string name, string opts )
{
	// Default is JUST(ASIS) for fields of a TB model, so change from the default of JUST(LEFT)

	RC = 0 ;
	if ( !isvalidName( name ) )
	{
		RC = 20 ;
		PERR = "Invalid field name " + name + " entered for TB field" ; return  ;
	}

	debug2( "Adding tb field name >>" << name << "<< " << endl ) ;
	tb_fields = tb_fields + " " + name ;

	for ( int i = 0 ; i < tb_depth ; i++ )
	{
		field * m_fld       = new field   ;
		m_fld->field_cua    = cuaFT ;
		m_fld->field_prot   = cuaAttrProt [ cuaFT ] ;
		m_fld->field_row    = tb_row + i ;
		m_fld->field_col    = col -1     ;
		m_fld->field_length = length     ;
		m_fld->field_just   = 'A'        ;

		if ( cuaFT == CEF || cuaFT == DATAIN || cuaFT == NEF ) { m_fld->field_input = true  ; }

		fieldOptsParse( RC, opts, m_fld->field_caps, m_fld->field_just, m_fld->field_numeric, m_fld->field_padchar, m_fld->field_skip ) ;
		if ( RC > 0 )
		{
			RC = 20 ;
			PERR = "Error in options for field " + name + ". Entry is " + opts ; return ;
		}

		m_fld->field_tb = true  ;
		fieldList[ name + "." + d2ds( i ) ] = m_fld  ;
	}
}


void pPanel::create_pdc( string abc_name, string pdc_name, string pdc_run, string pdc_parm, string pdc_unavail )
{
	int i      ;
	bool found ;
	abc t_abc  ;

	RC    = 0     ;
	found = false ;

	for ( i = 0 ; i < ab.size() ; i++ )
	{
		if ( ab.at(i).abc_name == abc_name ) { found = true ; break ; }
	}
	if ( !found )
	{
		t_abc.abc_name   = abc_name ;
		t_abc.abc_col    = abc_pos  ;
		abc_pos          = abc_pos + abc_name.size() + 2 ;
		ab.push_back( t_abc ) ;
		i = ab.size()-1       ;
	}
	ab.at(i).add_pdc( pdc_name, pdc_run, pdc_parm, pdc_unavail ) ;
}


void pPanel::update_field_values( int & RC )
{
	// Update field_values from the dialogue variables (may not exist so treat RC=8 from getDialogueVar as normal completion)
	//
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

	RC = 0 ;
	debug1( "Updating field values from dialogue variables (normal search order)" << endl ) ;
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
					  it1->second->field_value = right( it1->second->field_value, it1->second->field_length, ' ' ) ;
					  break ;
			}
		}
		it1->second->field_changed = false ;
	}
	if ( RC == 8 ) { RC = 0 ; }

	for ( it2 = dynAreaList.begin() ; it2 != dynAreaList.end() ; it2++ )
	{
		darea  = p_funcPOOL->vlocate( RC, 0, it2->first, NOCHECK ) ;
		if ( RC > 0 ) { PERR = "Error:  Dynamic area variable has not been defined in the function pool" ; return  ; }
		sname  = dynAreaList[ it2->first ]->dynArea_shadow_name ;
		shadow = p_funcPOOL->vlocate( RC, 0, sname, NOCHECK )   ;
		if ( RC > 0 ) { PERR = "Error:  Dynamic area shadow variable has not been defined in the function pool" ; return  ; }
		if ( (*darea).size() > (*shadow).size() )
		{
			log( "W", "Shadow variable " << sname << " size is smaller than the data variable " << it2->first << " size.  Results may be unpredictable" << endl ) ;
			log( "W", "Data variable size   = " << (*darea).size() << endl ) ;
			log( "W", "Shadow variable size = " << (*shadow).size() << endl ) ;
		}
		(*darea).resize( it2->second->dynArea_width * it2->second->dynArea_depth, ' ' )  ;
		(*shadow).resize( it2->second->dynArea_width * it2->second->dynArea_depth, ' ' ) ;
		for ( int i = 0 ; i < it2->second->dynArea_depth ; i++ )
		{
			fieldList[ it2->first + "." + d2ds( i )]->field_value        = (*darea).substr( i * it2->second->dynArea_width, it2->second->dynArea_width )  ;
			fieldList[ it2->first + "." + d2ds( i )]->field_shadow_value = (*shadow).substr( i * it2->second->dynArea_width, it2->second->dynArea_width ) ;
		}
	}
}


void pPanel::display_literals()
{
	RC = 0 ;
	debug2( "Displaying literals " << endl ) ;

	for ( uint i = 0 ; i < literalList.size() ; i++ )
	{
		literalList.at( i )->literal_display( win ) ;
	}
}


void pPanel::display_fields()
{
	debug2( "Displaying active fields (from field values only) " << endl ) ;
	map<string, field *>::iterator it;

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( !it->second->field_active ) { continue ; }
		it->second->display_field( win ) ;
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


void pPanel::cursor_to_field( int & RC, string f_name, int f_pos )
{
	int oX ;
	int oY ;

	RC = 0 ;
	if ( f_name == "" ) { f_name = CURFLD ; f_pos = CURPOS ; }
	if ( f_name == "" ) { return                           ; }

	if ( fieldList.find( f_name ) == fieldList.end() )
	{
		if ( dynAreaList.find( f_name ) == dynAreaList.end() )
		{
			p_col = 0  ;
			p_row = 0  ;
			RC    = 12 ;
			if ( !isvalidName( f_name ) ) { RC = 20 ; }
			return    ;
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
	return ;
}


void pPanel::get_home( uint & row, uint & col )
{
	// Return the physical position on the screen

	if ( fieldList.find( Home ) == fieldList.end() )
	{
		row = 0 ;
		col = 0 ;
	}
	else
	{
		row = fieldList[ Home ]->field_row ;
		col = fieldList[ Home ]->field_col ;
	}
	row = row + win_row ;
	col = col + win_col ;
}


string pPanel::field_getvalue( string f_name )
{
	return  fieldList[ f_name ]->field_value ;
}


void pPanel::field_setvalue( string f_name, string f_value )
{
	if ( f_value.size() > fieldList[ f_name ]->field_length ) { f_value.substr( 0, f_value.size() - 1 ) ; }
	fieldList[ f_name ]->field_value   = f_value ;
	fieldList[ f_name ]->field_changed = true    ;
	fieldList[ f_name ]->display_field( win )    ;
}


string pPanel::cmd_getvalue()
{
	return  fieldList[ CMDfield ]->field_value ;
}


void pPanel::cmd_setvalue( string f_value )
{
	if ( f_value.size() > fieldList[ CMDfield ]->field_length ) { f_value.substr( 0, f_value.size() - 1 ) ; }
	fieldList[ CMDfield ]->field_value   = f_value ;
	fieldList[ CMDfield ]->field_changed = true    ;
	fieldList[ CMDfield ]->display_field( win )    ;
}


string pPanel::field_getname( uint row, uint col )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field

	map<string, field *>::iterator it ;

	row = row - win_row ;
	col = col - win_col ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( (it->second->field_row == row) && (col >=it->second->field_col) && (col < (it->second->field_col + it->second->field_length )) )
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

	if ( !it->second->field_active ) return false ;

	row = it->second->field_row + win_row ;
	col = it->second->field_col + win_col ;
	return  true ;
}


fieldExc pPanel::field_getexec( string field )
{
	// If passed field is in the field execute table, return the structure fieldExc for that field as defined in )FIELD panel section.

	fieldExc t ;

	if ( fieldExcTable.find( field ) == fieldExcTable.end() ) { return t ; }
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

	row   = row - win_row ;
	col   = col - win_col ;
	p_row = row ;
	p_col = col ;

	prot = true ;
	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( (it->second->field_row == row) && (col >=it->second->field_col) && (col < (it->second->field_col + it->second->field_length )) )
		{
			if ( !it->second->field_active ) return ;
			if (  it->second->field_numeric && !isdigit( ch ) )   { return ; }
			if ( !it->second->field_dynArea && !it->second->field_input ) return ;
			if (  it->second->field_dynArea && !it->second->field_dyna_input( col ) ) { return ; }
			if ( !it->second->edit_field_insert( win, ch, col, Isrt ) ) { return ; }
			prot = false ;
			++p_col ;
			if ( (p_col == it->second->field_col + it->second->field_length) & (it->second->field_skip) )
			{
				field_tab_next( p_row, p_col ) ;
			}
			wrefresh( win ) ;
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

	map<string, field *>::iterator it;

	trow = row - win_row ;
	tcol = col - win_col ;

	prot = true ;
	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( (it->second->field_row == trow) && (tcol >it->second->field_col) && (tcol < (it->second->field_col + it->second->field_length )) )
		{
			if ( !it->second->field_active ) return ;
			if ( !it->second->field_dynArea && !it->second->field_input ) return ;
			if (  it->second->field_dynArea && !it->second->field_dyna_input( tcol ) ) return ;
			tcol = fieldList[ it->first ]->edit_field_backspace( win, tcol ) ;
			prot = false ;
			row  = trow + win_row ;
			col  = tcol + win_col ;
			wrefresh( win ) ;
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

	trow = row - win_row ;
	tcol = col - win_col ;

	prot = true ;
	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( (it->second->field_row == trow) && (tcol >=it->second->field_col) && (tcol < (it->second->field_col + it->second->field_length )) )
		{
			if ( !it->second->field_active ) return ;
			if ( !it->second->field_dynArea && !it->second->field_input ) return ;
			if (  it->second->field_dynArea && !it->second->field_dyna_input( tcol ) ) return ;
			fieldList[ it->first ]->edit_field_delete( win, tcol ) ;
			prot = false ;
			row  = trow + win_row ;
			col  = tcol + win_col ;
			wrefresh( win ) ;
			return ;
		}
	}
}


void pPanel::field_erase_eof( uint row, uint col, bool & prot )
{
	// Passed row/col is the physical position on the screen.  Adjust by the window offsets to find field

	map<string, field *>::iterator it;

	row = row - win_row ;
	col = col - win_col ;

	prot = true ;
	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( (it->second->field_row == row) && (col >=it->second->field_col) && (col < (it->second->field_col + it->second->field_length )) )
		{
			if ( !it->second->field_active ) return ;
			if ( !it->second->field_dynArea && !it->second->field_input ) return ;
			if (  it->second->field_dynArea && !it->second->field_dyna_input( col ) ) return ;
			fieldList[ it->first ]->field_erase_eof( win, col ) ;
			prot = false ;
			wrefresh( win ) ;
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
		if ( (it->second->field_row == trow) && (tcol >=it->second->field_col) &&  (tcol < (it->second->field_col + it->second->field_length )) )
		{
			if ( !it->second->field_active ) return ;
			if ( !it->second->field_dynArea && !it->second->field_input ) return ;
			if (  it->second->field_dynArea && !it->second->field_dyna_input( tcol ) ) { return ; }
			col = fieldList[ it->first ]->end_of_field( win, tcol ) + win_col ;
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
		if ( it->second->field_dynArea && it->second->field_dynDataInsp )
		{
			d_offset = it->second->field_dyna_input_offset( 0 ) ;
			if ( d_offset == -1 ) { continue ; }
		}
		t_offset = it->second->field_row * WSCRMAXW + it->second->field_col + d_offset ;
		if ( (t_offset > c_offset) & (t_offset < m_offset) )
		{
			m_offset = t_offset ;
			row      = it->second->field_row + win_row ;
			col      = it->second->field_col + d_offset + win_col ;
			cursor_moved = true ;
		}
	}
	if ( !cursor_moved  ) { get_home( row, col ) ; }
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
	uint o_row    ;
	uint o_col    ;
	uint trow    ;
	uint tcol    ;

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
		if ( it->second->field_dynArea && it->second->field_dynDataInsp )
		{
			if ( o_row == it->second->field_row ) { d_offset = it->second->field_dyna_input_offset( o_col ) ; }
			else                                  { d_offset = it->second->field_dyna_input_offset( 0 )     ; }
			if ( d_offset == -1 ) { continue ; }
		}
		t_offset = it->second->field_row * WSCRMAXW + it->second->field_col + d_offset ;
		if ( (t_offset > c_offset) & (t_offset < m_offset) )
		{
			m_offset = t_offset ;
			row      = it->second->field_row + win_row ;
			col      = it->second->field_col + d_offset + win_col ;
			cursor_moved = true ;
		}
	}
	if ( !cursor_moved ) { get_home( row, col ) ; }
}


string pPanel::get_field_help( string fld )
{
	if ( fieldHList.find( fld ) == fieldHList.end() ) { return "" ; }
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
		if ( (it->second->field_tb) && it->second->field_changed )
		{
			URID = p_funcPOOL->get( RC, 0, "ZURID." + d2ds( it->second->field_row - tb_row ), NOCHECK ) ;
			tb_linesChanged[ it->second->field_row - tb_row ] = URID ;
		}
	}
	p_funcPOOL->put( RC, 0, "ZTDSELS", tb_linesChanged.size() ) ;
}


bool pPanel::tb_lineChanged( int & ln, string & URID )
{
	//  Retrieve the next changed line on the tbdispl.  Return screen line number and URID of the table record
	//  Don't remove the pair from the list but update ZTDSELS

	map< int, string >::iterator it ;

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


void pPanel::clear_tb_linesChanged( int & RC )
{
	//  Clear all stored changed lines on a tbdispl with panel name and set ZTDSELS to zero

	RC = 0 ;

	tb_linesChanged.clear() ;
	p_funcPOOL->put( RC, 0, "ZTDSELS", 0 ) ;
}


void pPanel::remove_tb_lineChanged( int & RC )
{
	//  Remove the processed line from the list of changed lines

	RC = 0 ;
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

	posn  = "" ;
	if ( top <= rows )
	{
		posn = "Row " + d2ds( top ) + " of " + d2ds( rows ) ;
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
			s = word( tb_fields, j ) + "." + d2ds( i ) ;
			if ( i < size ) fieldList[ s ]->field_active = true  ;
			else            fieldList[ s ]->field_active = false ;
		}
	}
}



string pPanel::return_command( string opt )
{
	if ( commandTable.find( opt ) == commandTable.end() ) return "" ;
	return commandTable.at( opt ) ;
}


bool pPanel::display_pd( uint row, uint col )
{
	int i ;

	hide_pd() ;
	if ( row == 0 )
	{
		for ( i = 0 ; i < ab.size() ; i++ )
		{
			if ( (col >= ab.at(i).abc_col) && (col < (ab.at(i).abc_col + ab.at(i).abc_name.size()) ) )
			{
				ab.at(i).display_abc_sel( win ) ;
				ab.at(i).display_pd() ;
				abActive = true ;
				abIndex  = i ;
				p_col    = ab.at(i).abc_col + 2 ;
				p_row    = 2 ;
				return true ;
			}
		}
	}
	return false ;
}


void pPanel::display_pd()
{
	if ( !abActive ) return   ;
	ab.at( abIndex ).display_pd() ;
}


bool pPanel::is_pd_displayed()
{
	return abActive ;
}


void pPanel::display_pd_next()
{
	if ( !abActive ) return   ;
	if ( ++abIndex == ab.size() ) { abIndex = 0 ; }
	ab.at( abIndex ).display_abc_sel( win ) ;
	ab.at( abIndex ).display_pd() ;
	p_col = ab.at( abIndex ).abc_col + 2 ;
	p_row = 2 ;
}


void pPanel::hide_pd()
{
	if ( !abActive ) return  ;
	ab.at( abIndex ).hide_pd() ;
	ab.at( abIndex ).display_abc_unsel( win ) ;
}


pdc pPanel::retrieve_pdc( int row, int col )
{
	pdc t_pdc ;

	if ( !abActive ) return t_pdc ;
	ab.at( abIndex ).hide_pd() ;
	ab.at( abIndex ).display_abc_unsel( win ) ;
	abActive = false ;
	return ab.at( abIndex ).retrieve_pdc( row, col ) ;
}


void pPanel::display_boxes()
{
	vector<Box *>::iterator it ;

	for ( it = boxes.begin() ; it != boxes.end() ; it++ )
	{
		(*it)->display_box( win ) ;
	}
}


void pPanel::set_msg( string smsg, string lmsg, cuaType msgtype, bool msgalrm )
{
	SMSG    = smsg    ;
	LMSG    = lmsg    ;
	MSGTYPE = msgtype ;
	MSGALRM = msgalrm ;
}


void pPanel::clear_msg()
{
	SMSG = "" ;
	LMSG = "" ;
}


void pPanel::put_keylist( int entry, string keyv )
{
	Keylistl[ entry ] = keyv ;
}


string pPanel::get_keylist( int entry )
{
	if ( KEYLISTN == ""  || Keylistl.find( entry ) == Keylistl.end() ) { return "" ; }
	return Keylistl[ entry ] ;
}


void pPanel::display_MSG()
{
	if ( SMSG != "" )
	{
		debug1( "Selecting SMSG to display " << SMSG << endl ) ;
		wattrset( win, cuaAttr[ MSGTYPE ] ) ;
		mvwaddstr( win, 1, ( WSCRMAXW - SMSG.size()), SMSG.c_str() ) ;
		wattroff( win, cuaAttr[ MSGTYPE ] ) ;
		if ( !showLMSG && MSGALRM )
		{
			beep() ;
			MSGALRM = false ;
		}
	}
	if ( LMSG != "" && showLMSG )
	{
		debug1( "Selecting LMSG to display " << LMSG << endl ) ;
		wattrset( win, cuaAttr[ MSGTYPE ] )  ;
		mvwaddstr( win, 4, 1, LMSG.c_str() ) ;
		wattroff( win, cuaAttr[ MSGTYPE ] )  ;
		showLMSG = false               ;
	}
}


void pPanel::get_panel_info( int & RC, string a_name, string t_name, string w_name, string d_name, string r_name, string c_name )
{
	RC = 0 ;
	if ( dynAreaList.find( a_name ) == dynAreaList.end() )
	{
		log( "E", "PQUERY.  DYNAMIC AREA " << a_name << " not found" << endl ) ;
		RC = 8 ;
		return ;
	}

	if ( t_name != "" ) { p_funcPOOL->put( RC, 0, t_name, "DYNAMIC" ) ; }
	if ( w_name != "" ) { p_funcPOOL->put( RC, 0, w_name, dynAreaList[ a_name ]->dynArea_width ) ; }
	if ( d_name != "" ) { p_funcPOOL->put( RC, 0, d_name, dynAreaList[ a_name ]->dynArea_depth ) ; }
	if ( r_name != "" ) { p_funcPOOL->put( RC, 0, r_name, dynAreaList[ a_name ]->dynArea_row )   ; }
	if ( c_name != "" ) { p_funcPOOL->put( RC, 0, c_name, dynAreaList[ a_name ]->dynArea_col )   ; }
}


void pPanel::attr( int & RC, string field, string attrs )
{
	RC = 0 ;
	if ( fieldList.find( field ) == fieldList.end() )
	{
		log( "E", "ATTR.  Field " << field << "not found" << endl ) ;
		RC = 8  ;
	}
	else { RC = fieldList[ field ]->field_attr( attrs ) ; }
}


