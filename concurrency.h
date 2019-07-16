#include <iostream>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <time.h>
#include "debug.h"
using namespace std;

#define MAX_OBJ (10000)
#define MAX_OPERATION (10)
#define MAX_TRANSACTION (1000)

typedef struct _DATA {
	int val;
	pthread_mutex_t mutex;
} DATA;

typedef struct _OPERATION{
  int dataid;
  int rw; // 0-> read; 1-> write
} OPERATION;

typedef struct _TRANSACTION {
  OPERATION operation[MAX_OPERATION];
} TRANSACTION;

typedef struct _JOB {
  TRANSACTION tx[MAX_TRANSACTION];
} JOB;

void print_result(struct timeval begin, struct timeval end, int nthread);
void task(int dataid);
void giant_lock(int dataid);
void giant_unlock(int dataid);
