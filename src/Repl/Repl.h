#ifndef REPL_H
#define REPL_H

#include <string>

class Repl {

public:
	Repl();
	~Repl() = default;

	void welcome();
	std::string input();
	
	void bye(int signum);

	bool running();

private:

	static constexpr const char* m_WelcomeMessage = "MiniDB 1.0\n"
	"By Ojas Maheshwari\n";

	static constexpr const char* m_InputPrompt = "MiniDB> ";

	static constexpr const char* m_ByeMessage = "\nBye!\n";

	bool m_Running = false;
};


#endif
