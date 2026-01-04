import argparse
import time
import getpass
import os

SIGNATURE_OFFSET = 0x200
VER_MAJOR_OFFSET = 0x204
VER_MINOR_OFFSET = 0x208
CRC_DATA_LEN_OFFSET = 0x20C
DATA_BLOCK_OFFSET = 0x000
CRC_OFFSET = -4

# Name : CRC-16/CCITT
# Poly : 0x1021
# Init : 0xFFFF
# Revert : false
# XorOut : false
# Check: 0x29B1 ("123456789")


def crc16(array):
    crc = 0xFFFF
    for i in array:
        crc ^= i << 8
        for j in range(8):
            if crc & 0x8000:
                crc = (((crc << 1) & 0xFFFF) ^ 0x1021)
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


parser = argparse.ArgumentParser()
parser.add_argument('-i', '--input_file', help='input image file')
parser.add_argument('-o', '--output_file', help='output image file')

args = parser.parse_args()

input_file = args.input_file
output_file = args.output_file

if input_file is None:
    print('Error,  Input file is not set')
    exit(-2)

if output_file is None:
    output_file = input_file

with open(input_file, 'rb') as read_stream:
    data = read_stream.read()
    data_len = len(data[DATA_BLOCK_OFFSET:]) - 4
    crc = crc16(data[DATA_BLOCK_OFFSET:data_len])
    print('CRC = 0x%.8X' % crc)
    print('Len = 0x%.8X (%d bytes)' % (data_len, data_len))

with open(output_file, 'wb') as write_stream:
    write_stream.write(data)
    write_stream.seek(len(data)-4)
    write_stream.write(crc.to_bytes(4, byteorder='little'))
    # write_stream.seek(CRC_DATA_LEN_OFFSET)
    # write_stream.write(data_len.to_bytes(4, byteorder='little'))

exit(0)
