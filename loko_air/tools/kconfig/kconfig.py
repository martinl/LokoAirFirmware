import argparse
import os
import sys

from kconfiglib import Kconfig, _BOOL_TRISTATE, STRING, HEX, escape

def make_header(kconf):

    h_file = ''
    config_prefix = 'CONFIG_'

    for sym in kconf.unique_defined_syms:

        val = sym.str_value
        if not sym._write_to_conf:
            continue

        if sym.orig_type in _BOOL_TRISTATE:
            if val == "y":
                h_file += f"#define {config_prefix}{sym.name} 1\n"
            elif val == "n":
                h_file += f"#define {config_prefix}{sym.name} 0\n"
            elif val == "m":
                h_file += f"#define {config_prefix}{sym.name}_MODULE 1\n"

        elif sym.orig_type is STRING:
            h_file += f'#define {config_prefix}{sym.name} "{escape(val)}"\n'

        else:  # sym.orig_type in _INT_HEX:
            if sym.orig_type is HEX and \
                not val.startswith(("0x", "0X")):
                val = "0x" + val

            h_file += f"#define {config_prefix}{sym.name} {val}\n"

    return h_file

def write_autoconf_with_disabled_options(kconf, file_name):
    with open(file_name, "w") as autoconf_file:
        autoconf_file.write(make_header(kconf))

def main():
    args = parse_args()
    kconf = Kconfig(args.root_kconfig)

    # Enable some warnings
    kconf.warn_assign_undef = True

    print(kconf.load_config(args.configs[0]))
    for cfg in args.configs[1:]:
        # Merge config files
        print(kconf.load_config(cfg, replace=False))

    if kconf.warnings:
        for wrn in kconf.warnings:
            print("\n" + wrn, file=sys.stderr)
        sys.exit("Warnings in kconfig detected. Aborting!")

    print(kconf.write_config(args.out_config))
    # print(kconf.write_autoconf(args.header_out))
    write_kconfig_filenames(kconf, args.list_out)
    write_autoconf_with_disabled_options(kconf, args.header_out)


def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument("root_kconfig", help="Root kconfig file used for parsing")
    parser.add_argument("out_config", help=".config output file name")
    parser.add_argument("header_out", help="generated header file name")
    parser.add_argument("list_out", help="kconfig source file list output file name")
    parser.add_argument("configs", nargs="+", help="config files to be merged")

    return parser.parse_args()

def write_kconfig_filenames(kconf, kconfig_list_path):
    with open(kconfig_list_path, 'w') as out:
        for path in sorted({os.path.realpath(os.path.join(kconf.srctree, path))
                            for path in kconf.kconfig_filenames}):
            print(path, file=out)

if __name__ == "__main__":
    main()
