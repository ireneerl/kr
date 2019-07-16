#include "concurrency.h"
#include <list>

typedef struct _SET_RW {
  int dataid;
  int val;
} RW_SET;

typedef struct _FINISHED_READING {
  int tx_id;
  RW_SET writeset[MAX_OPERATION];
  int writenumber;
} FINISHED_READING;

DATA data[MAX_OBJ];
JOB *job;
pthread_t *Thread;
pthread_mutex_t GiantLock;

int *mark;
int Tx_id = 0;
int Nthread = 0;

list<FINISHED_READING> finished_reading_x;
bool
validation(const int readnumber, const RW_SET read_set[], const int tx_begin, const int tx_end)
{
  list<FINISHED_READING>::iterator itr;
  // first condition -> time overlaps
  for (itr = finished_reading_x.begin(); itr != finished_reading_x.end(); itr++) {
    if (itr->tx_id == tx_begin) {
      itr++;
      break;
    }
  }
  // second checking -> conflict
  for (;itr != finished_reading_x.end() && itr->tx_id <= tx_end; itr++) {
    for (int wid = 0; wid < itr->writenumber; wid++) {
      for (int rid = 0; rid < readnumber; rid++) {
        if (itr->writeset[wid].dataid == read_set[rid].dataid) {
          return false;
        }
      }
    }
  }
  return true;
}

void
transaction(int wid, int tid)
{
  RW_SET read_set[MAX_OPERATION];
  RW_SET write_set[MAX_OPERATION];
  OPERATION current[MAX_OPERATION];
  memcpy(current, job[wid].tx[tid].operation, sizeof(OPERATION) * MAX_OPERATION);

TRYAGAIN:
  int tx_begin = Tx_id;
  int readnumber = 0, writenumber = 0;
  bzero(read_set, MAX_OPERATION * sizeof(RW_SET));
  bzero(write_set, MAX_OPERATION * sizeof(RW_SET));
  for (int operationid = 0; operationid < MAX_OPERATION; operationid++) {
    if (current[operationid].rw == 0) {
      int dataid = current[operationid].dataid;
      read_set[readnumber].dataid = dataid;
      read_set[readnumber].val = data[dataid].val;
      readnumber++;
    }
  }
  for (int operationid = 0; operationid < MAX_OPERATION; operationid++) {
    if (current[operationid].rw == 1) {
      int dataid = current[operationid].dataid;
      write_set[writenumber].dataid = dataid;
      write_set[writenumber].val = 0; // No meaning
      writenumber++;
    }
  }
  if (pthread_mutex_lock(&GiantLock) != 0) ERR;
  int tx_end = Tx_id;
  if (Tx_id != 0) {
    bool valid = validation(readnumber, read_set, tx_begin, tx_end);
    if (valid == false){
      if (!pthread_mutex_unlock(&GiantLock)) ERR;
      goto TRYAGAIN;
    }
  }
  for (int i = 0; i < writenumber; i++){
    int dataid = write_set[i].dataid;
    int val = write_set[i].val;
    data[dataid].val = val;
  }
  if (pthread_mutex_unlock(&GiantLock) != 0) ERR;
}
static void *
worker(void *arg)
{
  int mainid = *(int *)arg; free(arg);
  for (int i = 0; i < MAX_TRANSACTION; i++) {
    transaction(mainid, i);
  }
  return NULL;
}
int
main(int argc, char *argv[])
{
  int i;

  struct timeval begin, end;
  if (argc == 2)
    Nthread = atoi(argv[1]);
  else
    Nthread = 4;
  Thread = (pthread_t *)calloc(Nthread, sizeof(pthread_t));
  	if (!Thread) ERR;
  pthread_mutex_init(&GiantLock, NULL);
  srand((unsigned int)time(0));

  job = (JOB *)calloc(Nthread, sizeof(JOB)); if (!job) ERR;
  for (int i = 0; i < Nthread; i++) {
    for (int j = 0; j < MAX_TRANSACTION; j++) {
      for (int k = 0; k < MAX_OPERATION; k++) {
        int rw_val = rand() % 2;
        if (rw_val == 0) rw_val = 0; else rw_val = 1;
        job[i].tx[j].operation[k].rw = rw_val;
        job[i].tx[j].operation[k].dataid = rand() % MAX_OBJ;
      }
    }
  }

  gettimeofday(&begin, NULL);
  for (i = 0; i < Nthread; i++) {
    int *mainid = (int *)calloc(1, sizeof(int)); if (!mainid) ERR;
    *mainid = i;
    int ret = pthread_create(&Thread[i], NULL, worker, mainid);
		if (ret < 0) ERR;
	}
  for (i = 0; i < Nthread; i++) {
		int ret = pthread_join(Thread[i], NULL);
		if (ret < 0) ERR;
	}
  gettimeofday(&end, NULL);
  print_result(begin, end, Nthread);
  return 0;
}
