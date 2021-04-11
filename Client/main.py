import sys
from PyQt5.QtWidgets import QApplication
from ui import MainWindow
from ble import BleController

MAC_ADDRESS = "10:d0:7a:17:83:a3"
CHAR_UUID = "713d0002-503e-4c75-ba94-3148f18d941e"

logfile = open("traces/seat-can.txt", "w")
ble = None

def close_ble():
    ble.runb = False
    logfile.close()

if __name__=='__main__':
    app = QApplication(sys.argv)
    window = MainWindow()
    ble = BleController(MAC_ADDRESS, CHAR_UUID, window.refresh, logfile)
    ble.start()
    app.lastWindowClosed.connect(close_ble)
    sys.exit(app.exec_())