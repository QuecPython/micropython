__all__ = [
    "subscribe",
    "unsubscribe",
    "publish",
    "publish_async",
    "publish_sync",
    "Lock"
]

import _thread


class Lock(object):
    def __init__(self):
        self.lock = _thread.allocate_lock()

    def acquire(self):
        self.lock.acquire()

    def release(self):
        self.lock.release()

    def __enter__(self):
        self.acquire()

    def __exit__(self, *args, **kwargs):
        self.release()


class TopicMap(object):
    def __init__(self):
        self.topic_callback_map = dict()
        self.lock = Lock()

    def append(self, topic, cb):
        with self.lock:
            if topic is not None and topic not in self.topic_callback_map:
                self.topic_callback_map[topic] = list()
            self.topic_callback_map[topic].append(cb)

    def delete(self, topic, cb):
        with self.lock:
            if cb is not None:
                for i, v in enumerate(self.topic_callback_map[topic]):
                    if cb.__class__.__name__ == "function":
                        if v == cb:
                            del self.topic_callback_map[topic][i]
                            return True
                    if cb.__class__.__name__ == "bound_method" and v.__class__.__name__ == "bound_method":
                        if cb.get_info() == v.get_info():
                            del self.topic_callback_map[topic][i]
                            return True
                return False
            else:
                del self.topic_callback_map[topic]
                return True

    def fire_async(self, topic, msg):
        with self.lock:
            cb_list = self.topic_callback_map.get(topic, list())
        for cb in cb_list[:]:
            _thread.start_new_thread(cb, (topic, msg))

    def fire_sync(self, topic, msg):
        with self.lock:
            cb_list = self.topic_callback_map.get(topic, list())
        for cb in cb_list[:]:
            cb(topic, msg)

    def show(self, topic):
        if topic is not None:
            return self.topic_callback_map[topic]
        else:
            return self.topic_callback_map.copy()


topic_map = TopicMap()


def sub_table(topic=None):
    return topic_map.show(topic)


def subscribe(topic, cb):
    """
    subscribe topic and cb
    """
    return topic_map.append(topic, cb)


def publish(topic, msg):
    """
    publish topic and msg
    """
    publish_async(topic, msg)


def unsubscribe(topic, cb=None):
    """
    cancel subscribe
    """
    return topic_map.delete(topic, cb)


def publish_async(topic, msg):
    topic_map.fire_async(topic, msg)


def publish_sync(topic, msg):
    topic_map.fire_sync(topic, msg)
