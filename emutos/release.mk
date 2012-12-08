#
# Makefile - Makefile fragment for building releases
#
# Copyright (c) 2011 EmuTOS development team.
#
# This file is distributed under the GPL, version 2 or at your
# option any later version.  See doc/license.txt for details.
#

#
# This file contains the targets used to build the release archives.
# It is included from the main Makefile.
#

# The version used in the archives names is deducted from changelog.txt
EXTRACT_VERSION = sed -f tools/version.sed doc/changelog.txt \
  |grep version \
  |sed 's/[^"]*"(CVS \([0-9]*\)-\([0-9]*\)-\([0-9]*\).*/CVS-\1\2\3/' \
  |sed 's/[^"]*"\([^"]*\).*/\1/'
VERSION = $(shell $(EXTRACT_VERSION))

# Check the current release version. For test purposes.
.PHONY: release-version
NODEP += release-version
release-version:
	@echo '$(VERSION)'

# This subset of the doc directory will be included in all the binary archives
DOCFILES = doc/announce.txt doc/authors.txt doc/bugs.txt doc/changelog.txt \
  doc/license.txt doc/status.txt doc/todo.txt doc/xhdi.txt

# The archives will be placed into this directory
RELEASE_DIR = release-archives

.PHONY: release-clean
NODEP += release-clean
release-clean:
	rm -rf $(RELEASE_DIR)

.PHONY: release-mkdir
NODEP += release-mkdir
release-mkdir:
	mkdir $(RELEASE_DIR)

.PHONY: release-src
NODEP += release-src
RELEASE_SRC = emutos-src-$(VERSION)
release-src:
	mkdir $(RELEASE_DIR)/$(RELEASE_SRC)
	cp -R $(filter-out . .. $(RELEASE_DIR), $(shell echo * .*)) $(RELEASE_DIR)/$(RELEASE_SRC)
	find $(RELEASE_DIR)/$(RELEASE_SRC) -type d -name CVS |xargs rm -r
	find $(RELEASE_DIR)/$(RELEASE_SRC) -type d -exec chmod 755 '{}' ';'
	find $(RELEASE_DIR)/$(RELEASE_SRC) -type f -exec chmod 644 '{}' ';'
	find $(RELEASE_DIR)/$(RELEASE_SRC) -type f -name '*.sh' -exec chmod 755 '{}' ';'
	tar -C $(RELEASE_DIR) --owner=0 --group=0 -zcvf $(RELEASE_DIR)/$(RELEASE_SRC).tar.gz $(RELEASE_SRC)
	rm -r $(RELEASE_DIR)/$(RELEASE_SRC)

.PHONY: release-512k
NODEP += release-512k
RELEASE_512K = emutos-512k-$(VERSION)
release-512k:
	$(MAKE) clean
	$(MAKE) 512
	mkdir $(RELEASE_DIR)/$(RELEASE_512K)
	cp etos512k.img etos512k.sym $(RELEASE_DIR)/$(RELEASE_512K)
	cat doc/readme-512k.txt readme.txt >$(RELEASE_DIR)/$(RELEASE_512K)/readme.txt
	mkdir $(RELEASE_DIR)/$(RELEASE_512K)/doc
	cp $(DOCFILES) $(RELEASE_DIR)/$(RELEASE_512K)/doc
	find $(RELEASE_DIR)/$(RELEASE_512K) -name '*.txt' -exec unix2dos '{}' ';'
	cd $(RELEASE_DIR) && zip -9 -r $(RELEASE_512K).zip $(RELEASE_512K)
	rm -r $(RELEASE_DIR)/$(RELEASE_512K)

.PHONY: release-256k
NODEP += release-256k
RELEASE_256K = emutos-256k-$(VERSION)
release-256k:
	$(MAKE) clean
	$(MAKE) all256
	mkdir $(RELEASE_DIR)/$(RELEASE_256K)
	cp etos256*.img $(RELEASE_DIR)/$(RELEASE_256K)
	cat doc/readme-256k.txt readme.txt >$(RELEASE_DIR)/$(RELEASE_256K)/readme.txt
	mkdir $(RELEASE_DIR)/$(RELEASE_256K)/doc
	cp $(DOCFILES) $(RELEASE_DIR)/$(RELEASE_256K)/doc
	find $(RELEASE_DIR)/$(RELEASE_256K) -name '*.txt' -exec unix2dos '{}' ';'
	cd $(RELEASE_DIR) && zip -9 -r $(RELEASE_256K).zip $(RELEASE_256K)
	rm -r $(RELEASE_DIR)/$(RELEASE_256K)

