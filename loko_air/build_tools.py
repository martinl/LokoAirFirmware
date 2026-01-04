

from pathlib import Path
import subprocess
import threading
import time
import sys
import os
from tools.version import version_tools

BUILD_DIR = 'build'
BIN_DIR = BUILD_DIR + '/out'
TEST_BUILD_DIR = BUILD_DIR + '/tests'
SRC_DIR = '.'
RELEASE_DIR = 'release/public'
MANUFACTURE_DIR = 'release/manufacture'
CMAKE_TOOLCHAIN_FILE = 'cmake/gcc_tools.cmake'
PROJ_VERSION_FILE = f'{SRC_DIR}/Core/Inc/version.h'
KEY_FILE = f'{SRC_DIR}/src/inc/bldr_app_key.h'

TARGETS = [
    {'name': 'loko', 'need_bldr': True, 'app_offset': 32*1024, 'encrypt_app': False},
    {'name': 'loko_e5', 'need_bldr': True, 'app_offset': 32*1024, 'encrypt_app': False},
]


verbose = False  # False  # True


def executer(*args):
    exec_str = ' '.join(args)
    if verbose:
        print("Execute: " + exec_str)
    return os.system(exec_str)

def copy_file(source_path, destination_path):
    print(f"Copying file from {source_path} to {destination_path}")
    try:
        # Open the source file in read-binary mode and destination file in write-binary mode
        with open(source_path, 'rb') as source_file:
            with open(destination_path, 'wb') as destination_file:
                # Read and write in chunks to handle large files efficiently
                while True:
                    chunk = source_file.read(1024)
                    if not chunk:
                        break
                    destination_file.write(chunk)
        print(f"File copied from {source_path} to {destination_path}")
    except FileNotFoundError:
        print(f"The file at {source_path} does not exist.")
    except PermissionError:
        print(f"Permission denied to copy the file to {destination_path}.")
    except Exception as e:
        print(f"An error occurred: {e}")

def check_gcc():
    gcc_path = os.getenv('GCC_PATH')
    if gcc_path is None:
        print("\tError! GCC_PATH not found!")
        print("\tPlease set correct GCC_PATH environment variable")
        return False

    print("\tGCC_PATH set to:", gcc_path)
    global GCC_PATH
    GCC_PATH = gcc_path

    #Add GCC bin to PATH
    os.environ['PATH'] = os.getenv('PATH')  + os.pathsep + gcc_path + '/bin' + os.pathsep

    return True


def get_app_bin_path(target):
    return f'{BIN_DIR}/{target["name"]}_app.bin'

def get_app_hex_path(target):
    return f'{BIN_DIR}/{target["name"]}_app.hex'


def get_bldr_bin_path(target):
    return f'{BIN_DIR}/{target["name"]}_bldr.bin'


def rename_and_copy_app_files(version_str):
    # loko_e5_1.294.hex
    executer('mkdir release')
    executer(f'mkdir {RELEASE_DIR}')
    for target in TARGETS:
        target_name = target['name']
        target_file = get_app_hex_path(target)
        new_name = f'{RELEASE_DIR}/{target_name}_{version_str}.hex'
        if target['encrypt_app'] == True:
            target_file = target_file.replace('.bin', '.ebf')
            new_name = new_name.replace('.bin', '.ebf')

        print('Rename: ' + target_file + ' to ' + new_name)
        os.rename(target_file, new_name)


def run_cmake_make_build():

    result = executer(f'cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE={CMAKE_TOOLCHAIN_FILE} -DTOOLCHAIN_PREFIX=\"{GCC_PATH}\"'
                      f' -S {SRC_DIR} -B {BUILD_DIR} -G\"Ninja\"')
    if result != 0:
        return 1

    result = executer(f'cmake --build {BUILD_DIR} --config Release --target all')
    if result != 0:
        return 1

    return 0


def run_test_build():

    result = executer(f'cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_CONFIG_UNIT_TESTS=1'
                      f' -S {SRC_DIR} -B {BUILD_DIR} -G\"Ninja\"')
    if result != 0:
        return 1

    result = executer(f'cmake --build {BUILD_DIR} --config Release --target generate-coverage')
    if result != 0:
        return 1

    return 0


def get_fw_version():

    return version_tools.get_versions(PROJ_VERSION_FILE)


def set_fw_version(major, minor):

    version_tools.set_versions(PROJ_VERSION_FILE, major, minor)


def add_crc_to_app():
    for target in TARGETS:
        app_path = get_app_bin_path(target)
        executer(
            f'python ./tools/crc_recorder/crc_recorder.py -i {app_path}')


def make_full_images(version_str):
    executer('mkdir release')
    executer(f'mkdir {MANUFACTURE_DIR}')
    for target in TARGETS:
        if target['need_bldr'] == True:
            app_path = get_app_bin_path(target)
            bldr_path = get_bldr_bin_path(target)
            full_image_path = f'{BUILD_DIR}/out/{target["name"]}_full_image_{version_str}.bin'
            app_offset = target['app_offset']
            executer(
                f'python ./tools/bin_concat/bin_concat.py -i {bldr_path}@0 {app_path}@{app_offset} -o {full_image_path} -p FF')
            copy_file(full_image_path, f'{MANUFACTURE_DIR}/{target["name"]}_full_image_{version_str}.bin')

