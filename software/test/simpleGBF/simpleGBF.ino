//********************************************************************
// 2023 Djevahirdjian Léo
//
// SIMPLE GBF
//
// Un petit test/exercice pour prendre en main Mozzi afin de coder un
// GBF deux voies assez simple et qui permettra, éventuellement,
// d'explorer les limites de génération du système.
//
//********************************************************************



//********************************************************************
//     DEFINITIONS - Interface Humain-Machine
//********************************************************************
// Encodeur rotatif
# define ROT_PIN0 6
# define ROT_PIN1 7
#include <RotaryEncoder.h>
RotaryEncoder *interface_encodeurRotatif = nullptr ;

void callback_updateEncodeurRotatif(){
  interface_encodeurRotatif->tick() ;
}

//********************************************************************
//     DEFINITIONS - Mozzi
//********************************************************************
// Template oscillateur
#include <Oscil.h>
// Chargement des tables pour oscillateurs
#include <tables/saw2048_int8.h>
#include <tables/sin2048_int8.h>

// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> Oscillateur_Scie1(SAW2048_DATA) ;
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> Oscillateur_Scie2(SAW2048_DATA) ;

int CH1_frequence = 440 ;
int CH2_frequence = 220 ;

// Fonction de lecture des contrôle et de mise à jour des paramètres
// de synthèse. Appelée à la fréquence CONTROL_RATE
void updateControl() {
  Oscillateur_Scie1.setFreq(CH1_frequence) ;
  Oscillateur_Scie2.setFreq(CH2_frequence) ;
}

// Fonction de génération de la sortie
AudioOutput_t updateAudio() {
  return MonoOutput::fromNBit(9, Oscillateur_Scie2.next()) ;
}



//********************************************************************
//     COEUR n°0 - Mozzi
//********************************************************************
void setup() {
  startMozzi(CONTROL_RATE) ;
}

void loop() {
  audioHook() ;
}



//********************************************************************
//     COEUR n°1 - Interface Homme-Machine
//********************************************************************
void setup1(void) {
  Serial.begin(115200) ;
  delay(1000) ;
  Serial.println("SimpleGBF démarre...") ;

  pinMode(ROT_PIN0, INPUT_PULLUP) ;
  pinMode(ROT_PIN1, INPUT_PULLUP) ;

  interface_encodeurRotatif = new RotaryEncoder(ROT_PIN1, ROT_PIN0, RotaryEncoder::LatchMode::FOUR3) ;

  attachInterrupt(digitalPinToInterrupt(ROT_PIN0), callback_updateEncodeurRotatif, CHANGE) ;
  attachInterrupt(digitalPinToInterrupt(ROT_PIN1), callback_updateEncodeurRotatif, CHANGE) ;
}

void loop1(){
  static int encodeurRotatif_positionMemoire = 0 ;

  int encodeurRotatif_Position = interface_encodeurRotatif->getPosition() ;
  if (encodeurRotatif_positionMemoire != encodeurRotatif_Position) {
    Serial.print("pos:") ;
    Serial.print(encodeurRotatif_Position) ;
    Serial.print(" dir:") ;
    Serial.println((int)(interface_encodeurRotatif->getDirection())) ;
    encodeurRotatif_positionMemoire = encodeurRotatif_Position ;
  }
}
