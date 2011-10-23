/* this is a simple hello world program */

#include "keyboard.h"



void initPorts(){
//parpot 1 is output mode and parport2 is input mode
	cyg_addrword_t *iosel_address1 = PARPORT_BASE1 + 4;
	cyg_addrword_t *iosel_address2 = PARPORT_BASE2 + 4;
	cyg_addrword_t *cntrl_address2 = PARPORT_BASE2 + 8;

	(*iosel_address1)=0x55;
	(*iosel_address2)=0xAA;
	(*cntrl_address2)=224;

}






/* Parport I/O modes
 *
 * I/O mode 		BIDEN			CNTRL bit 5 		  IO SEL
 *
 * Input			  0					X					0xAA
 *
 * Output			  0					X					0x55
 *
 * Output			  1					0					X
 *
 * Input			  1					1					X
 *
 * */

void writePort(char x, int parport_number){
	cyg_addrword_t *pr_address = PARPORT_BASE1 + PR_REG*4;//By default we will write in port1
		 if (parport_number == 2){
			pr_address= PARPORT_BASE2 + PR_REG*4;
		}
	(*pr_address)=x;

}

char readPort(int parport_number){
	char x,ready;
	cyg_addrword_t *pr_address = PARPORT_BASE2 + PR_REG*4; //By default we will read from port2
	if (parport_number == 1){
		pr_address = PARPORT_BASE1 + PR_REG*4;
	}
	x = (*pr_address);
	ready=(x & READY_MASK);
	if (ready != READY_MASK){return NOT_READY;}


	return (x & COLUMN_MASK);
}

char readKeyboard(){
	char row,row_num,col,key;
	while (1){
		for (row=0;row<ROWS;row++){
			row_num= 1<<row;
			writePort(row_num,1);
			col=readPort(2);

			if (col != 0 && col!=255){
				key = getKey(row_num,col);
				return key;
			}
			}

		}
	return 0;
	}




char getKey(char row, char col){

	switch (row){
	case 1:
		switch (col){
		case 1: return '1';
		case 2: return '2';
		case 4: return '3';
		case 8: return 'C';
		}
	case 2:
		switch (col){
		case 1: return '4';
		case 2: return '5';
		case 4: return '6';
		case 8: return 'D';
		}
	case 4:
		switch (col){
		case 1: return '7';
		case 2: return '8';
		case 4: return '9';
		case 8: return 'E';
		}

	case 8:
		switch (col){
		case 1: return 'A';
		case 2: return '0';
		case 4: return 'B';
		case 8: return 'F';
		}

	}
 return 'e';

}

//Sets the keyboard to the given value
void press_key(int key){
	cyg_addrword_t *keyboard_address = KEYBOARD_BASE;
	(*keyboard_address)=key;

}





