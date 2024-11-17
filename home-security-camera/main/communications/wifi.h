#ifndef WIFI_H
#define WIFI_H

void wifi_init();
void wifi_stop();
esp_err_t wifi_init_sta(char* ssid, size_t ssid_length, char* password, size_t password_length);
void wifi_init_ap();

#endif