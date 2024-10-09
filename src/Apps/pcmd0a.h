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

class pcmd0a : public TSOENV
{
	public:
		pcmd0a() ;
		void application() ;

	private:
		void run_command( string,
				  const string&,
				  const string& ) ;

		bool invoke_task_and_wait( const string&,
					   const string&,
					   const string& ) ;

		bool command_exists( const string& ) ;

		string get_string( const string&,
				   const string& ) ;

		string get_tempname( const string& ) ;

		bool isRexx( string ) ;

		string zscreen ;
		string zuser   ;

} ;