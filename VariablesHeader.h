// VariablesHeader.h

#ifndef VARIABLES_HEADER_H
#define VARIABLES_HEADER_H

// Librerías y estructuras
#include <Arduino.h>
#include <LoRaWan-RAK4630.h>
#include <SPI.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <LIS3DHTR.h>

#define SUCCESS 0

// Librerías y estructuras
LIS3DHTR<TwoWire> accel;
hw_config hwConfig;

// Constantes setup
#define  SERIAL_BAUD_RATE  115200
#define SERIAL_INIT_DELAY  5000
#define  SERIAL_INIT_LOADING 100
#define  ANALOG_READ_RESOLUTION 12
const eAnalogReference ANALOG_REFERENCE_TYPE = AR_INTERNAL_3_0;


// Constantes de tiempo y LoRaWAN
#define  LORAWAN_APP_INTERVAL_SAMPLE  1000
#define  LOW_BATTERY_INTERVAL  20000  
#define  LORAWAN_APP_INTERVAL 10000 

unsigned long previousMillis = 0;

// Configuración LoRaWAN
#define SCHED_MAX_EVENT_DATA_SIZE APP_TIMER_SCHED_EVENT_DATA_SIZE
#define SCHED_QUEUE_SIZE 60
#define LORAWAN_DATERATE DR_5
#define LORAWAN_TX_POWER TX_POWER_5
#define JOINREQ_NBTRIALS 3

// Variables de estado LoRaWAN
DeviceClass_t g_CurrentClass = CLASS_A;
LoRaMacRegion_t g_CurrentRegion = LORAMAC_REGION_AU915;
lmh_confirm g_CurrentConfirm = LMH_UNCONFIRMED_MSG;
uint8_t gAppPort = LORAWAN_APP_PORT;

// Claves OTAA
uint8_t nodeDeviceEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x06, 0x70, 0xD8};
uint8_t nodeAppEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x06, 0x70, 0xD8};
uint8_t nodeAppKey[16] = {0x35, 0x16, 0x61, 0x7A, 0x0B, 0x87, 0x9F, 0x43, 0xEF, 0x49, 0x09, 0xDD, 0x97, 0xEF, 0x94, 0xA3};

// Buffers y variables de batería
const unsigned long LORAWAN_APP_DATA_BUFF_SIZE = 1000;
uint8_t m_lora_app_data_buffer[LORAWAN_APP_DATA_BUFF_SIZE];
lmh_app_data_t m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0};

#define ACCBUFFERSIZE   10
#define DATABUFFERSIZE  100
unsigned long bufferIndex = 0;
char buffer[ACCBUFFERSIZE][100];

TimerEvent_t appTimer;
uint32_t batteryBuffer[ACCBUFFERSIZE];
unsigned long batteryBufferIndex = 0;
float voltiosBuffer[ACCBUFFERSIZE];
uint8_t batteryLevel = 100;
uint8_t previousBatteryLevel = 100;

// Configuración de batería y voltaje
const uint32_t PIN_VBAT = WB_A0;
uint32_t vbat_pin = PIN_VBAT;
const float VBAT_MV_PER_LSB = 0.73242188F;
const float VBAT_DIVIDER_COMP = 1.73;
const float REAL_VBAT_MV_PER_LSB = VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB;

// Contadores
uint8_t count = 0;
uint8_t count_fail = 0;

// JSON Document
const size_t JSON_BUFFER_SIZE = 512;
StaticJsonDocument<JSON_BUFFER_SIZE> jsonDocument;

// Constantes de voltaje y porcentaje
const float MIN_VOLTAGE = 3300.0;
const float MID_VOLTAGE = 3600.0;
const float VOLTAGE_STEP_1 = 30.0;
const float VOLTAGE_STEP_2 = 0.15;
const float PERCENTAGE_STEP_1 = 10.0;
const float PERCENTAGE_MULTIPLIER = 0.15;
const float LORAWAN_MULTIPLIER = 2.55;
#define  BATTERY_LOW_THRESHOLD 15  

// Parámetros LoRaWAN
lmh_param_t g_lora_param_init = {LORAWAN_ADR_OFF, LORAWAN_DATERATE, LORAWAN_PUBLIC_NETWORK, JOINREQ_NBTRIALS, LORAWAN_TX_POWER, LORAWAN_DUTYCYCLE_OFF};

// Declaración de funciones
void handlerSuccessJoinLorawan(void);
void handlerFailedJoinLorawan(void);
void handlerReceiveLorawan(lmh_app_data_t *app_data);
void handlerConfirmClassLorawan(DeviceClass_t Class);
void sendLoraFrame(void);
void initLoRaHardware(void);
void printWelcomeMessage(void);
void initializeTimers(void);
uint16_t calculateBatteryAverage();

// Callbacks LoRaWAN
lmh_callback_t g_lora_callbacks = {BoardGetBatteryLevel, BoardGetUniqueId, BoardGetRandomSeed,
                                            handlerReceiveLorawan, handlerSuccessJoinLorawan, handlerConfirmClassLorawan, handlerFailedJoinLorawan};

#endif 