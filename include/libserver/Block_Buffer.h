// -*- C++ -*-
/*
 * Block_Buffer.h
 *
 *  Created on: Mar 23, 2012
 *      Author: ChenLong
 *
 *
 * +------------+----------------+-----------------+
 * | head_bytes | readable bytes | writeable bytes |
 * |            |   (CONTENT)    |                 |
 * +------------+----------------+-----------------+
 * |            |                |                 |
 * 0  read_index(init_offset)  write_index     vector::size()
 *
 */

#ifndef BLOCK_BUFFER_H_
#define BLOCK_BUFFER_H_

#include "Lib_Log.h"
#include <stdint.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <endian.h>
#include <cstdio>
#include <cstring>
#include <algorithm>

#define BLOCK_LITTLE_ENDIAN
//#define BLOCK_BIG_ENDIAN

class Block_Buffer {
public:
	Block_Buffer(unsigned short max_use_times = 1, size_t init_size = 2048, size_t init_offset = 4 * sizeof(uint32_t));

	inline void reset(void);

	inline void swap(Block_Buffer &block);

	/// 当前缓冲内可读字节数
	inline size_t readable_bytes(void) const;

	/// 当前缓冲内可写字节数
	inline size_t writable_bytes(void) const;

	inline char * get_read_ptr(void);
	inline char * get_write_ptr(void);
	inline size_t get_buffer_size(void);

	inline int get_read_idx(void);
	inline void set_read_idx(int ridx);
	inline int get_write_idx(void);
	inline void set_write_idx(int widx);

	inline void copy(Block_Buffer *block);
	inline void copy(std::string const &str);
	inline void copy(char const *data, size_t len);
	inline void copy(void const *data, size_t len);

	inline void ensure_writable_bytes(size_t len);

	inline char *begin_write_ptr(void);
	inline const char *begin_write() const;

	inline void dump(void);
	inline void dump_inner(void);
	inline void debug(void);

	inline int peek_int8(int8_t &v);
	inline int peek_int16(int16_t &v);
	inline int peek_int32(int32_t &v);
	inline int peek_int64(int64_t &v);
	inline int peek_uint8(uint8_t &v);
	inline int peek_uint16(uint16_t &v);
	inline int peek_uint32(uint32_t &v);

	inline int read(char *data, size_t len);
	inline int read_int8(int8_t &v);
	inline int read_int16(int16_t &v);
	inline int read_int32(int32_t &v);
	inline int read_int64(int64_t &v);
	inline int read_uint8(uint8_t &v);
	inline int read_uint16(uint16_t &v);
	inline int read_uint32(uint32_t &v);
	inline int read_uint64(uint64_t &v);

	inline int read_int16_big_endian(int16_t &v);
    inline int read_int32_big_endian(int32_t &v);

	inline int write_int8(const int8_t v);
	inline int write_int16(const int16_t v);
	inline int write_int32(const int32_t v);
	inline int write_int64(const int64_t v);
	inline int write_uint8(const uint8_t v);
	inline int write_uint16(const uint16_t v);
	inline int write_uint32(const uint32_t v);
	inline int write_uint64(const uint64_t);
	inline int write_int64_var_big_endian(const int64_t v);

	inline int peek_double(double &v);
	inline int read_double(double &v);
	inline int write_double(const double v);

	inline int peek_string(std::string &str);
	inline int read_string(std::string &str);
	inline int write_string(const std::string &str);

	inline Block_Buffer &operator>>(int8_t &v);
	inline Block_Buffer &operator>>(int16_t &v);
	inline Block_Buffer &operator>>(int32_t &v);
	inline Block_Buffer &operator>>(uint8_t &v);
	inline Block_Buffer &operator>>(uint16_t &v);
	inline Block_Buffer &operator>>(uint32_t &v);

	inline Block_Buffer &operator<<(const int8_t v);
	inline Block_Buffer &operator<<(const int16_t v);
	inline Block_Buffer &operator<<(const int32_t v);
	inline Block_Buffer &operator<<(const uint8_t v);
	inline Block_Buffer &operator<<(const uint16_t v);
	inline Block_Buffer &operator<<(const uint32_t v);

