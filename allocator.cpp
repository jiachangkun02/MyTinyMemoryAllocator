#include<iostream>
#include<unistd.h>
#include<mutex>
#include<string.h>
using namespace std;

typedef char ALIGN[16];
class header
{
public:
        size_t size;
        unsigned is_free;//0表示内存块已经被使用了 1表示未被占用
        header* next;   
    
    ALIGN adress;
private:
     header(){}
     ~header(){}

};

typedef header header_t;

header_t *tail,*head;

static pthread_mutex_t global_malloc_lock;

header_t* get_free_block(size_t size)
{
    header_t* curr=head;
    while (curr)
    {
        if (curr->is_free && curr->size>=size)
        {
            return curr;
        }
        else
        {
            curr=curr->next;
        }
        
    }
    return NULL;
}


void *my_malloc(size_t size)
{
    void* block;
    header_t* fun_head;
    size_t total_size;
    if (!size)
    {
        return NULL;
    }
    pthread_mutex_lock(&global_malloc_lock);
    fun_head=(header_t*)get_free_block(size);
    if (fun_head)
    {
        fun_head->is_free=0;
        
        pthread_mutex_unlock(&global_malloc_lock);
        return (void*)(fun_head+1);
    }
    total_size =sizeof(header_t)+size;
    block= sbrk(total_size);
    if (!block)
    {
        cout<<"开辟空间失败"<<endl;
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }
    fun_head=(header_t*)block;
    fun_head->is_free=0;
    fun_head->size=size;
    fun_head->next=NULL;
    if(!head)
        head=fun_head;
    if(tail)
        tail->next=fun_head;
    tail=fun_head;
    pthread_mutex_unlock(&global_malloc_lock);
    cout<<"已经开辟好空间了"<<endl;
    return (void*)(fun_head+1);
    
}

void my_free(void* block)
{
    header_t *header,*tmp;
    void *progmabreak;
    if (block==NULL)
    {
        return;
    }
    pthread_mutex_lock(&global_malloc_lock);
    progmabreak=sbrk(0);//获取当前堆的结束地址
    header=(header_t*)block-1;
    if((char*)block+header->size==progmabreak)
    {
        if (head==tail)
        {
            head=tail=NULL;
        }
        else{
            tmp=head;
            while (head)
            {
                if (tmp->next==tail)
                {
                    tmp->next=NULL;
                    tail=tmp;
                }
                tmp=tmp->next;
            }
            
        }
        sbrk(-(sizeof(header_t)+header->size));
        pthread_mutex_unlock(&global_malloc_lock); 
        return;
        
    }
    //若不在堆尾部,直接释放
    head->is_free=1;
    pthread_mutex_unlock(&global_malloc_lock);
    cout<<"释放成功"<<endl;
} 

void *my_realloc(void *block,size_t size)
{
    header_t *header;
	void *ret;
	if (!block || !size)
		return malloc(size);
	header = (header_t*)block - 1;
	if (header->size >= size){
        cout<<"当前内存大于所给内存"<<endl;
		return block;
    }
	ret = malloc(size);
	
    if (!ret) {
        cout<<"内存重新分配失败"<<endl;
        return NULL;
    }
    memcpy(ret, block, header->size);
    my_free(block);
    cout<<"已经重新开辟好了"<<endl;
	return ret;
}



void test()
{
    int* a= (int*)my_malloc(5*sizeof(int));
    
    my_realloc(a,3);
    a=(int*)my_realloc(a,25);
    my_free(a);
}

// int main()
// {
//     test();
//     return 0;
// }