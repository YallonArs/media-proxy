#include <functional>
#include <optional>
#include <string>
#include <thread>

#include <pipewire/filter.h>
#include <pipewire/pipewire.h>

using std::string;

struct PipewireFilterProps {
	std::optional<string> name;
	std::optional<string> description;
	std::optional<string> server_sock;
	std::function<void(float *, float *, uint32_t)> callback;
};

class PipewireFilter {
private:
	typedef void *Port;
	struct Channel {
		Port in	 = nullptr,
			 out = nullptr;
	};
	struct Channels {
		Channel L, R;
	};
	struct FilterState {
		pw_main_loop *loop = nullptr;
		pw_filter *filter  = nullptr;

		Channels channels;
	};

	void process_channel(Channel ch);
	static void on_process(void *userdata, spa_io_position *position);
	void start();
	void stop();

	FilterState state;
	std::jthread _thread;

protected:
	PipewireFilterProps props;

public:
	PipewireFilter(int argc, char *argv[], PipewireFilterProps props);
	~PipewireFilter();

	void start_in_thread();
};
