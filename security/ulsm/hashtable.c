#include <linux/slab.h>

#include "include/hashtable.h"
#include "include/definitions.h"


struct hashtable_node {
	uint64_t value;
	uint32_t key;
	struct hlist_node hlist;
};

int hashtable_set(struct hashtable *hashtable, uint32_t key, uint64_t value)
{
	struct hashtable_node *node;

	if (!hashtable)
		return -1;

	spin_lock_bh(&hashtable->hash_lock);

	hash_for_each_possible(hashtable->table, node, hlist, key) {
		if (node->key == key) {
			/*found existing key - update value*/
			node->value = value;
			spin_unlock_bh(&hashtable->hash_lock);
			return SUCCESS;
		}
	}

	/*key doesn't exist*/
	node = kmalloc(sizeof(*node), GFP_ATOMIC);
	if (node == NULL) {
		spin_unlock_bh(&hashtable->hash_lock);

		return -ENOMEM;
	}

	node->key = key;
	node->value = value;
	hash_add(hashtable->table, &node->hlist, key);

	spin_unlock_bh(&hashtable->hash_lock);

	return SUCCESS;
}


int hashtable_delete(struct hashtable *hashtable, uint32_t key)
{
	struct hashtable_node *node;

	if (!hashtable)
		return -1;

	spin_lock_bh(&hashtable->hash_lock);

	hash_for_each_possible(hashtable->table, node, hlist, key) {
		if (node->key == key) {
			hash_del(&node->hlist);
			kfree(node);

			spin_unlock_bh(&hashtable->hash_lock);

			return SUCCESS;
		}
	}

	spin_unlock_bh(&hashtable->hash_lock);

	return -1;
}


uint64_t hashtable_get(struct hashtable *hashtable, uint32_t key)
{
	struct hashtable_node *node;
	uint64_t out = 0;

	if (!hashtable)
		return -1;

	spin_lock_bh(&hashtable->hash_lock);
	hash_for_each_possible(hashtable->table, node, hlist, key) {
		if (node->key == key) {
			out = node->value;
			spin_unlock_bh(&hashtable->hash_lock);

			return out;
		}
	}

	spin_unlock_bh(&hashtable->hash_lock);

	return -1;
}

void hashtable_free(struct hashtable *hashtable)
{
	int i = 0;
	struct hashtable_node *node;
	struct hlist_node *tmp;

	if (!hashtable)
		return;

	spin_lock_bh(&hashtable->hash_lock);

	hash_for_each_safe(hashtable->table, i, tmp, node, hlist) {
		hash_del(&node->hlist);
		kfree(node);
	}

	spin_unlock_bh(&hashtable->hash_lock);
}
void hashtable_init(struct hashtable *hashtable)
{
	hash_init(hashtable->table);
	spin_lock_init(&hashtable->hash_lock);
}

