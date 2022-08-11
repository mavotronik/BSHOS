/*

  BalticSlobodaHomeOS
  ver 2.25

*/

#include <GPRS_Shield_Arduino.h> // библиотека для работы с GPRS устройством
#include <avr/eeprom.h> // билиотека для работы с EEPROM

GPRS gprs(Serial1); // создаём объект класса GPRS и передаём в него объект Serial1

#define MESSAGE_LENGTH 160 // длина сообщения
#define MESSAGE_ON  "On" // текст сообщения о включении розетки
#define MESSAGE_OFF  "Off" // текст сообщения о выключении розетки
#define MESSAGE_STATE  "State" // текст сообщения о состоянии розетки
#define MESSAGE_TEMP  "Temp"// текст о запросе температуры
#define KEEP_TEMP10  "Keep10"// Текст о поддержании температуры на уровне 10 градусов
#define KEEP_TEMP15  "Keep15"// Текст о поддержании температуры на уровне 15 градусов
#define KEEP_TEMP20  "Keep20"// Текст о поддержании температуры на уровне 20 градусов
#define KEEP_TEMP25  "Keep25"// Текст о поддержании температуры на уровне 25 градусов

#define temperaturePin 2  // определение пина подключения датчика температуры
#define VCC 5.0           // напряжениe питания датчика VCC (5 В или 3.3 В)

#define REL1 5
#define REL2 8
#define REL3 9

char message[MESSAGE_LENGTH]; // текст сообщения
char phone[16]; // номер, с которого пришло сообщение
char datetime[24]; // дата отправки сообщения
bool stateRelay = false;
char mode; // Режим
char temp_mode; // Режим температуры
float temp; // Температура с датчика
float Tempr1; 
char Tempr2; 


void setup()
{
  pinMode(REL1, OUTPUT);         //назначаем 5 цифр. пин как выход
  pinMode(REL2, OUTPUT);        //назначаем 8 цифр. пин как выход
  pinMode(REL3, OUTPUT);       //назначаем 9 цифр. пин как выход

  Off(); //Вырубить релюхи

  Serial.begin(9600); // открываем последовательный порт для мониторинга действий в программе
  
  gprs.powerOn();// включаем GPRS шилд

  // открываем последовательный порт для мониторинга действий в программе
  Serial.begin(9600);
  Serial1.begin(9600);
  // открываем Serial-соединение с GPRS Shield
  
  Serial.println("BalticSlobodaHomeOS");
  Serial.println("ver 2.25");
  Serial.print("Loading");
  for (int lo = 0; lo < 5; lo++ ) {
    delay(350);
    Serial.print(".");
  }
  Serial.println("\r\n");
  Serial.println("Serial init OK");

  // проверяем есть ли связь с GPRS устройством
  while (!gprs.init()) {
    // если связи нет, ждём 1,5 секунды
    // и выводим сообщение об ошибке
    // процесс повторяется в цикле
    // пока не появится ответ от GPRS устройства
    delay(1500);
    Serial.print("GPRS init error\r\n");
  }
  // выводим сообщение об удачной инициализации GPRS Shield
  Serial.println("GPRS init success");

  gprs.sendSMS("+7xxxxxx", "SYSTEM: POWER IS ON!");
  delay(3000);
  gprs.sendSMS("+7xxxxxx", "SYSTEM: POWER IS ON!");
  delay(1000);
  aauto(); //Автовосстановление состояния при перезагрузке
}
void On() {
  // Включить релюхи
  digitalWrite(REL1, LOW);
  digitalWrite(REL2, LOW);
  digitalWrite(REL3, LOW);
  stateRelay = true;
}
void Off() {
  // Выключить релюхи
  digitalWrite(REL1, HIGH);
  digitalWrite(REL2, HIGH);
  digitalWrite(REL3, HIGH);
  stateRelay = false;
}
void aauto() {
  //Автоматическое восстановление при перезагрузке
  mode = eeprom_read_byte(0);
  temp_mode = eeprom_read_byte(3);
  if (mode == 0) {
    Off();
  }

  if (mode == 1) {
    On();
    gprs.sendSMS("+7xxxxxx", "SYSTEM: AUTO RESTART! Current mode is always On");
    delay(3000);
    gprs.sendSMS("+7xxxxxx", "SYSTEM: AUTO RESTART! Current mode is always On");
  }
}

