/**************************************************************************
*
*  SFX  (Sentence Feature eXtractor)
*
*  COPYRIGHT (c) 2018 Konan Technology, Inc.
*  All rights are reserved. No part of this work covered by the copyright
*  hereon may be reproduced or transmitted, in any form or by any means,
*  electronic, mechanical, photocopying, recording or otherwise, without
*  prior written permission of the copyright holder
*
*  DATE    : 2018. 3.
*  AUTHOR  : Hyun-Min Lee
*
*************************************************************************/

#include <stdio.h>
#include <string.h>

//#include "sy-mem.h"
#include "km-api.h"

#define	_4K		0x00001000
#define	eOK		0x00000000 // OK
#define	eERR	-0x00000001 // General errors

#define	FILE_OPEN(_msg, _fp, _fname, _suffix, _mode) { \
	char	fn[256]; \
	sprintf(fn, "%s%s", _fname, _suffix); \
	_fp = fopen(fn, _mode); \
	if (_fp == NULL) { \
		sprintf(_msg, "cannot open file '%s'", fn); \
		goto error; \
	} \
}

#define	FILE_CLOSE(_fp, _fn, _sf, _n, _s) { \
	char	fn[256]; \
	sprintf(fn, "%s%s", _fn, _sf); \
	if (_fp) { \
		fprintf(_fp, "\nTotal %d sentences %s.\n", _n, _s);\
		fclose(_fp); \
		printf("'%s' created.\n", fn);\
	}\
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// variable definition

char	m_data_path[256]={0,};
char	m_input_file[256]={0,};
char*	m_charset = "EUCKR";

void*	m_hd_lex = NULL;
void*	m_h_lex = NULL;
void*	m_hd_tag = NULL;
void*	m_h_tag = NULL;

/////////////////////////////////////////////////////////////////////////////////////////////////
// function definition

void usage(char *prog);
int parse_option(char* p_arg[], int n_arg);
int init(char msg[]);
void final(char msg[]);
int dll_open_all(char msg[]);
void dll_close_all();

/////////////////////////////////////////////////////////////////////////////////////////////////
// module fp definition

// UCC module

typedef int     (*F_UCC_INITIAL)(char msg[]);
typedef void    (*F_UCC_FINAL)();
typedef int     (*F_UCC_LOAD)(char msg[], char[]);
typedef int     (*F_UCC_CONV)(char msg[], char in_data[], int in_size, char out_buf[], int out_max, int* p_out_size);

// KMA module

typedef int     (*F_KMA_LOAD_DATA)(char msg[], char path[], char charset[], void** p_hm);
typedef void    (*F_KMA_UNLOAD_DATA)(void* hm);
typedef void    (*F_KMA_INSTALL_DATA)(void* hm);
typedef int     (*F_KMA_CREATE)(char msg[], void**);
typedef void    (*F_KMA_DESTROY)(void*);
typedef int     (*F_KMA_ANALYZE) (char msg[], void* hm, char para_iput_text[], int para_input_size, char input_charset[], void* p_user_conf, _KONAKDAT_Sent** para_pp_out_res, int* para_p_out_size_done);
typedef void	(*F_KMA_REGISTER_CALLBACK_KSC_TO_UTF8)(int (*callback_func)(char msg[], char in_data[], int in_size, char out_buf[], int out_max, int* p_out_size));
typedef void	(*F_KMA_REGISTER_CALLBACK_UTF8_TO_KSC)(int (*callback_func)(char msg[], char in_data[], int in_size, char out_buf[], int out_max, int* p_out_size));

// KPT module

typedef int     (*F_KPT2_LOAD_DATA)(char msg[], char path[], char charset[], void** p_hm);
typedef void    (*F_KPT2_UNLOAD_DATA)(char msg[], void* hm);
typedef int     (*F_KPT2_CREATE)(char msg[], void** p_hx);
typedef int     (*F_KPT2_DESTROY)(char msg[], void* hx);
typedef int     (*F_KPT2_TAG)(char msg[], void* hm, void* hx, void* para_p_sent_res, char input_charset[]);

