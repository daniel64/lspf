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

void field::field_init( errblock& err,
			int MAXW,
			int MAXD,
			const string& line,
			uint Area_num,
			uint Area_col )
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

	if ( Area_num > 0 && ( w2 == "MAX" || w2.compare( 0, 4, "MAX-" ) == 0 ) )
	{
		err.seterrid( "PSYE031U" ) ;
		return ;
	}

	if ( isnumeric( w2 ) )                      { row = ds2d( w2 ) ; }
	else if ( w2 == "MAX" )                     { row = MAXD       ; }
	else if ( w2.compare( 0, 4, "MAX-" ) == 0 ) { row = MAXD - ds2d( substr( w2, 5 ) )    ; }
	else                                        { err.seterrid( "PSYE031S", w2 ) ; return ; }

	if ( isnumeric( w3 ) )                      { col = ds2d( w3 ) ; }
	else if ( w3 == "MAX" )                     { col = MAXW       ; }
	else if ( w3.compare( 0, 4, "MAX-" ) == 0 ) { col = MAXW - ds2d( substr( w3, 5 ) )    ; }
	else                                        { err.seterrid( "PSYE031S", w3 ) ; return ; }

	if ( isnumeric( w4 ) )                      { len = ds2d( w4 )     ; }
	else if ( w4 == "MAX" )                     { len = MAXW - col + 1 ; }
	else if ( w4.compare( 0, 4, "MAX-" ) == 0 ) { len = MAXW - col - ds2d( substr( w4, 5 ) ) + 1 ; }
	else                                        { err.seterrid( "PSYE031S", w4 ) ; return        ; }

	if ( Area_num == 0 && row > MAXD )
	{
		err.seterrid( "PSYE031A", d2ds( row ), d2ds( MAXD ) ) ;
		return ;
	}

	if ( col > MAXW )
	{
		err.seterrid( "PSYE031B", d2ds( col ), d2ds( MAXW ) ) ;
		return ;
	}

	if ( w5 == "PWD" )
	{
		fType     = NEF  ;
		field_pwd = true ;
		field_cua = NEF  ;
		field_colour1 = cuaAttr[ NEF ] ;
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
	field_length = len ;

	if ( Area_num > 0 )
	{
		field_area_row  = field_row ;
		field_area_col  = field_col ;
		field_col      += Area_col  ;
		field_visible   = false ;
	}

	field_endcol = field_col + field_length - 1 ;
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
			err.seterrid( "PSYE031V" ) ;
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
			err.seterrid( "PSYE031V" ) ;
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
			err.seterrid( "PSYE031V" ) ;
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
			err.seterrid( "PSYE031V" ) ;
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
			err.seterrid( "PSYE031V" ) ;
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
			err.seterrid( "PSYE031V" ) ;
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


void Area::Area_init( errblock& err,
		      int MAXW,
		      int MAXD,
		      uint num,
		      const string& line )
{
	// Format of AREA entry in panels (FORMAT 1 VERSION 1 )
	// AREA row col width depth

	// w1      w2         w3   w4    w5     w6
	// AREA    MAX-10 MAX-20   MAX   MAX-6  ZAREA

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
		err.seterrid( "PSYE032H", rest ) ;
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
	else                                        { err.seterrid( "PSYE031S", w2 ) ; return ; }

	if ( isnumeric( w3 ) )                      { col = ds2d( w3 )  ; }
	else if ( w3 == "MAX" )                     { col = MAXW        ; }
	else if ( w3.compare( 0, 4, "MAX-" ) == 0 ) { col = MAXW - ds2d( substr( w3, 5 ) )    ; }
	else                                        { err.seterrid( "PSYE031S", w3 ) ; return ; }

	if ( isnumeric( w4 ) )                      { width = ds2d( w4  )    ; }
	else if ( w4 == "MAX" )                     { width = MAXW - col + 1 ; }
	else if ( w4.compare( 0, 4, "MAX-" ) == 0 ) { width = MAXW - col - ds2d( substr( w4, 5 ) ) + 1 ; }
	else                                        { err.seterrid( "PSYE031S", w4 ) ; return          ; }

	if ( isnumeric( w5 ) )                      { depth = ds2d( w5 )     ; }
	else if ( w5 == "MAX" )                     { depth = MAXD - row + 1 ; }
	else if ( w5.compare( 0, 4, "MAX-" ) == 0 ) { depth = MAXD - row - ds2d( substr( w5, 5 ) ) + 1 ; }
	else                                        { err.seterrid( "PSYE031S", w5 ) ; return          ; }

	if ( ( row + depth - 1 ) > MAXD )
	{
		err.seterrid( "PSYE032L", d2ds( row ), d2ds( depth ), d2ds( MAXD ) ) ;
		return ;
	}
	if ( ( col + width - 1 ) > MAXW )
	{
		err.seterrid( "PSYE032M", d2ds( col ), d2ds( width ), d2ds( MAXW ) ) ;
		return ;
	}

	if ( width < 20 )
	{
		err.seterrid( "PSYE032N" ) ;
		return ;
	}
	if ( depth < 2 )
	{
		err.seterrid( "PSYE032O" ) ;
		return ;
	}

	Area_num   = num   ;
	Area_row   = row-1 ;
	Area_col   = col-1 ;
	Area_width = width ;
	Area_depth = depth - 1 ;
}


void Area::get_info( uint& row, uint& col, uint& width )
{
	row   = Area_row ;
	col   = Area_col ;
	width = Area_width ;
}


void Area::get_info( uint& row, uint& col, uint& width, uint& depth )
{
	row   = Area_row ;
	col   = Area_col ;
	width = Area_width ;
	depth = Area_depth ;
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
	// Scroll area so field t is visible:
	// ...above the top line - move to the top line
	// ...below the end line - move to the end line

	if ( t->field_area_row < pos )
	{
		pos = t->field_area_row ;
	}
	else
	{
		pos = t->field_area_row - Area_depth + 1  ;
	}
}


bool Area::cursor_on_area( uint row, uint col )
{
	return ( row >= Area_row && row <= ( Area_row + Area_depth ) &&
		 col >= Area_col && col <  ( Area_col + Area_width ) ) ;
}


int Area::scroll_up( uint row, uint col )
{
	bool onArea = false ;

	uint oldpos = pos ;
	uint amnt ;

	if ( pos == 0 ) { return -1 ; }

	if ( Area_depth == 1 )
	{
		amnt = 1 ;
	}
	else if ( row >  Area_row && row < ( Area_row + Area_depth ) &&
		  col >= Area_col && col < ( Area_col + Area_width ) )
	{
		amnt   = Area_depth + Area_row - row ;
		onArea = true ;
	}
	else
	{
		amnt = Area_depth - 1 ;
	}

	pos = ( amnt > pos ) ? 0 : pos - amnt ;

	return ( ( !onArea || ( onArea && row == Area_row + Area_depth ) ) ? 0 : oldpos - pos ) ;
}


int Area::scroll_down( uint row, uint col )
{
	bool onArea = false ;

	uint oldpos = pos ;
	uint amnt ;

	if ( pos == maxPos ) { return -1 ; }

	if ( Area_depth == 1 )
	{
		amnt = 2 ;
	}
	else if ( row > ( Area_row + 1 ) && row <= ( Area_row + Area_depth ) &&
		  col >= Area_col        && col <  ( Area_col + Area_width ) )
	{
		amnt   = row - Area_row ;
		onArea = true ;
	}
	else
	{
		amnt = Area_depth ;
	}

	pos = pos + amnt - 1 ;

	if ( pos > maxPos )
	{
		pos = maxPos ;
	}

	return ( ( !onArea || ( onArea && row == Area_row + 1 ) ) ? 0 : pos - oldpos ) ;
}


void Area::check_overlapping_fields( errblock& err, const string& area_name )
{
	uint idx = 1 ;

	uint i ;
	uint j ;
	uint k ;

	string t1 ;
	string t2 ;
	string t3 ;

	map<int, string>xref ;

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
				err.seterrid( "PSYE043A", it->first, xref[ fieldMap[ i ] ], area_name ) ;
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
		k = j + (*it)->text_value.size() ;
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
					err.seterrid( "PSYE043B", t2, t3, area_name ) ;
				}
				else
				{
					err.seterrid( "PSYE043C", t1, t2, area_name ) ;
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

	for ( auto it = fieldList.begin() ; it != fieldList.end() ; ++it )
	{
		if ( it->second->field_area_row >= pos &&
		     it->second->field_area_row < lim )
		{
			it->second->field_row     = it->second->field_area_row + Area_row - pos + 1 ;
			it->second->field_visible = true ;
			it->second->field_active  = true ;
		}
		else
		{
			it->second->field_visible = false ;
			it->second->field_active  = false ;
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
	if ( pos == 0 && maxRow < Area_depth )
	{
		return si1 ;
	}
	else if ( pos == 0 && maxRow >= Area_depth )
	{
		return si2 ;
	}
	else if ( pos == maxPos )
	{
		return si3 ;
	}
	return si4 ;
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

	string w2 ;
	string w3 ;
	string w4 ;
	string w5 ;
	string w6 ;
	string w7 ;
	string t  ;
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
		err.seterrid( "PSYE032K" ) ;
		return ;
	}
	if ( !isvalidName( w6 ) )
	{
		err.seterrid( "PSYE022J", w6, "Dynamic area" ) ;
		return ;
	}

	if ( w7 == "" )
	{
		err.seterrid( "PSYE032E" ) ;
		return ;
	}
	if ( !isvalidName( w7 ) )
	{
		err.seterrid( "PSYE022J", w7, "Dynamic area shadow" ) ;
		return ;
	}

	if ( isnumeric( w2 ) )                      { row = ds2d( w2  ) ; }
	else if ( w2 == "MAX" )                     { row = MAXD        ; }
	else if ( w2.compare( 0, 4, "MAX-" ) == 0 ) { row = MAXD - ds2d( substr( w2, 5 ) )    ; }
	else                                        { err.seterrid( "PSYE031S", w2 ) ; return ; }

	if ( isnumeric( w3 ) )                      { col = ds2d( w3 )  ; }
	else if ( w3 == "MAX" )                     { col = MAXW        ; }
	else if ( w3.compare( 0, 4, "MAX-" ) == 0 ) { col = MAXW - ds2d( substr( w3, 5 ) )    ; }
	else                                        { err.seterrid( "PSYE031S", w3 ) ; return ; }

	if ( isnumeric( w4 ) )                      { width = ds2d( w4  )    ; }
	else if ( w4 == "MAX" )                     { width = MAXW - col + 1 ; }
	else if ( w4.compare( 0, 4, "MAX-" ) == 0 ) { width = MAXW - col - ds2d( substr( w4, 5 ) ) + 1 ; }
	else                                        { err.seterrid( "PSYE031S", w4 ) ; return          ; }

	if ( isnumeric( w5 ) )                      { depth = ds2d( w5 )     ; }
	else if ( w5 == "MAX" )                     { depth = MAXD - row + 1 ; }
	else if ( w5.compare( 0, 4, "MAX-" ) == 0 ) { depth = MAXD - row - ds2d( substr( w5, 5 ) ) + 1 ; }
	else                                        { err.seterrid( "PSYE031S", w5 ) ; return          ; }

	if ( ( row + depth - 1 ) > MAXD )
	{
		err.seterrid( "PSYE032A", d2ds( row ), d2ds( depth ), d2ds( MAXD ) ) ;
		return ;
	}
	if ( ( col + width - 1 ) > MAXW )
	{
		err.seterrid( "PSYE032B", d2ds( col ), d2ds( width ), d2ds( MAXW ) ) ;
		return ;
	}

	if ( da_dataIn != "" )
	{
		dynArea_dataInsp = true ;
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
			err.seterrid( "PSYE032J" ) ;
			return ;
		}
	}
	else if ( err.RSN0() )
	{
		err.seterrid( "PSYE031G" ) ;
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
			err.seterrid( "PSYE032J" ) ;
			return ;
		}
	}
	else if ( err.RSN0() )
	{
		err.seterrid( "PSYE031G" ) ;
		return ;
	}

	t = parseString( err, rest, "SCROLL()" ) ;
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
			err.seterrid( "PSYE031G" ) ;
		}
		else
		{
			err.seterrid( "PSYE032C", t ) ;
		}
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
			       char inv_schar,
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
			field_shadow_value.insert( pos, 1, inv_schar ) ;
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
		field_shadow_value[ pos ] = inv_schar ;
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
	display_field( win, inv_schar, ddata_map, schar_map ) ;
	field_changed = true ;
}


