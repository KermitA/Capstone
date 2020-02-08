deps_config := \
	/c/esp/esp-mdf/esp-idf/components/app_trace/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/aws_iot/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/bt/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/driver/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/esp32/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/esp_adc_cal/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/esp_event/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/esp_http_client/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/esp_http_server/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/ethernet/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/fatfs/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/freemodbus/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/freertos/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/heap/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/libsodium/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/log/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/lwip/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/mbedtls/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/mdns/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/mqtt/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/nvs_flash/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/openssl/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/pthread/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/spi_flash/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/spiffs/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/tcpip_adapter/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/vfs/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/wear_levelling/Kconfig \
	/c/esp/esp-mdf/esp-idf/components/bootloader/Kconfig.projbuild \
	/c/esp/esp-mdf/esp-idf/components/esptool_py/Kconfig.projbuild \
	/c/esp/esp-mdf/esp-idf/components/partition_table/Kconfig.projbuild \
	/c/esp/esp-mdf/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
