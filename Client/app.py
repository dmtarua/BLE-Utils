from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5 import *

OBD_PIDS = ["ENGINE_LOAD",
    "COOLANT_TEMPERATURE",
    "SHORT_TERM_FUEL_TRIM",
    "LONG_TERM_FUEL_TRIM",
    "ENGINE_RPM",
    "VEHICLE_SPEED",
    "TIMING_ADVANCE",
    "INTAKE_AIR_TEMPERATURE",
    "MAF_AIR_FLOW_RATE",
    "THROTTLE",
    "O2_SENSOR_2",
    "ENGINE_RUN_TIME",
    "DISTANCE_TRAVELED_WITH_MIL_ON",
    "COMMANDED_EVAPORATIVE_PURGE",
    "FUEL_TANK_LEVEL_INPUT",
    "WARM_UPS_SINCE_CODES_CLEARED",
    "DISTANCE_TRAVELED_SINCE_CODES_CLEARED",
    "ABSOLUTE_BAROMETRIC_PRESSURE",
    "O2_SENSOR_1",
    "CATALYST_TEMPERATURE_BANK1_SENSOR1",
    "MONITOR_STATUS",
    "CONTROL_MODULE_VOLTAGE",
    "ABSOLUTE_LOAD_VALUE",
    "FUEL_AIR_COMMANDED_EQUIV_RATIO",
    "RELATIVE_THROTTLE",
    "AMBIENT_AIR_TEMPERATURE",
    "ABSOLUTE_THROTTLE_B",
    "ACCELERATOR_PEDAL_POSITION_D",
    "ACCELERATOR_PEDAL_POSITION_E",
    "COMMANDED_THROTTLE_ACTUATOR"
]

class ObdList(QWidget):
    def __init__(self):
        super().__init__()
        self.start()

    def start(self):
        layout = QGridLayout()
        self.setLayout(layout)
        self.listwidget = QListWidget()
        for i in range(0, 30):
            self.listwidget.insertItem(i, OBD_PIDS[i])
        self.listwidget.clicked.connect(self.clicked)
        layout.addWidget(self.listwidget)

    def clicked(self, qmodelindex):
        item = self.listwidget.currentItem()

class ObdInfo(QWidget):
    def __init__(self):
        super().__init__()
        self.start()

    def start(self):
        layout = QVBoxLayout()
        self.setLayout(layout)
        self.frame = QLineEdit()
        self.pid = QLineEdit()
        self.value = QLineEdit()
        layout.addWidget(ObdFrame())
        layout.addWidget(ObdPid())
        layout.addWidget(ObdValue())

class ObdFrame(QWidget):
    def __init__(self):
        super().__init__()
        self.start()

    def start(self):
        layout = QHBoxLayout()
        self.setLayout(layout)
        self.label = QLabel("Frame: ")
        self.box = QLineEdit()
        self.box.setReadOnly(True)
        layout.addWidget(self.label)
        layout.addWidget(self.box)

class ObdPid(QWidget):
    def __init__(self):
        super().__init__()
        self.start()

    def start(self):
        layout = QHBoxLayout()
        self.setLayout(layout)
        self.label = QLabel("PID: ")
        self.box = QLineEdit()
        self.box.setReadOnly(True)
        layout.addWidget(self.label)
        layout.addWidget(self.box)

class ObdValue(QWidget):
    def __init__(self):
        super().__init__()
        self.start()

    def start(self):
        layout = QHBoxLayout()
        self.setLayout(layout)
        self.label = QLabel("Value: ")
        self.box = QLineEdit()
        self.box.setReadOnly(True)
        layout.addWidget(self.label)
        layout.addWidget(self.box)

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle('OBD Reader')
        self.setWindowIcon(QtGui.QIcon('./icons/obd.png')) 
        self.setGeometry(10, 10, 640, 480)
        self.addWidgets()

    def addWidgets(self):
        layout = QHBoxLayout()
        widgets = [ObdList, ObdInfo]
        for w in widgets:
            layout.addWidget(w())
        widget = QWidget()
        widget.setLayout(layout)
        self.setCentralWidget(widget)
        self.show()

if __name__=='__main__':
    app = QApplication([])
    mw = MainWindow()
    app.exec_()