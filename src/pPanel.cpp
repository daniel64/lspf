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
#define MOD_NAME pPanel
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
	PanelTitle  = ""     ;
	abIndex     = 0      ;
	opt_field   = 0      ;
	tb_model    = false  ;
	tb_depth    = 0      ;
	tb_fields   = ""     ;
	dyn_depth   = 0      ;
	ZPHELP      = ""     ;
	PERR        = ""     ;
	CMDfield    = "ZCMD" ;
	Home        = "ZCMD" ;
}


pPanel::~pPanel()
{
	/* iterate over the 4 panel widget types, literal, field (inc tbfield, dynarea field), dynArea, Boxes and delete them */

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
	for ( it3 = Boxes.begin() ; it3 != Boxes.end() ; it3++ )
	{
		delete (*it3) ;
	}

}


void pPanel::init( int & RC )
{
	RC = 0  ;

	ZSCRMAXD = ds2d( p_poolMGR->get( RC, "ZSCRMAXD", SHARED ) ) ;
	if ( RC > 0 ) { PERR = "ZSCRMAXD poolMGR.get failed" ; log("C", PERR ) ; RC = 20 ; return ; }
	ZSCRMAXW = ds2d( p_poolMGR->get( RC, "ZSCRMAXW", SHARED ) ) ;
	if ( RC > 0 ) { PERR = "ZSCRMAXW poolMGR.get failed" ; log("C", PERR ) ; RC = 20 ; return ; }
}


