#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

//#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define DHTTYPE DHT11 // DHT 11

# define DHTPIN_alt 2 // Digital pin connected to the DHT sensor
# define DHTPIN_ust 3 // Digital pin connected to the DHT sensor
# define btnRIGHT   0
# define btnUP      1
# define btnDOWN    2
# define btnLEFT    3
# define btnSELECT  4
# define btnNONE    5

#define RELAY1  A3
#define RELAY2  A4
#define RELAY3  A5

DHT dht_alt(DHTPIN_alt, DHTTYPE);
DHT dht_ust(DHTPIN_ust, DHTTYPE);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // select the pins used on the LCD panel

int adc_key_in;
int lcd_key  = 0;
int row = 0   ;
int col = 0   ;
int sec = 0   ;
unsigned long mil;
int dak = 0   ;
int saat = 0  ;
int gun = 0   ;
float alt_nem ;
float ust_nem ;
float alt_isi ;
float ust_isi ;
float isi_gel = 37.8 ;
float nem_gel = 50   ;
float isi_cik = 37.4 ;
float nem_cik = 60   ;
float hed_isi = 37.8 ;
float hed_nem = 50 ;
int   gel_gun = 18   ;

int isActive = 0; //aktif kuluçka var mı ? default 0 olacak
int flag = 0;
int t1 = 0;
int t2 = 1;


byte up[] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};
byte down[] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100
};

void setup() {
  // put your setup code here, to run once:
  load_Eprom();
  dht_alt.begin();
  dht_ust.begin();
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.createChar(0, up);
  lcd.createChar(1, down);
  // pinMode(RELAY1, OUTPUT);
  //  pinMode(RELAY2, OUTPUT);
  //  pinMode(RELAY3, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (isActive == 1) {
    get_temperature();
    get_time();
    button_works();
    menu();
    doWork();
    circulation(1);
  }
  else {
    circulation(0);
    nem(0);
    isit(0);
    initiate();
  }
}

void get_temperature() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  delay(250);
  alt_nem = dht_alt.readHumidity();
  ust_nem = dht_ust.readHumidity();
  alt_isi = dht_alt.readTemperature();
  ust_isi = dht_ust.readTemperature();
}

void show_temp() {
  lcd.setCursor(0, 0);
  lcd.print("Ust");
  lcd.print(ust_nem, 0);
  lcd.print("% ");
  lcd.print(ust_isi, 1);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Alt");
  lcd.print(alt_nem, 0);
  lcd.print("% ");
  lcd.print(alt_isi, 1);
  lcd.print("C");
  Serial.println("show_temp");
}

void menu() {
  if (isActive == 1 ) {
    lcd.setCursor(0, 0);
    lcd.clear();
    if (row == 0 ) {
      if ( col == 0) {
        show_temp(); //ısı nem göster
      }
      else if (col == 1 ) {
        show_time(); // saati göster
      }
    }
    else if ( row == 1 ) {// ısı ayarla 1 sağa basınca gelir
      if ( col == 0) {
        set_temp();
      }
    }
    else if (row == 2 ) {//nem ayarla 2 sağ
      if (col == 0 ) {
        set_hum();
      }
    }
  }
}

void set_temp() {
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("Sicaklik Ayar");
  lcd.setCursor(0, 1);
  lcd.print(hed_isi);
  EEPROM.put(5, hed_isi);
}

void set_hum() {
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("Nem Ayar");
  lcd.setCursor(0, 1);
  lcd.print(hed_nem);
  EEPROM.put(10, hed_nem);
}

void get_time() {
  if ((millis() - mil) >= 1000) {
    sec = sec + 1;
    mil = millis();
  }
  if (sec == 60 ) {
    sec = 0;
    dak = dak + 1;
    EEPROM.write(15, dak);
  }
  if (dak == 60) {
    dak = 0;
    saat = saat + 1;
    EEPROM.write(20, saat);
  }
  if (saat == 24) {
    saat = 0;
    gun = gun + 1;
    EEPROM.write(25, gun);
  }
}

