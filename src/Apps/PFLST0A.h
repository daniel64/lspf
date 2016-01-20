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


class PFLST0A : public pApplication
{
	public:
		void application() ;

	private:
		void createFileList1( string ="" ) ;
		void createFileList2( string, string ="" ) ;
		void createSearchList( string ) ;

		string showListing() ;
		void showInfo( string ) ;
		int  processPrimCMD()   ;
		void copyDirs( string, string, string, bool & ) ;
		void modifyAttrs( string ) ;
		string expandDir( string ) ;
		void browseTree( string )  ;

		vector<string> SearchList ;
		bool   UseSearch ;

		int    CRP ;
		string MSG ;
		string RSN ;

		string ZPATH  ;
		string ZDIR   ;
		string DSLIST ;
		string SEL, ENTRY, MESSAGE, TYPE, PERMISS, SIZE, STCDATE, MODDATE, MODDATES ;
		string IENTRY, ITYPE, IOWNER, IGROUP, IINODE, INLNKS, IRLNK, IPERMISS, ISIZE, ISTCDATE, IMODDATE, IACCDATE, IMAJ, IMIN, IBLKSIZE ;
		string ISETUID, ISETGID, ISTICKY, IOWNERN, IGROUPN ;
} ;
