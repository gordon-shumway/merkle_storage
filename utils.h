#pragma once

#include <string>

#define merge4x8to32(var, start) (((uint32_t)var[start]) << 24) + (((uint32_t)var[start+1]) << 16) + (((uint32_t)var[start+2]) << 8) + var[start+3]
#define	split32to4x8(var, start, val32) \
	var[start] = (uint8_t)(val32 >> 24);	\
	var[start+1] = (uint8_t)(val32 >> 16);	\
	var[start+2] = (uint8_t)(val32 >> 8);	\
	var[start+3] = (uint8_t)val32;

bool is_file_exists(const std::string& path);
void delete_file(const std::string& path);

