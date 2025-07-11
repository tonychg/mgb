#include "gb/sm83.h"
#include "gb/alloc.h"
#include "gb/fs.h"
#include "gb/types.h"
#include <cjson/cJSON.h>
#include <stdlib.h>

struct context_cycle {
	u16 pc;
	u16 pc_value;
};

struct context {
	u16 pc;
	u16 sp;
	u8 a;
	u8 f;
	u8 b;
	u8 c;
	u8 d;
	u8 e;
	u8 h;
	u8 l;
	u8 *memory;
};

struct test_case {
	struct context *initial;
	struct context *final;
	struct context_cycle *cycles;
	int size;
};

struct test_suite {
	char *path;
	struct test_case **tests;
	int size;
	int total;
};

static void test_context_destroy(struct context *ctx)
{
	if (ctx) {
		zfree(ctx->memory);
	}
	zfree(ctx);
}

static void test_case_destroy(struct test_case *test_case)
{
	if (test_case) {
		test_context_destroy(test_case->initial);
		test_context_destroy(test_case->final);
		zfree(test_case->cycles);
	}
	zfree(test_case);
}

static void test_suite_destroy(struct test_suite *suite)
{
	for (int i = 0; i < suite->size; i++) {
		test_case_destroy(suite->tests[i]);
	}
	zfree(suite);
}

static struct test_case *test_case_alloc()
{
	struct test_case *test_case;

	test_case = (struct test_case *)malloc(sizeof(struct test_case));
	if (!test_case)
		goto error;
	test_case->initial = (struct context *)malloc(sizeof(struct context));
	test_case->final = (struct context *)malloc(sizeof(struct context));
	if (!test_case->initial || !test_case->final)
		goto free_test_case;
	test_case->cycles = NULL;
	return test_case;

free_test_case:
	test_case_destroy(test_case);
error:
	return NULL;
}

static void serialize_test_context(cJSON *context, struct context *test_context)
{
	cJSON *ram = NULL, *segment = NULL, *item = NULL;

	test_context->pc = cJSON_GetObjectItem(context, "pc")->valueint;
	test_context->sp = cJSON_GetObjectItem(context, "sp")->valueint;
	test_context->a = cJSON_GetObjectItem(context, "a")->valueint;
	test_context->f = cJSON_GetObjectItem(context, "f")->valueint;
	test_context->b = cJSON_GetObjectItem(context, "b")->valueint;
	test_context->c = cJSON_GetObjectItem(context, "c")->valueint;
	test_context->d = cJSON_GetObjectItem(context, "d")->valueint;
	test_context->e = cJSON_GetObjectItem(context, "e")->valueint;
	test_context->h = cJSON_GetObjectItem(context, "h")->valueint;
	test_context->l = cJSON_GetObjectItem(context, "l")->valueint;
	ram = cJSON_GetObjectItem(context, "ram");
	// TODO handle interrupts
	test_context->memory = (u8 *)calloc(0xFFFF, sizeof(u8));
	if (!test_context->memory) {
		printf("Failed to allocate memory\n");
		return;
	}
	cJSON_ArrayForEach(segment, ram)
	{
		int segment_index = 0;
		u16 address = 0;
		u8 value = 0;
		cJSON_ArrayForEach(item, segment)
		{
			if (segment_index == 0)
				address = item->valueint;
			else if (segment_index == 1)
				value = item->valueint;
			segment_index++;
		}
		test_context->memory[address] = value;
	}
}

static void serialize_cycles(cJSON *cycles, struct test_case *test_case)
{
	cJSON *cycle = NULL, *segment = NULL, *item = NULL;

	test_case->size = cJSON_GetArraySize(cycles);
	test_case->cycles = (struct context_cycle *)malloc(
		test_case->size * sizeof(struct context_cycle));
	cJSON_ArrayForEach(cycle, cycles)
	{
		int segment_index = 0;
		u16 pc = 0;
		u8 pc_value = 0;
		cJSON_ArrayForEach(item, segment)
		{
			if (segment_index == 0)
				pc = item->valueint;
			else if (segment_index == 1)
				pc_value = item->valueint;
			segment_index++;
		}
		test_case->cycles[segment_index].pc = pc;
		test_case->cycles[segment_index].pc_value = pc_value;
	}
}

static void serialize_test(cJSON *test, struct test_case *test_case)
{
	cJSON *initial = cJSON_GetObjectItem(test, "initial");
	cJSON *final = cJSON_GetObjectItem(test, "final");
	cJSON *cycles = cJSON_GetObjectItem(test, "cycles");

	serialize_test_context(initial, test_case->initial);
	serialize_test_context(final, test_case->final);
	serialize_cycles(cycles, test_case);
}

static struct test_suite *serialize_tests(char *buffer)
{
	struct test_suite *suite;
	cJSON *json = NULL, *json_test;

	json = cJSON_Parse(buffer);
	suite = (struct test_suite *)malloc(sizeof(struct test_suite));
	if (!suite)
		goto error;
	suite->total = cJSON_GetArraySize(json);
	suite->tests = (struct test_case **)malloc(sizeof(struct test_case *) *
						   suite->total);
	if (!suite->tests)
		goto free_test_suite;
	suite->size = 0;
	cJSON_ArrayForEach(json_test, json)
	{
		suite->tests[suite->size] = test_case_alloc();
		if (!suite->tests[suite->size]) {
			goto free_test_cases;
		}
		serialize_test(json_test, suite->tests[suite->size]);
		suite->size++;
	}
	cJSON_Delete(json);
	return suite;

free_test_cases:
	test_suite_destroy(suite);
free_test_suite:
	zfree(suite);
error:
	cJSON_Delete(json);
	return NULL;
}

