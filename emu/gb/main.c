#include "emu/gb.h"
#include <stdlib.h>
#include <string.h>

// clang-format off
struct gb_option options[] = {
	{ "-d/--debug         Enable debugger", "--debug", "-d", 0, GB_OPTION_DEBUG },
	{ "-r/--rom <path>    Path of the ROM", "--rom", "-r", 1, GB_OPTION_ROM },
	{ "-n/--no-video      Disable video rendering", "--no-video", "-n", 0, GB_OPTION_NO_VIDEO },
	{ "-s/--scale <int>   Scale viewport", "--scale", "-s", 1, GB_OPTION_SCALE },
};
// clang-format on

static void print_help()
{
	printf("usage: gb [ARGS]\n");
	for (int i = 0; i < ARRAY_SIZE(options); i++)
		printf("   %s\n", options[i].description);
}

static void gb_context_init(struct gb_context *ctx)
{
	ctx->flags = GB_DEFAULT_FLAGS;
	ctx->rom_path = NULL;
	ctx->scale = 1;
	ctx->joypad = 0xFF;
}

static void parse_option(struct gb_context *ctx, int i, int argc, char **argv)
{
	for (int j = 0; j < ARRAY_SIZE(options); j++) {
		if (strcmp(argv[i], options[j].l) &&
		    strcmp(argv[i], options[j].s)) {
			continue;
		}
		switch (options[j].type) {
		case GB_OPTION_DEBUG:
			GB_FLAG_ENABLE(GB_DEBUG);
			break;
		case GB_OPTION_NO_VIDEO:
			GB_FLAG_ENABLE(GB_VIDEO);
			break;
		case GB_OPTION_ROM:
			if (i + 1 < argc)
				ctx->rom_path = argv[i + 1];
			break;
		case GB_OPTION_SCALE:
			if (i + 1 < argc)
				ctx->scale = atoi(argv[i + 1]);
			break;
		}
		break;
	}
}

static void print_header(struct gb_context *ctx)
{
	printf("Starting up ...\n");
	printf("Debug: %s ", GB_FLAG(GB_DEBUG) ? "On" : "Off");
	printf("Video: %s ", GB_FLAG(GB_VIDEO) ? "On" : "Off");
	printf("\n");
	printf("Rom: %s\n", ctx->rom_path ? ctx->rom_path : "Not loaded");
	printf("Scale: %d\n", ctx->scale);
}

static int context_create(struct gb_context *ctx, int argc, char **argv)
{
	gb_context_init(ctx);
	for (int i = 1; i < argc; i++)
		parse_option(ctx, i, argc, argv);
	if (!ctx->rom_path)
		return -1;
	if (ctx->scale < 1)
		return -1;
	return 0;
}

static int emulator_run(int argc, char **argv)
{
	struct gb_context ctx;
	if (context_create(&ctx, argc, argv)) {
		print_help();
		return 0;
	} else {
		print_header(&ctx);
	}
	gb_start_emulator(&ctx);
	gb_stop_emulator(&ctx);
	return ctx.exit_code;
}

int main(int argc, char **argv)
{
	if (emulator_run(argc, argv))
		printf("emulator terminate with errors\n");
	return 0;
}
