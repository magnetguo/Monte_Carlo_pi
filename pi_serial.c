#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

int main(int argc, char *argv[]) {

	unsigned long number_of_tosses = strtol(argv[1], NULL, 10);
	static unsigned long number_in_circles;

	unsigned turn = 0;
	unsigned TURNS = strtol(argv[2], NULL, 10);
	unsigned long random_each_turn = number_of_tosses / TURNS  * 2;

	int *buffer;

	double x, y;
	double pi;
	
	struct timeval time_rand_start;
	struct timeval time_rand_end;
	struct timeval time_calc_start;
	struct timeval time_calc_end;

	double time = 0.0;
	double time_rand = 0.0;
	double time_calculate = 0.0;

	FILE* result;

	buffer = (int*)malloc(sizeof(int) * random_each_turn);
	
	srandom(clock());
	result = fopen("result.txt", "a+");

	while(turn <TURNS) {
		gettimeofday(&time_rand_start, NULL);

		for(long i=0; i<random_each_turn; i++) {
			buffer[i] = random();
		}

		gettimeofday(&time_rand_end, NULL);
		gettimeofday(&time_calc_start, NULL);

		for(long i=0; i<random_each_turn; i+=2) {
			x = (buffer[i] / (double) RAND_MAX)*2-1;
			y = (buffer[i+1]/ (double) RAND_MAX)*2-1;
			if(x*x+y*y <= 1.0)
				number_in_circles++;
		}

		gettimeofday(&time_calc_end, NULL);

		time_rand += ((double)(time_rand_end.tv_sec*1000000+time_rand_end.tv_usec) - 
			(double)(time_rand_start.tv_sec*1000000+time_rand_start.tv_usec)) / 1000;
        time_calculate += ((double)(time_calc_end.tv_sec*1000000+time_calc_end.tv_usec) - 
        	(double)(time_calc_start.tv_sec*1000000+time_calc_start.tv_usec)) / 1000;
        time = time_rand + time_calculate;
        turn++;
	}

	pi = (double)number_in_circles / (double)number_of_tosses * 4.0;
	fprintf(result, "the time interval is: %.2f ms\n", time);
	fprintf(result, "the random generation time is: %.2f ms\n", time_rand);
	fprintf(result, "the calculate time is: %.2f ms\n", time_calculate);
	fprintf(result,"the assumption of pi is:%.14lf\n\n", pi);

}