	inline Block_Buffer &operator>>(double &v);
	inline Block_Buffer &operator<<(const double v);

	inline Block_Buffer &operator>>(std::string &v);
	inline Block_Buffer &operator<<(const std::string &v);

	inline void make_message(int msg_id, int status = 0);
	inline void finish_message(void);

	int move_data(size_t dest, size_t begin, size_t end);

	int insert_head(Block_Buffer *buf);

	size_t capacity(void);

	void recycle_space(void);

	bool is_legal(void);

	bool verify_read(size_t s);

private:
	inline char *begin(void);
	inline const char *begin(void) const;
	inline void make_space(size_t len);

private:
	unsigned short max_use_times_;
	unsigned short use_times_;
	size_t init_size_;
	size_t init_offset_;
	size_t read_index_, write_index_;
	std::vector<char> buffer_;
};

////////////////////////////////////////////////////////////////////////////////

#define peek8(x)	\
	memcpy(&x, &(buffer_[read_index_]), sizeof(uint8_t));

#define peek16(x)	\
	uint16_t t;		\
	memcpy(&t, &(buffer_[read_index_]), sizeof(t));	\
	x = be16toh(t);

#define peek32(x)	\
	uint32_t t;		\
	memcpy(&t, &(buffer_[read_index_]), sizeof(t));	\
	x = be32toh(t);

#define peek64(x)	\
	uint64_t t;		\
	memcpy(&t, &(buffer_[read_index_]), sizeof(t));	\
	x = be64toh(t);

#define read8(x)	\
	memcpy(&x, &(buffer_[read_index_]), sizeof(uint8_t));	\
	read_index_ += sizeof(uint8_t);

#define read16(x)	\
	uint16_t t;		\
	memcpy(&t, &(buffer_[read_index_]), sizeof(t));	\
	x = be16toh(t);	\
	read_index_ += sizeof(uint16_t);

#define read32(x)	\
	uint32_t t;		\
	memcpy(&t, &(buffer_[read_index_]), sizeof(t));	\
	x = be32toh(t);	\
	read_index_ += sizeof(uint32_t);

#define read64(x)	\
	uint64_t t;		\
	memcpy(&t, &(buffer_[read_index_]), sizeof(t));	\
	x = be64toh(t);	\
	read_index_ += sizeof(uint64_t);

#define write8(x)	\
	copy(&x, sizeof(uint8_t));

#define write16(x)	\
	uint16_t t;		\
	t = htobe16(x);	\
	copy(&t, sizeof(uint16_t));

#define write32(x)	\
	uint32_t t;		\
	t = htobe32(x);	\
	copy(&t, sizeof(uint32_t));

#define write64(x)	\
	uint64_t t;		\
	t = htobe64(x);	\
	copy(&t, sizeof(uint64_t));

#define swap_double(d)						\
	({ double __v, __x = (d);				\
		__asm__ ("bswap %q0" : "=r" (__v) : "0" (__x));	\
		__v; })

#define htobe_double(d)	swap_double(d)
#define betoh_double(d) swap_double(d)

void Block_Buffer::reset(void) {
	++use_times_;
	recycle_space();

	ensure_writable_bytes(init_offset_);
	read_index_ = write_index_ = init_offset_;
}

void Block_Buffer::swap(Block_Buffer &block) {
	std::swap(this->max_use_times_, block.max_use_times_);
	std::swap(this->use_times_, block.use_times_);
	std::swap(this->init_size_, block.init_size_);
	std::swap(this->init_offset_, block.init_offset_);
	std::swap(this->read_index_, block.read_index_);
	std::swap(this->write_index_, block.write_index_);
	buffer_.swap(block.buffer_);
}

size_t Block_Buffer::readable_bytes(void) const {
	return write_index_ - read_index_;
}

size_t Block_Buffer::writable_bytes(void) const {
	if (buffer_.size() < write_index_)
		return 0;
	return buffer_.size() - write_index_;
}

