/*
 * SessionManager.cpp
 *
 * Created on: 2013-02-16 15:52
 *     Author: glendy
 */

#include "SessionManager.h"

SessionManager::SessionManager(void) :
    account_session_map_(get_hash_table_size(5000))
{ /*NULL*/ }

SessionManager::~SessionManager(void)
{
    this->account_session_map_.unbind_all();
    this->session_pool_.clear();
}

void SessionManager::clear(void)
{
    for (AccountSessionMap::iterator iter = this->account_session_map_.begin();
            iter != this->account_session_map_.end(); ++iter)
    {
        this->session_pool_.push(iter->second);
    }
    this->account_session_map_.unbind_all();
}

SessionManager::SessionDetailPool *SessionManager::session_pool(void)
{
    return &(this->session_pool_);
}

int SessionManager::bind_account_session(const string &account, SessionDetail *session)
{
    return this->account_session_map_.bind(account, session);
}

int SessionManager::unbind_account_session(const string &account, SessionDetail *&session)
{
    return this->account_session_map_.unbind(account, session);
}

int SessionManager::find_account_session(const string &account, SessionDetail *&session)
{
    return this->account_session_map_.find(account, session);
}

int SessionManager::unbind_and_push(const string &account)
{
	SessionDetail *session = 0;
    JUDGE_RETURN(this->unbind_account_session(account, session) == 0, -1);

	return this->session_pool()->push(session);
}

int SessionManager::update_session_time(const string& account, int add_time)
{
    SessionDetail *session = 0;
    JUDGE_RETURN(this->find_account_session(account, session) == 0, 0);

    return session->refresh_check_tick(add_time);
}

int SessionManager::update_session(int sid, const string &account, const string &str_session,
        Int64 role_id, const string &address, int port)
{
    SessionDetail *session = 0;
    if (this->find_account_session(account, session) != 0)
    {
        session = this->session_pool()->pop();
        JUDGE_RETURN(session != NULL, -1);

        session->__account = account;
        this->bind_account_session(account, session);
    }

    session->__timeout_tick = Time_Value::gettimeofday() + Time_Value(SESSION_TIMEOUT_TICK);
    session->__session = str_session;
    session->__client_sid = sid;
    session->__role_id = role_id;
    session->__address = address;
    session->__port = port;

    return 0;
}

