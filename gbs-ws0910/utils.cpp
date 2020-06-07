#include "utils.h"

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
		if (commands[1] == ">" || commands[1] == "<" || commands[1] == "|" || commands[1][0] == '-')
			exec_with_args(commands);
		else
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

strVec envr()
{
	char **env = environ;
	strVec envs;
	for (std::size_t i = 0; env[i] != NULL; ++i)
		envs.push_back(env[i]);
	return envs;
}

void set_env(strVec &commands)
{
	commands.push_back("");
	if (getenv(commands[1].c_str()) != NULL)
		std::cout << "Variable already defined. Updating.\n";
	setenv(commands[1].c_str(), commands[2].c_str(), 1);
	perror("");
}

void unset_env(strVec &commands)
{
	commands.push_back("");
	if (getenv(commands[1].c_str()) == NULL)
		std::cout << "Variable already undefined.\n";
	else
	{
		unsetenv(commands[1].c_str());
		perror("");
	}
}

strVec tokenize(std::string command)
{
	std::stringstream cmd_line(command);
	std::string token;
	strVec tokens;
	while (cmd_line >> token)
	{
		tokens.push_back(token);
	}
	return tokens;
}

char **getNextProgram(strVec &commands)
{
	size_t argc = 0;
	std::find_if(commands.begin(), commands.end(), [&](auto iter) {
		if (iter == ">" || iter == "<" || iter == "|")
			return true;
		argc++;
		return false;
	});

	if (argc == 0)
		return NULL;

	char **program_with_args = new char *[argc + 1];
	program_with_args[argc] = NULL;
	for (size_t i = 0; i < argc; i++)
	{
		program_with_args[i] = new char[commands.front().length() + 1];
		strcpy(program_with_args[i], commands.front().c_str());
		commands.pop_front();
	}

	return program_with_args;
}

std::string getFile(strVec &commands)
{
	if (commands.begin() == commands.end())
		return std::string();
	auto file = commands.front();
	commands.pop_front();
	return file;
}

void initPipes(int pipes[][2], size_t size)
{
	for (size_t i = 0; i < size; i++)
		pipe(pipes[i]);
}

void closePipesTill(int pipes[][2], size_t _this)
{
	for (size_t i = 0; i < _this; i++)
	{
		close(pipes[i][READ_END]);
		close(pipes[i][WRITE_END]);
	}
}

int openFile(fileMap &files, const int key, const int file_mode)
{
	if (files.find(key) != files.end())
	{
		if (file_mode) //output file required
		{
			if (files[key].second.empty())
				return file_mode;

			int outfile = open(files[key].second.c_str(), O_WRONLY | O_CREAT);
			return outfile;
		}
		if (!file_mode) //input file required
		{
			if (files[key].first.empty())
				return file_mode;

			int infile = open(files[key].first.c_str(), O_RDONLY);
			return infile;
		}
	}
	return file_mode; //if entry doesn't exist return file_mode which means STDIN or STDOUT
}

void execute(progVec &programs, fileMap &files)
{
	int status;
	int parent;
	unsigned int prog_count = 1;

	if (programs.size() > 1)
	{
		int pipes[programs.size() - 1][2];
		unsigned int total_pipes = programs.size() - 1;
		initPipes(pipes, total_pipes);
		unsigned int this_pipe = 0; //start from the 0th pipe

		while (!programs.empty())
		{
			auto next = programs.front();
			programs.pop_front();
			if (!fork())
			{
				if (this_pipe) //2nd program onward; read from previous pipe
				{
					dup2(pipes[this_pipe - 1][READ_END], STDIN_FILENO);
				}
				else //first program; can read from file
				{
					int infile = openFile(files, prog_count, 0);
					dup2(infile, STDIN_FILENO);
				}
				if (this_pipe != total_pipes) //middle programs; write to own pipe
				{
					dup2(pipes[this_pipe][WRITE_END], STDOUT_FILENO);
				}
				else //last program; possibly write to file
				{
					int outfile = openFile(files, prog_count, 1);
					dup2(outfile, STDOUT_FILENO);
				}
				closePipesTill(pipes, this_pipe);
				execvp(next[0], next);
			}

			if (this_pipe)
				closePipesTill(pipes, this_pipe);
			wait(&status);
			if (status == -1)
			{
				std::cerr << "Exec error.\n";
				return;
			}
			this_pipe++;
			prog_count++;
		}
	}
	else //one and only program; can read and write to files but nothing to do with pipe
	{
		auto program = programs.front();
		parent = fork();
		if (parent < 0)
			perror("Fork Error ");
		else if (parent > 0)
		{
			wait(&status);
		}
		else
		{
			//TODO: CLOSE ALL PIPES HERE
			int outfile = openFile(files, prog_count, 1);
			int infile = openFile(files, prog_count, 0);
			dup2(outfile, STDOUT_FILENO);
			dup2(infile, STDIN_FILENO);
			execvp(program[0], program);
		}
	}
}

void exec_with_args(strVec &commands)
{
	auto env_val = getpwd() + "/gbsh";
	setenv("parent", env_val.c_str(), 1);

	progVec program_list;
	fileMap file_map;

	program_list.push_back(getNextProgram(commands));
	while (!commands.empty())
	{
		auto next_token = commands.front();
		commands.pop_front();
		if (next_token == "|")
		{
			auto next = getNextProgram(commands);
			if (next == NULL)
			{
				std::cerr << "Invalid Command: Nothing to pipe into.\n";
				return;
			}
			else
				program_list.push_back(next);
		}
		else if (next_token == ">")
		{
			auto outfile = getFile(commands);
			if (outfile.empty())
			{
				std::cerr << "Invalid Command: Nothing to write output to.\n";
				return;
			}
			else
			{
				size_t key = program_list.size();
				if (file_map.find(key) != file_map.end())
					file_map[key].second = outfile;
				else
					file_map.insert({key, std::make_pair("", outfile)});
			}
		}
		else if (next_token == "<")
		{
			auto infile = getFile(commands);
			if (infile.empty())
			{
				std::cerr << "Invalid Command: Nothing to read input from.\n";
				return;
			}
			else
			{
				size_t key = program_list.size();
				if (file_map.find(key) != file_map.end())
					file_map[key].first = infile;
				else
					file_map.insert({key, std::make_pair(infile, "")});
			}
		}
		else
		{
			std::cerr << "Invalid Command.\n";
			return;
		}
	}
	execute(program_list, file_map);
}

void run(strVec &commands, std::string& prompt)
{

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
	else if (commands[0] == "environ")
	{
		strVec env = envr();
		for (auto i : env)
		{
			std::cout << i << std::endl;
		}
	}
	else if (commands[0] == "setenv")
	{
		set_env(commands);
	}
	else if (commands[0] == "unsetenv")
	{
		unset_env(commands);
	}
	else
	{
		exec_with_args(commands);
	}
}

