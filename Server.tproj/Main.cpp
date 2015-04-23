#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/resource.h>

#include "revision.h"
#include "defaultPaths.h"

#include "Headers.h"
#include "OS.h"
#include "Socket.h"
#include "IdleTask.h"
#include "RunServer.h"
#include "TimeoutTask.h"
#include "ConfigPrefsParser.h"

#include <mcheck.h>

static int sSigIntCount = 0;
static int sSigTermCount = 0;
static pid_t sChildPID = 0;

void usage()
{
    const char *usage_name = PLATFORM_SERVER_BIN_NAME;

    printf("%s/%s ( Build/%s; Platform/%s; %s)\n", PLATFORM_SERVER_TEXT_NAME,
                                                                kVersionString,
                                                                kBuildString,
                                                                "linux",
                                                                kCommentString);
    printf("usage: %s [ -d | -p port | -v | -c /myconfigpath.conf | -h ]\n", usage_name);
    printf("-d: Run in the foreground\n");
    printf("-p XXX: Specify the admin port of the server\n");
    printf("-c /myconfigpath.conf: Specify a config file\n"); 
    printf("-i Interact with web server\n");
    printf("-h: Prints usage\n");
}

bool sendtochild(int sig, pid_t myPID)
{
    if (sChildPID != 0 && sChildPID != myPID) // this is the parent
    {   // Send signal to child
        ::kill(sChildPID, sig);
        return true;
    }

    return false;
}

