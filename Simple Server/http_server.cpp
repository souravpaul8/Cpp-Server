#include "http_server.hh"

#include <vector>

#include <sys/stat.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include<iostream>
#include <cstdio>
#include <unistd.h>
#include<time.h>
#include <map>

using namespace std;

std::map<std::string, std::string> htmlMap = {
      {"/", "<!DOCTYPE html> <html lang=\"en\">\
              <head>\
                <meta charset=\"UTF-8\">\
                <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\
                <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
                <title>Document</title>\
              </head>\
              <body>\
                <h1>This is BootCampus.</h1>\
                <a href=\"apart1/index.html\">Apartment 1</a>\
                <a href=\"apart2/index.html\">Apartment 2</a>\
                <a href=\"apart3/index.html\">Apartment 3</a>\
              </body>\
              </html>"},
      {"/index.html", "<!DOCTYPE html> <html lang=\"en\">\
              <head>\
                <meta charset=\"UTF-8\">\
                <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\
                <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
                <title>Document</title>\
              </head>\
              <body>\
                <h1>This is BootCampus.</h1>\
                <a href=\"apart1/index.html\">Apartment 1</a>\
                <a href=\"apart2/index.html\">Apartment 2</a>\
                <a href=\"apart3/index.html\">Apartment 3</a>\
              </body>\
              </html>"},
      {"/apart1/index.html", "<!DOCTYPE html>\
                  <html lang=\"en\">\
                  <head>\
                    <meta charset=\"UTF-8\">\
                    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\
                    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
                    <title>Apartment 1</title>\
                  </head>\
                  <body>\
                    <h1> This is Apartment 1. Child of Society BootCampus.</h1>\
                    <a href=\"flat11/index.html\">Flat 1</a>\
                    <a href=\"flat12/index.html\">Flat 2</a>\
                    <a href=\"../index.html\">GO to home</a>\
                  </body>\
                  </html>"},
      {"/apart1/flat11/index.html", "<!DOCTYPE html>\
                          <html lang=\"en\">\
                          <head>\
                              <meta charset=\"UTF-8\">\
                              <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\
                              <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
                              <title>Flat 11</title>\
                          </head>\
                          <body>\
                              <h1>Welcome to 1st Flat of Apartment 1.</h1>\
                              <a href=\"../../index.html\">GO to home</a>\
                          </body>\
                          </html>"},

      {"/apart1/flat12/index.html", "<!DOCTYPE html>\
                          <html lang=\"en\">\
                          <head>\
                              <meta charset=\"UTF-8\">\
                              <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\
                              <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
                              <title>Flat 12</title>\
                          </head>\
                          <body>\
                              <h1>Welcome to 2nd Flat of Apartment 1.</h1>\
                              <a href=\"../../index.html\">GO to home</a>\
                          </body>\
                          </html>"},
      {"/apart2/index.html", "<!DOCTYPE html>\
                  <html lang=\"en\">\
                  <head>\
                      <meta charset=\"UTF-8\">\
                      <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\
                      <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
                      <title>Apartment 2</title>\
                  </head>\
                  <body>\
                      <h1>This is Apartment 2. Child of Society BootCampus.</h1>\
                      <a href=\"flat21/index.html\">Flat 1st</a>\
                      <a href=\"../index.html\">GO to home</a>\
                  </body>\
                  </html>"},
      {"/apart2/flat21/index.html", "<!DOCTYPE html>\
                                      <html lang=\"en\">\
                                      <head>\
                                          <meta charset=\"UTF-8\">\
                                          <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\
                                          <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
                                          <title>Document</title>\
                                      </head>\
                                      <body>\
                                          <h1>Welcome to 1st Flat of Apartment 2.</h1>\
                                          <a href=\"../../index.html\">GO to home</a>\
                                      </body>\
                                      </html>"},
      {"/apart3/index.html", "<!DOCTYPE html>\
                                <html lang=\"en\">\
                                <head>\
                                    <meta charset=\"UTF-8\">\
                                    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\
                                    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
                                    <title>Apartment 1</title>\
                                </head>\
                                <body>\
                                    <h1>This is Apartment 3. Child of Society BootCampus.</h1>\
                                    <a href=\"flat31/index.html\">Flat 1st</a>\
                                    <a href=\"flat32/index.html\">Flat 2nd</a>\
                                    <a href=\"../index.html\">GO to home</a>\
                                </body>\
                                </html>"},

        {"/apart3/flat31/index.html", "<!DOCTYPE html>\
                                        <html lang=\"en\">\
                                        <head>\
                                            <meta charset=\"UTF-8\">\
                                            <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\
                                            <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
                                            <title>Flat31</title>\
                                        </head>\
                                        <body>\
                                            <h1>Welcome to 1st Flat of Apartment 3.</h1>\
                                            <a href=\"../../index.html\">GO to home</a>\
                                        </body>\
                                        </html>"},

        {"/apart3/flat32/index.html", "<!DOCTYPE html>\
                                        <html lang=\"en\">\
                                        <head>\
                                            <meta charset=\"UTF-8\">\
                                            <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\
                                            <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
                                            <title>Flat 32</title>\
                                        </head>\
                                        <body>\
                                            <h1>Welcome to 2nd Flat of Apartment 3.</h1>\
                                            <a href=\"../../index.html\">GO to home</a>\
                                        </body>\
                                        </html>"},

      };

vector<string> split(const string &s, char delim) {
  vector<string> elems;

  stringstream ss(s);
  string item;

  while (getline(ss, item, delim)) {
    if (!item.empty())
      elems.push_back(item);
  }

  return elems;
}

HTTP_Request::HTTP_Request(string request) {
  vector<string> lines = split(request, '\n');
  vector<string> first_line = split(lines[0], ' ');

  this->HTTP_version = "1.0"; // We'll be using 1.0 irrespective of the request
  this->method = first_line[0];
  this->url = first_line[1];

  if (this->method != "GET") {
    cerr << "Method '" << this->method << "' not supported" << endl;
    exit(1);
  }
}

HTTP_Response *handle_request(string req) {
  HTTP_Request *request = new HTTP_Request(req);

  HTTP_Response *response = new HTTP_Response();

  response->HTTP_version = "1.0";

  struct stat sb;

  if (htmlMap.find(request->url) != htmlMap.end()) // requested path exists
  {
    response->status_code = "200";
    response->status_text = "OK";
    response->content_type = "text/html";
    string body;

     std::cout << "1 " << request->url << std::endl;

    body = body + htmlMap[request->url];

    response->body = body;
    
  }

  else {
    response->status_code = "404";
    response->content_type = "text/html";
    response->body = "<html><h1>404 NOT FOUND</h1><html>";
    
  }

  time_t t; 
  time(&t);
  //cout<<"\nThis program has been writeen at (date and time): %s"<<ctime(&t);

  char *timet =ctime(&t);
  timet[strlen(timet)-1] = '\0';
  response->time= timet;

  delete request;

  return response;
}

string HTTP_Response::get_string() {
 
 return ("HTTP/"+HTTP_version+" "+status_code+" "+status_text+"\r\n"+"Date: "+ time+"\r\n"+"Content-Type:"+content_type+"\r\n"+"Content-Length:"+to_string(strlen(body.c_str()))+"\r\n\n"+body);

}

