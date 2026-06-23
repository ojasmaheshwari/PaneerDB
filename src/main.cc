#include "QueryParser.h"
#include "Repl/Repl.h"
#include "engine/Engine.h"
#include <iostream>
#include <variant>

int main() {
  std::cout << "Welcome!\n";
  std::cout << "Made by Ojas Maheshwari\n\n";

  Repl repl("PaneerDB");
  Engine engine;

  while (repl.running()) {
    auto input = repl.input();

    if (input == "exit") {
      repl.exit();
      continue;
    }

    QueryParser queryParser;
    queryParser.tokenize(input);

    auto parseResult = queryParser.parse();
    if (parseResult) {
      // parseResult->print();

      engine.execute(parseResult);

      delete parseResult;
    }
  }

  std::cout << "\nBye!\n";
}
