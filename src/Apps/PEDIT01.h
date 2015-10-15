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

class iline
{
	private:
		bool iline_file    ;
		bool iline_note    ;
		bool iline_prof    ;
		bool iline_col     ;
		bool iline_excl    ;
		bool iline_tod     ;
		bool iline_bod     ;
		bool iline_hex     ;
		bool iline_chg     ;
		bool iline_error   ;
		bool iline_hidden  ;
		bool iline_newisrt ;
		bool iline_deleted ;
		int  iline_lvl     ;
		int  iline_lvlp    ;
		int  iline_lvln    ;
		string iline_label ;
		string iline_lcc   ;
		string iline_data  ;

		iline()
		{
			iline_file    = false ;
			iline_note    = false ;
			iline_prof    = false ;
			iline_col     = false ;
			iline_excl    = false ;
			iline_tod     = false ;
			iline_bod     = false ;
			iline_hex     = false ;
			iline_chg     = false ;
			iline_error   = false ;
			iline_hidden  = false ;
			iline_newisrt = false ;
			iline_deleted = false ;
			iline_lvl     = 0     ;
			iline_lvlp    = -1    ;
			iline_lvln    = -1    ;
			iline_label   = ""    ;
			iline_lcc     = ""    ;
			iline_data    = ""    ;
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
		void initialise()        ;
		bool termOK()            ;
		void read_file()         ;
		bool saveFile()          ;
		void fill_dynamic_area() ;
		void getZAREAchanges()   ;
		void updateData()        ;
		void actionSLCMDS()      ;
		void actionBLCMDS()      ;
		void actionPCMD()        ;
		void actionUNDO()        ;
		void actionREDO()        ;
		bool REDOavailable       ;

		int  setFindChangeExcl( char ) ;
		bool returnLabelItr( string, vector<iline * >::iterator & , int & ) ;

		int firstLine            ;
		int maxLines             ;
		int startCol             ;
		int maxCol               ;
		int aRow                 ;
		int aCol                 ;
		int undoLevel            ;

		bool profSave            ;
		bool profNulls           ;
		bool profLock            ;
		bool profCaps            ;
		bool profHex             ;		

		vector<iline *> data     ;
		map<int, int> s2data     ;
		map<int, bool> sChanged  ;
		map<int, bool> sTouched  ;
		
		e_find find_parms  ;

		bool rebuildZAREA  ;
		bool fileChanged   ;

		string file    ;
		string CURFLD  ;
		int    CURPOS  ;
		string MSG     ;
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

		string ZPSAVE  ;
		string ZPNULLS ;
		string ZPLOCK  ;
		string ZPCAPS  ;
		string ZPHEX   ;


		string fileType ;

		string blkcmds  ;
		string sglcmds  ;
		string spllcmds ;
		string todlcmds ;
		string bodlcmds ;
		string ABOReq   ;
		string ABOList  ;
		string MultOK   ;

		string sdg ;
		string sdy ;
		string sdw ;
		string sdr ;
		string div ;
		string slg ;
		string sly ;
		string slw ;
		string slr ;


/*		const string blkcmds  ;
		const string sglcmds  ;
		const string spllcmds ;
		const string todlcmds ;
		const string bodlcmds ;
		const string ABOReq   ;
		const string ABOList  ;
		const string MultOK   ;
*/
/*		string blkcmds  = "CC MM DD RR XX (( )) UCC LCC" ;
		string sglcmds  = "C M D R X I ( ) UC LC COL COLS S A B O F L";
		string spllcmds = "COL COLS A B I C M D R CC MM DD RR";
		string todlcmds = "COL COLS A I";
		string bodlcmds = "B";
		string ABOReq   = "CC MM C M";
		string ABOList  = "A B O";
		string MultOK   = "C M D X R UC LC RR (( )) ( ) F L"; */
} ;

