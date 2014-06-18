#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <frosttools/threads.hpp>
#include <frosttools/time.hpp>

#define NUM_THREADS  3
#define TCOUNT 10
#define COUNT_LIMIT 12

int count = 0;
int thread_ids[3] = { 0, 1, 2 };

frosttools::threading::Mutex count_mutex;
frosttools::threading::ConditionVariable count_threshold_cv;

void *inc_count(void *t)
{
	int i;
	long my_id = (long) t;

	for (i = 0; i < TCOUNT; i++)
	{
		count_mutex.lock(); //pthread_mutex_lock(&count_mutex);

		count++;

		/*
		 Check the value of count and signal waiting thread when condition is
		 reached.  Note that this occurs while mutex is locked.
		 */
		if (count == COUNT_LIMIT)
		{
			count_threshold_cv.notifyOne();//pthread_cond_signal(&count_threshold_cv);
			printf("inc_count(): thread %ld, count = %d  Threshold reached.\n",
					my_id, count);
		}
		printf("inc_count(): thread %ld, count = %d, unlocking mutex\n", my_id, count);
		count_mutex.unlock();//pthread_mutex_unlock(&count_mutex);

		/* Do some "work" so threads can alternate on mutex lock */
		sleep(1);
	}
	pthread_exit(NULL);
}

void *watch_count(void *t)
{
	long my_id = (long) t;

	printf("Starting watch_count(): thread %ld\n", my_id);

	/*
	 Lock mutex and wait for signal.  Note that the pthread_cond_wait
	 routine will automatically and atomically unlock mutex while it waits.
	 Also, note that if COUNT_LIMIT is reached before this routine is run by
	 the waiting thread, the loop will be skipped to prevent pthread_cond_wait
	 from never returning.
	 */
	count_mutex.lock();		//pthread_mutex_lock(&count_mutex);
	while (count < COUNT_LIMIT)
	{
		count_threshold_cv.wait(count_mutex);//pthread_cond_wait(&count_threshold_cv, &count_mutex);
		printf("watch_count(): thread %ld Condition signal received.\n", my_id);
		count += 125;
		printf("watch_count(): thread %ld count now = %d.\n", my_id, count);
	}
	count_mutex.unlock();	//pthread_mutex_unlock(&count_mutex);
	pthread_exit(NULL);
}

int native_test(int argc, char *argv[])
{
	int i, rc;
	long t1 = 1, t2 = 2, t3 = 3;
	pthread_t threads[3];
	pthread_attr_t attr;

	/* Initialize mutex and condition variable objects */
	//pthread_mutex_init(&count_mutex, NULL);
	//pthread_cond_init(&count_threshold_cv, NULL);

	/* For portability, explicitly create threads in a joinable state */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_create(&threads[0], &attr, watch_count, (void *) t1);
	pthread_create(&threads[1], &attr, inc_count, (void *) t2);
	pthread_create(&threads[2], &attr, inc_count, (void *) t3);

	/* Wait for all threads to complete */
	for (i = 0; i < NUM_THREADS; i++)
	{
		pthread_join(threads[i], NULL);
	}
	printf("Main(): Waited on %d  threads. Done.\n", NUM_THREADS);

	/* Clean up and exit */
	pthread_attr_destroy(&attr);
	//pthread_mutex_destroy(&count_mutex);
	//pthread_cond_destroy(&count_threshold_cv);
	pthread_exit(NULL);
}

namespace TimingTest
{

frosttools::threading::Mutex waiter_mutex;
frosttools::threading::ConditionVariable waiter_event;

enum WaiterCmd
{
	CmdExit,
	CmdEmpty,
	CmdMessage,
};

WaiterCmd waiter_cmd = CmdEmpty;
char waiter_message[255];

void * test_waiter(void * )
{
	printf("Starting waiter thread\n");


	int timeout = 500;


	while (true)
	{
		frosttools::Timer timer;
		char buffer[255];

		timer.start();
		waiter_mutex.lock();		//pthread_mutex_lock(&count_mutex);
		int result = waiter_event.waitFor(waiter_mutex, timeout);//pthread_cond_wait(&count_threshold_cv, &count_mutex);
		WaiterCmd cmd = waiter_cmd;

		strcpy(buffer, waiter_message);
		waiter_mutex.unlock();	//pthread_mutex_unlock(&count_mutex);
		int timeDelta = timer.currentTimeMS();
		timer.stop();

		if(result == frosttools::threading::cvError)
		{
			printf("waitFor failed\n");
		}
		else if(result == frosttools::threading::cvTimeout)
		{
			printf("waitFor timeout after %dms\n", timeDelta);
		}
		else
		{
			if(cmd == CmdExit)
			{
				printf("Got shutdown command. Exiting\n");
				break;
			}
			else if(cmd == CmdEmpty)
				continue;
			else if(cmd == CmdMessage)
			{
				printf("Waiter got message \"%s\" after %dms\n", buffer, timeDelta);
			}
			waiter_cmd = CmdEmpty;
		}
	}


	pthread_exit(NULL);
}

void sendCommand(WaiterCmd cmd, const char * message)
{
	waiter_mutex.lock();
	waiter_cmd = cmd;
	strcpy(waiter_message, message);
	waiter_event.notifyOne();
	waiter_mutex.unlock();
}

int timer_check()
{
	long t1 = 1, t2 = 2, t3 = 3;
	pthread_t thread;
	pthread_attr_t attr;

	/* For portability, explicitly create threads in a joinable state */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_create(&thread, &attr, test_waiter, (void *) t1);

	frosttools::threading::sleep(600);
	sendCommand(CmdMessage, "Hello 1");
	frosttools::threading::sleep(1700);
	sendCommand(CmdMessage, "Hello 2");
	frosttools::threading::sleep(1300);
	sendCommand(CmdExit, "Hello 3");

	pthread_join(thread, NULL);
	return 0;
}

}



int main(int argc, char *argv[])
{
	TimingTest::timer_check();
	return 0;
}
