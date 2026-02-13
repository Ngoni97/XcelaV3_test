#include <Arduino.h>
#include <EEPROM.h>
#include <SPIFFS.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// GPS Module connection pins
#define GPS_RX_PIN 18  // Connect to GPS TX
#define GPS_TX_PIN 17  // Connect to GPS RX

// Buzzer and Relay pins
#define BUZZER 11
#define RELAY 12

// GPS Baud Rate (NEO-6M default is 9600)
#define GPS_BAUD 9600

// Create TinyGPSPlus object
TinyGPSPlus gps;

// Create hardware serial object for GPS
HardwareSerial gpsSerial(1); // Use UART1

// EEPROM size in bytes (max 512 bytes recommended for ESP32)
#define EEPROM_SIZE 512

// Test file paths for SPIFFS
#define TEST_FILE "/test.txt"
#define TEST_FILE_2 "/data.bin"

// Function prototypes
void testEEPROM();
void testSPIFFS();
void printSPIFFSInfo();
void writeTestFile();
void readTestFile();
void listSPIFFSFiles();
void deleteSPIFFSFile(const char* path);

void setup() {
  Serial.begin(115200);
  delay(2000); // Wait for serial monitor

  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_TX_PIN, GPS_RX_PIN);

  
  Serial.println("\n\n========================================");
  Serial.println("ESP32-S3 GPS and SPIFFS and EEPROM Test");
  Serial.println("========================================\n");

  pinMode(BUZZER, OUTPUT);
  pinMode(RELAY, OUTPUT);

  digitalWrite(BUZZER, HIGH);
  delay(1000);
  digitalWrite(BUZZER, LOW);
  delay(1000);
  digitalWrite(RELAY, HIGH);
  delay(1000);
  digitalWrite(RELAY, LOW);

  // Test EEPROM
  testEEPROM();
  
  delay(1000);
  
  // Test SPIFFS
  testSPIFFS();
  
  Serial.println("\n========================================");
  Serial.println("All tests completed!");
  Serial.println("========================================\n");
}

void loop() {

  // ====== READ GPS ======
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // ====== PRINT ALL GPS DATA WHEN UPDATED ======
  if (gps.location.isUpdated()) {

    Serial.println("========= GPS DATA =========");

    // LOCATION
    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);

    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);

    Serial.print("Location Age (ms): ");
    Serial.println(gps.location.age());

    // ALTITUDE
    Serial.print("Altitude: ");
    Serial.print(gps.altitude.meters());
    Serial.println(" m");

    // SPEED
    Serial.print("Speed: ");
    Serial.print(gps.speed.kmph());
    Serial.println(" km/h");

    // COURSE / DIRECTION
    Serial.print("Course: ");
    Serial.print(gps.course.deg());
    Serial.println(" degrees");

    // DATE
    Serial.print("Date: ");
    Serial.print(gps.date.day());
    Serial.print("/");
    Serial.print(gps.date.month());
    Serial.print("/");
    Serial.println(gps.date.year());

    // TIME UTC
    Serial.print("Time (UTC): ");
    Serial.print(gps.time.hour());
    Serial.print(":");
    Serial.print(gps.time.minute());
    Serial.print(":");
    Serial.println(gps.time.second());

    // SATELLITES
    Serial.print("Satellites: ");
    Serial.println(gps.satellites.value());

    // HDOP
    Serial.print("HDOP (accuracy): ");
    Serial.println(gps.hdop.hdop());

    Serial.println("-----------------------------");
  }

}

// ==================== EEPROM TESTS ====================

