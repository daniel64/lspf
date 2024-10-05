/*
  Copyright (c) 2021 Daniel John Erdos

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
int REXXENTRY rxiniExit_ft( RexxExitContext*,
			    int,
			    int,
			    PEXIT ) ;

int REXXENTRY rxterExit_ft( RexxExitContext*,
			    int,
			    int,
			    PEXIT ) ;

int REXXENTRY rxsioExit_ft( RexxExitContext*,
			    int,
			    int,
			    PEXIT ) ;

#endif


pFTailor::pFTailor( errblock& err,
		    fPOOL* fp,
		    void* t_appl,
		    bool  tmp ) : pFTailor()
{
	taskId   = err.taskid ;
	funcPool = fp ;
	istemp   = tmp ;
	pAppl    = t_appl ;
}


pFTailor::~pFTailor()
{
#ifdef HAS_REXX_SUPPORT
	if ( instance )
	{
		instance->Terminate() ;
	}
#endif
}


void pFTailor::ftincl( errblock& err,
		       const string& ft_skel,
		       bool noft )
{
	TRACE_FUNCTION() ;

	reinitialise() ;

	vector<ftline*> skel_incl ;
	vector<ftc_main*> ft_input ;

	set<string> ft_incls ;
	set<string> ft_tables ;

	read_skeleton( err,
		       ft_skel,
		       skel_incl,
		       ft_input,
		       ft_incls,
		       noft ) ;
	if ( err.getRC() > 0 )
	{
		free_storage( skel_incl, ft_input ) ;
		return ;
	}

	preproc_skeleton( err, skel_incl, ft_input ) ;
	if ( err.error() || ft_input.empty() )
	{
		free_storage( skel_incl, ft_input ) ;
		return ;
	}

	proc_input( err, ft_tables, *ft_input.begin() ) ;

	free_storage( skel_incl, ft_input ) ;
}


void pFTailor::ftclose( errblock& err,
			const string& ofile )
{
	TRACE_FUNCTION() ;

	err.setRC( 0 ) ;

	std::ofstream fout( ofile.c_str() ) ;
	if ( !fout.is_open() )
	{
		err.seterrid( TRACE_INFO(), "PSYE025H", ofile, 20 ) ;
		return ;
	}

	for ( auto& ln : ft_output )
	{
		fout << ln << endl ;
	}

	fout.close() ;
}


void pFTailor::reinitialise()
{
	//
	// FTINCL can be called multiple times before FTCLOSE, so lets
	// reset anything that may have changed in the previous call.
	//

	TRACE_FUNCTION() ;

	ft_curtab = nullptr ;

	char_ctrl = ')' ;
	char_var  = '&' ;
	def3      = '?' ;
	char_tab  = '!' ;
	char_cs1  = '<' ;
	char_cs2  = '|' ;
	char_cs3  = '>' ;
}


void pFTailor::free_storage( vector<ftline*>& skel_incl,
			     vector<ftc_main*>& ft_input )
{
	TRACE_FUNCTION() ;

	for ( auto p : ft_input )
	{
		delete p ;
	}

	for ( auto p : skel_incl )
	{
		delete p ;
	}
}


void pFTailor::read_skeleton( errblock& err,
			      const string& ft_skel,
			      vector<ftline*>& skel_incl,
			      vector<ftc_main*>& ft_input,
			      set<string>& ft_incls,
			      bool noft,
			      bool main,
			      bool opt,
			      const string& src_skel,
			      int  src_ln )
{
	//
	// RC =  0 Normal completion
	// RC =  8 Skeleton not found
	// RC = 20 Severe error
	//

	TRACE_FUNCTION() ;

	int ln = 0 ;

	string w2 ;
	string w3 ;
	string w4 ;
	string skline ;

	string sk_file ;
	string sk_default = defs ;

	string l_IM      = ")IM" ;
	string l_DEFAULT = ")DEFAULT" ;

	char lchar_ctrl = ')' ;

	int ws ;

	err.setRC( 0 ) ;

	if ( ft_incls.find( ft_skel ) != ft_incls.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE026L", ft_skel, src_info( src_skel, src_ln ) ) ;
		return ;
	}

	ft_incls.insert( ft_skel ) ;

	pApplication* appl = static_cast<pApplication*>(pAppl) ;
	locator loc( appl->get_libdef_search_paths( s_ZSLIB ), ft_skel ) ;

	loc.locate() ;
	if ( loc.errors() )
	{
		err.seterrid( TRACE_INFO(), loc.msgid(), loc.mdata() ) ;
		return ;
	}

	sk_file = loc.entry() ;
	if ( sk_file == "" )
	{
		if ( main )
		{
			err.setRC( 8 ) ;
			ft_incls.erase( ft_skel ) ;
		}
		else if ( !opt )
		{
			err.seterrid( TRACE_INFO(), "PSYE026M", ft_skel, src_info( src_skel, src_ln ) ) ;
		}
		return ;
	}

	std::ifstream skel( sk_file.c_str() ) ;
	if ( !skel.is_open() )
	{
		err.seterrid( TRACE_INFO(), "PSYE025K", src_info( src_skel, src_ln ), 20 ) ;
		return ;
	}

	skel_incl.push_back( new ftline( ft_skel, defs, -1, noft ) ) ;

	err.setftsrc() ;

	while ( getline( skel, skline ) )
	{
		++ln ;
		err.setsrc( &skline ) ;
		if ( !noft && word( skline, 1 ) == l_IM )
		{
			ws = words( skline ) ;
			if ( ws > 4 )
			{
				err.seterrid( TRACE_INFO(), "PSYE026T", subword( skline, 5 ), src_info( ft_skel, ln ) ) ;
				return ;
			}
			w3 = upper( word( skline, 3 ) ) ;
			w4 = upper( word( skline, 4 ) ) ;
			if ( w3 == "OPT" )
			{
				swap( w3, w4 ) ;
			}
			if ( w3 != "" && w3 != "NT" )
			{
				err.seterrid( TRACE_INFO(), "PSYE026E", w3, ")IM", src_info( ft_skel, ln ) ) ;
				return ;
			}
			if ( w4 != "" && w4 != "OPT" )
			{
				err.seterrid( TRACE_INFO(), "PSYE026E", w4, ")IM", src_info( ft_skel, ln ) ) ;
				return ;
			}
			read_skeleton( err,
				       word( skline, 2 ),
				       skel_incl,
				       ft_input,
				       ft_incls,
				       ( w3 == "NT" ),
				       false,
				       ( w4 == "OPT" ),
				       ft_skel,
				       ln ) ;
			if ( err.error() ) { return ; }
			err.setRC( 0 ) ;
			skel_incl.push_back( new ftline( ft_skel, sk_default, -1, noft ) ) ;
			continue ;
		}
		else if ( !noft && word( skline, 1 ) == l_DEFAULT )
		{
			if ( words( skline ) > 2 )
			{
				err.seterrid( TRACE_INFO(), "PSYE026T", subword( skline, 3 ), src_info( ft_skel, ln ) ) ;
				return ;
			}
			w2 = word( skline, 2 ) ;
			if ( w2.size() != 7 )
			{
				err.seterrid( TRACE_INFO(), "PSYE026Q", w2, src_info( ft_skel, ln ) ) ;
				return ;
			}
			sk_default     = skline ;
			lchar_ctrl     = w2[ 0 ] ;
			l_IM[ 0 ]      = lchar_ctrl ;
			l_DEFAULT[ 0 ] = lchar_ctrl ;
		}
		skel_incl.push_back( new ftline( ft_skel, skline, ln, noft ) ) ;
	}

	if ( skel.bad() )
	{
		err.seterrid( TRACE_INFO(), "PSYE025L", src_info( ft_skel, ln ), 20 ) ;
		return ;
	}

	skel.close() ;

	ft_incls.erase( ft_skel ) ;

	err.clearsrc() ;
}


void pFTailor::preproc_skeleton( errblock& err,
				 vector<ftline*>& skel_incl,
				 vector<ftc_main*>& ft_input )
{
	//
	// Preprocess skeletons.
	//

	TRACE_FUNCTION() ;

	err.setftsrc() ;

	string l_CM      = ")CM" ;
	string l_ENDREXX = ")ENDREXX" ;

	ftc_rexx* ft_rexx = nullptr ;

	vector<string> rexx_code ;

	stack<FT_TYPE> types ;

	stack<FT_TYPE> do_dots ;

	stack<ftc_main*> ft_dos ;
	stack<ftc_main*> ft_dots ;
	stack<ftc_main*> ft_sels ;

	ftc_main* ft_crnt = nullptr ;
	ftc_main* ft_prev = nullptr ;

	for ( auto psk : skel_incl )
	{
		ft_prev = ft_crnt ;
		if ( word( psk->line, 1 ) == l_ENDREXX )
		{
			if ( !ft_rexx )
			{
				err.seterrid( TRACE_INFO(), "PSYE025M", src_info( psk->name, psk->ln ), 20 ) ;
				return ;
			}
			auto result = rexx_inlines.insert( pair<ftc_rexx*, vector<string>>( ft_rexx, vector<string>() ) ) ;
			vector<string>& t = result.first->second ;
			t       = rexx_code ;
			ft_rexx = nullptr ;
			rexx_code.clear() ;
			continue ;
		}
		else if ( ft_rexx )
		{
			rexx_code.push_back( psk->line ) ;
			continue ;
		}
		if ( strip( psk->line ) == "" )
		{
			continue ;
		}
		if ( word( psk->line, 1 ) == l_CM )
		{
			if ( !psk->noft ) { continue ; }
			ft_crnt = new ftc_main( &psk->line, &psk->name, psk->ln, true ) ;
		}
		else if ( ( psk->line.size() > 1 && psk->line[ 0 ] == char_ctrl && psk->line[ 1 ] != ' ' ) && !psk->noft )
		{
			err.setsrc( &psk->line ) ;
			ft_crnt = new ftc_main( err, psk->line, &psk->name, char_var, psk->ln ) ;
			if ( err.error() )
			{
				if ( ft_crnt->ft_rexx )
				{
					err.setftsrc( src_info_line( ft_crnt ) ) ;
				}
				delete ft_crnt ;
				return ;
			}
			if ( ft_crnt->ft_def )
			{
				char_ctrl = ft_crnt->ft_def->char_ctrl() ;
				char_var  = ft_crnt->ft_def->char_var() ;
				l_CM[ 0 ]      = char_ctrl ;
				l_ENDREXX[ 0 ] = char_ctrl ;
			}
			else if ( ft_crnt->ft_do )
			{
				ft_dos.push( ft_crnt ) ;
				types.push( FT_DO ) ;
				do_dots.push( FT_DO ) ;
				if ( ft_dos.size() > 255 )
				{
					err.seterrid( TRACE_INFO(), "PSYE026H", src_info( ft_crnt ), 20 ) ;
					delete ft_crnt ;
					return ;
				}
			}
			else if ( ft_crnt->ft_enddo )
			{
				if ( types.empty() || ft_dos.empty() || types.top() != FT_DO )
				{
					err.seterrid( TRACE_INFO(), "PSYE026N", ")DO", ")ENDDO", src_info( ft_crnt ), 20 ) ;
					delete ft_crnt ;
					return ;
				}
				types.pop() ;
				do_dots.pop() ;
				ft_crnt->ft_jump          = ft_dos.top() ;
				ft_crnt->ft_jump->ft_jump = ft_crnt ;
				ft_dos.pop() ;
			}
			else if ( ft_crnt->ft_dot )
			{
				ft_dots.push( ft_crnt ) ;
				types.push( FT_DOT ) ;
				do_dots.push( FT_DOT ) ;
				if ( ft_dots.size() > 4 )
				{
					err.seterrid( TRACE_INFO(), "PSYE026F", src_info( ft_crnt ), 20 ) ;
					delete ft_crnt ;
					return ;
				}
			}
			else if ( ft_crnt->ft_enddot )
			{
				if ( types.empty() || ft_dots.empty() || types.top() != FT_DOT )
				{
					err.seterrid( TRACE_INFO(), "PSYE026N", ")DOT", ")ENDDOT", src_info( ft_crnt ), 20 ) ;
					delete ft_crnt ;
					return ;
				}
				types.pop() ;
				do_dots.pop() ;
				ft_crnt->ft_jump          = ft_dots.top() ;
				ft_crnt->ft_jump->ft_jump = ft_crnt ;
				ft_dots.pop() ;
			}
			else if ( ft_crnt->ft_iterate )
			{
				if ( ft_dos.empty() )
				{
					err.seterrid( TRACE_INFO(), "PSYE026N", ")DO", ")ITERATE", src_info( ft_crnt ), 20 ) ;
					delete ft_crnt ;
					return ;
				}
			}
			else if ( ft_crnt->ft_leave )
			{
				if ( ft_crnt->ft_leave->dot )
				{
					if ( ft_dots.empty() || do_dots.top() != FT_DOT )
					{
						err.seterrid( TRACE_INFO(), "PSYE026N", ")DOT", ")LEAVE", src_info( ft_crnt ), 20 ) ;
						delete ft_crnt ;
						return ;
					}
				}
				else
				{
					if ( ft_dos.empty() || do_dots.top() != FT_DO )
					{
						err.seterrid( TRACE_INFO(), "PSYE026N", ")DO", ")LEAVE", src_info( ft_crnt ), 20 ) ;
						delete ft_crnt ;
						return ;
					}
				}
			}
			else if ( ft_crnt->ft_sel )
			{
				ft_sels.push( ft_crnt ) ;
				types.push( FT_SEL ) ;
				if ( ft_sels.size() > 32 )
				{
					err.seterrid( TRACE_INFO(), "PSYE026G", src_info( ft_crnt ), 20 ) ;
					delete ft_crnt ;
					return ;
				}
			}
			else if ( ft_crnt->ft_endsel )
			{
				if ( types.empty() || ft_sels.empty() || types.top() != FT_SEL )
				{
					err.seterrid( TRACE_INFO(), "PSYE026N", ")SEL", ")ENDSEL", src_info( ft_crnt ), 20 ) ;
					delete ft_crnt ;
					return ;
				}
				types.pop() ;
				ft_sels.top()->ft_jump = ft_crnt ;
				ft_sels.pop() ;
			}
#ifdef HAS_REXX_SUPPORT
			else if ( ft_crnt->ft_rexx && ft_crnt->ft_rexx->rexx_inline() )
			{
				ft_rexx = ft_crnt->ft_rexx ;
			}
#else
			else if ( ft_crnt->ft_rexx )
			{
				err.seterrid( TRACE_INFO(), "PSYE034I", "File tailoring", src_info( ft_crnt ), 20 ) ;
				delete ft_crnt ;
				return ;
			}
#endif
		}
		else
		{
			ft_crnt = new ftc_main( &psk->line, &psk->name, psk->ln, psk->noft ) ;
		}
		if ( ft_prev )
		{
			ft_prev->ft_next = ft_crnt ;
		}
		ft_crnt->ft_prev = ft_prev ;
		ft_input.push_back( ft_crnt ) ;
	}

	if ( !ft_dos.empty() )
	{
		err.setsrc( ft_dos.top()->ft_line2 ) ;
		err.seterrid( TRACE_INFO(), "PSYE026N", ")ENDDO", ")DO", src_info( ft_dos.top() ), 20 ) ;
		return ;
	}

	if ( !ft_dots.empty() )
	{
		err.setsrc( ft_dots.top()->ft_line2 ) ;
		err.seterrid( TRACE_INFO(), "PSYE026N", ")ENDDOT", ")DOT", src_info( ft_dots.top() ), 20 ) ;
		return ;
	}

	if ( !ft_sels.empty() )
	{
		err.setsrc( ft_sels.top()->ft_line2 ) ;
		err.seterrid( TRACE_INFO(), "PSYE026N", ")ENDSEL", ")SEL", src_info( ft_sels.top() ), 20 ) ;
		return ;
	}

	if ( ft_rexx )
	{
		err.seterrid( TRACE_INFO(), "PSYE025N", src_info( ft_crnt ), 20 ) ;
		return ;
	}

	err.clearsrc() ;
}


void pFTailor::proc_input( errblock& err,
			   set<string>& ft_tables,
			   const ftc_main* ft_crnt )
{
	TRACE_FUNCTION() ;

	bool ft_dummy = false ;

	proc_input( err, ft_tables, ft_crnt, ft_dummy ) ;
}


void pFTailor::proc_input( errblock& err,
			   set<string>& ft_tables,
			   const ftc_main* ft_crnt,
			   bool& ft_leave )
{
	//
	// Process skeletons.
	//

	TRACE_FUNCTION() ;

	string temp ;

	bool jump ;
	bool do_leave ;

	bool top      = true ;
	bool do_until = false ;

	int  do_loop = 0 ;
	int  do_to   = 0 ;
	int  do_by   = 1 ;
	int  do_for  = 0 ;

	err.setftsrc() ;

	while ( ft_crnt )
	{
		err.setsrc( ft_crnt->ft_line2 ) ;
		if ( ft_crnt->ft_blank )
		{
			for ( int i = 0 ; i < ft_crnt->ft_num ; ++i )
			{
				ft_output.push_back( "" ) ;
			}
		}
		else if ( ft_crnt->ft_def )
		{
			temp      = ft_crnt->ft_def->get() ;
			char_ctrl = temp[ 0 ] ;
			char_var  = temp[ 1 ] ;
			def3      = temp[ 2 ] ;
			char_tab  = temp[ 3 ] ;
			char_cs1  = temp[ 4 ] ;
			char_cs2  = temp[ 5 ] ;
			char_cs3  = temp[ 6 ] ;
		}
		else if ( ft_crnt->ft_do )
		{
			if ( ft_crnt->ft_do->l_type1 )
			{
				if ( top )
				{
					do_loop = get_val( err, ft_crnt, ft_crnt->ft_do->v_loop, ft_crnt->ft_do->l_loop ) ;
					if ( err.error() ) { return ; }
					top = false ;
				}
				if ( do_loop > 0 )
				{
					proc_input_do( err, ft_tables, ft_crnt, do_leave ) ;
					if ( err.error() ) { return ; }
					if ( !do_leave )
					{
						--do_loop ;
						continue ;
					}
				}
				top = true ;
				ft_crnt = ft_crnt->ft_jump ;
			}
			else if ( ft_crnt->ft_do->l_type2 )
			{
				if ( top )
				{
					if ( ft_crnt->ft_do->e_loop )
					{
						do_loop = get_val( err, ft_crnt, ft_crnt->ft_do->v_loop, ft_crnt->ft_do->l_loop ) ;
						if ( err.error() ) { return ; }
					}
					if ( ft_crnt->ft_do->e_to )
					{
						do_to = get_val( err, ft_crnt, ft_crnt->ft_do->v_to, ft_crnt->ft_do->l_to ) ;
						if ( err.error() ) { return ; }
					}
					if ( ft_crnt->ft_do->e_by )
					{
						do_by = get_val( err, ft_crnt, ft_crnt->ft_do->v_by, ft_crnt->ft_do->l_by ) ;
						if ( err.error() ) { return ; }
					}
					if ( ft_crnt->ft_do->e_for )
					{
						do_for = get_val( err, ft_crnt, ft_crnt->ft_do->v_for, ft_crnt->ft_do->l_for ) ;
						if ( err.error() ) { return ; }
					}
					top = false ;
				}
				if ( loop_cond( err, ft_crnt, do_loop, do_to, do_for ) && !do_until )
				{
					proc_input_do( err, ft_tables, ft_crnt, do_leave ) ;
					if ( err.error() ) { return ; }
					if ( !do_leave )
					{
						if ( ft_crnt->ft_do->e_until )
						{
							do_until = check_cond( err, ft_crnt ) ;
							if ( err.error() ) { return ; }
						}
						do_loop += do_by ;
						--do_for ;
						continue ;
					}
				}
				if ( err.error() ) { return ; }
				top      = true ;
				do_until = false ;
				ft_crnt = ft_crnt->ft_jump ;
			}
			else
			{
				proc_input_do( err, ft_tables, ft_crnt, do_leave ) ;
				if ( err.error() ) { return ; }
				if ( !do_leave )
				{
					continue ;
				}
				top     = true ;
				ft_crnt = ft_crnt->ft_jump ;
			}
		}
		else if ( ft_crnt->ft_enddo )
		{
			return ;
		}
		else if ( ft_crnt->ft_dot )
		{
			proc_input_dot( err, ft_tables, ft_crnt ) ;
			if ( err.error() )
			{
				return ;
			}
			ft_crnt = ft_crnt->ft_jump ;
		}
		else if ( ft_crnt->ft_enddot )
		{
			if ( ft_crnt->ft_jump->ft_dot->scan )
			{
				tbscan( err, sub_var( err, ft_crnt->ft_jump->ft_dot->table ) ) ;
			}
			else
			{
				tbskip( err, sub_var( err, ft_crnt->ft_jump->ft_dot->table ) ) ;
			}
			if ( err.error() )
			{
				err.setftsrc( src_info_line( ft_crnt ) ) ;
				return ;
			}
			else if ( err.getRC() > 0 )
			{
				return ;
			}
			ft_crnt = ft_crnt->ft_jump ;
		}
		else if ( ft_crnt->ft_line1 )
		{
			if ( ft_crnt->ln != -1 )
			{
				if ( ft_crnt->ft_noft )
				{
					ft_output.push_back( *ft_crnt->ft_line1 ) ;
				}
				else
				{
					temp = apply_tabs( sub_vars( cond_subs( err, ft_crnt ) ) ) ;
					if ( err.error() )
					{
						err.setftsrc( src_info_line( ft_crnt ) ) ;
						return ;
					}
					if ( temp != "" )
					{
						ft_output.push_back( temp ) ;
					}
				}
			}
		}
		else if ( ft_crnt->ft_iterate )
		{
			return ;
		}
		else if ( ft_crnt->ft_leave )
		{
			ft_leave = true ;
			return ;
		}
		else if ( ft_crnt->ft_set )
		{
			proc_input_set( err, ft_crnt ) ;
			if ( err.error() ) { return ; }
		}
		else if ( ft_crnt->ft_rexx )
		{
			proc_input_rexx( err, ft_crnt ) ;
			if ( err.error() )
			{
				err.setftsrc( src_info_line( ft_crnt ) ) ;
				return ;
			}
		}
		else if ( ft_crnt->ft_nop )
		{
			noop ;
		}
		else if ( ft_crnt->ft_sel )
		{
			proc_input_sel( err, ft_crnt, jump ) ;
			if ( err.error() ) { return ; }
			if ( jump )
			{
				ft_crnt = ft_crnt->ft_jump ;
			}
		}
		else if ( ft_crnt->ft_tb )
		{
			ft_curtab = ft_crnt->ft_tb ;
		}
		ft_crnt = ft_crnt->ft_next ;
	}

	err.clearsrc() ;
}


void pFTailor::proc_input_do( errblock& err,
			      set<string>& ft_tables,
			      const ftc_main* ft_crnt,
			      bool& do_leave )
{
	//
	// Process )DO statement
	//

	TRACE_FUNCTION() ;

	do_leave = false ;

	proc_input( err, ft_tables, ft_crnt->ft_next, do_leave ) ;
	if ( err.error() ) { return ; }
}


void pFTailor::proc_input_dot( errblock& err,
			       set<string>& ft_tables,
			       const ftc_main* ft_crnt )
{
	//
	// Process )DOT statement
	//
	// If table already open, start at top and leave at top.
	// If table closed, close table at end.
	// If table not found, end with error unless OPT specified.
	//
	// Table name may be a dialogue variable.
	//

	TRACE_FUNCTION() ;

	bool tab_opened = false ;
	bool tab_cpairs ;

	string table ;

	ftc_dot* ft_dot = ft_crnt->ft_dot ;

	table = upper( sub_var( err, ft_dot->table ) ) ;
	if ( err.error() )
	{
		err.setftsrc( src_info( ft_crnt ) ) ;
		return ;
	}

	if ( !isvalidName( table ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE026C", table, src_info( ft_crnt ) ) ;
		return ;
	}

	if ( ft_tables.find( table ) != ft_tables.end() )
	{
		err.seterrid( TRACE_INFO(), "PSYE026J", table, src_info( ft_crnt ), 20 ) ;
		return ;
	}

	tbquery( err, table, tab_cpairs ) ;
	if ( err.RC12() )
	{
		if ( ft_dot->scan && ft_dot->sarg == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE026Y", table, src_info( ft_crnt ), 20 ) ;
			return ;
		}
		tbopen( err, table ) ;
		if ( err.error() )
		{
			err.setftsrc( src_info_line( ft_crnt ) ) ;
			return ;
		}
		if ( err.RC8() )
		{
			if ( ft_dot->opt )
			{
				err.setRC( 0 ) ;
			}
			else
			{
				err.seterrid( TRACE_INFO(), "PSYE026K", table, src_info( ft_crnt ), 20 ) ;
			}
			return ;
		}
		else if ( err.getRC() > 0 )
		{
			return ;
		}
		tab_opened = true ;
	}
	else if ( err.error() )
	{
		err.setftsrc( src_info_line( ft_crnt ) ) ;
		return ;
	}
	else
	{
		if ( ft_dot->scan && ft_dot->sarg == "" )
		{
			if ( !tab_cpairs )
			{
				err.seterrid( TRACE_INFO(), "PSYE026Z", table, src_info( ft_crnt ), 20 ) ;
				return ;
			}
		}
		tbtop( err, table ) ;
		if ( err.error() )
		{
			err.setftsrc( src_info_line( ft_crnt ) ) ;
			return ;
		}
	}

	if ( ft_dot->scan && ft_dot->sarg != "" )
	{
		tbsarg( err, table, ft_dot->sarg ) ;
		if ( err.error() )
		{
			err.setftsrc( src_info_line( ft_crnt ) ) ;
			return ;
		}
		if ( err.getRC() > 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE027A", table, src_info( ft_crnt ), 20 ) ;
			return ;
		}
	}

	ft_tables.insert( table ) ;

	( ft_dot->scan ) ? tbscan( err, table ) : tbskip( err, table ) ;
	if ( err.error() )
	{
		err.setftsrc( src_info_line( ft_crnt ) ) ;
		return ;
	}

	if ( err.RC0() )
	{
		proc_input( err, ft_tables, ft_crnt->ft_next ) ;
		if ( err.error() ) { return ; }
	}

	( tab_opened ) ? tbend( err, table ) : tbtop( err, table ) ;
	if ( err.error() )
	{
		err.setftsrc( src_info_line( ft_crnt ) ) ;
		return ;
	}

	ft_tables.erase( table ) ;
}


#ifdef HAS_REXX_SUPPORT
void pFTailor::proc_input_rexx( errblock& err,
				const ftc_main* ft_crnt )
{
	//
	// Process )REXX statement.
	//
	// ZFTXRC:
	// RC =  0 Normal completion.
	// RC =  8 File tailoring REXX-defined failure.  lspf issues setmsg().
	// RC = 20 Severe error in the file tailoring rexx.
	//         Any other value also gives a severe error.
	//
	// ZFTXMSG - message id used to set SETMSG when ZFTXRC = 8 (PSYE034R if blank) or ZERRMSG if a severe error (PSYE032Q if blank).
	//
	// Setup REXX initialisation exit RXINI and termination exit RXTER to handle
	// setting REXX/lspf variables before and after REXX program execution.
	//
	// Setup REXX exit RXSIO (subfunction RXSIOSAY and RXSIOTRC) to redirect say/trace output to the rdisplay() method
	// and RXSIOTRD to call the pull() method for the rexx PULL/ARG PULL statements and
	// RXSIODTR to call the pull() method for interactive debug input.
	//

	TRACE_FUNCTION() ;

	int i ;
	int j ;

	int zzftxrc ;

	string w ;
	string msg ;
	string tmp ;

	string rxname ;

	const string msg_source = "file tailoring" ;

	ftc_rexx* ft_rexx = ft_crnt->ft_rexx ;

	set<string> vars ;

	tempfile tfile( "lspf-ftrexx" ) ;

	err.setRC( 0 ) ;

	for ( auto& var : ft_rexx->vars )
	{
		if ( var.front() == char_var )
		{
			tmp = upper( sub_var( err, var ) ) ;
			if ( err.error() ) { return ; }
			replace( tmp.begin(), tmp.end(), ',', ' ' ) ;
			for ( i = 1, j = words( tmp ) ; i <= j ; ++i )
			{
				w = word( tmp, i ) ;
				if ( !isvalidName( w ) )
				{
					err.seterrid( TRACE_INFO(), "PSYE013A", ")REXX statement", w ) ;
					return ;
				}
				vars.insert( w ) ;
			}
		}
		else
		{
			if ( !isvalidName( var ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE013A", ")REXX statement", var ) ;
				return ;
			}
			vars.insert( var ) ;
		}
	}

	exitInfo1.funcPool   = funcPool ;
	exitInfo1.p_poolMGR  = p_poolMGR ;
	exitInfo1.vars       = &vars ;
	exitInfo1.err        = &err ;
	exitInfo1.pAppl      = pAppl ;

	if ( ft_rexx->rexx_inline() )
	{
		tfile.open() ;
		auto it1 = rexx_inlines.find( ft_rexx ) ;
		for ( auto it2 = it1->second.begin() ; it2 != it1->second.end() ; ++it2 )
		{
			tfile << *it2 << endl ;
		}
		tfile.close() ;
		rxname = tfile.name() ;
	}
	else
	{
		rxname = sub_var( err, ft_rexx->rexx ) ;
		if ( err.error() ) { return ; }
		if ( rxname == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE034R", ft_rexx->rexx ) ;
			return ;
		}
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
		llog( "E", "FTAILOR error running REXX.: " << ( ( ft_rexx->rexx_inline() ) ? "*inline*" : rxname ) << endl ) ;
		llog( "E", "   Condition Code . . . . .: " << condition.code << endl ) ;
		llog( "E", "   Condition Error Text . .: " << threadContext->CString( condition.errortext ) << endl ) ;
		llog( "E", "   Condition Message. . . .: " << threadContext->CString( condition.message ) << endl ) ;
		llog( "E", "   Line Error Occured . . .: " << condition.position << endl ) ;
		if ( ft_rexx->rexx_inline() )
		{
			size_t i = 1 ;
			llog( "E", "***** Failing inline REXX code **************************************************************************" << endl ) ;
			auto it1 = rexx_inlines.find( ft_rexx ) ;
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
			err.seterrid( TRACE_INFO(), "PSYE034Q" ) ;
		}
		instance->Terminate() ;
		instance = nullptr ;
		return ;
	}
	else if ( err.error() )
	{
		instance->Terminate() ;
		instance = nullptr ;
		return ;
	}

	if ( !datatype( exitInfo1.zftxrc, 'W' ) )
	{
		msg = ( exitInfo1.zftxmsg == "" ) ? "PSYE034Q" : exitInfo1.zftxmsg ;
		putDialogueVar( err, "ZERRMSG", msg ) ;
		err.seterrid( TRACE_INFO(), msg ) ;
		llog( "E", "File Tailoring REXX returned a non-numeric value for ZFTXRC.  Value=" << exitInfo1.zftxrc << endl ) ;
	}
	else
	{
		zzftxrc = ds2d( exitInfo1.zftxrc ) ;
		if ( zzftxrc != 0 && zzftxrc != 8 )
		{
			msg = ( exitInfo1.zftxmsg == "" ) ? "PSYE034Q" : exitInfo1.zftxmsg ;
			putDialogueVar( err, "ZERRMSG", msg ) ;
			err.seterrid( TRACE_INFO(), msg ) ;
			llog( "E", "File Tailoring REXX returned a value for ZFTXRC that is not 0 or 8.  Value=" << exitInfo1.zftxrc << endl ) ;
		}
		else if ( zzftxrc == 8 )
		{
			zftxrc  = zzftxrc ;
			zftxmsg = exitInfo1.zftxmsg ;
		}
	}

	instance->Terminate() ;
	instance = nullptr ;
}


void pFTailor::start_rexx_interpreter( errblock& err )
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
	options[ 0 ].option     = (void*)&exitInfo1 ;
	options[ 1 ].optionName = DIRECT_EXITS ;
	options[ 1 ].option     = (void*)exits ;
	options[ 2 ].optionName = EXTERNAL_CALL_PATH ;
	options[ 2 ].option     = rxpath.c_str() ;
	options[ 3 ].optionName = "" ;

	exits[ 0 ].sysexit_code = RXINI ;
	exits[ 0 ].handler      = rxiniExit_ft ;
	exits[ 1 ].sysexit_code = RXTER ;
	exits[ 1 ].handler      = rxterExit_ft ;
	exits[ 2 ].sysexit_code = RXSIO ;
	exits[ 2 ].handler      = rxsioExit_ft ;
	exits[ 3 ].sysexit_code = RXENDLST ;
	exits[ 3 ].handler      = nullptr ;

	if ( !RexxCreateInterpreter( &instance, &threadContext, options ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE034T" ) ;
	}
}


int REXXENTRY rxiniExit_ft( RexxExitContext* context,
			    int exitnumber,
			    int subfunction,
			    PEXIT parmblock )

{
	//
	// OOREXX initialisation exit to set REXX variables at the start of program execution.
	// Also set special variables ZFTXRC and ZFTXMSG in the REXX variable pool.
	//
	// Variable handling:
	//   Copy required variables from the function/SHARED/PROFILE pool to the REXX variable pool.
	//
	// If the function pool variable has been defined as an integer, always return character format.
	//

	void* vptr ;

	string val ;

	vptr = context->GetApplicationData() ;
	exitInfo_ft* exitInfo1 = static_cast<exitInfo_ft*>(vptr) ;

	context->SetContextVariable( "ZFTXRC", context->String( "0" ) ) ;

	context->SetContextVariable( "ZFTXMSG", context->String( "" ) ) ;

	for ( auto& var : *exitInfo1->vars )
	{

		fVAR* pvar = exitInfo1->funcPool->getfVAR( *exitInfo1->err, var ) ;
		if ( exitInfo1->err->error() ) { return RXEXIT_HANDLED ; }
		if ( pvar )
		{
			val = pvar->sget( var ) ;
		}
		else
		{
			val = exitInfo1->p_poolMGR->get( *exitInfo1->err, var, ASIS ) ;
			if ( exitInfo1->err->error() ) { return RXEXIT_HANDLED ; }
		}
		context->SetContextVariable( var.c_str(), context->String( val.c_str(), val.size() ) ) ;
	}

	return RXEXIT_HANDLED ;
}


int REXXENTRY rxterExit_ft( RexxExitContext* context,
			    int exitnumber,
			    int subfunction,
			    PEXIT parmblock )

{
	//
	// OOREXX termination exit to set dialogue variables at the end of program execution.
	// Also set special variables ZFTXRC and ZFTXMSG in the lspf function pool.
	//
	// Variable handling:
	//   Copy required variables from the REXX variable pool to the function pool.
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
	exitInfo_ft* exitInfo1 = static_cast<exitInfo_ft*>(vptr) ;

	for ( auto& var : *exitInfo1->vars )
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
		fVAR* pvar = exitInfo1->funcPool->getfVAR( *exitInfo1->err, var ) ;
		if ( exitInfo1->err->error() ) { return RXEXIT_HANDLED ; }
		if ( pvar )
		{
			pvar->put( *exitInfo1->err, var, val ) ;
		}
		else
		{
			exitInfo1->funcPool->put2( *exitInfo1->err, var, val ) ;
		}
		if ( exitInfo1->err->error() ) { return RXEXIT_HANDLED ; }
	}

	optr = context->GetContextVariable( "ZFTXRC" ) ;
	exitInfo1->zftxrc = ( optr != NULLOBJECT ) ? context->CString( optr ) : "ZFTXRC" ;
	exitInfo1->funcPool->put2( *exitInfo1->err, "ZFTXRC", exitInfo1->zftxrc ) ;
	if ( exitInfo1->err->error() ) { return RXEXIT_HANDLED ; }

	optr = context->GetContextVariable( "ZFTXMSG" ) ;
	exitInfo1->zftxmsg = ( optr != NULLOBJECT ) ? context->CString( optr ) : "ZFTXMSG" ;
	exitInfo1->funcPool->put2( *exitInfo1->err, "ZFTXMSG", exitInfo1->zftxmsg ) ;
	if ( exitInfo1->err->error() ) { return RXEXIT_HANDLED ; }

	return RXEXIT_HANDLED ;
}


int REXXENTRY rxsioExit_ft( RexxExitContext* context,
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
		exitInfo_ft* exitInfo1  = static_cast<exitInfo_ft*>(vptr) ;
		pApplication* appl      = static_cast<pApplication*>(exitInfo1->pAppl) ;
		RXSIOSAY_PARM* say_parm = (RXSIOSAY_PARM *)ParmBlock ;
		appl->rdisplay( string( say_parm->rxsio_string.strptr, say_parm->rxsio_string.strlength ), false ) ;
		return RXEXIT_HANDLED ;
	}

	case RXSIOTRD:
	{
		string ans ;
		vptr = context->GetApplicationData() ;
		exitInfo_ft* exitInfo1  = static_cast<exitInfo_ft*>(vptr) ;
		pApplication* appl      = static_cast<pApplication*>(exitInfo1->pAppl) ;
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
		exitInfo_ft* exitInfo1  = static_cast<exitInfo_ft*>(vptr) ;
		pApplication* appl      = static_cast<pApplication*>(exitInfo1->pAppl) ;
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
void pFTailor::proc_input_rexx( errblock& err,
				const ftc_main* ft_crnt )
{
	//
	// Dummy routine for when REXX has not been enabled.
	//
}
#endif


void pFTailor::proc_input_sel( errblock& err,
			       const ftc_main* ft_crnt,
			       bool& jump )
{
	//
	// Process )SEL statement
	//
	// Perform boolean operations from left to right with equal precedence.
	//

	TRACE_FUNCTION() ;

	int lhs_ival ;
	int rhs_ival ;

	bool sub_true ;
	bool sel_true ;
	bool sel_AND  ;
	bool numeric  ;

	string rhs_val ;
	string lhs_val ;

	ftc_sel* ft_sel = ft_crnt->ft_sel ;

	sel_true = ft_sel->sel_AND ;
	sel_AND  = ft_sel->sel_AND ;

	while ( ft_sel )
	{
		if ( ft_sel->lvar )
		{
			lhs_val = getDialogueVar( err, ft_sel->lval ) ;
			if ( err.error() )
			{
				err.setftsrc( src_info_line( ft_crnt ) ) ;
				return ;
			}
		}
		else
		{
			lhs_val = ft_sel->lval ;
		}

		if ( ft_sel->rvar )
		{
			rhs_val = getDialogueVar( err, ft_sel->rval ) ;
			if ( err.error() )
			{
				err.setftsrc( src_info_line( ft_crnt ) ) ;
				return ;
			}
		}
		else
		{
			rhs_val = ft_sel->rval ;
		}

		if ( isnumeric( lhs_val ) && isnumeric( rhs_val ) )
		{
			lhs_ival = ds2d( lhs_val ) ;
			rhs_ival = ds2d( rhs_val ) ;
			numeric  = true ;
		}
		else
		{
			numeric = false ;
		}

		switch ( ft_sel->cond )
		{
			case RL_EQ:
				sub_true = ( numeric ) ? ( lhs_ival == rhs_ival ) : ( lhs_val == rhs_val ) ;
				break ;

			case RL_NE:
				sub_true = ( numeric ) ? ( lhs_ival != rhs_ival ) : ( lhs_val != rhs_val ) ;
				break ;

			case RL_GT:
				sub_true = ( numeric ) ? ( lhs_ival >  rhs_ival ) : ( lhs_val >  rhs_val ) ;
				break ;

			case RL_GE:
				sub_true = ( numeric ) ? ( lhs_ival >= rhs_ival ) : ( lhs_val >= rhs_val ) ;
				break ;

			case RL_LE:
				sub_true = ( numeric ) ? ( lhs_ival <= rhs_ival ) : ( lhs_val <= rhs_val ) ;
				break ;

			case RL_LT:
				sub_true = ( numeric ) ? ( lhs_ival <  rhs_ival ) : ( lhs_val <  rhs_val ) ;
				break ;
		}
		sel_true = ( sel_AND ) ? sel_true && sub_true : sel_true || sub_true ;
		sel_AND  = ft_sel->sel_AND ;
		ft_sel   = ft_sel->ft_sel_next ;
	}

	jump = !sel_true ;
}


void pFTailor::proc_input_set( errblock& err,
			       const ftc_main* ft_crnt )
{
	//
	// Process )SET statement
	//

	TRACE_FUNCTION() ;

	int result ;

	string expr = ft_crnt->ft_set->expr ;

	if ( expr.front() == '\'' )
	{
		expr = qstring1( err, expr ) ;
		if ( err.error() )
		{
			err.setftsrc( src_info_line( ft_crnt ) ) ;
			return ;
		}
		putDialogueVar( err, ft_crnt->ft_set->var, expr ) ;
		if ( err.error() )
		{
			err.setftsrc( src_info_line( ft_crnt ) ) ;
			return ;
		}
		return ;
	}

	if ( words( expr ) == 1 )
	{
		if ( expr.front() == char_var )
		{
			expr = getDialogueVar( err, expr.substr( 1 ) ) ;
			if ( err.error() )
			{
				err.setftsrc( src_info_line( ft_crnt ) ) ;
				return ;
			}
		}
		putDialogueVar( err, ft_crnt->ft_set->var, expr ) ;
		if ( err.error() )
		{
			err.setftsrc( src_info_line( ft_crnt ) ) ;
		}
		return ;
	}

	Lexer lexer( sub_vars( expr ) ) ;
	Interpreter interpreter( err, lexer ) ;
	if ( err.error() )
	{
		err.setftsrc( src_info_line( ft_crnt ) ) ;
		return ;
	}

	result = interpreter.expr( err ) ;
	if ( err.error() )
	{
		err.setftsrc( src_info_line( ft_crnt ) ) ;
		return ;
	}

	putDialogueVar( err, ft_crnt->ft_set->var, d2ds( result ) ) ;
	if ( err.error() )
	{
		err.setftsrc( src_info_line( ft_crnt ) ) ;
		return ;
	}
}


string pFTailor::cond_subs( errblock& err,
			    const ftc_main* ft_crnt )
{
	//
	// Perform conditional substitution.
	//
	// <&A|&B> becomes
	// &A      if &A is non-blank else
	// &B
	//
	// Also
	// << becomes <
	// || becomes |
	// >> becomes >
	//
	// These characters can be changed using the )DEFAULT statement.
	//

	TRACE_FUNCTION() ;

	string r = *ft_crnt->ft_line1 ;

	size_t p1 ;
	size_t p2 ;
	size_t p3 ;
	size_t p4 ;
	size_t p5 ;

	size_t l1 ;
	size_t l2 ;

	string t1 ;
	string t2 ;

	string val ;

	p1 = r.find( char_cs1 ) ;
	while ( p1 != string::npos )
	{
		if ( p1 < r.size()-1 && r[ p1+1 ] == char_cs1 )
		{
			r.erase( p1, 1 ) ;
			p1 = r.find( char_cs1, p1+1 ) ;
			continue ;
		}
		p2 = r.find_first_not_of( ' ', p1+1 ) ;
		if ( p2 == string::npos || r[ p2 ] != char_var )
		{
			err.seterrid( TRACE_INFO(), "PSYE026V", src_info( ft_crnt ) ) ;
			return "" ;
		}
		p3 = r.find( char_cs2, p2 ) ;
		if ( p3 == string::npos )
		{
			err.seterrid( TRACE_INFO(), "PSYE026V", src_info( ft_crnt ) ) ;
			return "" ;
		}
		p4 = r.find_first_not_of( ' ', p3+1 ) ;
		if ( p4 == string::npos )
		{
			err.seterrid( TRACE_INFO(), "PSYE026V", src_info( ft_crnt ) ) ;
			return "" ;
		}
		p5 = r.find( char_cs3, p4 ) ;
		if ( p5 == string::npos )
		{
			err.seterrid( TRACE_INFO(), "PSYE026V", src_info( ft_crnt ) ) ;
			return "" ;
		}
		t1 = strip( r.substr( p2, p3-p2 ) ) ;
		t2 = strip( r.substr( p4, p5-p4 ) ) ;
		l1 = t1.size() ;
		l2 = t2.size() ;
		if ( t1 == "" ||
		     t1.front() != char_var ||
		     !isvalidName( t1.substr( 1 ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE026V", src_info( ft_crnt ) ) ;
			return "" ;
		}
		t1 = getDialogueVar( err, t1.substr( 1 ) ) ;
		if ( err.error() )
		{
			err.setftsrc( src_info_line( ft_crnt ) ) ;
			return "" ;
		}
		if ( t2 != "" && t2.front() == char_var )
		{
			if ( !isvalidName( t2.substr( 1 ) ) )
			{
				err.seterrid( TRACE_INFO(), "PSYE026V", src_info( ft_crnt ) ) ;
				return "" ;
			}
			t2 = getDialogueVar( err, t2.substr( 1 ) ) ;
			if ( err.error() )
			{
				err.setftsrc( src_info_line( ft_crnt ) ) ;
				return "" ;
			}
		}
		if ( t1 == "" )
		{
			val = string( p4-p3-1, ' ' ) + t2 + string( p5-(p4+l2), ' ' ) ;
		}
		else
		{
			val = string( p2-p1-1, ' ' ) + t1 + string( p3-(p2+l1), ' ' ) ;
		}
		r.replace( p1, p5-p1+1, val ) ;
		p1 = r.find( char_cs1, p1+1 ) ;
	}

	t1 = string( 2, char_cs2 ) ;
	p1 = r.find( t1 ) ;
	while ( p1 != string::npos )
	{
		r.erase( p1, 1 ) ;
		p1 = r.find( t1, ( p1+1 ) ) ;
	}

	t1 = string( 2, char_cs3 ) ;
	p1 = r.find( t1 ) ;
	while ( p1 != string::npos )
	{
		r.erase( p1, 1 ) ;
		p1 = r.find( t1, ( p1+1 ) ) ;
	}

	return r ;
}


string pFTailor::apply_tabs( const string& s )
{
	//
	// Apply TABS to a string
	//

	TRACE_FUNCTION() ;

	string r = s ;

	size_t p = r.find( char_tab ) ;
	size_t t ;

	while ( p != string::npos )
	{
		if ( p < r.size()-1 && r[ p+1 ] == char_tab )
		{
			r.erase( p, 1 ) ;
			p = r.find( char_tab, p+1 ) ;
			continue ;
		}
		t = ( ft_curtab ) ? ft_curtab->get_tabpos( p ) : 255 ;
		if ( t > 0 )
		{
			r.replace( p, 1, ( t-p-1 ), ' ' ) ;
		}
		p = r.find( char_tab, p+1 ) ;
	}

	return r ;
}


void pFTailor::tbend( errblock& err,
		      const string& table )
{
	TRACE_FUNCTION() ;

	p_tableMGR->destroyTable( err,
				  table,
				  "TBEND" ) ;
}


void pFTailor::tbopen( errblock& err,
		       const string& table )
{
	TRACE_FUNCTION() ;

	pApplication* appl = static_cast<pApplication*>(pAppl) ;
	p_tableMGR->tbopen( err,
			    table,
			    NOWRITE,
			    appl->get_libdef_search_paths( s_ZTLIB ),
			    SHARE ) ;
}


void pFTailor::tbquery( errblock& err,
			const string& table,
			bool& cpairs )
{
	TRACE_FUNCTION() ;

	p_tableMGR->tbquery( err,
			     table,
			     cpairs ) ;
}


void pFTailor::tbsarg( errblock& err,
		       const string& table,
		       const string& cond_pairs )
{
	TRACE_FUNCTION() ;

	p_tableMGR->tbsarg( err,
			    funcPool,
			    table,
			    "",
			    "NEXT",
			    cond_pairs ) ;
}


void pFTailor::tbscan( errblock& err,
		       const string& table )
{
	TRACE_FUNCTION() ;

	p_tableMGR->tbscan( err,
			    funcPool,
			    table ) ;
}


void pFTailor::tbskip( errblock& err,
		       const string& table )
{
	TRACE_FUNCTION() ;

	p_tableMGR->tbskip( err,
			    funcPool,
			    table,
			    1 ) ;
}


void pFTailor::tbtop( errblock& err,
		      const string& table )
{
	TRACE_FUNCTION() ;

	p_tableMGR->tbtop( err,
			   table ) ;
}


bool pFTailor::loop_cond( errblock& err,
			  const ftc_main* ft_crnt,
			  int do_loop,
			  int do_to,
			  int do_for )
{
	//
	// Check the loop condition on a )DO statement.
	//

	TRACE_FUNCTION() ;

	if ( ft_crnt->ft_do->e_loop )
	{
		putDialogueVar( err, ft_crnt->ft_do->var, d2ds( do_loop ) ) ;
		if ( err.error() )
		{
			err.setftsrc( src_info_line( ft_crnt ) ) ;
			return false ;
		}
	}

	if ( ft_crnt->ft_do->e_to && do_loop > do_to )
	{
		return false ;
	}

	if ( ft_crnt->ft_do->e_for && do_for < 1 )
	{
		return false ;
	}

	if ( ft_crnt->ft_do->e_while )
	{
		return check_cond( err, ft_crnt ) ;
	}

	return true ;
}


bool pFTailor::check_cond( errblock& err,
			   const ftc_main* ft_crnt )
{
	//
	// Evaluate the WHILE or UNTIL statement.
	//
	// Only numeric values currently allowed.
	//

	TRACE_FUNCTION() ;

	bool result ;

	int rhs ;
	int lhs ;

	rhs = get_val( err, ft_crnt, ft_crnt->ft_do->v_rhs, ft_crnt->ft_do->l_rhs ) ;
	if ( err.error() ) { return false ; }

	lhs = get_val( err, ft_crnt, ft_crnt->ft_do->v_lhs, ft_crnt->ft_do->l_lhs ) ;
	if ( err.error() ) { return false ; }

	switch ( ft_crnt->ft_do->cond )
	{
	case RL_EQ:
		result = ( lhs == rhs ) ;
		break ;

	case RL_NE:
		result = ( lhs != rhs ) ;
		break ;

	case RL_GT:
		result = ( lhs > rhs ) ;
		break ;

	case RL_GE:
		result = ( lhs >= rhs ) ;
		break ;

	case RL_LE:
		result = ( lhs <= rhs ) ;
		break ;

	case RL_LT:
		result = ( lhs < rhs ) ;
		break ;
	}

	return result ;
}


string pFTailor::getDialogueVar( errblock& err,
				 const string& var )
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
	// RC =  0 Normal completion.
	// RC =  8 Variable not found.
	// RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	string* pstr ;

	fVAR* pvar ;

	pvar = funcPool->getfVAR( err, var ) ;
	if ( err.error() ) { return "" ; }

	if ( pvar )
	{
		return pvar->sget( var ) ;
	}
	else
	{
		pstr = p_poolMGR->vlocate( err, var ) ;
		if ( err.error() ) { return "" ; }
		switch ( err.getRC() )
		{
			case 0:
				return *pstr ;

			case 4:
				return p_poolMGR->get( err, var ) ;

			case 8:
				funcPool->put1( err, var, "" ) ;
		}
	}

	return "" ;
}


void pFTailor::putDialogueVar( errblock& err,
			       const string& var,
			       const string& val )
{
	//
	// Store data for a dialogue variable in the function pool.
	// Creates an implicit function pool variable if one does not already exist.
	//
	// If the variable has been vdefined as an INTEGER, convert to integer and put.
	//
	// RC =  0 Normal completion.
	// RC = 20 Severe error.
	//

	TRACE_FUNCTION() ;

	fVAR* pvar ;

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


int pFTailor::get_val( errblock& err,
		       const ftc_main* ft_crnt,
		       const string& s,
		       int i )
{
	//
	// Get integer value.
	//

	TRACE_FUNCTION() ;

	string temp ;

	if ( s != "" )
	{
		if ( s.front() != char_var )
		{
			err.seterrid( TRACE_INFO(), "PSYE013A", ")DO statement", s ) ;
			return 0 ;
		}
		temp = getDialogueVar( err, s.substr( 1 ) ) ;
		if ( err.error() ) { return 0 ; }
		if ( !datatype( temp, 'W' ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE027D", s.substr( 1 ), temp, src_info_line( ft_crnt ) ) ;
			return 0 ;
		}
		i = ds2d( temp ) ;
	}

	return i ;
}


string pFTailor::sub_var( errblock& err,
			  const string& v )
{
	TRACE_FUNCTION() ;

	return ( v.front() == char_var ) ? getDialogueVar( err, upper( v.substr( 1 ) ) ) : v ;
}


string pFTailor::sub_vars( string s )
{
	//
	// In string, s, substitute variables starting with char_var for their dialogue value.
	// The variable is delimited by any non-valid variable characters.
	// Rules: A '.' at the end of a variable, concatinates the value with the string following the dot
	//        .. reduces to .
	//        && reduces to & with no variable substitution.
	//
	// The default character & can be changed via the )DEFAULT control statement.
	//

	TRACE_FUNCTION() ;

	size_t p1 ;
	size_t p2 ;

	string var ;
	string val ;

	errblock err( taskId ) ;

	p1 = 0 ;

	while ( true )
	{
		p1 = s.find( char_var, p1 ) ;
		if ( p1 == string::npos || p1 == s.size() - 1 ) { break ; }
		++p1 ;
		if ( s[ p1 ] == char_var )
		{
			s.erase( p1, 1 ) ;
			p1 = s.find_first_not_of( char_var, p1 ) ;
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


string pFTailor::src_info( const ftc_main* m )
{
	TRACE_FUNCTION() ;

	return *m->name + " record-" + d2ds( m->ln ) ;
}


string pFTailor::src_info( const string& name,
			   int ln )
{
	TRACE_FUNCTION() ;

	return name + " record-" + d2ds( ln ) ;
}


string pFTailor::src_info_line( const ftc_main* m )
{
	TRACE_FUNCTION() ;

	return ( m->ft_line2 ) ? *m->name + " record-" + d2ds( m->ln ) + ": " + *m->ft_line2
			       : *m->name + " record-" + d2ds( m->ln ) ;
}
