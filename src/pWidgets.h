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

class dynArea
{
	private:
		dynArea()
		{
			dynArea_userModsp = false ;
			dynArea_dataModsp = false ;
			dynArea_scroll    = false ;
			dynArea_Attrs     = "" ;
			dynArea_inAttrs   = "" ;
			dynArea_olen      = 65535 ;
		}

		dynArea( errblock&,
			 int maxw,
			 int maxd,
			 const string&,
			 const string&,
			 const string&,
			 uint = 0 ) ;

		void   add( void* ) ;

		uint   dynArea_row ;
		uint   dynArea_col ;
		uint   dynArea_width ;
		uint   dynArea_depth ;
		uint   dynArea_area ;
		uint   dynArea_olen ;
		bool   dynArea_userModsp ;
		bool   dynArea_dataModsp ;
		bool   dynArea_scroll ;
		char   dynArea_UserMod ;
		char   dynArea_DataMod ;
		string dynArea_Attrs ;
		string dynArea_inAttrs ;
		string dynArea_name ;
		string dynArea_oprefix ;
		string dynArea_olenvar ;
		string dynArea_shadow_name ;

		vector<void*> fieldList ;

	friend class pPanel ;
	friend class field ;
	friend class field_ext1 ;
} ;


class field_ext1
{
	//
	// Field extension structure for dynamic areas.
	//

	public:
		field_ext1()
		{
			field_ext1_shadow  = "" ;
			field_ext1_index   = 0 ;
			field_ext1_changed = false ;
		}

		void field_ext1_set( dynArea* da,
				     int index )
		{
			field_ext1_dynArea = da ;
			field_ext1_index   = index ;
			if ( da->dynArea_oprefix != "" )
			{
				string suf = d2ds( index, 3 ) ;
				field_ext1_overflow_vname = da->dynArea_oprefix + "V" + suf ;
				field_ext1_overflow_sname = da->dynArea_oprefix + "S" + suf ;
				field_ext1_overflow_uname = da->dynArea_oprefix + "U" + suf ;
			}
		}

		void field_ext1_load( const string* str1,
				      const string* str2 )
		{
			field_ext1_overflow_value.assign( *str1, 0, field_ext1_dynArea->dynArea_olen ) ;
			field_ext1_overflow_shadow.assign( *str2, 0, field_ext1_dynArea->dynArea_olen ) ;
			field_ext1_overflow_shadow.resize( field_ext1_overflow_value.size(), ' ' ) ;

			field_ext1_changed = false ;
		}

		void field_ext1_load( const string* str1,
				      const string* str2,
				      const string* str3 )
		{
			field_ext1_overflow_value.assign( *str1, 0, field_ext1_dynArea->dynArea_olen ) ;
			field_ext1_overflow_shadow.assign( *str2, 0, field_ext1_dynArea->dynArea_olen ) ;
			field_ext1_overflow_shadow.resize( field_ext1_overflow_value.size(), ' ' ) ;

			field_ext1_changed = ( *str3 == "Y" ) ;
		}

		void field_ext1_clear()
		{
			field_ext1_overflow_value  = "" ;
			field_ext1_overflow_shadow = "" ;

			field_ext1_changed = true ;
		}

		void field_ext1_clear_input( const string attrs )
		{
			size_t p1 = field_ext1_overflow_value.find_first_of( attrs ) ;
			if ( p1 == string::npos )
			{
				field_ext1_overflow_value  = "" ;
				field_ext1_overflow_shadow = "" ;
			}
			else
			{
				field_ext1_overflow_value.replace( 0, p1, p1, ' ' ) ;
				field_ext1_overflow_shadow.replace( 0, p1, p1, 0xFE ) ;
			}

			field_ext1_changed = true ;
		}

		void field_ext1_reset()
		{
			field_ext1_changed = false ;
		}

		bool field_ext1_has_overflow() const
		{
			return ( field_ext1_overflow_vname != "" ) ;
		}


		bool field_ext1_getch( char& c1,
				       char& c2 )
		{
			if ( field_ext1_overflow_value == "" ) { return false ; }

			c1 = field_ext1_overflow_value.front() ;
			field_ext1_overflow_value.erase( 0, 1 ) ;

			c2 = field_ext1_overflow_shadow.front() ;
			field_ext1_overflow_shadow.erase( 0, 1 ) ;

			field_ext1_changed = true ;

			return true ;
		}

		bool field_ext1_putch( char c1,
				       char c2 )
		{
			if ( field_ext1_overflow_value.size() > 0 &&
			     field_ext1_overflow_value.size() == field_ext1_dynArea->dynArea_olen &&
			     field_ext1_overflow_value.back() == ' ' )
			{
				field_ext1_overflow_value.pop_back() ;
				field_ext1_overflow_shadow.pop_back() ;
			}
			else if ( field_ext1_overflow_value.size() >= field_ext1_dynArea->dynArea_olen )
			{
				return false ;
			}

			field_ext1_overflow_value.insert( 0, 1, c1 ) ;
			field_ext1_overflow_shadow.insert( 0, 1, c2 ) ;

			field_ext1_changed = true ;

			return true ;
		}

	private:
		dynArea* field_ext1_dynArea ;

