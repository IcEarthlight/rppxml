# RPPXML

A Python library for parsing and manipulating REAPER project files (RPP) using WDL implementation.

## Features

- Parse RPP files into Python objects
- Manipulate RPP content programmatically
- Write modified content back to RPP files
- Support .rpp, RTrackTemplate, RfxChain, and more
- Type hints for better IDE support

## Installation

```bash
pip install rppxml
```

## Requirements

- Python 3.10 or later
- C++ compiler with C++11 support

## Development Setup

1. Clone the repository with submodules:
```bash
git clone --recursive https://github.com/IcEarthlight/rppxml.git
cd rppxml
```

2. Install development dependencies:
```bash
pip install -r requirements.txt
```

## Usage

```python
import rppxml
from rppxml import RPPXML

# parse from file
project: RPPXML = rppxml.load("project.rpp")

# parse from string
content: str = """
<REAPER_PROJECT 0.1 "6.75/linux-x86_64" 1681651369
  <TRACK
    NAME "Track 1"
  >
>
"""
project: RPPXML = rppxml.loads(content)

# access data
print(project.params)  # [0.1, "6.75/linux-x86_64", 1681651369]
track: RPPXML = project.children[0]
print(track.name)      # "TRACK"
print(track.params)    # []
print(track.children)  # [['NAME', 'Track 1']]

# create new content
track: RPPXML = RPPXML("TRACK", params=[], children=[
    ["NAME", "New Track"]
])

# write to string
rpp_str: str = rppxml.dumps(track)

# write to file
rppxml.dump(track, "output.rpp")
```

## Building from Source

Using setuptools:
```bash
python setup.py build
```

## Testing Using VSCode

Copy `.vscode/project-settings.json` to `.vscode/settings.json` and go to the Testing view to run tests.

## Contributing

1. Fork the repository
2. Create your feature branch
3. Make your changes
4. Run the tests
5. Submit a pull request

## Acknowledgments

- Uses WDL (Cockos WDL) for RPP parsing
- Built with pybind11 for Python bindings
