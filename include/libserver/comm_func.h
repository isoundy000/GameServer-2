// -*- C++ -*-
/*
 * comm_func.h
 *
 *  Created on: Apr 9, 2012
 *      Author: ChenLong
 */

#ifndef COMM_FUNC_H_
#define COMM_FUNC_H_

#include "Lib_Log.h"
#include "math.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>

inline void set_nonblock(int fd) {
	int flags = ::fcntl(fd, F_GETFL, 0);
	::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

inline int max_fd(void) {
	struct rlimit rl;
	int ret = 0;

	if ((ret = getrlimit(RLIMIT_NOFILE, &rl)) == -1)
		LOG_SYS_ABORT("getrlimit");
	else
		return rl.rlim_cur;

	return sysconf (_SC_OPEN_MAX);
}

/// 返回大于1.5 * num的第一个素数
inline size_t get_hash_table_size(unsigned int num) {
	size_t return_num = 1.5 * num, i, j;
	while (1) {
		/// 素数判断
		j = (size_t)sqrt(return_num++);
		for (i = 2; i < j; ++i) {
			if (return_num % i == 0)
				break;
		}
		if (i == j)
			return return_num;
	}
	return 0;
}

inline unsigned int ELFHash(const char *str, unsigned int len) {
/*	unsigned int hash = 0, x = 0;

	for(size_t i = 0; i < len; ++i) {
		hash = (hash << 4) + str[i];
		if((x = hash & 0xF0000000L) != 0) {
			hash ^= (x >> 24);
		}
		hash &= ~x;
	}

	return hash;*/
	//return static_cast<unsigned int>(~len);
	return static_cast<unsigned int>(~1314);
}

#endif /* COMM_FUNC_H_ */
