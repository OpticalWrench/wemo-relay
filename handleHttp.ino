
#define DEFAULT_PAGE "relay.html"
#define NOT_FOUND_404_PAGE "404.html"

String getContentType(String fullFilename) {

  String contentType = "text/text";
  
  if(fullFilename.endsWith(".html")) {
    contentType = "text/html";
  } else if(fullFilename.endsWith(".htm")) {
    contentType = "text/html";
  } else if(fullFilename.endsWith(".css")) {
    contentType = "text/css";
  } else if(fullFilename.endsWith(".js")) {
    contentType = "application/javascript";
  } else if(fullFilename.endsWith(".jpg")) {
    contentType = "image/jpeg";
  } else if(fullFilename.endsWith(".jpeg")) {
    contentType = "image/jpeg";
  } else if(fullFilename.endsWith(".gif")) {
    contentType = "image/gif";
  } else if(fullFilename.endsWith(".png")) {
    contentType = "image/png";
  } else if(fullFilename.endsWith(".json")) {
    contentType = "application/json";
  } else if (fullFilename.endsWith(".ico")) {
    contentType = "image/x-icon";
  }

  return contentType;
}

void redirectClient(String targetPage) {
  #ifdef DEBUG
    Serial.print("redirecting to "); Serial.println(targetPage);
  #endif
  server.sendHeader("Location", targetPage);
  server.send(307); // using temporary redirect response so browser will still access "/" in future
  delay(1);
}

void returnOK() {
  String response = "OK";
  server.send ( 200, "text/plain", response );
}

void handleGarageButton() {
  activateGarageDoorButton();
  //returnOK();
  redirectClient(DEFAULT_PAGE);

  #ifdef DEBUG
    Serial.print("\nBUTTON!\n");
  #endif

}

void handleDirectoryListing() {
  String response = "<html><head><title>esp directory listing</title></head><body>";

  String path = server.arg("path");

  if(path == "") {
    path = "/";
  } 

  response += "directory listing for<br/>";
  response += path;
  response += "<br/><br/>";

  Dir dir = SPIFFS.openDir(path);
  while (dir.next()) {
      response += dir.fileName();
      File f = dir.openFile("r");
      response += "<br/>";
      response += "&nbsp&nbsp&nbsp&nbsp&nbspfile size: ";
      response += f.size();
      response += " bytes";
      response += "<br/>";
      f.close();
  }

  response += "</body></html>";
  server.send ( 200, "text/html", response );
}

void handleDirectoryListingWithLinks() {
  String response = "<html><head><title>esp directory listing</title></head><body>";

  String path = server.arg("path");

  if(path == "") {
    path = "/";
  } 

  response += "directory listing for<br/>";
  response += path;
  response += "<br/><br/>";

  Dir dir = SPIFFS.openDir(path);
  while (dir.next()) {
      response += "<a href=\"viewfile?file=";
      response += dir.fileName();
      response += "\">";
      response += dir.fileName();
      response += "</a>";
      File f = dir.openFile("r");
      response += "<br/>";
      response += "&nbsp&nbsp&nbsp&nbsp&nbspfile size: ";
      response += f.size();
      response += " bytes";
      response += "<br/>";
      f.close();
  }

  response += "<br/><a href=\"index.html\">back to start page</a></body></html>";
  server.send ( 200, "text/html", response );
}

