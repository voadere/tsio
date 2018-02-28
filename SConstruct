Decider('MD5-timestamp')

Help("""
Compiling the tsio package.
""")

DEBUG = ARGUMENTS.get('DEBUG', '0')
Help("    DEBUG=[1/0]          enable/disable debugging.  Default = 0.\n")

STD = ARGUMENTS.get('STD', '11')
Help("    STD=[11/14/17]       set c++ standard in compiles.  Default = 11.\n")

COMPILER = ARGUMENTS.get('COMPILER', 'gcc')
Help("    COMPILER=[gcc/clang] set compiler type.  Default = gcc.\n")

COVERAGE = ARGUMENTS.get('COVERAGE', '0')
Help("    COVERAGE=[1/0]       enable coverage. Default is 0 (coverage disabled).\n")

gcc = Environment(CC='gcc',
                  CXX='g++')

clang = Environment(CC='clang',
                    CXX='clang++')

if DEBUG == '1':
    gcc.Append(CCFLAGS=' -g')
    clang.Append(CCFLAGS=' -g')
else:
    gcc.Append(CCFLAGS=' -O3 -fomit-frame-pointer -Wall -Wmissing-declarations -Wsign-compare -Wconversion -Wno-sign-conversion -Wold-style-cast -Wno-parentheses -Wno-attributes')
    clang.Append(CCFLAGS=' -O3 -fomit-frame-pointer -Wall -Wmissing-declarations -Wsign-compare -Wconversion -Wno-sign-conversion -Wold-style-cast -Wno-parentheses -Wno-missing-braces')


if COVERAGE == '1':
    gcc.Append(CCFLAGS=' -fprofile-arcs -ftest-coverage')
    gcc.Append(LINKFLAGS=' -fprofile-arcs -ftest-coverage')
    clang.Append(CCFLAGS=' -fprofile-instr-generate -fcoverage-mapping')
    clang.Append(LINKFLAGS=' -fprofile-instr-generate -fcoverage-mapping')

if COMPILER == 'clang':
    compiler=clang
else:
    compiler=gcc

if STD == '17':
   compiler.Append(CCFLAGS='-std=c++17')
else:
   compiler.Append(CCFLAGS='-std=c++11')

libources=['tsio.cpp']

tsiolib=compiler.Library('tsio', libources)

sources=['tsioTest.cpp']

compiler.Program('tsioTest', sources,
         LIBS=['tsio'], LIBPATH=['.'],
         CPPPATH=['.'])

