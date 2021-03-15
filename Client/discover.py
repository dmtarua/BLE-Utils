import os
import sys
import logging
import asyncio
import platform
from bleak import BleakClient
from bleak import _logger as logger

MAC_ADDRESS = "10:d0:7a:17:83:a3"
CHARACTERISTIC_UUID = "713d0002-503e-4c75-ba94-3148f18d941e"

async def run(address, debug=False):
    log = logging.getLogger(__name__)
    if debug:
        log.setLevel(logging.DEBUG)
        h = logging.StreamHandler(sys.stdout)
        h.setLevel(logging.DEBUG)
        log.addHandler(h)

    async with BleakClient(address) as client:
        log.info(f"Connected: {client.is_connected}")
        for service in client.services:
            log.info(f"[Service] {service}")
            for char in service.characteristics:
                if "read" in char.properties:
                    try:
                        value = bytes(await client.read_gatt_char(char.uuid))
                        log.info(f"\t[Characteristic] {char} ({','.join(char.properties)}), Value: {value}")
                    except Exception as e:
                        log.error(f"\t[Characteristic] {char} ({','.join(char.properties)}), Value: {e}")

                else:
                    value = None
                    log.info(f"\t[Characteristic] {char} ({','.join(char.properties)}), Value: {value}")

                for descriptor in char.descriptors:
                    try:
                        value = bytes(await client.read_gatt_descriptor(descriptor.handle))
                        log.info(f"\t\t[Descriptor] {descriptor}) | Value: {value}")
                    except Exception as e:
                        log.error(f"\t\t[Descriptor] {descriptor}) | Value: {e}")
                        
if __name__ == "__main__":
    os.environ["PYTHONASYNCIODEBUG"] = str(1)
    loop = asyncio.get_event_loop()
    loop.set_debug(True)
    loop.run_until_complete(run(MAC_ADDRESS, True))