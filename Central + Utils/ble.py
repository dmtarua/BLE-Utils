import sys
from bluepy.btle import Scanner, DefaultDelegate, Peripheral, Characteristic, Service

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print ("Discovered device", dev.addr)
        elif isNewData:
            print ("Received new data from", dev.addr)

def scanAll():
    scanner = Scanner().withDelegate(ScanDelegate())
    devices = scanner.scan(10.0)

    for dev in devices:
        print ("Device %s (%s), RSSI=%d dB" % (dev.addr, dev.addrType, dev.rssi))
        for (adtype, desc, value) in dev.getScanData():
            print ("  %s = %s" % (desc, value))


def getServices(mac):
    periph = Peripheral()
    print("Connecting to: ", mac)
    periph.connect(mac)

    print("Services List: ")
    for s in periph.getServices():
        print("\tUUID: ", s.uuid)
        for c in s.getCharacteristics():
            print("\t\tUUID: ", c.uuid," Properties: ", c.propertiesToString(), "Handle:", c.getHandle())

    print("Disconnecting...")
    periph.disconnect()

def getCharacteristics(mac):
    periph = Peripheral()
    print("Connecting to: ", mac)
    periph.connect(mac)

    print("Characteristics List: ")
    for c in periph.getCharacteristics():
        print("\tUUID: ", c.uuid," Properties: ", c.propertiesToString(), "Handle:", c.getHandle())

    print("Disconnecting...")
    periph.disconnect()

def write(mac):
    periph = Peripheral()
    print("Connecting to: ", mac)
    periph.connect(mac)

    periph.writeCharacteristic(14, b'xff')

    print("Disconnecting...")
    periph.disconnect()

if(len(sys.argv) < 2):
    print ("Usage: \tpython ble.py scan\nor \tpython ble.py connect [MAC]")
elif (str(sys.argv[1]) == "scan"):
    scanAll()
elif (str(sys.argv[1]) == "services"):
    getServices("28:ed:e0:a5:b0:84")
elif (str(sys.argv[1]) == "characteristics"):
    getCharacteristics("28:ed:e0:a5:b0:84")
elif (str(sys.argv[1]) == "write"):
    write("28:ed:e0:a5:b0:84")