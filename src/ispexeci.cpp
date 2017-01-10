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

/* ispexeci - ISPEXEC interface module             */
/*                                                 */

int execiAddpop( pApplication *, const string& )   ;
int execiBrowse( pApplication *, const string& )   ;
int execiDisplay( pApplication *, const string& )  ;
int execiControl( pApplication *, const string& )  ;
int execiEdit( pApplication *, const string& )     ;
int execiGetmsg( pApplication *, const string& )   ;
int execiLibdef( pApplication *, const string& )   ;
int execiPquery( pApplication *, const string& )   ;
int execiRDisplay( pApplication *, const string& ) ;
int execiRempop( pApplication *, const string& )   ;
int execiSelect( pApplication *, const string& )   ;
int execiSetmsg( pApplication *, const string& )   ;
int execiTBAdd( pApplication *, const string& )    ;
int execiTBBottom( pApplication *, const string& ) ;
int execiTBCreate( pApplication *, const string& ) ;
int execiTBClose( pApplication *, const string& )  ;
int execiTBDelete( pApplication *, const string& ) ;
int execiTBDispl( pApplication *, const string& )  ;
int execiTBEnd( pApplication *, const string& )    ;
int execiTBErase( pApplication *, const string& )  ;
int execiTBExist( pApplication *, const string& )  ;
int execiTBGet( pApplication *, const string& )    ;
int execiTBMod( pApplication *, const string& )    ;
int execiTBPut( pApplication *, const string& )    ;
int execiTBOpen( pApplication *, const string& )   ;
int execiTBQuery( pApplication *, const string& )  ;
int execiTBSarg( pApplication *, const string& )   ;
int execiTBSave( pApplication *, const string& )   ;
int execiTBScan( pApplication *, const string& )   ;
int execiTBSkip( pApplication *, const string& )   ;
int execiTBSort( pApplication *, const string& )   ;
int execiTBTop( pApplication *, const string& )    ;
int execiTBVClear( pApplication *, const string& ) ;
int execiVerase( pApplication *, const string& )   ;
int execiVget( pApplication *, const string& )     ;
int execiView( pApplication *, const string& )     ;
int execiVput( pApplication *, const string& )     ;

void execiSyntaxError( pApplication *, const string& ) ;

