## Transport Catalogue
![C++ version](https://img.shields.io/badge/C%2B%2B-17-blue)
![Building on Linux](https://github.com/jys1670/cpp-transport-catalogue/actions/workflows/building_linux.yml/badge.svg)
![Building on Windows](https://github.com/jys1670/cpp-transport-catalogue/actions/workflows/building_windows.yml/badge.svg)
![Linux CI](https://github.com/jys1670/cpp-transport-catalogue/actions/workflows/linux_ci_test.yml/badge.svg)

This repository is an implementation of simplistic public transport navigator.
As of now, there is only JSON input/output interface which is [briefly described here](https://github.com/jys1670/cpp-transport-catalogue/blob/main/docs/json.md).


### Structure
```
.
├── cmake
│   ├── CompileOptions.cmake
│   └── Docs.cmake
├── docs
│   ├── examples
│   ├── html
│   ├── latex
│   ├── Doxyfile
│   ├── index.html
│   └── json.md
├── proto
│   ├── graph.proto
│   ├── map_renderer.proto
│   ├── svg.proto
│   ├── transport_catalogue.proto
│   └── transport_router.proto
├── tests
│   ├── CMakeLists.txt
│   ├── test.cpp
│   ├── timetest_input.json
│   └── timetest_output.json
├── transport-catalogue
│   ├── CMakeLists.txt
│   ├── domain.cpp
│   ├── domain.h
│   ├── geo.cpp
│   ├── geo.h
│   ├── graph.h
│   ├── json_builder.cpp
│   ├── json_builder.h
│   ├── json.cpp
│   ├── json.h
│   ├── json_reader.cpp
│   ├── json_reader.h
│   ├── main.cpp
│   ├── map_renderer.cpp
│   ├── map_renderer.h
│   ├── request_handler.cpp
│   ├── request_handler.h
│   ├── router.h
│   ├── serialization.cpp
│   ├── serialization.h
│   ├── svg.cpp
│   ├── svg.h
│   ├── transport_catalogue.cpp
│   ├── transport_catalogue.h
│   ├── transport_router.cpp
│   └── transport_router.h
├── CMakeLists.txt
└── README.md
```
### Building:

Requirements:
* GCC or alternative (supporting C++17 or later)
* Cmake (tested with 3.21)
* Protobuf (static version by default) (tested with 3.21.7)
* [optional] Boost (tested with 1.80.0)
* [optional] Doxygen (tested with 1.9.5)


Preparations:
```sh
git clone https://github.com/jys1670/cpp-transport-catalogue.git
cd cpp-transport-catalogue
mkdir build && cd build
```

Building and running main executable:
```sh
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release --target main
cd bin
./main [make_base|process_requests] ...
```

Building and running unit tests (requires [Boost](https://www.boost.org/)):
```sh
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTING=ON ..
cmake --build . --config Release --target unit_tests
cd bin
./unit_tests
```

Updating documentation:
```sh
cmake --build . --config Release --target doxygen
```

### Example usage:

 - [Input request (1)](https://raw.githubusercontent.com/jys1670/cpp-transport-catalogue/main/docs/examples/map_input.json), which adds some objects to database and asks to print corresponding map

 - [Answer (1)](https://raw.githubusercontent.com/jys1670/cpp-transport-catalogue/main/docs/examples/map_output.json), containing SVG map as string
```sh
# parse input, generate database and save it as file
# (to avoid regeneration of database stat_requests 
# are processed separately in "process_requests" mode)
./main make_base < map_input.json
# use existing database and process stat_requests
./main process_requests < map_input.json > map_output.json
```
<img src="https://raw.githubusercontent.com/jys1670/cpp-transport-catalogue/main/docs/examples/example_map.svg" width="650" height="600" alt="example-map">

- [Input request (2)](https://raw.githubusercontent.com/jys1670/cpp-transport-catalogue/main/docs/examples/route_input.json), which asks to find fastest path between stops

- [Answer (2)](https://raw.githubusercontent.com/jys1670/cpp-transport-catalogue/main/docs/examples/route_output.json), contains travel time and path description in `items` keys

### Documentation

You can check it out here (incomplete): [Transport-Catalogue-Documentation](https://jys1670.github.io/cpp-transport-catalogue/html/index.html).