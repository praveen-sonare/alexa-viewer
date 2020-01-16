/*
 * Copyright (C) 2019,2020 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AFBCLIENT_H
#define AFBCLIENT_H

#include <string>
#include <thread>
#include <map>
#include <functional>
#include <systemd/sd-event.h>
#include <json-c/json.h>

extern "C"
{
#include <afb/afb-wsj1.h>
#include <afb/afb-ws-client.h>
}

class AfbClient
{
public:
	AfbClient(int port, const std::string &token);
	~AfbClient();

	using reply_callback_fn = std::function<void(json_object*)>;
	using event_callback_fn = std::function<void(const char*, json_object*, void*)>;

	int call(const std::string &api, const std::string &verb, struct json_object* arg, reply_callback_fn cb = nullptr);
	int call_sync(const std::string &api, const std::string &verb, struct json_object* arg, struct json_object **resp = NULL);
	int subscribe(const std::string &api, const std::string &event, const std::string &eventValueString = "event");
	int subscribe(const std::string &api, json_object *j_obj, const std::string &subscribeValueString = "subscribe");
	int unsubscribe(const std::string &api, const std::string &eventString, const std::string &eventValueString = "event");
	int unsubscribe(const std::string &api, json_object *j_obj, const std::string &unsubscribeValueString = "unsubscribe");
	void set_event_callback(event_callback_fn cb, void *closure);

	static void on_event_cb(void *closure, const char* event, struct afb_wsj1_msg *msg) {
		if(closure)
			static_cast<AfbClient*>(closure)->on_event(event, msg);
	}

private:
	struct afb_wsj1 *m_ws = nullptr;
	struct afb_wsj1_itf m_itf;
	std::thread m_afb_thread;
	sd_event *m_afb_loop = nullptr;
	bool m_valid = false;
	event_callback_fn m_event_cb = nullptr;
	void *m_event_cb_closure = nullptr;

	void on_event(const char* event, struct afb_wsj1_msg *msg);
};

#endif // AFBCLIENT_H
