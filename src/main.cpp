#include <Arduino.h>
#include <Servo.h>

//--------------------------- Настройки ------------------------------------
// 

int ButtonPin = 6; // Пин к которому подключен тумблер
int led = 8; // Пин для светодиода подсветки
int ServoPin = 4; // Пин к которому подключено серво

int ServoMinPos = 25; // И.П. рычага
int ServoMaxPos = 120; // Положение, в котором рычаг отключает тумблер

// Число движений, по умолчанию 7
// Если вы добавляли свои движения укажите общее число движений
int MoveNum = 7; // По умолчанию 7

//------------------------ Конец Настроек ---------------------------------

Servo servo1; // Имя серво
int pos1; // Угол серво
int ServoSpeed = 1; // скорость движения серво

int URoBoX = 0; // Номер движения

// Если тумблер по каким-то причинам не может быть выключен
// Включается режим возврата рычага в И.П.
boolean fail = false; // Маркер ошибки выключения тумблера
int failDelay = 3000; // Время до включения режима возврата рычага в И.П. из-за ошибки выключение тумблера
int failCount = 0; // Счетчик неудачных попыток выключений тумблера

int ButtonState = 0; // Статус тумблера 1 = Вкл., 0 = Выкл.
int offdelay = 200; // Задержка до начала движения, разная в каждом движении.

boolean goup = false; // Маркер подъема рычага
boolean godown = false; // Маркер опускания рычага
boolean ON = false; // Маркер включения тумблера

long previousMillis; // Для таймера движений
long delaybeforeoff; // Для таймера задержки до начала движения
long failMillis; // Для таймера запуска режима ошибки выключения тумблера

int MoveStep = 0; // Для учета числа шагов в движении

