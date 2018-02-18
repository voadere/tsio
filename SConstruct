Decider('MD5-timestamp')

DEBUG = ARGUMENTS.get('DEBUG', '0')
STD = ARGUMENTS.get('STD', '11')

gcc = Environment(CC='gcc',
                  CXX='g++')

if DEBUG == '1':
    gcc.Append(CCFLAGS=' -g')
else:
    gcc.Append(CCFLAGS=' -O3 -fomit-frame-pointer -Wall -Wmissing-declarations -Wsign-compare -Wconversion -Wno-sign-conversion -Wold-style-cast -Wno-parentheses')


clang = Environment(CC='clang',
                    CXX='clang++')


if DEBUG == '1':
    clang.Append(CCFLAGS=' -g')
else:
    clang.Append(CCFLAGS=' -O3 -fomit-frame-pointer -Wall -Wmissing-declarations -Wsign-compare -Wconversion -Wno-sign-conversion -Wold-style-cast -Wno-parentheses')

compiler=clang

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

