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


def is_valid_slug(slug):
    return re.match(r'^[a-zA-Z0-9_\-]+$', slug) != None


def str_to_identifier(s):
    if not s:
        return "_"
    # Identifiers can't start with a number
    if s[0].isdigit():
        s = "_" + s
    # Capitalize first letter
    s = s[0].upper() + s[1:]
    # Replace special characters with underscore
    s = re.sub(r'\W', '_', s)
    return s


def create_plugin(slug, plugin_dir=None):
    pass


def create_manifest(slug, plugin_dir="."):
    pass


# Default manifest


def create_module(slug, panel_filename=None, source_filename=None):
    # Check slug
    if not is_valid_slug(slug):
        raise UserException("Slug must only contain ASCII letters, numbers, '-', and '_'.")

    # Read manifest
    manifest_filename = 'plugin.json'
    with open(manifest_filename, "r") as f:
        manifest = json.load(f)

    # Check if module manifest exists
    module_manifest = find(lambda m: m['slug'] == slug, manifest['modules'])
    if module_manifest:
        eprint(f"Module {slug} already exists in plugin.json. Edit this file to modify the module manifest.")

    else:
        # Add module to manifest
        module_manifest = {}
        module_manifest['slug'] = slug
        module_manifest['name'] = input_default("Module name", slug)
        module_manifest['description'] = input_default("One-line description (optional)")
        tags = input_default(
            "Tags (comma-separated, case-insensitive, see https://vcvrack.com/manual/Manifest#modules-tags for list)")
        tags = tags.split(",")
        tags = [tag.strip() for tag in tags]
        if len(tags) == 1 and tags[0] == "":
            tags = []
        module_manifest['tags'] = tags

        manifest['modules'].append(module_manifest)

        # Write manifest
        with open(manifest_filename, "w") as f:
            json.dump(manifest, f, indent="  ")

        eprint(f"Added {slug} to {manifest_filename}")

    # Check filenames
    if panel_filename:
        if not os.path.exists(panel_filename):
            raise UserException(f"Panel not found at {panel_filename}.")

        if source_filename and os.path.exists(source_filename):
            if input_default(f"{source_filename} already exists. Overwrite? (y/n)", "n").lower() != "y":
                return

        # Read SVG XML
        tree = xml.etree.ElementTree.parse(panel_filename)

        components = panel_to_components(tree)

        # Tell user to add model to plugin.hpp and plugin.cpp
        identifier = str_to_identifier(slug)
        eprint(f"""
To enable the module, add

	extern Model* model{identifier};

to plugin.hpp, and add

	p->addModel(model{identifier});

to the init() function in plugin.cpp.""")

        # Write source
        # source = components_to_source(components, slug)
        #
        # if source_filename:
        #     with open(source_filename, "w") as f:
        #         f.write(source)
        #     eprint(f"Source file generated at {source_filename}")
        # else:
        #     print(source)
        components_to_sourcefiles(components, slug)