int pPanel::loadPanel( string p_name, string paths )
{
	int i, j             ;
	int p1, p2           ;
	int pVersion         ;
	int pFormat          ;
	int tbfield_col      ;
	int tbfield_ss       ;
	int tbfield_sz       ;

	string s, w, w1, w2  ;
	string w3, w4, w5    ;
	string w6, w7        ;
	string rest          ;
	string filename, line2 ;
	string vars          ;
	string fld, hlp      ;

	bool body(false)     ;
        bool comment(false)  ;
	bool command(false)  ;
	bool init(false)     ;
	bool reinit(false)   ;
	bool proc(false)     ;
	bool help(false)     ;
	bool ispnts(false)   ;
	bool found           ;

	char line1[ 256 ]    ;
	cuaType fType        ;
	ifstream panel       ;
	ifstream pincl       ;

	vector< string > pSource      ;
	vector< string >::iterator it ;
	
	RC = 0 ;
	if ( !isvalidName( p_name ) ) { PERR = "Panel name " + p_name + " is invalid" ; return 20 ; }

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
	if ( !found ) { PERR = "Panel file not found for " + p_name ; return  12 ; }

	debug1( "Loading panel " << p_name << " from " <<  filename << endl ) ;	

	panel.open( filename.c_str() ) ;
	while ( true )
	{
		panel.getline( line1, 256 ) ;
		if ( panel.fail() != 0 ) break ;
		line2.assign( line1, panel.gcount() - 1 ) ;
		if ( line2.find_first_not_of( ' ' ) == string::npos ) continue ;
		w1 = upper( word( line2, 1 ) ) ;
		w2 = word( line2, 2 ) ;
		if ( substr( w1, 1, 2 ) == "--" ) continue ;
		if ( substr( w1, 1, 1 ) == "#" )  continue ;
		if ( w1 == ")COMMENT" )    { comment = true  ; continue ; }
		if ( w1 == ")ENDCOMMENT" ) { comment = false ; continue ; }
		if ( comment ) continue ;
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
						PERR = "Panel INCLUDE file " + filename + " is not a regular file" ;
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
				PERR = "Panel INCLUDE file " + w2 + " not found for " + p_name ; return  12 ;
			}
			debug1( "Loading panel INCLUDE " << w2 << " from " <<  filename << endl ) ;	
			pincl.open( filename.c_str() ) ;
			while ( true )
			{
				pincl.getline( line1, 256 ) ;
				if ( pincl.fail() != 0 ) break ;
				line2.assign( line1, pincl.gcount() - 1 ) ;
				if ( line2.find_first_not_of( ' ' ) == string::npos ) continue ;
				w1 = word( line2, 1 ) ;
				if ( substr( w1, 1, 2 ) == "--" ) continue ;
				if ( substr( w1, 1, 1 ) == "#" )  continue ;
				if ( w1 == ")COMMENT" )    { comment = true  ; continue ; }
				if ( w1 == ")ENDCOMMENT" ) { comment = false ; continue ; }
				if ( comment ) continue ;
				pSource.push_back( line2 ) ;
			}
			pincl.close() ;
			continue      ;
		}
		pSource.push_back( line2 ) ;
	}
	panel.close() ;

	tbfield_col = 0  ;
	tbfield_ss  = 2  ;
	tbfield_sz  = 0  ;

	for ( it = pSource.begin() ; it != pSource.end() ; it++ )
	{
		line2 = *it ;
		p1 = line2.find( "/*" ) ;
		if ( p1 != string::npos ) 
		{
			p2 = line2.find( "*/" ) ;
			if ( p2 != string::npos ) { line2.replace( p1, p2-p1+2, p2-p1+2, ' ' ) ; }
		}
		if ( line2.find( '\t' ) != string::npos )
		{
			PERR = "Tabs not allowed in panel source" ;
			return 20 ;
		}
		w1 = upper( word( line2, 1 ) ) ;
		w2 = word( line2, 2 ) ;
		if ( w1 == ")PANEL" )
		{
			i = pos( " VERSION=", line2 ) ;
			j = pos( " FORMAT=", line2 )  ;
			if ( i == 0 || j == 0 )
			{
				PERR = "Invalid )PANEL statement in panel " + p_name ; return 20 ;
			}
			pVersion = ds2d( word( substr( line2, i+9 ), 1 ) ) ;
			pFormat  = ds2d( word( substr( line2, j+8 ), 1 ) ) ;
			log( "I", "Panel format " << pFormat << " Panel version " << pVersion << endl ) ;
			continue ;
		}
		if ( w1 == ")INIT" )	   { init = true  ; continue ; }
		if ( w1 == ")ENDINIT" )    { init = false ; continue ; }

		if ( w1 == ")REINIT" )	   { reinit = true  ; continue ; }
		if ( w1 == ")ENDREINIT")   { reinit = false ; continue ; }

		if ( w1 == ")BODY" )	   { body = true  ; continue ; }
		if ( w1 == ")ENDBODY" )	   { body = false ; continue ; }

		if ( w1 == ")PROC" )	   { proc = true  ; continue ; }
		if ( w1 == ")ENDPROC" )    { proc = false ; continue ; }

		if ( w1 == ")COMMAND" )	   { command = true  ; continue ; }
		if ( w1 == ")ENDCOMMAND" ) { command = false ; continue ; }

		if ( w1 == ")HELP" )	   { help = true  ; continue ; }
		if ( w1 == ")ENDHELP" )    { help = false ; continue ; }

		if ( w1 == ")PNTS" )	   { ispnts = true  ; continue ; }
		if ( w1 == ")ENDPNTS" )    { ispnts = false ; continue ; }

		if ( command )
		{
			w2 = strip( subword( line2, 2 ), 'B', '"' ) ;
			commandTable[ w1 ] = w2 ;
			debug2( "Adding command " << w1 << " options " << w2 << endl ) ;
			continue ;
		}

		if ( init || reinit || proc )
		{
			if ( w1 == "VGET" || w1 == "VPUT" )
			{
				panstmnt m_stmnt ;
				VPUTGET m_VPG( line2 ) ;
				if ( m_VPG.vpg_RC != 0 )
				{
					PERR = "Error in VPUT or VGET statement " + strip( line2 ) ; return 20 ;
				}
				m_stmnt.ps_vputget = true ;
				m_stmnt.ps_column  = line2.find_first_not_of( ' ' ) ;
				if ( init )         { vpgListi.push_back( m_VPG ) ; initstmnts.push_back( m_stmnt ) ; }
				else if ( reinit )  { vpgListr.push_back( m_VPG ) ; reinstmnts.push_back( m_stmnt ) ; }
				else		    { vpgList.push_back( m_VPG )  ; procstmnts.push_back( m_stmnt ) ; }
				continue ;
			}
			if ( (w1[ 0 ] == '&' || w1[ 0 ] == '.' ) && ( line2.find( "TRUNC(" ) == string::npos && line2.find( "TRANS(" ) == string::npos ) )
			{
				ASSGN m_assgn( line2 ) ;
				panstmnt m_stmnt       ;
				if ( m_assgn.as_RC != 0 )
				{
					PERR = "Error in assignment statement " + strip( line2 ) ; return 20 ;
				}
				if ( m_assgn.as_isattr && ( fieldList.find( m_assgn.as_lhs ) == fieldList.end() ) )
				{
					if ( wordpos( m_assgn.as_lhs, tb_fields ) == 0 )
					{
						PERR = "Error in .ATTR statement. Field " + m_assgn.as_lhs + " not found" ; return 20 ;
					}
					else
					{
						m_assgn.as_istb = true ;
					}
				}
				m_stmnt.ps_assign = true ;
				m_stmnt.ps_column = line2.find_first_not_of( ' ' ) ;
				if ( init )         { assgnListi.push_back( m_assgn ) ; initstmnts.push_back( m_stmnt ) ; }
				else if ( reinit )  { assgnListr.push_back( m_assgn ) ; reinstmnts.push_back( m_stmnt ) ; }
				else		    { assgnList.push_back( m_assgn )  ; procstmnts.push_back( m_stmnt ) ; }
				continue ;
			}
		}

		if ( proc )
		{
			if ( w1 == "IF" )
			{
				IFSTMNT m_if( line2 ) ;
				panstmnt m_stmnt      ;
				if ( m_if.if_RC != 0 )
				{
					PERR = "Error in IF statement " + strip( line2 ) ; return 20 ;
				}
				if ( wordpos( m_if.if_lhs, tb_fields ) > 0 ) { m_if.if_istb = true ; }
				ifList.push_back( m_if ) ;
				m_stmnt.ps_if     = true ;
				m_stmnt.ps_column = line2.find_first_not_of( ' ' ) ;
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
				m_stmnt.ps_column = line2.find_first_not_of( ' ' ) ;
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
					PERR = "No matching IF statement found for ELSE at column " + d2ds( m_stmnt.ps_column+1 ) ;
					return 20 ;
				}
				continue ;
			}
			if ( w1 == "EXIT" && w2 == "" )
			{
				panstmnt m_stmnt ;
				m_stmnt.ps_exit = true ;
				m_stmnt.ps_column = line2.find_first_not_of( ' ' ) ;
				procstmnts.push_back( m_stmnt ) ;
				continue ;
			}
			if ( w1 == "VER" )
			{
				VERIFY m_VER( line2 ) ;
				panstmnt m_stmnt     ;
				if ( m_VER.ver_RC != 0 )
				{
					PERR = "Error in VER statement " + strip( line2 ) ; return 20 ;
				}
				if ( wordpos( m_VER.ver_field, tb_fields ) > 0 )
				{
					m_VER.ver_tbfield = true ;
				}
				else if ( fieldList.find( m_VER.ver_field ) == fieldList.end() )
				{
					PERR = "Error in VER statement. Field " + m_VER.ver_field + " not found" ; return 20 ;
				}
				verList.push_back( m_VER ) ;
				m_stmnt.ps_verify = true   ;
				m_stmnt.ps_column = line2.find_first_not_of( ' ' ) ;
				procstmnts.push_back( m_stmnt ) ;
				continue ;
			}
			if ( line2.find( "TRUNC(" ) != string::npos )
			{
				TRUNC m_trunc( line2 ) ;
				panstmnt m_stmnt      ;
				if ( m_trunc.trnc_RC != 0 )
				{
					PERR = "Error in TRUNC statement " + strip( line2 ) ; return 20 ;
				}
				truncList.push_back( m_trunc ) ;
				m_stmnt.ps_trunc  = true       ;
				m_stmnt.ps_column = line2.find_first_not_of( ' ' ) ;
				procstmnts.push_back( m_stmnt ) ;
				continue ;
			}
			if ( line2.find( "TRANS(" ) != string::npos )
			{
				TRANS m_trans( line2 ) ;
				panstmnt m_stmnt      ;
				if ( m_trans.trns_RC != 0 )
				{
					PERR = "Error in TRANS statement " + strip( line2 ) ; return 20 ;
				}
				transList.push_back( m_trans ) ;
				m_stmnt.ps_trans  = true       ;
				m_stmnt.ps_column = line2.find_first_not_of( ' ' ) ;
				procstmnts.push_back( m_stmnt ) ;
				continue ;
			}
		}

		if ( help )
		{
			i = pos( " FIELD(", " " + line2 ) ;
			if ( i > 0 ) { j = pos( ")", line2, i ) ; }
			if ( i == 0 || j == 0 )
			{
				PERR = "Invalid FIELD help entry in )HELP section.  Missing bracket" ; return 20 ;
			}
			fld = strip( substr( line2, i+6, j-i-6 ) ) ;
			if ( !isvalidName( fld ) )
			{
				PERR = "Invalid HELP entry field name " + fld ; return 20 ;
			}
			if ( fieldList.find( fld ) == fieldList.end() )
			{
				PERR = "Invalid HELP statement.  Field " + fld + " does not exist" ; return 20 ;
			}

			i = pos( " HELP(", line2 ) ;
			if ( i > 0 ) { j = pos( ")", line2, i ) ; }
			if ( i == 0 || j == 0 )
			{
				PERR = "Invalid FIELD help entry in )HELP section.  Missing bracket" ; return 20 ;
			}
			hlp = strip( substr( line2, i+6, j-i-6 ) ) ;
			if ( !isvalidName( hlp ) )
			{
				PERR = "Invalid HELP entry name " + hlp ; return 20 ;
			}
			fieldHList[ fld ] = hlp ;
			continue ;
		}

		if ( ispnts )
		{
			pnts m_pnts( line2 ) ;
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
						PERR = "Field " + m_pnts.pnts_field + " not found in panel" ;
						return  20 ;
					}
				}
				if ( fieldList.find( m_pnts.pnts_var ) == fieldList.end() )
				{
					PERR = "Variable " + m_pnts.pnts_var + " not found in panel" ;
					return  20 ;
				}
				pntsTable[ m_pnts.pnts_field ] = m_pnts ;
			}
			else
			{
				PERR = "Error parsing point-and-shoot line " + line2 ;
				return 20 ;
			}
			continue ;
		}

		if ( !body ) 
		{
			PERR = "Panel " + p_name + " error.  Invalid line: " + line2 ; return 20 ;
		}

		if ( w1 == "HOME" )
		{
			if ( !isvalidName( w2 ) ) { PERR = "Error creating home field " + w2 + " for panel" + p_name ; return 20 ; } ;
			Home = w2 ;
			continue  ;
		}
		else if ( w1 == "CMD" )
		{
			if ( !isvalidName( w2 ) ) { PERR = "Error creating command field " + w2 + " for panel" + p_name ; return 20 ; } ;
			CMDfield = w2 ;
			continue      ;
		}
		else if ( w1 == "PANELTITLE" )
		{
			PanelTitle = strip( strip( subword( line2, 2 ) ), 'B', '"' ) ;
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
			RC = m_lit->literal_init( ZSCRMAXW, ZSCRMAXD, opt_field, line2 ) ;
			if ( RC > 0 ) { PERR = "Error creating literal for panel " + p_name ; delete m_lit ; return 20 ; }
			literalList.push_back( m_lit ) ;
			continue ;
		}
		else if ( w1 == "FIELD" )
		{
			w7 = word( line2, 7 ) ;
			if ( !isvalidName( w7 ) )
			{
				PERR = "Invalid field name >>" + w7 + "<< on line: " + line2 ; return 20 ;
			}

			if ( fieldList.find( w7 ) != fieldList.end() )
			{
				PERR = "Field " + w7 + " already exists on panel" ; return  20 ;
			}

			field * m_fld = new field ;
			RC = m_fld->field_init( ZSCRMAXW, ZSCRMAXD, line2 ) ;
			if ( RC > 0 ) { PERR = "Error creating field for panel " + p_name ; delete m_fld ; return 20 ; } ;
			fieldList[ w7 ] = m_fld  ;
			continue ;
		}
		else if ( w1 == "DYNAREA" )
		{
			debug2( "Creating dynArea" << endl ) ;
			w6 = word( line2, 6 ) ;
			if ( !isvalidName( w6 ) )
			{
				PERR = "Invalid field name " + w6 + " entered for dynamic area" ; return  20 ;
			}

			dynArea * m_dynArea = new dynArea ;
			RC = m_dynArea->dynArea_init( ZSCRMAXW, ZSCRMAXD, line2 ) ;
			if ( RC > 0 ) { PERR = "Error creating dynArea for panel " + p_name ; delete m_dynArea ; return 20 ; } ;

			dyn_width = m_dynArea->dynArea_width ;
			dyn_depth = m_dynArea->dynArea_depth ;

			dynAreaList[ w6 ] = m_dynArea ;
			for ( i = 0 ; i < m_dynArea->dynArea_depth ; i++ )
			{
				field * m_fld             = new field ;
				m_fld->field_cua          = AB        ;
				m_fld->field_row          = m_dynArea->dynArea_row + i ;
				m_fld->field_col          = m_dynArea->dynArea_col     ;
				m_fld->field_length       = m_dynArea->dynArea_width   ;
				m_fld->field_dynArea      = true                       ;
				m_fld->field_dynDataInsp  = m_dynArea->dynArea_DataInsp  ;
				m_fld->field_dynDataOutsp = m_dynArea->dynArea_DataOutsp ;
				m_fld->field_dynUserModsp = m_dynArea->dynArea_UserModsp ;
				m_fld->field_dynDataModsp = m_dynArea->dynArea_DataModsp ;
				m_fld->field_dynDataIn    = m_dynArea->dynArea_DataIn    ;
				m_fld->field_dynDataOut   = m_dynArea->dynArea_DataOut   ;
				m_fld->field_dynUserMod   = m_dynArea->dynArea_UserMod   ;
				m_fld->field_dynDataMod   = m_dynArea->dynArea_DataMod   ;
				fieldList[ w6 + "." + d2ds( i ) ] = m_fld  ;
			}
			continue ;
		}
		else if ( w1 == "BOX" )
		{
			debug2( "Creating box" << endl ) ;
			w2 = word( line2, 2 ) ;
			Box * m_box = new Box ;
			m_box->Box_init( ZSCRMAXW, ZSCRMAXD, line2 ) ;
			if ( RC > 0 ) { PERR = "Error creating box for panel " + p_name ; delete m_box ; return 20 ; } ;
			Boxes.push_back(m_box ) ;
			continue ;
		}
		else if ( w1 == "PDC" )
		{
			debug2( "Creating pdc" << endl ) ;
			w2 = word( line2, 2 ) ;
			w3 = word( line2, 3 ) ;
			if ( w3[ 0 ] == '\"' )
			{
				p1   = pos( "\"", line2 ) ;
				p2   = pos( "\"", line2, p1+1 ) ;
				w3   = substr( line2, p1+1, p2-p1-1 ) ;
				rest = substr( line2, p2+1 ) ;
			}
			else
			{
				rest = subword( line2, 4 ) ;
			}
			if ( word( rest, 1 ) != "ACTION" ) { RC = 20 ; }
			w5   = word( rest, 2)     ;
			rest = subword( rest, 3 ) ;
			if ( substr( w5, 1, 4 ) != "RUN(" ) { RC = 20 ; }
			p2   = pos( ")", w5, 5 ) ;
			w5   = substr( w5, 5, p2-5 ) ;
			w6 = "" ;
			if ( substr( rest, 1, 5 ) == "PARM(" )
			{
				p2 = lastpos( ")", rest ) ;
				w6 = strip( substr( rest, 6, p2-7 ), 'B', '\"' ) ;
			}
			if ( RC == 0 ) { create_pdc( w2, w3, w5, w6 ) ; }
			if ( RC > 0 ) { PERR = "Error creating pdc for panel " + p_name ; return 20 ; } ;
			continue ;
		}
		else if ( w1 == "TBMODEL" )
		{
			debug2( "Creating tbmodel" << endl ) ;
			w3 = word( line2, 3 ) ;
			int start_row = ds2d( word( line2, 2 ) ) - 1;

			if ( isnumeric( w3 ) )                   { tb_depth = ds2d( w3 ) ; }
			else if ( w3 == "MAX" )                  { tb_depth = ZSCRMAXD - start_row ; }
			else if ( substr( w3, 1, 4 ) == "MAX-" ) { tb_depth = ZSCRMAXD - ds2d( substr( w3, 5 ) ) - start_row ; }
			else 					 { return 20        ; }
			tb_model = true      ;
			tb_row   = start_row ;
			scrollOn = true      ;
			if ( (start_row + tb_depth ) > ZSCRMAXD ) { tb_depth = (ZSCRMAXD - start_row) ; }
			p_funcPOOL->put( RC, 0, "ZTDDEPTH", tb_depth ) ;
			continue ;
		}
		else if ( w1 == "TBFIELD" )
		{
			int tlen ;
			int tcol ;
			w3 = word( line2, 3 ) ;
			if ( upper( w2 ) == "SEPSIZE" )
			{
				tbfield_ss = ds2d( w3 ) ;
				continue ;
			}
			else if ( w2 == "+" )
			{
				tcol = tbfield_col + tbfield_sz + tbfield_ss ;
			}
			else if ( w2[ 0 ] == '+' )
			{
				tcol = tbfield_col + ds2d( substr( w2, 2 ) ) ;
			}
			else
			{
				tcol = ds2d( w2 ) ;
			}
			if ( isnumeric( w3 ) )                   { tlen = ds2d( w3 ) ; }
			else if ( w3 == "MAX" )                  { tlen = ZSCRMAXW - tcol + 1 ; }
			else if ( substr( w3, 1, 4 ) == "MAX-" ) { tlen = ZSCRMAXW - tcol - ds2d( substr( w3, 5 ) ) + 1 ; }
			else 				         { return 20        ; }
			tbfield_col = tcol    ;
			tbfield_sz  = tlen    ;
			w4 = word( line2, 4 ) ;
  			if ( cuaAttrName.find( w4 ) == cuaAttrName.end() )
			{
				PERR = "Unknown field CUA attribute type " + w4  ; return 20 ;
			}
			fType = cuaAttrName[ w4 ] ;
			debug2( "Creating tbfield" << endl ) ;
			create_tbfield( tcol, tlen, fType, word( line2, 6 ), word( line2, 5 ) ) ;
			if ( RC > 0 ) { return 20 ; }
			continue ;
		}
		else 
		{
			PERR = "Panel " + p_name + " error.  Invalid line: " + line2 ; return 20 ;
		}
	}

	if ( scrollOn && ( fieldList.find( "ZSCROLL" ) == fieldList.end() ) )
	{
		PERR = "Field ZSCROLL not defined for scrollable panel" ; return 20 ;
	}
	if ( fieldList.find( Home ) == fieldList.end() )
	{
		PERR = "Home field " + Home + " not defined in panel body" ; return 20 ;
	}
	if ( fieldList.find( CMDfield ) == fieldList.end() )
	{
		PERR = "Command field " + CMDfield + " not defined in panel body" ; return 20 ;
	}

	PANELID = p_name ;
	debug1( "Panel loaded and processed successfully" << endl ) ;
	return  0 ;
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
	// Upate a dialogue variable where it resides in the standard search order.  If not found, create an implicit function pool variable

	if     ( p_funcPOOL->ifexists( RC, var ) ) { p_funcPOOL->put( RC, 0, var, val )   ; }
	else if ( p_poolMGR->ifexists( RC, var ) ) { p_poolMGR->put( RC, var, val, ASIS ) ; }
	else                                       { p_funcPOOL->put( RC, 0, var, val )   ; }
}


