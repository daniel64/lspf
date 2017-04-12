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


void pPanel::loadPanel( errblock& err, const string& p_name, const string& paths )
{
	int i, j, k          ;
	int p1, p2           ;
	int pVersion         ;
	int pFormat          ;

	string ww, w1, w2    ;
	string w3, w4, w5    ;
	string w6, w7, ws    ;
	string t1, t2        ;
	string rest          ;
	string filename      ;
	string pline         ;
	string oline         ;
	string fld, hlp      ;

	bool body(false)     ;
	bool command(false)  ;
	bool init(false)     ;
	bool reinit(false)   ;
	bool proc(false)     ;
	bool help(false)     ;
	bool ispnts(false)   ;
	bool isfield(false)  ;
	bool found           ;

	std::ifstream panl   ;
	std::ifstream pincl  ;

	vector< string > pSource      ;
	vector< string >::iterator it ;

	map<string, field *>::iterator it1;

	RC = 0 ;

	readPanel( err, pSource, p_name, paths, p_name ) ;
	if ( err.error() ) { return ; }

	for ( it = pSource.begin() ; it != pSource.end() ; it++ )
	{
		pline = *it   ;
		oline = *it   ;
		trim( oline ) ;
		p1 = pline.find( "/*" ) ;
		if ( p1 != string::npos )
		{
			p2 = pline.find( "*/" ) ;
			if ( p2 != string::npos ) { pline.replace( p1, p2-p1+2, p2-p1+2, ' ' ) ; }
		}
		if ( pline.find( '\t' ) != string::npos )
		{
			err.seterrid( "PSYE011A" ) ;
			return ;
		}
		w1 = upper( word( pline, 1 ) ) ;
		p1 = w1.find( '=' )            ;
		if ( p1 != string::npos )
		{
			if ( p1 == w1.size()-1 ) { w3 = word( pline, 2 )  ; }
			else                     { w3 = w1.substr( p1+1 ) ; }
			w1 = w1.substr( 0, p1 ) ;
			w2 = "="                ;
		}
		else
		{
			w2 = word( pline, 2 ) ;
			if ( w2.size() > 1 && w2.front() == '=' )
			{
				w3 = w2.erase( 0, 1 ) ;
				w2 = "=" ;
			}
			else
			{
				w3 = word( pline, 3 ) ;
			}
		}
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
		if ( w1 == ")PANEL" )
		{
			i = pos( " VERSION=", pline ) ;
			j = pos( " FORMAT=", pline )  ;
			if ( i == 0 || j == 0 )
			{
				err.seterrid( "PSYE011B" ) ;
				err.setsrc( oline ) ;
				return ;
			}
			pVersion = ds2d( word( substr( pline, i+9 ), 1 ) ) ;
			pFormat  = ds2d( word( substr( pline, j+8 ), 1 ) ) ;
			llog( "I", "Panel format " << pFormat << " Panel version " << pVersion << endl ) ;
			i = pos( " KEYLIST(", pline ) ;
			if ( i > 0 )
			{
				j = pos( ",", pline, i ) ;
				k = pos( ")", pline, i ) ;
				if ( j == 0 || k == 0 || j > k )
				{
					err.seterrid( "PSYE011B" ) ;
					err.setsrc( oline ) ;
					return ;
				}
				KEYLISTN = strip( pline.substr( i+8, j-i-9 ) ) ;
				KEYAPPL  = strip( pline.substr( j, k-j-1 ) )   ;
				if ( !isvalidName( KEYLISTN ) || !isvalidName4( KEYAPPL ) )
				{
					err.seterrid( "PSYE011C" ) ;
					err.setsrc( oline ) ;
					return ;
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
				if ( k == 0 )
				{
					err.seterrid( "PSYE011D" )  ;
					err.setsrc( oline ) ;
					return ;
				}
				ws = substr( pline, j+8, k-j-8 ) ;
				j  = ws.find( ',' )       ;
				if ( j == string::npos )
				{
					err.seterrid( "PSYE011D" )  ;
					err.setsrc( oline ) ;
					return ;
				}
				t1 = strip( ws.substr( 0, j) ) ;
				t2 = strip( ws.substr( j+1) )  ;
				win_width = ds2d( t1 ) ;
				win_depth = ds2d( t2 ) ;
				pwin   = newwin( win_depth, win_width, 0, 0 ) ;
				bwin   = newwin( win_depth+2, win_width+2, 0, 0 ) ;
				bpanel = new_panel( bwin ) ;
				set_panel_userptr( bpanel, new panel_data( ZSCRNUM ) ) ;
				WSCRMAXW = win_width ;
				WSCRMAXD = win_depth ;
			}
			j = pos( " CMD(", pline ) ;
			if ( j > 0 )
			{
				k  = pos( ")", pline, j ) ;
				if ( k == 0 )
				{
					err.seterrid( "PSYE011D" )  ;
					err.setsrc( oline ) ;
					return ;
				}
				CMDfield = strip( substr( pline, j+5, k-j-5 ) ) ;
				if ( !isvalidName( CMDfield ) )
				{
					err.seterrid( "PSYE022J", "command field", w7 ) ;
					err.setsrc( oline ) ;
					return ;
				}
			}
			j = pos( " HOME(", pline ) ;
			if ( j > 0 )
			{
				k = pos( ")", pline, j ) ;
				if ( k == 0 )
				{
					err.seterrid( "PSYE011D" )  ;
					err.setsrc( oline ) ;
					return ;
				}
				Home = strip( substr( pline, j+6, k-j-6 ) ) ;
				if ( !isvalidName( Home ) )
				{
					err.seterrid( "PSYE022J", "home field", w7 ) ;
					err.setsrc( oline ) ;
					return ;
				}
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
			if ( w1.back() == ':' )
			{
				w1.pop_back() ;
				if ( !isvalidName( w1 ) )
				{
					err.seterror( "Invalid label '"+ w1 +"'" ) ;
					err.setsrc( oline ) ;
					return ;
				}
				panstmnt m_stmnt ;
				m_stmnt.ps_label  = upper( w1 ) ;
				m_stmnt.ps_column = pline.find_first_not_of( ' ' ) ;
				if ( init )        { initstmnts.push_back( m_stmnt ) ; }
				else if ( reinit ) { reinstmnts.push_back( m_stmnt ) ; }
				else               { procstmnts.push_back( m_stmnt ) ; }
				pline.replace( m_stmnt.ps_column, w1.size()+1, w1.size()+1, ' ' ) ;
				w1 = upper( word( pline, 1 ) ) ;
				if ( w1 == "" ) { continue ; }
				p1 = w1.find( '=' )            ;
				if ( p1 != string::npos )
				{
					if ( p1 == w1.size()-1 ) { w3 = word( pline, 2 )  ; }
					else                     { w3 = w1.substr( p1+1 ) ; }
					w1 = w1.substr( 0, p1 ) ;
					w2 = "="                ;
				}
				else
				{
					w2 = word( pline, 2 ) ;
					if ( w2.size() > 1 && w2.front() == '=' )
					{
						w3 = w2.erase( 0, 1 ) ;
						w2 = "=" ;
					}
					else
					{
						w3 = word( pline, 3 ) ;
					}
				}
			}
			p1 = w1.find( '(' ) ;
			if ( p1 != string::npos )
			{
				w1.erase( p1 ) ;
			}
			if ( w1 == "VGET" || w1 == "VPUT" )
			{
				panstmnt m_stmnt ;
				VPUTGET m_VPG    ;
				m_VPG.parse( err, pline ) ;
				if ( err.error() )
				{
					err.setsrc( oline ) ;
					return ;
				}
				m_stmnt.ps_vputget = true ;
				m_stmnt.ps_column  = pline.find_first_not_of( ' ' ) ;
				if ( init )        { vpgListi.push_back( m_VPG ) ; initstmnts.push_back( m_stmnt ) ; }
				else if ( reinit ) { vpgListr.push_back( m_VPG ) ; reinstmnts.push_back( m_stmnt ) ; }
				else               { vpgListp.push_back( m_VPG ) ; procstmnts.push_back( m_stmnt ) ; }
				continue ;
			}
			if ( ( w1[ 0 ] == '&' || w1[ 0 ] == '.' ) &&
			     ( w2 == "="                        ) &&
			     ( substr( w3, 1, 5 ) != "TRUNC"    ) &&
			     ( substr( w3, 1, 5 ) != "TRANS"    ) )
			{
				ASSGN m_assgn    ;
				panstmnt m_stmnt ;
				m_assgn.parse( err, pline ) ;
				if ( err.error() )
				{
					err.setsrc( oline ) ;
					return ;
				}
				if ( m_assgn.as_isattr && ( fieldList.find( m_assgn.as_lhs ) == fieldList.end() ) )
				{
					if ( wordpos( m_assgn.as_lhs, tb_fields ) == 0 )
					{
						err.seterror( "Invalid .ATTR statement. Field '"+ m_assgn.as_lhs +"' not found" ) ;
						err.setsrc( oline ) ;
						return ;
					}
					else
					{
						m_assgn.as_istb = true ;
					}
				}
				m_stmnt.ps_assign = true ;
				m_stmnt.ps_column = pline.find_first_not_of( ' ' ) ;
				if ( init )        { assgnListi.push_back( m_assgn ) ; initstmnts.push_back( m_stmnt ) ; }
				else if ( reinit ) { assgnListr.push_back( m_assgn ) ; reinstmnts.push_back( m_stmnt ) ; }
				else               { assgnListp.push_back( m_assgn ) ; procstmnts.push_back( m_stmnt ) ; }
				continue ;
			}
			if ( w1 == "IF" )
			{
				IFSTMNT m_if     ;
				panstmnt m_stmnt ;
				m_if.parse( err, pline ) ;
				if ( err.error() )
				{
					err.setsrc( oline ) ;
					return ;
				}
				if ( wordpos( m_if.if_lhs, tb_fields ) > 0 ) { m_if.if_istb = true ; }
				m_stmnt.ps_if     = true  ;
				m_stmnt.ps_column = pline.find_first_not_of( ' ' ) ;
				if ( init )        { ifListi.push_back( m_if ) ; initstmnts.push_back( m_stmnt ) ; }
				else if ( reinit ) { ifListr.push_back( m_if ) ; reinstmnts.push_back( m_stmnt ) ; }
				else               { ifListp.push_back( m_if ) ; procstmnts.push_back( m_stmnt ) ; }
				continue ;
			}
			if ( w1 == "ELSE" && w2 == "" )
			{
				vector<panstmnt> * p_stmnt  ;
				vector<IFSTMNT>  * p_ifList ;
				if ( init )        { p_stmnt = &initstmnts ; p_ifList = &ifListi ; }
				else if ( reinit ) { p_stmnt = &reinstmnts ; p_ifList = &ifListr ; }
				else               { p_stmnt = &procstmnts ; p_ifList = &ifListp ; }
				IFSTMNT m_if        ;
				m_if.if_else = true ;
				panstmnt m_stmnt    ;
				j                 = 0     ;
				found             = false ;
				m_stmnt.ps_else   = true  ;
				m_stmnt.ps_column = pline.find_first_not_of( ' ' ) ;
				for ( i = p_stmnt->size()-1 ; i >= 0 ; i-- )
				{
					if ( p_stmnt->at( i ).ps_if || p_stmnt->at( i ).ps_else ) { j++ ; }
					if ( p_stmnt->at( i ).ps_if && (p_stmnt->at( i ).ps_column == m_stmnt.ps_column))
					{
						m_if.if_stmnt = p_ifList->size()-j ;
						p_ifList->push_back( m_if ) ;
						p_stmnt->push_back( m_stmnt ) ;
						found = true ;
						break        ;
					}
					if ( p_stmnt->at( i ).ps_column <= m_stmnt.ps_column )
					{
						break ;
					}
				}
				if ( !found )
				{
					err.seterror( "No matching IF statement found for ELSE at column "+ d2ds( m_stmnt.ps_column+1 ) ) ;
					err.setsrc( oline ) ;
					return ;
				}
				continue ;
			}
			if ( substr( w1, 1, 7 ) == "REFRESH"  )
			{
				panstmnt m_stmnt ;
				t1 = pline ;
				if ( init )
				{
					err.seterror( "REFRESH statement is invalid within the )INIT section" ) ;
					err.setsrc( trim( t1 ) ) ;
					return ;
				}
				m_stmnt.ps_refresh = true ;
				m_stmnt.ps_column  = pline.find_first_not_of( ' ' ) ;
				iupper( pline ) ;
				pline = strip( pline.erase( 0, pline.find( "REFRESH" )+7 ) ) ;
				if ( !isvalidName ( pline ) && pline != "*" )
				{
					if ( pline.front() != '(' || pline.back() != ')' )
					{
						err.seterror( "Invalid REFRESH statement found in )PROC or )REINIT section" ) ;
						err.setsrc( trim( t1 ) ) ;
						return ;
					}
					pline.pop_back() ;
					pline.erase( 0, 1 ) ;
					replace( pline.begin(), pline.end(), ',', ' ' ) ;
				}
				for ( i = words( pline ), j = 1 ; j <= i ; j++ )
				{
					w1 = word( pline, j ) ;
					if ( w1 == "*" )
					{
						m_stmnt.ps_rlist = "*" ;
					}
					else
					{
						if ( !isvalidName( w1 ) )
						{
							err.seterror( "Invalid field name '"+ w1 +"' on REFRESH statement" ) ;
							err.setsrc( trim( t1 ) ) ;
							return ;
						}
						if ( fieldList.count( w1 ) == 0 )
						{
							err.seterror( "REFRESH field '"+ w1 +"' not on the panel" ) ;
							err.setsrc( trim( t1 ) ) ;
							return ;
						}
						if ( m_stmnt.ps_rlist != "*" ) { m_stmnt.ps_rlist += " " + w1 ; }
					}
				}
				if ( m_stmnt.ps_rlist == "" )
				{
					err.seterror( "Variable name or name-list not coded for 'REFRESH' statement" ) ;
					err.setsrc( trim( t1 ) ) ;
					return ;
				}
				if ( reinit ) { reinstmnts.push_back( m_stmnt ) ; }
				else          { procstmnts.push_back( m_stmnt ) ; }
				continue ;
			}
			if ( w1 == "EXIT" && w2 == "" )
			{
				panstmnt m_stmnt ;
				m_stmnt.ps_exit   = true ;
				m_stmnt.ps_column = pline.find_first_not_of( ' ' ) ;
				if ( init )        { initstmnts.push_back( m_stmnt ) ; }
				else if ( reinit ) { reinstmnts.push_back( m_stmnt ) ; }
				else               { procstmnts.push_back( m_stmnt ) ; }
				continue ;
			}
			if ( w1 == "GOTO" && w2 != "" && w3 == "" )
			{
				panstmnt m_stmnt ;
				m_stmnt.ps_goto = true ;
				w2              = upper( w2 ) ;
				if ( !isvalidName( w2 ) )
				{
					err.seterror( "Invalid label '"+ w2 +"' on GOTO statement" ) ;
					err.setsrc( oline ) ;
					return ;
				}
				m_stmnt.ps_label  = w2 ;
				m_stmnt.ps_column = pline.find_first_not_of( ' ' ) ;
				if ( init )        { initstmnts.push_back( m_stmnt ) ; }
				else if ( reinit ) { reinstmnts.push_back( m_stmnt ) ; }
				else               { procstmnts.push_back( m_stmnt ) ; }
				continue ;
			}
			if ( w2 == "=" && substr( w3, 1, 5 ) == "TRANS" )
			{
				TRANS m_trans    ;
				panstmnt m_stmnt ;
				m_trans.parse( err, pline ) ;
				if ( err.error() )
				{
					err.setsrc( oline ) ;
					return ;
				}
				m_stmnt.ps_trans  = true ;
				m_stmnt.ps_column = pline.find_first_not_of( ' ' ) ;
				if ( init )        { transListi.push_back( m_trans ) ; initstmnts.push_back( m_stmnt ) ; }
				else if ( reinit ) { transListr.push_back( m_trans ) ; reinstmnts.push_back( m_stmnt ) ; }
				else               { transListp.push_back( m_trans ) ; procstmnts.push_back( m_stmnt ) ; }
				continue ;
			}
			if ( w2 == "=" && substr( w3, 1, 5 ) == "TRUNC" )
			{
				TRUNC m_trunc    ;
				panstmnt m_stmnt ;
				m_trunc.parse( err, pline ) ;
				if ( err.error() )
				{
					err.setsrc( oline ) ;
					return ;
				}
				m_stmnt.ps_trunc  = true ;
				m_stmnt.ps_column = pline.find_first_not_of( ' ' ) ;
				if ( init )        { truncListi.push_back( m_trunc ) ; initstmnts.push_back( m_stmnt ) ; }
				else if ( reinit ) { truncListr.push_back( m_trunc ) ; reinstmnts.push_back( m_stmnt ) ; }
				else               { truncListp.push_back( m_trunc ) ; procstmnts.push_back( m_stmnt ) ; }
				continue ;
			}
			if ( w1 == "VER" )
			{
				VERIFY m_VER     ;
				panstmnt m_stmnt ;
				m_VER.parse( err, pline ) ;
				if ( err.error() )
				{
					err.setsrc( oline ) ;
					return ;
				}
				if ( wordpos( m_VER.ver_var, tb_fields ) > 0 )
				{
					m_VER.ver_tbfield = true ;
				}
				m_VER.ver_field = ( fieldList.count( m_VER.ver_var ) > 0 || m_VER.ver_tbfield ) ;
				m_stmnt.ps_verify = true ;
				m_stmnt.ps_column = pline.find_first_not_of( ' ' ) ;
				if ( init )        { verListi.push_back( m_VER ) ; initstmnts.push_back( m_stmnt ) ; }
				else if ( reinit ) { verListr.push_back( m_VER ) ; reinstmnts.push_back( m_stmnt ) ; }
				else               { verListp.push_back( m_VER ) ; procstmnts.push_back( m_stmnt ) ; }
				continue ;
			}
		}

		if ( help )
		{
			i = pos( " FIELD(", " "+ pline ) ;
			if ( i > 0 ) { j = pos( ")", pline, i ) ; }
			if ( i == 0 || j == 0 )
			{
				err.seterror( "Invalid FIELD help entry in )HELP section.  Missing bracket" ) ;
				err.setsrc( oline ) ;
				return ;
			}
			fld = strip( substr( pline, i+6, j-i-6 ) ) ;
			if ( !isvalidName( fld ) )
			{
				err.seterror( "Invalid HELP entry field name '"+ fld +"'" ) ;
				err.setsrc( oline ) ;
				return ;
			}
			if ( fieldList.find( fld ) == fieldList.end() )
			{
				err.seterror( "Invalid HELP statement.  Field '"+ fld +"' does not exist" ) ;
				err.setsrc( oline ) ;
				return ;
			}

			i = pos( " HELP(", pline ) ;
			if ( i > 0 ) { j = pos( ")", pline, i ) ; }
			if ( i == 0 || j == 0 )
			{
				err.seterror( "Invalid FIELD help entry in )HELP section.  Missing bracket" ) ;
				err.setsrc( oline ) ;
				return ;
			}
			hlp = strip( substr( pline, i+6, j-i-6 ) ) ;
			if ( !isvalidName( hlp ) )
			{
				err.seterror( "Invalid HELP entry name '"+ hlp +"'" ) ;
				err.setsrc( oline ) ;
				return ;
			}
			fieldHList[ fld ] = hlp ;
			continue ;
		}

		if ( ispnts )
		{
			pnts m_pnts ;
			m_pnts.parse( err, pline ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
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
					err.seterror( "Field '"+ m_pnts.pnts_field +"' not found in panel" ) ;
					err.setsrc( oline ) ;
					return ;
				}
			}
			if ( fieldList.count( m_pnts.pnts_var ) == 0 )
			{
				err.seterror( "Variable '"+ m_pnts.pnts_var +"' not found in panel" ) ;
				err.setsrc( oline ) ;
				return ;
			}
			pntsTable[ m_pnts.pnts_field ] = m_pnts ;
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
						err.seterror( "Field '"+ t1 +"' not found on panel" ) ;
						err.setsrc( oline ) ;
						return ;
					}
				}
			}
			if ( i == 0 || j == 0 )
			{
				err.seterror( "Invalid format of FIELD() definition" ) ;
				err.setsrc( oline ) ;
				return ;
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
				err.seterror( "Invalid format of EXEC() definition" ) ;
				err.setsrc( oline ) ;
				return ;
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
					err.seterror( "Field '"+ ww +"' passed on field command for '"+ t1 +"' is not defined in panel body" ) ;
					err.setsrc( oline ) ;
					return ;
				}
			}
			if ( fieldExcTable.find( t1 ) != fieldExcTable.end() )
			{
				err.seterror( "Duplicate field command entry in )FIELD panel section for '"+ t1 +"'" ) ;
				err.setsrc( oline ) ;
				return ;
			}
			fieldExcTable[ t1 ] = t_fe ;
			continue ;
		}

		if ( !body )
		{
			err.seterrid( "PSYE041E" ) ;
			err.setsrc( oline ) ;
			return ;
		}
		else if ( w1 == "PANELTITLE" )
		{
			panelTitle = strip( strip( subword( pline, 2 ) ), 'B', '"' ) ;
			continue ;
		}
		else if ( w1 == "PANELDESCR" )
		{
			panelDescr = strip( strip( subword( pline, 2 ) ), 'B', '"' ) ;
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
			m_lit->literal_init( err, WSCRMAXW, WSCRMAXD, opt_field, pline ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				delete m_lit ;
				return ;
			}
			literalList.push_back( m_lit ) ;
			continue ;
		}
		else if ( w1 == "FIELD" )
		{
			field * fld = new field ;
			fld->field_init( err, WSCRMAXW, WSCRMAXD, upper( pline ) ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				delete fld ;
				return ;
			}
			w7 = err.getUserData() ;
			if ( !isvalidName( w7 ) )
			{
				err.seterrid( "PSYE022J", "field", w7 ) ;
				err.setsrc( oline ) ;
				delete fld ;
				return ;
			}

			if ( fieldList.count( w7 ) > 0 )
			{
				err.seterrid( "PSYE041C", "field", w7 ) ;
				err.setsrc( oline ) ;
				delete fld ;
				return ;
			}
			fieldList[ w7 ] = fld ;
			continue ;
		}
		else if ( w1 == "DYNAREA" )
		{
			debug2( "Creating dynArea" << endl ) ;
			w6 = word( pline, 6 ) ;
			if ( !isvalidName( w6 ) )
			{
				err.seterrid( "PSYE022J", "dynamic area", w7 ) ;
				err.setsrc( oline ) ;
				return ;
			}

			if ( dynAreaList.count( w6 ) > 0 )
			{
				err.seterrid( "PSYE041C", "dynamic area", w7 ) ;
				err.setsrc( oline ) ;
				return ;
			}

			dynArea * m_dynArea = new dynArea ;
			m_dynArea->dynArea_init( err, WSCRMAXW, WSCRMAXD, upper( pline ) ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				delete m_dynArea ;
				return ;
			}

			dyn_width = m_dynArea->dynArea_width ;
			dyn_depth = m_dynArea->dynArea_depth ;

			dynAreaList[ w6 ] = m_dynArea ;
			field a ;
			a.field_cua         = AB ;
			a.field_col         = m_dynArea->dynArea_col   ;
			a.field_length      = m_dynArea->dynArea_width ;
			a.field_cole        = m_dynArea->dynArea_col + m_dynArea->dynArea_width ;
			a.field_dynArea     = true      ;
			a.field_dynArea_ptr = m_dynArea ;
			for ( i = 0 ; i < m_dynArea->dynArea_depth ; i++ )
			{
				field * fld    = new field ;
				*fld           = a         ;
				fld->field_row = m_dynArea->dynArea_row + i ;
				fieldList[ w6 + "." + d2ds( i ) ] = fld ;
			}
			continue ;
		}
		else if ( w1 == "BOX" )
		{
			debug2( "Creating box" << endl ) ;
			w2 = word( pline, 2 ) ;
			Box * m_box = new Box ;
			m_box->box_init( err, WSCRMAXW, WSCRMAXD, pline ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				delete m_box ;
				return ;
			}
			boxes.push_back( m_box ) ;
			continue ;
		}
		else if ( w1 == "PDC" )
		{
			debug2( "Creating pdc" << endl ) ;
			create_pdc( err, pline ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			continue ;
		}
		else if ( w1 == "TBMODEL" )
		{
			debug2( "Creating tbmodel" << endl ) ;
			iupper( pline ) ;
			w3 = word( pline, 3 ) ;
			int start_row = ds2d( word( pline, 2 ) ) - 1 ;

			if ( isnumeric( w3 ) )                      { tb_depth = ds2d( w3 ) ; }
			else if ( w3 == "MAX" )                     { tb_depth = WSCRMAXD - start_row ; }
			else if ( w3.compare( 0, 4, "MAX-" ) == 0 ) { tb_depth = WSCRMAXD - ds2d( substr( w3, 5 ) ) - start_row ; }
			else
			{
				err.seterrid( "PSYE031B", w3 ) ;
				err.setsrc( oline ) ;
				return ;
			}
			tb_model = true      ;
			tb_row   = start_row ;
			scrollOn = true      ;
			t1 = subword( pline, 4 ) ;
			t2 = parseString( err, t1, "ROWS()" ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			if ( t2 != "" && !findword( t2, "SCAN ALL" ) )
			{
				err.seterrid( "PSYE041Q", t2 ) ;
				err.setsrc( oline ) ;
				return ;
			}
			tb_scan  = ( t2 == "SCAN" ) ;
			tb_clear = parseString( err, t1, "CLEAR()" ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			replace( tb_clear.begin(), tb_clear.end(), ',', ' ' ) ;
			for ( j = words( tb_clear ), i = 1 ; i <= j ; i++ )
			{
				t2 = word( tb_clear, i ) ;
				if ( !isvalidName( t2 ) )
				{
					err.seterrid( "PSYE013A", "TBMODEL CLEAR", t2 ) ;
					err.setsrc( oline ) ;
					return ;
				}
			}
			if ( words( t1 ) > 0 )
			{
				err.seterrid( "PSYE032H", t1 ) ;
				err.setsrc( oline ) ;
				return ;
			}
			if ( (start_row + tb_depth ) > WSCRMAXD ) { tb_depth = (WSCRMAXD - start_row) ; }
			p_funcPOOL->put( err, "ZTDDEPTH", tb_depth ) ;
			continue ;
		}
		else if ( w1 == "TBFIELD" )
		{
			iupper( pline ) ;
			create_tbfield( err, pline ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			continue ;
		}
		else
		{
			err.seterrid( "PSYE041D", w1 ) ;
			err.setsrc( oline ) ;
			return ;
		}
	}

	if ( scrollOn )
	{
		if ( fieldList.count( "ZSCROLL" ) == 0 )
		{
			err.seterror( "Field ZSCROLL not defined for scrollable panel" ) ;
			return ;
		}
		fieldList[ "ZSCROLL" ]->field_caps = true ;
	}

	if ( fieldList.count( Home ) == 0 )
	{
		err.seterror( "Home field '"+ Home +"' not defined in panel body" ) ;
		return ;
	}

	if ( fieldList.count( CMDfield ) == 0 )
	{
		err.seterror( "Command field '"+ CMDfield +"' not defined in panel body" ) ;
		return ;
	}

	if ( REXX )
	{
		for ( it1 = fieldList.begin() ; it1 != fieldList.end() ; it1++ )
		{
			if ( !p_funcPOOL->ifexists( err, it1->first ) ) { syncDialogueVar( it1->first ) ; }
		}
	}

	fwin  = newwin( ZSCRMAXD, ZSCRMAXW, 0, 0 ) ;
	panel = new_panel( fwin ) ;
	set_panel_userptr( panel, new panel_data( ZSCRNUM ) ) ;

	PANELID = p_name ;
	debug1( "Panel loaded and processed successfully" << endl ) ;
	return ;
}


void pPanel::readPanel( errblock& err, vector<string>& src, const string& name, const string& paths, string slist )
{
	int i ;
	int j ;

	string filename ;
	string pline ;
	string w1    ;
	string w2    ;
	string type  ;

	bool comment ;
	bool found   ;

	std::ifstream panl ;

	if ( words( slist ) == 1 ) { type = "Panel"   ; }
	else                       { type = "INCLUDE" ; }

	found = false ;
	for ( i = getpaths( paths ), j = 1 ; j <= i ; j++ )
	{
		filename = getpath( paths, j ) + name ;
		if ( exists( filename ) )
		{
			if ( !is_regular_file( filename ) )
			{
				err.seterrid( "PSYE041L", type, filename ) ;
				return ;
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
		if ( type == "Panel" )
		{
			err.seterrid( "PSYE021B", name, 12 ) ;
		}
		else
		{
			err.seterrid( "PSYE041P", name, 20 ) ;
		}
		return ;
	}

	panl.open( filename.c_str() ) ;
	if ( !panl.is_open() )
	{
		err.seterrid( "PSYE041M", type, filename ) ;
		return ;
	}

	comment = false ;
	while ( getline( panl, pline ) )
	{
		if ( pline.find_first_not_of( ' ' ) == string::npos ) { continue ; }
		w1 = upper( word( pline, 1 ) ) ;
		w2 = word( pline, 2 ) ;
		if ( w1.compare( 0, 2, "--" ) == 0 || w1.front() == '#' ) { continue ; }
		if ( w1 == ")END" )        { break                      ; }
		if ( w1 == ")COMMENT" )    { comment = true  ; continue ; }
		if ( w1 == ")ENDCOMMENT" ) { comment = false ; continue ; }
		if ( comment )             { continue                   ; }
		if ( w1 == ")INCLUDE" )
		{
			if ( wordpos( w2, slist ) )
			{
				err.seterrid( "PSYE041O", name, w2 ) ;
				err.setsrc( trim( pline ) ) ;
				panl.close() ;
				return ;
			}
			readPanel( err, src, w2, paths, slist +" "+ w2 ) ;
			if ( err.error() )
			{
				panl.close() ;
				return ;
			}
			continue ;
		}
		src.push_back( pline ) ;
	}

	if ( panl.bad() )
	{
		err.seterrid( "PSYE041N", type, filename ) ;
	}
	panl.close() ;
}