// SFX module
typedef char* (*F_SFX_GET_MODULE_VERSION)(char path[]);
typedef int (*F_SFX_LOAD_DATA)(char msg[], char path[]);
typedef int (*F_SFX_UNLOAD_DATA)(char msg[]);
typedef int (*F_SFX_PROFILE) (char msg[], 
		char sent_text[], int sent_size, int sent_no, int sent_off,
		char charset[], void* ma_result, 
		char option[], void* p_user_conf,
		char* doc_text, int doc_size, 
		char* wanted_prop, 
		FILE* fp_match, FILE* fp_notmatch, 
		int* para_out_match);

void	*m_p_ucc_module = NULL;
void	*m_p_kma_module = NULL;
void	*m_p_kpt2_module = NULL;
void	*m_p_sfx_module = NULL;

F_UCC_INITIAL	m_fp_ucc_initialize;
F_UCC_FINAL		m_fp_ucc_finalize;
F_UCC_LOAD		m_fp_ucc_load_ksc_to_utf8;
F_UCC_LOAD		m_fp_ucc_load_utf8_to_ksc;
F_UCC_CONV		m_fp_ucc_conv_ksc_to_utf8;
F_UCC_CONV		m_fp_ucc_conv_utf8_to_ksc;

F_KMA_LOAD_DATA		m_fp_kma_load_data;
F_KMA_UNLOAD_DATA	m_fp_kma_unload_data;
F_KMA_INSTALL_DATA	m_fp_kma_install_data;
F_KMA_CREATE		m_fp_kma_create;
F_KMA_DESTROY		m_fp_kma_destroy;
F_KMA_ANALYZE		m_fp_kma_analyze;
F_KMA_REGISTER_CALLBACK_KSC_TO_UTF8	m_fp_kma_register_callback_ksc_to_utf8;
F_KMA_REGISTER_CALLBACK_UTF8_TO_KSC	m_fp_kma_register_callback_utf8_to_ksc;

F_KPT2_LOAD_DATA		m_fp_kpt2_load_data;
F_KPT2_UNLOAD_DATA	m_fp_kpt2_unload_data;
F_KPT2_CREATE		m_fp_kpt2_create;
F_KPT2_DESTROY		m_fp_kpt2_destroy;
F_KPT2_TAG			m_fp_kpt2_tag;

F_SFX_LOAD_DATA		m_fp_sfx_load_data;
F_SFX_UNLOAD_DATA	m_fp_sfx_unload_data;
F_SFX_PROFILE		m_fp_sfx_profile;


int main(int argc, char *argv[])
{
	char	msg[1024];

	char	in_buf[_4K];
	int		bi, bl;
	int		sent_size;
	int		sent_no;

	char	*p;
	char	*p_prop = NULL;
	char	*p_text = NULL;

	_KONAKDAT_Sent		*p_sent;

	FILE	*fp = NULL;

	int		n_match;

	// 외부 패러미터 확인

	if (parse_option(argv+1, argc-1) < 0) {
		usage(argv[0]);
		return eERR;
	}

	// 초기화

	if (init(msg) < 0) goto error;

	// fp 지정

	if (m_input_file[0] == '\0') {
		fp = stdin;
	} else {
		FILE_OPEN(msg, fp, m_input_file, "", "r");
	}

	// 분석 

	while(1) {
		if (fp == stdin) fprintf(stdout, "SFX> ");
		if (fgets(in_buf, _4K, fp) == NULL) break;

		p = strpbrk(in_buf, "\r\n");
		if (p != NULL) *p = 0;

		if (fp == stdin) {	// interactive 방식인 경우만 quit 확인
			if (!strcasecmp(in_buf, "q")) break;
			if (!strcasecmp(in_buf, "qu")) break;
			if (!strcasecmp(in_buf, "qui")) break;
			if (!strcasecmp(in_buf, "quit")) break;
		}

		if (*in_buf == 0) continue;

		p_prop = NULL;
		p_text = in_buf;
		fprintf(stdout, "\n>>> %s\n", p_text);

		bl = strlen(p_text);
		sent_no = 0;

		for (bi=0; bi<bl;) {
			if ((*m_fp_kma_analyze)(msg, m_h_lex, p_text+bi, bl-bi, m_charset, NULL, &p_sent, &sent_size) < 0) break;
			if (p_sent == NULL) break;
			if (sent_size == 0) break;

			if (!p_sent || p_sent->m_n_word <= 0) goto next;

			if ((*m_fp_kpt2_tag)(msg, m_hd_tag, m_h_tag, p_sent, m_charset)) goto error;

			if ((*m_fp_sfx_profile)(msg,
			                p_text+bi, sent_size, sent_no, bi, m_charset, p_sent, "", NULL,
							p_text, bl, p_prop, NULL, NULL, &n_match)) goto error;

			next:

			++sent_no;
			bi += sent_size;
		}
	}


	// 정리

	if (fp!=stdin && fp!=NULL)	fclose(fp);
	final(msg);

	return eOK;

	error:

	printf("Error: %s\n", msg);
	return eERR;
}

