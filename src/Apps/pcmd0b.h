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

#include <boost/circular_buffer.hpp>

#define N_RED     0x03
#define N_GREEN   0x04
#define N_YELLOW  0x05
#define N_BLUE    0x06
#define N_MAGENTA 0x07
#define N_TURQ    0x08
#define N_WHITE   0x09

#define B_RED     0x0A
#define B_GREEN   0x0B
#define B_YELLOW  0x0C
#define B_BLUE    0x0D
#define B_MAGENTA 0x0E
#define B_TURQ    0x0F
#define B_WHITE   0x10

#define R_RED     0x11
#define R_GREEN   0x12
#define R_YELLOW  0x13
#define R_BLUE    0x14
#define R_MAGENTA 0x15
#define R_TURQ    0x16
#define R_WHITE   0x17

using namespace std;

class pcmd0b : public pApplication
{
	public:
		pcmd0b() ;
		void application() ;

	private:
		void copy_output( const string&,
				  const string&,
				  const string& ) ;

		void fill_dynamic_area( bool ) ;
		void actionZVERB() ;
		bool invoke_task( string,
				  const string&,
				  const string& ) ;

		bool isRexx( string ) ;
		void bottom_of_data() ;
		string command_prompt() ;
		string get_tempname( const string& ) ;

		int topLine    ;
		int startCol   ;

		uint maxCol    ;
		uint zasize    ;

		size_t buf_limit ;

		string wd      ;
		string msg     ;
		string inLine  ;

		int    zaread  ;
		int    zareaw  ;
		string zcmd    ;
		string zcmd1   ;
		string zverb   ;
		string znode   ;
		string zhome   ;
		string zscreen ;
		string zuser   ;
		string zarea   ;
		string zshadow ;
		string zsbtask ;

		int    zscreend ;
		int    zscreenw ;

		string sdr ;
		string sdw ;
		string sdy ;
		string sdg ;
		string sdt ;

		string ansitxt ;

		string zscrolla ;
		int    zscrolln ;

		uint   ppos ;

		bool rebuildZAREA ;
		bool running ;
		bool ansi_on ;

		const string ansi_red     = "\x1b[31m" ;
		const string ansi_green   = "\x1b[32m" ;
		const string ansi_yellow  = "\x1b[33m" ;
		const string ansi_blue    = "\x1b[34m" ;
		const string ansi_magenta = "\x1b[35m" ;
		const string ansi_turq    = "\x1b[36m" ;
		const string ansi_white   = "\x1b[37m" ;

		void action_ANSI() ;
		void action_CLEAR() ;
		void action_LABELS() ;
		void action_LIST() ;
		void action_INFO() ;
		void action_HELP() ;
		void action_HISTORY() ;

		void action_ScrollLeft() ;
		void action_ScrollRight() ;
		void action_ScrollUp() ;
		void action_ScrollDown() ;

		void action_prev() ;

		void term_resize() ;

		void remove_wd_lld() ;
		void add_path_history() ;
		void chng_directory() ;

		string full_name( const string& ,
				  const string& ) ;

		vector<string> lines ;
		map<string,pair<int,string>> cmds ;
		boost::circular_buffer<string> path_history ;

		map<string, int> labels ;

		map<int, int> kw_ansi = {
		  { 31, N_RED     },
		  { 32, N_GREEN   },
		  { 33, N_YELLOW  },
		  { 34, N_BLUE    },
		  { 35, N_MAGENTA },
		  { 36, N_TURQ    },
		  { 37, N_WHITE   },
		  { 41, R_RED     },
		  { 42, R_GREEN   },
		  { 43, R_YELLOW  },
		  { 44, R_BLUE    },
		  { 45, R_MAGENTA },
		  { 46, R_TURQ    },
		  { 47, R_WHITE   },
		  { 91, B_RED     },
		  { 92, B_GREEN   },
		  { 93, B_YELLOW  },
		  { 94, B_BLUE    },
		  { 95, B_MAGENTA },
		  { 96, B_TURQ    },
		  { 97, B_WHITE   } } ;

		map<string, void(pcmd0b::*)()>icmdList  = { { "-ANSI",   &pcmd0b::action_ANSI    },
							    { "-CLEAR",  &pcmd0b::action_CLEAR   },
							    { "-LABELS", &pcmd0b::action_LABELS  },
							    { "-LIST",   &pcmd0b::action_LIST    },
							    { "-INFO",   &pcmd0b::action_INFO    },
							    { "-HELP",   &pcmd0b::action_HELP    },
							    { "-HIST",   &pcmd0b::action_HISTORY } } ;

		map<string, void(pcmd0b::*)()>scrollList = { { "UP",    &pcmd0b::action_ScrollUp    },
							     { "DOWN",  &pcmd0b::action_ScrollDown  },
							     { "LEFT",  &pcmd0b::action_ScrollLeft  },
							     { "RIGHT", &pcmd0b::action_ScrollRight } } ;

} ;
