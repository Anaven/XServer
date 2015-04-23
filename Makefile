NAME = XServer
C++ = $(CPLUS)
CC = $(CCOMP)
LINK = $(LINKER)
CCFLAGS += $(COMPILER_FLAGS) -g -Wall -Wno-format-y2k $(INCLUDE_FLAG) PlatformHeader.h
LIBS = $(CORE_LINK_LIBS) -lUtilitiesLib

# OPTIMIZATION
#CCFLAGS += -O3

# EACH DIRECTORY WITH HEADERS MUST BE APPENDED IN THIS MANNER TO THE CCFLAGS

CCFLAGS += -I.
CCFLAGS += -IServer.tproj
CCFLAGS += -IUtilitiesLib
CCFLAGS += -ISession
CCFLAGS += -ILog

# EACH DIRECTORY WITH A STATIC LIBRARY MUST BE APPENDED IN THIS MANNER TO THE LINKOPTS

LINKOPTS = -LUtilitiesLib -LLib

C++FLAGS = $(CCFLAGS)

CFILES = 

CPPFILES =	Server.tproj/XServer.cpp \
        	Session/NetStream.cpp \
        	Log/RollingLog.cpp \
        	Log/Log.cpp \
        	Server.tproj/Main.cpp \
        	Server.tproj/SessionListenerSocket.cpp \
        	Server.tproj/ConfigPrefsParser.cpp \
        	Server.tproj/PacketPool.cpp \
        	Server.tproj/RunServer.cpp
            
# CCFLAGS += $(foreach dir,$(HDRS),-I$(dir))

LIBFILES = UtilitiesLib/libUtilitiesLib.a

all: XServer

XServer: $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)  $(LIBFILES)
	$(LINK) -o $@ $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) $(COMPILER_FLAGS) -pg $(LINKOPTS) $(LIBS) 

install: XServer

clean:
	rm -f $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)

.SUFFIXES: .cpp .c .o

.cpp.o:
	$(C++) -c -o $*.o $(DEFINES) $(C++FLAGS) $*.cpp

.c.o:
	$(CC) -c -o $*.o $(DEFINES) $(CCFLAGS) $*.c

