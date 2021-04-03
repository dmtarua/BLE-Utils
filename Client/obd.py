from mem import Value_Memory, Frame_Memory

def map_response(message):
    size = message[0]
    mode = message[1]
    pid = message[2]
    data_a = int(message[3])
    data_b = int(message[4])
    data_c = int(message[5])
    data_d = int(message[6])

    Frame_Memory["ALL_MESSAGES"] = message

    if pid == 0x04:
        Frame_Memory["ENGINE_LOAD"] = message
        Value_Memory["ENGINE_LOAD"]= (100/255)*data_a
    elif pid == 0x05:
        Frame_Memory["COOLANT_TEMPERATURE"] = message
        Value_Memory["COOLANT_TEMPERATURE"] = data_a - 40
    elif pid == 0x06:
        Frame_Memory["SHORT_TERM_FUEL_TRIM"] = message
        Value_Memory["SHORT_TERM_FUEL_TRIM"] = (100/128)*data_a - 100
    elif pid == 0x07:
        Frame_Memory["LONG_TERM_FUEL_TRIM"] = message
        Value_Memory["LONG_TERM_FUEL_TRIM"] = (100/128)*data_a - 100
    elif pid == 0x0c:
        Frame_Memory["ENGINE_RPM"] = message
        Value_Memory["ENGINE_RPM"] = (256*data_a + data_b)/4
    elif pid == 0x0d:
        Frame_Memory["VEHICLE_SPEED"] = message
        Value_Memory["VEHICLE_SPEED"] = data_a
    elif pid == 0x0e:
        Frame_Memory["TIMING_ADVANCE"] = message
        Value_Memory["TIMING_ADVANCE"] = data_a/2 - 64