/* this is a simple hello world program */
#include <cyg/kernel/kapi.h>

#include <stdio.h>
#include "keyboard.c"

char col;

int main(void)
{
  printf("Testing parallel ports\n");
  initPorts();
  col=0;

testKeyboard();

  return 0;
}




void testKeyboard(){
char key;
char old_key='g';
int i;
//key = readKeyboard();
printf("I can read: %c",key);

for (i=0;i<0x10;i++){
	press_key(i);
	printf("Trying to press %d ",i);
	key=readKeyboard();
	printf("I can read: %c \n",key);
}
for (i=0;i<=0x10;i++){
	press_key(i);
	printf("Trying to press %d ",i);
	key=readKeyboard();
	printf("I can read: %c \n",key);
}


/*while (1){
	key = readKeyboard();
	if (key!=old_key){
		old_key=key;
		printf("The keyboard has been pressed:\n");
		printf(key);
		printf("\n");

	}
}*/




writePort(1,1);
col=readPort(2);
while (col== NOT_READY){col=readPort(2);}

printf("When I set row number 0 I can read %d from column \n ",col);


writePort(2,1);
col=readPort(2);
while (col== NOT_READY){col=readPort(2);}

printf("When I set row number 1 I can read %d from column \n ",col);

writePort(4,1);

col=readPort(2);
while (col == NOT_READY){col=readPort(2);}

	printf("When I set row number 2 I can read %d from column \n ",col);


writePort(8,1);

col=readPort(2);
while (col== NOT_READY){col=readPort(2);}

printf("When I set row number 3 I can read %d from column \n ",col);



}
