/*
 * keyboard.h
 *
 *  Created on: Jul 18, 2011
 *      Author: adrian
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_


#include <cyg/kernel/kapi.h>
#include <stdio.h>


#define PARPORT_BASE1 0xD800040
#define PARPORT_BASE2 0xD800060
#define KEYBOARD_BASE 0xD800080
#define PR_REG 0
#define SR_REG 1
#define IOSEL_REG 1
#define CTL_REG 2
#define CMD_REG  2
#define COLUMN_MASK 0x0F
#define READY_MASK 0x80
#define NOT_READY 255
#define ROWS 4
#define COLUMNS 4




void initPorts(void);
void writePort(char x,int parport_number);
char readPort(int parport_number);
char getKey(char row,char col);
char readKeyboard(void);
void testKeyboard(void);
void press_key(int key);

#endif /* KEYBOARD_H_ */
