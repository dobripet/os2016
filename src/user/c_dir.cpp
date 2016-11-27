#include "rtl.h"
#include "c_dir.h"
#include <sstream>

HRESULT handle_dir(char *path, FDHandle STDOUT, FDHandle STDERR);

//vypise obsah adresare/u
size_t __stdcall dir(const CONTEXT &regs) {

	FDHandle STDIN = (FDHandle)regs.R8;
	FDHandle STDOUT = (FDHandle)regs.R9;
	FDHandle STDERR = (FDHandle)regs.R10;
	char * arg = (char*)regs.Rcx;

	//parsovani argumentu
	std::string switches;
	std::vector<std::string> args;
	if (!parseCommandParams(arg, &switches, &args)) {
		char * errTxt = (char*)(("DIR: " + get_error_message() + '\n').c_str());
		Write_File(STDOUT, errTxt, strlen(errTxt));
		return (size_t)1;
	}

	//zpracovani prepinacu
	for (size_t s = 0; s < switches.length(); s++) {
		if (tolower(switches[s]) == 'h') {
			char * msg = "Print list of files and subdirectories in a directory.\n\n  DIR [path]\n\0";
			if (!Write_File(STDOUT, msg, strlen(msg))) {
				if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
					std::string msg = "DIR: An error occurred while writing to STDOUT\n";
					Print_Last_Error(STDERR, msg);
					return (size_t)1;
				}
			}
			return (size_t)0;
		}
		else {
			std::string msg("DIR: Invalid switch: ");
			msg += switches[s];
			msg += " \n";
			Write_File(STDERR, (char*)(msg.c_str()), strlen(msg.c_str()));
			return (size_t)1;
		}
	}


	//zpracovani bez parametru, vypise aktualni adresar
	if (args.size() == 0) {
		if (handle_dir(nullptr, STDOUT, STDERR) != S_OK) {
			return (size_t)1;
		}
	}
	else {
		//zpracovani adresaru o jednom 
		for (size_t i = 0; i < args.size(); i++) {
			char * path = (char*)(args[i].c_str());
			if (handle_dir(path, STDOUT, STDERR) != S_OK) {
				return (size_t)1;
			}
		}
	}
	return (size_t)0;
}

//slouzi k vypisu tabulky pro jeden adresar
HRESULT handle_dir(char *path, FDHandle STDOUT, FDHandle STDERR) {
	bool success;
	size_t written;
	size_t size;
	size_t sum = 0;
	int dir_count = 0;
	size_t file_count = 0;
	std::vector<node_info*> all_info;
	Get_Dir_Nodes(&all_info, path);
	if (all_info.size() < 1) {
		Print_Last_Error(STDERR, "DIR: an error occured while processing: " + std::string(path)+ "\n");
		return S_FALSE;
	}
	std::stringstream ss;
	std::stringstream text;
	//vypis pro slozku
	if (all_info[0]->type == TYPE_DIRECTORY) {
		text << "Directory of: " << all_info[0]->path << "\n\n";
		ss.width(30);
		ss << "Name";
		text << ss.str();
		ss.str("");
		ss.width(20);
		ss << "Size/Type";
		text <<  ss.str() << "\n";
		ss.str("");
		ss.width(30);
		ss << ".";
		text << ss.str();
		ss.str("");
		ss.width(20);
		ss << "<DIR>";
		text << ss.str() << "\n";
		ss.str("");
		ss.width(30);
		ss << "..";
		text << ss.str();
		ss.str("");
		ss.width(20);
		ss << "<DIR>";
		text << ss.str() << "\n";
	}
	//vypis pro soubor
	else {
		text << "Directory of: " << all_info[0]->pathParent << "\n\n";
		ss.width(30);
		ss << "Name";
		text << ss.str();
		ss.str("");
		ss.width(20);
		ss << "Size/Type";
		text << ss.str() << "\n";
		ss.str("");
		ss.width(30);
		ss << all_info[0]->name;
		text << ss.str();
		ss.str("");
		ss.width(20);
		ss << all_info[0]->size;
		sum += all_info[0]->size;
		file_count++;
		text << ss.str() << "\n";
	}
	delete all_info[0];
	//vypis potomku
	for (size_t i = 1; i < all_info.size(); i++) {
		std::string line = "";
		std::stringstream ss;
		ss.width(30);
		ss << all_info[i]->name;
		line += ss.str();
		ss.str("");
		ss.width(20);
		if (all_info[i]->type == TYPE_DIRECTORY) {
			ss << "<DIR>";
			dir_count++;
		}
		else {
			ss << all_info[i]->size;
			sum += all_info[i]->size;
			file_count++;
		}
		line += ss.str();
		text << line << "\n";
		delete all_info[i];
	}
	text << "\t\t" << file_count << " File(s) \t" << sum << " bytes\n";
	text << "\t\t" << dir_count << " Dir(s) \n";
	size = text.str().length();
	success = Write_File(STDOUT, (char *)text.str().c_str(), size, &written);
	//osetreni chyby vypisu
	if (!success || written != size) {
		if (Get_Last_Error() != ERR_IO_PIPE_READCLOSED) {
			Print_Last_Error(STDERR, "DIR: writing to stdout failed.\n");
			return (size_t)1;
		}
	}
	return (size_t)0;
}