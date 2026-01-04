import argparse
import sys
import os


def set_versions(version_file, major_version, minor_version):
    with open(version_file, "r") as f:
        d = f.readlines()

    with open(version_file, "w") as f:
        for i in d:
            if i.find("FIRMWARE_VERSION_MAJOR") != -1:
                f.write("#define FIRMWARE_VERSION_MAJOR              {}\n".format(
                    major_version))
            elif i.find("FIRMWARE_VERSION_MINOR") != -1:
                f.write("#define FIRMWARE_VERSION_MINOR              {}\n".format(
                    minor_version))
            else:
                f.write(i)


def get_versions(version_file):
    major_ver = 0
    minor_ver = 0

    with open(version_file, "r") as f:
        d = f.readlines()

        for i in d:
            if i.find("FIRMWARE_VERSION_MAJOR") != -1:
                for s in i.split():
                    if s.isdigit():
                        major_ver = s
            if i.find("FIRMWARE_VERSION_MINOR") != -1:
                for s in i.split():
                    if s.isdigit():
                        minor_ver = s

    return [major_ver, minor_ver]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--get', dest='get',
                        action='store_true', help='get major')
    parser.add_argument('--set', dest='set',
                        action='store_true', help='set major')
    parser.add_argument('--inc', dest='inc', action='store_true',
                        help='increment minor number')
    parser.add_argument('-major', '--major_version', nargs='?',
                        const=-1, type=int, help='major version')
    parser.add_argument('-minor', '--minor_version', nargs='?',
                        const=-1, type=int, help='minor version')
    parser.add_argument('-file', '--version_file', help='version file')

    args = parser.parse_args()

    is_get = args.get
    is_set = args.set
    is_inc = args.inc
    major_version = args.major_version
    minor_version = args.minor_version
    version_file = args.version_file

    if (is_set is None) and (is_get is None):
        print("[Version tools] Please chose action: get, set or inc")

    if (major_version is None) and (minor_version is None) and (version_file is None):
        print("[Version tools] Please chose major or minor")
        exit(-1)

    if version_file is None:
        print("[Version tools] File is not set")
        exit(-1)

    if not os.path.isfile(version_file):
        print("[Version]File is not exist")
        exit(-1)

    [tmp_major_ver, tmp_minor_ver] = get_versions(version_file)

    if is_inc:
        major_version = int(tmp_major_ver)
        minor_version = int(tmp_minor_ver) + 1
        print("[Version tools] Minor version number: ", minor_version)
        set_versions(version_file, major_version, minor_version)
        exit(0)

    if is_get:
        if major_version == -1:
            print("{}".format(tmp_major_ver))
            sys.exit(int(tmp_major_ver))
        if minor_version == -1:
            print("{}".format(tmp_minor_ver))
            exit(int(tmp_minor_ver))

    if is_set:
        if major_version == -1 or major_version == None:
            major_version = int(tmp_major_ver)
        if minor_version == -1 or minor_version == None:
            minor_version = int(tmp_minor_ver)
        set_versions(version_file, major_version, minor_version)
        exit(0)


if __name__ == '__main__':
    exit(main())
