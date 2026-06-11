#include "Repl.h"
#include <csignal>
#include <iostream>
#include <string>

Repl::Repl()
	: m_Running(true)
{
	signal(SIGINT, Repl::bye);
}

void Repl::welcome()
{
	std::cout << m_WelcomeMessage << '\n';
}

std::string Repl::input()
{
	std::cout << m_InputPrompt;
	
	std::string input;
	std::getline(std::cin, input);

	return input;
}

void Repl::bye(int signum)
{
	m_Running = false;
	std::cout << m_ByeMessage;
}

bool Repl::running()
{
	return m_Running;
}
