#ifndef HTTPSERVER_H
#define HTTPSERVER_H

// clang-format off
#include <winsock2.h>
#include <ws2tcpip.h>
// clang-format on

#include <string>
#include <thread>

class ApiController;

// Raw Winsock2 HTTP server on a separate thread.
class HttpServer {
public:
  HttpServer(int port = 8080);
  ~HttpServer();

  bool start(ApiController *controller);
  void stop();
  bool isRunning() const;

private:
  int port;
  SOCKET listenSocket;
  bool running;
  std::thread serverThread;
  ApiController *controller;

  void serverLoop();
  void handleClient(SOCKET clientSocket);

  struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
  };

  HttpRequest parseRequest(const std::string &raw);
  void sendResponse(SOCKET client, int statusCode,
                    const std::string &statusText, const std::string &body,
                    const std::string &contentType = "application/json");
  void sendCorsHeaders(SOCKET client);
};

#endif
