/*
 Práctica 03 - Brais Gude Martínez
   Fecha: 01/03/2026
   Funcionamiento: Este programa simula el comportamiento de un brazo robótico de dos ejes y pinza controlado de forma independiente por tres potenciómetros:
     - Servo A (acimut, eje horizontal):
         Gira el brazo a la izquierda o derecha (plano horizontal, 0°–180°).
         Entrada: potenciómetro en el pin analógico A0.
         Salida: servo en el pin digital 9.
         Precisión: 12° (el servo solo se mueve en saltos de 12 en 12 grados).
     - Servo B (elevación, eje vertical):
         Sube y baja el brazo sobre el horizonte (0°–180°).
         Entrada: potenciómetro en el pin analógico A1.
         Salida: servo en el pin digital 10.
         Precisión: 5° (el servo solo se mueve en saltos de 5 en 5 grados).
     - Servo C (pinza):
         Abre y cierra la pinza del brazo (0°–180°).
         Entrada: potenciómetro en el pin analógico A2.
         Salida: servo en el pin digital 11.
         Precisión: 0.5° (usa writeMicroseconds() para mayor resolución).
   El programa se actualiza un máximo de 8 veces por segundo (cada 125 ms) porque así lo pide el enunciado.
   Entradas analógicas: A0 (acimut), A1 (elevación), A2 (pinza)
   Salidas PWM: Pin 9 (acimut), Pin 10 (elevación), Pin 11 (pinza)
*/
#include <Servo.h>

// Potenciómetros
constexpr int PotAcimut    = A0;
constexpr int PotElevacion = A1;
constexpr int PotPinza     = A2;

// Servos
constexpr int ServoAcimut    =  9;
constexpr int ServoElevacion = 10;
constexpr int ServoPinza     = 11;

// Tiempos
constexpr unsigned long PeriodoMilisegundos = 125; // Máximo 8 accionamentos

// Precisión de cada servo en º (Los saltos que da al moverse)
constexpr float PasoAcimut    = 12.0;
constexpr float PasoElevacion =  5.0;
constexpr float PasoPinza     =  0.5;

Servo servoAcimut;
Servo servoElevacion;
Servo servoPinza;

// Ángulos anteriores para detectar cambios (inicializados a -1 para forzar la primera escritura)
float anguloAcimutAnterior    = -1;
float anguloElevacionAnterior = -1;
float anguloPinzaAnterior     = -1;

// Redondea un ángulo al múltiplo más próximo
float Redondeo(float angulo, float paso) {
  return round(angulo / paso) * paso;
}

// Conversión de grados a microsegundos.
// Como write solo acepta enteros, se multiplica por 1000 para usar la parte decimal como precision
int AnguloaMS(float grados) {
  return (int)(1000.0 + (grados / 180.0) * 1000.0);
}

void setup() {
  Serial.begin(9600);
  servoAcimut.attach(ServoAcimut);
  servoElevacion.attach(ServoElevacion);
  servoPinza.attach(ServoPinza);
  Serial.println("Maquinote listo");
}

void loop() {
  // Lectura de los potenciómetros, analogread tira un valor entre 0 y 1023
  int valorAcimut    = analogRead(PotAcimut);
  int valorElevacion = analogRead(PotElevacion);
  int valorPinza     = analogRead(PotPinza);

  // Conversión a grados con un mapeado (0º a 180º)
  float anguloAcimut    = (float)map(valorAcimut,    0, 1023, 0, 180);
  float anguloElevacion = (float)map(valorElevacion, 0, 1023, 0, 180);
  float anguloPinza = (valorPinza / 1023.0) * 180.0;

  // Redondeo de precisión para los saltos de los servos. En el servo C es un poco irrelevante porque nunca se va a notar.
  anguloAcimut    = Redondeo(anguloAcimut,    PasoAcimut);
  anguloElevacion = Redondeo(anguloElevacion, PasoElevacion);
  anguloPinza     = Redondeo(anguloPinza,     PasoPinza);

  // Solo actúa si algún ángulo ha cambiado respecto al ciclo anterior
  if (anguloAcimut    != anguloAcimutAnterior    ||
      anguloElevacion != anguloElevacionAnterior ||
      anguloPinza     != anguloPinzaAnterior) {

    // Escritura a los servos. El C se controla con ms para más precisión, por eso lo del comentario anterior.
    servoAcimut.write((int)anguloAcimut);
    servoElevacion.write((int)anguloElevacion);
    servoPinza.writeMicroseconds(AnguloaMS(anguloPinza));

    // Información del estado actual x serie
    Serial.print("Acimut: ");      Serial.print(anguloAcimut);
    Serial.print(" grados  |  Elev: "); Serial.print(anguloElevacion);
    Serial.print(" grados  |  Pinza: "); Serial.print(anguloPinza);
    Serial.println(" grados");

    // Actualización de los ángulos anteriores
    anguloAcimutAnterior    = anguloAcimut;
    anguloElevacionAnterior = anguloElevacion;
    anguloPinzaAnterior     = anguloPinza;
  }

  // Espera para limitar a frecuencia de accionamento a un máximo de 8 veces por segundo
  delay(PeriodoMilisegundos);
}