#include "shell.h"
#include "rtl.h"
#include "parser.hpp"
#include "c_wc.h"
#include <iostream>
#include <thread>
#include <chrono>

/*
int wc(int argc, char * argv[]) {
	std::cout << "bezi program wc s parametry: " << argc << "\n";
	return 0;
}*/

size_t __stdcall shell(const CONTEXT &regs) {

	/*
	THandle stdin = Create_File("CONOUT$", FILE_SHARE_WRITE);	//nahradte systemovym resenim, zatim viz Console u CreateFile na MSDN
	const char* hello = "Hello world!\n";
	size_t written;
	Write_File(stdin, hello, strlen(hello), written);
	Close_File(stdin);
	*/


	std::cout << std::endl << "Ukazka parsovani" << std::endl;
	Parser p;
	//std::string pes = "type \"file.txt|\" > \"jinejfile/o k.txt\" nikdy nechci | dir bla bla  \"c://ppxx\"c://pp < soubor.txt|  wc /lv/a\"aaa|todle\" > xoxo.txt < pesss.txt /x";
	std::string pes = "wc parametr1 parametr2  | wc parametr3 | wc p1 p2 p3 | wc a b c d";
	std::cout << "Prikaz: " << pes << std::endl;
	std::vector<Parsed_command_params> commands;
	if (!p.parse_commands(pes, &commands)) {
		std::cout << p.get_error_message() << std::endl << std::endl;;
	}
	else {
		std::cout << "Parsing OK." << std::endl;
		//for (Parsed_command_params paramz : commands) {
		for (int i = 0, lastCommand = commands.size() - 1; i < commands.size(); i++) {
			Parsed_command_params paramz = commands[i];

			/*std::cout << "command: " << paramz.com;
			std::cout << std::endl << "params: ";
			for (std::string ss : paramz.params) {
				std::cout << ss << ", ";
			}
			std::cout << std::endl << "switches: " << paramz.switches << std::endl;
			std::cout << "in: " << paramz.stdinpath << std::endl;
			std::cout << "out: " << paramz.stdoutpath << std::endl << std::endl;*/

			/*	TEntryPoint t;
				if (paramz.com == "wc") {
					t = (TEntryPoint)wc;
				}*/
				//else if echo
				//else if dir
				//atd....
			/*	else {
					//nejakej error - ale sem by to nikdy nemelo spadnout, protoze to kontroluje uz parser
				}
				*/
			std::cout << "\nPoustim z shellu proces: " << paramz.com << ", size=" << paramz.params.size() << std::endl;
			command_params par;

			par.STDIN = GetStdHandle(STD_INPUT_HANDLE);
			par.STDOUT = GetStdHandle(STD_OUTPUT_HANDLE);
			par.STDERR = GetStdHandle(STD_ERROR_HANDLE);
			par.params = paramz.params;
			par.name = paramz.com;
			//par.current_node = ..	
			par.wait = (i == lastCommand);
			

			Create_Process(/*&t, */&par);

		}

	}
	//std::this_thread::sleep_for(std::chrono::seconds(1));

//	TEntryPoint t = (TEntryPoint)wc;
//	Create_Process(&t, 1, nullptr, "WC","tady","rootik", GetStdHandle(STD_INPUT_HANDLE), GetStdHandle(STD_OUTPUT_HANDLE), GetStdHandle(STD_ERROR_HANDLE));
	//Create_Process(&wc, )

	return 0;
}