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
        required_params = {}
        optional_params = {}

        for child in node:
            if isinstance(child, CMakeParamNode):
                if child["required"]:
                    required_params[child["name"]] = child
                else:
                    optional_params[child["name"]] = child
            elif isinstance(child, CMakeBriefNode):
                par = nodes.paragraph()
                self.state.nested_parse(child['content'], self.content_offset, par)
                brief_nodes.append(par)
            else:
                output_nodes.append(child)

        # Build the content of the box
        sl = [self.arguments[0] + '(\n']
        for rp, paramnode in required_params.items():
            if paramnode["multi"]:
                sl.append(" "*(len(self.arguments[0])+2) + paramnode['name'] + ' ' + paramnode['argname'] + '1 [' + paramnode['argname'] + '2 ...]\n')
            if paramnode["single"]:
                sl.append(" "*(len(self.arguments[0])+2) + paramnode['name'] + ' ' + paramnode['argname'] + '\n')
            if paramnode["option"]:
                sl.append(" "*(len(self.arguments[0])+2) + paramnode['name'] + '\n')

        for op, paramnode in optional_params.items():
            if paramnode["multi"]:
                sl.append(' '*(len(self.arguments[0])+1) + '[' + paramnode['name'] + ' ' + paramnode['argname'] + '1 [' + paramnode['argname'] + '2 ...]' + ']\n')
            if paramnode["single"]:
                sl.append(" "*(len(self.arguments[0])+1) + '['+ paramnode['name'] + ' ' + paramnode['argname'] + ']\n')
            if paramnode["option"]:
                sl.append(" "*(len(self.arguments[0])+1) + '['+ paramnode['name'] + ']\n')

        sl.append(")\n")
        lb = nodes.literal_block(''.join(sl), ''.join(sl))
        brief_nodes.append(lb)

        dl = nodes.definition_list()
        for param, paramnode in chain(required_params.items(), optional_params.items()):
            dli = nodes.definition_list_item()
            dl += dli

            dlit = nodes.term(text=param)
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
                   'single': lambda s: True
                   }
    has_content = True

    def run(self):
        node = CMakeParamNode()
        # set defaults:
        node['name'] = self.arguments[0]
        node['single'] = self.options.get('single', False)
        node['multi'] = self.options.get('multi', False)
        node['option'] = self.options.get('option', False)
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
