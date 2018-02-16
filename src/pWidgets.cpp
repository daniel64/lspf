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

	cuaType fType  ;

	err.setRC( 0 ) ;

	ws = words( line ) ;
	if ( ws < 7 )
	{
		err.seterrid( "PSYE035P" ) ;
		return ;
	}

	w2   = word( line, 2 ) ;
	w3   = word( line, 3 ) ;
	w4   = word( line, 4 ) ;
	w5   = word( line, 5 ) ;

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
	else
	{
		if ( cuaAttrName.count( w5 ) == 0 )
		{
			err.seterrid( "PSYE032F", w5 ) ;
			return ;
		}
		fType = cuaAttrName[ w5 ] ;
	}

	field_cua    = fType ;
	field_row    = row-1 ;
	field_col    = col-1 ;
	field_length = len   ;
	field_cole   = field_col + field_length ;
	field_input  = ( cuaAttrUnprot.count( fType ) > 0 ) ;

	field_opts( err, opts ) ;
	if ( err.error() ) { return ; }

	return ;
}


void field::field_opts( errblock& err, string& opts )
{
	// CAPS(ON,OFF)
	// JUST(LEFT,RIGHT,ASIS)
	// NUMERIC(ON,OFF)
	// PAD(char,NULLS,USER)
	// SKIP(ON,OFF)

	int p1 ;
	int p2 ;

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

	if ( trim( opts ) != "" )
	{
		err.seterrid( "PSYE032H", opts ) ;
		return ;
	}
}


void dynArea::dynArea_init( errblock& err, int MAXW, int MAXD, const string& line )
{
	// Format of DYNAREA entry in panels (FORMAT 1 VERSION 1 )
	// DYNAREA row col width depth   A-name S-name DATAIN() DATAOUT() USERMOD() DATAMOD()

	// w1      w2         w3   w4    w5     w6     w7      <-----------------keywords------------------>
	// DYNAREA MAX-10 MAX-20   MAX   MAX-6  ZAREA  ZSHADOW DATAIN(01) DATAOUT(02) USERMOD(03) DATAMOD(04)

	// if width=MAX  width=MAXW-col+1
	// if depth=MAX  depth=MAXD-row+1

	// UserMod - field touched
	// DataMod - field changed

	// UserMod but not DataMod - Attr changed to UserMod even if the field was changed
	// DataMod but not UserMod - Attr changed to DataMod only if the field value has changed
	// UserMod and DataMod     - Attr changed to UserMod if no change to field value, else DataMod
	// Neither                 - Attr unchanged

	int row   ;
	int col   ;
	int width ;
	int depth ;
	int p1    ;
	int p2    ;

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

	p1 = rest.find( " DATAIN(" ) ;
	if ( p1 != string::npos )
	{
		p2 = rest.find( ')', p1 ) ;
		if ( p2 == string::npos )
		{
			err.seterrid( "PSYE032D" ) ;
			return ;
		}
		t = rest.substr( p1+8, p2-p1-8 ) ;
		trim( t ) ;
		if ( t.size() == 0 )
		{
			err.seterrid( "PSYE031G" ) ;
			return ;
		}
		dynArea_DataInsp = true ;
		if      ( t.size() == 1 ) { dynArea_DataIn = t[0]          ; }
		else if ( t.size() == 2 ) { dynArea_DataIn = xs2cs( t )[0] ; }
		else                      { err.seterrid( "PSYE032B" )     ; return  ; }
		rest = rest.erase( p1, p2-p1+1 ) ;
	}
	p1 = rest.find( " DATAOUT(" ) ;
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
		dynArea_DataOutsp = true ;
		if      ( t.size() == 1 ) { dynArea_DataOut = t[0]          ; }
		else if ( t.size() == 2 ) { dynArea_DataOut = xs2cs( t )[0] ; }
		else                      { err.seterrid( "PSYE032B" )      ; return ; }
		rest = rest.erase( p1, p2-p1+1 ) ;
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
		dynArea_UserModsp = true ;
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
		dynArea_DataModsp = true ;
		if      ( t.size() == 1 ) { dynArea_DataMod = t[0]          ; }
		else if ( t.size() == 2 ) { dynArea_DataMod = xs2cs( t )[0] ; }
		else                      { err.seterrid( "PSYE032B" )      ; return ; }
		rest = rest.erase( p1, p2-p1+1 ) ;
	}

	if ( (dynArea_UserModsp || dynArea_DataModsp) && !dynArea_DataInsp )
	{
		err.seterrid( "PSYE032C" ) ;
		return ;
	}

	if ( trim( rest ) != "" )
	{
		err.seterrid( "PSYE032H", rest ) ;
		return ;
	}

	dynArea_row         = row-1 ;
	dynArea_col         = col-1 ;
	dynArea_width       = width ;
	dynArea_depth       = depth ;
	dynArea_shadow_name = w7    ;

	if ( dynArea_DataInsp  ) { dynArea_FieldIn.push_back( dynArea_DataIn  ) ; }
	if ( dynArea_UserModsp ) { dynArea_FieldIn.push_back( dynArea_UserMod ) ; }
	if ( dynArea_DataModsp ) { dynArea_FieldIn.push_back( dynArea_DataMod ) ; }

	dynArea_Field = dynArea_FieldIn ;
	if ( dynArea_DataOutsp ) { dynArea_Field.push_back( dynArea_DataOut   ) ; }

	return ;
}


