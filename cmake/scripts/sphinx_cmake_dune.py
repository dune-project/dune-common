# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

""" A cmake extension for Sphinx

tailored for the Dune project.
This is used during `make doc` to build the
build system documentation.
"""

from docutils import nodes
from docutils.parsers.rst import Directive
from itertools import chain

class CMakeParamNode(nodes.Element):
    pass

class CMakeBriefNode(nodes.Element):
    pass

class CMakeFunction(Directive):
    # We do require the name to be an argument
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = False
    has_content = True

    def run(self):
        env = self.state.document.settings.env

        # Parse the content of the directive recursively
        node = nodes.Element()
        node.document = self.state.document
        self.state.nested_parse(self.content, self.content_offset, node)

        brief_nodes = []
        output_nodes = []
        positional_params = []
        required_params = {}
        optional_params = {}

        for child in node:
            if isinstance(child, CMakeParamNode):
                if child["positional"]:
                    positional_params.append(child)
                elif child["required"]:
                    required_params[child["name"]] = child
                else:
                    optional_params[child["name"]] = child
            elif isinstance(child, CMakeBriefNode):
                par = nodes.paragraph()
                self.state.nested_parse(child['content'], self.content_offset, par)
                brief_nodes.append(par)
            else:
                output_nodes.append(child)

        def render_required(paramnode):
            if paramnode["multi"]:
                sl.append(" "*5 + paramnode['name'] + ' ' + paramnode['argname'] + '1 [' + paramnode['argname'] + '2 ...]\n')
            if paramnode["single"]:
                sl.append(" "*5 + paramnode['name'] + ' ' + paramnode['argname'] + '\n')
            if paramnode["option"]:
                sl.append(" "*5 + paramnode['name'] + '\n')
            if paramnode["special"]:
                sl.append(" "*5 + paramnode['argname'] + '\n')

        def render_optional(paramnode):
            if paramnode["multi"]:
                sl.append(' '*4 + '[' + paramnode['name'] + ' ' + paramnode['argname'] + '1 [' + paramnode['argname'] + '2 ...]' + ']\n')
            if paramnode["single"]:
                sl.append(" "*4 + '['+ paramnode['name'] + ' ' + paramnode['argname'] + ']\n')
            if paramnode["option"]:
                sl.append(" "*4 + '['+ paramnode['name'] + ']\n')
            if paramnode["special"]:
                sl.append(" "*4 + '['+ paramnode['argname'] + ']\n')

        # Build the content of the box
        sl = [self.arguments[0] + '(\n']

        for paramnode in positional_params:
            if paramnode["required"]:
                render_required(paramnode)
            else:
                render_optional(paramnode)

        for rp, paramnode in required_params.items():
            render_required(paramnode)
        for op, paramnode in optional_params.items():
            render_optional(paramnode)

        sl.append(")\n")
        lb = nodes.literal_block(''.join(sl), ''.join(sl))
        brief_nodes.append(lb)

        dl = nodes.definition_list()
        for paramnode in chain(positional_params, required_params.values(), optional_params.values()):
            dli = nodes.definition_list_item()
            dl += dli

            dlit = nodes.term(text=paramnode["name"])
            dli += dlit

            dlic = nodes.definition()
            dli += dlic
            self.state.nested_parse(paramnode['content'], self.content_offset, dlic)

        # add the parameter list to the output
        brief_nodes.append(dl)

        return brief_nodes + output_nodes

class CMakeBrief(Directive):
    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = False
    has_content = True

    def run(self):
        node = CMakeBriefNode()
        node['content'] = self.content
        return [node]

class CMakeParam(Directive):
    # We do require the name to be an argument
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = False
    option_spec = {'argname' : lambda s: s,
                   'multi': lambda s: True,
                   'option': lambda s: True,
                   'positional' : lambda s: True,
                   'required': lambda s: True,
                   'single': lambda s: True,
                   'special': lambda s: True
                   }
    has_content = True

    def run(self):
        node = CMakeParamNode()
        # set defaults:
        node['name'] = self.arguments[0]
        node['single'] = self.options.get('single', False)
        node['multi'] = self.options.get('multi', False)
        node['option'] = self.options.get('option', False)
        node['special'] = self.options.get('special', False)
        node['positional'] = self.options.get('positional', False)
        node['required'] = self.options.get('required', False)
        node['argname'] = self.options.get('argname', self.arguments[0].lower() if self.arguments[0].lower()[-1:] != 's' else self.arguments[0].lower()[:-1])
        node['content'] = self.content
        if node['positional']:
            node['argname'] = ''
        return [node]


class CMakeVariable(Directive):
    # We do require the name to be an argument
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = False
    option_spec = {'argname' : lambda s: s,
                   'multi': lambda s: True,
                   'option': lambda s: True,
                   'positional' : lambda s: True,
                   'required': lambda s: True,
                   'single': lambda s: True
                   }
    has_content = True

    def run(self):
        node = nodes.paragraph()
        self.state.nested_parse(self.content, self.content_offset, node)
        return [node]

class CMakeModule(Directive):
    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = False
    has_content = True

    def run(self):
        node = nodes.paragraph()
        self.state.nested_parse(self.content, self.content_offset, node)
        return [node]

def setup(app):
    app.add_node(CMakeBriefNode)
    app.add_node(CMakeParamNode)
    app.add_directive('cmake_module', CMakeModule)
    app.add_directive('cmake_brief', CMakeBrief)
    app.add_directive('cmake_function', CMakeFunction)
    app.add_directive('cmake_param', CMakeParam)
    app.add_directive('cmake_variable', CMakeVariable)

    return {'version': '0.1'}
