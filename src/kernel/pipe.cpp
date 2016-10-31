#include "pipe.h"

pipe::pipe()
{
	InitializeConditionVariable(&buffer_full);
	InitializeConditionVariable(&buffer_empty);
	InitializeCriticalSection(&crit_sec);
}

//TODO prestat psat, kdyz uz nikdo necte
bool pipe::write(char c)
{
	if (closed_out) {
		return false;
	}
	while (queue.size() >= MAX_SIZE && !closed_out) { //TODO spis by mel bejt vstup do kriticky sekce uz pred timhle (pac tu chci size)
		//TODO wait on condition
		SleepConditionVariableCS(&buffer_full, &crit_sec, INFINITE);
	} 
	if (closed_out) {
		return false;
	}
	//TODO critical section start
	EnterCriticalSection(&crit_sec);
	queue.push(c);
	//TODO critical section end
	LeaveCriticalSection(&crit_sec);
	//TODO wake condition up
	WakeConditionVariable(&buffer_empty);
	return true;
}


//TODO prestat cist, pokud uz nikdo nepise (resit pripady, kdy spim kvuli prazdny fronte, ale pipa uz je zavrena pro zapis)
char pipe::read()
{
	while (queue.size() < 1 && !closed_in) {
		SleepConditionVariableCS(&buffer_empty, &crit_sec, INFINITE);
	}
	if (closed_in) {
		return EOF_char;
	}
	//TODO critical section start
	EnterCriticalSection(&crit_sec);
	char ret = queue.front();
	queue.pop();
	//TODO critical section end
	LeaveCriticalSection(&crit_sec);
	//TODO wake condition up
	WakeConditionVariable(&buffer_full);
	return ret;
}

pipe::~pipe()
{
	DeleteCriticalSection(&crit_sec);	
	WakeAllConditionVariable(&buffer_empty);
	WakeAllConditionVariable(&buffer_full);
}
