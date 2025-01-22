#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// Wi-Fi bilgileri
const char* ssid = "Galaxy S20 FE8A23";
const char* password = "kmrl2227";

// Kamera yapılandırma
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// HTTP sunucusu
WebServer server(80);

// Kamera akış fonksiyonu
void handleStream() {
    WiFiClient client = server.client();
    String response = "HTTP/1.1 200 OK\r\n"
                      "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
    client.write(response.c_str(), response.length());

    while (true) {
        camera_fb_t *fb = esp_camera_fb_get(); // Kamera çerçevesini al
        if (!fb) {
            Serial.println("Kamera çerçevesi alınamadı.");
            break;
        }

        // JPEG format kontrolü
        if (fb->format == PIXFORMAT_JPEG) {
            String part = "--frame\r\n"
                          "Content-Type: image/jpeg\r\n"
                          "Content-Length: " + String(fb->len) + "\r\n\r\n";
            client.write(part.c_str(), part.length());
            client.write(fb->buf, fb->len);
            client.write("\r\n");
        }

        esp_camera_fb_return(fb); // Çerçeve belleğini serbest bırak

        // İstemcinin bağlantısını kontrol et
        if (!client.connected()) {
            break;
        }
    }
}

// Ana sayfa yönlendirme
void handleRoot() {
    String html = "<html><body>";
    html += "<h1>ESP32-CAM Canlı Görüntü</h1>";
    html += "<p><a href=\"/stream\">Canlı Görüntüyü İzle</a></p>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    // Kamera yapılandırması
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;

    // Kamera başlatma
    if (esp_camera_init(&config) != ESP_OK) {
        Serial.println("Kamera başlatılamadı.");
        return;
    }

    // Wi-Fi bağlantısı
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi bağlantısı tamamlandı.");
    Serial.print("ESP32-CAM URL: http://");
    Serial.println(WiFi.localIP());

    // Web server rotaları
    server.on("/", handleRoot);
    server.on("/stream", handleStream);
    server.begin();
    Serial.println("Web sunucusu başlatıldı.");
}

void loop() {
    server.handleClient();
}
