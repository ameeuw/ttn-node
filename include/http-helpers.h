#ifndef HTTP_HELPERS_H_
#define HTTP_HELPERS_H_

#include "include.h"

void updateClientRegistry();
void collect(uint16_t counterValue);
extern std::map<String, tasmotaNode> clientRegistry;

#endif
