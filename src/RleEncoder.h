#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

void encodeFrame(const uint8_t* frame, size_t size, std::vector<uint8_t>& out);