void pPanel::display_panel( int & RC )
{
	debug1( "Displaying fields and action bar " << endl ) ;

	RC = 0 ;
	clear() ;

	attrset( cuaAttr[ PT ] ) ;
	mvprintw( 2, ( ZSCRMAXW - PanelTitle.size() ) / 2, PanelTitle.c_str() ) ;
	attroff( cuaAttr[ PT ] ) ;

	if ( tb_model )
	{
		p_funcPOOL->put( RC, 0, "ZTDSELS", 0 ) ;
		display_tb_mark_posn() ;
		tb_fields_active_inactive() ;
	}

	display_ab()          ;
	display_literals()    ;
	update_field_values( RC ) ;

	display_fields() ;
	display_boxes()  ;
	hide_pd()        ;
	display_pd()     ;
	display_MSG()    ;
	return           ;
}


void pPanel::refresh( int & RC )
{
	debug1( "Refreshing panel fields and action bar " << endl ) ;

	RC = 0  ;
	clear() ;

	attrset( cuaAttr[ PT ] ) ;
	mvprintw( 2, ( ZSCRMAXW - PanelTitle.size() ) / 2, PanelTitle.c_str() ) ;
	attroff( cuaAttr[ PT ] ) ;

	display_ab()       ;
	display_literals() ;
	display_fields()   ;
	display_boxes()    ;
	hide_pd()          ;
	display_MSG()      ;
	display_pd()       ;

	if ( tb_model ) { display_tb_mark_posn() ; }
}