def make_ebf():

    for target in TARGETS:
        if target['encrypt_app'] == True:
            app_path = get_app_bin_path(target)
            app_ebf_path = app_path.replace('.bin', '.ebf')
            executer(
                f'python ./tools/fw_encryptor/fw_encryptor.py -in {app_path} -out {app_ebf_path} -k {KEY_FILE}')


def make_c_array_from_bootloader():

    for target in TARGETS:
        if target['need_bldr'] == True:
            bldr_path = get_bldr_bin_path(target)
            bldr_c_file = f'{SRC_DIR}/src/bldr_utils/bootloader_{target["name"]}.c'
            executer(
                f'python ./tools/bin2hex/bin2hex.py -i {bldr_path} -o{bldr_c_file}')


def remove_artifact_folder():
    result = executer(f'rm -rf {BUILD_DIR}')
    if result != 0:
        return result

    result = executer(f'rm -rf {RELEASE_DIR}')
    if result != 0:
        return result

    return 0


def code_format():
    sources_root = SRC_DIR+ '/Core'
    for root, dirs, files in os.walk(sources_root):
        for dir_name in dirs:
            subfolder_path = os.path.join(root, dir_name)
            if '3rd_party'not  in subfolder_path:
                print(subfolder_path)
                executer(f'clang-format --verbose -style=file:.clang-format -i {subfolder_path}/*.c {subfolder_path}/*.h' )
    executer(f'clang-format --verbose -style=file:.clang-format -i {sources_root}/*.c {sources_root}/*.h' )


def code_clean(root_folder_path):
    header_filter = ""#-header-filter=.*"
    for root, dirs, files in os.walk(root_folder_path):
        for dir_name in dirs:
            subfolder_path = os.path.join(root, dir_name)
            print(subfolder_path)
            executer(f'clang-tidy -p {BUILD_DIR} {header_filter} -fix -fix-errors {subfolder_path}/*.c {subfolder_path}/*.h' )
    executer(f'clang-tidy -p {BUILD_DIR} {header_filter} -fix -fix-errors {root_folder_path}/*.c {root_folder_path}/*.h' )


def generate_release_notes():
    executer('cz changelog')

    return 0


def clean():
    executer(f'rm -rf {BUILD_DIR}')
    executer(f'rm -rf {RELEASE_DIR}')

    return 0

def is_repo_clean():
    return executer('git diff-index --quiet HEAD --') == 0

def inc_build_number():
    ver_major, ver_minor = get_fw_version()
    ver_minor = str(int(ver_minor) + 1)
    set_fw_version(ver_major, ver_minor)
    version_str = f"v{ver_major}.{ver_minor}"
    print(f"New version: {version_str}")

    return 0

def commit_changes(commit_msg = None):
    ver_major, ver_minor = get_fw_version()
    version_str = f'v{ver_major}.{ver_minor}'
    executer('git add .')
    if not commit_msg:
        executer(f'git commit -am \"[BUILD] {version_str}\"')
        return 1
    return executer(f'git commit -am \"{commit_msg}\"')


def stage_release_preparation(link_bootloader):
    clean()
    inc_build_number()
    generate_release_notes()
    code_format()
    commit_changes()
    return build_firmware(link_bootloader)


def stage_release(link_bootloader):
    clean()
    return build_firmware(link_bootloader)


def build_firmware(link_bootloader):

    print('Checking GCC...')
    if check_gcc() == False:
        print('Can\'t find GCC')
        return 1


    ver_major, ver_minor = get_fw_version()
    version_str = f"v{ver_major}.{ver_minor}"
    print(f"Current fw version: {version_str}")

    remove_artifact_folder()

    print("Build all...")
    result = run_cmake_make_build()
    if result != 0:
        return result

    if link_bootloader == True:
        print("Generate c-array and include it to code...")
        make_c_array_from_bootloader()

        print("Build app with new bootloader...")
        result = run_cmake_make_build()
        if result != 0:
            return result

    print("Add CRC to application binaries...")
    add_crc_to_app()

    print("Make full image binaries...")
    make_full_images(version_str)

    # print("Make EBF files...")
    # make_ebf()

    print(f"Add version({version_str}) to name...")
    rename_and_copy_app_files(version_str)

    return 0


def build_tests():
    remove_artifact_folder()
    return run_test_build()


def main():
    link_bootloader = False  # False  # True
    print("-=Build tools=-")

    for i in range(1, len(sys.argv)):

        if sys.argv[i] == '--link-bootloader':
            link_bootloader = True
            print("Link bootloader enabled")
        elif sys.argv[i] == '--verbose':
            verbose = True
            print("Verbose enabled")

    for i in range(1, len(sys.argv)):
        print('argument:', i, 'value:', sys.argv[i])
        if sys.argv[i] == '--release-preparation':
            return stage_release_preparation(link_bootloader)
        elif sys.argv[i] == '--release':
            return stage_release(link_bootloader)
        elif sys.argv[i] == '--release-notes':
            return generate_release_notes()
        elif sys.argv[i] == '--code-format':
            return code_format()
        elif sys.argv[i] == '--code-clean':
            return code_clean(sys.argv[i + 1])
        elif sys.argv[i] == '--is-repo-clean':
            return is_repo_clean()
        elif sys.argv[i] == '--commit-changes':
            return commit_changes()
        elif sys.argv[i] == '--inc-build-number':
            return inc_build_number()
        elif sys.argv[i] == '--tests':
            return build_tests()

    return 1

if __name__ == '__main__':
    exit(main())
