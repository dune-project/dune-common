#!/usr/bin/python
import sys
import re
import os
import fnmatch
import errno
from optparse import OptionParser
from pyparsing import *

class rem_proc:
    def __init__(self, name):
        self.name=name
        self.found=False
    def parse(self,tokens):
        self.found=True
    def foreach_begin(self):
        return "\nforeach(i ${"+self.name+"})\n"

    def foreach_end(self):
        return "endforeach(i ${"+self.name+"})\n"

    def process_SUBDIRS(self):
        return ''.join([self.foreach_begin(),
                        "  if(${i} STREQUAL \"test\")\n",
                        "    set(opt EXCLUDE_FROM_ALL)\n",
                        "  endif(${i} STREQUAL \"test\")\n",
                        "  add_subdirectory(${i} ${opt})\n",
                        "  unset(opt)\n"+self.foreach_end()])
        
    def process(self):
        s=""
        if self.found:
            s= getattr(self, "process_%s" % self.name)()
        return s

        
    def process_TESTS(self):
        return ''.join(['\n',
                        self.foreach_begin(),
                        "  add_test(${i} ${i})\n",
                        self.foreach_end()])

    def process_XFAIL_TESTS(self):
        return ''.join(['\n#Set properties for failing tests\n',
                        'set_tests_properties(\n',
                        '  ${XFAIL_TESTS} PROPERTIES\n',
                        '  WILL_FAIL true)'])

    def process_COMPILE_XFAIL_TESTS(self):
        return ''.join(['# Add test for programs that should not compile',
                        '\nforeach(_TEST ${COMPILE_XFAIL_TESTS})\n'
                        '  add_test(NAME ${_TEST}',
                        '          COMMAND ${CMAKE_COMMAND} --build . --target ${_TEST} --config $<CONFIGURATION>)\n',
                        'endforeach(_TEST)\n',
                        'set_tests_properties(${COMPILE_XFAIL_TESTS}\n',
                        '  PROPERTIES WILL_FAIL true)'])


def parse_dependency(tokens):
    tokens[0][0]='add_dependencies('+tokens[0][0]+' '
    tokens[0][-1]=tokens[0][-1]+')'

def comment_lines(tokens):
    tokens[0][0]='# The following lines are comment out as the translation is not clear. TODO review\n#\n# '+tokens[0][0]
    if len(tokens[0])>0:
        for i in range(1, len(tokens[0])):
            if tokens[0][i] == '\n':
                tokens[0][i]='\n#'

def repl_gnudir(m):
    #print "gnudir %s" % m
    return "CMAKE_INSTALL_"+m.upper()

def parse_sources(tokens):
    target=re.sub('_$', '', tokens[0][0])
    tokens[0][0]='add_executable('+target+' '
    tokens[0][-1]=tokens[0][-1]+')'
                                 
class installer:
    def __init__(self, name):
        self.name = name
        self.found_dirs = set()
        self.regex= re.compile(r'(?P<prefix>\S+[^_])_'+self.name)
        self.install_prefix = {
            'SCRIPTS': 'PROGRAMS',
            'DATA': 'FILES',
            'PROGRAMS': 'TARGETS',
            'LIBRARIES': 'TARGETS',
            'HEADERS': 'FILES',
            'check_PROGRAMS': ''
            }[name]
    def create_parser(self,lowercase_name,opt_ws,equals,value):
        parser=Group(Combine(Optional(lowercase_name)+self.name)+Suppress(opt_ws)+ equals + opt_ws + Optional(value))
        return parser
    def parse(self,tokens):
        #import pdb; pdb.set_trace()
        m=self.regex.match(tokens[0][0])
        if m:
            self.found_dirs.add(m.group('prefix'))                
        else:
            self.found_dirs.add('')

    def process(self):
        #import pdb; pdb.set_trace()
        s=""
        for i in self.found_dirs:
            if i:
                install_dir=i+'dir'
            else:
                install_dir=repl_gnudir({
                    'SCRIPTS': 'bindir',
                    'DATA': 'datadir',
                    'PROGRAMS': 'bindir',
                    'LIBRARIES': 'libdir',
                    'HEADERS': 'includedir'
                    }[self.name])
            if not (i == ""):
                var=""+i+'_'+self.name
            else:
                var=self.name
            if not (var.strip().startswith('noinst_') 
                    or var.strip().startswith('check_') 
                    or var.strip().startswith('dist_noinst_')):
                s= s+"\ninstall("+self.install_prefix+" ${"+var+"} DESTINATION ${"+install_dir+"})"
        return s

