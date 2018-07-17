#include<unistd.h>  //sleep()  wite()
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>//输出文件信息
#include<sys/stat.h>//判断是否目录
#include<pthread.h>//使用线程

/*
linux中不带pthread库，所以编译时应如下在后加上 -lpthread
gcc thread.c -o thread -lpthread
*/
char *source_arr[512];//存放源文件路径的数组
char *destination_arr[512];//存放目标文件路径的数组
int source_arr_index=0;//存放源文件路径的数组的索引，就是for(int i=xx;..;..)那个i
int destination_arr_index=0;//存放目标文件路径的数组的索引
pthread_mutex_t mutex;//声明一个互斥锁mutex
int i=0;//多个线程函数用到这个i，用于记录是否复制完毕，因此作为全局变量处理～

/*字符串处理函数*/
int endwith(char* s,char c) //用于判断字符串结尾是否为“/”与“.”
{
    if(s[strlen(s)-1]==c)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
char* str_contact(char* str1,char* str2) //字符串连接
{
    char* result;
    result=(char*)malloc(strlen(str1)+strlen(str2)+1);//str1的长度+str2的长度+\0;
    if(!result) //如果内存动态分配失败
    {
        printf("字符串连接时，内存动态分配失败\n");
        exit(1);
    }
    strcat(result,str1);
    strcat(result,str2);//字符串拼接
    return result;
}

/*遍历函数*/
int is_dir(char* path) //判断是否是目录
{
    struct stat st;
    stat(path,&st);
    if(S_ISDIR(st.st_mode))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/*遍历source_path并且在destination_path中创建好对应的文件夹，并将需要复制的文件加到source_arr和destination_arr数组中，相当于生产多进程中的任务*/
void read_folder(char* source_path,char *destination_path) //复制文件夹
{
    if(!opendir(destination_path))
    {
        if (mkdir(destination_path,0777))//如果不存在就用mkdir函数来创建，0777表示读写以及执行权限
        {
            printf("创建文件夹失败！");
        }
    }
    char *path;
    path=(char*)malloc(512);//相当于其它语言的String path=""，纯C环境下的字符串必须自己管理大小，这里为path直接申请512的位置的空间，用于目录的拼接
    path=str_contact(path,source_path);//这三句，相当于path=source_path
    struct dirent* filename;
    DIR* dp=opendir(path);//用DIR指针指向这个文件夹
    while(filename=readdir(dp)) //遍历DIR指针指向的文件夹，也就是文件数组。
    {
        memset(path,0,sizeof(path));
        path=str_contact(path,source_path);
        //如果source_path,destination_path以路径分隔符结尾，那么source_path/,destination_path/直接作路径即可
        //否则要在source_path,destination_path后面补个路径分隔符再加文件名，谁知道你传递过来的参数是f:/a还是f:/a/啊？
        char *file_source_path;
        file_source_path=(char*)malloc(512);
        file_source_path=str_contact(file_source_path,source_path);
        if(!endwith(source_path,'/'))
        {
            file_source_path=str_contact(source_path,"/");
        }
        char *file_destination_path;
        file_destination_path=(char*)malloc(512);
        file_destination_path=str_contact(file_destination_path,destination_path);
        if(!endwith(destination_path,'/'))
        {
            file_destination_path=str_contact(destination_path,"/");
        }
        //取文件名与当前文件夹拼接成一个完整的路径
        file_source_path=str_contact(file_source_path,filename->d_name);

        if(is_dir(file_source_path)) //如果是目录
        {
            if(!endwith(file_source_path,'.')) //同时并不以.结尾，因为Linux在所有文件夹都有一个.文件夹用于连接上一级目录，必须剔除，否则进行递归的话，后果无法想象！
            {
                file_destination_path=str_contact(file_destination_path,filename->d_name);//对目标文件夹的处理，取文件名与当前文件夹拼接成一个完整的路径
                read_folder(file_source_path,file_destination_path);//进行递归调用，相当于进入这个文件夹进行遍历～
            }
        }
        else //否则，将源文件于目标文件的路径分别存入相关数组
        {
            //对目标文件夹的处理，取文件名与当前文件夹拼接成一个完整的路径
            //file_destination_path=str_contact(file_destination_path,"pre_");//给目标文件重命名，这里示意如何加个前缀～^_^
            file_destination_path=str_contact(file_destination_path,filename->d_name);
            source_arr[source_arr_index]=file_source_path;
            source_arr_index++;


            destination_arr[destination_arr_index]=file_destination_path;
            destination_arr_index++;

        }
    }
}

/*复制文件*/
void copy_file(char* source_path,char *destination_path) //复制文件
{
    char buffer[1024]; //缓冲区，先将文件数据复制到缓冲区，再从缓冲区复制到目标位置
    FILE *in,*out;//定义两个文件流，分别用于文件的读取和写入int len;
    if((in=fopen(source_path,"r"))==NULL) //打开源文件的文件流
    {
        printf("源文件打开失败！\n");
        exit(1);
    }
    if((out=fopen(destination_path,"w"))==NULL) //打开目标文件的文件流
    {
        printf("目标文件创建失败！\n");
        exit(1);
    }
    int len;//len为fread读到的字节长
    while((len=fread(buffer,1,1024,in))>0) //从源文件中读取数据并放到缓冲区中，第二个参数1也可以写成sizeof(char)
    {
        fwrite(buffer,1,len,out);//将缓冲区的数据写到目标文件中
    }
    fclose(out);
    fclose(in);
}

/*
 线程执行函数
 在read_folder函数中已经生成任务数组，线程的目标就是完成任务数组中的任务
*/
void *thread_function(void *arg)
{
    while(i<destination_arr_index)
    {
        if(pthread_mutex_lock(&mutex)!=0) //对互斥锁上锁，临界区开始
        {
            printf("%s的互斥锁创建失败！\n",(char *)arg);
            pthread_exit(NULL);
        }
        if(i<destination_arr_index)
        {
            copy_file(source_arr[i],destination_arr[i]);//复制单一文件
            printf("%s复制%s到%s成功！\n",(char *)arg,source_arr[i],destination_arr[i]);
            i++;
            sleep(1);//该线程挂起1秒
        }
        else //否则退出
        {
            pthread_exit(NULL);//退出线程
        }
        pthread_mutex_unlock(&mutex);//解锁，临界区结束
        sleep(1);//该线程挂起1秒
    }
    pthread_exit(NULL);//退出线程
}

/*主函数*/
int main(int argc,char *argv[])
{
    if(argv[1]==NULL||argv[2]==NULL)
    {
        printf("请输入两个文件夹路径，第一个为源，第二个为目的！\n");
        exit(1);
    }
    char* source_path=argv[1];//取用户输入的第一个参数
    char* destination_path=argv[2];//取用户输入的第二个参数
    DIR* source=opendir(source_path);
    DIR* destination=opendir(destination_path);
    if(!source||!destination)
    {
        printf("你输入的一个参数或者第二个参数不是文件夹！\n");
    }
    read_folder(source_path,destination_path);//进行文件夹的遍历
    /*线程并发开始*/
    pthread_mutex_init(&mutex,NULL);//初始化这个互斥锁
    //声明并创建三个线程
    pthread_t t1;
    pthread_t t2;
    pthread_t t3;
    if(pthread_create(&t1,NULL,thread_function,"线程1")!=0)
    {
        printf("创建线程失败！程序结束！\n");
        exit(1);
    }
    if(pthread_create(&t2,NULL,thread_function,"线程2")!=0)
    {
        printf("创建线程失败！程序结束！\n");
        exit(1);
    }
    if(pthread_create(&t3,NULL,thread_function,"线程3")!=0)
    {
        printf("创建线程失败！程序结束！\n");
        exit(1);
    }

    pthread_join(t1,NULL);
    pthread_join(t2,NULL);
    pthread_join(t3,NULL);
    //三个线程都完成才能执行以下的代码
    pthread_mutex_destroy(&mutex);//销毁这个互斥锁
    /*线程并发结束*/
    return 0;
}
