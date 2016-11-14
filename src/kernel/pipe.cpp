#include "pipe.h"
#include <iostream>

pipe::pipe()
{
	InitializeConditionVariable(&buffer_full);
	InitializeConditionVariable(&buffer_empty);
	InitializeCriticalSection(&crit_sec);
	closed_in = false;
	closed_out = false;
}

bool pipe::write(char * s, size_t len, size_t * written)
{
	*written = 0;
	for (int i = 0; i < len; i++) {
		if (!write(s[i])) {
			*written = i;
			return false;
		}
	}
	*written = len;
	return true;
}

bool pipe::write(char c)
{
	if (closed_out) {
		return false;
	}
	while (queue.size() >= MAX_SIZE && !closed_out) {
		SleepConditionVariableCS(&buffer_full, &crit_sec, INFINITE);
	} 
	if (closed_out) {
		return false;
	}
	EnterCriticalSection(&crit_sec);
	queue.push(c);
	LeaveCriticalSection(&crit_sec);
	WakeConditionVariable(&buffer_empty);
	return true;
}

bool pipe::read(size_t count, char *str, size_t *r)
{
	int pos = 0;
	while (pos < count) {
		
		char c = read();
		str[pos++] = c;          
		if (c == EOF) { 
			break;
		}
	}
	*r = pos;
	return true;
}

bool pipe::peek(size_t * available)
{
	return ((*available = queue.size()) != 0);
}

char pipe::read()
{
	while (queue.size() < 1 && !closed_in) {
		SleepConditionVariableCS(&buffer_empty, &crit_sec, INFINITE);
	}
	if (closed_in && queue.size() < 1) {
		return EOF;
	}
	EnterCriticalSection(&crit_sec);
	char ret = queue.front();
	queue.pop();
	LeaveCriticalSection(&crit_sec);
	WakeConditionVariable(&buffer_full);
	return ret;
}

void pipe::close_read()
{
	closed_out = true;
	WakeConditionVariable(&buffer_full);
}

void pipe::close_write()
{
	closed_in = true;
	WakeConditionVariable(&buffer_empty);
}

pipe::~pipe()
{
	DeleteCriticalSection(&crit_sec);	
	WakeAllConditionVariable(&buffer_empty);
	WakeAllConditionVariable(&buffer_full);
}
