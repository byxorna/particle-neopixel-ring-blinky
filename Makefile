.DEFAULT_GOAL := build
FIRMWARE = firmware.bin
# NOOTE: newer targets do not seem to work well, because of some bullshit looking for WIFI
TARGET = 0.6.3
FASTLED_VERSION ?= 3.1.5
PROJECT ?= blinky

deps:
	@echo Installing deps
	yarn global add particle-cli

.PHONY: $(FIRMWARE)
$(FIRMWARE):
	particle cloud compile photon project.properties src/particle/$(PROJECT) --target $(TARGET) --saveTo $(FIRMWARE)

#.PHONY: $(FIRMWARE)
#$(FIRMWARE):
#	particle compile photon src/particle/$(PROJECT) lib/FastLED-$(FASTLED_VERSION) --target $(TARGET) --saveTo $(FIRMWARE)
##	particle compile photon src/particle/$(PROJECT) lib/FastLED-$(FASTLED_VERSION) --target $(TARGET) --saveTo $(FIRMWARE)

.PHONY: build
build: $(FIRMWARE)

.PHONY: flash
flash: build $(FIRMWARE)
	@echo Flashing firmware
	particle flash --usb $(FIRMWARE)