		string field_ext1_shadow ;
		string field_ext1_overflow_vname ;
		string field_ext1_overflow_sname ;
		string field_ext1_overflow_uname ;
		string field_ext1_overflow_value ;
		string field_ext1_overflow_shadow ;

		int  field_ext1_index ;
		bool field_ext1_changed ;

		set<size_t> field_ext1_usermod ;

	friend class pPanel ;
	friend class field ;
} ;


class field_ext2
{
	//
	// Field extension structure for table displays.
	//

	public:
		field_ext2()
		{
			field_ext2_index = 0 ;
		}

		field_ext2( const string& name ) : field_ext2()
		{
			field_ext2_name = name ;
		}

		void field_ext2_set( int index )
		{
			field_ext2_index = index ;
		}

	private:
		string field_ext2_name ;
		int    field_ext2_index ;

	friend class pPanel ;
	friend class field ;
} ;


class field_ext3
{
	//
	// Field extension structure for scrollable fields.
	//

	public:
		field_ext3()
		{
			ssfield_start  = 0 ;
			ssfield_tblen  = 0 ;
			ssfield_snlen  = 0 ;
			ssfield_value  = "" ;
			ssfield_scale  = "" ;
			ssfield_sfield = nullptr ;
		}

		field_ext3( sfield* s,
			    uint flen ) : field_ext3()
		{
			ssfield_sfield = s ;
			ssfield_fdlen  = flen ;
		}

	private:
		size_t  ssfield_start ;
		size_t  ssfield_fdlen ;
		size_t  ssfield_tblen ;
		uint    ssfield_snlen ;
		string  ssfield_value ;
		string  ssfield_scale ;
		string  ssfield_sind2 ;
		sfield* ssfield_sfield ;

		const string& ssfield_get_value()
		{
			return ssfield_value ;
		}

		uint ssfield_get_display_len()
		{
			//
			// Calculate display length:
			// If no LEN() specified, set to the maximum of variable length and field length (default display length).
			// If LEN() specified (numeric or variable) set to maximum of LEN(), default display length.
			//
			// For table displays, use ssfield_tblen which is the largest length on the current display.
			//

			return ( ssfield_tblen == 0 ) ? max( ssfield_sfield->len, max( ssfield_fdlen, ssfield_value.size() ) ) : ssfield_tblen ;
		}

		void ssfield_set_len( size_t l )
		{
			//
			// Set the LEN() value.  Minimum size of len set this way is always ssfield_fdlen.
			//
			// Adjust ssfield_start in case len has been reduced too much.
			//

			ssfield_sfield->len = max( l, max( ssfield_fdlen, ssfield_value.size() ) ) ;

			if ( ssfield_start > ( ssfield_sfield->len - ssfield_fdlen ) )
			{
				ssfield_start = ( ssfield_sfield->len > ssfield_fdlen ) ? ssfield_sfield->len - ssfield_fdlen : 0 ;
			}
		}

		void ssfield_set_tblen( size_t l )
		{
			//
			// Set the temporary display length override for table displays.
			//

			if ( ssfield_tblen != l )
			{
				ssfield_tblen = l ;
				ssfield_scale = "" ;
				ssfield_sind2 = "" ;
			}
		}

		bool ssfield_update_tblen_max()
		{
			//
			// Update tblen if variable value is larger, and return true ;
			//

			if ( ssfield_tblen >= ssfield_value.size() )
			{
				return false ;
			}

			ssfield_tblen = ssfield_value.size() ;
			ssfield_scale = "" ;
			ssfield_sind2 = "" ;

			return true ;
		}

		string ssfield_get_display_value()
		{
			if ( !ssfield_sfield->scroll_on )
			{
				if ( !ssfield_sfield->scrollvar )
				{
					if ( ssfield_value.size() > ssfield_fdlen )
					{
						ssfield_value.erase( ssfield_fdlen ) ;
					}
					ssfield_start = 0 ;
				}
				return ssfield_value ;
			}

			if ( ssfield_start >= ssfield_value.size() )
			{
				return "" ;
			}

			return ( ssfield_value.size() > ( ssfield_start + ssfield_fdlen ) ? ssfield_value.substr( ssfield_start, ssfield_fdlen ) :
											    ssfield_value.substr( ssfield_start ) ) ;
		}

		void ssfield_put_value( const string& s )
		{
			if ( !ssfield_sfield->scroll_on && ssfield_value.size() > ssfield_fdlen )
			{
				ssfield_value = s.substr( 0, ssfield_fdlen ) ;
			}
			else
			{
				ssfield_value = ( s.size() > 32767 ) ? s.substr( 0, 32767 ) : s ;
			}
		}

		void ssfield_update_value( const string& val )
		{
			if ( ssfield_value.size() < ssfield_start )
			{
				ssfield_value.resize( ssfield_start, ' ' ) ;
			}
			ssfield_value.replace( ssfield_start, ssfield_fdlen, left( val, ssfield_fdlen ) ) ;
			trim_right( ssfield_value ) ;
			if ( ssfield_value.size() > 32767 )
			{
				ssfield_value.erase( 32767 ) ;
			}
		}

		void ssfield_update_fdlen( uint l )
		{
			ssfield_fdlen = l ;
		}