char *Block_Buffer::get_read_ptr(void) {
	return begin() + read_index_;
}

char *Block_Buffer::get_write_ptr(void) {
	return begin() + write_index_;
}

size_t Block_Buffer::get_buffer_size(void) {
	return buffer_.size();
}

int Block_Buffer::get_read_idx(void) {
	return read_index_;
}

void Block_Buffer::set_read_idx(int ridx) {
	if ((size_t)ridx > buffer_.size() || (size_t)ridx > write_index_) {
		LOG_ABORT("set_read_idx error ridx = %d.", ridx);
		debug();
	}

	read_index_ = ridx;
}

int Block_Buffer::get_write_idx(void) {
	return write_index_;
}

void Block_Buffer::set_write_idx(int widx) {
	if ((size_t)widx > buffer_.size() || (size_t)widx < read_index_) {
		LOG_ABORT("set_write_idx error widx = %d.", widx);
		debug();
	}

	write_index_ = widx;
}

void Block_Buffer::ensure_writable_bytes(size_t len) {
	if (writable_bytes() < len) {
		make_space(len);
	}
}

void Block_Buffer::make_space(size_t len) {
//	int cond_pos = read_index_ - init_offset_;
//	size_t read_begin, head_size;
//	if (cond_pos < 0) {
//		read_begin = init_offset_ = read_index_;
//		head_size = 0;
//		LOG_USER_TRACE("read_index_ = %u, init_offset_ = %u", read_index_, init_offset_);
//	}
//	else {
//		read_begin = init_offset_;
//		head_size = cond_pos;
//	}
//
//	if (writable_bytes() + head_size < len) {
//		buffer_.resize(write_index_ + len);
//	} else {
//		/// 把数据移到头部，为写腾出空间
//		size_t readable = readable_bytes();
//		std::copy(begin() + read_index_, begin() + write_index_, begin() + read_begin);
//		read_index_ = read_begin;
//		write_index_ = read_index_ + readable;
//	}
	buffer_.resize(write_index_ + len);
}

char *Block_Buffer::begin_write_ptr(void) {
	return begin() + write_index_;
}

const char *Block_Buffer::begin_write(void) const {
	return begin() + write_index_;
}

char *Block_Buffer::begin(void) {
	return &*buffer_.begin();
}

const char *Block_Buffer::begin(void) const {
	return &*buffer_.begin();
}

void Block_Buffer::copy(void const *data, size_t len) {
	copy(static_cast<const char*> (data), len);
}

void Block_Buffer::copy(char const *data, size_t len) {
	ensure_writable_bytes(len);
	std::copy(data, data + len, get_write_ptr());
	write_index_ += len;
}

void Block_Buffer::copy(std::string const &str) {
	copy(str.data(), str.length());
}

void Block_Buffer::copy(Block_Buffer *block) {
	copy(block->get_read_ptr(), block->readable_bytes());
}

void Block_Buffer::dump_inner(void) {
	int ridx = get_read_idx();
	uint16_t len = 0;
	read_uint16(len);
	printf("len = %d\n", len);
	dump();
	set_read_idx(ridx);
}

void Block_Buffer::dump(void) {
	if (write(STDOUT_FILENO, this->get_read_ptr(), this->readable_bytes()) == -1) {
		//std::cerr << "write return -1\n";
	}
	//printf("\n");
}

void Block_Buffer::debug(void) {
	LOG_DEBUG_INFO("  read_index = %ul, write_index = %ul, buffer_.size = %ul.", read_index_, write_index_, buffer_.size());;
}

