// JSON Server
//
// author: Larry Li
// email: larryli@qq.com

#ifndef JSON_SERVER_H
#define JSON_SERVER_H

#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/

typedef std::function<void(AsyncWebServerRequest *request, JsonObject &root)> JsonHandler;

class JsonServer: public AsyncWebServer {
  public:
    JsonServer(uint16_t port): AsyncWebServer(port) {}
    ~JsonServer() {}
    AsyncCallbackWebHandler& onJson(const char* uri, WebRequestMethodComposite method, JsonHandler handle);
};

#endif
