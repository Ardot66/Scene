SHELL = cmd

TEMP = $(CURDIR)/Temp
BIN = $(CURDIR)/Bin
SOURCE = Source
TESTS = Tests
LIB = Libraries

NAME = Scene
TESTS_NAME = Tests

DLL := $(BIN)/lib$(NAME).dll
TESTS_EXE := $(BIN)/$(TESTS_NAME).exe

COLLECTIONS = Collections
COMPONENT_OBJECTS = ComponentObjects

HEADERS := $(abspath $(wildcard */*.h Libraries/*/*/*.h))
INCLUDE := $(dir $(addprefix -I,$(HEADERS)))

export BIN
export TEMP
export HEADERS
export INCLUDE

All: Libs $(DLL) $(TESTS_EXE)
	$(TESTS_EXE)

Libs:
	$(foreach MAKEFILE, $(wildcard $(LIB)/*), "$(MAKE)" --no-print-directory -C $(MAKEFILE) &) echo Compiled sub-makefiles

$(DLL): $(HEADERS) $(SOURCE)/$(NAME).c
	gcc -fPIC -c $(INCLUDE) $(SOURCE)/$(NAME).c -o $(TEMP)/$(NAME).o
	gcc -s -shared $(TEMP)/$(NAME).o -L$(BIN) -l$(COLLECTIONS) -l$(COMPONENT_OBJECTS) -lpthread -o $(DLL)

$(TESTS_EXE): $(HEADERS) $(DLL) $(TESTS)/$(TESTS_NAME).c
	gcc -c $(INCLUDE) $(TESTS)/$(TESTS_NAME).c -o $(TEMP)/$(TESTS_NAME).o
	gcc -s $(TEMP)/$(TESTS_NAME).o -L$(BIN) -l$(NAME) -l$(COLLECTIONS) -l$(COMPONENT_OBJECTS) -lpthread -o $(TESTS_EXE)

Clean:
	DEL /Q "$(BIN)\*.dll" "$(BIN)\*.exe"
	DEL /Q "$(TEMP)\*.o"