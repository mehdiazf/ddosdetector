# How the system works (dev) #
## Streams ##
![ddosdetector](images/dev-scheme.png)

The main thread of the daemon (**main**) creates several types of threads for
different tasks:

* packet handler threads **netmap receiver** (their number is equal to the
number of rxring queues of the network card);
* stream **watcher**;
* thread **task runner**;
* flow **controld**.

The **netmap receiver** handler threads are created one for each ring queue
(*rxring*) of incoming network card packets. The *NetmapReceiver* class (the
*collector.hpp*file) calculates the available number of queues and creates
threads. Each *netmap receiver* thread, independently of the others, processes
its packets from its queue and checks them against a separate copy of the rules
(the *RulesCollection* file *class rules.cpp*). A separate copy of the rules for
each thread allows you to avoid unnecessary locks when checking packets (if you
make one thread-safe version of the rules collection for all threads, then while
one thread is working with this collection, the rest will wait). The *netmap
receiver* thread only checks the received packets against the rules and
increments the counters in its copy of the rules collection, while other threads
check triggers, calculate counter deltas, and perform tasks. That is, the
processor threads spend CPU time only on the fastest possible parsing of packets
from the network card queues.

Stream **watcher** (function *wathcer()* in the file *ddosdetector.cpp*)
performs multiple tasks:

* keeps the rule collections up-to-date for netmap receiver handler threads;
* updates collections of thread-handler rules;
* calculates the delta values of the counters (bytes per second, packets per
second);
* checks rule triggers and adds trigger jobs to the job queue.

To check collections of handler threads, the watcher thread works with a vector
containing shared_ptr pointers to these collections. The rule collections are
checked for up-to-date once per second. The collection of rules of each handler
thread is compared with the reference collection, and if they differ, the
collection of the handler thread is updated. The reference collection is also
used to store the totaled counters from all the handler threads, i.e. the
reference collection contains up-to-date data on the received traffic. The
delta values are calculated from the reference collection (after the counters
have been collected from all the handler threads). Then, also using the
reference collection, the triggers are checked, and if any of the triggers are
triggered, the TraggerAction task (*action.hpp* file) is added to the task
queue. The task queue itself is processed by another thread - **task runner**.

The **task runner** thread waits for a trigger job by checking the job queue,
as soon as a job appears in the queue it starts. Tasks run in the background,
and the thread does not wait for the task to finish.

The **controld** thread is responsible for the system management console. This
thread starts the TCP / UNIX socket server. When a user connects to the console
and makes changes to the rule set, the changes are made in the reference
collection and only after a second (maximum) are synchronized across all the
handler threads.

## Synchronization tools and shared data ##
The rules collection class **RulesCollection** (file *rules.hpp*), contains a
set of thread-safe rule lists **RulesList** (file *rules.hpp*), one for each
L4 protocol. Each **RulesList** is a public member of the RulesCollection
class that is handled directly. Synchronization of the threads is performed at
the level of each RulesList using boost::shared_mutex. Operations: batch
checks; counter updates; trigger checks-are performed immediately with all
RulesList elements in a single mutex lock.

## Files ##
The project has the following file structure
```bash
$ tree
.
├── action.cpp
├── action.hpp
├── collector.cpp
├── collector.hpp
├── controld.cpp
├── controld.hpp
├── ddosdetector.cpp
├── docs
│ ├── EXAMPLE_RULES.md
│ └── INFLUXDB.md
├── exceptions.cpp
├── exceptions.hpp
├── functions.cpp
├── functions.hpp
├── influxdb.cpp
├── influxdb.hpp
├── lib
│ └── queue.hpp
├── Makefile
├── parser.cpp
├── parser.hpp
├── proto
│ ├── baserule.cpp
│ ├── baserule.hpp
│ ├── icmp.cpp
│ ├── icmp.hpp
│ ├── ip.cpp
│ ├── ip.hpp
│ ├── tcp.cpp
│ ├── tcp.hpp
│ ├── udp.cpp
│ └── udp.hpp
├── README.md
├── rules.cpp
├── rules.hpp
├── scripts
│ └── send-dump.py
├── sys
│ └── net
│ ├── netmap.h
│ └── netmap_user.h
└── test
└── cppcheck_suppress.cfg
```
Learn more about header files:

* **action.hpp** - classes *Action* and *TriggerJob* responsible for setting
triggered rule triggers, that is, what and how will be executed when the trigger
is triggered;
* **collector.hpp** - the *NetmapPoller* and *NetmapReceiver* classes are
responsible for working with the netmap driver, receiving and transmitting
packets for checking according to the rules;
* **controld.hpp** - classes *ControlSession<T>* and *ControlServer*
responsible for the management console, creating a TCP/UNIX socket server,
parsing received commands;
* **ddosdetector.cpp -main application file**
* **exceptions.hpp** - exception classes, placed in a separate file so that
they can be used as a library in other modules where these exceptions can occur;
* **functions.hpp** - a set of general functions (string conversion, log
initialization, etc.);
* **lib/queue.hpp** - the *ts_queue<T>* class of a thread-safe queue, used for
queuing trigger jobs
* **parser.hpp** - class and functions for parsing/parsing commands from the
management console, or loaded from a file
* **proto/baserule.hpp** - the *NumRange<T>*, *NumComparable<T>*, and
*BaseRule* classes are the base classes for creating traffic analysis rules;
* **proto/ip. hpp** - the *Ipv4Rule* class is responsible for the L3 rule
parameters of the ipv4 packet layer;
* **proto/[icmp, tcp, udp]. hpp** - L4 layer protocol parameter classes;
* **rules.hpp** - the *RulesList<T>*, *RulesCollection*, and *RulesFileLoader*
classes are the main classes for working with traffic validation rules.
The *RulesList<T>* class implements protection of shared data between packet
receiving threads;
* **sys/** - system libraries for working with the netmap driver (taken from
the netmap repository).
* **docs/** - documentation directory
* **influxdb.hpp** - class *InfluxClient* for working with the InfluxDB
database
* **scripts/** - directory with scripts that can be called by the system after
the trigger is triggered
