#include "QueryParser.h"
#include "Repl/Repl.h"
#include <iostream>

int main() {
  std::cout << "Welcome!\n";
  std::cout << "Made by Ojas Maheshwari\n\n";

  Repl repl("PaneerDB");
  QueryParser queryParser;

  while (repl.running()) {
    auto input = repl.input();
    auto tokens = queryParser.tokenize(input);

    // print the tokens
    for (const auto &token : tokens) {
      std::cout << token.value << ' ' << token.getTypeName() << '\n';
    }

    if (input == "exit") {
      repl.exit();
    }
  }

  std::cout << "\nBye!\n";
}