def panel_to_components(tree):
    ns = {
        "svg": "http://www.w3.org/2000/svg",
        "inkscape": "http://www.inkscape.org/namespaces/inkscape",
    }

    root = tree.getroot()
    # Get SVG scale relative to mm
    root_height = root.get('height')
    if root_height.endswith("mm"):
        scale = 1
    else:
        svg_dpi = 75
        mm_per_in = 25.4
        scale = mm_per_in / svg_dpi

    # Get components layer
    group = root.find(".//svg:g[@inkscape:label='components']", ns)
    # Illustrator uses `data-name` (in Unique object ID mode) or `id` (in Layer Names object ID mode) for the group name.
    # Don't test with `not group` since Elements with no subelements are falsy.
    if group is None:
        group = root.find(".//svg:g[@data-name='components']", ns)
    if group is None:
        group = root.find(".//svg:g[@id='components']", ns)
    if group is None:
        raise UserException("Could not find \"components\" layer on panel")

    components = {}
    components['params'] = []
    components['inputs'] = []
    components['outputs'] = []
    components['lights'] = []
    components['widgets'] = []

    for el in group:
        c = {}

        # Get name
        name = el.get('{' + ns['inkscape'] + '}label')
        # Illustrator names
        if not name:
            name = el.get('data-name')
        if not name:
            name = el.get('id')
        if not name:
            name = ""
        # Split name and component class name
        names = name.split('#', 1)
        name = names[0]
        if len(names) >= 2:
            c['cls'] = names[1]
        name = str_to_identifier(name).upper()
        c['name'] = name

        # Get position
        if el.tag == '{' + ns['svg'] + '}rect':
            x = float(el.get('x')) * scale
            y = float(el.get('y')) * scale
            width = float(el.get('width')) * scale
            height = float(el.get('height')) * scale
            c['x'] = round(x, 3)
            c['y'] = round(y, 3)
            c['width'] = round(width, 3)
            c['height'] = round(height, 3)
            c['cx'] = round(x + width / 2, 3)
            c['cy'] = round(y + height / 2, 3)
        elif el.tag == '{' + ns['svg'] + '}circle' or el.tag == '{' + ns['svg'] + '}ellipse':
            cx = float(el.get('cx')) * scale
            cy = float(el.get('cy')) * scale
            c['cx'] = round(cx, 3)
            c['cy'] = round(cy, 3)
        else:
            eprint(f"Element in components layer is not rect, circle, or ellipse: {el}")
            continue

        # Get color
        color = None
        # Get color from fill attribute
        fill = el.get('fill')
        if fill:
            color = fill
        # Get color from CSS fill style
        if not color:
            style = el.get('style')
            if style:
                color_match = re.search(r'fill:\S*(#[0-9a-fA-F]{6})', style)
                color = color_match.group(1)
        if not color:
            eprint(f"Cannot get color of component: {el}")
            continue

        color = color.lower()

        if color == '#ff0000' or color == '#f00' or color == 'red':
            components['params'].append(c)
        if color == '#00ff00' or color == '#0f0' or color == 'lime':
            components['inputs'].append(c)
        if color == '#0000ff' or color == '#00f' or color == 'blue':
            components['outputs'].append(c)
        if color == '#ff00ff' or color == '#f0f' or color == 'magenta':
            components['lights'].append(c)
        if color == '#ffff00' or color == '#ff0' or color == 'yellow':
            components['widgets'].append(c)

    # Sort components
    top_left_sort = lambda w: w['cy'] + 0.01 * w['cx']
    components['params'] = sorted(components['params'], key=top_left_sort)
    components['inputs'] = sorted(components['inputs'], key=top_left_sort)
    components['outputs'] = sorted(components['outputs'], key=top_left_sort)
    components['lights'] = sorted(components['lights'], key=top_left_sort)
    components['widgets'] = sorted(components['widgets'], key=top_left_sort)

    eprint(
        f"Found {len(components['params'])} params, {len(components['inputs'])} inputs, {len(components['outputs'])} outputs, {len(components['lights'])} lights, and {len(components['widgets'])} custom widgets in \"components\" layer.")
    return components


def components_to_modulecpp(components, slug):
    identifier = str_to_identifier(slug)
    source = ""

    # read templateModule.cpp
    with open("src/templates/templateModule.cpp") as f:
        source = f.read()

    # replace date
    year = str(datetime.datetime.now().year)
    source.replace("YEAR", year)

    # replace slug
    source.replace("SLUG", identifier)
    # replace ADDWIDGETS
    source.replace("ADDWIDGETS", components_to_paramwidgets(components, identifier))

    # replace CONFIGWIDGETNAME
    source.replace("CONFIGWIDGETNAME", "//configure input and output names\n")

    return source


def components_to_composite(components, slug):
    identifier = str_to_identifier(slug)
    source = ""

    # read templateModule.cpp
    with open("src/templates/templateComposite.h") as f:
        source = f.read()

    # replace date
    year = str(datetime.datetime.now().year)
    source.replace("YEAR", year)

    # replace slug
    source.replace("SLUG", identifier)

    # replace PORTENUMS
    source.replace("PORTENUMS", components_to_portenums(components, identifier))

    # replace CASEPARAMDESCRIPTIONS
    source.replace("CASEPARAMDESCRIPTIONS", components_to_paramdescriptions(components, identifier))

    return source


def components_to_test(components, slug):
    identifier = str_to_identifier(slug)
    source = ""

    # read templateModule.cpp
    with open("src/templates/templateComposite.h") as f:
        source = f.read()

    # replace date
    year = str(datetime.datetime.now().year)
    source.replace("YEAR", year)

    # replace slug
    source.replace("SLUG", identifier)

    return source


def components_to_paramdescriptions(components, identifier):
    source = ""

    for c in components['params']:
        source += f"""case {identifier}Comp<TBase>::{c['name']}_PARAM:
            ret = {{0.0f, 1.0f, 0.5f, "{c['name']}", " ", freqBase, freqMul}};
            break;

    """
    return source


def components_to_portenums(components, identifier):
    # Params
    source = ""
    source += """
	enum ParamId {"""

    for c in components['params']:
        source += f"""
		    {c['name']}_PARAM,"""
    source += """
		PARAMS_LEN
	};"""

    # Inputs
    source += """
        enum InputId {"""
    for c in components['inputs']:
        source += f"""
            {c['name']}_INPUT,"""
    source += """
            INPUTS_LEN
        };"""

    # Outputs
    source += """
        enum OutputId {"""
    for c in components['outputs']:
        source += f"""
            {c['name']}_OUTPUT,"""
    source += """
            OUTPUTS_LEN
        };"""

    # Lights
    source += """
        enum LightId {"""
    for c in components['lights']:
        source += f"""
            {c['name']}_LIGHT,"""
    source += """
            LIGHTS_LEN
        };"""

    return source