void pPanel::display_panel_update( int & RC )
{
	//  For all changed fields, apply attributes (upper case, left/right justified), and copy back to function pool
	//  If END entered with pull down displayed, just remove the pull down and clear ZVERB
	//  If cursor on a point-and-shoot field (PS), set variable as in the )PNTS panel section if defined there
	//  When copying to the function pool, change value as follows:
	//      JUST(LEFT/RIGHT) leading and trailing spaces are removed
	//      JUST(ASIS) Only trailing spaces are removed

	int fieldNum    ;
	int scrollAmt   ;
	int p           ;

	string wd       ;
	string l        ;
	string CMDVerb  ;
	string CMD      ;
	string name, t  ;
	string val      ;
	string vars     ;
	string var      ;
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
		return ;
	}
	else if ( CMDVerb == "END" || CMDVerb == "EXIT" || CMDVerb == "RETURN" ) { return ; }

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
		if ( MSGID != "" ) { return ; }
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
		CMD    = fieldList[ CMDfield ]->field_value ;
		msgfld = CMDfield ;
		if ( CMD == "" )
		{
			CMD = fieldList[ "ZSCROLL" ]->field_value ;
			msgfld = "ZSCROLL" ;
		}
		if ( findword( CMDVerb, "UP DOWN LEFT RIGHT" ) )
		{
			if ( tb_model ) { scrollAmt = tb_depth  ; }
			else            { scrollAmt = dyn_depth ; }
			if ( (CMDVerb == "LEFT") || (CMDVerb == "RIGHT") )
			{
				if ( !tb_model ) { scrollAmt = dyn_width ; }
			}
			if ( isnumeric( CMD ) )
			{
				if ( CMD.size() > 6 )
				{
					MSGID  = "PSYS01I" ;
					CURFLD = msgfld    ;
					return             ;
				}
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
						if ( fieldNum == 0 ) { p_poolMGR->put( RC, "ZSCROLLN", d2ds( scrollAmt ),  SHARED ) ; }
						else                 { p_poolMGR->put( RC, "ZSCROLLN", d2ds( fieldNum-1 ), SHARED ) ; }
						break ;
					case 'D':
						p_poolMGR->put( RC, "ZSCROLLN", d2ds( scrollAmt ), SHARED ) ;
						break ;
					case 'H':
						p_poolMGR->put( RC, "ZSCROLLN", d2ds( scrollAmt/2 ), SHARED ) ;
						break ;
					case 'P':
						p_poolMGR->put( RC, "ZSCROLLN", d2ds( scrollAmt ), SHARED ) ;
						break ;
					default:
						MSGID  = "PSYS01I" ;
						CURFLD = msgfld    ;
						return             ;
					}
				}
			}
			fieldList[ CMDfield ]->field_value = "" ;
			p_funcPOOL->put( RC, 0, CMDfield, "" )  ;
		}
	}
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
					if ( assgnListi.at( i_assign ).as_retlen )     { t = d2ds( t.size() )   ; }
					else if ( assgnListi.at( i_assign ).as_upper ) { t = upper( t )         ; }
					else if ( assgnListi.at( i_assign ).as_words ) { t = d2ds( words( t ) ) ; }
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
	map< int, string >::iterator it2  ;
	
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
					if ( assgnListr.at( i_assign ).as_retlen )     { t = d2ds( t.size() )   ; }
					else if ( assgnListr.at( i_assign ).as_upper ) { t = upper( t )         ; }
					else if ( assgnListr.at( i_assign ).as_words ) { t = d2ds( words( t ) ) ; }
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
	string CMDVerb  ;
	string CMD      ;
	string name, t  ;
	string val      ;
	string vars     ;
	string var      ;
	string fieldNam ;
	string msgfld   ;
	string dTRAIL   ;

	bool found      ;
	bool if_skip    ;

	map<string, field *>::iterator it;
	map< int, string >::iterator it2 ;

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
			val = getDialogueVar( ifList.at( i_if ).if_lhs ) ;
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
					if ( assgnList.at( i_assign ).as_retlen )     { t = d2ds( t.size() )   ; }
					else if ( assgnList.at( i_assign ).as_upper ) { t = upper( t )         ; }
					else if ( assgnList.at( i_assign ).as_words ) { t = d2ds( words( t ) ) ; }
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
        for ( int i = 0 ; i < ZSCRMAXD ; i ++ )
        {
                move( i, 0 ) ;
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


void pPanel::create_pdc( string abc_name, string pdc_name, string pdc_run, string pdc_parm )
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
	ab.at(i).add_pdc( pdc_name, pdc_run, pdc_parm ) ;
}


