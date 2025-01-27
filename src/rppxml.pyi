from __future__ import annotations

from os import PathLike
from typing import List, Any, overload, TypeAlias

RPPValue: TypeAlias = str | int | float
RPPParams: TypeAlias = List[RPPValue]
RPPChild: TypeAlias = RPPXML | List[RPPValue]

class RPPXML:
    """RPP XML parser using WDL implementation"""
    name: str
    params: RPPParams
    children: List[RPPChild]

    @overload
    def __init__(self, name: str = "") -> None: ...
    @overload
    def __init__(self, name: str, params: RPPParams = [], children: List[RPPChild] = []) -> None: ...
    def __repr__(self) -> str: ...
    def __eq__(self, other: Any) -> bool: ...
    def __copy__(self) -> RPPXML: ...
    def __deepcopy__(self, memo: Any) -> RPPXML: ...

def loads(rpp_str: str) -> RPPXML | List[RPPXML]:
    """Parse RPP from string"""
    ...

def load(filename: PathLike) -> RPPXML | List[RPPXML]:
    """Parse RPP from file"""
    ...

def dumps(obj: RPPXML) -> str:
    """Convert to RPP string"""
    ...

def dump(obj: RPPXML, filename: PathLike) -> None:
    """Write to RPP file"""
    ... 