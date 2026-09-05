#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <esp_http_server.h>

httpd_handle_t start_webserver(void);
void stop_webserver(httpd_handle_t server);

/** Returns the last-reported phone battery level (-1 = not available). */
int  web_server_get_phone_battery(void);

#endif // WEB_SERVER_H