int Block_Buffer::peek_int8(int8_t &v) {
	if (verify_read(sizeof(v))) {
		memcpy(&v, &(buffer_[read_index_]), sizeof(v));
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::peek_int16(int16_t &v) {
	if (verify_read(sizeof(v))) {
#ifdef BLOCK_BIG_ENDIAN
		uint16_t t, u;
		memcpy(&t, &(buffer_[read_index_]), sizeof(t));
		u = be16toh(t);
		memcpy(&v, &u, sizeof(v));
#endif
#ifdef BLOCK_LITTLE_ENDIAN
		memcpy(&v, &(buffer_[read_index_]), sizeof(v));
#endif
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::peek_int32(int32_t &v) {
	if (verify_read(sizeof(v))) {
#ifdef BLOCK_BIG_ENDIAN
		uint32_t t, u;
		memcpy(&t, &(buffer_[read_index_]), sizeof(t));
		u = be32toh(t);
		memcpy(&v, &u, sizeof(v));
#endif
#ifdef BLOCK_LITTLE_ENDIAN
		memcpy(&v, &(buffer_[read_index_]), sizeof(v));
#endif
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::peek_int64(int64_t &v) {
	if (verify_read(sizeof(v))) {
#ifdef BLOCK_BIG_ENDIAN
		uint64_t t, u;
		memcpy(&t, &(buffer_[read_index_]), sizeof(t));
		u = be64toh(t);
		memcpy(&v, &u, sizeof(v));
#endif
#ifdef BLOCK_LITTLE_ENDIAN
		memcpy(&v, &(buffer_[read_index_]), sizeof(v));
#endif
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::peek_uint8(uint8_t &v) {
	if (verify_read(sizeof(v))) {
		memcpy(&v, &(buffer_[read_index_]), sizeof(v));
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::peek_uint16(uint16_t &v) {
	if (verify_read(sizeof(v))) {
#ifdef BLOCK_BIG_ENDIAN
		uint16_t t;
		memcpy(&t, &(buffer_[read_index_]), sizeof(t));
		v = be16toh(t);
#endif
#ifdef BLOCK_LITTLE_ENDIAN
		memcpy(&v, &(buffer_[read_index_]), sizeof(v));
#endif
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::peek_uint32(uint32_t &v) {
	if (verify_read(sizeof(v))) {
#ifdef BLOCK_BIG_ENDIAN
		uint32_t t;
		memcpy(&t, &(buffer_[read_index_]), sizeof(t));
		v = be32toh(t);
#endif
#ifdef BLOCK_LITTLE_ENDIAN
		memcpy(&v, &(buffer_[read_index_]), sizeof(v));
#endif
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::read(char *data, size_t len){
    if (verify_read(len)) {
        ::memcpy(data, &(buffer_[read_index_]), len);
        read_index_ += len;
    } else {
        LOG_USER_TRACE("out of range");
        return -1;
    }
    return 0;
}

int Block_Buffer::read_int8(int8_t &v) {
	if (verify_read(sizeof(v))) {
		memcpy(&v, &(buffer_[read_index_]), sizeof(v));
		read_index_ += sizeof(v);
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::read_int16(int16_t &v) {
	if (verify_read(sizeof(v))) {
#ifdef BLOCK_BIG_ENDIAN
		uint16_t t, u;
		memcpy(&t, &(buffer_[read_index_]), sizeof(t));
		u = be16toh(t);
		memcpy(&v, &u, sizeof(v));
		read_index_ += sizeof(v);
#endif
#ifdef BLOCK_LITTLE_ENDIAN
		memcpy(&v, &(buffer_[read_index_]), sizeof(v));
		read_index_ += sizeof(v);
#endif
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::read_int16_big_endian(int16_t &v) {
	if (verify_read(sizeof(v))) {
		uint16_t t, u;
		memcpy(&t, &(buffer_[read_index_]), sizeof(t));
		u = be16toh(t);
		memcpy(&v, &u, sizeof(v));
		read_index_ += sizeof(v);
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::read_int32_big_endian(int32_t &v) {
    if (verify_read(sizeof(v))) {
        uint32_t t;
        unsigned char *v_chr = (unsigned char *)(&t);
        memcpy(&t, &(buffer_[read_index_]), sizeof(t));
        read_index_ += sizeof(t);

        int v_size = sizeof(t) / 2;
        unsigned char tmp_chr;
        for (int i = 0; i < v_size; ++i)
        {
        	tmp_chr = v_chr[i];
            v_chr[i] = v_chr[sizeof(t) - i - 1];
            v_chr[sizeof(t) - i - 1] = tmp_chr;
        }
        v = t;
    } else {
        LOG_USER_TRACE("out of range");
        return -1;
    }
    return 0;
}

int Block_Buffer::read_int32(int32_t &v) {
	if (verify_read(sizeof(v))) {
#ifdef BLOCK_BIG_ENDIAN
		uint32_t t, u;
		memcpy(&t, &(buffer_[read_index_]), sizeof(t));
		u = be32toh(t);
		memcpy(&v, &u, sizeof(v));
		read_index_ += sizeof(v);
#endif
#ifdef BLOCK_LITTLE_ENDIAN
		memcpy(&v, &(buffer_[read_index_]), sizeof(v));
		read_index_ += sizeof(v);
#endif
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::read_int64(int64_t &v) {
	if (verify_read(sizeof(v))) {
#ifdef BLOCK_BIG_ENDIAN
		uint64_t t, u;
		memcpy(&t, &(buffer_[read_index_]), sizeof(t));
		u = be64toh(t);
		memcpy(&v, &u, sizeof(v));
		read_index_ += sizeof(v);
#endif
#ifdef BLOCK_LITTLE_ENDIAN
		memcpy(&v, &(buffer_[read_index_]), sizeof(v));
		read_index_ += sizeof(v);
#endif
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::read_uint8(uint8_t &v) {
	if (verify_read(sizeof(v))) {
		memcpy(&v, &(buffer_[read_index_]), sizeof(v));
		read_index_ += sizeof(v);
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::read_uint16(uint16_t &v) {
	if (verify_read(sizeof(v))) {
#ifdef BLOCK_BIG_ENDIAN
		uint16_t t;
		memcpy(&t, &(buffer_[read_index_]), sizeof(t));
		v = be16toh(t);
		read_index_ += sizeof(v);
#endif
#ifdef BLOCK_LITTLE_ENDIAN
		memcpy(&v, &(buffer_[read_index_]), sizeof(v));
		read_index_ += sizeof(v);
#endif
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::read_uint32(uint32_t &v) {
	if (verify_read(sizeof(v))) {
#ifdef BLOCK_BIG_ENDIAN
		uint32_t t;
		memcpy(&t, &(buffer_[read_index_]), sizeof(t));
		v = be32toh(t);
		read_index_ += sizeof(v);
#endif
#ifdef BLOCK_LITTLE_ENDIAN
		memcpy(&v, &(buffer_[read_index_]), sizeof(v));
		read_index_ += sizeof(v);
#endif
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::read_uint64(uint64_t &v) {
	if (verify_read(sizeof(v))) {
#ifdef BLOCK_BIG_ENDIAN
		uint64_t t;
		memcpy(&t, &(buffer_[read_index_]), sizeof(t));
		v = be64toh(t);
		read_index_ += sizeof(v);
#endif
#ifdef BLOCK_LITTLE_ENDIAN
		memcpy(&v, &(buffer_[read_index_]), sizeof(v));
		read_index_ += sizeof(v);
#endif
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::write_int8(const int8_t v) {
	copy(&v, sizeof(v));
	return 0;
}

int Block_Buffer::write_int16(const int16_t v) {
#ifdef BLOCK_BIG_ENDIAN
	uint16_t t, u;
	t = *((uint16_t *)&v);
	u = htobe16(t);
	copy(&u, sizeof(u));
#endif
#ifdef BLOCK_LITTLE_ENDIAN
	copy(&v, sizeof(v));
#endif
	return 0;
}

int Block_Buffer::write_int32(const int32_t v) {
#ifdef BLOCK_BIG_ENDIAN
	uint32_t t, u;
	t = *((uint32_t *)&v);
	u = htobe32(t);
	copy(&u, sizeof(u));
#endif
#ifdef BLOCK_LITTLE_ENDIAN
	copy(&v, sizeof(v));
#endif
	return 0;
}

int Block_Buffer::write_int64(const int64_t v) {
#ifdef BLOCK_BIG_ENDIAN
	uint64_t t, u;
	t = *((uint64_t *)&v);
	u = htobe64(t);
	copy(&u, sizeof(u));
#endif
#ifdef BLOCK_LITTLE_ENDIAN
	copy(&v, sizeof(v));
#endif
	return 0;
}

int Block_Buffer::write_uint8(const uint8_t v) {
	copy(&v, sizeof(v));
	return 0;
}

int Block_Buffer::write_uint16(const uint16_t v) {
#ifdef BLOCK_BIG_ENDIAN
	uint16_t t;
	t = htobe16(v);
	copy(&t, sizeof(t));
#endif
#ifdef BLOCK_LITTLE_ENDIAN
	copy(&v, sizeof(v));
#endif
	return 0;
}

int Block_Buffer::write_uint32(const uint32_t v) {
#ifdef BLOCK_BIG_ENDIAN
	uint32_t t;
	t = htobe32(v);
	copy(&t, sizeof(t));
#endif
#ifdef BLOCK_LITTLE_ENDIAN
	copy(&v, sizeof(v));
#endif
	return 0;
}

int Block_Buffer::write_uint64(const uint64_t v) {
#ifdef BLOCK_BIG_ENDIAN
	uint64_t t;
	t = htobe64(v);
	copy(&t, sizeof(t));
#endif

#ifdef BLOCK_LITTLE_ENDIAN
	copy(&v, sizeof(v));
#endif
	return 0;
}

int Block_Buffer::write_int64_var_big_endian(const int64_t v) {
	int64_t t = 0, value = v;
	unsigned byte = 0;
	for (int i = 0; i < 8; ++i)
	{
		byte = (value & 0x0FF);
		value = (value >> 8);
		if (byte == 0)
			break;
		t = ((t << 8) | (byte & 0x0FF));
	}
	copy(&t, sizeof(t));
	return 0;
}

int Block_Buffer::peek_double(double &v) {
#ifdef BLOCK_BIG_ENDIAN
	uint64_t t, u;
	memcpy(&t, &(buffer_[read_index_]), sizeof(t));
	u = be64toh(t);
	memcpy(&v, &u, sizeof(v));
#endif
#ifdef BLOCK_LITTLE_ENDIAN
	memcpy(&v, &(buffer_[read_index_]), sizeof(v));
#endif
	return 0;
}

int Block_Buffer::read_double(double &v) {
	if (verify_read(sizeof(v))) {
#ifdef BLOCK_BIG_ENDIAN
		uint64_t t, u;
		memcpy(&t, &(buffer_[read_index_]), sizeof(t));
		u = be64toh(t);
		memcpy(&v, &u, sizeof(v));
		read_index_ += sizeof(t);
#endif
#ifdef BLOCK_LITTLE_ENDIAN
		memcpy(&v, &(buffer_[read_index_]), sizeof(v));
		read_index_ += sizeof(v);
#endif
	} else {
		LOG_USER_TRACE("out of range");
		return -1;
	}
	return 0;
}

int Block_Buffer::write_double(const double v) {
#ifdef BLOCK_BIG_ENDIAN
	uint64_t t, u;
	t = *((uint64_t *)&v);
	u = htobe64(t);
	copy(&u, sizeof(u));
#endif
#ifdef BLOCK_LITTLE_ENDIAN
	copy(&v, sizeof(v));
#endif
	return 0;
}

int Block_Buffer::peek_string(std::string &str) {
	uint16_t len;
	read_uint16(len);
	if (len < 0) return -1;
	str.append(buffer_[read_index_], len);
	return 0;
}

int Block_Buffer::read_string(std::string &str) {
	uint16_t len = 0;
	read_uint16(len);
	if (len < 0) return -1;
	str.resize(len);
	memcpy((char *)str.c_str(), this->get_read_ptr(), len);
	//str.append(buffer_[read_index_], len);
	read_index_ += len;
	return 0;
}

int Block_Buffer::write_string(const std::string &str) {
	uint16_t len = str.length();
	write_uint16(len);
	copy(str.c_str(), str.length());
	return 0;
}

Block_Buffer &Block_Buffer::operator>>(int8_t &v) {
	read_int8(v);
	return *this;
}

Block_Buffer &Block_Buffer::operator>>(int16_t &v) {
	read_int16(v);
	return *this;
}

Block_Buffer &Block_Buffer::operator>>(int32_t &v) {
	read_int32(v);
	return *this;
}

Block_Buffer &Block_Buffer::operator>>(uint8_t &v) {
	read_uint8(v);
	return *this;
}

Block_Buffer &Block_Buffer::operator>>(uint16_t &v) {
	read_uint16(v);
	return *this;
}

Block_Buffer &Block_Buffer::operator>>(uint32_t &v) {
	read_uint32(v);
	return *this;
}

Block_Buffer &Block_Buffer::operator<<(const int8_t v) {
	write_int8(v);
	return *this;
}

Block_Buffer &Block_Buffer::operator<<(const int16_t v) {
	write_int16(v);
	return *this;
}

Block_Buffer &Block_Buffer::operator<<(const int32_t v) {
	write_int32(v);
	return *this;
}

Block_Buffer &Block_Buffer::operator<<(const uint8_t v) {
	write_uint8(v);
	return *this;
}

Block_Buffer &Block_Buffer::operator<<(const uint16_t v) {
	write_uint16(v);
	return *this;
}

Block_Buffer &Block_Buffer::operator<<(const uint32_t v) {
	write_uint32(v);
	return *this;
}

Block_Buffer &Block_Buffer::operator>>(double &v) {
	read_double(v);
	return *this;
}

Block_Buffer &Block_Buffer::operator<<(const double v) {
	write_double(v);
	return *this;
}

Block_Buffer &Block_Buffer::operator>>(std::string &v) {
	read_string(v);
	return *this;
}

Block_Buffer &Block_Buffer::operator<<(const std::string &v) {
	write_string(v);
	return *this;
}

void Block_Buffer::make_message(int msg_id, int status) {
	write_uint16(0); /// length
	write_uint32(msg_id);
	write_int32(status);
}

void Block_Buffer::finish_message(void) {
	int len = readable_bytes() - sizeof(uint16_t);
	int wr_idx = get_write_idx();
	set_write_idx(get_read_idx());
	write_uint16(len);
	set_write_idx(wr_idx);
}

inline int Block_Buffer::move_data(size_t dest, size_t begin, size_t end) {
	if (begin >= end) {
		LOG_USER_TRACE("begin = %ul, end = %ul, dest = %ul.", begin, end, dest);
		return -1;
	}
	size_t len = end - begin;
	this->ensure_writable_bytes(dest + len);
	std::memmove(this->begin() + dest, this->begin() + begin, len);
	return 0;
}

inline int Block_Buffer::insert_head(Block_Buffer *buf) {
	if (! buf) {
		LOG_USER_TRACE("buf == 0");
		return -1;
	}

	size_t len = 0;
	if ((len = buf->readable_bytes()) <= 0) {
		return -1;
	}

	size_t dest = read_index_ + len;

	move_data(dest, read_index_, write_index_);
	std::memcpy(this->get_read_ptr(),  buf->get_read_ptr(), len);

	this->set_write_idx(this->get_write_idx() + len);

	return 0;
}

inline size_t Block_Buffer::capacity(void) {
	return buffer_.capacity();
}

inline void Block_Buffer::recycle_space(void) {
//	if (max_use_times_ == 0)
//		return;
	if (init_offset_ == read_index_ && init_offset_ == write_index_)
		return;

	/*if (use_times_ >= max_use_times_)*/ {
//			std::vector<char> buffer_free(init_offset_ + init_size_);
 			buffer_.resize(init_offset_ + init_size_);
 			read_index_ = write_index_ = init_offset_;
 			use_times_ = 0;
 			ensure_writable_bytes(init_offset_);
	}
}

inline bool Block_Buffer::is_legal(void) {
	return read_index_ < write_index_;
}

inline bool Block_Buffer::verify_read(size_t s) {
	return (read_index_ + s <= write_index_) && (write_index_ <= buffer_.size());
}

#endif /* BLOCK_BUFFER_H_ */
