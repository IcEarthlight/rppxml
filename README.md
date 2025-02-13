# RPPXML

A Python library for efficiently parsing and manipulating REAPER xml files (project files, backups and templates).

## Features

- Implemented in C++ for high performance
- Parse RPP files into Python objects
- Manipulate RPP content programmatically
- Write modified content back to RPP files
- Support .rpp, .rpp-bak, .RfxChain, .RTrackTemplate, and more
- Type hints for better IDE support

## Installation

```bash
pip install rppxml
```

## Requirements

- Python 3.10 or later

## Usage

```python
import rppxml
from rppxml import RPPXML

# parse from file
project: RPPXML = rppxml.load("project.rpp")

# parse from string
content: str = """<REAPER_PROJECT 0.1 "6.75/linux-x86_64" 1681651369
  <TRACK
    NAME "Track 1"
  >
>
"""
project: RPPXML = rppxml.loads(content)

# access data
print(project.params)              # [0.1, "6.75/linux-x86_64", 1681651369]
track: RPPXML = project.children[0]
print(track.name)                  # "TRACK"
print(track.params)                # []
print(track.children)              # [['NAME', 'Track 1']]

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

Requires C++ compiler with C++14 support.

1. Clone the repository with submodules:
```bash
git clone --recursive https://github.com/IcEarthlight/rppxml.git
cd rppxml
```

2. Install development dependencies:
```bash
pip install -r requirements.txt
```

3. Build with setuptools:
```bash
python setup.py bdist_wheel
```

## Testing Using VSCode

Copy `.vscode/project-settings.json` to `.vscode/settings.json` and go to the Testing view to run tests or directly run `python -m pytest tests/test_rppxml.py` at the project root directory.

## Comparative Advantages

| Comparision Content | RPPXML | [Perlence/rpp](https://github.com/Perlence/rpp) |
| --- | --- | --- |
| Implementation | mainly in C++ with Cockos WDL file handling | mainly in Python and uses PLY as a parser framework |
| Load from a 14.8MB RPP File | 1.48s ± 41.8ms (13x faster) | 19.1s ± 1.44s |
| Support Format | `.rpp`, `.rpp-bak`, `.RfxChain`, `.RTrackTemplate` | `.rpp`, `.RTrackTemplate` only |
| Last Update | 2025-02-13 | 2023-04-30 |

## Contributing

1. Fork the repository
2. Create your feature branch
3. Make your changes
4. Run the tests
5. Submit a pull request

## Acknowledgments

- Uses [WDL](https://github.com/justinfrankel/WDL) for file IO and parsing
- Built with [pybind11](https://github.com/pybind/pybind11) for Python bindings
