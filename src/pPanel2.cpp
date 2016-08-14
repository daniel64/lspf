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
#define MOD_NAME pPanel2
#define LOGOUT   aplog


int pPanel::loadPanel( string p_name, string paths )
{
	int i, j, k          ;
	int p1, p2           ;
	int pVersion         ;
	int pFormat          ;
	int tbfield_col      ;
	int tbfield_sz       ;

	string ww, w1, w2    ;
	string w3, w4, w5    ;
	string w6, w7, ws    ;
	string t1, t2        ;
	string rest          ;
	string filename      ;
	string pline         ;
	string fld, hlp      ;

	bool body(false)     ;
	bool comment(false)  ;
	bool command(false)  ;
	bool init(false)     ;
	bool reinit(false)   ;
	bool proc(false)     ;
	bool help(false)     ;
	bool ispnts(false)   ;
	bool isfield(false)  ;
	bool found           ;

	cuaType fType        ;
	std::ifstream panl   ;
	std::ifstream pincl  ;

	const string e1("Error in )BODY statement for panel ") ;

	vector< string > pSource      ;
	vector< string >::iterator it ;

	map<string, field *>::iterator it1;

	RC = 0 ;
	if ( !isvalidName( p_name ) ) { PERR = "Panel name "+ p_name +" is invalid" ; return 20 ; }

	found = false ;
	i = getpaths( paths ) ;
	for ( j = 1 ; j <= i ; j++ )
	{
		filename = getpath( paths, j ) + p_name ;
		if ( exists( filename ) )
		{
			if ( !is_regular_file( filename ) )
			{
				PERR = "Panel file " + filename + " is not a regular file" ;
				return  20 ;
			}
			else
			{
				found = true ;
				break        ;
			}
		}
	}
	if ( !found ) { PERR = "Panel file not found for "+ p_name ; return  12 ; }

	debug1( "Loading panel "+ p_name +" from "+ filename << endl ) ;

	panl.open( filename.c_str() ) ;
	if ( !panl.is_open() )
	{
		PERR = "Error opening panel file "+ filename ;
		return 20 ;
	}
	while ( getline( panl, pline ) )
	{
		if ( pline.find_first_not_of( ' ' ) == string::npos ) { continue ; }
		w1 = upper( word( pline, 1 ) ) ;
		w2 = word( pline, 2 ) ;
		if ( substr( w1, 1, 2 ) == "--" || w1[ 0 ] == '#' ) { continue ; }
		if ( w1 == ")END" )        { break                      ; }
		if ( w1 == ")COMMENT" )    { comment = true  ; continue ; }
		if ( w1 == ")ENDCOMMENT" ) { comment = false ; continue ; }
		if ( comment )             { continue                   ; }
		if ( w1 == ")INCLUDE" )
		{
			found = false ;
			i = getpaths( paths ) ;
			for ( j = 1 ; j <= i ; j++ )
			{
				filename = getpath( paths, j ) + w2 ;
				if ( exists( filename ) )
				{
					if ( !is_regular_file( filename ) )
					{
						PERR = "Panel INCLUDE file "+ filename +" is not a regular file" ;
						return  20 ;
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
				PERR = "Panel INCLUDE file "+ w2 +" not found for " + p_name ; return  12 ;
			}
			debug1( "Loading panel INCLUDE "+ w2 +" from "+ filename << endl ) ;
			pincl.open( filename.c_str() ) ;
			if ( !pincl.is_open() )
			{
				PERR = "Error opening INCLUDE file "+ filename ;
				return 20 ;
			}
			while ( getline( pincl, pline ) )
			{
				if ( pline.find_first_not_of( ' ' ) == string::npos ) { continue ; }
				w1 = word( pline, 1 ) ;
				if ( substr( w1, 1, 2 ) == "--" || w1[ 0 ] == '#' ) { continue ; }
				if ( w1 == ")END" )        { break                      ; }
				if ( w1 == ")COMMENT" )    { comment = true  ; continue ; }
				if ( w1 == ")ENDCOMMENT" ) { comment = false ; continue ; }
				if ( comment )             { continue                   ; }
				pSource.push_back( pline ) ;
			}
			if ( pincl.bad() )
			{
				pincl.close() ;
				PERR = "Error while reading INCLUDE file "+ filename ;
				return 20 ;
			}
			pincl.close() ;
			continue      ;
		}
		pSource.push_back( pline ) ;
	}
	if ( panl.bad() )
	{
		panl.close() ;
		PERR = "Error while reading panel file "+ filename ;
		return 20 ;
	}
	panl.close() ;

	tbfield_col = 0 ;
	tbfield_sz  = 0 ;

	for ( it = pSource.begin() ; it != pSource.end() ; it++ )
	{
		pline = *it ;
		p1 = pline.find( "/*" ) ;
		if ( p1 != string::npos )
		{
			p2 = pline.find( "*/" ) ;
			if ( p2 != string::npos ) { pline.replace( p1, p2-p1+2, p2-p1+2, ' ' ) ; }
		}
		if ( pline.find( '\t' ) != string::npos )
		{
			PERR = "Tabs not allowed in panel source" ;
			return 20 ;
		}
		w1 = upper( word( pline, 1 ) ) ;
		if ( w1[ 0 ] == ')' )
		{
			body    = false ;
			command = false ;
			init    = false ;
			reinit  = false ;
			proc    = false ;
			help    = false ;
			ispnts  = false ;
			isfield = false ;
		}
		w2 = word( pline, 2 ) ;
		if ( w1 == ")PANEL" )
		{
			i = pos( " VERSION=", pline ) ;
			j = pos( " FORMAT=", pline )  ;
			if ( i == 0 || j == 0 )
			{
				PERR = "Invalid )PANEL statement in panel "+ p_name ; return 20 ;
			}
			pVersion = ds2d( word( substr( pline, i+9 ), 1 ) ) ;
			pFormat  = ds2d( word( substr( pline, j+8 ), 1 ) ) ;
			log( "I", "Panel format " << pFormat << " Panel version " << pVersion << endl ) ;
			i = pos( " KEYLIST(", pline ) ;
			if ( i > 0 )
			{
				j = pos( ",", pline, i ) ;
				k = pos( ")", pline, i ) ;
				if ( j == 0 || k == 0 || j > k )
				{
					PERR = "Invalid )PANEL statement in panel " + p_name ; return 20 ;
				}
				KEYLISTN = strip( pline.substr( i+8, j-i-9 ) ) ;
				KEYAPPL  = strip( pline.substr( j, k-j-1 ) )   ;
				if ( !isvalidName( KEYLISTN ) || !isvalidName4( KEYAPPL ) )
				{
					PERR = "Invalid Keylist parameters on )PANEL statement in panel "+ p_name ; return 20 ;
				}
			}
			continue ;
		}
		if ( w1 == ")BODY" )
		{
			j = pos( " WINDOW(", pline ) ;
			if ( j > 0 )
			{
				k  = pos( ")", pline, j ) ;
				if ( k == 0 ) { PERR = e1 + p_name ; return 20 ; }
				ws = substr( pline, j+8, k-j-8 ) ;
				j  = ws.find( ',' )       ;
				if ( j == string::npos ) { PERR = e1 + p_name ; return 20 ; }
				t1 = strip( ws.substr( 0, j) ) ;
				t2 = strip( ws.substr( j+1) )  ;
				win_width = ds2d( t1 ) ;
				win_depth = ds2d( t2 ) ;
				win  = newwin( win_depth, win_width, win_row, win_col ) ;
				bwin = newwin( win_depth+2, win_width+2, win_row, win_col ) ;
				WSCRMAXW = win_width   ;
				WSCRMAXD = win_depth   ;
			}
			else
			{
				win   = newwin( WSCRMAXD, WSCRMAXW, 0, 0 ) ;
				panel = new_panel( win ) ;
				set_panel_userptr( panel, new panel_data( ZSCRNUM ) ) ;
			}
			j = pos( " CMD(", pline ) ;
			if ( j > 0 )
			{
				k  = pos( ")", pline, j ) ;
				if ( k == 0 ) { PERR = e1 + p_name ; return 20 ; }
				CMDfield = strip( substr( pline, j+5, k-j-5 ) ) ;
				if ( !isvalidName( CMDfield ) ) { PERR = "Error creating command field "+ CMDfield +" for panel "+ p_name ; return 20 ; }
			}
			j = pos( " HOME(", pline ) ;
			if ( j > 0 )
			{
				k  = pos( ")", pline, j ) ;
				if ( k == 0 ) { PERR = e1 + p_name ; return 20 ; }
				Home = strip( substr( pline, j+6, k-j-6 ) ) ;
				if ( !isvalidName( Home ) ) { PERR = "Error creating home field "+ Home +" for panel "+ p_name ; return 20 ; }
			}
			body = true  ;
			continue ;
		}
		else if ( w1 == ")PROC" )    { proc    = true ; continue ; }
		else if ( w1 == ")INIT" )    { init    = true ; continue ; }
		else if ( w1 == ")REINIT" )  { reinit  = true ; continue ; }
		else if ( w1 == ")COMMAND" ) { command = true ; continue ; }
		else if ( w1 == ")HELP" )    { help    = true ; continue ; }
		else if ( w1 == ")PNTS" )    { ispnts  = true ; continue ; }
		else if ( w1 == ")FIELD" )   { isfield = true ; continue ; }

		if ( command )
		{
			w2 = strip( subword( pline, 2 ), 'B', '"' ) ;
			commandTable[ w1 ] = w2 ;
			debug2( "Adding command "+ w1 +" options "+ w2 << endl ) ;
			continue ;
		}

		if ( init || reinit || proc )
		{
			if ( w1 == "VGET" || w1 == "VPUT" )
			{
				panstmnt m_stmnt ;
				VPUTGET m_VPG( pline ) ;
				if ( m_VPG.vpg_RC != 0 )
				{
					PERR = "Error in VPUT or VGET statement "+ strip( pline ) ; return 20 ;
				}
				m_stmnt.ps_vputget = true ;
				m_stmnt.ps_column  = pline.find_first_not_of( ' ' ) ;
				if ( init )         { vpgListi.push_back( m_VPG ) ; initstmnts.push_back( m_stmnt ) ; }
				else if ( reinit )  { vpgListr.push_back( m_VPG ) ; reinstmnts.push_back( m_stmnt ) ; }
				else                { vpgList.push_back( m_VPG )  ; procstmnts.push_back( m_stmnt ) ; }
				continue ;
			}
			if ( (w1[ 0 ] == '&' || w1[ 0 ] == '.' ) && ( pline.find( "TRUNC(" ) == string::npos && pline.find( "TRANS(" ) == string::npos ) )
			{
				ASSGN m_assgn( pline ) ;
				panstmnt m_stmnt       ;
				if ( m_assgn.as_RC != 0 )
				{
					PERR = "Error in assignment statement "+ strip( pline ) ; return 20 ;
				}
				if ( m_assgn.as_isattr && ( fieldList.find( m_assgn.as_lhs ) == fieldList.end() ) )
				{
					if ( wordpos( m_assgn.as_lhs, tb_fields ) == 0 )
					{
						PERR = "Error in .ATTR statement. Field "+ m_assgn.as_lhs +" not found" ; return 20 ;
					}
					else
					{
						m_assgn.as_istb = true ;
					}
				}
				m_stmnt.ps_assign = true ;
				m_stmnt.ps_column = pline.find_first_not_of( ' ' ) ;
				if ( init )         { assgnListi.push_back( m_assgn ) ; initstmnts.push_back( m_stmnt ) ; }
				else if ( reinit )  { assgnListr.push_back( m_assgn ) ; reinstmnts.push_back( m_stmnt ) ; }
				else                { assgnList.push_back( m_assgn )  ; procstmnts.push_back( m_stmnt ) ; }
				continue ;
			}
		}

		if ( proc )
		{
			if ( w1 == "IF" )
			{
				IFSTMNT m_if( pline ) ;
				panstmnt m_stmnt      ;
				if ( m_if.if_RC != 0 )
				{
					PERR = "Error in IF statement "+ strip( pline ) ; return 20 ;
				}
				if ( wordpos( m_if.if_lhs, tb_fields ) > 0 ) { m_if.if_istb = true ; }
				ifList.push_back( m_if ) ;
				m_stmnt.ps_if     = true ;
				m_stmnt.ps_column = pline.find_first_not_of( ' ' ) ;
				procstmnts.push_back( m_stmnt ) ;
				continue ;
			}
			if ( w1 == "ELSE" && w2 == "" )
			{
				IFSTMNT m_if        ;
				m_if.if_else = true ;
				panstmnt m_stmnt    ;
				j                 = 0     ;
				found             = false ;
				m_stmnt.ps_else   = true  ;
				m_stmnt.ps_column = pline.find_first_not_of( ' ' ) ;
				for ( i = procstmnts.size()-1 ; i >= 0 ; i-- )
				{
					if ( procstmnts.at( i ).ps_if || ( procstmnts.at( i ).ps_else ) ) { j++ ; }
					if ( procstmnts.at( i ).ps_if && (procstmnts.at( i ).ps_column == m_stmnt.ps_column) )
					{
						m_if.if_stmnt = ifList.size()-j ;
						ifList.push_back( m_if ) ;
						procstmnts.push_back( m_stmnt ) ;
						found = true ;
						break        ;
					}
					if ( procstmnts.at( i ).ps_column <= m_stmnt.ps_column ) { break ; }
				}
				if ( !found )
				{
					PERR = "No matching IF statement found for ELSE at column "+ d2ds( m_stmnt.ps_column+1 ) ;
					return 20 ;
				}
				continue ;
			}
			if ( w1 == "EXIT" && w2 == "" )
			{
				panstmnt m_stmnt ;
				m_stmnt.ps_exit = true ;
				m_stmnt.ps_column = pline.find_first_not_of( ' ' ) ;
				procstmnts.push_back( m_stmnt ) ;
				continue ;
			}
			if ( w1 == "VER" )
			{
				VERIFY m_VER( pline ) ;
				panstmnt m_stmnt     ;
				if ( m_VER.ver_RC != 0 )
				{
					PERR = "Error in VER statement "+ strip( pline ) ; return 20 ;
				}
				if ( wordpos( m_VER.ver_field, tb_fields ) > 0 )
				{
					m_VER.ver_tbfield = true ;
				}
				else if ( fieldList.find( m_VER.ver_field ) == fieldList.end() )
				{
					PERR = "Error in VER statement. Field "+ m_VER.ver_field +" not found" ; return 20 ;
				}
				verList.push_back( m_VER ) ;
				m_stmnt.ps_verify = true   ;
				m_stmnt.ps_column = pline.find_first_not_of( ' ' ) ;
				procstmnts.push_back( m_stmnt ) ;
				continue ;
			}
			if ( pline.find( "TRUNC(" ) != string::npos )
			{
				TRUNC m_trunc( pline ) ;
				panstmnt m_stmnt      ;
				if ( m_trunc.trnc_RC != 0 )
				{
					PERR = "Error in TRUNC statement "+ strip( pline ) ; return 20 ;
				}
				truncList.push_back( m_trunc ) ;
				m_stmnt.ps_trunc  = true       ;
				m_stmnt.ps_column = pline.find_first_not_of( ' ' ) ;
				procstmnts.push_back( m_stmnt ) ;
				continue ;
			}
			if ( pline.find( "TRANS(" ) != string::npos )
			{
				TRANS m_trans( pline ) ;
				panstmnt m_stmnt      ;
				if ( m_trans.trns_RC != 0 )
				{
					PERR = "Error in TRANS statement "+ strip( pline ) ; return 20 ;
				}
				transList.push_back( m_trans ) ;
				m_stmnt.ps_trans  = true       ;
				m_stmnt.ps_column = pline.find_first_not_of( ' ' ) ;
				procstmnts.push_back( m_stmnt ) ;
				continue ;
			}
		}

		if ( help )
		{
			i = pos( " FIELD(", " " + pline ) ;
			if ( i > 0 ) { j = pos( ")", pline, i ) ; }
			if ( i == 0 || j == 0 )
			{
				PERR = "Invalid FIELD help entry in )HELP section.  Missing bracket" ; return 20 ;
			}
			fld = strip( substr( pline, i+6, j-i-6 ) ) ;
			if ( !isvalidName( fld ) )
			{
				PERR = "Invalid HELP entry field name "+ fld ; return 20 ;
			}
			if ( fieldList.find( fld ) == fieldList.end() )
			{
				PERR = "Invalid HELP statement.  Field "+ fld +" does not exist" ; return 20 ;
			}

			i = pos( " HELP(", pline ) ;
			if ( i > 0 ) { j = pos( ")", pline, i ) ; }
			if ( i == 0 || j == 0 )
			{
				PERR = "Invalid FIELD help entry in )HELP section.  Missing bracket" ; return 20 ;
			}
			hlp = strip( substr( pline, i+6, j-i-6 ) ) ;
			if ( !isvalidName( hlp ) )
			{
				PERR = "Invalid HELP entry name "+ hlp ; return 20 ;
			}
			fieldHList[ fld ] = hlp ;
			continue ;
		}

		if ( ispnts )
		{
			pnts m_pnts( pline ) ;
			if ( m_pnts.pnts_RC == 0 )
			{
				if ( fieldList.find( m_pnts.pnts_field ) == fieldList.end() )
				{
					found = false ;
					for ( uint i = 0 ; i < literalList.size() ; i++ )
					{
						if ( m_pnts.pnts_field == literalList.at( i )->literal_name )
						{
							found = true ;
							break        ;
						}
					}
					if ( !found )
					{
						PERR = "Field "+ m_pnts.pnts_field +" not found in panel" ;
						return  20 ;
					}
				}
				if ( fieldList.find( m_pnts.pnts_var ) == fieldList.end() )
				{
					PERR = "Variable "+ m_pnts.pnts_var +" not found in panel" ;
					return  20 ;
				}
				pntsTable[ m_pnts.pnts_field ] = m_pnts ;
			}
			else
			{
				PERR = "Error parsing point-and-shoot line "+ pline ;
				return 20 ;
			}
			continue ;
		}

		if ( isfield )
		{
			fieldExc t_fe ;
			i = pos( "FIELD(", pline ) ;
			if ( i > 0 )
			{
				j = pos( ")", pline, i ) ;
				if ( j > 0 )
				{
					t1 = strip( substr( pline, i+6, j-i-6 ) ) ;
					if ( fieldList.find( t1 ) == fieldList.end() )
					{
						PERR = "Field "+ t1 +" not found on panel" ; return 20 ;
					}
				}
			}
			if ( i == 0 || j == 0 )
			{
				PERR = "Invalid format of FIELD() definition. " + pline ;
				return 20 ;
			}
			i = pos( "EXEC('", pline ) ;
			if ( i > 0 )
			{
				j = pos( "')", pline, i ) ;
				if ( j > 0 )
				{
					t2 = strip( substr( pline, i+6, j-i-6 ) ) ;
					t_fe.fieldExc_command = t2 ;
				}
			}
			if ( i == 0 || j == 0 )
			{
				PERR = "Invalid format of EXEC() definition. "+ pline ;
				return 20 ;
			}
			i = pos( "PASS(", pline ) ;
			if ( i > 0 )
			{
				j = pos( ")", pline, i ) ;
				if ( j > 0 )
				{
					t2 = strip( substr( pline, i+5, j-i-5 ) ) ;
					t_fe.fieldExc_passed = t2 ;
				}
			}
			for ( j = words( t_fe.fieldExc_passed ), i = 1 ; i <= j ; i++ )
			{
				ww = word( t_fe.fieldExc_passed, i ) ;
				if ( fieldList.find( ww ) == fieldList.end() )
				{
					PERR = "Field "+ ww +" passed on field command for "+ t1 +" is not defined in panel body" ; return 20 ;
				}
			}
			if ( fieldExcTable.find( t1 ) != fieldExcTable.end() )
			{
				PERR = "Duplicate field command entry in )FIELD panel section for "+ t1 ; return 20 ;
			}
			fieldExcTable[ t1 ] = t_fe ;
			continue ;
		}

		if ( !body )
		{
			PERR = "Panel " + p_name + " error.  Invalid line: "+ pline ; return 20 ;
		}

		else if ( w1 == "PANELTITLE" )
		{
			PanelTitle = strip( strip( subword( pline, 2 ) ), 'B', '"' ) ;
			continue ;
		}

		else if ( w1 == "PRIMARYMENU" )
		{
			debug1( "Creating a primary options menu" << endl ) ;
			primaryMenu = true ;
			continue ;
		}

		else if ( w1 == "SCROLLON" )
		{
			scrollOn = true ;
			continue ;
		}

		else if ( w1 == "LITERAL" )
		{
			literal * m_lit = new literal ;
			RC = m_lit->literal_init( WSCRMAXW, WSCRMAXD, opt_field, pline ) ;
			if ( RC > 0 ) { PERR = "Error creating literal for panel "+ p_name ; delete m_lit ; return 20 ; }
			literalList.push_back( m_lit ) ;
			continue ;
		}
		else if ( w1 == "FIELD" )
		{
			w7 = word( pline, 7 ) ;
			if ( !isvalidName( w7 ) )
			{
				PERR = "Invalid field name >>"+ w7 +"<< on line: " + pline ; return 20 ;
			}

			if ( fieldList.find( w7 ) != fieldList.end() )
			{
				PERR = "Field "+ w7 +" already exists on panel" ; return  20 ;
			}

			field * m_fld = new field ;
			RC = m_fld->field_init( WSCRMAXW, WSCRMAXD, pline ) ;
			if ( RC > 0 ) { PERR = "Error creating field for panel "+ p_name ; delete m_fld ; return 20 ; }  ;
			fieldList[ w7 ] = m_fld  ;
			continue ;
		}
		else if ( w1 == "DYNAREA" )
		{
			debug2( "Creating dynArea" << endl ) ;
			w6 = word( pline, 6 ) ;
			if ( !isvalidName( w6 ) )
			{
				PERR = "Invalid field name "+ w6 +" entered for dynamic area" ; return  20 ;
			}

			dynArea * m_dynArea = new dynArea ;
			RC = m_dynArea->dynArea_init( WSCRMAXW, WSCRMAXD, pline ) ;
			if ( RC > 0 ) { PERR = "Error creating dynArea for panel "+ p_name ; delete m_dynArea ; return 20 ; }

			dyn_width = m_dynArea->dynArea_width ;
			dyn_depth = m_dynArea->dynArea_depth ;

			dynAreaList[ w6 ] = m_dynArea ;
			for ( i = 0 ; i < m_dynArea->dynArea_depth ; i++ )
			{
				field * m_fld            = new field ;
				m_fld->field_cua         = AB        ;
				m_fld->field_row         = m_dynArea->dynArea_row + i ;
				m_fld->field_col         = m_dynArea->dynArea_col     ;
				m_fld->field_length      = m_dynArea->dynArea_width   ;
				m_fld->field_cole        = m_dynArea->dynArea_col + m_dynArea->dynArea_width ;
				m_fld->field_dynArea     = true           ;
				m_fld->field_dynArea_ptr = m_dynArea      ;
				fieldList[ w6 + "." + d2ds( i ) ] = m_fld ;
			}
			continue ;
		}
		else if ( w1 == "BOX" )
		{
			debug2( "Creating box" << endl ) ;
			w2 = word( pline, 2 ) ;
			Box * m_box = new Box ;
			RC = m_box->box_init( WSCRMAXW, WSCRMAXD, pline ) ;
			if ( RC > 0 ) { PERR = "Error creating box for panel "+ p_name ; delete m_box ; return 20 ; }
			boxes.push_back( m_box ) ;
			continue ;
		}
		else if ( w1 == "PDC" )
		{
			debug2( "Creating pdc" << endl ) ;
			w2 = word( pline, 2 ) ;
			w3 = word( pline, 3 ) ;
			if ( w3[ 0 ] == '\"' )
			{
				p1   = pos( "\"", pline ) ;
				p2   = pos( "\"", pline, p1+1 ) ;
				w3   = substr( pline, p1+1, p2-p1-1 ) ;
				rest = substr( pline, p2+1 ) ;
			}
			else
			{
				rest = subword( pline, 4 ) ;
			}
			if ( word( rest, 1 ) != "ACTION" ) { RC = 20 ; }
			w5   = word( rest, 2)     ;
			rest = subword( rest, 3 ) ;
			if ( substr( w5, 1, 4 ) != "RUN(" ) { RC = 20 ; }
			p2   = pos( ")", w5, 5 ) ;
			w5   = substr( w5, 5, p2-5 ) ;
			w7   = "" ;
			p1 = pos ( "UNAVAIL(", rest ) ;
			if ( p1 > 0 )
			{
				p2 = pos( ")", rest, p1 ) ;
				w7 = strip( substr( rest, p1+8, p2-p1-8 )) ;
				rest = delstr( rest, p1, p2-p1+1 ) ;
			}
			w6 = "" ;
			if ( substr( rest, 1, 5 ) == "PARM(" )
			{
				p2 = lastpos( ")", rest ) ;
				w6 = strip( substr( rest, 6, p2-7 ), 'B', '\"' ) ;
			}
			if ( RC == 0 ) { create_pdc( w2, w3, w5, w6, w7 ) ; }
			if ( RC > 0 ) { PERR = "Error creating pdc for panel "+ p_name ; return 20 ; }
			continue ;
		}
		else if ( w1 == "TBMODEL" )
		{
			debug2( "Creating tbmodel" << endl ) ;
			w3 = word( pline, 3 ) ;
			int start_row = ds2d( word( pline, 2 ) ) - 1;

			if ( isnumeric( w3 ) )                   { tb_depth = ds2d( w3 ) ; }
			else if ( w3 == "MAX" )                  { tb_depth = WSCRMAXD - start_row ; }
			else if ( substr( w3, 1, 4 ) == "MAX-" ) { tb_depth = WSCRMAXD - ds2d( substr( w3, 5 ) ) - start_row ; }
			else                                     { return 20        ; }
			tb_model = true      ;
			tb_row   = start_row ;
			scrollOn = true      ;
			if ( (start_row + tb_depth ) > WSCRMAXD ) { tb_depth = (WSCRMAXD - start_row) ; }
			p_funcPOOL->put( RC, 0, "ZTDDEPTH", tb_depth ) ;
			continue ;
		}
		else if ( w1 == "TBFIELD" )
		{
			int tlen ;
			int tcol ;
			w3 = word( pline, 3 ) ;
			if ( w2[ 0 ] == '+' )
			{
				if ( w2[ 1 ] == '+' )
				{
					tcol = tbfield_col + tbfield_sz + ds2d( substr( w2, 3 ) ) ;
				}
				else
				{
					tcol = tbfield_col + ds2d( substr( w2, 2 ) ) ;
				}
			}
			else
			{
				tcol = ds2d( w2 ) ;
			}
			if      ( isnumeric( w3 ) )              { tlen = ds2d( w3 ) ; }
			else if ( w3 == "MAX" )                  { tlen = WSCRMAXW - tcol + 1 ; }
			else if ( substr( w3, 1, 4 ) == "MAX-" ) { tlen = WSCRMAXW - tcol - ds2d( substr( w3, 5 ) ) + 1 ; }
			else                                     { return 20         ; }
			tbfield_col = tcol    ;
			tbfield_sz  = tlen    ;
			w4 = word( pline, 4 ) ;
			if ( cuaAttrName.find( w4 ) == cuaAttrName.end() )
			{
				PERR = "Unknown field CUA attribute type "+ w4  ; return 20 ;
			}
			fType = cuaAttrName[ w4 ] ;
			debug2( "Creating tbfield" << endl ) ;
			create_tbfield( tcol, tlen, fType, word( pline, 6 ), word( pline, 5 ) ) ;
			if ( RC > 0 ) { return 20 ; }
			continue ;
		}
		else
		{
			PERR = "Panel "+ p_name +" error.  Invalid line: "+ pline ; return 20 ;
		}
	}

	if ( scrollOn && ( fieldList.find( "ZSCROLL" ) == fieldList.end() ) )
	{
		PERR = "Field ZSCROLL not defined for scrollable panel" ; return 20 ;
	}
	if ( fieldList.find( Home ) == fieldList.end() )
	{
		PERR = "Home field "+ Home +" not defined in panel body" ; return 20 ;
	}
	if ( fieldList.find( CMDfield ) == fieldList.end() )
	{
		PERR = "Command field "+ CMDfield +" not defined in panel body" ; return 20 ;
	}

	if ( REXX )
	{
		for ( it1 = fieldList.begin() ; it1 != fieldList.end() ; it1++ )
		{
			if ( !p_funcPOOL->ifexists( RC, it1->first ) ) { syncDialogueVar( it1->first ) ; }
		}
	}

	PANELID = p_name ;
	debug1( "Panel loaded and processed successfully" << endl ) ;
	return 0 ;
}
