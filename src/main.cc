#include "Repl/Repl.h"
#include <iostream>

int main() {
	Repl repl;

	repl.welcome();

	while (true) {
		auto input = repl.input();
		std::cout << input << '\n';
	}
}
