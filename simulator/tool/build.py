import glob
import os
import sys
import subprocess
import socket
from concurrent.futures import ThreadPoolExecutor
from concurrent.futures import as_completed

VERBOSE = 0
SKIP_STRIP = 1

SRC_PATH = ''
INCLUDE_PATH = '/../include/mu'
OUT_PATH = SRC_PATH + '/out'
DUMP_PATH = OUT_PATH + '/dump'

TARGET = '-rel-p3'
BUILD_PATH = str(os.path.dirname(os.path.abspath(__file__))) + '/mu_compiler/'
BIN_PATH = f'{BUILD_PATH}llvm{TARGET}/bin/'
MU_LINKER_FILE = INCLUDE_PATH + '/mu_linker.ld'
MS_LINKER_FILE = INCLUDE_PATH + '/ms_linker.ld'
TOOL_PATH = SRC_PATH + '/../lib/src'

OPTIMIZE = '-O3'

# https://llvm.org/docs/LinkTimeOptimization.html

BUILD_CPP = BIN_PATH + f'clang -march=metis+m -L{BUILD_PATH}llvm-builtin{TARGET} -lclang_rt.builtins-metis \
        -ffreestanding -nostdlib -nodefaultlibs -fno-exceptions -z max-page-size=4 \
        --ld-path={BUILD_PATH}llvm{TARGET}/bin/ld.lld \
        -Werror=return-type -I{BUILD_PATH}llvm-libc{TARGET} -I{BUILD_PATH}llvm-libc{TARGET}/include -L{BUILD_PATH}llvm-libc{TARGET}/newlib -lc \
        {OPTIMIZE} -g'
#-flto

LITE_BUILD_CPP = BIN_PATH + f'clang -march=metis+m -L{BUILD_PATH}llvm-builtin{TARGET} -lclang_rt.builtins-metis \
        -ffreestanding -nostdlib -nodefaultlibs -fno-exceptions -z max-page-size=4 \
        --ld-path={BUILD_PATH}llvm{TARGET}/bin/ld.lld \
        -Werror=return-type -I{BUILD_PATH}llvm-libc{TARGET} -I{BUILD_PATH}llvm-libc{TARGET}/include -L{BUILD_PATH}llvm-libc{TARGET}/newlib -lc \
        {OPTIMIZE} -g'

BUILD_LIB = BIN_PATH + f'clang --target=metis-unknown-elf -march=metis+m {OPTIMIZE} -g  \
        -ffreestanding -nostdlib -nodefaultlibs -L{BUILD_PATH}llvm-builtin{TARGET} -lclang_rt.builtins-metis \
        --ld-path={BUILD_PATH}llvm{TARGET}/bin/ld.lld \
        -Wall -Wno-unused-parameter -fPIC \
        -Werror=return-type'


LIB_ACHIVER = BIN_PATH + 'llvm-ar rc'

DUMP_CPP = BIN_PATH + 'llvm-objdump --mattr=+m -d -S -l' # For Debugging
DUMP2_CPP = BIN_PATH + 'llvm-objdump --mattr=+m -t -s' # For Section
STIRIP_CPP = BIN_PATH + 'llvm-strip -s'


MASTER_DEFINITION_LIST = [
    '_MU_',
    '_MASTER_'
]

SLAVE_DEFINITION_LIST = [
    '_MU_',
    '_SLAVE_'
]

ADMIN_DEFINITION_LIST = [
    '_MU_',
    '_ADMIN_'
]

LIB_SOURCE_LIST = [
    '../include/mu/mu_loader.cpp',
    '../include/mu/mu_printf.cpp',
    '../include/mu/mu_alloc.cpp',
]
INCLUDE_LIST = [
    '../include/mu',
    '../include/sys'
]

def Command(command):
    if (VERBOSE >= 1):
        print(command)
    #proc = subprocess.Popen(command, stdout=subprocess.PIPE, shell=True)
    #(out, err) = proc.communicate()
    exitcode, out = subprocess.getstatusoutput(command)
    if (exitcode != 0):
        print(out)

    return out, exitcode


# Create Folder
def CreateFolder():
    if (os.path.isdir(OUT_PATH) == False):
        os.mkdir(OUT_PATH)

    if (os.path.isdir(DUMP_PATH) == False):
        os.mkdir(DUMP_PATH)

def deleteObjFiles():
    for file in glob.glob(OUT_PATH + '/*.o'):
        os.remove(file)

def deleteOldFiles():
    deleteObjFiles()

    for file in glob.glob(OUT_PATH + '/*.os'):
        os.remove(file)

    for file in glob.glob(DUMP_PATH + '/*.*'):
        os.remove(file)

