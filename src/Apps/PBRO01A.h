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

class b_shadow
{
	private:
		string bs_Shadow ;
		bool   bs_vShadow ;
		bool   bs_wShadow ;
	b_shadow()
	{
		bs_Shadow  = ""    ;
		bs_vShadow = false ;
		bs_wShadow = false ;
	}
	friend class PBRO01A ;
} ;


class b_find
{
	private:
		string f_string  ;
		string f_estring ;
		int    f_occurs  ;
		int    f_line    ;
		int    f_lines   ;
		int    f_offset  ;
		bool   f_regreq  ;
		bool   f_text    ;
		bool   f_asis    ;
		bool   f_hex     ;
		bool   f_pic     ;
		bool   f_rreg    ;
		int    f_scol    ;
		int    f_ecol    ;
		bool   f_oncol   ;
		bool   f_fset    ;
		char   f_dir     ;
		char   f_mtch    ;
	b_find()
	{
		f_string  = ""    ;
		f_estring = ""    ;
		f_occurs  = 0     ;
		f_line    = 0     ;
		f_lines   = 0     ;
		f_offset  = 0     ;
		f_regreq  = false ;
		f_text    = false ;
		f_asis    = false ;
		f_hex     = false ;
		f_pic     = false ;
		f_rreg    = false ;
		f_scol    = 0     ;
		f_ecol    = 0     ;
		f_oncol   = false ;
		f_fset    = false ;
		f_dir     = 'N'   ;
		f_mtch    = 'C'   ;
	}
	friend class PBRO01A ;
} ;


class PBRO01A : public pApplication
{
	public:
		PBRO01A() ;
		void application() ;

	private:
		static b_find Global_bfind_parms ;

		void read_file( string )    ;
		void fill_dynamic_area()    ;
		void fill_hilight_shadow()  ;

		int  setFind()              ;
		void actionFind( int, int ) ;

		string determineLang( string ) ;

		int  topLine  ;
		uint maxLines ;
		int  startCol ;

		uint maxCol  ;

		vector<string> data     ;
		vector<b_shadow> shadow ;
		map< char, string > typList ;

		b_find find_parms ;
		hilight hlight    ;

		bool hexOn  ;
		bool colsOn ;
		bool Asbin  ;
		bool binOn  ;
		bool textOn ;
		bool hilightOn ;

		string zcmd    ;
		string zverb   ;
		string zcurfld ;
		int    zcurpos ;

		string msg     ;
		string cmd     ;
		string zrow1   ;
		string zcol1   ;
		string zcol2   ;
		string zarea   ;
		string zshadow ;
		string cshadow ;
		string zareat  ;
		int    zareaw  ;
		int    zaread  ;
		uint   zasize  ;
		int    zaline  ;
		string zscrolla ;
		int    zscrolln ;

		string type  ;
		string str   ;
		string occ   ;
		string lines ;

		string zzstr1 ;
		string zdsn   ;
		string fileType ;
		string detLang  ;
		string entLang  ;
} ;
