/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>

#include <pthread.h>
#include <queue>

#include "http_server.hh"

#define THREAD_POOL_SIZE 50
#define MAX_QUEUE_SIZE 10000

pthread_t thread_pool[THREAD_POOL_SIZE];
queue<int> fd_queue;
pthread_mutex_t fd_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t thread_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t main_cv = PTHREAD_COND_INITIALIZER;

void *start_function(void *arg);

void error(char *msg)
{
  perror(msg);
}

void *thread_function(void *arg)
{
  int fd = 0;
  while (true)
  {
    pthread_mutex_lock(&fd_queue_mutex);
    while (fd_queue.empty())
    {
      pthread_cond_wait(&thread_cv, &fd_queue_mutex);
    }
    fd = fd_queue.front();
    fd_queue.pop();
    if (fd_queue.size() == (MAX_QUEUE_SIZE - 1))
    {
      pthread_cond_signal(&main_cv);
    }
    pthread_mutex_unlock(&fd_queue_mutex);

    if (fd != 0)
    {
      start_function(&fd);
    }
    //sleep(20);
  }
}

void *start_function(void *arg)
{
  int my_arg = *((int *)arg);
  char buffer[256];
  int n;

  // ...thread processing...
  bzero(buffer, 256);
  n = read(my_arg, buffer, 255);
  if (n <= 0)
  {
    error("ERROR reading from socket");
    close(my_arg);
    return arg;
  }
  // cout << "Here is the message: \n"
  //      << buffer << endl;

  HTTP_Response *response = handle_request(buffer);

  string tosend = response->get_string();

  // printf("Here is the response: %s", tosend.c_str());

  n = write(my_arg, tosend.c_str(), strlen(tosend.c_str()));
  if (n < 0)
    error("ERROR writing to socket");

  close(my_arg);
  delete response;
  return arg;
}

int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;

  if (argc < 2)
  {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  // creation of threads in pool
  for (int i = 0; i < THREAD_POOL_SIZE; i++)
  {
    pthread_create(&thread_pool[i], NULL, thread_function, NULL);
  }

  /* create socket */

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  /* fill in port number to listen on. IP address can be anything (INADDR_ANY)
   */

  bzero((char *)&serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  /* bind socket to this port number on this machine */

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  /* listen for incoming connection requests */

  listen(sockfd, 10000);
  clilen = sizeof(cli_addr);

  /* accept a new request, create a newsockfd */

  while (1)
  {

    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0){
      error("ERROR on accept");
      continue;
      }

    pthread_mutex_lock(&fd_queue_mutex);
    while (fd_queue.size() >= MAX_QUEUE_SIZE)
    {
      cout << "ERROR on queue full" << endl;
      pthread_cond_wait(&main_cv, &fd_queue_mutex);
    }
    // cout<<"pushing element in queue"<<endl;
    fd_queue.push(newsockfd);
    pthread_cond_signal(&thread_cv);
    pthread_mutex_unlock(&fd_queue_mutex);
  }

  return 0;
}
