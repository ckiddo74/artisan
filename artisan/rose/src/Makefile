ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
PACKAGES=/artisan/build
BUILD_DIR = $(PACKAGES)/artrose.lib
ARTROSE_LIB = $(PACKAGES)/lib/artrose.so
CPPFLAGS = -std=c++14 -fPIC -D__DEBUG__ -Wno-deprecated-declarations -MD -MP 
SRCDIR=$(ROOT_DIR)
PYTHON_VER=$(shell python3 -c 'import sys;(x,y)=sys.version_info[:2];print(str(x)+"."+str(y))')

ifeq ($(wildcard /artisan/.),)
$(error makefile must be running within artisan container)
endif

UNPARSER_SRCS = $(SRCDIR)/unparser/hUnparser.cpp \
				$(SRCDIR)/unparser/hUnparserEx.cpp \
				$(SRCDIR)/unparser/hbcpp.cpp \
				$(SRCDIR)/unparser/hbcpp/anyobj.cpp  \
				$(SRCDIR)/unparser/hbcpp/bcpp.cpp \
				$(SRCDIR)/unparser/hbcpp/hanging.cpp \
				$(SRCDIR)/unparser/hbcpp/strings.cpp \
				$(SRCDIR)/unparser/hbcpp/verbose.cpp \
				$(SRCDIR)/unparser/hbcpp/baseq.cpp \
				$(SRCDIR)/unparser/hbcpp/config.cpp \
				$(SRCDIR)/unparser/hbcpp/stacklis.cpp \
				$(SRCDIR)/unparser/hbcpp/tabs.cpp 

UTILS_SRCS = $(SRCDIR)/utils/rose_utils.cpp		

PY_FNS_SRCS = $(SRCDIR)/py_fns/py_unparse_prj.cpp \
              $(SRCDIR)/py_fns/py_instrument.cpp \
              $(SRCDIR)/py_fns/py_query.cpp \
              $(SRCDIR)/py_fns/query_grammar.cpp  \
			  $(SRCDIR)/py_fns/py_meta.cpp

ENTITY_SRCS = $(SRCDIR)/entity/Entity.cpp \
	          $(SRCDIR)/entity/EntityManager.cpp \

SRCS=$(SRCDIR)/artrose.cpp \
     $(SRCDIR)/py_sgnode.cpp \
	 $(ENTITY_SRCS) \
	 $(UNPARSER_SRCS) \
	 $(UTILS_SRCS) \
	 $(PY_FNS_SRCS)

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

ENTITIES := $(shell find $(SRCDIR)/entity/defs -maxdepth 1 -name \*.hpp)
	  
all: $(ARTROSE_LIB)

$(ARTROSE_LIB): $(OBJS) $(BUILD_DIR)/$(SRCDIR)/entity_defs.hpp $(PACKAGES)/lib/librose.a
	g++ $(OBJS) -o $@ -std=c++14 -fPIC -shared \
	        -L ./support -L $(PACKAGES)/lib -L/usr/lib/python$(PYTHON_VER)/config-$(PYTHON_VER)m-x86_64-linux-gnu\
	        -Wl,-Bstatic -lrose -lhpdf  -lboost_iostreams -lboost_filesystem -lboost_system -lboost_wave -lboost_python3 -lboost_regex -lboost_timer -lboost_chrono -lboost_thread  -lpython$(PYTHON_VER)m-pic  \
			-lquadmath -lm -lc  -static-libstdc++ -static-libgcc 

$(BUILD_DIR)/%.cpp.o: %.cpp 
	mkdir -p $(dir $@)
	g++ \
	     -I $(PACKAGES)/include \
	     -I $(PACKAGES)/include/rose \
	     -I $(SRCDIR) \
	     -I /usr/include/python$(PYTHON_VER)m \
	     -I $(BUILD_DIR)/$(SRCDIR) \
	     -c $< $(CPPFLAGS) -o $@

$(BUILD_DIR)/$(SRCDIR)/entity/EntityManager.cpp.o: $(BUILD_DIR)/$(SRCDIR)/entity_defs.hpp $(SRCDIR)/entity/EntityManager.cpp		 

$(BUILD_DIR)/$(SRCDIR)/entity_defs.hpp: $(SRCDIR)/entity/defs $(ENTITIES)
	mkdir -p $(dir $@)
	$(ROOT_DIR)/scripts/gen_entity_defs.sh > $(BUILD_DIR)/$(SRCDIR)/entity_defs.hpp

-include $(SRCS:%.cpp=$(BUILD_DIR)/%.cpp.d)

clean:
	rm -rf $(BUILD_DIR) $(ARTROSE_LIB) 



