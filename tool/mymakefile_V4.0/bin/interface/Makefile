all:
	make -C api/operate/
	make -C api
	make -C module
	make -C app
ifeq ($(SUPPORT_BUILD_SAMPLE), y)
	make -C api/operate/sample
	make -C sample 
endif

app:
	make -C app clean
	make -C app

sample:
	make -C sample clean
	make -C sample 

module:
	make -C module clean
	make -C module

api:
	make -C api clean
	make -C api

clean:
	make -C module clean
	make -C app clean

cleanall:
#	make -C api/operate	clean
	make -C api	clean
	make -C module	clean
	make -C app	clean
ifeq ($(SUPPORT_BUILD_SAMPLE), y)
	make -C api/operate/sample clean
	make -C sample	clean
endif

cleanallall:
	make -C api/operate	cleanall
	make -C api	cleanall
	make -C module	cleanall
	make -C app	cleanall
ifeq ($(SUPPORT_BUILD_SAMPLE), y)
	make -C api/operate/sample cleanall
	make -C sample	cleanall
endif
	@rm $(PROJECT_PATH)/out/* -rf
	
copy:
	make -C api	copy
	make -C module	copy
	make -C app	copy
ifeq ($(SUPPORT_BUILD_SAMPLE), y)
	make -C api/operate/sample copy
	make -C sample 	copy
endif
