
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>

#include "esma_channel.h"
#include "esma_template.h"

#include "core/esma_alloc.h"
#include "core/esma_logger.h"

#include "common/compiler.h"
#include "common/parser_helper.h"

/* TODO: Необходимо добавить секцию ticks. В настоящий момент она в зачатке. */

struct esma_template *esma_template_new(char *name)
{
	struct esma_template *tmpl = NULL;
	   int err;

	tmpl = esma_malloc(sizeof(struct esma_template));
	if (unlikely(NULL == tmpl))
		return NULL;

	err = esma_template_init(tmpl, name);
	if (err) {
		esma_free(tmpl);
		return NULL;
	}

	return tmpl;
}

int esma_template_init(struct esma_template *tmpl, char *name)
{
	struct esma_state_template *est_init;
	struct esma_state_template *est_fini;
	int err;

	if (unlikely(NULL == tmpl))
		return 1;

	if (unlikely(NULL == name))
		return 1;

	sprintf(tmpl->name, "%s", name);

	err = esma_array_init(&tmpl->states, 10, sizeof(struct esma_state_template));
	if (err) {
		esma_engine_log_err("%s()/%s - can't allocate array for 10 states\n", __func__, name);
		goto __fail;
	}

	/* create INIT state */
	est_init = esma_array_push(&tmpl->states);

	err = esma_array_init(&est_init->trans, 8, sizeof(struct esma_trans_template));
	if (err) {
		esma_engine_log_err("%s()/%s - state 'init': can't allocate array for 8 trans\n", __func__, name);
		esma_array_free(&est_init->trans);
		goto __fail;
	}

	est_init->ntrans = 0;

	memcpy(est_init->name, "init", strlen("init"));
	/* end */

	/* create FINI state */
	est_fini = esma_array_push(&tmpl->states);

	err = esma_array_init(&est_fini->trans, 8, sizeof(struct esma_trans_template));
	if (err) {
		esma_engine_log_err("%s()/%s - state 'fini': can't allocate array for 8 trans\n", __func__, name);
		esma_array_free(&est_fini->trans);
		goto __fail;
	}

	est_fini->ntrans = 0;

	memcpy(est_fini->name, "fini", strlen("fini"));
	/* end */

	tmpl->nstates = 2;
	return 0;

__fail:
	esma_array_free(&tmpl->states);
	return 1;
}

void esma_template_free(struct esma_template *tmpl)
{
	if (NULL == tmpl)
		return;

	for (int i = 0; i < tmpl->nstates; i++) {
		struct esma_state_template *est;

		est = esma_array_n(&tmpl->states, i);
		esma_array_free(&est->trans);
	}

	esma_array_free(&tmpl->states);
	return;
}

void esma_template_print(struct esma_template *tmpl)
{
	printf("tmpl->name: %s\n", tmpl->name);

        for (int i = 0; i < tmpl->nstates; i++) {
                struct esma_state_template *state;

                state = esma_array_n(&tmpl->states, i);
                printf("STATE: %s\n", state->name);
                for (int j = 0; j < state->trans.nitems; j++) {
                        struct esma_trans_template *trans;
                        trans = esma_array_n(&state->trans, j);
                        printf("\tTRANS: %s -> %s; code: %d",
                                                state->name,
                                                trans->next_state->name,
                                                trans->action_code);
			switch (trans->ch_type) {
			case ESMA_CH_NONE:

				printf("\n");
				break;

			case ESMA_CH_DATA:
				printf(" channel type: Data; events: %d\n", trans->ch_data);
				break;

			case ESMA_CH_SIGN:
				printf(" channel type: Sign; number: %d\n", trans->ch_data);
				break;

			case ESMA_CH_TICK:
				printf(" channel type: Tick; interval_msec: %d\n", trans->ch_data);
				break;
			}
                }
        }
}


/* Use only in one thread; use consistenly  */
#define IS_VALID_STATE_CHAR(p) \
	(IS_LETTER(p) || IS_DIGIT(p) || *p == '_')

#define IS_VALID_FUNC_CHAR(p) \
	IS_VALID_STATE_CHAR(p)

