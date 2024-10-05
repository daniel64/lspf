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

using namespace lspfc ;

/******************************************************************************************************************/
/*                                               FIELD CLASS                                                      */
/******************************************************************************************************************/

field::field( errblock& err,
	      int MAXW,
	      int MAXD,
	      map<unsigned char, char_attrs*> char_attrlist,
	      const string& line,
	      uint Area_num,
	      uint Area_col ) : field()
{
	//
	// Format of field entry in panels (FORMAT 1 VERSION 1 ).
	// FIELD row col len cuaAttr opts field_name
	// w1    w2  w3  w4  w5      w6   w7
	//
	// FIELD row col len Attr    field_name
	// w1    w2  w3  w4  ATTR(+) w6
	//
	// FIELD 3  14  90  NEF CAPS(On),pad('_'),just(left),numeric(off),skip(on),nojump(on) ZCMD
	//
	// )ATTR section TYPE(INPUT|OUTPUT|cua) character can be used in place of a CUA attribute.  In this
	// case, options are not allowed.
	//
	// Valid CUA attributes for FIELD statement:
	// CEF, EE, LEF, NEF, VOI, LID, LI, SC
	//   Unprotected:
	//     EE, LEF, CEF, NEF
	//   Protected:
	//     VOI, LID, LI, SC
	//

	int row ;
	int col ;
	int len ;
	int ws  ;

	char field_char ;

	bool inlne = false ;

	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;
	string t  ;
	string opts ;
	string rest ;
	string name ;

	attType fType ;

	err.setRC( 0 ) ;

	ws = words( line ) ;

	w2 = word( line, 2 ) ;
	w3 = word( line, 3 ) ;
	w4 = word( line, 4 ) ;
	w5 = word( line, 5 ) ;

	opts = subword( line, 6, ws-6 ) ;
	name = word( line, ws ) ;
	err.setUserData( name ) ;

	if ( Area_num > 0 && ( w2 == "MAX" || w2.compare( 0, 4, "MAX-" ) == 0 ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE031U" ) ;
		return ;
	}

	if ( isnumeric( w2 ) )                      { row = ds2d( w2 ) ; }
	else if ( w2 == "MAX" )                     { row = MAXD       ; }
	else if ( w2.compare( 0, 4, "MAX-" ) == 0 ) { row = MAXD - ds2d( substr( w2, 5 ) )    ; }
	else                                        { err.seterrid( TRACE_INFO(), "PSYE031S", w2 ) ; return ; }

	if ( isnumeric( w3 ) )                      { col = ds2d( w3 ) ; }
	else if ( w3 == "MAX" )                     { col = MAXW       ; }
	else if ( w3.compare( 0, 4, "MAX-" ) == 0 ) { col = MAXW - ds2d( substr( w3, 5 ) )    ; }
	else                                        { err.seterrid( TRACE_INFO(), "PSYE031S", w3 ) ; return ; }

	if ( isnumeric( w4 ) )                      { len = ds2d( w4 )     ; }
	else if ( w4 == "MAX" )                     { len = MAXW - col + 1 ; }
	else if ( w4.compare( 0, 4, "MAX-" ) == 0 ) { len = MAXW - col - ds2d( substr( w4, 5 ) ) + 1 ; }
	else                                        { err.seterrid( TRACE_INFO(), "PSYE031S", w4 ) ; return ; }

	if ( Area_num == 0 && row > MAXD )
	{
		err.seterrid( TRACE_INFO(), "PSYE031A", name, d2ds( row ), d2ds( MAXD ) ) ;
		return ;
	}

	if ( col > MAXW )
	{
		err.seterrid( TRACE_INFO(), "PSYE031B", name, d2ds( col ), d2ds( MAXW ) ) ;
		return ;
	}

	if ( cuaAttrName.count( w5 ) == 0 )
	{
		rest = subword( line, 5 ) ;
		if ( rest.compare( 0, 5, "ATTR(" ) != 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE032F", w5 ) ;
			return ;
		}
		t = parseString1( err, rest, "ATTR()" ) ;
		if ( err.error() ) { return ; }
		if ( words( rest ) > 1 )
		{
			err.seterrid( TRACE_INFO(), "PSYE032H", rest ) ;
			return ;
		}
		if ( t == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE035S" ) ;
			return ;
		}
		if ( t.size() > 2 || ( t.size() == 2 && not ishex( t ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE041B" ) ;
			return ;
		}
		field_char = ( t.size() == 2 ) ? xs2cs( t ).front() : t.front() ;
		auto it = char_attrlist.find( field_char ) ;
		if ( it == char_attrlist.end() )
		{
			t = isprint( field_char ) ? string( 1, field_char ) : c2xs( field_char ) ;
			err.seterrid( TRACE_INFO(), "PSYE036D", t ) ;
			return ;
		}
		if ( it->second->is_text() )
		{
			err.seterrid( TRACE_INFO(), "PSYE035T" ) ;
			return ;
		}
		field_char_attrs = it->second ;
	}
	else
	{
		if ( ws < 7 )
		{
			err.seterrid( TRACE_INFO(), "PSYE035Q" ) ;
			return ;
		}
		if ( !findword( w5, "CEF EE LEF NEF VOI LID LI SC" ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE035R", w5 ) ;
			return ;
		}
		fType = cuaAttrName[ w5 ] ;
		inlne = true ;
	}

	if ( ( col + len - 1 ) > MAXW )
	{
		err.seterrid( TRACE_INFO(), "PSYE031X", name ) ;
		return ;
	}

	field_row    = row-1 ;
	field_col    = col-1 ;
	field_length = len ;

	if ( Area_num > 0 )
	{
		field_area_row  = field_row ;
		field_area_col  = field_col ;
		field_col      += Area_col ;
		field_visible   = false ;
	}

	field_endcol    = field_col + field_length - 1 ;
	field_validname = isvalidName( err.getUserData() ) ;

	if ( inlne )
	{
		field_opts( err, fType, opts ) ;
		if ( err.error() ) { return ; }
	}
}


void field::field_opts( errblock& err,
			attType type,
			const string& opts )
{
	delete field_inline_attrs ;

	field_inline_attrs = new char_attrs ;

	field_inline_attrs->set_cuatype( type ) ;

	field_inline_attrs->parse_inline( err, opts ) ;
}


char field::field_get_padchar()
{
	return ( field_inline_attrs ) ? field_inline_attrs->get_padchar() :
	       ( field_char_attrs )   ? field_char_attrs->get_padchar()   : ' ' ;
}


attType field::field_get_type()
{
	return ( field_inline_attrs ) ? field_inline_attrs->get_type() :
	       ( field_char_attrs )   ? field_char_attrs->get_type()   : NONE ;
}


char field::field_get_just()
{
	//
	// Scrollable fields are JUST ASIS unless at the start of the field and no characters to the right.
	//
	char just = ( field_inline_attrs ) ? field_inline_attrs->get_just( field_tb_ext ) :
		    ( field_char_attrs )   ? field_char_attrs->get_just( field_tb_ext )   : 'L' ;

	return ( field_scroll_on() && field_sf_ext->ssfield_at_beginning() && just == 'L' && !field_sf_ext->ssfield_chars_right() ) ? 'L' :
	       ( field_scroll_on()  ) ? 'A' : just ;
}


uint field::field_get_colour()
{
	return ( field_inline_attrs ) ? field_inline_attrs->get_colour() :
	       ( field_char_attrs )   ? field_char_attrs->get_colour()   : RED ;
}


bool field::field_is_input()
{
	return ( field_inline_attrs ) ? field_inline_attrs->is_input() :
	       ( field_char_attrs )   ? field_char_attrs->is_input()   : false ;
}


bool field::field_is_cua_input()
{
	return ( field_inline_attrs ) ? field_inline_attrs->is_cua_input() :
	       ( field_char_attrs )   ? field_char_attrs->is_cua_input()   : false ;
}


bool field::field_is_input_pas()
{
	return ( field_inline_attrs ) ? field_inline_attrs->is_input_pas() :
	       ( field_char_attrs )   ? field_char_attrs->is_input_pas()   : false ;
}


bool field::field_is_caps()
{
	return ( field_inline_attrs ) ? field_inline_attrs->is_caps( field_tb_ext ) :
	       ( field_char_attrs )   ? field_char_attrs->is_caps( field_tb_ext )   : false ;
}


bool field::field_is_caps_in()
{
	return ( field_inline_attrs ) ? field_inline_attrs->is_caps_in( field_tb_ext ) :
	       ( field_char_attrs )   ? field_char_attrs->is_caps_in( field_tb_ext )   : false ;
}


bool field::field_is_caps_out()
{
	return ( field_inline_attrs ) ? field_inline_attrs->is_caps_out( field_tb_ext ) :
	       ( field_char_attrs )   ? field_char_attrs->is_caps_out( field_tb_ext )   : false ;
}


bool field::field_is_skip()
{
	return ( field_inline_attrs ) ? field_inline_attrs->is_skip() :
	       ( field_char_attrs )   ? field_char_attrs->is_skip()   : false ;
}


bool field::field_is_numeric()
{
	return ( field_inline_attrs ) ? field_inline_attrs->is_numeric() :
	       ( field_char_attrs )   ? field_char_attrs->is_numeric()   : false ;
}


bool field::field_is_nojump()
{
	return ( field_inline_attrs ) ? field_inline_attrs->is_nojump() :
	       ( field_char_attrs )   ? field_char_attrs->is_nojump()   : false ;
}


bool field::field_is_paduser()
{
	return ( field_inline_attrs ) ? field_inline_attrs->is_paduser() :
	       ( field_char_attrs )   ? field_char_attrs->is_paduser()   : false ;
}


bool field::field_is_pas()
{
	return ( field_inline_attrs ) ? field_inline_attrs->is_pas() :
	       ( field_char_attrs )   ? field_char_attrs->is_pas()   : false ;
}


bool field::field_is_passwd()
{
	return ( field_inline_attrs ) ? field_inline_attrs->is_passwd() :
	       ( field_char_attrs )   ? field_char_attrs->is_passwd()   : false ;
}


bool field::field_is_intens_non()
{
	return ( field_inline_attrs ) ? field_inline_attrs->is_intens_non() :
	       ( field_char_attrs )   ? field_char_attrs->is_intens_non()   : false ;
}


bool field::field_is_prot( bool tabpas )
{
	return ( tabpas ) ? !field_is_input_pas() : !field_is_input() ;
}


bool field::field_is_padc()
{
	return ( field_inline_attrs ) ? field_inline_attrs->is_padc() :
	       ( field_char_attrs )   ? field_char_attrs->is_padc()   : false ;
}


bool field::field_is_scrollable()
{
	return ( field_sf_ext ) ;
}


bool field::field_is_tbdispl()
{
	return ( field_tb_ext ) ;
}


bool field::field_is_dynamic()
{
	return ( field_da_ext ) ;
}


bool field::field_scroll_lr()
{
	return ( field_sf_ext && field_sf_ext->ssfield_scroll_lr() ) ;
}


bool field::field_scroll_on()
{
	return ( field_sf_ext && field_sf_ext->ssfield_scroll_on() ) ;
}


bool field::field_scroll_off()
{
	return ( !field_sf_ext || field_sf_ext->ssfield_scroll_off() ) ;
}


void field::field_add_scroll( sfield* s )
{
	field_sf_ext = new field_ext3( s, field_length ) ;
}


bool field::field_has_lenvar()
{
	return ( field_sf_ext->ssfield_has_lenvar() ) ;
}


bool field::field_has_ind()
{
	return ( field_sf_ext->ssfield_has_ind() ) ;
}


bool field::field_has_lind()
{
	return ( field_sf_ext->ssfield_has_lind() ) ;
}


bool field::field_has_rind()
{
	return ( field_sf_ext->ssfield_has_rind() ) ;
}


bool field::field_has_sind()
{
	return ( field_sf_ext->ssfield_has_sind() ) ;
}


bool field::field_has_scale()
{
	return ( field_sf_ext->ssfield_has_scale() ) ;
}


const string& field::field_get_lenvar()
{
	return ( field_sf_ext->ssfield_get_lenvar() ) ;
}


const string& field::field_get_ind1()
{
	return ( field_sf_ext->ssfield_get_ind1() ) ;
}


string field::field_get_ind2()
{
	return ( field_sf_ext->ssfield_get_ind2() ) ;
}


const string& field::field_get_lind1()
{
	return ( field_sf_ext->ssfield_get_lind1() ) ;
}


string field::field_get_lind2()
{
	return ( field_sf_ext->ssfield_get_lind2() ) ;
}


const string& field::field_get_rind1()
{
	return ( field_sf_ext->ssfield_get_rind1() ) ;
}


string field::field_get_rind2()
{
	return ( field_sf_ext->ssfield_get_rind2() ) ;
}


const string& field::field_get_sind1()
{
	return ( field_sf_ext->ssfield_get_sind1() ) ;
}


string field::field_get_sind2( uint l )
{
	return ( field_sf_ext->ssfield_get_sind2( l ) ) ;
}


const string& field::field_get_scale1()
{
	return ( field_sf_ext->ssfield_get_scale1() ) ;
}


const string& field::field_get_scale2( uint l )
{
	return ( field_sf_ext->ssfield_get_scale2( l ) ) ;
}


uint field::field_get_start()
{
	return ( field_sf_ext->ssfield_get_start() ) ;
}


bool field::field_scroll_right( uint l )
{
	if ( field_changed && field_is_input() )
	{
		field_prep_input() ;
	}

	bool result = ( field_sf_ext->ssfield_scroll_right( l ) ) ;

	field_value = field_sf_ext->ssfield_get_display_value() ;

	return result ;
}


bool field::field_scroll_left( uint l )
{
	if ( field_changed && field_is_input() )
	{
		field_prep_input() ;
	}

	bool result = ( field_sf_ext->ssfield_scroll_left( l ) ) ;

	field_value = field_sf_ext->ssfield_get_display_value() ;

	return result ;
}


bool field::field_scroll_to_pos( uint p )
{
	bool scrolled = field_sf_ext->ssfield_scroll_to_pos( p ) ;

	if ( scrolled )
	{
		field_value = field_sf_ext->ssfield_get_display_value() ;
	}

	return scrolled ;
}


void field::field_scroll_to_start()
{
	field_scroll_to_pos( 0 ) ;
}


void field::field_incr_pos()
{
	field_update_ssvalue() ;
	field_sf_ext->ssfield_incr_pos() ;
	field_value = field_sf_ext->ssfield_get_display_value() ;
}


void field::field_decr_pos()
{
	field_update_ssvalue() ;
	field_sf_ext->ssfield_decr_pos() ;
	field_value = field_sf_ext->ssfield_get_display_value() ;
}


void field::field_scroll_erase_eof( uint p )
{
	field_update_ssvalue() ;
	field_sf_ext->ssfield_erase_eof( p ) ;
	field_value = field_sf_ext->ssfield_get_display_value() ;
}


void field::field_scroll_erase_spaces( uint p )
{
	field_update_ssvalue() ;
	field_sf_ext->ssfield_erase_spaces( p ) ;
	field_value = field_sf_ext->ssfield_get_display_value() ;
}


void field::field_scroll_erase_word( uint p )
{
	field_update_ssvalue() ;
	field_sf_ext->ssfield_erase_word( p ) ;
	field_value = field_sf_ext->ssfield_get_display_value() ;
}


bool field::field_has_scroll_var()
{
	return field_sf_ext->ssfield_has_scroll_var() ;
}


const string& field::field_get_scroll_var()
{
	return field_sf_ext->ssfield_get_scroll_var() ;
}


bool field::field_has_rcol_var()
{
	return field_sf_ext->ssfield_has_rcol_var() ;
}


const string& field::field_get_rcol_var()
{
	return field_sf_ext->ssfield_get_rcol_var() ;
}


uint field::field_get_rcol()
{
	return field_sf_ext->ssfield_get_rcol() ;
}


bool field::field_has_lcol_var()
{
	return field_sf_ext->ssfield_has_lcol_var() ;
}


const string& field::field_get_lcol_var()
{
	return field_sf_ext->ssfield_get_lcol_var() ;
}


uint field::field_get_lcol()
{
	return field_sf_ext->ssfield_get_lcol() ;
}


void field::field_set_lcol( uint l )
{
	field_sf_ext->ssfield_set_lcol( l ) ;
	field_value = field_sf_ext->ssfield_get_display_value() ;
}


void field::field_set_scroll_parm( const string& v )
{
	bool old_scroll = field_scroll_on() ;

	field_sf_ext->ssfield_set_scroll_parm( v ) ;

	if ( old_scroll != field_scroll_on() )
	{
		field_changed = true ;
	}
}


bool field::field_chars_right()
{
	return ( field_sf_ext && field_sf_ext->ssfield_chars_right() ) ;
}


bool field::field_getch( char& c )
{
	return field_sf_ext->ssfield_getch( c ) ;
}


bool field::field_putch( char c )
{
	return field_sf_ext->ssfield_putch( c ) ;
}


void field::field_update_ssvalue()
{
	field_sf_ext->ssfield_update_value( field_value ) ;
}


size_t field::field_get_value_size()
{
	return field_sf_ext->ssfield_get_value_size() ;
}


void field::field_update_fdlen()
{
	field_sf_ext->ssfield_update_fdlen( field_length ) ;
}


void field::field_put_value( const string& s )
{
	if ( field_is_scrollable() )
	{
		field_sf_ext->ssfield_put_value( s ) ;
		field_value = field_sf_ext->ssfield_get_display_value() ;
	}
	else
	{
		field_value = s ;
	}
}


const string& field::field_get_value()
{
	return ( field_scroll_off() ) ? field_value : field_sf_ext->ssfield_get_value()  ;
}


uint field::field_get_display_len()
{
	return ( field_scroll_off() ) ? field_value.size() : field_sf_ext->ssfield_get_display_len() ;
}


void field::field_set_tblen( uint l )
{
	field_sf_ext->ssfield_set_tblen( l ) ;
}


bool field::field_update_tblen_max()
{
	return field_sf_ext->ssfield_update_tblen_max() ;
}


void field::field_set_len( uint l )
{
	field_sf_ext->ssfield_set_len( l ) ;
}


bool field::field_value_blank()
{
	return ( field_scroll_off() ) ? ( field_value == "" ) : field_sf_ext->ssfield_value_is_blank() ;
}


void field::field_set_index( dynArea* da,
			     int index )
{
	field_da_ext->field_ext1_set( da, index ) ;
}


void field::field_set_index( int index )
{
	field_tb_ext->field_ext2_set( index ) ;
}


int field::field_get_index()
{
	return ( field_da_ext ) ? field_da_ext->field_ext1_index : field_tb_ext->field_ext2_index ;
}


const string& field::field_get_name()
{
	return ( field_da_ext ) ? field_da_ext->field_ext1_dynArea->dynArea_name : field_tb_ext->field_ext2_name ;
}


void field::field_reset()
{
	//
	// Clear the list of touched attribute positions before a panel display.
	//

	field_da_ext->field_ext1_usermod.clear() ;
}


bool field::edit_field_insert( WINDOW* win,
			       char ch,
			       char inv_schar,
			       int col,
			       bool& prot,
			       map<unsigned char, uint>& ddata_map,
			       map<unsigned char, uint>& schar_map )
{
	//
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_inAttrs is
	// non-blank and there is an input attribute byte at the start of the field.
	//
	// Use the pad character or nulls to fill fields when the cursor is past the end if the field value.
	// Nulls are removed from the field before further processing by the application.
	//

	uint pos ;

	size_t p1 ;
	size_t p2 ;
	size_t p3 ;

	bool isattr = true ;

	dynArea* da ;

	const char nulls( 0x00 ) ;

	char upad = field_is_paduser() ? field_paduchar : field_get_padchar() ;

	pos = (col - field_col ) ;
	if ( pos >= field_value.size() )
	{
		field_value.resize( pos+1, ( upad == ' ' ) ? ' ' : nulls ) ;
	}

	if ( field_da_ext )
	{
		da = field_da_ext->field_ext1_dynArea ;
		p2 = field_value.find_first_of( da->dynArea_Attrs, pos ) ;
		if ( p2 == string::npos )
		{
			isattr = false ;
			p2 = field_value.size() ;
		}
		--p2 ;
		p3 = field_value.find( nulls, pos ) ;
		if ( !isattr && p3 == string::npos && field_da_ext->field_ext1_has_overflow() &&
		     ( field_da_ext->field_ext1_overflow_value != "" || field_value.back() != ' ' ) )
		{
			if ( !field_da_ext->field_ext1_putch( field_value.back(), field_da_ext->field_ext1_shadow.back() ) )
			{
				return false ;
			}
			field_value.pop_back() ;
			field_da_ext->field_ext1_shadow.pop_back() ;
		}
		else if ( p3 != string::npos && p3 <= p2 )
		{
			field_value.erase( p3, 1 ) ;
			field_da_ext->field_ext1_shadow.erase( p3, 1 ) ;
		}
		else if ( field_value[ p2 ] != ' ' )
		{
			return false ;
		}
		else
		{
			field_value.erase( p2, 1 ) ;
			field_da_ext->field_ext1_shadow.erase( p2, 1 ) ;
		}
		p1 = field_value.find_last_of( da->dynArea_inAttrs, pos ) ;
		field_da_ext->field_ext1_usermod.insert( p1 ) ;
		field_value.insert( pos, 1, ch ) ;
		field_da_ext->field_ext1_shadow.insert( pos, 1, inv_schar ) ;
	}
	else
	{
		if ( field_is_scrollable() )
		{
			field_value.resize( field_length, ' ' ) ;
			if ( pos == field_length - 1 )
			{
				if ( !field_putch( field_value.back() ) )
				{
					return false ;
				}
				field_value[ pos ] = ch ;
				field_incr_pos() ;
				display_field( win, inv_schar, ddata_map, schar_map ) ;
				prot          = false ;
				field_changed = true ;
				return false ;
			}
			if ( !field_putch( field_value[ field_length - 1 ] ) )
			{
				return false ;
			}
		}
		else if ( field_length == field_value.size() )
		{
			return false ;
		}
		p3 = field_value.find( nulls, pos ) ;
		if ( p3 != string::npos )
		{
			field_value.erase( p3, 1 ) ;
		}
		field_value.insert( pos, 1, ch ) ;
	}

	display_field( win, inv_schar, ddata_map, schar_map ) ;
	field_changed = true ;

	return true ;
}


bool field::edit_field_replace( WINDOW* win,
				char ch,
				char inv_schar,
				int col,
				map<unsigned char, uint>& ddata_map,
				map<unsigned char, uint>& schar_map )
{
	//
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_inAttrs is
	// non-blank and there is an input attribute byte at the start of the field.
	//
	// Use the pad character or nulls to fill fields when the cursor is past the end if the field value.
	// These are then removed before further processing by the application.
	//

	size_t p1 ;

	uint pos ;

	const char nulls( 0x00 ) ;

	char upad = field_is_paduser() ? field_paduchar : field_get_padchar() ;

	pos = ( col - field_col ) ;
	if ( pos >= field_value.size() )
	{
		field_value.resize( pos+1, ( upad == ' ' ) ? ' ' : nulls ) ;
	}

	if ( field_da_ext )
	{
		if ( field_value[ pos ] != nulls && !isprint( field_value[ pos ] ) )
		{
			return false ;
		}
		p1 = field_value.find_last_of( field_da_ext->field_ext1_dynArea->dynArea_inAttrs, pos ) ;
		field_da_ext->field_ext1_usermod.insert( p1 ) ;
		field_da_ext->field_ext1_shadow[ pos ] = inv_schar ;
	}
	field_value[ pos ] = ch ;

	display_field( win, inv_schar, ddata_map, schar_map ) ;
	field_changed = true ;

	return true ;
}


void field::edit_field_delete( WINDOW* win,
			       int col,
			       char inv_schar,
			       map<unsigned char, uint>& ddata_map,
			       map<unsigned char, uint>& schar_map )
{
	//
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_inAttrs is
	// non-blank and there is an input attribute byte at the start of the field.
	//

	uint pos ;

	char c1 ;
	char c2 ;

	size_t p1 ;
	size_t p2 ;

	const char nulls( 0x00 ) ;

	bool isattr = true ;

	dynArea* da ;

	pos = col - field_col ;

	if ( field_chars_right() )
	{
		field_value.resize( field_length, ' ' ) ;
	}
	else if ( pos >= field_value.size() )
	{
		return ;
	}

	if ( field_value[ pos ] != nulls && !isprint( field_value[ pos ] ) ) { return ; }

	if ( field_da_ext )
	{
		da = field_da_ext->field_ext1_dynArea ;
		p1 = field_value.find_last_of( da->dynArea_inAttrs, pos ) ;
		field_da_ext->field_ext1_usermod.insert( p1 ) ;
		p2 = field_value.find_first_of( da->dynArea_Attrs, pos )  ;
		if ( p2 == string::npos )
		{
			p2     = field_value.size() ;
			isattr = false ;
		}
		if ( !isattr && field_da_ext->field_ext1_getch( c1, c2 ) )
		{
			field_value.push_back( c1 ) ;
			field_da_ext->field_ext1_shadow.push_back( c2 ) ;
		}
		else
		{
			field_value.insert( p2, 1, ' ' ) ;
			field_da_ext->field_ext1_shadow.insert( p2, 1, 0xFE ) ;
		}
		field_da_ext->field_ext1_shadow.erase( pos, 1 ) ;
	}

	field_value.erase( pos, 1 ) ;
	field_blank( win ) ;

	if ( field_scroll_on() && field_getch( c1 ) )
	{
		field_value.resize( field_length, nulls ) ;
		p1 = field_value.find_last_not_of( nulls ) ;
		if ( p1 == string::npos )
		{
			field_value = string( field_length, ' ' ) ;
		}
		else if ( p1 < ( field_length - 1 ) )
		{
			field_value.replace( p1+1, field_length-p1-1, field_length-p1-1, ' ' )  ;
		}
		field_value[ field_length - 1 ] = c1 ;
	}

	display_field( win, inv_schar, ddata_map, schar_map ) ;
	field_changed = true ;
}


int field::edit_field_backspace( WINDOW* win,
				 int col,
				 char inv_schar,
				 map<unsigned char, uint>& ddata_map,
				 map<unsigned char, uint>& schar_map )
{
	//
	// If this is a dynamic area, we know it is an input field so pos > 0 (to allow for the input attribute byte).
	//

	size_t pos ;

	pos = col - field_col ;

	if ( field_chars_right() )
	{
		field_value.resize( field_length, ' ' ) ;
	}
	else if ( pos > field_value.size() )
	{
		return --col ;
	}

	if ( field_da_ext && field_da_ext->field_ext1_dynArea->dynArea_Attrs.find( field_value[ pos-1 ] ) != string::npos )
	{
		return col ;
	}

	--col ;

	if ( pos == 0 && field_scroll_on() )
	{
		field_decr_pos() ;
		++col ;
	}

	edit_field_delete( win, col, inv_schar, ddata_map, schar_map ) ;

	return col ;
}


void field::field_erase_eof( WINDOW* win,
			     uint col,
			     char inv_schar,
			     map<unsigned char, uint>& ddata_map,
			     map<unsigned char, uint>& schar_map,
			     bool end_os )
{
	//
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_inAttrs is
	// non-blank and there is an input attribute byte at the start of the field.
	//

	size_t pos ;
	size_t p1 ;
	size_t p2 ;

	dynArea* da ;

	if ( ( field_col + field_value.size() ) < col ) return ;

	pos = ( col - field_col ) ;

	if ( field_da_ext )
	{
		da = field_da_ext->field_ext1_dynArea ;
		p1 = field_value.find_last_of( da->dynArea_inAttrs, pos ) ;
		field_da_ext->field_ext1_usermod.insert( p1 ) ;
		p2 = field_value.find_first_of( da->dynArea_Attrs, pos ) ;
		if ( p2 == string::npos )
		{
			p2 = field_value.size() ;
			if ( end_os && field_da_ext->field_ext1_has_overflow() )
			{
				field_da_ext->field_ext1_clear_input( da->dynArea_Attrs ) ;
			}
		}
		field_value.replace( pos, p2-pos, p2-pos, ' ' )  ;
		field_da_ext->field_ext1_shadow.replace( pos, p2-pos, p2-pos, 0xFE ) ;
	}
	else
	{
		field_blank( win ) ;
		if ( end_os && field_scroll_on() )
		{
			field_scroll_erase_eof( pos ) ;
		}
		else if ( pos < field_value.size() )
		{
			field_value.erase( pos ) ;
		}
	}

	display_field( win, inv_schar, ddata_map, schar_map ) ;
	field_changed = true ;
}


void field::field_erase_spaces( WINDOW* win,
				uint col,
				char inv_schar,
				map<unsigned char, uint>& ddata_map,
				map<unsigned char, uint>& schar_map )
{
	//
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_inAttrs is
	// non-blank and there is an input attribute byte at the start of the field.
	//

	size_t pos ;
	size_t p1 ;
	size_t p2 ;

	if ( ( field_col + field_value.size() ) < col ) return ;

	pos = ( col - field_col ) ;

	if ( field_da_ext )
	{
		p2 = field_value.find_first_not_of( ' ', pos ) ;
		if ( p2 == string::npos ) { p2 = field_value.size() ; }
		while ( p2 > pos )
		{
			edit_field_delete( win, ( field_col + pos ), inv_schar, ddata_map, schar_map ) ;
			--p2 ;
		}
	}
	else
	{
		field_blank( win ) ;
		if ( field_scroll_on() )
		{
			field_scroll_erase_spaces( pos ) ;
		}
		else if ( pos < field_value.size() )
		{
			p1 = field_value.find_first_not_of( ' ', pos ) ;
			if ( p1 != string::npos )
			{
				field_value.erase( pos, ( p1 - pos ) ) ;
			}
			else
			{
				field_value.erase( pos ) ;
			}
		}
	}

	display_field( win, inv_schar, ddata_map, schar_map ) ;
	field_changed = true ;
}


void field::field_erase_word( WINDOW* win,
			      uint col,
			      char inv_schar,
			      map<unsigned char, uint>& ddata_map,
			      map<unsigned char, uint>& schar_map )
{
	//
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_inAttrs is
	// non-blank and there is an input attribute byte at the start of the field.
	//
	// When deleting a word, leading spaces are kept but trailing spaces are removed.
	//

	size_t pos = ( col - field_col ) ;

	dynArea* da ;

	size_t p1 ;
	size_t p2 ;

	if ( field_da_ext )
	{
		da = field_da_ext->field_ext1_dynArea ;
		string wspace = " " + da->dynArea_Attrs ;
		if ( wspace.find_first_of( field_value[ pos ] ) != string::npos )
		{
			pos = field_value.find_first_not_of( wspace, pos ) ;
		}
		else
		{
			pos = field_value.find_last_of( wspace, pos ) ;
			if ( pos != string::npos )
			{
				++pos ;
			}
		}
		if ( pos != string::npos )
		{
			while ( wspace.find_first_of( field_value[ pos ] ) == string::npos )
			{
				edit_field_delete( win, (field_col + pos), inv_schar, ddata_map, schar_map ) ;
			}
			p1 = field_value.find_first_of( da->dynArea_Attrs, pos ) ;
			p2 = field_value.find_first_not_of( ' ', pos ) ;
			if ( ( p1 == string::npos && p2 != string::npos ) || ( p2 < p1 ) )
			{
				while ( field_value[ pos ] == ' ' )
				{
					edit_field_delete( win, (field_col + pos), inv_schar, ddata_map, schar_map ) ;
				}
			}
		}
	}
	else
	{
		field_blank( win ) ;
		if ( field_scroll_on() )
		{
			field_scroll_erase_word( pos ) ;
			field_changed = true ;
		}
		else if ( pos < field_value.size() )
		{
			pos = field_value.find_first_not_of( ' ', pos ) ;
			if ( pos != string::npos )
			{
				idelword( field_value, words( field_value.substr( 0, pos+1 ) ), 1 ) ;
				field_changed = true ;
			}
		}
	}

	display_field( win, inv_schar, ddata_map, schar_map ) ;
}


void field::field_blank( WINDOW* win )
{
	const char nulls( 0x00 ) ;

	char* blanks = new char[ field_length+1 ] ;

	char upad = field_is_paduser() ? field_paduchar : field_get_padchar() ;

	if ( upad == nulls )
	{
		upad = ' ' ;
	}

	for ( unsigned int i = 0 ; i < field_length ; ++i )
	{
		blanks[ i ] = upad ;
	}

	blanks[ field_length ] = nulls ;
	wstandend( win ) ;
	mvwaddstr( win, field_row, field_col, blanks ) ;

	delete[] blanks ;
}


void field::field_remove_nulls_da()
{
	//
	// Remove all nulls from a dynamic area field.
	//
	// For dynamic areas, remove nulls from input fields that have been touched or changed.
	// Trailing nulls are changed to blanks.  Keep field size constant by adding blanks at
	// the end of the input field when removing nulls.  In this case, change shadow variable to 0xFF
	// so we know where the real data ends.
	//
	// Keep shadow variable in sync.
	//

	size_t p1 ;
	size_t p2 ;
	size_t p3 ;

	const char nulls( 0x00 ) ;

	field_ext1* dx = field_da_ext ;
	dynArea* da    = dx->field_ext1_dynArea ;

	for ( auto it = dx->field_ext1_usermod.begin() ; it != dx->field_ext1_usermod.end() ; ++it )
	{
		p1 = *it + 1 ;
		p3 = field_value.find_first_of( da->dynArea_Attrs, p1 ) ;
		p3 = ( p3 == string::npos ) ? field_value.size() - 1 : p3 - 1 ;
		p2 = field_value.find_last_not_of( nulls, p3 ) ;
		if ( p2 == string::npos )
		{
			field_value.replace( p1, p3-p1+1, p3-p1+1, ' ' ) ;
			field_da_ext->field_ext1_shadow.replace( p1, p3-p1+1, p3-p1+1, 0xFF ) ;
			break ;
		}
		if ( p3 > p2 )
		{
			field_value.replace( p2+1, p3-p2, p3-p2, ' ' ) ;
			field_da_ext->field_ext1_shadow.replace( p2+1, p3-p2, p3-p2, 0xFF ) ;
		}
		while ( p1 < p2 )
		{
			if ( field_value[ p1 ] == nulls )
			{
				field_value.erase( p1, 1 ) ;
				field_value.insert( p3, 1, ' ' ) ;
				field_da_ext->field_ext1_shadow.erase( p1, 1 ) ;
				field_da_ext->field_ext1_shadow.insert( p3, 1, 0xFF ) ;
				--p2 ;
				continue ;
			}
			++p1 ;
		}
	}
}


void field::field_clear( WINDOW* win )
{
	field_value = ""   ;
	field_blank( win ) ;
	field_changed = true ;
}


int field::end_of_field( WINDOW* win,
			 uint col )
{
	//
	// If this is a dynamic area, we know at this point this is an input field.
	// Strip trailing nulls if not a dynamic area.
	//

	size_t p1 ;
	size_t p2 ;
	size_t pos ;

	string padc( "\0 ", 2 ) ;

	const char nulls( 0x00 ) ;

	if ( field_da_ext )
	{
		pos = ( col - field_col ) ;
		p2  = field_value.find_first_of( field_da_ext->field_ext1_dynArea->dynArea_Attrs, pos ) ;
		if ( p2 == string::npos ) { p2 = field_value.size() ; }
		p1  = field_value.find_last_not_of( padc, p2-1 ) ;
		if ( p1 == p2 - 1 ) { return p1 + field_col     ; }
		else                { return p1 + field_col + 1 ; }
	}
	else
	{
		pos = field_value.find_last_not_of( nulls ) ;
		if ( pos == string::npos )
		{
			field_value = "" ;
			return field_col ;
		}
		else
		{
			if ( pos < field_value.size()-1 ) { field_value.erase( pos+1 ) ; }
		}
		if ( field_value.size() == field_length )
		{
			return ( field_col + field_value.size() - 1 ) ;
		}
		else
		{
			return ( field_col + field_value.size() ) ;
		}
	}

	return 0 ;
}


int field::start_of_field( WINDOW* win,
			   uint col )
{
	//
	// Return the start of the input field at the current position.
	//
	// For a dynamic area, this is the byte after the input attribute before the current position.
	// For a normal field, this is the start column of the field.
	//
	// If this is a dynamic area, we know at this point this is an input field.
	//

	size_t p1 = 0 ;

	if ( field_da_ext )
	{
		p1 = ( col - field_col ) ;
		p1 = field_value.find_last_of( field_da_ext->field_ext1_dynArea->dynArea_inAttrs, p1 ) + 1 ;
	}

	return field_col + p1 ;
}


int field::field_next_word( WINDOW* win,
			    uint col )
{
	//
	// Return the position of the start of the next word in a field.
	//
	// If this is a dynamic area, we know at this point this is an input field.
	//

	size_t pos = ( col - field_col ) ;
	size_t p1  = 0 ;
	size_t p2 ;

	dynArea* da ;

	if ( field_da_ext )
	{
		da = field_da_ext->field_ext1_dynArea ;
		string wspace = " " + da->dynArea_Attrs ;
		p1 = field_value.find_first_of( wspace, pos ) ;
		if ( p1 == string::npos )
		{
			p1 = 0 ;
		}
		else
		{
			p1 = field_value.find_first_not_of( wspace, p1 ) ;
			if ( p1 == string::npos )
			{
				p1 = 0 ;
			}
			else
			{
				p2 = field_value.find_last_of( da->dynArea_Attrs, p1 ) ;
				if ( da->dynArea_inAttrs.find_first_of( field_value[ p2 ] ) == string::npos )
				{
					p1 = 0 ;
				}
			}
		}
	}
	else
	{
		p1 = field_value.find( ' ', pos ) ;
		if ( p1 == string::npos )
		{
			p1 = 0 ;
		}
		else
		{
			p1 = field_value.find_first_not_of( ' ', p1 ) ;
			if ( p1 == string::npos ) { p1 = 0 ; }
		}
	}

	return ( p1 == 0 ) ? col : ( field_col + p1 ) ;
}


int field::field_prev_word( WINDOW* win,
			    uint col )
{
	//
	// Return the position of the start of the previous word in a field.
	//
	// If this is a dynamic area, we know at this point this is an input field.
	//

	size_t pos = ( col - field_col ) ;
	size_t p1  = 0 ;

	dynArea* da ;

	if ( field_da_ext )
	{
		da = field_da_ext->field_ext1_dynArea ;
		string wspace = " " + da->dynArea_inAttrs ;
		p1 = field_value.find_last_of( wspace, pos ) ;
		if ( p1 == string::npos )
		{
			p1 = 0 ;
		}
		else
		{
			if ( da->dynArea_Attrs.find_first_of( field_value[ p1 ] ) != string::npos )
			{
				++p1 ;
			}
			else
			{
				p1 = field_value.find_last_not_of( wspace, p1 ) ;
				if ( p1 == string::npos )
				{
					p1 = 0 ;
				}
				else
				{
					p1 = field_value.find_last_of( wspace, p1 ) ;
					if ( p1 == string::npos )
					{
						p1 = 0 ;
					}
					else
					{
						++p1 ;
					}
				}
			}
		}
	}
	else
	{
		p1 = field_value.find_last_of( ' ', pos ) ;
		if ( p1 == string::npos )
		{
			p1 = 0 ;
		}
		else
		{
			p1 = field_value.find_last_not_of( ' ', p1 ) ;
			if ( p1 == string::npos )
			{
				p1 = 0 ;
			}
			else
			{
				p1 = field_value.find_last_of( ' ', p1 ) ;
				if ( p1 == string::npos )
				{
					p1 = 0 ;
				}
				else
				{
					++p1 ;
				}
			}
		}
	}

	return ( field_col + p1 ) ;
}


int field::field_first_word( WINDOW* win,
			     uint col )
{
	//
	// Return the position of the start of the first word in field where col is positioned.
	//
	// For a dynamic area field, that is the first word in the current input field area.
	//
	// If this is a dynamic area, we know at this point this is an input field.
	//

	size_t p1 = 0 ;
	size_t p2 ;

	dynArea* da ;

	if ( field_da_ext )
	{
		da = field_da_ext->field_ext1_dynArea ;
		p1 = field_value.find_last_of( da->dynArea_inAttrs, ( col - field_col ) ) ;
		p2 = p1 + 1 ;
		p1 = field_value.find_first_not_of( ' ', p2 ) ;
		if ( p1 == string::npos || ( da->dynArea_Attrs.find_first_of( field_value[ p1 ] ) != string::npos ) )
		{
			p1 = p2 ;
		}
	}
	else
	{
		p1 = field_value.find_first_not_of( ' ' ) ;
		if ( p1 == string::npos )
		{
			p1 = 0 ;
		}
	}

	return ( field_col + p1 ) ;
}


int field::field_last_word( WINDOW* win,
			    uint col )
{
	//
	// Return the position of the start of the last word in field where col is positioned.
	//
	// For a dynamic area field, that is the last word in the current input field area.
	//
	// If this is a dynamic area, we know at this point this is an input field.
	//

	size_t pos = col - field_col ;
	size_t p1  = field_length ;

	if ( field_da_ext )
	{
		p1 = field_value.find_first_of( field_da_ext->field_ext1_dynArea->dynArea_Attrs, pos ) ;
		if ( p1 == string::npos )
		{
			p1 = field_length ;
		}
	}


	return field_prev_word( win, ( p1 + field_col - 1 ) ) ;
}


bool field::field_dyna_input( uint col )
{
	//
	// Return true if the current position is an input area.
	//
	// When this routine is called, we only know the field is a dynamic area.
	//
	// Check if we are on a field attribute byte (input or output).
	// Find the previous field attribute.
	// If found, test to see if it is an input attribute.
	//

	size_t pos ;
	size_t p1  ;

	dynArea* da = field_da_ext->field_ext1_dynArea ;

	if ( da->dynArea_inAttrs == "" ) { return false ; }

	pos = col - field_col ;

	if ( da->dynArea_Attrs.find( field_value[ pos ] ) != string::npos ) { return false ; }

	p1 = field_value.find_last_of( da->dynArea_Attrs, pos ) ;
	if ( p1 == string::npos ) { return false ; }

	p1 = da->dynArea_inAttrs.find( field_value[ p1 ] ) ;
	if ( p1 == string::npos ) { return false ; }

	return true ;
}


int field::field_dyna_input_offset( uint col )
{
	//
	// Return the offset of the first non-attribute byte of the next input field.
	//
	// When this routine is called, we know the field is a dynamic area.
	//

	size_t p1 = ( col < field_col ) ? 0 : ( col - field_col ) ;

	dynArea* da = field_da_ext->field_ext1_dynArea ;

	while ( true )
	{
		p1 = field_value.find_first_of( da->dynArea_inAttrs, p1 ) ;
		if ( p1 == string::npos ) { return -1 ; }
		++p1 ;
		if ( p1 >= field_value.size() ) { return -1 ; }
		if ( da->dynArea_Attrs.find( field_value[ p1 ] ) == string::npos ) { break ; }
	}

	return p1 ;
}


int field::field_dyna_input_offset_prev( uint col )
{
	//
	// Return the offset of the first non-attribute byte in the previous input field.
	//
	// When this routine is called, we know the field is a dynamic area.
	//

	size_t p1  ;

	if ( col < field_col + 2 ) { return -1 ; }

	p1 = ( col > field_endcol ) ? ( field_length - 1 ) : ( col - field_col - 2 ) ;

	dynArea* da = field_da_ext->field_ext1_dynArea ;

	while ( true )
	{
		p1 = field_value.find_last_of( da->dynArea_inAttrs, p1 ) ;
		if ( p1 == string::npos ) { return -1 ; }
		++p1 ;
		if ( p1 >= field_value.size() ) { return -1 ; }
		if ( da->dynArea_Attrs.find( field_value[ p1 ] ) == string::npos ) { break ; }
		if ( p1 < 2 ) { return -1 ; }
		p1 -= 2 ;
	}

	return p1 ;
}


void field::field_attr( errblock& err,
			const string& attrs,
			bool chng_once )
{
	//
	// Change field attributes.  Only parameters specified are changed, other
	// attributes are left unchanged.
	//

	if ( field_char_attrs )
	{
		delete field_inline_attrs ;
		field_inline_attrs  = new char_attrs ;
		*field_inline_attrs = *field_char_attrs ;
		field_inline_attrs->override_attrs( err,
						    attrs,
						    chng_once,
						    true ) ;
		if ( err.error() ) { return ; }
	}
	else if ( field_inline_attrs )
	{
		field_inline_attrs->override_attrs( err,
						    attrs,
						    chng_once ) ;
		if ( err.error() ) { return ; }
	}
}


bool field::field_attr_reset_once()
{
	//
	// Reset field attribute to use the original attribute stucture if flagged for one redisplay.
	//

	bool rem_override = ( field_inline_attrs && field_inline_attrs->is_once() ) ;

	if ( rem_override )
	{
		field_attr_reset() ;
	}

	return rem_override ;
}


void field::field_attr_reset()
{
	//
	// Reset field attribute to use the original attribute stucture.
	//

	if ( field_char_attrs )
	{
		delete field_inline_attrs ;
		field_inline_attrs = nullptr ;
	}
	else if ( field_inline_attrs )
	{
		field_inline_attrs->remove_override() ;
	}
}


void field::field_prep_input()
{
	//
	// Prepare the field to be copied to the function pool (non-dynamic area fields only).
	//
	// Remove nulls from the field:
	//
	// For JUST(ASIS), replace leading nulls with the pad character.
	// For JUST(RIGHT), replace trailing nulls with the pad character.
	//
	// Replace all nulls between first non-null character and the last non-null/non-space
	// character with the pad character.
	//
	// JUST(LEFT/RIGHT) leading and trailing spaces are removed.
	// JUST(ASIS) Only trailing spaces are removed.
	//

	int l ;

	const char nulls( 0x00 ) ;

	string spnulls( "\0 ", 2 ) ;

	size_t p1 ;
	size_t p2 ;
	size_t pf = field_value.find_first_not_of( spnulls ) ;
	size_t pl = field_value.find_last_not_of( spnulls ) ;

	char upad = ( field_is_paduser() ) ? field_paduchar : field_get_padchar() ;
	char just = field_get_just() ;

	if ( just == 'A' )
	{
		if ( pf != string::npos && pf > 0 )
		{
			field_value.replace( 0, pf, pf, upad ) ;
		}
	}
	else if ( just == 'R' )
	{
		if ( pl != string::npos && pl < ( field_value.size() - 1 ) )
		{
			l = field_value.size() - 1 - pl ;
			field_value.replace( pl + 1, l, l, upad ) ;
		}
	}

	if ( pf != string::npos && pl != string::npos )
	{
		for ( size_t i = ( pf + 1 ) ; i < pl ; ++i )
		{
			if ( field_value[ i ] == nulls )
			{
				field_value[ i ] = upad ;
			}
		}
	}

	p1 = 0 ;
	while ( true )
	{
		p1 = field_value.find( nulls, p1 ) ;
		if ( p1 == string::npos ) { break ; }
		p2 = field_value.find_first_not_of( nulls, p1 ) ;
		if ( p2 == string::npos )
		{
			field_value.erase( p1 ) ;
			break ;
		}
		else
		{
			field_value.erase( p1, p2-p1 ) ;
		}
	}

	switch ( just )
	{
		case 'L':
		case 'R': trim( field_value ) ;
			  break ;

		case 'A': trim_right( field_value ) ;
			  break ;
	}

	if ( field_sf_ext )
	{
		field_update_ssvalue() ;
	}
}


void field::field_prep_display()
{
	//
	// Prepare the field for display.
	//
	// Apply just(right|left|asis) to non-dynamic area input/output fields:
	//
	//     JUST(LEFT)  strip off leading and trailing spaces.
	//     JUST(RIGHT) strip off trailing pad character.
	//                 strip off leading and trailing spaces.
	//                 pad on the left with the pad character.
	//     JUST(ASIS)  strip off trailing spaces only.
	//

	const char nulls( 0x00 ) ;

	char upad ;

	switch ( field_get_just() )
	{
		case 'L': trim( field_value ) ;
			  break ;

		case 'R':
			  upad = ( field_is_paduser() ) ? field_paduchar : field_get_padchar() ;
			  istrip( field_value, 'T', upad ) ;
			  trim( field_value ) ;
			  field_value = right( field_value, field_length, nulls ) ;
			  break ;

		case 'A': trim_right( field_value ) ;
	}
}


void field::field_apply_caps()
{
	//
	// Set upper case if CAPS ON.
	//

	if ( field_is_caps() )
	{
		iupper( field_value ) ;
	}
}


void field::field_apply_caps_in()
{
	//
	// Set upper case if CAPS ON.  For data being stored in the function pool.
	//

	if ( field_is_caps_in() )
	{
		iupper( field_value ) ;
	}
}


void field::field_apply_caps_out()
{
	//
	// Set upper case if CAPS ON.  For data being displayed from the function pool.
	//

	if ( field_is_caps_out() )
	{
		iupper( field_value ) ;
	}
}


void field::field_apply_caps_uncond()
{
	//
	// Set upper case unconditionally (for scroll fields).
	//

	iupper( field_value ) ;
}


string& field::convert_value( string& s )
{
	//
	// Convert 's' according to the field options.
	// Set upper case if CAPS ON.
	//

	if ( field_is_caps() )
	{
		iupper( s ) ;
	}

	return s ;
}


void field::field_update_datamod_usermod( string* darea,
					  int offset )
{
	//
	// Set the DATAIN attribute to DATAMOD for any dynamic area input field that has changed.
	// Set the DATAIN attribute to USERMOD for any dynamic area input field that has been touched.
	//
	// Specified on DYNAREA...
	// USERMOD but not DATAMOD - Attr changed to USERMOD even if the field was changed.
	// DATAMOD but not USERMOD - Attr changed to DATAMOD only if the field value has changed.
	// USERMOD and DATAMOD     - Attr changed to USERMOD if no change to field value, else DATAMOD.
	// Neither                 - Attr unchanged.
	//

	size_t p1 ;
	size_t p2 ;

	field_ext1* dx = field_da_ext ;
	dynArea* da    = dx->field_ext1_dynArea ;

	if ( not da->dynArea_dataModsp && not da->dynArea_userModsp ) { return ; }

	for ( auto it = dx->field_ext1_usermod.begin() ; it != dx->field_ext1_usermod.end() ; ++it )
	{
		if ( da->dynArea_userModsp )
		{
			field_value[ *it ] = da->dynArea_UserMod ;
		}
		if ( da->dynArea_dataModsp )
		{
			p1 = *it + 1 ;
			p2 = field_value.find_first_of( da->dynArea_Attrs, p1 ) ;
			if ( p2 == string::npos ) { p2 = field_value.size() ; }
			if ( field_value.compare( p1, p2-p1, *darea, offset+p1, p2-p1 ) != 0 )
			{
				field_value[ *it ] = da->dynArea_DataMod ;
			}
		}
	}
}


void field::display_field( WINDOW* win,
			   char inv_schar,
			   map<unsigned char, uint>& ddata_map,
			   map<unsigned char, uint>& schar_map )
{
	//
	// For non-dynamic area fields: if an input field, truncate if value size > field size else for output fields
	// display field size bytes and leave field value unchanged.
	//
	// Display the null character as the field pad character and pad the field with the same character.
	//
	// Colour/hilite taken from the shadow byte if valid (ie. corresponds to a TYPE(CHAR) ATTR entry).
	// Intensity and default colour/hilite taken from DATAIN/DATAOUT.
	// If no DATAIN/DATAOUT and shadow byte is not valid, default to colour WHITE.
	//
	// 00 X0 00 00 - X is the INTENSITY.
	// 00 0X 00 00 - X is the HILITE.
	// 00 00 XX 00 - X is the COLOUR.
	//

	uint i ;

	uint intens = 0 ;
	uint colour = 0 ;
	uint attr2  = 0 ;
	uint attrd  = 0 ;

	if ( !field_active ) { return ; }

	const uint cl_mask = RED  | GREEN | YELLOW  | BLUE      | MAGENTA     |
			     TURQ | WHITE | A_BLINK | A_REVERSE | A_UNDERLINE ;

	char nullc = ( field_nulls ) ? '.' : ' ' ;
	char upad  = ( field_is_paduser() ) ? field_paduchar : field_get_padchar() ;
	char pad ;
	char attr1 ;

	const char nulls( 0x00 ) ;

	string t ;

	string::iterator ita ;
	string::iterator its ;

	if ( field_da_ext )
	{
		dynArea* da = field_da_ext->field_ext1_dynArea ;
		if ( da->dynArea_Attrs != "" )
		{
			its = field_da_ext->field_ext1_shadow.begin() ;
			i   = 0 ;
			auto itc = schar_map.find( *its ) ;
			if ( itc == schar_map.end() )
			{
				wattrset( win, WHITE | field_intens ) ;
			}
			else
			{
				wattrset( win, itc->second | field_intens ) ;
			}
			attr1 = *its ;
			for ( ita = field_value.begin() ; ita != field_value.end() ; ++ita, ++its, ++i )
			{
				if ( attr1 != *its || attr2 != attrd )
				{
					itc = schar_map.find( *its ) ;
					if ( itc == schar_map.end() )
					{
						if ( colour == 0 )
						{
							wattrset( win, WHITE | field_intens ) ;
						}
						else
						{
							wattrset( win, colour | field_intens | intens ) ;
						}
					}
					else
					{
						wattrset( win, itc->second | field_intens | intens ) ;
					}
					attr1 = *its ;
					attr2 = attrd ;
				}
				if ( *ita == nulls )
				{
					mvwaddch( win, field_row, field_col+i, nullc ) ;
				}
				else if ( da->dynArea_Attrs.find( *ita ) != string::npos )
				{
					attr1  = inv_schar ;
					attrd  = ddata_map[ *ita ] ;
					intens = attrd & A_BOLD ;
					colour = attrd & cl_mask ;
					wattrset( win, ( colour & A_COLOR ) | field_intens | intens ) ;
					mvwaddch( win, field_row, field_col+i, ' ' ) ;
				}
				else if ( isprint( *ita ) )
				{
					mvwaddch( win, field_row, field_col+i, *ita ) ;
				}
				else
				{
					mvwaddch( win, field_row, field_col+i, '.' ) ;
				}
			}
		}
		else
		{
			its = field_da_ext->field_ext1_shadow.begin() ;
			i   = 0 ;
			auto itc = schar_map.find( *its ) ;
			if ( itc == schar_map.end() )
			{
				wattrset( win, WHITE | field_intens ) ;
			}
			else
			{
				wattrset( win, itc->second | field_intens ) ;
			}
			attr1 = *its ;
			for ( ita = field_value.begin() ; ita != field_value.end() ; ++ita, ++its, ++i )
			{
				if ( attr1 != *its )
				{
					itc = schar_map.find( *its ) ;
					if ( itc == schar_map.end() )
					{
						wattrset( win, WHITE | field_intens ) ;
					}
					else
					{
						wattrset( win, itc->second | field_intens ) ;
					}
					attr1 = *its ;
				}
				if ( *ita == nulls )
				{
					mvwaddch( win, field_row, field_col+i, nullc ) ;
				}
				else if ( isprint( *ita ) )
				{
					mvwaddch( win, field_row, field_col+i, *ita ) ;
				}
				else
				{
					mvwaddch( win, field_row, field_col+i, '.' ) ;
				}
			}
		}
	}
	else
	{
		wattrset( win, field_get_colour() | field_intens ) ;
		if ( field_is_intens_non() )
		{
			t = string( field_length, ' ' ) ;
		}
		else if ( field_is_input() )
		{
			if ( field_value.size() > field_length )
			{
				field_value.erase( field_length ) ;
			}
			t = field_value ;
		}
		else
		{
			t = field_value ;
			if ( field_value.size() > field_length )
			{
				t.erase( field_length ) ;
			}
		}
		pad = ( upad == nulls ) ? ' ' : upad ;
		if ( field_is_passwd() )
		{
			if ( t == "" )
			{
				if ( field_length > 7 )
				{
					t = "Password" ;
				}
			}
			else
			{
				replace_if( t.begin(), t.end(), []( char c )
								{
									return ( c != nulls && c != ' ' ) ;
								}, '*' ) ;
				replace_if( t.begin(), t.end(), []( char c )
								{
									return ( c == nulls ) ;
								}, pad ) ;
			}
		}
		else
		{
			nullc = ( field_nulls ) ? '.' : pad ;
			for ( ita = t.begin() ; ita != t.end() ; ++ita )
			{
				if      ( *ita == nulls    ) { *ita = nullc ; }
				else if ( !isprint( *ita ) ) { *ita = '.'   ; }
			}
		}
		if ( t != "" && field_is_padc() )
		{
			pad = ' ' ;
		}
		if ( field_is_input() )
		{
			t.resize( field_length, pad ) ;
		}
		else
		{
			switch ( field_get_just() )
			{
				case 'A':
				case 'L': t.resize( field_length, pad ) ;
					  break ;

				case 'R': t = right( t, field_length, pad ) ;
					  break ;

			}
		}
		mvwaddstr( win, field_row, field_col, t.c_str() ) ;
	}
}


bool field::cursor_on_field( uint row,
			     uint col )
{
	return ( row == field_row && col >= field_col && col <= field_endcol ) ;
}


/******************************************************************************************************************/
/*                                           SCROLLABLE AREA CLASS                                                */
/******************************************************************************************************************/

Area::Area( errblock& err,
	    int MAXW,
	    int MAXD,
	    uint num,
	    const string& line ) : Area()
{
	//
	// Format of AREA entry in panels (FORMAT 1 VERSION 1).
	// AREA row col width depth area_name
	//
	// w1      w2         w3   w4    w5     w6
	// AREA    2           2   25    5      SAREA1
	// AREA    MAX-10 MAX-20   MAX   MAX-6  SAREA2
	//

	int row   ;
	int col   ;
	int width ;
	int depth ;

	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;

	string rest = subword( line, 7 ) ;
	if ( rest != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", rest ) ;
		return ;
	}

	w2 = word( line, 2 ) ;
	w3 = word( line, 3 ) ;
	w4 = word( line, 4 ) ;
	w5 = word( line, 5 ) ;

	err.setRC( 0 ) ;

	if ( isnumeric( w2 ) )                      { row = ds2d( w2  ) ; }
	else if ( w2 == "MAX" )                     { row = MAXD        ; }
	else if ( w2.compare( 0, 4, "MAX-" ) == 0 ) { row = MAXD - ds2d( substr( w2, 5 ) )    ; }
	else                                        { err.seterrid( TRACE_INFO(), "PSYE031S", w2 ) ; return ; }

	if ( isnumeric( w3 ) )                      { col = ds2d( w3 )  ; }
	else if ( w3 == "MAX" )                     { col = MAXW        ; }
	else if ( w3.compare( 0, 4, "MAX-" ) == 0 ) { col = MAXW - ds2d( substr( w3, 5 ) )    ; }
	else                                        { err.seterrid( TRACE_INFO(), "PSYE031S", w3 ) ; return ; }

	if ( isnumeric( w4 ) )                      { width = ds2d( w4  )    ; }
	else if ( w4 == "MAX" )                     { width = MAXW - col + 1 ; }
	else if ( w4.compare( 0, 4, "MAX-" ) == 0 ) { width = MAXW - col - ds2d( substr( w4, 5 ) ) + 1 ; }
	else                                        { err.seterrid( TRACE_INFO(), "PSYE031S", w4 ) ; return          ; }

	if ( isnumeric( w5 ) )                      { depth = ds2d( w5 )     ; }
	else if ( w5 == "MAX" )                     { depth = MAXD - row + 1 ; }
	else if ( w5.compare( 0, 4, "MAX-" ) == 0 ) { depth = MAXD - row - ds2d( substr( w5, 5 ) ) + 1 ; }
	else                                        { err.seterrid( TRACE_INFO(), "PSYE031S", w5 ) ; return          ; }

	if ( ( row + depth - 1 ) > MAXD )
	{
		err.seterrid( TRACE_INFO(), "PSYE032L", d2ds( row ), d2ds( depth ), d2ds( MAXD ) ) ;
		return ;
	}
	if ( ( col + width - 1 ) > MAXW )
	{
		err.seterrid( TRACE_INFO(), "PSYE032M", d2ds( col ), d2ds( width ), d2ds( MAXW ) ) ;
		return ;
	}

	if ( width < 20 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032N" ) ;
		return ;
	}
	if ( depth < 2 )
	{
		err.seterrid( TRACE_INFO(), "PSYE032O" ) ;
		return ;
	}

	Area_num   = num   ;
	Area_row   = row-1 ;
	Area_col   = col-1 ;
	Area_width = width ;
	Area_depth = depth - 1 ;
}


void Area::get_info( uint& row,
		     uint& depth )
{
	row   = Area_row ;
	depth = min( ( maxRow + 1 ), Area_depth ) ;
}


void Area::get_info( uint& row,
		     uint& col,
		     uint& width )
{
	row   = Area_row ;
	col   = Area_col ;
	width = Area_width ;
}


void Area::get_info( uint& row,
		     uint& col,
		     uint& width,
		     uint& depth )
{
	row   = Area_row ;
	col   = Area_col ;
	width = Area_width ;
	depth = Area_depth ;
}


void Area::set_visible_depth( uint cvd )
{
	Visible_depth = min( ( maxRow + 1 ), Area_depth ) - cvd ;
	covered = cvd ;
}


void Area::add( text* t )
{
	textList.push_back( t ) ;
	if ( t->text_area_row > maxRow )
	{
		maxRow = t->text_area_row ;
		if ( maxRow >= Area_depth )
		{
			maxPos = maxRow - Area_depth + 1 ;
		}
	}
}


void Area::add( const string& name,
		field* t )
{
	fieldList[ name ] = t ;
	if ( t->field_area_row > maxRow )
	{
		maxRow = t->field_area_row ;
		if ( maxRow >= Area_depth )
		{
			maxPos = maxRow - Area_depth + 1 ;
		}
	}
	t->field_area = Area_num ;
}


void Area::make_visible( field* t )
{
	//
	// Scroll area so field t is visible:
	// ...Above the top line - move to the top line.
	// ...Below the end line - move to the end line.
	//

	if ( t->field_area_row < pos )
	{
		pos = t->field_area_row ;
	}
	else
	{
		pos = t->field_area_row - Visible_depth + 1 ;
	}
}


bool Area::in_visible_area( field* t )
{
	return ( ( t->field_row - Area_row ) <= Visible_depth ) ;
}


bool Area::cursor_on_area( uint row,
			   uint col )
{
	return ( row >= Area_row && row <= ( Area_row + Visible_depth ) &&
		 col >= Area_col && col <  ( Area_col + Area_width ) ) ;
}


int Area::scroll_up( uint row,
		     uint col )
{
	bool onArea = false ;

	uint oldpos = pos ;
	uint amnt ;

	if ( pos == 0 ) { return -1 ; }

	if ( Visible_depth == 1 )
	{
		amnt = 1 ;
	}
	else if ( row >  Area_row && row < ( Area_row + Visible_depth ) &&
		  col >= Area_col && col < ( Area_col + Area_width ) )
	{
		amnt   = Visible_depth + Area_row - row ;
		onArea = true ;
	}
	else
	{
		amnt = Visible_depth - 1 ;
	}

	pos = ( amnt > pos ) ? 0 : pos - amnt ;

	return ( ( !onArea || ( row == Area_row + Visible_depth ) ) ? 0 : oldpos - pos ) ;
}


int Area::scroll_down( uint row,
		       uint col )
{
	uint tmxPos = maxPos + covered ;

	bool onArea = false ;

	uint oldpos = pos ;
	uint amnt ;

	if ( pos == tmxPos ) { return -1 ; }

	if ( Visible_depth == 1 )
	{
		amnt = 2 ;
	}
	else if ( row > ( Area_row + 1 ) && row <= ( Area_row + Visible_depth ) &&
		  col >= Area_col        && col <  ( Area_col + Area_width ) )
	{
		amnt   = row - Area_row ;
		onArea = true ;
	}
	else
	{
		amnt = Visible_depth ;
	}

	pos = pos + amnt - 1 ;

	if ( pos > tmxPos )
	{
		pos = tmxPos;
	}

	return ( ( !onArea || ( row == Area_row + 1 ) ) ? 0 : pos - oldpos ) ;
}


bool Area::scroll_to_top()
{
	if ( pos == 0 ) { return false ; }

	pos = 0 ;

	return true ;
}


void Area::check_overlapping_fields( errblock& err,
				     const string& area_name )
{
	uint idx = 1 ;

	uint i ;
	uint j ;
	uint k ;

	string t1 ;
	string t2 ;
	string t3 ;

	map<int, string> xref ;

	short int* fieldMap = new short int[ ( maxRow + 1 ) * Area_width ]() ;

	for ( auto it = fieldList.begin() ; it != fieldList.end() ; ++it, ++idx )
	{
		xref[ idx ] = it->first ;
		j = it->second->field_area_row * Area_width + it->second->field_area_col ;
		k = j + it->second->field_length ;
		for ( i = j ; i < k ; ++i )
		{
			if ( fieldMap[ i ] != 0 )
			{
				t1 = it->first.substr( 0, it->first.find( '.' ) ) ;
				t2 = xref[ fieldMap[ i ] ] ;
				t2 = t2.substr( 0, t2.find( '.' ) ) ;
				err.seterrid( TRACE_INFO(), "PSYE043A", t1, t2, area_name ) ;
				delete[] fieldMap ;
				return ;
			}
			fieldMap[ i ] = idx ;
		}
	}

	xref[ idx ] = "" ;
	for ( auto it = textList.begin() ; it != textList.end() ; ++it )
	{
		j = (*it)->text_row * Area_width + (*it)->text_area_col ;
		k = j + (*it)->text_endcol - (*it)->text_col + 1 ;
		for ( i = j ; i < k ; ++i )
		{
			if ( fieldMap[ i ] != 0 )
			{
				t1  = xref[ fieldMap[ i ] ] ;
				t2  = d2ds( (*it)->text_row + 1 ) ;
				t2 += "," + d2ds( (*it)->text_area_col + 1 ) ;
				if ( t1 == "" )
				{
					t3 = d2ds( i + (*it)->text_area_col - j + 1 ) ;
					err.seterrid( TRACE_INFO(), "PSYE043B", t2, t3, area_name ) ;
				}
				else
				{
					err.seterrid( TRACE_INFO(), "PSYE043C", t1, t2, area_name ) ;
				}
				delete[] fieldMap ;
				return ;
			}
			fieldMap[ i ] = idx ;
		}
	}

	delete[] fieldMap ;
}


void Area::update_area()
{
	update_fields() ;
	update_text() ;
}


void Area::update_fields()
{
	uint lim = Area_depth + pos ;

	field* pfield ;

	for ( auto it = fieldList.begin() ; it != fieldList.end() ; ++it )
	{
		pfield = it->second ;
		if ( pfield->field_area_row >= pos &&
		     pfield->field_area_row < lim )
		{
			pfield->field_row     = pfield->field_area_row + Area_row - pos + 1 ;
			pfield->field_visible = true ;
			pfield->field_active  = true ;
		}
		else
		{
			pfield->field_visible = false ;
			pfield->field_active  = false ;
		}
	}
}


void Area::update_text()
{
	uint lim = Area_depth + pos ;

	for ( auto it = textList.begin() ; it != textList.end() ; ++it )
	{
		if ( (*it)->text_area_row >= pos &&
		     (*it)->text_area_row < lim )
		{
			(*it)->text_row = (*it)->text_area_row + Area_row - pos + 1 ;
			(*it)->text_visible = true ;
		}
		else
		{
			(*it)->text_visible = false ;
		}
	}
}


const char* Area::get_scroll_indicator()
{
	return ( pos == 0 && maxRow <  Visible_depth ) ? si1 :
	       ( pos == 0 && maxRow >= Visible_depth ) ? si2 :
	       ( pos == ( maxPos + covered ) ) ? si3 : si4 ;
}


bool Area::can_scroll()
{
	//
	// Return true if the area can be scrolled down.
	//

	return !( pos == 0 && maxRow < Visible_depth ) ;
}


/******************************************************************************************************************/
/*                                              DYNAMIC AREA CLASS                                                */
/******************************************************************************************************************/

dynArea::dynArea( errblock& err,
		  int MAXW,
		  int MAXD,
		  const string& line,
		  const string& da_dataIn,
		  const string& da_dataOut,
		  uint Area_num ) : dynArea()
{
	//
	// Format of DYNAREA entry in panels (FORMAT 1 VERSION 1).
	// DYNAREA row col width depth   A-name S-name USERMOD() DATAMOD() SCROLL()
	//
	// w1      w2         w3   w4    w5     w6     w7      <-------------------------keywords------------------------->
	// DYNAREA MAX-10 MAX-20   MAX   MAX-6  ZAREA  ZSHADOW USERMOD(03) DATAMOD(04) SCROLL(OFF|ON) OPREF(ABCD) OLEN(123)
	//
	// If width=MAX  width=MAXW-col+1.
	// If depth=MAX  depth=MAXD-row+1.
	//
	// OPREF defines a dialogue variable prefix to contain overflow data at the end of each line.  Max length is 4.
	// OLEN  defines a numeric value or dialogue variable to contain the maximum overflow data length.
	//
	// USERMOD - field touched.
	// DATAMOD - field changed.
	//
	// USERMOD but not DATAMOD - Attr changed to USERMOD even if the field was changed.
	// DATAMOD but not USERMOD - Attr changed to DATAMOD only if the field value has changed.
	// USERMOD and DATAMOD     - Attr changed to USERMOD if no change to field value, else DATAMOD.
	// Neither                 - Attr unchanged.
	//

	int row   ;
	int col   ;
	int width ;
	int depth ;

	string t  ;
	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;
	string w6 ;
	string w7 ;
	string rest ;

	w2   = word( line, 2 ) ;
	w3   = word( line, 3 ) ;
	w4   = word( line, 4 ) ;
	w5   = word( line, 5 ) ;
	w6   = word( line, 6 ) ;
	w7   = word( line, 7 ) ;
	rest = subword( line, 8 ) ;

	err.setRC( 0 ) ;

	if ( w6 == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032K" ) ;
		return ;
	}

	if ( !isvalidName( w6 ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE022J", w6, "Dynamic area" ) ;
		return ;
	}

	if ( w7 == "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032E" ) ;
		return ;
	}

	if ( !isvalidName( w7 ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE022J", w7, "Dynamic area shadow" ) ;
		return ;
	}

	if ( Area_num > 0 && ( w2 == "MAX" || w2.compare( 0, 4, "MAX-" ) == 0 ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE031U" ) ;
		return ;
	}

	if ( Area_num > 0 && ( w5 == "MAX" || w5.compare( 0, 4, "MAX-" ) == 0 ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE031Y" ) ;
		return ;
	}

	if ( isnumeric( w2 ) )                      { row = ds2d( w2  ) ; }
	else if ( w2 == "MAX" )                     { row = MAXD        ; }
	else if ( w2.compare( 0, 4, "MAX-" ) == 0 ) { row = MAXD - ds2d( substr( w2, 5 ) )    ; }
	else                                        { err.seterrid( TRACE_INFO(), "PSYE031S", w2 ) ; return ; }

	if ( isnumeric( w3 ) )                      { col = ds2d( w3 )  ; }
	else if ( w3 == "MAX" )                     { col = MAXW        ; }
	else if ( w3.compare( 0, 4, "MAX-" ) == 0 ) { col = MAXW - ds2d( substr( w3, 5 ) )    ; }
	else                                        { err.seterrid( TRACE_INFO(), "PSYE031S", w3 ) ; return ; }

	if ( isnumeric( w4 ) )                      { width = ds2d( w4  )    ; }
	else if ( w4 == "MAX" )                     { width = MAXW - col + 1 ; }
	else if ( w4.compare( 0, 4, "MAX-" ) == 0 ) { width = MAXW - col - ds2d( substr( w4, 5 ) ) + 1 ; }
	else                                        { err.seterrid( TRACE_INFO(), "PSYE031S", w4 ) ; return ; }

	if ( isnumeric( w5 ) )                      { depth = ds2d( w5 )     ; }
	else if ( w5 == "MAX" )                     { depth = MAXD - row + 1 ; }
	else if ( w5.compare( 0, 4, "MAX-" ) == 0 ) { depth = MAXD - row - ds2d( substr( w5, 5 ) ) + 1 ; }
	else                                        { err.seterrid( TRACE_INFO(), "PSYE031S", w5 ) ; return ; }

	if ( Area_num == 0 && ( row + depth - 1 ) > MAXD )
	{
		err.seterrid( TRACE_INFO(), "PSYE032A", d2ds( row ), d2ds( depth ), d2ds( MAXD ) ) ;
		return ;
	}

	if ( ( col + width - 1 ) > MAXW )
	{
		err.seterrid( TRACE_INFO(), "PSYE032B", d2ds( col ), d2ds( width ), d2ds( MAXW ) ) ;
		return ;
	}

	t = extractKWord( err, rest, "USERMOD()" ) ;
	if ( err.error() ) { return ; }
	if ( t != "" )
	{
		dynArea_userModsp = true ;
		if ( t.size() == 1 )
		{
			dynArea_UserMod = t[ 0 ] ;
		}
		else if ( t.size() == 2 && ishex( t ) )
		{
			dynArea_UserMod = xs2cs( t )[ 0 ] ;
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE032J" ) ;
			return ;
		}
	}
	else if ( err.RSN0() )
	{
		err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
		return ;
	}

	t = extractKWord( err, rest, "DATAMOD()" ) ;
	if ( err.error() ) { return ; }
	if ( t != "" )
	{
		dynArea_dataModsp = true ;
		if ( t.size() == 1 )
		{
			dynArea_DataMod = t[ 0 ] ;
		}
		else if ( t.size() == 2 && ishex( t ) )
		{
			dynArea_DataMod = xs2cs( t )[ 0 ] ;
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE032J" ) ;
			return ;
		}
	}
	else if ( err.RSN0() )
	{
		err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
		return ;
	}

	t = parseString1( err, rest, "SCROLL()" ) ;
	if ( err.error() ) { return ; }
	if ( t == "ON" )
	{
		dynArea_scroll = true ;
	}
	else if ( t == "OFF" )
	{
		dynArea_scroll = false ;
	}
	else if ( err.RSN0() )
	{
		if ( t == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
		}
		else
		{
			err.seterrid( TRACE_INFO(), "PSYE032C", t ) ;
		}
		return ;
	}

	t = parseString1( err, rest, "OPREF()" ) ;
	if ( err.error() ) { return ; }
	if ( err.RSN0() )
	{
		if ( t == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
			return ;
		}
		if ( t.size() > 4 || !isvalidName( t ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE022J", t, "Dynamic area overflow prefix" ) ;
			return ;
		}
	}

	dynArea_oprefix = t ;

	t = parseString1( err, rest, "OLEN()" ) ;
	if ( err.error() ) { return ; }
	if ( err.RSN0() )
	{
		if ( t == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE031G" ) ;
			return ;
		}
		if ( isvalidName( t ) )
		{
			dynArea_olenvar = t ;
		}
		else if ( t.size() > 5 || !datatype( t, 'W' ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE033J" ) ;
			return ;
		}
		else
		{
			dynArea_olen = ds2d( t ) ;
			if ( dynArea_olen > 65535 )
			{
				err.seterrid( TRACE_INFO(), "PSYE033J" ) ;
				return ;
			}
		}
	}

	if ( trim( rest ) != "" )
	{
		err.seterrid( TRACE_INFO(), "PSYE032H", rest ) ;
		return ;
	}

	dynArea_row   = row-1 ;
	dynArea_col   = col-1 ;
	dynArea_width = width ;
	dynArea_depth = depth ;
	dynArea_area  = width * depth ;
	dynArea_name  = w6 ;
	dynArea_shadow_name = w7 ;

	dynArea_inAttrs = da_dataIn ;
	dynArea_Attrs   = da_dataIn + da_dataOut ;
}


void dynArea::add( void* p )
{
	fieldList.push_back( p ) ;
}


/******************************************************************************************************************/
/*                                              TEXT CLASS                                                        */
/******************************************************************************************************************/

text::text( errblock& err,
	    int MAXW,
	    int MAXD,
	    map<unsigned char, char_attrs*> char_attrlist,
	    uint& opt_field,
	    uint& rp_field,
	    string line,
	    uint Area_num,
	    uint Area_col ) : text()
{
	//
	// Format of text entry in panels (FORMAT 1 VERSION 1).
	// text row col cuaAttr (EXPAND) value
	// w1   w2  w3  w4      w5
	// text 3   1   FP      "COMMAND ===> "
	// text 3   1   ATTR(+) "COMMAND ===> "
	// OR
	// with optional length
	// w1   w2  w3  w4     w5      w6
	// text 3   1   15     FP      "COMMAND ===> "
	// text 3   1   15     ATTR(+) "COMMAND ===> "
	// OR
	// w1   w2  w3  w4      w5     w6
	// text 3   1   FP      EXPAND "-"
	// text 3   1   ATTR(+) EXPAND "-"
	//
	// Use EXPAND to repeat the value to end of the screen (MAXW).
	//
	// )ATTR section TYPE() statement can be used in place of a CUA attribute.
	//
	// Valid CUA attributes for Text, protected:
	//    CH, CT, DT, ET, FP, NT, PIN, PS, PT, SAC, SI, SUC, WASL, WT
	// Valid non-CUA attributes for Text, protected:
	//    TEXT
	//

	int l   ;
	int row ;
	int col ;
	int len = 0 ;

	char c ;

	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;
	string t  ;
	string llen ;
	string rest ;

	w4 = upper( word( line, 4 ) ) ;
	if ( isnumeric( w4 ) || w4 == "MAX" || w4.compare( 0, 4, "MAX-" ) == 0 )
	{
		llen = w4 ;
		line = delword( line, 4, 1 ) ;
	}

	w2 = upper( word( line, 2 ) ) ;
	w3 = upper( word( line, 3 ) ) ;
	w4 = upper( word( line, 4 ) ) ;

	if ( Area_num > 0 && ( w2 == "MAX" || w2.compare( 0, 4, "MAX-" ) == 0 ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE031U" ) ;
		return ;
	}

	if ( isnumeric( w2 ) )                      { row = ds2d( w2 ) ; }
	else if ( w2 == "MAX" )                     { row = MAXD       ; }
	else if ( w2.compare( 0, 4, "MAX-" ) == 0 ) { row = MAXD - ds2d( substr( w2, 5 ) )    ; }
	else                                        { err.seterrid( TRACE_INFO(), "PSYE031S", w2 ) ; return ; }

	if ( isnumeric( w3 ) )                      { col = ds2d( w3 ) ; }
	else if ( w3 == "MAX" )                     { col = MAXW       ; }
	else if ( w3.compare( 0, 4, "MAX-" ) == 0 ) { col = MAXW - ds2d( substr( w3, 5 ) )    ; }
	else                                        { err.seterrid( TRACE_INFO(), "PSYE031S", w3 ) ; return ; }

	if ( llen != "" )
	{
		if ( isnumeric( llen ) )  { len = ds2d( llen ) ; }
		else if ( llen == "MAX" ) { len = MAXW - col + 1 ; }
		else                      { len = MAXW - col - ds2d( substr( llen, 5 ) ) + 1 ; }
	}

	if ( cuaAttrName.count( w4 ) == 0 )
	{
		rest = subword( line, 4 ) ;
		if ( upper( rest ).compare( 0, 5, "ATTR(" ) != 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE032F", w4 ) ;
			return ;
		}
		t = parseString1( err, rest, "ATTR()" ) ;
		if ( err.error() ) { return ; }
		if ( t == "" )
		{
			err.seterrid( TRACE_INFO(), "PSYE036C" ) ;
			return ;
		}
		if ( t.size() > 2 || ( t.size() == 2 && not ishex( t ) ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE041B" ) ;
			return ;
		}
		c = ( t.size() == 2 ) ? xs2cs( t ).front() : t.front() ;
		auto itc = char_attrlist.find( c ) ;
		if ( itc == char_attrlist.end() )
		{
			t = isprint( c ) ? string( 1, c ) : c2xs( c ) ;
			err.seterrid( TRACE_INFO(), "PSYE036D", t ) ;
			return ;
		}
		text_char_attrs = itc->second ;
		if ( !text_char_attrs->is_text_attr() )
		{
			t = isprint( c ) ? string( 1, c ) : c2xs( c ) ;
			err.seterrid( TRACE_INFO(), "PSYE036E", t ) ;
			return ;
		}
	}
	else
	{
		if ( !findword( w4, "CH CT DT ET FP NT IMT PIN PS PT RP SAC SI SUC WASL WT" ) )
		{
			err.seterrid( TRACE_INFO(), "PSYE036F", w4 ) ;
			return ;
		}
		text_inline_attrs = new text_attrs( cuaAttrName[ w4 ] ) ;
		rest              = subword( line, 5 ) ;
	}

	if ( Area_num == 0 && row > MAXD )
	{
		err.seterrid( TRACE_INFO(), "PSYE036A", d2ds( row ), d2ds( MAXD ) ) ;
		return ;
	}
	if ( col > MAXW )
	{
		err.seterrid( TRACE_INFO(), "PSYE036B", d2ds( col ), d2ds( MAXW ) ) ;
		return ;
	}

	text_row = row - 1 ;
	text_col = col - 1 ;

	if ( Area_num > 0 )
	{
		text_area_row = text_row ;
		text_area_col = text_col ;
		text_col     += Area_col ;
		text_visible  = false ;
	}

	w5 = upper( word( rest, 1 ) ) ;
	if ( w5 == "EXPAND" )
	{
		text_value = qstring( err, subword( rest, 2 ) ) ;
		if ( err.error() ) { return ; }
		if ( text_value.size() == 0 )
		{
			err.seterrid( TRACE_INFO(), "PSYE041K" ) ;
			return ;
		}
		l = ( MAXW - col ) / text_value.size() + 1 ;
		text_value = substr( copies( text_value, l ), 1, ( MAXW - col + 1 ) ) ;
	}
	else
	{
		text_value = qstring( err, rest ) ;
		if ( err.error() ) { return ; }
	}

	text_length = ( len > 0 ) ? len : text_value.size() ;

	if ( int( col + text_length - 1 ) > MAXW )
	{
		text_length = MAXW - col + 1 ;
	}

	text_endcol = text_col + text_length - 1 ;

	switch ( get_type() )
	{
	case PS:
		if ( Area_num > 99 )
		{
			err.seterrid( TRACE_INFO(), "PSYE042V", "Point-and-shoot" ) ;
			return ;
		}
		text_name = "ZPS" + d2ds( Area_num, 2 ) + d2ds( ++opt_field, 3 ) ;
		break ;

	case RP:
		if ( Area_num > 99 )
		{
			err.seterrid( TRACE_INFO(), "PSYE042V", "Reference phrase" ) ;
			return ;
		}
		text_name = "ZRP" + d2ds( Area_num, 2 ) + d2ds( ++rp_field, 3 ) ;
		break ;

	}
}


void text::text_display( WINDOW* win )
{
	//
	// If text_char_attrs is set, use char_attrs structure to set the text attributes,
	// otherwise use attributes in the inline_attrs structure.
	//

	if ( text_visible )
	{
		if ( text_char_attrs )
		{
			wattrset( win, text_char_attrs->get_colour() | text_intens ) ;
		}
		else
		{
			wattrset( win, text_inline_attrs->get_colour() | text_intens ) ;
		}
		text_xvalue.resize( text_length, ' ' ) ;
		mvwaddstr( win, text_row, text_col, text_xvalue.c_str() ) ;
	}
}


attType text::get_type()
{
	return ( text_char_attrs ) ? text_char_attrs->get_type() : text_inline_attrs->get_type() ;
}


bool text::cursor_on_text( uint row,
			   uint col )
{
	return ( row == text_row && col >= text_col && col <= text_endcol ) ;
}


/******************************************************************************************************************/
/*                                            TBFIELD CLASS                                                       */
/******************************************************************************************************************/


bool tbfield::set_tbfield( uint col, uint len )
{
	bool changed = ( tbfield_col_var == "" ) ? len != tbfield_len :
		       ( tbfield_len_var == "" ) ? col != tbfield_col :
						   col != tbfield_col || len != tbfield_len ;
	if ( tbfield_col_var != "" )
	{
		tbfield_col = col ;
	}

	if ( tbfield_len_var != "" )
	{
		tbfield_len = len ;
	}

	return changed ;
}



void tbfield::update_fields()
{
	uint col    = tbfield_col - 1 ;
	uint endcol = tbfield_col + tbfield_len - 2 ;

	bool active = ( tbfield_len != 0 ) ;

	for ( auto pfield : fieldList )
	{
		pfield->field_col    = col ;
		pfield->field_length = tbfield_len ;
		pfield->field_active = active ;
		pfield->field_endcol = endcol ;
		if ( pfield->field_is_scrollable() )
		{
			pfield->field_update_fdlen() ;
		}
	}
}

/******************************************************************************************************************/
/*                                             PULL DOWN CHOICE CLASS                                             */
/******************************************************************************************************************/

void pdc::display_pdc_avail( WINDOW* win,
			     size_t maxw,
			     attType type,
			     int pos )
{
	string t = right( d2ds( pos ), 2 ) + ". " + pdc_xdesc ;

	if ( pdc_dvars )
	{
		mvwaddstr( win, pos, 4, string( maxw + 4, ' ' ).c_str() ) ;
	}

	wattrset( win, cuaAttr[ type ] | pdc_intens ) ;
	mvwaddstr( win, pos, 4, t.c_str() ) ;
}


void pdc::display_pdc_unavail( WINDOW* win,
			       size_t maxw,
			       attType type,
			       int pos )
{
	string t ;

	if ( pos < 10 )
	{
		t = " *. " + pdc_xdesc ;
	}
	else
	{
		t = d2ds( pos ) + ". " + pdc_xdesc ;
		t.replace( 0, 1, "*" ) ;
	}

	if ( pdc_dvars )
	{
		mvwaddstr( win, pos, 4, string( maxw + 4, ' ' ).c_str() ) ;
	}

	wattrset( win, cuaAttr[ type ] | pdc_intens ) ;
	mvwaddstr( win, pos, 4, t.c_str() ) ;
}


/******************************************************************************************************************/
/*                                             ACTION BAR CHOICE CLASS                                            */
/******************************************************************************************************************/

void abc::create_window( uint row,
			 uint col )
{
	win = newwin( ( abc_maxh + 2 ), ( abc_maxw + 10 ), row, col ) ;
	wattrset( win, cuaAttr[ AB ] | abc_intens ) ;
	box( win, 0, 0 ) ;
}


string abc::getDialogueVar( errblock& err,
			    const string& var )
{
	string* pstr ;

	fVAR* pvar ;

	if ( selPanel )
	{
		return p_poolMGR->get( err, var, ASIS ) ;
	}

	pvar = funcPool->getfVAR( err, var ) ;
	if ( err.error() ) { return "" ; }

	if ( pvar )
	{
		return pvar->sget( var ) ;
	}
	else
	{
		pstr = p_poolMGR->vlocate( err, var ) ;
		if ( err.error() ) { return "" ; }
		switch ( err.getRC() )
		{
			case 0:
				 return *pstr ;
			case 4:
				 return p_poolMGR->get( err, var ) ;
			case 8:
				 funcPool->put1( err, var, "" ) ;
		}
	}

	return "" ;
}


void abc::putDialogueVar( errblock& err,
			  const string& var,
			  const string& val )
{
	fVAR* pvar ;

	if ( selPanel )
	{
		p_poolMGR->put( err, var, val, ASIS ) ;
	}
	else
	{
		pvar = funcPool->getfVAR( err, var ) ;
		if ( err.error() ) { return ; }
		if ( pvar )
		{
			pvar->put( err, var, val ) ;
		}
		else
		{
			funcPool->put2( err, var, val ) ;
		}
	}
}


string abc::sub_vars( errblock& err,
		      string s,
		      bool& dvars )
{
	//
	// In string, s, substitute variables starting with '&' for their dialogue value.
	// The variable is delimited by any non-valid variable characters.
	// Rules: A '.' at the end of a variable, concatinates the value with the string following the dot
	//        .. reduces to .
	//        && reduces to & with no variable substitution.
	//
	// Set dvars to true if the string contains dialogue variables, else false.
	//

	size_t p1 ;
	size_t p2 ;

	string var ;
	string val ;

	const string validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#$@" ;
	p1 = 0 ;

	dvars = false ;
	while ( true )
	{
		p1 = s.find( '&', p1 ) ;
		if ( p1 == string::npos || p1 == s.size() - 1 ) { break ; }
		++p1 ;
		if ( s[ p1 ] == '&' )
		{
			s.erase( p1, 1 ) ;
			p1 = s.find_first_not_of( '&', p1 ) ;
			continue ;
		}
		p2  = s.find_first_not_of( validChars, p1 ) ;
		if ( p2 == string::npos ) { p2 = s.size() ; }
		var = upper( s.substr( p1, p2-p1 ) ) ;
		if ( isvalidName( var ) )
		{
			val = getDialogueVar( err, var ) ;
			if ( !err.error() )
			{
				trim_right( val ) ;
				if ( p2 < s.size() && s[ p2 ] == '.' )
				{
					s.replace( p1-1, var.size()+2, val ) ;
				}
				else
				{
					s.replace( p1-1, var.size()+1, val ) ;
				}
				p1 = p1 + val.size() - 1 ;
				dvars = true ;
			}
		}
	}
	return s ;
}


void abc::add_pdc( const pdc& m_pdc )
{
	++abc_maxh ;
	pdcList.push_back( m_pdc ) ;
}


void abc::display_abc_sel( WINDOW* win )
{
	wattrset( win, cuaAttr[ AB ] | abc_intens ) ;
	mvwaddstr( win, 0, abc_col, abc_desc.c_str() ) ;
}


void abc::display_abc_unsel( WINDOW* win,
			     bool hi_mnem )
{
	wattrset( win, cuaAttr[ ABU ] | abc_intens ) ;
	mvwaddstr( win, 0, abc_col, abc_desc.c_str() ) ;

	if ( abc_mnem1 > 0 )
	{
		uint i = abc_mnem1 - 1 ;
		if ( hi_mnem )
		{
			wattrset( win, TURQ | abc_intens | A_UNDERLINE ) ;
		}
		else
		{
			wattrset( win, cuaAttr[ ABU ] | abc_intens | A_UNDERLINE ) ;
		}
		mvwaddch( win, 0, abc_col+i, abc_desc[ i ] ) ;
	}
}


void abc::display_pd( errblock& err,
		      uint zscrmaxd,
		      uint zscrmaxw,
		      const string& zvars,
		      uint win_row,
		      uint win_col,
		      uint row,
		      uint home )
{
	//
	// Display pull-down and highlight choice cursor is placed on if not unavailable.
	// Resize pull-down window if necessary.
	//
	// Row is the physical position on the screen, as is abc_row1 and abc_col1.
	//

	int i ;

	uint w_row ;
	uint w_col ;

	abc_row1 = win_row + 1 ;
	abc_col1 = win_col + abc_col ;

	size_t maxw = 0 ;

	if ( ( pdcList.size() + 2 ) > zscrmaxd )
	{
		err.seterrid( TRACE_INFO(), "PSYS013R" ) ;
		return ;
	}

	if ( ( win_row + pdcList.size() + 2 ) >= zscrmaxd )
	{
		abc_row1 = zscrmaxd - pdcList.size() - 2 ;
	}

	if ( home > 0 )
	{
		row = abc_row1 + home ;
	}

	if ( row > abc_row1 && row < ( abc_row1 + pdcList.size() + 1 ) )
	{
		currChoice = row - abc_row1 - 1 ;
	}

	for ( auto it = pdcList.begin() ; it != pdcList.end() ; ++it )
	{
		if ( it->pdc_dvars )
		{
			it->pdc_xdesc = sub_vars( err, it->pdc_desc, it->pdc_dvars ) ;
			if ( !it->pdc_dvars )
			{
				it->pdc_desc = "" ;
			}
			if ( it->pdc_xdesc.size() > 64 )
			{
				it->pdc_xdesc.erase( 64 ) ;
			}
		}
		maxw = max( it->pdc_xdesc.size(), maxw ) ;
	}

	if ( ( win_col + abc_col + maxw + 10 ) >= zscrmaxw )
	{
		abc_col1 = zscrmaxw - maxw - 10 ;
	}

	if ( !pd_created )
	{
		abc_maxw = maxw ;
		create_window( abc_row1, abc_col1 ) ;
		panel = new_panel( win ) ;
		pd_created = true ;
	}
	else if ( abc_maxw != maxw )
	{
		abc_maxw = maxw ;
		WINDOW* win1 = win ;
		create_window( abc_row1, abc_col1 ) ;
		replace_panel( panel, win ) ;
		delwin( win1 ) ;
	}
	else
	{
		getbegyx( win, w_row, w_col ) ;
		if ( w_row != abc_row1 || w_col != abc_col1 )
		{
			mvwin( win, abc_row1, abc_col1 ) ;
		}
	}

	i = 1 ;
	for ( auto it = pdcList.begin() ; it != pdcList.end() ; ++it, ++i )
	{
		if ( it->pdc_unavail != "" && getDialogueVar( err, it->pdc_unavail ) == "1" )
		{
			it->display_pdc_unavail( win,
						 abc_maxw,
						 PUC,
						 i ) ;
		}
		else
		{
			it->display_pdc_avail( win,
					       abc_maxw,
					       ( currChoice == ( i - 1 ) ) ? AMT : PAC,
					       i ) ;
		}
	}

	choiceVar = zvars ;
	show_panel( panel ) ;
}


void abc::hide_pd()
{
	hide_panel( panel ) ;
}


void abc::get_pd_home( uint& row,
		       uint& col )
{
	row = abc_row1 + 1 ;
	col = abc_col1 + 2 ;
}


pdc abc::retrieve_choice( errblock& err )
{
	if ( pdcList[ currChoice ].pdc_unavail != "" &&
	     getDialogueVar( err, pdcList[ currChoice ].pdc_unavail ) == "1" )
	{
		if ( choiceVar != "" )
		{
			putDialogueVar( err, choiceVar, "" ) ;
		}
		return pdc() ;
	}

	if ( choiceVar != "" )
	{
		putDialogueVar( err, choiceVar, d2ds( currChoice + 1 ) ) ;
	}

	return pdcList.at( currChoice ) ;
}


int abc::retrieve_choice_number()
{
	return currChoice ;
}


void abc::get_msg_position( uint& row,
			    uint& col,
			    uint win_col )
{
	row = abc_maxh + 2 ;
	col = abc_col1 - win_col ;
}


const string& abc::get_abc_desc()
{
	return abc_desc ;
}


bool abc::cursor_on_abc( uint col )
{
	return ( col >= abc_col && col < ( abc_col + abc_desc.size() ) ) ;
}


bool abc::cursor_on_pulldown( uint row,
			      uint col )
{
	uint w_row ;
	uint w_col ;

	getbegyx( win, w_row, w_col ) ;

	return ( ( row >  w_row && row <= ( w_row + abc_maxh ) )  &&
		 ( col >= w_col && col <  ( w_col + abc_maxw + 10 ) ) ) ;
}


/******************************************************************************************************************/
/*                                                BOX CLASS                                                       */
/******************************************************************************************************************/

Box::Box( errblock& err,
	  int MAXW,
	  int MAXD,
	  const string& line ) : Box()
{
	//
	// Format of BOX entry in panels (FORMAT 1 VERSION 1 )
	// BOX  row col width depth Colour   B-title
	// w1   w2  w3  w4    w5    w6       w7
	// BOX  7   7   41    22    WHITE    "Test Dynamic Area 1"
	//

	int row    ;
	int col    ;
	int width  ;
	int depth  ;

	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;
	string w6 ;

	string title ;

	w2 = word( line, 2 ) ;
	w3 = word( line, 3 ) ;
	w4 = upper( word( line, 4 ) ) ;
	w5 = upper( word( line, 5 ) ) ;
	w6 = upper( word( line, 6 ) ) ;

	if ( !datatype( w2, 'W' ) ||
	     !datatype( w3, 'W' ) )
	{
		err.seterrid( TRACE_INFO(), "PSYE019E" ) ;
		return ;
	}

	row = ds2d( w2 ) ;
	col = ds2d( w3 ) ;

	if ( isnumeric( w4 ) )                      { width = ds2d( w4 )     ; }
	else if ( w4 == "MAX" )                     { width = MAXW - col + 1 ; }
	else if ( w4.compare( 0, 4, "MAX-" ) == 0 ) { width = MAXW - col - ds2d( substr( w4, 5 ) ) + 1 ; }
	else                                        { err.seterrid( TRACE_INFO(), "PSYE019E", w4 ) ; return          ; }

	if ( isnumeric( w5 ) )                      { depth = ds2d( w5 )     ; }
	else if ( w5 == "MAX" )                     { depth = MAXD - row + 1 ; }
	else if ( w5.compare( 0, 4, "MAX-" ) == 0 ) { depth = MAXD - row - ds2d( substr( w5, 5 ) ) + 1 ; }
	else                                        { err.seterrid( TRACE_INFO(), "PSYE019E", w5 ) ; return          ; }

	if ( row > (MAXD - 2) )
	{
		err.seterrid( TRACE_INFO(), "PSYE034A" ) ;
		return ;
	}

	title = qstring( err, subword( line, 7 ) ) ;
	if ( err.error() ) { return ; }

	if ( width > ( MAXW - col + 1 ) ) { width = ( MAXW - col + 1 ) ; } ;
	if ( depth > ( MAXD - row + 1 ) ) { depth = ( MAXD - row + 1 ) ; } ;

	box_row    = row - 1 ;
	box_col    = col - 1 ;
	box_width  = width   ;
	box_depth  = depth   ;
	box_colour = colourName[ w6 ] ;
	if ( title != "" )
	{
		box_title = " " + title + " "  ;
	}
}


void Box::display_box( WINDOW* win,
		       string title,
		       uint colour,
		       uint intens )
{
	wattrset( win, colour | intens | box_intens ) ;
	draw_box( win, title ) ;
}


void Box::display_box( WINDOW* win,
		       string title )
{
	wattrset( win, box_colour | box_intens ) ;
	draw_box( win, title ) ;
}


void Box::draw_box( WINDOW* win,
		    string& title )
{
	if ( title.size() > box_width - 4 )
	{
		title.erase( box_width - 4 ) ;
	}

	mvwaddch( win, box_row, box_col, ACS_ULCORNER ) ;
	mvwaddch( win, box_row, box_col + box_width - 1, ACS_URCORNER ) ;

	mvwaddch( win, box_row + box_depth - 1, box_col, ACS_LLCORNER ) ;
	mvwaddch( win, box_row + box_depth - 1, box_col + box_width - 1, ACS_LRCORNER ) ;

	mvwhline( win, box_row, (box_col + 1), ACS_HLINE, (box_width - 2) ) ;
	mvwhline( win, (box_row + box_depth - 1), (box_col + 1), ACS_HLINE, (box_width - 2) ) ;

	mvwvline( win, (box_row + 1), box_col, ACS_VLINE, (box_depth - 2) ) ;
	mvwvline( win, (box_row + 1), (box_col + box_width - 1 ), ACS_VLINE, (box_depth - 2) ) ;

	mvwaddstr( win, box_row, (box_col + ( ( box_width - title.size() ) / 2 ) ), title.c_str() ) ;
}
