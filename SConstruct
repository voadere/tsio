Decider('MD5-timestamp')

DEBUG = ARGUMENTS.get('DEBUG', '0')

gcc = Environment(CC='gcc',
                  CXX='g++',
                  CCFLAGS='-std=c++17')

if DEBUG == '1':
    gcc.Append(CCFLAGS=' -g')
else:
    gcc.Append(CCFLAGS=' -O3 -fomit-frame-pointer -Wall -Wmissing-declarations -Wsign-compare -Wconversion -Wno-sign-conversion -Wold-style-cast -Wno-parentheses')

clang = Environment(CC='clang',
                    CXX='clang++',
                    CCFLAGS='-std=c++17')

if DEBUG == '1':
    clang.Append(CCFLAGS=' -g')
else:
    clang.Append(CCFLAGS=' -O3 -fomit-frame-pointer -Wall -Wmissing-declarations -Wsign-compare -Wconversion -Wno-sign-conversion -Wold-style-cast -Wno-parentheses')

compiler=clang

libources=['tsio.cpp']

tsiolib=compiler.Library('tsio', libources)

sources=['tsioTest.cpp']

compiler.Program('tsioTest', sources,
         LIBS=['tsio'], LIBPATH=['.'],
         CPPPATH=['.'])