		bool ssfield_getch( char& c )
		{
			uint pos = ssfield_start + ssfield_fdlen ;

			if ( ssfield_value.size() <= pos ) { return false ; }

			c = ssfield_value[ pos ] ;
			ssfield_value.erase( pos, 1 ) ;

			return true ;
		}

		bool ssfield_putch( char c )
		{
			if ( ssfield_value.size() >= 32767 )
			{
				return false ;
			}

			if ( ssfield_value.size() < ( ssfield_start + ssfield_fdlen ) )
			{
				ssfield_value.resize( ( ssfield_start + ssfield_fdlen ), ' ' ) ;
			}

			ssfield_value.insert( ( ssfield_start + ssfield_fdlen ), 1, c ) ;
			trim_right( ssfield_value ) ;

			return true ;
		}

		void ssfield_erase_eof( uint p )
		{
			uint i = ssfield_start + p ;
			if ( i < ssfield_value.size() )
			{
				ssfield_value.erase( i ) ;
			}
		}

		void ssfield_erase_spaces( uint p )
		{
			uint i = ssfield_start + p ;
			if ( i < ssfield_value.size() )
			{
				uint j = ssfield_value.find_first_not_of( ' ', i ) ;
				if ( j != string::npos )
				{
					ssfield_value.erase( i, ( j - i ) ) ;
				}
				else if ( i < ssfield_value.size() )
				{
					ssfield_value.erase( i ) ;
				}
			}
		}

		void ssfield_erase_word( uint p )
		{
			uint i = ssfield_start + p ;
			if ( i < ssfield_value.size() )
			{
				i = ssfield_value.find_first_not_of( ' ', i ) ;
				if ( i != string::npos )
				{
					idelword( ssfield_value, words( ssfield_value.substr( 0, i+1 ) ), 1 ) ;
				}
			}
		}

		bool ssfield_value_is_blank()
		{
			return ( ssfield_value == "" ) ;
		}

		uint ssfield_get_start()
		{
			return ssfield_start ;
		}

		bool ssfield_at_beginning()
		{
			return ( ssfield_start == 0 ) ;
		}

		bool ssfield_has_lenvar()
		{
			return ( ssfield_sfield->lenvar != "" ) ;
		}

		bool ssfield_has_ind()
		{
			return ( ssfield_sfield->ind1 != "" ) ;
		}

		bool ssfield_has_lind()
		{
			return ( ssfield_sfield->lind1 != "" ) ;
		}

		bool ssfield_has_rind()
		{
			return ( ssfield_sfield->rind1 != "" ) ;
		}

		bool ssfield_has_sind()
		{
			return ( ssfield_sfield->sind1 != "" ) ;
		}

		bool ssfield_has_scale()
		{
			return ( ssfield_sfield->scale != "" ) ;
		}

		const string& ssfield_get_lenvar()
		{
			return ssfield_sfield->lenvar ;
		}

		const string& ssfield_get_ind1()
		{
			return ssfield_sfield->ind1 ;
		}

		const string& ssfield_get_lind1()
		{
			return ssfield_sfield->lind1 ;
		}

		const string& ssfield_get_rind1()
		{
			return ssfield_sfield->rind1 ;
		}

		const string& ssfield_get_sind1()
		{
			return ssfield_sfield->sind1 ;
		}

		string ssfield_get_ind2()
		{
			uint dl = ssfield_get_display_len() ;

			return ( !ssfield_sfield->scroll_on )                     ? "  " :
			       ( ssfield_start == 0 && dl <= ssfield_fdlen )      ? "  " :
			       ( ssfield_start == 0 && dl  > ssfield_fdlen )      ? ssfield_sfield->s2ind2 :
			       ( dl            == ssfield_fdlen + ssfield_start ) ? ssfield_sfield->s3ind2 : ssfield_sfield->s4ind2 ;
		}

		string ssfield_get_lind2()
		{
			return ( !ssfield_sfield->scroll_on ) ? " " :
			       ( ssfield_start == 0 )         ? " " : ssfield_sfield->lind2 ;
		}

		string ssfield_get_rind2()
		{
			uint dl = ssfield_get_display_len() ;

			return ( !ssfield_sfield->scroll_on )                     ? " " :
			       ( ssfield_start == 0 && dl <= ssfield_fdlen )      ? " " :
			       ( ssfield_start == 0 && dl  > ssfield_fdlen )      ? ssfield_sfield->rind2 :
			       ( dl            == ssfield_fdlen + ssfield_start ) ? " " : ssfield_sfield->rind2 ;
		}

