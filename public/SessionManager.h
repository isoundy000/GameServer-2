/*
 * SessionManager.h
 *
 * Created on: 2013-02-16 15:28
 *     Author: glendy
 */

#ifndef _SESSIONMANAGER_H_
#define _SESSIONMANAGER_H_

#include "PubStruct.h"

class SessionManager
{
public:
    typedef HashMap<string, SessionDetail *, NULL_MUTEX> AccountSessionMap;
    typedef ObjectPoolEx<SessionDetail> SessionDetailPool;

public:
    SessionManager(void);
    ~SessionManager(void);
    void clear(void);

    SessionDetailPool *session_pool(void);

    int bind_account_session(const string &account, SessionDetail *session);
    int unbind_account_session(const string &account, SessionDetail *&session);
    int find_account_session(const string &account, SessionDetail *&session);

    int unbind_and_push(const string &account);
    int update_session_time(const string& account, int add_time = SESSION_TIMEOUT_TICK);
    int update_session(int sid, const string &account, const string &session,
            Int64 role_id = 0, const string &address = string(), int port = 0);

protected:
    AccountSessionMap account_session_map_;
    SessionDetailPool session_pool_;
};

typedef Singleton<SessionManager> SessionManagerSingle;
#define SESSION_MANAGER    (SessionManagerSingle::instance())

#endif //_SESSIONMANAGER_H_
