PACKAGE := $(shell basename $(CURDIR))
CPU_TARGET := arm1176jzf-s
export JULIA_PKG_OFFLINE = true
export JULIA_DEPOT_PATH = $(CURDIR)/../jl_depot

all: README.md README.pdf

.PHONY: README.md
README.md:
	julia --project -e \
		"using $(PACKAGE); println($(PACKAGE).readme())" > $@

README.pdf: README.md
	pandoc $< --pdf-engine=xelatex \
	          -V geometry:margin=20mm \
	          -V 'monofont:FiraCode-Regular.ttf' \
	          -o $@

JL := julia16 -C $(CPU_TARGET) --project
#JL := julia --project

jl:
	$(JL) -e "using $(PACKAGE)" -i

jlrun:
	$(JL) -e "using $(PACKAGE); $(PACKAGE).main()" -i

jltrace:
	$(JL) --trace-compile=precompile_new.jl \
             -e "using $(PACKAGE); $(PACKAGE).main()" -i

#jlenv: export JULIA_PKG_OFFLINE = false
jlenv:
	$(JL)

ssh:
	ssh -t -i ../../../docker-raspbian/id_pi.pem pi@xrthandcontrolprogrammer.local

starttmux:
	tmux new-session -d -s $(PACKAGE) make run_sysimage


ARCH := $(shell uname -m)

sysimage:
	$(JL) create_sysimage.jl $(PACKAGE) $(CPU_TARGET) $(ARCH)

run_sysimage:
	sudo $(JL) -J $(PACKAGE).$(ARCH).sysimage \
	           -e "using $(PACKAGE); $(PACKAGE).main()"

install:
	sudo mkdir -p /usr/local/$(PACKAGE)
	sudo chown pi:pi /usr/local/$(PACKAGE)
	cp *.elf *.md5 \
	   Project.toml Makefile \
	   $(PACKAGE).$(ARCH).sysimage \
	   /usr/local/$(PACKAGE)
