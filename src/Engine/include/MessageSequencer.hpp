#ifndef MESSAGESEQUENCER_HPP
#define MESSAGESEQUENCER_HPP

#include "Protocol.hpp"
#include "UDPSocket.hpp"

#include <deque>
#include <vector>
#include <time.h>

/* we start to resend after 50ms,                       */
/* then doubling the resend interval after each resend. */
struct QueueMessage {
    QueueMessage() { }
    QueueMessage(sequence_no_t seq_no, flags_t flags, command_t cmd, data_len_t len, data_t *data)
        : resends(0), last_resend_ms(50), seq_no(seq_no), flags(flags), cmd(cmd), len(len), data(data)
    {
        touch.tv_nsec = 0;
        touch.tv_sec = 0;
    }

    ~QueueMessage() {
        if (data) {
            delete[] data;
        }
    }

    gametime_t touch;
    pico_size_t resends;
    int last_resend_ms;
    sequence_no_t seq_no;
    flags_t flags;
    command_t cmd;
    data_len_t len;
    data_t *data;
};

struct Connection {
    Connection(hostaddr_t host, hostport_t port)
        : host(host), port(port), ping_time(0) { }

    hostaddr_t host;
    hostport_t port;
    ms_t ping_time;
};

struct SequencerHeap : public Connection {
    typedef std::deque<QueueMessage *> QueuedMessages;

    SequencerHeap(hostaddr_t host, hostport_t port)
        : Connection(host, port), sent_pings(0), active(true),
          last_send_unrel_seq_no(0), last_send_rel_seq_no(0),
          last_recv_unrel_seq_no(0), last_recv_rel_seq_no(0)
    {
        last_ping.tv_sec = 0;
        last_ping.tv_nsec = 0;
    }

    int sent_pings;
    bool active;

    sequence_no_t last_send_unrel_seq_no;
    sequence_no_t last_send_rel_seq_no;
    sequence_no_t last_recv_unrel_seq_no;
    sequence_no_t last_recv_rel_seq_no;

    gametime_t last_ping;

    QueuedMessages in_queue;
    QueuedMessages out_queue;
};

class MessageSequencer {
public:
    enum LogoutReason {
        LogoutReasonRegular = 0,
        LogoutReasonPingTimeout,
        LogoutReasonTooManyResends,
        LogoutApplicationQuit
    };

    enum RefusalReason {
        RefusalReasonServerFull = 0,
        RefusalReasonWrongPassword
    };

    MessageSequencer(hostport_t port, pico_size_t num_heaps, const std::string& name, const std::string& password) throw (Exception);
    MessageSequencer(hostaddr_t server_host, hostport_t server_port) throw (Exception);
    virtual ~MessageSequencer();

    void login(const std::string& password, data_len_t len, const void *data) throw (Exception);
    void login(data_len_t len, const void *data) throw (Exception);
    void logout() throw (Exception);

    void request_server_info(hostaddr_t host, hostport_t port) throw (Exception);
    void broadcast(flags_t flags, data_len_t len, const void *data) throw (Exception);
    void push(flags_t flags, data_len_t len, const void *data) throw (Exception);
    void push(flags_t flags, command_t cmd, data_len_t len, const void *data) throw (Exception);
    void push(const Connection *c, flags_t flags, data_len_t len, const void *data) throw (Exception);
    void push(const Connection *c, command_t cmd, flags_t flags, data_len_t len, const void *data) throw (Exception);
    bool cycle() throw (Exception);
    void kill(const Connection *c) throw (Exception);

    virtual void event_status(hostaddr_t host, hostport_t port, const std::string& name,
        int max_clients, int cur_clients, ms_t ping_time, bool secured,
        int protocol_version) throw (Exception) { }
    virtual void event_access_denied(RefusalReason reason) throw (Exception) { }
    virtual void event_login(const Connection *c, data_len_t len, void *data) throw (Exception) { }
    virtual void event_data(const Connection *c, data_len_t len, void *data) throw (Exception) { }
    virtual void event_logout(const Connection *c, LogoutReason reason) throw (Exception) { }

private:
    typedef std::vector<SequencerHeap *> SequencerHeaps;

    pico_size_t max_heaps;
    bool is_client;
    std::string name;
    std::string password;
    UDPSocket socket;
    NetMessage *pmsg;
    NetMessageData *pdata;

    SequencerHeaps heaps;
    char buffer[1024];

    void ack(SequencerHeap *heap, sequence_no_t seq_no) throw (Exception);
    void process_incoming(SequencerHeap *heap, NetMessage *msg) throw (Exception);
    void sorted_insert(SequencerHeap::QueuedMessages& msg_queue, QueueMessage *smsg);
    SequencerHeap *find_heap(hostaddr_t host, hostport_t port);
    SequencerHeap *find_heap(const Connection *c);
    void delete_heap(SequencerHeap *heap);
    void flush_queues(SequencerHeap *heap);
    void slack_send(hostaddr_t host, hostport_t port, sequence_no_t seq_no,
        flags_t flags, command_t cmd, data_len_t len, const void *data) throw (Exception);
    void delete_all_heaps();
};

#endif // MESSAGESEQUENCER_HPP
