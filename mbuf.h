
/*
**mem_manage.c:内存管理接口函数
**
**CopyRight (c) 2010
**
**修改记录:
**2010-06-28    :JC 创建
*/

#ifndef __MEM_MANAGER_H
#define __MEM_MANAGER_H

OAL_DECLS_BEGIN

#define MBUF_BLK_MINX0 0  /*内存块标识符0*/
#define MBUF_BLK_MINX2 1  /*内存块标识符1*/
#define MBUF_BLK_MINX4 2  /*内存块标识符2*/
#define MBUF_BLK_MINX8 3  /*内存块标识符3*/
#define MBUF_BLK_MINX16 4 /*内存块标识符4*/
#define MBUF_BLK_MINX32 5 /*内存块标识符5*/
typedef struct __mbuf_stat_t
{
    unsigned total; /*该缓冲块的总块数*/
    unsigned used;  /*当前已经使用掉的块数*/
    unsigned peak;  /*峰值使用的内存块数*/
} mbuf_stat_t;

typedef struct mbuf_blkhdr_t
{
    uint32_t fixed_header;                  /* 固定的头位置标示，固定，用于检测内存被篡改 */
    int32_t magic;                          /* 块标识符*/
    uint32_t blksiz;                        /* 分配的内存大小 */
    uint32_t tag;                           /* 该消息的唯一标示 */
    char *dbg_mbuf;                         /* 内存调用信息,调试专用*/
    struct mbuf_blkhdr_t *blkpre, *blknext; /* 下一缓冲块指针*/
} mbuf_blkhdr_t;

typedef struct mbuf_blktail_t
{
    uint32_t fixed_tailer; /* 固定的尾位置标示，固定，用于检测内存被篡改 */
} mbuf_blktail_t;

#define MBUF_EXCEPTION_HDR_OVER 0  /*内存头溢出*/
#define MBUF_EXCEPTION_TAIL_OVER 1 /*内存尾溢出*/
#define MBUF_EXCEPTION_REFREE 2    /*内存重复释放*/

#define MBUF_EXCEPTION_RST_GOON 0    /*内存错误处理结果，继续运行*/
#define MBUF_EXCEPTION_RST_OVER (-1) /*内存错误处理结果，死机*/
/*参数1:出错类型,
参数2:出错地址
返回值:0-继续运行，-1=异常出错*/
typedef int32_t (*pfunc_mbuf_dbg)(uint32_t, void *); /* 专为MBUF设计的异常回调函数指针 */

void maccess(void);
void munaccess(void);
void *__mget(size_t cb, const char *pf, int line);
void __mput(void *blkp);
#ifdef OS_VERSION
void mbuf_init(unsigned prio);
void mbuf_init_dbg(unsigned prio, uint32_t dbg_flag, pfunc_mbuf_dbg user_dbg_func);
#else
void mbuf_init(void);
void mbuf_init_dbg(uint32_t dbg_flag, pfunc_mbuf_dbg user_dbg_func);
#endif
const mbuf_stat_t *mbuf_query(int magic);
const mbuf_blkhdr_t *mbuf_query_blk(int magic);
#if 0
#define _S_LINE(x) #x
#define __S_LINE(x) _S_LINE(x)
#define __S_LINE__ __S_LINE(__LINE__)

#define mget(cb) mget_l((cb), ("" __FILE__ ":" __S_LINE__))
#define mgetz(cb) mgetz_l((cb), ("" __FILE__ ":" __S_LINE__))
#endif

#define mget(cb) mget_l((cb), __FUNCTION__, __LINE__)
#define mgetz(cb) mgetz_l((cb), __FUNCTION__, __LINE__)

#if 0
#define mget(cb) ({                                 \
    void *blkp;                                     \
    const char info[128] = {0};                     \
    maccess();                                      \
    sprintf(info, "%s,%d", __FUNCTION__, __LINE__); \
    blkp = __mget(cb, info);                        \
    munaccess();                                    \
    blkp;                                           \
})

#define mgetz(cb) ({                                \
    void *blkp;                                     \
    const char info[128] = {0};                     \
    maccess();                                      \
    sprintf(info, "%s,%d", __FUNCTION__, __LINE__); \
    blkp = __mget(cb, info);                        \
    munaccess();                                    \
    memset(blkp, 0, cb);                            \
    blkp;                                           \
})

#define mput(blkp) ({ \
    if (NULL != blkp) \
    {                 \
        maccess();    \
        __mput(blkp); \
        munaccess();  \
    }                 \
})

#endif

#include "list.h"

extern oal_list_t mp_info_list[6];
typedef struct mp_info
{
    oal_list_t access_info_list;
    void *ptr;
} mp_info_t;

static void add_to_list(int magic, oal_list_t *new)
{
    oal_list_add(&mp_info_list[magic], new);
}

static inline void *mget_l(size_t cb, const char *fp, int line)
{
    mbuf_blkhdr_t *hdr;
    void *blkp;

    maccess();
    blkp = __mget(cb, fp, line);
    

    if (blkp != NULL)
    {
        hdr = (mbuf_blkhdr_t *)blkp - 1;

        hdr->dbg_mbuf = malloc(128);

        if (hdr->dbg_mbuf != NULL)
        {
            sprintf(hdr->dbg_mbuf, "%s:%d", fp, line);
        }

        if (hdr->magic >= 0)
        {
            mp_info_t *mp_info = malloc(sizeof(mp_info_t));

            mp_info->ptr = blkp;
           // oal_list_add(&mp_info_list[hdr->magic], &(mp_info->access_info_list));
            add_to_list(hdr->magic, &(mp_info->access_info_list));
        }
    }

    munaccess();

    return blkp;
}

static inline void *mgetz_l(size_t cb, const char *fp, int line)
{
    mbuf_blkhdr_t *hdr;
    void *blkp;

    maccess();
    blkp = __mget(cb, fp, line);

    if (blkp != NULL)
    {
        hdr = (mbuf_blkhdr_t *)blkp - 1;

        hdr->dbg_mbuf = malloc(128);

        if (hdr->dbg_mbuf != NULL)
        {
            sprintf(hdr->dbg_mbuf, "%s:%d", fp, line);
        }

        if (hdr->magic >= 0)
        {
            mp_info_t *mp_info = malloc(sizeof(mp_info_t));

            mp_info->ptr = blkp;
            //oal_list_add(&mp_info_list[hdr->magic], &(mp_info->access_info_list));
            add_to_list(hdr->magic, &(mp_info->access_info_list));
        }
    }

    munaccess();

    memset(blkp, 0, cb);
    return blkp;
}

static inline void mput(void *blkp)
{
    mbuf_blkhdr_t *hdr;
    oal_list_t *list, *next;
    mp_info_t *mp_info;

    if (blkp != NULL)
    {
        maccess();

        hdr = (mbuf_blkhdr_t *)blkp - 1;

        if (hdr->dbg_mbuf != NULL)
        {
            free(hdr->dbg_mbuf);

            hdr->dbg_mbuf = NULL;
        }

        if (hdr->magic >= 0)
        {
            oal_list_foreach_safe(list, next, &mp_info_list[hdr->magic])
            {
                mp_info = oal_list_entry(list, mp_info_t, access_info_list);
                if (mp_info->ptr == blkp)
                {
                    oal_list_del(&mp_info->access_info_list);
                    free(mp_info);
                }
            }
        }

        
        __mput(blkp);
        munaccess();
    }
}

OAL_DECLS_END
#endif /* __MEM_MANAGER_H  */
