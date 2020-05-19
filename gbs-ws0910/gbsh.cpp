#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>

std::string getPrompt()
{	
	char host[50], cwd[250];
	gethostname(host, sizeof(host));
	const char* login = getlogin();
	getcwd(cwd, sizeof(cwd));
	return std::string(login) + "@" + 
	std::string(login) + " " + std::string(cwd)
	+ "> ";				
}

std::string getpwd(){
	char pwd[50];
	getcwd(pwd, sizeof(pwd));
	return pwd;
}

void cls(){
	std::cout<<"\e[1;1H\e[2J"<<'\n';
}

std::vector<std::string> tokenize(std::string command){
	std::stringstream cmd_line(command);
	std::string token;
	std::vector<std::string> tokens;
	while(cmd_line>>token){
		tokens.push_back(token);
	}
	return tokens;
}

int main(int argc, char *argv[]) {
	std::string prompt = getPrompt();
	std::string command;
	while(true){
		std::cout<<prompt;
		std::getline(std::cin,command);
		std::vector<std::string> commands = tokenize(command);
		if (commands[0] == "exit"){
			exit(0);
		}
		else if (commands[0] == "pwd"){
			std::cout<<getpwd()<<std::endl;
		}
		else if (commands[0] == "clear"){
			cls();
		}
		else if (commands[0] == "ls"){

		}
		else{
			for (auto i:commands)
				std::cout<<i<<std::endl;
		}
	}
 
	exit(0); // exit normally	
}