		const string& ssfield_get_sind2( uint l )
		{
			if ( !ssfield_sfield->scroll_on )
			{
				ssfield_sind2 = "" ;
				return ssfield_sind2 ;
			}

			uint dl = ssfield_get_display_len() ;

			if ( ssfield_sind2 != "" && l == ssfield_snlen ) { return ssfield_sind2 ; }

			l = max( uint( 3 ), l ) ;

			ssfield_snlen = l ;

			if ( ssfield_start == 0 && dl <= ssfield_fdlen )
			{
				ssfield_sind2 = string( l, ssfield_sfield->sind2[ 1 ] ) ;
			}
			else if ( ssfield_start == 0 && dl > ssfield_fdlen )
			{
				ssfield_sind2 = string( l-1, ssfield_sfield->sind2[ 1 ] ) +
						string( 1, ssfield_sfield->sind2[ 2 ] ) ;
			}
			else if ( dl == ssfield_fdlen + ssfield_start )
			{
				ssfield_sind2 = string( 1, ssfield_sfield->sind2[ 0 ] ) +
						string( l-1, ssfield_sfield->sind2[ 1 ] ) ;
			}
			else
			{
				ssfield_sind2 = string( 1, ssfield_sfield->sind2[ 0 ] ) +
						string( l-2, ssfield_sfield->sind2[ 1 ] ) +
						string( 1, ssfield_sfield->sind2[ 2 ] ) ;
			}

			return ssfield_sind2 ;
		}

		const string& ssfield_get_scale1()
		{
			return ssfield_sfield->scale ;
		}

		const string& ssfield_get_scale2( uint len )
		{
			if ( !ssfield_sfield->scroll_on )
			{
				ssfield_scale = "" ;
				return ssfield_scale ;
			}

			if ( ssfield_scale != "" ) { return ssfield_scale ; }

			int i ;
			int j ;
			int k ;
			int l ;
			int n ;
			int s ;

			string t1 = "" ;
			string t2 = "" ;

			const string ruler = "----+----" ;

			s = ssfield_start + 1 ;
			l = ( ssfield_start + len ) / 10 ;

			i = s % 10 ;
			j = s / 10 ;

			if ( i > 0 )
			{
				++j ;
				t1 = substr( ruler, i, 10-i ) ;
			}

			for ( k = 0 ; k <= l ; ++k, ++j )
			{
				t2 = d2ds( j ) ;
				n  = t1.size() - t2.size() + 1 ;
				if ( n < 0 )
				{
					t2.erase( 0, abs( n ) ) ;
					t1 = "" ;
				}
				else
				{
					t1.erase( n ) ;
				}
				t1 += t2 + ruler ;
			}

			ssfield_scale = t1.substr( 0, len ) ;

			return ssfield_scale ;
		}

		bool ssfield_chars_right()
		{
			return ssfield_value.size() > ( ssfield_start + ssfield_fdlen ) ;
		}

		bool ssfield_scroll_right( uint amnt )
		{
			uint dl = ssfield_get_display_len() ;

			if ( ( dl <= ssfield_fdlen ) || ( dl > ssfield_start && ( dl - ssfield_start ) == ssfield_fdlen ) ) { return false ; }

			uint s = ( amnt == 0 ) ? ssfield_fdlen : min( amnt, dl ) ;

			ssfield_start += s ;

			if ( ssfield_start > ( dl - ssfield_fdlen ) )
			{
				ssfield_start = dl - ssfield_fdlen ;
			}

			ssfield_scale = "" ;
			ssfield_sind2 = "" ;

			return true ;
		}

		bool ssfield_scroll_left( uint amnt )
		{
			if ( ssfield_start == 0 ) { return false ; }

			uint dl = ssfield_get_display_len() ;

			uint s = ( amnt == 0 ) ? ssfield_fdlen : min( amnt, dl ) ;

			ssfield_start = ( ssfield_start < s ) ? 0 : ssfield_start - s ;

			ssfield_scale = "" ;
			ssfield_sind2 = "" ;

			return true ;
		}

		bool ssfield_scroll_to_pos( uint p )
		{
			uint i  = ssfield_start ;

			uint dl = ssfield_get_display_len() ;

			if ( p == ssfield_start || dl <= ssfield_fdlen )  { return false ; }

			ssfield_start = min( p, uint( dl - ssfield_fdlen ) ) ;

			if ( ssfield_start != i )
			{
				ssfield_scale = "" ;
				ssfield_sind2 = "" ;
			}

			return ( ssfield_start != i ) ;
		}

		void ssfield_incr_pos()
		{
			++ssfield_start ;
		}

		void ssfield_decr_pos()
		{
			--ssfield_start ;
		}

		bool ssfield_scroll_lr()
		{
			return ( ssfield_sfield->scroll_on && ssfield_sfield->scroll_lr ) ;
		}

		bool ssfield_scroll_on()
		{
			return ssfield_sfield->scroll_on ;
		}

		bool ssfield_scroll_off()
		{
			return !ssfield_sfield->scroll_on ;
		}

		bool ssfield_has_scroll_var()
		{
			return ssfield_sfield->scrollvar ;
		}

		bool ssfield_has_rcol_var()
		{
			return ( ssfield_sfield->rcol != "" ) ;
		}

		const string& ssfield_get_rcol_var()
		{
			return ssfield_sfield->rcol ;
		}

		uint ssfield_get_rcol()
		{
			return ( ssfield_start + ssfield_fdlen ) ;
		}

		bool ssfield_has_lcol_var()
		{
			return ( ssfield_sfield->lcol != "" ) ;
		}

		const string& ssfield_get_lcol_var()
		{
			return ssfield_sfield->lcol ;
		}

		uint ssfield_get_lcol()
		{
			return ( ssfield_start + 1 ) ;
		}

		void ssfield_set_lcol( uint l )
		{
			ssfield_scroll_to_pos( l-1 ) ;
		}

