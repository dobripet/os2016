#include "shell.h"
#include "rtl.h"
#include "parser.hpp"
#include <iostream>

size_t __stdcall shell(const CONTEXT &regs) {

	/*
	THandle12x stdin = Create_File("CONOUT$", FILE_SHARE_WRITE);	//nahradte systemovym resenim, zatim viz Console u CreateFile na MSDN
	const char* hello = "Hello world!\n";
	size_t written;
	Write_File(stdin, hello, strlen(hello), written);
	Close_File(stdin);
	*/
	
	
	std::cout << std::endl << "Ukazka parsovani" << std::endl;
	Parser p;
	std::string pes = "type \"file.txt|\" > \"jinejfile/o k.txt\" nikdy nechci | dir bla bla  \"c://ppxx\"c://pp < soubor.txt|  wc /lv/a\"aaa|todle\" > xoxo.txt < pesss.txt /x";
	std::cout << "Prikaz: " << pes << std::endl;
	std::vector<Command_params> commands;
	if (!p.parse_commands(pes, &commands)) {
		std::cout << p.get_error_message() << std::endl << std::endl;;
	}
	else {
		std::cout << "OK" << std::endl;
		for (Command_params paramz : commands) {
			std::cout << "command: " << paramz.com;
			std::cout << std::endl << "params: ";
			for (std::string ss : paramz.params) {
				std::cout << ss << ", ";
			}
			std::cout << std::endl << "switches: " << paramz.switches << std::endl;
			std::cout << "in: " << paramz.stdinpath << std::endl;
			std::cout << "out: " << paramz.stdoutpath << std::endl << std::endl;
		}
	}
	

	return 0;
}