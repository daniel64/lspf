/*
  Copyright (c) 2015 Daniel John Erdos

  This program is free software; you can redistribute it and/or modifyf
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

void field::field_init( errblock& err, int MAXW, int MAXD, const string& line )
{
	// Format of field entry in panels (FORMAT 1 VERSION 1 )
	// FIELD row col len cuaAttr opts field_name
	// w1    w2  w3  w4  w5      w6   w7

	// FIELD 3  14  90  NEF CAPS(On),pad('_'),just(left),numeric(off),skip(on) ZCMD

	int row ;
	int col ;
	int len ;
	int ws  ;

	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;
	string opts ;

	attType fType  ;

	err.setRC( 0 ) ;

	ws = words( line ) ;
	if ( ws < 7 )
	{
		err.seterrid( "PSYE035Q" ) ;
		return ;
	}

	w2 = word( line, 2 ) ;
	w3 = word( line, 3 ) ;
	w4 = word( line, 4 ) ;
	w5 = word( line, 5 ) ;

	opts = subword( line, 6, ws-6 ) ;
	err.setUserData( word( line, ws ) ) ;

	if ( isnumeric( w2 ) )                      { row = ds2d( w2 ) ; }
	else if ( w2 == "MAX" )                     { row = MAXD       ; }
	else if ( w2.compare( 0, 4, "MAX-" ) == 0 ) { row = MAXD - ds2d( substr( w2, 5 ) )    ; }
	else                                        { err.seterrid( "PSYE031B", w2 ) ; return ; }

	if ( row > MAXD )
	{
		err.seterrid( "PSYE031A", d2ds( row ), d2ds( MAXD ) ) ;
		return ;
	}

	if ( isnumeric( w3 ) )                      { col = ds2d( w3 ) ; }
	else if ( w3 == "MAX" )                     { col = MAXW       ; }
	else if ( w3.compare( 0, 4, "MAX-" ) == 0 ) { col = MAXW - ds2d( substr( w3, 5 ) )    ; }
	else                                        { err.seterrid( "PSYE031B", w3 ) ; return ; }

	if ( isnumeric( w4 ) )                      { len = ds2d( w4 )     ; }
	else if ( w4 == "MAX" )                     { len = MAXW - col + 1 ; }
	else if ( w4.compare( 0, 4, "MAX-" ) == 0 ) { len = MAXW - col - ds2d( substr( w4, 5 ) ) + 1 ; }
	else                                        { err.seterrid( "PSYE031B", w4 ) ; return        ; }

	if ( w5 == "PWD" )
	{
		fType     = NEF  ;
		field_pwd = true ;
	}
	else if ( cuaAttrName.count( w5 ) == 0 )
	{
		if ( noncuaAttrName.count( w5 ) == 0 )
		{
			err.seterrid( "PSYE032F", w5 ) ;
			return ;
		}
		fType         = noncuaAttrName[ w5 ] ;
		field_cua     = NONE ;
		field_colour1 = ( w5 == "INPUT" ) ? RED : WHITE ;
		field_colour2 = field_colour1 ;
	}
	else
	{
		fType         = cuaAttrName[ w5 ] ;
		field_cua     = fType ;
		field_colour1 = cuaAttr[ fType ] ;
	}

	field_row    = row-1 ;
	field_col    = col-1 ;
	field_length = len   ;
	field_endcol = field_col + field_length ;
	field_input  = ( attrUnprot.count( fType ) > 0 ) ;

	field_opts( err, opts ) ;
}


void field::field_opts( errblock& err, string& opts )
{
	// CAPS(ON,OFF)
	// JUST(LEFT,RIGHT,ASIS)
	// NUMERIC(ON,OFF)
	// PAD(char,NULLS,USER)
	// SKIP(ON,OFF)
	// NOJUMP(ON,OFF)

	size_t p1 ;
	size_t p2 ;

	char quote ;

	string t   ;

	err.setRC( 0 ) ;

	if ( opts == "NONE" ) { return ; }

	opts = "," + opts ;
	p1   = opts.find( ",CAPS(" ) ;
	if ( p1 != string::npos )
	{
		p2 = opts.find( ')', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		t = opts.substr( p1+6, p2-p1-6 ) ;
		trim( t ) ;
		if ( t.size() == 0 )
		{
			err.seterrid( "PSYE031G" ) ;
			return ;
		}
		if      ( t == "ON"  ) { field_caps = true  ; }
		else if ( t == "OFF" ) { field_caps = false ; }
		else
		{
			err.seterrid( "PSYE035K", t ) ;
			return ;
		}
		opts = opts.erase( p1, p2-p1+1 ) ;
	}

	p1 = opts.find( ",JUST(" ) ;
	if ( p1 != string::npos )
	{
		p2 = opts.find( ')', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		t = opts.substr( p1+6, p2-p1-6 ) ;
		trim( t ) ;
		if ( t.size() == 0 )
		{
			err.seterrid( "PSYE031G" ) ;
			return ;
		}
		if      ( t == "LEFT"  ) { field_just = 'L' ; }
		else if ( t == "RIGHT" ) { field_just = 'R' ; }
		else if ( t == "ASIS"  ) { field_just = 'A' ; }
		else
		{
			err.seterrid( "PSYE035L", t ) ;
			return ;
		}
		opts = opts.erase( p1, p2-p1+1 ) ;
	}

	p1 = opts.find( ",NUMERIC(" ) ;
	if ( p1 != string::npos )
	{
		p2 = opts.find( ')', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		t = opts.substr( p1+9, p2-p1-9 ) ;
		trim( t ) ;
		if ( t.size() == 0 )
		{
			err.seterrid( "PSYE031G" ) ;
			return ;
		}
		if      ( t == "ON"  ) { field_numeric = true  ; }
		else if ( t == "OFF" ) { field_numeric = false ; }
		else
		{
			err.seterrid( "PSYE035M", t ) ;
			return ;
		}
		opts = opts.erase( p1, p2-p1+1 ) ;
	}

	p1 = opts.find( ",PAD(" ) ;
	if ( p1 != string::npos )
	{
		p2 = opts.find( ')', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		t = opts.substr( p1+5, p2-p1-5 ) ;
		trim( t ) ;
		if ( t == "" )
		{
			err.seterrid( "PSYE031G" ) ;
			return ;
		}
		else if ( t == "USER" )
		{
			field_paduser = true ;
		}
		else if ( t == "NULLS" )
		{
			field_padchar = 0x00 ;
		}
		else
		{
			quote = t.front() ;
			if ( quote == '\'' || quote == '"' )
			{
				if ( t.size() > 1 && t.back() != quote )
				{
					err.seterrid( "PSYE033F" ) ;
					return ;
				}
				t.pop_back()    ;
				t.erase( 0, 1 ) ;
			}
			if ( t.size() != 1 )
			{
				err.seterrid( "PSYE035N", t ) ;
				return ;
			}
			field_padchar = t.front() ;
		}
		opts = opts.erase( p1, p2-p1+1 ) ;
	}

	p1 = opts.find( ",SKIP(" ) ;
	if ( p1 != string::npos )
	{
		p2 = opts.find( ')', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		t = opts.substr( p1+6, p2-p1-6 ) ;
		trim( t ) ;
		if ( t.size() == 0 )
		{
			err.seterrid( "PSYE031G" ) ;
			return ;
		}
		if      ( t == "ON" )  { field_skip = true  ; }
		else if ( t == "OFF" ) { field_skip = false ; }
		else
		{
			err.seterrid( "PSYE035O", t ) ;
			return ;
		}
		opts = opts.erase( p1, p2-p1+1 ) ;
	}

	p1 = opts.find( ",NOJUMP(" ) ;
	if ( p1 != string::npos )
	{
		p2 = opts.find( ')', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		t = opts.substr( p1+8, p2-p1-8 ) ;
		trim( t ) ;
		if ( t.size() == 0 )
		{
			err.seterrid( "PSYE031G" ) ;
			return ;
		}
		if      ( t == "ON" )  { field_nojump = true  ; }
		else if ( t == "OFF" ) { field_nojump = false ; }
		else
		{
			err.seterrid( "PSYE035P", t ) ;
			return ;
		}
		opts = opts.erase( p1, p2-p1+1 ) ;
	}

	if ( trim( opts ) != "" )
	{
		err.seterrid( "PSYE032H", opts ) ;
		return ;
	}
}


void dynArea::dynArea_init( errblock& err,
			    int MAXW,
			    int MAXD,
			    const string& line,
			    const string& da_dataIn,
			    const string& da_dataOut )
{
	// Format of DYNAREA entry in panels (FORMAT 1 VERSION 1 )
	// DYNAREA row col width depth   A-name S-name USERMOD() DATAMOD() SCROLL()

	// w1      w2         w3   w4    w5     w6     w7      <--------------keywords-------------->
	// DYNAREA MAX-10 MAX-20   MAX   MAX-6  ZAREA  ZSHADOW USERMOD(03) DATAMOD(04) SCROLL(OFF|ON)

	// if width=MAX  width=MAXW-col+1
	// if depth=MAX  depth=MAXD-row+1

	// USERMOD - field touched
	// DATAMOD - field changed

	// USERMOD but not DATAMOD - Attr changed to USERMOD even if the field was changed
	// DATAMOD but not USERMOD - Attr changed to DATAMOD only if the field value has changed
	// USERMOD and DATAMOD     - Attr changed to USERMOD if no change to field value, else DATAMOD
	// Neither                 - Attr unchanged

	int row   ;
	int col   ;
	int width ;
	int depth ;

	size_t p1 ;
	size_t p2 ;

	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;
	string w7 ;
	string t  ;
	string rest ;

	w2   = word( line, 2 ) ;
	w3   = word( line, 3 ) ;
	w4   = word( line, 4 ) ;
	w5   = word( line, 5 ) ;
	w7   = word( line, 7 ) ;
	rest = " " + subword( line, 8 ) ;

	err.setRC( 0 ) ;

	if ( isnumeric( w2 ) )                      { row = ds2d( w2  ) ; }
	else if ( w2 == "MAX" )                     { row = MAXD        ; }
	else if ( w2.compare( 0, 4, "MAX-" ) == 0 ) { row = MAXD - ds2d( substr( w2, 5 ) )    ; }
	else                                        { err.seterrid( "PSYE031B", w2 ) ; return ; }

	if ( isnumeric( w3 ) )                      { col = ds2d( w3 )  ; }
	else if ( w3 == "MAX" )                     { col = MAXW        ; }
	else if ( w3.compare( 0, 4, "MAX-" ) == 0 ) { col = MAXW - ds2d( substr( w3, 5 ) )    ; }
	else                                        { err.seterrid( "PSYE031B", w3 ) ; return ; }

	if ( isnumeric( w4 ) )                      { width = ds2d( w4  )    ; }
	else if ( w4 == "MAX" )                     { width = MAXW - col + 1 ; }
	else if ( w4.compare( 0, 4, "MAX-" ) == 0 ) { width = MAXW - col - ds2d( substr( w4, 5 ) ) + 1 ; }
	else                                        { err.seterrid( "PSYE031B", w4 ) ; return          ; }

	if ( isnumeric( w5 ) )                      { depth = ds2d( w5 )     ; }
	else if ( w5 == "MAX" )                     { depth = MAXD - row + 1 ; }
	else if ( w5.compare( 0, 4, "MAX-" ) == 0 ) { depth = MAXD - row - ds2d( substr( w5, 5 ) ) + 1 ; }
	else                                        { err.seterrid( "PSYE031B", w5 ) ; return          ; }

	if ( row > MAXD )
	{
		err.seterrid( "PSYE032A", d2ds( row ), d2ds( MAXD ) ) ;
		return ;
	}
	if ( width > (MAXW - col+1) ) { width = (MAXW - col+1) ; }
	if ( depth > (MAXD - row+1) ) { depth = (MAXD - row+1) ; }

	if ( w7 == "" )
	{
		err.seterrid( "PSYE032E" ) ;
		return ;
	}

	if ( da_dataIn != "" )
	{
		dynArea_dataInsp = true ;
	}

	p1 = rest.find( " USERMOD(" ) ;
	if ( p1 != string::npos )
	{
		p2 = rest.find( ')', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		t = rest.substr( p1+9, p2-p1-9 ) ;
		trim( t ) ;
		if ( t.size() == 0 )
		{
			err.seterrid( "PSYE031G" ) ;
			return ;
		}
		dynArea_userModsp = true ;
		if      ( t.size() == 1 ) { dynArea_UserMod = t[0]          ; }
		else if ( t.size() == 2 ) { dynArea_UserMod = xs2cs( t )[0] ; }
		else                      { err.seterrid( "PSYE032B" )      ; return ; }
		rest = rest.erase( p1, p2-p1+1 ) ;
	}

	p1 = rest.find( " DATAMOD(" ) ;
	if ( p1 != string::npos )
	{
		p2 = rest.find( ')', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		t = rest.substr( p1+9, p2-p1-9 ) ;
		trim( t ) ;
		if ( t.size() == 0 )
		{
			err.seterrid( "PSYE031G" ) ;
			return ;
		}
		dynArea_dataModsp = true ;
		if      ( t.size() == 1 ) { dynArea_DataMod = t[0]          ; }
		else if ( t.size() == 2 ) { dynArea_DataMod = xs2cs( t )[0] ; }
		else                      { err.seterrid( "PSYE032B" )      ; return ; }
		rest = rest.erase( p1, p2-p1+1 ) ;
	}

	t = parseString( err, rest, "SCROLL()" ) ;
	if ( err.error() ) { return ; }
	if ( t == "ON" )
	{
		dynArea_scroll = true ;
	}
	else if ( t == "OFF" || t == "" )
	{
		dynArea_scroll = false ;
	}
	else
	{
		err.seterrid( "PSYE032C", t ) ;
		return ;
	}

	if ( trim( rest ) != "" )
	{
		err.seterrid( "PSYE032H", rest ) ;
		return ;
	}

	dynArea_row   = row-1 ;
	dynArea_col   = col-1 ;
	dynArea_width = width ;
	dynArea_depth = depth ;
	dynArea_shadow_name = w7 ;

	dynArea_inAttrs = da_dataIn ;
	dynArea_Attrs   = da_dataIn + da_dataOut ;
}


void field::field_reset()
{
	// Clear the list of touched attribute positions before a panel display

	field_usermod.clear() ;
}


bool field::edit_field_insert( WINDOW* win,
			       char ch,
			       char def_schar,
			       int col,
			       map<unsigned char, uint>& ddata_map,
			       map<unsigned char, uint>& schar_map )
{
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_dataInsp is true and
	// there is an input attribute byte at the start of the field

	// Use the pad character or nulls to fill fields when the cursor is past the end if the field value.
	// Nulls are removed from the field before further processing by the application.

	uint pos ;

	size_t p1  ;
	size_t p2  ;

	dynArea* da ;
	const char nulls(0x00) ;

	char upad = field_paduser ? field_paduchar : field_padchar ;

	pos = (col - field_col ) ;
	if ( pos >= field_value.size() )
	{
		field_value.resize( pos+1, ( upad == ' ' ) ? ' ' : nulls ) ;
	}

	if ( field_dynArea )
	{
		da = field_dynArea ;
		p2 = field_value.find_first_of( da->dynArea_Attrs, pos ) ;
		p2 = ( p2 == string::npos ) ? field_value.size() - 1 : p2 - 1 ;
		if ( field_value[ p2 ] != ' '  &&
		     field_value[ p2 ] != nulls ) { return false ; }
		field_value.erase( p2, 1 ) ;
		field_shadow_value.erase( p2, 1 ) ;
		p1 = field_value.find_last_of( da->dynArea_inAttrs, pos ) ;
		field_usermod.insert( p1 ) ;
		if ( field_value[ pos ] == nulls )
		{
			field_value[ pos ] = ch ;
		}
		else
		{
			field_value.insert( pos, 1, ch ) ;
			field_shadow_value.insert( pos, 1, def_schar ) ;
		}
	}
	else
	{
		if ( field_length == field_value.size() ) { return false ; }
		if ( field_value[ pos ] == nulls )
		{
			field_value[ pos ] = ch ;
		}
		else
		{
			field_value.insert( pos, 1, ch ) ;
		}
	}

	display_field( win, ddata_map, schar_map ) ;
	field_changed = true ;
	return true ;
}


bool field::edit_field_replace( WINDOW* win,
				char ch,
				char def_schar,
				int col,
				map<unsigned char, uint>& ddata_map,
				map<unsigned char, uint>& schar_map )
{
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_dataInsp is true and
	// there is an input attribute byte at the start of the field

	// Use the pad character or nulls to fill fields when the cursor is past the end if the field value
	// These are then removed before further processing by the application

	size_t p1 ;

	uint pos ;

	const char nulls(0x00) ;

	char upad = field_paduser ? field_paduchar : field_padchar ;

	pos = (col - field_col ) ;
	if ( pos >= field_value.size() )
	{
		field_value.resize( pos+1, ( upad == ' ' ) ? ' ' : nulls ) ;
	}

	if ( field_dynArea )
	{
		if ( field_value[ pos ] != nulls && !isprint( field_value[ pos ] ) )
		{
			return false ;
		}
		p1 = field_value.find_last_of( field_dynArea->dynArea_inAttrs, pos ) ;
		field_usermod.insert( p1 ) ;
		field_shadow_value[ pos ] = def_schar ;
	}
	field_value[ pos ] = ch ;

	display_field( win, ddata_map, schar_map ) ;
	field_changed = true ;
	return true ;
}


void field::edit_field_delete( WINDOW* win,
			       int col,
			       map<unsigned char, uint>& ddata_map,
			       map<unsigned char, uint>& schar_map )
{
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_dataInsp is true
	// and there is an input attribute byte at the start of the field.

	uint pos ;

	size_t p1 ;
	size_t p2 ;

	const char nulls(0x00) ;

	dynArea* da ;

	pos = col - field_col ;
	if ( pos > field_value.size() ) { return ; }
	if ( field_value[ pos ] != nulls && !isprint( field_value[ pos ] ) ) { return ; }

	if ( field_dynArea )
	{
		da = field_dynArea ;
		p1 = field_value.find_last_of( da->dynArea_inAttrs, pos ) ;
		field_usermod.insert( p1 ) ;
		p2 = field_value.find_first_of( da->dynArea_Attrs, pos )  ;
		if ( p2 == string::npos ) { p2 = field_value.size() ; }
		field_value.insert( p2, 1, ' ' ) ;
		field_shadow_value.insert( p2, 1, 0xFE ) ;
		field_shadow_value.erase( pos, 1 ) ;
	}

	field_value.erase( pos, 1 ) ;
	field_blank( win ) ;
	display_field( win, ddata_map, schar_map ) ;
	field_changed = true ;
}


int field::edit_field_backspace( WINDOW* win,
				 int col,
				 map<unsigned char, uint>& ddata_map,
				 map<unsigned char, uint>& schar_map )
{
	// If this is a dynamic area, we know it is an input field so pos > 0 (to allow for the input attribute byte)

	size_t pos ;

	pos = col - field_col ;
	if ( pos > field_value.size() ) { return --col ; }

	if ( field_dynArea && field_dynArea->dynArea_Attrs.find( field_value[ pos-1 ] ) != string::npos )
	{
		return col ;
	}

	--col ;
	edit_field_delete( win, col, ddata_map, schar_map ) ;
	return col ;
}


void field::field_erase_eof( WINDOW* win,
			     uint col,
			     map<unsigned char, uint>& ddata_map,
			     map<unsigned char, uint>& schar_map )
{
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_dataInsp is true,
	// and there is an input attribute byte at the start of the field

	int pos ;

	size_t p1 ;
	size_t p2 ;

	dynArea* da ;

	if ( ( field_col + field_value.size() ) < col ) return ;
	pos = (col - field_col) ;

	if ( field_dynArea )
	{
		da = field_dynArea ;
		p1 = field_value.find_last_of( da->dynArea_inAttrs, pos ) ;
		field_usermod.insert( p1 ) ;
		p2 = field_value.find_first_of( da->dynArea_Attrs, pos ) ;
		if ( p2 == string::npos ) { p2 = field_value.size() ; }
		field_value.replace( pos, p2-pos, p2-pos, ' ' )  ;
		field_shadow_value.replace( pos, p2-pos, p2-pos, 0xFE ) ;
	}
	else
	{
		field_blank( win ) ;
		field_value.erase( pos+1 ) ;
	}

	display_field( win, ddata_map, schar_map ) ;
	field_changed = true ;
}


void field::field_blank( WINDOW* win )
{
	char  upad   = field_paduser ? field_paduchar : field_padchar ;
	char* blanks = new char[ field_length+1 ] ;

	const char nulls(0x00) ;

	for ( unsigned int i = 0 ; i < field_length ; ++i )
	{
		blanks[ i ] = ( upad == nulls ) ? ' ' : upad ;
	}

	blanks[ field_length ] = nulls ;
	wstandend( win ) ;
	mvwaddstr( win, field_row, field_col, blanks ) ;

	delete[] blanks ;
}


void field::field_remove_nulls_da()
{
	// Remove all nulls from a dynamic area field

	// For dynamic areas, remove nulls from input fields that have been touched or changed.
	// Trailing nulls are changed to blanks.  Keep field size constant by adding blanks at
	// the end of the input field when removing nulls.  In this case, change shadow variable to 0xFF
	// so we know where the real data ends.

	// Keep shadow variable in sync

	size_t p1 ;
	size_t p2 ;
	size_t p3 ;

	const char nulls(0x00) ;

	dynArea* da = field_dynArea ;

	p1 = 0 ;

	for ( auto it = field_usermod.begin() ; it != field_usermod.end() ; ++it )
	{
		p1 = *it + 1 ;
		p3 = field_value.find_first_of( da->dynArea_Attrs, p1 ) ;
		p3 = ( p3 == string::npos ) ? field_value.size() - 1 : p3 - 1 ;
		p2 = field_value.find_last_not_of( nulls, p3 ) ;
		if ( p2 == string::npos )
		{
			field_value.replace( p1, p3-p1+1, p3-p1+1, ' ' ) ;
			field_shadow_value.replace( p1, p3-p1+1, p3-p1+1, 0xFF ) ;
			break ;
		}
		if ( p3 > p2 )
		{
			field_value.replace( p2+1, p3-p2, p3-p2, ' ' ) ;
			field_shadow_value.replace( p2+1, p3-p2, p3-p2, 0xFF ) ;
		}
		while ( p1 < p2 )
		{
			if ( field_value[ p1 ] == nulls )
			{
				field_value.erase( p1, 1 ) ;
				field_value.insert( p3, 1, ' ' ) ;
				field_shadow_value.erase( p1, 1 ) ;
				field_shadow_value.insert( p3, 1, 0xFF ) ;
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


int field::end_of_field( WINDOW* win, uint col )
{
	// If this is a dynamic area, we know at this point this is an input field.
	// Strip trailing nulls if not a dynamic area

	size_t pos ;
	size_t p1  ;
	size_t p2  ;

	string padc = "" ;

	padc.push_back( 0x00 ) ;
	padc.push_back( 0x20 ) ;

	const char nulls(0x00) ;

	if ( field_dynArea )
	{
		pos = (col - field_col) ;
		p2  = field_value.find_first_of( field_dynArea->dynArea_Attrs, pos ) ;
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
			return ( field_col + field_value.size()-1 ) ;
		}
		else
		{
			return ( field_col + field_value.size() ) ;
		}
	}
	return 0 ;
}


bool field::field_dyna_input( uint col )
{
	// When this routine is called, we only know the field is a dynamic area.
	// Check if we are on a field attribute byte
	// Find the previous field attribute.
	// If found, test to see if it is an input attribute

	size_t pos ;
	size_t p1  ;

	dynArea* da = field_dynArea ;

	if ( !da->dynArea_dataInsp ) { return false ; }

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
	// When this routine is called, we know the field is a dynamic area

	size_t pos ;
	size_t p1  ;

	pos = ( col < field_col ) ? 0 : (col - field_col) ;

	p1 = field_value.find_first_of( field_dynArea->dynArea_inAttrs, pos ) ;
	if ( p1 == string::npos ) { return -1 ; }
	return p1+1 ;
}


void field::field_attr( errblock& err, string attrs, bool chng_once )
{
	// Change field attributes.  Only parameters specified are changed, other
	// attributes are left unchanged.
	// Used for .ATTR() panel function.

	// Format:

	// TYPE(CUA) (change to/from PS not allowed)
	// COLOUR(RED/GREEN/YELLOW/BLUE/MAGENTA/TURQ/WHITE)
	// INTENS(LOW/HIGH)
	// HILITE(NONE/BLINK/REVERSE/USCORE)

	// For NON-CUA field_colour1, default to the current CUA value in case only one attribute is changed

	// Hex format for field_colour1:
	// 00 X0 00 00 - X is the INTENSITY (inmask)
	// 00 0X 00 00 - X is the HILITE (himask)
	// 00 00 XX 00 - X is the COLOUR (clmask)

	const uint clmask = RED | GREEN | YELLOW | BLUE | MAGENTA | TURQ | WHITE ;
	const uint himask = A_BLINK  | A_REVERSE | A_UNDERLINE ;
	const uint inmask = A_NORMAL | A_BOLD    | A_INVIS ;

	char_attrs temp ;

	temp.setattr( err, attrs ) ;
	if ( err.error() ) { return ; }

	if ( temp.typeChange )
	{
		if      ( temp.type == TEXT   ) { field_input  = false ; }
		else if ( temp.type == OUTPUT ) { field_input  = false ; }
		else if ( temp.type == INPUT  ) { field_input  = true  ; }
		else
		{
			if ( temp.type == PS || field_cua == PS )
			{
				err.seterrid( "PSYE035A" ) ;
				return ;
			}
			if ( temp.typecua )
			{
				field_cua     = temp.type ;
				field_colour1 = cuaAttr[ field_cua ] ;
			}
			else
			{
				field_cua = NONE ;
			}
			field_input = ( attrUnprot.count( field_cua ) > 0 ) ;
			return ;
		}
	}

	field_colour1 &= ~clmask     ;
	field_colour1 |= temp.colour ;

	field_colour1 &= ~inmask     ;
	field_colour1 |= temp.intens ;

	field_colour1 &= ~himask     ;
	field_colour1 |= temp.hilite ;

	field_attr_once = chng_once ;
}


void field::field_attr()
{
	// Reset field attribute to use the CUA value or original non-CUA value

	field_colour1   = ( field_cua == NONE ) ? field_colour2 : cuaAttr[ field_cua ] ;
	field_attr_once = false ;
}


void field::field_prep_input()
{
	// Prepare the field to be copied to the function pool (non-dynamic area fields only)

	// Remove nulls from the field.
	// Apply just(right|left|asis) to non-dynamic area input/output fields
	//    JUST(LEFT/RIGHT) leading and trailing spaces are removed
	//    JUST(ASIS) Only trailing spaces are removed

	size_t p1 ;
	size_t p2 ;

	const char nulls(0x00) ;

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

	switch( field_just )
	{
		case 'L':
		case 'R': trim( field_value ) ;
			  break ;

		case 'A': trim_right( field_value ) ;
			  break ;
	}
}


void field::field_prep_display()
{
	// Prepare the field for display

	// Apply just(right|left|asis) to non-dynamic area input/output fields
	//     JUST(LEFT)  strip off leading and trailing spaces
	//     JUST(RIGHT) strip off trailing spaces only and pad to the left with nulls to size field_length
	//     JUST(ASIS)  no change

	switch( field_just )
	{
		case 'L': trim( field_value ) ;
			  break ;

		case 'R': trim_right( field_value ) ;
			  field_value = right( field_value, field_length, 0x00 ) ;
	}
}


void field::field_set_caps()
{
	// Set upper case if CAPS ON

	if ( field_caps )
	{
		iupper( field_value ) ;
	}
}


void field::field_update_datamod_usermod( string* darea, int offset )
{
	// Set the DATAIN attribute to DATAMOD for any dynamic area input field that has changed.
	// Set the DATAIN attribute to USERMOD for any dynamic area input field that has been touched.

	// Specified on DYNAREA...
	// USERMOD but not DATAMOD - Attr changed to USERMOD even if the field was changed
	// DATAMOD but not USERMOD - Attr changed to DATAMOD only if the field value has changed
	// USERMOD and DATAMOD     - Attr changed to USERMOD if no change to field value, else DATAMOD
	// Neither                 - Attr unchanged

	size_t p1 ;
	size_t p2 ;

	dynArea* da = field_dynArea ;

	if ( not da->dynArea_dataModsp && not da->dynArea_userModsp ) { return ; }

	for ( auto it = field_usermod.begin() ; it != field_usermod.end() ; ++it )
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
			if ( field_value.compare( p1, p2-p1, (*darea), offset+p1, p2-p1 ) != 0 )
			{
				field_value[ *it ] = da->dynArea_DataMod ;
			}
		}
	}
}


void field::display_field( WINDOW* win,
			   map<unsigned char, uint>& ddata_map,
			   map<unsigned char, uint>& schar_map )
{
	// For non-dynamic area fields: if an input field, truncate if value size > field size else for output fields
	// display field size bytes and leave field value unchanged (necessary for table display fields)

	// Display the null character as the field pad character and pad the field with the same character

	// Colour/hilite taken from the shadow byte if valid (ie. corresponds to a TYPE(CHAR) ATTR entry)
	// Intensity and default colour/hilite taken from DATAIN/DATAOUT.
	// If no DATAIN/DATAOUT and shadow byte is not valid, default to colour WHITE.

	// 00 X0 00 00 - X is the INTENSITY
	// 00 0X 00 00 - X is the HILITE
	// 00 00 XX 00 - X is the COLOUR

	// Call ncurses touchline() for the field row as the update does not always appear without it

	uint i ;

	uint intens = 0 ;
	uint attr2  = 0 ;
	uint attrd  = 0 ;
	uint colour = 0 ;

	const uint clmask = RED  | GREEN | YELLOW  | BLUE      | MAGENTA     |
			    TURQ | WHITE | A_BLINK | A_REVERSE | A_UNDERLINE ;

	char nullc = field_nulls ? '.' : ' ' ;
	char upad  = field_paduser ? field_paduchar : field_padchar ;
	char attr1 ;

	string t   ;

	const char nulls(0x00) ;

	string::iterator ita ;
	string::iterator its ;

	if ( !field_active ) { return ; }

	if ( field_dynArea )
	{
		dynArea* da = field_dynArea ;
		if ( da->dynArea_Attrs != "" )
		{
			its = field_shadow_value.begin() ;
			i   = 0 ;
			auto itc = schar_map.find( (*its) ) ;
			if ( itc == schar_map.end() )
			{
				wattrset( win, WHITE | field_intens ) ;
			}
			else
			{
				wattrset( win, itc->second | field_intens ) ;
			}
			attr1 = (*its) ;
			for ( ita = field_value.begin() ; ita != field_value.end() ; ++ita, ++its, ++i )
			{
				if ( attr1 != (*its) || attr2 != attrd )
				{
					itc = schar_map.find( (*its) ) ;
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
					attr1 = (*its) ;
					attr2 = attrd  ;
				}
				if ( (*ita) == nulls )
				{
					mvwaddch( win, field_row, field_col+i, nullc ) ;
				}
				else if ( da->dynArea_Attrs.find( (*ita) ) != string::npos )
				{
					mvwaddch( win, field_row, field_col+i, ' ' ) ;
					attrd  = ddata_map[ *ita ] ;
					intens = attrd & A_BOLD ;
					colour = attrd & clmask ;
				}
				else if ( isprint( (*ita) ) )
				{
					mvwaddch( win, field_row, field_col+i, (*ita) ) ;
				}
				else
				{
					mvwaddch( win, field_row, field_col+i, '.' ) ;
				}
			}
		}
		else
		{
			its  = field_shadow_value.begin() ;
			i    = 0 ;
			auto itc = schar_map.find( (*its) ) ;
			if ( itc == schar_map.end() )
			{
				wattrset( win, WHITE | field_intens ) ;
			}
			else
			{
				wattrset( win, itc->second | field_intens ) ;
			}
			attr1 = (*its) ;
			for ( ita = field_value.begin() ; ita != field_value.end() ; ++ita, ++its, ++i )
			{
				if ( attr1 != (*its) )
				{
					itc = schar_map.find( (*its) ) ;
					if ( itc == schar_map.end() )
					{
						wattrset( win, WHITE | field_intens ) ;
					}
					else
					{
						wattrset( win, itc->second | field_intens ) ;
					}
					attr1 = (*its) ;
				}
				if (  (*ita) == nulls )
				{
					mvwaddch( win, field_row, field_col+i, nullc ) ;
				}
				else if ( isprint( (*ita) ) )
				{
					mvwaddch( win, field_row, field_col+i, (*ita) ) ;
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
		wattrset( win, field_colour1 | field_intens ) ;
		if ( field_input )
		{
			if ( field_value.size() > field_length )
			{
				field_value.erase( field_length ) ;
			}
			t = field_value ;
		}
		else
		{
			if ( field_value.size() > field_length ) { t = field_value.substr( 0, field_length ) ; }
			else                                     { t = field_value                           ; }
		}
		if ( field_pwd )
		{
			t = string( field_value.size(), '*' ) ;
		}
		else
		{
			for ( ita = t.begin() ; ita != t.end() ; ++ita )
			{
				if      ( (*ita) == nulls   ) { (*ita) = ( upad == nulls ) ? ' ' : upad ; }
				else if ( !isprint( (*ita)) ) { (*ita) = '.'  ; }
			}
		}
		if ( field_input )
		{
			t.resize( field_length, ( upad == nulls ) ? ' ' : upad ) ;
		}
		mvwaddstr( win, field_row, field_col, t.c_str() ) ;
	}
	touchline( win, field_row, 1 ) ;
}


bool field::cursor_on_field( uint row, uint col )
{
	if ( field_row != row) { return false ; } ;
	if ( col >= field_col && col < field_endcol ) { return true ; }
	return false ;
}


void text::text_init( errblock& err, int MAXW, int MAXD, int& opt_field, const string& line )
{
	// Format of text entry in panels (FORMAT 1 VERSION 1 )
	// text row col cuaAttr (EXPAND) value
	// w1   w2  w3  w4      w5
	// text 3   1   OUTPUT  "COMMAND ===> "
	// OR
	// w1   w2  w3  w4      w5     w6
	// text 3   1   OUTPUT  EXPAND "-"
	// Use EXPAND to repeat the value to end of the screen (MAXW)

	int row   ;
	int col   ;
	int l     ;

	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;

	attType fType ;

	w2 = word( line, 2 ) ;
	w3 = word( line, 3 ) ;
	w4 = word( line, 4 ) ;
	w5 = word( line, 5 ) ;

	iupper( w2 ) ;
	iupper( w3 ) ;
	iupper( w4 ) ;

	if ( isnumeric( w2 ) )                      { row = ds2d( w2 ) ; }
	else if ( w2 == "MAX" )                     { row = MAXD       ; }
	else if ( w2.compare( 0, 4, "MAX-" ) == 0 ) { row = MAXD - ds2d( substr( w2, 5 ) )    ; }
	else                                        { err.seterrid( "PSYE031B", w2 ) ; return ; }

	if ( isnumeric( w3 ) )                      { col = ds2d( w3 ) ; }
	else if ( w3 == "MAX" )                     { col = MAXW       ; }
	else if ( w3.compare( 0, 4, "MAX-" ) == 0 ) { col = MAXW - ds2d( substr( w3, 5 ) )    ; }
	else                                        { err.seterrid( "PSYE031B", w3 ) ; return ; }

	if ( cuaAttrName.count( w4 ) == 0 )
	{
		err.seterrid( "PSYE032F", w4 ) ;
		return ;
	}
	fType = cuaAttrName[ w4 ] ;

	if ( row > MAXD )
	{
		err.seterrid( "PSYE036A", d2ds( row ), d2ds( MAXD ) ) ;
		return ;
	}
	if ( col > MAXW )
	{
		err.seterrid( "PSYE036B", d2ds( col ), d2ds( MAXW ) ) ;
		return ;
	}

	text_cua = fType   ;
	text_row = row - 1 ;
	text_col = col - 1 ;
	if ( upper( w5 ) == "EXPAND" )
	{
		text_value = strip( strip( subword( line, 6 ) ), 'B', '"' ) ;
		if ( text_value.size() == 0 )
		{
			err.seterrid( "PSYE041K" ) ;
			return ;
		}
		l = (MAXW - col) / text_value.size() + 1 ;
		text_value = substr( copies( text_value, l ), 1, MAXW-col+1 ) ;
	}
	else
	{
		text_value = strip( strip( subword( line, 5 ) ), 'B', '"' ) ;
	}
	text_endcol = text_col + text_value.size() ;
	if ( fType == PS )
	{
		text_name = "ZPS01" + d2ds( ++opt_field, 3 ) ;
	}
}


void text::text_display( WINDOW* win )
{
	wattrset( win, cuaAttr[ text_cua ] | text_intens ) ;
	mvwaddstr( win, text_row, text_col, text_xvalue.c_str() ) ;
}


bool text::cursor_on_text( uint row, uint col )
{
	if ( text_row != row) { return false ; } ;
	if ( col >= text_col && col < text_endcol ) { return true ; }
	return false ;
}


void dynArea::setsize( int row, int col, int width, int depth )
{
	dynArea_row   = row - 1 ;
	dynArea_col   = col - 1 ;
	dynArea_width = width   ;
	dynArea_depth = depth   ;
}


void pdc::display_pdc_avail( WINDOW* win, attType type, int pos )
{
	string t = d2ds( pos ) + ". " + pdc_xdesc ;

	wattrset( win, cuaAttr[ type ] | pdc_intens ) ;
	mvwaddstr( win, pos, 4, t.c_str() ) ;
}


void pdc::display_pdc_unavail( WINDOW* win, attType type, int pos )
{
	string t = "*. " + pdc_xdesc ;

	wattrset( win, cuaAttr[ type ] | pdc_intens ) ;
	mvwaddstr( win, pos, 4, t.c_str() ) ;
}


void abc::create_window( uint row, uint col )
{
	win = newwin( abc_maxh + 2, abc_maxw + 10, row+1, col+abc_col ) ;
	wattrset( win, cuaAttr[ AB ] | abc_intens ) ;
	box( win, 0, 0 ) ;
}


string abc::getDialogueVar( errblock& err, const string& var )
{
	string*  p_str    ;
	dataType var_type ;

	if ( selPanel )
	{
		return p_poolMGR->get( err, var, ASIS ) ;
	}

	var_type = p_funcPOOL->getType( err, var ) ;
	if ( err.error() ) { return "" ; }

	if ( err.RC0() )
	{
		if ( var_type == INTEGER )
		{
			return d2ds( p_funcPOOL->get( err, 0, var_type, var ) ) ;
		}
		else
		{
			return p_funcPOOL->get( err, 0, var ) ;
		}
	}
	else
	{
		p_str = p_poolMGR->vlocate( err, var ) ;
		if ( err.error() ) { return "" ; }
		switch ( err.getRC() )
		{
			case 0:
				 return *p_str ;
			case 4:
				 return p_poolMGR->get( err, var ) ;
			case 8:
				 p_funcPOOL->put( err, var, "" ) ;
		}
	}
	return "" ;
}


void abc::putDialogueVar( errblock& err, const string& var, const string& val )
{
	if ( selPanel )
	{
		p_poolMGR->put( err, var, val, ASIS ) ;
	}
	else
	{
		p_funcPOOL->put( err, var, val ) ;
	}
}


string abc::sub_vars( errblock& err, string s, bool& dvars )
{
	// In string, s, substitute variables starting with '&' for their dialogue value
	// The variable is delimited by any non-valid variable characters.
	// Rules: A '.' at the end of a variable, concatinates the value with the string following the dot
	//        .. reduces to .
	//        && reduces to & with no variable substitution

	// Set dvars to true if the string contains dialogue variables, else false

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


void abc::display_abc_unsel( WINDOW* win )
{
	wattrset( win, cuaAttr[ ABU ] | abc_intens ) ;
	mvwaddstr( win, 0, abc_col, abc_desc.c_str() ) ;
}


void abc::display_pd( errblock& err, uint p_row, uint p_col, uint row )
{
	// Display pull-down and highlight choice cursor is placed on if not unavailable.
	// Resize pull-down window if necessary

	int i ;

	uint w_row ;
	uint w_col ;

	size_t maxw = 0 ;

	if ( row > 1 && row < pdcList.size() + 2 )
	{
		currChoice = row - 2 ;
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
		}
		maxw = max( it->pdc_xdesc.size(), maxw ) ;
	}

	if ( !pd_created )
	{
		abc_maxw = maxw ;
		create_window( p_row, p_col ) ;
		panel = new_panel( win ) ;
		pd_created = true ;
	}
	else if ( abc_maxw != maxw )
	{
		abc_maxw = maxw ;
		WINDOW* win1 = win ;
		create_window( p_row, p_col ) ;
		replace_panel( panel, win ) ;
		delwin( win1 ) ;
	}
	else
	{
		getbegyx( win, w_row, w_col ) ;
		if ( w_row != p_row+1 || w_col != p_col+abc_col )
		{
			mvwin( win, p_row+1, p_col+abc_col ) ;
		}
	}

	i = 1 ;
	for ( auto it = pdcList.begin() ; it != pdcList.end() ; ++it, ++i )
	{
		if ( it->pdc_unavail != "" && getDialogueVar( err, it->pdc_unavail ) == "1" )
		{
			it->display_pdc_unavail( win, PUC, i ) ;
		}
		else
		{
			it->display_pdc_avail( win, (i-1) == currChoice ? AMT : PAC, i ) ;
		}
	}
	choiceVar = p_funcPOOL->get( err, 8, ".ZVARS", NOCHECK ) ;
	show_panel( panel ) ;
}


void abc::hide_pd()
{
	hide_panel( panel ) ;
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


int abc::get_pd_col()
{
	return abc_col ;
}


void abc::get_msg_position( uint& row, uint& col )
{
	row = abc_maxh + 2 ;
	col = abc_col ;
}


const string& abc::get_abc_desc()
{
	return abc_desc ;
}


bool abc::cursor_on_pulldown( uint row, uint col )
{
	uint w_row ;
	uint w_col ;

	getbegyx( win, w_row, w_col ) ;

	return ( ( row >  w_row && row < ( w_row + abc_maxh + 1  ) )  &&
		 ( col >= w_col && col < ( w_col + abc_maxw + 10 ) ) ) ;
}


void Box::box_init( errblock& err, int MAXW, int MAXD, const string& line )
{
	// Format of BOX entry in panels (FORMAT 1 VERSION 1 )
	// BOX  row col width depth Colour   B-title
	// w1   w2  w3  w4    w5    w6       w7
	// BOX  7   7   41    22    WHITE    "Test Dynamic Area 1"

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
	w4 = word( line, 4 ) ;
	w5 = word( line, 5 ) ;
	w6 = word( line, 6 ) ;

	if ( !datatype( w2, 'W' ) ||
	     !datatype( w3, 'W' ) ||
	     !datatype( w4, 'W' ) ||
	     !datatype( w5, 'W' ) )
	{
		err.seterrid( "PSYE019E" ) ;
		return ;
	}
	row   = ds2d( w2 ) ;
	col   = ds2d( w3 ) ;
	width = ds2d( w4 ) ;
	depth = ds2d( w5 ) ;

	if ( row > (MAXD - 2) )
	{
		err.seterrid( "PSYE034A" ) ;
		return ;
	}

	title = subword( line, 7 ) ;
	if ( title.size() > 2 && title.front() == '"' )
	{
		if ( title.back() != '"' )
		{
			err.seterrid( "PSYE033F" ) ;
			return ;
		}
		title.erase( 0, 1 ) ;
		title.pop_back()    ;
	}

	if ( width > (MAXW - col+1) ) { width = (MAXW - col+1) ; } ;
	if ( depth > (MAXD - row+1) ) { depth = (MAXD - row+1) ; } ;

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


void Box::display_box( WINDOW* win, string title )
{
	int offset ;

	if ( title.size() > box_width-4 )
	{
		title.erase( box_width-4 ) ;
	}

	wattrset( win, box_colour | box_intens ) ;

	mvwaddch( win, box_row, box_col, ACS_ULCORNER ) ;
	mvwaddch( win, box_row, box_col + box_width - 1, ACS_URCORNER ) ;

	mvwaddch( win, box_row + box_depth - 1, box_col, ACS_LLCORNER ) ;
	mvwaddch( win, box_row + box_depth - 1, box_col + box_width - 1, ACS_LRCORNER ) ;

	mvwhline( win, box_row, (box_col + 1), ACS_HLINE, (box_width - 2) ) ;
	mvwhline( win, (box_row + box_depth - 1), (box_col + 1), ACS_HLINE, (box_width - 2) ) ;

	mvwvline( win, (box_row + 1), box_col, ACS_VLINE, (box_depth - 2) ) ;
	mvwvline( win, (box_row + 1), (box_col + box_width - 1 ), ACS_VLINE, (box_depth - 2) ) ;

	offset = ( box_width - title.size() ) / 2 ;
	mvwaddstr( win, box_row, (box_col + offset), title.c_str() ) ;

}
