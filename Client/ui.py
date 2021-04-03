from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5 import *
from mem import PID_Memory, Value_Memory, Frame_Memory, Units_Memory

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle('OBD Reader')
        self.setWindowIcon(QtGui.QIcon('./icons/car.png')) 
        self.setGeometry(240, 240, 640, 480)
        self.addWidgets()
        self.list_window.listwidget.setCurrentItem(self.list_window.widgetitems[0])
        self.refresh()

    def refresh(self):
        item = self.list_window.listwidget.currentItem()
        if item != None:
            pid = PID_Memory[item.text()]
            value = Value_Memory[item.text()]
            frame = Frame_Memory[item.text()]
            if item.text() == "ALL_MESSAGES" and Frame_Memory["ALL_MESSAGES"] != None:
                self.list_window.info.frame.box.setText(''.join('{:02x}'.format(x) for x in frame))
            elif frame == None:
                self.list_window.info.frame.box.setText("N/A")
                self.list_window.info.pid.box.setText("N/A")
                self.list_window.info.value.box.setText("N/A")
            else:
                self.list_window.info.frame.box.setText(''.join('{:02x}'.format(x) for x in frame))
                self.list_window.info.pid.box.setText(f"{pid:#0{4}x}")
                self.list_window.info.value.box.setText(str(value) +Units_Memory[item.text()])

    def addWidgets(self):
        layout = QHBoxLayout()
        self.info_window = self.ObdInfo()
        self.list_window = self.ObdList(self.info_window)
        layout.addWidget(self.list_window)
        layout.addWidget(self.info_window)
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
            self.widgetitems = []
            for data in PID_Memory:
                item = QListWidgetItem(data)
                self.widgetitems.append(item)
                self.listwidget.addItem(item)
            self.listwidget.clicked.connect(self.clicked)
            layout.addWidget(self.listwidget)

        def clicked(self, qmodelindex):
            item = self.listwidget.currentItem()
            pid = PID_Memory[item.text()]
            value = Value_Memory[item.text()]
            frame = Frame_Memory[item.text()]
            if item.text() == "ALL_MESSAGES" and Frame_Memory["ALL_MESSAGES"] != None:
                self.info.frame.box.setText(''.join('{:02x}'.format(x) for x in frame))
            elif frame == None:
                self.info.frame.box.setText("N/A")
                self.info.pid.box.setText("N/A")
                self.info.value.box.setText("N/A")
            else:
                self.info.frame.box.setText(''.join('{:02x}'.format(x) for x in frame))
                self.info.pid.box.setText(f"{pid:#0{4}x}")
                self.info.value.box.setText(str(value) +Units_Memory[item.text()])

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