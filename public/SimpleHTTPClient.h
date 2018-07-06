/*
 * SimpleHTTPClient.h
 *
 *  Created on: 2014年1月17日
 *      Author: xing
 */

#ifndef SIMPLEHTTPCLIENT_H_
#define SIMPLEHTTPCLIENT_H_
#include <string>


class SimpleHTTPClient{
public:
	SimpleHTTPClient();
	virtual ~SimpleHTTPClient();

	SimpleHTTPClient &init(const std::string host, const unsigned int port, int timeout = 10);
	int tcpconnect(void);

	int post(const std::string url, const std::string &data, bool check_err = true);
	int get(const std::string url, const std::string &query, std::string &respond, bool check_err = true);
private:
	int hostname_resolv(const char *hostname, std::string &ipaddr);
private:
	std::string hostname_;
	unsigned int  port_;
	int timeout_;

private:
	const static char *HEAD_FORMAT_ ;
	const  static unsigned int BUF_MAX_LEN_ = 2048;
};

#endif /* SIMPLEHTTPCLIENT_H_ */
