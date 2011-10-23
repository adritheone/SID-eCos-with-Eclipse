/* this is a very simple program meant to demonstrate
   a basic use of time, alarms and alarm-handling functions
   in eCos */

#include <cyg/kernel/kapi.h>
#include "keyboard.c"

#include <stdio.h>

#define NTHREADS 2
#define STACKSIZE 4096

static cyg_handle_t thread[NTHREADS];

static cyg_thread thread_obj[NTHREADS];
static char stack[NTHREADS][STACKSIZE];

static void writer( cyg_addrword_t data );
static void reader(cyg_addrword_t data);

/* we install our own startup routine which sets up
    threads and starts the scheduler */
void cyg_user_start(void)

{
  cyg_thread_create(4, writer, (cyg_addrword_t) 0,
		    "writer_thread", (void *) stack[0],
		    STACKSIZE, &thread[0], &thread_obj[0]);

  cyg_thread_create(4, reader, (cyg_addrword_t) 0,
		    "reader_thread", (void *) stack[1],
		    STACKSIZE, &thread[1], &thread_obj[1]);
  initPorts();
  cyg_thread_resume(thread[1]);
  cyg_thread_resume(thread[0]);


}

/* we need to declare the alarm handling function (which is
   defined below), so that we can pass it to
   cyg_alarm_initialize() */
cyg_alarm_t test_alarm_func;

/* alarm_prog() is a thread which sets up an alarm which is then
   handled by test_alarm_func() */
static void writer(cyg_addrword_t data)
{
  printf("Starting writer thread...\n");
  cyg_handle_t test_counterH, system_clockH, test_alarmH;
  cyg_tick_count_t ticks;
  cyg_alarm test_alarm;
  unsigned how_many_alarms = 0, prev_alarms = 0, tmp_how_many;
  int key= 0x0;
  system_clockH = cyg_real_time_clock();
  cyg_clock_to_counter(system_clockH, &test_counterH);
  cyg_alarm_create(test_counterH, test_alarm_func,
		   (cyg_addrword_t) &how_many_alarms,
		   &test_alarmH, &test_alarm);
  cyg_alarm_initialize(test_alarmH, cyg_current_time()+200, 200);

  /* get in a loop in which we read the current time and
     print it out, just to have something scrolling by */
  for (;;) {
    //ticks = cyg_current_time();
    //printf("Time is %llu\n", ticks);
    /* note that we must lock access to how_many_alarms, since the
       alarm handler might change it.  this involves using the
       annoying temporary variable tmp_how_many so that I can keep the
       critical region short */
    cyg_scheduler_lock();
    tmp_how_many = how_many_alarms;
    cyg_scheduler_unlock();
    if (prev_alarms != tmp_how_many) {
    	press_key(key);

    	if (key==0xf)key=0x0;
      printf("  --->writer thread writes: %x\n", key);
      prev_alarms = tmp_how_many;
      key++;
    }
    cyg_thread_delay(15);
  }
}

static void reader(cyg_addrword_t data){
	char old_key='e';
	char key;

	printf("Starting reader thread...\n");

	while(1){
		key=readKeyboard();
	//	printf("Reader thread reads: %c \n",key);
		if (key!=old_key){
			old_key=key;
		printf("Reader thread reads: %c \n",key);
		}
	}
}

/* test_alarm_func() is invoked as an alarm handler, so
   it should be quick and simple.  in this case it increments
   the data that is passed to it. */
void test_alarm_func(cyg_handle_t alarmH, cyg_addrword_t data)
{
  ++*((unsigned *) data);
}
