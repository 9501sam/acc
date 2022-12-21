#ifndef __FLAG_H__
#define __FLAG_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef enum{
	FLAG_DECLARE = 0,
	FLAG_INCLUDE,
	FLAGSIZE
} Flag;

// all flag record in one integer 'record' from left to right
typedef struct{
	int record;
} Flags;

static inline Flags* flag_init(){
	Flags* flags = (Flags*)calloc(1, sizeof(Flags));
	return flags;
}

static inline void flag_reset(Flags *flags){
	flags->record = 0;
}

static inline void flag_on(Flags *flags, Flag ind){
	flags->record |= 1 << (31 - ind);
}

static inline void flag_off(Flags *flags, Flag ind){
	flags->record &= ~(1 << (31 - ind));
}

static inline bool flag_isset(Flags *flags, Flag ind){
	return 1 & (flags->record >> (31 - ind));
}

#endif /* __FLAG_H__ */
