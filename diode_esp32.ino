/*
  DiodeLab ESP32-C3 SuperMini
  ─────────────────────────────────────────
*/

#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>

// ═══════════════════════════════════════════════
// FIXED URL: Added /rest/v1/diode_readings
const char* SUPABASE_URL = "https://oymahbdbphpmjhisadva.supabase.co/rest/v1/diode_readings";
const char* SUPABASE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im95bWFoYmRicGhwbWpoaXNhZHZhIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NzcxMjIzNTUsImV4cCI6MjA5MjY5ODM1NX0.LC2X7dvn4TtpF-VhCUZ6jmWh-779gTPbHqDSgNrdk2A";
// ═══════════════════════════════════════════════

#define POT_PIN  1       // GPIO 1 → middle leg of potentiometer
#define INTERVAL 500     // send every 500ms

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n=== DiodeLab Starting ===");

  WiFiManager wm;
  wm.setConfigPortalTimeout(180); 

  bool connected = wm.autoConnect("DiodeLab-Setup");

  if (!connected) {
    Serial.println("Failed to connect. Restarting...");
    delay(2000);
    ESP.restart();
  }

  Serial.println("WiFi connected!");
  Serial.println("IP: " + WiFi.localIP().toString());
  Serial.println("Sending data to Supabase...");
}

void loop() {
  int raw = analogRead(POT_PIN);                 
  float voltage = (raw / 4095.0) * 3.3;          

  Serial.printf("Raw: %4d  |  Voltage: %.3f V\n", raw, voltage);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // The URL now includes the table path
    http.begin(SUPABASE_URL);
    
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey",        SUPABASE_KEY);
    http.addHeader("Authorization", String("Bearer ") + SUPABASE_KEY);
    http.addHeader("Prefer",        "return=minimal");

    String body = "{\"node_a_volt\":" + String(voltage, 4) + "}";
    int code = http.POST(body);

    if (code == 201 || code == 200) {
      Serial.println("Supabase: OK");
    } else {
      Serial.printf("Supabase error: %d\n", code);
      // If you still get a 404, check that the table name is exactly "diode_readings"
    }

    http.end();
  } else {
    Serial.println("WiFi lost. Reconnecting...");
    WiFi.reconnect();
  }

  delay(INTERVAL);
}