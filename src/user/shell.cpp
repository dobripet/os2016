#include "shell.h"
#include "rtl.h"

/* zavre roury procesu na obou stranach*/
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
	bool stdinIsConsole = *((bool*)regs.R14);

	std::string st = "Shell is running.\n";
	char * st_ = (char *)st.c_str();
	Write_File(STDOUT, st_, strlen(st_));

	char * buf_command = new char[1001];
	buf_command[1000] = '\0';
	bool run = true;

	//cyklus shellu
	while (run) {

		std::string shell_ = "\n" + *path + ">";
		char * shell__ = (char *)shell_.c_str();
		Write_File(STDOUT, shell__, strlen(shell__));

		size_t filled;
		Read_File(STDIN, 1000, buf_command, &filled);
		if (filled == 0 && buf_command[filled] == EOF) { //goodbye
			break;
		}
		buf_command[filled] = '\0';

		//split na radky
		std::vector<std::string> linesStr;
		splitByLines(buf_command, &linesStr);

		//cyklus radek
		//for (auto &line : linesStr) {
		for (size_t t = 0; t < linesStr.size(); t++) {
			std::string line = linesStr[t];

			if (t != 0) {
				shell_ = "\n" + *path + ">";
				shell__ = (char *)shell_.c_str();
				Write_File(STDOUT, shell__, strlen(shell__));
			}

			if (!stdinIsConsole) {
				std::string command = line + '\n';
				char * command_ = (char *)command.c_str();
				Write_File(STDOUT, command_, strlen(command_));
			}
			
			std::vector<std::string> commandsStr;
			//nasekame podle rour na jednotlive prikazy
			if (!splitByPipes(line, &commandsStr)) {
				std::string txt = get_error_message() + "\n";
				char * err = (char*)(txt.c_str());
				Write_File(STDOUT, err, strlen(err));
				continue;
			}
			if (commandsStr.size() == 0) {
				continue;
			}

			bool parseErr = false;
			std::vector<parsed_command> commandsParsed;
			//parsovani stringu na strukturu prikazu, ve ktere je jmeno prikazu, presmerovani a zbytek jako jeden velky parametr pro proces
			for (auto &s : commandsStr) {
				parsed_command par;
				if (!parseCommandRedirects(s, &par)) {
					std::string txt = get_error_message() + "\n";
					char * errTxt = (char*)(txt.c_str());
					Write_File(STDOUT, errTxt, strlen(errTxt));
					parseErr = true;
					break;
				}
				commandsParsed.push_back(par);
			}
			if (parseErr) {
				continue;
			}

			//Pred rgen je treba vyrobit dalsi proces na detekci EOF, protoze winapi ReadFile je blokujici (coz v rgenu nechceme),
			//ale vyrobime ho jen kdyz nema jiny vstup (roura nebo soubor - do nich muzeme nahlizet bez blokovani).
			if (commandsParsed[0].com == "rgen" && !commandsParsed[0].redirectstdin) {
				parsed_command paramz = { "dummy", false, false, false, "", "", "" };
				commandsParsed.insert(commandsParsed.begin(), paramz);
			}

			std::vector<FDHandle> pipeWrite;
			std::vector<FDHandle> pipeRead;
			//vyrobeni tolika rour, kolik bude (pocet - 1) procesu
			for (size_t i = 0; i < commandsParsed.size() - 1; i++) {
				FDHandle write, read;
				Open_Pipe(&write, &read);
				pipeWrite.push_back(write);
				pipeRead.push_back(read);
			}

			//zde budou ulozeny pidy procesu, abychom je pak mohli joinout
			std::vector<int> process_handles; 
			//spustime vsechny prikazy
			for (size_t i = 0, lastCommand = commandsParsed.size() - 1; i < commandsParsed.size(); i++) {
				parsed_command &current_params = commandsParsed[i];

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

					std::string switches;
					std::vector<std::string> args;
					if (!parseCommandParams(current_params.arg, &switches, &args)) {
						char * errTxt = (char*)((get_error_message() + '\n').c_str());
						Write_File(STDOUT, errTxt, strlen(errTxt));
					}
					else if (args.size() != 1) {
						Write_File(STDOUT, (char*)(*path).c_str(), (*path).length());
					}
					else {
						if (!Change_Dir((char*)args[0].c_str())) {
							Print_Last_Error(STDERR);
						}
					}
					continue;
				}

				command_params par;
				par.stdinIsConsole = true;
				FDHandle std_in, std_out, std_err, curr_dir;
				bool stdinIsPipe = false;
				/* STDIN */
				if (current_params.redirectstdin) {
					par.stdinIsConsole = false;
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
						//presmerovani se nepovedlo. Proces se nebude vubec spustet. Zavreme zapisovaci rouru (cteci zavrena o kousek vyse)
						if (i != lastCommand) {
							Close_File(pipeWrite[i]);
						}
						continue;
					}
				}
				else if (i == 0) {
					//prvni prikaz, zdedime (duplikujeme handle) stdin od rodice.
					par.stdinIsConsole = stdinIsConsole;
					Duplicate_File(STDIN, &std_in);
				}
				else {
					//nemame presmerovani, ani to neni prvni prikaz - bude se cist z roury.
					par.stdinIsConsole = false;
					std_in = pipeRead[i - 1];
					stdinIsPipe = true;
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
						//presmerovani se nepovedlo. Proces se nebude vubec spustet. Zavreme cteci rouru (zapisovsaci zavrena o kousek vyse)
						if (i != 0) {
							Close_File(pipeRead[i - 1]);
						}
						if (!stdinIsPipe) {//zavreme stdin, pokud to neni roura (tj. oteviral se kvuli nemu soubor nebo duplikoval stdin shellu)
							Close_File(std_in);
						}
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

				par.handles.push_back(std_in);
				par.handles.push_back(std_out);
				par.arg = (char*)(current_params.arg.c_str());
				par.name = current_params.com.c_str();

				/*STDERR - zduplikujeme od shellu*/
				Duplicate_File(STDERR, &std_err);
				par.handles.push_back(std_err);

				/*CURRENT DIR - zduplikujeme od shellu*/
				Duplicate_File(CURRENT_DIR, &curr_dir);
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

		}//konec cyklu radek
		if (buf_command[filled] == EOF) { //goodbye
			break;
		}	
	} //konec cyklu shellu


	st = "\nShell exits.\n";
	st_ = (char *)st.c_str();
	Write_File(STDOUT, st_, strlen(st_));

	delete[] buf_command;
	return(size_t)0;
}