bool field::edit_field_insert( WINDOW * win, char ch, int col, char pad, bool snulls )
{
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_DataInsp is true and
	// there is an input attribute byte at the start of the field

	// Use the null character (x'00') to fill fields when the cursor is past the end if the field value
	// These are then removed before further processing by the application

	uint pos ;

	int p1  ;
	int p2  ;

	dynArea * da ;
	const char nulls(0x00) ;

	pos = (col - field_col ) ;
	if ( pos >= field_value.size() )
	{
		field_value.resize( pos+1, nulls ) ;
	}

	if ( field_dynArea )
	{
		da = field_dynArea ;
		p2 = field_value.find_first_of( da->dynArea_Field, pos ) ;
		if ( p2 == string::npos ) { p2 = field_value.size()-1 ; }
		else                      { p2--                      ; }
		if ( field_value[ p2 ] != ' '  &&
		     field_value[ p2 ] != nulls ) { return false ; }
		field_value.erase( p2, 1 ) ;
		field_shadow_value.erase( p2, 1 ) ;
		p1 = field_value.find_last_of( da->dynArea_FieldIn, pos ) ;
		if ( da->dynArea_DataModsp )
		{
			if ( field_value[ p1 ] != da->dynArea_DataMod )
			{
				if ( ch != ' ' )
				{
					field_value[ p1 ] = da->dynArea_DataMod ;
				}
				else if ( field_value.substr( pos, p2-pos ) != string( p2-pos, ' ' ) )
				{
					field_value[ p1 ] = da->dynArea_DataMod ;
				}
				else if ( da->dynArea_UserModsp )
				{
					field_value[ p1 ] = da->dynArea_UserMod ;
				}
			}
		}
		else if ( da->dynArea_UserModsp )
		{
			field_value[ p1 ] = da->dynArea_UserMod ;
		}
		if ( field_value[ pos ] == nulls )
		{
			field_value[ pos ]        = ch       ;
			field_shadow_value[ pos ] = B_YELLOW ;
		}
		else
		{
			field_value.insert( pos, 1, ch ) ;
			field_shadow_value.insert( pos, 1, B_YELLOW ) ;
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

	display_field( win, pad, snulls ) ;
	field_changed = true ;
	return true ;
}


bool field::edit_field_replace( WINDOW * win, char ch, int col, char pad, bool snulls )
{
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_DataInsp is true and
	// there is an input attribute byte at the start of the field

	// Use the null character (x'00') to fill fields when the cursor is past the end if the field value
	// These are then removed before further processing by the application

	int p1   ;

	uint pos ;

	dynArea * da ;

	const char nulls(0x00) ;

	pos = (col - field_col ) ;
	if ( pos >= field_value.size() )
	{
		field_value.resize( pos+1, nulls ) ;
	}

	if ( field_dynArea )
	{
		if ( field_value[ pos ] != nulls && !isprint( field_value[ pos ] ) ) { return false ; }
		da = field_dynArea ;
		field_shadow_value[ pos ] = B_YELLOW ;
		p1 = field_value.find_last_of( da->dynArea_FieldIn, pos ) ;
		if ( da->dynArea_DataModsp )
		{
			if ( field_value[ p1 ] != da->dynArea_DataMod )
			{
				if ( field_value[ pos ] != ch )
				{
					field_value[ p1 ] = da->dynArea_DataMod ;
				}
				else if ( da->dynArea_UserModsp )
				{
					field_value[ p1 ] = da->dynArea_UserMod ;
				}
			}
		}
		else if ( da->dynArea_UserModsp )
		{
			field_value[ p1 ] = da->dynArea_UserMod ;
		}

	}
	field_value[ pos ] = ch ;

	display_field( win, pad, snulls ) ;
	field_changed = true ;
	return true ;
}


void field::edit_field_delete( WINDOW * win, int col, char pad, bool snulls )
{
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_DataInsp is true
	// and there is an input attribute byte at the start of the field.

	uint pos ;

	int p1 ;
	int p2 ;

	const char nulls(0x00) ;

	dynArea * da ;

	pos = col - field_col ;
	if ( pos > field_value.size() ) { return ; }
	if ( field_value[ pos ] != nulls && !isprint( field_value[ pos ] ) ) { return ; }

	if ( field_dynArea )
	{
		da = field_dynArea ;
		p1 = field_value.find_last_of( da->dynArea_FieldIn, pos ) ;
		p2 = field_value.find_first_of( da->dynArea_Field, pos )  ;
		if ( p2 == string::npos ) { p2 = field_value.size() ; }
		if ( da->dynArea_DataModsp )
		{
			if ( field_value[ p1 ] != da->dynArea_DataMod )
			{
				if ( field_value.substr( pos, p2-pos ) != string( p2-pos, ' ' ) )
				{
					field_value[ p1 ] = da->dynArea_DataMod ;
				}
				else if ( da->dynArea_UserModsp )
				{
					field_value[ p1 ] = da->dynArea_UserMod ;
				}
			}
		}
		else if ( da->dynArea_UserModsp )
		{
			field_value[ p1 ] = da->dynArea_UserMod ;
		}
		field_value.insert( p2, 1, ' ' ) ;
		field_shadow_value.insert( p2, 1, 0xFE ) ;
		field_shadow_value.erase( pos, 1 ) ;
	}

	field_value.erase( pos, 1 ) ;
	display_field( win, pad, snulls ) ;
	field_changed = true ;
}


int field::edit_field_backspace( WINDOW * win, int col, char pad, bool snulls )
{
	// If this is a dynamic area, we know it is an input field so pos > 0 (to allow for the input attribute byte)

	int pos ;

	pos = col - field_col ;
	if ( pos > field_value.size() ) { return --col ; }

	if ( field_dynArea )
	{
		if ( field_dynArea->dynArea_Field.find_first_of( field_value[ pos-1 ] ) != string::npos )
		{
			return col ;
		}
	}

	col-- ;
	edit_field_delete( win, col, pad, snulls ) ;
	return col ;
}


void field::field_erase_eof( WINDOW * win, uint col, char pad, bool snulls )
{
	// If this is a dynamic area, we know at this point this is an input field, so dynArea_DataInsp is true,
	// and there is an input attribute byte at the start of the field

	int pos ;
	int p1  ;
	int p2  ;

	dynArea * da ;

	if ( ( field_col + field_value.size() ) < col ) return ;
	pos = (col - field_col) ;

	if ( field_dynArea )
	{
		da = field_dynArea ;
		p1 = field_value.find_last_of( da->dynArea_FieldIn, pos ) ;
		p2 = field_value.find_first_of( da->dynArea_Field, pos ) ;
		if ( p2 == string::npos ) { p2 = field_value.size() ; }
		if ( da->dynArea_DataModsp )
		{
			if ( field_value[ p1 ] != da->dynArea_DataMod )
			{
				if ( field_value.substr( pos, p2-pos ) != string( p2-pos, ' ' ) )
				{
					field_value[ p1 ] = da->dynArea_DataMod ;
				}
				else if ( da->dynArea_UserModsp )
				{
					field_value[ p1 ] = da->dynArea_UserMod ;
				}
			}
		}
		else if ( da->dynArea_UserModsp )
		{
			field_value[ p1 ] = da->dynArea_UserMod ;
		}
		field_value.replace( pos, p2-pos, p2-pos, ' ' )  ;
		field_shadow_value.replace( pos, p2-pos, p2-pos, 0xFE ) ;
	}
	else
	{
		field_blank( win, pad ) ;
		field_value.erase( pos+1 ) ;
	}

	display_field( win, pad, snulls ) ;
	field_changed = true ;
}


void field::field_blank( WINDOW * win, char pad )
{
	char upad ;
	char * blanks = new char[ field_length+1 ] ;

	upad = field_paduser ? pad : field_padchar ;
	for ( int i = 0 ; i < field_length ; i++ )
	{
		blanks[ i ] = upad ;
	}

	blanks[ field_length ] = 0x00 ;
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

	// If a field that has only been touched, changes, change DataIn to DataMod (if specified)

	// Keep shadow variable in sync

	// BUG: Nulls not removed from a touched/changed field with no USERMOD/DATAMOD parameter specified
	// (as we cannot determine if the field has been touched/changed without these)

	int p1 ;
	int p2 ;
	int p3 ;
	int pattr ;

	bool changed ;

	const char nulls(0x00) ;

	dynArea * da ;

	p1 = 0 ;
	da = field_dynArea ;
	if ( !da->dynArea_DataInsp ) { return ; }

	while ( true )
	{
		p1 = field_value.find_first_of( da->dynArea_FieldIn, p1 ) ;
		if ( p1 == string::npos ) { return ; }
		pattr = p1 ;
		p1++ ;
		p3 = field_value.find_first_of( da->dynArea_Field, p1 ) ;
		if ( p3 == string::npos ) { p3 = field_value.size() - 1 ; }
		else                      { p3--                        ; }
		p2 = field_value.find_last_not_of( nulls, p3 ) ;
		if ( p2 == string::npos )
		{
			field_value.replace( p1, p3-p1+1, p3-p1+1, ' ' ) ;
			field_shadow_value.replace( p1, p3-p1+1, p3-p1+1, 0xFF ) ;
			if ( da->dynArea_DataModsp )
			{
				field_value[ pattr ] = da->dynArea_DataMod ;
			}
			break ;
		}
		changed = false ;
		if ( p3 > p2 )
		{
			field_value.replace( p2+1, p3-p2, p3-p2, ' ' ) ;
			field_shadow_value.replace( p2+1, p3-p2, p3-p2, 0xFF ) ;
			changed = true ;
		}
		while ( true )
		{
			if ( p1 >= p2 ) { break ; }
			if ( field_value[ p1 ] == nulls )
			{
				changed = true ;
				field_value.erase( p1, 1 ) ;
				field_value.insert( p3, 1, ' ' ) ;
				field_shadow_value.erase( p1, 1 ) ;
				field_shadow_value.insert( p3, 1, 0xFF ) ;
				p2-- ;
				continue ;
			}
			p1++ ;
		}
		if ( changed && da->dynArea_DataModsp )
		{
			field_value[ pattr ] = da->dynArea_DataMod ;
		}
	}
}


void field::field_clear( WINDOW * win, char pad )
{
	field_value = ""     ;
	field_blank( win, pad ) ;
	field_changed = true ;
}


int field::end_of_field( WINDOW * win, uint col )
{
	// If this is a dynamic area, we know at this point this is an input field.
	// Strip trailing nulls if not a dynamic area

	int pos ;
	int p1  ;
	int p2  ;

	string padc = "" ;

	padc.push_back( 0x00 ) ;
	padc.push_back( 0x20 ) ;

	const char nulls(0x00) ;

	if ( field_dynArea )
	{
		pos = (col - field_col) ;
		p2  = field_value.find_first_of( field_dynArea->dynArea_Field, pos ) ;
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

	uint pos ;
	int p1   ;

	dynArea * da = field_dynArea ;

	if ( !da->dynArea_DataInsp ) { return false ; }

	pos = col - field_col ;

	if ( da->dynArea_Field.find_first_of( field_value[ pos ] ) != string::npos ) { return false ; }

	p1 = field_value.find_last_of( da->dynArea_Field, pos ) ;
	if ( p1 == string::npos ) { return false ; }

	p1 = da->dynArea_FieldIn.find_first_of( field_value[ p1 ] ) ;
	if ( p1 == string::npos ) { return false ; }

	return true ;
}


int field::field_dyna_input_offset( uint col )
{
	// When this routine is called, we know the field is a dynamic area

	uint pos ;
	int  p1  ;

	pos = ( col < field_col ) ? 0 : (col - field_col) ;

	p1 = field_value.find_first_of( field_dynArea->dynArea_FieldIn, pos ) ;
	if ( p1 == string::npos ) { return -1 ; }
	return p1+1 ;
}


void field::field_attr( errblock& err, string attrs )
{
	// Format:

	// TYPE(CUA) (change to/from PS not allowed)

	// COLOUR(RED/GREEN/YELLOW/BLUE/MAGENTA/TURQ/WHITE)
	// INTENSE(LOW/HIGH)
	// HILITE(NONE/BLINK/REVERSE/USCORE)

	// For NON-CUA field_colour, default to the current CUA value in case only one attribute is changed
	// Hex format for field_colour:
	// 00 X0 00 00   - X is the INTENSITY
	// 00 0X 00 00   - X is the HILITE
	// 00 00 XX 00   - X is the COLOUR

	int p1 ;
	int p2 ;

	string cua    ;
	string col    ;
	string hilite ;
	string intens ;

	err.setRC( 0 ) ;

	cua    = "" ;
	col    = "" ;
	intens = "" ;
	hilite = "" ;

	p1 = pos( "TYPE(", attrs ) ;
	if ( p1 > 0 )
	{
		p2    = pos( ")", attrs, p1 ) ;
		if ( p2 == 0 ) { err.seterrid( "PSYE032D" ) ; return ; }
		cua   = strip( substr( attrs, (p1 + 5), (p2 - (p1 + 5)) ) ) ;
		attrs = delstr( attrs, p1, (p2 - p1 + 1) ) ;
		if      ( cua == "TEXT"    ) { field_input  = false ; }
		else if ( cua == "OUTPUT"  ) { field_input  = false ; }
		else if ( cua == "INPUT"   ) { field_input  = true  ; }
		else
		{
			if ( trim( attrs ) != "" ) { err.seterrid( "PSYE032H", attrs ) ; return ; }
			if ( cuaAttrName.count( cua ) == 0 )  { err.seterrid( "PSYE032F", cua ) ; return ; }
			if ( cua == "PS" || field_cua == PS ) { err.seterrid( "PSYE035A" ) ; return ; }
			field_cua    = cuaAttrName[ cua ] ;
			field_input  = ( cuaAttrUnprot.count( field_cua ) > 0 ) ;
			field_usecua = true ;
			return ;
		}
	}
	if ( field_usecua )
	{
		field_colour = cuaAttr[ field_cua ] ;
	}
	p1 = pos( "COLOUR(", attrs ) ;
	if ( p1 > 0 )
	{
		p2    = pos( ")", attrs, p1 ) ;
		if ( p2 == 0 ) { err.seterrid( "PSYE032D" ) ; return ; }
		col   = strip( substr( attrs, (p1 + 7), (p2 - (p1 + 7)) ) ) ;
		attrs = delstr( attrs, p1, (p2 - p1 + 1) ) ;
		field_colour = field_colour & 0XFFFF00FF ;
		if      ( col == "RED"     ) { field_colour = field_colour | RED     ; }
		else if ( col == "GREEN"   ) { field_colour = field_colour | GREEN   ; }
		else if ( col == "YELLOW"  ) { field_colour = field_colour | YELLOW  ; }
		else if ( col == "BLUE"    ) { field_colour = field_colour | BLUE    ; }
		else if ( col == "MAGENTA" ) { field_colour = field_colour | MAGENTA ; }
		else if ( col == "TURQ"    ) { field_colour = field_colour | TURQ    ; }
		else if ( col == "WHITE"   ) { field_colour = field_colour | WHITE   ; }
		else                         { err.seterrid( "PSYE035B", col, "COLOUR" ) ; return ; }
	}
	p1 = pos( "INTENSE(", attrs ) ;
	if ( p1 > 0 )
	{
		field_colour = field_colour & 0XFF0FFFFF ;
		p2     = pos( ")", attrs, p1 ) ;
		if ( p2 == 0 ) { err.seterrid( "PSYE032D" ) ; return ; }
		intens = strip( substr( attrs, (p1 + 8), (p2 - (p1 + 8)) ) ) ;
		attrs  = delstr( attrs, p1, (p2 - p1 + 1) ) ;
		if      ( intens == "HIGH" ) { field_colour = field_colour | A_BOLD   ; }
		else if ( intens == "LOW"  ) { field_colour = field_colour | A_NORMAL ; }
		else                         { err.seterrid( "PSYE035B", intens, "INTENSE" ) ; return ; }
	}
	p1 = pos( "HILITE(", attrs ) ;
	if ( p1 > 0 )
	{
		field_colour = field_colour & 0XFFF0FFFF ;
		p2     = pos( ")", attrs, p1 ) ;
		if ( p2 == 0 ) { err.seterrid( "PSYE032D" ) ; return ; }
		hilite = strip( substr( attrs, (p1 + 7), (p2 - (p1 + 7)) ) ) ;
		attrs  = delstr( attrs, p1, (p2 - p1 + 1) ) ;
		if      ( hilite == "NONE"    ) {}
		else if ( hilite == "BLINK"   ) { field_colour = field_colour | A_BLINK     ; }
		else if ( hilite == "REVERSE" ) { field_colour = field_colour | A_REVERSE   ; }
		else if ( hilite == "USCORE"  ) { field_colour = field_colour | A_UNDERLINE ; }
		else                            { err.seterrid( "PSYE035B", hilite, "HILITE" ) ; return ; }
	}
	if ( trim( attrs ) != "" )
	{
		err.seterrid( "PSYE032H", attrs ) ;
		return ;
	}
	field_usecua = false ;
	return ;
}


void field::field_attr()
{
	// Reset field attribute to use the CUA value

	field_usecua = true ;
}


void field::field_prep_input()
{
	// Prepare the field to be copied to the function pool (non-dynamic area fields only)

	// Remove nulls from the field.
	// Apply just(right|left|asis) to non-dynamic area input/output fields
	//    JUST(LEFT/RIGHT) leading and trailing spaces are removed
	//    JUST(ASIS) Only trailing spaces are removed

	int p1 ;
	int p2 ;

	const char nulls(0x00) ;

	p1 = 0 ;
	while ( true )
	{
		p1 = field_value.find_first_of( nulls, p1 ) ;
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


void field::field_DataMod_to_UserMod( string * darea, int offset )
{
	// Reset any dynArea_DataMod attributes to dynArea_UserMod or field_dynDataIn, for dynamic fields that
	// have not changed. This is for fields that have been changed, then changed back to the original.
	// (This has to show as touched/input attr, not changed).

	// When this is called, we know dynDataIn and dynDataMod have been specified (but not necessarily dynUserMod,
	// or dynDataOut).  If no change to the field, attr=dynUserMod if specified, else dynDataIn

	int p1 ;
	int p2 ;

	dynArea * da = field_dynArea ;

	p1 = 0 ;
	while ( true )
	{
		p1 = field_value.find_first_of( da->dynArea_DataMod, p1 ) ;
		if ( p1 == string::npos ) { break ; }
		p1++ ;
		p2 = field_value.find_first_of( da->dynArea_Field, p1 ) ;
		if ( p2 == string::npos ) { p2 = field_value.size() ; }
		if ( field_value.compare( p1, p2-p1, (*darea), offset+p1, p2-p1 ) == 0 )
		{
			if ( da->dynArea_UserModsp ) { field_value[ p1-1 ] = da->dynArea_UserMod ; }
			else                         { field_value[ p1-1 ] = da->dynArea_DataIn  ; }
		}
		p1 = p2 ;
	}
}


void field::display_field( WINDOW * win, char pad, bool snulls )
{
	// For non-dynamic area fields: if an input field, truncate if value size > field size else for output fields
	// display field size bytes and leave field value unchanged (necessary for table display fields)

	// Display the null character as the field pad character and pad the field with the same character

	// Call ncurses touchline() for the field row as the update does not always appear without it

	uint i     ;
	char nullc ;
	char attr  ;
	string t   ;

	const char nulls(0x00) ;
	char upad  ;

	string::iterator it1 ;
	string::iterator it2 ;

	dynArea * da ;

	if ( !field_active ) { return ; }

	nullc = snulls ? '.' : ' ' ;

	upad  = field_paduser ? pad : field_padchar ;

	if ( field_dynArea )
	{
		da = field_dynArea ;
		if ( da->dynArea_DataInsp || da->dynArea_DataOutsp )
		{
			it2  = field_shadow_value.begin() ;
			i    = 0 ;
			attr = (*it2) ;
			wattrset( win, usrAttr[ attr ] ) ;
			for ( it1 = field_value.begin() ; it1 != field_value.end() ; it1++, it2++, i++ )
			{
				if ( attr != (*it2) ) { wattrset( win, usrAttr[ (*it2) ] ) ; attr = (*it2) ; }
				if (  (*it1) == nulls )
				{
					mvwaddch( win, field_row, field_col+i, nullc ) ;
				}
				else if (  da->dynArea_Field.find_first_of( (*it1) ) != string::npos )
				{
					mvwaddch( win, field_row, field_col+i, ' ' ) ;
				}
				else if ( isprint( (*it1) ) )
				{
					mvwaddch( win, field_row, field_col+i, (*it1) ) ;
				}
				else
				{
					mvwaddch( win, field_row, field_col+i, '.' ) ;
				}
			}
		}
		else
		{
			it2  = field_shadow_value.begin() ;
			i    = 0 ;
			attr = (*it2) ;
			wattrset( win, usrAttr[ attr ] ) ;
			for ( it1 = field_value.begin() ; it1 != field_value.end() ; it1++, it2++, i++ )
			{
				if ( attr != (*it2) ) { wattrset( win, usrAttr[ (*it2) ] ) ; attr = (*it2) ; }
				if (  (*it1) == nulls )
				{
					mvwaddch( win, field_row, field_col+i, nullc ) ;
				}
				else if ( isprint( (*it1) ) )
				{
					mvwaddch( win, field_row, field_col+i, (*it1) ) ;
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
		wattrset( win, field_usecua ? cuaAttr[ field_cua ] : field_colour ) ;
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
		for ( it1 = t.begin() ; it1 != t.end() ; it1++ )
		{
			if      ( (*it1) == nulls   ) { (*it1) = upad ; }
			else if ( !isprint( (*it1)) ) { (*it1) = '.'  ; }
		}
		if ( field_pwd )
		{
			t = string( field_value.size(), '*' ) ;
		}
		t.resize( field_length, upad ) ;
		mvwaddstr( win, field_row, field_col, t.c_str() ) ;
	}
	touchline( win, field_row, 1 ) ;
}


bool field::cursor_on_field( uint row, uint col )
{
	if ( field_row != row) { return false ; } ;
	if ( col >= field_col && col < field_cole ) { return true ; }
	return false ;
}


void literal::literal_init( errblock& err, int MAXW, int MAXD, int& opt_field, const string& line )
{
	// Format of literal entry in panels (FORMAT 1 VERSION 1 )
	// LITERAL row col cuaAttr (EXPAND) value
	// w1      w2  w3  w4      w5
	// LITERAL 3   1   OUTPUT  "COMMAND ===> "
	// OR
	// w1      w2  w3  w4      w5     w6
	// LITERAL 3   1   OUTPUT  EXPAND "-"
	// Use EXPAND to repeat the value to end of the screen (MAXW)

	int row   ;
	int col   ;
	int l     ;

	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;

	cuaType fType ;

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

	literal_cua = fType   ;
	literal_row = row - 1 ;
	literal_col = col - 1 ;
	if ( upper( w5 ) == "EXPAND" )
	{
		literal_value = strip( strip( subword( line, 6 ) ), 'B', '"' ) ;
		if ( literal_value.size() == 0 )
		{
			err.seterrid( "PSYE041K" ) ;
			return ;
		}
		l = (MAXW - col) / literal_value.size() + 1 ;
		literal_value = substr( copies( literal_value, l ), 1, MAXW-col+1 ) ;
	}
	else
	{
		literal_value = strip( strip( subword( line, 5 ) ), 'B', '"' ) ;
	}
	literal_cole = literal_col + literal_value.size() ;
	if ( fType == PS )
	{
		literal_name = "ZPS01" + d2ds( ++opt_field, 3 ) ;
	}
	return ;
}


void literal::literal_display( WINDOW * win )
{
	wattrset( win, cuaAttr[ literal_cua ] ) ;
	mvwaddstr( win, literal_row, literal_col, literal_xvalue.c_str() ) ;
	wattroff( win, cuaAttr[ literal_cua ] ) ;
}


bool literal::cursor_on_literal( uint row, uint col )
{
	if ( literal_row != row) { return false ; } ;
	if ( col >= literal_col && col < literal_cole ) { return true ; }
	return false ;
}


void dynArea::setsize( int row, int col, int width, int depth )
{
	dynArea_row   = row - 1 ;
	dynArea_col   = col - 1 ;
	dynArea_width = width   ;
	dynArea_depth = depth   ;
}


void pdc::display_pdc_avail( WINDOW* win, cuaType type, int pos )
{
	string t = d2ds( pos ) + ". " + pdc_xdesc ;

	wattrset( win, cuaAttr[ type ] ) ;
	mvwaddstr( win, pos, 4, t.c_str() ) ;
	wattroff( win, cuaAttr[ type ] ) ;
}


void pdc::display_pdc_unavail( WINDOW* win, cuaType type, int pos )
{
	string t = "*. " + pdc_xdesc ;

	wattrset( win, cuaAttr[ type ] ) ;
	mvwaddstr( win, pos, 4, t.c_str() ) ;
	wattroff( win, cuaAttr[ type ] ) ;
}


void abc::create_window( uint row, uint col )
{
	win = newwin( abc_maxh + 2, abc_maxw + 10, row+1, col+abc_col ) ;
	wattrset( win, cuaAttr[ AB ] ) ;
	box( win, 0, 0 ) ;
	wattroff( win, cuaAttr[ AB ] ) ;
}


string abc::getDialogueVar( errblock& err, const string& var )
{
	string * p_str    ;
	dataType var_type ;

	if ( selPanel )
	{
		return p_poolMGR->get( err, var, ASIS ) ;
	}

	var_type = p_funcPOOL->getType( err, var, NOCHECK ) ;
	if ( err.error() ) { return "" ; }

	if ( err.RC0() )
	{
		if ( var_type == INTEGER )
		{
			return d2ds( p_funcPOOL->get( err, 0, var_type, var ) ) ;
		}
		else
		{
			return p_funcPOOL->get( err, 0, var, NOCHECK ) ;
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

	int p1 ;
	int p2 ;

	string var ;
	string val ;

	const string validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#$@" ;
	p1 = 0 ;

	dvars = false ;
	while ( true )
	{
		p1 = s.find( '&', p1 ) ;
		if ( p1 == string::npos || p1 == s.size() - 1 ) { break ; }
		p1++ ;
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


void abc::display_abc_sel( WINDOW * win )
{
	wattrset( win, cuaAttr[ AB ] ) ;
	mvwaddstr( win, 0, abc_col, abc_desc.c_str() ) ;
	wattroff( win, cuaAttr[ AB ] ) ;
}


void abc::display_abc_unsel( WINDOW * win )
{
	wattrset( win, cuaAttr[ ABU ] ) ;
	mvwaddstr( win, 0, abc_col, abc_desc.c_str() ) ;
	wattroff( win, cuaAttr[ ABU ] ) ;
}


void abc::display_pd( errblock& err, uint p_row, uint p_col, uint row )
{
	// Display pull-down and highlight choice cursor is placed on if not unavailable.
	// Resize pull-down window if necessary

	int i ;
	int w_row ;
	int w_col ;
	int maxw = 0 ;

	if ( row > 1 && row < pdcList.size() + 2 )
	{
		currChoice = row - 2 ;
	}


	for ( auto it = pdcList.begin() ; it != pdcList.end() ; it++ )
	{
		if ( it->pdc_dvars )
		{
			it->pdc_xdesc = sub_vars( err, it->pdc_desc, it->pdc_dvars ) ;
			if ( !it->pdc_dvars )
			{
				it->pdc_desc = "" ;
			}
		}
		maxw = max( int( it->pdc_xdesc.size() ), maxw ) ;
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
		WINDOW * win1 = win ;
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
	for ( auto it = pdcList.begin() ; it != pdcList.end() ; it++, i++ )
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


void abc::get_msg_position( int& row, int& col )
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
	int w_row ;
	int w_col ;

	getbegyx( win, w_row, w_col ) ;

	return ( ( row >  w_row && row < ( w_row + abc_maxh + 1  ) )  &&
		 ( col >= w_col && col < ( w_col + abc_maxw + 10 ) ) ) ;
}


void Box::box_init( errblock& err, int MAXW, int MAXD, const string& line )
{
	// Format of BOX entry in panels (FORMAT 1 VERSION 1 )
	// BOX  row col width depth cuaAttr  B-title
	// w1   w2  w3  w4    w5    w6       w7
	// BOX  7   7   41    22    N_WHITE  "Test Dynamic Area 1"

	int row    ;
	int col    ;
	int width  ;
	int depth  ;
	int colour ;

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

	colour = ( usrAttrNames.count( w6 ) > 0 ) ? usrAttrNames[ w6 ] : B_GREEN ;

	box_row    = row - 1 ;
	box_col    = col - 1 ;
	box_width  = width   ;
	box_depth  = depth   ;
	box_colour = usrAttr[ colour ] ;
	if ( title != "" )
	{
		box_title = " " + title + " "  ;
	}
}


void Box::display_box( WINDOW * win, string title )
{
	int offset ;

	if ( title.size() > box_width-4 )
	{
		title.erase( box_width-4 ) ;
	}

	wattrset( win, box_colour ) ;

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

	wattroff( win, box_colour ) ;
}
