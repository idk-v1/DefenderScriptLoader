#include <fstream>
#include <vector>
#include <iostream>
#include <string>


enum Actions
{
	NONE,  // 
	FIRE,  // 
	WAIT,  // double time
	MOVE,  // int x, int y
	SPAWN, // int entity, int x, int y, int script
	DIR,   // char dir
};

struct Action
{
	Actions type = NONE;
	union
	{
		struct
		{
			int entity;
			int x;
			int y;
			int script;
		} spawn;
		struct
		{
			int x;
			int y;
		} move;
		struct
		{
			double time;
		} wait;
		struct
		{
			char dir;
		} dir;
		struct {} fire;
	};
	Action* next = NULL;

	void print(int indent = 0)
	{
		for (int i = 0; i < indent; i++)
			std::cout << "  ";

		switch (type)
		{
		case Actions::SPAWN:
			std::cout << "Spawn(" <<
				"entity: " << spawn.entity <<
				", x: " << spawn.x <<
				", y: " << spawn.y <<
				", script: " << spawn.script <<
				")\n";
			break;

		case Actions::MOVE:
			std::cout << "Move(" <<
				"x: " << move.x <<
				", y: " << move.y <<
				")\n";
			break;

		case Actions::DIR:
			std::cout << "Dir(" <<
				"dir: " << dir.dir <<
				")\n";
			break;

		case Actions::WAIT:
			std::cout << "Wait(" <<
				"time: " << wait.time <<
				")\n";
			break;

		case Actions::FIRE:
			std::cout << "Fire()\n";
			break;
		}
	}
};

struct Script
{
	Action actions;
	Action* loopPtr = NULL;
};

std::string formatLine(const std::string& line)
{
	std::string ret;

	for (int i = 0; i < line.length(); i++)
	{
		switch (line[i])
		{
		// Remove whitespace
		case ' ':
		case '\t':
			break;

		// Ignore comments, line done
		case '/':
			return ret;

		default:
			ret.push_back(tolower(line[i]));
		}
	}

	return ret;
}

std::vector<std::string> tokenize(std::string line)
{
	std::vector<std::string> tokens;
	tokens.push_back(std::string());
	for (int i = 0; i < line.length(); i++)
	{
		if (line[i] == ',')
		{
			if (tokens[tokens.size() - 1].size() > 0)
				tokens.push_back(std::string());
		}
		else
			tokens[tokens.size() - 1].push_back(line[i]);
	}

	return tokens;
}


std::vector<Script> loadScripts(std::string name)
{
	std::vector<Script> scripts;

	std::ifstream file(name);
	if (file.is_open())
	{
		scripts.push_back(Script());
		Action* lastAction = &scripts[scripts.size() - 1].actions;

		std::string line;
		while (std::getline(file, line))
		{
			line = formatLine(line);
			
			if (line.length() > 0) // Ignore now empty lines
			{
				std::vector<std::string> tokens = tokenize(line);
				if (tokens.size() > 0)
				{
					Action* action = new Action();
					if (tokens[0] == "end")
					{
						scripts[scripts.size() - 1].loopPtr = 
							scripts[scripts.size() - 1].loopPtr->next;

						scripts.push_back(Script());
						lastAction = &scripts[scripts.size() - 1].actions;

						delete action;
						continue;
					}
					else if (tokens[0] == "spawn")
					{
						action->type = Actions::SPAWN;
						action->spawn.entity = atoi(tokens[1].data());
						action->spawn.x = atoi(tokens[2].data());
						action->spawn.y = atoi(tokens[3].data());
						action->spawn.script = atoi(tokens[4].data());
					}
					else if (tokens[0] == "move")
					{
						action->type = Actions::MOVE;
						action->move.x = atoi(tokens[1].data());
						action->move.y = atoi(tokens[2].data());
					}
					else if (tokens[0] == "dir")
					{
						action->type = Actions::DIR;
						action->dir.dir = tokens[1][0];
					}
					else if (tokens[0] == "wait")
					{
						action->type = Actions::WAIT;
						action->wait.time = atof(tokens[1].data());
					}
					else if (tokens[0] == "fire")
					{
						action->type = Actions::FIRE;
					}
					else if (tokens[0] == "loop")
					{
						// loopPtr is null, im too tired to think
						// just set it to next after done with all this stuff
						scripts[scripts.size() - 1].loopPtr = lastAction;

						delete action;
						continue;
					}
					else // invalid op
					{
						delete action;
					}
					lastAction->next = action;
					lastAction = action;
				}
			}
		}

		file.close();
	}
	else
		std::cout << "Failed to open \"" << name << "\"\n";

	scripts.pop_back(); // makes extra script, easier not to fight it
	return scripts;
}

int main()
{
	std::vector<Script> scripts = loadScripts("test.dscr");

	for (int i = 0; i < scripts.size(); i++)
	{
		std::cout << "\nScript [" << i << "]:\n";
		Action* next = scripts[i].actions.next;
		bool loop = false;
		for (int c = 0; next; c++)
		{
			if (scripts[i].loopPtr == next)
				loop = true;
			next->print(loop);
			next = next->next;
		}
	}

	return 0;
}