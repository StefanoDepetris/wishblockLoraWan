#include "VariablesHeader.h"

/**
* @brief Configuración inicial del dispositivo.
* Inicializa hardware LoRa, establece región, inicia temporizadores y se une a la red LoRaWAN.
*/
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial && millis() < SERIAL_INIT_DELAY) {
    Serial.println("Loading..");
    delay(SERIAL_INIT_LOADING);
  }

  initLoRaHardware();
  printWelcomeMessage();
  initializeTimers();

  lmh_setDevEui(nodeDeviceEUI);
  lmh_setAppEui(nodeAppEUI);
  lmh_setAppKey(nodeAppKey);

  if (lmh_init(&g_lora_callbacks, g_lora_param_init, true, g_CurrentClass, g_CurrentRegion) != SUCCESS) {
    Serial.println("Error initializing LoRaWAN.");
    return;
  }
  analogReference(ANALOG_REFERENCE_TYPE);
  analogReadResolution(ANALOG_READ_RESOLUTION);
  readVBAT();
  lmh_join();
}

/**
 * @brief Inicializa el hardware relacionado con LoRa.
 */
void initLoRaHardware() {
  lora_hardware_init(hwConfig);
  Wire.begin();
  accel.begin(Wire);
  accel.setOutputDataRate(LIS3DHTR_DATARATE_25HZ);
  lora_rak4630_init();
}

/**
 * @brief Muestra información de bienvenida por medio de Serial.
 */
void printWelcomeMessage() {
  Serial.println("=====================================");
  Serial.println("Welcome to RAK4630 LoRaWan!!!");
  switch (g_CurrentRegion) {
    case LORAMAC_REGION_AU915:
      Serial.println("Region: AU915");
      break;
    case LORAMAC_REGION_US915:
      Serial.println("Region: US915");
      break;
  }
  Serial.println("=====================================");
}

/**
 * @brief Inicializa temporizadores.
 */
void initializeTimers() {
  TimerInit(&appTimer, txLoraPeriodicHandler);
  Serial.println("Timers_init success.");
}

/**
  * @brief Bucle principal del programa.
  * Realiza la lectura periódica del acelerómetro y el nivel de batería, y envía datos a la red LoRaWAN.
  */
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= LORAWAN_APP_INTERVAL_SAMPLE) {
    previousMillis = currentMillis;
    setSampleAcceleration();
    setAvgLevelBattery();
  }
   aunit::TestRunner::run();
}

/**
  * @brief Calcula y almacena el nivel de batería promedio.
  */
void setAvgLevelBattery() {
  float vbat_mv = readVBAT();
  uint8_t vbat_per = mvToPercent(vbat_mv);

  batteryBuffer[batteryBufferIndex] = vbat_per;
  voltiosBuffer[batteryBufferIndex] = vbat_mv;

  batteryBufferIndex = (batteryBufferIndex + 1) % ACCBUFFERSIZE;
}

/**
  * @brief Obtiene una muestra de aceleración y la almacena en el buffer.
  */
void setSampleAcceleration() {
  char date[DATABUFFERSIZE];
  float x, y, z;
  accel.getAcceleration(&x, &y, &z);
  sprintf(date, "[%.2f,%.2f,%.2f]", x, y, z);
  strcpy(buffer[bufferIndex], date);
  bufferIndex = (bufferIndex + 1) % ACCBUFFERSIZE;
}

/**
  * @brief Manejador para cuando la unión a la red LoRaWAN falla.
  */
void handlerSuccessJoinLorawan(void) {
  Serial.println("OTAA Mode, Network Joined!");
  lmh_error_status ret = lmh_class_request(g_CurrentClass);
  if (ret == LMH_SUCCESS) {
    delay(LORAWAN_APP_INTERVAL_SAMPLE);
    TimerSetValue(&appTimer, LORAWAN_APP_INTERVAL);
    TimerStart(&appTimer);
  }
}

/**
  * @brief Manejador para cuando la unión a la red LoRaWAN falla.
  */
void handlerFailedJoinLorawan(void) {
  Serial.println("OTAA join failed!");
  Serial.println("Check your EUI's and Keys's!");
  Serial.println("Check if a Gateway is in range!");
}

/**
  * @brief Manejador para cuando se recibe un paquete LoRaWAN.
  * @param app_data Datos de la aplicación LoRaWAN recibidos.
  */
void handlerReceiveLorawan(lmh_app_data_t* app_data) {
  Serial.printf("LoRa Packet received on port %d, size:%d, rssi:%d, snr:%d, data:%s\n",
                app_data->port, app_data->buffsize, app_data->rssi, app_data->snr, app_data->buffer);
}

