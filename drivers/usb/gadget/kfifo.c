/*
 * A simple kernel FIFO implementation.
 *
 * Copyright (C) 2004 Stelian Pop <stelian@popies.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/kfifo.h>
#include <linux/log2.h>
#include <linux/uaccess.h>

#if 1
static void _kfifo_init_gadget(struct kfifo *fifo, void *buffer,
      unsigned int size)
{
   fifo->buffer = buffer;
   fifo->size = size;

   kfifo_reset(fifo);
}
#endif

/**
 * kfifo_init - allocates a new FIFO using a preallocated buffer
 * @buffer: the preallocated buffer to be used.
 * @size: the size of the internal buffer, this have to be a power of 2.
 * @gfp_mask: get_free_pages mask, passed to kmalloc()
 * @lock: the lock to be used to protect the fifo buffer
 *
 * Do NOT pass the kfifo to kfifo_free() after use! Simply free the
 * &struct kfifo with kfree().
 */
void kfifo_init_gadget(struct kfifo *fifo, void *buffer, unsigned int size)
{
	/* size must be a power of 2 */
	BUG_ON(!is_power_of_2(size));
	_kfifo_init_gadget(fifo, buffer, size);
}
//EXPORT_SYMBOL(kfifo_init);

/**
 * kfifo_alloc - allocates a new FIFO and its internal buffer
 * @size: the size of the internal buffer to be allocated.
 * @gfp_mask: get_free_pages mask, passed to kmalloc()
 * @lock: the lock to be used to protect the fifo buffer
 *
 * The size will be rounded-up to a power of 2.
 */
int kfifo_alloc_gadget(struct kfifo *fifo, unsigned int size, gfp_t gfp_mask)

{
	unsigned char *buffer;

	/*
	 * round up to the next power of 2, since our 'let the indices
	 * wrap' tachnique works only in this case.
	 */
	if (size & (size - 1)) {
		BUG_ON(size > 0x80000000);
		size = roundup_pow_of_two(size);
	}

	buffer = kmalloc(size, gfp_mask);
	if (!buffer) {
		_kfifo_init_gadget(fifo, NULL, 0);
		return -ENOMEM;
	}
	return 0;

}
//EXPORT_SYMBOL(kfifo_alloc);

/**
 * kfifo_free - frees the FIFO
 * @fifo: the fifo to be freed.
 */
void kfifo_free_gadget(struct kfifo *fifo)
{
	kfree(fifo->buffer);
	_kfifo_init_gadget(fifo, NULL, 0);

}
//EXPORT_SYMBOL(kfifo_free);

#if 0
/**
 * __kfifo_put - puts some data into the FIFO, no locking version
 * @fifo: the fifo to be used.
 * @buffer: the data to be added.
 * @len: the length of the data to be added.
 *
 * This function copies at most @len bytes from the @buffer into
 * the FIFO depending on the free space, and returns the number of
 * bytes copied.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these functions.
 */
//unsigned int __kfifo_put(struct kfifo *fifo,
//			 unsigned char *buffer, unsigned int len)
void kfifo_skip(struct kfifo *fifo, unsigned int len)
{
   if (len < kfifo_len(fifo)) {
      __kfifo_add_out(fifo, len);
      return;
   }
   kfifo_reset_out(fifo);
}
EXPORT_SYMBOL(kfifo_skip);

static inline void __kfifo_in_data(struct kfifo *fifo,
      const void *from, unsigned int len, unsigned int off)
{
	unsigned int l;

   /*
    * Ensure that we sample the fifo->out index -before- we
    * start putting bytes into the kfifo.
    */

   smp_mb();

   off = __kfifo_off(fifo, fifo->in + off);

   /* first put the data starting from fifo->in to buffer end */
   l = min(len, fifo->size - off);
   memcpy(fifo->buffer + off, from, l);

   /* then put the rest (if any) at the beginning of the buffer */
   memcpy(fifo->buffer, from + l, len - l);
}

static inline void __kfifo_out_data(struct kfifo *fifo,
      void *to, unsigned int len, unsigned int off)
{
   unsigned int l;

   /*
    * Ensure that we sample the fifo->in index -before- we
    * start removing bytes from the kfifo.
   */

   smp_rmb();

   off = __kfifo_off(fifo, fifo->out + off);

   /* first get the data from fifo->out until the end of the buffer */
   l = min(len, fifo->size - off);
   memcpy(to, fifo->buffer + off, l);

   /* then get the rest (if any) from the beginning of the buffer */
   memcpy(to + l, fifo->buffer, len - l);
}

