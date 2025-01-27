# RPPXML

A Python library for parsing and writing REAPER project files (RPP) using WDL's implementation.

## Development Setup

### Prerequisites
- CMake 3.14 or higher
- Python 3.10 or higher
- A C++ compiler supporting C++11

### Building
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### VSCode Setup
1. Copy `.vscode/project-settings.json` contents to `.vscode/settings.json`
2. Install the Python extension for VSCode
3. Install pytest:
   ```bash
   pip install pytest
   ```

### Running Tests
Tests can be run directly from VSCode:
1. Open the Testing view (flask icon in the sidebar)
2. Click the play button to run all tests
3. Or click individual play buttons next to specific tests

Alternatively, you can run tests from the command line:
```bash
pytest tests -v
```