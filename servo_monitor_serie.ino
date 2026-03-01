/* Práctica 03 - Brais Gude Martínez
   Fecha: 01/03/2026
   Funcionamiento: Se combinan las tareas 1 y 2 para hacer la tarea 3. Se acciona un servo (Pin 3) mediante un potenciometro (A0)
   y el monitor serie. El monitor tiene prioridad siempre y los accionamientos estan limitados a 6667ms (0.15hz calculado).
   
   Se establece una deteccion de ordenes definidas para el monitor serie tanto en caracteres como en numero. El programa
   detecta posibles errores y no hace nada en caso de ordenes no reconocidas/validas.
   
   Se usa millis en vez de delay porque asi no queda bloqueado el programa (y consecuentemente los unsigned longs para no desbordar)
   Se usa constexpr int porque me parecio curioso (vi que era mas o menos nuevo en C++), no aporta mucho pero da un minimo de rendimiento al calcular en el compilador.
   Finalmente se hace lo que se pide mediante if else y operadores como AND/comparadores.
   Aparte, por decisión personal, añado en el monitor serie que avise cada vez que esté listo para recibir una orden nueva el programa. 
   
   El programa no va muy bien en el monitor de Tinkercad pero sí que funciona en el IDE de Arduino perfectamente.*/

#include <Servo.h>

constexpr int Servo1 = 3;
constexpr int Potenciometro = A0;
constexpr int AnguloMin = 0;
constexpr int AnguloMax = 180;
constexpr int PrecisionAngulo = 10;
constexpr unsigned long PeriodoMiliseg = 6667; // 1000ms / 0.15Hz ≈ 6667ms

Servo servo1;
int AnguloActual = -1; // Posición real actual del servo
int AnguloAnteriorPot = -1; // Última posición leída del potenciómetro
unsigned long UltimoAccionamento = 0; // Guarda el último momento en que se accionó el servo en ms

bool PrimerAccionamento = true; // Omite la espera en el primer accionamiento
bool ListoAnunciado = true; // Verifica si se ha anunciado que se puede enviar una nueva orden tras el último accionamiento

int OrdenDireccion(String orden) { // Convierte un texto a un ángulo. Devuelve -1 si el texto no es reconocido.
  if (orden == "izquierda")          return 0;
  if (orden == "diagonal izquierda") return 45;
  if (orden == "centro")            return 90;
  if (orden == "diagonal derecha")  return 135;
  if (orden == "derecha")           return 180;
  return -1; // Valor de error
}

int leerPotenciometro() { // Lectura del potenciometro y su mapeo para redondear el angulo a la precision definnida
  int EstadoPotenciometro = analogRead(Potenciometro);
  int RedondeoAngulo = map(EstadoPotenciometro, 0, 1023, AnguloMin, AnguloMax);
  return (RedondeoAngulo / PrecisionAngulo) * PrecisionAngulo;
}

// Se lee el monitor serie y te devuelve un angulo si hay orden valida, si no te da un -1 (nada)
int lerSerie() { 
  if (Serial.available() == 0) return -1;

  String orden = Serial.readStringUntil('\n');
  orden.toLowerCase();
  orden.trim();

  boolean esNumero = true;
  for (int i = 0; i < orden.length(); i++) {
    if (!isDigit(orden[i])) {
      esNumero = false;
      break;
    }
  }
// Si es un numero lo convierte a angulo, si no orden de texto: en ambos casos tiene que ser valido, si no no hace nada devolviendo un -1
  if (esNumero && orden.length() > 0) {
    int angulo = (int) orden.toFloat();
    if (angulo >= AnguloMin && angulo <= AnguloMax) return angulo;
    Serial.println("Numero fuera de rango (0-180).");
    return -1;
  }

  int angulo = OrdenDireccion(orden);
  if (angulo == -1) Serial.println("Orden no reconocida: " + orden);
  return angulo;
}

// Control del servo con el monitor serie y el potenciometro con prioridades
void setup() {
  Serial.begin(9600);
  servo1.attach(Servo1);
  servo1.write(0);
  AnguloActual = 0;
  AnguloAnteriorPot = leerPotenciometro(); // Se lee el potenciometro al inicio
  servo1.write(0);
  Serial.println("Servo en 0º. Esperando ordenes...");
}

void loop() {
  int anguloObjetivo = -1;

  // Prioridad 1: monitor serie
  int anguloSerie = lerSerie();
  if (anguloSerie != -1) {
    anguloObjetivo = anguloSerie;
    Serial.print("Orden serie: ");
    Serial.print(anguloObjetivo);
    Serial.println("º");

  // Prioridad 2: potenciometro
  } else {
    int anguloPot = leerPotenciometro();
    if (anguloPot != AnguloAnteriorPot) {
      AnguloAnteriorPot = anguloPot; // Se actualiza la posicion para detectar cambios
      anguloObjetivo   = anguloPot;
    }
    // Si el potenciómetro no cambió, anguloObjetivo sigue siendo -1 y no hace nada
  }

  // Solo se acciona el servo si hay orden válida y el ángulo es diferente al actual + si ha pasado el tiempo mínimo
  unsigned long ahora = millis();
  if (anguloObjetivo != -1 && anguloObjetivo != AnguloActual && (PrimerAccionamento || ahora - UltimoAccionamento >= PeriodoMiliseg)) {
    servo1.write(anguloObjetivo);
    AnguloActual = anguloObjetivo;
    UltimoAccionamento = ahora;
    PrimerAccionamento = false;
    ListoAnunciado = false;
    Serial.print("Servo movido a: ");
    Serial.print(anguloObjetivo);
    Serial.println("º");
  }

  if (!ListoAnunciado && millis() - UltimoAccionamento >= PeriodoMiliseg) {
    Serial.println("Listo para recibir nueva orden.");
    ListoAnunciado = true;
  }
}
