
/*!
  \file
  INI 解析

  提供给内核模块使用.

*/


//#include "vns_sys.h"
//#include "vns_config.h"



//#define MCFG_DBG_ON

#ifndef MCFG_DBG_ON
#define mcfg_pr(...) do{}while(0)
#endif
//#define mcfg_malloc(len) 	kmalloc(len,GFP_KERNEL)
//#define mcfg_free(p) 		kfree(p)
#define mcfg_malloc(len) 	malloc(len)
#define mcfg_free(p) 		free(p)
#ifdef MCFG_DBG_ON
#define mcfg_pr printk
#endif

#ifdef __KERNEL__
#include "linux/ctype.h"
#define strtol simple_strtol


#define CUTOFF 8

static void shortsort(char *lo, char *hi, unsigned width, int (*comp)(const void *, const void *));
static void swap(char *p, char *q, unsigned int width);
void qsort(void *base, unsigned num, unsigned width, int (*comp)(const void *, const void *))
{
	char *lo, *hi;
	char *mid;
	char *loguy, *higuy;
	unsigned size;
	char *lostk[30], *histk[30];
	int stkptr;

	if (num < 2 || width == 0) return;
	stkptr = 0;

	lo = base;
	hi = (char *) base + width * (num - 1);

recurse:
	size = (hi - lo) / width + 1;

	if (size <= CUTOFF)
	{
		shortsort(lo, hi, width, comp);
	}
	else
	{
		mid = lo + (size / 2) * width;
		swap(mid, lo, width);

		loguy = lo;
		higuy = hi + width;

		for (;;)
		{
			do { loguy += width; } while (loguy <= hi && comp(loguy, lo) <= 0);
			do { higuy -= width; } while (higuy > lo && comp(higuy, lo) >= 0);
			if (higuy < loguy) break;
			swap(loguy, higuy, width);
		}

		swap(lo, higuy, width);

		if (higuy - 1 - lo >= hi - loguy)
		{
			if (lo + width < higuy)
			{
				lostk[stkptr] = lo;
				histk[stkptr] = higuy - width;
				++stkptr;
			}

			if (loguy < hi)
			{
				lo = loguy;
				goto recurse;
			}
		}
		else
		{
			if (loguy < hi)
			{
				lostk[stkptr] = loguy;
				histk[stkptr] = hi;
				++stkptr;
			}

			if (lo + width < higuy)
			{
				hi = higuy - width;
				goto recurse;
			}
		}
	}

	--stkptr;
	if (stkptr >= 0)
	{
		lo = lostk[stkptr];
		hi = histk[stkptr];
		goto recurse;
	}
	else
		return;
}

static void shortsort(char *lo, char *hi, unsigned width, int (*comp)(const void *, const void *))
{
	char *p, *max;

	while (hi > lo)
	{
		max = lo;
		for (p = lo+width; p <= hi; p += width) if (comp(p, max) > 0) max = p;
		swap(max, hi, width);
		hi -= width;
	}
}

static void swap(char *a, char *b, unsigned width)
{
	char tmp;

	if (a != b)
	{
		while (width--)
		{
			tmp = *a;
			*a++ = *b;
			*b++ = tmp;
		}
	}
}

#else
#include "ctype.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>
#include <stdio.h>
#include <string.h>
#include "vns_list.h"
#include "vns_config.h"
#endif

#ifndef isspace
//#define isspace(c)	(c == ' ' || c == '\t' || c == 10 || c == 13 || c == 0)
#endif





typedef struct{
	char*key;
	char*val;

	//struct list_head read_cb_head;
	//struct list_head write_cb_head;

	struct hlist_node hnode;

	mcfg_cb_t read_cb;
	mcfg_cb_t write_cb;
}entry_t;

// 默认16 个. 大于一半，且冲突后就扩大一倍.

typedef struct{
	int entry_num;
	int buddy_size;
	struct hlist_head *buddy;
	int tot_key_len;
	int tot_val_len;
}dict_t;


