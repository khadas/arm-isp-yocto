
define module_func
	@$(MAKE) -C $(KERNEL_SRC) M=$(M)/$(1) modules
endef

define module_clean
	@$(MAKE) -C $(KERNEL_SRC) M=$(M)/$(1) clean
endef

define module_install
	@echo "$(MAKE) INSTALL_MOD_STRIP=1 M=$(M)/$(1) -C $(KERNEL_SRC) modules_install"
	@$(MAKE) INSTALL_MOD_STRIP=1 M=$(M)/$(1) -C $(KERNEL_SRC) modules_install
	mkdir -p ${OUT_DIR}/../vendor_lib
	cp $(OUT_DIR)/$(M)/$(1)/*.ko ${OUT_DIR}/../vendor_lib/modules/
endef

all:
	$(call module_func,dw9800w)
	$(call module_func,dw9714)

clean:
	$(call module_clean,dw9800w)
	$(call module_clean,dw9714)

modules_install:
	$(call module_install,dw9800w)
	$(call module_install,dw9714)