int field::edit_field_backspace( WINDOW* win,
				 int col,
				 char inv_schar,
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
	edit_field_delete( win, col, inv_schar, ddata_map, schar_map ) ;
	return col ;
}


void field::field_erase_eof( WINDOW* win,
			     uint col,
			     char inv_schar,
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

	display_field( win, inv_schar, ddata_map, schar_map ) ;
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

	pos = ( col < field_col ) ? 0 : ( col - field_col ) ;

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

	if ( temp.colourChange )
	{
		field_colour1 &= ~clmask     ;
		field_colour1 |= temp.colour ;
	}

	if ( temp.intensChange )
	{
		field_colour1 &= ~inmask     ;
		field_colour1 |= temp.intens ;
	}

	if ( temp.hiliteChange )
	{
		field_colour1 &= ~himask     ;
		field_colour1 |= temp.hilite ;
	}

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
			   char inv_schar,
			   map<unsigned char, uint>& ddata_map,
			   map<unsigned char, uint>& schar_map )
{
	// For non-dynamic area fields: if an input field, truncate if value size > field size else for output fields
	// display field size bytes and leave field value unchanged (necessary for table display fields).

	// Display the null character as the field pad character and pad the field with the same character.

	// Colour/hilite taken from the shadow byte if valid (ie. corresponds to a TYPE(CHAR) ATTR entry).
	// Intensity and default colour/hilite taken from DATAIN/DATAOUT.
	// If no DATAIN/DATAOUT and shadow byte is not valid, default to colour WHITE.

	// 00 X0 00 00 - X is the INTENSITY
	// 00 0X 00 00 - X is the HILITE
	// 00 00 XX 00 - X is the COLOUR

	// Call ncurses touchline() for the field row as the update does not always appear without it.

	uint i ;

	uint intens = 0 ;
	uint attr2  = 0 ;
	uint attrd  = 0 ;
	uint colour = 0 ;

	const uint clmask = RED  | GREEN | YELLOW  | BLUE      | MAGENTA     |
			    TURQ | WHITE | A_BLINK | A_REVERSE | A_UNDERLINE ;

	char nullc = ( field_nulls ) ? '.' : ' ' ;
	char upad  = ( field_paduser ) ? field_paduchar : field_padchar ;
	char attr1 ;

	string t ;

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
					attr1  = inv_schar ;
					attrd  = ddata_map[ *ita ] ;
					intens = attrd & A_BOLD ;
					colour = attrd & clmask ;
					wattrset( win, ( colour & A_COLOR ) | field_intens | intens ) ;
					mvwaddch( win, field_row, field_col+i, ' ' ) ;
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
				if ( (*ita) == nulls )
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
	return ( row == field_row && col >= field_col && col <= field_endcol ) ;
}


