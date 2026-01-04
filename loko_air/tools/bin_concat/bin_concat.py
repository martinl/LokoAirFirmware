import argparse
import os


def concatenation(file_list, output_file_name, padding_byte=b'\x00'):
    with open(output_file_name, 'wb') as f_dest:
        for cat_file in file_list:
            print('Offset: 0x{1:08X}({1}), {0}, {2}(0x{2:X}) bytes,  '.format(
                cat_file['name'], cat_file['offset'], cat_file['size']))
            while (f_dest.tell() < cat_file['offset']):
                f_dest.write(padding_byte)

            # seek for case when offset less than current size
            f_dest.seek(cat_file['offset'], 0)

            with open(cat_file['f_path'], 'rb') as f:
                f_dest.write(f.read())

    out_size = os.path.getsize(output_file_name)
    print('Output file: {}, {} bytes({:.2f}KB)'.format(
        output_file_name, out_size, out_size / 1024))


def split_arg(arg):
    fpath, offset_str = arg.split('@')
    offset = 0
    if offset_str.startswith('0x'):
        offset = int(offset_str, 16)
    else:
        offset = int(offset_str)

    if not os.path.exists(fpath):
        print('Error: file not found:', fpath)
        return None

    size = os.path.getsize(fpath)
    name = os.path.basename(fpath)

    return {'f_path': fpath, 'name': name, 'offset': offset, 'size': size}


def print_overlap_info(file_list):
    for i in range(0, len(file_list) - 1):
        current_file_position = file_list[i]['offset'] + file_list[i]['size']
        if current_file_position > file_list[i + 1]['offset']:
            print('WARNING: File {0} will overlap at offset {1}(0x{1:X}) by {2}'.format(
                file_list[i]['f_path'],  current_file_position - file_list[i+1]['offset'], file_list[i + 1]['f_path']))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', default=[], nargs='*',
                        help='input file list, format: -f file_1_path.bin@offset1  -f file_2_path.bin@offset2 ')
    parser.add_argument('-o', '--output', help='output file')
    parser.add_argument('-p', '--padding', default='00',
                        help='padding byte, -p FF')
    args = parser.parse_args()

    # build/bootloader/bootloader.bin@0 build/partition_table/partition-table.bin@0x8000 build/ota_data_initial.bin 0x10000 build/rg_lock.bin@0xd000

    if len(args.input) == 0:
        print('Error: please provide input files, -i file-name.bin@offset')
        exit(-1)

    if len(args.input) == 1:
        print('One file detected, just insert padding')

    file_list = []
    for in_file in args.input:
        file_info = split_arg(in_file)
        if file_info == None:
            return 1
        file_list.append(file_info)

    file_list = sorted(file_list, key=lambda tup: tup['offset'])

    print_overlap_info(file_list)

    out_file_name = args.output
    if out_file_name is None:
        out_file_name = ''
        for info in file_list:
            out_file_name += '{}_'.format(info['name'])
        out_file_name = out_file_name[:-1] + '.bin'

    padding_str = args.padding[0:2]
    padding = int(padding_str, 16).to_bytes(length=1, byteorder='little')
    print('Padding byte: 0x{}'.format(padding_str))

    concatenation(file_list, out_file_name, padding)

    return 0


if __name__ == '__main__':
    exit(main())
