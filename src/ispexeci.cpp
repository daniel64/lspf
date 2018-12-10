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

/*                                      */
/* ispexeci - ISPEXEC interface module  */
/*                                      */

void execiAddpop( pApplication*, const string&, errblock& )   ;
void execiBrowse( pApplication*, const string&, errblock& )   ;
void execiDeq( pApplication*, const string&, errblock& )      ;
void execiDisplay( pApplication*, const string&, errblock& )  ;
void execiControl( pApplication*, const string&, errblock& )  ;
void execiEdit( pApplication*, const string&, errblock& )     ;
void execiEdrec( pApplication*, const string&, errblock& )    ;
void execiEnq( pApplication*, const string&, errblock& )      ;
void execiGetmsg( pApplication*, const string&, errblock& )   ;
void execiLibdef( pApplication*, const string&, errblock& )   ;
void execiLog( pApplication*, const string&, errblock& )      ;
void execiPquery( pApplication*, const string&, errblock& )   ;
void execiQlibdef( pApplication*, const string&, errblock& )  ;
void execiQScan( pApplication*, const string&, errblock& )    ;
void execiQtabopen( pApplication*, const string&, errblock& ) ;
void execiRDisplay( pApplication*, const string&, errblock& ) ;
void execiRempop( pApplication*, const string&, errblock& )   ;
void execiSelect( pApplication*, const string&, errblock& )   ;
void execiSetmsg( pApplication*, const string&, errblock& )   ;
void execiTBAdd( pApplication*, const string&, errblock& )    ;
void execiTBBottom( pApplication*, const string&, errblock& ) ;
void execiTBCreate( pApplication*, const string&, errblock& ) ;
void execiTBClose( pApplication*, const string&, errblock& )  ;
void execiTBDelete( pApplication*, const string&, errblock& ) ;
void execiTBDispl( pApplication*, const string&, errblock& )  ;
void execiTBEnd( pApplication*, const string&, errblock& )    ;
void execiTBErase( pApplication*, const string&, errblock& )  ;
void execiTBExist( pApplication*, const string&, errblock& )  ;
void execiTBGet( pApplication*, const string&, errblock& )    ;
void execiTBMod( pApplication*, const string&, errblock& )    ;
void execiTBPut( pApplication*, const string&, errblock& )    ;
void execiTBOpen( pApplication*, const string&, errblock& )   ;
void execiTBQuery( pApplication*, const string&, errblock& )  ;
void execiTBSarg( pApplication*, const string&, errblock& )   ;
void execiTBSave( pApplication*, const string&, errblock& )   ;
void execiTBScan( pApplication*, const string&, errblock& )   ;
void execiTBSkip( pApplication*, const string&, errblock& )   ;
void execiTBSort( pApplication*, const string&, errblock& )   ;
void execiTBTop( pApplication*, const string&, errblock& )    ;
void execiTBVClear( pApplication*, const string&, errblock& ) ;
void execiVerase( pApplication*, const string&, errblock& )   ;
void execiVget( pApplication*, const string&, errblock& )     ;
void execiView( pApplication*, const string&, errblock& )     ;
void execiVput( pApplication*, const string&, errblock& )     ;

map<string, void(*)(pApplication*,const string&, errblock&)> execiServices = {
		  { "ADDPOP",   execiAddpop   },
		  { "BROWSE",   execiBrowse   },
		  { "DEQ",      execiDeq      },
		  { "DISPLAY",  execiDisplay  },
		  { "CONTROL",  execiControl  },
		  { "EDIT",     execiEdit     },
		  { "EDREC",    execiEdrec    },
		  { "ENQ",      execiEnq      },
		  { "GETMSG",   execiGetmsg   },
		  { "LIBDEF",   execiLibdef   },
		  { "LOG",      execiLog      },
		  { "PQUERY",   execiPquery   },
		  { "QLIBDEF",  execiQlibdef  },
		  { "QSCAN",    execiQScan    },
		  { "QTABOPEN", execiQtabopen },
		  { "RDISPLAY", execiRDisplay },
		  { "REMPOP",   execiRempop   },
		  { "SELECT",   execiSelect   },
		  { "SETMSG",   execiSetmsg   },
		  { "TBADD",    execiTBAdd    },
		  { "TBBOTTOM", execiTBBottom },
		  { "TBCREATE", execiTBCreate },
		  { "TBCLOSE",  execiTBClose  },
		  { "TBDELETE", execiTBDelete },
		  { "TBDISPL",  execiTBDispl  },
		  { "TBEND",    execiTBEnd    },
		  { "TBERASE",  execiTBErase  },
		  { "TBEXIST",  execiTBExist  },
		  { "TBGET",    execiTBGet    },
		  { "TBMOD",    execiTBMod    },
		  { "TBPUT",    execiTBPut    },
		  { "TBOPEN",   execiTBOpen   },
		  { "TBQUERY",  execiTBQuery  },
		  { "TBSARG",   execiTBSarg   },
		  { "TBSAVE",   execiTBSave   },
		  { "TBSCAN",   execiTBScan   },
		  { "TBSKIP",   execiTBSkip   },
		  { "TBSORT",   execiTBSort   },
		  { "TBTOP",    execiTBTop    },
		  { "TBVCLEAR", execiTBVClear },
		  { "VERASE",   execiVerase   },
		  { "VGET",     execiVget     },
		  { "VIEW",     execiView     },
		  { "VPUT",     execiVput     } } ;