void pPanel::update_field_values( int & RC )
{
	// Update field_values from the dialogue variables (may not exist so treat RC=8 from getDialogueVar as normal completion)
	//
	// Apply just(right|left|asis) to non-dynamic area input/output fields
	//     JUST(LEFT)  strip off leading and trailing spaces
	//     JUST(RIGHT) strip off trailing spaces only and pad to the left with spaces to size field_length
	//     JUST(ASIS) no change
	// Treat dynamic areas differently - they must reside in the function pool.

	string t1     ;
	string sname  ;
	string shadow ;

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
		t1     = p_funcPOOL->get( RC, 0, it2->first )           ;
		sname  = dynAreaList[ it2->first ]->dynArea_shadow_name ;
		shadow = p_funcPOOL->get( RC, 0, sname )                ;
		if ( t1.size() > shadow.size() )
		{
			log( "W", "Shadow variable " << sname << " size is smaller than the data variable " << it2->first << " size.  Results may be unpredictable" << endl ) ;
			log( "W", "Data variable size   = " << t1.size() << endl ) ;
			log( "W", "Shadow variable size = " << shadow.size() << endl ) ;
		}
		t1.resize( it2->second->dynArea_width * it2->second->dynArea_depth, ' ' )     ;
		shadow.resize( it2->second->dynArea_width * it2->second->dynArea_depth, ' ' ) ;
	        for ( int i = 0 ; i < it2->second->dynArea_depth ; i++ )
	        {
			fieldList[ it2->first + "." + d2ds( i )]->field_value        = t1.substr( i * it2->second->dynArea_width, it2->second->dynArea_width ) ;
			fieldList[ it2->first + "." + d2ds( i )]->field_shadow_value = shadow.substr( i * it2->second->dynArea_width, it2->second->dynArea_width ) ;
	        }
	}
}


