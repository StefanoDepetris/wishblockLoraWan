#include "VariablesHeader.h"

/**
* @brief Configuración inicial del dispositivo.
* 
* Inicializa hardware LoRa, establece región, inicia temporizadores y se une a la red LoRaWAN.
*/
void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial && millis() < SERIAL_INIT_DELAY)
  {
    Serial.println("Cargando..");
    delay(SERIAL_INIT_LOADING);
  }

  initLoRaHardware();
  printWelcomeMessage();
  if (initializeTimers() != SUCCESS)
  {
    Serial.println("Error al inicializar los temporizadores.");
    return;
  }
  lmh_setDevEui(nodeDeviceEUI);
  lmh_setAppEui(nodeAppEUI);
  lmh_setAppKey(nodeAppKey);

  if (lmh_init(&g_lora_callbacks, g_lora_param_init, true, g_CurrentClass, g_CurrentRegion) != SUCCESS)
  {
    Serial.println("Error al inicializar LoRaWAN.");
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
void initLoRaHardware()
{
  lora_hardware_init(hwConfig);
  Wire.begin();
  accel.begin(Wire);
  accel.setOutputDataRate(LIS3DHTR_DATARATE_25HZ);
  lora_rak4630_init();
}

/**
 * @brief Muestra información de bienvenida por medio de Serial.
 */
void printWelcomeMessage()
{
  Serial.println("=====================================");
  Serial.println("Welcome to RAK4630 LoRaWan!!!");
  switch (g_CurrentRegion)
  {
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
 * @return Código de error, 0 si es exitoso.
 */
uint32_t initializeTimers()
{
  uint32_t err_code = timers_init();
  if (err_code != SUCCESS)
  {
    Serial.printf("timers_init failed - %d\n", err_code);
  }
  else
  {
    Serial.printf("timers_init success - %d\n", err_code);
  }
  return err_code;
}

  /**
  * @brief Bucle principal del programa.
  * 
  * Realiza la lectura periódica del acelerómetro y el nivel de batería, y envía datos a la red LoRaWAN.
  */
  void loop()
  {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= LORAWAN_APP_INTERVAL_SAMPLE)
    {
      previousMillis = currentMillis;
      setSampleAcceleration();
      setAvgLevelBattery();
    }
  }

  /**
  * @brief Calcula y almacena el nivel de batería promedio.
  */
  void setAvgLevelBattery()
  {
    float vbat_mv = readVBAT();
    uint8_t vbat_per = mvToPercent(vbat_mv);

    batteryBuffer[batteryBufferIndex] = vbat_per;
    voltiosBuffer[batteryBufferIndex] = vbat_mv;

    batteryBufferIndex = (batteryBufferIndex + 1) % ACCBUFFER_SIZE;
  }

  /**
  * @brief Obtiene una muestra de aceleración y la almacena en el buffer.
  */ 
  void setSampleAcceleration()
  {
    char date[100];
    float x, y, z;
    accel.getAcceleration(&x, &y, &z);
    sprintf(date, "[%.2f,%.2f,%.2f]", x, y, z);
    strcpy(buffer[bufferIndex], date);
    bufferIndex = (bufferIndex + 1) % ACCBUFFER_SIZE;
  }

  /**
  * @brief Manejador para cuando la unión a la red LoRaWAN falla.
  */
  void lorawan_has_joined_handler(void)
  {
    Serial.println("OTAA Mode, Network Joined!");
    
    lmh_error_status ret = lmh_class_request(g_CurrentClass);
    if (ret == LMH_SUCCESS)
    {
      delay(LORAWAN_APP_INTERVAL_SAMPLE);
      TimerSetValue(&appTimer, LORAWAN_APP_INTERVAL);
      TimerStart(&appTimer);
    }
  }

  /**
  * @brief Manejador para cuando la unión a la red LoRaWAN falla.
  */
  void lorawan_join_failed_handler(void)
  {
    Serial.println("OTAA join failed!");
    Serial.println("Check your EUI's and Keys's!");
    Serial.println("Check if a Gateway is in range!");
  }

  /**
  * @brief Manejador para cuando se recibe un paquete LoRaWAN.
  * @param app_data Datos de la aplicación LoRaWAN recibidos.
  */
  void lorawan_rx_handler(lmh_app_data_t *app_data)
  {
    Serial.printf("LoRa Packet received on port %d, size:%d, rssi:%d, snr:%d, data:%s\n",
                  app_data->port, app_data->buffsize, app_data->rssi, app_data->snr, app_data->buffer);
  }

  /**
  * @brief Manejador para cuando se confirma la clase del dispositivo en LoRaWAN.
  * @param Class Clase del dispositivo en LoRaWAN.
  */
  void lorawan_confirm_class_handler(DeviceClass_t Class)
  {
    Serial.printf("switch to class %c done\n", "ABC"[Class]);
    m_lora_app_data.buffsize = 0;
    m_lora_app_data.port = gAppPort;
    lmh_send(&m_lora_app_data, g_CurrentConfirm);
  }

  /**
  * @brief Prepara y envía un frame LoRaWAN con datos del acelerómetro y batería.
  */
  void send_lora_frame(void)
  {
    uint16_t avgBat = 0;
    String accData = "";
    if (lmh_join_status_get() != LMH_SET)
    {
        return;
    }

    // Limpiar el documento JSON
    jsonDocument.clear();

    // Crear un objeto JSON para los datos del acelerómetro
    for (int fila = 0; fila < ACCBUFFER_SIZE; fila++)
    {
        for (int col = 0; col < 100; col++)
        {
            char elemento = buffer[fila][col];
            if (elemento == '\0')
                break;
            accData += String(elemento);
        }
    }

    batteryLevel = calculateBatteryAverage();

    // Agregar el nivel de batería al documento JSON
    jsonDocument["message"] = accData + " " + String(batteryLevel);

    // Serializar el JsonDocument a una cadena
    String jsonString;
    serializeJson(jsonDocument, jsonString);

    // Imprimir la cadena JSON en el puerto serie (opcional)
    Serial.println("Se envía: " + jsonString);

    // Configurar los datos para el envío LoRaWAN
    m_lora_app_data.buffsize = jsonString.length();
    jsonString.getBytes(m_lora_app_data.buffer, m_lora_app_data.buffsize + 1);

    // Enviar los datos por LoRaWAN
    lmh_error_status error = lmh_send(&m_lora_app_data, g_CurrentConfirm);

    switch (error)
    {
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
  * @return El promedio de la batería.
  */
  uint16_t calculateBatteryAverage()
  {
      uint16_t avgBat = 0;
      for (int i = 0; i < ACCBUFFER_SIZE; i++)
      {
          avgBat += batteryBuffer[i];
      }
      return avgBat / ACCBUFFER_SIZE;
  }

  /**
  * @brief Manejador de temporizador para el envío periódico de mensajes LoRaWAN.
  */
  void tx_lora_periodic_handler(void)
  {
    if (batteryLevel <= 15 && previousBatteryLevel > 15)
    {
      TimerSetValue(&appTimer, LORAWAN_APP_INTERVAL);
      Serial.println("BATERIA BAJA");
    }
    else if (batteryLevel > 15 && previousBatteryLevel <= 15)
    {
      TimerSetValue(&appTimer, 10000);
    }

    previousBatteryLevel = batteryLevel;

    TimerStart(&appTimer);
    Serial.println("Sending frame now...");
    send_lora_frame();
  }

  /**
  * @brief Inicializa los temporizadores del sistema.
  * @return 0 si la inicialización es exitosa.
  */
  static uint32_t timers_init(void)
  {
    TimerInit(&appTimer, tx_lora_periodic_handler);
    return 0;
  }

  /**
  * @brief Lee el nivel de batería en milivoltios.
  * @return Nivel de batería en milivoltios.
  */
  float readVBAT(void)
  {
    float raw = analogRead(vbat_pin);
    return raw * REAL_VBAT_MV_PER_LSB;
  }

  /**
  * @brief Convierte milivoltios a porcentaje de batería.
  * @param mvolts Nivel de batería en milivoltios.
  * @return Porcentaje de batería.
  */
 uint8_t mvToPercent(float mvolts)
  {
  if (mvolts < MIN_VOLTAGE)
    return 0;

  if (mvolts < MID_VOLTAGE)
  {
    mvolts -= MIN_VOLTAGE;
    return mvolts / VOLTAGE_STEP_1;
  }

  mvolts -= MID_VOLTAGE;
  return static_cast<uint8_t>(PERCENTAGE_STEP_1 + (mvolts * PERCENTAGE_MULTIPLIER));
  }

  /**
  * @brief Convierte milivoltios a valor de batería LoRaWAN.
  * @param mvolts Nivel de batería en milivoltios.
  * @return Valor de batería para LoRaWAN.
  */
  uint8_t mvToLoRaWanBattVal(float mvolts)
  {
  if (mvolts < MIN_VOLTAGE)
    return 0;

  if (mvolts < MID_VOLTAGE)
  {
    mvolts -= MIN_VOLTAGE;
    return static_cast<uint8_t>(mvolts / VOLTAGE_STEP_1 * LORAWAN_MULTIPLIER);
  }

  mvolts -= MID_VOLTAGE;
  return static_cast<uint8_t>((PERCENTAGE_STEP_1 + (mvolts * PERCENTAGE_MULTIPLIER)) * LORAWAN_MULTIPLIER);
  }
