/*
  BalticSlobodaHomeOS
  ver 2.24a

  ----Пофикшено и добавлено----
  Пофикшено: Избавились от ненужных переменных
  Добавлено: Поддержание температуры в заданных пределах (Не видит температуру)
  ----Пофикшено и добавлено----
*/

//----Библиотеки----
// библиотека для работы с GPRS устройством
#include <GPRS_Shield_Arduino.h>
// билиотека для работы с EEPROM
#include <avr/eeprom.h>
//----Библиотеки----

// создаём объект класса GPRS и передаём в него объект Serial1
GPRS gprs(Serial1);
//----Сообщения----
// длина сообщения
#define MESSAGE_LENGTH 160
// текст сообщения о включении системы
#define MESSAGE_ON  "On"
// текст сообщения о выключении системы
#define MESSAGE_OFF  "Off"
// текст сообщения о состоянии розетки
#define MESSAGE_STATE  "State"
// текст сообщения о запросе температуры
#define MESSAGE_TEMP  "Temp"
// текст сообщения о включении системы на 10 градусов
#define MESSAGE_ON10  "On10"
// текст сообщения о включении системы на 20 градусов
#define MESSAGE_ON20  "On20"
// текст сообщения о включении системы на 25 градусов
#define MESSAGE_ON25  "On25"
//----Сообщения----
//----Датчик температуры----
#define temperaturePin 2  // определение пина подключения датчика температуры
#define VCC 5.0           // напряжениe питания датчика VCC (5 В или 3.3 В)
//----Датчик температуры----
//----Переменные----
#define REL1 5
#define REL2 8
#define REL3 9
// текст сообщения
char message[MESSAGE_LENGTH];
// номер, с которого пришло сообщение
char phone[16];
// дата отправки сообщения
char datetime[24];
bool stateRelay = false;
char stat;
float temp;
float Tempr1;
char Tempr2;

//----Переменные----
void setup()
{
  pinMode(REL1, OUTPUT);         //назначаем 5 цифр. пин как выход
  pinMode(REL2, OUTPUT);        //назначаем 8 цифр. пин как выход
  pinMode(REL3, OUTPUT);       //назначаем 9 цифр. пин как выход

  Off(); //Вырубить релюхи


  // открываем последовательный порт для мониторинга действий в программе
  Serial.begin(9600);
  // включаем GPRS шилд
  gprs.powerOn();

  // открываем последовательный порт для мониторинга действий в программе
  Serial.begin(9600);
  Serial1.begin(9600);
  // открываем Serial-соединение с GPRS Shield
  

  Serial.println("BalticSlobodaHomeOS");
  Serial.println("ver 2.24a");
  Serial.print("Loading");
  for (int lo = 0; lo < 5; lo++ ) {
    delay(350);
    Serial.print(".");
  }
  Serial.println("\r\n");
  Serial.println("Serial init OK");

  // проверяем есть ли связь с GPRS устройством
  while (!gprs.init()) {
    // если связи нет, ждём 1 секунду
    // и выводим сообщение об ошибке
    // процесс повторяется в цикле
    // пока не появится ответ от GPRS устройства
    delay(1000);
    Serial.print("Init error\r\n");
  }
  // выводим сообщение об удачной инициализации GPRS Shield
  Serial.println("GPRS init success");

  gprs.sendSMS("+79213310104", "SYSTEM: POWER IS ON!");
  delay(3000);
  gprs.sendSMS("+79062585121", "SYSTEM: POWER IS ON!");
  delay(1000);
  aauto(); //Автовосстановление состояния при перезагрузке
}
void On() {
  // Включить релюхи
  digitalWrite(REL1, LOW);
  digitalWrite(REL2, LOW);
  digitalWrite(REL3, LOW);
}
void Off() {
  // Выключить релюхи
  digitalWrite(REL1, HIGH);
  digitalWrite(REL2, HIGH);
  digitalWrite(REL3, HIGH);
}
void On10() {
  temp_get();
  if (Tempr1 < 10){
  On();
  }
  else
  Off();
}
void On20() {
  temp_get();
  if (Tempr1 < 20){
  On();
  }
  else
  Off();
}
void On25() {
  temp_get();
  if (Tempr1 < 25){
  On();
  }
  else
  Off();
}
void main_temp(){
  stat = eeprom_read_byte(0);
  if (stat == 2){
    On10();
  }
  if (stat == 3){
    On20();
  }
  if (stat == 4){
    On25();
  }
  delay(100);
  
}
void aauto() {
  //Автоматическое восстановление при перезагрузке
  stat = eeprom_read_byte(0);
  if (stat == 0) {
    Off();
    stateRelay = false;
  }
  if (stat == 1) {
    On();
    stateRelay = true;
    gprs.sendSMS("+79213310104", "SYSTEM: AUTO RESTART!");
    delay(3000);
    gprs.sendSMS("+79062585121", "SYSTEM: AUTO RESTART!");
  }
}
void temp_get() {
  Tempr1 = 0;
  for (int i = 0; i < 100; i++ )
  {
    int data = analogRead(temperaturePin); // получение данных с датчика
    delay(50);
    float voltage = data * VCC; // конвертация значения в напряжение
    voltage = voltage / 1024.0;

    delay(10); //задержка
    temp = (voltage - 0.5) * 100; // конвертация напряжения в температуру
    Tempr1 = Tempr1 + temp;

   
  }
}

