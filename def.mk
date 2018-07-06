CFLAGS=
CXXFLAGS=
server_CFLAGS=-Wall -Wno-reorder -O0 -g3 -fmessage-length=0 -std=c++0x
server_CXXFLAGS=-Wall -Wno-reorder -O0 -g3 -fmessage-length=0 -std=c++0x
DEFS =-DTASK_RENAME -DTEST_SERIAL -DLOCAL_DEBUG -DSEQUENCE_VALIDATE -DNO_BROAD_PORT -DTEST_COMMAND -DNEW_LOGIN_LOGIC -DUSE_DOMAIN -ULOCAL_DEBUG -USEQUENCE_VALIDATE -UTEST_SERIAL -UTEST_COMMAND0

server_LDFLAGS=-lmysqlcppconn -lmongoclient -lboost_thread-mt -lboost_filesystem -lboost_program_options -ljsoncpp -lcrypto -lprotobuf -lpthread
server_LDADD= ${prefix}/lib/liblibserver.a

