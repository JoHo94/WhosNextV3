#ifndef WEBSERVERHANDLER_H
#define WEBSERVERHANDLER_H

#include <ESPAsyncWebServer.h>

class WebServerHandler {
public:
    WebServerHandler(uint16_t port);
    void begin();
    void enable();
    void disable();

private:
    AsyncWebServer server;
    bool isEnabled;
    void listFiles(AsyncWebServerRequest *request);
    void deleteFile(AsyncWebServerRequest *request);
    void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void createDirectories(String path);
    void serveFile(AsyncWebServerRequest *request); // New function declaration
};

#endif // WEBSERVERHANDLER_H
