#include "QueryParser.h"
#include "Repl/Repl.h"
#include <iostream>
#include <variant>

int main() {
  std::cout << "Welcome!\n";
  std::cout << "Made by Ojas Maheshwari\n\n";

  Repl repl("PaneerDB");
  QueryParser queryParser;

  while (repl.running()) {
    auto input = repl.input();
    queryParser.tokenize(input);

    auto parseResult = queryParser.parse();
    if (auto* str = std::get_if<SelectStatement>(&parseResult)) {
      (*str).print();
    }

    if (input == "exit") {
      repl.exit();
    }
  }

  std::cout << "\nBye!\n";
}
