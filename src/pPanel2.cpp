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

void pPanel::load_panel( errblock& err,
			 const string& p_name,
			 const string& paths )
{
	//
	// Load and process panel source.
	//

	TRACE_FUNCTION() ;

	uint i ;
	uint j ;

	uint z1_row ;
	uint z1_col ;
	uint z2_row ;
	uint z2_col ;

	uint cur_section = 0 ;
	uint area_num  = 0 ;
	uint opt_field = 0 ;
	uint rp_field  = 0 ;
	uint abc_mnem  = 0 ;
	uint tb_lline  = 0 ;
	uint tb_lcol   = 0 ;
	uint tb_lsz    = 0 ;
	uint width     = 0 ;

	uint xscrmaxw = zscrmaxw ;

	size_t p1 ;

	string ww ;
	string w1 ;
	string w6 ;
	string w7 ;
	string ws ;
	string t1 ;
	string t2 ;
	string t3 ;
	string pline ;
	string abc_desc ;
	string area_name ;
	string def_attrs( "%+_" ) ;

	const string inv_attrs( "\0 &", 3 ) ;

	bool abar    = false ;
	bool attr    = false ;
	bool body    = false ;
	bool init    = false ;
	bool area    = false ;
	bool abcinit = false ;
	bool reinit  = false ;
	bool proc    = false ;
	bool abcproc = false ;
	bool ishelp  = false ;
	bool ispnts  = false ;
	bool isfield = false ;
	bool nocmdfd = false ;
	bool tbvars  = false ;

	bool ex_panel  = false ;
	bool ex_attr   = false ;
	bool ex_body   = false ;
	bool ex_proc   = false ;
	bool ex_init   = false ;
	bool ex_reinit = false ;
	bool ex_help   = false ;
	bool ex_pnts   = false ;
	bool ex_field  = false ;

	field* pfield ;
	Area*  pArea ;

	vector<string> pSource ;
	vector<string> tSource ;
	vector<string>::iterator it ;

	map<string, field*>::iterator it1;

	err.setpanelsrc() ;

	parser panelLang ;
	panelLang.optionUpper() ;

	read_source( err,
		     tSource,
		     p_name,
		     paths,
		     p_name ) ;
	if ( err.error() ) { return ; }

	if ( err.RC4() )
	{
		convert_source( err, tSource ) ;
		if ( err.error() ) { return ; }
	}

	preprocess_source( err,
			   tSource,
			   pSource,
			   p_name ) ;
	if ( err.error() ) { return ; }

	if ( tb_modlines > 8 )
	{
		err.seterrid( TRACE_INFO(), "PSYE042C" ) ;
		return ;
	}

	for ( it = pSource.begin() ; it != pSource.end() ; ++it )
	{
		pline = *it ;
		err.setsrc( &(*it) ) ;
		if ( pline.find( '\t' ) != string::npos )
		{
			err.seterrid( TRACE_INFO(), "PSYE011A" ) ;
			return ;
		}
		w1 = upper( word( pline, 1 ) ) ;
		if ( w1.front() == ')' )
		{
			abar    = false ;
			attr    = false ;
			body    = false ;
			init    = false ;
			area    = false ;
			abcinit = false ;
			reinit  = false ;
			proc    = false ;
			abcproc = false ;
			ishelp  = false ;
			ispnts  = false ;
			isfield = false ;
		}
		if ( w1 == ")PANEL" )
		{
			if ( cur_section > _PANEL )
			{
				err.seterrid( TRACE_INFO(), "PSYE011T", ")PANEL" ) ;
				return ;
			}
			if ( ex_panel )
			{
				err.seterrid( TRACE_INFO(), "PSYE011Q", ")PANEL" ) ;
				return ;
			}
			cur_section = _PANEL ;
			iupper( pline ) ;
			i = pos( " VERSION=", pline ) ;
			j = pos( " FORMAT=", pline )  ;
			if ( i == 0 || j == 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE011B" ) ;
				return ;
			}
			pline.erase( j, 8 ) ;
			pline.erase( i, 9 ) ;
			p_version = ds2d( word( substr( pline, i+9 ), 1 ) ) ;
			p_format  = ds2d( word( substr( pline, j+8 ), 1 ) ) ;
			idelword( pline, 1, 1 ) ;
			ws = parseString1( err, pline, "KEYLIST()" ) ;
			if ( err.error() ) { return ; }
			if ( pline != "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE032H", pline ) ;
				return ;
			}
			if ( ws != "" )
			{
				replace( ws.begin(), ws.end(), ',', ' ' ) ;
				i        = words( ws ) ;
				keylistn = word( ws, 1 ) ;
				if ( i > 3 || i < 1 || !isvalidName( keylistn ) )
				{
					err.seterrid( TRACE_INFO(), "PSYE011C" ) ;
					return ;
				}
				if ( i > 1 )
				{
					keyappl = word( ws, 2 ) ;
					if ( !isvalidName4( keyappl ) )
					{
						err.seterrid( TRACE_INFO(), "PSYE011C" ) ;
						return ;
					}
					if ( i > 2 )
					{
						if ( word( ws, 3 ) != "SHARED" )
						{
							err.seterrid( TRACE_INFO(), "PSYE011C" ) ;
							return ;
						}
						keyshr = true ;
					}
				}
			}
			ex_panel = true ;
			continue ;
		}
		if ( w1 == ")ATTR" )
		{
			if ( cur_section > _ATTR )
			{
				err.seterrid( TRACE_INFO(), "PSYE011T", ")ATTR" ) ;
				return ;
			}
			cur_section = _ATTR ;
			if ( ex_attr )
			{
				err.seterrid( TRACE_INFO(), "PSYE011Q", ")ATTR" ) ;
				return ;
			}
			idelword( pline, 1, 1 ) ;
			t1 = parseString1( err, pline, "DEFAULT()" ) ;
			if ( err.error() ) { return ; }
			if ( pline != "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE032H", pline ) ;
				return ;
			}
			if ( err.RSN0() )
			{
				if ( t1.size() != 3     ||
				     t1[ 0 ] == t1[ 1 ] ||
				     t1[ 0 ] == t1[ 2 ] ||
				     t1[ 1 ] == t1[ 2 ] )
				{
					err.seterrid( TRACE_INFO(), "PSYE043K" ) ;
					return ;
				}
				if ( t1.find_first_of( inv_attrs ) != string::npos )
				{
					err.seterrid( TRACE_INFO(), "PSYE042I" ) ;
					return ;
				}
				def_attrs = t1 ;
			}
			auto pc = def_attrs.begin() ;
			char_attrlist[ *pc ]     = new char_attrs( err, "TYPE(TEXT) INTENS(HIGH)" ) ;
			char_attrlist[ *(++pc) ] = new char_attrs( err, "TYPE(TEXT) INTENS(LOW)" ) ;
			char_attrlist[ *(++pc) ] = new char_attrs( err, "TYPE(INPUT) INTENS(HIGH)" ) ;
			attr    = true ;
			ex_attr = true ;
			continue ;
		}
		if ( w1 == ")ABC" )
		{
			if ( cur_section >= _BODY )
			{
				err.seterrid( TRACE_INFO(), "PSYE011T", ")ABC" ) ;
				return ;
			}
			cur_section = _ABC ;
			abc_desc = extractKWord( err, pline, "DESC()" ) ;
			if ( err.error() ) { return ; }
			if ( err.RSN4() )
			{
				abc_desc = extractKWord( err, pline, "ABCTEXT()" ) ;
				if ( err.error() ) { return ; }
				if ( abc_desc == "" )
				{
					err.seterrid( TRACE_INFO(), "PSYE011G" ) ;
					return ;
				}
			}
			else if ( abc_desc == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE011G" ) ;
				return ;
			}
			if ( abc_desc.size() > 64 )
			{
				err.seterrid( TRACE_INFO(), "PSYE011O", "ABC" ) ;
				return ;
			}
			t1 = extractKWord( err, pline, "MNEM()" ) ;
			if ( err.error() ) { return ; }
			abc_mnem = ds2d( t1 ) ;
			if ( t1 != "" && ( !datatype( t1, 'W' ) || abc_mnem < 1 || abc_mnem > abc_desc.size() ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE042L", d2ds( abc_desc.size() ) ) ;
				return ;
			}
			if ( pline != ")ABC" )
			{
				err.seterrid( TRACE_INFO(), "PSYE032H", subword( pline, 2  ) ) ;
				return ;
			}
			abar = true ;
			continue ;
		}
		if ( w1 == ")ABCINIT" )
		{
			if ( cur_section >= _BODY )
			{
				err.seterrid( TRACE_INFO(), "PSYE011T", ")ABCINIT" ) ;
				return ;
			}
			if ( cur_section != _ABC )
			{
				err.seterrid( TRACE_INFO(), "PSYE011U", ")ABCINIT", ")ABC" ) ;
				return ;
			}
			cur_section = _ABCINIT ;
			abcinit = true ;
			continue ;
		}
		if ( w1 == ")ABCPROC" )
		{
			if ( cur_section >= _BODY )
			{
				err.seterrid( TRACE_INFO(), "PSYE011T", ")ABCPROC" ) ;
				return ;
			}
			if ( cur_section != _ABCINIT )
			{
				err.seterrid( TRACE_INFO(), "PSYE011U", ")ABCPROC", ")ABCINIT" ) ;
				return ;
			}
			cur_section = _ABCPROC ;
			abcproc = true ;
			continue ;
		}
		if ( w1 == ")BODY" )
		{
			if ( cur_section > _BODY )
			{
				err.seterrid( TRACE_INFO(), "PSYE011T", ")BODY" ) ;
				return ;
			}
			if ( cur_section == _ABC )
			{
				err.seterrid( TRACE_INFO(), "PSYE011V" ) ;
				return ;
			}
			if ( ex_body )
			{
				err.seterrid( TRACE_INFO(), "PSYE011Q", ")BODY" ) ;
				return ;
			}
			cur_section = _BODY ;
			idelword( pline, 1, 1 ) ;
			iupper( pline ) ;
			ws = parseString1( err, pline, "WIDTH()" ) ;
			if ( err.error() ) { return ; }
			if ( ws != "" )
			{
				width = get_value( err, ws, "BODY WIDTH()", t2, 0 ) ;
				if ( err.error() ) { return ; }
				if ( t2 != "" )
				{
					winvars[ t2 ] = width ;
				}
				if ( width < 80 || width > zscrmaxw )
				{
					err.seterrid( TRACE_INFO(), "PSYE011X" ) ;
					return ;
				}
				zscrmaxw = width ;
				wscrmaxw = width ;
			}
			ws = parseString1( err, pline, "WINDOW()" ) ;
			if ( err.error() ) { return ; }
			if ( ws != "" )
			{
				p1 = ws.find( ',' ) ;
				if ( p1 == string::npos )
				{
					if ( words( ws ) == 2 )
					{
						t1 = word( ws, 1 ) ;
						t2 = word( ws, 2 ) ;
					}
					else
					{
						err.seterrid( TRACE_INFO(), "PSYE012A" ) ;
						return ;
					}
				}
				else
				{
					t1 = strip( ws.substr( 0, p1 ) ) ;
					t2 = strip( ws.substr( p1+1  ) ) ;
				}
				win_width = get_value( err, t1, "BODY WINDOW()", t3, zscrmaxw ) ;
				if ( err.error() ) { return ; }
				if ( t3 != "" )
				{
					winvars[ t3 ] = win_width ;
				}
				win_depth = get_value( err, t2, "BODY WINDOW()", t3, zscrmaxd ) ;
				if ( err.error() ) { return ; }
				if ( t3 != "" )
				{
					winvars[ t3 ] = win_depth ;
				}
				if ( width > 0 && ( win_width > width - 2 ) )
				{
					err.seterrid( TRACE_INFO(), "PSYE011D" ) ;
					return ;
				}
				if ( win_width > zscrmaxw - 2 )
				{
					err.seterrid( TRACE_INFO(), "PSYE011E" ) ;
					return ;
				}
				if ( win_depth > zscrmaxd - 2 )
				{
					err.seterrid( TRACE_INFO(), "PSYE011F" ) ;
					return ;
				}
				pwin   = newwin( win_depth, win_width, 0, 0 ) ;
				bwin   = newwin( win_depth+2, win_width+2, 0, 0 ) ;
				bpanel = new_panel( bwin ) ;
				set_panel_userptr( bpanel, new panel_data( zscrnum, true, this ) ) ;
				wscrmaxw = win_width ;
				wscrmaxd = win_depth ;
			}
			cmdfield = parseString1( err, pline, "CMD()" ) ;
			if ( err.error() ) { return ; }
			if ( err.RSN0() )
			{
				if ( cmdfield == "" )
				{
					nocmdfd = true ;
				}
				else if ( !isvalidName( cmdfield ) )
				{
					err.seterrid( TRACE_INFO(), "PSYE022J", cmdfield, "CMD field" ) ;
					return ;
				}
			}
			pos_smsg = parseString1( err, pline, "SMSG()" ) ;
			if ( err.error() ) { return ; }
			if ( pos_smsg != "" && !isvalidName( pos_smsg ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE022J", pos_smsg, "SMSG field" ) ;
				return ;
			}
			pos_lmsg = parseString1( err, pline, "LMSG()" ) ;
			if ( err.error() ) { return ; }
			if ( pos_lmsg != "" && !isvalidName( pos_lmsg ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE022J", pos_lmsg, "LMSG field" ) ;
				return ;
			}
			t1 = parseString1( err, pline, "DEFAULT()" ) ;
			if ( err.error() ) { return ; }
			if ( err.RSN0() )
			{
				if ( t1.size() != 3     ||
				     t1[ 0 ] == t1[ 1 ] ||
				     t1[ 0 ] == t1[ 2 ] ||
				     t1[ 1 ] == t1[ 2 ] )
				{
					err.seterrid( TRACE_INFO(), "PSYE043K" ) ;
					return ;
				}
				if ( t1.find_first_of( inv_attrs ) != string::npos )
				{
					err.seterrid( TRACE_INFO(), "PSYE042I" ) ;
					return ;
				}
				if ( ex_attr )
				{
					for ( uint i = 0 ; i < def_attrs.size() ; ++i )
					{
						delete char_attrlist[ def_attrs[ i ] ] ;
						char_attrlist.erase( def_attrs[ i ] ) ;
					}
				}
				def_attrs = t1 ;
			}
			if ( err.RSN0() || !ex_attr )
			{
				auto pc = def_attrs.begin() ;
				if ( char_attrlist.count( *pc ) == 0 )
				{
					char_attrlist[ *pc ] = new char_attrs( err, "TYPE(TEXT) INTENS(HIGH)" ) ;
				}
				if ( char_attrlist.count( *(++pc) ) == 0 )
				{
					char_attrlist[ *pc ] = new char_attrs( err, "TYPE(TEXT) INTENS(LOW)" ) ;
				}
				if ( char_attrlist.count( *(++pc) ) == 0 )
				{
					char_attrlist[ *pc ] = new char_attrs( err, "TYPE(INPUT) INTENS(HIGH)" ) ;
				}
			}
			if ( pline != "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE032H", pline ) ;
				return ;
			}
			body    = true ;
			ex_body = true ;
			continue ;
		}
		if ( w1 == ")AREA" )
		{
			if ( cur_section > _AREA )
			{
				err.seterrid( TRACE_INFO(), "PSYE011T", ")AREA" ) ;
				return ;
			}
			cur_section = _AREA ;
			if ( words( pline ) > 2 )
			{
				err.seterrid( TRACE_INFO(), "PSYE032H", subword( pline, 3 ) ) ;
				return ;
			}
			t1 = upper( word( pline, 2 ) ) ;
			if ( t1 == "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE032P" ) ;
				return ;
			}
			if ( !isvalidName( t1 ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE032Q", t1 ) ;
				return ;
			}
			if ( AreaList.count( t1 ) == 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE042T", t1 ) ;
				return ;
			}
			area      = true ;
			area_name = t1 ;
			opt_field = 0  ;
			rp_field  = 0  ;
			if ( Area1 == "" ) { Area1 = t1 ; }
			continue ;
		}
		if ( w1.front() == ')' && words( pline ) > 1 )
		{
			err.seterrid( TRACE_INFO(), "PSYE032H", subword( pline, 2  ) ) ;
			return ;
		}
		if ( w1 == ")INIT" )
		{
			if ( cur_section > _INIT )
			{
				err.seterrid( TRACE_INFO(), "PSYE011T", ")INIT" ) ;
				return ;
			}
			if ( ex_init )
			{
				err.seterrid( TRACE_INFO(), "PSYE011Q", ")INIT" ) ;
				return ;
			}
			cur_section = _INIT ;
			init    = true ;
			ex_init = true ;
			continue ;
		}
		if ( w1 == ")REINIT" )
		{
			if ( cur_section > _REINIT )
			{
				err.seterrid( TRACE_INFO(), "PSYE011T", ")REINIT" ) ;
				return ;
			}
			if ( ex_reinit )
			{
				err.seterrid( TRACE_INFO(), "PSYE011Q", ")REINIT" ) ;
				return ;
			}
			cur_section = _REINIT ;
			reinit    = true ;
			ex_reinit = true ;
			continue ;
		}
		if ( w1 == ")PROC" )
		{
			if ( cur_section > _PROC )
			{
				err.seterrid( TRACE_INFO(), "PSYE011T", ")PROC" ) ;
				return ;
			}
			if ( ex_proc )
			{
				err.seterrid( TRACE_INFO(), "PSYE011Q", ")PROC" ) ;
				return ;
			}
			cur_section = _PROC ;
			proc    = true ;
			ex_proc = true ;
			continue ;
		}
		if ( w1 == ")HELP" )
		{
			if ( cur_section > _HELP )
			{
				err.seterrid( TRACE_INFO(), "PSYE011T", ")HELP" ) ;
				return ;
			}
			if ( ex_help )
			{
				err.seterrid( TRACE_INFO(), "PSYE011Q", ")HELP" ) ;
				return ;
			}
			cur_section = _HELP ;
			ishelp  = true ;
			ex_help = true ;
			continue ;
		}
		if ( w1 == ")PNTS" )
		{
			if ( cur_section > _PNTS )
			{
				err.seterrid( TRACE_INFO(), "PSYE011T", ")PNTS" ) ;
				return ;
			}
			if ( ex_pnts )
			{
				err.seterrid( TRACE_INFO(), "PSYE011Q", ")PNTS" ) ;
				return ;
			}
			cur_section = _PNTS ;
			ispnts  = true ;
			ex_pnts = true ;
			continue ;
		}
		if ( w1 == ")FIELD" )
		{
			if ( cur_section > _FIELD )
			{
				err.seterrid( TRACE_INFO(), "PSYE011T", ")FIELD" ) ;
				return ;
			}
			if ( ex_field )
			{
				err.seterrid( TRACE_INFO(), "PSYE011Q", ")FIELD" ) ;
				return ;
			}
			cur_section = _FIELD ;
			isfield  = true ;
			ex_field = true ;
			continue ;
		}
		if ( abar )
		{
			if ( w1 == "PDC" )
			{
				create_pdc( err,
					    abc_desc,
					    abc_mnem,
					    pline ) ;
				if ( err.error() ) { return ; }
				continue ;
			}
			else
			{
				err.seterrid( TRACE_INFO(), "PSYE041D", w1 ) ;
				return ;
			}
		}

		if ( init || abcinit || reinit || proc || abcproc )
		{
			if ( w1 == "*ENDREXX" )
			{
				err.seterrid( TRACE_INFO(), "PSYE043M" ) ;
				return ;
			}
			panstmnt* m_stmnt = new panstmnt( pline.find_first_not_of( ' ' ) ) ;
			vector<panstmnt*>* p_stmnt ;
			if      ( init )    { p_stmnt = &initstmnts ; }
			else if ( abcinit ) { p_stmnt = &abc_initstmnts[ abc_desc ] ; }
			else if ( abcproc ) { p_stmnt = &abc_procstmnts[ abc_desc ] ; }
			else if ( reinit )  { p_stmnt = &reinstmnts ; }
			else                { p_stmnt = &procstmnts ; }
			if ( w1.back() == ':' && w1.find( '\'' ) == string::npos )
			{
				w1.pop_back() ;
				if ( !isvalidName( w1 ) )
				{
					err.seterrid( TRACE_INFO(), "PSYE041R", w1 )  ;
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
			panelLang.parse( err, pline ) ;
			if ( err.error() ) { return ; }
			token tx ;
			switch ( panelLang.getStatementType() )
			{
			case ST_VGET:
			case ST_VPUT:
				createPanel_Vputget( err, panelLang, m_stmnt ) ;
				if ( err.error() )
				{
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
					delete m_stmnt ;
					return ;
				}
				p_stmnt->push_back( m_stmnt ) ;
				if ( m_stmnt->ps_prexx )
				{
					setup_panelREXX( err, p_stmnt, m_stmnt->ps_prexx, pSource, it ) ;
					if ( err.error() ) { return ; }
				}
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
					delete m_stmnt ;
					return ;
				}
				p_stmnt->push_back( m_stmnt ) ;
				if ( m_stmnt->ps_prexx )
				{
					setup_panelREXX( err, p_stmnt, m_stmnt->ps_prexx, pSource, it ) ;
					if ( err.error() ) { return ; }
				}
				break ;

			case ST_REFRESH:
				if ( init || abcinit )
				{
					err.seterrid( TRACE_INFO(), "PSYE041U" ) ;
					delete m_stmnt ;
					return ;
				}
				createPanel_Refresh( err, panelLang, m_stmnt ) ;
				if ( err.error() )
				{
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
					err.seterrid( TRACE_INFO(), "PSYE041Z", tx.value1 ) ;
					delete m_stmnt ;
					return ;
				}
				m_stmnt->ps_label = tx.value1 ;
				p_stmnt->push_back( m_stmnt ) ;
				break ;

			case ST_VEDIT:
				createPanel_Vedit( err, panelLang, m_stmnt ) ;
				if ( err.error() )
				{
					delete m_stmnt ;
					return ;
				}
				p_stmnt->push_back( m_stmnt ) ;
				break ;

			case ST_PREXX:
				if ( !init && !reinit && !proc )
				{
					err.seterrid( TRACE_INFO(), "PSYE034U" ) ;
					delete m_stmnt ;
					return ;
				}
				createPanel_REXX( err, panelLang, m_stmnt ) ;
				if ( err.error() )
				{
					delete m_stmnt ;
					return ;
				}
				p_stmnt->push_back( m_stmnt ) ;
				setup_panelREXX( err, p_stmnt, m_stmnt->ps_prexx, pSource, it ) ;
				if ( err.error() ) { return ; }
				break ;

			case ST_PANEXIT:
				if ( !init && !reinit && !proc )
				{
					err.seterrid( TRACE_INFO(), "PSYE038U" ) ;
					delete m_stmnt ;
					return ;
				}
				createPanel_PANEXIT( err, panelLang, m_stmnt ) ;
				if ( err.error() )
				{
					delete m_stmnt ;
					return ;
				}
				p_stmnt->push_back( m_stmnt ) ;
				break ;

			case ST_VERIFY:
				createPanel_Verify( err, panelLang, m_stmnt ) ;
				if ( err.error() )
				{
					delete m_stmnt ;
					return ;
				}
				p_stmnt->push_back( m_stmnt ) ;
				break ;

			case ST_EOF:
				delete m_stmnt ;
				break ;

			case ST_ERROR:
				err.seterrid( TRACE_INFO(), "PSYE041E" ) ;
				delete m_stmnt ;
				return ;
			}
			continue ;
		}

		if ( ishelp )
		{
			load_panel_help( err, pline ) ;
			if ( err.error() ) { return ; }
			continue ;
		}

		if ( ispnts )
		{
			load_panel_pnts( err, pline ) ;
			if ( err.error() ) { return ; }
			continue ;
		}

		if ( isfield )
		{
			load_panel_field( err, pline ) ;
			if ( err.error() ) { return ; }
			continue ;
		}

		if ( attr )
		{
			load_panel_attr( err, pline, inv_attrs, def_attrs ) ;
			if ( err.error() ) { return ; }
			continue ;
		}

		if ( area )
		{
			auto it = AreaList.find( area_name ) ;
			pArea   = it->second ;
			if ( w1 == "LITERAL" || w1 == "TEXT" )
			{
				text* m_lit = new text( err,
							pArea->get_width(),
							pArea->get_depth(),
							char_attrlist,
							opt_field,
							rp_field,
							pline,
							pArea->get_num(),
							pArea->get_col() ) ;
				if ( err.error() )
				{
					delete m_lit ;
					return ;
				}
				textList.push_back( m_lit ) ;
				pArea->add( m_lit ) ;
				if ( m_lit->get_type() == PS )
				{
					text_pas.push_back( m_lit ) ;
				}
				else if ( m_lit->get_type() == RP )
				{
					text_rp.push_back( m_lit ) ;
				}
			}
			else if ( w1 == "FIELD" )
			{
				field* fld = new field( err,
							pArea->get_width(),
							pArea->get_depth(),
							char_attrlist,
							upper( pline ),
							pArea->get_num(),
							pArea->get_col() ) ;
				if ( err.error() )
				{
					delete fld ;
					return ;
				}
				w7 = err.getUserData() ;
				if ( !fld->field_validname )
				{
					err.seterrid( TRACE_INFO(), "PSYE022J", w7, "field" ) ;
					delete fld ;
					return ;
				}
				if ( tb_field( w7 ) ||
				     dynAreaList.count( w7 ) > 0 ||
				     AreaList.count( w7 ) > 0 )
				{
					err.seterrid( TRACE_INFO(), "PSYE043D", w7 ) ;
					delete fld ;
					return ;
				}
				if ( fieldList.count( w7 ) > 0 )
				{
					err.seterrid( TRACE_INFO(), "PSYE041C", "field", w7 ) ;
					delete fld ;
					return ;
				}
				add_field( w7, fld ) ;
				pArea->add( w7, fld ) ;
			}
			else if ( w1 == "DYNAREA" )
			{
				iupper( pline ) ;
				w6 = word( pline, 6 ) ;
				if ( tb_field( w6 ) ||
				     fieldList.count( w6 ) > 0 ||
				     AreaList.count( w6 ) > 0 )
				{
					err.seterrid( TRACE_INFO(), "PSYE043D", w6 ) ;
					return ;
				}
				if ( dynAreaList.count( w6 ) > 0 )
				{
					err.seterrid( TRACE_INFO(), "PSYE041C", "dynamic area", w6 ) ;
					return ;
				}
				dynArea* m_dynArea = new dynArea( err,
								  pArea->get_width(),
								  pArea->get_depth(),
								  pline,
								  da_dataIn,
								  da_dataOut,
								  pArea->get_num() ) ;
				if ( err.error() )
				{
					delete m_dynArea ;
					return ;
				}
				if ( m_dynArea->dynArea_scroll )
				{
					err.seterrid( TRACE_INFO(), "PSYE043J" ) ;
					delete m_dynArea ;
					return ;
				}
				pArea->get_num(),
				dynAreaList[ w6 ] = m_dynArea ;
				field a ;
				a.field_da_ext   = new field_ext1 ;
				a.field_col      = m_dynArea->dynArea_col ;
				a.field_area_col = a.field_col ;
				a.field_col     += pArea->get_col() ;
				a.field_length   = m_dynArea->dynArea_width ;
				a.field_endcol   = m_dynArea->dynArea_col + m_dynArea->dynArea_width - 1 ;
				a.field_visible  = false ;
				for ( i = 0 ; i < m_dynArea->dynArea_depth ; ++i )
				{
					field* fld     = new field( a ) ;
					fld->field_row = m_dynArea->dynArea_row + i ;
					m_dynArea->add( (void*)fld ) ;
					fld->field_area_row  = fld->field_row ;
					fld->field_set_index( m_dynArea, i ) ;
					t1 = w6 + "." + d2ds( i ) ;
					add_field( t1, fld ) ;
					pArea->add( t1, fld ) ;
				}
			}
			else
			{
				err.seterrid( TRACE_INFO(), "PSYE041E" ) ;
				return ;
			}
			continue ;
		}

		if ( !body )
		{
			err.seterrid( TRACE_INFO(), "PSYE041E" ) ;
			return ;
		}
		if ( w1 == "ACTIONBAR" )
		{
			vector<abc*> temp ;
			vector<abc*>::iterator it ;
			vector<string> vec ;
			qwords( err, pline, vec ) ;
			if ( err.error() ) { return ; }
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
					err.seterrid( TRACE_INFO(), "PSYE011K", vec[ i ] ) ;
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
		if ( w1 == "PANELTITLE" )
		{
			panelTitle = qstring( err, subword( pline, 2 ) ) ;
			if ( err.error() ) { return ; }
			continue ;
		}
		if ( w1 == "PANELDESC" )
		{
			panelDesc = qstring( err, subword( pline, 2 ) ) ;
			if ( err.error() ) { return ; }
			continue ;
		}
		if ( w1 == "LITERAL" || w1 == "TEXT" )
		{
			text* m_lit = new text( err,
						wscrmaxw,
						wscrmaxd,
						char_attrlist,
						opt_field,
						rp_field,
						pline ) ;
			if ( err.error() )
			{
				delete m_lit ;
				return ;
			}
			textList.push_back( m_lit ) ;
			if ( m_lit->get_type() == PS )
			{
				text_pas.push_back( m_lit ) ;
			}
			else if ( m_lit->get_type() == RP )
			{
				text_rp.push_back( m_lit ) ;
			}
			continue ;
		}
		if ( w1 == "FIELD" )
		{
			field* fld = new field( err,
						wscrmaxw,
						wscrmaxd,
						char_attrlist,
						upper( pline ) ) ;
			if ( err.error() )
			{
				delete fld ;
				return ;
			}
			w7 = err.getUserData() ;
			if ( !fld->field_validname )
			{
				err.seterrid( TRACE_INFO(), "PSYE022J", w7, "field" ) ;
				delete fld ;
				return ;
			}

			if ( tb_field( w7 ) ||
			     dynAreaList.count( w7 ) > 0,
			     AreaList.count( w7 ) > 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE043D", w7 ) ;
				delete fld ;
				return ;
			}
			if ( fieldList.count( w7 ) > 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE041C", "field", w7 ) ;
				delete fld ;
				return ;
			}

			if ( ( pos_smsg == w7 || pos_lmsg == w7 ) && attrUnprot.count( fld->field_get_type() ) > 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE042K", ( pos_smsg == w7 ) ? "SMSG" : "LMSG" ) ;
				delete fld ;
				return ;
			}
			add_field( w7, fld ) ;
			continue ;
		}
		if ( w1 == "AREA" )
		{
			if ( words( pline ) > 6 )
			{
				err.seterrid( TRACE_INFO(), "PSYE032H", subword( pline, 7 ) ) ;
				return ;
			}
			if ( words( pline ) < 6 )
			{
				err.seterrid( TRACE_INFO(), "PSYE032R" ) ;
				return ;
			}
			iupper( pline ) ;
			w6 = word( pline, 6 ) ;
			if ( !isvalidName( w6 ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE042U" ) ;
				return ;
			}
			if ( tb_field( w6 ) ||
			     fieldList.count( w6 ) > 0 ||
			     dynAreaList.count( w6 ) > 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE043D", w7 ) ;
				return ;
			}
			if ( AreaList.count( w6 ) > 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE041C", "AREA", w6 ) ;
				return ;
			}
			Area* m_Area = new Area( err,
						 wscrmaxw,
						 wscrmaxd,
						 ++area_num,
						 pline ) ;
			if ( err.error() )
			{
				delete m_Area ;
				return ;
			}
			AreaList[ w6 ]      = m_Area ;
			AreaNum[ area_num ] = m_Area ;
			continue ;
		}
		if ( w1 == "DYNAREA" )
		{
			iupper( pline ) ;
			w6 = word( pline, 6 ) ;
			if ( tb_field( w6 ) ||
			     fieldList.count( w6 ) > 0 ||
			     AreaList.count( w6 ) > 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE043D", w7 ) ;
				return ;
			}
			if ( dynAreaList.count( w6 ) > 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE041C", "dynamic area", w6 ) ;
				return ;
			}
			if ( tb_field( w6 ) || fieldList.count( w6 ) > 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE043D", w6 ) ;
				return ;
			}
			dynArea* m_dynArea = new dynArea( err,
							  wscrmaxw,
							  wscrmaxd,
							  pline,
							  da_dataIn,
							  da_dataOut ) ;
			if ( err.error() )
			{
				delete m_dynArea ;
				return ;
			}
			if ( m_dynArea->dynArea_scroll )
			{
				if ( scrollOn )
				{
					err.seterrid( TRACE_INFO(), "PSYE042R" ) ;
					delete m_dynArea ;
					return ;
				}
				scrollOn    = true ;
				dyns_depth  = m_dynArea->dynArea_depth ;
				dyns_width  = m_dynArea->dynArea_width ;
				dyns_toprow = m_dynArea->dynArea_row ;
			}
			dynAreaList[ w6 ] = m_dynArea ;
			field a ;
			a.field_da_ext  = new field_ext1 ;
			a.field_col     = m_dynArea->dynArea_col   ;
			a.field_length  = m_dynArea->dynArea_width ;
			a.field_endcol  = m_dynArea->dynArea_col + m_dynArea->dynArea_width - 1 ;
			for ( i = 0 ; i < m_dynArea->dynArea_depth ; ++i )
			{
				field* fld     = new field( a ) ;
				m_dynArea->add( (void*)fld ) ;
				fld->field_row = m_dynArea->dynArea_row + i ;
				fld->field_set_index( m_dynArea, i ) ;
				add_field( w6 + "." + d2ds( i ), fld ) ;
			}
			continue ;
		}
		if ( w1 == "BOX" )
		{
			Box* m_box = new Box( err,
					      wscrmaxw,
					      wscrmaxd,
					      pline ) ;
			if ( err.error() )
			{
				delete m_box ;
				return ;
			}
			boxes.push_back( m_box ) ;
			continue ;
		}
		if ( w1 == "TBMODEL" )
		{
			if ( tb_model )
			{
				if ( words( pline ) > 1 )
				{
					err.seterrid( TRACE_INFO(), "PSYE042A" ) ;
					return ;
				}
				tb_lcol = 0 ;
				tb_lsz  = 0 ;
				tbvars  = false ;
				++tb_lline ;
				continue ;
			}
			iupper( pline ) ;
			ww = word( pline, 3 ) ;
			tb_toprow = ds2d( word( pline, 2 ) ) - 1 ;

			if ( isnumeric( ww ) )                      { tb_pdepth = ds2d( ww ) ; }
			else if ( ww == "MAX" )                     { tb_pdepth = wscrmaxd - tb_toprow ; }
			else if ( ww.compare( 0, 4, "MAX-" ) == 0 ) { tb_pdepth = wscrmaxd - ds2d( substr( ww, 5 ) ) - tb_toprow ; }
			else
			{
				err.seterrid( TRACE_INFO(), "PSYE031S", ww ) ;
				return ;
			}
			tb_rdepth = ( tb_pdepth / tb_modlines ) ;
			if ( tb_toprow > wscrmaxd )
			{
				err.seterrid( TRACE_INFO(), "PSYE031T", d2ds( tb_toprow ), d2ds( wscrmaxd ) ) ;
				return ;
			}
			if ( scrollOn )
			{
				err.seterrid( TRACE_INFO(), "PSYE042S" ) ;
				return ;
			}
			scrollOn = true ;
			tb_model = true ;
			t1 = subword( pline, 4 ) ;
			t2 = parseString1( err, t1, "ROWS()" ) ;
			if ( err.error() ) { return ; }
			if ( t2 != "" && !findword( t2, "SCAN ALL" ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE041Q", t2 ) ;
				return ;
			}
			tb_scan = ( t2 == "SCAN" ) ;
			t2      = parseString1( err, t1, "CLEAR()" ) ;
			if ( err.error() ) { return ; }
			replace( t2.begin(), t2.end(), ',', ' ' ) ;
			for ( j = words( t2 ), i = 1 ; i <= j ; ++i )
			{
				t3 = word( t2, i ) ;
				if ( !isvalidName( t3 ) )
				{
					err.seterrid( TRACE_INFO(), "PSYE013A", "TBMODEL CLEAR", t3 ) ;
					return ;
				}
				tb_clear.insert( t3 ) ;
			}
			if ( t1 != "" )
			{
				err.seterrid( TRACE_INFO(), "PSYE032H", t1 ) ;
				return ;
			}
			if ( ( tb_toprow + tb_pdepth ) > wscrmaxd )
			{
				tb_pdepth = wscrmaxd - tb_toprow ;
			}
			funcPool->put2( err, "ZTDDEPTH", tb_pdepth ) ;
			continue ;
		}
		if ( w1 == "TBFIELD" )
		{
			iupper( pline ) ;
			create_tbfield( err, pline, tb_lline, tb_lcol, tb_lsz, tbvars ) ;
			if ( err.error() ) { return ; }
			continue ;
		}
		if ( w1 == "TBTEXT" )
		{
			create_tbtext( err, pline, tb_lline, tb_lcol, tb_lsz, tbvars ) ;
			if ( err.error() ) { return ; }
			continue ;
		}
		err.seterrid( TRACE_INFO(), "PSYE041D", w1 ) ;
		return ;
	}

	if ( !ex_body )
	{
		err.seterrid( TRACE_INFO(), "PSYE011S", ")BODY" ) ;
		return ;
	}

	err.clearsrc() ;

	check_overlapping_fields( err ) ;
	if ( err.error() ) { return ; }

	if ( AreaList.size() > 0 )
	{
		build_Areas( err ) ;
		if ( err.error() ) { return ; }
	}

	if ( pos_smsg != "" && fieldList.count( pos_smsg ) == 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE042M", pos_smsg, "Short message" ) ;
		return ;
	}

	if ( pos_lmsg != "" && fieldList.count( pos_lmsg ) == 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE042M", pos_lmsg, "Long message" ) ;
		return ;
	}


	if ( cmdfield != "" )
	{
		it1 = fieldList.find( cmdfield ) ;
		if ( it1 == fieldList.end() )
		{
			err.seterrid( TRACE_INFO(), "PSYE042M", cmdfield, "Command" ) ;
			return ;
		}
		else if ( it1->second->field_area != 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE043E" ) ;
			return ;
		}
		else if ( !it1->second->field_is_input() )
		{
			err.seterrid( TRACE_INFO(), "PSYE043L", cmdfield ) ;
			return ;
		}
	}

	z1_row = -1 ;
	z1_col = -1 ;
	z2_row = -1 ;
	z2_col = -1 ;
	t1     = "" ;
	t2     = "" ;
	for ( it1 = fieldList.begin() ; it1 != fieldList.end() ; ++it1 )
	{
		pfield = it1->second ;
		if ( pfield->field_is_scrollable() && pfield->field_has_lcol_var() )
		{
			lcolmmap.insert( make_pair( pfield->field_get_lcol_var(), pfield ) ) ;
		}
		if ( !pfield->field_active     ||
		     !pfield->field_visible    ||
		     !pfield->field_is_input() ||
		     !pfield->field_validname )
		{
			continue ;
		}
		if ( !nocmdfd && cmdfield == "" && pfield->field_area == 0 )
		{
			if ( ( z1_row  > pfield->field_row ) ||
			     ( z1_row == pfield->field_row &&
			       z1_col  > pfield->field_col ) )
			{
				z1_row = pfield->field_row ;
				z1_col = pfield->field_col ;
				t1     = it1->first ;
			}
		}
		if ( ( z2_row  > pfield->field_row ) ||
		     ( z2_row == pfield->field_row &&
		       z2_col  > pfield->field_col ) )
		{
			z2_row = pfield->field_row ;
			z2_col = pfield->field_col ;
			t2     = it1->first ;
		}
	}

	home = t2 ;
	if ( !nocmdfd && cmdfield == "" )
	{
		cmdfield = t1 ;
	}

	if ( scrollOn && cmdfield != "" )
	{
		it1    = fieldList.find( cmdfield ) ;
		t1     = "" ;
		i      = 0  ;
		z1_row = it1->second->field_row ;
		z1_col = it1->second->field_col ;
		z2_col = -1 ;
		for ( it1 = fieldList.begin() ; it1 != fieldList.end() ; ++it1 )
		{
			pfield = it1->second ;
			if ( !pfield->field_active     ||
			     !pfield->field_visible    ||
			     !pfield->field_is_input() ||
			     !pfield->field_validname  ||
			      pfield->field_row != z1_row ||
			      pfield->field_col <= z1_col ||
			      pfield->field_area != 0 )
			{
				continue ;
			}
			if ( z2_col > pfield->field_col )
			{
				z2_col = pfield->field_col ;
				i      = pfield->field_length ;
				t1     = it1->first ;
			}
		}
		if ( i == 4 )
		{
			scroll = t1 ;
		}
	}

	if ( selPanel && cmdfield != "ZCMD" )
	{
		err.seterrid( TRACE_INFO(), "PSYE042P" ) ;
		return ;
	}

	if ( abc_pos > wscrmaxw )
	{
		err.seterrid( TRACE_INFO(), "PSYE011P" ) ;
		return ;
	}

	for ( i = 0 ; i < 251 ; ++i )
	{
		unsigned char c = i ;
		if ( schar_map.count( c ) == 0 )
		{
			inv_schar = c ;
			break ;
		}
	}

	build_fieldMap() ;

	build_jumpList() ;

	fwin  = newwin( zscrmaxd, xscrmaxw, 0, 0 ) ;
	panel = new_panel( fwin ) ;
	set_panel_userptr( panel, new panel_data( zscrnum, this ) ) ;

	panelid = p_name ;
}


void pPanel::load_panel_help( errblock& err,
			      string& pline )
{
	TRACE_FUNCTION() ;

	uint i ;
	uint j ;

	bool found ;

	string t1 ;
	string t2 ;

	help* hlp = new help( err, pline ) ;
	if ( err.error() )
	{
		delete hlp ;
		return ;
	}

	if ( !tb_field( hlp->help_field ) && fieldList.count( hlp->help_field ) == 0 )
	{
		found = false ;
		if ( hlp->help_field.compare( 0, 4, "ZABC" ) == 0 )
		{
			if ( hlp->help_field.size() != 6 )
			{
				err.seterrid( TRACE_INFO(), "PSYE045F" ) ;
				delete hlp ;
				return ;
			}
			t1 = hlp->help_field.substr( 4, 2 ) ;
			if ( !datatype( t1, 'W' ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE045F" ) ;
				delete hlp ;
				return ;
			}
			i = ds2d( t1 ) ;
			if ( i < 1 || i > ab.size() )
			{
				err.seterrid( TRACE_INFO(), "PSYE045F" ) ;
				delete hlp ;
				return ;
			}
			found = true ;
		}
		else if ( hlp->help_field.compare( 0, 4, "ZPDC" ) == 0 )
		{
			if ( hlp->help_field.size() != 8 )
			{
				err.seterrid( TRACE_INFO(), "PSYE045G" ) ;
				delete hlp ;
				return ;
			}
			t1 = hlp->help_field.substr( 4, 2 ) ;
			t2 = hlp->help_field.substr( 6, 2 ) ;
			if ( !datatype( t1, 'W' ) || ( !datatype( t2, 'W' ) ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE045G" ) ;
				delete hlp ;
				return ;
			}
			i = ds2d( t1 ) ;
			j = ds2d( t2 ) ;
			if ( i < 1 || i > ab.size() )
			{
				err.seterrid( TRACE_INFO(), "PSYE045G" ) ;
				delete hlp ;
				return ;
			}
			if ( j < 1 || j > ab[ i - 1 ]->get_pd_rows() )
			{
				err.seterrid( TRACE_INFO(), "PSYE045G" ) ;
				delete hlp ;
				return ;
			}
			found = true ;
		}
		else if ( hlp->help_field.compare( 0, 3, "ZRP" ) == 0 )
		{
			for ( auto it = text_rp.begin() ; it != text_rp.end() ; ++it )
			{
				if ( hlp->help_field == (*it)->text_name )
				{
					found = true ;
					break ;
				}
			}
		}
		if ( !found )
		{
			err.seterrid( TRACE_INFO(), "PSYE045C", hlp->help_field ) ;
			delete hlp ;
			return ;
		}
	}

	field_help[ hlp->help_field ] = hlp ;
}


void pPanel::load_panel_pnts( errblock& err,
			      const string& pline )
{
	TRACE_FUNCTION() ;

	bool found ;

	pnts m_pnts( err, pline ) ;
	if ( err.error() ) { return ; }

	if ( !tb_field( m_pnts.pnts_field ) && fieldList.count( m_pnts.pnts_field ) == 0 )
	{
		found = false ;
		for ( auto it = text_pas.begin() ; it != text_pas.end() ; ++it )
		{
			if ( m_pnts.pnts_field == (*it)->text_name )
			{
				found = true ;
				break ;
			}
		}
		if ( !found )
		{
			err.seterrid( TRACE_INFO(), "PSYE042E", m_pnts.pnts_field ) ;
			return ;
		}
	}

	m_pnts.pnts_fvar = ( fieldList.count( m_pnts.pnts_var ) > 0 ) ;

	pntsTable[ m_pnts.pnts_field ] = m_pnts ;
}


void pPanel::load_panel_field( errblock& err,
			       string& pline )
{
	//
	// There are two forms of the FIELD statement:
	// 1) Field execute entry
	// 2) Scrollable field entry
	//

	TRACE_FUNCTION() ;

	int i ;
	int j ;

	field* pfield ;

	string ww ;

	fieldXct* fld_exc = new fieldXct( err, pline ) ;
	if ( err.error() )
	{
		delete fld_exc ;
		sfield* sfld = new sfield( err, pline ) ;
		sfieldsList.push_back( sfld ) ;
		if ( err.error() ) { return ; }
		auto it = fieldList.find( sfld->field ) ;
		if ( it == fieldList.end() && !tb_field( sfld->field ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE043G", sfld->field ) ;
			return ;
		}
		if ( sfld->ind1 != "" && fieldList.count( sfld->ind1 ) == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE043G", sfld->ind1 ) ;
			return ;
		}
		if ( sfld->lind1 != "" && fieldList.count( sfld->lind1 ) == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE043G", sfld->lind1 ) ;
			return ;
		}
		if ( sfld->rind1 != "" && fieldList.count( sfld->rind1 ) == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE043G", sfld->rind1 ) ;
			return ;
		}
		if ( sfld->sind1 != "" && fieldList.count( sfld->sind1 ) == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE043G", sfld->sind1 ) ;
			return ;
		}
		if ( sfld->scale != "" && fieldList.count( sfld->scale ) == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE043G", sfld->scale ) ;
			return ;
		}
		if ( tb_field( sfld->field ) )
		{
			tblenvars[ sfld->field ] = make_pair( sfld->lenvar, sfld->len ) ;
			for ( int i = 0 ; i < tb_rdepth ; ++i )
			{
				auto it = fieldList.find( sfld->field + "." + d2ds( i ) ) ;
				pfield  = it->second ;
				if ( i == 0 && sfields.count( pfield ) > 0 )
				{
					err.seterrid( TRACE_INFO(), "PSYE042J", sfld->field ) ;
					return ;
				}
				sfields[ pfield ] = make_pair( it->first, sfld ) ;
				pfield->field_add_scroll( sfld ) ;
			}
		}
		else
		{
			pfield = it->second ;
			if ( sfields.count( pfield ) > 0 )
			{
				err.seterrid( TRACE_INFO(), "PSYE042J", sfld->field ) ;
				return ;
			}
			sfields[ pfield ] = make_pair( it->first, sfld ) ;
			pfield->field_add_scroll( sfld ) ;
		}
		return ;
	}

	if ( fieldList.count( fld_exc->fieldXct_field ) == 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE043G", fld_exc->fieldXct_field ) ;
		delete fld_exc ;
		return ;
	}

	for ( j = words( fld_exc->fieldXct_passed ), i = 1 ; i <= j ; ++i )
	{
		ww = word( fld_exc->fieldXct_passed, i ) ;
		if ( fieldList.find( ww ) == fieldList.end() )
		{
			err.seterrid( TRACE_INFO(), "PSYE043H", ww, fld_exc->fieldXct_field ) ;
			delete fld_exc ;
			return ;
		}
	}

	if ( field_xct_table.count( fld_exc->fieldXct_field ) > 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE042J", fld_exc->fieldXct_field ) ;
		delete fld_exc ;
		return ;
	}

	field_xct_table[ fld_exc->fieldXct_field ] = fld_exc ;
}


void pPanel::load_panel_attr( errblock& err,
			      string& pline,
			      const string& inv_attrs,
			      string& def_attrs )
{
	TRACE_FUNCTION() ;

	char c ;

	string w1 ;

	char_attrs* attrchar ;

	w1 = word( pline, 1 ) ;
	if ( w1.size() > 2 || ( w1.size() == 2 && not ishex( w1 ) ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE041B" ) ;
		return ;
	}

	c = ( w1.size() == 2 ) ? xs2cs( w1 ).front() : w1.front() ;
	if ( inv_attrs.find( c ) != string::npos )
	{
		err.seterrid( TRACE_INFO(), "PSYE042I" ) ;
		return ;
	}

	attrchar = new char_attrs( err, subword( pline, 2 ) ) ;
	if ( err.error() )
	{
		delete attrchar ;
		return ;
	}

	if ( char_attrlist.count( c ) > 0 )
	{
		if ( def_attrs.find( c ) != string::npos )
		{
			def_attrs.erase( def_attrs.find( c ), 1 ) ;
			delete char_attrlist[ c ] ;
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE011H", isprint( c ) ? string( 1, c ) : c2xs( c ) ) ;
			delete attrchar ;
			return ;
		}
	}

	char_attrlist[ c ] = attrchar ;
	switch ( attrchar->get_type() )
	{
	case DATAIN:
		da_dataIn.push_back( c ) ;
		ddata_map[ c ] = attrchar->get_colour() ;
		break ;

	case DATAOUT:
		da_dataOut.push_back( c ) ;
		ddata_map[ c ] = attrchar->get_colour() ;
		break ;

	case CHAR:
		if ( (unsigned char)c > 250 )
		{
			err.seterrid( TRACE_INFO(), "PSYE011I" ) ;
			return ;
		}
		if ( schar_map.size() > 127 )
		{
			err.seterrid( TRACE_INFO(), "PSYE011J" ) ;
			return ;
		}
		schar_map[ c ] = attrchar->get_colour() ;
		break ;

	default:
		break ;
	}
}


void pPanel::create_tbfield( errblock& err,
			     const string& pline,
			     uint& tb_lline,
			     uint& tb_lcol,
			     uint& tb_lsz,
			     bool& tbvars )
{
	//
	// Create the field objects for a TBFIELD statement.
	//
	// Also create a tbfield object for any TBFIELD statements that contain variables
	// for the column and/or lengths.
	//

	TRACE_FUNCTION() ;

	int i ;
	int j ;
	int ws ;

	uint tcol ;
	uint tlen ;

	char field_char ;

	bool inlne     = false ;
	bool has_dvars = false ;

	string t    ;
	string w2   ;
	string w3   ;
	string w4   ;
	string rest ;
	string opts ;
	string name ;
	string nidx ;
	string varc ;
	string varl ;

	attType fType ;

	field  a   ;
	field* fld ;

	vector<field*> fields ;

	fields.reserve( tb_rdepth ) ;

	ws = words( pline ) ;
	w2 = word( pline, 2 ) ;
	w3 = word( pline, 3 ) ;
	w4 = word( pline, 4 ) ;

	opts = upper( subword( pline, 5, ws-5 ) ) ;
	name = word( pline, ws ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE031F", name, "TBFIELD" ) ;
		return  ;
	}

	if ( tb_field( name ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE041A", name ) ;
		return  ;
	}

	if ( fieldList.count( name ) > 0 || dynAreaList.count( name ) > 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE043D", name ) ;
		return  ;
	}

	if ( w2.size() > 1 && w2.front() == '+' )
	{
		if ( tbvars )
		{
			err.seterrid( TRACE_INFO(), "PSYE031Z" ) ;
			return ;
		}
		if ( w2.size() > 2 && w2[ 1 ] == '+' )
		{
			tcol = tb_lcol + tb_lsz + ds2d( substr( w2, 3 ) ) ;
		}
		else
		{
			tcol = tb_lcol + ds2d( substr( w2, 2 ) ) ;
		}
	}
	else if ( isnumeric( w2 ) )                 { tcol = ds2d( w2 ) ; }
	else if ( w2 == "MAX" )                     { tcol = wscrmaxw   ; }
	else if ( w2.compare( 0, 4, "MAX-" ) == 0 ) { tcol = wscrmaxw - ds2d( substr( w2, 5 ) ) ; }
	else if ( isvalidName( w2 ) )
	{
		varc      = w2 ;
		tcol      = 0 ;
		tbvars    = true ;
		tb_dvars  = true ;
		has_dvars = true ;
	}
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE031S", w2 ) ;
		return ;
	}

	if ( isnumeric( w3 ) )                      { tlen = ds2d( w3 ) ; }
	else if ( w3 == "MAX" )                     { tlen = wscrmaxw - tcol + 1 ; }
	else if ( w3.compare( 0, 4, "MAX-" ) == 0 ) { tlen = wscrmaxw - tcol - ds2d( substr( w3, 5 ) ) + 1 ; }
	else if ( isvalidName( w3 ) )
	{
		varl      = w3 ;
		tlen      = 0  ;
		tbvars    = true ;
		tb_dvars  = true ;
		has_dvars = true ;
	}
	else
	{
		err.seterrid( TRACE_INFO(), "PSYE031S", w3 ) ;
		return ;
	}

	if ( tcol > wscrmaxw )
	{
		err.seterrid( TRACE_INFO(), "PSYE031B", name, d2ds( tcol ), d2ds( wscrmaxw ) ) ;
		return ;
	}

	tb_lcol = tcol ;
	tb_lsz  = tlen ;

	if ( cuaAttrName.count( w4 ) == 0 )
	{
		rest = subword( pline, 4 ) ;
		if ( rest.compare( 0, 5, "ATTR(" ) != 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE032F", w4 ) ;
			return ;
		}
		t = parseString1( err, rest, "ATTR()" ) ;
		if ( err.error() ) { return ; }
		if ( words( rest ) > 1 )
		{
			err.seterrid( TRACE_INFO(), "PSYE032H", rest ) ;
			return ;
		}
		if ( t == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE035S" ) ;
			return ;
		}
		if ( t.size() > 2 || ( t.size() == 2 && !ishex( t ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE041B" ) ;
			return ;
		}
		field_char = ( t.size() == 2 ) ? xs2cs( t ).front() : t.front() ;
		auto it = char_attrlist.find( field_char ) ;
		if ( it == char_attrlist.end() )
		{
			t = isprint( field_char ) ? string( 1, field_char ) : c2xs( field_char ) ;
			err.seterrid( TRACE_INFO(), "PSYE036D", t ) ;
			return ;
		}
		if ( it->second->is_text() )
		{
			err.seterrid( TRACE_INFO(), "PSYE035T" ) ;
			return ;
		}
		a.field_char_attrs = it->second ;
	}
	else
	{
		if ( ws != 6 )
		{
			err.seterrid( TRACE_INFO(), "PSYE035Q" ) ;
			return ;
		}
		fType = cuaAttrName[ w4 ] ;
		inlne = true ;
	}

	if ( inlne )
	{
		a.field_opts( err, fType, opts ) ;
		if ( err.error() ) { return ; }
	}

	if ( ( varc == "" && varl == "" ) && ( tcol + tlen - 1 ) > wscrmaxw )
	{
		 err.seterrid( TRACE_INFO(), "PSYE031X", name ) ;
		 return ;
	}
	a.field_col    = tcol - 1 ;
	a.field_length = tlen ;
	a.field_endcol = tcol + tlen - 2 ;
	a.field_tb_ext = new field_ext2( name ) ;

	for ( i = tb_lline, j = 0 ; j < tb_rdepth ; i += tb_modlines, ++j )
	{
		fld = new field( a ) ;
		fld->field_row = tb_toprow + i ;
		fld->field_set_index( j ) ;
		nidx = name +"."+ d2ds( j ) ;
		add_field( nidx, fld ) ;
		funcPool->put3( nidx, "" ) ;
		if ( has_dvars )
		{
			fields.push_back( fld ) ;
		}
	}

	tb_fields.insert( name ) ;

	if ( has_dvars )
	{
		tbfields.push_back( new tbfield( name, varc, varl, tcol, tlen, fields ) ) ;
	}
}


void pPanel::create_tbtext( errblock& err,
			    const string& pline,
			    uint& tb_lline,
			    uint& tb_lcol,
			    uint& tb_lsz,
			    bool& tbvars )
{
	//
	// Create the text objects for a TBTEXT statement.
	// These are kept in the tbtextList vector.
	//

	TRACE_FUNCTION() ;

	int i ;
	int j ;
	int l ;

	uint tcol ;

	char c ;

	text m_txt ;

	string t ;
	string w2 ;
	string w3 ;
	string w4 ;
	string rest ;

	w2 = upper( word( pline, 2 ) ) ;
	w3 = upper( word( pline, 3 ) ) ;

	if ( w2.size() > 1 && w2.front() == '+' )
	{
		if ( tbvars )
		{
			err.seterrid( TRACE_INFO(), "PSYE031Z" ) ;
			return ;
		}
		if ( w2.size() > 2 && w2[ 1 ] == '+' )
		{
			tcol = tb_lcol + tb_lsz + ds2d( substr( w2, 3 ) ) ;
		}
		else
		{
			tcol = tb_lcol + ds2d( substr( w2, 2 ) ) ;
		}
	}
	else if ( isnumeric( w2 ) )                 { tcol = ds2d( w2 ) ; }
	else if ( w2 == "MAX" )                     { tcol = wscrmaxd   ; }
	else if ( w2.compare( 0, 4, "MAX-" ) == 0 ) { tcol = wscrmaxd - ds2d( substr( w2, 5 ) )    ; }
	else                                        { err.seterrid( TRACE_INFO(), "PSYE031S", w2 ) ; return ; }

	if ( tcol > wscrmaxw )
	{
		err.seterrid( TRACE_INFO(), "PSYE036B", d2ds( tcol ), d2ds( wscrmaxw ) ) ;
		return ;
	}

	if ( cuaAttrName.count( w3 ) == 0 )
	{
		rest = subword( pline, 3 ) ;
		if ( upper( rest ).compare( 0, 5, "ATTR(" ) != 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE032F", w3 ) ;
			return ;
		}
		t = parseString1( err, rest, "ATTR()" ) ;
		if ( err.error() ) { return ; }
		if ( t == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE036C" ) ;
			return ;
		}
		if ( t.size() > 2 || ( t.size() == 2 && !ishex( t ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE041B" ) ;
			return ;
		}
		c = ( t.size() == 2 ) ? xs2cs( t ).front() : t.front() ;
		auto itc = char_attrlist.find( c ) ;
		if ( itc == char_attrlist.end() )
		{
			err.seterrid( TRACE_INFO(), "PSYE036D", isprint( c ) ? string( 1, c ) : c2xs( c ) ) ;
			return ;
		}
		m_txt.text_char_attrs = itc->second ;
		if ( !m_txt.text_char_attrs->is_text_attr() )
		{
			err.seterrid( TRACE_INFO(), "PSYE036E", isprint( c ) ? string( 1, c ) : c2xs( c ) ) ;
			return ;
		}
	}
	else
	{
		if ( !findword( w3, "CH CT DT ET FP NT IMT PIN PS PT RP SAC SI SUC WASL WT" ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE036F", w3 ) ;
			return ;
		}
		m_txt.text_inline_attrs = new text_attrs( cuaAttrName[ w3 ] ) ;
		rest                    = subword( pline, 4 ) ;
	}

	w4 = upper( word( rest, 1 ) ) ;
	if ( w4 == "EXPAND" )
	{
		m_txt.text_value = qstring( err, subword( rest, 2 ) ) ;
		if ( err.error() ) { return ; }
		if ( m_txt.text_value.size() == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE041K" ) ;
			return ;
		}
		l = ( wscrmaxw - tcol ) / m_txt.text_value.size() + 1 ;
		m_txt.text_value = substr( copies( m_txt.text_value, l ), 1, ( wscrmaxw - tcol + 1 ) ) ;
	}
	else
	{
		m_txt.text_value = qstring( err, rest ) ;
		if ( err.error() ) { return ; }
	}

	tb_lcol = tcol ;
	tb_lsz  = m_txt.text_value.size() ;

	m_txt.text_col    = tcol - 1 ;
	m_txt.text_endcol = tcol + tb_lsz - 2 ;
	m_txt.text_length = tb_lsz ;

	for ( i = tb_lline, j = 0 ; j < tb_rdepth ; i += tb_modlines, ++j )
	{
		text* m_tbtext     = new text( m_txt ) ;
		m_tbtext->text_row = tb_toprow + i ;
		tbtextList.push_back( m_tbtext ) ;
	}
}


void pPanel::add_field( const string& name,
			field* pfield )
{
	TRACE_FUNCTION() ;

	fieldList[ name ]    = pfield ;
	addr2field[ pfield ] = name ;
}


void pPanel::create_pdc( errblock& err,
			 const string& abc_desc,
			 uint abc_mnem,
			 const string& pline )
{
	//
	// ab is a vector list of action-bar-choices (abc objects).
	// Each action-bar-choice is a vector list of pull-down-choices (pdc objects).
	//

	TRACE_FUNCTION() ;

	uint p ;

	bool oquote ;

	string uline = upper( pline ) ;
	string head ;
	string tail ;

	string pdc_desc ;
	string pdc_run  ;
	string pdc_parm ;
	string pdc_unavail ;

	abc* pabc ;

	vector<abc*>::iterator it ;

	oquote = false ;
	for ( p = 0 ; p < uline.size() ; ++p )
	{
		if ( oquote )
		{
			if ( uline[ p ] == '\'' )
			{
				oquote = false ;
			}
		}
		else if ( uline[ p ] == '\'' )
		{
			oquote = true ;
		}
		else if ( uline.compare( p, 6, "ACTION" ) == 0 )
		{
			break ;
		}
	}

	if ( p < uline.size() )
	{
		head = pline.substr( 0, p-1 ) ;
		head = delword( head, 1, 1 ) ;
		tail = pline.substr( p+6 ) ;
	}
	else
	{
		head = subword( pline, 2 ) ;
	}

	pdc_desc = extractKWord( err, head, "DESC()" ) ;
	if ( err.error() ) { return ; }
	if ( err.RSN4() )
	{
		pdc_desc = extractKWord( err, head, "PDCTEXT()" ) ;
		if ( err.error() ) { return ; }
		if ( pdc_desc == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE011W" ) ;
			return ;
		}
	}
	else if ( pdc_desc == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE011W" ) ;
		return ;
	}

	if ( pdc_desc.size() > 64 )
	{
		err.seterrid( TRACE_INFO(), "PSYE011O", "PDC" ) ;
		return ;
	}

	pdc_unavail = parseString1( err, head, "UNAVAIL()" ) ;
	if ( err.error() ) { return ; }

	if ( pdc_unavail != "" && !isvalidName( pdc_unavail ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE031D", pdc_unavail ) ;
		return ;
	}

	if ( head != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", head ) ;
		return ;
	}

	pdc_run = parseString1( err, tail, "RUN()" ) ;
	if ( err.error() ) { return ; }

	pdc_parm = extractKWord( err, tail, "PARM()" ) ;
	if ( err.error() ) { return ; }

	if ( tail != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", tail ) ;
		return ;
	}

	for ( it = ab.begin() ; it != ab.end() ; ++it )
	{
		pabc = *it ;
		if ( pabc->abc_desc == abc_desc )
		{
			break ;
		}
	}

	if ( it == ab.end() )
	{
		pabc = new abc( funcPool, selPanel ) ;
		pabc->abc_desc = abc_desc ;
		if ( abc_mnem > 0 )
		{
			pabc->abc_mnem1 = abc_mnem ;
			pabc->abc_mnem2 = toupper( abc_desc[ abc_mnem - 1 ] ) ;
		}
		pabc->abc_col = abc_pos ;
		abc_pos      += abc_desc.size() + 2 ;
		ab.push_back( pabc ) ;
	}

	pabc->add_pdc( pdc( pdc_desc, pdc_run, pdc_parm, pdc_unavail ) ) ;
}


void pPanel::check_overlapping_fields( errblock& err,
				       bool force_tbcheck )
{
	//
	// Check if any fields, text or scroll areas are overlapping.
	//

	TRACE_FUNCTION() ;

	int idx = 1 ;

	uint i ;
	uint j ;
	uint k ;
	uint l ;

	uint row ;
	uint col ;
	uint width ;
	uint depth ;

	string t1 ;
	string t2 ;
	string t3 ;

	vector<text*>* pall_text = &textList ;
	vector<text*> temp_text ;

	map<int, string>xref ;

	delete[] fieldMap ;

	fieldMap = new short int[ zscrmaxd * zscrmaxw ]() ;

	field* pfield ;

	for ( auto it = fieldList.begin() ; it != fieldList.end() ; ++it, ++idx )
	{
		pfield = it->second ;
		if ( !pfield->field_visible || ( pfield->field_tb_ext && tb_dvars && !force_tbcheck ) ) { continue ; }
		xref[ idx ] = it->first ;
		j = pfield->field_row * zscrmaxw + pfield->field_col ;
		k = j + pfield->field_length ;
		for ( i = j ; i < k ; ++i )
		{
			if ( fieldMap[ i ] != 0 )
			{
				t1 = xref[ fieldMap[ i ] ] ;
				t1 = t1.substr( 0, t1.find( '.' ) ) ;
				err.seterrid( TRACE_INFO(), "PSYE042W", it->first.substr( 0, it->first.find( '.' ) ), t1 ) ;
				delete[] fieldMap ;
				fieldMap = nullptr ;
				return ;
			}
			fieldMap[ i ] = idx ;
		}
	}

	for ( auto it = AreaList.begin() ; it != AreaList.end() ; ++it, ++idx )
	{
		it->second->get_info( row, col, width, depth ) ;
		xref[ idx ] = it->first ;
		for ( l = row ; l < (row + depth) ; ++l )
		{
			j = l * zscrmaxw + col ;
			k = j + width ;
			for ( i = j ; i < k ; ++i )
			{
				if ( fieldMap[ i ] != 0 )
				{
					t1 = xref[ fieldMap[ i ] ] ;
					t1 = t1.substr( 0, t1.find( '.' ) ) ;
					err.seterrid( TRACE_INFO(), "PSYE042W", it->first, t1 ) ;
					delete[] fieldMap ;
					fieldMap = nullptr ;
					return ;
				}
				fieldMap[ i ] = idx ;
			}
		}
	}

	if ( !tbtextList.empty() )
	{
		temp_text = textList ;
		for_each( tbtextList.begin(), tbtextList.end(),
			[ &temp_text ](text* a)
			{
				temp_text.push_back( a ) ;
			} ) ;
		pall_text = &temp_text ;
	}

	xref[ idx ] = "" ;
	for ( auto it = pall_text->begin() ; it != pall_text->end() ; ++it )
	{
		if ( !(*it)->text_visible ) { continue ; }
		j = (*it)->text_row * zscrmaxw + (*it)->text_col ;
		k = j + (*it)->text_endcol - (*it)->text_col + 1 ;
		for ( i = j ; i < k ; ++i )
		{
			if ( fieldMap[ i ] != 0 )
			{
				t1  = xref[ fieldMap[ i ] ] ;
				t1  = t1.substr( 0, t1.find( '.' ) ) ;
				t2  = d2ds( (*it)->text_row + 1 ) ;
				t2 += "," + d2ds( (*it)->text_col + 1 ) ;
				if ( t1 == "" )
				{
					t3 = d2ds( i + (*it)->text_col - j + 1 ) ;
					err.seterrid( TRACE_INFO(), "PSYE042X", t2, t3 ) ;
				}
				else
				{
					err.seterrid( TRACE_INFO(), "PSYE042Y", t1, t2 ) ;
				}
				delete[] fieldMap ;
				fieldMap = nullptr ;
				return ;
			}
			fieldMap[ i ] = idx ;
		}
	}

	delete[] fieldMap ;
	fieldMap = nullptr ;
}


void pPanel::build_fieldMap()
{
	//
	// FieldMap - 2 bytes for each screen byte (max fields per panel 65,535)
	//            0 - no field at this position.
	//            n - Index into fieldAddrs.
	//
	// FieldAddrs - pointer to array of field pointers indexed by fieldMap.
	//              First member is NULL to indicate no field.
	//

	TRACE_FUNCTION() ;

	using fieldptr = field* ;

	int idx = 1 ;

	uint i ;
	uint j ;
	uint k ;

	field* pfield ;

	delete[] fieldMap ;
	delete[] fieldAddrs ;

	fieldMap = new short int[ zscrmaxd * zscrmaxw ]() ;

	fieldAddrs      = new fieldptr[ fieldList.size() + 1 ] ;
	fieldAddrs[ 0 ] = nullptr ;

	for ( auto it = fieldList.begin() ; it != fieldList.end() ; ++it, ++idx )
	{
		pfield = it->second ;
		if ( pfield->field_visible )
		{
			fieldAddrs[ idx ] = pfield ;
			j = pfield->field_row * zscrmaxw + pfield->field_col ;
			k = j + pfield->field_length ;
			for ( i = j ; i < k ; ++i )
			{
				fieldMap[ i ] = idx ;
			}
		}
	}
}


void pPanel::build_jumpList()
{
	//
	// Build the jump list of all input fields followed by leader dots and ==>.
	//

	TRACE_FUNCTION() ;

	int k ;

	string t1 ;

	field* pfield ;

	for ( auto ita = fieldList.begin() ; ita != fieldList.end() ; ++ita )
	{
		pfield = ita->second ;
		if ( !pfield->field_da_ext &&
		     !pfield->field_tb_ext &&
		     !pfield->field_is_nojump() &&
		      pfield->field_is_input()  &&
		      cmdfield != ita->first &&
		      pfield->field_length > 1 )
		{
			for ( auto itb = textList.begin() ; itb != textList.end() ; ++itb )
			{
				if ( pfield->field_row == (*itb)->text_row &&
				     pfield->field_col == (*itb)->text_endcol + 2 )
				{
					k = (*itb)->text_value.size() - 3 ;
					if ( k > 0 )
					{
						t1 = (*itb)->text_value.substr( k ) ;
						if ( t1 == "..." ||
						     t1 == ". ." ||
						     t1 == "==>" )
						{
							jumpList[ ita->first ] = pfield ;
						}
					}
					break ;
				}
			}
		}
	}
}


void pPanel::build_Areas( errblock& err )
{
	TRACE_FUNCTION() ;

	Area* pArea ;

	for ( auto& a : AreaList )
	{
		pArea = a.second ;
		if ( pArea->not_defined() )
		{
			err.seterrid( TRACE_INFO(), "PSYE042Z", a.first ) ;
			return ;
		}
		pArea->check_overlapping_fields( err, a.first ) ;
		if ( err.error() ) { return ; }
		pArea->update_area() ;
	}
}


void pPanel::read_source( errblock& err,
			  vector<string>& src,
			  string name,
			  const string& paths,
			  const string& slist )
{
	//
	// Read panel source and process )INCLUDE statements.
	//
	//   Return codes:
	//    0 Normal completion.  lspf source read.
	//    4 Normal completion.  ISPF source read.
	//   20 Severe error.
	//

	TRACE_FUNCTION() ;

	string temp ;
	string line ;
	string type ;
	string var ;
	string w1 ;
	string w2 ;

	bool ispf_format = false ;
	bool comment     = false ;

	bool fformat ;

	std::ifstream panl ;

	type = ( words( slist ) == 1 ) ? "Panel" : "INCLUDE" ;

	fformat = ( type != "Panel" ) ;

	locator loc( paths, name ) ;
	loc.locate() ;

	if ( loc.errors() )
	{
		err.seterrid( TRACE_INFO(), loc.msgid(), loc.mdata() ) ;
		return ;
	}
	else if ( loc.not_found() )
	{
		iupper( name ) ;
		( type == "Panel" ) ? err.seterrid( TRACE_INFO(), "PSYE021B", name, 12 )
				    : err.seterrid( TRACE_INFO(), "PSYE041P", name, 20 ) ;
		return ;
	}

	panl.open( loc.entry().c_str() ) ;
	if ( !panl.is_open() )
	{
		err.seterrid( TRACE_INFO(), "PSYE041M", type, loc.entry() ) ;
		return ;
	}

	while ( getline( panl, line ) )
	{
		w1 = upper( word( line, 1 ) ) ;
		if ( w1 == ")COMMENT" )         { comment = true  ; continue ; }
		else if ( w1 == ")ENDCOMMENT" ) { comment = false ; continue ; }
		if ( comment )                  { continue                   ; }
		if ( !fformat && w1 == ")PANEL" )
		{
			if ( line.find( " FORMAT=" ) == string::npos )
			{
				ispf_format = true ;
			}
			fformat = true ;
		}
		if ( w1 == ")MODEL" )
		{
			src.push_back( line ) ;
			while ( getline( panl, line ) )
			{
				if ( line == "" ) { continue ; }
				if ( line.front() == ')' ) { break ; }
				if ( line.front() == '&' )
				{
					var  = upper( word( line, 1 ).substr( 1 ) ) ;
					temp = getDialogueVar( err, var ) ;
					if ( err.error() )
					{
						panl.close() ;
						return ;
					}
					if ( temp == "" )
					{
						err.seterrid( TRACE_INFO(), "PSYE043O", var ) ;
						panl.close() ;
						return ;
					}
					modvars[ var ] = temp ;
					if ( temp.compare( 0, 4, "OMIT" ) != 0 )
					{
						src.push_back( temp ) ;
					}
				}
				else
				{
					src.push_back( line ) ;
				}
			}
		}
		if ( !fformat && line.size() > 0 && line.front() == ')' )
		{
			fformat     = true ;
			ispf_format = true ;
		}
		if ( w1 == ")INCLUDE" )
		{
			w2 = upper( word( line, 2 ) ) ;
			if ( findword( w2, slist ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE041O", name, w2 ) ;
				err.setsrc( trim( line ) ) ;
				panl.close() ;
				return ;
			}
			read_source( err,
				     src,
				     w2,
				     paths,
				     slist + " " + w2 ) ;
			if ( err.error() )
			{
				panl.close() ;
				return ;
			}
			continue ;
		}
		else if ( w1 == ")END" )
		{
			src.push_back( line ) ;
			break ;
		}
		src.push_back( line ) ;
	}

	if ( panl.bad() )
	{
		err.seterrid( TRACE_INFO(), "PSYE041N", type, loc.name() ) ;
		panl.close() ;
		return ;
	}

	panl.close() ;

	if ( ispf_format )
	{
		err.setRC( 4 ) ;
	}
}


#ifdef HAS_REXX_SUPPORT
void pPanel::convert_source( errblock& err,
			     vector<string>& src )
{
	//
	// Convert panel from native ISPF to lspf format.
	// This is done by calling the REXX panconv procedure.
	//
	// If in debug mode, list out the source produced by panconv even
	// when there is an error during conversion (if any produced).
	//

	TRACE_FUNCTION() ;

	const char* rexxName = "panconv" ;

	string val1 ;
	string line ;

	std::chrono::steady_clock::time_point startTime ;

	if ( err.debugLevel2() )
	{
		startTime = std::chrono::steady_clock::now() ;
	}

	tempfile tfile1( "lspf-input" ) ;
	tempfile tfile2( "lspf-output" ) ;

	tfile1.open() ;
	for ( auto it = src.begin() ; it != src.end() ; ++it )
	{
		tfile1 << *it << endl;
	}
	tfile1.close() ;

	pApplication* appl = static_cast<pApplication*>(pAppl) ;
	string rxpath      = appl->sysexec() ;

	RexxInstance* instance   ;
	RexxThreadContext* threadContext ;
	RexxArrayObject args     ;
	RexxCondition condition  ;
	RexxDirectoryObject cond ;
	RexxObjectPtr result     ;

	RexxOption options[ 2 ] ;

	options[ 0 ].optionName = EXTERNAL_CALL_PATH ;
	options[ 0 ].option     = rxpath.c_str() ;
	options[ 1 ].optionName = "" ;
	options[ 1 ].option     = (void*)nullptr ;

	if ( !RexxCreateInterpreter( &instance, &threadContext, options ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE034T" ) ;
		return ;
	}

	args = threadContext->NewArray( 6 ) ;
	threadContext->ArrayPut( args, threadContext->String( "INTERNAL" ), 1 ) ;
	threadContext->ArrayPut( args, threadContext->String( tfile1.name_cstr() ), 2 ) ;
	threadContext->ArrayPut( args, threadContext->String( tfile2.name_cstr() ), 3 ) ;
	threadContext->ArrayPut( args, threadContext->String( d2ds( zscrmaxw ).c_str() ), 4 ) ;
	threadContext->ArrayPut( args, threadContext->String( d2ds( zscrmaxd ).c_str() ), 5 ) ;
	threadContext->ArrayPut( args, threadContext->String( ( err.debugMode() ) ? "YES" : "NO" ), 6 ) ;

	result = threadContext->CallProgram( rexxName, args ) ;

	if ( threadContext->CheckCondition() )
	{
		cond = threadContext->GetConditionInfo() ;
		threadContext->DecodeConditionInfo( cond, &condition ) ;
		llog( "E", "PANEL error running REXX. .: " << rexxName << endl ) ;
		llog( "E", "   Condition Code . . . . .: " << condition.code << endl ) ;
		llog( "E", "   Condition Error Text . .: " << threadContext->CString( condition.errortext ) << endl ) ;
		llog( "E", "   Condition Message. . . .: " << threadContext->CString( condition.message ) << endl ) ;
		llog( "E", "   Line Error Occured . . .: " << condition.position << endl ) ;
		if ( condition.code == Rexx_Error_Program_unreadable_notfound )
		{
			val1 = "REXX panconv NOT FOUND" ;
		}
		else
		{
			val1  = "Error on line: "+ d2ds( condition.position ) ;
			val1 += ".  Condition code: " ;
			val1 += d2ds( condition.code ) ;
			val1 += ".  Error Text: " ;
			val1 += threadContext->CString( condition.errortext ) ;
			val1 += ".  Message: " ;
			val1 += threadContext->CString( condition.message ) ;
			if ( val1.size() > 512 ) { val1.erase( 512 ) ; }
		}
		putDialogueVar( err, "ZVAL1", val1 ) ;
		if ( err.error() )
		{
			instance->Terminate() ;
			return ;
		}
		err.seterrid( TRACE_INFO(), "PSYS012Z" ) ;
		instance->Terminate() ;
		return ;
	}
	else if ( result != NULLOBJECT )
	{
		val1  = "Panel conversion error: " ;
		val1 += threadContext->CString( result ) ;
		putDialogueVar( err, "ZVAL1", val1 ) ;
		if ( err.error() )
		{
			instance->Terminate() ;
			return ;
		}
		err.seterrid( TRACE_INFO(), "PSYS012Z" ) ;
	}

	instance->Terminate() ;

	std::ifstream panl ;

	src.clear() ;

	panl.open( tfile2.name_cstr() ) ;
	while ( getline( panl, line ) )
	{
		src.push_back( line ) ;
	}

	if ( err.debugLevel2() )
	{
		auto endTime = std::chrono::steady_clock::now() ;
		auto resp = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime ).count() ;
		for ( auto& ln : src )
		{
			llog("-", "Panel source | " << ln << endl ) ;
		}
		llog("I", "ISPF -> lspf panel conversion time was " << resp << " ms" << endl ) ;
	}

	panl.close() ;
}


#else
void pPanel::convert_source( errblock& err,
			     vector<string>& src )
{
	//
	// If REXX has not been enabled, issue error message.
	//

	TRACE_FUNCTION() ;

	err.seterrid( TRACE_INFO(), "PSYE034I", "Panel" ) ;
}
#endif


void pPanel::preprocess_source( errblock& err,
				vector<string>& tSource,
				vector<string>& pSource,
				const string& name )
{
	//
	// Preprocess panel source:
	// 1) Remove comments and blank lines.
	// 2) Any lists split on multiple lines are put on one line.
	// 3) Any )ATTR statements split on multiple lines are put on one line.
	// 4) Any )FIELD statements split on multiple lines are put on one line.
	// 5) Join any open quoted string on multiple lines.
	// 6) Put PDC statements on one line.
	//
	// If panel source was in ISPF format, it has already been converted to lspf format by this point by panconv.
	//

	TRACE_FUNCTION() ;

	vector<string>::iterator it ;

	size_t p1 ;

	string w1 ;
	string temp ;
	string temp1 ;
	string temp2 ;

	bool abc    = false ;
	bool attr   = false ;
	bool ex_end = false ;
	bool body   = false ;
	bool lang   = false ;
	bool rexx   = false ;
	bool field  = false ;

	if ( err.debugLevel3() )
	{
		for ( it = tSource.begin() ; it != tSource.end() ; ++it )
		{
			llog("-", "Before PREP | " << *it << endl ) ;
		}
	}

	for ( it = tSource.begin() ; it != tSource.end() ; ++it )
	{
		p1 = it->find_first_not_of( ' ' ) ;
		if ( p1 == string::npos ) { continue ; }
		if ( it->compare( p1, 2, "--" ) == 0 ) { continue ; }
		rem_comments( *it ) ;
		trim_right( *it ) ;
		if ( *it == "" ) { continue ; }
		w1 = upper( word( *it, 1 ) ) ;
		if ( w1.front() == ')' )
		{
			attr  = false ;
			abc   = false ;
			body  = false ;
			field = false ;
			lang  = false ;
			rexx  = false ;
		}
		if ( abc )
		{
			if ( w1 == "PDC" )
			{
				if ( temp2 != "" )
				{
					pSource.push_back( temp2 ) ;
				}
				temp2 = *it ;
				continue ;
			}
			if ( w1 == "ACTION" )
			{
				if ( temp2 == "" )
				{
					err.seterrid( TRACE_INFO(), "PSYE041V" ) ;
					err.setsrc( trim( *it ) ) ;
					return ;
				}
				temp2 = temp2 + " " + trim( *it ) ;
				pSource.push_back( temp2 ) ;
				temp2 = "" ;
				continue ;
			}
		}
		else if ( temp2 != "" )
		{
			pSource.push_back( temp2 ) ;
			temp2 = "" ;
		}
		if ( attr )
		{
			if ( w1.size() > 2 )
			{
				temp = trim_right( pSource.back() ) + " " + trim( *it ) ;
				pSource.back() = temp ;
				continue ;
			}
		}
		else if ( field )
		{
			if ( w1.compare( 0, 6, "FIELD(" ) != 0 )
			{
				temp1 = trim_right( pSource.back() ) + " " + trim( *it ) ;
				pSource.back() = temp1 ;
				continue ;
			}
		}
		else if ( body )
		{
			if ( bodykws.count( w1 ) == 0 )
			{
				temp1 = trim_right( pSource.back() ) + " " + trim( *it ) ;
				pSource.back() = temp1 ;
				continue ;
			}
			else if ( w1 == "TBMODEL" )
			{
				++tb_modlines ;
			}
		}
		else if ( lang && !rexx )
		{
			if ( ibrackets( *it, '(' ) )
			{
				temp = *it ;
				for ( ++it ; it != tSource.end() ; ++it )
				{
					rem_comments( *it ) ;
					if ( it->size() > 1 && it->front() == ')' && it->at( 1 ) != ' ' )
					{
						break ;
					}
					trim( *it ) ;
					if ( *it == "" ) { continue ; }
					temp = temp + " " + *it ;
					if ( ibrackets( *it, ')' ) ) { break ; }

				}
				w1 = upper( word( temp, 1 ) ) ;
				if ( w1.compare( 0, 5, "*REXX" ) == 0 && countc( temp, '(' ) == 1 )
				{
					rexx = true ;
				}
				pSource.push_back( temp ) ;
				if ( it == tSource.end() ) { break ; }
				continue ;
			}
			else if ( it->back() == '+' && countc( *it, '\'' ) % 2 != 0 )
			{
				temp = *it ;
				temp.pop_back() ;
				for ( ++it ; it != tSource.end() ; ++it )
				{
					rem_comments( *it ) ;
					if ( it->size() > 1 && it->front() == ')' && it->at( 1 ) != ' ' )
					{
						break ;
					}
					trim_left( *it ) ;
					if ( *it == "" ) { continue ; }
					temp = temp + *it ;
					trim_right( *it ) ;
					if ( it->back() != '+' ) { break ; }

				}
				if ( it == tSource.end() ) { break ; }
				pSource.push_back( temp ) ;
				continue ;
			}
			else if ( w1.compare( 0, 5, "*REXX" ) == 0 && countc( *it, '(' ) == 1 )
			{
				rexx = true ;
			}
		}
		else if ( lang && rexx )
		{
			if ( word( w1, 1 ) == "*ENDREXX" )
			{
				rexx = false ;
			}
		}
		if ( w1 == ")END" )
		{
			ex_end = true ;
			break ;
		}
		else if ( w1 == ")ATTR"  ) { attr  = true ; }
		else if ( w1 == ")ABC"   ) { abc   = true ; }
		else if ( w1 == ")BODY"  ) { body  = true ; }
		else if ( w1 == ")AREA"  ) { body  = true ; }
		else if ( w1 == ")FIELD" ) { field = true ; }
		else if ( w1 == ")PROC" || w1 == ")INIT" || w1 == ")REINIT" || w1 == "ABCINIT" || w1 == ")ABCPROC" ) { lang = true ; }
		pSource.push_back( *it ) ;
	}

	if ( temp2 != "" )
	{
		pSource.push_back( temp2 ) ;
	}

	if ( err.debugLevel3() )
	{
		for ( it = pSource.begin() ; it != pSource.end() ; ++it )
		{
			llog("-", "After PREP  | " << *it << endl ) ;
		}
	}

	if ( !ex_end )
	{
		err.seterrid( TRACE_INFO(), "PSYE011S", ")END" ) ;
		return ;
	}
}


bool pPanel::reload()
{
	//
	// Signal reload panel if:
	//    Any MODEL variables have changed value.
	//    Any BODY WINDOW() or WIDTH() statement dialogue variables have changed value.
	//    The terminal has been resized.
	//

	TRACE_FUNCTION() ;

	errblock err ;

	if ( term_resize )
	{
		return true ;
	}

	if ( modvars.size() > 0 )
	{
		for ( auto& v : modvars )
		{
			if ( getDialogueVar( err, v.first ) != v.second )
			{
				return true ;
			}
		}
	}

	if ( winvars.size() > 0 )
	{
		for ( auto& v : winvars )
		{
			if ( ds2d( getDialogueVar( err, v.first ) ) != v.second )
			{
				return true ;
			}
		}
	}

	return false ;
}


void pPanel::createPanel_If( errblock& err,
			     parser& v,
			     panstmnt* m_stmnt,
			     bool init,
			     bool init_reinit_proc )
{
	//
	// The if-statement may have an inline statement but the address is in the same location.
	//

	TRACE_FUNCTION() ;

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
			err.seterrid( TRACE_INFO(), "PSYE041U" ) ;
		}
		else
		{
			createPanel_Refresh( err, v, m_stmnt ) ;
		}
		break ;

	case ST_EXIT:
		m_stmnt->ps_exit = true ;
		break ;

	case ST_GOTO:
		t = v.getToken( 1 ) ;
		m_stmnt->ps_goto = true ;
		if ( !isvalidName( t.value1 ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE041Z", t.value1 ) ;
		}
		else
		{
			m_stmnt->ps_label = t.value1 ;
		}
		break ;

	case ST_VEDIT:
		createPanel_Vedit( err, v, m_stmnt ) ;
		break ;

	case ST_PREXX:
		createPanel_REXX( err, v, m_stmnt ) ;
		break ;

	case ST_PANEXIT:
		createPanel_PANEXIT( err, v, m_stmnt ) ;
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
		err.seterrid( TRACE_INFO(), "PSYE041E" ) ;
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
	//
	// The else-statement may have an inline statement but the address is in the same location.
	//

	TRACE_FUNCTION() ;

	bool found = false ;

	token t ;

	vector<panstmnt* >::iterator ips ;

	if ( p_stmnt->size() == 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE041T", d2ds( m_stmnt->ps_column+1 ) ) ;
		return ;
	}

	ips = p_stmnt->begin() + p_stmnt->size() ;

	do
	{
		--ips ;
		if ( (*ips)->ps_if && (*ips)->ps_column == m_stmnt->ps_column )
		{
			m_stmnt->ps_else = (*ips)->ps_if ;
			found = true ;
			break ;
		}
		if ( (*ips)->ps_column <= m_stmnt->ps_column )
		{
			break ;
		}
	} while ( ips != p_stmnt->begin() ) ;

	if ( !found )
	{
		err.seterrid( TRACE_INFO(), "PSYE041T", d2ds( m_stmnt->ps_column+1 ) ) ;
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
			err.seterrid( TRACE_INFO(), "PSYE041U" ) ;
		}
		else
		{
			createPanel_Refresh( err, v, m_stmnt ) ;
		}
		break ;

	case ST_EXIT:
		m_stmnt->ps_exit = true ;
		break ;

	case ST_GOTO:
		t = v.getToken( 1 ) ;
		if ( !isvalidName( t.value1 ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE041Z", t.value1 ) ;
		}
		else
		{
			m_stmnt->ps_goto  = true ;
			m_stmnt->ps_label = t.value1 ;
		}
		break ;

	case ST_VEDIT:
		createPanel_Vedit( err, v, m_stmnt ) ;
		break ;

	case ST_PREXX:
		createPanel_REXX( err, v, m_stmnt ) ;
		break ;

	case ST_PANEXIT:
		createPanel_PANEXIT( err, v, m_stmnt ) ;
		break ;

	case ST_VERIFY:
		createPanel_Verify( err, v, m_stmnt ) ;
		break ;

	case ST_IF:
	case ST_ELSE:
	case ST_ERROR:
		err.seterrid( TRACE_INFO(), "PSYE041E" ) ;
	}
}


void pPanel::createPanel_REXX( errblock& err,
			       parser& v,
			       panstmnt* m_stmnt )
{
	//
	// The *REXX-statement.
	//

	TRACE_FUNCTION() ;

#ifdef HAS_REXX_SUPPORT
	PREXX* m_rexx = new PREXX ;

	m_rexx->parse( err, v ) ;
	if ( err.error() )
	{
		delete m_rexx ;
		return ;
	}

	m_stmnt->ps_prexx = m_rexx ;
#else
	err.seterrid( TRACE_INFO(), "PSYE034I", "Panel" ) ;
#endif
}


void pPanel::createPanel_PANEXIT( errblock& err,
				  parser& v,
				  panstmnt* m_stmnt )
{
	//
	// The PANEXIT-statement.
	//

	TRACE_FUNCTION() ;

#ifdef HAS_REXX_SUPPORT
	PANEXIT* m_panexit = new PANEXIT ;

	m_panexit->parse( err, v ) ;
	if ( err.error() )
	{
		delete m_panexit ;
		return ;
	}

	m_stmnt->ps_panexit = m_panexit ;
#else
	err.seterrid( TRACE_INFO(), "PSYE034I", "Panel" ) ;
#endif
}


void pPanel::createPanel_Assign( errblock& err,
				 parser& v,
				 panstmnt* m_stmnt,
				 bool init_reinit_proc )
{
	TRACE_FUNCTION() ;

	ASSGN* m_assign = new ASSGN ;

	m_assign->parse( err, v ) ;
	if ( err.error() )
	{
		delete m_assign ;
		return ;
	}

	if ( m_assign->as_isattc && not init_reinit_proc )
	{
		err.seterrid( TRACE_INFO(), "PSYE042Q", m_assign->as_lhs.value ) ;
		delete m_assign ;
		return ;
	}
	m_stmnt->ps_assign = m_assign ;
}


void pPanel::createPanel_Vedit( errblock& err,
				parser& v,
				panstmnt* m_stmnt )
{
	TRACE_FUNCTION() ;

	VEDIT* m_VED = new VEDIT ;

	m_VED->parse( err, v ) ;
	if ( err.error() )
	{
		delete m_VED ;
		return ;
	}

	if ( fieldList.count( m_VED->ved_var ) == 0 )
	{
		err.seterrid( TRACE_INFO(), "PSYE044A", m_VED->ved_var ) ;
		delete m_VED ;
		return ;
	}

	m_stmnt->ps_vedit  = m_VED ;
}


void pPanel::createPanel_Verify( errblock& err,
				 parser& v,
				 panstmnt* m_stmnt )
{
	TRACE_FUNCTION() ;

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


void pPanel::createPanel_Vputget( errblock& err,
				  parser& v,
				  panstmnt* m_stmnt )
{
	TRACE_FUNCTION() ;

	VPUTGET* m_VPG = new VPUTGET ;

	m_VPG->parse( err, v ) ;
	if ( err.error() )
	{
		delete m_VPG ;
		return ;
	}
	m_stmnt->ps_vputget = m_VPG ;
}


void pPanel::createPanel_Refresh( errblock& err,
				  parser& v,
				  panstmnt* m_stmnt )
{
	TRACE_FUNCTION() ;

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
					err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
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
					err.seterrid( TRACE_INFO(), "PSYE041X", t.value1 ) ;
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
					err.seterrid( TRACE_INFO(), "PSYE041W", t.value1 ) ;
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
				err.seterrid( TRACE_INFO(), "PSYE041X", t.value1 ) ;
				return ;
			}
		}
		else if ( t.value1 != "*" )
		{
			err.seterrid( TRACE_INFO(), "PSYE041W", t.value1 ) ;
			return ;
		}
		m_stmnt->ps_rlist = t.value1 ;
		v.getNextToken() ;
	}

	if ( !v.isCurrentType( TT_EOT ) )
	{
		t = v.getCurrentToken() ;
		err.seterrid( TRACE_INFO(), "PSYE032H", t.value1 ) ;
		return ;
	}
	m_stmnt->ps_refresh = true ;
}


void pPanel::setup_panelREXX( errblock& err,
			      vector<panstmnt*>* p_stmnt,
			      PREXX* prexx,
			      vector<string>& pSource,
			      vector<string>::iterator& it )
{
	//
	// Complete the setup of the panel REXX statement object.
	// - Add all input/output field variables defined in the body section if '*' specified.
	// - Store inline REXX code if no REXX procedure specified.
	//
	// Create a dummy panel statement object for each line of inline REXX code (all addresses zero) and set the start column
	// so we keep the IF/ELSE column alignment check.
	//

	TRACE_FUNCTION() ;

	if ( prexx->all_vars )
	{
		for ( auto& f : fieldList )
		{
			if ( f.second->field_validname )
			{
				prexx->vars.insert( f.first ) ;
			}
		}
		for ( auto& f : tb_fields )
		{
			prexx->vars.insert( f ) ;
		}
		for ( auto& d : dynAreaList )
		{
			prexx->vars.insert( d.first ) ;
		}
	}

	if ( prexx->rexx_inline() )
	{
		auto result = rexx_inlines.insert( pair<PREXX*, vector<string>>( prexx, vector<string>() ) ) ;
		vector<string>& src = result.first->second ;
		for ( ++it ; it != pSource.end() && upper( word( *it, 1 ) ) != "*ENDREXX" ; ++it )
		{
			src.push_back( *it ) ;
			p_stmnt->push_back( new panstmnt( it->find_first_not_of( ' ' ) ) ) ;
		}
		if ( it == pSource.end() )
		{
			err.seterrid( TRACE_INFO(), "PSYE043N" ) ;
			return ;
		}
		p_stmnt->push_back( new panstmnt( it->find_first_not_of( ' ' ) ) ) ;
	}
}
