#include "shell.h"
#include "rtl.h"
#include "parser.hpp"
#include "c_wc.h"
#include <iostream>
#include <thread>
#include <chrono>

#pragma warning(disable: 4996)

size_t __stdcall shell(const CONTEXT &regs) {

	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;

	/*
	THandle stdin = Create_File("CONOUT$", FILE_SHARE_WRITE);	//nahradte systemovym resenim, zatim viz Console u CreateFile na MSDN
	const char* hello = "Hello world!\n";
	size_t written;
	Write_File(stdin, hello, strlen(hello), written);
	Close_File(stdin);
	*/

	/*
	const char* hello = "Test zapisu: Hello world!\n";
	size_t written;
	Write_File(STDOUT, hello, strlen(hello), written);
	*/
	/*
	char* t = "Test zapisu do souboru!\n";
	Write_File(STDOUT, t, strlen(t));
	FDHandle h;
	Open_File(&h, "C:\testik.txt", F_MODE_WRITE);
	char* t2 = "Tohle do souboru!\n";
	Write_File(h, t2, strlen(t2));
	Close_File(h);
	Open_File(&h, "C:\testik.txt", F_MODE_READ);
	char r[256];
	size_t filled;
	Read_File(h, 50, r, &filled);
	Close_File(h);
	Write_File(STDOUT, r, filled);
	*/

	/*
	tady to nejak bude bezet ve while(true) dokud nebude ctrl+z nebo tak neco
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

		std::vector<FDHandle> pipeWrite;
		std::vector<FDHandle> pipeRead;

		//vyrobeni tolika rour, kolik bude (pocet - 1) procesu
		for (size_t i = 0; i < commands.size() - 1; i++) {
			FDHandle write, read;
			Open_Pipe(&write, &read);
			pipeWrite.push_back(write);
			pipeRead.push_back(read);
		}

		for (size_t i = 0, lastCommand = commands.size() - 1; i < commands.size(); i++) {
			Parsed_command_params paramz = commands[i];

			/*
			TODO

			1/ Zahrnout:
				- paramz.stdinpath
				- paramz.stdoutpath

			2/ Do/ze souboru bude i kdyz bude roura. (presmerovani ma prednost pred rourou)
				- Jak to vyresit s procesem, kterej na roure ceka na svuj vstup, 
				  ale nic nedostane, protoze predchozi proces ma stdout do souboru?
				  - mozna by mohl shell na predchozi proces cekat a dalsimu pak dat pipu se zavrenym vstupem
				(a analogicky se psanim, kdyz dalsi proces ma stdin ze souboru - i kdyz to se mozna resit nemusi -
				proste dam procesu k zapisovani rouru, kterej rovnou zavru vystup, takze to proces pozna a nebude tam uz psat)
			*/

			command_params par;
			par.name = paramz.com.c_str();
			if (paramz.redirectstdin) {
				//par.STDIN = otevrit soubor(paramz.stdinpath);
			}
			else if (i == 0) {
				par.STDIN = 0;
			}
			else {
				par.STDIN = pipeRead[i - 1];
			}

			if (paramz.redirectstdout) {
				//par.STDOUT = otevrit soubor(paramz.stdoutpath);
			}
			else if (i == lastCommand) {
				par.STDOUT = 1;
			}
			else {
				par.STDOUT = pipeWrite[i];
			}

			par.STDERR = STDERR;
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

			//par.current_node = shell current node
			par.waitForProcess = (i == lastCommand); //budeme cekat na posledni proces
			Create_Process(&par);
		}
	}
	return 0;
}