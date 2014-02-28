/*
 *Ryan Houlihan 
 *Lab: Resource Allocator 
 *CIS 3207 

 *Program creates CLIENTNUM clients and a server with RESOURCES resources. 
 *The clients ask for a random number of resources by sending a message to the server queue. 
 *The server queue process's the message and sends a message to the clients queue, with allocated resources. 
 *The client "uses" these for a random amount of time and then returns them to the server via messages to the servers queue. 
 *Afterwords the client sleeps for a random amount of time and then asks for more resources.
 *I used a mutex to print the log contents wihtout interruption.
 */


#include "./serverClient.h"
#define SERVER 0
#define CLIENT 1 

// Globals
pthread_mutex_t mutex;
static size_t MESSAGESIZE = sizeof(MESSAGE);
time_t rawtime;
struct tm * timeinfo;
struct timeval tv;

int main(){
  pthread_t threads[CLIENTNUM+1], serverThread;
  pthread_attr_t attr;
  long sid;
  key_t mykey;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_mutex_init(&mutex, NULL);
  // create server
  mykey = ftok(".", 'a');
  sid = msgget(mykey, IPC_CREAT | 0660);
  // Sends server its own PID
  pthread_create(&threads[0], &attr, server, (void *)sid);

  int x;

  // create clients
  for(x = 0; x < CLIENTNUM; x++)
    pthread_create(&threads[CLIENT+x], &attr, client, (void *)sid);

  // waits for threads to finish
  for(x = 0; x < CLIENTNUM+1; x++)
    pthread_join(threads[x], 0);

  // Clean and exit
  pthread_attr_destroy(&attr);
  pthread_exit(0);

  return 0;
}

/*Server:
 * While(done < clients): 
 *  getmessage()
 *  If(type = Allocate && resourceAvailable > resourcesNeeded)
 *       for(x < resourcesNeeded)
 *           for(y < ttlResources)
 *               if(resources[y] == 0)
 *                    sendClient(resource[y])
 *  else if(type = allocate)
 *     for(x < resourcesNeeded)
 *       getmessage(type = RETURN)
 *          sendClient(returned resource)
 *  else if(type = RETURN)
 *      resource[message] = 0
 *  else if(type = DONE)
 *     done++
 *  else
 *     exit()
 */
void *server(void* sid){
  int *resources, done = 0, x, y, ttlNeed, ttlRe = RESOURCES, tmpmessage[BUFSIZE];
  MESSAGE msg;
  long qid = (long) sid; 
  file = fopen(FILENAME, "a");
  // Initialize all resources to empty
  resources = (int *) calloc(RESOURCES,sizeof(int));
  // While clients still need resources
  while(done < CLIENTNUM){
    // Wait for message from client
    if((msgrcv(qid, &msg, MESSAGESIZE, 0, MSG_NOERROR))==-1){
      perror("Server: msgrcv");
      exit(1);
    }
    // If client wants resources and we have them
    if((msg.mtype == ALLOCATE) && (ttlRe >= (int) msg.message[0])){
      msg.mto = msg.mfrom;
      msg.mfrom = qid;
      // find which resources are available
      ttlNeed = msg.message[0];
      findResource(resources, msg.message, ttlNeed);
      ttlRe -= ttlNeed;
      //LOG INFO
      pthread_mutex_lock(&mutex);
      time(&rawtime);
      timeinfo = localtime(&rawtime);
      gettimeofday(&tv, NULL);
      fprintf(file,"\n>>::TIMESTAMP[%d]::<< %s", (int)tv.tv_usec, asctime(timeinfo)); 
      for(x = 0; x < ttlNeed; x++)
	fprintf(file,"\t SERVER[%ld]::CLIENT[%ld] was sent [resource[%d]]\n", qid, msg.mto, msg.message[x]);
     
       // Send client requested resources
      msgsnd(msg.mto, &msg, MESSAGESIZE, 0);
      pthread_mutex_unlock(&mutex);
    }
    // If client wants resources but we don't have them
    else if(msg.mtype == ALLOCATE){
      int waitPID = msg.mfrom;
      ttlNeed = msg.message[0];
    // Recover ttl amount of resources client asked for
      int tmp = ttlNeed-ttlRe;
      for(x = 0; x < tmp; x++){
	if((msgrcv(qid, &msg, MESSAGESIZE, RETURN, 0))==-1){
	  perror("Server: msgrcv");
	  exit(2);
	}
	resources[msg.message[0]-1] = 0;
	ttlRe++;
	//LOG INFO
	pthread_mutex_lock(&mutex);
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	gettimeofday(&tv, NULL);
	fprintf(file, "\n>>::TIMESTAMP[%d]::<< %s \t SERVER[%ld]::CLIENT[%ld] returned [resource[%d]]\n",(int)tv.tv_usec, asctime(timeinfo), qid, msg.mfrom, msg.message[0]);
	pthread_mutex_unlock(&mutex);
      }
      findResource(resources, msg.message, ttlNeed);
      ttlRe = 0;
      msg.mtype = ALLOCATE;
      msg.mfrom = qid;
      msg.mto = waitPID;
      //LOG INFO
      pthread_mutex_lock(&mutex);
      time(&rawtime);
      timeinfo = localtime(&rawtime);
      gettimeofday(&tv, NULL);
      fprintf(file, "\n>>::TIMESTAMP[%d]::<< %s", (int)tv.tv_usec, asctime(timeinfo)); 
      for(x = 0; x < ttlNeed; x++)
	fprintf(file,"\t SERVER[%ld]::CLIENT[%ld] was sent [resource[%d]]\n", qid,  msg.mto, msg.message[x]);
      // Send allocation message to client
      if((msgsnd(msg.mto, &msg, MESSAGESIZE, 0))==-1)
	exit(-1);
      pthread_mutex_unlock(&mutex);
    }    
    // Return resource client is done with
    else if(msg.mtype == RETURN){
      resources[msg.message[0]-1] = 0;
      ttlRe++;
      //LOG INFO
      pthread_mutex_lock(&mutex);
      time(&rawtime);
      timeinfo = localtime(&rawtime);
      gettimeofday(&tv, NULL);
      fprintf(file, "\n>>::TIMESTAMP[%d]::<< %s \t SERVER[%ld]::CLIENT[%ld] returned [resource[%d]]\n", (int)tv.tv_usec, asctime(timeinfo), qid, msg.mfrom, msg.message[0]);
      pthread_mutex_unlock(&mutex);
    }
    else if(msg.mtype == DONE){
      pthread_mutex_lock(&mutex);
      time(&rawtime);
      timeinfo = localtime(&rawtime);
      gettimeofday(&tv, NULL);
      fprintf(file, "\n>>::TIMESTAMP[%d]::<< %s \t SERVER[%ld]::CLIENT[%ld] is DONE\n", (int)tv.tv_usec, asctime(timeinfo), qid, msg.mfrom);
      pthread_mutex_unlock(&mutex);
      done++;
    }
    else
      exit(-1);
  }
  fprintf(file, "\n<<::RESOURCE ARRAY::>>\n");
  for(x = 0; x < RESOURCES; x++)
    fprintf(file, "\tResource[%d] = %d\n", x+1, resources[x]);
  // Close queue and file and exit
  fclose(file);
  msgctl(qid, IPC_RMID, (struct msqid_ds *) 0);
  pthread_exit(NULL);
  return 0;
}

