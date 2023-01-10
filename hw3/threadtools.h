#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

extern int timeslice, switchmode;

typedef struct TCB_NODE *TCB_ptr;
typedef struct TCB_NODE{
    jmp_buf  Environment;
    int      Thread_id;
    TCB_ptr  Next;
    TCB_ptr  Prev;
    int i, N;
    int w, x, y, z;
} TCB;

extern jmp_buf MAIN, SCHEDULER;
extern TCB_ptr Head;
extern TCB_ptr Current;
extern TCB_ptr Work;
extern sigset_t base_mask, waiting_mask, tstp_mask, alrm_mask;

void sighandler(int signo);
void scheduler();

// Call function in the argument that is passed in
#define ThreadCreate(function, thread_id, number)                                         \
{                                                                                         \
	/* Please fill this code section. */												  \
    if(setjmp(MAIN) == 0)                          \
        function(thread_id, number);                \
}

// Build up TCB_NODE for each function, insert it into circular linked-list
#define ThreadInit(thread_id, number)                                                     \
{                                                                                         \
	/* Please fill this code section. */												  \
    Current = (TCB_ptr)malloc(sizeof(TCB));                                     \
    Current->Thread_id = thread_id;                        \
    Current->N = number;     \
    if(Head == NULL){            \
        Head = Current;            \
        Work = Head;            \
    }                               \
    Current->Next = Head;       \
    Current->Prev = Work;              \
    Work->Next = Current;                         \
    Work = Current;             \
    Head->Prev = Current;  \
}

// Call this while a thread is terminated
#define ThreadExit()                                                                      \
{                                                                                         \
	/* Please fill this code section. */	    											  \
    if(Current->Prev == Current)            \
        longjmp(MAIN, 1);               \
    Current->Prev->Next = Current->Next;        \
    Current->Next->Prev = Current->Prev;        \
    longjmp(SCHEDULER, 2);                                          \
}

// Decided whether to "context switch" based on the switchmode argument passed in main.c
#define ThreadYield()                                                                     \
{                                                                                         \
	/* Please fill this code section. */												  \
    if(switchmode == 0){                        \
        longjmp(SCHEDULER, 1);                      \
    }                                               \
    else{                                               \
        sigpending(&waiting_mask);                                    \
        if(sigismember(&waiting_mask, SIGTSTP)){    \
            sigsuspend(&alrm_mask);     \
            sigsuspend(&base_mask);         \
            longjmp(SCHEDULER, 1);          \
        }                                   \
        else if(sigismember(&waiting_mask, SIGALRM)){            \
            sigsuspend(&tstp_mask);     \
            sigsuspend(&base_mask);         \
            longjmp(SCHEDULER, 1);      \
        }                   \
        else{               \
                        \
        }                   \
    }                                                   \
}