class target_flags:
    def __init__(self, suffix):
        self.suffix=suffix
        self.known_flags = {
            'ALBERTA_': ['add_dune_alberta_flags',''],
            'AM_': ['target_link_libraries', '${DUNE_LIBS}'],
            'ALUGRID_': ['add_dune_alugrid_flags',''],
            'UG_':  ['add_dune_ug_flags', ''],
            'SUPERLU_': ['add_dune_superlu_flags', ''],
            'DUNEMPI': ['add_dune_mpi_flags', ''],
            'AMIRAMESH_': ['add_dune_amiramesh_flags',''],
            'BOOST_': ['add_dune_boost_flags',''],
            'GMP_': ['add_dune_gmp_flags',''],
            'GRAPE_': ['add_dune_grape_flags',''],
            'PARMETIS_': ['add_dune_parmetis_flags',''],
            'PETSC_': ['add_dune_petsc_flags',''],
            'PSURFACE_': ['add_dune_psurface_flags','']
            }
        for i in range(1,9):
            self.known_flags.update({
                    'ALBERTA'+str(i)+'D_' : ['add_dune_alberta_flags', ''+'GRIDDIM '+str(i)]})
    def parse(self, stri,loc,tokens):
        #print 'target_flags'
        #print self.suffix
        #print tokens
        if (self.suffix == 'CPPFLAGS' or self.suffix=='CXXFLAGS'):
            line=''.join(tokens[0])
            s=''
            for key, value in self.known_flags.items():
                if line.find(key)>-1:
                    s+='\n'+value[0]+'('+re.sub(r'set\((.*)_', '\\1',tokens[0][0])+value[1]+')'
            tokens[0]=s
            #print tokens
        else:
            comment_lines(tokens)

class dir_parser:
    def __init__(self):
        self.has_doxygen=False
    def parse_dir(self,stri,loc,tokens):
        #import pdb;    pdb.set_trace()
        #print "parse_dir"
        #print tokens
        tokens[0][1]=''.join(tokens[0][1:])
        if tokens[0][1].find('doxygen')>-1:
            self.has_doxygen=True
        #print range(2, len(tokens[0]))
        if len(tokens[0]) >=2:
            for i in range(2, len(tokens[0])):
                tokens[0].pop()
    #print tokens
    #print "str="+stri+" |"
    #print "loc="+str(loc)+" |"
    #print tokens[0][1]
    #for i in range(0,len(tokens[0])):
    
            tokens[0][1]=re.sub(r'\$\{([a-z]+dir)\}(\S*)\)', (lambda m: "${"+repl_gnudir(m.group(1))+"}"+m.group(2)+')'), tokens[0][1])
    #print tokens[0][1]
            tokens[0][1]=tokens[0][1].replace('${CMAKE_INSTALL_DATADIR}/doc',
                                                  '${CMAKE_INSTALL_DOCDIR}')
            tokens[0][1]=tokens[0][1].replace('${CMAKE_INSTALL_DATAROOTDIR}/doc',
                                                  '${CMAKE_INSTALL_DOCDIR}')
    #print tokens

    def process(self):
        if self.has_doxygen:
            return '\n\n# Create and install doxygen documentation\nadd_doxygen_target()\n'
        else:
            return ''

def parse_include(tokens):
    tokens[0][0]='# include not needed for CMake\n# '+tokens[0][0]

