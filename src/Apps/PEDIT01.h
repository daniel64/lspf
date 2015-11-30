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

using namespace std;


class icmd
{
	private:
		string icmd_COMMAND  ;
		int    icmd_sURID    ;
		int    icmd_Rpt      ;
		char   icmd_ABO      ;
		int    icmd_OSize    ;
		int    icmd_dURID    ;
		int    icmd_oURID    ;
		bool   icmd_ABO_req  ;
		bool   icmd_overlay  ;
		bool   icmd_cutpaste ;
		int    icmd_eURID    ;
		icmd()
		{
			icmd_COMMAND  = ""    ;
			icmd_sURID    = 0     ;
			icmd_ABO      = ' '   ;
			icmd_OSize    = 0     ;
			icmd_Rpt      = 0     ;
			icmd_dURID    = 0     ;
			icmd_oURID    = 0     ;
			icmd_ABO_req  = false ;
			icmd_overlay  = false ;
			icmd_cutpaste = false ;
			icmd_eURID    = 0     ;
		}
		void icmd_clear()
		{
			icmd_COMMAND  = ""    ;
			icmd_sURID    = 0     ;
			icmd_ABO      = ' '   ;
			icmd_OSize    = 0     ;
			icmd_Rpt      = 0     ;
			icmd_dURID    = 0     ;
			icmd_oURID    = 0     ;
			icmd_ABO_req  = false ;
			icmd_overlay  = false ;
			icmd_cutpaste = false ;
			icmd_eURID    = 0     ;
		}
	friend class PEDIT01 ;
} ;


class idata
{
	private:
		int    id_lvl  ;
		string id_data ;
		idata()
		{
			id_lvl    = 0  ;
			id_data   = "" ;
		}
	friend class iline   ;
	friend class PEDIT01 ;
} ;


class iposition
{
	public:
		int    ipo_line ;
		int    ipo_URID ;
		iposition()
		{
			ipo_line = 0 ;
			ipo_URID = 0 ;
		}
	friend class PEDIT01 ;
} ;



class ichange
{
	private:
		int    iURID   ;
		int    ilvl    ;
		char   iaction ;
		ichange()
		{
			iURID   = 0   ;
			ilvl    = 0   ;
			iaction = ' ' ;
		}

	friend class iline   ;
	friend class PEDIT01 ;
} ;


class ipline
{
	private:
		bool   ip_file ;
		bool   ip_note ;
		bool   ip_prof ;
		bool   ip_col  ;
		bool   ip_bnds ;
		bool   ip_excl ;
		bool   ip_hex  ;
		string ip_data ;
		ipline()
		{
			ip_file = false ;
			ip_note = false ;
			ip_prof = false ;
			ip_col  = false ;
			ip_bnds = false ;
			ip_excl = false ;
			ip_hex  = false ;
			ip_data = ""    ;
		}
		
	friend class PEDIT01 ;
} ;


class iline
{
	private:
		static stack<ichange> Global_Undo ;
		static stack<ichange> Global_Redo ;
		static int  maxURID        ;
		static bool recoverOFF     ;

		bool   il_file    ;
		bool   il_note    ;
		bool   il_prof    ;
		bool   il_col     ;
		bool   il_bnds    ;
		bool   il_excl    ;
		bool   il_tod     ;
		bool   il_bod     ;
		bool   il_hex     ;
		bool   il_chg     ;
		bool   il_error   ;
		bool   il_deleted ;
		bool   il_newisrt ;
		string il_label   ;
		string il_lc1     ;
		string il_lc2     ;
		int    il_rept    ;
		int    il_URID    ;
		stack <idata> il_idata      ;
		stack <idata> il_idata_redo ;

