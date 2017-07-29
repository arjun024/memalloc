#include <unistd.h>
#include <string.h>
#include <pthread.h>
/* Only for the debug printf */
#include <stdio.h>


struct header_t {
	size_t size;
	unsigned is_free;
	struct header_t *next;
};

struct header_t *head = NULL, *tail = NULL;
pthread_mutex_t global_malloc_lock;

struct header_t *get_free_block(size_t size)
{
	
	struct header_t *curr = head;
	while(curr) {
		/* see if there's a free block that can accomodate requested size */
		if (curr->is_free && curr->size >= size)
			return curr;
		curr = curr->next;
	}
	printf("get free block\n");
	return NULL;
}

void free(void *block)
{
	//printf ("entered free \n");
	struct header_t *header, *tmp;
	/* program break is the end of the process's data segment */
	void *programbreak;

	if (!block)
		return;

	pthread_mutex_lock(&global_malloc_lock);
	header = (struct header_t*)block - 1;
	//printf("obtained lock\n");
	/* sbrk(0) gives the current program break address */
	programbreak = sbrk(0);

	/*
	   Check if the block to be freed is the last one in the
	   linked list. If it is, then we could shrink the size of the
	   heap and release memory to OS. Else, we will keep the block
	   but mark it as free.
	 */
	if ((char*)block + header->size == programbreak) {
		if (head == tail) {
			head = tail = NULL;
		} else {
			tmp = head;
			while (tmp) {
				
				if(tmp->next == tail) {
					tmp->next = NULL;
					tail = tmp;
				}
				tmp = tmp->next;
			}
		}
		/*
		   sbrk() with a negative argument decrements the program break.
		   So memory is released by the program to OS.
		*/
		sbrk(0 - header->size - sizeof(struct header_t));
		/* Note: This lock does not really assure thread
		   safety, because sbrk() itself is not really
		   thread safe. Suppose there occurs a foregin sbrk(N)
		   after we find the program break and before we decrement
		   it, then we end up realeasing the memory obtained by
		   the foreign sbrk().
		*/
		pthread_mutex_unlock(&global_malloc_lock);
		return;
	}
		
	struct header_t *cur=header;
	
	while (cur->next && (cur->next->is_free) && (char *)(cur->next)==(char *)(cur)+sizeof(struct header_t)+cur->size){
		header->size=(header->size)+(cur->next->size);
		cur=cur->next;
		
	header->is_free = 1;
	header->next=cur;
	/*Added Coalescing 
	
	if there are adjacent free blocks,they will be united(coalesced) in one free block and its size will be the total size of the adjacent free blocks.
	*/
	pthread_mutex_unlock(&global_malloc_lock);
	//printf("after lock release\n");
	return ;

}

void *malloc(size_t size)
{
	size_t total_size;
	void *block;
	struct header_t *header;
	if (size==0){
		return NULL;}
	pthread_mutex_lock(&global_malloc_lock);
	header = get_free_block(size);
	if (header) {
		/* Woah, found a free block to accomodate requested memory. */
		header->is_free = 0;
		pthread_mutex_unlock(&global_malloc_lock);
		return (void*)(header + 1);
	}
	/* We need to get memory to fit in the requested block and header from OS. */
	total_size = sizeof(struct header_t) + size;
	block = sbrk(total_size);
	if (block == (void*) -1) {
		pthread_mutex_unlock(&global_malloc_lock);
		return NULL;
	}
	header = block;
	header->size = size;
	header->is_free = 0;
	header->next = NULL;
	if (!head)
		head = header;
	if (tail)
		tail->next = header;
	tail = header;
	pthread_mutex_unlock(&global_malloc_lock);
	return (void*)(header + 1);
}

void *calloc(size_t num, size_t nsize)
{	
	size_t size;
	void *block;
	if (!num || !nsize)
		return NULL;
	size = num * nsize;
	/* check mul overflow */
	if (nsize != size / num)
		return NULL;
	block = malloc(size);
	if (!block)
		return NULL;
	memset(block, 0, size);
	return block;
}

void *realloc(void *block, size_t size)
{
	struct header_t *header;
	void *ret;
	if (!block || !size)
		return malloc(size);
	header = (struct header_t*)block - 1;
	if (header->size >= size)
		return block;
	ret = malloc(size);
	if (ret) {
		/* Relocate contents to the new bigger block */
		memcpy(ret, block, header->size);
		/* Free the old memory block */
		free(block);
	}
	return ret;
}

/* A debug function to print the entire link list */
void print_mem_list()
{
	struct header_t *curr = head;
	printf("head = %p, tail = %p \n", (void*)head, (void*)tail);
	while(curr) {
		printf("addr = %p, size = %zu, is_free=%u, next=%p\n",
			(void*)curr, curr->size, curr->is_free, (void*)curr->next);
		curr = curr->next;
	}
}

