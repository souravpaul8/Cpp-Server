/* run using: ./load_gen localhost <server port> <number of concurrent users>
   <think time (in s)> <test duration (in s)> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>

#include <pthread.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

#define MAX_EVENTS 1

FILE *log_file;
int token = 0;
pthread_mutex_t tokenMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;

int time_up;
//FILE *log_file;

// user info struct
struct user_info {
  // user id
  int id;

  // socket info
  int portno;
  char *hostname;
  float think_time;

  // user metrics
  int total_count;
  float total_rtt;
};


struct thread_args {
  struct user_info *info;
  int user_count;
};

// error handling function
void error(char *msg) {
  perror(msg);
  //exit(0);
}

// time diff in seconds
float time_diff(struct timeval *t2, struct timeval *t1) {
  return (t2->tv_sec - t1->tv_sec) + (t2->tv_usec - t1->tv_usec) / 1e6;
}

void getTimestamp(char *timestamp, int timestampSize) {
    struct timeval tv;
    struct tm *tm_info;

    gettimeofday(&tv, NULL);
    tm_info = localtime(&tv.tv_sec);

    snprintf(timestamp, timestampSize, "%02d:%02d:%02d.%06ld", 
        tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, tv.tv_usec);
}

void *calculate_throughput(void *arg) {
  struct thread_args *args = (struct thread_args *)arg;
  struct user_info *info = args->info;
  int user_count = args->user_count;

  int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
  if(timer_fd == -1){
    perror("timerfd_fd");
    exit(1);
  }

  struct itimerspec timer_spec;
  memset(&timer_spec, 0, sizeof(timer_spec));
  timer_spec.it_value.tv_sec = 1;
  timer_spec.it_value.tv_nsec = 0;
  timer_spec.it_interval.tv_sec = 1;
  timer_spec.it_interval.tv_nsec = 0;

  if(timerfd_settime(timer_fd,0, &timer_spec, NULL) == -1) {
    perror("timerfd_settime");
    exit(1);
  }
  struct timeval start, end;

  int epoll_fd = epoll_create1(0);
  if(epoll_fd == -1) {
    perror("epoll_create1");
    exit(1);
  }

  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = timer_fd;
  if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &event) == -1) {
    perror("epoll_ctl");
    exit(1);
  }

  gettimeofday(&start, NULL);
  int last_count = 0;
  while (!time_up) {
      struct epoll_event events[MAX_EVENTS];
      int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
      if (num_events == -1) {
          perror("epoll_wait");
          exit(1);
      }
      for (int i = 0; i < num_events; i++) {
          if (events[i].data.fd == timer_fd) {
            uint64_t c = 0; //number of times the timer expired
            int r = read(timer_fd, &c, sizeof(c));
            int count=0;
            for (int i = 0; i < user_count; i++) {
              // Access info[i] to work with user-specific data
              count=count+info[i].total_count;
            }
            gettimeofday(&end, NULL);
            float wait_time_s = time_diff(&end, &start);
            float throughput = ((float)(count - last_count)) / wait_time_s;
            printf("%f\n", throughput);
            // printf("Throughput (requests/second): %f\n", throughput);
            // printf("Count: %d\n", count-last_count);
            // printf("Total Count: %d\n", last_count);
            last_count = count;
            gettimeofday(&start, NULL);
          }
      }
  }
  close(epoll_fd);
  close(timer_fd);
}

// user thread function
void *user_function(void *arg) {
  /* get user info */
  struct user_info *info = (struct user_info *)arg;

  int sockfd, n,num;
  char buffer[1024];
  char timestampStart[20];
  char timestampEnd[20]; // Adjust the size as needed
  struct timeval start, end;

  struct sockaddr_in serv_addr;
  struct hostent *server;
  info->total_count=0;
  info->total_rtt=0;

  char* requests[] = {"/index.html ","/apart1/index.html ","/apart2/index.html ","/apart1/flat11/index.html ", "/apart1/flat12/index.html ",
                      "/apart2/flat21/index.html ","/apart3/flat31/index.html ","/apart3/flat32/index.html "};

  
  server=gethostbyname(info->hostname);
  if(server== NULL){
    error("No such host");
  }
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bzero((char *)&serv_addr.sin_addr.s_addr, sizeof(serv_addr.sin_addr.s_addr));
  serv_addr.sin_port = htons(info->portno);

  while (1) {
    /* start timer */
    gettimeofday(&start, NULL);
    getTimestamp(timestampStart, sizeof(timestampStart));
    //printf("user id is %d %s %f\n",info->id,info->hostname,info->think_time);

    /* TODO: create socket */
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0){
      error("Error opening socket");
      continue;
    }

    /* TODO: send message to server */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    	error("ERROR connecting");
    	close(sockfd);
    	continue;
    }

    char tokenString[20];
    pthread_mutex_lock(&tokenMutex);
    token++;
    sprintf(tokenString, "%d",token);
    pthread_mutex_unlock(&tokenMutex);

    bzero(buffer, 1024);
    //strcpy(buffer,"GET /apart2/ HTTP/1.0\n");
    strcpy(buffer,"GET");
    strcat(buffer, " /");
    strcat(buffer, tokenString);
    num = (rand() % (8));
    strcat(buffer,requests[num]);
    strcat(buffer,"HTTP/1.0\r\n");
    strcat(buffer, "\r\n");
    
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0){
    	error("ERROR writing to socket");
    	close(sockfd);
    	continue;
    }

    //printf("%s\n", buffer);

    /* TODO: read reply from server */
    bzero(buffer, 1024);
    
    char fileBuffer[4096];
    strcat(fileBuffer, "======================================\n");
    strcat(fileBuffer, "token: ");
    strcat(fileBuffer, tokenString);
    strcat(fileBuffer, "\n");
    //printf("%s\n", tokenString);
    while((n = read(sockfd, buffer, sizeof(buffer))) > 0) {
        strncat(fileBuffer, buffer, n);
        // Check if the headerBuffer is getting too large, and break the loop if necessary
        if (strlen(fileBuffer) >= 4096 - 1) {
            printf("fileBuffer is full, breaking the loop\n");
            break;
        }
    }
    getTimestamp(timestampEnd, sizeof(timestampEnd));
    strcat(fileBuffer, "\n");
    strcat(fileBuffer, "request_begin_time: ");
    strcat(fileBuffer, timestampStart);
    strcat(fileBuffer, "\n");
    strcat(fileBuffer, "request_end_time: ");
    strcat(fileBuffer, timestampEnd);
    strcat(fileBuffer, "\n");
    strcat(fileBuffer, "======================================\n");

    pthread_mutex_lock(&fileMutex);
    fprintf(log_file, "%s \n", fileBuffer);
    fflush(log_file);
    pthread_mutex_unlock(&fileMutex);
    bzero(fileBuffer, 4096);
    //printf("buffer:%s \t %s \n", buffer, tokenString);
    
    if (n< 0){
      error("ERROR reading from socket");
      printf("Error at read\n");
    }
    else{
      info->total_count++;
    }
    //fprintf(log_file, "User #%d finished\n", info->id);
    //printf("%s\n", buffer);
  
    /* TODO: close socket */
    close(sockfd);

    /* end timer */
    gettimeofday(&end, NULL);

    /* if time up, break */
    if (time_up)
      break;

    /* TODO: update user metrics */
    info->total_rtt=info->total_rtt+time_diff(&end,&start);

    /* TODO: sleep for think time */
    usleep(info->think_time*(1000000));
  }

  /* exit thread */
  info->total_rtt=info->total_rtt/info->total_count;
  //fprintf(log_file, "User #%d finished\n", info->id);
  //fflush(log_file);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  int user_count, portno, test_duration;
  float think_time;
  char *hostname;
  int avg_count=0;
  float avg_rtt=0,throughput;

  if (argc != 6) {
    fprintf(stderr,
            "Usage: %s <hostname> <server port> <number of concurrent users> "
            "<think time (in s)> <test duration (in s)>\n",
            argv[0]);
    exit(0);
  }

  hostname = argv[1];
  portno = atoi(argv[2]);
  user_count = atoi(argv[3]);
  think_time = atof(argv[4]);
  test_duration = atoi(argv[5]);

  printf("Hostname: %s\n", hostname);
  printf("Port: %d\n", portno);
  printf("User Count: %d\n", user_count);
  printf("Think Time: %f s\n", think_time);
  printf("Test Duration: %d s\n", test_duration);
  
  /* open log file */
  log_file = fopen("log.txt", "w");
  if (log_file == NULL) {
      perror("Error opening log file");
      exit(1);
  }

  pthread_t threads[user_count];
  struct user_info info[user_count];
  struct timeval start, end;

  struct thread_args thread_data;
  thread_data.info = info;
  thread_data.user_count = user_count;
  pthread_t throughput_thread;
  pthread_create(&throughput_thread, NULL, calculate_throughput, (void *)&thread_data);

 
  /* start timer */
  gettimeofday(&start, NULL);
  time_up = 0;
  for (int i = 0; i < user_count; ++i) {
    /* TODO: initialize user info */
    info[i].id=i;
    info[i].hostname = hostname;
    info[i].portno=portno;
    info[i].think_time=think_time;

    /* TODO: create user thread */
    pthread_create(&threads[i],NULL,user_function,&info[i]);

    //fprintf(log_file, "Created thread %d\n", i);
  }

  /* TODO: wait for test duration */
  sleep(test_duration);

  //fprintf(log_file, "Woke up\n");

  /* end timer */
  time_up = 1;
  gettimeofday(&end, NULL);

  /* TODO: wait for all threads to finish */
  for (int i = 0; i < user_count; ++i) {
    pthread_join(threads[i],NULL);
  }

  /* TODO: print results */
  for (int i = 0; i < user_count; ++i) {
    //printf("user id %d  req. count: %d rtt :%f \n",i,info[i].total_count, info[i].total_rtt);
    avg_rtt=avg_rtt+info[i].total_rtt;
    avg_count=avg_count+info[i].total_count;
  }
  printf("avg count is %d rtt sum is %f \n",avg_count,avg_rtt);
  float wait_time_s=time_diff(&end,&start);
  avg_rtt=avg_rtt/user_count;
  throughput=avg_count/wait_time_s;//to do correct formula
  printf("average throughput : %f \n",throughput);
  printf("avg rtt : %f \n",avg_rtt);
 
  printf("Ending program\n");

  /* close log file */
  fclose(log_file);
  return 0;
}
