Ryan Houlihan
Resource Allocator

Contents:
	ServerClientLog.txt: Log file of programs out
	serverClient.c: Program code
	serverClient.h: Header file

-Resource Allocator
	  - Single independent thread
	  - If client requests resources:
	       - Are resources available?
	       	     - Yes. Send requesting client the item numbers of requested resources. 
		     - No. Save request and satisfy it when enough resources are free
	  - Allocator Message Queue
	  	     - Queue to store requests from clients
 	  

________ORIGINAL OUTLINE:_________
 -4 Clients
   - request certain number of resources via its message queue
   - Holds the resources for a random amount of time, T1
   - Releases resources one by one until all are returned
     	      - Send message to allocator with: client ID, RETURN operation request, and the -resource number
   - Sleeps again for time, T2
   - Repeats previous 4 steps
   - Pauses T seconds after each consumtion before resuming
   - Client Message Queue
	 - Queue of all messages sent from the server to the client

 -Logger
	- Records a time stamp of every action in a log file. 

	
Main: 
      pthread_create(server)
      server.msqid = msgget(key, msgflg)
      for(client_num)
	pthread_create(client, server.msqid)
      //wait for all threads
      for(client_threads)
	pthread_join(client_threads) // wait on all the threads
      pthread_exit(NULL) 

RandomDelay:
	if(Resources == True)
		     wait(random()%client.T1)
		     
	else
		wait(client.T2)

ServerThread:
	while(TRUE)
		wait(server.empty)
		if(resources == 0 || moreResources == True)
			     message = msgrcv(only recieve RETURN messages)
	        else
			message = msgrcv
		moreResources = process(message)
	msgctl(closeQueue) // close queue and exit
	exit()

ClientThread:
	if(server = msgget(key, 0) == -1) // check for existing message queue
		// no queue found
		exit()
	client = msgget(clientQueue)	
	while(true)
		msg.msg_fm = clientID
		msg.msg_to = server
		msgsnd(resourceWanted)
		signal(server.empty)
		if(msgrcv)
			RandomDelay()
			for(resourcesUsed)
				msgsnd(Return resource)
	msgctl(closeQueue)
		
Process:
	if( Client wants resources && resources are avaliable)
	    for(x in requestedResources)
		for(y in resources[])
			if(resources[y] == 1)
				requestedResources[x] = resources[y]
				resources[y] = 0
	    msgsnd(requestedResources:resources], ClientID)
    	    resources += requestedResources
	else if( client wants to return resources)
	     resource[resourceID] = 1
	     resourceNum += 1
	else
		return 0
	return 1

