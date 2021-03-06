#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <tools.h>
#include <hardware.h>
#include "yield.h"

struct ctx_s * current_ctx = NULL;
struct ctx_s * ctx_ring = NULL;
struct ctx_s * ctx_hda;

static void * initial_ebp;
static void * initial_esp;

static unsigned int current_ctx_start_time;
unsigned int cpt_index = 1;

int init_ctx(struct ctx_s * ctx, size_t stack_size, funct_t f, void * arg) {
	ctx->ctx_stack = malloc(stack_size);

	if (ctx->ctx_stack == NULL )
		return RETURN_FAILURE;

	ctx->ctx_ebp = ((char*) ctx->ctx_stack);
	ctx->ctx_esp = ctx->ctx_ebp + stack_size;

	ctx->ctx_id = cpt_index++;
	ctx->ctx_f = f;
	ctx->ctx_arg = arg;
	ctx->ctx_state = CTX_INIT;
	ctx->ctx_magic = CTX_MAGIC;
	ctx->ctx_start_time = (unsigned int) time(NULL );
	ctx->ctx_exec_time = 0;

	return RETURN_SUCCESS;
}

void switch_to_ctx(struct ctx_s * ctx) {
	struct ctx_s *temp;
	assert(ctx->ctx_magic == CTX_MAGIC);
	irq_disable();

	while (ctx->ctx_state == CTX_END || ctx->ctx_state == CTX_STOP) {
		if (ctx_ring == ctx) {
			ctx_ring = current_ctx;
		}

		if (ctx == current_ctx) {
			fprintf(stderr, "All context in the ring are blocked.\n");
		}
		temp = ctx;
		ctx = ctx->ctx_next;

		if (ctx->ctx_state == CTX_END) {
			temp->ctx_next = ctx->ctx_next;
			free(ctx->ctx_stack);
			free(ctx);
		}
	}

	if (current_ctx != NULL ) {

		asm ("movl %%esp, %0" "\n\t" "movl %%ebp, %1"
				: "=r"(current_ctx->ctx_esp), "=r"(current_ctx->ctx_ebp)
				:);

		/* Put the new uptime in the structure */
		current_ctx->ctx_exec_time += (((int) time(NULL ))
				- current_ctx_start_time);
	}

	current_ctx = ctx;

	/* Start the uptime */
	current_ctx_start_time = (unsigned int) time(NULL );

	asm ("movl %0, %%esp" "\n\t" "movl %1, %%ebp"
			:
			: "r"(current_ctx->ctx_esp), "r"(current_ctx->ctx_ebp));

	if (current_ctx->ctx_state == CTX_INIT) {
		irq_enable();
		start_current_ctx();
	}
	irq_enable();
	return;

}

void start_current_ctx() {

	current_ctx->ctx_state = CTX_EXE;
	current_ctx->ctx_f(current_ctx->ctx_arg);
	current_ctx->ctx_state = CTX_END;

	if (current_ctx->ctx_next == current_ctx) {
		asm ("movl %0, %%esp" "\n\t" "movl %1, %%ebp"
				:
				: "r"(initial_esp), "r"(initial_ebp));
	}

}

int create_ctx(int stack_size, funct_t f, void* args) {
	struct ctx_s * new_ctx;
	new_ctx = malloc(sizeof(struct ctx_s));
	if (new_ctx == 0) {
		return RETURN_FAILURE;
	}

	if (ctx_ring == NULL ) {
		ctx_ring = new_ctx;
		new_ctx->ctx_next = new_ctx;
	} else {
		new_ctx->ctx_next = ctx_ring->ctx_next;
		ctx_ring->ctx_next = new_ctx;
	}
	if (init_ctx(new_ctx, (size_t) stack_size, f, args) == RETURN_FAILURE) {
		return RETURN_FAILURE;
	}

	return new_ctx->ctx_id;
}



/*
 * TODO
 *
 *
 * Modifier le yield hw, retirer le code et proteger les acces disque par des semaphores (write et read + init sem)
 * retirer du ring le ctx_hda, au moment où on le transfere (else)
 *
 *
 *
 *
 * garder 20 ms
 *
 *
 */


void yield_hw() {

	if (ctx_hda != NULL ) {

		_mask(15);
		struct ctx_s * next = ctx_hda;
		ctx_hda = ctx_hda->ctx_next;
		switch_to_ctx(next);
		_mask(1);
	} else {
		ctx_hda = current_ctx;
		yield();
	}
}

void yield() {

	_out(TIMER_ALARM, 0xffffffff - 20);

	if (ctx_ring == NULL )
		return;

	if (current_ctx != NULL ) {

		switch_to_ctx(current_ctx->ctx_next);

	} else {
		asm ("movl %%esp, %0" "\n\t" "movl %%ebp, %1"
				: "=r"(initial_esp), "=r"(initial_ebp)
				:);
		switch_to_ctx(ctx_ring);
	}

}

void get_state_name(enum ctx_state_e state, char* string) {
	if (state == CTX_INIT) {
		strcpy(string, "INIT");
	} else if (state == CTX_EXE) {
		strcpy(string, "EXE");
	} else if (state == CTX_STOP) {
		strcpy(string, "STOP");
	} else if (state == CTX_END) {
		strcpy(string, "END");
	} else {
		strcpy(string, "UNK");
	}
}
