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
	int k  ;

	uint i ;
	uint j ;

	size_t p1 ;

	string ww ;
	string w1 ;
	string w6 ;
	string w7 ;
	string ws ;
	string t1 ;
	string t2 ;
	string pline    ;
	string oline    ;
	string fld, hlp ;
	string abc_desc ;

	bool abar    = false ;
	bool attr    = false ;
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

	map<string, field*>::iterator it1;

	char_attrs attrchar ;

	err.setpanelsrc() ;

	parser panelLang ;
	panelLang.optionUpper() ;

	readPanel( err, pSource, p_name, paths, p_name ) ;
	if ( err.error() ) { return ; }

	for ( it = pSource.begin() ; it != pSource.end() ; ++it )
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
			abar    = false ;
			attr    = false ;
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
				err.setsrc( oline ) ;
				return ;
			}
			abar = true ;
			continue ;
		}
		if ( w1 == ")ATTR" )
		{
			if ( words( pline ) > 1 )
			{
				err.seterrid( "PSYE032H", subword( pline, 2  ) ) ;
				err.setsrc( oline ) ;
				return ;
			}
			attr = true ;
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
			p_version = ds2d( word( substr( pline, i+9 ), 1 ) ) ;
			p_format  = ds2d( word( substr( pline, j+8 ), 1 ) ) ;
			ws = parseString( err, pline, "KEYLIST()" ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			if ( ws != "" )
			{
				p1 = ws.find( ',' ) ;
				if ( p1 == string::npos )
				{
					err.seterrid( "PSYE011B" ) ;
					err.setsrc( oline ) ;
					return ;
				}
				keylistn = strip( ws.substr( 0, p1 ) ) ;
				keyappl  = strip( ws.substr( p1+1  ) ) ;
				if ( !isvalidName( keylistn ) || !isvalidName4( keyappl ) )
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
				p1 = ws.find( ',' ) ;
				if ( p1 == string::npos )
				{
					err.seterrid( "PSYE011D" ) ;
					err.setsrc( oline ) ;
					return ;
				}
				t1 = strip( ws.substr( 0, p1 ) ) ;
				t2 = strip( ws.substr( p1+1  ) ) ;
				win_width = ds2d( t1 ) ;
				if ( win_width > zscrmaxw - 1 )
				{
					err.seterrid( "PSYE011E" ) ;
					err.setsrc( oline ) ;
					return ;
				}
				win_depth = ds2d( t2 ) ;
				if ( win_depth > zscrmaxd - 2 )
				{
					err.seterrid( "PSYE011F" ) ;
					err.setsrc( oline ) ;
					return ;
				}
				pwin   = newwin( win_depth, win_width, 0, 0 ) ;
				bwin   = newwin( win_depth+2, win_width+2, 0, 0 ) ;
				bpanel = new_panel( bwin ) ;
				set_panel_userptr( bpanel, new panel_data( zscrnum ) ) ;
				wscrmaxw = win_width ;
				wscrmaxd = win_depth ;
			}
			cmdfield = parseString( err, pline, "CMD()" ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			if ( cmdfield != "" && !isvalidName( cmdfield ) )
			{
				err.seterrid( "PSYE022J", cmdfield, "CMD field" ) ;
				err.setsrc( oline ) ;
				return ;
			}
			home = parseString( err, pline, "HOME()" ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			if ( home != "" && !isvalidName( home ) )
			{
				err.seterrid( "PSYE022J", home, "HOME field" ) ;
				err.setsrc( oline ) ;
				return ;
			}
			ws = parseString( err, pline, "SCROLL()" ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			if ( ws != "" )
			{
				if ( !isvalidName( ws ) )
				{
					err.seterrid( "PSYE022J", ws, "SCROLL field" ) ;
					err.setsrc( oline ) ;
					return ;
				}
				scroll = ws ;
			}
			pos_smsg = parseString( err, pline, "SMSG()" ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			if ( pos_smsg != "" && !isvalidName( pos_smsg ) )
			{
				err.seterrid( "PSYE022J", pos_smsg, "SMSG field" ) ;
				err.setsrc( oline ) ;
				return ;
			}
			pos_lmsg = parseString( err, pline, "LMSG()" ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			if ( pos_lmsg != "" && !isvalidName( pos_lmsg ) )
			{
				err.seterrid( "PSYE022J", pos_lmsg, "LMSG field" ) ;
				err.setsrc( oline ) ;
				return ;
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

		if ( abar )
		{
			if ( w1 == "PDC" )
			{
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
			panstmnt* m_stmnt = new panstmnt ;
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
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
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
				createPanel_Assign( err,
						    panelLang,
						    m_stmnt,
						    ( init || reinit || proc ) ) ;
				if ( err.error() )
				{
					err.setsrc( oline ) ;
					delete m_stmnt ;
					return ;
				}
				p_stmnt->push_back( m_stmnt )  ;
				break ;

			case ST_IF:
				createPanel_If( err,
						panelLang,
						m_stmnt,
						( init || abcinit ),
						( init || reinit || proc ) ) ;
				if ( err.error() )
				{
					err.setsrc( oline ) ;
					delete m_stmnt ;
					return ;
				}
				p_stmnt->push_back( m_stmnt ) ;
				break ;

			case ST_ELSE:
				createPanel_Else( err,
						  panelLang,
						  m_stmnt,
						  p_stmnt,
						  ( init || abcinit ),
						  ( init || reinit || proc ) ) ;
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
					err.seterrid( "PSYE041Z", tx.value1 ) ;
					err.setsrc( oline ) ;
					delete m_stmnt ;
					return ;
				}
				m_stmnt->ps_label = tx.value1 ;
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
				for ( i = 0 ; i < textList.size() ; ++i )
				{
					if ( m_pnts.pnts_field == textList.at( i )->text_name )
					{
						break ;
					}
				}
				if ( i == textList.size() )
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
			for ( j = words( t_fe.fieldExc_passed ), i = 1 ; i <= j ; ++i )
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

		if ( attr )
		{
			if ( w1.size() > 2 || ( w1.size() == 2 && not ishex( w1 ) ) )
			{
				err.seterrid( "PSYE041B" ) ;
				err.setsrc( oline ) ;
				return ;
			}
			ww = subword( pline, 2 ) ;
			attrchar.setattr( err, ww ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			char c = ( w1.size() == 2 ) ? xs2cs( w1 ).front() : w1.front() ;
			if ( char_attrlist.count( c ) > 0 )
			{
				err.seterrid( "PSYE011H", isprint( c ) ? string( 1, c ) : cs2xs( c ) ) ;
				err.setsrc( oline ) ;
				return ;
			}
			char_attrlist[ c ]   = attrchar ;
			colour_attrlist[ c ] = attrchar.get_colour() ;
			switch ( attrchar.get_type() )
			{
			case DATAIN:
				da_dataIn += string( 1, c ) ;
				ddata_map[ c ] = attrchar.get_colour() ;
				break ;

			case DATAOUT:
				da_dataOut += string( 1, c ) ;
				ddata_map[ c ] = attrchar.get_colour() ;
				break ;

			case CHAR:
				if ( (unsigned char)c > 250 )
				{
					err.seterrid( "PSYE011I" ) ;
					err.setsrc( oline ) ;
					return ;
				}
				if ( schar_map.size() > 127 )
				{
					err.seterrid( "PSYE011J" ) ;
					err.setsrc( oline ) ;
					return ;
				}
				schar_map[ c ] = attrchar.get_colour() ;
				break ;

			default:
				break ;
			}
			continue ;
		}

		if ( !body )
		{
			err.seterrid( "PSYE041E" ) ;
			err.setsrc( oline ) ;
			return ;
		}
		if ( w1 == "ACTIONBAR" )
		{
			vector<abc*> temp ;
			vector<abc*>::iterator it ;
			vector<string> vec ;
			qwords( err, pline, vec ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				return ;
			}
			abc_pos = 2 ;
			for ( i = 1 ; i < vec.size() ; ++i )
			{
				for ( it = ab.begin() ; it != ab.end() ; ++it )
				{
					if ( (*it)->get_abc_desc() != vec[ i ] ) { continue ; }
					(*it)->abc_col = abc_pos ;
					abc_pos += (*it)->abc_desc.size() + 2 ;
					temp.push_back( *it ) ;
					break ;
				}
				if ( it == ab.end() )
				{
					err.seterrid( "PSYE011K", vec[ i ] ) ;
					err.setsrc( oline ) ;
					return ;
				}
				ab.erase( it ) ;
			}
			for_each( ab.begin(), ab.end(),
				[](abc* a)
				{
					delete a ;
				} ) ;
			ab = temp ;
			continue ;
		}
		else if ( w1 == "PANELTITLE" )
		{
			panelTitle = strip( strip( subword( pline, 2 ) ), 'B', '"' ) ;
			continue ;
		}
		else if ( w1 == "PANELDESC" )
		{
			panelDesc = strip( strip( subword( pline, 2 ) ), 'B', '"' ) ;
			continue ;
		}
		else if ( w1 == "LITERAL" || w1 == "TEXT" )
		{
			text* m_lit = new text ;
			m_lit->text_init( err, wscrmaxw, wscrmaxd, opt_field, pline ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				delete m_lit ;
				return ;
			}
			textList.push_back( m_lit ) ;
			if ( m_lit->text_cua == PS )
			{
				textPS.push_back( m_lit ) ;
			}
			continue ;
		}
		else if ( w1 == "FIELD" )
		{
			field* fld = new field ;
			fld->field_init( err, wscrmaxw, wscrmaxd, upper( pline ) ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				delete fld ;
				return ;
			}
			w7 = err.getUserData() ;
			if ( !isvalidName( w7 ) )
			{
				err.seterrid( "PSYE022J", w7, "field" ) ;
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

			if ( ( pos_smsg == w7 || pos_lmsg == w7 ) && attrUnprot.count( fld->field_cua ) > 0 )
			{
				err.seterrid( "PSYE042K", ( pos_smsg == w7 ) ? "SMSG" : "LMSG" ) ;
				err.setsrc( oline ) ;
				delete fld ;
				return ;
			}

			if ( fld->field_cua == PS )
			{
				fieldPS[ w7 ] = fld ;
			}

			fieldList[ w7 ] = fld ;
			continue ;
		}
		else if ( w1 == "DYNAREA" )
		{
			w6 = word( pline, 6 ) ;
			if ( !isvalidName( w6 ) )
			{
				err.seterrid( "PSYE022J", w7, "dynamic area" ) ;
				err.setsrc( oline ) ;
				return ;
			}

			if ( dynAreaList.count( w6 ) > 0 )
			{
				err.seterrid( "PSYE041C", "dynamic area", w7 ) ;
				err.setsrc( oline ) ;
				return ;
			}

			dynArea* m_dynArea = new dynArea ;
			m_dynArea->dynArea_init( err, wscrmaxw, wscrmaxd, upper( pline ), da_dataIn, da_dataOut ) ;
			if ( err.error() )
			{
				err.setsrc( oline ) ;
				delete m_dynArea ;
				return ;
			}

			if ( scrollOn && m_dynArea->dynArea_scroll )
			{
				err.seterrid( "PSYE042R" ) ;
				err.setsrc( oline ) ;
				delete m_dynArea ;
				return ;
			}

			if ( not scrollOn )
			{
				scrollOn = m_dynArea->dynArea_scroll ;
			}

			dyn_width = m_dynArea->dynArea_width ;
			dyn_depth = m_dynArea->dynArea_depth ;

			dynAreaList[ w6 ] = m_dynArea ;
			field a ;
			a.field_cua     = AB ;
			a.field_col     = m_dynArea->dynArea_col   ;
			a.field_length  = m_dynArea->dynArea_width ;
			a.field_endcol  = m_dynArea->dynArea_col + m_dynArea->dynArea_width ;
			a.field_dynArea = m_dynArea ;
			for ( i = 0 ; i < m_dynArea->dynArea_depth ; ++i )
			{
				field* fld     = new field ;
				*fld           = a         ;
				fld->field_row = m_dynArea->dynArea_row + i ;
				fieldList[ w6 + "." + d2ds( i ) ] = fld ;
			}
			continue ;
		}
		else if ( w1 == "BOX" )
		{
			Box* m_box = new Box ;
			m_box->box_init( err, wscrmaxw, wscrmaxd, pline ) ;
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
			iupper( pline ) ;
			ww = word( pline, 3 ) ;
			uint start_row = ds2d( word( pline, 2 ) ) - 1 ;

			if ( isnumeric( ww ) )                      { tb_depth = ds2d( ww ) ; }
			else if ( ww == "MAX" )                     { tb_depth = wscrmaxd - start_row ; }
			else if ( ww.compare( 0, 4, "MAX-" ) == 0 ) { tb_depth = wscrmaxd - ds2d( substr( ww, 5 ) ) - start_row ; }
			else
			{
				err.seterrid( "PSYE031B", ww ) ;
				err.setsrc( oline ) ;
				return ;
			}
			if ( scrollOn )
			{
				err.seterrid( "PSYE042S" ) ;
				err.setsrc( oline ) ;
				return ;
			}
			scrollOn  = true ;
			tb_model  = true ;
			tb_toprow = start_row ;
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
			for ( j = words( tb_clear ), i = 1 ; i <= j ; ++i )
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
			if ( (start_row + tb_depth ) > wscrmaxd ) { tb_depth = (wscrmaxd - start_row) ; }
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
		it1 = fieldList.find( scroll ) ;
		if ( it1 == fieldList.end() )
		{
			err.seterrid( "PSYE042M", scroll, "Scroll" ) ;
			return ;
		}
		it1->second->field_caps = true ;
	}

	if ( home != "" && fieldList.count( home ) == 0 )
	{
		err.seterrid( "PSYE042M", home, "Home" ) ;
		return ;
	}

	if ( cmdfield != "" && fieldList.count( cmdfield ) == 0 )
	{
		err.seterrid( "PSYE042M", cmdfield, "Command" ) ;
		return ;
	}

	if ( pos_smsg != "" && fieldList.count( pos_smsg ) == 0 )
	{
		err.seterrid( "PSYE042M", pos_smsg, "Short message" ) ;
		return ;
	}

	if ( pos_lmsg != "" && fieldList.count( pos_lmsg ) == 0 )
	{
		err.seterrid( "PSYE042M", pos_lmsg, "Long message" ) ;
		return ;
	}

	if ( cmdfield == "" && fieldList.count( "ZCMD" ) > 0 )
	{
		cmdfield = "ZCMD" ;
	}

	if ( home == "" && cmdfield != "" )
	{
		home = cmdfield ;
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

	if ( Rexx )
	{
		for ( it1 = fieldList.begin() ; it1 != fieldList.end() ; ++it1 )
		{
			if ( !p_funcPOOL->ifexists( err, it1->first ) ) { syncDialogueVar( err, it1->first ) ; }
		}
	}

	for ( i = 0 ; i < 251 ; ++i )
	{
		unsigned char c = i ;
		if ( schar_map.count( c ) == 0 )
		{
			def_schar = c ;
			break ;
		}
	}

	for ( auto ita = fieldList.begin() ; ita != fieldList.end() ; ++ita )
	{
		if ( not ita->second->field_dynArea &&
		     not ita->second->field_tb      &&
		     not ita->second->field_nojump  &&
			 ita->second->field_input   &&
			 cmdfield != ita->first     &&
			 ita->second->field_length > 1 )
		{
			for ( auto itb = textList.begin() ; itb != textList.end() ; ++itb )
			{
				if ( ita->second->field_row == (*itb)->text_row &&
				     ita->second->field_col == (*itb)->text_endcol + 1 )
				{
					k = (*itb)->text_value.size() - 3 ;
					if ( k > 0 )
					{
						t1 = (*itb)->text_value.substr( k, 3 ) ;
						if ( t1 == "..." ||
						     t1 == ". ." ||
						     t1 == " .:" ||
						     t1 == ". :" ||
						     t1 == "==>" )
						{
							ita->second->field_jump = true ;
							jump_fields             = true ;
						}
					}
					break ;
				}
			}
		}
	}

	fwin  = newwin( zscrmaxd, zscrmaxw, 0, 0 ) ;
	panel = new_panel( fwin ) ;
	set_panel_userptr( panel, new panel_data( zscrnum ) ) ;

	panelid = p_name ;
}


void pPanel::readPanel( errblock& err,
			vector<string>& src,
			const string& name,
			const string& paths,
			string slist )
{
	int i ;
	int p ;

	size_t p1 ;
	size_t p2 ;

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

	for ( p = getpaths( paths ), i = 1 ; i <= p ; ++i )
	{
		filename = getpath( paths, i ) + name ;
		try
		{
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
		catch ( boost::filesystem::filesystem_error &e )
		{
			err.seterrid( "PSYS012C", e.what() ) ;
			return ;
		}
		catch (...)
		{
			err.seterrid( "PSYS012C", "Entry: "+ filename ) ;
			return ;
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
		p1 = pline.find_first_not_of( ' ' ) ;
		if ( p1 == string::npos ) { continue ; }
		if ( pline.compare( p1, 2, "--" ) == 0 || pline.compare( p1, 1, "#" ) == 0 ) { continue ; }
		p1 = pline.find( "/*", p1 ) ;
		if ( p1 != string::npos )
		{
			p2 = pline.find( "*/" ) ;
			if ( p2 != string::npos ) { pline.replace( p1, p2-p1+2, p2-p1+2, ' ' ) ; }
		}
		trim_right( pline ) ;
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
			if ( findword( w2, slist ) )
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


void pPanel::createPanel_If( errblock& err,
			     parser& v,
			     panstmnt* m_stmnt,
			     bool init,
			     bool init_reinit_proc )
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
		createPanel_Assign( err, v, m_stmnt, init_reinit_proc ) ;
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
		if ( !isvalidName( t.value1 ) )
		{
			err.seterrid( "PSYE041Z", t.value1 ) ;
			break ;
		}
		m_stmnt->ps_label = t.value1 ;
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
}


void pPanel::createPanel_Else( errblock& err,
			       parser& v,
			       panstmnt* m_stmnt,
			       vector<panstmnt* >* p_stmnt,
			       bool init,
			       bool init_reinit_proc )
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

	ips = p_stmnt->begin() + p_stmnt->size() ;

	do
	{
		--ips ;
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
		createPanel_Assign( err, v, m_stmnt, init_reinit_proc ) ;
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
		if ( !isvalidName( t.value1 ) )
		{
			err.seterrid( "PSYE041Z", t.value1 ) ;
			break ;
		}
		m_stmnt->ps_goto  = true ;
		m_stmnt->ps_label = t.value1 ;
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


void pPanel::createPanel_Assign( errblock& err,
				 parser& v,
				 panstmnt* m_stmnt,
				 bool init_reinit_proc )
{
	ASSGN* m_assgn = new ASSGN ;

	m_assgn->parse( err, v ) ;
	if ( err.error() )
	{
		delete m_assgn ;
		return ;
	}

	m_assgn->as_istb = ( tb_fields.count( m_assgn->as_lhs ) > 0 ) ;

	if ( m_assgn->as_isattr && ( fieldList.find( m_assgn->as_lhs ) == fieldList.end() ) && !m_assgn->as_istb )
	{
		err.seterrid( "PSYE041S", m_assgn->as_lhs ) ;
		delete m_assgn ;
		return ;
	}

	if ( m_assgn->as_isattc && not init_reinit_proc )
	{
		err.seterrid( "PSYE042Q", m_assgn->as_lhs ) ;
		delete m_assgn ;
		return ;
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

	m_VER->ver_tbfield = ( tb_fields.count( m_VER->ver_var ) > 0 ) ;
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
				if ( fieldList.count( t.value1 )   == 0 &&
				     tb_fields.count( t.value1 )   == 0 &&
				     dynAreaList.count( t.value1 ) == 0 )
				{
					err.seterrid( "PSYE041X", t.value1 ) ;
					return ;
				}
				if ( m_stmnt->ps_rlist != "*" )
				{
					m_stmnt->ps_rlist += " " + t.value1 ;
				}
			}
			else
			{
				if ( t.value1 != "*" )
				{
					err.seterrid( "PSYE041W", t.value1 ) ;
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
			if ( fieldList.count( t.value1 )   == 0 &&
			     tb_fields.count( t.value1 )   == 0 &&
			     dynAreaList.count( t.value1 ) == 0 )
			{
				err.seterrid( "PSYE041X", t.value1 ) ;
				return ;
			}
		}
		else if ( t.value1 != "*" )
		{
			err.seterrid( "PSYE041W", t.value1 ) ;
			return ;
		}
		m_stmnt->ps_rlist = t.value1 ;
		v.getNextToken() ;
	}

	if ( !v.isCurrentType( TT_EOT ) )
	{
		t = v.getCurrentToken() ;
		err.seterrid( "PSYE032H", t.value1 ) ;
		return ;
	}
	m_stmnt->ps_refresh = true ;
}
