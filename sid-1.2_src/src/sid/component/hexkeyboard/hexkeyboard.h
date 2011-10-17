
// hexkeyboard.h -


#ifndef HEXKEYBOARD_H
#define HEXKEYBOARD_H

#include <sidtypes.h>
#include <sidcomp.h>
#include <sidso.h>
#include <sidcomputil.h>
#include <sidpinutil.h>
#include <sidbusutil.h>	



using std::vector;
using std::string;
using std::ostream;
using std::istream;
using std::ios;
using std::cerr;
using std::cout;

using sid::component;
using sid::host_int_1;
using sid::host_int_2;
using sid::host_int_4;
using sid::component_library;
using sid::COMPONENT_LIBRARY_MAGIC;
//using sidutil::no_bus_component;
using sidutil::fixed_pin_map_component;
using sidutil::fixed_bus_map_component;
using sidutil::no_accessor_component;
using sidutil::no_relation_component;

namespace hexadecimal_keyboard
	{

using namespace std;

//Implementing a 4x4 hexadecimal keyboard
#define ROWS 4
#define COLUMNS 4


// None of the sid API routines is allowed to throw an exception.
#define NT throw()






class HexKeyboard: public virtual sid::component,
			protected fixed_bus_map_component,
		    protected fixed_pin_map_component,
		    protected no_accessor_component,
		    protected no_relation_component
{
public:
	HexKeyboard():row0(this,0),row1(this,1),row2(this,2),row3(this,3),keyboard_bus(this){
		initkeyboard();		}
	// dummy dtor
	~HexKeyboard() throw ()
	{
	}
//Provide implementations for abstracgt methods in sid::component
//See include/sidcomp.h

vector<string> pin_names() NT;
sid::pin* find_pin(const string& name) NT;
sid::component::status connect_pin(const string& name, sid::pin* pin) NT;
sid::component::status disconnect_pin(const string& name, sid::pin* pin) NT;
vector<sid::pin*> connected_pins(const string& name) NT;
//vector<string> accessor_names() NT;
//sid::component::status connect_accessor(const string& name, sid::bus* bus) NT;
//sid::component::status disconnect_accessor(const string& name, sid::bus* bus) NT;
vector<string> bus_names() NT;
sid::bus* find_bus(const string& name) NT;
sid::bus* connected_bus(const string& name) NT;
vector<string> attribute_names() NT;
vector<string> attribute_names(const string& category) NT;
string attribute_value(const string& name) NT;
sid::component::status set_attribute_value(const string& name, const string& value) NT;
vector<string> relationship_names() NT;
sid::component::status relate(const string& rel, sid::component* c) NT;
sid::component::status unrelate(const string& rel, sid::component* c) NT;
vector<sid::component*> related_components(const string& rel) NT;
//sid::bus::status read(sid::host_int_4 addr, sid::little_int_1& data);
//sid::bus::status read(sid::host_int_4 addr, sid::big_int_1& data);
//sid::bus::status read(sid::host_int_4 addr, sid::little_int_2& data);
//sid::bus::status read(sid::host_int_4 addr, sid::big_int_2& data);
//sid::bus::status read(sid::host_int_4 addr, sid::little_int_4& data);
//sid::bus::status read(sid::host_int_4 addr, sid::big_int_4& data);
//sid::bus::status read(sid::host_int_4 addr, sid::little_int_8& data);
//sid::bus::status read(sid::host_int_4 addr, sid::big_int_8& data);
//sid::bus::status write(sid::host_int_4 addr, sid::little_int_1 data);
//sid::bus::status write(sid::host_int_4 addr, sid::big_int_1 data);
//sid::bus::status write(sid::host_int_4 addr, sid::little_int_2 data);
//sid::bus::status write(sid::host_int_4 addr, sid::big_int_2 data);
//sid::bus::status write(sid::host_int_4 addr, sid::little_int_4 data);
//sid::bus::status write(sid::host_int_4 addr, sid::big_int_4 data);
//sid::bus::status write(sid::host_int_4 addr, sid::little_int_8 data);
//sid::bus::status write(sid::host_int_4 addr, sid::big_int_8 data);


private: 
void initkeyboard();
void clean_keyboard();
void drive_column(int col_number, sid::host_int_4 value);
void press_key(sid::host_int_4 value);
sid::host_int_4 row_values[ROWS];
bool activated_rows[ROWS];
int keys[ROWS][COLUMNS];




//Receviers of my output pins
sid::pin* col0;
sid::pin* col1;
sid::pin* col2;
sid::pin* col3; 
sid::pin* ready;



friend class Row_pin;


class Row_pin: public sid::pin //input pins
{
	//Row_pin is a specialized pin.
	//It calls HexKeyboard->refresh_output(*Row_pin row) when driven
	public:
		Row_pin() NT;
		Row_pin (HexKeyboard* k,int num) :keyboard(k), row_number(num) {}
		void driven(sid::host_int_4 value) NT {keyboard->refresh_output(this,value);}
		void setRowNumber(int number){row_number=number;}		
		int getRowNumber(){return row_number;}
	private:
		HexKeyboard* keyboard;
		int row_number;
		
};

void refresh_output(Row_pin* row, sid::host_int_4 value);
 Row_pin row0,row1,row2,row3;
 






friend class Keyboard_bus;


class Keyboard_bus: public sid::bus //input bus
{

	public:
		Keyboard_bus() NT;
		Keyboard_bus (HexKeyboard* k) :keyboard(k){}
		// Prototypes for bus read/write methods of all kinds.
		sid::bus::status read(sid::host_int_4 addr, sid::little_int_1& data) NT;
		sid::bus::status read(sid::host_int_4 addr, sid::big_int_1& data) NT;
		sid::bus::status read(sid::host_int_4 addr, sid::little_int_2& data) NT;
		sid::bus::status read(sid::host_int_4 addr, sid::big_int_2& data) NT;
		sid::bus::status read(sid::host_int_4 addr, sid::little_int_4& data) NT;
		sid::bus::status read(sid::host_int_4 addr, sid::big_int_4& data) NT;
		sid::bus::status read(sid::host_int_4 addr, sid::little_int_8& data) NT;
		sid::bus::status read(sid::host_int_4 addr, sid::big_int_8& data) NT;
		
		sid::bus::status write(sid::host_int_4 addr, sid::little_int_1 data) NT;
		sid::bus::status write(sid::host_int_4 addr, sid::big_int_1 data) NT;
		sid::bus::status write(sid::host_int_4 addr, sid::little_int_2 data) NT;
		sid::bus::status write(sid::host_int_4 addr, sid::big_int_2 data) NT;
		sid::bus::status write(sid::host_int_4 addr, sid::little_int_4 data) NT;
		sid::bus::status write(sid::host_int_4 addr, sid::big_int_4 data) NT;
		sid::bus::status write(sid::host_int_4 addr, sid::little_int_8 data) NT;
		sid::bus::status write(sid::host_int_4 addr, sid::big_int_8 data) NT;
		


		
		
		
	private:
		HexKeyboard* keyboard;
		
};





Keyboard_bus keyboard_bus;


 };
 

}



#endif // PARPORT_H	
