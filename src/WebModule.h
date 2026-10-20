#pragma once

#include <cstdint>
#include <cstddef>

namespace WebModule {
void begin();
void wsCleanupClients();
int wsNumberOfConnectedClients();
bool wsAvailableForWrite();
void wsSendBuf(const void* pBuf, size_t len);
}  // namespace WebModule