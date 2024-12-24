#pragma once

#if defined(_MSC_VER)
#define CHUNK_EXPORT __declspec(dllexport)
#else
#define CHUNK_EXPORT __attribute__((visibility("default")))
#endif