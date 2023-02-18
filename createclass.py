#!/usr/bin/env python3

import sys
import os
import re
import json
import xml.etree.ElementTree
import datetime

# Version check
f"Python 3.6+ is required"


class UserException(Exception):
    pass


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def find(f, array):
    for a in array:
        if f(a):
            return f


def input_default(prompt, default=""):
    str = input(f"{prompt} [{default}]: ")
    if str == "":
        return default
    return str













def components_to_modulecpp(components, slug):
    identifier = str_to_identifier(slug)
    source = ""

    # read templateModule.cpp
    with open("src/templates/templateModule.cpp") as f:
        source = f.read()

    # replace date
    year = str(datetime.datetime.now().year)
    source = source.replace("YEAR", year)

    # replace slug
    source = source.replace("SLUG", identifier)
    # replace ADDWIDGETS
    source = source.replace("ADDWIDGETS", components_to_paramwidgets(components, identifier))

    # replace CONFIGWIDGETNAME
    source = source.replace("CONFIGWIDGETNAME", components_to_PortNaming(components))

    return source



def components_to_test(components, slug):
    identifier = str_to_identifier(slug)
    source = ""

    # read templateModule.cpp
    with open("src/templates/templateTest.cpp") as f:
        source = f.read()

    # replace date
    year = str(datetime.datetime.now().year)
    source = source.replace("YEAR", year)

    # replace slug
    source = source.replace("SLUG", identifier)

    return source




def components_to_sourcefiles(components, slug):
    identifier = str_to_identifier(slug)
    with open(f"""src/modules/{identifier}.cpp""", "w") as f:
        f.write(components_to_modulecpp(components, slug))

    with open(f"""src/composites/{identifier}.h""", "w") as f:
        f.write(components_to_composite(components, slug))

    with open(f"""test/test{identifier}.cpp""", "w") as f:
        f.write(components_to_test(components, slug))

    edit_plugin_and_testmain_files(identifier)

    edit_readme(identifier)


def edit_plugin_and_testmain_files(identifier):
    with open(f"""src/plugin.cpp""", "r") as f:
        source = f.read()
        source = source.replace("// ADD ADDMODEL", f"""
    p->addModel(model{identifier});
    // ADD ADDMODEL
        """)

    with open(f"""src/plugin.cpp""", "w") as f:
        f.write(source)

    with open(f"""src/plugin.hpp""", "r") as f:
        source = f.read()
        source = source.replace("//ADD EXTERNS", f"""
extern Model* model{identifier};
//ADD EXTERNS
        """)

    with open(f"""src/plugin.hpp""", "w") as f:
        f.write(source)

    with open(f"""test/main.cpp""", "r") as f:
        source = f.read()
        source = source.replace("// ADD EXTERN", f"""// ADD EXTERN
extern void test{identifier}();
        """)

        source = source.replace("// ADD NEWTEST", f"""    // ADD NEWTEST
    test{identifier}();
        """)

    with open(f"""test/main.cpp""", "w") as f:
        f.write(source)


def usage(script):
    text = f"""
        studio 6 plus 1 class creator script

        Usage: {script} < command > ...
        Commands:

        createclass < ClassName > [plugin dir] i.e. ./createclass.py createclass Filter src/dsp/

       
        """
    eprint(text)

def create_header(classname):
    with open("src/templates/templateClass.h") as f:
        source = f.read()

    # replace date
    year = str(datetime.datetime.now().year)
    source = source.replace("YEAR", year)

    # replace slug
    source = source.replace("CLASSNAME", classname)

    return source


def create_test(classname):
    with open("src/templates/templateClassTest.cpp") as f:
        source = f.read()

    # replace date
    year = str(datetime.datetime.now().year)
    source = source.replace("YEAR", year)

    # replace slug
    source = source.replace("CLASSNAME", classname)

    return source

def add_test(classname):
    with open(f"""test/main.cpp""", "r") as f:
        source = f.read()
        source = source.replace("// ADD EXTERN", f"""// ADD EXTERN
extern void test{classname}();
        """)

        source = source.replace("// ADD NEWTEST", f"""    // ADD NEWTEST
    test{classname}();
        """)

    with open(f"""test/main.cpp""", "w") as f:
        f.write(source)




def create_class(class_name, directory):
    filename = directory + class_name + ".h"

    with open(filename, "w") as f:
        f.write(create_header(class_name))

    testfilename = f"""test/test{class_name}.cpp"""

    with open(testfilename, "w") as f:
        f.write(create_test(class_name))

    add_test(class_name)



def parse_args(args):
    script = args.pop(0)
    if len(args) == 0:
        usage(script)
        return

    cmd = args.pop(0)
    if cmd == 'createclass':
        create_class(*args)

    else:
        eprint(f"Command not found: {cmd}")


if __name__ == "__main__":
    try:
        parse_args(sys.argv)
    except KeyboardInterrupt:
        pass
    except UserException as e:
        eprint(e)
        sys.exit(1)
