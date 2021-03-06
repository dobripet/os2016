#include "rtl.h"
#include "c_wc.h"
#include <sstream>

const int BUFFER_SIZE = 1024;

//spocita pocet radek, slov a bytu vstupu, statistiku vypise
size_t __stdcall wc(const CONTEXT &regs) {

	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	char * arg = (char*)regs.Rcx;

	//parsovani argumentu
	std::string switches;
	std::vector<std::string> args;
	if (!parseCommandParams(arg, &switches, &args)) {
		char * errTxt = (char*)(("WC: " + get_error_message() + '\n').c_str());
		Write_File(STDOUT, errTxt, strlen(errTxt));
		return (size_t)1;
	}

	//zpracovani prepinacu
	for (size_t s = 0; s < switches.length(); s++) {
		if (tolower(switches[s]) == 'h') {
			char * msg = "Counts lines, words and bytes of an input.\n\n  WC[[drive:][path]]filename\n\0";
			if (!Write_File(STDOUT, msg, strlen(msg))) {
				if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
					std::string msg = "WC: An error occurred while writing to STDOUT\n";
					Print_Last_Error(STDERR, msg);
					return (size_t)1;
				}
			}
			return (size_t)0;
		}
		else {
			std::string msg("WC: Invalid switch: ");
			msg += switches[s];
			msg += " \n";
			Write_File(STDERR, (char*)msg.c_str(), strlen(msg.c_str()));
			return (size_t)1;
		}
	}


	size_t written;
	size_t size;
	bool success;

	//zpracovani bez parametru
	if (args.size() == 0) {
		//cte ze vstupu dokud neni EOF
		size_t filled;
		size_t words = 0;
		size_t lines = 0;
		size_t bytes = 0;
		char buffer[BUFFER_SIZE + 1];
		bool prev_blank = false;
		while (true) {
			Read_File(STDIN, BUFFER_SIZE, buffer, &filled);
			for (size_t i = 0; i < filled; i++) {
				if (isblank(buffer[i])) {
					//mezery a tabulatory
					if (!prev_blank) {
						words++;
						prev_blank = true;
					}
				}
				else if (buffer[i] == '\r') {
					//nedelej nic
				}
				else if (buffer[i] == '\n') {
					//nove radky
					lines++; 
					if (!prev_blank) {
						words++;
					}
					prev_blank = true;
				}
				else {
					//normalni znak
					prev_blank = false;
				}
				bytes++;
			}
			if (buffer[filled] == EOF) { //konec
				break;
			}

		}
		//hlavicka
		std::string text = "";
		std::stringstream ss;
		ss.width(10);
		ss << "Lines";
		text +=ss.str();
		ss.str("");
		ss.width(10);
		ss << "Words";
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << "Bytes" << "\n";
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << lines;
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << words;
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << bytes << "\n";;
		text += ss.str();
		ss.str("");
		size = text.length();
		success = Write_File(STDOUT, (char *)text.c_str(), size, &written);
		//osetreni chyby vypisu
		if (!success || written != size) {
			if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
				Print_Last_Error(STDERR, "WC: An error occurred while writing to STDOUT\n");
				return (size_t)1;
			}
			return (size_t)0;
		}
	}
	else {
		//hlavicka
		std::string text = "";
		std::stringstream ss;
		ss.width(10);
		ss << "Lines";
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << "Words";
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << "Bytes";
		text += ss.str();
		ss.str("");
		ss.width(20);
		ss << "Name" << "\n";
		text += ss.str();
		ss.str("");
		size = text.length();
		success = Write_File(STDOUT, (char *)text.c_str(), size, &written);
		//osetreni chyby vypisu
		if (!success || written != size) {
			if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
				Print_Last_Error(STDERR, "WC: An error occurred while writing to STDOUT\n");
				return (size_t)1;
			}
			return (size_t)0;
		}
		size_t sum_words = 0;
		size_t sum_lines = 0;
		size_t sum_bytes = 0;
		//postupne zpracovani kazdeho souboru
		for (int i = 0; i < args.size(); i++) {
			char * path = (char*)args[i].c_str();
			FDHandle file;
			if (!Open_File(&file, path, F_MODE_READ)) {
				Print_Last_Error(STDERR, "WC: An error occurred while reading from: " + std::string(path));
				continue;//pokracuj k dalsimu souboru
			}
			char buffer[BUFFER_SIZE + 1];
			bool prev_blank = false;
			size_t filled;
			size_t words = 0;
			size_t lines = 0;
			size_t bytes = 0;
			text = "";
			//cte cely soubor a pocita statistiku
			do {
				if (!Read_File(file, BUFFER_SIZE, buffer, &filled)) {
					Print_Last_Error(STDERR, "WC: An error occurred while reading from: " + std::string(path));
					break;
				}
				for (size_t i = 0; i < filled; i++) {
					if (isblank(buffer[i])) {
						//mezery a tabulatory
						if (!prev_blank) {
							words++;
							prev_blank = true;
						}
					}
					else if (buffer[i] == '\r') {
						//nedelej nic
					}
					else if (buffer[i] == '\n') {
						//nove radky
						lines++;
						if (!prev_blank) {
							words++;
						}
						prev_blank = true;
					}
					else {
						//normalni znak
						prev_blank = false;
					}
					bytes++;
				}
			} while (BUFFER_SIZE == filled);
			//tabulka
			ss.width(10);
			ss << lines;
			sum_lines += lines;
			text += ss.str();
			ss.str("");
			ss.width(10);
			ss << words;
			sum_words += words;
			text += ss.str();
			ss.str("");
			ss.width(10);
			ss << bytes;
			sum_bytes += bytes;
			text += ss.str();
			ss.str("");
			ss.width(20);
			ss << path << "\n";
			text += ss.str();
			ss.str("");
			size = text.length();
			success = Write_File(STDOUT, (char *)text.c_str(), size, &written);
			//osetreni chyby vypisu
			if (!success || written != size) {
				if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
					Print_Last_Error(STDERR, "WC: An error occurred while writing to STDOUT\n");
					return (size_t)1;
				}
				return (size_t)0;
			}
		}
		//celkova statistika
		text = "";
		ss.width(10);
		ss << sum_lines;
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << sum_words;
		text += ss.str();
		ss.str("");
		ss.width(10);
		ss << sum_bytes;
		text += ss.str();
		ss.str("");
		ss.width(20);
		ss << "Total" << "\n";
		text += ss.str();
		ss.str("");
		size = text.length();
		success = Write_File(STDOUT, (char *)text.c_str(), size, &written);
		//osetreni chyby vypisu
		if (!success || written != size) {
			if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
				Print_Last_Error(STDERR, "WC: An error occurred while writing to STDOUT\n");
				return (size_t)1;
			}
			return (size_t)0;
		}
	}
	return (size_t)0;
}