.PHONY: release-192k
NODEP += release-192k
RELEASE_192K = emutos-192k-$(VERSION)
release-192k:
	$(MAKE) clean
	$(MAKE) all192
	mkdir $(RELEASE_DIR)/$(RELEASE_192K)
	cp etos192*.img $(RELEASE_DIR)/$(RELEASE_192K)
	cat doc/readme-192k.txt readme.txt >$(RELEASE_DIR)/$(RELEASE_192K)/readme.txt
	mkdir $(RELEASE_DIR)/$(RELEASE_192K)/doc
	find $(RELEASE_DIR)/$(RELEASE_192K) -name '*.txt' -exec unix2dos '{}' ';'
	cp $(DOCFILES) $(RELEASE_DIR)/$(RELEASE_192K)/doc
	find $(RELEASE_DIR)/$(RELEASE_192K) -name '*.txt' -exec unix2dos '{}' ';'
	cd $(RELEASE_DIR) && zip -9 -r $(RELEASE_192K).zip $(RELEASE_192K)
	rm -r $(RELEASE_DIR)/$(RELEASE_192K)

.PHONY: release-cartridge
NODEP += release-cartridge
RELEASE_CARTRIDGE = emutos-cartridge-$(VERSION)
release-cartridge:
	$(MAKE) clean
	$(MAKE) cart
	mkdir $(RELEASE_DIR)/$(RELEASE_CARTRIDGE)
	cp $(ROM_CARTRIDGE) $(RELEASE_DIR)/$(RELEASE_CARTRIDGE)
	cp emutos.stc $(RELEASE_DIR)/$(RELEASE_CARTRIDGE)
	cat doc/readme-cartridge.txt readme.txt >$(RELEASE_DIR)/$(RELEASE_CARTRIDGE)/readme.txt
	mkdir $(RELEASE_DIR)/$(RELEASE_CARTRIDGE)/doc
	cp $(DOCFILES) $(RELEASE_DIR)/$(RELEASE_CARTRIDGE)/doc
	find $(RELEASE_DIR)/$(RELEASE_CARTRIDGE) -name '*.txt' -exec unix2dos '{}' ';'
	cd $(RELEASE_DIR) && zip -9 -r $(RELEASE_CARTRIDGE).zip $(RELEASE_CARTRIDGE)
	rm -r $(RELEASE_DIR)/$(RELEASE_CARTRIDGE)

.PHONY: release-aranym
NODEP += release-aranym
RELEASE_ARANYM = emutos-aranym-$(VERSION)
release-aranym:
	$(MAKE) clean
	$(MAKE) aranym
	mkdir $(RELEASE_DIR)/$(RELEASE_ARANYM)
	cp $(ROM_ARANYM) $(RELEASE_DIR)/$(RELEASE_ARANYM)
	cat doc/readme-aranym.txt readme.txt >$(RELEASE_DIR)/$(RELEASE_ARANYM)/readme.txt
	mkdir $(RELEASE_DIR)/$(RELEASE_ARANYM)/doc
	cp $(DOCFILES) $(RELEASE_DIR)/$(RELEASE_ARANYM)/doc
	find $(RELEASE_DIR)/$(RELEASE_ARANYM) -name '*.txt' -exec unix2dos '{}' ';'
	cd $(RELEASE_DIR) && zip -9 -r $(RELEASE_ARANYM).zip $(RELEASE_ARANYM)
	rm -r $(RELEASE_DIR)/$(RELEASE_ARANYM)

