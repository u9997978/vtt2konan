/**************************************************************************
 *
 *  KMA  -- A KOREAN MORPHOLOGICAL ANALYSIS MODULE
 *
 *  COPYRIGHT (c) 1999-2018 KONAN TECHNOLOGY, INC.
 *  All rights are reserved. No part of this work covered by the copyright
 *  hereon may be reproduced or transmitted, in any form or by any means, 
 *  electronic, mechanical, photocopying, recording or otherwise, without 
 *  prior written permission of the copyright holder.
 *
 *  DATE        :  2013. 2. 7
 *  AUTHOR      :  SEUNG HYUN YANG
 *  HISTORY     :
 *
 *************************************************************************/

#ifndef	_KMA_H
#define	_KMA_H

struct _KONAKDAT_Lemma {
	char*	m_str;
	char*	m_dummy;
	char	m_tag;
	char	m_int_tag;
	short	m_surf_off;
	short	m_surf_len;
};

struct _KONAKDAT_LemmaList {
	_KONAKDAT_Lemma*	m_p_lemma;
	int				m_n_lemma;
	float			m_tag_score; 
	int				m_aux;
};

struct _KONAKDAT_Word {
	char*			m_str;
	char*			m_str2;
	int				m_offset;
	_KONAKDAT_LemmaList	*m_p_lemma_list;
	int					m_n_lemma_list;
};

struct _KONAKDAT_Sent {
	char			*m_str;
	_KONAKDAT_Word	*m_p_word;		
	int				m_n_word;
};

// external tag set 

enum {
	NN=0, 	// noun
	NP, 	// noun, pro-
	NX, 	// noun, auxiliary
	NU, 	// noun, numeral
	VV, 	// verb
	VX, 	// verb, auxiliary
	AJ, 	// adjective
	AX, 	// adjective, auxiliary
	AD, 	// adverb
	DT, 	// determiner
	DN,		// determiner, numeral
	IJ, 	// interjection
	JO, 	// particle, postpositinal
	CP, 	// copula
	EM, 	// ending, final
	EP, 	// ending, prefinal
	PF,		// prefix
	SF, 	// suffix, common
	SV,		// suffix, verbal derivative
	SJ,		// suffix, adjectival derivative
	SN,		// suffix, nominal derivative
	SY, 	// symbol
	ZZ		// etc
};

// internal tag set 

enum {
	_TAG_MIN=0,
	_NN,	// 1 noun, common
	_NC,	// 2 noun, compound
	_NM,	// 3 noun, common, left-catenatable (SCAN-specific)
	_NP,	// 4 noun, pro-
	_NX,	// 5 noun, auxiliary, common
	_NW,	// 6 noun, auxiliary, left-catenatable (SCAN-specific)
	_NT,	// 7 noun, auxiliary, unital 
	_NU,	// 8 noun, numeral
	_ND,	// 9 noun, numeral, digit
	_NR,	// 10 noun, proper
	_NK,	// 11 noun, unknown
	_VV,	// 12 verb, common
	_VX,	// 13 verb, auxiliary
	_AJ,	// 14 adjective, common
	_AX,	// 15 adjective, auxiliary
	_AD,	// 16 adverb, common
	_AC,	// 17 adverb, common, left-catenatable (SCAN-specific)
	_AE,	// 18 adverb, common, left-catenatable (SCAN-specific)
	_DT,	// 19 determiner, common
	_DU,	// 20 determiner, numeral
	_DN,	// 21 reserved
	_IJ,	// 22 interjection
	_PP,	// 23 particle, postpositional, common 
	_PN,	// 24 particle, postpositional, adnominal
	_CP,	// 25 copula
	_EF,	// 26 ending, final, common
	_EC,	// 27 ending, final, ¾î/¾Æ
	_ER,	// 28 ending, final, relativizer
	_EN,	// 29 ending, final, nominalizer
	_EP,	// 30 ending, prefinal
	_ET,	// 31 ending, final, terminating
	_SF,	// 32 suffix, common
	_PF,	// 33 prefix
	_SV,	// 34 suffix, verbalizer
	_SJ,	// 35 suffix, adjectivalizer
	_SN,	// 36 suffix, nominalizer
	_ST,	// 37 suffix, unital (SCAN-specific)
	_SU,	// 38 suffix, numeral (SCAN-specific)
	_SY,	// 39 symbol
	_UK,	// 40
	_UC,	// 41
	_XX, 	// 42
	_TAG_MAX
};

#ifdef  __cplusplus
extern "C" {
#endif

extern	int	KMA_load_data(char msg[], char data_path[], char para_charset[], void** p_hd);
extern	int	KMA_load_data2(char msg[], char data_path[], char para_charset[], char para_coined_word_file[], char para_hanja_sound_file[], void** p_hd);
extern	void	KMA_unload_data(void* hd);
extern	int	KMA_create(char msg[], void** p_hd);
extern	void	KMA_destroy(void* hd);
extern int	KMA_analyze(char msg[], void* hd, char para_input_text[], int para_input_size, char para_charset[], _KONAKDAT_Sent** para_pp_out_res, int* para_p_size_done);
extern int	KMA_analyze_profiled(char msg[], void* hd, char para_input_text[], int para_input_size, char para_charset[], int para_analysis_level, _KONAKDAT_Sent** para_pp_out_res, int* para_p_size_done, int para_p_mem[], int para_p_dic[], int para_p_rma[]);
extern int	KMA_lookup_dic(char msg[], void* hd, char para_input_text[], int para_input_size, char para_charset[], _KONAKDAT_Sent** para_pp_out_res, int* para_p_size_done);
extern void	KMA_format(_KONAKDAT_Sent* para_p_sent_res, char buf[], int buf_max, int* p_buf_len);
extern void	KMA_unformat(char input_data[], int input_size, char buf[], int buf_max, int* p_buf_len, int* p_sent_len, _KONAKDAT_Sent** para_pp_sent_res);
extern void	KMA_register_callback_ksc_to_utf8(int (*callback_func)(char msg[], char in_data[], int in_size, char out_buf[], int out_max, int* p_out_size));
extern void	KMA_register_callback_utf8_to_ksc(int (*callback_func)(char msg[], char in_data[], int in_size, char out_buf[], int out_max, int* p_out_size));

#ifdef  __cplusplus
}
#endif

#endif


