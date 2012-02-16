
#ifndef __CHESS_DATA_TYPES_GENERIC_H__
#define __CHESS_DATA_TYPES_GENERIC_H__
#define MIN(__x__, __y__) (__x__<__y__?__x__:__y__)
#define MAX(__x__, __y__) (__x__>__y__?__x__:__y__) 

#define OK 1
#define ERROR 0
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>

#define U64(u) (u##ULL)

using namespace std;

typedef unsigned char UINT8;
typedef char INT8;
typedef unsigned short int UINT16;
typedef short int INT16;
typedef unsigned long int UINT32;
typedef unsigned long long int UINT64;
typedef long int INT32;
typedef long int STATUS;
enum BOOL { FALSE, TRUE };

void ReverseString(char *);
char *print64(UINT64, char *);
char *print64_hex(UINT64, char *);

UINT32 getRandomNumber(UINT32, UINT32);

#endif
