#include "memory.h"
#include "print.h"
#include "string.h"


#define PG_SIZE 4096 //4KB/page

#define MEM_BITMAP_BASE 0xc009a000 //支持四页的位图，最大可管理512MB的物理内存

#define KERNEL_HEAP_START 0xc0100000 //堆内存：3GB + 1MB开始

#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22) //返回虚拟地址的高10位
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12) //返回虚拟地址的中间10位

//内存池
struct pool {
    struct bitmap pool_bitmap;
    uint32_t phy_addr_start; //该内存池管理的物理内存起始地址
    uint32_t pool_size; //该内存池的容量（字节数）
};

struct pool kernel_pool, user_pool;
struct virtual_addr kernel_vaddr;

/**
 * 在pf表示的虚拟内存池中申请cnt个虚拟页
 * @return 成功：返回虚拟页的起始地址； 失败：返回NULL
 */
void *vaddr_get(enum pool_flag pf, uint32_t cnt){
    put_str("\nvaddr_get start\n", 0x07);

    int vaddr_start = 0, idx_bit_start = -1;
    uint32_t count = 0;
    if(pf == PF_KERNEL){
        idx_bit_start = bitmap_apply_cnt(&kernel_vaddr.vaddr_bitmap, cnt);
        if(idx_bit_start == -1) return NULL;
        
        while(count < cnt){
            bitmap_set_idx(&kernel_vaddr.vaddr_bitmap, idx_bit_start + count, 1);
            count ++;
        }
        vaddr_start = kernel_vaddr.vaddr_start + idx_bit_start * PG_SIZE;
    }
    else{

    }
    return (void *)vaddr_start;

}

/**
 * 求虚拟地址vaddr对应的pte指针
 * @return pte指针
 */
uint32_t *pte_ptr(uint32_t vaddr){
    // 0xffc00000让处理器在最后一个pde中取出页目录表的物理地址
    uint32_t pde_idx = vaddr & 0xffc00000; //高10位,后面0
    uint32_t *pte = (uint32_t *)(0xffc00000 + (pde_idx >> 10) + (PTE_IDX(vaddr) * 4));
    return pte;
}

/**
 * 求虚拟地址vaddr对应的pde指针
 * @return pde指针
 */
uint32_t *pde_ptr(uint32_t vaddr){
    // 0xfffffxxx获得页目录表的物理地址
    uint32_t *pde = (uint32_t *)(0xfffff000 + PDE_IDX(vaddr) * 4);
    return pde;
}

/**
 * page allocate 在p指向的物理内存池中分配一个物理页
 * @return 成功：返回页框的物理地址；失败：返回NULL
 */ 
void *palloc(struct pool* p){
    int idx = bitmap_apply_cnt(&p -> pool_bitmap, 1);
    if(idx == -1) return NULL;
    bitmap_set_idx(&p -> pool_bitmap, idx, 1);

    uint32_t page_phyaddr = (idx * PG_SIZE) + p -> phy_addr_start;
    return (void *)page_phyaddr;
}

/** 
 * 在页表中添加虚拟地址vaddr和物理大致page_phyaddr的映射
 */ 
void page_table_add(void *_vaddr, void *_page_phyaddr){
    uint32_t vaddr = (uint32_t)_vaddr;
    uint32_t page_phyaddr = (uint32_t)_page_phyaddr;
    uint32_t *pde = pde_ptr(vaddr);
    uint32_t *pte = pte_ptr(vaddr);
    
    if(*pde & PG_P_1){ //该页目录项已存在
        if((*pte & PG_P_1) == 0){ //该页表项不存在
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        }
        else {
            put_str("pte repeat\n", 0x07);
            while(1);
        }
    }
    else {
        uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool);
        *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        memset((void *)((int)pte & 0xfffff000), 0, PG_SIZE);//pde_phyaddr对应的物理内存页清空
        if((*pte & PG_P_1) == 0){ //该页表项不存在
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        }
        else {
            put_str("pte repeat\n", 0x07);
            while(1);
        }
    }
}

/**
 * 在pf对应的内存池中分配cnt个页
 * @return 成功：返回起始虚拟地址；失败：返回NULL 
 */