void pPanel::display_literals()
{
	RC = 0 ;
	debug2( "Displaying literals " << endl ) ;

	for ( uint i = 0 ; i < literalList.size() ; i++ )
	{
		literalList.at( i )->literal_display() ;
	}
}


void pPanel::display_fields()
{
	debug2( "Displaying active fields (from field values only) " << endl ) ;
	map<string, field *>::iterator it;

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( !it->second->field_active ) { continue ; }
		it->second->display_field() ;
	}
}


void pPanel::display_ab()
{
	int i ;

	for ( i = 0 ; i < ab.size() ; i++ )
	{
		ab.at( i ).display_abc_unsel() ;
	}

        attrset( cuaAttr[ ABSL ] ) ;
	mvhline( 1, 0, ACS_HLINE, ZSCRMAXW ) ;
        attroff( cuaAttr[ ABSL ] ) ;
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
		p_row = fieldList[ f_name ]->field_row ;
	}
	return ;
}


void pPanel::get_home( uint & row, uint & col )
{
	if ( fieldList.find( Home ) == fieldList.end() )
	{
		col = 0 ;
		row = 0 ;
	}
	else
	{
		col = fieldList[ Home ]->field_col ;
		row = fieldList[ Home ]->field_row ;
	}
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
	fieldList[ f_name ]->display_field()         ;
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
	fieldList[ CMDfield ]->display_field()         ;
}