void usage(char *prog)
{
	printf("SFX (build:%s %s)\n", __DATE__, __TIME__);
	printf("usage: %s [OPTION] [INPUT FILE]\n", prog);
	printf("       -d <data-path>\n");
	printf("          data (default)\n");
	printf("       -c <charset>\n");
	printf("          EUCKR (default)\n");
	printf("          UTF8\n");
}

#define EARG(_msg) { fprintf(stderr, "%s\n", _msg); return eERR; }

int parse_option(char* p_arg[], int n_arg)
{
	register int	i;

	for (i=0; i<n_arg; i++) {
		if (!strcmp(p_arg[i], "-c")) {
			if ((++i) >= n_arg) EARG("missing charset.");
			char	*s;
			s = p_arg[i];
			if (!strcasecmp(s, "EUC") ||
				!strcasecmp(s, "EUCKR") ||
				!strcasecmp(s, "EUC-KR") ||
				!strcasecmp(s, "EUC_KR")) {
				m_charset = "EUCKR";
			} else
			if (!strcasecmp(s, "UTF8") ||
				!strcasecmp(s, "UTF-8") ||
				!strcasecmp(s, "UTF_8")) {
				m_charset = "UTF8";
			} else {
				fprintf(stderr, "unknown charset '%s'.\n", s);
				return eERR;
			}
		} else
		if (!strcmp(p_arg[i], "-d")) {
			if ((++i) >= n_arg) EARG("missing data directory.");
			strcpy(m_data_path, p_arg[i]);

		} else
		if (*p_arg[i]=='-') {
			fprintf(stderr, "unknown option '%s'.\n", p_arg[i]+1);
			return eERR;
		} else
		if (m_input_file[0] == '\0') {
			strcpy(m_input_file, p_arg[i]);
		} else {
			fprintf(stderr, "too many input files.\n");
			return eERR;
		}
	}

	if (m_data_path[0] == '\0') strcpy(m_data_path, "data");

	return eOK;
}

#ifdef  _WINNT
#   include    <windows.h>
#   define     DLL_OPEN(_path)     ((HINSTANCE)LoadLibrary(_path))
#   define     DLL_CLOSE(_dp)      FreeLibrary((HINSTANCE)(_dp))
#   define     DLL_FUNC(_dp, _fn)  GetProcAddress((HINSTANCE)_dp, _fn)
#   define     DLL_ERROR()         ""
#   include	   <io.h>
#   define     POSIX_ACCESS    _access
#else
#   include    <sys/time.h>
#   include    <dirent.h>
#   include    <dlfcn.h>
#   define     DLL_OPEN(_path)     dlopen(_path, RTLD_LAZY)
#   define     DLL_CLOSE(_dp)      dlclose(_dp)
#   define     DLL_FUNC(_dp, _fn)  dlsym(_dp, _fn)
#   define     DLL_ERROR()         dlerror()
#   include    <unistd.h>
#   define     POSIX_ACCESS    access
#endif

