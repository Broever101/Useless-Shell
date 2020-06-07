#include "utils.h"

int main(int argc, char *argv[])
{

	signal(SIGINT, SIG_IGN);
	std::string prompt = getPrompt();
	std::string command;
	std::string path = getpwd() + "/gbsh";
	setenv("SHELL", path.c_str(), 1);
	while (true)
	{
		strVec commands;
		std::cout << prompt;
		std::getline(std::cin, command);
		if (command.empty())
			continue;
		commands = tokenize(command);
		if (commands.back() == "&")
		{
			commands.pop_back();
			if (!fork())
			{
				run(commands, prompt);
				exit(0);
			}
		}
		else 
		{
			run(commands, prompt);
		}		
	}
	exit(0); // exit normally
}