void show_time() {
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("Gun: ");
  lcd.print(gun);
  lcd.setCursor(0, 1);
  if (saat < 10) {
    lcd.print("0");
    lcd.print(saat);
  } else {
    lcd.print(saat);
  }
  lcd.print(":");
  if (dak < 10) {
    lcd.print("0");
    lcd.print(dak);
  } else {
    lcd.print(dak);
  }
  lcd.print(":");
  if (sec < 10) {
    lcd.print("0");
    lcd.print(sec);
  } else {
    lcd.print(sec);
  }
}
void button_works() {
  lcd_key = read_LCD_buttons();  // read the buttons

  if (lcd_key == 0) { //sağdaki butona basıldı menü bi yana geçti
    ++row ; // sıfırsa daha eksiye gitmesin kontrolü ekle mutlaka overflow yersin
    if ( row > 2 ) {
      row = 0 ;
    }
  }
  else if ( lcd_key == 3 ) { //soldaki buton bi geri menü.
    if ( row > 0) {
      --row;
    }
  }
  else if ( lcd_key == 2 ) {//aşağı buton
    if (row == 1) {
      hed_isi = hed_isi - 0.5;
      void set_temp() ;
      return;
    }
    else if ( row == 2 ) {
      hed_nem = hed_nem - 0.5;
      void set_hum() ;
      return;
    }
    ++col;
    if ( col > 1 ) {
      col = 0;
    }
  }
  else if ( lcd_key == 1 ) { //yukarı buton
    if (row == 1) {
      hed_isi = hed_isi + 0.5;
      void set_temp() ;
      return;
    }
    else if ( row == 2 ) {
      hed_nem = hed_nem + 0.5;
      void set_hum() ;
      return;
    }
    if ( col > 0) {
      --col;
    }
  }
  else if ( lcd_key == 4) {
    ++t1;
    Serial.println(t1 - t2);
    if ( (t1 - t2) > 4) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Bitir?");
      delay(500);
      if ( (t1 - t2) > 5) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sonlandirildi");
        delay(2000);
        lcd.clear();
        clearVars();
      }
    }
  }
}

void initiate() {
  if (isActive == 0 ) {
    lcd.setCursor(0, 0);
    lcd.print("Basla");
    lcd.setCursor(0, 1);
    lcd.print("Tavuk");
    lcd_key = read_LCD_buttons();
    if (lcd_key == 4) {
      lcd.clear();
      lcd.print("Basliyor..");
      delay(500);
      clearVars();
      isActive = 1;
      EEPROM.write(1, 1);
      loadTavuk();
    }
    //Serial.println(lcd_key);
  }
}

// read the buttons
int read_LCD_buttons() {
  adc_key_in = analogRead(0); // read the value from the sensor
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 250)  return btnUP;
  if (adc_key_in < 410)  return btnDOWN;
  if (adc_key_in < 620)  return btnLEFT;
  if (adc_key_in < 780)  return btnSELECT;

  // For V1.0 comment the other threshold and use the one below:

  //  if (adc_key_in < 50)   return btnRIGHT;
  //  if (adc_key_in < 195)  return btnUP;
  //  if (adc_key_in < 380)  return btnDOWN;
  //  if (adc_key_in < 555)  return btnLEFT;
  //  if (adc_key_in < 790)  return btnSELECT;

  Serial.println(adc_key_in);
  // return btnNONE; // when all others fail, return this...

}
void load_Eprom() {
  isActive = EEPROM.read(1);
  dak      = EEPROM.read(15);
  saat     = EEPROM.read(20);
  gun      = EEPROM.read(25);
  EEPROM.get(5, hed_isi);
  EEPROM.get(10, hed_nem);
}

