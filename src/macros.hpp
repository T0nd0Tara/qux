#pragma once

#define NOT_IMPLEMENTED(msg)                                                   \
  do {                                                                         \
    std::cerr << "NOT_IMPLEMENTED: " << (msg) << "\n";                         \
    assert(false);                                                             \
  } while (false)

#define NOT_REACHABLE(msg)                                                     \
  do {                                                                         \
    std::cerr << "NOT_REACHABLE: " << (msg) << "\n";                           \
    assert(false);                                                             \
  } while (false)
