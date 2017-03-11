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

void execiAddpop( pApplication *, const string&, errblock& )   ;
void execiBrowse( pApplication *, const string&, errblock& )   ;
void execiDisplay( pApplication *, const string&, errblock& )  ;
void execiControl( pApplication *, const string&, errblock& )  ;
void execiEdit( pApplication *, const string&, errblock& )     ;
void execiGetmsg( pApplication *, const string&, errblock& )   ;
void execiLibdef( pApplication *, const string&, errblock& )   ;
void execiPquery( pApplication *, const string&, errblock& )   ;
void execiRDisplay( pApplication *, const string&, errblock& ) ;
void execiRempop( pApplication *, const string&, errblock& )   ;
void execiSelect( pApplication *, const string&, errblock& )   ;
void execiSetmsg( pApplication *, const string&, errblock& )   ;
void execiTBAdd( pApplication *, const string&, errblock& )    ;
void execiTBBottom( pApplication *, const string&, errblock& ) ;
void execiTBCreate( pApplication *, const string&, errblock& ) ;
void execiTBClose( pApplication *, const string&, errblock& )  ;
void execiTBDelete( pApplication *, const string&, errblock& ) ;
void execiTBDispl( pApplication *, const string&, errblock& )  ;
void execiTBEnd( pApplication *, const string&, errblock& )    ;
void execiTBErase( pApplication *, const string&, errblock& )  ;
void execiTBExist( pApplication *, const string&, errblock& )  ;
void execiTBGet( pApplication *, const string&, errblock& )    ;
void execiTBMod( pApplication *, const string&, errblock& )    ;
void execiTBPut( pApplication *, const string&, errblock& )    ;
void execiTBOpen( pApplication *, const string&, errblock& )   ;
void execiTBQuery( pApplication *, const string&, errblock& )  ;
void execiTBSarg( pApplication *, const string&, errblock& )   ;
void execiTBSave( pApplication *, const string&, errblock& )   ;
void execiTBScan( pApplication *, const string&, errblock& )   ;
void execiTBSkip( pApplication *, const string&, errblock& )   ;
void execiTBSort( pApplication *, const string&, errblock& )   ;
void execiTBTop( pApplication *, const string&, errblock& )    ;
void execiTBVClear( pApplication *, const string&, errblock& ) ;
void execiVerase( pApplication *, const string&, errblock& )   ;
void execiVget( pApplication *, const string&, errblock& )     ;
void execiView( pApplication *, const string&, errblock& )     ;
void execiVput( pApplication *, const string&, errblock& )     ;