void testEEPROM() {
  Serial.println("\n--- EEPROM Test Start ---");
  
  // Initialize EEPROM with predefined size
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("Failed to initialize EEPROM!");
    return;
  }
  Serial.println("✓ EEPROM initialized successfully");
  Serial.printf("✓ EEPROM size: %d bytes\n", EEPROM_SIZE);
  
  // Test 1: Write and read single byte
  Serial.println("\nTest 1: Write/Read single byte");
  byte testByte = 42;
  EEPROM.write(0, testByte);
  EEPROM.commit(); // Important: commit changes to flash
  byte readByte = EEPROM.read(0);
  Serial.printf("  Written: %d, Read: %d - %s\n", testByte, readByte, 
                (testByte == readByte) ? "PASS" : "FAIL");
  
  // Test 2: Write and read integer
  Serial.println("\nTest 2: Write/Read integer");
  int testInt = 12345;
  EEPROM.put(10, testInt);
  EEPROM.commit();
  int readInt;
  EEPROM.get(10, readInt);
  Serial.printf("  Written: %d, Read: %d - %s\n", testInt, readInt, 
                (testInt == readInt) ? "PASS" : "FAIL");
  
  // Test 3: Write and read float
  Serial.println("\nTest 3: Write/Read float");
  float testFloat = 3.14159;
  EEPROM.put(20, testFloat);
  EEPROM.commit();
  float readFloat;
  EEPROM.get(20, readFloat);
  Serial.printf("  Written: %.5f, Read: %.5f - %s\n", testFloat, readFloat, 
                (abs(testFloat - readFloat) < 0.0001) ? "PASS" : "FAIL");
  
  // Test 4: Write and read string
  Serial.println("\nTest 4: Write/Read string");
  String testString = "ESP32-S3";
  int addr = 30;
  for (size_t i = 0; i < testString.length(); i++) {
    EEPROM.write(addr + i, testString[i]);
  }
  EEPROM.write(addr + testString.length(), '\0'); // Null terminator
  EEPROM.commit();
  
  String readString = "";
  char c;
  int i = 0;
  while ((c = EEPROM.read(addr + i)) != '\0' && i < 50) {
    readString += c;
    i++;
  }
  Serial.printf("  Written: '%s', Read: '%s' - %s\n", 
                testString.c_str(), readString.c_str(), 
                (testString == readString) ? "PASS" : "FAIL");
  
  // Test 5: Write array of bytes
  Serial.println("\nTest 5: Write/Read byte array");
  byte testArray[5] = {10, 20, 30, 40, 50};
  addr = 100;
  for (int i = 0; i < 5; i++) {
    EEPROM.write(addr + i, testArray[i]);
  }
  EEPROM.commit();
  
  byte readArray[5];
  for (int i = 0; i < 5; i++) {
    readArray[i] = EEPROM.read(addr + i);
  }
  
  Serial.print("  Written: [");
  for (int i = 0; i < 5; i++) {
    Serial.printf("%d%s", testArray[i], (i < 4) ? ", " : "");
  }
  Serial.print("]\n  Read:    [");
  for (int i = 0; i < 5; i++) {
    Serial.printf("%d%s", readArray[i], (i < 4) ? ", " : "");
  }
  Serial.println("]");
  
  bool arrayMatch = true;
  for (int i = 0; i < 5; i++) {
    if (testArray[i] != readArray[i]) arrayMatch = false;
  }
  Serial.printf("  Result: %s\n", arrayMatch ? "PASS" : "FAIL");
  
  // Test 6: Clear EEPROM section
  Serial.println("\nTest 6: Clear EEPROM section");
  addr = 200;
  int clearSize = 10;
  for (int i = 0; i < clearSize; i++) {
    EEPROM.write(addr + i, 255); // Write 0xFF
  }
  EEPROM.commit();
  
  bool allCleared = true;
  for (int i = 0; i < clearSize; i++) {
    if (EEPROM.read(addr + i) != 255) allCleared = false;
  }
  Serial.printf("  Cleared %d bytes - %s\n", clearSize, allCleared ? "PASS" : "FAIL");
  
  Serial.println("\n--- EEPROM Test Complete ---");
}

// ==================== SPIFFS TESTS ====================

