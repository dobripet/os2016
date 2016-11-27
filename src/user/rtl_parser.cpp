#include "rtl.h"

#include <regex>
#include <set>

std::string err_msg;

std::string get_error_message() {
	return err_msg;
}

const std::set<std::string> SET_valid_commands = { "exit", "cd", "md", "wc", "rd", "echo", "dir", "type", "freq", "rgen", "ps", "sort", "shell", "del" };
const std::regex RGX_nab("[^a-zA-Z]");
//const std::regex RGX_abnum("[a-zA-Z0-9\\\\.]+");
const std::regex RGX_arg("[a-zA-z0-9\\\\._]+|[^ \t\n\r\"<>]+");

//rozseka poslanou sekvenci prikazu (line) podle rour (pokud nejsou mezi "")
bool splitByPipes(std::string line, std::vector<std::string> * commandsStr) {
	//zahodime znaky konce radky, pokud se na konci "line" nachazi
	size_t cnt = line.length();
	while (cnt > 0 && (line[cnt - 1] == '\n' || line[cnt - 1] == '\r')) {
		cnt--;
	}
	if (cnt < 1) {
		return true;
	}
	if (cnt != line.length()) {
		line = line.substr(0, cnt);
	}

	size_t pos = line.find('\"'), oldpos = 0, lastpos = 0;
	size_t posQbeg = 0, posQend = 0, posPipe = 0;

	// neni "  -- muzeme splitnout podle vsech | abychom ziskali jednotlive prikazy
	if (pos == std::string::npos) {
		while ((pos = line.find('|', oldpos)) != std::string::npos) {
			(*commandsStr).push_back(line.substr(oldpos, pos - oldpos));
			oldpos = pos + 1;
		}
		(*commandsStr).push_back(line.substr(oldpos, std::string::npos));
	}
	// je "  -- muzeme splitnout podle | jen pokud se nenachazi mezi lichym a sudym po sobe jdoucim "
	else {
		while (posPipe != std::string::npos) {

			posQbeg = line.find('\"', oldpos);
			posQend = line.find('\"', posQbeg + 1);
			posPipe = line.find('|', oldpos);

			//k oteviraci " chybi zaviraci
			if (posQend == std::string::npos && posQbeg != std::string::npos) {
				err_msg = "Wrong \" parity.";
				return false;
			}
			// | je mezi ""
			if (posPipe < posQend && posPipe > posQbeg) {
				oldpos = posQend + 1;
			}
			else {
				if (posPipe - lastpos <= 1) {
					err_msg = "Multiple consecutive | symbols.";
					return false;
				}
				(*commandsStr).push_back(line.substr(lastpos, posPipe - lastpos));
				oldpos = posPipe + 1;
				lastpos = oldpos;
			}
		}
	}
	return true;
}

