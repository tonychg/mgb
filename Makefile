MAKE = make
MAKEFLAGS += --no-print-directory

ALL_PROGRAMS =
ALL_PROGRAMS += mgb
ALL_PROGRAMS += tests

define run_submakefile
	@for program in $(ALL_PROGRAMS) ; do \
		$(MAKE) -C $$program  $(1) ; \
	done
endef

all:
	@$(call run_submakefile,all)

clean:
	@$(call run_submakefile,clean)

test:
	$(MAKE) -C tests test
