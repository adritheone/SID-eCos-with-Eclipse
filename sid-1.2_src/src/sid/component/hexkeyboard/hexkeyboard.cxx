#include "hexkeyboard.h"








namespace hexadecimal_keyboard
{
using namespace std;

void HexKeyboard::initkeyboard(){
const int initialkeys[ROWS][COLUMNS]={
					{0,0,0,0},
					{0,0,0,0},	
					{0,0,0,0},	
					{0,0,0,0}
};
int i,j;
	for (i=0;i<ROWS;i++){
		for (j=0;j<COLUMNS;j++){
			keys[i][j]=initialkeys[i][j];
			activated_rows[i]=false;
			row_values[i]=	(sid::host_int_4) 0;
		}
	}
	

}



//Drive a value to the required pin
void HexKeyboard::drive_column (int col_number, sid::host_int_4 value)
{
	switch (col_number) 
	{	
		case 0:
			col0->driven(value);
			break;
		case 1:
			col1->driven(value);
			break;
		case 2:
			col2->driven(value);
			break;
		case 3:
			col3->driven(value);
			break;
	}			
}

sid::bus* HexKeyboard::connected_bus(const string& name) NT {
	//if (name == "keyboard_bus")return &keyboard_bus;
	return 0;
}
//Return the pin that is connected to a named pin
//We recognize "col0-3"
vector<sid::pin*> HexKeyboard::connected_pins (const string& name) NT
{
	vector<sid::pin*> pins;
	if (name=="col0")pins.push_back(col0);
	if (name=="col1")pins.push_back(col1);
	if (name=="col2")pins.push_back(col2);
	if (name=="col3")pins.push_back(col3);
	if (name=="ready")pins.push_back(ready);
		
	return pins;

}

//Connect a pin to a named pin
//Our output pins are col0-3

//We do NOT allow multiple pins to be connected to our output pins
sid::component::status HexKeyboard::connect_pin (const string& name, sid::pin* pin) NT
{
	if (name=="col0"){
	col0=pin;
	return sid::component::ok;
	}
	
	if (name=="col1"){
	col1=pin;
	return sid::component::ok;	
	}
		
	if (name=="col2"){
	col2=pin;
	return sid::component::ok;	
	}
	
	if (name=="col3"){
	col3=pin;
	return sid::component::ok;
	}
	if (name=="ready"){
	ready=pin;
	return sid::component::ok;
	}


	return sid::component::not_found;
}

//Disconnect a pin from a named pin
sid::component::status
HexKeyboard::disconnect_pin (const string& name, sid::pin* pin) NT
{
	if (name == "col0" && col0 == pin)
	{
		col0=0;
		return sid::component::ok;	
	}
	else if (name == "col1" && col1 == pin)
	{
		col1=0;
		return sid::component::ok;	
	}
	else if (name == "col2" && col2 == pin)
	{
		col2=0;
		return sid::component::ok;	
	}
	else if (name == "col3" && col3 == pin)
	{
		col3=0;
		return sid::component::ok;	
	}
	else if (name == "ready" && ready == pin)
	{
		ready=0;
		return sid::component::ok;	
	}
	return sid::component::not_found;


}



//Press key
void HexKeyboard::press_key(sid::host_int_4 value){
	clean_keyboard();



switch (value)
{
case 0x0:
keys[3][1] = 1;
break;

case 0x1:
keys[0][0] = 1;
break;

case 0x2:
keys[0][1] = 1;
break;

case 0x3:
keys[0][2] = 1;
break;

case 0x4:
keys[1][0] = 1;
break;

case 0x5:
keys[1][1] = 1;
break;

case 0x6:
keys[1][2] = 1;
break;

case 0x7:
keys[2][0] = 1;
break;

case 0x8:
keys[2][1] = 1;
break;

case 0x9:
keys[2][2] = 1;
break;

case 0xa:
keys[3][0] = 1;
break;

case 0xb:
keys[3][2] = 1;
break;

case 0xc:
keys[0][3] = 1;
break;

case 0xd:
keys[1][3] = 1;
break;

case 0xe:
keys[2][3] = 1;
break;

case 0xf:
keys[3][3] = 1;
break;

default:
keys[3][3] = 1;

}

}

void HexKeyboard::clean_keyboard(){
	int i,j;	
	for(i=0;i<COLUMNS;i++){
		for (j=0;j<ROWS;j++){
			keys[i][j]=0;
		}
	}

}

//Refresh output. Drives Column outputs depending on driven rows and keyboard state
void HexKeyboard::refresh_output(Row_pin* row, sid::host_int_4 value)
{
	int column,rowi;
	int row_number = row->getRowNumber();
	row_values[row_number]=value;
	activated_rows[row_number]=true;

	if ((activated_rows[0])&&(activated_rows[1])&&(activated_rows[2])&&(activated_rows[3])){	//We shall only refresh output when all row pins have been driven
	
		activated_rows[0] = activated_rows[1] = activated_rows[2] = activated_rows[3] = false;
		for(rowi=0;rowi<ROWS;rowi++){
			if (row_values[rowi] == (sid::host_int_4) 1){
				for (column=0;column<COLUMNS; column++){
					if (keys[rowi][column]==1){
						drive_column(column,(sid::host_int_4)1);
					}
					else{
						drive_column(column,(sid::host_int_4)0);
					}
					
					
				}
				break;
			}	
		
		}		
		
		ready->driven((sid::host_int_4)1);
		
	}
	else ready->driven((sid::host_int_4)0);
}

//Return a list of pin names which are visible to other components
vector<string> HexKeyboard::pin_names() NT
{
	vector<string> pins;
	pins.push_back("row0");
	pins.push_back("row1");
	pins.push_back("row2");
	pins.push_back("row3");
	
	return pins;
}

//Find a pin of a given name.
//We recognize "row0-3"
sid::pin* HexKeyboard::find_pin (const string& name) NT
{
	if (name=="row0") return& row0;
	if (name=="row1") return& row1;
	if (name=="row2") return& row2;
	if (name=="row3") return& row3;
	
	return 0;
}



vector<string> HexKeyboard::bus_names () NT{
	vector<string> buses;
	buses.push_back("keyboard_bus");
	
	return buses;
}
sid::bus* HexKeyboard::find_bus(const string& name) NT{
	if (name=="keyboard_bus") return &keyboard_bus;
	return 0;
}


sid::bus::status
HexKeyboard::Keyboard_bus::write(sid::host_int_4 addr, sid::little_int_4 data) NT{return sid::bus::unpermitted;}




//The remaining abstract methods from sid::component need to be implemented
sid::bus::status HexKeyboard::Keyboard_bus::read(sid::host_int_4 addr, sid::little_int_1& data) NT{return sid::bus::unpermitted;}
sid::bus::status HexKeyboard::Keyboard_bus::read(sid::host_int_4 addr, sid::big_int_1& data) NT{return sid::bus::unpermitted;}
sid::bus::status HexKeyboard::Keyboard_bus::read(sid::host_int_4 addr, sid::little_int_2& data) NT{return sid::bus::unpermitted;}
sid::bus::status HexKeyboard::Keyboard_bus::read(sid::host_int_4 addr, sid::big_int_2& data) NT{return sid::bus::unpermitted;}
sid::bus::status HexKeyboard::Keyboard_bus::read(sid::host_int_4 addr, sid::little_int_4& data) NT{return sid::bus::unpermitted;}
sid::bus::status HexKeyboard::Keyboard_bus::read(sid::host_int_4 addr, sid::big_int_4& data) NT{return sid::bus::unpermitted;}
sid::bus::status HexKeyboard::Keyboard_bus::read(sid::host_int_4 addr, sid::little_int_8& data) NT{return sid::bus::unpermitted;}
sid::bus::status HexKeyboard::Keyboard_bus::read(sid::host_int_4 addr, sid::big_int_8& data) NT{return sid::bus::unpermitted;}

sid::bus::status HexKeyboard::Keyboard_bus::write(sid::host_int_4 addr, sid::little_int_1 data) NT{
keyboard->press_key(data);

switch (addr)
{
case 0x0:
keyboard->press_key(data);
break;


default:
return sid::bus::unmapped;
}
return sid::bus::ok;
}

sid::bus::status HexKeyboard::Keyboard_bus::write(sid::host_int_4 addr, sid::big_int_1 data) NT{return sid::bus::unpermitted;}
sid::bus::status HexKeyboard::Keyboard_bus::write(sid::host_int_4 addr, sid::little_int_2 data) NT{return sid::bus::unpermitted;}

sid::bus::status HexKeyboard::Keyboard_bus::write(sid::host_int_4 addr, sid::big_int_2 data) NT{return sid::bus::unpermitted;}
sid::bus::status HexKeyboard::Keyboard_bus::write(sid::host_int_4 addr, sid::big_int_4 data) NT{return sid::bus::unpermitted;}
sid::bus::status HexKeyboard::Keyboard_bus::write(sid::host_int_4 addr, sid::little_int_8 data) NT{return sid::bus::unpermitted;}

sid::bus::status HexKeyboard::Keyboard_bus::write(sid::host_int_4 addr, sid::big_int_8 data) NT{return sid::bus::unpermitted;}


vector<string>
HexKeyboard::attribute_names() NT
{
return vector<string>();
}
vector<string>
HexKeyboard::attribute_names(const string& category) NT
{
return vector<string>();
}
string
HexKeyboard::attribute_value(const string& name) NT
{
// No attributes-return the empty string for any attribute value.
return string();
}
sid::component::status
HexKeyboard::set_attribute_value(const string& name, const string& value) NT
{
// No attributes-return not_found regardless of attribute name.
return sid::component::not_found;
}
vector<sid::component*>
HexKeyboard::related_components(const string& rel) NT
{
// No related components.
return vector<sid::component*>();
}
sid::component::status
HexKeyboard::unrelate(const string& rel, sid::component* c) NT
{
// No related components; always unfound.
return sid::component::not_found;
}
sid::component::status
HexKeyboard::relate(const string& rel, sid::component* c) NT
{
// No related components; always unfound.
return sid::component::not_found;
}
vector<string>
HexKeyboard::relationship_names() NT
{
// No relations.
return vector<string>();
}



/* Following methods are used to create library,
   and store information about hexadecimal keyboard in library. 
*/
static vector<string>
HexKeyboardListTypes() NT
{
  vector<string> types;
  types.push_back(string("hw-hexkeyboard-1"));
  return types;
}


static component*
HexKeyboardCreate(const string& typeName) NT
{
  if (typeName == "hw-hexkeyboard-1")
    return new HexKeyboard();
  else
    return 0;
}


static void
HexKeyboardDelete(sid::component* c) NT
{
  delete dynamic_cast<HexKeyboard*>(c);
}


}//end namespace
// This symbol is used by the library loader to validate the library
// and instantiate components of the types supported by this library.
DLLEXPORT extern const sid::component_library hexkeyboard_component_library;

const sid::component_library hexkeyboard_component_library =
{
  sid::COMPONENT_LIBRARY_MAGIC,
  & hexadecimal_keyboard::HexKeyboardListTypes, 
  & hexadecimal_keyboard::HexKeyboardCreate,
  & hexadecimal_keyboard::HexKeyboardDelete
};