string pPanel::field_getname( int row, int col )
{
	map<string, field *>::iterator it ;
	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( (it->second->field_row == row) && (col >=it->second->field_col) && (col < (it->second->field_col + it->second->field_length )) )
		{
			if ( !it->second->field_active ) return "" ;
			return it->first ;
		}
	}
	return  "" ;
}


void pPanel::field_clear( string f_name )
{
        fieldList[ f_name ]->field_clear() ;
}


void pPanel::field_edit( uint row, uint col, char ch, bool Isrt, bool & prot )
{
	p_row = row ;
	p_col = col ;
	map<string, field *>::iterator it;

	prot = true ;
	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( (it->second->field_row == row) && (col >=it->second->field_col) && (col < (it->second->field_col + it->second->field_length )) )
		{
			if ( !it->second->field_active ) return ;
			if (  it->second->field_numeric && !isdigit( ch ) )   { return ; }
			if ( !it->second->field_dynArea && !it->second->field_input ) return ;
			if (  it->second->field_dynArea && !it->second->field_dyna_input( col ) ) { return ; }
			if ( !it->second->edit_field_insert( ch, col, Isrt ) ) { return ; }
			prot = false ;
			++p_col ;
			if ( (p_col == it->second->field_col + it->second->field_length) & (it->second->field_skip) )
			{
				field_tab_next( p_row, p_col ) ;
			}
			return ;
		}
	}
}


void pPanel::field_backspace( uint & row, uint & col, bool & prot )
{
	map<string, field *>::iterator it;

	prot = true ;
	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( (it->second->field_row == row) && (col >it->second->field_col) && (col < (it->second->field_col + it->second->field_length )) )
		{
			if ( !it->second->field_active ) return ;
			if ( !it->second->field_dynArea && !it->second->field_input ) return ;
			if (  it->second->field_dynArea && !it->second->field_dyna_input( col ) ) return ;
			col  = fieldList[ it->first ]->edit_field_backspace( col ) ;
			prot = false ;
			return ;
		}
	}
}


void pPanel::field_delete_char( uint row, uint col, bool & prot )
{
	map<string, field *>::iterator it;

	prot = true ;
	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( (it->second->field_row == row) && (col >=it->second->field_col) && (col < (it->second->field_col + it->second->field_length )) )
		{
			if ( !it->second->field_active ) return ;
			if ( !it->second->field_dynArea && !it->second->field_input ) return ;
			if (  it->second->field_dynArea && !it->second->field_dyna_input( col ) ) return ;
			fieldList[ it->first ]->edit_field_delete( col ) ;
			prot = false ;
			return ;
		}
	}
}


void pPanel::field_erase_eof( uint row, uint col, bool & prot )
{
	map<string, field *>::iterator it;

	prot = true ;
	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( (it->second->field_row == row) && (col >=it->second->field_col) && (col < (it->second->field_col + it->second->field_length )) )
		{
			if ( !it->second->field_active ) return ;
			if ( !it->second->field_dynArea && !it->second->field_input ) return ;
			if (  it->second->field_dynArea && !it->second->field_dyna_input( col ) ) return ;
			fieldList[ it->first ]->field_erase_eof( col ) ;
			prot = false ;
			return ;
		}
	}
}


void pPanel::cursor_eof( uint & row, uint & col )
{
	map<string, field *>::iterator it;
	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( (it->second->field_row == row) && (col >=it->second->field_col) &&  (col < (it->second->field_col + it->second->field_length )) )
		{
			if ( !it->second->field_active ) return ;
			if ( !it->second->field_dynArea && !it->second->field_input ) return ;
			if (  it->second->field_dynArea && !it->second->field_dyna_input( col ) ) { return ; }
			col = fieldList[ it->first ]->end_of_field( col ) ;
			return ;
		}
	}
}


void pPanel::field_tab_down( uint & row, uint & col )
{
	int t_offset ;
	int m_offset ;
	int c_offset ;
	int d_offset ;
	int o_row    ;

	bool cursor_moved(false) ;
	map<string, field *>::iterator it;

	c_offset = row * ZSCRMAXW + col ;
	m_offset = ZSCRMAXD * ZSCRMAXW  ;
	o_row = row ;

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
		t_offset = it->second->field_row * ZSCRMAXW + it->second->field_col + d_offset ;
		if ( (t_offset > c_offset) & (t_offset < m_offset) )
		{
			m_offset = t_offset ;
			row      = it->second->field_row ;
			col      = it->second->field_col + d_offset ;
			cursor_moved = true ;
		}
	}
	if ( !cursor_moved  ) { get_home( row, col ) ; }
}