/**
  * @brief Manejador para cuando se confirma la clase del dispositivo en LoRaWAN.
  * @param Class Clase del dispositivo en LoRaWAN.
  */
void handlerConfirmClassLorawan(DeviceClass_t Class) {
  Serial.printf("switch to class %c done\n", "ABC"[Class]);
  m_lora_app_data.buffsize = 0;
  m_lora_app_data.port = gAppPort;
  lmh_send(&m_lora_app_data, g_CurrentConfirm);
}

/**
  * @brief Prepara y envía un frame LoRaWAN con datos del acelerómetro y batería.Crea un JsonObject para los datos del acelerometro y bateria
  */
void sendLoraFrame(void) {
  if (lmh_join_status_get() != LMH_SET) {
    return;
  }

  memset(m_lora_app_data.buffer, 0, LORAWAN_APP_DATA_BUFF_SIZE);
  m_lora_app_data.port = gAppPort;

  String accData = "";
  for (int fila = 0; fila < ACCBUFFERSIZE; fila++) {
    for (int col = 0; col < DATABUFFERSIZE; col++) {
      char elemento = buffer[fila][col];
      if (elemento == '\0') {
        break;
      }
      accData += String(elemento);
    }
  }

  batteryLevel = calculateBatteryAverage(batteryBuffer);
  jsonDocument["message"] = accData + " " + String(batteryLevel);

  Serial.print("Sending: ");
  serializeJson(jsonDocument, Serial);
  Serial.println();

  size_t size = measureJson(jsonDocument);
  Serial.print("Size: ");
  Serial.println(size);
  if (size > LORAWAN_APP_DATA_BUFF_SIZE) {
    Serial.println("Error: El tamaño del JSON excede el tamaño del buffer");
    return;
  }

  serializeJson(jsonDocument, m_lora_app_data.buffer, size);
  m_lora_app_data.buffsize = size;
  jsonDocument.clear();  // Libera la memoria del documento JSON

  // Enviar los datos por LoRaWAN
  lmh_error_status status = lmh_send(&m_lora_app_data, g_CurrentConfirm);

  switch (status) {
    case LMH_SUCCESS:
      count++;
      Serial.printf("lmh_send ok count %d\n", count);
      break;
    case LMH_BUSY:
      count_fail++;
      Serial.printf("lmh_send fail <BUSY> count %d\n", count_fail);
      break;
    case LMH_ERROR:
      count_fail++;
      Serial.printf("lmh_send fail <SIZE_ERROR> count %d\n", m_lora_app_data.buffsize);
      break;
  }
}

/**
 * @brief Calcula el promedio de la batería.
 * @param batteryBuffer Puntero al buffer de la batería.
 * @return El promedio de la batería.
 */
uint32_t calculateBatteryAverage(const uint32_t* batteryBuffer) {
  uint32_t avgBat = 0;
  for (int i = 0; i < ACCBUFFERSIZE; i++) {
    avgBat += batteryBuffer[i];
  }
  return avgBat / ACCBUFFERSIZE;
}

/**
  * @brief Manejador de temporizador para el envío periódico de mensajes LoRaWAN.
  */
void txLoraPeriodicHandler(void) {
  if (batteryLevel <= BATTERY_LOW_THRESHOLD && previousBatteryLevel > BATTERY_LOW_THRESHOLD) {
    TimerSetValue(&appTimer, LOW_BATTERY_INTERVAL);
  } else if (batteryLevel > BATTERY_LOW_THRESHOLD && previousBatteryLevel <= BATTERY_LOW_THRESHOLD) {
    TimerSetValue(&appTimer, LORAWAN_APP_INTERVAL);
  }
  previousBatteryLevel = batteryLevel;
  TimerStart(&appTimer);
  Serial.println("Sending frame now...");
  sendLoraFrame();
}

/**
  * @brief Lee el nivel de batería en milivoltios.
  * @return Nivel de batería en milivoltios.
  */
float readVBAT(void) {
  float raw = analogRead(vbat_pin);
  return raw * REAL_VBAT_MV_PER_LSB;
}

/**
  * @brief Convierte milivoltios a porcentaje de batería.
  * @param mvolts Nivel de batería en milivoltios.
  * @return Porcentaje de batería.
  */
uint8_t mvToPercent(float mvolts) {
  if (mvolts < MIN_VOLTAGE)
    return 0;

  if (mvolts < MID_VOLTAGE) {
    mvolts -= MIN_VOLTAGE;
    return mvolts / VOLTAGE_STEP_1;
  }

  mvolts -= MID_VOLTAGE;
  return static_cast<uint8_t>(PERCENTAGE_STEP_1 + (mvolts * PERCENTAGE_MULTIPLIER));
}

