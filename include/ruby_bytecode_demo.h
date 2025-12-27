// Stub bytecode header - will be replaced by compile_ruby.py
// This allows building without mrbc installed (Ruby disabled)

#ifndef __RUBY_BYTECODE_DEMO_H__
#define __RUBY_BYTECODE_DEMO_H__

#include <stdint.h>

// Empty bytecode - Ruby code will not run until mrbc is installed
// and compile_ruby.py regenerates this file
const uint8_t demo_bytecode[] = {};
const size_t demo_bytecode_len = 0;

#endif