def parse_if(tokens):

    expression = tokens[0][1]+' '
    tokens[0][0] ='if('
    tokens[0][2] =')\n'
    tokens[0][len([0])-2] ='endif('+expression+')'
    if tokens[0].elsestart :
        tokens[0][4] ='else('+expression+')'
    
def parse_assign(tokens):
    tokens[0][0]='set('+tokens[0][0]+' '
    tokens[0][-1]=tokens[0][-1]+')'

def parse_append(tokens):
    tokens[0][0]='list(APPEND '+tokens[0][0]+' '
    tokens[0][-1]=tokens[0][-1]+')'


def parse_var(stri,loc,tokens):
    #print "parse_var"
    #print tokens
    #print "str="+stri+" |"
    #print "loc=%s|" % loc
    tokens[0]='${'+tokens[0]+'}'
    #print tokens

def rename_var(tokens):
    #print 'rename_var |%s|' % tokens[0]
    #print tokens
    name = re.sub('[ ]*([a-zA-Z0-9_])[ ]*[\n]?', '\\1', tokens[0])
    #print '|%s|' %name
    tokens[0]={
        'ALBERTA': 'ALBERTA_FOUND',
        'ALUGRID': 'ALUGRID_FOUND',
        'AMIRAMESH': 'AMIRAMESH_FOUND',
        'CONVERT': 'CONVERT_FOUND',
        'DUNEWEB': 'DUNEWEB_FOUND',
        'EIGEN': 'EIGEN_FOUND',
        'GLUT': 'GLUT_FOUND',
        'GMP': 'GMP_FOUND',
        'GRAPE': 'GRAPE_FOUND',
        'INKSCAPE': 'INKSCAPE_FOUND',
        'MPI': 'MPI_CXX_FOUND',
        'PARDISO': 'PARDISO_FOUND',
        'PETSC': 'PETSC_FOUND',
        'PSURFACE': 'PSURFACE_FOUND',
        'SUPERLU': 'SUPERLU_FOUND',
        'TEX4HT': 'TEX4HT_FOUND',
        'UG': 'UG_FOUND',
        'WML': 'WML_FOUND'}.setdefault(name, tokens[0])
    if name.startswith('HAVE_DUNE'):
        tokens[0]=re.sub(r'have-(.*)$', '\\1', name.lower().replace('_','-'))+'_FOUND'
    #print tokens[0]

#print rename_var(['bla'])
#print rename_var(['UG'])
#print rename_var(['A'])
change_var2=1

def parse_var2(stri,loc,tokens):
    if change_var2:
        tokens[0]='${'+tokens[0]+'}'
    else:
        tokens[0][0]='@'+tokens[0][0]+'@'

class is_present:
    def __init__(self):
        self.found=False
    def parse(self,tokens):
        self.found=True

def repl_tabs(tokens):
    for i in range(0, len(tokens)):
        tokens[i]=tokens[i].replace('\t', '    ')

