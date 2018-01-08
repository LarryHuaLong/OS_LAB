#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>

char file_type(mode_t st_mode)					//判断文件类型，并返回代表该文件类型的字母
{
	switch(st_mode & S_IFMT){
		case S_IFSOCK:return 's';
		case S_IFLNK:return 'l';
		case S_IFREG:return '-';
		case S_IFBLK:return 'b';
		case S_IFDIR:return 'd';
		case S_IFCHR:return 'c';
		case S_IFIFO:return 'P';
		default: return'?';
	}
}
int print_perm(mode_t st_mode)					//打印权限信息
{
	putchar(st_mode & S_IRUSR ? 'r':'-');		//owner读权限
	putchar(st_mode & S_IWUSR ? 'w':'-');		//owner写权限
	putchar(st_mode & S_IXUSR ? (st_mode & S_ISUID ? 's':'x') :(st_mode & S_ISUID ? 'S':'-'));//owner执行权限
	putchar(st_mode & S_IRGRP ? 'r':'-');		//group读权限
	putchar(st_mode & S_IWGRP ? 'w':'-');		//group写权限
	putchar(st_mode & S_IXGRP ? (st_mode & S_ISGID ? 's':'x') :(st_mode & S_ISGID ? 'S':'-'));//group执行权限
	putchar(st_mode & S_IROTH ? 'r':'-');		//others读权限
	putchar(st_mode & S_IWOTH ? 'w':'-');		//others写权限
	putchar(st_mode & S_IXOTH ? (st_mode & S_ISVTX ? 't':'x') :(st_mode & S_ISVTX ? 'T':'-'));//others执行权限
	return 0;
}
void printdir(char *dirpath,char *dirname)
{
	static char *months[] = {"Jan", "Feb", "Mar",
							"Apr", "May", "Jun",
							"Jul", "Aug", "Sep",
							"Oct", "Nov", "Dec"};
	DIR *p_dir;
	struct dirent *p_dirent;
	struct stat statbuf;
	struct passwd *p_passwd;
	struct group *p_group;
	struct tm *p_time;
	int pathlen = strlen(dirpath)+strlen(dirname)+2;
	char *path = (char*)malloc(pathlen);
	if(strlen(dirpath))
		sprintf(path,"%s/%s",dirpath,dirname);
	else
		sprintf(path,"%s",dirname);
	printf("%s:\n",path);						//显示当前遍历的路径名
	if(NULL == (p_dir = opendir(dirname))){		//打开文件目录
		perror("opendir");
		return;
	}
	if(-1 == chdir(dirname)){					//将dirname设置为当前目录
		perror("chdir");
		return;
	}
	int maxsize = 0;
	int total_512B_blocks = 0;
	while(NULL != (p_dirent = readdir(p_dir))){	//读取每个目录项，输出属性
		if('.' == p_dirent->d_name[0])
			continue;							//忽略 "."、".."和隐藏文件
		if(-1 == lstat(p_dirent->d_name,&statbuf)){	//获取当前目录项的属性
			perror(p_dirent->d_name);
			return;
		}
		if(statbuf.st_size > maxsize)
			maxsize = statbuf.st_size;
		total_512B_blocks += statbuf.st_blocks;
	}
	printf("total %d\n",total_512B_blocks / 2);	//显示当前目录包含的总块数
	int maxsizelen = 1;
	while(maxsize /= 10)
		maxsizelen++;
	rewinddir(p_dir);							//目录流定位到开始
	while(NULL != (p_dirent = readdir(p_dir))){	//读取每个目录项，输出属性
		if('.' == p_dirent->d_name[0])
			continue;							//忽略 "."、".."和隐藏文件
		if(-1 == lstat(p_dirent->d_name,&statbuf)){	//获取当前目录项的属性
			perror(p_dirent->d_name);
			return;
		}
		p_time = localtime(&statbuf.st_mtime);	//转换时间格式
		p_passwd = getpwuid(statbuf.st_uid);	//获取用户名
		p_group = getgrgid(statbuf.st_gid);		//获取用户组名
		putchar(file_type(statbuf.st_mode));	//输出文件类型
		print_perm(statbuf.st_mode);			//输出访问权限信息
		printf(" %d %s %s %*d %s %2d %02d:%02d %s",
				(int)statbuf.st_nlink,			//连接数
				p_passwd->pw_name,				//拥有者用户名
				p_group->gr_name,				//拥有组名
				maxsizelen,						//控制文件大小显示的长度
				(int)statbuf.st_size,			//文件大小
				months[p_time->tm_mon],			//最后一次修改时间
				p_time->tm_mday,
				p_time->tm_hour,
				p_time->tm_min,
				p_dirent->d_name);				//文件名
		if('l' == file_type(statbuf.st_mode)){	//如果是符号连接文件,则读取连接的内容并显示
			static char buf[256] = "";
			int len = 0;
			if(-1 == (len = readlink(p_dirent->d_name,buf,256))){
				perror("readlink");
				return;
			}
			printf(" -> %.*s\n",len,buf);
		}
		else
			putchar('\n');
	}
	rewinddir(p_dir);							//目录流定位到开始
	while(NULL != (p_dirent = readdir(p_dir))){	//读到一个目录项
		if('.' == p_dirent->d_name[0])
			continue;							//忽略 ".\"、"..\"和隐藏文件
		if(-1 == lstat(p_dirent->d_name,&statbuf)){
			perror(p_dirent->d_name);
			return;
		}
		if('d' == file_type(statbuf.st_mode)){	//如果是目录文件
			putchar('\n');						//空行
			printdir(path,p_dirent->d_name);	//递归调用printdir,打印子目录的信息
		}
	}
	if(-1 == chdir("..")){						//返回上级目录
		perror("chdir (\"..\")");
		return;
	}
	closedir(p_dir);
	return;
}
int main(){
	printdir("",".");							//遍历当前目录
	return 0;
}