map<string, int(*)(pApplication *,const string&)> execiServices = {
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


int ispexeci( pApplication * thisAppl, const string& s )
{
	// Call sub_vars to resolve all dialogue variables in string 's' first
	// Then parse string to call the correct pApplication method for the service

	// Except for keywords relating to paths and files, the interface is case insensitive

	string s1 ;
	string w1 ;

	const string e1( " service invalid with this interface" ) ;
	const string e2( " not a recognised service name" ) ;
	const string InvalidServices = "VDEFINE VDELETE VCOPY VREPLACE VRESET" ;

	map<string, int(*)(pApplication *,const string&)>::iterator it ;

	s1 = thisAppl->sub_vars( s ) ;
	w1 = upper( word( s1, 1 ) )  ;

	if ( findword( w1, InvalidServices ) )
	{
		log( "E", w1 + e1 <<endl) ;
		thisAppl->RC = 20 ;
		thisAppl->checkRCode( w1 + e1 ) ;
		return 20 ;
	}

	it = execiServices.find( w1 ) ;
	if ( it == execiServices.end() )
	{
		log( "E", w1 + e2 <<endl ) ;
		thisAppl->RC = 20 ;
		thisAppl->checkRCode( w1 + e2 ) ;
		return 20 ;
	}

	return it->second( thisAppl, s1 ) ;
}


int execiAddpop( pApplication * thisAppl, const string& s )
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
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	ap_row = parseString( rlt, str, "ROW()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( ap_row == "" ) { i_row = 0              ; }
	else                { i_row = ds2d( ap_row ) ; }

	ap_col = parseString( rlt, str, "COLUMN()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( ap_col == "" ) { i_col = 0              ; }
	else                { i_col = ds2d( ap_col ) ; }

	if ( str != "" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->addpop( ap_loc, i_row, i_col ) ;
	return thisAppl->RC ;
}


int execiBrowse( pApplication * thisAppl, const string& s )
{
	bool   rlt ;

	string str ;
	string pan ;
	string fl  ;

	str = subword( s, 2 ) ;

	fl  = parseString( rlt, str, "FILE()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	pan = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( str != "" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->browse( fl, iupper( pan ) ) ;
	return thisAppl->RC ;
}


int execiControl( pApplication * thisAppl, const string& s )
{
	string s1 ;

	s1 = upper( s ) ;
	thisAppl->control( word( s1, 2 ), word( s1, 3 ), subword( s1, 4 ) ) ;
	return thisAppl->RC ;
}


int execiDisplay( pApplication * thisAppl, const string& s )
{
	bool   rlt ;

	string str ;
	string pan ;
	string msg ;
	string cursor ;
	string csrpos ;

	str = upper( subword( s, 2 ) ) ;

	pan = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	msg = parseString( rlt, str, "MSG()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	cursor = parseString( rlt, str, "CURSOR()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	csrpos = parseString( rlt, str, "CSRPOS()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( str != "" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( csrpos == "" ) { csrpos = "1" ; }

	thisAppl->display( pan, msg, cursor, ds2d( csrpos ) ) ;
	return thisAppl->RC ;
}


int execiEdit( pApplication * thisAppl, const string& s )
{
	bool   rlt ;

	string str ;
	string pan ;
	string fl  ;

	str = subword( s, 2 ) ;

	fl  = parseString( rlt, str, "FILE()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	pan = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( str != "" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->edit( fl, iupper( pan ) ) ;
	return thisAppl->RC ;
}


int execiGetmsg( pApplication * thisAppl, const string& s )
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

	msg  = parseString( rlt, str, "MSG()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	smsg = parseString( rlt, str, "SHORTMSG()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	lmsg = parseString( rlt, str, "LONGMSG()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	alm  = parseString( rlt, str, "ALARM()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	hlp  = parseString( rlt, str, "HELP()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	typ  = parseString( rlt, str, "TYPE()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	wndo = parseString( rlt, str, "WINDOW()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( str != "" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->getmsg( msg, smsg, lmsg, alm, hlp, typ, wndo ) ;
	return thisAppl->RC ;
}


int execiLibdef( pApplication * thisAppl, const string& s )
{
	bool   rlt ;

	string ld_files ;
	string str      ;

	str      = subword( s, 3 ) ;
	ld_files = parseString( rlt, str, "ID()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( str != "" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->libdef( word( s, 2 ), ld_files ) ;
	return thisAppl->RC ;
}


int execiPquery( pApplication * thisAppl, const string& s )
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
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	pq_arean = parseString( rlt, str, "AREANAME()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	pq_areat = parseString( rlt, str, "AREATYPE()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	pq_width = parseString( rlt, str, "WIDTH()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	pq_depth = parseString( rlt, str, "DEPTH()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	pq_row   = parseString( rlt, str, "ROW()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	pq_col   = parseString( rlt, str, "COLUMN()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( str != "" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->pquery( pq_panel, pq_arean, pq_areat, pq_width, pq_depth, pq_row, pq_col ) ;
	return thisAppl->RC ;
}


int execiRDisplay( pApplication * thisAppl, const string& s )
{
	// Call rdisplay(s,false) which does not do dialogue variable substitution as this has
	// already been done.

	thisAppl->rdisplay( substr( s, 10 ), false ) ;
	return thisAppl->RC ;
}


int execiRempop( pApplication * thisAppl, const string& s )
{
	string ap_all ;

	ap_all = upper( subword( s, 2 ) ) ;

	if ( ap_all != "" && ap_all != "ALL" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->rempop( ap_all ) ;
	return thisAppl->RC ;
}


int execiSelect( pApplication * thisAppl, const string& s )
{
	// The SELECT parser may replace PGM with a variable name (eg. for a REXX command), so
	// substitute with its dialogue variable value

	selobj SEL ;

	if ( !SEL.parse( subword( s, 2 ) ) )
	{
		execiSyntaxError( thisAppl, s ) ;
		return 20 ;
	}

	if ( SEL.PGM[ 0 ] == '&' )
	{
		thisAppl->vcopy( SEL.PGM.erase( 0, 1 ), SEL.PGM, MOVE ) ;
	}

	thisAppl->select( SEL ) ;
	return thisAppl->RC ;
}


int execiSetmsg( pApplication * thisAppl, const string& s )
{
	bool   rlt ;

	string str   ;
	string s_msg ;

	msgSET t ;

	str = upper( subword( s, 2 ) ) ;

	s_msg  = parseString( rlt, str, "MSG()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	if      ( str == "COND" ) { t = COND   ; }
	else if ( str == "" )     { t = UNCOND ; }
	else    { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->setmsg( s_msg, t ) ;
	return thisAppl->RC ;
}


int execiTBAdd( pApplication * thisAppl, const string& s )
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
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_numrows = parseString( rlt, str, "MULT()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( tb_numrows == "" ) { tb_numrows = "1" ; }

	tb_order = str ;
	if ( tb_order != "" && tb_order != "ORDER" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbadd( tb_name, tb_namelst, tb_order, ds2d( tb_numrows ) ) ;
	return thisAppl->RC ;
}


int execiTBBottom( pApplication * thisAppl, const string& s )
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

	tb_savenm  = parseString( rlt, str, "SAVENAME()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_rowid   = parseString( rlt, str, "ROWID()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_crpnm   = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_noread = str ;
	if ( tb_noread != "" && tb_noread != "NOREAD" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbbottom( tb_name, tb_savenm, tb_rowid, tb_noread, tb_crpnm ) ;
	return thisAppl->RC ;
}


int execiTBClose( pApplication * thisAppl, const string& s )
{
	bool   rlt ;

	string str      ;
	string tb_name  ;
	string tb_nname ;
	string tb_path  ;

	tb_name  = upper( word( s, 2 ) ) ;
	str      = subword( s, 3 )       ;

	tb_nname = parseString( rlt, str, "NAME()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_path  = parseString( rlt, str, "LIBRARY()" ) ;
	if ( !rlt || words( str ) > 0 ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbclose( tb_name, tb_nname, tb_path ) ;
	return thisAppl->RC ;
}


int execiTBCreate( pApplication * thisAppl, const string& s )
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
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	iupper( str ) ;
	tb_keys  = parseString( rlt, str, "KEYS()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_names = parseString( rlt, str, "NAMES()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	t_save = WRITE ;
	t = parseString( rlt, str, "WRITE" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( t == "" )
	{
		t = parseString( rlt, str, "NOWRITE" ) ;
		if ( !rlt )    { execiSyntaxError( thisAppl, s ) ; return 20 ; }
		if ( t != "" ) { t_save = NOWRITE ; }
	}

	t_rep = NOREPLACE ;
	t = parseString( rlt, str, "REPLACE" ) ;
	if ( !rlt )    { execiSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( t != "" ) { t_rep = REPLACE ; }

	t_disp = EXCLUSIVE ;
	t = parseString( rlt, str, "SHARE" ) ;
	if ( !rlt )    { execiSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( t != "" ) { t_disp = SHARE ; }

	if ( words( str ) > 0 ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbcreate( tb_name, tb_keys, tb_names, t_save, t_rep, tb_paths, t_disp ) ;
	return thisAppl->RC ;
}


int execiTBDelete( pApplication * thisAppl, const string& s )
{
	if ( words( s ) != 2 ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbdelete( upper( word( s, 2 ) ) ) ;
	return thisAppl->RC ;
}


int execiTBDispl( pApplication * thisAppl, const string& s )
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

	tb_pan     = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_msg     = parseString( rlt, str, "MSG()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_cursor  = parseString( rlt, str, "CURSOR()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_csrrow  = parseString( rlt, str, "CSRROW()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( tb_csrrow == "" ) { i_csrrow = 1                ; }
	else                   { i_csrrow = ds2d( tb_csrrow ) ; }

	tb_csrpos  = parseString( rlt, str, "CSRPOS()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( tb_csrpos == "" ) { i_csrpos = 1                 ; }
	else                   { i_csrpos = ds2d( tb_csrpos ) ; }

	tb_autosel = parseString( rlt, str, "AUTOSEL()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_posn    = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_rowid   = parseString( rlt, str, "ROWID()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( str != "" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbdispl( tb_name, tb_pan, tb_msg, tb_cursor, i_csrrow, i_csrpos, tb_autosel, tb_posn, tb_rowid ) ;
	return thisAppl->RC ;
}


int execiTBEnd( pApplication * thisAppl, const string& s )
{
	if ( words( s ) != 2 ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbend( upper( word( s, 2 ) ) ) ;
	return thisAppl->RC ;
}


int execiTBErase( pApplication * thisAppl, const string& s )
{
	bool   rlt ;

	string str     ;
	string tb_name ;
	string tb_path ;

	tb_name = upper( word( s, 2 ) ) ;
	str     = subword( s, 3 ) ;

	tb_path = parseString( rlt, str, "LIBRARY()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( str != "" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tberase( tb_name, tb_path ) ;
	return thisAppl->RC ;
}


int execiTBExist( pApplication * thisAppl, const string& s )
{
	if ( words( s ) != 2 ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbexist( upper( word( s, 2 ) ) ) ;
	return thisAppl->RC ;
}


int execiTBGet( pApplication * thisAppl, const string& s )
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

	tb_savenm  = parseString( rlt, str, "SAVENAME()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_rowid   = parseString( rlt, str, "ROWID()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_crpnm   = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_noread = str ;
	if ( tb_noread != "" && tb_noread != "NOREAD" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbget( tb_name, tb_savenm, tb_rowid, tb_noread, tb_crpnm ) ;
	return thisAppl->RC ;
}


int execiTBMod( pApplication * thisAppl, const string& s )
{
	bool   rlt ;

	string str       ;
	string t         ;
	string tb_name   ;
	string tb_savenm ;
	string tb_order  ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_savenm  = parseString( rlt, str, "SAVE()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_order = str ;
	if ( tb_order != "" && tb_order != "ORDER" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbmod( tb_name, tb_savenm, tb_order ) ;
	return thisAppl->RC ;
}


int execiTBPut( pApplication * thisAppl, const string& s )
{
	bool   rlt ;

	string str       ;
	string t         ;
	string tb_name   ;
	string tb_savenm ;
	string tb_order  ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_savenm  = parseString( rlt, str, "SAVE()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_order = str ;
	if ( tb_order != "" && tb_order != "ORDER" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbput( tb_name, tb_savenm, tb_order ) ;
	return thisAppl->RC ;
}


int execiTBOpen( pApplication * thisAppl, const string& s )
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
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( t == "" )
	{
		t = parseString( rlt, str, "NOWRITE" ) ;
		if ( !rlt )    { execiSyntaxError( thisAppl, s ) ; return 20 ; }
		if ( t != "" ) { t_save = NOWRITE ; }
	}

	str      = subword( s, 4 ) ;
	tb_paths = parseString( rlt, str, "LIBRARY()" ) ;
	if ( !rlt || words( str ) > 1 ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_disp  = iupper( str ) ;
	if      ( tb_disp == "SHARE" ) { t_disp = SHARE     ; }
	else if ( tb_disp == ""      ) { t_disp = EXCLUSIVE ; }
	else                           { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbopen( tb_name, t_save, tb_paths, t_disp ) ;
	return thisAppl->RC ;
}


int execiTBQuery( pApplication * thisAppl, const string& s )
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

	tb_name  = upper( word( s, 2 ) )    ;
	str      = upper( subword( s, 3 ) ) ;

	tb_keys  = parseString( rlt, str, "KEYS()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_names   = parseString( rlt, str, "NAMES()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_rownum  = parseString( rlt, str, "ROWNUM()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_keynum  = parseString( rlt, str, "KEYNUM()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_namenum = parseString( rlt, str, "NAMENUM()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_pos     = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_srtflds = parseString( rlt, str, "SORTFLDS()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_sarglst = parseString( rlt, str, "SARGLIST()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_sargcnd = parseString( rlt, str, "SARGCOND()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_sargdir = parseString( rlt, str, "SARGDIR()" ) ;
	if ( !rlt || words( str ) > 0 ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbquery( tb_name, tb_keys, tb_names, tb_rownum, tb_keynum, tb_namenum, tb_pos, tb_srtflds, tb_sarglst, tb_sargcnd, tb_sargdir ) ;
	return thisAppl->RC ;
}


int execiTBSarg( pApplication * thisAppl, const string& s )
{
	bool   rlt ;

	string str      ;
	string tb_name  ;
	string tb_arglst  ;
	string tb_namecnd ;
	string tb_dir     ;

	tb_name  = upper( word( s, 2 ) )    ;
	str      = upper( subword( s, 3 ) ) ;

	tb_arglst  = parseString( rlt, str, "ARGLIST()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_namecnd = parseString( rlt, str, "NAMECOND()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_dir = str ;
	if ( tb_dir == "" ) { tb_dir = "NEXT" ; }
	if ( tb_dir != "NEXT" && tb_dir != "PREVIOUS" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbsarg( tb_name, tb_arglst, tb_dir, tb_namecnd ) ;
	return thisAppl->RC ;
}


int execiTBSave( pApplication * thisAppl, const string& s )
{
	bool   rlt ;

	string str      ;
	string tb_name  ;
	string tb_nname ;
	string tb_path  ;

	tb_name  = upper( word( s, 2 ) ) ;
	str      = subword( s, 3 ) ;

	tb_nname = upper( parseString( rlt, str, "NAME()" ) ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_path  = parseString( rlt, str, "LIBRARY()" ) ;
	if ( !rlt || words( str ) > 0 ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbsave( tb_name, tb_nname, tb_path ) ;
	return thisAppl->RC ;
}


int execiTBScan( pApplication * thisAppl, const string& s )
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
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_savenm  = parseString( rlt, str, "SAVENAME()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_rowid   = parseString( rlt, str, "ROWID()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_crpnm   = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_condlst = parseString( rlt, str, "CONDLIST()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_dir = "NEXT" ;
	t = parseString( rlt, str, "NEXT" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( t == "" )
	{
		t = parseString( rlt, str, "PREVIOUS" ) ;
		if ( !rlt )    { execiSyntaxError( thisAppl, s ) ; return 20 ; }
		if ( t != "" ) { tb_dir = "PREVIOUS" ; }
	}

	tb_noread = str ;
	if ( tb_noread != "" && tb_noread != "NOREAD" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbscan( tb_name, tb_arglst, tb_savenm, tb_rowid, tb_dir, tb_noread, tb_crpnm, tb_condlst ) ;
	return thisAppl->RC ;
}


int execiTBSkip( pApplication * thisAppl, const string& s )
{
	int    i_num ;

	bool   rlt   ;

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

	tb_num  = parseString( rlt, str, "NUMBER()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }
	if ( tb_num == "" ) { i_num = 1                 ; }
	else                { i_num = ds2d( tb_num ) ; }

	tb_savenm  = parseString( rlt, str, "SAVENAME()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_rowid   = parseString( rlt, str, "ROWID()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_row     = parseString( rlt, str, "ROW()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_crpnm   = parseString( rlt, str, "POSITION()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	tb_noread = str ;
	if ( tb_noread != "" && tb_noread != "NOREAD" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbskip( tb_name, i_num, tb_savenm, tb_rowid, tb_row, tb_noread, tb_crpnm ) ;
	return thisAppl->RC ;
}


int execiTBSort( pApplication * thisAppl, const string& s )
{
	bool rlt ;

	string str     ;
	string tb_name ;
	string tb_flds ;

	tb_name = upper( word( s, 2 ) )    ;
	str     = upper( subword( s, 3 ) ) ;

	tb_flds = parseString( rlt, str, "FIELDS()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( str != "" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbsort( tb_name, tb_flds ) ;
	return thisAppl->RC ;
}


int execiTBTop( pApplication * thisAppl, const string& s )
{
	if ( words( s ) != 2 ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbtop( upper( word( s, 2 ) ) ) ;
	return thisAppl->RC ;
}


int execiTBVClear( pApplication * thisAppl, const string& s )
{
	if ( words( s ) != 2 ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->tbvclear( upper( word( s, 2 ) ) ) ;
	return thisAppl->RC ;
}


int execiVerase( pApplication * thisAppl, const string& s )
{
	bool rlt ;

	string str  ;
	string vars ;

	poolType pType ;

	str = upper( subword( s, 2 ) ) ;

	vars = parseString( rlt, str, "()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

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
	else                         { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->verase( vars, pType ) ;
	return thisAppl->RC ;
}


int execiVget( pApplication * thisAppl, const string& s )
{
	// If this is called from the REXX interface module, VREPLACE first with nulls so a variable not found
	// results in a blank value, instead of the variable name.

	int i  ;
	int n  ;

	bool rlt ;

	string str  ;
	string vars ;

	poolType pType ;

	str = upper( subword( s, 2 ) ) ;

	vars = parseString( rlt, str, "()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

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
			thisAppl->vreplace( word( vars, i ), "" ) ;
		}
	}

	if      ( str == "SHARED"  ) { pType = SHARED  ; }
	else if ( str == "PROFILE" ) { pType = PROFILE ; }
	else if ( str == "ASIS"    ) { pType = ASIS    ; }
	else if ( str == ""        ) { pType = ASIS    ; }
	else                         { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->vget( vars, pType ) ;
	return thisAppl->RC ;
}


int execiView( pApplication * thisAppl, const string& s )
{
	bool   rlt ;

	string str ;
	string pan ;
	string fl  ;

	str = subword( s, 2 ) ;

	fl  = parseString( rlt, str, "FILE()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	pan = parseString( rlt, str, "PANEL()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	if ( str != "" ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->view( fl, iupper( pan ) ) ;
	return thisAppl->RC ;
}


int execiVput( pApplication * thisAppl, const string& s )
{
	bool rlt ;

	string str  ;
	string vars ;

	poolType pType ;

	str = upper( subword( s, 2 ) ) ;

	vars = parseString( rlt, str, "()" ) ;
	if ( !rlt ) { execiSyntaxError( thisAppl, s ) ; return 20 ; }

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
	else                         { execiSyntaxError( thisAppl, s ) ; return 20 ; }

	thisAppl->vput( vars, pType ) ;
	return thisAppl->RC ;
}


void execiSyntaxError( pApplication * thisAppl, const string& s )
{
	thisAppl->RC = 20 ;
	thisAppl->checkRCode( "Syntax error in service: "+s ) ;
}