static unsigned int __hash33(char*s){
	unsigned int hash=0;
	char c;
	while((c=*s++)){
		if(c=='-'){
			return hash;
		}
		hash+=(hash<<5) + hash +(char)tolower((int)c);
	}
	return hash;
}


static dict_t*dict_new(void){
	dict_t*d;
	d=mcfg_malloc(sizeof(dict_t));
	if(!d){
		return NULL;
	}
	memset(d,0,sizeof(dict_t));
	d->buddy_size=16;
	d->buddy = mcfg_malloc(sizeof(struct hlist_head ) * d->buddy_size);
	if(!d->buddy){
		mcfg_free(d);
		return NULL;
	}
	memset(d->buddy,0,sizeof(struct hlist_head ) * d->buddy_size);
	return d;
}

static int dict_free(dict_t*d){
	int i;
	struct hlist_node*pos;
	entry_t*entry;

	for(i=0;i<d->buddy_size;i++){
		hlist_for_each(pos,&d->buddy[i]){
			entry=hlist_entry(pos,entry_t,hnode);
			if(entry->key){
				mcfg_free(entry->key);
			}
			if(entry->val){
				mcfg_free(entry->val);
			}
			mcfg_free(entry);
		}
	}
	mcfg_free(d->buddy);
	mcfg_free(d);
	return 0;
}

char*__mstrdup(char*s,int*size){
	char*p;
	int len=strlen(s);

	p=mcfg_malloc(len+1);
	if(!p){
		return NULL;
	}
	memcpy(p,s,len+1);
	*size=len+1;
	return p;
}

static entry_t*dict_get_entry(dict_t*d,char*key,int new,char*val){
	struct hlist_node*pos;
	struct hlist_node*n;
	entry_t*entry=NULL;
	entry_t*tmp_entry=NULL;
	unsigned int hash;
	int grow_flag=0;
	int i;
	int len;
	char*find;

	//mcfg_pr("d=[%p]  key=[%s]  new=[%d]  val=[%p]\n",d,key,new,val);

	hash=__hash33(key);
	hash%=d->buddy_size;
	find=strchr(key,'-');

	//printf("key=%s,hash=%d\n",key,hash);

	hlist_for_each(pos, &d->buddy[hash]){
		entry=hlist_entry(pos,entry_t,hnode);
		if(find){
			if(strncasecmp(key,entry->key,find-key)==0){
				break;
			}
		}
		else{
			if(strcasecmp(key,entry->key)==0){
				break;
			}
		}
		entry=NULL;
	}


	if(!entry && new){
		len=sizeof(entry_t);
		entry=mcfg_malloc(len);
		if(!entry){
			return NULL;
		}
		memset(entry,0,len);

		if(!hlist_empty(&d->buddy[hash]) ){
			if((d->entry_num<<2)>d->buddy_size){
				grow_flag=1;
			}
		}

		entry->key=__mstrdup(key,&len);
		if(!entry->key){
			mcfg_free(entry);
			return NULL;
		}

		d->tot_key_len +=len+5;

		hlist_add_head(&entry->hnode,&d->buddy[hash]);
		d->entry_num++;

		//动态增长.
		if(grow_flag){
			struct hlist_head *buddy;
			len = sizeof(struct hlist_head )*d->buddy_size*2;
			buddy=mcfg_malloc(len);
			if(!buddy){
				goto end;
			}
			memset(buddy,0,len);

			for(i=0;i<d->buddy_size;i++){
				hlist_for_each_safe(pos,n,&d->buddy[i]){
					tmp_entry=hlist_entry(pos,entry_t,hnode);
					hash=__hash33(tmp_entry->key);
					hash%=(d->buddy_size*2);
					hlist_add_head(&tmp_entry->hnode,&buddy[hash]);
				}
			}
			d->buddy_size*=2;
			mcfg_free(d->buddy);
			d->buddy = buddy;
		}
	}

end:
	if(entry){
		if(val){
			if(entry->val){
				mcfg_pr("mod %s : %s\n",entry->key,entry->val);
				d->tot_val_len-=strlen(entry->val)+5;
				mcfg_free(entry->val);
			}

			entry->val=__mstrdup(val,&len);
			if(!entry->val){
				return NULL;
			}
			d->tot_val_len+=len+5;
			mcfg_pr("add %s : %s\n",entry->key,entry->val);
		}
		return entry;
	}
	return NULL;
}

