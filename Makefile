all:
	@$(shell mkdir -p bin/)
	@$(MAKE) -f Makefile.msgflood
	@$(MAKE) -f Makefile.addflood
	@$(MAKE) -f Makefile.regflood

.PHONY: clean
clean:
	@$(MAKE) -f Makefile.msgflood clean
	@$(MAKE) -f Makefile.addflood clean
	@$(MAKE) -f Makefile.regflood clean

.PHONY: remove
remove: clean
	@$(MAKE) -f Makefile.msgflood remove
	@$(MAKE) -f Makefile.addflood remove
	@$(MAKE) -f Makefile.regflood remove
