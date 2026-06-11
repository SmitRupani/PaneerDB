#ifndef REPL_H
#define REPL_H

#include <string>

class Repl {

public:
	Repl(const std::string &inputPrompt);
	~Repl() = default;

	std::string input();
	bool running();
    void exit();

private:
	bool m_Running;
    std::string m_InputPrompt;
};


#endif