#define RESOLVE_FUNC(_type, _p_module, _pf, _func_name) { \
	_pf = (_type)DLL_FUNC(_p_module, _func_name); \
	if ((_pf)==NULL) { \
		sprintf(msg, "%s not defined.", _func_name); \
		return eERR;\
	} \
}

#define	RESOLVE_MODNAME(_mod_name, _data_path, _cand_1, _cand_2) { \
	sprintf(_mod_name, "%s/%s", _data_path, _cand_1); \
	if (POSIX_ACCESS(_mod_name, 4) != 0) { \
		char	_tmp[256]; \
		sprintf(_tmp, "%s/%s", _data_path, _cand_2); \
		if (POSIX_ACCESS(_tmp, 4) != 0) { \
			sprintf(msg, "module '%s' not exist.", _mod_name); \
			return eERR; \
		} \
		strcpy(_mod_name, _tmp); \
	} \
}

int	dll_open_all(char msg[])
{
	char	mod[256];

	RESOLVE_MODNAME(mod, m_data_path, "libucc.so", "kql048.mod");

	m_p_ucc_module = DLL_OPEN(mod);
	if (m_p_ucc_module == NULL) {
		sprintf(msg, "module '%s' corrupted.", mod);
		return eERR;
	}

	RESOLVE_FUNC(F_UCC_INITIAL, m_p_ucc_module, m_fp_ucc_initialize, "ucc_initialize");
	RESOLVE_FUNC(F_UCC_FINAL, m_p_ucc_module, m_fp_ucc_finalize, "ucc_finalize");
	RESOLVE_FUNC(F_UCC_LOAD, m_p_ucc_module, m_fp_ucc_load_ksc_to_utf8, "ucc_load_ksc_to_utf8");
	RESOLVE_FUNC(F_UCC_LOAD, m_p_ucc_module, m_fp_ucc_load_utf8_to_ksc, "ucc_load_utf8_to_ksc");
	RESOLVE_FUNC(F_UCC_CONV, m_p_ucc_module, m_fp_ucc_conv_ksc_to_utf8, "ucc_conv_ksc_to_utf8");
	RESOLVE_FUNC(F_UCC_CONV, m_p_ucc_module, m_fp_ucc_conv_utf8_to_ksc, "ucc_conv_utf8_to_ksc");

	RESOLVE_MODNAME(mod, m_data_path, "libmodkma.so", "kql270.mod");

	m_p_kma_module = DLL_OPEN(mod);
	if (m_p_kma_module == NULL) {
		sprintf(msg, "module '%s' corrupted.", mod);
		return eERR;
	}

	RESOLVE_FUNC(F_KMA_LOAD_DATA, m_p_kma_module, m_fp_kma_load_data, "KMA_load_data");
	RESOLVE_FUNC(F_KMA_UNLOAD_DATA, m_p_kma_module, m_fp_kma_unload_data, "KMA_unload_data");
	RESOLVE_FUNC(F_KMA_INSTALL_DATA, m_p_kma_module, m_fp_kma_install_data, "KMA_install_data");
	RESOLVE_FUNC(F_KMA_CREATE, m_p_kma_module, m_fp_kma_create, "KMA_create");
	RESOLVE_FUNC(F_KMA_DESTROY, m_p_kma_module, m_fp_kma_destroy, "KMA_destroy");
	RESOLVE_FUNC(F_KMA_ANALYZE, m_p_kma_module, m_fp_kma_analyze, "KMA_analyze");
	RESOLVE_FUNC(F_KMA_REGISTER_CALLBACK_KSC_TO_UTF8, m_p_kma_module, m_fp_kma_register_callback_ksc_to_utf8, "KMA_register_callback_ksc_to_utf8");
	RESOLVE_FUNC(F_KMA_REGISTER_CALLBACK_UTF8_TO_KSC, m_p_kma_module, m_fp_kma_register_callback_utf8_to_ksc, "KMA_register_callback_utf8_to_ksc");

	RESOLVE_MODNAME(mod, m_data_path, "libmodkpt2.so", "kql274.mod");

	m_p_kpt2_module = DLL_OPEN(mod);
	if (m_p_kpt2_module == NULL) {
		sprintf(msg, "module '%s' corrupted.", mod);
		return eERR;
	}

	RESOLVE_FUNC(F_KPT2_LOAD_DATA, m_p_kpt2_module, m_fp_kpt2_load_data, "KPT2_load_data");
	RESOLVE_FUNC(F_KPT2_UNLOAD_DATA, m_p_kpt2_module, m_fp_kpt2_unload_data, "KPT2_unload_data");
	RESOLVE_FUNC(F_KPT2_CREATE, m_p_kpt2_module, m_fp_kpt2_create, "KPT2_create");
	RESOLVE_FUNC(F_KPT2_DESTROY, m_p_kpt2_module, m_fp_kpt2_destroy, "KPT2_destroy");
	RESOLVE_FUNC(F_KPT2_TAG, m_p_kpt2_module, m_fp_kpt2_tag, "KPT2_tag");


	RESOLVE_MODNAME(mod, m_data_path, "libmodsfx.so", "kql203.mod");

	m_p_sfx_module = DLL_OPEN(mod);
	if (m_p_sfx_module == NULL) {
		sprintf(msg, "module '%s' corrupted.", mod);
		return -1;
	}

	RESOLVE_FUNC(F_SFX_LOAD_DATA, m_p_sfx_module, m_fp_sfx_load_data, "SFX_load_data");
	RESOLVE_FUNC(F_SFX_UNLOAD_DATA, m_p_sfx_module, m_fp_sfx_unload_data, "SFX_unload_data");
	RESOLVE_FUNC(F_SFX_PROFILE, m_p_sfx_module, m_fp_sfx_profile, "SFX_profile");

	return eOK;

}

