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

#include "afbclient.h"
#include <string>
#include <cstring>
#include <iostream>
#include <mutex>
#include <condition_variable>

#undef DEBUG
//#define DEBUG

struct call_data
{
	bool sync;
	std::mutex mutex;
	std::condition_variable cv;
	bool ready;
	std::function<void(json_object*)> cb;
	json_object *resp;
};

static void on_hangup_cb(void *closure, struct afb_wsj1 *wsj)
{
}

static void on_call_cb(void *closure, const char *api, const char *verb, struct afb_wsj1_msg *msg)
{
}

static void on_reply_cb(void *closure, struct afb_wsj1_msg *msg)
{
	call_data *data = (call_data*) closure;
	struct json_object* reply;

	if(!data)
		goto reply_done;

	reply = afb_wsj1_msg_object_j(msg);
	if(reply) {
#ifdef DEBUG
		std::cerr << __FUNCTION__ << ": reply = " << \
			json_object_to_json_string_ext(reply, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY) << \
			std::endl;
#endif
		if(data->sync) {
			data->resp = reply;

			// Increase reference count since we are going to use
			// reply after this callback returns, caller must do a
			// put.
			json_object_get(reply);
		} else if(data->cb != nullptr) {
			data->cb(reply);
		}
	}
reply_done:
	if(data->sync) {
		// Signal reply is done
		{
			std::lock_guard<std::mutex> lk(data->mutex);
			data->ready = true;
		}
		data->cv.notify_one();
	}
}

//
// on_event_cb is inline in afbclient.h
//

static void *afb_loop_thread(struct sd_event* loop)
{
	for(;;)
		sd_event_run(loop, 30000000);
}

AfbClient::AfbClient(const int port, const std::string &token)
{
	std::string uri;

	if(sd_event_new(&m_afb_loop) < 0) {
		std::cerr << __FUNCTION__ << ": Failed to create event loop" << std::endl;
		return;
	}

	// Initialize interface for websocket
	m_itf.on_hangup = on_hangup_cb;
	m_itf.on_call = on_call_cb;
	m_itf.on_event = on_event_cb;

	uri = "ws://localhost:" + std::to_string(port) + "/api?token=" + token;
#ifdef DEBUG
	std::cerr << "Using URI: " << uri << std::endl;
#endif
	m_ws = afb_ws_client_connect_wsj1(m_afb_loop, uri.c_str(), &m_itf, this);
	if(m_ws) {
		m_afb_thread = std::thread(afb_loop_thread, m_afb_loop);
	} else {
		std::cerr << __FUNCTION__ << ": Failed to create websocket connection" << std::endl;
		goto error;
	}

	m_valid = true;
	return;
error:
	if(m_afb_loop) {
		sd_event_unref(m_afb_loop);
		m_afb_loop = nullptr;
	}
	return;
}

AfbClient::~AfbClient(void)
{
	sd_event_unref(m_afb_loop);
	afb_wsj1_unref(m_ws);
}

int AfbClient::call(const std::string &api, const std::string &verb, struct json_object *arg, reply_callback_fn cb)
{
	if(!m_valid)
		return -1;

	call_data data;
	data.sync = false;
	data.cb = cb;
	int rc = afb_wsj1_call_j(m_ws, api.c_str(), verb.c_str(), arg, on_reply_cb, (void*) &data);
	if(rc < 0) {
		std::cerr << __FUNCTION__ << \
			": Failed to call " << \
			api.c_str() << \
			"/" << \
			verb.c_str() << \
			std::endl;
	}
	return rc;
}

int AfbClient::call_sync(const std::string &api, const std::string &verb, struct json_object *arg, struct json_object **resp)
{
	if(!m_valid)
		return -1;

	call_data data;
	data.sync = true;
	data.ready = false;
	data.cb = nullptr;
	int rc = afb_wsj1_call_j(m_ws, api.c_str(), verb.c_str(), arg, on_reply_cb, (void*) &data);
	if(rc >= 0) {
		// Wait for response
		std::unique_lock<std::mutex> lk(data.mutex);
		data.cv.wait(lk, [&]{ return data.ready; });

		if(resp)
			*resp = data.resp;
		else
			json_object_put(data.resp);
	} else {
		std::cerr << __FUNCTION__ << \
			": Failed to call " << \
			api.c_str() << \
			"/" << \
			verb.c_str() << \
			std::endl;
	}
	return rc;
}

// subscribe call for simple forms like { "event": "foo" } or { "signal": "foo" }
int AfbClient::subscribe(const std::string &api, const std::string &event, const std::string &eventValueString)
{
	if(!m_valid)
		return -1;

        struct json_object *j_obj = json_object_new_object();
        json_object_object_add(j_obj, eventValueString.c_str(), json_object_new_string(event.c_str()));
        return call_sync(api, std::string("subscribe"), j_obj);
}

// subscribe call allowing passing in complete json_object for argument, for subscribing with e.g. arrays
int AfbClient::subscribe(const std::string &api, json_object *j_obj, const std::string &subscribeValueString)
{
	if(!m_valid || j_obj == nullptr)
		return -1;

        return call_sync(api, subscribeValueString, j_obj);
}

// unsubscribe call for simple forms like { "event": "foo" } or { "signal": "foo" }
int AfbClient::unsubscribe(const std::string &api, const std::string &event, const std::string &eventValueString)
{
	if(!m_valid)
		return -1;

        struct json_object *j_obj = json_object_new_object();
        json_object_object_add(j_obj, eventValueString.c_str(), json_object_new_string(event.c_str()));
        return call_sync(api, std::string("unsubscribe"), j_obj);
}

// unsubscribe call allowing passing in complete json_object for argument, for unsubscribing with e.g. arrays
int AfbClient::unsubscribe(const std::string &api, json_object *j_obj, const std::string &unsubscribeValueString)
{
	if(!m_valid || j_obj == nullptr)
		return -1;

        return call_sync(api, unsubscribeValueString, j_obj);
}

void AfbClient::set_event_callback(event_callback_fn cb, void *closure)
{
	m_event_cb = cb;
	m_event_cb_closure = closure;
}

void AfbClient::on_event(const char* event, struct afb_wsj1_msg *msg)
{
	if(m_event_cb == nullptr)
		return;

	struct json_object *contents = afb_wsj1_msg_object_j(msg);
#ifdef DEBUG
	std::cerr << __FUNCTION__ << ": contents = " << \
		json_object_to_json_string_ext(contents, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY) << \
		std::endl;
#endif
	struct json_object *data;
	if(json_object_object_get_ex(contents, "data", &data)) {
		m_event_cb(event, data, m_event_cb_closure);
	}
}