void pPanel::field_tab_next( uint & row, uint & col )
{
	int t_offset ;
	int m_offset ;
	int c_offset ;
	int d_offset ;
	int o_row    ;
	int o_col    ;

	bool cursor_moved(false) ;
	map<string, field *>::iterator it;

	c_offset = row * ZSCRMAXW + col ;
	m_offset = ZSCRMAXD * ZSCRMAXW  ;
	o_row    = row                  ;
	o_col    = col                  ;

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
		t_offset = it->second->field_row * ZSCRMAXW + it->second->field_col + d_offset ;
		if ( (t_offset > c_offset) & (t_offset < m_offset) )
		{
			m_offset = t_offset ;
			row      = it->second->field_row ;
			col      = it->second->field_col + d_offset ;
			cursor_moved = true ;
		}
	}
	if ( !cursor_moved  ) { get_home( row, col ) ; }
}


string pPanel::get_field_help( string fld )
{
	if ( fieldHList.find( fld ) == fieldHList.end() ) { return "" ; }
	return fieldHList[ fld ] ;
}


void pPanel::set_tb_linesChanged()
{
	//  Store changed lines for processing by the application if requested via tbdispl with no panel name
	//  Format is a list of line-number/URID pairs where changed field data is stored as tb_FIELDNAME-URID in the function pool

	int p  ;

	string URID ;

	map<string, field *>::iterator it ;

	for ( it = fieldList.begin() ; it != fieldList.end() ; it++ )
	{
		if ( (it->second->field_tb) && it->second->field_changed )
		{
			URID = p_funcPOOL->get( RC, 0, "ZURID." + d2ds( it->second->field_row - tb_row ), NOCHECK ) ;
			p = it->first.find( '.' ) ;
			p_funcPOOL->put( RC, 0, it->first.substr( 0, p ) + "-" + URID, it->second->field_value, NOCHECK ) ;
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
	//  Clear all stored changed lines on a tbdispl with panel name and remove the changed field data from the
	//  function pool (generic delete on tb_FIELDNAME-)

	int ws ;
	int i  ;

	tb_linesChanged.clear() ;

	ws  = words( tb_fields ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		p_funcPOOL->dlete_gen( RC, word( tb_fields, i ) + "-" ) ;
		if ( RC > 8 ) { RC = 20 ; PERR = "Internal error occured clearing generic URID data" ; return ; }
	}
	p_funcPOOL->put( RC, 0, "ZTDSELS", 0 ) ;
	RC = 0 ;
}


void pPanel::clear_URID_tb_lineChanged( int & RC, string URID )
{
	//  Remove the processed line from the list of changed lines and remove the updated field data from the
	//  function pool (stored under name tb_FIELDNAME-URID)

	int ws ;
	int i  ;

	map< int, string >::iterator it ;

	ws  = words( tb_fields ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		p_funcPOOL->dlete( RC, word( tb_fields, i ) + "-" + URID, NOCHECK ) ;
		if ( RC > 8 ) { RC = 20 ; PERR = "Internal error occured deleting data for URID=" + URID ; return ; }
	}
	RC = 0 ;
	it = tb_linesChanged.begin() ;
	tb_linesChanged.erase( it ) ;
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

	attrset( WHITE ) ;
	if ( size < tb_depth )
	{
		mark = p_funcPOOL->get( RC, 0, "ZTDMARK" ) ;
		mvaddnstr( tb_row + size, 0, mark.c_str(), mark.length() ) ;
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
	mvaddnstr( 2, ZSCRMAXW - posn.length(), posn.c_str(), posn.length() ) ;
	attroff( WHITE ) ;
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
				debug1( "Found pulldown " << ab.at(i).abc_name << endl ) ;
				ab.at(i).display_abc_sel() ;
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
	ab.at( abIndex ).display_abc_sel() ;
	ab.at( abIndex ).display_pd() ;
	p_col = ab.at( abIndex ).abc_col + 2 ;
	p_row = 2 ;
}


void pPanel::hide_pd()
{
	if ( !abActive ) return  ;
	ab.at( abIndex ).hide_pd() ;
	ab.at( abIndex ).display_abc_unsel() ;
}


pdc pPanel::retrieve_pdc( int row, int col )
{
	pdc t_pdc ;

	if ( !abActive ) return t_pdc ;
	ab.at( abIndex ).hide_pd() ;
	ab.at( abIndex ).display_abc_unsel() ;
	abActive = false ;
	return ab.at( abIndex ).retrieve_pdc( row, col ) ;
}


void pPanel::display_boxes()
{
	vector<Box *>::iterator it ;

	for ( it = Boxes.begin() ; it != Boxes.end() ; it++ )
	{
		(*it)->display_box() ;
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


void pPanel::display_MSG()
{
	if ( SMSG != "" )
	{
		debug1( "Selecting SMSG to display " << SMSG << endl ) ;
		attrset( cuaAttr[ MSGTYPE ] ) ;
		mvprintw( 1, ( ZSCRMAXW - SMSG.size()), SMSG.c_str() ) ;
		attroff( cuaAttr[ MSGTYPE ] ) ;
		if ( !showLMSG && MSGALRM )
		{
			beep() ;
			MSGALRM = false ;
		}
	}
	if ( LMSG != "" && showLMSG )
	{
		debug1( "Selecting LMSG to display " << LMSG << endl ) ;
		attrset( cuaAttr[ MSGTYPE ] )  ;
		mvprintw( 4, 1, LMSG.c_str() ) ;
		attroff( cuaAttr[ MSGTYPE ] )  ;
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

