function Decode(fPort, obj) {
    var msg = "";
    var data = {};
    var umbral_variacion = 0.1;
    var muestras = [];
    var huboMovimiento;
    
    console.log("Contenido de obj:", obj);

    msg = obj.message;
  
    // Convertir el objeto a cadena
    var dataString = JSON.stringify(msg);
  
    // Encontrar la posición del último corchete
    var lastBracketPosition = dataString.lastIndexOf(']');
  
    // Obtener el nivel de batería desde la cadena
    var battery = dataString.substring(lastBracketPosition + 1, dataString.length - 2).trim();
  
    // Eliminar el nivel de batería de dataString
    dataString = dataString.substring(0, lastBracketPosition + 1);
  
    // Separar las muestras
    var muestrasString = dataString.split("][");
  
    // Procesar las muestras
    var moduloAnterior = 0.0;
    var moduloMovimiento = 0.0;
    var contadorMovimiento = 0;
  
    for (var i = 0; i < muestrasString.length; i++) {
      // Eliminar corchetes al principio y al final de cada muestra
      var muestra = muestrasString[i].replace("[", "").replace("]", "");
  
      // Obtener las lecturas como un array de strings
      var lecturas = muestra.split(",");
  
      // Convertir las lecturas a valores double
      var lecturaX = parseFloat(lecturas[0]);
      var lecturaY = parseFloat(lecturas[1]);
      var lecturaZ = parseFloat(lecturas[2]);
  
      // Calcular el módulo tridimensional
      var modulo = Math.sqrt(lecturaX * lecturaX + lecturaY * lecturaY + lecturaZ * lecturaZ);
  
      // Comparar con el valor anterior solo si no es la primera muestra
      if (moduloAnterior !== 0.0 && Math.abs(modulo - moduloAnterior) > umbral_variacion) {
        contadorMovimiento++;
        moduloMovimiento=Math.abs(modulo - moduloAnterior)- umbral_variacion;
      }
  
      // Actualizar el valor anterior
      moduloAnterior = modulo;
    }
    
    // Determinar si hubo movimiento
    if (contadorMovimiento >= 3) {
      huboMovimiento = "True";
    } else {
      huboMovimiento = "False";
      moduloMovimiento=0.0;
    }
  
    // Agregar los tags al objeto de datos
    data.tags = {
      wishblock_battery_level: battery,
      wiblock_movement: huboMovimiento,
      wishblock_magnitud: moduloMovimiento,
    };
  
    // Devolver el objeto de datos modificado
    return data;
  }

  module.exports={
    Decode
  }