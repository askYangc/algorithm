#ifndef __DEAD_LOCK_STUB_H__
#define __DEAD_LOCK_STUB_H__

#include <stdio.h>
#include <stdint.h>

#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "stlc_list.h"

#include <deque>
#include <vector>
#include <map>


/* use this need #include "dead_lock_stub.h" and add code DeadLockGraphic::getInstance().start_check(); */


#ifndef COLOR_END

#define	COLOR_RED	"\033[31m\033[1m"
#define	COLOR_GREEN	"\033[32m\033[1m"
#define	COLOR_YELLOW	"\033[33m\033[1m"
#define	COLOR_END	"\033[0m"

#endif

#define WAIT_MUTEXLOCK_TIMEOUT 5

struct thread_graphic_vertex_t {
    int indegress;
    std::vector<uint64_t> vertexs;
    
    thread_graphic_vertex_t()
        : indegress(0) {
    }
};

class Thread_req_lock 
{
public:
    Thread_req_lock(){}
    Thread_req_lock(uint64_t lock_addr, const char *file, int line, const char *funcname, const char *lock_name) {
        this->lock_addr = lock_addr;
        this->line = line;
        this->t = time(NULL);
        memcpy(this->file, file, sizeof(this->file) - 1);
        memcpy(this->funcname, funcname, sizeof(this->funcname) - 1);
        memcpy(this->lock_name, lock_name, sizeof(this->lock_name) - 1);
    }  
    
    Thread_req_lock& operator=(const Thread_req_lock &b) {
        this->lock_addr = b.lock_addr;
        this->line = b.line;
        this->t = b.t;
        memcpy(this->file, b.file, sizeof(this->file) - 1);
        memcpy(this->lock_name, b.lock_name, sizeof(this->lock_name) - 1);
        memcpy(this->funcname, b.funcname, sizeof(this->funcname) - 1);
    }
    
    void set_req_lock(uint64_t lock_addr, const char *file, int line, const char *funcname, const char *lock_name) {
        this->lock_addr = lock_addr;
        this->line = line;
        this->t = time(NULL);
        memset(this->file, 0, sizeof(this->file));
        memset(this->funcname, 0, sizeof(this->file));
        memset(this->lock_name, 0, sizeof(this->file));
        
        memcpy(this->file, file, sizeof(this->file) - 1);
        memcpy(this->funcname, funcname, sizeof(this->funcname) - 1);
        memcpy(this->lock_name, lock_name, sizeof(this->lock_name) - 1);        
    }
    
    uint64_t get_lock_addr() const {
        return this->lock_addr;
    }

    int time_2_str(char *s, int len,  time_t t) const{
        struct tm tm;

        localtime_r (&t, &tm);
        return strftime (s, len, "%y-%m-%d %T", &tm);
    }
    
    void thread_req_lock_show(uint64_t thread_id) const{   
        char _buf[32];
        time_2_str(_buf, sizeof(_buf), this->t);
        printf("%llu %swait lock%s %s%s%s(%llu) at: file %s%s%s, function %s%s%s, line %s%d%s, start_time: %s (%u)\n", thread_id, COLOR_YELLOW, COLOR_END, COLOR_RED, this->lock_name, COLOR_END, 
        this->lock_addr, COLOR_GREEN, this->file, COLOR_END, COLOR_GREEN, this->funcname, COLOR_END, COLOR_GREEN, this->line, COLOR_END, _buf, this->t);
    }    
    
    uint32_t get_wait_time() const {
        return this->t;
    }
    
    uint8_t is_valid() const {
        return this->valid;
    }
    
    void set_valid(uint8_t valid) {
        this->valid = valid;
    }
    
 
private:
    uint8_t valid;
    uint32_t t;
    uint64_t lock_addr;
    char lock_name[20];
    char file[20];
    int line;
    char funcname[20];    
};

typedef struct {
    struct stlc_list_head link;
    char file[20];
    int line;
    char funcname[20];   
}hold_lock_point_t;

static hold_lock_point_t *hold_lock_point_init(const char *file, int line, const char *funcname)
{
    hold_lock_point_t *p = (hold_lock_point_t*)calloc(sizeof(hold_lock_point_t), 1);
    if(p) {
        p->line = line;
        memcpy(p->file, file, sizeof(p->file) - 1);
        memcpy(p->funcname, funcname, sizeof(p->funcname) - 1);   
    }

    return p;
}

static void hold_lock_point_free(hold_lock_point_t *p)
{
    if(p) {
        free(p);
    }
}

static void hold_lock_point_all_free(struct stlc_list_head *head)
{
    hold_lock_point_t *pos, *n;
    if(head) {
        stlc_list_for_each_entry_safe(pos, n, head, link) {
            stlc_list_del(&pos->link);
            hold_lock_point_free(pos);
        }
    }
}