def components_to_paramwidgets(components, identifier):
    # Params
    source = ""
    if len(components['params']) > 0:
        source += "\n"
    for c in components['params']:
        if 'x' in c:
            source += f"""
		addParam(createParam<{c.get('cls', 'RoundBlackKnob')}>(mm2px(Vec({c['x']}, {c['y']})), module, {identifier}::{c['name']}_PARAM));"""
        else:
            source += f"""
		addParam(createParamCentered<{c.get('cls', 'RoundBlackKnob')}>(mm2px(Vec({c['cx']}, {c['cy']})), module, {identifier}::{c['name']}_PARAM));"""

    # Inputs
    if len(components['inputs']) > 0:
        source += "\n"
    for c in components['inputs']:
        if 'x' in c:
            source += f"""
		addInput(createInput<{c.get('cls', 'PJ301MPort')}>(mm2px(Vec({c['x']}, {c['y']})), module, {identifier}::{c['name']}_INPUT));"""
        else:
            source += f"""
		addInput(createInputCentered<{c.get('cls', 'PJ301MPort')}>(mm2px(Vec({c['cx']}, {c['cy']})), module, {identifier}::{c['name']}_INPUT));"""

    # Outputs
    if len(components['outputs']) > 0:
        source += "\n"
    for c in components['outputs']:
        if 'x' in c:
            source += f"""
		addOutput(createOutput<{c.get('cls', 'PJ301MPort')}>(mm2px(Vec({c['x']}, {c['y']})), module, {identifier}::{c['name']}_OUTPUT));"""
        else:
            source += f"""
		addOutput(createOutputCentered<{c.get('cls', 'PJ301MPort')}>(mm2px(Vec({c['cx']}, {c['cy']})), module, {identifier}::{c['name']}_OUTPUT));"""

    # Lights
    if len(components['lights']) > 0:
        source += "\n"
    for c in components['lights']:
        if 'x' in c:
            source += f"""
		addChild(createLight<{c.get('cls', 'MediumLight<RedLight>')}>(mm2px(Vec({c['x']}, {c['y']})), module, {identifier}::{c['name']}_LIGHT));"""
        else:
            source += f"""
		addChild(createLightCentered<{c.get('cls', 'MediumLight<RedLight>')}>(mm2px(Vec({c['cx']}, {c['cy']})), module, {identifier}::{c['name']}_LIGHT));"""

    # Widgets
    if len(components['widgets']) > 0:
        source += "\n"
    for c in components['widgets']:
        if 'x' in c:
            source += f"""
		// mm2px(Vec({c['width']}, {c['height']}))
		addChild(createWidget<{c.get('cls', 'Widget')}>(mm2px(Vec({c['x']}, {c['y']}))));"""
        else:
            source += f"""
		addChild(createWidgetCentered<{c.get('cls', 'Widget')}>(mm2px(Vec({c['cx']}, {c['cy']}))));"""

    return source


def components_to_sourcefiles(components, slug):
    identifier = str_to_identifier(slug)
    with open(f"""src/modules/{identifier}.cpp""", "w") as f:
        f.write(components_to_modulecpp(components, slug))

    with open(f"""src/compositers/{identifier}.h""", "w") as f:
        f.write(components_to_composite(components, slug))

    with open(f"""test/{identifier}Test.cpp""", "w") as f:
        f.write(components_to_modulecpp(components, slug))


