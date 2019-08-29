MAKEDIR=/artisan/setup
PID := $(shell cat /proc/$$$$/status | grep PPid | awk '{print $$2}')
JOBS := $(shell ps -p ${PID} -f | tail -n1 | grep -oP '\-j *\d+' | sed 's/-j//')
ifeq "${JOBS}" ""
JOBS := 1
endif

BOOST_VER=62
ROSEVER=0.9.11.30
ROSEREPO=git@github.com:rose-compiler/rose.git
#ROSEREPO=git@gitlab.doc.ic.ac.uk:jgfc/rose-compiler.git

#ROSE_EDG_URL=http://edg-binaries.rosecompiler.org
ROSE_EDG_URL=http://www.doc.ic.ac.uk/~jgfc/rose

BUILDDIR=/artisan/build
CODEDIR=$(BUILDDIR)/rose
ROSESRCDIR=$(CODEDIR)/src.$(ROSEVER)
ROSESRCDIR_PATCHED=$(CODEDIR)/src-patched.$(ROSEVER)
ROSEBUILDDIR=$(BUILDDIR)/rose/build.$(ROSEVER)
INSTALLDIR=$(BUILDDIR)
IN_ARTISAN_DEV=$(wildcard /artisan/.)

ifeq ($(wildcard /artisan/.),)
$(error makefile must be running within artisan container)
endif

$(INSTALLDIR)/lib/artrose.so: $(INSTALLDIR)/lib/librose.a 
	cd /artisan/artisan/rose/src && $(MAKE)

$(INSTALLDIR)/lib/librose.a: $(ROSEBUILDDIR)/Makefile $(INSTALLDIR)/include/edg/usr/
	cd $(ROSEBUILDDIR) && $(MAKE) && $(MAKE) install
    @echo "ROSE library is installed in: $(shell readlink -f $(INSTALLDIR))!"

$(INSTALLDIR)/include/edg/usr/: 
	rm -rf $(INSTALLDIR)/include/edg/usr.original
	mkdir -p $(INSTALLDIR)/include/edg/usr.original
	cp /usr/include $(INSTALLDIR)/include/edg/usr.original -R
	GCCHEADERS=$(shell gcc -print-search-dirs | grep install | awk '{ print $$2 }') && \
	    GCCHEADERS_TARGET=$(INSTALLDIR)/include/edg/usr.original/$${GCCHEADERS#/usr/}  && \
		mkdir -p $$GCCHEADERS_TARGET && \
	    cp $$GCCHEADERS/include $$GCCHEADERS_TARGET -R && \
		cp $$GCCHEADERS/include-fixed $$GCCHEADERS_TARGET -R
	cp $(INSTALLDIR)/include/edg/usr.original $(INSTALLDIR)/include/edg/usr -R

#	&& rm -rf $(INSTALLDIR)/include/edg && tar xzvf $(MAKEDIR)/artifacts/edg_49.tgz -C $(INSTALLDIR)/include

# the sed command allow us to remove the "/" from usr, to change the system header location
$(ROSEBUILDDIR)/Makefile: $(ROSESRCDIR_PATCHED)/configure
	cd $(MAKEDIR) && ./configure-rose $(ROSESRCDIR_PATCHED) $(ROSEBUILDDIR) $(INSTALLDIR)	
	@stdbuf -o0 echo -e "\n*** getting EDG binary (please wait)... ***\n" 
	cd $(ROSEBUILDDIR)/src/frontend/CxxFrontend && \
	   $(MAKE) show-edg-url 2> /dev/null | grep -i "^http" | xargs basename > edg-file.txt &&  \
       cat edg-file.txt | xargs -L1 -I '@' wget http://edg-binaries.rosecompiler.org/@

# prepare configuration in the source
$(ROSESRCDIR_PATCHED)/configure: $(ROSESRCDIR_PATCHED) 
	cd $(ROSESRCDIR_PATCHED) && ./build
	touch $(ROSESRCDIR_PATCHED)/configure

# patch rose if necessary
$(ROSESRCDIR_PATCHED): $(ROSESRCDIR)
	cd $(CODEDIR) && $(MAKEDIR)/../bin/artisan-patch $(ROSESRCDIR) $(MAKEDIR)/patches/rose.patch $(ROSESRCDIR_PATCHED)

# git clone ROSE source
$(ROSESRCDIR): $(INSTALLDIR)/include/boost/version.hpp
	mkdir -p $(CODEDIR)	
	rm -rf $(ROSESRCDIR)
	git clone -b 'v$(ROSEVER)' --single-branch --depth 1 $(ROSEREPO) $(ROSESRCDIR) 

# Steps to push new tags to Imperial DOC
#   git clone -b v0.9.11.30 --single-branch git@github.com:rose-compiler/rose.git  # no depth
#   git remote add doc git@gitlab.doc.ic.ac.uk:jgfc/rose-compiler.git
#   git push doc v0.9.11.30

####################### boost
$(INSTALLDIR)/include/boost/version.hpp: $(CODEDIR)/boost_1_$(BOOST_VER)_0/b2
	cd $(CODEDIR)/boost_1_$(BOOST_VER)_0/ && \
	  ./b2 --prefix=$(INSTALLDIR) cxxflags="-fPIC -Wno-unused-local-typedefs -Wno-unused-variable -Wno-strict-overflow -O3" cflags="-O3 -fPIC -Wno-unused-local-typedefs -Wno-unused-variable -Wno-strict-overflow" -sNO_BZIP2=1 link=static link=shared release install -j$(JOBS)
	touch $(INSTALLDIR)/include/boost/version.hpp

$(CODEDIR)/boost_1_$(BOOST_VER)_0/b2: $(CODEDIR)/boost_1_$(BOOST_VER)_0/bootstrap.sh
	cd $(CODEDIR)/boost_1_$(BOOST_VER)_0/ && \
 	   ./bootstrap.sh  --prefix=$(INSTALLDIR) --with-libraries="chrono,timer,date_time,filesystem,iostreams,program_options,random,regex,serialization,signals,system,thread,wave,python"  --without-icu  --with-python=/usr/bin/python3

$(CODEDIR)/boost_1_$(BOOST_VER)_0/bootstrap.sh: $(CODEDIR)/boost-1.$(BOOST_VER).0.tar.bz2
	tar xf $(CODEDIR)/boost-1.$(BOOST_VER).0.tar.bz2 -C $(CODEDIR)
	touch $(CODEDIR)/boost_1_$(BOOST_VER)_0/bootstrap.sh

$(CODEDIR)/boost-1.$(BOOST_VER).0.tar.bz2:
	mkdir -p $(CODEDIR)
	wget -O $(CODEDIR)/boost-1.$(BOOST_VER).0.tar.bz2 \
	        http://sourceforge.net/projects/boost/files/boost/1.$(BOOST_VER).0/boost_1_$(BOOST_VER)_0.tar.bz2/download

remake-artrose.lib:
	rm -rf $(INSTALLDIR)/lib/artrose.so
	cd $(MAKEDIR) && $(MAKE) 

remake-rose.lib:
	rm -rf $(INSTALLDIR)/lib/librose.a
	cd $(MAKEDIR) && $(MAKE) 

remake-artrose:
	cd /artisan/artisan/rose && $(MAKE) clean
	cd $(MAKEDIR) && $(MAKE) 

remake-rose:
	rm -rf $(INSTALLDIR)/lib/librose.a
	rm -rf $(ROSEBUILDDIR)	
	rm -rf $(ROSESRCDIR_PATCHED)
	cd $(MAKEDIR) && $(MAKE) 

clean:
	rm -rf $(BUILDDIR)