		iline()
		{
			il_file    = false ;
			il_note    = false ;
			il_prof    = false ;
			il_col     = false ;
			il_bnds    = false ;
			il_excl    = false ;
			il_tod     = false ;
			il_bod     = false ;
			il_hex     = false ;
			il_chg     = false ;
			il_error   = false ;
			il_deleted = false ;
			il_newisrt = false ;
			il_label   = ""    ;
			il_lc1     = ""    ;
			il_lc2     = ""    ;
			il_rept    = 0     ;
			il_URID    = ++maxURID ;
		}
		void clearLc12()
		{
			il_lc1 = "" ;
			il_lc2 = "" ;
		}
		void put_idata( string s, int lvl )
		{
			idata d   ;
			ichange t ;

			if ( recoverOFF )
			{
				d.id_data = s      ;
				if   ( il_idata.empty() ) { il_idata.push( d ) ; }
				else { il_idata.top() = d ; }
			}
			else
			{
				d.id_lvl = lvl     ;
				d.id_data = s      ;
				il_idata.push( d ) ;
				il_deleted = false ;
				t.iURID = il_URID  ;
				t.ilvl  = lvl      ;
				Global_Undo.push( t ) ;
			}
			clear_Global_Redo() ;
		}
		void set_il_deleted()
		{
			il_deleted = true ;
			if ( !recoverOFF ) { Global_Undo.top().iaction = 'D' ; }
		}
		string get_idata()
		{
			return il_idata.top().id_data ;
		}
		void undo_idata()
		{
			if ( recoverOFF ) { return ; }
			if ( !il_idata.empty() )
			{
				if ( Global_Undo.top().iaction == 'D' )
				{
					il_deleted = false ;
				}
				il_idata_redo.push( il_idata.top() ) ;
				il_idata.pop()                       ;
				if ( il_idata.empty() )
				{
					il_deleted = true ;
				}
			}
			Global_Redo.push ( Global_Undo.top() ) ;
			Global_Undo.pop() ;
		}
		void redo_idata()
		{
			if ( recoverOFF ) { return ; }
			if ( Global_Redo.top().iaction == 'D' )
			{
				il_deleted = true ;
			}
			else
			{
				il_deleted = false  ;
			}
			il_idata.push( il_idata_redo.top() ) ;
			il_idata_redo.pop() ;
			Global_Undo.push ( Global_Redo.top() ) ;
			Global_Redo.pop() ;
		}
//		void clear_redo_idata()
//		{
//			while ( true )
//			{
//				if ( il_idata_redo.empty() ) { break ; }
//				il_idata_redo.pop() ;
//			}
//		}
		void clear_Global_Undo()
		{
			while ( true )
			{
				if ( Global_Undo.empty() ) { break ; }
				Global_Undo.pop() ;
			}
		}
		void clear_Global_Redo()
		{
			while ( true )
			{
				if ( Global_Redo.empty() ) { break ; }
				Global_Redo.pop() ;
			}
		}
		ichange get_Undo_URID()
		{
			ichange i ;
			if ( recoverOFF || Global_Undo.empty() ) { return i ; }
			return Global_Undo.top() ;
		}
		ichange get_Redo_URID()
		{
			ichange i ;
			if ( recoverOFF ) { return i ; }
			if ( Global_Redo.empty() ) { return i ; }
			return Global_Redo.top() ;
		}
		void flatten_idata()
		{
			idata  d  ;

			d.id_data = il_idata.top().id_data ;
			d.id_lvl  = il_idata.top().id_lvl  ;
			while ( true )
			{
				if ( il_idata.empty() ) { break ; }
				il_idata.pop() ;
			}
			while ( true )
			{
				if ( il_idata_redo.empty() ) { break ; }
				il_idata_redo.pop() ;
			}
			il_idata.push( d ) ;
		}
		int get_idata_lvl()
		{
			return il_idata.top().id_lvl ;
		}
		int get_Global_Undo_Size()
		{
			return Global_Undo.size() ;
		}
		int get_Global_Redo_Size()
		{
			return Global_Redo.size() ;
		}

	friend class PEDIT01 ;
} ;

class e_find
{
	private:
		string fcx_string  ;
		string fcx_cstring ;
		bool   fcx_regreq  ;
		bool   fcx_text    ;
		bool   fcx_asis    ;
		bool   fcx_hex     ;
		bool   fcx_pic     ;
		bool   fcx_rreg    ;
		string fcx_slab    ;
		string fcx_elab    ;
		int    fcx_scol    ;
		int    fcx_ecol    ;
		bool   fcx_change  ;
		bool   fcx_fset    ;
		bool   fcx_cset    ;
		char   fcx_dir     ;
		char   fcx_mtch    ;
		char   fcx_prevcmd ;
	e_find()
	{
		fcx_string  = ""    ;
		fcx_cstring = ""    ;
		fcx_regreq  = false ;
		fcx_text    = false ;
		fcx_asis    = false ;
		fcx_hex     = false ;
		fcx_pic     = false ;
		fcx_rreg    = false ;
		fcx_slab    = ""    ;
		fcx_elab    = ""    ;
		fcx_scol    = 0     ;
		fcx_ecol    = 0     ;
		fcx_fset    = false ;
		fcx_cset    = false ;
		fcx_dir     = 'N'   ;
		fcx_mtch    = ' '   ;
		fcx_prevcmd = ' '   ;
	}
	friend class PEDIT01 ;
} ;