		const string& ssfield_get_scroll_var()
		{
			return ssfield_sfield->scroll ;
		}

		size_t ssfield_get_value_size()
		{
			return ssfield_value.size() ;
		}

		void ssfield_set_scroll_parm( const string& v )
		{
			if ( v == "OFF" || v == "NO" )
			{
				ssfield_sfield->scroll_on = false ;
				if ( ssfield_value.size() > ssfield_fdlen )
				{
					ssfield_value.erase( ssfield_fdlen ) ;
				}
				ssfield_start = 0 ;
			}
			else
			{
				ssfield_sfield->scroll_on = true ;
			}
		}

	friend class pPanel ;
	friend class field ;
} ;


class field
{
	public:
		static char field_paduchar ;
		static bool field_nulls ;
		static uint field_intens ;

	private:
		field()
		{
			field_changed      = false ;
			field_active       = true ;
			field_validname    = false ;
			field_inline_attrs = nullptr ;
			field_char_attrs   = nullptr ;
			field_da_ext       = nullptr ;
			field_tb_ext       = nullptr ;
			field_sf_ext       = nullptr ;
			field_visible      = true ;
			field_area         = 0 ;
		}

		field( errblock&,
		       int,
		       int,
		       map<unsigned char, char_attrs*>,
		       const string&,
		       uint = 0,
		       uint = 0 ) ;

		~field()
		{
			delete field_inline_attrs ;
			delete field_da_ext ;
			delete field_tb_ext ;
			delete field_sf_ext ;
		}

		field( const field& ca )
		{
			field_row        = ca.field_row ;
			field_col        = ca.field_col ;
			field_area_row   = ca.field_area_row ;
			field_area_col   = ca.field_area_col ;
			field_endcol     = ca.field_endcol ;
			field_length     = ca.field_length ;
			field_value      = ca.field_value ;
			field_changed    = ca.field_changed ;
			field_active     = ca.field_active ;
			field_validname  = ca.field_validname ;
			field_char_attrs = ca.field_char_attrs ;
			field_visible    = ca.field_visible ;
			field_area       = ca.field_area ;

			if ( ca.field_inline_attrs )
			{
				field_inline_attrs  = new char_attrs ;
				*field_inline_attrs = *ca.field_inline_attrs ;
			}
			else
			{
				field_inline_attrs = nullptr ;
			}

			field_da_ext = ( ca.field_da_ext ) ? new field_ext1 : nullptr ;
			field_tb_ext = ( ca.field_tb_ext ) ? new field_ext2( ca.field_tb_ext->field_ext2_name ) : nullptr ;
			field_sf_ext = nullptr ;
		}

		field operator = ( const field& rhs )
		{
			field_row        = rhs.field_row ;
			field_col        = rhs.field_col ;
			field_area_row   = rhs.field_area_row ;
			field_area_col   = rhs.field_area_col ;
			field_endcol     = rhs.field_endcol ;
			field_length     = rhs.field_length ;
			field_value      = rhs.field_value ;
			field_changed    = rhs.field_changed ;
			field_active     = rhs.field_active ;
			field_validname  = rhs.field_validname ;
			field_char_attrs = rhs.field_char_attrs ;
			field_visible    = rhs.field_visible ;
			field_area       = rhs.field_area ;

			delete field_inline_attrs ;
			if ( rhs.field_inline_attrs )
			{
				field_inline_attrs  = new char_attrs ;
				*field_inline_attrs = *rhs.field_inline_attrs ;
			}
			else
			{
				field_inline_attrs = nullptr ;
			}

			delete field_da_ext ;
			delete field_tb_ext ;
			delete field_sf_ext ;

			field_da_ext = ( rhs.field_da_ext ) ? new field_ext1 : nullptr ;
			field_tb_ext = ( rhs.field_tb_ext ) ? new field_ext2( rhs.field_tb_ext->field_ext2_name ) : nullptr ;
			field_sf_ext = nullptr ;

			return *this ;
		}

		unsigned int  field_row ;
		unsigned int  field_col ;
		unsigned int  field_area_row ;
		unsigned int  field_area_col ;
		unsigned int  field_endcol ;
		unsigned int  field_length ;
		string        field_value ;
		bool          field_changed ;
		bool          field_active ;
		bool          field_validname ;
		char_attrs*   field_inline_attrs ;
		char_attrs*   field_char_attrs ;
		field_ext1*   field_da_ext ;
		field_ext2*   field_tb_ext ;
		field_ext3*   field_sf_ext ;
		bool          field_visible ;
		unsigned int  field_area ;

		void field_opts( errblock&,
				 attType,
				 const string& ) ;

		void field_reset() ;

		bool cursor_on_field( uint,
				      uint ) ;

		void display_field( WINDOW*,
				    char,
				    map<unsigned char, uint>&,
				    map<unsigned char, uint>& ) ;

		bool edit_field_insert( WINDOW*,
					char,
					char,
					int,
					bool&,
					map<unsigned char, uint>&,
					map<unsigned char, uint>& ) ;

		bool edit_field_replace( WINDOW*,
					 char,
					 char,
					 int,
					 map<unsigned char, uint>&,
					 map<unsigned char, uint>& ) ;

