all:
	@$(shell mkdir -p bin/)
	@$(MAKE) -f Makefile.regflood

.PHONY: clean
clean:
	@$(MAKE) -f Makefile.regflood clean

.PHONY: remove
remove: clean
	@$(MAKE) -f Makefile.regflood remove