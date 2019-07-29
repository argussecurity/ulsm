/**
 * @file
 *
 * Declares the hash uint to ulong class
 */
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <linux/hashtable.h>
#include <linux/spinlock.h>


/** The hashtable default size. */
#define HASHTABLE_DEFAULT_SIZE 2048

/**
 * hashtable of uint32 (key) to u64 (data)
 */
struct hashtable {
	/**
	 * Decleration of a Linux hashtable.
	 * @see https://kernelnewbies.org/FAQ/Hashtables
	 */
	DECLARE_HASHTABLE(table, ilog2(HASHTABLE_DEFAULT_SIZE));
	/** Lock, to syncronize operations on the hashtable. */
	spinlock_t hash_lock;
};

/**
 * Returns the value assigned to `key` in `out`.
 *
 * @param [in]	hashtable The hashtable.
 * @param [in]	key The key.
 * @param [out]	out The output buffer.
 *
 * @return Error code
 */
uint64_t hashtable_get(struct hashtable *hashtable, uint32_t key);


/**
 * Deletes the node with the given key from a hash table
 *
 * @param [in, out]	hashtable The hashtable.
 * @param [in]		key The key.
 *
 * @return Error code
 */
int hashtable_delete(struct hashtable *hashtable, uint32_t key);

/**
 * Sets (or inserts) a key-value pair into a hash table.
 *
 * @param [in, out]	hashtable The hashtable.
 * @param [in]		key The key.
 * @param [in]		value The value to set.
 *
 * @return Error code
 */
int hashtable_set(struct hashtable *hashtable, uint32_t key, uint64_t value);

/**
 * Deletes the hash table and free its memory
 *
 * @param [in,out] hashtable The hashtable.
 */
void hashtable_free(struct hashtable *hashtable);

/**
 * Creates and initializes the hashtable
 *
 * @param [in,out] hashtable The hashtable.
 */
void hashtable_init(struct hashtable *hashtable);

#endif /** HASHTABLE_H */
