import os
import sys
import logging
import asyncio
import platform
from bleak import BleakClient
from bleak import _logger as logger

MAC_ADDRESS = "10:d0:7a:17:83:a3"
CHAR_UUID = "713d0002-503e-4c75-ba94-3148f18d941e"

def notification_handler(sender, data):
    print("{0}: {1}".format(sender, ''.join('{:02x}'.format(x) for x in data)))

async def run(address, debug = False):
    log = logging.getLogger(__name__)
    if debug:
        log.setLevel(logging.DEBUG)
        h = logging.StreamHandler(sys.stdout)
        h.setLevel(logging.DEBUG)
        log.addHandler(h)

    async with BleakClient(address) as client:
        log.info(f"Connected: {client.is_connected}")
        await client.start_notify(CHARACTERISTIC_UUID, notification_handler)
        while True:
            await asyncio.sleep(5.0)
        await client.stop_notify(CHAR_UUID)

if __name__ == "__main__":
    os.environ["PYTHONASYNCIODEBUG"] = str(1)
    loop = asyncio.get_event_loop()
    loop.set_debug(True)
    loop.run_until_complete(run(MAC_ADDRESS, True))