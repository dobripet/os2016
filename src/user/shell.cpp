#include "shell.h"
#include "rtl.h"
#include "parser.hpp"
#include "c_wc.h"
#include <iostream>
#include <thread>
#include <chrono>

#pragma warning(disable: 4996) //std::string::copy


/* zavre roury procesy na obou stranach*/
void close_pipes(std::vector<FDHandle> in, std::vector<FDHandle> out, size_t i, size_t last) {
	if (i != 0) {
		Close_File(in[i - 1]);
	}
	if (i != last) {
		Close_File(out[i]);
	}
}

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

	//cyklus shellu
	while (run) {

		std::string shell_ = "\n\n" + *path + ">";
		char * shell__ = (char *)shell_.c_str();
		Write_File(STDOUT, shell__, strlen(shell__));

		size_t filled;
		Read_File(STDIN, 1000, buf_command, &filled);
		if (buf_command[filled] == EOF) { //goodbye
			break;
		}
		buf_command[filled] = '\0';

		std::vector<Parsed_command_params> commands_parsed;
		if (!parser.parse_commands(std::string(buf_command), &commands_parsed)) {
			std::cout << parser.get_error_message() << std::endl;;
		}
		else {

			if (commands_parsed.size() == 0) {
				continue;
			}

			//Pred rgen je treba vyrobit dalsi proces na detekci EOF, protoze winapi ReadFile je blokujici (coz v rgenu nechceme),
			//ale vyrobime ho jen kdyz nema jiny vstup (roura nebo soubor - do nich muzeme nahlizet bez blokovani).
			if (commands_parsed[0].com == "rgen" && !commands_parsed[0].redirectstdin) {
				Parsed_command_params paramz = { "dummy", false, false, false, false, "", "", "" };
				commands_parsed.insert(commands_parsed.begin(), paramz);
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

			//spustime vsechny prikazy
			for (size_t i = 0, lastCommand = commands_parsed.size() - 1; i < commands_parsed.size(); i++) {
				Parsed_command_params &current_params = commands_parsed[i];

				/* EXIT built-in command */
				if (current_params.com == "exit") {
					if (i != 0) {
						//na jiz spustene procesy se ceka na konci cyklu shellu
						char * msg = "Waiting for processes to finish before exit.\n\0";
						Write_File(STDOUT, msg, strlen(msg));
					}
					//zavreme vsechny nasledujici roury (uz nebudou potreba, kdyz tedkonc exit)
					for (size_t j = i; j <= lastCommand; j++) {
						close_pipes(pipeRead, pipeWrite, j, lastCommand);
					}
					run = false;
					break;
				}

				/* CD built-in command */
				if (current_params.com == "cd") {
					close_pipes(pipeRead, pipeWrite, i, lastCommand);
				
					if (current_params.params.size() != 1) {
						Write_File(STDOUT, (char*)(*path).c_str(), (*path).length());
					}
					else {
						if (!Change_Dir((char*)current_params.params[0].c_str())) {
							Print_Last_Error(STDERR);
						}
					}
					continue;
				}

				command_params par;
				FDHandle std_in, std_out, std_err, curr_dir;
				par.name = current_params.com.c_str();

				/* STDIN */
				if (current_params.redirectstdin) {
					//neni to prvni prikaz, tj. ma presmerovani ze souboru i z roury
					if (i != 0) {
						//Stdin ze souboru ma prednost pred rourou, takze to udelame tak,
						//ze zavreme vstupni rouru pro cteni - a producent (predchozi
						//proces) by mel poznat, ze roura je pro cteni zavrena.
						Close_File(pipeRead[i - 1]);
					}

					bool fail = !Open_File(&std_in, current_params.stdinpath.c_str(), F_MODE_READ);
					if (fail) {
						Print_Last_Error(STDERR, "Redirecting STDIN failed for path: \"" + (current_params.stdinpath) + "\". ");
						//presmerovani se nepovedlo. Proces se nebude vubec spustet. Zavreme tedy roury na obou stranach.
						close_pipes(pipeRead, pipeWrite, i, lastCommand);
						continue;
					}
				}
				else if (i == 0) {
					//prvni prikaz, zdedime (duplikujeme handle) stdin od rodice.
					Duplicate_File(STDIN, &std_in);
				}
				else {
					//nemame presmerovani, ani to neni prvni prikaz - bude se cist z roury.
					std_in = pipeRead[i - 1];
				}

				/* STDOUT */
				if (current_params.redirectstdout) {
					//neni to posledni prikaz, tj. ma presmerovani do souboru i do roury
					if (i != lastCommand) {
						//Presmerovani do souboru ma prednost pred rourou, takze
						//to udelame tak, ze zavreme vystupni rouru pro zapis.
						//Konzument to pozna a ukonci se, protoze nebude mit co cist.
						Close_File(pipeWrite[i]);
					}

					bool fail = !Open_File(&std_out, current_params.stdoutpath.c_str(), F_MODE_WRITE, !current_params.appendstdout);
					if (fail) {
						Print_Last_Error(STDERR, "Redirecting STDOUT failed for path: \"" + (current_params.stdoutpath) + "\". ");
						//presmerovani se nepovedlo. Proces se nebude vubec spustet. Zavreme tedy roury na obou stranach.
						close_pipes(pipeRead, pipeWrite, i, lastCommand);
						continue;
					}

				}
				else if (i == lastCommand) {
					//posledni prikaz, zdedime (duplikujeme handle) stdout od rodice.
					Duplicate_File(STDOUT, &std_out);
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

				/*STDERR - zduplikujeme od shellu*/
				/*bool ok = */Duplicate_File(STDOUT, &std_err);
				par.handles.push_back(std_err);

				/*CURRENT DIR - zduplikujeme od shellu*/
				/*bool ok = */Duplicate_File(CURRENT_DIR, &curr_dir); 
				par.handles.push_back(curr_dir);

				int pid;
				Create_Process(&par, &pid);
				if (pid < 0) {
					Print_Last_Error(STDERR);
				}
				process_handles.push_back(pid);

			}//konec cyklu spusteni prikazu

			//pockame na vsechny spustene procesy, nez se kontrola vrati shellu
			for (auto &pid : process_handles) {
				Join_and_Delete_Process(pid);
			}
		}
	} //konec cyklu shellu

	delete[] buf_command;
	return(size_t)0;
}