include userset.mk
include $(GCCMK_PATH)paths.mk

SRC_WFS?=./webfiles
BIN_WFS?=webfs.bin
DIR_WFS?=$(BIN_DIR)
OUT_WFS?=$(DIR_WFS)/$(BIN_WFS)

webfs:
	@mkdir -p $(BIN_DIR)
	@./tools/webfs/WEBFS22.exe -h "*.htm, *.html, *.cgi, *.xml, *.bin, *.txt, *.wav" -z "mdbini.bin, *.inc, *.ini, snmp.bib, *.ovl" $(SRC_WFS) $(DIR_WFS) $(BIN_WFS)
#@$(PYTHON) ./tools/webfs/webfs_tool.py build -s $(SRC_WFS) -d *.htm,*.html,*.cgi,*.xml,*.bin,*.txt,*.wav,*.js -n *.inc,*.ini,snmp.bib,*.ovl $(OUT_WFS)
	
$(OUT_WFS):webfs 

.PHONY: flash_clear_feep
flash_clear_feep:
	@$(PYTHON) $(GCCMK_PATH)rtltool.py -p $(COM_PORT) es 0x004000 0x004000

.PHONY: flash_webfs
flash_webfs: $(OUT_WFS)
	@$(PYTHON) $(GCCMK_PATH)rtltool.py -p $(COM_PORT) wf 0x0D0000 $(OUT_WFS)

.PHONY: upload_webfs
upload_webfs: $(OUT_WFS)
	@$(PYTHON) ./tools/webfs/webfs_upload.py $(OUT_WFS) http://rtl871x1/fsupload rtl871x webfs_write
	
clean:
	@rm -rf $(BIN_DIR)	