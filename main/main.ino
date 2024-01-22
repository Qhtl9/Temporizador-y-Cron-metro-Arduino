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
bool Botones_estados[3] = { false, false, false };

int opc = 0;
int i;
int* Cron_pro;
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

void Actualizar_botones(bool Limpiar) {
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
      if (Limpiar) {
        oled.clear();
      }
    }
  }
}

void Temporizador(int Fecha_inicial, int Temporizador_[3]) {  // segundos desde medianoche de 1/1/1970
}

void Temporizador_personalizado() {
  Serial.print("Success 1\n");
  Serial.println("Success 1.5");
}

void Cronometro() {
  inicio = myRTC.now();
  Actualizar_botones(false);
  oled.clear();
  while (!Botones_estados[0]) {
    now = myRTC.now();
    Cron_pro = Segundos_a_tiempo(now.unixtime() - inicio.unixtime());

    oled.setCursor(0, 0);
    sprintf(Tiempo_formato, "  %02d:%02d:%02d", Cron_pro[0], Cron_pro[1], Cron_pro[2]);
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

  Actualizar_botones(true);

  if (Botones_estados[1]) {
    opc = (opc + 1) % opc_to;
  }

  if (Botones_estados[2]) {
    opc = (opc - 1 + opc_to) % opc_to;
  }

  oled.setCursor(0, 0);
  if (opc < N_temp) {
    Cron_pro = Temporizadores[opc];
    sprintf(Tiempo_formato, "  %02d:%02d:%02d", Cron_pro[0], Cron_pro[1], Cron_pro[2]);
    oled.print(Tiempo_formato);
  } else {
    oled.print(cprint_oled(Opciones_extra[opc - N_temp], 11));
    if (Botones_estados[0]) {
      Funcs[opc - N_temp]();
    }
  }
}