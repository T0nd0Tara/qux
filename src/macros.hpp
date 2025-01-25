#pragma once

#define NOT_IMPLEMENTED(msg) \
  assert(false && "NOT_IMPLEMENTED" && (msg))

#define NOT_REACHABLE(msg) \
  assert(false && "NOT_REACHABLE" && (msg))