static void hold_lock_point_list_cp(const struct stlc_list_head *old, struct stlc_list_head *newx)
{
    hold_lock_point_t *pos, *n;
    if(old && newx) {
        stlc_list_for_each_entry(pos, old, link) {
            n = hold_lock_point_init(pos->file, pos->line, pos->funcname);
            if(n) {
                stlc_list_add(&n->link, newx);
            }
        }
    }
}

class Thread_hold_lock 
{
public:
    Thread_hold_lock()
        : count(0),thread_id(0) {
            STLC_INIT_LIST_HEAD(&this->point_list);
            memset(this->lock_name, 0, sizeof(this->lock_name));
    }  
    
    Thread_hold_lock(int count, uint64_t thread_id, const char *lock_name) {
        this->count = count;
        this->thread_id = thread_id;
        memcpy(this->lock_name, lock_name, sizeof(this->lock_name) - 1);
        STLC_INIT_LIST_HEAD(&this->point_list);
    }  
    
    Thread_hold_lock(const Thread_hold_lock &b) {
        this->count = b.count;
        this->thread_id = b.thread_id;
        memcpy(this->lock_name, b.lock_name, sizeof(this->lock_name) - 1);
        STLC_INIT_LIST_HEAD(&this->point_list);
    }
    
    Thread_hold_lock& operator=(const Thread_hold_lock &b) {
        this->count = b.count;
        this->thread_id = b.thread_id;
        STLC_INIT_LIST_HEAD(&this->point_list);
        hold_lock_point_list_cp(&b.point_list, &this->point_list);
    }   
    
    ~Thread_hold_lock() {
        hold_lock_point_all_free(&this->point_list);
    }
    
    void set_hold_lock(uint64_t thread_id, const char *lock_name) {
        this->thread_id = thread_id;
        memset(this->lock_name, 0, sizeof(this->lock_name));
        memcpy(this->lock_name, lock_name, sizeof(this->lock_name) - 1);
    }
    
    void putStack(const char *file, int line, const char *funcname) {
        hold_lock_point_t *p = hold_lock_point_init(file, line, funcname);
        if(p) {
            stlc_list_add(&p->link, &this->point_list);
            this->count++;
        }
    }
    
    void popStack() {
        if(!stlc_list_empty(&this->point_list)) {
            hold_lock_point_t *p = stlc_list_entry(this->point_list.next, hold_lock_point_t, link);
            stlc_list_del(&p->link);
            hold_lock_point_free(p);
            this->count--;
        }
    }
    
    void thread_hold_lock_show(uint64_t local_addr) const{
        hold_lock_point_t *p;
        printf("%sthread %llu %s %shold lock_name(lock_addr)%s: %s%s%s(%llu), count: %s%d%s\n", COLOR_YELLOW, this->thread_id, COLOR_END, COLOR_GREEN, COLOR_END, COLOR_RED, this->lock_name, COLOR_END, local_addr, COLOR_GREEN, this->count, COLOR_END);
        stlc_list_for_each_entry(p, &this->point_list, link) {
            printf("%s  lock at%s: file %s%s%s, function %s%s%s, line %s%d%s\n", COLOR_YELLOW, COLOR_END, COLOR_GREEN, p->file, COLOR_END, COLOR_GREEN, p->funcname, COLOR_END, COLOR_GREEN, p->line, COLOR_END);
        }
    } 
 
public:
    int count;
    uint64_t thread_id; 
    char lock_name[20];
    
private:
    struct stlc_list_head point_list;   //hold_lock_point_t
};

class DeadLockGraphic {

public:
    static DeadLockGraphic &getInstance() {
        static DeadLockGraphic instance;
        return instance;
    }

    void lock_before(uint64_t thread_id, uint64_t lock_addr, const char *file, int line, const char *funcname, const char *lock_name) {
        pthread_mutex_lock(&m_mutex);
        // (A) m_thread_apply_lock, 添加 thread_id => lock_addr
        if(m_thread_apply_lock.find(thread_id) == m_thread_apply_lock.end()) {
            m_thread_apply_lock[thread_id] = Thread_req_lock(lock_addr, file, line, funcname, lock_name);
        }else {
            m_thread_apply_lock[thread_id].set_req_lock(lock_addr, file, line, funcname, lock_name);
        }
        m_thread_apply_lock[thread_id].set_valid(1);
        
        pthread_mutex_unlock(&m_mutex);
    }

