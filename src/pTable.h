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

class Table
{
	public:
		Table()
		{
			refCount       = 1     ;
			CRP            = 0     ;
			maxURID        = 0     ;
			changed        = false ;
			tab_path       = ""    ;
			sa_namelst     = ""    ;
			sa_cond_pairs  = ""    ;
			sa_dir         = ""    ;
			sort_ir        = ""    ;
			sort_flds      = ""    ;
			sort_type      = ""    ;
			sorted_on_keys = false ;
			sorted_asc     = false ;
		}
		void   saveTable( int & RC, string m_name, string m_path ) ;
		void   loadRow( int & RC, vector< string > m_flds ) ;
		void   reserveSpace( int tot_rows ) ;
		void   resetChanged() { changed  = false ; } ;

		void   fillfVARs( int & RC, fPOOL & funcPOOL, int depth, int posn ) ;
		int    getCRP() { return CRP ; }
		void   cmdsearch( int & RC, fPOOL & funcPOOL, string srch ) ;

		void   tbadd( int & RC, fPOOL & funcPOOL, string tb_namelst, string tb_order, int tb_num_of_rows )  ;
		void   tbbottom( int & RC ) ;
		void   tbdelete( int & RC, fPOOL & funcPOOL ) ;
		void   tbexist( int & RC, fPOOL & funcPOOL ) ;
		void   tbget( int & RC, fPOOL & funcPOOL, string tb_savenm, string tb_rowid_vn, string tb_noread, string tb_crp_name ) ;
		void   tbmod( int & RC, fPOOL & funcPOOL, string tb_namelst, string tb_order ) ;
		void   tbput( int & RC, fPOOL & funcPOOL, string tb_namelst, string tb_order ) ;
		void   tbquery( int & RC, fPOOL & funcPOOL, string tb_keyn, string tb_varn, string tb_rownn, string tb_keynn, string tb_namenn, string tb_crpn, string tb_sirn, string tb_lstn, string tb_condn, string tb_dirn ) ;
		void   tbsarg( int & RC, fPOOL & funcPOOL, string tb_namelst, string tb_next_prev, string tb_cond_pairs ) ;
		void   tbscan( int &RC, fPOOL & funcPOOL, string tb_namelst, string tb_varname, string tb_rowid_vn, string tb_next_prev, string tb_read, string tb_crp_name, string tb_condlst ) ;
		void   tbskip( int & RC, fPOOL & funcPOOL, int num, string tb_savenm, string tb_rowid_vn, string tb_rowid, string tb_noread, string tb_crp_name ) ;
		void   tbsort( int & RC, fPOOL & funcPOOL, string tb_fields ) ;
		void   tbtop( int & RC ) ;
		void   tbvclear( int & RC, fPOOL & funcPOOL ) ;

	private:
		vector< vector< string > > table  ;

		map< string, tbsearch > sarg ;

		int    ownerTask      ;
		string tab_keys       ;
		string tab_flds       ;
		string tab_all        ;
		string tab_path       ;
		bool   changed        ;
		int    refCount       ;
		int    maxURID        ;
		int    num_keys       ;
		int    num_flds       ;
		int    num_all        ;
		int    CRP            ;
		bool   sorted_on_keys ;
		bool   sorted_asc     ;
		string sort_ir        ;
		string sort_flds      ;
		string sort_type      ;
		string sa_namelst     ;
		string sa_cond_pairs  ;
		string sa_dir         ;
		tbSAVE tab_SAVE       ;
		tbDISP tab_DISP       ;


		friend class tableMGR ;
};


class tableMGR
{
	public:
		tableMGR() ;

		void   tbget( int & RC, fPOOL & funcPOOL, string tb_name, string tb_savenm, string tb_rowid_vn, string tb_noread, string tb_crp_name  ) ;

		void   fillfVARs( int & RC, fPOOL & funcPOOL, string tb_name, int depth, int posn ) ;
		int    getCRP( int & RC, string tb_name ) ;
		void   cmdsearch( int & RC, fPOOL & funcPOOL, string tb_name, string srch ) ;

		void   createTable( int & RC, int task, string tb_name, string keys, string flds, tbSAVE m_SAVE, tbREP m_REP, string m_path, tbDISP m_DISP ) ;
		void   destroyTable( int & RC, int task, string tb_name ) ;
		void   loadTable( int & RC, int task, string tb_name, tbSAVE=WRITE, tbDISP=EXCLUSIVE, string src="" ) ;
		void   saveTable( int & RC, int task, string tb_name, string m_newname, string m_path ) ;
		void   saveTableifWRITE( int & RC, int task, string tb_name, string m_newname, string m_path ) ;
		bool   isloaded( string tb_name ) ;
		bool   tablexists( string tb_name, string tb_path ) ;

		void   statistics() ;
		void   snap() ;

		void   tbadd( int & RC, fPOOL & funcPOOL, string tb_name, string tb_namelst, string tb_order, int tb_num_of_rows ) ;
		void   tbbottom( int & RC, string tb_name ) ;
		void   tbdelete( int & RC, fPOOL & funcPOOL, string tb_name ) ;
		void   tberase( int & RC, string tb_name, string tb_path ) ;
		void   tbexist( int & RC, fPOOL & funcPOOL, string tb_name ) ;
		void   tbmod( int & RC, fPOOL & funcPOOL, string tb_name, string tb_namelst, string tb_order ) ;
		void   tbput( int & RC, fPOOL & funcPOOL, string tb_name, string tb_namelst, string tb_order ) ;
		void   tbquery( int & RC, fPOOL & funcPOOL, string tb_name, string tb_keyn, string tb_varn, string tb_rownn, string tb_keynn, string tb_namenn, string tb_crpn, string tb_sirn, string tb_lstn, string tb_condn, string tb_dirn ) ;
		void   tbsarg( int & RC, fPOOL & funcPOOL, string tb_name, string tb_namelst, string tb_next_prev, string tb_cond_pairs ) ;
		void   tbskip( int & RC, fPOOL & funcPOOL, string tb_name, int num, string tb_varname, string tb_rowid_vn, string tb_rowid, string tb_noread, string tb_crp_name ) ;
		void   tbscan( int & RC, fPOOL & funcPOOL, string tb_name, string tb_namelst, string tb_savenm, string tb_rowid_vn, string tb_next_prev, string tb_read, string tb_crp_name, string tb_condlst ) ;
		void   tbsort( int & RC, fPOOL & funcPOOL, string tb_name, string tb_fields ) ;
		void   tbtop( int & RC, string tb_name ) ;
		void   tbvclear( int & RC, fPOOL & funcPOOL, string tb_name ) ;

	private:
		map< string, Table > tables  ;
};
