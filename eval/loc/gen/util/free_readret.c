#include "util.h"

/**
 * @brief Frees the memory allocated for a read_ret structure and its buffer.
 *
 * This function implements the deallocation process for a `read_ret` object.
 * It first frees the dynamically allocated character buffer (`p->buf`) and then
 * frees the memory for the structure (`p`) itself.
 *
 * @param p A pointer to the `read_ret` structure to be freed.
 *
 */
void free_readret(struct read_ret *p)
{
	free(p->buf);
	free(p);
}
