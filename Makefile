.DEFAULT_GOAL := build
FIRMWARE = firmware.bin
# NOOTE: newer targets do not seem to work well, because of some bullshit looking for WIFI
TARGET ?= 0.6.3
DEVICE ?= photon
FASTLED_VERSION ?= 3.1.5
PROJECT ?= blinky

deps:
	@echo Installing deps
	yarn global add particle-cli

.PHONY: $(FIRMWARE)
$(FIRMWARE):
	particle cloud compile $(DEVICE) project.properties src/particle/$(PROJECT) --target $(TARGET) --saveTo $(FIRMWARE)

#.PHONY: $(FIRMWARE)
#$(FIRMWARE):
#	particle compile $(DEVICE) src/particle/$(PROJECT) lib/FastLED-$(FASTLED_VERSION) --target $(TARGET) --saveTo $(FIRMWARE)
##	particle compile $(DEVICE) src/particle/$(PROJECT) lib/FastLED-$(FASTLED_VERSION) --target $(TARGET) --saveTo $(FIRMWARE)

.PHONY: build
build: $(FIRMWARE)

.PHONY: flash
flash: build $(FIRMWARE)
	@echo Flashing firmware
	particle flash --usb $(FIRMWARE)