void doWork() {
//toleransı 0.5 ten 1 e çıkardım.
  if ( gun < gel_gun) {
    //**************************ısıtma*************
    if (((alt_isi + ust_isi) / 2) - hed_isi < 0.4 ) { //okunan değer hedefden çok aşağıda ısıt
      lcd.setCursor(14, 0);
      lcd.print("C");
      lcd.write(byte(0));
      Serial.println((alt_isi + ust_isi) / 2);
      Serial.println("ısıt");
      Serial.println(((alt_isi + ust_isi) / 2) - hed_isi);
      isit(1);
    }
    else if (((alt_isi + ust_isi) / 2) - hed_isi > 0.4) {//okunan ısı hedeften yüksek soğut
      lcd.setCursor(14, 0);
      lcd.print("C");
      lcd.write(byte(1));
      Serial.println((alt_isi + ust_isi) / 2);
      Serial.println("soğut");
      isit(0);
    }
    else {
      // lcd.clear();
      Serial.println("bişey yapma dur ısıtma");
    }
    //**************************ısıtma*************
    //**************************nem*************
    if (((alt_nem + ust_nem) / 2) - hed_nem < 1.5 ) {//okunan nem hedeften yüksek hava al
      lcd.setCursor(14, 1);
      lcd.print("N");
      lcd.write(byte(0));
      Serial.println((alt_nem + ust_nem) / 2);
      Serial.println("nemle");
      Serial.println(((alt_nem + ust_nem) / 2) - hed_nem);
      nem(0);
    }
    else if (((alt_nem + ust_nem) / 2) - hed_nem > 1.5 ) {//okunan nem hedeften düşük havayı kapat
      lcd.setCursor(14, 1);
      lcd.print("N");
      lcd.write(byte(1));
      Serial.println((alt_nem + ust_nem) / 2);
      Serial.println("anti nemle");
      Serial.println(((alt_nem + ust_nem) / 2) - hed_nem);
      nem(1);
    }
    else {
      //lcd.clear();
      Serial.println("bişey yapma dur nemleme");
    }
    //**************************nem*************
  }
  //  else if ( gun > gel_gun) {
  //    if (((alt_isi + ust_isi) / 2) - hed_isi < 0.5 ) { //okunan değer hedefden çok aşağıda ısıt
  //    }
  //    else if (((alt_isi + ust_isi) / 2) - hed_isi > 0.5 ) {//okunan ısı hedeften yüksek soğut
  //    }
  //  }
  else if (gun == gel_gun && saat == 1) { // çıkım zamanı hedef ısı ve nemi değiştir. Alarm olabilir bu aşamada.
    hed_isi = isi_cik;
    hed_nem = nem_cik;
    lcd.print("cikim zamani geldi");
  }
}
void loadTavuk() {

  isi_gel = 37.8 ;
  nem_gel = 50   ;
  isi_cik = 37.4 ;
  nem_cik = 60   ;
  gel_gun = 18   ;

  hed_isi = isi_gel;
  hed_nem = nem_gel;

}

void clearVars() {

  isActive = 0;
  dak = 0;
  sec = 0;
  saat = 0;
  gun = 0;
  hed_isi = 0;
  hed_nem = 0;
  for (int i = 0; i < 50; i++) {
    EEPROM.write(i, 0);
  }
  lcd.clear();
}

void isit(int i) {
  //i 1 ise aç sıfır ise kapat
  pinMode(RELAY1, OUTPUT);
  if (i == 1) {
    digitalWrite(RELAY1, LOW);          // Turns ON Relays 1
  }
  else if (i == 0 ) {
   digitalWrite(RELAY1, HIGH);          // Turns ON Relays 1
  }
}
void nem(int i) {
  pinMode(RELAY2, OUTPUT);
  if (i == 1) {
    digitalWrite(RELAY2, LOW);          // Turns ON Relays 1
  }
  else if (i == 0 ) {
    digitalWrite(RELAY2, HIGH);          // Turns ON Relays 1
  }
}

void circulation(int i) {
  pinMode(RELAY3, OUTPUT);
  if (i == 1) {
    digitalWrite(RELAY3, LOW);          // Turns ON Relays 1
  }
  else if (i == 0 ) {
    digitalWrite(RELAY3, HIGH);          // Turns ON Relays 1
  }
}
/*EPROM
   1 - isActive
   5 - hedef ısı
   10- hedef nem
   15- dakika
   20- saat
   25- Gün

 * */