static int dict_unset_entry(dict_t*d,char*key){
	entry_t*entry;
	entry=dict_get_entry(d,key,0,NULL);
	if(entry){
		hlist_del(&entry->hnode);
		if(entry->key){
			mcfg_pr("del %s :",entry->key);
			d->tot_key_len -=strlen(entry->key)+5;
			mcfg_free(entry->key);
		}
		if(entry->val){
			mcfg_pr(": %s\n",entry->val);
			d->tot_val_len-=strlen(entry->val)+5;
			mcfg_free(entry->val);
		}
		mcfg_free(entry);
		d->entry_num--;
	}
	return 0;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
int mcfg_dict_new(int*dict){
	*dict=(int)dict_new();
	if(*dict==0){
		return -1;
	}
	return 0;
}

int mcfg_simple_getbool(char*filename,char*key,int*val){
	int rv;
	int dict;
	rv=mcfg_dict_new(&dict);
	rv|=mcfg_load_file(dict,filename);
	rv|=mcfg_getbool(dict, key,val);
	rv|=mcfg_dict_free(dict);
	return rv;
}

int mcfg_simple_setbool(char*filename,char*key,int val){
	int rv;
	int dict;
	rv=mcfg_dict_new(&dict);
	rv|=mcfg_setbool(dict, key, val);
	rv|=mcfg_update_file(dict,filename);
	rv|=mcfg_dict_free(dict);
	return rv;
}

int mcfg_simple_getint(char*filename,char*key,int*val){
	int rv;
	int dict;
	rv=mcfg_dict_new(&dict);
	rv|=mcfg_load_file(dict,filename);
	rv|=mcfg_getint(dict, key,val);
	rv|=mcfg_dict_free(dict);
	return rv;
}

int mcfg_simple_setint(char*filename,char*key,int val){
	int rv;
	int dict;
	rv=mcfg_dict_new(&dict);
	rv|=mcfg_setint(dict, key, val);
	rv|=mcfg_update_file(dict,filename);
	rv|=mcfg_dict_free(dict);
	return rv;
}

int mcfg_simple_getstring(char*filename,char*key,char*s,int size){
	int rv;
	int dict;
	rv=mcfg_dict_new(&dict);
	rv|=mcfg_load_file(dict,filename);
	rv|=mcfg_getstring(dict, key,s,size);
	rv|=mcfg_dict_free(dict);
	return rv;
}

int mcfg_simple_setstring(char*filename,char*key,char*s){
	int rv;
	int dict;
	rv=mcfg_dict_new(&dict);
	rv|=mcfg_setstring(dict, key, s);
	rv|=mcfg_update_file(dict,filename);
	rv|=mcfg_dict_free(dict);
	return rv;
}

//__delete_all_key__
int mcfg_delete_file(int dict,const char*filename){
	char tmpf[30];
	char cmd_buf[1024];
	static int num=0;
	num+=rand();
	sprintf(tmpf,"/tmp/mcfg_tmp_ini.%d",num++);
	if(mcfg_dump_file(dict,tmpf)){
		unlink(tmpf);
		return -1;
	}
	sprintf(cmd_buf,"/usr/bin/perl /root/change_ini.pl delete %s %s",tmpf,filename);
	if(system(cmd_buf)){
		unlink(tmpf);
		return -1;
	}
	unlink(tmpf);
	return 0;
}

/*
* 更新ini文件,调用perl脚本，保留文件的排版.
*/
int mcfg_update_file(int dict,const char*filename){
	char tmpf[30];
	char cmd_buf[1024];
	static int num=0;
	num+=rand();
	sprintf(tmpf,"/tmp/mcfg_tmp_ini.%d",num++);
	if(mcfg_dump_file(dict,tmpf)){
		unlink(tmpf);
		return -1;
	}
	sprintf(cmd_buf,"/usr/bin/perl /root/change_ini.pl update %s %s",tmpf,filename);
	if(system(cmd_buf)){
		unlink(tmpf);
		return -1;
	}
	unlink(tmpf);
	return 0;
}

int mcfg_dump_file(int dict,const char*filename){
	char*buffer;
	int len=1024*1024;
	int outsize=0;
	FILE*fp;

	buffer = (char*)malloc(len);
	if(buffer==NULL){
		printf("mcfg_dump_file err 1\n");
		return -1;
	}
	buffer[0]=0;

	fp = fopen(filename,"wb");
	if(fp==NULL){
		free(buffer);
		printf("mcfg_dump_file err 2\n");
		return -1;
	}

	if(mcfg_dump_buffer(dict,buffer,len,&outsize)){
		free(buffer);
		fclose(fp);
		printf("mcfg_dump_file err 3\n");
		return -1;
	}
	if(outsize){
		fwrite(buffer,1,outsize-1,fp);
	}
	free(buffer);
	fclose(fp);
	return 0;
}

int mcfg_dump_buffer(int dict,char*buffer,int size,int*outsize){
	int len=0;
	int i;
	int num;
	int rv;
	int totlen=0;
	entry_t*entry=NULL;
	dict_t*d=(dict_t*)dict;
	char**keys;

	char*p;
	char sec[50]={0};
	if(outsize==NULL){
		return -1;
	}
	*outsize=0;
	if(!d){
		return -1;
	}
	len=mcfg_get_keys_len(dict);
	if(len<=0){
		return -1;
	}
	keys=(char**)mcfg_malloc(len);
	rv = mcfg_get_keys(dict,keys,len,&num);
	if(rv){
		mcfg_free(keys);
		return rv;
	}

	for(i=0;i<num;i++){
		entry=dict_get_entry(d,keys[i],0,NULL);
		if(entry){
			if(entry->val){
				len = strlen(keys[i]) + strlen(entry->val) + 2;
			}
			else{
				len = strlen(keys[i]) + 2;
			}

			if((totlen + len) > size){
				//d3(("size err\n"));
				break;
			}

			if((p=strchr(keys[i],'.'))) {
				len=p-keys[i];
				if(strncmp(sec,keys[i],len)){
					memcpy(sec,keys[i],len);
					sec[len]=0;
					totlen+=sprintf(buffer+totlen,"\n[%s]\n",sec);
				}
				p++;
			}else{
				p=keys[i];
			}

			if(entry->val){
				totlen+=sprintf(buffer+totlen,"%s=%s",p,entry->val);
			}
			else{
				totlen+=sprintf(buffer+totlen,";%s=undef",p);
			}
			if(entry->read_cb){
				totlen+=sprintf(buffer+totlen," ;R");
			}
			if(entry->write_cb){
				totlen+=sprintf(buffer+totlen," ;W");
			}
			totlen+=sprintf(buffer+totlen,"\n");
		}
		else{
#if 0
			//d3("not found key=[%s],%d\n",keys[i],(int)entry);
			if(entry){
				//d3("val=%d\n",(int)entry->val);
			}
#endif
		}
	}

	*outsize=totlen+1;
	mcfg_free(keys);
	return 0;
}

char*__remove_black(char*s){
	int len;
	while(*s && isspace(*s)){
		s++;
	}
	len=strlen(s);
	while(len){
		if(isspace(s[len-1])){
			s[len-1]=0;
			len--;
			continue;
		}
		break;
	}
	return s;
}

int mstrlcpy(char*d,char*s,int len){
	int i=len;
	if(!len)return len;

	len--;
	while(len-- && *s){
		*d++ = *s++;
	}
	*d=0;
	return i - len - 1;
}

#define __line_buff_size 8*1024
int mcfg_load_buffer(int dict,char*buffer,int size) {
	int rv;
	//char line_buff[__line_buff_size];
	char *line_buff;

	char*line;
	int line_len;
	char*p;
	char*in=buffer;

	char*key;
	char*val;

	char sec_buff[50]={0};
	char*sec=NULL;

	char tmp[100];
	int len;

	if(dict==0){
		return -1;
	}

	line_buff=(char*)mcfg_malloc(__line_buff_size);
	if(line_buff==NULL){
		return -1;
	}

	while((in - buffer) < size ){
		p=strchr(in,'\n');
		if(p==NULL){
			line_len=strlen(in);
		}else{
			line_len=p-in;
		}
		if(line_len>=__line_buff_size){
			mcfg_free(line_buff);
			return -1;
		}

		line=line_buff;
		memcpy(line,in,line_len);
		in+=line_len+1;
		line[line_len]=0;

		//忽略注释
		p=strchr(line,';');
		if(p){
			*p=0;
		}

		line=__remove_black(line);
		if(!line[0]){
			//空行
			continue;
		}

		if(line[0]=='[') {
			p=strchr(line+1,']');
			if(!p){
				mcfg_free(line_buff);
				return -1;
			}

			len=p - line - 1;
			if(len>=sizeof(sec_buff)){
				mcfg_free(line_buff);
				return -1;
			}
			memcpy(sec_buff,line+1,len);
			sec=sec_buff;
			sec[len]=0;
			sec=__remove_black(sec);


			if(!sec[0]){
				//mcfg_pr("sec=%s\n",sec);
				//[]
				mcfg_free(line_buff);
				return -1;
			}
			continue;
		}

		p=strchr(line,'=');
		if(p){
			*p=0;
		}else{
			mcfg_free(line_buff);
			return -1;
		}

		key=line;
		val=p+1;
		key=__remove_black(key);
		val=__remove_black(val);

		if(key[0] && val[0]){
			if(sec && sec[0]){
				sprintf(tmp,"%s.%s",sec,key);
				rv=mcfg_setstring(dict,tmp,val);
				mcfg_pr("key=%s,val=%s\n",tmp,val);
			}else{
				rv=mcfg_setstring(dict,key,val);
				mcfg_pr("key=%s,val=%s\n",key,val);
			}
		}
	}
	mcfg_free(line_buff);
	return 0;
}

int mcfg_dict_free(int dict){
	dict_t*d=(dict_t*)dict;
	if(!d){
		return -1;
	}
	dict_free(d);
	return 0;
}

int mcfg_find_key(int dict,char*key){
	dict_t*d=(dict_t*)dict;
	if(!d){
		return -1;
	}
	return (int)dict_get_entry(d,key,0,NULL);
}

int mcfg_unset_key(int dict,char*key){
	if(!dict){
		return -1;
	}
	return dict_unset_entry((dict_t*)dict,key);
}

int mcfg_get_secs(int dict,char*buffer,int size,char**sec,int*num){
	//dict_t*d=(dict_t*)dict;

	return 0;
}

int mcfg_get_buffer_len(int dict){
	dict_t*d=(dict_t*)dict;
	if(!d){
		return -1;
	}
	return (d->tot_val_len + d->tot_key_len);
}

int mcfg_get_keys_len(int dict){
	dict_t*d=(dict_t*)dict;
	if(!d){
		return -1;
	}
	return (4*d->entry_num + d->tot_key_len);
}

static int __mcompare(const void*a1,const void*b1){
	const char*a=*(char**)a1;
	const char*b=*(char**)b1;

	int a_have_sec=0;
	int b_have_sec=0;
	int rv;

	if(strchr(a,'.')){
		a_have_sec=1;
	}
	if(strchr(b,'.')){
		b_have_sec=1;
	}

	if(a_have_sec==b_have_sec){
		rv = strcmp(a,b);
	}
	else{
		rv= a_have_sec - b_have_sec;
	}
	mcfg_pr("%s <=> %s : %d\n",a,b,rv);
	return rv;
}

int mcfg_match_key(int dict,void(*match)(int dict,char*key)){
	int i;
	struct hlist_node*pos;
	dict_t*d=(dict_t*)dict;
	entry_t*entry;

	if(!d){
		return -1;
	}

	for(i=0;i<d->buddy_size;i++){
		hlist_for_each(pos, &d->buddy[i]){
			entry=hlist_entry(pos,entry_t,hnode);
			match(dict,entry->key);
		}
	}
	return 0;
}

int mcfg_get_keys(int dict,char**buffer,int buffer_size,int*key_num){
	int i;
	struct hlist_node*pos;
	dict_t*d=(dict_t*)dict;
	entry_t*entry;
	char*p;
	int len;
	int num;

	if(!d){
		return -1;
	}
	if(buffer_size<(4*d->entry_num + d->tot_key_len)){
		return -1;
	}

	p = (char*)buffer + 4*d->entry_num;
	num=0;
	for(i=0;i<d->buddy_size;i++){
		hlist_for_each(pos, &d->buddy[i]){
			entry=hlist_entry(pos,entry_t,hnode);
			len=strlen(entry->key)+1;

			if((p+len) > ((char*)buffer + buffer_size)){
				//d3(("mcfg_get_keys buffer_size err\n"));
			}

			memcpy(p,entry->key,len);
			buffer[num]=p;
			num++;
			p+=len;
		}
	}

	if(num){
		qsort(buffer,num,4,__mcompare);
	}

	*key_num=num;
	return 0;
}

int mcfg_getbool(int dict,char*key,int*val){
	dict_t*d=(dict_t*)dict;
	entry_t*entry;
	char c;
	if(!d){
		return -1;
	}
	entry=dict_get_entry(d,key,0,NULL);
	if(!entry||!entry->val){
		return -1;
	}

	c=entry->val[0];
	if (c=='y' || c=='Y' || c=='1' || c=='t' || c=='T') {
		*val = 1 ;
		return 0;
	} else if (c=='n' || c=='N' || c=='0' || c=='f' || c=='F') {
		*val = 0 ;
		return 0;
	}
	return -1;
}

int mcfg_setbool(int dict,char*key,int val){
	dict_t*d=(dict_t*)dict;
	entry_t*entry;
	if(!d){
		return -1;
	}
	entry=dict_get_entry(d,key,1,val?"yes":"no");
	if(!entry){
		return -1;
	}
	return 0;
}

int mcfg_getint(int dict,char*key,int*val){
	dict_t*d=(dict_t*)dict;
	entry_t*entry;
	char*ptr;

	if(!d){
		return -1;
	}
	entry=dict_get_entry(d,key,0,NULL);
	if(!entry||!entry->val){
		return -1;
	}
	*val=(int)strtol(entry->val,&ptr,0);

	return 0;
}

int mcfg_setint(int dict,char*key,int val){
	dict_t*d=(dict_t*)dict;
	entry_t*entry;
	char tmp[20];
	if(!d){
		return -1;
	}
	sprintf(tmp,"%d",val);
	entry=dict_get_entry(d,key,1,tmp);
	if(!entry){
		return -1;
	}
	return 0;
}

int mcfg_getstring(int dict,char*key,char*s,int size){
	dict_t*d=(dict_t*)dict;
	entry_t*entry;
	if(!d){
		return -1;
	}
	entry=dict_get_entry(d,key,0,NULL);
	if(!entry||!entry->val){
		return -1;
	}
	mstrlcpy(s,entry->val,size);
	return 0;
}

int mcfg_setstring(int dict,char*key,char*s){
	dict_t*d=(dict_t*)dict;
	entry_t*entry;
	if(!d){
		return -1;
	}
	entry=dict_get_entry(d,key,1,s);
	if(!entry){
		return -1;
	}
	return 0;
}

int mcfg_read(int dict,char*key,char*buffer,int size,int*oksize){
	int len;
	dict_t*d=(dict_t*)dict;
	entry_t*entry;
	if(!d){
		return -1;
	}
	entry=dict_get_entry(d,key,0,NULL);
	if(!entry){
		return -1;
	}
	if(entry->read_cb){
		return entry->read_cb(dict,key,buffer,size,oksize);
	}
	if(entry->val){
		len=mstrlcpy(buffer,entry->val,size);
		*oksize=len+1;
		return 0;
	}
	return -1;
}

int mcfg_write(int dict,char*key,char*buffer,int size,int*oksize){
	dict_t*d=(dict_t*)dict;
	entry_t*entry;
	if(!d){
		return -1;
	}
	entry=dict_get_entry(d,key,0,buffer);
	if(!entry){
		return -1;
	}
	if(entry->write_cb){
		return entry->write_cb(dict,key,buffer,size,oksize);
	}
	*oksize=size;
	return 0;
}


int mcfg_register_read(int dict,char*key,mcfg_cb_t r_cb){
	dict_t*d=(dict_t*)dict;
	entry_t*entry;
	if(!d){
		return -1;
	}
	entry=dict_get_entry(d,key,1,NULL);
	if(!entry){
		return -1;
	}
	entry->read_cb=r_cb;
	return 0;
}


int mcfg_register_write(int dict,char*key,mcfg_cb_t w_cb){
	dict_t*d=(dict_t*)dict;
	entry_t*entry;
	if(!d){
		return -1;
	}
	entry=dict_get_entry(d,key,1,NULL);
	if(!entry){
		return -1;
	}
	entry->write_cb=w_cb;
	return 0;
}

#ifdef __KERNEL__

int  mcfg_load_file(int dict,char*filename)
{
	return 0;
}
#else
static int get_file_size(const char *filename)
{
	struct stat file_stat;
	if(stat(filename, &file_stat) == -1){
		return -1;
	}
	else{
		return file_stat.st_size;
	}
}
int mcfg_load_file(int dict,const char*filename){
	int fd;
	int file_size = -1;
	int read_num = -1;
	char *file_buffer = NULL;

	if(dict==0 || filename==NULL){
		return -1;
	}

	file_size = get_file_size(filename);
	if(file_size != -1) {
		file_buffer= malloc(file_size + 1);
		if(file_buffer == NULL) {
			//printf("the file_buffer is not alloc the memory\n");
			return -1;
		}
	}
	else {
		//printf("the file is nothing\n");
		return -1;
	}

	fd = open(filename, O_RDONLY);
	if(fd == -1) {
		//printf("file open is failed\n");
		free(file_buffer);
		return -1;
	}
	//flock(fd, LOCK_EX);

	read_num = read(fd, file_buffer, file_size);
	if(read_num <= 0) {
		//printf("read file is failed\n");
		free(file_buffer);
		//flock(fd, LOCK_UN);
		close(fd);
		return -1;
	}

	if(mcfg_load_buffer(dict,file_buffer,file_size) == -1) {
		//printf("mcfg_load_buffer is failed\n");
		free(file_buffer);
		//flock(fd, LOCK_UN);
		close(fd);
		return -1;
	}
	free(file_buffer);
	//flock(fd, LOCK_UN);
	close(fd);
	return 0;
}
#endif


int g_ipg_cfg=0;
int g_ts_cfg = 0;


#ifdef __KERNEL__


#define EXPORT_SYMTAB
EXPORT_SYMBOL(g_ipg_cfg);
EXPORT_SYMBOL(g_ts_cfg);

EXPORT_SYMBOL(mcfg_dict_new);
EXPORT_SYMBOL(mcfg_load_buffer);
EXPORT_SYMBOL(mcfg_dump_buffer);
EXPORT_SYMBOL(mcfg_dict_free);
EXPORT_SYMBOL(mcfg_find_key);
EXPORT_SYMBOL(mcfg_unset_key);
EXPORT_SYMBOL(mcfg_get_buffer_len);
EXPORT_SYMBOL(mcfg_get_keys_len);
EXPORT_SYMBOL(mcfg_get_keys);
EXPORT_SYMBOL(mcfg_getbool);
EXPORT_SYMBOL(mcfg_setbool);
EXPORT_SYMBOL(mcfg_getint);
EXPORT_SYMBOL(mcfg_setint);
EXPORT_SYMBOL(mcfg_getstring);
EXPORT_SYMBOL(mcfg_setstring);
EXPORT_SYMBOL(mcfg_register_read);
EXPORT_SYMBOL(mcfg_register_write);
EXPORT_SYMBOL(mcfg_read);
EXPORT_SYMBOL(mcfg_write);

#endif








