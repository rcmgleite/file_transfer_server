/*
 * web_server_conn_utils.h
 *
 *  Created on: Jun 17, 2014
 *      Author: rafael
 */

#ifndef WEB_SERVER_CONN_UTILS_H_
#define WEB_SERVER_CONN_UTILS_H_

void open_listener(int *listener);
void bind_to_port(int _socket, int _port, int reuse);

#endif /* WEB_SERVER_CONN_UTILS_H_ */
