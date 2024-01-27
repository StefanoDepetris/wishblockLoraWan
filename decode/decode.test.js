const { Decode } = require('./decode')
describe('Decode', () => {
    test('Debe devolver wiblock_movement en False con muestras sin movimiento', () => {
      // Mock data
      const fPort = 1;
      const obj = {
        'message': '[1.00,-0.10,0.00][1.00,-0.05,-0.05][1.00,-0.10,0.00][1.00,-0.10,0.00][1.00,-0.10,0.00][1.00,-0.10,0.00][1.00,-0.10,0.00][1.00,-0.10,0.00][1.00,-0.10,0.00][1.00,-0.10,0.00] 100'
      };
      
      // Llamar a la función
      const result = Decode(fPort, obj);
  
      // Verificar el resultado
      expect(result.tags.wishblock_movement).toBe("False");
    });

    test('Debe devolver wisblock_movement en True con muestras con movimiento', () => {
        // Mock data
        const fPort = 1;
        const obj = {
          'message': '[1.00,-0.10,0.00][-1.00,-0.05,-0.05][1.00,-0.10,0.00][1.00,1.00,0.00][1.00,-0.10,0.00][1.00,-0.10,0.00][1.00,-0.10,1.00][1.00,-0.10,1.00][1.00,-0.10,0.00][1.00,-0.10,0.00] 100'
        };
        
        // Llamar a la función
        const result = Decode(fPort, obj);
    
        // Verificar el resultado
        expect(result.tags.wishblock_movement).toBe("True");
      });

      test('Debe devolver wishblock_battery_level con el porcentaje de bateria pasado en la muestra', () => {
        // Mock data
        const fPort = 1;
        const obj = {
          'message': '[1.00,-0.10,0.00][-1.00,-0.05,-0.05][1.00,-0.10,0.00][1.00,1.00,0.00][1.00,-0.10,0.00][1.00,-0.10,0.00][1.00,-0.10,1.00][1.00,-0.10,1.00][1.00,-0.10,0.00][1.00,-0.10,0.00] 88%'
        };
        
        // Llamar a la función
        const result = Decode(fPort, obj);
    
        // Verificar el resultado
        expect(result.tags.wishblock_battery_level).toBe("88");
      });
  });