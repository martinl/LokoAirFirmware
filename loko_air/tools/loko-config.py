import argparse
import os
import serial
from scanf import scanf

# usage example:
# python loko-config.py --id1 123456 --id2 222222 --freq 868000000 --interval 60 --port COM3 --info

is_verbose = False


def parse_record_line(line):
    record = scanf('#%d %d/%d/%d %d:%d:%d %f, %f\n', line)
    if record != None:
        n = '{}'.format(record[0])
        data = '{:02}/{:02}/{:02}'.format(record[1], record[2], record[3])
        time = '{:02}:{:02}:{:02}'.format(record[4], record[5], record[6])
        lat = '{}'.format(record[7])
        lon = '{}'.format(record[8])
        return {'n': n, 'day': record[3], 'date': data, 'time': time, 'lat': lat, 'lon': lon}
    return None


def save_gtrace(record_info, gnss_records):
    file_name = 'loko-id-{:08}-{:08}'.format(
        record_info['id1'], record_info['id2'])
    create_csv_file(file_name + '-full', gnss_records)
    create_gpx_file(file_name + '-full', gnss_records)
    day_last = gnss_records[0]['day']
    day_count = 0
    day_traces = []
    for record in gnss_records:
        if record['day'] != day_last:
            day_count += 1
            day_last = record['day']
            date = '-' + day_traces[0]['date'].replace('/', '_')
            create_gpx_file(file_name + date, day_traces)
            day_traces.clear()
        day_traces.append(record)

    if day_count > 0 and len(day_traces) > 0:
        date = '-' + day_traces[0]['date'].replace('/', '_')
        create_gpx_file(file_name + date, day_traces)


def create_csv_file(name, data):
    file_name = name + '.csv'
    sequence_n = 1
    while os.path.exists(file_name):
        file_name = '{}({}).csv'.format(name, sequence_n)
        sequence_n += 1

    csv_file = open(file_name, 'w')
    csv_file.write('#,date,time,lat,lon\r')
    for record in data:
        csv_file.write('{},{},{},{},{}\r'.format(
            record['n'], record['date'], record['time'], record['lat'], record['lon']))
    csv_file.close()
    print('created file({} points):{}'.format(len(data), file_name))


def create_gpx_file(name, data):
    header = '''<?xml version="1.0" encoding="UTF-8"?>\n<gpx xmlns="http://www.topografix.com/GPX/1/1" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd" version="1.1" creator="loko-configurator">\n  <trk>\n    <trkseg>\n'''
    tail = '''    </trkseg>\n  </trk>\n</gpx>'''

    file_name = name + '.gpx'
    sequence_n = 1
    while os.path.exists(file_name):
        file_name = '{}({}).gpx'.format(name, sequence_n)
        sequence_n += 1

    gpx_file = open(file_name, 'w')
    gpx_file.write(header)
    for record in data:
        gpx_file.write(
            '      <trkpt lat=\"{}\" lon=\"{}\">\n      </trkpt>\n'.format(record['lat'], record['lon']))
    gpx_file.write(tail)
    gpx_file.close()
    print('created file({} points):{}'.format(len(data), file_name))


def send_cmd(ser, cmd):
    if is_verbose:
        print('Send cmd: %s' % cmd)
    ser.flush()
    ser.write(cmd)


def wait_for_ok(ser):
    while True:
        line = ser.readline()
        if line == b'OK\r\n':
            print("OK")
            return True
        elif line == b'':
            return False
        if is_verbose:
            print('>>>%s' % line)
    return False


def wait_for_payload(ser):
    while True:
        line = ser.readline()
        if line == b'OK\r\n':
            print("OK")
            return True
        elif line == b'':
            print("Error: no OK response")
            return False
        if is_verbose:
            print('>>>%s' % line)
        text = line.decode()
        n = text.find('\t.')
        if n > 0:
            print('  %s' % text[n+2:], end='')
    return False


def wait_for_records(ser):
    gnss_records = []
    record_info = []
    while True:
        byte_line = ser.readline()
        string_line = byte_line.decode()
        if byte_line == b'OK\r\n':
            received_record_count = len(gnss_records)
            if record_info['count'] != received_record_count:
                print('Wrong read count, received {}, expected {}'.format(
                    received_record_count, record_info['count']))
                return False
            if received_record_count == 0:
                print("Zero record count, nothing to save")
                return True
            save_gtrace(record_info, gnss_records)
            print("OK")
            return True
        elif string_line.find('Record found ') != -1:
            _info = scanf('Record found %u, ID1=%u, ID2=%u\r', string_line)
            record_info = {'count': int(_info[0]), 'id1': str(
                _info[1]), 'id2': str(_info[2])}
        elif byte_line == b'':
            print("Error: no OK response")
            return False
        else:
            record = parse_record_line(string_line)
            if record != None:
                gnss_records.append(record)
            print(string_line, end='')
    return False

def  chek_hex_key(key, max_len):
    if len(key) != max_len:
        return False

    hex_chars = set('0123456789abcdefABCDEF')

    return all(c in hex_chars for c in key)


