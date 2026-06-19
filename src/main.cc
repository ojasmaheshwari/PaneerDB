#include "QueryParser.h"
#include "Repl/Repl.h"
#include <iostream>
#include <variant>

int main() {
  std::cout << "Welcome!\n";
  std::cout << "Made by Ojas Maheshwari\n\n";

  Repl repl("PaneerDB");

  while (repl.running()) {
    auto input = repl.input();

    QueryParser queryParser;
    queryParser.tokenize(input);

    auto parseResult = queryParser.parse();
    parseResult->print();

    if (input == "exit") {
      repl.exit();
    }
  }

  std::cout << "\nBye!\n";
}
