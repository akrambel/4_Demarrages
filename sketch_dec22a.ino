#define BLYNK_TEMPLATE_ID "TMPL2Mb2k2S1H"
#define BLYNK_TEMPLATE_NAME "TP"
#define BLYNK_AUTH_TOKEN "b_piDT_QxCelhx0XatJZK-FD259DEW9t"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "TP-LINK_41EEBC";
char pass[] = "LINA@2024@";

// États des relais et modes
bool sens1Active = false;    // Sens 1 actif (KM1)
bool sens2Active = false;    // Sens 2 actif (KM2)
bool etoileActive = false;   // Phase étoile active (KM4)
bool triangleActive = false; // Phase triangle active (KM3)
bool modeStatorique = false; // Mode statorique actif
bool modeRotorique = false;  // Mode rotorique actif
bool pvActive = false;       // Petite vitesse active
bool gvActive = false;       // Grande vitesse active
bool etoileTriangleActive = false; // Étoile-triangle actif
bool statoriqueActive = false;     // Statorique actif
bool rotoriqueActive = false;      // Rotorique actif
bool systemRunning = false;        // Système en fonctionnement ou non

// Pins des relais
#define RELAIS_SENS1 D0  // KM1
#define RELAIS_SENS2 D1  // KM2
#define RELAIS_PHASE1 D2 // KM3 (triangle, GV ou phase 3)
#define RELAIS_PHASE2 D3 // KM4 (étoile, rotorique ou phase 2)
#define RELAIS_PHASE3 D4 // KM5 (GV - phase 3)
#define RELAIS_ET D5     // KM6 (étoile-triangle)
#define RELAIS_ST D6     // KM7 (statorique)
#define RELAIS_RT D7     // KM8 (rotorique)

SimpleTimer timer;
int timerId = -1; // ID du temporisateur en cours

// Fonction d'arrêt général
void stopAll() {
  if (timerId != -1) {
    timer.deleteTimer(timerId); // Arrête la temporisation en cours
    timerId = -1;
  }

  digitalWrite(RELAIS_SENS1, HIGH);
  digitalWrite(RELAIS_SENS2, HIGH);
  digitalWrite(RELAIS_PHASE1, HIGH);
  digitalWrite(RELAIS_PHASE2, HIGH);
  digitalWrite(RELAIS_PHASE3, HIGH);
  digitalWrite(RELAIS_ET, HIGH); // Désactiver relais 6 (étoile-triangle)
  digitalWrite(RELAIS_ST, HIGH); // Désactiver relais 7 (statorique)
  digitalWrite(RELAIS_RT, HIGH); // Désactiver relais 8 (rotorique)

  sens1Active = false;
  sens2Active = false;
  etoileActive = false;
  triangleActive = false;
  etoileTriangleActive = false;
  statoriqueActive = false;
  rotoriqueActive = false;
  pvActive = false;
  gvActive = false;
  systemRunning = false;
}

// Petite vitesse (PV)
void startPV(bool sens1) {
  stopAll();
  if (sens1) {
    digitalWrite(RELAIS_SENS1, LOW);
    sens1Active = true;
  } else {
    digitalWrite(RELAIS_SENS2, LOW);
    sens2Active = true;
  }

  digitalWrite(RELAIS_PHASE1, LOW); // Activation de la petite vitesse (KM3)
  pvActive = true;
  systemRunning = true;
}

// Grande vitesse (GV)
void startGV(bool sens1) {
  stopAll();
  if (sens1) {
    digitalWrite(RELAIS_SENS1, LOW);
    sens1Active = true;
  } else {
    digitalWrite(RELAIS_SENS2, LOW);
    sens2Active = true;
  }

  digitalWrite(RELAIS_PHASE2, LOW); // Activation initiale de la grande vitesse (KM4)
  gvActive = true;
  systemRunning = true;

  timerId = timer.setTimeout(3000, []() {
    digitalWrite(RELAIS_PHASE2, HIGH); // Désactivation de KM4 après 3 secondes
    digitalWrite(RELAIS_PHASE3, LOW); // Activation de KM5
    timerId = -1; // Réinitialise l'ID du timer
  });
}

