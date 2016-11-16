#include <set>
#include <regex>
#include <iostream>

#include "parser.hpp"

const bool OK = true;
const bool ERR = !OK;
const std::set<std::string> SET_valid_commands = {"exit", "cd", "md", "wc", "rd", "echo", /*"dir",*/ "type", "freq", "rgen", "ps", "sort", "shell"}; 
const std::regex RGX_nab("[^a-zA-z]");
const std::regex RGX_command_params("<|>|\"|/[a-zA-z]+|[^ \t\"]+");

std::string Parser::get_error_message() {
	return err_msg;
}

bool Parser::parse_commands(std::string line, std::vector<struct Parsed_command_params> * commands) {
	
	size_t cnt = line.length();
	while (cnt > 0 && (line[cnt - 1] == '\n' || line[cnt - 1] == '\r')) {
		cnt--;
	}
	if (cnt <= 1) {
		return true;
	}
	if (cnt != line.length()) {
		line = line.substr(0, cnt);
	}

	std::vector<std::string> commandsStr;
	size_t pos = line.find('\"'), oldpos = 0, lastpos = 0;
	size_t posQbeg = 0, posQend = 0, posPipe = 0;
	
	// neni "  -- muzeme splitnout podle vsech | abychom ziskali jednotlive prikazy
	if (pos == std::string::npos) {
		while((pos = line.find('|', oldpos)) != std::string::npos) {
			commandsStr.push_back(line.substr(oldpos, pos - oldpos));
			oldpos = pos + 1;
		}
		commandsStr.push_back(line.substr(oldpos, std::string::npos));
		
	// je "  -- muzeme splitnout podle | jen pokud se nenachazi mezi lichym a sudym po sobe jdoucim "
	} else {		
		while(posPipe != std::string::npos) {
		
			posQbeg = line.find('\"', oldpos);
			posQend = line.find('\"', posQbeg + 1);
			posPipe = line.find('|', oldpos);
			
			//k oteviraci " chybi zaviraci
			if (posQend == std::string::npos && posQbeg != std::string::npos) {
				err_msg = "Wrong \" parity.";
				return ERR;
			}
			// | je mezi ""
			if (posPipe < posQend && posPipe > posQbeg) {
				oldpos = posQend + 1;
			}
			else {
				if (posPipe - lastpos <= 1) {
					err_msg = "Multiple consecutive | symbols.";
					return ERR;
				}
				commandsStr.push_back(line.substr(lastpos, posPipe - lastpos));
				oldpos = posPipe + 1;
				lastpos = oldpos;
			}
		}				
	}
	for (std::string com : commandsStr) {
		
		Parsed_command_params par;

		if (SET_valid_commands.count(com) > 0) {			
			par = { "", false, false, false, "", "", "" };
			par.com = com;
			commands->push_back(par);
			continue;
		}

		if (!parse_command(com, &par)) {
			return ERR;
		} else {
			commands->push_back(par);
		}
	}
	return OK;
}

bool Parser::parse_command(std::string command, struct Parsed_command_params * paramz) {
	
	command += " ";
	*paramz = {"", false, false, false, "", "", ""};
	bool leftpending = false, rightpending = false;
	int pos = 0;
	
	//skip leading whitespace
	while (isspace(command[pos]) != 0) {
		pos++;
	}
	command = command.substr(pos);
	
	//separate command word
	std::smatch command_match;
	std::regex_search(command, command_match, RGX_nab);
	std::string command_keyword = command.substr(0, command_match.position());
	command = command.substr(command_match.position(), std::string::npos);
	
	//check whether command word is valid
	if(SET_valid_commands.count(command_keyword) < 1) {
		err_msg = "Unknown command \"" + command_keyword + "\"";	
		return ERR;
	}
	paramz->com = command_keyword;
	
	while(std::regex_search(command, command_match, RGX_command_params)) {
		
		std::string str = command_match.str();
		command = command_match.suffix();
		
		if (str == "\"") {
			size_t len = command.find('"');
			std::string par = command.substr(0, len);
			if (leftpending) {
				paramz->stdinpath = par;
				leftpending = false;
			} else if (rightpending) {
				paramz->stdoutpath = par;
				rightpending = false;
			} else {
				paramz->params.push_back(par);
			}
			command = command.substr(len + 1, std::string::npos);
		}
		else if (str == "<") {
			if (paramz->redirectstdin) {
				err_msg = "Attempting to redirect stdin twice.";
				return ERR;
			}		
			paramz->redirectstdin = true;
			if (leftpending || rightpending) {
				err_msg = "Two redirect symbols in a row.";
				return ERR;
			}
			leftpending = true;
		}
		else if (str == ">") {
			if (paramz->redirectstdout) {
				err_msg = "Attempting to redirect stdout twice.";
				return ERR;
			}		
			paramz->redirectstdout = true;
			if (rightpending || leftpending) {
				err_msg = "Two redirect symbols in a row.";
				return ERR;
			}
			rightpending = true;			
		}
		else if (str[0] == '/') {
			if (leftpending || rightpending) {
				err_msg = "Symbol / is followed by > or < symbol.";
				return ERR;
			}
			if(str.length() == 1) {
				err_msg = "Symbol / is not followed by alphabetic symbols.";
				return ERR;
			} 
			if (str[1] == '/') {
				err_msg = "Symbol / is followed by another / symbol.";
				return ERR;
			}
			paramz->hasswitches = true;
			paramz->switches = paramz->switches + str.substr(1, std::string::npos);
		}
		else {
			if (leftpending) {
				paramz->stdinpath = str;
				leftpending = false;
			} else if (rightpending) {
				paramz->stdoutpath = str;
				rightpending = false;
			} else {
				paramz->params.push_back(str);
			}
		}		
	}
	
	if (leftpending || rightpending) {
		err_msg = "Command ends with redirect symbol.";
		return ERR;
	}
	return OK;
}



