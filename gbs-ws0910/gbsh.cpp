#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <vector>
#include <errno.h>
#include <filesystem>
#include <sstream>

typedef std::vector<std::string> strVec;
namespace fs = std::filesystem;

std::string getPrompt()
{
	char host[50], cwd[250];
	gethostname(host, sizeof(host));
	const char *login = getlogin();
	getcwd(cwd, sizeof(cwd));
	return std::string(login) + "@" +
		   std::string(login) + " " + std::string(cwd) + "> ";
}

std::string getpwd()
{
	char pwd[50];
	getcwd(pwd, sizeof(pwd));
	return pwd;
}

void ls(strVec &commands)
{

	if (commands.size() == 1)
		commands.push_back(getpwd());
	if (fs::directory_entry(commands[1]).is_directory())
	{
		for (const auto &file : fs::directory_iterator(commands[1]))
			std::cout << file.path().filename() << "\t";
		std::cout << std::endl;
	}
	else
	{
		std::cout << "Not a valid directory.\n";
	}
}

void cd(std::string &prompt, strVec &commands)
{
	if (commands.size() == 1)
		commands.push_back("/home");
	if (chdir(commands[1].c_str()) == 0)
		prompt = getPrompt();
	else
		perror("Error ");
}

void clear()
{
	std::cout << "\e[1;1H\e[2J" << '\n';
}

std::vector<std::string> tokenize(std::string command)
{
	std::stringstream cmd_line(command);
	std::string token;
	std::vector<std::string> tokens;
	while (cmd_line >> token)
	{
		tokens.push_back(token);
	}
	return tokens;
}

int main(int argc, char *argv[])
{
	std::string prompt = getPrompt();
	std::string command;
	while (true)
	{
		std::cout << prompt;
		std::getline(std::cin, command);
		strVec commands = tokenize(command);
		if (commands[0] == "exit")
		{
			exit(0);
		}
		else if (commands[0] == "pwd")
		{
			auto path = getpwd();
			std::cout << path << std::endl;
		}
		else if (commands[0] == "clear")
		{
			clear();
		}
		else if (commands[0] == "ls")
		{
			ls(commands);
		}
		else if (commands[0] == "cd")
		{
			cd(prompt, commands);
		}
		else
		{
			for (auto i : commands)
				std::cout << i << std::endl;
		}
	}

	exit(0); // exit normally
}
