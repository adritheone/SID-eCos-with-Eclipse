#include "hexkeyboard.h"








namespace hexadecimal_keyboard
{
using namespace std;

void HexKeyboard::initkeys(){
const int initialkeys[ROWS][COLUMNS]={
					{0,1,0,0},
					{0,0,1,0},	
					{0,0,0,0},	
					{1,0,0,1}
};
int i,j;
	for (i=0;i<ROWS;i++){
		for (j=0;j<COLUMNS;j++){
			keys[i][j]=initialkeys[i][j];		
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

//Return the pin that is connected to a named pin
//We recognize "col0-3"
vector<sid::pin*> HexKeyboard::connected_pins (const string& name) NT
{
	vector<sid::pin*> pins;
	if (name=="col0")pins.push_back(col0);
	if (name=="col1")pins.push_back(col1);
	if (name=="col2")pins.push_back(col2);
	if (name=="col3")pins.push_back(col3);
		
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
	
	return sid::component::not_found;


}









//Refresh output. Drives Column outputs depending on driven rows and keyboard state
void HexKeyboard::refresh_output(Row_pin* row)
{
	int column;
	for (column=0;column<COLUMNS; column++){
		if (keys[row->getRowNumber()][column]==1){
		drive_column(column,1);
		}	

	}
	
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








//The remaining abstract methods from sid::component need to be implemented

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