def am_2_cmake_string(amstring):    
    ParserElement.setDefaultWhitespaceChars("")
    word = Word(alphanums+"*:.-_=<>&!;,/")
    ws=Word(' ')
    opt_ws=Optional(ws)
    varopen = Literal("$(")#.replaceWith("${")
    varclose = Literal(")")#.replaceWith("}")
    var = Combine(varopen.suppress() + Word(alphanums+"_-")+varclose.suppress())
    var.addParseAction(rename_var)
    var.addParseAction(parse_var)
    var2 = Combine(Literal("@").suppress() +Word(alphanums+"_-")+Literal("@").suppress())
    var2.setParseAction(parse_var2)
    wordorvar = var|var2| word
    qwordorvar=("\""+wordorvar+"\"")|wordorvar
    NL = opt_ws.suppress() + Or(lineEnd,stringEnd)
    #NL.setDebug()
    multiple_words = wordorvar + ZeroOrMore( opt_ws + wordorvar)
    qmultiple_words = ("\""+multiple_words+"\"")|multiple_words
    word_line = qmultiple_words+ZeroOrMore(opt_ws+qmultiple_words)
    comment = Group('#'+ restOfLine('commenttext'))
    continuation = Suppress('\\')+LineEnd()
    #continuation.setDebug()
    equals = Literal("=").suppress()
    value = ZeroOrMore(opt_ws + Optional(word_line) + opt_ws + continuation)+ opt_ws+ word_line
    name = Word(alphanums+"_-")
    beginif = Literal("if")
    #beginif.setDebug()
    lines = Forward()
    ifexpression = restOfLine + NL
    ifexpression.setParseAction(rename_var)
    ifbody = Group(lines)
    endif = Literal('endif')+restOfLine.suppress()
    elsestatement = Forward()
    elsestatement << (opt_ws+ Literal('else').setResultsName('elsestart') + NL + Group (lines))
    #endif.setDebug()
    ifstatement =Forward()
    ifstatement << Group(beginif + ifexpression + Group(lines) + Optional(elsestatement) + opt_ws + endif)#+ NL# +endif #+ NL
    #ifstatement.setDebug()
    ifstatement.setParseAction(parse_if)

    varAssign = Group(name + Suppress(opt_ws)+ equals + opt_ws + Optional(value))
    varAssign.setParseAction(parse_assign)

    docsAssign = Group(Or(Literal("DOCFILES"), Literal("DOCFILES_EXTRA"))+ Suppress(opt_ws)+ equals + opt_ws + Optional(value))
    docsAssign.setParseAction(parse_assign)
    docs_present = is_present()
    docsAssign.addParseAction(docs_present.parse)
    varAddAssign = Group(name + Suppress(opt_ws) + Literal('+=').suppress() + Suppress(opt_ws) + Optional(value))
    varAddAssign.setParseAction(parse_append)

    # Grammar for <lowercase>dir. A parser is needed to convert the GNU autoconf dirs to
    # CMAKE GNU dirs.
    dir_name = Regex("[a-z0-9_-]+dir")#Word(srange("[a-z0-9_-]"))
    dirAssign=Group(dir_name+Suppress(opt_ws)+ equals + opt_ws + Optional(value))
    dirAssign.setParseAction(parse_assign)
    dirParser = dir_parser()
    dirAssign.addParseAction(dirParser.parse_dir)
    #dirAssign.setDebug()

    lowercase_name_= Regex("[a-z0-9_-]+_")

    cppflagsAssign = Group(lowercase_name_+'CPPFLAGS' + Suppress(opt_ws)+ equals + opt_ws + Optional(value))
    cppflagsAssign.addParseAction(parse_assign)
    cppflags=target_flags('CPPFLAGS')
    cppflagsAssign.addParseAction(cppflags.parse)

    cxxflagsAssign = Group(lowercase_name_+'CXXFLAGS' + Suppress(opt_ws)+ equals + opt_ws + Optional(value))
    cxxflags=target_flags('CXXFLAGS')
    cxxflagsAssign.addParseAction(parse_assign)
    cxxflagsAssign.addParseAction(cxxflags.parse)

    ldflagsAssign = Group(lowercase_name_+'LDFLAGS' + Suppress(opt_ws)+ equals + opt_ws + Optional(value))
    ldflagsAssign.addParseAction(parse_assign)
    ldflags=target_flags('LDFLAGS')
    ldflagsAssign.addParseAction(ldflags.parse)

    ldaddAssign = Group(lowercase_name_+'LDADD' + Suppress(opt_ws)+ equals + opt_ws + Optional(value))
    ldaddAssign.addParseAction(parse_assign)
    ldadd=target_flags('LDADD')
    ldaddAssign.addParseAction(ldadd.parse)

    libsAssign = Group(lowercase_name_+'LIBS' + Suppress(opt_ws)+ equals + opt_ws + Optional(value))
    libsAssign.addParseAction(parse_assign)
    libs=target_flags('LIBS')
    libsAssign.addParseAction(libs.parse)

    
    

    # Process dir_SOURCES assignment
    sourcesAssign = Group(lowercase_name_+Suppress(Literal("SOURCES"))+Suppress(opt_ws)+ equals + Suppress(opt_ws) + Optional(value))
    sourcesAssign.setParseAction(parse_sources)

    # Process <dir>_DATA assignment
    data_install = installer("DATA")
    dataAssign = data_install.create_parser(lowercase_name_, opt_ws, equals, value)
    #dataAssign.setDebug()
    dataAssign.addParseAction(data_install.parse)
    dataAssign.addParseAction(parse_assign)
    #dataAssign.addParseAction(lambda tokens: data_install.parse(tokens))

    # Process check_Programs but never install them
    check_programs = installer("check_PROGRAMS")
    check_programsAssign = check_programs.create_parser(lowercase_name_, opt_ws, equals, value)

    # Process <dir>_PROGRAMS assignment
    programs_install = installer("PROGRAMS")
    programsAssign = programs_install.create_parser(lowercase_name_, opt_ws, equals, value)
    programsAssign.addParseAction(programs_install.parse)
    programsAssign.addParseAction(parse_assign)

    # Process <dir>_LIBRARIES assignment
    libraries_install = installer("LIBRARIES")
    librariesAssign = libraries_install.create_parser(lowercase_name_, opt_ws, equals, value)
    librariesAssign.addParseAction(libraries_install.parse)
    librariesAssign.addParseAction(parse_assign)

    # Process <dir>_SCRIPTS assignment
    scripts_install = installer("SCRIPTS")
    scriptsAssign = scripts_install.create_parser(lowercase_name_, opt_ws, equals, value)
    scriptsAssign.addParseAction(scripts_install.parse)
    scriptsAssign.addParseAction(parse_assign)

    # Process <dir>_HEADERS assignment
    headers_install = installer("HEADERS")
    headersAssign = headers_install.create_parser(lowercase_name_, opt_ws, equals, value)
    headersAssign.addParseAction(headers_install.parse)
    headersAssign.addParseAction(parse_assign)

    # SUBDIRS
    subdirsAssign = Group(Combine(Literal("SUBDIRS"))+Suppress(opt_ws)+ equals + opt_ws + Optional(value))
    subdirsAssign.setParseAction(parse_assign)
    subdirs= rem_proc("SUBDIRS")
    subdirsAssign.addParseAction(subdirs.parse)

    testsAssign = Group(Combine(Literal("TESTS"))+Suppress(opt_ws)+ equals + opt_ws + Optional(value))
    #testsAssign.setDebug()
    testsAssign.setParseAction(parse_assign)
    tests=rem_proc("TESTS")
    testsAssign.addParseAction(tests.parse)

    fail_testsAssign = Group(Combine(Literal("XFAIL_TESTS"))+Suppress(opt_ws)+ equals + opt_ws + Optional(value))
    fail_testsAssign.setParseAction(parse_assign)
    fail_tests=rem_proc("XFAIL_TESTS")
    fail_testsAssign.addParseAction(fail_tests.parse)

    compile_fail_testsAssign = Group(Combine(Literal("COMPILE_XFAIL_TESTS"))+Suppress(opt_ws)+ equals + opt_ws + Optional(value))
    compile_fail_testsAssign.setParseAction(parse_assign)
    compile_fail_tests=rem_proc("COMPILE_XFAIL_TESTS")
    compile_fail_testsAssign.addParseAction(compile_fail_tests.parse)

    # Process include lines and just comment them out.
    includeLine = opt_ws.suppress() + Group(Literal('include') + restOfLine )
    includeLine.setParseAction(parse_include)
    #includeLine.setDebug()
    rule_line = multiple_words+Combine(Optional(Literal("\\")),lineEnd)
    dependency_rule= Group(opt_ws+word+opt_ws+Suppress(Literal(':'))+ opt_ws+multiple_words)
    dependency_rule.setParseAction(parse_dependency)

    easy_rule = Group(opt_ws+word+opt_ws+Literal(':')+ opt_ws+Optional(word_line)+opt_ws+
                      ZeroOrMore(Or(NL,continuation)+ws+Optional(word_line)+opt_ws)+NL)
    easy_rule.setParseAction(comment_lines)
    #easy_rule.setDebug()

    lines << ZeroOrMore(opt_ws+(dependency_rule+NL|easy_rule|(sourcesAssign|dataAssign|check_programsAssign|programsAssign|librariesAssign|scriptsAssign|headersAssign|subdirsAssign|compile_fail_testsAssign|fail_testsAssign|testsAssign|dirAssign|cppflagsAssign|cxxflagsAssign|ldaddAssign|ldflagsAssign|libsAssign|varAddAssign|docsAssign|varAssign|comment|ifstatement|includeLine|Empty())+NL))

    s= ''.join(lines.parseString(amstring)._asStringList())
    #print s
    s=s+data_install.process()
    s=s+programs_install.process()
    s=s+libraries_install.process()
    s=s+scripts_install.process()
    s=s+headers_install.process()
    s=s+subdirs.process()

    s+=tests.process()
    s+=fail_tests.process()
    s+=compile_fail_tests.process()
    s+=dirParser.process()
    if tests.found or compile_fail_tests.found:
        s+=''.join(['\n\n# We do not want want to build the tests during make all\n',
                    '# but just build them on demand\n',
                    'add_directory_test_target(_test_target)\n',
                    'add_dependencies(${_test_target} ${TESTS} ${COMPILE_XFAIL_TESTS})'])
    if docs_present.found:
        s=s+''.join(['\n# Install documentation',
                     '\n# We assume that ${DOCFILES} and ${DOCFILES_EXTRA} are targets\n',
                     'install(TARGETS ${DOCFILES} ${DOCFILES_EXTRA} DESTINATION ${docdir})\n'])

    return s

