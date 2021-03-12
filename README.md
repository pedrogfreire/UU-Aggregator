# UU-Aggregator

Aggregator's code to be executed in the Raspberry PI

This is the code waits for a UART data input with the standard (<ID;VALUE>). Finds the ID in the lookup table, encrypt it and sends it to the IHAP device through bluetooth.

Compilation:
The main file for compilation is AggMain.c

Use GCC Compiler:
```linux
gcc AggMain.c -lcrypto -lssl -ljson-c -lm -lbluetooth -lcurl
```
------------------------------------------------------------------------------------
Before Running the Code:

The Raspberry Bluetooth needs to be paired with the IHAP device. This can be done by using the NXP Android App called TagWriter and write a Bluetooth tag with the Raspberry Pi Blueooth Unique Address ("MAC Address").

------------------------------------------------------------------------------------
Data Input:

This codes contains a few JSON files that carries both configuration information and data structures for the functioning of the aggregator. They are decribed as following:

**aggregatorInfo.json** - Contains the Aggregator's data header message. All the data send by the aggregator will have the content of this file as header.  
**config.json** - Configures the desire destination of the message. "I" is to send to IHAP through bluetooth and "C" is to send directly to Alleato using post HTTP with the CURL funtion.  
**header.json** - Contains the header of the encrypted message to be send to the IHAP. Differently from the aggregatorInfo, this header is not encrypted but it is added to all encrypted data before transmitting to the IHAP. It contains the Encryption type information.  
**sensorsLookupTable.json** - Constains the lookup table with all the information necessary of the IMDs and it's corresponding ID.  

------------------------------------------------------------------------------------
Libraries Instalation:

**Lib Bluez** - https://learn.adafruit.com/install-bluez-on-the-raspberry-pi/installation  
**OpenSSL-dev** - Sudo apt-get install libssl-dev  
**Curl** - sudo apt-get install libcurl4-gnutls-dev  
**JSON** - https://github.com/json-c/json-c  
**UART** - https://raspberry-projects.com/pi/programming-in-c/uart-serial-port/using-the-uart - sudo apt-get install minicom  


