function Decode(fPort, obj) {
  var msg = "";
  var data = {};
  var threshold_variation  = 0.1;
  var wasMovement;

  // Obtener el mensaje como cadena
  for (var i = 0; i < obj.length; i++) {
    msg += (String.fromCharCode(obj[i]));
  }
  
  // Parsear el objeto JSON desde la cadena
  data = JSON.parse(msg);

  // Convertir el objeto a cadena
  var dataString = JSON.stringify(data);

  // Encontrar la posición del último corchete
  var lastBracketPosition = dataString.lastIndexOf(']');

  // Obtener el nivel de batería desde la cadena
  var battery = dataString.substring(lastBracketPosition + 1, dataString.length - 2).trim();


  // Eliminar el nivel de batería de dataString
  dataString = dataString.substring(0, lastBracketPosition + 1);

  // Separar las muestras
  var muestrasString = dataString.split("][");

  // Procesar las muestras
  var previousModule = 0.0;
  var movementModule = 0.0;
  var movementCounter  = 0;

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
    if (previousModule !== 0.0 && Math.abs(modulo - previousModule) > threshold_variation) {
      movementCounter ++;
      movementModule=Math.abs(modulo - previousModule)- threshold_variation ;
    }

    // Actualizar el valor anterior
    previousModule = modulo;
  }
  
  // Determinar si hubo movimiento
  if (movementCounter>= 3) {
    wasMovement = "True";
  } else {
    wasMovement = "False";
    movementModule=0.0;
  }

  // Agregar los tags al objeto de datos
  data.tags = {
    wishblock_battery_level: battery,
    wishblock_movement: wasMovement,
    wishblock_magnitude: movementModule,
  };

  // Devolver el objeto de datos modificado
  return data;
}

module.exports={
  Decode
}