def init_cmake_module(module_name):
    return''.join(['# set up project\n',
                  'project("'+module_name+'" C CXX)\n\n#circumvent not building docs\nset(BUILD_DOCS 1)\n\n',
                  '# general stuff\n',
                  'cmake_minimum_required(VERSION 2.8.6)\n\n',
                  '#find dune-common and set the module path\n',
                  'find_package(dune-common)\n'
                  'list(APPEND CMAKE_MODULE_PATH ${dune-common_MODULE_PATH}\n'
                  '     "${CMAKE_SOURCE_DIR}/cmake/modules")# make sure our own modules are found\n\n',
                  '#include the dune macros\n'
                  'include(DuneMacros)\n\n',
                  '# start a dune project with information from dune.module\n',
                  'dune_project()\n\n'])

def finalize_cmake_module(module_name):
    upper_name =re.sub('-(\S)', lambda m: m.group(1).capitalize(),
                       module_name.capitalize())
    name_wo_dasch=module_name.replace('-', '')
    lines = ['\nadd_subdirectory(cmake/modules)\n',
             '# finalize the dune project, e.g., generate config.h etc.\n'
             'finalize_dune_project(GENERATE_CONFIG_H_CMAKE)\n']
    return ''.join(lines)

def create_cmake_dirs_and_file(dirname, module_name):
    upper_name =re.sub('-(\S)', lambda m: m.group(1).capitalize(),
                       module_name.capitalize())
    print 'module_name %s' % module_name
    print 'upper_name %s' % upper_name
    dirs={'modules': os.path.join(dirname, 'cmake', 'modules')}
    cdirs=[]
    for dir in dirs.values():
        try:
            print "Creating %s" % dir
            os.makedirs(dir)
        except OSError as exception:
            if exception.errno != errno.EEXIST:
                raise
    text="message(AUTHOR_WARNING \"TODO: Implement module test.\")"
    output=open(os.path.join(dirs['modules'], upper_name+'Macros.cmake'), 'w')
    output.write(text)
    output.close()
    # CMakeLists.txt in module directory
    # list files *.cmake
    all_cmake_files=[]
    for root, dirnames, filenames in os.walk(dirs['modules']):
        all_cmake_files.extend(fnmatch.filter(filenames,'*.cmake')[:])
    lines=['set(modules \n  ', '\n  '.join(all_cmake_files), '\n)\n']
    output=open(os.path.join(dirs['modules'], 'CMakeLists.txt'), 'w')
    lines.extend(['install(FILES "${modules}" DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/modules)\n'])
    output.write(''.join(lines))
    output.close()


