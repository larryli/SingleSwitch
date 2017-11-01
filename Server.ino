// HTTP 服务

static AsyncWebServer server(80);
//static AsyncEventSource es("/events");

// HTTP 服务配置
void server_setup()
{
  server.on("/", HTTP_GET, server_root);
  server.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404);
  });
  server.begin();
  Serial.println(F("Server start"));
}

static void server_root(AsyncWebServerRequest *request)
{
  request->send(200, F("text/plain"), F("Hello Switch"));
}
