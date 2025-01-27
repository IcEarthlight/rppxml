#pragma once

#include <string>
#include <memory>
#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace rppxml {

class RPPXML {
public:
    RPPXML() = default;
    RPPXML(const std::string& name, 
           const std::vector<py::object>& params = std::vector<py::object>(),
           const std::vector<py::object>& children = std::vector<py::object>())
        : name(name), params(params), children(children) { }

    std::string name;
    std::vector<py::object> params;
    std::vector<py::object> children;  // can be either RPPXML or vector<py::object>
};

// convert RPP string to Python object
py::object loads(const std::string& rpp_str);

// convert RPP file to Python object
py::object load(const std::string& filename);

// convert Python object to RPP string
std::string dumps(const py::object& obj);

// convert Python object to RPP file
void dump(const py::object& obj, const std::string& filename);

}