void ispexeci( pApplication* thisAppl, const string& s, errblock& err )
{
	// Call sub_vars to resolve all dialogue variables in string 's' first
	// Then parse string to call the correct pApplication method for the service

	// Except for keywords relating to paths and files, the interface is case insensitive

	string s1 ;
	string w1 ;

	const string InvalidServices = "VDEFINE VDELETE VCOPY VMASK VREPLACE VRESET" ;

	map<string, void(*)(pApplication*, const string&, errblock&)>::iterator it ;

	s1 = thisAppl->sub_vars( s ) ;
	w1 = upper( word( s1, 1 ) )  ;

	err.setRC( 0 ) ;
	err.setsrc( s1 ) ;
	err.setdialogsrc() ;

	if ( findword( w1, InvalidServices ) )
	{
		err.seterrid( "PSYE019A", w1 ) ;
		return ;
	}

	it = execiServices.find( w1 ) ;
	if ( it == execiServices.end() )
	{
		err.seterrid( "PSYE019B", w1 ) ;
		return ;
	}

	it->second( thisAppl, s1, err ) ;
}


void execiAddpop( pApplication* thisAppl, const string& s, errblock& err )
{
	int i_row ;
	int i_col ;

	string str    ;

	string ap_loc ;
	string ap_row ;
	string ap_col ;

	str = upper( subword( s, 2 ) ) ;

	ap_loc = parseString( err, str, "POPLOC()" ) ;
	if ( err.error() ) { return ; }

	ap_row = parseString( err, str, "ROW()" ) ;
	if ( err.error() ) { return ; }

	if ( ap_row == "" ) { i_row = 0              ; }
	else                { i_row = ds2d( ap_row ) ; }

	ap_col = parseString( err, str, "COLUMN()" ) ;
	if ( err.error() ) { return ; }

	if ( ap_col == "" ) { i_col = 0              ; }
	else                { i_col = ds2d( ap_col ) ; }

	if ( str != "" )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->addpop( ap_loc, i_row, i_col ) ;
}


