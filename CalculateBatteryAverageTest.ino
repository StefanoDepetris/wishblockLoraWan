// #include <AUnit.h>

// // Define el nombre de la suite de pruebas
// testSuite(BatteryTests);

// // Define el caso de prueba para calculateBatteryAverage
// test(calculateBatteryAverageTest) {
//   // Configura tus datos de prueba, asegurándote de que ACCBUFFERSIZE sea mayor que 0
//   const int ACCBUFFERSIZE = 10;
//   uint16_t batteryBufferMock[ACCBUFFERSIZE] = { 80, 85, 90, 95, 100, 105, 110, 115, 120, 125 };

//   // Llama a la función que estás probando
//   uint16_t result = calculateBatteryAverage();

//   // Verifica si el resultado es el esperado
//   uint16_t expected = (80 + 85 + 90 + 95 + 100 + 105 + 110 + 115 + 120 + 125) / ACCBUFFERSIZE;
//   assertEqual(result, expected);
// }

// // Agrega esto al final de tu archivo para ejecutar las pruebas
// void setup() {
//   Serial.begin(115200);
//   while (!Serial)
//     ;
// }

// void loop() {
//   aunit::TestRunner::run();
// }