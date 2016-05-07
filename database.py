import json
import os.path
import re
from collections import namedtuple


def parse_dictionary(strings):
    dictionary = dict()
    for string in strings:
        key_value = re.split('=', string)
        if len(key_value) != 2:
            raise Exception("Invalid key=value pair (" + string + ")")
        dictionary[key_value[0]] = key_value[1]
    return dictionary


# Selector
# --------
Selector = namedtuple("Selector", "grid parameters")
def selector(grid, **parameters):
    return Selector(grid, parameters)


# DataBase
# --------

class DataBase:
    class CompileError(Exception):
        '''raise this when there's a problem compiling an extension module'''
        def __init__(self, db, cpptype):
            Exception.__init__("DataBase::CompileError:")
            self.db = db
            self.cpptype = cpptype

        def __str__(self):
            return "Can not build extnesion module for the " + self.db +\
                   " with type '" + self.cpptype + "'"

    class RangeError(Exception):
        def __init__(self, grid):
            Exception.__init__("DataBase::RangeError:")
            self.grid = grid

        def __str__(self):
            return "Cannot find definition for '" + self.grid + "'"

    class MissingParameter(Exception):
        def __init__(self, parameter):
            Exception.__init__("DataBase::MissingParameter:")
            self.parameter = parameter

        def __str__(self):
            return "No value specified for parameter '" + self.parameter + "'"

    class UnknownTranslation(Exception):
        def __init__(self, parameter, value):
            Exception.__init__("DataBase::UnknownTranslation:")
            self.parameter = parameter
            self.value = value

        def __str__(self):
            return "Unable to translate value '" + self.value +\
                   "' for parameter '" + self.parameter + "'"

    class FailedParameterCheck(Exception):
        def __init__(self, checkStr, check):
            Exception.__init__("DataBase::FailedParameterCheck:")
            self.checkStr = checkStr
            self.check = check

        def __str__(self):
            return "Check '" + self.checkStr + "' failed ('" + self.check +\
                   "' evaluates to false)"

    def __init__(self, *includes):
        self.defs = dict()
        for defs in includes:
            print(os.path.basename(defs))
            self.defs.update(json.load(open(defs, 'r')))

    def __len__(self):
        return len(self.defs)

    def iterkeys(self):
        return iter(sorted(self.defs.keys()))

    def __iter__(self):
        return self.iterkeys()

    def __repr__(self):
        return json.dumps(self.defs, indent=2)

    def __getitem__(self, key):
        return self.defs[key]

    def get_variables(self, grid):
        variables = []
        string = self.defs[grid]["type"]
        while True:
            match = re.search(r"\$\(([a-zA-Z]+)\)", string)
            if not match:
                return variables
            if match.group(1) not in variables:
                variables += [match.group(1)]
            string = string[match.end():]

    def get_defaults(self, grid):
        return parse_dictionary(self.defs[grid]["default"])\
                if "default" in self.defs[grid] else dict()

    def get_includes(self, selector):
        if selector.grid not in self.defs:
            raise DataBase.RangeError(selector.grid)
        return self.defs[selector.grid]["include"]

    def get_type(self, selector):
        if selector.grid not in self.defs:
            raise DataBase.RangeError(selector.grid)
        defs = self.defs[selector.grid]

        translations = dict()
        if "translate" in defs:
            for option in defs["translate"]:
                translations[option] = parse_dictionary(defs["translate"][option])
        return self.replace(defs["type"], selector.parameters,
                            self.get_defaults(selector.grid), translations)

    def check_parameters(self, selector):
        defs = self.defs[selector.grid]
        if "checks" in defs:
            for test in defs["checks"]:
                self.check(test, selector.parameters, self.get_defaults(selector.grid))

    # private methods

    def replace(self, string, parameters, defaults, translations):
        while True:
            match = re.search(r"\$\(([a-zA-Z]+)\)", string)
            if not match:
                break
            option = match.group(1)
            if option in parameters:
                value = parameters[option]
            elif option in defaults:
                value = self.replace(defaults[option], parameters, defaults, translations)
                parameters[option] = value
            else:
                raise DataBase.MissingParameter(option)
            if option in translations:
                if value in translations[option]:
                    value = translations[option][value]
                else:
                    raise DataBase.UnknownTranslation(option, value)
            string = string[:match.start()] + value + string[match.end():]
        return string

    def check(self, string, parameters, defaults):
        before = string
        while True:
            match = re.search(r"\$\(([a-zA-Z]+)\)", string)
            if not match:
                break
            option = match.group(1)
            if option in parameters:
                value = parameters[option]
            elif option in defaults:
                value = defaults[option]
            else:
                raise DataBase.MissingParameter(option)
            string = string[:match.start()] + value + string[match.end():]
        if not eval(string):
            raise DataBase.FailedParameterCheck(before, string)
