#include "threadtools.h"

// Please complete this three functions. You may refer to the macro function defined in "threadtools.h"

// Mountain Climbing
// You are required to solve this function via iterative method, instead of recursion.
void MountainClimbing(int thread_id, int number){
	/* Please fill this code section. */
	ThreadInit(thread_id, number);
	Current->x = 1;	  //x = ways to climb to height of z-1
	Current->y = 1;   //y = ways to climb to height of z
	Current->z = 1;        
	if(setjmp(Current->Environment) == 0)
		longjmp(MAIN, 1);
	for(Current->i = 0; Current->z != Current->N; Current->i ++){
		
		sleep(1);
		/* function work */
		if(Current->N <= 1){
			printf("Mountain Climbing: 1\n");
		}
		int k = Current->x + Current->y;
		Current->x = Current->y;
		Current->y = k;
		Current->z ++;
		printf("Mountain Climbing: %d\n", Current->y);
		
		if(setjmp(Current->Environment) == 0)
			ThreadYield();
	} 
	ThreadExit();

}

// Reduce Integer
// You are required to solve this function via iterative method, instead of recursion.
void ReduceInteger(int thread_id, int number){
	/* Please fill this code section. */
	ThreadInit(thread_id, number);
	
	if(setjmp(Current->Environment) == 0)
		longjmp(MAIN, 1);
	for(Current->i = 1; Current->N != 1; Current->i ++){
		sleep(1);
		/* function work */
		if(Current->N % 2 == 0)
			Current->N /= 2;
		else if(Current->N == 3 || Current->N & 3 == 1)
			Current->N --;
		else	
			Current->N ++;

		printf("Reduce Integer: %d\n", Current->i);
		if(setjmp(Current->Environment) == 0)
			ThreadYield();
	} 
	ThreadExit();
}

// Operation Count
// You are required to solve this function via iterative method, instead of recursion.
void OperationCount(int thread_id, int number){
	/* Please fill this code section. */
	ThreadInit(thread_id, number);
	if(Current->N % 2 == 1)
		Current->x = 0;
	else	
		Current->x = -1;
	Current->y = 0;

	if(setjmp(Current->Environment) == 0)
		longjmp(MAIN, 1);
	
	for(Current->i = 0; Current->i < Current->N / 2; Current->i ++){
		sleep(1);
		/* function work */
		Current->x += 2;
		Current->y += Current->x;
		printf("Operation Count: %d\n", Current->y);
		if(setjmp(Current->Environment) == 0)
			ThreadYield();
	} 
	ThreadExit();
}
