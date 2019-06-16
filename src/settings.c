#include "settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jansson.h>

static const char ** parse_string_array(json_t * root, unsigned int * num_strings)
{
	if (root == NULL || !json_is_array(root))
	{
		return NULL;
	}

	size_t count = json_array_size(root);

	if (count == 0)
	{
		return NULL;
	}

	const char ** strings = malloc(sizeof(char*) * count);
	memset(strings, 0, sizeof(char*) * count);

	*num_strings = 0;

	unsigned int index = 0;
	json_t * value = NULL;

	json_array_foreach(root, index, value)
	{
		if (json_is_string(value))
		{
			strings[*num_strings] = json_string_value(value);
			(*num_strings)++;
		}
	}

	if (*num_strings == 0)
	{
		free(strings);
		return NULL;
	}

	return strings;
}

static bool parse_config(json_t * root, struct config * config)
{
	json_t * executable = NULL;
	json_t * working_directory = NULL;
	json_t * parameters = NULL;
	json_t * required_cpu_features = NULL;
	json_t * required_libs = NULL;

	// get all properties
	const char * key = NULL;
	json_t * value = NULL;

	json_object_foreach(root, key, value)
	{
		if (!strcmp("executable", key))
		{
			executable = value;
		}
		else if (!strcmp("working_directory", key))
		{
			working_directory = value;
		}
		else if (!strcmp("parameters", key))
		{
			parameters = value;
		}
		else if (!strcmp("required_cpu_features", key))
		{
			required_cpu_features = value;
		}
		else if (!strcmp("required_libs", key))
		{
			required_libs = value;
		}
		else
		{
			// NOT SUPPORTED
		}
	}

	if (!executable || !json_is_string(executable))
	{
		return false;
	}

	config->executable = json_string_value(executable);
	config->working_directory = json_string_value(working_directory);

	config->parameters = parse_string_array(parameters, &config->num_parameters);
	config->required_cpu_features = parse_string_array(required_cpu_features, &config->num_required_cpu_features);
	config->required_libs = parse_string_array(required_libs, &config->num_required_libs);

	return true;
}

static struct config * parse_configs(json_t * root, unsigned int * num_configs)
{
	if (!root || !json_is_array(root))
	{
		return NULL;
	}

	size_t count = json_array_size(root);

	if (count == 0)
	{
		return NULL;
	}

	struct config * configs = calloc(1, sizeof(struct config) * count);

	*num_configs = 0;

	unsigned int index = 0;
	json_t * value = NULL;

	json_array_foreach(root, index, value)
	{
		if (json_is_object(value))
		{
			if (parse_config(value, &configs[*num_configs]))
			{
				(*num_configs)++;
			}
		}
	}

	if (*num_configs == 0)
	{
		free(configs);
		return NULL;
	}

	return configs;
}

static struct splashscreen * parse_splashscreen(json_t * root)
{
	if (!root || !json_is_object(root))
	{
		return NULL;
	}

	struct splashscreen * splashscreen = calloc(1, sizeof(struct splashscreen));

	// get all properties
	const char * key = NULL;
	json_t * value = NULL;

	json_object_foreach(root, key, value)
	{
		if (!strcmp("title", key))
		{
			splashscreen->title = json_string_value(value);
		}
		else if (!strcmp("background", key))
		{
			splashscreen->background = json_string_value(value);
		}
		else if (!strcmp("animation", key))
		{
			splashscreen->animation = json_string_value(value);
		}
		else if (!strcmp("icon", key))
		{
			splashscreen->icon = json_string_value(value);
		}
	}

	return splashscreen;
}

bool read_settings(const char * filename, struct settings * conf)
{
	FILE * f = fopen(filename, "r");

	if (!f)
	{
		fprintf(stderr, "Error: can't open file\n");
		return false;
	}

	json_error_t error;
	json_t * root = json_loadf(f, 0, &error);

	fclose(f);

	if (!root)
	{
		fprintf(stderr, "Error: on line %d: %s\n", error.line, error.text);
		return false;
	}

	if (!json_is_object(root))
	{
		fprintf(stderr, "Error: root is not an object\n");
		json_decref(root);
		return false;
	}

	struct splashscreen * splashscreen = NULL;
	unsigned int num_configs = 0;
	struct config * configs = NULL;

	// get all properties
	const char * key = NULL;
	json_t * value = NULL;

	json_object_foreach(root, key, value)
	{
		if (!strcmp("splashscreen", key))
		{
			splashscreen = parse_splashscreen(value);
		}
		else if (!strcmp("configurations", key))
		{
			configs = parse_configs(value, &num_configs);
		}
		else
		{
			// NOT SUPPORTED
		}
	}

	if (!configs)
	{
		return false;
	}

	conf->splashscreen = splashscreen;
	conf->configs = configs;
	conf->num_configs = num_configs;

	return true;
}