.PHONY: release-firebee
NODEP += release-firebee
RELEASE_FIREBEE = emutos-firebee-$(VERSION)
release-firebee:
	$(MAKE) clean
	$(MAKE) firebee
	mkdir $(RELEASE_DIR)/$(RELEASE_FIREBEE)
	cp $(SREC_FIREBEE) $(RELEASE_DIR)/$(RELEASE_FIREBEE)
	cat doc/readme-firebee.txt readme.txt >$(RELEASE_DIR)/$(RELEASE_FIREBEE)/readme.txt
	mkdir $(RELEASE_DIR)/$(RELEASE_FIREBEE)/doc
	cp $(DOCFILES) $(RELEASE_DIR)/$(RELEASE_FIREBEE)/doc
	find $(RELEASE_DIR)/$(RELEASE_FIREBEE) -name '*.txt' -exec unix2dos '{}' ';'
	cd $(RELEASE_DIR) && zip -9 -r $(RELEASE_FIREBEE).zip $(RELEASE_FIREBEE)
	rm -r $(RELEASE_DIR)/$(RELEASE_FIREBEE)

.PHONY: release-amiga
NODEP += release-amiga
RELEASE_AMIGA = emutos-amiga-$(VERSION)
release-amiga:
	$(MAKE) clean
	$(MAKE) amiga-kickdisk
	mkdir $(RELEASE_DIR)/$(RELEASE_AMIGA)
	cp $(ROM_AMIGA) $(RELEASE_DIR)/$(RELEASE_AMIGA)
	cp $(AMIGA_KICKDISK) $(RELEASE_DIR)/$(RELEASE_AMIGA)
	cat doc/readme-amiga.txt readme.txt >$(RELEASE_DIR)/$(RELEASE_AMIGA)/readme.txt
	mkdir $(RELEASE_DIR)/$(RELEASE_AMIGA)/doc
	cp $(DOCFILES) $(RELEASE_DIR)/$(RELEASE_AMIGA)/doc
	find $(RELEASE_DIR)/$(RELEASE_AMIGA) -name '*.txt' -exec unix2dos '{}' ';'
	cd $(RELEASE_DIR) && zip -9 -r $(RELEASE_AMIGA).zip $(RELEASE_AMIGA)
	rm -r $(RELEASE_DIR)/$(RELEASE_AMIGA)

.PHONY: release-ram
NODEP += release-ram
RELEASE_RAM = emutos-ram-$(VERSION)
release-ram:
	$(MAKE) clean
	$(MAKE) ram
	mkdir $(RELEASE_DIR)/$(RELEASE_RAM)
	cp boot.prg ramtos.img $(RELEASE_DIR)/$(RELEASE_RAM)
	cat doc/readme-ram.txt readme.txt >$(RELEASE_DIR)/$(RELEASE_RAM)/readme.txt
	mkdir $(RELEASE_DIR)/$(RELEASE_RAM)/doc
	cp $(DOCFILES) $(RELEASE_DIR)/$(RELEASE_RAM)/doc
	find $(RELEASE_DIR)/$(RELEASE_RAM) -name '*.txt' -exec unix2dos '{}' ';'
	cd $(RELEASE_DIR) && zip -9 -r $(RELEASE_RAM).zip $(RELEASE_RAM)
	rm -r $(RELEASE_DIR)/$(RELEASE_RAM)

.PHONY: release-floppy
NODEP += release-floppy
RELEASE_FLOPPY = emutos-floppy-$(VERSION)
release-floppy:
	$(MAKE) clean
	$(MAKE) flop UNIQUE=us
	mkdir $(RELEASE_DIR)/$(RELEASE_FLOPPY)
	cp emutos.st $(RELEASE_DIR)/$(RELEASE_FLOPPY)
	cat doc/readme-floppy.txt readme.txt >$(RELEASE_DIR)/$(RELEASE_FLOPPY)/readme.txt
	mkdir $(RELEASE_DIR)/$(RELEASE_FLOPPY)/doc
	cp $(DOCFILES) $(RELEASE_DIR)/$(RELEASE_FLOPPY)/doc
	find $(RELEASE_DIR)/$(RELEASE_FLOPPY) -name '*.txt' -exec unix2dos '{}' ';'
	cd $(RELEASE_DIR) && zip -9 -r $(RELEASE_FLOPPY).zip $(RELEASE_FLOPPY)
	rm -r $(RELEASE_DIR)/$(RELEASE_FLOPPY)

# Main goal to build a full release distribution
.PHONY: release
NODEP += release
release: distclean release-clean release-mkdir \
  release-src release-512k release-256k release-192k release-cartridge \
  release-aranym release-firebee release-amiga release-ram release-floppy
	$(MAKE) clean
	@echo '# Packages successfully generated inside $(RELEASE_DIR)'