void setup ()
{
  servo1.attach(ServoPin); // Подключение серво к пину
  servo1.write(ServoMinPos); // Установка серво в И.П.
  
  pinMode(ButtonPin, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(7, false);
  digitalWrite(9, HIGH);
  digitalWrite(led, LOW);
}

void loop(){

  // Таймер
  unsigned long currentMillis = millis();

  // Считывает статус тумблера 1 = Вкл., 0 = Выкл.
  ButtonState = !digitalRead(ButtonPin);

  // Если число неудочных попыток выключения, 3 и более
  // То движения выбиратся не будут
  // Чтобы сбросить число попыток, выключите и включите тумблер
  if (failCount < 3){
    if (URoBoX == 0) {
      URoBoX = random(1, MoveNum + 1);
    }
  }

  // Отключение режима Неудачного Выключение Тумблера
  if (fail && servo1.read() == ServoMinPos){
    ON = false;
    fail = false;
  }
  // Если тумблер выключен, обнуляется счетчик неудачных попыток выключить тумблер
  if (ButtonState == 0){
    failCount = 0;
  }
  
  digitalWrite(led, !ButtonState);

  //--------------------- Движение 1 ------------------------
  // Рычаг быстро выскакивает, выключает и быстро уходит
  if (URoBoX == 1){
    // Тумвлер Включен
    if (ButtonState == 1 && !ON){
      delaybeforeoff = currentMillis;
      failMillis = currentMillis;
      ON = true;
      godown = false;
      ServoSpeed = 1;
      offdelay = 200;
    }

    // Задержка до начала выхода рычага
    if (ON && currentMillis - delaybeforeoff > offdelay){
      goup = true;
    }

    // Выход рычага до отключения тумблера
    if(currentMillis - previousMillis > ServoSpeed && pos1 < ServoMaxPos && goup && !fail){
      previousMillis = currentMillis;
      pos1 ++;
      servo1.write(pos1);
    }

    // Если тумблер Выключен
    if (ButtonState == 0 && goup){
      goup = false;
      godown = true;
      ServoSpeed = 1;
      ON = false;
    }

    // Возвращение рычага в исходную позицию
    if(currentMillis - previousMillis > ServoSpeed && pos1 > ServoMinPos && godown){
      previousMillis = currentMillis;
      pos1 --;
      servo1.write(pos1);
      // Завершение движения
      if (godown && servo1.read() == ServoMinPos){
        URoBoX = 0;
      }
    }

  }
  //--------------------- Конец Движения 1 ------------------------

  //--------------------- Движение 2 ------------------------
  // Медленный выход рычага, задержка под крышкой и резкое выключение тумблера.
  if (URoBoX == 2){
    // Тумблер Включен
    if (ButtonState == 1 && !ON){
      delaybeforeoff = currentMillis;
      failMillis = currentMillis;
      ON = true;
      godown = false;
      ServoSpeed = 30;
      offdelay = 1;
      MoveStep = 0;
    }

    // Задержка до начала движения
    if (ON && currentMillis - delaybeforeoff > offdelay){
      goup = true;
    }

    // Медленное движение рычага до поднятия крышки
    if(currentMillis - previousMillis > ServoSpeed && MoveStep == 0 && goup && !fail){
      previousMillis = currentMillis;
      pos1 ++;
      servo1.write(pos1);
      if (pos1 == ServoMaxPos - 50){
        MoveStep ++;
      }
    }

    // Задержка рычага под крышкой
    if(currentMillis - previousMillis > 900 && MoveStep == 1 && goup && !fail){
      previousMillis = currentMillis;
      MoveStep ++;
      ServoSpeed = 1;
    }

    // Быстрое движение рычага до отключения тумблера
    if(currentMillis - previousMillis > ServoSpeed && pos1 < ServoMaxPos && MoveStep == 2 && goup && !fail){
      previousMillis = currentMillis;
      pos1 ++;
      servo1.write(pos1);
    }

    // Тумблер Выключен
    if (ButtonState == 0 && goup){
      goup = false;
      godown = true;
      ServoSpeed = 1;
      ON = false;
    }

    // Быстрое возвращение рычага в И.П.
    if(currentMillis - previousMillis > ServoSpeed && pos1 > ServoMinPos && godown){
      previousMillis = currentMillis;
      pos1 --;
      servo1.write(pos1);
      // Завершение движения
      if (godown && servo1.read() == ServoMinPos){
        URoBoX = 0;
      }
    }


  }
  //--------------------- Конец Движения 2 ------------------------

  //--------------------- Движение 3 ------------------------
  // Медленный выход рычага и резкий рывок из-под крышки, выключение тумблера и быстрое возвращение.
  if (URoBoX == 3){
    // Тумблер включен
    if (ButtonState == 1 && !ON){
      delaybeforeoff = currentMillis;
      failMillis = currentMillis;
      ON = true;
      godown = false;
      ServoSpeed = 30;
      offdelay = 1;
    }

    // Задержка до начала движения
    if (ON && currentMillis - delaybeforeoff > offdelay){
      goup = true;
    }

    // Медленный выход рычага
    if(currentMillis - previousMillis > ServoSpeed && pos1 < ServoMaxPos && goup && !fail){
      previousMillis = currentMillis;
      pos1 ++;
      servo1.write(pos1);
      // Быстрое движение до отключения тумблера
      if (pos1 == ServoMaxPos - 50){
        ServoSpeed = 1;
      }
    }

    // Тумблер Выключен
    if (ButtonState == 0 && goup){
      goup = false;
      godown = true;
      ServoSpeed = 1;
      ON = false;
    }

    // Возвращение рычага в И.П.
    if(currentMillis - previousMillis > ServoSpeed && pos1 > ServoMinPos && godown){
      previousMillis = currentMillis;
      pos1 --;
      servo1.write(pos1);
      // Завершение движения
      if (godown && servo1.read() == ServoMinPos){
        URoBoX = 0;
      }
    }


  }
  //--------------------- Конец Движения 3 ------------------------

  //--------------------- Движение 4 ------------------------
  // Медленный выход рычага, выключение тумблера и медленное возвращение
  if (URoBoX == 4){
    // Тумблер включен
    if (ButtonState == 1 && !ON){
      delaybeforeoff = currentMillis;
      failMillis = currentMillis;
      ON = true;
      godown = false;
      ServoSpeed = 5;
      offdelay = 1;
    }

    // Задержка до начала движения
    if (ON && currentMillis - delaybeforeoff > offdelay){
      goup = true;
    }

    // Выход рычага до поднятия крышки
    if(currentMillis - previousMillis > ServoSpeed && pos1 < ServoMaxPos && goup && !fail){
      previousMillis = currentMillis;
      pos1 ++;
      servo1.write(pos1);
      // Замедление скорости после поднятия крышки
      if (pos1 == ServoMaxPos - 50){
        ServoSpeed = 20;
      }
      // Ускорение перед выключением тумблера
      // На медленной скорости может не выключить
      if (pos1 == ServoMaxPos - 15){
        ServoSpeed = 1;
      }
    }

    // Тумблер Выключен
    if (ButtonState == 0 && goup){
      goup = false;
      godown = true;
      ServoSpeed = 20;
      ON = false;
    }

    // Медленное возвращение рычага под крышку
    if(currentMillis - previousMillis > ServoSpeed && pos1 > ServoMinPos && godown){
      previousMillis = currentMillis;
      pos1 --;
      servo1.write(pos1);
      // Ускорение рычага
      if (pos1 == ServoMaxPos - 50){
        ServoSpeed = 1;
      }
      // Завершение движения
      if (godown && servo1.read() == ServoMinPos){
        URoBoX = 0;
      }
    }


  }
  //--------------------- Конец Движения 4 ------------------------

  //--------------------- Движение 5 ------------------------
  // Быстрый выход рычага до тумблера, задержка у тумблера (без выключения)
  // Медленный уход под крышку, быстрый выход, отключение тумблера и быстрый возврат
  if (URoBoX == 5){
    // Тумблер Включен
    if (ButtonState == 1 && !ON){
      delaybeforeoff = currentMillis;
      failMillis = currentMillis;
      ON = true;
      godown = false;
      ServoSpeed = 1;
      offdelay = 100;
      MoveStep = 0;
      failDelay = 6000;
    }

    // Задержка до начала движения
    if (ON && currentMillis - delaybeforeoff > offdelay){
      goup = true;
    }

    // Шаг 0 - Выход рычага до тумблера
    if(currentMillis - previousMillis > ServoSpeed && pos1 < ServoMaxPos - 15 && goup && MoveStep == 0 && !fail){
      previousMillis = currentMillis;
      pos1 ++;
      servo1.write(pos1);
      // Изменение номера шага и скорости
      if (pos1 == ServoMaxPos - 15){
        MoveStep ++;
        ServoSpeed = 20;
      }
    }

    // Шаг 1 - Задержка у рычага у тумблера
    if(currentMillis - previousMillis > 1500 && goup && MoveStep == 1 && !fail){
      previousMillis = currentMillis;
      MoveStep ++;
      ServoSpeed = 20;
    }

    // Шаг 2 - Медленный уход рычага под крышку
    if(currentMillis - previousMillis > ServoSpeed && pos1 > ServoMinPos && goup && MoveStep == 2 && !fail){
      previousMillis = currentMillis;
      pos1 --;
      servo1.write(pos1);
      // Изменение номера шага и скорости
      if (pos1 == ServoMinPos){
        MoveStep ++;
        ServoSpeed = 1;
      }
    }

    // Шаг 3 - Быстрое выключение тумблера
    if(currentMillis - previousMillis > ServoSpeed && pos1 < ServoMaxPos && goup && MoveStep == 3 && !fail){
      previousMillis = currentMillis;
      pos1 ++;
      servo1.write(pos1);
    }

    // Тумблер Выключен
    if (ButtonState == 0 && goup){
      goup = false;
      godown = true;
      ServoSpeed = 1;
      ON = false;
    }

    // Возвращение рычага в И.П.
    if(currentMillis - previousMillis > ServoSpeed && pos1 > ServoMinPos && godown){
      previousMillis = currentMillis;
      pos1 --;
      servo1.write(pos1);
      // Завершение движения
      if (godown && servo1.read() == ServoMinPos){
        URoBoX = 0;
        failDelay = 3000;
      }
    }

  }
  //--------------------- Конец Движения 5 ------------------------

  //--------------------- Движение 6 ------------------------
  // Рычаг медленно приподнимает крышку, задержка, медленно уходит обратно
  // После задержки быстрый выход и выключение тумблера
  if (URoBoX == 6){
    // Тумблер включен
    if (ButtonState == 1 && !ON){
      delaybeforeoff = currentMillis;
      failMillis = currentMillis;
      ON = true;
      godown = false;
      ServoSpeed = 10;
      offdelay = 200;
      MoveStep = 0;
    }

    // Задержка до начала движения
    if (ON && currentMillis - delaybeforeoff > offdelay){
      goup = true;
    }

    // Шаг 0 - Выход рычага до поднятия крышки
    if(currentMillis - previousMillis > ServoSpeed && pos1 < ServoMaxPos && MoveStep == 0 && goup && !fail){
      previousMillis = currentMillis;
      pos1 ++;
      servo1.write(pos1);
      // Изменение номера шага
      if (pos1 == ServoMaxPos - 60){
        MoveStep ++;
      }
    }

    // Шаг 1 - Задержка рычага под поднятой крышкой
    if(currentMillis - previousMillis > 500 && MoveStep == 1 && goup && !fail){
      previousMillis = currentMillis;
      MoveStep ++;
    }

    // Шаг 2 - Возврат рычага в И.П
    if(currentMillis - previousMillis > ServoSpeed && MoveStep == 2 && goup && !fail){
      previousMillis = currentMillis;
      pos1 --;
      servo1.write(pos1);
      // Изменение номера шага
      if (pos1 == ServoMinPos){
        MoveStep ++;
      }
    }

    // Шаг 3 - Задержка рычага в И.П.
    if(currentMillis - previousMillis > 700 && MoveStep == 3 && goup && !fail){
      previousMillis = currentMillis;
      MoveStep ++;
      ServoSpeed = 1;
    }

    // Шаг 4 - Быстрое движение рычага до отключения тумблера
    if(currentMillis - previousMillis > ServoSpeed && pos1 < ServoMaxPos && MoveStep == 4 && goup && !fail){
      previousMillis = currentMillis;
      pos1 ++;
      servo1.write(pos1);
    }

    // Тумблер Выключен
    if (ButtonState == 0 && goup){
      goup = false;
      godown = true;
      ServoSpeed = 1;
      ON = false;
    }

    // Возврат рычага в И.П.
    if(currentMillis - previousMillis > ServoSpeed && pos1 > ServoMinPos && godown){
      previousMillis = currentMillis;
      pos1 --;
      servo1.write(pos1);
      // Завершение движения
      if (godown && servo1.read() == ServoMinPos){
        URoBoX = 0;
      }
    }

  }
  //--------------------- Конец Движения 6 ------------------------

  //--------------------- Движение 7 ------------------------
  // Рычаг быстро выключает тумблер, затем медленно не полностью уходит под крышку
  // После задержки под крышкой, быстрый возврат тумблера в И.П.
  if (URoBoX == 7){
    // Тумблер Включен
    if (ButtonState == 1 && !ON){
      delaybeforeoff = currentMillis;
      failMillis = currentMillis;
      ON = true;
      godown = false;
      ServoSpeed = 1;
      offdelay = 100;
      MoveStep = 0;
    }

    // Задержка до начала движения
    if (ON && currentMillis - delaybeforeoff > offdelay){
      goup = true;
    }

    // Выход рычага до отключения тумблера
    if(currentMillis - previousMillis > ServoSpeed && pos1 < ServoMaxPos && goup && !fail){
      previousMillis = currentMillis;
      pos1 ++;
      servo1.write(pos1);
    }

    // Тумблер Выключен
    if (ButtonState == 0 && goup){
      goup = false;
      godown = true;
      ServoSpeed = 10;
      ON = false;
    }

    // Шаг 0 - Возврат рычага под крышку
    if(currentMillis - previousMillis > ServoSpeed && MoveStep == 0 && godown){
      previousMillis = currentMillis;
      pos1 --;
      servo1.write(pos1);
      // Изменение номера шага
      if (pos1 == ServoMaxPos - 40){
        MoveStep ++;
      }
    }

    // Шаг 1 - Задержка рычага под открытой крышкой
    if(currentMillis - previousMillis > 500 && MoveStep == 1 && godown){
      previousMillis = currentMillis;
      MoveStep ++;
      ServoSpeed = 1;
    }

    // Шаг 2 - Возврат рычага в И.П.
    if(currentMillis - previousMillis > ServoSpeed && pos1 > ServoMinPos && MoveStep == 2 && godown){
      previousMillis = currentMillis;
      pos1 --;
      servo1.write(pos1);
      // Завершение движения
      if (godown && servo1.read() == ServoMinPos){
        URoBoX = 0;
      }
    }

  }
  //--------------------- Конец Движения 7 ------------------------

  // Если тумблер не отключается, рычаг возвращается в И.П.
  if (currentMillis - failMillis > failDelay && goup && !fail){
    goup = false;
    godown = true;
    ServoSpeed = 1;
    fail = true;
    failCount ++;
  }

}