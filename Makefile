.DEFAULT_GOAL := build
FIRMWARE = firmware.bin
# NOOTE: newer targets do not seem to work well, because of some bullshit looking for WIFI
# tested working 2021.07.22
# - 1.0.0, 1.5.2 perlin-noise-ring, works fine
# - 3.1.0 didnt work, came up green LED, no patterns :(
# - 2.1.0 also does not work
TARGET ?= 1.5.2
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
