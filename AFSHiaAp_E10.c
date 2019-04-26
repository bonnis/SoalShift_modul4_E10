#define FUSE_USE_VERSION 28

#define _XOPEN_SOURCE 700

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

static const char *dirpath = "/home/bonnis/shift4";
static const char *charlist= "qE1~ YMUR2\"`hNIdPzi%%^t@(Ao:=CQ,nx4S[7mHFye#aT6+v)DfKL$r?bkOGB>}!9_wV']jcp5JZ&Xl|\\8s;g<{3.u*W-0";
static const int key=17;

int search(char c)
{
    int i, n=strlen(charlist);
    for(i=0;i<n;i++)
    {
        if (c==charlist[i])
        {
            return i;
        }
    }
    return 0;
}

void enkripsi(char* stringn, int shift)
{
    int i=0;
    while(stringn[i]!='\0')
    {
        if(stringn[i]=='/') 
		{
			i++;
			continue;
		}
        stringn[i] = charlist[(search(stringn[i])+shift)%strlen(charlist)];
		i++;
    }
}

void dekripsi(char* stringn, int shift)
{
    int i=0;
    while(stringn[i]!='\0')
    {
        if(stringn[i]=='/') 
		{
			i++;
			continue;
		}
		int temp = search(stringn[i])-shift;
		int res = temp;
		if(temp<0)
		{
			temp=temp*-1;
			res = strlen(charlist)-temp;
		}
        stringn[i] = charlist[res];
		i++;
    }
}

static int xmp_create(const char *path, mode_t mode,
                      struct fuse_file_info *fi)
{
        int res;
        char fpath[1000];
		char temp[100];
		strcpy(temp,path);
		enkripsi(temp,key);
	    sprintf(fpath,"%s%s",dirpath,temp);
        res = open(fpath, fi->flags, mode);
        if (res == -1)
                return -errno;
        fi->fh = res;
        return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
        int res;
        char fpath[1000];
		char temp[100];
		strcpy(temp,path);
		enkripsi(temp,key);
	    sprintf(fpath,"%s%s",dirpath,temp);
        res = mkdir(fpath, mode);
        if (res == -1)
                return -errno;
        return 0;
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
    int res;
	char fpath[1000];
	char temp[200];
	strcpy(temp,path);
	if(!strcmp(path,".")||!strcmp(path,".."));
	else enkripsi(temp,key);
	sprintf(fpath,"%s%s",dirpath,temp);
	res = lstat(fpath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		struct stat cek;
		struct group *grup;
		struct passwd *usr;

		char temp[1000]={0};
		strcpy(temp,de->d_name);
		char temp2[1000];
		if(!strcmp(de->d_name,".")||!strcmp(de->d_name,".."));
		else{

			memset(&cek, 0, sizeof(cek));
			sprintf(temp2, "%s/%s",fpath,de->d_name);
			stat(temp2,&cek);
			char temp3[100];
			strcpy(temp3,temp);
			grup = getgrgid(cek.st_gid); 
			usr = getpwuid(cek.st_uid);


			if(!strcmp(grup->gr_name,"rusak")&&(!strcmp(usr->pw_name,"chipset")||!strcmp(usr->pw_name,"ic_controller"))&&!(cek.st_mode & S_IROTH))
			{
				char holder[] = "/filemiris.txt";
				enkripsi(holder,key);
				dekripsi(temp3,key);
				char newstring[1000];
				sprintf(newstring,"%s%s",dirpath,holder);
				FILE* new = fopen(newstring,"a");
				fprintf(new,"%s %d %d %ld : %s\n", temp3, grup->gr_gid,usr->pw_gid, cek.st_atime, ctime(&cek.st_atime));
				fclose(new);
				remove(temp2);
				continue;
			}
		}

		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;

        if(!strcmp(de->d_name,".")||!strcmp(de->d_name,".."));
		else dekripsi(temp,key);

		res = (filler(buf, temp, &st, 0));
			if(res!=0) break;
	}

