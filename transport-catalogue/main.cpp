#include "generic_parser.h"
#include "tests.h"
#include <iomanip>
#include <iostream>

int main() {
  TestBasicInputOutput();
  GenericParserInit<std::istream, std::ostream> parser_input{std::cin,
                                                             std::cout};
  GenericParser<std::istream, std::ostream> parser{parser_input};
  parser.ProcessInput();
}