void* malloc_page(enum pool_flag pf, uint32_t cnt) {
    if(!(cnt > 0 && cnt < 3840)) return NULL; //一个内存池16MB，15MB=3840页，限制其不超过3840页
    
    //申请虚拟地址
    void* vaddr_start = vaddr_get(pf, cnt);
    if (vaddr_start == NULL) return NULL;

    //从内存池中申请物理页
    uint32_t vaddr = (uint32_t)vaddr_start;
    struct pool* p = (pf == PF_KERNEL) ? &kernel_pool : &user_pool;

    for(int i = 0; i < cnt; i ++){
        void* page_phyaddr = palloc(p);
        if (page_phyaddr == NULL) return NULL;
        page_table_add((void*)vaddr, page_phyaddr); 
        vaddr += PG_SIZE;
    }
    return vaddr_start;
}

/**
 * 从内核内存池中申请cnt页
 * @return 成功：返回虚拟地址；失败：返回NULL
 */
void* get_kernel_pages(uint32_t cnt) {
    put_str("\nget_kernel_pages start\n", 0x07);
    void* vaddr =  malloc_page(PF_KERNEL, cnt);
    if (vaddr != NULL) {
        memset(vaddr, 0, cnt * PG_SIZE);
    }
    return vaddr;
}

/**
 * 初始化内存池
 */ 
void mem_pool_init(uint32_t all_mem){
    put_str("\nmemory pool init start\n", 0x07);
    uint32_t page_table_size = 256 * PG_SIZE; //页目录表 +　页表　共占用的空间
    uint32_t used_mem = page_table_size + 0x100000; //　页目录表＋页表＋低端１MB
    uint32_t free_mem = all_mem - used_mem;
    uint16_t all_free_page = free_mem / PG_SIZE;
    uint16_t kernel_free_page = all_free_page / 2; //物理内存池划分为两个等大的内存池
    uint16_t user_free_page = all_free_page - kernel_free_page;
 
    uint32_t kernel_bitmap_len = kernel_free_page / 8; //bitmap中1位表示1页
    uint32_t user_bitmap_len = user_free_page / 8;

    uint32_t kernel_pool_start = used_mem;
    uint32_t user_pool_start = kernel_pool_start + kernel_free_page * PG_SIZE;

    //内核内存池、用户内存池
    kernel_pool.phy_addr_start = kernel_pool_start;
    user_pool.phy_addr_start = user_pool_start;

    kernel_pool.pool_size = kernel_free_page * PG_SIZE;
    user_pool.pool_size = user_free_page * PG_SIZE;

    kernel_pool.pool_bitmap.len = kernel_bitmap_len;
    user_pool.pool_bitmap.len = user_bitmap_len;

    kernel_pool.pool_bitmap.bits = (void *) MEM_BITMAP_BASE;
    user_pool.pool_bitmap.bits = (void *)(MEM_BITMAP_BASE + kernel_bitmap_len);

    bitmap_init(&kernel_pool.pool_bitmap);
    bitmap_init(&user_pool.pool_bitmap);

    put_uint(MEM_BITMAP_BASE, 16, 0x07);
    put_char('\n', 0x07);
    put_uint(MEM_BITMAP_BASE, 10, 0x07);
    put_str("      kernel_pool_bitmap_start:", 0x07); put_uint((int)kernel_pool.pool_bitmap.bits, 16, 0x07);
    put_str(" kernel_pool_phy_addr_start:", 0x07); put_int(kernel_pool.phy_addr_start, 16, 0x07);
    put_str("\n", 0x07);
    put_str("      user_pool_bitmap_start:", 0x07); put_uint((int)user_pool.pool_bitmap.bits, 16, 0x07);
    put_str(" user_pool_phy_addr_start:", 0x07); put_int(user_pool.phy_addr_start, 16, 0x07);
    put_str("\n", 0x07);

    //内核虚拟地址(堆)
    kernel_vaddr.vaddr_bitmap.len = kernel_bitmap_len;
    kernel_vaddr.vaddr_bitmap.bits = (void *)(MEM_BITMAP_BASE + kernel_bitmap_len + user_bitmap_len);
    kernel_vaddr.vaddr_start = KERNEL_HEAP_START;
    bitmap_init(&kernel_vaddr.vaddr_bitmap);
    
    put_uint(MEM_BITMAP_BASE, 16, 0x07);
    put_char('\n', 0x07);
    put_uint(MEM_BITMAP_BASE, 10, 0x07);
    put_str("kernel_vaddr.vaddr_bitmap.start:", 0x07); put_uint((int)kernel_vaddr.vaddr_bitmap.bits, 16, 0x07);
    put_str("\n", 0x07);
    put_str("kernel_vaddr.vaddr_start:", 0x07); put_uint((int)kernel_vaddr.vaddr_start, 16, 0x07);
    put_str("\n", 0x07);

    put_str("\nmem pool init done\n", 0x07);    
}

void mem_init() {
    mem_pool_init(0x2000000); //32MB
}

