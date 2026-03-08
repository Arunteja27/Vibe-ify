#include "HttpServer.h"
#include "ApiController.h"
#include <iostream>
#include <sstream>

using namespace std;

HttpServer::HttpServer(int port)
    : port(port), listenSocket(INVALID_SOCKET), running(false),
      controller(nullptr) {}

HttpServer::~HttpServer() { stop(); }

bool HttpServer::start(ApiController *ctrl) {
  controller = ctrl;

  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    cerr << "  WSAStartup failed." << endl;
    return false;
  }

  listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (listenSocket == INVALID_SOCKET) {
    cerr << "  Socket creation failed." << endl;
    WSACleanup();
    return false;
  }

  int opt = 1;
  setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt,
             sizeof(opt));

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if (bind(listenSocket, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
    cerr << "  Bind failed on port " << port << "." << endl;
    closesocket(listenSocket);
    WSACleanup();
    return false;
  }

  if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
    cerr << "  Listen failed." << endl;
    closesocket(listenSocket);
    WSACleanup();
    return false;
  }

  running = true;
  serverThread = thread(&HttpServer::serverLoop, this);
  cout << "  API server started on http://localhost:" << port << endl;
  return true;
}

void HttpServer::stop() {
  running = false;
  if (listenSocket != INVALID_SOCKET) {
    closesocket(listenSocket);
    listenSocket = INVALID_SOCKET;
  }
  if (serverThread.joinable())
    serverThread.join();
  WSACleanup();
}

bool HttpServer::isRunning() const { return running; }

void HttpServer::serverLoop() {
  while (running) {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(listenSocket, &readSet);

    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    int result = select(0, &readSet, nullptr, nullptr, &timeout);
    if (result <= 0)
      continue;

    SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET)
      continue;

    handleClient(clientSocket);
    closesocket(clientSocket);
  }
}

void HttpServer::handleClient(SOCKET clientSocket) {
  char buffer[8192];
  memset(buffer, 0, sizeof(buffer));

  int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
  if (bytesRead <= 0)
    return;

  string rawRequest(buffer, bytesRead);
  HttpRequest req = parseRequest(rawRequest);

  if (req.method == "OPTIONS") {
    sendCorsHeaders(clientSocket);
    return;
  }

  if (controller) {
    string response = controller->handleRequest(req.method, req.path, req.body);
    sendResponse(clientSocket, 200, "OK", response);
  } else {
    sendResponse(clientSocket, 500, "Internal Server Error",
                 "{\"error\": \"No controller\"}");
  }
}

HttpServer::HttpRequest HttpServer::parseRequest(const string &raw) {
  HttpRequest req;

  size_t firstLine = raw.find("\r\n");
  if (firstLine == string::npos)
    firstLine = raw.find("\n");
  string line = raw.substr(0, firstLine);

  size_t sp1 = line.find(' ');
  size_t sp2 = line.find(' ', sp1 + 1);

  if (sp1 != string::npos)
    req.method = line.substr(0, sp1);
  if (sp1 != string::npos && sp2 != string::npos)
    req.path = line.substr(sp1 + 1, sp2 - sp1 - 1);

  size_t bodyStart = raw.find("\r\n\r\n");
  if (bodyStart != string::npos)
    req.body = raw.substr(bodyStart + 4);

  return req;
}

void HttpServer::sendResponse(SOCKET client, int statusCode,
                              const string &statusText, const string &body,
                              const string &contentType) {
  ostringstream response;
  response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
  response << "Content-Type: " << contentType << "\r\n";
  response << "Content-Length: " << body.length() << "\r\n";
  response << "Access-Control-Allow-Origin: *\r\n";
  response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
  response << "Access-Control-Allow-Headers: Content-Type\r\n";
  response << "Connection: close\r\n";
  response << "\r\n";
  response << body;

  string data = response.str();
  send(client, data.c_str(), (int)data.length(), 0);
}

void HttpServer::sendCorsHeaders(SOCKET client) {
  string response = "HTTP/1.1 204 No Content\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                    "Access-Control-Allow-Headers: Content-Type\r\n"
                    "Content-Length: 0\r\n"
                    "Connection: close\r\n"
                    "\r\n";
  send(client, response.c_str(), (int)response.length(), 0);
}
