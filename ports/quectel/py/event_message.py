__all__ = [
    "EventMessageObject",
    "Event",
    "EventManager"
]

import _thread


class EventMessageObject(object):
    def __init__(self, event, msg, callback=None):
        self.__event = event
        self.__msg = msg
        self.__callback = callback

    @property
    def callback(self):
        return self.__callback

    @property
    def event(self):
        return self.__event

    @property
    def event_name(self):
        return self.__event.name

    @property
    def msg(self):
        return self.__msg

    def apply(self):
        self.event(event_message=self)


class Queue(object):
    def __init__(self):
        self.items = []

    def enqueue(self, item):
        self.items.append(item)

    def dequeue(self):
        return self.items.pop(0)

    def empty(self):
        return self.size() == 0

    def size(self):
        return len(self.items)


class Event(list):

    def __init__(self, name, *args):
        super(Event, self).__init__(args)
        self.event_manager = None
        self.__name = name

    @property
    def name(self):
        return self.__name

    def clear(self):
        del self[:]

    def add_handler(self, handler):
        if not callable(handler):
            raise TypeError("'%s' is not callable." % handler)

        self.append(handler)
        return handler

    def add_handler_via(self):
        def decorator(fn):
            self.add_handler(fn)
            return fn

        return decorator

    def remove_handler(self, handler):
        self.remove(handler)

    def post(self, message, callback=None):
        ms = EventMessageObject(self, message, callback)
        if self.event_manager is not None:
            self.event_manager.post_msg(ms)

    def fire(self, *args, **kwargs):
        for handler in self:  # Iterate over handlers
            try:
                handler(*args, **kwargs)  # Execute handler with given args.
            except StopIteration:  # Stop iterating if handler raised StopIter
                break
            except Exception as e:
                pass
            else:
                pass

    def __call__(self, *args, **kwargs):
        self.fire(*args, **kwargs)


class EventManager(object):
    def __init__(self):
        self.__eventQueue = Queue()

        self.__active = 0

        self.__lock_post = _thread.allocate_lock()

        self.__lock_get = _thread.allocate_lock()

        self.__lock_get.acquire()

    def register_event(self, event):
        event.event_manager = self

    def remove_event(self, event):
        if self in event.event_managers:
            event.event_managers.remove(self)

    def wait(self):
        self.__lock_get.acquire()
        em = self.__eventQueue.dequeue()
        return em

    def run(self):
        if not self.status:
            self.__active = 1
        while self.status:
            self.__lock_get.acquire()
            self.__process_event()

    def start(self):
        if not self.status:
            self.__active = 1
            _thread.start_new_thread(self.run, ())
        return self.status

    @property
    def status(self):
        return self.__active

    def stop(self):
        if self.status:
            self.__active = 0
            if self.__lock_get.locked():
                self.__lock_get.release()
        return self.status

    def __process_event(self):
        for em in range(self.__eventQueue.size()):
            em = self.__eventQueue.dequeue()
            em.apply()

    def send(self, em):
        """

        :param em: event message
        :return:
        """
        self.__eventQueue.enqueue(em)

    def post_msg(self, em):
        # lock post
        self.__lock_post.acquire()
        self.send(em)
        self.__lock_post.release()
        if self.__lock_get.locked():
            # locked release
            self.__lock_get.release()