def am_2_cmake(amfile, cmakefile, module_root=False):
    output =  open(cmakefile, 'w')
    if module_root:
        dirname=os.path.dirname(amfile)
        dune_module_path = os.path.join(dirname, 'dune.module')
        module_file = open(dune_module_path, 'r')
        lines = module_file.readlines()
        print ''.join(lines)
        m=re.search(r'.*Module:[ \t]*(\S+)[ \t]*[\n]?.*', ''.join(lines))
        if not m:
            raise Exception("Could not find module name in dune.module file.")
        module_file.close()
        module_name=m.group(1)
        print 'Module name is %s'% module_name
        output.write(init_cmake_module(module_name))
        create_cmake_dirs_and_file(dirname, module_name)
    input = open(amfile, 'r')
    lines = input.readlines()
    str = ''.join(lines)
    output.write(re.sub('\s+\n', '\n', am_2_cmake_string(str)))
    input.close()
    if module_root:
        output.write(finalize_cmake_module(module_name))
    output.close()

def am_2_cmake_dir(directory):
    for root, dirnames, filenames in os.walk(directory):
        for filename in fnmatch.filter(filenames, 'Makefile.am'):
            print "root=%s" % root
            print "filename=%s" % filename
            amfile=os.path.join(root, filename)
            cmfile=os.path.join(root, 'CMakeLists.txt')
            is_module_root = len(fnmatch.filter(filenames, 'dune.module'))
            
            print ''.join(['Converting ', amfile, ' -> ', cmfile])
            print is_module_root
            am_2_cmake(amfile, cmfile, is_module_root)
    
    # Add doxygen target
    doxygendir=os.path.join(directory, 'doc', 'doxygen')
    if os.path.isdir(doxygendir) and os.path.exists(os.path.join(doxygendir, 'Doxylocal')):
        output=open(os.path.join(doxygendir, 'CMakeLists.txt'),'a')
        output.write('\n'.join(['# Create Doxyfile.in and Doxyfile, and doxygen documentation',
'add_doxygen_target()']))
        output.close()
    # Add directives to create CMake packe configuration files with autotools
    output=open("Makefile.am'", "a")
    output.write('\n'.join(['', '# Generate package configuration files for finding',
                            '# installed modules with CMake',
                            'include $(top_srcdir)/am/cmake-pkg-config\n']))
    output.close()

def main():
    usage = "usage: am2cmake "
    parser = OptionParser(usage=usage)
    parser.add_option("-a", "--am-file", dest="amfile",
                      help="The Makefile.am", metavar="FILE")
    parser.add_option("-c", "--cmake-file",
                      dest="cmakefile", 
                      help="The CMake file")
    parser.add_option("-d", "--directory",
                      dest="directory",
                      help="Root directory of dune-module")
    (options, args) = parser.parse_args()
    if not options.amfile and not options.directory:
        parser.error("options -a or option -d has to be present")
    if options.directory and (options.amfile or options.cmakefile):
        parser.error("Cannot provide a directory and a CMake or am file.")
    if not options.cmakefile and not options.directory:
        options.cmakefile = options.amfile.replace('Makefile.am', 'CMakeLists.txt')
    
    #import pdb; pdb.set_trace()
    if options.directory:
        am_2_cmake_dir(options.directory)
    else:
        print ''.join([options.amfile, '->', options.cmakefile])
        am_2_cmake(options.amfile, options.cmakefile)
if __name__ == "__main__":
    main()
