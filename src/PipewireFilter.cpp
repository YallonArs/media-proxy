#include "PipewireFilter.h"

#include <stdexcept>
#include <magic_enum.hpp>
#include <iostream>
#include <thread>

using namespace std::string_literals;

PipewireFilter::PipewireFilter(int argc, char *argv[], PipewireFilterProps props) : props(props) {
	pw_init(&argc, &argv);

	if (!props.server_sock.has_value()) {
		const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
		if (runtime_dir != nullptr) {
			props.server_sock = string(runtime_dir) + "/pipewire-0";
		} else {
			throw std::runtime_error("cannot get pipewire server path");
		}
	}

	props.name		  = props.name.value_or("media-proxy-filter");
	props.description = props.description.value_or("Filter for media-proxy");

	setenv("PIPEWIRE_REMOTE", props.server_sock.value().c_str(), 1);

	state.loop = pw_main_loop_new(nullptr);

	pw_filter_events filter_events {};
	filter_events.version = PW_VERSION_FILTER_EVENTS;
	filter_events.process = on_process;

	pw_properties *pw_props = pw_properties_new(
		PW_KEY_MEDIA_TYPE, "Audio",
		PW_KEY_MEDIA_CATEGORY, "Filter",
		PW_KEY_MEDIA_ROLE, "DSP",
		PW_KEY_NODE_NAME, props.name.value().c_str(),
		PW_KEY_NODE_DESCRIPTION, props.description.value().c_str(),
		// tell WirePlumber/Session Manager NOT to auto-connect this node
		PW_KEY_NODE_AUTOCONNECT, "false",
		nullptr
	);

	// create the filter node
	state.filter = pw_filter_new_simple(
		pw_main_loop_get_loop(state.loop),
		props.name.value().c_str(),
		pw_props,
		&filter_events,
		this
	);

	const char *format = "32 bit float mono audio";

	auto create_port = [this, format](spa_direction direction, char channel) {
		const string port_name = (direction == PW_DIRECTION_INPUT ? "In "s : "Out "s) + channel;
		const string audio_channel = "F"s + channel;

		return pw_filter_add_port(
			state.filter,
			direction, PW_FILTER_PORT_FLAG_MAP_BUFFERS, 0,
			pw_properties_new(
				PW_KEY_FORMAT_DSP, format,
				PW_KEY_PORT_NAME, port_name.c_str(),
				PW_KEY_AUDIO_CHANNEL, audio_channel.c_str(),
				nullptr
			),
			nullptr, 0
		);
	};

	auto create_channel = [create_port](char channel) {
		return Channel(
			create_port(PW_DIRECTION_INPUT, channel),
			create_port(PW_DIRECTION_OUTPUT, channel)
		);
	};

	state.channels.L = create_channel('L');
	state.channels.R = create_channel('R');

	if (pw_filter_connect(state.filter, PW_FILTER_FLAG_RT_PROCESS, nullptr, 0) < 0) {
		throw std::runtime_error("Failed to connect filter.");
	}
}

PipewireFilter::~PipewireFilter() {
	if (state.loop) stop();
	// join thread before destroying pipewire objects
	_thread = {};
	pw_filter_destroy(state.filter);
	pw_main_loop_destroy(state.loop);
	pw_deinit();
}

void PipewireFilter::on_process(void *userdata, spa_io_position *position) {
	(void)position;
	auto *filter = static_cast<PipewireFilter *>(userdata);

	filter->process_channel(filter->state.channels.L);
	filter->process_channel(filter->state.channels.R);
}

void PipewireFilter::process_channel(Channel ch) {
	void *in_port = ch.in;
	void *out_port = ch.out;

	if (!in_port || !out_port) return;

	// grab the next available buffers from PipeWire
	pw_buffer *b_in	 = pw_filter_dequeue_buffer(in_port);
	pw_buffer *b_out = pw_filter_dequeue_buffer(out_port);

	if (b_in != nullptr && b_out != nullptr) {
		spa_data *d_in	= b_in->buffer->datas;
		spa_data *d_out = b_out->buffer->datas;

		// use 32-bit floats
		float *src = static_cast<float *>(d_in[0].data);
		float *dst = static_cast<float *>(d_out[0].data);

		uint32_t n_samples = d_in[0].chunk->size / sizeof(float);

		if (src && dst) {
			props.callback(src, dst, n_samples);
		}

		// copy chunk metadata
		d_out[0].chunk->size   = d_in[0].chunk->size;
		d_out[0].chunk->stride = d_in[0].chunk->stride;
		d_out[0].chunk->offset = d_in[0].chunk->offset;
	}

	// return the buffers back to PipeWire
	if (b_in) pw_filter_queue_buffer(in_port, b_in);
	if (b_out) pw_filter_queue_buffer(out_port, b_out);
}

void PipewireFilter::start() {
	pw_main_loop_run(state.loop);
}

void PipewireFilter::stop() {
	pw_main_loop_quit(state.loop);
}


void PipewireFilter::start_in_thread() {
	_thread = std::jthread([this] { start(); });
}
