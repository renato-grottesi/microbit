/*
  Arduino Mario Bros Tunes
  With Piezo Buzzer and PWM
  by: Dipto Pratyaksa
  last updated: 31/3/13
*/

//Mario main theme melody
uint8_t mario_melody[] = {
  NOTE::E7, NOTE::E7, NOTE::Z0, NOTE::E7, 
  NOTE::Z0, NOTE::C7, NOTE::E7, NOTE::Z0,
  NOTE::G7, NOTE::Z0, NOTE::Z0,  NOTE::Z0,
  NOTE::G6, NOTE::Z0, NOTE::Z0, NOTE::Z0, 

  NOTE::C7, NOTE::Z0, NOTE::Z0, NOTE::G6, 
  NOTE::Z0, NOTE::Z0, NOTE::E6, NOTE::Z0, 
  NOTE::Z0, NOTE::A6, NOTE::Z0, NOTE::B6, 
  NOTE::Z0, NOTE::AS6, NOTE::A6, NOTE::Z0, 

  NOTE::G6, NOTE::E7, NOTE::G7, 
  NOTE::A7, NOTE::Z0, NOTE::F7, NOTE::G7, 
  NOTE::Z0, NOTE::E7, NOTE::Z0,NOTE::C7, 
  NOTE::D7, NOTE::B6, NOTE::Z0, NOTE::Z0,

  NOTE::C7, NOTE::Z0, NOTE::Z0, NOTE::G6, 
  NOTE::Z0, NOTE::Z0, NOTE::E6, NOTE::Z0, 
  NOTE::Z0, NOTE::A6, NOTE::Z0, NOTE::B6, 
  NOTE::Z0, NOTE::AS6, NOTE::A6, NOTE::Z0, 

  NOTE::G6, NOTE::E7, NOTE::G7, 
  NOTE::A7, NOTE::Z0, NOTE::F7, NOTE::G7, 
  NOTE::Z0, NOTE::E7, NOTE::Z0,NOTE::C7, 
  NOTE::D7, NOTE::B6, NOTE::Z0, NOTE::Z0
};
//Mario main them tempo
uint8_t mario_tempo[] = {
  12, 12, 12, 12, 
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12, 

  12, 12, 12, 12,
  12, 12, 12, 12, 
  12, 12, 12, 12, 
  12, 12, 12, 12, 

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
};

//
//Underworld melody
uint8_t underworld_melody[] = {
  NOTE::C4, NOTE::C5, NOTE::A3, NOTE::A4, 
  NOTE::AS3, NOTE::AS4, NOTE::Z0,
  NOTE::Z0,
  NOTE::C4, NOTE::C5, NOTE::A3, NOTE::A4, 
  NOTE::AS3, NOTE::AS4, NOTE::Z0,
  NOTE::Z0,
  NOTE::F3, NOTE::F4, NOTE::D3, NOTE::D4,
  NOTE::DS3, NOTE::DS4, NOTE::Z0,
  NOTE::Z0,
  NOTE::F3, NOTE::F4, NOTE::D3, NOTE::D4,
  NOTE::DS3, NOTE::DS4, NOTE::Z0,
  NOTE::Z0, NOTE::DS4, NOTE::CS4, NOTE::D4,
  NOTE::CS4, NOTE::DS4, 
  NOTE::DS4, NOTE::GS3,
  NOTE::G3, NOTE::CS4,
  NOTE::C4, NOTE::FS4,NOTE::F4, NOTE::E3, NOTE::AS4, NOTE::A4,
  NOTE::GS4, NOTE::DS4, NOTE::B3,
  NOTE::AS3, NOTE::A3, NOTE::GS3,
  NOTE::Z0, NOTE::Z0, NOTE::Z0
};
//Underwolrd tempo
uint8_t underworld_tempo[] = {
  12, 12, 12, 12, 
  12, 12, 6,
  3,
  12, 12, 12, 12, 
  12, 12, 6,
  3,
  12, 12, 12, 12, 
  12, 12, 6,
  3,
  12, 12, 12, 12, 
  12, 12, 6,
  6, 18, 18, 18,
  6, 6,
  6, 6,
  6, 6,
  18, 18, 18,18, 18, 18,
  10, 10, 10,
  10, 10, 10,
  3, 3, 3
};


