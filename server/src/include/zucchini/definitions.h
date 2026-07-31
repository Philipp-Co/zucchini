#ifndef __ZUCCHINI_DEFINITIONS_H__
#define __ZUCCHINI_DEFINITIONS_H__

#define ZUC_LIKELY(x)   __builtin_expect(!!(x), 1)
#define ZUC_UNLIKELY(x) __builtin_expect(!!(x), 0)

#endif
