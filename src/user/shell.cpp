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
	//TODO char * currentDir = what?

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
			FDHandle std_in, std_out;// , std_err = STDERR;
			par.name = paramz.com.c_str();

			if (paramz.redirectstdin) {

				//TODO it's the first command, so it has both stdin file redirect and pipe input - What to do???
				if (i != 0) {
					/*
					TODO myslim, ze muzeme rouru rovnou zavrit pro cteni. (pokud predpokladama, ze stdin ze
					souboru ma prednost pred ctenim z roury).  A producent ma smulu - nema kam psat a ukonci se.
					*/
				}

				//std_in = otevrit soubor(paramz.stdinpath);
			}
			else if (i == 0) {
				//std_in = 0; //tady by mozna mel zdedit stdin od rodice? tj. handle duplikovat
				//TODO urcite duplikovat!
				//Open_File(CURRENT_DIR, &std_in);
				std_in = 0;
			}
			else {
				std_in = pipeRead[i - 1];
			}

			if (paramz.redirectstdout) {

				//TODO it's not the last command, so it has both stdout file redirect and pipe outuput - What to do???
				if (i != lastCommand) {
					/*
					Vystup do souboru ma prednost - roura nedostane nic. Ale myslim, ze nemuzeme rouru rovnou zavrit pro zapis,
					protoze konzument by to detekoval tak, ze zadnej vstup nedostane a ukoncil by se - a tim padem by se ukoncil
					drive, nez predchozi proces a to se nam moc nelibi.

					Takze bych to udelal tak, ze rouru (stale otevrenou pro zapis) pridame do seznamu souboru producenta, ktery ale do
					ni nic nezapise. A ten teprve az pri svem ukonceni rouru zavre. (A dalsi proces (konzument) se teprve az pak ukonci).
					*/

					//TODO Jak predat rouru do seznamu deskriptoru konzumenta? (Kdyz STDOUT v parametrech zabira soubor)
					//Mozna predat vsechno jako vektor/seznam/whatever misto 3 promennejch STDIN/OUT/ERR.
				}

				//std_out = otevrit soubor(paramz.stdoutpath);
			}
			else if (i == lastCommand) {
				//std_out = 1; //tady by mozna mel zdedit stdin od rodice? tj musela by se duplikovat handle
				//TODO urcite duplikovat!
				//Open_File(CURRENT_DIR, &std_out);
				std_out = 0;
			}
			else {
				std_out = pipeWrite[i];
			}

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

			par.handles.push_back(std_in);
			par.handles.push_back(std_out);
			par.handles.push_back(STDERR);

			
			//duplicate current dir for new process
			FDHandle h;
			/*bool ok = */Open_File(CURRENT_DIR, &h); //navratova hodnota muze bejt fail
			par.handles.push_back(h); 

			par.waitForProcess = (i == lastCommand);// || paramz.com == "shell"; //budeme cekat na posledni proces 
			//TODO asi na shell cekat taky? (jakoze tenhle se blokne, kdyz existuje jinej)
			//nebo nejak jinak? nvm
			/*
			cekat na takovy veci jako "wc" kdyz nema jinej vstup? Nevim
			ale kazdopadne WC musi pozrat vstup shellu? Roura? Jak pak resit konec?

			ja bych na nej proste cekal, jenze kam pak pujde WC stdout? V cmd windowsu je platny "wc < soubor.txt | wc"
			Prvni WC udela soubor  a druhy vystup z prvniho
			*/
			
			Create_Process(&par);
		}
	}

	/*
	Shell pobezi ve while(true) {..}.
	Ukoncen bude asi kdyz dostane EOF? (Ctrl+Z)
	Musime nejak resit, aby kontroloval, jestli se nema ukoncit,
	ale zaroven se nesmi tim ctenim zablokovat (coz se normalne pri cteni deje) - nejakej peek? timeout? nebo co?
	*/

	/*
	cd "path"
	Asi bude nutny nejak posilat PID shellu, protoze v tabulce otevrenych souboru se musi zmenit pro shell. 
	*/

	return 0;
}