void testSPIFFS() {
  Serial.println("\n--- SPIFFS Test Start ---");
  
  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) { // true = format if mount fails
    Serial.println("✗ SPIFFS mount failed!");
    return;
  }
  Serial.println("✓ SPIFFS mounted successfully");
  
  // Print SPIFFS info
  printSPIFFSInfo();
  
  // Test 1: Write text file
  Serial.println("\nTest 1: Write text file");
  writeTestFile();
  
  // Test 2: Read text file
  Serial.println("\nTest 2: Read text file");
  readTestFile();
  
  // Test 3: Append to file
  Serial.println("\nTest 3: Append to file");
  File file = SPIFFS.open(TEST_FILE, FILE_APPEND);
  if (file) {
    file.println("Appended line 1");
    file.println("Appended line 2");
    file.close();
    Serial.println("✓ Data appended successfully");
  } else {
    Serial.println("✗ Failed to open file for appending");
  }
  
  // Read again to verify append
  readTestFile();
  
  // Test 4: Write binary file
  Serial.println("\nTest 4: Write binary file");
  file = SPIFFS.open(TEST_FILE_2, FILE_WRITE);
  if (file) {
    byte binaryData[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    size_t written = file.write(binaryData, 10);
    file.close();
    Serial.printf("✓ Written %d bytes to binary file\n", written);
  } else {
    Serial.println("✗ Failed to create binary file");
  }
  
  // Test 5: Read binary file
  Serial.println("\nTest 5: Read binary file");
  file = SPIFFS.open(TEST_FILE_2, FILE_READ);
  if (file) {
    Serial.printf("  File size: %d bytes\n", file.size());
    Serial.print("  Data: [");
    while (file.available()) {
      byte b = file.read();
      Serial.printf("%d%s", b, file.available() ? ", " : "");
    }
    Serial.println("]");
    file.close();
    Serial.println("✓ Binary file read successfully");
  } else {
    Serial.println("✗ Failed to read binary file");
  }
  
  // Test 6: List all files
  Serial.println("\nTest 6: List all files");
  listSPIFFSFiles();
  
  // Test 7: Check if file exists
  Serial.println("\nTest 7: Check file existence");
  Serial.printf("  %s exists: %s\n", TEST_FILE, 
                SPIFFS.exists(TEST_FILE) ? "YES" : "NO");
  Serial.printf("  /nonexistent.txt exists: %s\n", 
                SPIFFS.exists("/nonexistent.txt") ? "YES" : "NO");
  
  // Test 8: Rename file
  Serial.println("\nTest 8: Rename file");
  if (SPIFFS.rename(TEST_FILE_2, "/renamed.bin")) {
    Serial.println("✓ File renamed successfully");
    listSPIFFSFiles();
  } else {
    Serial.println("✗ Failed to rename file");
  }
  
  // Test 9: Delete files
  Serial.println("\nTest 9: Delete files");
  deleteSPIFFSFile(TEST_FILE);
  deleteSPIFFSFile("/renamed.bin");
  
  // Final file list
  Serial.println("\nFinal file list:");
  listSPIFFSFiles();
  
  // Print final SPIFFS info
  printSPIFFSInfo();
  
  Serial.println("\n--- SPIFFS Test Complete ---");
}

void printSPIFFSInfo() {
  Serial.println("\n--- SPIFFS Information ---");
  Serial.printf("  Total bytes: %d\n", SPIFFS.totalBytes());
  Serial.printf("  Used bytes:  %d\n", SPIFFS.usedBytes());
  Serial.printf("  Free bytes:  %d\n", SPIFFS.totalBytes() - SPIFFS.usedBytes());
  Serial.printf("  Usage: %.1f%%\n", 
                (float)SPIFFS.usedBytes() / SPIFFS.totalBytes() * 100);
  Serial.println("-------------------------");
}

void writeTestFile() {
  File file = SPIFFS.open(TEST_FILE, FILE_WRITE);
  if (!file) {
    Serial.println("✗ Failed to open file for writing");
    return;
  }
  
  file.println("ESP32-S3 SPIFFS Test File");
  file.println("=========================");
  file.println("Line 1: Hello from ESP32-S3!");
  file.println("Line 2: SPIFFS is working!");
  file.printf("Line 3: Millis = %lu\n", millis());
  
  file.close();
  Serial.println("✓ Test file written successfully");
}

void readTestFile() {
  File file = SPIFFS.open(TEST_FILE, FILE_READ);
  if (!file) {
    Serial.println("✗ Failed to open file for reading");
    return;
  }
  
  Serial.printf("  File: %s\n", TEST_FILE);
  Serial.printf("  Size: %d bytes\n", file.size());
  Serial.println("  Contents:");
  Serial.println("  ---");
  while (file.available()) {
    String line = file.readStringUntil('\n');
    Serial.printf("  %s\n", line.c_str());
  }
  Serial.println("  ---");
  file.close();
  Serial.println("✓ File read successfully");
}

void listSPIFFSFiles() {
  File root = SPIFFS.open("/");
  if (!root) {
    Serial.println("✗ Failed to open root directory");
    return;
  }
  
  if (!root.isDirectory()) {
    Serial.println("✗ Root is not a directory");
    return;
  }
  
  Serial.println("  Files in SPIFFS:");
  File file = root.openNextFile();
  int fileCount = 0;
  while (file) {
    Serial.printf("    - %s (%d bytes)\n", file.name(), file.size());
    fileCount++;
    file = root.openNextFile();
  }
  
  if (fileCount == 0) {
    Serial.println("    (no files)");
  } else {
    Serial.printf("  Total files: %d\n", fileCount);
  }
}

void deleteSPIFFSFile(const char* path) {
  if (SPIFFS.remove(path)) {
    Serial.printf("  ✓ Deleted: %s\n", path);
  } else {
    Serial.printf("  ✗ Failed to delete: %s\n", path);
  }
}