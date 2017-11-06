// JSON Server
//
// author: Larry Li
// email: larryli@qq.com

#include "JsonServer.h"

AsyncCallbackWebHandler& JsonServer::onJson(const char* uri, WebRequestMethodComposite method, JsonHandler handle)
{
  return on(uri, method, [&](AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream(F("text/json"));
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    handle(request, root);
    if (root["ok"] == 0) {
      response->setCode(422);
    }
    root.printTo(*response);
    request->send(response);
  });
}
