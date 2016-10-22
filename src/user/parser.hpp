#pragma once
#include <string>
#include <vector>

/*
enum Command {
	rd,
	md,
	echo,
	dir,
	freq,
	rgen,
	ps,
	type,
	wc,
	sort
};
*/

typedef struct Command_params {

//Command com;
	std::string com;
	bool redirectstdout;
	bool redirectstdin;
	bool hasswitches;
	std::string switches;
	std::string stdoutpath;
	std::string stdinpath;
	std::vector<std::string> params;	
	
} Command_params;

typedef struct Parser {

	private:
		std::string err_msg;
		bool parse_command(std::string line, struct Command_params * command);

	public:
		Parser() : err_msg("No error.") {};
		std::string get_error_message();
		bool parse_commands(std::string line, std::vector<struct Command_params> * commands);
		//~Parser();
		
} Parser;