#define IS_ROW(p)	\
	*p == '-' && *(p + 1) == '>'

#define IS_NOT_ROW(p)	\
	!(IS_ROW(p))

#define IS_TRANS_SRC_INIT(p) \
	str_4_cmp(p, 'i','n','i','t')

#define IS_TRANS_DST_SELF(p) \
	str_4_cmp(p, 's','e','l','f')

#define IS_DELIM(p) \
        *p == ' ' || *p == '\t' || *p == '\n'

enum states {
/* 0 */	st_start = 0,
/* 1 */	st_idle,

/* 2 */	st_state_section,						/* state */
/* 3 */		st_state_section_enter,					/* state { */
/* 4 */			st_state_name_enter,				/* 	idle */
/* 5 */			st_state_name_leave,				/* 	; */
/* 6 */		st_state_section_leave,					/* }; */

/* 7 */	st_ticks_section,						/* ticks */
/* 8 */		st_ticks_section_enter,					/* ticks { */
/* 9 */			st_ticks_tick_enter,				/* 	T0: 1000ms */
/* 10 */		st_ticks_tick_leave,				/* 	; */
/* 11 */	st_ticks_section_leave,					/* }; */

/* 12 */st_trans_section,						/* trans */
/* 13 */	st_trans_section_enter,					/* trans { */
/* 14 */		st_trans_state_src,				/* 	idle */
/* 15 */		st_trans_state_row,				/* 	idle -> */
/* 16 */		st_trans_state_dst,				/* 	idle -> wait */

/* 17 */		st_trans_state_dst_colon,			/* 	idle -> wait: */

/* 18 */		st_trans_code,					/* 	idle -> wait: 5 */
/* 19 */		st_trans_code_channel,				/*	idle -> wait: [for ex. tick_0] */
/* 20 */		st_trans_code_channel_tick_interval,		/* 	idle -> wait: tick_0: 1000s:[;] */
/* 21 */		st_trans_code_channel_tick_interval_period,	/* 	idle -> wait: tick_0: 1000s: ESMA_TM_PERIODIC; */
/* 22 */		st_trans_code_channel_sign_number,		/* 	idle -> wait: sign_1: SIGUSR1 */
/* 23 */		st_trans_code_channel_data_event,		/* 	idle -> wait: data_1: ESMA_POLLIN*/
/* 24 */		st_trans_semicolon,				/*	idle -> wait: 5; */
/* 25 */	st_trans_section_leave,					/* }; */

	/* error states */
	st_too_long_line,
	st_no_states,
};

static int state;

int _new_esma_state_template(struct esma_template *tmpl, char *name, u32 name_len)
{
	struct esma_state_template *state;
           int err;

	state = esma_array_push(&tmpl->states);
	memcpy(state->name, name, name_len);

	err = esma_array_init(&state->trans, 8, sizeof(struct esma_state_template));
	if (err) {
		esma_engine_log_err("%s()/%s - state '%s': can't allocate memory for 8 trans\n",
				__func__, tmpl->name, name);
		esma_array_free(&state->trans);
		return 1;
	}

	tmpl->nstates++;
	return 0;
}

/* TODO: необходимо добавлять состояния по индексу кода; сейчас делается обычный push */
int _new_esma_trans_template(struct esma_state_template *src,
			     struct esma_state_template *dst,
			     u32 code,
			     int ch_type, int ch_data, int periodic)
{
	struct esma_trans_template *trans;

	switch (ch_type) {
	case ESMA_CH_NONE:

		src->ntrans++;
		src->max_code = code > src->max_code ? code: src->max_code;
		break;

	case ESMA_CH_TICK:

		src->max_tick_code = code > src->max_tick_code ? code : src->max_tick_code;
		break;

	case ESMA_CH_DATA:

		src->max_data_code = code > src->max_data_code ? code : src->max_data_code;
		break;

	case ESMA_CH_SIGN:

		src->max_sign_code = code > src->max_sign_code ? code : src->max_sign_code;
		break;
	}

	trans = esma_array_push(&src->trans);
	if (NULL == trans) {
		esma_engine_log_err("%s()/%s - esma_array_push() returned NULL\n", __func__, dst->name);
		return 1;
	}

	trans->action_code = code;
	trans->next_state = dst;
	trans->ch_type = ch_type;
	trans->ch_data = ch_data;
	trans->ch_periodic = periodic;
	return 0;
}

