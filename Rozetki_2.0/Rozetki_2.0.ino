// библиотека для работы с GPRS устройством
#include <GPRS_Shield_Arduino.h>
 
// библиотека для эмуляции Serial порта
// она нужна для работы библиотеки GPRS_Shield_Arduino
#include <SoftwareSerial.h>
 
// создаём объект mySerial и передаём номера управляющих пинов RX и TX
SoftwareSerial mySerial(10, 11);
 
// создаём объект класса GPRS и передаём в него объект mySerial 
GPRS gprs(mySerial);
// можно указать дополнительные параметры — пины PK и ST
// по умолчанию: PK = 2, ST = 3
// GPRS gprs(mySerial, 2, 3);

// длина сообщения
#define MESSAGE_LENGTH 160
// текст сообщения о включении розетки
#define MESSAGE_ON  "On"
// текст сообщения о выключении розетки
#define MESSAGE_OFF  "Off"
// текст сообщения о состоянии розетки
#define MESSAGE_STATE  "State"
#define MESSAGE_TEMP  "Temp"
// текст о запросе температуры

// температурный датчик
#define temperaturePin 2  // определение пина подключения модуля датчика температуры
#define VCC 5.0           // определение напряжения питания датчика VCC (5 В или 3.3 В)

// текст сообщения
char message[MESSAGE_LENGTH];
// номер, с которого пришло сообщение
char phone[16];
// дата отправки сообщения
char datetime[24];

 bool stateRelay = false;