		void edit_field_delete( WINDOW*,
					int,
					char,
					map<unsigned char, uint>&,
					map<unsigned char, uint>& ) ;

		int  edit_field_backspace( WINDOW*,
					   int,
					   char,
					   map<unsigned char, uint>&,
					   map<unsigned char, uint>& ) ;

		void field_remove_nulls_da() ;
		void field_blank( WINDOW* win ) ;
		void field_clear( WINDOW* win ) ;

		void field_erase_eof( WINDOW* win,
				      unsigned int col,
				      char schar,
				      map<unsigned char, uint>&,
				      map<unsigned char, uint>&,
				      bool ) ;

		void field_erase_spaces( WINDOW* win,
					 unsigned int col,
					 char schar,
					 map<unsigned char, uint>&,
					 map<unsigned char, uint>& ) ;

		void field_erase_word( WINDOW* win,
				       unsigned int col,
				       char schar,
				       map<unsigned char, uint>&,
				       map<unsigned char, uint>& ) ;

		bool field_dyna_input( uint ) ;
		int  field_dyna_input_offset( uint ) ;
		int  field_dyna_input_offset_prev( uint ) ;

		void field_update_datamod_usermod( string*,
						   int ) ;
		void field_attr( errblock&,
				 const string&,
				 bool = false ) ;

		void field_set_index( dynArea*,
				      int ) ;
		void field_set_index( int ) ;
		int  field_get_index() ;
		const string& field_get_name() ;

		void field_attr_reset() ;
		bool field_attr_reset_once() ;
		void field_prep_input() ;
		void field_prep_display() ;
		void field_apply_caps() ;
		void field_apply_caps_in() ;
		void field_apply_caps_out() ;
		void field_apply_caps_uncond() ;
		uint field_get_row()      { return field_row ; }

		string& convert_value( string& ) ;

		const string& field_get_value() ;
		uint  field_get_display_len() ;
		void  field_set_len( uint ) ;
		void  field_set_tblen( uint ) ;
		bool  field_update_tblen_max() ;
		bool  field_value_blank() ;
		void  field_put_value( const string& ) ;

		int  end_of_field( WINDOW*,
				   uint ) ;

		int  start_of_field( WINDOW*,
				     uint ) ;

		int  field_next_word( WINDOW*,
				      uint ) ;

		int  field_prev_word( WINDOW*,
				      uint ) ;

		int  field_first_word( WINDOW*,
				       uint ) ;

		int  field_last_word( WINDOW*,
				      uint ) ;

		attType field_get_type() ;

		char field_get_padchar() ;
		char field_get_just() ;
		uint field_get_colour() ;

		void field_add_scroll( sfield* ) ;

		bool field_has_lenvar() ;

		bool field_has_ind() ;
		bool field_has_lind() ;
		bool field_has_rind() ;
		bool field_has_sind() ;
		bool field_has_scale() ;

		const string& field_get_lenvar() ;
		const string& field_get_ind1() ;
		const string& field_get_lind1() ;
		const string& field_get_rind1() ;
		const string& field_get_sind1() ;
		const string& field_get_scale1() ;

		string field_get_ind2() ;
		string field_get_lind2() ;
		string field_get_rind2() ;
		string field_get_sind2( uint ) ;
		const string& field_get_scale2( uint ) ;

		uint field_get_start() ;
		bool field_scroll_right( uint ) ;
		bool field_scroll_left( uint ) ;
		bool field_scroll_to_pos( uint ) ;
		void field_scroll_to_start() ;
		void field_incr_pos() ;
		void field_decr_pos() ;
		void field_scroll_erase_eof( uint ) ;
		void field_scroll_erase_spaces( uint ) ;
		void field_scroll_erase_word( uint ) ;
		bool field_has_scroll_var() ;
		const string& field_get_scroll_var() ;
		bool field_has_rcol_var() ;
		const string& field_get_rcol_var() ;
		uint field_get_rcol() ;
		bool field_has_lcol_var() ;
		const string& field_get_lcol_var() ;
		uint field_get_lcol() ;
		void field_set_lcol( uint ) ;
		void field_set_scroll_parm( const string& ) ;
		bool field_chars_right() ;
		bool field_getch( char& ) ;
		bool field_putch( char ) ;

		size_t field_get_value_size() ;

		void field_update_ssvalue() ;

		void field_update_fdlen() ;

		bool field_is_input() ;
		bool field_is_input_pas() ;
		bool field_is_cua_input() ;
		bool field_is_caps() ;
		bool field_is_caps_in() ;
		bool field_is_caps_out() ;
		bool field_is_skip() ;
		bool field_is_nojump() ;
		bool field_is_paduser() ;
		bool field_is_numeric() ;
		bool field_is_pas() ;
		bool field_is_passwd() ;
		bool field_is_intens_non() ;
		bool field_is_padc() ;
		bool field_is_prot( bool ) ;
		bool field_is_tbdispl() ;
		bool field_is_dynamic() ;
		bool field_is_scrollable() ;
		bool field_scroll_lr() ;
		bool field_scroll_on() ;
		bool field_scroll_off() ;


	friend class pPanel ;
	friend class Area ;
	friend class tbfield ;
} ;


