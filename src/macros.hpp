#pragma once

#define NOT_IMPLEMENTED(msg)                                                   \
  std::cerr << "NOT_IMPLEMENTED: " << (msg) << "\n";                           \
  assert(false)

#define NOT_REACHABLE(msg)                                                     \
  std::cerr << "NOT_REACHABLE: " << (msg) << "\n";                             \
  assert(false)
