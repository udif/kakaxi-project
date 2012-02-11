
/*!
\file
*/

#ifndef VNS_CONFIG_H
#define VNS_CONFIG_H

//#include "vns_sys.h"


extern int g_ipg_cfg;
extern int g_ts_cfg;

typedef int(*mcfg_cb_t)(int dict,char*key,char*buffer,int size,int*oksize);

int mcfg_dict_new(int*dict);
int mcfg_load_buffer(int dict,char*buffer,int size);
int mcfg_load_file(int dict,const char*filename);
int mcfg_dump_file(int dict,const char*filename);
int mcfg_dump_buffer(int dict,char*buffer,int size,int*outsize);
int mcfg_dict_free(int dict);
int mcfg_find_key(int dict,char*key);
int mcfg_unset_key(int dict,char*key);
int mcfg_get_secs(int dict,char*buffer,int size,char**sec,int*num);
int mcfg_get_buffer_len(int dict);
int mcfg_get_keys_len(int dict);
int mcfg_get_keys(int dict,char*buffer[],int buffer_size,int*key_num);
int mcfg_getbool(int dict,char*key,int*val);
int mcfg_setbool(int dict,char*key,int val);
int mcfg_getint(int dict,char*key,int*val);
int mcfg_setint(int dict,char*key,int val);
int mcfg_getstring(int dict,char*key,char*s,int size);
int mcfg_setstring(int dict,char*key,char*s);
int mcfg_register_read(int dict,char*key,mcfg_cb_t r_cb);
int mcfg_register_write(int dict,char*key,mcfg_cb_t w_cb);
int mcfg_read(int dict,char*key,char*buffer,int size,int*oksize);
int mcfg_write(int dict,char*key,char*buffer,int size,int*oksize);

#endif


