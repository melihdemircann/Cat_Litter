#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

// WiFi ağ bilgileri
const char *ssid = "BUDA_2.4GHz";               // Kablosuz ağınızın SSID'si
const char *password = "BUDA2021";             // Kablosuz ağınızın şifresi

// Firebase bilgileri
#define FIREBASE_HOST "koku-ace82-default-rtdb.firebaseio.com"  // Firebase Realtime Database URL
#define FIREBASE_AUTH "qADDd6G9WesV3sEHxS01pYz41VnwzrMqghHQRwuC" // Firebase Secret Key

// Firebase sunucusunun SHA-1 fingerprint'i
const char *FIREBASE_FINGERPRINT = "6D:29:9B:3F:09:A5:1E:7C:70:08:E0:00:0F:74:D6:4E:39:12:2E:A7"; // Firebase güvenli bağlantı için kullanılan parmak izi

// MQ-135 sensör pini
const int mq135Pin = A0;  // MQ-135 sensörünün bağlandığı analog pin

void setup() {
  Serial.begin(9600);  // Seri haberleşme başlatılır
  Serial.println("\nWiFi'ye bağlanılıyor..."); // WiFi'ye bağlanma durumu

  // WiFi bağlantısı başlatılır
  WiFi.mode(WIFI_STA);  // WiFi'yi istasyon (STA) moduna alır
  WiFi.begin(ssid, password); // WiFi'ye bağlanmaya çalışır

  // Bağlantı sağlanana kadar bekler
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);  // 500 ms bekle
    Serial.print(".");  // Bağlantı süreci hakkında bilgi verir
  }

  // Bağlantı sağlandığında bilgiler seri monitöre yazdırılır
  Serial.println("\nWiFi bağlantısı başarılı!");
  Serial.print("IP Adresi: ");
  Serial.println(WiFi.localIP());  // Bağlantı sağlandığında IP adresini yazdırır

  // Firebase sunucusuna fingerprint doğrulaması yapılır
  if (!checkFingerprint()) {  // Eğer fingerprint doğrulaması başarısız olursa
    Serial.println("Sunucu fingerprint doğrulaması başarısız! Bağlantı kesildi.");
    while (true);  // Program durur
  }
  Serial.println("Fingerprint doğrulandı. Bağlantı güvenli.");  // Başarılı bağlantı durumunda bilgi verir

  // Firebase başlatılır
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);  // Firebase'e bağlanmak için gerekli parametrelerle başlatılır
  Serial.println("Firebase başlatıldı.");  // Firebase'in başarıyla başlatıldığını belirtir
}

void loop() {
  // MQ-135 sensöründen veri okuma
  int sensorValue = analogRead(mq135Pin);  // MQ-135 sensöründen analog değer okunur
  float voltage = sensorValue * (3.3 / 1024.0); // Analog değeri gerilim değerine dönüştürür
  float ammoniaPPM = map(sensorValue, 0, 1023, 0, 1000); // Sensör değeri ppm (parts per million) birimine ölçeklendirilir

  // Seri monitöre sensör verilerini yazdırır
  Serial.print("Sensör Değeri: ");
  Serial.print(sensorValue);
  Serial.print(" | Gerilim: ");
  Serial.print(voltage);
  Serial.print(" V | Amonyak (PPM): ");
  Serial.println(ammoniaPPM);

  // Firebase'e amonyak verisini yazma
  Serial.println("Firebase'e amonyak verisi yazılıyor...");  // Firebase'e veri gönderme işlemi başlatılır
  Firebase.setFloat("MQ135/Amonyak", ammoniaPPM);  // Firebase'e amonyak değeri gönderilir
  if (Firebase.failed()) {  // Eğer Firebase işlemi başarısız olursa
    Serial.print("Amonyak Yazma Hatası: ");
    Serial.println(Firebase.error());  // Hata mesajı yazdırılır
    delay(2000);  // 2 saniye beklenir
    return;  // Fonksiyondan çıkılır
  }
  Serial.println("Amonyak verisi başarıyla yazıldı!");  // Verinin Firebase'e başarıyla yazıldığını belirten mesaj

  delay(2000); // 2 saniye bekle, sensör okumalarını yapmadan önce veri yazılacak
}

// Fingerprint doğrulama fonksiyonu
bool checkFingerprint() {
  WiFiClientSecure client;  // Güvenli bağlantı için WiFiClientSecure nesnesi oluşturulur
  client.setTimeout(5000);  // Timeout süresi 5 saniye olarak ayarlanır
  if (!client.connect(FIREBASE_HOST, 443)) {  // Sunucuya bağlantı kurulamazsa
    Serial.println("Sunucuya bağlanılamadı!");  // Hata mesajı yazdırılır
    return false;  // False döndürülür
  }

  // Sunucu fingerprint doğrulaması yapılır
  if (client.verify(FIREBASE_FINGERPRINT, FIREBASE_HOST)) {  // Fingerprint doğrulaması başarılıysa
    return true;  // True döndürülür
  } else {
    Serial.println("Fingerprint doğrulaması başarısız!");  // Fingerprint doğrulama başarısızsa mesaj yazdırılır
    return false;  // False döndürülür
  }
}