//najde v poslanem prikazu (commandStr) jmeno prikazu a presmerovani z/do souboru, ostatni bude ponechano v jednom velkem stringu
//napr. z: echo text1 >> soubor text2 " < text3"
//udela: prikaz=echo, stdout=soubor, text=text1  text2  < text3
bool parseCommandRedirects(std::string commandStr, Parsed_command * parsedCommand) {

	commandStr += " ";
	*parsedCommand = { "", false, false, false, "", "", "" };
	bool inPending = false, outPending = false;

	size_t pos = 0;
	//skip leading whitespace
	while (isspace(commandStr[pos]) != 0) {
		pos++;
		if (pos == commandStr.length()) {
			err_msg = "Empty command";
			return false;
		}
	}
	commandStr = commandStr.substr(pos);

	//separate command word
	std::smatch command_match;
	std::regex_search(commandStr, command_match, RGX_nab);
	std::string command_keyword = commandStr.substr(0, command_match.position());
	commandStr = commandStr.substr(command_match.position(), std::string::npos);

	//to lower case
	for (size_t i = 0; i < command_keyword.length(); i++) {
		if (command_keyword[i] >= 'A' && command_keyword[i] <= 'Z') {
			command_keyword[i] = command_keyword[i] - ('Z' - 'z');
		}
	}

	//check whether command word is valid
	if (SET_valid_commands.count(command_keyword) < 1) {
		err_msg = "Unknown command \"" + command_keyword + "\"";
		return false;
	}
	parsedCommand->com = command_keyword;
	std::string arg = "";
	if (isspace(commandStr[0])) {
		commandStr = commandStr.substr(1, std::string::npos);
	}

	for (size_t i = 0; i < commandStr.length(); i++) {
		char c = commandStr[i];

		if (c == '\"') {
			pos = commandStr.find('"', i + 1);
			if (pos == std::string::npos) {
				err_msg = "Wrong \" parity";
				return false;
			}
			else if (inPending) {
				parsedCommand->redirectstdin = true;
				parsedCommand->stdinpath = commandStr.substr(i + 1, (pos - i - 1));
				inPending = false;
				arg += ' ';
			}
			else if (outPending) {
				parsedCommand->redirectstdout = true;
				parsedCommand->stdoutpath = commandStr.substr(i + 1, (pos - i - 1));
				outPending = false;
				arg += ' ';
			}
			else {
				arg += commandStr.substr(i, (pos - i)) + '\"';
			}
			i = pos;
		}

		else if (c == '<') {
			if (inPending) {
				err_msg = "Multiple consecutive > symbols in a row.";
				return false;
			}
			if (outPending) {
				err_msg = "Symbol < follows > symbol.";
				return false;
			}
			if (parsedCommand->redirectstdin) {
				err_msg = "Attempting to redirect stdin twice.";
				return false;
			}
			inPending = true;
		}

		else if (c == '>') {

			if (outPending) {
				if (parsedCommand->appendstdout) {
					err_msg = "Multiple consecutive > symbols in a row.";
					return false;
				}
				else {
					parsedCommand->appendstdout = true;
					continue;
				}
			}
			if (inPending) {
				err_msg = "Symbol > follows < symbol.";
				return false;
			}
			if (parsedCommand->redirectstdout) {
				err_msg = "Attempting to redirect stdout twice.";
				return false;
			}
			outPending = true;
		}

		else {
			if (inPending || outPending) {
				if (isspace(c)) {
					continue;
				}
				std::string suffix = commandStr.substr(i, std::string::npos);
				std::regex_search(suffix, command_match, RGX_arg);
				std::string m = command_match.str();
				if (inPending) {
					parsedCommand->redirectstdin = true;
					parsedCommand->stdinpath = m;
					inPending = false;
				}
				else {
					parsedCommand->redirectstdout = true;
					parsedCommand->stdoutpath = m;
					outPending = false;
				}
				i += m.length() - 1;
				arg += ' ';
			}
			else {
				arg += c;
			}
		}
	}
	if (inPending || outPending) {
		err_msg = "Command ends with redirect symbol.";
		return false;
	}
	parsedCommand->arg = arg;
	return true;
}

//rozdeli cely argument commandu na jednotlive switche a argumenty
//napr. z: text1 /s "delsi text" /ab
//udela: switches=sab, args={text1, delsi text}
bool parseCommandParams(std::string command, std::string *switches, std::vector<std::string> *args) {

	(*switches) = "";
	for (size_t i = 0; i < command.length(); i++) {
		char c = command[i];
		if (c == '\"') {
			size_t pos = command.find('\"', i + 1);
			if (pos == std::string::npos) {
				err_msg = "Wrong \" parity";
				return false;
			}
			std::string a = command.substr(i + 1, pos - i - 1);
			(*args).push_back(a);
			i = pos;
		}
		else if (c == '/') {
			bool hasSome = false;
			while (true) {
				i++;
				if (i == command.length() && !hasSome) {
					err_msg = "Invalid syntax: command ends with / symbol";
					return false;
				}
				c = command[i];
				if (isspace(c) || c == '\"') {
					if (hasSome) {
						i--;
						break;
					}
					else {
						err_msg = "Invalid syntax: empty switch";
						return false;
					}
				}
				if((*switches).find(c) == std::string::npos) {
					(*switches) += c;
					hasSome = true;
				}
			}
		}
		else if (isspace(c)) {
			continue;
		}
		else {
			std::string a = "";
			while (true) {
				a += command[i];
				if (i + 1 == command.length() ||
					isspace(command[i + 1]) ||
					command[i + 1] == '\"' //||
					//command[i + 1] == '/'
					) {

					break;
				}
				i++;
			}
			if (a.length() > 0) {
				(*args).push_back(a);
			}
		}
	}
	return true;
}