map<string, void(*)(pApplication *,const string&, errblock&)> execiServices = {
		  { "ADDPOP",   execiAddpop   },
		  { "BROWSE",   execiBrowse   },
		  { "DISPLAY",  execiDisplay  },
		  { "CONTROL",  execiControl  },
		  { "EDIT",     execiEdit     },
		  { "GETMSG",   execiGetmsg   },
		  { "LIBDEF",   execiLibdef   },
		  { "PQUERY",   execiPquery   },
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


void ispexeci( pApplication * thisAppl, const string& s, errblock& err )
{
	// Call sub_vars to resolve all dialogue variables in string 's' first
	// Then parse string to call the correct pApplication method for the service

	// Except for keywords relating to paths and files, the interface is case insensitive

	string s1 ;
	string w1 ;

	const string InvalidServices = "VDEFINE VDELETE VCOPY VMASK VREPLACE VRESET" ;

	map<string, void(*)(pApplication *, const string&, errblock&)>::iterator it ;

	err.setRC( 0 )   ;
	err.setsrc( &s ) ;

	s1 = thisAppl->sub_vars( s ) ;
	w1 = upper( word( s1, 1 ) )  ;

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
	return ;
}


void execiAddpop( pApplication * thisAppl, const string& s, errblock& err )
{
	int i_row ;
	int i_col ;

	bool  rlt ;

	string str    ;
	string ap_loc ;
	string ap_row ;
	string ap_col ;

	str = upper( subword( s, 2 ) ) ;

	ap_loc = parseString( rlt, str, "POPLOC()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	ap_row = parseString( rlt, str, "ROW()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	if ( ap_row == "" ) { i_row = 0              ; }
	else                { i_row = ds2d( ap_row ) ; }

	ap_col = parseString( rlt, str, "COLUMN()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	if ( ap_col == "" ) { i_col = 0              ; }
	else                { i_col = ds2d( ap_col ) ; }

	if ( str != "" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->addpop( ap_loc, i_row, i_col ) ;
	return ;
}


void execiBrowse( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string str ;
	string pan ;
	string fl  ;

	str = subword( s, 2 ) ;

	fl = parseString( rlt, str, "FILE()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	pan = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt || str != "" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->browse( fl, iupper( pan ) ) ;
	return ;
}


void execiControl( pApplication * thisAppl, const string& s, errblock& err )
{
	string s1 ;

	s1 = upper( s ) ;
	thisAppl->control( word( s1, 2 ), word( s1, 3 ), subword( s1, 4 ) ) ;
	return ;
}


void execiDisplay( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string str ;
	string pan ;
	string msg ;
	string cursor ;
	string csrpos ;

	str = upper( subword( s, 2 ) ) ;

	pan = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	msg = parseString( rlt, str, "MSG()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	cursor = parseString( rlt, str, "CURSOR()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	csrpos = parseString( rlt, str, "CSRPOS()" ) ;
	if ( !rlt || str != "" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	if ( csrpos == "" ) { csrpos = "1" ; }

	thisAppl->display( pan, msg, cursor, ds2d( csrpos ) ) ;
	return ;
}


void execiEdit( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string str  ;
	string pan  ;
	string mac  ;
	string prof ;
	string fl   ;

	str = subword( s, 2 ) ;

	fl = parseString( rlt, str, "FILE()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	pan = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	mac = parseString( rlt, str, "MACRO()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	prof = parseString( rlt, str, "PROFILE()" ) ;
	if ( !rlt || str != "" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}


	thisAppl->edit( fl, iupper( pan ), iupper( mac ), iupper( prof ) ) ;
	return ;
}


void execiGetmsg( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string msg  ;
	string smsg ;
	string lmsg ;
	string alm  ;
	string hlp  ;
	string typ  ;
	string wndo ;

	string str  ;

	str = upper( subword( s, 2 ) ) ;

	msg = parseString( rlt, str, "MSG()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	smsg = parseString( rlt, str, "SHORTMSG()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	lmsg = parseString( rlt, str, "LONGMSG()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	alm = parseString( rlt, str, "ALARM()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	hlp = parseString( rlt, str, "HELP()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	typ = parseString( rlt, str, "TYPE()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	wndo = parseString( rlt, str, "WINDOW()" ) ;
	if ( !rlt || str != "" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->getmsg( msg, smsg, lmsg, alm, hlp, typ, wndo ) ;
	return ;
}


void execiLibdef( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string ld_files ;
	string str      ;

	str      = subword( s, 3 ) ;
	ld_files = parseString( rlt, str, "ID()" ) ;
	if ( !rlt || str != "" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->libdef( word( s, 2 ), ld_files ) ;
	return ;
}


void execiPquery( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string str      ;
	string pq_panel ;
	string pq_arean ;
	string pq_areat ;
	string pq_width ;
	string pq_depth ;
	string pq_row   ;
	string pq_col   ;

	str      = upper( subword( s, 2 ) ) ;
	pq_panel = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	pq_arean = parseString( rlt, str, "AREANAME()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	pq_areat = parseString( rlt, str, "AREATYPE()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	pq_width = parseString( rlt, str, "WIDTH()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	pq_depth = parseString( rlt, str, "DEPTH()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	pq_row = parseString( rlt, str, "ROW()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	pq_col = parseString( rlt, str, "COLUMN()" ) ;
	if ( !rlt || str != "" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->pquery( pq_panel, pq_arean, pq_areat, pq_width, pq_depth, pq_row, pq_col ) ;
	return ;
}


void execiRDisplay( pApplication * thisAppl, const string& s, errblock& err )
{
	// Call rdisplay(s,false) which does not do dialogue variable substitution as this has
	// already been done.

	thisAppl->rdisplay( substr( s, 10 ), false ) ;
	return ;
}


void execiRempop( pApplication * thisAppl, const string& s, errblock& err )
{
	string ap_all ;

	ap_all = upper( subword( s, 2 ) ) ;

	if ( ap_all != "" && ap_all != "ALL" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->rempop( ap_all ) ;
	return ;
}


void execiSelect( pApplication * thisAppl, const string& s, errblock& err )
{
	// The SELECT parser may replace PGM with a variable name (eg. for a REXX command), so
	// substitute with its dialogue variable value

	selobj SEL ;

	if ( !SEL.parse( subword( s, 2 ) ) )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}


	if ( SEL.PGM[ 0 ] == '&' )
	{
		thisAppl->vcopy( SEL.PGM.erase( 0, 1 ), SEL.PGM, MOVE ) ;
	}

	thisAppl->select( SEL ) ;
	return ;
}


void execiSetmsg( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string str   ;
	string s_msg ;

	msgSET t ;

	str = upper( subword( s, 2 ) ) ;

	s_msg = parseString( rlt, str, "MSG()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	if      ( str == "COND" ) { t = COND   ; }
	else if ( str == "" )     { t = UNCOND ; }
	else
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->setmsg( s_msg, t ) ;
	return ;
}


void execiTBAdd( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string str        ;
	string tb_name    ;
	string tb_namelst ;
	string tb_order   ;
	string tb_numrows ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_namelst = parseString( rlt, str, "SAVE()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_numrows = parseString( rlt, str, "MULT()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}
	if ( tb_numrows == "" ) { tb_numrows = "1" ; }

	tb_order = str ;
	if ( tb_order != "" && tb_order != "ORDER" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbadd( tb_name, tb_namelst, tb_order, ds2d( tb_numrows ) ) ;
	return ;
}


void execiTBBottom( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string str       ;
	string t         ;
	string tb_name   ;
	string tb_savenm ;
	string tb_crpnm  ;
	string tb_rowid  ;
	string tb_noread ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_savenm = parseString( rlt, str, "SAVENAME()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_rowid = parseString( rlt, str, "ROWID()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_crpnm = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_noread = str ;
	if ( tb_noread != "" && tb_noread != "NOREAD" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbbottom( tb_name, tb_savenm, tb_rowid, tb_noread, tb_crpnm ) ;
	return ;
}


void execiTBClose( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string str      ;
	string tb_name  ;
	string tb_nname ;
	string tb_path  ;

	tb_name  = upper( word( s, 2 ) ) ;
	str      = subword( s, 3 )       ;

	tb_nname = parseString( rlt, str, "NAME()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_path = parseString( rlt, str, "LIBRARY()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbclose( tb_name, tb_nname, tb_path ) ;
	return ;
}


void execiTBCreate( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string str      ;
	string t        ;
	string tb_name  ;
	string tb_keys  ;
	string tb_names ;
	string tb_save  ;
	string tb_rep   ;
	string tb_paths ;
	string tb_disp  ;

	tbSAVE t_save ;
	tbDISP t_disp ;
	tbREP  t_rep  ;

	tb_name = upper( word( s, 2 ) ) ;
	str     = subword( s, 3 ) ;

	tb_paths = parseString( rlt, str, "LIBRARY()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	iupper( str ) ;
	tb_keys  = parseString( rlt, str, "KEYS()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_names = parseString( rlt, str, "NAMES()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	t_save = WRITE ;
	t = parseString( rlt, str, "WRITE" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}
	if ( t == "" )
	{
		t = parseString( rlt, str, "NOWRITE" ) ;
		if ( !rlt )
		{
			err.seterrid( "PSYE019C" ) ;
			return ;
		}
		if ( t != "" ) { t_save = NOWRITE ; }
	}

	t_rep = NOREPLACE ;
	t = parseString( rlt, str, "REPLACE" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	if ( t != "" ) { t_rep = REPLACE ; }

	t_disp = EXCLUSIVE ;
	t = parseString( rlt, str, "SHARE" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}
	if ( t != "" ) { t_disp = SHARE ; }

	if ( words( str ) > 0 )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbcreate( tb_name, tb_keys, tb_names, t_save, t_rep, tb_paths, t_disp ) ;
	return ;
}


void execiTBDelete( pApplication * thisAppl, const string& s, errblock& err )
{
	if ( words( s ) != 2 )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbdelete( upper( word( s, 2 ) ) ) ;
	return ;
}


void execiTBDispl( pApplication * thisAppl, const string& s, errblock& err )
{
	int i_csrpos ;
	int i_csrrow ;

	bool   rlt ;

	string str  ;
	string tb_name ;
	string tb_pan  ;
	string tb_msg  ;
	string tb_cursor  ;
	string tb_csrrow  ;
	string tb_csrpos  ;
	string tb_autosel ;
	string tb_posn    ;
	string tb_rowid   ;

	str = upper( subword( s, 2 ) ) ;

	tb_pan = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_msg = parseString( rlt, str, "MSG()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_cursor = parseString( rlt, str, "CURSOR()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_csrrow = parseString( rlt, str, "CSRROW()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}
	if ( tb_csrrow == "" ) { i_csrrow = 1                ; }
	else                   { i_csrrow = ds2d( tb_csrrow ) ; }

	tb_csrpos = parseString( rlt, str, "CSRPOS()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}
	if ( tb_csrpos == "" ) { i_csrpos = 1                 ; }
	else                   { i_csrpos = ds2d( tb_csrpos ) ; }

	tb_autosel = parseString( rlt, str, "AUTOSEL()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_posn = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_rowid = parseString( rlt, str, "ROWID()" ) ;
	if ( !rlt || str != "" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbdispl( tb_name, tb_pan, tb_msg, tb_cursor, i_csrrow, i_csrpos, tb_autosel, tb_posn, tb_rowid ) ;
	return ;
}


void execiTBEnd( pApplication * thisAppl, const string& s, errblock& err )
{
	if ( words( s ) != 2 )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbend( upper( word( s, 2 ) ) ) ;
	return ;
}


void execiTBErase( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string str     ;
	string tb_name ;
	string tb_path ;

	tb_name = upper( word( s, 2 ) ) ;
	str     = subword( s, 3 ) ;

	tb_path = parseString( rlt, str, "LIBRARY()" ) ;
	if ( !rlt || str != "" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tberase( tb_name, tb_path ) ;
	return ;
}


void execiTBExist( pApplication * thisAppl, const string& s, errblock& err )
{
	if ( words( s ) != 2 )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbexist( upper( word( s, 2 ) ) ) ;
	return ;
}


void execiTBGet( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string str       ;
	string t         ;
	string tb_name   ;
	string tb_savenm ;
	string tb_crpnm  ;
	string tb_rowid  ;
	string tb_noread ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_savenm = parseString( rlt, str, "SAVENAME()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_rowid = parseString( rlt, str, "ROWID()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_crpnm = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_noread = str ;
	if ( tb_noread != "" && tb_noread != "NOREAD" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbget( tb_name, tb_savenm, tb_rowid, tb_noread, tb_crpnm ) ;
	return ;
}


void execiTBMod( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string str       ;
	string t         ;
	string tb_name   ;
	string tb_savenm ;
	string tb_order  ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_savenm = parseString( rlt, str, "SAVE()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_order = str ;
	if ( tb_order != "" && tb_order != "ORDER" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbmod( tb_name, tb_savenm, tb_order ) ;
	return ;
}


void execiTBPut( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string str       ;
	string t         ;
	string tb_name   ;
	string tb_savenm ;
	string tb_order  ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_savenm = parseString( rlt, str, "SAVE()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_order = str ;
	if ( tb_order != "" && tb_order != "ORDER" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbput( tb_name, tb_savenm, tb_order ) ;
	return ;
}


void execiTBOpen( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string str      ;
	string t        ;
	string tb_name  ;
	string tb_save  ;
	string tb_paths ;
	string tb_disp  ;

	tbSAVE t_save ;
	tbDISP t_disp ;

	tb_name = upper( word( s, 2 ) ) ;
	str     = subword( s, 3 ) ;

	t_save = WRITE ;
	t = parseString( rlt, str, "WRITE" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	if ( t == "" )
	{
		t = parseString( rlt, str, "NOWRITE" ) ;
		if ( !rlt )
		{
			err.seterrid( "PSYE019C" ) ;
			return ;
		}
		if ( t != "" ) { t_save = NOWRITE ; }
	}

	str      = subword( s, 4 ) ;
	tb_paths = parseString( rlt, str, "LIBRARY()" ) ;
	if ( !rlt || words( str ) > 1 )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_disp  = iupper( str ) ;
	if      ( tb_disp == "SHARE" ) { t_disp = SHARE     ; }
	else if ( tb_disp == ""      ) { t_disp = EXCLUSIVE ; }
	else
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbopen( tb_name, t_save, tb_paths, t_disp ) ;
	return ;
}


void execiTBQuery( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

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

	tb_keys = parseString( rlt, str, "KEYS()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_names = parseString( rlt, str, "NAMES()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_rownum = parseString( rlt, str, "ROWNUM()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_keynum = parseString( rlt, str, "KEYNUM()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_namenum = parseString( rlt, str, "NAMENUM()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_pos = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_srtflds = parseString( rlt, str, "SORTFLDS()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_sarglst = parseString( rlt, str, "SARGLIST()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_sargcnd = parseString( rlt, str, "SARGCOND()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_sargdir = parseString( rlt, str, "SARGDIR()" ) ;
	if ( !rlt || words( str ) > 0 )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbquery( tb_name, tb_keys, tb_names, tb_rownum, tb_keynum, tb_namenum, tb_pos, tb_srtflds, tb_sarglst, tb_sargcnd, tb_sargdir ) ;
	return ;
}


void execiTBSarg( pApplication * thisAppl, const string& s, errblock& err )
{
	bool rlt ;

	string str ;
	string tb_name  ;
	string tb_arglst  ;
	string tb_namecnd ;
	string tb_dir     ;

	tb_name  = upper( word( s, 2 ) )    ;
	str      = upper( subword( s, 3 ) ) ;

	tb_arglst = parseString( rlt, str, "ARGLIST()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_namecnd = parseString( rlt, str, "NAMECOND()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_dir = str ;
	if ( tb_dir == "" ) { tb_dir = "NEXT" ; }
	if ( tb_dir != "NEXT" && tb_dir != "PREVIOUS" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbsarg( tb_name, tb_arglst, tb_dir, tb_namecnd ) ;
	return ;
}


void execiTBSave( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string str      ;
	string tb_name  ;
	string tb_nname ;
	string tb_path  ;

	tb_name  = upper( word( s, 2 ) ) ;
	str      = subword( s, 3 ) ;

	tb_nname = upper( parseString( rlt, str, "NAME()" ) ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_path = parseString( rlt, str, "LIBRARY()" ) ;
	if ( !rlt || words( str ) > 0 )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbsave( tb_name, tb_nname, tb_path ) ;
	return ;
}


void execiTBScan( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt   ;

	string str ;
	string t   ;
	string tb_name    ;
	string tb_arglst  ;
	string tb_savenm  ;
	string tb_rowid   ;
	string tb_crpnm   ;
	string tb_condlst ;
	string tb_dir     ;
	string tb_noread  ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_arglst = parseString( rlt, str, "ARGLIST()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_savenm = parseString( rlt, str, "SAVENAME()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_rowid = parseString( rlt, str, "ROWID()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_crpnm = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_condlst = parseString( rlt, str, "CONDLIST()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_dir = "NEXT" ;
	t = parseString( rlt, str, "NEXT" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}
	if ( t == "" )
	{
		t = parseString( rlt, str, "PREVIOUS" ) ;
		if ( !rlt )
		{
			err.seterrid( "PSYE019C" ) ;
			return ;
		}
		if ( t != "" ) { tb_dir = "PREVIOUS" ; }
	}

	tb_noread = str ;
	if ( tb_noread != "" && tb_noread != "NOREAD" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbscan( tb_name, tb_arglst, tb_savenm, tb_rowid, tb_dir, tb_noread, tb_crpnm, tb_condlst ) ;
	return ;
}


void execiTBSkip( pApplication * thisAppl, const string& s, errblock& err )
{
	int i_num ;

	bool rlt  ;

	string str       ;
	string t         ;
	string tb_name   ;
	string tb_num    ;
	string tb_savenm ;
	string tb_crpnm  ;
	string tb_rowid  ;
	string tb_row    ;
	string tb_noread ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_num = parseString( rlt, str, "NUMBER()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}
	if ( tb_num == "" ) { i_num = 1                 ; }
	else                { i_num = ds2d( tb_num ) ; }

	tb_savenm = parseString( rlt, str, "SAVENAME()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_rowid = parseString( rlt, str, "ROWID()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_row = parseString( rlt, str, "ROW()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_crpnm = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	tb_noread = str ;
	if ( tb_noread != "" && tb_noread != "NOREAD" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbskip( tb_name, i_num, tb_savenm, tb_rowid, tb_row, tb_noread, tb_crpnm ) ;
	return ;
}


void execiTBSort( pApplication * thisAppl, const string& s, errblock& err )
{
	bool rlt ;

	string str     ;
	string tb_name ;
	string tb_flds ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_flds = parseString( rlt, str, "FIELDS()" ) ;
	if ( !rlt || str != "" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbsort( tb_name, tb_flds ) ;
	return ;
}


void execiTBTop( pApplication * thisAppl, const string& s, errblock& err )
{
	if ( words( s ) != 2 )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbtop( upper( word( s, 2 ) ) ) ;
	return ;
}


void execiTBVClear( pApplication * thisAppl, const string& s, errblock& err )
{
	if ( words( s ) != 2 )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->tbvclear( upper( word( s, 2 ) ) ) ;
	return ;
}


void execiVerase( pApplication * thisAppl, const string& s, errblock& err )
{
	bool rlt ;

	string str  ;
	string vars ;

	poolType pType ;

	str = upper( subword( s, 2 ) ) ;

	vars = parseString( rlt, str, "()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

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
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->verase( vars, pType ) ;
	return ;
}


void execiVget( pApplication * thisAppl, const string& s, errblock& err )
{
	// If this is called from the REXX interface module, VREPLACE first with nulls so a variable not found
	// results in a blank value, instead of the variable name.

	int i  ;
	int n  ;

	bool rlt ;

	string str  ;
	string vars ;
	string var  ;

	poolType pType ;

	str = upper( subword( s, 2 ) ) ;

	vars = parseString( rlt, str, "()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

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
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->vget( vars, pType )  ;
	return ;
}


void execiView( pApplication * thisAppl, const string& s, errblock& err )
{
	bool   rlt ;

	string str ;
	string pan ;
	string fl  ;

	str = subword( s, 2 ) ;

	fl = parseString( rlt, str, "FILE()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	pan = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt || str != "" )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->view( fl, iupper( pan ) ) ;
	return ;
}


void execiVput( pApplication * thisAppl, const string& s, errblock& err )
{
	bool rlt ;

	string str  ;
	string vars ;

	poolType pType ;

	str = upper( subword( s, 2 ) ) ;

	vars = parseString( rlt, str, "()" ) ;
	if ( !rlt )
	{
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

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
		err.seterrid( "PSYE019C" ) ;
		return ;
	}

	thisAppl->vput( vars, pType ) ;
	return ;
}