void text::text_init( errblock& err,
		      int MAXW,
		      int MAXD,
		      uint& opt_field,
		      const string& line,
		      uint Area_num,
		      uint Area_col )
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

	if ( Area_num > 0 && ( w2 == "MAX" || w2.compare( 0, 4, "MAX-" ) == 0 ) )
	{
		err.seterrid( "PSYE031U" ) ;
		return ;
	}

	if ( isnumeric( w2 ) )                      { row = ds2d( w2 ) ; }
	else if ( w2 == "MAX" )                     { row = MAXD       ; }
	else if ( w2.compare( 0, 4, "MAX-" ) == 0 ) { row = MAXD - ds2d( substr( w2, 5 ) )    ; }
	else                                        { err.seterrid( "PSYE031S", w2 ) ; return ; }

	if ( isnumeric( w3 ) )                      { col = ds2d( w3 ) ; }
	else if ( w3 == "MAX" )                     { col = MAXW       ; }
	else if ( w3.compare( 0, 4, "MAX-" ) == 0 ) { col = MAXW - ds2d( substr( w3, 5 ) )    ; }
	else                                        { err.seterrid( "PSYE031S", w3 ) ; return ; }

	if ( cuaAttrName.count( w4 ) == 0 )
	{
		err.seterrid( "PSYE032F", w4 ) ;
		return ;
	}
	fType = cuaAttrName[ w4 ] ;

	if ( Area_num == 0 && row > MAXD )
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

	if ( Area_num > 0 )
	{
		text_area_row = text_row ;
		text_area_col = text_col ;
		text_col     += Area_col ;
		text_visible  = false ;
	}

	if ( upper( w5 ) == "EXPAND" )
	{
		text_value = strip( strip( subword( line, 6 ) ), 'B', '"' ) ;
		if ( text_value.size() == 0 )
		{
			err.seterrid( "PSYE041K" ) ;
			return ;
		}
		l = ( MAXW - col ) / text_value.size() + 1 ;
		text_value = substr( copies( text_value, l ), 1, ( MAXW - col + 1 ) ) ;
	}
	else
	{
		text_value = strip( strip( subword( line, 5 ) ), 'B', '"' ) ;
	}

	text_endcol = text_col + text_value.size() - 1 ;

	if ( fType == PS )
	{
		if ( Area_num > 99 )
		{
			err.seterrid( "PSYE042V" ) ;
			return ;
		}
		text_name = "ZPS" + d2ds( Area_num, 2 ) + d2ds( ++opt_field, 3 ) ;
	}
}


