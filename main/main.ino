#include <Tiny4kOLED.h>

#include <Wire.h>
#include <DS3231.h>

RTClib myRTC;
DateTime now;
DateTime inicio;

#define Buzzer 6  // Buzzer pin

#define N_temp 5                // Número de temporizadores
#define opc_ex 2                // Número de opciones extra en el menú (Además de los temporizadores)
#define opc_to opc_ex + N_temp  // Número de opciones en el menú

int Temporizadores[N_temp][3] = { { 0, 5, 0 },  // Horas, minutos, segundos
                                  { 0, 10, 0 },
                                  { 0, 15, 0 },
                                  { 0, 30, 0 },
                                  { 1, 0, 0 } };

String Opciones_extra[opc_ex] = { "Custom",
                                  "Timer" };

bool Botones_actuales[3] = { false, false, false };
bool Botones_archivados[3] = { false, false, false };
bool Botones_estados[3] = { false, false, false };  // Verde, rojo, negro

int counter = 0;
unsigned long milisegundos_anteriores = 0;
int opc = 0;
int i;
int* Cronometro_progreso;
char Tiempo_formato[9];

String cprint_oled(String Original, int Tamano_linea) {
  int pad = (Tamano_linea - Original.length()) / 2;
  if (pad > 0) {
    String Salida = "";
    for (i = 0; i < pad; i++) {
      Salida += " ";
    }
    Original = Salida + Original;
  }
  return Original;
}

int Tiempo_a_segundos(int horas, int minutos, int segundos) {
  return (horas * 3600) + (minutos * 60) + segundos;
}

int* Segundos_a_tiempo(unsigned long unixTime) {
  // Calculate hours, minutes, and seconds
  unsigned long hours = (unixTime / 3600) % 24;
  unsigned long minutes = (unixTime / 60) % 60;
  unsigned long seconds = unixTime % 60;

  // Create an integer array to store the result
  static int timeArray[3];  // Index 0: hours, Index 1: minutes, Index 2: seconds
  timeArray[0] = static_cast<int>(hours);
  timeArray[1] = static_cast<int>(minutes);
  timeArray[2] = static_cast<int>(seconds);

  // Return the integer array
  return timeArray;
}

void Actualizar_botones(bool Limpiar_pantalla) {
  for (i = 0; i < 3; i++) {
    Botones_actuales[i] = digitalRead(i + 8);
    Botones_estados[i] = false;
    if (Botones_actuales[i] != Botones_archivados[i]) {
      /*Serial.print("Cambio detectado: ");
      Serial.print(i);
      Serial.print(" de ");
      Serial.print(Botones_archivados[i]);
      Serial.print(" a ");
      Serial.println(Botones_actuales[i]);*/

      Botones_archivados[i] = Botones_actuales[i];

      if (Botones_archivados[i] == true) {
        Botones_estados[i] = !Botones_estados[i];
      }
    }

    if (Botones_estados[i]) {
      analogWrite(Buzzer, (i * 100) + 100);
      delay(50);
      analogWrite(Buzzer, 0);
      if (Limpiar_pantalla) {
        oled.clear();
      }
    }
  }
}

void Temporizador(int horas, int minutos, int segundos) {
  inicio = myRTC.now();
  Actualizar_botones(true);
  now = myRTC.now();  // En teoría no se puede llegar hasta aquí sin que la variable esté actualizada, pero de igual manera se actualiza por precaución

  unsigned long fin;
  fin = inicio.unixtime() + Tiempo_a_segundos(horas, minutos, segundos);
  bool pausado = false;
  pausado = false;

  while (!Botones_estados[0] && now.unixtime() <= fin) {
    now = myRTC.now();
    Cronometro_progreso = Segundos_a_tiempo(fin - now.unixtime());

    oled.setCursor(0, 0);
    sprintf(Tiempo_formato, "  %02d:%02d:%02d", Cronometro_progreso[0], Cronometro_progreso[1], Cronometro_progreso[2]);
    oled.print(Tiempo_formato);
    Actualizar_botones(false);

    if (Botones_estados[1]) {
      pausado = true;
    }

    while (pausado) {
      Actualizar_botones(false);

      if (Botones_estados[1]) {
        pausado = false;
        fin = fin - now.unixtime();
        now = myRTC.now();
        fin = now.unixtime() + fin;
      }

      if (Botones_estados[0]) {
        break;
      }
    }
  }

  if (!Botones_estados[0]) {
    Actualizar_botones(true);
    oled.clear();
    oled.setCursor(0, 0);
    oled.print(cprint_oled("Done!", 11));
    while (!Botones_estados[0]) {
      if (millis() - milisegundos_anteriores > 166) {
        counter++;
        milisegundos_anteriores = millis();
      }

      switch (counter % 6) {
        case 1:
          analogWrite(Buzzer, 25);
          break;
        case 3:
          analogWrite(Buzzer, 25);
          break;
        default:
          analogWrite(Buzzer, 0);
          break;
      }

      Actualizar_botones(true);
    }
    analogWrite(Buzzer, 0);
  }
}

void Temporizador_personalizado() {
  Serial.print("Success 1\n");
  Serial.println("Success 1.5");
}

void Cronometro() {
  inicio = myRTC.now();
  Actualizar_botones(true);

  while (!Botones_estados[0]) {
    now = myRTC.now();
    Cronometro_progreso = Segundos_a_tiempo(now.unixtime() - inicio.unixtime());

    oled.setCursor(0, 0);
    sprintf(Tiempo_formato, "  %02d:%02d:%02d", Cronometro_progreso[0], Cronometro_progreso[1], Cronometro_progreso[2]);
    oled.print(Tiempo_formato);
    Actualizar_botones(true);
  }
}

void (*Funcs[opc_ex])() = { &Temporizador_personalizado,  // Funciones de las opciones extra
                            &Cronometro };



void setup() {
  oled.begin(128, 64, sizeof(tiny4koled_init_128x64r), tiny4koled_init_128x64r);
  oled.setFontX2Smooth(FONT6X8P);
  oled.clear();
  oled.on();

  for (i = 8; i < 11; i++) {
    pinMode(i, INPUT);
  }

  pinMode(Buzzer, OUTPUT);

  Serial.begin(9600);
  Wire.begin();
  delay(500);
}

void loop() {
  now = myRTC.now();

  Serial.println(now.unixtime());

  Actualizar_botones(true);

  if (Botones_estados[1]) {
    opc++;
  }

  if (Botones_estados[2]) {
    opc--;
  }

  if (opc >= opc_to) {
    opc = 0;
  }

  if (opc < 0) {
    opc = opc_to - 1;
  }

  oled.setCursor(0, 0);
  if (opc < N_temp) {
    Cronometro_progreso = Temporizadores[opc];  // No funciona si no se hace así, no tengo idea de por qué
    sprintf(Tiempo_formato, "  %02d:%02d:%02d", Cronometro_progreso[0], Cronometro_progreso[1], Cronometro_progreso[2]);
    oled.print(Tiempo_formato);

    if (Botones_estados[0]) {
      Cronometro_progreso = Temporizadores[opc];  // No funciona si no se hace así, no tengo idea de por qué
      Temporizador(Cronometro_progreso[0], Cronometro_progreso[1], Cronometro_progreso[2]);
      oled.clear();
    }
  } else {
    oled.print(cprint_oled(Opciones_extra[opc - N_temp], 11));
    if (Botones_estados[0]) {
      Funcs[opc - N_temp]();
      oled.clear();
    }
  }
}