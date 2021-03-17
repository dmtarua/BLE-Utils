from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5 import *
from mem import PID_Memory, Value_Memory, Frame_Memory, Units_Memory

class UI:
    def __init__(self):
        self.app = QApplication([])
        self.mw = self.MainWindow()
        self.app.exec_()

    def refresh_info(self):
        print("refreshed")
        # TODO

    class MainWindow(QMainWindow):
        def __init__(self):
            super().__init__()
            self.setWindowTitle('OBD Reader')
            self.setWindowIcon(QtGui.QIcon('./icons/obd.png')) 
            self.setGeometry(10, 10, 640, 480)
            self.addWidgets()

        def addWidgets(self):
            layout = QHBoxLayout()
            self.info_widget = self.ObdInfo()
            self.list_widget = self.ObdList(self.info_widget)
            layout.addWidget(self.list_widget)
            layout.addWidget(self.info_widget)
            container = QWidget()
            container.setLayout(layout)
            self.setCentralWidget(container)
            self.show()

        class ObdList(QWidget):
            def __init__(self, info):
                super().__init__()
                self.info = info
                self.start()

            def start(self):
                layout = QGridLayout()
                self.setLayout(layout)
                self.listwidget = QListWidget()
                index = 0
                for data in PID_Memory:
                    index += 1
                    self.listwidget.insertItem(index, data.name)
                self.listwidget.clicked.connect(self.clicked)
                layout.addWidget(self.listwidget)

            def clicked(self, qmodelindex):
                item = self.listwidget.currentItem()
                pid = PID_Memory[item.text()].value
                value = Value_Memory[item.text()].value
                frame = Frame_Memory[item.text()].value
                if pid == None:
                    self.info.frame.box.setText("N/A")
                    self.info.pid.box.setText("N/A")
                    self.info.value.box.setText("N/A")
                else:
                    self.info.frame.box.setText(f"{frame:#0{18}x}")
                    self.info.pid.box.setText(f"{pid:#0{4}x}")
                    self.info.value.box.setText(str(value) +Units_Memory[item.text()].value)

        class ObdInfo(QWidget):
            def __init__(self):
                super().__init__()
                self.start()

            def start(self):
                layout = QVBoxLayout()
                self.setLayout(layout)
                self.frame = self.ObdFrame()
                self.pid = self.ObdPid()
                self.value = self.ObdValue()
                layout.addWidget(self.frame)
                layout.addWidget(self.pid)
                layout.addWidget(self.value)

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