void sigcatcher(int sig, int /*sinfo*/, struct sigcontext* /*sctxt*/)
{
    printf("Signal %d caught\n", sig);

    pid_t myPID = getpid();
    //
    // SIGHUP means we should reread our preferences
    if (sig == SIGHUP)
    {
        if (sendtochild(sig, myPID))
        {
            return;
        }
        else
        {
            // This is the child process.
            // Re-read our preferences.
            //RereadPrefsTask* task = new RereadPrefsTask;
            //task->Signal(Task::kStartEvent);
        }
    }
        
    //Try to shut down gracefully the first time, shutdown forcefully the next time
    if (sig == SIGINT) // kill the child only
    {
        if (sendtochild(sig, myPID))
        {
            return;// ok we're done 
        }
        else
        {
            //
            // Tell the server that there has been a SigInt, the main thread will start
            // the shutdown process because of this. The parent and child processes will quit.
            if (sSigIntCount == 0)
            	XServer::GetServer()->SetSigInt();
            sSigIntCount++;
        }
    }
    
	if (sig == SIGTERM || sig == SIGQUIT) // kill child then quit
    {
        if (sendtochild(sig, myPID))
        {
             return;// ok we're done 
        }
        else
        {
            // Tell the server that there has been a SigTerm, the main thread will start
            // the shutdown process because of this only the child will quit
    
            if (sSigTermCount == 0)
            	XServer::GetServer()->SetSigTerm();
            sSigTermCount++;
        }
    }

    if (sig == SIGUSR1)
    {
        printf("exit....\n");
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    struct sigaction act;

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = (void(*)(int))&sigcatcher;

    (void)::sigaction(SIGPIPE, &act, NULL);
    (void)::sigaction(SIGHUP, &act, NULL);
    //(void)::sigaction(SIGINT, &act, NULL);
    (void)::sigaction(SIGTERM, &act, NULL);
    (void)::sigaction(SIGQUIT, &act, NULL);
    (void)::sigaction(SIGALRM, &act, NULL);
    (void)::sigaction(SIGUSR1, &act, NULL);

    struct rlimit rl;
    
    // set it to the absolute maximum that the operating system allows - have to be superuser to do this
    rl.rlim_cur = RLIM_INFINITY;
    rl.rlim_max = RLIM_INFINITY;
    
    setrlimit (RLIMIT_NOFILE, &rl);

    /*
    getrlimit(RLIMIT_NOFILE,  &rl); //now get the real max value
    printf("current open file limit =%u\n", (UInt32) rl.rlim_cur);
    printf("current open file max =%u\n", (UInt32) rl.rlim_max);
    */
    
    //First thing to do is to read command-line arguments.
    int ch = 0;
    int thePort = 0;
    int theInitialState = XServer::kRunningState;

    bool dontFork = false;
    static const char* sDefaultConfigFilePath = DEFAULTPATHS_ETC_DIR "access_server.conf";

    const char* theConfigFilePath = sDefaultConfigFilePath;
    while ((ch = getopt(argc, argv, "vdp:c:h")) != EOF) // opt: means requires option arg
    {
    switch(ch)
        {
            case 'v':
                usage();
                ::exit(0);    
            case 'd':
                dontFork = true;
                break;                           
        	case 'p':
            	Assert(optarg != NULL);// this means we didn't declare getopt options correctly or there is a bug in getopt.
            	thePort = ::atoi(optarg);
            	break;
        	case 'c':
            	Assert(optarg != NULL);// this means we didn't declare getopt options correctly or there is a bug in getopt.
            	theConfigFilePath = optarg;
            	break;
        	case 'h':
            	usage();
                ::exit(0);
        	default:
            	break;
        }
    }

	if (thePort < 0 || thePort > 65535)
    { 
        ::printf("Invalid port value = %d max value = 65535\n", thePort);
        ::exit (-1);
    }

    ConfigPrefsParser theConfigPaser(theConfigFilePath);
    if (!theConfigPaser.DoesFileExist())
    {
        ::printf("Configure file %s is not exist\n", theConfigFilePath);
        ::exit(-1);
    }

	int err = theConfigPaser.Parse();
	if (err != 0)
    {
        ::printf("Could not load configuration file at %s. (%d)\n", theConfigFilePath, Thread::GetErrno());
        ::exit(-1);
    }

	if (!dontFork)
    {
    	if (daemon(0, 0) != 0)
        {
            ::printf("Failed to daemonize process. Error = %d\n", Thread::GetErrno());
            ::exit(-1);
        }
    }

    int status = 0;
    int pid = 0;
    pid_t processID = 0;

	if (!dontFork)
    {
        // loop until the server exits normally. If the server doesn't exit
        // normally, then restart it.
        // normal exit means the following the child quit 
        processID = fork();
        if (processID > 0) // this is the parent and we have a child
        {
            sChildPID = processID;
            status = 0;
            while (status == 0) //loop on wait until status is != 0;
            {    
             	pid = wait(&status);
             	int exitStatus = WEXITSTATUS(status);
            	if (WIFEXITED(status) && pid > 0 && status != 0) // child exited with status -2 restart or -1 don't restart 
                {
                	printf("child exited with status=%d\n", exitStatus);
                	if ( exitStatus == -1) // child couldn't run don't try again
                    {
                    	printf("child exited with -1 fatal error so parent is exiting too.\n");
                    	exit (EXIT_FAILURE); 
                    }
                	break;
                }
                
            	if (WIFSIGNALED(status)) // child exited on an unhandled signal (maybe a bus error or seg fault)
                {    
                	printf("child was signalled\n");
                	break; 
                }

            	if (pid == -1 && status == 0) // parent woken up by a handled signal
                   	continue;
                   
             	if (pid > 0 && status == 0)
                 {
                 	printf("child exited cleanly so parent is exiting\n");
                 	exit(EXIT_SUCCESS);                        
                }
                
            	printf("child died for unknown reasons parent is exiting\n");
            	exit (EXIT_FAILURE);
            }

        	if (processID != 0) //the parent is quitting
        	exit(EXIT_SUCCESS);  
        }
        else if (processID == 0) // must be the child
            ; // do next
        else 
        	exit(EXIT_FAILURE); // fork error
    }
    
    sChildPID = 0;
    //we have to do this again for the child process, because sigaction states
    //do not span multiple processes.
    (void)::sigaction(SIGPIPE, &act, NULL);
    (void)::sigaction(SIGHUP, &act, NULL);
    //(void)::sigaction(SIGINT, &act, NULL);
    (void)::sigaction(SIGTERM, &act, NULL);
    (void)::sigaction(SIGQUIT, &act, NULL);
    (void)::sigaction(SIGUSR1, &act, NULL);

    if (StartServer(&theConfigPaser, thePort, theInitialState, dontFork) != XServer::kFatalErrorState)
    {    
        RunServer();
        exit(EXIT_SUCCESS);
    }
    else
        exit(-1); //Cant start server don't try again

    return 0;    
}

