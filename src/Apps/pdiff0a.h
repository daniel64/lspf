/*
  Copyright (c) 2020 Daniel John Erdos

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
using namespace boost::filesystem ;


class entry_info
{
	public:
		entry_info()
		{
			mod = false ;
			inA = false ;
			inB = false ;
			isd = false ;
			ma  = "" ;
			mb  = "" ;
			sa  = "" ;
			sb  = "" ;
		}

		void clear()
		{
			file = "" ;
			dira = "" ;
			dirb = "" ;
			mod  = false ;
			inA  = false ;
			inB  = false ;
			isd  = false ;
			ma   = "" ;
			mb   = "" ;
			sa   = "" ;
			sb   = "" ;
			oa   = "" ;
			ob   = "" ;
		}

		string file ;
		string dira ;
		string dirb ;
		bool   mod ;
		bool   inA ;
		bool   inB ;
		bool   isd ;
		string ma  ;
		string mb  ;
		string sa  ;
		string sb  ;
		string oa  ;
		string ob  ;

		bool operator == ( const entry_info& rhs ) const
		{
			return ( file == rhs.file ) ;
		}

		bool operator != ( const entry_info& rhs ) const
		{
			return ( file != rhs.file ) ;
		}

		bool operator > ( const entry_info& rhs ) const
		{
			return ( file > rhs.file ) ;
		}

		bool operator < ( const entry_info& rhs ) const
		{
			return ( file < rhs.file ) ;
		}

} ;


class pdiff0a : public pApplication
{
	public:
		pdiff0a() ;

		void application() ;

	private:
		void initialise() ;

		void create_patch( const char*,
				   string ) ;

		void compare_files( const string&,
				    const string&,
				    const string& = "",
				    bool = false ) ;

		void compare_dirs( const string&,
				   const string& ) ;

		void merge_files( const string&,
				  const string& ) ;

		void copy_entry( const string&,
				 const string& ) ;

		void edit_file( const string& ) ;

		string create_diff_cmd( const string&,
					const string&,
					const string& ) ;

		void execute_cmd( int&,
				  const string&,
				  vector<string>& ) ;

		void execute_cmd( int&,
				  const string&,
				  const string& ) ;

		void create_table( set<entry_info>& ) ;

		void parse_diff_output( const string&,
					const string&,
					vector<string>&,
					set<entry_info>& ) ;

		struct stat get_lstat( const string& ) ;
		string moddate( const struct stat& ) ;
		string moddats( const struct stat& ) ;
		string filesiz( const struct stat& ) ;

		bool moddate_greater( const struct stat&,
				      const struct stat& ) ;

		string full_name( const string&,
				  const string& ) ;

		string dir( const string& ) ;

		string file_name( const string& ) ;

		void copy_file_attributes( const struct stat&,
					   const string&,
					   const string& ) ;

		void set_tb_variables( uint ) ;

		string get_shared_var( const string& ) ;

		void set_shared_var( const string&,
				     const string& ) ;

		void set_shared_var( const string&,
				     int ) ;

		string create_tempname() ;

		string entrya   ;
		string entryb   ;
		string showi    ;
		string showa    ;
		string showb    ;
		string recur1   ;
		string cmpins   ;
		string cmpigd   ;
		string cmpigb   ;
		string cmpits   ;
		string cmpigt   ;
		string exclpatt ;
		string ignregex ;
		string procopts ;
		string dffutt   ;
		string dfcntx   ;
		string cdir     ;

		string isfile1  ;
		string isdir1   ;

		const char* patch_dir ;
		const char* patch_file ;

		string table    ;
		string sel      ;
		string entry    ;
		string inA      ;
		string inB      ;
		string mod      ;
		string isd      ;
		string mdatea   ;
		string sizea    ;
		string mdateb   ;
		string sizeb    ;
		string mdateao  ;
		string mdatebo  ;

		string zstr1    ;
		string zuser    ;
		string zscreen  ;
		string zverb    ;
		int    zscreenw ;
		int    zscreend ;

		string lit1     ;
		string lit2     ;
		string lit3     ;

		int    zcurinx  ;
		int    ztdtop   ;
		int    ztdsels  ;
		int    crp      ;

		int    f1len    ;
		int    f2col    ;
		int    f2len    ;
		int    f3col    ;
		int    f3len    ;
		int    f4col    ;
		int    f4len    ;

		int    f5len    ;
		int    f6col    ;
		int    f6len    ;
		int    f7col    ;
		int    f7len    ;
		int    f8col    ;
		int    f8len    ;
		int    f9col    ;
		int    f9len    ;

} ;