class tbfield
{
	public:
		tbfield()
		{
			tbfield_name    = "" ;
			tbfield_col_var = "" ;
			tbfield_len_var = "" ;
			tbfield_col     = 0  ;
			tbfield_len     = 0  ;
		}

		tbfield( const string& name,
			 const string& varc,
			 const string& varl,
			 uint tcol,
			 uint tlen,
			 const vector<field*>& fields ) : tbfield()
		{
			tbfield_name     = name ;
			tbfield_col_var  = varc ;
			tbfield_len_var  = varl ;
			if ( varc == "" )
			{
				tbfield_col = tcol ;
			}
			if ( varl == "" )
			{
				tbfield_len = tlen ;
			}
			fieldList = fields ;
		}

	private:
		string tbfield_name ;
		string tbfield_col_var ;
		string tbfield_len_var ;

		unsigned int tbfield_col ;
		unsigned int tbfield_len ;

		vector<field*> fieldList ;

		bool set_tbfield( uint,
				  uint ) ;

		void update_fields() ;

	friend class pPanel ;
	friend class field ;
} ;


class text
{
	public:
		static uint text_intens ;

	private:
		text()
		{
			text_name         = "" ;
			text_visible      = true ;
			text_dvars        = true ;
			text_inline_attrs = nullptr ;
			text_char_attrs   = nullptr ;
		}

		text( errblock&,
		      int,
		      int,
		      map<unsigned char, char_attrs*>,
		      uint&,
		      uint&,
		      string,
		      uint = 0,
		      uint = 0 ) ;

		~text()
		{
			delete text_inline_attrs ;
		}

		text( const text& ca )
		{
			text_row        = ca.text_row ;
			text_col        = ca.text_col ;
			text_area_row   = ca.text_area_row ;
			text_area_col   = ca.text_area_col ;
			text_endcol     = ca.text_endcol ;
			text_length     = ca.text_length ;
			text_value      = ca.text_value ;
			text_xvalue     = ca.text_xvalue ;
			text_char_attrs = ca.text_char_attrs ;
			text_visible    = ca.text_visible ;
			text_dvars      = ca.text_dvars ;

			if ( ca.text_inline_attrs )
			{
				text_inline_attrs   = new text_attrs ;
				*text_inline_attrs = *ca.text_inline_attrs ;
			}
			else
			{
				text_inline_attrs = nullptr ;
			}
		}

		text operator = ( const text& rhs )
		{
			text_row        = rhs.text_row ;
			text_col        = rhs.text_col ;
			text_area_row   = rhs.text_area_row ;
			text_area_col   = rhs.text_area_col ;
			text_endcol     = rhs.text_endcol ;
			text_length     = rhs.text_length ;
			text_value      = rhs.text_value ;
			text_char_attrs = rhs.text_char_attrs ;
			text_visible    = rhs.text_visible ;

			delete text_inline_attrs ;
			if ( rhs.text_inline_attrs )
			{
				text_inline_attrs   = new text_attrs ;
				*text_inline_attrs = *rhs.text_inline_attrs ;
			}
			else
			{
				text_inline_attrs = nullptr ;
			}

			return *this ;
		}

		uint   text_row ;
		uint   text_col ;
		uint   text_area_row ;
		uint   text_area_col ;
		uint   text_length ;
		uint   text_endcol ;
		string text_value ;
		string text_xvalue ;
		string text_name ;
		bool   text_visible ;
		bool   text_dvars ;
		text_attrs* text_inline_attrs ;
		char_attrs* text_char_attrs ;

		void text_display( WINDOW* ) ;

		bool cursor_on_text( uint,
				     uint ) ;

		attType get_type() ;

	friend class pPanel ;
	friend class Area ;
} ;


class Area
{
	private:
		Area() :
		si1( "           " ),
		si2( "More:     +" ),
		si3( "More:   -  " ),
		si4( "More:   - +" )
		{
			pos     = 0 ;
			maxRow  = 0 ;
			maxPos  = 0 ;
			covered = 0 ;
			downmsg = false ;
			upmsg   = false ;
			Visible_depth = 0 ;
		}

		Area( errblock&,
		      int,
		      int,
		      uint,
		      const string& ) ;

		void add( text* ) ;

		void add( const string&,
			  field* ) ;

		void get_info( uint&,
			       uint& ) ;

		void get_info( uint&,
			       uint&,
			       uint& ) ;

		void get_info( uint&,
			       uint&,
			       uint&,
			       uint& ) ;

		uint get_width()   { return Area_width ; }
		uint get_depth()   { return Area_depth ; }
		uint get_col()     { return Area_col ; }
		uint get_num()     { return Area_num ; }

		bool not_defined() { return fieldList.empty() && textList.empty() ; }

		void make_visible( field* ) ;

		bool in_visible_area( field* ) ;

		bool cursor_on_area( uint,
				     uint ) ;

		void check_overlapping_fields( errblock&,
					       const string& ) ;

		void update_area() ;

		void tod_issued()  { upmsg   = true  ; }
		void eod_issued()  { downmsg = true  ; }
		void reset_tod()   { upmsg   = false ; }
		void reset_eod()   { downmsg = false ; }
		bool is_tod()      { return upmsg    ; }
		bool is_eod()      { return downmsg  ; }