// Démarrage étoile-triangle
void startStarTriangle(bool sens1) {
  stopAll();
  digitalWrite(RELAIS_ET, LOW); // Activation de KM6 (étoile-triangle)
  etoileTriangleActive = true; // Marquer le démarrage en étoile-triangle
  if (sens1) {
    digitalWrite(RELAIS_SENS1, LOW);
    sens1Active = true;
  } else {
    digitalWrite(RELAIS_SENS2, LOW);
    sens2Active = true;
  }

  digitalWrite(RELAIS_PHASE2, LOW); // Phase étoile active (KM4)
  etoileActive = true;
  systemRunning = true;

  timerId = timer.setTimeout(3000, []() {
    digitalWrite(RELAIS_PHASE2, HIGH); // Désactivation de l'étoile (KM4)
    etoileActive = false;

    digitalWrite(RELAIS_PHASE1, LOW); // Activation du triangle (KM3)
    triangleActive = true;
    timerId = -1;
  });
}

// Démarrage statorique
void startStatorique(bool sens1) {
  stopAll();
  digitalWrite(RELAIS_ST, LOW); // Activation de KM7 (statorique)
  statoriqueActive = true; // Marquer le démarrage statorique
  if (sens1) {
    digitalWrite(RELAIS_SENS1, LOW);
    sens1Active = true;
  } else {
    digitalWrite(RELAIS_SENS2, LOW);
    sens2Active = true;
  }

  timerId = timer.setTimeout(3000, []() {
    digitalWrite(RELAIS_PHASE1, LOW); // Activation de KM3
    modeStatorique = true;
    systemRunning = true;
    timerId = -1;
  });
}

// Démarrage rotorique
void startRotorique(bool sens1) {
  stopAll();
  digitalWrite(RELAIS_RT, LOW); // Activation de KM8 (rotorique)
  rotoriqueActive = true; // Marquer le démarrage rotorique
  if (sens1) {
    digitalWrite(RELAIS_SENS1, LOW);
    sens1Active = true;
  } else {
    digitalWrite(RELAIS_SENS2, LOW);
    sens2Active = true;
  }

  timerId = timer.setTimeout(3000, []() {
    digitalWrite(RELAIS_PHASE2, LOW); // Activation de KM4
    modeRotorique = true;
    systemRunning = true;
    timerId = -1;
  });
}

// Boutons Blynk
BLYNK_WRITE(V0) { if (param.asInt() && !systemRunning) startStarTriangle(true); }
BLYNK_WRITE(V1) { if (param.asInt() && !systemRunning) startStarTriangle(false); }
BLYNK_WRITE(V3) { if (param.asInt() && !systemRunning) startStatorique(true); }
BLYNK_WRITE(V4) { if (param.asInt() && !systemRunning) startStatorique(false); }
BLYNK_WRITE(V5) { if (param.asInt() && !systemRunning) startRotorique(true); }
BLYNK_WRITE(V6) { if (param.asInt() && !systemRunning) startRotorique(false); }
BLYNK_WRITE(V7) { if (param.asInt() && !systemRunning) startPV(true); }
BLYNK_WRITE(V8) { if (param.asInt() && !systemRunning) startPV(false); }
BLYNK_WRITE(V9) { if (param.asInt() && !systemRunning) startGV(true); }
BLYNK_WRITE(V10) { if (param.asInt() && !systemRunning) startGV(false); }
BLYNK_WRITE(V2) { if (param.asInt()) stopAll(); }

void setup() {
  Serial.begin(115200);
  pinMode(RELAIS_SENS1, OUTPUT);
  pinMode(RELAIS_SENS2, OUTPUT);
  pinMode(RELAIS_PHASE1, OUTPUT);
  pinMode(RELAIS_PHASE2, OUTPUT);
  pinMode(RELAIS_PHASE3, OUTPUT);
  pinMode(RELAIS_ET, OUTPUT);
  pinMode(RELAIS_ST, OUTPUT);
  pinMode(RELAIS_RT, OUTPUT);
  stopAll();
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
}

void loop() {
  Blynk.run();
  timer.run();
}



void grandeVitesse(int sens) {
    if (sens == 1) { // Sens 1 activé par V9
        digitalWrite(relais1, HIGH);
        digitalWrite(relais3, HIGH);
        delay(3000); // Attente de 3 secondes
        digitalWrite(relais1, LOW);
        digitalWrite(relais3, LOW);
        digitalWrite(relais4, HIGH);
        delay(1000); // Attente de 1 seconde
        digitalWrite(relais5, HIGH);
        digitalWrite(relais1, HIGH);
    } else if (sens == 2) { // Sens 2 activé par V10
        digitalWrite(relais2, HIGH);
        digitalWrite(relais3, HIGH);
        delay(3000); // Attente de 3 secondes
        digitalWrite(relais2, LOW);
        digitalWrite(relais3, LOW);
        digitalWrite(relais4, HIGH);
        delay(1000); // Attente de 1 seconde
        digitalWrite(relais5, HIGH);
        digitalWrite(relais2, HIGH);
    }
}