def components_to_source(components, slug):
    identifier = str_to_identifier(slug)
    source = ""

    source += f"""#include "plugin.hpp"


struct {identifier} : Module {{"""

    # Params
    source += """
	enum ParamId {"""
    for c in components['params']:
        source += f"""
		{c['name']}_PARAM,"""
    source += """
		PARAMS_LEN
	};"""

    # Inputs
    source += """
	enum InputId {"""
    for c in components['inputs']:
        source += f"""
		{c['name']}_INPUT,"""
    source += """
		INPUTS_LEN
	};"""

    # Outputs
    source += """
	enum OutputId {"""
    for c in components['outputs']:
        source += f"""
		{c['name']}_OUTPUT,"""
    source += """
		OUTPUTS_LEN
	};"""

    # Lights
    source += """
	enum LightId {"""
    for c in components['lights']:
        source += f"""
		{c['name']}_LIGHT,"""
    source += """
		LIGHTS_LEN
	};"""

    source += f"""

	{identifier}() {{
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);"""

    for c in components['params']:
        source += f"""
		configParam({c['name']}_PARAM, 0.f, 1.f, 0.f, "");"""

    for c in components['inputs']:
        source += f"""
		configInput({c['name']}_INPUT, "");"""

    for c in components['outputs']:
        source += f"""
		configOutput({c['name']}_OUTPUT, "");"""

    source += """
	}

	void process(const ProcessArgs& args) override {
	}
};"""

    source += f"""


struct {identifier}Widget : ModuleWidget {{
	{identifier}Widget({identifier}* module) {{
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/{slug}.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));"""

    # Params
    if len(components['params']) > 0:
        source += "\n"
    for c in components['params']:
        if 'x' in c:
            source += f"""
		addParam(createParam<{c.get('cls', 'RoundBlackKnob')}>(mm2px(Vec({c['x']}, {c['y']})), module, {identifier}::{c['name']}_PARAM));"""
        else:
            source += f"""
		addParam(createParamCentered<{c.get('cls', 'RoundBlackKnob')}>(mm2px(Vec({c['cx']}, {c['cy']})), module, {identifier}::{c['name']}_PARAM));"""

    # Inputs
    if len(components['inputs']) > 0:
        source += "\n"
    for c in components['inputs']:
        if 'x' in c:
            source += f"""
		addInput(createInput<{c.get('cls', 'PJ301MPort')}>(mm2px(Vec({c['x']}, {c['y']})), module, {identifier}::{c['name']}_INPUT));"""
        else:
            source += f"""
		addInput(createInputCentered<{c.get('cls', 'PJ301MPort')}>(mm2px(Vec({c['cx']}, {c['cy']})), module, {identifier}::{c['name']}_INPUT));"""

    # Outputs
    if len(components['outputs']) > 0:
        source += "\n"
    for c in components['outputs']:
        if 'x' in c:
            source += f"""
		addOutput(createOutput<{c.get('cls', 'PJ301MPort')}>(mm2px(Vec({c['x']}, {c['y']})), module, {identifier}::{c['name']}_OUTPUT));"""
        else:
            source += f"""
		addOutput(createOutputCentered<{c.get('cls', 'PJ301MPort')}>(mm2px(Vec({c['cx']}, {c['cy']})), module, {identifier}::{c['name']}_OUTPUT));"""

    # Lights
    if len(components['lights']) > 0:
        source += "\n"
    for c in components['lights']:
        if 'x' in c:
            source += f"""
		addChild(createLight<{c.get('cls', 'MediumLight<RedLight>')}>(mm2px(Vec({c['x']}, {c['y']})), module, {identifier}::{c['name']}_LIGHT));"""
        else:
            source += f"""
		addChild(createLightCentered<{c.get('cls', 'MediumLight<RedLight>')}>(mm2px(Vec({c['cx']}, {c['cy']})), module, {identifier}::{c['name']}_LIGHT));"""

    # Widgets
    if len(components['widgets']) > 0:
        source += "\n"
    for c in components['widgets']:
        if 'x' in c:
            source += f"""
		// mm2px(Vec({c['width']}, {c['height']}))
		addChild(createWidget<{c.get('cls', 'Widget')}>(mm2px(Vec({c['x']}, {c['y']}))));"""
        else:
            source += f"""
		addChild(createWidgetCentered<{c.get('cls', 'Widget')}>(mm2px(Vec({c['cx']}, {c['cy']}))));"""

    source += f"""
	}}
}};


Model* model{identifier} = createModel<{identifier}, {identifier}Widget>("{slug}");"""

    return source


def usage(script):
    text = f"""VCV Rack Plugin Development Helper

Usage: {script} <command> ...
Commands:

createplugin <slug> [plugin dir]

	A directory will be created and initialized with a minimal plugin template.
	If no plugin directory is given, the slug is used.

createmanifest <slug> [plugin dir]

	Creates a `plugin.json` manifest file in an existing plugin directory.
	If no plugin directory is given, the current directory is used.

createmodule <module slug> [panel file] [source file]

	Adds a new module to the plugin manifest in the current directory.
	If a panel and source file are given, generates a template source file initialized with components from a panel file.
	Example:
		{script} createmodule MyModule res/MyModule.svg src/MyModule.cpp

	See https://vcvrack.com/manual/PanelTutorial.html for creating SVG panel files.
"""
    eprint(text)


def parse_args(args):
    script = args.pop(0)
    if len(args) == 0:
        usage(script)
        return

    cmd = args.pop(0)
    if cmd == 'createplugin':
        create_plugin(*args)
    elif cmd == 'createmodule':
        create_module(*args)
    elif cmd == 'createmanifest':
        create_manifest(*args)
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
