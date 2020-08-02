// OBD CAN message IDs
const auto OBD_CAN_BROADCAST_ID    = 0X7DF;
const auto OBD_CAN_REQUEST_ID      = 0x7E0;
const auto OBD_CAN_REPLY_ID_MIN    = 0x7E8;
const auto OBD_CAN_REPLY_ID_MAX    = 0x7EF;

// OBD services / modes
const auto OBD_MODE_CURRENT_DATA = 0x01;

// OBD PIDs
const auto OBD_PID_SUPPORTED_PIDS_01_20                  = 0x00;
// MIL = malfunction indicator lamp = check engine light
const auto OBD_PID_MIL_STATUS                            = 0x01;
const auto OBD_PID_FUEL_SYSTEM_STATUS                    = 0x03;
const auto OBD_PID_ENGINE_LOAD                           = 0x04;
const auto OBD_PID_COOLANT_TEMPERATURE                   = 0x05;
const auto OBD_PID_SHORT_TERM_FUEL_TRIM                  = 0x06;
const auto OBD_PID_LONG_TERM_FUEL_TRIM                   = 0x07;
const auto OBD_PID_ENGINE_RPM                            = 0x0c;
const auto OBD_PID_VEHICLE_SPEED                         = 0x0d;
const auto OBD_PID_TIMING_ADVANCE                        = 0x0e;
const auto OBD_PID_INTAKE_AIR_TEMPERATURE                = 0x0f;
const auto OBD_PID_MAF_AIR_FLOW_RATE                     = 0x10;
const auto OBD_PID_THROTTLE                              = 0x11;
const auto OBD_PID_O2_SENSORS_PRESENT                    = 0x13;
const auto OBD_PID_O2_SENSOR_2                           = 0x15;
const auto OBD_PID_OBD_STANDARDS                         = 0x1c;
const auto OBD_PID_ENGINE_RUN_TIME                       = 0x1f;
const auto OBD_PID_SUPPORTED_PIDS_21_40                  = 0x20;
const auto OBD_PID_DISTANCE_TRAVELED_WITH_MIL_ON         = 0x21;
const auto OBD_PID_COMMANDED_EVAPORATIVE_PURGE           = 0x2e;
const auto OBD_PID_FUEL_TANK_LEVEL_INPUT                 = 0x2f;
const auto OBD_PID_WARM_UPS_SINCE_CODES_CLEARED          = 0x30;
const auto OBD_PID_DISTANCE_TRAVELED_SINCE_CODES_CLEARED = 0x31;
const auto OBD_PID_ABSOLUTE_BAROMETRIC_PRESSURE          = 0x33;
const auto OBD_PID_O2_SENSOR_1                           = 0x34;
const auto OBD_PID_CATALYST_TEMPERATURE_BANK1_SENSOR1    = 0x3c;
const auto OBD_PID_SUPPORTED_PIDS_41_60                  = 0x40;
const auto OBD_PID_MONITOR_STATUS                        = 0X41;
const auto OBD_PID_CONTROL_MODULE_VOLTAGE                = 0X42;
const auto OBD_PID_ABSOLUTE_LOAD_VALUE                   = 0X43;
const auto OBD_PID_FUEL_AIR_COMMANDED_EQUIV_RATIO        = 0X44;
const auto OBD_PID_RELATIVE_THROTTLE                     = 0X45;
const auto OBD_PID_AMBIENT_AIR_TEMPERATURE               = 0X46;
const auto OBD_PID_ABSOLUTE_THROTTLE_B                   = 0X47;
const auto OBD_PID_ACCELERATOR_PEDAL_POSITION_D          = 0X49;
const auto OBD_PID_ACCELERATOR_PEDAL_POSITION_E          = 0X4a;
const auto OBD_PID_COMMANDED_THROTTLE_ACTUATOR           = 0X4c;

const size_t NUM_PIDS_TO_REQUEST = 30;
const uint8_t pidsToRequest[NUM_PIDS_TO_REQUEST] = {
  OBD_PID_ENGINE_LOAD,
  OBD_PID_COOLANT_TEMPERATURE,
  OBD_PID_SHORT_TERM_FUEL_TRIM,
  OBD_PID_LONG_TERM_FUEL_TRIM,
  OBD_PID_ENGINE_RPM,
  OBD_PID_VEHICLE_SPEED,
  OBD_PID_TIMING_ADVANCE,
  OBD_PID_INTAKE_AIR_TEMPERATURE,
  OBD_PID_MAF_AIR_FLOW_RATE,
  OBD_PID_THROTTLE,
  OBD_PID_O2_SENSOR_2,
  OBD_PID_ENGINE_RUN_TIME,
  OBD_PID_DISTANCE_TRAVELED_WITH_MIL_ON,
  OBD_PID_COMMANDED_EVAPORATIVE_PURGE,
  OBD_PID_FUEL_TANK_LEVEL_INPUT,
  OBD_PID_WARM_UPS_SINCE_CODES_CLEARED,
  OBD_PID_DISTANCE_TRAVELED_SINCE_CODES_CLEARED,
  OBD_PID_ABSOLUTE_BAROMETRIC_PRESSURE,
  OBD_PID_O2_SENSOR_1,
  OBD_PID_CATALYST_TEMPERATURE_BANK1_SENSOR1,
  OBD_PID_MONITOR_STATUS,
  OBD_PID_CONTROL_MODULE_VOLTAGE,
  OBD_PID_ABSOLUTE_LOAD_VALUE,
  OBD_PID_FUEL_AIR_COMMANDED_EQUIV_RATIO,
  OBD_PID_RELATIVE_THROTTLE,
  OBD_PID_AMBIENT_AIR_TEMPERATURE,
  OBD_PID_ABSOLUTE_THROTTLE_B,
  OBD_PID_ACCELERATOR_PEDAL_POSITION_D,
  OBD_PID_ACCELERATOR_PEDAL_POSITION_E,
  OBD_PID_COMMANDED_THROTTLE_ACTUATOR
};
