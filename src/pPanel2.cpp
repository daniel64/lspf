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

void pPanel::loadPanel( errblock& err, const string& p_name, const string& paths )
{
	int i ;
	int j ;
	int pVersion ;
	int pFormat  ;

	string ww         ;
	string w1         ;
	string w6, w7, ws ;
	string t1, t2     ;
	string pline      ;
	string oline      ;
	string fld, hlp   ;
	string abc_desc   ;

	bool abc     = false ;
	bool body    = false ;
	bool init    = false ;
	bool abcinit = false ;
	bool reinit  = false ;
	bool proc    = false ;
	bool abcproc = false ;
	bool help    = false ;
	bool ispnts  = false ;
	bool isfield = false ;

	vector<string> pSource      ;
	vector<string>::iterator it ;

	map<string, field *>::iterator it1;

	parser panelLang ;
	panelLang.optionUpper() ;

	readPanel( err, pSource, p_name, paths, p_name ) ;
	if ( err.error() ) { return ; }

	for ( it = pSource.begin() ; it != pSource.end() ; it++ )
	{
		pline = *it   ;
		oline = *it   ;
		trim( oline ) ;
		if ( pline.find( '\t' ) != string::npos )
		{
			err.seterrid( "PSYE011A" ) ;
			return ;
		}
		w1 = upper( word( pline, 1 ) ) ;
		if ( w1.front() == ')' )
		{
			abc     = false ;
			body    = false ;
			init    = false ;
			abcinit = false ;
			reinit  = false ;
			proc    = false ;
			abcproc = false ;
			help    = false ;
			ispnts  = false ;
			isfield = false ;
		}
		if ( w1 == ")ABC" )
		{
			abc_desc = extractKWord( err, pline, "DESC()" ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			if ( abc_desc == "" )
			{
				err.seterrid( "PSYE011G" ) ;
				err.setsrc( oline ) ;
				return ;
			}
			if ( pline != ")ABC" )
			{
				err.seterrid( "PSYE032H", subword( pline, 2  ) ) ;
				return ;
			}
			abc = true ;
			continue ;
		}
		if ( w1 == ")PANEL" )
		{
			iupper( pline ) ;
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
			ws = parseString( err, pline, "KEYLIST()" ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			if ( ws != "" )
			{
				j = ws.find( ',' ) ;
				if ( j == string::npos )
				{
					err.seterrid( "PSYE011B" ) ;
					err.setsrc( oline ) ;
					return ;
				}
				KEYLISTN = strip( ws.substr( 0, j ) ) ;
				KEYAPPL  = strip( ws.substr( j+1  ) ) ;
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
			idelword( pline, 1, 1 ) ;
			iupper( pline ) ;
			ws = parseString( err, pline, "WINDOW()" ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			if ( ws != "" )
			{
				j = ws.find( ',' ) ;
				if ( j == string::npos )
				{
					err.seterrid( "PSYE011D" ) ;
					err.setsrc( oline ) ;
					return ;
				}
				t1 = strip( ws.substr( 0, j ) ) ;
				t2 = strip( ws.substr( j+1 ) )  ;
				win_width = ds2d( t1 ) ;
				if ( win_width > ZSCRMAXW - 1 )
				{
					err.seterrid( "PSYE011E" ) ;
					err.setsrc( oline ) ;
					return ;
				}
				pwin = newwin( win_depth, win_width, 0, 0 ) ;
				win_depth = ds2d( t2 ) ;
				if ( win_depth > ZSCRMAXD - 2 )
				{
					err.seterrid( "PSYE011F" ) ;
					err.setsrc( oline ) ;
					return ;
				}
				pwin   = newwin( win_depth, win_width, 0, 0 ) ;
				bwin   = newwin( win_depth+2, win_width+2, 0, 0 ) ;
				bpanel = new_panel( bwin ) ;
				set_panel_userptr( bpanel, new panel_data( ZSCRNUM ) ) ;
				WSCRMAXW = win_width ;
				WSCRMAXD = win_depth ;
			}
			ws = parseString( err, pline, "CMD()" ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			if ( ws != "" )
			{
				cmdfield = ws ;
				if ( !isvalidName( cmdfield ) )
				{
					err.seterrid( "PSYE022J", "COMMAND field", cmdfield ) ;
					err.setsrc( oline ) ;
					return ;
				}
			}
			ws = parseString( err, pline, "HOME()" ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			if ( ws != "" )
			{
				Home = ws ;
				if ( !isvalidName( Home ) )
				{
					err.seterrid( "PSYE022J", "HOME field", Home ) ;
					err.setsrc( oline ) ;
					return ;
				}
			}
			ws = parseString( err, pline, "SCROLL()" ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			if ( ws != "" )
			{
				scroll = ws ;
				if ( !isvalidName( scroll ) )
				{
					err.seterrid( "PSYE022J", "SCROLL field", scroll ) ;
					err.setsrc( oline ) ;
					return ;
				}
			}
			if ( pline != "" )
			{
				err.seterrid( "PSYE032H", pline ) ;
				err.setsrc( oline ) ;
				return ;
			}
			body = true ;
			continue ;
		}
		else if ( w1 == ")ABCINIT" ) { abcinit = true ; continue ; }
		else if ( w1 == ")ABCPROC" ) { abcproc = true ; continue ; }
		else if ( w1 == ")PROC" )    { proc    = true ; continue ; }
		else if ( w1 == ")INIT" )    { init    = true ; continue ; }
		else if ( w1 == ")REINIT" )  { reinit  = true ; continue ; }
		else if ( w1 == ")HELP" )    { help    = true ; continue ; }
		else if ( w1 == ")PNTS" )    { ispnts  = true ; continue ; }
		else if ( w1 == ")FIELD" )   { isfield = true ; continue ; }

		if ( abc )
		{
			if ( w1 == "PDC" )
			{
				debug2( "Creating pdc" << endl ) ;
				create_pdc( err, abc_desc, pline ) ;
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
		if ( init || abcinit || reinit || proc || abcproc )
		{
			panstmnt * m_stmnt = new panstmnt ;
			m_stmnt->ps_column = pline.find_first_not_of( ' ' ) ;
			vector<panstmnt* >* p_stmnt ;
			if      ( init )    { p_stmnt = &initstmnts ; }
			else if ( abcinit ) { p_stmnt = &abc_initstmnts[ abc_desc ] ; }
			else if ( abcproc ) { p_stmnt = &abc_procstmnts[ abc_desc ] ; }
			else if ( reinit )  { p_stmnt = &reinstmnts ; }
			else                { p_stmnt = &procstmnts ; }
			if ( w1.back() == ':' )
			{
				w1.pop_back() ;
				if ( !isvalidName( w1 ) )
				{
					err.seterrid( "PSYE041R", w1 )  ;
					err.setsrc( oline ) ;
					delete m_stmnt ;
					return ;
				}
				m_stmnt->ps_label = upper( w1 ) ;
				pline.replace( m_stmnt->ps_column, w1.size()+1, w1.size()+1, ' ' ) ;
				trim( pline ) ;
				if ( pline == "" )
				{
					p_stmnt->push_back( m_stmnt ) ;
					continue ;
				}
			}
			panelLang.parseStatement( err, pline ) ;
			if ( err.error() ) { return ; }
			token tx ;
			switch ( panelLang.getStatementType() )
			{
			case ST_VGET:
			case ST_VPUT:
				createPanel_Vputget( err, panelLang, m_stmnt ) ;
				if ( err.error() )
				{
					err.setsrc( oline ) ;
					delete m_stmnt ;
					return ;
				}
				p_stmnt->push_back( m_stmnt )  ;
				break ;

			case ST_ASSIGN:
				createPanel_Assign( err, panelLang, m_stmnt ) ;
				if ( err.error() )
				{
					err.setsrc( oline ) ;
					delete m_stmnt ;
					return ;
				}
				p_stmnt->push_back( m_stmnt )  ;
				break ;

			case ST_IF:
				createPanel_If( err, panelLang, m_stmnt, init || abcinit ) ;
				if ( err.error() )
				{
					err.setsrc( oline ) ;
					delete m_stmnt ;
					return ;
				}
				p_stmnt->push_back( m_stmnt ) ;
				break ;

			case ST_ELSE:
				createPanel_Else( err, panelLang, m_stmnt, p_stmnt, init || abcinit ) ;
				if ( err.error() )
				{
					err.setsrc( oline ) ;
					delete m_stmnt ;
					return ;
				}
				break ;

			case ST_REFRESH:
				if ( init || abcinit )
				{
					err.seterrid( "PSYE041U" ) ;
					err.setsrc( oline ) ;
					delete m_stmnt ;
					return ;
				}
				createPanel_Refresh( err, panelLang, m_stmnt ) ;
				if ( err.error() )
				{
					err.setsrc( oline ) ;
					delete m_stmnt ;
					return ;
				}
				p_stmnt->push_back( m_stmnt ) ;
				break ;

			case ST_EXIT:
				m_stmnt->ps_exit = true ;
				p_stmnt->push_back( m_stmnt ) ;
				break ;

			case ST_GOTO:
				m_stmnt->ps_goto = true ;
				tx = panelLang.getToken( 1 ) ;
				if ( tx.subtype != TS_NAME )
				{
					err.seterrid( "PSYE041Z", tx.value ) ;
					err.setsrc( oline ) ;
					delete m_stmnt ;
					return ;
				}
				m_stmnt->ps_label = tx.value  ;
				p_stmnt->push_back( m_stmnt ) ;
				break ;

			case ST_VERIFY:
				createPanel_Verify( err, panelLang, m_stmnt ) ;
				if ( err.error() )
				{
					err.setsrc( oline ) ;
					delete m_stmnt ;
					return ;
				}
				p_stmnt->push_back( m_stmnt ) ;
				break ;

			case ST_EOF:
				delete m_stmnt ;
				break ;

			case ST_ERROR:
				err.seterrid( "PSYE041E" ) ;
				err.setsrc( oline ) ;
				delete m_stmnt ;
				return ;
			}
			continue ;
		}

		if ( help )
		{
			fld = parseString( err, pline, "FIELD()" ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			if ( fld == "" )
			{
				err.seterrid( "PSYE042A" ) ;
				err.setsrc( oline ) ;
				return ;
			}
			if ( !isvalidName( fld ) )
			{
				err.seterrid( "PSYE042B", fld ) ;
				err.setsrc( oline ) ;
				return ;
			}
			if ( fieldList.find( fld ) == fieldList.end() )
			{
				err.seterrid( "PSYE042C", fld ) ;
				err.setsrc( oline ) ;
				return ;
			}

			hlp = parseString( err, pline, "HELP()" ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			if ( hlp == "" )
			{
				err.seterrid( "PSYE042A" ) ;
				err.setsrc( oline ) ;
				return ;
			}
			if ( !isvalidName( hlp ) )
			{
				err.seterrid( "PSYE042D", hlp ) ;
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
				for ( i = 0 ; i < literalList.size() ; i++ )
				{
					if ( m_pnts.pnts_field == literalList.at( i )->literal_name )
					{
						break ;
					}
				}
				if ( i == literalList.size() )
				{
					err.seterrid( "PSYE042E", m_pnts.pnts_field ) ;
					err.setsrc( oline ) ;
					return ;
				}
			}
			if ( fieldList.count( m_pnts.pnts_var ) == 0 )
			{
				err.seterrid( "PSYE042F", m_pnts.pnts_var ) ;
				err.setsrc( oline ) ;
				return ;
			}
			pntsTable[ m_pnts.pnts_field ] = m_pnts ;
			continue ;
		}

		if ( isfield )
		{
			fieldExc t_fe ;
			t_fe.parse( err, pline ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			if ( fieldList.count( t_fe.fieldExc_field ) == 0 )
			{
				err.seterrid( "PSYE042E", t_fe.fieldExc_field ) ;
				err.setsrc( oline ) ;
				return ;
			}
			for ( j = words( t_fe.fieldExc_passed ), i = 1 ; i <= j ; i++ )
			{
				ww = word( t_fe.fieldExc_passed, i ) ;
				if ( fieldList.find( ww ) == fieldList.end() )
				{
					err.seterrid( "PSYE042I", ww, t_fe.fieldExc_field ) ;
					err.setsrc( oline ) ;
					return ;
				}
			}
			if ( fieldExcTable.count( t_fe.fieldExc_field ) > 0 )
			{
				err.seterrid( "PSYE042J", t_fe.fieldExc_field ) ;
				err.setsrc( oline ) ;
				return ;
			}
			fieldExcTable[ t_fe.fieldExc_field ] = t_fe ;
			continue ;
		}

		if ( !body )
		{
			err.seterrid( "PSYE041E" ) ;
			err.setsrc( oline ) ;
			return ;
		}
		if ( w1 == "PANELTITLE" )
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
			if ( m_lit->literal_cua == PS )
			{
				literalPS.push_back( m_lit ) ;
			}
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
			a.field_cua     = AB ;
			a.field_col     = m_dynArea->dynArea_col   ;
			a.field_length  = m_dynArea->dynArea_width ;
			a.field_cole    = m_dynArea->dynArea_col + m_dynArea->dynArea_width ;
			a.field_dynArea = m_dynArea ;
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
		else if ( w1 == "TBMODEL" )
		{
			debug2( "Creating tbmodel" << endl ) ;
			iupper( pline ) ;
			ww = word( pline, 3 ) ;
			int start_row = ds2d( word( pline, 2 ) ) - 1 ;

			if ( isnumeric( ww ) )                      { tb_depth = ds2d( ww ) ; }
			else if ( ww == "MAX" )                     { tb_depth = WSCRMAXD - start_row ; }
			else if ( ww.compare( 0, 4, "MAX-" ) == 0 ) { tb_depth = WSCRMAXD - ds2d( substr( ww, 5 ) ) - start_row ; }
			else
			{
				err.seterrid( "PSYE031B", ww ) ;
				err.setsrc( oline ) ;
				return ;
			}
			scrollOn = true      ;
			tb_model = true      ;
			tb_row   = start_row ;
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
		if ( fieldList.count( scroll ) == 0 )
		{
			err.seterrid( "PSYE042K" ) ;
			return ;
		}
		fieldList[ scroll ]->field_caps = true ;
	}

	if ( Home != "" && fieldList.count( Home ) == 0 )
	{
		err.seterrid( "PSYE042L", Home ) ;
		return ;
	}

	if ( cmdfield != "" && fieldList.count( cmdfield ) == 0 )
	{
		err.seterrid( "PSYE042M", cmdfield ) ;
		return ;
	}

	if ( cmdfield == "" && fieldList.count( "ZCMD" ) > 0 )
	{
		cmdfield = "ZCMD" ;
	}

	if ( Home == "" && cmdfield != "" )
	{
		Home = cmdfield ;
	}

	if ( scrollOn && cmdfield == "" )
	{
		err.seterrid( "PSYE042N", cmdfield ) ;
		return ;
	}

	if ( selectPanel && cmdfield != "ZCMD" )
	{
		err.seterrid( "PSYE042P" ) ;
		return ;
	}

	if ( REXX )
	{
		for ( it1 = fieldList.begin() ; it1 != fieldList.end() ; it1++ )
		{
			if ( !p_funcPOOL->ifexists( err, it1->first ) ) { syncDialogueVar( err, it1->first ) ; }
		}
	}

	fwin  = newwin( ZSCRMAXD, ZSCRMAXW, 0, 0 ) ;
	panel = new_panel( fwin ) ;
	set_panel_userptr( panel, new panel_data( ZSCRNUM ) ) ;

	panelid = p_name ;
	return ;
}


void pPanel::readPanel( errblock& err, vector<string>& src, const string& name, const string& paths, string slist )
{
	int i  ;
	int p  ;
	int p1 ;
	int p2 ;

	string filename ;
	string pline ;
	string w1    ;
	string w2    ;
	string w3    ;
	string type  ;
	string temp1 ;
	string temp2 ;

	bool comment ;
	bool split1  ;
	bool abc     ;

	std::ifstream panl ;

	type = ( words( slist ) == 1 ) ? "Panel" : "INCLUDE" ;

	for ( p = getpaths( paths ), i = 1 ; i <= p ; i++ )
	{
		filename = getpath( paths, i ) + name ;
		if ( exists( filename ) )
		{
			if ( !is_regular_file( filename ) )
			{
				err.seterrid( "PSYE041L", type, filename ) ;
				return ;
			}
			break ;
		}
	}
	if ( i > p )
	{
		type == "Panel" ? err.seterrid( "PSYE021B", name, 12 ) : err.seterrid( "PSYE041P", name, 20 ) ;
		return ;
	}

	panl.open( filename.c_str() ) ;
	if ( !panl.is_open() )
	{
		err.seterrid( "PSYE041M", type, filename ) ;
		return ;
	}

	comment = false ;
	abc     = false ;
	split1  = false ;
	temp2   = ""    ;
	while ( getline( panl, pline ) )
	{
		p1 = pline.find( "/*" ) ;
		if ( p1 != string::npos )
		{
			p2 = pline.find( "*/" ) ;
			if ( p2 != string::npos ) { pline.replace( p1, p2-p1+2, p2-p1+2, ' ' ) ; }
		}
		trim_right( pline ) ;
		if ( pline == "" ) { continue ; }
		if ( split1 )
		{
			temp1 += " " + trim_left( pline ) ;
			if ( pline.back() == ')' )
			{
				src.push_back( temp1 ) ;
				split1 = false ;
			}
			continue ;
		}
		w1 = upper( word( pline, 1 ) ) ;
		if ( w1.compare( 0, 2, "--" ) == 0 || w1.front() == '#' ) { continue ; }
		if ( w1.front() == ')' ) { abc = false ; }
		if ( abc )
		{
			if ( w1 == "PDC" )
			{
				if ( temp2 != "" )
				{
					src.push_back( temp2 ) ;
				}
				temp2 = pline ;
				continue ;
			}
			if ( w1 == "ACTION" )
			{
				if ( temp2 == "" )
				{
					err.seterrid( "PSYE041V", name, w2 ) ;
					err.setsrc( trim( pline ) ) ;
					panl.close() ;
					return ;
				}
				temp2 = temp2 + " " + trim( pline ) ;
				src.push_back( temp2 ) ;
				temp2 = "" ;
				continue ;
			}
		}
		else if ( temp2 != "" )
		{
			src.push_back( temp2 ) ;
			temp2 = "" ;
		}
		w2 = word( pline, 2 ) ;
		w3 = word( pline, 3 ) ;
		if ( w2 == "=" && w3.compare( 0, 5, "TRANS" ) == 0 && pline.back() != ')' )
		{
			split1 = true  ;
			temp1  = pline ;
			continue       ;
		}
		if ( w1 == ")END" )        { break                      ; }
		if ( w1 == ")COMMENT" )    { comment = true  ; continue ; }
		if ( w1 == ")ENDCOMMENT" ) { comment = false ; continue ; }
		if ( comment )             { continue                   ; }
		if ( w1 == ")ABC" )        { abc = true                 ; }
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

	if ( temp2 != "" )
	{
		src.push_back( temp2 ) ;
	}

	if ( panl.bad() )
	{
		err.seterrid( "PSYE041N", type, filename ) ;
	}
	panl.close() ;
}


void pPanel::createPanel_If( errblock& err, parser& v, panstmnt* m_stmnt, bool init )
{
	// The if-statement may have an inline statement but the address is in the same location.

	token t ;

	IFSTMNT* m_if = new IFSTMNT ;

	m_if->parse( err, v ) ;
	if ( err.error() )
	{
		delete m_if ;
		return ;
	}

	switch ( v.getStatementType() )
	{
	case ST_ASSIGN:
		createPanel_Assign( err, v, m_stmnt ) ;
		break ;

	case ST_EOF:
		break ;

	case ST_REFRESH:
		if ( init )
		{
			err.seterrid( "PSYE041U" ) ;
			break ;
		}
		createPanel_Refresh( err, v, m_stmnt ) ;
		break ;

	case ST_EXIT:
		m_stmnt->ps_exit = true ;
		break ;

	case ST_GOTO:
		t = v.getToken( 1 ) ;
		m_stmnt->ps_goto = true ;
		if ( !isvalidName( t.value ) )
		{
			err.seterrid( "PSYE041Z", t.value ) ;
			break ;
		}
		m_stmnt->ps_label = t.value ;
		break ;

	case ST_VERIFY:
		createPanel_Verify( err, v, m_stmnt ) ;
		break ;

	case ST_VGET:
	case ST_VPUT:
		createPanel_Vputget( err, v, m_stmnt ) ;
		break ;

	case ST_IF:
	case ST_ELSE:
	case ST_ERROR:
		err.seterrid( "PSYE041E" ) ;
	}
	if ( err.error() )
	{
		delete m_if ;
	}
	else
	{
		m_stmnt->ps_if = m_if ;
	}
	return ;
}


void pPanel::createPanel_Else( errblock& err, parser& v, panstmnt* m_stmnt, vector<panstmnt* >* p_stmnt, bool init )
{
	// The else-statement may have an inline statement but the address is in the same location.

	bool found = false ;

	token t ;

	vector<panstmnt* >::iterator ips ;

	if ( p_stmnt->size() == 0 )
	{
		err.seterrid( "PSYE041T", d2ds( m_stmnt->ps_column+1 ) ) ;
		return ;
	}

	ips = p_stmnt->begin() ;
	advance( ips, p_stmnt->size() ) ;

	do
	{
		ips-- ;
		if ( (*ips)->ps_if && (*ips)->ps_column == m_stmnt->ps_column )
		{
			m_stmnt->ps_else = (*ips)->ps_if ;
			p_stmnt->push_back( m_stmnt ) ;
			found = true ;
			break        ;
		}
		if ( (*ips)->ps_column <= m_stmnt->ps_column )
		{
			break ;
		}
	} while ( ips != p_stmnt->begin() ) ;

	if ( !found )
	{
		err.seterrid( "PSYE041T", d2ds( m_stmnt->ps_column+1 ) ) ;
		return ;
	}

	v.eraseTokens( 0 ) ;
	switch ( v.getStatementType() )
	{
	case ST_EOF:
		break ;

	case ST_VGET:
	case ST_VPUT:
		createPanel_Vputget( err, v, m_stmnt ) ;
		break ;

	case ST_ASSIGN:
		createPanel_Assign( err, v, m_stmnt ) ;
		break ;

	case ST_REFRESH:
		if ( init )
		{
			err.seterrid( "PSYE041U" ) ;
			break ;
		}
		createPanel_Refresh( err, v, m_stmnt ) ;
		break ;

	case ST_EXIT:
		m_stmnt->ps_exit = true ;
		break ;

	case ST_GOTO:
		t = v.getToken( 1 ) ;
		if ( !isvalidName( t.value ) )
		{
			err.seterrid( "PSYE041Z", t.value ) ;
			break ;
		}
		m_stmnt->ps_goto  = true ;
		m_stmnt->ps_label = t.value ;
		break ;

	case ST_VERIFY:
		createPanel_Verify( err, v, m_stmnt ) ;
		break ;

	case ST_IF:
	case ST_ELSE:
	case ST_ERROR:
		err.seterrid( "PSYE041E" ) ;
	}
}


void pPanel::createPanel_Assign( errblock& err, parser& v, panstmnt* m_stmnt )
{
	ASSGN* m_assgn = new ASSGN ;

	m_assgn->parse( err, v ) ;
	if ( err.error() )
	{
		delete m_assgn ;
		return ;
	}
	if ( m_assgn->as_isattr && ( fieldList.find( m_assgn->as_lhs ) == fieldList.end() ) )
	{
		if ( wordpos( m_assgn->as_lhs, tb_fields ) == 0 )
		{
			err.seterrid( "PSYE041S", m_assgn->as_lhs ) ;
			delete m_assgn ;
			return ;
		}
		else
		{
			m_assgn->as_istb = true ;
		}
	}
	m_stmnt->ps_assgn = m_assgn ;
}


void pPanel::createPanel_Verify( errblock& err, parser& v, panstmnt* m_stmnt )
{
	VERIFY* m_VER = new VERIFY ;

	m_VER->parse( err, v ) ;
	if ( err.error() )
	{
		delete m_VER ;
		return ;
	}
	if ( wordpos( m_VER->ver_var, tb_fields ) > 0 )
	{
		m_VER->ver_tbfield = true ;
	}
	m_VER->ver_pnfield = ( fieldList.count( m_VER->ver_var ) > 0 || m_VER->ver_tbfield ) ;
	m_stmnt->ps_ver    = m_VER ;
}


void pPanel::createPanel_Vputget( errblock& err, parser& v, panstmnt* m_stmnt )
{
	VPUTGET* m_VPG = new VPUTGET ;

	m_VPG->parse( err, v ) ;
	if ( err.error() )
	{
		delete m_VPG ;
		return ;
	}
	m_stmnt->ps_vputget = m_VPG ;
}


void pPanel::createPanel_Refresh( errblock& err, parser& v, panstmnt* m_stmnt )
{
	token t ;

	v.getFirstToken() ;
	v.getNextToken()  ;

	if ( v.getNextIfCurrent( TS_OPEN_BRACKET ) )
	{
		while ( true )
		{
			if ( v.getNextIfCurrent( TS_COMMA ) )
			{
				continue ;
			}
			else if ( v.getNextIfCurrent( TS_CLOSE_BRACKET ) )
			{
				if ( m_stmnt->ps_rlist == "" )
				{
					err.seterrid( "PSYE031G" ) ;
					return ;
				}
				break ;
			}
			t = v.getCurrentToken() ;
			if ( t.subtype == TS_NAME )
			{
				if ( fieldList.count( t.value ) == 0 )
				{
					err.seterrid( "PSYE041X", t.value ) ;
					return ;
				}
				if ( m_stmnt->ps_rlist != "*" )
				{
					m_stmnt->ps_rlist += " " + t.value ;
				}
			}
			else
			{
				if ( t.value != "*" )
				{
					err.seterrid( "PSYE041W", t.value ) ;
					return ;
				}
				m_stmnt->ps_rlist = "*" ;
			}
			v.getNextToken() ;
		}
	}
	else
	{
		t = v.getCurrentToken() ;
		if ( t.subtype == TS_NAME )
		{
			if ( fieldList.count( t.value ) == 0 )
			{
				err.seterrid( "PSYE041X", t.value ) ;
				return ;
			}
		}
		else if ( t.value != "*" )
		{
			err.seterrid( "PSYE041W", t.value ) ;
			return ;
		}
		m_stmnt->ps_rlist = t.value ;
		v.getNextToken() ;
	}

	if ( !v.isCurrentType( TT_EOT ) )
	{
		t = v.getCurrentToken() ;
		err.seterrid( "PSYE032H", t.value ) ;
		return ;
	}
	m_stmnt->ps_refresh = true ;
}
