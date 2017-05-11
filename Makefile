all:
	$(MAKE) -C winforms
	$(MAKE) -C winapi
	$(MAKE) -C fontdemo


clean:
	$(MAKE) -C winforms clean
	$(MAKE) -C winapi clean
	$(MAKE) -C fontdemo clean


.PHONY: all clean

