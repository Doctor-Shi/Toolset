
/*
**mem_manage.c:�ڴ����ӿں���
**
**CopyRight (c) 2010
**
**�޸ļ�¼:
**2010-06-28    :JC ����
*/

#ifndef __MEM_MANAGER_H
#define __MEM_MANAGER_H

OAL_DECLS_BEGIN

#define MBUF_BLK_MINX0 0  /*�ڴ���ʶ��0*/
#define MBUF_BLK_MINX2 1  /*�ڴ���ʶ��1*/
#define MBUF_BLK_MINX4 2  /*�ڴ���ʶ��2*/
#define MBUF_BLK_MINX8 3  /*�ڴ���ʶ��3*/
#define MBUF_BLK_MINX16 4 /*�ڴ���ʶ��4*/
#define MBUF_BLK_MINX32 5 /*�ڴ���ʶ��5*/
typedef struct __mbuf_stat_t
{
    unsigned total; /*�û������ܿ���*/
    unsigned used;  /*��ǰ�Ѿ�ʹ�õ��Ŀ���*/
    unsigned peak;  /*��ֵʹ�õ��ڴ����*/
} mbuf_stat_t;

typedef struct mbuf_blkhdr_t
{
    uint32_t fixed_header;                  /* �̶���ͷλ�ñ�ʾ���̶������ڼ���ڴ汻�۸� */
    int32_t magic;                          /* ���ʶ��*/
    uint32_t blksiz;                        /* ������ڴ��С */
    uint32_t tag;                           /* ����Ϣ��Ψһ��ʾ */
    char *dbg_mbuf;                         /* �ڴ������Ϣ,����ר��*/
    struct mbuf_blkhdr_t *blkpre, *blknext; /* ��һ�����ָ��*/
} mbuf_blkhdr_t;

typedef struct mbuf_blktail_t
{
    uint32_t fixed_tailer; /* �̶���βλ�ñ�ʾ���̶������ڼ���ڴ汻�۸� */
} mbuf_blktail_t;

#define MBUF_EXCEPTION_HDR_OVER 0  /*�ڴ�ͷ���*/
#define MBUF_EXCEPTION_TAIL_OVER 1 /*�ڴ�β���*/
#define MBUF_EXCEPTION_REFREE 2    /*�ڴ��ظ��ͷ�*/

#define MBUF_EXCEPTION_RST_GOON 0    /*�ڴ������������������*/
#define MBUF_EXCEPTION_RST_OVER (-1) /*�ڴ��������������*/
/*����1:��������,
����2:�����ַ
����ֵ:0-�������У�-1=�쳣����*/
typedef int32_t (*pfunc_mbuf_dbg)(uint32_t, void *); /* רΪMBUF��Ƶ��쳣�ص�����ָ�� */

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
