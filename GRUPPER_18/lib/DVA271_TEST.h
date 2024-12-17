#ifndef DVA271_TEST_H
#define DVA271_TEST_H

int init(int flag);
int test_leds();
int test_hc595(int fake_temp);
int test_write();
int test_read();
int multithread_fun();

#endif
