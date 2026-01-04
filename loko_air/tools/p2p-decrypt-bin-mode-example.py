
import base64
import struct

#pip install cryptography
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend
from Crypto.Cipher import AES

def unpack_lat_lon(packed_data):
    # Extract and convert 3 bytes for latitude to a signed 24-bit integer
    lat_lon_scaled = (packed_data[0] << 16) | (packed_data[1] << 8) | packed_data[2]
    if lat_lon_scaled & 0x800000:  # Check if the sign bit is set for a negative value
        lat_lon_scaled -= 0x1000000  # Convert to signed 24-bit integer

    scaling_factor = 10000.0
    lat_lon = lat_lon_scaled / scaling_factor

    return lat_lon

def bin_unpack_vbat(vbat):
    return (vbat + 27) * 0.1


def parse_loko_bin_packet(bin_data, key):
    id1 = 0
    id2 = 0
    vbat_mv = 0
    packet_version = 0
    lat = 0.0
    lon = 0.0
    alt_meters = 0
    speed_mps = 0
    data = bytes.fromhex(bin_data)
    if len(data) == 15:
        id1, id2, vb_version, lat_24bit, lon_24bit= struct.unpack("<II B 3s 3s", data)
        packet_version = (vb_version >> 4) & 0x0F
        vbat_mv = bin_unpack_vbat(vb_version & 0x0F)
        lat = unpack_lat_lon(lat_24bit)
        lon = unpack_lat_lon(lon_24bit)
    elif len(data) == 18:
        id1, id2, vb_version, lat_24bit, lon_24bit, speed_mps, alt_meters= struct.unpack("<II B 3s 3s B h", data)
        packet_version = (vb_version >> 4) & 0x0F
        vbat_mv = bin_unpack_vbat(vb_version & 0x0F)
        lat = unpack_lat_lon(lat_24bit)
        lon = unpack_lat_lon(lon_24bit)
    elif len(data) == 25:
        id1, id2, vb_version, aes_payload= struct.unpack(">II B 16s", data)
        packet_version = (vb_version >> 4) & 0x0F
        encrypted_bytes = bytes(aes_payload)

        # Initialize the AES cipher in ECB mode
        cipher = aes(key, 1)  # 1 for ECB mode
        # cipher = AES.new(key, AES.MODE_ECB)
        decrypted_bytes = cipher.decrypt(encrypted_bytes)
        checksum = sum(decrypted_bytes[:-1]) % 256

        (vb_version, lat_24bit, lon_24bit, speed_mps, alt_meters, reserved1,  integrity) = struct.unpack('<B3s3sBH5sB', decrypted_bytes)
        if checksum != integrity:
            print('Can\'t decrypt, possible wrong key')
        else:
            vbat_mv = bin_unpack_vbat(vb_version & 0x0F)
            lat = unpack_lat_lon(lat_24bit)
            lon = unpack_lat_lon(lon_24bit)

    return {'id1': id1, 'id2': id2, 'lat': lat, 'lon': lon, 'vbat': vbat_mv, 'alt': alt_meters, 'mps': speed_mps}


enc_payload = "0000000000000000302C01694AAD3996B9831555F75B4251BC"
p1_payload = "00000000000000001F05F1C206F672"
extended_payload = "00000000000000001E08582403DB94027A00"
secret_p2p_key_hex  = "0000000000000000000000000000000000000000000000000000000000000000"
# Convert hexadecimal key to bytes
key = bytes.fromhex(secret_p2p_key_hex)

print(parse_loko_bin_packet(enc_payload, key))
print(parse_loko_bin_packet(p1_payload, key))
print(parse_loko_bin_packet(extended_payload, key))