# search
def SearchCppFiles(sourcePath):
    global SRC_PATH
    global INCLUDE_PATH
    global OUT_PATH
    global DUMP_PATH
    global MU_LINKER_FILE
    global MS_LINKER_FILE
    global TOOL_PATH
    SRC_PATH = sourcePath + SRC_PATH
    INCLUDE_PATH = sourcePath + INCLUDE_PATH
    OUT_PATH = sourcePath + OUT_PATH
    DUMP_PATH = sourcePath + DUMP_PATH
    MU_LINKER_FILE = sourcePath + MU_LINKER_FILE
    MS_LINKER_FILE = sourcePath + MS_LINKER_FILE
    TOOL_PATH = sourcePath + TOOL_PATH

    cppfiles = []
    for file in glob.glob(SRC_PATH + "/*.cpp"):
        cppfiles.append([file, "slave"])

    for file in glob.glob(SRC_PATH + "/master/*.cpp"):
        cppfiles.append([file, "master"])

    for file in glob.glob(SRC_PATH + "/slave/*.cpp"):
        cppfiles.append([file, "slave"])

    for file in glob.glob(SRC_PATH + "/admin/*.cpp"):
        cppfiles.append([file, "admin"])

    for file in glob.glob(SRC_PATH + "/test/*.cpp"):
        fileName = file[file.rfind('/')+1:]
        cppfiles.append([file, fileName[:fileName.find('_')]])

    return cppfiles

"""
def BuildLibrary():
    lib_source_list_str = ' '.join(list(map(lambda path: SRC_PATH + '/' + path , LIB_SOURCE_LIST)))

    include_list_str = ' '.join(list(map(lambda path: '-I ' + SRC_PATH + '/' + path , INCLUDE_LIST)))

    compile_list = [
            BUILD_LIB,
            ' -T ' + LINKER_FILE,
            include_list_str,
            lib_source_list_str,
            ' -o ' + OUT_PATH + '/' + 'libmu.o'
    ]
    data, result = Command(' '.join(compile_list))

    if (result == 0):
            achiver_cmd_list = [
                LIB_ACHIVER,
                OUT_PATH + '/' + 'libmu.a',
                OUT_PATH + '/' + 'libmu.o'
            ]
            Command(' '.join(achiver_cmd_list))
"""

def compile_file(cppfileInfo):
    cppfile = cppfileInfo[0]
    type = cppfileInfo[1]

    filename = cppfile[cppfile.rfind('/') + 1 : cppfile.rfind('.') ]
    out_filename = OUT_PATH + '/' + filename + '.o'

    build_cpp = LITE_BUILD_CPP
    if (type == "slave"):
        startup_file = INCLUDE_PATH + '/mu_startup.s'
        definition_list_str = ' '.join(list(map(lambda define: '-D' +define , SLAVE_DEFINITION_LIST)))
        build_cpp = BUILD_CPP
        linker_file = ' -T ' + MU_LINKER_FILE
    elif (type == "master"):
        startup_file = SRC_PATH + '/master/' + "master_mu_startup.s"
        definition_list_str = ' '.join(list(map(lambda define: ' -D' +define , MASTER_DEFINITION_LIST)))
        linker_file = ' -T ' + MS_LINKER_FILE
    else:
        startup_file = SRC_PATH + '/admin/' + "admin_mu_startup.s"
        definition_list_str = ' '.join(list(map(lambda define: ' -D' +define , ADMIN_DEFINITION_LIST)))
        linker_file = ' -T ' + MS_LINKER_FILE

    include_list_str = ' '.join(list(map(lambda path: '-I ' + SRC_PATH + '/' + path , INCLUDE_LIST)))
    #lib_path = ' -l' + 'mu' + ' -L ' + OUT_PATH
    source_list_str = ' '.join(list(map(lambda path: SRC_PATH + '/' + path , LIB_SOURCE_LIST)))

    if (SKIP_STRIP):
        out_filename += 's'

    print('== [ Build ' + cppfile + '  ] == ')
    compile_list = [
        build_cpp,
        linker_file,
        startup_file,
        cppfile,
        definition_list_str,
        #lib_path,
        source_list_str,
        include_list_str,
        ' -o '+ out_filename
    ]

    compile_cmd = ' '.join(compile_list)

    data, result = Command(compile_cmd)
    if (result == 0):
        strip_filename = out_filename + 's'
        if (SKIP_STRIP):
            strip_filename = out_filename
        else:
            strip_cmd = STIRIP_CPP + ' ' + out_filename + ' -o ' + strip_filename
            Command(strip_cmd)

        dump_filename = DUMP_PATH + '/' + filename
        dump_cmd = DUMP_CPP + ' ' + strip_filename + ' > ' + dump_filename + '_debug.txt'
        Command(dump_cmd)

        dump_cmd2 = DUMP2_CPP + ' ' + strip_filename + ' > ' + dump_filename + '_sec.txt'
        Command(dump_cmd2)
    else:
        exit(1)

def Build(cppfiles):
    CreateFolder()
    deleteOldFiles()
    #BuildLibrary()

    # build
    with ThreadPoolExecutor(max_workers = 16) as executor:
      results = executor.map(compile_file, cppfiles)

def main():
    if (len(sys.argv) != 2):
        print('Need src path')
        exit(1)

    soucre_path = sys.argv[1]
    cppfiles = SearchCppFiles(soucre_path)
    if (len(cppfiles) == 0):
        print('path : ' + os.getcwd())
        print("Not found .cpp files")
        exit(1)

    Build(cppfiles)
    deleteObjFiles()

if __name__ == "__main__":
	main()