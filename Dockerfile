#
# To Build:
#       docker build -t usbvalve_tbfa/arduino-cli .
#
# To Run:
#       docker run --rm --name usbvalve_tbfa -v $PWD:/mnt usbvalve_tbfa/arduino-cli /mnt/USBvalve_tbfa
#

FROM ubuntu:22.04
WORKDIR /app

# OS setup
RUN apt-get update -y \ 
    && apt-get install -y git wget python3 \
    && apt-get autoremove -y \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# arduino-cli setup
RUN cd /app \
    && git clone --recursive https://github.com/arduino/arduino-cli.git \
    && cd arduino-cli \
    && ./install.sh \
    && export PATH=$PATH:/app/arduino-cli/bin \
    && arduino-cli --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json core search 2040 \
    && arduino-cli --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json core install rp2040:rp2040 \
    && arduino-cli lib install "Adafruit TinyUSB Library" \
    && arduino-cli lib install "Adafruit SSD1306" \
    && arduino-cli lib install "Pico PIO USB" \
    && arduino-cli lib install "XxHash_arduino" \
    && arduino-cli lib install "GFX Library for Arduino" \
    && arduino-cli lib install "ArduinoJson" \
    && arduino-cli lib install "NeoPixelConnect" \
    && arduino-cli lib uninstall "SdFat - Adafruit Fork"

# Compilation setup
RUN echo  "#!/bin/bash" > /app/entrypoint.sh \
    && echo  "export PATH=\$PATH:/app/arduino-cli/bin" >> /app/entrypoint.sh \
    && echo  "arduino-cli compile --fqbn rp2040:rp2040:rpipico --board-options \"usbstack=tinyusb\" --board-options \"freq=120\" --output-dir \"/mnt/USBvalve_tbfa_out\" \"\$1\"" >> /app/entrypoint.sh \
    && chmod +x /app/entrypoint.sh

ENTRYPOINT ["/app/entrypoint.sh"]
