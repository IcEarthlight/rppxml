"""
RPPXML
======

This module provides functionality for parsing and manipulating REAPER project files (RPP)
and related formats like RTrackTemplate and RfxChain.

Main Classes:
    RPPXML: The main class representing an RPPXML block

Main Functions:
    load: Parse RPP from file
    loads: Parse RPP from string
    dump: Write to RPP file
    dumps: Convert to RPP string
"""

from .rppxml import *

__version__ = "0.1.4"
__author__ = "IcEarthlight <earthlight2187@hotmail.com>"