static inline int __kfifo_from_user_data(struct kfifo *fifo,
    const void __user *from, unsigned int len, unsigned int off,
    unsigned *lenout)
{
   unsigned int l;
   int ret;

	/*
	 * Ensure that we sample the fifo->out index -before- we
	 * start putting bytes into the kfifo.
	 */

	smp_mb();

	off = __kfifo_off(fifo, fifo->in + off);


	/* first put the data starting from fifo->in to buffer end */
	l = min(len, fifo->size - off);
 //   printk(KERN_INFO" The l value is ----> %d in %s\n",l,__func__);
	ret = copy_from_user(fifo->buffer + off, from, l);
	if (unlikely(ret)) {
		*lenout = ret;
		return -EFAULT;
	}
	*lenout = l;

	/* then put the rest (if any) at the beginning of the buffer */

	ret = copy_from_user(fifo->buffer, from + l, len - l);
	*lenout += ret ? ret : len - l;
	
 	return ret ? -EFAULT : 0;
}
static inline int __kfifo_to_user_data(struct kfifo *fifo,
      void __user *to, unsigned int len, unsigned int off, unsigned *lenout)
{
   unsigned int l;
   int ret;
  
    /*
       * Ensure that we sample the fifo->in index -before- we
       * start removing bytes from the kfifo.
    */
   smp_rmb();

   off = __kfifo_off(fifo, fifo->out + off);

   /* first get the data from fifo->out until the end of the buffer */
   l = min(len, fifo->size - off);
 //  printk(KERN_INFO"1. The length is %d \n",l);
   ret = copy_to_user(to, fifo->buffer + off, l);
   *lenout = l;
   if (unlikely(ret)) {
      *lenout -= ret;
      return -EFAULT;
   }

   /* then get the rest (if any) from the beginning of the buffer */
   len -= l;
 //   printk(KERN_INFO" 2. The length is %d \n",len);
   ret = copy_to_user(to + l, fifo->buffer, len);
   if (unlikely(ret)) {
      *lenout += len - ret;
      return -EFAULT;
   }
   *lenout += len;
   return 0;
}

unsigned int __kfifo_in_n(struct kfifo *fifo,
   const void *from, unsigned int len, unsigned int recsize)
{
   if (kfifo_avail(fifo) < len + recsize)
      return len + 1;

   __kfifo_in_data(fifo, from, len, recsize);
   return 0;
}
EXPORT_SYMBOL(__kfifo_in_n);
 
/**
 * kfifo_in - puts some data into the FIFO
 * @fifo: the fifo to be used.
 * @from: the data to be added.
 * @len: the length of the data to be added.
 *
 * This function copies at most @len bytes from the @from buffer into
 * the FIFO depending on the free space, and returns the number of
 * bytes copied.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these functions.
 */
unsigned int kfifo_in(struct kfifo *fifo, const void *from,
            unsigned int len)
{
   len = min(kfifo_avail(fifo), len);

   __kfifo_in_data(fifo, from, len, 0);
   __kfifo_add_in(fifo, len);
    return len;
 }
EXPORT_SYMBOL(kfifo_in);

unsigned int __kfifo_in_generic(struct kfifo *fifo,
   const void *from, unsigned int len, unsigned int recsize)
{
   return __kfifo_in_rec(fifo, from, len, recsize);
}
EXPORT_SYMBOL(__kfifo_in_generic);

unsigned int __kfifo_out_n(struct kfifo *fifo,
   void *to, unsigned int len, unsigned int recsize)
{
   if (kfifo_len(fifo) < len + recsize)
      return len;

   __kfifo_out_data(fifo, to, len, recsize);
   __kfifo_add_out(fifo, len + recsize);
   return 0;
}
EXPORT_SYMBOL(__kfifo_out_n);
 
unsigned int kfifo_out(struct kfifo *fifo, void *to, unsigned int len)
 {
   len = min(kfifo_len(fifo), len);
 
   __kfifo_out_data(fifo, to, len, 0);
   __kfifo_add_out(fifo, len);
   return len;
}
EXPORT_SYMBOL(kfifo_out);
 
/**
 * kfifo_out_peek - copy some data from the FIFO, but do not remove it
 * @fifo: the fifo to be used.
 * @to: where the data must be copied.
 * @len: the size of the destination buffer.
 * @offset: offset into the fifo
 *
 * This function copies at most @len bytes at @offset from the FIFO
 * into the @to buffer and returns the number of copied bytes.
 * The data is not removed from the FIFO.
 */
