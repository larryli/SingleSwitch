// JSON Server
//
// author: Larry Li
// email: larryli@qq.com

#include "JsonServer.h"

AsyncCallbackWebHandler& JsonServer::onJson(const char* uri, WebRequestMethodComposite method, JsonHandler handle)
{
  return on(uri, method, [&](AsyncWebServerRequest * request) {
    AsyncWebHeader* referer = request->getHeader("Referer");
    AsyncResponseStream *response = request->beginResponseStream(F("text/json"));
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    // csrf 检测 referer
    if (referer != nullptr && referer->value().indexOf("://" + request->host() + "") == -1) {
      root["ok"] = 0;
      root["message"] = F("内部错误，请重试。");
    } else {
      handle(request, root);
    }
    if (root["ok"] == 0) {
      response->setCode(422);
    }
    root.printTo(*response);
    request->send(response);
  });
}