void setup()
{
 pinMode(5,OUTPUT);          //назначаем 5 цифр. пин как выход
 pinMode(8,OUTPUT);         //назначаем 8 цифр. пин как выход
 pinMode(9,OUTPUT);        //назначаем 9 цифр. пин как выход
 

  // подаём на пин реле «низкий уровень» (размыкаем реле)
 // digitalWrite(5, LOW);
 // digitalWrite(8, LOW);
 // digitalWrite(9, LOW);

  digitalWrite(5, HIGH);
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);
  // открываем последовательный порт для мониторинга действий в программе
  Serial.begin(9600);
  // включаем GPRS шилд
  gprs.powerOn();
  // ждём, пока не откроется монитор последовательного порта
  // для того, чтобы отследить все события в программе
  while (!Serial) {

  }
  // открываем последовательный порт для мониторинга действий в программе
  Serial.begin(9600);
  // открываем Serial-соединение с GPRS Shield
  mySerial.begin(9600);
  // ждём, пока не откроется монитор последовательного порта
  // для того, чтобы отследить все события в программе
  while (!Serial) {
  }
  Serial.print("Serial init OK\r\n");
  
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

  gprs.sendSMS(" +7xxxxxxxxxx", "SYSTEM POWER IS ON!");
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
  float Tempr1;
  char Tempr2;
  
  Tempr1 = 0;
  if (strcmp(f_message, MESSAGE_ON) == 0) {
    // если сообщение — с текстом «On»,
    // выводим сообщение в Serial
    // и подаём на замыкаем реле
    Serial.println("OK! Power is On");  
    //digitalWrite(5, HIGH);
    //delay(3000);
    //digitalWrite(8, HIGH);
    //delay(3000);
    //digitalWrite(9, HIGH);

    digitalWrite(5, LOW);
    delay(3000);
    digitalWrite(8, LOW);
    delay(3000);
    digitalWrite(9, LOW);
    stateRelay = true;
    // на номер, с которого пришёл запрос,
    // отправляем смс с текстом о включении питания
    gprs.sendSMS(f_phone, "OK! Power is On");
  } else if (strcmp(f_message, MESSAGE_OFF) == 0) {
    // если пришло сообщение с текстом «Off»,
    // выводим сообщение в Serial
    // и размыкаем реле
    Serial.println("OK! Power is Off");
    //digitalWrite(5, LOW);  Щилд инвертирует сигнал с high на low и наоборот
    //delay(3000);
    //digitalWrite(8, LOW);
    //delay(3000);
   // digitalWrite(9, LOW);

    digitalWrite(5, HIGH);
    delay(3000);
    digitalWrite(8, HIGH);
    delay(3000);
    digitalWrite(9, HIGH);
    
    stateRelay = false;
    // на номер, с которого пришёл запрос
    // отправляем смс с текстом о выключении питания
    gprs.sendSMS(f_phone, "OK! Power is Off");
  } else if (strcmp(f_message, MESSAGE_STATE) == 0) {
    // если пришло сообщение с текстом «State»,
    // отправляем сообщение с состоянием реле
    // измерение температуры

          
   
    if (stateRelay) {
      Serial.println("State: Power is On");
      gprs.sendSMS(f_phone, "Power is On");
      delay(3000);
     
       //gprs.sendSMS(f_phone, "Power is On");
       //delay(3000);
       // gprs.sendSMS(f_phone, "Power is On");
        //delay(3000);
    } else {
      Serial.println("State: Power is Off ");
      gprs.sendSMS(f_phone, "Power is Off");
      delay(3000);
     
    }
  } 
  else if (strcmp(f_message, MESSAGE_TEMP) == 0) {
    // если пришло сообщение с текстом «Temp»,
    // измерение температуры
  Tempr1 = 0;
   for(int i = 0; i<200; i++ )
   {
        int data = analogRead(temperaturePin); // получение данных с датчика 
        delay(50);
        float voltage = data * VCC; // конвертация значения в напряжение
        voltage = voltage / 1024.0; 

        delay(10); //задержка
        float temp = (voltage - 0.5) * 100; // конвертация напряжения в температуру
        Tempr1 = Tempr1 + temp;
        delay(10); // задержка 
   }
   Tempr1 = Tempr1/200;// среднее арифмитическое значение температуры
   char Stroka = "No temperatere";
   //Tempr2 = Tempr1;
   int Tempr3 = int(Tempr1);
   Tempr2 = char(Tempr3);// преобразование в символьный тип
        Serial.print("Temperature = "); //  вывод показаний в монитор Serial-порта
        Serial.print(Tempr1);
        Serial.println(" °C");
        Serial.println("    ----------    ");
        Serial.print(Tempr3);
        Serial.println(" °C");
        
          if (Tempr3 == 0 )  {gprs.sendSMS(f_phone, "0 °C");}
          if (Tempr3 == 1 )  {gprs.sendSMS(f_phone, "1 °C");}
          if (Tempr3 == 2 )  {gprs.sendSMS(f_phone, "2 °C");}
          if (Tempr3 == 3 )  {gprs.sendSMS(f_phone, "3 °C");}
          if (Tempr3 == 4 )  {gprs.sendSMS(f_phone, "4 °C");}
          if (Tempr3 == 5 )  {gprs.sendSMS(f_phone, "5 °C");}
          if (Tempr3 == 6 )  {gprs.sendSMS(f_phone, "6 °C");}
          if (Tempr3 == 7 )  {gprs.sendSMS(f_phone, "7 °C");}
          if (Tempr3 == 8 )  {gprs.sendSMS(f_phone, "8 °C");}
          if (Tempr3 == 9 )  {gprs.sendSMS(f_phone, "9 °C");}
          if (Tempr3 == 10 )  {gprs.sendSMS(f_phone, "10 °C");}
          if (Tempr3 == 11 )  {gprs.sendSMS(f_phone, "11 °C");}
          if (Tempr3 == 12 )  {gprs.sendSMS(f_phone, "12 °C");}
          if (Tempr3 == 13 )  {gprs.sendSMS(f_phone, "13 °C");}
          if (Tempr3 == 14 )  {gprs.sendSMS(f_phone, "14 °C");}
          if (Tempr3 == 15 )  {gprs.sendSMS(f_phone, "15 °C");}
          if (Tempr3 == 16 )  {gprs.sendSMS(f_phone, "16 °C");}
          if (Tempr3 == 17 )  {gprs.sendSMS(f_phone, "17 °C");}
          if (Tempr3 == 18 )  {gprs.sendSMS(f_phone, "18 °C");}
          if (Tempr3 == 19 )  {gprs.sendSMS(f_phone, "19 °C");}
          if (Tempr3 == 20 )  {gprs.sendSMS(f_phone, "20 °C");}
          if (Tempr3 == 21 )  {gprs.sendSMS(f_phone, "21 °C");}
          if (Tempr3 == 22 )  {gprs.sendSMS(f_phone, "22 °C");}
          if (Tempr3 == 23 )  {gprs.sendSMS(f_phone, "23 °C");}
          if (Tempr3 == 24 )  {gprs.sendSMS(f_phone, "24 °C");}
          if (Tempr3 == 25 )  {gprs.sendSMS(f_phone, "25 °C");}
          if (Tempr3 == 26 )  {gprs.sendSMS(f_phone, "26 °C");}
          if (Tempr3 == 27 )  {gprs.sendSMS(f_phone, "27 °C");}
          if (Tempr3 == 28 )  {gprs.sendSMS(f_phone, "28 °C");}
          if (Tempr3 == 29 )  {gprs.sendSMS(f_phone, "29 °C");}
          if (Tempr3 == 30 )  {gprs.sendSMS(f_phone, "30 °C");}
          if (Tempr3 == 31 )  {gprs.sendSMS(f_phone, "31 °C");}
          if (Tempr3 == 32 )  {gprs.sendSMS(f_phone, "32 °C");}
          if (Tempr3 == 33 )  {gprs.sendSMS(f_phone, "33 °C");}
          if (Tempr3 == 34 )  {gprs.sendSMS(f_phone, "34 °C");}
          if (Tempr3 == 35 )  {gprs.sendSMS(f_phone, "35 °C");}
          if (Tempr3 == 36 )  {gprs.sendSMS(f_phone, "36 °C");}
          if (Tempr3 == 37 )  {gprs.sendSMS(f_phone, "37 °C");}
          if (Tempr3 == 38 )  {gprs.sendSMS(f_phone, "38 °C");}
          if (Tempr3 == 39 )  {gprs.sendSMS(f_phone, "39 °C");}
          if (Tempr3 == 40 )  {gprs.sendSMS(f_phone, "40 °C");}
          if (Tempr3 > 40 )  {gprs.sendSMS(f_phone, "!Warning! The temperature is more 40 degrees");}
          if (Tempr3 == -1 )  {gprs.sendSMS(f_phone, "-1 °C");}
          if (Tempr3 == -2 )  {gprs.sendSMS(f_phone, "-2 °C");}
          if (Tempr3 == -3 )  {gprs.sendSMS(f_phone, "-3 °C");}
          if (Tempr3 == -4 )  {gprs.sendSMS(f_phone, "-4 °C");}
          if (Tempr3 == -5 )  {gprs.sendSMS(f_phone, "-5 °C");}
          if (Tempr3 < -5 )  {gprs.sendSMS(f_phone, "!Warning! The temperature is below -5 degrees");}
          delay(3000);

        Tempr1 = 0;
  } 
  else {
    // если сообщение содержит неизвестный текст,
    // отправляем сообщение с текстом об ошибке
    Serial.println("Error... unknown command!");
    gprs.sendSMS(f_phone, "Error...unknown command!");
    }
  }





  