void dll_close_all()
{
	DLL_CLOSE(m_p_ucc_module);
	DLL_CLOSE(m_p_kma_module);
	DLL_CLOSE(m_p_kpt2_module);
	DLL_CLOSE(m_p_sfx_module);
}

int init(char msg[])
{
	if (dll_open_all(msg) < 0) goto error;

	if ((*m_fp_ucc_initialize)(msg) ||
		(*m_fp_ucc_load_utf8_to_ksc)(msg, m_data_path) ||
		(*m_fp_ucc_load_ksc_to_utf8)(msg, m_data_path))
		goto error;

	(*m_fp_kma_register_callback_utf8_to_ksc)((*m_fp_ucc_conv_utf8_to_ksc));
	(*m_fp_kma_register_callback_ksc_to_utf8)((*m_fp_ucc_conv_ksc_to_utf8));

	if ((*m_fp_kma_load_data)(msg, m_data_path, m_charset, &m_hd_lex)) goto error;
	(*m_fp_kma_install_data)(m_hd_lex);
	if ((*m_fp_kma_create)(msg, &m_h_lex)) goto error;

	if ((*m_fp_kpt2_load_data)(msg, m_data_path, m_charset, &m_hd_tag)) goto error;
	if ((*m_fp_kpt2_create)(msg, &m_h_tag)) goto error;

	if ((*m_fp_sfx_load_data)(msg, m_data_path)) goto error;

	return eOK;

	error:

	return eERR;
}

void final(char msg[])
{
	(*m_fp_sfx_unload_data)(msg);

	(*m_fp_kpt2_destroy)(msg, m_h_tag);
	(*m_fp_kpt2_unload_data)(msg, m_hd_tag);

	(*m_fp_kma_destroy)(m_h_lex);
	(*m_fp_kma_unload_data)(m_hd_lex);

	(*m_fp_ucc_finalize)();

	dll_close_all();
}