unsigned int kfifo_out_peek(struct kfifo *fifo, void *to, unsigned int len,
             unsigned offset)
{
   len = min(kfifo_len(fifo), len + offset);
 
   __kfifo_out_data(fifo, to, len, offset);
   return len;
}
EXPORT_SYMBOL(kfifo_out_peek);
 
unsigned int __kfifo_out_generic(struct kfifo *fifo,
   void *to, unsigned int len, unsigned int recsize,
   unsigned int *total)
{
   return __kfifo_out_rec(fifo, to, len, recsize, total);
}
EXPORT_SYMBOL(__kfifo_out_generic);
 
unsigned int __kfifo_from_user_n(struct kfifo *fifo,
   const void __user *from, unsigned int len, unsigned int recsize)
{
   unsigned total;
 
  if (kfifo_avail(fifo) < len + recsize)
     return len + 1;
 
   __kfifo_from_user_data(fifo, from, len, recsize, &total);
   return total;
}
EXPORT_SYMBOL(__kfifo_from_user_n);
 
/**
 * kfifo_from_user - puts some data from user space into the FIFO
 * @fifo: the fifo to be used.
 * @from: pointer to the data to be added.
 * @len: the length of the data to be added.
 * @total: the actual returned data length.
 *
 * This function copies at most @len bytes from the @from into the
 * FIFO depending and returns -EFAULT/0.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these functions.
 */
int kfifo_from_user(struct kfifo *fifo,
        const void __user *from, unsigned int len, unsigned *total)
{
   int ret;
   len = min(kfifo_avail(fifo), len);
   ret = __kfifo_from_user_data(fifo, from, len, 0, total);
   if (ret)
      return ret;
   __kfifo_add_in(fifo, len);
   return 0;
 }
EXPORT_SYMBOL(kfifo_from_user);

unsigned int __kfifo_from_user_generic(struct kfifo *fifo,
   const void __user *from, unsigned int len, unsigned int recsize)
{
   return __kfifo_from_user_rec(fifo, from, len, recsize);
}
EXPORT_SYMBOL(__kfifo_from_user_generic);

unsigned int __kfifo_to_user_n(struct kfifo *fifo,
   void __user *to, unsigned int len, unsigned int reclen,
   unsigned int recsize)
{
   unsigned int ret, total;

   if (kfifo_len(fifo) < reclen + recsize)
      return len;

   ret = __kfifo_to_user_data(fifo, to, reclen, recsize, &total);

   if (likely(ret == 0))
      __kfifo_add_out(fifo, reclen + recsize);

   return total;
}
EXPORT_SYMBOL(__kfifo_to_user_n);

/**
 * kfifo_to_user - gets data from the FIFO and write it to user space
 * @fifo: the fifo to be used.
 * @to: where the data must be copied.
 * @len: the size of the destination buffer.
 * @lenout: pointer to output variable with copied data
 *
 * This function copies at most @len bytes from the FIFO into the
 * @to buffer and 0 or -EFAULT.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these functions.
 */
int kfifo_to_user(struct kfifo *fifo,
   void __user *to, unsigned int len, unsigned *lenout)
{
   int ret;
   
   len = min(kfifo_len(fifo), len);
   
   ret = __kfifo_to_user_data(fifo, to, len, 0, lenout);

   __kfifo_add_out(fifo, *lenout);
   return ret;
}
EXPORT_SYMBOL(kfifo_to_user);

unsigned int __kfifo_to_user_generic(struct kfifo *fifo,
   void __user *to, unsigned int len, unsigned int recsize,
   unsigned int *total)
{
   return __kfifo_to_user_rec(fifo, to, len, recsize, total);
}
EXPORT_SYMBOL(__kfifo_to_user_generic);

unsigned int __kfifo_peek_generic(struct kfifo *fifo, unsigned int recsize)
{
   if (recsize == 0)
      return kfifo_avail(fifo);

   return __kfifo_peek_n(fifo, recsize);
}
EXPORT_SYMBOL(__kfifo_peek_generic);

void __kfifo_skip_generic(struct kfifo *fifo, unsigned int recsize)
{
   __kfifo_skip_rec(fifo, recsize);
}
EXPORT_SYMBOL(__kfifo_skip_generic);
#endif
