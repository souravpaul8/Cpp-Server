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
  char cwd[1024];

  string url = getcwd(cwd, sizeof(cwd)) + string("/html_files") + request->url;
  //cout<<"url is "<<url<<endl;

  response->HTTP_version = "1.0";

  struct stat sb;

  if (stat(url.c_str(), &sb) == 0) // requested path exists
  {
    response->status_code = "200";
    response->status_text = "OK";
    response->content_type = "text/html";
    string body;

    if (S_ISDIR(sb.st_mode)) {
      url=url+"/index.html";
    }

    string rdstring;
    ifstream file (url);
    if ( file.is_open() ) {
    while ( file ) {
      getline (file, rdstring);
      body=body+rdstring;

    }
    
    }else{
      body="<html><h1>404 NOT FOUND</h1><html>";  // requested path does not exists
    }

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

