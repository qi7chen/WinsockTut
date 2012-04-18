
#include <winsock2.h>
#include <MSWSock.h>
#include <memory>
#include <map>
#include "../../src/common/utility.h"


class clients
{
public:
    clients();
    ~clients();

    bool start(const TCHAR* host, short port, int count);

private:
    clients(const clients&);
    clients& operator = (const clients&);

    void        on_connected(PER_HANDLE_DATA* handle_data);
    void        on_recv(PER_HANDLE_DATA* handle_data);
    void        on_close(PER_HANDLE_DATA* handle_data);
    void        after_sent(PER_HANDLE_DATA* handle_data);

    bool                create_one_client(const sockaddr_in& addr);
    PER_HANDLE_DATA*    alloc_handle_data(SOCKET sock_fd);
    void                free_handle_data(PER_HANDLE_DATA* handle_data);

private:
    HANDLE              comletport_handle_;
    LPFN_CONNECTEX      fnconnectex_;
    std::map<SOCKET, PER_HANDLE_DATA*>  info_map_;
};