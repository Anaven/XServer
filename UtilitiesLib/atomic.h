#ifndef _ATOMIC_H_
#define _ATOMIC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Other atomic routines */

extern unsigned int compare_and_store(unsigned int oval,
                                unsigned int nval, unsigned int *area);

extern unsigned int atomic_add(unsigned int *area, int val);

extern unsigned int atomic_or(unsigned int *area, unsigned int mask);

extern unsigned int atomic_sub(unsigned int *area, int val);

#ifdef __cplusplus
}
#endif

#endif /* _ATOMIC_H_ */