void handleViewFile() {

  String response = "<html><head><title>esp directory listing</title></head><body>";

  if(server.args() < 1) {
    redirectClient(NOT_FOUND_404_PAGE);
    return;
  }

  String fullFilePath = server.arg("file");
  #ifdef DEBUG
    Serial.print("client requested view of: "); Serial.println(fullFilePath);
  #endif

  response += "file contents listing for<br/>";
  response += fullFilePath;
  response += "<br/><br/>";

  File fileToPrint = SPIFFS.open(fullFilePath, "r");
  int lineCount = 0;

  if(fileToPrint.size() > 3000) {
  // TODO cannot fit over 4k of file text into a String or the ESP reboots!
    fileToPrint.close();
    response += "<br/>file too large to view (over 4k)<br/><br/>";
    response += "<a href=\"lslinks\">back</a></body></html>";
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.send ( 200, "text/html", response );
    return;
  }
  if(fileToPrint) {
    while(fileToPrint.available()) {
      //Lets read line by line from the file
      String programLine = fileToPrint.readStringUntil('\n');
      response += ++lineCount;
      response += "&nbsp&nbsp&nbsp";
      response += programLine;
      response += "<br/>";
    }
    fileToPrint.close();
  } else {
    redirectClient(NOT_FOUND_404_PAGE);
    return;
  }

  response += "<a href=\"lslinks\">back</a></body></html>";
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send ( 200, "text/html", response );

}

#ifdef DEBUG
  void lsPrint(String dirPath = "") {
    String path = "/";
    if(dirPath != "") {
      path = dirPath;
    } 
    
    Serial.println();
    Serial.print("directory listing for "); Serial.println(path);

    Dir dir = SPIFFS.openDir(path);
    while (dir.next()) {
        Serial.println(dir.fileName());
        File f = dir.openFile("r");
        Serial.print("     file size: "); Serial.println(f.size());
        f.close();
    }
    Serial.println();
  }

  void printFileContents(String filePath = "") {
    File fileToPrint = SPIFFS.open(filePath, "r");
    if(fileToPrint) {
      while(fileToPrint.available()) {
        //Lets read line by line from the file
        String line = fileToPrint.readStringUntil('\n');
        Serial.println(line);
      }
      fileToPrint.close();
    }
  }
#endif

void handleRoot() {
  #ifdef DEBUG
    Serial.print("handleRoot() -> client request: "); Serial.println(server.uri());
  #endif
  redirectClient(DEFAULT_PAGE);
}

void notFound404() {
  #ifdef DEBUG
    Serial.print("notFound404() -> client request: "); Serial.println(server.uri());
  #endif
  redirectClient(NOT_FOUND_404_PAGE);
}

void handleEndPointNotFound() {

  // file requests are not programmed endpoints. If the requested file exists then send it to the requesting client.

  String fileFullPath = server.uri();

  #ifdef DEBUG
    Serial.print("handleEndPointNotFound() -> client request: "); Serial.println(fileFullPath);
  #endif

  // check if file exists
  if(!SPIFFS.exists(fileFullPath)) {
    #ifdef DEBUG
      Serial.print("Failed to find "); Serial.println(fileFullPath);
    #endif
    notFound404();
    return;
  }

  // try to open the file
  File requestedFile = SPIFFS.open(fileFullPath, "r");
  String contentLength = "0";
  String contentType = "unknown/unknown";

  if (!requestedFile) {
    #ifdef DEBUG
      Serial.print("Failed to open "); Serial.println(fileFullPath);
    #endif
    notFound404();
    return;
  }

  size_t fileSize = requestedFile.size();

  contentLength = fileSize; 
  
  server.setContentLength(contentLength.toInt());
  contentType = getContentType(requestedFile.name());

  //Serial.print("contentLength = "); Serial.println(contentLength);
  //Serial.print("contentType = "); Serial.println(contentType);

  server.send( 200, contentType.c_str() );

  size_t chunkSize = 500;
  if (fileSize < 500) {
    chunkSize = fileSize;
  }
  while(requestedFile.available() > 0)
  {
    char buf[chunkSize];
    requestedFile.read(reinterpret_cast<uint8_t*>(buf), chunkSize);

    server.client().write(buf, chunkSize);
    if(requestedFile.available() < 500)
      chunkSize = requestedFile.available();
  }
  requestedFile.close();
  server.client().stop(); // needed if not sending Content-Length.
}

