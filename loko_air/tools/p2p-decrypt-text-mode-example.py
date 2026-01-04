
import base64
import struct

#pip install cryptography
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend
from Crypto.Cipher import AES

received_payload = "00,000,KsC72EMf5cAYJU8eATDTMg==" #00,000,180.12345,-180.54321,4210
secret_p2p_key_hex  = "0000000000000000000000000000000000000000000000000000000000000000"
# Convert hexadecimal key to bytes
key = bytes.fromhex(secret_p2p_key_hex)

id1, id2, encrypted_payload = received_payload.split(',')
print(encrypted_payload)

encrypted_bytes = base64.b64decode(encrypted_payload)
print(f'Len {len(encrypted_bytes)} {encrypted_bytes}')

cipher = AES.new(key, AES.MODE_ECB)
decrypted_bytes = cipher.decrypt(encrypted_bytes)
checksum = sum(decrypted_bytes[:-1]) % 256

# Alternate way:
# Initialize the cipher with AES256 algorithm and ECB mode
# cipher = Cipher(algorithms.AES(key), modes.ECB(), backend=default_backend())
# decryptor = cipher.decryptor()
# decrypted_bytes = decryptor.update(encrypted_bytes) + decryptor.finalize()
# print(decrypted_bytes)  # Assuming the decrypted bytes represent text data


# Unpack the decrypted bytes into their respective data types
(lat, lon, vbat_mv, reserved1, reserved2, integrity) = struct.unpack('<ffHIBB', decrypted_bytes)
print("Latitude:", lat)
print("Longitude:", lon)
print("Battery Voltage (mV):", vbat_mv)
print("Reserved 1:", reserved1)
print("Reserved 2:", reserved2)
print("integrity:", integrity, " checksum:", checksum)
