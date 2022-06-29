#include "domain.h"
#include "json_reader.h"
#include "request_handler.h"
#include "tests.h"

int main() {
    TestBasicInputOutput();

    GenericReaderInit reader_input{std::cin, std::cout};
    JsonReader reader{reader_input};
    reader.ProcessInput(OutputFormat::Json {});
}