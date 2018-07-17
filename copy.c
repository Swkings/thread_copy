#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>//输出文件信息
#include<sys/stat.h>//判断是否目录

/* stat 结构体包含的信息，我们用到st_mode判断文件是文件夹还是单一文件
 *  struct stat {
 *     dev_t     st_dev;         // 包含这个文件的设备 ID
 *     ino_t     st_ino;         // inode 编号
 *     mode_t    st_mode;        // 访问权限
 *     nlink_t   st_nlink;       // 硬链接数量
 *     uid_t     st_uid;         // 用户ID
 *     gid_t     st_gid;         // 组ID
 *     dev_t     st_rdev;        // 设备ID
 *     off_t     st_size;        // 文件占用的字节数
 *     blksize_t st_blksize;     // 文件系统块大小
 *     blkcnt_t  st_blocks;      // 文件占用了几个512字节
 *     time_t    st_atime;       // 最后访问时间
 *     time_t    st_mtime;       // 最后更改时间
 *     time_t    st_ctime;       // 最后状态更改时间
 * };
 */

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
/*字符串处理函数*/
int endwith(char* s,char c) //用于判断字符串结尾是否为“/”，主要是判断路径，有的路径后以“/”结尾，有些不是，而且拼接路径需要判断是否需要加“/”
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
/*主要用于拼接路径*/
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
/*复制函数*/
void copy_file(char* source_path,char *destination_path) //复制文件
{
    char buffer[1024];
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
void copy_folder(char* source_path,char *destination_path) //复制文件夹
{
    if(!opendir(destination_path))
    {
        if (mkdir(destination_path,0777))//如果不存在就用mkdir函数来创建
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
        if(!endwith(source_path,'/'))
        {
            file_source_path=str_contact(file_source_path,source_path);
            file_source_path=str_contact(source_path,"/");
        }
        else
        {
            file_source_path=str_contact(file_source_path,source_path);
        }
        char *file_destination_path;
        file_destination_path=(char*)malloc(512);
        if(!endwith(destination_path,'/'))
        {
            file_destination_path=str_contact(file_destination_path,destination_path);
            file_destination_path=str_contact(destination_path,"/");
        }
        else
        {
            file_destination_path=str_contact(file_destination_path,destination_path);
        }
        //取文件名与当前文件夹拼接成一个完整的路径
        file_source_path=str_contact(file_source_path,filename->d_name);
        file_destination_path=str_contact(file_destination_path,filename->d_name);
        if(is_dir(file_source_path)) //如果是目录
        {
            if(!endwith(file_source_path,'.')) //同时并不以.结尾，因为Linux在所有文件夹都有一个.文件夹用于连接上一级目录，必须剔除，否则进行递归的话，后果无法想象
            {
                copy_folder(file_source_path,file_destination_path);//进行递归调用，相当于进入这个文件夹进行复制～
            }
        }
        else
        {
            copy_file(file_source_path,file_destination_path);//否则按照单一文件的复制方法进行复制。
            printf("复制%s到%s成功！\n",file_source_path,file_destination_path);
        }
    }
}
/*主函数*/
int main(int argc,char *argv[])
{
    if(argv[1]==NULL||argv[1]==NULL)
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
        //对单个文件进行拷贝
        int len;
        char buf[1024];
        FILE *in,*out;
        in=fopen(argv[1],"r+");
        out=fopen(argv[2],"w+");
        while(len=fread(buf,1,sizeof(buf),in))
        {
            fwrite(buf,1,len,out);
        }
        printf("复制%s到%s成功！\n",argv[1],argv[2]);
        return 0;
    }
    copy_folder(source_path,destination_path);//进行文件夹的拷贝
    return 0;

}
