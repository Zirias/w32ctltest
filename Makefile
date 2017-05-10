all:
	$(MAKE) -C winforms
	$(MAKE) -C winapi


clean:
	$(MAKE) -C winforms clean
	$(MAKE) -C winapi clean


.PHONY: all clean