    void lock_after(uint64_t thread_id, uint64_t lock_addr, const char *file, int line, const char *funcname, const char *lock_name) {
        pthread_mutex_lock(&m_mutex);
        // (B)m_thread_apply_lock, 去除 thread_id => lock_addr
        //m_thread_apply_lock.erase(thread_id);
        m_thread_apply_lock[thread_id].set_valid(0);
        
        // (A)m_lock_belong_thread, add lock_addr => thread_id
        if ( m_lock_belong_thread.find(lock_addr) == m_lock_belong_thread.end()) {
            m_lock_belong_thread.insert(std::map<uint64_t, Thread_hold_lock> :: value_type(lock_addr, Thread_hold_lock(0, thread_id, lock_name)));
            //m_lock_belong_thread[lock_addr] = Thread_hold_lock(0, thread_id);  
        }else {
            m_lock_belong_thread[lock_addr].set_hold_lock(thread_id, lock_name);
        }
        
        m_lock_belong_thread[lock_addr].putStack(file, line, funcname);
        
        //m_lock_belong_thread[lock_addr] = thread_id;
        pthread_mutex_unlock(&m_mutex);
    }
    
    void unlock_after(uint64_t thread_id, uint64_t lock_addr) {
        pthread_mutex_lock(&m_mutex);
        // (B)m_lock_belong_thread, remove lock_addr => thread_id
        m_lock_belong_thread[lock_addr].popStack();
#if 0        
        if(m_lock_belong_thread[lock_addr].count == 0){
            m_lock_belong_thread.erase(lock_addr);
        }
#endif     
        //m_lock_belong_thread.erase(lock_addr);
        pthread_mutex_unlock(&m_mutex);
    }
    
    void show_waitout_lock(const Thread_req_lock &rl, uint64_t thread_id) {
        printf("%sthread %u is waitting lock%s\n", COLOR_YELLOW, thread_id, COLOR_END);
        rl.thread_req_lock_show(thread_id);
    }
    
    void check_wait_lock_time() {
        uint32_t t = time(NULL);
        for ( std::map<uint64_t, Thread_req_lock>::const_iterator iter = m_thread_apply_lock.begin(); 
                iter != m_thread_apply_lock.end(); iter++  ) {
            const Thread_req_lock &rl = iter->second;
            if(rl.is_valid() == 0) {
                continue;
            }
            if(t - rl.get_wait_time() > WAIT_MUTEXLOCK_TIMEOUT) {
                show_waitout_lock(rl, iter->first);
                if (m_lock_belong_thread.find(rl.get_lock_addr()) == m_lock_belong_thread.end() ) {
                    continue;
                }
                const Thread_hold_lock &hl = m_lock_belong_thread[rl.get_lock_addr()];

                hl.thread_hold_lock_show(rl.get_lock_addr());
            }
        }
    }

    void show_dead_lock_infomation(std::map<uint64_t, thread_graphic_vertex_t> &graphics) 
    {
        for ( std::map<uint64_t, thread_graphic_vertex_t>::const_iterator iter = graphics.begin(); iter != graphics.end(); iter++ ) {
            const thread_graphic_vertex_t &gvert = iter->second;
            if(gvert.indegress != 0) {
                uint64_t thd_id = iter->first;
                printf("=====================thread info=====================\n");
                printf("thd_id: %s%llu%s, indegress: %d\n", COLOR_RED, thd_id, COLOR_END, gvert.indegress);
                printf("wait lock:\n");
                if(m_thread_apply_lock.find(thd_id) == m_thread_apply_lock.end()) {
                    printf("%snot find %llu in m_thread_apply_lock, it's wrong%s\n", COLOR_RED, thd_id, COLOR_END);
                    continue;
                }
                m_thread_apply_lock[thd_id].thread_req_lock_show(thd_id);
                
             
                printf("\nwait these threads unlock:%s\n", COLOR_YELLOW);
                for ( size_t i = 0; i < gvert.vertexs.size(); i++ ) {
                    printf("%u ", gvert.vertexs[i]);
                }
                printf("%s\n", COLOR_END);   
                printf("hold these locks:\n");                      
                for ( std::map<uint64_t, Thread_hold_lock>::const_iterator iter = m_lock_belong_thread.begin(); 
                    iter != m_lock_belong_thread.end(); iter++  ) {  
                    const Thread_hold_lock &hl = iter->second;
                    if(hl.thread_id == thd_id) {
                        hl.thread_hold_lock_show(iter->first);
                    }
                } 
                printf("=====================end=====================\n\n");
            }
        }        
    }
    
