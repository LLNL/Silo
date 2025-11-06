# This file is included by CTest at test time (not configure time)
set(ENV{ASAN_OPTIONS} "detect_leaks=1:halt_on_error=1:malloc_context_size=50:fast_unwind_on_malloc=0:detect_stack_use_after_return=1")
