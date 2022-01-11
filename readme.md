
#What is OTA programming in ESP32?

The OTA programming allows updating/uploading a new program to ESP32 using Wi-Fi instead of requiring the user to connect the ESP32 to a computer via USB to perform the update.

OTA functionality is extremely useful in case of no physical access to the ESP module. It helps reduce the amount of time spent for updating each ESP module at the time of maintenance.
 
One important feature of OTA is that one central location can send an update to multiple ESPs sharing same network.

The only disadvantage is that you have to add an extra code for OTA with every sketch you upload, so that you’re able to use OTA in the next update.