    void check_dead_lock()
    {
        pthread_mutex_lock(&m_mutex);

        // 构建有向图
        std::map<uint64_t, thread_graphic_vertex_t> graphics;
        for ( std::map<uint64_t, Thread_req_lock>::const_iterator iter = m_thread_apply_lock.begin(); 
                iter != m_thread_apply_lock.end(); iter++  ) {
            uint64_t thd_id1 = iter->first;
            const Thread_req_lock &lo = iter->second;
            if(!lo.is_valid()) {
                continue;
            }
            uint64_t lock_id = lo.get_lock_addr(); 
            if ( m_lock_belong_thread.find(lock_id) == m_lock_belong_thread.end() ) {
                continue;
            }           
            
            if(m_lock_belong_thread[lock_id].count == 0) {
                continue;
            }
            //uint64_t thd_id2 = m_lock_belong_thread[lock_id];
            uint64_t thd_id2 = m_lock_belong_thread[lock_id].thread_id;
            
            if ( graphics.find(thd_id1) == graphics.end() ) {
                graphics[thd_id1] = thread_graphic_vertex_t();
            }
            if ( graphics.find(thd_id2) == graphics.end() ) {
                graphics[thd_id2] = thread_graphic_vertex_t();
            }

            // 保存有向边
            graphics[thd_id1].vertexs.push_back(thd_id2);
            // 入度 indegress++
            graphics[thd_id2].indegress++;
        }

        // 检测流程一
        uint64_t counter = 0;
        std::deque<uint64_t> graphics_queue;
        for ( std::map<uint64_t, thread_graphic_vertex_t>::const_iterator iter = graphics.begin();
                iter != graphics.end(); iter++ ) {
            uint64_t thd_id = iter->first;
            const thread_graphic_vertex_t &gvert = iter->second;
            if ( gvert.indegress == 0 ) {
                graphics_queue.push_back(thd_id);
                counter ++;
            }
        }
            
        // 检测流程二
        while ( !graphics_queue.empty() ) {
            uint64_t thd_id = graphics_queue.front();
            graphics_queue.pop_front();

            const thread_graphic_vertex_t &gvert = graphics[thd_id];
            // 遍历邻近有向边
            for ( size_t i = 0; i < gvert.vertexs.size(); i++ ) {
                uint64_t thd_id2 = gvert.vertexs[i];
                graphics[thd_id2].indegress --;
                if ( graphics[thd_id2].indegress == 0 ) {
                    graphics_queue.push_back(thd_id2);
                    counter++;
                }
            }
        }

        // 检测流程三
        if ( counter != graphics.size() ) {
            printf("%sFound Dead Lock!!!!!!!!!!!!%s\n", COLOR_RED, COLOR_END);
            show_dead_lock_infomation(graphics);
        } else {
            printf("No Found Dead Lock.\n");
            check_wait_lock_time();
        }   
        
        pthread_mutex_unlock(&m_mutex);
    }

    void start_check() {
        pthread_t tid;
        pthread_create(&tid, NULL, thread_rountine, (void *)(this));
    }

    static void *thread_rountine(void *args) {
        DeadLockGraphic *ptr_graphics = static_cast<DeadLockGraphic *>(args);
        while ( true ) {
            // 每十秒检测一次
            sleep(5);
            ptr_graphics->check_dead_lock();
        }
    }

private:
    
    // lock 对应 线程 拥有者的map
    std::map<uint64_t, Thread_hold_lock> m_lock_belong_thread;

    // 线程尝试去申请的lock map
    std::map<uint64_t, Thread_req_lock> m_thread_apply_lock; 

    pthread_mutex_t m_mutex;


private:
    DeadLockGraphic() {
        pthread_mutex_init(&m_mutex, NULL);
    }
    ~DeadLockGraphic() {
        pthread_mutex_destroy(&m_mutex);
    }
private:
    DeadLockGraphic(const DeadLockGraphic &) {
    }
    DeadLockGraphic& operator=(const DeadLockGraphic &) {
        return *this;    
    }

private:
    

};

#include <sys/syscall.h>

#define gettid() syscall(__NR_gettid)

// 拦截lock, 添加before, after操作, 记录锁与线程的关系
#define pthread_mutex_lock(x)                                                                       \
    do {                                                                                            \
        DeadLockGraphic::getInstance().lock_before(gettid(), reinterpret_cast<uint64_t>(x), __FILE__, __LINE__, __FUNCTION__, #x);        \
        pthread_mutex_lock(x);                                                                      \
        DeadLockGraphic::getInstance().lock_after(gettid(), reinterpret_cast<uint64_t>(x), __FILE__, __LINE__, __FUNCTION__, #x);         \
    } while (false);

// 拦截unlock, 添加after操作, 解除锁和线程的关系
#define pthread_mutex_unlock(x)                                                                     \
    do {                                                                                            \
        pthread_mutex_unlock(x);                                                                    \
        DeadLockGraphic::getInstance().unlock_after(gettid(), reinterpret_cast<uint64_t>(x));       \
    } while(false);



#endif
