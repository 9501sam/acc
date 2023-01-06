#include <assert.h>

#include "flag.h"



void test_flag_init(){
	Flags *flags = flag_init();
	assert(flags->record == 0);
}

void test_flag_reset(){
	Flags *flags = flag_init();
	flag_on(flags, FLAG_INCLUDE);
	flag_reset(flags);
	assert(flags->record == 0);
}

void test_flag_on(){
	Flags *flags = flag_init();
	flag_on(flags, FLAG_DECLARE);
	assert(flags->record == 1 << 31);
}

void test_flag_off(){
	Flags *flags = flag_init();
	flag_on(flags, FLAG_DECLARE);
	assert(flags->record == 1 << 31);
	flag_on(flags, FLAG_INCLUDE);
	assert(flags->record == (1 << 31) + (1 << 30));
	flag_off(flags, FLAG_DECLARE);
	assert(flags->record == 1 << 30);
}

void test_flag_switch(){
	Flags *flags = flag_init();
	flag_on(flags, FLAG_DECLARE);
	assert(flags->record == 1 << 31);
	flag_switch(flags, FLAG_DECLARE);
    assert(flags->record == 0);
}

void test_flag_isset(){
	Flags *flags = flag_init();
	flag_on(flags, FLAG_DECLARE);
	assert(flag_isset(flags, FLAG_DECLARE) == true);
	assert(flag_isset(flags, FLAG_INCLUDE) == false);
}
	
int main(int argc, char** argv){
	test_flag_init();
	test_flag_reset();
	test_flag_on();
	test_flag_off();
    test_flag_switch();
	test_flag_isset();
	return 0;
}
