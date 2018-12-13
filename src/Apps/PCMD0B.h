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

class PCMD0B : public pApplication
{
	public:
		PCMD0B() ;
		void application() ;

	private:
		void copy_output( const string&, const string&, const string& ) ;
		void fill_dynamic_area( bool ) ;
		void actioniCommand() ;
		void actionZVERB()    ;
		bool invoke_task( string, string&, const string& ) ;
		void bottom_of_data() ;
		string command_prompt() ;
		string get_tempname( const string& ) ;

		int topLine    ;
		int startCol   ;
		uint maxCol    ;
		uint zasize    ;

		string wd      ;
		string msg     ;
		string inLine  ;

		int    zaread  ;
		int    zareaw  ;
		string zcmd    ;
		string zverb   ;
		string znode   ;
		string zscreen ;
		string zuser   ;
		string zarea   ;
		string zshadow ;
		string zareat  ;

		string sdr ;
		string sdw ;
		string sdy ;
		string sdg ;
		string sdt ;

		string zscrolla ;
		int    zscrolln ;

		bool rebuildZAREA ;
		bool running      ;

		vector<string> lines ;
		map<string,string> cmds ;
};