void loop()
{
  // если пришло новое сообщение
  if (gprs.ifSMSNow())
  {
    // читаем его
    // если есть хотя бы одно непрочитанное сообщение,
    // читаем его
    gprs.readSMS(message, phone, datetime);

    // выводим номер, с которого пришло смс
    Serial.print(F("From number: "));
    Serial.println(phone);

    // выводим дату, когда пришло смс
    Serial.print(F("Datetime: "));
    Serial.println(datetime);

    // выводим текст сообщения
    Serial.print(F("Recieved Message: "));
    Serial.println(message);

    // вызываем функцию изменения состояния реле
    // в зависимости от текста сообщения
    setRelay(phone, message);
  }
}
void setRelay(char f_phone[], char f_message[])
{

  //Tempr1 = 0;
  if (strcmp(f_message, MESSAGE_ON) == 0) {
    // если сообщение — с текстом «On»,
    // выводим сообщение в Serial
    // и подаём на замыкаем реле
    Serial.println(F("OK! Power is On"));
    On();
    stateRelay = true;
    stat = 1 ;
    eeprom_write_byte(0, stat);
    // на номер, с которого пришёл запрос,
    // отправляем смс с текстом о включении питания
    gprs.sendSMS(f_phone, "OK! Power is On");

  } else if (strcmp(f_message, MESSAGE_OFF) == 0) {
    // если пришло сообщение с текстом «Off»,
    // выводим сообщение в Serial
    // и размыкаем реле
    Serial.println(F("OK! Power is Off"));
    Off();
    stateRelay = false;
    stat = 0;
    eeprom_write_byte(0, stat);
    // на номер, с которого пришёл запрос
    // отправляем смс с текстом о выключении питания
    gprs.sendSMS(f_phone, "OK! Power is Off.");
    
  } else if (strcmp(f_message, MESSAGE_ON10) == 0) {
    // если пришло сообщение с текстом «On10»,
    // выводим сообщение в Serial
    // и /.....
    Serial.println(F("OK! Thermostatic mode is on! Set temperature to 10. "));
    Serial.print(F("Temperature now is: "));
    Serial.print(temp);
    stateRelay = true;
    stat = 2;
    eeprom_write_byte(0, stat);
    // на номер, с которого пришёл запрос
    // отправляем смс с текстом о выключении питания
    gprs.sendSMS(f_phone, "OK! Thermostatic mode is on! Set temperature to 10. ");

    } else if (strcmp(f_message, MESSAGE_ON20) == 0) {
    // если пришло сообщение с текстом «On20»,
    // выводим сообщение в Serial
    // и /.....
    Serial.println(F("OK! Thermostatic mode is on! Set temperature to 20. "));
    Serial.print(F("Temperature now is: "));
    Serial.print(temp);
    stateRelay = true;
    stat = 3;
    eeprom_write_byte(0, stat);
    // на номер, с которого пришёл запрос
    // отправляем смс с текстом о выключении питания
    gprs.sendSMS(f_phone, "OK! Thermostatic mode is on! Set temperature to 20. ");

    } else if (strcmp(f_message, MESSAGE_ON25) == 0) {
    // если пришло сообщение с текстом «On25»,
    // выводим сообщение в Serial
    // и /.....
    Serial.println(F("OK! Thermostatic mode is on! Set temperature to 25. "));
    Serial.print(F("Temperature now is: "));
    Serial.print(temp);
    stateRelay = true;
    stat = 4;
    eeprom_write_byte(0, stat);
    // на номер, с которого пришёл запрос
    // отправляем смс с текстом о выключении питания
    gprs.sendSMS(f_phone, "OK! Thermostatic mode is on! Set temperature to 25. ");
    
  } else if (strcmp(f_message, MESSAGE_STATE) == 0) {
    // если пришло сообщение с текстом «State»,
    // отправляем сообщение с состоянием реле
      if (stateRelay) {
      Serial.println(F("State: Power is On"));
      gprs.sendSMS(f_phone, "STATE: Power is On");
      delay(1000);

    } else {
      Serial.println("State: Power is Off ");
      gprs.sendSMS(f_phone, "STATE: Power is Off");
      delay(1000);

    }
  }
  else if (strcmp(f_message, MESSAGE_TEMP) == 0) {
    // если пришло сообщение с текстом «Temp»,
    // измерение температуры
    temp_get();
    delay(10);
    Tempr2 = Tempr1 / 100; // среднее арифмитическое значение температуры
    //char Stroka = "No temperature";
    //Tempr2 = Tempr1;
    //int Tempr3 = int(Tempr1);
    //Tempr2 = char(Tempr3);// преобразование в символьный тип
    Serial.print(F("Temperature = ")); //  вывод показаний в монитор Serial-порта
    Serial.print(Tempr1);
    Serial.println(F(" °C"));
    Serial.println("    ----------    ");
    Serial.print(Tempr2);
    Serial.println(F(" °C"));
    
      if (Tempr2 == 0 )  {gprs.sendSMS(f_phone, "Temperature now is: 0 °C");}
      if (Tempr2 == 1 )  {gprs.sendSMS(f_phone, "Temperature now is: 1 °C");}
      if (Tempr2 == 2 )  {gprs.sendSMS(f_phone, "Temperature now is: 2 °C");}
      if (Tempr2 == 3 )  {gprs.sendSMS(f_phone, "Temperature now is: 3 °C");}
      if (Tempr2 == 4 )  {gprs.sendSMS(f_phone, "Temperature now is: 4 °C");}
      if (Tempr2 == 5 )  {gprs.sendSMS(f_phone, "Temperature now is: 5 °C");}
      if (Tempr2 == 6 )  {gprs.sendSMS(f_phone, "Temperature now is: 6 °C");}
      if (Tempr2 == 7 )  {gprs.sendSMS(f_phone, "Temperature now is: 7 °C");}
      if (Tempr2 == 8 )  {gprs.sendSMS(f_phone, "Temperature now is: 8 °C");}
      if (Tempr2 == 9 )  {gprs.sendSMS(f_phone, "Temperature now is: 9 °C");}
      if (Tempr2 == 10 )  {gprs.sendSMS(f_phone, "Temperature now is: 10 °C");}
      if (Tempr2 == 11 )  {gprs.sendSMS(f_phone, "Temperature now is: 11 °C");}
      if (Tempr2 == 12 )  {gprs.sendSMS(f_phone, "Temperature now is: 12 °C");}
      if (Tempr2 == 13 )  {gprs.sendSMS(f_phone, "Temperature now is: 13 °C");}
      if (Tempr2 == 14 )  {gprs.sendSMS(f_phone, "Temperature now is: 14 °C");}
      if (Tempr2 == 15 )  {gprs.sendSMS(f_phone, "Temperature now is: 15 °C");}
      if (Tempr2 == 16 )  {gprs.sendSMS(f_phone, "Temperature now is: 16 °C");}
      if (Tempr2 == 17 )  {gprs.sendSMS(f_phone, "Temperature now is: 17 °C");}
      if (Tempr2 == 18 )  {gprs.sendSMS(f_phone, "Temperature now is: 18 °C");}
      if (Tempr2 == 19 )  {gprs.sendSMS(f_phone, "Temperature now is: 19 °C");}
      if (Tempr2 == 20 )  {gprs.sendSMS(f_phone, "Temperature now is: 20 °C");}
      if (Tempr2 == 21 )  {gprs.sendSMS(f_phone, "Temperature now is: 21 °C");}
      if (Tempr2 == 22 )  {gprs.sendSMS(f_phone, "Temperature now is: 22 °C");}
      if (Tempr2 == 23 )  {gprs.sendSMS(f_phone, "Temperature now is: 23 °C");}
      if (Tempr2 == 24 )  {gprs.sendSMS(f_phone, "Temperature now is: 24 °C");}
      if (Tempr2 == 25 )  {gprs.sendSMS(f_phone, "Temperature now is: 25 °C");}
      if (Tempr2 == 26 )  {gprs.sendSMS(f_phone, "Temperature now is: 26 °C");}
      if (Tempr2 == 27 )  {gprs.sendSMS(f_phone, "Temperature now is: 27 °C");}
      if (Tempr2 == 28 )  {gprs.sendSMS(f_phone, "Temperature now is: 28 °C");}
      if (Tempr2 == 29 )  {gprs.sendSMS(f_phone, "Temperature now is: 29 °C");}
      if (Tempr2 == 30 )  {gprs.sendSMS(f_phone, "Temperature now is: 30 °C");}
      if (Tempr2 == 31 )  {gprs.sendSMS(f_phone, "Temperature now is: 31 °C");}
      if (Tempr2 == 32 )  {gprs.sendSMS(f_phone, "Temperature now is: 32 °C");}
      if (Tempr2 == 33 )  {gprs.sendSMS(f_phone, "Temperature now is: 33 °C");}
      if (Tempr2 == 34 )  {gprs.sendSMS(f_phone, "Temperature now is: 34 °C");}
      if (Tempr2 == 35 )  {gprs.sendSMS(f_phone, "Temperature now is: 35 °C");}
      if (Tempr2 == 36 )  {gprs.sendSMS(f_phone, "Temperature now is: 36 °C");}
      if (Tempr2 == 37 )  {gprs.sendSMS(f_phone, "Temperature now is: 37 °C");}
      if (Tempr2 == 38 )  {gprs.sendSMS(f_phone, "Temperature now is: 38 °C");}
      if (Tempr2 == 39 )  {gprs.sendSMS(f_phone, "Temperature now is: 39 °C");}
      if (Tempr2 == 40 )  {gprs.sendSMS(f_phone, "Temperature now is: 40 °C");}
      if (Tempr2 > 40 )  {gprs.sendSMS(f_phone, "!Warning! The temperature is more 40 degrees");}
      if (Tempr2 == -1 )  {gprs.sendSMS(f_phone, "Temperature now is: -1 °C");}
      if (Tempr2 == -2 )  {gprs.sendSMS(f_phone, "Temperature now is: -2 °C");}
      if (Tempr2 == -3 )  {gprs.sendSMS(f_phone, "Temperature now is: -3 °C");}
      if (Tempr2 == -4 )  {gprs.sendSMS(f_phone, "Temperature now is: -4 °C");}
      if (Tempr2 == -5 )  {gprs.sendSMS(f_phone, "Temperature now is: -5 °C");}
      if (Tempr2 < -5 )  {gprs.sendSMS(f_phone, "!Warning! The temperature is below -5 degrees");}
      delay(100);


  }

  else {
    // если сообщение содержит неизвестный текст,
    // отправляем сообщение с текстом об ошибке
    Serial.println(F("Error... unknown command!"));
    gprs.sendSMS(f_phone, "Error...unknown command!");
  }
}
