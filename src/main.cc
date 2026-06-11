#include "Repl/Repl.h"
#include <iostream>

int main() {
    Repl repl("PaneerDB");

    std::cout << "Welcome!\n";
    std::cout << "Made by Ojas Maheshwari\n\n";

	while (repl.running()) {
		auto input = repl.input();
		std::cout << input << '\n';

        if (input == "exit") {
            repl.exit();
        }
	}

    std::cout << "\nBye!\n";
}
