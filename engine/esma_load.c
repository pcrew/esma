
#include "esma.h"

#include "core/esma_array.h"
#include "core/esma_alloc.h"
#include "core/esma_logger.h"

#include "utils/load_tool.h"

#include "common/macro_magic.h"

static struct state *_esma_find_next_state_by_name(struct esma *esma, struct esma_trans_template *tmpl)
{
	for (int i = 0; i < esma->states.nitems; i++) {
		struct state *state = esma_array_n(&esma->states, i);

		if (NULL == state)
			return NULL;

		if (0 == strcmp(state->name, tmpl->next_state->name))
			return state;
	}

	return NULL;
}

static int _esma_prepare_states(struct esma *esma, struct esma_template *tmpl)
{
	for (int i = 0; i < tmpl->nstates; i++) {
		struct esma_state_template *state_tmpl = esma_array_n(&tmpl->states, i);
		struct state *state = esma_array_push(&esma->states);
		  char *name = alloca(256);
		   int ntrans;
		   int err;

		if (NULL == state_tmpl || NULL == state)
			goto __fail;

		sprintf(name, "%s_%s_enter", esma->name, state_tmpl->name);
		state->enter = load_tool(NULL, name);
		if (NULL == state->enter) {
			esma_engine_log_err("%s()/%s - can't find '%s' function\n", __func__, tmpl->name, name);
			goto __fail;
		}

		sprintf(name, "%s_%s_leave", esma->name, state_tmpl->name);
		state->leave = load_tool(NULL, name);
		if (NULL == state->leave) {
			esma_engine_log_err("%s()/%s - can't find '%s' function\n", __func__, tmpl->name, name);
			goto __fail;
		}

		ntrans = max(state_tmpl->max_code, state_tmpl->ntrans);

		err = esma_array_init(&state->trans, ntrans + 1, sizeof(struct trans));
		if (err) {
			esma_engine_log_err("%s()/%s - can't allocate array of trans for '%s' state\n",
					__func__, tmpl->name, state_tmpl->name);
			goto __fail;
		}

		err = esma_array_init(&state->tick_trans, state_tmpl->max_tick_code + 1, sizeof(struct trans));
		if (err) {
			esma_engine_log_err("%s()/%s - can't allocate array of tick trans for '%s' state\n",
					__func__, tmpl->name, state_tmpl->name);
			goto __fail;
		}

		err = esma_array_init(&state->sign_trans, state_tmpl->max_sign_code + 1, sizeof(struct trans));
		if (err) {
			esma_engine_log_err("%s()/%s - can't allocate array of sign trans for '%s' state\n",
					__func__, tmpl->name, state_tmpl->name);
			goto __fail;
		}

		sprintf(state->name, state_tmpl->name, strlen(state_tmpl->name));
	}

	return 0;

__fail:
	return 1;
}

static int _esma_prepare_trans(struct esma *esma, struct esma_template *tmpl)
{
	for (int i = 0; i < esma->states.nitems; i++) {
		struct esma_state_template *state_tmpl;
		struct state *state;
		  char *name;

		state_tmpl = esma_array_n(&tmpl->states, i);
		state = esma_array_n(&esma->states, i);
		name = alloca(256);

		if (NULL == state_tmpl || NULL == state) {
			esma_engine_log_err("%s() - state_tmpl or state is NULL\n", __func__);
			goto __fail;
		}

		for (int j = 0; j < state_tmpl->trans.nitems; j++) {
			struct esma_trans_template *trans_tmpl;
			struct trans *trans;
			action action;
			   int code;

			trans_tmpl = esma_array_n(&state_tmpl->trans, j);
			if (NULL == trans_tmpl) {
				esma_engine_log_err("%s()/%s - trans_tmpl is NULL\n", __func__, tmpl->name);
				goto __fail;
			}

			code = trans_tmpl->action_code;

			switch (trans_tmpl->ch_type) {
			case ESMA_CH_NONE:
				trans = esma_array_n(&state->trans, code);
				sprintf(name, "%s_%s_%d", esma->name, state->name, code);
				break;

			case ESMA_CH_TICK:
				trans = esma_array_push(&state->tick_trans);
				trans->ch.info.tick.periodic = trans_tmpl->ch_periodic;
				sprintf(name, "%s_%s_tick_%d", esma->name, state->name, code);
				break;

			case ESMA_CH_SIGN:
				trans = esma_array_push(&state->sign_trans);
				sprintf(name, "%s_%s_sign_%d", esma->name, state->name, code);
				break;

			case ESMA_CH_DATA:
				switch (trans_tmpl->ch_data) {
				case ESMA_POLLIN:
					trans = &state->data_pollin;
					break;

				case ESMA_POLLOUT:
					trans = &state->data_pollout;
					break;

				case ESMA_POLLERR:
				case ESMA_POLLHUP:
					trans = &state->data_pollerr;
					break;

				default:
					goto __fail;
				}

				sprintf(name, "%s_%s_data_%d", esma->name, state->name, code);
				break;

			default:
				esma_engine_log_err("%s()/%s invalid channel type '%s' state: %d\n",
						__func__, tmpl->name, state->name, trans_tmpl->ch_type);
				goto __fail;
			}

			if (NULL == trans) {
				esma_engine_log_err("%s()/%s trans for '%s' state is NULL\n",
						__func__, tmpl->name, state->name);
				goto __fail;
			}

			trans->ch.type = trans_tmpl->ch_type;
			trans->ch.raw_data = trans_tmpl->ch_data;

			/* Meely section enter*/
			if (NULL == trans_tmpl->next_state) {
				action = load_tool(NULL, name);
				if (NULL == action) {
					esma_engine_log_err("%s()/%s - can't find %s()\n", __func__, tmpl->name, name);
					goto __fail;
				}

				trans->code = code;
				trans->action = action;
				trans->next_state = NULL;
				continue;
			} /* Meely section leave */

			/* Moore section enter */
			trans->code = code;
			trans->action = NULL;
			trans->next_state = _esma_find_next_state_by_name(esma, trans_tmpl);
			if (NULL == trans->next_state) {
				esma_engine_log_err("%s()/%s - next state for '%s' state is NULL\n",
						__func__, tmpl->name, state->name);
				goto __fail;
			}
			/* Moore section leave */
		}
	}
	return 0;

__fail:
	return 1;
}

int esma_load(struct esma *esma, struct esma_template *tmpl)
{
	int err;

	strcpy(esma->name, tmpl->name);

	err = esma_array_init(&esma->states, tmpl->nstates, sizeof(struct state));
	if (err) {
		esma_engine_log_ftl("%s()/%s - esma_array_init() failed\n", __func__, tmpl->name);
		goto __fail;
	}

	err = _esma_prepare_states(esma, tmpl);
	if (err) {
		esma_engine_log_ftl("%s()/%s - _esma_prepare_states() failed\n", __func__, tmpl->name);
		goto __fail;
	}

	err = _esma_prepare_trans(esma, tmpl);
	if (err) {
		esma_engine_log_ftl("%s()/%s - _esma_prepare_trans() failed\n", __func__, tmpl->name);
		goto __fail;
	}

	return 0;

__fail:
	return 1;
}
