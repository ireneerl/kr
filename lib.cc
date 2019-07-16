#include "concurrency.h"

extern _DATA data[];
void
giant_lock(int id)
{
	if (pthread_mutex_lock(&data[id].mutex)) ERR;
}

void
giant_unlock(int id)
{
	if (pthread_mutex_unlock(&data[id].mutex)) ERR;
}

void
print_result(struct timeval begin, struct timeval end, int nthread)
{
  long usec;
  double sec;

  usec = (end.tv_sec - begin.tv_sec) * 1000 * 1000 + (end.tv_usec - begin.tv_usec);
  sec = (double)usec / 1000.0 / 1000.0;
  printf("Throughput: %f (trans/sec)\n", (double)nthread * MAX_TRANSACTION / sec);
}

/* Read Modify Write */
void
job(int dataid)
{
	int new_val;

	new_val = data[dataid].val; // read
  data[dataid].val = new_val + 10;
}