if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('--reset', dest='reset', action='store_true', help='re-boot device')
    parser.add_argument('--info', dest='info', action='store_true', help='show current settings')
    parser.add_argument('--verbose', dest='verbose', action='store_true', help='show data exchange')
    parser.add_argument('--id1', nargs='?', const=-1, type=int, help='set ID1')
    parser.add_argument('--id2', nargs='?', const=-1, type=int, help='set ID2')
    parser.add_argument('--gtrace_print', dest='gtrace_print', action='store_true', help='Print all GNSS Records')
    parser.add_argument('--gtrace_erase', dest='gtrace_erase', action='store_true', help='Erase all GNSS record')
    parser.add_argument('--gtrace_period', nargs='?', const=-1, type=int, help='Set save period')
    parser.add_argument('--freq', nargs='?', const=-1, type=int, help='set lora frequency in Hz')
    parser.add_argument('--interval', nargs='?', const=-1, type=int, help='set auto wakeup interval in seconds')
    parser.add_argument('--port', help='serial port name')

    parser.add_argument('--lorawan', help='Enable or Disable lorawan stack')
    parser.add_argument('--dev-eui', help='Set dev-eui key for LoRaWAN\r\n --dev-eui 0123456789ABCDEF')
    parser.add_argument('--app-eui', help='Set app-eui key for LoRaWAN\r\n --app-eui 0123456789ABCDEF')
    parser.add_argument('--app-key', help='Set app-key key for LoRaWAN\r\n --app-key 0123456789ABCDEF0123456789ABCDEF')
    parser.add_argument('--p2p-key', help='Set p2p key\r\n --p2p-key 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF')
    parser.add_argument('--p2p-encryption', help='Enable or Disable p2p air traffic encryption')
    args = parser.parse_args()
    # print(args)

    is_verbose = args.verbose

    if (args.port == None):
        print("Please provide serial port number(--port)")
        exit(1)

    print('Open port: %s' % args.port)
    ser = serial.Serial(args.port, 115200, timeout=1)

    send_cmd(ser, b'debug 0\n')
    wait_for_ok(ser)

    if (args.info == True):
        print("Read information:")
        send_cmd(ser, b'info\n')
        if wait_for_payload(ser) == False:
            print("No response for: Info")

    if (args.id1 != None):
        send_cmd(ser, b'set id1 %d\n' % args.id1)
        if wait_for_ok(ser) == False:
            print("No response for: ID1")

    if (args.id2 != None):
        send_cmd(ser, b'set id2 %d\n' % args.id2)
        if wait_for_ok(ser) == False:
            print("No response for: ID2")

    if (args.freq != None):
        send_cmd(ser, b'set freq %d\n' % args.freq)
        if wait_for_ok(ser) == False:
            print("No response for: Freq")

    if (args.interval != None):
        send_cmd(ser, b'set interval %d\n' % args.interval)
        if wait_for_ok(ser) == False:
            print("No response for: Interval")

    if (args.gtrace_print == True):
        print("Read information:")
        send_cmd(ser, b'gtrace print\n')
        if wait_for_records(ser) == False:
            print("No response for: gtrace print")

    if (args.gtrace_erase == True):
        print("Erase GNSS Trace records...")
        send_cmd(ser, b'gtrace erase\n')
        if wait_for_ok(ser) == False:
            print("No response for: gtrace erase")

    if (args.gtrace_period != None):
        send_cmd(ser, b'gtrace period %d\n' % args.gtrace_period)
        if wait_for_ok(ser) == False:
            print("No response for: gtrace period")

    if (args.p2p_encryption != None):
        if args.p2p_encryption == '1':
            print("Enabling p2p encryption....")
            send_cmd(ser, b'p2p encryption 1\n')
        else:
            print("Disabling p2p encryption....")
            send_cmd(ser, b'p2p encryption 0\n')
        if wait_for_ok(ser) == False:
            print("No response for: set p2p encryption")

    if (args.dev_eui != None):
        key = args.dev_eui
        if chek_hex_key(key, 16):
            send_cmd(ser, b'set dev-eui ' + key.encode() + b'\n')
            if wait_for_ok(ser) == False:
                print("No response for: set dev-eui")
        else:
            print(f'Wrong key: {key}')

    if (args.app_eui != None):
        key = args.app_eui
        if chek_hex_key(key, 16):
            send_cmd(ser, b'set app-eui ' + key.encode() + b'\n')
            if wait_for_ok(ser) == False:
                print("No response for: set app-eui")
        else:
            print(f'Wrong key: {key}')

    if (args.app_key != None):
        key = args.app_key
        if chek_hex_key(key, 32):
            send_cmd(ser, b'set app-key ' + key.encode() + b'\n')
            if wait_for_ok(ser) == False:
                print("No response for: set app-key")
        else:
            print(f'Wrong key: {key}')

    if (args.p2p_key != None):
        key = args.p2p_key
        if chek_hex_key(key, 64):
            send_cmd(ser, b'set p2p-key ' + key.encode() + b'\n')
            if wait_for_ok(ser) == False:
                print("No response for: set p2p-key")
        else:
            print(f'Wrong key: {key}')

    if (args.lorawan != None):
        if args.lorawan == '1':
            print("Enabling lorawan mode....")
            send_cmd(ser, b'enable lorawan mode 1\nreset\n')
        else:
            print("Disabling lorawan mode....")
            send_cmd(ser, b'enable lorawan mode 0\nreset\n')
        if wait_for_ok(ser) == False:
            print("No response for: enable lorawan mode")

    if (args.reset == True):
        send_cmd(ser, b'reset\n')
        if wait_for_ok(ser) == False:
            print("No response for: reset")

# python tools/loko-config.py --port /dev/stdout --p2p-key 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF
