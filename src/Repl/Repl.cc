#include "Repl.h"
#include <iostream>
#include <string>

Repl::Repl(const std::string &inputPrompt)
    : m_InputPrompt(inputPrompt)
{
    m_Running = true;
}

std::string Repl::input()
{
	std::cout << m_InputPrompt << '>';
	
	std::string input;
	std::getline(std::cin, input);

	return input;
}

bool Repl::running()
{
	return m_Running;
}

void Repl::exit()
{
    m_Running = false;
}
