#ifndef PROCESS_H 
#define PROCESS_H 

#include "utils.h"

#define LEGAL_CHAR_SET "abcdefghijklmnopqrstuvwxyzABSCDEFGHIJKLMNOPQRSTUVWQYZ1234567890._:-"

/* error codes for all functions */
enum errors { 
    CMDNFND = -1,
    EARGNUM = 1, 
    EEXITST = 2, 
    EPARSE  = 3,
    ETZ     = 4,
    EPRINT  = 5,
    EBADMA  = 6,
    ENOMEMR	= 7,
    EINVADDR = 8,
    EINVPATH = 9,
    ENTFND	 = 10,
    ENOCWD	 = 11,
};

int process(const input_t *args);
#endif
