#include "types.h"
#include "stat.h"
#include "user.h"

int taskCnt = 16;

void func( int i ){
   	 int buffer[1024];		
     
	for( int k = 0; k <100000; k++){
             
			for( int j = 0; j < 8; j++){
                    int rnd = (k+j)%1024;
					if( buffer[rnd] %k ==0)
					    buffer[rnd] = rnd;
				    else
					   buffer[rnd] = rnd+1;
			  }
			    
			  		
	}
	int pid = getpid();
	printf(1, "Task-%d Pid-%d is done turnround-time %d  swtch_cnt %d\n\n", i, pid,
			       gettt(), getSwtchCnt( )); 
	exit();

}


int main(int argc, char **argv){
	int pid;
	
	int policy = atoi(argv[1]);
    setSchd(policy);

	setnice(getpid(), 0);
	
    for(int  i = 0; i < taskCnt; i++ ){ 
	    pid = fork();

		if( pid == 0 ){
		       pid = getpid( );
			   setnice(pid,31-i);
			   printf(1, "Task-%d Pid-%d  nice %d == %d\n",i, pid,getnice(), 31-i);
			   sleep(1);
			   func( i );  	  
		}
	}
    
	setnice(getpid( ), 30);
	pid = getpid();
	for(int i = 0; i< taskCnt ;i++)
	   wait( );

    printf(1, "Parent Pid-%d is done  turnround-time %d  swtch_cnt %d\n\n", 
	       pid, gettt(), getSwtchCnt( )); 
	exit();
}