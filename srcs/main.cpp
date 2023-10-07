#include <ios>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "Server/EventHandler.hpp"
#include "Util/StreamReader.hpp"
#include "Util/Logger.hpp"

volatile bool should_continue;

static void sigint_handler(int signal_code)
{
	(void)signal_code;
	should_continue = false;
}

/// @return false: FAIL
static bool parse_config(const char *file, Config &config)
{
	std::ifstream input(file, std::ios::in | std::ios::binary);
	if (input.fail())
	{
		std::cerr << "Error: Input file " << file << " does not exist." << std::endl;
		return false;
	}
	std::vector<char> array((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
	const std::vector<char> &ref = array;
	std::vector<char>::const_iterator itr = ref.begin();
	try
	{
		StreamReader::read(itr, ref.end(), config);
		config.make_servers_disposable();
		config.validate();
	}
	catch (std::exception &e)
	{
		Logger::log_error(e);
		return false;
	}
	return true;
}

static bool prepare_standard_inout()
{
	int null_fd = ::open("/dev/null", O_RDWR);
	if (null_fd < 0)
		return false;
	if (null_fd == STDIN_FILENO)
	{
		if (::dup2(null_fd, STDOUT_FILENO) < 0)
			return false;
	}
	else
	{
		if (null_fd == STDOUT_FILENO)
		{
			if (::dup2(null_fd, STDIN_FILENO) < 0)
				return false;
		}
		else
		{
			if (::dup2(null_fd, STDIN_FILENO) < 0)
				return false;
			if (::dup2(null_fd, STDOUT_FILENO) < 0)
				return false;
		}
	}
	::close(null_fd);
	return true;
}

int main(int argc, char **argv, char **envp)
{
	HttpStatusCode::init();
	const char *config_file_path = "./default.conf";
	if (argc >= 2)
		config_file_path = argv[1];
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, sigint_handler);
	if (!prepare_standard_inout())
	{
		std::cerr << "Error: standard input and output preparation failed. " << current_time << std::endl;
		return 1;
	}
	EnvironmentVariableDictionary dictionary(envp);
	Config config(dictionary);
	if (!parse_config(config_file_path, config))
	{
		std::cerr << "Error: Config file parse failed. Path: " << config_file_path << " " << current_time << std::endl;
		return 1;
	}
	if (!config.prepare())
	{
		std::cerr << "Error: Error Pages preparation failed. " << current_time << std::endl;
		return 1;
	}
	EventHandler handler(config);
	if (!handler.prepare_listeners())
	{
		std::cerr << "Error: Opening listening port(s) has failed. " << current_time << std::endl;
		return 1;
	}
	should_continue = true;
	while (should_continue && handler.not_empty())
	{
		try
		{
			handler.wait(config.get_wait_time());
			handler.process_listen_sockets();
			handler.process_write_resources();
			handler.process_read_resources();
			handler.process_connections();
			handler.process_requests();
			handler.cull_connections();
		}
		catch (const std::bad_alloc &e)
		{
			handler.emergency_free(e);
		}
	}
	return 0;
}