	closedir(dp);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	char fpath[1000];
	char temp[100];
	strcpy(temp,path);
	enkripsi(temp,key);
	sprintf(fpath,"%s%s",dirpath,temp);

	int fd;
	int res;
	if(fi == NULL)
			fd = open(fpath, O_RDONLY);
	else
			fd = fi->fh;
	
	if (fd == -1)
			return -errno;
	res = pread(fd, buf, size, offset);
	if (res == -1)
			res = -errno;
	if(fi == NULL)
			close(fd);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
	char fpath[1000];
	char temp[100];
	strcpy(temp,path);
	enkripsi(temp,key);
	sprintf(fpath,"%s%s",dirpath,temp);
	int fd;
	int res;
	(void) fi;
	if(fi == NULL)
			fd = open(fpath, O_WRONLY);
	else
			fd = fi->fh;
	
	if (fd == -1)
			return -errno;
	res = pwrite(fd, buf, size, offset);
	if (res == -1)
			res = -errno;
	if(fi == NULL)
			close(fd);
	return res;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	char fpath[1000];
	char temp[100];
	strcpy(temp,path);
	enkripsi(temp,key);
	sprintf(fpath,"%s%s",dirpath,temp);
	int res;
	res = open(fpath, fi->flags);
	if (res == -1)
			return -errno;
	fi->fh = res;
	return 0;
}

static int xmp_access(const char *path, int mask)
{
	char fpath[1000];
	char temp[100];
	strcpy(temp,path);
	enkripsi(temp,key);
	sprintf(fpath,"%s%s",dirpath,temp);
        int res;
        res = access(fpath, mask);
        if (res == -1)
                return -errno;
        return 0;
}

// #ifdef HAVE_UTIMENSAT
static int xmp_utimens(const char *path, const struct timespec ts[2],
                       struct fuse_file_info *fi)
{
        (void) fi;
        int res;
		char fpath[1000];
		char temp[100];
		strcpy(temp,path);
		enkripsi(temp,key);
		sprintf(fpath, "%s%s",dirpath,temp);
        /* don't use utime/utimes since they follow symlinks */
        res = utimensat(0, fpath, ts, AT_SYMLINK_NOFOLLOW);
        if (res == -1)
                return -errno;
        return 0;
}
// // #endif

static int xmp_rename(const char *from, const char *to, unsigned int flags)
{
        int res;
        if (flags)
                return -EINVAL;
		char tempf[100];
		char pathf[1000];
		strcpy(tempf,from);
		enkripsi(tempf,key);
		sprintf(pathf, "%s%s",dirpath,tempf);

		char tempt[100];
		char patht[1000];
		strcpy(tempt,to);
		enkripsi(tempt,key);
		sprintf(patht, "%s%s",dirpath,tempt);

        res = rename(pathf, patht);
        if (res == -1)
                return -errno;
        return 0;
}

static int xmp_chmod(const char *path, mode_t mode,
                     struct fuse_file_info *fi)
{
        (void) fi;
        int res;
		char fpath[1000];
		char temp[100];
		strcpy(temp,path);
		enkripsi(temp,key);
		sprintf(fpath, "%s%s",dirpath,temp);
        res = chmod(fpath, mode);
        if (res == -1)
                return -errno;
        return 0;
}
static int xmp_chown(const char *path, uid_t uid, gid_t gid,
                     struct fuse_file_info *fi)
{
        (void) fi;
        int res;
		char fpath[1000];
		char temp[100];
		strcpy(temp,path);
		enkripsi(temp,key);
		sprintf(fpath, "%s%s",dirpath,temp);
        res = lchown(fpath, uid, gid);
        if (res == -1)
                return -errno;
        return 0;
}

static struct fuse_operations xmp_oper = {
	.utimens	= xmp_utimens,
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
    .create     = xmp_create,
	.rename		= xmp_rename,
    .mkdir      = xmp_mkdir,
	.chmod      = xmp_chmod,
    .chown      = xmp_chown,
	.access     = xmp_access,
	.open       = xmp_open,
    .write      = xmp_write,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}