void temp_get() {
   
    int data = analogRead(temperaturePin); // получение данных с датчика
    delay(50);
    float voltage = data * VCC; // конвертация значения в напряжение
    voltage = voltage / 1024.0;
    delay(10); //задержка
    temp = (voltage - 0.5) * 100; // конвертация напряжения в температуру
    delay(10); // задержка
  }

void temp_keeper() {
  if (mode == 3)
    if (temp_mode == 1){
      temp_get();
        if (temp <= 10)
          On();
        else 
          Off();
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
    Serial.print("From number: ");
    Serial.println(phone);

    // выводим дату, когда пришло смс
    Serial.print("Datetime: ");
    Serial.println(datetime);

    // выводим текст сообщения
    Serial.print("Recieved Message: ");
    Serial.println(message);

    // вызываем функцию изменения состояния реле
    // в зависимости от текста сообщения
    setRelay(phone, message);
  }

  
}

void setRelay(char f_phone[], char f_message[])
{
  if (strcmp(f_message, MESSAGE_ON) == 0) { // если сообщение — с текстом «On», выводим сообщение в Serial и включаем реле
    
    Serial.println("Mode always On");
    On();
    mode = 1 ;
    temp_mode = 0;
    eeprom_write_byte(0, mode);
    eeprom_write_byte(3, temp_mode);
    gprs.sendSMS(f_phone, "Mode set to always On"); // на номер, с которого пришёл запрос, отправляем смс с текстом о включении питания

} else if (strcmp(f_message, MESSAGE_OFF) == 0) { // если пришло сообщение с текстом «Off», выводим сообщение в Serial и вsключаем реле
    
    Serial.println("Mode always Off");
    Off();
    mode = 0;
    temp_mode = 0;
    eeprom_write_byte(0, mode);
    eeprom_write_byte(3, temp_mode);
    gprs.sendSMS(f_phone, "Mode set to always Off"); //на номер, с которого пришёл запрос, отправляем смс с текстом о выключении питания

} else if (strcmp(f_message, MESSAGE_STATE) == 0) {
    // если пришло сообщение с текстом «State»,
    // отправляем сообщение с состоянием реле
 
    if (stateRelay) {
      Serial.println("Relay on");
      gprs.sendSMS(f_phone, "STATE: Relay is On now");
      delay(1000);

    } else {
      Serial.println("Relay off");
      gprs.sendSMS(f_phone, "STATE: Relay is Off now");
      delay(1000);

    }

} else if (strcmp(f_message, KEEP_TEMP10) == 0) {
     mode = 3;
     temp_mode = 1;
     eeprom_write_byte(0, mode);
     eeprom_write_byte(3, temp_mode);
     Serial.println("Mode keep 10 deg");
     gprs.sendSMS(f_phone, "Mode set to keep 10 deg");

} else if (strcmp(f_message, KEEP_TEMP15) == 0) {
     mode = 3;
     temp_mode = 2;
     eeprom_write_byte(0, mode);
     eeprom_write_byte(3, temp_mode);
     Serial.println("Mode keep 15 deg");
     gprs.sendSMS(f_phone, "Mode set to keep 15 deg");

} else if (strcmp(f_message, KEEP_TEMP20) == 0) {
     mode = 3;
     temp_mode = 3;
     eeprom_write_byte(0, mode);
     eeprom_write_byte(3, temp_mode);
     Serial.println("Mode keep 20 deg");
     gprs.sendSMS(f_phone, "Mode set to keep 20 deg");

} else if (strcmp(f_message, KEEP_TEMP25) == 0) {
     mode = 3;
     temp_mode = 4;
     eeprom_write_byte(0, mode);
     eeprom_write_byte(3, temp_mode);
     Serial.println("Mode keep 25 deg");
     gprs.sendSMS(f_phone, "Mode set to keep 25 deg");
}

}