static int flags = -1;
static int code = -1;
static int ch_type = ESMA_CH_NONE;
static int ch_data = -1;

static struct esma_state_template *est_src = NULL;
static struct esma_state_template *est_dst = NULL;

#define log_dbg_msg(...) printf(__VA_ARGS__);
static int __decode_dbuf(char *line, struct esma_template *et)
{
	char *p = line;
	char *end = line + strlen(line) - 1;
	char *t, *s; /* helpers pointers */

	int err;
	int ret;

	t = p;
	s = p;
	while (p != end) {

		switch (state) {

		case st_start:
		case st_idle:

			if (IS_DELIM(p)) {
				p++;
				break;
			}

/* TODO: Добавить проверку наличия двух и более секций states в конфигурации: в случае нахождения второй секции выходить с ошибкой */
			if (end - p >= 6) {
				if (str_6_cmp(p, 's','t','a','t','e','s')) {
					state = st_state_section;
					p += 6;
					break;
				}
			}

			if (end - p >= 5) {
				if (str_5_cmp(p, 't','r','a','n','s')) {
					state = st_trans_section;
					p += 5;
					break;
				}

				if (str_5_cmp(p, 't','i','c','k','s')) {
					state = st_ticks_section;
					p += 5;
					break;
				}
			}

			p++;
			break;

		case st_ticks_section:

			if (IS_NOT_OPEN_BRACE(p)) {
				p++;
				break;
			}

			state = st_ticks_section_enter;
			p++;
			break;

		case st_ticks_section_enter:

			if (IS_CLOSE_BRACE(p)) {
				state = st_state_section_leave;
				p++;
				break;
			}

			if (IS_DELIM(p) || IS_SEMICOLON(p)) {
				p++;
				break;
			}

			break;

		case st_state_section:

			if (IS_NOT_OPEN_BRACE(p)) {
				p++;
				break;
			}

			state = st_state_section_enter;
			p++;
			break;

		case st_state_section_enter:

			if (IS_CLOSE_BRACE(p)) {
				state = st_state_section_leave;
				p++;
				break;
			}

			if (IS_DELIM(p) || IS_SEMICOLON(p)) {
				p++;
				break;
			}

			if (IS_VALID_STATE_CHAR(p)) {
				state = st_state_name_enter;
				break;
			}

			esma_engine_log_err("%s()/%s - section 'states': bad symbol '%c'\n",
					__func__, et->name, *p);

			goto __fail;

		case st_state_name_enter:

			t = s = p;
#if 0
			if (IS_DELIM(t) || IS_SEMICOLON(t)) {
				p++;
				break;
			}
#endif

			if (IS_CLOSE_BRACE(p)) {

				if (et->nstates == 2) { /* states section is empty */
					state = st_no_states;
					esma_engine_log_err("%s()/%s - section 'states': section is empty\n", __func__, et->name);
					goto __fail;
				}

				state = st_state_section_leave;
				p++;
				break;
			}

			while(IS_VALID_STATE_CHAR(t)) {
				t++;
			}

			if (4 == t - s) { /* for special states - init and fini */
				if (str_4_cmp(s, 'i','n','i','t')) {
					p += 4;
					state = st_state_name_leave;
					break;
				}

				if (str_4_cmp(s, 'f','i','n','i')) {
					p += 4;
					state = st_state_name_leave;
					break;
				}
			}

			err = _new_esma_state_template(et, s, t - s);
			if (err) {
				esma_engine_log_err("%s()/%s - section 'states': _new_esma_state_template(): failed\n", __func__, et->name);
				return err;
			}

			p = t;
			state = st_state_name_leave;
			break;

		case st_state_name_leave:

			if (IS_SEMICOLON(p)) {
				p++;
				state = st_state_section_enter;
				break;
			}

			if (IS_DELIM(p)) {
				p++;
				break;
			}

			esma_engine_log_err("%s()/%s - section 'states': need semicolon after state name\n",
					__func__, et->name);
			goto __fail;

		case st_state_section_leave:

			if(IS_DELIM(p)) {
				p++;
				break;
			}

			if (IS_SEMICOLON(p)) {
				state = st_idle;
				p++;
				break;
			}

			esma_engine_log_err("%s()/%s - section 'states': need semicolon after closed brace\n",
					__func__, et->name);
			goto __fail;

		case st_trans_section:

			if (IS_DELIM(p)) {
				p++;
				break;
			}

			if (IS_OPEN_BRACE(p)) {
				p++;
				state = st_trans_section_enter;
				break;
			}

/* TODO: необходимо избавиться */
#if 0
			if (IS_CLOSE_BRACE(p)) {
				p++;
				state = st_trans_section_leave;
				break;
			}
#endif
			esma_engine_log_err("%s()/%s - section 'states': invalid symbol '%c' after 'trans' word\n",
					__func__, et->name, *p);
			goto __fail;

		case st_trans_section_enter:

			if (IS_DELIM(p)) {
				p++;
				break;
			}

			if (IS_CLOSE_BRACE(p)) {
				p++;
				state = st_trans_section_leave;
				break;
			}

			if (IS_VALID_STATE_CHAR(p)) {
				state = st_trans_state_src;
				break;
			}

			esma_engine_log_err("%s()/%s - section 'trans': Invalid symbol '%c' after '{'\n", __func__, et->name, *p);
			goto __fail;

		case st_trans_state_src:

			if (IS_DELIM(p)) {
				p++;
				break;
			}

			s = t = p;

			while (IS_VALID_STATE_CHAR(t)) {
				t++;
			}

			for (int i = 0; i < et->nstates; i++) {
				   int mismatch;
				struct esma_state_template *tmpl;

				tmpl = esma_array_n(&et->states, i);
				if (NULL == tmpl)
					break;

				if (strlen(tmpl->name) != t - s)
					 continue;

				mismatch = strncmp(tmpl->name, s, t - s);
				if (mismatch)
					 continue;

				est_src = tmpl;
				break;
			}

			if (NULL == est_src) {
				*t = 0;
				esma_engine_log_err("%s()/%s - section 'trans': src state '%s' not found\n", __func__, et->name, s);
				goto __fail;
			}

			p = t;
			state = st_trans_state_row;
			break;

		case st_trans_state_row:

			if (IS_DELIM(p)) {
				p++;
				break;
			}

			if (end - p >= 2) {
				if (IS_ROW(p)) {
					p += 2;
					state = st_trans_state_dst;
					break;
				}

				esma_engine_log_err("%s()/%s - section 'trans':  invalid symbols '%c%c': need '->'\n",
						__func__, et->name, *p, *(p + 1));
				goto __fail;
			}

			esma_engine_log_err("%s()/%s - section 'trans': need '->' after state name\n", __func__, et->name);
			goto __fail;

		case st_trans_state_dst:

			if (IS_DELIM(p)) {
				p++;
				break;
			}

			s = t = p;

			while (IS_VALID_STATE_CHAR(t)) {
				t++;
			}

			if (4 == t - s) { /* special state for moore machine */
				if (str_4_cmp(s, 's','e','l','f')) {
					est_dst = NULL;
					p = t;
					state = st_trans_state_dst_colon;
					break;
				}
			}

			for (int i = 0; i < et->nstates; i++) {
				   int mismatch;
				struct esma_state_template *tmpl;

				tmpl = esma_array_n(&et->states, i);
				if (NULL == tmpl)
					break;

				if (strlen(tmpl->name) != t - s)
					 continue;

				mismatch = strncmp(tmpl->name, s, t - s);
				if (0 != mismatch)
					 continue;

				est_dst = tmpl;
				break;
			}

			if (NULL == est_dst) {
				*t = 0;
				esma_engine_log_err("%s()/%s - section 'trans': dst state '%s' not found\n",
						__func__, et->name, s);
				goto __fail;
			}

			p = t;
			state = st_trans_state_dst_colon;
			break;

		case st_trans_state_dst_colon:

			if (IS_DELIM(p)) {
				p++;
				break;
			}

			if (IS_NOT_COLON(p)) {
				esma_engine_log_err("%s()/%s - section 'trans': need colon after dst state\n",
						__func__, et->name);
				goto __fail;
			}

			state = st_trans_code;
			p++;
			break;

		case st_trans_code:

			if (IS_DELIM(p)) {
				p++;
				break;
			}

			s = p;

			while (IS_VALID_FUNC_CHAR(s)) {
				s++;
			}

			ret = sscanf(p, "%d", &code);
			if (1 != ret) {
				state = st_trans_code_channel;
				break;
			}

			p = s;

			state = st_trans_semicolon;
			break;

		case st_trans_code_channel:

			ret = sscanf(p, "tick_%d", &code);
			if (1 == ret) {
				p = s;
				ch_type = ESMA_CH_TICK;
				state = st_trans_code_channel_tick_interval;
				break;
			}

			ret = sscanf(p, "data_%d", &code);
			if (1 == ret) {
				p = s;
				ch_type = ESMA_CH_DATA;
				state = st_trans_code_channel_data_event;
				break;
			}

			ret = sscanf(p, "sign_%d", &code);
			if (1 == ret) {
				p = s;
				ch_type = ESMA_CH_SIGN;
				state = st_trans_code_channel_sign_number;;
				break;
			}

			*s = 0;
			esma_engine_log_err("%s()/%s - section 'trans': can't read code '%s'\n",
					__func__, et->name, p);
			goto __fail;

		case st_trans_code_channel_tick_interval:

			while (IS_DELIM(p)) {
				p++;
			}

			if (IS_NOT_COLON(p)) {
				goto __fail;
			}

			p++;

			while (IS_DELIM(p)) {
				p++;
			}

			s = p;

			while (IS_DIGIT(s)) {
				s++;
			}

			if (s == p)
				goto __fail;

			if (*s == 'm' && *(s + 1) == 's') {
				ret = sscanf(p, "%dms", &ch_data);
				if (ret != 1) {
					esma_engine_log_err("%s()/%s - section 'trans': can't read tick interval\n",
							__func__, et->name);
					goto __fail;
				}

				p = s + 2;
			}

			if (*s == 's') {
				ret = sscanf(p, "%ds", &ch_data);
				if (ret != 1) {
					esma_engine_log_err("%s()/%s - section 'trans': can't read tick interval\n",
							__func__, et->name);
					goto __fail;
				}

				ch_data *= 1000;
				p = s + 1;
			}

			while (IS_DELIM(p)) {
				p++;
			}

			if (IS_COLON(p)) {
				p++;
				state = st_trans_code_channel_tick_interval_period;
				break;
			}

			if (IS_SEMICOLON(p)) {
				state = st_trans_semicolon;
				break;
			}

			esma_engine_log_err("%s()/%s - section 'trans': error in tick interval\n",
					__func__, et->name);
			goto __fail;

		case st_trans_code_channel_tick_interval_period:

			while (IS_DELIM(p)) {
				p++;
			}

			s = p;

			while (IS_LETTER(p)) {
				p++;
			}

			if (p - s != 4)	{ /* 'ESMA' */
				esma_engine_log_err("%s()/%s - section 'trans': tick interval: invalid period name\n",
						__func__, et->name);
				goto __fail;
			}

			if (!str_4_cmp(s, 'E','S','M','A')) {
				esma_engine_log_err("%s()/%s - section 'trans': tick interval: invalid period name\n",
						__func__, et->name);
				goto __fail;
			}

			if (IS_NOT_UNDERSCORE(p)) {
				esma_engine_log_err("%s()/%s - section 'trans': tick interval: invalid period name\n",
						__func__, et->name);
				goto __fail;
			}

			p++;

			if ('T' != *p || 'M' != *(p + 1)) {
				esma_engine_log_err("%s()/%s - section 'trans': tick interval: invalid period name\n",
						__func__, et->name);
				goto __fail;
			}

			p += 2;

			if (IS_NOT_UNDERSCORE(p)) {
				esma_engine_log_err("%s()/%s - section 'trans': tick interval: invalid period name\n",
						__func__, et->name);
				goto __fail;
			}

			p++;
			s = p;

			while (IS_LETTER(p)) {
				p++;
			}

			switch (p - s) {
				case 8:	/* 'PERIODIC' */
					if (str_8_cmp(s, 'P','E','R','I','O','D','I','C')) {
						state = st_trans_semicolon;
						flags = ESMA_TM_PERIODIC;
						break;
					}

					esma_engine_log_err("%s()/%s - section 'trans': tick interval: invalid period name\n",
						__func__, et->name);

					goto __fail;

				case 7:	/* 'ONESHOT' */
					if (str_7_cmp(s, 'O','N','E','S','H','O','T')) {
						state = st_trans_semicolon;
						flags = ESMA_TM_ONESHOT;
						break;
					}

					esma_engine_log_err("%s()/%s - section 'trans': tick interval: invalid period name\n",
						__func__, et->name);

					goto __fail;

				default:

					esma_engine_log_err("%s()/%s - section 'trans': tick interval: invalid period name\n",
						__func__, et->name);

					goto __fail;
			}

			break;

		case st_trans_code_channel_sign_number:

			if (IS_DELIM(p)) {
				p++;
				break;
			}

			if (IS_NOT_COLON(p)) {
				esma_engine_log_err("%s()/%s - section 'trans': need colon\n", __func__, et->name);
				goto __fail;
			}

			p++;

			while (IS_DELIM(p)) {
				p++;
			}

			s = p;
			while (IS_LETTER(s) || IS_DIGIT(s)) {
				s++;
			}

			if (!str_3_cmp(p, 'S', 'I', 'G')) {
				esma_engine_log_err("%s()/%s - section 'trans': invalid signal name\n", __func__, et->name);
				goto __fail;
			}

			p += 3;
			switch (s - p) {
			case 3:
				if (str_3_cmp(p, 'B', 'U', 'S')) {
					ch_data = SIGBUS;
					state = st_trans_semicolon;
					break;
				}

				if (str_3_cmp(p, 'F', 'P', 'E')) {
					ch_data = SIGFPE;
					state = st_trans_semicolon;
					break;
				}

				if (str_3_cmp(p, 'H', 'U', 'P')) {
					ch_data = SIGHUP;
					state = st_trans_semicolon;
					break;
				}

				if (str_3_cmp(p, 'I', 'L', 'L')) {
					ch_data = SIGILL;
					state = st_trans_semicolon;
					break;
				}

				if (str_3_cmp(p, 'I', 'N', 'T')) {
					ch_data = SIGINT;
					state = st_trans_semicolon;
					break;
				}

				if (str_3_cmp(p, 'S', 'Y', 'S')) {
					ch_data = SIGSYS;
					state = st_trans_semicolon;
					break;
				}

				if (str_3_cmp(p, 'U', 'R', 'G')) {
					ch_data = SIGURG;
					state = st_trans_semicolon;
					break;
				}

				esma_engine_log_err("%s()/%s - section 'trans': invalid signal name\n", __func__, et->name);
				goto __fail;

			case 4:
				if (str_4_cmp(p, 'A', 'B', 'R', 'T')) {
					ch_data = SIGABRT;
					state = st_trans_semicolon;
					break;
				}

				if (str_4_cmp(p, 'A', 'L', 'R', 'M')) {
					ch_data = SIGALRM;
					state = st_trans_semicolon;
					break;
				}

				if (str_4_cmp(p, 'C', 'H', 'L', 'D')) {
					ch_data = SIGCHLD;
					state = st_trans_semicolon;
					break;
				}

				if (str_4_cmp(p, 'C', 'O', 'N', 'T')) {
					ch_data = SIGCONT;
					state = st_trans_semicolon;
					break;
				}

				if (str_4_cmp(p, 'K', 'I', 'L', 'L')) {
					ch_data = SIGKILL;
					state = st_trans_semicolon;
					break;
				}

				if (str_4_cmp(p, 'P', 'I', 'P', 'E')) {
					ch_data = SIGPIPE;
					state = st_trans_semicolon;
					break;
				}

				if (str_4_cmp(p, 'Q', 'U', 'I', 'T')) {
					ch_data = SIGQUIT;
					state = st_trans_semicolon;
					break;
				}

				if (str_4_cmp(p, 'S', 'E', 'G', 'V')) {
					ch_data = SIGSEGV;
					state = st_trans_semicolon;
					break;
				}

				if (str_4_cmp(p, 'S', 'T', 'O', 'P')) {
					ch_data = SIGSTOP;
					state = st_trans_semicolon;
					break;
				}

				if (str_4_cmp(p, 'T', 'E', 'R', 'M')) {
					ch_data = SIGTERM;
					state = st_trans_semicolon;
					break;
				}

				if (str_4_cmp(p, 'T', 'S', 'T', 'P')) {
					ch_data = SIGTSTP;
					state = st_trans_semicolon;
					break;
				}

				if (str_4_cmp(p, 'T', 'T', 'I', 'N')) {
					ch_data = SIGTTIN;
					state = st_trans_semicolon;
					break;
				}

				if (str_4_cmp(p, 'T', 'T', 'O', 'U')) {
					ch_data = SIGTTOU;
					state = st_trans_semicolon;
					break;
				}


				if (str_4_cmp(p, 'U', 'S', 'R', '1')) {
					ch_data = SIGUSR1;
					state = st_trans_semicolon;
					break;
				}


				if (str_4_cmp(p, 'U', 'S', 'R', '2')) {
					ch_data = SIGUSR2;
					state = st_trans_semicolon;
					break;
				}


				if (str_4_cmp(p, 'P', 'O', 'L', 'L')) {
					ch_data = SIGPOLL;
					state = st_trans_semicolon;
					break;
				}


				if (str_4_cmp(p, 'P', 'R', 'O', 'F')) {
					ch_data = SIGPROF;
					state = st_trans_semicolon;
					break;
				}

				if (str_4_cmp(p, 'T', 'R', 'A', 'P')) {
					ch_data = SIGTRAP;
					state = st_trans_semicolon;
					break;
				}

				if (str_4_cmp(p, 'X', 'C', 'P', 'U')) {
					ch_data = SIGXCPU;
					state = st_trans_semicolon;
					break;
				}

				if (str_4_cmp(p, 'X', 'C', 'P', 'U')) {
					ch_data = SIGXCPU;
					state = st_trans_semicolon;
					break;
				}

				esma_engine_log_err("%s()/%s - section 'trans': invalid signal name\n", __func__, et->name);
				goto __fail;

			default:

				if (str_6_cmp(p, 'V', 'T', 'A', 'L', 'R', 'M')) {
					ch_data = SIGVTALRM;
					state = st_trans_semicolon;
					break;
				}

				esma_engine_log_err("%s()/%s - section 'trans': invalid signal name\n", __func__, et->name);
				goto __fail;
			}

			p = s;
			break;

		case st_trans_code_channel_data_event:

			if (IS_DELIM(p)) {
				p++;
				break;
			}

			if (IS_NOT_COLON(p)) {
				esma_engine_log_err("%s()/%s - section 'trans': need colon\n", __func__, et->name);
				goto __fail;
			}

			p++;

			while (IS_DELIM(p)) {
				p++;
			}

			s = p;
			while (IS_LETTER(s) || *s == '_') {
				s++;
			}

			if (!str_5_cmp(p, 'E', 'S', 'M', 'A', '_')) {
				esma_engine_log_err("%s()/%s - section 'trans': invalid event name\n", __func__, et->name);
				goto __fail;
			}
			p += 5;

			switch (s - p) {
			case 7:
				if (str_7_cmp(p, 'P', 'O', 'L', 'L', 'O', 'U', 'T')) {
					ch_data = ESMA_POLLOUT;
					state = st_trans_semicolon;
					break;
				}

				if (str_7_cmp(p, 'P', 'O', 'L', 'L', 'E', 'R', 'R')) {
					ch_data = ESMA_POLLERR;
					state = st_trans_semicolon;
					break;
				}

				if (str_7_cmp(p, 'P', 'O', 'L', 'L', 'H', 'U', 'P')) {
					ch_data = ESMA_POLLHUP;
					state = st_trans_semicolon;
					break;
				}

				esma_engine_log_err("%s()/%s - section 'trans': invalid event name\n", __func__, et->name);
				goto __fail;

			case 6:
				if (str_6_cmp(p, 'P', 'O', 'L', 'L', 'I', 'N')) {
					ch_data = ESMA_POLLIN;
					state = st_trans_semicolon;
					break;
				}

				esma_engine_log_err("%s()/%s - section 'trans': invalid event name\n", __func__, et->name);
				goto __fail;;;;;;;;;;;;;;;;;;;;

			default:

				esma_engine_log_err("%s()/%s - section 'trans': invalid event name\n", __func__, et->name);
				goto __fail;;;;;;;;;;;;;;;;;;;;
			}

			p = s;
			break;

		case st_trans_semicolon:

			if (IS_DELIM(p)) {
				p++;
				break;
			}

			if (IS_NOT_SEMICOLON(p)) {
				esma_engine_log_err("%s()/%s - section 'trans': need semicolon after trans description.\n",
						__func__, et->name);
				goto __fail;
			}

			err = _new_esma_trans_template(est_src, est_dst, code, ch_type, ch_data, flags);
			if (err) {
				esma_engine_log_err("%s()/%s - section 'trans': can't set trans: %s -> %s\n",
						__func__, et->name, est_src->name, est_dst->name);
				goto __fail;
			}

			code = -1;
			ch_type = ESMA_CH_NONE;
			ch_data = -1;
			flags = -1;

			est_src = NULL;
			est_dst = NULL;

			p++;
			state = st_trans_section_enter;
			break;

		case st_trans_section_leave:

			if (IS_DELIM(p)) {
				p++;
			}

			if (IS_NOT_SEMICOLON(p)) {
				esma_engine_log_err("%s()/%s - section 'trans': need semicolon after section\n",
						__func__, et->name);
				goto __fail;
			}

			p++;
			state = st_idle;
			break;
		}
	}

	return 0;

__fail:

	esma_engine_log_err("%s()/%s - have error. Last state: %d\n", __func__, et->name, state);
	switch (state) {
	case st_no_states:
		esma_engine_log_err("%s()/%s - can't find any state\n", __func__, et->name);
		return 1;

	default:
		return 1;
	}
}

