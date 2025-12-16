#include "./msg_queue.hh"

MSGQueue::MSGQueue() {}

MSGQueue::~MSGQueue() {}

// push a ``msg'' to the msg queue
// we assume that the msg queue has no capacity limit
void MSGQueue::push_msg(const std::string &msg) {
    std::unique_lock<std::mutex> lock(_mtx);
    _msg_queue.push(msg);
    _condition.notify_one();
}

// pop a msg in the front of msg queue if it is not empty
// return a std::string variable representing the msg
std::string MSGQueue::pop_msg() {
    std::unique_lock<std::mutex> lock(_mtx);
	std::string msg;
    _condition.wait(lock, [&]() -> bool {
        return !_msg_queue.empty();
    });
    msg = _msg_queue.front();
    _msg_queue.pop();
    return msg;
}