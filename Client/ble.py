import os, sys, logging, asyncio
from threading import Thread
from bleak import BleakClient, _logger as logger
from obd import map_response

class BleController(Thread):
    def __init__(self, mac, char, callback, log):
        Thread.__init__(self) 
        self.MAC_ADDRESS = mac
        self.CHAR_UUID = char
        self.callback = callback
        self.runb = True
        self.logfile = log

    def run(self):
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        loop.run_until_complete(self.listen(self.MAC_ADDRESS))
        loop.close()

    def notification_handler(self, sender, data):
        print("{0}: {1}".format(sender, ''.join('{:02x}'.format(x) for x in data)))
        if(self.runb):
            self.logfile.write("{0}: {1}".format(sender, ''.join('{:02x}'.format(x) for x in data)))
        map_response(data)
        self.callback()

    async def listen(self, address):
        async with BleakClient(address) as client:
            await client.start_notify(self.CHAR_UUID, self.notification_handler)
            while self.runb:
                await asyncio.sleep(1)
            await client.stop_notify(self.CHAR_UUID)