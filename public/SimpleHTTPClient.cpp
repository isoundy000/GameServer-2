/*
 * SimpleHTTPClient.cpp
 *
 *  Created on: 2014年1月17日
 *      Author: xing
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include "Log.h"
#include "GameHeader.h"
#include "SimpleHTTPClient.h"

//#define MSG_USER printf

const char * SimpleHTTPClient::HEAD_FORMAT_ =
"%s %s HTTP/1.0\r\n" // 使用1.0版本， 避免web服务器返回 chunked 数据
"Host: %s:%d\r\n"
"User-Agent: fswr SimpleHTTPClient 0.1\r\n"
"Connection: Close\r\n"
"Accept: text/*\r\n"
"Content-Type: application/x-www-form-urlencoded; charset=UTF-8\r\n";

SimpleHTTPClient::SimpleHTTPClient():
	hostname_(""),port_(0),timeout_(0) {
	// TODO Auto-generated constructor stub
}



SimpleHTTPClient::~SimpleHTTPClient() {
	// TODO Auto-generated destructor stub
}


SimpleHTTPClient &SimpleHTTPClient::init(const std::string host, const unsigned int port, int timeout)
{
	hostname_  = host;
	port_ = port;
	timeout_ = timeout;
//	::memset(buffer, 0, BUF_MAX_LEN_);
	return *this;
}

int SimpleHTTPClient::hostname_resolv(const char *hostname, string &ipaddr)
{
	in_addr in_addr;
	// hostname 就是一个IP 地址, 直接返回
	if( (inet_aton(hostname, &in_addr)) != 0)
	{
		ipaddr = hostname;
		return 0;
	}

	struct hostent *resolv_info;
	if( (resolv_info=gethostbyname(hostname)) == NULL ||
			resolv_info->h_length <= 0)
	{
		return -1;
	}
	char buf[32] = {0};
	inet_ntop(resolv_info->h_addrtype, resolv_info->h_addr_list[0], buf, sizeof(buf));
	ipaddr = buf;
	return 0;
}

int SimpleHTTPClient::tcpconnect(void)
{
	int connfd = 0;
	int ret = 0;
	struct sockaddr_in serveraddr;

	if ((connfd = ::socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		return -1;
	}

	::memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port_);

	std::string host_ip;
	ret = hostname_resolv(hostname_.c_str(), host_ip);
	ret = ::inet_pton(AF_INET, host_ip.c_str(), &serveraddr.sin_addr);
	if ( ret <= 0) {
		MSG_USER("host error: %s", hostname_.c_str());
		::close(connfd);
		return ret;
	}

	struct timeval timeout_tv;
	timeout_tv.tv_sec = timeout_;
	timeout_tv.tv_usec = 0;
	if(::setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, &timeout_tv, sizeof(timeout_tv)) == -1)
	{
		MSG_USER("setsockopt SO_RCVTIMEO error");
		::close(connfd);
		return -1;
	}
	if(::setsockopt(connfd, SOL_SOCKET, SO_SNDTIMEO, &timeout_tv, sizeof(timeout_tv)) == -1)
	{
		MSG_USER("setsockopt SO_SNDTIMEO error");
		::close(connfd);
		return -1;
	}

	ret = ::connect(connfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if ( ret < 0) {
		MSG_USER("connect error: %s", strerror(errno));
        ::close(connfd);
		return ret;
	}

	return connfd;
}

/*
 * POST HTTP REQUEST, return HTTP STATUS CODE or 0 or -1
 * */