class PEDIT01 : public pApplication
{
	public:
		void application() ;

	private:
		void showEditEntry()      ;
		void Edit()               ;
		void cleanup_custom()     ;
		void initialise()         ;
		bool termOK()             ;
		void read_file()          ;
		bool saveFile()           ;
		void fill_dynamic_area()  ;
		void getZAREAchanges()    ;
		void updateData()         ;

		bool checkLineCommands()  ;
		void actionLineCommands() ;
		void actionPCMD()         ;

		void actionUNDO()         ;
		void actionREDO()         ;
		void removeRecoveryData() ;
		
		int  getLastEX( int )     ;
		int  getEXBlock( int )    ;

		int  getFileLine( int )   ;
		int  getDataLine( int )   ;

		string overlay( string, string, bool & ) ;
		bool xformLineCmd( string, string &, int & ) ;
		int  getRangeSize( int, int ) ;
		int  truncateSize( int ) ;
		
		void copyToClipboard( vector<ipline> & vip ) ;
		void clearClipboard( string )  ;
		void getClipboard( vector<ipline> & vip ) ;

		vector<iline * >::iterator getLineItr( int ) ;
		vector<iline * >::iterator getLineBeforeItr( int ) ;

		int  setFindChangeExcl( char ) ;
		bool returnLabelItr( string, vector<iline * >::iterator & , int & ) ;

		int topLine              ;
		int startCol             ;
		int maxCol               ;
		int aRow                 ;
		int aCol                 ;
		int Level                ;

		bool cursorPlaced        ;
		int  cursorPlaceType     ;
		int  cursorPlaceURID     ;
		int  cursorPlacePos      ;
		int  cursorPlaceChar     ;

		void clearCursor()       ;
		void placeCursor( int, int, int=0 ) ;
		void positionCursor()    ;

		bool profSave            ;
		bool profNulls           ;
		bool profLock            ;
		bool profCaps            ;
		bool profHex             ;
		bool profTabs            ;
		int  profTabSz           ;

		bool stripST             ;
		bool convTabs            ;
		bool convSpaces          ;
		bool cutActive           ;
		bool cutReplace          ;
		bool pasteActive         ;
		bool pasteKeep           ;

		vector<iline *> data     ;
		map<int, iposition> s2data ;
		map<int, bool> sChanged  ;
		map<int, bool> sTouched  ;
		vector<icmd> icmds       ;
		
		e_find find_parms  ;

		bool rebuildZAREA  ;
		bool fileChanged   ;

		string CURFLD  ;
		int    CURPOS  ;
		string MSG     ;
		string MSG1    ;
		string ZCMD1   ;
		string OCMD    ;

		string ZFILE   ;
		string ZROW1   ;
		string ZROW2   ;
		string ZCOL1   ;
		string ZAREA   ;
		string ZSHADOW ;
		string ZAREAT  ;
		int    ZAREAW  ;
		int    ZAREAD  ;
		int    ZDATAW  ;
		int    ZASIZE  ;
		string CAREA   ;
		string ZLINES  ;

		string ZEDSAVE  ;
		string ZEDNULLS ;
		string ZEDLOCK  ;
		string ZEDCAPS  ;
		string ZEDHEX   ;
		string ZEDTABS  ;
		string ZEDTABSZ ;
		string ZEDRECOV ;

		string fileType  ;
		string clipboard ;
		string CLIPTABL  ;

		string sdg ;
		string sdy ;
		string sdw ;
		string sdr ;
		string div ;
		string slg ;
		string sly ;
		string slw ;
		string slr ;

		const string blkcmds  = "CC MM DD HXX OO RR XX (( )) UCC LCC MMD" ;
		const string sglcmds  = "A B BNDS C COL COLS D F HX I L LC M MD O R S UC X ( )" ;
		const string spllcmds = "COL COLS A B I C M D R CC MM DD RR" ;
		const string todlcmds = "COL COLS A I BNDS" ;
		const string bodlcmds = "B" ;
		const string ABOReq   = "CC MM C M" ;
		const string ABOList  = "A B O" ;
		const string ReptOK   = "C M D HX MD X O R UC LC RR (( )) ( ) F L" ;

} ;