void text::text_display( WINDOW* win )
{
	if ( text_visible )
	{
		wattrset( win, cuaAttr[ text_cua ] | text_intens ) ;
		mvwaddstr( win, text_row, text_col, text_xvalue.c_str() ) ;
	}
}


bool text::cursor_on_text( uint row, uint col )
{
	return ( row == text_row && col >= text_col && col <= text_endcol ) ;
}


void pdc::display_pdc_avail( WINDOW* win, attType type, int pos )
{
	string t = right( d2ds( pos ), 2 ) + ". " + pdc_xdesc ;

	wattrset( win, cuaAttr[ type ] | pdc_intens ) ;
	mvwaddstr( win, pos, 4, t.c_str() ) ;
}


void pdc::display_pdc_unavail( WINDOW* win, attType type, int pos )
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
				 p_funcPOOL->put1( err, var, "" ) ;
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
		p_funcPOOL->put1( err, var, val ) ;
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

	if ( abc_mnem1 > 0 )
	{
		uint i = abc_mnem1 - 1 ;
		wattrset( win, cuaAttr[ ABU ] | abc_intens | A_UNDERLINE ) ;
		mvwaddch( win, 0, abc_col+i, abc_desc[ i ] ) ;
	}
}


void abc::display_pd( errblock& err, const string& zvars, uint p_row, uint p_col, uint row )
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
			it->display_pdc_avail( win, ( currChoice == ( i - 1 ) ) ? AMT : PAC, i ) ;
		}
	}
	choiceVar = zvars ;
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

	return ( ( row >  w_row && row <= ( w_row + abc_maxh ) )  &&
		 ( col >= w_col && col <  ( w_col + abc_maxw + 10 ) ) ) ;
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
	     !datatype( w3, 'W' ) )
	{
		err.seterrid( "PSYE019E" ) ;
		return ;
	}

	row = ds2d( w2 ) ;
	col = ds2d( w3 ) ;

	if ( isnumeric( w4 ) )                      { width = ds2d( w4 )     ; }
	else if ( w4 == "MAX" )                     { width = MAXW - col + 1 ; }
	else if ( w4.compare( 0, 4, "MAX-" ) == 0 ) { width = MAXW - col - ds2d( substr( w4, 5 ) ) + 1 ; }
	else                                        { err.seterrid( "PSYE019E", w4 ) ; return          ; }

	if ( isnumeric( w5 ) )                      { depth = ds2d( w5 )     ; }
	else if ( w5 == "MAX" )                     { depth = MAXD - row + 1 ; }
	else if ( w5.compare( 0, 4, "MAX-" ) == 0 ) { depth = MAXD - row - ds2d( substr( w5, 5 ) ) + 1 ; }
	else                                        { err.seterrid( "PSYE019E", w5 ) ; return          ; }

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


void Box::display_box( WINDOW* win, string title, uint colour, uint intens )
{
	wattrset( win, colour | intens | box_intens ) ;
	draw_box( win, title ) ;
}


void Box::display_box( WINDOW* win, string title )
{
	wattrset( win, box_colour | box_intens ) ;
	draw_box( win, title ) ;
}


void Box::draw_box( WINDOW* win, string& title )
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