int esma_template_set_by_path(struct esma_template *et, char *path)
{
	FILE *file;
	struct stat stat;
	struct esma_dbuf dbuf;
	char c;
	int err = 0;

	file = fopen(path, "r");
	if (NULL == file) {
		esma_engine_log_ftl("%s() - can't open '%s' file.\n", __func__, path);
		return 1;
	}

	err = fstat(fileno(file), &stat);
	if (err) {
		esma_engine_log_ftl("%s() - fstat('%s'): failed\n", __func__, path);
		return 1;
	}

	if (0 == stat.st_size) {
		esma_engine_log_ftl("%s()  - file '%s' is empty\n", __func__, path);
		return 1;
	}

	err = esma_dbuf_init(&dbuf, stat.st_size + 1);
	if (err) {
		esma_engine_log_ftl("%s() - esma_dbuf_init(%ld bytes): failed\n", __func__, stat.st_size + 1);
		return 1;
	}

	while (1) {

		c = fgetc(file);
		if (EOF == c)
			break;

		*(dbuf.pos++) = c;
	}

	*dbuf.pos = 0;
	dbuf.cnt = dbuf.len;
	dbuf.pos = dbuf.loc;

	err = esma_template_set_by_dbuf(et, &dbuf);
	esma_dbuf_free(&dbuf);

	return err;
}

int esma_template_set_by_dbuf(struct esma_template *et, struct esma_dbuf *dbuf)
{
	int err;

	if (NULL == et || NULL == dbuf)
		return 1;

	dbuf->pos = dbuf->loc;
	state = st_start;

	err = __decode_dbuf((char *) dbuf->loc, et);

	est_src = NULL;
	est_dst = NULL;
	ch_type = ESMA_CH_NONE;
	ch_data = -1;
	code = -1;

	state = st_start;

	return err;
}
