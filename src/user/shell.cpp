#include "shell.h"
#include "rtl.h"
#include "parser.hpp"
#include "c_wc.h"
#include <iostream>
#include <thread>
#include <chrono>

#pragma warning(disable: 4996)

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
	std::string pes = "wc parametr1 parametr2  | wc parametr3 | wc p1 p2 p3 /n | wc a b c d /jp";
	std::cout << "Prikaz: " << pes << std::endl;
	std::vector<Parsed_command_params> commands;
	if (!p.parse_commands(pes, &commands)) {
		std::cout << p.get_error_message() << std::endl << std::endl;;
	}
	else {
		std::cout << "Parsing OK." << std::endl;
		//for (Parsed_command_params paramz : commands) {
		for (size_t i = 0, lastCommand = commands.size() - 1; i < commands.size(); i++) {
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
				//	std::cout << "\nPoustim z shellu proces: " << paramz.com << ", size=" << paramz.params.size() << std::endl;
	
			/*
			
			TODO

			1/ Zahrnout:
				- paramz.stdinpath
				- paramz.stdoutpath
				- paramz.switches

			*/

			command_params par;
			par.name = paramz.com.c_str();
			par.STDIN = GetStdHandle(STD_INPUT_HANDLE); //nahradit prislusnym souborem, rourou, nebo stdinem z konzole
			par.STDOUT = GetStdHandle(STD_OUTPUT_HANDLE); //nahradit prislusnym souborem, rourou, nebo stdoutem do konzole
			par.STDERR = GetStdHandle(STD_ERROR_HANDLE);
			par.argc = static_cast<int>(paramz.params.size());
			par.argv = new char*[par.argc];
			for (int i = 0; i < par.argc; i++) {
				par.argv[i] = new char[paramz.params[i].length() + 1];
				paramz.params[i].copy(par.argv[i], paramz.params[i].length(), 0);
				par.argv[i][paramz.params[i].length()] = '\0';
			}
			if (paramz.hasswitches) {
				par.switches = new char[paramz.switches.length() + 1];
				paramz.switches.copy(par.switches, paramz.switches.length(), 0);
				par.switches[paramz.switches.length()] = '\0';
			}
			else {
				par.switches = new char[1];
				par.switches[0] = '\0';
			}
			
			//par.current_node = ..	
			par.waitForProcess = (i == lastCommand); //budeme cekat pouze na posledni proces
			Create_Process(&par);
		}

	}
	//std::this_thread::sleep_for(std::chrono::seconds(1));

//	TEntryPoint t = (TEntryPoint)wc;
//	Create_Process(&t, 1, nullptr, "WC","tady","rootik", GetStdHandle(STD_INPUT_HANDLE), GetStdHandle(STD_OUTPUT_HANDLE), GetStdHandle(STD_ERROR_HANDLE));
	//Create_Process(&wc, )

	return 0;
}