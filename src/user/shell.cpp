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
	FDHandle CURRENT_DIR = (FDHandle)regs.R11;
	std::string *path = (std::string *) regs.R13;
	
	Parser parser;
	char * buf_command = new char[1001];
	buf_command[1000] = '\0';
	bool run = true;
	
	while (run) {

		std::string shell_ = "\n\n" + *path + ">";
		char * shell__ = (char *)shell_.c_str();
		Write_File(STDOUT, shell__, strlen(shell__));

		size_t filled;
		Read_File(STDIN, 1000, buf_command, &filled);
		buf_command[filled] = '\0';
		if (buf_command[filled - 1] == EOF) { //goodbye
			break;
		}
		
		std::vector<Parsed_command_params> commands_parsed;
		if (!parser.parse_commands(std::string(buf_command), &commands_parsed)) {
			std::cout << parser.get_error_message() << std::endl;;
		}
		else {
			
			if (commands_parsed.size() == 0) {
				continue;
			}
			
			std::vector<FDHandle> pipeWrite;
			std::vector<FDHandle> pipeRead;

			//vyrobeni tolika rour, kolik bude (pocet - 1) procesu
			for (size_t i = 0; i < commands_parsed.size() - 1; i++) {
				FDHandle write, read;
				Open_Pipe(&write, &read);
				pipeWrite.push_back(write);
				pipeRead.push_back(read);
			}

			std::vector<int> process_handles;

			for (size_t i = 0, lastCommand = commands_parsed.size() - 1; i < commands_parsed.size(); i++) {
				Parsed_command_params &current_params = commands_parsed[i];

				if (current_params.com == "exit") {
					if (i != 0) {
						//na jiz spustene procesy se ceka na konci cyklu shellu
						char * msg = "Waiting for processes to finish before exit.\n\0";
						Write_File(STDOUT, msg, strlen(msg));
					}		
					//zavreme vsechny nasledujici roury (uz nebudou potreba, kdyz tedkonc exit)
					for (size_t j = i; j <= lastCommand; j++) {
						if (j != 0) {
							Close_File(pipeRead[j - 1]);
						}
						if (j != lastCommand) {
							Close_File(pipeWrite[j]);
						}
					}
					run = false;
					break;
				}
				if (current_params.com == "cd") {
					if (i != 0) {
						Close_File(pipeRead[i - 1]);
					}
					if (i != lastCommand) {
						Close_File(pipeWrite[i]);
					}
					if (current_params.params.size() != 1) {
						Write_File(STDOUT, (char*)(*path).c_str(), (*path).length());
					} else {
						if (!Change_Dir((char*)current_params.params[0].c_str())) {
							switch (Get_Last_Error())
							{
							case ERR_IO_PATH_NOEXIST: {
								char * err = "This path does not exist.\n\0";
								Write_File(STDOUT, err, strlen(err));
								break;
							}
							case ERR_IO_FILE_ISFILE: {
								char * err = "Target is not a directory.\n\0";
								Write_File(STDOUT, err, strlen(err));
								break;
							}
							default: {
								char * err = "Unspecified error occured.\n\0";
								Write_File(STDOUT, err, strlen(err));
								break;
							}
							}//end switch
						}
					}
					continue;
				}
		
				command_params par;
				FDHandle std_in, std_out, std_err, if_pipe_and_stdout = -2;
				par.name = current_params.com.c_str();

				if (current_params.redirectstdin) {

					//neni to prvni prikaz, tj. ma presmerovani ze souboru i z roury
					if (i != 0) {
						//proste zavreme jeho rouru pro cteni - a producent (predchozi
						//proces) by mel poznat, ze roura je pro cteni zavrena a ukoncit se.
						Close_File(pipeRead[i - 1]); //TODO test - bude to fungovat????
					}

					bool fail = !Open_File(&std_in, current_params.stdinpath.c_str(), F_MODE_READ);
					if (fail) {
						std::cout << "DEBUG SHELL: redirecting stdin failed" << std::endl;
						//TODO zavreni rour na obou stranach??
						//nutno resit, protoze se lehko muze stat, ze soubor nepujde otevrit (napr. blb uzivatel presmeruje ze slozky)
						continue;
					}
				}
				else if (i == 0) {
					//prvni prikaz, zdedime (duplikujeme handle) stdin od rodice.
					Open_File(STDIN, &std_in);
				}
				else {
					//nemame presmerovani, ani to neni prvni prikaz - bude se cist z roury.
					std_in = pipeRead[i - 1];
				}

				if (current_params.redirectstdout) {

					//neni to posledni prikaz, tj. ma presmerovani do souboru i do roury
					if (i != lastCommand) {
						
						//Presmerovani do souboru ma prednost, takze to udelame tak, 
						//ze tento proces producenta stejne dostane rouru do seznamu souboru..
						//a normalne ji uzavre jako kazdy jiny soubor, az se bude ukoncovat.
						//if_pipe_and_stdout = pipeWrite[i]; //TODO test
						
						//Asi taky muzu s klidem tu rouru zavrit. Kdyz tedka shell ceka na vsechny procesy, tak je jedno, kdo driv skonci.
						Close_File(pipeWrite[i]); //TODO test
					}

					bool fail = !Open_File(&std_out, current_params.stdoutpath.c_str(), F_MODE_WRITE);
					if (fail) {
						std::cout << "DEBUG SHELL: redirecting stdout failed" << std::endl;
						//TODO zavreni rour na obou stranach??
						//nutno resit, protoze se lehko muze stat, ze soubor nepujde otevrit (napr. blb uzivatel presmeruje ze slozky)
						continue;
					}

				}
				else if (i == lastCommand) {
					//posledni prikaz, zdedime (duplikujeme handle) stdout od rodice.
					Open_File(STDOUT, &std_out);
				}
				else {
					//nemame presmerovani, ani to neni posledni prikaz - bude se psat do roury.
					std_out = pipeWrite[i];
				}

				par.argc = static_cast<int>(current_params.params.size());
				par.argv = new char*[par.argc];
				for (int i = 0; i < par.argc; i++) {
					par.argv[i] = new char[current_params.params[i].length() + 1];
					current_params.params[i].copy(par.argv[i], current_params.params[i].length(), 0);
					par.argv[i][current_params.params[i].length()] = '\0';
				}
				if (current_params.hasswitches) {
					par.switches = new char[current_params.switches.length() + 1];
					current_params.switches.copy(par.switches, current_params.switches.length(), 0);
					par.switches[current_params.switches.length()] = '\0';
				}
				else {
					par.switches = new char[1];
					par.switches[0] = '\0';
				}

				par.handles.push_back(std_in);
				par.handles.push_back(std_out);

				//duplicate stderr for current process
				/*bool ok = */Open_File(STDOUT, &std_err); //navratova hodnota muze bejt fail
				par.handles.push_back(std_err);

				//duplicate current dir for new process
				FDHandle h;
				/*bool ok = */Open_File(CURRENT_DIR, &h); //navratova hodnota muze bejt fail
				par.handles.push_back(h);

				if (if_pipe_and_stdout != -2) {
					par.handles.push_back(if_pipe_and_stdout);
					if_pipe_and_stdout = -2;
				}

				int pid;
				Create_Process(&par, &pid);
				process_handles.push_back(pid);
			}

			for (auto &pid : process_handles) {
				Join_and_Delete_Process(pid);
			}
		}
	}



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
	Open_File(&h, "C://testik.txt", F_MODE_WRITE);
	char* t2 = "Tohle do souboru!\n";
	Write_File(h, t2, strlen(t2));
	Close_File(h);
	Open_File(&h, "C://testik.txt", F_MODE_READ);
	char r[256];
	size_t filled;
	Read_File(h, 50, r, &filled);
	Close_File(h);
	Write_File(STDOUT, r, filled);
	*/

	/*
	char *c[] = {"cesticka"};
	command_params *para =  new command_params;
	para->argc = 1;
	para->argv = c;
	para->name = "echo";
	std::vector <FDHandle> handles;
	FDHandle stdindup;
	Open_File(STDIN, &stdindup);
	FDHandle stdoutdup;
	Open_File(STDOUT, &stdoutdup);
	FDHandle stderrdup;
	Open_File(STDERR, &stderrdup);
	FDHandle cdir;
	Open_File(CURRENT_DIR, &cdir);
	handles.push_back(stdindup);
	handles.push_back(stdoutdup);
	handles.push_back(stderrdup);
	handles.push_back(cdir);
	para->handles = handles;
	std::cout << "volame\n";
	int pidi;
	Create_Process(para, &pidi);
	std::cout << "pidi " << pidi << "\n";
	Join_and_Delete_Process(pidi);
	std::cout << "konec\n";
	delete para;*/
	
	/*
	tady to nejak bude bezet ve while(true) dokud nebude ctrl+z nebo tak neco
	*/
	/*
	//std::cout << std::endl << "Ukazka parsovani" << std::endl;
	Parser p;
	//std::string pes = "type \"file.txt|\" > \"jinejfile/o k.txt\" nikdy nechci | dir bla bla  \"c://ppxx\"c://pp < soubor.txt|  wc /lv/a\"aaa|todle\" > xoxo.txt < pesss.txt /x";
	std::string pes = "wc parametr1 parametr2  | wc parametr3 | wc p1 p2 p3 /n | wc a b c d /jp";
	//pes = "wc";
	//std::cout << "Prikaz: " << pes << std::endl;
	std::vector<Parsed_command_params> commands_parsed;
	if (!p.parse_commands(pes, &commands_parsed)) {
		std::cout << p.get_error_message() << std::endl << std::endl;;
	}
	*/
	//else *to samy jako nahore*

	/*
	Shell pobezi ve while(true) {..}.
	Ukoncen bude asi kdyz dostane EOF? (Ctrl+Z)
	Musime nejak resit, aby kontroloval, jestli se nema ukoncit,
	ale zaroven se nesmi tim ctenim zablokovat (coz se normalne pri cteni deje) - nejakej peek? timeout? nebo co?
	plati i pro dalsi procesy jako je rgen
	*/

	/*
	cd "path"
	Asi bude nutny nejak posilat PID shellu, protoze v tabulce otevrenych souboru se musi zmenit pro shell. 
	*/

	return 0;
}