int SimpleHTTPClient::post(const std::string url, const std::string &post_data, const bool check_err)
{
	static char buffer[BUF_MAX_LEN_];
	::memset(buffer, 0, BUF_MAX_LEN_);
	char *header_data = buffer;
	char *header_write_ptr = header_data;
	::snprintf(header_write_ptr, BUF_MAX_LEN_, HEAD_FORMAT_,
			"POST", url.c_str(), hostname_.c_str(), port_);
	header_write_ptr += strlen(header_data);

	::snprintf(header_write_ptr, BUF_MAX_LEN_-strlen(header_data),
			"Content-Length: %d\r\n\r\n", (int)post_data.size()); //添加自定义的头部

	int connfd = tcpconnect();
	if( connfd <= 0 ){
		MSG_USER("SimpleHTTPClient::post() connect error");
		return -1;
	}
	std::string send_data = header_data + post_data;
	MSG_DEBUG("post data: %s", post_data.c_str());
	MSG_DEBUG("%s", send_data.c_str());
	if( ::write( connfd, send_data.c_str(), send_data.size()) <= 0)
	{
		MSG_USER("SimpleHTTPClient::post() write error.");
		MSG_USER("%s", send_data.c_str());
		::close(connfd);
		return -1;
	}

	if(!check_err)
	{
		::close(connfd);
		return 0;
	}

	char *recv_data = buffer;
	::memset(recv_data, 0, BUF_MAX_LEN_);
	if(::read(connfd, recv_data, BUF_MAX_LEN_-1) < 0)
	{
		MSG_USER("SimpleHTTPClient::post()  read error");
		MSG_USER("%s", send_data.c_str());
		::close(connfd);
		return -1;
	}
	::close(connfd);
	MSG_USER("php recv: %s", recv_data);

	char *body_start = strstr(recv_data, "\r\n\r\n");
	if(body_start != NULL)
	{
		body_start += 4;
		MSG_DEBUG("respond: %s", body_start);
	}

	char *httpcode_start = strstr(recv_data, " ");
	if(httpcode_start == NULL) return -1;
	httpcode_start += 1;

	char *httpcode_end = strstr(httpcode_start, " ");
	if(httpcode_end == NULL) return  -1;

	*httpcode_end = 0; // 截断
	int httpcode = ::atoi(httpcode_start);
	if(httpcode == 0) httpcode = -1;

	return httpcode;
}

int SimpleHTTPClient::get(const std::string url, const std::string &query, std::string &respond, bool check_err)
{
	static char buffer[BUF_MAX_LEN_];
	::memset(buffer, 0, BUF_MAX_LEN_);
	char *header_data = buffer;
	char *header_write_ptr = header_data;

	std::string uri;
	if(query.empty())
		uri = url;
	else
		uri = url + "?" + query;

	::snprintf(header_write_ptr, BUF_MAX_LEN_, HEAD_FORMAT_,
			"GET", uri.c_str(), hostname_.c_str(), port_);

	header_write_ptr += strlen(header_data);
	::snprintf(header_write_ptr, BUF_MAX_LEN_-strlen(header_data), "\r\n");

	MSG_DEBUG("send request:\n%s", header_data);

	int connfd = tcpconnect();
	if( connfd <= 0 ){
		MSG_USER("SimpleHTTPClient::get() connect error");
		return connfd;
	}

	if( ::write( connfd, header_data, ::strlen(header_data)) <= 0)
	{
		MSG_USER("SimpleHTTPClient::get() write error");
		::close(connfd);
		return -1;
	}

	if(!check_err)
	{
		::close(connfd);
		return 0;
	}

	char *recv_data = buffer;
	::memset(recv_data, 0, BUF_MAX_LEN_);
	if(::read(connfd, recv_data, BUF_MAX_LEN_-1) < 0)
	{
		MSG_USER("SimpleHTTPClient::get()  read error");
		::close(connfd);
		return -1;
	}
	::close(connfd);

	char *body_start = strstr(recv_data, "\r\n\r\n");
	if(body_start != NULL)
	{
		body_start += 4;
		respond = body_start;
		MSG_DEBUG("respond: %s", body_start);
	}

	char *httpcode_start = strstr(recv_data, " ");
	if(httpcode_start == NULL) return -1;
	httpcode_start += 1;

	char *httpcode_end = strstr(httpcode_start, " ");
	if(httpcode_end == NULL) return  -1;

	*httpcode_end = 0; // 截断
	int httpcode = ::atoi(httpcode_start);

	if(httpcode / 100 == 2)
		httpcode = 0;

	return httpcode;
}
////// 测试
//int main()
//{
//	SimpleHTTPClient().init("127.0.0.1", 80).post("/index.php", "a=10&b=20");
//	SimpleHTTPClient().init("127.0.0.1", 80).get("/index.php", "a=10");
//	return 0;
//}
