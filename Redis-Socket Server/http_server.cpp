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
  response->status_code = "200";
  response->status_text = "OK";
  response->content_type = "text/html";
  string body;

  //std::cout << "1 " << request->url << std::endl;

  body =  "<!DOCTYPE html> <html lang=\"en\">\
                      <html>\
                      <head>\
                          <title>Hello Sourav</title>\
                      </head>\
                      <body>\
                          <p>Hello Sourav</p>\
                      </body>\
                      </html>";

  response->body = body;

  // time_t t; 
  // time(&t);
  // //cout<<"\nThis program has been writeen at (date and time): %s"<<ctime(&t);

  // char *timet =ctime(&t);
  // timet[strlen(timet)-1] = '\0';
  // response->time= timet;

  delete request;

  return response;
}

string HTTP_Response::get_string() {
 
 return ("HTTP/"+HTTP_version+" "+status_code+" "+status_text+"\r\n"+"Content-Type:"+content_type+"\r\n"+"Content-Length:"+to_string(strlen(body.c_str()))+"\r\n\n"+body);

}

