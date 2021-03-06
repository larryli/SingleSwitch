// HTTP 服务

static JsonServer server(80);

// HTTP 服务配置
void server_setup()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, F("text/plain"), F("Hello Switch"));
  });
  api_setup();
  server.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404);
  });
  server.begin();
  Serial.println(F("Server start"));
}