void execiBrowse( pApplication* thisAppl, const string& s, errblock& err )
{
	string str ;

	string br_panel ;
	string br_file  ;

	str = subword( s, 2 ) ;

	br_file = parseString( err, str, "FILE()" ) ;
	if ( err.error() ) { return ; }

	br_panel = parseString( err, str, "PANEL()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->browse( br_file, iupper( br_panel ) ) ;
}


void execiDeq( pApplication* thisAppl, const string& s, errblock& err )
{
	string t   ;
	string str ;
	string deq_qname ;
	string deq_rname ;

	enqSCOPE deq_scope = GLOBAL ;

	str = subword( s, 2 ) ;

	deq_qname = extractKWord( err, str, "QNAME()" ) ;
	if ( err.error() ) { return ; }

	deq_rname = extractKWord( err, str, "RNAME()" ) ;
	if ( err.error() ) { return ; }

	t = parseString( err, str, "LOCAL" ) ;
	if ( err.error() ) { return ; }
	if ( t == "OK" )
	{
		deq_scope = LOCAL ;
	}
	else
	{
		t = parseString( err, str, "GLOBAL" ) ;
		if ( err.error() ) { return ; }
		if ( t == "OK" )
		{
			deq_scope = GLOBAL ;
		}
	}

	if ( str != "" )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->deq( deq_qname, deq_rname, deq_scope ) ;
}


void execiControl( pApplication* thisAppl, const string& s, errblock& err )
{
	string s1 ;

	s1 = upper( s ) ;
	thisAppl->control( word( s1, 2 ), word( s1, 3 ), subword( s1, 4 ) ) ;
}


void execiDisplay( pApplication* thisAppl, const string& s, errblock& err )
{
	string str ;

	string di_panel ;
	string di_msg   ;
	string di_cursor ;
	string di_csrpos ;

	str = upper( subword( s, 2 ) ) ;

	di_panel = parseString( err, str, "PANEL()" ) ;
	if ( err.error() ) { return ; }

	di_msg = parseString( err, str, "MSG()" ) ;
	if ( err.error() ) { return ; }

	di_cursor = parseString( err, str, "CURSOR()" ) ;
	if ( err.error() ) { return ; }

	di_csrpos = parseString( err, str, "CSRPOS()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	if ( di_csrpos == "" ) { di_csrpos = "1" ; }

	thisAppl->display( di_panel, di_msg, di_cursor, ds2d( di_csrpos ) ) ;
}


void execiEdit( pApplication* thisAppl, const string& s, errblock& err )
{
	string str  ;

	string ed_panel ;
	string ed_macro ;
	string ed_profile ;
	string ed_file  ;
	string ed_lcmds ;
	string ed_confc ;
	string ed_presv ;

	str = subword( s, 2 ) ;

	ed_file = parseString( err, str, "FILE()" ) ;
	if ( err.error() ) { return ; }

	ed_panel = parseString( err, str, "PANEL()" ) ;
	if ( err.error() ) { return ; }

	ed_macro = parseString( err, str, "MACRO()" ) ;
	if ( err.error() ) { return ; }

	ed_profile = parseString( err, str, "PROFILE()" ) ;
	if ( err.error() ) { return ; }

	ed_lcmds = parseString( err, str, "LINECMDS()" ) ;
	if ( err.error() ) { return ; }

	ed_confc = parseString( err, str, "CONFIRM()" ) ;
	if ( err.error() ) { return ; }

	ed_presv = parseString( err, str, "PRESERVE" ) ;
	if ( err.error() ) { return ; }

	if ( ed_presv == "OK" ) { ed_presv = "PRESERVE" ; }

	if ( str != "" )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->edit( ed_file,
			iupper( ed_panel ),
			iupper( ed_macro ),
			iupper( ed_profile ),
			iupper( ed_lcmds ),
			iupper( ed_confc ),
			iupper( ed_presv ) ) ;
}


void execiEdrec( pApplication* thisAppl, const string& s, errblock& err )
{
	if ( words( s ) > 2 )
	{
		err.seterrid( "PSYE032H", subword( s, 3 ) ) ;
		return ;
	}

	thisAppl->edrec( upper( word( s, 2 ) ) ) ;
}


void execiEnq( pApplication* thisAppl, const string& s, errblock& err )
{
	string t   ;
	string str ;
	string enq_qname ;
	string enq_rname ;

	enqDISP  enq_disp  = EXC    ;
	enqSCOPE enq_scope = GLOBAL ;

	str = subword( s, 2 ) ;

	enq_qname = extractKWord( err, str, "QNAME()" ) ;
	if ( err.error() ) { return ; }

	enq_rname = extractKWord( err, str, "RNAME()" ) ;
	if ( err.error() ) { return ; }

	t = parseString( err, str, "LOCAL" ) ;
	if ( err.error() ) { return ; }
	if ( t == "OK" )
	{
		enq_scope = LOCAL ;
	}
	else
	{
		t = parseString( err, str, "GLOBAL" ) ;
		if ( err.error() ) { return ; }
		if ( t == "OK" )
		{
			enq_scope = GLOBAL ;
		}
	}

	iupper( str ) ;
	if      ( str == "SHR" ) { enq_disp = SHR ; }
	else if ( str == ""    ) { enq_disp = EXC ; }
	else
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->enq( enq_qname, enq_rname, enq_disp, enq_scope ) ;
}


void execiGetmsg( pApplication* thisAppl, const string& s, errblock& err )
{
	string gm_msg  ;
	string gm_smsg ;
	string gm_lmsg ;
	string gm_alarm ;
	string gm_help ;
	string gm_type ;
	string gm_window ;

	string str ;

	str = upper( subword( s, 2 ) ) ;

	gm_msg = parseString( err, str, "MSG()" ) ;
	if ( err.error() ) { return ; }

	gm_smsg = parseString( err, str, "SHORTMSG()" ) ;
	if ( err.error() ) { return ; }

	gm_lmsg = parseString( err, str, "LONGMSG()" ) ;
	if ( err.error() ) { return ; }

	gm_alarm = parseString( err, str, "ALARM()" ) ;
	if ( err.error() ) { return ; }

	gm_help = parseString( err, str, "HELP()" ) ;
	if ( err.error() ) { return ; }

	gm_type = parseString( err, str, "TYPE()" ) ;
	if ( err.error() ) { return ; }

	gm_window = parseString( err, str, "WINDOW()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->getmsg( gm_msg,
			  gm_smsg,
			  gm_lmsg,
			  gm_alarm,
			  gm_help,
			  gm_type,
			  gm_window ) ;
}


void execiLibdef( pApplication* thisAppl, const string& s, errblock& err )
{
	string str      ;

	string ld_files ;
	string ld_procopt ;

	str      = subword( s, 2 ) ;
	ld_files = parseString( err, str, "ID()" ) ;
	if ( err.error() ) { return ; }

	if ( words( str ) > 3 )
	{
		err.seterrid( "PSYE032H", subword( str, 4 ) ) ;
		return ;
	}

	iupper( str ) ;

	ld_procopt = word( str, 3 ) ;

	thisAppl->libdef( word( str, 1 ), word( str, 2 ), ld_files, ld_procopt ) ;
}


void execiLog( pApplication* thisAppl, const string& s, errblock& err )
{
	string str   ;

	string lg_msgid ;

	str = upper( subword( s, 2 ) ) ;
	lg_msgid = parseString( err, str, "MSG()" ) ;
	if ( err.error() ) { return ; }

	if ( words( str ) > 0 )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->log( lg_msgid ) ;
}


void execiPquery( pApplication* thisAppl, const string& s, errblock& err )
{
	string str      ;

	string pq_panel ;
	string pq_arean ;
	string pq_areat ;
	string pq_width ;
	string pq_depth ;
	string pq_row   ;
	string pq_col   ;

	str      = upper( subword( s, 2 ) ) ;
	pq_panel = parseString( err, str, "PANEL()" ) ;
	if ( err.error() ) { return ; }

	pq_arean = parseString( err, str, "AREANAME()" ) ;
	if ( err.error() ) { return ; }

	pq_areat = parseString( err, str, "AREATYPE()" ) ;
	if ( err.error() ) { return ; }

	pq_width = parseString( err, str, "WIDTH()" ) ;
	if ( err.error() ) { return ; }

	pq_depth = parseString( err, str, "DEPTH()" ) ;
	if ( err.error() ) { return ; }

	pq_row = parseString( err, str, "ROW()" ) ;
	if ( err.error() ) { return ; }

	pq_col = parseString( err, str, "COLUMN()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->pquery( pq_panel,
			  pq_arean,
			  pq_areat,
			  pq_width,
			  pq_depth,
			  pq_row,
			  pq_col ) ;
}


void execiQlibdef( pApplication* thisAppl, const string& s, errblock& err )
{
	string str     ;

	string ql_lib  ;
	string ql_type ;
	string ql_id   ;

	ql_lib  = upper( word( s, 2 ) ) ;
	str     = upper( subword( s, 3 ) ) ;
	ql_type = parseString( err, str, "TYPE()" ) ;
	if ( err.error() ) { return ; }

	ql_id = parseString( err, str, "ID()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->qlibdef( ql_lib, ql_type, ql_id ) ;
}


void execiQScan( pApplication* thisAppl, const string& s, errblock& err )
{
	string t   ;
	string str ;
	string qsc_qname ;
	string qsc_rname ;

	enqDISP  qsc_disp  = EXC    ;
	enqSCOPE qsc_scope = GLOBAL ;

	str = subword( s, 2 ) ;

	qsc_qname = extractKWord( err, str, "QNAME()" ) ;
	if ( err.error() ) { return ; }

	qsc_rname = extractKWord( err, str, "RNAME()" ) ;
	if ( err.error() ) { return ; }

	t = parseString( err, str, "LOCAL" ) ;
	if ( err.error() ) { return ; }
	if ( t == "OK" )
	{
		qsc_scope = LOCAL ;
	}
	else
	{
		t = parseString( err, str, "GLOBAL" ) ;
		if ( err.error() ) { return ; }
		if ( t == "OK" )
		{
			qsc_scope = GLOBAL ;
		}
	}

	iupper( str ) ;
	if      ( str == "SHR" ) { qsc_disp = SHR ; }
	else if ( str == ""    ) { qsc_disp = EXC ; }
	else
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->qscan( qsc_qname, qsc_rname, qsc_disp, qsc_scope ) ;
}


void execiQtabopen( pApplication* thisAppl, const string& s, errblock& err )
{
	string str     ;
	string qt_list ;

	str     = upper( subword( s, 2 ) ) ;
	qt_list = parseString( err, str, "LIST()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->qtabopen( qt_list ) ;
}


void execiRDisplay( pApplication* thisAppl, const string& s, errblock& err )
{
	// Call rdisplay(s,false) which does not do dialogue variable substitution as this has
	// already been done.

	thisAppl->rdisplay( substr( s, 10 ), false ) ;
}


void execiRempop( pApplication* thisAppl, const string& s, errblock& err )
{
	thisAppl->rempop( upper( subword( s, 2 ) ) ) ;
}


void execiSelect( pApplication* thisAppl, const string& s, errblock& err )
{
	selobj SEL ;

	if ( !SEL.parse( err, subword( s, 2 ) ) ) { return ; }

	thisAppl->select( SEL ) ;
}


void execiSetmsg( pApplication* thisAppl, const string& s, errblock& err )
{
	string str   ;
	string s_msg ;

	msgSET t ;

	str = upper( subword( s, 2 ) ) ;

	s_msg = parseString( err, str, "MSG()" ) ;
	if ( err.error() ) { return ; }

	if      ( str == "COND" ) { t = COND   ; }
	else if ( str == "" )     { t = UNCOND ; }
	else
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->setmsg( s_msg, t ) ;
}


void execiTBAdd( pApplication* thisAppl, const string& s, errblock& err )
{
	string str        ;

	string tb_name    ;
	string tb_namelst ;
	string tb_numrows ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_namelst = parseString( err, str, "SAVE()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_namelst != "" ) { tb_namelst = "(" + tb_namelst + ")" ; }

	tb_numrows = parseString( err, str, "MULT()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_numrows == "" ) { tb_numrows = "0" ; }

	thisAppl->tbadd( tb_name, tb_namelst, str, ds2d( tb_numrows ) ) ;
}


void execiTBBottom( pApplication* thisAppl, const string& s, errblock& err )
{
	string str       ;

	string tb_name   ;
	string tb_savenm ;
	string tb_crpnm  ;
	string tb_rowid  ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_savenm = parseString( err, str, "SAVENAME()" ) ;
	if ( err.error() ) { return ; }

	tb_rowid = parseString( err, str, "ROWID()" ) ;
	if ( err.error() ) { return ; }

	tb_crpnm = parseString( err, str, "POSITION()" ) ;
	if ( err.error() ) { return ; }

	thisAppl->tbbottom( tb_name, tb_savenm, tb_rowid, str, tb_crpnm ) ;
}


void execiTBClose( pApplication* thisAppl, const string& s, errblock& err )
{
	string str      ;

	string tb_name  ;
	string tb_nname ;
	string tb_path  ;

	tb_name  = upper( word( s, 2 ) ) ;
	str      = subword( s, 3 )       ;

	tb_nname = parseString( err, str, "NAME()" ) ;
	if ( err.error() ) { return ; }

	tb_path = parseString( err, str, "LIBRARY()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->tbclose( tb_name, tb_nname, tb_path ) ;
}


void execiTBCreate( pApplication* thisAppl, const string& s, errblock& err )
{
	string str      ;
	string t        ;

	string tb_name  ;
	string tb_keys  ;
	string tb_names ;
	string tb_paths ;

	tbWRITE tb_write ;
	tbDISP  tb_disp  ;
	tbREP   tb_rep   ;

	tb_name = upper( word( s, 2 ) ) ;
	str     = subword( s, 3 ) ;

	tb_paths = parseString( err, str, "LIBRARY()" ) ;
	if ( err.error() ) { return ; }

	iupper( str ) ;
	tb_keys  = parseString( err, str, "KEYS()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_keys != "" ) { tb_keys = "(" + tb_keys + ")" ; }

	tb_names = parseString( err, str, "NAMES()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_names != "" ) { tb_names = "(" + tb_names + ")" ; }

	tb_write = WRITE ;
	t = parseString( err, str, "WRITE" ) ;
	if ( err.error() ) { return ; }
	if ( t == "" )
	{
		t = parseString( err, str, "NOWRITE" ) ;
		if ( err.error() ) { return ; }
		if ( t != "" ) { tb_write = NOWRITE ; }
	}

	tb_rep = NOREPLACE ;
	t = parseString( err, str, "REPLACE" ) ;
	if ( err.error() ) { return ; }

	if ( t != "" ) { tb_rep = REPLACE ; }

	tb_disp = EXCLUSIVE ;
	t = parseString( err, str, "SHARE" ) ;
	if ( err.error() ) { return ; }
	if ( t != "" ) { tb_disp = SHARE ; }

	if ( words( str ) > 0 )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->tbcreate( tb_name,
			    tb_keys,
			    tb_names,
			    tb_write,
			    tb_rep,
			    tb_paths,
			    tb_disp ) ;
}


void execiTBDelete( pApplication* thisAppl, const string& s, errblock& err )
{
	if ( words( s ) > 2 )
	{
		err.seterrid( "PSYE032H", subword( s, 3 ) ) ;
		return ;
	}

	thisAppl->tbdelete( upper( word( s, 2 ) ) ) ;
}


void execiTBDispl( pApplication* thisAppl, const string& s, errblock& err )
{
	int i_csrpos ;
	int i_csrrow ;

	string str  ;

	string tb_name  ;
	string tb_panel ;
	string tb_msg   ;
	string tb_cursor  ;
	string tb_csrrow  ;
	string tb_csrpos  ;
	string tb_autosel ;
	string tb_posn    ;
	string tb_rowid   ;
	string tb_msgloc  ;

	tb_name = upper( word( s, 2 ) ) ;
	str = upper( subword( s, 3 ) ) ;

	tb_panel = parseString( err, str, "PANEL()" ) ;
	if ( err.error() ) { return ; }

	tb_msg = parseString( err, str, "MSG()" ) ;
	if ( err.error() ) { return ; }

	tb_cursor = parseString( err, str, "CURSOR()" ) ;
	if ( err.error() ) { return ; }

	tb_csrrow = parseString( err, str, "CSRROW()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_csrrow == "" ) { i_csrrow = 0                 ; }
	else                   { i_csrrow = ds2d( tb_csrrow ) ; }

	tb_csrpos = parseString( err, str, "CSRPOS()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_csrpos == "" ) { i_csrpos = 1                 ; }
	else                   { i_csrpos = ds2d( tb_csrpos ) ; }

	tb_autosel = parseString( err, str, "AUTOSEL()" ) ;
	if ( err.error() ) { return ; }

	tb_posn = parseString( err, str, "POSITION()" ) ;
	if ( err.error() ) { return ; }

	tb_rowid = parseString( err, str, "ROWID()" ) ;
	if ( err.error() ) { return ; }

	tb_msgloc = parseString( err, str, "MSGLOC()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->tbdispl( tb_name,
			   tb_panel,
			   tb_msg,
			   tb_cursor,
			   i_csrrow,
			   i_csrpos,
			   tb_autosel,
			   tb_posn,
			   tb_rowid,
			   tb_msgloc ) ;
}


void execiTBEnd( pApplication* thisAppl, const string& s, errblock& err )
{
	if ( words( s ) > 2 )
	{
		err.seterrid( "PSYE032H", subword( s, 3 ) ) ;
		return ;
	}

	thisAppl->tbend( upper( word( s, 2 ) ) ) ;
}


void execiTBErase( pApplication* thisAppl, const string& s, errblock& err )
{
	string str     ;

	string tb_name ;
	string tb_path ;

	tb_name = upper( word( s, 2 ) ) ;
	str     = subword( s, 3 ) ;

	tb_path = parseString( err, str, "LIBRARY()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->tberase( tb_name, tb_path ) ;
}


void execiTBExist( pApplication* thisAppl, const string& s, errblock& err )
{
	if ( words( s ) > 2 )
	{
		err.seterrid( "PSYE032H", subword( s, 3 ) ) ;
		return ;
	}

	thisAppl->tbexist( upper( word( s, 2 ) ) ) ;
}


void execiTBGet( pApplication* thisAppl, const string& s, errblock& err )
{
	string str       ;

	string tb_name   ;
	string tb_savenm ;
	string tb_crpnm  ;
	string tb_rowid  ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_savenm = parseString( err, str, "SAVENAME()" ) ;
	if ( err.error() ) { return ; }

	tb_rowid = parseString( err, str, "ROWID()" ) ;
	if ( err.error() ) { return ; }

	tb_crpnm = parseString( err, str, "POSITION()" ) ;
	if ( err.error() ) { return ; }

	thisAppl->tbget( tb_name,
			 tb_savenm,
			 tb_rowid,
			 str,
			 tb_crpnm ) ;
}


void execiTBMod( pApplication* thisAppl, const string& s, errblock& err )
{
	string str       ;

	string tb_name   ;
	string tb_savenm ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_savenm = parseString( err, str, "SAVE()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_savenm != "" ) { tb_savenm = "(" + tb_savenm + ")" ; }

	thisAppl->tbmod( tb_name, tb_savenm, str ) ;
}


void execiTBPut( pApplication* thisAppl, const string& s, errblock& err )
{
	string str       ;

	string tb_name   ;
	string tb_savenm ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_savenm = parseString( err, str, "SAVE()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_savenm != "" ) { tb_savenm = "(" + tb_savenm + ")" ; }

	thisAppl->tbput( tb_name, tb_savenm, str ) ;
}


void execiTBOpen( pApplication* thisAppl, const string& s, errblock& err )
{
	string str      ;
	string t        ;

	string tb_name  ;
	string tb_paths ;

	tbWRITE tb_write ;
	tbDISP  tb_disp  ;

	tb_name = upper( word( s, 2 ) ) ;
	str     = subword( s, 3 ) ;

	tb_write = WRITE ;
	t = parseString( err, str, "WRITE" ) ;
	if ( err.error() ) { return ; }

	if ( t == "" )
	{
		t = parseString( err, str, "NOWRITE" ) ;
		if ( err.error() ) { return ; }
		if ( t != "" ) { tb_write = NOWRITE ; }
	}

	tb_paths = parseString( err, str, "LIBRARY()" ) ;
	if ( err.error() ) { return ; }

	iupper( str ) ;
	if      ( str == "SHARE" ) { tb_disp = SHARE     ; }
	else if ( str == ""      ) { tb_disp = EXCLUSIVE ; }
	else
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->tbopen( tb_name,
			  tb_write,
			  tb_paths,
			  tb_disp ) ;
}


void execiTBQuery( pApplication* thisAppl, const string& s, errblock& err )
{
	string str        ;

	string tb_name    ;
	string tb_keys    ;
	string tb_names   ;
	string tb_rownum  ;
	string tb_keynum  ;
	string tb_namenum ;
	string tb_pos     ;
	string tb_srtflds ;
	string tb_sarglst ;
	string tb_sargcnd ;
	string tb_sargdir ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_keys = parseString( err, str, "KEYS()" ) ;
	if ( err.error() ) { return ; }

	tb_names = parseString( err, str, "NAMES()" ) ;
	if ( err.error() ) { return ; }

	tb_rownum = parseString( err, str, "ROWNUM()" ) ;
	if ( err.error() ) { return ; }

	tb_keynum = parseString( err, str, "KEYNUM()" ) ;
	if ( err.error() ) { return ; }

	tb_namenum = parseString( err, str, "NAMENUM()" ) ;
	if ( err.error() ) { return ; }

	tb_pos = parseString( err, str, "POSITION()" ) ;
	if ( err.error() ) { return ; }

	tb_srtflds = parseString( err, str, "SORTFLDS()" ) ;
	if ( err.error() ) { return ; }

	tb_sarglst = parseString( err, str, "SARGLIST()" ) ;
	if ( err.error() ) { return ; }

	tb_sargcnd = parseString( err, str, "SARGCOND()" ) ;
	if ( err.error() ) { return ; }

	tb_sargdir = parseString( err, str, "SARGDIR()" ) ;
	if ( err.error() ) { return ; }

	if ( words( str ) > 0 )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->tbquery( tb_name,
			   tb_keys,
			   tb_names,
			   tb_rownum,
			   tb_keynum,
			   tb_namenum,
			   tb_pos,
			   tb_srtflds,
			   tb_sarglst,
			   tb_sargcnd,
			   tb_sargdir ) ;
}


void execiTBSarg( pApplication* thisAppl, const string& s, errblock& err )
{
	string str ;

	string tb_name  ;
	string tb_arglst  ;
	string tb_namecnd ;

	tb_name  = upper( word( s, 2 ) )    ;
	str      = upper( subword( s, 3 ) ) ;

	tb_arglst = parseString( err, str, "ARGLIST()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_arglst != "" ) { tb_arglst = "(" + tb_arglst + ")" ; }

	tb_namecnd = parseString( err, str, "NAMECOND()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_namecnd != "" ) { tb_namecnd = "(" + tb_namecnd + ")" ; }

	thisAppl->tbsarg( tb_name, tb_arglst, str, tb_namecnd ) ;
}


void execiTBSave( pApplication* thisAppl, const string& s, errblock& err )
{
	string str      ;

	string tb_name  ;
	string tb_nname ;
	string tb_path  ;

	tb_name  = upper( word( s, 2 ) ) ;
	str      = subword( s, 3 ) ;

	tb_nname = parseString( err, str, "NAME()" ) ;
	if ( err.error() ) { return ; }

	tb_path = parseString( err, str, "LIBRARY()" ) ;
	if ( err.error() ) { return ; }

	if ( words( str ) > 0 )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->tbsave( tb_name, tb_nname, tb_path ) ;
}


void execiTBScan( pApplication* thisAppl, const string& s, errblock& err )
{
	string str ;
	string t   ;

	string tb_name    ;
	string tb_arglst  ;
	string tb_savenm  ;
	string tb_rowid   ;
	string tb_crpnm   ;
	string tb_condlst ;
	string tb_dir     ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_arglst = parseString( err, str, "ARGLIST()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_arglst != "" ) { tb_arglst = "(" + tb_arglst + ")" ; }

	tb_savenm = parseString( err, str, "SAVENAME()" ) ;
	if ( err.error() ) { return ; }

	tb_rowid = parseString( err, str, "ROWID()" ) ;
	if ( err.error() ) { return ; }

	tb_crpnm = parseString( err, str, "POSITION()" ) ;
	if ( err.error() ) { return ; }

	tb_condlst = parseString( err, str, "CONDLIST()" ) ;
	if ( err.error() ) { return ; }
	if ( tb_condlst != "" ) { tb_condlst = "(" + tb_condlst + ")" ; }

	tb_dir = "NEXT" ;
	t = parseString( err, str, "NEXT" ) ;
	if ( err.error() ) { return ; }
	if ( t == "" )
	{
		t = parseString( err, str, "PREVIOUS" ) ;
		if ( err.error() ) { return ; }
		if ( t != "" ) { tb_dir = "PREVIOUS" ; }
	}

	thisAppl->tbscan( tb_name,
			  tb_arglst,
			  tb_savenm,
			  tb_rowid,
			  tb_dir,
			  str,
			  tb_crpnm,
			  tb_condlst ) ;
}


void execiTBSkip( pApplication* thisAppl, const string& s, errblock& err )
{
	int i_num ;

	string str ;

	string tb_name   ;
	string tb_num    ;
	string tb_savenm ;
	string tb_crpnm  ;
	string tb_rowid  ;
	string tb_row    ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_num = parseString( err, str, "NUMBER()" ) ;
	if ( err.error() ) { return ; }

	if ( tb_num == "" ) { i_num = 1              ; }
	else                { i_num = ds2d( tb_num ) ; }

	tb_savenm = parseString( err, str, "SAVENAME()" ) ;
	if ( err.error() ) { return ; }

	tb_rowid = parseString( err, str, "ROWID()" ) ;
	if ( err.error() ) { return ; }

	tb_row = parseString( err, str, "ROW()" ) ;
	if ( err.error() ) { return ; }

	tb_crpnm = parseString( err, str, "POSITION()" ) ;
	if ( err.error() ) { return ; }

	thisAppl->tbskip( tb_name,
			  i_num,
			  tb_savenm,
			  tb_rowid,
			  tb_row,
			  str,
			  tb_crpnm ) ;
}


void execiTBSort( pApplication* thisAppl, const string& s, errblock& err )
{
	string str     ;

	string tb_name ;
	string tb_flds ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_flds = parseString( err, str, "FIELDS()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}
	if ( tb_flds != "" ) { tb_flds = "(" + tb_flds + ")" ; }

	thisAppl->tbsort( tb_name, tb_flds ) ;
}


void execiTBTop( pApplication* thisAppl, const string& s, errblock& err )
{
	if ( words( s ) > 2 )
	{
		err.seterrid( "PSYE032H", subword( s, 3 ) ) ;
		return ;
	}

	thisAppl->tbtop( upper( word( s, 2 ) ) ) ;
}


void execiTBVClear( pApplication* thisAppl, const string& s, errblock& err )
{
	if ( words( s ) > 2 )
	{
		err.seterrid( "PSYE032H", subword( s, 3 ) ) ;
		return ;
	}

	thisAppl->tbvclear( upper( word( s, 2 ) ) ) ;
}


void execiVerase( pApplication* thisAppl, const string& s, errblock& err )
{
	string str  ;
	string vars ;

	poolType pType ;

	str = upper( subword( s, 2 ) ) ;

	vars = parseString( err, str, "()" ) ;
	if ( err.error() ) { return ; }

	replace( vars.begin(), vars.end(), ',', ' ' ) ;
	if ( words( vars ) == 0 )
	{
		vars = word( str, 1 )    ;
		str  = subword( str, 2 ) ;
	}

	if      ( str == "SHARED"  ) { pType = SHARED  ; }
	else if ( str == "PROFILE" ) { pType = PROFILE ; }
	else if ( str == "ASIS"    ) { pType = ASIS    ; }
	else if ( str == ""        ) { pType = ASIS    ; }
	else if ( str == "BOTH"    ) { pType = BOTH    ; }
	else
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->verase( vars, pType ) ;
}


void execiVget( pApplication* thisAppl, const string& s, errblock& err )
{
	// If this is called from the REXX interface module, VREPLACE first with nulls so a variable not found
	// results in a blank value, instead of the variable name.

	int i  ;
	int n  ;

	string str  ;
	string vars ;
	string var  ;

	poolType pType ;

	str = upper( subword( s, 2 ) ) ;

	vars = parseString( err, str, "()" ) ;
	if ( err.error() ) { return ; }

	replace( vars.begin(), vars.end(), ',', ' ' ) ;
	n = words( vars ) ;
	if ( n == 0 )
	{
		vars = word( str, 1 )    ;
		n    = 1                 ;
		str  = subword( str, 2 ) ;
	}

	if ( thisAppl->rexxName != "" )
	{
		for ( i = 1 ; i <= n ; i++ )
		{
			var = word( vars, i ) ;
			if ( isvalidName( var ) )
			{
				thisAppl->vreplace( var, "" ) ;
			}
		}
	}

	if      ( str == "SHARED"  ) { pType = SHARED  ; }
	else if ( str == "PROFILE" ) { pType = PROFILE ; }
	else if ( str == "ASIS"    ) { pType = ASIS    ; }
	else if ( str == ""        ) { pType = ASIS    ; }
	else
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->vget( vars, pType ) ;
}


void execiView( pApplication* thisAppl, const string& s, errblock& err )
{
	string str ;

	string vi_panel ;
	string vi_file  ;

	str = subword( s, 2 ) ;

	vi_file = parseString( err, str, "FILE()" ) ;
	if ( err.error() ) { return ; }

	vi_panel = parseString( err, str, "PANEL()" ) ;
	if ( err.error() ) { return ; }

	if ( str != "" )
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->view( vi_file, iupper( vi_panel ) ) ;
}


void execiVput( pApplication* thisAppl, const string& s, errblock& err )
{
	string str  ;
	string vars ;

	poolType pType ;

	str = upper( subword( s, 2 ) ) ;

	vars = parseString( err, str, "()" ) ;
	if ( err.error() ) { return ; }

	replace( vars.begin(), vars.end(), ',', ' ' ) ;
	if ( words( vars ) == 0 )
	{
		vars = word( str, 1 )    ;
		str  = subword( str, 2 ) ;
	}

	if      ( str == "SHARED"  ) { pType = SHARED  ; }
	else if ( str == "PROFILE" ) { pType = PROFILE ; }
	else if ( str == "ASIS"    ) { pType = ASIS    ; }
	else if ( str == ""        ) { pType = ASIS    ; }
	else
	{
		err.seterrid( "PSYE032H", str ) ;
		return ;
	}

	thisAppl->vput( vars, pType ) ;
}