		bool can_scroll() ;

		int  scroll_up( uint,
				uint ) ;

		int  scroll_down( uint,
				  uint ) ;

		bool scroll_to_top() ;

		void set_visible_depth( uint ) ;

		const char* get_scroll_indicator() ;

		uint pos ;
		uint maxRow ;
		uint maxPos ;
		uint covered ;

		uint Area_num ;
		uint Area_row ;
		uint Area_col ;
		uint Area_width ;
		uint Area_depth ;

		uint Visible_depth ;

		bool upmsg ;
		bool downmsg ;

		const char* si1 ;
		const char* si2 ;
		const char* si3 ;
		const char* si4 ;

		void update_fields() ;
		void update_text() ;

		map<string, field*> fieldList ;
		vector<text*> textList ;

	friend class field ;
	friend class pPanel ;
} ;


class pdc
{
	public:
		static uint pdc_intens ;

		pdc()
		{
			pdc_desc    = "" ;
			pdc_xdesc   = "" ;
			pdc_dvars   = true ;
			pdc_run     = "" ;
			pdc_parm    = "" ;
			pdc_unavail = "" ;
			pdc_inact   = true ;
		}

		pdc( const string& a,
		     const string& b,
		     const string& c,
		     const string& d )
		{
			pdc_desc    = a ;
			pdc_xdesc   = "" ;
			pdc_dvars   = true ;
			pdc_run     = b ;
			pdc_parm    = c ;
			pdc_unavail = d ;
			pdc_inact   = false ;
		}

		string pdc_desc ;
		string pdc_xdesc ;
		bool   pdc_dvars ;
		string pdc_run ;
		string pdc_parm ;
		string pdc_unavail ;
		bool   pdc_inact ;

		void   display_pdc_avail( WINDOW*,
					  size_t,
					  attType,
					  int ) ;

		void   display_pdc_unavail( WINDOW*,
					    size_t,
					    attType,
					    int ) ;
} ;


class abc
{
	public:
		static uint abc_intens ;
		static poolMGR* p_poolMGR ;

		abc()
		{
			abc_maxh   = 0 ;
			abc_maxw   = 0 ;
			currChoice = 0 ;
			choiceVar  = "" ;
			pd_created = false ;
		}

		abc( fPOOL* p, bool b )
		{
			funcPool   = p ;
			selPanel   = b ;
			abc_maxh   = 0 ;
			abc_maxw   = 0 ;
			currChoice = 0 ;
			abc_mnem1  = 0 ;
			abc_row1   = 0 ;
			abc_col1   = 0 ;
			abc_col    = 0 ;
			choiceVar  = "" ;
			pd_created = false ;
		}

		~abc()
		{
			if ( pd_created )
			{
				del_panel( panel ) ;
				delwin( win ) ;
			}
		}

		string       abc_desc ;
		char         abc_mnem2 ;
		unsigned int abc_mnem1 ;
		unsigned int abc_row1 ;
		unsigned int abc_col1 ;
		unsigned int abc_col ;
		unsigned int abc_maxh ;
		unsigned int abc_maxw ;

		void   add_pdc( const pdc& ) ;
		void   display_abc_sel( WINDOW* ) ;
		void   display_abc_unsel( WINDOW*,
					  bool ) ;
		void   display_pd( errblock&,
				   uint,
				   uint,
				   const string&,
				   uint,
				   uint,
				   uint,
				   uint = 1 ) ;
		void   hide_pd() ;
		uint   get_pd_col()    { return abc_col  ; }
		uint   get_pd_col1()   { return abc_col1 ; }
		uint   get_pd_row1()   { return abc_row1 ; }
		uint   get_pd_rows()   { return pdcList.size() ; }
		void   get_msg_position( uint&,
					 uint&,
					 uint ) ;
		void   get_pd_home( uint&,
				    uint& ) ;
		const  string& get_abc_desc() ;
		pdc    retrieve_choice( errblock& ) ;
		int    retrieve_choice_number() ;
		bool   cursor_on_abc( uint ) ;
		bool   cursor_on_pulldown( uint,
					   uint ) ;

	private:
		fPOOL* funcPool ;
		bool   pd_created ;
		bool   selPanel ;
		int    currChoice ;
		string choiceVar ;

		void   putDialogueVar( errblock&,
				       const string&,
				       const string& ) ;

		string getDialogueVar( errblock&,
				       const string& ) ;

		string sub_vars( errblock&,
				 string,
				 bool& ) ;

		void   create_window( uint,
				      uint ) ;

		vector<pdc> pdcList ;

		WINDOW* win ;
		PANEL* panel ;
} ;


class Box
{
	public:
		static uint box_intens ;

	private:
		Box()
		{
			box_title = "" ;
		}

		Box( errblock&,
		     int,
		     int,
		     const string& ) ;

		void display_box( WINDOW*,
				  string ) ;

		void display_box( WINDOW*,
				  string,
				  uint,
				  uint ) ;

		string box_title ;
		uint box_row ;
		uint box_col ;
		uint box_width ;
		uint box_depth ;
		uint box_colour ;

		void draw_box( WINDOW*,
			       string& ) ;

	friend class pPanel ;
} ;

