#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main() {
	int times = 50;
	while(times--) {
		time_t t = time(NULL);
	    struct tm * local;
		local = localtime(&t);
		printf("Those output come from child, %s \n", asctime(local));
		sleep(1);
	}
}
