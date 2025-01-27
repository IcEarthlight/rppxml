import os
import sys

# add the build directory to Python path
build_debug_dir = os.path.join(os.path.dirname(os.path.dirname(__file__)), "build", "Debug")
build_release_dir = os.path.join(os.path.dirname(os.path.dirname(__file__)), "build", "Release")
sys.path.append(build_debug_dir)
sys.path.append(build_release_dir)

import textwrap
import pytest
import rppxml

def test_basic_parsing():
    # test basic block with parameters
    xml: str = textwrap.dedent("""\
    <OBJECT 0.1 "str" 256
      PARAM1 "" 1 2
      PARAM2 "a" analyze
      <SUBOBJECT my/dir ""
        something
        1 2 3 0 0 0 - - -
      >
      <NOTES 0 2
      >
    >""")
    
    obj = rppxml.loads(xml)
    
    # check main object
    assert obj.name == "OBJECT"
    assert obj.params == [0.1, "str", 256]
    
    # check children
    assert len(obj.children) == 4
    assert obj.children[0] == ["PARAM1", "", 1, 2]
    assert obj.children[1] == ["PARAM2", "a", "analyze"]
    
    # check nested object
    subobj = obj.children[2]
    assert isinstance(subobj, rppxml.RPPXML)
    assert subobj.name == "SUBOBJECT"
    assert subobj.params == ["my/dir", ""]
    assert len(subobj.children) == 2
    assert subobj.children[0] == ["something"]
    assert subobj.children[1] == [1, 2, 3, 0, 0, 0, "-", "-", "-"]
    
    # check empty object
    notes = obj.children[3]
    assert isinstance(notes, rppxml.RPPXML)
    assert notes.name == "NOTES"
    assert notes.params == [0, 2]
    assert len(notes.children) == 0

def test_roundtrip():
    # test that dumps(loads(x)) preserves structure
    original: str = textwrap.dedent("""\
    <OBJECT 0.1 "str with spaces" 256
      PARAM1 "" 1 2
      PARAM2 "a" analyze
      <SUBOBJECT my/dir ""
        something
        1 2 3 0 0 0 - - -
      >
      <NOTES 0 2
      >
    >""")

    obj = rppxml.loads(original)
    dumped: str = rppxml.dumps(obj)
    reloaded = rppxml.loads(dumped)
    
    assert obj.name == reloaded.name
    assert obj.params == reloaded.params
    assert obj.children == reloaded.children

def test_error_handling():
    # test invalid input
    # with pytest.raises(RuntimeError, match="must start with '<'"):
    #     rppxml.loads("invalid input")
    
    with pytest.raises(RuntimeError, match="Filename cannot be empty"):
        rppxml.load("")

def test_special_characters():
    # test handling of special characters and whitespace
    xml: str = """<OBJ 0\t1        "a b c" \n2\n>"""
    obj = rppxml.loads(xml)
    
    assert obj.name == "OBJ"
    assert obj.params == [0, 1, "a b c"]
    assert obj.children == [[2]]
    
    # check that dumps preserves quotes where needed
    dumped: str = rppxml.dumps(obj)
    assert '"a b c"' in dumped  # space-containing string should be quoted 
