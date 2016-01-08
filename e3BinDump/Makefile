
include ./Makefile.inc

TARGET		:= e3BinDump.exe
TARGLIB		:= libe3Event.so 

MAINDIR		:= $(shell pwd)
INCDIR		:= $(MAINDIR)/include
SRCDIR		:= $(MAINDIR)/src
BINDIR 		:= $(MAINDIR)/bin
LIBDIR 		:= $(MAINDIR)/lib

LINKDEFS	:= e3DataBlock e3GPSData e3SORData e3GEOData e3WADData e3VWSData HPTDCEvent V1190Event e3RawHit e3Hit e3RawEvent

INCLUDES  	:= $(wildcard $(INCDIR)/*.h)                          
SOURCES     	:= $(wildcard $(SRCDIR)/*.cxx)
FSOURCES  	:= $(wildcard $(SRCDIR)/*.f)
OBJECTS		:= $(SOURCES:$(SRCDIR)/%.cxx=$(BINDIR)/%.o)
OBJECTS		:= $(OBJECTS:$(SRCDIR)/%.cpp=$(BINDIR)/%.o)
OBJECTS      	+= $(BINDIR)/e3EventCint.o 
MAINOBJ        	:= $(BINDIR)/main.o $(BINDIR)/anyoption.o

#-------------------------------------------------
# primary targets
#-------------------------------------------------

.PHONY : lib clean

exec: $(TARGET)

$(TARGET): $(MAINOBJ) $(TARGLIB)
	@echo "Creating executable file $@"
	@$(LD) $(LDFLAGS) $^ -o $@  $(ROOTLIBS)

lib: $(TARGLIB)

$(TARGLIB): $(filter-out $(MAINOBJ),$(OBJECTS))
	@echo "Creating shared library $@"
	@$(LD) $(SHARED) $(LDFLAGS) $^ -o $@ $(ROOTLIBS) $(ROOTGLIBS)   

clean:
	@echo "Cleaning up ..."
	@$(DEL) $(MAINDIR)/$(TARGET) $(MAINDIR)/$(TARGLIB) \
	$(OBJECTS) $(BINDIR)/*.d \
	$(INCDIR)/LinkDefs.h $(INCDIR)/e3EventCint.h \
	$(SRCDIR)/e3EventCint.cxx 

#-------------------------------------------------
# specific ROOT Classes rules
#-------------------------------------------------

$(BINDIR)/e3EventCint.o: $(SRCDIR)/e3EventCint.cxx
	echo "Compiling $< ..."	
	@$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(SRCDIR)/e3EventCint.cxx : $(INCDIR)/LinkDefs.h  
	@echo "Creating dictionary $@"
	@$(ROOTSYS)/bin/rootcint -f $@ -c $(CXXFLAGS) -I$(INCDIR) $(INCDIR)/e3Event.h $^
	@mv -f $(SRCDIR)/e3EventCint.h $(INCDIR)/

$(INCDIR)/LinkDefs.h : 
	@echo "Generating $@"
	@$(INCDIR)/LinkDefMaker.pl $(LINKDEFS)
	@mv -f LinkDefs.h $(INCDIR)/

#-------------------------------------------------
# generic rules
#-------------------------------------------------

$(BINDIR)/%.o : $(SRCDIR)/%.cxx; 
	@echo "Compiling $< ..." 
	@$(CXX) $(CXXFLAGS) $(ROOTCFLAGS) -I$(INCDIR) -c $< -o $@

$(BINDIR)/%.o : $(SRCDIR)/%.f; 
	@echo "Compiling $< ..." 
	@$(FC) $(FCFLAGS) -c $< -o $@

$(BINDIR)/%.d : $(SRCDIR)/%.cxx; 
	@echo "Making dependencies for file $< ..."
	@set -e;\
	$(CXX) -MM $(CXXFLAGS) $(ROOTCFLAGS) -I$(INCDIR) -w $< | \
	sed 's!$*\.o!$(BINDIR)& $@!' >$@;\
	[ -s $@ ] || rm -f $@		

ifneq ($(MAKECMDGOALS),clean)
-include $(SOURCES:$(SRCDIR)/%.cxx=$(BINDIR)/%.d)
endif


