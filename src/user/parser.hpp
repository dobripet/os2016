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

typedef struct Parsed_command_params {

//Command com;
	std::string com;
	bool redirectstdout;
	bool redirectstdin;
	bool hasswitches;
	std::string switches;
	std::string stdoutpath;
	std::string stdinpath;
	std::vector<std::string> params;	
	
} parsed_params;

typedef struct Parser {

	private:
		std::string err_msg;
		bool parse_command(std::string line, struct Parsed_command_params * command);

	public:
		Parser() : err_msg("No error.") {};
		std::string get_error_message();
		bool parse_commands(std::string line, std::vector<struct Parsed_command_params> * commands);
		//~Parser();
		
} Parser;
