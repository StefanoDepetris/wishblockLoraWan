const { Decode } = require('./decode')
describe('Decode', () => {
    test('Debe devolver wishblock_movement en False con muestras sin movimiento', () => {
        // FPort simulado
        const fPort = 1;

        // Llamar a la función Decode con los datos de entrada decodificados
        const result = Decode(fPort,  prepareDataMoveFalse());

        // Verificar el resultado
        expect(result.tags.wishblock_movement).toBe("False");
    });

    
    test('Debe devolver wisblock_movement en True con muestras con movimiento', () => {
          // FPort simulado
          const fPort = 1;

          // Llamar a la función Decode con los datos de entrada decodificados
          const result = Decode(fPort, prepareDataMoveTrue());

          // Verificar el resultado
          expect(result.tags.wishblock_movement).toBe("True");
    });

    test('Debe devolver wishblock_battery_level con el porcentaje de bateria pasado en la muestra', () => {
         // FPort simulado
         const fPort = 1;
 
         // Llamar a la función Decode con los datos de entrada decodificados
         const result = Decode(fPort,prepareDataMoveFalse());
      
          // Verificar el resultado
          expect(result.tags.wishblock_battery_level).toBe("140");
    });


    function prepareDataMoveFalse() {
      const muestrasString = "[0.00,-0.05,-1.00][-0.05,-0.05,-0.95][0.00,-0.05,-0.95][0.00,-0.05,-1.00][0.00,-0.10,-1.05][0.05,-0.10,-1.00][0.00,-0.05,-0.95][0.00,-0.05,-0.95][0.00,-0.10,-1.00][0.00,-0.10,-1.00] 140";

      // Convertir las muestras en un objeto JavaScript
      const obj = { message: muestrasString };
  
      // Convertir el objeto JavaScript en una cadena JSON
      const objJSON = JSON.stringify(obj);
  
      // Crear un buffer a partir de la cadena JSON
      const objBytes = Buffer.from(objJSON);
  
      return objBytes;
  }

  function prepareDataMoveTrue() {
    const muestrasString = "[1.00,-0.05,-1.00][-0.05,-0.05,-0.95][1.00,-0.05,-0.95][0.00,-1.05,-1.00][0.00,-0.10,-1.05][0.05,-0.10,-1.00][0.00,-0.05,-0.00][0.00,-0.05,-0.95][0.00,-0.10,-1.00][0.00,-0.10,-1.00] 140";
    // Convertir las muestras en un objeto JavaScript
    const obj = { message: muestrasString };

    // Convertir el objeto JavaScript en una cadena JSON
    const objJSON = JSON.stringify(obj);

    // Crear un buffer a partir de la cadena JSON
    const objBytes = Buffer.from(objJSON);

    return objBytes;
  }

  });