import os, sys, logging, asyncio
from threading import Thread
from bleak import BleakClient, _logger as logger
from obd import map_response

class BleController(Thread):
    def __init__(self, mac, char, callback):
        Thread.__init__(self) 
        self.MAC_ADDRESS = mac
        self.CHAR_UUID = char
        self.callback = callback

    def run(self):
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        loop.run_until_complete(self.listen(self.MAC_ADDRESS))

    def notification_handler(sender, data):
        print("{0}: {1}".format(sender, ''.join('{:02x}'.format(x) for x in data)))
        map_response(data)
        self.callback()

    async def listen(self, address):
        async with BleakClient(address) as client:
            print(f"Connected: {client.is_connected}")
            await client.start_notify(self.CHAR_UUID, self.notification_handler)
            while True:
                await asyncio.sleep(5.0)
            await client.stop_notify(self.CHAR_UUID)