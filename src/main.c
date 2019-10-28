#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#ifdef __linux__
#include <unistd.h>
#include <signal.h> // sigaction(), sigsuspend(), sig*()
#include <sys/wait.h>

#include <cpuid.h>
#include <dlfcn.h>
#endif // __linux__

#ifdef WIN32
#include <Windows.h>
#endif

#include "settings.h"
#include "splashscreen.h"

#ifdef _MSC_VER 
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

int find_supported_config(struct settings conf)
{
	int eax, ebx, ecx, edx;

#if __linux__
	__cpuid(0, eax, ebx, ecx, edx);
#else
	{
		int regs[4];
		__cpuid(regs, 0);
		eax = regs[0];
		ebx = regs[1];
		ecx = regs[2];
		edx = regs[3];
	}
#endif

	printf("EAX: %x\nEBX: %x\nECX: %x\nEDX: %x\n", eax, ebx, ecx, edx);

#if __linux__
	if (ebx == signature_AMD_ebx && ecx == signature_AMD_ecx && edx == signature_AMD_edx)
	{
		printf("AuthenticAMD\n");
	}
	else if (ebx == signature_INTEL_ebx && ecx == signature_INTEL_ecx && edx == signature_INTEL_edx)
	{
		printf("GenuineIntel\n");
	}
#endif

#if __linux__
	__cpuid(1, eax, ebx, ecx, edx);
#else
	{
		int regs[4];
		__cpuid(regs, 1);
		eax = regs[0];
		ebx = regs[1];
		ecx = regs[2];
		edx = regs[3];
	}
#endif

	const char * features_names[] =
	{
		"MMX",
		"SSE",
		"SSE2",
		"SSE3",
		"SSE4_1",
		"SSE4_2",
		"AVX",
		"AVX2",
	};

	bool supported_features[sizeof(features_names) / sizeof(features_names[0])] =
	{
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
	};

#if __linux__
	if (edx & bit_MMX)
	{
		supported_features[0] = true;
	}
	if (edx & bit_SSE)
	{
		supported_features[1] = true;
	}
	if (edx & bit_SSE2)
	{
		supported_features[2] = true;
	}
	if (ecx & bit_SSE3)
	{
		supported_features[3] = true;
	}
	if (ecx & bit_SSE4_1)
	{
		supported_features[4] = true;
	}
	if (ecx & bit_SSE4_2)
	{
		supported_features[5] = true;
	}
	if (ecx & bit_AVX)
	{
		supported_features[6] = true;
	}

	supported_features[7] = false; // AVX2
#endif

	printf("EAX: %x\nEBX: %x\nECX: %x\nEDX: %x\n", eax, ebx, ecx, edx);

	printf("-----------------------------\n");

	int first_supported_config = -1;

	for (unsigned int i = 0; i < conf.num_configs; ++i)
	{
		printf("CONFIG %d\n", i);

		bool config_supported = true;

		for (unsigned int j = 0; j < conf.configs[i].num_required_libs; ++j)
		{
#if __linux__
			void * lib = dlopen(conf.configs[i].required_libs[j], RTLD_LAZY);

			if (lib != NULL)
			{
				printf("%s : FOUND \n", conf.configs[i].required_libs[j]);
			}
			else
			{
				printf("%s : NOT FOUND \n", conf.configs[i].required_libs[j]);
				config_supported = false;
			}

			dlclose(lib);
#endif // __linux__
		}

		for (unsigned int j = 0; j < conf.configs[i].num_required_cpu_features; ++j)
		{
			bool found = false;

			for (unsigned int feature_index = 0; feature_index < sizeof(features_names) / sizeof(features_names[0]); ++feature_index)
			{
				if (0 == strcasecmp(conf.configs[i].required_cpu_features[j], features_names[feature_index]))
				{
					found = true;

					if (supported_features[feature_index])
					{
						printf("%s : SUPPORTED \n", features_names[feature_index]);
					}
					else
					{
						printf("%s : UNSUPPORTED \n", features_names[feature_index]);
						config_supported = false;
					}
				}
			}

			if (!found)
			{
				printf("Unknown CPU feature : %s\n", conf.configs[i].required_cpu_features[j]);
				config_supported = false;
			}
		}

		if (config_supported)
		{
			first_supported_config = i;
			printf("CONFIG %d SUPPORTED\n", i);
			break;
		}

		printf("-----------------------------\n");
	}

	return first_supported_config;
}

#ifdef __linux__
void handle_signal(int signal)
{
	const char *signal_name;

	switch (signal)
	{
		case SIGCHLD:
		{
			signal_name = "SIGCHLD";
		}
		break;

		case SIGUSR1:
		{
			signal_name = "SIGUSR1";
		}
		break;

		default:
		{
			fprintf(stderr, "Error: caught wrong signal: %d\n", signal);
			return;
		}
	}

	hide_splashscreen();

	printf("Done handling %s\n\n", signal_name);
}
#endif // __linux__

