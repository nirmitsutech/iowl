###############################################################################
#
#
#
#
#
#
#
###############################################################################
export OS=$(shell uname)
export TOPBUILDDIR=$(shell pwd)
export PLATFORM=$(shell uname -m)
export SUBDIRS :=src client
export MAKEFILE := Makefile
ifeq ($(TOPBUILDDIR)/make.rules,$(wildcard $(TOPBUILDDIR)/make.rules ))
include $(TOPBUILDDIR)/make.rules
endif
all:      default
install:  
	  cd src && make -f $(MAKEFILE) install
clean:   
	  cd src && make -f $(MAKEFILE) clean
default:
	    cd src && make -f $(MAKEFILE)
	    cd client && make -f $(MAKEFILE)
