#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

struct config
{
	const char * executable;
	const char * working_directory;
	const char ** parameters;
	const char ** required_cpu_features;
	const char ** required_libs;

	unsigned int num_parameters;
	unsigned int num_required_cpu_features;
	unsigned int num_required_libs;
};

struct splashscreen
{
	const char * title;
	const char * background;
	const char * animation;
	const char * icon;
};

struct settings
{
	struct splashscreen * splashscreen;
	struct config * configs;
	unsigned int num_configs;
};

bool read_settings(const char * filename, struct settings * conf);

#endif // CONFIG_H