static void sm83_reset_from_context(struct sm83_core *cpu, struct context *ctx)
{
	cpu->pc = ctx->pc;
	cpu->sp = ctx->sp;
	cpu->a = ctx->a;
	cpu->f = ctx->f;
	cpu->b = ctx->b;
	cpu->c = ctx->c;
	cpu->d = ctx->d;
	cpu->e = ctx->e;
	cpu->h = ctx->h;
	cpu->l = ctx->l;
	for (int addr = 0; addr < 0xFFFF + 1; addr++) {
		cpu->memory->write8(cpu, addr, ctx->memory[addr]);
	}
}

static void assert_register(const char *name, int result, int expected,
			    int *failed_assertions, bool verbose)
{
	if (result != expected) {
		if (verbose)
			printf("Register %s (result) $%02X(%03d) != (expected) $%02X(%03d)\n",
			       name, result, result, expected, expected);
		(*failed_assertions)++;
	}
}

static void assert_memory(u8 *result, u8 *expected, int *failed_assertions,
			  bool verbose)
{
	for (int i = 0; i < 0xFFFF + 1; i++) {
		if (result[i] != expected[i]) {
			if (verbose)
				printf("Memory [$%04X] (result) $%02X(%03d) != (expected) $%02X(%03d)\n",
				       i, result[i], result[i], expected[i],
				       expected[i]);
			(*failed_assertions)++;
		}
	}
}

static int assert_context(struct sm83_debugger *debugger, struct context *final,
			  bool verbose)
{
	int failed_assertions = 0;
	struct sm83_core *cpu = debugger->cpu;

	assert_register("A", cpu->a, final->a, &failed_assertions, verbose);
	assert_register("F", cpu->f, final->f, &failed_assertions, verbose);
	assert_register("B", cpu->b, final->b, &failed_assertions, verbose);
	assert_register("C", cpu->c, final->c, &failed_assertions, verbose);
	assert_register("D", cpu->d, final->d, &failed_assertions, verbose);
	assert_register("E", cpu->e, final->e, &failed_assertions, verbose);
	assert_register("H", cpu->h, final->h, &failed_assertions, verbose);
	assert_register("L", cpu->l, final->l, &failed_assertions, verbose);
	assert_register("SP", cpu->sp, final->sp, &failed_assertions, verbose);
	assert_register("PC", cpu->pc, final->pc, &failed_assertions, verbose);
	assert_memory(debugger->bus, final->memory, &failed_assertions,
		      verbose);
	return failed_assertions;
}

static int test_suite_run(struct test_suite *suite, bool verbose)
{
	int failed = 0;

	for (int i = 0; i < suite->size; i++) {
		if (verbose)
			printf("[%04d] Running %s\n", i, suite->path);
		struct sm83_debugger *debugger = sm83_debugger_init(NULL);
		sm83_reset_from_context(debugger->cpu,
					suite->tests[i]->initial);
		for (int c = 0; c < suite->tests[i]->size; c++) {
			sm83_cpu_step(debugger->cpu);
			if (verbose)
				sm83_cpu_debug(debugger->cpu);
		}
		if (assert_context(debugger, suite->tests[i]->final, verbose) >
		    0)
			failed++;
		sm83_debugger_destroy(debugger);
	}
	return failed;
}

static int test_with_file(char *path, bool verbose)
{
	char *buffer;
	struct test_suite *suite;
	int failed = 0;

	buffer = (char *)readfile(path);
	if (!buffer) {
		if (verbose)
			printf("Failed to open file\n");
		return -1;
	}
	suite = serialize_tests(buffer);
	zfree(buffer);
	suite->path = path;
	if (!suite) {
		if (verbose)
			printf("Failed to serialize test suite\n");
		goto free_buffer;
	}
	failed += test_suite_run(suite, verbose);
	test_suite_destroy(suite);
	return failed;

free_buffer:
	zfree(buffer);
	return -1;
}

static void run_all()
{
	char path[1024];
	int failed = 0;
	int failed_test = 0;
	int total_test = 0;

	for (int opcode = 0; opcode <= 0xFF; opcode++) {
		sprintf(path, "misc/sm83/v1/%02x.json", opcode);
		int failed_result = test_with_file(path, false);
		if (failed_result == -1)
			continue;
		if (failed_result > 0) {
			printf("Testing %s [FAIL]\n", path);
			failed_test++;
		}
		failed += failed_result;
		total_test++;
	}
	for (int opcode = 0; opcode <= 0xFF; opcode++) {
		sprintf(path, "misc/sm83/v1/cb %02x.json", opcode);
		int failed_result = test_with_file(path, false);
		if (failed_result == -1)
			continue;
		if (failed_result > 0) {
			printf("Testing %s [FAIL]\n", path);
			failed_test++;
		}
		failed += failed_result;
		total_test++;
	}
	printf("Failed %d/%d\n", failed_test, total_test);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		run_all();
	} else if (argc == 2) {
		test_with_file(argv[1], true);
	}
}
