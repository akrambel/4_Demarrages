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
bool sens1Active = false;
bool sens2Active = false;
bool etoileActive = false;
bool triangleActive = false;
bool modeStatorique = false;
bool modeRotorique = false;
bool pvActive = false;
bool gvActive = false;
bool etoileTriangleActive = false;
bool statoriqueActive = false;
bool rotoriqueActive = false;
bool systemRunning = false;

// Pins des relais
#define RELAIS_SENS1 D0
#define RELAIS_SENS2 D1
#define RELAIS_PHASE1 D2
#define RELAIS_PHASE2 D3
#define RELAIS_PHASE3 D4
#define RELAIS_ET D5
#define RELAIS_ST D6
#define RELAIS_RT D7

SimpleTimer timer;
int timerId = -1;

void stopAll() {
  if (timerId != -1) {
    timer.deleteTimer(timerId);
    timerId = -1;
  }

  digitalWrite(RELAIS_SENS1, HIGH);
  digitalWrite(RELAIS_SENS2, HIGH);
  digitalWrite(RELAIS_PHASE1, HIGH);
  digitalWrite(RELAIS_PHASE2, HIGH);
  digitalWrite(RELAIS_PHASE3, HIGH);
  digitalWrite(RELAIS_ET, HIGH);
  digitalWrite(RELAIS_ST, HIGH);
  digitalWrite(RELAIS_RT, HIGH);

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


void startGV(bool sens1) {
  stopAll();

  if (sens1) {
    // Si le sens 1 est choisi, activer uniquement le relais 1
    digitalWrite(RELAIS_SENS1, LOW); // Activation relais 1
    digitalWrite(RELAIS_PHASE1, LOW); // Activation relais 3
    sens1Active = true;
  } else {
    // Si le sens 2 est choisi, activer uniquement le relais 2
    digitalWrite(RELAIS_SENS2, LOW); // Activation relais 2
    digitalWrite(RELAIS_PHASE1, LOW); // Activation relais 3
    sens2Active = true;
  }

  gvActive = true;
  systemRunning = true;

  // Temporisation pour désactiver relais 1/2 et 3, puis activer relais 4
  timerId = timer.setTimeout(3000, []() {
    // Désactivation de relais 1 et 2
    digitalWrite(RELAIS_SENS1, HIGH);
    digitalWrite(RELAIS_SENS2, HIGH);
    digitalWrite(RELAIS_PHASE1, HIGH); // Désactivation relais 3
    // Activation relais 4
    digitalWrite(RELAIS_PHASE2, LOW);

    // Temporisation pour activer relais 5 et 1/2
    timer.setTimeout(1000, []() {
      digitalWrite(RELAIS_PHASE3, LOW); // Activation relais 5
      if (sens1Active) {
        digitalWrite(RELAIS_SENS1, LOW); // Réactivation relais 1
      } else {
        digitalWrite(RELAIS_SENS2, LOW); // Réactivation relais 2
      }
    });
  });
}

// Autres démarrages (inchangés)
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

void startStarTriangle(bool sens1) {
  stopAll();
  digitalWrite(RELAIS_ET, LOW); // Activation de KM6 (étoile-triangle)
  etoileTriangleActive = true;
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

void startStatorique(bool sens1) {
  stopAll();
  digitalWrite(RELAIS_ST, LOW); // Activation de KM7 (statorique)
  statoriqueActive = true;
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

void startRotorique(bool sens1) {
  stopAll();
  digitalWrite(RELAIS_RT, LOW); // Activation de KM8 (rotorique)
  rotoriqueActive = true;
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
