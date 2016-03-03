//mpi版本蒙特卡洛求pi
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>
#include <sys/time.h>


int main(int argc, char* argv[])
{
	int my_rank;
	int p;

	unsigned long number_of_tosses = strtol(argv[1], NULL, 10);//每个进程都读入自己的总投掷数
	unsigned long number_in_circle = 0;
	unsigned long number_in_circles = 0;

	long turn = 0;
	long TURNS = strtol(argv[2], NULL, 10);//读入参数2，即分的段数

	int *Sendbuffer;
	int *Recvbuffer;
	
	FILE* result;
	FILE* log_detailed;
	FILE* log_overall;

	double x, y;
	double pi;

	long random_each;
	int error = 0;//错误标志，用来广播

	struct timeval time1;
	struct timeval time2;
	struct timeval time3;
	struct timeval time4;
	struct timeval time5;
	struct timeval time6;
	double time = 0.0;
	double time_rand = 0.0;
	double time_scatter = 0.0;
	double time_calculate = 0.0;
	
	//用于实际测试各语句墙上时钟
	//printf("beforeInit\n");
	MPI_Init(&argc, &argv);
	//printf("afterInit\n");
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	//printf("afterRank\n");
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	//printf("afterSize\n");

	random_each = number_of_tosses / TURNS / p * 2;//计算出每段需要产生的随机数大小
	//printf("%ld\n", TURNS);

	Recvbuffer = (int*)malloc(sizeof(int) * random_each);//计算进程分配的空间

	if (my_rank == 0)
	{
		result = fopen("result.txt", "a+");
		log_overall = fopen("log_overall.txt", "a+");

		gettimeofday(&time1, NULL);
		//printf("time1\n");
		Sendbuffer = (int*)malloc(sizeof(int) * random_each * p);//需要多少，分配多少

		if(Sendbuffer == NULL || Recvbuffer == NULL)//这个必须要有错误检查了，很有可能不够分配
		{
			fprintf(stderr, "no enouph space, please increase para two\n");
			error = 1;
			MPI_Bcast(&error, 1, MPI_INT, 0, MPI_COMM_WORLD);//广播出错信息
			exit(1);
		}

		//若分配成功，则进入随机数生成阶段
		srandom(clock());//这次从头到尾就只用一个随机数，从里面取一段一段。这也是mpi的优势吧

		char name[50];

		while(turn < TURNS){
			//每段都是一个循环
			sprintf(name, "log_turn_%ld.txt", turn);
			log_detailed = fopen(name, "w+");

			gettimeofday(&time6, NULL);
			fprintf(log_detailed, "before_Random\n");

			//从单个种子随机数列中取数
			for(long i=0; i<random_each * p; i++)
				Sendbuffer[i] = random();

			gettimeofday(&time3, NULL);//3-1,生成随机数的时间
			fprintf(log_detailed, "Random_done\n");

			//进行scatter
			//MPI_Scatter(Sendbuffer, random_each, MPI_INT, Recvbuffer, random_each, MPI_INT, 0, MPI_COMM_WORLD);
			for(int i=p-1;i>=0;i--)//倒着传输，减少别的进程的等待时间
			{
				if (i == 0)//给予自身
					for(int j=0; j<random_each;j++)
						Recvbuffer[j] = Sendbuffer[j];
				else
					MPI_Send(&Sendbuffer[i*random_each], random_each, MPI_INT, i, i, MPI_COMM_WORLD);
			}

			fprintf(log_detailed, "Scatter_done\n");
			gettimeofday(&time4, NULL);

			for(long i=0; i<random_each; i+=2)
			{
				x = (Recvbuffer[i] / (double) RAND_MAX)*2-1;
				y = (Recvbuffer[i+1]/ (double) RAND_MAX)*2-1;
				if(x*x+y*y <= 1.0)
					number_in_circle++;
			}
			gettimeofday(&time5, NULL);
			fprintf(log_detailed, "Calculate_done\n");

			fprintf(log_overall, "the current turn %ld has be done at %lf\n",turn, ((double)time5.tv_sec+(double)time5.tv_usec/1000000));	
	
			fclose(log_detailed);
			turn++;
		}
		MPI_Reduce(&number_in_circle, &number_in_circles, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

		pi = (double)number_in_circles / (double)number_of_tosses * 4.0;

		gettimeofday(&time2, NULL);
		//printf("time2\n");

		time = ((double)(time2.tv_sec*1000000+time2.tv_usec) - (double)(time1.tv_sec*1000000+time1.tv_usec)) / 1000;
		time_rand = ((double)(time3.tv_sec*1000000+time3.tv_usec) - (double)(time6.tv_sec*1000000+time6.tv_usec)) / 1000;
		time_scatter = ((double)(time4.tv_sec*1000000+time4.tv_usec) - (double)(time3.tv_sec*1000000+time3.tv_usec)) / 1000;
		time_calculate = ((double)(time5.tv_sec*1000000+time5.tv_usec) - (double)(time4.tv_sec*1000000+time4.tv_usec)) / 1000;

		fprintf(result, "***the test for send nodes:%d, scale:%lu, div:%ld***\n", p, number_of_tosses, TURNS);
		fprintf(result, "the time interval is: %.2f ms\n", time);
		fprintf(result, "the random generation time is: %.2f ms\n", time_rand * TURNS);
		fprintf(result, "the scatter time is: %.2f ms\n", time_scatter * TURNS);
		fprintf(result, "the calculate time is: %.2f ms\n", time_calculate *TURNS);

		fprintf(result,"the assumption of pi is:%.14lf\n\n", pi);

	fclose(result);
	fclose(log_overall);

	}

	else
	{
		Sendbuffer = NULL;//必须置NULL！
		
		//MPI_Bcast(&error, 1, MPI_INT, 0, MPI_COMM_WORLD);//接收出错信息，若没有，程序正常运行。
		//这里假设既然计算节点内存消耗还小于随机数发生节点，若发生节点都不出问题，自然这个也不出问题
		//printf("rank other received\n");
		//if(error == 1)
			//exit(1)

		//if(buffer == NULL)
		//{
		//	error = 1;
		//	MPI_Send(&error, 1, MPI_INT, 0, my_rank, MPI_COMM_WORLD);//错误校验模块，最后完善
		//}

		while(turn < TURNS)
		{
			//接收相关随机数，不知道重名是不是可以哈
			//MPI_Scatter(Sendbuffer, random_each, MPI_INT, Recvbuffer, random_each, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Recv(Recvbuffer, random_each, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			//计算核心逻辑
			for(long i=0; i<random_each; i+=2)
			{
				x = (Recvbuffer[i] / (double) RAND_MAX)*2-1;
				y = (Recvbuffer[i+1]/ (double) RAND_MAX)*2-1;
				if(x*x+y*y <= 1.0)
					number_in_circle++;
			}
			turn++;
		}
		MPI_Reduce(&number_in_circle, &number_in_circles, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
			//printf("%ld\n",number_in_circle);
	}

	free(Sendbuffer);
	free(Recvbuffer);

	MPI_Finalize();

	return 0;
}


