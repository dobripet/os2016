#pragma once

#include <queue>
#include <Windows.h>

/*

Iplementace pipy podle vzoru producent-konzument.
Producent bude cekat na podmince na vzbuzeni konzumentem, pokud bude chtit zapisovat do plne pipy.
Analogicky plati i pro cteni z pipy konzumentem.
Manipulace s datovou strukturou (zde std::queue) je osetrena kritickou sekci.

Viz https://msdn.microsoft.com/en-us/library/windows/desktop/ms686903(v=vs.85).aspx

*/

typedef struct pipe {

private:

	const int EOF_char = -1;
	const int MAX_SIZE = 2048;
	bool closed_in;
	bool closed_out;
	std::queue<char> queue;
	CRITICAL_SECTION crit_sec;
	CONDITION_VARIABLE buffer_full, buffer_empty;

public:

	pipe();
	bool write(char c);
	char read();
	~pipe();

} pipe;