// Finds an available resource
void *findResource(int* resources, int* message, int reNeed){
  int x,y;
  for(x = 0; x < reNeed; x++){
    for(y = 0; y < RESOURCES; y++){
       if(resources[y] == 0){
	    message[x] = y+1;
	    resources[y] = 1;
	    y = RESOURCES;
      }
    }
  }
}

/*Client
 * createQueue
 * while(notDone)
 *   sendResourceRequest
 *   recieveResources
 *   sleep()
 *   returnResources
 *   sleep()
 * sendDoneStatus(server)
 */
void *client(void* sid){
  int key, z, x, resourceNeeded;
  MESSAGE msg;
  long qid, serverID = (long) sid;
  int T1 = (random()%10 + 1), T2 = (random()%10 + 1), reReq = (random()%BUFSIZE + 1);
  file = fopen(FILENAME, "a");

  // Create message queue for Client
  key = random()%12000;
  while((qid = msgget(key, 0))!=-1)
    key = random()%12000;
  if((qid = msgget(key, IPC_CREAT | 0660))==-1)
    exit(1);

  // For TURNS, request resources, sleep, return resources, sleep
  for(z = 0; z < TURNS; z++){
    // Create allocation request to server
    msg.mfrom = qid;
    msg.mto = serverID;
    msg.mtype = ALLOCATE;
    pthread_mutex_lock(&mutex);
    resourceNeeded = (random()%reReq)+1;
    msg.message[0] = resourceNeeded;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    gettimeofday(&tv, NULL);
    fprintf(file, "\n>>::TIMESTAMP[%d]::<< %s \t CLIENT[%ld] requests [%d] Resources\n", (int)tv.tv_usec, asctime(timeinfo), qid, msg.message[0]); 
    // Send allocation request to server
    if((msgsnd(msg.mto, &msg, MESSAGESIZE, 0))==-1)
      exit(-1);
    pthread_mutex_unlock(&mutex);
    // Wait for response from server
    if(msgrcv(qid, &msg, MESSAGESIZE, ALLOCATE, 0)==-1)
      exit(2);
    
    // LOG INFO
    pthread_mutex_lock(&mutex);
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    gettimeofday(&tv, NULL);
    fprintf(file, "\n>>::TIMESTAMP[%d]::<< %s", (int)tv.tv_usec, asctime(timeinfo)); 
    for(x = 0; x<resourceNeeded; x++)
      fprintf(file,"\t CLIENT[%ld]::Recieved resource[%d]\n", qid, msg.message[x]); 
    pthread_mutex_unlock(&mutex);
    
    // Hold onto resources
    sleep((random()%T1));

    // Return resources to Server
    msg.mto = serverID;
    msg.mfrom = qid;
    msg.mtype = RETURN;
   
    // Log and return each resource to server
    for(x = 0; x < resourceNeeded; x++){
      pthread_mutex_lock(&mutex);
      msg.message[0] = msg.message[x];
      time(&rawtime);
      timeinfo = localtime(&rawtime);
      gettimeofday(&tv, NULL);
      fprintf(file, "\n>>::TIMESTAMP[%d]::<< %s \t CLIENT[%ld]::Returning resource[%d]\n", (int) tv.tv_usec, asctime(timeinfo), qid, msg.message[0]); 
      if((msgsnd(msg.mto, &msg, MESSAGESIZE, 0))==-1)
	  exit(-1);
    pthread_mutex_unlock(&mutex);
    }
    // Take a short break
    sleep(random()%T2);
  }
  // Tell server client is done and no longe needs resources
  msg.mtype = DONE;
  if((msgsnd(serverID, &msg, MESSAGESIZE, 0))==-1)
    exit(-1);
  // Close client queue
  msgctl(qid, IPC_RMID, (struct msqid_ds *) 0);
  pthread_exit(0);
  return 0;
}
