// curl -F "file=@$PWD/index.html" 192.168.1.XX/upload
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

#define LEFT_MOTOR_PIN_1 9
#define LEFT_MOTOR_PIN_2 10
#define RIGHT_MOTOR_PIN_1 4
#define RIGHT_MOTOR_PIN_2 5

ESP8266WebServer server;
char* ssid = "Whegolas";

// hold uploaded file
File fsUploadFile;

// websockets server for handling messages sent by the client
WebSocketsServer webSocket = WebSocketsServer(81);

void setup()
{
  pinMode(LEFT_MOTOR_PIN_1, OUTPUT);
  pinMode(LEFT_MOTOR_PIN_2, OUTPUT);
  pinMode(RIGHT_MOTOR_PIN_1, OUTPUT);
  pinMode(RIGHT_MOTOR_PIN_2, OUTPUT);

  SPIFFS.begin();
  Serial.begin(115200);
  delay(2000);
  Serial.println();

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP("Whegolas") ? "Ready" : "Failed!");

  server.on("/", handleIndexFile);
  // list available files
  server.on("/list", HTTP_GET, handleFileList);
  // handle file upload
  server.on("/upload", HTTP_POST, [](){
    server.send(200, "text/plain", "{\"success\":1}");
  }, handleFileUpload);
  server.begin();
  webSocket.begin();
  // function to be called whenever there's a websocket event
  webSocket.onEvent(webSocketEvent);
}

void setMotorSpeed(int pin1, int pin2, double motor_speed) {
  Serial.println(motor_speed);
  if(motor_speed == 0) {
    analogWrite(pin1, 0);
    analogWrite(pin2, 0);
  } else if(motor_speed > 0){
    analogWrite(pin2, 0);
    analogWrite(pin1, (double) (motor_speed * 255));
  } else {
    analogWrite(pin1, 0);
    analogWrite(pin2, (double) (-motor_speed * 255));
  }
}

void driveMotors(double forwards, double turnSpeed) {
  double left = forwards + turnSpeed;
  double right = forwards - turnSpeed;

  double max_value = max(max(left, right), 1.0);
  left = left / max_value;
  right = right / max_value;

  setMotorSpeed(LEFT_MOTOR_PIN_1, LEFT_MOTOR_PIN_2, left);
  setMotorSpeed(RIGHT_MOTOR_PIN_1, RIGHT_MOTOR_PIN_1, right);
}

void loop()
{
//  for(double i=-1.0; i < 1; i+=0.01) {
//    setMotorSpeed(LEFT_MOTOR_PIN_1, LEFT_MOTOR_PIN_2, i);
//    delay(10);
//  }
//
//  for(double i=1.0; i > -1; i-=0.01) {
//    setMotorSpeed(LEFT_MOTOR_PIN_1, LEFT_MOTOR_PIN_2, i);
//    delay(10);
//  }
  
  webSocket.loop();
  server.handleClient();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  if(type == WStype_TEXT)
  {
    // handle the websocket messages with direction and speed
    // by parsing the parameters from a JSON string
    String payload_str = String((char*) payload);
    // using the ArduinoJson library
    StaticJsonDocument<200> doc;
    // deserialize the data
    DeserializationError error = deserializeJson(doc, payload_str);
    // parse the parameters we expect to receive (TO-DO: error handling)
    String turnSpeed = doc["xSpeed"]; // Right is positive
    String forwardSpeed = doc["ySpeed"]; // Forward is positive

    Serial.print("forward:");
    Serial.print(forwardSpeed);
    Serial.print(", turn:");
    Serial.println(turnSpeed);

    driveMotors(forwardSpeed.toDouble(), turnSpeed.toDouble());
  }
}

void handleFileUpload()
{
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    if(!filename.startsWith("/"))
      filename = "/"+filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
  } else if(upload.status == UPLOAD_FILE_WRITE)
  {
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END)
  {
    if(fsUploadFile)
      fsUploadFile.close();
    Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
  }
}

void handleFileList()
{
  String path = "/";
  // Assuming there are no subdirectories
  Dir dir = SPIFFS.openDir(path);
  String output = "[";
  while(dir.next())
  {
    File entry = dir.openFile("r");
    // Separate by comma if there are multiple files
    if(output != "[")
      output += ",";
    output += String(entry.name()).substring(1);
    entry.close();
  }
  output += "]";
  server.send(200, "text/plain", output);
}

void handleIndexFile()
{
  File file = SPIFFS.open("/index.html","r");
  server.streamFile(file, "text/html");
  file.close();
}