#ifdef WIN32
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	int argc = __argc;
	char **argv = __argv;
#else
int main (int argc, char **argv)
{
#endif
	if (argc < 2)
	{
		fprintf(stderr, "Error: missing config file parameter\n");
		return -1;
	}

	struct settings conf;

	if (!read_settings(argv[1], &conf))
	{
		fprintf(stderr, "Error: invalid file\n");
		return -1;
	}

	bool skip_splashscreen = false;

	// ------------------------------------------------------------------

#ifdef __linux__
	struct sigaction sa;

	// Setup the sighub handler
	sa.sa_handler = &handle_signal;

	// Restart the system call, if at all possible
	sa.sa_flags = SA_RESTART;

	// Block every signal during the handler
	sigfillset(&sa.sa_mask);

	if (sigaction(SIGUSR1, &sa, NULL) == -1)
	{
		perror("Error: cannot handle SIGUSR1"); // Should not happen
		skip_splashscreen = true;
	}
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
	{
		perror("Error: cannot handle SIGCHLD"); // Should not happen
		skip_splashscreen = true;
	}

	// ------------------------------------------------------------------

	pid_t p = fork();

	if (p < 0)
	{
		perror("Error: could not fork");
		return -2;
	}
	else if (p == 0)
	{
		int first_supported_config = find_supported_config(conf);

		fflush(stdout);

		if (first_supported_config >= 0 && first_supported_config < (int)conf.num_configs)
		{
			const char ** parameters = malloc(sizeof(char*) * (conf.configs[first_supported_config].num_parameters + 2));

			parameters[0] = conf.configs[first_supported_config].executable;

			for (unsigned int i = 0; i < conf.configs[first_supported_config].num_parameters; ++i)
			{
				parameters[i+1] = conf.configs[first_supported_config].parameters[i];
			}

			parameters[(conf.configs[first_supported_config].num_parameters + 1)] = NULL;

			execvp(conf.configs[first_supported_config].executable, (char * const *)parameters);
		}
		else
		{
			printf("NO CONFIG SUPPORTED\n");
			return(-2);
		}

		return(-1);
	}
#endif // __linux__

	const char * SteamTenfoot = getenv("SteamTenfoot");

	if (SteamTenfoot != NULL && !strcmp(SteamTenfoot, "1"))
	{
		skip_splashscreen = true; // FULLSCREEN / PREVENT SPLASHSCREEN
	}

	if (conf.splashscreen == NULL)
	{
		skip_splashscreen = true; // NO SPLASHSCREEN
	}

	// ------------------------------------------------------------------

#ifdef __linux__
	if (!skip_splashscreen)
	{
		const char * XDG_SESSION_TYPE = getenv("XDG_SESSION_TYPE");

		if (!strcasecmp(XDG_SESSION_TYPE, "x11"))
		{
			show_splashscreen(conf.splashscreen->title, conf.splashscreen->background, conf.splashscreen->animation, conf.splashscreen->icon);
		}
		else if (!strcasecmp(XDG_SESSION_TYPE, "wayland"))
		{
			// TODO
		}
		else
		{
			// unsupported (Mir?)
		}
	}

	int status = 0;
	waitpid(p, &status, 0);
	printf("child ended with code : %d\n", WEXITSTATUS(status));
#else
	int first_supported_config = find_supported_config(conf);

	if (first_supported_config >= 0 && first_supported_config < (int)conf.num_configs)
	{
		unsigned int total_len = strlen(conf.configs[first_supported_config].executable);

		for (unsigned int i = 0; i < conf.configs[first_supported_config].num_parameters; ++i)
		{
			total_len += 1 + strlen(conf.configs[first_supported_config].parameters[i]);
		}

		char * parameters = malloc(sizeof(char) * (total_len + 1));
		memset(parameters, 0, sizeof(char) * (total_len + 1));

		char * current_parameters = parameters;

		strcpy(current_parameters, conf.configs[first_supported_config].executable);

		current_parameters += strlen(conf.configs[first_supported_config].executable);

		for (unsigned int i = 0; i < conf.configs[first_supported_config].num_parameters; ++i)
		{
			*current_parameters = ' ';
			current_parameters += 1;

			strcpy(current_parameters, conf.configs[first_supported_config].parameters[i]);

			current_parameters += strlen(conf.configs[first_supported_config].parameters[i]);

			*current_parameters = '\0';
		}

		STARTUPINFO info = { sizeof(info) };
		PROCESS_INFORMATION processInfo;
		if (CreateProcess(conf.configs[first_supported_config].executable, lpCmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
		{
			WaitForSingleObject(processInfo.hProcess, INFINITE);
			CloseHandle(processInfo.hProcess);
			CloseHandle(processInfo.hThread);
		}

		if (!skip_splashscreen)
		{
			show_splashscreen_win32(hInstance, conf.splashscreen->title, conf.splashscreen->background, conf.splashscreen->animation, conf.splashscreen->icon);
		}
	}
#endif

	// TODO : free config

	return 0;
}
