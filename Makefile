#The Target Binary Programs for the Backend
MASTERTARGET    := MasterServer
LOADTARGET	  := LoadBalancer
STORAGETARGET := StorageServer

#The Target Binary Programs for the Frontend
FRONTSERVERTARGET   := FrontendServer
FRONTLOADTARGET     := FrontendLoadBalancer


#The Target Binary Programs for library files
LIBTARGET 	  := libr

#The Directories, Source, Includes, Objects, Binary and Resources
BACKENDDIR  	:= src/backend
FRONTENDDIR     := src/frontend
LIBDIR			:= src/lib

BUILDDIR    := obj
TARGETDIR   := bin

SRCEXT      := cpp
OBJEXT      := o

#Flags, Libraries and Includes
CFLAGS =  -Werror -Wall -lpthread -g -o

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------

# Currently assuming each .cpp file represents an individual server
# i.e. there's no lists of compiled sources that support some server
# if this case changes let Leon know and I'll dit it, or make a list urself as follows:
# NEWSERVERSOURCES     := $(wildcard $(BACKENDDIR)/*.cpp)
# NEWSERVEROBJECTS 	:= $(BACKENDDIR:.cpp=.o)

MASTERSOURCES 		:= $(wildcard $(BACKENDDIR)/MasterServer.cpp)
MASTEROBJECTS		:= $(MASTERSOURCES:.cpp=.o)
LOADBALANCERSOURCES 	:= $(wildcard $(BACKENDDIR)/LoadBalancer.cpp)
LOADBALANCEROBJECTS		:= $(LOADBALANCERSOURCES:.cpp=.o)
STORAGESOURCES 	:= $(wildcard $(BACKENDDIR)/StorageServer.cpp)
STORAGEOBJECTS	:= $(STORAGESOURCES:.cpp=.o)

FRONTSERVERSOURCES 	:= $(wildcard $(FRONTENDDIR)/main_server.cc)
FRONTSERVEROBJECTS	:= $(FRONTSERVERSOURCES:.cpp=.o)

FRONTLOADSOURCES 	:= $(wildcard $(FRONTENDDIR)/FrontendLoadBalancer.cpp)
FRONTLOADOBJECTS	:= $(FRONTLOADSOURCES:.cpp=.o)

LIBSOURCES		:= $(wildcard $(LIBDIR)/*.cc)
LIBOBJECTS		:= $(LIBSOURCES:.cc=.o)

all: backend frontend

remake: realclean all

directories:
	@mkdir -p $(BACKENDDIR)
	@mkdir -p $(FRONTENDDIR)
	@mkdir -p $(LIBDIR)
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(TARGETDIR)

frontend: $(FRONTSERVERTARGET) $(FRONTLOADTARGET) 

$(FRONTSERVERTARGET): $(FRONTSERVEROBJECTS) $(LIBOBJECTS)
	g++ -std=c++17  $^ -lpthread -g -o $(TARGETDIR)/$(FRONTSERVERTARGET)
	@mv $(FRONTSERVEROBJECTS) $(BUILDDIR) 2>/dev/null; true

$(FRONTLOADTARGET): $(FRONTLOADOBJECTS) $(LIBOBJECTS)
	g++ -std=c++17  $^ -lpthread -g -o $(TARGETDIR)/$(FRONTLOADTARGET)
	@mv $(FRONTLOADOBJECTS) $(BUILDDIR) 2>/dev/null; true

backend: $(MASTERTARGET) $(LOADTARGET) $(STORAGETARGET)

$(MASTERTARGET): $(MASTEROBJECTS) $(LIBOBJECTS)
	g++ $^ -lpthread -g -o $(TARGETDIR)/$(MASTERTARGET)
	@mv $(MASTEROBJECTS) $(BUILDDIR) 2>/dev/null; true

$(LOADTARGET): $(LOADBALANCEROBJECTS) $(LIBOBJECTS)
	g++ $^ -lpthread -g -o $(TARGETDIR)/$(LOADTARGET)
	@mv $(LOADBALANCEROBJECTS) $(BUILDDIR) 2>/dev/null; true

$(STORAGETARGET): $(STORAGEOBJECTS) $(LIBOBJECTS)
	g++ $^ -lpthread -g -o $(TARGETDIR)/$(STORAGETARGET)
	@mv $(STORAGEOBJECTS) $(BUILDDIR) 2>/dev/null; true

pack:
	rm -f submit-project.zip
	zip -r submit-project.zip README Makefile *.c* *.h*

clean::
	rm -fv $(TARGETDIR)/$(MASTERTARGET) $(TARGETDIR)/$(LOADTARGET) $(TARGETDIR)/$(STORAGETARGET) $(TARGETDIR)/$(FRONTSERVERTARGET) $(TARGETDIR)/$(FRONTLOADTARGET) *~ $(BUILDDIR)/*.o submit-project.zip

realclean:: clean
	rm -fv submit-project.zip
