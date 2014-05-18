#ifndef _IO_H_
#define _IO_H_

void kprintf(int fd, const char *str);
int kwrite (int fd, char byte);
int kread (int fd, char *byte);
int kremove(const char *path);
int kcreate(const char *path);
int kopen(const char *path);
int kclose(int fd);
int mkdir (const char *dirname);
int rmdir (const char *dirname);
int kls(void);
int kpwd(void);
int kcd(const char *path);

#endif
