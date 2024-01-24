#include <AUnit.h>

//Test de funcion que calcula el promedio de las muestras.
test(calculateBatteryAverageTest) {
  uint32_t batteryBufferMock[ACCBUFFERSIZE] = { 80, 85, 90, 95, 100, 105, 110, 115, 120, 125 };

  uint32_t result = calculateBatteryAverage(batteryBufferMock);

  uint32_t expected = (80 + 85 + 90 + 95 + 100 + 105 + 110 + 115 + 120 + 125) / ACCBUFFERSIZE;
  assertEqual(result, expected);
}

//Test de funcion que convierte milivoltios a porcentaje de batería.
test(mvToPercentTestMinVoltage) {
  float mvoltsMock1 = 3000.0;

  uint8_t result1 = mvToPercent(mvoltsMock1);

  assertEqual(result1, 0);
}

//Test de funcion que convierte milivoltios a porcentaje de batería.
test(mvToPercentTestMidVoltage) {
  float mvoltsMock1 = 3400.0;

  uint8_t result1 = mvToPercent(mvoltsMock1);

  uint8_t expected1 = static_cast<uint8_t>((mvoltsMock1- MIN_VOLTAGE) / VOLTAGE_STEP_1);
  assertEqual(result1, expected1);
}


//Test de funcion que convierte milivoltios a porcentaje de batería.
test(mvToPercentTest) {
  float mvoltsMock1 = 3700.0;

  uint8_t result1 = mvToPercent(mvoltsMock1);

  uint8_t expected1 = static_cast<uint8_t>(PERCENTAGE_STEP_1 + ((mvoltsMock1-MID_VOLTAGE) * PERCENTAGE_MULTIPLIER));
  assertEqual(result1, expected1);
}