TITLE := PAY-LAUNCH
VERSION := 01.00
TITLE_ID := PAYL00001
CONTENT_ID := IV0000-PAYL00001_00-PAYLAUNCHAPP0001
TOOLCHAIN := $(OO_PS4_TOOLCHAIN)
PKGDIR := $(TOOLCHAIN)/bin/linux
INTDIR := build
LIBS := -lc -lkernel -lc++ -lSceUserService -lSceVideoOut -lScePad -lSceSysmodule -lSceFreeType -lSDL2
CXX := clang++
LD := ld.lld
CXXFLAGS := --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -O2 -c -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include -isystem $(TOOLCHAIN)/include/c++/v1 -I$(TOOLCHAIN)/include/SDL2 -I$(TOOLCHAIN)/include/freetype2
LDFLAGS := -m elf_x86_64 -pie --script $(TOOLCHAIN)/link.x --eh-frame-hdr -L$(TOOLCHAIN)/lib $(LIBS) $(TOOLCHAIN)/lib/crt1.o
ASSETS := $(shell find assets -type f 2>/dev/null | LC_ALL=C sort)
LIBMODULES := $(wildcard sce_module/*)

all: $(CONTENT_ID).pkg

$(INTDIR):
	mkdir -p $(INTDIR)

$(INTDIR)/main.o: src/main.cpp | $(INTDIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

eboot.bin: $(INTDIR)/main.o
	$(LD) $< -o $(INTDIR)/paylaunch.elf $(LDFLAGS)
	$(PKGDIR)/create-fself -in=$(INTDIR)/paylaunch.elf -out=$(INTDIR)/paylaunch.oelf --eboot "eboot.bin" --paid 0x3800000000000011

sce_sys/param.sfo: Makefile
	$(PKGDIR)/PkgTool.Core sfo_new $@
	$(PKGDIR)/PkgTool.Core sfo_setentry $@ APP_TYPE --type Integer --maxsize 4 --value 1
	$(PKGDIR)/PkgTool.Core sfo_setentry $@ APP_VER --type Utf8 --maxsize 8 --value '$(VERSION)'
	$(PKGDIR)/PkgTool.Core sfo_setentry $@ ATTRIBUTE --type Integer --maxsize 4 --value 0
	$(PKGDIR)/PkgTool.Core sfo_setentry $@ CATEGORY --type Utf8 --maxsize 4 --value 'gd'
	$(PKGDIR)/PkgTool.Core sfo_setentry $@ CONTENT_ID --type Utf8 --maxsize 48 --value '$(CONTENT_ID)'
	$(PKGDIR)/PkgTool.Core sfo_setentry $@ DOWNLOAD_DATA_SIZE --type Integer --maxsize 4 --value 0
	$(PKGDIR)/PkgTool.Core sfo_setentry $@ SYSTEM_VER --type Integer --maxsize 4 --value 0
	$(PKGDIR)/PkgTool.Core sfo_setentry $@ TITLE --type Utf8 --maxsize 128 --value '$(TITLE)'
	$(PKGDIR)/PkgTool.Core sfo_setentry $@ TITLE_ID --type Utf8 --maxsize 12 --value '$(TITLE_ID)'
	$(PKGDIR)/PkgTool.Core sfo_setentry $@ VERSION --type Utf8 --maxsize 8 --value '$(VERSION)'

pkg.gp4: eboot.bin sce_sys/param.sfo sce_sys/icon0.png $(LIBMODULES) $(ASSETS)
	$(PKGDIR)/create-gp4 -out $@ --content-id=$(CONTENT_ID) --files "$^"

$(CONTENT_ID).pkg: pkg.gp4
	$(PKGDIR)/PkgTool.Core pkg_build $< .

clean:
	rm -rf $(INTDIR) eboot.bin pkg.gp4 sce_sys/param.sfo *.pkg

.PHONY: all clean
