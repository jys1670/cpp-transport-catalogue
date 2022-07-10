# Transport Catalogue

This repository is a simple implementation of information storage, which can potentially be used in some kind of navigation process.
As of now, there is only JSON interface capable of printing/visualizing stops and routes information.

**Building:**

```sh
git clone https://github.com/jys1670/cpp-transport-catalogue.git
cd cpp-transport-catalogue
mkdir build && cd build
cmake ..
make
# To run 
./transport-catalogue
```

**Example usage:**

 - [Input request (2)](https://raw.githubusercontent.com/jys1670/cpp-transport-catalogue/main/tests/2.json), which adds some objects to database and asks to print corresponding map

 - [Answer (2)](https://raw.githubusercontent.com/jys1670/cpp-transport-catalogue/main/tests/2a.json), containing SVG map as string

<img src="https://raw.githubusercontent.com/jys1670/cpp-transport-catalogue/main/docs/examples/example_map.svg" width="650" height="600" alt="example-map">

### Documentation
- **Is in a work-in-progress state**
- You can check it out here: [Transport-Catalogue-Documentation](https://jys1670.github.io/cpp-transport-catalogue/html/index.html).