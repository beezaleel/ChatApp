#include "pch.h"
#include "Buffer.h"
#include <iostream>
#include <string>
#include <vector>


const unsigned int BYTE_SIZE = 8;

Buffer::Buffer()
	: Buffer(0) {}

Buffer::Buffer(size_t size) {
	m_buffer.resize(size);
}

Buffer::~Buffer() {}

void Buffer::WriteSignedInt(int value) {
	ResizeBuffer(sizeof(value));
	for (int i = 0; i < m_buffer.size(); i++) {
		m_buffer[i] = value >> (i * BYTE_SIZE);
	}
}

int Buffer::ReadSignedInt(size_t index) {
	int value = 0;
	for (int i = 0; i < sizeof(value); i++) {
		value += m_buffer[index] << (i * BYTE_SIZE);
		index++;
	}
	return value;
}

void Buffer::WriteShort(short value) {
	ResizeBuffer(sizeof(value));
	for (int i = 0; i < sizeof(value); i++) {
		m_buffer[i] = value >> (i * BYTE_SIZE);
	}
}
short Buffer::ReadShort(size_t index) {
	short value = 0;
	for (int i = 0; i < sizeof(value); i++) {
		value += m_buffer[index] << (i * BYTE_SIZE);
		index++;
	}
	return value;
}

void Buffer::WriteString(char* value) {
	// Plus one to because of `\0`
	size_t charLen = strlen(value) + 1;
	ResizeBuffer(charLen);
	std::cout << "The value of size is: " << sizeof(value[0]) << std::endl;
	for (int i = 0; i < charLen; i++) {
		if (i == (charLen - 1)) {
			m_buffer[i] = '\0';
		}
		else {
			m_buffer[i] = value[i];
		}
	}
}

char* Buffer::ReadString(size_t index) {
	char* value = (char*)malloc(sizeof(char)*m_buffer.size());
	for (int i = 0; i < m_buffer.size(); i++) {
		value[i] = m_buffer[i];
	}
	return value;
}

void Buffer::ResizeBuffer(size_t size) {
	m_buffer.resize(size);
}
