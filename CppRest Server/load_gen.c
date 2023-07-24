/* run using: ./load_gen localhost <server port> <number of concurrent users>
   <think time (in s)> <test duration (in s)> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>

#include <pthread.h>
#include <sys/time.h>


int time_up;
FILE *log_file;

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

// error handling function
void error(char *msg) {
  perror(msg);
  //exit(0);
}

// time diff in seconds
float time_diff(struct timeval *t2, struct timeval *t1) {
  return (t2->tv_sec - t1->tv_sec) + (t2->tv_usec - t1->tv_usec) / 1e6;
}

// user thread function
void *user_function(void *arg) {
  /* get user info */
  struct user_info *info = (struct user_info *)arg;

  int sockfd, n,num;
  char buffer[256];
  struct timeval start, end;

  struct sockaddr_in serv_addr;
  struct hostent *server;
  info->total_count=0;
  info->total_rtt=0;

  char* requests[] = {" / "," /apart1/ "," /apart2/ "," /apart1/flat11/ ", " /apart1/flat12/",
                      " /apart2/flat21/ "," /apart3/flat31/"," /apart3/flat32/ "};

  
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

    bzero(buffer, 256);
    //strcpy(buffer,"GET /apart2/ HTTP/1.0\n");
    strcpy(buffer,"GET");
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

    /* TODO: read reply from server */
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n<= 0){
      error("ERROR reading from socket");
      printf("Error at read\n");
    }
    else{
      info->total_count++;
    }

    
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
  fprintf(log_file, "User #%d finished\n", info->id);
  fflush(log_file);
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
  log_file = fopen("load_gen.log", "w");

  pthread_t threads[user_count];
  struct user_info info[user_count];
  struct timeval start, end;

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

    fprintf(log_file, "Created thread %d\n", i);
  }

  /* TODO: wait for test duration */
  sleep(test_duration);

  fprintf(log_file, "Woke up\n");

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
  throughput=avg_count/wait_time_s;//to do correct formula
  avg_rtt=avg_rtt/user_count;
  printf("average throughput is %f avg rtt: %f \n",throughput,avg_rtt);
 
  printf("Ending program\n");

  /* close log file */
  fclose(log_file);

  return 0;
}
