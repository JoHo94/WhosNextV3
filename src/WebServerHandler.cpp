#include "WebServerHandler.h"
#include <SD.h>
#include <SPI.h>

WebServerHandler::WebServerHandler(uint16_t port) : server(port), isEnabled(false) {}

void WebServerHandler::begin() {
    server.on("/list", HTTP_GET, [this](AsyncWebServerRequest *request) {
        listFiles(request);
    });

    server.on("/delete", HTTP_POST, [this](AsyncWebServerRequest *request) {
        deleteFile(request);
    });    
    server.on(
        "/upload", HTTP_POST,
        [](AsyncWebServerRequest *request) { request->send(200, "text/plain", "Upload complete"); },
        [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            handleFileUpload(request, filename, index, data, len, final); 
        }
    );
    server.on("/file", HTTP_GET, [this](AsyncWebServerRequest *request) {
        serveFile(request);
    });

    server.begin();
}

void WebServerHandler::enable(const String& ssid, const String& password) {
    if (!isEnabled) {
        WiFi.softAP(ssid.c_str(), password.c_str());
        begin();
        isEnabled = true;
    }
}

void WebServerHandler::disable() {
    if (isEnabled) {
        server.end();
        WiFi.softAPdisconnect(true);
        isEnabled = false;
    }
}

void WebServerHandler::listFiles(AsyncWebServerRequest *request) {
    File root = SD.open("/");
    if (!root || !root.isDirectory()) {
        request->send(500, "application/json", "{\"error\": \"Failed to open SD card\"}");
        return;
    }

    String response = "{\"files\": [";
    bool isFirst = true;

    File entry = root.openNextFile();
    while (entry) {
        if (!entry.isDirectory()) {
            if (!isFirst) response += ",";
            response += "{\"name\": \"" + String(entry.name()) + "\", \"size\": " + String(entry.size()) + "}";
            isFirst = false;
        }
        entry.close();
        entry = root.openNextFile();
    }

    root.rewindDirectory();
    File subDir = root.openNextFile();
    while (subDir) {
        if (subDir.isDirectory()) {
            String folderName = String(subDir.name());
            File subEntry = subDir.openNextFile();
            while (subEntry) {
                if (!subEntry.isDirectory()) {
                    if (!isFirst) response += ",";
                    response += "{\"name\": \"" + folderName + "/" + String(subEntry.name()) + "\", \"size\": " + String(subEntry.size()) + "}";
                    isFirst = false;
                }
                subEntry.close();
                subEntry = subDir.openNextFile();
            }
        }
        subDir.close();
        subDir = root.openNextFile();
    }

    response += "]}";
    request->send(200, "application/json", response);
}

void WebServerHandler::deleteFile(AsyncWebServerRequest *request) {
    if (!request->hasParam("file", true)) {
        request->send(400, "text/plain", "Missing 'file' parameter");
        return;
    }
    
    String filename = "/" + request->getParam("file", true)->value();
    if (SD.exists(filename)) {
        SD.remove(filename);
        request->send(200, "text/plain", "File deleted");
    } else {
        request->send(404, "text/plain", "File not found");
    }
}

void WebServerHandler::createDirectories(String path) {
    String dir = "";
    for (int i = 1; i < path.length(); i++) {
        if (path[i] == '/') {
            dir = path.substring(0, i);
            if (!SD.exists(dir)) {
                SD.mkdir(dir);
            }
        }
    }
}

void WebServerHandler::handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    static File uploadFile;
    
    Serial.printf("Upload Request: filename=%s, index=%d, len=%d, final=%d\n", filename.c_str(), index, len, final);
    String folder = "";
    
    if (request->hasParam("folder", true)) {
        folder = request->getParam("folder", true)->value();
        Serial.println(folder);
    }

    Serial.println("Folder is: " + folder);
    
    // Fullpath:
    String fullPath = "/" + folder + "/" + filename;
    if(folder == "") {
        fullPath = "/" + filename;
    }
    Serial.println("Fullpath is: " + fullPath);
    
    // Ensure the folder exists
    if (!SD.exists("/" + folder)) {
        Serial.printf("Creating directory: %s\n", ("/" + folder).c_str());
        SD.mkdir("/" + folder);
    }

    if (!index) {
        if (SD.exists(fullPath)) {
            Serial.printf("File already exists, deleting: %s\n", fullPath.c_str());
            SD.remove(fullPath);
        }
        Serial.printf("Opening file for writing: %s\n", fullPath.c_str());
        uploadFile = SD.open(fullPath, FILE_WRITE);
        if (!uploadFile) {
            request->send(500, "text/plain", "Failed to open file for writing");
            return;
        }
    }

    if (uploadFile) {
        Serial.printf("Writing %d bytes to file\n", len);
        uploadFile.write(data, len);
    }

    if (final) {
        Serial.println("Closing file");
        uploadFile.close();
        request->send(200, "text/plain", "File uploaded successfully");
    }
}

void WebServerHandler::serveFile(AsyncWebServerRequest *request) {
    if (!request->hasParam("name")) {
        request->send(400, "text/plain", "Missing 'name' parameter");
        return;
    }

    String folder = "";
    if (request->hasParam("name")) {
        folder = request->getParam("name")->value();
        Serial.println(folder);
    }
    Serial.println("name is: " + folder);

    String filename = "/" + request->getParam("name")->value();
    if (SD.exists(filename)) {
        File file = SD.open(filename, FILE_READ);
        if (file) {
            Serial.println("Sending file.");
            Serial.printf("File size: %d bytes\n", file.size());
            request->send(SD, filename, "audio/mpeg");
            file.close();
        } else {
            Serial.println("Faield to open file");
            request->send(500, "text/plain", "Failed to open file");
        }
    } else {
        request->send(404, "text/plain", "File not found");
    }
}
