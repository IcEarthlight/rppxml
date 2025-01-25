#include "rppxml.hpp"
#include <WDL/projectcontext.h>
#include <WDL/heapbuf.h>
#include <sstream>
#include <cstring>  // for memcpy

namespace py = pybind11;

namespace rppxml {

namespace {

// parse a line into tokens
std::vector<py::object> parse_line(const char *line)
{
    std::vector<py::object> tokens;
    const char *p = line;
    std::string token;
    
    while (*p) {
        // skip whitespace
        while (*p && isspace(*p)) p++;
        if (!*p) break;
        
        // handle quoted string
        if (*p == '"') {
            p++; // skip opening quote
            token.clear();
            while (*p && *p != '"') {
                token += *p++;
            }
            if (*p == '"') p++; // skip closing quote
            tokens.push_back(py::cast(token));
            continue;
        }
        
        // handle normal token
        token.clear();
        while (*p && !isspace(*p)) {
            token += *p++;
        }
        
        // try to parse as number
        try {
            size_t pos;
            // try as integer
            int i = std::stoi(token, &pos);
            if (pos == token.length()) {
                tokens.push_back(py::cast(i));
                continue;
            }
            
            // try as float
            double d = std::stod(token, &pos);
            if (pos == token.length()) {
                tokens.push_back(py::cast(d));
                continue;
            }
        } catch (...) {
            // not a number, treat as string
        }
        
        tokens.push_back(py::cast(token));
    }
    return tokens;
}

// convert ProjectStateContext to RPPXML
py::object parse_context(ProjectStateContext *ctx)
{
    char line[4096];
    std::vector<std::unique_ptr<RPPXML>> stack;
    std::vector<py::object> result;
    
    while (!ctx->GetLine(line, sizeof(line))) {
        if (line[0] == '<') {
            // start of a new block
            std::vector<py::object> tokens = parse_line(line + 1);  // skip '<'
            if (!tokens.empty()) {
                std::unique_ptr<RPPXML> block = std::make_unique<RPPXML>(
                    py::cast<std::string>(tokens[0]));
                // rest of tokens are parameters
                block->params.assign(tokens.begin() + 1, tokens.end());
                stack.push_back(std::move(block));
            }
        }
        else if (line[0] == '>') {
            // end of current block
            if (!stack.empty()) {
                std::unique_ptr<RPPXML> block = std::move(stack.back());
                stack.pop_back();
                
                if (stack.empty()) {
                    // top-level block
                    result.push_back(py::cast(*block));
                } else {
                    // add to parent's children
                    stack.back()->children.push_back(py::cast(*block));
                }
            }
        }
        else if (!stack.empty()) {
            // content line within a block
            std::vector<py::object> tokens = parse_line(line);
            if (!tokens.empty()) {
                stack.back()->children.push_back(py::cast(tokens));
            }
        }
    }
    
    // return single object if only one, otherwise list
    if (result.size() == 1) {
        return result[0];
    }
    return py::cast(result);
}

// helper function to check if a string needs quotes to avoid ambiguity
bool needs_quotes(const std::string &str)
{
    if (str.empty()) return true; // empty strings ("") needs quotes

    try {
        size_t pos = 0;
        std::stod(str, &pos);
        if (pos == str.size()) // ensure the whole string is parsed ("123abc")
            return true; // an integer/float number needs quotes

    } catch (...) { }

    for (char c : str)
        if (isspace(c) || c == '"' || c == '\'' || c == '>' || c == '<' || 
            c == '\\' || c == '/' || !isprint(c))
            return true;
    
    return false;
}

// helper function to convert py::object to valid non-ambiguous string
std::string stringify_pyobj(const py::object &obj)
{
    if (py::isinstance<py::str>(obj)) {
        std::string str = py::str(obj);
        // add quotes if string is ambiguous
        if (needs_quotes(str)) {
            str = "\"" + str + "\"";
        }
        return str;
    }

    return py::str(obj);
}

// convert RPPXML to ProjectStateContext
void write_context(const RPPXML &obj, ProjectStateContext *ctx)
{
    // write block header with name and params
    std::string line = "<" + obj.name;
    for (const py::object &param : obj.params) {
        line += " ";
        line += stringify_pyobj(param);
    }
    ctx->AddLine("%s", line.c_str());
    
    // write children
    for (const py::object &child : obj.children) {
        try {
            // try to cast to RPPXML
            RPPXML block = child.cast<RPPXML>();
            write_context(block, ctx);
        } catch (const py::cast_error&) {
            // not a block, must be a parameter list
            std::vector<py::object> params = child.cast<std::vector<py::object>>();
            std::string line;
            for (const py::object &param : params) {
                if (!line.empty()) line += " ";
                line += stringify_pyobj(param);
            }
            ctx->AddLine("%s", line.c_str());
        }
    }
    
    // write block end
    ctx->AddLine(">");
}

} // anonymous namespace

py::object loads(const std::string &rpp_str)
{
    // check if input looks like RPP content
    if (rpp_str.empty() || rpp_str[0] != '<') {
        throw std::runtime_error("Invalid RPP content: must start with '<'");
    }

    WDL_HeapBuf hb;
    void *buf = hb.Resize((int)rpp_str.size() + 1);  // +1 for null terminator
    if (buf) {
        memcpy(buf, rpp_str.c_str(), rpp_str.size() + 1);  // copy including null terminator
    }
    
    std::unique_ptr<ProjectStateContext> ctx(ProjectCreateMemCtx_Read(&hb));
    if (!ctx) {
        throw std::runtime_error("Failed to create context from string");
    }
    
    return parse_context(ctx.get());
}

py::object load(const std::string &filename)
{
    if (filename.empty()) {
        throw std::runtime_error("Filename cannot be empty");
    }

    std::unique_ptr<ProjectStateContext> ctx(ProjectCreateFileRead(filename.c_str()));
    if (!ctx) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    return parse_context(ctx.get());
}

std::string dumps(const py::object &obj)
{
    WDL_HeapBuf hb;
    std::unique_ptr<ProjectStateContext> ctx(ProjectCreateMemCtx_Write(&hb));
    if (!ctx) {
        throw std::runtime_error("Failed to create context for writing");
    }
    
    write_context(obj.cast<RPPXML>(), ctx.get());
    delete ctx.release();  // flush the context
    
    const char *data = (const char *)hb.Get();
    size_t len = hb.GetSize();
    // find actual string length (stop at first null)
    while (len > 0 && data[len-1] == '\0') len--;
    
    return std::string(data, len);
}

void dump(const py::object &obj, const std::string &filename)
{
    std::unique_ptr<ProjectStateContext> ctx(ProjectCreateFileWrite(filename.c_str()));
    if (!ctx) {
        throw std::runtime_error("Failed to create file: " + filename);
    }
    
    write_context(obj.cast<RPPXML>(), ctx.get());
}

PYBIND11_MODULE(rppxml, m)
{
    m.doc() = "RPP XML parser using WDL implementation";
    
    py::class_<RPPXML>(m, "RPPXML")
        .def(py::init<>())
        .def(py::init<const std::string &>())
        .def_readwrite("name", &RPPXML::name)
        .def_readwrite("params", &RPPXML::params)
        .def_readwrite("children", &RPPXML::children)
        .def("__repr__", [](const RPPXML &self) {
            std::ostringstream ss;
            ss << "RPPXML(name='" << self.name << "', "
               << "params=" << py::str(py::cast(self.params)) << ", "
               << "children=" << py::str(py::cast(self.children)) << ")";
            return ss.str();
        })
        .def("__eq__", [](const RPPXML &self, py::object other) {
            try {
                // Try to cast other to RPPXML
                if (!py::isinstance<RPPXML>(other)) {
                    py::print("Not a RPPXML instance");
                    return false;
                }
                
                // Get the RPPXML reference
                const RPPXML& rhs = other.cast<const RPPXML&>();
                
                // Compare name directly (string comparison works fine)
                if (self.name != rhs.name) {
                    return false;
                }
                
                // Compare params using Python's comparison
                py::object self_params = py::cast(self.params);
                py::object rhs_params = py::cast(rhs.params);
                if (!self_params.equal(rhs_params)) {
                    return false;
                }
                
                // Compare children using Python's comparison
                py::object self_children = py::cast(self.children);
                py::object rhs_children = py::cast(rhs.children);
                if (!self_children.equal(rhs_children)) {
                    return false;
                }
                
                return true;
            } catch (const py::error_already_set&) {
                py::print("Python error during comparison");
                return false;
            } catch (const std::exception& e) {
                py::print("C++ error during comparison:", e.what());
                return false;
            }
        })
        .def("__copy__", [](const RPPXML &self) {
            RPPXML copy;
            copy.name = self.name;
            copy.params = self.params;
            copy.children = self.children;
            return copy;
        })
        .def("__deepcopy__", [](const RPPXML &self, py::dict) {
            RPPXML copy;
            copy.name = self.name;
            copy.params = py::cast<std::vector<py::object>>(py::module::import("copy").attr("deepcopy")(self.params));
            copy.children = py::cast<std::vector<py::object>>(py::module::import("copy").attr("deepcopy")(self.children));
            return copy;
        });
    
    m.def("loads", &loads, "Parse RPP from string",
          py::arg("rpp_str"));
          
    m.def("load", &load, "Parse RPP from file",
          py::arg("filename"));
          
    m.def("dumps", &dumps, "Convert to RPP string",
          py::arg("obj"));
          
    m.def("dump", &dump, "Write to RPP file",
          py::arg("obj"),
          py::arg("filename"));
}

} // namespace rppxml 