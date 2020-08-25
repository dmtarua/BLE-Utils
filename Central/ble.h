#define BLE_SCAN_TYPE        0x00   // Passive scanning
#define BLE_SCAN_INTERVAL    0x0060 // 60 ms
#define BLE_SCAN_WINDOW      0x0030 // 30 ms

typedef struct {
  uint16_t  connected_handle;
  uint8_t   addr_type;
  bd_addr_t addr;
  struct {
    gatt_client_service_t service;
    struct {
      gatt_client_characteristic_t chars;
      gatt_client_characteristic_descriptor_t descriptor[2];
    } chars[2];  
  } service;
} Device_t;

Device_t device;
uint8_t  chars_index = 0;
uint8_t  desc_index = 0;

// Connect handle.
static uint16_t connected_id = 0xFFFF;

// The service uuid to be discovered.
static uint8_t service1_uuid[16] = { 0x71,0x3d,0x00,0x00,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };
static uint8_t gatt_notify_flag = 0;
